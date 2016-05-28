/*++ 
 * drivers\media\video\wmt\jpeg\hw-jdec.c 
 * WonderMedia common SoC hardware JPEG decoder driver
 *
 * Copyright (c) 2010  WonderMedia  Technologies, Inc.
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
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/

#define HW_JDEC_C

/*------------------------------------------------------------------------------
 * ChangeLog
 *
 * 2010-08-10  Willy Chunag  <willychuang@wondermedia.com.tw>
 *      - Add License declaration and ChangeLog
 *
------------------------------------------------------------------------------*/

#include <linux/dma-mapping.h>      /* For dma_alloc_coherent() only */

#include "hw-jdec.h"

//#define CFG_VERSION_CHECK

#define DEVICE_NAME "WMT-JDEC" /* appear in /proc/devices & /proc/wm-jdec */
#define DRIVER_NAME "wmt-jdec"

#define JDEC_GET_STATUS(drv)       (drv)->_status
#define JDEC_SET_STATUS(drv, sta)  (drv)->_status = (sta)

static int jpeg_dev_ref = 0; /* is device open */
static int jpeg_serial_no = 0;

static spinlock_t   jdec_lock;

static int is_mjpeg_run = 0;

DECLARE_MUTEX(jdec_decoding_sema);

    
/*!*************************************************************************
* ioctl_get_capability
* 
* Private Function by Willy Chuang, 2008/12/4
*/
/*!
* \brief
*    Get JPEG hardware capability
*
* \retval  0 if success
*/ 
static int ioctl_get_capability(jdec_drvinfo_t *drvinfo, jdec_capability_t *capab)
{
//    char buf[80];
    
    if( wmt_jdec_get_capability(drvinfo) == 0) {
//        int i;
        
        memcpy(capab, drvinfo->capab_tbl, sizeof(jdec_capability_t));

        DBG_MSG("Hardware supported capabilities\n"); 
        DBG_MSG("--------------------------------\n"); 
        DBG_MSG("  Identity:        0x%x (%d)\n", capab->identity, capab->identity & 0xFFFF);
        DBG_MSG("  Chip ID:           %d\n", capab->chip_id );  
        DBG_MSG("  Baseline:          %d\n", capab->baseline );  
        DBG_MSG("  Progressive:       %d\n", capab->progressive );  
        DBG_MSG("  Graylevel:         %d\n", capab->graylevel );  
        DBG_MSG("  Partial decode:    %d\n", capab->partial_decode );  
        DBG_MSG("  Ddecoded to YC420: %d\n", capab->decoded_to_YC420 );  
        DBG_MSG("  Ddecoded to YC422: %d\n", capab->decoded_to_YC422H );  
        DBG_MSG("  Ddecoded to YC444: %d\n", capab->decoded_to_YC444 );  
        DBG_MSG("  Ddecoded to RGB:   %d\n", capab->decoded_to_ARGB );  
#if 0
        for(i=0; i<capab->scale_ratio_num; i++) {
            if( i < (capab->scale_ratio_num-1)) {
                sprintf(buf, "1/%d, ", capab->scale_fator[i] );  
            }
            else {
                sprintf(buf, "1/%d\n", capab->scale_fator[i] );  
            }
        }
        DBG_MSG("Scale down factor: %s", buf);  
#endif
    }

    return 0;
} /* End of ioctl_get_capability() */

/*!*************************************************************************
* ioctl_set_attr
* 
* Private Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*    JPEG hardware initialization
*
* \retval  0 if success
*/ 
static int ioctl_set_attr(jdec_drvinfo_t *drvinfo, jdec_attr_t *attr)
{
	int                ret;
    jdec_hdr_info_t    *hdr;
    jdec_decode_info_t *di;

    if( is_mjpeg_run && (drvinfo->_is_mjpeg != 1) ) {
        DBG_ERR("MJPEG is playing now, cannot decode another JPEG!\n");
        return -1;
    }
    down_interruptible(&jdec_decoding_sema);  // lock 
    
	// MJPEG will open only once because it will not close when finsh or flush
	if(!is_mjpeg_run){
		ret = wmt_jdec_open(drvinfo); // HW should not be opened 
		if(ret){
            DBG_ERR("HW init fail. (%s decode)\n", 
				(attr->hdr.profile & DECFLAG_MJPEG_BIT)?"MJPEG":"JPEG");
			up(&jdec_decoding_sema);
			return ret;
		}
		drvinfo->_is_hw_init = 1;
        DBG_MSG("%s init HW.\n", 
			(attr->hdr.profile & DECFLAG_MJPEG_BIT)?"MJPEG":"JPEG");
	}

    if(attr->hdr.profile & DECFLAG_MJPEG_BIT) {
        drvinfo->_is_mjpeg = 1;
        is_mjpeg_run = 1;
    }
        
    JDEC_SET_STATUS(drvinfo, STA_ATTR_SET);

    DBG_MSG("profile:       %x\n", attr->hdr.profile );  
    DBG_MSG("sof_w:         %d\n", attr->hdr.sof_w );  
    DBG_MSG("sof_h:         %d\n", attr->hdr.sof_h );  
    DBG_MSG("filesize:      %d\n", attr->hdr.filesize );  
    DBG_MSG("src_color:     %d\n", attr->hdr.src_color );  
    DBG_MSG("scanline:      %d\n", attr->di.scanline );  
    DBG_MSG("scale_factor:  %d\n", attr->di.scale_factor );
    DBG_MSG("decoded_color: %d\n", attr->di.decoded_color );  
    DBG_MSG("Partial dec:   %d\n", attr->di.pd.enable );  
    if(attr->di.pd.enable) {
        DBG_MSG("PD X:          %d\n", attr->di.pd.x );  
        DBG_MSG("PD Y:          %d\n", attr->di.pd.y );  
        DBG_MSG("PD width:      %d\n", attr->di.pd.w );  
        DBG_MSG("PD height:     %d\n", attr->di.pd.h );  
    }
   
    memcpy(&drvinfo->attr, attr, sizeof(jdec_attr_t));
   
    hdr = &drvinfo->attr.hdr;
    di  = &drvinfo->attr.di;
    
    drvinfo->_remain_byte = attr->hdr.filesize;
    drvinfo->_segment     = JDEC_SEG_NONE;

    /*--------------------------------------------------------------------------
        Do decoded_color change if necessary
    --------------------------------------------------------------------------*/
    switch( di->decoded_color ) {
        case VDO_COL_FMT_ARGB:
            break;
        case VDO_COL_FMT_AUTO:
            if((hdr->src_color == VDO_COL_FMT_YUV444) || 
               (hdr->src_color == VDO_COL_FMT_YUV422V) ){
                di->decoded_color = VDO_COL_FMT_YUV444;
            }
            else if(hdr->src_color == VDO_COL_FMT_YUV420) {
                di->decoded_color = VDO_COL_FMT_YUV420;
            }
            else if(hdr->src_color == VDO_COL_FMT_GRAY) {
                di->decoded_color = VDO_COL_FMT_YUV420;
            }
            else {
                di->decoded_color = VDO_COL_FMT_YUV422H;
            }
            break;
        case VDO_COL_FMT_YUV420:
        case VDO_COL_FMT_YUV422H:
        case VDO_COL_FMT_YUV444:
            break;
        default:
            DBG_ERR("Not supported decoded color(%d)\n", di->decoded_color);
            return -1;
    }
    if(attr->di.decoded_color != di->decoded_color) {
        DBG_MSG("Change decoded color from %d to %d\n", attr->di.decoded_color, 
                                                         di->decoded_color);
    }
    /*--------------------------------------------------------------------------
        We must ensure the decoded_color is one of following values:
        VDO_COL_FMT_YUV420
        VDO_COL_FMT_YUV422H
        VDO_COL_FMT_YUV444
        VDO_COL_FMT_ARGB
    --------------------------------------------------------------------------*/
    wmt_jdec_set_attr(drvinfo);
    drvinfo->_set_attr_ready = 1;
    
    return 0;
} /* End of ioctl_set_attr() */

/*!*************************************************************************
* ioctl_decode_proc
* 
* Private Function by Willy Chuang, 2008/11/25
*/
/*!
* \brief
*    Open JPEG hardware
*
* \retval  0 if success
*/ 
static int ioctl_decode_proc(jdec_drvinfo_t *drvinfo, jdec_input_t *arg)
{
    int ret = -EINVAL;

    if( drvinfo->_set_attr_ready != 1){
        DBG_ERR("Cannot decode without set attr ready!\n");
        memcpy(&drvinfo->arg_in, arg, sizeof(jdec_input_t));
        return ret;
    }
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_PROC"
    ==========================================================================*/
    DBG_MSG("src_addr:        0x%x\n", arg->src_addr );
    DBG_MSG("src_size:        %d\n",   arg->src_size );
    DBG_MSG("flags:           0x%x\n", arg->flags );
    DBG_MSG("dst_y_addr:      0x%x\n", arg->dst_y_addr );
    DBG_MSG("dst_y_size:      %d\n",   arg->dst_y_size );
    DBG_MSG("dst_c_addr:      0x%x\n", arg->dst_c_addr );
    DBG_MSG("dst_c_size:      %d\n",   arg->dst_c_size );

    DBG_MSG("dst_y_addr_user: 0x%x\n", arg->dst_y_addr_user );
    DBG_MSG("dst_c_addr_user: 0x%x\n", arg->dst_c_addr_user );
    DBG_MSG("dec_to_x:        %d\n",   arg->dec_to_x );
    DBG_MSG("dec_to_y:        %d\n",   arg->dec_to_y );

    DBG_MSG("filesize: %d, remain: %d, seg_size: %d\n", drvinfo->attr.hdr.filesize, 
                                    drvinfo->_remain_byte, arg->src_size);
    /* Wait for previous segment decoded */
    while( drvinfo->_status & STA_DMA_START ) {
        if(drvinfo->_status & STA_DECODE_DONE) {
            /* This JPEG was decoded already. We do not need to do anything */
            return 0;
        }
        DBG_MSG("Wait for DMA finished\n");
        msleep(1);
    }

    /*--------------------------------------------------------------------------
        Step 1: Check whether first or last segment
    --------------------------------------------------------------------------*/
    drvinfo->_multi_seg_en = arg->flags & FLAG_MULTI_SEGMENT_EN;   
    if(drvinfo->_multi_seg_en) {
        /* Multi segment decoding is enabled */
        if((arg->flags & FLAG_FIRST_SEGMENT) || (drvinfo->_remain_byte == drvinfo->attr.hdr.filesize)) {
            drvinfo->_segment |= JDEC_SEG_FIRST;  /* first segment */
        }
        else {
            drvinfo->_segment &= (~JDEC_SEG_FIRST);
        }
        if(drvinfo->_remain_byte < arg->src_size){
            DBG_ERR("Illegal segment size: %d (> %d)\n", arg->src_size, drvinfo->_remain_byte);
        }
        drvinfo->_remain_byte -= arg->src_size;
        if((arg->flags & FLAG_LAST_SEGMENT) || (drvinfo->_remain_byte == 0) ){
            drvinfo->_segment |= JDEC_SEG_LAST;  /* last segment */
        }    
        DBG_MSG("_segment: 0x%x\n", drvinfo->_segment );
    }
    memcpy(&drvinfo->arg_in, arg, sizeof(jdec_input_t));

    /*--------------------------------------------------------------------------
        Step 2: Transfer the input address as PRD foramt
    --------------------------------------------------------------------------*/
    if(user_to_prdt(arg->src_addr, arg->src_size, (struct prdt_struct *)drvinfo->prd_virt, 
                     MAX_INPUT_BUF_SIZE/sizeof(struct prdt_struct))){
        DBG_ERR("input buffer to prd table error\n");
        JDEC_SET_STATUS(drvinfo, STA_ERR_BAD_PRD);
        return -EFAULT;
    }
//    JDEC_SET_STATUS(drvinfo, STA_DECODEING);
    ret = wmt_jdec_decode_proc(drvinfo);    

    return ret;
} /* End of ioctl_decode_proc() */

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
static int copy_frame(jdec_drvinfo_t *drvinfo, jdec_frameinfo_t *frame_info)
{
    memcpy(frame_info, &drvinfo->arg_out, sizeof(jdec_frameinfo_t));

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

    drvinfo->_segment = JDEC_SEG_NONE;
    drvinfo->_set_attr_ready = 0;

    up(&jdec_decoding_sema); 

    return 0;    
} /* End of copy_frame() */

/*!*************************************************************************
* ioctl_decode_finish
* 
* Private Function by Willy Chuang, 2008/11/25
*/
/*!
* \brief
*    Open JPEG hardware
*
* \retval  0 if success
*/ 
int ioctl_decode_finish(jdec_drvinfo_t *drvinfo, jdec_frameinfo_t *frame_info)
{
    int ret = -EINVAL;
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_FINISH"
    ==========================================================================*/
    if( drvinfo->_set_attr_ready != 1){
        DBG_ERR("Decode fail without set attr ready!\n");
    }
    else {
        memcpy(&drvinfo->arg_out, frame_info, sizeof(jdec_frameinfo_t));

        ret = wmt_jdec_decode_finish(drvinfo);
    }
    copy_frame(drvinfo, frame_info);

	if(drvinfo->_is_hw_init && !is_mjpeg_run){
        // Jason: close is needed only for JPEG.
        // close will be called when release 
        // if driver uses as MJPEG
        wmt_jdec_close(drvinfo);
        DBG_MSG("JPEG uninit HW in finish.\n");
	}

    return ret;
} /* End of ioctl_decode_finish() */

/*!*************************************************************************
* ioctl_decode_flush
* 
* Private Function by Willy Chuang, 2009/12/03
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int ioctl_decode_flush(jdec_drvinfo_t *drvinfo, jdec_frameinfo_t *frame_info)
{
    int ret = -EINVAL;
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_FLUSH"
    ==========================================================================*/

    if( drvinfo->_set_attr_ready != 1){
        DBG_ERR("Decode fail without set attr ready!\n");
    }
    else {
        memcpy(&drvinfo->arg_out, frame_info, sizeof(jdec_frameinfo_t));

        ret = wmt_jdec_decode_flush(drvinfo);
    }
    copy_frame(drvinfo, frame_info);

	if(drvinfo->_is_hw_init && !is_mjpeg_run){
        // Jason: close is needed only for JPEG.
        // close will be called when release 
        // if driver uses as MJPEG
        wmt_jdec_close(drvinfo);
        DBG_MSG("JPEG uninit HW in flush.\n");
	}

    return ret;
} /* End of ioctl_decode_flush() */

/*!*************************************************************************
* ioctl_get_status
* 
* Private Function by Willy Chuang, 2010/06/14
*/
/*!
* \brief
*	Get driver status
*
* \retval  0 if success
*/ 
static int ioctl_get_status(jdec_drvinfo_t *drvinfo, jdec_status_t *jsts)
{
    int ret;
    
    ret = wmt_jdec_get_status(drvinfo, jsts);

    return ret;
} /* End of ioctl_get_status() */

/*!*************************************************************************
* jdec_ioctl
* 
* Private Function by Willy Chuang, 2008/12/2
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
static int jdec_ioctl(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg)
{
    int err = 0;
    int retval = -EINVAL;
    jdec_attr_t       attr;
    jdec_input_t      input;
    jdec_frameinfo_t  frame_info;
    jdec_capability_t capab;
    jdec_status_t     drvsts;
    unsigned long     flags =0;
    jdec_drvinfo_t   *drvinfo;
    
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
    spin_lock_irqsave(&jdec_lock, flags);
    drvinfo = (jdec_drvinfo_t *)filp->private_data;

    TRACE("drvinfo: %p, cmd: 0x%x\n", (void *)drvinfo, cmd);
    if( is_mjpeg_run && (drvinfo->_is_mjpeg == 0) ) {
        DBG_ERR("Cannot decode JPEG when MJPEG is playing!\n");
        goto EXIT_jdec_ioctl;
    }
    
	// allow to change current driver info only if in the same HW open
	if((cmd == VDIOSET_DECODE_INFO) || (cmd == VDIOSET_DECODE_PROC) ||
		(cmd == VDIOGET_DECODE_FINISH) || (cmd == VDIOGET_DECODE_FLUSH) ||
		(cmd == VDIOGET_DECODE_STATUS))
    wmt_jdec_set_drv(drvinfo);

    jdec_ref_cur = drvinfo->ref_num;

    switch(cmd){       
        case VDIOSET_DECODE_INFO:
            DBG_DETAIL("Receive IOCTL:VDIOSET_DECODE_INFO\n");
            retval = copy_from_user( &attr, (const void *)arg, 
                                      sizeof(jdec_attr_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOSET_DECODE_INFO!\n");
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&attr, jdec_attr_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOSET_DECODE_INFO\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", attr.identity, attr.size);
            }
            else 
#endif
            {
                spin_unlock_irqrestore(&jdec_lock, flags);
                                
                retval = ioctl_set_attr(drvinfo, &attr);
                                
                spin_lock_irqsave(&jdec_lock, flags);
            }
            break;

        case VDIOSET_DECODE_PROC:
            DBG_DETAIL("Receive IOCTL:VDIOSET_DECODE_PROC\n");
            retval = copy_from_user((jdec_input_t *)&input, 
                                  (const void *)arg, sizeof(jdec_input_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOSET_DECODE_PROC!\n");   
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&input, jdec_input_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOSET_DECODE_PROC\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", input.identity, input.size);
            }
            else
#endif
            {
    			if(((PAGE_ALIGN(input.src_size)/PAGE_SIZE)*8) > MAX_INPUT_BUF_SIZE){
	    			DBG_ERR("input size overflow. %x/%x\n",
		    			input.src_size, MAX_INPUT_BUF_SIZE*4096/8);
			    	return -EFAULT;
			    }
                /*--------------------------------------------------------------
                    Before HW decoding finish, we cannot decode another JPEG
                --------------------------------------------------------------*/

                spin_unlock_irqrestore(&jdec_lock, flags);
                                
                retval = ioctl_decode_proc(drvinfo, &input);
                                
                spin_lock_irqsave(&jdec_lock, flags);
            }
            break;

        case VDIOGET_DECODE_FINISH:
        case VDIOGET_DECODE_FLUSH:
            DBG_DETAIL("Receive IOCTL:VDIOGET_DECODE_FINISH\n");
            if( JDEC_GET_STATUS(drvinfo) ==  STA_ERR_BAD_PRD ) {
                retval = -1;
                break;
            }
            retval = copy_from_user((vd_ioctl_cmd*)&frame_info, (const void *)arg, 
                                   sizeof(vd_ioctl_cmd));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOGET_DECODE_FINISH!\n");   
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&frame_info, jdec_frameinfo_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOGET_DECODE_FINISH\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", frame_info.identity, frame_info.size);
                retval = -ENOTTY;
            }
            else 
#endif
            {
                spin_unlock_irqrestore(&jdec_lock, flags);
                if( cmd == VDIOGET_DECODE_FINISH ) {
                    retval = ioctl_decode_finish(drvinfo, &frame_info);
                }
                else {
                    retval = ioctl_decode_flush(drvinfo, &frame_info);
                }
                copy_to_user((jdec_frameinfo_t *)arg, &frame_info, 
                                           sizeof(jdec_frameinfo_t));
                spin_lock_irqsave(&jdec_lock, flags);
            }
            break;

        case VDIOGET_DECODE_STATUS:
            DBG_DETAIL("Receive IOCTL:VDIOGET_DECODE_STATUS\n");
            retval = copy_from_user((vd_ioctl_cmd*)&drvsts, (const void *)arg, 
                                   sizeof(jdec_status_t));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOGET_CAPABILITY!\n");   
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&drvsts, jdec_status_t, VD_JPEG);
            if( retval ) {
                DBG_ERR("Version conflict (0x%x) in VDIOGET_DECODE_STATUS\n", retval);
                DBG_MSG("identity: 0x%x, size:%d\n", drvsts.identity, drvsts.size);
                retval = -ENOTTY;
            }
            else
#endif
            {
                spin_unlock_irqrestore(&jdec_lock, flags);

                retval = ioctl_get_status(drvinfo, &drvsts);
                if(retval == 0) {
                    retval = copy_to_user((jdec_status_t *)arg, &drvsts, 
                                           sizeof(jdec_status_t));
                }
                spin_lock_irqsave(&jdec_lock, flags);
            }
            break;

        case VDIOGET_CAPABILITY:
		{
            jdec_drvinfo_t   info;
			drvinfo = &info;

            DBG_DETAIL("Receive IOCTL:VDIOGET_CAPABILITY\n");
            retval = copy_from_user((vd_ioctl_cmd*)&capab, (const void *)arg, 
                                   sizeof(vd_ioctl_cmd));
            if( retval ) {
                DBG_ERR("copy_from_user FAIL in VDIOGET_CAPABILITY!\n");   
                break;
            }
#ifdef CFG_VERSION_CHECK
            retval = IOCTL_CMD_CHECK(&capab, jdec_capability_t, VD_JPEG);
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
                    retval = copy_to_user((jdec_capability_t *)arg, &capab, 
                                           sizeof(jdec_capability_t));
                }
            }
            break;
        }

        default:
            DBG_ERR("Unknown IOCTL:0x%x in jdec_ioctl()!\n", cmd);
            retval = -EINVAL;
            break;
    } /* switch(cmd) */
EXIT_jdec_ioctl:    
    spin_unlock_irqrestore(&jdec_lock, flags);
     
    TRACE("Leave\n");

    return retval;
} /* End of jdec_ioctl() */

/*!*************************************************************************
* jdec_open
* 
* Private Function by Willy Chuang, 2008/12/2
*/
/*!
* \brief
*    It is called when driver is opened, and initial hardward and resource
* \parameter
*   inode   [IN] a pointer point to struct inode
*   filp    [IN] a pointer point to struct file
* \retval  0 if success
*/ 
static int jdec_open(struct inode *inode, struct file *filp)
{
    jdec_drvinfo_t *drvinfo;
    struct videodecoder_info *vd_info;
    int ret = 0;

    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Initial Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&jdec_lock);
    jpeg_dev_ref++;
    jpeg_serial_no++;
    jdec_ref_cur = jpeg_serial_no;
        
    /*--------------------------------------------------------------------------
        Step 2: Initial hardware decoder
    --------------------------------------------------------------------------*/
    drvinfo = kmalloc(sizeof(jdec_drvinfo_t), GFP_KERNEL);
    if( drvinfo == 0 ) {
        DBG_ERR("Allocate %d bytes fail!\n", sizeof(jdec_drvinfo_t));
        ret = -EBUSY;
        goto EXIT_jdec_open;
    }
    memset(drvinfo, 0, sizeof(jdec_drvinfo_t));
    /*--------------------------------------------------------------------------
        private_data from VD is "PRD table virtual & physical address"
    --------------------------------------------------------------------------*/
    vd_info = (struct videodecoder_info *)filp->private_data;
    
    drvinfo->prd_virt = (unsigned int)vd_info->prdt_virt;
    drvinfo->prd_addr = (unsigned int)vd_info->prdt_phys;

    DBG_DETAIL("prd_virt: 0x%x, prd_addr: 0x%x\n", drvinfo->prd_virt, drvinfo->prd_addr);
    
    /* change privata_data to drvinfo */
    filp->private_data = drvinfo;
    drvinfo->size      = sizeof(jdec_drvinfo_t);
    drvinfo->_lock     = jdec_lock;
    drvinfo->ref_num   = jpeg_dev_ref;
    drvinfo->_segment  = JDEC_SEG_NONE;
    drvinfo->_is_mjpeg = 0;
	drvinfo->_is_hw_init = 0;

    JDEC_SET_STATUS(drvinfo, STA_READY);

// Jason: HW open has been moved to ioctl_set_attr for multi-open
// scenario:
//    [JPEG] - 1. HW open when ioctl_set_attr
//             2. HW close when ioctl_decode_finish or ioctl_decode_flush
//             ** MSD is included because no additional ioctl_set_attr will be called 
//                and no additional ioctl_decode_finish will be called btw segments.
//             ** Partial decode is included because each partial decode is considered
//                individual jpeg decode
//   [MJPEG] - 1. HW open when first time ioctl_set_attr
//             2. HW close when device is closed.
#if 0
    if(jpeg_dev_ref == 1) {
        ret = wmt_jdec_open(drvinfo);
    }
#endif

EXIT_jdec_open:
    spin_unlock(&jdec_lock);
    TRACE("Leave\n");
    
    return ret;
} /* End of jdec_open() */

/*!*************************************************************************
* jdec_release
* 
* Private Function by Willy Chuang, 2008/12/2
*/
/*!
* \brief
*       It is called when driver is released, and reset hardward and free resource
* \parameter
*   inode   [IN] a pointer point to struct inode
*   filp    [IN] a pointer point to struct file
* \retval  0 if success
*/ 
static int jdec_release(struct inode *inode, struct file *filp)
{
    jdec_drvinfo_t *drvinfo = (jdec_drvinfo_t *)filp->private_data;

    TRACE("Enter\n");
    
    if(jpeg_dev_ref == 0){
        DBG_ERR("jdec_release() is called without being opened!\n");
        return 0;
    }
    spin_lock(&jdec_lock);
    /*--------------------------------------------------------------------------
        Step 1: Close hardware decoder
    --------------------------------------------------------------------------*/
#if 0 // Jason: see jdec_open
    if(jpeg_dev_ref == 1) {
        wmt_jdec_close(drvinfo);
    }
#endif
    jpeg_dev_ref--;
    
    JDEC_SET_STATUS(drvinfo, STA_CLOSE);

    /*--------------------------------------------------------------------------
        Step 2: Release Linux settings
    --------------------------------------------------------------------------*/
#if 0
    spin_lock(&jdec_lock);
    if(!jpeg_dev_ref) {
        spin_unlock(&jdec_lock);
        return -EBUSY;
    }   
//    jpeg_dev_ref--;
#endif
    if( drvinfo->_is_mjpeg == 1 ) {
        // Jason: close is needed only for MJPEG.
        // close has been called when finish or flush
        // if driver uses as JPEG
        wmt_jdec_close(drvinfo);
        DBG_MSG("MJPEG uninit HW.\n");
        is_mjpeg_run = 0;
    }

    if( filp->private_data ) {
        kfree(filp->private_data);
    }

    spin_unlock(&jdec_lock);
    module_put(THIS_MODULE);

    TRACE("Leave\n");

    return 0;
} /* End of jdec_release() */

/*!*************************************************************************
* jdec_probe
* 
* Private Function by Willy Chuang, 2008/11/18
*/
/*!
* \brief
*       Probe
* \retval  0 if success
*/ 
static int jdec_probe(void /*struct device *dev*/)
{
    TRACE("Enter\n");

    spin_lock_init(&jdec_lock);

    wmt_jdec_probe();

    TRACE("Leave\n");

    return 0;
} /* End of jdec_probe() */

/*!*************************************************************************
* jdec_remove
* 
* Private Function by Willy Chuang, 2008/11/18
*/
/*!
* \brief
*       remove jpeg module
* \retval  0 if success
*/ 
static int jdec_remove(void)
{   
    TRACE("Enter\n");
    TRACE("Leave\n");

    return 0;
} /* End of jdec_remove() */

/*!*************************************************************************
* jdec_suspend
* 
* Private Function by Willy Chuang, 2008/11/18
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
static int jdec_suspend(pm_message_t state)
{
    TRACE("Enter\n");
    switch (state.event) {
        case PM_EVENT_SUSPEND:
        case PM_EVENT_FREEZE:
        case PM_EVENT_PRETHAW:
        default:
            wmt_jdec_suspend();
            break;	
    }
    TRACE("Leave\n");
    
    return 0;
} /* End of jdec_suspend() */

/*!*************************************************************************
* jdec_resume
* 
* Private Function by Willy Chuang, 2008/11/18
*/
/*!
* \brief
*       resume jpeg module
* \retval  0 if success
*/ 
static int jdec_resume(void)
{
    TRACE("Enter\n");
   
    wmt_jdec_resume();

    TRACE("Leave\n");

    return 0;
} /* End of jdec_resume() */

/*!*************************************************************************
* jdec_get_info
* 
* Private Function by Willy Chuang, 2008/12/23
*/
/*!
* \brief
*       resume jpeg module
* \retval  0 if success
*/ 
#ifdef CONFIG_PROC_FS
static int jdec_get_info(char *buf, char **start, off_t offset, int len)
{
    return wmt_jdec_get_info(buf, start, offset, len);
} /* End of jdec_get_info() */
#endif

/*!*************************************************************************
* jdec_setup
* 
* Private Function by Willy Chuang, 2008/12/23
*/
/*!
* \brief
*       
* \retval  0 if success
*/
static int jdec_setup(void)
{
    return jdec_probe();
} /* End of jdec_setup() */

/*!*************************************************************************
    platform device struct define
****************************************************************************/

struct videodecoder jpeg_decoder = {
    .name    = DRIVER_NAME,
    .id      = VD_JPEG,
    .setup   = jdec_setup,
    .remove  = jdec_remove,
    .suspend = jdec_suspend,
    .resume  = jdec_resume,
    .fops = {
                .owner   = THIS_MODULE,
                .open    = jdec_open,
                //.read    = jdec_read,
                //.write   = jdec_write,
                .ioctl   = jdec_ioctl,
                //.mmap    = jdec_mmap,
                .release = jdec_release,
            },
#ifdef CONFIG_PROC_FS
    .get_info = jdec_get_info,
#endif /* CONFIG_PROC_FS */
    .device = NULL,
};

/*!*************************************************************************
* hw_jdec_init
* 
* Private Function by Willy Chuang, 2008/11/18
*/
/*!
* \brief
*       init jpeg module
* \retval  0 if success
*/ 
static int hw_jdec_init(void)
{
    int ret;

    TRACE("Enter\n");

    ret = videodecoder_register(&jpeg_decoder);
    
    TRACE("Leave\n");

    return ret;
} /* End of hw_jdec_init() */

module_init(hw_jdec_init);

/*!*************************************************************************
* hw_jdec_exit
* 
* Private Function by Willy Chuang, 2008/11/18
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static void hw_jdec_exit(void)
{
    TRACE("Enter\n");

    videodecoder_unregister(&jpeg_decoder);

    TRACE("Leave\n");

    return;
} /* End of hw_jdec_exit() */

module_exit(hw_jdec_exit);

MODULE_AUTHOR("WonderMedia Multimedia SW Team Willy Chuang");
MODULE_DESCRIPTION("WMT JPEG decode device driver");
MODULE_LICENSE("GPL");

/*------------------------------------------------------------------------------*/
/*--------------------End of Function Body -----------------------------------*/

#undef HW_JDEC_C
