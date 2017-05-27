#include "../DCMotor/DCMotor.h"
#include "../PhotoelectricSensor/PhSensor.h"
#include "usart.h"
#include "CPrintf.h"

#include "FreeRTOS.h"
#include "task.h"

static DCMotor_TypeDef dcMotor[DCMOTOR_COUNT];
DCMotor_TypeDef *pDCMotor = dcMotor;

//直流电机的频率最好是在13kHz~15kHz之间，这里设置为14kHz
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

//static void DCMotor_SetCMD(DCMotorEnum_TypeDef num, Status sta)
//{
//    if(sta != ENABLE)
//    {
////        TIM_CCxCmd(DCMotorPin[num].TIMx, DCMotorPin[num].TIM_Channel, TIM_CCx_Disable);
////        GPIO_SetBits(DCMotorPin[num].PWM_GPIOx, DCMotorPin[num].PWM_GPIO_Pin);
////        //TIM_ForcedOC1Config(DCMotorPin[num].TIMx, TIM_ForcedAction_Active);
////        GPIO_SetBits(DCMotorPin[num].DIR_GPIOx, DCMotorPin[num].DIR_GPIO_Pin);
//        
////        DCMotor_SetDir(num, CCW);
////        DCMotor_SetSpeed(num, 0);
//    }
//    else
//    {
////        TIM_CCxCmd(DCMotorPin[num].TIMx, DCMotorPin[num].TIM_Channel, TIM_CCx_Enable);
////        //GPIO_SetBits(DCMotorPin[num].PWM_GPIOx, DCMotorPin[num].PWM_GPIO_Pin);
////        //GPIO_SetBits(DCMotorPin[num].DIR_GPIOx, DCMotorPin[num].DIR_GPIO_Pin);
//    }
//}

//设置直流电机的速度
//其实是设置PWM的占空比，0~100,100为最大占空比
void DCMotor_SetSpeed(DCMotorEnum_TypeDef num, uint8_t speed)
{
    dcMotor[num].speed = speed;
    
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

//设置废液口电机（直流泵10）的方向
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

void DCMotor_SetPos(DCMotorEnum_TypeDef num, uint8_t pos)
{
	dcMotor[num].curCount = 0;
	dcMotor[num].desCount = pos;
}

void DCMotor_UpdatePos(DCMotorEnum_TypeDef num)
{
	dcMotor[num].curCount++;		
}

uint8_t DCMotor_IsOnPos(DCMotorEnum_TypeDef num)
{
	return (dcMotor[num].curCount >= dcMotor[num].desCount); 		
}

//如果向下卡住了，则向上动一下，在向下走，直到到位；向上同理
void DCMotor_WastePump_SetPos(Position_TypeDef pos)
{
	uint16_t cnt = 0;
	uint8_t flag = 0;

	if(pos == UP)
	{
		PhSensor_SetCheckEdge(PHSENSOR_UP, RASINGEDGE);

		DCMotor_SetPos(PUMP_WASTE, 1);
		//DCMotor_SetSpeed(PUMP_WASTE, 100-30);
		DCMotor_SetSpeed(PUMP_WASTE, 0); //CW方向时，100表示不转，0为最大速度
		DCMotor_SetDir(PUMP_WASTE, CW);
		//DCMotor_SetCMD(PUMP_WASTE, ENABLE);

		cDebug("UP\n");
		while(!DCMotor_IsOnPos(PUMP_WASTE))
		{
			//cDebug("cnt = %d\n", cnt);
			if(flag == 0)
			{
				cnt++;
				//cDebug("+++++++ cnt = %d\n", cnt);
				if(cnt >= 300)
				{
					cnt = 0;
					//pDCMotor->SetPos(1);
					//DCMotor_SetCMD(PUMP_WASTE, DISABLE);
					DCMotor_SetDir(PUMP_WASTE, CCW);
                    DCMotor_SetSpeed(PUMP_WASTE, 100);
					//DCMotor_SetCMD(PUMP_WASTE, ENABLE);
					flag = 1;
					cDebug("UP down\n");
				}
			}
			else
			{
				cnt++;
				//cDebug("------ cnt = %d\n", cnt);
				if(cnt >= 100)
				{
					cnt = 0;
					DCMotor_SetPos(PUMP_WASTE, 1);
					//DCMotor_SetCMD(PUMP_WASTE, DISABLE);
					DCMotor_SetDir(PUMP_WASTE, CW);
                    DCMotor_SetSpeed(PUMP_WASTE, 0);	
					//DCMotor_SetCMD(PUMP_WASTE, ENABLE);
					flag = 0;
					cDebug("UP up\n");
				}
			}
			
			if(PhSensor_GetStatus(PHSENSOR_UP))
				DCMotor_UpdatePos(PUMP_WASTE);	
		}
		cDebug("UP finish\n");
		//DCMotor_SetCMD(PUMP_WASTE, DISABLE);		
	}
	else
	{
		PhSensor_SetCheckEdge(PHSENSOR_DOWN, RASINGEDGE);
	
		DCMotor_SetPos(PUMP_WASTE, 1);
		//DCMotor_SetSpeed(PUMP_WASTE, 30);
		DCMotor_SetSpeed(PUMP_WASTE, 100);  //CCW方向时，0表示不转，100为最大速度
		DCMotor_SetDir(PUMP_WASTE, CCW);
		//DCMotor_SetCMD(PUMP_WASTE, ENABLE);

		cDebug("DOWN\n");

		while(!DCMotor_IsOnPos(PUMP_WASTE))
		{
			//cDebug("cnt = %d\n", cnt);
			if(flag == 0)
			{
				cnt++;
				//cDebug("+++++++ cnt = %d\n", cnt);
				if(cnt >= 300)
				{
					cnt = 0;
					//DCMotor_SetPos(1);
					//DCMotor_SetCMD(PUMP_WASTE, DISABLE);
					DCMotor_SetSpeed(PUMP_WASTE, 0);
					DCMotor_SetDir(PUMP_WASTE, CW);
					//DCMotor_SetCMD(PUMP_WASTE, ENABLE);
					flag = 1;
					cDebug("DOWN up\n");
				}
			}
			else
			{
				cnt++;
				//cDebug("------ cnt = %d\n", cnt);
				if(cnt >= 100)
				{
					cnt = 0;
					DCMotor_SetPos(PUMP_WASTE, 1);
					//DCMotor_SetCMD(PUMP_WASTE, DISABLE);
					DCMotor_SetSpeed(PUMP_WASTE, 100);
					DCMotor_SetDir(PUMP_WASTE, CCW);
					//DCMotor_SetCMD(PUMP_WASTE, ENABLE);
					flag = 0;
					cDebug("DOWN down\n");
				}	
			}

			if(PhSensor_GetStatus(PHSENSOR_DOWN))
				DCMotor_UpdatePos(PUMP_WASTE);	
		}
		cDebug("DOWN finish\n");

		//DCMotor_SetCMD(PUMP_WASTE, DISABLE);
		DCMotor_SetDir(PUMP_WASTE, CW);//废液泵电机停止的时候要拉高方向引脚		
	}

	flag = 0;
	cnt = 0;
}

//PWM初始化
void DCMotor_Init(void)
{ 
	uint8_t i;
    GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4, ENABLE);	//使能定时器3时钟  
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1|RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB
                        |RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD
                        |RCC_APB2Periph_GPIOE, ENABLE);
    
    for(i=0;i<SIZEOF(DCMotorPin);i++)
    {
        GPIO_InitStructure.GPIO_Pin = DCMotorPin[i].PWM_GPIO_Pin; //TIM_CH2
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(DCMotorPin[i].PWM_GPIOx, &GPIO_InitStructure);//初始化GPIO
        if(DCMotorPin[i].GPIO_Remap != 0)
            GPIO_PinRemapConfig(DCMotorPin[i].GPIO_Remap, ENABLE);        
        
        GPIO_InitStructure.GPIO_Pin = DCMotorPin[i].DIR_GPIO_Pin; //TIM_CH2
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //复用推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(DCMotorPin[i].DIR_GPIOx, &GPIO_InitStructure);//初始化GPIO
        GPIO_SetBits(DCMotorPin[i].DIR_GPIOx, DCMotorPin[i].DIR_GPIO_Pin);//初始化时输出高电平
        
        TIM_TimeBaseStructure.TIM_Period = DCMOTOR_DEFAULT_PERIOD; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值,72000000/900=80KHz
        TIM_TimeBaseStructure.TIM_Prescaler = 0; //设置用来作为TIMx时钟频率除数的预分频值 
        TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
        TIM_TimeBaseInit(DCMotorPin[i].TIMx, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
        
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2  TIM_OCMode_Timing
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
        TIM_OCInitStructure.TIM_Pulse = 0;//设置待装入捕获比较寄存器的脉冲值, 初始化时要输出高电平
        
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
        }
        
        TIM_ARRPreloadConfig(DCMotorPin[i].TIMx, ENABLE); 
        TIM_Cmd(DCMotorPin[i].TIMx, ENABLE);
        //TIM_CtrlPWMOutputs(DCMotorPin[i].TIMx, ENABLE);
        
        dcMotor[i].direction = CW;
        dcMotor[i].speed = 0;
        dcMotor[i].timPulse = 0;
        dcMotor[i].timPeriod = DCMOTOR_DEFAULT_PERIOD;
    }
    
    /****************************************************************************************/
	pDCMotor->control = 0x00;

//	for(i=0;i<SIZEOF(DCMotorPin);i++)
//	{
//		pDCMotor->SetCMD((DCMotorEnum_TypeDef)i, DISABLE); //停止所有電機
//	}
//	pDCMotor->SetDir(PUMP_WASTE, CW); //废液泵电机停止的时候要拉高方向引脚

//	pDCMotor->SetCMD(DCMOTOR12, ENABLE);//夹管阀先使能，到时调用pDCMotor->SetSpeed(DCMOTOR12, 0);开，调用pDCMotor->SetSpeed(DCMOTOR12, 100);关
//	pDCMotor->SetSpeed(DCMOTOR12, 100);//关闭夹管阀
}

void DCMotorTestTask(void)
{
//	if(pDCMotor->control == 0x01)
//	{
//		pSensor->SetCheckEdge(RASINGEDGE);
//	
//		pDCMotor->SetPos(1);
//		pDCMotor->SetSpeed(DCMOTOR1, 10);
//		pDCMotor->SetDir(DCMOTOR1, CW);
//		pDCMotor->SetCMD(DCMOTOR1, ENABLE);

//		while(!pDCMotor->IsOnPos())
//		{
//			if(pSensor->GetStatus(SENSOR_UP))
//				pDCMotor->UpdatePos();	
//		}
//		pDCMotor->SetCMD(DCMOTOR1, DISABLE);

//		//pDCMotor->WastePump_SetPos(UP);

//		pDCMotor->control = 0x00;
//	}
//	else if(pDCMotor->control == 0x02)
//	{
//		pSensor->SetCheckEdge(RASINGEDGE);
//	
//		pDCMotor->SetPos(1);
//		pDCMotor->SetSpeed(DCMOTOR1, 90);
//		pDCMotor->SetDir(DCMOTOR1, CW);
//		pDCMotor->SetCMD(DCMOTOR1, ENABLE);

//		while(!pDCMotor->IsOnPos())
//		{
//			if(pSensor->GetStatus(SENSOR_UP))
//				pDCMotor->UpdatePos();
//		}
//		pDCMotor->SetCMD(DCMOTOR1, DISABLE);

//		//pDCMotor->WastePump_SetPos(DOWN);

//		pDCMotor->control = 0x00;
//	}

    while(1)
    {
        if(pDCMotor[0].control == 0x01)
        {
//            pDCMotor->SetCMD(DCMOTOR1, DISABLE);
            DCMotor_SetSpeed(DCMOTOR1, pDCMotor->temp*10);
//            pDCMotor->SetDir(DCMOTOR1, CW);
//            pDCMotor->SetCMD(DCMOTOR1, ENABLE);
        }
        else if(pDCMotor[0].control == 0x02)
        {
//            pDCMotor->SetCMD(DCMOTOR1, DISABLE);
//            pDCMotor->SetSpeed(DCMOTOR1, 90);
            DCMotor_SetDir(DCMOTOR1, (Direction_TypeDef)pDCMotor->temp);
            //pDCMotor->SetCMD(DCMOTOR1, ENABLE);
        }
        else if(pDCMotor[0].control == 0x03)
        {
            //pDCMotor->SetCMD(DCMOTOR1, DISABLE);
        }
        else if(pDCMotor[0].control == 0x04)
        {
            //pDCMotor->SetCMD(DCMOTOR1, ENABLE);
        }
        
        pDCMotor->control = 0;
        
        vTaskDelay(100);
    }
  
} 
