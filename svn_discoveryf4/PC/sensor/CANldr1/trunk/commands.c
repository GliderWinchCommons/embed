/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : commands.c
* Author	     : deh
* Date First Issued  : 09/19/2013
* Board              : PC
* Description        : Routines related to keyboard commands
*******************************************************************************/
/*
This takes care of dispatching keyboard commands.
02-06-2014 rev 191 Add 'h' cmd
*/

#include "commands.h"
#include "timer_thread.h"
#include "download.h"
extern int fpListsw;

u32 msg_sw;	// Command in effect
/* **************************************************************************************
 * void do_command_timeout(void);
 * @brief	: Main loop encountered a timeout
 * @param	: none as yet
 * ************************************************************************************** */
void do_command_timeout(void)
{
	switch (msg_sw)
	{
	case 'p': // Program load
//		cmd_p_timeout();
		break;

	case 261: // Command 'a' 
		break;

	default:  // Ignore all others
		break;
	}
	return;
}

/* **************************************************************************************
 * void do_command_keybrd(char* p);
 * @brief	: A line from the keyboard starts the execution of some command
 * @param	: p = pointer to \0 terminated string
 * ************************************************************************************** */
int s_cmd_onoff = 0;	// zero = don't send test msgs to CAN

void do_command_keybrd(char* p)
{
}
/* **************************************************************************************
 * void do_canbus_msg(struct CANRCVBUF* p);
 * @brief	: We arrive here with a msg from the CAN BUS
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_canbus_msg(struct CANRCVBUF* p)
{
	download_canbus_msg(p);
}

/* **************************************************************************************
 * void do_pc_to_gateway(struct CANRCVBUF* p);
 * @brief	: FOR TESTING ####
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_pc_to_gateway(struct CANRCVBUF* p)
{
	int i;
	printf("%08x %d ",p->id, p->dlc);
	for (i = 0; i < (p->dlc & 0xf); i++)
		printf("%02x ",p->cd.u8[i]);
	printf("\n");
	return;
}
/* **************************************************************************************
 * void do_printmenu(void);
 * @brief	: Print menu for the hapless Op
 * ************************************************************************************** */
void do_printmenu(void)
{

	printf("Control C to quit program\n");
	return;
}
/* **************************************************************************************
 * void do_command_timing(void);
 * @brief	: Timeout ticks
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_command_timing(void)
{
	return;
}
