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
        //����UI����
		size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE); //�ӻ�������ȡһ��ָ��
		if(size > 0)//���յ�ָ��
		{
			//cDebug("cmd_buffer size = %d", size);
			//for(i=0;i<size;i++)
			//	cDebug("%#x", cmd_buffer[i]);

			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//����ָ��
		}
        
        IWDG_Feed();//���WK_UP����,��ι��
        
        vTaskDelay(10);
    }
}

//������
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	uart_init(115200);	//��ʼ�����ڲ�����Ϊ115200
	LED_Init();			//��ʼ��LED
    Beep_Init();
    Button_Init();
    PhSensor_Init();
    RelayMOS_Init();
    DCMotor_Init();
    StepMotor_Init();
    
    //initUI();
    
    IWDG_Init(4,625);    //���Ƶ��Ϊ64,����ֵΪ625,���ʱ��Ϊ1s
    
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




	


