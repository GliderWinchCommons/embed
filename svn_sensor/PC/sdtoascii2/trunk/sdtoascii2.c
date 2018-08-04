/************************************************************************************
sdtoascii2.c--SD card packet reader to file
06-20-2014, 0803/2018
Read SD card from logger and output ascii/hex lines (which can be piped to a file)
*************************************************************************************/
/*
Hack of ../../SD-read/trunk/sdpkf.c to implement date/time search and output ascii/hex lines

08/03/2018 update sdtoascii for newer CAN log SD format (no timestamp, and not compressed msgs)

// Example--
cd ~/svn_sensor/PC/sdtoascii/trunk
make && sudo ./sdtoascii2  [default to /dev/sdb]
make && sudo ./sdtoascii2 /dev/sdb
make && sudo ./sdtoascii2 ~/SDimageSD4
make && sudo ./sdtoascii2 -[options] <path/file>

options:
-g gateway format
-t time search

*/


#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



#include "packet_extract.h"
#include "packet_print.h"
#include "can_write.h"
#include "packet_search.h"
#include "sdlog.h"

void printhelp(void);
void progressprint(struct CANRCVBUF *p);

char *pfiledefault = "/dev/sdb"; // Default input: SD card

//#define BLOCKSIZE  512
unsigned char BlockBuff[SDCARD_BLOCKSIZE1];	// Current sd block
struct PKTP pktp;			// Current pid, packet size, packet pointer
struct PKTP pktp1;			// Current pid, packet size, packet pointer
#define PKTMAX	128
struct PKTP pktarray[PKTMAX];

struct LOGBLK blk;
struct BLOCKPARSED blkparse;


off64_t o_pos;
off64_t o_end;
off64_t o_blk;

int readablock_err;
int fd1;
int fd2;

uint32_t blknum;
uint32_t blkend;

/* Command line switches */
#define SWCT	5	// Number of switches to search
#define SW_G	sw[0]	// Gateway output format
#define SW_T	sw[1]	// Specify start end date/times
#define SW_B	sw[2]	// Specify start block number and count
#define SW_P	sw[3]	// Print to terminal in block format
#define SW_H	sw[4]	// Help
char cmd[SWCT] = {'g','t','b','p','h'};
int   sw[SWCT];		// 0 = switch is off, 1 = switch is on

/********************************************************************************
 * int looklikeaswitch(char *pfile, char *p, int *sw);
 * @brief	: Does this string look like a command line switch?
 * @param	: pointer to file name
 * @param	: pointer to argument
 * @param	: pointer to sw array
 * @return	:  1 = OK
 *		:  0 = first char not '-' so is must not be a switch
 *		: -1 = first char not '-' and next char not 't', 'g', 'b', etc.
 *		: -2 = help
********************************************************************************/
int looklikeaswitch(char *pfile, char *p, int *sw)
{
	int k,m,n;
	int any = 0;

	if (*p == '-')
	{ // First char of an argument means it is a switch
		k = strlen(p);
		for (m = 1; m < k; m++)
		{
			any = 0;
			for (n = 0; n < SWCT; n++)
			{
				if ( *(p+m) == cmd[n] )
				{ 
					*(sw+n) = 1; any += 1;
				}
			}
			if (any == 0)
			{
				printf("switch option not found: %c %s\n",*(p+m),p);
				printhelp();
				return -1;
			}
		}
		if (SW_H == 1) {printhelp(); return -2;}
		return 1;
	}
	strncpy (pfile,p, 256);
	return 0;	// 1st char not '-', so it is not a switch
}
/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
	int progress = 0;

	char filename[256];	// Input file path/name

	struct LOGBLK logblk;

	char vv[192];
	char ww[256];
	char *pc;

	struct tm t;
	time_t start_t;
	time_t t_low;
	time_t t_high;


	FILE *fpOut;

	int highest_nonzero_blk;
	int highest_pid;
	unsigned long long pidx,pidE,pid0;

	int count;
	int i;
	int j;
	int ipktend;

	int readct;
	int err;
	unsigned long long pid_prev;
	int sw_1st = 1;

	printf ("\n'sdtoascii2.c' - 08/02/2018\n\n");
	strcpy(filename, pfiledefault);	// Setup default input file name
	if (argc > 4)
	{
		printf("Too many arguments: %d\n", argc); return -1;
	}
	if (argc > 1)
	{ 
		for (i = 1; i < argc; i++)
		{
			if ((j=looklikeaswitch(filename, *(argv+i), sw)) < 0) return -1;
		}
	}

	printf("Input file name: %s\n",filename);
	/* Open the input file which is likely to be the SD card. */
	fd1 = open( filename, O_RDONLY);
	if (fd1 < 0) 
	{
		printf ("\nOPEN FAIL: %u %s %d\n\thint: did you use sudo?\n",argc,filename,fd1);
		return -1;
	}

	/* Command line switch: Enter block start and count */
	if (SW_B == 1)
	{ 
		printf("\nEnter: block_number  and count: \n");
		scanf("%u %u",&blknum,&count);		
		printf ("start at block %u, count %u\n",blknum,count);
		blkend = (blknum + count); // End and start block numbers
	}

	/* Command line switch: Enter start date/tiome */
	if (SW_T == 1)
	{ /* Get Start date/time */
		int opt = 0; int yy; int mm; int dd; int hh; int nn; int ss;
		while (opt == 0)
		{
			printf("Note: the hour you enter should be one lower if daylight time\n");
			printf("Enter yy mm dd hh nn (year (00 - 99) month (1-12)  day of month (1 - 31)  hour (0 - 23) minute (0 - 59) second (0 - 59)\nyy mm dd hh mm ss\n");
			scanf("%u %u %u %u %u %u",&yy, &mm, &dd, &hh, &nn, &ss);
			if ((yy < 0) || (yy > 99)) {printf("year    out of range: %u\n",yy); continue;}
			if ((mm < 1) || (mm > 12)) {printf("month   out of range: %u\n",mm); continue;}
			if ((dd < 1) || (dd > 31)) {printf("day     out of range: %u\n",dd); continue;}
			if ((hh < 0) || (hh > 23)) {printf("hour    out of range: %u\n",hh); continue;}
			if ((nn < 0) || (nn > 59)) {printf("minute  out of range: %u\n",nn); continue;}
			if ((ss < 0) || (ss > 59)) {printf("second  out of range: %u\n",ss); continue;}
			opt = 1;
		}

		/* Fill tm struct with values extracted from gps */
		t.tm_sec =	 ss;
		t.tm_min = 	 nn;
		t.tm_hour =	 hh;
		t.tm_mday =	 dd;
		t.tm_mon =	 mm-1;
		t.tm_year =	 yy + (2000 - 1900);
		t.tm_isdst = 0;		// Be sure daylight time is off
		start_t =  mktime (&t);	// Convert to time_t format

		printf("Search for start date/time: %s",ctime (&start_t));
	}

	int ret;
	int r = 0;
	time_t tim;
	while (1)
	{
	blk.blk = r++;
	ret = readablock(fd1, &blk);
	if (ret < 0)
	printf("readablock ret: %d\n",ret);

	ret = packet_extract_msgs(&blkparse, &blk.data[0]);
	if (ret < 0)
	{
		printf("packet_extract_msgs ret: %d\n",ret);
		if (ret == -1) break;
	}

	ret = packet_extract_time(&blkparse, 0xE1000000);
	if (ret >= 0)
	{
	//  printf("blk: %d TIME: %d\b\n",r, blkparse.nt);
	//  packet_extract_print(&blkparse);
	    printf("%4d ", r);
		 progressprint(&blkparse.can[blkparse.nt]);
       tim = packet_extract_uint32_t(&blkparse.can[blkparse.nt], 1);
		printf ("  %s", ctime(&tim));
//		printf("ret: 0x%08X\n",tim);
	}
 }	
		printf("\n=== DONE ====\n");
		return 0;
}
/********************************************************************************
 * void printhelp(void);
 * @brief	: Command line switch -h
********************************************************************************/
void printhelp(void)
{
	printf("HELP: usage: sdtoascii [options] [path/file for input]\noptions:\n");
	printf(" -g : gateway output format (default - winch (.WCH) format)\n");
	printf(" -t : Specify start:end date/times(not implemented)\n");
	printf(" -b : Specify start block number and count\n");
	printf(" -p : Print old block format (primitive debugging)\n");
	printf(" -h : This help listing\n");

	printf(" Example\nsudo ./sdtoascii -g /dev/sdf [gateway format] [input from SD card on /dev/sdf]\n");
	return;
}
/********************************************************************************
 * void progressprint(struct CANRCVBUF *p);
 * @brief	: List one CAN msg
********************************************************************************/
void progressprint(struct CANRCVBUF *p)
{
	int i;
	printf("%08X %d: ",p->id,p->dlc);
	for (i = 0; i < p->dlc; i++)
	{
		printf("%02X",p->cd.uc[i]);
	}
//	printf("\n");
	return;
}

