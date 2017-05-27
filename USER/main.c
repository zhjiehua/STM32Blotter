#include "sys.h"
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

#include "../Logic/managerment.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "./main.h"

/************************************************

************************************************/
#define StepMotorTest_TASK_PRIORITY  ( tskIDLE_PRIORITY + 6 )
#define DCMotorTest_TASK_PRIORITY    ( tskIDLE_PRIORITY + 5 )
#define led1_TASK_PRIORITY           ( tskIDLE_PRIORITY + 4 )
#define led2_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define uart_TASK_PRIORITY           ( tskIDLE_PRIORITY + 2 )
#define phSensorScan_TASK_PRIORITY   ( tskIDLE_PRIORITY + 1 )

void UITask(void)
{
    qsize size;
    
    while(1)
    {
        //处理UI数据
		size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE); //从缓冲区获取一条指令
		if(size > 0)//接收到指令
		{
			//cDebug("cmd_buffer size = %d", size);
			//for(i=0;i<size;i++)
			//	cDebug("%#x", cmd_buffer[i]);

			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//处理指令
		}
        
        IWDG_Feed();//如果WK_UP按下,则喂狗
        
        vTaskDelay(10);
    }
}

//主函数
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	uart_init(115200);	//初始化串口波特率为115200
	LED_Init();			//初始化LED
    Beep_Init();
    Button_Init();
    PhSensor_Init();
    RelayMOS_Init();
    DCMotor_Init();
    StepMotor_Init();
    
    //initUI();
    
    IWDG_Init(4,625);    //与分频数为64,重载值为625,溢出时间为1s
    
	printf("hello TCN-I project!\r\n");
	
	/* Start the tasks defined within this file/specific to this demo. */
    xTaskCreate( (TaskFunction_t)LED1Task, "LED1Task", 10, NULL, led1_TASK_PRIORITY, NULL );
	xTaskCreate( (TaskFunction_t)LED2Task, "LED2Task", 10, NULL, led2_TASK_PRIORITY, NULL );	
	xTaskCreate( (TaskFunction_t)UartTask, "UartTask", 50, NULL, uart_TASK_PRIORITY, NULL );
	xTaskCreate( (TaskFunction_t)PhSensorScanTask, "PhSensorScanTask", 50, NULL, phSensorScan_TASK_PRIORITY, NULL );
    xTaskCreate( (TaskFunction_t)DCMotorTestTask, "DCMotorTestTask", 50, NULL, DCMotorTest_TASK_PRIORITY, NULL );
    xTaskCreate( (TaskFunction_t)StepMotorTestTask, "StepMotorTestTask", 100, NULL, StepMotorTest_TASK_PRIORITY, NULL );
    
    //xTaskCreate( (TaskFunction_t)UITask, "UITask", ui_TASK_STACKDEP, NULL, ui_TASK_PRIORITY, NULL );
    
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	return 0;
}




	


