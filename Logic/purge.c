#ifdef __cplusplus	  
extern "C" {
#endif

#include "project.h"
#include "managerment.h"
#include "CPrintf.h"
#include "stdio.h"

#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/Beep/beep.h"

#include "FreeRTOS.h"
#include "task.h"
    
//吸取废液
void wasteFluidAbsorb(void)
{
	uint8_t relCoord;

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

	//将废液槽转到废液口位置
	relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[8]); //废液口位置
	StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //将废液槽转到废液口

	//启动真空泵
	//pDCMotor->SetCMD(PUMP_VACUUM, ENABLE);
    DCMotor_Run(PUMP_VACUUM, CW, 100);
	//松开夹管阀
	//pDCMotor->SetSpeed(PUMP_PINCH, 0);
    DCMotor_Run(PUMP_PINCH, CW, 0);

	//延时，抽空废液瓶的空气形成气压差
	vTaskDelay(255*6);

	//放下废液口
	DCMotor_WastePump_SetPos(DOWN);

	//延时一小段时间
	vTaskDelay(255*13);

	//提起废液口
	DCMotor_WastePump_SetPos(UP);

	//延时，吸取剩下的液体
	vTaskDelay(255*6);

	//关闭夹管阀
	//pDCMotor->SetSpeed(PUMP_PINCH, 100);
    DCMotor_Run(PUMP_PINCH, CW, 100);
	//停止真空泵
	//pDCMotor->SetCMD(PUMP_VACUUM, DISABLE);
    DCMotor_Run(PUMP_VACUUM, CW, 0);

	//状态显示孵育中
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
}


 //清洗程序
void PurgeTask(void)
{
	uint16_t i;
	uint8_t relCoord;
	uint8_t str[50];
				
	if(pProjectMan->purgePumpSel == 0x00)
	{
		SetButtonValue(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0x00); //设置按键为弹起状态
		return;
	}

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

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面

	//回原点
	StepMotor_Home(STEPMOTOR_MAIN);

	//将手动点回原位
	relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);
	StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[9]为手动点

	//提示将废液槽置于槽板号1的位置
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the Waste Tank to the plate1");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请将废液槽置于【板号1】位置");

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面

	if(pProjectMan->lang == 0)	
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"Preparation finish!");
	else
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"准备完成！");

	//清洗要清洗的管道
	//pStepMotor->offset = STEPMOTOR_PUMP_OFFSET;
	pStepMotor->offset = pProjectMan->posCali2;
	for(i=0;i<8;i++)
	{
		if(pProjectMan->purgePumpSel & (0x01<<i))
		{
			if(pProjectMan->lang == 0)
				sprintf((char*)str, "Washing the tube %d ……", i+1);
			else
				sprintf((char*)str, "清洗【管道%d】中……", i+1);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)str);

			//将废液槽转到对应的泵位置
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[i]);
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[8]为废液口

			//pDCMotor->SetSpeed((DCMotorEnum_TypeDef)i, 10);
			//pDCMotor->SetCMD((DCMotorEnum_TypeDef)i, ENABLE);
            DCMotor_Run((DCMotorEnum_TypeDef)i, CW, 10);            
			vTaskDelay(255*3);
			//pDCMotor->SetCMD((DCMotorEnum_TypeDef)i, DISABLE);
            DCMotor_Stop((DCMotorEnum_TypeDef)i);

			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
		}
	}
	//pStepMotor->offset = STEPMOTOR_OFFSET;
	pStepMotor->offset = pProjectMan->posCali1;
	
	//吸取废液
	wasteFluidAbsorb();

	//提示将要清洗的管道置于清洁干燥处
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the tubes that to be washed in the clean and dry place");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请将要清洗的管道置于清洁干燥处");

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面

	//抽干要清洗的管道
	//pStepMotor->offset = STEPMOTOR_PUMP_OFFSET;
	pStepMotor->offset = pProjectMan->posCali2;
	for(i=0;i<8;i++)
	{
		if(pProjectMan->purgePumpSel & (0x01<<i))
		{
			if(pProjectMan->lang == 0)
				sprintf((char*)str, "Pumping water from the tube %d", i+1);
			else
				sprintf((char*)str, "抽空【管道%d】中……", i+1);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)str);

			//将废液槽转到对应的泵位置
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[i]);
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[8]为3号槽（即废液槽）

			DCMotor_Run((DCMotorEnum_TypeDef)i, CW, 10);			
			vTaskDelay(255*3);
			DCMotor_Stop((DCMotorEnum_TypeDef)i);

			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
		}
	}
	//pStepMotor->offset = STEPMOTOR_OFFSET;
	pStepMotor->offset = pProjectMan->posCali1;

	//吸取废液
	wasteFluidAbsorb();

	//将废液槽转到手动点
	relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]); //手动点位置
	StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //将废液槽转到手动点

	//提示取走废液槽
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please take away the Waste Tank");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请移走废液槽");
	
	beepAlarm(1);	
	while(pProjectMan->tipsButton == TIPS_NONE);
	
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(PURGEPAGE_INDEX);//跳转到清洗页面

	//提示清洗完成
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Wash finish!");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"清洗完成!");

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
	
	pProjectMan->tipsButton = TIPS_NONE;
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
    
    //删除自己
    vTaskDelete( NULL );
}

#ifdef __cplusplus
}
#endif
