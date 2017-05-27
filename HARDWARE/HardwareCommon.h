#ifndef __HARDWARECOMMON_H
#define __HARDWARECOMMON_H	 
#include "sys.h"
#include "stm32f10x_exti.h"
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
#include "stm32f10x.h"

#define SIZEOF(X)   sizeof(X)/sizeof(X[0])

typedef FunctionalState Status;

typedef enum
{
	CW = 0,	//正转
	CCW,	//反转
}Direction_TypeDef;

typedef struct
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    GPIOMode_TypeDef GPIO_Mode;
    uint8_t NVIC_IRQChannelPreemptionPriority;
    uint8_t NVIC_IRQChannelSubPriority;
    EXTITrigger_TypeDef EXTI_Trigger;
    
    char *name;
    void (*CallBackFun)(void *para); //进入中断后的回调函数
}ExtiConfig_Typedef;
		 				    
#endif

