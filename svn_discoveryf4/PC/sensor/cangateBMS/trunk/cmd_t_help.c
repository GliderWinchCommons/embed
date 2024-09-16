/******************************************************************************
* File Name          : cmd_t_help.c
* Date First Issued  : 07/03/2024
* Board              : PC
* Description        : Help text for ta command
*******************************************************************************/
#ifndef __CMD_TA_HELP
#define __CMD_TA_HELP

char* cmd_t_help = {
" ===============================================\n"	
"Module Columns displaying selected status bits\n"
"(red = on; green = off)\n"
"   DH2CL (FET status)\n"
 "D - DUMP   FET\n"
 "H - HEATER FET\n"
 "2 - DUMP2  FET\n"
 "C - Low current charger\n"
 "L - Low current charging at low cell voltage level\n"
"\n"
"   ZX+-BCD (Battery status)\n"
"Z - One or more cells: Reading exactly zero\n"
"X - One or more cells: Negative or over 5v indicative of open wire\n"
"+ - One or more cells: Above max limit\n"
"- - One or more cells: Below min limit\n"
"B - Cell balancing in progress\n"
"C - Charging in progress\n"
"D - Dump is in progress\n"
"\n"
"  STB (Mode status)\n"
"S - Self-discharge Mode\n"
"T - Cell has tripped\n"
"B - Cells below max but tripped\n"
"\n"
"Module Columns displaying not-obvious numbers\n"
"total  - sum of installed cell readings (volts)\n"
"ave    - average (mv)\n"
"max    - maximum voltage cell (mv)\n"
"max_at - cell number of max cell\n"
"min    - minimum voltage cell (mv)\n"
"min_at - cell number of minimum cell\n"
"std    - standard deviation of cells\n"
"Tamb, Tcell, Texit - Thermistor calibrated temperatures (deg C)\n"
"I      - BMS current sense w calibrated current (amps)\n"
"BAT    - Battery status byte (hex)\n"
"FET    - FET status byte (hex)\n"
"MOD    - Mode status byte (hex)\n"
"fan    - Fan speed 0 - 100% (pwm)\n"
"fanrpm - Fan speed (rpm)\n"
};
#endif
