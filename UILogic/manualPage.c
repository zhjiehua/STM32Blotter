#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "../Logic/motorManagerment.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/RelayMOS/RelayMOS.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void manualPageButtonProcess(uint16 control_id, uint8 state)
{
	switch(control_id)
	{
		case MANUAL_PUMP1_BUTTON:
		case MANUAL_PUMP2_BUTTON:
		case MANUAL_PUMP3_BUTTON:
		case MANUAL_PUMP4_BUTTON:
		case MANUAL_PUMP5_BUTTON:
		case MANUAL_PUMP6_BUTTON:
		case MANUAL_PUMP7_BUTTON:
		case MANUAL_PUMP8_BUTTON:
		{
			if(state)
				DCMotor_Run((DCMotorEnum_TypeDef)(control_id-1), pMotorMan->manualRawDir, pMotorMan->motorParaPumpSpeed);//DCMOTORSPEED
			else
				DCMotor_Stop((DCMotorEnum_TypeDef)(control_id-1));
		}
		break;
		case MANUAL_WASTEPUMP_BUTTON:
		{
			if(state)
				DCMotor_Run(PUMP_WASTE, pMotorMan->manualRawDir, pMotorMan->motorParaWastePumpSpeed);
			else
				DCMotor_Stop(PUMP_WASTE);
		}
		break;
		case MANUAL_VACUUMPUMP_BUTTON:
		{
			if(state)
				MOS2 = 1;
			else
				MOS2 = 0;
		}
		break;
		case MANUAL_PINCHVALVE_BUTTON:
		{
			if(state)
				RELAY = 1;
			else
				RELAY = 0;
		}
		break;
		case MANUAL_TURNTABLE_BUTTON:
		{
			if(state)
			{
				StepMotor_SetSpeed(STEPMOTOR_MAIN, 8);
				StepMotor_SetDir(STEPMOTOR_MAIN, pMotorMan->manualRawDir);
				StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);
			}
			else
			{
				StepMotor_SetCMD(STEPMOTOR_MAIN, DISABLE);
				vTaskDelay(500);
				StepMotor_Stop(STEPMOTOR_MAIN);
			}
		}
		break;
		case MANUAL_CWCCW_BUTTON:
			pMotorMan->manualRawDir = (Direction_TypeDef)state;
		break;
		
		//Èä¶¯±Ã
		case MANUAL_PUMPCWCCW_BUTTON:
			pMotorMan->manualPumpDir = (Direction_TypeDef)state;
		break;
		case MANUAL_PUMPSTART_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_PUMP & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_PUMP;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_PUMPSTART_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
					
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		
		//·ÏÒº±Ã
		case MANUAL_WASTEPUMPUP_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_WASTEPUMP_UP & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_WASTEPUMP_UP;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_WASTEPUMPUP_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		case MANUAL_WASTEPUMPDOWN_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_WASTEPUMP_DOWN & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_WASTEPUMP_DOWN;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_WASTEPUMPDOWN_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		
		//×ªÅÌ
		case MANUAL_TURNTABLEHOME_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_TURNTABLE_HOME & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_TURNTABLE_HOME;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLEHOME_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		case MANUAL_TURNTABLENEXT_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_TURNTABLE_NEXT & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_TURNTABLE_NEXT;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLENEXT_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		case MANUAL_TURNTABLELAST_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_TURNTABLE_LAST & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_TURNTABLE_LAST;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLELAST_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		case MANUAL_TURNTABLENEXT2_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_TURNTABLE_LAST & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_TURNTABLE_NEXT2;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLENEXT2_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
					
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		case MANUAL_TURNTABLELAST2_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_TURNTABLE_LAST2 & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_TURNTABLE_LAST2;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLELAST2_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
					
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		case MANUAL_TURNTABLEPOS_BUTTON:
			if(state)
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					//pProjectMan->projectStatus = (MANUAL_TURNTABLE_POS & PROJECT_RUNNING);
					pProjectMan->projectStatus = MANUAL_TURNTABLE_POS;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLEPOS_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
					
				xSemaphoreGive(pProjectMan->projectStatusSem);
			}
			else
				pProjectMan->projectStopFlag = 1;
		break;
		case MANUAL_PUMPSEL_BUTTON:
		break;
		case MANUAL_BACK_BUTTON:
		break;
		default:
			cDebug("manualPage BUTTON error!\n");
		break;
	}
}


void manualPageMenuProcess(uint16 control_id, uint8 item)
{
	switch(control_id)
	{
	case MANUAL_PUMPSEL_MENU:
		{
			pMotorMan->manualPumpNum = (DCMotorEnum_TypeDef)item;
		}
		break;
	default:
		cDebug("manualPage MENU error!\n");
		break;
	}
}

void manualPageEditProcess(uint16 control_id, uint8 *str)
{
	switch(control_id)
	{
		case MANUAL_PUMPAMOUNT_EDIT:
			pMotorMan->manualPumpAmount = StringToInt32(str);
		break;	
		case MANUAL_TURNTABLEPOS_EDIT:
			pMotorMan->manualTTPos = StringToInt32(str);
		break;			
		default:
			cDebug("manualPage EDIT error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
