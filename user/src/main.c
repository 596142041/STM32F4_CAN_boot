#include "main.h"
#include "can_bootloader.h"
extern CanRxMsg CAN1_RxMessage;
extern uint8_t	data_temp[128];
extern volatile uint8_t CAN1_CanRxMsgFlag;//接收到CAN数据后的标志
int main(void)
{
	if(*((uint32_t *)APP_EXE_FLAG_ADDR)==0x78563412)
	{
	CAN_BOOT_JumpToApplication(APP_START_ADDR);
	}
	delay_init(168);
	delay_ms(100);
	__set_PRIMASK(0);//开启总中断
	CAN_Configuration(250000); 
	while (1)
	{
		if(CAN1_CanRxMsgFlag)
		{
			CAN_BOOT_ExecutiveCommand(&CAN1_RxMessage);
			CAN1_CanRxMsgFlag = 0;
		}
	}
} 
/****************************************************************************************************
After Build - User command #1: fromelf.exe --bin -o ..\output\BootLoader.bin ..\output\BootLoader.axf
******************************************************************************************************/
