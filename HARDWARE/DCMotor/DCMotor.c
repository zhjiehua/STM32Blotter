#include "../DCMotor/DCMotor.h"
#include "../PhotoelectricSensor/PhSensor.h"
#include "../RelayMOS/RelayMOS.h"
#include "usart.h"
#include "CPrintf.h"
#include "StepMotor/StepMotor.h"
#include "NTCResistor/NTCResistor.h"

#include "../Logic/managerment.h"

#include "FreeRTOS.h"
#include "task.h"

static DCMotor_TypeDef dcMotor[DCMOTOR_COUNT];
DCMotor_TypeDef *pDCMotor = dcMotor;

//ֱ�������Ƶ���������13kHz~15kHz֮�䣬��������Ϊ14kHz
const DCMotorPin_TypeDef DCMotorPin[] =
{
    {GPIOD, GPIO_Pin_15, GPIO_Remap_TIM4, TIM4, TIM_Channel_4, GPIOB, GPIO_Pin_10},//"DCMotor1"
    {GPIOD, GPIO_Pin_14, GPIO_Remap_TIM4, TIM4, TIM_Channel_3, GPIOB, GPIO_Pin_11},//"DCMotor2"
    {GPIOD, GPIO_Pin_13, GPIO_Remap_TIM4, TIM4, TIM_Channel_2, GPIOB, GPIO_Pin_12},//"DCMotor3"
    {GPIOD, GPIO_Pin_12, GPIO_Remap_TIM4, TIM4, TIM_Channel_1, GPIOB, GPIO_Pin_13},//"DCMotor4"
    {GPIOE, GPIO_Pin_14, GPIO_FullRemap_TIM1, TIM1, TIM_Channel_4, GPIOE, GPIO_Pin_15},//"DCMotor5"
    {GPIOE, GPIO_Pin_13, GPIO_FullRemap_TIM1, TIM1, TIM_Channel_3, GPIOE, GPIO_Pin_12},//"DCMotor6"
    {GPIOE, GPIO_Pin_11, GPIO_FullRemap_TIM1, TIM1, TIM_Channel_2, GPIOE, GPIO_Pin_10},//"DCMotor7"
    {GPIOE, GPIO_Pin_9 , GPIO_FullRemap_TIM1, TIM1, TIM_Channel_1, GPIOE, GPIO_Pin_8},//"DCMotor8"
    {GPIOB, GPIO_Pin_1 , 0, TIM3, TIM_Channel_4, GPIOE, GPIO_Pin_7},//"DCMotor9"
    {GPIOB, GPIO_Pin_0 , 0, TIM3, TIM_Channel_3, GPIOB, GPIO_Pin_2},//"DCMotor10"
    {GPIOA, GPIO_Pin_7 , 0, TIM3, TIM_Channel_2, GPIOC, GPIO_Pin_5},//"DCMotor11"
    {GPIOA, GPIO_Pin_6 , 0, TIM3, TIM_Channel_1, GPIOC, GPIO_Pin_4},//"DCMotor12"
};

//����ֱ��������ٶ�
//��ʵ������PWM��ռ�ձȣ�0~100,100Ϊ���ռ�ձ�
void DCMotor_SetSpeed(DCMotorEnum_TypeDef num, uint8_t speed)
{
    dcMotor[num].speed = speed;
    
	if(speed > 100)
		speed = 100;
	
    if(dcMotor[num].direction == CCW)
        dcMotor[num].timPulse = (uint16_t)((float)speed/100.0*(float)(dcMotor[num].timPeriod));
    else
        dcMotor[num].timPulse = (uint16_t)((float)(100-speed)/100.0*(float)(dcMotor[num].timPeriod));
    
    switch(DCMotorPin[num].TIM_Channel)
    {
        case TIM_Channel_1:
            TIM_SetCompare1(DCMotorPin[num].TIMx, dcMotor[num].timPulse);
            break;
        case TIM_Channel_2:
            TIM_SetCompare2(DCMotorPin[num].TIMx, dcMotor[num].timPulse);
            break;
        case TIM_Channel_3:
            TIM_SetCompare3(DCMotorPin[num].TIMx, dcMotor[num].timPulse);
            break;
        case TIM_Channel_4:
            TIM_SetCompare4(DCMotorPin[num].TIMx, dcMotor[num].timPulse);
            break;
        default:
            break;
    }
}

//���÷�Һ�ڵ����ֱ����10���ķ���
void DCMotor_SetDir(DCMotorEnum_TypeDef num, Direction_TypeDef dir)
{
    dcMotor[num].direction = dir;
    if(dir == CW)
        GPIO_ResetBits(DCMotorPin[num].DIR_GPIOx, DCMotorPin[num].DIR_GPIO_Pin);
    else
        GPIO_SetBits(DCMotorPin[num].DIR_GPIOx, DCMotorPin[num].DIR_GPIO_Pin);
}

void DCMotor_Stop(DCMotorEnum_TypeDef num)
{
    DCMotor_SetDir(num, CCW);
    DCMotor_SetSpeed(num, 0);
}

void DCMotor_Run(DCMotorEnum_TypeDef num, Direction_TypeDef dir, uint8_t speed)
{
    DCMotor_SetDir(num, dir);
    DCMotor_SetSpeed(num, speed);
}

//PWM��ʼ��
void DCMotor_Init(void)
{ 
	uint8_t i;
    GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��  
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1|RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB
                        |RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD
                        |RCC_APB2Periph_GPIOE, ENABLE);
    
    for(i=0;i<SIZEOF(DCMotorPin);i++)
    {
        GPIO_InitStructure.GPIO_Pin = DCMotorPin[i].PWM_GPIO_Pin; //TIM_CH2
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(DCMotorPin[i].PWM_GPIOx, &GPIO_InitStructure);//��ʼ��GPIO
        if(DCMotorPin[i].GPIO_Remap != 0)
            GPIO_PinRemapConfig(DCMotorPin[i].GPIO_Remap, ENABLE);        
        
        GPIO_InitStructure.GPIO_Pin = DCMotorPin[i].DIR_GPIO_Pin; //TIM_CH2
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�����������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(DCMotorPin[i].DIR_GPIOx, &GPIO_InitStructure);//��ʼ��GPIO
        GPIO_SetBits(DCMotorPin[i].DIR_GPIOx, DCMotorPin[i].DIR_GPIO_Pin);//��ʼ��ʱ����ߵ�ƽ
        
        TIM_TimeBaseStructure.TIM_Period = DCMOTOR_DEFAULT_PERIOD; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ,72000000/900=80KHz
        TIM_TimeBaseStructure.TIM_Prescaler = 0; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
        TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
        TIM_TimeBaseInit(DCMotorPin[i].TIMx, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
        
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2  TIM_OCMode_Timing
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
        TIM_OCInitStructure.TIM_Pulse = 0;//���ô�װ�벶��ȽϼĴ���������ֵ, ��ʼ��ʱҪ����ߵ�ƽ
        
        if(DCMotorPin[i].TIMx == TIM1 || DCMotorPin[i].TIMx == TIM8)
        {
            TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;
            TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Disable;
            TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;
            TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;
        }
        switch(DCMotorPin[i].TIM_Channel)
        {
            case TIM_Channel_1:
                TIM_OC1Init(DCMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(DCMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
            case TIM_Channel_2:
                TIM_OC2Init(DCMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(DCMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
            case TIM_Channel_3:
                TIM_OC3Init(DCMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(DCMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
            case TIM_Channel_4:
                TIM_OC4Init(DCMotorPin[i].TIMx, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(DCMotorPin[i].TIMx, TIM_OCPreload_Enable);
                break;
            default:
                break;
        }

        if(DCMotorPin[i].TIMx == TIM1 || DCMotorPin[i].TIMx == TIM8)
        {
            TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
            TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//????????? 
            TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//????????? 
            TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;         //????
            TIM_BDTRInitStructure.TIM_DeadTime = 0x90;                                         //??????
            TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                 //??????
            TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;//??????
            TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;//?????? 
            TIM_BDTRConfig(DCMotorPin[i].TIMx, &TIM_BDTRInitStructure);
			
			TIM_CtrlPWMOutputs(DCMotorPin[i].TIMx, ENABLE);
        }
        
        TIM_ARRPreloadConfig(DCMotorPin[i].TIMx, ENABLE); 
        TIM_Cmd(DCMotorPin[i].TIMx, ENABLE);
        
        dcMotor[i].direction = CW;
        dcMotor[i].speed = 0;
        dcMotor[i].timPulse = 0;
        dcMotor[i].timPeriod = DCMOTOR_DEFAULT_PERIOD;
    }
    
    /****************************************************************************************/
	pDCMotor->control = 0x00;

//	for(i=0;i<SIZEOF(DCMotorPin);i++)
//	{
//		pDCMotor->SetCMD((DCMotorEnum_TypeDef)i, DISABLE); //ֹͣ����늙C
//	}
//	pDCMotor->SetDir(PUMP_WASTE, CW); //��Һ�õ��ֹͣ��ʱ��Ҫ���߷�������

//	pDCMotor->SetCMD(DCMOTOR12, ENABLE);//�йܷ���ʹ�ܣ���ʱ����pDCMotor->SetSpeed(DCMOTOR12, 0);��������pDCMotor->SetSpeed(DCMOTOR12, 100);��
//	pDCMotor->SetSpeed(DCMOTOR12, 100);//�رռйܷ�
}
