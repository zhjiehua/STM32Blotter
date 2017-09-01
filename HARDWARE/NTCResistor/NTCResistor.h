#ifndef __NTCRESISTOR_H
#define __NTCRESISTOR_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//蜂鸣器驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
typedef struct
{
	float temperature;
	float resistorKOhm;
}NTCResistor_T_R_TypeDef;

typedef struct
{
	uint16_t adcVal;
	float ntcResistor;
	float temperature;
}ADCResTemp_TypeDef;

#define PULLUPRESISTOR1 10000.0   //10K电阻
#define PULLUPRESISTOR2 10000.0
#define PULLUPRESISTOR3 10000.0
#define PULLUPRESISTOR4 10000.0

extern const NTCResistor_T_R_TypeDef NTCResistor_50K3950_Table[];

extern ADCResTemp_TypeDef adcTemp[3];

uint16_t getADCValueAfterFilter(uint8_t channel);

void NTCResitor_Init(void);	//初始化
void NTCResistorTask(void);

#endif

