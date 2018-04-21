/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_l.c
* Author	     : deh
* Date First Issued  : 09/20/2013
* Board              : PC
* Description        : List time from time sync msgs on CAN BUS
*******************************************************************************/
/*
04/20/2018 - Saved in git the old version for the POD format (64 ticks extension)
cmd_l.[ch].save1
Modified 'l' command for straight 4 byte unix time, with CAN address keyboard entry
*/

#include "cmd_l.h"
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits
/******************************************************************************
 * int cmd_l_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
static 	u32 keybrd_id;

int cmd_l_init(char* p)
{
	if (strlen(p) < 10)
	{ // Here too few chars
		printf("%d is too few chars, e.g.\n",(int)strlen(p));
		printf("l 30e00000\n");
		return -1;
	}
	sscanf((p+1), "%x",&keybrd_id);
	printf("Using CAN address: 0x%08X\n",keybrd_id);
   return 0;
}

/******************************************************************************
 * void cmd_l_datetime(struct CANRCVBUF* p);
 * @brief	: Format and print date time from time sync msg in readable form
 ******************************************************************************/
void cmd_l_datetime(struct CANRCVBUF* p)
{
	/* Linux format time.  Tick count shifted right 11 bits (2048) */
	char vv[256];
	char* pc;
	int j;

#define PAYOFFSET 1	// Byte index in payload where unix time starts
	union TPAY
	{
		unsigned char uc[4];
		uint32_t i;
		time_t ut;
	}tpay;

	/* Selct the CAN id for which we have grave interest */
	if (!((p->id & 0xfffffffc) == (keybrd_id & 0xfffffffc))) return;

	/* Extract unix time from CAN payload */
	for (j = 0; j < 4; j++)
		tpay.uc[j] = p->cd.uc[j+PAYOFFSET];
	
	/* Convert to ascii */
		sprintf (&vv[1],"LOCAL %s", ctime(&tpay.ut));
		pc = strchr(&vv[1],'\n'); *pc = ' '; vv[0] = '\n';
		printf("%s",vv);
	return;
}
