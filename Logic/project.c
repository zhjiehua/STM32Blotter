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
void fillTube(void)
{
	uint16 i = 0;
	uint8_t relCoord;

	//��ʾ�Ƿ��ע�ܵ�
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(TIPS2PAGE_INDEX);//��ת����ʾ2ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether fill the tube?");
	else
		SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"�Ƿ��ע�ܵ���");

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	if(pProjectMan->tipsButton == TIPS_CANCEL)
	{
		pProjectMan->tipsButton = TIPS_NONE;
		SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
		return;
	}
	pProjectMan->tipsButton = TIPS_NONE;

	//״̬��ʾ��ע�ܵ���
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In filling tube����");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��ע�ܵ��С���");

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

	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE); //�ȴ���ѡ�����
		
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��

	if(pProjectMan->pumpSelPumpSel == 0x00)
		return;

	//��ԭ��
	//pStepMotor->Home();

	//���ֶ����ԭλ
	relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);
	StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[9]Ϊ�ֶ���

	//��ʾ����Һ�����ڲ۰��1��λ��
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the WASTE TANK in the Plate1");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"��ѷ�Һ�����ڰ��1λ��");
	
	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	pProjectMan->tipsButton = TIPS_NONE;
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
	
	//��עҪ��ע�Ĺܵ�
	//pStepMotor->offset = STEPMOTOR_PUMP_OFFSET;
	pStepMotor->offset = pProjectMan->posCali2;
	for(i=0;i<8;i++)
	{
		if(pProjectMan->pumpSelPumpSel & (0x01<<i))
		{
			//����Һ��ת����Ӧ�ı�λ��
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[i]);
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[9]Ϊ��Һ��
			
			vTaskDelay(125);

            DCMotor_Run((DCMotorEnum_TypeDef)i, CW, 10);			
			vTaskDelay(380);			
			DCMotor_Stop((DCMotorEnum_TypeDef)i);

			while(1)
			{
				//��ʾ�Ƿ������ע�ܵ�
				SetScreen(TIPS2PAGE_INDEX);//��ת����ʾ2ҳ��
				if(pProjectMan->lang == 0)
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"Whether continue fill the tube?");
				else
					SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t*)"�Ƿ������ע�ܵ���");
				
				beepAlarm(1);
				while(pProjectMan->tipsButton == TIPS_NONE);
					
				SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
				if(pProjectMan->tipsButton == TIPS_CANCEL)
				{
					pProjectMan->tipsButton = TIPS_NONE;
					break;	
				}
				else
				{
					pProjectMan->tipsButton = TIPS_NONE;
					//������ע�ܵ�
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
	
	//��ȡ��Һ
	wasteFluidAbsorb();

	//����Һ��ת���ֶ���λ��
	relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);
	StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, AbsCoordinate[9]); //AbsCoordinate[9]Ϊ��Һ��

	//��ʾ�ƶ���Һ��
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please take away the Waste Tank");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"�����߷�Һ��");
	
	beepAlarm(1);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
	pProjectMan->tipsButton = TIPS_NONE;

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
}

//���ð忨
void placePlate(void)
{
	uint8_t startPlate, endPlate;
	uint8_t relCoord;
	uint8_t str[50];
	uint16_t i;
	startPlate = (pProjectMan->startTank-1)/TANK_PER_PLATE+1;
	endPlate = (pProjectMan->endTank-1)/TANK_PER_PLATE+1;
	
	//״̬��ʾ���ò۰���
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In placing Plate����");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"���ò۰��С���");

	//for(i=startPlate;i<=endPlate;i++)
	for(i=endPlate;i>=startPlate;i--)
	{
		//����Ӧ�۰�ת���ֶ���
		relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);	
		StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, TANK_PER_PLATE*(i-1)+3);

		//��ʾ���ò۰�
		SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
		if(pProjectMan->lang == 0)
			sprintf((char*)str, "Please place the PLATE in the Plate%d", i);
		else
			sprintf((char*)str, "�뽫�۰����ڡ����%d��λ��", i);
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)str);

		beepAlarm(1);
		while(pProjectMan->tipsButton == TIPS_NONE);
			
		SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
		pProjectMan->tipsButton = TIPS_NONE;		
	}

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");

	//SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��	
}

//��Һ
void imbibition(void)
{
	uint16_t i, j;
	uint8_t relCoord;
	uint16_t delayTime;

	if(pProjectMan->pCurRunningAction->imbiAmount)//��Һ������0
	{
		//״̬��ʾ��Һ��
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In imbibing����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��Һ�С���");

		//������ձ�
		//pDCMotor->SetCMD(PUMP_VACUUM, ENABLE);
        DCMotor_Run(PUMP_VACUUM, CW, 100);
		//�ɿ��йܷ�
		//pDCMotor->SetSpeed(PUMP_PINCH, 0);
        DCMotor_Run(PUMP_PINCH, CW, 0);

		//��ʱ����շ�Һƿ�Ŀ����γ���ѹ��
		vTaskDelay(255*6);

		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//������Һ�༭������Ϊ��ǰ��
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, i);			

			//����Ӧ��ת����Һ��λ��
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[8]);	
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, i);

			//���·�Һ��
			DCMotor_WastePump_SetPos(DOWN);
			//cDebug("DOWN\n");
			
			//��ʱ��Ӧʱ��
			delayTime = pProjectMan->pCurRunningAction->imbiAmount;
			for(j=0;j<delayTime;j++)
				vTaskDelay(167);

			//�����Һ��
			DCMotor_WastePump_SetPos(UP);
			//cDebug("UP\n");	
		}
		
		//��ʱ����ȡʣ�µ�Һ��
		vTaskDelay(255*6);

		//�رռйܷ�
		//pDCMotor->SetSpeed(PUMP_PINCH, 100);
        DCMotor_Run(PUMP_PINCH, CW, 100);
		//ֹͣ��ձ�
		//pDCMotor->SetCMD(PUMP_VACUUM, DISABLE);
        DCMotor_Run(PUMP_VACUUM, CW, 0);
		
		//������Һ�༭������Ϊ0
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, 0);		
	}

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");	
}

//������ʾ
void hint(void)
{
	uint16_t i;
	uint8_t relCoord;
	uint8_t str[50];

	if(pProjectMan->pCurRunningAction->tips != NO_TIPS)//����ʾ
	{
		//״̬��ʾ��Һ��
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In hinting����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��ʾ�С���");
		
		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{	
			//����Ӧ��ת���ֶ���λ��
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[9]);	
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, i);
			
			//��ʾ������Ʒ��Ĥ��
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

			while(pProjectMan->tipsButton == TIPS_NONE)
				beepAlarm(pProjectMan->pCurRunningAction->voice+1);

			SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��
			pProjectMan->tipsButton = TIPS_NONE;
		}	
	}

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	SetScreen(RUNNINGPAGE_INDEX);//��ת������ҳ��	
}

//��ע
void adding(void)
{
	uint8_t relCoord, times;
	//uint8_t str[50];
	uint16_t i, j;
	float time;

	if(pProjectMan->pCurRunningAction->pump != 8)//��ѡ��ã����8Ϊ0����ʾ�ޱ�
	{
		//״̬��ʾ��Һ��
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In adding����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��ע�С���");

		//���±ü�������Ϊ��ǰѡ��ıñ��
		SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, pProjectMan->pCurRunningAction->pump+1);		

		//for(i=pProjectMan->startTank;i<=pProjectMan->endTank;i++)
		//pStepMotor->offset = STEPMOTOR_PUMP_OFFSET;
		pStepMotor->offset = pProjectMan->posCali2;
		for(i=pProjectMan->endTank;i>=pProjectMan->startTank;i--)
		{
			//���¼�ע�༭������Ϊ��ǰ��
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, i);			

			//����Ӧ��ת����Һ��λ��
			relCoord = StepMotor_Abs2Rel(STEPMOTOR_MAIN, AbsCoordinate[pProjectMan->pCurRunningAction->pump]);	
			StepMotor_RelativePosition(STEPMOTOR_MAIN, relCoord, i);
			
			vTaskDelay(6000);

			//�䶯�ü�ע
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

	//���¼�ע�༭������Ϊ0
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, 0);
	//���±ü�������Ϊ0
	SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, 0);

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");	
}

//����
void incubation(void)
{
	if(pProjectMan->pCurRunningAction->shakeTime.hour > 0
		|| pProjectMan->pCurRunningAction->shakeTime.minute > 0)//����ʱ�����0
	{
		//״̬��ʾ������
		if(pProjectMan->lang == 0)
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In incubating����");
		else
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"�����С���");

		//����RTC
		StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);

	   	//����ҡ���ٶ�
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //��
				//pStepMotor->SetSpeed(SPEDD_SLOW);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
			break;
			case 1:	 //��
				//pStepMotor->SetSpeed(SPEDD_MIDDLE);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_MIDDLE);
			break;
			case 2:	 //��
				//pStepMotor->SetSpeed(SPEDD_FAST);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_FAST);
			break;
			default: //Ĭ��
				//pStepMotor->SetSpeed(SPEDD_SLOW);
                StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
			break;
		}

		//ʹ����ͣ ֹͣ��ť
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 1);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 1);

		//ת��ת�̲��ȴ�����ʱ�䵽
		StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);

		while(pProjectMan->RTCTimeout == 0) //�ȴ�ʱ�䵽
		{
			//����Ƿ�����ͣ
			if(pProjectMan->exception == EXCEPTION_PAUSE)
			{
				//pProjectMan->exception = EXCEPTION_NONE;

				//��ͣRTC
				PauseTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);

				//ֹͣ����׼��
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
					case 1:	 //��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 4);
					break;
					case 2:	 //��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 7);
					break;
					default: //Ĭ��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
				}
				//pStepMotor->StopAndAlign(2);

				//�ȵ��ָ�
				while(pProjectMan->exception != EXCEPTION_NONE)
				{
					if(pProjectMan->rotateFlag == 1)
					{
						//��һ����
						StepMotor_Position(STEPMOTOR_MAIN, CCW, 1);
						pProjectMan->rotateFlag = 0;
					}
				}

				if(pProjectMan->jumpTo == 1) //��ͣҳ�����ת�����õ�
				{
					pProjectMan->curLoopTime = pProjectMan->pCurRunningAction->loopTime; //�����˳�ѭ��	
					StepMotor_Home(STEPMOTOR_MAIN);	//��ԭ��
					//pProjectMan->jumpTo = 0;
					return;
				}

				//����ҡ���ٶ�
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //��
						//pStepMotor->SetSpeed(SPEDD_SLOW);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
					break;
					case 1:	 //��
						//pStepMotor->SetSpeed(SPEDD_MIDDLE);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_MIDDLE);
					break;
					case 2:	 //��
						//pStepMotor->SetSpeed(SPEDD_FAST);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_FAST);
					break;
					default: //Ĭ��
						//pStepMotor->SetSpeed(SPEDD_SLOW);
                        StepMotor_SetSpeed(STEPMOTOR_MAIN, SPEDD_SLOW);
					break;
				}

				//ת��ת�̲��ȴ�����ʱ�䵽
				StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);
				pProjectMan->RTCTimeout = 0;
				//�ָ�RTC
				StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
			}
			else if(pProjectMan->exception == EXCEPTION_STOP)
			{
				//pProjectMan->exception = EXCEPTION_NONE;

				StopTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);

				//ֹͣ����׼��
				switch(pProjectMan->pCurRunningAction->shakeSpeed)
				{
					case 0:	 //��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
					case 1:	 //��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 4);
					break;
					case 2:	 //��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 7);
					break;
					default: //Ĭ��
						StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
					break;
				}
				//pStepMotor->StopAndAlign(2);

				pProjectMan->RTCTimeout = 0;

				//os_delete_task(TASK_PROJECT);	//ɾ���Լ�	
			}
		}
		pProjectMan->RTCTimeout = 0;		

		//ֹͣ����׼��
		switch(pProjectMan->pCurRunningAction->shakeSpeed)
		{
			case 0:	 //��
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
			break;
			case 1:	 //��
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 4);
			break;
			case 2:	 //��
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 7);
			break;
			default: //Ĭ��
				StepMotor_StopAndAlign(STEPMOTOR_MAIN, 2);
			break;
		}

		//������ͣ ֹͣ��ť
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 0);
		SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 0);

		//������Ҫ��ԭ��
		StepMotor_Home(STEPMOTOR_MAIN);	
	}

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");	
}

//ִ�ж���
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

	//��״̬��ʾ
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");

	//�ȵ�ת��ֹͣ
	while(StepMotor_IsStop(STEPMOTOR_MAIN));

	//״̬��ʾ��Һ��
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In preparing����");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"׼���С���");

	//��ԭ��
	StepMotor_Home(STEPMOTOR_MAIN);

	//��ע�ܵ�
	fillTube();

	//���ð忨
	placePlate();

	//״̬��ʾ��Һ��
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Preparation finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"׼����ɣ�");

	//ִ�ж���
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
			
			//��Һ
			imbibition();

			//������ʾ
			hint();

			//��ע
			adding();

			//����
			incubation();
		}
	}

	//״̬��ʾ���
	if(pProjectMan->lang == 0)
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"Finish!");
	else
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"���!");
	
	//��ԭ��
	StepMotor_Home(STEPMOTOR_MAIN);

	//��ʾ�������
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Project has Finish!");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"��Ŀ���!");
	
	beepAlarm(5);
	while(pProjectMan->tipsButton == TIPS_NONE);
		
	pProjectMan->tipsButton = TIPS_NONE;

	//������Ŀҳ��
	SetScreen(PROJECTPAGE_INDEX);	
    
    //ɾ���Լ�
    vTaskDelete( NULL );
}

#ifdef __cplusplus
}
#endif
