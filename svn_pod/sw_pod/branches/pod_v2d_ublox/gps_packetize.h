/******************************************************************************
* File Name          : gps_packetize.h
* Date First Issued  : 09/05/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : GPS time handling
*******************************************************************************/
#ifndef __P1_GPS_PACKETIZE
#define __P1_GPS_PACKETIZE

#define GPSLIMITCT	90	// Limit time to get a good fix/time (secs)
#define GPSTIMEOUTCT	90	// Limit time to get a good fix/time (secs)
#define GPSDISCARDCT	4	// Number of initial good readings to discard
#define GPSSDPACKETSSKIP 10	// Number of GPS packets skip between writes to SD Card
#define PKT_GPS_ID	ID_GPS	// ID for GPS packets

#include "p1_common.h"
#include "p1_gps_time_convert.h"

/******************************************************************************/
struct PKT_PTR gps_packetize_poll(void);
/* @brief	: Do all things nice and good for gps
 * @return	: Pointer & count--zero = not ready.
 ******************************************************************************/
void gps_packetize_restart(void);
/* @brief	: Look for good gps data and store a packet
 ******************************************************************************/
struct PKT_PTR gps_packetize_get(void);
/* @brief	: Pointer to packet if ready
 * @return	: Pointer & count--zero = not ready.
 ******************************************************************************/
void gps_send_time_packet(void);
/* @brief	: Setup packet for sending
 ******************************************************************************/
struct PKT_PTR gpsfix_packetize_get(void);
/* @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
char * gpsfix_get_save(void);
char *gpspubx00_get_save(void);
/* @brief	: Get pointer to zero terminated string with data extracted from $GPGGA/$PUBX,00 sentences
 * @return	: Point to string.
 ******************************************************************************/
 void time_display(char *p, long long llt, long long llt_prev, int shift);
 /* @brief	: Display time
 * @param	: p = pointer to char string
 * @param	: llt = long long with coded time
 * @param	: llt_prev = previous llt
 * @param 	: shift = number of bits to shift-right llt for 32b linux format
 ******************************************************************************/

void shuttest(char c);

/* These are in 'gps_packetize.c' */
extern volatile unsigned int gps_sd_pkt_ready;		// SD packet ready:    	0 = not ready, + = ready. ?
extern volatile short gps_sd_pkt_ctr;		// SD packet ready:    	0 = not ready, + = ready. ?
extern volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
extern volatile short gps_monitor_flag;		// Monitor data:       	0 = ignore gps, + = collect data ?
extern volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown
extern unsigned short usGGAsavectr;	// GGA save flag/ctr for PC monitoring

/* Packet of GPS time versus RTC tick counter (@1) */
extern struct TIMESTAMPGP1 pkt_gps_sd;		// GPS time versus rtc tickcounter packet writing to SD card
extern struct TIMESTAMPGP1 pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring

/* Used for setting offset calibration */
extern unsigned int uiConsecutiveGoodGPSctr;	// Count of consecutive good GPS fixes
extern int nOffsetCalFlag;			// 0 = we have not updated the freq offset calibration

extern char cGPS_flag;
extern char cGPS_ready;		// Enough GPS fixes for it to have settled in

extern unsigned char gps_sentence_flag;	// GPS sentence listing on/off flag

extern short gps_limit_ctr;	// Discard fixes until at least 4 secs of consecutive good fixes
extern unsigned int uiGPSrtc;	// RTC tickcount saved at EOL of GPS time

extern u8 	gps_poll_flag;	// 0 = idle; 1 = update .SYS time
extern u32 gps_poll_flag_ctr;	// Running count of GPS v SYS time updates

#endif

