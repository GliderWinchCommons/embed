/******************************************************************************
* File Name          : packet_extract.h
* Date First Issued  : 06/14/2013, 08/03/2018
* Board              : any
* Description        : Extract CAN msgs from a logger SD card file block
*******************************************************************************/

#ifndef __PACKET_EXTRACT
#define __PACKET_EXTRACT

#include <stdio.h>
#include <stdlib.h>
#include "common_can.h"

#define	SDCARD_BLOCKSIZE1		512
#define	SDLOG_DATASIZE			(512-8)

#define MSGSPERBLOCK	29	// Number of CAN msgs (16 bytes) per log format block

struct BLOCKPARSED
{
	struct CANRCVBUF can[MSGSPERBLOCK];
	uint32_t n;		// Number of CAN msgs 
	uint32_t nt;	// Index in 'can' for unix time msg, negative = none
	uint64_t pid;	// Extracted block PID
	 int8_t pksz;	// Packet size flag: 0 = OK; -x = count of number bad
};

struct PKTP
{
	int	ct;		// Packet size
	unsigned char*	p;	// Pointer to packet
	unsigned long long pid;
};
struct	LOGBLK			/* Used for local structured access */
{
	unsigned char data[SDLOG_DATASIZE];	// One block minus space for long long at end
	unsigned long long pid;		// Last 8 bytes holds the PID
	int blk;			// Nice place for the block number
};
union BLOCK			/* Union of raw & structured blocks */
{
	char raw[SDCARD_BLOCKSIZE1];			/* Buffer for "sdcard.[ch]" calls */
	struct LOGBLK str;
};

/*******************************************************************************/
int packet_extract_msgs(struct BLOCKPARSED* pbp, unsigned char* pblk);
/* @brief 	: Extract PID and extract CAN msgs for the block
 * @param	: pbp  = pointer to array of CAN msgs with some other stuff
 * @param	: pblk = pointer to control block with SD data
 * @param	: canid= CAN id for finding time msgs
 * @return	:  0 = OK; below are reasons for concern, alarm, or frustration--
 *          : -1 = PID and data all zero
 *          : -2 = PID zero, but something else in block was not zero
 *          : -3 = packet not 16 bytes (CAN struct size)
 *          : -4 = PID not zero, but there was no data
*******************************************************************************/
int packet_extract_time(struct BLOCKPARSED* pbp, unsigned int canid);
/* @brief	: Check CAN ids in array for a time msg
 * @param	: pbp  = pointer to block loaded with CAN msgs
 * @param	: canid = CAN id for time msg being searched
 * @return	: 0 = OK
*******************************************************************************/
void packet_extract_print(struct BLOCKPARSED* pbp);
/* @brief	: Check CAN ids in array for a time msg
 * @param	: pbp = pointer to block that has been extracted
*******************************************************************************/
uint32_t packet_extract_uint32_t(struct CANRCVBUF *pcan, uint32_t offset);
/* @brief	:  Extract uint32_t from offset in payload
 * @param	: pcan = pointer to CAN msg struct
 * @param	: offset = number of bytes offset for beginning of uint
*******************************************************************************/
int packet_extract_format(char *pout, struct BLOCKPARSED* pbp);
/* @brief 	: Write extracted CAN msgs in hex/ascii gateway format
 * @param	: pout = pointer to output ascii string
 * @param	: pbp  = pointer to array of CAN msgs with some other stuff
 * @return	: Number of chars in output
*******************************************************************************/


#endif

