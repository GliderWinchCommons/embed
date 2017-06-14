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
#include <string.h>

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
//	char vv[96];

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
		USART1_txint_puts(" 2 phase diff: tim2_diff\n\r");
		USART1_txint_puts(" 3 sys ticks per sec\n\r");
		USART1_txint_puts(" 4 ctr: gps_poll_flag_ctr\n\r");
		USART1_txint_send();
		state = 2;
		break;

	case 2:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full
		USART1_txint_puts(" 5 ctr: gps_ready_flag\n\r");
		USART1_txint_puts(" 6 thermistor: deg C\n\r");
		USART1_txint_puts(" 7 Unix 32b time\n\r");
		USART1_txint_puts(" 8 GPS date/time\n\r");
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

		/* 2 Phase er: Number of ticks the OC differed from the 1 PPS */
		printf(" %3d",tim2_diff);

		/* 3 Processor ticks in one sec */
		printf("%10d", tim2_ic);

		/* 4 Jam date/time: gps_packetize found GPS differed from SYS date/time. */
		printf (" %4d",gps_poll_flag_ctr);

		/* 5 GPS ready flag */
		printf(" %2d",cGPS_ready);
	
		/* 6 Display the thermistor temperature in degrees C */
		printf ("%4d.%02u",strAlltime.uiThermtmp/100,(strAlltime.uiThermtmp % 100) );	
				
		/* 7 Make Linux format time */
		uitemp = (  (pkt_gps_mn.alltime.GPS.ull >> 6) );
		printf (" %10u",  uitemp );

		/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
		uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

		/* 8 Reconvert to ascii (should match the above ascii where appropriate) */
		char* pt = ctime((const time_t*)&uitemp);
#define GCMDDEBUG
#ifdef GCMDDEBUG
char* pc = strchr(pt,'\n'); // Eliminate '\n' from ctime to make all on one line
*pc = ' '; printf ("  %s", pt);
#else
		printf ("  %s\r", pt);
#endif

/* DEBUG: Useful for debugging Tim2_pod_se.c */
#ifdef GCMDDEBUG

/* Make Linux format time from SYS to compare to GPS */
uitemp = (  (strAlltime.SYS.ull >> 6) );
printf (" %11u",  uitemp );

/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

/* Reconvert to ascii (should match the above ascii where appropriate) */
pt = ctime((const time_t*)&uitemp);
pc = strchr(pt,'\n'); *pc = ' '; printf ("  %s", pt);

/* Display average processor ticks per sec */
int yy = (ticks_per_oc_fraction * 10000)/(1 << TIM2SCALE);
printf(" %d.%04d ",ticks_per_oc_whole,yy);

/* OC interrupt count */
static unsigned int tim2debug0_prev;
printf(" %d",tim2debug0-tim2debug0_prev);tim2debug0_prev = tim2debug0;

int64_t aa = (ticks_ave_scaled * 1000)/(1 << TIM2SCALE);
int64_t bb = aa/1000;
int64_t cc = aa - (bb * 1000);
printf(" %d.%03d", (int32_t)bb,(int32_t)cc);

#endif

USART1_txint_puts("\n\r");			

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
