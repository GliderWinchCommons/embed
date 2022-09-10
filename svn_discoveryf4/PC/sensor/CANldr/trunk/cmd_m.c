/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_m.c
* Author	     : deh
* Date First Issued  : 09/20/2013
* Board              : PC
* Description        : Details for handling the 'm' command (list msgs for the id entered)
*******************************************************************************/
/*
*/

#include "cmd_m.h"

/******************************************************************************
 * int cmd_m_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
static 	u32 keybrd_id;
static  char type;

int cmd_m_init(char* p)
{
	type = *(p+1); 
        switch (type)
	{
	case ' ':
		if (strlen(p) < 6)
		{ // Here too few chars
			printf("Too few chars for the 'm' command (needs 8), example\nm 30e00000 [for m command and unit id]\n");
			return -1;
		}
 	    	sscanf( (p+1), "%x",&keybrd_id);
	    	printf ("ID: %X\n",keybrd_id);
//	    	keybrd_id = keybrd_id << 16;
	    	return 0;
	
	case 'f': // Payload as float|float
		if (strlen(p) < 10)
		{ // Here too few chars
			printf("Too few chars for the 'mf' command (needs 10), example\nmf 30e00000\n");
			return -1;
		}
		sscanf((p+2), "%x",&keybrd_id);
	    printf ("ID: payload: float|float %X\n",keybrd_id);
	    return 0;

	default:
	    printf("char following m was not one implemented %c\n",type);
	}
	return -1;
}


/******************************************************************************
 * void cmd_m_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_m_do_msg(struct CANRCVBUF* p)
{
	int i;
	double dtmp1, dtmp2;

	if (!((p->id & 0xfffffffc) == (keybrd_id & 0xfffffffc))) return;
	switch (type)
	{
	case ' ':
		printf("%08x %d ",p->id, p->dlc);
		for (i = 0; i < (p->dlc & 0xf); i++)
			printf("%02X ",p->cd.u8[i]);
		printf("\n");
		break;
	case 'f':
                if (p->dlc == 8)
		{
			dtmp1 = p->cd.f[0]; dtmp2 = p->cd.f[1];
			printf("%08X %12.8e %12.8e\n",p->id, dtmp1,dtmp2);
			break;
		}
		printf("DLC not 8: %d %08x\n",p->dlc,p->id);
		break;

	default:
		printf("something wrong with m command. type = %c\n", type);
	}

	return;
}

