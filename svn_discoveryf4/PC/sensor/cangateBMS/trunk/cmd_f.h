/******************************************************************************
* File Name          : cmd_f.h
* Date First Issued  : 05/13/2014
* Board              : PC
* Description        : Display gps fix: lat lon ht
*******************************************************************************/

#ifndef __CMD_F_PC
#define __CMD_F_PC

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
int cmd_f_init(char* p);
/* @brief 	: Set CAN id
 * @param	: p = pointer to keyboard input chars
 * @return	: 0 = OK; -1 = entry no good
*******************************************************************************/
void cmd_f_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output current msg ID's ('n' command)
*******************************************************************************/



#endif

