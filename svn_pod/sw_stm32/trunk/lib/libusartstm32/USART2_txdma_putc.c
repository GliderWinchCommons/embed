/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_txdma_putc.c
* Hackor             : deh
* Date First Issued  : 10/06/2010
* Description        : Add a char to a line buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"
/* 
NOTE:
If 'putc' adds a char to the current line buffer being filled.  If the end is reached
before 'send()' is called by the main program, then this routine issues a 'send()'.
This prevents the buffer from being overrun, and chars lost when the line to be sent
is longer than the line buffer.  Note, that there can be problems when 'putc' issues
a 'send()' and the mainline also issues a 'send()' when there are are no chars.  Under
some circumstances this can hang 'send()' waiting for a completion that will never
come.  Best to make sure the line buffer is long enough for the line that will be sent.
*/
/*******************************************************************************
* char USART2_txdma_putc(char c);
* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: 0 = buffer did not overflow; 1 = overflow, chars could be lost
*******************************************************************************/
char USART2_txdma_putc(char c)
{
	/* If we have filled all the line buffers, then we are screwed and must wait */
	while (USART2_txdma_busy() == 1);	// Loop until buffer free

 	*pUSARTcbt2->ptx_now_m++ = c;	// Store a char

	/* Are we at the end of the buffer? */
	if (pUSARTcbt2->ptx_now_m >= (pUSARTcbt2->ptx_begin_m + pUSARTcbt2->tx_ln_sz) ) 
	{ // Here we hit the end of the line buffer, so 
		USART2_txdma_send();	// Step to next line buffer
		return 1;		// Show caller we hit the end
	}
 	return 0;	
}
