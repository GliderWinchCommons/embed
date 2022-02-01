/******************************************************************************
* File Name          : cmd_c.h
* Date First Issued  : 12/03/2016
* Board              : PC
* Description        : Details for handling the 'c' command (list msgs for the id entered)
*******************************************************************************/

#ifndef __CMD_C_PC
#define __CMD_C_PC

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
int cmd_c_init(char* p);
/* @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
void cmd_c_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/



#endif

