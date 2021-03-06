-- FUNCTIONS_TYPE table
-- 05/20/2016, 10/25/2020
--
-- FUNCTIONS_TYPE varchar(24) PRIMARY KEY,
--  Type of function that might be used many places.  All instances
--  will have the same parameter/calibration/CAN ID layout, plus a set
--  of values for each instance.
-- 
-- FUNCTION_TYPE_CODE NUMERIC UNIQUE,
--  This code is used in the STM32 for looking up a function type command
--  CAN ID.
--
-- DESCRIPTION8 varchar(128) NOT NULL UNIQUE
--  Good words that might mean something to someone. 
--
DELETE from FUNCTIONS_TYPE;
--                                 FUNCTION_TYPE    FUNCTION_TYPE_CODE    DESCRIPTION8
INSERT INTO FUNCTIONS_TYPE VALUES ('SHAFT_ENCODER', 1, 'Sensor, shaft: Drive shaft encoder' );
INSERT INTO FUNCTIONS_TYPE VALUES ('ENGINE_SENSOR', 2, 'Sensor, engine: rpm, manifold pressure, throttle setting, temperature' );
INSERT INTO FUNCTIONS_TYPE VALUES ('TENSION_a',     3, 'Tension_a: Tension AD7799 #1' );
INSERT INTO FUNCTIONS_TYPE VALUES ('TENSION_a2',    4, 'Tension_a: Tension AD7799 #2' );
INSERT INTO FUNCTIONS_TYPE VALUES ('CABLE_ANGLE',   5, 'Cable angle AD7799' );
INSERT INTO FUNCTIONS_TYPE VALUES ('TENSION_c',     6, 'Tension_c: Tension op-amp' );
INSERT INTO FUNCTIONS_TYPE VALUES ('TIMESYNC',      7, 'GPS time sync distribution msg' );
INSERT INTO FUNCTIONS_TYPE VALUES ('HC_SANDBOX_1',  8, 'Host Controller: sandbox function 1' );
INSERT INTO FUNCTIONS_TYPE VALUES ('MC',            9, 'Master Controller' );
INSERT INTO FUNCTIONS_TYPE VALUES ('TILT_SENSE',   10, 'Tilt sensor' );
INSERT INTO FUNCTIONS_TYPE VALUES ('YOGURT_1',     11, 'Yogurt_1: Ver 1 of maker' );
INSERT INTO FUNCTIONS_TYPE VALUES ('GPS',          12, 'GPS' );
INSERT INTO FUNCTIONS_TYPE VALUES ('LOGGER',       13, 'Logger' );
INSERT INTO FUNCTIONS_TYPE VALUES ('CANSENDER',    14, 'Cansender' );
INSERT INTO FUNCTIONS_TYPE VALUES ('SHEAVE_UP_H',  15, 'Uppersheave (TIM2 encoder)' );
INSERT INTO FUNCTIONS_TYPE VALUES ('SHEAVE_LO_H',  16, 'Lowersheave (TIM5 encoder)' );
INSERT INTO FUNCTIONS_TYPE VALUES ('ENG_MANIFOLD', 17, 'Sensor, engine: manifold pressure' );
INSERT INTO FUNCTIONS_TYPE VALUES ('ENG_RPM',      18, 'Sensor, engine: rpm' );
INSERT INTO FUNCTIONS_TYPE VALUES ('ENG_THROTTLE', 19, 'Sensor, engine: throttle' );
INSERT INTO FUNCTIONS_TYPE VALUES ('ENG_T1',       20, 'Sensor, engine: temperature 1' );
INSERT INTO FUNCTIONS_TYPE VALUES ('ENG_T2',       21, 'Sensor, engine: temperature 2' );
INSERT INTO FUNCTIONS_TYPE VALUES ('DRIVE_SHAFT',  22, 'Sensor, shaft: ../sensor/shaft/trunk version' );
INSERT INTO FUNCTIONS_TYPE VALUES ('PWRBOX',       23, 'Pwrbox, input power voltage monitor' );
-- Additions for proto winch
INSERT INTO FUNCTIONS_TYPE VALUES ('BRAKE',        24, 'Drum brake' );
INSERT INTO FUNCTIONS_TYPE VALUES ('LEVELWIND',    25, 'Levelwind' );
INSERT INTO FUNCTIONS_TYPE VALUES ('DRUM',         26, 'Drum speed, odometer, cable management' );
INSERT INTO FUNCTIONS_TYPE VALUES ('CPDISPLAY20x4',27, 'Control Panel: Display: 20x4 LCD' );
INSERT INTO FUNCTIONS_TYPE VALUES ('CPSWS_V1',     28, 'Control Panel: Switches & Control Lever: version 1' );
INSERT INTO FUNCTIONS_TYPE VALUES ('CPLEDS_V1',    29, 'Control Panel: LEDs: version 1' );
INSERT INTO FUNCTIONS_TYPE VALUES ('BEEPER_V1',    30, 'Beeper: version 1' );
INSERT INTO FUNCTIONS_TYPE VALUES ('CNTCTR',       31, 'Contactor: original' );
INSERT INTO FUNCTIONS_TYPE VALUES ('GENCMD',       32, 'Generic command (MC, PC, HC, etc.' );
INSERT INTO FUNCTIONS_TYPE VALUES ('PCCMD',        33, 'PC command' );
INSERT INTO FUNCTIONS_TYPE VALUES ('RC',           34, 'Remote Controller' );





