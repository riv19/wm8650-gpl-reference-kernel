/*++
linux/drivers/mmc/host/mmc_atsmb.c

Copyright (c) 2008  WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation,
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/
 
#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>

#include <linux/mmc/host.h>
/*#include <linux/mmc/protocol.h>*/
#include <linux/mmc/card.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/mmc.h>

#include <linux/completion.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <asm/dma.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/memory.h>
#include <mach/hardware.h>
//#include <mach/gpio_if.h>

#include <asm/scatterlist.h>
#include <asm/sizes.h>

#include "mmc_atsmb.h"
#include <mach/multicard.h>
#include <mach/irqs.h>

#define mprintk  

#if 0
#define DBG(host, fmt, args...)	\
	pr_debug("%s: %s: " fmt, mmc_hostname(host->mmc), __func__ , args)
#endif
#define ATSMB_TIMEOUT_TIME (HZ*2)

//add by jay,for modules support
static u64 wmt_sdmmc_dma_mask = 0xffffffffUL;
static struct resource wmt_sdmmc_resources[] = {
	[0] = {
		.start	= 0xd800a000,
		.end	= 0xd800a3FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SDC0,
		.end	= IRQ_SDC0,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start  = IRQ_SDC0_DMA,
		.end    = IRQ_SDC0_DMA,
		.flags  = IORESOURCE_IRQ,
	},
	/*2008/10/6 RichardHsu-s*/
	 [3] = {
	     .start    = IRQ_PMC_WAKEUP,
	     .end      = IRQ_PMC_WAKEUP,
	     .flags    = IORESOURCE_IRQ,
	 },
	 /*2008/10/6 RichardHsu-e*/
};


//
static void atsmb_release(struct device * dev) {}

/*#ifdef CONFIG_MMC_DEBUG*/
#define MORE_INFO
#if 0
#define DBG(x...)	printk(KERN_ALERT x)
#define DBGR(x...)	printk(KERN_ALERT x)
#else
#define DBG(x...)	do { } while (0)
#define DBGR(x...)	do { } while (0)
#endif

#if 1
void atsmb_dump_reg(struct atsmb_host *host)
{
	DBGR("\n+---------------------------Registers----------------------------+\n");
#ifdef MORE_INFO
	DBGR("%16s = 0x%8x  |", "CTL", *ATSMB_CTL);
	DBGR("%16s = 0x%8x\n", "CMD_IDX", *ATSMB_CMD_IDX);

	DBGR("%16s = 0x%8x  |", "RSP_TYPE", *ATSMB_RSP_TYPE);
	DBGR("%16s = 0x%8x\n", "CMD_ARG", *ATSMB_CMD_ARG);

	DBGR("%16s = 0x%8x  |", "BUS_MODE", *ATSMB_BUS_MODE);
	DBGR("%16s = 0x%8x\n", "BLK_LEN", *ATSMB_BLK_LEN);

	DBGR("%16s = 0x%8x  |", "BLK_CNT", *ATSMB_BLK_CNT);
	DBGR("%16s = 0x%8x\n", "RSP_0", *ATSMB_RSP_0);

	DBGR("%16s = 0x%8x  |", "RSP_1", *ATSMB_RSP_1);
	DBGR("%16s = 0x%8x\n", "RSP_2", *ATSMB_RSP_2);

	DBGR("%16s = 0x%8x  |", "RSP_3", *ATSMB_RSP_3);
	DBGR("%16s = 0x%8x\n", "RSP_4", *ATSMB_RSP_4);

	DBGR("%16s = 0x%8x  |", "RSP_5", *ATSMB_RSP_5);
	DBGR("%16s = 0x%8x\n", "RSP_6", *ATSMB_RSP_6);

	DBGR("%16s = 0x%8x  |", "RSP_7", *ATSMB_RSP_7);
	DBGR("%16s = 0x%8x\n", "RSP_8", *ATSMB_RSP_8);

	DBGR("%16s = 0x%8x  |", "RSP_9", *ATSMB_RSP_9);
	DBGR("%16s = 0x%8x\n", "RSP_10", *ATSMB_RSP_10);

	DBGR("%16s = 0x%8x  |", "RSP_11", *ATSMB_RSP_11);
	DBGR("%16s = 0x%8x\n", "RSP_12", *ATSMB_RSP_12);

	DBGR("%16s = 0x%8x  |", "RSP_13", *ATSMB_RSP_13);
	DBGR("%16s = 0x%8x\n", "RSP_14", *ATSMB_RSP_14);

	DBGR("%16s = 0x%8x\n", "RSP_15", *ATSMB_RSP_15);

	DBGR("%16s = 0x%8x  |", "CURBLK_CNT", *ATSMB_CURBLK_CNT);
	DBGR("%16s = 0x%8x\n", "INT_MASK_0", *ATSMB_INT_MASK_0);

	DBGR("%16s = 0x%8x  |", "INT_MASK_1", *ATSMB_INT_MASK_1);
#endif

	DBGR("%16s = 0x%8x\n", "SD_STS_0", *ATSMB_SD_STS_0);

	DBGR("%16s = 0x%8x  |", "SD_STS_1", *ATSMB_SD_STS_1);
	DBGR("%16s = 0x%8x\n", "SD_STS_2", *ATSMB_SD_STS_2);
#ifdef MORE_INFO
	DBGR("%16s = 0x%8x  |", "SD_STS_3", *ATSMB_SD_STS_3);

	DBGR("%16s = 0x%8x\n", "RSP_TOUT", *ATSMB_RSP_TOUT);

	DBGR("%16s = 0x%8x  |", "CLK_SEL", *ATSMB_CLK_SEL);
	DBGR("%16s = 0x%8x\n", "EXT_CTL", *ATSMB_EXT_CTL);

	DBGR("%16s = 0x%8x  |", "SHDW_BLKLEN", *ATSMB_SHDW_BLKLEN);
	DBGR("%16s = 0x%8x\n", "TIMER_VAL", *ATSMB_TIMER_VAL);
#endif
	DBGR("%16s = 0x%8x  |", "PDMA_GCR", *ATSMB_PDMA_GCR);
	DBGR("%16s = 0x%8x\n", "PDMA_IER", *ATSMB_PDMA_IER);
	DBGR("%16s = 0x%8x  |", "PDMA_ISR", *ATSMB_PDMA_ISR);
	DBGR("%16s = 0x%8x\n", "PDMA_DESPR", *ATSMB_PDMA_DESPR);
	DBGR("%16s = 0x%8x  |", "PDMA_RBR", *ATSMB_PDMA_RBR);
	DBGR("%16s = 0x%8x\n", "PDMA_DAR", *ATSMB_PDMA_DAR);
	DBGR("%16s = 0x%8x  |", "PDMA_BAR", *ATSMB_PDMA_BAR);
	DBGR("%16s = 0x%8x\n", "PDMA_CPR", *ATSMB_PDMA_CPR);
	DBGR("%16s = 0x%8x  |", "PDMA_CCR", *ATSMB_PDMA_CCR);
#ifdef MORE_INFO
	DBG("%16s = 0x%8x\n", "PMC_SD_CLK", *PMC_SD_CLK);
#endif
	DBGR("\n+----------------------------------------------------------------+\n");
}
#else
void atsmb_dump_reg(struct atsmb_host *host) {}
#endif

static unsigned int fmax = 515633;
/*2008/10/6 RichardHsu-s*/
static unsigned int MMC_DRIVER_VERSION;

int SCC_ID(void){
				unsigned short val;
				val = REG16_VAL(0xd8120002);
		return val;
}

int get_chip_version(void) /*2008/05/01 janshiue modify for A1 chip*/
{
				u32 tmp;
				tmp = REG32_VAL(0xd8120000);
				tmp = ((tmp & 0xF00) >> 4) + 0x90 + ((tmp & 0xFF) - 1);
				return tmp;
}

void get_driver_version(void)
{
		if (SCC_ID() == 0x3426) {
			if (get_chip_version() < 0xA1)
				MMC_DRIVER_VERSION = MMC_DRV_3426_A0;
			 else if (get_chip_version() == 0xA1)
				MMC_DRIVER_VERSION = MMC_DRV_3426_A1;
			 else
				MMC_DRIVER_VERSION = MMC_DRV_3426_A2;
		} else if(SCC_ID() == 0x3437) {
			if (get_chip_version() < 0xA1)
				MMC_DRIVER_VERSION = MMC_DRV_3437_A0;
			else
				MMC_DRIVER_VERSION = MMC_DRV_3437_A1;
		} else if(SCC_ID() == 0x3429) {
            MMC_DRIVER_VERSION = MMC_DRV_3429;
		} else if(SCC_ID() == 0x3451) {
			if (get_chip_version() < 0xA1)
				MMC_DRIVER_VERSION = MMC_DRV_3451_A0;
        } else if(SCC_ID() == 0x3465) {
            MMC_DRIVER_VERSION = MMC_DRV_3465;
        }
}
/*2008/10/6 RichardHsu-e*/

/**********************************************************************
Name  	    : atsmb_alloc_desc
Function    : To config PDMA descriptor setting.
Calls       :
Called by   :
Parameter   :
Author 	    : Janshiue Wu
History	    :
***********************************************************************/
static inline int atsmb_alloc_desc(struct atsmb_host *host,
									unsigned int	bytes)
{
	void	*DescPool = NULL;
	DBG("[%s] s\n",__func__);

	DescPool = dma_alloc_coherent(host->mmc->parent, bytes, &(host->DescPhyAddr), GFP_KERNEL);
	if (!DescPool) {
		DBG("can't allocal desc pool=%8X %8X\n", DescPool, host->DescPhyAddr);
		DBG("[%s] e1\n",__func__);
		return -1;
	}
	DBG("allocal desc pool=%8X %8X\n", DescPool, host->DescPhyAddr);
	host->DescVirAddr = (unsigned long *)DescPool;
	host->DescSize = bytes;
	DBG("[%s] e2\n",__func__);
	return 0;
}

/**********************************************************************
Name  	    : atsmb_config_desc
Function    : To config PDMA descriptor setting.
Calls       :
Called by   :
Parameter   :
Author 	    : Janshiue Wu
History	    :
***********************************************************************/
static inline void atsmb_config_desc(unsigned long *DescAddr,
									unsigned long *BufferAddr,
									unsigned long Blk_Cnt)
{

	int i = 0 ;
	unsigned long *CurDes = DescAddr;
	DBG("[%s] s\n",__func__);

	/*	the first Descriptor store for 1 Block data
	 *	(512 bytes)
	 */
	for (i = 0 ; i < 3 ; i++) {
		atsmb_init_short_desc(CurDes, 0x80, BufferAddr, 0);
		BufferAddr += 0x20;
		CurDes += sizeof(struct SD_PDMA_DESC_S)/4;
	}
	if (Blk_Cnt > 1) {
		atsmb_init_long_desc(CurDes, 0x80, BufferAddr, CurDes + (sizeof(struct SD_PDMA_DESC_L)/4), 0);
		BufferAddr += 0x20;
		CurDes +=  sizeof(struct SD_PDMA_DESC_L)/4;
		/*	the following Descriptor store the rest Blocks data
		 *	(Blk_Cnt - 1) * 512 bytes
		 */
		atsmb_init_long_desc(CurDes, (Blk_Cnt - 1) * 512,
			BufferAddr, CurDes + (sizeof(struct SD_PDMA_DESC_L)/4), 1);
	} else {
		atsmb_init_long_desc(CurDes, 0x80, BufferAddr, CurDes + (sizeof(struct SD_PDMA_DESC_L)/4), 1);
	}
	DBG("[%s] e\n",__func__);
}
/**********************************************************************
Name  	 : atsmb_config_dma
Function    : To set des/src address, byte count to transfer, and DMA channel settings,
			 and DMA ctrl. register.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static inline void atsmb_config_dma(unsigned long config_dir,
									unsigned long dma_mask,
									struct atsmb_host *host)
{
    DBG("[%s] s\n",__func__);
    /* Enable DMA */
	*ATSMB_PDMA_GCR = SD_PDMA_GCR_DMA_EN;
	*ATSMB_PDMA_GCR = SD_PDMA_GCR_SOFTRESET;
	*ATSMB_PDMA_GCR = SD_PDMA_GCR_DMA_EN;
	/*open interrupt*/
	*ATSMB_PDMA_IER = SD_PDMA_IER_INT_EN;
	/*Make sure host could co-work with DMA*/
	*ATSMB_SD_STS_2 |= ATSMB_CLK_FREEZ_EN;

	/*Set timer timeout value*/
	/*If clock is 390KHz*/
	if (host->current_clock < 400000)
		*ATSMB_TIMER_VAL = 0x200;		/*1024*512*(1/390K) second*/
	else
		*ATSMB_TIMER_VAL = 0x1fff;		/*why not to be 0xffff?*/
	
	/*clear all DMA INT status for safety*/
	*ATSMB_PDMA_ISR |= SD_PDMA_IER_INT_STS;

	/* hook desc */
	*ATSMB_PDMA_DESPR = host->DescPhyAddr;
	if (config_dir == DMA_CFG_WRITE)
		*ATSMB_PDMA_CCR &= SD_PDMA_CCR_IF_to_peripheral;
	else
		*ATSMB_PDMA_CCR |= SD_PDMA_CCR_peripheral_to_IF;

	host->DmaIntMask = dma_mask; /*save success event*/

	*ATSMB_PDMA_CCR |= SD_PDMA_CCR_RUN;
	DBG("[%s] e\n",__func__);
}

/**********************************************************************
Name  	 : atsmb_prep_cmd
Function    :
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static inline void atsmb_prep_cmd(struct atsmb_host *host,
									u32 opcode,
									u32 arg,
									unsigned int flags,
									u16  blk_len,
									u16  blk_cnt,
									unsigned char int_maks_0,
									unsigned char int_mask_1,
									unsigned char cmd_type,
									unsigned char op)
{
	DBG("[%s] s\n",__func__);
	
	/*set cmd operation code and arguments.*/
	host->opcode = opcode;
	*ATSMB_CMD_IDX = opcode; /* host->opcode is set for further use in ISR.*/
	*ATSMB_CMD_ARG = arg;

#if 0   /* Fixme to support SPI mode, James Tian*/
    if ((flags && MMC_RSP_NONE) == MMC_RSP_NONE)
        *ATSMB_RSP_TYPE = ATMSB_TYPE_R0;
    else if ((flags && MMC_RSP_R1) == MMC_RSP_R1)
        *ATSMB_RSP_TYPE = ATMSB_TYPE_R1;
    else if ((flags && MMC_RSP_R1B) == MMC_RSP_R1B)
        *ATSMB_RSP_TYPE = ATMSB_TYPE_R1b;
    else if ((flags && MMC_RSP_R2) == MMC_RSP_R2)
        *ATSMB_RSP_TYPE = ATMSB_TYPE_R2;
    else if ((flags && MMC_RSP_R3) == MMC_RSP_R3)
        *ATSMB_RSP_TYPE = ATMSB_TYPE_R3;
    else if ((flags && MMC_RSP_R6) == MMC_RSP_R6)
        *ATSMB_RSP_TYPE = ((opcode != SD_SEND_IF_COND) ? ATMSB_TYPE_R6 : ATMSB_TYPE_R7);
    else
		*ATSMB_RSP_TYPE = ATMSB_TYPE_R0;
#endif

#if 1
	/*set cmd response type*/
	switch (flags) {
	case MMC_RSP_NONE | MMC_CMD_AC:
	case MMC_RSP_NONE | MMC_CMD_BC:
		*ATSMB_RSP_TYPE = ATMSB_TYPE_R0;
		break;
	case MMC_RSP_R1 | MMC_CMD_ADTC:
	case MMC_RSP_R1 | MMC_CMD_AC:
		*ATSMB_RSP_TYPE = ATMSB_TYPE_R1;
		break;
	case MMC_RSP_R1B | MMC_CMD_AC:
		*ATSMB_RSP_TYPE = ATMSB_TYPE_R1b;
		break;
	case MMC_RSP_R2 | MMC_CMD_BCR:
	case MMC_RSP_R2 | MMC_CMD_AC:
		*ATSMB_RSP_TYPE = ATMSB_TYPE_R2;
		break;
	case MMC_RSP_R3 | MMC_CMD_BCR:
		*ATSMB_RSP_TYPE = ATMSB_TYPE_R3;
		break;
	case MMC_RSP_R6 | MMC_CMD_BCR: /*MMC_RSP_R6 = MMC_RSP_R7.*/
		*ATSMB_RSP_TYPE = ((opcode != SD_SEND_IF_COND) ?
			ATMSB_TYPE_R6 : ATMSB_TYPE_R7);
		break;
	default:
		*ATSMB_RSP_TYPE = ATMSB_TYPE_R0;
		break;
	}
#endif

	/*reset Response FIFO*/
	*ATSMB_CTL |= 0x08;

	/* SD Host enable Clock */
    *ATSMB_BUS_MODE |= ATSMB_CST;    

	/*Set Cmd-Rsp Timeout to be maximum value.*/
	*ATSMB_RSP_TOUT = 0xFE;

	/*clear all status registers for safety*/
	*ATSMB_SD_STS_0 |= 0xff;
	*ATSMB_SD_STS_1 |= 0xff;
	*ATSMB_SD_STS_2 |= 0xff;
	//*ATSMB_SD_STS_2 |= 0x7f;
	*ATSMB_SD_STS_3 |= 0xff;

	//set block length and block count for cmd requesting data
	*ATSMB_BLK_LEN &=~(0x07ff);
	*ATSMB_BLK_LEN |= blk_len;
	//*ATSMB_SHDW_BLKLEN = blk_len;
	*ATSMB_BLK_CNT = blk_cnt;


	*ATSMB_INT_MASK_0 |= int_maks_0;
	*ATSMB_INT_MASK_1 |= int_mask_1;

	//Set Auto stop for Multi-block access
	if(cmd_type == 3 || cmd_type == 4)
	{
		//auto stop command set.
		*ATSMB_EXT_CTL |= 0x01;

/*
 * Enable transaction abort.
 * When CRC error occurs, CMD 12 would be automatically issued.
 * That is why we cannot enable R/W CRC error INTs.
 * If we enable CRC error INT, we would handle this INT in ISR and then issue CMD 12 via software.
 */
		*ATSMB_BLK_LEN |= 0x0800;
	}

	/*Set read or write*/
	if (op == 1)
		*ATSMB_CTL &=  ~(0x04);
	else if (op == 2)
		*ATSMB_CTL |=  0x04;

	/*for Non data access command, command type is 0.*/
	*ATSMB_CTL &= 0x0F;
	*ATSMB_CTL |= (cmd_type<<4);
	DBG("[%s] e\n",__func__);
}

static inline void atsmb_issue_cmd(void)
{
	*ATSMB_CTL |= ATSMB_START;
}
/**********************************************************************
Name  	 : atsmb_request_end
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static void
atsmb_request_end(struct atsmb_host *host, struct mmc_request *mrq)
{
	DBG("[%s] s\n",__func__);
	/*
	 * Need to drop the host lock here; mmc_request_done may call
	 * back into the driver...
	 */
	spin_unlock(&host->lock);
	/*DBG("100");*/
	mmc_request_done(host->mmc, mrq);
	/*DBG("101\n");*/
	spin_lock(&host->lock);
	DBG("[%s] e\n",__func__);
}

/**********************************************************************
Name  	 : atsmb_data_done
Function    :
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
void atsmb_wait_done(void *data)
{
	struct atsmb_host *host = (struct atsmb_host *) data;
	DBG("[%s] s\n",__func__);

	WARN_ON(host->done_data == NULL);
	complete(host->done_data);
	host->done_data = NULL;
	host->done = NULL;
	DBG("[%s] e\n",__func__);
}

/**********************************************************************
Name  	 : atsmb_start_data
Function    : If we start data, there must be only four cases.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static void atsmb_start_data(struct atsmb_host *host)
{
	DECLARE_COMPLETION(complete);
	unsigned char cmd_type = 0;
	unsigned char op = 0;	/*0: non-operation; 1:read; 2: write*/
	unsigned char mask_0 = 0;
	unsigned char mask_1 = 0;
	unsigned long dma_mask = 0;

	struct mmc_data *data = host->data;
	struct mmc_command *cmd = host->cmd;

	struct scatterlist *sg = NULL;
	unsigned int sg_len = 0;

	unsigned int total_blks = 0;		/*total block number to transfer*/
	u32 card_addr = 0;
	unsigned long dma_len = 0;
	unsigned long total_dma_len = 0;
	dma_addr_t dma_phy = 0;			/* physical address used for DMA*/
	unsigned int dma_times = 0;  /*times dma need to transfer*/
	unsigned int dma_loop = 0;
	unsigned int sg_num = 0;
	int loop_cnt = 10000;
	unsigned int sg_transfer_len = 0; /*record each time dma transfer sg length */

	DBG("[%s] s\n",__func__);
	data->bytes_xfered = 0;
	cmd->error = 0;
	data->error = 0;

	/*for loop*/
	sg = data->sg;
	sg_len = data->sg_len;

	dma_times = (sg_len / MAX_DESC_NUM);
	if (sg_len % MAX_DESC_NUM)
		dma_times++;
	DBG("dma_times = %d  sg_len = %d sg = %x\n", dma_times, sg_len, sg);
	card_addr = cmd->arg; /*may be it is block-addressed, or byte-addressed.*/
	total_blks = data->blocks;
	dma_map_sg(&(host->mmc->class_dev), sg, sg_len,
				((data->flags)&MMC_DATA_READ) ? DMA_FROM_DEVICE : DMA_TO_DEVICE);


	for (dma_loop = 1 ; dma_loop <= dma_times; dma_loop++, sg_len -= sg_transfer_len) {
		DBG("dma_loop = %d  sg_len = %d  sg_transfer_len = %d\n", dma_loop, sg_len, sg_transfer_len);
		if (dma_loop < dma_times)
			sg_transfer_len = MAX_DESC_NUM;
		else
			sg_transfer_len = sg_len;
		DBG("sg_transfer_len = %d\n", sg_transfer_len);
		/*
		*Firstly, check and wait till card is in the transfer state.
		*For our hardware, we can not consider
		*the card has successfully tranfered its state from data/rcv to trans,
		*when auto stop INT occurs.
		*/
		loop_cnt = 10000;
		do {
			loop_cnt--;
			WARN_ON(loop_cnt == 0);
			host->done_data = &complete;
			host->done = &atsmb_wait_done;
			host->soft_timeout = 1;
			atsmb_prep_cmd(host,
							MMC_SEND_STATUS,
							(host->mmc->card->rca)<<16,
							MMC_RSP_R1 | MMC_CMD_AC,
							0,
							0,
							0,		/*mask_0*/
							ATSMB_RSP_DONE_EN
							|ATSMB_RSP_CRC_ERR_EN
							|ATSMB_RSP_TIMEOUT_EN,
							0,		/*cmd type*/
							0);		/*read or write*/
			atsmb_issue_cmd();
			DBG("%16s = 0x%8x  |", "INT_MASK_1", *ATSMB_INT_MASK_1);
			DBG("%16s = 0x%8x \n", "SD_STS_1", *ATSMB_SD_STS_1);
			/*ISR would completes it.*/
			wait_for_completion_timeout(&complete, ATSMB_TIMEOUT_TIME);

			WARN_ON(host->soft_timeout == 1);
			if (host->soft_timeout == 1) {
				DBG("%s soft_timeout.\n", __func__);
				atsmb_dump_reg(host);
			}
			if (cmd->error != MMC_ERR_NONE) {
				cmd->error = MMC_ERR_FAILED;
				DBG("Getting Status failed.\n");
				goto end;
			}
		} while ((cmd->resp[0] & 0x1f00) != 0x900 && loop_cnt > 0); /*wait for trans state.*/

		/*
		* Now, we can safely issue read/write command.
		* We can not consider this request as multi-block acess or single one via opcode,
		* as request is splitted into sgs.
		* Some sgs may be single one, some sgs may be multi one.
		*/

		dma_phy = sg_dma_address(sg);
		dma_len = sg_dma_len(sg);
		DBG("dma_len = %d data->blksz = %d sg_len = %d\n", dma_len, data->blksz, sg_len);
		if ((dma_len / (data->blksz)) == 1 && (sg_len == 1)) {
			if (data->flags&MMC_DATA_READ) {/* read operation*/
				host->opcode = MMC_READ_SINGLE_BLOCK;
				cmd_type = 2;
				op = 1;
				mask_0  = 0;	/*BLOCK_XFER_DONE INT skipped, we use DMA TC INT*/
				mask_1 = (ATSMB_READ_CRC_ERR_EN
							|ATSMB_DATA_TIMEOUT_EN
							/*Data Timeout and CRC error */
							|ATSMB_RSP_CRC_ERR_EN
							|ATSMB_RSP_TIMEOUT_EN);
							/*Resp Timeout and CRC error */
				dma_mask = SD_PDMA_CCR_Evt_success;
			} else {/*write operation*/
				host->opcode = MMC_WRITE_BLOCK;
				cmd_type = 1;
				op = 2;
				/*====That is what we want===== DMA TC INT skipped*/
				mask_0  = ATSMB_BLOCK_XFER_DONE_EN;
				mask_1 = (ATSMB_WRITE_CRC_ERR_EN
							|ATSMB_DATA_TIMEOUT_EN
							/*Data Timeout and CRC error */
							|ATSMB_RSP_CRC_ERR_EN
							|ATSMB_RSP_TIMEOUT_EN);
							/*Resp Timeout and CRC error */
				dma_mask = 0;
			}
		} else {  /*more than one*/
			if (data->flags&MMC_DATA_READ) {/* read operation*/
				host->opcode = MMC_READ_MULTIPLE_BLOCK;
				cmd_type = 4;
				op = 1;
				mask_0  = 0;	/*MULTI_XFER_DONE_EN skipped*/
				mask_1 = (ATSMB_AUTO_STOP_EN	/*====That is what we want=====*/
							|ATSMB_DATA_TIMEOUT_EN
							/*Data Timeout and CRC error */
							|ATSMB_RSP_CRC_ERR_EN
							|ATSMB_RSP_TIMEOUT_EN);
							/*Resp Timeout and CRC error */
				dma_mask = 0;
			} else {/*write operation*/
				host->opcode = MMC_WRITE_MULTIPLE_BLOCK;
				cmd_type = 3;
				op = 2;
				mask_0  = 0;	/*MULTI_XFER_DONE INT skipped*/
				mask_1 = (ATSMB_AUTO_STOP_EN		/*====That is what we want=====*/
							|ATSMB_DATA_TIMEOUT_EN
							/*Data Timeout and CRC error */
							|ATSMB_RSP_CRC_ERR_EN
							|ATSMB_RSP_TIMEOUT_EN);
							/*Resp Timeout and CRC error */
				dma_mask = 0;
			}
		}
		/*To controller every sg done*/
		host->done_data = &complete;
		host->done = &atsmb_wait_done;
		/*sleep till ISR wakes us*/
		host->soft_timeout = 1;	/*If INT comes early than software timer, it would be cleared.*/

		total_dma_len = 0;
		DBG("host->DescVirAddr = %x host->DescSize=%x\n", host->DescVirAddr, host->DescSize);
		memset(host->DescVirAddr, 0, host->DescSize);
		for (sg_num = 0 ; sg_num < sg_transfer_len ; sg++, sg_num++) {

			/*
			 * Now, we can safely issue read/write command.
			 * We can not consider this request as multi-block acess or single one via opcode,
			 * as request is splitted into sgs.
			 * Some sgs may be single one, some sgs may be multi one.
			 */

			dma_phy = sg_dma_address(sg);
			dma_len = sg_dma_len(sg);
			total_dma_len = total_dma_len + dma_len;
			DBG("sg_num=%d sg_transfer_len=%d sg=%x sg_len=%d total_dma_len=%d dma_len=%d\n",
				sg_num, sg_transfer_len, sg, sg_len, total_dma_len, dma_len);
			/*2009/01/15 janshiue add*/
			if (sg_num < sg_transfer_len - 1) {
				/* means the last descporitor */
				atsmb_init_short_desc(
				host->DescVirAddr + (sg_num * sizeof(struct SD_PDMA_DESC_S)/4),
				dma_len, (unsigned long *)dma_phy, 0);
			} else {
				atsmb_init_short_desc(
				host->DescVirAddr + (sg_num * sizeof(struct SD_PDMA_DESC_S)/4),
				dma_len, (unsigned long *)dma_phy, 1);
			}
		/*2009/01/15 janshiue add*/

		}
		/*operate our hardware*/
		atsmb_prep_cmd(host,
						host->opcode,
			/*arg, may be byte addressed, may be block addressed.*/
						card_addr,
						cmd->flags,
						data->blksz - 1,	/* in fact, it is useless.*/
			/* for single one, it is useless. but for multi one, */
			/* it would be used to tell auto stop function whether it is done.*/
						total_dma_len/(data->blksz),
						mask_0,
						mask_1,
						cmd_type,
						op);

		atsmb_config_dma((op == 1) ? DMA_CFG_READ : DMA_CFG_WRITE,
						 dma_mask,
						 host);

		atsmb_issue_cmd();
		wait_for_completion_timeout(&complete,
			ATSMB_TIMEOUT_TIME*sg_transfer_len); /*ISR would completes it.*/

		WARN_ON(host->soft_timeout == 1);
		if (host->soft_timeout == 1)
			atsmb_dump_reg(host);

		/*check everything goes okay or not*/
		if (cmd->error != MMC_ERR_NONE
			&& data->error != MMC_ERR_NONE) {
			DBG("CMD or Data failed error=%X DescVirAddr=%8X dma_phy=%8X dma_mask = %x\n",
				cmd->error, host->DescVirAddr, dma_phy, host->DmaIntMask);
			goto end;
		}
//		card_addr += total_dma_len>>(mmc_card_blockaddr(host->mmc->card_selected) ? 9 : 0);
		card_addr += total_dma_len>>(mmc_card_blockaddr(host->mmc->card) ? 9 : 0);	//zhf: modified by James Tian
		data->bytes_xfered += total_dma_len;


	}

	WARN_ON(total_blks != data->bytes_xfered >> 9);
	host->opcode = 0;
end:
	spin_lock(&host->lock);
	atsmb_request_end(host, host->mrq);
	spin_unlock(&host->lock);
	DBG("[%s] e\n",__func__);
}


/**********************************************************************
Name  	 : atsmb_cmd_with_data_back
Function    :
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static void  atsmb_cmd_with_data_back(struct atsmb_host *host)
{

	struct scatterlist *sg = NULL;
	unsigned int sg_len = 0;
	DBG("[%s] s\n",__func__);
	/*for loop*/
	sg = host->data->sg;
	sg_len = host->data->sg_len;
	dma_map_sg(&(host->mmc->class_dev), sg, sg_len, DMA_FROM_DEVICE);

	/*2009/01/15 janshiue add*/
	memset(host->DescVirAddr, 0, host->DescSize);
	atsmb_init_long_desc(host->DescVirAddr, sg_dma_len(sg), (unsigned long *)sg_dma_address(sg), 0, 1);
	/*2009/01/15 janshiue add*/
	/*prepare for cmd*/
	atsmb_prep_cmd(host,					/*host*/
					host->cmd->opcode, 	/*opcode*/
					host->cmd->arg,		/*arg*/
					host->cmd->flags,		/*flags*/
					sg_dma_len(sg)-1,		/*block length*/
					0,		/*block size: It looks like useless*/
					0,		/*int_mask_0*/
					(ATSMB_RSP_CRC_ERR_EN|ATSMB_RSP_TIMEOUT_EN
					|ATSMB_READ_CRC_ERR_EN |ATSMB_DATA_TIMEOUT_EN), /*int_mask_1*/
					2, 					/*cmd_type*/
					1);					/*op*/

	atsmb_config_dma(DMA_CFG_READ,
					 SD_PDMA_CCR_Evt_success,
					 host);
	atsmb_issue_cmd();
	DBG("[%s] e\n",__func__);
}
/**********************************************************************
Name  	 : atsmb_start_cmd
Function    :
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static void atsmb_start_cmd(struct atsmb_host *host)
{
	DBG("[%s] s\n",__func__);
	atsmb_prep_cmd(host,
					host->cmd->opcode,
					host->cmd->arg,
					host->cmd->flags,
					0,		/*useless*/
					0,		/*useless*/
					0,		/*mask_0*/
					ATSMB_RSP_DONE_EN|ATSMB_RSP_CRC_ERR_EN|ATSMB_RSP_TIMEOUT_EN,
					0,		/*cmd type*/
					0);		/*read or write*/
	atsmb_issue_cmd();
	DBG("[%s] e\n",__func__);
}
/**********************************************************************
Name  	 : atsmb_fmt_check_rsp
Function    :
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static inline void atsmb_fmt_check_rsp(struct atsmb_host *host)
{

	u8 tmp_resp[4] = {0};
	int i, j, k;
	DBG("[%s] s\n",__func__);
	if (host->cmd->flags != (MMC_RSP_R2 | MMC_CMD_AC)
		&& host->cmd->flags != (MMC_RSP_R2 | MMC_CMD_BCR)) {
		for (j = 0, k = 1; j < 4; j++, k++)
			tmp_resp[j] = *(REG8_PTR(_RSP_0 + MMC_ATSMB_BASE+k));

		host->cmd->resp[0] = (tmp_resp[0] << 24) |
			(tmp_resp[1]<<16) | (tmp_resp[2]<<8) | (tmp_resp[3]);
	} else {
		for (i = 0, k = 1; i < 4; i++) { /*R2 has 4 u32 response.*/
			for (j = 0; j < 4; j++) {
				if (k < 16)
					tmp_resp[j] = *(REG8_PTR(_RSP_0 + MMC_ATSMB_BASE+k));
				else /* k =16*/
					tmp_resp[j] = *(ATSMB_RSP_0);
					k++;
			 }
			host->cmd->resp[i] = (tmp_resp[0]<<24) | (tmp_resp[1]<<16) |
				(tmp_resp[2]<<8) | (tmp_resp[3]);
		}
	}

	/*
	* For the situation that we need response,
	* but response registers give us all zeros, we consider this operation timeout.
	*/
	if (host->cmd->flags != (MMC_RSP_NONE | MMC_CMD_AC)
		&& host->cmd->flags != (MMC_RSP_NONE | MMC_CMD_BC)) {
		if (host->cmd->resp[0] == 0 && host->cmd->resp[1] == 0
			&& host->cmd->resp[2] == 0 && host->cmd->resp[3] == 0)
//			host->cmd->error = MMC_ERR_TIMEOUT;
			host->cmd->error = -ETIMEDOUT;	//zhf: modified by James Tian
	}
	DBG("[%s] e\n",__func__);
}
/**********************************************************************
Name  	 : atsmb_get_slot_status
Function    : Our host only supports one slot.
Calls		:
Called by	:
Parameter :
returns	: 1: in slot; 0: not in slot.
Author 	 : Leo Lee
History	:
***********************************************************************/
static int atsmb_get_slot_status(struct mmc_host *mmc)
{
//	struct atsmb_host *host = mmc_priv(mmc);
	unsigned char status_0 = 0;
//	unsigned long flags;
	unsigned long ret = 0;
	DBG("[%s] s\n",__func__);
//	spin_lock_irqsave(&host->lock, flags); // Vincent Li mark out for CONFIG_PREEMPT_RT
	status_0 = *ATSMB_SD_STS_0;
//	spin_unlock_irqrestore(&host->lock, flags); // Vincent Li mark out for CONFIG_PREEMPT_RT
	/* after WM3400 A1 ATSMB_CARD_IN_SLOT_GPI = 1 means not in slot*/
	if (MMC_DRIVER_VERSION >= MMC_DRV_3426_A0) {
		ret = ((status_0 & ATSMB_CARD_NOT_IN_SLOT_GPI) ? 0 : 1);
		DBG("[%s] e1\n",__func__);
		return ret;
	} else {
		ret = ((status_0 & ATSMB_CARD_NOT_IN_SLOT_GPI) ? 1 : 0);
		DBG("[%s] e2\n",__func__);
		return ret;
	}
	DBG("[%s] e3\n",__func__);	
		return 0;
}

/**********************************************************************
Name        : atsmb_init_short_desc
Function    :
Calls       :
Called by   :
Parameter   :
Author      : Janshiue Wu
History     :
***********************************************************************/
int atsmb_init_short_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr, int End)
{
	struct SD_PDMA_DESC_S *CurDes_S;
	DBG("[%s] s\n",__func__);
	CurDes_S = (struct SD_PDMA_DESC_S *) DescAddr;
	CurDes_S->ReqCount = ReqCount;
	CurDes_S->i = 0;
	CurDes_S->format = 0;
	CurDes_S->DataBufferAddr = BufferAddr;
	if (End) {
		CurDes_S->end = 1;
		CurDes_S->i = 1;
	}
	DBG("[%s] e\n",__func__);
	return 0;
}

/**********************************************************************
Name        : atsmb_init_long_desc
Function    :
Calls       :
Called by   :
Parameter   :
Author      : Janshiue Wu
History     :
***********************************************************************/
int atsmb_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount,
	unsigned long *BufferAddr, unsigned long *BranchAddr, int End)
{
	struct SD_PDMA_DESC_L *CurDes_L;
	DBG("[%s] s\n",__func__);
	CurDes_L = (struct SD_PDMA_DESC_L *) DescAddr;
	CurDes_L->ReqCount = ReqCount;
	CurDes_L->i = 0;
	CurDes_L->format = 1;
	CurDes_L->DataBufferAddr = BufferAddr;
	CurDes_L->BranchAddr = BranchAddr;
	if (End) {
		CurDes_L->end = 1;
		CurDes_L->i = 1;
	}
	DBG("[%s] e\n",__func__);
	return 0;
}

/**********************************************************************
Name  	 : atsmb_dma_isr
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static irqreturn_t atsmb_dma_isr(int irq, void *dev_id)
{

	struct atsmb_host *host = dev_id;
	u8 status_0, status_1, status_2, status_3;
	u32 pdma_event_code = 0;
	DBG("[%s] s\n",__func__);

	spin_lock(&host->lock);
	/*Get INT status*/
	status_0 = *ATSMB_SD_STS_0;
	status_1 = *ATSMB_SD_STS_1;
	status_2 = *ATSMB_SD_STS_2;
	status_3 = *ATSMB_SD_STS_3;
	/* after WM3426 A0 using PDMA */
	if (MMC_DRIVER_VERSION >= MMC_DRV_3426_A0) {

		pdma_event_code  = *ATSMB_PDMA_CCR & SD_PDMA_CCR_EvtCode;

		/* clear INT status to notify HW clear EventCode*/
		*ATSMB_PDMA_ISR |= SD_PDMA_IER_INT_STS;

		/*printk("dma_isr event code = %X\n", *ATSMB_PDMA_CCR);*/
		/*We expect cmd with data sending back or read single block cmd run here.*/
		if (pdma_event_code == SD_PDMA_CCR_Evt_success) {
			/*means need to update the data->error and cmd->error*/
			if (host->DmaIntMask == SD_PDMA_CCR_Evt_success) {
				host->data->error = MMC_ERR_NONE;
				host->cmd->error = MMC_ERR_NONE;
			}
			/*else do nothing*/
			DBG("dma_isr PDMA OK\n");
			/*atsmb_dump_reg(host);*/
		}
		/*But unluckily, we should also be prepare for all kinds of error situation.*/
		else {
			host->data->error = MMC_ERR_FAILED;
			host->cmd->error = MMC_ERR_FAILED;
			DBGR("** dma_isr PDMA fail**  event code = %X\n", *ATSMB_PDMA_CCR);
			atsmb_dump_reg(host);
		}

		if (host->DmaIntMask == SD_PDMA_CCR_Evt_success)
		    atsmb_fmt_check_rsp(host);

		/*disable INT*/
		*ATSMB_PDMA_IER &= ~SD_PDMA_IER_INT_EN;
	}

	/*wake up some one who is sleeping.*/
	if ((pdma_event_code != SD_PDMA_CCR_Evt_success) || (host->DmaIntMask == SD_PDMA_CCR_Evt_success)) {
		if (host->done_data) {/* We only use done_data when requesting data.*/
			host->soft_timeout = 0;
			host->done(host);
		} else
			atsmb_request_end(host, host->mrq); /*for cmd with data sending back.*/
	}
	spin_unlock(&host->lock);
	DBG("[%s] e\n",__func__);
	return IRQ_HANDLED;
}

/**********************************************************************
Name  	 : atsmb_regular_isr
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
//static irqreturn_t atsmb_regular_isr(int irq, void *dev_id, struct pt_regs *regs)
irqreturn_t atsmb_regular_isr(int irq, void *dev_id)
{

	struct atsmb_host *host = dev_id;
	u8 status_0, status_1, status_2, status_3;
	u32 pdma_sts;
	
	DBG("[%s] s\n",__func__);
	WARN_ON(host == NULL);

	disable_irq_nosync(irq);
	spin_lock(&host->lock);

	/*Get INT status*/
	status_0 = *ATSMB_SD_STS_0;
	/*******************************************************
						card insert interrupt
	********************************************************/
	if ((status_0 & ATSMB_DEVICE_INSERT)      /*Status Set and IRQ enabled*/
	/*To aviod the situation that we intentionally disable IRQ to do rescan.*/
		&& (*ATSMB_INT_MASK_0 & 0x80)) {

		if (host->mmc->ops->get_slot_status(host->mmc)) {
			host->mmc->scan_retry = 3;
			host->mmc->card_scan_status = false;
		} else {
			host->mmc->scan_retry = 0;
			host->mmc->card_scan_status = false;
		}

		mmc_detect_change(host->mmc, 0);
		/*Taipei Side Request: Disable INSERT IRQ when doing rescan.*/
		//*ATSMB_INT_MASK_0 &= (~0x80);/* or 40?*/	//zhf: marked by James Tian
		/*Then we clear the INT status*/
		iowrite32(ATSMB_DEVICE_INSERT, host->base+_SD_STS_0);
		spin_unlock(&host->lock);
		enable_irq(irq);
		DBG("[%s] e1\n",__func__);
		return IRQ_HANDLED;
	}

	status_1 = *ATSMB_SD_STS_1;
	status_2 = *ATSMB_SD_STS_2;
	status_3 = *ATSMB_SD_STS_3;

	pdma_sts = *ATSMB_PDMA_CCR;
	/*******************************************************
							command interrupt.
	*******************************************************/
	/*for write single block*/
	if (host->opcode == MMC_WRITE_BLOCK) {
		if ((status_0 & ATSMB_BLOCK_XFER_DONE)
			&& (status_1 & ATSMB_RSP_DONE)) {
			host->data->error = MMC_ERR_NONE;
			host->cmd->error = MMC_ERR_NONE;
			/* okay, what we want.*/
		} else {
			host->data->error = MMC_ERR_FAILED;
			host->cmd->error = MMC_ERR_FAILED;
			atsmb_dump_reg(host);
		}
	} else if (host->opcode == MMC_WRITE_MULTIPLE_BLOCK
			|| host->opcode == MMC_READ_MULTIPLE_BLOCK) {
		if ((status_1 & (ATSMB_AUTO_STOP|ATSMB_RSP_DONE))
			&& (status_0 & ATSMB_MULTI_XFER_DONE)) {
			/*If CRC error occurs, I think this INT would not occrs.*/
			/*okay, that is what we want.*/
			host->data->error = MMC_ERR_NONE;
			host->cmd->error = MMC_ERR_NONE;
		} else {
			host->data->error = MMC_ERR_FAILED;
			host->cmd->error = MMC_ERR_FAILED;
			atsmb_dump_reg(host);

		}
	} else if (host->opcode == MMC_READ_SINGLE_BLOCK) {/* we want DMA TC, run here, must be error.*/
		host->data->error = MMC_ERR_FAILED;
		host->cmd->error = MMC_ERR_FAILED;
		atsmb_dump_reg(host);
	} else {
/* command, not request data*/
/* the command which need data sending back,*/
/* like switch_function, send_ext_csd, send_scr, send_num_wr_blocks.*/
/* NOTICE: we also send status before reading or writing data, so SEND_STATUS should be excluded.*/
		if (host->data && host->opcode != MMC_SEND_STATUS) {
			host->data->error = MMC_ERR_FAILED;
			host->cmd->error = MMC_ERR_FAILED;
			atsmb_dump_reg(host);
		} else {			/* Just command, no need data sending back.*/
			if (status_1 & ATSMB_RSP_DONE) {
				/*Firstly, check data-response is busy or not.*/
				if (host->cmd->flags == (MMC_RSP_R1B | MMC_CMD_AC)) {
					int i = 10000;
					
					while (status_2 & ATSMB_RSP_BUSY) {
						status_2 = *ATSMB_SD_STS_2;
						if (--i == 0)
							break;
						DBG(" IRQ:Status_2 = %d, busy!\n", status_2);
					}
					if (i == 0)
						printk("[MMC driver] Error :SD data-response always busy!");
				}
#if 1
/*for our host, even no card in slot, for SEND_STATUS also returns no error.*/
/*The protocol layer depends on SEND_STATUS to check whether card is in slot or not.*/
/*In fact, we can also avoid this situation by checking the response whether they are all zeros.*/
				if (!atsmb_get_slot_status(host->mmc) && host->opcode == MMC_SEND_STATUS) {
					host->cmd->retries = 0; /* No retry.*/
//					host->cmd->error = MMC_ERR_INVALID;
					host->cmd->error = -EINVAL;
				} else
#endif
				host->cmd->error = MMC_ERR_NONE;
			} else {
				if (status_1 & ATSMB_RSP_TIMEOUT)/* RSP_Timeout .*/
//					host->cmd->error = MMC_ERR_TIMEOUT;
					host->cmd->error = -ETIMEDOUT;
				else /*or RSP CRC error*/
//					host->cmd->error = MMC_ERR_BADCRC;
					host->cmd->error = -EILSEQ;
				atsmb_dump_reg(host);
			}
		}
	}
	atsmb_fmt_check_rsp(host);

	/*disable INT */
	*ATSMB_INT_MASK_0 &= ~(ATSMB_BLOCK_XFER_DONE_EN | ATSMB_MULTI_XFER_DONE_EN);
	*ATSMB_INT_MASK_1 &= ~(ATSMB_WRITE_CRC_ERR_EN|ATSMB_READ_CRC_ERR_EN|ATSMB_RSP_CRC_ERR_EN
		|ATSMB_DATA_TIMEOUT_EN|ATSMB_AUTO_STOP_EN|ATSMB_RSP_TIMEOUT_EN|ATSMB_RSP_DONE_EN);


	/*clear INT status. In fact, we will clear again before issuing new command.*/
	*ATSMB_SD_STS_0 |= status_0;
	*ATSMB_SD_STS_1 |= status_1;

	if (*ATSMB_PDMA_ISR & SD_PDMA_IER_INT_STS)
		*ATSMB_PDMA_ISR |= SD_PDMA_IER_INT_STS;

	/*wake up some one who is sleeping.*/
	if (host->done_data) { /* We only use done_data when requesting data.*/
		host->soft_timeout = 0;
		host->done(host);
	} else
		atsmb_request_end(host, host->mrq); /*for cmd without data.*/

	spin_unlock(&host->lock);
	enable_irq(irq);
	DBG("[%s] e3\n",__func__);
	return IRQ_HANDLED;
}
EXPORT_SYMBOL(atsmb_regular_isr);

/**********************************************************************
Name  	 : atsmb_get_ro
Function    :.
Calls		:
Called by	:
Parameter :
returns	: 0 : write protection is disabled. 1: write protection is enabled.
Author 	 : Leo Lee
History	:
***********************************************************************/
int atsmb_get_ro(struct mmc_host *mmc)
{
	struct atsmb_host *host = mmc_priv(mmc);
	unsigned char status_0 = 0;
	unsigned long flags;
	unsigned long ret = 0;
	DBG("[%s] s\n",__func__);
	spin_lock_irqsave(&host->lock, flags);
	status_0 = *ATSMB_SD_STS_0;
	spin_unlock_irqrestore(&host->lock, flags);
	DBG("[atsmb_get_ro]\nstatus_0 = 0x%x\n", status_0);
	ret = ((status_0 & ATSMB_WRITE_PROTECT) ? 0 : 1);
	DBG("[%s] e\n",__func__);
	return ret;
}
/**********************************************************************
Name  	 : atsmb_get_slot_status
Function    :
Calls		:
Called by	:
Parameter :
returns	: 1: in slot; 0: not in slot.
Author 	 : Leo Lee
History	:
***********************************************************************/
void atsmb_dump_host_regs(struct mmc_host *mmc)
{
	struct atsmb_host *host = mmc_priv(mmc);
	DBG("[%s] s\n",__func__);
	atsmb_dump_reg(host);
	DBG("[%s] e\n",__func__);
}
EXPORT_SYMBOL(atsmb_dump_host_regs);

/**********************************************************************
Name  	 : atsmb_request
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static void atsmb_request(struct mmc_host *mmc, struct mmc_request *mrq)
{

	struct atsmb_host *host = mmc_priv(mmc);
	DBG("[%s] s\n",__func__);
	
	/* May retry process comes here.*/
	host->mrq = mrq;
	host->data = mrq->data;
	host->cmd = mrq->cmd;
	host->done_data = NULL;
	host->done = NULL;

	/*for data request*/
	if (host->data) {
		if (host->cmd->opcode == MMC_WRITE_BLOCK
			|| host->cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK
			|| host->cmd->opcode == MMC_READ_SINGLE_BLOCK
			|| host->cmd->opcode == MMC_READ_MULTIPLE_BLOCK)
			atsmb_start_data(host);
		else
			atsmb_cmd_with_data_back(host);
	} else
		atsmb_start_cmd(host);
	DBG("[%s] e\n",__func__);
}
/**********************************************************************
Name  	 : atsmb_set_ios
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static void atsmb_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{

	struct atsmb_host *host = mmc_priv(mmc);
	unsigned long flags;
	DBG("[%s] s\n",__func__);
	spin_lock_irqsave(&host->lock, flags);

	if (ios->power_mode == MMC_POWER_OFF) {
		if (MMC_DRIVER_VERSION == MMC_DRV_3465) {
            /* stop SD output clock */
			*ATSMB_BUS_MODE &= ~(ATSMB_CST);

            /*  disable SD Card power  */			
			/*set SD0 power pin as GPO pin*/
			GPIO_CTRL_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			GPIO_OC_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			/*set internal pull up*/
			GPIO_PULL_CTRL_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			/*set internal pull enable*/
			GPIO_PULL_EN_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			/*disable SD0 power*/
			GPIO_OD_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			
                    
			/* Disable Pull up/down resister of SD Bus */
			GPIO_PULL_CTRL_GP13_XDIO_BYTE_VAL &=  ~SD0_PIN;

			/*  Config SD0 to GPIO  */
            GPIO_CTRL_GP13_XDIO_BYTE_VAL |= SD0_PIN;
			
			 /*  SD0 all pins output low  */
            GPIO_OD_GP13_XDIO_BYTE_VAL &= ~SD0_PIN;
			 
            /*  Config SD0 to GPO   */
            GPIO_OC_GP13_XDIO_BYTE_VAL |= SD0_PIN;
                  
        }

	} else if (ios->power_mode == MMC_POWER_UP) {
		if (MMC_DRIVER_VERSION == MMC_DRV_3465) {       /* 3429 power up setting */

            /*  disable SD Card power  */
            /*set SD0 power pin as GPO pin*/
			GPIO_CTRL_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			GPIO_OC_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			/*set internal pull up*/
			GPIO_PULL_CTRL_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			/*set internal pull enable*/
			GPIO_PULL_EN_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
			/*disable SD0 power*/
			GPIO_OD_GP13_XDIO_BYTE_VAL |= GPIO_SD0_POWER;
                 				

            /* do not config GPIO_SD0_CD because ISR has already run,
			 * config card detect will issue ISR storm.
			 */
			/*  Config SD to GPIO  */
			GPIO_CTRL_GP13_XDIO_BYTE_VAL |= SD0_PIN;
			/*  SD all pins output low  */
			GPIO_OD_GP13_XDIO_BYTE_VAL &= ~SD0_PIN;
			/*  Config SD to GPO   */
			GPIO_OC_GP13_XDIO_BYTE_VAL |= SD0_PIN;
			
            
            
			/* Pull up/down resister of SD Bus */
           /*Disable Clock & CMD Pull enable*/
			GPIO_PULL_EN_GP13_XDIO_BYTE_VAL &= ~(GPIO_SD0_Clock | GPIO_SD0_Command);
			
			/*Set CD ,WP ,DATA pin pull up*/
			GPIO_PULL_CTRL_GP3_STORGE_BYTE_VAL |= GPIO_SD0_CD;
			GPIO_PULL_CTRL_GP13_XDIO_BYTE_VAL |= (GPIO_SD0_Data | GPIO_SD0_WriteProtect);
			
			/*Enable CD ,WP ,DATA  internal pull*/
			GPIO_PULL_EN_GP3_STORGE_BYTE_VAL |= GPIO_SD0_CD;
			GPIO_PULL_EN_GP13_XDIO_BYTE_VAL |= (GPIO_SD0_Data | GPIO_SD0_WriteProtect);
			
            
			spin_unlock_irqrestore(&host->lock, flags);
			msleep(100);
			spin_lock_irqsave(&host->lock, flags);

			/*  enable SD Card Power  */
			GPIO_OD_GP13_XDIO_BYTE_VAL &= ~GPIO_SD0_POWER;
            

            /* enable SD output clock */
			*ATSMB_BUS_MODE |= ATSMB_CST;
			
			spin_unlock_irqrestore(&host->lock, flags);
			msleep(1);
			spin_lock_irqsave(&host->lock, flags);

			/* Config SD0 back to function  */
			GPIO_CTRL_GP13_XDIO_BYTE_VAL &= ~SD0_PIN;
			GPIO_CTRL_GP3_STORGE_BYTE_VAL 	&= ~GPIO_SD0_CD;
			
        }
	} else {
		/*nothing to do when powering on.*/
	}



    if (ios->clock == mmc->f_min) {
 		host->current_clock = auto_pll_divisor(DEV_SDMMC0, SET_DIV, 1, 390);
        DBG("[%s]ATSMB Host 400KHz\n", __func__);            
    }
	else if (ios->clock >= 50000000) {
		host->current_clock = auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 45);
        DBG("[%s]ATSMB Host 50MHz\n", __func__);
	} else if ((ios->clock >= 25000000) && (ios->clock < 50000000)) {
        host->current_clock = auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 24);
        DBG("[%s]ATSMB Host 25MHz\n", __func__);
	} else if ((ios->clock >= 20000000) && (ios->clock < 25000000)) {
        host->current_clock = auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 20);
        DBG("[%s]ATSMB Host 20MHz\n", __func__);
	} else {
        host->current_clock = auto_pll_divisor(DEV_SDMMC0, SET_DIV, 1, 390);
        DBG("[%s]ATSMB Host 390KHz\n", __func__);
	}
    
	if (ios->bus_width == MMC_BUS_WIDTH_8) {
		*ATSMB_EXT_CTL |= (0x04);
	} else if (ios->bus_width == MMC_BUS_WIDTH_4) {
		*ATSMB_BUS_MODE |= ATSMB_BUS_WIDTH_4;
		*ATSMB_EXT_CTL &= ~(0x04);
	} else {
		*ATSMB_BUS_MODE &= ~(ATSMB_BUS_WIDTH_4);
		*ATSMB_EXT_CTL &= ~(0x04);
	}
#if 0
	if (ios->timing == MMC_TIMING_SD_HS)
		*ATSMB_EXT_CTL |= 0x80; /*HIGH SPEED MODE*/
	else
		*ATSMB_EXT_CTL &= ~(0x80);
#endif
#if 0	//zhf: marked by James Tian
	if (ios->ins_en == MMC_INSERT_IRQ_EN) 
        *ATSMB_INT_MASK_0 |= (0x80);/* or 40?*/
	else
        *ATSMB_INT_MASK_0 &= (~0x80);/* or 40?*/        
#endif

	spin_unlock_irqrestore(&host->lock, flags);
	DBG("[%s] e\n",__func__);
}


static const struct mmc_host_ops atsmb_ops = {
	.request	= atsmb_request,
	.set_ios	= atsmb_set_ios,
	.get_ro	= atsmb_get_ro,
	.get_slot_status = atsmb_get_slot_status,
	.dump_host_regs = atsmb_dump_host_regs,
};

/**********************************************************************
Name  	 :atsmb_probe
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static int __init atsmb_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mmc_host *mmc_host  = NULL;
	struct atsmb_host *atsmb_host = NULL;
	struct resource *resource = NULL;
	int  irq[2] = {0};
	int ret = 0;
	DBG("[%s] s\n",__func__);
	if (!pdev) {
		ret = -EINVAL; /* Invalid argument */
		goto the_end;
	}
	
    if (MMC_DRIVER_VERSION == MMC_DRV_3465) {
		/* Pull up/down resister of SD CD */
		GPIO_PULL_CTRL_GP3_STORGE_BYTE_VAL |= GPIO_SD0_CD;
		GPIO_PULL_EN_GP3_STORGE_BYTE_VAL |= GPIO_SD0_CD;	
    }

    resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!resource) {
		ret = -ENXIO;	/* No such device or address */
		printk(KERN_ALERT "[MMC/SD driver] Getting platform resources failed!\n");
		goto the_end;
	}
	if (!request_mem_region(resource->start, SZ_1K, DRIVER_NAME)) {
		ret = -EBUSY;
		printk(KERN_ALERT "[MMC/SD driver] Request memory region failed!\n");
		goto the_end ;
	}

	irq[0] = platform_get_irq(pdev, 0);	/*get IRQ for device*/;
	irq[1] = platform_get_irq(pdev, 1);	/*get IRQ for dma*/;

    if (irq[0] == NO_IRQ || irq[1] == NO_IRQ) {
		ret = -ENXIO;/* No such device or address */
		printk(KERN_ALERT "[MMC/SD driver] Get platform IRQ failed!\n");
		goto rls_region;
	}

	/*allocate a standard msp_host structure attached with a atsmb structure*/
	mmc_host = mmc_alloc_host(sizeof(struct atsmb_host), dev);
	if (!mmc_host) {
		ret = -ENOMEM;
		printk(KERN_ALERT "[MMC/SD driver] Allocating driver's data failed!\n");
		goto rls_region;
	}
	dev_set_drvdata(dev, (void *)mmc_host); /* mmc_host is driver data for the atsmb dev.*/
	atsmb_host = mmc_priv(mmc_host);

	mmc_host->ops = &atsmb_ops;

	mmc_host->ocr_avail = MMC_VDD_32_33|MMC_VDD_33_34;

	mmc_host->f_min = 390425; /*390.425Hz = 400MHz/64/16*/
	mmc_host->f_max = 50000000; /* in fact, the max frequency is 400MHz( = 400MHz/1/1)*/
	mmc_host->caps = MMC_CAP_4_BIT_DATA | MMC_CAP_SD_HIGHSPEED
					| MMC_CAP_MMC_HIGHSPEED;	// | MMC_CAP_MULTIWRITE;/* |MMC_CAP_8_BIT_DATA;*/	//zhf: marked by James Tian

	mmc_host->max_hw_segs = 128;	/*we use software sg. so we could manage even larger number.*/
	mmc_host->max_phys_segs = 128;
	/*1MB per each request */
	/*we have a 16 bit block number register, and block length is 512 bytes.*/
	mmc_host->max_req_size = 16*512*(mmc_host->max_hw_segs);
	mmc_host->max_seg_size = 65024; /* 0x7F*512 PDMA one descriptor can transfer 64K-1 byte*/
	mmc_host->max_blk_size = 2048;	/* our block length register is 11 bits.*/
	mmc_host->max_blk_count = (mmc_host->max_req_size)/512;

	/*set the specified host -- ATSMB*/
#ifdef CONFIG_MMC_UNSAFE_RESUME
	mmc_host->card_attath_status = card_attach_status_unchange;
#endif
	atsmb_host->base = ioremap(resource->start, SZ_1K);
	if (!atsmb_host->base) {
		printk(KERN_ALERT "[MMC/SD driver] IO remap failed!\n");
		ret = -ENOMEM;
		goto fr_host;
	}

	atsmb_host->mmc = mmc_host;
	spin_lock_init(&atsmb_host->lock);
	atsmb_host->res = resource;/* for atsmb_remove*/

	/*disable all interrupt and clear status by resetting controller.*/
	*ATSMB_BUS_MODE |= ATSMB_SFTRST;
	*ATSMB_BLK_LEN &= ~(0xa000);
	*ATSMB_SD_STS_0 |= 0xff;
	*ATSMB_SD_STS_1 |= 0xff;

	/* WM3437 A0 default not output clock, after SFTRST need to enable SD clock */
	//if (MMC_DRIVER_VERSION >= MMC_DRV_3437_A0) /* including 3429 */
   	*ATSMB_BUS_MODE |= ATSMB_CST;

	atsmb_host->regular_irq = irq[0];
	atsmb_host->dma_irq = irq[1];

    ret = request_irq(atsmb_host->regular_irq,
					atsmb_regular_isr,
					IRQF_SHARED,			//SA_SHIRQ, /*SA_INTERRUPT, * that is okay?*/	//zhf: modified by James Tian, should be IRQF_SHARED?
					DRIVER_NAME,
					(void *)atsmb_host);
	if (ret) {
		printk(KERN_ALERT "[MMC/SD driver] Failed to register regular ISR!\n");
		goto unmap;
	}

	ret = request_irq(atsmb_host->dma_irq,
					atsmb_dma_isr,
					IRQF_DISABLED,	//	SA_INTERRUPT,  //zhf: modified by James Tian
					DRIVER_NAME,
					(void *)atsmb_host);
	if (ret) {
		printk(KERN_ALERT "[MMC/SD driver] Failed to register DMA ISR!\n");
		goto fr_regular_isr;
	}

	/*enable card insertion interrupt and enable DMA and its Global INT*/
	*ATSMB_BLK_LEN |= (0xa000); /* also, we enable GPIO to detect card.*/
	*ATSMB_SD_STS_0 |= 0xff;
	*ATSMB_INT_MASK_0 |= 0x80; /*or 0x40?*/

	/*allocation dma descriptor*/
	ret = atsmb_alloc_desc(atsmb_host, sizeof(struct SD_PDMA_DESC_S) * MAX_DESC_NUM);
	if (ret == -1) {
		printk(KERN_ALERT "[MMC/SD driver] Failed to allocate DMA descriptor!\n");
			goto fr_dma_isr;
	}
	printk(KERN_INFO "WMT ATSMB (AHB To SD/MMC Bus) controller registered!\n");
	mmc_add_host(mmc_host);
	/* config CardDetect pin to SD function */
    GPIO_CTRL_GP3_STORGE_BYTE_VAL 	&= ~GPIO_SD0_CD;
	DBG("[%s] e1\n",__func__);
	return 0;

fr_dma_isr:
	free_irq(atsmb_host->dma_irq, atsmb_host);
fr_regular_isr:
	free_irq(atsmb_host->regular_irq, atsmb_host);
unmap:
	iounmap(atsmb_host->base);
fr_host:
	dev_set_drvdata(dev, NULL);
	mmc_free_host(mmc_host);
rls_region:
	release_mem_region(resource->start, SZ_1K);
the_end:
	printk(KERN_ALERT "[MMC/SD driver] ATSMB0 probe Failed!\n") ;
	DBG("[%s] e2\n",__func__);
	return ret;
}
/**********************************************************************
Name  	 : atsmb_remove
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static int atsmb_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mmc_host *mmc_host = (struct mmc_host *)dev_get_drvdata(dev);
	struct atsmb_host *atsmb_host;

	DBG("[%s] s\n",__func__);
	atsmb_host = mmc_priv(mmc_host);
	if (!mmc_host || !atsmb_host) {
		printk(KERN_ALERT "[MMC/SD driver] ATSMB0 remove method failed!\n");
		DBG("[%s] e1\n",__func__);
		return -ENXIO;
	}
	mmc_remove_host(mmc_host);

	/*disable interrupt by resetting controller -- for safey*/
	*ATSMB_BUS_MODE |= ATSMB_SFTRST;
	*ATSMB_BLK_LEN &= ~(0xa000);
	*ATSMB_SD_STS_0 |= 0xff;
	*ATSMB_SD_STS_1 |= 0xff;

	(void)free_irq(atsmb_host->regular_irq, atsmb_host);
	(void)free_irq(atsmb_host->dma_irq, atsmb_host);
	(void)iounmap(atsmb_host->base);
	(void)release_mem_region(atsmb_host->res->start, SZ_1K);
	dev_set_drvdata(dev, NULL);
	/*free dma descriptor*/
	dma_free_coherent(atsmb_host->mmc->parent, atsmb_host->DescSize,
	atsmb_host->DescVirAddr, atsmb_host->DescPhyAddr);
	(void)mmc_free_host(mmc_host);/* also free atsmb_host.*/
	DBG("[%s] e2\n",__func__);
	return 0;
}

/**********************************************************************
Name  	 : atsmb_shutdown
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Tommy Huang
History	:
***********************************************************************/
static void atsmb_shutdown(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mmc_host *mmc_host = (struct mmc_host *)dev_get_drvdata(dev);

	DBG("[%s] s\n",__func__);
	/*Disable card detect interrupt*/
	*ATSMB_INT_MASK_0 &= ~0x80;
	mmc_remove_host(mmc_host);
	DBG("[%s] e\n",__func__);
	
}

/**********************************************************************
Name  	 : atsmb_suspend
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
#ifdef CONFIG_PM
static int atsmb_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct device *dev = &pdev->dev;
	struct mmc_host *mmc = (struct mmc_host *)dev_get_drvdata(dev);
	int ret = 0;
	DBG("[%s] s\n",__func__);

	if (mmc) {
		/*struct atsmb_host *host = mmc_priv(mmc);*/
		ret = mmc_suspend_host(mmc, state);
		if (ret == 0) {
			/*disable all interrupt and clear status by resetting controller. */
			*ATSMB_BUS_MODE |= ATSMB_SFTRST;
			*ATSMB_BLK_LEN &= ~(0xa000);
			*ATSMB_SD_STS_0 |= 0xff;
			*ATSMB_SD_STS_1 |= 0xff;

		}
		/*disable source clock*/
		auto_pll_divisor(DEV_SDMMC0, CLK_DISABLE, 0, 0);
#ifdef CONFIG_MMC_UNSAFE_RESUME
		/*clean SD card attatch status change*/
		PMCWS_VAL |= BIT9;
		mmc->card_attath_status = card_attach_status_unchange;
#endif
	}

	DBG("[%s] e\n",__func__);
	return ret;
}
/**********************************************************************
Name  	 : atsmb_resume
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Leo Lee
History	:
***********************************************************************/
static int atsmb_resume(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mmc_host *mmc = (struct mmc_host *)dev_get_drvdata(dev);
	int ret = 0;
	DBG("[%s] s\n",__func__);

	/*
	 * enable interrupt, DMA, etc.
	 * Supply power to slot.
	 */
	if (mmc) {
		/*enable source clock*/
		auto_pll_divisor(DEV_SDMMC0, CLK_ENABLE, 0, 0);

        udelay(1);
		/*enable card insertion interrupt and enable DMA and its Global INT*/
		*ATSMB_BUS_MODE |= ATSMB_SFTRST;
		*ATSMB_BLK_LEN |= (0xa000);
		*ATSMB_INT_MASK_0 |= 0x80;	/* or 40?*/
#ifdef CONFIG_MMC_UNSAFE_RESUME
		/*modify SD card attatch status change*/
		if ((PMCWS_VAL & BIT9) && !mmc->bus_dead) {
			/*card change when suspend mode*/
			mmc->card_attath_status = card_attach_status_change;
			/*clean SD card attatch status change*/
			PMCWS_VAL |= BIT9;
		}
#endif
		ret = mmc_resume_host(mmc);
	}

	DBG("[%s] e\n",__func__);
	return ret;
}
#else
#define atsmb_suspend	NULL
#define atsmb_resume	NULL
#endif

static struct platform_driver atsmb_driver = {
	.driver.name		= "sdmmc",
	//.probe		= atsmb_probe,
	.remove		= atsmb_remove,
	.shutdown	= atsmb_shutdown,
	.suspend	= atsmb_suspend,
	.resume		= atsmb_resume,
};

static struct platform_device wmt_sdmmc_device = {
	.name			= "sdmmc",
	.id				= 0,
	.dev            = {
	.dma_mask 		= &wmt_sdmmc_dma_mask,
	.coherent_dma_mask = ~0,
	.release = atsmb_release,
	},
	.num_resources  = ARRAY_SIZE(wmt_sdmmc_resources),
	.resource       = wmt_sdmmc_resources,
};


static int __init atsmb_init(void)
{
	int ret;
	DBG("[%s] s\n",__func__);
	get_driver_version();
	
	if (platform_device_register(&wmt_sdmmc_device))//add by jay,for modules support
		return -1;
	//ret = platform_driver_register(&atsmb_driver);
	ret = platform_driver_probe(&atsmb_driver, atsmb_probe);

	DBG("[%s] e\n",__func__);
	return ret;
}

static void __exit atsmb_exit(void)
{
	DBG("[%s] s\n",__func__);
	(void)platform_driver_unregister(&atsmb_driver);
	(void)platform_device_unregister(&wmt_sdmmc_device);//add by jay,for modules support
	DBG("[%s] e\n",__func__);
}

module_init(atsmb_init);
module_exit(atsmb_exit);
module_param(fmax, uint, 0444);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [AHB to SD/MMC Bridge] driver");
MODULE_LICENSE("GPL");
