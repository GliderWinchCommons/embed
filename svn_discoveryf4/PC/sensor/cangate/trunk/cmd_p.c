/******************************************************************************
* File Name          : cmd_p.c
* Date First Issued  : 10/09/2014
* Board              : PC
* Description        : Program load for one unit
*******************************************************************************/
/*
./mm && ./cangate /dev/ttyUSB2 ../test/testmsg2B.txt
cd ~/svn_discoveryf4/PC/sensor/cangate/trunk
./mm && ./cangate 127.0.0.1 32123 ../test/testmsg2B.txt

Terms:
block	- one flash block of the stm32, e.g. 1024 or 2048 bytes
segment	- contiguous memory addresses for binary bytes in image built from .srec


10-11/2014 rev 246 working w/o highflash block
*/

#include "cmd_p.h"

#include "parse.h"





/******************************************************************************
 * int cmd_p_init(char* p);
 * @brief 	: Get path/file from input file for program loading
 * @param	: fpList = file with path/file lines
 * @param	: 0 = init was OK; -1 = failed
*******************************************************************************/
/* Originally this was much bigger */
int cmd_p_init(char* p)
{
	int ret;


	return 0;
}

/******************************************************************************
 * static void cmd_p_do_msg(void);
 * @brief 	: Do program loading sequence: enters from timer (timer_thread.c)
 * @param	: fpList = file with test msgs
 * @param	: fd = file descriptor for serial port
*******************************************************************************/
static void cmd_p_do_msg(void)
{
	return;
}
/******************************************************************************
 * void cmd_p_do_msg1(struct CANRCVBUF* pcan);
 * @brief 	: Deal with incoming CAN msgs
 * @param	: pcan = pointer to struct with CAN msg
*******************************************************************************/
/* 
Note:  This routine is entered when there is a CAN msg.  When a msg arrives that
we expect, then the next step is taken.  The sequence is to send a CAN msg that will
result in a response.  When the response is received the next CAN msg is sent.  The 
'cmd_p_do_msg' routine above is entered when there are 'select' timeouts in the main
loop of 'cangate.c'.  These are 1 ms ticks.  When the count reaches a threshold the
loading sequence is aborted.
*/ 
void cmd_p_do_msg1(struct CANRCVBUF* pcan)
{
	return;
}

