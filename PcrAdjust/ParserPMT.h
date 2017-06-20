#pragma once
#include <vector>

#define TS_FEC_PACKET_SIZE 204
#define TS_DVHS_PACKET_SIZE 192
#define TS_PACKET_SIZE 188
#define TS_MAX_PACKET_SIZE 204

#define NB_PID_MAX 8192
#define MAX_SECTION_SIZE 4096

/* pids */
#define PAT_PID                 0x0000
#define SDT_PID                 0x0011

/* table ids */
#define PAT_TID   0x00
#define PMT_TID   0x02
#define M4OD_TID  0x05
#define SDT_TID   0x42

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_HEVC      0x24
#define STREAM_TYPE_VIDEO_CAVS      0x42
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1

#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x82
#define STREAM_TYPE_AUDIO_TRUEHD    0x83

typedef struct TS_PMT_Stream
{
	unsigned stream_type : 8; //ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��  
	unsigned elementary_PID : 13; //����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��  
	unsigned ES_info_length : 12; //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��  
	unsigned descriptor;
}TS_PMT_Stream;

//PMT ��ṹ��  
typedef struct TS_PMT
{
	unsigned table_id : 8; //�̶�Ϊ0x02, ��ʾPMT��  
	unsigned section_syntax_indicator : 1; //�̶�Ϊ0x01  
	unsigned zero : 1; //0x01  
	unsigned reserved_1 : 2; //0x03  
	unsigned section_length : 12;//������λbit��Ϊ00����ָʾ�ε�byte�����ɶγ�����ʼ������CRC��  
	unsigned program_number : 16;// ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID  
	unsigned reserved_2 : 2; //0x03  
	unsigned version_number : 5; //ָ��TS����Program map section�İ汾��  
	unsigned current_next_indicator : 1; //����λ��1ʱ����ǰ���͵�Program map section���ã�  
										 //����λ��0ʱ��ָʾ��ǰ���͵�Program map section�����ã���һ��TS����Program map section��Ч��  
	unsigned section_number : 8; //�̶�Ϊ0x00  
	unsigned last_section_number : 8; //�̶�Ϊ0x00  
	unsigned reserved_3 : 3; //0x07  
	unsigned PCR_PID : 13; //ָ��TS����PIDֵ����TS������PCR��  
						   //��PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ��  
						   //�������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��  
	unsigned reserved_4 : 4; //Ԥ��Ϊ0x0F  
	unsigned program_info_length : 12; //ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����  

	std::vector<TS_PMT_Stream> PMT_Stream;  //ÿ��Ԫ�ذ���8λ, ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��  
	unsigned reserved_5 : 3; //0x07  
	unsigned reserved_6 : 4; //0x0F  
	unsigned CRC_32 : 32;
} TS_PMT;