#ifdef __cplusplus	  
extern "C" {
#endif

#include "project.h"
#include "managerment.h"
#include "../User/CPrintf.h"
#include "stdio.h"

#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/RelayMOS/RelayMOS.h"
#include "../HARDWARE/Beep/beep.h"
#include "../Logic/motorManagerment.h"
	
#include "FreeRTOS.h"
#include "task.h"

void CalibraProgram(uint8_t *pStopFlag)
{
	uint8_t i;
	int32_t pidOut;
	float time;
	
	cDebug("========caliPage start to run the CalibraProgram!\n");
	
//	PID_UpdateActualPoint(&(pProjectMan->pumpCaliPID), pProjectMan->caliAmount);
//	pidOut = PID_Calc(&(pProjectMan->pumpCaliPID));
//	time = pProjectMan->caliAmount + pidOut;
	time = pProjectMan->pCaliPumpTime[pProjectMan->caliPumpSel];
	
	cDebug("time = %f\r\n", time);
	
	//��ʾ�Ƿ��ע�ܵ�
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS2PAGE_INDEX);//��ת����ʾ2ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"Whether fill the tube?");
	else
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"�Ƿ��ע�ܵ���");
	
	SetBuzzer(BEEPER_TIME_SHORT);
    xSemaphoreGive(pProjectMan->lcdUartSem);
	
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	if(pProjectMan->tipsButton == TIPS_CANCEL)
	{
		pProjectMan->tipsButton = TIPS_NONE;
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetScreen(CALIBRATIONPAGE_INDEX);//��ת��У׼ҳ��
		xSemaphoreGive(pProjectMan->lcdUartSem);
	}
	else
	{
        DCMotor_Run((DCMotorEnum_TypeDef)(pProjectMan->caliPumpSel), CW, pMotorMan->motorParaPumpSpeed);
		vTaskDelay(FILLINGTUBE_TIME);				
		DCMotor_Stop((DCMotorEnum_TypeDef)(pProjectMan->caliPumpSel));
	
		while(1)
		{
			//��ʾ�Ƿ������ע�ܵ�
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetScreen(TIPS2PAGE_INDEX);//��ת����ʾ2ҳ��
			if(pProjectMan->lang == 0)
				SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"Whether continue fill the tube?");
			else
				SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"�Ƿ������ע�ܵ���");
			
			SetBuzzer(BEEPER_TIME_SHORT);
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			while(pProjectMan->tipsButton == TIPS_NONE)
				vTaskDelay(10);
			
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetScreen(CALIBRATIONPAGE_INDEX);//��ת������ҳ��
			xSemaphoreGive(pProjectMan->lcdUartSem);
			if(pProjectMan->tipsButton == TIPS_CANCEL)
			{
				pProjectMan->tipsButton = TIPS_NONE;
				break;	
			}
			else
			{
				pProjectMan->tipsButton = TIPS_NONE;
				//������ע�ܵ�
                DCMotor_Run((DCMotorEnum_TypeDef)(pProjectMan->caliPumpSel), CW, pMotorMan->motorParaPumpSpeed);
                vTaskDelay(FILLINGTUBE_CON_TIME);				
                DCMotor_Stop((DCMotorEnum_TypeDef)(pProjectMan->caliPumpSel));
			}
		}		
	}

    vTaskDelay(1000);

	//��ʼУ׼
	for(i=0;i<40;i++)
	{
		DCMotor_Run((DCMotorEnum_TypeDef)(pProjectMan->caliPumpSel), CW, pMotorMan->motorParaPumpSpeed);
		//ֱ���������40�ε�1mL�����Թ�40mL		
		vTaskDelay(time);		
		DCMotor_Stop((DCMotorEnum_TypeDef)(pProjectMan->caliPumpSel));
		vTaskDelay(500);
    }
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetControlEnable(CALIBRATIONPAGE_INDEX, CALI_PUMPSELECT_BUTTON, 1);
	SetControlEnable(CALIBRATIONPAGE_INDEX, CALI_START_BUTTON, 1);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	pProjectMan->caliFlag = 1;
}

#ifdef __cplusplus
}
#endif
