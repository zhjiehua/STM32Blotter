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
    
//吸取废液
void wasteFluidAbsorb(uint8_t *pStopFlag)
{
	uint8_t relCoord;

	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	//状态显示孵育中
	if(pProjectMan->lang == 0)
	{
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In absorbing waste fluid……");
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"In absorbing waste fluid……");
	}
	else
	{
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"吸取废液中……");
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"吸取废液中……");
	}
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//将废液槽转到废液口位置
	relCoord = Turntable_Abs2Rel(AbsCoordinate[8]); //废液口位置
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //将废液槽转到废液口

	//启动真空泵
	MOS2 = 1;
	
	//松开夹管阀
	RELAY = 1;

	//延时，抽空废液瓶的空气形成气压差
	vTaskDelay(255*6);

	//放下废液口
	WastePump_SetPos(DOWN, pMotorMan->motorParaWastePumpSpeed, pStopFlag);

	//延时一小段时间
	vTaskDelay(255*13);

	//提起废液口
	WastePump_SetPos(UP, pMotorMan->motorParaWastePumpSpeed, pStopFlag);

	//延时，吸取剩下的液体
	vTaskDelay(255*6);

	//关闭夹管阀
	RELAY = 0;
	//停止真空泵
	MOS2 = 0;

	//状态显示孵育中
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}


 //清洗程序
void PurgeProgram(uint8_t *pStopFlag)
{
	uint16_t i;
	uint8_t relCoord;
	uint8_t str[50];
	
	if(pProjectMan->purgePumpSel == 0x00)
	{
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetButtonValue(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0x00); //设置按键为弹起状态
		xSemaphoreGive(pProjectMan->lcdUartSem);
		return;
	}

	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetControlVisiable(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0);
	SetControlVisiable(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP1_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP2_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP3_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP4_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP5_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP6_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP7_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP8_BUTTON, 0);

	if(pProjectMan->lang == 0)
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"In preparing……");
	else
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"准备中……");

	//提示将要清洗的管道置于蒸馏水中
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the tube that to be washed in the distilled water");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请将要清洗的管道置于蒸馏水中");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//回原点
	Turntable_Home(pStopFlag);

	//将手动点回原位
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]为手动点

	//提示将废液槽置于槽板号1的位置
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the Waste Tank to the plate1");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请将废液槽置于【板号1】位置");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面

	if(pProjectMan->lang == 0)
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"Preparation finish!");
	else
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"准备完成！");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//清洗要清洗的管道
	Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
	for(i=0;i<8;i++)
	{
		if(pProjectMan->purgePumpSel & (0x01<<i))
		{
			if(pProjectMan->lang == 0)
				sprintf((char*)str, "Washing the tube %d ……", i+1);
			else
				sprintf((char*)str, "清洗【管道%d】中……", i+1);
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)str);
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//将废液槽转到对应的泵位置
			relCoord = Turntable_Abs2Rel(AbsCoordinate[i]);
			Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[8]为废液口

            DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);            
			vTaskDelay(4000);
            DCMotor_Stop((DCMotorEnum_TypeDef)i);

			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
	}
	Turntable_SetOffset(pMotorMan->motorParaWastePumpCalib);
	
	//吸取废液
	wasteFluidAbsorb(pStopFlag);

	//提示将要清洗的管道置于清洁干燥处
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the tubes that to be washed in the clean and dry place");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请将要清洗的管道置于清洁干燥处");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//抽干要清洗的管道
	Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
	for(i=0;i<8;i++)
	{
		if(pProjectMan->purgePumpSel & (0x01<<i))
		{
			if(pProjectMan->lang == 0)
				sprintf((char*)str, "Pumping water from the tube %d", i+1);
			else
				sprintf((char*)str, "抽空【管道%d】中……", i+1);
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)str);
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//将废液槽转到对应的泵位置
			relCoord = Turntable_Abs2Rel(AbsCoordinate[i]);
			Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[8]为3号槽（即废液槽）

			DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);			
			vTaskDelay(4000);
			DCMotor_Stop((DCMotorEnum_TypeDef)i);

			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
	}
	Turntable_SetOffset(pMotorMan->motorParaWastePumpCalib);

	//吸取废液
	wasteFluidAbsorb(pStopFlag);

	//将废液槽转到手动点
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]); //手动点位置
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //将废液槽转到手动点

	//提示取走废液槽
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please take away the Waste Tank");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请移走废液槽");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面

	//提示清洗完成
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Wash finish!");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"清洗完成!");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面

	if(pProjectMan->lang == 0)
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"Wash finish!");
	else
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"清洗完成!");

	SetButtonValue(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0x00); //设置按键为弹起状态
	SetControlVisiable(PURGEPAGE_INDEX, PURGE_START_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP1_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP2_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP3_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP4_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP5_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP6_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP7_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP8_BUTTON, 1);
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

#ifdef __cplusplus
}
#endif
