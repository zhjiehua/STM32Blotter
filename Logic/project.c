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

#include "main.h"
    
/************************************************************************/
/* 校准页面下拉列表字符串                                             */
/************************************************************************/
const char caliPumpMenuText[8][3] = {
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
};

/************************************************************************/
/* 动作编辑页面下拉列表字符串                                             */
/************************************************************************/
const char actionPumpMenuText[9][3] = {
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"0",
};

const char actionTipsMenuText[3][9] = {
	"None",
	"Sample",
	"Membrane",
};
const char actionTipsMenuTextCh[3][5] = {
	"无",
	"样本",
	"膜条",
};

const char actionVoiceMenuText[4][7] = {
	"None",
	"Short",
	"Middle",
	"Long",
};
const char actionVoiceMenuTextCh[4][3] = {
	"无",
	"短",
	"中",
	"长",
};

const char actionSpeedMenuText[3][7] = {
	"Slow",
	"Middle",
	"Fast",
};
const char actionSpeedMenuTextCh[3][5] = {
	"慢速",
	"中速",
	"快速",
};

//char actSpeedMenuTextCh[3][5] = {
//	"慢速",
//	"中速",
//	"快速",
//};

/************************************************************************/
/* 信息页面下拉列表字符串                                             */
/************************************************************************/
const char langMenuText[2][8] = {
	"English",
	"Chinese",
};
const char langMenuTextCh[2][5] = {
	"英文",
	"中文",
};


//定义12个项目
//Project_TypeDef project[PROJECT_COUNT];
Project_TypeDef project[1];



/******************************************************************************************************/

//灌注管道
void fillTube(void)
{
	uint16 i = 0;
	uint8_t relCoord;

	//提示是否灌注管道
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(TIPS2PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether fill the tube?");
	else
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"是否灌注管道？");

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	if(pProjectMan->tipsButton == TIPS_CANCEL)
	{
		pProjectMan->tipsButton = TIPS_NONE;
		SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
		return;
	}
	pProjectMan->tipsButton = TIPS_NONE;

	//状态显示灌注管道中
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In filling tube……");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"灌注管道中……");

	SetScreen(SELECTPUMPPAGE_INDEX);//跳转到泵选择页面
	BatchBegin(SELECTPUMPPAGE_INDEX);//更新泵选择页面控件状态
	BatchSetButtonValue(PUMPSEL_PUMP1_BUTTON, pProjectMan->pumpSelPumpSel&PUMP1_MASK);
	BatchSetButtonValue(PUMPSEL_PUMP2_BUTTON, pProjectMan->pumpSelPumpSel&PUMP2_MASK);
	BatchSetButtonValue(PUMPSEL_PUMP3_BUTTON, pProjectMan->pumpSelPumpSel&PUMP3_MASK);
	BatchSetButtonValue(PUMPSEL_PUMP4_BUTTON, pProjectMan->pumpSelPumpSel&PUMP4_MASK);
	BatchSetButtonValue(PUMPSEL_PUMP5_BUTTON, pProjectMan->pumpSelPumpSel&PUMP5_MASK);
	BatchSetButtonValue(PUMPSEL_PUMP6_BUTTON, pProjectMan->pumpSelPumpSel&PUMP6_MASK);
	BatchSetButtonValue(PUMPSEL_PUMP7_BUTTON, pProjectMan->pumpSelPumpSel&PUMP7_MASK);
	BatchSetButtonValue(PUMPSEL_PUMP8_BUTTON, pProjectMan->pumpSelPumpSel&PUMP8_MASK);
	BatchEnd();

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE); //等待泵选择完成
		
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面

	if(pProjectMan->pumpSelPumpSel == 0x00)
		return;

	//回原点
	//pStepMotor->Home();

	//将手动点回原位
	relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);
	StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[9]为手动点

	//提示将废液槽置于槽板号1的位置
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the WASTE TANK in the Plate1");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请把废液槽置于板号1位置");
	
	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
	
	//灌注要灌注的管道
	//pStepMotor->offset = STEPMOTOR_PUMP_OFFSET;
	pStepMotor->offset = pProjectMan->posCali2;
	for(i=0;i<8;i++)
	{
		if(pProjectMan->pumpSelPumpSel & (0x01<<i))
		{
			//将废液槽转到对应的泵位置
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[i]);
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[9]为废液槽
			
			vTaskDelay(125);

            DCMotor_Run((DCMotorEnum_TypeDef)i, CW, 10);			
			vTaskDelay(380);			
			DCMotor_Stop((DCMotorEnum_TypeDef)i);

			while(1)
			{
				//提示是否继续灌注管道
				SetScreen(TIPS2PAGE_INDEX);//跳转到提示2页面
				if(pProjectMan->lang == 0)
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether continue fill the tube?");
				else
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"是否继续灌注管道？");
				
				beepAlarm(1);
				while(pProjectMan->tipsButton == TIPS_NONE);
					
				SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
				if(pProjectMan->tipsButton == TIPS_CANCEL)
				{
					pProjectMan->tipsButton = TIPS_NONE;
					break;	
				}
				else
				{
					pProjectMan->tipsButton = TIPS_NONE;
					//继续灌注管道
					DCMotor_Run((DCMotorEnum_TypeDef)i, CW, 10);			
                    vTaskDelay(125);			
                    DCMotor_Stop((DCMotorEnum_TypeDef)i);
				}
			}

			vTaskDelay(255);
		}
	}
	//pStepMotor->offset = STEPMOTOR_OFFSET;
	pStepMotor->offset = pProjectMan->posCali1;
	
	//吸取废液
	wasteFluidAbsorb();

	//将废液槽转到手动点位置
	relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);
	StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[9]为废液槽

	//提示移动废液槽
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please take away the Waste Tank");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请移走废液槽");
	
	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
	pProjectMan->tipsButton = TIPS_NONE;

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
}

//放置板卡
void placePlate(void)
{
	uint8_t startPlate, endPlate;
	uint8_t relCoord;
	uint8_t str[50];
	uint16_t i;
	startPlate = (pProjectMan->startTank-1)/TANK_PER_PLATE+1;
	endPlate = (pProjectMan->endTank-1)/TANK_PER_PLATE+1;
	
	//状态显示放置槽板中
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In placing Plate……");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"放置槽板中……");

	//for(i=startPlate;i<=endPlate;i++)
	for(i=endPlate;i>=startPlate;i--)
	{
		//将对应槽板转到手动点
		relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);	
		StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, TANK_PER_PLATE*(i-1)+3);

		//提示放置槽板
		SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
		if(pProjectMan->lang == 0)
			sprintf((char*)str, "Please place the PLATE in the Plate%d", i);
		else
			sprintf((char*)str, "请将槽板置于【板号%d】位置", i);
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)str);

		beepAlarm(1);
		while(pProjectMan->tipsButton == TIPS_NONE);
			
		SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
		pProjectMan->tipsButton = TIPS_NONE;		
	}

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");

	//SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面	
}

//吸液
void imbibition(void)
{
	uint16_t i, j;
	uint8_t relCoord;
	uint16_t delayTime;

	if(pProjectMan->pCurRunningAction->imbiAmount)//吸液量大于0
	{
		//状态显示吸液中
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In imbibing……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"吸液中……");

		//启动真空泵
		//pDCMotor->SetCMD(PUMP_VACUUM, ENABLE);
        DCMotor_Run(PUMP_VACUUM, CW, 100);
		//松开夹管阀
		//pDCMotor->SetSpeed(PUMP_PINCH, 0);
        DCMotor_Run(PUMP_PINCH, CW, 0);

		//延时，抽空废液瓶的空气形成气压差
		vTaskDelay(255*6);

		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//更新吸液编辑框内容为当前槽
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, i);			

			//将对应槽转到吸液口位置
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[8]);	
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, i);

			//放下废液口
			DCMotor_WastePump_SetPos(DOWN);
			//cDebug("DOWN\n");
			
			//延时对应时间
			delayTime = pProjectMan->pCurRunningAction->imbiAmount;
			for(j=0;j<delayTime;j++)
				vTaskDelay(167);

			//提起废液口
			DCMotor_WastePump_SetPos(UP);
			//cDebug("UP\n");	
		}
		
		//延时，吸取剩下的液体
		vTaskDelay(255*6);

		//关闭夹管阀
		//pDCMotor->SetSpeed(PUMP_PINCH, 100);
        DCMotor_Run(PUMP_PINCH, CW, 100);
		//停止真空泵
		//pDCMotor->SetCMD(PUMP_VACUUM, DISABLE);
        DCMotor_Run(PUMP_VACUUM, CW, 0);
		
		//更新吸液编辑框内容为0
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 0);		
	}

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");	
}

//动作提示
void hint(void)
{
	uint16_t i;
	uint8_t relCoord;
	uint8_t str[50];

	if(pProjectMan->pCurRunningAction->tips != NO_TIPS)//有提示
	{
		//状态显示吸液中
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In hinting……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"提示中……");
		
		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{	
			//将对应槽转到手动点位置
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);	
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, i);
			
			//提示放置样品或膜条
			SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
			if(pProjectMan->pCurRunningAction->tips == SAMPLE_TIPS)
			{
				if(pProjectMan->lang == 0)
					sprintf((char*)str, "Please place the SAMPLE in the Tank%d", i);
				else
					sprintf((char*)str, "请将【样本】置于【槽号%d】位置", i);
				SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)str);				
			}
			else
			{
				if(pProjectMan->lang == 0)
					sprintf((char*)str, "Please place the MEMBRANE in the Tank%d", i);
				else
					sprintf((char*)str, "请将【膜条】置于【槽号%d】位置", i);
				SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)str);
			}

			while(pProjectMan->tipsButton == TIPS_NONE)
				beepAlarm(pProjectMan->pCurRunningAction->voice+1);

			SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
			pProjectMan->tipsButton = TIPS_NONE;
		}	
	}

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面	
}

//加注
void adding(void)
{
	uint8_t relCoord, times;
	//uint8_t str[50];
	uint16_t i, j;
	float time;

	if(pProjectMan->pCurRunningAction->pump != 8)//有选择泵，编号8为0，表示无泵
	{
		//状态显示吸液中
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In adding……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"加注中……");

		//更新泵辑框内容为当前选择的泵编号
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, pProjectMan->pCurRunningAction->pump+1);		

		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		//pStepMotor->offset = STEPMOTOR_PUMP_OFFSET;
		pStepMotor->offset = pProjectMan->posCali2;
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//更新加注编辑框内容为当前槽
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, i);			

			//将对应槽转到吸液口位置
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[pProjectMan->pCurRunningAction->pump]);	
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, i);
			
			vTaskDelay(6000);

			//蠕动泵加注
			times = pProjectMan->pCurRunningAction->addAmount;
			time = pProjectMan->pCaliPumpPara[pProjectMan->pCurRunningAction->pump];
			//SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, pProjectMan->pCurRunningAction->addAmount);
			//SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 10*time);
			//cDebug("adding: %d mL\n", pProjectMan->pCurRunningAction->addAmount);
			
            //pDCMotor->SetCMD((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump, ENABLE);
            DCMotor_Run((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump, CW, 10);
			for(j=0;j<times;j++)
				vTaskDelay(time);	
			//pDCMotor->SetCMD((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump, DISABLE);
			DCMotor_Stop((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump);
            
			vTaskDelay(255);
		}
		//pStepMotor->offset = STEPMOTOR_OFFSET;
		pStepMotor->offset = pProjectMan->posCali1;	
	}

	//更新加注编辑框内容为0
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, 0);
	//更新泵辑框内容为0
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, 0);

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");	
}

//孵育
void incubation(void)
{
	if(pProjectMan->pCurRunningAction->shakeTime.hour > 0
		|| pProjectMan->pCurRunningAction->shakeTime.minute > 0)//孵育时间大于0
	{
		//状态显示孵育中
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In incubating……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"孵育中……");

		//启动RTC
		StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);

	   	//设置摇动速度
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //慢
				//pStepMotor->SetSpeed(SPEDD_SLOW);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
			break;
			case 1:	 //中
				//pStepMotor->SetSpeed(SPEDD_MIDDLE);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_MIDDLE);
			break;
			case 2:	 //快
				//pStepMotor->SetSpeed(SPEDD_FAST);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_FAST);
			break;
			default: //默认
				//pStepMotor->SetSpeed(SPEDD_SLOW);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
			break;
		}

		//使能暂停 停止按钮
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 1);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 1);

		//转动转盘并等待孵育时间到
		StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);

		while(pProjectMan->RTCTimeout == 0) //等待时间到
		{
			//检查是否有暂停
			if(pProjectMan->exception == EXCEPTION_PAUSE)
			{
				//pProjectMan->exception = EXCEPTION_NONE;

				//暂停RTC
				PauseTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);

				//停止并对准槽
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //慢
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
					case 1:	 //中
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 4);
					break;
					case 2:	 //快
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 7);
					break;
					default: //默认
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
				}
				//pStepMotor->StopAndAlign(2);

				//等到恢复
				while(pProjectMan->exception != EXCEPTION_NONE)
				{
					if(pProjectMan->rotateFlag == 1)
					{
						//下一个槽
						StepMotor_Position(STEPMOTOR_MAIN, CCW, 1);
						pProjectMan->rotateFlag = 0;
					}
				}

				if(pProjectMan->jumpTo == 1) //暂停页面的跳转功能用到
				{
					pProjectMan->curLoopTime = pProjectMan->pCurRunningAction->loopTime; //用于退出循环	
					StepMotor_Home(STEPMOTOR_MAIN);	//回原点
					//pProjectMan->jumpTo = 0;
					return;
				}

				//设置摇动速度
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //慢
						//pStepMotor->SetSpeed(SPEDD_SLOW);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
					break;
					case 1:	 //中
						//pStepMotor->SetSpeed(SPEDD_MIDDLE);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_MIDDLE);
					break;
					case 2:	 //快
						//pStepMotor->SetSpeed(SPEDD_FAST);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_FAST);
					break;
					default: //默认
						//pStepMotor->SetSpeed(SPEDD_SLOW);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
					break;
				}

				//转动转盘并等待孵育时间到
				StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);
				pProjectMan->RTCTimeout = 0;
				//恢复RTC
				StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
			}
			else if(pProjectMan->exception == EXCEPTION_STOP)
			{
				//pProjectMan->exception = EXCEPTION_NONE;

				StopTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);

				//停止并对准槽
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //慢
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
					case 1:	 //中
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 4);
					break;
					case 2:	 //快
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 7);
					break;
					default: //默认
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
				}
				//pStepMotor->StopAndAlign(2);

				pProjectMan->RTCTimeout = 0;

				//os_delete_task(TASK_PROJECT);	//删除自己	
			}
		}
		pProjectMan->RTCTimeout = 0;		

		//停止并对准槽
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //慢
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
			break;
			case 1:	 //中
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 4);
			break;
			case 2:	 //快
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 7);
			break;
			default: //默认
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
			break;
		}

		//除能暂停 停止按钮
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 0);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 0);

		//结束需要回原点
		StepMotor_Home(STEPMOTOR_MAIN);	
	}

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");	
}

//执行动作
void execAction(Action_TypeDef act)
{
	 ;
}

/****************************************************************************************************/

void ProjectTask(void)
{
	uint16_t i;
	uint32_t rtcTime;
	pProjectMan->curTank = pProjectMan->startTank;
	pProjectMan->curLoopTime = 1;
	pProjectMan->RTCTimeout = 0;

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");

	//等到转盘停止
	while(StepMotor_IsStop(STEPMOTOR_MAIN));

	//状态显示吸液中
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In preparing……");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"准备中……");

	//回原点
	StepMotor_Home(STEPMOTOR_MAIN);

	//灌注管道
	fillTube();

	//放置板卡
	placePlate();

	//状态显示吸液中
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Preparation finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"准备完成！");

	//执行动作
	for(i=0;i<ACTIONS_PER_PROJECT;i++)
	{
		if(pProjectMan->jumpTo == 1)
		{
			if(&pProjectMan->pCurRunningProject->action[i] != pProjectMan->pCurRunningAction)
				continue;
			else
				pProjectMan->jumpTo = 0;	
		}
		pProjectMan->pCurRunningAction = &pProjectMan->pCurRunningProject->action[i];		
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_ACTION_EDIT, (uint8_t*)pProjectMan->pCurRunningAction->name);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, 0);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, 0);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 0);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_LOOPTIME_EDIT, 1);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_TOTALLOOPTIME_EDIT, pProjectMan->pCurRunningAction->loopTime);
		rtcTime = pProjectMan->pCurProject->action[i].shakeTime.hour*3600
					+ pProjectMan->pCurProject->action[i].shakeTime.minute*60;
		SeTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC, rtcTime);

		for(pProjectMan->curLoopTime=1;pProjectMan->curLoopTime<=pProjectMan->pCurRunningAction->loopTime;pProjectMan->curLoopTime++)
		{
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_LOOPTIME_EDIT, pProjectMan->curLoopTime);
			
			//吸液
			imbibition();

			//动作提示
			hint();

			//加注
			adding();

			//孵育
			incubation();
		}
	}

	//状态显示完成
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"完成!");
	
	//回原点
	StepMotor_Home(STEPMOTOR_MAIN);

	//提示动作完成
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Project has Finish!");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"项目完成!");
	
	beepAlarm(5);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	pProjectMan->tipsButton = TIPS_NONE;

	//跳到项目页面
	SetScreen(PROJECTPAGE_INDEX);	
    
    //删除自己
    vTaskDelete( NULL );
}

#ifdef __cplusplus
}
#endif
