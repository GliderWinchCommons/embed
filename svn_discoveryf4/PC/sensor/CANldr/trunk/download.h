/******************************************************************************
* File Name          : download.h
* Date First Issued  : 09/09/2022
* Board              : PC
* Description        : CAN download 
*******************************************************************************/

#ifndef __DOWNLOAD
#define __DOWNLOAD

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
 void download_init(void);
/* @brief 	: Init stuff
 ******************************************************************************/
void download_canbus_msg(struct CANRCVBUF* p);
/* @brief 	: Incoming CAN msgs
 * param    : p = pointer to CAN msg struct
*******************************************************************************/
void download_time_chk(void);
/* @brief 	: Poll wait-for-response timeout
 * param    : p = pointer to CAN msg struct
*******************************************************************************/


#endif

