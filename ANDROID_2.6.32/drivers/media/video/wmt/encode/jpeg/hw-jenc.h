/* PVCS version log
** $Log:  $
 * 
 */
#ifndef HW_jenc_H
/* To assert that only one occurrence is included */
#define HW_jenc_H

/*--- hw-jenc.h ----------------------------------------------------------------
*   Copyright (C) 2008 WonderMedia Tech. Inc.
*
* MODULE       : hw-jenc.h
* AUTHOR       : Willy Chuang
* DATE         : 2008/11/25
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Willy Chuang, 2008/11/20
*	First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#ifdef __KERNEL__
#include "../../wmt-vdlib.h"
#endif
#include "../../wmt-vd.h"
#include "com-jenc.h"

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
#define jenc_IRQ_MODE
//#define JPEC_POLL_MODE


//#define jenc_DEBUG    /* Flag to enable debug message */
//#define jenc_DEBUG_DETAIL
//#define jenc_TRACE


#ifdef jenc_DEBUG
  #define DBG_MSG(fmt, args...)    PRINT("KERN_INFO {%s} " fmt, __FUNCTION__ , ## args)
#else
  #define DBG_MSG(fmt, args...)
#endif

#ifdef jenc_DEBUG_DETAIL
#define DBG_DETAIL(fmt, args...)   PRINT("KERN_DEBUG {%s} " fmt, __FUNCTION__ , ## args)
#else
#define DBG_DETAIL(fmt, args...)
#endif

#ifdef jenc_TRACE
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
    /* Error Status */
    STA_ERR_FLAG       = 0x80000000,
    STA_ERR_DMA_TX     = 0x80100000,
    STA_ERR_DECODING   = 0x80200000,
    STA_ERR_BAD_PRD    = 0x80400000,
    STA_ERR_UNKNOWN    = 0x80800000
} jenc_status;


typedef enum {
    jenc_SEG_NONE   = 0x00000000,
    jenc_SEG_FIRST  = 0x00000001,
    jenc_SEG_LAST   = 0x00000002
} jenc_segment;

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/
typedef struct {
    unsigned int      size;     /* sizeof(jenc_drvinfo_t) */
    jenc_attr_t       attr;     /* Come from user space for HW decoder */
    jenc_input_t      arg_in;   /* Come from user space */
    jenc_frameinfo_t  arg_out;  /* Return to user space later */

    jenc_pd           pd_mcu;
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

    jenc_capability_t *capab_tbl;
    
    /* Following member is used for multi-tasks */

    /* Following members are used for hw-jpeg.c only */
    unsigned int      _timeout;      
    jenc_status       _status;
    unsigned int      _multi_seg_en;
    unsigned int      _remain_byte;
    unsigned int      _set_attr_ready;
    jenc_segment      _segment_no;
    unsigned int      _segment_done;
    
#ifdef __KERNEL__
    spinlock_t        _lock;
#endif
} jenc_drvinfo_t;



/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef HW_JENC_C /* allocate memory for variables only in wm8510-jenc.c */
    #define EXTERN
#else
    #define EXTERN   extern
#endif /* ifdef WM8510_jenc_C */


/* EXTERN int      viaapi_xxxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

/*------------------------------------------------------------------------------
   All JPEG HW decoder should export and implement follwing APIs
------------------------------------------------------------------------------*/
int wmt_jenc_set_drv(jenc_drvinfo_t *drv);
jenc_drvinfo_t * wmt_jenc_get_drv(void);

int wmt_jenc_probe(void);
int wmt_jenc_set_attr(jenc_drvinfo_t *drv);
int wmt_jenc_decode_proc(jenc_drvinfo_t *drv);
int wmt_jenc_decode_finish(jenc_drvinfo_t *drv);
int wmt_jenc_decode_flush(jenc_drvinfo_t *drv);
int wmt_jenc_open(jenc_drvinfo_t *drv);
int wmt_jenc_close(jenc_drvinfo_t *drv);
#ifdef __KERNEL__
int wmt_jenc_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
void wmt_jenc_isr(void);
#endif
int wmt_jenc_suspend(void);
int wmt_jenc_resume(void);
int wmt_jenc_get_info(char *buf, char **start, off_t offset, int len);

    
#endif /* ifndef HW_jenc_H */

/*=== END hw-jenc.h ==========================================================*/
