#ifndef __CAN_BOOTLOADER_H
#define __CAN_BOOTLOADER_H
/* Includes ------------------------------------------------------------------*/
#include "user_config.h"
#include "delay.h"
#include "can_driver.h"
#include "flash.h"
/* Private typedef -----------------------------------------------------------*/
#define CMD_WIDTH   4         //不要修改
#define CMD_MASK    0xF       //不要修改 
#define ADDR_MASK   0x1FFFFFF //不要修改
#define APP_EXE_FLAG_ADDR          ((uint32_t)0x08007800)
#define APP_START_ADDR             ((uint32_t)0x08008000)
#define APP_Write_START_ADDR       ((uint32_t)0x08008000)
#define APP_Write_END_ADDR         ((uint32_t)0x080FFFFF)
#define APP_EXE_FLAG_START_ADDR    ((uint32_t)0x08004000)
#define CAN_BL_APP      0xAAAAAA
#define CAN_BL_BOOT     0x555555 
#define DEVICE_ADDR     0x132 //设备地址
//故障信息表
#define DEVICE_ADDR_ERROR  0xA0
#define ERASE_ERROR        0xA1
#define WRITE_ERROR        0xA2
#define READ_LEN_ERROR     0xA3
#define MSG_DATA_LEN_ERROR 0xA4
#define FILE_TYPE_ERROR    0xA5
#define CRC_ERROR          0xA6
#define FLASH_ADDR_ERROR   0xA7
#define WRITE_LEN_ERROR    0xA8
//----------------------以下宏定义是对芯片型号进行宏定义---------------------------- 
#define STM32F407IGT6     3
//---------------------------------------------------
#define File_None 0xF0
#define File_bin  0xF1
#define File_hex  0xF2
//---------------------------------------------------
typedef  void (*pFunction)(void); 
typedef struct _CBL_CMD_LIST
{
	//Bootloader相关命令
	unsigned char Read;         //读取flash数据
	unsigned char Erase;        //擦出APP储存扇区数据
	unsigned char Write;        //以多字节形式写数据
	unsigned char Check;        //检测节点是否在线，同时返回固件信息
	unsigned char Excute;       //执行固件
	unsigned char WriteInfo;    //设置多字节写数据相关参数（写起始地址，数据量）
	unsigned char SetBaudRate;  //设置节点波特率
	//节点返回状态,关键
	unsigned char CmdFaild;     //命令执行失败
	unsigned char CmdSuccess;   //命令执行成功
}CBL_CMD_LIST; 
typedef struct _bootloader_data
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
//-------------------------------------------------------------------------------
typedef struct _Device_INFO
{
	union
	{
		unsigned short int all;
		struct
		{
			unsigned short int Device_addr:	12;
			unsigned short int reserve:	4;
		}bits;//设备地址
	}Device_addr;
	union
	{
		unsigned long int all;
		struct
		{
			unsigned long int FW_TYPE:24;//固件类型
			unsigned long int Chip_Value:8;//控制器芯片类型
		}bits;
	}FW_TYPE;
		union
	{
		unsigned long  int all;
		struct
		{
			unsigned long  int Version:	7;//固件版本
			unsigned long  int date:	5;//日期
			unsigned long  int month:	4;//月
			unsigned long  int year:	16;//年
		}bits;
	}FW_Version;//固件版本 
}Device_INFO;
extern Device_INFO DEVICE_INFO;
uint32_t GetSector(uint32_t Address);
FLASH_Status CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
uint16_t CAN_BOOT_GetAddrData(void);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr);
unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len);
#endif
/*********************************END OF FILE**********************************/

