/******************************************************************************
* File Name          : cmd_b.c
* Date First Issued  : 08/04/2019
* Board              : PC
* Description        : Display contactor polled msgs
*******************************************************************************/
/*
*/

#include "cmd_b.h"

#define CANID_POLL1 0x50400000
#define CANID_POLL2 0x50600000
#define CANID_HB1   0xFF800000
#define CANID_HB2   0xFF000000

static 	u32 canid1;
static 	u32 canid2;

/******************************************************************************
 * static printmenu(char* p);
 * @brief 	: Print boilerplate
 * @param	: p = pointer to line
*******************************************************************************/
static void printmenu(char* p)
{
	printf(" bp<enter>\n  CONTACTOR msg gps msg POLLED\n default CAN IDs\t%08X %08X\n",CANID_POLL1,CANID_POLL2);
	printf(" bp aaaaaaaa bbbbbbbb<enter>\n  a = CAN ID msg1, b = CAN ID msg2\n");
   printf(" bh<enter>\n  CONTACTOR msg gps msg HEARTBEAT\n default CAN IDs\t%08X %08X\n",CANID_HB1,CANID_HB2);
	printf(" bh aaaaaaaa bbbbbbbb<enter>\n  a = CAN ID msg1, b = CAN ID msg2\n");
	return;
}
/******************************************************************************
 * int cmd_b_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_b_init(char* p)
{
	uint32_t len = strlen(p);
	printf("%c %i\n",*p,len);
	if ((len < 2) || ((len > 3) && (len < 20)))
	{
		printmenu("Too few chars\n");
		return 0;
	}	
	switch (*(p+1))
	{
	case 'p':
		canid1 = CANID_POLL1;
		canid2 = CANID_POLL2;
		if (len > 20)
		{
			sscanf((p+2),"%x %x",&canid1,&canid2);
		}
		break;

	case 'h':
		canid1 = CANID_HB1;
		canid2 = CANID_HB2;
		if (len > 20)
		{
			sscanf((p+2),"%x %x",&canid1,&canid2);
		}
		break;

	default:
		printmenu("2nd char not recognized\n");
		return 0;
	}

	printf("\nDisplay two payload float pairs using CAN IDs: %08X %08X\n", canid1, canid2);
	return 0;
}

/******************************************************************************
 * void cmd_b_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_b_do_msg(struct CANRCVBUF* p)
{
	union UIF
	{
		uint32_t ui;
		float f;
	}tmp1,tmp2;

	if ((p->id & 0xfffffffc) == canid1)
	{
		tmp1.ui = p->cd.ui[0];
		tmp2.ui = p->cd.ui[1];
		printf(" %10.3f %10.3f",tmp1.f,tmp2.f);
		return;
	}
	if ((p->id & 0xfffffffc) == canid2)
	{
		tmp1.ui = p->cd.ui[0];
		tmp2.ui = p->cd.ui[1];
		printf(" %10.3f %10.3f\n",tmp1.f,tmp2.f);
	}
	return;
}

