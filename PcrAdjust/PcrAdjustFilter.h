#pragma once
#include "streams.h"
#include <vector>
#include <set>
#include "ParserPMT.h"
typedef struct TS_packet_header
{
	unsigned sync_byte : 8; //ͬ���ֽ�, �̶�Ϊ0x47,��ʾ�������һ��TS����
	unsigned transport_error_indicator : 1; //��������ָʾ��
	unsigned payload_unit_start_indicator : 1; //��Ч���ص�Ԫ��ʼָʾ��

	unsigned transport_priority : 1; //��������, 1��ʾ�����ȼ�,������ƿ����õ��������ò���
	unsigned PID : 13; //PID
	unsigned transport_scrambling_control : 2; //������ſ��� 
	unsigned adaption_field_control : 2; //����Ӧ���� 01������Ч���أ�10���������ֶΣ�11���е����ֶκ���Ч���ء�Ϊ00�����������д���
	unsigned continuity_counter : 4; //���������� һ��4bit�ļ���������Χ0-15
} TS_packet_header;

[uuid("06402A72-0DA7-4258-9DAC-497F505F3D87")]
class CPcrAdjustFilter :
	public CTransInPlaceFilter
{
public:
	CPcrAdjustFilter(TCHAR *pName, LPUNKNOWN lpunk, HRESULT *phr);
	virtual ~CPcrAdjustFilter();

public:
	static CUnknown* CALLBACK CreateInstance(LPUNKNOWN pUnkOuter, HRESULT* phr);
	std::vector<int> kmpNext(BYTE* pPattern, const int nLen);
	int kmp(BYTE* pBuf, BYTE* pPattern, int nBuflen);
protected:
	HRESULT Transform(IMediaSample* pSample);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT StartStreaming();
	HRESULT StopStreaming();
private:
	bool GetandAdjustPcr(BYTE* pBuffer, long datalen);
	HRESULT ParseTSPacketHeader(BYTE* pBuffer,TS_packet_header* TS_header);
	int GetPMT_PID(BYTE* pBuffer);
	int GetPCR_PID(BYTE* pBuffer);
	HRESULT Parser_PMT_Table(TS_PMT* pmt, BYTE* pBuffer);
	int64_t GetPTSDTS(BYTE* pBuffer, long datalen);
	int64_t ParserPts(const BYTE* pBuffer);
private:
	int m_nStartOffset = 0;
	bool m_bNeedSync = true;
	int m_nPmt_PID = 0;
	int m_nPcr_PID = 0;
	int64_t m_nLastPcr = 0;
	std::vector<int> m_nVideo_PID;
	std::set<int> m_nAduio_PID;
	int64_t m_ncurrentPTS = 0;
	int64_t m_ncurrentDTS = 0;
	int64_t m_nLastVideoPts = 0;
};

