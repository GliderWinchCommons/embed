/******************************************************************************
* File Name          : pwrbox_param.c
* Date First Issued  : 06/18/2018
* Board              : F103 Arduino
* Description        : Manually load parmeter table into high flash
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/gen_db.h"
#include "../pwrbox_idx_v_struct.h"

/* .ld file defines where ".appparam1" is located */
__attribute__ ((section(".appparam1"))) // Place in flash location
#include "db/idx_v_val.c"

