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

#include "sys.h"
#include "StepMotor/StepMotor.h"
#include "Beep/beep.h"
#include "LED/LED.h"
#include "usart.h"
#include "PhotoelectricSensor/PhSensor.h"
#include "stdlib.h"
#include "CPrintf.h"
#include "../Logic/managerment.h"

#include "FreeRTOS.h"
#include "task.h"

static StepMotor_TypeDef stepMotor[STEPMOTOR_COUNT];
StepMotor_TypeDef *pStepMotor = stepMotor;

const StepMotorPin_TypeDef StepMotorPin[] = 
{
    {GPIOA, GPIO_Pin_1, TIM5, TIM_Channel_2, TIM5_IRQn, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_0},//"StepMotor1"
    {GPIOA, GPIO_Pin_3, TIM2, TIM_Channel_4, TIM2_IRQn, GPIOA, GPIO_Pin_4, GPIOA, GPIO_Pin_5},//"StepMotor2"
};

//����Ҫ����ת�̻�ԭ���ľ���λ��
const uint8_t AbsCoordinate[10] = 
{
	20, // POS_PUMP1 = 
	21,	// POS_PUMP2 = 
	22,	// POS_PUMP3 = 
	23,	// POS_PUMP4 = 
	24,	// POS_PUMP5 = 
	25,	// POS_PUMP6 = 
	26,	// POS_PUMP7 = 
	27,	// POS_PUMP8 = 

	48, //��Һ�� POS_WASTE = 
	3,  //�ֶ��� POS_HANDLE = 	
};

#if 0
//��λ�ٶ�
const SpeedLevel_TypeDef speedLevel[] = {
	{STEPMOTOR_FREQ(0.10), 0.10*SPEED_CONST},	//0
	{STEPMOTOR_FREQ(0.13), 0.13*SPEED_CONST},
	{STEPMOTOR_FREQ(0.16), 0.16*SPEED_CONST},
	{STEPMOTOR_FREQ(0.20), 0.20*SPEED_CONST},
	{STEPMOTOR_FREQ(0.24), 0.24*SPEED_CONST},
	{STEPMOTOR_FREQ(0.29), 0.29*SPEED_CONST},	//5
	{STEPMOTOR_FREQ(0.34), 0.34*SPEED_CONST},
	{STEPMOTOR_FREQ(0.40), 0.40*SPEED_CONST},
	{STEPMOTOR_FREQ(0.48), 0.48*SPEED_CONST},
	{STEPMOTOR_FREQ(0.60), 0.60*SPEED_CONST},
	{STEPMOTOR_FREQ(0.75), 0.75*SPEED_CONST},	//10
	{STEPMOTOR_FREQ(0.90), 0.90*SPEED_CONST},
	{STEPMOTOR_FREQ(1.05), 1.05*SPEED_CONST},
	{STEPMOTOR_FREQ(1.20), 1.20*SPEED_CONST},
	{STEPMOTOR_FREQ(1.40), 1.40*SPEED_CONST},
	{STEPMOTOR_FREQ(1.60), 1.60*SPEED_CONST},	//15
	{STEPMOTOR_FREQ(1.80), 1.80*SPEED_CONST},
	{STEPMOTOR_FREQ(2.15), 2.15*SPEED_CONST},
	{STEPMOTOR_FREQ(2.30), 2.30*SPEED_CONST},
};
#else
//��λ�ٶ�
const SpeedLevel_TypeDef speedLevel[] = {
	{STEPMOTOR_FREQ(0.10), 0.10*SPEED_CONST},   //0
	{STEPMOTOR_FREQ(0.15), 0.15*SPEED_CONST},
	{STEPMOTOR_FREQ(0.20), 0.20*SPEED_CONST},
	{STEPMOTOR_FREQ(0.25), 0.25*SPEED_CONST},
	{STEPMOTOR_FREQ(0.40), 0.40*SPEED_CONST},
	{STEPMOTOR_FREQ(0.60), 0.60*SPEED_CONST},	//5
	{STEPMOTOR_FREQ(0.80), 0.80*SPEED_CONST},
	{STEPMOTOR_FREQ(1.00), 1.00*SPEED_CONST},
	{STEPMOTOR_FREQ(1.20), 1.20*SPEED_CONST},
	{STEPMOTOR_FREQ(1.40), 1.40*SPEED_CONST},
	{STEPMOTOR_FREQ(1.60), 1.60*SPEED_CONST},	//10
	{STEPMOTOR_FREQ(1.80), 1.80*SPEED_CONST},
	{STEPMOTOR_FREQ(2.00), 2.00*SPEED_CONST},
	{STEPMOTOR_FREQ(2.20), 2.20*SPEED_CONST},
};
#endif

//���ò����������
void StepMotor_SetDir(uint8_t num, Direction_TypeDef dir)
{
	if(dir == CW)
	{
		STEP1_DIR = 0;
		stepMotor[num].direction = CW;
	}
	else
	{
		STEP1_DIR = 1;
		stepMotor[num].direction = CCW;
	}
}

//���ò�������ٶ�
void StepMotor_SetSpeed(uint8_t num, uint8_t speedIndex)
{
	stepMotor[num].desSpeedIndex = speedIndex;	 //����Ŀ���ٶ�
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
            TIM_SetCounter(StepMotorPin[num].TIMx, stepMotor[num].pSpeedLevel[0].speed);
            TIM_SetCompare2(StepMotorPin[num].TIMx, (stepMotor[num].pSpeedLevel[0].speed)/2);
            stepMotor[num].status = 0x01;
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
    if((((stepMotor[num].status)&0x01) != 0) && stepMotor[num].speedStatus != SPEED_POSOFFSET)
        stepMotor[num].speedStatus = SPEED_STOP;
	
	//TF1 = 1; //���붨ʱ��1�ж�		
}

void StepMotor_StopAndAlign(uint8_t num, uint8_t len)
{
	StepMotor_SetCMD(num, DISABLE);   //��������

	StepMotor_SetPos(num, len);
	while(!StepMotor_IsOnPos(num))
	{
		if(PhSensor_GetStatus(PHSENSOR_POS))
			StepMotor_UpdatePos(num);	
	} 
	StepMotor_Stop(num);	  //�յ�λ�õ�����ֹͣ
	while(!StepMotor_IsStop(num));//�ȵ����ֹͣ
}

void StepMotor_SetPos(uint8_t num, uint8_t pos)
{
	stepMotor[num].curCount = 0;
	stepMotor[num].desCount = pos;	
}

void StepMotor_UpdatePos(uint8_t num)
{
	stepMotor[num].curCount++;

	if(stepMotor[num].direction == CW)
	{
		stepMotor[num].curPos++;
		if(stepMotor[num].curPos >= TANK_COUNT)
			stepMotor[num].curPos = 0;	
	}
	else
	{
		stepMotor[num].curPos--;
		if(stepMotor[num].curPos < 0)
			stepMotor[num].curPos = TANK_COUNT-1;
	}
	
	//Uart_SendData(pStepMotor->curPos);			
}

uint8_t StepMotor_IsOnPos(uint8_t num)
{
	return (stepMotor[num].curCount >= stepMotor[num].desCount); 		
}

uint8_t StepMotor_IsStop(uint8_t num)
{
	return (((stepMotor[num].status)&0x01) == 0); 
    //return (((StepMotorPin[num].TIMx->CR1)&TIM_CR1_CEN) == 0);   
}

//ת�̻�ԭ��
void StepMotor_Home(uint8_t num)
{
	cDebug("Home\n");

	PhSensor_SetCheckEdge(PHSENSOR_HOME, FALLINGEDGE);

	StepMotor_SetPos(num, 1);
	StepMotor_SetSpeed(num, SPEDD_HOME);  //��8���ٶ�
	StepMotor_SetDir(num, CCW);
	StepMotor_SetCMD(num, ENABLE);

	while(!StepMotor_IsOnPos(num))
	{
		if(PhSensor_GetStatus(PHSENSOR_HOME))
			StepMotor_UpdatePos(num);	
	}
	
	StepMotor_SetPos(num, 9);
	while(!StepMotor_IsOnPos(num))
	{
		if(PhSensor_GetStatus(PHSENSOR_POS))
			StepMotor_UpdatePos(num);
	}
	 
	StepMotor_SetCMD(num, DISABLE);   //ԭ�㴫������⵽��������

	StepMotor_SetPos(num, 2);
	while(!StepMotor_IsOnPos(num))
	{
		if(PhSensor_GetStatus(PHSENSOR_POS))
			StepMotor_UpdatePos(num);	
	} 
	StepMotor_Stop(num);	  //��3��λ�ü�⵽����ֹͣ
	while(!StepMotor_IsStop(num));	

	stepMotor[num].curPos = 0;  //���õ�ǰλ��Ϊ0
}

//����ת��ת������ϵ�ģ���ԣ�λ��
uint8_t StepMotor_Abs2Rel(uint8_t num, uint8_t absCoord)
{
	return ((absCoord + stepMotor[num].curPos) % TANK_COUNT);	
}

//ת��ת�����궨λ��disΪ����
void StepMotor_Position(uint8_t num, Direction_TypeDef dir, uint8_t dis)
{
	PhSensor_SetCheckEdge(PHSENSOR_POS, FALLINGEDGE);
	
	//���ò����������
	if(dir == CW) 
		StepMotor_SetDir(num, CW);
	else
		StepMotor_SetDir(num, CCW);
	
	//
	if(dis == 1)
	{
		StepMotor_SetSpeed(num, SPEED_POSITION1);  //��5���ٶ�
		StepMotor_SetPos(num, 1);
		StepMotor_SetCMD(num, ENABLE);	
		while(!StepMotor_IsOnPos(num))
		{
			if(PhSensor_GetStatus(PHSENSOR_POS))
				StepMotor_UpdatePos(num);	
		} 
		StepMotor_Stop(num);   //�յ㵽����ֹͣ	
	}
	else if(dis == 2)
	{
		StepMotor_SetSpeed(num, SPEED_POSITION2);  //��8���ٶ�
		StepMotor_SetPos(num, 1);
		StepMotor_SetCMD(num, ENABLE);	
		while(!StepMotor_IsOnPos(num))
		{
			if(PhSensor_GetStatus(PHSENSOR_POS))
				StepMotor_UpdatePos(num);	
		}  
		StepMotor_SetCMD(num, DISABLE);   //ǰ1��λ�õ���������
		
		StepMotor_SetPos(num, 1);
		while(!StepMotor_IsOnPos(num))
		{
			if(PhSensor_GetStatus(PHSENSOR_POS))
				StepMotor_UpdatePos(num);	
		} 
		StepMotor_Stop(num);	  //�յ�λ�õ�����ֹͣ	
	}
	else if(dis > 2)
	{
		StepMotor_SetSpeed(num, SPEDD_POSITION);  //��8���ٶ�
		StepMotor_SetPos(num, dis - 2);
		StepMotor_SetCMD(num, ENABLE);	
		while(!StepMotor_IsOnPos(num))
		{
			if(PhSensor_GetStatus(PHSENSOR_POS))
				StepMotor_UpdatePos(num);	
		} 
		StepMotor_SetCMD(num, DISABLE);   //ǰ2��λ�õ���������
		
		StepMotor_SetPos(num, 2);
		while(!StepMotor_IsOnPos(num))
		{
			if(PhSensor_GetStatus(PHSENSOR_POS))
				StepMotor_UpdatePos(num);	
		} 
		StepMotor_Stop(num);	  //�յ�λ�õ�����ֹͣ	
	}
	
	//�ȵ��������ֹͣ
	while(!StepMotor_IsStop(num));

	//ƫ��			
}

//ת��������궨λ��srcTankҪת��desTank��λ��					   
void StepMotor_RelativePosition(uint8_t num, uint8_t desTank, uint8_t srcTank)
{
	int8_t len;
	uint8_t dis;
	Direction_TypeDef dir;

	len = desTank - srcTank;  //��תΪ��
	//len = srcTank - desTank;	//��תΪ��

	if(len == 0)
		return;

//	if(abs(len) > TANK_COUNT/2)
//	{
//		if(len > 0) //ԭ����˳ʱ��
//			dir = CW;
//		else  //ԭ������ʱ��
//			dir = CCW;
//
//		dis = TANK_COUNT - abs(len);
//	}
//	else
//	{
//		if(len > 0)
//			dir = CCW;
//		else
//			dir = CW;
//			
//		dis = abs(len);	
//	}

	dir = CCW;
	if(len < 0)
		dis = TANK_COUNT + len;
	else
		dis = len;

	StepMotor_Position(num, dir, dis);

//	for(i=0;i<dis;i++)
//	{
//		if(dir == CW)
//		{
//			pStepMotor->curPos++;
//			if(pStepMotor->curPos >= TANK_COUNT)
//				pStepMotor->curPos = 0;
//		}
//		else
//		{
//			pStepMotor->curPos--;
//			if(pStepMotor->curPos < 0)
//				pStepMotor->curPos = TANK_COUNT-1;
//		}
//	}	
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
        TIM_TimeBaseStructure.TIM_Period = STEPMOTOR_DEFAULT_PERIOD; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ,��72000000/900=80KHz
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
        
        GPIO_SetBits(StepMotorPin[i].EN_GPIOx, StepMotorPin[i].EN_GPIO_Pin);  //ʹ�ܲ���������
        GPIO_ResetBits(StepMotorPin[i].DIR_GPIOx, StepMotorPin[i].DIR_GPIO_Pin);
        
        NVIC_InitStructure.NVIC_IRQChannel = StepMotorPin[i].NVIC_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = i;		//�����ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
        NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
        
        TIM_ARRPreloadConfig(StepMotorPin[i].TIMx, ENABLE);
        TIM_ITConfig(StepMotorPin[i].TIMx, TIM_IT_Update, ENABLE);
        //TIM_Cmd(StepMotorPin[i].TIMx, ENABLE);
    }
    //��ʹ�ò������2�ж�
    TIM_ITConfig(StepMotorPin[1].TIMx, TIM_IT_Update, DISABLE);
    
//    for(i=0;i<SIZEOF(speedLevel);i++)
//    {
//        printf("speedLevel[%d].speed = %d\t\n", i, speedLevel[i].speed);
//        printf("speedLevel[%d].speedConst = %d\t\n", i, speedLevel[i].speedConst);
//    }
    
    /****************��ʼ����������ṹ��***********************************/
	pStepMotor->offset = STEPMOTOR_OFFSET;
	//pStepMotor->offset = pProjectMan->posCali1;
	pStepMotor->speedStatus = SPEED_NONE;
	pStepMotor->pSpeedLevel = speedLevel;
	pStepMotor->curSpeedIndex = 0;
	pStepMotor->desSpeedIndex = 0;

	pStepMotor->control = 0x00;
}

void TIM2_IRQHandler(void)
{
    uint32_t oldBasePri = portSET_INTERRUPT_MASK_FROM_ISR();
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}

/* Timer5 interrupt routine */
void TIM5_IRQHandler(void)
{
	static uint16_t cnt = 0;
    //static uint16_t ledCnt = 0;
    uint32_t oldBasePri = portSET_INTERRUPT_MASK_FROM_ISR();
    
//    if(ledCnt++ > 5000)
//    {
//        ledCnt = 0;
//        LED1 = !LED1;
//    } 
    
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
    //			pStepMotor->curSpeedIndex = 0;
    //			pStepMotor->speedStatus = SPEED_POSOFFSET;
    //	
    //			cnt = 0;
    //			//TR1 = 0;  //ֹͣ��ʱ��1

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
                stepMotor[0].speedStatus = SPEED_POSOFFSET;
            }
            break;
            case SPEED_POSOFFSET:
            {
                static uint16_t offset = 0;

                offset++;
                if(offset++ >= stepMotor[0].offset)
                {
                    stepMotor[0].curSpeedIndex = 0;
                    stepMotor[0].speedStatus = SPEED_NONE;
                    //TR1 = 0;  //ֹͣ��ʱ��1
                    TIM_Cmd(TIM5, DISABLE);
                    printf("interrupt---stop the timer5\n");
                    offset = 0;
                    cnt = 0;
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

        //���¶�ʱ��
        TIM_SetCounter(TIM5, stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speed);
        TIM_SetCompare2(TIM5, (stepMotor[0].pSpeedLevel[stepMotor[0].curSpeedIndex].speed)/2);
        
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}

void StepMotorTestTask(void)
{
//	if(pStepMotor->control == 0x01) //������ʱ��
//	{
//		pStepMotor->SetPos(1);
//		pSensor->SetCheckEdge(FALLINGEDGE);

//		//pStepMotor->SetTimer(ENABLE); //������ʱ��1

//		while(!pStepMotor->IsOnPos())
//		{
//			if(pSensor->GetStatus(SENSOR_POS))
//				pStepMotor->UpdatePos();	
//		}
//		//pStepMotor->SetCMD(DISABLE);
//		pStepMotor->Stop();
//	}
    
    while(1)
    {
        if(pStepMotor[0].control == 0x01)
        {
            printf("stepMotor set speed %d\n", pStepMotor[0].temp);
            StepMotor_SetSpeed(0, pStepMotor[0].temp);
            StepMotor_SetCMD(0, ENABLE);
        }
        else if(pStepMotor[0].control == 0x02)
        {
            printf("stepMotor set disable\n");
            StepMotor_SetCMD(0, DISABLE);
        }
        else if(pStepMotor[0].control == 0x03)
        {
            printf("stepMotor stop\n");
            StepMotor_Stop(0);
        }
        else if(pStepMotor[0].control == 0x05)
        {
//            pStepMotor->SetCMD(DCMOTOR1, DISABLE);
//            pStepMotor->SetSpeed(DCMOTOR1, 90);
            printf("stepMotor set direction\n");
            StepMotor_SetDir(0, (Direction_TypeDef)pStepMotor[0].temp);
            //pStepMotor->SetCMD(DCMOTOR1, ENABLE);
        }
//        else
//        {
//            pStepMotor->SetCMD(DCMOTOR1, DISABLE);
//        }
        
        pStepMotor[0].control = 0;
        
        vTaskDelay(100);
    }
}
