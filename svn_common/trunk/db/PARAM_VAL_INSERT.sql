-- PARAM_VAL table
-- 05/29/2016
--
-- NOTE: Fields that are later converted to numeric fail with leading blanks,
--  ' 4' fails
--  '04' passes
--  '4'  passes
-- ' 8192' passes(!)
--
-- PRIMARY KEY(FUNCTION_NAME,PARAM_NAME),
--  The FUNCTION_NAME with PARAM_NAME must be unique.
--  Combination comprises the key for this table.  The combo
--  is required since some PARAM_NAME will be the same, e.g. polling msg.
--
-- FUNCTION_NAME varchar(24),
--  The specific instance of a FUNCTION_TYPE
--  Note the difference between 'FUNCTION_NAME' and 'FUNCTION_TYPE', e.g.
--  'TENSION_a11' versus 'TENSION_a', where the latter can be applied to many
--  instances, but the former is specific, i.e. the number '1' (which could mean drum #1).
--
-- PARAM_NAME varchar(48)
--  The same name that is used in PARAM_LIST (which links the two tables).
--
-- PARAM_VAL varchar(48),
--   The value (four bytes) for this parameter.  A java routine uses PARAM_LIST to
--   obtain the number type (e.g. float) for converting the string into a four byte hex
--   that the unit can use.
--
--   Note that in the case of a number type that is a CAN ID the name of the CAN ID will 
--   be looked up in the CANID table.
--
-- DESCRIPTION11 varchar(128) NOT NULL
--   mumbo jumbo for a confused programmer
--   Naming convention:  Example Tension_a21
--     Function type (e.g. Tension_a)
--     Function instance (e.g. 2 - AD7799 #2)
--     Drum number (e.g. 1)
--
-- The number of entries for each FUNCTION_NAME must equal the number in PARAM_LIST (which is the
--   generic list for all instances of the FUNCTION_TYPE.
--
DELETE FROM PARAM_VAL;
--
-- ===========================================================================================================================================================================
-- PWRBOX: Instance 1 of powerbox
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL               FUNCTION_TYPE      DESCRIPTION11
-- The CAN hw filter will be setFOR the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CRC'              ,'0',     'PWRBOX',' 1 Pwrbox1: CRC');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_VERSION'          ,'1',     'PWRBOX',' 2 Pwrbox1: Version number');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_HEARTBEAT_TIME_CT','1000',  'PWRBOX',' 3 Pwrbox1: Time (ms) between HB msg');
-- ADC calibrated = (adc count - offset)*scale
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET1'     ,'0.0'        ,'PWRBOX',' 4 Pwrbox1: ADC 0 5VREG offset na');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE1'      ,'0.001246917','PWRBOX',' 5 Pwrbox1: ADC 0 5VREG scale volts/step');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET2'     ,'0.0'        ,'PWRBOX',' 6 Pwrbox1: ADC 1 CANVB offset na');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE2'      ,'0.003928678','PWRBOX',' 7 Pwrbox1: ADC 1 CANVB scale volts/step');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET3'     ,'0.0'        ,'PWRBOX',' 8 Pwrbox1: ADC 2 DIODE offset na');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE3'      ,'0.004025775','PWRBOX',' 9 Pwrbox1: ADC 2 DIODE scale volts/step');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET4'     ,'0.0'        ,'PWRBOX','10 Pwrbox1: ADC 3 INPWR offset na');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE4'      ,'0.003974026','PWRBOX','11 Pwrbox1: ADC 3 INPWR scale volts/step');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET5'     ,'2068.0'     ,'PWRBOX','12 Pwrbox1: ADC 4 HALLE offset zero');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE5'      ,'0.042462845','PWRBOX','13 Pwrbox1: ADC 4 HALLE scale amps/step');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET6'     ,'0.0'        ,'PWRBOX','14 Pwrbox1: ADC 5 SPAR1 offset');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE6'      ,'1.0'        ,'PWRBOX','15 Pwrbox1: ADC 5 SPAR1 scale');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET7'     ,'0.0'        ,'PWRBOX','16 Pwrbox1: ADC 6 ITEMP offset Vref');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE7'      ,'1.0'        ,'PWRBOX','17 Pwrbox1: ADC 6 ITEMP scale');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_OFFSET8'     ,'0.0'        ,'PWRBOX','18 Pwrbox1: ADC 7 IVREF offset');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CAL_SCALE8'      ,'0.002218499','PWRBOX','19 Pwrbox1: ADC 7 IVREF scale volts/step');

INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR0_K'     ,'128' ,'PWRBOX','20 Pwrbox1: IIR1 Filter 0 factor: 5v regulator');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR0_SCALE' ,'2'   ,'PWRBOX','21 Pwrbox1: IIR1 Filter 0 scale : 5v regulator');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR1_K'     ,'10'  ,'PWRBOX','22 Pwrbox1: IIR2 Filter 1 factor: Hall-effect sensor');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR1_SCALE' ,'1'   ,'PWRBOX','23 Pwrbox1: IIR2 Filter 1 scale : Hall-effect sensor');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR2_K'     ,'10'  ,'PWRBOX','24 Pwrbox1: IIR3 Filter 2 factor: CAN bus volts');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR2_SCALE' ,'1'   ,'PWRBOX','25 Pwrbox1: IIR3 Filter 2 scale : CAN bus volts');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR3_K'     ,'128' ,'PWRBOX','26 Pwrbox1: IIR4 Filter 3 factor: Power source volts');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_IIR3_SCALE' ,'2'   ,'PWRBOX','27 Pwrbox1: IIR4 Filter 3 scale : Power source volts');

INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_HB_R'    ,'CANID_HB_PWRBOX1' ,'PWRBOX','28 Pwrbox: CANID: Heartbeat: input voltage, bus voltage');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_MSG_R'   ,'CANID_MSG_PWRBOX1','PWRBOX','29 Pwrbox: CANID: Msg: input voltage, bus voltage ');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_ALARM_R' ,'CANID_ALM_PWRBOX1','PWRBOX','30 Pwrbox: CANID: Alarm: input voltage, bus voltage');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_ALARM_RATE' ,'50'            ,'PWRBOX','31 Pwrbox: Time (ms) between alarm msgs, when below threshold');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_ALARM_THRES','10.5'          ,'PWRBOX','32 Pwrbox: Voltage threshold for alarm msgs');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT1','CANID_HB_TIMESYNC'  ,'PWRBOX','33 Pwrbox1: CANID 0: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT2','CANID_MSG_TIME_POLL','PWRBOX','34 Pwrbox1: CANID 1: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT3','CANID_DUMMY'        ,'PWRBOX','35 Pwrbox1: CANID 2 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT4','CANID_DUMMY'        ,'PWRBOX','36 Pwrbox1: CANID 3 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT5','CANID_DUMMY'        ,'PWRBOX','37 Pwrbox1: CANID 4 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT6','CANID_DUMMY'        ,'PWRBOX','38 Pwrbox1: CANID 5 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT7','CANID_DUMMY'        ,'PWRBOX','39 Pwrbox1: CANID 6 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_CANID_HW_FILT8','CANID_DUMMY'        ,'PWRBOX','40 Pwrbox1: CANID 7 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_LVL_HB_R'   ,'CANID_LVL_HB_PWRBOX1'  ,'PWRBOX','41 Pwrbox1: CANID: Heartbeat: level wind current');
INSERT INTO PARAM_VAL VALUES ('PWRBOX1','PWRBOX_LVL_MSG_R'  ,'CANID_LVL_MSG_PWRBOX1' ,'PWRBOX','42 Pwrbox1: CANID: Msg: level wind current');
--
-- ===========================================================================================================================================================================
-- ENGINE_MANIFOLD: Instance 1 = ENG1_MANIFOLD
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL               FUNCTION_TYPE      DESCRIPTION11
-- The CAN hw filter will be setFOR the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CRC'              ,'0',     'ENG_MANIFOLD','Eng1_manifold: CRC');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_VERSION'          ,'1',     'ENG_MANIFOLD','Eng1_manifold: Version number');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_HEARTBEAT_TIME_CT','1000',  'ENG_MANIFOLD','Eng1_manifold: Time (ms) between HB msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_PRESS_OFFSET'     ,'-1.3',  'ENG_MANIFOLD','Eng1_manifold: Manifold pressure offset');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_PRESS_SCALE'      ,'0.92E-2', 'ENG_MANIFOLD','Eng1_manifold: Manifold pressure  scale (inch Hg)');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_POLL_MANFLD_R','CANID_MSG_ENG1_MANFLD','ENG_MANIFOLD','Eng1_manifold: CANID: Poll response: manifold pressure, calibrated');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_HB_MANFLD_R'  ,'CANID_HB_ENG1_MANFLD' ,'ENG_MANIFOLD','Eng1_manifold: CANID: Heartbeat: manifold pressure, calibrated');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT1','CANID_HB_TIMESYNC'     ,'ENG_MANIFOLD','EPS_1: CANID 1: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT2','CANID_MSG_TIME_POLL'   ,'ENG_MANIFOLD','MC: CANID 2:Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT3','CANID_CMD_ENG1_MANFLDI','ENG_MANIFOLD','Eng1_manifold: CANID 3: command');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT4','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 4 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT5','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 5 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT6','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 6 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT7','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 7 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_MANIFOLD','ENG_MAN_CANID_HW_FILT8','CANID_DUMMY', 'ENG_MANIFOLD','Eng1_manifold: CANID 8 add CAN hw filter for incoming msg');

--=============================================================================================================================================================================
-- ENGINE_RPM: Instance 1 = ENG1_RPM
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL               FUNCTION_TYPE      DESCRIPTION11
-- The CAN hw filter will be setFOR the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CRC'              ,'0',     'ENG_RPM','Eng1_rpm: CRC');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_VERSION'          ,'1',     'ENG_RPM','Eng1_rpm: Version number');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_HEARTBEAT_TIME_CT','1000',   'ENG_RPM','Eng1_rpm: Time (ms) between HB msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_SEG_CT'           ,'8',      'ENG_RPM','Eng1_rpm: Number of counts per two revolutions');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_POLL_RPM_R','CANID_MSG_ENG1_RPM','ENG_RPM','Eng1_rpm: CANID: rpm, calibrated');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_HB_RPM_R'  ,'CANID_HB_ENG1_RPM' ,'ENG_RPM','Eng1_rpm: CANID: Heartbeat: rpm, calibrated');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT1','CANID_HB_TIMESYNC'     ,'ENG_RPM','EPS_1: CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT2','CANID_MSG_TIME_POLL'   ,'ENG_RPM','MC: CANID:Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT3','CANID_CMD_ENG1_RPMI','ENG_RPM','Eng1_rpm: CANID: command');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT4','CANID_DUMMY', 'ENG_RPM','Eng1_rpm: CANID 4 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT5','CANID_DUMMY', 'ENG_RPM','Eng1_rpm: CANID 5 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT6','CANID_DUMMY', 'ENG_RPM','Eng1_rpm: CANID 6 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT7','CANID_DUMMY', 'ENG_RPM','Eng1_rpm: CANID 7 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_RPM','ENG_RPM_CANID_HW_FILT8','CANID_DUMMY', 'ENG_RPM','Eng1_rpm: CANID 8 add CAN hw filter for incoming msg');
--=============================================================================================================================================================================
-- ENGINE_THROTTLE: Instance 1 = ENG1_THROTTLE
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL               FUNCTION_TYPE      DESCRIPTION11
-- The CAN hw filter will be setFOR the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CRC'              ,'0',    'ENG_THROTTLE', 'Eng1_throttle:: CRC');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_VERSION'          ,'1',    'ENG_THROTTLE', 'Eng1_throttle:: Version number');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_HEARTBEAT_TIME_CT','1000', 'ENG_THROTTLE', 'Eng1_throttle:: Time (ms) between HB msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_OPENFULL'         ,'4045', 'ENG_THROTTLE', 'Eng1_throttle:: ADC when throttle full open');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CLOSED'           ,'50',   'ENG_THROTTLE', 'Eng1_throttle:: ADC when throttle closed');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_POLL_THROTTLE_R','CANID_MSG_ENG1_THROTTLE','ENG_THROTTLE','Eng1_throttle:: CANID: Poll response: throttle (0.0-100.0)');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_HB_THROTTLE_R'  ,'CANID_HB_ENG1_THROTTLE' ,'ENG_THROTTLE','Eng1_throttle:: CANID: Heartbeat: throttle (0.0-100.0)');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT1','CANID_HB_TIMESYNC'       ,'ENG_THROTTLE','EPS_1: CANID 1: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT2','CANID_MSG_TIME_POLL'     ,'ENG_THROTTLE','MC: CANID 2:Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT3','CANID_CMD_ENG1_THROTTLEI','ENG_THROTTLE','Eng1_throttle:: CANID 3: command');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT4','CANID_DUMMY', 'ENG_THROTTLE','Eng1_throttle:: CANID 4 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT5','CANID_DUMMY', 'ENG_THROTTLE','Eng1_throttle:: CANID 5 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT6','CANID_DUMMY', 'ENG_THROTTLE','Eng1_throttle:: CANID 6 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT7','CANID_DUMMY', 'ENG_THROTTLE','Eng1_throttle:: CANID 7 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_THROTTLE','ENG_THR_CANID_HW_FILT8','CANID_DUMMY', 'ENG_THROTTLE','Eng1_throttle:: CANID 8 add CAN hw filter for incoming msg');
--=============================================================================================================================================================================
-- ENGINE_TEMPERATURE #1: Instance 1 = ENG1_T1
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL               FUNCTION_TYPE      DESCRIPTION11
-- The CAN hw filter will be setFOR the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CRC'               ,'0'       ,'ENG_T1','Eng1_t1: CRC');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_VERSION'           ,'1'       ,'ENG_T1','Eng1_t1: Version number');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_HEARTBEAT_TIME_CT' ,'5000'    ,'ENG_T1','Eng1_t1: Time (ms) between HB msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CONST_B'           ,'3380'    ,'ENG_T1','Eng1_t1: Thermistor param: constant B');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_R_SERIES'          ,'7.63'    ,'ENG_T1','Eng1_t1: Thermistor param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_R_ROOMTMP'         ,'10.0'    ,'ENG_T1','Eng1_t1: Thermistor param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_REF_TEMP'          ,'290.0'   ,'ENG_T1','Eng1_t1: Thermistor param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_TEMP_OFFSET'       ,'0.5'     ,'ENG_T1','Eng1_t1: Thermistor param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_TEMP_SCALE'        ,'1.0'     ,'ENG_T1','Eng1_t1: Thermistor param: Thermistor temp scale correction');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_POLL_T1_R'      ,'CANID_MSG_ENG1_T1','ENG_T1','Eng1_t1: CANID: Poll response: CANID: Poll response: temperature #1 (deg C)');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_HB_T1_R'        ,'CANID_HB_ENG1_T1' ,'ENG_T1','Eng1_t1: CANID: Heartbeat: CANID: Heartbeat: temperature #1 (deg C)');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT1' ,'CANID_HB_TIMESYNC'  ,'ENG_T1','Eng1_t1: CANID 1 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT2' ,'CANID_MSG_TIME_POLL','ENG_T1','Eng1_t1: CANID 2 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT3' ,'CANID_CMD_ENG1_T1I' ,'ENG_T1','Eng1_t1: CANID 3 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT4' ,'CANID_DUMMY','ENG_T1','Eng1_t1: CANID 4 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT5' ,'CANID_DUMMY','ENG_T1','Eng1_t1: CANID 5 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT6' ,'CANID_DUMMY','ENG_T1','Eng1_t1: CANID 6 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT7' ,'CANID_DUMMY','ENG_T1','Eng1_t1: CANID 7 added to CAN hardware filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENG1_T1','ENG_T1_CANID_HW_FILT8' ,'CANID_DUMMY','ENG_T1','Eng1_t1: CANID 8 added to CAN hardware filter to allow incoming msg');
--============================================================================================================================================================================
-- ENGINE_SENSOR1
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL               FUNCTION_TYPE      DESCRIPTION11
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_CRC'                 ,'0'      ,'ENGINE_SENSOR','Eng1: CRC');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_VERSION'             ,'1'      ,'ENGINE_SENSOR','Eng1: Version number');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_HEARTBEAT_TIME_CT'   ,'1000'   ,'ENGINE_SENSOR','Eng1: Time (ms) between HB msg');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_SEG_CT'       ,'8'      ,'ENGINE_SENSOR','Eng1: Counts per revolution on engine');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_PRESS_OFFSET' ,'-0.5'   ,'ENGINE_SENSOR','Eng1: Manifold pressure offset');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_PRESS_SCALE'  ,'103.88' ,'ENGINE_SENSOR','Eng1: Manifold pressure  scale (inch Hg)');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_THERM1_CONST_B'      ,'3380'   ,'ENGINE_SENSOR','Eng1: Thermistor param: constant B');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_THERM1_R_SERIES'     ,'10.0'   ,'ENGINE_SENSOR','Eng1: Thermistor param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_THERM1_R_ROOMTMP'    ,'10.0'   ,'ENGINE_SENSOR','Eng1: Thermistor param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_THERM1_REF_TEMP'     ,'290.0'  ,'ENGINE_SENSOR','Eng1: Thermistor param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_THERM1_TEMP_OFFSET'  ,'0.0'    ,'ENGINE_SENSOR','Eng1: Thermistor param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_THERM1_TEMP_SCALE'   ,'1.0'    ,'ENGINE_SENSOR','Eng1: Thermistor param: Thermistor temp scale correction');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_THROTTLE_lO'         ,'500'    ,'ENGINE_SENSOR','Eng1: Throttle pot ADC reading: closed');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR1_THROTTLE_HI' ,'4000'   ,'ENGINE_SENSOR','Eng1: Throttle pot ADC reading: full open');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_POLL_RPMMANFLD_R'   ,'CANID_MSG_ENG1_RPMMANFLD_R','ENGINE_SENSOR','Eng1: CANID: rpm:manifold, calibrated');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_POLL_TEMPTHRTL_R'   ,'CANID_MSG_ENG1_TEMPTHRTL_R','ENGINE_SENSOR','Eng1: CANID: temperature:throttle pot, calibrated');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_HB_RPMMANFLD'       ,'CANID_HB_ENG1_RPMMANFLD'   ,'ENGINE_SENSOR','Eng1: CANID: raw readings: rpm:manifold pressure');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_HB_THRMTHRTL'       ,'CANID_HB_ENG1_THRMTHRTL'   ,'ENGINE_SENSOR','Eng1: CANID: raw readings: thermistor:throttle');
-- The CAN hardware filter will be set to allow the following *incoming** msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT1','CANID_HB_TIMESYNC'   ,'ENGINE_SENSOR','EPS_1: CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT2','CANID_MSG_TIME_POLL' ,'ENGINE_SENSOR','MC: CANID:Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT3','CANID_CMD_ENG1_I'    ,'ENGINE_SENSOR','Eng1: CANID: command');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT4','CANID_DUMMY', 'ENGINE_SENSOR','Eng1: 47 CANID 4 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT5','CANID_DUMMY', 'ENGINE_SENSOR','Eng1: 48 CANID 5 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT6','CANID_DUMMY', 'ENGINE_SENSOR','Eng1: 49 CANID 6 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT7','CANID_DUMMY', 'ENGINE_SENSOR','Eng1: 50 CANID 7 add CAN hw filter for incoming msg');
INSERT INTO PARAM_VAL VALUES ('ENGINE_SENSOR1','ENGINE_SENSOR_CANID_HW_FILT8','CANID_DUMMY', 'ENGINE_SENSOR','Eng1: 51 CANID 8 add CAN hw filter for incoming msg');
 --===========================================================================================================================================================================
-- ENCODERA_1 (H = high resolution) Upper sheave (../svn_discoveryf4/sensor/shaft_encode/trunk)
--                           FUNCTION_NAME     PARAM_NAME          PARAM_VAL         FUNCTION_TYPE      DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_LIST_CRC'		,'0',	   	 	'SHEAVE_H',	'Encoder_f4_1up: 1 CRC');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_LIST_VERSION'	,'1',      		'SHEAVE_H',	'Encoder_f4_1up: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_HEARTBEAT_CT'	,'250',			'SHEAVE_H',	'Encoder_f4_1up: 3 Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_CT_PER_REV'	,'720',			'SHEAVE_H',	'Encoder_f4_1up: 4 Number of counts per revolution');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_DIST_PER_REV'	,'0.558575174','SHEAVE_H',	'Encoder_f4_1up: 5 Distance per revolution (meters)');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_HEARTBEAT_MSG','CANID_HB_UPPER1_H_RAW','SHEAVE_H', 	'Encoder_f4_1up: 6 CANID: Heartbeat sends raw (long long) running encoder count');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_POLL_MSG'		,'CANID_MSG_UPPER1_H_CAL','SHEAVE_H', 	'Encoder_f4_1up: 7 CANID: hi-res: msg--upper calibrated--distance and speed');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_POLL'	    	,'CANID_DUMMY',		'SHEAVE_H',  	'Encoder_f4_1up: 8 CANID: Poll with time sync msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','ENCODER_POLL_R'	  	,'CANID_DUMMY',		'SHEAVE_H',	'Encoder_f4_1up: 9 CANID: Response to POLL');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',	'SHEAVE_H',	'Encoder_f4_1up: 10 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	'SHEAVE_H',	'Encoder_f4_1up: 11 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT3'    ,'CANID_TST_SHEAVE_1UP','SHEAVE_H',	'Encoder_f4_1up: 12 CANID 3 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT4'    ,'CANID_CMD_UPPER1_HI',	'SHEAVE_H',	'Encoder_f4_1up: 13 CANID 4 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT5'    ,'CANID_DUMMY', 	'SHEAVE_H',	'Encoder_f4_1up: 14 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT6'    ,'CANID_DUMMY',  	'SHEAVE_H',	'Encoder_f4_1up: 15 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'SHEAVE_H',	'Encoder_f4_1up: 16 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1UP','SHEAVE_H_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'SHEAVE_H',	'Encoder_f4_1up: 17 CANID 8 add CAN hw filter to allow incoming msg');
--===========================================================================================================================================================================
-- ENCODERA_1 (H = high resolution) Lower sheave (../svn_discoveryf4/sensor/shaft_encode/trunk)
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL               FUNCTION_TYPE      DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_LIST_CRC'		,'0',	   	 	'SHEAVE_H',	'Encoder_f4_1lo: 1 CRC');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_LIST_VERSION'	,'1',      		'SHEAVE_H',	'Encoder_f4_1lo: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_HEARTBEAT_CT'	,'250',			'SHEAVE_H',	'Encoder_f4_1lo: 3 Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_CT_PER_REV'		,'720',			'SHEAVE_H',	'Encoder_f4_1lo: 4 Number of counts per revolution');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_DIST_PER_REV'	,'0.558575174',		'SHEAVE_H',	'Encoder_f4_1lo: 5 Distance per revolution (meters)');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_HEARTBEAT_MSG'	,'CANID_HB_LOWER1_H_RAW','SHEAVE_H', 	'Encoder_f4_1lo: 6 CANID: Heartbeat sends raw (long long) running encoder count');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_POLL_MSG'		,'CANID_MSG_LOWER1_H_CAL','SHEAVE_H', 	'Encoder_f4_1lo: 7 CANID: hi-res: msg--lower calibrated--distance and speed');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_POLL'	    	,'CANID_DUMMY',		'SHEAVE_H',  	'Encoder_f4_1lo: 8 CANID: Poll with time sync msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','ENCODER_POLL_R'	  	,'CANID_DUMMY',		'SHEAVE_H',	'Encoder_f4_1lo: 9 CANID: Response to POLL');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',	'SHEAVE_H',	'Encoder_f4_1lo: 10 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	'SHEAVE_H',	'Encoder_f4_1lo: 11 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT3'    ,'CANID_TST_SHEAVE_1LO','SHEAVE_H',	'Encoder_f4_1lo: 12 CANID 3 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT4'    ,'CANID_CMD_LOWER1_HI','SHEAVE_H',	'Encoder_f4_1lo: 13 CANID 4 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT5'    ,'CANID_DUMMY', 	'SHEAVE_H',	'Encoder_f4_1lo: 14 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT6'    ,'CANID_DUMMY',  	'SHEAVE_H',	'Encoder_f4_1lo: 15 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'SHEAVE_H',	'Encoder_f4_1lo: 16 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('SHEAVE_1LO','SHEAVE_H_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'SHEAVE_H',	'Encoder_f4_1lo: 17 CANID 8 add CAN hw filter to allow incoming msg');
-- ===================================================================================================================================================================
-- SHAFT1 (drive shaft encoder, newer version (../svn_sensor/sensor.shaft/trunk)
--                           FUNCTION_NAME     PARAM_NAME    PARAM_VAL  FUNCTION_TYPE     DESCRIPTION11
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_LIST_CRC'       ,'0'     ,'DRIVE_SHAFT','Shaft1: CRC');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_LIST_VERSION'   ,'1'     ,'DRIVE_SHAFT','Shaft1: Version number');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_HEARTBEAT_CT'   ,'1000'  ,'DRIVE_SHAFT','Shaft1: Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_ADC3_HTR'       ,'900'   ,'DRIVE_SHAFT','Shaft1: High threshold register setting, ADC3');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_ADC3_LTR'       ,'400'   ,'DRIVE_SHAFT','Shaft1: Low  threshold register setting, ADC3');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_ADC2_HTR'       ,'1100'  ,'DRIVE_SHAFT','Shaft1: High threshold register setting, ADC2');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_ADC2_LTR'       ,'700'   ,'DRIVE_SHAFT','Shaft1: Low  threshold register setting, ADC2');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CID_MSG_SPEED'  ,'CANID_MSG_SHAFT1_SPEED','DRIVE_SHAFT','Shaft1: CANID: Shaft speed, calibrated, response to poll');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CID_HB_SPEED'   ,'CANID_HB_SHAFT1_SPEED' ,'DRIVE_SHAFT','Shaft1: CANID: Shaft speed, calibrated, heartbeat');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CID_MSG_CT'     ,'CANID_MSG_SHAFT1_COUNT','DRIVE_SHAFT','Shaft1: CANID: Shaft running counT, response to poll');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CID_HB_CT '     ,'CANID_HB_SHAFT1_COUNT' ,'DRIVE_SHAFT','Shaft1: CANID: Shaft running count, heartbeat');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_NUMBER_SEG'     ,'16',    'DRIVE_SHAFT','Shaft1: Number of segments on code wheel');
-- The CAN hardware filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT1' ,'CANID_HB_TIMESYNC'  ,'DRIVE_SHAFT','Shaft1: CANID 1: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT2' ,'CANID_MSG_TIME_POLL','DRIVE_SHAFT','Shaft1: CANID 2:Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT3' ,'CANID_CMD_SHAFT1I'  ,'DRIVE_SHAFT','Shaft1: CANID 3: command');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT4' ,'CANID_DUMMY', 'DRIVE_SHAFT','Shaft1: CANID 4 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT5' ,'CANID_DUMMY', 'DRIVE_SHAFT','Shaft1: CANID 5 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT6' ,'CANID_DUMMY', 'DRIVE_SHAFT','Shaft1: CANID 6 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT7' ,'CANID_DUMMY', 'DRIVE_SHAFT','Shaft1: CANID 7 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_CANID_HW_FILT8' ,'CANID_DUMMY', 'DRIVE_SHAFT','Shaft1: CANID 8 added to CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_IIR_HB_K'       , '4'     ,'DRIVE_SHAFT','Shaft1: IIR Filter factor: divisor sets time const: for heart-beat msg');
INSERT INTO PARAM_VAL VALUES('SHAFT1','SHAFT_IIR_HB_SCALE'   , '64'    ,'DRIVE_SHAFT','Shaft1: IIR Filter scale : upscaling (integer math): for heart-beat msg');
-- ==================================================================================================================================================================
-- CANSENDER_1
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL                FUNCTION_TYPE     DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('CANSENDER_1','CANSENDER_LIST_CRC'	   ,'0',	   	 	'CANSENDER',	'Cansender_1: 1 CRC');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_1','CANSENDER_LIST_VERSION'	,'1',      		'CANSENDER',	'Cansender_1: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_1','CANSENDER_HEARTBEAT_CT'	,'500',        'CANSENDER',	'Cansender_1: 3 Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_1','CANSENDER_HEARTBEAT_MSG','CANID_HB_CANSENDER_1'   ,'CANSENDER',  	'Cansender_1: 4 CANID: Hearbeat sends running count');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_1','CANSENDER_POLL'	    	,'CANID_POLL_CANSENDER'   ,'CANSENDER',  	'Cansender_1: 5 CANID: Poll this cansender');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_1','CANSENDER_POLL_R'       ,'CANID_POLLR_CANSENDER_1','CANSENDER',	'Cansender_1: 6 CANID: Response to POLL');
-- ==================================================================================================================================================================
-- CANSENDER_2
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL                FUNCTION_TYPE     DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('CANSENDER_2','CANSENDER_LIST_CRC'	,'0',                      'CANSENDER','Cansender_2: 1 CRC');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_2','CANSENDER_LIST_VERSION'	,'1',                   'CANSENDER','Cansender_2: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_2','CANSENDER_HEARTBEAT_CT'	,'375',			         'CANSENDER','Cansender_2: 3 Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_2','CANSENDER_HEARTBEAT_MSG','CANID_HB_CANSENDER_2','CANSENDER','Cansender_2: 4 CANID: Hearbeat sends running count');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_2','CANSENDER_POLL'	    	,'CANID_POLL_CANSENDER','CANSENDER','Cansender_2: 5 CANID: Poll this cansender');
INSERT INTO PARAM_VAL VALUES ('CANSENDER_2','CANSENDER_POLL_R'	  	,'CANID_POLLR_CANSENDER_2','CANSENDER','Cansender_2: 6 CANID: Response to POLL');
-- ==================================================================================================================================================================
-- GPS_1
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_LIST_CRC'              ,'0', 		'GPS',		'GPS_1: 1 CRC ');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_LIST_VERSION'          ,'1', 		'GPS',		'GPS_1: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_HEARTBEAT_TIME_CT'     ,'1000', 	'GPS',		'GPS_1: 3 Time (ms) between unix time HB msgs');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_HEARTBEAT_LLH_CT'      ,'10000', 	'GPS',		'GPS_1: 4 Time (ms) between burst of lat lon height HB msgs');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_HEARTBEAT_LLH_DELAY_CT','1100',	   'GPS',		'GPS_1: 5 Time (ms) between lat/lon and lon/ht msgs');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_HEARTBEAT_TIME'        ,'CANID_HB_GPS_TIME_1','GPS','GPS_1: 6 Heartbeat unix time');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_HEARTBEAT_LLH'         ,'CANID_HB_GPS_LLH_1' ,'GPS','GPS_1: 7 Heartbeat (3 separate msgs) lattitude longitude height');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_DISABLE_SYNCMSGS'      ,'0'                  ,'GPS','GPS_1: 8 time sync msgs: 0 = enable  1 = disable');
INSERT INTO PARAM_VAL VALUES ('GPS_1','GPS_TIME_SYNC_MSG'         ,'CANID_HB_TIMESYNC'  ,'GPS','GPS_1: 9 Time sync msg');
--==================================================================================================================================================================
-- LOGGER_1
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('LOGGER_1','LOGGER_LIST_CRC'        ,'0',              'LOGGER','Logger_1: 1 CRC ');
INSERT INTO PARAM_VAL VALUES ('LOGGER_1','LOGGER_LIST_VERSION'    ,'1',              'LOGGER','Logger_1: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('LOGGER_1','LOGGER_HEARTBEAT1_CT'   ,'8000',           'LOGGER','Logger_1: 3 Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES ('LOGGER_1','LOGGER_HEARTBEAT_MSG' ,'CANID_HB_LOGGER_1','LOGGER','Logger_1: 4 CANID: Hearbeat sends running count of logged msgs');
INSERT INTO PARAM_VAL VALUES ('LOGGER_1','LOGGER_SUSPEND_R'     ,'CANID_ALM_PWRBOX1','LOGGER','Logger_1: 5 CANID: Low voltage alarm msg suspends logging ');
INSERT INTO PARAM_VAL VALUES ('LOGGER_1','LOGGER_SUSPEND_CT'      ,'750'            ,'LOGGER','Logger_1: 6 Suspend duration (ms) after receiving alarm msg');
--===================================================================================================================================================================
-- GPS_2
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_LIST_CRC'               ,'0'               ,'GPS','GPS_2: 1 CRC ');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_LIST_VERSION'      		 ,'1'               ,'GPS','GPS_2: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_HEARTBEAT_TIME_CT'		 ,'1000'            ,'GPS','GPS_2: 3 Time (ms) between unix time HB msgs');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_HEARTBEAT_LLH_CT'		 ,'10000'           ,'GPS','GPS_2: 4 Time (ms) between burst of lat lon height HB msgs');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_HEARTBEAT_LLH_DELAY_CT' ,'1050'            ,'GPS','GPS_2: 5 Time (ms) between lat/lon and lon/ht msgs');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_HEARTBEAT_TIME'     ,'CANID_HB_GPS_TIME_2' ,'GPS','GPS_2: 6 Heartbeat unix time');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_HEARTBEAT_LLH'      ,'CANID_HB_GPS_LLH_2'  ,'GPS','GPS_2: 7 Heartbeat (3 separate msgs) lattitude longitude height');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_DISABLE_SYNCMSGS'   ,'0'                   ,'GPS','GPS_2: 8 time sync msgs: 0 = enable  1 = disable');
INSERT INTO PARAM_VAL VALUES ('GPS_2','GPS_TIME_SYNC_MSG'      ,'CANID_HB_TIMESYNC' ,'GPS','GPS_2: 9 Time sync msg');
--===================================================================================================================================================================
-- LOGGER_2
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('LOGGER_2','LOGGER_LIST_CRC'        ,'0',  		       'LOGGER','Logger_2: 1 CRC ');
INSERT INTO PARAM_VAL VALUES ('LOGGER_2','LOGGER_LIST_VERSION'    ,'1', 		       'LOGGER','Logger_2: 2 Version number');
INSERT INTO PARAM_VAL VALUES ('LOGGER_2','LOGGER_HEARTBEAT1_CT'   ,'8000', 	       'LOGGER','Logger_2: 3 Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_VAL VALUES ('LOGGER_2','LOGGER_HEARTBEAT_MSG' ,'CANID_HB_LOGGER_2','LOGGER','Logger_2: 4 CANID: Hearbeat sends running count of logged msgs');
INSERT INTO PARAM_VAL VALUES ('LOGGER_2','LOGGER_SUSPEND_R'     ,'CANID_ALM_PWRBOX1','LOGGER','Logger_2: 5 CANID: Low voltage alarm msg suspends logging ');
INSERT INTO PARAM_VAL VALUES ('LOGGER_2','LOGGER_SUSPEND_CT'      ,'750'            ,'LOGGER','Logger_2: 6 Suspend duration (ms) after receiving alarm msg');
--===================================================================================================================================================================
-- TENSION_a11
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_LIST_CRC',   	         '0', 'TENSION_a',  'Tension_a11: 1 CRC for tension list');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_LIST_VERSION',         '1',	'TENSION_a',  'Tension_a11: 2 Version number for Tension List');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_AD7799_1_OFFSET',      '0',  'TENSION_a',  'Tension_a11: 3 AD7799 final offset');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_AD7799_1_SCALE',  '0.11078E-3','TENSION_a', 'Tension_a11: 4 AD7799 final Scale (kgf) FS 250kgf 2mv/v');

INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a11:  5 Thermistor1 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_R_SERIES' ,   '10.0',	'TENSION_a',  'Tension_a11:  6 Thermistor1 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a11:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a11:  8 Thermistor1 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a11:  9 Thermistor1 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a11: 10 Thermistor1 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a11: 11 Thermistor2 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_R_SERIES',    '10.0',	'TENSION_a',  'Tension_a11: 12 Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a11: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a11: 14 Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a11: 15 Thermistor2 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a11: 16 Thermistor2 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a11: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a11: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a11: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM1_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a11: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a11: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a11: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a11: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_THERM2_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a11: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_HEARTBEAT_CT',     '250',	'TENSION_a',  'Tension_a11: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_DRUM_NUMBER',        '1',	'TENSION_a',  'Tension_a11: 26 Drum sys number for this function instance');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_DRUM_FUNCTION_BIT',  '1',	'TENSION_a',  'Tension_a11: 27 Drum sys poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_DRUM_POLL_BIT',      '1',	'TENSION_a',  'Tension_a11: 28 Drum sys poll 1st payload byte bit for drum # (function instance)');
-- These don't have be grouped together, but it does make a bit easier for the reader
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANPRM_TENSION'  ,'CANID_MSG_TENSION_a11', 'TENSION_a',  'Tension_a11: 29 CANID: can msg tension for AD7799 #1');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_MSG_TIME_POLL'   ,'CANID_MSG_TIME_POLL',	  'TENSION_a',  'Tension_a11: 30 CANID: MC: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_TIMESYNC'        ,'CANID_HB_TIMESYNC', 	  'TENSION_a',  'Tension_a11: 31 CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_HEARTBEAT'       ,'CANID_HB_TENSION_a11',  'TENSION_a',  'Tension_a11: 32 CANID: Heartbeat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANIDTEST'       ,'CANID_TST_TENSION_a11', 'TENSION_a',  'Tension_a11: 33 Test');
-- Tacking additions on the end avoids the trouble of renumbering the sequence numbers
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_IIR_POLL_K'        , '04',	'TENSION_a',  	'Tension_a11: 34 IIR Filter factor: divisor sets time const: reading for polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_IIR_POLL_SCALE'    , '128',	'TENSION_a',  	'Tension_a11: 35 Filter scale : upscaling (due to integer math): for polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_IIR_HB_K'          , '100',	'TENSION_a',  	'Tension_a11: 36 IIR Filter factor: divisor sets time const: reading for heart-beat  msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_IIR_HB_SCALE'      , '128',	'TENSION_a',  	'Tension_a11: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg');
--
-- TENSION_a_IIR_USEME should be the same for all functions in the same unit.  The bit tells the program if this function instance is to be used as the program may support more
-- instances of the function than the particular hw provides, e.g. only one AD7799.
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_USEME'    	  , '3',		'TENSION_a',  	'Tension_a11: 38 skip or use this function swit ch');
-- Internal zero recalibration.  New readings are filtered and used for current conversions.
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_IIR_Z_RECAL_K'     , '10',   	'TENSION_a',  	'Tension_a11: 39 IIR Filter factor: zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_IIR_Z_RECAL_SCALE' , '64',  	'TENSION_a',  	'Tension_a11: 40 IIR Filter scale : zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_Z_RECAL_CT'        ,'470',  	'TENSION_a',  	'Tension_a11: 41 ADC conversion counts between zero recalibrations');
-- Limits for reasonable readings.
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_LIMIT_HI'         ,'1200.0',	'TENSION_a',  	'Tension_a11: 42 Exceeding this calibrated limit (+) means invalid reading');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_LIMIT_LO'         ,'-700.0', 	'TENSION_a',  	'Tension_a11: 43 Exceeding this calibrated limit (-) means invalid reading');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',		'TENSION_a',	'Tension_a11: 44 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	'TENSION_a',	'Tension_a11: 45 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT3'    ,'CANID_TST_TENSION_a11',	'TENSION_a',	'Tension_a11: 46 CANID 3 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT4'    ,'CANID_CMD_TENSION_a11I', 	'TENSION_a',	'Tension_a11: 47 CANID 4 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT5'    ,'CANID_DUMMY', 	  	'TENSION_a',	'Tension_a11: 48 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT6'    ,'CANID_DUMMY',  		'TENSION_a',	'Tension_a11: 49 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a11: 50 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a11','TENSION_a_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a11: 51 CANID 8 add CAN hw filter to allow incoming msg');
--
--======================================================================================================================================================================
-- TENSION_a21
-- Second AD7799 on the same board, i.e. AD7799 #2(b)
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_LIST_CRC',   		'0',	'TENSION_a',  'Tension_a21:   1 CRC for tension list');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_LIST_VERSION',    	'1',	'TENSION_a',  'Tension_a21:   2 Version number for Tension List');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_AD7799_1_OFFSET',     '5076',	'TENSION_a',  'Tension_a21:   3 AD7799 offset');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_AD7799_1_SCALE',  '0.2954E-3', 	'TENSION_a',  'Tension_a21: AD7799 #1 Scale (kgf) FS 1000kgf 3mv/v');

INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a21:  5 Thermistor1 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_R_SERIES' ,   '10.0',	'TENSION_a',  'Tension_a21:  6 Thermistor1 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a21:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a21:  8 Thermistor1 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a21:  9 Thermistor1 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a21: 10 Thermistor1 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a21: 11 Thermistor2 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_R_SERIES',    '10.0',	'TENSION_a',  'Tension_a21: 12 Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a21: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a21: 14 Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a21: 15 Thermistor2 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a21: 16 Thermistor2 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a21: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a21: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a21: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM1_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a21: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a21: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a21: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a21: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_THERM2_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a21: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_HEARTBEAT_CT',       '1000',	'TENSION_a',  'Tension_a21: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_DRUM_NUMBER',           '1', 	'TENSION_a',  'Tension_a21: 26 Drum sys number for this function instance');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_DRUM_FUNCTION_BIT',     '1',	'TENSION_a',  'Tension_a21: 27 Drum sys poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_DRUM_POLL_BIT',         '1',	'TENSION_a',  'Tension_a21: 28 Drum sys poll 1st payload byte bit for drum # (function instance)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANPRM_TENSION'  ,'CANID_MSG_TENSION_a21','TENSION_a',  'Tension_a21: 29 CANID: can msg tension for AD7799 #2');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_MSG_TIME_POLL'   ,'CANID_MSG_TIME_POLL',	 'TENSION_a',  'Tension_a21: 30 CANID: MC: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_TIMESYNC'        ,'CANID_HB_TIMESYNC', 	 'TENSION_a',  'Tension_a21: 31 CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_HEARTBEAT'       ,'CANID_HB_TENSION_a21', 'TENSION_a',  'Tension_a21: 32 CANID: Heartbeat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANIDTEST'       ,'CANID_TST_TENSION_a21','TENSION_a',  'Tension_a21: 33 Test');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_IIR_POLL_K'        , '04',	'TENSION_a',  	'Tension_a21: 34 IIR Filter factor: divisor sets time const: reading polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_IIR_POLL_SCALE'    , '128',	'TENSION_a',  	'Tension_a21: 35 Filter scale : upscaling (due to integer math): polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_IIR_HB_K'          , '512',	'TENSION_a',  	'Tension_a21: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_IIR_HB_SCALE'      , '128',	'TENSION_a',  	'Tension_a21: 37 Filter scale : upscaling (due to integer math): heart-beat  msg');

-- instances of the function than the particular hw provides, e.g. only one AD7799.
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_USEME'    	 , '3',		'TENSION_a',  	'Tension_a: 38 skip or use this function switch');
-- Internal zero recalibration.  New readings are filtered and used for current conversions.
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_IIR_Z_RECAL_K'     , '10',   	'TENSION_a',  	'Tension_a21: 39 IIR Filter factor: zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_IIR_Z_RECAL_SCALE' , '128',  	'TENSION_a',  	'Tension_a21: 40 IIR Filter scale : zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_Z_RECAL_CT'        ,'470',  	'TENSION_a',  	'Tension_a21: 41 ADC conversion counts between zero recalibrations');
-- Limits for reasonable readings.
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_LIMIT_HI'         ,'1200.0',	'TENSION_a',  	'Tension_a21: 42 Exceeding this calibrated limit (+) means invalid reading');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_LIMIT_LO'         ,'-700.0', 	'TENSION_a',  	'Tension_a21: 43 Exceeding this calibrated limit (-) means invalid reading');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',		'TENSION_a',	'Tension_a21: 44 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	'TENSION_a',	'Tension_a21: 45 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT3'    ,'CANID_TST_TENSION_a21',	'TENSION_a',	'Tension_a21: 46 CANID 3 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT4'    ,'CANID_CMD_TENSION_a21I',  	'TENSION_a',	'Tension_a21: 47 CANID 4 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT5'    ,'CANID_DUMMY', 	  	'TENSION_a',	'Tension_a21: 48 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT6'    ,'CANID_DUMMY',  		'TENSION_a',	'Tension_a21: 49 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a21: 50 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a21','TENSION_a_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a21: 51 CANID 8 add CAN hw filter to allow incoming msg');
--
--====================================================================================================================================================================
-- TENSION_a12
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_LIST_CRC',   		'0',	'TENSION_a',  'Tension_a12:   1 CRC for tension list');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_LIST_VERSION',    	'1',	'TENSION_a',  'Tension_a12:   2 Version number for Tension List');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_AD7799_1_OFFSET',     '4789',	'TENSION_a',  'Tension_a12:   3 AD7799 offset');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_AD7799_1_SCALE',  '0.283543155E-2', 'TENSION_a',  'Tension_a12: AD7799 #1 Scale (convert to kgf)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a12:  5 Thermistor1 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_R_SERIES' ,   '10.0',	'TENSION_a',  'Tension_a12:  6 Thermistor1 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a12:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a12:  8 Thermistor1 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a12:  9 Thermistor1 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a12: 10 Thermistor1 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a12: 11 Thermistor2 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_R_SERIES',    '10.0',	'TENSION_a',  'Tension_a12: 12 Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a12: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a12: 14 Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a12: 15 Thermistor2 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a12: 16 Thermistor2 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a12: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a12: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a12: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM1_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a12: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a12: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a12: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a12: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_THERM2_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a12: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_HEARTBEAT_CT',       '2000',	'TENSION_a',  'Tension_a12: 25 Heart-Beat: Count of time ticks between autonomous msgs');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_DRUM_NUMBER',           '2', 	'TENSION_a',  'Tension_a12: 26 Drum sys number for this function instance');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_DRUM_FUNCTION_BIT',     '1',	'TENSION_a',  'Tension_a12: 27 Drum sys poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_DRUM_POLL_BIT',         '2',	'TENSION_a',  'Tension_a12: 28 Drum sys poll 1st payload byte bit for drum # (function instance)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANPRM_TENSION'  ,'CANID_MSG_TENSION_a12','TENSION_a',  'Tension_a12: 29 CANID: can msg tension for AD7799 #2');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_MSG_TIME_POLL'   ,'CANID_MSG_TIME_POLL',	 'TENSION_a',  'Tension_a12: 30 CANID: MC: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_TIMESYNC'        ,'CANID_HB_TIMESYNC', 	 'TENSION_a',  'Tension_a12: 31 CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_HEARTBEAT'       ,'CANID_HB_TENSION_a12', 'TENSION_a',  'Tension_a12: 32 CANID: Heartbeat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANIDTEST'       ,'CANID_TST_TENSION_a12','TENSION_a',  'Tension_a12: 33 Test');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_IIR_POLL_K'        , '04',	'TENSION_a',  	'Tension_a12: 34 IIR Filter factor: divisor sets time const: reading polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_IIR_POLL_SCALE'    , '128',	'TENSION_a',  	'Tension_a12: 35 Filter scale : upscaling (due to integer math): polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_IIR_HB_K'          , '512',	'TENSION_a',  	'Tension_a12: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_IIR_HB_SCALE'      , '128',	'TENSION_a',  	'Tension_a12: 37 Filter scale : upscaling (due to integer math): heart-beat  msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_USEME'    	 , '3',		'TENSION_a',  	'Tension_a12: 38 skip or use this function switch');
-- Internal zero recalibration.  New readings are filtered and used for current conversions.
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_IIR_Z_RECAL_K'     , '10',   	'TENSION_a',  	'Tension_a12: 39 IIR Filter factor: zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_IIR_Z_RECAL_SCALE' , '128',  	'TENSION_a',  	'Tension_a12: 40 IIR Filter scale : zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_Z_RECAL_CT'        ,'470',  	'TENSION_a',  	'Tension_a12: 41 ADC conversion counts between zero recalibrations');
-- Limits for reasonable readings.
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_LIMIT_HI'         ,'1200.0',	'TENSION_a',  	'Tension_a12: 42 Exceeding this calibrated limit (+) means invalid reading');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_LIMIT_LO'         ,'-700.0', 	'TENSION_a',  	'Tension_a12: 43 Exceeding this calibrated limit (-) means invalid reading');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',		'TENSION_a',	'Tension_a12: 44 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	'TENSION_a',	'Tension_a12: 45 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT3'    ,'CANID_TST_TENSION_a12',	'TENSION_a',	'Tension_a12: 46 CANID 3 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT4'    ,'CANID_CMD_TENSION_a12I', 	'TENSION_a',	'Tension_a12: 47 CANID 4 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT5'    ,'CANID_DUMMY', 	  	'TENSION_a',	'Tension_a12: 48 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT6'    ,'CANID_DUMMY',  		'TENSION_a',	'Tension_a12: 49 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a12: 50 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a12','TENSION_a_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a12: 51 CANID 8 add CAN hw filter to allow incoming msg');
--
--===================================================================================================================================================================
-- TENSION_a22
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_LIST_CRC',   		'0',	'TENSION_a',  'Tension_a22:   1 CRC for tension list');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_LIST_VERSION',    	'1',	'TENSION_a',  'Tension_a22:   2 Version number for Tension List');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_AD7799_1_OFFSET',     '4789',	'TENSION_a',  'Tension_a22:   3 AD7799 offset');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_AD7799_1_SCALE',  '0.283543155E-2', 'TENSION_a',  'Tension_a22: AD7799 #1 Scale (convert to kgf)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a22:  5 Thermistor1 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_R_SERIES' ,   '10.0',	'TENSION_a',  'Tension_a22:  6 Thermistor1 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a22:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a22:  8 Thermistor1 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a22:  9 Thermistor1 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a22: 10 Thermistor1 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a22: 11 Thermistor2 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_R_SERIES',    '10.0',	'TENSION_a',  'Tension_a22: 12 Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a22: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a22: 14 Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a22: 15 Thermistor2 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a22: 16 Thermistor2 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a22: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a22: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a22: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM1_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a22: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a22: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a22: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a22: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_THERM2_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a22: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_HEARTBEAT_CT',       '8192',	'TENSION_a',  'Tension_a22: 25 Heart-Beat: Count of time ticks between autonomous msgs');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_DRUM_NUMBER',           '2', 	'TENSION_a',  'Tension_a22: 26 Drum sys number for this function instance');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_DRUM_FUNCTION_BIT',     '1',	'TENSION_a',  'Tension_a22: 27 Drum sys poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_DRUM_POLL_BIT',         '0',	'TENSION_a',  'Tension_a22: 28 Drum sys poll 1st payload byte bit for drum # (function instance)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANPRM_TENSION'  ,'CANID_MSG_TENSION_a22','TENSION_a',  'Tension_a22: 29 CANID: can msg tension for AD7799 #2');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_MSG_TIME_POLL'   ,'CANID_MSG_TIME_POLL',	 'TENSION_a',  'Tension_a22: 30 CANID: MC: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_TIMESYNC'        ,'CANID_HB_TIMESYNC', 	 'TENSION_a',  'Tension_a22: 31 CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_HEARTBEAT'       ,'CANID_HB_TENSION_a22', 'TENSION_a',  'Tension_a22: 32 CANID: Heartbeat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANIDTEST'       ,'CANID_TST_TENSION_a12','TENSION_a',  'Tension_a22: 33 Test');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_IIR_POLL_K'        , '04',	'TENSION_a',  	'Tension_a22: 34 IIR Filter factor: divisor sets time const: reading polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_IIR_POLL_SCALE'    , '128',	'TENSION_a',  	'Tension_a22: 35 Filter scale : upscaling (due to integer math): polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_IIR_HB_K'          , '512',	'TENSION_a',  	'Tension_a22: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_IIR_HB_SCALE'      , '128',	'TENSION_a',  	'Tension_a22: 37 Filter scale : upscaling (due to integer math): heart-beat  msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_USEME'    	 , '3',		'TENSION_a',  	'Tension_a22: 38 skip or use this function switch');
-- Internal zero recalibration.  New readings are filtered and used for current conversions.
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_IIR_Z_RECAL_K'     , '10',   	'TENSION_a',  	'Tension_a22: 39 IIR Filter factor: zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_IIR_Z_RECAL_SCALE' , '128',  	'TENSION_a',  	'Tension_a22: 40 IIR Filter scale : zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_Z_RECAL_CT'        ,'470',  	'TENSION_a',  	'Tension_a22: 41 ADC conversion counts between zero recalibrations');
-- Limits for reasonable readings.
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_LIMIT_HI'         ,'1200.0',	'TENSION_a',  	'Tension_a22: 42 Exceeding this calibrated limit (+) means invalid reading');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_LIMIT_LO'         ,'-700.0', 	'TENSION_a',  	'Tension_a22: 43 Exceeding this calibrated limit (-) means invalid reading');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',		 'TENSION_a', 'Tension_a22: 44 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	 'TENSION_a', 'Tension_a22: 45 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT3'    ,'CANID_TST_TENSION_a22', 'TENSION_a', 'Tension_a22: 46 CANID 3 add CAN hw filter allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT4'    ,'CANID_CMD_TENSION_a22I','TENSION_a', 'Tension_a22: 47 CANID 4 add CAN hw filter allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT5'    ,'CANID_DUMMY', 	'TENSION_a','Tension_a22: 48 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT6'    ,'CANID_DUMMY',  	'TENSION_a','Tension_a22: 49 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'TENSION_a','Tension_a22: 50 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a22','TENSION_a_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'TENSION_a','Tension_a22: 51 CANID 8 add CAN hw filter to allow incoming msg');
--
--===================================================================================================================================================================
-- TENSION_a1G (First AD7799 port, first POD unit shipped to GSM, strapped to plywood sheet)
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_LIST_CRC',   	            '0','TENSION_a','Tension_a1G: 1 CRC for tension list');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_LIST_VERSION',            '1','TENSION_a','Tension_a1G: 2 Version number for Tension List');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_AD7799_1_OFFSET',         '0','TENSION_a','Tension_a1G: 3 AD7799 final offset');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_AD7799_1_SCALE', '0.11078E-3','TENSION_a','Tension_a1G: 4 AD7799 final Scale (kgf) FS 250kgf 2mv/v');

INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a1G:  5 Thermistor1 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_R_SERIES' ,   '10.0',	'TENSION_a',  'Tension_a1G:  6 Thermistor1 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a1G:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a1G:  8 Thermistor1 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a1G:  9 Thermistor1 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a1G: 10 Thermistor1 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a1G: 11 Thermistor2 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_R_SERIES',    '10.0',	'TENSION_a',  'Tension_a1G: 12 Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a1G: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a1G: 14 Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a1G: 15 Thermistor2 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a1G: 16 Thermistor2 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a1G: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a1G: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a1G: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM1_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a1G: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a1G: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a1G: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a1G: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_THERM2_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a1G: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_HEARTBEAT_CT',     '250',	'TENSION_a',  'Tension_a1G: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_DRUM_NUMBER',        '1',	'TENSION_a',  'Tension_a1G: 26 Drum sys number for this function instance');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_DRUM_FUNCTION_BIT',  '1',	'TENSION_a',  'Tension_a1G: 27 Drum sys poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_DRUM_POLL_BIT',      '1',	'TENSION_a',  'Tension_a1G: 28 Drum sys poll 1st payload byte bit for drum # (function instance)');
-- These don't have be grouped together, but it does make a bit easier for the reader
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANPRM_TENSION'  ,'CANID_MSG_TENSION_a1G', 'TENSION_a',  'Tension_a1G: 29 CANID: can msg tension for AD7799 #1');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_MSG_TIME_POLL'   ,'CANID_MSG_TIME_POLL',	  'TENSION_a',  'Tension_a1G: 30 CANID: MC: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_TIMESYNC'        ,'CANID_HB_TIMESYNC', 	  'TENSION_a',  'Tension_a1G: 31 CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_HEARTBEAT'       ,'CANID_HB_TENSION_a1G',  'TENSION_a',  'Tension_a1G: 32 CANID: Heartbeat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANIDTEST'       ,'CANID_TST_TENSION_a1G', 'TENSION_a',  'Tension_a1G: 33 Test');
-- Tacking additions on the end avoids the trouble of renumbering the sequence numbers
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_IIR_POLL_K'        , '04',	'TENSION_a',  	'Tension_a1G: 34 IIR Filter factor: divisor sets time const: reading for polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_IIR_POLL_SCALE'    , '128',	'TENSION_a',  	'Tension_a1G: 35 Filter scale : upscaling (due to integer math): for polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_IIR_HB_K'          , '100',	'TENSION_a',  	'Tension_a1G: 36 IIR Filter factor: divisor sets time const: reading for heart-beat  msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_IIR_HB_SCALE'      , '128',	'TENSION_a',  	'Tension_a1G: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg');
--
-- TENSION_a_IIR_USEME should be the same for all functions in the same unit.  The bit tells the program if this function instance is to be used as the program may support more
-- instances of the function than the particular hw provides, e.g. only one AD7799.
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_USEME'    	  , '3',		'TENSION_a',  	'Tension_a1G: 38 skip or use this function swit ch');
-- Internal zero recalibration.  New readings are filtered and used for current conversions.
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_IIR_Z_RECAL_K'     , '10',   	'TENSION_a',  	'Tension_a1G: 39 IIR Filter factor: zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_IIR_Z_RECAL_SCALE' , '64',  	'TENSION_a',  	'Tension_a1G: 40 IIR Filter scale : zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_Z_RECAL_CT'        ,'470',  	'TENSION_a',  	'Tension_a1G: 41 ADC conversion counts between zero recalibrations');
-- Limits for reasonable readings.
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_LIMIT_HI'         ,'1200.0',	'TENSION_a',  	'Tension_a1G: 42 Exceeding this calibrated limit (+) means invalid reading');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_LIMIT_LO'         ,'-700.0', 	'TENSION_a',  	'Tension_a1G: 43 Exceeding this calibrated limit (-) means invalid reading');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',		'TENSION_a',	'Tension_a1G: 44 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	'TENSION_a',	'Tension_a1G: 45 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT3'    ,'CANID_TST_TENSION_a1G',	'TENSION_a',	'Tension_a1G: 46 CANID 3 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT4'    ,'CANID_CMD_TENSION_a1GI', 	'TENSION_a',	'Tension_a1G: 47 CANID 4 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT5'    ,'CANID_DUMMY', 	  	'TENSION_a',	'Tension_a1G: 48 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT6'    ,'CANID_DUMMY',  		'TENSION_a',	'Tension_a1G: 49 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a1G: 50 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a1G','TENSION_a_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'TENSION_a',	'Tension_a1G: 51 CANID 8 add CAN hw filter to allow incoming msg');
--
--======================================================================================================================================================================
-- TENSION_a2G  (Second AD7799 port, first POD unit shipped to GSM, strapped to plywood sheet)
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_LIST_CRC',               '0','TENSION_a','Tension_a2G: 1 CRC for tension list');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_LIST_VERSION',           '1','TENSION_a','Tension_a2G: 2 Version number for Tension List');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_AD7799_1_OFFSET',     '5076','TENSION_a','Tension_a2G: 3 AD7799 offset');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_AD7799_1_SCALE', '0.2954E-3','TENSION_a','Tension_a2G: 4 AD7799 #1 Scale (kgf) FS 1000kgf 3mv/v');

INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_CONST_B',   '3380.0',	'TENSION_a','Tension_a2G:  5 Thermistor1 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_R_SERIES' ,   '10.0',	'TENSION_a','Tension_a2G:  6 Thermistor1 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_R_ROOMTMP',   '10.0',	'TENSION_a','Tension_a2G:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_REF_TEMP',   '290.0',	'TENSION_a','Tension_a2G:  8 Thermistor1 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_TEMP_OFFSET',  '0.0',	'TENSION_a','Tension_a2G:  9 Thermistor1 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_TEMP_SCALE',   '1.0',	'TENSION_a','Tension_a2G: 10 Thermistor1 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_CONST_B',   '3380.0',	'TENSION_a',  'Tension_a2G: 11 Thermistor2 param: const B');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_R_SERIES',    '10.0',	'TENSION_a',  'Tension_a2G: 12 Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_R_ROOMTMP',   '10.0',	'TENSION_a',  'Tension_a2G: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_REF_TEMP',   '290.0',	'TENSION_a',  'Tension_a2G: 14 Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_TEMP_OFFSET',  '0.0',	'TENSION_a',  'Tension_a2G: 15 Thermistor2 param: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_TEMP_SCALE',   '1.0',	'TENSION_a',  'Tension_a2G: 16 Thermistor2 param: Thermistor temp scale correction');

INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a2G: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a2G: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a2G: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM1_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a2G: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_COEF_0',	 '5.0', 	'TENSION_a',  'Tension_a2G: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_COEF_1',	'1.13', 	'TENSION_a',  'Tension_a2G: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_COEF_2',	 '0.0', 	'TENSION_a',  'Tension_a2G: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_THERM2_COEF_3',	 '0.0', 	'TENSION_a',  'Tension_a2G: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_HEARTBEAT_CT',       '1000',	'TENSION_a',  'Tension_a2G: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_DRUM_NUMBER',           '1', 'TENSION_a',  'Tension_a2G: 26 Drum sys number for this function instance');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_DRUM_FUNCTION_BIT',     '1',	'TENSION_a',  'Tension_a2G: 27 Drum sys poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_DRUM_POLL_BIT',         '1',	'TENSION_a',  'Tension_a2G: 28 Drum sys poll 1st payload byte bit for drum # (function instance)');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANPRM_TENSION'  ,'CANID_MSG_TENSION_a2G','TENSION_a',  'Tension_a2G: 29 CANID: can msg tension for AD7799 #2');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_MSG_TIME_POLL'   ,'CANID_MSG_TIME_POLL',	 'TENSION_a',  'Tension_a2G: 30 CANID: MC: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_TIMESYNC'        ,'CANID_HB_TIMESYNC', 	 'TENSION_a',  'Tension_a2G: 31 CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_HEARTBEAT'       ,'CANID_HB_TENSION_a2G', 'TENSION_a',  'Tension_a2G: 32 CANID: Heartbeat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANIDTEST'       ,'CANID_TST_TENSION_a2G','TENSION_a',  'Tension_a2G: 33 Test');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_IIR_POLL_K'        , '04',	'TENSION_a','Tension_a2G: 34 IIR Filter factor: divisor sets time const: reading polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_IIR_POLL_SCALE'    , '128',	'TENSION_a','Tension_a2G: 35 Filter scale : upscaling (due to integer math): polled msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_IIR_HB_K'          , '512',	'TENSION_a','Tension_a2G: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_IIR_HB_SCALE'      , '128',	'TENSION_a','Tension_a2G: 37 Filter scale : upscaling (due to integer math): heart-beat  msg');

-- instances of the function than the particular hw provides, e.g. only one AD7799.
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_USEME'    	 , '3',		'TENSION_a',  	'Tension_a: 38 skip or use this function switch');
-- Internal zero recalibration.  New readings are filtered and used for current conversions.
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_IIR_Z_RECAL_K'     , '10', 'TENSION_a', 	'Tension_a2G: 39 IIR Filter factor: zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_IIR_Z_RECAL_SCALE' , '128','TENSION_a', 	'Tension_a2G: 40 IIR Filter scale : zero recalibration');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_Z_RECAL_CT'        ,'470', 'TENSION_a', 	'Tension_a2G: 41 ADC conversion counts between zero recalibrations');
-- Limits for reasonable readings.
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_LIMIT_HI'         ,'1200.0',	'TENSION_a','Tension_a2G: 42 Exceeding this calibrated limit (+) means invalid reading');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_LIMIT_LO'         ,'-700.0', 'TENSION_a','Tension_a2G: 43 Exceeding this calibrated limit (-) means invalid reading');
-- The CAN hw filter will be set to allow the following *incoming* msgs with these CAN IDs to be recognized (CANID_DUMMY is not loaded)
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT1'    ,'CANID_HB_TIMESYNC',		 'TENSION_a','Tension_a2G: 44 CANID 1 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT2'    ,'CANID_MSG_TIME_POLL',	 'TENSION_a','Tension_a2G: 45 CANID 2 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT3'    ,'CANID_TST_TENSION_a2G', 'TENSION_a','Tension_a2G: 46 CANID 3 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT4'    ,'CANID_CMD_TENSION_a2GI','TENSION_a','Tension_a2G: 47 CANID 4 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT5'    ,'CANID_DUMMY',   	'TENSION_a','Tension_a2G: 48 CANID 5 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT6'    ,'CANID_DUMMY', 	'TENSION_a','Tension_a2G: 49 CANID 6 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT7'    ,'CANID_DUMMY',	  	'TENSION_a','Tension_a2G: 50 CANID 7 add CAN hw filter to allow incoming msg');
INSERT INTO PARAM_VAL VALUES ('TENSION_a2G','TENSION_a_CANID_HW_FILT8'    ,'CANID_DUMMY',	  	'TENSION_a','Tension_a2G: 51 CANID 8 add CAN hw filter to allow incoming msg');
--

--========================================================================================================================================================================
-- Cable Angle 1
--                           FUNCTION_NAME     PARAM_NAME                PARAM_VAL        FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_LIST_CRC'		,'0',	  'CABLE_ANGLE',  '  1 Cable_angle_1: CRC for Cable angle list');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_LIST_VERSION'	,'1',	  'CABLE_ANGLE',  '  2 Cable_angle_1: Version number for cable angle List');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_HEARTBEAT_CT'      	,'8192',  'CABLE_ANGLE',  '  3 Cable Angle_1: Heart-Beat: Count of time ticks between autonomous msgs');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_DRUM_NUMBER'		,'1',	  'CABLE_ANGLE',  '  4 Cable angle_1: Drum sys number for this function instance');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_DRUM_FUNCTION_BIT'	,'1',	  'CABLE_ANGLE',  '  5 Cable angle_1: f_pollbit: Drum sys poll 1st payload byte bit for drum #');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_DRUM_POLL_BIT'	,'0',	  'CABLE_ANGLE',  '  6 Cable angle_1: p_pollbit: Drum sys poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_MIN_TENSION'	        ,'100.1', 'CABLE_ANGLE',  '  7 Cable Angle_1: Minimum tension required (units to match)');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_RATE_CT'		,'2',     'CABLE_ANGLE',  '  8 Cable Angle_1: Rate count: Number of tension readings between cable angle msgs');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_ALARM_REPEAT'	,'9',	  'CABLE_ANGLE',  '  9 Cable Angle_1: Number of times alarm msg is repeated');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_CALIB_COEF_0'	,'0.0',   'CABLE_ANGLE',  ' 10 Cable Angle_1: Cable angle polynomial coeff 0');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_CALIB_COEF_1'	,'1.0',   'CABLE_ANGLE',  ' 11 Cable Angle_1: Cable angle polynomial coeff 1');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_CALIB_COEF_2'	,'0.0',   'CABLE_ANGLE',  ' 12 Cable Angle_1: Cable angle polynomial coeff 2');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_CALIB_COEF_3'	,'0.0',   'CABLE_ANGLE',  ' 13 Cable Angle_1: Cable angle polynomial coeff 3');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_CANPRM_TENSION'  ,'CANID_MSG_TENSION_a21',	'CABLE_ANGLE',  ' 14 Cable angle_1: CANID: can msg tension from sheave load-pin');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_MSG_TIME_POLL'   ,'CANID_MSG_TIME_POLL',	'CABLE_ANGLE',  ' 15 Cable angle_1: CANID: MC: Time msg/Group polling');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_TIMESYNC'        ,'CANID_HB_TIMESYNC', 	'CABLE_ANGLE',  ' 16 Cable angle_1: CANID: GPS time sync distribution msg');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_HEARTBEAT_MSG'   ,'CANID_HB_CABLE_ANGLE_1',	'CABLE_ANGLE',  ' 17 Cable angle_1: CANID: Heartbeat msg');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_HEARTBEAT_CMDI'  ,'CANID_CMD_CABLE_ANGLE_1R','CABLE_ANGLE',  ' 17 Cable angle_1: CANID: Listen for command');
INSERT INTO PARAM_VAL VALUES ('CABLE_ANGLE_1','CABLE_ANGLE_USEME'    	         , '1',   'CABLE_ANGLE',  ' 18 Cable Angle_1: skip or use this function switch');
--
--=========================================================================================================================================================================
-- Yogurt maker 
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL  FUNCTION_TYPE         DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_LIST_CRC',              '0', 'YOGURT_1',  'Yogurt_1:  1 CRC for this list');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_LIST_VERSION',          '1', 'YOGURT_1',  'Yogurt_1:  2 Version number for Tension List');
-- Thermistor #1
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_CONST_B',   '3360.0', 'YOGURT_1',  'Yogurt_1:  3 Thermistor1 param: const B');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_R_SERIES' ,   '10.0', 'YOGURT_1',  'Yogurt_1:  4 Thermistor1 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_R_ROOMTMP',   '10.0', 'YOGURT_1',  'Yogurt_1:  5 Thermistor1 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_REF_TEMP',   '290.0', 'YOGURT_1',  'Yogurt_1:  6 Thermistor1 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_COEF_0',       '6.0', 'YOGURT_1',  'Yogurt_1:  7 Thermistor1 param: polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_COEF_1',      '1.00', 'YOGURT_1',  'Yogurt_1:  8z Thermistor1 param: polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_COEF_2',       '0.0', 'YOGURT_1',  'Yogurt_1:  9 Thermistor1 param: polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM1_COEF_3',       '0.0', 'YOGURT_1',  'Yogurt_1: 10 Thermistor1 param: polynomial coeff 3 (x^3)');
-- Thermistor #2
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_CONST_B',   '3390.0', 'YOGURT_1',  'Yogurt_1: 11 Thermistor2 param: const B');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_R_SERIES',    '10.0', 'YOGURT_1',  'Yogurt_1: 12 Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_R_ROOMTMP',   '10.0', 'YOGURT_1',  'Yogurt_1: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_REF_TEMP',   '290.0', 'YOGURT_1',  'Yogurt_1: 14 Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_COEF_0',       '5.3', 'YOGURT_1',  'Yogurt_1: 15 Thermistor2 param: polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_COEF_1',      '1.03', 'YOGURT_1',  'Yogurt_1: 16 Thermistor2 param: polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_COEF_2',       '0.0', 'YOGURT_1',  'Yogurt_1: 17 Thermistor2 param: polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM2_COEF_3',       '0.0', 'YOGURT_1',  'Yogurt_1: 18 Thermistor2 param: polynomial coeff 3 (x^3)');
-- Thermistor #3
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_CONST_B',   '3340.0', 'YOGURT_1',  'Yogurt_1: 19 Thermistor3 param: const B');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_R_SERIES',    '10.0', 'YOGURT_1',  'Yogurt_1: 20 Thermistor3 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_R_ROOMTMP',   '10.0', 'YOGURT_1',  'Yogurt_1: 21 Thermistor3 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_REF_TEMP',   '290.0', 'YOGURT_1',  'Yogurt_1: 22 Thermistor3 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_COEF_0',       '5.8', 'YOGURT_1',  'Yogurt_1: 23 Thermistor3 param: polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_COEF_1',     '1.007', 'YOGURT_1',  'Yogurt_1: 24 Thermistor3 param: polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_COEF_2',       '0.0', 'YOGURT_1',  'Yogurt_1: 25 Thermistor3 param: polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM3_COEF_3',       '0.0', 'YOGURT_1',  'Yogurt_1: 26 Thermistor3 param: polynomial coeff 3 (x^3)');
-- Thermistor #4
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_CONST_B',   '3340.0', 'YOGURT_1',  'Yogurt_1: 27 Thermistor4 param: const B');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_R_SERIES',    '10.0', 'YOGURT_1',  'Yogurt_1: 28 Thermistor4 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_R_ROOMTMP',   '10.0', 'YOGURT_1',  'Yogurt_1: 29 Thermistor4 param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_REF_TEMP',   '290.0', 'YOGURT_1',  'Yogurt_1: 30 Thermistor4 param: Reference temp for thermistor');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_COEF_0',       '5.2', 'YOGURT_1',  'Yogurt_1: 31 Thermistor4 param: polynomial coeff 0 (offset)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_COEF_1',      '1.04', 'YOGURT_1',  'Yogurt_1: 32 Thermistor4 param: polynomial coeff 1 (scale)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_COEF_2',       '0.0', 'YOGURT_1',  'Yogurt_1: 33 Thermistor4 param: polynomial coeff 2 (x^2)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_THERM4_COEF_3',       '0.0', 'YOGURT_1',  'Yogurt_1: 34 Thermistor4 param: polynomial coeff 3 (x^3)');

INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTLTEMP_HEAT_PAST', '160.0', 'YOGURT_1',	'Yogurt_1: 35 Pasteur: Control set-point temperature (deg F) heat to this temp');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTLTEMP_DUR_PAST',    '0.5', 'YOGURT_1',	'Yogurt_1: 36 Pasteur: Time duration at temp (hours.frac_hours)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTLTEMP_COOL_PAST', '110.0', 'YOGURT_1',	'Yogurt_1: 37 Pasteur: Control end-point temperature (deg F) cool to this temp');

INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTLTEMP_HEAT_FERM', '110.0', 'YOGURT_1',	'Yogurt_1: 38 Ferment: Control set-point temperature (deg F) heat to this temp');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTLTEMP_DUR_FERM',   '13.0', 'YOGURT_1',	'Yogurt_1: 39 Ferment: Time duration at temp (hours.frac_hours)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTLTEMP_COOL_FERM',  '45.0', 'YOGURT_1',	'Yogurt_1: 40 Ferment: Control end-point temperature (deg F) cool to this temp');

INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTL_THERM_SHELL',	'3',  'YOGURT_1',	'Yogurt_1: 41 Thermistor number for shell temp (0 - 3)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTL_THERM_POT',	'2',  'YOGURT_1',	'Yogurt_1: 42 Thermistor number for center of pot temp (0 - 3)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTL_THERM_AIRIN',	'0',  'YOGURT_1',	'Yogurt_1: 43 Thermistor number for air inlet to fan temp (0 - 3)');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTL_THERM_AIROUT','1',  'YOGURT_1',	'Yogurt_1: 44 Thermistor number for air coming out of holes (0 - 3)');

INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTL_THERM_LOOP_P',	'12000.0',	'YOGURT_1',	'Yogurt_1: 45 Control loop: Proportional coeff');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTL_THERM_LOOP_I',	'6.0',		'YOGURT_1',	'Yogurt_1: 46 Control loop: Integral coeff');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CTL_THERM_LOOP_D',	'240.0E3', 	'YOGURT_1',	'Yogurt_1: 47 Control loop: Derivative coeff');

INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_CMD','CANID_CMD_YOGURT_1R','YOGURT_1',  'Yogurt_1: 48 CANID: cid_yog_cmd: Yogurt maker parameters');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_MSG','CANID_MSG_YOGURT_1', 'YOGURT_1',  'Yogurt_1: 49 CANID: cid_yog_msg: Yogurt maker msgs');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_HB',	'CANID_HB_YOGURT_1', 'YOGURT_1',  'Yogurt_1: 50 CANID: cid_yog_hb: Yogurt maker heart-beats');

INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_HEATCONSTANT_KM_P',    '0.0100',	'YOGURT_1',	'Yogurt_1: 51  Control, stored heat const Pasteur phase');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_HEATCONSTANT_KM_M',    '0.0200',	'YOGURT_1',	'Yogurt_1: 52  Control, stored heat const Ferment phase');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_INTEGRATEINIT_A',      '-4000.0',	'YOGURT_1',	'Yogurt_1: 53  Control, integrator initialization, a of  a + b*x ');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_INTEGRATEINIT_B',      '434.0',	'YOGURT_1',	'Yogurt_1: 54  Control, integrator initialization, b of  a + b*x ');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_STABILIZETIMEDELAY_P', '200',	'YOGURT_1',	'Yogurt_1: 55  Control, time delay for temperature stabilization, Pasteur');
INSERT INTO PARAM_VAL VALUES ('YOGURT_1','YOGURT_1_STABILIZETIMEDELAY_F', '1070',	'YOGURT_1',	'Yogurt_1: 56  Control, time delay for temperature stabilization, Ferment');
--=====================================================================================================================================================================
-- MCL: Master Controller Launch parameters
--                           FUNCTION_NAME     PARAM_NAME             PARAM_VAL        FUNCTION_TYPE       DESCRIPTION11
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_LIST_CRC'	     ,	 	'0', 	'MCL',	'mcl: crc: CRC: Master Controller Launch parameters');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_LIST_VERSION'     ,	 	'1', 	'MCL',	'mcl: version: Master Controller Launch parameters');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_GROUND_TENSION_FACTOR',    	'1.00', 	'MCL',	'mcl: ground tension factor: Master Controller Launch parameters ');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_CLIMB_TENSION_FACTOR',    	'1.30', 	'MCL',	'mcl: climb tension factor: Master Controller Launch parameters ');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_GLIDER_MASS',   	 	'600.0', 	'MCL',	'mcl: glider mass (KG): Master Controller Launch parameters ');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_GLIDER_WEIGHT',   	 	'625', 	'MCL',	'mcl: @ glider weight (KG): Master Controller Launch parameters ');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_SOFT_START_TIME',  	 	'1500', 	'MCL',	'mcl: soft start timeM (MS): Master Controller Launch parameters ');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_K1',  	 			'45.95', 	'MCL',	'mcl: soft start const: k1: Master Controller Launch parameters ');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_PROFILE_TRIG_CABLE_SPEED',	'6.5',	 	'MCL',	'mcl: rotation taper: cable trigger speed (/MS): Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_MAX_GROUND_CABLE_SPEED',	'71.0', 	'MCL',	'mcl: rotation taper: max ground cable speed (M/S): Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_K2',				'0.074', 	'MCL',	'mcl: rotation taper: const k2: Master Controller Launch parameterS ');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_PEAK_CABLE_SPEED_DROP',	'8.5',	 	'MCL',	'mcl: transition to ramp: peak cable_speed_drop (M/S): Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_RAMP_TIME'		,	'6000', 	'MCL',	'mcl: ramp taper up: ramp time (MS): Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_K3'			,	'1.0',	 	'MCL',	'mcl: ramp taper up: const k3: Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_TAPERANGLETRIG'	,	'75', 		'MCL',	'mcl: end of climb taper down: taper angle trig (DEG): Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_TAPERTIME'	,		'2500', 	'MCL',	'mcl: end of climb taper down: taper time: Master (MS) Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_K4'	,			'1.0',	 	'MCL',	'mcl: end of climb taper down: const k4: Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_RELEASEDELTA'	,		'2.5', 		'MCL',	'mcl: end of climb taper down: release delta: Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_MAX_PARACHUTE_TENSION',	'50.0', 	'MCL',	'mcl: parachute tension taper: max parachute tension (KGF): Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_PARACHUTE_TAPER_SPEED',	'65.0', 	'MCL',	'mcl: parachute tension taper: parachute taper speed (M/S): Master Controller Launch parameter');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_MAX_PARACHUTE_CABLE_SPEED',	'80.0', 	'MCL',	'mcl: parachute tension taper: max chute cable speed (M/S): Master Controller Launch param');
INSERT INTO PARAM_VAL VALUES ('MCL','MCL_K5',				'1.1',	 	'MCL',	'mcl: parachute tension taper: const k5: Master Controller Launch parameter');

