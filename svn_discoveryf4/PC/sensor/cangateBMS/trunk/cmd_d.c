/******************************************************************************
* File Name          : cmd_d.c
* Date First Issued  : 01/31/2022
* Board              : PC
* Description        : Display BMS cell readings
*******************************************************************************/
/*
*/

#include "cmd_d.h"
#include "../../../../../svn_common/trunk/db/gen_db.h"

static int compare_v(const void *a, const void *b);

#define CANID_HB1   0xB0201124
#define NCELL 18 // Max number of cells 

/* 16b codes for impossible voltage readings. */
#define CELLVBAD   65535 // Yet to be defined
#define CELLVNONE  65534 // Cell position not installed
#define CELLVOPEN  65533 // Installed, but wire appears open

 #define MISCQ_HEARTBEAT   0   // reserved for heartbeat
 #define MISCQ_STATUS      1 // status
 #define MISCQ_CELLV_CAL   2 // cell voltage: calibrated
 #define MISCQ_CELLV_ADC   3 // cell voltage: adc counts
 #define MISCQ_TEMP_CAL    4 // temperature sensor: calibrated
 #define MISCQ_TEMP_ADC    5 // temperature sensor: adc counts
 #define MISCQ_DCDC_V      6 // isolated dc-dc converter output voltage
 #define MISCQ_CHGR_V      7 // charger hv voltage
 #define MISCQ_HALL_CAL    8 // Hall sensor: calibrated
 #define MISCQ_HALL_ADC    9 // Hall sensor: adc counts
 #define MISCQ_CELLV_HI   10 // Highest cell voltage
 #define MISCQ_CELLV_LO   11 // Lowest cell voltage
 #define MISCQ_FETBALBITS 12 // FET on/off discharge bits
 #define MISCQ_TOPOFSTACK 18 // BMS top-of-stack voltage

struct CELLMSG
{
	float f;      // u16 converted to float
	uint16_t u16; // reading (100uv)
	uint8_t flag; // 0 = no reading; 1 = reading received
	uint8_t num;  // Cell number
};

static 	u32 canid1;
static struct CELLMSG cellmsg[NCELL];
static uint8_t hbseq;
static uint8_t request;

/******************************************************************************
 * static printmenu(char* p);
 * @brief 	: Print boilerplate
 * @param	: p = pointer to line
*******************************************************************************/
static void printmenu(char* p)
{
	printf(" d aaaaaaaa [CANid]\n");
	return;
}
/******************************************************************************
 * static printstatus(void);
 * @brief 	: Print bit meaninngs for status msg
*******************************************************************************/
static void printstatus(void)
{
	printf(
"BSTATUS_NOREADING (1 << 0) // Exactly zero = no reading\n"
"BSTATUS_OPENWIRE  (1 << 1) // Negative or over 5v indicative of open wire\n"
"BSTATUS_CELLTOOHI (1 << 2) // One or more cells above max limit\n"
"BSTATUS_CELLTOOLO (1 << 3) // One or more cells below min limit\n"
"BSTATUS_CELLBAL   (1 << 4) // Cell balancing in progress\n"
"BSTATUS_CHARGING  (1 << 5) // Charging in progress\n"
"BSTATUS_DUMPTOV   (1 << 6) // Dump to a voltage in progress\n\n"
"FET_DUMP     (1 << 0) // 1 = DUMP FET ON\n"
"FET_HEATER   (1 << 1) // 1 = HEATER FET ON\n"
"FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)\n"
"FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate\n"
"FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate\n\n"
"CELLBALANCE Discharge FET bits; e.g., 0x0001 = cell #1 FET ON\n\n"
"Battery FET CELLBALANCE\n"
	);
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

	printf("%c%c %i\n",*p,*(p+1),len);

	request = 'c';
	canid1 = CANID_HB1;

	if (len < 2)
	{
		printmenu("Too few chars\n");
		return -1;
	}
	else if (len >= 3)
	{	
		switch ( *(p+1) )
		{ 
		case 'c': // Cell readings display request
			request = 'c';
			break;

		case 's': // Status display request
			request = 's';
			
			break;

		case 'z': // Sorted by cell voltage
			request = 'z';
			break;

		case ' ': // default to cell readings & default CANID
		case '\n':
			request = 'c';
			break;

		default:
			printf("2nd char not recognized: %c\n", *(p+1));
			canid1 = 0;
			return -1;
		}

		if ((len < 13) && (len > 4))
		{ // Likely a CANID has beed added to command
			sscanf( (p+2), "%x",&canid1);		
		}
	}

	printf("request %c",request);
	if (request == 'c')
		printf(": cell readings by cell number");
	else if (request == 'z')
		printf(": cell readings sorted by voltage");
	else
		printf(": status");

	printf ("\tCANID: %08X\n",canid1);

	/* For cell readings, show no readings received & print header */
	if ((request == 'c') || (request == 'z'))
	{
		printf("\n  ");
		for (i = 0; i < NCELL; i++)
		{
			cellmsg[i].flag = 0;    // Set to no readings
			cellmsg[i].num = (i+1); // Cell numbers
			if (request == 'c')
				printf("%5d   ",i+1);
		}
		printf("\n");
	}
	else
		printstatus();
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
//printf("\n%08X %d: ", p->id,p->dlc);for (i=0; i<p->dlc; i++) printf(" %02X",p->cd.uc[i]);
/*	
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLPOLL',	42,	'[1]-[7] cell readings request: emc or pc');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_TYPE2',		43,	'[1]-[7] format 2: misc responses to poll');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLHB',	44,	'[1]-[7] cell readings: respojnse to timeout heartbeat');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_MISCHB',    45,	'[1]-[7] misc data: timeout heartbeat');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLEMC',   46,	'[1]-[7] cell readings: response to emc sent CELLPOLL');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_CELLPC',    47,	'[1]-[7] cell readings: response to pc sent CELLPOLL');
*/	/* Here, CAN msg is for us. */
	// Is it cell readings 
	if ( ((p->cd.uc[0] == CMD_CMD_CELLHB )  ||
		  (p->cd.uc[0] == CMD_CMD_CELLEMC)  ||
		  (p->cd.uc[0] == CMD_CMD_CELLPC )) && 
		 ((request == 'c') || (request == 'z')) )
	{
		// Check msg is for the same reading group
		if ((p->cd.uc[1] & 0xf) != (hbseq & 0xf))
		{ // Here sequence number changed. Assume all received
			// 
			if (request == 'z')
			{ // Here, sort array by voltage
				for (i = 0; i < NCELL; i++)
				{ // Set no-reading cells to zero
					if (cellmsg[i].flag == 0)
						  cellmsg[i].u16 = 0;
				}
				/* Sort on Cell voltages (ascending) */
				qsort(&cellmsg[0], NCELL, sizeof(struct CELLMSG), compare_v);
				// Print cell number header
				for (i = 0; i < NCELL; i++)
				{
					printf("%6i  ",cellmsg[i].num);
					cellmsg[i].num = i+1;
				}
				printf("\n");				

			}

			for (i = 0; i < NCELL; i++)
			{
				if (cellmsg[i].flag != 0)
				{
					if (cellmsg[i].u16 == CELLVNONE)
					{
						printf(" -9999.0");
//						printf("  .omit.");
					}
					else
					{
						if (cellmsg[i].u16 == CELLVOPEN)
						{
							printf("  7000.0");
						}
						else
						{
							dtmp = cellmsg[i].u16; // Convert to float
//printf("%8d",cellmsg[i].u16);							
							printf("%8.1f",dtmp*0.1); 
							cellmsg[i].flag = 0; // Clear was-read flag
						}
					}
				}
				else
				{ // No readings for this cell
					printf("    -1.0");
				}
			}
			printf("\n"); // print line
			/* Update to new sequence number. */
			hbseq = p->cd.uc[1] & 0xf;
		}

		/* Add readings in payload into array cell positions. */
		// Check payload size 
		switch(dlc)
		{
//			case 4: n = 1; break; 
//			case 6: n = 2; break; 
			case 8: n = 3; break; 

			default:
				{
					printf("\nDLC not 8 %d",dlc);
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
	/* Here, not a cell msg. */
	/*
#define BSTATUS_NOREADING (1 << 0)	// Exactly zero = no reading
#define BSTATUS_OPENWIRE  (1 << 1)  // Negative or over 5v indicative of open wire
#define BSTATUS_CELLTOOHI (1 << 2)  // One or more cells above max limit
#define BSTATUS_CELLTOOLO (1 << 3)  // One or more cells below min limit
#define BSTATUS_CELLBAL   (1 << 4)  // Cell balancing in progress
#define BSTATUS_CHARGING  (1 << 5)  // Charging in progress
#define BSTATUS_DUMPTOV   (1 << 6)  // Dump to a voltage in progress

#define FET_DUMP     (1 << 0) // 1 = DUMP FET ON
#define FET_HEATER   (1 << 1) // 1 = HEATER FET ON
#define FET_DUMP2    (1 << 2) // 1 = DUMP2 FET ON (external charger)
#define FET_CHGR     (1 << 3) // 1 = Charger FET enabled: Normal charge rate
#define FET_CHGR_VLC (1 << 4) // 1 = Charger FET enabled: Very Low Charge rate

	*/	
	if ((p->cd.uc[0] == CMD_CMD_TYPE2)  && (request == 's'))
	{ // Here, status msgs
		printf("%02X %02X %04X\n",p->cd.uc[4],p->cd.uc[5],p->cd.us[3] );
	}
	return;
}
/* ************************************************************************************************************ 
 * Comparison function for qsort
 * ************************************************************************************************************ */
static int compare_v(const void *a, const void *b)
{
    const struct CELLMSG *da = (const struct CELLMSG *) a;
    const struct CELLMSG *db = (const struct CELLMSG *) b;
     
    return (da->u16 > db->u16) - (da->u16 < db->u16);
}

