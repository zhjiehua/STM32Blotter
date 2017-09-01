#ifdef __cplusplus	  
extern "C" {
#endif

#include "project.h"
#include "managerment.h"
#include "CPrintf.h"
#include "stdio.h"

#include "../HARDWARE/RelayMOS/RelayMOS.h"
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../Logic/motorManagerment.h"
	
#include "FreeRTOS.h"
#include "task.h"
    
//��ȡ��Һ
void wasteFluidAbsorb(uint8_t *pStopFlag)
{
	uint8_t relCoord;

	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	//״̬��ʾ������
	if(pProjectMan->lang == 0)
	{
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"In absorbing waste fluid����");
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"In absorbing waste fluid����");
	}
	else
	{
		SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"��ȡ��Һ�С���");
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"��ȡ��Һ�С���");
	}
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//����Һ��ת����Һ��λ��
	relCoord = Turntable_Abs2Rel(AbsCoordinate[8]); //��Һ��λ��
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //����Һ��ת����Һ��

	//������ձ�
	MOS2 = 1;
	
	//�ɿ��йܷ�
	RELAY = 1;

	//��ʱ����շ�Һƿ�Ŀ����γ���ѹ��
	vTaskDelay(255*6);

	//���·�Һ��
	WastePump_SetPos(DOWN, pMotorMan->motorParaWastePumpSpeed, pStopFlag);

	//��ʱһС��ʱ��
	vTaskDelay(255*13);

	//�����Һ��
	WastePump_SetPos(UP, pMotorMan->motorParaWastePumpSpeed, pStopFlag);

	//��ʱ����ȡʣ�µ�Һ��
	vTaskDelay(255*6);

	//�رռйܷ�
	RELAY = 0;
	//ֹͣ��ձ�
	MOS2 = 0;

	//״̬��ʾ������
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(RUNNINGPAGE_INDEX, RUNNING_STATUS_EDIT, (uint8_t*)"");
	SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
	xSemaphoreGive(pProjectMan->lcdUartSem);
}


 //��ϴ����
void PurgeProgram(uint8_t *pStopFlag)
{
	uint16_t i;
	uint8_t relCoord;
	uint8_t str[50];
	
	if(pProjectMan->purgePumpSel == 0x00)
	{
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetButtonValue(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0x00); //���ð���Ϊ����״̬
		xSemaphoreGive(pProjectMan->lcdUartSem);
		return;
	}

	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetControlVisiable(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0);
	SetControlVisiable(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP1_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP2_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP3_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP4_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP5_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP6_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP7_BUTTON, 0);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP8_BUTTON, 0);

	if(pProjectMan->lang == 0)
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"In preparing����");
	else
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"׼���С���");

	//��ʾ��Ҫ��ϴ�Ĺܵ���������ˮ��
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the tube that to be washed in the distilled water");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"�뽫Ҫ��ϴ�Ĺܵ���������ˮ��");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
		
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//��ת����ϴҳ��
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//��ԭ��
	Turntable_Home(pStopFlag);

	//���ֶ����ԭλ
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]);
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[9]Ϊ�ֶ���

	//��ʾ����Һ�����ڲ۰��1��λ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ1ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the Waste Tank to the plate1");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"�뽫��Һ�����ڡ����1��λ��");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//��ת����ϴҳ��

	if(pProjectMan->lang == 0)
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"Preparation finish!");
	else
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"׼����ɣ�");
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//��ϴҪ��ϴ�Ĺܵ�
	Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
	for(i=0;i<8;i++)
	{
		if(pProjectMan->purgePumpSel & (0x01<<i))
		{
			if(pProjectMan->lang == 0)
				sprintf((char*)str, "Washing the tube %d ����", i+1);
			else
				sprintf((char*)str, "��ϴ���ܵ�%d���С���", i+1);
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)str);
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//����Һ��ת����Ӧ�ı�λ��
			relCoord = Turntable_Abs2Rel(AbsCoordinate[i]);
			Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[8]Ϊ��Һ��

            DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);            
			vTaskDelay(4000);
            DCMotor_Stop((DCMotorEnum_TypeDef)i);

			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
	}
	Turntable_SetOffset(pMotorMan->motorParaWastePumpCalib);
	
	//��ȡ��Һ
	wasteFluidAbsorb(pStopFlag);

	//��ʾ��Ҫ��ϴ�Ĺܵ����������ﴦ
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ2ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please place the tubes that to be washed in the clean and dry place");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"�뽫Ҫ��ϴ�Ĺܵ����������ﴦ");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//��ת����ϴҳ��
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//���Ҫ��ϴ�Ĺܵ�
	Turntable_SetOffset(pMotorMan->motorParaCCWPumpCalib);
	for(i=0;i<8;i++)
	{
		if(pProjectMan->purgePumpSel & (0x01<<i))
		{
			if(pProjectMan->lang == 0)
				sprintf((char*)str, "Pumping water from the tube %d", i+1);
			else
				sprintf((char*)str, "��ա��ܵ�%d���С���", i+1);
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)str);
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			//����Һ��ת����Ӧ�ı�λ��
			relCoord = Turntable_Abs2Rel(AbsCoordinate[i]);
			Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //AbsCoordinate[8]Ϊ3�Ųۣ�����Һ�ۣ�

			DCMotor_Run((DCMotorEnum_TypeDef)i, CW, pMotorMan->motorParaPumpSpeed);			
			vTaskDelay(4000);
			DCMotor_Stop((DCMotorEnum_TypeDef)i);

			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"");
			xSemaphoreGive(pProjectMan->lcdUartSem);
		}
	}
	Turntable_SetOffset(pMotorMan->motorParaWastePumpCalib);

	//��ȡ��Һ
	wasteFluidAbsorb(pStopFlag);

	//����Һ��ת���ֶ���
	relCoord = Turntable_Abs2Rel(AbsCoordinate[9]); //�ֶ���λ��
	Turntable_RelativePosition(relCoord, AbsCoordinate[9], pStopFlag); //����Һ��ת���ֶ���

	//��ʾȡ�߷�Һ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ2ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Please take away the Waste Tank");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"�����߷�Һ��");
	
	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//��ת����ϴҳ��

	//��ʾ��ϴ���
	SetScreen(TIPS1PAGE_INDEX);//��ת����ʾ2ҳ��
	if(pProjectMan->lang == 0)
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"Wash finish!");
	else
		SetTextValue(TIPS1PAGE_INDEX, TIPS1_TIPS_EDIT, (uint8_t*)"��ϴ���!");

	SetBuzzer(BEEPER_TIME_SHORT);
	xSemaphoreGive(pProjectMan->lcdUartSem);
	while(pProjectMan->tipsButton == TIPS_NONE)
		vTaskDelay(10);
	
	pProjectMan->tipsButton = TIPS_NONE;
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetScreen(PURGEPAGE_INDEX);//��ת����ϴҳ��

	if(pProjectMan->lang == 0)
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"Wash finish!");
	else
		SetTextValue(PURGEPAGE_INDEX, PURGE_STATUS_EDIT, (uint8_t*)"��ϴ���!");

	SetButtonValue(PURGEPAGE_INDEX, PURGE_START_BUTTON, 0x00); //���ð���Ϊ����״̬
	SetControlVisiable(PURGEPAGE_INDEX, PURGE_START_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP1_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP2_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP3_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP4_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP5_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP6_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP7_BUTTON, 1);
	SetControlEnable(PURGEPAGE_INDEX, PURGE_PUMP8_BUTTON, 1);
	xSemaphoreGive(pProjectMan->lcdUartSem);
}

#ifdef __cplusplus
}
#endif
