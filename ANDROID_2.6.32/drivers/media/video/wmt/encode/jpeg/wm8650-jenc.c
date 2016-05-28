/* PVCS version log
** $Log:  $
 */
#define WM8650_jenc_C

/*--- wm8650-jenc.c ---------------------------------------------------------------
*   Copyright (C) 2009 WonderMedia Tech. Inc.
*
* MODULE       : wm8650-jenc.c
* AUTHOR       : 
* DATE         : 
* DESCRIPTION  : 
*-----------------------------------------------------------------------------*/
#include "hw-jenc.h"
#include "../../wmt-dsplib.h"

/*!*************************************************************************
* wait_decode_finish
* 
* Private Function 
*/
/*!
* \brief
*	Wait HW decoder finish job
*
* \retval  none
*/ 
static int wait_decode_finish(jenc_drvinfo_t *drv)
{
    dsplib_mbox_t mbox;
    int ret = 0;
    dsp_done_t * done_packet;
    int * encode_size = NULL;    
    /*--------------------------------------------------------------------------
        Step 5: Send decoding info to DSP
    --------------------------------------------------------------------------*/
    mbox.cmd = CMD_D2A_VENC_DONE;
    
    DBG_MSG("Send Wait finsih CMD: \n");
    DBG_MSG("mbox.cmd: 0x%x\n", mbox.cmd);

    dsplib_cmd_recv(&mbox,CMD_D2A_VENC_DONE);

    DBG_MSG("Received DSP Msg: \n");
    DBG_MSG("mbox.cmd: 0x%x\n", mbox.cmd);
    DBG_MSG("mbox.w1:  0x%x\n", mbox.w1);
    DBG_MSG("mbox.w2:  0x%x\n", mbox.w2);
    DBG_MSG("mbox.w3:  0x%x\n", mbox.w3);
    done_packet = (dsp_done_t *)phys_to_virt(mbox.w1);
    encode_size = (int *)(done_packet + 1);
    drv->arg_out.mjpeg_size = *encode_size;
    
    drv->_status |= STA_DECODE_DONE;

    return ret;
} /* End of wait_decode_finish() */

/*!*************************************************************************
* dsp_cmd_handler
* 
* Private Function 
*/
/*!
* \brief
*	JPEG hardware send command
*
* \retval  0 if success
*/ 
static int dsp_cmd_handler(
    unsigned int cmd,
    unsigned int w1,
    unsigned int w2,
    unsigned int w3)
{
    dsplib_mbox_t mbox;

    mbox.cmd = cmd;
    mbox.w1  = w1;
    mbox.w2  = w2;
    mbox.w3  = w3; 
    
    DBG_MSG("mbox.cmd: 0x%x\n", mbox.cmd);
    DBG_MSG("mbox.w1:  0x%x\n", mbox.w1);
    DBG_MSG("mbox.w2:  0x%x\n", mbox.w2);
    DBG_MSG("mbox.w3:  0x%x\n", mbox.w3);
    
    dsplib_cmd_send(cmd,w1,w2,w3);

    return 0;
} /* End of dsp_cmd_handler() */

/*!*************************************************************************
* wmt_jenc_decode_proc
* 
* API Function
*/
/*!
* \brief
*	Start JPEG Encode 
*
* \retval  0 if success
*/ 
int wmt_jenc_decode_proc(jenc_drvinfo_t *drv)
{
    TRACE("Enter\n");
    memcpy((void*)drv->prd_virt,&drv->arg_in.src_Y_addr,sizeof(jenc_input_t) - 10*4);
                        
    dsp_cmd_handler(CMD_A2D_VENC, drv->prd_addr, 0, drv->prd_addr + 128); 
    
    TRACE("drv->arg_in.src_Y_addr 0x%x\n",*(unsigned int *)drv->prd_virt);
    TRACE("drv->arg_in.src_C_addr 0x%x\n",*((unsigned int *)drv->prd_virt + 1));
    TRACE("drv->arg_in.dst_addr 0x%x\n",*((unsigned int *)drv->prd_virt + 2));
    TRACE("drv->arg_in.pic_width %d\n",*((unsigned int *)drv->prd_virt + 3));
    TRACE("drv->arg_in.pic_height %d\n",*((unsigned int *)drv->prd_virt + 4));
    TRACE("drv->arg_in.buf_width %d\n",*((unsigned int *)drv->prd_virt + 5));
    TRACE("drv->arg_in.buf_height %d\n",*((unsigned int *)drv->prd_virt + 6));

    TRACE("Leave\n");

    return 0;
} /* End of wmt_jenc_decode_proc() */

/*!*************************************************************************
* wmt_jenc_decode_finish
* 
*/
/*!
* \brief
*	Wait JPEG hardware finished
*
* \retval  0 if success
*/ 
int wmt_jenc_decode_finish(jenc_drvinfo_t *drv)
{
    int   ret;
    
   /*==========================================================================
        In this function, we just wait for HW decoding finished.
    ==========================================================================*/
    /*--------------------------------------------------------------------------
        Step 1: Wait intrrupt (jenc_INTRQ)
    --------------------------------------------------------------------------*/
    ret =  wait_decode_finish(drv);

    return ret;
} /* End of wmt_jenc_decode_finish() */

/*!*************************************************************************
* wmt_jenc_decode_flush
* 
*/
/*!
* \brief
*	Cancel JPEG decoding process
*
* \retval  0 if success
*/ 
int wmt_jenc_decode_flush(jenc_drvinfo_t *drv)
{
    return 0;
} /* End of wmt_jenc_decode_flush() */


/*!*************************************************************************
* wmt_jenc_suspend
* 
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jenc_suspend(void)
{
    // TO DO
    return 0;
} /* End of wmt_jenc_suspend() */

/*!*************************************************************************
* wmt_jenc_resume
* 
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jenc_resume(void)
{
    // TO DO
    return 0;
} /* End of wmt_jenc_resume() */
/*!*************************************************************************
* wmt_jenc_set_drv
* 
*/
/*!
* \brief
*	Set current JPEG driver objet
*
* \retval  0 if success
*/ 
int wmt_jenc_set_drv(jenc_drvinfo_t *drv)
{
    return 0;
} /* End of wmt_jenc_set_drv() */
/*!*************************************************************************
* wmt_jenc_get_info
* 
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jenc_get_info(char *buf, char **start, off_t offset, int len)
{
    char *p = buf;
    return (p - buf);
} /* End of wmt_jenc_get_info() */
/*!*************************************************************************
* wmt_jenc_open
* 
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jenc_open(jenc_drvinfo_t *drv)
{
    TRACE("Enter\n");

    /*--------------------------------------------------------------------------
        Step 1: Send decoding info to DSP
    --------------------------------------------------------------------------*/
    dsp_cmd_handler(CMD_A2D_VENC_OPEN, VE_JPEG, 0, 0);

    TRACE("Leave\n");

    return 0;
} /* End of wmt_jenc_open() */

/*!*************************************************************************
* wmt_jenc_close
* 
*/
/*!
* \brief
*	Close JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jenc_close(jenc_drvinfo_t *drv)
{
    TRACE("Enter\n");

    /*--------------------------------------------------------------------------
        Step 1: Send decoding info to DSP
    --------------------------------------------------------------------------*/
    dsp_cmd_handler(CMD_A2D_VENC_CLOSE, VE_JPEG, 0, 0);

    TRACE("Leave\n");

    return 0;
} /* End of wmt_jenc_close() */

/*!*************************************************************************
* wmt_jenc_probe
* 
*/
/*!
* \brief
*	Probe function
*
* \retval  0 if success
*/ 
int wmt_jenc_probe(void)
{
    TRACE("Enter\n");

    TRACE("Leave\n");

    return 0;
} /* End of wmt_jenc_probe() */

/*--------------------End of Function Body -----------------------------------*/

#undef WM8605_jenc_C
