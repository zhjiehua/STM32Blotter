#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void purgePageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
		case PURGE_PUMP1_BUTTON:
		{
			state ? (pProjectMan->purgePumpSel |= PUMP1_MASK) : (pProjectMan->purgePumpSel &= ~PUMP1_MASK);
		}
		break;
		case PURGE_PUMP2_BUTTON:
			{
				state ? (pProjectMan->purgePumpSel |= PUMP2_MASK) : (pProjectMan->purgePumpSel &= ~PUMP2_MASK);
			}
			break;
		case PURGE_PUMP3_BUTTON:
			{
				state ? (pProjectMan->purgePumpSel |= PUMP3_MASK) : (pProjectMan->purgePumpSel &= ~PUMP3_MASK);
			}
			break;
		case PURGE_PUMP4_BUTTON:
			{
				state ? (pProjectMan->purgePumpSel |= PUMP4_MASK) : (pProjectMan->purgePumpSel &= ~PUMP4_MASK);
			}
			break;
		case PURGE_PUMP5_BUTTON:
			{
				state ? (pProjectMan->purgePumpSel |= PUMP5_MASK) : (pProjectMan->purgePumpSel &= ~PUMP5_MASK);
			}
			break;
		case PURGE_PUMP6_BUTTON:
			{
				state ? (pProjectMan->purgePumpSel |= PUMP6_MASK) : (pProjectMan->purgePumpSel &= ~PUMP6_MASK);
			}
			break;
		case PURGE_PUMP7_BUTTON:
			{
				state ? (pProjectMan->purgePumpSel |= PUMP7_MASK) : (pProjectMan->purgePumpSel &= ~PUMP7_MASK);
			}
			break;
		case PURGE_PUMP8_BUTTON:
			{
				state ? (pProjectMan->purgePumpSel |= PUMP8_MASK) : (pProjectMan->purgePumpSel &= ~PUMP8_MASK);
			}
			break;
		case PURGE_START_BUTTON:
			{
				cDebug("========purgePage start to run the PURGE program!\n");
				//pProjectMan->runningType = RUNNING_PURGE;

				//createTask(TASK_PURGE);
                xTaskCreate( (TaskFunction_t)PurgeTask, "PurgeTask", purge_TASK_STACKDEP, NULL, purge_TASK_PRIORITY, NULL );
			}
			break;
		case PURGE_BACK_BUTTON:
			break;
		default:
			cDebug("purgePage BUTTON error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
