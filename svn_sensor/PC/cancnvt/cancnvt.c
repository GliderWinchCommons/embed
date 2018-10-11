/******************************************************************************
* File Name          : cancnvt.c
* Date First Issued  : 10/09/2018
* Board              : Linux PC
* Description        : Convert gateway format CAN msgs to ascii readable payloads
*******************************************************************************/
/*
gcc -Wall cancnvt.c -o cancnvt 
gcc -Wall cancnvt.c -o cancnvt && ./cancnvt < ~/logcsa/181008_212136

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

	/* =============== Read table of CANID v payoad type ================================= */
	
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
					n = sprintf(&cout[0],"0x%08X  99 ",cantblx.id);
					for (m = 0; m < cantblx.dlc; m++)
					{
						n = sprintf(&cout[n]," %02X",cantblx.uc[m]);
					}
				}
				printf("%s\n",cout);
		}
	}
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



