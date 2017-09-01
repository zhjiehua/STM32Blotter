/*

*/

#ifndef __STEPMOTOR_H__
#define __STEPMOTOR_H__
#include "../HardwareCommon.h"
#include "../PhotoelectricSensor/PhSensor.h"

#define STEPMOTOR_MAIN 0
#define STEPMOTOR_SLAVE 1

typedef struct
{
//	uint16_t speed;
//	uint16_t speedConst;
	float speed;
	float speedConst;
}SpeedLevel_TypeDef;

typedef enum
{
	SPEED_NONE,	//静止或者匀速中
	SPEED_ACC,  //加速
	SPEED_DEC,	//减速
	SPEED_STOP, //用于立即停止步进电机？？
	SPEED_POSOFFSET,  //电机不能在传感器位置停止，这样当出现装盘晃动时会导致传感器检测到抖动
}SpeedStatus_TypeDef;

typedef struct
{
    GPIO_TypeDef* PWM_GPIOx;
    uint16_t PWM_GPIO_Pin;
    
    TIM_TypeDef* TIMx;   
    uint16_t TIM_Channel;
    
    uint8_t NVIC_IRQChannel;
    
    /*****************************/
    GPIO_TypeDef* DIR_GPIOx;
    uint16_t DIR_GPIO_Pin;
    
    /*****************************/
    GPIO_TypeDef* EN_GPIOx;
    uint16_t EN_GPIO_Pin;

}StepMotorPin_TypeDef;

typedef struct
{
	uint16_t offset;  //位置偏移，用于校正转盘，默认450
	uint32_t pulseCount;	//走多少个脉冲
	
	Direction_TypeDef direction;  //记录方向

	SpeedStatus_TypeDef speedStatus;
	const SpeedLevel_TypeDef *pSpeedLevel;  //速度级别数组
	uint16_t curSpeedIndex; //当前速度，用于加减速
	uint16_t desSpeedIndex; //目标速度

    uint8_t status;		//bit8 : 1,进入脉冲技术模式; 0,进入普通模式
						//bit1 : 1,电机正在运行中; 0,电机已经停止
	
	uint8_t control;  //步进电机使能控制
}StepMotor_TypeDef;

extern StepMotor_TypeDef *pStepMotor;
extern SpeedLevel_TypeDef *speedLevel;
extern const StepMotorPin_TypeDef StepMotorPin[];

void StepMotor_CreateSpeedTable(void);
void StepMotor_SetDir(uint8_t num, Direction_TypeDef dir);
void StepMotor_SetSpeed(uint8_t num, uint8_t speedIndex);
void StepMotor_SetPluse(uint8_t num, uint32_t pulseCount);
void StepMotor_SetCMD(uint8_t num, Status sta);
void StepMotor_Stop(uint8_t num);
uint8_t StepMotor_IsStop(uint8_t num);

void StepMotor_Init(void);

#endif
