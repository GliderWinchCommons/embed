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
static uint32_t mctr;
static uint32_t mctr_prev;

#define KEYBRD_ID_MAX 16 // Number of CAN ids to select
static 	u32 keybrd_id[KEYBRD_ID_MAX]; // CAN id selection list
static  u8 keybrd_ct; // Number of CAN ids in selection list
static  char type;

/******************************************************************************
 * static int keybrd_scan(char * p);
 * @brief 	: After 'm ' scan keyboard entry for CAN ids
 * @param	: p = pointer to line entered on keyboard +1
 * @return	: 0 = none found; KEYBRD_ID_MAX = bogus; Number found
*******************************************************************************/
static int keybrd_scan(char * p)
{
	int ret; // Number of items converted in sscanf
	int ct;  // Number of chars processed in sscanf
	int shift = 0;
	uint32_t* pw = &keybrd_id[0]; // CAN id selection list 

	for (keybrd_ct = 0; keybrd_ct < KEYBRD_ID_MAX; keybrd_ct++)
	{
		ret = sscanf(p,"%x%n",pw,&ct);
printf("%2d %2d %08X %d\n", keybrd_ct, ret, keybrd_id[keybrd_ct], ct);
		if (ret <= 0)
		{
			return keybrd_ct;
		}

		// Fix up scaling/shift for different ID entry forms--
		switch (ct)
		{
		case 4: // e.g. " 1E0" 11b ID shortest form
			shift = 20;
			break;
		case 5: // e.g. " 1E00" 11b ID form but still 11b
			shift = 16;
			break;
		case 9: // e.g. " 45600000" 11b or 29b in long form
			shift = 0;
			break;
		default: // Likely an extra space  
			shift = 0;
			printf("ID input not quite right. It should be (where . stands for a space\n"
				"\t.234 or\n"
				"\t.2340 or\n"
				"\t.23400000\n");
			printf("Example: 234 4660 4660000C would all be OK (no extra spaces or other inbetween IDs\n");
		}
		keybrd_id[keybrd_ct] = keybrd_id[keybrd_ct] << shift;

		p  += ct; // Advance input line pointer
		pw += 1;  // Next ID position
	}
	printf("keybrd_scan: generating list of IDs ran off the end. %d\n",keybrd_ct);
	return keybrd_ct;
}
/******************************************************************************
 * static void printf_keybrd_id_list(void);
 * @brief 	: Print the CAN id list for selection 
 ******************************************************************************/
static void printf_keybrd_id_list(void)
{
	int i;
	for (i = 0; i < keybrd_ct; i++)
	{
		printf("%2d 0x%08X\n",i, keybrd_id[i]);
	}
	return;
}
/******************************************************************************
 * int cmd_m_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/

int cmd_m_init(char* p)
{
	int ret;
	mctr = 0;
	mctr_prev = 0;

	type = *(p+1); 
    switch (type)
	{
	case ' ':
		if (strlen(p) < 6)
		{ // Here too few chars
			printf("Too few chars for the 'm' command (needs at least 6), example\nm 30e0 [for m command and unit id]\n");
			return -1;
		}
		ret = keybrd_scan(p+1);
		printf ("Number of IDs to selection: %d\n",ret);
		printf_keybrd_id_list();

    	return 0;
	
	case 'f': // Payload as float|float
		if (strlen(p) < 10)
		{ // Here too few chars
			printf("Too few chars for the 'mf' command (needs 10), example\nmf 30e00000\n");
			return -1;
		}
		sscanf((p+2), "%x",&keybrd_id[0]);
	    printf ("ID: payload: float|float %X\n",keybrd_id[0]);
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
	int diff;
	double dtmp1, dtmp2;

	mctr += 1;

	for (i = 0; i < keybrd_ct; i++)
	{
		if (((p->id & 0xfffffffc) == (keybrd_id[i] & 0xfffffffc)))
		{
			/* Show number of CAN msgs between this CAN ID. */
			diff = (int)(mctr - mctr_prev);
			mctr_prev = mctr;

			switch (type)
			{
			case ' ':
				printf("%6d %4d %08x %d ",mctr, diff, p->id, p->dlc);
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
	}
	return;	
}

