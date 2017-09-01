#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../Logic/motorManagerment.h"
#include "../HardwareCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

void infoPageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
		case INFO_LANG_BUTTON:
		break;
		case INFO_MANUAL_EDIT:
		break;
		case INFO_MOTORPARA_EDIT:
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_PUMPSPEED_EDIT, pMotorMan->motorParaPumpSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_WASTEPUMPSPEED_EDIT, pMotorMan->motorParaWastePumpSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTHOMESPEED_EDIT, pMotorMan->motorParaTTHomeSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTSTEP1SPEED_EDIT, pMotorMan->motorParaTTStep1Speed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTSTEP2SPEED_EDIT, pMotorMan->motorParaTTStep2Speed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTLOCATIONSPEED_EDIT, pMotorMan->motorParaTTLocationSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTLOWSPEED_EDIT, pMotorMan->motorParaLowSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTMIDDLESPEED_EDIT, pMotorMan->motorParaMiddleSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTFASTSPEED_EDIT, pMotorMan->motorParaFastSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTSTARTFREQ_EDIT, pMotorMan->motorParaStartFreq);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTENDFREQ_EDIT, pMotorMan->motorParaEndFreq);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTACCERALATE_EDIT, pMotorMan->motorParaAccSpeed);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTSTEPTIME_EDIT, pMotorMan->motorParaStepTime);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTSPEEDLEVEL_EDIT, pMotorMan->motorParaSpeedLevel);
		
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_WASTEPUMPCALIB_EDIT, pMotorMan->motorParaWastePumpCalib);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_CWPUMPCALIB_EDIT, pMotorMan->motorParaCWPumpCalib);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_CCWPUMPCALIB_EDIT, pMotorMan->motorParaCCWPumpCalib);
			xSemaphoreGive(pProjectMan->lcdUartSem);
		break;
		case INFO_BACK_BUTTON:
		break;
		default:
			cDebug("infoPage BUTTON error!\n");
		break;
	}
}

void infoPageMenuProcess(uint16 control_id, uint8 item)
{
	switch(control_id)
	{
	case INFO_LANG_MENU:
		{
			pProjectMan->lang = (Language_TypeDef)item;
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			if(item == 0)
			{
				SetLanguage(0, 1);
				SetTextValue(INFORMATIONPAGE_INDEX, INFO_LANG_EDIT, (uint8_t*)langMenuText[pProjectMan->lang]);
			}
			else
			{
				SetLanguage(1, 0);
				SetTextValue(INFORMATIONPAGE_INDEX, INFO_LANG_EDIT, (uint8_t*)langMenuTextCh[pProjectMan->lang]);
			}
			xSemaphoreGive(pProjectMan->lcdUartSem);
			AT24CXX_Write(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), 1);
		}
		break;
	default:
		cDebug("infoPage MENU error!\n");
		break;
	}
}

void infoPageEditProcess(uint16 control_id, uint8 *str)
{
	switch(control_id)
	{				   
		default:
			cDebug("infoPage EDIT error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
