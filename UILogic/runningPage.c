#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void runningPageButtonProcess(uint16 control_id, uint8  state)
{
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	switch(control_id)
	{
	case RUNNING_PAUSE_BUTTON:
		if(pProjectMan->lang == 0)
			SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t *)"Are you really want to PAUSE the program?");
		else
			SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t *)"ÊÇ·ñ¡¾ÔÝÍ£¡¿³ÌÐò£¿");
		//pProjectMan->exception = EXCEPTION_PAUSE;
		pProjectMan->exceptionButtonFlag = EXCEPTION_PAUSE;
		SetScreen(TIPS2PAGE_INDEX);
		break;
	case RUNNING_STOP_BUTTON:
		if(pProjectMan->lang == 0)
			SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t *)"Are you really want to STOP the program?");
		else
			SetTextValue(TIPS2PAGE_INDEX, TIPS2_TIPS_EDIT, (uint8_t *)"ÊÇ·ñ¡¾Í£Ö¹¡¿³ÌÐò£¿");
		//pProjectMan->exception = EXCEPTION_STOP;
		pProjectMan->exceptionButtonFlag = EXCEPTION_STOP;
		SetScreen(TIPS2PAGE_INDEX);
		break;
		default:
			cDebug("runningPage BUTTON error!\n");
		break;
	}
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

void runningPageRTCTimeoutProcess(uint16 control_id)
{
	pProjectMan->RTCTimeout = 1;
	//cDebug("========runningPage RTC timeout!\n");
}

#ifdef __cplusplus
}
#endif
