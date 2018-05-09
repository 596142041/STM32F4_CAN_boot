#include "flash.h"
static u16 STM32F4_read_flash(u32 address)
{
	return *(vu16*)address;
	
}
void STM32F4_Write_Flash(u32 adress,u16 *pdata,u32 len)
{
	FLASH_Status status = FLASH_COMPLETE;//
	u32 add_start = 0x00;//
	u32 add_end   = 0x00;//
	if(adress <STM32F4_FLASH_BASE||adress%2)//判断你地址是否正确
	{
		return;
	}
	//计算起始地址和结束地址
	add_start = adress;
	add_end = adress+len*2;//半字
	FLASH_Unlock();//对Flash解锁;
	FLASH_DataCacheCmd(DISABLE);//不使能flash缓存
	if(adress < (STM32F4_FLASH_BASE+STM32F4_FLASH_Size))
	{
		while(add_start < add_end)
		{
			if(STM32F4_read_flash(add_start) != 0xffff)//擦除数据
				{
					status = FLASH_EraseSector(GetSector(add_start),VoltageRange_3);
					if(status != FLASH_COMPLETE)
					{
						break;//出现错误;
					}
				}
			else
				{
					add_start=add_start+2;
				}
		}	
	}
	if(status == FLASH_COMPLETE)
	{
			while(add_start < add_end)
			{
				if(FLASH_ProgramHalfWord(add_start,*pdata) != FLASH_COMPLETE)
				{
					break;
				}
				else
				{
					add_start+=2;
					pdata++;
				}
			}
	}
	FLASH_DataCacheCmd(ENABLE);//不使能flash缓存
	FLASH_Lock();
}
void STM32F4_Read_Flash(u32 adress,u16 *pdata,u32 len)
{
	int cnt = 0;
	for(cnt = 0;cnt < len;cnt++)
	{
		pdata[cnt] = STM32F4_read_flash(adress);
		adress=adress+2;//因为是一个全字所以加4，半字加2
	}
}

static uint32_t GetSector(uint32_t Address)
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
