/******************************************************************************
* File Name          : BMSlayoutrenum.c
* Date First Issued  : 03/27/2025
* Board              : Linux PC
* Description        : Renumber BMS layout lines
*******************************************************************************/
/*

cd ~/GliderWinchCommons/embed/svn_sensor/PC/cancnvtmatlab

gcc -Wall BMSlayoutrenum.c -o BMSlayoutrenum
./BMSlayoutrenum <start seq number>  < input file
./BMSlayoutrenum 65  < BMSrenum-250327.txt

gcc -Wall BMSlayoutrenum.c -o BMSlayoutrenum && ./BMSlayoutrenum 65  < BMSrenum-250327.txt
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
	int seq;
	char ctmp[128];

	sscanf(argv[1],"%d",&seq);
//printf("start seq number %d\n",seq);

	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		if (buf[0] == '#')
		{
			printf("%s",buf);
		}
		else
		{
			sprintf(ctmp,"%4i",seq);
			seq += 1;
			memmove(&buf[5],ctmp,4);
			printf("%s",buf); 
		}
	}
	return 0;
}