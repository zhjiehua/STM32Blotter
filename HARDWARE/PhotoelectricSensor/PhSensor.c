#include "PhSensor.h"
#include "usart.h"
#include "../UILogic/pageCommon.h"
#include "../DCMotor/DCMotor.h"
#include "../StepMotor/StepMotor.h"
#include "CPrintf.h"

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

const PhSensorPin_TypeDef phSensorPin[] = 
{
    {GPIOC, GPIO_Pin_12},//"EXIN1"
    {GPIOD, GPIO_Pin_0},//"EXIN2"
    {GPIOD, GPIO_Pin_1},//"EXIN3"
    {GPIOD, GPIO_Pin_2},//"EXIN4"
    {GPIOD, GPIO_Pin_3},//"EXIN5"
    {GPIOD, GPIO_Pin_4},//"EXIN6"
    {GPIOD, GPIO_Pin_5},//"EXIN7"
    {GPIOD, GPIO_Pin_6},//"EXIN8"
    {GPIOD, GPIO_Pin_7},//"EXIN9"
    {GPIOB, GPIO_Pin_3},//"EXIN10"  需要重定义
    {GPIOB, GPIO_Pin_4},//"EXIN11"
    {GPIOB, GPIO_Pin_5},//"EXIN12"
    {GPIOB, GPIO_Pin_6},//"EXIN13"
    {GPIOB, GPIO_Pin_7},//"EXIN14"
    {GPIOB, GPIO_Pin_8},//"EXIN15"
    {GPIOB, GPIO_Pin_9},//"EXIN16"
};

typedef struct
{
	uint8_t sensorIndex;		//传感器接口
	uint8_t uartLCDInputIndex;	//LCD输入点状态控件序号
}SensorMap_TypeDef;

const SensorMap_TypeDef sensorMap[16] = 
{
	{1, 5},
	{2, 6},
	{3, 13},
	{4, 14},
	{5, 11},
	{6, 12},
	{7, 255},
	{8, 10},
	{9, 9},
	{10, 255},
	{11, 7},
	{12, 8},
	{13, 3},
	{14, 4},
	{15, 1},
	{16, 2},
};

//传感器状态
static PhSensor_TypeDef phSensor;
PhSensor_TypeDef *pPhSensor = &phSensor;

uint32_t posTimerCounter = 0;

void PhSensor_SingleScan(PhSensorEnum_TypeDef num)
{
	uint8_t preFlag = 0;
	uint8_t curFlag = 0;
	
	if(phSensor.curStatusSingle & (PHSENSOR1_MASK << num))
	{
		phSensor.preStatusSingle |= (PHSENSOR1_MASK << num);
		preFlag = 1;
	}
	else
		phSensor.preStatusSingle &= ~(PHSENSOR1_MASK << num);
	
	if(GPIO_ReadInputDataBit(phSensorPin[num].GPIOx, phSensorPin[num].GPIO_Pin))
	{
        phSensor.curStatusSingle |= (PHSENSOR1_MASK << num);
		curFlag = 1;
	}
	else
		phSensor.curStatusSingle &= ~(PHSENSOR1_MASK << num);
	
	if((curFlag ^ preFlag) & preFlag)
		phSensor.fallingEdgeSingle |= ((uint32_t)(0x0001) << num);
	else
		phSensor.fallingEdgeSingle &= ~((uint32_t)(0x0001) << num);
	
	if((curFlag ^ preFlag) & curFlag)
		phSensor.rasingEdgeSingle |= ((uint32_t)(0x0001) << num);
	else
		phSensor.rasingEdgeSingle &= ~((uint32_t)(0x0001) << num);
}

uint8_t PhSensor_SingleCheck(PhSensorEnum_TypeDef num)
{
	return (!!(phSensor.curStatusSingle & (PHSENSOR1_MASK << num)));
}

uint8_t PhSensor_SingleCheckEdge(PhSensorEnum_TypeDef num, CheckEdge_TypeDef edge)
{
	if(edge == FALLINGEDGE)
		return (!!(phSensor.fallingEdgeSingle & (PHSENSOR1_MASK << num)));
	else
		return (!!(phSensor.rasingEdgeSingle & (PHSENSOR1_MASK << num)));
}

void PhSensor_Scan(void)
{
    uint8_t i;
	phSensor.preStatus = phSensor.curStatus;
	
    for(i=0;i<SIZEOF(phSensorPin);i++)
    {
        if(GPIO_ReadInputDataBit(phSensorPin[i].GPIOx, phSensorPin[i].GPIO_Pin))
            phSensor.curStatus |= PHSENSOR1_MASK << i;
        else
			phSensor.curStatus &= ~(PHSENSOR1_MASK << i);
    }
    
	phSensor.fallingEdge = (phSensor.curStatus ^ phSensor.preStatus) & phSensor.preStatus;
	phSensor.rasingEdge = (phSensor.curStatus ^ phSensor.preStatus) & phSensor.curStatus;
}

uint8_t PhSensor_CheckEdge(PhSensorEnum_TypeDef num, CheckEdge_TypeDef edge)
{
	if(edge == FALLINGEDGE)
		return (!!(phSensor.fallingEdge & (PHSENSOR1_MASK << num)));
	else
		return (!!(phSensor.rasingEdge & (PHSENSOR1_MASK << num)));
}

uint8_t PhSensor_Check(PhSensorEnum_TypeDef num)
{
	return (!!(phSensor.curStatus & (PHSENSOR1_MASK << num)));
}
	
//定时器7中断服务程序
void TIM7_IRQHandler(void)   //TIM3中断
{
	if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		phSensor.posTimerCounter++;
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);  //清除TIMx更新中断标志 
	}
}

//外部中断0服务程序 
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetFlagStatus(EXTI_Line12))
	{
		if((pStepMotor[0].status & 0x01) && (pStepMotor[0].speedStatus == SPEED_NONE))//如果转盘正在转且加速完成进入了匀速阶段
		{
			if(PCin(12) == 0) //下降沿
			{
				phSensor.posTimerCounter = 0;
				TIM_Cmd(TIM7, ENABLE);
				//cDebug("start tim7\r\n");
			}
			else	//上升沿
			{
				//TIM_SetCounter(TIM7, 0);
				TIM_Cmd(TIM7, DISABLE);
				
				if(phSensor.counterBufferStatFlag == 0)
				{
					if(phSensor.counterBufferIndex < 6)
					{
						phSensor.counterBufferIndex++;
						phSensor.posTimerCounterAverage += phSensor.posTimerCounter;
					}
					else
					{
						phSensor.counterBufferStatFlag = 1;
						phSensor.counterBufferIndex = 0;
						phSensor.posTimerCounterAverage /= 6;
						phSensor.posTimerCounterAverage *= 1.5;
					}
				}
				else
				{
					if(phSensor.posTimerCounter > phSensor.posTimerCounterAverage && phSensor.posTimerCounter < phSensor.posTimerCounterAverage*2)
					{
						phSensor.homeFlag = 1;
						phSensor.counterBufferStatFlag = 0;
						phSensor.posTimerCounterAverage = 0;
						cDebug("homeFlag\r\n");
					}
				}
				//cDebug("posTimerCounter = %d\r\n", phSensor.posTimerCounter);
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line12); //清除LINE0上的中断标志位
	}
}

void PhSensor_PosIntInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟
	
	//外部中断配置
    EXTI_ClearITPendingBit(EXTI_Line12);
    
    //GPIOE.2 中断线以及中断初始化配置   下降沿触发
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12);
    
  	EXTI_InitStructure.EXTI_Line = EXTI_Line12;	//KEY2
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
	
	//位置传感器中断
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	TIM_Cmd(TIM7, DISABLE);
	
	TIM_TimeBaseStructure.TIM_Period = 3600; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	72000000/3600 = 20KHz  50us
	TIM_TimeBaseStructure.TIM_Prescaler = 0; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ARRPreloadConfig(TIM7, ENABLE);
	
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//初始化PB8为输出口.并使能这个口的时钟		    
//蜂鸣器初始化
void PhSensor_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    uint8_t i;
    
    for(i=0;i<SIZEOF(phSensorPin);i++)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA << ((phSensorPin[i].GPIOx-GPIOA)/(GPIOB-GPIOA)), ENABLE);
        
        GPIO_InitStructure.GPIO_Pin = phSensorPin[i].GPIO_Pin;	//EXINx
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	//上拉输入
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(phSensorPin[i].GPIOx, &GPIO_InitStructure);

        phSensor.checkEdge[i] = FALLINGEDGE;
    }
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	
	PhSensor_PosIntInit();
	
    phSensor.preStatus = 0x00;
	phSensor.curStatus = 0x00;
	phSensor.fallingEdge = 0x00;
    phSensor.rasingEdge = 0x00;
	
	phSensor.preStatusSingle = 0x00;
	phSensor.curStatusSingle = 0x00;
	phSensor.fallingEdgeSingle = 0x00;
    phSensor.rasingEdgeSingle = 0x00;
}

void PhSensorScanTask(void)
{
    uint8_t i;

	PhSensor_Scan();
	
	for(i=0;i<4;i++)
	{
//		if(PhSensor_CheckEdge((PhSensorEnum_TypeDef)i, FALLINGEDGE))
//			printf("--phSensor%d is pressed!\n", i);
		
//		PhSensor_SingleScan((PhSensorEnum_TypeDef)i);
//		if(PhSensor_SingleCheckEdge((PhSensorEnum_TypeDef)i, FALLINGEDGE))
//			printf("--phSensor%d is pressed!\n", i);

//		SetButtonValue(INPUTPAGE_INDEX, sensorMap[i].uartLCDInputIndex, 
//			(sensorMap[i].uartLCDInputIndex == 6) ? (PhSensor_Check((PhSensorEnum_TypeDef)i)) : (!PhSensor_Check((PhSensorEnum_TypeDef)i)));
	}
}
