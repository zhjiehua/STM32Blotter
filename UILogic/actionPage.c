#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"
#include "../HARDWARE/24CXX/24cxx.h"

#ifdef __cplusplus
extern "C" {
#endif

void actionPageButtonProcess(uint16 control_id, uint8  state)
{
	uint16_t addrOffset;
	switch(control_id)
	{
		case ACTEDIT_SAVE_BUTTON:
		{
			//保存动作参数
			pProjectMan->pCurEditAction->pump = pProjectMan->pump;
			pProjectMan->pCurEditAction->tips = pProjectMan->tips;
			pProjectMan->pCurEditAction->voice = pProjectMan->voice;
			pProjectMan->pCurEditAction->addAmount = pProjectMan->addAmount;
			pProjectMan->pCurEditAction->imbiAmount = pProjectMan->imbiAmount;
			pProjectMan->pCurEditAction->shakeSpeed = pProjectMan->shakeSpeed;
			pProjectMan->pCurEditAction->shakeTime.hour = pProjectMan->shakeTime.hour;
			pProjectMan->pCurEditAction->shakeTime.minute = pProjectMan->shakeTime.minute;
			pProjectMan->pCurEditAction->loopTime = pProjectMan->loopTime;
			cDebug("========actionPage Save the ACTION data!\n");

			addrOffset = pProjectMan->pCurEditProject->index*PROJECT_SIZE + (pProjectMan->pCurEditAction->index)*ACTION_SIZE;
			AT24CXX_Write(PROJECT_BASEADDR+addrOffset, (uint8_t*)pProjectMan->pCurEditAction, ACTION_SIZE-NAME_SIZE);  //保存参数
		}
		break;
		case ACTEDIT_PUMPSEL_BUTTON:
		break;
		case ACTEDIT_TIPSSEL_BUTTON:
		break;
		case ACTEDIT_VOICESEL_BUTTON:
		break;
		case ACTEDIT_SPEEDSEL_BUTTON:
		break;
		case ACTEDIT_BACK_BUTTON:
		break;
		default:
			cDebug("actionPage BUTTON error!\n");
		break;
	}
}

void actionPageEditProcess(uint16 control_id, uint8 *str)
{
	switch(control_id)
	{
		case ACTEDIT_ADDAMOUNT_EDIT:
			if(pProjectMan->pump == 8)//0，无泵
				SetTextValueInt32(ACTIONPAGE_INDEX, ACTEDIT_ADDAMOUNT_EDIT, 0);
			else
				pProjectMan->addAmount = (uint8)StringToInt32(str);//StringToFloat(str);
		break;
		case ACTEDIT_IMBIAMOUNT_EDIT:
			pProjectMan->imbiAmount = StringToInt32(str);
			break;
		case ACTEDIT_TIMEHOUR_EDIT:
			pProjectMan->shakeTime.hour = StringToInt32(str);
			break;
		case ACTEDIT_TIMEMIN_EDIT:
			pProjectMan->shakeTime.minute = StringToInt32(str);
			break;
		case ACTEDIT_LOOPTIME_EDIT:
			pProjectMan->loopTime = StringToInt32(str);
			break;
		default:
			cDebug("actionPage EDIT error!\n");
		break;
	}
}

void actionPageMenuProcess(uint16 control_id, uint8 item)
{
	switch(control_id)
	{
	case ACTEDIT_PUMPSEL_MENU:
		{
			if(pProjectMan->pump != item)
			{
				//如果是pump0转换到其他值，则初始化加注量为最小值2
				if(pProjectMan->pump == PUMP0)
				{
					pProjectMan->addAmount = 2;
					SetTextValueInt32(ACTIONPAGE_INDEX, ACTEDIT_ADDAMOUNT_EDIT, 2);
				}	

				pProjectMan->pump = (Pump_TypeDef)item;
				if(item == PUMP0) //0，无泵
				{
					pProjectMan->addAmount = 0;
					//SetTextValueFloat(ACTIONPAGE_INDEX, ACTEDIT_ADDAMOUNT_EDIT, 0.0);
					SetTextValueInt32(ACTIONPAGE_INDEX, ACTEDIT_ADDAMOUNT_EDIT, 0);
				}
			}	
		}
		break;
	case ACTEDIT_TIPSSEL_MENU:
		{
			pProjectMan->tips = (Tips_TypeDef)item;
		}
		break;
	case ACTEDIT_VOICESEL_MENU:
		{
			pProjectMan->voice = (Voice_TypeDef)item;
		}
		break;
	case ACTEDIT_SPEEDSEL_MENU:
		{
			pProjectMan->shakeSpeed = (ShakeSpeed_TypeDef)item;
		}
		break;
	default:
		cDebug("actionPage MENU error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
