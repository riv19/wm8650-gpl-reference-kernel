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
* MODULE       : COM_MPEG2.h
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
#ifndef COM_MPEG2_H
#define COM_MPEG2_H
/* To assert that only one occurrence is included */
//------------------------------------------------
//  Include Header
//------------------------------------------------
#include "../com-vd.h"

/*----------------- EXPORTED PRIVATE CONSTANTS -------------------------------*/

/*----------------- EXPORTED PRIVATE MACROS ----------------------------------*/
#define MPEG2_VERSION     102   /* version: 0.01.02 */

#define IOCTL_MPEG2_INIT(cmd, _struct_type_) \
        memset((cmd), 0, sizeof(_struct_type_)); \
        IOCTL_CMD_INIT((vd_ioctl_cmd *)cmd, _struct_type_, VD_MPEG, MPEG2_VERSION)

/*----------------- EXPORTED PRIVATE TYPES -----------------------------------*/
//Example// typedef void api_xxx_t;
typedef enum {
  MP2_STA_DECSTATUS_NONE  = 0x00000000,
  MP2_STA_SIZE_OVERFLOW   = 0x10000000,
  MP2_STA_FMT_NOT_SUPPORT = 0x20000000,
  MP2_STA_READY           = 0x01000000,
  MP2_STA_CLOSE           = 0x02000000,
  /* DEC Status */
  MP2_STA_DEC_RUN         = 0x00000010,
  MP2_STA_DEC_DONE        = 0x00000020,
  MP2_STA_DEC_SKIP        = 0x00000030,
  MP2_STA_DEC_PARSING     = 0x00000040,
  /* Error Status */
  MP2_STA_ERR_DECODING    = 0x00000001,
  MP2_STA_ERR_TIMEOUT     = 0x00000002,
  MP2_STA_ERR_UNKNOWN     = 0x00000003,
  /* Parsing Status */
  MP2_STA_SEQ_END         = 0x00001000,
  MP2_STA_ISO_END         = 0x00002000,
  MP2_STA_DECSTATUS_MAX   = 0xFFFFFFFF
} MP2_STATUS;

typedef enum {
  MPEG1 = 0x1, // MPEG1
  MPEG2 = 0x2  // MPEG2
} mpeg2_version;

typedef struct mpeg2_hdr_s {
  //-----------------------------------------
  // Sequence Header
  //-----------------------------------------
  //~~~// sequence_header()
  //unsigned int   S_sequence_header_start_code;// 32 bits //*Global*//
  unsigned short horizontal_size_value; // 12 bits //*Must*//
  unsigned short vertical_size_value; // 12 bits //*Must*//
  unsigned char  aspect_ratio_information; // 4 bits
  unsigned char  frame_rate_code; // 4 bits
  //Memo// frame rate
  ///***************************
  // 0000  Forbidden
  // 0001  24000/1001(23.97)
  // 0010  24
  // 0011  25
  // 0100  30000/1001(29.97)
  // 0101  30
  // 0110  50
  // 0111  60000/1001(59.94)
  // 1000  60
  //***************************/

  unsigned int   bit_rate_value; // 18 bits
  unsigned char  marker_bit; // 1 bits
  unsigned short vbv_buffer_size_value; // 10 bits
  unsigned char  constrained_parameters_flag; // 1 bits

  unsigned char  load_intra_quant_mat; // 1 bits
  unsigned char  intra_quant_mat[64]; // 8*64 bits
  unsigned char  load_non_intra_quant_mat; // 1 bits
  unsigned char  nonintra_quant_mat[64]; // 8*64 bits

  //~~~// sequence_extension()
  //Unsigned int   S_extension_start_code; // 32 bits //*Global*//
  unsigned char  extension_start_code_identifier; // 4 bits
  unsigned char  profile_and_level_indication; // 8 bits
  unsigned char  progressive_sequence; // 1 bits
  unsigned char  chroma_format; // 2 bits
  //Memo// chroma format
  ///***************************
  // 00  Reserved
  // 01  4:2:0
  // 10  4:2:2
  // 11  4:4:4
  //***************************/

  unsigned char  horizontal_size_extension; // 2 bits
  unsigned char  vertical_size_extension; // 2 bits
  unsigned short bit_rate_extension; // 12 bits
  unsigned char  vbv_buffer_size_extension; // 8 bits
  unsigned char  low_delay; // 1 bits
  unsigned char  frame_rate_extension_n; // 2 bits
  unsigned char  frame_rate_extension_d; // 5 bits

  //~~~// sequence_display_extension()
  unsigned char  video_format; // 3 bits
  //Memo// chroma format
  ///***************************
  // 000  Component
  // 001  PAL
  // 010  NTSC
  // 011  SECAM
  // 100  MAC
  // 101  Unspecified video format
  // 110  Reserved
  // 111  Reserved
  //***************************/

  unsigned char  colour_description; // 1 bits
  unsigned char  colour_primaries; // 8 bits
  unsigned char  transfer_characteristics; // 8 bits
  unsigned char  matrix_coefficients; // 8 bits
  unsigned short display_horizontal_size; // 14 bits
  unsigned short display_vertical_size; // 14 bits

  //~~~// sequence_scalable_extension()
  unsigned char  scalable_mode; // 2 bits
  unsigned char  layer_id; // 4 bits
  unsigned short lower_layer_prediction_horizontal_size; // 14 bits
  unsigned short lower_layer_prediction_vertical_size; // 14 bits
  unsigned char  horizontal_subsampling_factor_m; // 5 bits
  unsigned char  horizontal_subsampling_factor_n; // 5 bits
  unsigned char  vertical_subsampling_factor_m; // 5 bits
  unsigned char  vertical_subsampling_factor_n; // 5 bits
  unsigned char  picture_mux_enable; // 1 bits
  unsigned char  mux_to_progressive_sequence; // 1 bits
  unsigned char  picture_mux_order; // 3 bits
  unsigned char  picture_mux_factor; // 3 bits

  //-----------------------------------------
  // Group Picture
  //-----------------------------------------
  //~~~// group_of_pictures_header()
  //unsigned int   S_group_start_code; // 32 bits //*Global*//
                 //unsigned int   time_code; // 25 bits //*Must*//
                 // time_code struct
  unsigned char  drop_frame_flag; // 1 bits // (value range)
  unsigned char  time_code_hours; // 5 bits // 0 - 23
  unsigned char  time_code_minutes; // 6 bits // 0 - 59
                 // marker_bit // 1 bits
  unsigned char  time_code_seconds; // 6 bits // 0 - 59
  unsigned char  time_code_pictures; // 6 bits // 0 - 59

  unsigned char  closed_gop; // 1 bits
  unsigned char  broken_link; // 1 bits

  //-----------------------------------------
  // Picture Header
  //-----------------------------------------
  //~~~// picture_header()
  //unsigned int   S_picture_start_code; // 32 bits //*Global*//
  unsigned short temporal_reference; // 10 bits
  unsigned char  picture_coding_type; // 3 bits
  //Memo// picture coding type
  ///***************************
  // 000  Forbidden
  // 001  intra-coded(I)
  // 010  predictive-coded(P)
  // 011  bidirectionally-predictive-coded(B)
  // 100  shall not be used (dc intra-coded(D))
  // 101  Reserved
  // 110  Reserved
  // 111  Reserved
  //***************************/

  unsigned short vbv_delay; // 16 bits
  unsigned char  full_pel_forward_vector; // 1 bits
  unsigned char  forward_f_code; // 3 bits
  unsigned char  full_pel_backward_vector; // 1 bits
  unsigned char  backward_f_code; // 3 bits
  unsigned char  extra_bit_picture; // 1 bits
  unsigned char  extra_information_picture; // 8 bits

  //~~~// picture_coding_extension()
  unsigned short f_code; // 16 bits //*Must*//
  //Memo// f_code
  ///***************************
  // f_code[0][0] // forward horizontal // 4 bits
  // f_code[0][1] // forward vertical // 4 bits
  // f_code[1][0] // backward horizontal // 4 bits
  // f_code[1][1] // backward vertical // 4 bits
  //***************************/

  unsigned char  intra_dc_precision; // 2 bits
  unsigned char  picture_structure; // 2 bits //*Must*//
  //Memo// picture_structure
  ///***************************
  // 01 // Top Field  // 10 // Botton Field  // 11 // Frame picture
  //***************************/

  unsigned char  top_field_first; // 1 bits //*Must*//
  unsigned char  frame_pred_frame_dct; // 1 bits
  unsigned char  concealment_motion_vectors; // 1 bits
  unsigned char  q_scale_type; // 1 bits
  unsigned char  intra_vlc_format; // 1 bits
  unsigned char  alternate_scan; // 1 bits
  unsigned char  repeat_first_field; // 1 bits
  unsigned char  chroma_420_type; // 1 bits
  unsigned char  progressive_frame; // 1 bits
  unsigned char  composite_display_flag; // 1 bits
  unsigned char  v_axis; // 1 bits
  unsigned char  field_sequence; // 3 bits
  unsigned char  sub_carrier; // 1 bits
  unsigned char  burst_amplitude; // 7 bits
  unsigned char  sub_carrier_phase; // 8 bits

  //~~~// quant_matrix_extension()
  unsigned char  load_chroma_intra_quant_mat; // 1 bits
  unsigned char  chroma_intra_quant_mat[64]; // 8*64 bits
  unsigned char  load_chroma_non_intra_quant_mat; // 1 bits
  unsigned char  chroma_non_intra_quant_mat[64]; // 8*64 bits

  //~~~// picture_display_extension()
  unsigned short frame_centre_horizontal_offset; // 16 bits
  unsigned short frame_centre_vertical_offset; // 16 bits

  //~~~// picture_temporal_scalable_extension()
  unsigned char  reference_select_code; // 2 bits
  unsigned short forward_temporal_reference; // 10 bits
  unsigned short backward_temporal_reference; // 10 bits

  //~~~// picture_spatial_scalable_extension()
  unsigned short lower_layer_temporal_reference; // 10 bits
  unsigned short lower_layer_horizontal_offset; // 15 bits
  unsigned short lower_layer_vertical_offset; // 15 bits
  unsigned char  spatial_temporal_weight_code_table_index; // 2 bits
  unsigned char  lower_layer_progressive_frame; // 1 bits
  unsigned char  lower_layer_deinterlaced_field_select; // 1 bits

  //~~~// copyright_extension()
  unsigned char  copyright_flag; // 1 bits
  unsigned char  copyright_identifier; // 8 bits
  unsigned char  original_or_copy; // 1 bits
  unsigned char  reserved; // 7 bits
  unsigned short copyright_number_1; // 20 bits
  unsigned short copyright_number_2; // 22 bits
  unsigned short copyright_number_3; // 22 bits
} mpeg2_hdr_t;

typedef struct mpeg2_dec_s {
  mpeg2_hdr_t    mpeg2_header;
  mpeg2_version  version;
} mpeg2_hdr_info_t;

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
} mpeg2_input_t;

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
  MP2_STATUS      Decoded_status; // Frame decoded status
  unsigned int    consumedbytes;  // consumed byte in VLD
} mpeg2_frameinfo_t;

typedef struct {
  VD_DECODE_ATTR_M;
  mpeg2_hdr_info_t hdr_info;
} mpeg2_attr_t;

typedef struct {
  VD_CAPABILITY_M;
  unsigned int chip_id;
} mpeg2_capability_t;

/*----------------- EXPORTED PRIVATE VARIABLES -------------------------------*/
/* allocate memory for variables only in XXX.c */
#ifdef COM_MPEG2_C
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
