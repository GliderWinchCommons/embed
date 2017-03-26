/******************************************************************************
* File Name          : cmd_c.h
* Date First Issued  : 12/03/2016
* Board              : PC
* Description        : Launch parameter transfer test
*******************************************************************************/

#ifndef __CMD_C_MC
#define __CMD_C_MC

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "common_can.h"	// Definitions common to CAN and this project.

/******************************************************************************/
void cmd_c_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
void pushbutton_pinsetup(void);
/* @brief 	: Setup PA0 pin setup
*******************************************************************************/



#endif

