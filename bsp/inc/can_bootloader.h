/**
  ******************************************************************************
  * @file    can_bootloader.h
  * $Author: wdluo $
  * $Revision: 17 $
  * $Date:: 2012-07-06 11:16:48 +0800 #$
  * @brief   ����CAN���ߵ�Bootloader����.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_BOOTLOADER_H
#define __CAN_BOOTLOADER_H
/* Includes ------------------------------------------------------------------*/
#include "user_config.h"
#include "delay.h"
#include "can_driver.h"
/* Private typedef -----------------------------------------------------------*/
#define CMD_WIDTH   4         //��Ҫ�޸�
#define CMD_MASK    0xF       //��Ҫ�޸�
#define CAN_ID_TYPE 1         //1Ϊ��չ֡��0Ϊ��׼֡����Ҫ�޸�
#define ADDR_MASK   0x1FFFFFF //��Ҫ�޸�
#define APP_EXE_FLAG_ADDR          ((uint32_t)0x08007800)
#define APP_START_ADDR             ((uint32_t)0x08008000)
#define APP_EXE_FLAG_START_ADDR    ((uint32_t)0x08004000)
#define CAN_BL_BOOT     0x55555555
#define CAN_BL_APP      0xAAAAAAAA
#define FW_TYPE         CAN_BL_BOOT

typedef struct
{
  //Bootloader�������
  unsigned char Erase;        //����APP������������
  unsigned char WriteInfo;    //���ö��ֽ�д������ز�����д��ʼ��ַ����������
  unsigned char Write;        //�Զ��ֽ���ʽд����
  unsigned char Check;        //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
  unsigned char SetBaudRate;  //���ýڵ㲨����
  unsigned char Excute;       //ִ�й̼�
  //�ڵ㷵��״̬
  unsigned char CmdSuccess;   //����ִ�гɹ�
  unsigned char CmdFaild;     //����ִ��ʧ��
}CBL_CMD_LIST; 
typedef struct
{
	union
	{
		u32 all;
		struct
		{
			u16 cmd:4;
			u16 addr:12;
			u16 reserve:16;
		}bit;
	}ExtId;
	unsigned char IDE;   
	unsigned char DLC;   
	u8 data[8];
}bootloader_data;

uint32_t GetSector(uint32_t Address);
FLASH_Status CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
uint16_t CAN_BOOT_GetAddrData(void);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr);
unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len);
#endif
/*********************************END OF FILE**********************************/

