/*++ 
 * linux/drivers/media/video/wmt/.
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
 * 4F, 531, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/
/*------------------------------------------------------------------------------
*   Copyright (C) 2010 WonderMedia Tech. Inc.
*
* MODULE       : MPEG2 Video Decoder
* AUTHOR       : Welkin Chen
* DATE         : 2010/11/01
* DESCRIPTION  : All rights reserved.
*-----------------------------------------------------------------------------*/
/*--- History ------------------------------------------------------------------
* Version 0.01, Welkin Chen, 2009/06/12
* First version
*
* Version 0.1, Welkin Chen, 2010/11/01
*
*-----------------------------------------------------------------------------*/
/*----------------- MODULE DEPENDENCY ----------------------------------------*/
#ifndef WM865_MPEG2_C
#define WM865_MPEG2_C
/* To assert that only one occurrence is included */
//------------------------------------------------
//  Include Header
//------------------------------------------------
#include "../wmt-vd.h"
#include "../wmt-dsplib.h"
#include "../wmt-vdlib.h"
#include "wmt-mpeg2.h"

/*----------------------------------------------------------------------------*/
/*------------------------------------------------
    Debug Control
------------------------------------------------*/
#undef NAME 
#undef THIS_TRACE
#undef THIS_DEBUG
#undef THIS_DEBUG_DETAIL
#undef THIS_REG_TRACE

#define NAME MPEG2
//#define THIS_TRACE          NAME##_TRACE
//#define THIS_DEBUG          NAME##_DEBUG
//#define THIS_DEBUG_DETAIL   NAME##_DEBUG_DETAIL
//#define THIS_REG_TRACE      NAME##_REG_TRACE
/*------------------------------------------------
    Dump Message
------------------------------------------------*/
#ifdef __KERNEL__
  #define DPRINT  printk
#else
  #define DPRINT  printf
  #define KERN_EMERG
  #define KERN_ALERT
  #define KERN_CRIT
  #define KERN_ERR
  #define KERN_WARNING
  #define KERN_NOTICE
  #define KERN_INFO
  #define KERN_DEBUG
#endif

#define EMERG(fmt,args...)    DPRINT(KERN_EMERG   "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#define ALERT(fmt,args...)    DPRINT(KERN_ALERT   "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#define CRIT(fmt,args...)     DPRINT(KERN_CRIT    "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#define ERR(fmt,args...)      DPRINT(KERN_ERR     "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#define WARNING(fmt,args...)  DPRINT(KERN_WARNING "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#define NOTICE(fmt,args...)   DPRINT(KERN_NOTICE  "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#define INFO(fmt,args...)     DPRINT(KERN_INFO    "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#define DEBUG(fmt,args...)    DPRINT(KERN_DEBUG   "[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)

#ifdef THIS_DEBUG_DETAIL
  #define THIS_DEBUG          NAME##_DEBUG
  #define DBG_DETAIL(fmt, args...) DPRINT("[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#else
  #define DBG_DETAIL(fmt, args...)
#endif

#ifdef THIS_DEBUG
  #define DBG(fmt, args...)   DPRINT("[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#else
  #define DBG(fmt, args...)
#endif

#ifdef THIS_TRACE
  #define TRACE(fmt, args...) DPRINT("[%s](%d):" fmt, __FUNCTION__, __LINE__, ## args)
#else
  #define TRACE(fmt, args...)
#endif

//#define DSP_JUDGE
#ifdef DSP_JUDGE
#define DSPP(x...)          printk(KERN_ERR x)
//#define DSPP(fmt, args...)  printk(KERN_ERR fmt, __FUNCTION__ , ## args)
#else
#define DSPP(x...)
//#define DSPP(fmt, args...)
#endif

//#define TIME_JUDGE
#ifdef TIME_JUDGE
#define TIME(x...)          printk(KERN_ERR x)
//#define TIME(fmt, args...)  printk(KERN_ERR fmt, __FUNCTION__ , ## args)
#else
#define TIME(x...)
//#define TIME(fmt, args...)
#endif

#ifdef TIME_JUDGE
//==============================================
//Time Measure
#include <linux/time.h>
static unsigned int   mp2_total_tm;    /* total time */
static unsigned int   mp2_interval_tm; /* interval time */
static unsigned int   mp2_total_cnt;   /* total counter */
static unsigned int   mp2_count;       /* interval counter */
static unsigned int   mp2_reset;       /* reset counter */
static unsigned int   mp2_max;         /* max time */
static unsigned int   mp2_min;         /* min time */
static struct timeval mp2_start;       /* start time */
static struct timeval mp2_end;         /* end time */
static unsigned int   mp2_threshold;   /* us */
static struct timeval mp2_get_s;       /* start time */
static struct timeval mp2_get_e;       /* end time */
static int            mp2_this_time;
//==============================================
#endif
/*----------------------------------------------------------------------------*/

#define DEVICE_NAME "WMT-MPEG2"   /* appear in /proc/devices & /proc/wm-mpeg2 */
#define DRIVER_NAME "wmt-mpeg2"

#define VD_MB_PIXEL 16
#define DSP_OFFSET  64

#define MPEG2_GET_STATUS(drv)       (drv)->_status
#define MPEG2_SET_STATUS(drv, sta)  (drv)->_status = (sta)

static int mpeg2_dev_ref = 0; /* is device open */
static spinlock_t  mpeg2_lock;
DECLARE_MUTEX(mpeg2_decoding_sema);

static int frame_num_cnt = 0;

mpeg2_frameinfo_t mpeg2info;
/*----------------------------------------------------------------------------*/
/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Reset global variable
* \retval  N/A
*/
static void mpeg2_var_reset(void)
{
    //Reset Global variable
    frame_num_cnt = 0;
    return;
}

#if 0
static void mpeg2_hdr_dbg(void *hdraddr)
{
    mpeg2_hdr_info_t *hdr = (mpeg2_hdr_info_t *)hdraddr;
    ERR("horizontal_size_value                           %x\n", hdr->mpeg2_header.horizontal_size_value);
    ERR("vertical_size_value                             %x\n", hdr->mpeg2_header.vertical_size_value);
    ERR("aspect_ratio_information                        %x\n", hdr->mpeg2_header.aspect_ratio_information);
    ERR("frame_rate_code                                 %x\n", hdr->mpeg2_header.frame_rate_code);
    ERR("bit_rate_value                                  %x\n", hdr->mpeg2_header.bit_rate_value);
    ERR("marker_bit                                      %x\n", hdr->mpeg2_header.marker_bit);
    ERR("vbv_buffer_size_value                           %x\n", hdr->mpeg2_header.vbv_buffer_size_value);
    ERR("constrained_parameters_flag                     %x\n", hdr->mpeg2_header.constrained_parameters_flag);
    ERR("load_intra_quant_mat                            %x\n", hdr->mpeg2_header.load_intra_quant_mat);
    ERR("intra_quant_mat[64]                             %x\n", hdr->mpeg2_header.intra_quant_mat[64]);
    ERR("load_non_intra_quant_mat                        %x\n", hdr->mpeg2_header.load_non_intra_quant_mat);
    ERR("nonintra_quant_mat[64]                          %x\n", hdr->mpeg2_header.nonintra_quant_mat[64]);
    ERR("extension_start_code_identifier                 %x\n", hdr->mpeg2_header.extension_start_code_identifier);
    ERR("profile_and_level_indication                    %x\n", hdr->mpeg2_header.profile_and_level_indication);
    ERR("progressive_sequence                            %x\n", hdr->mpeg2_header.progressive_sequence);
    ERR("chroma_format                                   %x\n", hdr->mpeg2_header.chroma_format);
    ERR("horizontal_size_extension                       %x\n", hdr->mpeg2_header.horizontal_size_extension);
    ERR("vertical_size_extension                         %x\n", hdr->mpeg2_header.vertical_size_extension);
    ERR("bit_rate_extension                              %x\n", hdr->mpeg2_header.bit_rate_extension);
    ERR("vbv_buffer_size_extension                       %x\n", hdr->mpeg2_header.vbv_buffer_size_extension);
    ERR("low_delay                                       %x\n", hdr->mpeg2_header.low_delay);
    ERR("frame_rate_extension_n                          %x\n", hdr->mpeg2_header.frame_rate_extension_n);
    ERR("frame_rate_extension_d                          %x\n", hdr->mpeg2_header.frame_rate_extension_d);
    ERR("video_format                                    %x\n", hdr->mpeg2_header.video_format);
    ERR("colour_description                              %x\n", hdr->mpeg2_header.colour_description);
    ERR("colour_primaries                                %x\n", hdr->mpeg2_header.colour_primaries);
    ERR("transfer_characteristics                        %x\n", hdr->mpeg2_header.transfer_characteristics);
    ERR("matrix_coefficients                             %x\n", hdr->mpeg2_header.matrix_coefficients);
    ERR("display_horizontal_size                         %x\n", hdr->mpeg2_header.display_horizontal_size);
    ERR("display_vertical_size                           %x\n", hdr->mpeg2_header.display_vertical_size);
    ERR("scalable_mode                                   %x\n", hdr->mpeg2_header.scalable_mode);
    ERR("layer_id                                        %x\n", hdr->mpeg2_header.layer_id);
    ERR("lower_layer_prediction_horizontal_size          %x\n", hdr->mpeg2_header.lower_layer_prediction_horizontal_size);
    ERR("lower_layer_prediction_vertical_size            %x\n", hdr->mpeg2_header.lower_layer_prediction_vertical_size);
    ERR("horizontal_subsampling_factor_m                 %x\n", hdr->mpeg2_header.horizontal_subsampling_factor_m);
    ERR("horizontal_subsampling_factor_n                 %x\n", hdr->mpeg2_header.horizontal_subsampling_factor_n);
    ERR("vertical_subsampling_factor_m                   %x\n", hdr->mpeg2_header.vertical_subsampling_factor_m);
    ERR("vertical_subsampling_factor_n                   %x\n", hdr->mpeg2_header.vertical_subsampling_factor_n);
    ERR("picture_mux_enable                              %x\n", hdr->mpeg2_header.picture_mux_enable);
    ERR("mux_to_progressive_sequence                     %x\n", hdr->mpeg2_header.mux_to_progressive_sequence);
    ERR("picture_mux_order                               %x\n", hdr->mpeg2_header.picture_mux_order);
    ERR("picture_mux_factor                              %x\n", hdr->mpeg2_header.picture_mux_factor);
    ERR("drop_frame_flag                                 %x\n", hdr->mpeg2_header.drop_frame_flag);
    ERR("time_code_hours                                 %x\n", hdr->mpeg2_header.time_code_hours);
    ERR("time_code_minutes                               %x\n", hdr->mpeg2_header.time_code_minutes);
    ERR("time_code_seconds                               %x\n", hdr->mpeg2_header.time_code_seconds);
    ERR("time_code_pictures                              %x\n", hdr->mpeg2_header.time_code_pictures);
    ERR("closed_gop                                      %x\n", hdr->mpeg2_header.closed_gop);
    ERR("broken_link                                     %x\n", hdr->mpeg2_header.broken_link);
    ERR("temporal_reference                              %x\n", hdr->mpeg2_header.temporal_reference);
    ERR("picture_coding_type                             %x\n", hdr->mpeg2_header.picture_coding_type);
    ERR("vbv_delay                                       %x\n", hdr->mpeg2_header.vbv_delay);
    ERR("full_pel_forward_vector                         %x\n", hdr->mpeg2_header.full_pel_forward_vector);
    ERR("forward_f_code                                  %x\n", hdr->mpeg2_header.forward_f_code);
    ERR("full_pel_backward_vector                        %x\n", hdr->mpeg2_header.full_pel_backward_vector);
    ERR("backward_f_code                                 %x\n", hdr->mpeg2_header.backward_f_code);
    ERR("extra_bit_picture                               %x\n", hdr->mpeg2_header.extra_bit_picture);
    ERR("extra_information_picture                       %x\n", hdr->mpeg2_header.extra_information_picture);
    ERR("f_code                                          %x\n", hdr->mpeg2_header.f_code);
    ERR("intra_dc_precision                              %x\n", hdr->mpeg2_header.intra_dc_precision);
    ERR("picture_structure                               %x\n", hdr->mpeg2_header.picture_structure);
    ERR("top_field_first                                 %x\n", hdr->mpeg2_header.top_field_first);
    ERR("frame_pred_frame_dct                            %x\n", hdr->mpeg2_header.frame_pred_frame_dct);
    ERR("concealment_motion_vectors                      %x\n", hdr->mpeg2_header.concealment_motion_vectors);
    ERR("q_scale_type                                    %x\n", hdr->mpeg2_header.q_scale_type);
    ERR("intra_vlc_format                                %x\n", hdr->mpeg2_header.intra_vlc_format);
    ERR("alternate_scan                                  %x\n", hdr->mpeg2_header.alternate_scan);
    ERR("repeat_first_field                              %x\n", hdr->mpeg2_header.repeat_first_field);
    ERR("chroma_420_type                                 %x\n", hdr->mpeg2_header.chroma_420_type);
    ERR("progressive_frame                               %x\n", hdr->mpeg2_header.progressive_frame);
    ERR("composite_display_flag                          %x\n", hdr->mpeg2_header.composite_display_flag);
    ERR("v_axis                                          %x\n", hdr->mpeg2_header.v_axis);
    ERR("field_sequence                                  %x\n", hdr->mpeg2_header.field_sequence);
    ERR("sub_carrier                                     %x\n", hdr->mpeg2_header.sub_carrier);
    ERR("burst_amplitude                                 %x\n", hdr->mpeg2_header.burst_amplitude);
    ERR("sub_carrier_phase                               %x\n", hdr->mpeg2_header.sub_carrier_phase);
    ERR("load_chroma_intra_quant_mat                     %x\n", hdr->mpeg2_header.load_chroma_intra_quant_mat);
    ERR("chroma_intra_quant_mat[64]                      %x\n", hdr->mpeg2_header.chroma_intra_quant_mat[64]);
    ERR("load_chroma_non_intra_quant_mat                 %x\n", hdr->mpeg2_header.load_chroma_non_intra_quant_mat);
    ERR("chroma_non_intra_quant_mat[64]                  %x\n", hdr->mpeg2_header.chroma_non_intra_quant_mat[64]);
    ERR("frame_centre_horizontal_offset                  %x\n", hdr->mpeg2_header.frame_centre_horizontal_offset);
    ERR("frame_centre_vertical_offset                    %x\n", hdr->mpeg2_header.frame_centre_vertical_offset);
    ERR("reference_select_code                           %x\n", hdr->mpeg2_header.reference_select_code);
    ERR("forward_temporal_reference                      %x\n", hdr->mpeg2_header.forward_temporal_reference);
    ERR("backward_temporal_reference                     %x\n", hdr->mpeg2_header.backward_temporal_reference);
    ERR("lower_layer_temporal_reference                  %x\n", hdr->mpeg2_header.lower_layer_temporal_reference);
    ERR("lower_layer_horizontal_offset                   %x\n", hdr->mpeg2_header.lower_layer_horizontal_offset);
    ERR("lower_layer_vertical_offset                     %x\n", hdr->mpeg2_header.lower_layer_vertical_offset);
    ERR("spatial_temporal_weight_code_table_index        %x\n", hdr->mpeg2_header.spatial_temporal_weight_code_table_index);
    ERR("lower_layer_progressive_frame                   %x\n", hdr->mpeg2_header.lower_layer_progressive_frame);
    ERR("lower_layer_deinterlaced_field_select           %x\n", hdr->mpeg2_header.lower_layer_deinterlaced_field_select);
    ERR("copyright_flag                                  %x\n", hdr->mpeg2_header.copyright_flag);
    ERR("copyright_identifier                            %x\n", hdr->mpeg2_header.copyright_identifier);
    ERR("original_or_copy                                %x\n", hdr->mpeg2_header.original_or_copy);
    ERR("reserved                                        %x\n", hdr->mpeg2_header.reserved);
    ERR("copyright_number_1                              %x\n", hdr->mpeg2_header.copyright_number_1);
    ERR("copyright_number_2                              %x\n", hdr->mpeg2_header.copyright_number_2);
    ERR("copyright_number_3                              %x\n", hdr->mpeg2_header.copyright_number_3);
    ERR("version                                         %x\n", hdr->version);
}

static void core_mpeg2_data_dump(unsigned int addr, unsigned int size)
{
    int cnt = 0;
    char *ptraddr;

#ifdef __KERNEL__
    //method_1
    ptraddr = (char*) kmalloc(size,	GFP_KERNEL);
    copy_from_user(ptraddr, (const void *)addr, size);
    //method_2
    //ptraddr = (char *)user_to_virt((char*)addr);
#else
    ptraddr = (char*)(addr);
#endif
    DBG("buf   :0x%8x(%8d)\n", (unsigned int)ptr, (unsigned int)size);
    DBG("bufptr:0x%8x\n", (unsigned int)ptraddr);

    for( cnt=0 ; cnt <= size; cnt++ ){
        //ERR("(%d)data   0x%x\n", cnt, *ptraddr++);
        ERR("(%d)data   0x%x\n", cnt, ptraddr[cnt]);
    }

#ifdef __KERNEL__
    //method_1
    kfree(ptraddr);
#endif

    return;
}
#endif

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Get hardware capability
* \retval  0 if success
*/
int wmt_mpeg2_get_capability(mpeg2_drvinfo_t *drv)
{
    TRACE("Enter\n");
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Set hardware attribution
* \retval  0 if success
*/
int wmt_mpeg2_set_attr(mpeg2_drvinfo_t *drv)
{
    mpeg2_hdr_info_t  *hdr_in   = &(drv->attr.hdr_info);
    mpeg2_frameinfo_t *ret_info = &mpeg2info;
    unsigned int mb_width = 0, mb_height = 0;
    unsigned int frame_width = 0, frame_height = 0;
    unsigned int frame_fwidth = 0, frame_fheight = 0;
    TRACE("Enter\n");

    //mpeg2_hdr_dbg(hdr_in); //dump hdr parsing info

    // Width & Height
    frame_width   = (hdr_in->mpeg2_header.horizontal_size_value);
    frame_height  = (hdr_in->mpeg2_header.vertical_size_value);

    mb_width      = ((frame_width  + VD_MB_PIXEL - 1) / VD_MB_PIXEL);
    mb_height     = ((frame_height + VD_MB_PIXEL - 1) / VD_MB_PIXEL);

    frame_fwidth  = (mb_width  * VD_MB_PIXEL);
    frame_fheight = (mb_height * VD_MB_PIXEL);
    DBG_DETAIL("Width:%4d Height:%4d\n", frame_width, frame_height);

    ret_info->fb.img_w = frame_width;   // width of valid image (unit: pixel)
    ret_info->fb.img_h = frame_height;  // height of valid image (unit: line)
    ret_info->fb.fb_w  = frame_fwidth;  // width of frame buffer (scanline offset) (unit: pixel)
    ret_info->fb.fb_h  = frame_fheight; // height of frame buffer (unit: line)

    // MP2:Framee  Type // 0:I  1:P  2:B  3:S  4:Unknow
    // MP4:Picture Type // 0:S  1:I  2:P  3:B  4:D  5:Unknow
    DBG_DETAIL("picture_type:%d\n", hdr_in->mpeg2_header.picture_coding_type);

    // 01 // Top Field  // 10 // Botton Field  // 11 // Frame picture
    if ((hdr_in->mpeg2_header.picture_structure) == 0x3){ // 11
        DBG_DETAIL("Frame struct\n");
    }
    else { // 10 // 01
        DBG_DETAIL("Interlaced struct\n");
    }

    if ((hdr_in->version) == 0x1){
        DBG_DETAIL("MPEG1 bitstream\n");
    }
    else
    if ((hdr_in->version) == 0x2){
        DBG_DETAIL("MPEG2 bitstream\n");
    }

    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Start hardware decode
* \retval  0 if success
*/
int wmt_mpeg2_decode_proc(mpeg2_drvinfo_t *drv)
{
    mpeg2_input_t     *input_in  = &(drv->arg_in);
    mpeg2_frameinfo_t *ret_info  = &mpeg2info;
    unsigned int  ref_prd_phy, *refprdvirt;
    unsigned int  w1, w2, w3;
    TRACE("Enter\n");

    //core_mpeg2_data_dump(input_in->src_addr, input_in->src_size); //dump hdr data
    frame_num_cnt++;
    DBG_DETAIL("frame_num_cnt %d\n", frame_num_cnt);
    DBG_DETAIL("prd table addr phy:0x%8x virt:0x%8x\n", phyaddr, virtaddr);
    //----------------------------
    (ret_info->fb.y_addr) = (input_in->dst_y_addr); // Addr of Y plane in YUV domain or RGB plane in ARGB domain
    (ret_info->fb.c_addr) = (input_in->dst_c_addr); // C plane address
    (ret_info->fb.y_size) = (input_in->dst_y_size); // Buffer size in bytes
    (ret_info->fb.c_size) = (input_in->dst_c_size); // Buffer size in bytes
    //----------------------------
    (ret_info->fb.bpp)        = 8; // bits per pixel (8/16/24/32)
    (ret_info->fb.col_fmt)    = VDO_COL_FMT_YUV420; // Color format on frame buffer
    (ret_info->consumedbytes) = (input_in->data_vld_size); // consumed byte
    //----------------------------

    if ((input_in->linesize)%16){
        ERR("linesize need to do alignment\n");
    }

    //Set FB width hight addr info to DSP
    refprdvirt = (unsigned int *)vdlib_prd_virt();
    *(refprdvirt+0) = (input_in->linesize);// FB width
    *(refprdvirt+1) = ((input_in->dst_y_size)/(input_in->linesize));// FB height
    *(refprdvirt+2) = (input_in->prev_y_addr);
    *(refprdvirt+3) = (input_in->prev_c_addr);
    *(refprdvirt+4) = (input_in->dst_y_addr);
    *(refprdvirt+5) = (input_in->dst_c_addr);
    *(refprdvirt+6) = (input_in->next_y_addr);
    *(refprdvirt+7) = (input_in->next_c_addr);
    DSPP("refprdvirt: 0x%8x\n", (unsigned int)refprdvirt);
    DSPP("*(refprdvirt+0):  0x%8x\n", *(refprdvirt+0));
    DSPP("*(refprdvirt+1):  0x%8x\n", *(refprdvirt+1));
    DSPP("*(refprdvirt+2):  0x%8x\n", *(refprdvirt+2));
    DSPP("*(refprdvirt+3):  0x%8x\n", *(refprdvirt+3));
    DSPP("*(refprdvirt+4):  0x%8x\n", *(refprdvirt+4));
    DSPP("*(refprdvirt+5):  0x%8x\n", *(refprdvirt+5));
    DSPP("*(refprdvirt+6):  0x%8x\n", *(refprdvirt+6));
    DSPP("*(refprdvirt+7):  0x%8x\n", *(refprdvirt+7));

    ref_prd_phy = vdlib_prd_phy();
    //Send decoding info to DSP
    w1 = (drv->prd_addr);
    w2 = ref_prd_phy;
    w3 = ref_prd_phy + DSP_OFFSET;
    DSPP("CMD_A2D_VDEC w1:w2:w3 0x%x 0x%x 0x%x\n", w1,w2,w3);
#ifdef TIME_JUDGE
    //==============================================
    //Time Measure
    do_gettimeofday(&mp2_start);
    TIME("Start sec: %ld, msec: %ld\n", mp2_start.tv_sec, mp2_start.tv_usec/1000);
    //==============================================
#endif

    wmt_vd_timer_start(VD_MPEG);
    //Send decoding info to DSP
    dsplib_cmd_send(CMD_A2D_VDEC, w1, w2, w3);

    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Get hardware finish info
* \retval  0 if success
*/
int wmt_mpeg2_decode_finish(mpeg2_drvinfo_t *drv)
{
    mpeg2_frameinfo_t *info_out = &(drv->arg_out);
    mpeg2_frameinfo_t *ret_info = &mpeg2info;
    dsplib_mbox_t mbox;
    dsp_done_t *dspinfo;
    int retval = 0;
    TRACE("Enter\n");

    //send cmd to DSP
    retval = dsplib_cmd_recv(&mbox, CMD_D2A_VDEC_DONE);
    DSPP("CMD_D2A_VDEC_DONE w1:w2:w3 0x%x 0x%x 0x%x\n", mbox.w1, mbox.w2, mbox.w3);
    wmt_vd_timer_stop(VD_MPEG);

#ifdef TIME_JUDGE
    //==============================================
    //Time Measure
    do_gettimeofday(&mp2_end);
    TIME("End   sec: %ld, msec: %ld\n", mp2_end.tv_sec, mp2_end.tv_usec/1000);
    //==============================================
    /* unit in us */
    if (mp2_start.tv_sec == mp2_end.tv_sec){
        mp2_this_time = (mp2_end.tv_usec - mp2_start.tv_usec);
    }
    else {
        mp2_this_time = (mp2_end.tv_sec - mp2_start.tv_sec) * 1000000 + (mp2_end.tv_usec - mp2_start.tv_usec);
    }
    if (mp2_this_time < 0){
        TIME("Start sec: %ld, msec: %ld\n", mp2_start.tv_sec, mp2_start.tv_usec/1000);
        TIME("End   sec: %ld, msec: %ld\n", mp2_end.tv_sec, mp2_end.tv_usec/1000);
    }

    mp2_total_tm += mp2_this_time;
    mp2_interval_tm += mp2_this_time;
    mp2_total_cnt++;
    mp2_count++;

    if (mp2_this_time >= mp2_max) mp2_max = mp2_this_time;

    if (mp2_this_time <= mp2_min) mp2_min = mp2_this_time;

    if (mp2_this_time > mp2_threshold){
        TIME(" (%d) Decode time(%d) over %d (msec)\n", mp2_total_cnt, mp2_this_time/1000, mp2_threshold/1000);
    }
    if ((mp2_reset != 0) && (mp2_count >= mp2_reset)){
        TIME("=================================================\n");
        TIME(" Avg. time   = %d (msec)\n", mp2_interval_tm/mp2_count/1000);
        TIME("(~ %d) Decode Time Range[%d ~ %d](msec)\n", mp2_total_cnt, mp2_min/1000, mp2_max/1000);
        mp2_count = mp2_interval_tm = 0;
    }
    //==============================================
#endif

    if (retval == 0){
        if ((mbox.w1 == (vdlib_prd_phy() + DSP_OFFSET))){
            dspinfo = (void *)(vdlib_prd_virt() + DSP_OFFSET);
            DSPP("bs_type       %d\n", dspinfo->bs_type);
            DSPP("decode_status %d\n", dspinfo->decode_status);
            DSPP("frame_bitcnt  %d\n", dspinfo->frame_bitcnt);
            if (dspinfo->decode_status == 0){
                (ret_info->Decoded_status) = MP2_STA_DEC_DONE;
                MPEG2_SET_STATUS(drv, STA_DECODE_DONE);
                DBG("deocded ok\n");
                retval = 0;
            }
            else {
                (ret_info->Decoded_status) = MP2_STA_ERR_DECODING;
                MPEG2_SET_STATUS(drv, STA_ERR_DECODING);
                ERR("deocded fail\n");
                retval = -1;
            }
        }
        else{
            (ret_info->Decoded_status) = MP2_STA_ERR_DECODING;
            MPEG2_SET_STATUS(drv, STA_ERR_DECODING);
            ERR("Unexpected DSP returned addr! 0x%x : 0x%x\n", mbox.w1, (vdlib_prd_phy() + DSP_OFFSET));
            retval = -1;
        }
    }
    else{
        (ret_info->Decoded_status) = MP2_STA_FMT_NOT_SUPPORT;
        MPEG2_SET_STATUS(drv, STA_ERR_DECODING);
        ERR("expected DSP returned value!\n");
        retval = -1;
    }

    memcpy(info_out, &mpeg2info, sizeof(mpeg2_frameinfo_t));

    TRACE("Leave\n");
    return retval;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Open hardware
* \retval  0 if success
*/
int wmt_mpeg2_open(mpeg2_drvinfo_t *drv)
{
    int ret = 0;
    TRACE("Enter\n");
    mpeg2_var_reset();
    //send cmd to DSP
    ret = dsplib_cmd_send(CMD_A2D_VDEC_OPEN, VD_MPEG, 0, 0);

    wmt_vd_timer_init(VD_MPEG, 0 /*frames*/, 100 /*ms*/);

#ifdef TIME_JUDGE
    //==============================================
    //Time Measure
    memset(&mp2_start, 0, sizeof(mp2_start));
    memset(&mp2_end, 0, sizeof(mp2_end));
    mp2_threshold = 500 * 1000; /* us *//* X ms */
    mp2_max       = 0;
    mp2_min       = 0xFFFFFFF;
    mp2_reset = mp2_count = mp2_interval_tm = mp2_total_tm = mp2_total_cnt = 0;
    //==============================================
#endif
    TRACE("Leave\n");
    return ret;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Close hardware
* \retval  0 if success
*/
int wmt_mpeg2_close(void)
{
    TRACE("Enter\n");
    mpeg2_var_reset();
    //send cmd to DSP
    dsplib_cmd_send(CMD_A2D_VDEC_CLOSE, VD_MPEG, 0, 0);
#ifdef TIME_JUDGE
    //==============================================
    //Time Measure
    if (mp2_total_cnt){
        TIME("=== Timer status:\n");
        TIME("Total count = %d \n", mp2_total_cnt);
        TIME("Total time  = %d (msec)\n", mp2_total_tm/1000);
        TIME("Avg. time   = %d (msec)\n", mp2_total_tm/mp2_total_cnt/1000);        
        TIME("Max time    = %d (msec)\n", mp2_max/1000);
        TIME("Min time    = %d (msec)\n", mp2_min/1000);
        TIME("==========================================\n");
    }
    memset(&mp2_start, 0, sizeof(mp2_start));
    memset(&mp2_end, 0, sizeof(mp2_end));
    mp2_threshold = 0;
    mp2_max       = 0;
    mp2_min       = 0;
    mp2_reset = mp2_count = mp2_interval_tm = mp2_total_tm = mp2_total_cnt = 0;
    //==============================================
#endif
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Get hardware capability
*
* \retval  0 if success
*/
static int ioctl_get_capability(mpeg2_drvinfo_t *drvinfo, mpeg2_capability_t *capab)
{
    TRACE("Enter\n");
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
    ==========================================================================*/
    if (wmt_mpeg2_get_capability(drvinfo) == 0){
        memcpy(capab, drvinfo->capab_tbl, sizeof(mpeg2_capability_t));
    }
    DBG("Hardware supported capabilities\n");
    DBG("--------------------------------\n");
    DBG("Identity:        0x%x (%d)\n", capab->identity, capab->identity & 0xFFFF);
    DBG("Chip ID:           %d\n", capab->chip_id);
    DBG("Progressive:       %d\n", capab->progressive);
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Set hardware attribution
* \retval  0 if success
*/
static int ioctl_set_attr(mpeg2_drvinfo_t *drvinfo, mpeg2_attr_t *attr)
{
    int ret;
    TRACE("Enter\n");
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_ATTR"
    ==========================================================================*/
    memcpy(&drvinfo->attr, attr, sizeof(mpeg2_attr_t));
    DBG("Width:         %d\n", drvinfo->attr.hdr_info.mpeg2_header.video_object_layer_width);
    DBG("Height:        %d\n", drvinfo->attr.hdr_info.mpeg2_header.video_object_layer_height);
    DBG("version:       %d\n", drvinfo->attr.hdr_info.version);
    MPEG2_SET_STATUS(drvinfo, STA_ATTR_SET);
    ret = wmt_mpeg2_set_attr(drvinfo);
    TRACE("Leave\n");
    return ret;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Start hardware decode
* \retval  0 if success
*/
static int ioctl_decode_proc(mpeg2_drvinfo_t *drvinfo, mpeg2_input_t *arg)
{
    int ret;
    TRACE("Enter\n");
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_PROC"
    ==========================================================================*/
    DBG("src_addr:        %8x\n", arg->src_addr);
    DBG("src_size:        %8d\n", arg->src_size);
    DBG("flags:         0x%8x\n", arg->flags);
    DBG("linesize:        %8d\n", arg->linesize);
    //==============================================
    DBG("dst_y_addr:    0x%8x\n", arg->dst_y_addr);
    DBG("dst_y_size:      %8d\n", arg->dst_y_size);
    DBG("dst_c_addr:    0x%8x\n", arg->dst_c_addr);
    DBG("dst_c_size:      %8d\n", arg->dst_c_size);
    DBG("prev_y_addr:   0x%8x\n", arg->prev_y_addr);
    DBG("prev_c_addr:   0x%8x\n", arg->prev_c_addr);
    DBG("next_y_addr:   0x%8x\n", arg->next_y_addr);
    DBG("next_c_addr:   0x%8x\n", arg->next_c_addr);
    //==============================================
    memcpy(&drvinfo->arg_in, arg, sizeof(mpeg2_input_t));
    //build prd table
    if (user_to_prdt(arg->src_addr, arg->src_size, (struct prdt_struct *)drvinfo->prd_virt, MAX_INPUT_BUF_SIZE/sizeof(struct prdt_struct))){
        ERR("input buffer to prd table error!!\n");
        MPEG2_SET_STATUS(drvinfo, STA_ERR_BAD_PRD);
        return -EFAULT;
    }
    MPEG2_SET_STATUS(drvinfo, STA_DECODEING);
    ret = wmt_mpeg2_decode_proc(drvinfo);
    TRACE("Leave\n");
    return ret;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Get hardware finish info
* \retval  0 if success
*/
static int ioctl_decode_finish(mpeg2_drvinfo_t *drvinfo, mpeg2_frameinfo_t *frame_info)
{
    int ret;
    TRACE("Enter\n");
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_FINISH"
    ==========================================================================*/
    memcpy(&drvinfo->arg_out, frame_info, sizeof(mpeg2_frameinfo_t));
    ret = wmt_mpeg2_decode_finish(drvinfo);
    memcpy(frame_info, &drvinfo->arg_out, sizeof(mpeg2_frameinfo_t));
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
    TRACE("Leave\n");
    return ret;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   It implement io control for AP
* \parameter
*   inode  [IN] a pointer point to struct inode
*   filp   [IN] a pointer point to struct file
*   cmd
*   arg
* \retval  0 if success
*/
static int mpeg2_ioctl(struct inode *inode, struct file  *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0, retval = 0;
    unsigned long  flags =0;
    mpeg2_drvinfo_t     *drvinfo;
    mpeg2_capability_t  capab;
    mpeg2_attr_t        attr;
    mpeg2_input_t       input;
    mpeg2_frameinfo_t   frame_info;

    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Check input arguments
    --------------------------------------------------------------------------*/
    if (_IOC_TYPE(cmd) != VD_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > VD_IOC_MAXNR) return -ENOTTY;

    /* check argument area */
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
    else
    if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));

    if (err) return -EFAULT;
    /*--------------------------------------------------------------------------
        Step 2: IOCTL handler
    --------------------------------------------------------------------------*/
    spin_lock_irqsave(&mpeg2_lock, flags);
    drvinfo = (mpeg2_drvinfo_t *)filp->private_data;
    switch (cmd){       
      case VDIOSET_DECODE_INFO:
           DBG("Receive IOCTL:VDIOSET_DECODE_INFO\n");
           retval = copy_from_user(&attr, (const void *)arg, sizeof(mpeg2_attr_t));
           if (retval){
               ERR("copy_from_user FAIL in VDIOSET_DECODE_INFO!\n");
               break;
           }
           retval = IOCTL_CMD_CHECK(&attr, mpeg2_attr_t, VD_MPEG);
           if (retval){
               ERR("Version conflict (0x%x) in VDIOSET_DECODE_INFO\n", retval);
               DBG("identity: 0x%x, size:%d\n", attr.identity, attr.size);
           }
           else {
               retval = ioctl_set_attr(drvinfo, &attr);
           }
           break;
      case VDIOSET_DECODE_PROC:
           DBG("Receive IOCTL:VDIOSET_DECODE_PROC\n");
           retval = copy_from_user((mpeg2_input_t *)&input, (const void *)arg, sizeof(mpeg2_input_t));
           if (retval){
               ERR("copy_from_user FAIL in VDIOSET_DECODE_PROC!\n");   
               break;
           }
           retval = IOCTL_CMD_CHECK(&input, mpeg2_input_t, VD_MPEG);
           if (retval){
               ERR("Version conflict (0x%x) in VDIOSET_DECODE_PROC\n", retval);
               DBG("identity: 0x%x, size:%d\n", input.identity, input.size);
           }
           else { 
#if 1 // ??  
               if (((PAGE_ALIGN(input.src_size)/PAGE_SIZE)*8) > MAX_INPUT_BUF_SIZE){
                   ERR("input size overflow. %x/%x\n", input.src_size, MAX_INPUT_BUF_SIZE*4096/8);
                   return -EFAULT;
               }
#endif       
               down_interruptible(&mpeg2_decoding_sema);
               /*--------------------------------------------------------------
                   Before HW decoding finish, we cannot decode another MPEG2
               --------------------------------------------------------------*/
               spin_unlock_irqrestore(&mpeg2_lock, flags);
               retval = ioctl_decode_proc(drvinfo, &input);
               spin_lock_irqsave(&mpeg2_lock, flags);
           }
           break;
      case VDIOGET_DECODE_FINISH:
           DBG("Receive IOCTL:VDIOGET_DECODE_FINISH\n");
           if (MPEG2_GET_STATUS(drvinfo) == STA_ERR_BAD_PRD){
               retval = -1;
               break;
           }
           retval = copy_from_user((vd_ioctl_cmd*)&frame_info, (const void *)arg, sizeof(vd_ioctl_cmd));
           if (retval){
               ERR("copy_from_user FAIL in VDIOGET_DECODE_FINISH!\n");   
               break;
           }
           retval = IOCTL_CMD_CHECK(&frame_info, mpeg2_frameinfo_t, VD_MPEG);
           if (retval){
               ERR("Version conflict (0x%x) in VDIOGET_DECODE_FINISH\n", retval);
               DBG("identity: 0x%x, size:%d\n", frame_info.identity, frame_info.size);
               retval = -ENOTTY;
           }
           else {
               spin_unlock_irqrestore(&mpeg2_lock, flags);
               retval = ioctl_decode_finish(drvinfo, &frame_info);
               copy_to_user((mpeg2_frameinfo_t *)arg, &frame_info, sizeof(mpeg2_frameinfo_t));
               spin_lock_irqsave(&mpeg2_lock, flags);
               /*--------------------------------------------------------------
                   Before HW decoding finish, we cannot decode another MPEG2
               --------------------------------------------------------------*/
               up(&mpeg2_decoding_sema); 
           }
           break;
      case VDIOGET_CAPABILITY:
           DBG("Receive IOCTL:VDIOGET_CAPABILITY\n");
           retval = copy_from_user((vd_ioctl_cmd*)&capab, (const void *)arg, sizeof(vd_ioctl_cmd));
           if (retval){
               ERR("copy_from_user FAIL in VDIOGET_CAPABILITY!\n");   
               break;
           }
           retval = IOCTL_CMD_CHECK(&capab, mpeg2_capability_t, VD_MPEG);
           if (retval){
               ERR("Version conflict (0x%x) in VDIOGET_CAPABILITY\n", retval);
               DBG("identity: 0x%x, size:%d\n", capab.identity, capab.size);
               retval = -ENOTTY;
           }
           else {
               retval = ioctl_get_capability(drvinfo, &capab);
               if (retval == 0){
                   retval = copy_to_user((mpeg2_capability_t *)arg, &capab, sizeof(mpeg2_capability_t));
               }
           }
           break;
      default:
           ERR("Unknown IOCTL:0x%x in mpeg2_ioctl()!\n", cmd);
           retval = -ENOTTY;
           break;
    } /* switch(cmd) */
    spin_unlock_irqrestore(&mpeg2_lock, flags);
    TRACE("Leave\n");
    return retval;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Probe
* \retval  0 if success
*/
static int mpeg2_probe(void)
{
    TRACE("Enter\n");
    spin_lock_init(&mpeg2_lock);
    vdlib_prd_init();
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   It is called when driver is opened, and initial hardward and resource
* \parameter
*   inode  [IN] a pointer point to struct inode
*   filp   [IN] a pointer point to struct file
* \retval  0 if success
*/
static int mpeg2_open(struct inode *inode, struct file *filp)
{
    mpeg2_drvinfo_t *drvinfo;
    struct videodecoder_info *vd_info;
    int ret = 0;
    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Initial Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&mpeg2_lock);
    if (mpeg2_dev_ref){
        /* Currently we do not support multi-open */
        spin_unlock(&mpeg2_lock);
        return -EBUSY;
    }   
    mpeg2_dev_ref++;
    spin_unlock(&mpeg2_lock);
    /*--------------------------------------------------------------------------
        Step 2: Initial hardware decoder
    --------------------------------------------------------------------------*/
    drvinfo = kmalloc(sizeof(mpeg2_drvinfo_t), GFP_KERNEL);
    if (drvinfo == 0){
        ERR("Allocate %d bytes fail!\n", sizeof(mpeg2_drvinfo_t));
        ret = -EBUSY;
        goto EXIT_mpeg2_open;
    }
    /*--------------------------------------------------------------------------
        private_data from VD is "PRD table virtual & physical address"
    --------------------------------------------------------------------------*/
    vd_info = (struct videodecoder_info *)filp->private_data;
    drvinfo->prd_virt = (unsigned int)vd_info->prdt_virt;
    drvinfo->prd_addr = (unsigned int)vd_info->prdt_phys;
    DBG("prd_virt: 0x%x, prd_addr: 0x%x\n", drvinfo->prd_virt, drvinfo->prd_addr);
    /* change privata_data to drvinfo */
    filp->private_data = drvinfo;
    drvinfo->size      = sizeof(mpeg2_drvinfo_t);
    drvinfo->_lock     = mpeg2_lock;
    MPEG2_SET_STATUS(drvinfo, STA_READY);
    ret = wmt_mpeg2_open(drvinfo);
EXIT_mpeg2_open:
    TRACE("Leave\n");
    return ret;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   It is called when driver is released, and reset hardward and free resource
* \parameter
*   inode  [IN] a pointer point to struct inode
*   filp   [IN] a pointer point to struct file
* \retval  0 if success
*/
static int mpeg2_release(struct inode *inode, struct file *filp)
{
    mpeg2_drvinfo_t *drvinfo = (mpeg2_drvinfo_t *)filp->private_data;
    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 0: Check status
    --------------------------------------------------------------------------*/
    if (MPEG2_GET_STATUS(drvinfo) == STA_DECODEING){
        WARNING("Abnormally exit when driver still decoding.\n");
        up(&mpeg2_decoding_sema); 
    }
    /*--------------------------------------------------------------------------
        Step 1: Close hardware decoder
    --------------------------------------------------------------------------*/
    MPEG2_SET_STATUS(drvinfo, STA_CLOSE);
    wmt_mpeg2_close();
    if (filp->private_data){
        kfree(filp->private_data);
    }
    /*--------------------------------------------------------------------------
        Step 2: Release Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&mpeg2_lock);
    if (!mpeg2_dev_ref){
        spin_unlock(&mpeg2_lock);
        return -EBUSY;
    }   
    mpeg2_dev_ref--;
    spin_unlock(&mpeg2_lock);
    module_put(THIS_MODULE);
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   setup module
* \retval  0 if success
*/
static int mpeg2_setup(void)
{
    TRACE("Enter\n");
    TRACE("Leave\n");
    return mpeg2_probe();
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   remove module
* \retval  0 if success
*/
static int mpeg2_remove(void)
{   
    TRACE("Enter\n");
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   suspend module
* \parameter
*   dev
*   state
*   level
* \retval  0 if success
*/
static int mpeg2_suspend(pm_message_t state)
{
    TRACE("Enter\n");
    switch (state.event){
      case PM_EVENT_SUSPEND:
      case PM_EVENT_FREEZE:
      case PM_EVENT_PRETHAW:
      default:
           break;	
    }
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   resume module
* \retval  0 if success
*/
static int mpeg2_resume(void)
{
    TRACE("Enter\n");
    TRACE("Leave\n");
    return 0;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*
* \retval  0 if success
*/
#ifdef CONFIG_PROC_FS
static int mpeg2_get_info(char *buf, char **start, off_t offset, int len)
{
    char *p = buf;
    TRACE("Enter\n");
    // TO DO
    p += sprintf(p, "%s previous state READY\n", DRIVER_NAME);
    p += sprintf(p, "%s current state READY\n\n", DRIVER_NAME);
    p += sprintf(p, "%s show other information\n\n", DRIVER_NAME);
    // other information or register
    // ...
    TRACE("Leave\n");
    return (p - buf);
}
#endif
/*!*************************************************************************
    platform device struct define
****************************************************************************/

struct videodecoder mpeg2_decoder = {
    .name    = DRIVER_NAME,
    .id      = VD_MPEG,
    .setup   = mpeg2_setup,
    .remove  = mpeg2_remove,
    .suspend = mpeg2_suspend,
    .resume  = mpeg2_resume,
    .fops    = {
                .owner   = THIS_MODULE,
                .open    = mpeg2_open,
                //.read    = mpeg2_read,
                //.write   = mpeg2_write,
                .ioctl   = mpeg2_ioctl,
                //.mmap    = mpeg2_mmap,
                .release = mpeg2_release,
               },
    .device = NULL,
#ifdef CONFIG_PROC_FS
    .get_info = mpeg2_get_info,
#endif /* CONFIG_PROC_FS */
};

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   init module
* \retval  0 if success
*/
static int hw_mpeg2_init(void)
{
    int ret;
    TRACE("Enter\n");
    ret = videodecoder_register(&mpeg2_decoder);
    TRACE("Leave\n");
    return ret;
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   exit module
* \retval  0 if success
*/
static void hw_mpeg2_exit(void)
{
    TRACE("Enter\n");
    videodecoder_unregister(&mpeg2_decoder);
    TRACE("Leave\n");
    return;
}

module_init(hw_mpeg2_init);
module_exit(hw_mpeg2_exit);
MODULE_AUTHOR("WonderMedia Multimedia SW Team Welkin Chen");
MODULE_DESCRIPTION("WMT MPEG2 decode device driver");
MODULE_LICENSE("GPL");
/*----------------------------------------------------------------------------*/
/*=== END ====================================================================*/
#endif
