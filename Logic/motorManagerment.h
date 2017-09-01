#ifndef _MOTORMANAGERMENT_H_
#define _MOTORMANAGERMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../UILogic/pageCommon.h"
#include "project.h"
#include "../HMI/cmd_queue.h"
#include "../HMI/cmd_process.h"
#include "../HMI/hmi_user_uart.h"
#include "../HARDWARE/HardwareCommon.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
	
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#define STEPMOTOR_OFFSET  100  //废液口要对准槽孔，需要偏移
#define STEPMOTOR_PUMP_OFFSET  0  //废液口要对准槽孔，需要偏移
	
#define TANK_COUNT 50
#define TANK_PER_PLATE 5

typedef struct
{
	uint8_t curCount;	//当前第几个信号
	uint8_t desCount;	//计算第几个信号后停止电机，0表示不停
	
	int8_t curPos;	//当前位置，如果是顺时针旋转就++，否则--
	
	StepMotor_TypeDef *turntableStepMotor;
	
	Direction_TypeDef manualRawDir;
	Direction_TypeDef manualPumpDir;
	DCMotorEnum_TypeDef manualPumpNum;
	uint8_t manualPumpAmount;
	int8_t manualTTPos;
	
	uint8_t motorParaPumpSpeed;
	uint8_t motorParaWastePumpSpeed;
	uint8_t motorParaTTHomeSpeed;
	uint8_t motorParaTTStep1Speed;
	uint8_t motorParaTTStep2Speed;
	uint8_t motorParaTTLocationSpeed;
	uint8_t motorParaLowSpeed;
	uint8_t motorParaMiddleSpeed;
	uint8_t motorParaFastSpeed;
	
	uint32_t motorParaStartFreq;
	uint32_t motorParaEndFreq;
	uint32_t motorParaAccSpeed;
	uint32_t motorParaStepTime;
	uint16_t motorParaSpeedLevel;
	
	uint16_t motorParaWastePumpCalib; //废液口位置校准参数
	uint16_t motorParaCWPumpCalib; //蠕动泵位置校准参数
	uint16_t motorParaCCWPumpCalib; //蠕动泵位置校准参数
}MotorMan_TypeDef;

extern const uint8_t AbsCoordinate[];
extern MotorMan_TypeDef *pMotorMan;

void Turntable_SetPos(uint8_t pos);
void Turntable_UpdatePos(void);
uint8_t Turntable_IsOnPos(void);
void Turntable_SetOffset(uint16_t offset);
void Turntable_Home(uint8_t *pStopFlag);
void Turntable_StopAndAlign(uint8_t len, uint8_t *pStopFlag);
uint8_t Turntable_Abs2Rel(uint8_t absCoord);
void Turntable_Position(Direction_TypeDef dir, uint8_t dis, uint8_t *pStopFlag);
void Turntable_RelativePosition(uint8_t desTank, uint8_t srcTank, uint8_t *pStopFlag);

void WastePump_SetPos(Position_TypeDef pos, uint8_t speed, uint8_t *pStopFlag);
	
#ifdef __cplusplus
}
#endif

#endif
