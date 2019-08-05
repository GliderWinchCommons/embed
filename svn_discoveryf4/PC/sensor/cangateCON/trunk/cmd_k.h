/******************************************************************************
* File Name          : cmd_s.h
* Date First Issued  : 12/10/2013
* Board              : PC
* Description        : Send test msgs to CAN gateway
*******************************************************************************/

#ifndef __CMD_K_PC
#define __CMD_K_PC

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
void cmd_k_do_msg(struct CANRCVBUF* p);
/* @brief 	: Send CAN msgs
 * @param	: p = pointer to CAN msg
*******************************************************************************/
int cmd_k_init(char* p);
/* @brief 	: Setup timing
 * @param	: p = pointer to keyboard input
*******************************************************************************/
void cmd_k_timeout(void);
/* @brief 	: Send keep-alive msg
*******************************************************************************/

#endif

