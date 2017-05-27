#include "led.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PC,PA�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //LED1-->PC.9 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOC.9
 GPIO_SetBits(GPIOC,GPIO_Pin_9);						 //PC.9 �����

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	    		 //LED2-->PA.8 �˿�����, �������
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 GPIO_SetBits(GPIOA,GPIO_Pin_8); 						 //PA.8 ����� 
}
 
void LED1Task(void)
{
	while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_9);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
		vTaskDelay(1000);
	}
}

void LED2Task(void)
{
	while(1)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
		vTaskDelay(500);
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
		vTaskDelay(500);
	}
}
