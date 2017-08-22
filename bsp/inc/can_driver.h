#ifndef __CAN_DRIVER_H
#define __CAN_DRIVER_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Functions -----------------------------------------------------------------*/
void CAN_Configuration(uint32_t BaudRate);
uint8_t CAN_WriteData(CanTxMsg *TxMessage);
uint16_t Read_CAN_Address(void);
void CAN_Address_GPIO_Config(void);

#endif /*__CAN_H */

/***********************************ÎÄ¼þ½áÊø***********************************/
