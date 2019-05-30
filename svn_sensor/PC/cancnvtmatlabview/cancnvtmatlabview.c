/******************************************************************************
* File Name          : cancnvtmatlabview.c
* Date First Issued  : 05/26/2019
* Board              : Linux PC
* Description        : Select csv fields from output from cancnvtmatlab.c and present as columns
*******************************************************************************/
/*
Hack of cancnvtmatlab.c 

Compile--
gcc -Wall cancnvtmatlabview.c -o cancnvtmatlabview 

Convert log file data to csv file and pipe to cancnvtmatlabview to present selected csv fields in columns--
./cancnvtmatlab csvlinelayout.txt csvfieldselect.txt < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt | ./cancnvtmatlabview viewselect.txt | tee z

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

/* Line buffer size */
#define LINESZ 2048	// Longest CAN msg line length
char buf[LINESZ];




/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k;
int mm = 0;
char* pv;

	/* =============== Check command line arguments ===================================== */
	if (argc != 2)
	{
		printf("\nOne argument with a file path/name required.  argc = %d\n",argc);
		return -1;
	}

	/* =============== Read input file ================================================ */
/*
# Column 1: Type of line: 'R' = rates
# Column 2: CSV line rate (csv lines/sec)
# Column 3: Output rate for this program (lines/sec)
R  64  2
#
# Column 1: Type of line: 'V' = Identifies as a view select line
# Column 2: CSV line field position number to be viewed (see csvfieldselect.txt)
# Column 3: Number of chars allowed for field, including spaced between columns
# Column 4: Running average (1 - 128)
# Column 5: IIR filter coeff ( = no IIR filtering)
# Column 6: Description of field (copy it from csvfieldselect.txt)
#
#   2   3  4    5     6
V   1  12 32  0.0 "Tension_a11: Drum 1 calibrated tension, polled by time msg"
V   3  12 32  0.0 "Tension_a12: Drum 1 calibrated tension, polled by time msg"
V   9   7 32  0.0 "DMOC HV status: voltage (volts)"
V  10   7 32  0.0 "DMOC HV status: current (amps)"
V  11   8 32  0.0 "DMOC Actual Speed (rpm?)"
V  17   8 32  0.0 "DMOC Actual Torque"
*/

int ratecsv = 0;	// Number of csv lines per sec
int rateoutput = 0; // Number of output lines per sec

		if ( (fpIn[0] = fopen (argv[1],"r")) == NULL)
		{
			printf ("\nInput file did not open: %s\n",argv[2]); 
			return -1;
		}

		/* Load data from this file, ignoring comment lines. */
		while ( (fgets (&buf[0],LINESZ,fpIn[2])) != NULL)	// Get a line
		{
			switch (buf[0])
			{
			case 'R':
				sscanf(buf,"%d %d",&ratecsv, &rateoutput);
				if ((ratecsv < 1) || (ratecsv > 128))
				{
					printf("ERROR: ratecsv out-of-bounds: %d\n",ratecsv);
					return -1;
				}
				if (rateoutput < 1)
				{
					printf("ERROR: rateoutput too small: %d\n",rateoutput);
					return -1;
				}
				if (rateoutput > ratecsv)
				{
					printf("ERROR: rateoutput greater than csv line rate: %d\n",rateoutput);
					return -1;
				}
				printf("\ncsv rate     (lines/sec): %d\n",ratecsv);
				printf("output rate (lines /sec): %d\n\n",rateoutput);
					break;

			case 'V':


					break;

			case '#':
			case ' ':
				break;
			default:
				printf("BOGUS first char: %c %s",buf[0],buf);
			}

		}
mm += 1;
	}

/* ==================== Convert input data ================================================ */
	int m;
	unsigned int ui;
	uint32_t nn;
	struct CANRCVBUF can;
	struct CANFLDLAYOUT* pcanfldlayout;
	char* pfmt;            // Point to format for this field
	double dtmp;           // Temp of last-good-reading
	char cline[CSVLINESZ]; // CSV output array
	char* pcline;	        // CSV line pointer
	int skip = 0; // Skip count to throttle output for debugging

	pcline = &cline[0];
	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		if (strlen(buf) > 12) // Skip lines too short to be useful
		{
		}		
	}
}


