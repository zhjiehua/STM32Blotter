#include "delay.h"
#include "managerment.h"
#include "motorManagerment.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "CPrintf.h"
#include "../HARDWARE/DCMotor/dcmotor.h"
#include "../HARDWARE/StepMotor/stepmotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HARDWARE/LED/led.h"
#include "../HARDWARE/WDG/wdg.h"
#include "../HARDWARE/LED/led.h"
#include "../HMI/hmi_driver.h"
#include "../HMI/hmi_user_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* ������Ŀ����ṹ��                                                   */
/************************************************************************/
ProjectMan_TypeDef ProjectMan;
ProjectMan_TypeDef *pProjectMan = &ProjectMan;

void initProjectMan(ProjectMan_TypeDef *pm)
{
	pm->pCurRunningProject = NULL;
	pm->pCurRunningAction = NULL;
	pm->curTank = 1;
	pm->RTCTimeout = 0;
	pm->curLoopTime = 1;

	pm->pCurProject = &project[0];
	pm->startTank = 1;
	pm->endTank = 1;

	pm->pCurEditProject = &project[0];
	pm->pCurEditAction = &(pm->pCurEditProject->action[0]);

	pm->pCurJumptoAction = pm->pCurRunningAction;
	
	pm->backflowPumpSel = 0x00;
	pm->purgePumpSel = 0x00;
	pm->pumpSelPumpSel = 0x00;

	pm->caliPumpSel = 1;
	pm->pCaliPumpPara = caliPumpPara;
	pm->pCaliPumpTime = caliPumpTime;
	pm->caliAmount = pm->pCaliPumpPara[pm->caliPumpSel];

	pm->tipsButton = TIPS_NONE;
	pm->runningType = RUNNING_NONE;
	pm->exception = EXCEPTION_NONE;
	pm->exceptionButtonFlag = EXCEPTION_NONE;
	pm->rotateFlag = 0;
	pm->jumpTo = 0;
}

/************************************************************************/
/* ���ڽ��ջ�����                                                       */
/************************************************************************/
uint8 cmd_buffer[CMD_MAX_SIZE];

#define EEPROM_DEFAULT 0x11223345

//��ʱ���ص�����
void vTimerCallback( TimerHandle_t xTimer )
{
	// do something no block
	// ��ȡ��ʱ��ID
	uint32_t ulID = ( uint32_t ) pvTimerGetTimerID( xTimer );

	pProjectMan->timerExpireFlag[ulID] = 1;
	
	//ֹͣ��ʱ��
	xTimerStop( xTimer, 0 );
}

//��ʼ�������ʱ��
void initSoftwareTimer(void)
{
	uint32_t i;
	for(i=0;i<SOFTWARETIMER_COUNT;i++)
	{
		//���붨ʱ��
		pProjectMan->xTimerUser[i] = xTimerCreate
					   //��ʱ�����֣�ֻ�ڵ���ʱ����
					   ("Timer's name",
					   //������ڣ��������ó�10ms
					   10 / portTICK_PERIOD_MS,   
					   //�Ƿ��Զ����أ���������Ϊ�Զ�����
					   pdTRUE,
					   //����ʶ�����ĸ���ʱ�����ûص�����
					  ( void * ) i,
					   //�ص�����
					  vTimerCallback);

		 if( pProjectMan->xTimerUser[i] != NULL ) {
			//������ʱ����0��ʾ������
			xTimerStart( pProjectMan->xTimerUser[i], 0 );
		}
		else
		{
			while(1);
		}
	}
}

void initUI(void)
{
	//��ʼ����Ŀ�ṹ��
	char str[NAME_SIZE];
	int16 i;
	uint32 j;
	uint32_t dat;

    UartInit(38400);

	if(END_BASEADDR >= EE_TYPE)
	{
		while(1)
		{
			cDebug("eeprom is out\r\n");
			delay_ms(1000);
			LED2=!LED2;
		}
	}
	cDebug("END_BASEADDR = %d\r\n", END_BASEADDR);
	
#if 1   
	AT24CXX_Read(RESET_BASEADDR, (uint8_t*)(&dat), sizeof(uint32_t));//�Ƿ��һ�ο�������3��
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(RESET_BASEADDR, (uint8_t*)(&dat), sizeof(uint32_t));
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(RESET_BASEADDR, (uint8_t*)(&dat), sizeof(uint32_t));
	cDebug("dat = 0x%X\r\n", dat);
	if(dat != EEPROM_DEFAULT) //�ǣ���ʼ��EEPROM�е�����
	{
		cDebug("RESET_DEFAULT\n");

		pProjectMan->lang = Chinese;//Chinese   English
		AT24CXX_Write(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), 1);

		SetScreen(TIPS0PAGE_INDEX);//��ת����ʾ0ҳ��
		if(pProjectMan->lang == 0)
			SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, (uint8_t*)"Restore factory settings����");
		else
			SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, (uint8_t*)"���ڻָ��������á���");

		//��ʼ��λ��У׼����
		pMotorMan->motorParaWastePumpCalib = 300;
		pMotorMan->motorParaCWPumpCalib = 170;
		pMotorMan->motorParaCCWPumpCalib = 0;
		AT24CXX_Write(WASTEPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaWastePumpCalib), 2);
		AT24CXX_Write(CWPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaCWPumpCalib), 2);
		AT24CXX_Write(CCWPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaCCWPumpCalib), 2);
		
		//��ʼ��У׼����
		initCaliPumpPara(40, 1620);
		AT24CXX_Write(CALIBPUMPPARA_BASEADDR, (uint8_t*)caliPumpPara, PUMP_COUNT*4);
		AT24CXX_Write(CALIBPUMPTIME_BASEADDR, (uint8_t*)caliPumpTime, PUMP_COUNT*4);

		//��ʼ���������
		pMotorMan->motorParaPumpSpeed = 100;
		pMotorMan->motorParaWastePumpSpeed = 60;
		pMotorMan->motorParaTTHomeSpeed = 6;
		pMotorMan->motorParaTTStep1Speed = 0;
		pMotorMan->motorParaTTStep2Speed = 2;
		pMotorMan->motorParaTTLocationSpeed = 5;
		pMotorMan->motorParaLowSpeed = 2;
		pMotorMan->motorParaMiddleSpeed = 5;
		pMotorMan->motorParaFastSpeed = 8;
		AT24CXX_Write(PARAPUMPSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaPumpSpeed), 1);
		AT24CXX_Write(PARAWASTEPUMPSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaWastePumpSpeed), 1);
		AT24CXX_Write(PARATTHOMESPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTHomeSpeed), 1);
		AT24CXX_Write(PARATTSTEP1SPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTStep1Speed), 1);
		AT24CXX_Write(PARATTSTEP2SPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTStep2Speed), 1);
		AT24CXX_Write(PARATTLOCATIONSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTLocationSpeed), 1);
		AT24CXX_Write(PARALOWSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaLowSpeed), 1);
		AT24CXX_Write(PARAMIDDLESPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaMiddleSpeed), 1);
		AT24CXX_Write(PARAFASTSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaFastSpeed), 1);
		
		pMotorMan->motorParaStartFreq = 2000;
		pMotorMan->motorParaEndFreq = 10000;
		pMotorMan->motorParaAccSpeed = 10000;
		pMotorMan->motorParaStepTime = 100;
		AT24CXX_Write(PARASTARTFREQ_BASEADDR, (uint8_t*)(&pMotorMan->motorParaStartFreq), 4);
		AT24CXX_Write(PARAENDFREQ_BASEADDR, (uint8_t*)(&pMotorMan->motorParaEndFreq), 4);
		AT24CXX_Write(PARAACCSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaAccSpeed), 4);
		AT24CXX_Write(PARASTEPTIME_BASEADDR, (uint8_t*)(&pMotorMan->motorParaStepTime), 4);
		
		
		//��ʼ����Ŀ����
		for(i=PROJECT_COUNT-1;i>=0;i--)
		{
			memset(str, '\0', NAME_SIZE);
			sprintf(str, "project%d", i);
			initProjectStruct(&project[0], i, (uint8_t*)str);

			cDebug("project[%d].name = %s\r\n", i, project[0].name);
			cDebug("project[%d].index = %d\r\n", i, (uint16_t)project[0].index);

			AT24CXX_Write(PROJECT_BASEADDR+i*PROJECT_SIZE, (uint8_t*)(&project[0]), PROJECT_SIZE);	
		}

		dat = EEPROM_DEFAULT;
		AT24CXX_Write(RESET_BASEADDR, (uint8_t*)&dat, sizeof(uint32_t));	

		SetScreen(LOGOPAGE_INDEX);//��ת��LOGOҳ��
	}
	else //�񣬴�EEPROM�ж�ȡ����
	{
		cDebug("read data from EEPROM\n");

		AT24CXX_Read(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), 1); //�������Բ���
		if(pProjectMan->lang == 0)
		{
			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"System initializing����");
		}
		else
			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳ��ʼ���С���");

		//����λ��У׼����
		AT24CXX_Read(WASTEPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaWastePumpCalib), 2);
		AT24CXX_Read(CWPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaCWPumpCalib), 2);
		AT24CXX_Read(CCWPUMPCALIB_BASEADDR, (uint8_t*)(&pMotorMan->motorParaCCWPumpCalib), 2);
		
		//����У׼����
		AT24CXX_Read(CALIBPUMPPARA_BASEADDR, (uint8_t*)caliPumpPara, PUMP_COUNT*4);
		AT24CXX_Read(CALIBPUMPTIME_BASEADDR, (uint8_t*)caliPumpTime, PUMP_COUNT*4); 

		//�����������
		AT24CXX_Read(PARAPUMPSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaPumpSpeed), 1);
		AT24CXX_Read(PARAWASTEPUMPSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaWastePumpSpeed), 1);
		AT24CXX_Read(PARATTHOMESPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTHomeSpeed), 1);
		AT24CXX_Read(PARATTSTEP1SPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTStep1Speed), 1);
		AT24CXX_Read(PARATTSTEP2SPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTStep2Speed), 1);
		AT24CXX_Read(PARATTLOCATIONSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaTTLocationSpeed), 1);
		AT24CXX_Read(PARALOWSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaLowSpeed), 1);
		AT24CXX_Read(PARAMIDDLESPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaMiddleSpeed), 1);
		AT24CXX_Read(PARAFASTSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaFastSpeed), 1);
		
		AT24CXX_Read(PARASTARTFREQ_BASEADDR, (uint8_t*)(&pMotorMan->motorParaStartFreq), 4);
		AT24CXX_Read(PARAENDFREQ_BASEADDR, (uint8_t*)(&pMotorMan->motorParaEndFreq), 4);
		AT24CXX_Read(PARAACCSPEED_BASEADDR, (uint8_t*)(&pMotorMan->motorParaAccSpeed), 4);
		AT24CXX_Read(PARASTEPTIME_BASEADDR, (uint8_t*)(&pMotorMan->motorParaStepTime), 4);
		
		//������һ����Ŀ����
		AT24CXX_Read(PROJECT_BASEADDR, (uint8_t*)&project[0], PROJECT_SIZE);  
		
		cDebug("PROJECT_SIZE = %d\r\n", PROJECT_SIZE);
		cDebug("project[0].name = %s\r\n", project[0].name);
		cDebug("project[0].index = %d\r\n", (uint16_t)project[0].index);	
	}
#endif
	
   	//��������
	if(pProjectMan->lang == 0)
		SetLanguage(0, 1);
	else
		SetLanguage(1, 0);

	//��ʼ����Ŀ����ṹ��
	initProjectMan(pProjectMan);

	/*��մ��ڽ��ջ�����*/
	queue_reset();

	//��ʱһ��ʱ��
	for(j=0;j<65536;j++);

	cDebug("initUI success\n");

	//������������
	SetHandShake();//������������
	SetHandShake();//�������������һ����������ᶪʧ
	//SetHandShake();//������������
	
	//PID_Init(&(pProjectMan->pumpCaliPID), 8.0, 0, 0);
	//PID_UpdateSetPoint(&(pProjectMan->pumpCaliPID), 1.0);
}

/************************************************************************/
/* ������ʼ��Demo                                                       */
/************************************************************************/
#if 0
const Action_TypeDef actionDemo = {
	PUMP1,
	SAMPLE_TIPS,
	MIDDLE_VOICE,
	1,
	1,
	SLOW_SPEED,
	{0, 1},
	1,
	1,
	"action0"
};
#else
const Action_TypeDef actionDemo = {
	PUMP0,
	NO_TIPS,
	NO_VOICE,
	0,
	0,
	SLOW_SPEED,
	{0, 0},
	1,
	1,
	"action0"
};
#endif

//�����䶯��У׼�����б�
float caliPumpPara[PUMP_COUNT];
uint32 caliPumpTime[PUMP_COUNT];

//��ʼ��У׼����
void initCaliPumpPara(float para, uint32 time)
{
	uint16 i;
	for(i=0;i<PUMP_COUNT;i++)
	{
		caliPumpPara[i] = para;
		caliPumpTime[i] = time;
	}
}

//��ʼ����Ŀ�ṹ��
void initProjectStruct(Project_TypeDef* pro, uint8 index, uint8 *name)
{
	uint16 i;
	char str[NAME_SIZE];

	memset(pro->name, '\0', NAME_SIZE);
	memcpy(pro->name, name, NAME_SIZE);	//�޸���Ŀ��
	pro->index = index;

	for(i=0;i<ACTIONS_PER_PROJECT;i++)
	{
		memset(str, '\0', NAME_SIZE);
		memset(pro->action[i].name, '\0', NAME_SIZE);
		sprintf(str, "action%d", i);

		memcpy((void *)(&(pro->action[i])), (void *)(&actionDemo), sizeof(actionDemo));

		memcpy(pro->action[i].name, str, NAME_SIZE);  //�޸Ķ�����
		pro->action[i].index = i;//�޸����
	}
}

void HomeProgram(uint8_t *pStopFlag)
{
	if(pProjectMan->lang == 0)
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"System initializing����");
	else
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳ��ʼ���С���");
    
	//�����Һ��
	WastePump_SetPos(UP, pMotorMan->motorParaWastePumpSpeed, pStopFlag);

	//��ԭ��
	Turntable_Home(pStopFlag);
	SetScreen(MAINPAGE_INDEX);
	beepAlarm(1);
}

void UITask(void)
{
    qsize size;
	
    while(1)
    {
        //����UI����
		size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE); //�ӻ�������ȡһ��ָ��
		if(size > 0)//���յ�ָ��
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//����ָ��
		}
		
        vTaskDelay(10);
		
        IWDG_Feed();//ι��
		
		//PhSensorScanTask();
		LED1Task(); //LED1��˸ָʾUI�߳���������
	}
}

void ProjectTask(void)
{
	uint32_t j;

	pProjectMan->lcdNotifyResetFlag = 0;
	
	ResetDevice();//��λ������
	ResetDevice();//��λ������
	
	for(j=0;j<336;j++);
	//for(j=0;j<65536;j++);
	//for(j=0;j<65536;j++);
	
	//������������
	SetHandShake();//������������
	SetHandShake();//�������������һ����������ᶪʧ
	SetHandShake();//������������
	
	while(!pProjectMan->lcdNotifyResetFlag)
		vTaskDelay(100);

	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(MAINPAGE_INDEX);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	while(1)
	{
		if(pProjectMan->projectStatus&PROJECT_RUNNING)
		{
			switch(pProjectMan->projectStatus&0x7F)
			{
				case PROJECT_PROJECT:
					cDebug("start ProjectProgram\r\n");
					ProjectProgram(&pProjectMan->projectStopFlag);		//��Ŀ
				break;
				case PROJECT_BACKFLOW:
					//�ݲ�����������
				
				break;
				case PROJECT_PURGE:
					cDebug("start PurgeProgram\r\n");
					PurgeProgram(&pProjectMan->projectStopFlag);		//��ϴ
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_CALIBRATION:
					cDebug("start CalibraProgram\r\n");
					CalibraProgram(&pProjectMan->projectStopFlag);		//У׼
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(CALIBRATIONPAGE_INDEX, CALI_START_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				
				case MANUAL_PUMP:
					if(pMotorMan->manualPumpAmount)
					{
						DCMotor_Run(pMotorMan->manualPumpNum, pMotorMan->manualPumpDir, pMotorMan->motorParaPumpSpeed);
						vTaskDelay(pProjectMan->pCaliPumpPara[pMotorMan->manualPumpNum]*pMotorMan->manualPumpAmount);
						DCMotor_Stop(pMotorMan->manualPumpNum);
					}
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_PUMPSTART_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_WASTEPUMP_UP:
					WastePump_SetPos(UP, pMotorMan->motorParaWastePumpSpeed, &pProjectMan->projectStopFlag);
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_WASTEPUMPUP_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_WASTEPUMP_DOWN:
					WastePump_SetPos(DOWN, pMotorMan->motorParaWastePumpSpeed, &pProjectMan->projectStopFlag);
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_WASTEPUMPDOWN_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_TURNTABLE_HOME:
					Turntable_Home(&pProjectMan->projectStopFlag);
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLEHOME_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_TURNTABLE_NEXT:
					Turntable_SetOffset(pMotorMan->motorParaCWPumpCalib);
					Turntable_Position(CW, 1, &pProjectMan->projectStopFlag);
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLENEXT_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_TURNTABLE_LAST:
					Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
					Turntable_Position(CCW, 1, &pProjectMan->projectStopFlag);
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLELAST_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_TURNTABLE_NEXT2:
					Turntable_SetOffset(pMotorMan->motorParaCWPumpCalib);
					Turntable_Position(CW, 2, &pProjectMan->projectStopFlag);
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLENEXT2_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_TURNTABLE_LAST2:
					Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
					Turntable_Position(CCW, 2, &pProjectMan->projectStopFlag);
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLELAST2_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case MANUAL_TURNTABLE_POS:
					if(pMotorMan->manualTTPos > 0)
					{
						Turntable_SetOffset(pMotorMan->motorParaCWPumpCalib);
						Turntable_Position(CW, pMotorMan->manualTTPos, &pProjectMan->projectStopFlag);
					}
					else if(pMotorMan->manualTTPos < 0)
					{
						Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
						Turntable_Position(CCW, 0x100-pMotorMan->manualTTPos, &pProjectMan->projectStopFlag);
					}
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_TURNTABLEPOS_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				default:
				break;
			}
			xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
			pProjectMan->projectStatus = 0;
			pProjectMan->projectStopFlag = 0;
			xSemaphoreGive(pProjectMan->projectStatusSem);
		}
		else
			vTaskDelay(10);
		
		vTaskDelay(10);
		LED2Task(); //LED2��˸ָʾPROJECT�߳���������	
	}
}

#ifdef __cplusplus
}
#endif
