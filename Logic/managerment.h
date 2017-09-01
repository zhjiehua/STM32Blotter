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

//项目程序代号
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

	//tips2页面
	TIPSSOURCE_FILLTIPS,
	TIPSSOURCE_RUNNINGPAUSE,
	TIPSSOURCE_RUNNINGSTOP,

	//tips1页面
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
/* 项目管理结构体                                                       */
/************************************************************************/
typedef struct
{
	uint8_t projectStatus; //bit7表示测试程序正在运行，bit6~bit0对应项目程序代号
	uint8_t projectStopFlag; //
	SemaphoreHandle_t projectStatusSem;
	
	SemaphoreHandle_t lcdUartSem;
	
	Project_TypeDef *pCurRunningProject;  //运行页面当前正在运行的项目
	Action_TypeDef *pCurRunningAction;  //运行页面当前正在运行的动作
	uint8 curTank;  //运行页面当前动作的槽
	uint8 curLoopTime;  //运行页面当前第几次循环
	uint8 RTCTimeout;  //运行页面RTC定时时间到标志，0:未到；>0:定时器溢出

	Project_TypeDef *pCurProject;  //项目页面当前选择的项目
	uint8 startTank;  //项目页面起始槽
	uint8 endTank;  //项目页面终止槽

	Project_TypeDef *pCurEditProject;  //项目编辑页面当前编辑的项目
	Action_TypeDef *pCurEditAction;  //项目编辑页面当前编辑的动作

	Action_TypeDef *pCurJumptoAction;  //暂停页面的跳转选择动作

	uint8 backflowPumpSel;  //回流页面的泵选择按钮状态
	uint8 purgePumpSel;  //清洗页面的泵选择按钮状态
	uint8 pumpSelPumpSel;  //泵选择页面的泵选择按钮状态

	uint8 caliPumpSel;  //校准页面的泵选择按钮状态
	uint8 caliFlag;
	uint32 *pCaliPumpTime;  //实际的校准泵参数列表
	float *pCaliPumpPara;  //实际的校准泵参数列表
	float caliAmount;  //校准页面实际加注量的输入临时参数

	//动作编辑页面的输入临时参数
	Pump_TypeDef pump;//泵编号
	Tips_TypeDef tips;//提示
	Voice_TypeDef voice;//声音
	uint8 addAmount;//加注量
	uint8 imbiAmount;//吸液量
	ShakeSpeed_TypeDef shakeSpeed;//摇动速度
	ShakeTime_TypeDef shakeTime;//摇动时间
	uint8 loopTime;//循环次数

	Exception_TypeDef exception;  //异常
	Exception_TypeDef exceptionButtonFlag;  //当暂停或停止按钮按下时该标志位设置为PAUSE/STOP，如果在弹出的对话框中点击了OK才设置exception为PAUSE/STOP

	uint8 rotateFlag;  //用于暂停页面的旋转标志
	uint8 jumpTo;  //用于暂停页面的跳转标志

	TipsButton_TypeDef tipsButton;  //提示页面的按钮状态
	RunningType_TypeDef runningType;  //正在运行的类型

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
