/*

*/

#ifndef __STEPMOTOR_H__
#define __STEPMOTOR_H__
#include "../HardwareCommon.h"
#include "../PhotoelectricSensor/PhSensor.h"

/*�����������*/
#define STEPMOTOR_ANGLEPERSTEP  1.8  //������������  
#define STEPMOTOR_FREDIV  128  //���������Ƶϵ��
#define STEPMOTOR_PULSEPERROUND  ((360/STEPMOTOR_ANGLEPERSTEP)*STEPMOTOR_FREDIV)  //�������ÿת��Ҫ���ٸ�����

//�������1r/s��Ҫ���õĶ�ʱ������
#define STEPMOTOR_FREQ_ROUNDPERSEC (72000000/STEPMOTOR_PULSEPERROUND)

//һ�㲽����������ٶ�Ϊ600r/min=10r/s������1000r/min=16.7r/s��������ؼ����½�����������һ�㲻��������ô�ߵ��ٶ�
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


#define STEPMOTOR_OFFSET  1400  //��Һ��Ҫ��׼�ۿף���Ҫƫ��

#define STEPMOTOR_PUMP_OFFSET  100 //�䶯�ö�׼

#define SPEED_CONST  1500 //���ڼӼ��ٵĲ���ʱ�䳣��

#if 0
	#define SPEED_POSITION1 3  //��λ����Ϊ1���ٶ�
	#define SPEED_POSITION2 9  //��λ����Ϊ2���ٶ�
	#define SPEDD_POSITION  13  //��λ�������3���ٶ�
	#define SPEDD_HOME	    13  //��ԭ����ٶ�
	#define SPEDD_SLOW	    9  //�����ٶ�-��
	#define SPEDD_MIDDLE    14  //�����ٶ�-��
	#define SPEDD_FAST	    17 //�����ٶ�-��
#else
	#define SPEED_POSITION1 2  //��λ����Ϊ1���ٶ�
	#define SPEED_POSITION2 5  //��λ����Ϊ2���ٶ�
	#define SPEDD_POSITION  8  //��λ�������3���ٶ�
	#define SPEDD_HOME	    8  //��ԭ����ٶ�
	#define SPEDD_SLOW	    5  //�����ٶ�-��
	#define SPEDD_MIDDLE    9  //�����ٶ�-��
	#define SPEDD_FAST	    13 //�����ٶ�-��
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
	SPEED_NONE,	//��ֹ����������
	SPEED_ACC,  //����
	SPEED_DEC,	//����
	SPEED_STOP, //��������ֹͣ�����������
	SPEED_POSOFFSET,  //��������ڴ�����λ��ֹͣ������������װ�̻ζ�ʱ�ᵼ�´�������⵽����
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
	uint8_t curCount;    //��ǰ�ڼ����ź�
	uint8_t desCount; 	 //����ڼ����źź�ֹͣ�����0��ʾ��ͣ	

	int8_t curPos;  //��ǰλ�ã������˳ʱ����ת��++������--
	//int8_t desPos;  //Ŀ��λ��
	uint16_t offset;  //λ��ƫ�ƣ�����У��ת�̣�Ĭ��450

	Direction_TypeDef direction;  //��¼����

	SpeedStatus_TypeDef speedStatus;
	const SpeedLevel_TypeDef *pSpeedLevel;  //�ٶȼ�������
	uint16_t curSpeedIndex; //��ǰ�ٶȣ����ڼӼ���
	uint16_t desSpeedIndex; //Ŀ���ٶ�

    uint8_t status;
	uint8_t control;  //�������ʹ�ܿ���
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
