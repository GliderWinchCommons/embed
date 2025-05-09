/******************************************************************************
* File Name          : cmd_w.c
* Date First Issued  : 04/11/2018
* Board              : PC
* Description        : Details for handling command 'w' list float or integer payload
*******************************************************************************/
/*
*/

#include "cmd_w.h"

static 	u32 keybrd_id;
static 	u32 payloadidx;	// Can payload byte index 0 - 5
static 	char payloadtype; // integer or float
static   char type;
static u32 msgctr; // Running count of incoming msgs
static u32 msgctr_prev;

/******************************************************************************
 * int cmd_w_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
void printerror(char *p)
{
	printf("%s",p);
	printf("w 30e00000 [integer payload starting at [0]]\n");
	printf("wi0 30e00000 [integer payload starting at [0]]\n");
	printf("wi1 30e00000 [integer payload starting at [1]]\n");
	printf("wf0 30e00000 [float payload starting at [0]]\n");
	printf("wf1 30e00000 [float payload starting at [1]]\n");
	printf("wfx 30e00000 [float payload starting at [x] x 0 - 5]\n");
	printf("wy0 30e00000 [integer payload starting at [0] # BIG ENDIAN #]\n");
	printf("wb0 30e00000 [byte payload starting at [0]]\n");
	printf("wff 83e00000 Two floats\n");
	printf("wfi 83a00000 One float [0], one int[4]\n");
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
		if (*(p+2) == 'f')
			payloadtype = '2'; // Two floats (FF_FF)
		else
		{
			if (*(p+2) == 'i') // Float int (FF_S32)
				payloadtype = '3';
			else
				payloadtype = 'f'; // Single float w offset idx
		}
		break;
	case 'i': // Payload as integer
		payloadtype = 'i';
		break;
	case 'h': // Payload as half word
		payloadtype = 'h';
		break;
	case 'y': // Payload as 2 byte int: Big Endian
		payloadtype = 'y';
		break;
	case 'b': // Payload as integer byte
		payloadtype = 'b';
		break;
	default:
		printerror("Second char should be i,f,y, or b\n");
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
		case '6':
		case '7':
			payloadidx = *(p+2) - '0'; // Convert to decimal
			break;
		case 'f':
		case 'i':
			break;
		default:
			printerror("Payload index not 0 - 5, or\n  wff <can id>\n  wfi <can id>\n");
			return -1;
	}
	sscanf((p+3), "%x",&keybrd_id);
  	printf ("ID: %x Starting at payload index: %d Type: %c\n",keybrd_id, payloadidx,payloadtype);
  	msgctr = 0;
  	msgctr_prev = 0;
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
	int32_t diff;
	union FINT
	{
		float f;
		uint8_t uc[4];
		uint16_t u16[2];
		uint32_t i;
		int32_t n;
	}uf;

	msgctr += 1;

	if (!((p->id & 0xfffffffc) == (keybrd_id & 0xfffffffc))) return;

	if (p->dlc < (payloadidx + 2))
	{
		printf("%08X %d dlc should be %d\n",p->id, p->dlc,(payloadidx + 2));
		return;
	}

	/* Number of CAN msgs between selected msg. */
	diff = (int32_t)(msgctr - msgctr_prev);
	msgctr_prev = msgctr;

	switch (payloadtype)
	{
	case ' ':
	case 'i':
		for (i = 0; i < 4; i++)	// Copy payload payload bytes into integer
			uf.uc[i] = p->cd.uc[i+payloadidx];							
		printf("LE FULL: %6d %4d %08X %d %8d\n",msgctr, diff, p->id, p->dlc, uf.i);
		break;

	case 'h':
		uf.u16[0] = (p->cd.uc[payloadidx+1] << 8) + p->cd.uc[payloadidx];
		printf("LE HALF: %6d %4d %08X %d %8d\n",msgctr, diff, p->id, p->dlc, uf.u16[0]);
		break;
		
	case 'y': // Big Endian 2 byte int payload
			uf.i = (p->cd.uc[payloadidx] << 8) + p->cd.uc[payloadidx+1];
		printf("BE HALF: %6d %4d %08X %d %d %8d\n",msgctr, diff, p->id, payloadidx, p->dlc, uf.n );
		break;

	case 'b': // Single byte
		printf("BYTE: %6d %4d %08X %d %4d %02X\n",msgctr, diff, p->id, p->dlc,p->cd.uc[payloadidx],p->cd.uc[payloadidx]);
		break;

	case 'f':
		for (i = 0; i < 4; i++)	// Copy payload bytes into float
			uf.uc[i] = p->cd.uc[i+payloadidx];							
		printf(" %6d  %4d %08X %d %15.6f\n",msgctr, diff, p->id, p->dlc, uf.f);
		break;

	case '2': // Two floats, payload type: (FF_FF)
		for (i = 0; i < 4; i++)	// Copy payload bytes into float
			uf.uc[i] = p->cd.uc[i];							
		printf(" %6d  %4d %08X %d %15.6f ",msgctr, diff, p->id, p->dlc, uf.f);
		for (i = 0; i < 4; i++)	// Copy payload bytes into float
			uf.uc[i] = p->cd.uc[i+4];							
		printf(" %15.6f\n",uf.f);
		break;

    case '3': // Float, int, payload type: (FF_S32)
		for (i = 0; i < 4; i++)	// Copy payload bytes into float
			uf.uc[i] = p->cd.uc[i];							
		printf(" %6d  %4d %08X %d %15.6f ",msgctr, diff, p->id, p->dlc, uf.f);
		printf(" %15d\n",p->cd.ui[1]);
		break;


	default:
		printf("something wrong with w command. type = %c\n", payloadtype);
	}

	return;
}

