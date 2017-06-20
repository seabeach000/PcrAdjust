#pragma once
#include "streams.h"
#include <vector>
#include <set>
#include "ParserPMT.h"
typedef struct TS_packet_header
{
	unsigned sync_byte : 8; //同步字节, 固定为0x47,表示后面的是一个TS分组
	unsigned transport_error_indicator : 1; //传输误码指示符
	unsigned payload_unit_start_indicator : 1; //有效荷载单元起始指示符

	unsigned transport_priority : 1; //传输优先, 1表示高优先级,传输机制可能用到，解码用不着
	unsigned PID : 13; //PID
	unsigned transport_scrambling_control : 2; //传输加扰控制 
	unsigned adaption_field_control : 2; //自适应控制 01仅含有效负载，10仅含调整字段，11含有调整字段和有效负载。为00解码器不进行处理
	unsigned continuity_counter : 4; //连续计数器 一个4bit的计数器，范围0-15
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

