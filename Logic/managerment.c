#include "managerment.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "CPrintf.h"
#include "../HARDWARE/DCMotor/dcmotor.h"
#include "../HARDWARE/StepMotor/stepmotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HMI/hmi_driver.h"
#include "../HMI/hmi_user_uart.h"

#include "main.h"

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

void initUI(void)
{
	//��ʼ����Ŀ�ṹ��
	char str[NAME_SIZE];
	int16 i;
	uint32 j;
	uint32_t dat;

    UartInit(38400);
    
	AT24CXX_Read(RESET_DEFAULT, (uint8_t*)(&dat), sizeof(uint32_t));//�Ƿ��һ�ο�������3��
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(RESET_DEFAULT, (uint8_t*)(&dat), sizeof(uint32_t));
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(RESET_DEFAULT, (uint8_t*)(&dat), sizeof(uint32_t));
	if(dat != EEPROM_DEFAULT) //�ǣ���ʼ��EEPROM�е�����
	{
		cDebug("RESET_DEFAULT\n");

		pProjectMan->lang = English;
		AT24CXX_Write(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), LANGUAGE_SIZE);

		SetScreen(TIPS0PAGE_INDEX);//��ת����ʾ0ҳ��
		//if(pProjectMan->lang == 0)
			SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, (uint8_t*)"Restore factory settings����");
		//else
		//	SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, "���ڻָ��������á���");

		//��ʼ��У׼����
		initCaliPumpPara(4.0);
		AT24CXX_Write(CALIBPARA_BASEADDR, (uint8_t*)caliPumpPara, CALIBPARA_SIZE);

		//��ʼ��λ��У׼����
		pProjectMan->posCali1 = STEPMOTOR_OFFSET;
		pProjectMan->posCali2 = STEPMOTOR_PUMP_OFFSET;
		AT24CXX_Write(POSCALI_BASEADDR, (uint8_t*)(&pProjectMan->posCali1), POSCALI_SIZE);

		//��ʼ����Ŀ����
		for(i=PROJECT_COUNT-1;i>=0;i--)
		{
			memset(str, '\0', NAME_SIZE);
			sprintf(str, "project%d", i);
			initProjectStruct(&project[0], i, (uint8_t*)str);

			cDebug("project[0].name = %s\n", project[0].name);
			cDebug("project[0].index = %d\n", (uint16_t)project[0].index);

			AT24CXX_Write(PROJECT_BASEADDR+i*PROJECT_SIZE, (uint8_t*)(&project[0]), PROJECT_SIZE);	
		}
		
		dat = EEPROM_DEFAULT;
		AT24CXX_Write(RESET_DEFAULT, (uint8_t*)&dat, sizeof(uint32_t));	

		SetScreen(LOGOPAGE_INDEX);//��ת��LOGOҳ��
	}
	else //�񣬴�EEPROM�ж�ȡ����
	{
		cDebug("read data from EEPROM\n");
		
//		//��ʼ��λ��У׼����
//		pProjectMan->posCali1 = STEPMOTOR_OFFSET;
//		pProjectMan->posCali2 = STEPMOTOR_PUMP_OFFSET;
//		AT24CXX_Write(POSCALI_BASEADDR, (uint8_t*)(&pProjectMan->posCali1), POSCALI_SIZE);

		AT24CXX_Read(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), LANGUAGE_SIZE); //�������Բ���

		if(pProjectMan->lang == 0)
		{
			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"System initializing����");
		}
		else
			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳ��ʼ���С���");

		AT24CXX_Read(CALIBPARA_BASEADDR, (uint8_t*)caliPumpPara, CALIBPARA_SIZE); //����У׼����

		AT24CXX_Read(POSCALI_BASEADDR, (uint8_t*)&pProjectMan->posCali1, POSCALI_SIZE);  //����λ��У׼����

//		for(i=0;i<PUMP_COUNT;i++)
//			cDebug("caliPumpPara[%d] = %f\n", i, caliPumpPara[i]);
		//AT24CXX_Read(PROJECT_BASEADDR + PROJECT_SIZE, (uint8_t*)&project[0], PROJECT_SIZE);  //������һ����Ŀ����
		AT24CXX_Read(PROJECT_BASEADDR, (uint8_t*)&project[0], PROJECT_SIZE);  //������һ����Ŀ����
		
		//cDebug("PROJECT_SIZE = %d\n", PROJECT_SIZE);
		cDebug("project[0].name = %s\n", project[0].name);
		cDebug("project[0].index = %d\n", (uint16_t)project[0].index);	
	}

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
}

//void uartInterrupt(uint8 data)
//{
//	queue_push(data);
//}

//void loopForever(void)
//{
//	/************************************************************************/
//	/*��������                                                              */
//	/************************************************************************/
//	//��Ŀ����
//	switch(pProjectMan->runningType)
//	{
//	case RUNNING_NONE:
//		break;
//	case RUNNING_PROJECT: //��Ŀ������
//	{
//		projectProgram();
//		pProjectMan->runningType = RUNNING_NONE;
//	}
//	break;
//	case RUNNING_BACKFLOW:
//		break;
//	case RUNNING_PURGE:	 //ִ����ϴ����
//		purgeProgram();
//		pProjectMan->runningType = RUNNING_NONE;  //����ִ�����Ҫ�����־	
//		break;
//	case RUNNING_CALIBRATION:
//		break;
//	case RUNNING_HOME:
//		//��ԭ��
//		pStepMotor->Home();
//		SetScreen(MAINPAGE_INDEX);
//		pProjectMan->runningType = RUNNING_NONE;
//		beepAlarm(1);
//		break;
//	default:
//		break;
//	}
//}

/************************************************************************/
/* ������ʼ��Demo                                                       */
/************************************************************************/
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

//�����䶯��У׼�����б�
float caliPumpPara[PUMP_COUNT];


//��ʼ��У׼����
void initCaliPumpPara(float para)
{
	uint16 i;
	for(i=0;i<PUMP_COUNT;i++)
		caliPumpPara[i] = para;
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

void HomeTask(void)
{
	if(pProjectMan->lang == 0)
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"System initializing����");
	else
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳ��ʼ���С���");
    
	//�����Һ��
	DCMotor_WastePump_SetPos(UP);

	//��ԭ��
	StepMotor_Home(STEPMOTOR_MAIN);
	SetScreen(MAINPAGE_INDEX);
	beepAlarm(1);
    
    vTaskDelete( NULL );
}


#ifdef __cplusplus
}
#endif
