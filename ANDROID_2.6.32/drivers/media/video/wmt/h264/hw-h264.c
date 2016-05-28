/*++ 
 * linux/drivers/media/video/wmt/h264/hw-h264.c
 * WonderMedia h264 decoder device driver
 *
 * Copyright c 2010  WonderMedia  Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 4F, 533, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/
#ifndef HW_H264_C
/* To assert that only one occurrence is included */
#define HW_H264_C

#include <linux/dma-mapping.h>      /* For dma_alloc_coherent() only */
#include <mach/memblock.h>  /* For MB driver only */
#include "hw-h264.h"
#include "../wmt-vd.h"

#define DEVICE_NAME "WMT-H264" /* appear in /proc/devices & /proc/wm-h264 */
#define DRIVER_NAME "wmt-h264"

#define PRD_MAX_SIZE        (60*1024)  /* Max 64k */

#define H264_GET_STATUS(drv)       (drv)->_status
#define H264_SET_STATUS(drv, sta)  (drv)->_status = (sta)

static int h264_dev_ref = 0; /* is device open */

static spinlock_t   h264_lock;

DECLARE_MUTEX(h264_decoding_sema);

    
/*!*************************************************************************
* ioctl_get_capability
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*	Get H264 hardware capability
*
* \retval  0 if success
*/ 
static int ioctl_get_capability(h264_drvinfo_t *drvinfo, h264_capability_t *capab)
{
    memcpy(&drvinfo->capab_tbl , capab, sizeof(h264_capability_t));

#if 0
   // if(user_to_prdt(arg->src_addr, arg->src_size, (struct prdt_struct *)drvinfo->prd_virt, 
   //                  MAX_INPUT_BUF_SIZE/sizeof(struct prdt_struct))){
    if(user_to_prdt((unsigned long)capab->buf, capab->buf_size, (struct prdt_struct *)drvinfo->prd_virt, 
                     MAX_INPUT_BUF_SIZE/sizeof(struct prdt_struct))){
        DBG_ERR("get capab input buffer to prd table error!!\n");
        return -EFAULT;
    }
#endif

    if( wmt_h264_get_capability(drvinfo) == 0) {
        memcpy(capab, &drvinfo->capab_tbl, sizeof(h264_capability_t));

        DBG("Hardware supported capabilities\n");
        DBG("--------------------------------\n");
        //DBG("  Identity:        0x%x (%d)\n", capab->identity, capab->identity & 0xFFFF);
        //DBG("  Chip ID:           %d\n", capab->chip_id );
        //DBG("  Progressive:       %d\n", capab->progressive );
    }
    return 0;
} /* End of ioctl_get_capability() */

static int ioctl_flush(void)
{

       int ret;
	 ret = wmt_h264_flush();

       return ret;
} /* End of ioctl_get_capability() */


/*!*************************************************************************
* ioctl_set_attr
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*	H264 hardware initialization
*
* \retval  0 if success
*/ 
static int ioctl_set_attr(h264_drvinfo_t *drvinfo, h264_attr_t *attr)
{
	// h264_decode_info_t    *hdr_in = &(drvinfo->attr.hdr);

    H264_SET_STATUS(drvinfo, STA_ATTR_SET);

    //DBG("Width:         %d\n", hdr_in->h264_header.video_object_layer_width);
    //DBG("Height:        %d\n", hdr_in->h264_header.video_object_layer_height);
    //DBG("version:       %d\n", hdr_in->version);
  
    memcpy(&drvinfo->attr, attr, sizeof(h264_attr_t));

    wmt_h264_set_attr(drvinfo);
    
    return 0;
} /* End of ioctl_set_attr() */






/*!*************************************************************************
* ioctl_decode_proc
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*	Open H264 hardware
*
* \retval  0 if success
*/ 
static int ioctl_decode_proc(h264_drvinfo_t *drvinfo, vd_decode_input_t *arg)
{
    int ret;
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_PROC"
    ==========================================================================*/
    DBG("src_addr:        %8x\n", arg->src_addr);
    DBG("src_size:        %8d\n", arg->src_size);
    DBG("flags:         0x%8x\n", arg->flags);
#if 0	
    DBG("dst_y_addr:    0x%8x\n", arg->dst_y_addr);
    DBG("dst_y_size:      %8d\n", arg->dst_y_size);
    DBG("dst_c_addr:    0x%8x\n", arg->dst_c_addr);
    DBG("dst_c_size:      %8d\n", arg->dst_c_size);

    DBG("prev_y_addr:   0x%8x\n", arg->prev_y_addr);
    DBG("prev_c_addr:   0x%8x\n", arg->prev_c_addr);
    DBG("next_y_addr:   0x%8x\n", arg->next_y_addr);
    DBG("next_c_addr:   0x%8x\n", arg->next_c_addr);

    DBG("linesize:        %8d\n", arg->linesize);
    DBG("bits_count:      %8d\n", arg->bits_count);
    DBG("data_start_bit:  %8d\n", arg->data_start_bit);
    DBG("data_vld_add:  0x%8x\n", arg->data_vld_add);
    DBG("data_vld_size:   %8d\n", arg->data_vld_size);
#endif

    memcpy(&drvinfo->arg_in, arg, sizeof(vd_decode_input_t));

    /*--------------------------------------------------------------------------
        Step 1: Transfer the input address as PRD foramt
    --------------------------------------------------------------------------*/
#ifdef H264_TEST_FAKE_BITSTREAM
	rdptr =(char *)arg->src_addr;
      rdsize = arg->src_size;
#endif



    if(user_to_prdt(arg->src_addr, arg->src_size, (struct prdt_struct *)drvinfo->prd_virt, 
                     MAX_INPUT_BUF_SIZE/sizeof(struct prdt_struct))){
        DBG_ERR("input buffer to prd table error!!\n");
        H264_SET_STATUS(drvinfo, STA_ERR_BAD_PRD);
        return -EFAULT;
    }
    DBG("user_to_prdt addr 0x%08x size 0x%08x  prdtbl 0x%08x \n",arg->src_addr,arg->src_size,drvinfo->prd_virt);


    
    H264_SET_STATUS(drvinfo, STA_DECODEING);
    ret = wmt_h264_decode_proc(drvinfo);    

    return ret;
} /* End of ioctl_decode_proc() */

/*!*************************************************************************
* ioctl_decode_finish
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*	Open H264 hardware
*
* \retval  0 if success
*/ 
static int ioctl_decode_finish(h264_drvinfo_t *drvinfo, h264_frameinfo_t *frame_info)
{
    int ret;
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_PROC"
    ==========================================================================*/

    memcpy(&drvinfo->arg_out, frame_info, sizeof(h264_frameinfo_t));

    ret = wmt_h264_decode_finish(drvinfo);
    memcpy(frame_info, &drvinfo->arg_out, sizeof(h264_frameinfo_t));

    DBG("img_w:           %8d\n", frame_info->fb.img_w);
    DBG("img_h:           %8d\n", frame_info->fb.img_h);
    DBG("fb_w:            %8d\n", frame_info->fb.fb_w);
    DBG("fb_h:            %8d\n", frame_info->fb.fb_h);
    DBG("phy_addr_y:    0x%8x\n", frame_info->fb.y_addr);
    DBG("y_size:          %8d\n", frame_info->fb.y_size);
    DBG("phy_addr_c:    0x%8x\n", frame_info->fb.c_addr);
    DBG("c_size:          %8d\n", frame_info->fb.c_size);
    DBG("col_fmt:         %8d\n", frame_info->fb.col_fmt);
    DBG("bpp:             %8d\n", frame_info->fb.bpp);
    DBG("y_addr_user:             %8x\n", frame_info->y_addr_user);
    DBG("c_addr_user:             %8x\n", frame_info->c_addr_user);
    return ret;
} /* End of ioctl_decode_finish() */

/*!*************************************************************************
* h264_ioctl
* 
* Private Function by Max Chen, 2010/04/20
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
void h264_dumpVirtualMem(char *buf,int len)
{
	int index=0;
		char *vir_buf=buf;

#ifdef DEBUG	
#endif
	printk(">>>>>dump addr 0x %08X length %d  \n",(unsigned int)vir_buf,len);
	for (index=0;index<len;index+=16)
	{
		printk("0x%08X\t",(unsigned int)(vir_buf+index));
		printk("%08X\t",*(volatile unsigned int *)(vir_buf+index));
		printk("%08X\t",*(volatile unsigned int *)(vir_buf+index+4));
		printk("%08X\t",*(volatile unsigned int *)(vir_buf+index+8));
		printk("%08X\t",*(volatile unsigned int *)(vir_buf+index+12));
		printk("\n");
	}
}

           char tmbuf[256];

static int h264_ioctl(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg)
{
    int err = 0;
    int retval = 0;
    h264_attr_t       attr;
    vd_decode_input_t        input;
    h264_frameinfo_t    frame_info;
    h264_capability_t capab;
    unsigned long      flags =0;
    h264_drvinfo_t    *drvinfo;
    
    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Check input arguments
    --------------------------------------------------------------------------*/
    if( _IOC_TYPE(cmd) != VD_IOC_MAGIC )	return -ENOTTY;
    if( _IOC_NR(cmd) > VD_IOC_MAXNR )	return -ENOTTY;

    /* check argument area */
    if( _IOC_DIR(cmd) & _IOC_READ )
        err = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
    else if ( _IOC_DIR(cmd) & _IOC_WRITE )
        err = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));

    if( err ) return -EFAULT;
      
    /*--------------------------------------------------------------------------
        Step 2: IOCTL handler
    --------------------------------------------------------------------------*/
    spin_lock_irqsave(&h264_lock, flags);
    drvinfo = (h264_drvinfo_t *)filp->private_data;
    
    switch(cmd){       
        case VDIOSET_DECODE_INFO:
            DBG_DETAIL("Receive IOCTL:VDIOSET_DECODE_INFO\n");
            printk("Receive IOCTL:VDIOSET_DECODE_INFO\n");

            retval = copy_from_user(&attr, (const void *)arg, sizeof(h264_attr_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOSET_DECODE_INFO!\n");
                break;
            }
            retval = IOCTL_CMD_CHECK(&attr, h264_attr_t, VD_H264);
            if( retval ) {
            	  //printk("sizeof(h264_attr_t) %d \n",sizeof(h264_attr_t));	
                DBG_ERR("Version conflict (0x%x) in VDIOSET_DECODE_INFO\n", retval);
                DBG_ERR("identity: 0x%x, size:%d\n", attr.identity, attr.size);
            }
            else {
                retval = ioctl_set_attr(drvinfo, &attr);
            }
            break;

        case VDIOSET_DECODE_PROC:
            DBG_DETAIL("Receive IOCTL:VDIOSET_DECODE_PROC\n");
            retval = copy_from_user((vd_decode_input_t *)&input, (const void *)arg, sizeof(vd_decode_input_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOSET_DECODE_PROC!\n");   
                break;
            }
            retval = IOCTL_CMD_CHECK(&input, vd_decode_input_t, VD_H264);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOSET_DECODE_PROC\n", retval);
                DBG("identity: 0x%x, size:%d\n", input.identity, input.size);
            }
            else { 
            	   //copy_from_user(tmbuf,input.src_addr,8);
            	  // h264_dumpVirtualMem(tmbuf,8);
#if 0
                if( ((PAGE_ALIGN(input.src_size)/PAGE_SIZE)*8) > MAX_INPUT_BUF_SIZE ) {
                    DBG_ERR("input size overflow. %x/%x\n", input.src_size, MAX_INPUT_BUF_SIZE*4096/8);
                    return -EFAULT;
                }
#endif
                down_interruptible(&h264_decoding_sema);
                /*--------------------------------------------------------------
                    Before HW decoding finish, we cannot decode another H264
                --------------------------------------------------------------*/

                spin_unlock_irqrestore(&h264_lock, flags);
                                
                retval = ioctl_decode_proc(drvinfo, &input);
                                
                spin_lock_irqsave(&h264_lock, flags);
            }
            break;

        case VDIOGET_DECODE_FINISH:
            DBG_DETAIL("Receive IOCTL:VDIOGET_DECODE_FINISH\n");
            if( H264_GET_STATUS(drvinfo) ==  STA_ERR_BAD_PRD ) {
                retval = -1;
                break;
            }
            retval = copy_from_user((vd_ioctl_cmd*)&frame_info, (const void *)arg, sizeof(vd_ioctl_cmd));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOGET_DECODE_FINISH!\n");   
                break;
            }
            retval = IOCTL_CMD_CHECK(&frame_info, h264_frameinfo_t, VD_H264);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOGET_DECODE_FINISH\n", retval);
                DBG("identity: 0x%x, size:%d\n", frame_info.identity, frame_info.size);
                retval = -ENOTTY;
            }
            else {
                spin_unlock_irqrestore(&h264_lock, flags);
                retval = ioctl_decode_finish(drvinfo, &frame_info);
                copy_to_user((h264_frameinfo_t *)arg, &frame_info, sizeof(h264_frameinfo_t));
                spin_lock_irqsave(&h264_lock, flags);

                /*--------------------------------------------------------------
                    Before HW decoding finish, we cannot decode another H264
                --------------------------------------------------------------*/
                up(&h264_decoding_sema); 
            }
            break;

        case VDIOGET_CAPABILITY:
            printk("Receive IOCTL:VDIOGET_CAPABILITY\n");
            retval = copy_from_user((h264_capability_t*)&capab, (const void *)arg, sizeof(h264_capability_t));
            //h264_dumpVirtualMem(tmbuf,8);


            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOGET_CAPABILITY!\n");   
                break;
            }
            retval = IOCTL_CMD_CHECK(&capab, h264_capability_t, VD_H264);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOGET_CAPABILITY\n", retval);
                DBG("identity: 0x%x, size:%d\n", capab.identity, capab.size);
                retval = -ENOTTY;
            }
            else {
                retval = ioctl_get_capability(drvinfo, &capab);
                if( retval == 0 ) {
                    retval = copy_to_user((h264_capability_t *)arg, &capab, sizeof(h264_capability_t));
                }
            }
            break;
        case VDIOGET_DECODE_FLUSH:
		retval = ioctl_flush();
            break;
        default:
            DBG_ERR("Unknown IOCTL:0x%x in h264_ioctl()!\n", cmd);
            retval = -ENOTTY;
            break;
    } /* switch(cmd) */
    spin_unlock_irqrestore(&h264_lock, flags);
     
    TRACE("Leave\n");

    return retval;
} /* End of h264_ioctl() */

/*!*************************************************************************
* h264_open
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*    It is called when driver is opened, and initial hardward and resource
* \parameter
*   inode   [IN] a pointer point to struct inode
*   filp    [IN] a pointer point to struct file
* \retval  0 if success
*/ 
static int h264_open(struct inode *inode, struct file *filp)
{
    h264_drvinfo_t *drvinfo;
    struct videodecoder_info *vd_info;
    int ret = 0;

    TRACE("Enter\n");
    init_MUTEX(&h264_decoding_sema);
    /*--------------------------------------------------------------------------
        Step 1: Initial Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&h264_lock);
    if(h264_dev_ref) {
        /* Currently we do not support multi-open */
        spin_unlock(&h264_lock);
        return -EBUSY;
    }   

    h264_dev_ref++;
    spin_unlock(&h264_lock);
        
    /*--------------------------------------------------------------------------
        Step 2: Initial hardware decoder
    --------------------------------------------------------------------------*/
    drvinfo = kmalloc(sizeof(h264_drvinfo_t), GFP_KERNEL);
    if( drvinfo == 0 ) {
        DBG_ERR("Allocate %d bytes fail!\n", sizeof(h264_drvinfo_t));
        ret = -EBUSY;
        goto EXIT_h264_open;
    }
    /*--------------------------------------------------------------------------
        private_data from VD is "PRD table virtual & physical address"
    --------------------------------------------------------------------------*/
    vd_info = (struct videodecoder_info *)filp->private_data;

    drvinfo->prd_virt = (unsigned int)vd_info->prdt_virt;
    drvinfo->prd_addr = (unsigned int)vd_info->prdt_phys;

    printk("prd_virt: 0x%x, prd_addr: 0x%x\n", drvinfo->prd_virt, drvinfo->prd_addr);
    
    /* change privata_data to drvinfo */
    filp->private_data = drvinfo;
    drvinfo->size      = sizeof(h264_drvinfo_t);
    drvinfo->_lock     = h264_lock;


    H264_SET_STATUS(drvinfo, STA_READY);

    ret = wmt_h264_open(drvinfo);

EXIT_h264_open:
    TRACE("Leave\n");
    
    return ret;
} /* End of h264_open() */

/*!*************************************************************************
* h264_release
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       It is called when driver is released, and reset hardward and free resource
* \parameter
*   inode   [IN] a pointer point to struct inode
*   filp    [IN] a pointer point to struct file
* \retval  0 if success
*/ 
static int h264_release(struct inode *inode, struct file *filp)
{
    h264_drvinfo_t *drvinfo = (h264_drvinfo_t *)filp->private_data;

    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Close hardware decoder
    --------------------------------------------------------------------------*/
    wmt_h264_close();
    
    H264_SET_STATUS(drvinfo, STA_CLOSE);

    if( filp->private_data ) {
        kfree(filp->private_data);
    }

    /*--------------------------------------------------------------------------
        Step 2: Release Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&h264_lock);
    if(!h264_dev_ref) {
        spin_unlock(&h264_lock);
        return -EBUSY;
    }   
    h264_dev_ref--;
    spin_unlock(&h264_lock);
    module_put(THIS_MODULE);

    TRACE("Leave\n");

    return 0;
} /* End of h264_release() */

/*!*************************************************************************
* h264_probe
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       Probe
* \retval  0 if success
*/ 
static int h264_probe(void /*struct device *dev*/)
{
    TRACE("Enter\n");

    spin_lock_init(&h264_lock);

    wmt_h264_probe();

    TRACE("Leave\n");

    return 0;
} /* End of h264_probe() */

/*!*************************************************************************
* h264_remove
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       remove h264 module
* \retval  0 if success
*/ 
static int h264_remove(void)
{   
    TRACE("Enter\n");
    TRACE("Leave\n");

    return 0;
} /* End of h264_remove() */

/*!*************************************************************************
* h264_suspend
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       suspend h264 module
* \parameter
*   dev
*   state
*   level
* \retval  0 if success
*/ 
static int h264_suspend(pm_message_t state)
{
    TRACE("Enter\n");
    switch (state.event) {
        case PM_EVENT_SUSPEND:
        case PM_EVENT_FREEZE:
        case PM_EVENT_PRETHAW:
        default:
            wmt_h264_suspend();
            break;	
    }
    TRACE("Leave\n");
    
    return 0;
} /* End of h264_suspend() */

/*!*************************************************************************
* h264_resume
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       resume h264 module
* \retval  0 if success
*/ 
static int h264_resume(void)
{
    TRACE("Enter\n");
    // TO DO
    TRACE("Leave\n");

    return 0;
} /* End of h264_resume() */

/*!*************************************************************************
* h264_get_info
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       resume h264 module
* \retval  0 if success
*/ 
#ifdef CONFIG_PROC_FS
static int h264_get_info(char *buf, char **start, off_t offset, int len)
{
    char *p = buf;

    TRACE("Enter\n");

    // TO DO
    p += sprintf(p, "%s previous state READY\n",     DRIVER_NAME);
    p += sprintf(p, "%s current state READY\n\n",    DRIVER_NAME);
    p += sprintf(p, "%s show other information\n\n", DRIVER_NAME);
    // other information or register
    // ...

    TRACE("Leave\n");

    return (p - buf);
} /* End of h264_get_info() */
#endif

/*!*************************************************************************
* h264_setup
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       
* \retval  0 if success
*/
static int h264_setup(void)
{
    TRACE("Enter\n");

    return h264_probe();
} /* End of h264_setup() */

/*!*************************************************************************
    platform device struct define
****************************************************************************/

struct videodecoder h264_decoder = {
    .name    = DRIVER_NAME,
    .id      = VD_H264,
    .setup   = h264_setup,
    .remove  = h264_remove,
    .suspend = h264_suspend,
    .resume  = h264_resume,
    .fops = {
                .owner   = THIS_MODULE,
                .open    = h264_open,
                //.read    = h264_read,
                //.write   = h264_write,
                .ioctl   = h264_ioctl,
                //.mmap    = h264_mmap,
                .release = h264_release,
            },
#ifdef CONFIG_PROC_FS
    .get_info = h264_get_info,
#endif /* CONFIG_PROC_FS */
    .device = NULL,
};

/*!*************************************************************************
* hw_h264_init
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*       init h264 module
* \retval  0 if success
*/ 
static int hw_h264_init(void)
{
    int ret;

    TRACE("Enter\n");

    ret = videodecoder_register(&h264_decoder);
    
    TRACE("Leave\n");

    return ret;
} /* End of hw_h264_init() */

module_init(hw_h264_init);

/*!*************************************************************************
* hw_h264_exit
* 
* Private Function by Max Chen, 2010/04/20
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static void hw_h264_exit(void)
{
    TRACE("Enter\n");

    videodecoder_unregister(&h264_decoder);

    TRACE("Leave\n");

    return;
} /* End of hw_h264_exit() */

module_exit(hw_h264_exit);

MODULE_AUTHOR("WonderMedia Multimedia SW Team Max Chen");
MODULE_DESCRIPTION("WM8525 H264 decode device driver");
MODULE_LICENSE("GPL");

/*------------------------------------------------------------------------------*/
/*--------------------End of Function Body -----------------------------------*/

#undef HW_H264_C
#endif
