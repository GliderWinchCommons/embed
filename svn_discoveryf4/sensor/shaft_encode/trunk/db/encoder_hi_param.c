/******************************************************************************
* File Name          : encoder_hi_param.c
* Date First Issued  : 08/10/2017
* Board              : f4
* Description        : Manually load parmeter table into high flash
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "gen_db.h"
#include "encoder_idx_v_struct.h"

#define SHEAVE_1UP
/* .ld file defines where ".appparam1" is located */
__attribute__ ((section(".appparam1"))) // Place in flash location
#include "idx_v_val.c"

