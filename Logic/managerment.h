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

typedef enum
{
	TIPSSOURCE_NONE,

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
	TIPS_NONE,
	TIPS_CANCEL,
	TIPS_OK,
}TipsButton_TypeDef;

typedef enum
{
	EXCEPTION_NONE,
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
	PROJECTSTATUS_IDLE,
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

	Language_TypeDef lang;
	uint16 posCali1; //��Һ��λ��У׼����
	uint16 posCali2; //�䶯��λ��У׼����
}ProjectMan_TypeDef;

/************************************************************************/
/*                                                                      */
/************************************************************************/

extern ProjectMan_TypeDef *pProjectMan;
extern uint8 cmd_buffer[CMD_MAX_SIZE];

void initProjectMan(ProjectMan_TypeDef *pm);
void initUI(void);

//void uartInterrupt(uint8 data);
//void loopForever(void);

#ifdef __cplusplus
}
#endif

#endif
