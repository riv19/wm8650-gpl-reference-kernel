/*++ 
 * linux/drivers/media/video/wmt/h264/com-h264.h
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
#ifndef COM_H264_H
/* To assert that only one occurrence is included */
#define COM_H264_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#ifdef __KERNEL__
#include "../com-vd.h"
#else
#include "com-vd.h"
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  VIAAPI_XXXX  1    *//*Example*/

#define H264_VERSION     100   /* version: 0.01.00 */
#define H264_MAX_FB      32    /* the maximum number of framebuffers */

#define IOCTL_H264_INIT(cmd, _struct_type_) \
        memset((cmd), 0, sizeof(_struct_type_)); \
        IOCTL_CMD_INIT((vd_ioctl_cmd *)cmd, _struct_type_, VD_H264, H264_VERSION)

/* YUV To RGB Mode */

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/


/*------------------------------------------------------------------------------
    Definitions of enum
------------------------------------------------------------------------------*/
typedef enum {
    H264_STA_NONE,
    H264_STA_DEC_RUN,
    H264_STA_DEC_DONE,
    H264_STA_DEC_SKIP,
    H264_STA_DEC_ERR,
    H264_STA_DEC_TIMEOUT,
    H264_STA_DECSTATUS_MAX,
} H264_STATUS;

typedef enum {
  AVC_STA_DECSTATUS_NONE  = 0x00000000,
  AVC_STA_SIZE_OVERFLOW   = 0x10000000,
  AVC_STA_FMT_NOT_SUPPORT = 0x20000000,
  AVC_STA_READY           = 0x01000000,
  AVC_STA_CLOSE           = 0x02000000,
  /* DEC Status */
  AVC_STA_DEC_RUN         = 0x00000010,
  AVC_STA_DEC_DONE        = 0x00000020,
  AVC_STA_DEC_SKIP        = 0x00000030,
  AVC_STA_DEC_PARSING     = 0x00000040,
  /* Error Status */
  AVC_STA_ERR_DECODING    = 0x00000001,
  AVC_STA_ERR_TIMEOUT     = 0x00000002,
  AVC_STA_ERR_UNKNOWN     = 0x00000003,
  /* Parsing Status */
  AVC_STA_SEQ_END         = 0x00001000,
  AVC_STA_ISO_END         = 0x00002000,
  AVC_STA_DECSTATUS_MAX   = 0xFFFFFFFF
} AVC_STATUS;

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
} avc_input_t;

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
    Header Information
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
    Frame Information
------------------------------------------------------------------------------*/
typedef struct {
	VD_FRAMEINFO_M;
    unsigned int     y_addr_user;  /* Y address in user space */
    unsigned int     c_addr_user;  /* C address in user space */
    unsigned int     consumedbytes;  /* consumed byte in VLD */
	H264_STATUS      status;
} h264_frameinfo_t;

/*------------------------------------------------------------------------------
    Decoder Information
------------------------------------------------------------------------------*/
typedef struct {
    unsigned int     y_addr_phys;  /* Y address in phys space */
    unsigned int     c_addr_phys;  /* C address in physical space */
    unsigned int     y_addr_user;  /* Y address in user space */
    unsigned int     c_addr_user;  /* C address in user space */
    unsigned int     linesize;     /* Length of one scanline (unit: pixel) for YC and RGB planes */
} h264_framebuf_t;

#define H264_DECODE_INFO_M \
    unsigned int      fbnum;        /* the number of FB */\
    unsigned int      fblvl;        /* base mb count of FB */\
    unsigned int      total_frame_count;        /* base mb count of FB */\
    h264_framebuf_t   fb[H264_MAX_FB];
/* End of H264_DECODE_INFO_M */

typedef struct {
    H264_DECODE_INFO_M;
} h264_decode_info_t;

/*------------------------------------------------------------------------------
    VDIOGET_CAPABILITY
------------------------------------------------------------------------------*/
typedef struct {
    VD_CAPABILITY_M;
    /* Input data for check capability */
	unsigned char *buf;
	unsigned int buf_size;

    /* Output capability */    
    unsigned int chip_id;

	unsigned int width;
	unsigned int height;
	unsigned int mini_frame_count;

} h264_capability_t;

/*------------------------------------------------------------------------------
    VDIOSET_DECODE_INFO
------------------------------------------------------------------------------*/
typedef struct {
    VD_DECODE_ATTR_M;

    /* Information for HW decoder */
    h264_decode_info_t  di;
} h264_attr_t;

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef H264_COMMON_C
    #define EXTERN
#else
    #define EXTERN   extern
#endif /* ifdef H264_COMMON_C */

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*------------------------------------------------------------------------------
    Macros below are used in user space only
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    API Functions used in user space only
------------------------------------------------------------------------------*/

    
#endif /* ifndef COM_H264_H */

/*=== END com-jdec.h ==========================================================*/
