#ifndef _PAGE_COMMON_H_
#define _PAGE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "../HMI/hmi_user_uart.h"
#include "../HMI/hmi_driver.h"
#include "../HMI/cmd_process.h"

/************************************************************************/
/* 页面ID定义                                                           */
/************************************************************************/
#define LOGOPAGE_INDEX					0
#define MAINPAGE_INDEX					1
#define PROJECTPAGE_INDEX				2
#define PROJECTEDITPAGE_INDEX			3
#define BACKFLOWPAGE_INDEX				4
#define PURGEPAGE_INDEX					5
#define CALIBRATIONPAGE_INDEX			6
#define INFORMATIONPAGE_INDEX			7
#define ACTIONPAGE_INDEX				8
#define RUNNINGPAGE_INDEX				9
#define PAUSEPAGE_INDEX					10
#define SELECTPUMPPAGE_INDEX			11
#define TIPS0PAGE_INDEX					12
#define TIPS1PAGE_INDEX					13
#define TIPS2PAGE_INDEX					14

/************************************************************************/
/* LOGO页面控件ID                                                         */
/************************************************************************/
#define LOGO_START_BUTTON				1
#define LOGO_STATUS_EDIT				2

/************************************************************************/
/* 主页面控件ID                                                         */
/************************************************************************/
#define MAIN_PROJECT_BUTTON				1
#define MAIN_PROEDIT_BUTTON				2
#define MAIN_BACKFLOW_BUTTON			3
#define MAIN_PURGE_BUTTON				4
#define MAIN_CALI_BUTTON				5
#define MAIN_INFO_BUTTON				6

/************************************************************************/
/* 项目页面控件ID                                                       */
/************************************************************************/
#define PRO_PROJECTNAME_EDIT			1
#define PRO_PREPROJECT_BUTTON			2
#define PRO_POSTPROJECT_BUTTON			3
#define PRO_STARTTANK_EDIT				4
#define PRO_ENDTANK_EDIT				5
#define PRO_STARTPROJECT_BUTTON			6
#define PRO_BACK_BUTTON					50

/************************************************************************/
/* 项目编辑页面控件ID                                                   */
/************************************************************************/
#define PROEDIT_PROJECTNAME_EDIT		1
#define PROEDIT_PREPROJECT_BUTTON		2
#define PROEDIT_POSTPROJECT_BUTTON		3
#define PROEDIT_ACTIONNAME_EDIT			4
#define PROEDIT_PREACTION_BUTTON		5
#define PROEDIT_POSTACTION_BUTTON		6
#define PROEDIT_EDITACTION_BUTTON		7
#define PROEDIT_BACK_BUTTON				50

/************************************************************************/
/* 回流页面控件ID                                                       */
/************************************************************************/
#define BACKFLOW_PUMP1_BUTTON			1
#define BACKFLOW_PUMP2_BUTTON			2
#define BACKFLOW_PUMP3_BUTTON			3
#define BACKFLOW_PUMP4_BUTTON			4
#define BACKFLOW_PUMP5_BUTTON			5
#define BACKFLOW_PUMP6_BUTTON			6
#define BACKFLOW_PUMP7_BUTTON			7
#define BACKFLOW_PUMP8_BUTTON			8
#define BACKFLOW_OK_BUTTON				9
#define BACKFLOW_BACK_BUTTON			50

/************************************************************************/
/* 清洗页面控件ID                                                       */
/************************************************************************/
#define PURGE_PUMP1_BUTTON				1
#define PURGE_PUMP2_BUTTON				2
#define PURGE_PUMP3_BUTTON				3
#define PURGE_PUMP4_BUTTON				4
#define PURGE_PUMP5_BUTTON				5
#define PURGE_PUMP6_BUTTON				6
#define PURGE_PUMP7_BUTTON				7
#define PURGE_PUMP8_BUTTON				8
#define PURGE_START_BUTTON				9
#define PURGE_STATUS_EDIT				10
#define PURGE_BACK_BUTTON				50

/************************************************************************/
/* 校准页面控件ID	                                                    */
/************************************************************************/
#define CALI_PUMPSELECT_EDIT			1
#define CALI_PUMPSELECT_BUTTON			2
#define CALI_PUMPSELECT_MENU			3
#define CALI_START_BUTTON				4
#define CALI_ACTUALAMOUNT_EDIT			5
#define CALI_OK_BUTTON					6
#define CALI_BACK_BUTTON				50

/************************************************************************/
/* 信息页面控件ID	                                                    */
/************************************************************************/
#define INFO_VERSION_EDIT				1
#define INFO_LANG_EDIT					2
#define INFO_LANG_BUTTON				3
#define INFO_LANG_MENU					4
#define INFO_POSCALI1_EDIT				5
#define INFO_POSCALI2_EDIT				6
#define INFO_BACK_BUTTON				50

/************************************************************************/
/* 动作编辑页面控件ID	                                                */
/************************************************************************/
#define ACTEDIT_PUMPSEL_EDIT			1
#define ACTEDIT_PUMPSEL_BUTTON			2
#define ACTEDIT_PUMPSEL_MENU			3
#define ACTEDIT_TIPSSEL_EDIT			4
#define ACTEDIT_TIPSSEL_BUTTON			5
#define ACTEDIT_TIPSSEL_MENU			6
#define ACTEDIT_VOICESEL_EDIT			7
#define ACTEDIT_VOICESEL_BUTTON			8
#define ACTEDIT_VOICESEL_MENU			9
#define ACTEDIT_ADDAMOUNT_EDIT			10
#define ACTEDIT_IMBIAMOUNT_EDIT			11
#define ACTEDIT_SPEEDSEL_EDIT			12
#define ACTEDIT_SPEEDSEL_BUTTON			13
#define ACTEDIT_SPEEDSEL_MENU			14
#define ACTEDIT_TIMEHOUR_EDIT			15
#define ACTEDIT_TIMEMIN_EDIT			16
#define ACTEDIT_LOOPTIME_EDIT			17
#define ACTEDIT_SAVE_BUTTON				18
#define ACTEDIT_BACK_BUTTON				50

/************************************************************************/
/* 运行页面控件ID	                                                    */
/************************************************************************/
#define RUNNING_PROJECT_EDIT			1
#define RUNNING_ACTION_EDIT				2
#define RUNNING_PUMP_EDIT				3
#define RUNNING_ADDTANK_EDIT			4
#define RUNNING_IMBITANK_EDIT			5
#define RUNNING_LOOPTIME_EDIT			6
#define RUNNING_TOTALLOOPTIME_EDIT		7
#define RUNNING_TIME_RTC				8
#define RUNNING_PAUSE_BUTTON			9
#define RUNNING_STOP_BUTTON				10
#define RUNNING_STATUS_EDIT				11

/************************************************************************/
/* 暂停页面控件ID	                                                    */
/************************************************************************/
#define PAUSE_ACTIONNAME_EDIT				1
#define PAUSE_PREACTION_BUTTON			2
#define PAUSE_POSTACTION_BUTTON			3
#define PAUSE_JUMPTO_BUTTON				4
#define PAUSE_ROTATE_BUTTON				5
#define PAUSE_RESUME_BUTTON				6

/************************************************************************/
/* 选择泵页面控件ID                                                     */
/************************************************************************/
#define PUMPSEL_PUMP1_BUTTON			1
#define PUMPSEL_PUMP2_BUTTON			2
#define PUMPSEL_PUMP3_BUTTON			3
#define PUMPSEL_PUMP4_BUTTON			4
#define PUMPSEL_PUMP5_BUTTON			5
#define PUMPSEL_PUMP6_BUTTON			6
#define PUMPSEL_PUMP7_BUTTON			7
#define PUMPSEL_PUMP8_BUTTON			8
#define PUMPSEL_OK_BUTTON				9

/************************************************************************/
/* 提示0页面控件ID                                                      */
/************************************************************************/
#define TIPS0_TIPS_EDIT					1

/************************************************************************/
/* 提示1页面控件ID                                                      */
/************************************************************************/
#define TIPS1_TIPS_EDIT					1
#define TIPS1_OK_BUTTON					2

/************************************************************************/
/* 提示2页面控件ID                                                      */
/************************************************************************/
#define TIPS2_TIPS_EDIT					1
#define TIPS2_OK_BUTTON					2
#define TIPS2_CANCEL_BUTTON				3

/************************************************************************/
/* 泵选择MASK                                                           */
/************************************************************************/
#define PUMP1_MASK						0x01
#define PUMP2_MASK						0x02
#define PUMP3_MASK						0x04
#define PUMP4_MASK						0x08
#define PUMP5_MASK						0x10
#define PUMP6_MASK						0x20
#define PUMP7_MASK						0x40
#define PUMP8_MASK						0x80

/************************************************************************/
/* 菜单选择控件使用                                                     */
/************************************************************************/
#define MENU_DISABLE					0x00
#define MENU_ENABLE						0x01

/************************************************************************/
/* 软件版本                                                             */
/************************************************************************/
#define PANEL_VERSION					"9.1.7"
#define VERSION							"0.1.0"

/*
 * 下拉列表字符串声明
 */
extern const char caliPumpMenuText[8][3];
extern const char actionPumpMenuText[9][3];
extern const char actionTipsMenuText[3][9];
extern const char actionTipsMenuTextCh[3][5];
extern const char actionVoiceMenuText[4][7];
extern const char actionVoiceMenuTextCh[4][3];
extern const char actionSpeedMenuText[3][7];
extern const char actionSpeedMenuTextCh[3][5];
extern const char langMenuText[2][8];
extern const char langMenuTextCh[2][5];
/*
 * 函数声明
 */
void mainPageButtonProcess(uint16 control_id, uint8  state);

void projectPageButtonProcess(uint16 control_id, uint8  state);
void projectPageEditProcess(uint16 control_id, uint8 *str);

void projectEditPageButtonProcess(uint16 control_id, uint8  state);
void projectEditPageEditProcess(uint16 control_id, uint8 *str);

void runningPageButtonProcess(uint16 control_id, uint8  state);
void runningPageRTCTimeoutProcess(uint16 control_id);

void backflowPageButtonProcess(uint16 control_id, uint8  state);

void purgePageButtonProcess(uint16 control_id, uint8  state);

void caliPageButtonProcess(uint16 control_id, uint8  state);
void caliPageEditProcess(uint16 control_id, uint8 *str);
void caliPageMenuProcess(uint16 control_id, uint8 item);

void infoPageButtonProcess(uint16 control_id, uint8  state);
void infoPageMenuProcess(uint16 control_id, uint8 item);
void infoPageEditProcess(uint16 control_id, uint8 *str);

void actionPageButtonProcess(uint16 control_id, uint8  state);
void actionPageEditProcess(uint16 control_id, uint8 *str);
void actionPageMenuProcess(uint16 control_id, uint8 item);

void runningPageButtonProcess(uint16 control_id, uint8  state);

void tips1PageButtonProcess(uint16 control_id, uint8  state);
void tips2PageButtonProcess(uint16 control_id, uint8  state);

void selectPumpPageButtonProcess(uint16 control_id, uint8  state);
void pausePageButtonProcess(uint16 control_id, uint8  state);

#ifdef __cplusplus
}
#endif

#endif
