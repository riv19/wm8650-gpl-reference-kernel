/*++ 
 * linux/drivers/media/video/wmt/h264/hw-h264.h
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
#ifndef HW_H264_H
/* To assert that only one occurrence is included */
#define HW_H264_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/

#include "com-h264.h"

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

#ifdef __KERNEL__
    #define DPRINT    printk
    #define DBG_INFO(fmt,args...)         DPRINT(KERN_INFO    "[core_%s]:" fmt, __FUNCTION__ , ## args)
    #define DBG_WARN(fmt, args...)        DPRINT(KERN_WARNING "[core_%s]:" fmt, __FUNCTION__ , ## args)
    #define DBG_ERR(fmt, args...)         DPRINT(KERN_ERR     "[core_%s]:" fmt, __FUNCTION__ , ## args)
#else
    #define DPRINT    printf
    #define DBG_INFO(fmt,args...)         DPRINT("[ap_%s]:" fmt, __FUNCTION__ , ## args)
    #define DBG_WARN(fmt, args...)        DPRINT("[ap_%s]:" fmt, __FUNCTION__ , ## args)
    #define DBG_ERR(fmt, args...)         DPRINT("[ap_%s]:" fmt, __FUNCTION__ , ## args)
#endif

/* The followign two macros should be enable one and only one */
//#define H264_TEST_FAKE_BITSTREAM

//#define H264_DEBUG    /* Flag to enable debug message */
//#define H264_DEBUG_DETAIL
//#define H264_TRACE

#ifdef H264_DEBUG
    #define DBG(fmt, args...)             DPRINT("[%s]:" fmt, __FUNCTION__ , ## args)
#else
    #define DBG(fmt, args...)
#endif

#ifdef H264_DEBUG_DETAIL
    #define DBG_DETAIL(fmt, args...)      DPRINT("[%s]:" fmt, __FUNCTION__ , ## args)
#else
    #define DBG_DETAIL(fmt, args...)
#endif

#ifdef H264_TRACE
    #define TRACE(fmt, args...)           DPRINT("[%s]:" fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...) 
#endif

#define DBG_SIZE_CHECK(addr, _type_)    \
                          if( ((_type_ *)addr)->size != sizeof(_type_) ){ \
                              DBG_ERR("Size conflict (size: %d != %d)\n", \
                                      ((_type_ *)addr)->size, sizeof(_type_)); \
                              return -1;\
                          }

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/

typedef enum {
    STA_READY          = 0x00010000,
    STA_CLOSE          = 0x00020000,
    STA_ATTR_SET       = 0x00040000,
    STA_DMA_START      = 0x00080000,
    STA_DECODE_DONE    = 0x00000001,
    STA_DECODEING      = 0x00000002,
    STA_DMA_MOVE_DONE  = 0x00000008,
    /* Error Status */
    STA_ERR_FLAG       = 0x80000000,
    STA_ERR_DMA_TX     = 0x80100000,
    STA_ERR_DECODING   = 0x80200000,
    STA_ERR_BAD_PRD    = 0x80400000,
    STA_ERR_UNKNOWN    = 0x80800000
} h264_status;

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/
typedef struct {
    unsigned int      size;
    h264_attr_t       attr;     /* Come from user space for HW decoder */
    vd_decode_input_t arg_in;   /* Come from user space */
    h264_frameinfo_t  arg_out;  /* Return to user space later */

    unsigned int      decoded_w;      /* Decoded Picture width (pixel) */ 
    unsigned int      decoded_h;      /* Decoded Picture height (pixel) */
    unsigned int      src_mcu_width;  /* Width in unit MCU */
    unsigned int      src_mcu_height; /* Height in unit MCU */
    unsigned int      scale_ratio;

    unsigned int      prd_virt;
    unsigned int      prd_addr;
    unsigned int      line_width_y;  /* Y or RGB */
    unsigned int      line_width_c;
    unsigned int      line_height; /* Y/C or RGB */

    /* Following members are used for hw-h264.c only */
    h264_capability_t  capab_tbl;
    
    h264_status       _status;
#ifdef __KERNEL__
    spinlock_t        _lock;
#endif
} h264_drvinfo_t;



/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef HW_H264_C /* allocate memory for variables only in wm8510-h264.c */
    #define EXTERN
#else
    #define EXTERN   extern
#endif /* ifdef WM8510_H264_C */

/* EXTERN int      viaapi_xxxx; *//*Example*/
#ifdef H264_TEST_FAKE_BITSTREAM
	EXTERN    char *rdptr;
       EXTERN    int rdsize;
#endif

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

/*------------------------------------------------------------------------------
   All H264 HW decoder should export and implement follwing APIs
------------------------------------------------------------------------------*/
int wmt_h264_probe(void);
int wmt_h264_get_capability(h264_drvinfo_t *drv);
int wmt_h264_set_attr(h264_drvinfo_t *drv);
int wmt_h264_decode_proc(h264_drvinfo_t *drv);
int wmt_h264_decode_finish(h264_drvinfo_t *drv);
int wmt_h264_open(h264_drvinfo_t *drv);
int wmt_h264_close(void);
int wmt_h264_isr(int irq, void *dev_id, struct pt_regs *regs);
int wmt_h264_flush(void);
int wmt_h264_suspend(void);


#endif /* ifndef HW_H264_H */

/*=== END hw-h264.h ==========================================================*/
