#include "beep.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

#include <stdlib.h>
//#include <malloc.h>

#include "sys.h"
#include "StepMotor/StepMotor.h"
#include "Beep/beep.h"
#include "LED/LED.h"
#include "usart.h"
#include "PhotoelectricSensor/PhSensor.h"
#include "24CXX/24cxx.h"
#include "stdlib.h"
#include "CPrintf.h"
#include "../Logic/managerment.h"
#include "../Logic/motorManagerment.h"
#include "FreeRTOS.h"
#include "task.h"

/*�����������*/
#define STEPMOTOR_ANGLEPERSTEP  1.8  //������������  
#define STEPMOTOR_FREDIV  32  //���������Ƶϵ��
#define STEPMOTOR_PULSEPERROUND  ((360/STEPMOTOR_ANGLEPERSTEP)*STEPMOTOR_FREDIV)  //�������ÿת��Ҫ���ٸ�����

//�������1r/s��Ҫ���õĶ�ʱ������
#define STEPMOTOR_FREQ_ROUNDPERSEC (72000000/STEPMOTOR_PULSEPERROUND)

//һ�㲽����������ٶ�Ϊ600r/min=10r/s������1000r/min=16.7r/s��������ؼ����½�����������һ�㲻��������ô�ߵ��ٶ�
#define STEPMOTOR_TIMER_CNT(r) (STEPMOTOR_FREQ_ROUNDPERSEC/(r))
//#define STEPMOTOR_TIMER_CNT(r) (72000000/(STEPMOTOR_PULSEPERROUND*(r)))

#define SPEED_CONST  1500//(STEPMOTOR_PULSEPERROUND/17) //���ڼӼ��ٵĲ���ʱ�䳣�� 1500

#define STEPMOTOR_COUNT 2
#define STEPMOTOR_DEFAULT_PERIOD STEPMOTOR_TIMER_CNT(0.1)

#define STEP1_EN    PAout(0) 
#define STEP1_DIR   PAout(2)
#define STEP1_PWM   PAout(1)

#define STEP2_EN    PAout(5) 
#define STEP2_DIR   PAout(4)
#define STEP2_PWM   PAout(3)

static StepMotor_TypeDef stepMotor[STEPMOTOR_COUNT];
StepMotor_TypeDef *pStepMotor = stepMotor;

const StepMotorPin_TypeDef StepMotorPin[] = 
{
    {GPIOA, GPIO_Pin_1, TIM5, TIM_Channel_2, TIM5_IRQn, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_0},//"StepMotor1"
    {GPIOA, GPIO_Pin_3, TIM2, TIM_Channel_4, TIM2_IRQn, GPIOA, GPIO_Pin_4, GPIOA, GPIO_Pin_5},//"StepMotor2"
};

#if 1
//��λ�ٶ�
//const SpeedLevel_TypeDef speedLevel[] = {
//	{STEPMOTOR_TIMER_CNT(0.01), 0.01*SPEED_CONST},   //0
//	{STEPMOTOR_TIMER_CNT(0.03), 0.03*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.06), 0.06*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.09), 0.09*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.12), 0.12*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.15), 0.15*SPEED_CONST},	//5
//	{STEPMOTOR_TIMER_CNT(0.18), 0.18*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.21), 0.21*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.24), 0.24*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.27), 0.27*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.30), 0.30*SPEED_CONST},	//10
//	{STEPMOTOR_TIMER_CNT(0.33), 0.33*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.36), 0.36*SPEED_CONST},
//	{STEPMOTOR_TIMER_CNT(0.39), 0.39*SPEED_CONST},
//};

SpeedLevel_TypeDef *speedLevel;
#else
//��λ�ٶ�
const SpeedLevel_TypeDef speedLevel[] = {
	{STEPMOTOR_TIMER_CNT(0.10), 0.10*SPEED_CONST},   //0
	{STEPMOTOR_TIMER_CNT(0.15), 0.15*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(0.20), 0.20*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(0.25), 0.25*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(0.40), 0.40*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(0.60), 0.60*SPEED_CONST},	//5
	{STEPMOTOR_TIMER_CNT(0.80), 0.80*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(1.00), 1.00*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(1.20), 1.20*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(1.40), 1.40*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(1.60), 1.60*SPEED_CONST},	//10
	{STEPMOTOR_TIMER_CNT(1.80), 1.80*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(2.00), 2.00*SPEED_CONST},
	{STEPMOTOR_TIMER_CNT(2.20), 2.20*SPEED_CONST},
};
#endif

#if 1

#define ACCERALATION	3000
#define SPEED_START		2000
#define SPEED_END		8000

void StepMotor_CreateSpeedTable(void)
{
	uint16_t i = 0;
	float startSpeed = pMotorMan->motorParaStartFreq;
	float endSpeed = pMotorMan->motorParaEndFreq;
	//float stepTime = 2.0/startSpeed;
	float stepTime = (float)(pMotorMan->motorParaStepTime)/1000.0;
	float time = 0;
	float speed = 0;
	
	//_init_alloc(HEAP_BASE,HEAP_END); 
	while(speed < endSpeed)
	{
		speed = startSpeed + pMotorMan->motorParaAccSpeed * time;
		i++;
		time += stepTime;
	}
	i--;
	
	//�����ڴ�ռ�
	speedLevel = (SpeedLevel_TypeDef*)malloc(sizeof(SpeedLevel_TypeDef)*i);
	
	i = 0;
	time = 0;
	speed = 0;
	while(speed < endSpeed)
	{
		speed = startSpeed + pMotorMan->motorParaAccSpeed * time;
		speedLevel[i].speed = 72000000/speed;
		speedLevel[i].speedConst = stepTime * speed;
		
		//cDebug("speedLevel[%d].speed = %f\t\n", i, speedLevel[i].speed);
		//cDebug("speedLevel[%d].speedConst = %f\t\n", i, speedLevel[i].speedConst);
		
		i++;
		time += stepTime;
	}
	i--;
	
	pMotorMan->motorParaSpeedLevel = i;
	//AT24CXX_Write(PARASPEEDLEVEL_BASEADDR, (uint8_t*)(&pMotorMan->motorParaSpeedLevel), 2);
	
	//cDebug("\nspeedLevel cnt = %d\t\n", i);
	//cDebug("\nstepTime = %f\t\n", stepTime);
}
#else
void StepMotor_CreateSpeedTable(void)
{
	uint8_t i;
	for(i=0;i<SPEDD_LEVEL_CNT;i++)
	{
		speedLevel[i].speed = STEPMOTOR_TIMER_CNT(0.1+(i+1)/7);
		speedLevel[i].speedConst = (i+1)/7*SPEED_CONST;
	}
}
#endif

//���ò����������
void StepMotor_SetDir(uint8_t num, Direction_TypeDef dir)
{
	if(dir == CW)
	{
		if(num == 0)
			STEP1_DIR = 0;
		else if(num == 1)
			STEP2_DIR = 0;
		stepMotor[num].direction = CW;
	}
	else
	{
		if(num == 0)
			STEP1_DIR = 1;
		else if(num == 1)
			STEP2_DIR = 1;
		stepMotor[num].direction = CCW;
	}
}

//���ò�������ٶ�
void StepMotor_SetSpeed(uint8_t num, uint8_t speedIndex)
{
	stepMotor[num].desSpeedIndex = speedIndex;	 //����Ŀ���ٶ�
}

//���ò������������
//������ô�������������������ģʽ
void StepMotor_SetPluse(uint8_t num, uint32_t pulseCount)
{
	stepMotor[num].pulseCount = pulseCount;	 //����������
	stepMotor[num].status |= 0x80;	//�����������ģʽ
}

//���ò����������
void StepMotor_SetCMD(uint8_t num, Status sta)
{
	if(sta == ENABLE)
	{
        if(((stepMotor[num].status)&0x01) == 0)
        {
            stepMotor[num].speedStatus = SPEED_ACC;  //����
            stepMotor[num].curSpeedIndex = 0;  //��ʼ�ٶ�Ϊ��С�ٶ�
			stepMotor[num].status |= 0x01;
            TIM_SetCounter(StepMotorPin[num].TIMx, stepMotor[num].pSpeedLevel[0].speed);
            TIM_SetCompare2(StepMotorPin[num].TIMx, (stepMotor[num].pSpeedLevel[0].speed)/2);
			GPIO_SetBits(StepMotorPin[0].EN_GPIOx, StepMotorPin[0].EN_GPIO_Pin);
            TIM_Cmd(StepMotorPin[num].TIMx, ENABLE);//������ʱ��
        }
		else if(stepMotor[num].curSpeedIndex <= stepMotor[num].desSpeedIndex)
        {
            stepMotor[num].speedStatus = SPEED_ACC;  //����
        }
        else
        {
            stepMotor[num].speedStatus = SPEED_DEC;
        }		
	}
	else
	{
        if(((stepMotor[num].status)&0x01) != 0)
        {
            stepMotor[num].desSpeedIndex = 0;
            stepMotor[num].speedStatus = SPEED_DEC;  //���٣����ٵ���С�ٶ�
        }

        //TIM_Cmd(StepMotorPin[0].TIMx, ENABLE);
		//TR1 = 0;  //ֹͣ��ʱ��1
		//pStepMotor->control = 0x00;
	}
}

//���ò������ֹͣ����
void StepMotor_Stop(uint8_t num)
{
    //if((((stepMotor[num].status)&0x01) != 0) && stepMotor[num].speedStatus != SPEED_POSOFFSET)
	if((((stepMotor[num].status)&0x01) != 0) && stepMotor[num].speedStatus != SPEED_STOP)
        stepMotor[num].speedStatus = SPEED_STOP;
	
	//TF1 = 1; //���붨ʱ��1�ж�		
}

//��鲽������Ƿ��Ѿ�ֹͣ
//����1,��ʾ�Ѿ�ֹͣ
uint8_t StepMotor_IsStop(uint8_t num)
{
	return !(stepMotor[num].status&0x01);
}

//���������ʼ��
void StepMotor_Init(void)
{
    /*********************��ʼ�������������***************************************/
    uint8_t i;
    GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
	//�����ٶȱ�
	StepMotor_CreateSpeedTable();
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM5, ENABLE);
    
    for(i=0;i<SIZEOF(StepMotorPin);i++)
    {
        GPIO_InitStructure.GPIO_Pin = StepMotorPin[i].EN_GPIO_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
        GPIO_Init(StepMotorPin[i].EN_GPIOx, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin = StepMotorPin[i].DIR_GPIO_Pin;
        GPIO_Init(StepMotorPin[i].DIR_GPIOx, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin = StepMotorPin[i].PWM_GPIO_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //�����������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
        GPIO_Init(StepMotorPin[i].PWM_GPIOx, &GPIO_InitStructure);
        
        //STEPMOTOR_DEFAULT_PERIOD
        TIM_TimeBaseStructure.TIM_Period = speedLevel[0].speed; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ,��72000000/900=80KHz
        TIM_TimeBaseStructure.TIM_Prescaler = 0; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
        TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;  //TIM���ϼ���ģʽ
        TIM_TimeBaseInit(StepMotorPin[i].TIMx, &TIM_TimeBaseStructure);
        
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2  TIM_OCMode_Timing
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
        TIM_OCInitStructure.TIM_Pulse = STEPMOTOR_DEFAULT_PERIOD/2; //���ô�װ�벶��ȽϼĴ���������ֵ
        
        switch(StepMotorPin[i].TIM_Channel)
        {
            case TIM_Channel_1:
                TIM_OC1Init(StepMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(StepMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
            case TIM_Channel_2:
                TIM_OC2Init(StepMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(StepMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
            case TIM_Channel_3:
                TIM_OC3Init(StepMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(StepMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
            case TIM_Channel_4:
                TIM_OC4Init(StepMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(StepMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
        }
        
        GPIO_ResetBits(StepMotorPin[i].EN_GPIOx, StepMotorPin[i].EN_GPIO_Pin);  //ʹ�ܲ���������
        GPIO_ResetBits(StepMotorPin[i].DIR_GPIOx, StepMotorPin[i].DIR_GPIO_Pin);
        
        NVIC_InitStructure.NVIC_IRQChannel = StepMotorPin[i].NVIC_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
        NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
        
        TIM_ARRPreloadConfig(StepMotorPin[i].TIMx, ENABLE);
        TIM_ITConfig(StepMotorPin[i].TIMx, TIM_IT_Update, ENABLE);
        //TIM_Cmd(StepMotorPin[i].TIMx, ENABLE);
		
		/****************��ʼ����������ṹ��***********************************/
		stepMotor[i].offset = pMotorMan->motorParaWastePumpCalib;
		stepMotor[i].speedStatus = SPEED_NONE;
		stepMotor[i].pSpeedLevel = speedLevel;
		stepMotor[i].curSpeedIndex = 0;
		stepMotor[i].desSpeedIndex = 0;
		stepMotor[i].status = 0;
		stepMotor[i].direction = CW;
		
		stepMotor[i].control = 0x00;
    }
    //��ʹ�ò������2�ж�
    //TIM_ITConfig(StepMotorPin[1].TIMx, TIM_IT_Update, DISABLE);
    
	GPIO_SetBits(StepMotorPin[0].EN_GPIOx, StepMotorPin[0].EN_GPIO_Pin);  //ʹ�ܲ���������
	GPIO_SetBits(StepMotorPin[1].EN_GPIOx, StepMotorPin[1].EN_GPIO_Pin);  //ʹ�ܲ���������
	
//    for(i=0;i<SIZEOF(speedLevel);i++)
//    {
//        printf("speedLevel[%d].speed = %d\t\n", i, speedLevel[i].speed);
//        printf("speedLevel[%d].speedConst = %d\t\n", i, speedLevel[i].speedConst);
//    }
}

void TIM2_IRQHandler(void)
{
	static uint16_t cnt = 0;
	static uint16_t offset = 0;
	static uint32_t pluseCount = 0;
    uint32_t oldBasePri = portSET_INTERRUPT_MASK_FROM_ISR();
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
		 switch(stepMotor[1].speedStatus)
		 {
            case SPEED_ACC: //����
            {
                cnt++;
                if(cnt >= stepMotor[1].pSpeedLevel[stepMotor[1].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[1].curSpeedIndex++;
                }
        
                if(stepMotor[1].curSpeedIndex >= stepMotor[1].desSpeedIndex) //�������
                {
                    stepMotor[1].curSpeedIndex = stepMotor[1].desSpeedIndex;
                    stepMotor[1].speedStatus = SPEED_NONE;
                }		
            }
            break;
            case SPEED_DEC: //����
            {
                cnt++;
                if(cnt >= stepMotor[1].pSpeedLevel[stepMotor[1].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[1].curSpeedIndex--;
                }
        
                if(stepMotor[1].curSpeedIndex <= stepMotor[1].desSpeedIndex)  //���ٵ�Ŀ���ٶ�
                {
                    stepMotor[1].curSpeedIndex = stepMotor[1].desSpeedIndex;
                    stepMotor[1].speedStatus = SPEED_NONE;
                    
                    cnt = 0;
                    //TR1 = 0;  //ֹͣ��ʱ��1
                    //TIM_Cmd(StepMotorPin[1].TIMx, DISABLE);
                }
            }
            break;
            case SPEED_STOP: 
            {
                offset++;
                if(offset++ >= stepMotor[1].offset)
                {
					offset = 0;
                    cnt = 0;
                    stepMotor[1].curSpeedIndex = 0;
                    stepMotor[1].speedStatus = SPEED_NONE;
					//stepMotor[0].status &= ~0x01;
					
					pluseCount = 0;
					stepMotor[1].status &= ~0x81; //ֹͣ�����ǿ���˳��������ģʽ
					
                    //ֹͣ��ʱ��1
					//GPIO_ResetBits(StepMotorPin[1].EN_GPIOx, StepMotorPin[1].EN_GPIO_Pin);
                    TIM_Cmd(TIM2, DISABLE);
                    //cDebug("interrupt---stop the timer5\n");
                    break;	
                }

                cnt++;
                if(cnt >= stepMotor[1].pSpeedLevel[stepMotor[1].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[1].curSpeedIndex--;
                }
        
                if(stepMotor[1].curSpeedIndex <= 0)  //���ٵ���С�ٶ�
                {
                    stepMotor[1].curSpeedIndex = 0;
                    cnt = 0;
                }
            }
            break;
            case SPEED_NONE:
            break;
            default:
            break;
        }

		if(stepMotor[1].status & 0x80)
		{
			pluseCount++;
			if(pluseCount >= stepMotor[1].pulseCount)
			{
				pluseCount = 0;
				offset = 0;
				cnt = 0;
				
				stepMotor[1].status &= ~0x81;
				stepMotor[1].curSpeedIndex = 0;
				stepMotor[1].speedStatus = SPEED_NONE;
				
				//ֹͣ��ʱ��1
				//GPIO_ResetBits(StepMotorPin[1].EN_GPIOx, StepMotorPin[1].EN_GPIO_Pin);
				TIM_Cmd(TIM2, DISABLE);
			}
		}
        //���¶�ʱ��
        TIM_SetCounter(TIM2, stepMotor[1].pSpeedLevel[stepMotor[1].curSpeedIndex].speed);
        TIM_SetCompare2(TIM2, (stepMotor[1].pSpeedLevel[stepMotor[1].curSpeedIndex].speed)/2);
		
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}

/* Timer5 interrupt routine */
void TIM5_IRQHandler(void)
{
	static uint16_t cnt = 0;
	static uint16_t offset = 0;
	static uint32_t pluseCount = 0;
    uint32_t oldBasePri = portSET_INTERRUPT_MASK_FROM_ISR();
    
    if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)
    {
        switch(stepMotor[0].speedStatus)
        {				  
            case SPEED_ACC: //����
            {
                cnt++;
                if(cnt >= stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[0].curSpeedIndex++;
                }
        
                if(stepMotor[0].curSpeedIndex >= stepMotor[0].desSpeedIndex) //�������
                {
                    stepMotor[0].curSpeedIndex = stepMotor[0].desSpeedIndex;
                    stepMotor[0].speedStatus = SPEED_NONE;
                }		
            }
            break;
            case SPEED_DEC: //����
            {
                cnt++;
                if(cnt >= stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[0].curSpeedIndex--;
                }
        
                if(stepMotor[0].curSpeedIndex <= stepMotor[0].desSpeedIndex)  //���ٵ�Ŀ���ٶ�
                {
                    stepMotor[0].curSpeedIndex = stepMotor[0].desSpeedIndex;
                    stepMotor[0].speedStatus = SPEED_NONE;
                    
                    cnt = 0;
                    //TR1 = 0;  //ֹͣ��ʱ��1
                    //TIM_Cmd(StepMotorPin[0].TIMx, DISABLE);
                }
            }
            break;
            case SPEED_STOP: 
            {
//                cnt++;
//                if(cnt >= stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speedConst)
//                {
//                    cnt = 0;
//                    stepMotor[0].curSpeedIndex--;
//                }
//        
//                if(stepMotor[0].curSpeedIndex <= 0)  //���ٵ���С�ٶ�
//                {
//                    stepMotor[0].curSpeedIndex = 0;			
//                    cnt = 0;
//                }
//                stepMotor[0].speedStatus = SPEED_POSOFFSET;
//            }
//            break;
//            case SPEED_POSOFFSET:
//            {
                offset++;
                if(offset++ >= stepMotor[0].offset)
                {
					offset = 0;
                    cnt = 0;
                    stepMotor[0].curSpeedIndex = 0;
                    stepMotor[0].speedStatus = SPEED_NONE;
					//stepMotor[0].status &= ~0x01;
					
					pluseCount = 0;
					stepMotor[0].status &= ~0x81; //ֹͣ�����ǿ���˳��������ģʽ
					
                    //ֹͣ��ʱ��1
					//GPIO_ResetBits(StepMotorPin[0].EN_GPIOx, StepMotorPin[0].EN_GPIO_Pin);
                    TIM_Cmd(TIM5, DISABLE);
                    //cDebug("interrupt---stop the timer5\n");
                    break;	
                }

                cnt++;
                if(cnt >= stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[0].curSpeedIndex--;
                }
        
                if(stepMotor[0].curSpeedIndex <= 0)  //���ٵ���С�ٶ�
                {
                    stepMotor[0].curSpeedIndex = 0;
                    cnt = 0;
                }
            }
            break;
            case SPEED_NONE:
            break;
            default:
            break;
        }

		if(stepMotor[0].status & 0x80)
		{
			pluseCount++;
			if(pluseCount >= stepMotor[0].pulseCount)
			{
				pluseCount = 0;
				offset = 0;
				cnt = 0;
				
				stepMotor[0].status &= ~0x81;
				stepMotor[0].curSpeedIndex = 0;
				stepMotor[0].speedStatus = SPEED_NONE;
				
				//ֹͣ��ʱ��1
				//GPIO_ResetBits(StepMotorPin[0].EN_GPIOx, StepMotorPin[0].EN_GPIO_Pin);
				TIM_Cmd(TIM5, DISABLE);
			}
		}
        //���¶�ʱ��
        TIM_SetCounter(TIM5, stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speed);
        TIM_SetCompare2(TIM5, (stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speed)/2);
        
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}
