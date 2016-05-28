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
* MODULE       : COM_MPEG4_H
* AUTHOR       : Welkin Chen
* DATE         : 2010/11/01
* DESCRIPTION  : All rights reserved.
*-----------------------------------------------------------------------------*/
/*--- History ------------------------------------------------------------------
* Version 0.01, Welkin Chen, 2009/06/12
* First version
*
* version 0.1, Welkin Chen, 2010/11/01
*
*-----------------------------------------------------------------------------*/
/*----------------- MODULE DEPENDENCY ----------------------------------------*/
#ifndef COM_MPEG4_H
#define COM_MPEG4_H
/* To assert that only one occurrence is included */
//------------------------------------------------
//  Include Header
//------------------------------------------------
#include "../com-vd.h"

/*----------------- EXPORTED PRIVATE CONSTANTS -------------------------------*/

/*----------------- EXPORTED PRIVATE MACROS ----------------------------------*/
#define MPEG4_VERSION     102   /* version: 0.01.02 */

#define IOCTL_MPEG4_INIT(cmd, _struct_type_) \
        memset((cmd), 0, sizeof(_struct_type_)); \
        IOCTL_CMD_INIT((vd_ioctl_cmd *)cmd, _struct_type_, VD_MPEG4, MPEG4_VERSION)

/*----------------- EXPORTED PRIVATE TYPES -----------------------------------*/
//Example// typedef void api_xxx_t;
typedef enum {
  MP4_STA_DECSTATUS_NONE  = 0x00000000,
  MP4_STA_SIZE_OVERFLOW   = 0x10000000,
  MP4_STA_FMT_NOT_SUPPORT = 0x20000000,
  MP4_STA_READY           = 0x01000000,
  MP4_STA_CLOSE           = 0x02000000,
  /* DEC Status */
  MP4_STA_DEC_RUN         = 0x00000010,
  MP4_STA_DEC_DONE        = 0x00000020,
  MP4_STA_DEC_SKIP        = 0x00000030,
  MP4_STA_DEC_PARSING     = 0x00000040,
  /* Error Status */
  MP4_STA_ERR_DECODING    = 0x00000001,
  MP4_STA_ERR_TIMEOUT     = 0x00000002,
  MP4_STA_ERR_UNKNOWN     = 0x00000003,
  /* Parsing Status */
  MP4_STA_SEQ_END         = 0x00001000,
  MP4_STA_ISO_END         = 0x00002000,
  MP4_STA_DECSTATUS_MAX   = 0xFFFFFFFF
} MP4_STATUS;

typedef enum {
  DIVX4 = 0x1, // DIVX 4.0
  DIVX5 = 0x2  // DIVX 5.0 (XVID)
} divx_version;

typedef struct motion_vector_s {
  int x;
  int y;
} motion_vector_t;

typedef struct mpeg4_hdr_s {
  //-----------------------------------------
  // Visual Object Sequence and Visual Object
  //-----------------------------------------
  //~~~// VisualObjectSequence()
  //unsigned int   S_visual_object_sequence_start_code;// 32 bits //*Global*//
  unsigned char  profile_and_level_indication;// 8 bits
  //unsigned int   S_visual_object_sequence_end_code;// 32 bits //*Global*//

  //~~~// VisualObject()
  //unsigned int   S_visual_object_start_code;// 32 bits //*Global*//
  unsigned char  is_visual_object_identifier;// 1 bit
  unsigned char  visual_object_verid;// 4 bits //*Must*//
  //Memo// the version number of the visual object

  unsigned char  visual_object_priority;// 3 bits
  unsigned char  visual_object_type;// 4 bits
  ///***************************
  // 0000 reserved
  // 0001 video ID
  // 0010 still texture ID
  // 0011 mesh ID
  // 0100 FBA ID
  // 0101 3D mesh ID
  // 0110 ~ 1111 reserved
  //***************************/
  //unsigned int   S_video_object_start_code;// 32 bits //*Global*//

  //~~~// video_signal_type()
  unsigned char  video_signal_type;// 1 bit
  unsigned char  video_format;// 3 bits
  //Memo// the representation of the pictures before being coded
  //Memo// If video_signal_type = 0 then the video format may be assumed to be "Unspecified video format"
  ///***************************
  // 000 Component
  // 001 PAL
  // 010 NTSC
  // 011 SECAM
  // 100 MAC
  // 101 Unspecified video format
  // 110 ~ 111 Reserved
  //***************************/

  unsigned char  video_range;// 1 bit
  //Memo// the black level and range of the luminance and chrominance signals

  unsigned char  colour_description;// 1 bit
  //Memo// the presence of colour_primaries, transfer_characteristics and matrix_coefficients

  unsigned char  colour_primaries;// 8 bits
  unsigned char  transfer_characteristics;// 8 bits
  unsigned char  matrix_coefficients;// 8 bits

  //-----------------------------------------
  // User data
  //-----------------------------------------
  //~~~// user_data()
  //unsigned int   S_user_data_start_code;// 32 bits //*Global*//
  unsigned int   user_data;// 8 bits //*Struct*//

  //-----------------------------------------
  // Visual Object Layer
  //-----------------------------------------
  //~~~// VideoObjectLayer()
  //v^v^v// Part1
  //unsigned int   S_video_object_layer_start_code;// 32 bits //*Global*//
  unsigned char  random_accessible_vol;// 1 bit
  //Memo// every VOP in this VOL is individually decodable

  unsigned char  video_object_type_indication;// 8 bits
  //Memo//Constrains the following bitstream to use tools from the indicated object type only

  unsigned char  is_object_layer_identifier;// 1 bit
  //Memo// version identification and priority is specified for the visual object layer

  unsigned char  video_object_layer_verid;// 4 bits //*Must*//
  //Memo// version number of the video object layer
  unsigned char  video_object_layer_priority;// 3 bits
  unsigned char  aspect_ratio_info;// 4 bits
  //Memo// defines the value of pixel aspect ratio
  ///***************************
  // 0000 0x0	Forbidden
  // 0001 0x1	1:1 (Square)
  // 0010 0x2	12:11 (625-type for 4:3 picture)           	<=>	PAR
  // 0011 0x3	10:11 (525-type for 4:3 picture)           	<=>	NTSC
  // 0100 0x4	16:11 (625-type stretched for 16:9 picture)	<=>	PAR
  // 0101 0x5	40:33 (525-type stretched for 16:9 picture)	<=>	NTSC
  // 0110 ~ 1110 reserved
  // 1111 0xf	extended PAR
  //***************************/

  unsigned char  par_width;// 8 bits
  unsigned char  par_height;// 8 bits
  //Memo// the horizontal and vertical size of pixel aspect ratio

  unsigned char  vol_control_parameters;// 1 bit
  //Memo// the presence of the following: chroma_format, low_delay, and vbv_parameters

  unsigned char  chroma_format;// 2 bits
  //Memo// the chrominance format
  ///***************************
  // 00 reserved
  // 01 4:2:0
  // 10 ~ 11 reserved
  //***************************/

  unsigned char  low_delay;// 1 bit
  //Memo// 1 the VOL contains no B-VOPs

  unsigned char  vbv_parameters;// 1 bit
  unsigned short first_half_bit_rate;// 15 bits
  unsigned short latter_half_bit_rate;// 15 bits
  unsigned short first_half_vbv_buffer_size;// 15 bits
  unsigned char  latter_half_vbv_buffer_size;// 3 bits
  unsigned short first_half_vbv_occupancy;// 11 bits
  unsigned short latter_half_vbv_occupancy;// 15 bits

  //v^v^v// Part2
  unsigned char  video_object_layer_shape;// 2 bits //*Must*//
  //Memo// the shape type of a video object layer
  ///***************************
  // 00 rectangular
  // 01 binary
  // 10 binary only
  // 11 grayscale
  //***************************/

  unsigned char  video_object_layer_shape_extension;// 4 bits
  //Memo// the number (up to 3) and type of auxiliary components that can be used
  ///***************************
  // video_object_layer_shape_extension		aux_comp_count
  //  0000 0001 0101 0111 1000              1
  //  0010 0011 1011 0110 1001              2
  //  0100 1010 1100                        3
  //  1101-1111                            TBD
  //***************************/

  unsigned short vop_time_increment_resolution;// 16 bits //*Must*//
  //Memo// the number of evenly spaced subintervals,
        // called ticks, within one modulo time

  unsigned char  fixed_vop_rate;//1 bits
  //Memo// all VOPs are coded with a fixed VOP rate

  unsigned short fixed_vop_time_increment;// 1-16 bits //*Must*//
  //Memo// this value represents the number of ticks
        // between two successive VOPs in the display order

  unsigned short video_object_layer_width;// 13 bits //*Must*//
  unsigned short video_object_layer_height;// 13 bits //*Must*//
  //Memo// the width/height of the displayable part of the luminance in pixel
  //Memo// in macroblocks is (video_object_layer_W/H+15)/16

  unsigned char  interlaced;// 1 bit //*Must*//
  //Memo// the VOP may contain interlaced video

  unsigned char  obmc_disable;// 1 bit
  //Memo// disable overlapped block motion compensation
  
  unsigned char  sprite_enable;// 1-2 bits //*Must*//
  //Memo// According video_object_layer_verid == "0001" or "0002"
  //Memo// the usage of static sprite coding or GMC
  ///***************************
  // 0  00 sprite not used
  // 1  01 static (Basic/Low Latency)
  // -  10 GMC (Global Motion Compensation)
  // -  11 Reserved
  //***************************/

  unsigned short sprite_width;// 13 bits
  unsigned short sprite_height;// 13 bits
  //Memo// the horizontal and vertical dimension of the sprite

  unsigned short sprite_left_coordinate;// 13 bits
  unsigned short sprite_top_coordinate;// 13 bits
  //Memo// the edge of the sprite. The value shall be divisible by two

  unsigned char  no_of_sprite_warping_points;// 6 bits //*Must*//
  unsigned char  sprite_warping_accuracy;// 2 bits //*Must*//
  //Memo// the quantisation accuracy of motion vectors
        // used in the warping process for sprites and GMC
  ///***************************
  // 00 1/2 pixel
  // 01 1/4 pixel
  // 10 1/8 pixel
  // 11 1/16 pixel
  ///***************************/

  unsigned char  sprite_brightness_change;// 1 bit //*Must*//
  unsigned char  low_latency_sprite_enable;// 1 bit //*Must*//
  unsigned char  sadct_disable;// 1 bit
  unsigned char  not_8_bit;// 1 bit //*Must*//
  //Memo// the video data precision is not 8 bits per pixel
        // and visual object type is N-bit

  unsigned char  quant_precision;// 4 bits
  //Memo// represent quantiser parameters. Values between 3 and 9.
  //Memo// If not_8_bit = 0, quant_precision is not transmitted,
        // it takes a default value of 5

  unsigned char  bits_per_pixel;// 4 bits
  unsigned char  no_gray_quant_update;// 1 bit
  unsigned char  composition_method;// 1 bit
  unsigned char  linear_composition;// 1 bit
  unsigned char  quant_type;// 1 bit //*Must*//
  //Memo// 1 first inverse quantisation method,
        // 0 second inverse quantisation method

  unsigned char  load_intra_quant_mat;// 1 bit
  unsigned char  intra_quant_mat[64];// 8*[2-64] bits //*Struct*//
  unsigned char  load_nonintra_quant_mat;// 1 bit
  unsigned char  nonintra_quant_mat[64];// 8*[2-64] bits //*Struct*//
  unsigned char  load_intra_quant_mat_grayscale;// 1 bit
  unsigned char  intra_quant_mat_grayscale[64];// 8*[2-64] bits //*Struct*//
  unsigned char  load_nonintra_quant_mat_grayscale;// 1 bit
  unsigned char  nonintra_quant_mat_grayscale[64];// 8*[2-64] bits //*Struct*//

  //v^v^v// Part3
  unsigned char  quarter_sample;// 1 bit //*Must*//
  //Memo// 0 half sample mode, 1 quarter sample mode

  unsigned char  complexity_estimation_disable;// 1 bit //*Must*//
  unsigned char  resync_marker_disable;// 1 bit //*Must*//
  unsigned char  data_partitioned;// 1 bit //*Must*//
  unsigned char  reversible_vlc;// 1 bit
  unsigned char  newpred_enable;// 1 bit //*Must*//
  unsigned char  requested_upstream_message_type;// 2 bits
  unsigned char  newpred_segment_type;// 1 bit
  unsigned char  reduced_resolution_vop_enable;// 1 bit //*Must*//
  unsigned char  scalability;// 1 bit //*Must*//
  //Memo// 1 current layer uses scalable coding,
  			// 0 current layer uses base-layer
  //Memo// for S(GMC)-VOPs shall be set to 0

  unsigned char  hierarchy_type;// 1 bit
  //Memo// 0 Spatial Scalability, 1 Temporal Scalability

  unsigned char  ref_layer_id;// 4 bits
  //Memo// the layer to be used as reference for prediction(s)
        // in the case of scalability

  unsigned char  ref_layer_sampling_direc;// 1 bit
  //Memo// 1 the resolution of the reference layer is higher
        // than the resolution of the layer being coded

  unsigned char  hor_sampling_factor_n;// 5 bits
  unsigned char  hor_sampling_factor_m;// 5 bits
  unsigned char  vert_sampling_factor_n;// 5 bits
  unsigned char  vert_sampling_factor_m;// 5 bits
  unsigned char  enhancement_type;// 1 bit //*Must*//
  //Memo// current layer enhances the partial region of the reference layer

  unsigned char  use_ref_shape;// 1 bit
  unsigned char  use_ref_texture;// 1 bit
  unsigned char  shape_hor_sampling_factor_n;// 5 bits
  unsigned char  shape_hor_sampling_factor_m;// 5 bits
  unsigned char  shape_vert_sampling_factor_n;// 5 bits
  unsigned char  shape_vert_sampling_factor_m;// 5 bits

  //unsigned int   S_stuffing_start_code;// 32 bits //*Global*//
  //S//	stuffing_byte;// 8 bits //*Struct*//
  //Memo// the 8-bit string in which the value is "11111111"

  //~~~// define_vop_complexity_estimation_header()
  unsigned char  estimation_method;// 2 bits //*Must*//
  //Memo// 00 Version 1,  01 version 2

  unsigned char  shape_complexity_estimation_disable;// 1 bit
  unsigned char  opaque;// 1 bit //*Must*//
  unsigned char  transparent;// 1 bit //*Must*//
  unsigned char  intra_cae;// 1 bit //*Must*//
  unsigned char  inter_cae;// 1 bit //*Must*//
  unsigned char  no_update;// 1 bit //*Must*//
  unsigned char  upsampling;// 1 bit //*Must*//
  //Memo// enabling transmission of the number of luminance and chrominance blocks coded
  //Memo// using opaque or transparent or IntraCAE or InterCAE
        // or no updateor or upsampling coding mode in % of the total number

  unsigned char  texture_complexity_estimation_set_1_disable;// 1 bit
  unsigned char  intra_blocks;// 1 bit //*Must*//
  unsigned char  inter_blocks;// 1 bit //*Must*//
  unsigned char  inter4v_blocks;// 1 bit //*Must*//
  unsigned char  not_coded_blocks;// 1 bit //*Must*//
  unsigned char  texture_complexity_estimation_set_2_disable;// 1 bit
  unsigned char  dct_coefs;// 1 bit //*Must*//
  unsigned char  dct_lines;// 1 bit //*Must*//
  unsigned char  vlc_symbols;// 1 bit //*Must*//
  unsigned char  vlc_bits;// 1 bit //*Must*//
  unsigned char  motion_compensation_complexity_disable;// 1 bit
  unsigned char  apm;// 1 bit //*Must*//
  //Memo// Advanced Prediction Mode

  unsigned char  npm;// 1 bit //*Must*//
  //Memo// Normal Prediction Mode

  unsigned char  interpolate_mc_q;// 1 bit //*Must*//
  unsigned char  forw_back_mc_q;// 1 bit //*Must*//
  unsigned char  halfpel2;// 1 bit //*Must*//
  unsigned char  halfpel4;// 1 bit //*Must*//
  unsigned char  version2_complexity_estimation_disable;// 1 bit
  unsigned char  sadct;// 1 bit //*Must*//
  unsigned char  quarterpel;// 1 bit //*Must*//
  //Memo// enabling transmission of the number of luminance and chrominance block
        // predicted by a quarterpel vector on one or two dimensions

  //-----------------------------------------
  // Group of Video Object Plane
  //-----------------------------------------
  //~~~// Group_of_VideoObjectPlane()
  //unsigned int   S_group_of_vop_start_code;// 32 bits //*Global*//
  unsigned int   time_code;// 18 bits
  unsigned char  closed_gov;// 1 bit
  unsigned char  broken_link;// 1 bit

  //-----------------------------------------
  // Video Object Plane and Video Plane with Short Header
  //-----------------------------------------
  //~~~// VideoObjectPlane()
  //unsigned int   S_vop_start_code;// 32 bits //*Global*//
  unsigned char  vop_coding_type;// 2 bits //*Must*//
  ///***************************
  // 00 intra-coded (I)
  // 01 predictive-coded (P)
  // 10 bidirectionally-predictive-coded (B)
  // 11 sprite (S)
  //***************************/

  unsigned char  modulo_time_base;// 1 bit //*Must*//
  unsigned short vop_time_increment;// 1-16 bits //*Must*//
  unsigned char  vop_coded;// 1 bit
  //Memo// 0 no subsequent data exists for the VOP

  unsigned short vop_id;// 4-15 bits
  unsigned char  vop_id_for_prediction_indication;// 1 bit
  unsigned short vop_id_for_prediction;// 4-15 bits
  unsigned char  vop_rounding_type;// 1 bit //*Must*//
  unsigned char  vop_reduced_resolution;// 1 bit
  unsigned short vop_width;// 13 bits //*Must*//
  unsigned short vop_height;// 13 bits //*Must*//
  //Memo// the horizontal and vertical size of the rectangle that includes the VOP
  //Memo// the width and height of the encoded luminance component
        // of VOP in macroblocks is (vop_H/W+15)/16.

  unsigned short vop_horizontal_mc_spatial_ref;// 13 bits
  unsigned short vop_vertical_mc_spatial_ref;// 13 bits
  unsigned char  background_composition;// 1 bit
  unsigned char  change_conv_ratio_disable;// 1 bit
  unsigned char  vop_constant_alpha;// 1 bit
  unsigned char  vop_constant_alpha_value;// 8 bits
  unsigned char  intra_dc_vlc_thr;// 3 bits //*Must*//
  unsigned char  top_field_first;// 1 bit
  unsigned char  alternate_vertical_scan_flag;// 1 bit //*Must*//
  unsigned char  sprite_transmit_mode;// 2 bits //*Must*//
  //Memo// the transmission mode of the sprite object
  ///***************************
  // 00 stop
  // 01 piece
  // 10 update
  // 11 pause
  //***************************/

  motion_vector_t warping_points[4];

  unsigned short vop_quant;// 3-9 bits //*Must*//
  //S//	vop_alpha_quant[i] ;// 6 bits //*Struct*//
  unsigned char  vop_fcode_forward;// 3 bits //*Must*//
  unsigned char  vop_fcode_backward;// 3 bits //*Must*//
  unsigned char  vop_shape_coding_type;// 1 bit
  unsigned char  load_backward_shape;// 1 bit
  unsigned short backward_shape_width;// 13 bits
  unsigned short backward_shape_height;// 13 bits
  unsigned short backward_shape_horizontal_mc_spatial_ref;// 13 bits
  unsigned short backward_shape_vertical_mc_spatial_ref;// 13 bits
  unsigned char  load_forward_shape;// 1 bit
  unsigned short forward_shape_width;// 13 bits
  unsigned short forward_shape_height;// 13 bits
  unsigned short forward_shape_horizontal_mc_spatial_ref;// 13 bits
  unsigned short forward_shape_vertical_mc_spatial_ref;// 13 bits
  unsigned char  ref_select_code;// 2 bits

  //~~~// Complexity Estimation Header()
  unsigned char  dcecs_opaque;// 8 bits
  unsigned char  dcecs_transparent;// 8 bits
  unsigned char  dcecs_intra_cae;// 8 bits
  unsigned char  dcecs_inter_cae;// 8 bits
  unsigned char  dcecs_no_update;// 8 bits
  unsigned char  dcecs_upsampling;// 8 bits
  unsigned char  dcecs_intra_blocks;// 8 bits
  unsigned char  dcecs_not_coded_blocks;// 8 bits
  unsigned char  dcecs_dct_coefs;// 8 bits
  unsigned char  dcecs_dct_lines;// 8 bits
  unsigned char  dcecs_vlc_symbols;// 8 bits
  unsigned char  dcecs_vlc_bits;// 4 bits

  unsigned char  dcecs_sadct;// 8 bits

  unsigned char  dcecs_inter_blocks;// 8 bits
  unsigned char  dcecs_inter4v_blocks;// 8 bits
  unsigned char  dcecs_apm;// 8 bits
  unsigned char  dcecs_npm;// 8 bits
  unsigned char  dcecs_forw_back_mc_q;// 8 bits
  unsigned char  dcecs_halfpel2;// 8 bits
  unsigned char  dcecs_halfpel4;// 8 bits

  unsigned char  dcecs_quarterpel;// 8 bits

  unsigned char  dcecs_interpolate_mc_q;// 8 bits

  //-----------------------------------------
  // Video Plane with Short Header
  //-----------------------------------------
  //~~~// video_plane_with_short_header()
  //unsigned int   S_short_video_start_marker;// 22 bits //*Global*//
  //Memo// an abbreviated header format is used for video content.
  //Memo// indicates video data which begins with a short_video_start_marker
        // rather than a longer start code such as visual_object_ start_code.
  //Memo// short_video_start_marker shall be byte aligned
        // by the insertion of zero to seven zero-valued bits
        // as necessary to achieve byte alignment prior to short_video_start_marker

  unsigned char  temporal_reference;// 8 bits
  unsigned char  zero_bit;// 1 bit
  unsigned char  split_screen_indicator;// 1 bit
  unsigned char  document_camera_indicator;// 1 bit
  unsigned char  full_picture_freeze_release;// 1 bit
  unsigned char  source_format;// 3 bits
  //Memo//
  ///***************************
  // 000	reserved	vop-Width	vop-Height  num_gobs_in_vop num_macroblocks_in_gob
  // 001	sub-QCIF   128        96             6               8
  // 010	QCIF       176       144             9              11
  // 011	CIF        352       288            18              22
  // 100	4CIF       704       576            18              88
  // 101	16CIF     1408      1152            18             352
  // 110 ~ 111      reserved
  //***************************/

  unsigned char  picture_coding_type;// 1 bit //*Must*//
  //Memo// indicates the vop_coding_type.
  //Memo// 0 vop_coding_type is "I", 1 vop_coding_type is "P"

  unsigned char  four_reserved_zero_bits;// 4 bits
  //P//	vop_quant;// 5 bits //*Pass*//
  unsigned char  pei;// 1 bit
  unsigned char  psupp;// 8 bits
  //unsigned int   S_short_video_end_marker;// 22 bits //*Global*//

  //~~~// gob_layer()
  //unsigned int	gob_resync_marker;// 17 bits
  //unsigned char	gob_number;// 5 bits
  //Memo// indicates the location of video data within the video plane
        // A group of blocks (GOB)
  unsigned int   num_gobs_in_vop;
  unsigned int   num_macroblocks_in_gob;
  unsigned char  gob_frame_id;// 2 bits
  unsigned char  quant_scale;// 5 bits

  //~~~// video_packet_header()
  unsigned int   resync_marker;// 17-23 bits //*Must*//
  //Memo// at least 16 zero¡¦s followed by a one "0 0000 0000 0000 0001"
  unsigned char  header_extension_code;// 1 bit
  //*P*//	vop_width;// 13 bits //*Pass*//
  //*P*//	vop_height;// 13 bits //*Pass*//
  //*P*//	vop_horizontal_mc_spatial_ref;// 13 bits //*Pass*//
  //*P*//	vop_vertical_mc_spatial_ref;// 13 bits //*Pass*//
  unsigned short macroblock_number;// 1-14 bits //*Must*//
  //*P*// quant_scale;// 5 bits //*Pass*//
  //*P*// modulo_time_base;// 1 bit //*Pass*//
  //*P*// vop_time_increment;// 1-16 bits //*Pass*//
  //*P*//	vop_coding_type;// 2 bits //*Pass*//
  //*P*//	change_conv_ratio_disable;// 1 bit //*Pass*//
  //*P*//	vop_shape_coding_type;// 1 bit //*Pass*//
  //*P*//	intra_dc_vlc_thr;// 3 bits //*Pass*//
  //*P*//	vop_reduced_resolution;// 1 bit //*Pass*//
  //*P*//	vop_fcode_forward;// 3 bits //*Pass*//
  //*P*//	vop_fcode_backward;// 3 bits //*Pass*//
  //*P*//	vop_id;// 4-15 bits //*Pass*//
  //Memo// the ID of VOP which is incremented by 1 whenever a VOP is encoded

  //*P*//	vop_id_for_prediction_indication;// 1 bit //*Pass*//
  //*P*//	vop_id_for_prediction;// 4-15 bits //*Pass*//

  //~~~// Spec Define directly
  unsigned char  short_video_header; //*Must*//
} mpeg4_hdr_t;

typedef struct mpeg4_dec_s {
  mpeg4_hdr_t     mpeg4_header;
  divx_version    version;
  //-------------------------------------------------------------------------
  //==>// VOL part 2 //sprite_trajectory
  //==>//VideoObjectPlane
  int             Tframe;            // After VideoObjectLayer parsing (-1)
  unsigned short  pp_time;           // trd
  unsigned short  pb_time;           // trb
  unsigned short  pp_field_time;     // trdi
  unsigned short  pb_field_time;     // trbi
  unsigned int    display_time;
  int             virtual_ref[2][2];
  int             sprite_offset[2][2];
  int             sprite_delta[2][2];
  int             sprite_shift[2];
} mpeg4_hdr_info_t;

/*------------------------------------------------
    IOCTL
------------------------------------------------*/
typedef struct {
  VD_INPUT_ARG_M;
/*
  unsigned int    src_addr;           // input buffer addr in user space
  unsigned int    src_size;           // input buffer size
  unsigned int    flags;              // control flag for HW decoder
  // --- Output Frame buffer information ---
  unsigned int    dst_y_addr;         // output address in Y (Physical addr )
  unsigned int    dst_y_size;         // output buffer size in Y
  unsigned int    dst_c_addr;         // output address in C (Physical addr )
  unsigned int    dst_c_size;         // output buffer size in C
  unsigned int    linesize;           // Length of one scanline (unit: pixel) for YC and RGB planes
*/
  // Reference Frame
  unsigned int    prev_y_addr;        // Reference fordward frame y physical address
  unsigned int    prev_c_addr;        // Reference fordward frame c physical address

  unsigned int    next_y_addr;        // Reference backward frame y physical address
  unsigned int    next_c_addr;        // Reference backward frame c physical address

  unsigned int    bits_count;         // bits count from BS start to slice start
  // VLDInfo // Common
  unsigned short  data_D0;
  unsigned short  data_D1;
  unsigned short  data_D2;
  unsigned int    data_start_bit;
  unsigned int    vbsdma_2_4_byte;    // 0:2byte 1:4byte alignment
  unsigned int    data_vld_add;       // VBSDMA start addr //user?
  unsigned int    data_vld_size;      // VBSDMA data size
} mpeg4_input_t;

typedef struct {
  VD_FRAMEINFO_M;
/*------------------
  //vdo_framebuf_t  fb
    unsigned int   y_addr;  // Addr of Y plane in YUV domain or RGB plane in ARGB domain
    unsigned int   c_addr;  // C plane address
    unsigned int   y_size;  // Buffer size in bytes
    unsigned int   c_size;  // Buffer size in bytes
    unsigned int   img_w;   // width of valid image (unit: pixel)
    unsigned int   img_h;   // height of valid image (unit: line)
    unsigned int   fb_w;    // width of frame buffer (scanline offset) (unit: pixel)
    unsigned int   fb_h;    // height of frame buffer (unit: line)
    unsigned int   bpp;     // bits per pixel (8/16/24/32)
    vdo_color_fmt  col_fmt; // Color format on frame buffer
    unsigned int   h_crop;  // Horental Crop (unit: pixel)
    unsigned int   v_crop;  // Vertical Crop (unit: pixel)
------------------*/
  MP4_STATUS      Decoded_status; // Frame decoded status
  unsigned int    consumedbytes;  // consumed byte in VLD
} mpeg4_frameinfo_t;

typedef struct {
  VD_DECODE_ATTR_M;
  mpeg4_hdr_info_t hdr_info;
} mpeg4_attr_t;

typedef struct {
  VD_CAPABILITY_M;
  unsigned int chip_id;
} mpeg4_capability_t;

/*----------------- EXPORTED PRIVATE VARIABLES -------------------------------*/
// allocate memory for variables only in XXX.c
#ifdef COM_MPEG4_C
    #define EXTERN
#else
    #define EXTERN extern
#endif
//Example// EXTERN int api_xxxx;

#undef EXTERN

/*----------------- EXPORTED PRIVATE FUNCTIONS -------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
//Example// extern void api_xxxx(void);

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/*=== END ====================================================================*/
#endif
