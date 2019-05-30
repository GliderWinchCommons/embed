/******************************************************************************
* File Name          : canlogpause.c
* Date First Issued  : 05/28/2019
* Board              : Linux PC
* Description        : Read a CAN log file and write msgs with pauses to simulate real-time
*******************************************************************************/
/*
Hack of cancnvtmatlabview.c 

Compile--
gcc -Wall canlogpause.c -o canlogpause

Execute--
 -i = Hex CAN id for time tick to pace copying
 -u = microseconds between CAN msgs
./canlogpause -i 00400000 -u 200
or 
./canlogpause -p 46800000 -t 15 -u 200
 -p = CAN id for pacing
 -t = milliseconds to pause with CAN id encountered
 -m = microseconds for other CAN msgs


*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../svn_common/trunk/db/gen_db.h"
#include "../../../svn_common/trunk/common_can.h"

#include <termios.h>
#include <time.h>

/* Line buffer size */
#define LINESZ 2048	// Longest CAN msg line length
char buf[LINESZ];

char *perror = "\
./canlogpause -i 00400000 -m 200\n\
 -i = Hex CAN id for time tick to pace copying\n\
 -u = microseconds between CAN msgs\n\
or \n\
./canlogpause -p 46800000 -t 15000 -m 200\n\
 -p = CAN id for pacing\n\
 -t = microseconds to pause with p CAN id encountered\n\
 -m = microseconds for other CAN msgs\n");



/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;

	// Command line value
	unsigned int tms;	// Pause time in millisec
	unsigned int tus; // Between CAN msgs microseconds
	unsigned int idp; // CAN id for time tick pause
	unsigned int idt; // CAN id for timed pause
	// Command line encounted
	unsigned int tmsx=0;	// Pause time in millisec
	unsigned int tusx=0; // Between CAN msgs microseconds
	unsigned int idpx=0; // CAN id for time tick pause
	unsigned int idtx=0; // CAN id for timed pause

	struct tv timem; // CAN duration 
		timem.tv_sec  = 0;
		timem.tv_usec = 200; // Time between CAN msgs (microseconds)

	struct tv timep; // Pacing
		timep.tv_sec  = 0;
		timep.tv_usec = 15625; // Time between bursts (microseconds)

	/* =============== Check command line arguments ===================================== */
	if (argc < 5) || (argc > 7)
	{
		printf("\nNot enough arguments\n",argc);
		printf("%s",perror);
		return -1;
	}

	for (i = 1; i < argc; i++)
	{
		if (**(argv+i) == '-')
		{ // Here command line switch argument
			switch ( *((*argv+i)+1) )
			{
			case 'T': case 't':
				sscanf( *(argv+i+1),"%i",&tms);
				tmsx = 1;
				timep.tv_usec = tms;
				i += 1;
				break;
			case 'P': case 'p':
				sscanf( *(argv+i+1),"%x",&idp);
				idpx = 1;
				timep.tv_usec = 15625; // 64/sec
				i += 1;
				break;
			case 'M': case 'm':
				sscanf( *(argv+i+1),"%x",&tus);
				tusx = 1;
				timem.tv_usec = tus;
				i += 1;
				break;
			case 'I': case 'i':
				sscanf( *(argv+i+1),"%x",&idt);
				idtx = 1;
				timep.tv_usec = 15625; // 64/sec
				i += 1;
				break;
			default:
				printf("options switch not recognized: %c\n",((*argv+i)+1));
				printf("%s",perror);
				return -1;
			}
		else
		{ // Here, not an option switch, Maybe
			printf("argument must be preceded by an option switch\n");
				printf("%s",perror);
				return -1;
		}
		if ((idpx != 0) && (idtx != 0)
		{
			printf("Having i and p options sws together is confusing\n");
			printf("%s",perror);
			return -1;
		}
	}
/* ==================== Pace in/out of data ================================================ */
	int m;
	struct CANRCVBUF can;


	if (idtx != 0)
		time.tv_usec = 15625; // 64/sec

	


	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		if (strlen(buf) > 12) // Skip lines too short to be useful
		{
//printf("IN : %s",&buf[2]);
			// The ascii hex order is little endian.  Convert to an unsigned int
			can.id = 0;
			for (m = 10; m >= 2; m-=2) // Build CAN id
			{
				sscanf(&buf[m],"%2x",&ui);
				can.id = (can.id << 8) + ui;
			}
			sscanf(&buf[10],"%2x",&can.dlc); // DLC

			// Get payload bytes	converted to binary
			for (m = 0; m < can.dlc; m++)
			{
				sscanf(&buf[12+2*m],"%2x",(unsigned int*)&can.cd.uc[m]);
			}
//printf("CAN: %08X\n",can.id);
			if (idtx != 0)
			{ // Here, use time tick CAN id to pace bursts
				if (can.id == can.id)
				{ // Here, time tick ID 
					select(NULL,NULL,NULL,&timep); // Delay
}


