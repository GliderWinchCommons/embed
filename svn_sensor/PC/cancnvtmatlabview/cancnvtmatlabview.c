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

Test with previously converted logfile--
gcc -Wall cancnvtmatlabview.c -o cancnvtmatlabview &&  ./cancnvtmatlabview viewselect1.txt < ../cancnvtmatlab/logfile190524c.csv

~/GliderWinchCommons/embed/svn_sensor/PC/canlogpause$ ./canlogpause < ~/GliderWinchItems/dynamometer/docs/data/logfile190601.txt | ../cancnvtmatlab/cancnvtmatlab ../cancnvtmatlab/csvlinelayout.txt ../cancnvtmatlab/csvfieldselect.txt | ../cancnvtmatlabview/cancnvtmatlabview ../cancnvtmatlabview/viewselect1.txt

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

/* Declarations */
char* findcsv(uint32_t n);

/* Line buffer size */
#define LINESZ 2048	// Longest CAN msg line length
char buf[LINESZ];

#define FILECTSZ 4	// Max number of specification files
FILE* fpIn[FILECTSZ];

#define CSVLINESZ 1024	// Max output line size
char cline[CSVLINESZ]; // CSV output array
	char* pcline;	        // CSV line pointer

#define MAXCOLDAT   32	// Max number of columns to display
#define AVEMAXSIZE 256	// Max number of readings to average
#define MAXSIZECOMMENT 96
struct COLMNDAT
{
	uint32_t csvpos;		// CSV field position
	uint32_t charct;		// Number chars in column
	uint32_t avesz;		// Number readings in average
	float iircoef;			// Single pole iir coefficient (0 = no iir)
	float f[AVEMAXSIZE]; // Readings for average
	char c[MAXSIZECOMMENT];
	
};
struct COLMNDAT colmndat[MAXCOLDAT];
uint32_t colidx;

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;
	int j;
	int m;

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
	char* pa;
	char* pb;
	char* pf;
	float field;

		if ( (fpIn[0] = fopen (argv[1],"r")) == NULL)
		{
			printf ("\nInput file did not open: %s\n",argv[1]); 
			return -1;
		}

		/* Load data from this file, ignoring comment lines. */
		i = 0; 
		while ( (fgets (&buf[0],LINESZ,fpIn[0])) != NULL)	// Get a line
		{
			if (strlen(buf) < 4) continue;
			switch (buf[0])
			{
			case 'R':
				sscanf(&buf[1],"%i %i",&ratecsv, &rateoutput);
				if ((ratecsv < 1) || (ratecsv > 128))
				{
					printf("ERR: ratecsv out-of-bounds: %d\n",ratecsv);
					return -1;
				}
				if (rateoutput < 1)
				{
					printf("ERR: rateoutput too small: %d\n",rateoutput);
					return -1;
				}
				if (rateoutput > ratecsv)
				{
					printf("ERR: rateoutput greater than csv line rate: %d\n",rateoutput);
					return -1;
				}
				printf("#\n# csv rate     (lines/sec): %d\n",ratecsv);
				printf("# output rate (lines /sec): %d\n\n",rateoutput);
					break;

			case 'V':
				sscanf(&buf[1],"%i %i %i %f",&colmndat[i].csvpos,&colmndat[i].charct,&colmndat[i].avesz,&colmndat[i].iircoef);
				pa = &buf[1];
				pa = strchr(pa,'"');
				if (pa == NULL) {printf("ERR: first double-quote missing\n"); break;}
				pa += 1;
				pb = pa;
				pb = strchr(pb,'"');
				if (pa == NULL) {printf("ERR: second double-quote missing\n"); break;}
				if ((pb - pa) >= MAXSIZECOMMENT) {printf("comment field looks too long\n");break;}
				strncpy(&colmndat[i].c[0],pa,(pb-pa));
				*pb = 0;

				i += 1;
				if (i >= MAXCOLDAT) i -= 1;
					break;

			case '#':
			case ' ':
				break;
			default:
				printf("ERR: BOGUS first char: %c %s",buf[0],buf);
			}
		}
		colidx = i;

		for (i = 0; i < colidx; i++)
		{
			printf("# %2i %3i %3i %3i %10.6f ",i,colmndat[i].csvpos,colmndat[i].charct,colmndat[i].avesz,colmndat[i].iircoef);
			printf("%s\n",colmndat[i].c);
		}

/* ==================== input data  & make columns ================================================ */
	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		if (strlen(buf) > 12) // Skip lines too short to be useful
		{
//printf("%s",buf);
			pcline = &cline[0];
			for (i = 0; i < colidx; i++)
			{
				pa = findcsv(colmndat[i].csvpos);
				pf = pa;
				pb = strchr(pa,',');
				m = (pb-pa);
				if (m < 1) continue;
				for (j = 0; j < (colmndat[i].charct - m); j++) 
					*pcline++ = ' '; // Pad
				for (j = 0; j < m; j++)
					*pcline++ = *pa++; // Copy data
				*pcline = 0;
				sscanf(pf,"%f",&field);
				printf("%10.3f ",field);
			}
			printf("%s\n",cline);
		}		
	}
}

/* Current line position */
char* px = &buf[0]; // Pointer for CSV field
uint32_t nx = 1;    // CSV field number
/* ******************************************* 
 * char* findcsv(uint32_t n);
 * @brief	: Find CSV field given CSV field number
 * @param	: n = CSV field number (1 - ...)
 * @return	: pointer to first char of CSV field
 *********************************************/
char* findcsv(uint32_t n)
{
//printf("nx: %i n: %i\n",nx,n);
//printf("%s",buf);
	if (n < nx) 
	{ // Move back to beginning
		px = &buf[0];
		nx = 1;
	}
	if ((nx==1) && (n==1)) return px;

	while (nx != n)
	{
//printf("nx: %i n: %i px: %s\n",nx,n,px);
		px = strchr(px,',');
		if (px == NULL) 
		{
			px = &buf[0];
			nx = 1;	
			return px;
		}
		px += 1; // Step off of ','
		nx += 1; // CSV field count
	}
	return px;
}


