/******************************************************************************
* File Name          : CANcellreadings.c
* Date First Issued  : 07/19/23
* Board              : Linux PC
* Description        : Convert logged CAN msgs to lines with cell readings
*******************************************************************************/
/*
Directory: ~/GliderWinchCommons/embed/svn_sensor/PC/CANcellreadings

Compile and execute example with one argument with CAN ID to select lines from STD input--
gcc CANcellreadings.c -o CANcellreadings -lm && ./CANcellreadings B0A00000 < ~/GliderWinchItems/BMS/PC/filterminicom/minicom-230718-201800.CAN
gcc CANcellreadings.c -o CANcellreadings -lm && ./CANcellreadings B1000000 < ~/G*ems/BMS/minicom-230724-152450.CAN
gcc CANcellreadings.c -o CANcellreadings -lm && ./CANcellreadings B1000000 < ~/G*ems/BMS/minicom-230724-152450.CAN | tee x

Procedure--
- Log CAN msgs
- Run CANcellreadings: ./CANcellreadings <CAN_ID> < <path/file>
- Save output add: | tee <file name> skip "| tee" and use "> <file name>"

Log file input snippet--
8E000040000112D3
8F000040000113D5
90000040000114D7
91000040000115D9
92000040000116DB
93000000B1082C06C69EB99E989E6A
94000000B1082C36E69EAC9E0A9F20
95000000B1082C66D29D129E3C9ED3
96000000B1082C96D19DEE9D799E1D
97000000B1082CC6709EE69E889EF6
98000000B1082CF6B09E629EBF9D19
99000040000117E3
9A000040000118E5
9B000040000119E7
9C00004000011AE9
9D00004000011BEB
9E00004000011CED
9F00004000011DEF
A000004000011EF1
A100004000011FF3


*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../svn_common/trunk/common_can.h"	// Definitions common to CAN and this project.
#include "../../../svn_common/trunk/db/gen_db.h"

/* 16b codes for impossible voltage readings. */
#define CELLVBAD   65535 // Yet to be defined
#define CELLVNONE  65534 // Cell position not installed
#define CELLVOPEN  65533 // Installed, but wire appears open

void do_msg(struct CANRCVBUF* p);

struct CANRCVBUF can;

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

#define NCELL 18

uint8_t groupctr; // The six cell readings are sent in a group.

double sum[18];
double ave[18];

struct CELLMSG
{
	float f;      // u16 converted to float
	uint16_t u16; // reading (100uv)
	uint8_t flag; // 0 = no reading; 1 = reading received
	uint8_t num;  // Cell number
};

u32 canid1;
struct CELLMSG cellmsg[NCELL];
uint8_t hbseq;
uint32_t ctr; // Input line counter
uint32_t ctrreadings; // Number of reading groups
uint32_t ctrout = 0;
uint32_t ctrchkerr = 0;
uint32_t ctrdlcerr = 0;
uint32_t ctrlinect = 0;
uint32_t ctrreadct = 0;

FILE* fpIn;
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;
	int j;
	int m;
	unsigned int uitmp;
	uint32_t id;
	uint32_t id_arg;


	uint32_t seq;

	union UI
	{
		uint8_t uc[4];
		uint32_t ui;
	}ui;

	if (argc != 2)
	{
		printf("\nNeed one and only one argument: I see %d\n",argc-1);
		return -1;
	}

	sscanf(*(argv+1),"%X",&id_arg);

	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		ctr += 1;
		if (strlen(buf) < 10) 
		{
			ctrlinect += 1;
			continue;
		}

		sscanf(&buf[ 0],"%2X",&seq);
		sscanf(&buf[10],"%2X",&uitmp);
		can.dlc = uitmp;
		if (uitmp > 8) 
		{
			ctrdlcerr += 1;
			continue;
		}
	uint32_t chk = CHECKSUM_INITIAL; //	0xa5a5	// Initial value for computing checksum
		chk += seq + can.dlc;
		for (i = 0; i < 4; i++)
		{
			sscanf(&buf[2+2*i],"%2X",&uitmp);
			ui.uc[i] = uitmp;
			chk += ui.uc[i];

		}
		can.id  = ui.ui;

		for (i = 0; i < can.dlc; i++)
		{
			sscanf(&buf[12+i*2],"%2X",&uitmp);
			can.cd.uc[i] = uitmp;
			chk += uitmp;			
		}

		chk += (chk >> 16);	// Add carries into high half word
		chk += (chk >> 16);	// Add carry if previous add generated a carry
		chk += (chk >> 8);  // Add high byte of low half word
		chk += (chk >> 8);  // Add carry if previous add generated a carry

		// Get checksum from line
		sscanf(&buf[12+i*2],"%2X",&uitmp);
//printf("chk: %08X, %s", chk, buf);

		if (can.id == id_arg)
		{
			if ((chk & 0xff) == (uitmp & 0xff))
			{
				do_msg(&can);
				ctrout += 1;
			}
			else
			{ // Count msgs with checksum errors
				ctrchkerr += 1;
			}

//			printf(buf,"%s");
//			printf("%5d: %08X %d: ",ctr,ui.ui,can.dlc);
//			for (i = 0; i < can.dlc; i++) printf("%02X",can.cd.uc[i]);
//			printf("\n");



		}
	}
	printf("count of lines in:     %7d\n",ctr);
	printf("line length too short: %7d\n",ctrlinect);
	printf("count selected 0x%08X: %7d divided by 7: %d\n",id_arg,ctrout,ctrout/7 );
	printf("ctrchkerr: %7d\n",ctrchkerr);
	printf("ctrdlcerr: %7d\n",ctrdlcerr);
	printf("ctrreadct: %7d\n",ctrreadct);
	printf("count reading groups: %d times 6: %d\n",ctrreadings, 6*ctrreadings);

		return 0;
}
void do_msg(struct CANRCVBUF* p)
{
	uint8_t i;
	uint8_t n;
	uint8_t dlc = p->dlc;
	uint8_t celln;
	double dtmp;

	if ( ((p->cd.uc[0] == CMD_CMD_CELLHB )  ||
		  (p->cd.uc[0] == CMD_CMD_CELLEMC)  ||
		  (p->cd.uc[0] == CMD_CMD_CELLPC ))  )
	{	
		ctrreadct += 1;
		// Check msg is for the same reading group
		if ((p->cd.uc[1] & 0xf) != (hbseq & 0xf))
		{ // Here sequence number changed. Assume all received
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
			ctrreadings += 1; // Number of reading groups
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
}

