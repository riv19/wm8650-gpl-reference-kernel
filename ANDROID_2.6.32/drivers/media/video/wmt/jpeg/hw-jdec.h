/*++ 
 * drivers\media\video\wmt\jpeg\hw-jdec.h 
 * WonderMedia common SoC hardware JPEG decoder driver
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
#ifndef HW_JDEC_H
/* To assert that only one occurrence is included */
#define HW_JDEC_H

/*------------------------------------------------------------------------------
 * ChangeLog
 *
 * 2010-08-10  Willy Chunag  <willychuang@wondermedia.com.tw>
 *      - Add License declaration and ChangeLog
 *
------------------------------------------------------------------------------*/

/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#ifdef __KERNEL__
#include "../wmt-vdlib.h"
#endif
#include "../wmt-vd.h"
#include "com-jdec.h"

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

#ifdef __KERNEL__
    #include <linux/delay.h>     // for mdelay() only
    #define PRINT           printk
#else
    #define mdelay() 
    #define PRINT           printf
    #define KERN_ERR     
    #define KERN_WARNING
    #define KERN_INFO
    #define KERN_DEBUG
#endif

/* The followign two macros should be enable one and only one */
#define JDEC_IRQ_MODE
//#define JPEC_POLL_MODE


//#define JDEC_DEBUG    /* Flag to enable debug message */
//#define JDEC_DEBUG_DETAIL
//#define JDEC_TRACE


#ifdef JDEC_DEBUG
  #define DBG_MSG(fmt, args...)    PRINT("KERN_INFO {%s} " fmt, __FUNCTION__ , ## args)
#else
  #define DBG_MSG(fmt, args...)
#endif

#ifdef JDEC_DEBUG_DETAIL
#define DBG_DETAIL(fmt, args...)   PRINT("KERN_DEBUG {%s} " fmt, __FUNCTION__ , ## args)
#else
#define DBG_DETAIL(fmt, args...)
#endif

#ifdef JDEC_TRACE
  #define TRACE(fmt, args...)      PRINT("KERN_DEBUG {%s}:  " fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...) 
#endif

#define DBG_ERR(fmt, args...)      PRINT("KERN_ERR *E* {%s} " fmt, __FUNCTION__ , ## args)


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
    STA_BD_BAND_DONE   = 0x00000010, /* BD: Banding decode */
    /* Error Status */
    STA_ERR_FLAG       = 0x80000000,
    STA_ERR_DMA_TX     = 0x80100000,
    STA_ERR_DECODING   = 0x80200000,
    STA_ERR_BAD_PRD    = 0x80400000,
    STA_ERR_UNKNOWN    = 0x80800000
} jdec_status;


typedef enum {
    /* Mask for MSD 0x0F */
    JDEC_SEG_NONE   = 0x00000000,
    JDEC_SEG_FIRST  = 0x00000001,
    JDEC_SEG_LAST   = 0x00000002,

    /* Mask for Split Write 0xF0 */
    JDEC_SPLIT_EN    = 0x00000010,
    JDEC_SPLIT_FIRST = 0x00000020,
    JDEC_SPLIT_LAST  = 0x00000040    
} jdec_segment;

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/
typedef struct {
    unsigned int      size;     /* sizeof(jdec_drvinfo_t) */
    jdec_attr_t       attr;     /* Come from user space for HW decoder */
    jdec_input_t      arg_in;   /* Come from user space */
    jdec_frameinfo_t  arg_out;  /* Return to user space later */

    jdec_pd           pd_mcu;
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

    unsigned int      bpp;          /* bits per pixel */

    unsigned int      req_y_size; /* Y or RGB */
    unsigned int      req_c_size;

    jdec_capability_t *capab_tbl;
    
    /* Following member is used for multi-tasks */
    unsigned int      ref_num;

    /* Following members are used for hw-jpeg.c only */
    unsigned int      _timeout;      
    jdec_status       _status;
    unsigned int      _multi_seg_en;
    jdec_segment      _segment;
    unsigned int      _remain_byte;
    unsigned int      _set_attr_ready;
    unsigned int      _is_mjpeg;
    jdec_segment      _segment_no;
    unsigned int      _segment_done;
    unsigned int      _is_hw_init;
    
#ifdef __KERNEL__
    spinlock_t        _lock;
#endif
} jdec_drvinfo_t;

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef HW_JDEC_C 
    #define EXTERN
#else
    #define EXTERN   extern
#endif

EXTERN unsigned int  jdec_ref_cur;  /* for multi-tasks */

/* EXTERN int      viaapi_xxxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

/*------------------------------------------------------------------------------
   All JPEG HW decoder should export and implement follwing APIs
------------------------------------------------------------------------------*/
int wmt_jdec_set_drv(jdec_drvinfo_t *drv);
jdec_drvinfo_t * wmt_jdec_get_drv(void);

int wmt_jdec_probe(void);
int wmt_jdec_get_capability(jdec_drvinfo_t *drv);
int wmt_jdec_set_attr(jdec_drvinfo_t *drv);
int wmt_jdec_decode_proc(jdec_drvinfo_t *drv);
int wmt_jdec_decode_finish(jdec_drvinfo_t *drv);
int wmt_jdec_decode_flush(jdec_drvinfo_t *drv);
int wmt_jdec_get_status(jdec_drvinfo_t *drvinfo, jdec_status_t *jsts);
int wmt_jdec_open(jdec_drvinfo_t *drv);
int wmt_jdec_close(jdec_drvinfo_t *drv);
#ifdef __KERNEL__
int wmt_jdec_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
void wmt_jdec_isr(void);
#endif
int wmt_jdec_suspend(void);
int wmt_jdec_resume(void);
int wmt_jdec_get_info(char *buf, char **start, off_t offset, int len);

    
#endif /* ifndef HW_JDEC_H */

/*=== END hw-jdec.h ==========================================================*/
