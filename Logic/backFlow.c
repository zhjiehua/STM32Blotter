#ifdef __cplusplus	  
extern "C" {
#endif

#include "project.h"
#include "managerment.h"
#include "CPrintf.h"
#include "stdio.h"

#include "../HARDWARE/RelayMOS/RelayMOS.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../Logic/motorManagerment.h"
	
#include "FreeRTOS.h"
#include "task.h"

 //ÇåÏ´³ÌÐò
void BackFlowProgram(uint8_t *pStopFlag)
{
	uint8_t i;

	for(i=0;i<8;i++)
	{
		if(pProjectMan->backflowPumpSel & (PUMP1_MASK<<i))
		{
			DCMotor_Run((DCMotorEnum_TypeDef)(i), CCW, pMotorMan->motorParaPumpSpeed);
			vTaskDelay(4000);
			DCMotor_Stop((DCMotorEnum_TypeDef)(i));
			vTaskDelay(500);
		}
	}
}

#ifdef __cplusplus
}
#endif
