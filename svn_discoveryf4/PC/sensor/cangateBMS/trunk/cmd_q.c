/******************************************************************************
* File Name          : cmd_q.c
* Date First Issued  : 09/05/2018
* Board              : PC
* Description        : 
*******************************************************************************/

#include <string.h>
#include "cmd_q.h"

/* From cangate */
extern FILE *fpList1;	// canldr.c File with paths and program names
extern int fdp;		// File descriptor for input file
extern int fpList1sw;	// File open switch: 0 = not opened

/* CANID.sql database parsed */
#define CANIDSQLSIZE	512	// Max number of CAN ids
static struct CANIDSQL canidsql[CANIDSQLSIZE];

/* List of received CAN IDs w index in canidsql */
struct CANIDLIST canidlist[CANIDSQLSIZE];

struct CANIDSQL canidlistnotfound;


/* Some things */
static struct CANIDWK wk;

int cmd_q_initsw = 0;	// OTO initialization switch

static char* psql = "INSERT INTO CANID VALUES";

/******************************************************************************
 * static char* search_copy(char* pout, char* pin);
 * @brief 	: Search next field and copy
 * @param	: pout = pointer to output
 * @param	: pin = pointer to input, already pointing to '\''
 * @return	: pointer to ending '\''
*******************************************************************************/
static char* search_copy(char* pout, char* pin)
{
	char* ptmp;
	pin++;
	if (pin == NULL)
	{
		printf("pin was NULL\n");
		return NULL;
	}
	ptmp = strchr(pin,'\''); // Spin to field end
	if (ptmp == NULL)
	{
		printf("Quote search error\n");
	 	return ptmp;
	}
	strncpy(pout,pin,(ptmp-pin));
	*(pout+(ptmp-pin)) = 0; // Terminator
	ptmp++;
	if (*ptmp == 0) return NULL;
	ptmp = strchr(ptmp,'\''); // Spin to beginning of next field
	return ptmp;
}
/******************************************************************************
 * int cmd_q_init(char *p);
 * @brief 	: Initialize from CANID_INSERT.sql file
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/
int cmd_q_init(char *p)
{
	#define CANIDMAXLINESIZE 1024
	char buf[CANIDMAXLINESIZE];	// File input line buffer

	char* p1;
//	int i;
	int itmp;
	int j;

	printf("cmd_q: Identify msgs\n");

	/* Did cangate startup open the file? */
	if (fpList1sw == 0)
	{ // Here, no.  Not much we can do.
		printf("CANID_INSERT.sql, or name specified on command line, did not open when program was started\n");
		return -1;
	}

	/* Initialization */
	if (cmd_q_initsw == 0)
	{ // Here, nothing has been initialized

		wk.size = 0;	// Number of entries from CANID_INSERT.sql file
		wk.listsz = 0;	// Number of entries in list of CANIDs

		canidlistnotfound.id = 0xffffffff;
		strcpy(&canidlistnotfound.name[0], "NOT in sql table");
		strcpy(&canidlistnotfound.hex[0], "ffffffff");
		strcpy(&canidlistnotfound.type[0], "         ");
		strcpy(&canidlistnotfound.msg_fmt[0], "UNDEF       ");
		
		rewind(fpList1);
		while ( (fgets (&buf[0],CANIDMAXLINESIZE,fpList1)) != NULL)
		{
			itmp = strncmp(buf,psql,strlen(psql));
			if (itmp == 0)
			{ // Looks like a good line
				p1 = strchr(buf,'\'');// Spin to 1st field
				
				// Copy fields to struct
				p1 = search_copy(&canidsql[wk.size].name[0],p1);	
				p1 = search_copy(&canidsql[wk.size].hex[0],p1);
				p1 = search_copy(&canidsql[wk.size].type[0],p1);
				p1 = search_copy(&canidsql[wk.size].msg_fmt[0],p1);
				p1 = search_copy(&canidsql[wk.size].description[0],p1);

				// Make entries same length
				j = strlen(&canidsql[wk.size].msg_fmt[0]);
				if ((12-j) > 0)
					strncat(&canidsql[wk.size].msg_fmt[0],"             ", 12-j);

				// Convert hex CANID to binary
				sscanf(&canidsql[wk.size].hex[0],"%x",&canidsql[wk.size].id);
				wk.size += 1;
				if (wk.size > CANIDMAXLINESIZE)
				{
					printf("CANID.sql file exceeded buffer size: %d\n",wk.size);
					return -1;
				}
			}
		}
		printf("Command q: Size of sql file edited = %d\n",wk.size);
		for (int i = 0; i < wk.size; i++)
		{
			printf("%4d 0X%08X %s: %s: %s: %s\n",i,canidsql[i].id,canidsql[i].name,canidsql[i].hex,canidsql[i].type,canidsql[i].msg_fmt);
		}
		cmd_q_initsw = 1;	// Show initialization has been completed		
	}
	return 0;
}
/******************************************************************************
 * static void print_list(struct CANIDLIST* p);
 * @brief 	: Print a msg
 * @param	: p = pointer to element to printed
*******************************************************************************/
static void print_list(struct CANIDSQL* p)
{
	printf("0X%08X   %s\t %s\n",p->id,p->msg_fmt,p->name);
	return;
}
/******************************************************************************
 * void cmd_q_do_msg(struct CANRCVBUF* p);
 * @brief 	: Build table and print
*******************************************************************************/
uint32_t ctrx;

void cmd_q_do_msg(struct CANRCVBUF* p)
{
	int i;
	if (cmd_q_initsw == 0) return;

//if (p->id == 0x00000000) return; // Who sent this!

	for (i = 0; i < wk.listsz; i++)
	{
		if (p->id == canidlist[i].id)
		{
			return;  // CAN ID for this msg is already in list
		}
	}
	/* Here, CANID not in list */
	canidlist[wk.listsz].id = p->id;	// Add CAN id to list

if (p->id == 0x0) printf("\nZero id: 0X%08X\n",p->id);

	// Find CAN id in CANID database
	canidlist[wk.listsz].idx = -1;		// Default if not found
	for (i = 0; i < wk.size; i++)
	{
		if (canidsql[i].id == p->id)
		{
			canidlist[wk.listsz].idx = i;
			break;
		}
	}

	wk.listsz += 1;
	

	printf("\n\n");

	for (i = 0; i < wk.listsz; i++)
	{
		if (canidlist[i].idx < 0)
		{ // Here CANID not found in sql list
			printf("0X%08X       NOT in sql file\n",canidlist[i].id);
		}
		else
		{
			print_list(&canidsql[canidlist[i].idx]);			
		}
	}
	printf("TOTAL %d\n",wk.listsz);
	return;
}

