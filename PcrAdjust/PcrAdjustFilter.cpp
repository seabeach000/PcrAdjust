#include "stdafx.h"
#include "PcrAdjustFilter.h"
#include <atlstr.h>
#define SYNC_BYTE 0x47
#define TS_PACKET 188
#define MPEG_PES_PACKET_START_PREFIX		"\x00\x00\x01"
#define MPEG_TS_HEADER_SIZE   4

#define MAKE_PID(p) (((p[0]<<8) + p[1]) & 0x1fff)

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif
#ifndef AV_RB24
#   define AV_RB24(x)                           \
    ((((const uint8_t*)(x))[0] << 16) |         \
     (((const uint8_t*)(x))[1] <<  8) |         \
      ((const uint8_t*)(x))[2])
#endif
#   define AV_RB32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])

CPcrAdjustFilter::CPcrAdjustFilter(TCHAR *pName, LPUNKNOWN lpunk, HRESULT *phr)
	:CTransInPlaceFilter(pName, lpunk, __uuidof(this), phr, false)
{
}


CPcrAdjustFilter::~CPcrAdjustFilter()
{
}

CUnknown* CALLBACK CPcrAdjustFilter::CreateInstance(LPUNKNOWN pUnkOuter, HRESULT* phr)
{
	return new CPcrAdjustFilter(TEXT("Zqvideo Ts Pcr adjust"), pUnkOuter, phr);
}

std::vector<int> CPcrAdjustFilter::kmpNext(BYTE* pPattern, const int nLen)
{
	std::vector<int> vNext(nLen);

	vNext[0] = 0;
	// i stands for index of string, j is temporary for particail match
	// values computing, at the beginning of each loop process, j is the
	// particial match value of former character .
	for (int i = 1, j = 0; i < nLen; ++i) {
		while (j > 0 && pPattern[i] != pPattern[j])
		{
			// This loop is to get a matching character recursively. Another
			// stop condition is when particial match value meets end.
			j = vNext[j - 1];// j will be recomputed in the recursion. Take
							 // care that next[j-1] is the particial match
							 // value of the first j characters substirng.
		}

		if (pPattern[i] == pPattern[j]) // If not in this case, j must meets end, equals to zero.
			++j;

		vNext[i] = j;
	}

	return vNext;

}

int CPcrAdjustFilter::kmp(BYTE* pBuf, BYTE* pPattern, int nBuflen)
{
	int nIdx = -1;

	if (NULL == pBuf || NULL == pPattern)
	{
		return nIdx;
	}

	// i stands for index of str string, j stands for index in dest string.
	// At the beginning of each loop process, j is the new position of dest
	// taht should be compared.

	//int nBufLen = strlen((const char*)pBuf);
	int nPatternLen = 3;// strlen((const char*)pPattern);

	std::vector<int> vNext = kmpNext(pPattern, nPatternLen);// call 0ne
	int j = 0;

	for (int i = 0; i < nBuflen; i++) {
		while (j > 0 && pBuf[i] != pPattern[j])
		{
			// This loop is to get a matching character recursively. Another
			// stop condition is when particial match value meets end.
			j = vNext[j - 1];// As i in str and j in dest is comparing,
							 // recomputing of j should be in the former
							 // character substring, which is next[j-1]
		}

		if (pBuf[i] == pPattern[j])
			j++;

		if (j == nPatternLen)
		{
			nIdx = i - nPatternLen + 1;
			break;
		}
	}

	return nIdx;
}

HRESULT CPcrAdjustFilter::Transform(IMediaSample* pSample)
{
	BYTE* pBuffer = 0;
	pSample->GetPointer(&pBuffer);
	long dataLen = pSample->GetActualDataLength();
	if (m_nStartOffset >= dataLen)
	{
		m_nStartOffset -= dataLen;
		return S_OK;
	}
	pBuffer += m_nStartOffset;
	dataLen -= m_nStartOffset;

	for (; dataLen > 0;)
	{
		if (m_bNeedSync || *pBuffer != SYNC_BYTE)
		{
			for (; dataLen > 0;)
			{
				while (*pBuffer != SYNC_BYTE && dataLen > 0)
				{
					pBuffer++;
					dataLen--;
				}
				if (*pBuffer != SYNC_BYTE || dataLen <= 0)
				{
					m_nStartOffset = 0;
					m_bNeedSync = true;
					return S_OK;                        //数据找完都没有找到同步点直接返回
				}
				if (dataLen < TS_PACKET)
				{
					m_bNeedSync = false;
					m_nStartOffset = TS_PACKET - dataLen;
					return S_OK;                        //找到同步点但不够一个ts包，也返回，得到一个偏移量
				}
				if (pBuffer[TS_PACKET] == SYNC_BYTE)
				{
					m_bNeedSync = false;
					break;                              //找到一个完整的ts包
				}

				pBuffer++;
				dataLen--;
			}
			if (dataLen <= 0)
			{
				m_nStartOffset = 0;
				m_bNeedSync = true;
				return S_OK;
			}
		}
		//到此找到同步
		ASSERT(*pBuffer == SYNC_BYTE);
		//针对每一个ts包进行分析并修改
		TS_packet_header tsHeader;
		ParseTSPacketHeader(pBuffer, &tsHeader);
		if (tsHeader.PID == 0 && m_nPmt_PID == 0)
		{
			//PAT
			m_nPmt_PID = GetPMT_PID(pBuffer);
		}                                                      //是PAT表就不是PMT表，每个PMT表一个ts包
		else if (m_nPcr_PID == 0 && m_nPmt_PID > 0)
		{
			//m_nPcr_PID = GetPCR_PID(pBuffer);
			if (m_nPmt_PID != tsHeader.PID)
			{
				return 0;
			}
			if (!tsHeader.payload_unit_start_indicator)
			{
				return 0;
			}
			//skip the ts header
			BYTE* pBufferTem = pBuffer + MPEG_TS_HEADER_SIZE;
			if (tsHeader.adaption_field_control & 0x02)
			{
				pBufferTem += 1 + pBufferTem[0];
			}
			pBufferTem += 1 + pBufferTem[0];

			TS_PMT pmt;
			Parser_PMT_Table(&pmt, pBufferTem);
			m_nPcr_PID = pmt.PCR_PID;
			for (std::vector<TS_PMT_Stream>::iterator iter = pmt.PMT_Stream.begin(); iter != pmt.PMT_Stream.end(); iter++)
			{
				if (iter->stream_type == STREAM_TYPE_VIDEO_H264 ||
					iter->stream_type == STREAM_TYPE_VIDEO_MPEG2 ||
					iter->stream_type == STREAM_TYPE_VIDEO_HEVC)
				{
					m_nVideo_PID.push_back(iter->elementary_PID);

				}
				else if (iter->stream_type == STREAM_TYPE_AUDIO_MPEG1 ||
					iter->stream_type == STREAM_TYPE_AUDIO_MPEG2 ||
					iter->stream_type == STREAM_TYPE_AUDIO_AAC ||
					iter->stream_type == STREAM_TYPE_AUDIO_AAC_LATM ||
					iter->stream_type == STREAM_TYPE_AUDIO_AC3)
				{
					m_nAduio_PID.insert(iter->elementary_PID);
				}
			}
		}
		else if (m_nPcr_PID > 0)
		{
			bool findPCR = false;
			if (tsHeader.PID == m_nPcr_PID)    //也有可能是视频包
			{
				findPCR = GetandAdjustPcr(pBuffer, TS_PACKET);
			}
			if (!findPCR)
			{
				//是否是PES包
				if (tsHeader.PID == m_nVideo_PID.front())
				{
					GetPTSDTS(pBuffer, TS_PACKET);
						//video pts 
						//CString strlog;
						//strlog.Format(_T("VideoPTS = %I64d,lastPcr = %I64d,videoPTS-m_nLastPcr = %I64d \r\n"), m_ncurrentPTS, m_nLastPcr, m_ncurrentPTS - m_nLastPcr / 300);
						//OutputDebugString(strlog);
						//CString strlog;
						//strlog.Format(_T("currentPts = %I64d,lastPts = %I64d,ts-m_nLastPts = %I64d \r\n"), m_ncurrentPTS, m_nLastVideoPts, m_ncurrentPTS - m_nLastVideoPts);
						//OutputDebugString(strlog);
						//m_nLastVideoPts = m_ncurrentPTS;
				}
				else
				{
					std::set<int>::iterator it;
					it = m_nAduio_PID.find(tsHeader.PID);
					if (it != m_nAduio_PID.end())
					{
						GetPTSDTS(pBuffer, TS_PACKET);
							//Audio pts
							CString strlog;
						strlog.Format(_T("			AudioPTS = %I64d,lastPcr = %I64d,audioPTS-m_nLastPcr = %I64d \r\n"), m_ncurrentPTS, m_nLastPcr, m_ncurrentPTS - m_nLastPcr / 300);
						//OutputDebugString(strlog);
					}
					else
					{
						//除了视频和音频可能还有别的类型
						GetPTSDTS(pBuffer, TS_PACKET);
					}
				}
			}
		}
		//到此包分析完毕

		pBuffer += TS_PACKET;
		dataLen -= TS_PACKET;
	}
	m_nStartOffset = -dataLen;
	return S_OK;
}

HRESULT CPcrAdjustFilter::CheckInputType(const CMediaType* mtIn)
{
	if (!mtIn)
		return E_POINTER;
	else if (mtIn->majortype == MEDIATYPE_Stream &&
		mtIn->subtype == MEDIASUBTYPE_MPEG2_TRANSPORT)
		return S_OK;
	else
		return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CPcrAdjustFilter::StartStreaming()
{
	return __super::StartStreaming();
}

HRESULT CPcrAdjustFilter::StopStreaming()
{
	m_nStartOffset = 0;
	m_bNeedSync = true;
	m_nPmt_PID = 0;
	m_nPcr_PID = 0;
	m_nLastPcr = 0;
	return __super::StopStreaming();
}

bool CPcrAdjustFilter::GetandAdjustPcr(BYTE* pBuffer, long datalen)
{
	int64_t pcr_h, pcr_l;
	int afc, len, flags;
	const uint8_t *p;
	unsigned int v;

	afc = (pBuffer[3] >> 4) & 3;
	if (afc <= 1)
		return false;
	p = pBuffer + 4;
	len = p[0];//得到adaptation_field_length
	p++;
	if (len == 0)
	{
		return false;
	}
	flags = *p++;
	len--;
	if (!(flags & 0x10))
	{
		return false;
	}
	if (len < 6)
	{
		return false;
	}
	v = AV_RB32(p);
	pcr_h = ((int64_t)v << 1) | (p[4] >> 7);
	pcr_l = ((p[4] & 1) << 8) | p[5];

	int64_t pcr = pcr_h * 300 + pcr_l;
	if (m_nLastPcr == 0)
	{
		m_nLastPcr = pcr;
	}
	CString strlog;
	strlog.Format(_T("currentPcr = %I64d,lastPcr = %I64d,pcr-m_nLastPcr = %I64d \r\n"), pcr, m_nLastPcr, pcr - m_nLastPcr);
	//OutputDebugString(strlog);
	m_nLastPcr = pcr;
	return true;
}

HRESULT CPcrAdjustFilter::ParseTSPacketHeader(BYTE* pBuffer, TS_packet_header* TS_header)
{
	TS_header->transport_error_indicator = pBuffer[1] >> 7;
	TS_header->payload_unit_start_indicator = pBuffer[1] >> 6 & 0x01;
	TS_header->transport_priority = pBuffer[1] >> 5 & 0x01;
	TS_header->PID = (pBuffer[1] & 0x1F) << 8 | pBuffer[2];
	TS_header->transport_scrambling_control = pBuffer[3] >> 6;
	TS_header->adaption_field_control = pBuffer[3] >> 4 & 0x03;
	TS_header->continuity_counter = pBuffer[3] & 0x0F; // 四位数据,应为0x0F
	return S_OK;
}

int CPcrAdjustFilter::GetPMT_PID(BYTE* pBuffer)
{
	//Skip the TS header
	BYTE* pBufferTem = pBuffer;
	pBufferTem += MPEG_TS_HEADER_SIZE;
	// Adaptation field exist or not 
	if ((pBuffer[3] >> 4) & 0x02)
	{
		pBufferTem += 1 + pBufferTem[0];// pBufferTem[0]是Adaptation field长度, 1为长度
	}
	// pBufferTem[0] 为 Point field长度， 1为Point field  
	pBufferTem += 1 + pBufferTem[0];
	// 8 为PAT表头长度，如果定义了表头，则用sizeof运算符 
	pBufferTem += 8;
	// NIT Program exist??  
	if (*(USHORT*)pBufferTem == 0)
	{
		pBufferTem += 4;// PAT的节目结构为4字节
	}
	pBufferTem += 2;// 节目号为两个字节

	return MAKE_PID(pBufferTem);
}

int CPcrAdjustFilter::GetPCR_PID(BYTE* pBuffer)
{
	TS_packet_header tsHeader;
	ParseTSPacketHeader(pBuffer, &tsHeader);
	BYTE* pBufferTem = pBuffer;
	if (m_nPmt_PID > 0)
	{
		if (m_nPmt_PID != tsHeader.PID)
		{
			return 0;
		}
		if (!tsHeader.payload_unit_start_indicator)
		{
			return 0;
		}
		//skip the ts header
		pBufferTem += MPEG_TS_HEADER_SIZE;
		if (tsHeader.adaption_field_control & 0x02)
		{
			pBufferTem += 1 + pBufferTem[0];
		}
		pBufferTem += 1 + pBufferTem[0];
		// 8 为PCR PID在PMT表头中的偏移
		pBufferTem += 8;
		m_nPcr_PID = MAKE_PID(pBufferTem);
		if (m_nPcr_PID == 0x1fff)
		{
			m_nPcr_PID = 0;
		}
	}

	return m_nPcr_PID;
}

HRESULT CPcrAdjustFilter::Parser_PMT_Table(TS_PMT* pmt, BYTE* pBuffer)
{
	pmt->table_id = pBuffer[0];
	pmt->section_syntax_indicator = pBuffer[1] >> 7;
	pmt->zero = pBuffer[1] >> 6 & 0x01;
	pmt->reserved_1 = pBuffer[1] >> 4 & 0x03;
	pmt->section_length = (pBuffer[1] & 0x0F) << 8 | pBuffer[2];
	pmt->program_number = pBuffer[3] << 8 | pBuffer[4];
	pmt->reserved_2 = pBuffer[5] >> 6;
	pmt->version_number = pBuffer[5] >> 1 & 0x1F;
	pmt->current_next_indicator = (pBuffer[5] << 7) >> 7;
	pmt->section_number = pBuffer[6];
	pmt->last_section_number = pBuffer[7];
	pmt->reserved_3 = pBuffer[8] >> 5;
	pmt->PCR_PID = ((pBuffer[8] << 8) | pBuffer[9]) & 0x1FFF;

	m_nPcr_PID = pmt->PCR_PID;

	pmt->reserved_4 = pBuffer[10] >> 4;
	pmt->program_info_length = (pBuffer[10] & 0x0F) << 8 | pBuffer[11];

	// Get CRC_32  
	int len = 0;
	len = pmt->section_length + 3;
	pmt->CRC_32 = (pBuffer[len - 4] & 0x000000FF) << 24
		| (pBuffer[len - 3] & 0x000000FF) << 16
		| (pBuffer[len - 2] & 0x000000FF) << 8
		| (pBuffer[len - 1] & 0x000000FF);
	int pos = 12;
	//program info descriptor
	if (pmt->program_info_length != 0)
		pos += pmt->program_info_length;
	//Get stream type and PID
	for (; pos <= (pmt->section_length + 2) - 4;)
	{
		TS_PMT_Stream pmt_stream;
		pmt_stream.stream_type = pBuffer[pos];
		pmt->reserved_5 = pBuffer[pos + 1] >> 5;
		pmt_stream.elementary_PID = ((pBuffer[pos + 1] << 8) | pBuffer[pos + 2]) & 0x1FFF;
		pmt->reserved_6 = pBuffer[pos + 3] >> 4;
		pmt_stream.ES_info_length = (pBuffer[pos + 3] & 0x0F) << 8 | pBuffer[pos + 4];

		pmt_stream.descriptor = 0x00;
		if (pmt_stream.ES_info_length != 0)
		{
			pmt_stream.descriptor = pBuffer[pos + 5];

			for (int len = 2; len <= pmt_stream.ES_info_length; len++)
			{
				pmt_stream.descriptor = pmt_stream.descriptor << 8 | pBuffer[pos + 4 + len];
			}
			pos += pmt_stream.ES_info_length;
		}
		pos += 5;
		pmt->PMT_Stream.push_back(pmt_stream);
		//TS_Stream_type.push_back(pmt_stream);		
	}

	return S_OK;
}

int64_t CPcrAdjustFilter::GetPTSDTS(BYTE* pBuffer, long datalen)
{
	int64_t ptsValue = 0, dtsValue = 0;
	BYTE PESStartprefix[3];
	PESStartprefix[0] = 0x00; PESStartprefix[1] = 0x00; PESStartprefix[2] = 0x01;
	int idx = kmp(pBuffer, PESStartprefix, datalen);
	if (idx >= 3 && idx < 175)
	{
		if (pBuffer[idx + 3] == 0xE0 || pBuffer[idx + 3] == 0xC0 || pBuffer[idx + 3] == 0xBD)
		{
			USHORT PTS_or_DTS_flag = (pBuffer[idx + 7] & 0xC0) >> 6;
			bool bHavePts = PTS_or_DTS_flag & 0x02;
			bool bHaveDts = PTS_or_DTS_flag & 0x01;
			if (!bHavePts)
				return false;

			if (bHavePts)
			{
				ptsValue = ParserPts(pBuffer + idx + 9);
				if (ptsValue == 0)
				{
					return false;
				}
				if(pBuffer[idx + 3] == 0xE0)
				{
					CString strlog;
					strlog.Format(_T("currentPts = %I64d,lastPts = %I64d,ts-m_nLastPcr = %I64d \r\n"), ptsValue, m_nLastPcr, ptsValue - m_nLastPcr/300);
					OutputDebugString(strlog);
					m_nLastVideoPts = ptsValue;
					m_ncurrentPTS = ptsValue;
				}
			}
			if (bHaveDts)
			{
				dtsValue = ParserPts(pBuffer + idx + 9 + 5);
				if (dtsValue == 0)
				{
					return false;
				}
				m_ncurrentDTS = dtsValue;
			}
		}
	}
	else
		return false;

	return true;
}

int64_t CPcrAdjustFilter::ParserPts(const BYTE* pBuffer)
{
	__int64 pts = (__int64)((pBuffer[0] >> 1) & 0x07) << 30;
	pts |= (AV_RB16(pBuffer + 1) >> 1) << 15;
	pts |= AV_RB16(pBuffer + 3) >> 1;
	return pts;
}
