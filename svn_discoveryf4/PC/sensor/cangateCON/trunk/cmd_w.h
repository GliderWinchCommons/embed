/******************************************************************************
* File Name          : cmd_w.h
* Date First Issued  : 04/11/2018
* Board              : PC
* Description        : Details for handling command 'w' -- list float payload
*******************************************************************************/

#ifndef __CMD_W_PC
#define __CMD_W_PC

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
int cmd_w_init(char* p);
/* @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
void cmd_w_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/


#endif

