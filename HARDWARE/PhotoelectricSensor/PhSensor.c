#include "PhSensor.h"
#include "usart.h"

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
    {GPIOB, GPIO_Pin_3},//"EXIN10"
    {GPIOB, GPIO_Pin_4},//"EXIN11"
    {GPIOB, GPIO_Pin_5},//"EXIN12"
    {GPIOB, GPIO_Pin_6},//"EXIN13"
    {GPIOB, GPIO_Pin_7},//"EXIN14"
    {GPIOB, GPIO_Pin_8},//"EXIN15"
    {GPIOB, GPIO_Pin_9},//"EXIN16"
};

//传感器状态
static PhSensor_TypeDef phSensor;
PhSensor_TypeDef *pPhSensor = &phSensor;

void PhSensor_Scan(void)
{
    uint8_t i;
	phSensor.preStatus = phSensor.curStatus;
	
    for(i=0;i<SIZEOF(phSensorPin);i++)
    {
        if(GPIO_ReadInputDataBit(phSensorPin[i].GPIOx, phSensorPin[i].GPIO_Pin))
            phSensor.curStatus &= ~(PHSENSOR1_MASK << i);
        else
            phSensor.curStatus |= PHSENSOR1_MASK << i;
    }
    
	phSensor.fallingEdge = (phSensor.curStatus ^ phSensor.preStatus) & phSensor.preStatus;
	phSensor.rasingEdge = (phSensor.curStatus ^ phSensor.preStatus) & phSensor.curStatus;
}

void PhSensor_SetPos(PhSensorEnum_TypeDef num, uint32_t pos)
{
	phSensor.curCount[num] = 0;
	phSensor.desCount[num] = pos;	
}

void PhSensor_SetCheckEdge(PhSensorEnum_TypeDef num, CheckEdge_TypeDef edge)
{
	phSensor.checkEdge[num] = edge;	
}

uint8_t PhSensor_IsOnPos(PhSensorEnum_TypeDef num)
{
	uint32_t edge;
	PhSensor_Scan();  //扫描

	if(phSensor.checkEdge[num] == FALLINGEDGE) //检测下降沿
		edge = phSensor.fallingEdge;
	else  //检测上升沿
		edge = phSensor.rasingEdge;
			
	if(edge & ((uint32_t)0x01<<num))
	{
		if(++phSensor.curCount[num] >= phSensor.desCount[num])
		{
			phSensor.curCount[num] = 0;
			phSensor.desCount[num] = 0;
			return 1;
		}
	}
	return 0;	 		
}

uint8_t PhSensor_GetStatus(PhSensorEnum_TypeDef num)
{
	uint32_t edge;
    uint8_t ret;
	//PhSensor_Scan();  //扫描

	if(phSensor.checkEdge[num] == FALLINGEDGE) //检测下降沿
		edge = phSensor.fallingEdge;
	else  //检测上升沿
		edge = phSensor.rasingEdge;
			
	if(edge & ((uint32_t)0x01<<num))
		ret = 1;
	else
		ret = 0;
	return ret;	 		
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
        phSensor.curCount[i] = 0;
        phSensor.desCount[i] = 0;
    }
    
    phSensor.preStatus = 0x00;
	phSensor.curStatus = 0x00;
	phSensor.fallingEdge = 0x00;
    phSensor.rasingEdge = 0x00;
}

void PhSensorScanTask(void)
{
    uint32_t cnt = 0;
    uint8_t i;
	while(1)
	{
		vTaskDelay(10);
        
        cnt++;
        if(cnt == 1000)
        {   
            cnt = 0;
            printf("hello PhSensorScanTask!\r\n");
        }

        PhSensor_Scan();
        
        for(i=0;i<SIZEOF(phSensorPin);i++)
        {
            if(phSensor.fallingEdge&(PHSENSOR1_MASK<<i))
                printf("phSensor%d is pressed!\n", i);
            
            PhSensor_SetCheckEdge((PhSensorEnum_TypeDef)i, FALLINGEDGE);
            if(PhSensor_GetStatus((PhSensorEnum_TypeDef)i))
                printf("--phSensor%d is pressed!\n", i);
        }
	}
}
