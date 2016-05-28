/*++ 
 * drivers\media\video\wmt\jpeg\wm8605-jdec.c 
 * WonderMedia WM8605 SoC hardware JPEG decoder driver
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

#define WM8605_JDEC_C

/*--- wm8605-jdec.c ---------------------------------------------------------------
*   Copyright (C) 2009 WonderMedia Tech. Inc.
*
* MODULE       : wm8605-jdec.c
* AUTHOR       : Willy Chuang
* DATE         : 2009/08/03
* DESCRIPTION  : 
*-----------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Willy Chuang, 2009/08/03
*	First version
*
*------------------------------------------------------------------------------*/

#include "hw-jdec.h"
#include "wmt-jdec.h"
#include "../wmt-dsplib.h"

//#define JDEC_PRD_DEBUG
#ifdef JDEC_PRD_DEBUG
   #define DBG_PRD(fmt, args...)     PRINT("{%s} " fmt, __FUNCTION__ , ## args)
#else
   #define DBG_PRD(fmt, args...)
#endif


#define ALIGN64(a)          (((a)+63) & (~63))
#define MAX(a, b)           ((a)>=(b))?(a):(b)

static jdec_drvinfo_t *current_drv = 0;

static jdec_capability_t capab_table = {
    /* identity */
    MAKE_IDENTITY(VD_JPEG, JDEC_VERSION),
    sizeof(jdec_capability_t), /* size */
    
    8605, /* chip id */
    
    1, /* baseline:1 */
    0, /* progressive:1 */
    1, /* int graylevel:1 */   
    1, /* int partial_decode:1 */
    1, /* decoded_to_YC420:1; */
    1, /* decoded_to_YC422H:1 */
    1, /* decoded_to_YC444:1 */
    1, /* decoded_to_ARGB:1 */
    1, /* split write: 1 */
    0, /* reseverd:23 */
    
    4, /* scale_ratio_num */
    {1,2,4,8,0,0,0,0} /* scale_fator[8] */
};

#ifdef __KERNEL__
DECLARE_WAIT_QUEUE_HEAD(jdec_wait);
#endif

/*!*************************************************************************
* wait_decode_finish
* 
* Private Function by Willy Chuang, 2008/12/3
*/
/*!
* \brief
*	Wait HW decoder finish job
*
* \retval  none
*/ 
static int wait_decode_finish(jdec_drvinfo_t *drv)
{
    dsplib_mbox_t mbox;
    int ret = 0;
	dsp_done_t *dspinfo;
    
    /*--------------------------------------------------------------------------
        LOOK OUT: STA_DMA_MOVE_DONE != STA_DECODE_DONE
                  Sometimes, a large pictuer will be individed into many samll 
                  segment to decode.
    --------------------------------------------------------------------------*/
    if((drv->_status & STA_ERR_DMA_TX) || (drv->_status & STA_ERR_UNKNOWN)){
        DBG_ERR("Unexcepted status: 0x%x\n", drv->_status);
        return -1;
    }
    if( drv->_status & STA_DECODE_DONE ){
        /* This frame was decoded finished */
        return 0;
    }
    /*--------------------------------------------------------------------------
        Step 5: Send decoding info to DSP
    --------------------------------------------------------------------------*/
    mbox.cmd = CMD_D2A_VDEC_DONE;
    
    DBG_MSG("Send Wait finsih CMD: \n");
    DBG_MSG("mbox.cmd: 0x%x\n", mbox.cmd);

    ret = dsplib_cmd_recv(&mbox,CMD_D2A_VDEC_DONE);
	dspinfo = (dsp_done_t *)phys_to_virt(mbox.w1);

    if( dspinfo->decode_status != 0 ) {
        /* DSP return error */
        DBG_ERR("Some error happend in DSP (code: %d)\n", dspinfo->decode_status);
        ret = -1;
    }
    else {
        drv->_status |= STA_DECODE_DONE;
        drv->arg_in._sts |= JDEC_STS_DECODE_DONE;
    }
    return ret;
} /* End of wait_decode_finish() */

/*!*************************************************************************
* wmt_jdec_seg_done
* 
* API Function by Willy Chuang, 2010/06/14
*/
/*!
* \brief
*	Callback function of DPS for segment done
*
* \retval  0 if success
*/ 
int wmt_jdec_seg_done(void)
{
    jdec_drvinfo_t *drv;

    drv = wmt_jdec_get_drv();
    if( drv ) {    
        drv->_segment_done++;
        drv->_segment_no++;
    
        drv->_status &= (~STA_DMA_START);
        drv->_status |= STA_DMA_MOVE_DONE;

        DBG_MSG("Segment Done(_segment_no: %d, _segment_done: %d)\n", drv->_segment_no, drv->_segment_done);    
    }
    return 0;
} /* End of wmt_jdec_seg_done() */

/*!*************************************************************************
* wmt_jdec_decode_done
* 
* API Function by Willy Chuang, 2010/06/24
*/
/*!
* \brief
*	Callback function of DPS for segment done
*
* \retval  0 if success
*/ 
int wmt_jdec_decode_done(void)
{
    jdec_drvinfo_t *drv;

    drv = wmt_jdec_get_drv();

    DBG_MSG("Decode Done\n");
    
    if( drv ) {
        drv->_status |= STA_DECODE_DONE;
    }
    return 0;
} /* End of wmt_jdec_decode_done() */

/*!*************************************************************************
* wmt_jdec_set_drv
* 
* API Function by Willy Chuang, 2009/9/29
*/
/*!
* \brief
*	Set current JPEG driver objet
*
* \retval  0 if success
*/ 
int wmt_jdec_set_drv(jdec_drvinfo_t *drv)
{
    current_drv = drv;
    
    return 0;
} /* End of wmt_jdec_set_drv() */

/*!*************************************************************************
* wmt_jdec_get_drv
* 
* API Function by Willy Chuang, 2009/9/29
*/
/*!
* \brief
*	get current JPEG driver objet
*
* \retval  0 if success
*/ 
jdec_drvinfo_t * wmt_jdec_get_drv(void)
{
    return current_drv;
} /* End of wmt_jdec_get_drv() */

/*!*************************************************************************
* wmt_jdec_get_capability
* 
* API Function by Willy Chuang, 2008/12/5
*/
/*!
* \brief
*	Get JPEG hardware capability
*
* \retval  0 if success
*/ 
int wmt_jdec_get_capability(jdec_drvinfo_t *drv)
{
    drv->capab_tbl = &capab_table;

    return 0;
} /* End of wmt_jdec_get_capability() */

/*!*************************************************************************
* wmt_jdec_set_attr
* 
* API Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*	JPEG hardware initialization
*
* \retval  0 if success
*/ 
int wmt_jdec_set_attr(jdec_drvinfo_t *drv)
{
    jdec_hdr_info_t    *hdr;
    jdec_decode_info_t *di;
    vdo_color_fmt       src_color;
    vdo_color_fmt       dst_color;
    unsigned int        scanline;
    int factor_c;
    
    TRACE("Enter\n");
    /*==========================================================================
        When this function is called, it means a new JPEG picture will come.
        In this case, we must stop old jobs and do reset.
    ==========================================================================*/
    hdr = &drv->attr.hdr;
    di  = &drv->attr.di;
    
    drv->_status = STA_ATTR_SET;

    /*--------------------------------------------------------------------------
        Step 1: Set core settings
    --------------------------------------------------------------------------*/
    src_color = hdr->src_color;
    dst_color = di->decoded_color;

    DBG_MSG("dst_color(%d)\n", dst_color);
    
    /*--------------------------------------------------------------------------
        Step 2: Partial decode settings 
    --------------------------------------------------------------------------*/

    /*--------------------------------------------------------------------------
        Step 3: Picture scaling control 
    --------------------------------------------------------------------------*/
    switch(di->scale_factor) {
        case SCALE_BEST_FIT:
        case SCALE_ORIGINAL:
            drv->scale_ratio = 0;
            break;
        case SCALE_HALF:
            drv->scale_ratio = 1;
            break;
        case SCALE_QUARTER:
            drv->scale_ratio = 2;
            break;
        case SCALE_EIGHTH:
            drv->scale_ratio = 3;
            break;
        default:
            DBG_ERR("Illegal scale ratio(%d)\n", di->scale_factor);
            drv->scale_ratio = 0;
            break;
    }
    /*--------------------------------------------------------------------------
        Step 4: Handle scanline offset issue
    --------------------------------------------------------------------------*/
    if(di->pd.enable) {
        drv->decoded_w = di->pd.w;
        drv->decoded_h = di->pd.h;
    }
    else {
        drv->decoded_w = hdr->sof_w;
        drv->decoded_h = hdr->sof_h;
    }
    scanline = drv->decoded_w;
    
    
    /* Because of HW limitation, Y line width should be multiple of 64. */
    switch(dst_color) {
        case VDO_COL_FMT_YUV420:
            drv->line_width_y = ALIGN64( ALIGN64(scanline) >> drv->scale_ratio );
            drv->line_width_y = ALIGN64( MAX(drv->line_width_y, di->scanline));
            drv->line_width_c = drv->line_width_y;
            drv->bpp = 8;
            factor_c = 2;
            break;
        case VDO_COL_FMT_YUV422H:
            drv->line_width_y = ALIGN64( ALIGN64(scanline) >> drv->scale_ratio );
            drv->line_width_y = ALIGN64( MAX(drv->line_width_y, di->scanline));
            drv->line_width_c = drv->line_width_y;
            drv->bpp = 8;
            factor_c = 1;
            break;
        case VDO_COL_FMT_YUV422V:
            drv->line_width_y = ALIGN64( ALIGN64(scanline) >> drv->scale_ratio );
            drv->line_width_y = ALIGN64( MAX(drv->line_width_y, di->scanline));
            drv->line_width_c = drv->line_width_y << 1;
            drv->bpp = 8;
            factor_c = 2;
            break;
        case VDO_COL_FMT_YUV444:
            drv->line_width_y = ALIGN64( ALIGN64(scanline) >> drv->scale_ratio );
            drv->line_width_y = ALIGN64( MAX(drv->line_width_y, di->scanline));
            drv->line_width_c = drv->line_width_y << 1;
            drv->bpp = 8;
            factor_c = 1;
            break;
        case VDO_COL_FMT_YUV411:
            drv->line_width_c = ALIGN64( ALIGN64(scanline) >> drv->scale_ratio );
            drv->line_width_c = ALIGN64( MAX(drv->line_width_c, di->scanline));
            drv->line_width_y = drv->line_width_c << 1;
            drv->bpp = 8;
            factor_c = 2;
            break;
        case VDO_COL_FMT_ARGB:
            drv->line_width_y = ALIGN64( ALIGN64(scanline << 2) >> drv->scale_ratio );
            drv->line_width_y = ALIGN64( MAX(drv->line_width_y, di->scanline));
            drv->line_width_c = 0;
            drv->bpp = 32;
            factor_c = 1;
            break;
        case VDO_COL_FMT_GRAY:
            drv->line_width_y = ALIGN64( ALIGN64(scanline) >> drv->scale_ratio );
            drv->line_width_y = ALIGN64( MAX(drv->line_width_y, di->scanline));
            drv->line_width_c = 0;
            drv->bpp = 8;
            factor_c = 1;
            break;
        default:
            DBG_ERR("Unknown color format(%d)\n", dst_color);
            factor_c = 1; // to avoid divid zero
            break;
    }
    DBG_MSG("sof_w: %d, sof_h: %d\n", hdr->sof_w, hdr->sof_h);
    DBG_MSG("line_width_y: %d, line_width_c: %d\n", drv->line_width_y, drv->line_width_c);

    drv->line_height = (((drv->decoded_h + 15)>>4)<<4) >> drv->scale_ratio;
    drv->req_y_size  = (drv->line_width_y * drv->line_height);
    drv->req_c_size  = (drv->line_width_c * drv->line_height)/ factor_c;

    DBG_MSG("Required memory size(Y, C): (%d, %d)\n", drv->req_y_size, drv->req_c_size);

    /*--------------------------------------------------------------------------
        Step 5: Send Attr to DSP
    --------------------------------------------------------------------------*/
    DBG_MSG("===> decoded_color: %d\n", drv->attr.di.decoded_color );  

    dsplib_cmd_send(CMD_A2D_VDEC_ATTR, (unsigned int)virt_to_phys(&drv->attr),
                    sizeof(drv->attr), 0);

    TRACE("Leave\n");
    
    return 0;
} /* End of wmt_jdec_set_attr() */

/*!*************************************************************************
* wmt_jdec_decode_proc
* 
* API Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jdec_decode_proc(jdec_drvinfo_t *drv)
{
    unsigned int  prd_addr_in;
    unsigned int  y_base, c_base;
    jdec_input_t *arg;
    jdec_attr_t  *attr;

    static unsigned int s_pph_buf[5];
    unsigned int  pph_phy;
	static dsp_done_t  dspinfo;
	unsigned int dsp_phy;
    int i, ret = 0;

   
    TRACE("Enter\n");
	DBG_MSG("dspinfo: 0x%x\n",&dspinfo);
    /*--------------------------------------------------------------------------
        Step 1: Check input arguments
    --------------------------------------------------------------------------*/
    arg = &drv->arg_in;
    attr = &drv->attr;

    while( drv->_status & STA_DMA_START) {
        DBG_ERR("BSDMA is busy now!\n");
    }
    
    DBG_PRD("Dma prd virt addr: 0x%08x\n", drv->prd_virt);
    DBG_PRD("Dma prd phy  addr: 0x%08x\n", drv->prd_addr);
    DBG_PRD("filesize: %d\n", attr->hdr.filesize );  

    /*--------------------------------------------------------------------------
        Step 2: Set PRD table
    --------------------------------------------------------------------------*/
    prd_addr_in = drv->prd_virt;
    for(i=0; ; i+=2) {
        unsigned int  addr, len;

        addr = *(unsigned int *)(prd_addr_in + i * 4);
        len  = *(unsigned int *)(prd_addr_in + (i + 1) * 4);

        DBG_PRD("[%02d]Addr: 0x%08x\n", i, addr);
        DBG_PRD("    Len:  0x%08x (%d)\n", len, (len & 0xFFFF));

#ifdef JDEC_PRD_DEBUG
{
    unsigned char *ptr;
    int j;  

    if(i==0) {
        ptr = (unsigned char *)phys_to_virt(addr);
        for(j=0; j<64; j++ ){
            if((j%16) == 15) {
                printk("0x%02x\n", *ptr);
            }
            else {
                printk("0x%02x ", *ptr);
            }
            ptr++;
        }
    }
}    
#endif  
        if(len & 0x80000000) {
            if((drv->_multi_seg_en) && !(drv->_segment & JDEC_SEG_LAST) ) {
                *(unsigned int *)(prd_addr_in + (i + 1) * 4) = ((len & 0x7FFFFFFF) | 0x40000000);
            }
            break;
        } /* if(len & 0x80000000) */
    } /* for(i=0; ; i+=2) */

    DBG_MSG("PRD_ADDR: 0x%x \n", drv->prd_addr);
    
    /*--------------------------------------------------------------------------
        Step 3: Check whether space of decoded buffer is enough or not
    --------------------------------------------------------------------------*/
    if(attr->di.decoded_color != VDO_COL_FMT_ARGB) {
        /* YUV color domain */
        if(arg->dst_y_size < drv->req_y_size) {
            DBG_ERR("Decoded buffer size (Y) is not enough (%d < %d)\n", 
                                        arg->dst_y_size, drv->req_y_size);
            goto EXIT_wmt_jdec_decode_proc;
        }
        if(arg->dst_c_size < drv->req_c_size) {
            DBG_ERR("Decoded buffer size (C) is not enough (%d < %d)\n", 
                                        arg->dst_c_size, drv->req_c_size);
            goto EXIT_wmt_jdec_decode_proc;
        }
    }
    else {
        /* RGB color domain */
        if(arg->dst_y_size < drv->req_y_size) {
            DBG_ERR("Decoded buffer size (ARGB) is not enough (%d < %d)\n", 
                                        arg->dst_y_size, drv->req_y_size);
            DBG_ERR("line_width_y: %d, line_height: %d\n", drv->line_width_y, drv->line_height);
                        
            goto EXIT_wmt_jdec_decode_proc;
        }
    }    
    if(arg->dst_y_addr == 0) {
        DBG_ERR("NULL Y Addr! Stop decoding!\n");
        goto EXIT_wmt_jdec_decode_proc;
    }
/*    if(arg->dst_c_addr == 0) {
        DBG_MSG("Warning: C Addr is NULL!\n");
    }    */
    /*--------------------------------------------------------------------------
        Step 4: 
    --------------------------------------------------------------------------*/
    if((arg->linesize !=0) && (arg->linesize > drv->line_width_y)) {
        DBG_MSG("linesize: %d\n", arg->linesize);
        /* In this case, we only support 4:2:0 mode */
        drv->line_width_y = arg->linesize;
        drv->line_width_c = drv->line_width_y >> 1; // temp
    }
    
    /* Physical address */
    y_base = arg->dst_y_addr;
    c_base = arg->dst_c_addr;

    if((arg->dec_to_x == 0) && ((arg->dec_to_y == 0))) {
        /* Decode to start address of destination buffer */
    }
    else {
        /* Decode to any postion of destination buffer */
        if(drv->attr.di.decoded_color == VDO_COL_FMT_YUV422H) {
            y_base += arg->dec_to_y * drv->line_width_y + arg->dec_to_x;
            c_base += arg->dec_to_y * drv->line_width_c + arg->dec_to_x;
            DBG_MSG("[422H] Phy. Y addr changed from: 0x%x to 0x%x\n", arg->dst_y_addr, y_base);
            DBG_MSG("[422H] Phy. C addr changed from: 0x%x to 0x%x\n", arg->dst_c_addr, c_base);
        }
        else if(drv->attr.di.decoded_color == VDO_COL_FMT_YUV444) {
            y_base += arg->dec_to_y * drv->line_width_y + arg->dec_to_x;
            c_base += arg->dec_to_y * drv->line_width_c + (arg->dec_to_x << 1);
            DBG_MSG("[444] Phy. Y addr changed from: 0x%x to 0x%x\n", arg->dst_y_addr, y_base);
            DBG_MSG("[444 Phy. C addr changed from: 0x%x to 0x%x\n", arg->dst_c_addr, c_base);
        }
        else if(drv->attr.di.decoded_color == VDO_COL_FMT_ARGB) {
            y_base += arg->dec_to_y * drv->line_width_y + (arg->dec_to_x << 2);
            c_base = 0;
            DBG_MSG("[ARGB] Phy. Y addr changed from: 0x%x to 0x%x\n", arg->dst_y_addr, y_base);
            DBG_MSG("[ARGB] Phy. C addr changed from: 0x%x to 0x%x\n", arg->dst_c_addr, c_base);
        }
        if(drv->attr.di.decoded_color == VDO_COL_FMT_YUV420) {
            y_base += arg->dec_to_y * drv->line_width_y + arg->dec_to_x;
            c_base += (arg->dec_to_y >> 1) * drv->line_width_c + arg->dec_to_x;
            DBG_MSG("[420] Phy. Y addr changed from: 0x%x to 0x%x\n", arg->dst_y_addr, y_base);
            DBG_MSG("[420] Phy. C addr changed from: 0x%x to 0x%x\n", arg->dst_c_addr, c_base);
        }
        else {
            DBG_MSG("decoded_color: %d\n", drv->attr.di.decoded_color);
        }
    }
    /*--------------------------------------------------------------------------
        Step 4: Interrupt enable
                bit 4: JPEG bitstream ERROR INT enable
                bit 3: JBSDMA INT enable
                bit 2: JPEG HW parsing SOF INT enable
                bit 1: VLD decode finish INT enable
                bit 0: JPEG decoded frame out finish INT enable
    --------------------------------------------------------------------------*/
    drv->_status = STA_DMA_START;

    /*--------------------------------------------------------------------------
        Step 5: Send decoding info to DSP
    --------------------------------------------------------------------------*/
    if( (drv->_multi_seg_en == 0) ||
        ((drv->_multi_seg_en) && (drv->_segment & JDEC_SEG_FIRST)) ){
        s_pph_buf[0] = y_base;
        s_pph_buf[1] = drv->line_width_y;
        s_pph_buf[2] = c_base;
        s_pph_buf[3] = drv->line_width_c;
    
        pph_phy = (unsigned int)virt_to_phys(s_pph_buf);
		dsp_phy = (unsigned int)virt_to_phys(&dspinfo);
		DBG_MSG("dspinfo: 0x%x\n",dsp_phy);

        dsplib_cmd_send(CMD_A2D_VDEC, drv->prd_addr, pph_phy, dsp_phy); 
    }
    else {
        dsplib_cmd_send(CMD_A2D_VDEC_NEW_SEG, drv->prd_addr, 0, 0); 
    }

EXIT_wmt_jdec_decode_proc:
    TRACE("Leave\n");

    return ret;
} /* End of wmt_jdec_decode_proc() */

/*!*************************************************************************
* set_frame_info
* 
* API Function by Willy Chuang, 2009/12/03
*/
/*!
* \brief
*	Set frame buffer information
*
* \retval  0 if success
*/ 
static int set_frame_info(jdec_drvinfo_t *drv)
{
    jdec_frameinfo_t *frame_info;
    jdec_attr_t      *attr;
    unsigned int      y_base, c_base;

    
    frame_info = &drv->arg_out;
    attr = &drv->attr;
    
    /* Physical address */
    y_base = drv->arg_in.dst_y_addr;
    c_base = drv->arg_in.dst_c_addr;
          
    /*--------------------------------------------------------------------------
        Step 2: Set information for application 
    --------------------------------------------------------------------------*/
    frame_info->fb.img_w   = drv->decoded_w / (1 << drv->scale_ratio);
    frame_info->fb.img_h   = drv->decoded_h / (1 << drv->scale_ratio);
    frame_info->fb.fb_w    = drv->line_width_y/(drv->bpp >> 3);
    frame_info->fb.fb_h    = drv->line_height;
    frame_info->fb.col_fmt = attr->di.decoded_color;
    frame_info->fb.bpp     = drv->bpp;
    frame_info->fb.h_crop  = 0;
    frame_info->fb.v_crop  = 0;
    frame_info->scaled     = 0;

    if(frame_info->fb.col_fmt != VDO_COL_FMT_ARGB) {
        /* YUV color domain */
        frame_info->fb.y_addr    = drv->arg_in.dst_y_addr;
        frame_info->fb.y_size    = drv->arg_in.dst_y_size;
        frame_info->fb.c_addr    = drv->arg_in.dst_c_addr;
        frame_info->fb.c_size    = drv->arg_in.dst_c_size;
        frame_info->y_addr_user  = drv->arg_in.dst_y_addr_user;
        frame_info->c_addr_user  = drv->arg_in.dst_c_addr_user;
    }
    else {
        /* RGB color domain */
        frame_info->fb.y_addr    = drv->arg_in.dst_y_addr;
        frame_info->fb.y_size    = drv->arg_in.dst_y_size;
        frame_info->fb.c_addr    = 0;
        frame_info->fb.c_size    = 0;
        frame_info->y_addr_user  = drv->arg_in.dst_y_addr_user;
        frame_info->c_addr_user  = 0;
    }
    if((attr->hdr.src_color == VDO_COL_FMT_GRAY) && 
       (attr->di.decoded_color != VDO_COL_FMT_ARGB)) {
        /* Draw C plane for gray level JPEG under YUV color space */
        void *vir_addr;
	    
    	vir_addr = (void *)phys_to_virt(frame_info->fb.c_addr);
        memset(vir_addr, 0x80, frame_info->fb.c_size);
    }    
    
    if((attr->di.scale_factor == SCALE_HALF) || (attr->di.scale_factor == SCALE_QUARTER)
        || (attr->di.scale_factor == SCALE_EIGHTH)){
        frame_info->scaled = attr->di.scale_factor;
    }
    return 0;
} /* End of set_frame_info()*/

/*!*************************************************************************
* wmt_jdec_decode_finish
* 
* API Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*	Wait JPEG hardware finished
*
* \retval  0 if success
*/ 
int wmt_jdec_decode_finish(jdec_drvinfo_t *drv)
{
    int   ret;
    
   /*==========================================================================
        In this function, we just wait for HW decoding finished.
    ==========================================================================*/
    /*--------------------------------------------------------------------------
        Step 1: Wait intrrupt (JDEC_INTRQ)
    --------------------------------------------------------------------------*/
    ret =  wait_decode_finish(drv);

    /*--------------------------------------------------------------------------
        Step 2: Set information for application 
    --------------------------------------------------------------------------*/
    set_frame_info(drv);

    return ret;
} /* End of wmt_jdec_decode_finish() */

/*!*************************************************************************
* wmt_jdec_decode_flush
* 
* API Function by Willy Chuang, 2009/12/03
*/
/*!
* \brief
*	Cancel JPEG decoding process
*
* \retval  0 if success
*/ 
int wmt_jdec_decode_flush(jdec_drvinfo_t *drv)
{
    /*--------------------------------------------------------------------------
        Step 1: Stop BSDMA
    --------------------------------------------------------------------------*/
//    stop_bsdam();
        
    /*--------------------------------------------------------------------------
        Step 2: Set information for application 
    --------------------------------------------------------------------------*/
    set_frame_info(drv);

    return 0;
} /* End of wmt_jdec_decode_flush() */

/*!*************************************************************************
* wmt_jdec_get_status
* 
* API Function by Willy Chuang, 2010/06/14
*/
/*!
* \brief
*	Get JPEG driver status
*
* \retval  0 if success
*/ 
int wmt_jdec_get_status(jdec_drvinfo_t *drv, jdec_status_t *jsts)
{    
    int ret = 0;

    jsts->sts_flag = 0;   
    if((jsts->flag_en & JDEC_STS_FLAG_MSD_DONE) && (drv->_multi_seg_en)) {
        do {
            if( drv->_segment_done ) {
                jsts->msd_seg_no = drv->_segment_no;
                jsts->sts_flag   = JDEC_STS_FLAG_MSD_DONE;
                drv->_segment_done--;
                break;
            }
            if( drv->_status & STA_DECODE_DONE ) {
                jsts->sts_flag |= JDEC_STS_FLAG_FRAME_DONE;
                break;
            }
            if(jsts->is_blocked == 0)
                break;
            /*------------------------------------------------------------------ 
                For last segment, we will receive FINISH_DONE istead of SEG_DONE
            ------------------------------------------------------------------*/
            if((jsts->is_blocked) && (drv->_segment & JDEC_SEG_LAST)){
                ret = wait_decode_finish(drv);
                if( drv->_status & STA_DECODE_DONE ){
                    jsts->sts_flag |= JDEC_STS_FLAG_FRAME_DONE;
                }
                break;
            }
            msleep(1);
        } while(1);
    }    
    return ret;
} /* End of wmt_jdec_get_status() */

/*!*************************************************************************
* wmt_jdec_suspend
* 
* API Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jdec_suspend(void)
{
    // TO DO
    return 0;
} /* End of wmt_jdec_suspend() */

/*!*************************************************************************
* wmt_jdec_resume
* 
* API Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jdec_resume(void)
{
    // TO DO
    return 0;
} /* End of wmt_jdec_resume() */

/*!*************************************************************************
* wmt_jdec_get_info
* 
* API Function by Willy Chuang, 2009/11/24
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jdec_get_info(char *buf, char **start, off_t offset, int len)
{
    jdec_drvinfo_t *drv;
    char *p = buf;

    p += sprintf(p, "WMT JDEC Start\n");

    drv = wmt_jdec_get_drv();
    if( drv == 0 ) {
        p += sprintf(p, "** JDEC driver was never opened **\n");
        goto EXIT_wmt_jdec_get_info;
    }

    p += sprintf(p, "**** [IN] jdec_attr_t ****\n");
    p += sprintf(p, "profile:    0x%08x\n", (drv->attr.hdr.profile & 0xFF));
    if( drv->attr.hdr.profile & DECFLAG_MJPEG_BIT) {
        p += sprintf(p, "-- MJPEG is playing\n");
    }
    p += sprintf(p, "sof_w:      %d\n", drv->attr.hdr.sof_w);
    p += sprintf(p, "sof_h:      %d\n", drv->attr.hdr.sof_h);
    p += sprintf(p, "filesize:   %d\n", drv->attr.hdr.filesize);
    p += sprintf(p, "src_color:  %d\n", drv->attr.hdr.src_color);
    p += sprintf(p, "avi1.type:  %d\n", drv->attr.hdr.avi1.type);
    p += sprintf(p, "field_size: %d\n", drv->attr.hdr.avi1.field_size);

    p += sprintf(p, "**** [IN] jdec_input_t ****\n");
    p += sprintf(p, "src_addr:   0x%08x\n", drv->arg_in.src_addr);
    p += sprintf(p, "src_size:   %d\n", drv->arg_in.src_size);
    p += sprintf(p, "flags:      0x%08x\n", drv->arg_in.flags);
    p += sprintf(p, "dst_y_addr: 0x%08x\n", drv->arg_in.dst_y_addr);
    p += sprintf(p, "dst_c_addr: 0x%08x\n", drv->arg_in.dst_c_addr);
    p += sprintf(p, "dst_y_size: %d\n", drv->arg_in.dst_y_size);
    p += sprintf(p, "dst_c_size: %d\n", drv->arg_in.dst_c_size);
    p += sprintf(p, "linesize:   %d\n", drv->arg_in.linesize);

    p += sprintf(p, "dst_y_addr_user: 0x%08x\n", drv->arg_in.dst_y_addr_user);
    p += sprintf(p, "dst_c_addr_user: 0x%08x\n", drv->arg_in.dst_c_addr_user);
    p += sprintf(p, "dec_to_x:   %d\n", drv->arg_in.dec_to_x);
    p += sprintf(p, "dec_to_y:   %d\n", drv->arg_in.dec_to_y);

    p += sprintf(p, "**** [OUT] jdec_frameinfo_t ****\n");
    p += sprintf(p, "y_addr:  0x%08x\n", drv->arg_out.fb.y_addr);
    p += sprintf(p, "c_addr:  0x%08x\n", drv->arg_out.fb.c_addr);
    p += sprintf(p, "y_size:  %d\n", drv->arg_out.fb.y_size);
    p += sprintf(p, "c_size:  %d\n", drv->arg_out.fb.c_size);
    p += sprintf(p, "img_w:   %d\n", drv->arg_out.fb.img_w);
    p += sprintf(p, "img_h:   %d\n", drv->arg_out.fb.img_h);
    p += sprintf(p, "fb_w:    %d\n", drv->arg_out.fb.fb_w);
    p += sprintf(p, "fb_h:    %d\n", drv->arg_out.fb.fb_h);

    p += sprintf(p, "bpp:     %d\n", drv->arg_out.fb.bpp);
    p += sprintf(p, "col_fmt: %d\n", drv->arg_out.fb.col_fmt);
    p += sprintf(p, "h_crop:  %d\n", drv->arg_out.fb.h_crop);
    p += sprintf(p, "v_crop:  %d\n", drv->arg_out.fb.v_crop);
    p += sprintf(p, "flag:    0x%08x\n", drv->arg_out.fb.flag);

    p += sprintf(p, "y_addr_user:  0x%08x\n", drv->arg_out.y_addr_user);
    p += sprintf(p, "c_addr_user:  0x%08x\n", drv->arg_out.c_addr_user);
    p += sprintf(p, "scaled:       %d\n", drv->arg_out.scaled);

    p += sprintf(p, "**** Variables ****\n");
    p += sprintf(p, "decoded_w:    %d\n", drv->decoded_w);
    p += sprintf(p, "decoded_h:    %d\n", drv->decoded_h);
    p += sprintf(p, "mcu_width:    %d\n", drv->src_mcu_width);
    p += sprintf(p, "mcu_height:   %d\n", drv->src_mcu_height);
    p += sprintf(p, "scale_ratio:  %d\n", drv->scale_ratio);

    p += sprintf(p, "prd_virt:     0x%08x\n", drv->prd_virt);
    p += sprintf(p, "prd_addr:     0x%08x\n", drv->prd_addr);
    p += sprintf(p, "line_width_y: %d\n", drv->line_width_y);
    p += sprintf(p, "line_width_c: %d\n", drv->line_width_c);
    p += sprintf(p, "bpp:          %d\n", drv->bpp);

    p += sprintf(p, "req_y_size:   %d\n", drv->req_y_size);
    p += sprintf(p, "req_c_size:   %d\n", drv->req_c_size);

    /* Following member is used for multi-tasks */
    p += sprintf(p, "ref_num:      %d\n", drv->ref_num);

    /* Following members are used for hw-jpeg.c only */
    p += sprintf(p, "timeout(ms):  %d\n", drv->_timeout);
    p += sprintf(p, "status:       0x%08x\n", drv->_status);
    p += sprintf(p, "multi_seg_en: %d\n", drv->_multi_seg_en);
    p += sprintf(p, "segment no:   %d\n", drv->_segment);
    p += sprintf(p, "remain_byte:  %d\n", drv->_remain_byte);
    p += sprintf(p, "_is_mjpeg:    %d\n", drv->_is_mjpeg);
EXIT_wmt_jdec_get_info:
    p += sprintf(p, "WMT JDEC End\n\n");

    return (p - buf);
} /* End of wmt_jdec_get_info() */

/*!*************************************************************************
* wmt_jdec_open
* 
* API Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*	Open JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jdec_open(jdec_drvinfo_t *drv)
{
    TRACE("Enter\n");

    /*--------------------------------------------------------------------------
        Step 1: Send decoding info to DSP
    --------------------------------------------------------------------------*/
    dsplib_cmd_send(CMD_A2D_VDEC_OPEN, VD_JPEG, 0, 0);

    TRACE("Leave\n");

    return 0;
} /* End of wmt_jdec_open() */

/*!*************************************************************************
* wmt_jdec_close
* 
* API Function by Willy Chuang, 2008/11/17
*/
/*!
* \brief
*	Close JPEG hardware
*
* \retval  0 if success
*/ 
int wmt_jdec_close(jdec_drvinfo_t *drv)
{
    TRACE("Enter\n");

    /*--------------------------------------------------------------------------
        Step 1: Send decoding info to DSP
    --------------------------------------------------------------------------*/
    dsplib_cmd_send(CMD_A2D_VDEC_CLOSE, VD_JPEG, 0, 0);

    TRACE("Leave\n");

    return 0;
} /* End of wmt_jdec_close() */

/*!*************************************************************************
* wmt_jdec_probe
* 
* API Function by Willy Chuang, 2009/5/22
*/
/*!
* \brief
*	Probe function
*
* \retval  0 if success
*/ 
int wmt_jdec_probe(void)
{
    TRACE("Enter\n");

    TRACE("Leave\n");

    return 0;
} /* End of wmt_jdec_probe() */

/*--------------------End of Function Body -----------------------------------*/

#undef WM8605_JDEC_C
