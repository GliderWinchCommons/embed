// 2020-03-28 12:55:12.228

// =========== PARAMETER ARRAY/TABLE SIZES ============================
// Note: The instances of the same function types should have the same size. 
//        If not, then some is wrong with the PARAMETERS_VAL_INSERT. 

#define CABLE_ANGLE_1_PARAM_SIZE 18
#define CANSENDER_1_PARAM_SIZE 6
#define CANSENDER_2_PARAM_SIZE 6
#define ENG1_MANIFOLD_PARAM_SIZE 15
#define ENG1_RPM_PARAM_SIZE 14
#define ENG1_T1_PARAM_SIZE 19
#define ENG1_THROTTLE_PARAM_SIZE 15
#define ENGINE_SENSOR1_PARAM_SIZE 14
#define GPS_1_PARAM_SIZE 9
#define GPS_2_PARAM_SIZE 9
#define LOGGER_1_PARAM_SIZE 6
#define LOGGER_2_PARAM_SIZE 6
#define MCL_PARAM_SIZE 21
#define PWRBOX1_PARAM_SIZE 42
#define SHAFT1_PARAM_SIZE 22
#define SHEAVE_1LO_PARAM_SIZE 17
#define SHEAVE_1UP_PARAM_SIZE 17
#define TENSION_a11_PARAM_SIZE 51
#define TENSION_a12_PARAM_SIZE 51
#define TENSION_a1G_PARAM_SIZE 51
#define TENSION_a21_PARAM_SIZE 51
#define TENSION_a22_PARAM_SIZE 51
#define TENSION_a2G_PARAM_SIZE 51
#define YOGURT_1_PARAM_SIZE 56


// =====================================================================
#ifdef CABLE_ANGLE_1	// Include following parameters?
const uint32_t paramval00[] = {
 CABLE_ANGLE_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6    1 Cable_angle_1: CRC for Cable angle list                                     */
 0x00000001 , /*   2 1                    5    2 Cable_angle_1: Version number for cable angle List                          */
 0x00002000 , /*   3 8192                 6    3 Cable Angle_1: Heart-Beat: Count of time ticks between autonomous msgs      */
 0x00000001 , /*   4 1                    6    4 Cable angle_1: Drum sys number for this function instance                   */
 0x00000001 , /*   5 1                    6    5 Cable angle_1: f_pollbit: Drum sys poll 1st payload byte bit for drum #     */
 0x00000000 , /*   6 0                    6    6 Cable angle_1: p_pollbit: Drum sys poll 2nd payload byte bit for this type of function*/
 0x42C83333 , /*   7 100.1               11    7 Cable Angle_1: Minimum tension required (units to match)                    */
 0x00000002 , /*   8 2                    6    8 Cable Angle_1: Rate count: Number of tension readings between cable angle msgs*/
 0x00000009 , /*   9 9                    6    9 Cable Angle_1: Number of times alarm msg is repeated                        */
 0x00000000 , /*  10 0.0                 11   10 Cable Angle_1: Cable angle polynomial coeff 0                               */
 0x3F800000 , /*  11 1.0                 11   11 Cable Angle_1: Cable angle polynomial coeff 1                               */
 0x00000000 , /*  12 0.0                 11   12 Cable Angle_1: Cable angle polynomial coeff 2                               */
 0x00000000 , /*  13 0.0                 11   13 Cable Angle_1: Cable angle polynomial coeff 3                               */
 0x38200000 , /*  14 CANID_MSG_TENSION_a21  17   14 Cable angle_1: CANID: can msg tension from sheave load-pin                  */
 0x20000000 , /*  15 CANID_MSG_TIME_POLL  17   15 Cable angle_1: CANID: MC: Time msg/Group polling                            */
 0x00400000 , /*  16 CANID_HB_TIMESYNC   17   16 Cable angle_1: CANID: GPS time sync distribution msg                        */
 0xE0A00000 , /*  17 CANID_HB_CABLE_ANGLE_1  17   17 Cable angle_1: CANID: Heartbeat msg                                         */
 0x00000001 , /*  18 1                    6   18 Cable Angle_1: skip or use this function switch                             */
};
#endif

// =====================================================================
#ifdef CANSENDER_1	// Include following parameters?
const uint32_t paramval01[] = {
 CANSENDER_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Cansender_1: 1 CRC                                                              */
 0x00000001 , /*   2 1                    5  Cansender_1: 2 Version number                                                   */
 0x000001F4 , /*   3 500                  6  Cansender_1: 3 Heartbeat count of time (ms) between msgs                        */
 0xF0200000 , /*   4 CANID_HB_CANSENDER_1  17  Cansender_1: 4 CANID: Hearbeat sends running count                              */
 0xE2000000 , /*   5 CANID_POLL_CANSENDER  17  Cansender_1: 5 CANID: Poll this cansender                                       */
 0xE2200000 , /*   6 CANID_POLLR_CANSENDER_1  17  Cansender_1: 6 CANID: Response to POLL                                          */
};
#endif

// =====================================================================
#ifdef CANSENDER_2	// Include following parameters?
const uint32_t paramval02[] = {
 CANSENDER_2_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Cansender_2: 1 CRC                                                              */
 0x00000001 , /*   2 1                    5  Cansender_2: 2 Version number                                                   */
 0x00000177 , /*   3 375                  6  Cansender_2: 3 Heartbeat count of time (ms) between msgs                        */
 0xF0400000 , /*   4 CANID_HB_CANSENDER_2  17  Cansender_2: 4 CANID: Hearbeat sends running count                              */
 0xE2000000 , /*   5 CANID_POLL_CANSENDER  17  Cansender_2: 5 CANID: Poll this cansender                                       */
 0xE2400000 , /*   6 CANID_POLLR_CANSENDER_2  17  Cansender_2: 6 CANID: Response to POLL                                          */
};
#endif

// =====================================================================
#ifdef ENG1_MANIFOLD	// Include following parameters?
const uint32_t paramval03[] = {
 ENG1_MANIFOLD_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Eng1_manifold: CRC                                                              */
 0x00000001 , /*   2 1                    5  Eng1_manifold: Version number                                                   */
 0x000003E8 , /*   3 1000                 5  Eng1_manifold: Time (ms) between HB msg                                         */
 0xBFA66666 , /*   4 -1.3                11  Eng1_manifold: Manifold pressure offset                                         */
 0x3C16BB99 , /*   5 0.92E-2             11  Eng1_manifold: Manifold pressure  scale (inch Hg)                               */
 0x40800000 , /*   6 CANID_MSG_ENG1_MANFLD  17  Eng1_manifold: CANID: Poll response: manifold pressure, calibrated              */
 0x82200000 , /*   7 CANID_HB_ENG1_MANFLD  17  Eng1_manifold: CANID: Heartbeat: manifold pressure, calibrated                  */
 0x00400000 , /*   8 CANID_HB_TIMESYNC   17  EPS_1: CANID 1: GPS time sync distribution msg                                  */
 0x20000000 , /*   9 CANID_MSG_TIME_POLL  17  MC: CANID 2:Time msg/Group polling                                              */
 0x80A00000 , /*  10 CANID_CMD_ENG1_MANFLDI  17  Eng1_manifold: CANID 3: command                                                 */
 0xFFFFFFFC , /*  11 CANID_DUMMY         17  Eng1_manifold: CANID 4 add CAN hw filter for incoming msg                       */
 0xFFFFFFFC , /*  12 CANID_DUMMY         17  Eng1_manifold: CANID 5 add CAN hw filter for incoming msg                       */
 0xFFFFFFFC , /*  13 CANID_DUMMY         17  Eng1_manifold: CANID 6 add CAN hw filter for incoming msg                       */
 0xFFFFFFFC , /*  14 CANID_DUMMY         17  Eng1_manifold: CANID 7 add CAN hw filter for incoming msg                       */
 0xFFFFFFFC , /*  15 CANID_DUMMY         17  Eng1_manifold: CANID 8 add CAN hw filter for incoming msg                       */
};
#endif

// =====================================================================
#ifdef ENG1_RPM	// Include following parameters?
const uint32_t paramval04[] = {
 ENG1_RPM_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Eng1_rpm: CRC                                                                   */
 0x00000001 , /*   2 1                    5  Eng1_rpm: Version number                                                        */
 0x000003E8 , /*   3 1000                 5  Eng1_rpm: Time (ms) between HB msg                                              */
 0x00000008 , /*   4 8                    6  Eng1_rpm: Number of counts per two revolutions                                  */
 0x40A00000 , /*   5 CANID_MSG_ENG1_RPM  17  Eng1_rpm: CANID: rpm, calibrated                                                */
 0x82600000 , /*   6 CANID_HB_ENG1_RPM   17  Eng1_rpm: CANID: Heartbeat: rpm, calibrated                                     */
 0x00400000 , /*   7 CANID_HB_TIMESYNC   17  EPS_1: CANID: GPS time sync distribution msg                                    */
 0x20000000 , /*   8 CANID_MSG_TIME_POLL  17  MC: CANID:Time msg/Group polling                                                */
 0x81200000 , /*   9 CANID_CMD_ENG1_RPMI  17  Eng1_rpm: CANID: command                                                        */
 0xFFFFFFFC , /*  10 CANID_DUMMY         17  Eng1_rpm: CANID 4 add CAN hw filter for incoming msg                            */
 0xFFFFFFFC , /*  11 CANID_DUMMY         17  Eng1_rpm: CANID 5 add CAN hw filter for incoming msg                            */
 0xFFFFFFFC , /*  12 CANID_DUMMY         17  Eng1_rpm: CANID 6 add CAN hw filter for incoming msg                            */
 0xFFFFFFFC , /*  13 CANID_DUMMY         17  Eng1_rpm: CANID 7 add CAN hw filter for incoming msg                            */
 0xFFFFFFFC , /*  14 CANID_DUMMY         17  Eng1_rpm: CANID 8 add CAN hw filter for incoming msg                            */
};
#endif

// =====================================================================
#ifdef ENG1_T1	// Include following parameters?
const uint32_t paramval05[] = {
 ENG1_T1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Eng1_t1: CRC                                                                    */
 0x00000001 , /*   2 1                    5  Eng1_t1: Version number                                                         */
 0x00001388 , /*   3 5000                 5  Eng1_t1: Time (ms) between HB msg                                               */
 0x45534000 , /*   4 3380                11  Eng1_t1: Thermistor param: constant B                                           */
 0x40F428F6 , /*   5 7.63                11  Eng1_t1: Thermistor param: Series resistor, fixed (K ohms)                      */
 0x41200000 , /*   6 10.0                11  Eng1_t1: Thermistor param: Thermistor room temp resistance (K ohms)             */
 0x43910000 , /*   7 290.0               11  Eng1_t1: Thermistor param: Reference temp for thermistor                        */
 0x3F000000 , /*   8 0.5                 11  Eng1_t1: Thermistor param: Thermistor temp offset correction (deg C)            */
 0x3F800000 , /*   9 1.0                 11  Eng1_t1: Thermistor param: Thermistor temp scale correction                     */
 0x41200000 , /*  10 CANID_MSG_ENG1_T1   17  Eng1_t1: CANID: Poll response: CANID: Poll response: temperature #1 (deg C)     */
 0x82800000 , /*  11 CANID_HB_ENG1_T1    17  Eng1_t1: CANID: Heartbeat: CANID: Heartbeat: temperature #1 (deg C)             */
 0x00400000 , /*  12 CANID_HB_TIMESYNC   17  Eng1_t1: CANID 1 added to CAN hardware filter to allow incoming msg             */
 0x20000000 , /*  13 CANID_MSG_TIME_POLL  17  Eng1_t1: CANID 2 added to CAN hardware filter to allow incoming msg             */
 0x81A00000 , /*  14 CANID_CMD_ENG1_T1I  17  Eng1_t1: CANID 3 added to CAN hardware filter to allow incoming msg             */
 0xFFFFFFFC , /*  15 CANID_DUMMY         17  Eng1_t1: CANID 4 added to CAN hardware filter to allow incoming msg             */
 0xFFFFFFFC , /*  16 CANID_DUMMY         17  Eng1_t1: CANID 5 added to CAN hardware filter to allow incoming msg             */
 0xFFFFFFFC , /*  17 CANID_DUMMY         17  Eng1_t1: CANID 6 added to CAN hardware filter to allow incoming msg             */
 0xFFFFFFFC , /*  18 CANID_DUMMY         17  Eng1_t1: CANID 7 added to CAN hardware filter to allow incoming msg             */
 0xFFFFFFFC , /*  19 CANID_DUMMY         17  Eng1_t1: CANID 8 added to CAN hardware filter to allow incoming msg             */
};
#endif

// =====================================================================
#ifdef ENG1_THROTTLE	// Include following parameters?
const uint32_t paramval06[] = {
 ENG1_THROTTLE_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Eng1_throttle:: CRC                                                             */
 0x00000001 , /*   2 1                    5  Eng1_throttle:: Version number                                                  */
 0x000003E8 , /*   3 1000                 5  Eng1_throttle:: Time (ms) between HB msg                                        */
 0x00000FCD , /*   4 4045                 6  Eng1_throttle:: ADC when throttle full open                                     */
 0x00000032 , /*   5 50                   6  Eng1_throttle:: ADC when throttle closed                                        */
 0x40E00000 , /*   6 CANID_MSG_ENG1_THROTTLE  17  Eng1_throttle:: CANID: Poll response: throttle (0.0-100.0)                      */
 0x82400000 , /*   7 CANID_HB_ENG1_THROTTLE  17  Eng1_throttle:: CANID: Heartbeat: throttle (0.0-100.0)                          */
 0x00400000 , /*   8 CANID_HB_TIMESYNC   17  EPS_1: CANID 1: GPS time sync distribution msg                                  */
 0x20000000 , /*   9 CANID_MSG_TIME_POLL  17  MC: CANID 2:Time msg/Group polling                                              */
 0x81600000 , /*  10 CANID_CMD_ENG1_THROTTLEI  17  Eng1_throttle:: CANID 3: command                                                */
 0xFFFFFFFC , /*  11 CANID_DUMMY         17  Eng1_throttle:: CANID 4 add CAN hw filter for incoming msg                      */
 0xFFFFFFFC , /*  12 CANID_DUMMY         17  Eng1_throttle:: CANID 5 add CAN hw filter for incoming msg                      */
 0xFFFFFFFC , /*  13 CANID_DUMMY         17  Eng1_throttle:: CANID 6 add CAN hw filter for incoming msg                      */
 0xFFFFFFFC , /*  14 CANID_DUMMY         17  Eng1_throttle:: CANID 7 add CAN hw filter for incoming msg                      */
 0xFFFFFFFC , /*  15 CANID_DUMMY         17  Eng1_throttle:: CANID 8 add CAN hw filter for incoming msg                      */
};
#endif

// =====================================================================
#ifdef ENGINE_SENSOR1	// Include following parameters?
const uint32_t paramval07[] = {
 ENGINE_SENSOR1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000008 , /*   1 8                    6  Eng1: Counts per revolution on engine                                           */
 0xBF000000 , /*   2 -0.5                11  Eng1: Manifold pressure offset                                                  */
 0x42CFC28F , /*   3 103.88              11  Eng1: Manifold pressure  scale (inch Hg)                                        */
 0x45534000 , /*   4 3380                11  Eng1: Thermistor param: constant B                                              */
 0x41200000 , /*   5 10.0                11  Eng1: Thermistor param: Series resistor, fixed (K ohms)                         */
 0x41200000 , /*   6 10.0                11  Eng1: Thermistor param: Thermistor room temp resistance (K ohms)                */
 0x43910000 , /*   7 290.0               11  Eng1: Thermistor param: Reference temp for thermistor                           */
 0x00000000 , /*   8 0.0                 11  Eng1: Thermistor param: Thermistor temp offset correction (deg C)               */
 0x3F800000 , /*   9 1.0                 11  Eng1: Thermistor param: Thermistor temp scale correction                        */
 0x43FA0000 , /*  10 500                 11  Eng1: Throttle pot ADC reading: closed                                          */
 0x40600000 , /*  11 CANID_MSG_ENG1_RPMMANFLD_R  17  Eng1: CANID: rpm:manifold, calibrated                                           */
 0x60600000 , /*  12 CANID_MSG_ENG1_TEMPTHRTL_R  17  Eng1: CANID: temperature:throttle pot, calibrated                               */
 0x4060000C , /*  13 CANID_HB_ENG1_RPMMANFLD  17  Eng1: CANID: raw readings: rpm:manifold pressure                                */
 0x6060000C , /*  14 CANID_HB_ENG1_THRMTHRTL  17  Eng1: CANID: raw readings: thermistor:throttle                                  */
};
#endif

// =====================================================================
#ifdef GPS_1	// Include following parameters?
const uint32_t paramval08[] = {
 GPS_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  GPS_1: 1 CRC                                                                    */
 0x00000001 , /*   2 1                    5  GPS_1: 2 Version number                                                         */
 0x000003E8 , /*   3 1000                 6  GPS_1: 3 Time (ms) between unix time HB msgs                                    */
 0x00002710 , /*   4 10000                6  GPS_1: 4 Time (ms) between burst of lat lon height HB msgs                      */
 0x0000044C , /*   5 1100                 6  GPS_1: 5 Time (ms) between lat/lon and lon/ht msgs                              */
 0xE1000000 , /*   6 CANID_HB_GPS_TIME_1  17  GPS_1: 6 Heartbeat unix time                                                    */
 0xE1C00000 , /*   7 CANID_HB_GPS_LLH_1  17  GPS_1: 7 Heartbeat (3 separate msgs) lattitude longitude height                 */
 0x00000000 , /*   8 0                    6  GPS_1: 8 time sync msgs: 0 = enable  1 = disable                                */
 0x00400000 , /*   9 CANID_HB_TIMESYNC   17  GPS_1: 9 Time sync msg                                                          */
};
#endif

// =====================================================================
#ifdef GPS_2	// Include following parameters?
const uint32_t paramval09[] = {
 GPS_2_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  GPS_2: 1 CRC                                                                    */
 0x00000001 , /*   2 1                    5  GPS_2: 2 Version number                                                         */
 0x000003E8 , /*   3 1000                 6  GPS_2: 3 Time (ms) between unix time HB msgs                                    */
 0x00002710 , /*   4 10000                6  GPS_2: 4 Time (ms) between burst of lat lon height HB msgs                      */
 0x0000041A , /*   5 1050                 6  GPS_2: 5 Time (ms) between lat/lon and lon/ht msgs                              */
 0xE1E00000 , /*   6 CANID_HB_GPS_TIME_2  17  GPS_2: 6 Heartbeat unix time                                                    */
 0xE2600000 , /*   7 CANID_HB_GPS_LLH_2  17  GPS_2: 7 Heartbeat (3 separate msgs) lattitude longitude height                 */
 0x00000000 , /*   8 0                    6  GPS_2: 8 time sync msgs: 0 = enable  1 = disable                                */
 0x00400000 , /*   9 CANID_HB_TIMESYNC   17  GPS_2: 9 Time sync msg                                                          */
};
#endif

// =====================================================================
#ifdef LOGGER_1	// Include following parameters?
const uint32_t paramval10[] = {
 LOGGER_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Logger_1: 1 CRC                                                                 */
 0x00000001 , /*   2 1                    5  Logger_1: 2 Version number                                                      */
 0x00001F40 , /*   3 8000                 6  Logger_1: 3 Heartbeat count of time (ms) between msgs                           */
 0xE1800000 , /*   4 CANID_HB_LOGGER_1   17  Logger_1: 4 CANID: Hearbeat sends running count of logged msgs                  */
 0x00800000 , /*   5 CANID_ALM_PWRBOX1   17  Logger_1: 5 CANID: Low voltage alarm msg suspends logging                       */
 0x000002EE , /*   6 750                  6  Logger_1: 6 Suspend duration (ms) after receiving alarm msg                     */
};
#endif

// =====================================================================
#ifdef LOGGER_2	// Include following parameters?
const uint32_t paramval11[] = {
 LOGGER_2_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Logger_2: 1 CRC                                                                 */
 0x00000001 , /*   2 1                    5  Logger_2: 2 Version number                                                      */
 0x00001F40 , /*   3 8000                 6  Logger_2: 3 Heartbeat count of time (ms) between msgs                           */
 0xE1A00000 , /*   4 CANID_HB_LOGGER_2   17  Logger_2: 4 CANID: Hearbeat sends running count of logged msgs                  */
 0x00800000 , /*   5 CANID_ALM_PWRBOX1   17  Logger_2: 5 CANID: Low voltage alarm msg suspends logging                       */
 0x000002EE , /*   6 750                  6  Logger_2: 6 Suspend duration (ms) after receiving alarm msg                     */
};
#endif

// =====================================================================
#ifdef MCL	// Include following parameters?
const uint32_t paramval12[] = {
 MCL_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  mcl: crc: CRC: Master Controller Launch parameters                              */
 0x00000001 , /*   2 1                    5  mcl: version: Master Controller Launch parameters                               */
 0x3F800000 , /*   3 1.00                11  mcl: ground tension factor: Master Controller Launch parameters                 */
 0x3FA66666 , /*   4 1.30                11  mcl: climb tension factor: Master Controller Launch parameters                  */
 0x44160000 , /*   5 600.0               11  mcl: glider mass (KG): Master Controller Launch parameters                      */
 0x441C4000 , /*   6 625                 11  mcl: @ glider weight (KG): Master Controller Launch parameters                  */
 0x44BB8000 , /*   7 1500                11  mcl: soft start timeM (MS): Master Controller Launch parameters                 */
 0x4237CCCD , /*   8 45.95               11  mcl: soft start const: k1: Master Controller Launch parameters                  */
 0x40D00000 , /*   9 6.5                 11  mcl: rotation taper: cable trigger speed (/MS): Master Controller Launch parameter*/
 0x428E0000 , /*  10 71.0                11  mcl: rotation taper: max ground cable speed (M/S): Master Controller Launch parameter*/
 0x3D978D50 , /*  11 0.074               11  mcl: rotation taper: const k2: Master Controller Launch parameterS              */
 0x41080000 , /*  12 8.5                 11  mcl: transition to ramp: peak cable_speed_drop (M/S): Master Controller Launch parameter*/
 0x00001770 , /*  13 6000                 6  mcl: ramp taper up: ramp time (MS): Master Controller Launch parameter          */
 0x3F800000 , /*  14 1.0                 11  mcl: ramp taper up: const k3: Master Controller Launch parameter                */
 0x42960000 , /*  15 75                  11  mcl: end of climb taper down: taper angle trig (DEG): Master Controller Launch parameter*/
 0x451C4000 , /*  16 2500                11  mcl: end of climb taper down: taper time: Master (MS) Controller Launch parameter*/
 0x3F800000 , /*  17 1.0                 11  mcl: end of climb taper down: const k4: Master Controller Launch parameter      */
 0x40200000 , /*  18 2.5                 11  mcl: end of climb taper down: release delta: Master Controller Launch parameter */
 0x42480000 , /*  19 50.0                11  mcl: parachute tension taper: max parachute tension (KGF): Master Controller Launch parameter*/
 0x42820000 , /*  20 65.0                11  mcl: parachute tension taper: parachute taper speed (M/S): Master Controller Launch parameter*/
 0x3F8CCCCD , /*  21 1.1                 11  mcl: parachute tension taper: const k5: Master Controller Launch parameter      */
};
#endif

// =====================================================================
#ifdef PWRBOX1	// Include following parameters?
const uint32_t paramval13[] = {
 PWRBOX1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6   1 Pwrbox1: CRC                                                                 */
 0x00000001 , /*   2 1                    5   2 Pwrbox1: Version number                                                      */
 0x000003E8 , /*   3 1000                 5   3 Pwrbox1: Time (ms) between HB msg                                            */
 0x00000000 , /*   4 0.0                 11   4 Pwrbox1: ADC 0 5VREG offset na                                               */
 0x3AA36F97 , /*   5 0.001246917         11   5 Pwrbox1: ADC 0 5VREG scale volts/step                                        */
 0x00000000 , /*   6 0.0                 11   6 Pwrbox1: ADC 1 CANVB offset na                                               */
 0x3B80BC24 , /*   7 0.003928678         11   7 Pwrbox1: ADC 1 CANVB scale volts/step                                        */
 0x00000000 , /*   8 0.0                 11   8 Pwrbox1: ADC 2 DIODE offset na                                               */
 0x3B83EAA6 , /*   9 0.004025775         11   9 Pwrbox1: ADC 2 DIODE scale volts/step                                        */
 0x00000000 , /*  10 0.0                 11  10 Pwrbox1: ADC 3 INPWR offset na                                               */
 0x3B82388C , /*  11 0.003974026         11  11 Pwrbox1: ADC 3 INPWR scale volts/step                                        */
 0x45014000 , /*  12 2068.0              11  12 Pwrbox1: ADC 4 HALLE offset zero                                             */
 0x3D2DED85 , /*  13 0.042462845         11  13 Pwrbox1: ADC 4 HALLE scale amps/step                                         */
 0x00000000 , /*  14 0.0                 11  14 Pwrbox1: ADC 5 SPAR1 offset                                                  */
 0x3F800000 , /*  15 1.0                 11  15 Pwrbox1: ADC 5 SPAR1 scale                                                   */
 0x00000000 , /*  16 0.0                 11  16 Pwrbox1: ADC 6 ITEMP offset Vref                                             */
 0x3F800000 , /*  17 1.0                 11  17 Pwrbox1: ADC 6 ITEMP scale                                                   */
 0x00000000 , /*  18 0.0                 11  18 Pwrbox1: ADC 7 IVREF offset                                                  */
 0x3B11643D , /*  19 0.002218499         11  19 Pwrbox1: ADC 7 IVREF scale volts/step                                        */
 0x00000080 , /*  20 128                  6  20 Pwrbox1: IIR1 Filter 0 factor: 5v regulator                                  */
 0x00000002 , /*  21 2                    6  21 Pwrbox1: IIR1 Filter 0 scale : 5v regulator                                  */
 0x0000000A , /*  22 10                   6  22 Pwrbox1: IIR2 Filter 1 factor: Hall-effect sensor                            */
 0x00000001 , /*  23 1                    6  23 Pwrbox1: IIR2 Filter 1 scale : Hall-effect sensor                            */
 0x0000000A , /*  24 10                   6  24 Pwrbox1: IIR3 Filter 2 factor: CAN bus volts                                 */
 0x00000001 , /*  25 1                    6  25 Pwrbox1: IIR3 Filter 2 scale : CAN bus volts                                 */
 0x00000080 , /*  26 128                  6  26 Pwrbox1: IIR4 Filter 3 factor: Power source volts                            */
 0x00000002 , /*  27 2                    6  27 Pwrbox1: IIR4 Filter 3 scale : Power source volts                            */
 0xFF200000 , /*  28 CANID_HB_PWRBOX1    17  28 Pwrbox: CANID: Heartbeat: input voltage, bus voltage                         */
 0xE3000000 , /*  29 CANID_MSG_PWRBOX1   17  29 Pwrbox: CANID: Msg: input voltage, bus voltage                               */
 0x00800000 , /*  30 CANID_ALM_PWRBOX1   17  30 Pwrbox: CANID: Alarm: input voltage, bus voltage                             */
 0x00000032 , /*  31 50                   6  31 Pwrbox: Time (ms) between alarm msgs, when below threshold                   */
 0x41280000 , /*  32 10.5                11  32 Pwrbox: Voltage threshold for alarm msgs                                     */
 0x00400000 , /*  33 CANID_HB_TIMESYNC   17  33 Pwrbox1: CANID 0: GPS time sync distribution msg                             */
 0x20000000 , /*  34 CANID_MSG_TIME_POLL  17  34 Pwrbox1: CANID 1: Time msg/Group polling                                     */
 0xFFFFFFFC , /*  35 CANID_DUMMY         17  35 Pwrbox1: CANID 2 add CAN hw filter for incoming msg                          */
 0xFFFFFFFC , /*  36 CANID_DUMMY         17  36 Pwrbox1: CANID 3 add CAN hw filter for incoming msg                          */
 0xFFFFFFFC , /*  37 CANID_DUMMY         17  37 Pwrbox1: CANID 4 add CAN hw filter for incoming msg                          */
 0xFFFFFFFC , /*  38 CANID_DUMMY         17  38 Pwrbox1: CANID 5 add CAN hw filter for incoming msg                          */
 0xFFFFFFFC , /*  39 CANID_DUMMY         17  39 Pwrbox1: CANID 6 add CAN hw filter for incoming msg                          */
 0xFFFFFFFC , /*  40 CANID_DUMMY         17  40 Pwrbox1: CANID 7 add CAN hw filter for incoming msg                          */
 0xFF400000 , /*  41 CANID_LVL_HB_PWRBOX1  17  41 Pwrbox1: CANID: Heartbeat: level wind current                                */
 0xE3400000 , /*  42 CANID_LVL_MSG_PWRBOX1  17  42 Pwrbox1: CANID: Msg: level wind current                                      */
};
#endif

// =====================================================================
#ifdef SHAFT1	// Include following parameters?
const uint32_t paramval14[] = {
 SHAFT1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Shaft1: CRC                                                                     */
 0x00000001 , /*   2 1                    5  Shaft1: Version number                                                          */
 0x000003E8 , /*   3 1000                 6  Shaft1: Heartbeat count of time (ms) between msgs                               */
 0x00000384 , /*   4 900                  6  Shaft1: High threshold register setting, ADC3                                   */
 0x00000190 , /*   5 400                  6  Shaft1: Low  threshold register setting, ADC3                                   */
 0x0000044C , /*   6 1100                 6  Shaft1: High threshold register setting, ADC2                                   */
 0x000002BC , /*   7 700                  6  Shaft1: Low  threshold register setting, ADC2                                   */
 0x82A00000 , /*   8 CANID_MSG_SHAFT1_SPEED  17  Shaft1: CANID: Shaft speed, calibrated, response to poll                        */
 0xE2800000 , /*   9 CANID_HB_SHAFT1_SPEED  17  Shaft1: CANID: Shaft speed, calibrated, heartbeat                               */
 0x82E00000 , /*  10 CANID_MSG_SHAFT1_COUNT  17  Shaft1: CANID: Shaft running counT, response to poll                            */
 0xE2A00000 , /*  11 CANID_HB_SHAFT1_COUNT  17  Shaft1: CANID: Shaft running count, heartbeat                                   */
 0x00000010 , /*  12 16                   6  Shaft1: Number of segments on code wheel                                        */
 0x00400000 , /*  13 CANID_HB_TIMESYNC   17  Shaft1: CANID 1: GPS time sync distribution msg                                 */
 0x20000000 , /*  14 CANID_MSG_TIME_POLL  17  Shaft1: CANID 2:Time msg/Group polling                                          */
 0xA0600000 , /*  15 CANID_CMD_SHAFT1I   17  Shaft1: CANID 3: command                                                        */
 0xFFFFFFFC , /*  16 CANID_DUMMY         17  Shaft1: CANID 4 added to CAN hw filter to allow incoming msg                    */
 0xFFFFFFFC , /*  17 CANID_DUMMY         17  Shaft1: CANID 5 added to CAN hw filter to allow incoming msg                    */
 0xFFFFFFFC , /*  18 CANID_DUMMY         17  Shaft1: CANID 6 added to CAN hw filter to allow incoming msg                    */
 0xFFFFFFFC , /*  19 CANID_DUMMY         17  Shaft1: CANID 7 added to CAN hw filter to allow incoming msg                    */
 0xFFFFFFFC , /*  20 CANID_DUMMY         17  Shaft1: CANID 8 added to CAN hw filter to allow incoming msg                    */
 0x00000004 , /*  21 4                    6  Shaft1: IIR Filter factor: divisor sets time const: for heart-beat msg          */
 0x00000040 , /*  22 64                   6  Shaft1: IIR Filter scale : upscaling (integer math): for heart-beat msg         */
};
#endif

// =====================================================================
#ifdef SHEAVE_1LO	// Include following parameters?
const uint32_t paramval15[] = {
 SHEAVE_1LO_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Encoder_f4_1lo: 1 CRC                                                           */
 0x00000001 , /*   2 1                    5  Encoder_f4_1lo: 2 Version number                                                */
 0x000000FA , /*   3 250                  6  Encoder_f4_1lo: 3 Heartbeat count of time (ms) between msgs                     */
 0x000002D0 , /*   4 720                  6  Encoder_f4_1lo: 4 Number of counts per revolution                               */
 0x3F0EFEC8 , /*   5 0.558575174         11  Encoder_f4_1lo: 5 Distance per revolution (meters)                              */
 0xD1800000 , /*   6 CANID_HB_LOWER1_H_RAW  17  Encoder_f4_1lo: 6 CANID: Heartbeat sends raw (long long) running encoder count  */
 0xD1A00000 , /*   7 CANID_MSG_LOWER1_H_CAL  17  Encoder_f4_1lo: 7 CANID: hi-res: msg--lower calibrated--distance and speed      */
 0xFFFFFFFC , /*   8 CANID_DUMMY         17  Encoder_f4_1lo: 8 CANID: Poll with time sync msg                                */
 0xFFFFFFFC , /*   9 CANID_DUMMY         17  Encoder_f4_1lo: 9 CANID: Response to POLL                                       */
 0x00400000 , /*  10 CANID_HB_TIMESYNC   17  Encoder_f4_1lo: 10 CANID 1 add CAN hw filter to allow incoming msg              */
 0x20000000 , /*  11 CANID_MSG_TIME_POLL  17  Encoder_f4_1lo: 11 CANID 2 add CAN hw filter to allow incoming msg              */
 0xD2200004 , /*  12 CANID_TST_SHEAVE_1LO  17  Encoder_f4_1lo: 12 CANID 3 add CAN hw filter to allow incoming msg              */
 0xD1400000 , /*  13 CANID_CMD_LOWER1_HI  17  Encoder_f4_1lo: 13 CANID 4 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  14 CANID_DUMMY         17  Encoder_f4_1lo: 14 CANID 5 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  15 CANID_DUMMY         17  Encoder_f4_1lo: 15 CANID 6 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  16 CANID_DUMMY         17  Encoder_f4_1lo: 16 CANID 7 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  17 CANID_DUMMY         17  Encoder_f4_1lo: 17 CANID 8 add CAN hw filter to allow incoming msg              */
};
#endif

// =====================================================================
#ifdef SHEAVE_1UP	// Include following parameters?
const uint32_t paramval16[] = {
 SHEAVE_1UP_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Encoder_f4_1up: 1 CRC                                                           */
 0x00000001 , /*   2 1                    5  Encoder_f4_1up: 2 Version number                                                */
 0x000000FA , /*   3 250                  6  Encoder_f4_1up: 3 Heartbeat count of time (ms) between msgs                     */
 0x000002D0 , /*   4 720                  6  Encoder_f4_1up: 4 Number of counts per revolution                               */
 0x3F0EFEC8 , /*   5 0.558575174         11  Encoder_f4_1up: 5 Distance per revolution (meters)                              */
 0xD1E00000 , /*   6 CANID_HB_UPPER1_H_RAW  17  Encoder_f4_1up: 6 CANID: Heartbeat sends raw (long long) running encoder count  */
 0xD1200000 , /*   7 CANID_MSG_UPPER1_H_CAL  17  Encoder_f4_1up: 7 CANID: hi-res: msg--upper calibrated--distance and speed      */
 0xFFFFFFFC , /*   8 CANID_DUMMY         17  Encoder_f4_1up: 8 CANID: Poll with time sync msg                                */
 0xFFFFFFFC , /*   9 CANID_DUMMY         17  Encoder_f4_1up: 9 CANID: Response to POLL                                       */
 0x00400000 , /*  10 CANID_HB_TIMESYNC   17  Encoder_f4_1up: 10 CANID 1 add CAN hw filter to allow incoming msg              */
 0x20000000 , /*  11 CANID_MSG_TIME_POLL  17  Encoder_f4_1up: 11 CANID 2 add CAN hw filter to allow incoming msg              */
 0xD2000004 , /*  12 CANID_TST_SHEAVE_1UP  17  Encoder_f4_1up: 12 CANID 3 add CAN hw filter to allow incoming msg              */
 0xD0E00000 , /*  13 CANID_CMD_UPPER1_HI  17  Encoder_f4_1up: 13 CANID 4 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  14 CANID_DUMMY         17  Encoder_f4_1up: 14 CANID 5 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  15 CANID_DUMMY         17  Encoder_f4_1up: 15 CANID 6 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  16 CANID_DUMMY         17  Encoder_f4_1up: 16 CANID 7 add CAN hw filter to allow incoming msg              */
 0xFFFFFFFC , /*  17 CANID_DUMMY         17  Encoder_f4_1up: 17 CANID 8 add CAN hw filter to allow incoming msg              */
};
#endif

// =====================================================================
#ifdef TENSION_a11	// Include following parameters?
const uint32_t paramval17[] = {
 TENSION_a11_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a11: 1 CRC for tension list                                             */
 0x00000001 , /*   2 1                    5  Tension_a11: 2 Version number for Tension List                                  */
 0x00000000 , /*   3 0                    5  Tension_a11: 3 AD7799 final offset                                              */
 0x38E8528F , /*   4 0.11078E-3          11  Tension_a11: 4 AD7799 final Scale (kgf) FS 250kgf 2mv/v                         */
 0x45534000 , /*   5 3380.0              11  Tension_a11:  5 Thermistor1 param: const B                                      */
 0x41200000 , /*   6 10.0                11  Tension_a11:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a11:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a11:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a11:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a11: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a11: 11 Thermistor2 param: const B                                      */
 0x41200000 , /*  12 10.0                11  Tension_a11: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a11: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a11: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a11: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a11: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a11: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a11: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a11: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  20 0.0                 11  Tension_a11: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x40A00000 , /*  21 5.0                 11  Tension_a11: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a11: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a11: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  24 0.0                 11  Tension_a11: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x000000FA , /*  25 250                  6  Tension_a11: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs*/
 0x00000001 , /*  26 1                    6  Tension_a11: 26 Drum sys number for this function instance                      */
 0x00000001 , /*  27 1                    6  Tension_a11: 27 Drum sys poll 2nd payload byte bit for this type of function    */
 0x00000001 , /*  28 1                    6  Tension_a11: 28 Drum sys poll 1st payload byte bit for drum # (function instance)*/
 0x38000000 , /*  29 CANID_MSG_TENSION_a11  17  Tension_a11: 29 CANID: can msg tension for AD7799 #1                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a11: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a11: 31 CANID: GPS time sync distribution msg                           */
 0xE0600000 , /*  32 CANID_HB_TENSION_a11  17  Tension_a11: 32 CANID: Heartbeat msg                                            */
 0xF800010C , /*  33 CANID_TST_TENSION_a11  17  Tension_a11: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a11: 34 IIR Filter factor: divisor sets time const: reading for polled msg*/
 0x00000080 , /*  35 128                  6  Tension_a11: 35 Filter scale : upscaling (due to integer math): for polled msg  */
 0x00000064 , /*  36 100                  6  Tension_a11: 36 IIR Filter factor: divisor sets time const: reading for heart-beat  msg*/
 0x00000080 , /*  37 128                  6  Tension_a11: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg*/
 0x00000003 , /*  38 3                    6  Tension_a11: 38 skip or use this function swit ch                               */
 0x0000000A , /*  39 10                   6  Tension_a11: 39 IIR Filter factor: zero recalibration                           */
 0x00000040 , /*  40 64                   6  Tension_a11: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a11: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a11: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a11: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a11: 44 CANID 1 add CAN hw filter to allow incoming msg                 */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a11: 45 CANID 2 add CAN hw filter to allow incoming msg                 */
 0xF800010C , /*  46 CANID_TST_TENSION_a11  17  Tension_a11: 46 CANID 3 add CAN hw filter to allow incoming msg                 */
 0x05C00004 , /*  47 CANID_CMD_TENSION_a11I  17  Tension_a11: 47 CANID 4 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a11: 48 CANID 5 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a11: 49 CANID 6 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a11: 50 CANID 7 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a11: 51 CANID 8 add CAN hw filter to allow incoming msg                 */
};
#endif

// =====================================================================
#ifdef TENSION_a12	// Include following parameters?
const uint32_t paramval18[] = {
 TENSION_a12_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a12:   1 CRC for tension list                                           */
 0x00000001 , /*   2 1                    5  Tension_a12:   2 Version number for Tension List                                */
 0x000012B5 , /*   3 4789                 5  Tension_a12:   3 AD7799 offset                                                  */
 0x3B39D2A6 , /*   4 0.283543155E-2      11  Tension_a12: AD7799 #1 Scale (convert to kgf)                                   */
 0x45534000 , /*   5 3380.0              11  Tension_a12:  5 Thermistor1 param: const B                                      */
 0x41200000 , /*   6 10.0                11  Tension_a12:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a12:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a12:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a12:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a12: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a12: 11 Thermistor2 param: const B                                      */
 0x41200000 , /*  12 10.0                11  Tension_a12: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a12: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a12: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a12: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a12: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a12: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a12: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a12: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  20 0.0                 11  Tension_a12: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x40A00000 , /*  21 5.0                 11  Tension_a12: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a12: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a12: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  24 0.0                 11  Tension_a12: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x000007D0 , /*  25 2000                 6  Tension_a12: 25 Heart-Beat: Count of time ticks between autonomous msgs         */
 0x00000002 , /*  26 2                    6  Tension_a12: 26 Drum sys number for this function instance                      */
 0x00000001 , /*  27 1                    6  Tension_a12: 27 Drum sys poll 2nd payload byte bit for this type of function    */
 0x00000002 , /*  28 2                    6  Tension_a12: 28 Drum sys poll 1st payload byte bit for drum # (function instance)*/
 0x38400000 , /*  29 CANID_MSG_TENSION_a12  17  Tension_a12: 29 CANID: can msg tension for AD7799 #2                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a12: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a12: 31 CANID: GPS time sync distribution msg                           */
 0xE0800000 , /*  32 CANID_HB_TENSION_a12  17  Tension_a12: 32 CANID: Heartbeat msg                                            */
 0xF800020C , /*  33 CANID_TST_TENSION_a12  17  Tension_a12: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a12: 34 IIR Filter factor: divisor sets time const: reading polled msg  */
 0x00000080 , /*  35 128                  6  Tension_a12: 35 Filter scale : upscaling (due to integer math): polled msg      */
 0x00000200 , /*  36 512                  6  Tension_a12: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg*/
 0x00000080 , /*  37 128                  6  Tension_a12: 37 Filter scale : upscaling (due to integer math): heart-beat  msg */
 0x00000003 , /*  38 3                    6  Tension_a12: 38 skip or use this function switch                                */
 0x0000000A , /*  39 10                   6  Tension_a12: 39 IIR Filter factor: zero recalibration                           */
 0x00000080 , /*  40 128                  6  Tension_a12: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a12: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a12: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a12: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a12: 44 CANID 1 add CAN hw filter to allow incoming msg                 */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a12: 45 CANID 2 add CAN hw filter to allow incoming msg                 */
 0xF800020C , /*  46 CANID_TST_TENSION_a12  17  Tension_a12: 46 CANID 3 add CAN hw filter to allow incoming msg                 */
 0xF800005C , /*  47 CANID_CMD_TENSION_a12I  17  Tension_a12: 47 CANID 4 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a12: 48 CANID 5 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a12: 49 CANID 6 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a12: 50 CANID 7 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a12: 51 CANID 8 add CAN hw filter to allow incoming msg                 */
};
#endif

// =====================================================================
#ifdef TENSION_a1G	// Include following parameters?
const uint32_t paramval19[] = {
 TENSION_a1G_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a1G: 1 CRC for tension list                                             */
 0x00000001 , /*   2 1                    5  Tension_a1G: 2 Version number for Tension List                                  */
 0x00000000 , /*   3 0                    5  Tension_a1G: 3 AD7799 final offset                                              */
 0x38E8528F , /*   4 0.11078E-3          11  Tension_a1G: 4 AD7799 final Scale (kgf) FS 250kgf 2mv/v                         */
 0x45534000 , /*   5 3380.0              11  Tension_a1G:  5 Thermistor1 param: const B                                      */
 0x41200000 , /*   6 10.0                11  Tension_a1G:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a1G:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a1G:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a1G:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a1G: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a1G: 11 Thermistor2 param: const B                                      */
 0x41200000 , /*  12 10.0                11  Tension_a1G: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a1G: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a1G: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a1G: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a1G: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a1G: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a1G: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a1G: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  20 0.0                 11  Tension_a1G: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x40A00000 , /*  21 5.0                 11  Tension_a1G: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a1G: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a1G: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  24 0.0                 11  Tension_a1G: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x000000FA , /*  25 250                  6  Tension_a1G: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs*/
 0x00000001 , /*  26 1                    6  Tension_a1G: 26 Drum sys number for this function instance                      */
 0x00000001 , /*  27 1                    6  Tension_a1G: 27 Drum sys poll 2nd payload byte bit for this type of function    */
 0x00000001 , /*  28 1                    6  Tension_a1G: 28 Drum sys poll 1st payload byte bit for drum # (function instance)*/
 0xF800043C , /*  29 CANID_MSG_TENSION_a1G  17  Tension_a1G: 29 CANID: can msg tension for AD7799 #1                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a1G: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a1G: 31 CANID: GPS time sync distribution msg                           */
 0xE0E20000 , /*  32 CANID_HB_TENSION_a1G  17  Tension_a1G: 32 CANID: Heartbeat msg                                            */
 0xF800041C , /*  33 CANID_TST_TENSION_a1G  17  Tension_a1G: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a1G: 34 IIR Filter factor: divisor sets time const: reading for polled msg*/
 0x00000080 , /*  35 128                  6  Tension_a1G: 35 Filter scale : upscaling (due to integer math): for polled msg  */
 0x00000064 , /*  36 100                  6  Tension_a1G: 36 IIR Filter factor: divisor sets time const: reading for heart-beat  msg*/
 0x00000080 , /*  37 128                  6  Tension_a1G: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg*/
 0x00000003 , /*  38 3                    6  Tension_a1G: 38 skip or use this function swit ch                               */
 0x0000000A , /*  39 10                   6  Tension_a1G: 39 IIR Filter factor: zero recalibration                           */
 0x00000040 , /*  40 64                   6  Tension_a1G: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a1G: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a1G: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a1G: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a1G: 44 CANID 1 add CAN hw filter to allow incoming msg                 */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a1G: 45 CANID 2 add CAN hw filter to allow incoming msg                 */
 0xF800041C , /*  46 CANID_TST_TENSION_a1G  17  Tension_a1G: 46 CANID 3 add CAN hw filter to allow incoming msg                 */
 0xF800007C , /*  47 CANID_CMD_TENSION_a1GI  17  Tension_a1G: 47 CANID 4 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a1G: 48 CANID 5 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a1G: 49 CANID 6 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a1G: 50 CANID 7 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a1G: 51 CANID 8 add CAN hw filter to allow incoming msg                 */
};
#endif

// =====================================================================
#ifdef TENSION_a21	// Include following parameters?
const uint32_t paramval20[] = {
 TENSION_a21_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a21:   1 CRC for tension list                                           */
 0x00000001 , /*   2 1                    5  Tension_a21:   2 Version number for Tension List                                */
 0x000013D4 , /*   3 5076                 5  Tension_a21:   3 AD7799 offset                                                  */
 0x399ADFEB , /*   4 0.2954E-3           11  Tension_a21: AD7799 #1 Scale (kgf) FS 1000kgf 3mv/v                             */
 0x45534000 , /*   5 3380.0              11  Tension_a21:  5 Thermistor1 param: const B                                      */
 0x41200000 , /*   6 10.0                11  Tension_a21:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a21:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a21:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a21:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a21: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a21: 11 Thermistor2 param: const B                                      */
 0x41200000 , /*  12 10.0                11  Tension_a21: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a21: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a21: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a21: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a21: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a21: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a21: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a21: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  20 0.0                 11  Tension_a21: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x40A00000 , /*  21 5.0                 11  Tension_a21: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a21: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a21: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  24 0.0                 11  Tension_a21: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x000003E8 , /*  25 1000                 6  Tension_a21: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs*/
 0x00000001 , /*  26 1                    6  Tension_a21: 26 Drum sys number for this function instance                      */
 0x00000001 , /*  27 1                    6  Tension_a21: 27 Drum sys poll 2nd payload byte bit for this type of function    */
 0x00000001 , /*  28 1                    6  Tension_a21: 28 Drum sys poll 1st payload byte bit for drum # (function instance)*/
 0x38200000 , /*  29 CANID_MSG_TENSION_a21  17  Tension_a21: 29 CANID: can msg tension for AD7799 #2                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a21: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a21: 31 CANID: GPS time sync distribution msg                           */
 0xE0C00000 , /*  32 CANID_HB_TENSION_a21  17  Tension_a21: 32 CANID: Heartbeat msg                                            */
 0xF800030C , /*  33 CANID_TST_TENSION_a21  17  Tension_a21: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a21: 34 IIR Filter factor: divisor sets time const: reading polled msg  */
 0x00000080 , /*  35 128                  6  Tension_a21: 35 Filter scale : upscaling (due to integer math): polled msg      */
 0x00000200 , /*  36 512                  6  Tension_a21: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg*/
 0x00000080 , /*  37 128                  6  Tension_a21: 37 Filter scale : upscaling (due to integer math): heart-beat  msg */
 0x00000003 , /*  38 3                    6  Tension_a: 38 skip or use this function switch                                  */
 0x0000000A , /*  39 10                   6  Tension_a21: 39 IIR Filter factor: zero recalibration                           */
 0x00000080 , /*  40 128                  6  Tension_a21: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a21: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a21: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a21: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a21: 44 CANID 1 add CAN hw filter to allow incoming msg                 */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a21: 45 CANID 2 add CAN hw filter to allow incoming msg                 */
 0xF800030C , /*  46 CANID_TST_TENSION_a21  17  Tension_a21: 46 CANID 3 add CAN hw filter to allow incoming msg                 */
 0x05E00004 , /*  47 CANID_CMD_TENSION_a21I  17  Tension_a21: 47 CANID 4 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a21: 48 CANID 5 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a21: 49 CANID 6 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a21: 50 CANID 7 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a21: 51 CANID 8 add CAN hw filter to allow incoming msg                 */
};
#endif

// =====================================================================
#ifdef TENSION_a22	// Include following parameters?
const uint32_t paramval21[] = {
 TENSION_a22_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a22:   1 CRC for tension list                                           */
 0x00000001 , /*   2 1                    5  Tension_a22:   2 Version number for Tension List                                */
 0x000012B5 , /*   3 4789                 5  Tension_a22:   3 AD7799 offset                                                  */
 0x3B39D2A6 , /*   4 0.283543155E-2      11  Tension_a22: AD7799 #1 Scale (convert to kgf)                                   */
 0x45534000 , /*   5 3380.0              11  Tension_a22:  5 Thermistor1 param: const B                                      */
 0x41200000 , /*   6 10.0                11  Tension_a22:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a22:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a22:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a22:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a22: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a22: 11 Thermistor2 param: const B                                      */
 0x41200000 , /*  12 10.0                11  Tension_a22: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a22: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a22: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a22: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a22: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a22: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a22: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a22: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  20 0.0                 11  Tension_a22: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x40A00000 , /*  21 5.0                 11  Tension_a22: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a22: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a22: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  24 0.0                 11  Tension_a22: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x00002000 , /*  25 8192                 6  Tension_a22: 25 Heart-Beat: Count of time ticks between autonomous msgs         */
 0x00000002 , /*  26 2                    6  Tension_a22: 26 Drum sys number for this function instance                      */
 0x00000001 , /*  27 1                    6  Tension_a22: 27 Drum sys poll 2nd payload byte bit for this type of function    */
 0x00000000 , /*  28 0                    6  Tension_a22: 28 Drum sys poll 1st payload byte bit for drum # (function instance)*/
 0x38600000 , /*  29 CANID_MSG_TENSION_a22  17  Tension_a22: 29 CANID: can msg tension for AD7799 #2                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a22: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a22: 31 CANID: GPS time sync distribution msg                           */
 0xE0E00000 , /*  32 CANID_HB_TENSION_a22  17  Tension_a22: 32 CANID: Heartbeat msg                                            */
 0xF800020C , /*  33 CANID_TST_TENSION_a12  17  Tension_a22: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a22: 34 IIR Filter factor: divisor sets time const: reading polled msg  */
 0x00000080 , /*  35 128                  6  Tension_a22: 35 Filter scale : upscaling (due to integer math): polled msg      */
 0x00000200 , /*  36 512                  6  Tension_a22: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg*/
 0x00000080 , /*  37 128                  6  Tension_a22: 37 Filter scale : upscaling (due to integer math): heart-beat  msg */
 0x00000003 , /*  38 3                    6  Tension_a22: 38 skip or use this function switch                                */
 0x0000000A , /*  39 10                   6  Tension_a22: 39 IIR Filter factor: zero recalibration                           */
 0x00000080 , /*  40 128                  6  Tension_a22: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a22: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a22: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a22: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a22: 44 CANID 1 add CAN hw filter to allow incoming msg                 */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a22: 45 CANID 2 add CAN hw filter to allow incoming msg                 */
 0xF800040C , /*  46 CANID_TST_TENSION_a22  17  Tension_a22: 46 CANID 3 add CAN hw filter allow incoming msg                    */
 0xF800006C , /*  47 CANID_CMD_TENSION_a22I  17  Tension_a22: 47 CANID 4 add CAN hw filter allow incoming msg                    */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a22: 48 CANID 5 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a22: 49 CANID 6 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a22: 50 CANID 7 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a22: 51 CANID 8 add CAN hw filter to allow incoming msg                 */
};
#endif

// =====================================================================
#ifdef TENSION_a2G	// Include following parameters?
const uint32_t paramval22[] = {
 TENSION_a2G_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a2G: 1 CRC for tension list                                             */
 0x00000001 , /*   2 1                    5  Tension_a2G: 2 Version number for Tension List                                  */
 0x000013D4 , /*   3 5076                 5  Tension_a2G: 3 AD7799 offset                                                    */
 0x399ADFEB , /*   4 0.2954E-3           11  Tension_a2G: 4 AD7799 #1 Scale (kgf) FS 1000kgf 3mv/v                           */
 0x45534000 , /*   5 3380.0              11  Tension_a2G:  5 Thermistor1 param: const B                                      */
 0x41200000 , /*   6 10.0                11  Tension_a2G:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a2G:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a2G:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a2G:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a2G: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a2G: 11 Thermistor2 param: const B                                      */
 0x41200000 , /*  12 10.0                11  Tension_a2G: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a2G: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a2G: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a2G: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a2G: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a2G: 17 Thermistor1 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a2G: 18 Thermistor1 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a2G: 19 Thermistor1 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  20 0.0                 11  Tension_a2G: 20 Thermistor1 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x40A00000 , /*  21 5.0                 11  Tension_a2G: 21 Thermistor2 param: Load-Cell temp comp polynomial coeff 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a2G: 22 Thermistor2 param: Load-Cell temp comp polynomial coeff 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a2G: 23 Thermistor2 param: Load-Cell temp comp polynomial coeff 2 (x^2) */
 0x00000000 , /*  24 0.0                 11  Tension_a2G: 24 Thermistor2 param: Load-Cell temp comp polynomial coeff 3 (x^3) */
 0x000003E8 , /*  25 1000                 6  Tension_a2G: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs*/
 0x00000001 , /*  26 1                    6  Tension_a2G: 26 Drum sys number for this function instance                      */
 0x00000001 , /*  27 1                    6  Tension_a2G: 27 Drum sys poll 2nd payload byte bit for this type of function    */
 0x00000001 , /*  28 1                    6  Tension_a2G: 28 Drum sys poll 1st payload byte bit for drum # (function instance)*/
 0xF800044C , /*  29 CANID_MSG_TENSION_a2G  17  Tension_a2G: 29 CANID: can msg tension for AD7799 #2                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a2G: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a2G: 31 CANID: GPS time sync distribution msg                           */
 0xE0E40000 , /*  32 CANID_HB_TENSION_a2G  17  Tension_a2G: 32 CANID: Heartbeat msg                                            */
 0xF800042C , /*  33 CANID_TST_TENSION_a2G  17  Tension_a2G: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a2G: 34 IIR Filter factor: divisor sets time const: reading polled msg  */
 0x00000080 , /*  35 128                  6  Tension_a2G: 35 Filter scale : upscaling (due to integer math): polled msg      */
 0x00000200 , /*  36 512                  6  Tension_a2G: 36 IIR Filter factor: divisor sets time const: reading heart-beat msg*/
 0x00000080 , /*  37 128                  6  Tension_a2G: 37 Filter scale : upscaling (due to integer math): heart-beat  msg */
 0x00000003 , /*  38 3                    6  Tension_a: 38 skip or use this function switch                                  */
 0x0000000A , /*  39 10                   6  Tension_a2G: 39 IIR Filter factor: zero recalibration                           */
 0x00000080 , /*  40 128                  6  Tension_a2G: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a2G: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a2G: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a2G: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a2G: 44 CANID 1 add CAN hw filter to allow incoming msg                 */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a2G: 45 CANID 2 add CAN hw filter to allow incoming msg                 */
 0xF800042C , /*  46 CANID_TST_TENSION_a2G  17  Tension_a2G: 46 CANID 3 add CAN hw filter to allow incoming msg                 */
 0xF800008C , /*  47 CANID_CMD_TENSION_a2GI  17  Tension_a2G: 47 CANID 4 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a2G: 48 CANID 5 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a2G: 49 CANID 6 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a2G: 50 CANID 7 add CAN hw filter to allow incoming msg                 */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a2G: 51 CANID 8 add CAN hw filter to allow incoming msg                 */
};
#endif

// =====================================================================
#ifdef YOGURT_1	// Include following parameters?
const uint32_t paramval23[] = {
 YOGURT_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Yogurt_1:  1 CRC for this list                                                  */
 0x00000001 , /*   2 1                    5  Yogurt_1:  2 Version number for Tension List                                    */
 0x45520000 , /*   3 3360.0              11  Yogurt_1:  3 Thermistor1 param: const B                                         */
 0x41200000 , /*   4 10.0                11  Yogurt_1:  4 Thermistor1 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*   5 10.0                11  Yogurt_1:  5 Thermistor1 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*   6 290.0               11  Yogurt_1:  6 Thermistor1 param: Reference temp for thermistor                   */
 0x40C00000 , /*   7 6.0                 11  Yogurt_1:  7 Thermistor1 param: polynomial coeff 0 (offset)                     */
 0x3F800000 , /*   8 1.00                11  Yogurt_1:  8z Thermistor1 param: polynomial coeff 1 (scale)                     */
 0x00000000 , /*   9 0.0                 11  Yogurt_1:  9 Thermistor1 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  10 0.0                 11  Yogurt_1: 10 Thermistor1 param: polynomial coeff 3 (x^3)                        */
 0x4553E000 , /*  11 3390.0              11  Yogurt_1: 11 Thermistor2 param: const B                                         */
 0x41200000 , /*  12 10.0                11  Yogurt_1: 12 Thermistor2 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*  13 10.0                11  Yogurt_1: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*  14 290.0               11  Yogurt_1: 14 Thermistor2 param: Reference temp for thermistor                   */
 0x40A9999A , /*  15 5.3                 11  Yogurt_1: 15 Thermistor2 param: polynomial coeff 0 (offset)                     */
 0x3F83D70A , /*  16 1.03                11  Yogurt_1: 16 Thermistor2 param: polynomial coeff 1 (scale)                      */
 0x00000000 , /*  17 0.0                 11  Yogurt_1: 17 Thermistor2 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  18 0.0                 11  Yogurt_1: 18 Thermistor2 param: polynomial coeff 3 (x^3)                        */
 0x4550C000 , /*  19 3340.0              11  Yogurt_1: 19 Thermistor3 param: const B                                         */
 0x41200000 , /*  20 10.0                11  Yogurt_1: 20 Thermistor3 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*  21 10.0                11  Yogurt_1: 21 Thermistor3 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*  22 290.0               11  Yogurt_1: 22 Thermistor3 param: Reference temp for thermistor                   */
 0x40B9999A , /*  23 5.8                 11  Yogurt_1: 23 Thermistor3 param: polynomial coeff 0 (offset)                     */
 0x3F80E560 , /*  24 1.007               11  Yogurt_1: 24 Thermistor3 param: polynomial coeff 1 (scale)                      */
 0x00000000 , /*  25 0.0                 11  Yogurt_1: 25 Thermistor3 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  26 0.0                 11  Yogurt_1: 26 Thermistor3 param: polynomial coeff 3 (x^3)                        */
 0x4550C000 , /*  27 3340.0              11  Yogurt_1: 27 Thermistor4 param: const B                                         */
 0x41200000 , /*  28 10.0                11  Yogurt_1: 28 Thermistor4 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*  29 10.0                11  Yogurt_1: 29 Thermistor4 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*  30 290.0               11  Yogurt_1: 30 Thermistor4 param: Reference temp for thermistor                   */
 0x40A66666 , /*  31 5.2                 11  Yogurt_1: 31 Thermistor4 param: polynomial coeff 0 (offset)                     */
 0x3F851EB8 , /*  32 1.04                11  Yogurt_1: 32 Thermistor4 param: polynomial coeff 1 (scale)                      */
 0x00000000 , /*  33 0.0                 11  Yogurt_1: 33 Thermistor4 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  34 0.0                 11  Yogurt_1: 34 Thermistor4 param: polynomial coeff 3 (x^3)                        */
 0x43200000 , /*  35 160.0               11  Yogurt_1: 35 Pasteur: Control set-point temperature (deg F) heat to this temp   */
 0x3F000000 , /*  36 0.5                 11  Yogurt_1: 36 Pasteur: Time duration at temp (hours.frac_hours)                  */
 0x42DC0000 , /*  37 110.0               11  Yogurt_1: 37 Pasteur: Control end-point temperature (deg F) cool to this temp   */
 0x42DC0000 , /*  38 110.0               11  Yogurt_1: 38 Ferment: Control set-point temperature (deg F) heat to this temp   */
 0x41500000 , /*  39 13.0                11  Yogurt_1: 39 Ferment: Time duration at temp (hours.frac_hours)                  */
 0x42340000 , /*  40 45.0                11  Yogurt_1: 40 Ferment: Control end-point temperature (deg F) cool to this temp   */
 0x00000003 , /*  41 3                    6  Yogurt_1: 41 Thermistor number for shell temp (0 - 3)                           */
 0x00000002 , /*  42 2                    6  Yogurt_1: 42 Thermistor number for center of pot temp (0 - 3)                   */
 0x00000000 , /*  43 0                    6  Yogurt_1: 43 Thermistor number for air inlet to fan temp (0 - 3)                */
 0x00000001 , /*  44 1                    6  Yogurt_1: 44 Thermistor number for air coming out of holes (0 - 3)              */
 0x463B8000 , /*  45 12000.0             11  Yogurt_1: 45 Control loop: Proportional coeff                                   */
 0x40C00000 , /*  46 6.0                 11  Yogurt_1: 46 Control loop: Integral coeff                                       */
 0x486A6000 , /*  47 240.0E3             11  Yogurt_1: 47 Control loop: Derivative coeff                                     */
 0x29800004 , /*  48 CANID_CMD_YOGURT_1R  17  Yogurt_1: 48 CANID: cid_yog_cmd: Yogurt maker parameters                        */
 0x29400000 , /*  49 CANID_MSG_YOGURT_1  17  Yogurt_1: 49 CANID: cid_yog_msg: Yogurt maker msgs                              */
 0x29600000 , /*  50 CANID_HB_YOGURT_1   17  Yogurt_1: 50 CANID: cid_yog_hb: Yogurt maker heart-beats                        */
 0x3C23D70A , /*  51 0.0100              11  Yogurt_1: 51  Control, stored heat const Pasteur phase                          */
 0x3CA3D70A , /*  52 0.0200              11  Yogurt_1: 52  Control, stored heat const Ferment phase                          */
 0xC57A0000 , /*  53 -4000.0             11  Yogurt_1: 53  Control, integrator initialization, a of  a + b*x                 */
 0x43D90000 , /*  54 434.0               11  Yogurt_1: 54  Control, integrator initialization, b of  a + b*x                 */
 0x000000C8 , /*  55 200                  6  Yogurt_1: 55  Control, time delay for temperature stabilization, Pasteur        */
 0x0000042E , /*  56 1070                 6  Yogurt_1: 56  Control, time delay for temperature stabilization, Ferment        */
};
#endif

// ================= COMMAND CANID TABLES ========================

#define CAN_UNIT_11_CMDID_TABLE_SIZE 2	// Tension_a2: R 2 AD7799 VE POD 1 port 2

#define CAN_UNIT_12_CMDID_TABLE_SIZE 4	// Yogurt_1: R Ver 1 of maker

#define CAN_UNIT_13_CMDID_TABLE_SIZE 2	// Tension_a:  R 1 AD7799 VE POD port 1

#define CAN_UNIT_14_CMDID_TABLE_SIZE 2	// Tension_a2: R 2 AD7799 VE POD port 2

#define CAN_UNIT_15_CMDID_TABLE_SIZE 4	// Tension_a2: R 2 AD7799 VE POD GSM 1 port 2

#define CAN_UNIT_16_CMDID_TABLE_SIZE 4	// Logger_2: R 

#define CAN_UNIT_1A_CMDID_TABLE_SIZE 4	// Cansender_1: R Sensor board test

#define CAN_UNIT_1B_CMDID_TABLE_SIZE 2	// Shaft encoder_a1: R lower

#define CAN_UNIT_1E_CMDID_TABLE_SIZE 4	// Pwrbox1: R lower

#define CAN_UNIT_1F_CMDID_TABLE_SIZE 2	// Sensor, shaft1: R Drive shaft encoder

#define CAN_UNIT_2_CMDID_TABLE_SIZE 2	//  Sensor, engine: R temperature #1

#define CAN_UNIT_3_CMDID_TABLE_SIZE 8	// Logger_1: R 

#define CAN_UNIT_E_CMDID_TABLE_SIZE 4	// Cable angle: R AD7799 #2 drum #1

#define CAN_UNIT_F_CMDID_TABLE_SIZE 4


// =====================================================================
#ifdef CAN_UNIT_11	// a #define is used to select the following
const struct FUNC_CANID func_canid00[] = {
{   0x02200000, CAN_UNIT_11_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1003,  0xF800801C }, /*   1 CAN_UNIT_11          CANID_CMD_TENSION_a0XR	 Tension_a: R 1 AD7799 VE POD Test (hence X) 0*/
{   3,  0xF800001C }, /*   2 CAN_UNIT_11          CANID_CMD_TENSION_a0XI	 Tension_a: I 1 AD7799 VE POD Test (hence X) 0*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_12	// a #define is used to select the following
const struct FUNC_CANID func_canid01[] = {
{   0x02400000, CAN_UNIT_12_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1004,  0x05E0000C }, /*   1 CAN_UNIT_12          CANID_CMD_TENSION_a21R	 Tension_a2: R 2 AD7799 VE POD 1 port 2*/
{   4,  0x05E00004 }, /*   2 CAN_UNIT_12          CANID_CMD_TENSION_a21I	 Tension_a2: I 2 AD7799 VE POD 1 port 2*/
{1003,  0x05C0000C }, /*   3 CAN_UNIT_12          CANID_CMD_TENSION_a11R	 Tension_a: R  2 AD7799 VE POD 1 port 2*/
{   3,  0x05C00004 }, /*   4 CAN_UNIT_12          CANID_CMD_TENSION_a11I	 Tension_a: I  2 AD7799 VE POD 1 port 1*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_13	// a #define is used to select the following
const struct FUNC_CANID func_canid02[] = {
{   0x02600000, CAN_UNIT_13_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1011,  0x29800004 }, /*   1 CAN_UNIT_13          CANID_CMD_YOGURT_1R	 Yogurt_1: R Ver 1 of maker    */
{  11,  0x29800000 }, /*   2 CAN_UNIT_13          CANID_CMD_YOGURT_1I	 Yogurt_1: I Ver 1 of maker    */
};
#endif

// =====================================================================
#ifdef CAN_UNIT_14	// a #define is used to select the following
const struct FUNC_CANID func_canid03[] = {
{   0x02E00000, CAN_UNIT_14_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1003,  0xF800804C }, /*   1 CAN_UNIT_14          CANID_CMD_TENSION_a1YR	 Tension_a:  R 1 AD7799 VE POD port 1*/
{   3,  0xF800004C }, /*   2 CAN_UNIT_14          CANID_CMD_TENSION_a1YI	 Tension_a:  I 1 AD7799 VE POD port 1*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_15	// a #define is used to select the following
const struct FUNC_CANID func_canid04[] = {
{   0x02A00000, CAN_UNIT_15_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1004,  0xF800806C }, /*   1 CAN_UNIT_15          CANID_CMD_TENSION_a22R	 Tension_a2: R 2 AD7799 VE POD port 2*/
{   4,  0xF800006C }, /*   2 CAN_UNIT_15          CANID_CMD_TENSION_a22I	 Tension_a2: I 2 AD7799 VE POD port 2*/
{1003,  0xF800805C }, /*   3 CAN_UNIT_15          CANID_CMD_TENSION_a12R	 Tension_a:  R 2 AD7799 VE POD 2 port 1*/
{   3,  0xF800005C }, /*   4 CAN_UNIT_15          CANID_CMD_TENSION_a12I	 Tension_a:  I 2 AD7799 VE POD 2 port 1*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_16	// a #define is used to select the following
const struct FUNC_CANID func_canid05[] = {
{   0x02C00000, CAN_UNIT_16_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1004,  0xF800808C }, /*   1 CAN_UNIT_16          CANID_CMD_TENSION_a2GR	 Tension_a2: R 2 AD7799 VE POD GSM 1 port 2*/
{   4,  0xF800008C }, /*   2 CAN_UNIT_16          CANID_CMD_TENSION_a2GI	 Tension_a2: I 2 AD7799 VE POD GSM 1 port 2*/
{1003,  0xF800807C }, /*   3 CAN_UNIT_16          CANID_CMD_TENSION_a1GR	 Tension_a:  R 2 AD7799 VE POD GSM 1 port 1*/
{   3,  0xF800007C }, /*   4 CAN_UNIT_16          CANID_CMD_TENSION_a1GI	 Tension_a:  I 2 AD7799 VE POD GSM 1 port 1*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_1A	// a #define is used to select the following
const struct FUNC_CANID func_canid06[] = {
{   0x03600000, CAN_UNIT_1A_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1013,  0xD1C0006C }, /*   1 CAN_UNIT_1A          CANID_CMD_LOGGER_2R	 Logger_2: R                   */
{  13,  0xD1C00064 }, /*   2 CAN_UNIT_1A          CANID_CMD_LOGGER_2I	 Logger_2: I                   */
{1012,  0xD1C0007C }, /*   3 CAN_UNIT_1A          CANID_CMD_GPS_2R	 GPS_2: R time sync distribution msg */
{  12,  0xD1C00074 }, /*   4 CAN_UNIT_1A          CANID_CMD_GPS_2I	 GPS_2: I  time sync distribution msg */
};
#endif

// =====================================================================
#ifdef CAN_UNIT_1B	// a #define is used to select the following
const struct FUNC_CANID func_canid07[] = {
{   0x03E00000, CAN_UNIT_1B_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1014,  0xA0200004 }, /*   1 CAN_UNIT_1B          CANID_CMD_CANSENDER_1R	 Cansender_1: R Sensor board test*/
{  14,  0xA0200000 }, /*   2 CAN_UNIT_1B          CANID_CMD_CANSENDER_1I	 Cansender_1: I Sensor board test*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_1E	// a #define is used to select the following
const struct FUNC_CANID func_canid08[] = {
{   0x04800000, CAN_UNIT_1E_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1016,  0xD1600004 }, /*   1 CAN_UNIT_1E          CANID_CMD_LOWER1_HR	 Shaft encoder_a1: R lower     */
{  16,  0xD1400000 }, /*   2 CAN_UNIT_1E          CANID_CMD_LOWER1_HI	 Shaft encoder_a1: I lower     */
{1015,  0xD0E00004 }, /*   3 CAN_UNIT_1E          CANID_CMD_UPPER1_HR	 Shaft encoder_a1: R upper     */
{  15,  0xD0E00000 }, /*   4 CAN_UNIT_1E          CANID_CMD_UPPER1_HI	 Shaft encoder_a1: I upper     */
};
#endif

// =====================================================================
#ifdef CAN_UNIT_1F	// a #define is used to select the following
const struct FUNC_CANID func_canid09[] = {
{   0x04A00000, CAN_UNIT_1F_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1023,  0xE3200000 }, /*   1 CAN_UNIT_1F          CANID_CMD_PWRBOX1R	 Pwrbox1: R lower              */
{  23,  0xE320000C }, /*   2 CAN_UNIT_1F          CANID_CMD_PWRBOX1I	 Pwrbox1: I lower              */
};
#endif

// =====================================================================
#ifdef CAN_UNIT_2	// a #define is used to select the following
const struct FUNC_CANID func_canid10[] = {
{   0x04000000, CAN_UNIT_2_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1022,  0xE2C00000 }, /*   1 CAN_UNIT_2           CANID_CMD_SHAFT1R	 Sensor, shaft1: R Drive shaft encoder*/
{  22,  0xA0600000 }, /*   2 CAN_UNIT_2           CANID_CMD_SHAFT1I	 Sensor, shaft1: I Drive shaft encoder*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_3	// a #define is used to select the following
const struct FUNC_CANID func_canid11[] = {
{   0x03800000, CAN_UNIT_3_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1020,  0x81E00000 }, /*   1 CAN_UNIT_3           CANID_CMD_ENG1_T1R	  Sensor, engine: R temperature #1*/
{  20,  0x81A00000 }, /*   2 CAN_UNIT_3           CANID_CMD_ENG1_T1I	  Sensor, engine: I temperature #1*/
{1019,  0x81800000 }, /*   3 CAN_UNIT_3           CANID_CMD_ENG1_THROTTLER	  Sensor, engine: R throttle position*/
{  19,  0x81600000 }, /*   4 CAN_UNIT_3           CANID_CMD_ENG1_THROTTLEI	  Sensor, engine: I throttle position*/
{1018,  0x81400000 }, /*   5 CAN_UNIT_3           CANID_CMD_ENG1_RPMR	  Sensor, engine: R rpm        */
{  18,  0x81200000 }, /*   6 CAN_UNIT_3           CANID_CMD_ENG1_RPMI	  Sensor, engine: I rpm        */
{1017,  0x80E00000 }, /*   7 CAN_UNIT_3           CANID_CMD_ENG1_MANFLDR	  Sensor, engine: R manifold pressure*/
{  17,  0x80A00000 }, /*   8 CAN_UNIT_3           CANID_CMD_ENG1_MANFLDI	  Sensor, engine: I manifold pressure*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_E	// a #define is used to select the following
const struct FUNC_CANID func_canid12[] = {
{   0x01C0000C, CAN_UNIT_E_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1013,  0xD1C0005C }, /*   1 CAN_UNIT_E           CANID_CMD_LOGGER_1R	 Logger_1: R                   */
{  13,  0xD1C00054 }, /*   2 CAN_UNIT_E           CANID_CMD_LOGGER_1I	 Logger_1: I                   */
{1012,  0xD1C0004C }, /*   3 CAN_UNIT_E           CANID_CMD_GPS_1R	 GPS_1: R time sync distribution msg*/
{  12,  0xD1C00044 }, /*   4 CAN_UNIT_E           CANID_CMD_GPS_1I	 GPS_1: I time sync distribution msg*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_F	// a #define is used to select the following
const struct FUNC_CANID func_canid13[] = {
{   0x01E0000C, CAN_UNIT_F_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1005,  0x0620000C }, /*   1 CAN_UNIT_F           CANID_CMD_CABLE_ANGLE_1R	 Cable angle: R AD7799 #2 drum #1*/
{   5,  0x06200000 }, /*   2 CAN_UNIT_F           CANID_CMD_CABLE_ANGLE_1I	 Cable angle: I AD7799 #2 drum #1*/
{1003,  0x05C0803C }, /*   3 CAN_UNIT_F           CANID_CMD_TENSION_a1WR	 Tension_a: R Tension drum #1 AD7799 #1*/
{   3,  0x05C0003C }, /*   4 CAN_UNIT_F           CANID_CMD_TENSION_a1WI	 Tension_a: I Tension drum #1 AD7799 #1*/
};
#endif
