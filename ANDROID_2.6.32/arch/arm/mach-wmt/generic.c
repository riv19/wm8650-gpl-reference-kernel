/*
  linux/arch/arm/mach-wmt/generic.c

  wmt generic architecture level codes
	Some descriptions of such software. Copyright (c) 2008  WonderMedia Technologies, Inc.

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
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/cpufreq.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/mach/map.h>
#include <asm/irq.h>
#include <asm/sizes.h>
#include <linux/i2c.h>

#include "generic.h"
#include <linux/android_pmem.h>
#include <linux/spi/spi.h>

#ifdef CONFIG_VT34XX_SPI_SUPPORT
#include <mach/vt34xx_spi.h>
#endif

#ifdef CONFIG_ROHM_BU21020_SUPPORT
#include <linux/spi/rohm_bu21020.h>
#endif

/* TODO*/
#define PMHC_HIBERNATE 0x05

extern void wmt_power_up_debounce_value(void);

static void wmt_power_off(void)
{
	/*set power button debounce value*/
	wmt_power_up_debounce_value();
#if 1 /*fan*/
	mdelay(100);

	local_irq_disable();

	/*
	 * Set scratchpad to zero, just in case it is used as a restart
	 * address by the bootloader. Since PB_RESUME button has been
	 * set to be one of the wakeup sources, clean the resume address
	 * will cause zacboot to issue a SW_RESET, for design a behavior
	 * to let PB_RESUME button be a power on button.
	 *
	 * Also force to disable watchdog timer, if it has been enabled.
	 */
	HSP0_VAL = 0;
	OSTW_VAL &= ~OSTW_WE;

	/*
	 * Well, I cannot power-off myself,
	 * so try to enter power-off suspend mode.
	 */
	PMHC_VAL = PMHC_HIBERNATE;
	asm("mcr%? p15, 0, %0, c7, c0, 4" : : "r" (0));		/* Force ARM to idle mode*/
#endif
}

static struct resource wmt_uart0_resources[] = {
	[0] = {
		.start  = 0xd8200000,
		.end    = 0xd820ffff,
		.flags  = IORESOURCE_MEM,
	},
};
/*
static struct resource wmt_uart1_resources[] = {
	[0] = {
		.start  = 0xd82b0000,
		.end    = 0xd82bffff,
		.flags  = IORESOURCE_MEM,
	},
};
*/
static struct platform_device wmt_uart0_device = {
	.name           = "uart",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(wmt_uart0_resources),
	.resource       = wmt_uart0_resources,
};
/*
static struct platform_device wmt_uart1_device = {
	.name           = "uart",
	.id             = 1,
	.num_resources  = ARRAY_SIZE(wmt_uart1_resources),
	.resource       = wmt_uart1_resources,
};
*/
static struct resource wmt_sf_resources[] = {
	[0] = {
		.start  = 0xd8002000,
		.end    = 0xd80023ff,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device wmt_sf_device = {
    .name           = "sf",
    .id             = 0,
    .num_resources  = ARRAY_SIZE(wmt_sf_resources),
    .resource       = wmt_sf_resources,
};
static struct resource wmt_nand_resources[] = {
	[0] = {
		.start  = 0xd8009000,
		.end    = 0xd80093FF,
		.flags  = IORESOURCE_MEM,
	},
};

static u64 wmt_nand_dma_mask = 0xffffffffUL;

static struct platform_device wmt_nand_device = {
	.name           = "nand",
	.id             = 0,
	.dev            = {
	.dma_mask 		= &wmt_nand_dma_mask,
	.coherent_dma_mask = ~0,
	},
	.num_resources  = ARRAY_SIZE(wmt_nand_resources),
	.resource       = wmt_nand_resources,
};

/* Jason, Address is not sure.*/
static struct resource wmt_i2s_resources[] = {
    [0] = {
		.start  = 0xD80ED800,
		.end    = 0xD80EDBFF,
		.flags  = IORESOURCE_MEM,
    },
};

static u64 wmt_i2s_dma_mask = 0xffffffffUL;

static struct platform_device wmt_i2s_device = {
	.name           = "i2s",
	.id             = 0,
	.dev            = {
	.dma_mask 		= &wmt_i2s_dma_mask,
	.coherent_dma_mask = ~0,
	},
	.num_resources  = ARRAY_SIZE(wmt_i2s_resources),
	.resource       = wmt_i2s_resources,
};


/*fan ++*/
static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.start = 0x0D800000,
	.size =  0x02000000,
	.no_allocator = 1,
	.cached = 1,
	.buffered = 1,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

#ifdef CONFIG_ITE_IT7260_SUPPORT
static struct i2c_board_info wmt_i2c_board_info[] = {
	{
		.type		= "IT7260-ts",
		.addr		= 0x46,
		.irq		= IRQ_GPIO
	},
};
#endif

#ifdef CONFIG_ROHM_BU21020_SUPPORT
static struct vt34xx_spi_slave bu21020_spi_slave_info = {
	.dma_en = 0,        /* spidev do not use dma     */
	.bits_per_word = 8, /* spidev use 8bits_per_word */
};

static struct bu21020_platform_data bu21020_pdata;
#endif

#ifdef CONFIG_VT34XX_SPI_SUPPORT
static struct spi_board_info vt34xx_spi_board_info[] = {
#ifdef CONFIG_ROHM_BU21020_SUPPORT
	{
		.modalias           = "bu21020",
		.platform_data      = &bu21020_pdata,
		.controller_data    = &bu21020_spi_slave_info,
		.irq                = IRQ_SPI,
		.max_speed_hz       = 2000000,          /* 2Mhz            */
		.bus_num            = 0,                    /* use spi master 0 */
		.mode               = SPI_CLK_MODE3,
		.chip_select        = 1,                    /* as slave 0, CS=0 */
	},
#endif
};
#endif

#ifdef CONFIG_VT34XX_SPI_SUPPORT
static struct vt34xx_spi_hw vt34xx_spi_info = {
	/* spi on vt34xx can support dma */
	.dma_support       = SPI_DMA_ENABLE,
	/* can support 4 slaves when vt34xx spi as master */
	.num_chipselect    = MAX_SPI_SLAVE,
	/* vt34xx spi support 16bits_per_word? i'm not sure */
	.bits_per_word_en  = BITS8_PER_WORD_EN,
	/* vt34xx spi can support multi-master also, but it seems we do not need it */
	.port_mode         = PORT_MODE_PTP,
	/* ssn driven low when enable */
	.ssn_ctrl          = SSN_CTRL_HARDWARE,
	/* actual 36bytes, but we use 32bytes */
	.fifo_size         = SPI_FIFO_SIZE,
	/* 4Kbytes, same as the DMA */
	.max_transfer_length = SPI_MAX_TRANSFER_LENGTH,
	/* it's really needed? i'm not sure   */
	.min_freq_hz       = SPI_MIN_FREQ_HZ,
	/* max freq 100Mhz */
	.max_freq_hz       = SPI_MAX_FREQ_HZ,
};

static struct resource vt34xx_spi_resources[] = {
	[0] = {
		.start = 0xD8240000,
		.end   = 0xD824FFFF,
		.flags = IORESOURCE_MEM,
	      },
	[1] = {
		.start = IRQ_SPI,
		.end   = IRQ_SPI,
		.flags = IORESOURCE_IRQ,
	      },
};

static u64 vt34xx_spi_dma_mask = 0xFFFFFFFFUL;

static struct platform_device vt34xx_spi_device = {
	.name              = "vt34xx_spi",
	.id                = 0,
	.dev               = {
		.dma_mask          = &vt34xx_spi_dma_mask,
		.coherent_dma_mask = ~0,
		.platform_data     = &vt34xx_spi_info,
	},
	.num_resources     = ARRAY_SIZE(vt34xx_spi_resources),
	.resource          = vt34xx_spi_resources,
};
#endif

static struct platform_device *wmt_devices[] __initdata = {
	&wmt_uart0_device,
	//&wmt_uart1_device,
	&wmt_sf_device,
	&wmt_nand_device,
	&wmt_i2s_device,
	&android_pmem_device, /*fan*/
#ifdef CONFIG_VT34XX_SPI_SUPPORT
	&vt34xx_spi_device,
#endif
};

static int __init wmt_init(void)
{
	pm_power_off = wmt_power_off;
	
#ifdef CONFIG_ITE_IT7260_SUPPORT	
	i2c_register_board_info(0, wmt_i2c_board_info, ARRAY_SIZE(wmt_i2c_board_info));
#endif
#ifdef CONFIG_VT34XX_SPI_SUPPORT
	spi_register_board_info(vt34xx_spi_board_info, ARRAY_SIZE(vt34xx_spi_board_info));
#endif

	return platform_add_devices(wmt_devices, ARRAY_SIZE(wmt_devices));
}

arch_initcall(wmt_init);
