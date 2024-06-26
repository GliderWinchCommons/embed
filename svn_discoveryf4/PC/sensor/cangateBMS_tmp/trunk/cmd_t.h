/******************************************************************************
* File Name          : cmd_t.h
* Date First Issued  : 02/03/2022
* Board              : PC
* Description        : Display BMS misc polled command responses
*******************************************************************************/

#ifndef __CMD_T_PC
#define __CMD_T_PC

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "common_can.h"	// Definitions common to CAN and this project.

/******************************************************************************/
int cmd_t_init(char* p);
/* @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
void cmd_t_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
int close_ncurses(void);
/* @brief 	: Clean up and close window
*******************************************************************************/

extern uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.

#endif

