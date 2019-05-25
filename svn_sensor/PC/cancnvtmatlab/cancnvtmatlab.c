/******************************************************************************
* File Name          : ccancnvtmatlab.c
* Date First Issued  : 05/08/2019
* Board              : Linux PC
* Description        : Convert gateway format CAN msgs to csv lines for matlab
*******************************************************************************/
/*
Hack of cancnvt 

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab 
gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab csvlinelayout.txt csvfieldselect.txt < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt

Arguments: 
  File with definition of CAN fields
  File with selection of fields for csv line output

Input: 
  standard input - raw CAN msgs

Ouput: 
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

/* TEMPORARY payload define until updated in SQL database */
#define I16_I16	      27 // 'I16_I16',	      27, 4,'[1]-[2]: uint16_t[0]; [3]-[2]: uint16_t[1]');
#define I16_I16_X6	   28 // 'I16_I16_X6',     28, 4,'[1]-[2]: uint16_t[0]; [3]-[2]: uint16_t[1]');
#define U8_U8_U8	      29 // 'U8_U8_U8',       29, 6,'[1]-[2]:[2] uint8_t');--
#define I16_X6          30 // 'I16_X6',         30, 7,'[1]-[2]: uint16_t,[6]: uint8_t');
#define I16_I16_I16_I16 31 // 'I16_I16_I16_I16',31, 8,'[1]-[2]:[3]-[2]:[5]-[4]:[7]-[6]:uint16_t');
#define I16__I16        32 // 'I16__I16',       32, 8,'[1]-[0]:uint16_t,[6]-[5]:uint16_t');
#define I16_I16_I16_X7	33 // 'I16_I16_I16_X6', 33, 8,'[1]-[2]:[3]-[2]:[5]-[4]:uint16_t,[6]:uint8_t');
#define I16_I16_X_U8_U8 34 // 'I16_I16_X_U8_U8',34, 8,'[1]-[2]:[3]-[2]:uint16_t,[5]:[6]:uint8_t');
#define I16             35 // 'I16',            18, 2,'[1]-[0]:uint16_t');			--

/* Line buffer size */
#define LINESZ 512	// Longest CAN msg line length
char buf[LINESZ];

/* CAN id selection input file  */
#define NAMESZ 128	// Max size of name field
#define CSVLINESZ 1024	// Max output line size

/* CAN field selection input list */
#define CSVSELECTSZ 512
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

/* CAN field layout list */
#define CANFLDLAYOUTSZ 512
#define MAXNUMFIELDS 5
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

char *paytype = "../../../svn_common/trunk/db/PAYLOAD_TYPE_INSERT.sql";
char *canid_insert = "../../../svn_common/trunk/db/CANID_INSERT.sql";

/* Command line switches */
char argflag = 0;   // Count number of command line switches
char cmdsw_t;
char cmdsw_x;
char cmdsw_c;
char * helplist = {"\nCommand line layout\n\
./cancnvtmatlab <options> <path/file input file #1> <path/file input file #2> <path/file input file ...> e.g.\n\
./cancnvtmatlab -t csvlinelayout.txt csvfieldselect.txt < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt\n\
  options\n\
 h - help\n\
 t - print input tables\n\
 x - do not convert input data file\n\
 c - csv position lines between csv data lines\n"};


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

//printf("cmdsw: t %i  x %i  c %i\n",cmdsw_t,cmdsw_x,cmdsw_c);	

//printf("argc: %i\n",argc);
	/* =============== Read input files ================================================ */
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
	unsigned int fldnum;  // Payload field number
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

				if (px == NULL) {printf("format field extact err c: %s\n",buf); return -1;}

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
	}

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
				for (k = 0; k < MAXNUMFIELDS; k++)
				{
					if (pcanfldlayout->canfield[k].fldnum != 0)
					{ // Convert & calibrate payload, and update last good reading (lgr)
						convertpayload(&can, &pcanfldlayout->canfield[k]);
					}
				}
			}
			else
			{ // Here, Not found: this CAN id does not have a payload layout entry in the array
				printf("# Not in payload layout array: 0X%0x\n", can.id);
			}
		}		
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
	unsigned int x;
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
('I16_I16_X6',     28, 4,'[1]-[0]: uint16_t[0]; [3]-[2]: uint16_t[1]');
('U8_U8_U8',       29, 6,'[1]-[2]:[2] uint8_t');--
('I16_X6',         30, 7,'[1]-[0]: uint16_t,[6]: uint8_t');
('I16_I16_I16_I16',31, 8,'[1]-[0]:[3]-[2]:[5]-[4]:[7]-[6]:uint16_t');
('I16__I16',       32, 8,'[1]-[0]:uint16_t,[6]-[5]:uint16_t');
('I16_I16_I16_X7', 33, 8,'[1]-[0]:[3]-[2]:[5]-[4]:uint16_t,[6]:uint8_t');
('I16_I16_X_U8_U8',34, 8,'[1]-[0]:[3]-[2]:uint16_t,[5]:[6]:uint8_t');
('I16',            35, 2,'[1]-[0]:uint16_t');			--


*/
void convertpayload(struct CANRCVBUF* pcanx, struct CANFIELD* pfld)
{
	/* k is index into payload field array. */
	if (pfld->fldnum < 1) return; // JIC field number zero 
	uint8_t k = pfld->fldnum-1; // k is index to payload field within 8 byte payload array

	union
	{
		unsigned int ui;
		signed int si;
		float ff;
	}ui_ff;
	ui_ff.ff = 0;

	/* Conversion to double after byte extraction. */
	// Most are unsigned, so default to 0
	char flag = 0; // unsigned = 0, signed = 1, float = 2;

	switch(pfld->paytype)
	{ 
	case I16:
	case I16_I16:
	case I16_I16_I16_I16:
		ui_ff.ui = payI16(pcanx,k*2);
		break;

	case I16_X6:
		if (k == 0) {ui_ff.ui = payI16(pcanx,0); break;}
		if (k == 1) {ui_ff.ui = pcanx->cd.uc[5]; break;}
		break;

	case I16_I16_X6:
		if (k != 2) {ui_ff.ui = payI16(pcanx,k*2); break;}
		if (k == 2) {ui_ff.ui = pcanx->cd.uc[5];   break;}
		break;

	case I16__I16:
		if (k == 1) k = 2;
	case I16_I16_I16_X7:
		if (k != 3) {ui_ff.ui = payI16(pcanx,k*2);break;}
		if (k == 3) {ui_ff.ui = pcanx->cd.uc[6];  break;}
		break;

	case I16_I16_X_U8_U8:
		if (k < 2)  {ui_ff.ui = payI16(pcanx,k*2);break;}
		if (k > 2)  {ui_ff.ui = pcanx->cd.uc[k];  break;}
		break;


	case NONE:
		break;

	case U8:
	case U8_U8:
	case U8_U8_U8:
		ui_ff.ui = pcanx->cd.uc[k];
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

	/* Convert to double */
	if      (flag == 1)
		pfld->lgr = ui_ff.si; // Signed -> double
	else if (flag == 0)
		pfld->lgr = ui_ff.ui; // Unsigned -> double
	else if (flag == 2)
		pfld->lgr = ui_ff.ff; // float -> double

	/* Calibrate and update last-good-reading as double */
	pfld->lgr = (pfld->lgr + pfld->offset) * pfld->scale;

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

