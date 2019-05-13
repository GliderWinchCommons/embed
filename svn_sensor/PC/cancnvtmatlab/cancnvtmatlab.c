/******************************************************************************
* File Name          : ccancnvtmatlab.c
* Date First Issued  : 05/08/2019
* Board              : Linux PC
* Description        : Convert gateway format CAN msgs to csv lines for matlab
*******************************************************************************/
/*
Hack of cancnvt 

gcc -Wall cancnvtmatlab.c -o cancnvtmatlab 
gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab < ~/logcsa/181008_212136
gcc -Wall cancnvtmatlab.c -o cancnvtmatlab && ./cancnvtmatlab < ~/GliderWinchItems/dynamometer/docs/data/log190504.txt | tee gsm1

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

//#define SELECT_SINGLECANID

union FLDTYPE
{
	float f;
	uint32_t u32;
	uint16_t u16;
	uint8_t u8;
};

#define PAYLDNAMESZ 64	// Size of payload name field
#define NUMFIELDSMAX 64	// Max number of CSV fields
struct CSVFIELD
{
	union FLDTYPE uf;	// Value of field
	uint32_t id;		// CAN id
	uint8_t	pos;		// Position in line
	uint8_t	upct;		// Number of updates since last line
	uint8_t	type;		// Field type: float, int, etc.
	uint8_t	payfld;	// Payload field number
	char name[PAYLDNAMESZ];		// Payload field name

}csvfield[NUMFIELDSMAX];
uint16_t csvfieldidx = 0;	// Number of field positions loaded
void cmd_f_do_msg(char* po, struct CANRCVBUF* p);
uint8_t DMOCchecksum(uint32_t id, uint8_t* puc, uint8_t dlc);
int DMOCstate(char* pc, uint8_t status);

/* Temporary payload define until updated in database */
#define I16_I16	      27 // 'I16_I16',	      27, 4,'[1]-[2]: uint16_t[0]; [3]-[2]: uint16_t[1]');
#define I16_I16_X6	   28 // 'I16_I16_X6',     28, 4,'[1]-[2]: uint16_t[0]; [3]-[2]: uint16_t[1]');
#define U8_U8_U8	      29 // 'U8_U8_U8',       29, 6,'[1]-[2]:[2] uint8_t');--
#define I16_X6          30 // 'I16_X6',         30, 7,'[1]-[2]: uint16_t,[6]: uint8_t');
#define I16_I16_I16_I16 31 // 'I16_I16_I16_I16',31, 8,'[1]-[2]:[3]-[2]:[5]-[4]:[7]-[6]:uint16_t');
#define I16_X6          32 // 'I16_X6',         32, 8,'[1]-[2]:uint16_t, [5]:uint8_t');
#define I16_I16_I16_X6	33 // 'I16_I16_I16_X6', 33, 8,'[1]-[2]:[3]-[2]:[5]-[4]:uint16_t,[6]:uint8_t');
#define I16_I16_X_U8_U8 34 // 'I16_I16_X_U8_U8',34, 8,'[1]-[2]:[3]-[2]:uint16_t,[5]:[6]:uint8_t');
#define I16             35 // 'I16',            18, 2,'[1]-[0]:uint16_t');			--

/* Line buffer size */
#define LINESZ 512	// Longest CAN msg line length
char buf[LINESZ];

/* One reading field */
struct FIELD
{
	int pos;				// Position on line (1-n)
	int typ;				// Type: 0 = int; 1 = float
	char b[LINESZ];	// Reading (string)
};

/* CAN id selection input file  */
#define NAMESZ 64	// Max size of name field
#define CANTBLSZ 512
struct CANTBL
{
	unsigned int id;	// CAN id
	int dlc;
	unsigned char uc[8];
	char tag;			// T = time, S = time sync, blank = all other
	int	fmt_code;	// E.g. U32 converted to 2
	char c[NAMESZ];	// CAN_INSERT.sql name	
}cantbl[CANTBLSZ];
int32_t	cantblsz = 0;

struct PAYTBL
{
	char ascii[24];
	int code;
};

struct PAYTBL paytbl[96];
unsigned int paytblsz = 0;


/* Identify time sync and date/time CAN msg CAN ids */
char tag;	// 'T' for date/time; 'S' for time sync msg

unsigned int canTid;	// CAN id for date/time msg	
unsigned int canSid;	// CAN id for time sync msg

/* Line number used for locating problems */
unsigned int linect = 0;

FILE* fpIn;

char *paytype = "../../../svn_common/trunk/db/PAYLOAD_TYPE_INSERT.sql";
char *canid_insert = "../../../svn_common/trunk/db/CANID_INSERT.sql";
//char *canid_insert = "w";

void payloadcnvt(char* p, struct CANTBL* pcanx);

/*******************************************************************************
 * static int cmpfunc (const void * a, const void * b);
 * @brief 	: Compare function for qsort and bsearch (see man pages)--using direct access
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfunc (const void * a, const void * b)
{
	const struct CANTBL *A = (struct CANTBL *)a;
	const struct CANTBL *B = (struct CANTBL *)b;
	long long aa = (unsigned int)A->id;
	long long bb = (unsigned int)B->id;


	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	char *pc;
	char *po;
	char *px;
	int i;

	/* **********************************************************************************  */
   /* Reading file with CAN id versus name.  The sequence in this file is the sequence of */
   /* the comma separate fields in the final output line                                  */
	/* **********************************************************************************  */

	/* =============== Read table of payload types ======================================= */
	if ( (fpIn = fopen (paytype,"r")) == NULL)
	{
		printf ("\nInput file did not open: %s\n",paytype); 
		exit (-1);
	}
printf("payload type file opened: %s\n",paytype);
	while ( (fgets (&buf[0],LINESZ,fpIn)) != NULL)	// Get a line
	{
		if (strncmp(buf,"INSERT",6) == 0)
		{
			po = &paytbl[paytblsz].ascii[0];
			pc = strchr(buf,'\''); // Spin forward to ascii nuemonic
			pc++;
			i = 0;
			while ((*pc != '\'')&&(*pc != 0)&&(i++ < 24)) *po++ = *pc++;

			pc++;
			i = 0;
			while ((*pc != ',')&&(*pc != 0)&&(i++ < 24)) pc++;
			pc++;
			sscanf(pc,"%d", &paytbl[paytblsz].code);
			paytblsz += 1;
		}
	}
	for (i = 0; i < paytblsz; i++)
	{
		printf("%3d %3d %s\n",i,paytbl[i].code,paytbl[i].ascii);
	}
	fclose(fpIn);
	printf("=================================================\n\n");

	/* =============== Read table of CANID v payload type ================================= */
	
	/* Convert CANID table ascii format field to numeric code */
	if ( (fpIn = fopen (canid_insert,"r")) == NULL)
	{
		printf ("\nInput file did not open: %s\n",canid_insert); 
		exit (-1);
	}
printf("canid_insert file opened: %s\n",canid_insert);

	while ( (fgets (&buf[0],LINESZ,fpIn)) != NULL)	// Get a line
	{
		if (strncmp(buf,"INSERT",6) == 0)
		{
//printf("%d %s",cantblsz,buf);
			po = &cantbl[cantblsz].c[0];
			pc = strchr(buf,'\''); // Spin forward to ascii CANID name
			pc++;
			i = 0;
			while ((*pc != '\'')&&(*pc != 0)&&(i++ < NAMESZ)) *po++ = *pc++;
			*po = 0;
			pc++;
			pc = strchr(pc,'\''); // Spin forward to ascii CANID hex
			pc++;
			sscanf(pc,"%x",&cantbl[cantblsz].id);
			for (i= 0; i < 4; i++)
			{
				pc = strchr(pc,'\''); // Spin forward to ascii format
				pc++;
			}

			px = pc;
			pc = strchr(pc,'\''); // Spin forward to end of ascii format
			*pc = 0; // String terminator

			// Look up code for this ascii format string
			strcpy(&cantbl[cantblsz].c[0],"LOOKUP FAILED");
			for (i = 0; i < paytblsz; i++)
			{ // Compare CANID_INSERT with PAYLOAD_TYPE ascii
				if ( (strcmp(px,&paytbl[i].ascii[0])) == 0)
				{
//printf("%d 0x%08X %s %d\n",cantblsz,cantbl[i].id,px,paytbl[i].code);
					cantbl[cantblsz].fmt_code = paytbl[i].code;
					break;
				}
			}
			cantblsz += 1;
		}
	}
	printf("cntblsz: %d\n",cantblsz);
	for (i = 0; i < cantblsz; i++)
	{
		printf("%3d 0x%08X %3d\n",i,cantbl[i].id,cantbl[i].fmt_code);
	}

	/* --------- SORT BY CAN ID ---------------------------------------------------------------------- */
	qsort(&cantbl[0], cantblsz, sizeof(struct CANTBL), cmpfunc);// Sort for bsearch'ing

	for (i = 0; i < cantblsz; i++)
	{
		printf("S %3d 0x%08X %3d\n",i,cantbl[i].id,cantbl[i].fmt_code);
	}
/* ==================== Convert data ================================================ */
	int m;
	int n;
	unsigned int ui;
	struct CANTBL cantblx;
	struct CANTBL* ptbl;
	char cout[512];
int tmp1,tmp2,tmp3;
double ftmp1,ftmp2;
int n1;

	while ( (fgets (&buf[0],LINESZ,stdin)) != NULL)	// Get a line from stdin
	{
		if (strlen(buf) > 12)
		{
//printf("%s",buf);
			// The ascii hex order is little endian.  Convert to an unsigned int
			cantblx.id = 0;
			for (m = 10; m >= 2; m-=2)
			{
				sscanf(&buf[m],"%2x",&ui);
				cantblx.id = (cantblx.id << 8) + ui;
			}
			sscanf(&buf[10],"%2x",&cantblx.dlc);

			// Get payload bytes	converted to binary
			for (m = 0; m < cantblx.dlc; m++)
			{
				sscanf(&buf[12+2*m],"%2x",(unsigned int*)&cantblx.uc[m]);
			}
		
			// Lookup payload code
				ptbl = bsearch(&cantblx.id, &cantbl[0], cantblsz, sizeof(struct CANTBL), cmpfunc);
				if (ptbl != NULL)
				{
					cantblx.fmt_code = ptbl->fmt_code;

//printf("K %08X %2d: %2d:",cantblx.id,cantblx.dlc,cantblx.fmt_code);
//for (n = 0; n < cantblx.dlc; n++) printf(" %02x",cantblx.uc[n]);printf("\n");

					n = sprintf(&cout[0],"0x%08X %3d ",cantblx.id,ptbl->fmt_code);
					payloadcnvt(&cout[n],&cantblx);
				}
				else
				{ // Here CAN id was not in CANID_INSERT.sql table
//printf("ID: 0x%08X dlc: %d %s \n",cantblx.id, cantblx.dlc,buf);
					if (!((cantblx.id == 0) || (cantblx.dlc > 8)))
					{
						n = sprintf(&cout[0],"0x%08X  99 :dlc %i:",cantblx.id,cantblx.dlc);
						for (m = 0; m < cantblx.dlc; m++)
						{
							sprintf(&cout[m*3+n]," %02X",cantblx.uc[m]);
						}
if (cantblx.id == 0xCA000000) // 0X650 - HV bus status
{
 tmp1 = cantblx.uc[0]*256+cantblx.uc[1];
 tmp2 = cantblx.uc[2]*256+cantblx.uc[3];
 ftmp1 = (double)tmp1 * 0.1;
 ftmp2 = ((double)tmp2 * 0.1) - 500.0;
 sprintf(&cout[m*3+n]," HVrx 0x%04X 0x%04X %8d %8d %7.1f %7.1f",tmp1,tmp2,tmp1,tmp2-5000,ftmp1,ftmp2);
}
if (cantblx.id == 0xCA200000) // 0X651 - Temperature
{
 sprintf(&cout[m*3+n]," TMrx %6d %6d %6d",cantblx.uc[0]-40,cantblx.uc[1]-40,cantblx.uc[2]-40);
}

if (cantblx.id == 0x47400000)    //  0X23A - Torque
{
 tmp1 = cantblx.uc[0]*(1<<8)+cantblx.uc[1];
 tmp2 = cantblx.uc[2]*(1<<8)+cantblx.uc[3];
 tmp3 = cantblx.uc[4]*(1<<8)+cantblx.uc[5];
 sprintf(&cout[m*3+n]," TQrx %6d %6d %6d %4u",tmp1-30000,tmp2-30000,tmp3-30000,cantblx.uc[6]);
}
if (cantblx.id == 0x47600000) // 0x23B - speed and current operation status
{
 tmp1 = cantblx.uc[0]*(1<<8)+cantblx.uc[1];
 n1 = sprintf(&cout[m*3+n]," SPrx 0x%04X %6d 0x%02X %2u ",tmp1,tmp1-20000,cantblx.uc[6],cantblx.uc[6]>>4);
 DMOCstate(&cout[m*3+n+n1], cantblx.uc[6]);
}
if (cantblx.id == 0x47C00000) //      0X23E --DQ volt amps rx
{
 tmp1 = cantblx.uc[0]*(1<<8)+cantblx.uc[1];
 tmp2 = cantblx.uc[2]*(1<<8)+cantblx.uc[3];
 sprintf(&cout[m*3+n]," DQrx 0x%04X %6d 0x%04X %6d ",tmp1,tmp1,tmp2,tmp2);
}
if (cantblx.id == 0x46400000) //      0X232 --Speed tx
{
  tmp1 = cantblx.uc[0]*(1<<8)+cantblx.uc[1];
  sprintf(&cout[m*3+n]," SPtx 0x%04X %d",tmp1,tmp1-20000);
}
if (cantblx.id == 0x46600000) //      0X233 --Torque tx
{
 tmp1 = cantblx.uc[0]*(1<<8)+cantblx.uc[1];
 tmp2 = cantblx.uc[2]*(1<<8)+cantblx.uc[3];
 sprintf(&cout[m*3+n]," ??tx 0x%04X %6d 0x%04X %6d ",tmp1,tmp1-30000,tmp2,tmp2-30000);
}
//Power limits plus setting ambient temp and whether to cool power train or go into limp mode
if (cantblx.id == 0x46800000) //     0X234 
{
 tmp1 = cantblx.uc[0]*(1<<8)+cantblx.uc[1]; // Regen watt limit ... MaxRegenWatts
 tmp2 = cantblx.uc[2]*(1<<8)+cantblx.uc[3]; // Accel limit ... MaxAccelWatts
 sprintf(&cout[m*3+n]," MWtx 0x%04X %6d 0x%04X %6d ",tmp1,4*(65000-tmp1),tmp2,4*tmp2);
}
if (cantblx.id == 0x05683004) // 0X00ad0600 unknown 29b DMOC
{
 tmp1 = cantblx.uc[0]*(1<<8)+cantblx.uc[1];
 tmp2 = cantblx.uc[2]*(1<<8)+cantblx.uc[3];
 sprintf(&cout[m*3+n]," ??rx 0x%04X %6d 0x%04X %6d ",tmp1,tmp1,tmp2,tmp2-30000);
}

					}
					else
					{
						sprintf(&cout[0],"0x%08X  98 %i\n",cantblx.id,cantblx.dlc);
					}
				}
/* Select one CAN id to be printf'd. */
#ifdef SELECT_SINGLECANID
//if (cantblx.id == 0xCA000000)
//if (cantblx.id == 0xCA200000)
//if (cantblx.id == 0x47400000)
//if (cantblx.id == 0x47600000)
if (cantblx.id == 0x46800000)
//if (cantblx.id == 0x05683004)
#endif
				printf("%s\n",cout);
		}
	}
printf("\nTEST of chksum generation\n");
/*
uint8_t DMOCchecksum(uint32_t id, uint8_t* puc, uint8_t dlc)
DMOC 0x232 tx: 0x4E 0x20 0x0 0x0 0x0 0x1 0x96 0xC6
0x46400000      0X232 
*/

// Test checksum generation: Note dlc
uint8_t uc[8] = {0x4E, 0x20, 0x0, 0x0, 0x0, 0x1, 0x96, 0xC6}; // GEVCU log
uint8_t chk = DMOCchecksum(0x46400000, &uc[0],7); // Test my routine
printf("From logs: DMOC 0x232 tx: 0x4E 0x20 0x0 0x0 0x0 0x1 0x96 0xC6\n");
printf("Generationed checksum of above w dlc = 7: 0x%02X\n",chk);
}

/* ******************************************************************************** 
 * unsigned payui(struct CANTBL* pcanx, int offset)
 * @brief	: Combine 4 bytes of payload into an unsigned int
 * @param	: pcanx = pointer to struct with payload
 * @param	: offset = number of bytes to offset for int
 **********************************************************************************/
unsigned payui(struct CANTBL* pcanx, int offset)
{
	unsigned int x;
		x  = pcanx->uc[0+offset] <<  0;
		x |= pcanx->uc[1+offset] <<  8;
		x |= pcanx->uc[2+offset] << 16;
		x |= pcanx->uc[3+offset] << 24;
		return x;
}
float payff(struct CANTBL* pcanx, int offset)
{
	union
	{
		unsigned int ui;
		float ff;
	}ui_ff;
	ui_ff.ui = payui(pcanx,offset);
	return ui_ff.ff;
}
/* ******************************************************************************** 
 * void payloadcnvt(char* p, struct CANTBL* pcanx)
 * @brief	: Convert payload bytes into formatted ascii output
 * @param	: p = pointer to output char array
 * @param	: pcanx = pointer to struct with payload bytes (binary)
 **********************************************************************************/
void structcnvt(struct CANRCVBUF* pcan, struct CANTBL* ptbl); // Declaration

void payloadcnvt(char* p, struct CANTBL* pcanx)
{
	unsigned int ui;
	unsigned int n;
	float ff;
	int m;
	struct CANRCVBUF can;

	struct tm* tb;
	char c[96];


	switch(pcanx->fmt_code)
	{
	case U16_U16:
		ui = payui(pcanx,0);
		n = sprintf(p,"%7d  ",ui);
		ui = payui(pcanx,2);
		sprintf((p+n),"%7d",ui);
		break;

	case NONE:
		break;

	case U8:
		sprintf(p,"%7d",pcanx->uc[0]);
		break;

	case U32:
		ui = payui(pcanx,0);
		sprintf(p,"%7d",ui);
		break;

	case U32_U32:
		ui = payui(pcanx,0);
		n = sprintf(p,"%7d  ",ui);
		ui = payui(pcanx,4);
		sprintf((p+n),"%7d",ui);
		break;

	case U8_U32:
		ui = payui(pcanx,1);
		sprintf(p,"%7d",ui);
		break;

	case FF:
		ff = payff(pcanx,0);
		sprintf(p,"%12.6f",ff);
		break;

	case FF_FF:
		ff = payff(pcanx,0);
		n = sprintf(p,"%12.6f  ",ff);
		ff = payff(pcanx,4);
		sprintf((p+n),"%12.6f",ff);
		break;

	case U8_FF:
		ff = payff(pcanx,1);
		sprintf(p,"%12.6f",ff);
		break;

	case UNIXTIME:
		ui = payui(pcanx,1);
		sprintf(p,"X %10i ",ui);
		tb = gmtime((time_t*)&ui);
		sprintf(c,"T %4d %2d %2d %2d %2d %2d",tb->tm_year+1900,tb->tm_mon+1,tb->tm_mday,tb->tm_hour,tb->tm_min,tb->tm_sec);
		strcat(p,c);
//printf("%s\n",p);
		break;

	case LAT_LON_HT:
		ui = payui(pcanx,2);
		structcnvt(&can,pcanx);
		cmd_f_do_msg(p,&can);
		break;

	default:
		strcat(p,"PAYLOAD NOT PROGRAMMED");
		m = strlen("PAYLOAD NOT PROGRAMMED");
		for (n = 0; n < pcanx->dlc; n++)
		{
				sprintf((p+n*3+m),"%02X ",pcanx->uc[n]);
		}
		break;
	}
//printf(":%s:\n",p);
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

void cmd_f_do_msg(char* po, struct CANRCVBUF* p)
{
	unsigned int code;
	int type;
	double dlat;	// Conversion to minutes lat
	double dlon;	// Conversion to minutes lon
	double dht;		// Conversion of height to meters

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
		sprintf (po,"BAD LAT/LON/HT TYPE CODE: %d code: %d\n",type, code);
		return;
	}
	dlat = gfx.lat; dlon = gfx.lon; dht = gfx.ht;
	sprintf(po,"%2d %2d %10.5f  %10.5f  %8.3f",gfx.fix, gfx.nsats,dlat/60.0E5, dlon/60.0E5, dht/1E3);

	return;
}
/* ************************************************************ 
 * void structcnvt(struct CANRCVBUF* pcan, struct CANTBL* ptbl);
 * Shameless hack: convert skinny struct to the "standard" struct
 * in order to use lat/long/ht code from other programs
 * ************************************************************/
void structcnvt(struct CANRCVBUF* pcan, struct CANTBL* ptbl)
{
	int i;
	pcan->id       = ptbl->id;
	pcan->dlc      = ptbl->dlc;
	for (i = 0; i < ptbl->dlc; i++)
	   pcan->cd.uc[i] = ptbl->uc[i];
	return;
}


/* *************************************************************************
 * uint8_t DMOCchecksum(uint8_t* puc, uint8_t dlc);
 *	@brief	: Compute CAN msg payload checksum for DMOC
 * @return	: checksum
 * *************************************************************************/
uint8_t DMOCchecksum(uint32_t id, uint8_t* puc, uint8_t dlc)
{
	uint8_t i;
	uint8_t sum;
	
	/* Checksum is based on a right justified 11b address. */
	sum = (id >> 21);

	for (i = 0; i < dlc; i++)
		sum += puc[i];

	return ((int)256 - (sum + 3));
}
/* *************************************************************************
 * int DMOCstate(char* pc, uint8_t status);
 *	@brief	: Convert byte 7 of 0x23B from DMOC to status ascii
 * @param	: pc = pointer to output string, next char to store position
 * @param	: status = byte 7 from DMOC CAN msg 0x23B
 * @return	: number of chars added
 * *************************************************************************/
int DMOCstate(char* pc, uint8_t status)
{
	int n = 0;
	uint8_t sts = status >> 4; // High nibble holds status code
	switch (sts)
	{
		
		case 0: //Initializing
			n = sprintf(pc,"DISABLED");
			break;
			
		case 1: //disabled
			n = sprintf(pc,"DISABLED");
			break;
			
		case 2: //ready (standby)
			n = sprintf(pc,"STANDBY");
			break;
			
		case 3: //enabled
			n = sprintf(pc,"ENABLE");
			break;
			
		case 4: //Power Down
			n = sprintf(pc,"POWERDOWN");
			break;
			
		case 5: //Fault
			n = sprintf(pc,"DISABLED");
			break;
			
		case 6: //Critical Fault
			n = sprintf(pc,"DISABLED");
			break;
			
		case 7: //LOS
			n = sprintf(pc,"DISABLED");
			break;
		}
	return n;
}
/*
void DmocMotorController::sendCmd3() {
	CAN_FRAME output;
	output.length = 8;
	output.id = 0x234;
	output.extended = 0; //standard frame
	output.rtr = 0;

	int regenCalc = 65000 - (MaxRegenWatts / 4);
	int accelCalc = (MaxAccelWatts / 4);
	output.data.bytes[0] = ((regenCalc & 0xFF00) >> 8); //msb of regen watt limit
	output.data.bytes[1] = (regenCalc & 0xFF); //lsb
	output.data.bytes[2] = ((accelCalc & 0xFF00) >> 8); //msb of acceleration limit
	output.data.bytes[3] = (accelCalc & 0xFF); //lsb
	output.data.bytes[4] = 0; //not used
	output.data.bytes[5] = 60; //20 degrees celsius
	output.data.bytes[6] = alive;
	output.data.bytes[7] = calcChecksum(output);

	CanHandler::getInstanceEV()->sendFrame(output);
}
*/
