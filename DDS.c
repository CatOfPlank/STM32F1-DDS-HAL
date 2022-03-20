//dds文件
//只需要改变波表点数N即可改变频率,最大频率可达2M
//设点数为N，则频率f=72M/psc/arr/N*2，注意N<MAX_LENGTH
//固定相位累加器步长m=1
//定时器更新事件时钟作为DDS主振荡器时钟
//乘二是因为DMA以字数据传输
#include "main.h"
#include "math.h"
#include "dac.h"
#include "tim.h"
#include "DDS.h"
//生成波表
DDS_TypeDef dds1;
volatile u16  dds_table1[MAX_LENGTH];

void dds_tableCreate(u16 length)//创建波表
{
        float sin_step = 2.0f * 3.1415926f / (float)(length-1);
        for (u16 i = 0; i < length; i++)
        {
            dds_table1[i] = (uint16_t)(DAC_AMP+(DAC_AMP*sinf(sin_step*(float)i)));//相位累加器步长m=1
        }
}
void dds_start()
{
		dds1.freq=1000;
		dds1.amp=4.5;
		TIM6->PSC = 180 - 1;//
		TIM6->ARR = 10 - 1;
		dds1.length=400;//N=4000,f=72M/1/10/4000*2=1K
		dds_tableCreate(dds1.length);
		HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1,(u32*)dds_table1,dds1.length/2,DAC_ALIGN_12B_R);//DMA每次搬运一个32位的字数据，u16的数组只需搬运length/2次
		HAL_TIM_Base_Start(&htim6);
}
//DMA无法间隔取点,只能通过改变波表长度来改变频率
void dds_setWaveParams(u32 freq,float amp)//设置参数
{
		HAL_TIM_Base_Stop(&htim6);
		HAL_DAC_Stop_DMA(&hdac,DAC_CHANNEL_1);
		dds1.freq=freq;
		if(freq==1)		//	for very low frequencies, FMCLK = 2kHz, 72M / 720 / 50 = 2kHz
		{
			TIM6->PSC = 720 - 1;
			TIM6->ARR = 50 - 1;
			dds1.length = 4000;
			dds_tableCreate(dds1.length);
		}
		else if(freq == 10)		//	FMCLK = 20kHz, 72M / 72 / 50 = 20kHz
		{
			TIM6->PSC = 72 - 1;
			TIM6->ARR = 50 - 1;
			dds1.length=4000;
			dds_tableCreate(dds1.length);
		}
		else if(freq > 10&&freq <=100)
		{
			TIM6->PSC = 72-1;
			TIM6->ARR = 50-1;
			dds1.length = (u32)(40000/freq);
			dds_tableCreate(dds1.length);			
		}
		else if(freq>100&&freq<=1000)
		{
			TIM6->PSC = 36-1;
			TIM6->ARR = 10-1;
			dds1.length = (u32)(400000/freq);
			dds_tableCreate(dds1.length);			
		}
		else if(freq>1000&&freq<=10000)
		{
			TIM6->PSC = 4-1;
			TIM6->ARR = 9-1;
			dds1.length = (u32)(4000000/freq);
			dds_tableCreate(dds1.length);			
		}
		else if(freq>10000&&freq<=400000)
		{
			TIM6->PSC = 0;
			TIM6->ARR = 18-1;
			dds1.length = (u32)(8000000/freq);
			dds_tableCreate(dds1.length);			
		}
		else 
		{
			TIM6->PSC = 0;
			TIM6->ARR = 9 - 1;
			dds1.length = (u32)(16000000/freq);
			dds_tableCreate(dds1.length);
		}
		dds1.amp =amp;
		for(u16 i=0;i<dds1.length;i++)
		{
			dds_table1[i] = dds_table1[i]*(amp/(DDS_MAX_AMP));
		}
		HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1,(u32*)dds_table1,(u32)dds1.length/2,DAC_ALIGN_12B_R);
		HAL_TIM_Base_Start(&htim6);
}