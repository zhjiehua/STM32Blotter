#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "../Logic/motorManagerment.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/RelayMOS/RelayMOS.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void motorParaPageButtonProcess(uint16 control_id, uint8 state)
{
	switch(control_id)
	{
		case MANUAL_BACK_BUTTON:
		break;
		case MOTORPARA_CREATETABLE_BUTTON:
			StepMotor_CreateSpeedTable();
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueInt32(MOTORPARAPAGE_INDEX, MOTORPARA_TTSPEEDLEVEL_EDIT, pMotorMan->motorParaSpeedLevel);
			xSemaphoreGive(pProjectMan->lcdUartSem);
		break;
		default:
			cDebug("manualPage BUTTON error!\n");
		break;
	}
}

void motorParaPageEditProcess(uint16 control_id, uint8 *str)
{
	switch(control_id)
	{
		case MOTORPARA_PUMPSPEED_EDIT:
			pMotorMan->motorParaPumpSpeed = StringToInt32(str);
			AT24CXX_Write(PARAPUMPSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaPumpSpeed), 1);
		break;
		case MOTORPARA_WASTEPUMPSPEED_EDIT:
			pMotorMan->motorParaWastePumpSpeed = StringToInt32(str);
			AT24CXX_Write(PARAWASTEPUMPSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaWastePumpSpeed), 1);
		break;
		case MOTORPARA_TTHOMESPEED_EDIT:
			pMotorMan->motorParaTTHomeSpeed = StringToInt32(str);
			AT24CXX_Write(PARATTHOMESPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTHomeSpeed), 1);
		break;
		case MOTORPARA_TTSTEP1SPEED_EDIT:
			pMotorMan->motorParaTTStep1Speed = StringToInt32(str);
			AT24CXX_Write(PARATTSTEP1SPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTStep1Speed), 1);
		break;
		case MOTORPARA_TTSTEP2SPEED_EDIT:
			pMotorMan->motorParaTTStep2Speed = StringToInt32(str);
			AT24CXX_Write(PARATTSTEP2SPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTStep2Speed), 1);
		break;
		case MOTORPARA_TTLOCATIONSPEED_EDIT:
			pMotorMan->motorParaTTLocationSpeed = StringToInt32(str);
			AT24CXX_Write(PARATTLOCATIONSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTLocationSpeed), 1);
		break;
		case MOTORPARA_TTLOWSPEED_EDIT:
			pMotorMan->motorParaLowSpeed = StringToInt32(str);
			AT24CXX_Write(PARALOWSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaLowSpeed), 1);
		break;
		case MOTORPARA_TTMIDDLESPEED_EDIT:
			pMotorMan->motorParaMiddleSpeed = StringToInt32(str);
			AT24CXX_Write(PARAMIDDLESPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaMiddleSpeed), 1);
		break;
		case MOTORPARA_TTFASTSPEED_EDIT:
			pMotorMan->motorParaFastSpeed = StringToInt32(str);
			AT24CXX_Write(PARAFASTSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaFastSpeed), 1);
		break;
		case MOTORPARA_TTSTARTFREQ_EDIT:
			pMotorMan->motorParaStartFreq = StringToInt32(str);
			AT24CXX_Write(PARASTARTFREQ_BASEADDR, (uint8_t*)(&pMotorMan->motorParaStartFreq), 4);
		break;
		case MOTORPARA_TTENDFREQ_EDIT:
			pMotorMan->motorParaEndFreq = StringToInt32(str);
			AT24CXX_Write(PARAENDFREQ_BASEADDR, (uint8_t*)(&pMotorMan->motorParaEndFreq), 4);
		break;
		case MOTORPARA_TTACCERALATE_EDIT:
			pMotorMan->motorParaAccSpeed = StringToInt32(str);
			AT24CXX_Write(PARAACCSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaAccSpeed), 4);
		break;
		case MOTORPARA_TTSTEPTIME_EDIT:
			pMotorMan->motorParaStepTime = StringToInt32(str);
			AT24CXX_Write(PARASTEPTIME_BASEADDR, (uint8_t*)(&pMotorMan->motorParaStepTime), 4);
		break;
		case MOTORPARA_WASTEPUMPCALIB_EDIT:
			pMotorMan->motorParaWastePumpCalib = StringToInt32(str);
			AT24CXX_Write(WASTEPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaWastePumpCalib), 2);
		break;
		case MOTORPARA_CWPUMPCALIB_EDIT:
			pMotorMan->motorParaCWPumpCalib = StringToInt32(str);
			AT24CXX_Write(CWPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaCWPumpCalib), 2);
		break;
		case MOTORPARA_CCWPUMPCALIB_EDIT:
			pMotorMan->motorParaCCWPumpCalib = StringToInt32(str);
			AT24CXX_Write(CCWPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaCCWPumpCalib), 2);
		break;
		default:
			cDebug("motorParaPage EDIT error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
