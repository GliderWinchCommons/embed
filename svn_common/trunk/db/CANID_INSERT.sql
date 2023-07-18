-- CANID table associates a CANID "name" with a hex value and usage type
-- 07/10/2015
--
-- CANID_NAME is used rather than a hex value, allowing changes to hex assignments without changing a program.
-- CANID_NAME varchar(48) PRIMARY KEY,
--  Each CAN ID is given a unique name.  A Java program produces a .h file with #define that connects the name with 
--  the hex code.  Programs are written using the name so that if there are changes in the hex code the source
--  code does not need to be changed and only recompiling is required.
--
--  Naming convention:
--  Prefix: CANID_
--  2nd group: 
--    MSG = Response to a poll
--    CMD = Function instance Command
--       Suffix: I or R (interrogate or response) 
--    HB  = Heartbeat (autonomous sending)
--    variable = Function instance name
--  Suffix: R = fucntion responds/sends (to poll or command)
--  Suffix: I = Incoming to function (usually poll or command)
--
-- CANID_HEX varchar(8) NOT NULL UNIQUE,
--  This is the hex code for the CAN ID left justified as defined in the STM32 Reference Manual, and as follows--
--
--  Bits in 32b word for CAN ID--
--
--   If a standard 11 bit CAN ID:
--      31:21 And, IDE (bit 2) is zero
--   If an extended CAN ID:
--      31:03 And, IDE (bit 2) is one
--
--   Lower value CAN IDs have higher CAN priority
--
--   Bit 1 RTR: Remote transmission request
--      0: Data frame
--      1: Remote frame
--
--   Bit 0 Reserved.  Used for triggering hardware to xmit
--
-- CANID_IR varchar(2) NOT NULL,
--  Designate a response or interrogate CAN ID
--  I = interrogate
--  R = response
--  ' ' = no designation
--
-- CANID_TYPE varchar(24) NOT NULL,
--  (11/3/2020) new meaning: CANID_TYPE = "function"
--  E.g. TENSION, CPSWSV1 (Control Panel Switches Version 1)
--
-- CANID_INST NUMERIC,
--  Instance number of a "function"
--  Instances: 0 = undefined, 1 = first, ...
--
-- CANID INFUNC NUMERIC,
--  Within a CANID_TYPE (i.e. "function") there are multiple
--    CAN msgs. E.g. heartbeat, abd polled response
--    0 - undefined, 1 = first, ...
--
-- CAN_MSG_FMT varchar(16) NOT NULL,
--  PAYLOAD_TYPE_NAME: Name for payload layout
--  'UNDEF' is valid name for "not defined" (i.e. default)
--
-- DESCRIPTION varchar(128) NOT NULL UNIQUE
--  Clever words for further eludication and identification (must be unique).  Since the description field is unique
--  a search can be made using the field.
--
-- Rebuild the table by--
-- Delete all the current values:
DELETE from CANID;
-- Then add everything that follows:                               CANID_INST CANID_INFUNC
--                         CANID_NAME                 CANID_HEX  CANID_TYPE     CAN_MSG_FMT     DESCRIPTION
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_0',	'48000000', 'TENSION_a',1,0, 'U8_FF','Tension_0: Default measurement canid');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a11',	'38000000', 'TENSION_a',2,0, 'U8_FF','Tension_a11: Drum 1 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a21',	'38200000', 'TENSION_a',3,0, 'U8_FF','Tension_a12: Drum 1 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a12',	'38400000', 'TENSION_a',4,0, 'U8_FF','Tension_a21: Drum 2 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a22',	'38600000', 'TENSION_a',5,0, 'U8_FF','Tension_a22: Drum 2 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_2',	'38800000', 'TENSION_2',1,0, 'U8_FF','Tension_2: calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a1G',	'F800043C', 'TENSION_a',6,0, 'U8_FF','Tension_a1G: Drum 2 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a2G',	'F800044C', 'TENSION_a',7,0, 'U8_FF','Tension_a2G: Drum 2 calibrated tension, polled by time msg');

INSERT INTO CANID VALUES ('CANID_TST_TENSION_a11',	'F800010C', 'TENSION_a',8,0, 'U8_FF','Tension_a11: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a12',	'F800020C', 'TENSION_a',9,0, 'U8_FF','Tension_a12: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a21',	'F800030C', 'TENSION_a',10,0,'U8_FF','Tension_a21: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a22',	'F800040C', 'TENSION_a',11,0,'U8_FF','Tension_a22: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a1G',	'F800041C', 'TENSION_a',12,0,'U8_FF','Tension_a1G: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a2G',	'F800042C', 'TENSION_a',13,0,'U8_FF','Tension_a2G: TESTING java program generation of idx_v_val.c');
--
-- Command CAN IDs for each function
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1WI',	'05C0003C', 'TENSION_a',14,0,'U8_U8_U32','Tension_a: I 1W Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1WR',	'05C0803C', 'TENSION_a',15,0,'U8_U8_U32','Tension_a: R 1W Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2WI',	'05C0004C', 'TENSION_a',16,0,'U8_U8_U32','Tension_a: I 2W Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2WR',	'05C0804C', 'TENSION_a',17,0,'U8_U8_U32','Tension_a: R 2W Command code: [0] command code, [1]-[8] depends on code');

INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a11I',	'05C00004', 'TENSION_a',18,0,'U8_U8_U32','Tension_a11: I Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a11R',	'05C0000C', 'TENSION_a',19,0,'U8_U8_U32','Tension_a11: R Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a21I',	'05E00004', 'TENSION_a',20,0,'U8_U8_U32','Tension_a21: I Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a21R',	'05E0000C', 'TENSION_a',21,0,'U8_U8_U32','Tension_a21: R Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a12I',	'F800005C', 'TENSION_a',22,0,'U8_U8_U32','Tension_a12: I 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a12R',	'F800805C', 'TENSION_a',23,0,'U8_U8_U32','Tension_a12: R 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a22I',	'F800006C', 'TENSION_a',24,0,'U8_U8_U32','Tension_a22: I 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a22R',	'F800806C', 'TENSION_a',25,0,'U8_U8_U32','Tension_a22: R 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a0XI',	'F800001C', 'TENSION_a',26,0,'U8_U8_U32','Tension_a:  I 1 AD7799 VE POD TESTING (hence 0) 0' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a0XR',	'F800801C', 'TENSION_a',27,0,'U8_U8_U32','Tension_a:  R 1 AD7799 VE POD TESTING (hence 0) 0' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1XI',	'F800002C', 'TENSION_a',28,0,'U8_U8_U32','Tension_a:  I 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1XR',	'F800802C', 'TENSION_a',29,0,'U8_U8_U32','Tension_a:  R 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2XI',	'F800003C', 'TENSION_a',30,0,'U8_U8_U32','Tension_a2: I 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2XR',	'F800803C', 'TENSION_a',31,0,'U8_U8_U32','Tension_a2: R 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1YI',	'F800004C', 'TENSION_a',32,0,'U8_U8_U32','Tension_a:  I 1 AD7799 VE POD TESTING (hence Y) 2' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1YR',	'F800804C', 'TENSION_a',33,0,'U8_U8_U32','Tension_a:  R 1 AD7799 VE POD TESTING (hence Y) 2' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1GI',	'F800007C', 'TENSION_a',34,0,'U8_U8_U32','Tension_a:  I 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1GR',	'F800807C', 'TENSION_a',35,0,'U8_U8_U32','Tension_a:  R 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2GI',	'F800008C', 'TENSION_a',36,0,'U8_U8_U32','Tension_a2: I 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2GR', 'F800808C', 'TENSION_a',37,0,'U8_U8_U32','Tension_a2: R 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_2I',   '05C0005C', 'TENSION_2',2,0, 'U8_U8_U32','Tension_2: I Tension_2: Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_2R',   '05C0805C', 'TENSION_2',3,0, 'U8_U8_U32','Tension_2: R Tension_2: Command code: [0] command code, [1]-[8] depends on code');

INSERT INTO CANID VALUES ('CANID_HB_TENSION_0',    'E0400000','TENSION_0',1,1, 	'U8_FF','Tension_0: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a11',  'E0600000','TENSION_a',1,1, 	'U8_FF','Tension_a11: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a21',  'E0C00000','TENSION_a',1,1, 	'U8_FF','Tension_a21: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a12',  'E0800000','TENSION_a',1,1, 	'U8_FF','Tension_a12: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a22',  'E0E00000','TENSION_a',1,1, 	'U8_FF','Tension_a22: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a1G',  'E0E20000','TENSION_a',1,1, 	'U8_FF','Tension_a1G: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a2G',  'E0E40000','TENSION_a',1,1, 	'U8_FF','Tension_a2G: Heartbeat');

INSERT INTO CANID VALUES ('CANID_MOTOR_1',          '2D000000','MOTOR_1',1,0,'UNDEF','MOTOR_1: Motor speed');

INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_0R',	  '0600000C','CABLE_ANGLE_0',1,1,'UNDEF','Cable_Angle_0: R Default measurement canid');
INSERT INTO CANID VALUES ('CANID_MSG_CABLE_ANGLE_1',	  '3A000000','CABLE_ANGLE_1',2,1,'UNDEF','Cable_Angle_1: for drum #1');
INSERT INTO CANID VALUES ('CANID_MSG_CABLE_ANGLE_1_ALARM','2B000000','CABLE_ANGLE_1',2,2,'UNDEF','Cable_Angle_1: unreliable for drum #1');
INSERT INTO CANID VALUES ('CANID_HB_CABLE_ANGLE_1',       'E0A00000','CABLE_ANGLE_1',2,3, 'UNDEF','Cable_Angle_1: Heartbeat');
INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_1R',	  '0620000C','CABLE_ANGLE_1',2,4,'UNDEF','Cable_Angle_1: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_0I',	  '06000000','GENCMD',1,12,'UNDEF','Cable_Angle0: I Default measurement canid');
INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_1I',	  '06200000','GENCMD',1,13,'UNDEF','Cable_Angle1: I [0] command code, [1]-[8] depends on code');


-- Engine with manifold pressure, rpm, throttle, temperature combined as one function
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_I'          ,'80600000','ENGINE_SENSOR',1,0,'UNDEF'  ,'Engine: code: I [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_R'          ,'80800000','ENGINE_SENSOR',2,0,'UNDEF'  ,'Engine: code: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_MSG_ENG1_RPMMANFLD_R','40600000','ENGINE_SENSOR',3,0,'FF_FF'  ,'Engine: rpm:manifold pressure');
INSERT INTO CANID VALUES ('CANID_MSG_ENG1_TEMPTHRTL_R','60600000','ENGINE_SENSOR',4,0,'FF_FF'  ,'Engine: temperature:throttle pot, calibrated');
INSERT INTO CANID VALUES ('CANID_HB_ENG1_RPMMANFLD'   ,'4060000C','ENGINE_SENSOR',5,0,'U32_U32','Engine: raw readings: rpm:manifold pressure');
INSERT INTO CANID VALUES ('CANID_HB_ENG1_THRMTHRTL'   ,'6060000C','ENGINE_SENSOR',6,0,'U32_U32','Engine: raw readings: thermistor:throttle');

-- Engine MANIFOLD PRESSURE: with manifold pressure, rpm, throttle, temperature as separate functions
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_MANFLDI' ,'80A00000','ENG_MANIFOLD',1,1,'UNDEF','Eng1_manfld: code: I [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_MANFLDR' ,'80E00000','ENG_MANIFOLD',1,2,'UNDEF','Eng1_manfld: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_MSG_ENG1_MANFLD'  ,'40800000','ENG_MANIFOLD',1,3,'U8_FF','Eng1_manfld: manifold pressure');
INSERT INTO CANID VALUES ('CANID_HB_ENG1_MANFLD'   ,'82200000','ENG_MANIFOLD',1,4,'U8_FF','Eng1_manfld: Heartbeat manifold pressure calibrated');

-- Engine RPM: with manifold pressure, rpm, throttle, temperature as separate functions
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_RPMI'    ,'81200000','ENG_RPM',1,1,'UNDEF','Eng1_rpm: cmd code: I [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_RPMR'    ,'81400000','ENG_RPM',1,2,'UNDEF','Eng1_rpm: cmd code: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_MSG_ENG1_RPM'     ,'40A00000','ENG_RPM',1,3,'U8_FF','Eng1_rpm: RPM calibrated');
INSERT INTO CANID VALUES ('CANID_HB_ENG1_RPM'      ,'82600000','ENG_RPM',1,4,'U8_FF','Eng1_rpm: Heartbeat rpm calibrated');

-- Engine THROTTLE: with manifold pressure, rpm, throttle, temperature as separate functions
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_THROTTLEI','81600000','ENG_THROTTLE',1,1,'UNDEF','Eng1_throttle: cmd code: I [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_THROTTLER','81800000','ENG_THROTTLE',1,2,'UNDEF','Eng1_throttle: cmd code: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_MSG_ENG1_THROTTLE' ,'40E00000','ENG_THROTTLE',1,3,'U8_FF','Eng1_throttle: Throttle calibrated');
INSERT INTO CANID VALUES ('CANID_HB_ENG1_THROTTLE'  ,'82400000','ENG_THROTTLE',1,4,'U8_FF','Eng1_throttle: Heartbeat throttle calibrated');

-- Engine TEMPERATURE #1: with manifold pressure, rpm, throttle, temperature as separate functions
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_T1I'     ,'81A00000','ENG_T1',1,1,'UNDEF','Eng1_t1: cmd code: I [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_ENG1_T1R'     ,'81E00000','ENG_T1',1,2,'UNDEF','Eng1_t1: cmd code: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_MSG_ENG1_T1'      ,'41200000','ENG_T1',1,3,'U8_FF','Eng1_t1: Temperature #1 calibrated');
INSERT INTO CANID VALUES ('CANID_HB_ENG1_T1'       ,'82800000','ENG_T1',1,4,'U8_FF','Eng1_t1: Heartbeat temperature #1 calibrated');

-- GPS/Logger
INSERT INTO CANID VALUES ('CANID_HB_FIX_HT_TYP_NSAT','B1C00000','GPS',1,1,'UNDEF','GPS: fix: heigth:type fix:number sats');
INSERT INTO CANID VALUES ('CANID_HB_FIX_LATLON',     'A1C00000','GPS',1,2,'UNDEF','GPS: fix: lattitude:longitude');
INSERT INTO CANID VALUES ('CANID_HB_LG_ER1',         'D1C00004','GPS',1,3,'UNDEF','GPS: 1st code  CANID-UNITID_CO_OLI GPS checksum error');
INSERT INTO CANID VALUES ('CANID_HB_LG_ER2',         'D1C00014','GPS',1,4,'UNDEF','GPS: 2nd code  CANID-UNITID_CO_OLI GPS Fix error');
INSERT INTO CANID VALUES ('CANID_HB_LG_ER3',         'D1C00024','GPS',1,5,'UNDEF','GPS: 3rd code  CANID-UNITID_CO_OLI GPS Time out of step');
INSERT INTO CANID VALUES ('CANID_CMD_GPS_1R',        'D1C0004C','GPS',1,6,'UNDEF','GPS_1: R CANID Command GPS 1');
INSERT INTO CANID VALUES ('CANID_CMD_GPS_2R',        'D1C0007C','GPS',2,6,'UNDEF','GPS_2: R CANID Command GPS 2');
INSERT INTO CANID VALUES ('CANID_HB_GPS_LLH_1',      'E1C00000','GPS',1,7, 'LAT_LON_HT','GPS_1: Heartbeat (3 separate msgs) lattitude longitude height');
INSERT INTO CANID VALUES ('CANID_HB_GPS_LLH_2',      'E2600000','GPS',2,7, 'LAT_LON_HT','GPS_2: Heartbeat (3 separate msgs) lattitude longitude height');
INSERT INTO CANID VALUES ('CANID_CMD_GPS_1I',        'D1C00044','GENCMD',1,1,'UNDEF','GPS_1: I CANID Command GPS 1');
INSERT INTO CANID VALUES ('CANID_CMD_GPS_2I',        'D1C00074','GENCMD',1,2,'UNDEF','GPS_2: I CANID Command GPS 2');
--
INSERT INTO CANID VALUES ('CANID_HB_TIMESYNC',       '00400000','TIMESYNC',1,1,'U8'   ,'GPS_1: GPS time sync distribution msg');
INSERT INTO CANID VALUES ('CANID_HB_TIMESYNC_2',     '00600000','TIMESYNC',1,2,'U8'   ,'GPS_2: GPS time sync distribution msg');
INSERT INTO CANID VALUES ('CANID_HB_TIMESYNC_X',     '03000000','TIMESYNC',1,3,'UNDEF','GPS_2: Obsolete GPS time sync distribution msg');
INSERT INTO CANID VALUES ('CANID_HB_UNIVERSAL_RESET','00200000','TIMESYNC',1,4,'UNDEF','Highest priority: reserved for Universal (if/when implemented)');
--
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_1R', 'D1C0005C','LOGGER',1,1,'UNDEF', 'Logger_1: R Command Logger 1');
INSERT INTO CANID VALUES ('CANID_HB_LOGGER_1',   'E1800000','LOGGER',1,2,'U8_U32','Logger_1: Heartbeat logging ctr');
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_2R', 'D1C0006C','LOGGER',2,1,'UNDEF', 'Logger_2: R Command Logger 2');
INSERT INTO CANID VALUES ('CANID_HB_LOGGER_2',   'E1A00000','LOGGER',2,2,'U8_U32','Logger_2: Heartbeat logging ctr');
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_1I', 'D1C00054','GENCMD',1,3,'UNDEF', 'Logger_1: I Command Logger 1');
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_2I', 'D1C00064','GENCMD',1,4,'UNDEF', 'Logger_2: I Command Logger 2');
-- Sensor board reflective shaft encoder (../svn_sensor/sensor/shaft/trunk)
INSERT INTO CANID VALUES ('CANID_CMD_SHAFT1R'      ,'E2C00000', 'DRIVE_SHAFT',1,1,'UNDEF' ,'Shaft1: R Command CAN: repsonse');
INSERT INTO CANID VALUES ('CANID_MSG_SHAFT1_SPEED' ,'82A00000', 'DRIVE_SHAFT',1,2,'U8_FF' ,'Shaft1: Speed (RPM), polled msg');
INSERT INTO CANID VALUES ('CANID_MSG_SHAFT1_COUNT' ,'82E00000', 'DRIVE_SHAFT',1,3,'U8_U32','Shaft1: Cumulative count, polled msg');
INSERT INTO CANID VALUES ('CANID_HB_SHAFT1_SPEED'  ,'E2800000', 'DRIVE_SHAFT',1,4,'U8_FF' ,'Shaft1: Speed (RPM), hearbeat');
INSERT INTO CANID VALUES ('CANID_HB_SHAFT1_COUNT'  ,'E2A00000', 'DRIVE_SHAFT',1,5,'U8_U32','Shaft1: Cumulative count, hearbeat');
INSERT INTO CANID VALUES ('CANID_CMD_SHAFT1I'      ,'A0600000', 'GENCMD',1,5,'UNDEF','Shaft1: I Command CAN: incoming');
-- Sensor board reflective shaft encoder (../svn_sensor/sensor/se2/trunk)
INSERT INTO CANID VALUES ('CANID_SE2H_ADC3_HistB',   'D0800034', 'SHAFT_LOWERSHV',1,1,'UNDEF','Shaft encoder: Lower sheave:SE2: ADC3 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE2H_COUNTERnSPEED','30800000', 'SHAFT_LOWERSHV',1,1,'UNDEF','Shaft encoder: Lower sheave:SE2: (Lower sheave) Count and speed');
INSERT INTO CANID VALUES ('CANID_SE2H_ERR_1',        'D0800014', 'SHAFT_LOWERSHV',1,1,'UNDEF','Shaft encoder: Lower sheave:SE2: error1');
INSERT INTO CANID VALUES ('CANID_SE2H_ERR_2',        'D0800074', 'SHAFT_LOWERSHV',1,1,'UNDEF','Shaft encoder: Lower sheave:SE2: error2');
INSERT INTO CANID VALUES ('CANID_CMD_LOWERSHVR',     'D0800004', 'SHAFT_LOWERSHV',1,1,'UNDEF','Shaft encoder: Lower sheave:SE2: R Command CAN: send commands to subsystem');
INSERT INTO CANID VALUES ('CANID_CMD_LOWERSHVI',     'D0800000', 'GENCMD',1,6,'UNDEF','Shaft encoder: Lower sheave:SE2: I Command CAN: send commands to subsystem');
-- Sensor board reflective shaft encoder (../svn_sensor/sensor/se3/trunk)
INSERT INTO CANID VALUES ('CANID_SE3H_ADC2_HistA',   'D0A00044','SHAFT_UPPERSHV',1,1,'UNDEF','Shaft encoder: Upper sheave:SE3: ADC2 HistogramA tx:req ct, switch buff;rx send ct');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC2_HistB',   'D0A00054','SHAFT_UPPERSHV',1,2,'UNDEF','Shaft encoder: Upper sheave:SE3: ADC2 HistogramB tx: bin number,rx: send bin ct');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC3_ADC2_RD', 'D0A00064','SHAFT_UPPERSHV',1,3,'UNDEF','Shaft encoder: Upper sheave:SE3: ADC3 ADC2 readings readout');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC3_HistA',   'D0A00024','SHAFT_UPPERSHV',1,4,'UNDEF','Shaft encoder: Upper sheave:SE3: ADC3 HistogramA tx:req ct, switch buff.rx: send ct');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC3_HistB',   'D0A00034','SHAFT_UPPERSHV',1,5,'UNDEF','Shaft encoder: Upper sheave:SE3: ADC3 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE3H_COUNTERnSPEED','30A00000','SHAFT_UPPERSHV',1,6,'UNDEF','Shaft encoder: Upper sheave:SE3: (upper sheave) Count and Speed');
INSERT INTO CANID VALUES ('CANID_SE3H_ERR_1',        'D0A00014','SHAFT_UPPERSHV',1,7,'UNDEF','Shaft encoder: Upper sheave:SE3: error1');
INSERT INTO CANID VALUES ('CANID_SE3H_ERR_2',        'D0A00004','SHAFT_UPPERSHV',1,8,'UNDEF','Shaft encoder: Upper sheave:SE3: error2');
INSERT INTO CANID VALUES ('CANID_CMD_UPPERSHVR',     'D0600004','SHAFT_UPPERSHV',1,9,'UNDEF','Shaft encoder: Upper sheave:SE3: R Command CAN: send commands to subsystem');
INSERT INTO CANID VALUES ('CANID_CMD_UPPERSHVI',     'D0600000','GENCMD',1,7,'UNDEF','Shaft encoder: Upper sheave:SE3: I Command CAN: send commands to subsystem');
-- Sensor board reflective shaft encoder (../svn_sensor/sensor/se4/trunk)
INSERT INTO CANID VALUES ('CANID_SE4H_ADC2_HistA',   'D0400044','DRIVE_SHAFT',1,1,'UNDEF','Drive shaft: ADC2 HistogramA tx: request count, switch buffers; rx send count');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC2_HistB',   'D0400054','DRIVE_SHAFT',1,2,'UNDEF','Drive shaft: ADC2 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC3_ADC2_RD', 'D0400064','DRIVE_SHAFT',1,3,'UNDEF','Drive shaft: ADC3 ADC2 readings readout');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC3_HistA',   'D0400024','DRIVE_SHAFT',1,4,'UNDEF','Drive shaft: ADC3 HistogramA tx: request count, switch buffers. rx: send count');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC3_HistB',   'D0400034','DRIVE_SHAFT',1,5,'UNDEF','Drive shaft: ADC3 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_CMD_DRIVE_SHAFTR',  'D0C00004','DRIVE_SHAFT',1,6,'UNDEF','Drive shaft: R Command CAN: send commands to subsystem');
INSERT INTO CANID VALUES ('CANID_SE4H_COUNTERnSPEED','30400000','DRIVE_SHAFT',1,7,'UNDEF','Drive shaft: (drive shaft) count and speed');
INSERT INTO CANID VALUES ('CANID_SE4H_ERR_1',        'D0400014','DRIVE_SHAFT',1,8,'UNDEF','Drive shaft: [2]elapsed_ticks_no_adcticks<2000 ct  [3]cic not in sync');
INSERT INTO CANID VALUES ('CANID_SE4H_ERR_2',        'D0400004','DRIVE_SHAFT',1,9,'UNDEF','Drive shaft: [0]encode_state er ct [1]adctick_diff<2000 ct');
INSERT INTO CANID VALUES ('CANID_CMD_DRIVE_SHAFTI',  'D0C00000','GENCMD',1,8,'UNDEF','Drive shaft: I Command CAN: send commands to subsystem');
-- Discovery F4 sheave shaft_encoder.c, two high resolution rotary encoders
INSERT INTO CANID VALUES ('CANID_CMD_UPPER1_HR',   'D0E00004','SHEAVE_UP_H',1,1,'UNDEF','F4 shaft_encoder: Upper sheave hi-res: R Command CAN: subsystem responds');
INSERT INTO CANID VALUES ('CANID_MSG_UPPER1_H_RAW','D1000000','SHEAVE_UP_H',1,2,'U32'  ,'F4 shaft_encoder: hi-res: msg--upper raw count and delta time');
INSERT INTO CANID VALUES ('CANID_MSG_UPPER1_H_CAL','D1200000','SHEAVE_UP_H',1,3,'FF_FF','F4 shaft_encoder: hi-res: msg--upper calibrated--distance and speed');
INSERT INTO CANID VALUES ('CANID_HB_UPPER1_H_RAW', 'D1E00000','SHEAVE_UP_H',1,4,'U32'  ,'F4 shaft_encoder: hi-res: heartbeat--upper raw count and delta time');
INSERT INTO CANID VALUES ('CANID_HB_UPPER1_H_CAL', 'D2000000','SHEAVE_UP_H',1,5,'FF_FF','F4 shaft_encoder: hi-res: heartbeat--upper calibrated--distance and speed');
INSERT INTO CANID VALUES ('CANID_TST_SHEAVE_1UP',  'D2000004','SHEAVE_UP_H',1,6,'FF_FF','F4 shaft_encoder: hi-res: testing poll UP');
INSERT INTO CANID VALUES ('CANID_CMD_UPPER1_HI',   'D0E00000','GENCMD',1,9,'UNDEF','F4 shaft_encoder: Upper sheave hi-res: I Command CAN: send commands to subsystem');
--
INSERT INTO CANID VALUES ('CANID_CMD_LOWER1_HR',   'D1600004','SHEAVE_LO_H',1,1,'UNDEF','F4 shaft_encoder: Lower sheave hi-res: R Command CAN: subsystem responds');
INSERT INTO CANID VALUES ('CANID_MSG_LOWER1_H_RAW','D2400000','SHEAVE_LO_H',1,2,'U32'  ,'F4 shaft_encoder: hi-res: msg--lower raw count');
INSERT INTO CANID VALUES ('CANID_MSG_LOWER1_H_CAL','D1A00000','SHEAVE_LO_H',1,3,'FF_FF','F4 shaft_encoder: hi-res: msg--lower calibrated--distance and speed');
INSERT INTO CANID VALUES ('CANID_HB_LOWER1_H_RAW', 'D1800000','SHEAVE_LO_H',1,4,'U32'  ,'F4 shaft_encoder: hi-res: heartbeat--lower raw count');
INSERT INTO CANID VALUES ('CANID_HB_LOWER1_H_CAL', 'D1C00000','SHEAVE_LO_H',1,5,'FF_FF','F4 shaft_encoder: hi-res: heartbeat--lower calibrated--distance and speed');
INSERT INTO CANID VALUES ('CANID_TST_SHEAVE_1LO',  'D2200004','SHEAVE_UP_H',1,6,'FF_FF','F4 shaft_encoder: hi-res: testing poll LO');
INSERT INTO CANID VALUES ('CANID_CMD_LOWER1_HI',   'D1400000','GENCMD',1,10,'UNDEF','F4 shaft_encoder: Lower sheave hi-res: I Command CAN: send commands to subsystem');
-- Tilt
INSERT INTO CANID VALUES ('CANID_TILT_ALARM',  '00A00000','TILT_SENSE',1,1,'UNDEF','Tilt: alarm: Vector angle exceeds limit');
INSERT INTO CANID VALUES ('CANID_TILT_ANGLE',  '42E00000','TILT_SENSE',1,2,'UNDEF','Tilt: Calibrated angles (X & Y)');
INSERT INTO CANID VALUES ('CANID_TILT_XYZ',    '42800000','TILT_SENSE',1,3,'UNDEF','Tilt: Calibrated to angle: x,y,z tilt readings');
INSERT INTO CANID VALUES ('CANID_TILT_XYZ_CAL','FFFFFFCC','TILT_SENSE',1,4,'UNDEF','Tilt: CANID: Raw tilt ADC readings');
INSERT INTO CANID VALUES ('CANID_TILT_XYZ_RAW','4280000C','TILT_SENSE',1,5,'UNDEF','Tilt: Tilt:Raw tilt ADC readings');
INSERT INTO CANID VALUES ('CANID_CMD_TILTR',   '42C00004','TILT_SENSE',1,6,'UNDEF','Tilt: R Command CANID');
INSERT INTO CANID VALUES ('CANID_CMD_TILTI',   '42C00000','GENCMD',1,11,'UNDEF','Tilt: I Command CANID');
-- 
INSERT INTO CANID VALUES ('CANID_HB_GATEWAY1', 'E0200000','GATEWAY',1,1,'NONE','Gateway1: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_GATEWAY2', 'E1200000','GATEWAY',2,1,'NONE','Gateway2: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_GATEWAY3', 'E1400000','GATEWAY',3,1,'NONE','Gateway3: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_GATEWAY4', 'E2E00000','GATEWAY',4,1,'NONE','Gateway4: Heartbeat');
--
INSERT INTO CANID VALUES ('CANID_HB_GPS_TIME_1','E1000000','GPS',1,1,'UNIXTIME','GPS_1: Heartbeat unix time');
INSERT INTO CANID VALUES ('CANID_HB_GPS_TIME_2','E1E00000','GPS',2,1,'UNIXTIME','GPS_2: Heartbeat unix time');
--
--SERT INTO CANID VALUES ('CANID_HB_CANSENDER_1',    'F0200000', 'CANSENDER',1,1,'U8_U32',	'Cansender_1: Heartbeat w ctr');
--SERT INTO CANID VALUES ('CANID_HB_CANSENDER_2',    'F0400000', 'CANSENDER',1,1,'U8_U32',	'Cansender_2: Heartbeat w ctr');
--SERT INTO CANID VALUES ('CANID_CMD_CANSENDER_1R',  'A0200004', 'CANSENDER',1,1,'UNDEF',	'Cansender_1: R Command CANID');
--SERT INTO CANID VALUES ('CANID_CMD_CANSENDER_2R',  'A0400004', 'CANSENDER',1,1,'UNDEF',	'Cansender_2: R Command CANID');
--SERT INTO CANID VALUES ('CANID_POLL_CANSENDER',    'E2000000', 'CANSENDER',1,1,'U8_U32',	'Cansender: Poll cansenders');
--SERT INTO CANID VALUES ('CANID_POLLR_CANSENDER_1', 'E2200000', 'CANSENDER',1,1,'U8_U32',	'Cansender_1: Response to POLL');
--SERT INTO CANID VALUES ('CANID_POLLR_CANSENDER_2', 'E2400000', 'CANSENDER',1,1,'U8_U32',	'Cansender_2: Response to POLL');

INSERT INTO CANID VALUES ('CANID_CMD_PWRBOX1R'   ,'E3200000','PWRBOX',1,1,'U8_U32','Pwrbox1: R Command CANID');
INSERT INTO CANID VALUES ('CANID_HB_PWRBOX1'     ,'FF200000','PWRBOX',1,2,'U8_FF', 'Pwrbox1: Polled msg');
INSERT INTO CANID VALUES ('CANID_ALM_PWRBOX1'    ,'00800000','PWRBOX',1,3,'U8',    'Pwrbox1: Input voltage low alarm msg');
INSERT INTO CANID VALUES ('CANID_LVL_HB_PWRBOX1' ,'FF400000','PWRBOX',1,4,'U8_FF', 'Pwrbox1: Heartbeat: level wind motor current');
INSERT INTO CANID VALUES ('CANID_LVL_MSG_PWRBOX1','E3400000','PWRBOX',1,5,'U8_FF', 'Pwrbox1: Msg: level wind motor current');
INSERT INTO CANID VALUES ('CANID_CMD_PWRBOX1I'   ,'E320000C','GENCMD',1,16,'U8_U32','Pwrbox1: I Command CANID');
--
INSERT INTO CANID VALUES ('CANID_CMD_SANDBOX_1R','28E00004','SANDBOX_1',1,1, 'UNDEF','HC: SANDBOX_1: R Launch parameters');
INSERT INTO CANID VALUES ('CANID_CMD_SANDBOX_1I','28E00000','GENCMD'   ,1,17,'UNDEF','HC: SANDBOX_1: I Launch parameters');

--INSERT INTO CANID VALUES ('CANID_CMD_YOGURT_1I',	'29800000', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: I Yogurt maker parameters');
--INSERT INTO CANID VALUES ('CANID_CMD_YOGURT_1R',	'29800004', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: R Yogurt maker parameters');
--SERT INTO CANID VALUES ('CANID_MSG_YOGURT_1',		'29400000', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: Yogurt maker msgs');
--SERT INTO CANID VALUES ('CANID_HB_YOGURT_1',		'29600000', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: Heart-beats');
--
-- Bridging and Recomputation output addresses
INSERT INTO CANID VALUES ('CANID_MSG_METER_TORQ1','4080000C','BRIDGE1',1,1,'U8_FF','Bridge1: Eng Manifold recomputed');
INSERT INTO CANID VALUES ('CANID_MSG_ENG1_RPM2'  ,'40A0000C','BRIDGE1',1,2,'U8_FF','Bridge1: Eng RPM recomputed');
-- DMOC sends
INSERT INTO CANID VALUES ('CANID_DMOC_ACTUALTORQ','47400000','DMOC',1,1,'I16',       'DMOC: Actual Torque: payload-30000');
INSERT INTO CANID VALUES ('CANID_DMOC_SPEED',     '47600000','DMOC',1,1,'I16_X6',    'DMOC: Actual Speed (rpm?)');
INSERT INTO CANID VALUES ('CANID_DMOC_DQVOLTAMP', '47C00000','DMOC',1,1,'I16_I16_I16_I16','DMOC: D volt:amp, Q volt:amp');
INSERT INTO CANID VALUES ('CANID_DMOC_TORQUE',    '05683004','DMOC',1,1,'I16_I16',   'DMOC: Torque,-(Torque-30000)');
INSERT INTO CANID VALUES ('CANID_DMOC_CRITICAL_F','056837fc','DMOC',1,1,'NONE',      'DMOC: Critical Fault: payload = DEADB0FF');
INSERT INTO CANID VALUES ('CANID_DMOC_HV_STATUS', 'CA000000','DMOC',1,1,'I16_I16_X6','DMOC: HV volts:amps, status');
INSERT INTO CANID VALUES ('CANID_DMOC_HV_TEMPS',  'CA200000','DMOC',1,1,'U8_U8_U8',  'DMOC: Temperature:rotor,invert,stator');
-- DMOC receive commands
INSERT INTO CANID VALUES ('CANID_DMOC_CMD_SPEED', '46400000','MMC',1,1,'I16_X6',         'DMOC: cmd: speed, key state');
INSERT INTO CANID VALUES ('CANID_DMOC_CMD_TORQ',  '46600000','MMC',1,1,'I16_I16_I16_X6', 'DMOC: cmd: torq,copy,standby,status');
INSERT INTO CANID VALUES ('CANID_DMOC_CMD_REGEN', '46800000','MMC',1,1,'I16_I16_X_U8_U8','DMOC: cmd: watt,accel,degC,alive');
-- Logging data: messages sent for logging capture and analysis
INSERT INTO CANID VALUES ('CANID_LOG_DMOCCMDSPD', 'E4000000','GENCMD',1,18,'FF_FF','GEVCUr: Control law: Desired speed: integrator');
-- Master Controller
--INSERT INTO CANID VALUES ('CANID_MC_SYSTEM_STATE',   '50000000','MC',1,1,'U8_U8',   'MC: System state: U8 = high|low nibbles ');
INSERT INTO CANID VALUES ('CANID_MC_DRUM_SELECT',    'D0800814','MC',1,2,'UNDEF','MC: Drum selection');
INSERT INTO CANID VALUES ('CANID_MC_GUILLOTINE',     '22000004','MC',1,3,'UNDEF','MC: Fire guillotine');
INSERT INTO CANID VALUES ('CANID_MSG_TIME_POLL',     '20000000','MC',1,4,'UNDEF','MC: Time msg/Group polling');
INSERT INTO CANID VALUES ('CANID_MC_STATE',          '26000000','MC',1,5,'U8_U8','MC: Launch state msg');
INSERT INTO CANID VALUES ('CANID_MC_TORQUE',         '25800000','MC',1,6,'UNDEF','MC: Motor torque');
INSERT INTO CANID VALUES ('CANID_MC_REQUEST_PARAM',  'D0800824','MC',1,7,'UNDEF','MC: Request parameters from HC');
INSERT INTO CANID VALUES ('CANID_MC_RQ_LAUNCH_PARAM','27000000','MC',1,8,'UNDEF','MC: Request launch parameters');
INSERT INTO CANID VALUES ('CANID_CMD_MCLR',          'FFE00004','MC',1,9,'UNDEF','MC: Master Controller Launch parameters R (MC)');
INSERT INTO CANID VALUES ('CANID_CMD_MCLI',          'FFE00000','HC',1,1,'UNDEF','MCL: Master Controller Launch parameters I (HC)');
-- Control Panel
--INSERT INTO CANID VALUES ('CANID_CP_CTL_RMT',    '29000000','CP','UNDEF','Control Panel: Control lever remote');
--INSERT INTO CANID VALUES ('CANID_CP_CTL_LCL',    '29200000','CP','UNDEF','Control Panel: Control lever local');
--INSERT INTO CANID VALUES ('CANID_CP_CTL_IN_RMT', '24C00000','CP','UNDEF','Control Panel: Control lever remote: input');
--INSERT INTO CANID VALUES ('CANID_CP_CTL_IN_LCL', '25000000','CP','UNDEF','Control Panel: Control lever  local: input');
--INSERT INTO CANID VALUES ('CANID_CP_CTL_OUT_RMT','2A000000','CP','UNDEF','Control Panel: Control lever output');
--
-- Contactor Blue Pill Motherboard unit #1
INSERT INTO CANID VALUES ('CANID_HB_CNTCTR1V',  'FF800000','CNTCTR',1,1,'FF_FF',   'Contactor1: Heartbeat: High voltage1:Current sensor1');
INSERT INTO CANID VALUES ('CANID_HB_CNTCTR1A' , 'FF000000','CNTCTR',1,2,'FF_FF',   'Contactor1: Heartbeat: High voltage2:Current sensor2');
INSERT INTO CANID VALUES ('CANID_MSG_CNTCTR1V', '50400000','CNTCTR',1,3,'FF_FF',   'Contactor1: poll response: HV1:Current sensor1');
INSERT INTO CANID VALUES ('CANID_MSG_CNTCTR1A', '50600000','CNTCTR',1,4,'FF_FF',   'Contactor1: poll response: HV2 battery gnd to: DMOC+, HV3 DMOC-');
INSERT INTO CANID VALUES ('CANID_CMD_CNTCTR1R', 'E3600000','CNTCTR',1,5,'U8_VAR',  'Contactor1: R Command response');
INSERT INTO CANID VALUES ('CANID_CMD_CNTCTRKAR','E3C00000','CNTCTR',1,6,'U8_U8_U8','Contactor1: R KeepAlive response');
INSERT INTO CANID VALUES ('CANID_CMD_GEVCURKAR','E3E00000','CNTCTR',1,7,'U8_U8',   'GEVCUr: R KeepAlive response');
INSERT INTO CANID VALUES ('CANID_CMD_GEVCURKAI','E4200000','CNTCTR',1,8,'U8_S8_S8_S8_S8','GEVCUr: I KeepAlive and connect command');
INSERT INTO CANID VALUES ('CANID_CMD_CNTCTR1I', 'E360000C','GENCMD',1,19,'U8_VAR', 'Contactor1: I Command incoming');
INSERT INTO CANID VALUES ('CANID_CMD_CNTCTRKAI','E3800000','GENCMD',1,20,'U8',     'Contactor1: I KeepAlive and connect command');
-- GEVCUr Development & test of DMOCs w motors
-- Drum node (level wind stepper, brake solenoid and air pressure, drum speed and odometer)
INSERT INTO CANID VALUES ('CANID_HB_DRUM1',      'E4800000','DRUM', 1,2,   'U8_U32','DRUM1: U8:Status,U32: odometer (encoder ticks)');
INSERT INTO CANID VALUES ('CANID_MSG_DRUM1',     'E4C00000','DRUM', 1,3,   'FF_FF' ,'DRUM1: FF: Speed (m/s) FF: odometer (m)');

INSERT INTO CANID VALUES ('CANID_TST_LVLWIND'  , 'E4D00000','LEVELWIND',1,1,'S8_U8_U8','TEST DRUM1');
INSERT INTO CANID VALUES ('CANID_HB_LEVELWIND_1','80000000','LEVELWIND',1,2,'S8_U8','DRUM 1: S8:Status,U8:state');
INSERT INTO CANID VALUES ('CANID_HB_LEVELWIND_2','80200000','LEVELWIND',2,2,'S8_U8','DRUM 2: S8:Status,U8:state');
INSERT INTO CANID VALUES ('CANID_HB_LEVELWIND_3','80400000','LEVELWIND',3,2,'S8_U8','DRUM 3: S8:Status,U8:state');
INSERT INTO CANID VALUES ('CANID_HB_LEVELWIND_4','80600004','LEVELWIND',4,2,'S8_U8','DRUM 4: S8:Status,U8:state');
INSERT INTO CANID VALUES ('CANID_HB_LEVELWIND_5','80800004','LEVELWIND',5,2,'S8_U8','DRUM 5: S8:Status,U8:state');
INSERT INTO CANID VALUES ('CANID_HB_LEVELWIND_6','80A00004','LEVELWIND',6,2,'S8_U8','DRUM 6: S8:Status,U8:state');
INSERT INTO CANID VALUES ('CANID_HB_LEVELWIND_7','80C00000','LEVELWIND',7,2,'S8_U8','DRUM 7: S8:Status,U8:state');
-- General purpose command. Drum bits and commmand code determine functions and nature of response
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_I1','B1000014','GENCMD',1,23,'U8_U8_U8_X4','1 incoming: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_I2','B1000024','GENCMD',1,24,'U8_U8_U8_X4','2:incoming: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_I3','B1000034','GENCMD',1,25,'U8_U8_U8_X4','3:incoming: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_I4','B1000044','GENCMD',1,26,'U8_U8_U8_X4','4:incoming: U8:drum bits,U8:command code,X4:four byte value');
-- Response to command. X4 = undefined four byte type, e.g. uin32_t, int32_t, float
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_R1','B1000114','LEVELWIND',1,3,'U8_U8_U8_X4','1: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_R2','B1000124','LEVELWIND',2,3,'U8_U8_U8_X4','2: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_R3','B1000134','LEVELWIND',3,3,'U8_U8_U8_X4','3: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_R4','B1000144','LEVELWIND',4,3,'U8_U8_U8_X4','4: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_R5','B1000154','LEVELWIND',5,3,'U8_U8_U8_X4','5: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_R6','B1000164','LEVELWIND',6,3,'U8_U8_U8_X4','6: U8:drum bits,U8:command code,X4:four byte value');
INSERT INTO CANID VALUES ('CANID_CMD_LEVELWIND_R7','B1000174','LEVELWIND',7,3,'U8_U8_U8_X4','7: U8:drum bits,U8:command code,X4:four byte value');
--
INSERT INTO CANID VALUES ('CANID_TST_STEPCMD'  ,'E4600000','GENCMD', 1,4, 'U8_FF' ,'DRUM1: U8: Enable,Direction, FF: CL position');
INSERT INTO CANID VALUES ('CANID_HB_STEPPER'   ,'E4A00000','STEPPER',1,1, 'U8_U32','STEPPER: U8: Status, U32: stepper position accum');
-- Control Panel Switches: Version 1
INSERT INTO CANID VALUES ('CANID_HB_CPSWSV1_1','31000000','CPMC',    1,1,'S8_U8_7','HB_CPSWSV1 1: S8:status,U8[7]: status,switches,drum sel,operational,spare,spare');
INSERT INTO CANID VALUES ('CANID_HB_CPSWSV1_2','31200000','CPREMOTE',1,2,'S8_U8_7','HB_CPSWSV1 2: S8:status,U8[7]: status,switches,drum sel,operational,spare,spare');
INSERT INTO CANID VALUES ('CANID_HB_CPSWSV1_3','31400000','CPHC',    1,3,'S8_U8_7','HB_CPSWSV1 3: S8:status,U8[7]: status,switches,drum sel,operational,spare,spare');
INSERT INTO CANID VALUES ('CANID_HB_CPSWSV1_4','31600000','GENCMD',  1,4,'S8_U8_7','HB_CPSWSV1 4: S8:status,U8[7]: status,switches,drum sel,operational,spare,spare');

INSERT INTO CANID VALUES ('CANID_HB_CPSWSCLV1_1','31800000','CPMC',    2,1,'S8_S16_FF_V','HB_CPSWSV1 1:S8:status, S16 CL: (+/-10000 )');
INSERT INTO CANID VALUES ('CANID_HB_CPSWSCLV1_2','31A00000','CPREMOTE',2,2,'S8_S16_FF_V','HB_CPSWSV1 2:S8:status, S16 CL: (+/-10000 )');
INSERT INTO CANID VALUES ('CANID_HB_CPSWSCLV1_3','31C00000','CPHC',    2,3,'S8_S16_FF_V','HB_CPSWSV1 3:S8:status, S16 CL: (+/-10000 )');
INSERT INTO CANID VALUES ('CANID_HB_CPSWSCLV1_4','31E00000','GENCMD',  2,4,'S8_S16_FF_V','HB_CPSWSV1 4:S8:status, S16 CL: (+/-10000 )');
--     Brake function
INSERT INTO CANID VALUES ('CANID_CMD_BRAKE1I','22000000','GENCMD',1,21,'U8_FF','BRAKE1: I U8: command code, FF: braking force ');
INSERT INTO CANID VALUES ('CANID_HB_BRAKE1R' ,'A1000000','BRAKE' ,1,1, 'U8_FF','BRAKE1: R U8: tbd, FF: tbd ');
INSERT INTO CANID VALUES ('CANID_CMD_BRAKE2I','21400000','GENCMD',1,22,'U8_FF','BRAKE2: I U8: command code, FF: braking force ');
INSERT INTO CANID VALUES ('CANID_HB_BRAKE2R' ,'A1200000','BRAKE' ,2,1, 'U8_FF','BRAKE2: R U8: tbd, FF: tbd ');
-- BEEPERV1 function (V1 = first version, 1 = first instance)
INSERT INTO CANID VALUES ('CANID_CMD_BEEPERV1_1' ,'D3000000','BEEPERV1',1,1,'U8_U8_U8_S8_U16','BEEPERV1 1 MC: U8:cmd code, U8:ON, U8:OFF, S8:Chirp, U16: Freq');
INSERT INTO CANID VALUES ('CANID_CMD_BEEPERV1_2' ,'D3200000','BEEPERV1',1,2,'U8_U8_U8_S8_U16','BEEPERV1 2 CP sws: U8:cmd code, U8:ON, U8:OFF, S8:Chirp, U16: Freq');
INSERT INTO CANID VALUES ('CANID_CMD_BEEPERV1_3' ,'D3400000','BEEPERV1',1,3,'U8_U8_U8_S8_U16','BEEPERV1 3 Health: U8:cmd code, U8:ON, U8:OFF, S8:Chirp, U16: Freq');
-- BMS module node
-- Universal CAN msg: EMC_I = EMC sends; PC_I = PC sends; R = BMS responds;
INSERT INTO CANID VALUES ('CANID_UNI_BMS_EMC_I','B0000000', 'BMSV1', 1,1,'U8_U8_U8_X4','BMSV1 UNIversal From EMC, Incoming msg to BMS: X4=target CANID');
INSERT INTO CANID VALUES ('CANID_UNI_BMS_PC_I' ,'B0200000', 'BMSV1', 1,1,'U8_U8_U8_X4','BMSV1 UNIversal From PC,  Incoming msg to BMS: X4=target CANID');
INSERT INTO CANID VALUES ('CANID_UNI_BMS01_R','B3040000', 'BMSV1', 1,1,'U8_U8_U8_X4','BQ76952   #BQ01 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS02_R','B3060000', 'BMSV1', 1,1,'U8_U8_U8_X4','MAX14921  #MX01 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS03_R','B3080000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD01 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS04_R','B30A0000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD02 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS05_R','B30C0000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD03 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS06_R','B30E0000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD04 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS07_R','B3100000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD05 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS08_R','B3120000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD06 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS09_R','B3140000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD07 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS10_R','B3160000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD08 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS11_R','B3180000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD09 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS12_R','B31A0000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD10 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS13_R','B31C0000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD11 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_UNI_BMS14_R','B31E0000', 'BMSV1', 1,1,'U8_U8_U8_X4','ADBMS1818 #AD12 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');

--     Send Battery Module Cell readings
INSERT INTO CANID VALUES ('CANID_CMD_BMS_CELLVQ_EMC','B0400000', 'BMSV1', 1,1,'U8','BMSV1 U8: EMC requests to BMS to send cellv, cmd code');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_CELLVQ_PC' ,'B0400004', 'BMSV1', 1,1,'U8','BMSV1 U8: PC  requests to BMS to send cellv, cmd code');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV11R','B0201114', 'BMSV1', 1,1,'U16_U16_U16_U16','01 BQ76952   #BQ01 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV12R','B0201124', 'BMSV1', 1,1,'U16_U16_U16_U16','02 MAX14921  #MX01 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV13R','B0201134', 'BMSV1', 3,1,'U16_U16_U16_U16','03 ADBMS1818 #AD02 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV14R','B0201144', 'BMSV1', 4,1,'U16_U16_U16_U16','04 ADBMS1818 #AD03 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV15R','B0201154', 'BMSV1', 5,1,'U16_U16_U16_U16','05 ADBMS1818 #AD04 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV16R','B0201164', 'BMSV1', 6,1,'U16_U16_U16_U16','06 ADBMS1818 #AD05 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV17R','B0201174', 'BMSV1', 7,1,'U16_U16_U16_U16','07 ADBMS1818 #AD06 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV18R','B0201184', 'BMSV1', 8,1,'U16_U16_U16_U16','08 ADBMS1818 #AD07 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV21R','B0201214', 'BMSV1',11,1,'U16_U16_U16_U16','09 ADBMS1818 #AD08 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV22R','B0201224', 'BMSV1',12,1,'U16_U16_U16_U16','10 ADBMS1818 #AD09 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV23R','B0201234', 'BMSV1',13,1,'U16_U16_U16_U16','11 ADBMS1818 #AD10 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV24R','B0201244', 'BMSV1',14,1,'U16_U16_U16_U16','12 ADBMS1818 #AD11 U16:id,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV25R','B0201254', 'BMSV1',15,1,'U16_U16_U16_U16','25 BMSV1 U8:n,U16:cellv n,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV26R','B0201264', 'BMSV1',16,1,'U16_U16_U16_U16','26 BMSV1 U8:n,U16:cellv n,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV27R','B0201274', 'BMSV1',17,1,'U16_U16_U16_U16','27 BMSV1 U8:n,U16:cellv n,U16:cellv n+1,U16:cellv n+2');
INSERT INTO CANID VALUES ('CANID_MSG_BMS_CELLV28R','B0201284', 'BMSV1',18,1,'U16_U16_U16_U16','28 BMSV1 U8:n,U16:cellv n,U16:cellv n+1,U16:cellv n+2');

--     Send various readings
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISCQ_EMC','B0400024', 'BMSV1', 1,1,'U8_U8_U8',   'BMSV1 U8: EMC requests for BMS to value for given cmd code');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISCQ_PC' ,'B0400014', 'BMSV1', 1,1,'U8_U8_U8',   'BMSV1 U8: PC  requests for BMS to value for given cmd code');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC11R','B0401114', 'BMSV1', 1,1,'U8_U8_U8_X4','11 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC12R','B0401124', 'BMSV1', 2,1,'U8_U8_U8_X4','12 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC13R','B0401134', 'BMSV1', 3,1,'U8_U8_U8_X4','13 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC14R','B0401144', 'BMSV1', 4,1,'U8_U8_U8_X4','14 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC15R','B0401154', 'BMSV1', 5,1,'U8_U8_U8_X4','15 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC16R','B0401164', 'BMSV1', 6,1,'U8_U8_U8_X4','16 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC17R','B0401174', 'BMSV1', 7,1,'U8_U8_U8_X4','17 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC18R','B0401184', 'BMSV1', 8,1,'U8_U8_U8_X4','18 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC21R','B0401214', 'BMSV1',11,1,'U8_U8_U8_X4','21 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC22R','B0401224', 'BMSV1',12,1,'U8_U8_U8_X4','22 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC23R','B0401234', 'BMSV1',13,1,'U8_U8_U8_X4','23 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC24R','B0401244', 'BMSV1',14,1,'U8_U8_U8_X4','24 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC25R','B0401254', 'BMSV1',15,1,'U8_U8_U8_X4','25 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC26R','B0401264', 'BMSV1',16,1,'U8_U8_U8_X4','26 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC27R','B0401274', 'BMSV1',17,1,'U8_U8_U8_X4','27 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');
INSERT INTO CANID VALUES ('CANID_CMD_BMS_MISC28R','B0401284', 'BMSV1',18,1,'U8_U8_U8_X4','28 BMSV1 U8:module_ID,U8:cmd code,U8:spare,X4:4 byte value ');

-- Nodes/Units (Loader addresses)
INSERT INTO CANID VALUES ('CANID_UNIT_BMS01','B0600000','UNIT_BMS01', 1,1,'U8_U8_U8_X4','BMS BQ76952   #01');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS02','B0800000','UNIT_BMS02', 1,1,'U8_U8_U8_X4','BMS MAX14921  #01');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS03','B0A00000','UNIT_BMS03', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #01');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS04','B0C00000','UNIT_BMS04', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #02');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS05','B0E00000','UNIT_BMS05', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #03');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS06','B1000000','UNIT_BMS06', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #04');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS07','B1200000','UNIT_BMS07', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #05');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS08','B1400000','UNIT_BMS08', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #06');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS09','B1600000','UNIT_BMS09', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #07');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS10','B1800000','UNIT_BMS10', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #08');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS11','B1A00000','UNIT_BMS11', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #09');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS12','B1E00000','UNIT_BMS12', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #10');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS13','B2000000','UNIT_BMS13', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #11');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS14','B2200000','UNIT_BMS14', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #12');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS15','B2400000','UNIT_BMS15', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #13');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS16','B2600000','UNIT_BMS16', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #14');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS17','B2800000','UNIT_BMS17', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #15');
INSERT INTO CANID VALUES ('CANID_UNIT_BMS18','B2A00000','UNIT_BMS18', 1,1,'U8_U8_U8_X4','BMS ADBMS1818 #16');

INSERT INTO CANID VALUES ('CANID_UNIT_EMCMMC1','A0000000','UNIT_ECMCL1', 1,1,'U8_U8_U8_X4','bmsmot 1 EMC/MMC Local');
INSERT INTO CANID VALUES ('CANID_UNIT_EMCMMC2','A0200000','UNIT_ECMCL2', 1,1,'U8_U8_U8_X4','bmsmot 2 EMC/MMC Local');
INSERT INTO CANID VALUES ('CANID_UNIT_EMCMMC3','A0400000','UNIT_ECMCM',  1,1,'U8_U8_U8_X4','bmsmot 3 EMC/MMC Master');

INSERT INTO CANID VALUES ('CANID_UNIT_EMCMMC1R','A1000000','UNIT_ECMCL1', 1,1,'U8_U8_U8_X4','bmsmot 1 EMC/MMC Local R');
INSERT INTO CANID VALUES ('CANID_UNIT_EMCMMC2R','A1200000','UNIT_ECMCL2', 1,1,'U8_U8_U8_X4','bmsmot 2 EMC/MMC Local R');
INSERT INTO CANID VALUES ('CANID_UNIT_EMCMMC3R','A1400000','UNIT_ECMCM',  1,1,'U8_U8_U8_X4','bmsmot 3 EMC/MMC Master R');

INSERT INTO CANID VALUES ('CANID_UNIT_2', '04000000','UNIT_2', 1,1,'U8','Sensor unit: Drive shaft encoder #1');
INSERT INTO CANID VALUES ('CANID_UNIT_3', '03800000','UNIT_3', 1,1,'U8','Sensor unit: Engine');
INSERT INTO CANID VALUES ('CANID_UNIT_4', '03A00000','UNIT_4', 1,1,'U8','Sensor unit: Lower sheave shaft encoder');
INSERT INTO CANID VALUES ('CANID_UNIT_5', '03C00000','UNIT_5', 1,1,'U8','Sensor unit: Upper sheave shaft encoder');
INSERT INTO CANID VALUES ('CANID_UNIT_8', '01000000','UNIT_8', 1,1,'U8','Sensor unit: Level wind');
INSERT INTO CANID VALUES ('CANID_UNIT_9', '01200000','UNIT_9', 1,1,'U8','Sensor unit: XBee receiver #1');
INSERT INTO CANID VALUES ('CANID_UNIT_A', '0140000C','UNIT_A', 1,1,'U8','Sensor unit: XBee receiver #2');
INSERT INTO CANID VALUES ('CANID_UNIT_B', '0160000C','UNIT_B', 1,1,'U8','Display driver/console');
INSERT INTO CANID VALUES ('CANID_UNIT_C', '0180000C','UNIT_C', 1,1,'U8','CAWs Olimex board');
INSERT INTO CANID VALUES ('CANID_UNIT_D', '01A0000C','UNIT_D', 1,1,'U8','POD board sensor prototype ("6" marked on board)');
INSERT INTO CANID VALUES ('CANID_UNIT_E', '01C0000C','UNIT_E', 1,1,'U8','Logger1: sensor board w ublox gps & SD card GSM 18-09-17');
INSERT INTO CANID VALUES ('CANID_UNIT_F', '01E0000C','UNIT_F', 1,1,'U8','Tension_1 & Cable_angle_1 Unit');
INSERT INTO CANID VALUES ('CANID_UNIT_10','0200000C','UNIT_10',1,1,'U8','Gateway1: 2 CAN');
INSERT INTO CANID VALUES ('CANID_UNIT_11','02200000','UNIT_11',1,1,'U8','Tension: 1 AD7799 VE POD brd 1');
INSERT INTO CANID VALUES ('CANID_UNIT_12','02400000','UNIT_12',1,1,'U8','Tension: 2 AD7799 VE POD brd 2 GSM 18-09-17');
INSERT INTO CANID VALUES ('CANID_UNIT_13','02600000','UNIT_13',1,1,'U8','Yogurt: Olimex board');
INSERT INTO CANID VALUES ('CANID_UNIT_14','02E00000','UNIT_14',1,1,'U8','Tension: 1 AD7799 w op-amp VE POD brd 3');
INSERT INTO CANID VALUES ('CANID_UNIT_15','02A00000','UNIT_15',1,1,'U8','Tension: 2 AD7799 VE POD brd 4');
INSERT INTO CANID VALUES ('CANID_UNIT_16','02C00000','UNIT_16',1,1,'U8','Tension: 2 AD7799 VE POD brd 5 GSM');
INSERT INTO CANID VALUES ('CANID_UNIT_17','03200000','UNIT_17',1,1,'U8','Gateway2: 1 CAN GSM 18-09-17');
INSERT INTO CANID VALUES ('CANID_UNIT_18','03400000','UNIT_18',1,1,'U8','Gateway3: 1 CAN');
INSERT INTO CANID VALUES ('CANID_UNIT_19','02800000','UNIT_19',1,1,'U8','Master Controller');
INSERT INTO CANID VALUES ('CANID_UNIT_1A','03600000','UNIT_1A',1,1,'U8','Logger2: sensor board w ublox gps & SD card');
INSERT INTO CANID VALUES ('CANID_UNIT_1B','03E00000','UNIT_1B',1,1,'U8','Sensor board: CAW experiments');
INSERT INTO CANID VALUES ('CANID_UNIT_1C','04200000','UNIT_1C',1,1,'U8','Sensor board: DEH spare 1');
INSERT INTO CANID VALUES ('CANID_UNIT_1D','04400000','UNIT_1D',1,1,'U8','Sensor board: DEH spare 2');
INSERT INTO CANID VALUES ('CANID_UNIT_1E','04800000','UNIT_1E',1,1,'U8','DiscoveryF4 shaft encoder unit 1');
INSERT INTO CANID VALUES ('CANID_UNIT_1F','04A00000','UNIT_1F',1,1,'U8','Pwrbox: Blue Pill perf board');
INSERT INTO CANID VALUES ('CANID_UNIT_20','04E00000','UNIT_20',1,1,'U8','Gateway4: 1 CAN');
INSERT INTO CANID VALUES ('CANID_UNIT_21','05000000','UNIT_21',1,1,'U8','Contactor1: Blue Pill Mboard');
INSERT INTO CANID VALUES ('CANID_UNIT_22','05200000','UNIT_22',1,1,'U8','GEVCUr: gevcu replasement: DiscoveyF4 Control Panel');
INSERT INTO CANID VALUES ('CANID_ALL_PC' ,'05400000','UNIT_X1',1,1,'U8','PC sends to all CAN loaders');
INSERT INTO CANID VALUES ('CANID_ALL_OTH','05600000','UNIT_X2',1,1,'U8','Someone else sends to all CAN loaders');

INSERT INTO CANID VALUES ('CANID_ELCON_TX','C7FA872C','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger transmit: ');
INSERT INTO CANID VALUES ('CANID_ELCON_RX','C0372FA4','ELCON1',1,1,'I16_I16_U8_U8_U8_U8','ELCON Charger receive: ');

--
INSERT INTO CANID VALUES ('CANID_UNIT_99',  'FFFFFF14', 'UNIT_99',1,1,'UNDEF','Dummy for missing CAN IDs');
INSERT INTO CANID VALUES ('CANID_DUMMY',    'FFFFFFFC', 'UNIT_NU',1,1,'UNDEF','Dummy ID: Lowest priority possible (Not Used)');
INSERT INTO CANID VALUES ('CANID_MSG_DUMMY','FFFFFF16', 'ANY',    1,1,'UNDEF','Dummy ID: Polled Msg dummy');

