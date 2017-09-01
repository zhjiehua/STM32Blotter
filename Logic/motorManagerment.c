#include "motorManagerment.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "CPrintf.h"
#include "../HARDWARE/DCMotor/dcmotor.h"
#include "../HARDWARE/StepMotor/stepmotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HMI/hmi_driver.h"
#include "../HMI/hmi_user_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

MotorMan_TypeDef motorMan;
MotorMan_TypeDef *pMotorMan = &motorMan;
	
//各重要点在转盘回原点后的绝对位置
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

	48, //废液口 POS_WASTE = 
	3,  //手动点 POS_HANDLE = 
};

void Turntable_SetPos(uint8_t pos)
{
	motorMan.curCount = 0;
	motorMan.desCount = pos;	
}

void Turntable_UpdatePos(void)
{
	motorMan.curCount++;

	if(motorMan.turntableStepMotor->direction == CW)
	{
		motorMan.curPos++;
		if(motorMan.curPos >= TANK_COUNT)
			motorMan.curPos = 0;	
	}
	else
	{
		motorMan.curPos--;
		if(motorMan.curPos < 0)
			motorMan.curPos = TANK_COUNT-1;
	}			
}

uint8_t Turntable_IsOnPos(void)
{
	return (motorMan.curCount >= motorMan.desCount); 		
}

void Turntable_StopAndAlign(uint8_t len, uint8_t *pStopFlag)
{
	StepMotor_SetCMD(STEPMOTOR_MAIN, DISABLE);   //立即减速

	Turntable_SetPos(len);
	while(!Turntable_IsOnPos() && !(*pStopFlag))
	{
		PhSensor_SingleScan(PHSENSOR_POS);
		if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
			Turntable_UpdatePos();
		vTaskDelay(10);
	} 
	StepMotor_Stop(STEPMOTOR_MAIN);	  //终点位置到立即停止
	while(!StepMotor_IsStop(STEPMOTOR_MAIN) && !(*pStopFlag))//等待电机停止
		vTaskDelay(10);
}

void Turntable_SetOffset(uint16_t offset)
{
	pStepMotor[0].offset = offset;
}

//转盘回原点
void Turntable_Home(uint8_t *pStopFlag)
{
	cDebug("Home\n");
	
	pStepMotor[0].offset = motorMan.motorParaWastePumpCalib;//motorParaWastePumpCalib  motorParaCCWPumpCalib
	
	Turntable_SetPos(1);
	StepMotor_SetSpeed(STEPMOTOR_MAIN, motorMan.motorParaTTHomeSpeed);
	StepMotor_SetDir(STEPMOTOR_MAIN, CCW);
	StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);

	PhSensor_SingleScan(PHSENSOR_HOME);
	while(!Turntable_IsOnPos() && !(*pStopFlag))
	{
		PhSensor_SingleScan(PHSENSOR_HOME);
		if(PhSensor_SingleCheckEdge(PHSENSOR_HOME, FALLINGEDGE))
			Turntable_UpdatePos();
		vTaskDelay(10);
	}

	Turntable_SetPos(8+3);//9
	PhSensor_SingleScan(PHSENSOR_POS);
	while(!Turntable_IsOnPos() && !(*pStopFlag))
	{
		PhSensor_SingleScan(PHSENSOR_POS);
		if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
			Turntable_UpdatePos();
		vTaskDelay(10);
	}
	 
	StepMotor_SetCMD(STEPMOTOR_MAIN, DISABLE);   //????????????

	Turntable_SetPos(4);//2
	PhSensor_SingleScan(PHSENSOR_POS);
	while(!Turntable_IsOnPos() && !(*pStopFlag))
	{
		PhSensor_SingleScan(PHSENSOR_POS);
		if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
			Turntable_UpdatePos();
		vTaskDelay(10);
	} 
	StepMotor_Stop(STEPMOTOR_MAIN);	  //第3个位置检测到立即停止
	while(!StepMotor_IsStop(STEPMOTOR_MAIN) && !(*pStopFlag))
		vTaskDelay(10);

	motorMan.curPos = 0;  //设置当前位置为0
}

//返回转盘转动坐标系的（相对）位置
uint8_t Turntable_Abs2Rel(uint8_t absCoord)
{
	return ((absCoord + motorMan.curPos) % TANK_COUNT);	
}

//转盘转动坐标定位，dis为距离
void Turntable_Position(Direction_TypeDef dir, uint8_t dis, uint8_t *pStopFlag)
{
	//设置步进电机方向
	if(dir == CW) 
		StepMotor_SetDir(STEPMOTOR_MAIN, CW);
	else
		StepMotor_SetDir(STEPMOTOR_MAIN, CCW);
	
	//
	if(dis == 1)
	{
		Turntable_SetPos(1);
		StepMotor_SetSpeed(STEPMOTOR_MAIN, motorMan.motorParaTTStep1Speed);  //
		StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);
		while(!Turntable_IsOnPos() && !(*pStopFlag))
		{
			PhSensor_SingleScan(PHSENSOR_POS);
			if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
				Turntable_UpdatePos();
			vTaskDelay(10);
		} 
		StepMotor_Stop(STEPMOTOR_MAIN);   //终点到立即停止
	}
	else if(dis == 2)
	{
		Turntable_SetPos(1);
		StepMotor_SetSpeed(STEPMOTOR_MAIN, motorMan.motorParaTTStep2Speed);  //?8???
		StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);	
		while(!Turntable_IsOnPos() && !(*pStopFlag))
		{
			PhSensor_SingleScan(PHSENSOR_POS);
			if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
				Turntable_UpdatePos();
			vTaskDelay(10);
		}  
		StepMotor_SetCMD(STEPMOTOR_MAIN, DISABLE);   //前1个位置到立即减速
		
		Turntable_SetPos(1);
		while(!Turntable_IsOnPos() && !(*pStopFlag))
		{
			PhSensor_SingleScan(PHSENSOR_POS);
			if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
				Turntable_UpdatePos();
			vTaskDelay(10);
		} 
		StepMotor_Stop(STEPMOTOR_MAIN);	  //终点位置到立即停止	
	}
	else if(dis > 2)
	{
		Turntable_SetPos(dis - 2);
		StepMotor_SetSpeed(STEPMOTOR_MAIN, motorMan.motorParaTTLocationSpeed);  //第8级速度
		StepMotor_SetCMD(STEPMOTOR_MAIN, ENABLE);	
		while(!Turntable_IsOnPos() && !(*pStopFlag))
		{
			PhSensor_SingleScan(PHSENSOR_POS);
			if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
				Turntable_UpdatePos();
			vTaskDelay(10);
		} 
		StepMotor_SetCMD(STEPMOTOR_MAIN, DISABLE);   //前2个位置到立即减速
		
		Turntable_SetPos(2);
		while(!Turntable_IsOnPos() && !(*pStopFlag))
		{
			PhSensor_SingleScan(PHSENSOR_POS);
			if(PhSensor_SingleCheckEdge(PHSENSOR_POS, FALLINGEDGE))
				Turntable_UpdatePos();
			vTaskDelay(10);
		} 
		StepMotor_Stop(STEPMOTOR_MAIN);	  //终点位置到立即停止	
	}
	
	//等待电机真正停止
	while(!StepMotor_IsStop(STEPMOTOR_MAIN) && !(*pStopFlag))
		vTaskDelay(10);

	//偏移		
}

//转盘相对坐标定位，srcTank要转到desTank的位置				   
void Turntable_RelativePosition(uint8_t desTank, uint8_t srcTank, uint8_t *pStopFlag)
{
	int8_t len;
	uint8_t dis;
	Direction_TypeDef dir;

	len = desTank - srcTank;  //????
	//len = srcTank - desTank;	//????

	if(len == 0)
		return;

//	if(abs(len) > TANK_COUNT/2)
//	{
//		if(len > 0) //??????
//			dir = CW;
//		else  //??????
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

	Turntable_Position(dir, dis, pStopFlag);

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

//废液泵
void WastePump_SetPos(Position_TypeDef pos, uint8_t speed, uint8_t *pStopFlag)
{
	if(pos == UP)
	{
		DCMotor_Run(PUMP_WASTE, CW, speed);
		PhSensor_SingleScan(PHSENSOR_UP);
		if(!PhSensor_SingleCheck(PHSENSOR_UP))
		{
			while(!PhSensor_SingleCheckEdge(PHSENSOR_UP, RASINGEDGE) && !(*pStopFlag))
			{
				vTaskDelay(10);
				PhSensor_SingleScan(PHSENSOR_UP);
			}
		}
		DCMotor_Stop(PUMP_WASTE);
	}
	else
	{
		DCMotor_Run(PUMP_WASTE, CCW, speed);
		PhSensor_SingleScan(PHSENSOR_DOWN);
		if(!PhSensor_SingleCheck(PHSENSOR_DOWN))
		{
			while(!PhSensor_SingleCheckEdge(PHSENSOR_DOWN, RASINGEDGE) && !(*pStopFlag))
			{
				vTaskDelay(10);
				PhSensor_SingleScan(PHSENSOR_DOWN);
			}
		}
		DCMotor_Stop(PUMP_WASTE);
	}
}

#ifdef __cplusplus
}
#endif
