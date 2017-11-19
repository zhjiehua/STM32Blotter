#ifndef _PROJECT_H_
#define _PROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../UILogic/pageCommon.h"

#define NAME_SIZE 11  //动作名称或者项目名称的最大长度
#define ACTIONS_PER_PROJECT  21  //每个项目有多少个动作
#define PROJECT_COUNT  3  //项目数量

#define PUMP_COUNT  8  //蠕动泵数量

//
#define ACTION_SIZE		   sizeof(Action_TypeDef)
#define PROJECT_SIZE       (ACTION_SIZE*ACTIONS_PER_PROJECT + NAME_SIZE + 1) 

//EEPROM地址分配
#define RESET_BASEADDR				0
#define LANGUAGE_BASEADDR			(RESET_BASEADDR + 4)
#define WASTEPUMPCALIB_BASEADDR		(LANGUAGE_BASEADDR + 1)
#define CWPUMPCALIB_BASEADDR		(WASTEPUMPCALIB_BASEADDR + 2)
#define CCWPUMPCALIB_BASEADDR		(CWPUMPCALIB_BASEADDR + 2)
#define CALIBPUMPPARA_BASEADDR		(CCWPUMPCALIB_BASEADDR + 2)
#define CALIBPUMPTIME_BASEADDR		(CALIBPUMPPARA_BASEADDR + PUMP_COUNT*4)

#define PARAPUMPSPEED_BASEADDR		(CALIBPUMPTIME_BASEADDR + PUMP_COUNT*4)
#define PARAWASTEPUMPSPEED_BASEADDR	(PARAPUMPSPEED_BASEADDR + 1)
#define PARATTHOMESPEED_BASEADDR	(PARAWASTEPUMPSPEED_BASEADDR + 1)
#define PARATTSTEP1SPEED_BASEADDR	(PARATTHOMESPEED_BASEADDR + 1)
#define PARATTSTEP2SPEED_BASEADDR	(PARATTSTEP1SPEED_BASEADDR + 1)
#define PARATTLOCATIONSPEED_BASEADDR	(PARATTSTEP2SPEED_BASEADDR + 1)
#define PARALOWSPEED_BASEADDR		(PARATTLOCATIONSPEED_BASEADDR + 1)
#define PARAMIDDLESPEED_BASEADDR	(PARALOWSPEED_BASEADDR + 1)
#define PARAFASTSPEED_BASEADDR		(PARAMIDDLESPEED_BASEADDR + 1)

#define PARASTARTFREQ_BASEADDR		(PARAFASTSPEED_BASEADDR + 1)
#define PARAENDFREQ_BASEADDR		(PARASTARTFREQ_BASEADDR + 4)
#define PARAACCSPEED_BASEADDR		(PARAENDFREQ_BASEADDR + 4)
#define PARASTEPTIME_BASEADDR		(PARAACCSPEED_BASEADDR + 4)
#define PARASPEEDLEVEL_BASEADDR		(PARASTEPTIME_BASEADDR + 4)

#define PROJECT_BASEADDR			(PARASPEEDLEVEL_BASEADDR + 2)

#define END_BASEADDR				(PROJECT_BASEADDR + PROJECT_SIZE*PROJECT_COUNT)

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
/* 动作结构体                                                           */
/************************************************************************/
typedef struct
{
	Pump_TypeDef pump;  //泵编号
	Tips_TypeDef tips;  //提示
	Voice_TypeDef voice;  //声音
	float addAmount;  //加注量
	uint8 imbiAmount;  //吸液量
	ShakeSpeed_TypeDef shakeSpeed;  //摇动速度
	ShakeTime_TypeDef shakeTime;  //摇动时间
	uint8 loopTime;  //循环次数
	uint8 index;

	uint8 name[NAME_SIZE];  //动作名
}Action_TypeDef;

/************************************************************************/
/* 项目结构体                                                           */
/************************************************************************/
typedef struct
{
	Action_TypeDef action[ACTIONS_PER_PROJECT];

	uint8 index;
	char name[NAME_SIZE];  //项目名
}Project_TypeDef;

extern Project_TypeDef project[1];
extern float caliPumpPara[PUMP_COUNT];
extern uint32 caliPumpTime[PUMP_COUNT];

void initProjectStruct(Project_TypeDef* pro, uint8 index, uint8 *name);
void initCaliPumpPara(float para, uint32 time);
void wasteFluidAbsorb(uint8_t *pStopFlag);
void ProjectProgram(uint8_t *pStopFlag);
void PurgeProgram(uint8_t *pStopFlag);
void HomeProgram(uint8_t *pStopFlag);
void CalibraProgram(uint8_t *pStopFlag);
void BackFlowProgram(uint8_t *pStopFlag);

#ifdef __cplusplus
}
#endif

#endif
