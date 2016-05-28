/*++ 
 * linux/drivers/media/video/wmt/wmt-dsplib.h
 * WonderMedia DSP driver
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

#ifndef WMT_DSPLIB_H
/* To assert that only one occurrence is included */
#define WMT_DSPLIB_H

/*--- wmt-dsplib.h---------------------------------------------------------------
*   Copyright (C) 2008  WonderMedia Technologies, Inc.
*
* MODULE       : wmt-dsplib.h
* AUTHOR       : Kenny Chou
* DATE         : 2009/08/03
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#ifndef APPLICATION
#include <asm/types.h>
#else
#include <sys/types.h>
#endif


/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
extern unsigned long num_physpages;

#define DSS_IM_BASE         (num_physpages << PAGE_SHIFT)
#define DSS_IO_BASE         (0xF8000000)
#define DSS_DM_PAGE_MAPPING_BASE      (0xD8000000)
#define AHB_ARF_BASE        AUDREGF_BASE_ADDR
#define AHB_DSPCFG_BASE     PART_OF_AUDREGF_BASE_ADDR
#define AHB_MBOX_BASE       DSS_MBOX_BASE_ADDR
#define AHB_PERM_BASE       DSS_PERM_BASE_ADDR
#define DSS_DM_OFFSET       (0x6E000)
#define DSP_WORKING_MEM_SZ       (0x200000)
#define DSP_PC_DEFAULT_VAL       (0xFFFFFFFF)
#define DSP_RAW_BUF_SZ 		PAGE_SIZE * 256	/* 1MB */

#define VD_JPEG    0x0001


/******************** Register Definition ********************/
//MBOX (AHB0 side) detail
#define AHB_MBOX_RXCMD      (0x00000000 + AHB_MBOX_BASE)
#define AHB_MBOX_RXW1       (0x00000004 + AHB_MBOX_BASE)
#define AHB_MBOX_RXW2       (0x00000008 + AHB_MBOX_BASE)
#define AHB_MBOX_RXW3       (0x0000000C + AHB_MBOX_BASE)

#define AHB_MBOX_TXCMD      (0x00000010 + AHB_MBOX_BASE)
#define AHB_MBOX_TXW1       (0x00000014 + AHB_MBOX_BASE)
#define AHB_MBOX_TXW2       (0x00000018 + AHB_MBOX_BASE)
#define AHB_MBOX_TXW3       (0x0000001C + AHB_MBOX_BASE)

#define AHB_MBOX_STAT       (0x00000020 + AHB_MBOX_BASE)


//PERM (AHB0 side) detail
#define AHB_PERM_PC_TBF_UPD     (0x00000000 + AHB_PERM_BASE)
#define AHB_PERM_PC_TRBF_CMD    (0x00000004 + AHB_PERM_BASE)
#define AHB_PERM_PC_TRBF        (0x00000008 + AHB_PERM_BASE)
#define AHB_PERM_PC_TBF_STAT    (0x0000000C + AHB_PERM_BASE)


#define ARF_GPI0            (0x00000000 + AHB_ARF_BASE)
#define ARF_GPI1            (0x00000004 + AHB_ARF_BASE)
#define ARF_GPO0            (0x00000008 + AHB_ARF_BASE)
#define ARF_GPO1            (0x0000000C + AHB_ARF_BASE)

#define ARF_ADSS_IMBASE     (0x00000040 + AHB_DSPCFG_BASE)
#define ARF_ADSS_DMBASE     (0x00000044 + AHB_DSPCFG_BASE)
#define ARF_ADSS_IOBASE     (0x00000048 + AHB_DSPCFG_BASE)
#define ARF_ADSSRST         (0x0000004C + AHB_DSPCFG_BASE)



/******************** ARM to DSP Mail Box Command ********************/
#define CMD_EXEC_DSP_IDLE       (0xE000000C)

#define CMD_A2D_VDEC_ATTR   (0xE1000030)
#define CMD_A2D_VDEC        (0xE1000031)
#define CMD_A2D_VDEC_OPEN   (0xE1000033)
#define CMD_A2D_VDEC_CLOSE  (0xE1000034)
#define CMD_A2D_VDEC_FLUSH  (0xE1000035)
#define CMD_A2D_VDEC_NEW_SEG	(0xE1000037)

/******************** Video Encode ***************/
//modify by aksenxu 20101224
#define CMD_A2D_VENC_ATTR         (0xE1000060UL)
#define CMD_A2D_VENC              (0xE1000061UL)
#define CMD_A2D_VENC_OPEN         (0xE1000063UL)
#define CMD_A2D_VENC_CLOSE        (0xE1000064UL)
#define CMD_A2D_VENC_FLUSH        (0xE1000065UL)
#define CMD_A2D_VENC_NEW_SEG      (0xE1000067UL)
//end

/******************** DSP to ARM Mail Box Command ********************/
#define CMD_DBG_PRINT       (0xD0000017)
#define CMD_DBG_PRINTF      (0xD000001E)
#define CMD_DBG_PRINTS      (0xD000001F)

#define CMD_D2A_VDEC_DONE	  (0xE1000032)
#define CMD_D2A_VDEC_SEG_DONE (0xE1000036)

//modify by aksenxu 20101224
#define CMD_D2A_VENC_DONE         (0xE1000062UL)
#define CMD_D2A_VENC_SEG_DONE     (0xE1000066UL)
//end
/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
typedef struct dsplib_mbox_s
{
    unsigned int cmd;
    unsigned int w1;
    unsigned int w2;
    unsigned int w3;
}dsplib_mbox_t;

typedef struct dsp_done_s {
    int bs_type;
    int decode_status;
    int frame_bitcnt;
} dsp_done_t;

typedef struct dsplib_fw_s {
  char *name;
  char *start_addr;
  unsigned int length;
} dsplib_fw_t;

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_DSPLIB_C /* allocate memory for variables only in wmt_audio.c */
#define EXTERN
#else
#define EXTERN   extern
#endif /* ifdef WMT_DSPLIB_C */



#undef EXTERN


/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/


/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
void dsplib_initial(struct device *device);
void dsplib_release(void);
int dsplib_suspend(struct device *dev, pm_message_t state);
int dsplib_resume(struct device *dev);
int dsplib_cmd_recv(dsplib_mbox_t *mbox, unsigned int cmd);
int dsplib_cmd_send(
    unsigned int cmd,
    unsigned int w1,
    unsigned int w2,
    unsigned int w3);


#endif 

/*=== END wmt-dsplib.h ==========================================================*/
