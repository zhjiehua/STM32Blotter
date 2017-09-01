#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "LED/led.h"
#include "Key/key.h"
#include "Beep/beep.h"
#include "Button/Button.h"
#include "PhotoelectricSensor/PhSensor.h"
#include "RelayMOS/RelayMOS.h"
#include "DCMotor/DCMotor.h"
#include "StepMotor/StepMotor.h"
#include "WDG/WDG.h"
#include "NTCResistor/NTCResistor.h"
#include "Timer/timer.h"
#include "24CXX/24cxx.h"

#include "../Logic/managerment.h"
#include "CPrintf.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/************************************************

************************************************/
#define default_TASK_PRIORITY           ( tskIDLE_PRIORITY + 4 )
#define uart_TASK_PRIORITY           	( tskIDLE_PRIORITY + 3 )
#define project_TASK_PRIORITY           ( tskIDLE_PRIORITY + 2 )
#define ui_TASK_PRIORITY                ( tskIDLE_PRIORITY + 1 )

#define default_TASK_STACKDEP			( 50 )
#define uart_TASK_STACKDEP            	( 250 )
#define project_TASK_STACKDEP           ( 512+250 )
#define ui_TASK_STACKDEP                ( 1024+250 )

void UrtTask(void);
void ProjectTask(void);
void UITask(void);

//主函数    500 / portTICK_PERIOD_MS = 500ms
int main(void)
{
	/*
	优先级分组4
	外设	优先级	用途
	Uart1		1	RS232接口
	EXTI0		1	Button1
	EXTI1~3		2	Button2~3
	Timer2、5	2	StepMotor
	Uart4		3	串口屏接口
	Timer6		2	辅助打印任务信息
	
	SysTick		15
	PendSV		15
	SVC			0
	*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组2
	
	delay_init();
	uart_init(115200);	//初始化串口波特率为115200
	LED_Init();			//初始化LED
	AT24CXX_Init();
	while(AT24CXX_Check())//检测不到24c64
	{
		cDebug("24C64 Check Failed!\n");
		delay_ms(300);
		LED1=!LED1;//DS0闪烁
	}
	
	//bsp_SetTIMforInt(TIM6, 20000, 2, 0); //50us中断1次

	//initSoftwareTimer();
	
	pProjectMan->projectStatusSem = xSemaphoreCreateMutex();
	xSemaphoreGive(pProjectMan->projectStatusSem);
	pProjectMan->lcdUartSem = xSemaphoreCreateMutex();
	xSemaphoreGive(pProjectMan->lcdUartSem);
//	pProjectMan->projectEventGroup = xEventGroupCreate();	
	
	initUI();
	
	Beep_Init();
	Button_Init();
	PhSensor_Init();
	RelayMOS_Init();
	DCMotor_Init();
	StepMotor_Init();
	//NTCResitor_Init();

	IWDG_Init(4,625);    //与分频数为64,重载值为625,溢出时间为1s
    
	cDebug("hello STM32Blotter project!\r\n");
	
	/* Start the tasks defined within this file/specific to this demo. */
	xTaskCreate( (TaskFunction_t)UITask, "UITask", ui_TASK_STACKDEP, NULL, ui_TASK_PRIORITY, &(pProjectMan->uiTaskHandle) );
	//xTaskCreate( (TaskFunction_t)UartTask, "UartTask", uart_TASK_STACKDEP, NULL, uart_TASK_PRIORITY, NULL );
	xTaskCreate( (TaskFunction_t)ProjectTask, "ProjectTask", project_TASK_STACKDEP, NULL, project_TASK_PRIORITY, &(pProjectMan->projectTaskHandle) );//projectTaskHandle

	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1);
	
	return 0;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
	printf("Task: %s Stack is overflow!\r\n", pcTaskName);
}
