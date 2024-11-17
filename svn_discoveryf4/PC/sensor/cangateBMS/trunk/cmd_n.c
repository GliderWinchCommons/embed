/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_n.c
* Author	     : deh
* Date First Issued  : 09/20/2013
* Board              : PC
* Description        : Details for handling the 'n' command (list id's and msg ct/sec)
*******************************************************************************/
/*
This lifts & modifies some code from--
~/svn_sensor/sensor/co1_Olimex/trunk/p1_PC_monitor_can.c

*/

#include "cmd_n.h"
#include <time.h>
#include <string.h>
#include <stdint.h>

#define CAN_TIMESYNCX 0x00400000	// Time sync CAN messge

time_t t_prior;

static u32 cansyncid;
static void cmd_n_count(u32 id);

#define CMDNTBLSIZE	4096		// Number of unique ID|data_type allowed
static u32 idtbl[CMDNTBLSIZE];		// Table of message ids
static u32 cttbl[2][CMDNTBLSIZE];	// Two tables of counts associated with msg ids
static int idxLast = 0;			// Points to available unfilled entry.

static u32 idx_cmd_n_ct;
/* Index for double buffer that switches each second */
static u32 idx_cmd_n_ct;	// Index for double buffering can msg counts

/* 1 second timing versus counts between time messages */
static u32 timemode;	// 0 = timer, 1 = counts between time messages

/* Count payloads for a more accurate bus load percentage. */
#define CANBAUD 500000
static uint32_t dlcsum;
static uint32_t dlcsum_prev;
static uint32_t msg_inct;
static uint32_t msg_inct_prev;
/******************************************************************************
 * void cmd_n_init(char* p);
 * @brief 	: Reset 
*******************************************************************************/
void cmd_n_init(char* p)
{
	int i;
	if (p != 0)
	{
		timemode = 1;	// Use a time sync CAN msg for 1 sec ticks
		if (strlen(p) < 10)
		{ // Assume a default timesync CAN address
			cansyncid = CAN_TIMESYNCX;	// Default time sync CAN id
		}
		else
		{ // Here, hapless Op appears to have typed a CAN id
			sscanf( (p+1), "%x",&cansyncid);
		}
		printf("Time sync CAN address being used for 1 sec timing: 0X%08X\n\n",cansyncid);

	}
	else	
		timemode = 0;	// Use PC OS ticks

	idxLast = 0;	// Reset end-of-table (no id's in table)
	for ( i = 0; i < CMDNTBLSIZE; i++) // Zero out count table
	{
		cttbl[0][i] = 0;	cttbl[1][i] = 0;
	}

	return;
}
/******************************************************************************
 * void cmd_n_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output current msg ID's ('n' command)
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'n' has been
turned on by the hapless Op typing 'n' as the first char and hitting return.
Command 'u' sets the mode to use the CAN time messages for making the 1 second
determination.
*/
void cmd_n_do_msg(struct CANRCVBUF* p)
{
	int i;
//	u8 *pc;
	int b;
	u32 totalct = 0;
	time_t t_now;
	time(&t_now);
	int sw = 0;
	unsigned int tmp;

//#define TESTFORTOTALCOUNTONLY
#ifdef TESTFORTOTALCOUNTONLY
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
static u32 idctr = 0;
static u32 throttle = 0;

{
	idctr += 1;
	if (p->id == 0x03000000)
	{
		
		throttle += 1;
		if (throttle >= 64)
		{
			printf (" %d\n\r",idctr);
			idctr = 0;
			throttle = 0;
		}
	}
	return;
}
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
#endif

//printf("%08X\n",p->id);

	/* The following detects 1 second demarcation has occured. */
	if (timemode == 0)
	{ // Here, system timer is used
		if (t_now != t_prior)	
		{ // Here, this is a new second
			sw = 1;
		}
	}
	else
	{ // Here we are using CAN time messages to mark the 1 sec point
		if (p->id == cansyncid)
		{
			if ((p->cd.us[0] & 0x3f) == 0) // Check the 1/64th sec ticks in the time payload
			{
				sw = 1;
			}
		}
	}

	if (sw == 1)
	{ // Here, this is a new second
		t_prior = t_now;
//printf("%08x\n",p->id);
		{ // Here, this starts the second
			b = (idx_cmd_n_ct & 0x01); // Index to buffer not currently being filled
			idx_cmd_n_ct += 1;		// Switch buffers

			/* Summary once per second. */
			// Here yes.  Output summary
			if (idxLast == 0)
			{ // Here, there were no id's found
				printf("No id's so far\n");
			}
			else
			{ // Here, the table has some entries.
				printf ("%2u:",idxLast);	// Number of table entries
				for (i = 0; i < idxLast; i++)
				{ // List'em all.  Minimize number of wasted spaces on line
					tmp = idtbl[i];
					if ((tmp & ~0xffe00000) == 0)
					{
						printf("%03X", (tmp >> 20) );	// 11b id as 3 hex
					}
					else
					{
						printf("%08X", tmp );	// full id
					}
					printf(" %2u|",cttbl[b][i]);// msg count
					totalct += cttbl[b][i];	// Build total count
					cttbl[b][i] = 0;	// Zero out msag ct for next round
				}
			}
//			printf("%4u\n",totalct);
#if 0
/* Add: byte ct and utilization */
/* NOTE: baud rate is hard coded, e.g. 230400. */
static unsigned int debug_inct_prev = 0;
extern unsigned int debug_inct;
double x = (debug_inct - debug_inct_prev);
printf("%3u %5u %4.1f",totalct, (debug_inct - debug_inct_prev), (x*100)/2000000 );
//printf("%3u %4.1f",totalct, (x*100)/921600 ); // Total msg ct, pct bandwidth used
//extern u32 err_seq;
//extern u32 err_ascii;
//printf(" %u %u\n",err_seq, err_ascii);
printf("\n");
debug_inct_prev = debug_inct;
#endif
			int32_t i1 = (int32_t)(msg_inct - msg_inct_prev);
			int32_t i2 = (int32_t)(dlcsum - dlcsum_prev);
//printf("inct %4d dlcsum %5d ",i1,i2);
			double d3  = (i2 * 8); // Number of payload bits
			double d4  = (i1 * (64+11)) + d3; // Total number bits
//printf(" d3 %5.0f d4 %5.0f :",d3,d4);
			printf(":%4d %4.1f\n",i1, d4*(100.0/CANBAUD));
			msg_inct_prev = msg_inct;
			dlcsum_prev   = dlcsum;
		}
	}
	cmd_n_count(p->id);	// Build table and count msgs
	dlcsum += p->dlc;   // Count payload bytes
	msg_inct += 1;      // Count CAN msgs

	return;
}
/******************************************************************************
 * static void cmd_n_count(u32 id);
 * @brief 	: Build table of id|data_types and counts
 * NOTE: this is entered under interrupt from 'can_log_buff' in 'can_log.c'
*******************************************************************************/
static void cmd_n_count(u32 id)
{
	u32 *p = &idtbl[0];
	int i = 0;
	int b;

//	if (id == 0) return; // Debugging

	/* Get the current cttbl buffer index */
	b = (idx_cmd_n_ct & 0x01);

	/* Search table for this ID|data_type */
	while ( ( id != *p ) && (i < idxLast) ) {p++; i++;}

	if (i < idxLast) // Did we find the id in the table?
	{ // Here, yes, found.  Increment count for this ID|data_type
		cttbl[b][i] += 1;
	}
	else
	{ // Here, not found.  Add to table
		idtbl[i] = id; 	// Save the id
		cttbl[b][i] = 1;	// Start off with the 1st count
		/* Advance the idx to the next open table entry. */
		idxLast +=1; if (idxLast >= CMDNTBLSIZE) idxLast = CMDNTBLSIZE-1;
	}
	return;
}

