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

#include "FreeRTOS.h"
#include "task.h"
    
void CalibrationTask(void)
{
	uint8_t i;
	uint8_t pumpSel = pProjectMan->caliPumpSel;
	float time = pProjectMan->pCaliPumpPara[pumpSel];
	//cDebug("========caliPage start to run the PUMP program!\n");

//	cDebug("pProjectMan->caliPumpSel = %d\n", (uint16_t)pProjectMan->caliPumpSel);
//	pDCMotor->SetCMD(pProjectMan->caliPumpSel, ENABLE);
//	os_wait(K_TMO, 1000, 0);
//	pDCMotor->SetCMD(pProjectMan->caliPumpSel, DISABLE);


	//提示是否灌注管道
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(TIPS2PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"Whether fill the tube?");
	else
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"是否灌注管道？");

	beepAlarm(1);
    
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	if(pProjectMan->tipsButton == TIPS_CANCEL)
	{
		pProjectMan->tipsButton = TIPS_NONE;
		SetScreen(CALIBRATIONPAGE_INDEX);//跳转到校准页面
	}
	else
	{
        DCMotor_Run((DCMotorEnum_TypeDef)pumpSel, CW, 10);
		vTaskDelay(380);				
		DCMotor_Stop((DCMotorEnum_TypeDef)pumpSel);
	
		while(1)
		{
			//提示是否继续灌注管道
			SetScreen(TIPS2PAGE_INDEX);//跳转到提示2页面
			if(pProjectMan->lang == 0)
				SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"Whether continue fill the tube?");
			else
				SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uchar*)"是否继续灌注管道？");
			
			beepAlarm(1);
			while(pProjectMan->tipsButton == TIPS_NONE);
				
			SetScreen(CALIBRATIONPAGE_INDEX);//跳转到运行页面
			if(pProjectMan->tipsButton == TIPS_CANCEL)
			{
				pProjectMan->tipsButton = TIPS_NONE;
				break;	
			}
			else
			{
				pProjectMan->tipsButton = TIPS_NONE;
				//继续灌注管道
                DCMotor_Run((DCMotorEnum_TypeDef)pumpSel, CW, 10);
                vTaskDelay(125);				
                DCMotor_Stop((DCMotorEnum_TypeDef)pumpSel);
			}
		}		
	}

    vTaskDelay(1000);

	//开始校准
    DCMotor_Run((DCMotorEnum_TypeDef)pProjectMan->caliPumpSel, CW, 10);
	//直流电机运行40次的1mL，所以共40mL
	for(i=0;i<40;i++)
		vTaskDelay(time);
	DCMotor_Stop((DCMotorEnum_TypeDef)pProjectMan->caliPumpSel);
    
	SetControlEnable(CALIBRATIONPAGE_INDEX, CALI_PUMPSELECT_BUTTON, 1);
	SetControlEnable(CALIBRATIONPAGE_INDEX, CALI_START_BUTTON, 1);
}

#ifdef __cplusplus
}
#endif
