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


#include "PC_gateway_comm.h"
#include "packet_extract.h"
#include "packet_print.h"
#include "can_write.h"
#include "packet_search.h"
#include "sdlog.h"

#define CANID_TIME 0xE1000000	// Default CAN ID time msg
uint32_t canid_time = CANID_TIME;

#define CANID_TIMESYNC 0x00400000	// Default CAN ID for time sync msgs
uint32_t canid_timesync = CANID_TIMESYNC;

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

int blknum;
int blkmax;	// Number blocks on card
int blkend;
int blkuse;	// Max less end-of-block reserved space

int sw_firsttime = 0;	// First time msg of run
time_t time_init;
time_t time_work;

char firsttime[32];		// Formatted time from first time msg
uint32_t synctime;		// time sync msg unix time
struct TIMEIDX
{
	uint32_t utime;	// unix time
	uint32_t nt;		// index in CAN msgs in a SD block
	long long oset;	// output file offset
};

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
	char filename[256];	// Input file path/name

	struct tm t;
	time_t start_t;

	FILE *fpOut;
	FILE *fpOut1;

	unsigned long long pidE,pid0;

	int count;
	int i;
	int j;


	printf ("\n'sdtoascii2.c' - 08/05/2018\n\n");
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

	/* Find end of SD card */
	o_pos = lseek64(fd1,(off64_t)0,SEEK_END);
	o_end = o_pos;		// Save end of SD position
	blkmax = o_pos/512;	// Number of blocks on SD card
	printf("Success: %u %s, SEEK_END returned (blocks) %d\t  %lli (bytes) \n",argc,*(argv+1),blkmax,(long long)(blkmax)*512L);

	blkuse = (blkmax - 1 - SDLOG_EXTRA_BLOCKS);

	// NOTE: 'getaPID' returns (PID + 1), so PID = 0 conveys an error.
	pidE = getaPID(fd1, blkuse); // Last block logging writes
	pid0 = getaPID(fd1, 0);
	if ((pidE == 0) || (pid0 == 0)) 
	{
 		printf("Error reading SD card: returned the PID block[0] = %llu block[%u] = %llu\n",pid0,blkuse,pidE); 
		return -1;
	}
	if (pidE == 1)
	{ // Here high end PID is zero.  Non-wrap situation
		if (pid0 == 1)
		{ // Here.  Beginning & End blocks PIDs are zero
			printf("NO DATA ON CARD: Beginning and End PIDs are both zero\n"); 
			return -1;
		}
	}
	printf("pidE %lld  pid0 %lld\n",pidE,pid0);
	printf("blkuse: %llu\n",blkuse);


	int ret;
	int r = 0;

	time_t tim;
	time_t time_temp;
	struct tm *ptm;
	char ftime[96];	

	char mout[1024];
// ========================================================================
	/* Find first valid date/time for naming output files. */
	// Run until: zero SD block | last usable
	while ((r < blkuse) && (sw_firsttime == 0))
	{
		blk.blk = r++;
		ret = readablock(fd1, &blk);
		if (ret < 0)
   		printf("#### ERROR readablock ret: %d\n",ret);

		ret = packet_extract_msgs(&blkparse, &blk.data[0]);
		if (ret < 0)
		{
			printf("#### ERROR packet_extract_msgs ret: %d\n",ret);
			if (ret == -1) exit(-1);
		}
	
		/* Check if block contains a date/time msg. */
		ret = packet_extract_time(&blkparse, canid_time);
		// check if time msg canid found && time msg is not zero
		if (ret >= 0)

		for (i = 0; i < blkparse.n; i++)
		{
			if (blkparse.can[i].id == canid_time) // date/time msg?
			{ // Here, yes.
				if ((blkparse.can[i].dlc == 5) &&
					 ((blkparse.can[i].cd.uc[1] != 0) |
					  (blkparse.can[i].cd.uc[2] != 0) |
 					  (blkparse.can[i].cd.uc[3] != 0) |
					  (blkparse.can[i].cd.uc[4] != 0)) )
				{					
					sw_firsttime = 1;
				 	progressprint(&blkparse.can[blkparse.nt]);
  	   		 	tim = packet_extract_uint32_t(&blkparse.can[blkparse.nt], 1);
					ptm = localtime(&tim);
					strftime(&ftime[0],96,"%y%m%d_%H%M%S",ptm);
	   		 	printf("%4d 1st TIME: blk %d idx %d time: %s\n", r,blk.blk,blkparse.nt,ftime);
					break;		
				}
			}
		}
	}

	if (r >= blkmax)	// No date/time msgs found?
	{ // Here, yes.
		printf("#### ERROR No time msgs found\n");
		exit(-1);
	}

	/* Here: we found a beginning date/time */

	/* Setup output file path/name */
	char ctmp[96];
	char *path = "/home/deh/logcsa/";
	strcpy (ctmp, path);	// Path ***Make this a command line arg***
	strcat (ctmp,ftime);		// Add date/tme to path
	strcat (ctmp,"sdcan");	// Extension
	if ( (fpOut = fopen (ctmp,"w")) == NULL)
	{
		printf ("\nOutput file fpOut did not open: %s\n",ctmp); return -1;
	}
	printf("\nOutput file fpOut opened: %s\n",ctmp);

	strcat (ctmp,".tim");	// Extension
	if ( (fpOut1 = fopen (ctmp,"w")) == NULL)
	{
		printf ("\nOutput file fpOut1 did not open: %s\n",ctmp); return -1;
	}
	printf("\nOutput file fpOut1 opened: %s\n",ctmp);

/* ==== Convert SD to ascii =================================================== */

	// Rewind to beginning
	lseek64(fd1,0L,SEEK_SET);

	// Run until all zero SD block encountered, or end
	r = 0; sw_firsttime = 0;
	while (r < blkuse)
	{
		blk.blk = r++;
		ret = readablock(fd1, &blk);
		if (ret < 0)
   		printf("readablock ret: %d\n",ret);

		ret = packet_extract_msgs(&blkparse, &blk.data[0]);
		if (ret < 0)
		{
			printf("####ERROR packet_extract_msgs ret: %d\n",ret);
			if (ret == -1) break;
		}
		/* Convert CAN binary to gateway format ASCII/HEX */
		ret = packet_extract_format(mout, &blkparse);
//	printf("\nextract_format return: %d\n",ret);

		if (ret != strlen(mout))
   		printf("ARGH: ret and strlen not equal; %d %d\n",ret,(int)strlen(mout));

//	printf("%s",mout);
		fwrite(mout, sizeof(char),ret,fpOut); // Output file


		ret = packet_extract_time(&blkparse, canid_time);
		for (i = 0; i < blkparse.n; i++)
		{
			if (blkparse.can[i].id == canid_timesync)
			{
				if (blkparse.can[i].cd.uc[0] == 0)
				{
					time_work += 1;
				}
			}
			if (blkparse.can[i].id == canid_time)
			{
				time_temp = packet_extract_uint32_t(&blkparse.can[i],1);
				if (sw_firsttime == 0)
				{
					sw_firsttime = 1;
					time_init = time_temp;
					time_work = time_init + 1;
					ptm = localtime(&tim);
					strftime(&ftime[0],96,"%y%m%d_%H%M%S",ptm);
			    	printf("%4d TIME  1st: blk %d idx %d time: %s\n", r,blk.blk,i,ftime);
				}
				else
				{
					if (time_temp != time_work)
					{
						ptm = localtime(&time_work);
						strftime(&ftime[0],96,"%y%m%d_%H%M%S",ptm);
				    	printf("%6d TIME step: blk %6d idx %2u time_work: %s ", r,blk.blk,i,ftime);
						ptm = localtime(&time_temp);							
						strftime(&ftime[0],96,"%y%m%d_%H%M%S",ptm);
				    	printf("time_temp: %s %3d\n",ftime,(int)(time_temp - time_work));
						time_work = time_temp;
						fwrite(ftime, sizeof(char),strlen(ftime),fpOut1); // Output file
						o_blk = r;
						o_pos = o_blk << 9;	// Convert block number of offset (bytes)
						sprintf(ftime," %lld %d %d\n",(long long)o_pos, r, i);// offset,block,index in block
						fwrite(ftime, sizeof(char),strlen(ftime),fpOut1); // Output file
					}
				}
			}
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

