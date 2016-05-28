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
* MODULE       : mpeg2.h
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
#ifndef WM865_MPEG2_H
#define WM865_MPEG2_H
/* To assert that only one occurrence is included */
//------------------------------------------------
//  Include Header
//------------------------------------------------
#include "com-mpeg2.h"

/*----------------- EXPORTED PRIVATE CONSTANTS -------------------------------*/

/*----------------- EXPORTED PRIVATE MACROS ----------------------------------*/
#define DBG_SIZE_CHECK(addr, _type_)  \
        if (((_type_ *)addr)->size != sizeof(_type_)){ \
            DBG_ERR("Size conflict (size: %d != %d)\n", ((_type_ *)addr)->size, sizeof(_type_)); \
            return -1; \
        }

/*----------------- EXPORTED PRIVATE TYPES -----------------------------------*/
//Example// typedef void api_xxx_t;
//------------------------------------------------
//  Definitions of enum
//------------------------------------------------
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
} mpeg2_status;

//------------------------------------------------
//  Definitions of structures
//------------------------------------------------
typedef struct {
  unsigned int       size;
  unsigned int       prd_virt;
  unsigned int       prd_addr;

  mpeg2_attr_t       attr;    /* Come from user space for HW decoder */
  mpeg2_input_t      arg_in;  /* Come from user space */
  mpeg2_frameinfo_t  arg_out; /* Return to user space later */
  mpeg2_capability_t *capab_tbl;

  /* Following members are used for hw-mpeg2.c only */
  mpeg2_status       _status;
#ifdef __KERNEL__
  spinlock_t         _lock;
#endif
} mpeg2_drvinfo_t;

/*----------------- EXPORTED PRIVATE VARIABLES -------------------------------*/
// allocate memory for variables only in XXX.c
#ifdef HW_MPEG2_C
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
/*------------------------------------------------------------------------------
   All MPEG2 HW decoder should export and implement follwing APIs
------------------------------------------------------------------------------*/
int wmt_mpeg2_get_capability(mpeg2_drvinfo_t *drv);
int wmt_mpeg2_set_attr(mpeg2_drvinfo_t *drv);
int wmt_mpeg2_decode_proc(mpeg2_drvinfo_t *drv);
int wmt_mpeg2_decode_finish(mpeg2_drvinfo_t *drv);
int wmt_mpeg2_isr(int irq, void *dev_id, struct pt_regs *regs);
int wmt_mpeg2_open(mpeg2_drvinfo_t *drv);
int wmt_mpeg2_close(void);
int wmt_mpeg2_probe(void);
int wmt_mpeg2_suspend(void);
int wmt_mpeg2_resume(void);

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/*=== END ====================================================================*/
#endif
