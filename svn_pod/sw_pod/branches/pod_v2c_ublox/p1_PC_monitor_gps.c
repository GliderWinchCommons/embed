/******************************************************************************
* File Name          : p1_PC_monitor_gps.h
* Author             : deh
* Date First Issued  : 09/13/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current gps to PC
*******************************************************************************/
/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/lib/libusupportstm32/adc_packetize.c
@2 = svn_pod/sw_pod/trunk/pod_v1/p1_PC_monitor_batt.c
*/

#include "p1_gps_time_convert.h"
#include "p1_common.h"
#include "calendar_arith.h"
#include <time.h>
#include "lltoa.h"
#include "Tim2_pod_se.h"

/* Subroutine prototypes */
void output_gps_cnt (unsigned int uiT);

/******************************************************************************
 * void p1_PC_monitor_gps(void);
 * @brief	: Output current GPS & tick ct to PC
 ******************************************************************************/
static short state;
static short gps_mn_pkt_ctrPrev;	// Previous packet ready count

void p1_PC_monitor_gps(void)
{
	unsigned int uitemp;
	char vv[96];

	switch (state)
	{
	case 0:
		if (gps_mn_pkt_ctr == gps_mn_pkt_ctrPrev) return;
		gps_mn_pkt_ctrPrev = gps_mn_pkt_ctr;	// Update for next round

		if (usMonHeader == 0)	// Check if a header has been placed
			state = 1;	// Here, no.  Place a column header
		else
			state = 3;	// Here, yes. Just put a line of data
		usMonHeader = 1;	// Only one header per menu selection
		break;

	case 1: /* Do a column header */
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full
		USART1_txint_puts("Columns--\n\r");
		USART1_txint_puts(" 1 c = skip time adjust, g = regular adj\n\r");
		USART1_txint_puts(" 2 diff = gps - sys time\n\r");
		USART1_txint_puts(" 3 phasing sum / 4096\n\r");
		USART1_txint_puts(" 4 rate deviation sum / 4096\n\r");
		USART1_txint_puts(" 5 sys ticks per sec\n\r");
		USART1_txint_send();
		state = 2;
		break;

	case 2:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full
		USART1_txint_puts(" 6 ctr: tim2_64th_0_er\n\r");
		USART1_txint_puts(" 7 ctr: tim2_64th_31_er\n\r");
		USART1_txint_puts(" 8 ctr: gps_poll_flag_ctr\n\r");
		USART1_txint_puts(" 9 thermistor: deg C\n\r");
		USART1_txint_puts("10 Unix 32b time\n\r");
		USART1_txint_puts("11 Human time\n\r");
		USART1_txint_send();
		state = 3;
		break;


	case 3:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full
		
		/* 'c' command stops the time adjusting with the gps */
		if (gps_timeadjustflag == 1)
		{ // Here, there is a 'c' command in effect
			USART1_txint_putc('c');
		}
		else
		{ // Here, 'g' command is in effect
			USART1_txint_putc('g');
		}

		/* Difference in ticks between GPS and rtc tick counter (print long long as separate longs) */
		printf("  %s ",lltoa(vv,strAlltime.DIF.ull,10) );

		/* Running sums where the whole is added to an OC interval (and removed from the sum) */
		// s32	phasing_sum;		// Running sum of accumulated phase adjustment
		// s32	deviation_sum;		// Running sum of accumulated error
		printf("%10d  %10d", phasing_sum/4096, deviation_sum/4096);

		/* Processor ticks in one sec */
		printf("%10d", ticksm);

		/* Running error counters */
		//tim2_64th_0_er;	// Count of date/time adjustments at interval 0
		//tim2_64th_31_er;// Count of date/time adjustments at interval 31
		printf (" %6d %6d",tim2_64th_0_er, tim2_64th_31_er);

		printf (" %4d",gps_poll_flag_ctr);

		/* GPS ready flag */
		printf(" %d",cGPS_ready);

		/* Average thermistor ADC reading (xxxxxx.x) */
//		printf(" %6d ",strAlltime.uiAdcTherm );
	
		/* Display the thermistor temperature in degrees C */
		printf ("%4d.%02u",strAlltime.uiThermtmp/100,(strAlltime.uiThermtmp % 100) );	
				
		/* Make Linux format time */
		uitemp = (  (pkt_gps_mn.alltime.GPS.ull >> 6) );
		printf (" %10u",  uitemp );

		/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
		uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

		/* Reconvert to ascii (should match the above ascii where appropriate) */
		printf ("  %s\r", ctime((const time_t*)&uitemp));		

		USART1_txint_send();		// Start line sending.

		state = 0;			// Reset state back to beginning 		
	}

	return;
}
/*****************************************************************************************
Setup an int as a seconds and tick within second
*****************************************************************************************/

void output_gps_cnt (unsigned int uiT)
{
	printf (" %8u %4u ",(uiT >> ALR_INC_ORDER),(uiT & ( (1<<ALR_INC_ORDER)-1) ) ) ;	// Setup the output

	return;
}
