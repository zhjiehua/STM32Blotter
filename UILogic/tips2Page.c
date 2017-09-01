#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void tips2PageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
	case TIPS2_OK_BUTTON:
		//cDebug("TIPS2_OK_BUTTON\n");
		if(pProjectMan->exceptionButtonFlag == EXCEPTION_PAUSE)
		{
			cDebug("+++EXCEPTION_PAUSE\n");
			pProjectMan->exception = EXCEPTION_PAUSE;
			pProjectMan->exceptionButtonFlag = EXCEPTION_NONE;

			//更新动作名称
			pProjectMan->pCurJumptoAction = pProjectMan->pCurRunningAction;
			
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PAUSEPAGE_INDEX, PAUSE_ACTIONNAME_EDIT, pProjectMan->pCurJumptoAction->name);
			SetScreen(PAUSEPAGE_INDEX);
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
		else if(pProjectMan->exceptionButtonFlag == EXCEPTION_STOP)
		{
			cDebug("+++EXCEPTION_STOP\n");
			pProjectMan->exception = EXCEPTION_STOP;
			pProjectMan->exceptionButtonFlag = EXCEPTION_NONE;
			
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetScreen(PROJECTPAGE_INDEX);
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
		else
		{
			cDebug("+++TIPS_OK\n");	
			pProjectMan->tipsButton = TIPS_OK;
		}
		break;
	case TIPS2_CANCEL_BUTTON:
		if(pProjectMan->exceptionButtonFlag != EXCEPTION_NONE)
		{
			cDebug("###EXCEPTION_NONE\n");
			pProjectMan->exceptionButtonFlag = EXCEPTION_NONE;
			
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetScreen(RUNNINGPAGE_INDEX);
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
		else
		{
			cDebug("###TIPS_CANCEL\n");
			pProjectMan->tipsButton = TIPS_CANCEL;
		}
		break;
	default:
		cDebug("tips2Page BUTTON error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
