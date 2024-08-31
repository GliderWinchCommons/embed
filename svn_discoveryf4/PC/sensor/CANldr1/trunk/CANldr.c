/******************************************************************************
* File Name          : CANldr.c
* Date First Issued  : 09/08/2022
* Board              : PC
* Description        : CAN downloader
*******************************************************************************/
/*
 Hack of winchPC.c which is--
 Hack of canonical.c & gateway.c
 See p. 277  _Linux Application Development_ Johnson, Troan, Addison-Wesley, 1998

10/28/2012
  Hack of dateselect.c to provide selection of launch times stored int POD.

09/12/2013
  Hack of 'launchselect.c'

11/27/2013
  Hack of 'canldr.c'

09/08/2022
  Hack of cangateCON.c et al.  

Start hub-server and socat to communicate with CAN bus gateway: e.g.--
hub-socat_b 32123 /dev/ttyUSB2 2000000

For socket--
CANldr <IP addr> <port> <CAN node ID> <path for extended binary file>

For serial port/USB connection--
CANldr <serial port> <CAN node ID> <path for extended binary file>

compile and execute: e.g.--
cd ~/GliderWinchCommons/embed/svn_discoveryf4/PC/sensor/CANldr/trunk
e.g. Download CAN node with CAN ID 0xB0A00000 and extended .bin file--
./mm && ./CANldr 127.0.0.1 32123 B0A00000 ~/GliderWinchItems/BMS/bmsadbms1818/build/bms1818.xbin


*/
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

#include "../../../../../svn_common/trunk/common_can.h"
//#include "../../../../../svn_common/trunk/db/can_db.h"

#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "commands.h"
#include "cmd_c.h"
#include "download.h"

#include "sockclient.h"

/* Subroutine prototypes */
int CANsendmsg(struct CANRCVBUF* pin);

/* Buffer for PC msg */
#define PCMSGBUFSIZE	256
u8 PCmsg[PCMSGBUFSIZE];

uint32_t xbin_addr;
uint32_t xbin_size;
uint32_t xbin_crc;
uint32_t xbin_checksum;
int xbin_in_ct;

#define MAXXBIN (1024*1024)
uint8_t bin[MAXXBIN];

static struct PCTOGATEWAY pctogateway; // Receives de-stuffed incoming msgs from PC.
static struct CANRCVBUF canrcvbuf;
//static struct PCTOGATECOMPRESSED pctogatecompressed;

/* Program ends when (exit_flag != 0). */
uint8_t exit_flag; 
int8_t exit_code; // Code to be passed out

/* CAN node loader CAN ID passed in from command line */
uint32_t CANnodeid;

/* Keyboard key to break loop and exit */
#define CONTROLC 0x03	/* Control C input char */

#define NOSERIALCHARS	10000	/* usec to wait for chars to be received on serial port */

/* baudrate settings are defined in <asm/termbits.h>, which is
  included by <termios.h> */
//#define BAUDRATE B115200
	//char* baudratesetting = "115200";
//#define BAUDRATE B230400
//#define BAUDRATE B460800
//#define BAUDRATE B921600
#define BAUDRATE B2000000
	char* baudratesetting = "2000000";

/* Serial port definition */

#define SERIALPORT "/dev/ttyUSB0"	/*  */
// #define _POSIX_SOURCE 1 /* POSIX compliant source */
char * serialport_default = SERIALPORT;
char * serialport;

#define SIZESERIALIN		8192	/* Size of buffer for serial input */
#define SIZESERIALOUTBUFF 	4096	/* Size of buffer for serial chars that go out */

#define LINEINSIZE		4096	/* Size of input line from stdin (keyboard) */

struct LINEIN
{
	char b[LINEINSIZE];
	int	idx;
	int	size;
}lineinK,lineinS;

#define BUFSIZE	4096
char buf[BUFSIZE];

struct timeval tv;	/* time value for select() */

/* The following needs to have file scope so that we can use them in signal handlers */
struct termios pts;	/* termios settings on port */
struct termios sts;	/* termios settings on stdout/in */
struct termios pots;	/* saved termios settings */
struct termios sots;	/* saved termios settings */
struct sigaction sact;	/* used to initialize the signal handler */
fd_set	ready;		/* used for select */

//int fds;	/* file descriptor, serial */
int fdp;	/* port file descriptor */

int done;	/* Prorgram termination flag */
int filedone;	/* File closed flag */
int opensw;	/* 0 = file has not been opened; 1 = file is open */
int nofilesw;	/* 0 = file name specified; 1 = no file name on command line */
int goodfilesw;	/* 0 = no file was opened and written; 1 = yes we did it */

int	linect;

char vv[512];	/* General purpose char buffer */

FILE *fpOut;
FILE *fpList;		// Test msgs to be sent file
int fpListsw = 0; // 0 = no test msg file list
#define FILENAME_TESTMSGS 't'

FILE *fpXbin;

/* Sequence number checking */
u8 sequence_prev;

/* Debug */
u32 err_seq;
u32 err_ascii;

/* =========================== Assemble an input line =================================== */
int linecompleted(struct LINEIN *p, char c)
{ /* Assemble a full line */
	p->b[p->idx++] = c;	/* Store the char received */
	if (p->idx >= (LINEINSIZE-1)) p->idx--; /* Don't run off end of buffer */
	if ( c == '\n')	/* NL completes the line */
	{
		p->b[p->idx] = '\0';	/* Place null at end of line */
		p->size = p->idx;	/* Save length of string */
		p->idx = 0;		/* reset the index */
		return 1;		/* show we have a full line */
	}
	return 0;

}
/* ====================================================================================== */
void cmdline_arg_error(int argc)
{
	printf ("Command line error: too many arguments %i args, for example--\n./CANldr /dev/ttyUSB0 B0A00000"
		"~/GliderWinchItems/BMS/bmsadcbms1818/build/bms1818.xbin\nOr\n"
				".CANldr 127.0.0.1 32123 B0A00000 "
				"~/GliderWinchItems/BMS/bmsadcbms1818/build/bms1818.xbin\n",argc); 
	return;	
}
/* Restore original terminal settings on exit */
void cleanup_termios(int signal)
{
	tcsetattr(fdp, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	printf("\nwinchPC exit due to Control C\n");
	exit(0);
}
/* ************************************************************************************************************ */
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
/* ************************************************************************************************************ */
int main(int argc, char *argv[])
{
/* Misc */
	int	i,j;		/* The for loop variable of choice */
	int	select_timeout;
	int	xret;	// Subroutine return value
	int	port;	// Socket port number
	uint8_t socketsw = 0;
//	char xbuf[256];

printf ("\n########## CANldr ########## 09/08/2022\n");

printf("BAUDRATE is FIXED AT: %s baud\n",baudratesetting);


	fpListsw = 0; // 0 = no test msg file list opened

/* Debug: Command line arguments */
printf ("argc %u\n",argc);
for (i = 0; i < argc; i++)
printf("%d %s\n",i, argv[i]);

	if (argc < 3)
	{
		printf("### TOO FEW COMMAND LINE ARGUMENTS: %d\n",argc);
		cmdline_arg_error(argc);
		 return -1;
	}

	if ((argc == 5) && (strncmp(argv[1], "/dev/", 5) == 0))
	{
		printf("### First command line arg started with /dev/ but arg count is for a tcp/ip socket\n");
		cmdline_arg_error(argc);
		 return -1;
	}

	if ((argc == 4) && (strncmp(argv[1], "/dev/", 5) != 0))
	{ // Here, assume a serial port
		socketsw = 3;
		sscanf(argv[2],"%X",&CANnodeid);
	}

	if ((argc == 5) && (strncmp(argv[1], "/dev/", 5) != 0))
	{ // Here, assume a socket
		socketsw = 4;
		sscanf(argv[3],"%X",&CANnodeid);
	}
	printf("CANnodeid: 0x%08X\n",CANnodeid);

	/* Determine if it is a socket connection or uart */
	if (socketsw == 0) // uart?
	{ // Here, yes.
		serialport = argv[1];

	/* =============== Serial port setup ===================================================================== */
//         fd = open(serialport, O_RDWR | O_NOCTTY | O_NONBLOCK );
         fdp = open(serialport, O_RDWR);

	 	if (fdp < 0) 
	 	{
	 		perror(serialport); 
	 		printf ("Error: So sorry, the serial port did not open: %s\n",serialport); 
	 		return -1; 
	 	}

        tcgetattr(fdp,&pots); 		/* Get the current port configuration for saving */

		/* Modify the serial port settings */
		pts.c_lflag &= ~ICANON;	/* Turn off canonicalization, i.e. set raw mode (no chars are special) */
		pts.c_lflag &=~(ECHO | ECHOCTL | ECHONL); /* No local echo */
		pts.c_cc[VMIN] = 1; /* See page 307 */
		pts.c_cc[VTIME] = 0;
		/* Without the following two lines the remote system would see Return twice for each press */
		pts.c_oflag &= ~ONLCR;
		pts.c_iflag &= ~ICRNL;

		pts.c_cflag &= ~(CSIZE | PARENB);
		pts.c_cflag |= CS8;	// 8 bits
		pts.c_cflag |= CREAD;	// Enable reading
		pts.c_cflag |= CLOCAL;	// Disable waiting for modem lines

		/* No flow control */
		pts.c_cflag &= ~CRTSCTS;
		pts.c_iflag &= ~(IXON | IXOFF | IXANY);
		/* Speed */
		cfsetospeed(&pts, BAUDRATE);
		cfsetispeed(&pts, BAUDRATE);
	 
		/* Modify the local terminal settings */
		tcgetattr(STDIN_FILENO,&sts);	/* Current settings */
		sots = sts;			/* Save current settings */
		sts.c_iflag &= ~BRKINT;		/* Turn off break.  Only useful when serial port can send break */
		sts.c_iflag |= IGNBRK;		/* Ignore breaks */
		sts.c_lflag &= ~ISIG;		/* This keeps driver from sending SIGINTR, SIGQUIT, SIGTSTP when
						INTR, QUIT, or SUSP is received. */
		sts.c_cc[VMIN] = 1;
		sts.c_cc[VTIME] = 0;
		sts.c_lflag &= ~ICANON;
		sts.c_lflag &= ~(ECHO | ECHOCTL | ECHONL); /* No local echo */

		/* Set the signal handler to restore the old termios handler */
		/* Setup the signal handlers */
		sact.sa_handler = cleanup_termios;
		sigaction(SIGHUP, &sact, NULL);
		sigaction(SIGINT, &sact, NULL); /* User sent Control C */
		sigaction(SIGPIPE, &sact, NULL);
		sigaction(SIGTERM, &sact, NULL);

		/* With signal handlers setup we can now set the modifed termios settings */
		tcsetattr(fdp, TCSANOW, &pts);	/* TCSANOW = take effect immediately */
		tcsetattr(STDIN_FILENO, TCSANOW, &sts);
	/* =============== End Serial port setup ================================================================ */
	}
	else
	{ // Here, not a uart.  See if it looks like an ip address
	/* =============== tcp/ip socket setup ================================================================== */
		char* parg = argv[1];
		j = 0;
		for (i = 0; i < strlen(argv[1]); i++)
		{
			if (*parg++ == '.') j += 1;
		}
		if ( j != 3) {printf("1st arg doesn't look like /dev/tty, or ip address xxx.xxx.xxx.xxx -- %s %i\n", argv[1],j); return -1;}

		/* Here, the arguments should be: <ip> <port> <optional test file> */
		if (argc < 3){printf ("1st arg looks like an IP, but not enough arguments\n"); return -1;}

		/* Here, next argument should be the port number */
		sscanf(argv[2],"%i",&port);

		fdp = sockclient_connect(argv[1], port);
		if (fdp < 0)
		{ 
			printf("## SOCKET DID NOT OPEN ##: ip %s, port: %i\n",argv[1],port);
			printf("  Comands that use CAN msgs will not work. \n"
				"  Cmd q (edit .txt file) will work.\n\n");		
		}
		else
		{
			printf ("Socket opened OK.  IP: %s PORT: %i\n",argv[1], port);
		}
	}

	if ( (fpXbin = fopen (argv[socketsw],"rb")) == NULL)
	{
		printf ("\nXbin file did not open: %s\n",argv[socketsw]);
		exit (-1);
	}
	printf("Xbin opened: %s\n",argv[socketsw]);

	/* Get header that was prepended by crc-srec.c. */
	fread(&xbin_addr,    4,1,fpXbin);
	fread(&xbin_size,    4,1,fpXbin);
	fread(&xbin_crc,     4,1,fpXbin);
	fread(&xbin_checksum,4,1,fpXbin);

	printf("xbin_addr:     0x%08X\n",xbin_addr);
	printf("xbin_size:     0x%08X\n",xbin_size);
	printf("xbin_crc:      0x%08X\n",xbin_crc);
	printf("xbin_checksum: 0x%08X\n",xbin_checksum);

	/* Readin binary data to be sent to CAN node. */
	xbin_in_ct = 0;
	do
	{
		xret =fread(&bin[xbin_in_ct],1,1,fpXbin);
		xbin_in_ct += 1;
		if (xbin_in_ct >= MAXXBIN)
		{
			printf("XBIN readin too large!!!: %d\b",xbin_in_ct);
			exit(-3);
		}
	} while (xret != 0);

	xbin_in_ct -= 1;
	if (xbin_in_ct != xbin_size)
	{
		printf("##ERR: xbin file size not matching size in header: xbin_in_ct %d xbin_size %d\n",xbin_in_ct,xbin_size);
	}
	printf("xbin_in_ct   : 0x%08X %d\n",xbin_in_ct,xbin_in_ct);

	/* Size check. (Mostly for debugging) */
	rewind(fpXbin);
	fseek(fpXbin,0L, SEEK_END);
	printf("fseek size   : 0x%08X\n",(unsigned int)ftell(fpXbin));
	
	lineinK.idx = 0;lineinS.idx=0;	/* jic */

	/* Timer for detecting response failure from gateway/CAN BUS */
	select_timeout = 1000*10;	// 1000 usec (1 ms) timeout
	struct timeval TMDETECT = {0,select_timeout}; //  timeout
	struct timeval tmdetect;

	PC_msg_initg(&pctogateway);		// Initialize struct for a msg from gateway to PC
	pctogateway.mode_link = MODE_LINK;	// Set modes for routines that receive and send CAN msgs

	download_init();

	tmdetect = TMDETECT;		/* Refresh timeout timer */
/* ************************************************************************************************** */
	/* The following is endless until broken by ctl C */
/* ************************************************************************************************** */
	do
	{
		FD_ZERO(&ready);		/* Clear file descriptors */
		FD_SET(STDIN_FILENO, &ready); 	/* Add stdin to set */
		FD_SET(fdp, &ready);		/* Add serial port to set */
		tmdetect = TMDETECT;		/* Refresh timeout timer */
	
		select (fdp+1, &ready, NULL, NULL, &tmdetect);	/* Wait for something to happen */

//		cmd_k_timeout(); // If msg_sw == k, send keep-alive contactor msgs.

		/* Send again if we timed out waiting for an expected response */
		if ( !( (FD_ISSET(fdp, &ready)) || (FD_ISSET(STDIN_FILENO, &ready)) ) )
		{ // When no file descriptors are responsible, then it must have been the timeout expiring
		/* Sending test msgs to CAN if we opened a file with the list. */
			tmdetect = TMDETECT;		/* Refresh timeout timer */
//			cmd_c_do_msg(NULL);	// Poll cmd_c: launch parameter timeout detection
			download_time_chk();
		}

		/* Incoming bytes from CAN gateway, arriving via serial port (file descriptor: 'fdp'). */
		if (FD_ISSET(fdp, &ready))	/* Was the serial port in the select's return? */
		{ /* fdp has characters for us */
			xret = USB_PC_get_msg_mode(fdp, &pctogateway, &canrcvbuf); // Build msg from incoming bytes
			if ( xret != 0 ) // Did the above result in frame, or line, from the gateway?
			{ // Here yes.  We have a msg, but it might not be valid.
				if (xret == 1)
				{ // Here the msg had not errors.
					do_canbus_msg(&canrcvbuf);	// Do something appropriate to the command in effect (commands.c)

					/* Poll for outputting test msgs, timed/throttled by counting incoming CAN msgs. */
//					if (fpListsw > 0)
//					{
//						cmd_s_do_msg(fpList, fdp, &canrcvbuf);
//					}
				}
				else
				{ // Something wrong with the msg
					if (xret < -4) // Did msg complete, but compression returned an error?
					{ // Here, yes.
						printf("CANuncompress error (with and without -4 offset): %i %i\n", xret, xret + 4);
					}
					else
					{ // Here, msg did not complete correctly: xret = (-4 <= (return code) < 0)
						printf("\nPC_msg_get_msg_mode error: %i\n",xret);
						err_ascii += 1;					
printf("\n%s\n",&pctogateway.asc[0]);
					}
					/* Check for sequence error (even though msg might have had errors). */
					sequence_prev += 1;
					if (pctogateway.seq != sequence_prev)
					{
//						printf("Seq err-- new %u old %u\n",pctogateway.seq,sequence_prev);
						sequence_prev = pctogateway.seq;
						err_seq += 1;
					}
				}
				PC_msg_initg(&pctogateway);	// Initialize struct for a msg from gateway to PC
			}
		}

		/* Incoming data arriving from keyboard (STDIN). */
		if (FD_ISSET(STDIN_FILENO, &ready))		// Was the keyboard in the select's return ?
		{ /* standard input has characters for us */
			i = read (STDIN_FILENO, buf, BUFSIZE);	// Read one or more chars from keyboard
			if (i >= 1)				// If no chars then assume it is a EOF
			{
				for (j = 0; j < i; j++)	// Look at all the chars available
				{
					write(STDOUT_FILENO,&buf[j],1); // Echo input back to the Hapless Op

					if (buf[j] == CONTROLC) done = 1;		// Ctl C causes loop to break
					if (linecompleted(&lineinK,buf[j]) == 1)	// Did we get a new line?
					{ // Receiving a '\n' line completes a line.
						do_command_keybrd(&lineinK.b[0]);	
					}
				}
			}
			else
			{
				done = 1;
			}
		}


	} while ((done == 0) || (exit_flag == 0)); // End "do" when someone sets the 'done' switch to non-zero

	/* Restore original terminal settings and exit */
	tcsetattr(fdp, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	printf ("\nCANldr killed from keyboard\n");

	// exit_code lets a script calling this know to proceed
	exit(exit_code);	
}
/* **************************************************************************************
 * int CANsendmsg(struct CANRCVBUF* pin);
 * @brief	: Send CAN msg to gateway.
 * @param	: pin = pointer to struct that is ready to go
 * @return	: 0 = OK;
 * ************************************************************************************** */

//extern int fdp;		/* port file descriptor (cangate.c) */
int CANsendmsg(struct CANRCVBUF* pin)
{	
	int size;
	struct PCTOGATECOMPRESSED pctogate;
	u8 c[128];	// Framed and byte stuffed chars that go to the serial port

	/* Compress the msg if possible */
	if (CANcompress(&pctogate, pin) == 0)
	{ // Here, compressed.
		size = PC_msg_prep(&c[0], 128, &pctogate.cm[0], (int)pctogate.ct);
	}
	else
	{ // Here, extended address format
		size = PC_msg_prep(&c[0], 128, (u8*)pin, (int)sizeof(struct CANRCVBUF));
	}
	write(fdp,&c[0],size);	// Send it out the serial port.
int i; printf("CT:%d %d ",(int)pctogate.ct,size); for (i=0;i<size;i++) printf("%02x ",c[i]); printf("\n");

	return 0;
}



