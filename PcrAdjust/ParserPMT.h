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
	unsigned stream_type : 8; //指示特定PID的节目元素包的类型。该处PID由elementary PID指定  
	unsigned elementary_PID : 13; //该域指示TS包的PID值。这些TS包含有相关的节目元素  
	unsigned ES_info_length : 12; //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数  
	unsigned descriptor;
}TS_PMT_Stream;

//PMT 表结构体  
typedef struct TS_PMT
{
	unsigned table_id : 8; //固定为0x02, 表示PMT表  
	unsigned section_syntax_indicator : 1; //固定为0x01  
	unsigned zero : 1; //0x01  
	unsigned reserved_1 : 2; //0x03  
	unsigned section_length : 12;//首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。  
	unsigned program_number : 16;// 指出该节目对应于可应用的Program map PID  
	unsigned reserved_2 : 2; //0x03  
	unsigned version_number : 5; //指出TS流中Program map section的版本号  
	unsigned current_next_indicator : 1; //当该位置1时，当前传送的Program map section可用；  
										 //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。  
	unsigned section_number : 8; //固定为0x00  
	unsigned last_section_number : 8; //固定为0x00  
	unsigned reserved_3 : 3; //0x07  
	unsigned PCR_PID : 13; //指明TS包的PID值，该TS包含有PCR域，  
						   //该PCR值对应于由节目号指定的对应节目。  
						   //如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。  
	unsigned reserved_4 : 4; //预留为0x0F  
	unsigned program_info_length : 12; //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。  

	std::vector<TS_PMT_Stream> PMT_Stream;  //每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定  
	unsigned reserved_5 : 3; //0x07  
	unsigned reserved_6 : 4; //0x0F  
	unsigned CRC_32 : 32;
} TS_PMT;