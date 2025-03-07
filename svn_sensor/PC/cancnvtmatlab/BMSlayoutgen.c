/******************************************************************************
* File Name          : BMSlayoutgen.c
* Date First Issued  : 03/05/2025
* Board              : Linux PC
* Description        : Generate the BMS sections for the cancnvtlayout.txt for the BMS
*******************************************************************************/
/*

cd ~/GliderWinchCommons/embed/svn_sensor/PC/cancnvtmatlab

gcc -Wall BMSlayoutgen.c -o BMSlayoutgen
./BMSlayoutgen <start seq number>  < input file
./BMSlayoutgen 59  < BMSlayoutgen.txt
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

#define LINESZ 1024
char buf[LINESZ];
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i;
	int seq;
	char ctmp[128];

	sscanf(argv[1],"%d",&seq);
//printf("start seq number %d\n",seq);

	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
//printf("%4i %s",seq++,buf);
		printf("#\n");
		for (i = 0; i < 27; i++)
		{
			sprintf(ctmp,"%4i",seq);
			seq += 1;
			memmove(&buf[1],ctmp,4);

			sprintf(ctmp,"%2i",i+1);
			memmove(&buf[15],ctmp,2);

//T  76 B1200000 18 51     0.0    1.0 "CANID_UNIT_BMS07"			
			printf("%s",buf); 
		}
	}
	return 0;
}