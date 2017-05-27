/*

*/

#ifndef __STEPMOTOR_H__
#define __STEPMOTOR_H__
#include "../HardwareCommon.h"
#include "../PhotoelectricSensor/PhSensor.h"

/*步进电机参数*/
#define STEPMOTOR_ANGLEPERSTEP  1.8  //步进电机步距角  
#define STEPMOTOR_FREDIV  128  //步进电机分频系数
#define STEPMOTOR_PULSEPERROUND  ((360/STEPMOTOR_ANGLEPERSTEP)*STEPMOTOR_FREDIV)  //步进电机每转需要多少个脉冲

//步进电机1r/s需要配置的定时器周期
#define STEPMOTOR_FREQ_ROUNDPERSEC (72000000/STEPMOTOR_PULSEPERROUND)

//一般步进电机运行速度为600r/min=10r/s，超过1000r/min=16.7r/s会出现力矩急速下降的现象，所以一般不会配置这么高的速度
#define STEPMOTOR_FREQ(r) (72000000/(STEPMOTOR_PULSEPERROUND*(r)))

#define STEPMOTOR_MAIN 0
#define STEPMOTOR_SLAVE 1

#define STEPMOTOR_COUNT 2
#define STEPMOTOR_DEFAULT_PERIOD STEPMOTOR_FREQ(0.1)

#define STEP1_EN    PAout(0) 
#define STEP1_DIR   PAout(2)
#define STEP1_PWM   PAout(1)

#define STEP2_EN    PAout(5) 
#define STEP2_DIR   PAout(4)
#define STEP2_PWM   PAout(3)


#define STEPMOTOR_OFFSET  1400  //废液口要对准槽孔，需要偏移

#define STEPMOTOR_PUMP_OFFSET  100 //蠕动泵对准

#define SPEED_CONST  1500 //用于加减速的步进时间常数

#if 0
	#define SPEED_POSITION1 3  //定位距离为1的速度
	#define SPEED_POSITION2 9  //定位距离为2的速度
	#define SPEDD_POSITION  13  //定位距离大于3的速度
	#define SPEDD_HOME	    13  //回原点的速度
	#define SPEDD_SLOW	    9  //孵育速度-慢
	#define SPEDD_MIDDLE    14  //孵育速度-中
	#define SPEDD_FAST	    17 //孵育速度-快
#else
	#define SPEED_POSITION1 2  //定位距离为1的速度
	#define SPEED_POSITION2 5  //定位距离为2的速度
	#define SPEDD_POSITION  8  //定位距离大于3的速度
	#define SPEDD_HOME	    8  //回原点的速度
	#define SPEDD_SLOW	    5  //孵育速度-慢
	#define SPEDD_MIDDLE    9  //孵育速度-中
	#define SPEDD_FAST	    13 //孵育速度-快
#endif

#define TANK_COUNT 50
#define TANK_PER_PLATE 5

typedef struct
{
	uint16_t speed;
	uint16_t speedConst;
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
	uint8_t curCount;    //当前第几个信号
	uint8_t desCount; 	 //计算第几个信号后停止电机，0表示不停	

	int8_t curPos;  //当前位置，如果是顺时针旋转就++，否则--
	//int8_t desPos;  //目标位置
	uint16_t offset;  //位置偏移，用于校正转盘，默认450

	Direction_TypeDef direction;  //记录方向

	SpeedStatus_TypeDef speedStatus;
	const SpeedLevel_TypeDef *pSpeedLevel;  //速度级别数组
	uint16_t curSpeedIndex; //当前速度，用于加减速
	uint16_t desSpeedIndex; //目标速度

    uint8_t status;
	uint8_t control;  //步进电机使能控制
    uint8_t temp;
}StepMotor_TypeDef;

extern StepMotor_TypeDef *pStepMotor;
extern const SpeedLevel_TypeDef speedLevel[];
extern const uint8_t AbsCoordinate[10];
extern const StepMotorPin_TypeDef StepMotorPin[];

void StepMotor_SetDir(uint8_t num, Direction_TypeDef dir);
void StepMotor_SetSpeed(uint8_t num, uint8_t speedIndex);
void StepMotor_SetCMD(uint8_t num, Status sta);
void StepMotor_Stop(uint8_t num);

void StepMotor_StopAndAlign(uint8_t num, uint8_t len);
void StepMotor_SetPos(uint8_t num, uint8_t pos);
void StepMotor_UpdatePos(uint8_t num);
uint8_t StepMotor_IsOnPos(uint8_t num);
uint8_t StepMotor_IsStop(uint8_t num);
void StepMotor_Home(uint8_t num);
uint8_t StepMotor_Abs2Rel(uint8_t num, uint8_t absCoord);
void StepMotor_Position(uint8_t num, Direction_TypeDef dir, uint8_t dis);
void StepMotor_RelativePosition(uint8_t num, uint8_t desTank, uint8_t srcTank);

void StepMotor_Init(void);
void StepMotorTestTask(void);
#endif
