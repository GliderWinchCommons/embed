/******************************************************************************
* File Name          : packet_extract.c
* Date First Issued  : 06/14/2013,08/03/2018
* Board              : any
* Description        : Extract CAN msgs from a logger SD card file block
*******************************************************************************/
/*
08/03/2018 Update for latest format (no time stamp on each CAN msg)
*/
#include <stdlib.h>
#include <string.h>
#include "packet_extract.h"
#include "PC_gateway_comm.h"

/*******************************************************************************
 * int packet_extract_msgs(struct BLOCKPARSED* pbp, unsigned char* pblk);
 * @brief 	: Extract PID and extract CAN msgs for the block
 * @param	: pbp  = pointer to array of CAN msgs with some other stuff
 * @param	: pblk = pointer to control block with SD data
 * @param	: canid= CAN id for finding time msgs
 * @return	:  0 = OK; below are reasons for concern, alarm, or frustration--
 *          : -1 = PID and data all zero
 *          : -2 = PID zero, but something else in block was not zero
 *          : -3 = packet not 16 bytes (CAN struct size)
 *          : -4 = PID not zero, but there was no data
*******************************************************************************/

int packet_extract_msgs(struct BLOCKPARSED* pbp, unsigned char* pblk)
{
	 // Point to PID at end of SD block
	unsigned char *px = pblk + SDLOG_DATASIZE - sizeof(unsigned long long);
	unsigned char *ptmp;

	union BLOCK *pb = (union BLOCK *)pblk;
	uint64_t *pll;
	int i;

	/* Extract pid */
	pbp->pid = (pb->str.pid);

	if (pbp->pid == 0) // Is PID zero (bad)?
	{ // Yes, check for no data, (using long long)
		pll = (uint64_t*)px;
		while ((*pll == 0) && (pll > (uint64_t*)pblk)) pll--;
		if (*pll == 0) return -1;  // All zero
		return -2;	// Just PID was zero
	}

	/* Here, PID not zero (OK) and looks like there is some data. */

	// Search back to first non-zero byte (which should be packet size)
	ptmp = px - 1;
	// Spin back through data
	i = 0;
	while ((*ptmp == 0) && (i <= SDLOG_DATASIZE)) {ptmp--;i += 1;}

	if (i >= SDLOG_DATASIZE) // Did we end still pointing to a zero?
	{ // Here data was all zero
		return -4;
	}

	/* Extraction */
	pbp->nt = -1;	// Initialize for no time msgs
	pbp->n  =  0;	// No CAN msgs...yet.
	pbp->pksz = 0;	// No bad packet sizes...yet.
	px = pblk;
	while (px <= ptmp)
	{
		memcpy((char*)&pbp->can[pbp->n], px, sizeof(struct CANRCVBUF));

		px += sizeof(struct CANRCVBUF);

		// All should be the same size (16 bytes)
		if (*px != sizeof(struct CANRCVBUF))
			pbp->pksz -= 1; // Count bad ones

		pbp->n += 1;	// Add to count in array
		px += 1;	// Step to next CAN msg
	}
	return 0;	// Return with success code
}
/*******************************************************************************
 * int packet_extract_time(struct BLOCKPARSED* pbp, unsigned int canid);
 * @brief	: Check CAN ids in array for a time msg
 * @param	: pbp  = pointer to block loaded with CAN msgs
 * @param	: canid = CAN id for time msg being searched
 * @return	: 0 = OK
*******************************************************************************/

int packet_extract_time(struct BLOCKPARSED* pbp, unsigned int canid)
{
	int i;
	pbp->nt = -1;
	for (i = 0; i < pbp->n; i++)
	{
		if (pbp->can[i].id == canid)
		{
			pbp->nt = i;
		}
	}
	return pbp->nt;
}
/*******************************************************************************
 * void packet_extract_print(struct BLOCKPARSED* pbp);
 * @brief	: Check CAN ids in array for a time msg
 * @param	: pbp = pointer to block that has been extracted
*******************************************************************************/
void packet_extract_print(struct BLOCKPARSED* pbp)
{
	printf ("Number of CAN msgs: %d\n",pbp->n);
	if (pbp->n < 0) return;
	if (pbp->n > MSGSPERBLOCK) return;

   printf ("PID number: %lli\n",(unsigned long long)pbp->pid);
	if (pbp->pid == 0) return;

	printf ("Number of time msgs: %d\n",pbp->nt);

	printf ("Number of packet sizes not 16: %d\n",pbp->pksz);

	int i,j;
	for (i = 0; i < pbp->n; i++)
	{
		printf("%2d 0X%08X %d:",i, pbp->can[i].id,pbp->can[i].dlc);
		if (pbp->can[i].dlc > 8)
		{
			printf("\tDLC too large: %d\n",pbp->can[i].dlc);
		}
		for (j = 0; j < pbp->can[i].dlc; j++)
		{
			printf("%02X ",pbp->can[i].cd.uc[j]);
		}
		printf("\n");
	}
	return;
}
/*******************************************************************************
 * uint32_t packet_extract_uint32_t(struct CANRCVBUF *pcan, uint32_t offset);
 * @brief	:  Extract uint32_t from offset in payload
 * @param	: pcan = pointer to CAN msg struct
 * @param	: offset = number of bytes offset for beginning of uint
*******************************************************************************/
uint32_t packet_extract_uint32_t(struct CANRCVBUF *pcan, uint32_t offset)
{
	uint32_t tmp;
	tmp  = pcan->cd.uc[offset + 0] <<  0;
	tmp |= pcan->cd.uc[offset + 1] <<  8;
	tmp |= pcan->cd.uc[offset + 2] << 16;
	tmp |= pcan->cd.uc[offset + 3] << 24;
	return tmp;
}
/*******************************************************************************
 * int packet_extract_format(char *pout, struct BLOCKPARSED* pbp);
 * @brief 	: Write extracted CAN msgs in hex/ascii gateway format
 * @param	: pout = pointer to output ascii string
 * @param	: pbp  = pointer to array of CAN msgs with some other stuff
 * @return	: Number of chars in output
*******************************************************************************/
uint8_t canseq = 0;	// Sequence number byte

int packet_extract_format(char *pout, struct BLOCKPARSED* pbp)
{
	int i,j;
	char* p = pout;
	uint8_t bin[18];	// Allow for max CAN msg length
	int binct;
	
	*p = 0;	// Null string, jic

	/* Rattle through all CAN msgs in this block */
	for (i = 0; i < pbp->n; i++)
	{
// Debug--test date/time position versus time sync msgs
//if ((pbp->can[i].id == 0x00400000) | (pbp->can[i].id == 0xE1000000))
{
		// Build an array of (binary) bytes
		bin[0] = canseq;	// Seq number
		bin[1] = pbp->can[i].id >> 0;	// CAN id
		bin[2] = pbp->can[i].id >> 8;
		bin[3] = pbp->can[i].id >> 16;
		bin[4] = pbp->can[i].id >> 24;
		bin[5] = pbp->can[i].dlc  >> 0; // Payload size

		binct = 6; // Seq|CANid|DLC: = 6 bytes

		// Copy payload
		if (bin[5] > 8) bin[5] = 0; // jic
		for (j = 0; j < bin[5]; j++)
		{
			bin[6+j] = pbp->can[i].cd.uc[j];
		}
		binct += bin[5];	// Add size of payload

		// Compute chksum using gateway routine
		bin[binct] = CANgenchksum(&bin[0], binct);

		// Convert binary to ascii/hex
		for (j = 0; j < (binct+1); j++)
		{
			p += sprintf(p,"%02X",bin[j]);
		}
		p += sprintf(p,"\n"); // Add newline to output string
		canseq += 1;
}	
	}
	return (p - pout);	// Return total number of chars in string
}
/*******************************************************************************
 * int packet_extract_first(struct PKTP *pktp, unsigned char* pblk);
 * @brief 	: Extract PID, find last packet in block
 * @param	: pointer to struct with packet count, pointer, and block pid
 * @param	: pblk--pointer to control block
 * @return	: 0 = OK
*******************************************************************************/

int packet_extract_first(struct PKTP *pktp, unsigned char* pblk)
{
	int i = 0;
	unsigned char* px = pblk + SDLOG_DATASIZE - 1;
	union BLOCK *pb = (union BLOCK *)pblk;

	/* Extract pid */
	pktp->pid = (pb->str.pid);

	/* Search back to first non-zero byte */	
	while ((*px ==0) && (i++ < SDLOG_DATASIZE)) px--;

	if (i >= SDLOG_DATASIZE) 
	{
		pktp->ct = i; return -2;
	}

	pktp->ct = *px;
	if ((*px > 250) || (*px < 1))
	{ // Here, error, ct is too big.
		  return -1;
	}

	pktp->p = px - *px;	// Point to beginning of packet
	
	return 0;
}
/*******************************************************************************
 * void packet_convert(struct CANRCVTIMBUF* pout, struct PKTP *pp);
 * @brief 	: convert packet to CAN type struct
 * @param	: pblk--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_convert(struct CANRCVTIMBUF* pout, struct PKTP *pp)
{
	pout->U.ull 	= *(unsigned long long*)pp->p;
	pout->R.id     	= *(unsigned int*)(pp->p + 8);
	pout->R.dlc    	= *(unsigned int*)(pp->p + 8 + 4);
	pout->R.cd.ull  = *(unsigned long long*)(pp->p + 8 + 4 + 4);
	return;	
}
