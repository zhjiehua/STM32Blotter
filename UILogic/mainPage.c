#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void mainPageButtonProcess(uint16 control_id, uint8  state)
{
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	switch(control_id)
	{
		case MAIN_PROJECT_BUTTON:
			SetTextValue(PROJECTPAGE_INDEX, PRO_PROJECTNAME_EDIT, (uint8_t*)pProjectMan->pCurProject->name);
			SetTextValueInt32(PROJECTPAGE_INDEX, PRO_STARTTANK_EDIT, pProjectMan->startTank);
			SetTextValueInt32(PROJECTPAGE_INDEX, PRO_ENDTANK_EDIT, pProjectMan->endTank);
			break;
		case MAIN_PROEDIT_BUTTON:
			SetTextValue(PROJECTEDITPAGE_INDEX, PROEDIT_PROJECTNAME_EDIT, (uint8_t*)pProjectMan->pCurEditProject->name);
			SetTextValue(PROJECTEDITPAGE_INDEX, PROEDIT_ACTIONNAME_EDIT, (uint8_t*)pProjectMan->pCurEditAction->name);
			break;
		case MAIN_BACKFLOW_BUTTON:
			BatchBegin(BACKFLOWPAGE_INDEX);
			BatchSetButtonValue(BACKFLOW_PUMP1_BUTTON, pProjectMan->backflowPumpSel&PUMP1_MASK);
			BatchSetButtonValue(BACKFLOW_PUMP2_BUTTON, pProjectMan->backflowPumpSel&PUMP2_MASK);
			BatchSetButtonValue(BACKFLOW_PUMP3_BUTTON, pProjectMan->backflowPumpSel&PUMP3_MASK);
			BatchSetButtonValue(BACKFLOW_PUMP4_BUTTON, pProjectMan->backflowPumpSel&PUMP4_MASK);
			BatchSetButtonValue(BACKFLOW_PUMP5_BUTTON, pProjectMan->backflowPumpSel&PUMP5_MASK);
			BatchSetButtonValue(BACKFLOW_PUMP6_BUTTON, pProjectMan->backflowPumpSel&PUMP6_MASK);
			BatchSetButtonValue(BACKFLOW_PUMP7_BUTTON, pProjectMan->backflowPumpSel&PUMP7_MASK);
			BatchSetButtonValue(BACKFLOW_PUMP8_BUTTON, pProjectMan->backflowPumpSel&PUMP8_MASK);
			BatchEnd();
			break;
		case MAIN_PURGE_BUTTON:
			BatchBegin(PURGEPAGE_INDEX);
			BatchSetButtonValue(PURGE_PUMP1_BUTTON, pProjectMan->purgePumpSel&PUMP1_MASK);
			BatchSetButtonValue(PURGE_PUMP2_BUTTON, pProjectMan->purgePumpSel&PUMP2_MASK);
			BatchSetButtonValue(PURGE_PUMP3_BUTTON, pProjectMan->purgePumpSel&PUMP3_MASK);
			BatchSetButtonValue(PURGE_PUMP4_BUTTON, pProjectMan->purgePumpSel&PUMP4_MASK);
			BatchSetButtonValue(PURGE_PUMP5_BUTTON, pProjectMan->purgePumpSel&PUMP5_MASK);
			BatchSetButtonValue(PURGE_PUMP6_BUTTON, pProjectMan->purgePumpSel&PUMP6_MASK);
			BatchSetButtonValue(PURGE_PUMP7_BUTTON, pProjectMan->purgePumpSel&PUMP7_MASK);
			BatchSetButtonValue(PURGE_PUMP8_BUTTON, pProjectMan->purgePumpSel&PUMP8_MASK);
			BatchSetText(PURGE_STATUS_EDIT, (uint8_t*)"");
			BatchEnd();
			break;
		case MAIN_CALI_BUTTON:
			pProjectMan->caliAmount = pProjectMan->pCaliPumpPara[pProjectMan->caliPumpSel];
			SetTextValue(CALIBRATIONPAGE_INDEX, CALI_PUMPSELECT_EDIT, (uint8_t*)caliPumpMenuText[pProjectMan->caliPumpSel]);
			//SetTextValueInt32(CALIBRATIONPAGE_INDEX, CALI_ACTUALAMOUNT_EDIT, 0);
			SetTextValueFloat(CALIBRATIONPAGE_INDEX, CALI_ACTUALAMOUNT_EDIT, pProjectMan->pCaliPumpPara[pProjectMan->caliPumpSel]);
			break;
		case MAIN_INFO_BUTTON:
			SetTextValue(INFORMATIONPAGE_INDEX, INFO_VERSION_EDIT, (uint8_t*)VERSION);
			if(pProjectMan->lang == 0)
				SetTextValue(INFORMATIONPAGE_INDEX, INFO_LANG_EDIT, (uint8_t*)(langMenuText[pProjectMan->lang]));
			else
				SetTextValue(INFORMATIONPAGE_INDEX, INFO_LANG_EDIT, (uint8_t*)(langMenuTextCh[pProjectMan->lang]));
			break;
		default:
			cDebug("mainPage BUTTON error!\n");
			break;
	}
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

#ifdef __cplusplus
}
#endif
