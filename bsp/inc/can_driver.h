#ifndef __CAN_DRIVER_H
#define __CAN_DRIVER_H 
#include "main.h" 
#include "can_bootloader.h"
#define CAN_Tx_Port      GPIOH
#define CAN_Tx_Pin       GPIO_Pin_13
#define CAN_Tx_Port_CLK  RCC_AHB1Periph_GPIOH 
#define CAN_Rx_Port      GPIOI
#define CAN_Rx_Pin       GPIO_Pin_9
#define CAN_Rx_Port_CLK  RCC_AHB1Periph_GPIOI 
typedef  struct _CAN_BaudRate
{
  unsigned char       CAN_SJW;
  unsigned char       CAN_BS1;
  unsigned char       CAN_BS2;
  unsigned short int  CAN_Prescaler;
  unsigned long  int  BaudRate; 
} CAN_BaudRate;
void CAN_Configuration(uint32_t BaudRate);
uint8_t CAN_WriteData(CanTxMsg *TxMessage);
uint16_t Read_CAN_Address(void);
void CAN_Address_GPIO_Config(void);

#endif  
