#ifndef __BUTTON_H
#define __BUTTON_H	 
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

#define BUTTON_COUNT 4	//	   

typedef struct
{
    uint8_t flag; //标志是否进入过中断
    void (*CallBackFun)(void *para); //进入中断后的回调函数
}Button_TypeDef;

extern Button_TypeDef button[BUTTON_COUNT];

void Button_Init(void);	//初始化
		 				    
#endif

