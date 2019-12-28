/******************************************************************************
* File Name          : cmd_p.h
* Date First Issued  : 11/21/2019
* Board              : PC
* Description        : GEVCUr - send keep-alive and commands
*******************************************************************************/

#ifndef __CMD_P_PC
#define __CMD_P_PC

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
void cmd_p_do_msg(struct CANRCVBUF* p);
/* @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/
int cmd_p_init(char* p);
/* @brief 	: Setup timing
 * @param	: p = pointer to keyboard input
*******************************************************************************/
void cmd_p_timeout(void);
/* @brief 	: Send keep-alive msg
*******************************************************************************/

#endif

