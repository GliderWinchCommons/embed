/******************************************************************************
* File Name          : itimerCAN.c
* Date First Issued  : 11/20/2019
* Board              : PC
* Description        : Timer for sending keep-alive CAN msgs
*******************************************************************************/
/*
This takes care of dispatching keyboard commands.
02-06-2014 rev 191 Add 'h' cmd
*/

#include "commands.h"
#include "timer_thread.h"
#include "cmd_a.h"
#include "cmd_b.h"
#include "cmd_f.h"
#include "cmd_h.h"
#include "cmd_l.h"
#include "cmd_m.h"
#include "cmd_n.h"
#include "cmd_p.h"
#include "cmd_q.h"
#include "cmd_r.h"
#include "cmd_c.h"
#include "cmd_w.h"
#include "cmd_s.h"
#include "cmd_k.h"

#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

void itimerCAN_sig(void);

static u8 canseqnumber = 0;
struct CANRCVBUF cantimer;

/******************************************************************************
 * int itimerCAN_init(char* p);
 * @brief 	: Setup timer for keep-alive type of repetitive CAN msg sending
 * @param	: p = pointer to keyboard input
*******************************************************************************/
int itimerCAN_init(char* p)
{
	


}
/******************************************************************************
 * void itimerCAN_sig(void);
 * @brief 	: Send keep-alive msg
*******************************************************************************/
void itimerCAN_sig(void)
{
	if (msg_sw != 'k') return;	// Command in effect

   struct timeval te; 
	gettimeofday(&te, NULL); // get current time
	long long millisec = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
	if (millisec < millisec_prev) return;
	{ // Here, time has expired, so send another keep-alive msg.
		millisec_prev = millisec + KEEPALIVEDURATION; // Next time for a keep-alive msg

		if (kaON != 0)
		{
			/* Send keep-alive msg with the current command code in payload [0] */
			sendcanmsg(&cantimer);		
		}
	}
	return;
}


