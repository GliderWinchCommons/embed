/******************************************************************************
* File Name          : canlogpause.c
* Date First Issued  : 05/28/2019
* Board              : Linux PC
* Description        : Read a CAN log file and write msgs with pauses to simulate real-time
*******************************************************************************/
/*
Hack of cancnvtmatlabview.c 

General idea--
  Read a log file with CAN msgs (gateway ascii format) and
pace the output of the msgs.  The output is piped to netcat
which sends to a TCP/IP connection to hub-server.  hub-server
distributes the msgs as if they were coming realtime from the
gateway.  

Compile--
gcc -Wall canlogpause.c -o canlogpause

Execute--
 -i = Hex CAN id for time tick to pace copying
 -p = microseconds when selected CAN id encountered
 -m = microseconds between CAN msgs
./canlogpause -i 00400000 -p 15625 -n 250 < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt

gcc -Wall canlogpause.c -o canlogpause && ./canlogpause -i 00400000 -p 15625 -m 250 < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt

Execute with default timings--
./canlogpause  < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt | nc localhost 32123

*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../svn_common/trunk/db/gen_db.h"
#include "../../../svn_common/trunk/common_can.h"

/* Line buffer size */
#define LINESZ 2048	// Longest CAN msg line length
char buf[LINESZ];

char *phelp = "\
./canlogpause -i 00400000 -p 15500 -m 200\n\
 -i = Hex CAN id for time tick to pace copying\n\
 -p = microseconds pause when CAN id encountered\n\
 -m = microseconds between other CAN msgs\n";

fd_set	ready;		/* used for select */
int fdp;	/* port file descriptor */

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;

	// Command line value
	unsigned int tms;	// Time between CAN msgs
	unsigned int tps; // Pause for specified CAN id

	struct timeval timem; // Between CAN msg duration (default)
		timem.tv_sec  = 0;
		timem.tv_usec = 250; // Time between CAN msgs (microseconds)

	struct timeval timep; // Pause for specified CAN id (default)
		timep.tv_sec  = 0;
		timep.tv_usec = 15625; // Time between bursts (microseconds)

	struct timeval timew; // Working copy
	

	unsigned int pauseid = 0x00400000; // CAN id to pace bursts (default)

	/* =============== Check command line arguments ===================================== */
	if  (argc > 7)
	{
		printf("\nToo many arguments: %i\n",argc);
		printf("%s",phelp);
		return -1;
	}

	char* px;
	char* px1;
	for (i = 1; i < argc; i++)
	{
		px  = argv[i+0];
		px1 = argv[i+1];
//printf("W: %i %s %s\n",i,px, px1);
		if ( *(px+0) == '-')
		{ // Here command line switch argument
			switch (*(px+1) )
			{
			case 'i':
				sscanf( px1,"%x",&pauseid);
				i += 1;
				break;
			case 'p':
				sscanf( px1,"%i",&tps);
				i += 1;
				break;
			case 'm':
				sscanf( px1,"%i",&tms);
				timem.tv_usec = tms;
				i += 1;
				break;
			default:
				printf("options switch not recognized: %c %s\n",*(px+1),px );
				printf("%s",phelp);
				return -1;
			}
		}
		else
		{ // Here, not an option switch, Maybe
			printf("argument must be preceded by an option switch\n");
				printf("%s",phelp);
				return -1;
		}
	}
//	printf("CANid: 0x%08X pause(us): %i intermsg(us): %i\n",pauseid,tps,tms);

/* ==================== Pace in/out of data ================================================ */
	int m;
	struct CANRCVBUF can;
	unsigned int ui;

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
			if (can.id == pauseid)
			{ // Here, use time tick CAN id to pace bursts
				timew = timep; // Delay
//printf("  TICK ");
			}
			else
			{
				timew = timem; // Delay
			}
			select(1,NULL,NULL,NULL,&timew); // Delay

			printf("%s",buf);	// Write input line to output
		}
	}
}


