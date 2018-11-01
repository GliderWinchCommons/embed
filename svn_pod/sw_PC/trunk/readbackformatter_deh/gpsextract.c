/******************************************************************************
* File Name          : gpsextract.c
* Date First Issued  : 10/28/2018
* Board              : Linux PC
* Description        : Convert downloaded file, for gps only
*******************************************************************************/
/*

// 01-05-2012 example--
gcc gosextract.c -o reformat -lm && sudo ./reformat ../dateselect/120106.023022

Example, with concatenated packet problem--
gcc reformat.c -o reformat -lm && sudo ./reformat ../dateselect/120722.182240

The timezone problem: the following 14400 was inserted to make the gps packet time
(which is gmt) match the tension packets (which are also gmt, but generated in the
POD).  Searchthe following line--
	uiLinux = mktime(&t) - 14400;	; // Linux time in one second ticks

export TZONEOFFSET=4
gcc gpsextract.c -o gpsextract -lm &&  ./gpsextract ~/winch/download/130317/130317.205045

// 10/28/2018 Hack of gpsextract.c
export TZONEOFFSET=5
gcc gpsextract.c -o gpsextract -lm &&  ./gpsextract ~/winch/download/181020/181020_atfield
*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

int tzoneadjust = 5;	// Number hours to adjust GPS to log time

#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits

/* Subroutine prototypes */
int gps_crc_check(char* p);
void pkt_tension(void);
void pkt_accelerometer(void);
void pkt_gps_sentence(void);
char* gps_extract_gpgga(char * pout, char * pin);
char* gps_extract_gpgga_ublox(char* pout, char* pin);
char* gps_extract_pgrmf(char* pout, char* pin);
char* gps_extract_pgrmv(char* pout, char* pin);
char* gps_extract_pgrme(char* pout, char* pin);
char* gps_extract_pubx00(char* pout, char* pin);
char* gps_extract_gpzda(char* pout, char* pin);
void outputboth(char *p);
unsigned long long pod_time_from_GPRMF(char * p);
char* field_skip(char* pin, int n);
void outputid(char c);
//static int compare_int(const void *a, const void *b);
void pkt_battery(void);
void pkt_pushbutton(void);
void pkt_gps_time(void);

char *gps_field(char *p, unsigned char n, unsigned int length );
char *gps_field_copy(char *pp, char *p, int length );
char* variable_field(char* pout, char* pin);
char* variable_field_digit(char* pout, char* pin, int size);
void ublox_combine_fields(char *pGPGGA, char *pGPZDA, char *pPUBX);

/* Line buffer size */
#define LINESIZE 1024
char buf[LINESIZE];

#define NUMSENTS	7	// Number of gps sentences
const char *gpsid[NUMSENTS] = {"GPGGA","PGRMF","PGRMV","PGRME","PUBX,","GPZDA","GPRMC"};
int nChk;	// Check that all sentences are present

/* Array to hold fields from sentence for assembly into a single output line */
#define GPSSIZE	512	// Line size for holding the extracted GPS fields
char cGPS[NUMSENTS][GPSSIZE];

/* Count the number of output lines of the various types */
#define NUMBERPKTTYPES	6
unsigned int linectr[NUMBERPKTTYPES];
/* From 'p1_common.h'
#define NUMBERPKTTYPES	6	// Number of packet types
#define ID_AD7799	1	// (@8)
#define ID_GPS		2	// (@11)
#define	ID_BATTTEMP	3	// (@13)
#define	ID_PUSHBUTTON	4	// (@12)
#define ID_ACCEL	5	// (@13)
#define ID_GPSFIX	6	// (@11) */
char * ptype[NUMBERPKTTYPES] = {"tension","gps set","battemp","pushbtn","accel  ","gps fix"};

/* Output line buf size */
#define OUTSIZE 1024
char bufout[OUTSIZE];

char zz[512]; //debug
unsigned long long ullX;

#define DATSIZE 200000	// Number of readings to deal with

/* Variables for sscanf of tension packet */
unsigned int id;
int tension[16];

/* For messing around with the various time forms */
unsigned long long ticktime2048;	// Ticktime from packet (1/2048th sec resolution)
unsigned long long ticktime64;		// Ticktime from packet in 1/64ths sec
unsigned long long linuxtime64;		// Ticktime >> 5 (1/64 sec resolution)
unsigned int linuxtime;			// Usual linux date/time
unsigned int linuxtimeN;
unsigned int subticktime;		// Lower 11 bits (2048) of ticktime from packet
unsigned int subsecfraction;

/* Type code is needed to distinguish between $GPGGA from Garmin and from ublox */
char cGPStype = 0;	// 0 = Garmin, 1 = u-blox

char fixstatus_pubx;
char fixstatus_gga;

char infile[512];	// Input file name buffer

/* You ought to be able to figure the following two */
FILE *fpIn;
FILE *fpOut;

int imax;

char bufadj[512];

// Line counters (mostly for debugging)
int rawlinectr;
int noasteriskctr;		// Count of type 6 lines with no asterisk
int otherlinectr;
int lessthan12ctr;
int defaultctr;
int gpsinputctr;
int gpserrctr;
int gpspubxctr;
int gpsgpzda;
int gpsgpgga;
int zerocharctr;

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j;
	char c;
	char* ptmp;

/* --------------------------- time zone adjustment ----------------------------------------------------------- */
	ptmp = getenv("TZONEOFFSET");
	if (ptmp == NULL)
	{
		printf ("Time zone adjust error: environmental variable TZONEOFFSET was not found\n"); exit(-1);
	}
	tzoneadjust = *ptmp - '0';
	if ((tzoneadjust > 8) || (tzoneadjust < 0))
	{
		printf ("Time zone adjust not within range ( 0 - 8), we have it as: %d\n",tzoneadjust); exit(-1);
	}


/* ************************************************************************************************************ */
/* Handle command line arguments, if any.                                                                       */
/*  Set these up on struct rwfile1                                                                              */
/* ************************************************************************************************************ */
	if (argc != 2)	// Requires an input file name on the command line
	{
		printf ("\nNeed input file name %d %s\n",argc,*argv);return -1;
	}

/* ************************************************************************************************************ */
/* Open read file                                                                                               */
/* ************************************************************************************************************ */
	if ( (fpIn = fopen (*(argv+1),"r")) == NULL)
	{
		printf ("\nInput file did not open %s\n",*argv); return -1;
	}
/* ************************************************************************************************************ */
/* Open an output file with same name as input file, but append 'R' at the end of the file name                 */
/* ************************************************************************************************************ */
	strcpy (infile, *(argv+1) );	// 
	strcat (infile,"R");
	if ( (fpOut = fopen (infile,"w")) == NULL)
	{
		printf ("\nOutput file did not open %s\n",infile); return -1;
	}	
	printf("\nOutfile name: %s\n",infile);
/* ************************************************************************************************************ */
/* Read in lines captured from POD                                                                              */
/* ************************************************************************************************************ */
	imax = 0;
	while ( (fgets (&buf[0],LINESIZE,fpIn)) != NULL)
	{
rawlinectr += 1;
		j = strlen(buf);
		c = buf[0];	// Packet ID is in the first char
		if (j< 11)
		{
			lessthan12ctr += 1;
			continue;
		}

		// Check for "zero" bogus line
		for (i = 0; i < j; i++)
		{
			if (buf[i] == 0x5C)
			{
				printf("\nERROR zero somewhere in the line at line: %d %s\n",rawlinectr,buf);
				zerocharctr += 1;
				i = -1;
				break;
			}
		}
		if (i < 0) continue;

		/* Adjust first char when download puts 0x0d in front of line */
		if (buf[0] == 0x0d)
		{
	  		strcpy(&bufadj[0],&buf[1]);
  			strcpy(buf,bufadj);
  			c = buf[0];	// Packet ID is in the first char
			printf("\n%d:%02x:%s\n",rawlinectr,c,buf);
		}
		switch (c)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			otherlinectr += 1;
			break;
		case '6': // Combine data from fields extracted from gps sentences
			if (strchr(buf,'*') == NULL) 
			if (gps_crc_check(&buf[0]) != 0)
			{
printf("ERROR crc check err: %d: %s\n",rawlinectr,buf);
				noasteriskctr += 1;
				break;
			}
			gpsinputctr += 1;
			pkt_gps_sentence();
			break;
		default:
			defaultctr += 1;
//printf("Line is greater than 12 chars: %d AND first char not in list: %c %s\n",j,c,buf);
			break;
		}
	}

	for (i = 0; i < NUMBERPKTTYPES; i++)
	{
		sprintf(zz,"Type%2u %s%8u\n",i+1,ptype[i],linectr[i]);
		outputboth(zz);
	}
	printf("noasteriskctr: %7d\n",noasteriskctr);
	printf("rawlinectr   : %7d\n",rawlinectr);
	printf("otherlinectr : %7d\n",otherlinectr);
	printf("lessthan12ctr: %7d\n",lessthan12ctr);
	printf("defaultctr   : %7d\n",defaultctr);
	printf("gpsinputctr  : %7d\n",gpsinputctr);
	printf("gpserrctr    : %7d\n",gpserrctr);

	printf("gpspubxctr   : %7d\n",gpspubxctr);
	printf("gpsgpzda     : %7d\n",gpsgpzda);
	printf("gpsgpgga     : %7d\n",gpsgpgga);
	printf("zerocharctr  : %7d\n",zerocharctr);


	printf ("Done\n");
	return 0;
}

/* ************************************************************************************************************* */
/* void pkt_gps_sentence(void);											 */
/* Convert accelerometer packet (ID = 6)									 */
/* ************************************************************************************************************* */

void pkt_gps_sentence(void)
{
/* Here is what the output line looks like after the sentences have be reworked and combined --
2496934758 34 54.95506N 081 57.28534W 1 08   415.1 060113172325.6 2   26     -2.2    -6.9    -0.0     4.6     6.7     8.1 6
Where the fields are--
Linux time (10)
Lat: degrees (2)
Lat: minutes (8)
N/S (1)
Long: degrees (3)
Long: minutes (8)
E/W (1)
Fix status (1)
Number Satellites (2)
Height (m) (7)
Date|time: ddmmyyhhmmss.s (14)
Fix type (1):
Speed over ground (km/hr) (4):
velocity E (m/s) (7):
velocity N (m/s) (7):
velocity Up (m/s)(7):
HDOP
VDOP
TDOP
id: '6'

*/
	int i,k,l;
	char *p = &buf[0];
	const char *pg;
	unsigned long long time64;		// Ticktime >> 5 (1/64 sec resolution)
	char vv[256];	// Temp char buff
	int len = strlen(buf);

	/* Convert hex time into something better */
	sscanf(buf,"%1x%10llx",&id,&ticktime2048);

	/* Convert ticktime2048 into linux date/time */
	linuxtime = (ticktime2048 >> 11) + PODTIMEEPOCH;	// Add epoch shift

	/* Isolate the subticks for each reading */
	subticktime = (ticktime2048 & 2047)>>5;

	/* Reduce tick time to resolution of tension readings */
	ticktime64 = ticktime2048 >> 5;
	/* Linux time in 64ths of a sec (for sorting A and T lines later */
	linuxtime64 = ticktime2048 >> 5;

	p=strchr(buf,'$');	// Point to '$' at beginning of sentence
	if (p == NULL) 
	{
		gpserrctr += 1;
		printf("ERROR gps sentence type 6 but no $ sign found at line: %d:%s",rawlinectr,buf);
		return;
	}
	p++;

	/* Determine sentence type */
	for (k = 0; k < NUMSENTS; k++)
	{
		i = strlen(gpsid[k]);
		if (strncmp(gpsid[k],p,i) == 0)
			break;
	}
	if (k >= NUMSENTS)
	{
		gpserrctr += 1;
		printf("ERROR gps sentence start not found for type 6 line, at line # %d:%d:%s",rawlinectr,k,buf);
		return;
	}

	/* Extract fields we want to assemble for the output */
	switch (k)	
	{
	case 0:	// GPGGA
		if (cGPStype == 0)
		{
			nChk |= 0x01;	// Garmin
			gps_extract_gpgga(&cGPS[0][0], p+6);	// Extract fields
		}
		else
		{
			nChk |= 0x01; 	// ublox. Since readout is backwards this one starts the new cycle		
			gps_extract_gpgga_ublox(&cGPS[0][0], p);	// Extract fields
		}
		gpsgpgga += 1;
		break;

	case 1: // PGRMF
		nChk |= 0x02; cGPStype = 0;
		gps_extract_pgrmf(&cGPS[1][0], p+6);
		break;

	case 2: // PGRMV
		nChk |= 0x04; cGPStype = 0;
		gps_extract_pgrmv(&cGPS[2][0], p+6);
		break;

	case 3: // PGRME
		nChk = 0x08;	// Since readout is backwards this one starts the new cycle
		 cGPStype = 0;
		gps_extract_pgrme(&cGPS[3][0], p+6);
		
//printf("# %s",buf); printf("# %s\n",cGPS);
		break;	

	case 4: // PUBX u-blox gps (Since only 4 letters, include the ',')
		cGPStype = 1;
		nChk = 0x10;
		gps_extract_pubx00(&cGPS[4][0], p);
		gpspubxctr += 1;
		break;

	case 5:	// GPZDA u-blox gps (time/date)
		cGPStype = 1;
		nChk |= 0x20;
		gps_extract_gpzda(&cGPS[5][0], p);	
		gpsgpzda += 1;	
		break;

	case 6: //GPRMC
		break;
	}

	/* Write extract data if all sentences were reported */
	if (nChk == 0x0f)	// All four sentences present for Garmin?
	{
		nChk = 0;

		/* Convert GPS time to extended linux time (seconds and 64 sub-seconds) */
		time64 = pod_time_from_GPRMF(&cGPS[1][0]);	// Convert GPS time linux64 time
		sprintf (vv,"%10llu ",time64);
		outputboth(vv);

		/* Output fields extracted from sentences */
		outputboth(&cGPS[0][0]); // GPGGA fields
		outputboth(&cGPS[1][0]); // PGRMF fields
		outputboth(&cGPS[2][0]); // PGRMV fields
		outputboth(&cGPS[3][0]); // PGRME fields
		outputid('6');	// Line ID (same as packet IDs)
		linectr[5] += 1;	// Count packet type 6

	}
	if (nChk == 0x31)	// All three sentences for u-blox?
	{
		nChk = 0;

		/* Convert GPS time to extended linux time (seconds and 64 sub-seconds) */
		time64 = pod_time_from_GPRMF(&cGPS[5][0]);	// Convert GPS time linux64 time
		sprintf (vv,"%10llu ",time64);
		outputboth(vv);

		/* Output fields extracted from sentences */
		//                         GPGGA       GPZDA       PUBX
		ublox_combine_fields(&cGPS[0][0],&cGPS[5][0],&cGPS[4][0]);
		linectr[5] += 1;	// Count packet type 6

	}

	return;
}
/* *************************************************************************************************************
 * char* gps_extract_gpgga_ublox(char* pout, char* pin);								 
 * Extract fields of $GPGGA											 
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* gps_extract_gpgga_ublox(char* pout, char* pin)
{
	int size = strlen(pin);	// Limit search of string
	char *p;
//	char *psave = pout;

	p = gps_field(pin, 9, size);	// Spin to MSL altitude field
	pout = variable_field(pout, p);

	p = gps_field(pin, 1, size);	// Spin to MSL altitude field
	fixstatus_gga = *p;			// Save for later

	*pout++ = 0;
//printf("%s",pin);
//printf("%s\n",psave);
//while (1==1);
	return pout;
}
/* *************************************************************************************************************
 * char* gps_extract_gpgga(char* pout, char* pin);								 
 * Extract fields of $GPGGA											 
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* gps_extract_gpgga(char* pout, char* pin)
{
	int i;
	double dX;		// Temp for handling float inputs

// Debug stuff
unsigned long long ullEpoch = PODTIMEEPOCH;/* Convert hex time into something better */
sscanf(buf,"%1x%10llx",&id,&ullX);
ullX = ullX >> 5;
ullX = ullX + (ullEpoch << 6);

	pin = field_skip(pin, 1); // Skip ahead to field #2
	
	/* Save latitude: input = 'ddmm.mmmmm' output */
//	pin++; // Skip ','
	*pout++ = *pin++; *pout++ = *pin++;	*pout++ = ' ';// degrees
	for (i = 0; i < 8; i++)	*pout++ = *pin++;
	pin++; // Skip ','

	*pout++ = *pin++; // Add 'N' or 'S'
	*pout++ = ' ';

	/* Save longitude: input = 'dddmm.mmmm' */
	pin++; // Skip ','
	*pout++ = *pin++;	*pout++ = *pin++; *pout++ = *pin++; *pout++ = ' ';	// degrees
	for (i = 0; i < 8; i++)	*pout++ = *pin++;
	pin++; // Skip ','
	*pout++ = *pin++; // Add 'W' or 'E'
	*pout++ = ' ';

	/* Save GPS quality digit */
	pin++; 	*pout++ = *pin++; *pout++ = ' ';

	/* Save Number of satellites */
	pin++; *pout++ = *pin++;  *pout++ = *pin++;

	pin++;
	while (*pin++ != ','); // Skip dilution field (variable width)

	/* Save height */
	sscanf(pin,"%lf",&dX);
	sprintf(pout,"%8.1F ",dX);

	return pout;	
}
/* *************************************************************************************************************
 * char* field_skip(char* pin, int n);
 * Skip over fields
 * Arg: pin = pointer to input buffer with gps sentence
 * Arg: n = count of ','
 * Ret: pointer points to beginning of field
 ************************************************************************************************************* */
char* field_skip(char* pin, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		while (*pin++ != ',');
	}
	return pin;	
}
/* *************************************************************************************************************
 * char* gps_extract_pgrmf(char* pout, char* pin);
 * Extract fields of $PGRMF											 
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* gps_extract_pgrmf(char* pout, char* pin)
{
	int i;
	int nN = 0;
	char * p = pin;

	p = field_skip(p, 2); // Skip ahead to field #3

	/* Save time of fix: day, month, year as 'ddmmyy.' */
	for (i = 0; i < 6; i++)	*pout++ = *p++;

	p++;	// Skip over ','

	/* Save time of fix: Hr, min, sec as 'hhmmss.s ' */
	for (i = 0; i < 8; i++)	*pout++ = *p++;

	*pout++ = ' ';	// Leave space from previous field
	


	pin = field_skip(pin, 10); // Skip to field #11
	*pout++ = *pin++;	// Fix type
	*pout++ = ' ';

	/* Speed over ground 0 - 1851 km/hr */
	pin++;		// Skip over ','

	/* Convert variable length field to a fixed length number */
	while (*pin != ',')
	{
		nN = (nN * 10) + *pin++ - '0';
	}
	sprintf (pout,"%4u ",nN);
	pout += 5;	

	// Add terminations to make anyone happy */
	*pout = 0;

	return pout;
}
/* *************************************************************************************************************
 * char* variable_field(char* pout, char* pin);
 * Convert one velocity field, .e.g. of $PGRMV										 
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
float clumsy;
char* variable_field(char* pout, char* pin)
{
	char c[16];	// Holds extracted field, e.g. -514.4
	char *p = &c[0];
	float fF;
	
	/* Copy ascii field to local string */
	while ((*pin != ',') && (*pin != '*'))
		*p++ = *pin++;
	*p = 0;		// String terminator

	/* Convert this variable length ASCII to float */
	sscanf(c,"%f",&fF);
	/* Clumsy addition pass back the floating number extracted */
	clumsy = fF;
	/* Convert the float to a fixed length & precision ASCII field */
	sprintf (pout,"%7.1f",fF);

	pout += 7;

	return pout;
}
/* *************************************************************************************************************
 * char* gps_extract_pgrmv(char* pout, char* pin);
 * Extract fields of $PGRMV											 
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* gps_extract_pgrmv(char* pout, char* pin)
{
	*pout++ = ' ';	// Leave space from previous field

	/* True east velocity, -514.4 to 514.4 meters/second for GPS 18x PC/LVC; -514.44 to 514.44 for GPS 18X 5Hz */
	pout = variable_field(pout, pin);
	*pout++ = ' ';
	while (*pin++ != ',');

	/* True north velocity, -514.4 to 514.4 meters/second for GPS 18x PC/LVC; -514.44 to 514.44 for GPS 18X 5Hz */
	pout = variable_field(pout, pin);
	*pout++ = ' ';
	while (*pin++ != ',');

	/* Up velocity, -999.9 to 999.9 meters/second for GPS 18x PC/LVC; -999.99 to 999.99 for GPS 18x-5Hz */
	pout = variable_field(pout, pin);

	// Add terminations to make anyone happy */
	*pout = 0;

	return pout;
}
/* *************************************************************************************************************
 * char* gps_extract_pgrme(char* pout, char* pin);
 * Extract fields of $PGRME
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* gps_extract_pgrme(char* pout, char* pin)
{
	*pout++ = ' ';	// Leave space from previous field

	/* Estimated horizontal position error (HPE), 0.0 to 999.9 meters */
	pout = variable_field(pout, pin);
	*pout++ = ' '; 
	pin = field_skip(pin, 2); // Skip over 'M,' to next field

	/* Estimated vertical position error (VPE), 0.0 to 999.9 meters */
	pout = variable_field(pout, pin);
	*pout++ = ' ';
	pin = field_skip(pin, 2); // Skip over 'M,' to next field

	/* UEstimated position error (EPE), 0.0 to 999.9 meters */
	pout = variable_field(pout, pin);

	*pout = 0;

	return pout;
}

/* *************************************************************************************************************
 * void ublox_combine_fields(char *pGPGGA, char *pGPZDA, char *pPUBX);
 * Brief: Combine fields extracted from the three sentences into one Garmin compatible output line
 * Args: Pointers to lines extracted.
 ************************************************************************************************************* */
void ublox_combine_fields(char *pGPGGA, char *pGPZDA, char *pPUBX)
{
	int i;
	char *pa = {"*"};
	char *pu = {"_"};
	char *ps = {"@"};

	/* PUBX lines was set up with space for inserting GGA and ZDA fields */

	/* Insert height from GPGGA sentence */
	i = strcspn(pPUBX, pa);	// Find number chard to beginning of height field
	if (i > 66) i = 65;
	strncpy( (pPUBX + i), pGPGGA, 7);

	/* Insert date|time from GPZDA sentence */
	i = strcspn(pPUBX, pu);	// Find number chars to beginning of date|time field
	if (i > 76) i = 76;
	strncpy( (pPUBX + i), pGPZDA, 14);	// Insert ddmmyyhhmmss.s

	/* Insert Fix type (saved from PUBX) */
	i = strcspn(pPUBX, ps);	// Find number chars to beginning of date|time field
	if (i > 76) i = 76;
	*(pPUBX + i) = fixstatus_gga;	// Insert code char

	/* Output */
	outputboth(pPUBX);

	/* Output id and new line, etc. */
	outputid('6');	// Line ID (same as packet IDs)

	/* Reset flags that show all three sentences present and extracted */
	nChk = 0;

	return;
}
/* *************************************************************************************************************
 * char* gps_extract_pubx00(char* pout, char* pin);
 * Extract fields of $PUBX,00 (u-blox gps)
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* gps_extract_pubx00(char* pout, char* pin)
{
	char *p;
	int qcode;
	char c;
	float v,vEast,vNorth,d;
	char vv[128];
	int i;
	int sz = strlen(pin);
//char* poutsave=pout;

	p = gps_field(pin, 3, sz); // Skip ahead to latitude

	/* Save latitude: input = 'ddmm.mmmmm' output */
	*pout++ = *p++; *pout++ = *p++;	*pout++ = ' ';// degrees
	for (i = 0; i < 8; i++)	*pout++ = *p++;	// Copy seconds

	p++; *pout++ = *p++; // Add 'N' or 'S'
	*pout++ = ' ';

	/* Save longitude: input = 'dddmm.mmmm' */
	p++; // Skip ','
	*pout++ = *p++;	*pout++ = *p++; *pout++ = *p++; *pout++ = ' ';	// degrees
	for (i = 0; i < 8; i++)	*pout++ = *p++;
	p++; // Skip ','
	*pout++ = *p++; // Add 'W' or 'E'
	*pout++ = ' ';
	
	/* Recode two digit ASCII quality into one digit numeric (to match Garmin) */
	p = gps_field(pin, 8, sz);
	qcode = ( (*p << 8) | (*(p+1) & 0xff) );
	switch (qcode)
	{
	case (('N' << 8) | ('F')): c = '0'; break; // No fix
	case (('D' << 8) | ('R')): c = '1'; break; // Dead reckoning
	case (('G' << 8) | ('2')): c = '2'; break; // Stand alone 2D
	case (('G' << 8) | ('3')): c = '3'; break; // Stand alone 3D
	case (('D' << 8) | ('2')): c = '4'; break; // Differential 2D
	case (('D' << 8) | ('3')): c = '5'; break; // Differential 3D
	case (('R' << 8) | ('K')): c = '6'; break; // Combined GPS + dead reckoning
	case (('T' << 8) | ('T')): c = '7'; break; // Time only
	default: c = 'x';
	}
	fixstatus_pubx = c;	// Save for insertion later.
	*pout++ = '@';		// Mark position for insertion from GPGGA
	*pout++ = ' ';
	
	/* Number of satellites */
//printf("%s",pin);
	p = gps_field(pin, 18, sz); // Skip ahead to number of sats
	pout = variable_field_digit(pout, p, 2);
	*pout++ = ' ';

	/* Allow space for height */
	for (i = 0; i < 7; i++) *pout++ = '*';
	*pout++ = ' ';

	/* Allow space for date & height */
	for (i = 0; i < 14; i++) *pout++ = '_';
	*pout++ = ' ';

	/* Fix quality */
	*pout++ = fixstatus_pubx; *pout++ = ' ';

	/* Ground speed (km/hr) */
	p = gps_field(pin, 11, sz);
	variable_field(vv, p);
	sprintf (pout,"%4.0f ",clumsy);
	pout += 4; *pout++ = ' ';*pout++ = ' ';

	/* Direction of course over ground.  Convert to East & North velocities. */
	v = clumsy * (1000.0/3600.0);	// Save speed over ground (convert to m/s)
	p = gps_field(pin, 12, sz);	// Course over ground field
	variable_field(vv, p);		// Extract and convert to double
	d = clumsy * (3.14159265359/180.0);	// Save course in radians
	vEast  = v * sinf(d);		// Convert to velocity East
	vNorth = v * cosf(d);		// Convert to velocity North
	sprintf (pout,"%7.1f ",vEast);
	pout += 7; *pout++ = ' ';
	sprintf (pout,"%7.1f ",vNorth);
	pout += 7; *pout++ = ' ';

	/* Vertical velocity */
	p = gps_field(pin, 13, sz);	// Vertical velocity (positive = downwards)
	variable_field(vv, p);		// Extract and convert to double (dump ascii into vv)
	sprintf (pout,"%7.1f ",-clumsy);// Reverse sign
	pout += 7; *pout++ = ' ';
		
	/* HDOP, estimated horizontal dilution of precision */
	p = gps_field(pin, 15, sz);
	pout = variable_field(pout, p);
	*pout++ = ' '; 

	/* VDOP, estimated vertical dilution of precision */
	p = gps_field(pin, 16, sz);	//	pout = variable_field(pout, pin);
	pout = variable_field(pout, p);
	*pout++ = ' ';

	/* TDOP, time dilution of precision */
	p = gps_field(pin, 17, sz );	//	pout = variable_field(pout, pin);
	pout = variable_field(pout, p);

	*pout = 0;
//printf("%s",pin);
//printf("%s X\n",poutsave);
//while(1==1);
	return pout;
}
/* *************************************************************************************************************
 * char* gps_extract_gpzda(char* pout, char* pin);
 * Extract fields of $GPZDA (u-blox gps)
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* gps_extract_gpzda(char* pout, char* pin)
{ 
/* Sample line
$GPZDA,211155.80,20,03,2013,00,00*6C
*/
	char *p;
	char *pt;
	int sz = strlen(pin);
	int w;
	char c[6];
	int i;

//char *psave=pout;

	p = gps_field(pin, 2, sz);	// Spin to field dd
	pt = gps_field_copy(pout, p, 2);	// Copy field
	w = (pt - pout);			// Number copied
	if (w != 2)			// Check if time missing
	{ // Here, overwrite with dummy day
		*(pout) = '1'; *(pout+1) = '2';		
	}

	p = gps_field(pin, 3, sz);	// Spin to mm
	pout = pt;
	pt = gps_field_copy(pout, p, 2);	// Copy field
	w = (pt - pout);			// Number copied
	if (w != 2)			// Check if time missing
	{ // Here, copy dummy month
		*(pout) = '0'; *(pout+1) = '1';		
	}

	pout = pt;
	p = gps_field(pin, 4, sz);	// Spin to yy
	pt = gps_field_copy(&c[0], p, 4);	// Copy field
	w = (pt - &c[0]);			// Number copied
	if (w != 4)			// Check if time missing
	{ // Here, copy dummy year
		*pout++ = '0'; *pout++ = '1';		
	}
	else
	{
		*pout++ = c[2]; *pout++ = c[3];
	}

	/* Copy hhmmss.ss field, but not the last .01 secs digit */
	p = gps_field(pin, 1, sz);	// Spin to field dd
	pt = gps_field_copy(pout, p, 8);	// Copy field
	w = (pt - pout);			// Number copied
	if (w != 8)			// Check if time missing
	{ // Here, overwrite with dummy hhmmss.s
		for (i = 0; i < 8; i++) *pout++ = '1';
		*(pout - 2) = '.';
	}
	pout = pt;
	

//printf("\nQ%s",pin); 
//*(psave + (pout-psave))=0;
//printf("M%s \n",psave);
//while(1==1);

	return pout;	
}
/* *************************************************************************************************************
 * void outputboth(char *p)
 * Brief: Outputs on the console and also the output file
 * Arg: p = pointer to a string
 ************************************************************************************************************* */
void outputboth(char *p)
{
	printf ("%s",p);	// For the hapless Op or piping

/* Use '>' or '| tee' on command line for output file */
//	fputs(p, fpOut);	// For the serious programmer
	return;
}
/* *************************************************************************************************************
 * void outputid(char c)
 * Brief: Outputs line ID.
 * Arg: c = line id (ascii)
 ************************************************************************************************************* */
void outputid(char c)
{
	char ww[16];
	char *pout = &ww[0];

	// Add terminations to make anyone happy */
	*pout++ = ' '; *pout++ = c; 	// Add packet ID
	*pout++ = 0x0a; 
	*pout = 0;
	outputboth(ww);
	return;
}
/* ================= Lifted from 'p1_gps_time_linux.c and modified =============================== */
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits
/******************************************************************************
 * static void ascii_bin(char * p);
 * @brief	: Convert two ascii to binary
 * @param	: pointer to high order ascii
 * @return	: binary number 0 - 99
 ******************************************************************************/
static int ascii_bin(char * p)
{
	int temp;
	temp = *p++ - '0';
	return (10 * temp) + *p - '0';
}
/******************************************************************************
 * unsigned long long pod_time_from_GPRMF(char * p);
 * @brief	: Extract and compute POD extended linux time (64 sub-second ticks)
 * @param	: p - pointer to GPS line
 * @return	: POD time in 64 ticks per sec
 ******************************************************************************/
unsigned long long pod_time_from_GPRMF(char * p)
{
	unsigned int uiLinux;
	unsigned long long ull,ull2;
	unsigned long long ullEpoch = PODTIMEEPOCH;
	struct tm t;

// Debug
//struct tm *ptm;
//char *pp;
/*
     tm_isdst  A flag that indicates whether daylight saving time is in effect at the time described.  The value is positive if  daylight  saving
     time is in effect, zero if it is not, and negative if the information is not available.
*/

	/* Fill tm struct with values extracted from gps */
	// 'ddmmyyhhmmss.s'
	t.tm_sec =	 ascii_bin(p+10);
	t.tm_min = 	 ascii_bin(p+ 8);
	t.tm_hour =	 ascii_bin(p+ 6);
	t.tm_mday =	 ascii_bin(p+ 0);
	t.tm_mon =	 ascii_bin(p+ 2) - 1;
	t.tm_year =	 ascii_bin(p+ 4) + (2000 - 1900);	
	t.tm_isdst = 0;
	uiLinux = mktime(&t) - (3600*tzoneadjust);	; // Linux time in one second ticks



	ull = uiLinux; 
	ull2 = ull << 6;	// Linux time in 64/sec ticks

	/* Convert 0.0, 0.2, 0.4, 0.6, 0.8 gps secs to nearest 1/64th tick */
	/* E.g.: 2 * 64 -> 128, round by adding 5, divide by 10 -> 13; 13/64 = .203125 */
	ull2 = ull2 + (((*(p+13)-'0') * 64) + 5)/10;

/* ########## SHAMELESS HACK FOR STANDARD TIME ############ */
//ull2 -= (3600 << 6);

//ptm = gmtime( (const time_t*)&uiLinux);
//pp = asctime (ptm);
//pp = gmtime((const time_t*)&uiLinux);

//uiLinux = ullX >> 6;
//printf("%10llu %10llu %10u %s |%s", ull2,ullX,uiLinux, p,pp);
//printf (" ||%s",	ctime((const time_t*)(&uiLinux)) );

//printf(" yr%3u mn%2u da%2u hh%2u mm%2u ss%2u\n",t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min,t.tm_sec);
	
	return (ull2 - (ullEpoch << 6));
}
/******************************************************************************
 * char *gps_field(char *p, u8 n, u16 length );
 * @brief	: Position pointer to comma separated field
 * @param	: p = pointer to GPS line
 * @param	: n = field, 0 = $xxx, 1 = first field following $xxx
 * @param	: length = max number of chars to stop the search
 * @return	: pointer to char following ',' or not found return original 'p'
 ******************************************************************************/
char *gps_field(char *p, unsigned char n, unsigned int length )
{
	unsigned char r = 0;	// Field counter
	char *psv = p;

	/* Stop--at end of line, or run out of fields, or too many chars searched */
	while ( (*p != 0xa) && (*p != 0xd) && (r < 23) && (length > 0))
	{ // Here we are good to go.
		if ( *p++ == ',')
		{
			r += 1;
			if (r == n) return p;  // Return point to char after ','
		}
		length -= 1;
	}
	return psv;	// We didn't find the field number specified
}
/******************************************************************************
 * char *gps_field_copy(char *pp, char *p, int length);
 * @brief	: Copy field from current position 'p' to next ',' or length limit
 * @param	: pp = pointer to output line
 * @param	: p = pointer to GPS input line
 * @param	: length = max number of chars to stop the copy
 * @return	: pp = pointing to next char position to store in
 ******************************************************************************/
char *gps_field_copy(char *pp, char *p, int length )
{
	/* Stop--at end of line, or run out of fields, too many chars searched, or ',' (next field) */
	while ( (*p != 0xa) && (*p != 0xd) && (length-- > 0) && (*p != ',') )
	{ // Here we are good to go.
		*pp++ = *p++;	// Copy input to output
	}
	return pp;
}
/* *************************************************************************************************************
 * char* variable_field_digit(char* pout, char* pin, int size);
 * Convert one velocity field, .e.g. of $PGRMV										 
 * Arg: pout = pointer to output buffer								
 * Arg: pin = pointer to input buffer with gps sentence
 * Arg: size = number of places in output
 * Ret: output pointer points to next free position in output buffer
 ************************************************************************************************************* */
char* variable_field_digit(char* pout, char* pin, int size)
{
	char c[16];	// Holds extracted field, e.g. -514.4
	unsigned int sz;
	char *p;
	char *pp;
	int i;
	
	if (size > 16) size = 16;

	/* Copy ascii field to local string */
	p = gps_field_copy(&c[0], pin, 16);
	sz = (p - &c[0]);	// Size of copied field 
	if (sz >= 16) sz = 15;
	
	/* Copy to fixed width field with leading blanks */
	pp = &c[0];
	for (i = 0; i < size; i++)
	{
		if ((size - i) > sz)	
			*pout++ = '0';
		else
			*pout++ = *pp++;
	}
	return pout;
}
/******************************************************************************
 * int gps_crc_check(char* p);
 * @brief	: Check for gps sentence crc
 * @param	: p = pointer to GPS line
 * @return	: zero == OK; non-zero = badness
 ******************************************************************************/
static unsigned char asctobin(char x)
{
	if (x <= '9') return (x - '0');
	return (x - 'A' + 10);
}
int gps_crc_check(char* p)
{
	char *px = strchr(p,'$');
	char *pend = p + strlen(p);
	unsigned char sum = 0;
	unsigned char z;
	
	/* Line must have a '$' and '*' following the4 '$' */
	if (px == NULL)             return -1;
	if (strchr(px,'*') == NULL) return -1;

	px++;	// Point to 1st char following the '$'

	while ( (*px != '*') && (px < pend) )
		sum ^= *px++;

	/* Convert GPS ASCII checksum */
	z = asctobin(*(pend-3));
	z = z*16 + asctobin(*(pend-2));

	return (z - sum);
}
