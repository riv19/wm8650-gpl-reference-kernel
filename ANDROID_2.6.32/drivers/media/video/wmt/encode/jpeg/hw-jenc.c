/* PVCS version log
** $Log:  $
 */
#define HW_JENC_C

/*--- hw-jenc.c ---------------------------------------------------------------
*   Copyright (C) 2008 WonderMedia Tech. Inc.
*
* MODULE       : hw-jenc.c
* AUTHOR       : Willy Chuang
* DATE         : 2008/11/24
* DESCRIPTION  : 
*-----------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Willy Chuang, 2008/11/24
*    First version
*
*------------------------------------------------------------------------------*/
#include <linux/dma-mapping.h>      /* For dma_alloc_coherent() only */

#include "hw-jenc.h"

//#define CFG_VERSION_CHECK

#define DEVICE_NAME "WMT-JENC" /* appear in /proc/devices & /proc/wm-jenc */
#define DRIVER_NAME "wmt-jenc"

#define JENC_GET_STATUS(drv)       (drv)->_status
#define JENC_SET_STATUS(drv, sta)  (drv)->_status = (sta)

static int jpeg_dev_ref = 0; /* is device open */

static spinlock_t   jenc_lock;

//DECLARE_MUTEX(jenc_decoding_sema);

    
/*!*************************************************************************
* ioctl_get_capability
* 
*/
/*!
* \brief
*    Get JPEG hardware capability
*
* \retval  0 if success
*/ 
static int ioctl_get_capability(jenc_drvinfo_t *drvinfo, jenc_capability_t *capab)
{
    return 0;
} /* End of ioctl_get_capability() */

/*!*************************************************************************
* ioctl_set_attr
* 
*/
/*!
* \brief
*    JPEG hardware initialization
*
* \retval  0 if success
*/ 
static int ioctl_set_attr(jenc_drvinfo_t *drvinfo, jenc_attr_t *attr)
{
    
    return 0;
} /* End of ioctl_set_attr() */

/*!*************************************************************************
* ioctl_decode_proc
* 
*/
/*!
* \brief
*    Open JPEG hardware
*
* \retval  0 if success
*/ 
static int ioctl_decode_proc(jenc_drvinfo_t *drvinfo, jenc_input_t *arg)
{
    int ret = -EINVAL;
    memcpy(&drvinfo->arg_in, arg, sizeof(jenc_input_t));
    ret = wmt_jenc_decode_proc(drvinfo);    

    return ret;
} /* End of ioctl_decode_proc() */

#if 0
/*!*************************************************************************
* copy_frame
* 
* Private Function by Willy Chuang, 2009/12/03
*/
/*!
* \brief
*	Copy frame buffer information
*
* \retval  0 if success
*/ 
static int copy_frame(jenc_drvinfo_t *drvinfo, jenc_frameinfo_t *frame_info)
{
    memcpy(frame_info, &drvinfo->arg_out, sizeof(jenc_frameinfo_t));

    DBG_MSG("img_w:           %d\n", frame_info->fb.img_w );  
    DBG_MSG("img_h:           %d\n", frame_info->fb.img_h );  
    DBG_MSG("fb_w:            %d\n", frame_info->fb.fb_w );  
    DBG_MSG("fb_h:            %d\n", frame_info->fb.fb_h );  
    DBG_MSG("phy_addr_y:    0x%x\n", frame_info->fb.y_addr );  
    DBG_MSG("y_size:          %d\n", frame_info->fb.y_size );  
    DBG_MSG("phy_addr_c:    0x%x\n", frame_info->fb.c_addr );  
    DBG_MSG("c_size:          %d\n", frame_info->fb.c_size );  
    DBG_MSG("col_fmt:         %d\n", frame_info->fb.col_fmt );  
    DBG_MSG("bpp:             %d\n", frame_info->fb.bpp );  

    drvinfo->_set_attr_ready = 0;

    down_interruptible(&jenc_decoding_sema); 

    return 0;    
} /* End of copy_frame() */
#endif
/*!*************************************************************************
* ioctl_decode_finish
* 
*/
/*!
* \brief
*    Open JPEG hardware
*
* \retval  0 if success
*/ 
static int ioctl_decode_finish(jenc_drvinfo_t *drvinfo, jenc_frameinfo_t *frame_info)
{
    int ret = -EINVAL;
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_FINISH"
    ==========================================================================*/
    ret = wmt_jenc_decode_finish(drvinfo);
    frame_info->mjpeg_size = drvinfo->arg_out.mjpeg_size;

    return ret;
} /* End of ioctl_decode_finish() */

/*!*************************************************************************
* ioctl_decode_flush
* 
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
static int ioctl_decode_flush(jenc_drvinfo_t *drvinfo, jenc_frameinfo_t *frame_info)
{
    int ret = -EINVAL;
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_FLUSH"
    ==========================================================================*/
#if 0
    if( drvinfo->_set_attr_ready != 1){
        DBG_ERR("Decode fail without set attr ready!\n");
    }
    else {
        memcpy(&drvinfo->arg_out, frame_info, sizeof(jenc_frameinfo_t));

        ret = wmt_jenc_decode_flush(drvinfo);
    }
    copy_frame(drvinfo, frame_info);
#endif
    return ret;
} /* End of ioctl_decode_flush() */

/*!*************************************************************************
* ioctl_get_status
* 
*/
/*!
* \brief
*	Get driver status
*
* \retval  0 if success
*/ 
static int ioctl_get_status(jenc_drvinfo_t *drvinfo, jenc_status_t *jsts)
{
    int ret = -1;
		//TBD
    return ret;
} /* End of ioctl_get_status() */

/*!*************************************************************************
* jenc_ioctl
* 
*/
/*!
* \brief
*       It implement io control for AP
* \parameter
*   inode   [IN] a pointer point to struct inode
*   filp    [IN] a pointer point to struct file
*   cmd
*   arg     
* \retval  0 if success
*/ 
static int jenc_ioctl(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg)
{
    int err = 0;
    int retval = -EINVAL;
    jenc_attr_t       attr;
    jenc_input_t      input;
    jenc_frameinfo_t  frame_info;
    jenc_capability_t capab;
    jenc_status_t     drvsts;
    unsigned long     flags =0;
    jenc_drvinfo_t   *drvinfo;
    
    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Check input arguments
    --------------------------------------------------------------------------*/
    if( _IOC_TYPE(cmd) != VD_IOC_MAGIC )    return -ENOTTY;
    if( _IOC_NR(cmd) > VD_IOC_MAXNR )    return -ENOTTY;

    /* check argument area */
    if( _IOC_DIR(cmd) & _IOC_READ )
        err = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
    else if ( _IOC_DIR(cmd) & _IOC_WRITE )
        err = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));

    if( err ) return -EFAULT;
      
    /*--------------------------------------------------------------------------
        Step 2: IOCTL handler
    --------------------------------------------------------------------------*/
    spin_lock_irqsave(&jenc_lock, flags);
    drvinfo = (jenc_drvinfo_t *)filp->private_data;

    TRACE("drvinfo: %p, cmd: 0x%x\n", (void *)drvinfo, cmd);
    
	// allow to change current driver info only if in the same HW open
	if((cmd == VDIOSET_DECODE_INFO) || (cmd == VDIOSET_DECODE_PROC) ||
		(cmd == VDIOGET_DECODE_FINISH) || (cmd == VDIOGET_DECODE_FLUSH) ||
		(cmd == VDIOGET_DECODE_STATUS))
    wmt_jenc_set_drv(drvinfo);


    switch(cmd){       
        case VDIOSET_DECODE_INFO:
            DBG_DETAIL("Receive IOCTL:VDIOSET_DECODE_INFO\n");
            retval = copy_from_user( &attr, (const void *)arg, 
                                      sizeof(jenc_attr_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOSET_DECODE_INFO!\n");
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&attr, jenc_attr_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOSET_DECODE_INFO\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", attr.identity, attr.size);
            }
            else 
#endif
            {
                retval = ioctl_set_attr(drvinfo, &attr);
            }
            break;

        case VDIOSET_DECODE_PROC:
            DBG_DETAIL("Receive IOCTL:VDIOSET_DECODE_PROC\n");
            retval = copy_from_user((jenc_input_t *)&input, 
                                  (const void *)arg, sizeof(jenc_input_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOSET_DECODE_PROC!\n");   
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&input, jenc_input_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOSET_DECODE_PROC\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", input.identity, input.size);
            }
            else
#endif
            {
//    			if(((PAGE_ALIGN(input.src_size)/PAGE_SIZE)*8) > MAX_INPUT_BUF_SIZE){
//	    			DBG_ERR("input size overflow. %x/%x\n",
//		    			input.src_size, MAX_INPUT_BUF_SIZE*4096/8);
//			    	return -EFAULT;
//			    }
//                down_interruptible(&jenc_decoding_sema); 
                /*--------------------------------------------------------------
                    Before HW decoding finish, we cannot decode another JPEG
                --------------------------------------------------------------*/

                spin_unlock_irqrestore(&jenc_lock, flags);
                                
                retval = ioctl_decode_proc(drvinfo, &input);
                                
                spin_lock_irqsave(&jenc_lock, flags);
            }
            break;

        case VDIOGET_DECODE_FINISH:
        case VDIOGET_DECODE_FLUSH:
            DBG_DETAIL("Receive IOCTL:VDIOGET_DECODE_FINISH\n");
//            if( JENC_GET_STATUS(drvinfo) ==  STA_ERR_BAD_PRD ) {
//                retval = -1;
//                break;
//            }
//            retval = copy_from_user((vd_ioctl_cmd*)&frame_info, (const void *)arg, 
//                                   sizeof(vd_ioctl_cmd));
//            if( retval ) {
//                DBG_ERR("copy_from_user FAIL in VDIOGET_DECODE_FINISH!\n");   
//                break;
//            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&frame_info, jenc_frameinfo_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOGET_DECODE_FINISH\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", frame_info.identity, frame_info.size);
                retval = -ENOTTY;
            }
            else 
#endif
            {
                spin_unlock_irqrestore(&jenc_lock, flags);
                if( cmd == VDIOGET_DECODE_FINISH ) {
                    retval = ioctl_decode_finish(drvinfo, &frame_info);
                }
                else {
                    retval = ioctl_decode_flush(drvinfo, &frame_info);
                }
                copy_to_user((jenc_frameinfo_t *)arg, &frame_info, 
                                           sizeof(jenc_frameinfo_t));
                spin_lock_irqsave(&jenc_lock, flags);

                /*--------------------------------------------------------------
                    Before HW decoding finish, we cannot decode another JPEG
                --------------------------------------------------------------*/
//                up(&jenc_decoding_sema); 
            }
            break;

        case VDIOGET_DECODE_STATUS:
            DBG_DETAIL("Receive IOCTL:VDIOGET_DECODE_STATUS\n");
            retval = copy_from_user((vd_ioctl_cmd*)&drvsts, (const void *)arg, 
                                   sizeof(jenc_status_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOGET_CAPABILITY!\n");   
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&drvsts, jenc_status_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOGET_DECODE_STATUS\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", drvsts.identity, drvsts.size);
                retval = -ENOTTY;
            }
            else
#endif
            {
                spin_unlock_irqrestore(&jenc_lock, flags);

                retval = ioctl_get_status(drvinfo, &drvsts);
                if(retval == 0) {
                    retval = copy_to_user((jenc_status_t *)arg, &drvsts, 
                                           sizeof(jenc_status_t));
                }
                spin_lock_irqsave(&jenc_lock, flags);
            }
            break;

        case VDIOGET_CAPABILITY:
		{
            jenc_drvinfo_t   info;
			drvinfo = &info;

            DBG_DETAIL("Receive IOCTL:VDIOGET_CAPABILITY\n");
            retval = copy_from_user((vd_ioctl_cmd*)&capab, (const void *)arg, 
                                   sizeof(vd_ioctl_cmd));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOGET_CAPABILITY!\n");   
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&capab, jenc_capability_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOGET_CAPABILITY\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", capab.identity, capab.size);
                retval = -ENOTTY;
            }
            else
#endif
            {
                retval = ioctl_get_capability(drvinfo, &capab);
                if(retval == 0) {
                    retval = copy_to_user((jenc_capability_t *)arg, &capab, 
                                           sizeof(jenc_capability_t));
                }
            }
            break;
        }

        default:
            DBG_ERR("Unknown IOCTL:0x%x in jenc_ioctl()!\n", cmd);
            retval = -EINVAL;
            break;
    } /* switch(cmd) */
//EXIT_jenc_ioctl:    
    spin_unlock_irqrestore(&jenc_lock, flags);
     
    TRACE("Leave\n");

    return retval;
} /* End of jenc_ioctl() */

/*!*************************************************************************
* jenc_open
* 
*/
/*!
* \brief
*    It is called when driver is opened, and initial hardward and resource
* \parameter
*   inode   [IN] a pointer point to struct inode
*   filp    [IN] a pointer point to struct file
* \retval  0 if success
*/ 
static int jenc_open(struct inode *inode, struct file *filp)
{
    jenc_drvinfo_t *drvinfo;
    struct videodecoder_info *vd_info;
    int ret = 0;

    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Initial Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&jenc_lock);
    if(jpeg_dev_ref == 1){
        spin_unlock(&jenc_lock);
        return -EBUSY;
    }
    jpeg_dev_ref = 1;
    
    /*--------------------------------------------------------------------------
        Step 2: Initial hardware decoder
    --------------------------------------------------------------------------*/
    drvinfo = kmalloc(sizeof(jenc_drvinfo_t), GFP_KERNEL);
    if( drvinfo == 0 ) {
        DBG_ERR("Allocate %d bytes fail!\n", sizeof(jenc_drvinfo_t));
        ret = -EBUSY;
        goto EXIT_jenc_open;
    }
    memset(drvinfo, 0, sizeof(jenc_drvinfo_t));
    /*--------------------------------------------------------------------------
        private_data from VD is "PRD table virtual & physical address"
    --------------------------------------------------------------------------*/
    vd_info = (struct videodecoder_info *)filp->private_data;
    
    drvinfo->prd_virt = (unsigned int)vd_info->prdt_virt;
    drvinfo->prd_addr = (unsigned int)vd_info->prdt_phys;

    DBG_DETAIL("prd_virt: 0x%x, prd_addr: 0x%x\n", drvinfo->prd_virt, drvinfo->prd_addr);
    
    /* change privata_data to drvinfo */
    filp->private_data = drvinfo;
    drvinfo->size      = sizeof(jenc_drvinfo_t);
    drvinfo->_lock     = jenc_lock;

    wmt_jenc_open(drvinfo);

EXIT_jenc_open:
    spin_unlock(&jenc_lock);
    TRACE("Leave\n");
    
    return ret;
} /* End of jenc_open() */

/*!*************************************************************************
* jenc_release
* 
*/
/*!
* \brief
*       It is called when driver is released, and reset hardward and free resource
* \parameter
*   inode   [IN] a pointer point to struct inode
*   filp    [IN] a pointer point to struct file
* \retval  0 if success
*/ 
static int jenc_release(struct inode *inode, struct file *filp)
{
    jenc_drvinfo_t *drvinfo = (jenc_drvinfo_t *)filp->private_data;

    TRACE("Enter\n");
    
    if(jpeg_dev_ref == 0){
        DBG_ERR("jenc_release() is called without being opened!\n");
        return 0;
    }
    spin_lock(&jenc_lock);
    /*--------------------------------------------------------------------------
        Step 1: Close hardware decoder
    --------------------------------------------------------------------------*/
    jpeg_dev_ref = 0;
    
    JENC_SET_STATUS(drvinfo, STA_CLOSE);

    /*--------------------------------------------------------------------------
        Step 2: Release Linux settings
    --------------------------------------------------------------------------*/
    wmt_jenc_close(drvinfo);
    DBG_MSG("MJPEG uninit HW.\n");

    if( filp->private_data ) {
        kfree(filp->private_data);
    }

    spin_unlock(&jenc_lock);
    module_put(THIS_MODULE);

    TRACE("Leave\n");

    return 0;
} /* End of jenc_release() */

/*!*************************************************************************
* jenc_probe
* 
*/
/*!
* \brief
*       Probe
* \retval  0 if success
*/ 
static int jenc_probe(void /*struct device *dev*/)
{
    TRACE("Enter\n");

    spin_lock_init(&jenc_lock);

    wmt_jenc_probe();

    TRACE("Leave\n");

    return 0;
} /* End of jenc_probe() */

/*!*************************************************************************
* jenc_remove
* 
*/
/*!
* \brief
*       remove jpeg module
* \retval  0 if success
*/ 
static int jenc_remove(void)
{   
    TRACE("Enter\n");
    TRACE("Leave\n");

    return 0;
} /* End of jenc_remove() */

/*!*************************************************************************
* jenc_suspend
* 
*/
/*!
* \brief
*       suspend jpeg module
* \parameter
*   dev
*   state
*   level
* \retval  0 if success
*/ 
static int jenc_suspend(pm_message_t state)
{
    TRACE("Enter\n");
    switch (state.event) {
        case PM_EVENT_SUSPEND:
        case PM_EVENT_FREEZE:
        case PM_EVENT_PRETHAW:
        default:
            wmt_jenc_suspend();
            break;	
    }
    TRACE("Leave\n");
    
    return 0;
} /* End of jenc_suspend() */

/*!*************************************************************************
* jenc_resume
* 
*/
/*!
* \brief
*       resume jpeg module
* \retval  0 if success
*/ 
static int jenc_resume(void)
{
    TRACE("Enter\n");
   
    wmt_jenc_resume();

    TRACE("Leave\n");

    return 0;
} /* End of jenc_resume() */

/*!*************************************************************************
* jenc_setup
* 
*/
/*!
* \brief
*       
* \retval  0 if success
*/
static int jenc_setup(void)
{
    return jenc_probe();
} /* End of jenc_setup() */

/*!*************************************************************************
    platform device struct define
****************************************************************************/

struct videodecoder jpeg_encoder = {
    .name    = DRIVER_NAME,
    .id      = VE_JPEG,
    .setup   = jenc_setup,
    .remove  = jenc_remove,
    .suspend = jenc_suspend,
    .resume  = jenc_resume,
    .fops = {
                .owner   = THIS_MODULE,
                .open    = jenc_open,
                .ioctl   = jenc_ioctl,
                .release = jenc_release,
            },
    .device = NULL,
};

/*!*************************************************************************
* hw_jenc_init
* 
*/
/*!
* \brief
*       init jpeg module
* \retval  0 if success
*/ 
static int hw_jenc_init(void)
{
    int ret;

    TRACE("Enter\n");

    ret = videodecoder_register(&jpeg_encoder);
    
    TRACE("Leave\n");

    return ret;
} /* End of hw_jenc_init() */

module_init(hw_jenc_init);

/*!*************************************************************************
* hw_jenc_exit
* 
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static void hw_jenc_exit(void)
{
    TRACE("Enter\n");

    videodecoder_unregister(&jpeg_encoder);

    TRACE("Leave\n");

    return;
} /* End of hw_jenc_exit() */

module_exit(hw_jenc_exit);

MODULE_DESCRIPTION("WM8650 JPEG encoder device driver");
MODULE_LICENSE("GPL");

/*------------------------------------------------------------------------------*/
/*--------------------End of Function Body -----------------------------------*/

#undef HW_JENC_C
