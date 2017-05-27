#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void selectPumpPageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
	case PUMPSEL_PUMP1_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP1_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP1_MASK);
		}
		break;
	case PUMPSEL_PUMP2_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP2_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP2_MASK);
		}
		break;
	case PUMPSEL_PUMP3_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP3_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP3_MASK);
		}
		break;
	case PUMPSEL_PUMP4_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP4_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP4_MASK);
		}
		break;
	case PUMPSEL_PUMP5_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP5_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP5_MASK);
		}
		break;
	case PUMPSEL_PUMP6_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP6_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP6_MASK);
		}
		break;
	case PUMPSEL_PUMP7_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP7_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP7_MASK);
		}
		break;
	case PUMPSEL_PUMP8_BUTTON:
		{
			state ? (pProjectMan->pumpSelPumpSel |= PUMP8_MASK) : (pProjectMan->pumpSelPumpSel &= ~PUMP8_MASK);
		}
		break;
	case PUMPSEL_OK_BUTTON:
		{
//			pProjectMan->proStatus = PROJECTSTATUS_FILLING;//×ªµ½¹à×¢×´Ì¬
//
//			SetScreen(RUNNINGPAGE_INDEX);
//			SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, "Filling the tube");

			pProjectMan->tipsButton = TIPS_OK;
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
