/*

*/
#ifndef __DCMOTOR_H__
#define __DCMOTOR_H__

#include "../HardwareCommon.h"
#include "../PhotoelectricSensor/PhSensor.h"

#define DCMOTOR_COUNT 12
#define DCMOTOR_DEFAULT_PERIOD 5141

#define PUMP_WASTE  DCMOTOR9  //∑œ“∫±√
#define PUMP_PINCH  DCMOTOR10  //º–π‹∑ß
#define PUMP_VACUUM DCMOTOR11   //’Êø’±√

typedef enum
{
	UP = 0,
	DOWN,
}Position_TypeDef;

typedef enum
{
	DCMOTOR1 = 0,
	DCMOTOR2,
	DCMOTOR3,
	DCMOTOR4,
	DCMOTOR5,
	DCMOTOR6,
	DCMOTOR7,
	DCMOTOR8,
	DCMOTOR9,
	DCMOTOR10,  //∑œ“∫±√
	DCMOTOR11,
	DCMOTOR12,
}DCMotorEnum_TypeDef;

typedef struct
{
    GPIO_TypeDef* PWM_GPIOx;
    uint16_t PWM_GPIO_Pin;
    uint32_t GPIO_Remap;
    
    TIM_TypeDef* TIMx;
    uint16_t TIM_Channel;
    
    /*****************************/
    GPIO_TypeDef* DIR_GPIOx;
    uint16_t DIR_GPIO_Pin;
}DCMotorPin_TypeDef;

typedef struct
{
	uint8_t control;
    uint16_t temp;

    Direction_TypeDef direction;
    uint8_t speed;
    uint16_t timPulse;
    uint16_t timPeriod;
}DCMotor_TypeDef;

extern DCMotor_TypeDef *pDCMotor;
extern const DCMotorPin_TypeDef DCMotorPin[];

void DCMotor_SetSpeed(DCMotorEnum_TypeDef num, uint8_t speed);
void DCMotor_SetDir(DCMotorEnum_TypeDef num, Direction_TypeDef dir);
void DCMotor_Stop(DCMotorEnum_TypeDef num);
void DCMotor_Run(DCMotorEnum_TypeDef num, Direction_TypeDef dir, uint8_t speed);

void DCMotor_Init(void);

void DCMotorTestTask(void);

#endif
