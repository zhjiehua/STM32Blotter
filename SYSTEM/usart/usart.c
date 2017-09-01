#include "sys.h"
#include "usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_adc.h"

#include "HardwareCommon.h"
#include "Beep/beep.h"
#include "Button/Button.h"
#include "PhotoelectricSensor/PhSensor.h"
#include "RelayMOS/RelayMOS.h"
#include "DCMotor/DCMotor.h"
#include "StepMotor/StepMotor.h"
#include "WDG/WDG.h"
#include "NTCResistor/adc.h"
#include "NTCResistor/NTCResistor.h"

#include "../HMI/cmd_process.h"
#include "../UILogic/pageCommon.h"

#include "CPrintf.h"

//#define SIZEOF(X)   sizeof(X)/sizeof(X[0])
	
////////////////////////////////////////////////////////////////////////////////// 	 
#include "FreeRTOS.h"
#include "task.h"
#include "../HMI/cmd_queue.h"
#include "../UILogic/pageCommon.h"
#include "../Logic/managerment.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;
	return ch;
}
#else 

/*使用microLib的方法*/
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
#endif
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  

//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
 	USART_DeInit(USART1);  //复位串口1
	
	//USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出  GPIO_Mode_Out_PP  GPIO_Mode_AF_PP
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入  GPIO_Mode_IN_FLOATING
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

	//GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	//GPIO_ResetBits(GPIOA, GPIO_Pin_10);
	
	//Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART1, ENABLE);                    //使能串口 

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 Res;
    uint32_t oldBasePri = portSET_INTERRUPT_MASK_FROM_ISR();
        
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据

		//USART_SendData(UART4, Res);
		
#if (LCDUART == 0)	//LCDUART		
		queue_push(Res);
#else            
		if((USART_RX_STA&0x8000)==0)//接收未完成
        {
            if(USART_RX_STA&0x4000)//接收到了0x0d
            {
                if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
                else USART_RX_STA|=0x8000;	//接收完成了 
            }
            else //还没收到0X0D
            {	
                if(Res==0x0d)USART_RX_STA|=0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
                    USART_RX_STA++;
                    if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
                }		 
            }
        }   		 
#endif
	} 
	 
    portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}

uint8_t pcWriteBuffer[500];

void UartTask(void)
{
    uint16_t i;
	uint8_t rtosInfoFlag = 0;
//	uint8_t buf[2048];
	
//	for(i=2047;i>=0;i++)
//	{
//		buf[i] = i;
//		vTaskDelay(1);
//	}
	
	while(1)
	{
		vTaskDelay(10);
		
        for(i=0;i<BUTTON_COUNT;i++)
        {
            if(button[i].flag)
            {
                char s[20];
                sprintf(s, "Button%d", i);
                cDebug("%s is pressed!\n", s);
                button[i].flag = 0;
            }
        }
        
        if(USART_RX_STA&0x8000)
		{					   
			//len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			
            switch(USART_RX_BUF[0])
            {
                case 'R':  //继电器
                    RELAY = !!(USART_RX_BUF[1]-0x30);
                break;
                case 'B':	//蜂鸣器
                    BEEP = !!(USART_RX_BUF[1]-0x30);
                break;
                case 'M':	//MOS管
                    //MOS1 = !!(USART_RX_BUF[1]-0x30);
                    MOS2 = !!(USART_RX_BUF[2]-0x30);
                break;
                case 'D':	//直流电机
                    pDCMotor[0].control = USART_RX_BUF[1]-0x30;
                    pDCMotor[0].temp = USART_RX_BUF[2]-0x30;
                break;
                case 'S':	//步进电机
//                    pStepMotor[0].control = USART_RX_BUF[1]-0x30;
//                    pStepMotor[0].temp = USART_RX_BUF[2]-0x30;
                break;
				case 'T':
//					setTemperature = (USART_RX_BUF[1]-0x30)*100 + (USART_RX_BUF[2]-0x30)*10 + (USART_RX_BUF[3]-0x30);
//					cDebug("setTemperature = %d\r\n", setTemperature);
//					PID_UpdateSetPoint(pPID, setTemperature);
				break;
				case 'P':
					switch(USART_RX_BUF[1])
					{
						case 'p':
							//pPID->Proportion = (float)(USART_RX_BUF[2])/10.0;
						break;
						case 'i':
							//pPID->Integral = (float)(USART_RX_BUF[2])/10.0;
						break;
						case 'd':
							//pPID->Derivative = (float)(USART_RX_BUF[2])/10.0;
						break;
						default:                
						break;
					}
				break;
				case 'p':
					rtosInfoFlag = 1;		
				break;
                default:
                    
                break;
            }
                
			USART_RX_STA=0;
		}

		if(rtosInfoFlag)
		{
			rtosInfoFlag = 0;
			printf("=================================================\r\n");
			printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
			vTaskList((char *)&pcWriteBuffer);
			printf("%s\r\n", pcWriteBuffer);
		
			printf("\r\n任务名       运行计数         使用率\r\n");
			vTaskGetRunTimeStats((char *)&pcWriteBuffer);
			printf("%s\r\n", pcWriteBuffer);
		}		
	}
}

#endif	

