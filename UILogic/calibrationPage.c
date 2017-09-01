#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HardwareCommon.h"

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void caliPageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
		case CALI_START_BUTTON:
		{
			xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
			
			//if(state)
			{
				if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetControlEnable(CALIBRATIONPAGE_INDEX, CALI_PUMPSELECT_BUTTON, 0);
					SetControlEnable(CALIBRATIONPAGE_INDEX, CALI_START_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
					
					pProjectMan->projectStopFlag = 0;
					pProjectMan->caliFlag = 0;
					pProjectMan->projectStatus = PROJECT_CALIBRATION;
					pProjectMan->projectStatus |= PROJECT_RUNNING;
				}
				else
				{
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(CALIBRATIONPAGE_INDEX, CALI_START_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				}
			}
//			else
//			{
//				pProjectMan->projectStopFlag = 1;
//			}

			xSemaphoreGive(pProjectMan->projectStatusSem);
			
			cDebug("========caliPage start to run the CALIBRATION program!\n");
		}
		break;
		case CALI_OK_BUTTON:
		{
			//保存校准参数
			uint16_t addrOffset;
			if(pProjectMan->caliFlag == 1)
			{
				pProjectMan->caliFlag = 0;
				pProjectMan->pCaliPumpPara[pProjectMan->caliPumpSel] = pProjectMan->caliAmount;
				pProjectMan->pCaliPumpTime[pProjectMan->caliPumpSel] *= 40.0/pProjectMan->pCaliPumpPara[pProjectMan->caliPumpSel];
				addrOffset = pProjectMan->caliPumpSel*4;
				AT24CXX_Write(CALIBPUMPPARA_BASEADDR+addrOffset, (uint8_t*)&(pProjectMan->pCaliPumpPara[pProjectMan->caliPumpSel]), 4);  //保存参数
				AT24CXX_Write(CALIBPUMPTIME_BASEADDR+addrOffset, (uint8_t*)&(pProjectMan->pCaliPumpTime[pProjectMan->caliPumpSel]), 4); 
				cDebug("========caliPage Save the calibration data!\n");
			}
		}
		break;
		case CALI_PUMPSELECT_BUTTON:
		break;
		case CALI_BACK_BUTTON:
		break;
		default:
			cDebug("caliPage BUTTON error!\n");
		break;
	}
}

void caliPageEditProcess(uint16 control_id, uint8 *str)
{
	//uint16_t temp;
	switch(control_id)
	{
		case CALI_ACTUALAMOUNT_EDIT:
			//temp = StringToInt32(str);
			pProjectMan->caliAmount = StringToFloat(str);//StringToFloat
			cDebug("pProjectMan->caliAmount = %f\n", pProjectMan->caliAmount);
		break;

		default:
			cDebug("caliPage EDIT error!\n");
		break;
	}
}

void caliPageMenuProcess(uint16 control_id, uint8 item)
{
	switch(control_id)
	{
		case CALI_PUMPSELECT_MENU:
		{
			pProjectMan->caliPumpSel = item;
			pProjectMan->caliAmount = pProjectMan->pCaliPumpPara[pProjectMan->caliPumpSel];
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueFloat(CALIBRATIONPAGE_INDEX, CALI_ACTUALAMOUNT_EDIT, pProjectMan->caliAmount);
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
		break;
		default:
		cDebug("caliPage MENU error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
