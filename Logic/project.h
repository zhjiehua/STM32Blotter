#ifndef _PROJECT_H_
#define _PROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../UILogic/pageCommon.h"

#define NAME_SIZE 11  //�������ƻ�����Ŀ���Ƶ���󳤶�
#define ACTIONS_PER_PROJECT  21  //ÿ����Ŀ�ж��ٸ�����
#define PROJECT_COUNT  5  //��Ŀ����

#define PUMP_COUNT  8  //�䶯������

//
#define ACTION_SIZE		   sizeof(Action_TypeDef)
#define PROJECT_SIZE       (ACTION_SIZE*ACTIONS_PER_PROJECT + NAME_SIZE + 1) 
#define CALIBPARA_SIZE     (sizeof(float)*PUMP_COUNT)
#define LANGUAGE_SIZE      (1)
#define POSCALI_SIZE       (4)
//EEPROM��ַ����
#define RESET_DEFAULT	   6
#define CALIBPARA_BASEADDR (RESET_DEFAULT+4)
#define LANGUAGE_BASEADDR  (CALIBPARA_BASEADDR + CALIBPARA_SIZE)
#define PROJECT_BASEADDR   (LANGUAGE_BASEADDR + LANGUAGE_SIZE)
#define POSCALI_BASEADDR   (PROJECT_BASEADDR + PROJECT_SIZE*PROJECT_COUNT)

typedef enum
{
	PUMP1 = 0,
	PUMP2,
	PUMP3,
	PUMP4,
	PUMP5,
	PUMP6,
	PUMP7,
	PUMP8,
	PUMP0,
}Pump_TypeDef;

typedef enum
{
	NO_TIPS = 0,
	SAMPLE_TIPS,
	MEMBRANE_TIPS,
}Tips_TypeDef;

typedef enum
{
	NO_VOICE = 0,
	SHORT_VOICE,
	MIDDLE_VOICE,
	LONG_VOICE,
}Voice_TypeDef;

typedef enum
{
	SLOW_SPEED = 0,
	MIDDLE_SPEED,
	FAST_SPEED,
}ShakeSpeed_TypeDef;

typedef struct 
{
	uint8 hour;
	uint8 minute;
}ShakeTime_TypeDef;

/************************************************************************/
/* �����ṹ��                                                           */
/************************************************************************/
typedef struct
{
	Pump_TypeDef pump;  //�ñ��
	Tips_TypeDef tips;  //��ʾ
	Voice_TypeDef voice;  //����
	uint8 addAmount;  //��ע��
	uint8 imbiAmount;  //��Һ��
	ShakeSpeed_TypeDef shakeSpeed;  //ҡ���ٶ�
	ShakeTime_TypeDef shakeTime;  //ҡ��ʱ��
	uint8 loopTime;  //ѭ������
	uint8 index;

	char name[NAME_SIZE];  //������
}Action_TypeDef;

/************************************************************************/
/* ��Ŀ�ṹ��                                                           */
/************************************************************************/
typedef struct
{
	Action_TypeDef action[ACTIONS_PER_PROJECT];

	uint8 index;
	char name[NAME_SIZE];  //��Ŀ��
}Project_TypeDef;

extern Project_TypeDef project[1];
extern float caliPumpPara[PUMP_COUNT];

void initProjectStruct(Project_TypeDef* pro, uint8 index, uint8 *name);
void initCaliPumpPara(float para);
void wasteFluidAbsorb(void);
void projectProgram(void);
void purgeProgram(void);
void homeProgram(void);
void calibraProgram(void);

#ifdef __cplusplus
}
#endif

#endif
