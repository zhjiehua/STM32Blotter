#include "beep.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//蜂鸣器驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
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

/*步进电机参数*/
#define STEPMOTOR_ANGLEPERSTEP  1.8  //步进电机步距角  
#define STEPMOTOR_FREDIV  32  //步进电机分频系数
#define STEPMOTOR_PULSEPERROUND  ((360/STEPMOTOR_ANGLEPERSTEP)*STEPMOTOR_FREDIV)  //步进电机每转需要多少个脉冲

//步进电机1r/s需要配置的定时器周期
#define STEPMOTOR_FREQ_ROUNDPERSEC (72000000/STEPMOTOR_PULSEPERROUND)

//一般步进电机运行速度为600r/min=10r/s，超过1000r/min=16.7r/s会出现力矩急速下降的现象，所以一般不会配置这么高的速度
#define STEPMOTOR_TIMER_CNT(r) (STEPMOTOR_FREQ_ROUNDPERSEC/(r))
//#define STEPMOTOR_TIMER_CNT(r) (72000000/(STEPMOTOR_PULSEPERROUND*(r)))

#define SPEED_CONST  1500//(STEPMOTOR_PULSEPERROUND/17) //用于加减速的步进时间常数 1500

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
//定位速度
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
//定位速度
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
	
	//申请内存空间
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

//设置步进电机方向
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

//设置步进电机速度
void StepMotor_SetSpeed(uint8_t num, uint8_t speedIndex)
{
	stepMotor[num].desSpeedIndex = speedIndex;	 //设置目标速度
}

//设置步进电机脉冲数
//如果调用此命令则进入了脉冲计数模式
void StepMotor_SetPluse(uint8_t num, uint32_t pulseCount)
{
	stepMotor[num].pulseCount = pulseCount;	 //设置脉冲数
	stepMotor[num].status |= 0x80;	//进入脉冲计数模式
}

//设置步进电机命令
void StepMotor_SetCMD(uint8_t num, Status sta)
{
	if(sta == ENABLE)
	{
        if(((stepMotor[num].status)&0x01) == 0)
        {
            stepMotor[num].speedStatus = SPEED_ACC;  //加速
            stepMotor[num].curSpeedIndex = 0;  //起始速度为最小速度
			stepMotor[num].status |= 0x01;
            TIM_SetCounter(StepMotorPin[num].TIMx, stepMotor[num].pSpeedLevel[0].speed);
            TIM_SetCompare2(StepMotorPin[num].TIMx, (stepMotor[num].pSpeedLevel[0].speed)/2);
			GPIO_SetBits(StepMotorPin[0].EN_GPIOx, StepMotorPin[0].EN_GPIO_Pin);
            TIM_Cmd(StepMotorPin[num].TIMx, ENABLE);//启动定时器
        }
		else if(stepMotor[num].curSpeedIndex <= stepMotor[num].desSpeedIndex)
        {
            stepMotor[num].speedStatus = SPEED_ACC;  //加速
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
            stepMotor[num].speedStatus = SPEED_DEC;  //减速，减速到最小速度
        }

        //TIM_Cmd(StepMotorPin[0].TIMx, ENABLE);
		//TR1 = 0;  //停止定时器1
		//pStepMotor->control = 0x00;
	}
}

//设置步进电机停止命令
void StepMotor_Stop(uint8_t num)
{
    //if((((stepMotor[num].status)&0x01) != 0) && stepMotor[num].speedStatus != SPEED_POSOFFSET)
	if((((stepMotor[num].status)&0x01) != 0) && stepMotor[num].speedStatus != SPEED_STOP)
        stepMotor[num].speedStatus = SPEED_STOP;
	
	//TF1 = 1; //进入定时器1中断		
}

//检查步进电机是否已经停止
//返回1,表示已经停止
uint8_t StepMotor_IsStop(uint8_t num)
{
	return !(stepMotor[num].status&0x01);
}

//步进电机初始化
void StepMotor_Init(void)
{
    /*********************初始化步进电机外设***************************************/
    uint8_t i;
    GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
	//创建速度表
	StepMotor_CreateSpeedTable();
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM5, ENABLE);
    
    for(i=0;i<SIZEOF(StepMotorPin);i++)
    {
        GPIO_InitStructure.GPIO_Pin = StepMotorPin[i].EN_GPIO_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
        GPIO_Init(StepMotorPin[i].EN_GPIOx, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin = StepMotorPin[i].DIR_GPIO_Pin;
        GPIO_Init(StepMotorPin[i].DIR_GPIOx, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin = StepMotorPin[i].PWM_GPIO_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
        GPIO_Init(StepMotorPin[i].PWM_GPIOx, &GPIO_InitStructure);
        
        //STEPMOTOR_DEFAULT_PERIOD
        TIM_TimeBaseStructure.TIM_Period = speedLevel[0].speed; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值,如72000000/900=80KHz
        TIM_TimeBaseStructure.TIM_Prescaler = 0; //设置用来作为TIMx时钟频率除数的预分频值 
        TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;  //TIM向上计数模式
        TIM_TimeBaseInit(StepMotorPin[i].TIMx, &TIM_TimeBaseStructure);
        
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2  TIM_OCMode_Timing
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
        TIM_OCInitStructure.TIM_Pulse = STEPMOTOR_DEFAULT_PERIOD/2; //设置待装入捕获比较寄存器的脉冲值
        
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
        
        GPIO_ResetBits(StepMotorPin[i].EN_GPIOx, StepMotorPin[i].EN_GPIO_Pin);  //使能步进驱动器
        GPIO_ResetBits(StepMotorPin[i].DIR_GPIOx, StepMotorPin[i].DIR_GPIO_Pin);
        
        NVIC_InitStructure.NVIC_IRQChannel = StepMotorPin[i].NVIC_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
        NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
        
        TIM_ARRPreloadConfig(StepMotorPin[i].TIMx, ENABLE);
        TIM_ITConfig(StepMotorPin[i].TIMx, TIM_IT_Update, ENABLE);
        //TIM_Cmd(StepMotorPin[i].TIMx, ENABLE);
		
		/****************初始化步进电机结构体***********************************/
		stepMotor[i].offset = pMotorMan->motorParaWastePumpCalib;
		stepMotor[i].speedStatus = SPEED_NONE;
		stepMotor[i].pSpeedLevel = speedLevel;
		stepMotor[i].curSpeedIndex = 0;
		stepMotor[i].desSpeedIndex = 0;
		stepMotor[i].status = 0;
		stepMotor[i].direction = CW;
		
		stepMotor[i].control = 0x00;
    }
    //不使用步进电机2中断
    //TIM_ITConfig(StepMotorPin[1].TIMx, TIM_IT_Update, DISABLE);
    
	GPIO_SetBits(StepMotorPin[0].EN_GPIOx, StepMotorPin[0].EN_GPIO_Pin);  //使能步进驱动器
	GPIO_SetBits(StepMotorPin[1].EN_GPIOx, StepMotorPin[1].EN_GPIO_Pin);  //使能步进驱动器
	
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
            case SPEED_ACC: //加速
            {
                cnt++;
                if(cnt >= stepMotor[1].pSpeedLevel[stepMotor[1].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[1].curSpeedIndex++;
                }
        
                if(stepMotor[1].curSpeedIndex >= stepMotor[1].desSpeedIndex) //加速完成
                {
                    stepMotor[1].curSpeedIndex = stepMotor[1].desSpeedIndex;
                    stepMotor[1].speedStatus = SPEED_NONE;
                }		
            }
            break;
            case SPEED_DEC: //减速
            {
                cnt++;
                if(cnt >= stepMotor[1].pSpeedLevel[stepMotor[1].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[1].curSpeedIndex--;
                }
        
                if(stepMotor[1].curSpeedIndex <= stepMotor[1].desSpeedIndex)  //减速到目标速度
                {
                    stepMotor[1].curSpeedIndex = stepMotor[1].desSpeedIndex;
                    stepMotor[1].speedStatus = SPEED_NONE;
                    
                    cnt = 0;
                    //TR1 = 0;  //停止定时器1
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
					stepMotor[1].status &= ~0x81; //停止电机，强制退出脉冲计数模式
					
                    //停止定时器1
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
        
                if(stepMotor[1].curSpeedIndex <= 0)  //减速到最小速度
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
				
				//停止定时器1
				//GPIO_ResetBits(StepMotorPin[1].EN_GPIOx, StepMotorPin[1].EN_GPIO_Pin);
				TIM_Cmd(TIM2, DISABLE);
			}
		}
        //更新定时器
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
            case SPEED_ACC: //加速
            {
                cnt++;
                if(cnt >= stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[0].curSpeedIndex++;
                }
        
                if(stepMotor[0].curSpeedIndex >= stepMotor[0].desSpeedIndex) //加速完成
                {
                    stepMotor[0].curSpeedIndex = stepMotor[0].desSpeedIndex;
                    stepMotor[0].speedStatus = SPEED_NONE;
                }		
            }
            break;
            case SPEED_DEC: //减速
            {
                cnt++;
                if(cnt >= stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speedConst)
                {
                    cnt = 0;
                    stepMotor[0].curSpeedIndex--;
                }
        
                if(stepMotor[0].curSpeedIndex <= stepMotor[0].desSpeedIndex)  //减速到目标速度
                {
                    stepMotor[0].curSpeedIndex = stepMotor[0].desSpeedIndex;
                    stepMotor[0].speedStatus = SPEED_NONE;
                    
                    cnt = 0;
                    //TR1 = 0;  //停止定时器1
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
//                if(stepMotor[0].curSpeedIndex <= 0)  //减速到最小速度
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
					stepMotor[0].status &= ~0x81; //停止电机，强制退出脉冲计数模式
					
                    //停止定时器1
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
        
                if(stepMotor[0].curSpeedIndex <= 0)  //减速到最小速度
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
				
				//停止定时器1
				//GPIO_ResetBits(StepMotorPin[0].EN_GPIOx, StepMotorPin[0].EN_GPIO_Pin);
				TIM_Cmd(TIM5, DISABLE);
			}
		}
        //更新定时器
        TIM_SetCounter(TIM5, stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speed);
        TIM_SetCompare2(TIM5, (stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speed)/2);
        
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}
