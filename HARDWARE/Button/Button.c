#include "Button.h"
#include "usart.h"
#include "stm32f10x_exti.h"
#include "PhotoelectricSensor/PhSensor.h"
#include "../Logic/managerment.h"
#include "../DCMotor/DCMotor.h"
#include "../StepMotor/StepMotor.h"
#include "../RelayMOS/RelayMOS.h"
#include "../WDG/WDG.h"
#include "../HARDWARE/24CXX/24cxx.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

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

Button_TypeDef button[BUTTON_COUNT];

//初始化PB8为输出口.并使能这个口的时钟		    
//蜂鸣器初始化
void Button_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);	 //使能GPIOB端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;//BEEP-->PB.8 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
    GPIO_Init(GPIOE, &GPIO_InitStructure);	 //根据参数初始化GPIOB.8
    
    // NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;		//子优先级3
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存�
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器  
    
    //外部中断配置
    EXTI_ClearITPendingBit(EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3);
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
}

void EXTI0_IRQHandler(void)
{
	uint32_t oldBasePri = portSET_INTERRUPT_MASK_FROM_ISR();
	
    button[0].flag = 1;
	
    EXTI_ClearITPendingBit(EXTI_Line0);
	portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}

void EXTI1_IRQHandler(void)
{
    button[1].flag = 1;
	
    EXTI_ClearITPendingBit(EXTI_Line1);
}

void EXTI2_IRQHandler(void)
{
    button[2].flag = 1;
    
    EXTI_ClearITPendingBit(EXTI_Line2);
}

void EXTI3_IRQHandler(void)
{
    button[3].flag = 1;
    
    EXTI_ClearITPendingBit(EXTI_Line3);
}
