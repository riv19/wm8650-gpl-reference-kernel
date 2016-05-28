/* PVCS version log
** $Log:  $
 * 
 */
#ifndef COM_VD_H
/* To assert that only one occurrence is included */
#define COM_VD_H

/*--- com-video.h---------------------------------------------------------------
*   Copyright (C) 2008 WonderMedia Tech. Inc.
*
* MODULE       : com-vd.h --- common video decode 
* AUTHOR       : Willy Chuang
* DATE         : 2008/11/20
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Willy Chuang, 2008/11/20
*   First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <asm/page.h>

#if 0 /* for MVL5.0 2.6.18+ kernel */
  #include <asm/arch/irqs.h>
  #include <asm/hardware.h>
  #include <asm/arch/com-video.h>
#else /* for Android 2.6.29 kernel */
  #include <mach/irqs.h>
  #include <mach/hardware.h>
  #include <mach/com-video.h>
#endif

#else
#include "com-video.h"
#endif /* #ifdef __KERNEL__ */

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

/*------------------------------------------------------------------------------
    Following macros define hardware decoder type for vd_ioctl_cmd 
------------------------------------------------------------------------------*/
#define VD_JPEG    0x0001
#define VD_MPEG    0x0002 
#define VD_MPEG4   0x0003
#define VD_DIVX    0x0004
#define VD_H264    0x0005
#define VD_RV      0x0006
#define VD_VC1     0x0007

#define VE_JPEG    0x0011


#define VD_MAX     0x0019

/* The format of version is defined below. If version is 1.15.3,
   Please use marco
     MAKE_IDENTITY(VD_JPEG, 11503)
   LOOK OUT: we only use 16 bits for version, so the max value is 
     65536 = 6.55.36
*/
#define MAKE_IDENTITY(vd_type, version)  ( ((vd_type) << 16) | (version & 0xFFFF))

/* Example: 
    jdec_input_t  arg_in;

    IOCTL_CMD_INIT(&arg_in, jdec_input_t, VD_JPEG, 10102)
*/
#define IOCTL_CMD_INIT(cmd, _struct_type_, vd_type, version)  \
             ((vd_ioctl_cmd *)cmd)->size     = sizeof(_struct_type_); \
             ((vd_ioctl_cmd *)cmd)->identity = MAKE_IDENTITY(vd_type, version)


#define CHECK_IDENTITY(identity, expected_type)  \
                     (((identity & 0xFFFF0000) == (expected_type << 16))?0:0x10)

#define CHECK_SIZE(size, _struct_type_)    \
                          (((size) == sizeof(_struct_type_))?0:0x01)

#define IOCTL_CMD_CHECK(ptr, _struct_type_, vd_type) \
             CHECK_IDENTITY( ((vd_ioctl_cmd *)ptr)->identity , vd_type ) | \
             CHECK_SIZE( ((vd_ioctl_cmd *)ptr)->size, _struct_type_ )

/*------------------------------------------------------------------------------
    Definitions of return value
------------------------------------------------------------------------------*/
#define RET_OK                (0)    
#define RET_VERSION_INVALID   (-1)  /* Invalid version */
#define RET_TIME_OUT          (-2)  /* HW time out */


/*------------------------------------------------------------------------------
    Following macros are defined and used for "flags" member in vd_decode_input_t 
------------------------------------------------------------------------------*/
/* BIT 0 ~ 15 reserved for each private decoder */
#define VD_INPUT_ARG_FLAGS_HURRY_UP   BIT(0)

/* Bit 16 ~ 29 reserved for common decoders in future */
#define FLAG_MULTI_SEGMENT_EN  BIT(29)  /* 0: Multi-segment decoding is disable
                                           1: Multi-segment decoding is enable */
#define FLAG_BUFPHYCONTI       BIT(30)  /* 0: IN buffer is not phy. continously
                                           1: IN buffer is phy. continously */
#define FLAG_NOBUFALLOCATED    BIT(31)  /* 0: Ouput buffer was allocated by user
                                           1: Ouput buffer was not allocated */

/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
/* The member of identity is defined as 
     Bit 31~16: used for hardware decoder type 
     Bit 15~0:  used for version 
*/
#define VD_IOCTL_CMD_M \
    unsigned int    identity; /* decoder type */\
    unsigned int    size      /* size of full structure */
/* End of VD_IOCTL_CMD_M */


#define VD_CAPABILITY_M \
    VD_IOCTL_CMD_M
/* End of VD_CAPABILITY_M */


#define VD_DECODE_ATTR_M \
    VD_IOCTL_CMD_M
/* End of VD_DECODE_ATTR_M */


#define VD_INPUT_ARG_M  \
    VD_IOCTL_CMD_M; \
    unsigned int    src_addr;  /* input buffer addr in user space */\
    unsigned int    src_size;  /* input buffer size */\
    unsigned int    flags;     /* control flag for HW decoder */\
    /* Output Frame buffer information */\
    unsigned int    dst_y_addr;  /* output address in Y (Physical addr )*/\
    unsigned int    dst_y_size;  /* output buffer size in Y */\
    unsigned int    dst_c_addr;  /* output address in C (Physical addr )*/\
    unsigned int    dst_c_size;  /* output buffer size in C */\
    unsigned int    linesize     /* Length of one scanline (unit: pixel) for YC and RGB planes */
/* End of VD_INPUT_ARG_M */


#define VD_FRAMEINFO_M  \
    VD_IOCTL_CMD_M; \
    vdo_framebuf_t  fb
/* End of VD_FRAMEINFO_M */

#define VD_STATUS_M \
    VD_IOCTL_CMD_M
/* End of VD_STATUS_M */

/*------------------------------------------------------------------------------
    Definitions of enum
------------------------------------------------------------------------------*/

  
/*------------------------------------------------------------------------------
    Definitions of Struct
------------------------------------------------------------------------------*/
#ifndef __KERNEL__
struct prdt_struct {
	unsigned int addr;
	unsigned short size;
	unsigned short reserve : 15;
	unsigned short EDT : 1;
}__attribute__((packed));
#endif

/* Following structure is used for all HW decoder as input arguments */
typedef struct {
    VD_IOCTL_CMD_M;
} vd_ioctl_cmd;

/* Following structure is used for all HW decoder to return HW capability */
typedef struct {
    VD_CAPABILITY_M;
} vd_capability_t;

/* Following structure is used for all HW decoder to set decoding information */
typedef struct {
    VD_DECODE_ATTR_M;
} vd_decode_attr_t;

/* Following structure is used for all HW decoder as input arguments */
typedef struct {
    VD_INPUT_ARG_M;
} vd_decode_input_t;


/* Following structure is used for all HW decoder as output arguments */
typedef struct {
    VD_FRAMEINFO_M;
} vd_frameinfo_t;

/* Following structure is used for all HW decoder to return driver status */
typedef struct {
    VD_STATUS_M;
} vd_status_t;

/* Following structure is used for all HW decoder as output arguments */
typedef struct {
    int   vd_fd;
    int   mb_fd;
    int   vd_id;
    void *priv;   /* private data for video decoder used */
} vd_handle_t;

/*------------------------------------------------------------------------------
   Macros below are used for driver in IOCTL
------------------------------------------------------------------------------*/
#define VD_IOC_MAGIC              'k'
#define VD_IOC_MAXNR              6

/* GET_CAPABILITY: application get HW capability through the IOCTL 
                   driver must cast vd_ioctl_cmd to vd_capability_t
*/
#define VDIOGET_CAPABILITY        _IOWR(VD_IOC_MAGIC, 0, vd_ioctl_cmd)

/* SET_DECODE_INFO: application send decoding info to driver through the IOCTL 
                   driver must cast vd_ioctl_cmd to vd_decode_attr_t
*/
#define VDIOSET_DECODE_INFO       _IOR(VD_IOC_MAGIC, 1, vd_ioctl_cmd)

/* SET_DECODE_PROC: application send decode data to driver through the IOCTL
                   driver must cast vd_ioctl_cmd to vd_decode_input_t
*/                   
#define VDIOSET_DECODE_PROC       _IOR(VD_IOC_MAGIC, 2, vd_ioctl_cmd)

/* GET_DECODE_FINISH: application waits decoding end through the IOCTL
                   driver must cast vd_ioctl_cmd to vd_frameinfo_t
*/
#define VDIOGET_DECODE_FINISH     _IOWR(VD_IOC_MAGIC, 3, vd_ioctl_cmd)

/* GET_DECODE_FLUSH: application flush decode status through the IOCTL
*/
#define VDIOGET_DECODE_FLUSH      _IOWR(VD_IOC_MAGIC, 4, vd_ioctl_cmd)

/* VDIOGET_DECODE_STATUS: application get status through the IOCTL
*/
#define VDIOGET_DECODE_STATUS     _IOWR(VD_IOC_MAGIC, 5, vd_ioctl_cmd)


#endif /* ifndef COM_VD_H */

/*=== END com-video.h ==========================================================*/
