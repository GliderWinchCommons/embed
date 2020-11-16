/******************************************************************************
* Date First Issued  : 05/05/2018
* Board              : PC
* Description        : histogram readout from shaft sensor
*******************************************************************************/
/*
02-07-2014 rev 193: Poll repsonse for shaft sensor
05/05/2018 Revise to match revised '../sensor/trunk/shaft.c'

*/
#include "cmd_h.h"
#include "cmd_q.h"
#include "USB_PC_gateway.h"

#define DEFAULTRX   0x26000000 //CANID_MC_STATE MC',1,5,'U8_U8','MC: Launch state msg');

static u32 idrx; // CAN id that CAN node sends (to "us)

/******************************************************************************
 * static void send_command(void);
 * @brief 	: Send command
*******************************************************************************/
	union U32B
	{
		u32 ui;
		uint8_t b[4];
	}u32b;

/******************************************************************************
 * int cmd_h_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_h_init(char* p)
{
	idrx = DEFAULTRX;	
	return 0;
}

/******************************************************************************
 * void cmd_h_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'h' command
 * @param	: 
*******************************************************************************/
// Master Controller state machine  definitions 
// Major state names
#define MC_INIT     ( 0 << 3)
#define MC_SAFE     ( 1 << 3)
#define MC_PREP     ( 2 << 3)
#define MC_ARMED    ( 3 << 3)
#define MC_GRNDRTN  ( 4 << 3)
#define MC_RAMP     ( 5 << 3)
#define MC_CLIMB    ( 6 << 3)
#define MC_RECOVERY ( 7 << 3)
#define MC_RETRIEVE ( 8 << 3)
#define MC_ABORT    ( 9 << 3)
#define MC_STOP     (10 << 3)
#define MC_TEST     (11 << 3)

void cmd_h_do_msg(struct CANRCVBUF* p)
{

	/* Select CAN msg that is the response to our command. */
	if ((p->id & 0xfffffffc) != idrx) return;

// Debugging
printf("%08X %i ",p->id,p->dlc);
int i;
for (i = 0; i < p->dlc; i++) printf(" %02X",p->cd.uc[i]);
printf(":  ");

	switch(p->cd.uc[0])
	{
case MC_INIT:     printf("INIT     "); break;
case MC_SAFE:     printf("SAFE     "); break;
case MC_PREP:     printf("PREP     "); break;
case MC_ARMED:    printf("ARMED    "); break;
case MC_GRNDRTN:  printf("GRNDRTN  "); break;
case MC_RAMP:     printf("RAMP     "); break;
case MC_CLIMB:    printf("CLIMB    "); break;
case MC_RECOVERY: printf("RECOVERY "); break;
case MC_RETRIEVE: printf("RETRIEVE "); break; 
case MC_ABORT:    printf("ABORT    "); break;
case MC_STOP:     printf("STOP     "); break;
case MC_TEST:     printf("TEST     "); break;
default:          printf("NONE: 0x%02X ",p->cd.uc[1]); break;
	}

	printf(": sub%3i\n",p->cd.uc[1]);

  return;
}
