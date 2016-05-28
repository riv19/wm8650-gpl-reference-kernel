/*--- wmt-vdlib.c ---------------------------------------------------------------
*   Copyright (C) 2006 WonderMedia Tech. Inc.
*
* MODULE       : wmt-vdlib.c
* AUTHOR       : Jason Lin
* DATE         : 2008/12/08
* DESCRIPTION  : 
*-----------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Jason Lin, 2008/12/08
*	First version
*
*------------------------------------------------------------------------------*/

#include <linux/dma-mapping.h>    /* For dma_alloc_coherent() only */

#include <mach/hardware.h>
#include "wmt-vdlib.h"

#define LIB_NAME        "wmt-vdlib"
#define LIB_VERSION     1	// 0.00.01

// #define DEBUG

#define INFO(fmt,args...) printk(KERN_INFO "[" DRIVER_NAME "] " fmt , ## args)
//#define WARN(fmt,args...) printk(KERN_WARNING "[" DRIVER_NAME "] " fmt , ## args)
#define ERROR(fmt,args...) printk(KERN_ERR "[" DRIVER_NAME "] " fmt , ## args)

#ifdef DEBUG
#define DBG(fmt, args...) printk(KERN_DEBUG "[" DRIVER_NAME "] %s: " fmt, __FUNCTION__ , ## args)
#else
#define DBG(fmt, args...)
#endif

#define DSP_PRD_LEN    512

static int dsp_ini_cnt = 0;
static char *dsp_virt = NULL;
static dma_addr_t  dsp_phy;


/*!*************************************************************************
* Private Function by Welkin Chen, 2010/11/01
*
* \brief 
*   init memory buffer for dsp internal
*/
int vdlib_prd_init(void)
{
    int retval = 0;

    if (dsp_ini_cnt == 0){
        DBG("allocate DSP prd buffer address\n");
        dsp_virt = (char *)dma_alloc_coherent(NULL, DSP_PRD_LEN, &dsp_phy, GFP_KERNEL);
        if (dsp_virt == NULL){
            printk("*E* Allocate DSP prd buffer fail. size(%d)\n", DSP_PRD_LEN);
            retval = -1;
        }
        DBG("Reset DSP prd\n");
        memset(dsp_virt, 0x0, DSP_PRD_LEN);
        dsp_ini_cnt = 1;
    }
    else { // more_ini_cnt
        DBG("Reset DSP prd\n");
        memset(dsp_virt, 0x0, DSP_PRD_LEN);
    }
    return retval;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2010/11/01
*
* \brief   
*   return virtual address for dsp internal
*/
unsigned int vdlib_prd_virt(void)
{
    return (unsigned int)dsp_virt;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2010/11/01
*
* \brief   
*   return physical address for dsp internal
*/
unsigned int vdlib_prd_phy(void)
{
    return (unsigned int)dsp_phy;
}