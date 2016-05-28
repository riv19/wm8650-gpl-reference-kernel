/*++ 
 * drivers\media\video\wmt\jpeg\com-jdec.h
 * Common interface for WonderMedia SoC hardware JPEG decoder driver
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
#ifndef COM_JDEC_H
/* To assert that only one occurrence is included */
#define COM_JDEC_H

/*------------------------------------------------------------------------------
 * ChangeLog
 *
 * 2010-08-10  Willy Chunag  <willychuang@wondermedia.com.tw>
 *      - Add License declaration and ChangeLog
 *
------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Willy Chuang, 2008/11/20
*	First version
*  
*  0.04.00 (2010-04-20): Add FLAG_SPLIT_WRITE_EN
*
*  0.03.00 (2009-12-03): Add DECFLAG_MJPEG_BIT
*
*  0.02.00 (2009-04-14): Add new member "avi1" in jdec_hdr_info_t to support 
*                        "AVI1" FOURCE in APP marker
*
*  0.01.04 (2009-02-03): Add new member "linesize" in vd_decode_input_t
*
*  0.01.03 (2009-01-20): Add two new members "hdr" and "di" in jdec_attr_t
*
*  0.01.02 (2009-01-16): Add a new member "scaled" in jdec_frameinfo_t
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#ifdef __KERNEL__
#include "../com-vd.h"
#else
#include "com-vd.h"
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  VIAAPI_XXXX  1    *//*Example*/

#define JDEC_VERSION     200   /* version: 0.02.00 */


#define IOCTL_JPEG_INIT(cmd, _struct_type_) \
        memset((cmd), 0, sizeof(_struct_type_)); \
        IOCTL_CMD_INIT((vd_ioctl_cmd *)cmd, _struct_type_, VD_JPEG, JDEC_VERSION)

/* YUV To RGB Mode */

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/


/*------------------------------------------------------------------------------
    Definitions of enum
------------------------------------------------------------------------------*/
typedef enum { 
    SCALE_ORIGINAL,       /* Original size */
    SCALE_HALF,           /* 1/2 of Original size */
    SCALE_QUARTER,        /* 1/4 of Original size */
    SCALE_EIGHTH,         /* 1/8 of Original size */
    SCALE_BEST_FIT,       /* Best fit to output resolution
                             Original size on center screen if size <= resolution
                             scale down to full screen if size > resolution */
    SCALE_THUMBS,         /* Scalue down to 96x96 */
    SCALE_MAX
} vd_scale_ratio;

typedef enum { 
    JT_FRAME,
    JT_FIELD_TOP,
    JT_FIELD_BOTTOM
} jpeg_type;

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/
/* Used for AVI1 FOURCC in APP0 only */
typedef struct {
    jpeg_type  type;
    int        field_size;  /* 0 if JT_FRAME; others: data size of a field */ 
} avi1_t; 

/*------------------------------------------------------------------------------
    Following macros are defined and used for "flags" member in vd_decode_input_t 
------------------------------------------------------------------------------*/
/* BIT 0 ~ 15 reserved for each private decoder */
#define FLAG_FIRST_SEGMENT      BIT(11)  /* 0: Not first segmetn in Multi-segment decoding
                                            1: First segment */
#define FLAG_LAST_SEGMENT       BIT(12)  /* 0: Not last segmetn in Multi-segment decoding
                                            1: Last segment */
#define FLAG_SPLIT_WRITE_EN     BIT(13)

/*------------------------------------------------------------------------------
   Following macro definition is used for profile in jdec_hdr_info_t
------------------------------------------------------------------------------*/
#define DECFLAG_MJPEG_BIT    BIT(31)  /* 0: JPEG file   1: MJPG file*/

/*------------------------------------------------------------------------------
    Header Information
------------------------------------------------------------------------------*/
#define JDEC_HEADER_INFO_M \
    unsigned int    profile;       /* Profile */\
    unsigned int    sof_w;         /* Picture width in SOF */  \
    unsigned int    sof_h;         /* Picture height in SOF */ \
    unsigned int    filesize;      /* File size of this JPEG */\
    vdo_color_fmt   src_color;     /* Picture color format */  \
    avi1_t          avi1
/* End of JDEC_HEADER_INFO_M */

typedef struct {
    JDEC_HEADER_INFO_M;
} jdec_hdr_info_t;

/*------------------------------------------------------------------------------
    Decoder Information
------------------------------------------------------------------------------*/
typedef struct {
    unsigned int enable;   /* Enable HW Partial decode or not */
    unsigned int x;        /* X of start decode (16xN alignment) (Unit: Pixel) */
    unsigned int y;        /* Y of start decode (16xN alignment) (Unit: Pixel) */
    unsigned int w;        /* Width of start decode (16xN alignment) (Unit: Pixel) */
    unsigned int h;        /* Height of start decode (16xN alignment) (Unit: Pixel) */
} jdec_pd; /* PD: Partial Decode */

typedef struct {
    unsigned int enable;   /* Enable HW Banding decode or not */
    unsigned int height;   /* The height of each band (16xN alignment) (Unit: Pixel) */
} jdec_bd; /* bd: Banding Decode */

#define JDEC_DECODE_INFO_M \
    jdec_pd         pd;             /* Settings for HW Partial decode */\
    unsigned int    scanline;       /* Scanline offset of FB (Unit: pixel) */\
    vd_scale_ratio  scale_factor;   /* Scale ratio */\
    vdo_color_fmt   decoded_color;  /* Decoded color space */\
    jdec_bd         bd              /* Settings for HW Banding decode */
/* End of JDEC_DECODE_INFO_M */

typedef struct {
    JDEC_DECODE_INFO_M;
} jdec_decode_info_t;

/*------------------------------------------------------------------------------
    VDIOGET_CAPABILITY
------------------------------------------------------------------------------*/
typedef struct {
    VD_CAPABILITY_M;
    
    unsigned int chip_id;
    
    /* Support list ----> */
    /* 0: Not support;  1: Did support */
        unsigned int baseline:1;
        unsigned int progressive:1;
        unsigned int graylevel:1;      

        unsigned int partial_decode:1;
        unsigned int decoded_to_YC420:1;
        unsigned int decoded_to_YC422H:1;
        unsigned int decoded_to_YC444:1;
        unsigned int decoded_to_ARGB:1;
        unsigned int split_write:1;
        unsigned int reseverd:23;
    /* <---- */        
    unsigned int scale_ratio_num;
    unsigned int scale_fator[8];
} jdec_capability_t;

/*------------------------------------------------------------------------------
    VDIOSET_DECODE_INFO
------------------------------------------------------------------------------*/
typedef struct {
    VD_DECODE_ATTR_M;

    /* Information in source picture */
    jdec_hdr_info_t     hdr;
    
    /* Information for HW decoder */
    jdec_decode_info_t  di;
} jdec_attr_t;

/*------------------------------------------------------------------------------
   Following structure is used for JPEG HW decoder as input arguments 
------------------------------------------------------------------------------*/
typedef struct {
    VD_INPUT_ARG_M;
    unsigned int    dst_y_addr_user;  /* Y address in user space */
    unsigned int    dst_c_addr_user;  /* C address in user space */
    unsigned int    dec_to_x;         /* Decode to x (Unit: pixel) */
    unsigned int    dec_to_y;         /* Decode to y (Unit: pixel)*/

    unsigned int    band_no;          /* Band number for band decode only */

    /* Returned information */
    unsigned int    _sts;             /* 0x00000001: JPEG decoded frame finish 
                                         0x00000002: BSDMA finish  
                                         0x00000004: Split write finish 
                                      */
} jdec_input_t;

/* The macros below are used for member "_sts" in jdec_input_t only */
#define JDEC_STS_DECODE_DONE         0x00000001  /* JPEG decoded frame finish  */
#define JDEC_STS_BSDMA_FINISH        0x00000002  /* BSDMA finish  */
#define JDEC_STS_BD_FINISH           0x00000004  /* Split write finish */

/*------------------------------------------------------------------------------
    Following structure is used for all HW decoder as output arguments
------------------------------------------------------------------------------*/
typedef struct {
	VD_FRAMEINFO_M;
    unsigned int     y_addr_user;  /* Y address in user space */
    unsigned int     c_addr_user;  /* C address in user space */
    unsigned int     scaled;       /* This value should be equal to one of below 
                                        SCALE_ORIGINAL = 0
                                        SCALE_HALF     = 1
                                        SCALE_QUARTER  = 2
                                        SCALE_EIGHTH   = 3
                                   */
} jdec_frameinfo_t;

/*------------------------------------------------------------------------------
    Following structure is used for all HW decoder to return driver status
------------------------------------------------------------------------------*/

#define JDEC_STS_FLAG_FRAME_DONE      0x00000001    /* To get decoding finished status */
#define JDEC_STS_FLAG_MSD_DONE        0x00000002    /* To get MSD finished status */

typedef struct {
    VD_STATUS_M;
    /* iformation set by application */
    unsigned int flag_en;           /* Operation of macro defined above */
    unsigned int is_blocked;        /* blocked in driver or not */
    
    /* iformation returned by driver */
    unsigned int sts_flag;          /* Operation of macro defined above */
    unsigned int msd_seg_no;       /* Number of finisehd Segments */
    unsigned int bd_band_no;       /* Number of finisehd band in BD */
    unsigned int err_code;

} jdec_status_t;

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef JPEG_COMMON_C /* allocate memory for variables only in vt8430_viaapi.c */
    #define EXTERN
#else
    #define EXTERN   extern
#endif /* ifdef JPEG_COMMON_C */

/* EXTERN int      viaapi_xxxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

/*------------------------------------------------------------------------------
    Macros below are used in user space only
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    API Functions used in user space only
------------------------------------------------------------------------------*/

    
#endif /* ifndef COM_JDEC_H */

/*=== END com-jdec.h ==========================================================*/
