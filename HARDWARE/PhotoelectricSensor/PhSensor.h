#ifndef __PHSENSOR_H
#define __PHSENSOR_H	 

#include "stm32f10x_exti.h"
#include "HardwareCommon.h"
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

#define PHSENSOR_COUNT 16	//��紫��������		   

typedef struct
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
}PhSensorPin_TypeDef;

/**************************************************************************************/
#define PHSENSOR1_MASK (uint32_t)0x01
#define PHSENSOR2_MASK 0x02
#define PHSENSOR3_MASK 0x04
#define PHSENSOR4_MASK 0x08
#define PHSENSOR5_MASK 0x10
#define PHSENSOR6_MASK 0x20
#define PHSENSOR7_MASK 0x40
#define PHSENSOR8_MASK 0x80
#define PHSENSORALL_MASK 0xFF

#define PHSENSOR_POS  PHSENSOR1
#define PHSENSOR_HOME PHSENSOR2
#define PHSENSOR_UP   PHSENSOR3
#define PHSENSOR_DOWN PHSENSOR4

typedef enum
{
	PHSENSOR1 = 0,
	PHSENSOR2,
	PHSENSOR3,
	PHSENSOR4,
	PHSENSOR5,
	PHSENSOR6,
	PHSENSOR7,
	PHSENSOR8,
    PHSENSOR9,
    PHSENSOR10,
    PHSENSOR11,
    PHSENSOR12,
    PHSENSOR13,
    PHSENSOR14,
    PHSENSOR15,
    PHSENSOR16,
}PhSensorEnum_TypeDef;

typedef enum
{
	RASINGEDGE = 0,
	FALLINGEDGE,
}CheckEdge_TypeDef;

typedef struct
{
	uint32_t curStatus;   //��ǰ״̬
	uint32_t preStatus;	 //��һ״̬���ڲ�ѯ��ʽ��
	uint32_t fallingEdge; //�����أ��ڲ�ѯ��ʽ��
	uint32_t rasingEdge; //�����أ��ڲ�ѯ��ʽ��

	uint32_t curCount[PHSENSOR_COUNT];    //��ǰ�ڼ����ź�
	uint32_t desCount[PHSENSOR_COUNT]; 	 //����ڼ����źź�ֹͣ�����0��ʾ��ͣ
	CheckEdge_TypeDef checkEdge[PHSENSOR_COUNT];   //����������

//	void (*Scan)(void);
//	void (*SetCheckEdge)(SensorEnum_TypeDef sensorNum, CheckEdge_TypeDef edge);
//	void (*SetPos)(uint32_t pos);
//	uint8_t (*IsOnPos)(SensorEnum_TypeDef sensorNum);
//	uint8_t (*GetStatus)(SensorEnum_TypeDef sensorNum);
}PhSensor_TypeDef;

extern const PhSensorPin_TypeDef phSensorPin[];
extern PhSensor_TypeDef *pPhSensor;

void PhSensor_Scan(void);
void PhSensor_SetPos(PhSensorEnum_TypeDef num, uint32_t pos);
void PhSensor_SetCheckEdge(PhSensorEnum_TypeDef num, CheckEdge_TypeDef edge);
uint8_t PhSensor_IsOnPos(PhSensorEnum_TypeDef num);
uint8_t PhSensor_GetStatus(PhSensorEnum_TypeDef num);

void PhSensor_Init(void);	//��ʼ��
void PhSensorScanTask(void);
		 				    
#endif

