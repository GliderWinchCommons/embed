/******************************************************************************
* File Name          : cancnvtmatlab.c
* Date First Issued  : 05/08/2019
* Board              : Linux PC
* Description        : Convert gateway format CAN msgs to csv lines for matlab
*******************************************************************************/
/*
Hack of cancnvt 

cd ~/GliderWinchCommons/embed/svn_sensor/PC/cancnvtmatlab

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab 

./cancnvtmatlab csvlinelayout200321.txt csvfieldselect200321.txt < ~/GliderWinchItems/GEVCUr/docs/data/log200220-2.txt

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab csvlinelayout200321.txt csvfieldselect200321.txt < ~/GliderWinchItems/GEVCUr/docs/data/log200220-2.txt

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab csvlinelayout200321.txt csvfieldselect200321.txt < ~/GliderWinchItems/GEVCUr/docs/data/log200220-2.txt | tee log200220-2.csv

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab csvlinelayout200328.txt csvfieldselect200328.txt < ~/GliderWinchItems/GEVCUr/docs/data/log200328-1.txt | tee log200328.csv

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab csvlinelayout200329.txt csvfieldselect200329.txt < ~/GliderWinchItems/GEVCUr/docs/data/log200329-1.txt | tee log200329.csv

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab  -# csvlinelayout250328.txt csvfieldselect250327.txt < log250325.txt | tee cancnvtmatlab-250325-0000.csv


01/29/2025
To compile--
gcc -Wall cancnvtmatlab.c -o cancnvtmatlab

To run--
File Arguments. Two files--
'layout' defines how the payload fields are unpacked for each CAN ID
'select' selects the payloads to be included in the csv file

Input & output is std--
E.g.
Input from log file, output to console--
./cancnvtmatlab csvlinelayout250127-1915.txt csvfieldselect200329.txt < ../cancnvt/log250127-1915.txt

Input from log file, output to console AND file--
./cancnvtmatlab csvlinelayout250127-1915.txt csvfieldselect200329.txt < ../cancnvt/log250127-1915.txt | tee cancnvtmatlab-250127-1915.csv

Arguments: 
  Options <-option>
     t - print the tables constructed from the input files
     x - do not convert input data file
     c - csv position lines between csv data lines
     h - help
     # - Skip listing output lines for CAN ids not in table
     '-' with none of the above chars show help
	
  Files:
     File with definition of CAN fields
     File with selection of fields for csv line output

Data Input: 
  standard input - raw CAN msgs

Ouput: 
  [the following is conditonal based on Option switches]
  input tables constructed from input files
  standard output - csv lines

(All caps came from database.  Others are temporarily local.)
( rx = received from DMOC; tx = sent by GEVCU)
00400000      0X002          2      7717 U8 	 CANID_HB_TIMESYNC
05683004 0X00ad0600   11339264       967 -----------------  ...............
46400000      0X232        562      2775 --Speed tx-------  ...............
46600000      0X233        563      2775 --Torque tx------  ...............
46800000      0X234        564      2775 --unchanging-----  ...............
47400000      0X23A        570      4831 --Torque rx------  ...............
47600000      0X23B        571      4831 --Speed&status rx  ...............
47C00000      0X23E        574      4831 --DQ volt amps rx  ...............
CA000000      0X650       1616      4831 --HV status-rx     ...............
CA200000      0X651       1617        97 --Temperatures---	...............
E1000000      0X708       1800       120 UNIXTIME 	 CANID_HB_GPS_TIME_1
E1200000      0X709       1801        60 NONE 	 CANID_HB_GATEWAY2
E1800000      0X70C       1804        15 U8_U32 	 CANID_HB_LOGGER_1
E1C00000      0X70E       1806        29 LAT_LON_HT 	 CANID_HB_GPS_LLH_1
 Number of CAN IDs: 14

0x00400000  23      20
0xCA000000  99 :dlc 8: 0B 02 13 88 00 64 00 FC HVrx 0x0B02 0x1388     2818        0   281.8     0.0
0x47C00000  99 :dlc 8: 7E 1F 47 E2 01 81 F5 82 DQrx 0x7E1F  32287 0x47E2  18402 
0xCA200000  99 :dlc 8: 3D 3C 3D 05 3F 00 00 00 TMrx     21     20     21
0x47400000  99 :dlc 8: 75 30 0C 40 68 EF 05 76 TQrx      0 -26864  -3137    5
0x05683004  99 :dlc 8: 01 27 74 08 00 00 00 00 ??rx 0x0127    295 0x7408   -296 
0x47600000  99 :dlc 8: 52 A5 00 00 00 FC 35 9A SPrx 0x52A5   1157 0x35  3 ENABLE
0x00400000  23      21
0xCA000000  99 :dlc 8: 0B 02 13 88 00 64 00 FC HVrx 0x0B02 0x1388     2818        0   281.8     0.0
0x47C00000  99 :dlc 8: 7C 9F 87 DE 03 81 F7 C4 DQrx 0x7C9F  31903 0x87DE  34782 
0x47400000  99 :dlc 8: 75 30 0C 40 68 EF 07 74 TQrx      0 -26864  -3137    7
0x47600000  99 :dlc 8: 52 A0 00 00 00 FC 37 9D SPrx 0x52A0   1152 0x37  3 ENABLE
0x00400000  23      22
0x46400000  99 :dlc 8: 4E 20 00 00 00 01 92 CA SPtx 0x4E20 0
0x46600000  99 :dlc 8: 75 30 75 30 75 30 02 D9 ??tx 0x7530      0 0x7530      0 
0x46800000  99 :dlc 8: B9 8C 92 7C 00 3C 02 38 MWtx 0xB98C  70000 0x927C 150000 
0xCA000000  99 :dlc 8: 0B 04 13 80 00 64 00 FC HVrx 0x0B04 0x1380     2820       -8   282.0    -0.8
0x47C00000  99 :dlc 8: 7D 5E 57 DD FF 81 B9 77 DQrx 0x7D5E  32094 0x57DD  22493 
0x47400000  99 :dlc 8: 75 24 0C 40 68 EF 09 7E TQrx    -12 -26864  -3137    9
0x47600000  99 :dlc 8: 52 A0 00 00 00 FC 39 9B SPrx 0x52A0   1152 0x39  3 ENABLE
0x00400000  23      23
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

#define PRINTTABLES // Print the tables constructed from the input files
#define SKIPOUTPUTLINECT 0 // Skip outputting lines of converted for debugging purposes
#define PRINTCSVPOSITIONNUMBERS // Insert line above CSV line to identify CSV positions easily
#define JUSTPRINTTABLES // Exit after printing tables

/* Line buffer size */
#define LINESZ 512	// Longest CAN msg line length
char buf[LINESZ];

/* CAN id selection input file  */
#define NAMESZ 128	// Max size of name field
#define CSVLINESZ 2048	// Max output line size

struct CANFIELD
{
	unsigned int linenum; // Line number
	  signed int fldnum;  // Payload field number
	unsigned int paytype; // Payload type number
	double offset;        // Offset
  double scale;         // Scale
	double lgr;           // Last Good Reading
   char c[NAMESZ];      // Description field
};

/* CAN field ==> selection <== input list */
#define CSVSELECTSZ 1024
struct CSVSELECT
{
	unsigned int pos;	   // Position on csv line
	unsigned int canfld; // CAN field spec linenumber
	unsigned int layoutidx;  // Index in array (sorted on CAN id)
	unsigned int canfieldidx;// Index of field within array element
	struct CANFIELD* pcfld; // Pointer to payfield field in canfldlayout
	char f[NAMESZ];      // printf format for this field
	char c[NAMESZ];      // Description field
}csvselect[CSVSELECTSZ];
int csvselectsz = 0;    // Number of entries loaded

/* CAN field ==> layout <== list */
#define CANFLDLAYOUTSZ 512
#define MAXNUMFIELDS 32
struct CANFLDLAYOUT	
{
	unsigned int id;      // CAN id
	struct CANFIELD canfield[MAXNUMFIELDS];
}canfldlayout[CANFLDLAYOUTSZ];
int canfldlayoutsz = 0;  // Number of entries loaded

unsigned int canidtimetick = 0; // CAN id for GPS 64/sec time ticks

/* Identify time sync and date/time CAN msg CAN ids */
char tag;	// 'T' for date/time; 'S' for time sync msg

/* Line number used for locating problems */
unsigned int linect = 0;

#define FILECTSZ 4	// Max number of specification files
FILE* fpIn[FILECTSZ];
int filect = 0;

char convertpayloadBMS_MULTI(struct CANRCVBUF* p, struct CANFIELD* pfld);

char *paytype = "../../../svn_common/trunk/db/PAYLOAD_TYPE_INSERT.sql";
char *canid_insert = "../../../svn_common/trunk/db/CANID_INSERT.sql";

/* Command line switches */
char argflag = 0;   // Count number of command line switches
char cmdsw_t;
char cmdsw_x;
char cmdsw_c;
char cmdsw_b;
char * helplist = {"\nCommand line layout\n\
./cancnvtmatlab <options> <path/file input file #1> <path/file input file #2> <path/file input file ...> e.g.\n\
./cancnvtmatlab -t csvlinelayout.txt csvfieldselect.txt < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt\n\
  options\n\
 h - help\n\
 t - print input tables\n\
 x - do not convert input data file\n\
 c - csv position lines between csv data lines\n\
 # - skip printing missing CAN ids\n"};

/* Declarations */
void    cmd_f_do_msg(struct CANFIELD* pfld, struct CANRCVBUF* p, int k);
uint8_t DMOCchecksum(uint32_t id, uint8_t* puc, uint8_t dlc);
int     DMOCstate(char* pc, uint8_t status);
void convertpayload(struct CANRCVBUF* play, struct CANFIELD* pfld);

/*******************************************************************************
 * static int cmpfunc (const void * a, const void * b);
 * @brief 	: Compare function for qsort and bsearch (see man pages)--using direct access
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfunc (const void * a, const void * b)
{
	const struct CANFLDLAYOUT *A = (struct CANFLDLAYOUT *)a;
	const struct CANFLDLAYOUT *B = (struct CANFLDLAYOUT *)b;
	long long aa = (unsigned int)A->id;
	long long bb = (unsigned int)B->id;

	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}
/*******************************************************************************
 * static char* copydesc(char *po, char *pi);
 * @brief 	: Copy field between two double quotes
 * @return	: pointer to end of second quotes; NULL = error
*******************************************************************************/
static char* copydesc(char *po, char *pi)
{
	char* p1 = strchr(pi,'"');
	if (p1 == NULL) return NULL;
	p1++;
	char* p2 = strchr(p1,'"');
	if (p2 == NULL) return NULL;
	if ((p2-p1+1) >= NAMESZ) return NULL;
	strncpy(po,p1,(p2-p1));
	*(po+(p2-p1)) = '\0';
	return (p2+1);
}

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k;
	int mm = 0;
	char* pv;
	uint8_t errflag = 0;

	/* =============== Check command line arguments ===================================== */
	if (argc < 2)
	{
		printf("\nError: at least one argument with a file path/name required.  argc = %d\n",argc);
		return -1;
	}

	for (i = 1; i < argc; i++)
	{
		if (**(argv+i) == '-')
		{ // Here command line switch argument
			pv = *(argv+i);
			j = 1;
			while ( *(pv+j) != 0)
			{
				switch ( *(pv+j) )
				{
				case 'h':
				case 'H':
					printf("%s",helplist);
					return 0;
				case 'T':
				case 't': cmdsw_t = 1; break;// t - print the tables constructed from the input files
				case 'X':
				case 'x': cmdsw_x = 1; break;// x - do not convert input data file
				case 'C':
				case 'c': cmdsw_c = 1; break;// c - csv position lines between csv data lines
				case '#': cmdsw_b = 1; break;// # - skip listing lines with missing CAN ids
				default:
					printf("\nCommand line switch %c is not in list\n%s",*(pv+j), helplist);
					break;
				}
				argflag += 1;
				j += 1;
			}
		}
	}

	if ((argc - argflag) >= FILECTSZ)
	{
		printf("\nMore arguments than input files allowed: argc = %d, command sw ct = %d, file ct allowed = %d\n",argc, argflag, FILECTSZ);
		return -1;
	}
//printf("cmdsw: t %i  x %i  c %i #%i\n",cmdsw_t,cmdsw_x,cmdsw_c,cmdsw_b);	
//printf("argc: %i\n",argc);

	/* =============== Read input specification files ================================================ */
	int ii;
	for (ii = 1; ii < argc; ii++)
	{
		if (**(argv+ii) == '-') continue;


		if ( (fpIn[ii] = fopen (argv[ii],"r")) == NULL)
		{
			printf ("\nInput file did not open: %s\n",argv[ii]); 
			return -1;
		}
int n = 0;
mm = 0;
	int flag = 0;
	unsigned int linenum;
	unsigned int id;
	  signed int fldnum;  // Payload field number
	unsigned int paytype; // Payload type number
	double offset;        // Offset
   double scale;         // Scale
   char c[NAMESZ];       // Description field
	char* px;
	

		/* Load data from this file, ignoring comment lines. */
		while ( (fgets (&buf[0],LINESZ,fpIn[ii])) != NULL)	// Get a line
		{
mm += 1;
			switch(buf[0])
			{
			case 'T': // CAN field layout spec	
				sscanf(&buf[1],"%i %x %i %i %lf %lf", 
					&linenum,
					&id,
					&fldnum,
					&paytype,
					&offset,
					&scale);
				px = copydesc(&c[0],&buf[1]); // Extract description 

				if (px == NULL) {printf("format field extract err c: %s\n",buf); return -1;}

				flag = 0;
				for (i = 0; i < canfldlayoutsz; i++)
				{
					if (canfldlayout[i].id == id)
					{
						flag = 1;
						break;
					}
				}

				canfldlayout[i].id = id;

				/* Copy parameters into payload field */
				if (fldnum > MAXNUMFIELDS) fldnum = MAXNUMFIELDS-1; // JIC
				if (fldnum == 0) fldnum = 1; // Field numbers run 1 - n
				canfldlayout[i].canfield[fldnum-1].fldnum  = fldnum;// Redundant
				canfldlayout[i].canfield[fldnum-1].linenum = linenum;
				canfldlayout[i].canfield[fldnum-1].paytype = paytype;
				canfldlayout[i].canfield[fldnum-1].offset  = offset;
				canfldlayout[i].canfield[fldnum-1].scale   = scale;
				canfldlayout[i].canfield[fldnum-1].lgr     = 0;
				strcpy(&canfldlayout[i].canfield[fldnum-1].c[0],&c[0]);

				if (flag == 0)
				{ // Here CAN id is not in array, so add it
					canfldlayoutsz += 1; // Advance index in table
					if (canfldlayoutsz >= CANFLDLAYOUTSZ)
					{
						canfldlayoutsz -= 1;
						printf("canfldlayout array overflow: %s",buf);
					}	
					canfldlayout[canfldlayoutsz].id =	mm;
				}
				break;

			case 'S': // CSV field selection
				sscanf(&buf[1],"%i %i", &csvselect[csvselectsz].pos,&csvselect[csvselectsz].canfld);
				px = copydesc(&csvselect[csvselectsz].f[0],&buf[1]); // Extract description 
				if (px == NULL) {printf("format field extact err f: %s\n",buf); return -1;}
				px = copydesc(&csvselect[csvselectsz].c[0],px); // Extract description 
				if (px == NULL) {printf("format field extact err c: %s\n",buf); return -1;}

				/* This was a quick way to make the order of lines the csv position, since the
					old method used the input file number and was cumbersome to make changes in the file. */
				csvselect[csvselectsz].pos = csvselectsz;
				csvselectsz += 1;
				if (csvselectsz >= CSVSELECTSZ)
				{
					csvselectsz -= 1;
					printf("csvselect array overflow: %s",buf);
				}
				break;
	
			case '#':
//				printf("COMMENT: %i %s",n, buf);
				break;

			default:
//				printf("NOT DATA: %s",buf);
				break;
			}
n += 1;
		}
		filect += 1;
	}


	if (cmdsw_t != 0) // Print input tables?
	{
		printf("\n#CAN payload layout in INPUT FILE order: %i %i \n",csvselectsz,mm);
		for (i = 0; i < canfldlayoutsz; i++)
		{
			printf("%3i 0x%08X\n",i,canfldlayout[i].id);
			for (j = 0; j < MAXNUMFIELDS; j++)
			{
				printf("\t%3i %3i %3i %3i %f %f",
						(j+1), 
						canfldlayout[i].canfield[j].linenum,
						canfldlayout[i].canfield[j].fldnum,
						canfldlayout[i].canfield[j].paytype,
						canfldlayout[i].canfield[j].offset,
						canfldlayout[i].canfield[j].scale);
				printf("# %s\n",canfldlayout[i].canfield[j].c);
			}	
		}
	}
	
	/* Save this time CAN id jic sorting moves it away from [0]. */
	canidtimetick = canfldlayout[0].id;


	/* Sort canfldlayout array for later bsearch location of CAN id. */
	qsort(&canfldlayout[0], canfldlayoutsz, sizeof(struct CANFLDLAYOUT), cmpfunc);// Sort for bsearch'ing

	if (cmdsw_t != 0) // Print input tables?
	{
		printf("\n#CAN payload layout in CAN id SORTED order:\n");
		for (i = 0; i < canfldlayoutsz; i++)
		{
			printf("%3i 0x%08X\n",i,canfldlayout[i].id);
			for (j = 0; j < MAXNUMFIELDS; j++)
			{
				printf("\t%3i %3i %3i %3i %f %f",
						(j+1), 
						canfldlayout[i].canfield[j].linenum,
						canfldlayout[i].canfield[j].fldnum,
						canfldlayout[i].canfield[j].paytype,
						canfldlayout[i].canfield[j].offset,
						canfldlayout[i].canfield[j].scale);
				printf(" %s\n",canfldlayout[i].canfield[j].c);
			}
		}
	}

	if (cmdsw_t != 0) // Print input tables?
	{
		printf("\n#CSV field selection\n");
		for (i = 0; i < csvselectsz; i++)
		{
			printf("#%3i %3i %3i",i+1,csvselect[i].pos,csvselect[i].canfld);
			printf("  %s"  ,csvselect[i].f);
			printf("\t\t%s\n",csvselect[i].c);
		}
		exit(0);
	}
//return 0;
/* ============= Make list of CAN id versus field definition number ================== */
/*
struct CSVSELECT
{
	unsigned int pos;	   // Position on csv line
	unsigned int canfld; // CAN field spec linenumber
	unsigned int layoutidx;  // Index in array (sorted on CAN id)
	unsigned int canfieldidx;// Index of field within array element
	struct CANFIELD* pcfld; // Pointer to payfield field in canfldlayout
	char f[NAMESZ];      // printf format for this field
	char c[NAMESZ];      // Description field
}csvselect[CSVSELECTSZ];
*/
/*
struct CANFLDLAYOUTFLDNUM
{
	unsigned int fldnum;     // CAN field identification number
	unsigned int layoutidx;  // Index in array (sorted on CAN id)
	unsigned int canfieldidx;// Index of field within array element
}; 
*/

unsigned int itmp;
unsigned int kflag;
	for (i = 0; i < csvselectsz; i++)
	{
		itmp = csvselect[i].canfld; // CAN field identification number
		kflag = 0;
		// Search field definition array for this csv field number
		for (j = 0; j < canfldlayoutsz; j++)
		{
			for (k = 0; k < MAXNUMFIELDS; k++) // Scan fields
			{
				if (canfldlayout[j].canfield[k].linenum == itmp)
				{
					csvselect[i].layoutidx   = j;
					csvselect[i].canfieldidx = k;
					csvselect[i].pcfld = &canfldlayout[j].canfield[k];
					kflag += 1;
				}
			}
		}
		if (kflag == 0)
		{
			printf("Error: CSV field not in definition list: %i %i %i\n",i,j,k);
		}
		if (kflag > 1)
		{
			printf("Error: CSV field duplication: %i %i %i %i\n",i,j,k,kflag);
		}
	}
	if (cmdsw_t != 0) // Print input tables?
	{
		printf("\n#CSV field SELECT loaded\n");
		printf("\
#Column 1: canselect array number\n\
#Column 2: CSV line field position number\n\
#Column 3: Field definition: CAN field number\n\
#Column 4: Index in sorted field definition array\n\
#Column 5: Index of payload field within field definition\n\
#Column 6: Format field for printf of this field\n\
#Column 7: Description of field\n");
		for (i = 0; i < csvselectsz; i++)
		{
			printf("%3i %3i %3i %3i %3i",(i+1),csvselect[i].pos,csvselect[i].canfld,csvselect[i].layoutidx,csvselect[i].canfieldidx);
			printf("  %s"  ,csvselect[i].f);
			printf("\t\t%s\n",csvselect[i].c);
		}
	}

	if (cmdsw_x != 0) // Do not convert input data?
		return 0;

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

		// ==== CAN msg has been converted from asci to binary =============== 
			/* Each time tick generates a csv line. */
//			if (canidtimetick == can.id)
			skip += 1;
			if (canidtimetick == can.id)
      { // Here, CAN msg is a time tick msg
				// Generate CSV line using last-good-reading
				// Build CSV line
  				skip = 0; // Debugging skip count to throttle output
				pcline = &cline[0];
				for (i = 0; i < csvselectsz; i++)
				{
					pfmt  = &csvselect[i].f[0];         // Point to format 
					j = csvselect[i].layoutidx;
					k = csvselect[i].canfieldidx;
					dtmp = canfldlayout[j].canfield[k].lgr;

#if 0
if (canfldlayout[i].id == 0xB2000000)
{
	printf("i %2d, j %2d, k %2d, %0.1f ",i,j,k,dtmp);
	for (int kk=0; kk < 32; kk++)
	{
			printf("%08X %d %0.0f ",canfldlayout[i].id,kk,canfldlayout[i].canfield[kk].lgr);
	}
	printf("\n");
}
#endif
					nn = sprintf(pcline,pfmt,dtmp); // Convert binary to formatted ascii
					pcline += nn;       // Add number chars generated
					strcpy(pcline,","); // Add comma separator
					pcline += 1;        // Point to next position in output 
				}
				*(pcline-1) = '\n';
				printf("%s",cline);

				// Line numbers above CSV entries for debugging aid
				if (cmdsw_c != 0)
				{
					pcline = &cline[0];
					char* pcline2 = &cline[0];
					int diff,g;
					for (i = 0; i < csvselectsz-1; i++)
					{
						nn=printf("%2i",i+1);
					   pcline=strchr(pcline,',');
						pcline++;
						diff = (pcline-pcline2)-nn;
						pcline2=pcline;
						if (diff<0)diff = 0;
						for (g = 0; g < diff; g++)
							printf(" ");
					}
					printf(" %2i\n",i+1);
				}
			}
			/* Update each Last Good Reading for each of the payload fields.
			   Update Last-Good-Reading for each payload field in this CAN msg */
			// Find struct with data layout for this CAN msg
			pcanfldlayout = bsearch(&can.id, &canfldlayout[0], canfldlayoutsz, sizeof(struct CANFLDLAYOUT), cmpfunc);
			if (pcanfldlayout != NULL)
			{ // Found in CAN layout array
				if (pcanfldlayout->canfield[0].paytype == BMS_MULTI)
				{
					convertpayloadBMS_MULTI(&can, &pcanfldlayout->canfield[0]);
				}
				else
				{
					for (k = 0; k < MAXNUMFIELDS; k++)
					{
						if (pcanfldlayout->canfield[k].fldnum != 0)
						{ // Convert & calibrate payload, and update last good reading (lgr)
							convertpayload(&can, &pcanfldlayout->canfield[k]);						
						}
					}
				}
			}
			else
			{ // Here, Not found: this CAN id does not have a payload layout entry in the array
				if (cmdsw_b == 0)	// Skip this line command line switch is off
					printf("# Not in payload layout array: 0X%08X\n", can.id);
				else
					errflag = 1;
			}
		}		
	}
	if (errflag != 0)
	{
		printf("\n# Not in payload layout array error, one or more times\n");
	}
}

/* ******************************************************************************** 
 * uintYXX payuYXX(struct CANRCVBUF* pcanx, int offset)
 * @brief	: Combine bytes to make an unsigned int: Y = U or I, XX = 32 or 16
 * @param	: pcanx = pointer to struct with payload
 * @param	: offset = number of bytes to offset for int
 * @return	: uint32_t, or uint16_t
 **********************************************************************************/
// Little endian
uint32_t payU32(struct CANRCVBUF* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->cd.uc[0+offset] <<  0;
		x |= pcanx->cd.uc[1+offset] <<  8;
		x |= pcanx->cd.uc[2+offset] << 16;
		x |= pcanx->cd.uc[3+offset] << 24;
		return x;
}
uint16_t payU16(struct CANRCVBUF* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->cd.uc[0+offset] <<  0;
		x |= pcanx->cd.uc[1+offset] <<  8;
		return x;
}
// Big endian
uint32_t payI32(struct CANRCVBUF* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->cd.uc[3+offset] <<  0;
		x |= pcanx->cd.uc[2+offset] <<  8;
		x |= pcanx->cd.uc[1+offset] << 16;
		x |= pcanx->cd.uc[0+offset] << 24;
		return x;
}
uint16_t payI16(struct CANRCVBUF* pcanx, int offset)
{
	uint16_t x;
		x  = pcanx->cd.uc[1+offset] <<  0;
		x |= pcanx->cd.uc[0+offset] <<  8;
		return x;
}
int16_t payY16(struct CANRCVBUF* pcanx, int offset)
{
	int16_t x;
		x  = pcanx->cd.uc[1+offset] <<  0;
		x |= pcanx->cd.uc[0+offset] <<  8;
		return x;
}

float payFF(struct CANRCVBUF* pcanx, int offset)
{
	union{uint32_t ui; float ff;} ui_ff;

		ui_ff.ui  = pcanx->cd.uc[0+offset] <<  0;
		ui_ff.ui |= pcanx->cd.uc[1+offset] <<  8;
		ui_ff.ui |= pcanx->cd.uc[2+offset] << 16;
		ui_ff.ui |= pcanx->cd.uc[3+offset] << 24;
		return ui_ff.ff;
}

/* ******************************************************************************** 
 * void convertpayload(struct CANRCVBUF* play, struct CANFIELD* pfld)
 * @brief	: Convert payload bytes and update (double)last-good-reading for payload field
 * @param	: play = pointer to CANFLDLAYOUT for CAN msg received 
 * @param	: pfld = pointer to CAN payload field array for this CAN unit
 **********************************************************************************/
/*
struct CANFIELD
{
	unsigned int linenum; // Line number
	unsigned int fldnum;  // Payload field number
	unsigned int paytype; // Payload type number
	double offset;        // Offset
   double scale;         // Scale
	double lgr;           // Last Good Reading
   char c[NAMESZ];       // Description field
};

For convenience, the following from PAYLOAD_TYPE_INSERT.sql--

('NONE',      0,  0, ' No payload bytes');
('FF',        1,  4, ' [0]-[3]: Full Float');			--
('FF_FF',     2,  8, ' [0]-[3]: Full Float[0]; [4]-[7]: Full Float[1]');	--
('U32',		 3,  4, ' [0]-[3]: uint32_t');				--
('U32_U32',	 4,  8, ' [0]-[3]: uint32_t[0]; [4]-[7]: uint32_t[1]');	--
('U8_U32',	 5,  5, ' [0]: uint8_t; [1]-[4]: uint32_t');		--
('S32',		 6,  4, ' [0]-[3]: int32_t');				--
('S32_S32',	 7,  8, ' [0]-[3]: int32_t[0]; [4]-[7]: int32_t[1]');	--
('U8_S32',	 8,  5, ' [0]: int8_t; [4]-[7]: int32_t');		--
('HF',        9,  2, ' [0]-[1]: Half-Float');			--
('F34F',     10,  3, ' [0]-[2]: 3/4-Float');				--
('xFF',      11,  5, ' [0]:[1]-[4]: Full-Float, first   byte  skipped');	--
('xxFF',     12,  6, ' [0]:[1]:[2]-[5]: Full-Float, first 2 bytes skipped');	--
('xxU32',    13,  6, ' [0]:[1]:[2]-[5]: uint32_t, first 2 bytes skipped');	--
('xxS32',    14,  6, ' [0]:[1]:[2]-[5]: int32_t, first 2 bytes skipped');	--
('U8_U8_U32',15,  6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1],uint32_t,');	--
('U8_U8_S32',16,  6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], int32_t,');	--
('U8_U8_FF',	17,  6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], Full Float,');--
('U16',      18,  2, ' [0]-[1]:uint16_t');			--
('S16',      19,  2, ' [0]-[1]: int16_t');			--
('LAT_LON_HT',20, 6, ' [0]:[1]:[2]-[5]: Fix type, bits fields, lat/lon/ht');	--
('U8_FF',    21,  5, ' [0]:[1]-[4]: uint8_t, Full Float');	--
('U8_HF',    22,  3, ' [0]:[1]-[2]: uint8_t, Half Float');	--
('U8',       23,  1, ' [0]: uint8_t');	--
('UNIXTIME', 24,  5, ' [0]: U8_U32 with U8 bit field stuff');	--
('U8_U8',    25,  2, ' [0]:[1]: uint8_t[0],uint8[1]');	--
('U8_U8_U8_U32',26,7, ' [0]:[1]:[2]:[3]-[5]: uint8_t[0],uint8_t[0],uint8_t[1], int32_t,');	--
('I16_I16',	      27, 4,'[1]-[0]: uint16_t[0]; [3]-[2]: uint16_t[1]');
('I16_I16_X6',     28, 4,'[1]-[0]: uint16_t[0]; [3]-[2]: uint16_t[1]; X');
('U8_U8_U8',       29, 6,'[1]-[2]:[2] uint8_t');--
('I16_X6',         30, 7,'[1]-[0]: uint16_t,[6]: uint8_t');
('I16_I16_I16_I16',31, 8,'[1]-[0]:[3]-[2]:[5]-[4]:[7]-[6]:uint16_t');
('I16__I16',       32, 8,'[1]-[0]:uint16_t,[6]-[5]:uint16_t');
('I16_I16_I16_X7', 33, 8,'[1]-[0]:[3]-[2]:[5]-[4]:uint16_t,[6]:uint8_t');
('I16_I16_X_U8_U8',34, 8,'[1]-[0]:[3]-[2]:uint16_t,[5]:[6]:uint8_t');
('I16',            35, 2,'[1]-[0]:uint16_t');	--
('U8_VAR',         36, 2,'[0]-uint8_t: [1]-[n]: variable dependent on first byte');	--
('U8_S8_S8_S8_S8', 37, 5,'[0]:uint8_t:[1]:[2]:[3]:[4]:int8_t (signed)');	--
('Y16_Y16_Y16_Y16',38,8,'[1]-[0]:[3]-[2]:[5]-[4]:[7]-[6]:int16_t');
('U8_U8_U8_U8_FF' ,39,8,'[0]:[1]:[2]:[3]:uint8_t,[4-7]:Full-Float');
('U8_U8_U8_S8_U16',40,6,'[0]:[1]:[2]:uint8_t,[3]:int_8,[[4]-[5]:uint16_t');
('U8_8'           ,41,8,'[0]-[7]: unit8_t[8]');
('S8_U8_7'        ,42,8,'[0]:int8_t,unit8_t[7]');
('S8_S16_FF_V'    ,43,7,'[0]:int8_t,uint16_t, added FF if DLC = 7');
('U8_U8_U8_X4'    ,44,7,'[0]:uint8_t:drum bits,uint8_t:command,uin8_t:subcmd,X4:four byte value');
('S8_U8'          ,45,8,'[0]:int8_t,unit8_t');
('U8_U8_U16_U16_U16'  ,46,7,'[0]:uint8_t,[1]:uint8_t,[2:3],[4:5],[6:7]:uint16_t');
('U16_U16_U16_U16'    ,47,7,'[0:1],[2:3],[4:5],[6:7]:uint16_t');
('I16_I16_U8_U8_U8_U8',48,8,'[1]-[0]:[3]-[2]:Big E,[4-7]uint8_t');
('U8_U8_U16_U16_S16'  ,49,7,'[0]:uint8_t,[1]:uint8_t,[2:3]:uint16_t,[4:5]:uint16_t,[6:7]:int16_t');
('FF_S32'             ,50,8,'[0]-[3]: float; [4]-[7]: int32_t[1]');	--


('LVL2B',	249,  6, ' [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Board code),[2]-[5]see table');	--
('LVL2R',	250,  6, ' [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Readings code),[2]-[5]see table');	--
('UNDEF',	255,  8, ' Undefined');			--

*/
	union
	{
		unsigned int ui;
		signed int si;
		uint16_t us[2]; // Unsigned 16b
		 int16_t ss[2]; // Signed 16b
		float ff;
	}ui_ff;

void convertpayload(struct CANRCVBUF* pcanx, struct CANFIELD* pfld)
{
//	if (pfld->paytype == 51)
//printf("PAY: canid %08X pfld->paytype %d\n",pcanx->id, pfld->paytype);
	
	uint8_t k = pfld->fldnum-1; // k is index to payload field within 8 byte payload array

	/* Union used to convert Little Endian payload bytes to float, ints, etc. */
	union ui_ff;
	ui_ff.ui = 0;

	int16_t y16x;
	int16_t y16off;

	/* Conversion to double after byte extraction. */
	// flag: Most are unsigned, so default to 0
	 // 0  unsigned
	 // 1  signed
	 // 2  float
	 // 3  16 rollover
	 // 4  16b signed
	char flag = 0;

	switch(pfld->paytype)
	{ 
	case BMS_MULTI: 
printf("\n\nconvertpayload case BMS_MULTI: SHOULD NOT HAVE COME HERE! %08X\n\n",pcanx->id);
return;

	case I16:
	case I16_I16:
	case I16_I16_I16_I16:
		ui_ff.ui = payI16(pcanx,k*2);
		break;

	case I16_I16_U8_U8_U8_U8: // ELCON format
		if (k == 0) {ui_ff.ui = payI16(pcanx,0); break;}
		if (k == 1) {ui_ff.ui = payI16(pcanx,2); break;}
		if (k >= 2) {ui_ff.ui = pcanx->cd.uc[k+2]; break;}
		break;

	case I16_X6:
		if (k == 0) {ui_ff.ui = payI16(pcanx,0); break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[5]; break;}
		break;

	case I16_I16_X6:
		if (k != 2) {ui_ff.ui = payI16(pcanx,k*2); break;}
		if (k == 2) {ui_ff.ui = pcanx->cd.uc[5];   break;}
		break;

	case I16__I16: // Not sure what this is about!
		if (k == 1) k = 2;
	case I16_I16_I16_X6:
		if (k != 3) {ui_ff.ui = payI16(pcanx,k*2);break;}
		if (k == 3) {ui_ff.ui = pcanx->cd.uc[6];  break;}
		break;

	case I16_I16_X_U8_U8:
		if (k < 2)  {ui_ff.ui = payI16(pcanx,k*2);break;}
		if (k > 2)  {ui_ff.ui = pcanx->cd.uc[k];  break;}
		break;

	case Y16_Y16_Y16_Y16:
		y16x = payY16(pcanx,k*2);
		flag = 3;
		break;

	case NONE:
		break;

	case U8:
	case U8_U8:
	case U8_U8_U8:
		ui_ff.ui = pcanx->cd.uc[k];
		break;

	case S8_U8:
		if (k == 0) {ui_ff.si = pcanx->cd.uc[0]; break;}
		if (k == 1) {flag = 1; ui_ff.ui = pcanx->cd.uc[1];}
		break;

	case U32:
	case U32_U32:
		ui_ff.ui = payU32(pcanx,k*4);
		break;

	case xxU32:
		k = 1;
	case UNIXTIME:
	case U8_U32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = payU32(pcanx,1); break;}
		break;

	case U8_U8_U32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[1]; break;}
		if (k == 2) {ui_ff.ui = payU32(pcanx,2); break;}
		break;
		
	case U8_U8_S32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[1]; break;}
		if (k == 2) 
		{
			ui_ff.ui = payU32(pcanx,2); 
			flag = 1;
		}
		break;

	case FF_S32:
		if (k == 0) {flag = 2; ui_ff.ff = payFF(pcanx,0); break;}
		if (k == 1) {flag = 1; ui_ff.si = payU32(pcanx,4);}
		break;

  case U8_U8_U16_U16_S16:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[1]; break;}
		if (k == 2) {ui_ff.ss[0] = payU16(pcanx,2); break;}
		if (k == 3) {ui_ff.ss[0] = payU16(pcanx,4); break;}
	  if (k == 4) {flag = 4; ui_ff.ss[0] = payU16(pcanx,6);}
		break;

	case xxS32:
		k = 1;
	case S32:
	case S32_S32:
		flag = 1; ui_ff.ui = payU32(pcanx,k*4);
		break;

	case U8_S32:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {flag = 1; ui_ff.ui = payU32(pcanx,1);}
		break;

	case FF:
	case FF_FF:
		flag = 2;
		ui_ff.ff = payFF(pcanx,k*4);
		break;

	case xxFF:
		k = 1;
	case U8_FF:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0]; break;}
		if (k == 1) {flag = 2;	ui_ff.ff = payFF(pcanx,1);}
		break;

	case U8_U8_FF:
		if (k == 0) {ui_ff.ui = pcanx->cd.uc[0];    break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[1];    break;}
		if (k == 2) {flag = 2; ui_ff.ff = payFF(pcanx,2); }
		break;		

	case LAT_LON_HT:
		cmd_f_do_msg(pfld, pcanx, k);
		break;		
		
	default:
		printf("\nPAYLOAD NOT PROGRAMMED!: id: 0X%08X paycode: %i\n",pcanx->id, pfld->paytype);
		break;
	}
#ifdef USEOLDCODE
	/* Convert to double */
	if      (flag == 1)
		pfld->lgr = ui_ff.si; // Signed -> double
	else if (flag == 0)
		pfld->lgr = ui_ff.ui; // Unsigned -> double
	else if (flag == 2)
		pfld->lgr = ui_ff.ff; // float -> double	
#else

	/* Calibrate and update last-good-reading as double */
	pfld->lgr = (pfld->lgr + pfld->offset) * pfld->scale;

	switch(flag)
	{
	case 0: // Unsigned -> double
		pfld->lgr = ui_ff.ui; // Unsigned -> double
		pfld->lgr = (pfld->lgr + pfld->offset) * pfld->scale;
		break;
	
	case 1:  // Signed -> double
		pfld->lgr = ui_ff.si;
		pfld->lgr = (pfld->lgr + pfld->offset) * pfld->scale;
		break;

	case 2: // Float
		pfld->lgr = ui_ff.ff; // float -> double
		pfld->lgr = (pfld->lgr + pfld->offset) * pfld->scale;
		break;

	case 3: // 16b w rollover
		y16off = pfld->offset;    // Convert double -> 16b signed
		y16x   = (y16x + y16off); // Apply offset (signed)
		pfld->lgr = y16x;         // Convert 16b to double
		pfld->lgr = pfld->lgr * pfld->scale; // Apply scaling
		break;

	case 4: // 16b signed		
		pfld->lgr = ui_ff.ss[0];
		pfld->lgr = (pfld->lgr + pfld->offset) * pfld->scale;
		break;
	}
#endif
	return;
}	

/* ********** Following lifted from cangate cmd_f *********************** */
/* Extract 4 byte unsigned integer from CAN payload */
unsigned int extract(struct CANRCVBUF* p)
{
	union INTB
	{
		unsigned int ui;
		unsigned char uc[4];
	}intb;

	intb.uc[0] = p->cd.uc[2+0];
	intb.uc[1] = p->cd.uc[2+1];
	intb.uc[2] = p->cd.uc[2+2];
	intb.uc[3] = p->cd.uc[2+3];
	return intb.ui;
}
struct GPSFIX // Lifted from ../svn_sensor/sensor/co1_sensor_ublox/trunk/p1_gps_time_convert.h
{
	int	lat;	// Latitude  (+/-  540000000 (minutes * 100,000) minus = S, plus = N)
	int	lon;	// Longitude (+/- 1080000000 (minutes * 100,000) minus = E, plus = W)
	int	ht;	// Meters (+/- meters * 10)
	unsigned char fix;	// Fix type 0 = NF, 1 = G1, 2 = G3, 3 = G3
	unsigned char nsats;	// Number of satellites
};
static struct GPSFIX gfx;

void cmd_f_do_msg(struct CANFIELD* pfld, struct CANRCVBUF* p, int k)
/*
k = field index number
  0 = fix code
  1 = number sats
  2 = lat
  3 = lon
  4 = ht
*/
{
	unsigned int code;
	int type;

	/* Extract bit fields from first two bytes of payload */
	gfx.fix = p->cd.uc[0] & 0xf; 	// Fix type 0 = NF, 1 = G1, 2 = G3, 3 = G3
	code = p->cd.uc[0] >> 4;		// Conversion resulting from parsing of the line
	type = p->cd.uc[1] & 0x7;		// Type: lat/lon/ht->0/1/2
	gfx.nsats = p->cd.uc[1] >> 3;	// Number of satellites

	switch(type)
	{
	case 0: // Latitude
		gfx.lat = extract(p);
		break;
	case 1: // Longitude
		gfx.lon = extract(p);
		break;
	case 2: // Height
		gfx.ht = extract(p);
		break;

	default:
		printf ("BAD LAT/LON/HT TYPE CODE: %d code: %d\n",type, code);
		return;
	}
//	dlat = gfx.lat; dlon = gfx.lon; dht = gfx.ht;
//	sprintf(po,"%2d %2d %10.5f  %10.5f  %8.3f",gfx.fix, gfx.nsats,dlat/60.0E5, dlon/60.0E5, dht/1E3);

	/* Update last-good-reading.  scaling takes place later. */
	if      (k == 0) pfld->lgr = gfx.fix;
	else if (k == 1) pfld->lgr = gfx.nsats;
	else if (k == 2) pfld->lgr = gfx.lat;
	else if (k == 3) pfld->lgr = gfx.lon;
	else if (k == 4) pfld->lgr = gfx.ht;
	return;
}
/* ******************************************************************************** 
 * char convertpayloadBMS_MULTI(struct CANRCVBUF* pcanx, struct CANFIELD* pfld);
 * @brief	: Convert payload for multi-format layout
 * @param	: pcanx = pointer to CAN msg
 * @param : pfld = pointer to struct with fields
 * @return: flag setting
 **********************************************************************************/
/* See GliderWinchItems/BMS/bmsadbms1818/Ourtasks/cancomm_items.h */
 #define MISCQ_HEARTBEAT   0   // reserved for heartbeat
 #define MISCQ_STATUS      1 // status
 #define MISCQ_CELLV_CAL   2 // cell voltage: calibrated
 #define MISCQ_CELLV_ADC   3 // cell voltage: adc counts
 #define MISCQ_TEMP_CAL    4 // temperature sensor: calibrated
 #define MISCQ_TEMP_ADC    5 // temperature sensor: adc counts for making calibration
 #define MISCQ_DCDC_V      6 // isolated dc-dc converter output voltage
 #define MISCQ_CHGR_V      7 // charger hv voltage
 #define MISCQ_HALL_CAL    8 // Hall sensor: calibrated
 #define MISCQ_HALL_ADC    9 // Hall sensor: adc counts for making calibration
 #define MISCQ_CELLV_HI   10 // Highest cell voltage
 #define MISCQ_CELLV_LO   11 // Lowest cell voltage
 #define MISCQ_FETBALBITS 12 // Read FET on|off discharge bits
 #define MISCQ_SET_DUMP	  13 // Set ON|OFF DUMP FET on|off
 #define MISCQ_SET_DUMP2  14 // Set ON|OFF DUMP2 FET FET: on|off
 #define MISCQ_SET_HEATER 15 // Set ON|OFF HEATER FET on|off
 #define MISCQ_TRICKL_OFF 17 // Turn trickle charger off for no more than ‘payload [3]’ secs
 #define MISCQ_TOPOFSTACK 18 // BMS top-of-stack voltage
 #define MISCQ_PROC_CAL   19 // Processor ADC calibrated readings
 #define MISCQ_PROC_ADC   20 // Processor ADC raw adc counts for making calibrations
 #define MISCQ_R_BITS     21 // Dump, dump2, heater, discharge bits
 #define MISCQ_CURRENT_CAL 24 // Below cell #1 minus, current resistor: calibrated
 #define MISCQ_CURRENT_ADC 25 // Below cell #1 minus, current resistor: adc counts
 #define MISCQ_UNIMPLIMENT 26 // Command requested is not implemented
 #define MISCQ_SET_FETBITS  27 // Set FET on/off discharge bits
 #define MISCQ_SET_DCHGTST  28 // Set discharge test via heater fet load on|off
 #define MISCQ_SET_DCHGFETS 30 // Set discharge FETs: all, on|off, or single
 #define MISCQ_SET_SELFDCHG 31 // Set ON|OFF self-discharge mode
 #define MISCQ_PRM_MAXCHG   32 // Get Parameter: Max charging current
 #define MISCQ_SET_ZEROCUR  33 // 1 = Zero external current in effect; 0 = maybe not.
 #define MISCQ_READ_AUX     34 // BMS responds with A,B,C,D AUX register readings (12 msgs)
 #define MISCQ_READ_ADDR    35 // BMS responds with 'n' bytes sent in [3]
 #define MISCQ_PROC_TEMP    36 // Processor calibrated internal temperature (deg C)
 #define MISCQ_CHG_LIMITS   37 // Show params: Module V max, Ext chg current max, Ext. chg bal
 #define MISCQ_MORSE_TRAP   38 // Retrieve stored morse_trap code.
 #define MISCQ_FAN_STATUS   39 // Retrieve fan: pct and rpm 
/* FIELD =>offset<= numbering for BMS
	 0 - 17 Cells #1- #18 readings (0.1 mv)
	18 - Temperature #1
	19 - Temperature #2
	20 - Temperature #3
  21 - Fan RPM
  22 - Fan pwm
  23 - Current sensor
  24 - Status: bat
  25 - Status: fet
  26 - Status: mode
*/
char convertpayloadBMS_MULTI(struct CANRCVBUF* p, struct CANFIELD* pfld)
{
	int i;
	uint8_t celln;
	char flag = 0;
	double dtmp;

	union FI
	{
		float f;
		uint32_t ui;
	}fi;	

	switch (p->cd.uc[0])
	{
	case CMD_CMD_CELLPC: // Here, Response to PC requesting Cell voltages
		celln = (p->cd.uc[1] >> 4) & 0xf; // Cell number of 1st payload U16
		for (i = 0; i < 3; i++)
		{ // Convert U16 cell reading and save in field array for this CAN ID
			dtmp = p->cd.us[i+1]; // Convert to float
			(pfld + i + celln)->lgr = dtmp;
//printf("BAT %08X %2d %0.1f\n",p->id,celln+i, (pfld + i + celln)->lgr*0.1);
		}
		break;

// TODO: Additional fields
	case CMD_CMD_MISCPC: // Here, Response to PC requesting misc (various) readings
		switch(p->cd.uc[1])
		{
		case MISCQ_TEMP_CAL: // (04) BMS Temperature
			uint8_t idx = (p->cd.uc[3] & 0x3);
			if (idx > 2) idx = 2; // JIC
			fi.ui = p->cd.ui[1];
			float ftmp = fi.f;
			dtmp = ftmp;
			(pfld + 18 + idx)->lgr = dtmp;
			break;

		case MISCQ_FAN_STATUS: // (39) Fan speed, fan rpm		
			fi.ui = p->cd.ui[1]; // RPM
			dtmp  = fi.f;
			(pfld + 21)->lgr = dtmp;
			(pfld + 22)->lgr = p->cd.uc[3];
			break;

		case MISCQ_CURRENT_CAL: // (24) Current sense calibrated
			fi.ui = p->cd.ui[1];
			dtmp = fi.f;
			(pfld + 23)->lgr = dtmp;
			break;

		case MISCQ_STATUS: // (01) Status: battery, fets, mode
			(pfld + 24)->lgr = p->cd.uc[4]; // Battery
			(pfld + 25)->lgr = p->cd.uc[5]; // FETs
			(pfld + 26)->lgr = p->cd.uc[6]; // Mode
			break;
		}	
		break;

	case CMD_CMD_MISCHB: // (45) Heartbeat (w status)
		(pfld + 24)->lgr = p->cd.uc[4]; // Battery
		(pfld + 25)->lgr = p->cd.uc[5]; // FETs
		(pfld + 26)->lgr = p->cd.uc[6]; // Mode
		break;

	default:
		printf("convertpayloadBMS_MULTI: CAN cd.uc([0] %d) ([1] %d) not in switch case, %08X\n",p->cd.uc[0],p->cd.uc[1],p->id);
		break;
	}

	return flag;
}