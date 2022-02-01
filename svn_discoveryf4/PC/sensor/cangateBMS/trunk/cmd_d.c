/******************************************************************************
* File Name          : cmd_d.c
* Date First Issued  : 01/31/2022
* Board              : PC
* Description        : Display BMS cell readings
*******************************************************************************/
/*
*/

#include "cmd_d.h"

#define CANID_HB1   0xB0201114
#define NCELL 16 // Max number of cells 

struct CELLMSG
{
	float f;
	uint16_t u16;
	uint8_t flag;
};

static 	u32 canid1;
static struct CELLMSG cellmsg[NCELL];
static uint8_t hbseq;

/******************************************************************************
 * static printmenu(char* p);
 * @brief 	: Print boilerplate
 * @param	: p = pointer to line
*******************************************************************************/
static void printmenu(char* p)
{

//	printf(" d aaaaaaaa [CANid]\n");
	return;
}
/******************************************************************************
 * int cmd_d_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_d_init(char* p)
{
	uint32_t len = strlen(p);
	uint32_t i;

	printf("%c %i\n",*p,len);
	canid1 = CANID_HB1;
	if (len < 2)
	{
		printmenu("Too few chars\n");
		return -1;
	}
	else if (len > 3)
	{	
		sscanf( (p+1), "%x",&canid1);
	}
	printf ("  ID: %08X\n",canid1);

	/* Show no readings received. print header */
	printf("\n  ");
	for (i = 0; i < NCELL; i++)
	{
		cellmsg[i].flag = 0;
		printf("%8d",i+1);
	}

	return 0;
}

/******************************************************************************
 * void cmd_d_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_d_do_msg(struct CANRCVBUF* p)
{
	uint8_t i;
	uint8_t n;
	uint8_t dlc = p->dlc;
	uint8_t celln;
	double dtmp;

	if ((p->id & 0xfffffffc) != canid1)
	{
		return; // Msg is not for us.
	}
//printf("\n%08X %X", p->cd.uc[1]);

	/* Here, CAN msg is for us. */
	// Check msg is for the same reading group
	if ((p->cd.uc[1] & 0xf) != (hbseq & 0xf))
	{ // Here sequence number changed
		// Print readings accumulated
		printf("\n%2d: ",hbseq);
		for (i = 0; i < NCELL; i++)
		{
			if (cellmsg[i].flag != 0)
			{
				dtmp = cellmsg[i].u16; // Convert to float
				printf("%8.1f",dtmp*0.1); 
				cellmsg[i].flag = 0; // Clear was-read flag
			}
			else
			{ // No readings for this cell
				printf("  ......");
			}
		}
		/* Update to new sequence number. */
		hbseq = p->cd.uc[1] & 0xf;
	}

	/* Add readings in payload into array cell positions. */
	// Check payload size 
	switch(dlc)
	{
		case 4: n = 1; break; 
		case 6: n = 2; break; 
		case 8: n = 3; break; 

		default:
			{
				printf("\nDLC not 4,6,or8 %d",dlc);
				return;
			}
	}
	// Cell number of 1st payload U16
	celln = (p->cd.uc[1] >> 4) & 0xf;
	for (i =0 ; i < n; i++)
		{
			cellmsg[celln+i].u16 = p->cd.us[i+1];
			cellmsg[celln+i].flag = 1;
		}
	return;
}

