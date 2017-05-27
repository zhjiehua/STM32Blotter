#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void pausePageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
	case PAUSE_PREACTION_BUTTON:
		{
			if(pProjectMan->pCurJumptoAction-1 < &pProjectMan->pCurRunningProject->action[0])
				pProjectMan->pCurJumptoAction = &pProjectMan->pCurRunningProject->action[ACTIONS_PER_PROJECT-1];
			else
				pProjectMan->pCurJumptoAction -= 1;

			SetTextValue(PAUSEPAGE_INDEX, PAUSE_ACTIONNAME_EDIT, (uint8_t*)pProjectMan->pCurJumptoAction->name);
		}
		break;
	case PAUSE_POSTACTION_BUTTON:
		{
			if(pProjectMan->pCurJumptoAction+1 > &pProjectMan->pCurRunningProject->action[ACTIONS_PER_PROJECT-1])
				pProjectMan->pCurJumptoAction = &pProjectMan->pCurRunningProject->action[0];
			else
				pProjectMan->pCurJumptoAction += 1;

			SetTextValue(PAUSEPAGE_INDEX, PAUSE_ACTIONNAME_EDIT, (uint8_t*)pProjectMan->pCurJumptoAction->name);
		}
		break;
	case PAUSE_JUMPTO_BUTTON:
		if(pProjectMan->pCurJumptoAction <= pProjectMan->pCurRunningAction)//如果要跳到当前动作之前的动作的，当做是恢复按钮用
		{
			pProjectMan->exception = EXCEPTION_NONE;
			SetScreen(RUNNINGPAGE_INDEX);
		}
		else
		{
			pProjectMan->pCurRunningAction = pProjectMan->pCurJumptoAction;
			pProjectMan->jumpTo = 1;
			pProjectMan->exception = EXCEPTION_NONE;
			SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_PAUSE_BUTTON, 0);
			SetControlVisiable(RUNNINGPAGE_INDEX, RUNNING_STOP_BUTTON, 0);
			SetScreen(RUNNINGPAGE_INDEX);
			cDebug("========pausePage JUMPTO program!\n");
		}
		break;
	case PAUSE_ROTATE_BUTTON:
		pProjectMan->rotateFlag = 1;
		cDebug("========pausePage ROTATE program!\n");
		break;
	case PAUSE_RESUME_BUTTON:
		pProjectMan->exception = EXCEPTION_NONE;
		SetScreen(RUNNINGPAGE_INDEX);
		cDebug("========pausePage RESUME program!\n");
		break;
		default:
			cDebug("pausePage BUTTON error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
