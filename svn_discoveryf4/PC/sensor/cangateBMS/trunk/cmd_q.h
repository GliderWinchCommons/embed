/******************************************************************************
* File Name          : cmd_q.h
* Date First Issued  : 12/27/2014
* Board              : PC
* Description        : CAN bus loader file: edit-check only
*******************************************************************************/

#ifndef __CMD_Q_PC
#define __CMD_Q_PC

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


/*
CREATE TABLE CANID 
(
CANID_NAME varchar(48) PRIMARY KEY,
CANID_HEX varchar(8) NOT NULL UNIQUE,
CANID_TYPE varchar(24) NOT NULL,
CAN_MSG_FMT varchar(16) NOT NULL,
DESCRIPTION varchar(128) NOT NULL UNIQUE
);
*/

#define CSSIZE_NAME 60
#define CSSIZE_HEX 16
#define CSSIZE_TYPE 64
#define CSSIZE_MSG_FMT 64
#define CSSIZE_DESCRIPTION 256
struct CANIDSQL
{
	unsigned int id;
	char name[CSSIZE_NAME+1];
	char hex[CSSIZE_HEX+1];
	char type[CSSIZE_TYPE+1];
	char msg_fmt[CSSIZE_MSG_FMT+1];
	char description[CSSIZE_DESCRIPTION+1];
};

struct CANIDWK
{
	int size;	// Number of entries in CANID.sql read in
	int listsz;	// Number of entries in unique CANIDs received

};

struct CANIDLIST
{
	unsigned int id;
	int idx;
};


/******************************************************************************/
int cmd_q_init(char* p);
/* @brief 	: edit-check CAN bus loader file
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/
void cmd_q_do_msg(struct CANRCVBUF* p);
/* @brief 	: Build table and print
*******************************************************************************/

#endif

