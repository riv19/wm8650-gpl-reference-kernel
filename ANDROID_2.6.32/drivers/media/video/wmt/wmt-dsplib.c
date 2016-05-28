/*++ 
 * linux/drivers/media/video/wmt/wmt-dsplib.c
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
#define WMT_DSPLIB_C

/*--- wmt-dsplib.c ---------------------------------------------------------------
*   Copyright (C) 2008  WonderMedia Technologies, Inc.
*
* MODULE       : wmt-dsplib.c
* AUTHOR       : Kenny Chou
* DATE         : 2009/08/03
*
* DESCRIPTION  : memory map of DSP usage
*
*		Reserved memory size for DSP raw files: 1MB
*  raw file buffer
*				+---------------+
*				|  All of raw	|
*				|	  files		|
*				|				|
*				|				|
*				+---------------+
*
*		Reserved memory size for DSP working area: 2MB
*  working area	+---------------+	DSS_IM_BASE
*				|		PM		|
*				|				|
*				|				|
*				|				|
*				+---------------+	DSS_DM_BASE = DSS_IM_BASE + 0x6E000
*				|		DM		|
*				|				|
*				|				|
*				|				|
*				+---------------+
*
*-----------------------------------------------------------------------------*/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <asm/sizes.h>
#include <asm/irq.h>
#include <mach/irqs.h>


#include "wmt-dsplib.h"

#define DSP_TIME_OUT      (10000 * HZ / 1000)
//#define LOAD_FW_ALWAYS

/*----------------------- INTERNAL PRIVATE MARCOS -------------------------*/
//#define DEBUG
#ifdef DEBUG
#define DBG(fmt, args...)         printk("[%s]:  " fmt, __FUNCTION__ , ## args)
#else
#define DBG(fmt, args...)    
#endif

#define DBG_ERR(fmt, args...)    printk("*E* {%s} " fmt, __FUNCTION__ , ## args)

//#define DSP_TRACE
#ifdef DSP_TRACE
  #define TRACE(fmt, args...)    printk("{%s}:  " fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...) 
#endif

/*----------------------- INTERNAL PRIVATE VARIABLES -------------------------*/
DECLARE_WAIT_QUEUE_HEAD(dsp_irq_event);

spinlock_t dsp_irqlock = SPIN_LOCK_UNLOCKED;

/* for DSP interface event handle */
volatile unsigned char dsp_dec_done_flag;
volatile unsigned char dsp_reset_flag;

/* variables for DSP */
volatile unsigned int dsp_idle_pc = DSP_PC_DEFAULT_VAL;
static unsigned int mbox_rx_cmd;
static unsigned int mbox_rx_w1;
static unsigned int mbox_rx_w2;
static unsigned int mbox_rx_w3;
volatile char __iomem *dsp_work_addr;
char dsp_print_buf[256];
unsigned decoder_type = 0;
static struct device *vd_dev;
char *dsp_raw_buf = NULL;
char *dsp_raw_buf_ptr = NULL;
static dma_addr_t dsp_raw_buf_phys = 0;

dsplib_fw_t dsp_name[] = {
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"wm8650_vdec_jpeg.pm.raw", 0, 0}, {"wm8650_vdec_jpeg.dm.raw", 0, 0},
								 {"wm8650_vdec_mpeg.pm.raw", 0, 0}, {"wm8650_vdec_mpeg.dm.raw", 0, 0},
								 {"wm8650_vdec_mpeg4.pm.raw", 0, 0}, {"wm8650_vdec_mpeg4.dm.raw", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"wm8650_vdec_h264.pm.raw", 0, 0}, {"wm8650_vdec_h264.dm.raw", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"NULL", 0, 0}, {"NULL", 0, 0},
								 {"wm8650_venc_jpeg.pm.raw",0,0}, {"wm8650_venc_jpeg.dm.raw",0,0},
								 {"END", 0, 0}
									};



/*----------------------- INTERNAL PRIVATE FUNCTIONS -------------------------*/
static int dsplib_mbox_tx(unsigned int cmd, unsigned int w1, unsigned int w2, unsigned int w3, int flag);
static irqreturn_t dsplib_isr_mbox_rx(int this_irq, void *dev_id);
static void dsplib_dsp_printf(uint32_t lStrAddr);
static void dsplib_dsp_prints(uint32_t lStrAddr, uint32_t w2, uint32_t w3);
static void dsplib_wakeup_dsp(unsigned char reload_flag);
static void dsplib_enter_idle(void);
static int dsplib_firmware_ld(void);
static void dsplib_fw_reload(void);
static void dump_dsp_pc(void);
static void dsplib_clock_ctrl(unsigned char enable_flag);


#ifdef CONFIG_WMT_VIDEO_DECODE_JPEG
    extern int wmt_jdec_seg_done(void);
    extern int wmt_jdec_decode_done(void);
#endif

extern unsigned int wmt_read_oscr(void);



/*!*************************************************************************
* dsplib_firmware_ld
* 
* Private Function by Kenny Chou, 2010/11/22
*/
/*!
* \brief
*        
* \retval 
*/ 
static int dsplib_firmware_ld(void)
{
	int ret = 0;
	const struct firmware *fw;
	int i;

	TRACE("Enter\n");

	/* loading all of DSP firmware from /lib/firmware to memory */
	for (i = 1; ; ) {
		if (!strcmp(dsp_name[i].name, "NULL")) {
			i++;
			continue;
		}
		else if (!strcmp(dsp_name[i].name, "END")) {
			break;
		}

		ret = request_firmware(&fw, dsp_name[i].name, vd_dev);
		if (ret != 0) {
			printk(KERN_ALERT "[%s] Failed to loading %s \n", __FUNCTION__, dsp_name[i].name);
			return -1;
		}

		memcpy((unsigned int *)dsp_raw_buf_ptr, fw->data, fw->size);
		
		dsp_name[i].start_addr = dsp_raw_buf_ptr;
		dsp_name[i].length = fw->size;

		dsp_raw_buf_ptr += fw->size;
		
		release_firmware(fw);
		
		if (dsp_raw_buf_ptr >= (dsp_raw_buf + DSP_RAW_BUF_SZ)) {
			printk(KERN_ALERT "[%s] dsp_raw_buf overflow! \n", __FUNCTION__);
			return -1;
		}

		i++;
	}

#if 0
	for (i = 0; i < 36; i++) {
		printk(KERN_ALERT "[%s] name=%s, addr=0x%x, length=%d \n", __FUNCTION__,
			dsp_name[i].name, dsp_name[i].start_addr, dsp_name[i].length);
	}
#endif	
	
	TRACE("Leave\n");

	return ret;
}


/*!*************************************************************************
* dsplib_fw_reload
* 
* Private Function by Kenny Chou, 2010/12/10
*/
/*!
* \brief
*        
* \retval 
*/ 
static void dsplib_fw_reload(void)
{
	memcpy((unsigned int *)dsp_work_addr,
		(unsigned int *)dsp_name[decoder_type * 2].start_addr,
		dsp_name[decoder_type * 2].length);

	memcpy((unsigned int *)(dsp_work_addr + DSS_DM_OFFSET),
		(unsigned int *)dsp_name[(decoder_type * 2) + 1].start_addr,
		dsp_name[(decoder_type * 2) + 1].length);

#if 0
	printk(KERN_ALERT "[dsplib_fw_reload] name=%s, length=%d, \n",
				dsp_name[decoder_type * 2].name, dsp_name[decoder_type * 2].length);
	printk(KERN_ALERT "[dsplib_fw_reload] name=%s, length=%d, \n",
				dsp_name[(decoder_type * 2) + 1].name, dsp_name[(decoder_type * 2) + 1].length);
#endif	
}


/*!*************************************************************************
* dsplib_initial
* 
* Private Function by Kenny Chou, 2010/11/16
*/
/*!
* \brief
*        
* \retval 
*/ 
void dsplib_initial(struct device *device)
{
	TRACE("Enter\n");

	/* disable DSP clock */
	dsplib_clock_ctrl(0);
	
	vd_dev = device;
	dsp_work_addr = ioremap(DSS_IM_BASE, DSP_WORKING_MEM_SZ);

	/* allocate non-cached memory for DSP raw files buffer, size = 1MB */
    dsp_raw_buf = dma_alloc_coherent(vd_dev, DSP_RAW_BUF_SZ, &dsp_raw_buf_phys, GFP_KERNEL|GFP_DMA);
    
    if (!dsp_raw_buf) {
        printk(KERN_ALERT "[%s] *** dma alloc ERROR: dsp_raw_buf *** \n", __FUNCTION__);
        return;
    }
    else {
        memset(dsp_raw_buf, 0x00, DSP_RAW_BUF_SZ);

        /* initial buffer pointer */
        dsp_raw_buf_ptr = dsp_raw_buf;
    }

	/* request resource for mail box interrupt */
    if (request_irq(IRQ_DSP, &dsplib_isr_mbox_rx, IRQF_DISABLED, "wmt-dsplib", NULL) < 0) {      
       	printk(KERN_ALERT "[%s] Failed to register IRQ_DSP \n", __FUNCTION__);
    }

	dsp_dec_done_flag = 0;
	dsp_reset_flag = 0;
	TRACE("Leave\n");
}


/*!*************************************************************************
* dump_dsp_pc
* 
* Private Function by Willy Chuang, 2010/11/06
*/
/*!
* \brief
*
*
* \retval  0 if success
*/ 
static void dump_dsp_pc(void)
{
    unsigned int stat = 0;
    
    do {
        stat = REG32_VAL(AHB_PERM_PC_TBF_STAT);
    }
    while (stat != 0x01);
    
    REG32_VAL(AHB_PERM_PC_TRBF_CMD) = 0x0;
    
    do {
        stat = REG32_VAL(AHB_PERM_PC_TBF_STAT);
    }
    while (stat != 0x01);
    
    printk(KERN_ALERT"GPO0=0x%x, GPO1=0x%x  \n", REG32_VAL(ARF_GPO0), REG32_VAL(ARF_GPO1));
    printk(KERN_ALERT"This is PC = 0x%x \n", REG32_VAL(AHB_PERM_PC_TRBF));
}


/*!*************************************************************************
* dsplib_enter_idle
* 
* Private Function by Kenny Chou, 2009/08/03
*/
/*!
* \brief
*        
* \retval 
*/ 
static void dsplib_enter_idle(void)
{
    unsigned int dsp_pc = 0;
    unsigned int stat = 0;
    unsigned int start_time = 0;
    
    if (dsp_idle_pc == DSP_PC_DEFAULT_VAL) {
		//printk(KERN_ALERT "[%s] DSP already in Idle mode! \n", __FUNCTION__);
		return;
    }

	if (dsplib_mbox_tx(CMD_EXEC_DSP_IDLE, 0, 0, 0, 0)) {
		dsp_idle_pc = DSP_PC_DEFAULT_VAL;
		//printk(KERN_ALERT "[%s] idle command fail! \n", __FUNCTION__);
		return;
	}

	start_time = wmt_read_oscr();
	
    while (dsp_idle_pc != dsp_pc) {
    	do {
        	stat = REG32_VAL(AHB_PERM_PC_TBF_STAT);
    	}
    	while (stat != 0x01);
    
        REG32_VAL(AHB_PERM_PC_TRBF_CMD) = 0x0;

        do {
            stat = REG32_VAL(AHB_PERM_PC_TBF_STAT);
        }
        while (stat != 0x01);

        dsp_pc = REG32_VAL(AHB_PERM_PC_TRBF);
        //printk(KERN_ALERT "dsp_pc=0x%x \n", dsp_pc);

        if (((wmt_read_oscr() - start_time) / 3000) > 300) {
        	printk(KERN_ALERT "[%s] TIME OUT! \n", __FUNCTION__);
        	dsp_reset_flag = 1;
        	break;
        }
    }

	dsp_idle_pc = DSP_PC_DEFAULT_VAL;
	//printk(KERN_ALERT "[%s] DSP Idle Done! \n", __FUNCTION__);
}


/*!*************************************************************************
* dsplib_clock_ctrl
* 
* Private Function by Kenny Chou, 2011/01/20
*/
/*!
* \brief
*        
* \retval 
*/ 
static void dsplib_clock_ctrl(unsigned char enable_flag)
{
	unsigned int stat;

	TRACE("Enter\n");

	if (enable_flag) {
		//printk(KERN_ALERT "[%s] enable DSP clock \n", __FUNCTION__);
		/* enable Bit0 DSP Power Control of DSP Power Shut Off Control and Status Register */
		stat = PMDSPPWR_VAL;
		stat |= BIT0;
		PMDSPPWR_VAL = stat;

		while(PMDSPPWR_VAL & BIT9);

    	/* enable Bit25 DSP and Bit28 MBOX Clock Enable of Clock Enables Lower Register */
		stat = PMCEL_VAL;
    	stat |= (BIT25 | BIT28);
    	PMCEL_VAL = stat;

    	while(PMCEL_VAL != stat);

    	/* enable Bit1 NA0, Bit8 PERM and Bit9 DSP CFG Clock Enable of Clock Enables Upper Register */
		stat = PMCEU_VAL;
    	stat |= (BIT1 | BIT8 | BIT9);
    	PMCEU_VAL = stat;

    	while(PMCEU_VAL != stat);
	}
	else {
		//printk(KERN_ALERT "[%s] disable DSP clock \n", __FUNCTION__);
		/* disable Bit0 DSP Power Control of DSP Power Shut Off Control and Status Register */
    	stat = PMDSPPWR_VAL;
		stat &= ~BIT0;
		PMDSPPWR_VAL = stat;

		while(PMDSPPWR_VAL & BIT8);

    	/* disable Bit25 DSP and Bit28 MBOX Clock Enable of Clock Enables Lower Register */
    	stat = PMCEL_VAL;
		stat &= ~(BIT25 | BIT28);
		PMCEL_VAL = stat;

		while(PMCEL_VAL != stat);

    	/* disable Bit1 NA0, Bit8 PERM and Bit9 DSP CFG Clock Enable of Clock Enables Upper Register */
    	stat = PMCEU_VAL;
		stat &= ~(BIT1 | BIT8 | BIT9);
		PMCEU_VAL = stat;

		while(PMCEU_VAL != stat);
	}
	
	TRACE("Leave\n");
}


/*!*************************************************************************
* dsplib_wakeup_dsp
* 
* Private Function by Kenny Chou, 2009/08/03
*/
/*!
* \brief
*        
* \retval 
*/ 
static void dsplib_wakeup_dsp(unsigned char reload_flag)
{
	unsigned int stat;
	//unsigned char *w1_ptr;
	
    TRACE("Enter\n");

	/* check Bit25 DSP Clock Enable of Clock Enables Lower Register at first */
	stat = PMCEL_VAL;
	stat &= BIT25;
	
	if (!stat) {
		/* enable DSP clock */
		dsplib_clock_ctrl(1);
	}
	
	REG32_VAL(ARF_ADSSRST) = 0x00;

	if (reload_flag) {
		//printk(KERN_ALERT "[%s] reload DSP FW \n", __FUNCTION__);
		
		/* execute DSP firmware */
		REG32_VAL(ARF_ADSS_IMBASE) = DSS_IM_BASE;
		REG32_VAL(ARF_ADSS_DMBASE) = DSS_DM_PAGE_MAPPING_BASE;
		REG32_VAL(ARF_ADSS_IOBASE) = DSS_IO_BASE;
	}

	//printk(KERN_ALERT "[%s] reset DSP! \n", __FUNCTION__);

	REG32_VAL(ARF_ADSSRST) = 0x03;        // Bit0: uDSP core reset, Bit1: ADSS reset

    /*
	 * Waiting for receive RX_W1:Bit1, RX_W2:Bit2 and RX_W3:Bit3,
	 * that means DSP is ready to work.
	 */
	do
    {
        stat = REG32_VAL(AHB_MBOX_STAT);
        //dump_dsp_pc();
    }while((stat & 0x0000000F) != 0x0000000E);

	stat = REG32_VAL(AHB_MBOX_RXW3);				/* w3 => dummy read, always 0 */
    stat = REG32_VAL(AHB_MBOX_RXW1);				/* w1 => DSP version */
    dsp_idle_pc = REG32_VAL(AHB_MBOX_RXW2);		/* w2 => PC of DSP Idle */

    /*w1_ptr = (unsigned char *)&stat;
	printk(KERN_ALERT "DSP %d.%02d.%02d.%02d \n",
		*(w1_ptr + 3), *(w1_ptr + 2), *(w1_ptr + 1), *w1_ptr);*/

	//printk(KERN_ALERT "[%s] dsp_idle_pc=0x%x \n", __FUNCTION__, dsp_idle_pc);

    dsp_reset_flag = 0;
    TRACE("Leave\n");

    return;
}


/*!*************************************************************************
* dsplib_mbox_tx
* 
* Private Function by Kenny Chou, 2009/08/03
*/
/*!
* \brief
*        
* \retval  
*/ 
static int dsplib_mbox_tx(
    unsigned int cmd,
    unsigned int w1, 
    unsigned int w2, 
    unsigned int w3, 
    int flag)
{
    unsigned int stat = 0;
    unsigned long flags = 0;
    unsigned int start_time = 0;
    int ret = 0;

    TRACE("Enter\n");

    if (dsp_reset_flag) {
    	ret = -1;
    	return ret;
    }
    
    if (flag) {
        spin_lock_irqsave(&dsp_irqlock, flags);
    }

    start_time = wmt_read_oscr();
    
    do
    {
        stat = REG32_VAL(AHB_MBOX_STAT);
        
        if (((wmt_read_oscr() - start_time) / 3000) > 1000) {
        	printk(KERN_ALERT "[%s] TIME OUT! \n", __FUNCTION__);
        	dsp_reset_flag = 1;
        	ret = -1;
        	return ret;
        }
    }while((stat & 0x000000F0) != 0x000000F0);

    REG32_VAL(AHB_MBOX_TXW1) = w1;
    REG32_VAL(AHB_MBOX_TXW2) = w2;
    REG32_VAL(AHB_MBOX_TXW3) = w3;
    REG32_VAL(AHB_MBOX_TXCMD) = cmd;

    if (flag) {
        spin_unlock_irqrestore(&dsp_irqlock, flags);
    }

    TRACE("Leave\n");
	return ret;
}


/*!*************************************************************************
* dsplib_isr_mbox_rx
* 
* Private Function by Kenny Chou, 2009/08/03
*/
/*!
* \brief
*        
* \retval 
*/ 
static irqreturn_t dsplib_isr_mbox_rx(
    int this_irq,
    void *dev_id)
{
    unsigned long flags;

    TRACE("Enter\n");
    spin_lock_irqsave(&dsp_irqlock, flags);
    
    mbox_rx_w1 = REG32_VAL(AHB_MBOX_RXW1);
    mbox_rx_w2 = REG32_VAL(AHB_MBOX_RXW2);
    mbox_rx_w3 = REG32_VAL(AHB_MBOX_RXW3);
    mbox_rx_cmd = REG32_VAL(AHB_MBOX_RXCMD);

    switch (mbox_rx_cmd)
    {
        case CMD_DBG_PRINT:                        /* print DSP information for debug used */
            printk(KERN_ALERT "[DSP_DBG:CMD_DBG_PRINT] 0x%08X 0x%08X 0x%08X \n" ,mbox_rx_w1, mbox_rx_w2, mbox_rx_w3);
            break;
        case CMD_DBG_PRINTF:                       /* print DSP formatted string for debug used */
            dsplib_dsp_printf(mbox_rx_w1);
            break;
        case CMD_DBG_PRINTS:                       /* print DSP string with w2,w3 for debug used */
            dsplib_dsp_prints(mbox_rx_w1, mbox_rx_w2, mbox_rx_w3);
            break;
        case CMD_D2A_VDEC_DONE:
#ifdef CONFIG_WMT_VIDEO_DECODE_JPEG        	
            if(decoder_type == VD_JPEG)
                wmt_jdec_decode_done();
#endif            
        	dsp_dec_done_flag = 1;
            wake_up_interruptible(&dsp_irq_event);
            break;
        case CMD_D2A_VENC_DONE:
        	dsp_dec_done_flag = 1;
            wake_up_interruptible(&dsp_irq_event);
            break;            
        case CMD_D2A_VDEC_SEG_DONE:
#ifdef CONFIG_WMT_VIDEO_DECODE_JPEG        	
            if(decoder_type == VD_JPEG)
                wmt_jdec_seg_done();
#endif            
            break;
        default:    
            break;
    }

    spin_unlock_irqrestore(&dsp_irqlock, flags);
    TRACE("Leave (cmd: 0x%x)\n", mbox_rx_cmd);
    
    return IRQ_HANDLED;
}


/*!*************************************************************************
* dsplib_release
* 
* Private Function by Kenny Chou, 2009/08/03
*/
/*!
* \brief
*        
* \retval  0 if success
*/ 
void dsplib_release(void)
{
	if (dsp_idle_pc != DSP_PC_DEFAULT_VAL) {
		dsplib_enter_idle();
		dsplib_clock_ctrl(0);
	}
	
    free_irq(IRQ_DSP, 0);
    dma_free_coherent(vd_dev, DSP_RAW_BUF_SZ, dsp_raw_buf, dsp_raw_buf_phys);
    decoder_type = 0;
}


/*!*************************************************************************
* dsplib_suspend
* 
* Private Function by Kenny Chou, 2009/08/03
*/
/*!
* \brief
*        
* \retval  0 if success
*/ 
int dsplib_suspend(struct device *dev, pm_message_t state)
{
    TRACE("Enter\n");
    switch ( state.event ) {
        case PM_EVENT_SUSPEND:
        	if (dsp_idle_pc != DSP_PC_DEFAULT_VAL) {
        		if (!dsp_reset_flag) {
					dsplib_enter_idle();
        		}
        		else {
        			dsp_idle_pc = DSP_PC_DEFAULT_VAL;
        		}
        		
				dsplib_clock_ctrl(0);
        	}
			break;
        case PM_EVENT_FREEZE:
        case PM_EVENT_PRETHAW:
            break;
    }
    TRACE("Leave\n");

    return 0;
} 


/*!*************************************************************************
* dsplib_resume
* 
* Private Function by Kenny Chou, 2009/08/03
*/
/*!
* \brief
*        
* \retval  0 if success
*/ 
int dsplib_resume(struct device *dev)
{
    TRACE("Enter\n");
	//dsplib_wakeup_dsp();
    TRACE("Leave\n");

    return 0;
}


/*!*************************************************************************
* dsplib_dsp_printf
* 
* Private Function by Akria Sheng, 2009/09/30
*/
/*!
* \brief
*        
* \retval  0 if success
*/ 
static void dsplib_dsp_printf(uint32_t lStrAddr)
{
  int16_t *spDSPStr;
  int8_t *cpDSPStr, cTmp;
  int16_t str_length;

  spDSPStr = (int16_t *)&dsp_work_addr[lStrAddr];
  cpDSPStr = &dsp_print_buf[0];
  str_length = 0;
  while (1)
  {
    cTmp = (char)*spDSPStr++;
    if (cTmp == '\0') break;
    if (str_length>=254) break;
    *cpDSPStr++ = cTmp;
    str_length++;
  }
  *cpDSPStr++ = '\0';
  printk(KERN_ALERT "%s" , &dsp_print_buf[0]);
}


/*!*************************************************************************
* dsplib_dsp_prints
* 
* Private Function by Akria Sheng, 2009/09/30
*/
/*!
* \brief
*        
* \retval  0 if success
*/ 
static void dsplib_dsp_prints(uint32_t lStrAddr, uint32_t w2, uint32_t w3)
{
  int16_t *spDSPStr;
  int8_t *cpDSPStr, cTmp;
  int16_t str_length;

  spDSPStr = (int16_t *)&dsp_work_addr[lStrAddr];
  cpDSPStr = &dsp_print_buf[0];
  str_length = 0;
  while (1)
  {
    cTmp = (char)*spDSPStr++;
    if (cTmp == '\0') break;
    if (str_length>=254) break;
    *cpDSPStr++ = cTmp;
    str_length++;
  }
  *cpDSPStr++ = '\0';
  printk(KERN_ALERT "%s (w2:%#08x, w3:%#08x)\n" , &dsp_print_buf[0], w2, w3);
}


/*!*************************************************************************
* dsplib_cmd_send
* 
* Private Function by Willy Chuang, 2010/10/29
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
int dsplib_cmd_send(
    unsigned int cmd,
    unsigned int w1,
    unsigned int w2,
    unsigned int w3)
{
    dsplib_mbox_t mbox;
    int ret = 0;
    
    TRACE("Enter\n");

    mbox.cmd = cmd;
    mbox.w1  = w1;
    mbox.w2  = w2;
    mbox.w3  = w3; 
    
    DBG("mbox.cmd: 0x%x\n", mbox.cmd);
    DBG("mbox.w1:  0x%x\n", mbox.w1);
    DBG("mbox.w2:  0x%x\n", mbox.w2);
    DBG("mbox.w3:  0x%x\n", mbox.w3);

    if ((mbox.cmd == CMD_A2D_VDEC_OPEN) || (mbox.cmd == CMD_A2D_VENC_OPEN)) {
		if (!decoder_type) {
			/* loading all of DSP firmware from /lib/firmware to memory */
			ret = dsplib_firmware_ld();
			if (ret)
				return ret;
		}
		//printk(KERN_ALERT "[%s] CMD_A2D_OPEN \n", __FUNCTION__);
    	
#ifdef LOAD_FW_ALWAYS
		/* copy DSP firmware always */
		decoder_type = w1;
		
		dsplib_fw_reload();
		dsplib_wakeup_dsp(1);
#else
		/* copy DSP firmware if necessary */
		if (decoder_type != w1) {
			//printk(KERN_ALERT "[%s] format change \n", __FUNCTION__);
			decoder_type = w1;
			dsplib_fw_reload();
			dsplib_wakeup_dsp(1);
		}
		else if (dsp_idle_pc == DSP_PC_DEFAULT_VAL) {
			//printk(KERN_ALERT "[%s] wakeup_dsp \n", __FUNCTION__);
			dsplib_wakeup_dsp(1);
		}
		else if (dsp_reset_flag) {
			//printk(KERN_ALERT "[%s] dsp_reset_flag=1 \n", __FUNCTION__);
			dsplib_enter_idle();
			dsplib_wakeup_dsp(0);
		}
#endif		
	}
   	else if ((mbox.cmd == CMD_A2D_VDEC) || (mbox.cmd == CMD_A2D_VENC)) {
		dsp_dec_done_flag = 0;
	}
    	
	ret = dsplib_mbox_tx(mbox.cmd, mbox.w1, mbox.w2, mbox.w3, 0);

	if ((mbox.cmd == CMD_A2D_VDEC_CLOSE) || (mbox.cmd == CMD_A2D_VENC_CLOSE)) {
		//printk(KERN_ALERT "[%s] CMD_A2D_CLOSE \n", __FUNCTION__);
		dsplib_enter_idle();
		dsplib_clock_ctrl(0);
	}

    TRACE("Leave\n");
	return ret;
} /* End of dsplib_cmd_send() */


/*!*************************************************************************
* dsplib_cmd_recv
* 
* Private Function by Willy Chuang, 2010/10/29
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
int dsplib_cmd_recv(dsplib_mbox_t *mbox, unsigned int cmd)
{
    dsp_done_t *dspinfo;
    int ret = 0;
    /*int i, data_cnt;
    unsigned int *dsp_data_ptr;*/

    TRACE("Enter\n");

    if (dsp_reset_flag) {
    	ret = -1;
    	return ret;
    }

    mbox->cmd = cmd;
    
    DBG("Received DSP Msg (cmd: 0x%x)\n", mbox->cmd);

    if ((mbox->cmd == CMD_D2A_VDEC_DONE) || (mbox->cmd == CMD_D2A_VENC_DONE)){
        ret = wait_event_interruptible_timeout(dsp_irq_event, (dsp_dec_done_flag != 0), DSP_TIME_OUT);

        dspinfo = (dsp_done_t *)phys_to_virt(mbox_rx_w1);
        
        DBG("bs_type:       0x%x\n", dspinfo->bs_type);
        DBG("decode_status: 0x%x\n", dspinfo->decode_status);
        DBG("frame_bitcnt:  0x%x\n", dspinfo->frame_bitcnt);
        
        if( dspinfo->decode_status != 0 ) {
            /* DSP return error */
            DBG_ERR("Some error happend in DSP (code: %d)\n", dspinfo->decode_status);
			dspinfo->decode_status = 0; // test only            
        }

        if (ret == 0) {
            printk(KERN_ALERT"*************************\n");
            printk(KERN_ALERT" DSP Decode Time Out in %s! \n", __FUNCTION__);

            dump_dsp_pc();
                      
            printk(KERN_ALERT"Please send this log to DSP RD to analyze\n");
            printk(KERN_ALERT"*************************\n");

#if 0
			dsp_data_ptr = (unsigned int *)dsp_work_addr;

			printk(KERN_ALERT "[DSP_DBG:CMD_DBG_PRINT] name=%s, length=%d, \n",
				dsp_name[decoder_type * 2].name, dsp_name[decoder_type * 2].length);

			for (i = 0, data_cnt = 0; data_cnt <= (dsp_name[decoder_type * 2].length); ) {
				printk(KERN_ALERT "[DSP_DBG:CMD_DBG_PRINT] 0x%08x 0x%08x 0x%x \n",
					*(dsp_data_ptr + i), *(dsp_data_ptr + i + 1), (0x80000000 + data_cnt));

				i += 2;
				data_cnt += 8;
			}

			dsp_data_ptr = (unsigned int *)(dsp_work_addr + DSS_DM_OFFSET);
			printk(KERN_ALERT "[DSP_DBG:CMD_DBG_PRINT] name=%s, length=%d, \n",
				dsp_name[(decoder_type * 2) + 1].name, dsp_name[(decoder_type * 2) + 1].length);

			for (i = 0, data_cnt = 0; data_cnt <= (dsp_name[(decoder_type * 2) + 1].length); ) {
				printk(KERN_ALERT "[DSP_DBG:CMD_DBG_PRINT] 0x%08x 0x%08x 0x%x \n",
					*(dsp_data_ptr + i), *(dsp_data_ptr + i + 1), (0x80000000 + data_cnt));

				i += 2;
				data_cnt += 8;
			}
#endif
			dsp_reset_flag = 1;
            ret = -1;
            return ret;
        }

        ret = 0;
    }

    mbox->cmd = mbox_rx_cmd;
    mbox->w1 = mbox_rx_w1;
    mbox->w2 = mbox_rx_w2;
    mbox->w3 = mbox_rx_w3;

    DBG("mbox.w1:  0x%x\n", mbox->w1);
    DBG("mbox.w2:  0x%x\n", mbox->w2);
    DBG("mbox.w3:  0x%x\n", mbox->w3);

    TRACE("Leave\n");
    
    return ret;
} /* End of dsplib_cmd_recv() */


/*--------------------End of Function Body -----------------------------------*/

#undef WMT_DSPLIB_C
