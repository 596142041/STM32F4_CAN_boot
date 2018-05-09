#include "can_bootloader.h"
//命令参数必须跟上位机软件的命令参数一致
CBL_CMD_LIST CMD_List = 
{
	.Read        = 0x0A, //读取flash数据
	.Erase       = 0x00, //擦除APP区域数据
	.Write       = 0x02,//以多字节形式写数据
	.Check       = 0x03,//检测节点是否在线，同时返回固件信息
	.Excute      = 0x05,//执行固件
	.CmdFaild    = 0x09,//命令执行失败
	.WriteInfo   = 0x01,//设置多字节写数据相关参数(写起始地址,数据量)
	.CmdSuccess  = 0x08,//命令执行成功
	.SetBaudRate = 0x04,//设置节点波特率
};
//相关设备信息
/*
包括当前芯片型号,当前设备地址,
当前设备固件类型,当前设备固件更改日期,当前设备版本号

*/
Device_INFO DEVICE_INFO =
{ 
 .Device_addr.bits.reserve      = 0x00,
 .FW_Version.bits.Version       = 02,
 .FW_Version.bits.date          = 28,
 .FW_Version.bits.month         = 04,
 .FW_Version.bits.year          = 2018,
 .FW_TYPE.bits.FW_TYPE          = CAN_BL_BOOT,
 .FW_TYPE.bits.Chip_Value       = STM32F407IGT6,
 .Device_addr.bits.Device_addr  = DEVICE_ADDR,
};
#define Buffer_size 1030
#define READ_MAX  256   //每次读取数据的最大长度,16位数据
uint8_t	 data_temp[Buffer_size];
uint16_t read_temp[READ_MAX];
uint8_t  can_cmd     = (uint8_t )0x00;//ID的bit0~bit3位为命令码
uint8_t  file_type   = (uint8_t )File_None;
u32 read_addr       = (u32)0x00;//读取数据起始地址
u32 read_len        = (u32)0x00;//读取数据长度
uint32_t exe_type    = (uint32_t)0x00;
uint16_t can_addr    = (uint16_t)0x00;//ID的bit4~bit15位为节点地址
uint32_t BaudRate    = (uint32_t)0x00;
uint16_t crc_data    = (uint16_t)0x00;
uint32_t data_size   = (uint32_t)0x00; 
uint32_t FlashSize   = (uint32_t)0x00;
uint32_t data_index  = (uint32_t)0x00;
uint32_t addr_offset = (uint32_t)0x00;
uint32_t start_addr  = (uint32_t)APP_START_ADDR; 
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
	uint16_t *pDataTemp=(uint16_t *)pData;
	uint32_t i;
	if(StartAddr<APP_EXE_FLAG_START_ADDR)
	{
		return FLASH_ERROR_PGS;
	} 
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	for(i=0;i<(DataNum>>1);i++)
	{
		FLASHStatus = FLASH_ProgramHalfWord(StartAddr, *pDataTemp);
		if (FLASHStatus == FLASH_COMPLETE)
		{
			StartAddr += 2;
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
		FLASHStatus = FLASH_EraseSector(SectorCounter, VoltageRange_3);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			return FLASHStatus;
		}
	}
	return FLASHStatus;
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
  if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)
  {  
    JumpAddress = *(__IO uint32_t*) (Addr + 4);
    Jump_To_Application = (pFunction) JumpAddress; 
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
	TxMessage.DLC   = 1;
	TxMessage.ExtId = 0;
	TxMessage.IDE   = CAN_Id_Extended;
	TxMessage.RTR   = CAN_RTR_Data;
	//获取地址信息
	Boot_ID_info.ExtId.all = pRxMessage->ExtId;
	can_cmd  = Boot_ID_info.ExtId.bit.cmd;
	can_addr = Boot_ID_info.ExtId.bit.addr;
	//判断接收的数据地址是否和本节点地址匹配，若不匹配则直接返回，不做任何事情
	if((can_addr!=DEVICE_ADDR)&&(can_addr!=0))
	{
		return;
	}
	//CMD_List.Erase，擦除Flash中的数据，需要擦除的Flash大小存储在Data[0]到Data[3]中
	//该命令必须在Bootloader程序中实现，在APP程序中可以不用实现
	//在DSP部分需要处理该命令
	if(can_cmd == CMD_List.Erase)
	{
		__set_PRIMASK(1);
		FLASH_Unlock();
		FlashSize = (pRxMessage->Data[0]<<0x18)|\
					(pRxMessage->Data[1]<<0x10)|\
					(pRxMessage->Data[2]<<0x08)|\
					(pRxMessage->Data[3]<<0x00);	
					
		file_type = pRxMessage->Data[4];
		ret =  CAN_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_START_ADDR+FlashSize);
		FLASH_Lock();	
		__set_PRIMASK(0);
		if(can_addr != 0x00)
		{
			if(ret==FLASH_COMPLETE)
			{
				TxMessage.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
				TxMessage.DLC = 0;
				TxMessage.Data[0] = 1;	
				start_addr = APP_START_ADDR;
			}
			else
			{
				TxMessage.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
				TxMessage.DLC = 1;
				TxMessage.Data[0] = 1;
			} 
			CAN_WriteData(&TxMessage);
		} 
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
			TxMessage.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.DLC = 0;
			delay_ms(20); 
			TxMessage.Data[0] = 2;
			CAN_WriteData(&TxMessage);
		}
		return;
	}
	//cmd_list.read,读取flash数据,
	//该命令是用于读取内部存储器数据
	//该命令在Bootloader和APP程序中国必须实现
		if(can_cmd == CMD_List.Read)
		{
			if(pRxMessage->DLC != 8)
			{
				TxMessage.DLC     = 0x02;
				TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
				TxMessage.Data[0] = (u8)CMD_List.Read;//主版本号，两字节
				TxMessage.Data[1] = (u8)MSG_DATA_LEN_ERROR;
				CAN_WriteData(&TxMessage);
				return;
			}
			read_addr =  (((u32)(pRxMessage->Data[0])&0xFFFFFFFF)<<0x18)|\
									 (((u32)(pRxMessage->Data[1])&0x00FFFFFF)<<0x10)|\
									 (((u32)(pRxMessage->Data[2])&0x0000FFFF)<<0x08)|\
									 (((u32)(pRxMessage->Data[3])&0x000000FF)<<0x00);
			read_len  =  (((u32)(pRxMessage->Data[4])&0xFFFFFFFF)<<0x18)|\
									 (((u32)(pRxMessage->Data[5])&0x00FFFFFF)<<0x10)|\
									 (((u32)(pRxMessage->Data[6])&0x0000FFFF)<<0x08)|\
									 (((u32)(pRxMessage->Data[7])&0x000000FF)<<0x00);
			if(read_len > READ_MAX)
			{
				TxMessage.DLC                              = 0x02;
				TxMessage.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
				TxMessage.Data[0] = (u8)CMD_List.Read;//主版本号，两字节
				TxMessage.Data[1] = (u8)READ_LEN_ERROR;
				CAN_WriteData(&TxMessage);
				return;
			}
			u16 read_len_temp  = 0;
			if(read_len%2 == 0)//因为每次只能读取N个字
			{
				read_len_temp = read_len;
			}
			else
			{
				read_len_temp = read_len+1;
			}
			read_len_temp = read_len_temp>>1;
			__set_PRIMASK(1);
			STM32F4_Read_Flash(read_addr,read_temp,read_len_temp);
			__set_PRIMASK(0);
			for(i = 0;i <read_len_temp;i++)
			{
				data_temp[i*2+1] = (read_temp[i]>>8)&0xFF;
				data_temp[i*2+0] = read_temp[i]&0xFF;
			}
			data_index = 0;
			i = 0;
			while(data_index < read_len)
			{
				int temp;
				temp = read_len - data_index;
				if (temp >= 8)
				{
					TxMessage.DLC             = 0x08;
					TxMessage.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
					for (i = 0; i < TxMessage.DLC; i++)
					{
						TxMessage.Data[i] = data_temp[data_index];
						data_index++;
					}

				}
				else
				{
					TxMessage.DLC             = temp;
					TxMessage.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
					for (i = 0; i < TxMessage.DLC; i++)
					{
						TxMessage.Data[i] = data_temp[data_index];
						data_index++;
					}
				}
				CAN_WriteData(&TxMessage);
			}
			data_index = 0;
			return;
		}
		
	//CMD_List.WriteInfo，设置写Flash数据的相关信息，比如数据起始地址，数据大小
	//数据偏移地址存储在Data[0]到Data[3]中,该偏移量表示当前数据包针针对文件起始的偏移量,同时还表示写入FLASH的偏移
	//  数据大小存储在Data[4]到Data[7]中，该函数必须在Bootloader程序中实现，APP程序可以不用实现
	if(can_cmd == CMD_List.WriteInfo)
	{
		__set_PRIMASK(1);
		
		addr_offset  = (pRxMessage->Data[0]<<0x18)|\
					   (pRxMessage->Data[1]<<0x10)|\
					   (pRxMessage->Data[2]<<0x08)|\
					   (pRxMessage->Data[3]<<0x00);
		if(file_type == File_bin)
		{
			start_addr   = APP_START_ADDR+addr_offset;
		}
		else if(file_type == File_hex)
		{
			start_addr   =  addr_offset;
		}
		else
		{
			start_addr   =  APP_START_ADDR;
		}
		data_size    = (pRxMessage->Data[4]<<0x18)|(pRxMessage->Data[5]<<0x10)|(pRxMessage->Data[6]<<0x08)|(pRxMessage->Data[7]<<0);
		data_index   = 0;
		__set_PRIMASK(0); 
		if(can_addr != 0x00)
		{
			if(start_addr<APP_Write_START_ADDR||start_addr>APP_Write_END_ADDR)
			{
				TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
				TxMessage.DLC     = 2;
				TxMessage.Data[0] = CMD_List.WriteInfo;
				TxMessage.Data[1] = FLASH_ADDR_ERROR;
				CAN_WriteData(&TxMessage);
				return;
			}
			else if(data_size > Buffer_size)
			{
				TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
				TxMessage.DLC     = 2;
				TxMessage.Data[0] = CMD_List.WriteInfo;
				TxMessage.Data[1] = WRITE_LEN_ERROR;
				CAN_WriteData(&TxMessage);
				return;
			}
			else
			{
				TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
				TxMessage.DLC     = 1;
				TxMessage.Data[0] = CMD_List.WriteInfo;
				CAN_WriteData(&TxMessage);
				return;
			}
		}
		else
		{
			TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.DLC     = 2;
			TxMessage.Data[0] = CMD_List.WriteInfo;
			TxMessage.Data[1] = DEVICE_ADDR_ERROR;
			CAN_WriteData(&TxMessage);
			return;
		}
		 
	}
	//CMD_List.Write，先将数据存储在本地缓冲区中，然后计算数据的CRC，若校验正确则写数据到Flash中
	//每次执行该数据，数据缓冲区的数据字节数会增加pRxMessage->DLC字节，
	//当数据量达到data_size（包含2字节CRC校验码）字节后
	//对数据进行CRC校验，若数据校验无误，则将数据写入Flash中
	//该函数在Bootloader程序中必须实现，APP程序可以不用实现
	if(can_cmd == CMD_List.Write)
	{
		if(file_type == File_bin)
		{
			if((data_index<data_size)&&(data_index<Buffer_size-2))
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
									TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
									TxMessage.DLC     = 1;
									TxMessage.Data[0] = 4;
								}
								else
								{
									TxMessage.ExtId    = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
									TxMessage.DLC      = 1;
									TxMessage.Data[0]  = CMD_List.Write; 
								} 
								CAN_WriteData(&TxMessage);
								return;
						}
						else
						{
							TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
							TxMessage.DLC     = 2;
							TxMessage.Data[0] = CMD_List.Write;  
							TxMessage.Data[1] = WRITE_ERROR;
						    CAN_WriteData(&TxMessage);
							return;
						}
					}
					else
					{
						TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
						TxMessage.DLC     = 2;
						TxMessage.Data[0] = CMD_List.Write;
						TxMessage.Data[1] = DEVICE_ADDR_ERROR; 
						CAN_WriteData(&TxMessage);
						return;
					}
				} 
			} 
		}
		else if(file_type == File_hex)
		{
			if((data_index<data_size)&&(data_index<Buffer_size-2))
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
									TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
									TxMessage.DLC     = 1;
									TxMessage.Data[0] = 4;
								}
								else
								{
									TxMessage.ExtId    = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
									TxMessage.DLC      = 1;
									TxMessage.Data[0]  = CMD_List.Write; 
								} 
								CAN_WriteData(&TxMessage);
								return;
						}
						else
						{
							TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
							TxMessage.DLC     = 2;
							TxMessage.Data[0] = CMD_List.Write;  
							TxMessage.Data[1] = WRITE_ERROR;
						    CAN_WriteData(&TxMessage);
							return;
						}
					}
					else
					{
						TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
						TxMessage.DLC     = 2;
						TxMessage.Data[0] = CMD_List.Write;
						TxMessage.Data[1] = DEVICE_ADDR_ERROR; 
						CAN_WriteData(&TxMessage);
						return;
					}
				} 
			}  
		}
		else
		{
			TxMessage.ExtId   = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdFaild;
			TxMessage.DLC     = 2;
			TxMessage.Data[0] = CMD_List.Write;
			TxMessage.Data[1] = FILE_TYPE_ERROR;
			CAN_WriteData(&TxMessage);
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
			TxMessage.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.DLC = 8; 
			TxMessage.Data[0] = (u8)(DEVICE_INFO.FW_Version.all>>24);;//主版本号，两字节
			TxMessage.Data[1] = (u8)(DEVICE_INFO.FW_Version.all>>16);
			TxMessage.Data[2] = (u8)(DEVICE_INFO.FW_Version.all>>8);//次版本号，两字节
			TxMessage.Data[3] = (u8)(DEVICE_INFO.FW_Version.all>>0);
			TxMessage.Data[4] = (u8)(DEVICE_INFO.FW_TYPE.bits.FW_TYPE>>16);
			TxMessage.Data[5] = (u8)(DEVICE_INFO.FW_TYPE.bits.FW_TYPE>>8);
			TxMessage.Data[6] = (u8)(DEVICE_INFO.FW_TYPE.bits.FW_TYPE>>0);
			TxMessage.Data[7] = (u8)(DEVICE_INFO.FW_TYPE.bits.Chip_Value>>0); 
			CAN_WriteData(&TxMessage);
			return;
		}
		
	}
	//CMD_List.Excute，控制程序跳转到指定地址执行
	//该命令在Bootloader和APP程序中都必须实现
	if(can_cmd == CMD_List.Excute)//该命令在DSP中已经实现
	{  
		exe_type = (pRxMessage->Data[0]<<0x10)|\
			       (pRxMessage->Data[1]<<0x08)|\
				   (pRxMessage->Data[2]<<0x00); 
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
