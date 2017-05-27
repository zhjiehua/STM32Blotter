#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"
#include "../HARDWARE/DCMotor/DCMotor.h"

#ifdef __cplusplus
extern "C" {
#endif

void backflowPageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
		case BACKFLOW_PUMP1_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP1_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP1_MASK);
			}
			//松开夹管阀
			//pDCMotor->SetSpeed(PUMP_PINCH, 0);
            DCMotor_Run(PUMP_PINCH, CW, 10);
			break;
		case BACKFLOW_PUMP2_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP2_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP2_MASK);
			}
			//松开夹管阀
			//pDCMotor->SetSpeed(PUMP_PINCH, 100);
            DCMotor_Run(PUMP_PINCH, CW, 100);
			break;
		case BACKFLOW_PUMP3_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP3_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP3_MASK);
			}
			break;
		case BACKFLOW_PUMP4_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP4_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP4_MASK);
			}
			break;
		case BACKFLOW_PUMP5_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP5_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP5_MASK);
			}
			break;
		case BACKFLOW_PUMP6_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP6_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP6_MASK);
			}
			break;
		case BACKFLOW_PUMP7_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP7_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP7_MASK);
			}
			break;
		case BACKFLOW_PUMP8_BUTTON:
			{
				state ? (pProjectMan->backflowPumpSel |= PUMP8_MASK) : (pProjectMan->backflowPumpSel &= ~PUMP8_MASK);
			}
			break;
		case BACKFLOW_OK_BUTTON:
			{
				cDebug("running backflow program!\n");
			}
			break;
		case BACKFLOW_BACK_BUTTON:
			break;
		default:
			cDebug("backflowPage BUTTON error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
