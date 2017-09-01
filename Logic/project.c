#ifdef __cplusplus	  
extern "C" {
#endif

#include "project.h"
#include "managerment.h"
#include "motorManagerment.h"
#include "CPrintf.h"
#include "stdio.h"

#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../Logic/motorManagerment.h"
#include "../HARDWARE/RelayMOS/RelayMOS.h"
	
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
void fillTube(uint8_t *pStopFlag)
{
	uint16 i = 0;
	uint8_t relCoord;

	//提示是否灌注管道
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS2PAGE_INDEX);//跳转到提示2页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether fill the tube?");
	else
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"是否灌注管道？");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	if(pProjectMan->tipsButton == TIPS_CANCEL)
	{
		pProjectMan->tipsButton = TIPS_NONE;
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
		xSemaphoreGive(pProjectMan->lcdUartSem);
		return;
	}
	pProjectMan->tipsButton = TIPS_NONE;

	//状态显示灌注管道中
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In filling tube……");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"灌注管道中……");

	for(i=0;i<ACTIONS_PER_PROJECT;i++)
	{
		if(pProjectMan->pCurRunningProject->action[i].pump < PUMP0)
			pProjectMan->pumpSelPumpSel |= (PUMP1_MASK << pProjectMan->pCurRunningProject->action[i].pump);
	}
	
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

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE) //等待泵选择完成
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
	xSemaphoreGive(pProjectMan->lcdUartSem);
	if(pProjectMan->pumpSelPumpSel == 0x00)
		return;

	//回原点
	//pStepMotor->Home();

	//将手动点回原位
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]为手动点

	vTaskDelay(1000);
	
	//提示将废液槽置于槽板号1的位置
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the WASTE TANK in the Plate1");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请把废液槽置于板号1位置");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//灌注要灌注的管道
	Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
	for(i=0;i<8;i++)
	{
		if(pProjectMan->pumpSelPumpSel & (0x01<<i))
		{
			//将废液槽转到对应的泵位置
			relCoord = Turntable_Abs2Rel(AbsCoordinate[i]);
			Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]为废液槽
			
			vTaskDelay(1000);

            DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);			
			vTaskDelay(FILLINGTUBE_TIME);			
			DCMotor_Stop((DCMotorEnum_TypeDef)i);

			while(1)
			{
				//提示是否继续灌注管道
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetScreen(TIPS2PAGE_INDEX);//跳转到提示2页面
				if(pProjectMan->lang == 0)
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether continue fill the tube?");
				else
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"是否继续灌注管道？");
				
				SetBuzzer(BEEPER_TIME_SHORT);
				xSemaphoreGive(pProjectMan->lcdUartSem);
				while(pProjectMan->tipsButton == TIPS_NONE)
					vTaskDelay(10);
				
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
				xSemaphoreGive(pProjectMan->lcdUartSem);
				if(pProjectMan->tipsButton == TIPS_CANCEL)
				{
					pProjectMan->tipsButton = TIPS_NONE;
					break;	
				}
				else
				{
					pProjectMan->tipsButton = TIPS_NONE;
					//继续灌注管道
					DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);			
                    vTaskDelay(FILLINGTUBE_CON_TIME);			
                    DCMotor_Stop((DCMotorEnum_TypeDef)i);
				}
			}

			vTaskDelay(500);
		}
	}
	Turntable_SetOffset(pMotorMan->motorParaWastePumpCalib);
	
	//吸取废液
	wasteFluidAbsorb(pStopFlag);

	//将废液槽转到手动点位置
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]为废液槽

	//提示移动废液槽
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please take away the Waste Tank");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"请移走废液槽");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
	
	pProjectMan->tipsButton = TIPS_NONE;

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//放置板卡
void placePlate(uint8_t *pStopFlag)
{
	uint8_t startPlate, endPlate;
	uint8_t relCoord;
	uint8_t str[50];
	uint16_t i;
	startPlate = (pProjectMan->startTank-1)/TANK_PER_PLATE+1;
	endPlate = (pProjectMan->endTank-1)/TANK_PER_PLATE+1;
	
	//状态显示放置槽板中
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In placing Plate……");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"放置槽板中……");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//for(i=startPlate;i<=endPlate;i++)
	for(i=endPlate;i>=startPlate;i--)
	{
		//将对应槽板转到手动点
		relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);	
		Turntable_RelativePosition(relCoord, TANK_PER_PLATE*(i-1)+3, pStopFlag);

		//提示放置槽板
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
		if(pProjectMan->lang == 0)
			sprintf((char*)str, "Please place the PLATE in the Plate%d", i);
		else
			sprintf((char*)str, "请将槽板置于【板号%d】位置", i);
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)str);

		SetBuzzer(BEEPER_TIME_SHORT);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		while(pProjectMan->tipsButton == TIPS_NONE)
			vTaskDelay(10);
		
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
		xSemaphoreGive(pProjectMan->lcdUartSem);
		pProjectMan->tipsButton = TIPS_NONE;
	}

	//清状态显示
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面	
}

//吸液
void imbibition(uint8_t *pStopFlag)
{
	uint16_t i, j;
	uint8_t relCoord;
	uint16_t delayTime;

	if(pProjectMan->pCurRunningAction->imbiAmount)//吸液量大于0
	{
		//状态显示吸液中
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In imbibing……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"吸液中……");
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//启动真空泵
		MOS2 = 1;
		
		//松开夹管阀
		RELAY = 1;
		
		//延时，抽空废液瓶的空气形成气压差
		vTaskDelay(4000);

		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//更新吸液编辑框内容为当前槽
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, i);		
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//将对应槽转到吸液口位置
			relCoord = Turntable_Abs2Rel(AbsCoordinate[8]);
			Turntable_RelativePosition(relCoord, i, pStopFlag);

			vTaskDelay(1000);
			
			//放下废液口
			WastePump_SetPos(DOWN, pMotorMan->motorParaWastePumpSpeed, pStopFlag);
			//cDebug("DOWN\n");
			
			//延时对应时间
			delayTime = pProjectMan->pCurRunningAction->imbiAmount;
			for(j=0;j<delayTime;j++)
				vTaskDelay(1000);

			//提起废液口
			WastePump_SetPos(UP, pMotorMan->motorParaWastePumpSpeed, pStopFlag);
			//cDebug("UP\n");	
		}
		
		//延时，吸取剩下的液体
		vTaskDelay(4000);

		//关闭夹管阀
		RELAY = 0;
		
		//停止真空泵
		MOS2 = 0;
		
		//更新吸液编辑框内容为0
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 0);
		xSemaphoreGive(pProjectMan->lcdUartSem);		
	}

	//清状态显示
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);	
}

//动作提示
void hint(uint8_t *pStopFlag)
{
	uint16_t i;
	uint8_t relCoord;
	uint8_t str[50];

	if(pProjectMan->pCurRunningAction->tips != NO_TIPS)//有提示
	{
		//状态显示吸液中
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In hinting……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"提示中……");
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{	
			//将对应槽转到手动点位置
			relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);	
			Turntable_RelativePosition(relCoord, i, pStopFlag);
			
			vTaskDelay(1000);
			
			//提示放置样品或膜条
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
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
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			while(pProjectMan->tipsButton == TIPS_NONE)
			{
				//beepAlarm(pProjectMan->pCurRunningAction->voice+1);
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetBuzzer(BEEPER_TIME_SHORT*(pProjectMan->pCurRunningAction->voice+1));
				xSemaphoreGive(pProjectMan->lcdUartSem);
				vTaskDelay(2000);
			}		
			
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
			xSemaphoreGive(pProjectMan->lcdUartSem);
			pProjectMan->tipsButton = TIPS_NONE;
		}	
	}

	//清状态显示
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	SetScreen(RUNNINGPAGE_INDEX);//跳转到运行页面
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//加注
void adding(uint8_t *pStopFlag)
{
	uint8_t relCoord;
	//uint8_t str[50];
	uint16_t i, j;
	float time, times;

	if(pProjectMan->pCurRunningAction->pump != 8)//有选择泵，编号8为0，表示无泵
	{
		//状态显示吸液中
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In adding……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"加注中……");

		//更新泵辑框内容为当前选择的泵编号
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, pProjectMan->pCurRunningAction->pump+1);		
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//更新加注编辑框内容为当前槽
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, i);			
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//将对应槽转到吸液口位置
			relCoord = Turntable_Abs2Rel(AbsCoordinate[pProjectMan->pCurRunningAction->pump]);	
			Turntable_RelativePosition(relCoord, i, pStopFlag);
			
			vTaskDelay(1000);

			//蠕动泵加注
			times = pProjectMan->pCurRunningAction->addAmount;
			time = pProjectMan->pCaliPumpTime[pProjectMan->pCurRunningAction->pump];
			//SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, pProjectMan->pCurRunningAction->addAmount);
			//SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 10*time);
			//cDebug("adding: %d mL\n", pProjectMan->pCurRunningAction->addAmount);

			while(times >= 2.0)
			{
				times--;
				
				DCMotor_Run((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump, CW, pMotorMan->motorParaPumpSpeed);
				vTaskDelay(time);	
				DCMotor_Stop((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump);
				
				vTaskDelay(500);
            }
			DCMotor_Run((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump, CW, pMotorMan->motorParaPumpSpeed);
			vTaskDelay(time * times);	
			DCMotor_Stop((DCMotorEnum_TypeDef)pProjectMan->pCurRunningAction->pump);
			
			cDebug("adding amount = %d\r\n", (uint32_t)(time*times));
			
			vTaskDelay(500);
		}
		Turntable_SetOffset(pMotorMan->motorParaWastePumpCalib);
	}

	//更新加注编辑框内容为0
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, 0);
	//更新泵辑框内容为0
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, 0);

	//清状态显示
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//孵育
void incubation(uint8_t *pStopFlag)
{
	if(pProjectMan->pCurRunningAction->shakeTime.hour > 0
		|| pProjectMan->pCurRunningAction->shakeTime.minute > 0)//孵育时间大于0
	{
		//状态显示孵育中
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In incubating……");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"孵育中……");

		//启动RTC
		StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
	   	//设置摇动速度
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //慢
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
			break;
			case 1:	 //中
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaMiddleSpeed);
			break;
			case 2:	 //快
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaFastSpeed);
			break;
			default: //默认
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
			break;
		}

		//使能暂停 停止按钮
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 1);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 1);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//转动转盘并等待孵育时间到
		StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);

		while(pProjectMan->RTCTimeout == 0) //等待时间到
		{
#if 1
			//检查是否有暂停
			if(pProjectMan->exception == EXCEPTION_PAUSE)
			{
				//pProjectMan->exception = EXCEPTION_NONE;

				//暂停RTC
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				PauseTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
				xSemaphoreGive(pProjectMan->lcdUartSem);
				
				//停止并对准槽
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //慢
						Turntable_StopAndAlign(2, pStopFlag);
					break;
					case 1:	 //中
						Turntable_StopAndAlign(4, pStopFlag);
					break;
					case 2:	 //快
						Turntable_StopAndAlign(7, pStopFlag);
					break;
					default: //默认
						Turntable_StopAndAlign(2, pStopFlag);
					break;
				}

				//等到恢复
				while(pProjectMan->exception != EXCEPTION_NONE)
				{
					if(pProjectMan->rotateFlag == 1)
					{
						//下一个槽
						Turntable_Position(CCW, 1, pStopFlag);
						pProjectMan->rotateFlag = 0;
					}
					vTaskDelay(10);
				}

				if(pProjectMan->jumpTo == 1) //暂停页面的跳转功能用到
				{
					pProjectMan->curLoopTime = pProjectMan->pCurRunningAction->loopTime; //用于退出循环	
					Turntable_Home(pStopFlag);	//回原点
					//pProjectMan->jumpTo = 0;
					
					vTaskDelay(1000);
					
					return;
				}

				//设置摇动速度
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //慢
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
					break;
					case 1:	 //中
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaMiddleSpeed);
					break;
					case 2:	 //快
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaFastSpeed);
					break;
					default: //默认
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
					break;
				}

				//转动转盘并等待孵育时间到
				StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);
				pProjectMan->RTCTimeout = 0;
				//恢复RTC
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
				xSemaphoreGive(pProjectMan->lcdUartSem);
			}
			else if(pProjectMan->exception == EXCEPTION_STOP)
			{
				//pProjectMan->exception = EXCEPTION_NONE;
				
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				StopTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
				xSemaphoreGive(pProjectMan->lcdUartSem);
				
				//停止并对准槽
//				switch(pProjectMan->pCurRunningAction->shakeSpeed)
//				{
//					case 0:	 //慢
//						Turntable_StopAndAlign(2, pStopFlag);
//					break;
//					case 1:	 //中
//						Turntable_StopAndAlign(4, pStopFlag);
//					break;
//					case 2:	 //快
//						Turntable_StopAndAlign(7, pStopFlag);
//					break;
//					default: //默认
//						Turntable_StopAndAlign(2, pStopFlag);
//					break;
//				}

//				pProjectMan->RTCTimeout = 0;

				break;
			}
#endif			
			vTaskDelay(10);
		}
		pProjectMan->RTCTimeout = 0;		

		//停止并对准槽
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //慢
				Turntable_StopAndAlign(2, pStopFlag);
			break;
			case 1:	 //中
				Turntable_StopAndAlign(4, pStopFlag);
			break;
			case 2:	 //快
				Turntable_StopAndAlign(7, pStopFlag);
			break;
			default: //默认
				Turntable_StopAndAlign(2, pStopFlag);
			break;
		}

		//除能暂停 停止按钮
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 0);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 0);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		vTaskDelay(1000);
		
		//结束需要回原点
		Turntable_Home(pStopFlag);
		
		vTaskDelay(1000);
	}

	//清状态显示
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//执行动作
void execAction(Action_TypeDef act)
{
	 ;
}

/****************************************************************************************************/

void ProjectProgram(uint8_t *pStopFlag)
{
	uint16_t i;
	uint32_t rtcTime;
	pProjectMan->curTank = pProjectMan->startTank;
	pProjectMan->curLoopTime = 1;
	pProjectMan->RTCTimeout = 0;

	//清状态显示
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//等到转盘停止
	while(!StepMotor_IsStop(STEPMOTOR_MAIN) && !(*pStopFlag))
		vTaskDelay(10);

	//状态显示吸液中
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In preparing……");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"准备中……");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//回原点
	Turntable_Home(pStopFlag);

	vTaskDelay(1000);
	
	//灌注管道
	fillTube(pStopFlag);

	//放置板卡
	placePlate(pStopFlag);

	//状态显示吸液中
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Preparation finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"准备完成！");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//执行动作
	for(i=0;i<ACTIONS_PER_PROJECT;i++)
	{
		vTaskDelay(10);
		
		if(pProjectMan->jumpTo == 1)
		{
			if(&pProjectMan->pCurRunningProject->action[i] != pProjectMan->pCurRunningAction)
				continue;
			else
				pProjectMan->jumpTo = 0;
		}
		pProjectMan->pCurRunningAction = &pProjectMan->pCurRunningProject->action[i];
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);		
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_ACTION_EDIT, (uint8_t*)pProjectMan->pCurRunningAction->name);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, 0);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, 0);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 0);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_LOOPTIME_EDIT, 1);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_TOTALLOOPTIME_EDIT, pProjectMan->pCurRunningAction->loopTime);
		rtcTime = pProjectMan->pCurProject->action[i].shakeTime.hour*3600
					+ pProjectMan->pCurProject->action[i].shakeTime.minute*60;
		SeTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC, rtcTime);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		vTaskDelay(10);
		
		for(pProjectMan->curLoopTime=1;pProjectMan->curLoopTime<=pProjectMan->pCurRunningAction->loopTime;pProjectMan->curLoopTime++)
		{
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_LOOPTIME_EDIT, pProjectMan->curLoopTime);
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//吸液
			imbibition(pStopFlag);

			//动作提示
			hint(pStopFlag);

			//加注
			adding(pStopFlag);

			//孵育
			incubation(pStopFlag);
			
			if(pProjectMan->exception == EXCEPTION_STOP)
				break;
		}
		
		if(pProjectMan->exception == EXCEPTION_STOP)
		{
			pProjectMan->exception = EXCEPTION_NONE;
			return;
		}
	}

	//状态显示完成
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"完成!");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//回原点
	Turntable_Home(pStopFlag);

	//提示动作完成
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//跳转到提示1页面
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Project has Finish!");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"项目完成!");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	pProjectMan->tipsButton = TIPS_NONE;

	//跳到项目页面
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PROJECTPAGE_INDEX);
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

#ifdef __cplusplus
}
#endif
