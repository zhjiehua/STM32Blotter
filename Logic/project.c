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
/* У׼ҳ�������б��ַ���                                             */
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
/* �����༭ҳ�������б��ַ���                                             */
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
	"��",
	"����",
	"Ĥ��",
};

const char actionVoiceMenuText[4][7] = {
	"None",
	"Short",
	"Middle",
	"Long",
};
const char actionVoiceMenuTextCh[4][3] = {
	"��",
	"��",
	"��",
	"��",
};

const char actionSpeedMenuText[3][7] = {
	"Slow",
	"Middle",
	"Fast",
};
const char actionSpeedMenuTextCh[3][5] = {
	"����",
	"����",
	"����",
};

//char actSpeedMenuTextCh[3][5] = {
//	"����",
//	"����",
//	"����",
//};

/************************************************************************/
/* ��Ϣҳ�������б��ַ���                                             */
/************************************************************************/
const char langMenuText[2][8] = {
	"English",
	"Chinese",
};
const char langMenuTextCh[2][5] = {
	"Ӣ��",
	"����",
};

//����12����Ŀ
//Project_TypeDef project[PROJECT_COUNT];
Project_TypeDef project[1];

/******************************************************************************************************/

//��ע�ܵ�
void fillTube(uint8_t *pStopFlag)
{
	uint16 i = 0;
	uint8_t relCoord;

	//��ʾ�Ƿ��ע�ܵ�
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS2PAGE_INDEX);//��ת����ʾ2ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether fill the tube?");
	else
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"�Ƿ��ע�ܵ���");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	if(pProjectMan->tipsButton == TIPS_CANCEL)
	{
		pProjectMan->tipsButton = TIPS_NONE;
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
		xSemaphoreGive(pProjectMan->lcdUartSem);
		return;
	}
	pProjectMan->tipsButton = TIPS_NONE;

	//״̬��ʾ��ע�ܵ���
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In filling tube����");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��ע�ܵ��С���");

	for(i=0;i<ACTIONS_PER_PROJECT;i++)
	{
		if(pProjectMan->pCurRunningProject->action[i].pump < PUMP0)
			pProjectMan->pumpSelPumpSel |= (PUMP1_MASK << pProjectMan->pCurRunningProject->action[i].pump);
	}
	
	SetScreen(SELECTPUMPPAGE_INDEX);//��ת����ѡ��ҳ��
	BatchBegin(SELECTPUMPPAGE_INDEX);//���±�ѡ��ҳ��ؼ�״̬
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
	while(pProjectMan->tipsButton == TIPS_NONE) //�ȴ���ѡ�����
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
	xSemaphoreGive(pProjectMan->lcdUartSem);
	if(pProjectMan->pumpSelPumpSel == 0x00)
		return;

	//��ԭ��
	//pStepMotor->Home();

	//���ֶ����ԭλ
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]Ϊ�ֶ���

	vTaskDelay(1000);
	
	//��ʾ����Һ�����ڲ۰��1��λ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the WASTE TANK in the Plate1");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"��ѷ�Һ�����ڰ��1λ��");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//��עҪ��ע�Ĺܵ�
	Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
	for(i=0;i<8;i++)
	{
		if(pProjectMan->pumpSelPumpSel & (0x01<<i))
		{
			//����Һ��ת����Ӧ�ı�λ��
			relCoord = Turntable_Abs2Rel(AbsCoordinate[i]);
			Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]Ϊ��Һ��
			
			vTaskDelay(1000);

            DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);			
			vTaskDelay(FILLINGTUBE_TIME);			
			DCMotor_Stop((DCMotorEnum_TypeDef)i);

			while(1)
			{
				//��ʾ�Ƿ������ע�ܵ�
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetScreen(TIPS2PAGE_INDEX);//��ת����ʾ2ҳ��
				if(pProjectMan->lang == 0)
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether continue fill the tube?");
				else
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"�Ƿ������ע�ܵ���");
				
				SetBuzzer(BEEPER_TIME_SHORT);
				xSemaphoreGive(pProjectMan->lcdUartSem);
				while(pProjectMan->tipsButton == TIPS_NONE)
					vTaskDelay(10);
				
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
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
					DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);			
                    vTaskDelay(FILLINGTUBE_CON_TIME);			
                    DCMotor_Stop((DCMotorEnum_TypeDef)i);
				}
			}

			vTaskDelay(500);
		}
	}
	Turntable_SetOffset(pMotorMan->motorParaWastePumpCalib);
	
	//��ȡ��Һ
	wasteFluidAbsorb(pStopFlag);

	//����Һ��ת���ֶ���λ��
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]Ϊ��Һ��

	//��ʾ�ƶ���Һ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please take away the Waste Tank");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"�����߷�Һ��");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
	
	pProjectMan->tipsButton = TIPS_NONE;

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//���ð忨
void placePlate(uint8_t *pStopFlag)
{
	uint8_t startPlate, endPlate;
	uint8_t relCoord;
	uint8_t str[50];
	uint16_t i;
	startPlate = (pProjectMan->startTank-1)/TANK_PER_PLATE+1;
	endPlate = (pProjectMan->endTank-1)/TANK_PER_PLATE+1;
	
	//״̬��ʾ���ò۰���
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In placing Plate����");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"���ò۰��С���");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//for(i=startPlate;i<=endPlate;i++)
	for(i=endPlate;i>=startPlate;i--)
	{
		//����Ӧ�۰�ת���ֶ���
		relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);	
		Turntable_RelativePosition(relCoord, TANK_PER_PLATE*(i-1)+3, pStopFlag);

		//��ʾ���ò۰�
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
		if(pProjectMan->lang == 0)
			sprintf((char*)str, "Please place the PLATE in the Plate%d", i);
		else
			sprintf((char*)str, "�뽫�۰����ڡ����%d��λ��", i);
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)str);

		SetBuzzer(BEEPER_TIME_SHORT);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		while(pProjectMan->tipsButton == TIPS_NONE)
			vTaskDelay(10);
		
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
		xSemaphoreGive(pProjectMan->lcdUartSem);
		pProjectMan->tipsButton = TIPS_NONE;
	}

	//��״̬��ʾ
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��	
}

//��Һ
void imbibition(uint8_t *pStopFlag)
{
	uint16_t i, j;
	uint8_t relCoord;
	uint16_t delayTime;

	if(pProjectMan->pCurRunningAction->imbiAmount)//��Һ������0
	{
		//״̬��ʾ��Һ��
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In imbibing����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��Һ�С���");
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//������ձ�
		MOS2 = 1;
		
		//�ɿ��йܷ�
		RELAY = 1;
		
		//��ʱ����շ�Һƿ�Ŀ����γ���ѹ��
		vTaskDelay(4000);

		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//������Һ�༭������Ϊ��ǰ��
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, i);		
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//����Ӧ��ת����Һ��λ��
			relCoord = Turntable_Abs2Rel(AbsCoordinate[8]);
			Turntable_RelativePosition(relCoord, i, pStopFlag);

			vTaskDelay(1000);
			
			//���·�Һ��
			WastePump_SetPos(DOWN, pMotorMan->motorParaWastePumpSpeed, pStopFlag);
			//cDebug("DOWN\n");
			
			//��ʱ��Ӧʱ��
			delayTime = pProjectMan->pCurRunningAction->imbiAmount;
			for(j=0;j<delayTime;j++)
				vTaskDelay(1000);

			//�����Һ��
			WastePump_SetPos(UP, pMotorMan->motorParaWastePumpSpeed, pStopFlag);
			//cDebug("UP\n");	
		}
		
		//��ʱ����ȡʣ�µ�Һ��
		vTaskDelay(4000);

		//�رռйܷ�
		RELAY = 0;
		
		//ֹͣ��ձ�
		MOS2 = 0;
		
		//������Һ�༭������Ϊ0
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 0);
		xSemaphoreGive(pProjectMan->lcdUartSem);		
	}

	//��״̬��ʾ
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);	
}

//������ʾ
void hint(uint8_t *pStopFlag)
{
	uint16_t i;
	uint8_t relCoord;
	uint8_t str[50];

	if(pProjectMan->pCurRunningAction->tips != NO_TIPS)//����ʾ
	{
		//״̬��ʾ��Һ��
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In hinting����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��ʾ�С���");
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{	
			//����Ӧ��ת���ֶ���λ��
			relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);	
			Turntable_RelativePosition(relCoord, i, pStopFlag);
			
			vTaskDelay(1000);
			
			//��ʾ������Ʒ��Ĥ��
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
			if(pProjectMan->pCurRunningAction->tips == SAMPLE_TIPS)
			{
				if(pProjectMan->lang == 0)
					sprintf((char*)str, "Please place the SAMPLE in the Tank%d", i);
				else
					sprintf((char*)str, "�뽫�����������ڡ��ۺ�%d��λ��", i);
				SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)str);				
			}
			else
			{
				if(pProjectMan->lang == 0)
					sprintf((char*)str, "Please place the MEMBRANE in the Tank%d", i);
				else
					sprintf((char*)str, "�뽫��Ĥ�������ڡ��ۺ�%d��λ��", i);
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
			SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
			xSemaphoreGive(pProjectMan->lcdUartSem);
			pProjectMan->tipsButton = TIPS_NONE;
		}	
	}

	//��״̬��ʾ
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//��ע
void adding(uint8_t *pStopFlag)
{
	uint8_t relCoord;
	//uint8_t str[50];
	uint16_t i, j;
	float time, times;

	if(pProjectMan->pCurRunningAction->pump != 8)//��ѡ��ã����8Ϊ0����ʾ�ޱ�
	{
		//״̬��ʾ��Һ��
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In adding����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��ע�С���");

		//���±ü�������Ϊ��ǰѡ��ıñ��
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, pProjectMan->pCurRunningAction->pump+1);		
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//���¼�ע�༭������Ϊ��ǰ��
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, i);			
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//����Ӧ��ת����Һ��λ��
			relCoord = Turntable_Abs2Rel(AbsCoordinate[pProjectMan->pCurRunningAction->pump]);	
			Turntable_RelativePosition(relCoord, i, pStopFlag);
			
			vTaskDelay(1000);

			//�䶯�ü�ע
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

	//���¼�ע�༭������Ϊ0
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, 0);
	//���±ü�������Ϊ0
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, 0);

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//����
void incubation(uint8_t *pStopFlag)
{
	if(pProjectMan->pCurRunningAction->shakeTime.hour > 0
		|| pProjectMan->pCurRunningAction->shakeTime.minute > 0)//����ʱ�����0
	{
		//״̬��ʾ������
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In incubating����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"�����С���");

		//����RTC
		StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
	   	//����ҡ���ٶ�
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //��
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
			break;
			case 1:	 //��
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaMiddleSpeed);
			break;
			case 2:	 //��
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaFastSpeed);
			break;
			default: //Ĭ��
                StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
			break;
		}

		//ʹ����ͣ ֹͣ��ť
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 1);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 1);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		//ת��ת�̲��ȴ�����ʱ�䵽
		StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);

		while(pProjectMan->RTCTimeout == 0) //�ȴ�ʱ�䵽
		{
#if 1
			//����Ƿ�����ͣ
			if(pProjectMan->exception == EXCEPTION_PAUSE)
			{
				//pProjectMan->exception = EXCEPTION_NONE;

				//��ͣRTC
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				PauseTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
				xSemaphoreGive(pProjectMan->lcdUartSem);
				
				//ֹͣ����׼��
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //��
						Turntable_StopAndAlign(2, pStopFlag);
					break;
					case 1:	 //��
						Turntable_StopAndAlign(4, pStopFlag);
					break;
					case 2:	 //��
						Turntable_StopAndAlign(7, pStopFlag);
					break;
					default: //Ĭ��
						Turntable_StopAndAlign(2, pStopFlag);
					break;
				}

				//�ȵ��ָ�
				while(pProjectMan->exception != EXCEPTION_NONE)
				{
					if(pProjectMan->rotateFlag == 1)
					{
						//��һ����
						Turntable_Position(CCW, 1, pStopFlag);
						pProjectMan->rotateFlag = 0;
					}
					vTaskDelay(10);
				}

				if(pProjectMan->jumpTo == 1) //��ͣҳ�����ת�����õ�
				{
					pProjectMan->curLoopTime = pProjectMan->pCurRunningAction->loopTime; //�����˳�ѭ��	
					Turntable_Home(pStopFlag);	//��ԭ��
					//pProjectMan->jumpTo = 0;
					
					vTaskDelay(1000);
					
					return;
				}

				//����ҡ���ٶ�
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //��
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
					break;
					case 1:	 //��
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaMiddleSpeed);
					break;
					case 2:	 //��
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaFastSpeed);
					break;
					default: //Ĭ��
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, pMotorMan->motorParaLowSpeed);
					break;
				}

				//ת��ת�̲��ȴ�����ʱ�䵽
				StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);
				pProjectMan->RTCTimeout = 0;
				//�ָ�RTC
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
				
				//ֹͣ����׼��
//				switch(pProjectMan->pCurRunningAction->shakeSpeed)
//				{
//					case 0:	 //��
//						Turntable_StopAndAlign(2, pStopFlag);
//					break;
//					case 1:	 //��
//						Turntable_StopAndAlign(4, pStopFlag);
//					break;
//					case 2:	 //��
//						Turntable_StopAndAlign(7, pStopFlag);
//					break;
//					default: //Ĭ��
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

		//ֹͣ����׼��
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //��
				Turntable_StopAndAlign(2, pStopFlag);
			break;
			case 1:	 //��
				Turntable_StopAndAlign(4, pStopFlag);
			break;
			case 2:	 //��
				Turntable_StopAndAlign(7, pStopFlag);
			break;
			default: //Ĭ��
				Turntable_StopAndAlign(2, pStopFlag);
			break;
		}

		//������ͣ ֹͣ��ť
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 0);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 0);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		vTaskDelay(1000);
		
		//������Ҫ��ԭ��
		Turntable_Home(pStopFlag);
		
		vTaskDelay(1000);
	}

	//��״̬��ʾ
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

//ִ�ж���
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

	//��״̬��ʾ
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//�ȵ�ת��ֹͣ
	while(!StepMotor_IsStop(STEPMOTOR_MAIN) && !(*pStopFlag))
		vTaskDelay(10);

	//״̬��ʾ��Һ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In preparing����");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"׼���С���");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//��ԭ��
	Turntable_Home(pStopFlag);

	vTaskDelay(1000);
	
	//��ע�ܵ�
	fillTube(pStopFlag);

	//���ð忨
	placePlate(pStopFlag);

	//״̬��ʾ��Һ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Preparation finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"׼����ɣ�");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//ִ�ж���
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
			
			//��Һ
			imbibition(pStopFlag);

			//������ʾ
			hint(pStopFlag);

			//��ע
			adding(pStopFlag);

			//����
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

	//״̬��ʾ���
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"���!");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//��ԭ��
	Turntable_Home(pStopFlag);

	//��ʾ�������
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Project has Finish!");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"��Ŀ���!");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	pProjectMan->tipsButton = TIPS_NONE;

	//������Ŀҳ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PROJECTPAGE_INDEX);
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

#ifdef __cplusplus
}
#endif
