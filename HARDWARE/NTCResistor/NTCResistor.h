#ifndef __NTCRESISTOR_H
#define __NTCRESISTOR_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
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

#define PULLUPRESISTOR1 10000.0   //10K����
#define PULLUPRESISTOR2 10000.0
#define PULLUPRESISTOR3 10000.0
#define PULLUPRESISTOR4 10000.0

extern const NTCResistor_T_R_TypeDef NTCResistor_50K3950_Table[];

extern ADCResTemp_TypeDef adcTemp[3];

uint16_t getADCValueAfterFilter(uint8_t channel);

void NTCResitor_Init(void);	//��ʼ��
void NTCResistorTask(void);

#endif

