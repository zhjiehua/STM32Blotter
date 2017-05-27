#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HardwareCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

void infoPageButtonProcess(uint16 control_id, uint8  state)
{
	switch(control_id)
	{
		case INFO_LANG_BUTTON:
		break;
		case INFO_BACK_BUTTON:
		break;
		default:
			cDebug("infoPage BUTTON error!\n");
		break;
	}
}

void infoPageMenuProcess(uint16 control_id, uint8 item)
{
	switch(control_id)
	{
	case INFO_LANG_MENU:
		{
			pProjectMan->lang = (Language_TypeDef)item;
			if(item == 0)
			{
				SetLanguage(0, 1);
				SetTextValue(INFORMATIONPAGE_INDEX, INFO_LANG_EDIT, (uint8_t*)langMenuText[pProjectMan->lang]);
			}
			else
			{
				SetLanguage(1, 0);
				SetTextValue(INFORMATIONPAGE_INDEX, INFO_LANG_EDIT, (uint8_t*)langMenuTextCh[pProjectMan->lang]);
			}
			AT24CXX_Write(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), LANGUAGE_SIZE);
		}
		break;
	default:
		cDebug("infoPage MENU error!\n");
		break;
	}
}

void infoPageEditProcess(uint16 control_id, uint8 *str)
{
	switch(control_id)
	{
		case INFO_POSCALI1_EDIT:
			//cDebug("pProjectMan->posCali1 = %d\n", pProjectMan->posCali1);
			pProjectMan->posCali1 = (uint16_t)StringToInt32(str);
			AT24CXX_Write(POSCALI_BASEADDR, (uint8_t*)(&pProjectMan->posCali1), 2);
			
		break;
		case INFO_POSCALI2_EDIT:
			//cDebug("pProjectMan->posCali2 = %d\n", pProjectMan->posCali2);
			pProjectMan->posCali2 = (uint16_t)StringToInt32(str);
			AT24CXX_Write(POSCALI_BASEADDR+2, (uint8_t*)(&pProjectMan->posCali2), 2);
			
		break;						   
		default:
			cDebug("infoPage EDIT error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
