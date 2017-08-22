#include "can_bootloader.h"
//命令参数必须跟上位机软件的命令参数一致
CBL_CMD_LIST CMD_List = 
{
  .Erase         = 0x00,      //擦除APP区域数据
  .WriteInfo     = 0x01,      //设置多字节写数据相关参数（写起始地址，数据量）
  .Write         = 0x02,      //以多字节形式写数据
  .Check         = 0x03,      //检测节点是否在线，同时返回固件信息
  .SetBaudRate   = 0x04,      //设置节点波特率
  .Excute        = 0x05,      //执行固件
  .CmdSuccess    = 0x08,      //命令执行成功
  .CmdFaild      = 0x09,      //命令执行失败
};
typedef  void (*pFunction)(void);
#define Buffer_size 80
uint8_t	data_temp[128];
uint8_t can_cmd    = 0x00;//ID的bit0~bit3位为命令码
uint16_t can_addr  = 0x00;//ID的bit4~bit15位为节点地址
uint32_t BaudRate;
uint16_t crc_data;
uint32_t addr_offset;
uint32_t exe_type = 0x00;
uint32_t FlashSize;
uint32_t start_addr = APP_START_ADDR;
uint32_t data_size=0;
uint32_t data_index=0;
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }

  return sector;
}
/**
  * @brief  将数据烧写到指定地址的Flash中 。
  * @param  Address Flash起始地址。
  * @param  Data 数据存储区起始地址。
  * @param  DataNum 数据字节数。
  * @retval 数据烧写状态。
  */
FLASH_Status CAN_BOOT_ProgramDatatoFlash(uint32_t StartAddr,uint8_t *pData,uint32_t DataNum) 
{
	FLASH_Status FLASHStatus=FLASH_COMPLETE;
	uint32_t *pDataTemp=(uint32_t *)pData;
	uint32_t i;
	if(StartAddr<APP_EXE_FLAG_START_ADDR)
	{
		return FLASH_ERROR_PGS;
	}
  /* Clear pending flags (if any) */  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	for(i=0;i<(DataNum>>2);i++)
	{
		FLASHStatus = FLASH_ProgramWord(StartAddr, *pDataTemp);
		if (FLASHStatus == FLASH_COMPLETE)
		{
			StartAddr += 4;
			pDataTemp++;
		}
		else
		{ 
			return FLASHStatus;
		}
	}
  return	FLASHStatus;
}
/**
  * @brief  擦出指定扇区区间的Flash数据 。
  * @param  StartPage 起始扇区地址
  * @param  EndPage 结束扇区地址
  * @retval 扇区擦出状态  
  */
FLASH_Status CAN_BOOT_ErasePage(uint32_t StartAddr,uint32_t EndAddr)
{ 
	FLASH_Status FLASHStatus=FLASH_COMPLETE;
	uint32_t StartSector, EndSector;
	uint32_t SectorCounter=0; 
	/* Get the number of the start and end sectors */
	StartSector = GetSector(StartAddr);
	EndSector = GetSector(EndAddr);

	if(StartAddr<APP_EXE_FLAG_START_ADDR)
	{
		return FLASH_ERROR_PGS;
	}
  /* Clear pending flags (if any) */  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	for (SectorCounter = StartSector; SectorCounter <= EndSector; SectorCounter += 8)
	{
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word */ 
		FLASHStatus = FLASH_EraseSector(SectorCounter, VoltageRange_3);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			return FLASHStatus;
		}
	}
	return FLASHStatus;
}

/**
  * @brief  获取节点地址信息
  * @param  None
  * @retval 节点地址。
  */
uint16_t CAN_BOOT_GetAddrData(void)
{
  return Read_CAN_Address();
}
/**
  * @brief  控制程序跳转到指定位置开始执行 。
  * @param  Addr 程序执行地址。
  * @retval 程序跳转状态。
  */
void CAN_BOOT_JumpToApplication(uint32_t Addr)
{
  static pFunction Jump_To_Application;
  __IO uint32_t JumpAddress; 
  __set_PRIMASK(1);//关闭所有中断
  /* Test if user code is programmed starting from address "ApplicationAddress" */
  if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)
  { 
    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (Addr + 4);
    Jump_To_Application = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*)Addr);
    Jump_To_Application();
  }
}


/**
  * @brief  执行主机下发的命令
  * @param  pRxMessage CAN总线消息
  * @retval 无
  */
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
	u8 i;
	CanTxMsg TxMessage;
	FLASH_Status ret;
	bootloader_data Boot_ID_info;
	//判断接收的数据地址是否和本节点地址匹配，若不匹配则直接返回，不做任何事情
	if((can_addr!=CAN_BOOT_GetAddrData())&&(can_addr!=0))
	{
		return;
	}
	TxMessage.DLC = 1;
	TxMessage.ExtId = 0;
	TxMessage.IDE = CAN_Id_Extended;
	TxMessage.RTR = CAN_RTR_Data;
	//获取地址信息
	Boot_ID_info.ExtId.all = pRxMessage->ExtId;
	can_cmd = Boot_ID_info.ExtId.bit.cmd;
	can_addr = Boot_ID_info.ExtId.bit.addr;
	//CMD_List.Erase，擦除Flash中的数据，需要擦除的Flash大小存储在Data[0]到Data[3]中
	//该命令必须在Bootloader程序中实现，在APP程序中可以不用实现
	//在DSP部分需要处理该命令
	if(can_cmd == CMD_List.Erase)
	{
		__set_PRIMASK(1);
		FLASH_Unlock();
		FlashSize = (pRxMessage->Data[0]<<24)|(pRxMessage->Data[1]<<16)|(pRxMessage->Data[2]<<8)|(pRxMessage->Data[3]<<0);
		ret =  CAN_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_START_ADDR+FlashSize);
		FLASH_Lock();	
		__set_PRIMASK(0);
		if(can_addr != 0x00)
		{
			if(ret==FLASH_COMPLETE)
			{
				TxMessage.ExtId = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
				TxMessage.DLC = 0;
				TxMessage.Data[0] = 1;				
			}
			else
			{
				TxMessage.ExtId = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdFaild;
				TxMessage.DLC = 1;
				TxMessage.Data[0] = 1;
			} 
			CAN_WriteData(&TxMessage);
		}
		start_addr = APP_START_ADDR;
		return;
	}
	//CMD_List.SetBaudRate，设置节点波特率，具体波特率信息存储在Data[0]到Data[3]中
	//更改波特率后，适配器也需要更改为相同的波特率，否则不能正常通信
	if(can_cmd == CMD_List.SetBaudRate)
	{
		__set_PRIMASK(1);
		BaudRate = (pRxMessage->Data[0]<<24)|(pRxMessage->Data[1]<<16)|(pRxMessage->Data[2]<<8)|(pRxMessage->Data[3]<<0);
		__set_PRIMASK(0);
		CAN_Configuration(BaudRate);
		if(can_addr != 0x00)
		{
			TxMessage.ExtId = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.DLC = 0;
			delay_ms(20); 
			TxMessage.Data[0] = 2;
			CAN_WriteData(&TxMessage);
		}
		return;
	}
	//CMD_List.WriteInfo，设置写Flash数据的相关信息，比如数据起始地址，数据大小
	//数据偏移地址存储在Data[0]到Data[3]中,该偏移量表示当前数据包针针对文件起始的偏移量,同时还表示写入FLASH的偏移
	//  数据大小存储在Data[4]到Data[7]中，该函数必须在Bootloader程序中实现，APP程序可以不用实现
	if(can_cmd == CMD_List.WriteInfo)
	{
		__set_PRIMASK(1);
		addr_offset  = (pRxMessage->Data[0]<<24)|(pRxMessage->Data[1]<<16)|(pRxMessage->Data[2]<<8)|(pRxMessage->Data[3]<<0);
		start_addr   = APP_START_ADDR+addr_offset;
		data_size    = (pRxMessage->Data[4]<<24)|(pRxMessage->Data[5]<<16)|(pRxMessage->Data[6]<<8)|(pRxMessage->Data[7]<<0);
		data_index   = 0;
		__set_PRIMASK(0);
		if(can_addr != 0x00)
		{
			TxMessage.ExtId   = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.DLC     = 0; 
			TxMessage.Data[0] = 3;
			CAN_WriteData(&TxMessage);	 
		}
	}
	//CMD_List.Write，先将数据存储在本地缓冲区中，然后计算数据的CRC，若校验正确则写数据到Flash中
	//每次执行该数据，数据缓冲区的数据字节数会增加pRxMessage->DLC字节，
	//当数据量达到data_size（包含2字节CRC校验码）字节后
	//对数据进行CRC校验，若数据校验无误，则将数据写入Flash中
	//该函数在Bootloader程序中必须实现，APP程序可以不用实现
	if(can_cmd == CMD_List.Write)
	{
		if((data_index<data_size)&&(data_index<1026))
		{
			__set_PRIMASK(1);
			for(i=0;i<pRxMessage->DLC;i++)
			{
				data_temp[data_index++] = pRxMessage->Data[i];
			}
			__set_PRIMASK(0);
		}
		if((data_index>=data_size)||(data_index>=(Buffer_size-2)))
		{
			crc_data = CRCcalc16(data_temp,data_size-2);//对接收到的数据做CRC校验，保证数据完整性
			if(crc_data==((data_temp[data_size-2])|(data_temp[data_size-1]<<8)))
			{
				__set_PRIMASK(1);
				FLASH_Unlock();
				ret =  CAN_BOOT_ProgramDatatoFlash(start_addr,data_temp,data_size-2);
				FLASH_Lock();	
				__set_PRIMASK(0);
				if(can_addr != 0x00)
				{
					if(ret==FLASH_COMPLETE)//FLASH写入成功,再次进行CRC校验
					{
							crc_data = CRCcalc16((unsigned char*)(start_addr),data_size-2);//再次对写入Flash中的数据进行CRC校验，确保写入Flash的数据无误
							if(crc_data!=((data_temp[data_size-2])|(data_temp[data_size-1]<<8)))
							{
								TxMessage.ExtId   = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdFaild;
								TxMessage.DLC     = 1;
								TxMessage.Data[0] = 4;
							}
							else
							{
								TxMessage.ExtId    = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
								TxMessage.DLC      = 0;
								TxMessage.Data[0]  = 0;
							}
							CAN_WriteData(&TxMessage);
					}
					else
					{
						TxMessage.ExtId   = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdFaild;
						TxMessage.DLC     = 1;
						TxMessage.Data[0] = 5;  
						CAN_WriteData(&TxMessage);
					}
				}
				else
				{
					TxMessage.ExtId   = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdFaild;
					TxMessage.DLC     = 1;
					TxMessage.Data[0] = 6;
					CAN_WriteData(&TxMessage);
				}
			}
		return;
		}
	}
	//CMD_List.Check，节点在线检测
	//节点收到该命令后返回固件版本信息和固件类型，
	//该命令在Bootloader程序和APP程序都必须实现 
	
	if(can_cmd == CMD_List.Check)//DSP中尚未实现,相对比较容易实现,主要是为实现APP再次更新应用程序
	{
		if(can_addr != 0x00)
		{
			TxMessage.ExtId = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.Data[0] = 0;//主版本号，两字节
			TxMessage.Data[1] = 1;
			TxMessage.Data[2] = 0;//次版本号，两字节
			TxMessage.Data[3] = 0;
			TxMessage.Data[4] = (uint8_t)(FW_TYPE>>24);
			TxMessage.Data[5] = (uint8_t)(FW_TYPE>>16);
			TxMessage.Data[6] = (uint8_t)(FW_TYPE>>8);
			TxMessage.Data[7] = (uint8_t)(FW_TYPE>>0);
			TxMessage.DLC = 8;
			CAN_WriteData(&TxMessage);
		}
	}
	//CMD_List.Excute，控制程序跳转到指定地址执行
	//该命令在Bootloader和APP程序中都必须实现
	if(can_cmd == CMD_List.Excute)//该命令在DSP中已经实现
	{ 
		exe_type = (pRxMessage->Data[0]<<24)|(pRxMessage->Data[1]<<16)|(pRxMessage->Data[2]<<8)|(pRxMessage->Data[3]<<0);
		if(exe_type == CAN_BL_APP)
		{
			if((*((uint32_t *)APP_START_ADDR)!=0xFFFFFFFF))
			{
				CAN_BOOT_JumpToApplication(APP_START_ADDR);
			}	
		}
		return;
	}
	return;
}
unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len)
{ 
	int i;
	unsigned short int crc_res =  0xFFFF;
	while(len--)
	{
		crc_res^=*data++;
		for(i = 0;i < 8;i++)
		{
			if(crc_res&0x01)
			{
				crc_res = (crc_res>>1)^0xa001;
			}
			else
			{
				crc_res = (crc_res>>1);
			}
		}
	}
	return crc_res;
}
