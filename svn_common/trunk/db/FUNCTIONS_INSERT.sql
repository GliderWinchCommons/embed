-- FUNCTIONS table
-- 07/11/2015, 05/11/2016
--
-- Associates specific functions (or instances of a generic function)
--   with a CAN unit, GENERIC function type, and the command CAN ID.
--   Programs can implement a number of generic functions, but units
--   running the program provide a specific instance of the generic function.
--
-- FUNCTION_NAME varchar(24) PRIMARY KEY,
--  Each *instance* of a function has a unique name, even though it there can be many functions that are
--  all the same FUNCTION_TYPE, e.g. tension for drum #1, #2, etc. are are the same *type*, but
--  each has a specific *name*.  
--
-- CANID_CMD_NAME varchar(48) NOT NULL UNIQUE,
--  The name of the CAN ID in CANID_INSERT.sql used for commands with this function.
--
-- NOTE: There are two command CAN ID's.  One for interrogation (I) and the other for the response (R).
--   Both belong to the function instance.  The app initialization needs to identify the I & R CAN IDs.
--   This is done by adding 4096 to the FUNCTION_CODE for the R CAN when the table is generated.  That
--   the initialization code can identify the pair by the FUNCTION_CODE and'ed with 0xfff, and also
--   identify the I and R CAN IDs.
--
-- FUNCTION_TYPE varchar(24),
--  The *type* of function for this *instance* of the function 
--
-- DESCRIPTION7 varchar(128) NOT NULL UNIQUE
--  Good words that might mean something to someone. 
--
-- PRIMARY KEY(FUNCTION_NAME,CMD_IR),
--   Each function instance has a I (interrogate) and R (response) CAN ID.
--
DELETE from FUNCTIONS;
--                                                                       CMD-IR
--                            FUNCTION_NAME       CANID_CMD_NAME               CAN_UNIT_NAME     FUNCTION_TYPE     DESCRIPTION7
INSERT INTO FUNCTIONS VALUES ('SHAFT1',	   'CANID_CMD_SHAFT1I'       ,'I', 'CAN_UNIT_2',	'DRIVE_SHAFT','Sensor, shaft1: I Drive shaft encoder' );
INSERT INTO FUNCTIONS VALUES ('SHAFT1',	   'CANID_CMD_SHAFT1R'       ,'R', 'CAN_UNIT_2',	'DRIVE_SHAFT','Sensor, shaft1: R Drive shaft encoder' );

INSERT INTO FUNCTIONS VALUES ('SHAFT_UPPERSHV', 'CANID_CMD_UPPERSHVI'  ,'I', 'CAN_UNIT_4',	'SHAFT_ENCODER','(Upper sheave) I' );
INSERT INTO FUNCTIONS VALUES ('SHAFT_UPPERSHV', 'CANID_CMD_UPPERSHVR'  ,'R', 'CAN_UNIT_4',	'SHAFT_ENCODER','(Upper sheave) R' );

INSERT INTO FUNCTIONS VALUES ('SHAFT_LOWERSHV', 'CANID_CMD_LOWERSHVI'  ,'I', 'CAN_UNIT_5',	'SHAFT_ENCODER','(Lower sheave) I Count and speed' );
INSERT INTO FUNCTIONS VALUES ('SHAFT_LOWERSHV', 'CANID_CMD_LOWERSHVR'  ,'R', 'CAN_UNIT_5',	'SHAFT_ENCODER','(Lower sheave) R Count and speed' );

-- The following pair are commented out.  These are if all four functions are combined into one (which was the original approach)
--INSERT INTO FUNCTIONS VALUES ('ENGINE_SENSOR', 	'CANID_CMD_ENGINE_SENSORI','I', 'CAN_UNIT_3', 'ENGINE_SENSOR','Sensor, engine: I rpm, manifold pressure, throttle setting, temperature' );
--INSERT INTO FUNCTIONS VALUES ('ENGINE_SENSOR', 	'CANID_CMD_ENGINE_SENSORR','R', 'CAN_UNIT_3', 'ENGINE_SENSOR','Sensor, engine: R rpm, manifold pressure, throttle setting, temperature' );
INSERT INTO FUNCTIONS VALUES ('ENG1_MANIFOLD',  'CANID_CMD_ENG1_MANFLDI'  ,'I', 'CAN_UNIT_3', 'ENG_MANIFOLD',' Sensor, engine: I manifold pressure' );
INSERT INTO FUNCTIONS VALUES ('ENG1_MANIFOLD',  'CANID_CMD_ENG1_MANFLDR'  ,'R', 'CAN_UNIT_3', 'ENG_MANIFOLD',' Sensor, engine: R manifold pressure' );
INSERT INTO FUNCTIONS VALUES ('ENG1_RPM',       'CANID_CMD_ENG1_RPMI'     ,'I', 'CAN_UNIT_3', 'ENG_RPM',' Sensor, engine: I rpm' );
INSERT INTO FUNCTIONS VALUES ('ENG1_RPM',       'CANID_CMD_ENG1_RPMR'     ,'R', 'CAN_UNIT_3', 'ENG_RPM',' Sensor, engine: R rpm' );
INSERT INTO FUNCTIONS VALUES ('ENG1_THROTTLE',  'CANID_CMD_ENG1_THROTTLEI','I', 'CAN_UNIT_3', 'ENG_THROTTLE',' Sensor, engine: I throttle position' );
INSERT INTO FUNCTIONS VALUES ('ENG1_THROTTLE',  'CANID_CMD_ENG1_THROTTLER','R', 'CAN_UNIT_3', 'ENG_THROTTLE',' Sensor, engine: R throttle position' );
INSERT INTO FUNCTIONS VALUES ('ENG1_T1',        'CANID_CMD_ENG1_T1I'      ,'I', 'CAN_UNIT_3', 'ENG_T1',' Sensor, engine: I temperature #1' );
INSERT INTO FUNCTIONS VALUES ('ENG1_T1',        'CANID_CMD_ENG1_T1R'      ,'R', 'CAN_UNIT_3', 'ENG_T1',' Sensor, engine: R temperature #1' );


INSERT INTO FUNCTIONS VALUES ('TENSION_a1W', 	'CANID_CMD_TENSION_a1WI', 'I', 'CAN_UNIT_F',	'TENSION_a',	'Tension_a: I Tension drum #1 AD7799 #1' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a1W', 	'CANID_CMD_TENSION_a1WR', 'R', 'CAN_UNIT_F',	'TENSION_a',	'Tension_a: R Tension drum #1 AD7799 #1' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a2W', 	'CANID_CMD_TENSION_a2WI' ,'I', 'CAN_UNIT_F',	'TENSION_a2',	'Tension_a: I Tension drum #1 AD7799 #2' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a2W', 	'CANID_CMD_TENSION_a2WR' ,'R', 'CAN_UNIT_F',	'TENSION_a2',	'Tension_a: R Tension drum #1 AD7799 #2' );

INSERT INTO FUNCTIONS VALUES ('CABLE_ANGLE_1', 'CANID_CMD_CABLE_ANGLE_1I','I', 'CAN_UNIT_F',	'CABLE_ANGLE', 	'Cable angle: I AD7799 #2 drum #1' );
INSERT INTO FUNCTIONS VALUES ('CABLE_ANGLE_1', 'CANID_CMD_CABLE_ANGLE_1R','R', 'CAN_UNIT_F',	'CABLE_ANGLE', 	'Cable angle: R AD7799 #2 drum #1' );

INSERT INTO FUNCTIONS VALUES ('TENSION_2', 	'CANID_CMD_TENSION_2I',     'I', 'CAN_UNIT_14',	'TENSION_c',	'Tension_2: I Tension AD7799 #1 drum #2' );
INSERT INTO FUNCTIONS VALUES ('TENSION_2', 	'CANID_CMD_TENSION_2R',     'R', 'CAN_UNIT_14',	'TENSION_c',	'Tension_2: R Tension AD7799 #1 drum #2' );

--INSERT INTO FUNCTIONS VALUES ('TENSION_CAL_1', 	'CANID_CMD_TENSION_CAL_1I',	'I', 'CAN_UNIT_15',	'TENSION_a',	'Calibration tension: I S-load-cell #1' );
--INSERT INTO FUNCTIONS VALUES ('TENSION_CAL_1', 	'CANID_CMD_TENSION_CAL_1R',	'R', 'CAN_UNIT_15',	'TENSION_a',	'Calibration tension: R S-load-cell #1' );

INSERT INTO FUNCTIONS VALUES ('CABLE_ANGLE_2','CANID_CMD_CABLE_ANGLE_2I','I', 'CAN_UNIT_14',	'CABLE_ANGLE',	'Cable angle: I AD7799 #2 drum #2' );
INSERT INTO FUNCTIONS VALUES ('CABLE_ANGLE_2','CANID_CMD_CABLE_ANGLE_2R','R', 'CAN_UNIT_14',	'CABLE_ANGLE',	'Cable angle: R AD7799 #2 drum #2' );

INSERT INTO FUNCTIONS VALUES ('GPS_1',		'CANID_CMD_GPS_1I',         'I', 'CAN_UNIT_E',	'GPS',		'GPS_1: I time sync distribution msg' );
INSERT INTO FUNCTIONS VALUES ('GPS_1',		'CANID_CMD_GPS_1R',         'R', 'CAN_UNIT_E',	'GPS',		'GPS_1: R time sync distribution msg' );

INSERT INTO FUNCTIONS VALUES ('LOGGER_1',	'CANID_CMD_LOGGER_1I',      'I', 'CAN_UNIT_E',	'LOGGER',	'Logger_1: I ' );
INSERT INTO FUNCTIONS VALUES ('LOGGER_1',	'CANID_CMD_LOGGER_1R',      'R', 'CAN_UNIT_E',	'LOGGER',	'Logger_1: R ' );

INSERT INTO FUNCTIONS VALUES ('GPS_2',		'CANID_CMD_GPS_2I',	       'I', 'CAN_UNIT_1A',	'GPS',		'GPS_2: I  time sync distribution msg ' );
INSERT INTO FUNCTIONS VALUES ('GPS_2',		'CANID_CMD_GPS_2R',	       'R', 'CAN_UNIT_1A',	'GPS',		'GPS_2: R time sync distribution msg ' );

INSERT INTO FUNCTIONS VALUES ('LOGGER_2',	     'CANID_CMD_LOGGER_2I', 'I', 'CAN_UNIT_1A',	'LOGGER',	'Logger_2: I ' );
INSERT INTO FUNCTIONS VALUES ('LOGGER_2',	     'CANID_CMD_LOGGER_2R', 'R', 'CAN_UNIT_1A',	'LOGGER',	'Logger_2: R ' );

INSERT INTO FUNCTIONS VALUES ('HC_SANDBOX_1','CANID_CMD_SANDBOX_1I',	 'I', 'CAN_UNIT_99',	'HC_SANDBOX_1',	'Host Controller: I sandbox function 1' );
INSERT INTO FUNCTIONS VALUES ('HC_SANDBOX_1','CANID_CMD_SANDBOX_1R',	 'R', 'CAN_UNIT_99',	'HC_SANDBOX_1',	'Host Controller: R sandbox function 1' );

INSERT INTO FUNCTIONS VALUES ('MC_a',		      'CANID_CMD_MCI',	    'I', 'CAN_UNIT_19',	'MC',	'Master Controller: I ' );
INSERT INTO FUNCTIONS VALUES ('MC_a',		      'CANID_CMD_MCR',	    'R', 'CAN_UNIT_19',	'MC',	'Master Controller: R ' );

INSERT INTO FUNCTIONS VALUES ('TILT_SENSE_a',	'CANID_CMD_TILTI',	 'I', 'CAN_UNIT_10',	'TILT_SENSE',	'Tilt sensor: I ' );
INSERT INTO FUNCTIONS VALUES ('TILT_SENSE_a',	'CANID_CMD_TILTR',	 'R', 'CAN_UNIT_10',	'TILT_SENSE',	'Tilt sensor: R ' );

INSERT INTO FUNCTIONS VALUES ('YOGURT_1',	  'CANID_CMD_YOGURT_1I', 	 'I', 'CAN_UNIT_13',	'YOGURT_1',	'Yogurt_1: I Ver 1 of maker' );
INSERT INTO FUNCTIONS VALUES ('YOGURT_1',	  'CANID_CMD_YOGURT_1R',	 'R', 'CAN_UNIT_13',	'YOGURT_1',	'Yogurt_1: R Ver 1 of maker' );
--                                                                   CMD-IR
--                            FUNCTION_NAME       CANID_CMD_NAME               CAN_UNIT_NAME     FUNCTION_TYPE     DESCRIPTION7
INSERT INTO FUNCTIONS VALUES ('TENSION_a0X',	'CANID_CMD_TENSION_a0XI' ,'I', 'CAN_UNIT_11',	'TENSION_a',	'Tension_a: I 1 AD7799 VE POD Test (hence X) 0' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a0X',	'CANID_CMD_TENSION_a0XR' ,'R', 'CAN_UNIT_11',	'TENSION_a',	'Tension_a: R 1 AD7799 VE POD Test (hence X) 0' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a11',	'CANID_CMD_TENSION_a11I','I', 'CAN_UNIT_12',	'TENSION_a',	'Tension_a: I  2 AD7799 VE POD 1 port 1' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a11',	'CANID_CMD_TENSION_a11R','R', 'CAN_UNIT_12',	'TENSION_a',	'Tension_a: R  2 AD7799 VE POD 1 port 2' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a21',	'CANID_CMD_TENSION_a21I','I', 'CAN_UNIT_12',	'TENSION_a2',	'Tension_a2: I 2 AD7799 VE POD 1 port 2' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a21',	'CANID_CMD_TENSION_a21R','R', 'CAN_UNIT_12',	'TENSION_a2',	'Tension_a2: R 2 AD7799 VE POD 1 port 2' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a1Y',	'CANID_CMD_TENSION_a1YI','I', 'CAN_UNIT_14',	'TENSION_a',	'Tension_a:  I 1 AD7799 VE POD port 1' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a1Y',	'CANID_CMD_TENSION_a1YR','R', 'CAN_UNIT_14',	'TENSION_a',	'Tension_a:  R 1 AD7799 VE POD port 1' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a12',	'CANID_CMD_TENSION_a12I','I', 'CAN_UNIT_15',	'TENSION_a',	'Tension_a:  I 2 AD7799 VE POD 2 port 1' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a12',	'CANID_CMD_TENSION_a12R','R', 'CAN_UNIT_15',	'TENSION_a',	'Tension_a:  R 2 AD7799 VE POD 2 port 1' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a22',	'CANID_CMD_TENSION_a22I','I', 'CAN_UNIT_15',	'TENSION_a2',	'Tension_a2: I 2 AD7799 VE POD port 2' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a22',	'CANID_CMD_TENSION_a22R','R', 'CAN_UNIT_15',	'TENSION_a2',	'Tension_a2: R 2 AD7799 VE POD port 2' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a1G',	'CANID_CMD_TENSION_a1GI','I', 'CAN_UNIT_16',	'TENSION_a',	'Tension_a:  I 2 AD7799 VE POD GSM 1 port 1' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a1G',	'CANID_CMD_TENSION_a1GR','R', 'CAN_UNIT_16',	'TENSION_a',	'Tension_a:  R 2 AD7799 VE POD GSM 1 port 1' );

INSERT INTO FUNCTIONS VALUES ('TENSION_a2G',	'CANID_CMD_TENSION_a2GI','I', 'CAN_UNIT_16',	'TENSION_a2',	'Tension_a2: I 2 AD7799 VE POD GSM 1 port 2' );
INSERT INTO FUNCTIONS VALUES ('TENSION_a2G',	'CANID_CMD_TENSION_a2GR','R', 'CAN_UNIT_16',	'TENSION_a2',	'Tension_a2: R 2 AD7799 VE POD GSM 1 port 2' );

INSERT INTO FUNCTIONS VALUES ('CANSENDER_1',	'CANID_CMD_CANSENDER_1I','I', 'CAN_UNIT_1B',	'CANSENDER',	'Cansender_1: I Sensor board test' );
INSERT INTO FUNCTIONS VALUES ('CANSENDER_1',	'CANID_CMD_CANSENDER_1R','R', 'CAN_UNIT_1B',	'CANSENDER',	'Cansender_1: R Sensor board test' );
--                                                                  CMD-IR
--                            FUNCTION_NAME       CANID_CMD_NAME               CAN_UNIT_NAME     FUNCTION_TYPE     DESCRIPTION7
INSERT INTO FUNCTIONS VALUES ('SHEAVE_1UP',	'CANID_CMD_UPPER1_HI','I',    'CAN_UNIT_1E',	'SHEAVE_UP_H',	'Shaft encoder_a1: I upper' );
INSERT INTO FUNCTIONS VALUES ('SHEAVE_1UP',	'CANID_CMD_UPPER1_HR','R',    'CAN_UNIT_1E',	'SHEAVE_UP_H',	'Shaft encoder_a1: R upper' );

INSERT INTO FUNCTIONS VALUES ('SHEAVE_1LO',	'CANID_CMD_LOWER1_HI','I',    'CAN_UNIT_1E',	'SHEAVE_LO_H',	'Shaft encoder_a1: I lower' );
INSERT INTO FUNCTIONS VALUES ('SHEAVE_1LO',	'CANID_CMD_LOWER1_HR','R',    'CAN_UNIT_1E',	'SHEAVE_LO_H',	'Shaft encoder_a1: R lower' );

INSERT INTO FUNCTIONS VALUES ('PWRBOX1',  	'CANID_CMD_PWRBOX1I' ,'I',    'CAN_UNIT_1F',	'PWRBOX',	'Pwrbox1: I lower' );
INSERT INTO FUNCTIONS VALUES ('PWRBOX1',	   'CANID_CMD_PWRBOX1R' ,'R',    'CAN_UNIT_1F',	'PWRBOX',	'Pwrbox1: R lower' );


