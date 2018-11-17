/******************************************************************************
* File Name          : lineswzero.c
* Date First Issued  : 11/16/2018
* Board              : Linux PC
* Description        : Count lines with embedded zeroes
*******************************************************************************/
/*
gcc lineswzero.c -o lineswzero -Wall && ./lineswzero < ~/winch/download/181020/log1
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

int linectr = 0;

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int linectr = 0;
	int linezctr = 0;
	int c = '0';
	char* p;
/* ********* Loop until file ends ******************************************** */
	do
	{
		if (c == 0)
		{
			linezctr += 1;
		}
		if (c == '\n')
		{
			linectr += 1;
		}

	} while ((c=fgetc(stdin)) != EOF);
	
	/* Summary */
	printf("Total line ct                   : %d\n",linectr);
	printf("Lines with one or more zeroes ct: %d\n",linezctr);

	return 0;
}

