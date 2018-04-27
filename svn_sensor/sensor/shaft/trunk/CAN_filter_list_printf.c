/******************************************************************************
* File Name          : CAN_filter_list_printf.c
* Date First Issued  : 04/06/2018
* Board              : f103
* Description        : Print the CAN filter list
*******************************************************************************/
#include "CAN_filter_list_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/* ************************************************************************************** 
 * void CAN_filter_list_printf(void);
 * @brief	: Print the values
 * ************************************************************************************** */
/* NOTE: the first filter bank has a mask and id, so the individual ids install
   starting at filter bank 1.
*/
void CAN_filter_list_printf(void)
{
	uint32_t* pcan;
	int i;

	uint8_t sw = 1;	// Reset-to-beginning for 'fetch'
	printf("\n\rComposite CAN ID list for FIFO 1 hardware filter\n\r");

	for (i = 1; i <= CANFILTERLISTSIZE; i++)
	{
		pcan = CAN_filter_build_fifo1_fetch(sw);
		if (pcan == NULL)	break;

		printf("%02d 0X%08X\n\r",i,(int)(*pcan));		
		sw = 0;
	}	

	sw = 1; 
	printf("\n\rComposite CAN ID list for FOFO 0 hardware filter\n\r");
	for (i = 1; i <= CANFILTERLISTSIZE; i++)
	{
		pcan = CAN_filter_build_fifo0_fetch(sw);
		if (pcan == NULL)
		{
			printf("\n\r");USART1_txint_send();
		   return;
		}
		if (*pcan == 0xffffffff)
			printf("#NOT VALID# ");
		printf("%02d 0X%08X\n\r",i,(int)(*pcan));
		sw = 0;
	}	
	printf("WOW! I think this is not correct\n\r\n\r");USART1_txint_send();
	return;
}
