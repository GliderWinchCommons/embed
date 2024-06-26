/******************************************************************************
* File Name          : unit_cmd_canid.c
* Date First Issued  : 05/12/2016
* Board              : f103
* Description        : Load command id table into high flash for a CAN unit
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/gen_db.h"
#include "../../../../../svn_common/trunk/common_highflash.h"

/* .ld file defines where ".appparam0a" is located */
__attribute__ ((section(".appparam0a"))) // Place in FLASHP0a location
#include "db/idx_v_val.c"	// The source code file with all the functions

