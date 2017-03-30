#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/************************************************

************************************************/

#define led1_TASK_PRIORITY           ( tskIDLE_PRIORITY + 1 )
#define led2_TASK_PRIORITY           ( tskIDLE_PRIORITY + 2 )
#define uart_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )

void vLED1()
{
	while(1)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		vTaskDelay(1000);
	}
}

void vLED2()
{
	while(1)
	{
		GPIO_SetBits(GPIOE, GPIO_Pin_5);
		vTaskDelay(500);
		GPIO_ResetBits(GPIOE, GPIO_Pin_5);
		vTaskDelay(500);
	}
}

//主函数
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	uart_init(115200);	//初始化串口波特率为115200
	LED_Init();			//初始化LED

	/* Start the tasks defined within this file/specific to this demo. */
    xTaskCreate( (TaskFunction_t)vLED1, "vLED1", 100, NULL, led1_TASK_PRIORITY, NULL );
	xTaskCreate( (TaskFunction_t)vLED2, "vLED2", 100, NULL, led2_TASK_PRIORITY, NULL );	
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	return 0;
}




	


