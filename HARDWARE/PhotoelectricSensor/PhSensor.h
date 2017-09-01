#ifndef __PHSENSOR_H
#define __PHSENSOR_H	 

#include "stm32f10x_exti.h"
#include "HardwareCommon.h"
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

#define PHSENSOR_COUNT 16	//光电传感器数量		   

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
	uint32_t curStatus;   //当前状态
	uint32_t preStatus;	 //上一状态，在查询方式用
	uint32_t fallingEdge; //上升沿，在查询方式用
	uint32_t rasingEdge; //上升沿，在查询方式用

	uint32_t curStatusSingle;   //当前状态
	uint32_t preStatusSingle;	 //上一状态，在查询方式用
	uint32_t fallingEdgeSingle; //上升沿，在查询方式用
	uint32_t rasingEdgeSingle; //上升沿，在查询方式用
	
	CheckEdge_TypeDef checkEdge[PHSENSOR_COUNT];   //检测边沿类型
}PhSensor_TypeDef;

extern const PhSensorPin_TypeDef phSensorPin[];
extern PhSensor_TypeDef *pPhSensor;

void PhSensor_SingleScan(PhSensorEnum_TypeDef num);
uint8_t PhSensor_SingleCheck(PhSensorEnum_TypeDef num);
uint8_t PhSensor_SingleCheckEdge(PhSensorEnum_TypeDef num, CheckEdge_TypeDef edge);
void PhSensor_Scan(void);
uint8_t PhSensor_Check(PhSensorEnum_TypeDef num);
uint8_t PhSensor_CheckEdge(PhSensorEnum_TypeDef num, CheckEdge_TypeDef edge);

void PhSensor_Init(void);	//初始化
void PhSensorScanTask(void);
		 				    
#endif

