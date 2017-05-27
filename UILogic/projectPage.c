#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"
#include "../HARDWARE/24CXX/24cxx.h"

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void projectPageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
		case PRO_POSTPROJECT_BUTTON:
		{
//			if(pProjectMan->pCurProject+1 > &project[PROJECT_COUNT-1])
//				pProjectMan->pCurProject = &project[0];
//			else
//				pProjectMan->pCurProject += 1;

			//从EEPROM读取项目数据
			uint16_t addrOffset;
			if(pProjectMan->pCurProject->index == PROJECT_COUNT-1)
				addrOffset = 0;  //读出第一个项目参数
			else
				addrOffset = pProjectMan->pCurProject->index+1;
			AT24CXX_Read(PROJECT_BASEADDR+addrOffset*PROJECT_SIZE, (uint8_t*)project, PROJECT_SIZE);  //读出下一个项目参数			

			SetTextValue(PROJECTPAGE_INDEX, PRO_PROJECTNAME_EDIT, (uint8_t*)pProjectMan->pCurProject->name);
		}
		break;
		case PRO_PREPROJECT_BUTTON:
		{
//			if(pProjectMan->pCurProject-1 < &project[0])
//				pProjectMan->pCurProject = &project[PROJECT_COUNT-1];
//			else
//				pProjectMan->pCurProject -= 1;

			//从EEPROM读取项目数据
			uint16_t addrOffset;
			if(pProjectMan->pCurProject->index == 0)
				addrOffset = PROJECT_COUNT-1;  //读出第一个项目参数
			else
				addrOffset = pProjectMan->pCurProject->index-1;
			AT24CXX_Read(PROJECT_BASEADDR+addrOffset*PROJECT_SIZE, (uint8_t*)project, PROJECT_SIZE);  //读出上一个项目参数

			SetTextValue(PROJECTPAGE_INDEX, PRO_PROJECTNAME_EDIT, (uint8_t*)pProjectMan->pCurProject->name);
		}
		break;
		case PRO_STARTPROJECT_BUTTON://开始项目，要初始化运行页面相应控件初始状态
		{
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"runing");
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_PROJECT_EDIT, (uint8_t*)pProjectMan->pCurProject->name);
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_ACTION_EDIT, (uint8_t*)pProjectMan->pCurProject->action[0].name);
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_PUMP_EDIT, (uint8_t*)"0");
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_ADDTANK_EDIT, (uint8_t*)"0");
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_IMBITANK_EDIT, (uint8_t*)"0");
			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_LOOPTIME_EDIT, (uint8_t*)"1");
			SetTextValueInt32(RUNNINGPAGE_INDEX, RUNNING_TOTALLOOPTIME_EDIT, pProjectMan->pCurProject->action[0].loopTime);

			StopTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
			if(pProjectMan->pCurProject->action[0].shakeTime.hour > 0
				|| pProjectMan->pCurProject->action[0].shakeTime.minute > 0)
			{
				uint32 rtcTime = pProjectMan->pCurProject->action[0].shakeTime.hour*3600
					+ pProjectMan->pCurProject->action[0].shakeTime.minute*60;
				SeTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC, rtcTime);
				//StartTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC);
			}
			else
				SeTimer(RUNNINGPAGE_INDEX, RUNNING_TIME_RTC, 0);

			SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 0);
			SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 0);

			pProjectMan->pCurRunningProject = pProjectMan->pCurProject;
			pProjectMan->pCurRunningAction = &pProjectMan->pCurRunningProject->action[0];
			//pProjectMan->runningType = RUNNING_PROJECT; //同步工作线程

			//创建线程
			//os_create_task(TASK_PROJECT);	//创建工程任务
			//createTask(TASK_PROJECT);
            xTaskCreate( (TaskFunction_t)ProjectTask, "ProjectTask", project_TASK_STACKDEP, NULL, project_TASK_PRIORITY, NULL );
            
			//cDebug("========projectPage start to run the PROJECT program\n");
		}
		break;
		case PRO_BACK_BUTTON:
			break;
		default:
			cDebug("projectPage BUTTON error!\n");
		break;
	}
}

void projectPageEditProcess(uint16 control_id, uint8 *str)
{
	switch(control_id)
	{
		case PRO_STARTTANK_EDIT:
			pProjectMan->startTank = (uint8)StringToInt32(str);
			if(pProjectMan->startTank > pProjectMan->endTank)
			{
				pProjectMan->endTank = pProjectMan->startTank;
				SetTextValue(PROJECTPAGE_INDEX, PRO_ENDTANK_EDIT, str);
			}
		break;
		case PRO_ENDTANK_EDIT:
			pProjectMan->endTank = (uint8)StringToInt32(str);
			if(pProjectMan->startTank > pProjectMan->endTank)
			{
				pProjectMan->startTank = pProjectMan->endTank;
				SetTextValue(PROJECTPAGE_INDEX, PRO_STARTTANK_EDIT, str);
			}
		break;
		default:
			cDebug("projectPage EDIT error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
