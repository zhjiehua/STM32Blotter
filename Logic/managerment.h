#ifndef _MANAGERMENT_H_
#define _MANAGERMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../UILogic/pageCommon.h"
#include "project.h"
#include "../HMI/cmd_queue.h"
#include "../HMI/cmd_process.h"
#include "../HMI/hmi_user_uart.h"

#include "../PID/PID.h"
	
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
	
#define SOFTWARETIMER_COUNT 3

#define BEEPER_TIME_SHORT	50
#define BEEPER_TIME_MIDDLE	100
#define BEEPER_TIME_FAST	200

#define FILLINGTUBE_TIME		4000
#define FILLINGTUBE_CON_TIME	500

//��Ŀ�������
#define PROJECT_PROJECT				1
#define PROJECT_BACKFLOW			2
#define PROJECT_PURGE				3
#define PROJECT_CALIBRATION			4
#define MANUAL_PUMP					5
#define MANUAL_WASTEPUMP_UP			6
#define MANUAL_WASTEPUMP_DOWN		7
#define MANUAL_TURNTABLE_HOME		8
#define MANUAL_TURNTABLE_NEXT		9
#define MANUAL_TURNTABLE_LAST		10
#define MANUAL_TURNTABLE_NEXT2		11
#define MANUAL_TURNTABLE_LAST2		12
#define MANUAL_TURNTABLE_POS		13

#define PROJECT_RUNNING				0x80
	
typedef enum
{
	TIPSSOURCE_NONE = 0,

	//tips2ҳ��
	TIPSSOURCE_FILLTIPS,
	TIPSSOURCE_RUNNINGPAUSE,
	TIPSSOURCE_RUNNINGSTOP,

	//tips1ҳ��
	TIPSSOURCE_PUTWASTETANK,
	TIPSSOURCE_GETWASTETANK,
	TIPSSOURCE_PUTTANK,
	TIPSSOURCE_GETTANK,
	TIPSSOURCE_SAMPLE,
	TIPSSOURCE_MEMBRANCE,
}TipsSource_TypeDef;

typedef enum
{
	TIPS_NONE = 0,
	TIPS_CANCEL,
	TIPS_OK,
}TipsButton_TypeDef;

typedef enum
{
	EXCEPTION_NONE = 0,
	EXCEPTION_PAUSE,
	EXCEPTION_STOP,
}Exception_TypeDef;

typedef enum
{
	RUNNING_NONE,
	RUNNING_PROJECT,
	RUNNING_BACKFLOW,
	RUNNING_PURGE,
	RUNNING_CALIBRATION,
	RUNNING_HOME,
}RunningType_TypeDef;

typedef enum
{
	English = 0,
	Chinese,
}Language_TypeDef;

typedef enum
{
	PROJECTSTATUS_IDLE = 0,
	PROJECTSTATUS_SELECTPUMP,
	PROJECTSTATUS_FILLING,
	PROJECTSTATUS_PLACEPLATE,
	PROJECTSTATUS_IMBIBING,
	PROJECTSTATUS_TIPS,
	PROJECTSTATUS_ADDING,
	PROJECTSTATUS_INCUBATION,
	PROJECTSTATUS_TIMEOUT,

	//PROJECTSTATUS_PAUSETIPS,
	PROJECTSTATUS_WAITING,
	PROJECTSTATUS_PAUSE,
}ProjectStatus_TypeDef;

/************************************************************************/
/* ��Ŀ����ṹ��                                                       */
/************************************************************************/
typedef struct
{
	uint8_t projectStatus; //bit7��ʾ���Գ����������У�bit6~bit0��Ӧ��Ŀ�������
	uint8_t projectStopFlag; //
	SemaphoreHandle_t projectStatusSem;
	
	SemaphoreHandle_t lcdUartSem;
	
	Project_TypeDef *pCurRunningProject;  //����ҳ�浱ǰ�������е���Ŀ
	Action_TypeDef *pCurRunningAction;  //����ҳ�浱ǰ�������еĶ���
	uint8 curTank;  //����ҳ�浱ǰ�����Ĳ�
	uint8 curLoopTime;  //����ҳ�浱ǰ�ڼ���ѭ��
	uint8 RTCTimeout;  //����ҳ��RTC��ʱʱ�䵽��־��0:δ����>0:��ʱ�����

	Project_TypeDef *pCurProject;  //��Ŀҳ�浱ǰѡ�����Ŀ
	uint8 startTank;  //��Ŀҳ����ʼ��
	uint8 endTank;  //��Ŀҳ����ֹ��

	Project_TypeDef *pCurEditProject;  //��Ŀ�༭ҳ�浱ǰ�༭����Ŀ
	Action_TypeDef *pCurEditAction;  //��Ŀ�༭ҳ�浱ǰ�༭�Ķ���

	Action_TypeDef *pCurJumptoAction;  //��ͣҳ�����תѡ����

	uint8 backflowPumpSel;  //����ҳ��ı�ѡ��ť״̬
	uint8 purgePumpSel;  //��ϴҳ��ı�ѡ��ť״̬
	uint8 pumpSelPumpSel;  //��ѡ��ҳ��ı�ѡ��ť״̬

	uint8 caliPumpSel;  //У׼ҳ��ı�ѡ��ť״̬
	uint8 caliFlag;
	uint32 *pCaliPumpTime;  //ʵ�ʵ�У׼�ò����б�
	float *pCaliPumpPara;  //ʵ�ʵ�У׼�ò����б�
	float caliAmount;  //У׼ҳ��ʵ�ʼ�ע����������ʱ����

	//�����༭ҳ���������ʱ����
	Pump_TypeDef pump;//�ñ��
	Tips_TypeDef tips;//��ʾ
	Voice_TypeDef voice;//����
	uint8 addAmount;//��ע��
	uint8 imbiAmount;//��Һ��
	ShakeSpeed_TypeDef shakeSpeed;//ҡ���ٶ�
	ShakeTime_TypeDef shakeTime;//ҡ��ʱ��
	uint8 loopTime;//ѭ������

	Exception_TypeDef exception;  //�쳣
	Exception_TypeDef exceptionButtonFlag;  //����ͣ��ֹͣ��ť����ʱ�ñ�־λ����ΪPAUSE/STOP������ڵ����ĶԻ����е����OK������exceptionΪPAUSE/STOP

	uint8 rotateFlag;  //������ͣҳ�����ת��־
	uint8 jumpTo;  //������ͣҳ�����ת��־

	TipsButton_TypeDef tipsButton;  //��ʾҳ��İ�ť״̬
	RunningType_TypeDef runningType;  //�������е�����

	TaskHandle_t projectTaskHandle;
	TaskHandle_t uiTaskHandle;
	
	TimerHandle_t xTimerUser[SOFTWARETIMER_COUNT];
	uint8_t timerExpireFlag[SOFTWARETIMER_COUNT];
	
	Language_TypeDef lang;
	
	//PID_TypeDef pumpCaliPID;
	
	uint8_t lcdNotifyResetFlag;
}ProjectMan_TypeDef;

/************************************************************************/
/*                                                                      */
/************************************************************************/

extern ProjectMan_TypeDef *pProjectMan;
extern uint8 cmd_buffer[CMD_MAX_SIZE];

void initProjectMan(ProjectMan_TypeDef *pm);
void initUI(void);
void initSoftwareTimer(void);

#ifdef __cplusplus
}
#endif

#endif
