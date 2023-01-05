/******************************************************************************
* File Name          : cmd_f.c
* Date First Issued  : 05/13/2014
* Board              : PC
* Description        : Display gps fix: lat lon ht
*******************************************************************************/
/*
This lifts & modifies some code from--
~/svn_sensor/sensor/co1_Olimex/trunk/p1_PC_monitor_can.c

*/

#include "cmd_f.h"


/******************************************************************************
 * void cmd_f_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output current msg ID's ('n' command)
*******************************************************************************/
#ifdef SAVES_LOOKING_UP_HOW_MSGS_ARE_CONSTRUCTED

/******************************************************************************
 * static void gpsfix_msg(struct GPSFUNCTION* p, int n);
 * @brief 	: Setup and send FIX msgs
 * @param	: p = pointer to logger function struct with params and more
 * @param	: n: 0 = lat, 1, long, 2 = height
 ******************************************************************************/
static void gpsfix_msg(struct GPSFUNCTION* p, int n)
{
	int* ppay;
	struct CANRCVBUF can;	// Temporary CAN msg
	/* Index to last buffer populated by gps_poll (below) at main interrupt level */
	uint32_t idx = p->gpsfixidx ^ 0x1;
	can.id  = p->gps_s.cid_hb_llh;	// CANID: Heartbeat fix (3 msgs) lat/lon/ht
	can.dlc = 6;
// can.dlc = 7; // For debugging
	can.cd.uc[0]  = (p->gpsfix[idx].code << 2); // Upper bits for conversion code
	can.cd.uc[0] |= (p->gpsfix[idx].fix & 0x3); // Fix type in lower two bits

	can.cd.uc[1]  = (p->gpsfix[idx].nsats << 3); // Upper bits for number of satellites
	can.cd.uc[1] |= (n & 0x7);	// Msg index (lat/lon/ht)

	if (n == 2) 	{ppay = &p->gpsfix[idx].ht;}
	else if (n == 1){ppay = &p->gpsfix[idx].lon;}
	else 		{ppay = &p->gpsfix[idx].lat;}
	can.cd.uc[2]  =  (*ppay >> 0);
	can.cd.uc[3]  =  (*ppay >> 8);
	can.cd.uc[4]  =  (*ppay >> 16);
	can.cd.uc[5]  =  (*ppay >> 24);
//can.cd.uc[6] = n; // Added debugging byte
	can_hub_send(&can, p->phub_gps);	// Send CAN msg to 'can_hub' 
	return;
}
/* (This saves looking it up.)
struct GPSFIX
{
	int	lat;	// Latitude  (+/-  540000000 (minutes * 100,000) minus = S, plus = N)
	int	lon;	// Longitude (+/- 1080000000 (minutes * 100,000) minus = E, plus = W)
	int	ht;	// Meters (+/- meters * 10)
	unsigned char fix;	// Fix type 0 = NF, 1 = G1, 2 = G3, 3 = G3
	unsigned char nsats;	// Number of satellites
	int	code;	// Conversion resulting from parsing of the line
};
*/
/* msg format
pcan.uc[0]
 0:3 fix type (.fix)
   0 = 0 = NF, 1 = G1, 2 = G3, 3 = G3
 4:7 conversion code 
   0 = conversion good
   -12 = conversion good, but NF (No Fix)
   other negative numbers = line has some error and conversion aborted
pcan.uc[1] 
 0:2 which reading
   0 = latitude
   1 = longitude
   2 = height
 3:7 Number of satellites (.nsats)

pcan.uc[2:5] = .lat or .lon
pcan.uc[2:3] = .ht
*/

#endif

#define CANIDREF	 0XE1C00000	// Start with default
static unsigned int canidref = CANIDREF;


int cmd_f_init(char* p)
{
	int pn = strlen(p);

	if (pn < 3)
	{ // Here use previous CANID for lat/lon/ht msgs
		printf("Using CANID 0x%08X\n",canidref);
		return 0;
	}
	if (pn < 11)
	{
		printf("%d istoo few chars. Expecting something such as--\nf E2600000<enter>\n",pn);
		return -1;
	}
	// Here, sufficient chars
  	sscanf( (p+1), "%x",&canidref);
	printf("Using CANID 0x%08X\n",canidref);
	return 0 ;
}


struct GPSFIX // Lifted from ../svn_sensor/sensor/co1_sensor_ublox/trunk/p1_gps_time_convert.h
{
	int	lat;	// Latitude  (+/-  540000000 (minutes * 100,000) minus = S, plus = N)
	int	lon;	// Longitude (+/- 1080000000 (minutes * 100,000) minus = E, plus = W)
	int	ht;	// Meters (+/- meters * 10)
	unsigned char fix;	// Fix type 0 = NF, 1 = G1, 2 = G3, 3 = G3
	unsigned char nsats;	// Number of satellites
};

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

static struct GPSFIX gfx;
static double dlat;
static double dlon;
static double dht;
static int linectr = 32;;

void cmd_f_do_msg(struct CANRCVBUF* p)
{
	unsigned int code;
	int type;
	int latd;	// Integer degrees: lat
	int lond;	// Integer degrees: lon
	double dlatd;	// Conversion to minutes lat
	double dlond;	// Conversion to minutes lon

	if (p->id != canidref) return;

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

		linectr += 1;
		if (linectr > 32)
		{
			linectr = 0;
			printf ("  Lat(deg)    Lon(deg)     Height(m)  Lat(deg:min) Lon(deg:min) fix n-sats\n");
			
		}
		dlat = gfx.lat; dlon = gfx.lon; dht = gfx.ht;
		printf("%10.5f  %10.5f  %8.3f    -- ",dlat/60.0E5, dlon/60.0E5, dht/1E3);
		latd = dlat/60.0E5; dlatd = dlat/60.0E5; dlatd -= latd; dlatd *= 60.0;
		lond = dlon/60.0E5; dlond = dlon/60.0E5; dlond -= lond; dlond *= 60.0;
		printf("%d:%1.5f  %d:%1.5f  %2i %2i\n",latd, dlatd, lond, dlond, gfx.fix, gfx.nsats);
		
		break;

	default:
		printf ("Bad type code: %d code: %d\n",type, code);
	}

	return;
}


