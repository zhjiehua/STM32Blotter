#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"

#include "../User/CPrintf.h"
#include "../UILogic/pageCommon.h"
#include "../Logic/managerment.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0

uint8 cmd_buffer[CMD_MAX_SIZE];

#define TIME_100MS 10

volatile uint32  timer_tick_count = 0; //定时器节拍

uint8 cmd_buffer[CMD_MAX_SIZE];

static int32 test_value = 0;
static uint8 update_en = 0;

void UpdateUI(void);

//程序入口
int main()
{
	qsize  size = 0;
	uint32 timer_tick_last_update = 0;

	/*配置时钟*/
	Set_System();
	/*配置串口中断*/
	Interrupts_Config();
	/*配置时钟节拍*/
	systicket_init();

	/*串口初始化,波特率设置为115200*/
	UartInit(115200);

	/*清空串口接收缓冲区*/
	queue_reset();

	/*延时等待串口屏初始化完毕,必须等待300ms*/
	delay_ms(300);

	while(1)
	{
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
		if(size>0)//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
		}		

		/****************************************************************************************************************
		特别注意
		MCU不要频繁向串口屏发送数据，否则串口屏的内部缓存区会满，从而导致数据丢失(缓冲区大小：标准型8K，基本型4.7K)
		1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
		2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。
		******************************************************************************************************************/

		//TODO: 添加用户代码
		//数据有更新时，每100毫秒刷新一次
		if(update_en&&timer_tick_count-timer_tick_last_update>=TIME_100MS)
		{
			update_en = 0;
			timer_tick_last_update = timer_tick_count;			

			UpdateUI();
		}
	}
}

#endif

/*! 
 *  \brief  消息处理流程，此处一般不需要更改
 *  \param msg 待处理消息
 *  \param size 消息长度
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//指令类型
	uint8 ctrl_msg = msg->ctrl_msg;   //消息的类型
	uint8 control_type = msg->control_type;//控件类型
	uint16 screen_id = PTR2U16(&msg->screen_id_high);//画面ID
	uint16 control_id = PTR2U16(&msg->control_id_high);//控件ID
	uint32 value = PTR2U32(msg->param);//数值

	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://触摸屏按下
	case NOTIFY_TOUCH_RELEASE://触摸屏松开
		//NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://写FLASH成功
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://写FLASH失败
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://读取FLASH成功
		//NotifyReadFlash(1,cmd_buffer+2,size-6);//去除帧头帧尾
		break;
	case NOTIFY_READ_FLASH_FAILD://读取FLASH失败
		//NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://读取RTC时间
		//NotifyReadRTC(cmd_buffer[1],cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//画面ID变化通知
			{
				NotifyScreen(screen_id);
			}
			else
			{
				switch(control_type)
				{
				case kCtrlButton: //按钮控件
					//cDebug("NotifyButton(), %d, %d, %d\n", screen_id, control_id, msg->param[1]);
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://文本控件
					//cDebug("NotifyText(), %d, %d, %s\n", screen_id, control_id, msg->param);
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //进度条控件
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //滑动条控件
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //仪表控件
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://菜单控件
					//cDebug("NotifyMenu(), %d, %d, %d, %d\n", screen_id, control_id, msg->param[0], msg->param[1]);
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://选择控件
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://倒计时控件
					NotifyTimer(screen_id,control_id);
					break;
				default:
					break;
				}
			}			
		}
		break;
	case NOTIFY_RESET:
		cDebug("NOTIFY_RESET\n");
		NotifyReset();
		break;
	case NOTIFY_ONLINE:
		cDebug("NOTIFY_ONLINE\n");
		//pProjectMan->runningType = RUNNING_HOME;
		pProjectMan->lcdNotifyResetFlag = 1;

		//创建线程
		//os_create_task(TASK_HOME);	//创建回原点任务，如果直接执行创建任务，会出现死机
        //xTaskCreate( (TaskFunction_t)HomeTask, "HomeTask", home_TASK_STACKDEP, NULL, home_TASK_PRIORITY, NULL );
    
		//createTask(TASK_HOME);

		break;
	default:
		break;
	}
}

//文本控件显示整数值
void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	char buffer[12] = {0};
	sprintf(buffer,"%ld",value);
	SetTextValue(screen_id,control_id,(uchar *)buffer);
}

//字符串转整数
int32 StringToInt32(uint8 *str)
{
	int32 v = 0;
	sscanf((char *)str,"%ld",&v);
	return v;
}

//文本控件显示浮点数
void SetTextValueFloat(uint16 screen_id, uint16 control_id, float value)
{
	char buffer[12] = {0};
	sprintf(buffer,"%.1f",value);
	SetTextValue(screen_id,control_id,(uchar *)buffer);
}

//字符串转整数
float StringToFloat(uint8 *str)
{
	float v = 0;
	sscanf((char *)str,"%f",&v);//"%lf"
	return v;
}

/*! 
 *  \brief  触摸屏复位通知
 *  \details  触摸屏上电复位或者被动复位后，执行此函数
 */
void NotifyReset(void)
{
	//TODO: 添加用户代码
	SetHandShake();//发送握手命令
}

/*! 
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
void NotifyScreen(uint16 screen_id)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
void NotifyTouchXY(uint8 press,uint16 x,uint16 y)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	//TODO: 添加用户代码
	switch(screen_id)
	{
		case LOGOPAGE_INDEX:
			break;		
		case MAINPAGE_INDEX:
			mainPageButtonProcess(control_id, state);
		break;
		case PROJECTPAGE_INDEX:
			projectPageButtonProcess(control_id, state);
		break;
		case PROJECTEDITPAGE_INDEX:
			projectEditPageButtonProcess(control_id, state);
		break;
		case BACKFLOWPAGE_INDEX:
			backflowPageButtonProcess(control_id, state);
		break;
		case PURGEPAGE_INDEX:
			purgePageButtonProcess(control_id, state);
			break;
		case CALIBRATIONPAGE_INDEX:
			caliPageButtonProcess(control_id, state);
			break;
		case INFORMATIONPAGE_INDEX:
			infoPageButtonProcess(control_id, state);
			break;
		case ACTIONPAGE_INDEX:
			actionPageButtonProcess(control_id, state);
			break;
		case RUNNINGPAGE_INDEX:
			runningPageButtonProcess(control_id, state);
			break;
		case TIPS1PAGE_INDEX:
			tips1PageButtonProcess(control_id, state);
			break;
		case TIPS2PAGE_INDEX:
			tips2PageButtonProcess(control_id, state);
			break;
		case PAUSEPAGE_INDEX:
			pausePageButtonProcess(control_id, state);
			break;
		case SELECTPUMPPAGE_INDEX:
			selectPumpPageButtonProcess(control_id, state);
			break;
		case MANUALPAGE_INDEX:
			manualPageButtonProcess(control_id, state);
			break;
		case MOTORPARAPAGE_INDEX:
			motorParaPageButtonProcess(control_id, state);
			break;
		default:
			cDebug("cmd_process NotifyButton error!\n");
		break;
	}
}

/*! 
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
	//TODO: 添加用户代码
	switch(screen_id)
	{
		case PROJECTPAGE_INDEX:
			projectPageEditProcess(control_id, str);
		break;
		case ACTIONPAGE_INDEX:
			actionPageEditProcess(control_id, str);
			break;
		case PROJECTEDITPAGE_INDEX:
			projectEditPageEditProcess(control_id, str);
			break;
		case CALIBRATIONPAGE_INDEX:
			caliPageEditProcess(control_id, str);
			break;
		case INFORMATIONPAGE_INDEX:
			infoPageEditProcess(control_id, str);
			break;
		case MANUALPAGE_INDEX:
			manualPageEditProcess(control_id, str);
			break;
		case MOTORPARAPAGE_INDEX:
			motorParaPageEditProcess(control_id, str);
			break;
		default:
			cDebug("cmd_process NotifyText error!\n");
		break;
	}
}

/*! 
 *  \brief  进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state)
{
	//TODO: 添加用户代码
	if(state == 1)
		return;

	switch(screen_id)
	{
		case CALIBRATIONPAGE_INDEX:
			caliPageMenuProcess(control_id, item);
		break;
		case ACTIONPAGE_INDEX:
			actionPageMenuProcess(control_id, item);
			break;
		case INFORMATIONPAGE_INDEX:
			infoPageMenuProcess(control_id, item);
			break;
		case MANUALPAGE_INDEX:
			manualPageMenuProcess(control_id, item);
			break;
		default:
			cDebug("cmd_process NotifyMenu error!\n");
		break;
	}
}

/*! 
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
	//TODO: 添加用户代码
	switch(screen_id)
	{
		case RUNNINGPAGE_INDEX:
			runningPageRTCTimeoutProcess(control_id);
			break;
		default:
			break;
	}
}

/*! 
 *  \brief  读取用户FLASH状态返回
 *  \param status 0失败，1成功
 *  \param _data 返回数据
 *  \param length 数据长度
 */
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
void NotifyWriteFlash(uint8 status)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取RTC时间，注意返回的是BCD码
 *  \param year 年（BCD）
 *  \param month 月（BCD）
 *  \param week 星期（BCD）
 *  \param day 日（BCD）
 *  \param hour 时（BCD）
 *  \param minute 分（BCD）
 *  \param second 秒（BCD）
 */
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second)
{
	//TODO: 添加用户代码
}

#ifdef __cplusplus
}
#endif
