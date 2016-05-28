 
#ifndef COM_JENC_H
#define COM_JENC_H


#ifdef __KERNEL__
#include "../../com-vd.h"
#else
#include "com-vd.h"
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  VIAAPI_XXXX  1    *//*Example*/

#define jenc_VERSION     200   /* version: 0.02.00 */


#define IOCTL_JPEG_INIT(cmd, _struct_type_) \
        memset((cmd), 0, sizeof(_struct_type_)); \
        IOCTL_CMD_INIT((vd_ioctl_cmd *)cmd, _struct_type_, VD_JPEG, jenc_VERSION)

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
#define FLAG_SPLIT_WRITE_EN   BIT(13)

/*------------------------------------------------------------------------------
   Following macro definition is used for profile in jenc_hdr_info_t
------------------------------------------------------------------------------*/
#define DECFLAG_MJPEG_BIT    BIT(31)  /* 0: JPEG file   1: MJPG file*/

/*------------------------------------------------------------------------------
    Header Information
------------------------------------------------------------------------------*/
#define jenc_HEADER_INFO_M \
    unsigned int    profile;       /* Profile */\
    unsigned int    sof_w;         /* Picture width in SOF */  \
    unsigned int    sof_h;         /* Picture height in SOF */ \
    unsigned int    filesize;      /* File size of this JPEG */\
    vdo_color_fmt   src_color;     /* Picture color format */  \
    avi1_t          avi1
/* End of jenc_HEADER_INFO_M */

typedef struct {
    jenc_HEADER_INFO_M;
} jenc_hdr_info_t;

/*------------------------------------------------------------------------------
    Decoder Information
------------------------------------------------------------------------------*/
typedef struct {
    unsigned int enable;   /* Enable HW Partial decode or not */
    unsigned int x;        /* X of start decode (16xN alignment) (Unit: Pixel) */
    unsigned int y;        /* Y of start decode (16xN alignment) (Unit: Pixel) */
    unsigned int w;        /* Width of start decode (16xN alignment) (Unit: Pixel) */
    unsigned int h;        /* Height of start decode (16xN alignment) (Unit: Pixel) */
} jenc_pd; /* PD: Partial Decode */

typedef struct {
    unsigned int enable;   /* Enable HW Banding decode or not */
    unsigned int height;   /* The height of each band (16xN alignment) (Unit: Pixel) */
} jenc_bd; /* bd: Banding Decode */

#define jenc_DECODE_INFO_M \
    jenc_pd         pd;             /* Settings for HW Partial decode */\
    unsigned int    scanline;       /* Scanline offset of FB (Unit: pixel) */\
    vd_scale_ratio  scale_factor;   /* Scale ratio */\
    vdo_color_fmt   decoded_color;  /* Decoded color space */\
    jenc_bd         bd              /* Settings for HW Banding decode */
/* End of jenc_DECODE_INFO_M */

typedef struct {
    jenc_DECODE_INFO_M;
} jenc_decode_info_t;

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
} jenc_capability_t;

/*------------------------------------------------------------------------------
    VDIOSET_DECODE_INFO
------------------------------------------------------------------------------*/
typedef struct {
    VD_DECODE_ATTR_M;
} jenc_attr_t;

/*------------------------------------------------------------------------------
   Following structure is used for JPEG HW decoder as input arguments 
------------------------------------------------------------------------------*/
typedef struct {
    VD_INPUT_ARG_M;
    unsigned int src_Y_addr;    //YC source address
    unsigned int src_C_addr;
    unsigned int dst_addr;      //mjpeg encoding dest address
    unsigned int pic_width;     //resolution
    unsigned int pic_height;
    unsigned int buf_width;
    unsigned int buf_height;
    unsigned int quality;       //0(worst)-100(best)
    unsigned int pattern;       //0:jpeg 1:video
    unsigned int fill_header;   //0:ARM fill header/tail  1:dsp fill header/tail    
} jenc_input_t;

/* The macros below are used for member "_sts" in jenc_input_t only */
#define jenc_STS_DECODE_DONE         0x00000001  /* JPEG decoded frame finish  */
#define jenc_STS_BSDMA_FINISH        0x00000002  /* BSDMA finish  */
#define jenc_STS_BD_FINISH           0x00000004  /* Split write finish */

/*------------------------------------------------------------------------------
    Following structure is used for all HW decoder as output arguments
------------------------------------------------------------------------------*/
typedef struct {
	VD_FRAMEINFO_M;
    unsigned int mjpeg_size;    /*mjpeg encoding size*/
} jenc_frameinfo_t;

/*------------------------------------------------------------------------------
    Following structure is used for all HW decoder to return driver status
------------------------------------------------------------------------------*/

#define jenc_STS_FLAG_FRAME_DONE      0x00000001    /* To get decoding finished status */
#define jenc_STS_FLAG_MSD_DONE        0x00000002    /* To get MSD finished status */

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

} jenc_status_t;

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

    
#endif /* ifndef COM_jenc_H */

/*=== END com-jenc.h ==========================================================*/
