/******************************************************************************
* File Name          : canid-filter.c
* Date First Issued  : 09/07/2018
* Board              : Linux PC
* Description        : Select lines based on a file with CAN ids
*******************************************************************************/
/*
04/06/2024 revisions
gcc canid-filter.c -o canid-filter -lm && ./canid-filter filter-file-deh.txt

See script: CAN-filter-incoming

09/07/2018 Hack reformat2.c

05-20-2012 Uli test problem

// 01-29-2012 example--
gcc reformat2.c -o reformat2 -lm && sudo ./reformat2 ../dateselect/120128.210900RS
Or, when the input is via stdin
gcc reformat2.c -o reformat2 -lm && sudo ./reformat2 < ../dateselect/120128.210900RS

Selection file with canid in first column, '0x00000000', is 1st & only argument of path/name.  Input data is stdin.
gcc canid-filter.c -o canid-filter && ./canid-filter ~/GliderWinchCommons/embed/svn_sensor/PC/csa-datap180811_114712sdcan.can | tee ~/logcsa/180811_114712sdcan.can.filtered

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

#define LISTTHELIST

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

#define FILTERFILEMAX 512
uint32_t canid[FILTERFILEMAX];
int filtersz = 0;

FILE* fpIn;
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;
	int m;
	unsigned int ui;
	uint32_t id;


	if ( (fpIn = fopen (*(argv+1),"r")) == NULL)
	{
		printf ("\nInput file did not open %s\n",*(argv+1)); return -1;
	}

//printf("Filter file opend:%s\n",*(argv+1));

	while ( (fgets (&buf[0],LINESIZE,fpIn)) != NULL)
	{
		i = strlen(buf);
		if (i < 7) continue;
		if (buf[0] == '#') continue;


		sscanf(buf,"%x\n",&canid[filtersz]);
		filtersz += 1;
		if (filtersz > FILTERFILEMAX) exit(-1);
	}
#ifdef LISTTHELIST
	for (i = 0; i < filtersz; i++)
	{
		printf("%2d 0x%08X\n",i+1,canid[i]);
	}
#endif
/* ************************************************************************************************************ */
/* Read in lines formated with 'reformat.c' piped to 'sort' and piped to this routine                           */
/* ************************************************************************************************************ */
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		i = strlen(buf);
		if (i < 7) 
		{
			printf("\t\tSHORT: %i",i);
			continue;
		}
		// Convert CAN id to friendly format
		id = 0;
		for (m = 10; m >= 2; m-=2)
		{
			sscanf(&buf[m],"%2x",&ui);
			id = (id << 8) + ui;
		}
		// Check msg against list
		for (i = 0; i < filtersz; i++)
		{
			if (id == canid[i])
			{
				printf("%s",buf);
				break;			
			}
		}
	}
	return 0;
}

