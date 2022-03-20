#ifndef __DDS_H
#define __DDS_H
#include "base.h"
#define MAX_LENGTH 		(u32)4096
#define DAC_AMP 		(u16)2048 //峰峰值
#define DDS_MAX_AMP 	(float)3.3f
void dds_tableCreate(u16 length);//生成正弦
void dds_start(void);
void dds_setWaveParams(u32 freq,float amp);//设置参数
typedef struct
{
	u32 freq;//正弦频率
	u32 length;//波表长度
	float amp;//正弦波幅度
} DDS_TypeDef;
#endif