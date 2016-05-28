/*--- wmt-vdlib.h---------------------------------------------------------------
*   Copyright (C) 2008 WonderMedia Tech. Inc.
*
* MODULE       : wmt-vdlib.h -- 
* AUTHOR       : Jason Lin
* DATE         : 2008/12/08
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Jason Lin, 2008/12/08
*	First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include <linux/module.h>
#include <mach/memblock.h>  /* For MB driver only */


/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

int vdlib_prd_init(void);
unsigned int vdlib_prd_virt(void);
unsigned int vdlib_prd_phy(void);
