/******************************************************************************
* File Name          : cmd_i.h
* Date First Issued  : 11/13/2020
* Board              : PC
* Description        : LEVELWIND: display hearbeat: status & state
*******************************************************************************/

#ifndef __CMD_I_PC
#define __CMD_I_PC

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
void cmd_i_do_msg(struct CANRCVBUF* p);
/* @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/


#endif

