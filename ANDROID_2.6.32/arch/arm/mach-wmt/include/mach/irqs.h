/*
 *	linux/include/asm-arm/arch-wmt/irqs.h
 *
 *	Some descriptions of such software. Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 *	This program is free software: you can redistribute it and/or modify it under the
 *	terms of the GNU General Public License as published by the Free Software Foundation,
 *	either version 2 of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful, but WITHOUT
 *	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *	You should have received a copy of the GNU General Public License along with
 *	this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	WonderMedia Technologies, Inc.
 *	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

/*
 *
 *  Interrupt sources.
 *
 */
/*** Interrupt Controller 0 ***/
#define IRQ_SDC1             0       /* SD Host controller 0             */
#define IRQ_SDC1_DMA         1
#define IRQ_APBB             2
/* #define IRQ_ATA           3 */
/* #define IRQ_SMC           4 */
#define IRQ_DSP              5
#define IRQ_GPIO             6
/* #define IRQ_XD            7 */
/* #define IRQ_XD_DMA        8 */
/* #define IRQ_LCD           9 */
#define IRQ_ETH0             10
#define IRQ_DMA_CH_0         11
#define IRQ_DMA_CH_1         12
#define IRQ_DMA_CH_2         13
#define IRQ_DMA_CH_3         14
#define IRQ_DMA_CH_4         15
#define IRQ_DMA_CH_5         16
#define IRQ_DMA_CH_6         17
#define IRQ_DMA_CH_7         18
#define IRQ_I2C              19
#define IRQ_SDC0             20      /* SD Host controller 1             */
#define IRQ_SDC0_DMA         21
#define IRQ_PMC_WAKEUP       22      /* PMC wakeup                       */
#define IRQ_KPAD             23
#define IRQ_SPI              24      /* Serial Peripheral Interface 0    */
/* #define IRQ_ROT           25 */
#define	IRQ_SUS_GPIO0        26
#define	IRQ_SUS_GPIO1        27
#define IRQ_NFC              28
#define IRQ_NFC_DMA          29
/* #define IRQ_MSC           30 */
/* #define IRQ_MSC_DMA       31 */
#define IRQ_UART0            32
#define IRQ_UART1            33
#define	IRQ_I2C1             34      /* I2S controller                   */
/* IRQ_REVERSED              35 */
#define IRQ_OST0             36      /* OS Timer match 0                 */
#define IRQ_OST1             37      /* OS Timer match 1                 */
#define IRQ_OST2             38      /* OS Timer match 2                 */
#define IRQ_OST3             39      /* OS Timer match 3                 */
/* IRQ_REVERSED              40  */
/* IRQ_REVERSED              41  */
/* IRQ_REVERSED              42  */
#define IRQ_UHDC             43
#define IRQ_DMA_CH_8         44
#define IRQ_DMA_CH_9         45
#define IRQ_DMA_CH_10        46
#define IRQ_DMA_CH_11        47
#define IRQ_RTC1             48      /* RTC_PCLK_INTR                    */
#define IRQ_RTC2             49      /* RTC_PCLK_RTI                     */
#define IRQ_DMA_CH_12        50
#define IRQ_DMA_CH_13        51
#define IRQ_DMA_CH_14        52
#define IRQ_DMA_CH_15        53
/* IRQ_REVERSED              54  */
#define IRQ_CIR              55
#define IRQ_IC1_IRQ0         56
#define IRQ_IC1_IRQ1         57
#define IRQ_IC1_IRQ2         58
#define IRQ_IC1_IRQ3         59
#define IRQ_IC1_IRQ4         60
#define IRQ_IC1_IRQ5         61
#define IRQ_IC1_IRQ6         62
#define IRQ_IC1_IRQ7         63
/*** Interrupt Controller 1 ***/
#define IRQ_VPP_IRQ0         64
#define IRQ_VPP_IRQ1         65
#define IRQ_VPP_IRQ2         66
#define IRQ_VPP_IRQ3         67
#define IRQ_VPP_IRQ4         68
#define IRQ_VPP_IRQ5         69
#define IRQ_VPP_IRQ6         70
#define IRQ_VPP_IRQ7         71
#define IRQ_VPP_IRQ8         72
#define IRQ_VPP_IRQ9         73
#define IRQ_VPP_IRQ10        74
#define IRQ_VPP_IRQ11        75
#define IRQ_VPP_IRQ12        76
#define IRQ_VPP_IRQ13        77
#define IRQ_VPP_IRQ14        78
#define IRQ_VPP_IRQ15        79
#define IRQ_VPP_IRQ16        80 	/* NA12								*/
#define IRQ_VPP_IRQ17        81 	/* NA12								*/
#define IRQ_VPP_IRQ18        82 	/* NA12								*/
#define IRQ_DZ_0             83		/*AUDPRF*/
#define IRQ_DZ_1             84
#define IRQ_DZ_2             85
#define IRQ_DZ_3             86
#define IRQ_DZ_4             87
#define IRQ_DZ_5             88
#define IRQ_DZ_6             89
#define IRQ_DZ_7             90
#define IRQ_DZ_8             91
/* IRQ_REVERSED              92 ~ 127 */

#endif
