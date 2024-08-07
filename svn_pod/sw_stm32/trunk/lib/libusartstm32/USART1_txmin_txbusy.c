/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txmin_txbusy.c 
* Hackor             : deh
* Date First Issued  : 10/03/2010 deh
* Description        : USART1 tx minimal, check transmit register emtpy
*******************************************************************************/ 

#include "../libopenstm32/usart.h"
#include "../libusartstm32/usartallproto.h"
#include "../libusartstm32/usartall.h" 

/******************************************************************************
 * u16 USART1_txmin_txbusy(void);
 * @brief	: Check for USART transmit register empty
 * @return	: 0 = busy, transmit register NOT emtpy; non-zero = emtpy, ready for char
******************************************************************************/
u16 USART1_txmin_busy(void)
{
	return (USART_SR(USART1) & USART_FLAG_TXE);
}
