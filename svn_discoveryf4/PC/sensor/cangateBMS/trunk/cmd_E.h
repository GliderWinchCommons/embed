/******************************************************************************
* File Name          : cmd_E.h
* Date First Issued  : 08/28/23
* Board              : PC
* Description        : Emc EMC/MMC'n' development
*******************************************************************************/

#ifndef __CMD_EMCN_PC
#define __CMD_EMCN_PC

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
int cmd_E_init(char* p);
/* @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
void cmd_E_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
extern uint32_t kaON;  // 0 = keep-alive is off; 1 = keep-alive = on.

#endif

