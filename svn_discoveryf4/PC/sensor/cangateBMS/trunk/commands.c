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

01/31/2022 many revisions
*/

#include "commands.h"
#include "timer_thread.h"
#include "cmd_a.h"
#include "cmd_b.h"
#include "cmd_c.h"
#include "cmd_C.h"
#include "cmd_d.h"
#include "cmd_e.h"
#include "cmd_E.h"
#include "cmd_f.h"
#include "cmd_h.h"
#include "cmd_i.h"
#include "cmd_j.h"
#include "cmd_k.h"
#include "cmd_l.h"
#include "cmd_m.h"
#include "cmd_n.h"
#include "cmd_p.h"
#include "cmd_q.h"
#include "cmd_r.h"
#include "cmd_s.h"
#include "cmd_t.h"
#include "cmd_v.h"
#include "cmd_w.h"

extern int fpListsw;
extern int cmd_q_initsw;

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

	/* This switch checks the 1st char of a keyboard entry for a command code. */
	switch(*p)
	{

	case 'a': // 'a' command (enable unit sendin ascii in CAN msgs & display)
		if (cmd_a_init(p) >= 0)
		msg_sw = 'a';
		break;

	case 'b': // 'd' command
		cmd_b_init(p);
		msg_sw = 'b';
		break;

	case 'd': // 'd' command
		if (cmd_d_init(p) >= 0)
			msg_sw = 'd';
		break;

	case 'C': // 'C' command
		if (cmd_C_init(p) >= 0)
			msg_sw = 'C';
		break;

	case 'e': // 'e' command
		if (cmd_e_init(p) >= 0)
			msg_sw = 'e';
		break;		

	case 'E': // 'E' command
		if (cmd_E_init(p) >= 0)
			msg_sw = 'E';
		break;		

	case 'f': // 'f' command: Display gps fix: lat/lon/ht
		if (cmd_f_init(p) >= 0) // negative return means invalid input
			msg_sw = 'f';
		break;

	case 'h': // 'h' command: histogram readout
		do_printmenu();	  // Nice display for the hapless Op.
		msg_sw = 'h';
		break;

	case 'i': // LEVELWIND: display hearteate: status & state
		msg_sw = 'i';
		break;

	case 'j': // LEVELWIND: send command & display retrieved reading
		cmd_j_init(p);
		msg_sw = 'j';
		break;

	case 'l': // Display time from time sync msg
		cmd_l_init(p);
		msg_sw = 'l';
		break;

	case 'n': // 'n' command (list all id's and number of msgs during 1 second)
		cmd_n_init(0);	// Display counts between timer 1 sec ticks
		msg_sw = 'n';
		break;

	case 'u': // 'n' command (list all id's and number of msgs during 1 second)
		cmd_n_init(p);	// Display counts between time messages
		msg_sw = 'n';
		break;

	case 'v': // 'v' command
		if (cmd_v_init(p) >= 0)
			msg_sw = 'v';
		break;				

	case 'w': // 'w' command (list a float payload for a canid)
		if (cmd_w_init(p) >= 0) // negative return means invalid input
			msg_sw = 'w';

		break;

	case 'm': // 'm' command (list msgs for the id entered)
		if (cmd_m_init(p) >= 0) // negative return means invalid input
			msg_sw = 'm';
		break;

	case 'p': // 'p' command (GEVCUr commands)
		cmd_p_init(p);	
		msg_sw = 'p';
		break;

	case 'q': // 'q' CAN bus loader file edit-check only.
		if (cmd_q_init(p) == 0)	// Run edit-check on file only.
//cmd_q_init(p);			
			msg_sw = 'q';
		break;

	case 'r': // 'r' command (RESET everyone)
		cmd_r_init(p);
		break;

	case 's': // CONTACTOR: send command & display retrieved reading

		cmd_s_init(p);
		msg_sw = 's';
		break;

	case 't': // 't' command
		if (cmd_t_init(p) >= 0)
			msg_sw = 't';
		break;		


	case 'k': // CONTACTOR: Keep-alive/command testing

		cmd_k_init(p);
		msg_sw = 'k';
		break;
static uint8_t menu_oto_sw = 0;
	case 'x': // 'x' cancels current command
//		cmd_a_do_stop();  // Disable ascii sending if enabled
		if (menu_oto_sw == 0)
		{
			menu_oto_sw = 1;
//			do_printmenu();	  // Nice display for the hapless Op.
		}
		timer_thread_shutdown(); // Cancel timer thread if running
		msg_sw = 'x';
		cmd_q_initsw = 0;	// OTO initialization switch
		kaON  = 0; // cmd_e timer by-pass
//		kaON1 = 0; // cmd_a timer by-pass
//		kaON2 = 0; // cmd_v timer by-pass
		close_ncurses(); // Command 't' window 
		break;

	case 'c': // 'c' requests & displays launch parameters
		cmd_c_init(p);
		msg_sw = 'c';
		break;

	default:
		printf("1st char not a command 0x%02x ASCII: %c\n",*p, *p);
		break;
	}
	return;
}
/* **************************************************************************************
 * void do_canbus_msg(struct CANRCVBUF* p);
 * @brief	: We arrive here with a msg from the CAN BUS
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_canbus_msg(struct CANRCVBUF* p)
{
	switch (msg_sw)
	{
	case 'a':
		cmd_a_do_msg(p);
		break;

	case 'b':
		cmd_b_do_msg(p);
		break;

	case 'c':
		cmd_c_do_msg(p);
		break;

	case 'C':
		cmd_C_do_msg(p);
		break;		

	case 'd':
		cmd_d_do_msg(p);
//		do_pc_to_gateway(p);	// Hex listing of the CAN msg
		break;

	case 'e':
		cmd_e_do_msg(p);
		break;		

	case 'E':
		cmd_E_do_msg(p);
		break;		

	case 'f':
		cmd_f_do_msg(p);		// Display fix
		break;

	case 'h':
//		cmd_h_do_msg(p);
		break;

	case 'i':
		cmd_i_do_msg(p);
		break;

	case 'j':
		cmd_j_do_msg(p);
		break;

	case 'l':
		cmd_l_datetime(p);
		break;

	case 'n':
		cmd_n_do_msg(p);
		break;

	case 'm':
		cmd_m_do_msg(p);
		break;

	case 'p':
		cmd_p_do_msg(p);
		break;

	case 's':
		cmd_s_do_msg(p);
		break;

	case 't':
		cmd_t_do_msg(p);
		break;			

	case 'k':
		cmd_k_do_msg(p);
		break;

	case 'q':
		cmd_q_do_msg(p);
		break;

	case 'v':
		cmd_v_do_msg(p);
		break;				

	case 'w':
		cmd_w_do_msg(p);
		break;

	default:
		break;
	}
	return;
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
	printf("a - Display selected BMS & CP CAN msgs: a<enter> for menu\n");
	printf("b - CONTACTOR: display polled msgs\n");
	printf("c - request & display launch parameters\n");
	printf("C - : Cooling function C<enter> for menu\n");	
	printf("d - BMS heartbeat\n\t"
				"d  - default (cell readings by cell number: CANID: B0201134)\n\t"
				"dc aaaaaaaa  (cell readings by cell number: CANID: aaaaaaaa\n\t"
				"dz - default (cell readings voltage sorted: CANID: B0201134)\n\t"
				"dz aaaaaaaa  (cell readings voltage sorted: CANID: aaaaaaaa\n\t"
				"ds -         (bms status: CANID: B0201134)\n\t"
				"ds aaaaaaaa  (bms status: CANID: aaaaaaaa)\n");

	printf("e - BMS polling msgs: e<enter> for menu\n");
	printf("E - EMCn (ELCON chging): E<enter> for menu\n");
	printf("f - display fix: (e.g. f<enter>, or f E2600000<enter>\n");
    printf("g - GEVCUr: command request to retrieve all readings\n");
	printf("h - This menu \n");
	printf("i - LEVELWIND: display status & state\n");
	printf("j - LEVELWIND: send command display response\n");
	printf("k - CONTACTOR: keep-alive\n\t"
				"ka - Begin sending keep-alive msgs\n\t"
				"kx - Stop keep-alive msgs\n\t"
				"k0 - disconnect\n\t"
				"k1 - connect\n\t"
				"k2 - reset\n");

	printf("l - list unix time/date in heartbeat time msgs (l e1000000)\n");
	printf("m - list msgs for id entered 'm xxxxxxxx (CAN ID as 8 hex digits)'\n");
	printf("n - list msg id's and msg ct during 1 sec (coarse computer timing)\n");
	printf("p - ELCON Charger: \n\t"
				"p  Set voltage and current to zero\n\t"
				"pd Display ELCON sends: (CANID: C7FA872E)\n\t"
				"pm Display msg => OTHERS SENT <= to ELCON: (CANID: C0372FA4)\n\t"
				"ps <vvv.v> <iii.i> Set voltage and current (CANID: C0372FA4)\n\t"
				"pj Set charger off bit to 1 \n\t"
				"po Set charger off bit to 0\n");

	printf("q - Identify received msgs from CANID.sql file\n");
	printf("r - send high priority RESET\n");
	printf("s - CONTACTOR: command request to retrieve all readings\n");
	printf("t - Display all cell voltages of battery string in fixed window\n");
	printf("u - list msg id's and msg ct between CAN 1 sec time mgs (e.g. u 00600000)\n");
	printf("w - list msgs float (wf), integer (wi), half (wh), BigEnd 2 byte int (wy), byte (wb), with payload byte offset, (wi1 E1800000)\n");
	printf("v - Version check (BMS) using CRC and CHKSUM\n");
	printf("x - cancel command\n");
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
