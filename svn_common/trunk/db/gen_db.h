// Defines from database pcc
// 2018-10-03 12:34:40.345

#define CANID_COUNT 229
#define  CANID_MSG_TENSION_0      0x48000000  // TENSION_a      : Tension_0: Default measurement canid
#define  CANID_MSG_TENSION_a11    0x38000000  // TENSION_a      : Tension_a11: Drum 1 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a21    0x38200000  // TENSION_a      : Tension_a12: Drum 1 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a12    0x38400000  // TENSION_a      : Tension_a21: Drum 2 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a22    0x38600000  // TENSION_a      : Tension_a22: Drum 2 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_2      0x38800000  // TENSION_2      : Tension_2: calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a1G    0xF800043C  // TENSION_a      : Tension_a1G: Drum 2 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a2G    0xF800044C  // TENSION_a      : Tension_a2G: Drum 2 calibrated tension, polled by time msg
#define  CANID_TST_TENSION_a11    0xF800010C  // TENSION_a      : Tension_a11: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a12    0xF800020C  // TENSION_a      : Tension_a12: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a21    0xF800030C  // TENSION_a      : Tension_a21: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a22    0xF800040C  // TENSION_a      : Tension_a22: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a1G    0xF800041C  // TENSION_a      : Tension_a1G: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a2G    0xF800042C  // TENSION_a      : Tension_a2G: TESTING java program generation of idx_v_val.c
#define  CANID_CMD_TENSION_a1WI   0x05C0003C  // TENSION_a      : Tension_a: I 1W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a1WR   0x05C0803C  // TENSION_a      : Tension_a: R 1W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a2WI   0x05C0004C  // TENSION_a      : Tension_a: I 2W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a2WR   0x05C0804C  // TENSION_a      : Tension_a: R 2W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a11I   0x05C00004  // TENSION_a      : Tension_a11: I Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a11R   0x05C0000C  // TENSION_a      : Tension_a11: R Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a21I   0x05E00004  // TENSION_a      : Tension_a21: I Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a21R   0x05E0000C  // TENSION_a      : Tension_a21: R Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a12I   0xF800005C  // TENSION_a      : Tension_a12: I 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a12R   0xF800805C  // TENSION_a      : Tension_a12: R 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a22I   0xF800006C  // TENSION_a      : Tension_a22: I 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a22R   0xF800806C  // TENSION_a      : Tension_a22: R 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a0XI   0xF800001C  // TENSION_a      : Tension_a:  I 1 AD7799 VE POD TESTING (hence 0) 0
#define  CANID_CMD_TENSION_a0XR   0xF800801C  // TENSION_a      : Tension_a:  R 1 AD7799 VE POD TESTING (hence 0) 0
#define  CANID_CMD_TENSION_a1XI   0xF800002C  // TENSION_a      : Tension_a:  I 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a1XR   0xF800802C  // TENSION_a      : Tension_a:  R 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a2XI   0xF800003C  // TENSION_a      : Tension_a2: I 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a2XR   0xF800803C  // TENSION_a      : Tension_a2: R 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a1YI   0xF800004C  // TENSION_a      : Tension_a:  I 1 AD7799 VE POD TESTING (hence Y) 2
#define  CANID_CMD_TENSION_a1YR   0xF800804C  // TENSION_a      : Tension_a:  R 1 AD7799 VE POD TESTING (hence Y) 2
#define  CANID_CMD_TENSION_a1GI   0xF800007C  // TENSION_a      : Tension_a:  I 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_a1GR   0xF800807C  // TENSION_a      : Tension_a:  R 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_a2GI   0xF800008C  // TENSION_a      : Tension_a2: I 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_a2GR   0xF800808C  // TENSION_a      : Tension_a2: R 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_2I     0x05C0005C  // TENSION_2      : Tension_2: I Tension_2: Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_2R     0x05C0805C  // TENSION_2      : Tension_2: R Tension_2: Command code: [0] command code, [1]-[8] depends on code
#define  CANID_MOTOR_1            0x2D000000  // MOTOR_1        : MOTOR_1: Motor speed
#define  CANID_CMD_CABLE_ANGLE_0I 0x06000000  // CABLE_ANGLE_0  : Cable_Angle0: I Default measurement canid
#define  CANID_CMD_CABLE_ANGLE_0R 0x0600000C  // CABLE_ANGLE_0  : Cable_Angle0: R Default measurement canid
#define  CANID_CMD_CABLE_ANGLE_1I 0x06200000  // CABLE_ANGLE_1  : Cable_Angle1: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_CABLE_ANGLE_1R 0x0620000C  // CABLE_ANGLE_1  : Cable_Angle1: R [0] command code, [1]-[8] depends on code
#define  CANID_MSG_CABLE_ANGLE_1  0x3A000000  // CABLE_ANGLE_1  : Cable_Angle1: for drum #1
#define  CANID_MSG_CABLE_ANGLE_1_ALARM 0x2B000000  // CABLE_ANGLE_1  : Cable_Angle1: unreliable for drum #1
#define  CANID_CMD_ENG1_I         0x80600000  // ENGINE_SENSOR  : Engine: code: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_ENG1_R         0x80800000  // ENGINE_SENSOR  : Engine: code: R [0] command code, [1]-[8] depends on code
#define  CANID_MSG_ENG1_RPMMANFLD_R 0x40600000  // ENGINE_SENSOR  : Engine: rpm:manifold pressure
#define  CANID_MSG_ENG1_TEMPTHRTL_R 0x60600000  // ENGINE_SENSOR  : Engine: temperature:throttle pot, calibrated
#define  CANID_HB_ENG1_RPMMANFLD  0x4060000C  // ENGINE_SENSOR  : Engine: raw readings: rpm:manifold pressure
#define  CANID_HB_ENG1_THRMTHRTL  0x6060000C  // ENGINE_SENSOR  : Engine: raw readings: thermistor:throttle
#define  CANID_CMD_ENG1_MANFLDI   0x80A00000  // ENG_MANIFOLD   : Eng1_manfld: code: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_ENG1_MANFLDR   0x80E00000  // ENG_MANIFOLD   : Eng1_manfld: R [0] command code, [1]-[8] depends on code
#define  CANID_MSG_ENG1_MANFLD    0x40800000  // ENG_MANIFOLD   : Eng1_manfld: manifold pressure
#define  CANID_HB_ENG1_MANFLD     0x82200000  // ENG_MANIFOLD   : Eng1_manfld: Heartbeat manifold pressure calibrated
#define  CANID_CMD_ENG1_RPMI      0x81200000  // ENG_RPM        : Eng1_rpm: cmd code: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_ENG1_RPMR      0x81400000  // ENG_RPM        : Eng1_rpm: cmd code: R [0] command code, [1]-[8] depends on code
#define  CANID_MSG_ENG1_RPM       0x40A00000  // ENG_RPM        : Eng1_rpm: RPM calibrated
#define  CANID_HB_ENG1_RPM        0x82600000  // ENG_RPM        : Eng1_rpm: Heartbeat rpm calibrated
#define  CANID_CMD_ENG1_THROTTLEI 0x81600000  // ENG_THROTTLE   : Eng1_throttle: cmd code: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_ENG1_THROTTLER 0x81800000  // ENG_THROTTLE   : Eng1_throttle: cmd code: R [0] command code, [1]-[8] depends on code
#define  CANID_MSG_ENG1_THROTTLE  0x40E00000  // ENG_THROTTLE   : Eng1_throttle: Throttle calibrated
#define  CANID_HB_ENG1_THROTTLE   0x82400000  // ENG_THROTTLE   : Eng1_throttle: Heartbeat throttle calibrated
#define  CANID_CMD_ENG1_T1I       0x81A00000  // ENG_T1         : Eng1_t1: cmd code: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_ENG1_T1R       0x81E00000  // ENG_T1         : Eng1_t1: cmd code: R [0] command code, [1]-[8] depends on code
#define  CANID_MSG_ENG1_T1        0x41200000  // ENG_T1         : Eng1_t1: Temperature #1 calibrated
#define  CANID_HB_ENG1_T1         0x82800000  // ENG_T1         : Eng1_t1: Heartbeat temperature #1 calibrated
#define  CANID_HB_FIX_HT_TYP_NSAT 0xB1C00000  // GPS            : GPS: fix: heigth:type fix:number sats
#define  CANID_HB_FIX_LATLON      0xA1C00000  // GPS            : GPS: fix: lattitude:longitude
#define  CANID_HB_LG_ER1          0xD1C00004  // GPS            : GPS: 1st code  CANID-UNITID_CO_OLI GPS checksum error
#define  CANID_HB_LG_ER2          0xD1C00014  // GPS            : GPS: 2nd code  CANID-UNITID_CO_OLI GPS Fix error
#define  CANID_HB_LG_ER3          0xD1C00024  // GPS            : GPS: 3rd code  CANID-UNITID_CO_OLI GPS Time out of step
#define  CANID_HB_TIMESYNC        0x00400000  // GPS            : GPS_1: GPS time sync distribution msg
#define  CANID_HB_TIMESYNC_2      0x00600000  // GPS            : GPS_2: GPS time sync distribution msg
#define  CANID_HB_TIMESYNC_X      0x03000000  // GPS            : GPS_2: Obsolete GPS time sync distribution msg
#define  CANID_HB_UNIVERSAL_RESET 0x00200000  // GPS            : Highest priority: reserved for Universal (if/when implemented)
#define  CANID_CMD_GPS_1I         0xD1C00044  // GPS            : GPS_1: I CANID Command GPS 1
#define  CANID_CMD_GPS_1R         0xD1C0004C  // GPS            : GPS_1: R CANID Command GPS 1
#define  CANID_CMD_GPS_2I         0xD1C00074  // GPS            : GPS_2: I CANID Command GPS 2
#define  CANID_CMD_GPS_2R         0xD1C0007C  // GPS            : GPS_2: R CANID Command GPS 2
#define  CANID_CMD_LOGGER_1I      0xD1C00054  // LOGGER         : Logger_1: I Command Logger 1
#define  CANID_CMD_LOGGER_1R      0xD1C0005C  // LOGGER         : Logger_1: R Command Logger 1
#define  CANID_CMD_LOGGER_2I      0xD1C00064  // LOGGER         : Logger_2: I Command Logger 2
#define  CANID_CMD_LOGGER_2R      0xD1C0006C  // LOGGER         : Logger_2: R Command Logger 2
#define  CANID_MC_SYSTEM_STATE    0x50000000  // MC             : MC: System state msg
#define  CANID_MC_DRUM_SELECT     0xD0800814  // MC             : MC: Drum selection
#define  CANID_HB_MC_MOTOR_1_KPALIVE 0xA0800000  // MC             : MC: Curtis Controller keepalive
#define  CANID_MC_REQUEST_PARAM   0xD0800824  // MC             : MC: Request parameters from HC
#define  CANID_MC_CONTACTOR       0x23000000  // MC             : MC: Contactor OPEN/CLOSE
#define  CANID_MC_BRAKES          0x21000000  // MC             : MC: Brakes APPLY/RELEASE
#define  CANID_MC_GUILLOTINE      0x22000000  // MC             : MC: Fire guillotine
#define  CANID_MC_RQ_LAUNCH_PARAM 0x27000000  // MC             : MC: Fire request launch parameters
#define  CANID_MSG_TIME_POLL      0x20000000  // MC             : MC: Time msg/Group polling
#define  CANID_MC_STATE           0x26000000  // MC             : MC: Launch state msg
#define  CANID_MC_TORQUE          0x25800000  // MC             : MC: Motor torque
#define  CANID_CMD_MCLI           0xFFE00000  // MCL            : MCL: Master Controller Launch parameters I (HC)
#define  CANID_CMD_MCLR           0xFFE00004  // MCL            : MCL: Master Controller Launch parameters R (MC)
#define  CANID_CP_CTL_RMT         0x29000000  // CP             : Control Panel: Control lever remote
#define  CANID_CP_CTL_LCL         0x29200000  // CP             : Control Panel: Control lever local
#define  CANID_CP_CTL_IN_RMT      0x24C00000  // CP             : Control Panel: Control lever remote: input
#define  CANID_CP_CTL_IN_LCL      0x25000000  // CP             : Control Panel: Control lever  local: input
#define  CANID_CP_CTL_OUT_RMT     0x2A000000  // CP             : Control Panel: Control lever output
#define  CANID_CMD_SHAFT1I        0xA0600000  // DRIVE_SHAFT    : Shaft1: I Command CAN: incoming
#define  CANID_CMD_SHAFT1R        0xE2C00000  // DRIVE_SHAFT    : Shaft1: R Command CAN: repsonse
#define  CANID_MSG_SHAFT1_SPEED   0x82A00000  // DRIVE_SHAFT    : Shaft1: Speed (RPM), polled msg
#define  CANID_MSG_SHAFT1_COUNT   0x82E00000  // DRIVE_SHAFT    : Shaft1: Cumulative count, polled msg
#define  CANID_HB_SHAFT1_SPEED    0xE2800000  // DRIVE_SHAFT    : Shaft1: Speed (RPM), hearbeat
#define  CANID_HB_SHAFT1_COUNT    0xE2A00000  // DRIVE_SHAFT    : Shaft1: Cumulative count, hearbeat
#define  CANID_SE2H_ADC3_HistB    0xD0800034  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: ADC3 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE2H_COUNTERnSPEED 0x30800000  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: (Lower sheave) Count and speed
#define  CANID_SE2H_ERR_1         0xD0800014  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: error1
#define  CANID_SE2H_ERR_2         0xD0800074  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: error2
#define  CANID_CMD_LOWERSHVI      0xD0800000  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: I Command CAN: send commands to subsystem
#define  CANID_CMD_LOWERSHVR      0xD0800004  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: R Command CAN: send commands to subsystem
#define  CANID_SE3H_ADC2_HistA    0xD0A00044  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC2 HistogramA tx:req ct, switch buff;rx send ct
#define  CANID_SE3H_ADC2_HistB    0xD0A00054  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC2 HistogramB tx: bin number,rx: send bin ct
#define  CANID_SE3H_ADC3_ADC2_RD  0xD0A00064  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC3 ADC2 readings readout
#define  CANID_SE3H_ADC3_HistA    0xD0A00024  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC3 HistogramA tx:req ct, switch buff.rx: send ct
#define  CANID_SE3H_ADC3_HistB    0xD0A00034  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC3 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE3H_COUNTERnSPEED 0x30A00000  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: (upper sheave) Count and Speed
#define  CANID_SE3H_ERR_1         0xD0A00014  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: error1
#define  CANID_SE3H_ERR_2         0xD0A00004  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: error2
#define  CANID_CMD_UPPERSHVI      0xD0600000  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: I Command CAN: send commands to subsystem
#define  CANID_CMD_UPPERSHVR      0xD0600004  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: R Command CAN: send commands to subsystem
#define  CANID_SE4H_ADC2_HistA    0xD0400044  // DRIVE_SHAFT    : Drive shaft: ADC2 HistogramA tx: request count, switch buffers; rx send count
#define  CANID_SE4H_ADC2_HistB    0xD0400054  // DRIVE_SHAFT    : Drive shaft: ADC2 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE4H_ADC3_ADC2_RD  0xD0400064  // DRIVE_SHAFT    : Drive shaft: ADC3 ADC2 readings readout
#define  CANID_SE4H_ADC3_HistA    0xD0400024  // DRIVE_SHAFT    : Drive shaft: ADC3 HistogramA tx: request count, switch buffers. rx: send count
#define  CANID_SE4H_ADC3_HistB    0xD0400034  // DRIVE_SHAFT    : Drive shaft: ADC3 HistogramB tx: bin number, rx: send bin count
#define  CANID_CMD_DRIVE_SHAFTI   0xD0C00000  // DRIVE_SHAFT    : Drive shaft: I Command CAN: send commands to subsystem
#define  CANID_CMD_DRIVE_SHAFTR   0xD0C00004  // DRIVE_SHAFT    : Drive shaft: R Command CAN: send commands to subsystem
#define  CANID_SE4H_COUNTERnSPEED 0x30400000  // DRIVE_SHAFT    : Drive shaft: (drive shaft) count and speed
#define  CANID_SE4H_ERR_1         0xD0400014  // DRIVE_SHAFT    : Drive shaft: [2]elapsed_ticks_no_adcticks<2000 ct  [3]cic not in sync
#define  CANID_SE4H_ERR_2         0xD0400004  // DRIVE_SHAFT    : Drive shaft: [0]encode_state er ct [1]adctick_diff<2000 ct
#define  CANID_CMD_UPPER1_HI      0xD0E00000  // SHEAVE_UP_H    : F4 shaft_encoder: Upper sheave hi-res: I Command CAN: send commands to subsystem
#define  CANID_CMD_UPPER1_HR      0xD0E00004  // SHEAVE_UP_H    : F4 shaft_encoder: Upper sheave hi-res: R Command CAN: subsystem responds
#define  CANID_MSG_UPPER1_H_RAW   0xD1000000  // SHEAVE_UP_H    : F4 shaft_encoder: hi-res: msg--upper raw count and delta time
#define  CANID_MSG_UPPER1_H_CAL   0xD1200000  // SHEAVE_UP_H    : F4 shaft_encoder: hi-res: msg--upper calibrated--distance and speed
#define  CANID_HB_UPPER1_H_RAW    0xD1E00000  // SHEAVE_UP_H    : F4 shaft_encoder: hi-res: heartbeat--upper raw count and delta time
#define  CANID_HB_UPPER1_H_CAL    0xD2000000  // SHEAVE_UP_H    : F4 shaft_encoder: hi-res: heartbeat--upper calibrated--distance and speed
#define  CANID_TST_SHEAVE_1UP     0xD2000004  // SHEAVE_UP_H    : F4 shaft_encoder: hi-res: testing poll UP
#define  CANID_CMD_LOWER1_HI      0xD1400000  // SHEAVE_LO_H    : F4 shaft_encoder: Lower sheave hi-res: I Command CAN: send commands to subsystem
#define  CANID_CMD_LOWER1_HR      0xD1600004  // SHEAVE_LO_H    : F4 shaft_encoder: Lower sheave hi-res: R Command CAN: subsystem responds
#define  CANID_MSG_LOWER1_H_RAW   0xD2400000  // SHEAVE_LO_H    : F4 shaft_encoder: hi-res: msg--lower raw count
#define  CANID_MSG_LOWER1_H_CAL   0xD1A00000  // SHEAVE_LO_H    : F4 shaft_encoder: hi-res: msg--lower calibrated--distance and speed
#define  CANID_HB_LOWER1_H_RAW    0xD1800000  // SHEAVE_LO_H    : F4 shaft_encoder: hi-res: heartbeat--lower raw count
#define  CANID_HB_LOWER1_H_CAL    0xD1C00000  // SHEAVE_LO_H    : F4 shaft_encoder: hi-res: heartbeat--lower calibrated--distance and speed
#define  CANID_TST_SHEAVE_1LO     0xD2200004  // SHEAVE_UP_H    : F4 shaft_encoder: hi-res: testing poll LO
#define  CANID_TILT_ALARM         0x00A00000  // TILT_SENSE     : Tilt: alarm: Vector angle exceeds limit
#define  CANID_TILT_ANGLE         0x42E00000  // TILT_SENSE     : Tilt: Calibrated angles (X & Y)
#define  CANID_TILT_XYZ           0x42800000  // TILT_SENSE     : Tilt: Calibrated to angle: x,y,z tilt readings
#define  CANID_TILT_XYZ_CAL       0xFFFFFFCC  // TILT_SENSE     : Tilt: CANID: Raw tilt ADC readings
#define  CANID_TILT_XYZ_RAW       0x4280000C  // TILT_SENSE     : Tilt: Tilt:Raw tilt ADC readings
#define  CANID_CMD_TILTI          0x42C00000  // TILT_SENSE     : Tilt: I Command CANID
#define  CANID_CMD_TILTR          0x42C00004  // TILT_SENSE     : Tilt: R Command CANID
#define  CANID_HB_GATEWAY1        0xE0200000  // GATEWAY        : Gateway1: Heartbeat
#define  CANID_HB_GATEWAY2        0xE1200000  // GATEWAY        : Gateway2: Heartbeat
#define  CANID_HB_GATEWAY3        0xE1400000  // GATEWAY        : Gateway3: Heartbeat
#define  CANID_HB_GATEWAY4        0xE2E00000  // GATEWAY        : Gateway4: Heartbeat
#define  CANID_HB_TENSION_0       0xE0400000  // TENSION_0      : Tension_0: Heartbeat
#define  CANID_HB_TENSION_a11     0xE0600000  // TENSION_a      : Tension_a11: Heartbeat
#define  CANID_HB_TENSION_a21     0xE0C00000  // TENSION_a      : Tension_a21: Heartbeat
#define  CANID_HB_TENSION_a12     0xE0800000  // TENSION_a      : Tension_a12: Heartbeat
#define  CANID_HB_TENSION_a22     0xE0E00000  // TENSION_a      : Tension_a22: Heartbeat
#define  CANID_HB_CABLE_ANGLE_1   0xE0A00000  // CABLE_ANGLE_1  : Cable_Angle_1: Heartbeat
#define  CANID_HB_TENSION_a1G     0xE0E20000  // TENSION_a      : Tension_a1G: Heartbeat
#define  CANID_HB_TENSION_a2G     0xE0E40000  // TENSION_a      : Tension_a2G: Heartbeat
#define  CANID_HB_GPS_TIME_1      0xE1000000  // GPS            : GPS_1: Heartbeat unix time
#define  CANID_HB_GPS_TIME_2      0xE1E00000  // GPS            : GPS_2: Heartbeat unix time
#define  CANID_HB_GPS_LLH_1       0xE1C00000  // GPS            : GPS_1: Heartbeat (3 separate msgs) lattitude longitude height
#define  CANID_HB_GPS_LLH_2       0xE2600000  // GPS            : GPS_2: Heartbeat (3 separate msgs) lattitude longitude height
#define  CANID_HB_LOGGER_1        0xE1800000  // LOGGER         : Logger_1: Heartbeat logging ctr
#define  CANID_HB_LOGGER_2        0xE1A00000  // LOGGER         : Logger_2: Heartbeat logging ctr
#define  CANID_HB_CANSENDER_1     0xF0200000  // CANSENDER      : Cansender_1: Heartbeat w ctr
#define  CANID_HB_CANSENDER_2     0xF0400000  // CANSENDER      : Cansender_2: Heartbeat w ctr
#define  CANID_CMD_CANSENDER_1I   0xA0200000  // CANSENDER      : Cansender_1: I Command CANID
#define  CANID_CMD_CANSENDER_1R   0xA0200004  // CANSENDER      : Cansender_1: R Command CANID
#define  CANID_CMD_CANSENDER_2I   0xA0400000  // CANSENDER      : Cansender_2: I Command CANID
#define  CANID_CMD_CANSENDER_2R   0xA0400004  // CANSENDER      : Cansender_2: R Command CANID
#define  CANID_POLL_CANSENDER     0xE2000000  // CANSENDER      : Cansender: Poll cansenders
#define  CANID_POLLR_CANSENDER_1  0xE2200000  // CANSENDER      : Cansender_1: Response to POLL
#define  CANID_POLLR_CANSENDER_2  0xE2400000  // CANSENDER      : Cansender_2: Response to POLL
#define  CANID_CMD_PWRBOX1I       0xE320000C  // PWRBOX         : Pwrbox1: I Command CANID
#define  CANID_CMD_PWRBOX1R       0xE3200000  // PWRBOX         : Pwrbox1: R Command CANID
#define  CANID_HB_PWRBOX1         0xFF200000  // PWRBOX         : Pwrbox1: Heartbeat w input voltage
#define  CANID_MSG_PWRBOX1        0xE3000000  // PWRBOX         : Pwrbox1: Polled msg
#define  CANID_ALM_PWRBOX1        0x00800000  // PWRBOX         : Pwrbox1: Input voltage low alarm msg
#define  CANID_LVL_HB_PWRBOX1     0xFF400000  // PWRBOX         : Pwrbox1: Heartbeat: level wind motor current
#define  CANID_LVL_MSG_PWRBOX1    0xE3400000  // PWRBOX         : Pwrbox1: Msg: level wind motor current
#define  CANID_CMD_SANDBOX_1I     0x28E00000  // SANDBOX_1      : HC: SANDBOX_1: I Launch parameters
#define  CANID_CMD_SANDBOX_1R     0x28E00004  // SANDBOX_1      : HC: SANDBOX_1: R Launch parameters
#define  CANID_CMD_YOGURT_1I      0x29800000  // YOGURT_1       : Yogurt: YOGURT_1: I Yogurt maker parameters
#define  CANID_CMD_YOGURT_1R      0x29800004  // YOGURT_1       : Yogurt: YOGURT_1: R Yogurt maker parameters
#define  CANID_MSG_YOGURT_1       0x29400000  // YOGURT_1       : Yogurt: YOGURT_1: Yogurt maker msgs
#define  CANID_HB_YOGURT_1        0x29600000  // YOGURT_1       : Yogurt: YOGURT_1: Heart-beats
#define  CANID_UNIT_2             0x04000000  // UNIT_2         : Sensor unit: Drive shaft encoder #1
#define  CANID_UNIT_3             0x03800000  // UNIT_3         : Sensor unit: Engine
#define  CANID_UNIT_4             0x03A00000  // UNIT_4         : Sensor unit: Lower sheave shaft encoder
#define  CANID_UNIT_5             0x03C00000  // UNIT_5         : Sensor unit: Upper sheave shaft encoder
#define  CANID_UNIT_8             0x01000000  // UNIT_8         : Sensor unit: Level wind
#define  CANID_UNIT_9             0x01200000  // UNIT_9         : Sensor unit: XBee receiver #1
#define  CANID_UNIT_A             0x0140000C  // UNIT_A         : Sensor unit: XBee receiver #2
#define  CANID_UNIT_B             0x0160000C  // UNIT_B         : Display driver/console
#define  CANID_UNIT_C             0x0180000C  // UNIT_C         : CAWs Olimex board
#define  CANID_UNIT_D             0x01A0000C  // UNIT_D         : POD board sensor prototype ("6" marked on board)
#define  CANID_UNIT_E             0x01C0000C  // UNIT_E         : Logger1: sensor board w ublox gps & SD card GSM 18-09-17
#define  CANID_UNIT_F             0x01E0000C  // UNIT_F         : Tension_1 & Cable_angle_1 Unit
#define  CANID_UNIT_10            0x0200000C  // UNIT_10        : Gateway1: 2 CAN
#define  CANID_UNIT_19            0x02800000  // UNIT_19        : Master Controller
#define  CANID_UNIT_11            0x02200000  // UNIT_11        : Tension: 1 AD7799 VE POD brd 1
#define  CANID_UNIT_12            0x02400000  // UNIT_12        : Tension: 2 AD7799 VE POD brd 2 GSM 18-09-17
#define  CANID_UNIT_13            0x02600000  // UNIT_13        : Yogurt: Olimex board
#define  CANID_UNIT_14            0x02E00000  // UNIT_14        : Tension: 1 AD7799 w op-amp VE POD brd 3
#define  CANID_UNIT_15            0x02A00000  // UNIT_15        : Tension: 2 AD7799 VE POD brd 4
#define  CANID_UNIT_16            0x02C00000  // UNIT_16        : Tension: 2 AD7799 VE POD brd 5 GSM
#define  CANID_UNIT_17            0x03200000  // UNIT_17        : Gateway2: 1 CAN GSM 18-09-17
#define  CANID_UNIT_18            0x03400000  // UNIT_18        : Gateway3: 1 CAN
#define  CANID_UNIT_1A            0x03600000  // UNIT_1A        : Logger2: sensor board w ublox gps & SD card
#define  CANID_UNIT_1B            0x03E00000  // UNIT_1B        : Sensor board: CAW experiments
#define  CANID_UNIT_1C            0x04200000  // UNIT_1C        : Sensor board: DEH spare 1
#define  CANID_UNIT_1D            0x04400000  // UNIT_1D        : Sensor board: DEH spare 2
#define  CANID_UNIT_1E            0x04800000  // UNIT_1E        : DiscoveryF4 shaft encoder unit 1
#define  CANID_UNIT_1F            0x04A00000  // UNIT_1F        : Pwrbox: Blue Pill board
#define  CANID_UNIT_20            0x04E00000  // UNIT_20        : Gateway4: 1 CAN
#define  CANID_UNIT_99            0xFFFFFF14  // UNIT_99        : Dummy for missing CAN IDs
#define  CANID_DUMMY              0xFFFFFFFC  // UNIT_NU        : Dummy ID: Lowest priority possible (Not Used)
#define  CANID_MSG_DUMMY          0xFFFFFF16  // ANY            : Dummy ID: Polled Msg dummy

#define NUMBER_TYPE_COUNT 17
#define  TYP_S8                  1         // 1             int8_t,   signed char, 1 byte
#define  TYP_U8                  2         // 1            uint8_t, unsigned char, 1 byte
#define  TYP_S16                 3         // 2            int16_t,   signed short, 2 bytes
#define  TYP_U16                 4         // 2           uint16_t, unsigned short, 2 bytes
#define  TYP_S32                 5         // 4            int32_t,   signed int, 4 bytes
#define  TYP_U32                 6         // 4           uint32_t, unsigned int, 4 bytes
#define  TYP_S64_L               7         // 4            int64_t,   signed long long, low  order 4 bytes
#define  TYP_S64_H               8         // 4            int64_t,   signed long long, high order 4 bytes
#define  TYP_U64_L               9         // 4           uint64_t, unsigned long long, low  order 4 bytes
#define  TYP_U64_H               10        // 4           uint64_t, unsigned long long, high order 4 bytes
#define  TYP_FLT                 11        // 4           float, 4 bytes
#define  TYP_12FLT               12        // 2           half-float, 2 bytes
#define  TYP_34FLT               13        // 3           3/4-float, 3 bytes
#define  TYP_DBL_L               14        // 4           double, low  order 4 bytes
#define  TYP_DBL_H               15        // 4           double, high order 4 bytes
#define  TYP_ASC                 16        // 4           ascii chars
#define  TYP_CANID               17        // 1           CANID (handled differently than a U32)

#define CMD_CODES_COUNT 36
#define  LDR_SET_ADDR            1         // 5 Set address pointer (not FLASH) (bytes 2-5):  Respond with last written address.
#define  LDR_SET_ADDR_FL         2         // 5 Set address pointer (FLASH) (bytes 2-5):  Respond with last written address.
#define  LDR_CRC                 3         // 8 Get CRC: 2-4 = count; 5-8 = start address; Reply CRC 2-4 na, 5-8 computed CRC 
#define  LDR_ACK                 4         // 1 ACK: Positive acknowledge (Get next something)
#define  LDR_NACK                5         // 1 NACK: Negative acknowledge (So? How do we know it is wrong?)
#define  LDR_JMP                 6         // 5 Jump: to address supplied (bytes 2-5)
#define  LDR_WRBLK               7         // 1 Done with block: write block with whatever you have.
#define  LDR_RESET               8         // 1 RESET: Execute a software forced RESET
#define  LDR_XON                 9         // 1 Resume sending
#define  LDR_XOFF                10        // 1 Stop sending
#define  LDR_FLASHSIZE           11        // 1 Get flash size; bytes 2-3 = flash block size (short)
#define  LDR_ADDR_OOB            12        // 1 Address is out-of-bounds
#define  LDR_DLC_ERR             13        // 1 Unexpected DLC
#define  LDR_FIXEDADDR           14        // 5 Get address of flash with fixed loader info (e.g. unique CAN ID)
#define  LDR_RD4                 15        // 5 Read 4 bytes at address (bytes 2-5)
#define  LDR_APPOFFSET           16        // 5 Get address where application begins storing.
#define  LDR_HIGHFLASHH          17        // 5 Get address of beginning of struct with crc check and CAN ID info for app
#define  LDR_HIGHFLASHP          18        // 8 Get address and size of struct with app calibrations, parameters, etc.
#define  LDR_ASCII_SW            19        // 2 Switch mode to send printf ASCII in CAN msgs
#define  LDR_ASCII_DAT           20        // 3-8 [1]=line position;[2]-[8]=ASCII chars
#define  LDR_WRVAL_PTR           21        // 2-8 Write: 2-8=bytes to be written via address ptr previous set.
#define  LDR_WRVAL_PTR_SIZE      22        // Write data payload size
#define  LDR_WRVAL_AI            23        // 8 Write: 2=memory area; 3-4=index; 5-8=one 4 byte value
#define  LDR_SQUELCH             24        // 8 Send squelch sending tick ct: 2-8 count
#define  CMD_GET_IDENT           30        // Get parameter using indentification name/number in byte [1]
#define  CMD_PUT_IDENT           31        // Put parameter using indentification name/number in byte [1]
#define  CMD_GET_INDEX           32        // Get parameter using index name/number in byte [1]
#define  CMD_PUT_INDEX           33        // Put parameter using index name/number in byte [1]; parameter in [2]-[5]
#define  CMD_REVERT              34        // Revert (re-initialize) working parameters/calibrations/CANIDs back to stored non-volatile values
#define  CMD_SAVE                35        // Write current working parameters/calibrations/CANIDs to non-volatile storage
#define  CMD_GET_READING         36        // Send a reading for the code specified in byte [1] specific to function
#define  CMD_GET_READING_BRD     37        // Send a reading for the code specified in byte [1] for board; common to functions
#define  CMD_LAUNCH_PARM_HDSHK   38        // Send msg to handshake transferring launch parameters
#define  CMD_SEND_LAUNCH_PARM    39        // Send msg to send burst of parameters
#define  CMD_REQ_HISTOGRAM       40        // Request histogram: [1] ADC #: [2] # consecutive:[3]-[6] # DMA buffers accumuleted in bins
#define  CMD_THISIS_HISTODATA    41        // Histogram data item: [1] ADC #:[2] bin # (0 - n), [3]-[6] bin count

#define PAYLOAD_TYPE_COUNT 30
#define  NONE                    0         //  No payload bytes                               
#define  FF                      1         //  [0]-[3]: Full Float                            
#define  FF_FF                   2         //  [0]-[3]: Full Float[0]; [4]-[7]: Full Float[1] 
#define  U32                     3         //  [0]-[3]: uint32_t                              
#define  U32_U32                 4         //  [0]-[3]: uint32_t[0]; [4]-[7]: uint32_t[1]     
#define  U8_U32                  5         //  [0]: uint8_t; [1]-[4]: uint32_t                
#define  S32                     6         //  [0]-[3]: int32_t                               
#define  S32_S32                 7         //  [0]-[3]: int32_t[0]; [4]-[7]: int32_t[1]       
#define  U8_S32                  8         //  [0]: int8_t; [4]-[7]: int32_t                  
#define  HF                      9         //  [0]-[1]: Half-Float                            
#define  F34F                    10        //  [0]-[2]: 3/4-Float                             
#define  xFF                     11        //  [0]:[1]-[4]: Full-Float, first   byte  skipped 
#define  xxFF                    12        //  [0]:[1]:[2]-[5]: Full-Float, first 2 bytes skipped
#define  xxU32                   13        //  [0]:[1]:[2]-[5]: uint32_t, first 2 bytes skipped
#define  xxS32                   14        //  [0]:[1]:[2]-[5]: int32_t, first 2 bytes skipped
#define  U8_U8_U32               15        //  [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1],uint32_t,
#define  U8_U8_S32               16        //  [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], int32_t,
#define  U8_U8_FF                17        //  [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], Full Float,
#define  U16                     18        //  [0]-[2]uint16_t                                
#define  S16                     19        //  [0]-[2] int16_t                                
#define  LAT_LON_HT              20        //  [0]:[1]:[2]-[5]: Fix type, bits fields, lat/lon/ht
#define  U8_FF                   21        //  [0]:[1]-[4]: uint8_t, Full Float               
#define  U8_HF                   22        //  [0]:[1]-[2]: uint8_t, Half Float               
#define  U8                      23        //  [0]: uint8_t                                   
#define  UNIXTIME                24        //  [0]: U8_U32 with U8 bit field stuff            
#define  U8_U8                   25        //  [0]:[1]: uint8_t[0],uint8[1]                   
#define  U8_U8_U8_U32            26        //  [0]:[1]:[2]:[3]-[5]: uint8_t[0],uint8_t[0],uint8_t[1], int32_t,
#define  LVL2B                   249       //  [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Board code),[2]-[5]see table
#define  LVL2R                   250       //  [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Readings code),[2]-[5]see table
#define  UNDEF                   255       //  Undefined                                      

#define PARAM_LIST_COUNT 338	// TOTAL COUNT OF PARAMETER LIST

#define  PWRBOX_CRC              	1         // Pwrbox:: CRC                                    
#define  PWRBOX_VERSION          	2         // Pwrbox:: Version number                         
#define  PWRBOX_HEARTBEAT_TIME_CT	3         // Pwrbox:: Time (ms) between HB msg               
#define  PWRBOX_CAL_OFFSET1      	4         // Pwrbox: ADC reading[0] offset                   
#define  PWRBOX_CAL_SCALE1       	5         // Pwrbox: ADC reading[0] scale                    
#define  PWRBOX_CAL_OFFSET2      	6         // Pwrbox: ADC reading[1] offset                   
#define  PWRBOX_CAL_SCALE2       	7         // Pwrbox: ADC reading[1] scale                    
#define  PWRBOX_CAL_OFFSET3      	8         // Pwrbox: ADC reading[2] offset                   
#define  PWRBOX_CAL_SCALE3       	9         // Pwrbox: ADC reading[2] scale                    
#define  PWRBOX_CAL_OFFSET4      	10        // Pwrbox: ADC reading[3] offset                   
#define  PWRBOX_CAL_SCALE4       	11        // Pwrbox: ADC reading[3] scale                    
#define  PWRBOX_CAL_OFFSET5      	12        // Pwrbox: ADC reading[4] offset                   
#define  PWRBOX_CAL_SCALE5       	13        // Pwrbox: ADC reading[4] scale                    
#define  PWRBOX_CAL_OFFSET6      	14        // Pwrbox: ADC reading[5] offset                   
#define  PWRBOX_CAL_SCALE6       	15        // Pwrbox: ADC reading[5] scale                    
#define  PWRBOX_CAL_OFFSET7      	16        // Pwrbox: ADC reading[6] offset                   
#define  PWRBOX_CAL_SCALE7       	17        // Pwrbox: ADC reading[6] scale                    
#define  PWRBOX_CAL_OFFSET8      	18        // Pwrbox: ADC reading[7] offset                   
#define  PWRBOX_CAL_SCALE8       	19        // Pwrbox: ADC reading[7] scale                    
#define  PWRBOX_IIR0_K           	20        // Pwrbox: IIR0 Filter factor: divisor sets time constant
#define  PWRBOX_IIR0_SCALE       	21        // Pwrbox: IIR0 Filter scale : upscaling (due to integer math)
#define  PWRBOX_IIR1_K           	22        // Pwrbox: IIR1 Filter factor: divisor sets time constant
#define  PWRBOX_IIR1_SCALE       	23        // Pwrbox: IIR1 Filter scale : upscaling (due to integer math)
#define  PWRBOX_IIR2_K           	24        // Pwrbox: IIR2 Filter factor: divisor sets time constant
#define  PWRBOX_IIR2_SCALE       	25        // Pwrbox: IIR2 Filter scale : upscaling (due to integer math)
#define  PWRBOX_IIR3_K           	26        // Pwrbox: IIR3 Filter factor: divisor sets time constant
#define  PWRBOX_IIR3_SCALE       	27        // Pwrbox: IIR3 Filter scale : upscaling (due to integer math)
#define  PWRBOX_HB_R             	28        // Pwrbox: CANID: Heartbeat: input voltage, bus voltage
#define  PWRBOX_MSG_R            	29        // Pwrbox: CANID: Msg: input voltage, bus voltage  
#define  PWRBOX_ALARM_R          	30        // Pwrbox: CANID: Alarm: input voltage, bus voltage
#define  PWRBOX_ALARM_RATE       	31        // Pwrbox: Time (ms) between alarm msgs, when below threshold
#define  PWRBOX_ALARM_THRES      	32        // Pwrbox: Voltage threshold for alarm msgs        
#define  PWRBOX_CANID_HW_FILT1   	33        // Pwrbox: CANID 1 added to CAN hw filter to allow incoming msg
#define  PWRBOX_CANID_HW_FILT2   	34        // Pwrbox: CANID 2 added to CAN hw filter to allow incoming msg
#define  PWRBOX_CANID_HW_FILT3   	35        // Pwrbox: CANID 3 added to CAN hw filter to allow incoming msg
#define  PWRBOX_CANID_HW_FILT4   	36        // Pwrbox: CANID 4 added to CAN hw filter to allow incoming msg
#define  PWRBOX_CANID_HW_FILT5   	37        // Pwrbox: CANID 5 added to CAN hw filter to allow incoming msg
#define  PWRBOX_CANID_HW_FILT6   	38        // Pwrbox: CANID 6 added to CAN hw filter to allow incoming msg
#define  PWRBOX_CANID_HW_FILT7   	39        // Pwrbox: CANID 7 added to CAN hw filter to allow incoming msg
#define  PWRBOX_CANID_HW_FILT8   	40        // Pwrbox: CANID 8 added to CAN hw filter to allow incoming msg
#define  PWRBOX_LVL_HB_R         	41        // Pwrbox:: CANID: Heartbeat: level wind current   
#define  PWRBOX_LVL_MSG_R        	42        // Pwrbox:: CANID: Msg: level wind current         

#define PARAM_LIST_CT_PWRBOX	42	// Count of same FUNCTION_TYPE in preceding list

#define  ENCODER_LIST_CRC        	1         // Encoder_f4_1: crc: CRC for tension list         
#define  ENCODER_LIST_VERSION    	2         // Encoder_f4_1: version: Version number for Tension List
#define  ENCODER_HEARTBEAT_CT    	3         // Encoder_f4_1: Heartbeat count of time (ms) between msgs
#define  ENCODER_CT_PER_REV      	4         // Encoder_f4_1: Number of counts per revolution   
#define  ENCODER_DIST_PER_REV    	5         // Encoder_f4_1: Distance per revolution (meters)  
#define  ENCODER_HEARTBEAT_MSG   	6         // Encoder_f4_1: Heartbeat sends raw (long long) running encoder count
#define  ENCODER_POLL_MSG        	7         // Encoder_f4_1: CANID: hi-res: msg--upper calibrated--distance and speed
#define  ENCODER_POLL            	8         // Encoder_f4_1: CANID: Poll with time sync msg    
#define  ENCODER_POLL_R          	9         // Encoder_f4_1: CANID: Response to POLL           
#define  SHEAVE_H_CANID_HW_FILT1 	10        // Encoder_f4_1: CANID 1 added to CAN hw filter to allow incoming msg
#define  SHEAVE_H_CANID_HW_FILT2 	11        // Encoder_f4_1: CANID 2 added to CAN hw filter to allow incoming msg
#define  SHEAVE_H_CANID_HW_FILT3 	12        // Encoder_f4_1: CANID 3 added to CAN hw filter to allow incoming msg
#define  SHEAVE_H_CANID_HW_FILT4 	13        // Encoder_f4_1: CANID 4 added to CAN hw filter to allow incoming msg
#define  SHEAVE_H_CANID_HW_FILT5 	14        // Encoder_f4_1: CANID 5 added to CAN hw filter to allow incoming msg
#define  SHEAVE_H_CANID_HW_FILT6 	15        // Encoder_f4_1: CANID 6 added to CAN hw filter to allow incoming msg
#define  SHEAVE_H_CANID_HW_FILT7 	16        // Encoder_f4_1: CANID 7 added to CAN hw filter to allow incoming msg
#define  SHEAVE_H_CANID_HW_FILT8 	17        // Encoder_f4_1: CANID 8 added to CAN hw filter to allow incoming msg

#define PARAM_LIST_CT_SHEAVE_H	17	// Count of same FUNCTION_TYPE in preceding list

#define  CANSENDER_LIST_CRC      	1         // Cansender: CRC                                  
#define  CANSENDER_LIST_VERSION  	2         // Cansender: Version number                       
#define  CANSENDER_HEARTBEAT_CT  	3         // Cansender: Heartbeat count of time (ms) between msgs
#define  CANSENDER_HEARTBEAT_MSG 	4         // Cansender: CANID: Hearbeat sends running count  
#define  CANSENDER_POLL          	5         // Cansender: CANID: Poll this cansender           
#define  CANSENDER_POLL_R        	6         // Cansender: CANID: Response to POLL              

#define PARAM_LIST_CT_CANSENDER	6	// Count of same FUNCTION_TYPE in preceding list

#define  SHAFT_LIST_CRC          	1         // Shaft: CRC                                      
#define  SHAFT_LIST_VERSION      	2         // Shaft: Version number                           
#define  SHAFT_HEARTBEAT_CT      	3         // Shaft: Heartbeat count of time (ms) between msgs
#define  SHAFT_ADC3_HTR          	4         // Shaft: High threshold register setting, ADC3    
#define  SHAFT_ADC3_LTR          	5         // Shaft: Low  threshold register setting, ADC3    
#define  SHAFT_ADC2_HTR          	6         // Shaft: High threshold register setting, ADC2    
#define  SHAFT_ADC2_LTR          	7         // Shaft: Low  threshold register setting, ADC2    
#define  SHAFT_CID_MSG_SPEED     	8         // Shaft: CANID: Shaft speed, calibrated, response to poll
#define  SHAFT_CID_HB_SPEED      	9         // Shaft: CANID: Shaft speed, calibrated, heartbeat
#define  SHAFT_CID_MSG_CT        	10        // Shaft: CANID: Shaft running count, response to poll
#define  SHAFT_CID_HB_CT         	11        // Shaft: CANID: Shaft running count, heartbeat    
#define  SHAFT_NUMBER_SEG        	12        // Shaft: Number of segments on code wheel         
#define  SHAFT_CANID_HW_FILT1    	13        // Shaft: CANID 1 added to CAN hw filter to allow incoming msg
#define  SHAFT_CANID_HW_FILT2    	14        // Shaft: CANID 2 added to CAN hw filter to allow incoming msg
#define  SHAFT_CANID_HW_FILT3    	15        // Shaft: CANID 3 added to CAN hw filter to allow incoming msg
#define  SHAFT_CANID_HW_FILT4    	16        // Shaft: CANID 4 added to CAN hw filter to allow incoming msg
#define  SHAFT_CANID_HW_FILT5    	17        // Shaft: CANID 5 added to CAN hw filter to allow incoming msg
#define  SHAFT_CANID_HW_FILT6    	18        // Shaft: CANID 6 added to CAN hw filter to allow incoming msg
#define  SHAFT_CANID_HW_FILT7    	19        // Shaft: CANID 7 added to CAN hw filter to allow incoming msg
#define  SHAFT_CANID_HW_FILT8    	20        // Shaft: CANID 8 added to CAN hw filter to allow incoming msg
#define  SHAFT_IIR_HB_K          	21        // Shaft: IIR Filter factor: divisor sets time constant: reading for hb msg
#define  SHAFT_IIR_HB_SCALE      	22        // Shaft: IIR Filter scale : upscaling (due to integer math): for hb msg

#define PARAM_LIST_CT_DRIVE_SHAFT	22	// Count of same FUNCTION_TYPE in preceding list

#define  TENSION_a_LIST_CRC      	1         // Tension_a: crc: CRC for tension list            
#define  TENSION_a_LIST_VERSION  	2         // Tension_a: version: Version number for Tension List
#define  TENSION_a_AD7799_1_OFFSET	3         // Tension_a: offset: AD7799 #1 offset             
#define  TENSION_a_AD7799_1_SCALE	4         // Tension_a: scale: AD7799 #1 Scale (convert to kgf)
#define  TENSION_a_THERM1_CONST_B	5         // Tension_a: Thermistor1 param: B: constant B     
#define  TENSION_a_THERM1_R_SERIES	6         // Tension_a: Thermistor1 param: RS: Series resistor, fixed (K ohms)
#define  TENSION_a_THERM1_R_ROOMTMP	7         // Tension_a: Thermistor1 param: R0: Thermistor room temp resistance (K ohms)
#define  TENSION_a_THERM1_REF_TEMP	8         // Tension_a: Thermistor1 param: TREF: Reference temp for thermistor
#define  TENSION_a_THERM1_TEMP_OFFSET	9         // Tension_a: Thermistor1 param: offset: Thermistor temp offset correction (deg C)
#define  TENSION_a_THERM1_TEMP_SCALE	10        // Tension_a: Thermistor1 param: B: scale:  Thermistor temp scale correction
#define  TENSION_a_THERM2_CONST_B	11        // Tension_a: Thermistor2 param: RS: constant B    
#define  TENSION_a_THERM2_R_SERIES	12        // Tension_a: Thermistor2 param: Series resistor, fixed (K ohms)
#define  TENSION_a_THERM2_R_ROOMTMP	13        // Tension_a: Thermistor2 param: R0: Thermistor room temp resistance (K ohms)
#define  TENSION_a_THERM2_REF_TEMP	14        // Tension_a: Thermistor2 param: TREF: Reference temp for thermistor
#define  TENSION_a_THERM2_TEMP_OFFSET	15        // Tension_a: Thermistor2 param: offset: hermistor temp offset correction (deg C)
#define  TENSION_a_THERM2_TEMP_SCALE	16        // Tension_a: Thermistor2 param: scale: Thermistor temp scale correction
#define  TENSION_a_THERM1_COEF_0 	17        // Tension_a: Thermistor1 param: comp_t1[0]: Load-Cell poly coeff 0 (offset)
#define  TENSION_a_THERM1_COEF_1 	18        // Tension_a: Thermistor1 param: comp_t1[1]: Load-Cell poly coeff 1 (scale)
#define  TENSION_a_THERM1_COEF_2 	19        // Tension_a: Thermistor1 param: comp_t1[2]: Load-Cell poly coeff 2 (x^2)
#define  TENSION_a_THERM1_COEF_3 	20        // Tension_a: Thermistor1 param: comp_t1[3]: Load-Cell poly coeff 3 (x^3)
#define  TENSION_a_THERM2_COEF_0 	21        // Tension_a: Thermistor2 param: comp_t2[0]: Load-Cell poly coeff 0 (offset)
#define  TENSION_a_THERM2_COEF_1 	22        // Tension_a: Thermistor2 param: comp_t2[1]: Load-Cell poly coeff 1 (scale)
#define  TENSION_a_THERM2_COEF_2 	23        // Tension_a: Thermistor2 param: comp_t2[2]: Load-Cell poly coeff 2 (x^2)
#define  TENSION_a_THERM2_COEF_3 	24        // Tension_a: Thermistor2 param: comp_t2[3]: Load-Cell poly coeff 3 (x^3)
#define  TENSION_a_HEARTBEAT_CT  	25        // Tension_a: hbct: Heart-Beat Count of time (milliseconds) between autonomous msgs
#define  TENSION_a_DRUM_NUMBER   	26        // Tension_a: drum: Drum system number for this function instance
#define  TENSION_a_DRUM_FUNCTION_BIT	27        // Tension_a: f_pollbit: Drum sys poll 1st payload byte bit, drum # (function instance)
#define  TENSION_a_DRUM_POLL_BIT 	28        // Tension_a: p_pollbit: Drum system poll 2nd payload byte bit for this function
#define  TENSION_a_CANPRM_TENSION	29        // Tension_a: CANID: cid_ten_msg:  canid msg Tension
#define  TENSION_a_MSG_TIME_POLL 	30        // Tension_a: CANID: cid_ten_poll:  canid MC: Time msg/Group polling
#define  TENSION_a_TIMESYNC      	31        // Tension_a: CANID: cid_gps_sync: canid time: GPS time sync distribution
#define  TENSION_a_HEARTBEAT     	32        // Tension_a: CANID: heartbeat                     
#define  TENSION_a_CANIDTEST     	33        // Tension_a: CANID: testing java program          
#define  TENSION_a_IIR_POLL_K    	34        // Tension_a: IIR Filter factor: divisor sets time constant: polled msg
#define  TENSION_a_IIR_POLL_SCALE	35        // Tension_a: IIR Filter scale : upscaling (due to integer math): for polled msg
#define  TENSION_a_IIR_HB_K      	36        // Tension_a: IIR Filter factor: divisor sets time constant: reading for hb msg
#define  TENSION_a_IIR_HB_SCALE  	37        // Tension_a: IIR Filter scale : upscaling (due to integer math): for hb msg
#define  TENSION_a_USEME         	38        // Tension_a: skip or use this function switch     
#define  TENSION_a_IIR_Z_RECAL_K 	39        // Tension_a: IIR Filter factor: divisor sets time constant: zero recalibration
#define  TENSION_a_IIR_Z_RECAL_SCALE	40        // Tension_a: IIR Filter scale : upscaling (integer math): zero recalibration
#define  TENSION_a_Z_RECAL_CT    	41        // Tension_a: ADC conversion counts between zero recalibrations
#define  TENSION_a_LIMIT_HI      	42        // Tension_a: Exceeding this calibrated limit (+) means invalid reading
#define  TENSION_a_LIMIT_LO      	43        // Tension_a: Exceeding this calibrated limit (-) means invalid reading
#define  TENSION_a_CANID_HW_FILT1	44        // Tension_a: CANID1 parameter list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT2	45        // Tension_a: CANID2 CANID parameter list for CAN hw filter to allow
#define  TENSION_a_CANID_HW_FILT3	46        // Tension_a: CANID3 CANID parameter list for CAN hw filter to allow
#define  TENSION_a_CANID_HW_FILT4	47        // Tension_a: CANID4 CANID parameter list for CAN hw filter to allow
#define  TENSION_a_CANID_HW_FILT5	48        // Tension_a: CANID5 CANID parameter list for CAN hw filter to allow
#define  TENSION_a_CANID_HW_FILT6	49        // Tension_a: CANID6 CANID parameter list for CAN hw filter to allow
#define  TENSION_a_CANID_HW_FILT7	50        // Tension_a: CANID7 CANID parameter list for CAN hw filter to allow
#define  TENSION_a_CANID_HW_FILT8	51        // Tension_a: CANID8 CANID parameter list for CAN hw filter to allow

#define PARAM_LIST_CT_TENSION_a	51	// Count of same FUNCTION_TYPE in preceding list

#define  CABLE_ANGLE_LIST_CRC    	1         // Cable Angle: CRC for cable angle list           
#define  CABLE_ANGLE_LIST_VERSION	2         // Cable Angle: Version number for Cable Angle List
#define  CABLE_ANGLE_HEARTBEAT_CT	3         // Cable Angle: Heart-Beat: Count of time ticks between autonomous msgs
#define  CABLE_ANGLE_DRUM_NUMBER 	4         // Cable Angle: drum: Drum system number for this function instance
#define  CABLE_ANGLE_DRUM_FUNCTION_BIT	5         // Cable Angle: f_pollbit: Drum system poll 1st payload byte bit for drum # (function instance)
#define  CABLE_ANGLE_DRUM_POLL_BIT	6         // Cable Angle: p_pollbit: Drum system poll 2nd payload byte bit for this type of function
#define  CABLE_ANGLE_MIN_TENSION 	7         // Cable Angle: Minimum tension required (units to match)
#define  CABLE_ANGLE_RATE_CT     	8         // Cable Angle: Rate count: Number of tension readings between cable angle msgs
#define  CABLE_ANGLE_ALARM_REPEAT	9         // Cable Angle: Number of times alarm msg is repeated
#define  CABLE_ANGLE_CALIB_COEF_0	10        // Cable Angle: Cable angle poly coeff 0           
#define  CABLE_ANGLE_CALIB_COEF_1	11        // Cable Angle: Cable angle poly coeff 1           
#define  CABLE_ANGLE_CALIB_COEF_2	12        // Cable Angle: Cable angle poly coeff 2           
#define  CABLE_ANGLE_CALIB_COEF_3	13        // Cable Angle: Cable angle poly coeff 3           
#define  CABLE_ANGLE_CANPRM_TENSION	14        // Cable Angle: CANID: can msg tension for sheave load-pin
#define  CABLE_ANGLE_MSG_TIME_POLL	15        // Cable Angle: CANID: cid_ten_poll: canid MC: Time msg/Group polling
#define  CABLE_ANGLE_TIMESYNC    	16        // Cable Angle: CANID: cid_gps_sync: canid time: GPS time sync distribution
#define  CABLE_ANGLE_HEARTBEAT_MSG	17        // Cable Angle: CANID: Heart-Beat: msg             
#define  CABLE_ANGLE_USEME       	18        // Cable Angle:  skip or use this function switch  

#define PARAM_LIST_CT_CABLE_ANGLE	18	// Count of same FUNCTION_TYPE in preceding list

#define  GPS_LIST_CRC            	1         // GPS: CRC                                        
#define  GPS_LIST_VERSION        	2         // GPS: Version number                             
#define  GPS_HEARTBEAT_TIME_CT   	3         // GPS: Time (ms) between unix time msgs           
#define  GPS_HEARTBEAT_LLH_CT    	4         // GPS: Time (ms) between burst of lat lon height msgs
#define  GPS_HEARTBEAT_LLH_DELAY_CT	5         // GPS: Time (ms) between lat/lon and lon/ht msgs  
#define  GPS_HEARTBEAT_TIME      	6         // GPS: Heartbeat unix time                        
#define  GPS_HEARTBEAT_LLH       	7         // GPS: Heartbeat (3 separate msgs) lattitude longitude height
#define  GPS_DISABLE_SYNCMSGS    	8         // GPS: time sync msgs; 0 = enable  1 = disable    
#define  GPS_TIME_SYNC_MSG       	9         // GPS: Time sync msg                              

#define PARAM_LIST_CT_GPS	9	// Count of same FUNCTION_TYPE in preceding list

#define  LOGGER_LIST_CRC         	1         // Logger: CRC                                     
#define  LOGGER_LIST_VERSION     	2         // Logger: Version number                          
#define  LOGGER_HEARTBEAT1_CT    	3         // Logger: Heartbeat count of time (ms) between msgs
#define  LOGGER_HEARTBEAT_MSG    	4         // Logger: CANID: Hearbeat sends running count of logged msgs
#define  LOGGER_SUSPEND_R        	5         // Logger: CANID: Low voltage alarm msg suspends logging 
#define  LOGGER_SUSPEND_CT       	6         // Logger: Suspend duration (ms) after receiving alarm msg

#define PARAM_LIST_CT_LOGGER	6	// Count of same FUNCTION_TYPE in preceding list

#define  ENG_MAN_CRC             	1         // Eng_manifold: CRC                               
#define  ENG_MAN_VERSION         	2         // Eng_manifold: Version number                    
#define  ENG_MAN_HEARTBEAT_TIME_CT	3         // Eng_manifold: Time (ms) between HB msg          
#define  ENG_MAN_PRESS_OFFSET    	4         // Eng_manifold: Manifold pressure offset          
#define  ENG_MAN_PRESS_SCALE     	5         // Eng_manifold: Manifold pressure  scale (inch Hg)
#define  ENG_MAN_POLL_MANFLD_R   	6         // Eng_manifold: CANID: Poll response: manifold pressure, calibrated
#define  ENG_MAN_HB_MANFLD_R     	7         // Eng_manifold: CANID: Heartbeat: manifold pressure, calibrated
#define  ENG_MAN_CANID_HW_FILT1  	8         // Eng_manifold: CANID 1 added to CAN hw filter to allow incoming msg
#define  ENG_MAN_CANID_HW_FILT2  	9         // Eng_manifold: CANID 2 added to CAN hw filter to allow incoming msg
#define  ENG_MAN_CANID_HW_FILT3  	10        // Eng_manifold: CANID 3 added to CAN hw filter to allow incoming msg
#define  ENG_MAN_CANID_HW_FILT4  	11        // Eng_manifold: CANID 4 added to CAN hw filter to allow incoming msg
#define  ENG_MAN_CANID_HW_FILT5  	12        // Eng_manifold: CANID 5 added to CAN hw filter to allow incoming msg
#define  ENG_MAN_CANID_HW_FILT6  	13        // Eng_manifold: CANID 6 added to CAN hw filter to allow incoming msg
#define  ENG_MAN_CANID_HW_FILT7  	14        // Eng_manifold: CANID 7 added to CAN hw filter to allow incoming msg
#define  ENG_MAN_CANID_HW_FILT8  	15        // Eng_manifold: CANID 8 added to CAN hw filter to allow incoming msg

#define PARAM_LIST_CT_ENG_MANIFOLD	15	// Count of same FUNCTION_TYPE in preceding list

#define  ENG_RPM_CRC             	1         // Eng_rpm: CRC                                    
#define  ENG_RPM_VERSION         	2         // Eng_rpm: Version number                         
#define  ENG_RPM_HEARTBEAT_TIME_CT	3         // Eng_rpm: Time (ms) between HB msg               
#define  ENG_RPM_SEG_CT          	4         // Eng_rpm: Counts per revolution on engine        
#define  ENG_RPM_POLL_RPM_R      	5         // Eng_rpm: CANID: Poll response, rpm, calibrated  
#define  ENG_RPM_HB_RPM_R        	6         // Eng_rpm: CANID: Heartbeat: rpm, calibrated      
#define  ENG_RPM_CANID_HW_FILT1  	7         // Eng_rpm: CANID 1 added to CAN hw filter to allow incoming msg
#define  ENG_RPM_CANID_HW_FILT2  	8         // Eng_rpm: CANID 2 added to CAN hw filter to allow incoming msg
#define  ENG_RPM_CANID_HW_FILT3  	9         // Eng_rpm: CANID 3 added to CAN hw filter to allow incoming msg
#define  ENG_RPM_CANID_HW_FILT4  	10        // Eng_rpm: CANID 4 added to CAN hw filter to allow incoming msg
#define  ENG_RPM_CANID_HW_FILT5  	11        // Eng_rpm: CANID 5 added to CAN hw filter to allow incoming msg
#define  ENG_RPM_CANID_HW_FILT6  	12        // Eng_rpm: CANID 6 added to CAN hw filter to allow incoming msg
#define  ENG_RPM_CANID_HW_FILT7  	13        // Eng_rpm: CANID 7 added to CAN hw filter to allow incoming msg
#define  ENG_RPM_CANID_HW_FILT8  	14        // Eng_rpm: CANID 8 added to CAN hw filter to allow incoming msg

#define PARAM_LIST_CT_ENG_RPM	14	// Count of same FUNCTION_TYPE in preceding list

#define  ENG_THR_CRC             	1         // Eng_throttle: CRC                               
#define  ENG_THR_VERSION         	2         // Eng_throttle: Version number                    
#define  ENG_THR_HEARTBEAT_TIME_CT	3         // Eng_throttle: Time (ms) between HB msg          
#define  ENG_THR_OPENFULL        	4         // Eng_throttle: ADC when throttle full open       
#define  ENG_THR_CLOSED          	5         // Eng_throttle: ADC when throttle closed          
#define  ENG_THR_POLL_THROTTLE_R 	6         // Eng_throttle: CANID: Poll response: throttle (0.0-100.0)
#define  ENG_THR_HB_THROTTLE_R   	7         // Eng_throttle: CANID: Heartbeat: throttle (0.0-100.0)
#define  ENG_THR_CANID_HW_FILT1  	8         // Eng_throttle: CANID 1 added to CAN hw filter to allow incoming msg
#define  ENG_THR_CANID_HW_FILT2  	9         // Eng_throttle: CANID 2 added to CAN hw filter to allow incoming msg
#define  ENG_THR_CANID_HW_FILT3  	10        // Eng_throttle: CANID 3 added to CAN hw filter to allow incoming msg
#define  ENG_THR_CANID_HW_FILT4  	11        // Eng_throttle: CANID 4 added to CAN hw filter to allow incoming msg
#define  ENG_THR_CANID_HW_FILT5  	12        // Eng_throttle: CANID 5 added to CAN hw filter to allow incoming msg
#define  ENG_THR_CANID_HW_FILT6  	13        // Eng_throttle: CANID 6 added to CAN hw filter to allow incoming msg
#define  ENG_THR_CANID_HW_FILT7  	14        // Eng_throttle: CANID 7 added to CAN hw filter to allow incoming msg
#define  ENG_THR_CANID_HW_FILT8  	15        // Eng_throttle: CANID 8 added to CAN hw filter to allow incoming msg

#define PARAM_LIST_CT_ENG_THROTTLE	15	// Count of same FUNCTION_TYPE in preceding list

#define  ENG_T1_CRC              	1         // Eng_t1: CRC                                     
#define  ENG_T1_VERSION          	2         // Eng_t1: Version number                          
#define  ENG_T1_HEARTBEAT_TIME_CT	3         // Eng_t1: Time (ms) between HB msg                
#define  ENG_T1_CONST_B          	4         // Eng_t1: Thermistor param: constant B            
#define  ENG_T1_R_SERIES         	5         // Eng_t1: Thermistor param: Series resistor, fixed (K ohms)
#define  ENG_T1_R_ROOMTMP        	6         // Eng_t1: Thermistor param: Thermistor room temp resistance (K ohms)
#define  ENG_T1_REF_TEMP         	7         // Eng_t1: Thermistor param: Reference temp for thermistor
#define  ENG_T1_TEMP_OFFSET      	8         // Eng_t1: Thermistor param: Thermistor temp offset correction (deg C)
#define  ENG_T1_TEMP_SCALE       	9         // Eng_t1: Thermistor param: Thermistor temp scale correction
#define  ENG_T1_POLL_T1_R        	10        // Eng_t1: CANID: Poll response: temperature #1 (deg C)
#define  ENG_T1_HB_T1_R          	11        // Eng_t1: CANID: Heartbeat: temperature #1 (deg C)
#define  ENG_T1_CANID_HW_FILT1   	12        // Eng_t1: CANID 1 added to CAN hw filter to allow incoming msg
#define  ENG_T1_CANID_HW_FILT2   	13        // Eng_t1: CANID 2 added to CAN hw filter to allow incoming msg
#define  ENG_T1_CANID_HW_FILT3   	14        // Eng_t1: CANID 3 added to CAN hw filter to allow incoming msg
#define  ENG_T1_CANID_HW_FILT4   	15        // Eng_t1: CANID 4 added to CAN hw filter to allow incoming msg
#define  ENG_T1_CANID_HW_FILT5   	16        // Eng_t1: CANID 5 added to CAN hw filter to allow incoming msg
#define  ENG_T1_CANID_HW_FILT6   	17        // Eng_t1: CANID 6 added to CAN hw filter to allow incoming msg
#define  ENG_T1_CANID_HW_FILT7   	18        // Eng_t1: CANID 7 added to CAN hw filter to allow incoming msg
#define  ENG_T1_CANID_HW_FILT8   	19        // Eng_t1: CANID 8 added to CAN hw filter to allow incoming msg

#define PARAM_LIST_CT_ENG_T1	19	// Count of same FUNCTION_TYPE in preceding list

#define  ENGINE__CRC             	1         // Engine_sensor: CRC                              
#define  ENGINE__VERSION         	2         // Engine_sensor: Version number                   
#define  ENGINE__HEARTBEAT_TIME_CT	3         // Engine_sensor: Time (ms) between HB msg         
#define  ENGINE_SENSOR_SEG_CT    	4         // Engine_sensor: Counts per revolution on engine  
#define  ENGINE_SENSOR_PRESS_OFFSET	5         // Engine_sensor: Manifold pressure offset         
#define  ENGINE_SENSOR_PRESS_SCALE	6         // Engine_sensor: Manifold pressure  scale (inch Hg)
#define  ENGINE_THERM1_CONST_B   	7         // Engine_sensor: Thermistor param: constant B     
#define  ENGINE_THERM1_R_SERIES  	8         // Engine_sensor: Thermistor param: Series resistor, fixed (K ohms)
#define  ENGINE_THERM1_R_ROOMTMP 	9         // Engine_sensor: Thermistor param: Thermistor room temp resistance (K ohms)
#define  ENGINE_THERM1_REF_TEMP  	10        // Engine_sensor: Thermistor param: Reference temp for thermistor
#define  ENGINE_THERM1_TEMP_OFFSET	11        // Engine_sensor: Thermistor param: Thermistor temp offset correction (deg C)
#define  ENGINE_THERM1_TEMP_SCALE	12        // Engine_sensor: Thermistor param: Thermistor temp scale correction
#define  ENGINE_THROTTLE_lO      	13        // Engine_sensor: Throttle pot ADC reading: closed 
#define  ENGINE_THROTTLE_HI      	14        // Engine_sensor: Throttle pot ADC reading: full open
#define  ENGINE_POLL_RPMMANFLD_R 	15        // Engine_sensor: CANID: Poll response: rpm:manifold, calibrated
#define  ENGINE_POLL_TEMPTHRTL_R 	16        // Engine_sensor: CANID: Poll response:temperature:throttle pot, calibrated
#define  ENGINE_HB_RPMMANFLD     	17        // Engine_sensor: CANID: Heartbeat: raw readings: rpm:manifold pressure
#define  ENGINE_HB_THRMTHRTL     	18        // Engine_sensor: CANID: Heartbeat: raw readings: thermistor:throttle
#define  ENGINE_CANID_HW_FILT1   	19        // Engine_sensor: CANID 1 added to CAN hw filter to allow incoming msg
#define  ENGINE_CANID_HW_FILT2   	20        // Engine_sensor: CANID 2 added to CAN hw filter to allow incoming msg
#define  ENGINE_CANID_HW_FILT3   	21        // Engine_sensor: CANID 3 added to CAN hw filter to allow incoming msg
#define  ENGINE_CANID_HW_FILT4   	22        // Engine_sensor: CANID 4 added to CAN hw filter to allow incoming msg
#define  ENGINE_CANID_HW_FILT5   	23        // Engine_sensor: CANID 5 added to CAN hw filter to allow incoming msg
#define  ENGINE_CANID_HW_FILT6   	24        // Engine_sensor: CANID 6 added to CAN hw filter to allow incoming msg
#define  ENGINE_CANID_HW_FILT7   	25        // Engine_sensor: CANID 7 added to CAN hw filter to allow incoming msg
#define  ENGINE_CANID_HW_FILT8   	26        // Engine_sensor: CANID 8 added to CAN hw filter to allow incoming msg

#define PARAM_LIST_CT_ENGINE_SENSOR	26	// Count of same FUNCTION_TYPE in preceding list

#define  YOGURT_1_LIST_CRC       	1         // Yogurt: crc: CRC for this list                  
#define  YOGURT_1_LIST_VERSION   	2         // Yogurt: version: Version number yogurt          
#define  YOGURT_1_THERM1_CONST_B 	3         // Yogurt: Thermistor1 param: B: constant B        
#define  YOGURT_1_THERM1_R_SERIES	4         // Yogurt: Thermistor1 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM1_R_ROOMTMP	5         // Yogurt: Thermistor1 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM1_REF_TEMP	6         // Yogurt: Thermistor1 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM1_COEF_0  	7         // Yogurt: Thermistor1 param: poly[0]:  poly coeff 0 (offset)
#define  YOGURT_1_THERM1_COEF_1  	8         // Yogurt: Thermistor1 param: poly[1]:  poly coeff 1 (scale)
#define  YOGURT_1_THERM1_COEF_2  	9         // Yogurt: Thermistor1 param: poly[2]:  poly coeff 2 (x^2)
#define  YOGURT_1_THERM1_COEF_3  	10        // Yogurt: Thermistor1 param: poly[3]:  poly coeff 3 (x^3)
#define  YOGURT_1_THERM2_CONST_B 	11        // Yogurt: Thermistor2 param: B: constant B        
#define  YOGURT_1_THERM2_R_SERIES	12        // Yogurt: Thermistor2 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM2_R_ROOMTMP	13        // Yogurt: Thermistor2 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM2_REF_TEMP	14        // Yogurt: Thermistor2 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM2_COEF_0  	15        // Yogurt: Thermistor2 param: poly[0]:  poly coeff 0 (offset)
#define  YOGURT_1_THERM2_COEF_1  	16        // Yogurt: Thermistor2 param: poly[1]:  poly coeff 1 (scale)
#define  YOGURT_1_THERM2_COEF_2  	17        // Yogurt: Thermistor2 param: poly[2]:  poly coeff 2 (x^2)
#define  YOGURT_1_THERM2_COEF_3  	18        // Yogurt: Thermistor2 param: poly[3]:  poly coeff 3 (x^3)
#define  YOGURT_1_THERM3_CONST_B 	19        // Yogurt: Thermistor3 param: B: constant B        
#define  YOGURT_1_THERM3_R_SERIES	20        // Yogurt: Thermistor3 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM3_R_ROOMTMP	21        // Yogurt: Thermistor3 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM3_REF_TEMP	22        // Yogurt: Thermistor3 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM3_COEF_0  	23        // Yogurt: Thermistor3 param: poly[0]:  poly coeff 0 (offset)
#define  YOGURT_1_THERM3_COEF_1  	24        // Yogurt: Thermistor3 param: poly[1]:  poly coeff 1 (scale)
#define  YOGURT_1_THERM3_COEF_2  	25        // Yogurt: Thermistor3 param: poly[2]:  poly coeff 2 (x^2)
#define  YOGURT_1_THERM3_COEF_3  	26        // Yogurt: Thermistor3 param: poly[3]:  poly coeff 3 (x^3)
#define  YOGURT_1_THERM4_CONST_B 	27        // Yogurt: Thermistor4 param: B: constant B        
#define  YOGURT_1_THERM4_R_SERIES	28        // Yogurt: Thermistor4 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM4_R_ROOMTMP	29        // Yogurt: Thermistor4 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM4_REF_TEMP	30        // Yogurt: Thermistor4 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM4_COEF_0  	31        // Yogurt: Thermistor4 param: poly[0]:  poly coeff 0 (offset)
#define  YOGURT_1_THERM4_COEF_1  	32        // Yogurt: Thermistor4 param: poly[1]:  poly coeff 1 (scale)
#define  YOGURT_1_THERM4_COEF_2  	33        // Yogurt: Thermistor4 param: poly[2]:  poly coeff 2 (x^2)
#define  YOGURT_1_THERM4_COEF_3  	34        // Yogurt: Thermistor4 param: poly[3]:  poly coeff 3 (x^3)
#define  YOGURT_1_CTLTEMP_HEAT_PAST	35        // Yogurt: Pasteur: Control set-point temperature (deg F) heat to this temp
#define  YOGURT_1_CTLTEMP_DUR_PAST	36        // Yogurt: Pasteur: Time duration at temp (hours.frac_hours)
#define  YOGURT_1_CTLTEMP_COOL_PAST	37        // Yogurt: Pasteur: Control end-point temperature (deg F) cool to this temp
#define  YOGURT_1_CTLTEMP_HEAT_FERM	38        // Yogurt: Ferment: Control set-point temperature (deg F) heat to this temp
#define  YOGURT_1_CTLTEMP_DUR_FERM	39        // Yogurt: Ferment: Time duration at temp (hours.frac_hours)
#define  YOGURT_1_CTLTEMP_COOL_FERM	40        // Yogurt: Ferment: Control end-point temperature (deg F) cool to this temp
#define  YOGURT_1_CTL_THERM_SHELL	41        // Yogurt: Thermistor number for shell temp (0 - 3)
#define  YOGURT_1_CTL_THERM_POT  	42        // Yogurt: Thermistor number for center of pot temp (0 - 3)
#define  YOGURT_1_CTL_THERM_AIRIN	43        // Yogurt: Thermistor number for air inlet to fan temp (0 - 3)
#define  YOGURT_1_CTL_THERM_AIROUT	44        // Yogurt: Thermistor number for air coming out of holes (0 - 3)
#define  YOGURT_1_CTL_THERM_LOOP_P	45        // Yogurt: Control loop: Proportional coeff        
#define  YOGURT_1_CTL_THERM_LOOP_I	46        // Yogurt: Control loop: Integral coeff            
#define  YOGURT_1_CTL_THERM_LOOP_D	47        // Yogurt: Control loop: Derivative coeff          
#define  YOGURT_1_CMD            	48        // Yogurt: CANID: cid_yog_cmd: Yogurt maker parameters
#define  YOGURT_1_MSG            	49        // Yogurt: CANID: cid_yog_msg: Yogurt maker msgs   
#define  YOGURT_1_HB             	50        // Yogurt: CANID: cid_yog_hb: Yogurt maker heart-beats
#define  YOGURT_1_HEATCONSTANT_KM_P	51        // Yogurt: Control, stored heat constant Pasteur phase
#define  YOGURT_1_HEATCONSTANT_KM_M	52        // Yogurt: Control, stored heat constant Ferment phase
#define  YOGURT_1_INTEGRATEINIT_A	53        // Yogurt: Control, integrator initialization, a of  a + b*x 
#define  YOGURT_1_INTEGRATEINIT_B	54        // Yogurt: Control, integrator initialization, b of  a + b*x 
#define  YOGURT_1_STABILIZETIMEDELAY_P	55        // Yogurt: Control, time delay for temperature stabilization, Pasteur
#define  YOGURT_1_STABILIZETIMEDELAY_F	56        // Yogurt: Control, time delay for temperature stabilization, Ferment

#define PARAM_LIST_CT_YOGURT_1	56	// Count of same FUNCTION_TYPE in preceding list

#define  MCL_LIST_CRC            	1         // mcl: crc: CRC: Master Controller Launch parameters
#define  MCL_LIST_VERSION        	2         // mcl: version: Master Controller Launch parameters
#define  MCL_GROUND_TENSION_FACTOR	3         // mcl: ground tension factor: Master Controller Launch parameters 
#define  MCL_CLIMB_TENSION_FACTOR	4         // mcl: climb tension factor: Master Controller Launch parameters 
#define  MCL_GLIDER_MASS         	5         // mcl: glider mass: Master Controller Launch parameters 
#define  MCL_GLIDER_WEIGHT       	6         // mcl: glider weight: Master Controller Launch parameters 
#define  MCL_SOFT_START_TIME     	7         // mcl: soft start time: Master Controller Launch parameters 
#define  MCL_K1                  	8         // mcl: soft start constant: k1: Master Controller Launch parameters 
#define  MCL_PROFILE_TRIG_CABLE_SPEED	9         // mcl: rotation taper: cable trigger speed: Master Controller Launch parameterS 
#define  MCL_MAX_GROUND_CABLE_SPEED	10        // mcl: rotation taper: max ground cable speed: Master Controller Launch parameterS 
#define  MCL_K2                  	11        // mcl: rotation taper: constant k2: Master Controller Launch parameterS 
#define  MCL_PEAK_CABLE_SPEED_DROP	12        // mcl: transition to ramp: peak cable_speed_drop: Master Controller Launch parameterS 
#define  MCL_RAMP_TIME           	13        // mcl: ramp taper up: ramp time: Master Controller Launch parameterS 
#define  MCL_K3                  	14        // mcl: ramp taper up: constant k3: Master Controller Launch parameterS 
#define  MCL_TAPERANGLETRIG      	15        // mcl: end of climb taper down: taper angle trig: Master Controller Launch parameterS 
#define  MCL_TAPERTIME           	16        // mcl: end of climb taper down: taper time: Master Controller Launch parameterS 
#define  MCL_K4                  	17        // mcl: end of climb taper down: constant k4: Master Controller Launch parameterS 
#define  MCL_RELEASEDELTA        	18        // mcl: end of climb taper down: release delta: Master Controller Launch parameterS 
#define  MCL_MAX_PARACHUTE_TENSION	19        // mcl: parachute tension taper: max parachute tension: MC Launch parameterS
#define  MCL_PARACHUTE_TAPER_SPEED	20        // mcl: parachute tension taper: taper speed: Master Controller Launch parameterS
#define  MCL_MAX_CHUTE_CABLE_SPEED	21        // mcl: parachute tension taper: max hute cable speed: Master Controller Launch parameterS
#define  MCL_K5                  	22        // mcl: parachute tension taper: constant k5: Master Controller Launch parameterS

#define PARAM_LIST_CT_MCL	22	// Count of same FUNCTION_TYPE in preceding list


#define READINGS_LIST_COUNT 27
#define  TENSION_READ_FILTADC_THERM1	1         // Tension: READING: double thrm[0]; Filtered ADC for Thermistor on AD7799
#define  TENSION_READ_FILTADC_THERM2	2         // Tension: READING: double thrm[1]; Filtered ADC for Thermistor external
#define  TENSION_READ_FORMCAL_THERM1	4         // Tension: READING: double degX[0]; Formula computed thrm for Thermistor on AD7799
#define  TENSION_READ_FORMCAL_THERM2	5         // Tension: READING: double degX[1]; Formula computed thrm for Thermistor external
#define  TENSION_READ_POLYCAL_THERM1	6         // Tension: READING: double degC[0]; Polynomial adjusted degX for Thermistor on AD7799
#define  TENSION_READ_POLYCAL_THERM2	7         // Tension: READING: double degC[1]; Polynomial adjusted degX for Thermistor external
#define  TENSION_READ_AD7799_LGR 	8         // Tension: READING: int32_t lgr; last_good_reading (no filtering or adjustments)
#define  TENSION_READ_AD7799_CALIB_1	9         // Tension: READING: ten_iircal[0];  AD7799 filtered (fast) and calibrated
#define  TENSION_READ_AD7799_CALIB_2	10        // Tension: READING: ten_iircal[1];  AD7799 filtered (slow) and calibrated
#define  TENSION_READ_CIC_RAW    	11        // Tension: READING: int32_t cicraw; cic before averaging
#define  TENSION_READ_CIC_AVE    	12        // Tension: READING: int32_t cicave; cic averaged for determining offset
#define  TENSION_READ_CIC_AVE_CT 	13        // Tension: READING: int32_t ave.n;  current count for above average
#define  TENSION_READ_OFFSET_REG 	14        // Tension: READING: Last reading of AD7799 offset register
#define  TENSION_READ_OFFSET_REG_FILT	15        // Tension: READING: Last filtered AD7799 offset register
#define  TENSION_READ_OFFSET_REG_RDBK	16        // Tension: READING: Last filtered AD7799 offset register set read-back
#define  TENSION_READ_FULLSCALE_REG	17        // Tension: READING: Last reading of AD7799 fullscale register
#define  TENSION_READ_POLL_MASK  	18        // Tension: READING: Mask for first two bytes of a poll msg (necessary?)
#define  TENSION_READ_READINGSCT 	19        // Tension: READING: Running count of readings (conversions completed)
#define  TENSION_READ_READINGSCT_LASTPOLL	20        // Tension: READING: Reading count the last time a poll msg sent
#define  TENSION_READ_OFFSET_CT  	21        // Tension: READING: Running ct of offset updates  
#define  TENSION_READ_ZERO_FLAG  	22        // Tension: READING: 1 = zero-calibration operation competed
#define  TENSION_READ_STATUS_BYTE	23        // Tension: READING: Reading status byte           
#define  TENSION_READ_IIR_OFFSET_K	24        // Tension: READING: IIR filter for offsets: parameter for setting time constant
#define  TENSION_READ_IIR_OFFSET_SCL	25        // Tension: READING: IIR filter for offsets: Scaling to improve spare bits with integer math
#define  TENSION_READ_IIR_Z_RECAl_W_K	26        // Tension: READING: IIR filter for zeroing: parameter for setting time constant
#define  TENSION_READ_IIR_Z_RECAl_W_SCL	27        // Tension: READING: IIR filter for zeroing: Scaling to improve spare bits with integer math
#define  ENCODER_READ_INPUTCAPTURE_CT	1         // Encoder: READING: TIM3 input capture count      

#define FUNC_BIT_PARAM_COUNT 28
#define  POLL_FUNC_BIT_TENSION   	0x1       // TENSION             Function bit: 2nd byte of poll msg: TENSION     
#define  POLL_FUNC_BIT_CABLE_ANGLE	0x2       // CABLE_ANGLE         Function bit: 2nd byte of poll msg: CABLE_ANGLE 
#define  POLL_FUNC_BIT_SHAFT_ODO_SPD	0x4       // SHAFT_ENCODER       Function bit: 2nd byte of poll msg: shaft odometer & speed
#define  POLL_FUNC_BIT_TILT      	0x8       // TILT_SENSE          Function bit: 2nd byte of poll msg: TILT        
#define  POLL_DO_NOT             	0x1       // DUMMY               Selection bit: No reply sent to poll msg        
#define  POLL_DRUM_BIT_2         	0x2       // SELECT_DRUM_1       Selection bit: 1st byte of poll msg Drum #1     
#define  POLL_DRUM_BIT_3         	0x4       // SELECT_DRUM_2       Selection bit: 1st byte of poll msg Drum #2     
#define  POLL_DRUM_BIT_4         	0x8       // SELECT_DRUM_3       Selection bit: 1st byte of poll msg Drum #3     
#define  POLL_DRUM_BIT_5         	0x10      // SELECT_DRUM_4       Selection bit: 1st byte of poll msg Drum #4     
#define  POLL_DRUM_BIT_6         	0x20      // SELECT_DRUM_5       Selection bit: 1st byte of poll msg Drum #5     
#define  POLL_DRUM_BIT_7         	0x40      // SELECT_DRUM_6       Selection bit: 1st byte of poll msg Drum #6     
#define  POLL_DRUM_BIT_8         	0x80      // SELECT_DRUM_7       Selection bit: 1st byte of poll msg Drum #7     
#define  STATUS_TENSION_BIT_NONEW	0x1       // TENSION             status: No new reading since last poll msg sent 
#define  STATUS_TENSION_BIT_EXCEEDHI	0x2       // TENSION             status: Reading limit hi exceed (open (-) connection?)
#define  STATUS_TENSION_BIT_EXCEEDLO	0x4       // TENSION             status: Reading limit lo exceed (open (+ or both) connection?)
#define  STATUS_TENSION_BIT_4    	0x8       // TENSION             status: spare 0x8                               
#define  STATUS_TENSION_BIT_5    	0x10      // TENSION             status: spare 0x10                              
#define  STATUS_TENSION_BIT_6    	0x20      // TENSION             status: spare 0x20                              
#define  STATUS_TENSION_BIT_7    	0x40      // TENSION             status: spare 0x40                              
#define  STATUS_TENSION_BIT_DONOTUSE	0x80      // TENSION             status: spare 0x80                              
#define  USEME_TENSION_BIT_AD7799_1	0x1       // TENSION             useme: 1st AD7799                               
#define  USEME_TENSION_BIT_AD7799_2	0x2       // TENSION             useme: 2nd AD7799                               
#define  USEME_TENSION_BIT_3     	0x4       // TENSION             useme: spare 0x4                                
#define  USEME_TENSION_BIT_4     	0x8       // TENSION             useme: spare 0x8                                
#define  USEME_TENSION_BIT_5     	0x10      // TENSION             useme: spare 0x10                               
#define  USEME_TENSION_BIT_6     	0x20      // TENSION             useme: spare 0x20                               
#define  USEME_TENSION_BIT_7     	0x40      // TENSION             useme: spare 0x40                               
#define  USEME_TENSION_BIT_8     	0x80      // TENSION             useme: spare 0x80                               

#define FUNCTION_TYPE_COUNT 23
#define  FUNCTION_TYPE_SHAFT_ENCODER           	1         // Sensor, shaft: Drive shaft encoder              
#define  FUNCTION_TYPE_ENGINE_SENSOR           	2         // Sensor, engine: rpm, manifold pressure, throttle setting, temperature
#define  FUNCTION_TYPE_TENSION_a               	3         // Tension_a: Tension AD7799 #1                    
#define  FUNCTION_TYPE_TENSION_a2              	4         // Tension_a: Tension AD7799 #2                    
#define  FUNCTION_TYPE_CABLE_ANGLE             	5         // Cable angle AD7799                              
#define  FUNCTION_TYPE_TENSION_c               	6         // Tension_c: Tension op-amp                       
#define  FUNCTION_TYPE_TIMESYNC                	7         // GPS time sync distribution msg                  
#define  FUNCTION_TYPE_HC_SANDBOX_1            	8         // Host Controller: sandbox function 1             
#define  FUNCTION_TYPE_MC                      	9         // Master Controller                               
#define  FUNCTION_TYPE_TILT_SENSE              	10        // Tilt sensor                                     
#define  FUNCTION_TYPE_YOGURT_1                	11        // Yogurt_1: Ver 1 of maker                        
#define  FUNCTION_TYPE_GPS                     	12        // GPS                                             
#define  FUNCTION_TYPE_LOGGER                  	13        // Logger                                          
#define  FUNCTION_TYPE_CANSENDER               	14        // Cansender                                       
#define  FUNCTION_TYPE_SHEAVE_UP_H             	15        // Uppersheave (TIM2 encoder)                      
#define  FUNCTION_TYPE_SHEAVE_LO_H             	16        // Lowersheave (TIM5 encoder)                      
#define  FUNCTION_TYPE_ENG_MANIFOLD            	17        // Sensor, engine: manifold pressure               
#define  FUNCTION_TYPE_ENG_RPM                 	18        // Sensor, engine: rpm                             
#define  FUNCTION_TYPE_ENG_THROTTLE            	19        // Sensor, engine: throttle                        
#define  FUNCTION_TYPE_ENG_T1                  	20        // Sensor, engine: temperature 1                   
#define  FUNCTION_TYPE_ENG_T2                  	21        // Sensor, engine: temperature 2                   
#define  FUNCTION_TYPE_DRIVE_SHAFT             	22        // Sensor, shaft: ../sensor/shaft/trunk version    
#define  FUNCTION_TYPE_PWRBOX                  	23        // Pwrbox, input power voltage monitor             

#define READINGS_BOARD_COUNT 15
#define  PROG_TENSION_READINGS_BOARD_NUM_AD7799	1         // Number of AD7799 that successfully initialized  
#define  PROG_TENSION_READINGS_BOARD_CAN_TXERR	2         // Count: total number of msgs returning a TERR flags (including retries)
#define  PROG_TENSION_READINGS_BOARD_CAN_TX_BOMBED	3         // Count: number of times msgs failed due to too many TXERR
#define  PROG_TENSION_READINGS_BOARD_CAN_ALST0_ERR	4         // Count: arbitration failure total                
#define  PROG_TENSION_READINGS_BOARD_CAN_ALST0_NART_ERR	5         // Count: arbitration failure when NART is on      
#define  PROG_TENSION_READINGS_BOARD_CAN_MSGOVRFLO	6         // Count: Buffer overflow when adding a msg        
#define  PROG_TENSION_READINGS_BOARD_CAN_SPURIOUS_INT	7         // Count: TSR had no RQCPx bits on (spurious interrupt)
#define  PROG_TENSION_READINGS_BOARD_CAN_NO_FLAGGED	8         // Count:                                          
#define  PROG_TENSION_READINGS_BOARD_CAN_PFOR_BK_ONE	9         // Count: Instances that pfor was adjusted in TX interrupt
#define  PROG_TENSION_READINGS_BOARD_CAN_PXPRV_FWD_ONE	10        // Count: Instances that pxprv was adjusted in for loop
#define  PROG_TENSION_READINGS_BOARD_CAN_RX0ERR_CT	11        // Count: FIFO 0 overrun                           
#define  PROG_TENSION_READINGS_BOARD_CAN_RX1ERR_CT	12        // Count: FIFO 1 overrun                           
#define  PROG_TENSION_READINGS_BOARD_CAN_CP1CP2	13        // Count: (RQCP1 | RQCP2) unexpectedly ON          
#define  PROG_TENSION_READINGS_BOARD_TXINT_EMPTYLIST	14        // Count: TX interrupt with pending list empty     
#define  PROG_TENSION_READINGS_BOARD_CAN1_BOGUS_CT	15        // Count: bogus CAN1 IDs rejected                  

/* TOTAL COUNT OF #defines = 743  */
/* Test 2016/06/12 */

