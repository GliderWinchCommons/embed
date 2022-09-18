-- CMD_CODES_INSERT.sql
-- 10/03/2016
--
-- Defines code numbers versus names for command
--  These commands are used for loading flash, updating
--  parameters, and retrieving readings
--   
--CMD_CODE_NAME varchar(24) PRIMARY KEY,
--
--CMD_CODE_NUMBER numeric(4) NOT NULL UNIQUE,
--
--DESCRIPTION4 varchar(128) NOT NULL UNIQUE
--

DELETE from CMD_CODES;

--                              CMD_CODE_NAME    CMD_CODE_NUMBER  DESCRIPTION4  (CODE in byte [0];   bytes 1-7 hold something else of dear value)
INSERT INTO CMD_CODES  VALUES ('LDR_SET_ADDR',      1,	'5 Set address pointer (not FLASH) (bytes 2-5):  Respond with last written address.');
INSERT INTO CMD_CODES  VALUES ('LDR_SET_ADDR_FL',   2,	'5 Set address pointer (FLASH) (bytes 2-5):  Respond with last written address.');
INSERT INTO CMD_CODES  VALUES ('LDR_CRC',           3,	'8 Get CRC: 4-7');
INSERT INTO CMD_CODES  VALUES ('LDR_ACK',           4,	'1 ACK: Positive acknowledge (Get next something)');
INSERT INTO CMD_CODES  VALUES ('LDR_NACK',          5,	'1 NACK: Negative acknowledge (So? How do we know it is wrong?)');
INSERT INTO CMD_CODES  VALUES ('LDR_JMP',           6,	'5 Jump: to address supplied (bytes 2-5)');
INSERT INTO CMD_CODES  VALUES ('LDR_WRBLK',         7,	'1 Done with block: write block with whatever you have.');
INSERT INTO CMD_CODES  VALUES ('LDR_RESET',         8,	'1 RESET: Execute a software forced RESET');
INSERT INTO CMD_CODES  VALUES ('LDR_XON',           9,	'1 Resume sending');
INSERT INTO CMD_CODES  VALUES ('LDR_XOFF',          10,	'1 Stop sending');
INSERT INTO CMD_CODES  VALUES ('LDR_FLASHSIZE',		11,	'1 Get flash size; bytes 2-3 = flash block size (short)');
INSERT INTO CMD_CODES  VALUES ('LDR_ADDR_OOB',		12,	'1 Address is out-of-bounds');
INSERT INTO CMD_CODES  VALUES ('LDR_DLC_ERR',		13,	'1 Unexpected DLC');
INSERT INTO CMD_CODES  VALUES ('LDR_FIXEDADDR',		14,	'5 Get address of flash with fixed loader info (e.g. unique CAN ID)');
INSERT INTO CMD_CODES  VALUES ('LDR_RD4',           15,	'5 Read 4 bytes at address (bytes 2-5)');
INSERT INTO CMD_CODES  VALUES ('LDR_APPOFFSET',		16,	'5 Get address where application begins storing.');
INSERT INTO CMD_CODES  VALUES ('LDR_HIGHFLASHH',	17,	'5 Get address of beginning of struct with crc check and CAN ID info for app');
INSERT INTO CMD_CODES  VALUES ('LDR_HIGHFLASHP',	18,	'8 Get address and size of struct with app calibrations, parameters, etc.');
INSERT INTO CMD_CODES  VALUES ('LDR_ASCII_SW',		19,	'2 Switch mode to send printf ASCII in CAN msgs');
INSERT INTO CMD_CODES  VALUES ('LDR_ASCII_DAT',		20,	'3-8 [1]=line position;[2]-[8]=ASCII chars');
INSERT INTO CMD_CODES  VALUES ('LDR_WRVAL_PTR',		21,	'2-8 Write: 2-8=bytes to be written via address ptr previous set.');
INSERT INTO CMD_CODES  VALUES ('LDR_WRVAL_PTR_SIZE',22,	'Write data payload size');
INSERT INTO CMD_CODES  VALUES ('LDR_WRVAL_AI',		23,	'8 Write: 2=memory area; 3-4=index; 5-8=one 4 byte value');
INSERT INTO CMD_CODES  VALUES ('LDR_SQUELCH',		24,	'8 Send squelch sending tick ct: 2-8 count');
INSERT INTO CMD_CODES  VALUES ('LDR_GET_RTC',		25,	'8 Send RTC register: 2 = reg number; 5-8 value');
INSERT INTO CMD_CODES  VALUES ('LDR_EOB',    		26,	'8 End of Block: 2-5 CRC or some value');
INSERT INTO CMD_CODES  VALUES ('LDR_EOF',    		27,	'8 End of Flash: 2-5 CRC or some value');
INSERT INTO CMD_CODES  VALUES ('LDR_DATA',    		28,	'8 1-7 bytes of data in payload');
INSERT INTO CMD_CODES  VALUES ('LDR_CHKSUM',  		29,	'8 Get checksum 4-7');

INSERT INTO CMD_CODES  VALUES ('CMD_GET_IDENT',		30,	'Get parameter using indentification name/number in byte [1]');
INSERT INTO CMD_CODES  VALUES ('CMD_PUT_IDENT',		31,	'Put parameter using indentification name/number in byte [1]');
INSERT INTO CMD_CODES  VALUES ('CMD_GET_INDEX',		32,	'Get parameter using index name/number in byte [1]');
INSERT INTO CMD_CODES  VALUES ('CMD_PUT_INDEX',		33,	'Put parameter using index name/number in byte [1]; parameter in [2]-[5]');
INSERT INTO CMD_CODES  VALUES ('CMD_REVERT',        34,	'Revert (re-initialize) working parameters/calibrations/CANIDs back to stored non-volatile values');
INSERT INTO CMD_CODES  VALUES ('CMD_SAVE',          35,	'Write current working parameters/calibrations/CANIDs to non-volatile storage');
INSERT INTO CMD_CODES  VALUES ('CMD_GET_READING',   36,	'Send a reading for the code specified in byte [1] specific to function');
INSERT INTO CMD_CODES  VALUES ('CMD_GET_READING_BRD',  37,	'Send a reading for the code specified in byte [1] for board; common to functions');
INSERT INTO CMD_CODES  VALUES ('CMD_LAUNCH_PARM_HDSHK',38,	'Send msg to handshake transferring launch parameters');
INSERT INTO CMD_CODES  VALUES ('CMD_SEND_LAUNCH_PARM', 39,	'Send msg to send burst of parameters');
INSERT INTO CMD_CODES  VALUES ('CMD_REQ_HISTOGRAM',    40, 'Request histogram: [1] ADC #: [2] # consecutive:[3]-[6] # DMA buffers accumuleted in bins');
INSERT INTO CMD_CODES  VALUES ('CMD_THISIS_HISTODATA', 41, 'Histogram data item: [1] ADC #:[2] bin # (0 - n), [3]-[6] bin count');

INSERT INTO CMD_CODES  VALUES ('CMD_CMD_TYPE1',		42,	'[1]-[7] format 1');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_TYPE2',		43,	'[1]-[7] format 2');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_TYPE3',		44,	'[1]-[7] format 3');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_HEARTBEAT', 45,	'[1]-[7] Heartbeat format specific to unit');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_TYPE4',     46,	'[1]-[7] format 4');

INSERT INTO CMD_CODES  VALUES ('CMD_CMD_SYS_RESET_ALL',166,	'0xA6: [0] Cause System Reset for all');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_SYS_RESET_CID',167,	'0xA7: [0] Cause System Reset for CAN ID sent in payload');
INSERT INTO CMD_CODES  VALUES ('CMD_CMD_SYS_RESET_EXT',168,	'0xA8: [0] Extend current loader wait duration for all, [1] = wait in 0.1 sec steps');