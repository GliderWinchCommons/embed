/******************************************************************************
* File Name          : cmd_i.c
* Date First Issued  : 11/13/2020
* Board              : PC
* Description        : LEVELWIND: display hearbeat: status & state
*******************************************************************************/
/*
11/13/2020 Hack from cmd_s.c

*/

enum CONTACTOR_CMD_CODES
{
	ADCRAW5V,         // PA0 IN0  - 5V sensor supply
	ADCRAW12V,        // PA7 IN7  - +12 Raw power to board
	ADCSTEPV,			// +96v stepper supply
};
#define NUM36SUBCMDREQ 5	// Number of sub-commands for gen cnd 36

#include "cmd_i.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

#define DEFAULTRX 0x80000000 // CANID_HB_LEVELWIND_1','','LEVELWIND',1,2,'S8_U8','DRUM 1: S8:Status,U8:state');

#define  LW_STATUS_GOOD              0 
// Red Statii
#define  LW_STATUS_OFF_AFTER_ERROR  -1 
#define  LW_STATUS_OVERRUN          -2
#define  LW_STATUS_MANUAL           -3
#define  LW_STATUS_INDEX_FAILURE    -4
#define  LW_STATUS_SWEEP_FAILURE    -5
#define  LW_STATUS_LOS_FAILURE      -6
#define  LW_STATUS_STEPPER_POWER    -7
#define  LW_STATUS_NODE_POWER       -8
//Yellow Statii
#define  LW_STATUS_SWEEPING          1 
#define  LW_STATUS_ARRESTING         2
#define  LW_STATUS_LOS_REINDEXING    3
#define  LW_STATUS_LOS_TRACKING      4

/* State */
#define LW_OFF       (0 << 4)
#define LW_OVERRUN   (1 << 4)
#define LW_MANUAL    (2 << 4)
#define LW_CENTER    (3 << 4)
#define LW_INDEX     (4 << 4)
#define LW_TRACK     (5 << 4)
#define LW_LOS       (6 << 4)

/******************************************************************************
 * void cmd_i_do_msg(struct CANRCVBUF* p);
 * @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/
void cmd_i_do_msg(struct CANRCVBUF* p)
{
	/* Select LEVELWIND hearbeat CAN msg. */
	if ((p->id & 0xfffffffc) != DEFAULTRX) return;

// Debugging
printf("RX %08X %i ",p->id,p->dlc);
int i;
for (i = 0; i < p->dlc; i++) printf(" %02X",p->cd.uc[i]);
//printf("\n");
	
	/* status */
	printf("%3i ",p->cd.uc[0]); // Status code
	switch (p->cd.sc[0])
	{
	case  LW_STATUS_GOOD:             //  0
		printf("LW_STATUS_GOOD           "); break;
	case  LW_STATUS_OFF_AFTER_ERROR:  // -1 
		printf("LW_STATUS_OFF_AFTER_ERROR"); break;
	case  LW_STATUS_OVERRUN:          // -2
		printf("LW_STATUS_OVERRUN        "); break; 
	case  LW_STATUS_MANUAL:           // -3
		printf("LW_STATUS_MANUAL         "); break;
	case  LW_STATUS_INDEX_FAILURE:    // -4
		printf("LW_STATUS_INDEX_FAILURE  "); break;
	case  LW_STATUS_SWEEP_FAILURE:    // -5
		printf("LW_STATUS_SWEEP_FAILURE  "); break;
	case  LW_STATUS_LOS_FAILURE:      // -6
		printf("LW_STATUS_LOS_FAILURE    "); break;
	case  LW_STATUS_STEPPER_POWER:    // -7
		printf("LW_STATUS_STEPPER_POWER  "); break;
	case  LW_STATUS_NODE_POWER:       // -8
		printf("LW_STATUS_NODE_POWER     "); break;
	case  LW_STATUS_SWEEPING:         //  1 
		printf("LW_STATUS_SWEEPING       "); break;
	case  LW_STATUS_ARRESTING:        //  2
		printf("LW_STATUS_ARRESTING      "); break;
	case  LW_STATUS_LOS_REINDEXING:   //  3
		printf("LW_STATUS_LOS_REINDEXING "); break;
	case  LW_STATUS_LOS_TRACKING:     //  4
		printf("LW_STATUS_LOS_TRACKING   "); break;
	default:
		printf("status code not coded    "); break;
	}

	/* state */

	printf("%2X ",p->cd.uc[1]); // State code
	switch (p->cd.uc[1])
	{
	case LW_OFF:  // (0 << 4)
		printf("LW_OFF    "); break;  
	case LW_OVERRUN: // (1 << 4)
		printf("LW_OVERRUN");; break;  
	case LW_MANUAL:  // (2 << 4)
		printf("LW_MANUAL ");; break;  
	case LW_CENTER:  // (3 << 4)
		printf("LW_CENTER ");; break;  
	case LW_INDEX:   // (4 << 4)
		printf("LW_INDEX  ");; break;  
	case LW_TRACK:   // (5 << 4)
		printf("LW_TRACK  ");; break;  
	case LW_LOS:     // (6 << 4)
		printf("LW_LOS    ");; break;  
	default:
		printf("no decode ");; break;  
	}
	printf("\n");

	return;
}

