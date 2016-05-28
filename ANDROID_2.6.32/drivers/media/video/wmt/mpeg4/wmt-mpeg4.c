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
* MODULE       : MPEG4 Video Decoder
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
#ifndef WM8605_MPEG4_C
#define WM8605_MPEG4_C
/* To assert that only one occurrence is included */
//------------------------------------------------
//  Include Header
//------------------------------------------------
#include "../wmt-vd.h"
#include "../wmt-dsplib.h"
#include "../wmt-vdlib.h"
#include "wmt-mpeg4.h"

/*----------------------------------------------------------------------------*/
/*------------------------------------------------
    Debug Control
------------------------------------------------*/
#undef NAME 
#undef THIS_TRACE
#undef THIS_DEBUG
#undef THIS_DEBUG_DETAIL
#undef THIS_REG_TRACE

#define NAME MPEG4
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
static unsigned int   mp4_total_tm;    /* total time */
static unsigned int   mp4_interval_tm; /* interval time */
static unsigned int   mp4_total_cnt;   /* total counter */
static unsigned int   mp4_count;       /* interval counter */
static unsigned int   mp4_reset;       /* reset counter */
static unsigned int   mp4_max;         /* max time */
static unsigned int   mp4_min;         /* min time */
static struct timeval mp4_start;       /* start time */
static struct timeval mp4_end;         /* end time */
static unsigned int   mp4_threshold;   /* us */
static struct timeval mp4_get_s;       /* start time */
static struct timeval mp4_get_e;       /* end time */
static int            mp4_this_time;
//==============================================
#endif
/*----------------------------------------------------------------------------*/

#define DEVICE_NAME "WMT-MPEG4"   /* appear in /proc/devices & /proc/wm-mpeg4 */
#define DRIVER_NAME "wmt-mpeg4"

#define VD_MB_PIXEL 16
#define DSP_OFFSET  64

#define MPEG4_GET_STATUS(drv)       (drv)->_status
#define MPEG4_SET_STATUS(drv, sta)  (drv)->_status = (sta)

static int mpeg4_dev_ref = 0; /* is device open */
static spinlock_t  mpeg4_lock;
DECLARE_MUTEX(mpeg4_decoding_sema);

static int frame_num_cnt = 0;

mpeg4_frameinfo_t mpeg4info;
/*----------------------------------------------------------------------------*/
/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   Reset global variable
* \retval  N/A
*/
static void mpeg4_var_reset(void)
{
    //Reset Global variable
    frame_num_cnt = 0;
    return;
}

#if 0
static void mpeg4_hdr_dbg(void *hdraddr)
{
    mpeg4_hdr_info_t *hdr = (mpeg4_hdr_info_t *)hdraddr;
    ERR("profile_and_level_indication                     %x\n", hdr->mpeg4_header.profile_and_level_indication);
    ERR("is_visual_object_identifier                      %x\n", hdr->mpeg4_header.is_visual_object_identifier);
    ERR("visual_object_verid                              %x\n", hdr->mpeg4_header.visual_object_verid);
    ERR("visual_object_priority                           %x\n", hdr->mpeg4_header.visual_object_priority);
    ERR("visual_object_type                               %x\n", hdr->mpeg4_header.visual_object_type);
    ERR("video_signal_type                                %x\n", hdr->mpeg4_header.video_signal_type);
    ERR("video_format                                     %x\n", hdr->mpeg4_header.video_format);
    ERR("video_range                                      %x\n", hdr->mpeg4_header.video_range);
    ERR("colour_description                               %x\n", hdr->mpeg4_header.colour_description);
    ERR("colour_primaries                                 %x\n", hdr->mpeg4_header.colour_primaries);
    ERR("transfer_characteristics                         %x\n", hdr->mpeg4_header.transfer_characteristics);
    ERR("matrix_coefficients                              %x\n", hdr->mpeg4_header.matrix_coefficients);
    ERR("user_data                                        %x\n", hdr->mpeg4_header.user_data);
    ERR("random_accessible_vol                            %x\n", hdr->mpeg4_header.random_accessible_vol);
    ERR("video_object_type_indication                     %x\n", hdr->mpeg4_header.video_object_type_indication);
    ERR("is_object_layer_identifier                       %x\n", hdr->mpeg4_header.is_object_layer_identifier);
    ERR("video_object_layer_verid                         %x\n", hdr->mpeg4_header.video_object_layer_verid);
    ERR("video_object_layer_priority                      %x\n", hdr->mpeg4_header.video_object_layer_priority);
    ERR("aspect_ratio_info                                %x\n", hdr->mpeg4_header.aspect_ratio_info);
    ERR("par_width                                        %x\n", hdr->mpeg4_header.par_width);
    ERR("par_height                                       %x\n", hdr->mpeg4_header.par_height);
    ERR("vol_control_parameters                           %x\n", hdr->mpeg4_header.vol_control_parameters);
    ERR("chroma_format                                    %x\n", hdr->mpeg4_header.chroma_format);
    ERR("low_delay                                        %x\n", hdr->mpeg4_header.low_delay);
    ERR("vbv_parameters                                   %x\n", hdr->mpeg4_header.vbv_parameters);
    ERR("first_half_bit_rate                              %x\n", hdr->mpeg4_header.first_half_bit_rate);
    ERR("latter_half_bit_rate                             %x\n", hdr->mpeg4_header.latter_half_bit_rate);
    ERR("first_half_vbv_buffer_size                       %x\n", hdr->mpeg4_header.first_half_vbv_buffer_size);
    ERR("latter_half_vbv_buffer_size                      %x\n", hdr->mpeg4_header.latter_half_vbv_buffer_size);
    ERR("first_half_vbv_occupancy                         %x\n", hdr->mpeg4_header.first_half_vbv_occupancy);
    ERR("latter_half_vbv_occupancy                        %x\n", hdr->mpeg4_header.latter_half_vbv_occupancy);
    ERR("video_object_layer_shape                         %x\n", hdr->mpeg4_header.video_object_layer_shape);
    ERR("video_object_layer_shape_extension               %x\n", hdr->mpeg4_header.video_object_layer_shape_extension);
    ERR("vop_time_increment_resolution                    %x\n", hdr->mpeg4_header.vop_time_increment_resolution);
    ERR("fixed_vop_rate                                   %x\n", hdr->mpeg4_header.fixed_vop_rate);
    ERR("fixed_vop_time_increment                         %x\n", hdr->mpeg4_header.fixed_vop_time_increment);
    ERR("video_object_layer_width                         %x\n", hdr->mpeg4_header.video_object_layer_width);
    ERR("video_object_layer_height                        %x\n", hdr->mpeg4_header.video_object_layer_height);
    ERR("interlaced                                       %x\n", hdr->mpeg4_header.interlaced);
    ERR("obmc_disable                                     %x\n", hdr->mpeg4_header.obmc_disable);
    ERR("sprite_enable                                    %x\n", hdr->mpeg4_header.sprite_enable);
    ERR("sprite_width                                     %x\n", hdr->mpeg4_header.sprite_width);
    ERR("sprite_height                                    %x\n", hdr->mpeg4_header.sprite_height);
    ERR("sprite_left_coordinate                           %x\n", hdr->mpeg4_header.sprite_left_coordinate);
    ERR("sprite_top_coordinate                            %x\n", hdr->mpeg4_header.sprite_top_coordinate);
    ERR("no_of_sprite_warping_points                      %x\n", hdr->mpeg4_header.no_of_sprite_warping_points);
    ERR("sprite_warping_accuracy                          %x\n", hdr->mpeg4_header.sprite_warping_accuracy);
    ERR("sprite_brightness_change                         %x\n", hdr->mpeg4_header.sprite_brightness_change);
    ERR("low_latency_sprite_enable                        %x\n", hdr->mpeg4_header.low_latency_sprite_enable);
    ERR("sadct_disable                                    %x\n", hdr->mpeg4_header.sadct_disable);
    ERR("not_8_bit                                        %x\n", hdr->mpeg4_header.not_8_bit);
    ERR("quant_precision                                  %x\n", hdr->mpeg4_header.quant_precision);
    ERR("bits_per_pixel                                   %x\n", hdr->mpeg4_header.bits_per_pixel);
    ERR("no_gray_quant_update                             %x\n", hdr->mpeg4_header.no_gray_quant_update);
    ERR("composition_method                               %x\n", hdr->mpeg4_header.composition_method);
    ERR("linear_composition                               %x\n", hdr->mpeg4_header.linear_composition);
    ERR("quant_type                                       %x\n", hdr->mpeg4_header.quant_type);
    ERR("load_intra_quant_mat                             %x\n", hdr->mpeg4_header.load_intra_quant_mat);
    //ERR("intra_quant_mat[64]                              %x\n", hdr->mpeg4_header.intra_quant_mat[64]);
    ERR("load_nonintra_quant_mat                          %x\n", hdr->mpeg4_header.load_nonintra_quant_mat);
    //ERR("nonintra_quant_mat[64]                           %x\n", hdr->mpeg4_header.nonintra_quant_mat[64]);
    ERR("load_intra_quant_mat_grayscale                   %x\n", hdr->mpeg4_header.load_intra_quant_mat_grayscale);
    //ERR("intra_quant_mat_grayscale[64]                    %x\n", hdr->mpeg4_header.intra_quant_mat_grayscale[64]);
    ERR("load_nonintra_quant_mat_grayscale                %x\n", hdr->mpeg4_header.load_nonintra_quant_mat_grayscale);
    //ERR("nonintra_quant_mat_grayscale[64]                 %x\n", hdr->mpeg4_header.nonintra_quant_mat_grayscale[64]);
    ERR("quarter_sample                                   %x\n", hdr->mpeg4_header.quarter_sample);
    ERR("complexity_estimation_disable                    %x\n", hdr->mpeg4_header.complexity_estimation_disable);
    ERR("resync_marker_disable                            %x\n", hdr->mpeg4_header.resync_marker_disable);
    ERR("data_partitioned                                 %x\n", hdr->mpeg4_header.data_partitioned);
    ERR("reversible_vlc                                   %x\n", hdr->mpeg4_header.reversible_vlc);
    ERR("newpred_enable                                   %x\n", hdr->mpeg4_header.newpred_enable);
    ERR("requested_upstream_message_type                  %x\n", hdr->mpeg4_header.requested_upstream_message_type);
    ERR("newpred_segment_type                             %x\n", hdr->mpeg4_header.newpred_segment_type);
    ERR("reduced_resolution_vop_enable                    %x\n", hdr->mpeg4_header.reduced_resolution_vop_enable);
    ERR("scalability                                      %x\n", hdr->mpeg4_header.scalability);
    ERR("hierarchy_type                                   %x\n", hdr->mpeg4_header.hierarchy_type);
    ERR("ref_layer_id                                     %x\n", hdr->mpeg4_header.ref_layer_id);
    ERR("ref_layer_sampling_direc                         %x\n", hdr->mpeg4_header.ref_layer_sampling_direc);
    ERR("hor_sampling_factor_n                            %x\n", hdr->mpeg4_header.hor_sampling_factor_n);
    ERR("hor_sampling_factor_m                            %x\n", hdr->mpeg4_header.hor_sampling_factor_m);
    ERR("vert_sampling_factor_n                           %x\n", hdr->mpeg4_header.vert_sampling_factor_n);
    ERR("vert_sampling_factor_m                           %x\n", hdr->mpeg4_header.vert_sampling_factor_m);
    ERR("enhancement_type                                 %x\n", hdr->mpeg4_header.enhancement_type);
    ERR("use_ref_shape                                    %x\n", hdr->mpeg4_header.use_ref_shape);
    ERR("use_ref_texture                                  %x\n", hdr->mpeg4_header.use_ref_texture);
    ERR("shape_hor_sampling_factor_n                      %x\n", hdr->mpeg4_header.shape_hor_sampling_factor_n);
    ERR("shape_hor_sampling_factor_m                      %x\n", hdr->mpeg4_header.shape_hor_sampling_factor_m);
    ERR("shape_vert_sampling_factor_n                     %x\n", hdr->mpeg4_header.shape_vert_sampling_factor_n);
    ERR("shape_vert_sampling_factor_m                     %x\n", hdr->mpeg4_header.shape_vert_sampling_factor_m);
    ERR("estimation_method                                %x\n", hdr->mpeg4_header.estimation_method);
    ERR("shape_complexity_estimation_disable              %x\n", hdr->mpeg4_header.shape_complexity_estimation_disable);
    ERR("opaque                                           %x\n", hdr->mpeg4_header.opaque);
    ERR("transparent                                      %x\n", hdr->mpeg4_header.transparent);
    ERR("intra_cae                                        %x\n", hdr->mpeg4_header.intra_cae);
    ERR("inter_cae                                        %x\n", hdr->mpeg4_header.inter_cae);
    ERR("no_update                                        %x\n", hdr->mpeg4_header.no_update);
    ERR("upsampling                                       %x\n", hdr->mpeg4_header.upsampling);
    ERR("texture_complexity_estimation_set_1_disable      %x\n", hdr->mpeg4_header.texture_complexity_estimation_set_1_disable);
    ERR("intra_blocks                                     %x\n", hdr->mpeg4_header.intra_blocks);
    ERR("inter_blocks                                     %x\n", hdr->mpeg4_header.inter_blocks);
    ERR("inter4v_blocks                                   %x\n", hdr->mpeg4_header.inter4v_blocks);
    ERR("not_coded_blocks                                 %x\n", hdr->mpeg4_header.not_coded_blocks);
    ERR("texture_complexity_estimation_set_2_disable      %x\n", hdr->mpeg4_header.texture_complexity_estimation_set_2_disable);
    ERR("dct_coefs                                        %x\n", hdr->mpeg4_header.dct_coefs);
    ERR("dct_lines                                        %x\n", hdr->mpeg4_header.dct_lines);
    ERR("vlc_symbols                                      %x\n", hdr->mpeg4_header.vlc_symbols);
    ERR("vlc_bits                                         %x\n", hdr->mpeg4_header.vlc_bits);
    ERR("motion_compensation_complexity_disable           %x\n", hdr->mpeg4_header.motion_compensation_complexity_disable);
    ERR("apm                                              %x\n", hdr->mpeg4_header.apm);
    ERR("npm                                              %x\n", hdr->mpeg4_header.npm);
    ERR("interpolate_mc_q                                 %x\n", hdr->mpeg4_header.interpolate_mc_q);
    ERR("forw_back_mc_q                                   %x\n", hdr->mpeg4_header.forw_back_mc_q);
    ERR("halfpel2                                         %x\n", hdr->mpeg4_header.halfpel2);
    ERR("halfpel4                                         %x\n", hdr->mpeg4_header.halfpel4);
    ERR("version2_complexity_estimation_disable           %x\n", hdr->mpeg4_header.version2_complexity_estimation_disable);
    ERR("sadct                                            %x\n", hdr->mpeg4_header.sadct);
    ERR("quarterpel                                       %x\n", hdr->mpeg4_header.quarterpel);
    ERR("time_code                                        %x\n", hdr->mpeg4_header.time_code);
    ERR("closed_gov                                       %x\n", hdr->mpeg4_header.closed_gov);
    ERR("broken_link                                      %x\n", hdr->mpeg4_header.broken_link);
    ERR("vop_coding_type                                  %x\n", hdr->mpeg4_header.vop_coding_type);
    ERR("modulo_time_base                                 %x\n", hdr->mpeg4_header.modulo_time_base);
    ERR("vop_time_increment                               %x\n", hdr->mpeg4_header.vop_time_increment);
    ERR("vop_coded                                        %x\n", hdr->mpeg4_header.vop_coded);
    ERR("vop_id                                           %x\n", hdr->mpeg4_header.vop_id);
    ERR("vop_id_for_prediction_indication                 %x\n", hdr->mpeg4_header.vop_id_for_prediction_indication);
    ERR("vop_id_for_prediction                            %x\n", hdr->mpeg4_header.vop_id_for_prediction);
    ERR("vop_rounding_type                                %x\n", hdr->mpeg4_header.vop_rounding_type);
    ERR("vop_reduced_resolution                           %x\n", hdr->mpeg4_header.vop_reduced_resolution);
    ERR("vop_width                                        %x\n", hdr->mpeg4_header.vop_width);
    ERR("vop_height                                       %x\n", hdr->mpeg4_header.vop_height);
    ERR("vop_horizontal_mc_spatial_ref                    %x\n", hdr->mpeg4_header.vop_horizontal_mc_spatial_ref);
    ERR("vop_vertical_mc_spatial_ref                      %x\n", hdr->mpeg4_header.vop_vertical_mc_spatial_ref);
    ERR("background_composition                           %x\n", hdr->mpeg4_header.background_composition);
    ERR("change_conv_ratio_disable                        %x\n", hdr->mpeg4_header.change_conv_ratio_disable);
    ERR("vop_constant_alpha                               %x\n", hdr->mpeg4_header.vop_constant_alpha);
    ERR("vop_constant_alpha_value                         %x\n", hdr->mpeg4_header.vop_constant_alpha_value);
    ERR("intra_dc_vlc_thr                                 %x\n", hdr->mpeg4_header.intra_dc_vlc_thr);
    ERR("top_field_first                                  %x\n", hdr->mpeg4_header.top_field_first);
    ERR("alternate_vertical_scan_flag                     %x\n", hdr->mpeg4_header.alternate_vertical_scan_flag);
    ERR("sprite_transmit_mode                             %x\n", hdr->mpeg4_header.sprite_transmit_mode);
    //ERR("warping_points[4]                                %x\n", hdr->mpeg4_header.warping_points[4]);
    ERR("vop_quant                                        %x\n", hdr->mpeg4_header.vop_quant);
    ERR("vop_fcode_forward                                %x\n", hdr->mpeg4_header.vop_fcode_forward);
    ERR("vop_fcode_backward                               %x\n", hdr->mpeg4_header.vop_fcode_backward);
    ERR("vop_shape_coding_type                            %x\n", hdr->mpeg4_header.vop_shape_coding_type);
    ERR("load_backward_shape                              %x\n", hdr->mpeg4_header.load_backward_shape);
    ERR("backward_shape_width                             %x\n", hdr->mpeg4_header.backward_shape_width);
    ERR("backward_shape_height                            %x\n", hdr->mpeg4_header.backward_shape_height);
    ERR("backward_shape_horizontal_mc_spatial_ref         %x\n", hdr->mpeg4_header.backward_shape_horizontal_mc_spatial_ref);
    ERR("backward_shape_vertical_mc_spatial_ref           %x\n", hdr->mpeg4_header.backward_shape_vertical_mc_spatial_ref);
    ERR("load_forward_shape                               %x\n", hdr->mpeg4_header.load_forward_shape);
    ERR("forward_shape_width                              %x\n", hdr->mpeg4_header.forward_shape_width);
    ERR("forward_shape_height                             %x\n", hdr->mpeg4_header.forward_shape_height);
    ERR("forward_shape_horizontal_mc_spatial_ref          %x\n", hdr->mpeg4_header.forward_shape_horizontal_mc_spatial_ref);
    ERR("forward_shape_vertical_mc_spatial_ref            %x\n", hdr->mpeg4_header.forward_shape_vertical_mc_spatial_ref);
    ERR("ref_select_code                                  %x\n", hdr->mpeg4_header.ref_select_code);
    ERR("dcecs_opaque                                     %x\n", hdr->mpeg4_header.dcecs_opaque);
    ERR("dcecs_transparent                                %x\n", hdr->mpeg4_header.dcecs_transparent);
    ERR("dcecs_intra_cae                                  %x\n", hdr->mpeg4_header.dcecs_intra_cae);
    ERR("dcecs_inter_cae                                  %x\n", hdr->mpeg4_header.dcecs_inter_cae);
    ERR("dcecs_no_update                                  %x\n", hdr->mpeg4_header.dcecs_no_update);
    ERR("dcecs_upsampling                                 %x\n", hdr->mpeg4_header.dcecs_upsampling);
    ERR("dcecs_intra_blocks                               %x\n", hdr->mpeg4_header.dcecs_intra_blocks);
    ERR("dcecs_not_coded_blocks                           %x\n", hdr->mpeg4_header.dcecs_not_coded_blocks);
    ERR("dcecs_dct_coefs                                  %x\n", hdr->mpeg4_header.dcecs_dct_coefs);
    ERR("dcecs_dct_lines                                  %x\n", hdr->mpeg4_header.dcecs_dct_lines);
    ERR("dcecs_vlc_symbols                                %x\n", hdr->mpeg4_header.dcecs_vlc_symbols);
    ERR("dcecs_vlc_bits                                   %x\n", hdr->mpeg4_header.dcecs_vlc_bits);
    ERR("dcecs_sadct                                      %x\n", hdr->mpeg4_header.dcecs_sadct);
    ERR("dcecs_inter_blocks                               %x\n", hdr->mpeg4_header.dcecs_inter_blocks);
    ERR("dcecs_inter4v_blocks                             %x\n", hdr->mpeg4_header.dcecs_inter4v_blocks);
    ERR("dcecs_apm                                        %x\n", hdr->mpeg4_header.dcecs_apm);
    ERR("dcecs_npm                                        %x\n", hdr->mpeg4_header.dcecs_npm);
    ERR("dcecs_forw_back_mc_q                             %x\n", hdr->mpeg4_header.dcecs_forw_back_mc_q);
    ERR("dcecs_halfpel2                                   %x\n", hdr->mpeg4_header.dcecs_halfpel2);
    ERR("dcecs_halfpel4                                   %x\n", hdr->mpeg4_header.dcecs_halfpel4);
    ERR("dcecs_quarterpel                                 %x\n", hdr->mpeg4_header.dcecs_quarterpel);
    ERR("dcecs_interpolate_mc_q                           %x\n", hdr->mpeg4_header.dcecs_interpolate_mc_q);
    ERR("temporal_reference                               %x\n", hdr->mpeg4_header.temporal_reference);
    ERR("zero_bit                                         %x\n", hdr->mpeg4_header.zero_bit);
    ERR("split_screen_indicator                           %x\n", hdr->mpeg4_header.split_screen_indicator);
    ERR("document_camera_indicator                        %x\n", hdr->mpeg4_header.document_camera_indicator);
    ERR("full_picture_freeze_release                      %x\n", hdr->mpeg4_header.full_picture_freeze_release);
    ERR("source_format                                    %x\n", hdr->mpeg4_header.source_format);
    ERR("picture_coding_type                              %x\n", hdr->mpeg4_header.picture_coding_type);
    ERR("four_reserved_zero_bits                          %x\n", hdr->mpeg4_header.four_reserved_zero_bits);
    ERR("pei                                              %x\n", hdr->mpeg4_header.pei);
    ERR("psupp                                            %x\n", hdr->mpeg4_header.psupp);
    ERR("num_gobs_in_vop                                  %x\n", hdr->mpeg4_header.num_gobs_in_vop);
    ERR("num_macroblocks_in_gob                           %x\n", hdr->mpeg4_header.num_macroblocks_in_gob);
    ERR("gob_frame_id                                     %x\n", hdr->mpeg4_header.gob_frame_id);
    ERR("quant_scale                                      %x\n", hdr->mpeg4_header.quant_scale);
    ERR("resync_marker                                    %x\n", hdr->mpeg4_header.resync_marker);
    ERR("header_extension_code                            %x\n", hdr->mpeg4_header.header_extension_code);
    ERR("macroblock_number                                %x\n", hdr->mpeg4_header.macroblock_number);
    ERR("short_video_header                               %x\n", hdr->mpeg4_header.short_video_header);
}

static void core_mpeg4_data_dump(unsigned int addr, unsigned int size)
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
int wmt_mpeg4_get_capability(mpeg4_drvinfo_t *drv)
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
int wmt_mpeg4_set_attr(mpeg4_drvinfo_t *drv)
{
    mpeg4_hdr_info_t  *hdr_in   = &(drv->attr.hdr_info);
    mpeg4_frameinfo_t *ret_info = &mpeg4info;
    unsigned int mb_width = 0, mb_height = 0;
    unsigned int frame_width = 0, frame_height = 0;
    unsigned int frame_fwidth = 0, frame_fheight = 0;
    TRACE("Enter\n");

    //mpeg4_hdr_dbg(hdr_in); //dump hdr parsing info

    // Width & Height
    frame_width   = (hdr_in->mpeg4_header.video_object_layer_width);
    frame_height  = (hdr_in->mpeg4_header.video_object_layer_height);

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
    DBG_DETAIL("vop_coding_type:%8d\n", hdr_in->mpeg4_header.vop_coding_type);

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
int wmt_mpeg4_decode_proc(mpeg4_drvinfo_t *drv)
{
    mpeg4_input_t     *input_in  = &(drv->arg_in);
    mpeg4_frameinfo_t *ret_info  = &mpeg4info;
    unsigned int  ref_prd_phy, *refprdvirt;
    unsigned int  w1, w2, w3;
    TRACE("Enter\n");

    //core_mpeg4_data_dump(input_in->src_addr, input_in->src_size);
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
    *(refprdvirt+1) = ((input_in->dst_y_size)/(input_in->linesize))+64;// FB height
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
    do_gettimeofday(&mp4_start);
    TIME("Start sec: %ld, msec: %ld\n", mp4_start.tv_sec, mp4_start.tv_usec/1000);
    //==============================================
#endif

    wmt_vd_timer_start(VD_MPEG4);
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
int wmt_mpeg4_decode_finish(mpeg4_drvinfo_t *drv)
{
    mpeg4_frameinfo_t *info_out = &(drv->arg_out);
    mpeg4_frameinfo_t *ret_info = &mpeg4info;
    dsplib_mbox_t mbox;
    dsp_done_t *dspinfo;
    int retval = 0;
    TRACE("Enter\n");

    //send cmd to DSP
    retval = dsplib_cmd_recv(&mbox, CMD_D2A_VDEC_DONE);
    DSPP("CMD_D2A_VDEC_DONE w1:w2:w3 0x%x 0x%x 0x%x\n", mbox.w1, mbox.w2, mbox.w3);
    wmt_vd_timer_stop(VD_MPEG4);

#ifdef TIME_JUDGE
    //==============================================
    //Time Measure
    do_gettimeofday(&mp4_end);
    TIME("End   sec: %ld, msec: %ld\n", mp4_end.tv_sec, mp4_end.tv_usec/1000);
    //==============================================
    /* unit in us */
    if (mp4_start.tv_sec == mp4_end.tv_sec){
        mp4_this_time = (mp4_end.tv_usec - mp4_start.tv_usec);
    }
    else {
        mp4_this_time = (mp4_end.tv_sec - mp4_start.tv_sec) * 1000000 + (mp4_end.tv_usec - mp4_start.tv_usec);
    }
    if (mp4_this_time < 0){
        TIME("Start sec: %ld, msec: %ld\n", mp4_start.tv_sec, mp4_start.tv_usec/1000);
        TIME("End   sec: %ld, msec: %ld\n", mp4_end.tv_sec, mp4_end.tv_usec/1000);
    }

    mp4_total_tm += mp4_this_time;
    mp4_interval_tm += mp4_this_time;
    mp4_total_cnt++;
    mp4_count++;

    if (mp4_this_time >= mp4_max) mp4_max = mp4_this_time;

    if (mp4_this_time <= mp4_min) mp4_min = mp4_this_time;

    if (mp4_this_time > mp4_threshold){
        TIME(" (%d) Decode time(%d) over %d (msec)\n", mp4_total_cnt, mp4_this_time/1000, mp4_threshold/1000);
    }
    if ((mp4_reset != 0) && (mp4_count >= mp4_reset)){
        TIME("=================================================\n");
        TIME(" Avg. time   = %d (msec)\n", mp4_interval_tm/mp4_count/1000);
        TIME("(~ %d) Decode Time Range[%d ~ %d](msec)\n", mp4_total_cnt, mp4_min/1000, mp4_max/1000);
        mp4_count = mp4_interval_tm = 0;
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
                (ret_info->Decoded_status) = MP4_STA_DEC_DONE;
                MPEG4_SET_STATUS(drv, STA_DECODE_DONE);
                DBG("deocded ok\n");
                retval = 0;
            }
            else {
                (ret_info->Decoded_status) = MP4_STA_ERR_DECODING;
                MPEG4_SET_STATUS(drv, STA_ERR_DECODING);
                ERR("deocded fail\n");
                retval = -1;
            }
        }
        else{
            (ret_info->Decoded_status) = MP4_STA_ERR_DECODING;
            MPEG4_SET_STATUS(drv, STA_ERR_DECODING);
            ERR("Unexpected DSP returned addr! 0x%x : 0x%x\n", mbox.w1, (vdlib_prd_phy() + DSP_OFFSET));
            retval = -1;
        }
    }
    else{
        (ret_info->Decoded_status) = MP4_STA_FMT_NOT_SUPPORT;
        MPEG4_SET_STATUS(drv, STA_ERR_DECODING);
        ERR("expected DSP returned value!\n");
        retval = -1;
    }

    memcpy(info_out, &mpeg4info, sizeof(mpeg4_frameinfo_t));

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
int wmt_mpeg4_open(mpeg4_drvinfo_t *drv)
{
    int ret = 0;
    TRACE("Enter\n");
    mpeg4_var_reset();
    //send cmd to DSP
    ret = dsplib_cmd_send(CMD_A2D_VDEC_OPEN, VD_MPEG4, 0, 0);

    wmt_vd_timer_init(VD_MPEG4, 0 /*frames*/, 100 /*ms*/);

#ifdef TIME_JUDGE
    //==============================================
    //Time Measure
    memset(&mp4_start, 0, sizeof(mp4_start));
    memset(&mp4_end, 0, sizeof(mp4_end));
    mp4_threshold = 500 * 1000; /* X ms => us */
    mp4_max       = 0;
    mp4_min       = 0xFFFFFFF;
    mp4_reset = mp4_count = mp4_interval_tm = mp4_total_tm = mp4_total_cnt = 0;
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
int wmt_mpeg4_close(void)
{
    TRACE("Enter\n");
    mpeg4_var_reset();
    //send cmd to DSP
    dsplib_cmd_send(CMD_A2D_VDEC_CLOSE, VD_MPEG4, 0, 0);
#ifdef TIME_JUDGE
    //==============================================
    //Time Measure
    if (mp4_total_cnt){
        TIME("=== Timer status:\n");
        TIME("Total count = %d \n", mp4_total_cnt);
        TIME("Total time  = %d (msec)\n", mp4_total_tm/1000);
        TIME("Avg. time   = %d (msec)\n", mp4_total_tm/mp4_total_cnt/1000);        
        TIME("Max time    = %d (msec)\n", mp4_max/1000);
        TIME("Min time    = %d (msec)\n", mp4_min/1000);
        TIME("==========================================\n");
    }
    memset(&mp4_start, 0, sizeof(mp4_start));
    memset(&mp4_end, 0, sizeof(mp4_end));
    mp4_threshold = 0;
    mp4_max       = 0;
    mp4_min       = 0;
    mp4_reset = mp4_count = mp4_interval_tm = mp4_total_tm = mp4_total_cnt = 0;
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
static int ioctl_get_capability(mpeg4_drvinfo_t *drvinfo, mpeg4_capability_t *capab)
{
    TRACE("Enter\n");
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
    ==========================================================================*/
    if (wmt_mpeg4_get_capability(drvinfo) == 0){
        memcpy(capab, drvinfo->capab_tbl, sizeof(mpeg4_capability_t));
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
static int ioctl_set_attr(mpeg4_drvinfo_t *drvinfo, mpeg4_attr_t *attr)
{
    int ret;
    TRACE("Enter\n");
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_ATTR"
    ==========================================================================*/
    memcpy(&drvinfo->attr, attr, sizeof(mpeg4_attr_t));
    DBG("Width:         %d\n", drvinfo->attr.hdr_info.mpeg4_header.video_object_layer_width);
    DBG("Height:        %d\n", drvinfo->attr.hdr_info.mpeg4_header.video_object_layer_height);
    DBG("version:       %d\n", drvinfo->attr.hdr_info.version);
    MPEG4_SET_STATUS(drvinfo, STA_ATTR_SET);
    ret = wmt_mpeg4_set_attr(drvinfo);
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
static int ioctl_decode_proc(mpeg4_drvinfo_t *drvinfo, mpeg4_input_t *arg)
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
    memcpy(&drvinfo->arg_in, arg, sizeof(mpeg4_input_t));
    //build prd table
    if (user_to_prdt(arg->src_addr, arg->src_size, (struct prdt_struct *)drvinfo->prd_virt, MAX_INPUT_BUF_SIZE/sizeof(struct prdt_struct))){
        ERR("input buffer to prd table error!!\n");
        MPEG4_SET_STATUS(drvinfo, STA_ERR_BAD_PRD);
        return -EFAULT;
    }
    MPEG4_SET_STATUS(drvinfo, STA_DECODEING);
    ret = wmt_mpeg4_decode_proc(drvinfo);
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
static int ioctl_decode_finish(mpeg4_drvinfo_t *drvinfo, mpeg4_frameinfo_t *frame_info)
{
    int ret;
    TRACE("Enter\n");
    /*==========================================================================
        This function is used to simulate when driver receive the IOCTL
        "VDIOSET_DECODE_FINISH"
    ==========================================================================*/
    memcpy(&drvinfo->arg_out, frame_info, sizeof(mpeg4_frameinfo_t));
    ret = wmt_mpeg4_decode_finish(drvinfo);
    memcpy(frame_info, &drvinfo->arg_out, sizeof(mpeg4_frameinfo_t));
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
static int mpeg4_ioctl(struct inode *inode, struct file  *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0, retval = 0;
    unsigned long  flags =0;
    mpeg4_drvinfo_t     *drvinfo;
    mpeg4_capability_t  capab;
    mpeg4_attr_t        attr;
    mpeg4_input_t       input;
    mpeg4_frameinfo_t   frame_info;

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
    spin_lock_irqsave(&mpeg4_lock, flags);
    drvinfo = (mpeg4_drvinfo_t *)filp->private_data;
    switch (cmd){       
      case VDIOSET_DECODE_INFO:
           DBG("Receive IOCTL:VDIOSET_DECODE_INFO\n");
           retval = copy_from_user(&attr, (const void *)arg, sizeof(mpeg4_attr_t));
           if (retval){
               ERR("copy_from_user FAIL in VDIOSET_DECODE_INFO!\n");
               break;
           }
           retval = IOCTL_CMD_CHECK(&attr, mpeg4_attr_t, VD_MPEG4);
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
           retval = copy_from_user((mpeg4_input_t *)&input, (const void *)arg, sizeof(mpeg4_input_t));
           if (retval){
               ERR("copy_from_user FAIL in VDIOSET_DECODE_PROC!\n");   
               break;
           }
           retval = IOCTL_CMD_CHECK(&input, mpeg4_input_t, VD_MPEG4);
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
               down_interruptible(&mpeg4_decoding_sema);
               /*--------------------------------------------------------------
                   Before HW decoding finish, we cannot decode another MPEG4
               --------------------------------------------------------------*/
               spin_unlock_irqrestore(&mpeg4_lock, flags);
               retval = ioctl_decode_proc(drvinfo, &input);
               spin_lock_irqsave(&mpeg4_lock, flags);
           }
           break;
      case VDIOGET_DECODE_FINISH:
           DBG("Receive IOCTL:VDIOGET_DECODE_FINISH\n");
           if (MPEG4_GET_STATUS(drvinfo) == STA_ERR_BAD_PRD){
               retval = -1;
               break;
           }
           retval = copy_from_user((vd_ioctl_cmd*)&frame_info, (const void *)arg, sizeof(vd_ioctl_cmd));
           if (retval){
               ERR("copy_from_user FAIL in VDIOGET_DECODE_FINISH!\n");   
               break;
           }
           retval = IOCTL_CMD_CHECK(&frame_info, mpeg4_frameinfo_t, VD_MPEG4);
           if (retval){
               ERR("Version conflict (0x%x) in VDIOGET_DECODE_FINISH\n", retval);
               DBG("identity: 0x%x, size:%d\n", frame_info.identity, frame_info.size);
               retval = -ENOTTY;
           }
           else {
               spin_unlock_irqrestore(&mpeg4_lock, flags);
               retval = ioctl_decode_finish(drvinfo, &frame_info);
               copy_to_user((mpeg4_frameinfo_t *)arg, &frame_info, sizeof(mpeg4_frameinfo_t));
               spin_lock_irqsave(&mpeg4_lock, flags);
               /*--------------------------------------------------------------
                   Before HW decoding finish, we cannot decode another MPEG4
               --------------------------------------------------------------*/
               up(&mpeg4_decoding_sema); 
           }
           break;
      case VDIOGET_CAPABILITY:
           DBG("Receive IOCTL:VDIOGET_CAPABILITY\n");
           retval = copy_from_user((vd_ioctl_cmd*)&capab, (const void *)arg, sizeof(vd_ioctl_cmd));
           if (retval){
               ERR("copy_from_user FAIL in VDIOGET_CAPABILITY!\n");   
               break;
           }
           retval = IOCTL_CMD_CHECK(&capab, mpeg4_capability_t, VD_MPEG4);
           if (retval){
               ERR("Version conflict (0x%x) in VDIOGET_CAPABILITY\n", retval);
               DBG("identity: 0x%x, size:%d\n", capab.identity, capab.size);
               retval = -ENOTTY;
           }
           else {
               retval = ioctl_get_capability(drvinfo, &capab);
               if (retval == 0){
                   retval = copy_to_user((mpeg4_capability_t *)arg, &capab, sizeof(mpeg4_capability_t));
               }
           }
           break;
      default:
           ERR("Unknown IOCTL:0x%x in mpeg4_ioctl()!\n", cmd);
           retval = -ENOTTY;
           break;
    } /* switch(cmd) */
    spin_unlock_irqrestore(&mpeg4_lock, flags);
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
static int mpeg4_probe(void)
{
    TRACE("Enter\n");
    spin_lock_init(&mpeg4_lock);
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
static int mpeg4_open(struct inode *inode, struct file *filp)
{
    mpeg4_drvinfo_t *drvinfo;
    struct videodecoder_info *vd_info;
    int ret = 0;
    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1: Initial Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&mpeg4_lock);
    if (mpeg4_dev_ref){
        /* Currently we do not support multi-open */
        spin_unlock(&mpeg4_lock);
        return -EBUSY;
    }   
    mpeg4_dev_ref++;
    spin_unlock(&mpeg4_lock);
    /*--------------------------------------------------------------------------
        Step 2: Initial hardware decoder
    --------------------------------------------------------------------------*/
    drvinfo = kmalloc(sizeof(mpeg4_drvinfo_t), GFP_KERNEL);
    if (drvinfo == 0){
        ERR("Allocate %d bytes fail!\n", sizeof(mpeg4_drvinfo_t));
        ret = -EBUSY;
        goto EXIT_mpeg4_open;
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
    drvinfo->size      = sizeof(mpeg4_drvinfo_t);
    drvinfo->_lock     = mpeg4_lock;
    MPEG4_SET_STATUS(drvinfo, STA_READY);
    ret = wmt_mpeg4_open(drvinfo);
EXIT_mpeg4_open:
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
static int mpeg4_release(struct inode *inode, struct file *filp)
{
    mpeg4_drvinfo_t *drvinfo = (mpeg4_drvinfo_t *)filp->private_data;
    TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 0: Check status
    --------------------------------------------------------------------------*/
    if (MPEG4_GET_STATUS(drvinfo) == STA_DECODEING){
        WARNING("Abnormally exit when driver still decoding.\n");
        up(&mpeg4_decoding_sema); 
    }
    /*--------------------------------------------------------------------------
        Step 1: Close hardware decoder
    --------------------------------------------------------------------------*/
    MPEG4_SET_STATUS(drvinfo, STA_CLOSE);
    wmt_mpeg4_close();
    if (filp->private_data){
        kfree(filp->private_data);
    }
    /*--------------------------------------------------------------------------
        Step 2: Release Linux settings
    --------------------------------------------------------------------------*/
    spin_lock(&mpeg4_lock);
    if (!mpeg4_dev_ref){
        spin_unlock(&mpeg4_lock);
        return -EBUSY;
    }   
    mpeg4_dev_ref--;
    spin_unlock(&mpeg4_lock);
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
static int mpeg4_setup(void)
{
    TRACE("Enter\n");
    TRACE("Leave\n");
    return mpeg4_probe();
}

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   remove module
* \retval  0 if success
*/
static int mpeg4_remove(void)
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
static int mpeg4_suspend(pm_message_t state)
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
static int mpeg4_resume(void)
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
static int mpeg4_get_info(char *buf, char **start, off_t offset, int len)
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

struct videodecoder mpeg4_decoder = {
    .name    = DRIVER_NAME,
    .id      = VD_MPEG4,
    .setup   = mpeg4_setup,
    .remove  = mpeg4_remove,
    .suspend = mpeg4_suspend,
    .resume  = mpeg4_resume,
    .fops    = {
                .owner   = THIS_MODULE,
                .open    = mpeg4_open,
                //.read    = mpeg4_read,
                //.write   = mpeg4_write,
                .ioctl   = mpeg4_ioctl,
                //.mmap    = mpeg4_mmap,
                .release = mpeg4_release,
               },
    .device = NULL,
#ifdef CONFIG_PROC_FS
    .get_info = mpeg4_get_info,
#endif /* CONFIG_PROC_FS */
};

/*!*************************************************************************
* Private Function by Welkin Chen, 2009/02/28
*
* \brief
*   init module
* \retval  0 if success
*/
static int hw_mpeg4_init(void)
{
    int ret;
    TRACE("Enter\n");
    ret = videodecoder_register(&mpeg4_decoder);
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
static void hw_mpeg4_exit(void)
{
    TRACE("Enter\n");
    videodecoder_unregister(&mpeg4_decoder);
    TRACE("Leave\n");
    return;
}

module_init(hw_mpeg4_init);
module_exit(hw_mpeg4_exit);
MODULE_AUTHOR("WonderMedia Multimedia SW Team Welkin Chen");
MODULE_DESCRIPTION("WMT MPEG4 decode device driver");
MODULE_LICENSE("GPL");
/*----------------------------------------------------------------------------*/
/*=== END ====================================================================*/
#endif
