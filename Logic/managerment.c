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
/* 定义项目管理结构体                                                   */
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
/* 串口接收缓冲区                                                       */
/************************************************************************/
uint8 cmd_buffer[CMD_MAX_SIZE];

#define EEPROM_DEFAULT 0x11223345

void initUI(void)
{
	//初始化项目结构体
	char str[NAME_SIZE];
	int16 i;
	uint32 j;
	uint32_t dat;

    UartInit(38400);
    
	AT24CXX_Read(RESET_DEFAULT, (uint8_t*)(&dat), sizeof(uint32_t));//是否第一次开机，读3次
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(RESET_DEFAULT, (uint8_t*)(&dat), sizeof(uint32_t));
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(RESET_DEFAULT, (uint8_t*)(&dat), sizeof(uint32_t));
	if(dat != EEPROM_DEFAULT) //是，初始化EEPROM中的数据
	{
		cDebug("RESET_DEFAULT\n");

		pProjectMan->lang = English;
		AT24CXX_Write(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), LANGUAGE_SIZE);

		SetScreen(TIPS0PAGE_INDEX);//跳转到提示0页面
		//if(pProjectMan->lang == 0)
			SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, (uint8_t*)"Restore factory settings……");
		//else
		//	SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, "正在恢复出厂设置……");

		//初始化校准参数
		initCaliPumpPara(4.0);
		AT24CXX_Write(CALIBPARA_BASEADDR, (uint8_t*)caliPumpPara, CALIBPARA_SIZE);

		//初始化位置校准参数
		pProjectMan->posCali1 = STEPMOTOR_OFFSET;
		pProjectMan->posCali2 = STEPMOTOR_PUMP_OFFSET;
		AT24CXX_Write(POSCALI_BASEADDR, (uint8_t*)(&pProjectMan->posCali1), POSCALI_SIZE);

		//初始化项目参数
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

		SetScreen(LOGOPAGE_INDEX);//跳转到LOGO页面
	}
	else //否，从EEPROM中读取数据
	{
		cDebug("read data from EEPROM\n");
		
//		//初始化位置校准参数
//		pProjectMan->posCali1 = STEPMOTOR_OFFSET;
//		pProjectMan->posCali2 = STEPMOTOR_PUMP_OFFSET;
//		AT24CXX_Write(POSCALI_BASEADDR, (uint8_t*)(&pProjectMan->posCali1), POSCALI_SIZE);

		AT24CXX_Read(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), LANGUAGE_SIZE); //读出语言参数

		if(pProjectMan->lang == 0)
		{
			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"System initializing……");
		}
		else
			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"系统初始化中……");

		AT24CXX_Read(CALIBPARA_BASEADDR, (uint8_t*)caliPumpPara, CALIBPARA_SIZE); //读出校准参数

		AT24CXX_Read(POSCALI_BASEADDR, (uint8_t*)&pProjectMan->posCali1, POSCALI_SIZE);  //读出位置校准参数

//		for(i=0;i<PUMP_COUNT;i++)
//			cDebug("caliPumpPara[%d] = %f\n", i, caliPumpPara[i]);
		//AT24CXX_Read(PROJECT_BASEADDR + PROJECT_SIZE, (uint8_t*)&project[0], PROJECT_SIZE);  //读出第一个项目参数
		AT24CXX_Read(PROJECT_BASEADDR, (uint8_t*)&project[0], PROJECT_SIZE);  //读出第一个项目参数
		
		//cDebug("PROJECT_SIZE = %d\n", PROJECT_SIZE);
		cDebug("project[0].name = %s\n", project[0].name);
		cDebug("project[0].index = %d\n", (uint16_t)project[0].index);	
	}

   	//设置语言
	if(pProjectMan->lang == 0)
		SetLanguage(0, 1);
	else
		SetLanguage(1, 0);

	//初始化项目管理结构体
	initProjectMan(pProjectMan);

	/*清空串口接收缓冲区*/
	queue_reset();

	//延时一段时间
	for(j=0;j<65536;j++);

	cDebug("initUI success\n");

	//发送握手命令
	SetHandShake();//发送握手命令
	SetHandShake();//发送握手命令，第一个握手命令会丢失
	//SetHandShake();//发送握手命令
}

//void uartInterrupt(uint8 data)
//{
//	queue_push(data);
//}

//void loopForever(void)
//{
//	/************************************************************************/
//	/*处理流程                                                              */
//	/************************************************************************/
//	//项目运行
//	switch(pProjectMan->runningType)
//	{
//	case RUNNING_NONE:
//		break;
//	case RUNNING_PROJECT: //项目运行中
//	{
//		projectProgram();
//		pProjectMan->runningType = RUNNING_NONE;
//	}
//	break;
//	case RUNNING_BACKFLOW:
//		break;
//	case RUNNING_PURGE:	 //执行清洗程序
//		purgeProgram();
//		pProjectMan->runningType = RUNNING_NONE;  //程序执行完成要清除标志	
//		break;
//	case RUNNING_CALIBRATION:
//		break;
//	case RUNNING_HOME:
//		//回原点
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
/* 动作初始化Demo                                                       */
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

//定义蠕动泵校准参数列表
float caliPumpPara[PUMP_COUNT];


//初始化校准参数
void initCaliPumpPara(float para)
{
	uint16 i;
	for(i=0;i<PUMP_COUNT;i++)
		caliPumpPara[i] = para;
}

//初始化项目结构体
void initProjectStruct(Project_TypeDef* pro, uint8 index, uint8 *name)
{
	uint16 i;
	char str[NAME_SIZE];

	memset(pro->name, '\0', NAME_SIZE);
	memcpy(pro->name, name, NAME_SIZE);	//修改项目名
	pro->index = index;

	for(i=0;i<ACTIONS_PER_PROJECT;i++)
	{
		memset(str, '\0', NAME_SIZE);
		memset(pro->action[i].name, '\0', NAME_SIZE);
		sprintf(str, "action%d", i);

		memcpy((void *)(&(pro->action[i])), (void *)(&actionDemo), sizeof(actionDemo));

		memcpy(pro->action[i].name, str, NAME_SIZE);  //修改动作名
		pro->action[i].index = i;//修改序号
	}
}

void HomeTask(void)
{
	if(pProjectMan->lang == 0)
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"System initializing……");
	else
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"系统初始化中……");
    
	//拉起废液口
	DCMotor_WastePump_SetPos(UP);

	//回原点
	StepMotor_Home(STEPMOTOR_MAIN);
	SetScreen(MAINPAGE_INDEX);
	beepAlarm(1);
    
    vTaskDelete( NULL );
}


#ifdef __cplusplus
}
#endif
