#include "can_bootloader.h"
//��������������λ��������������һ��
CBL_CMD_LIST CMD_List = 
{
  .Erase         = 0x00,      //����APP��������
  .WriteInfo     = 0x01,      //���ö��ֽ�д������ز�����д��ʼ��ַ����������
  .Write         = 0x02,      //�Զ��ֽ���ʽд����
  .Check         = 0x03,      //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
  .SetBaudRate   = 0x04,      //���ýڵ㲨����
  .Excute        = 0x05,      //ִ�й̼�
  .CmdSuccess    = 0x08,      //����ִ�гɹ�
  .CmdFaild      = 0x09,      //����ִ��ʧ��
};
typedef  void (*pFunction)(void);
#define Buffer_size 80
uint8_t	data_temp[128];
uint8_t can_cmd    = 0x00;//ID��bit0~bit3λΪ������
uint16_t can_addr  = 0x00;//ID��bit4~bit15λΪ�ڵ��ַ
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
  * @brief  ��������д��ָ����ַ��Flash�� ��
  * @param  Address Flash��ʼ��ַ��
  * @param  Data ���ݴ洢����ʼ��ַ��
  * @param  DataNum �����ֽ�����
  * @retval ������д״̬��
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
  * @brief  ����ָ�����������Flash���� ��
  * @param  StartPage ��ʼ������ַ
  * @param  EndPage ����������ַ
  * @retval ��������״̬  
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
  * @brief  ��ȡ�ڵ��ַ��Ϣ
  * @param  None
  * @retval �ڵ��ַ��
  */
uint16_t CAN_BOOT_GetAddrData(void)
{
  return Read_CAN_Address();
}
/**
  * @brief  ���Ƴ�����ת��ָ��λ�ÿ�ʼִ�� ��
  * @param  Addr ����ִ�е�ַ��
  * @retval ������ת״̬��
  */
void CAN_BOOT_JumpToApplication(uint32_t Addr)
{
  static pFunction Jump_To_Application;
  __IO uint32_t JumpAddress; 
  __set_PRIMASK(1);//�ر������ж�
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
  * @brief  ִ�������·�������
  * @param  pRxMessage CAN������Ϣ
  * @retval ��
  */
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
	u8 i;
	CanTxMsg TxMessage;
	FLASH_Status ret;
	bootloader_data Boot_ID_info;
	//�жϽ��յ����ݵ�ַ�Ƿ�ͱ��ڵ��ַƥ�䣬����ƥ����ֱ�ӷ��أ������κ�����
	if((can_addr!=CAN_BOOT_GetAddrData())&&(can_addr!=0))
	{
		return;
	}
	TxMessage.DLC = 1;
	TxMessage.ExtId = 0;
	TxMessage.IDE = CAN_Id_Extended;
	TxMessage.RTR = CAN_RTR_Data;
	//��ȡ��ַ��Ϣ
	Boot_ID_info.ExtId.all = pRxMessage->ExtId;
	can_cmd = Boot_ID_info.ExtId.bit.cmd;
	can_addr = Boot_ID_info.ExtId.bit.addr;
	//CMD_List.Erase������Flash�е����ݣ���Ҫ������Flash��С�洢��Data[0]��Data[3]��
	//�����������Bootloader������ʵ�֣���APP�����п��Բ���ʵ��
	//��DSP������Ҫ���������
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
	//CMD_List.SetBaudRate�����ýڵ㲨���ʣ����岨������Ϣ�洢��Data[0]��Data[3]��
	//���Ĳ����ʺ�������Ҳ��Ҫ����Ϊ��ͬ�Ĳ����ʣ�����������ͨ��
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
	//CMD_List.WriteInfo������дFlash���ݵ������Ϣ������������ʼ��ַ�����ݴ�С
	//����ƫ�Ƶ�ַ�洢��Data[0]��Data[3]��,��ƫ������ʾ��ǰ���ݰ�������ļ���ʼ��ƫ����,ͬʱ����ʾд��FLASH��ƫ��
	//  ���ݴ�С�洢��Data[4]��Data[7]�У��ú���������Bootloader������ʵ�֣�APP������Բ���ʵ��
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
	//CMD_List.Write���Ƚ����ݴ洢�ڱ��ػ������У�Ȼ��������ݵ�CRC����У����ȷ��д���ݵ�Flash��
	//ÿ��ִ�и����ݣ����ݻ������������ֽ���������pRxMessage->DLC�ֽڣ�
	//���������ﵽdata_size������2�ֽ�CRCУ���룩�ֽں�
	//�����ݽ���CRCУ�飬������У������������д��Flash��
	//�ú�����Bootloader�����б���ʵ�֣�APP������Բ���ʵ��
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
			crc_data = CRCcalc16(data_temp,data_size-2);//�Խ��յ���������CRCУ�飬��֤����������
			if(crc_data==((data_temp[data_size-2])|(data_temp[data_size-1]<<8)))
			{
				__set_PRIMASK(1);
				FLASH_Unlock();
				ret =  CAN_BOOT_ProgramDatatoFlash(start_addr,data_temp,data_size-2);
				FLASH_Lock();	
				__set_PRIMASK(0);
				if(can_addr != 0x00)
				{
					if(ret==FLASH_COMPLETE)//FLASHд��ɹ�,�ٴν���CRCУ��
					{
							crc_data = CRCcalc16((unsigned char*)(start_addr),data_size-2);//�ٴζ�д��Flash�е����ݽ���CRCУ�飬ȷ��д��Flash����������
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
	//CMD_List.Check���ڵ����߼��
	//�ڵ��յ�������󷵻ع̼��汾��Ϣ�͹̼����ͣ�
	//��������Bootloader�����APP���򶼱���ʵ�� 
	
	if(can_cmd == CMD_List.Check)//DSP����δʵ��,��ԱȽ�����ʵ��,��Ҫ��Ϊʵ��APP�ٴθ���Ӧ�ó���
	{
		if(can_addr != 0x00)
		{
			TxMessage.ExtId = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.Data[0] = 0;//���汾�ţ����ֽ�
			TxMessage.Data[1] = 1;
			TxMessage.Data[2] = 0;//�ΰ汾�ţ����ֽ�
			TxMessage.Data[3] = 0;
			TxMessage.Data[4] = (uint8_t)(FW_TYPE>>24);
			TxMessage.Data[5] = (uint8_t)(FW_TYPE>>16);
			TxMessage.Data[6] = (uint8_t)(FW_TYPE>>8);
			TxMessage.Data[7] = (uint8_t)(FW_TYPE>>0);
			TxMessage.DLC = 8;
			CAN_WriteData(&TxMessage);
		}
	}
	//CMD_List.Excute�����Ƴ�����ת��ָ����ִַ��
	//��������Bootloader��APP�����ж�����ʵ��
	if(can_cmd == CMD_List.Excute)//��������DSP���Ѿ�ʵ��
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
