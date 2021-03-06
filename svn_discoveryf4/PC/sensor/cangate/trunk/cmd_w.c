/******************************************************************************
* File Name          : cmd_w.c
* Date First Issued  : 04/11/2018
* Board              : PC
* Description        : Details for handling command 'w' list float or integer payload
*******************************************************************************/
/*
*/

#include "cmd_w.h"

/******************************************************************************
 * int cmd_w_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
static 	u32 keybrd_id;
static 	u32 payloadidx;	// Can payload byte index 0 - 5
static 	char payloadtype; // integer or float
static   char type;

void printerror(char *p)
{
			printf("%s",p);
			printf("w 30e00000 [integer payload starting at [0]]\n");
			printf("wi0 30e00000 [integer payload starting at [0]]\n");
			printf("wi1 30e00000 [integer payload starting at [1]]\n");
			printf("wf0 30e00000 [float payload starting at [0]]\n");
			printf("wf1 30e00000 [float payload starting at [1]]\n");
			printf("wfx 30e00000 [float payload starting at [x] x 0 - 5]\n");
	return;
}

int cmd_w_init(char* p)
{
		if (strlen(p) < 12)
		{ // Here too few chars
			printerror("Too few chars\n");
			return -1;
		}
	type = *(p+1); 	// This should be ' ', 'i', 'f'
        switch (type)
	{
	case ' ': // Default wi0 xxxxxxxx
		if (strlen(p) < 12)
		{ // Here too few chars
			printerror("Need 12 or more chars\n");
			return -1;
		}
 	    	sscanf( (p+1), "%x",&keybrd_id);
			payloadidx = 0; payloadtype = 'i';
	    	printf ("ID: %x Starting at payload index: %d Type: %c\n",keybrd_id, payloadidx,payloadtype);
	    	return 0;
	
	case 'f': // Payload as float
		payloadtype = 'f';
		break;
	case 'i': // Payload as integer
		payloadtype = 'i';
		break;
	default:
		printerror("Second char should be i or f\n");
		return -1;
	}
	switch (*(p+2)) // Get payload index
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			break;
		default:
			printerror("Payload index not 0 - 5\n");
			return -1;
	}
	sscanf((p+2), "%d %x",&payloadidx,&keybrd_id);
   return 0;
}

/******************************************************************************
 * void cmd_w_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_w_do_msg(struct CANRCVBUF* p)
{
	int i;
	union FINT
	{
		float f;
		uint8_t uc[4];
		uint32_t i;
	}uf;

	if (!((p->id & 0xfffffffc) == (keybrd_id & 0xfffffffc))) return;

	if (p->dlc < (payloadidx + 3))
	{
		printf("%08X %d dlc should be %d\n",p->id, p->dlc,(payloadidx + 3));
		return;
	}
	switch (payloadtype)
	{
	case ' ':
	case 'i':
		for (i = 0; i < 4; i++)	// Copy payload payload bytes into integer
			uf.uc[i] = p->cd.uc[i+payloadidx];							
		printf("%08X %d %8d\n",p->id, p->dlc, uf.i);
		break;

	case 'f':
		for (i = 0; i < 4; i++)	// Copy payload bytes into float
			uf.uc[i] = p->cd.uc[i+payloadidx];							
		printf("%08X %d %f\n",p->id, p->dlc, uf.f);
		break;

	default:
		printf("something wrong with w command. type = %c\n", payloadtype);
	}

	return;
}

