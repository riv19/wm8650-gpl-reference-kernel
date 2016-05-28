/*++
	linux/include/asm-arm/arch-wmt/wmt_gpio.h

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
--*/

/* Be sure that virtual mapping is defined right */

#ifndef __ASM_ARCH_HARDWARE_H
#error "You must include hardware.h, not wmt_gpio.h"
#endif

#ifndef __WMT_GPIO_H
/* To assert that only one occurrence is included */
#define __WMT_GPIO_H

/*=== wmt_gpio.h ================================================================
*   Copyright (C) 2008  WonderMedia Technologies, Inc.
*
* MODULE       : wmt_gpio.h --
* AUTHOR       : Kenny Chou
* DATE         : 2009/01/07
* DESCRIPTION  : General Purpose Input/Output definition
*------------------------------------------------------------------------------*/

/*--- History -------------------------------------------------------------------
*Version 0.01 , Kenny Chou, 2009/01/07
*    First version
*
*Version 0.02 , Tommy Huang, 2009/01/19
*    Second version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY --------------------------------------*/
#ifndef APPLICATION
#else
#endif

#ifndef __ASM_ARCH_HARDWARE_H
#include <mach/hardware.h>
#endif




/*-------------------- EXPORTED PRIVATE CONSTANTS -----------------------------*/

/*-------------------- EXPORTED PRIVATE TYPES----------------------------------*/

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef XXX_C /* allocate memory for variables only in xxx.c */
#       define EXTERN
#else
#       define EXTERN   extern
#endif /* ifdef XXX_C */


#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
#ifdef GPIO_BASE_ADDR               /* From wmt_map.h*/
#define __GPIO_BASE      GPIO_BASE_ADDR
#else
#define __GPIO_BASE      0xD8110000
#endif

#define GIRQ_LOW                        0x00            /* Input zero generate GPIO_IRQ signal */
#define GIRQ_HIGH                       0x01            /* Input one generate GPIO_IRQ signal */
#define GIRQ_FALLING            0x02            /* Falling edge generate GPIO_IRQ signal */
#define GIRQ_RISING                     0x03            /* Rising edge generate GPIO_IRQ signal */
#define GIRQ_BOTHEDGE           0x04
#define GIRQ_TYPEMASK           0x07
#define GIRQ_TYPE(idx, type)    ((type & GIRQ_TYPEMASK) << (idx * 8)) /* idx must be 0-3 */
#define GIRQ_EN_STS(idx)                ( 1 << ((idx+1)*8-1) )  /* idx must be 0-3 */


#define GPIO_ID_GP0_BYTE_ADDR 					(__GPIO_BASE + 0x0  )
#define GPIO_ID_GP1_BYTE_ADDR 					(__GPIO_BASE + 0x1  )
#define GPIO_ID_GP2_WAKEUP_SUS_BYTE_ADDR 		(__GPIO_BASE + 0x2  )
#define GPIO_ID_GP3_STORGE_BYTE_ADDR 			(__GPIO_BASE + 0x3  )
#define GPIO_ID_GP4_VDOUT_7_0_BYTE_ADDR    	    (__GPIO_BASE + 0x4  )
#define GPIO_ID_GP5_VDOUT_15_8_BYTE_ADDR        (__GPIO_BASE + 0x5  )
#define GPIO_ID_GP6_VDOUT_23_16_BYTE_ADDR 		(__GPIO_BASE + 0x6  )
#define GPIO_ID_GP7_VD_BYTE_ADDR       			(__GPIO_BASE + 0x7  )
#define GPIO_ID_GP8_VDIN_BYTE_ADDR     			(__GPIO_BASE + 0x8  )
#define GPIO_ID_GP9_VSYNC_BYTE_ADDR 			(__GPIO_BASE + 0x9  )
#define GPIO_ID_GP10_I2S_BYTE_ADDR 				(__GPIO_BASE + 0xA  )
#define GPIO_ID_GP11_SPI_BYTE_ADDR      		(__GPIO_BASE + 0xB  )
#define GPIO_ID_GP12_XD_BYTE_ADDR       		(__GPIO_BASE + 0xC  )
#define GPIO_ID_GP13_XDIO_BYTE_ADDR       		(__GPIO_BASE + 0xD  )
#define GPIO_ID_GP14_SD_BYTE_ADDR       		(__GPIO_BASE + 0xE  )
#define GPIO_ID_GP15_NAND_7_0_BYTE_ADDR       	(__GPIO_BASE + 0xF  )
#define GPIO_ID_GP16_NAND_15_8_BYTE_ADDR       	(__GPIO_BASE + 0x10 )
#define GPIO_ID_GP17_IDE_CFG_0_BYTE_ADDR       	(__GPIO_BASE + 0x11 )
#define GPIO_ID_GP18_IDE_CFG_1_BYTE_ADDR       	(__GPIO_BASE + 0x12 )
#define GPIO_ID_GP19_IDED_7_0_BYTE_ADDR       	(__GPIO_BASE + 0x13 )
#define GPIO_ID_GP20_IDED_15_8_BYTE_ADDR 		(__GPIO_BASE + 0x14 )
#define GPIO_ID_GP21_I2C_BYTE_ADDR       		(__GPIO_BASE + 0x15 )
#define GPIO_ID_GP22_UART_0_1_BYTE_ADDR       	(__GPIO_BASE + 0x16 )
#define GPIO_ID_GP23_UART_2_3_BYTE_ADDR       	(__GPIO_BASE + 0x17 )
#define GPIO_ID_GP24_TSDIN_7_0_BYTE_ADDR       	(__GPIO_BASE + 0x18 )
#define GPIO_ID_GP25_TS_BYTE_ADDR       		(__GPIO_BASE + 0x19 )
#define GPIO_ID_GP26_KPAD_BYTE_ADDR       		(__GPIO_BASE + 0x1A )
#define GPIO_ID_GP27_SPDIF_BYTE_ADDR       		(__GPIO_BASE + 0x1B )
#define GPIO_ID_GP28_NAND_CFG_0_BYTE_ADDR   	(__GPIO_BASE + 0x1C )
#define GPIO_ID_GP29_NAND_CFG_1_BYTE_ADDR       (__GPIO_BASE + 0x1D )
#define GPIO_ID_GP30_SPI_FLASH_BYTE_ADDR 		(__GPIO_BASE + 0x1E )
#define GPIO_ID_GP31_PWM_BYTE_ADDR 				(__GPIO_BASE + 0x1F )
#define GPIO_ID_GP60_USB_BYTE_ADDR				(__GPIO_BASE + 0x3C ) // fan 
#define GPIO_CTRL_GP0_BYTE_ADDR 				(__GPIO_BASE + 0x40 )  
#define GPIO_CTRL_GP1_BYTE_ADDR 				(__GPIO_BASE + 0x41 )              
#define GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR 	    (__GPIO_BASE + 0x42 )              
#define GPIO_CTRL_GP3_STORGE_BYTE_ADDR 		    (__GPIO_BASE + 0x43 )              
#define GPIO_CTRL_GP4_VDOUT_7_0_BYTE_ADDR    	(__GPIO_BASE + 0x44 )              
#define GPIO_CTRL_GP5_VDOUT_15_8_BYTE_ADDR      (__GPIO_BASE + 0x45 )              
#define GPIO_CTRL_GP6_VDOUT_23_16_BYTE_ADDR 	(__GPIO_BASE + 0x46 )              
#define GPIO_CTRL_GP7_VD_BYTE_ADDR       		(__GPIO_BASE + 0x47 )              
#define GPIO_CTRL_GP8_VDIN_BYTE_ADDR     		(__GPIO_BASE + 0x48 )              
#define GPIO_CTRL_GP9_VSYNC_BYTE_ADDR 		    (__GPIO_BASE + 0x49 )              
#define GPIO_CTRL_GP10_I2S_BYTE_ADDR 			(__GPIO_BASE + 0x4A )              
#define GPIO_CTRL_GP11_SPI_BYTE_ADDR      	    (__GPIO_BASE + 0x4B )              
#define GPIO_CTRL_GP12_XD_BYTE_ADDR       	    (__GPIO_BASE + 0x4C )              
#define GPIO_CTRL_GP13_XDIO_BYTE_ADDR       	(__GPIO_BASE + 0x4D )              
#define GPIO_CTRL_GP14_SD_BYTE_ADDR       	    (__GPIO_BASE + 0x4E )              
#define GPIO_CTRL_GP15_NAND_7_0_BYTE_ADDR       (__GPIO_BASE + 0x4F )              
#define GPIO_CTRL_GP16_NAND_15_8_BYTE_ADDR      (__GPIO_BASE + 0x50 )             
#define GPIO_CTRL_GP17_IDE_CFG_0_BYTE_ADDR      (__GPIO_BASE + 0x51 )             
#define GPIO_CTRL_GP18_IDE_CFG_1_BYTE_ADDR      (__GPIO_BASE + 0x52 )             
#define GPIO_CTRL_GP19_IDED_7_0_BYTE_ADDR       (__GPIO_BASE + 0x53 )             
#define GPIO_CTRL_GP20_IDED_15_8_BYTE_ADDR 	    (__GPIO_BASE + 0x54 )             
#define GPIO_CTRL_GP21_I2C_BYTE_ADDR       	    (__GPIO_BASE + 0x55 )
#define GPIO_CTRL_GP22_UART_0_1_BYTE_ADDR       (__GPIO_BASE + 0x56 )
#define GPIO_CTRL_GP23_UART_2_3_BYTE_ADDR       (__GPIO_BASE + 0x57 )             
#define GPIO_CTRL_GP24_TSDIN_7_0_BYTE_ADDR      (__GPIO_BASE + 0x58 )             
#define GPIO_CTRL_GP25_TS_BYTE_ADDR       	    (__GPIO_BASE + 0x59 )             
#define GPIO_CTRL_GP26_KPAD_BYTE_ADDR       	(__GPIO_BASE + 0x5A )             
#define GPIO_CTRL_GP27_SPDIF_BYTE_ADDR       	(__GPIO_BASE + 0x5B )             
#define GPIO_CTRL_GP28_NAND_CFG_0_BYTE_ADDR     (__GPIO_BASE + 0x5C )             
#define GPIO_CTRL_GP29_NAND_CFG_1_BYTE_ADDR     (__GPIO_BASE + 0x5D )             
#define GPIO_CTRL_GP30_SPI_FLASH_BYTE_ADDR 	    (__GPIO_BASE + 0x5E )             
#define GPIO_CTRL_GP31_PWM_BYTE_ADDR 			(__GPIO_BASE + 0x5F )	              
                     
#define GPIO_CTRL_GP60_USB_BYTE_ADDR			(__GPIO_BASE + 0x7C )//fan
                                          
#define GPIO_OC_GP0_BYTE_ADDR 			    	(__GPIO_BASE + 0x80 )                
#define GPIO_OC_GP1_BYTE_ADDR 			        (__GPIO_BASE + 0x81 )                    
#define GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR 	    (__GPIO_BASE + 0x82 )                    
#define GPIO_OC_GP3_STORGE_BYTE_ADDR 		    (__GPIO_BASE + 0x83 )                    
#define GPIO_OC_GP4_VDOUT_7_0_BYTE_ADDR         (__GPIO_BASE + 0x84 )                    
#define GPIO_OC_GP5_VDOUT_15_8_BYTE_ADDR        (__GPIO_BASE + 0x85 )                    
#define GPIO_OC_GP6_VDOUT_23_16_BYTE_ADDR       (__GPIO_BASE + 0x86 )                    
#define GPIO_OC_GP7_VD_BYTE_ADDR       	        (__GPIO_BASE + 0x87 )                    
#define GPIO_OC_GP8_VDIN_BYTE_ADDR     	        (__GPIO_BASE + 0x88 )                    
#define GPIO_OC_GP9_VSYNC_BYTE_ADDR 		    (__GPIO_BASE + 0x89 )                    
#define GPIO_OC_GP10_I2S_BYTE_ADDR 		        (__GPIO_BASE + 0x8A )                    
#define GPIO_OC_GP11_SPI_BYTE_ADDR      	    (__GPIO_BASE + 0x8B )                    
#define GPIO_OC_GP12_XD_BYTE_ADDR       	    (__GPIO_BASE + 0x8C )                    
#define GPIO_OC_GP13_XDIO_BYTE_ADDR             (__GPIO_BASE + 0x8D )                    
#define GPIO_OC_GP14_SD_BYTE_ADDR       	    (__GPIO_BASE + 0x8E )                    
#define GPIO_OC_GP15_NAND_7_0_BYTE_ADDR         (__GPIO_BASE + 0x8F )                    
#define GPIO_OC_GP16_NAND_15_8_BYTE_ADDR        (__GPIO_BASE + 0x90 )                    
#define GPIO_OC_GP17_IDE_CFG_0_BYTE_ADDR        (__GPIO_BASE + 0x91 )                    
#define GPIO_OC_GP18_IDE_CFG_1_BYTE_ADDR        (__GPIO_BASE + 0x92 )                    
#define GPIO_OC_GP19_IDED_7_0_BYTE_ADDR         (__GPIO_BASE + 0x93 )                    
#define GPIO_OC_GP20_IDED_15_8_BYTE_ADDR 	    (__GPIO_BASE + 0x94 )                    
#define GPIO_OC_GP21_I2C_BYTE_ADDR       	    (__GPIO_BASE + 0x95 )                    
#define GPIO_OC_GP22_UART_0_1_BYTE_ADDR         (__GPIO_BASE + 0x96 )                    
#define GPIO_OC_GP23_UART_2_3_BYTE_ADDR         (__GPIO_BASE + 0x97 )                    
#define GPIO_OC_GP24_TSDIN_7_0_BYTE_ADDR        (__GPIO_BASE + 0x98 )                    
#define GPIO_OC_GP25_TS_BYTE_ADDR       	    (__GPIO_BASE + 0x99 )                    
#define GPIO_OC_GP26_KPAD_BYTE_ADDR             (__GPIO_BASE + 0x9A )                    
#define GPIO_OC_GP27_SPDIF_BYTE_ADDR            (__GPIO_BASE + 0x9B )                    
#define GPIO_OC_GP28_NAND_CFG_0_BYTE_ADDR       (__GPIO_BASE + 0x9C )                    
#define GPIO_OC_GP29_NAND_CFG_1_BYTE_ADDR       (__GPIO_BASE + 0x9D )                    
#define GPIO_OC_GP30_SPI_FLASH_BYTE_ADDR 	    (__GPIO_BASE + 0x9E )                    
#define GPIO_OC_GP31_PWM_BYTE_ADDR 		        (__GPIO_BASE + 0x9F )	                     
#define GPIO_OC_GP60_USB_BYTE_ADDR				(__GPIO_BASE + 0xBC )//fan


#define GPIO_OD_GP0_BYTE_ADDR 			    	(__GPIO_BASE + 0xC0 )                  
#define GPIO_OD_GP1_BYTE_ADDR 			        (__GPIO_BASE + 0xC1 )                     
#define GPIO_OD_GP2_WAKEUP_SUS_BYTE_ADDR 	    (__GPIO_BASE + 0xC2 )                     
#define GPIO_OD_GP3_STORGE_BYTE_ADDR 		    (__GPIO_BASE + 0xC3 )                     
#define GPIO_OD_GP4_VDOUT_7_0_BYTE_ADDR         (__GPIO_BASE + 0xC4 )                     
#define GPIO_OD_GP5_VDOUT_15_8_BYTE_ADDR        (__GPIO_BASE + 0xC5 )                     
#define GPIO_OD_GP6_VDOUT_23_16_BYTE_ADDR       (__GPIO_BASE + 0xC6 )                     
#define GPIO_OD_GP7_VD_BYTE_ADDR       	        (__GPIO_BASE + 0xC7 )                     
#define GPIO_OD_GP8_VDIN_BYTE_ADDR     	        (__GPIO_BASE + 0xC8 )                     
#define GPIO_OD_GP9_VSYNC_BYTE_ADDR 		    (__GPIO_BASE + 0xC9 )                     
#define GPIO_OD_GP10_I2S_BYTE_ADDR 		        (__GPIO_BASE + 0xCA )                     
#define GPIO_OD_GP11_SPI_BYTE_ADDR      	    (__GPIO_BASE + 0xCB )                     
#define GPIO_OD_GP12_XD_BYTE_ADDR       	    (__GPIO_BASE + 0xCC )                     
#define GPIO_OD_GP13_XDIO_BYTE_ADDR             (__GPIO_BASE + 0xCD )                     
#define GPIO_OD_GP14_SD_BYTE_ADDR       	    (__GPIO_BASE + 0xCE )                     
#define GPIO_OD_GP15_NAND_7_0_BYTE_ADDR         (__GPIO_BASE + 0xCF )                     
#define GPIO_OD_GP16_NAND_15_8_BYTE_ADDR        (__GPIO_BASE + 0xD0 )                     
#define GPIO_OD_GP17_IDE_CFG_0_BYTE_ADDR        (__GPIO_BASE + 0xD1 )                     
#define GPIO_OD_GP18_IDE_CFG_1_BYTE_ADDR        (__GPIO_BASE + 0xD2 )                     
#define GPIO_OD_GP19_IDED_7_0_BYTE_ADDR         (__GPIO_BASE + 0xD3 )                     
#define GPIO_OD_GP20_IDED_15_8_BYTE_ADDR 	    (__GPIO_BASE + 0xD4 )                     
#define GPIO_OD_GP21_I2C_BYTE_ADDR       	    (__GPIO_BASE + 0xD5 )                     
#define GPIO_OD_GP22_UART_0_1_BYTE_ADDR         (__GPIO_BASE + 0xD6 )                     
#define GPIO_OD_GP23_UART_2_3_BYTE_ADDR         (__GPIO_BASE + 0xD7 )                     
#define GPIO_OD_GP24_TSDIN_7_0_BYTE_ADDR        (__GPIO_BASE + 0xD8 )                     
#define GPIO_OD_GP25_TS_BYTE_ADDR       	    (__GPIO_BASE + 0xD9 )                     
#define GPIO_OD_GP26_KPAD_BYTE_ADDR             (__GPIO_BASE + 0xDA )                     
#define GPIO_OD_GP27_SPDIF_BYTE_ADDR            (__GPIO_BASE + 0xDB )                     
#define GPIO_OD_GP28_NAND_CFG_0_BYTE_ADDR       (__GPIO_BASE + 0xDC )                     
#define GPIO_OD_GP29_NAND_CFG_1_BYTE_ADDR       (__GPIO_BASE + 0xDD )                     
#define GPIO_OD_GP30_SPI_FLASH_BYTE_ADDR 	    (__GPIO_BASE + 0xDE )                     
#define GPIO_OD_GP31_PWM_BYTE_ADDR 		        (__GPIO_BASE + 0xDF )	                     
#define GPIO_OD_GP60_USB_BYTE_ADDR				(__GPIO_BASE + 0xFC )//fan


#define GPIO_STRAP_STATUS_ADDR					(__GPIO_BASE + 0x100 )
#define GPIO_RING_OSC_EXIT_STS_4BYTE_ADDR 		(__GPIO_BASE + 0x104 )//mark
#define GPIO_AHB_CTRL_4BYTE_ADDR 				(__GPIO_BASE + 0x108 )
//#define GPIO_SF_NEGEDGE_SAMPLE_CTRL_4BYTE_ADDR 	(__GPIO_BASE + 0x10C )
#define GPIO_USB_OP_CTRL_4BYTE_ADDR				(__GPIO_BASE + 0x10C )//fan
#define GPIO_BONDING_OPTION_4BYTE_ADDR     		(__GPIO_BASE + 0x110 )
#define GPIO_RGMII_SR_CTL_4BYTE_ADDR     		(__GPIO_BASE + 0x114 )//fan
#define GPIO_PIN_SHARING_SEL_4BYTE_ADDR 		(__GPIO_BASE + 0x200 )

#define GPIO_INT_REQ_TYPE_0_ADDR				(__GPIO_BASE + 0x300)//fan
#define GPIO_UART_INT_REQ_TYPE_ADDR				(__GPIO_BASE + 0x304)//fan
#define GPIO_I2C_INT_REQ_TYPE_ADDR				(__GPIO_BASE + 0x308)//fan
#define GPIO_I2S_KAPD_INT_REQ_TYPE_ADDR			(__GPIO_BASE + 0x30C)//fan
#define GPIO_KAPD_INT_REQ_TYPE_ADDR				(__GPIO_BASE + 0x310)//fan
#define GPIO_PWM_INT_REQ_TYPE_ADDR				(__GPIO_BASE + 0x314)//fan
#define GPIO_INT_REQ_TYPE_1_ADDR				(__GPIO_BASE + 0x318)//fan

#define GPIO_INT_REQ_STS_ADDR					(__GPIO_BASE + 0x0320)	/* [Rx320] GPIO Interrupt Request Status Register */

#define GPIO_IO_DRV_GPIO1_4BYTE_ADDR			(__GPIO_BASE + 0x400 )//fan
#define GPIO_IO_DRV_SD_4BYTE_ADDR 				(__GPIO_BASE + 0x404 ) 
#define GPIO_IO_DRV_VID_4BYTE_ADDR 				(__GPIO_BASE + 0x408 )
#define GPIO_IO_DRV_SF_4BYTE_ADDR 				(__GPIO_BASE + 0x40C )
#define GPIO_IO_DRV_NAND_4BYTE_ADDR 			(__GPIO_BASE + 0x414 )
#define GPIO_IO_DRV_SPI_4BYTE_ADDR 				(__GPIO_BASE + 0x418 )

#define GPIO_PULL_EN_GP0_BYTE_ADDR 			    (__GPIO_BASE + 0x480 )
#define GPIO_PULL_EN_GP1_BYTE_ADDR 			    (__GPIO_BASE + 0x481 )    
#define GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR 	(__GPIO_BASE + 0x482 )    
#define GPIO_PULL_EN_GP3_STORGE_BYTE_ADDR 		(__GPIO_BASE + 0x483 )    
#define GPIO_PULL_EN_GP4_VDOUT_7_0_BYTE_ADDR    (__GPIO_BASE + 0x484 )    
#define GPIO_PULL_EN_GP5_VDOUT_15_8_BYTE_ADDR   (__GPIO_BASE + 0x485 )    
#define GPIO_PULL_EN_GP6_VDOUT_23_16_BYTE_ADDR  (__GPIO_BASE + 0x486 )    
#define GPIO_PULL_EN_GP7_VD_BYTE_ADDR       	(__GPIO_BASE + 0x487 )    
#define GPIO_PULL_EN_GP8_VDIN_BYTE_ADDR     	(__GPIO_BASE + 0x488 )    
#define GPIO_PULL_EN_GP9_VSYNC_BYTE_ADDR 		(__GPIO_BASE + 0x489 )    
#define GPIO_PULL_EN_GP10_I2S_BYTE_ADDR 		(__GPIO_BASE + 0x48A )    
#define GPIO_PULL_EN_GP11_SPI_BYTE_ADDR      	(__GPIO_BASE + 0x48B )    
#define GPIO_PULL_EN_GP12_XD_BYTE_ADDR       	(__GPIO_BASE + 0x48C )    
#define GPIO_PULL_EN_GP13_XDIO_BYTE_ADDR        (__GPIO_BASE + 0x48D )    
#define GPIO_PULL_EN_GP14_SD_BYTE_ADDR       	(__GPIO_BASE + 0x48E )    
#define GPIO_PULL_EN_GP15_NAND_7_0_BYTE_ADDR    (__GPIO_BASE + 0x48F )    
#define GPIO_PULL_EN_GP16_NAND_15_8_BYTE_ADDR   (__GPIO_BASE + 0x490 )    
#define GPIO_PULL_EN_GP17_IDE_CFG_0_BYTE_ADDR   (__GPIO_BASE + 0x491 )    
#define GPIO_PULL_EN_GP18_IDE_CFG_1_BYTE_ADDR   (__GPIO_BASE + 0x492 )    
#define GPIO_PULL_EN_GP19_IDED_7_0_BYTE_ADDR    (__GPIO_BASE + 0x493 )    
#define GPIO_PULL_EN_GP20_IDED_15_8_BYTE_ADDR 	(__GPIO_BASE + 0x494 )    
#define GPIO_PULL_EN_GP21_I2C_BYTE_ADDR       	(__GPIO_BASE + 0x495 )    
#define GPIO_PULL_EN_GP22_UART_0_1_BYTE_ADDR    (__GPIO_BASE + 0x496 )    
#define GPIO_PULL_EN_GP23_UART_2_3_BYTE_ADDR    (__GPIO_BASE + 0x497 )    
#define GPIO_PULL_EN_GP24_TSDIN_7_0_BYTE_ADDR   (__GPIO_BASE + 0x498 )    
#define GPIO_PULL_EN_GP25_TS_BYTE_ADDR       	(__GPIO_BASE + 0x499 )    
#define GPIO_PULL_EN_GP26_KPAD_BYTE_ADDR        (__GPIO_BASE + 0x49A )    
#define GPIO_PULL_EN_GP27_SPDIF_BYTE_ADDR       (__GPIO_BASE + 0x49B )    
#define GPIO_PULL_EN_GP28_NAND_CFG_0_BYTE_ADDR  (__GPIO_BASE + 0x49C )    
#define GPIO_PULL_EN_GP29_NAND_CFG_1_BYTE_ADDR  (__GPIO_BASE + 0x49D )    
#define GPIO_PULL_EN_GP30_SPI_FLASH_BYTE_ADDR 	(__GPIO_BASE + 0x49E )    
#define GPIO_PULL_EN_GP31_PWM_BYTE_ADDR 		(__GPIO_BASE + 0x49F )        
#define GPIO_PULL_EN_GP60_USB_TYTE_ADDR			(__GPIO_BASE + 0x4BC )    

#define GPIO_PULL_CTRL_GP0_BYTE_ADDR 			    (__GPIO_BASE + 0x4C0 )
#define GPIO_PULL_CTRL_GP1_BYTE_ADDR 			    (__GPIO_BASE + 0x4C1 )    
#define GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR 	(__GPIO_BASE + 0x4C2 )    
#define GPIO_PULL_CTRL_GP3_STORGE_BYTE_ADDR 		(__GPIO_BASE + 0x4C3 )    
#define GPIO_PULL_CTRL_GP4_VDOUT_7_0_BYTE_ADDR      (__GPIO_BASE + 0x4C4 )    
#define GPIO_PULL_CTRL_GP5_VDOUT_15_8_BYTE_ADDR     (__GPIO_BASE + 0x4C5 )    
#define GPIO_PULL_CTRL_GP6_VDOUT_23_16_BYTE_ADDR    (__GPIO_BASE + 0x4C6 )    
#define GPIO_PULL_CTRL_GP7_VD_BYTE_ADDR       	    (__GPIO_BASE + 0x4C7 )    
#define GPIO_PULL_CTRL_GP8_VDIN_BYTE_ADDR     	    (__GPIO_BASE + 0x4C8 )    
#define GPIO_PULL_CTRL_GP9_VSYNC_BYTE_ADDR 		    (__GPIO_BASE + 0x4C9 )    
#define GPIO_PULL_CTRL_GP10_I2S_BYTE_ADDR 		    (__GPIO_BASE + 0x4CA )    
#define GPIO_PULL_CTRL_GP11_SPI_BYTE_ADDR      	    (__GPIO_BASE + 0x4CB )    
#define GPIO_PULL_CTRL_GP12_XD_BYTE_ADDR       	    (__GPIO_BASE + 0x4CC )    
#define GPIO_PULL_CTRL_GP13_XDIO_BYTE_ADDR          (__GPIO_BASE + 0x4CD )    
#define GPIO_PULL_CTRL_GP14_SD_BYTE_ADDR       	    (__GPIO_BASE + 0x4CE )    
#define GPIO_PULL_CTRL_GP15_NAND_7_0_BYTE_ADDR      (__GPIO_BASE + 0x4CF )    
#define GPIO_PULL_CTRL_GP16_NAND_15_8_BYTE_ADDR     (__GPIO_BASE + 0x4D0 )    
#define GPIO_PULL_CTRL_GP17_IDE_CFG_0_BYTE_ADDR     (__GPIO_BASE + 0x4D1 )     
#define GPIO_PULL_CTRL_GP18_IDE_CFG_1_BYTE_ADDR     (__GPIO_BASE + 0x4D2 )    
#define GPIO_PULL_CTRL_GP19_IDED_7_0_BYTE_ADDR      (__GPIO_BASE + 0x4D3 )    
#define GPIO_PULL_CTRL_GP20_IDED_15_8_BYTE_ADDR 	(__GPIO_BASE + 0x4D4 )    
#define GPIO_PULL_CTRL_GP21_I2C_BYTE_ADDR       	(__GPIO_BASE + 0x4D5 )    
#define GPIO_PULL_CTRL_GP22_UART_0_1_BYTE_ADDR      (__GPIO_BASE + 0x4D6 )    
#define GPIO_PULL_CTRL_GP23_UART_2_3_BYTE_ADDR      (__GPIO_BASE + 0x4D7 )    
#define GPIO_PULL_CTRL_GP24_TSDIN_7_0_BYTE_ADDR     (__GPIO_BASE + 0x4D8 )    
#define GPIO_PULL_CTRL_GP25_TS_BYTE_ADDR       	    (__GPIO_BASE + 0x4D9 )    
#define GPIO_PULL_CTRL_GP26_KPAD_BYTE_ADDR          (__GPIO_BASE + 0x4DA )    
#define GPIO_PULL_CTRL_GP27_SPDIF_BYTE_ADDR         (__GPIO_BASE + 0x4DB )    
#define GPIO_PULL_CTRL_GP28_NAND_CFG_0_BYTE_ADDR    (__GPIO_BASE + 0x4DC )    
#define GPIO_PULL_CTRL_GP29_NAND_CFG_1_BYTE_ADDR    (__GPIO_BASE + 0x4DD )    
#define GPIO_PULL_CTRL_GP30_SPI_FLASH_BYTE_ADDR 	(__GPIO_BASE + 0x4DE )    
#define GPIO_PULL_CTRL_GP31_PWM_BYTE_ADDR 		    (__GPIO_BASE + 0x4DF )        
#define GPIO_PULL_CTRL_GP60_USB_BYTE_ADDR			(__GPIO_BASE + 0x4FC )//fan


#define GPIO_ID_GP0_BYTE_REG 					REG8_PTR(GPIO_ID_GP0_BYTE_ADDR 				    )
#define GPIO_ID_GP1_BYTE_REG 					REG8_PTR(GPIO_ID_GP1_BYTE_ADDR 				    )
#define GPIO_ID_GP2_WAKEUP_SUS_BYTE_REG 		REG8_PTR(GPIO_ID_GP2_WAKEUP_SUS_BYTE_ADDR 	    )
#define GPIO_ID_GP3_STORGE_BYTE_REG 			REG8_PTR(GPIO_ID_GP3_STORGE_BYTE_ADDR 		    )
#define GPIO_ID_GP4_VDOUT_7_0_BYTE_REG    	    REG8_PTR(GPIO_ID_GP4_VDOUT_7_0_BYTE_ADDR    	)
#define GPIO_ID_GP5_VDOUT_15_8_BYTE_REG        	REG8_PTR(GPIO_ID_GP5_VDOUT_15_8_BYTE_ADDR       )
#define GPIO_ID_GP6_VDOUT_23_16_BYTE_REG 		REG8_PTR(GPIO_ID_GP6_VDOUT_23_16_BYTE_ADDR 	    )
#define GPIO_ID_GP7_VD_BYTE_REG       			REG8_PTR(GPIO_ID_GP7_VD_BYTE_ADDR       		)
#define GPIO_ID_GP8_VDIN_BYTE_REG     			REG8_PTR(GPIO_ID_GP8_VDIN_BYTE_ADDR     		)
#define GPIO_ID_GP9_VSYNC_BYTE_REG 				REG8_PTR(GPIO_ID_GP9_VSYNC_BYTE_ADDR 		    )
#define GPIO_ID_GP10_I2S_BYTE_REG 				REG8_PTR(GPIO_ID_GP10_I2S_BYTE_ADDR 			)
#define GPIO_ID_GP11_SPI_BYTE_REG      			REG8_PTR(GPIO_ID_GP11_SPI_BYTE_ADDR      	    )
#define GPIO_ID_GP12_XD_BYTE_REG       			REG8_PTR(GPIO_ID_GP12_XD_BYTE_ADDR       	    )
#define GPIO_ID_GP13_XDIO_BYTE_REG       		REG8_PTR(GPIO_ID_GP13_XDIO_BYTE_ADDR       	    )
#define GPIO_ID_GP14_SD_BYTE_REG       			REG8_PTR(GPIO_ID_GP14_SD_BYTE_ADDR       	    )
#define GPIO_ID_GP15_NAND_7_0_BYTE_REG       	REG8_PTR(GPIO_ID_GP15_NAND_7_0_BYTE_ADDR        )
#define GPIO_ID_GP16_NAND_15_8_BYTE_REG       	REG8_PTR(GPIO_ID_GP16_NAND_15_8_BYTE_ADDR       )
#define GPIO_ID_GP17_IDE_CFG_0_BYTE_REG       	REG8_PTR(GPIO_ID_GP17_IDE_CFG_0_BYTE_ADDR       )
#define GPIO_ID_GP18_IDE_CFG_1_BYTE_REG       	REG8_PTR(GPIO_ID_GP18_IDE_CFG_1_BYTE_ADDR       )
#define GPIO_ID_GP19_IDED_7_0_BYTE_REG       	REG8_PTR(GPIO_ID_GP19_IDED_7_0_BYTE_ADDR        )
#define GPIO_ID_GP20_IDED_15_8_BYTE_REG 		REG8_PTR(GPIO_ID_GP20_IDED_15_8_BYTE_ADDR 	    )
#define GPIO_ID_GP21_I2C_BYTE_REG       		REG8_PTR(GPIO_ID_GP21_I2C_BYTE_ADDR       	    )
#define GPIO_ID_GP22_UART_0_1_BYTE_REG       	REG8_PTR(GPIO_ID_GP22_UART_0_1_BYTE_ADDR        )
#define GPIO_ID_GP23_UART_2_3_BYTE_REG       	REG8_PTR(GPIO_ID_GP23_UART_2_3_BYTE_ADDR        )
#define GPIO_ID_GP24_TSDIN_7_0_BYTE_REG       	REG8_PTR(GPIO_ID_GP24_TSDIN_7_0_BYTE_ADDR       )
#define GPIO_ID_GP25_TS_BYTE_REG       			REG8_PTR(GPIO_ID_GP25_TS_BYTE_ADDR       	    )
#define GPIO_ID_GP26_KPAD_BYTE_REG       		REG8_PTR(GPIO_ID_GP26_KPAD_BYTE_ADDR       	    )
#define GPIO_ID_GP27_SPDIF_BYTE_REG       		REG8_PTR(GPIO_ID_GP27_SPDIF_BYTE_ADDR       	)
#define GPIO_ID_GP28_NAND_CFG_0_BYTE_REG   	    REG8_PTR(GPIO_ID_GP28_NAND_CFG_0_BYTE_ADDR      )
#define GPIO_ID_GP29_NAND_CFG_1_BYTE_REG        REG8_PTR(GPIO_ID_GP29_NAND_CFG_1_BYTE_ADDR      )
#define GPIO_ID_GP30_SPI_FLASH_BYTE_REG 		REG8_PTR(GPIO_ID_GP30_SPI_FLASH_BYTE_ADDR 	    )
#define GPIO_ID_GP31_PWM_BYTE_REG 				REG8_PTR(GPIO_ID_GP31_PWM_BYTE_ADDR 			)
#define GPIO_ID_GP60_USB_BYTE_REG				REG8_PTR(GPIO_ID_GP60_USB_BYTE_ADDR				)//fan
#define GPIO_CTRL_GP0_BYTE_REG 					REG8_PTR(GPIO_CTRL_GP0_BYTE_ADDR 			    )  
#define GPIO_CTRL_GP1_BYTE_REG 					REG8_PTR(GPIO_CTRL_GP1_BYTE_ADDR 			    )              
#define GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_REG 	    REG8_PTR(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR 	)              
#define GPIO_CTRL_GP3_STORGE_BYTE_REG 		    REG8_PTR(GPIO_CTRL_GP3_STORGE_BYTE_ADDR 		)              
#define GPIO_CTRL_GP4_VDOUT_7_0_BYTE_REG    	REG8_PTR(GPIO_CTRL_GP4_VDOUT_7_0_BYTE_ADDR      )              
#define GPIO_CTRL_GP5_VDOUT_15_8_BYTE_REG      	REG8_PTR(GPIO_CTRL_GP5_VDOUT_15_8_BYTE_ADDR     )              
#define GPIO_CTRL_GP6_VDOUT_23_16_BYTE_REG 		REG8_PTR(GPIO_CTRL_GP6_VDOUT_23_16_BYTE_ADDR    )              
#define GPIO_CTRL_GP7_VD_BYTE_REG       		REG8_PTR(GPIO_CTRL_GP7_VD_BYTE_ADDR       	    )              
#define GPIO_CTRL_GP8_VDIN_BYTE_REG     		REG8_PTR(GPIO_CTRL_GP8_VDIN_BYTE_ADDR     	    )              
#define GPIO_CTRL_GP9_VSYNC_BYTE_REG 		    REG8_PTR(GPIO_CTRL_GP9_VSYNC_BYTE_ADDR 		    )              
#define GPIO_CTRL_GP10_I2S_BYTE_REG 			REG8_PTR(GPIO_CTRL_GP10_I2S_BYTE_ADDR 		    )              
#define GPIO_CTRL_GP11_SPI_BYTE_REG      	    REG8_PTR(GPIO_CTRL_GP11_SPI_BYTE_ADDR      	    )              
#define GPIO_CTRL_GP12_XD_BYTE_REG       	    REG8_PTR(GPIO_CTRL_GP12_XD_BYTE_ADDR       	    )              
#define GPIO_CTRL_GP13_XDIO_BYTE_REG       		REG8_PTR(GPIO_CTRL_GP13_XDIO_BYTE_ADDR          )              
#define GPIO_CTRL_GP14_SD_BYTE_REG       	    REG8_PTR(GPIO_CTRL_GP14_SD_BYTE_ADDR       	    )              
#define GPIO_CTRL_GP15_NAND_7_0_BYTE_REG        REG8_PTR(GPIO_CTRL_GP15_NAND_7_0_BYTE_ADDR      )              
#define GPIO_CTRL_GP16_NAND_15_8_BYTE_REG       REG8_PTR(GPIO_CTRL_GP16_NAND_15_8_BYTE_ADDR     )             
#define GPIO_CTRL_GP17_IDE_CFG_0_BYTE_REG       REG8_PTR(GPIO_CTRL_GP17_IDE_CFG_0_BYTE_ADDR     )             
#define GPIO_CTRL_GP18_IDE_CFG_1_BYTE_REG       REG8_PTR(GPIO_CTRL_GP18_IDE_CFG_1_BYTE_ADDR     )             
#define GPIO_CTRL_GP19_IDED_7_0_BYTE_REG        REG8_PTR(GPIO_CTRL_GP19_IDED_7_0_BYTE_ADDR      )             
#define GPIO_CTRL_GP20_IDED_15_8_BYTE_REG 	    REG8_PTR(GPIO_CTRL_GP20_IDED_15_8_BYTE_ADDR 	)             
#define GPIO_CTRL_GP21_I2C_BYTE_REG       	    REG8_PTR(GPIO_CTRL_GP21_I2C_BYTE_ADDR       	)             
#define GPIO_CTRL_GP22_UART_0_1_BYTE_REG        REG8_PTR(GPIO_CTRL_GP22_UART_0_1_BYTE_ADDR      )             
#define GPIO_CTRL_GP23_UART_2_3_BYTE_REG        REG8_PTR(GPIO_CTRL_GP23_UART_2_3_BYTE_ADDR      )             
#define GPIO_CTRL_GP24_TSDIN_7_0_BYTE_REG       REG8_PTR(GPIO_CTRL_GP24_TSDIN_7_0_BYTE_ADDR     )             
#define GPIO_CTRL_GP25_TS_BYTE_REG       	    REG8_PTR(GPIO_CTRL_GP25_TS_BYTE_ADDR       	    )             
#define GPIO_CTRL_GP26_KPAD_BYTE_REG       	 	REG8_PTR(GPIO_CTRL_GP26_KPAD_BYTE_ADDR          )             
#define GPIO_CTRL_GP27_SPDIF_BYTE_REG       	REG8_PTR(GPIO_CTRL_GP27_SPDIF_BYTE_ADDR         )             
#define GPIO_CTRL_GP28_NAND_CFG_0_BYTE_REG      REG8_PTR(GPIO_CTRL_GP28_NAND_CFG_0_BYTE_ADDR    )             
#define GPIO_CTRL_GP29_NAND_CFG_1_BYTE_REG      REG8_PTR(GPIO_CTRL_GP29_NAND_CFG_1_BYTE_ADDR    )             
#define GPIO_CTRL_GP30_SPI_FLASH_BYTE_REG 	    REG8_PTR(GPIO_CTRL_GP30_SPI_FLASH_BYTE_ADDR 	)             
#define GPIO_CTRL_GP31_PWM_BYTE_REG 			REG8_PTR(GPIO_CTRL_GP31_PWM_BYTE_ADDR 		    )	              
                     
#define GPIO_CTRL_GP60_USB_BYTE_REG				REG8_PTR(GPIO_CTRL_GP60_USB_BYTE_ADDR			)//fan
                                          
#define GPIO_OC_GP0_BYTE_REG 			    	REG8_PTR(GPIO_OC_GP0_BYTE_ADDR 			        )                
#define GPIO_OC_GP1_BYTE_REG 			        REG8_PTR(GPIO_OC_GP1_BYTE_ADDR 			        )
#define GPIO_OC_GP2_WAKEUP_SUS_BYTE_REG 	    REG8_PTR(GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR 	    )                    
#define GPIO_OC_GP3_STORGE_BYTE_REG 		    REG8_PTR(GPIO_OC_GP3_STORGE_BYTE_ADDR 		    )
#define GPIO_OC_GP4_VDOUT_7_0_BYTE_REG          REG8_PTR(GPIO_OC_GP4_VDOUT_7_0_BYTE_ADDR        )                    
#define GPIO_OC_GP5_VDOUT_15_8_BYTE_REG         REG8_PTR(GPIO_OC_GP5_VDOUT_15_8_BYTE_ADDR       )                    
#define GPIO_OC_GP6_VDOUT_23_16_BYTE_REG        REG8_PTR(GPIO_OC_GP6_VDOUT_23_16_BYTE_ADDR      )                    
#define GPIO_OC_GP7_VD_BYTE_REG       	        REG8_PTR(GPIO_OC_GP7_VD_BYTE_ADDR       	    )
#define GPIO_OC_GP8_VDIN_BYTE_REG     	        REG8_PTR(GPIO_OC_GP8_VDIN_BYTE_ADDR     	    )                    
#define GPIO_OC_GP9_VSYNC_BYTE_REG 		     	REG8_PTR(GPIO_OC_GP9_VSYNC_BYTE_ADDR 		    )                    
#define GPIO_OC_GP10_I2S_BYTE_REG 		        REG8_PTR(GPIO_OC_GP10_I2S_BYTE_ADDR 		    )                    
#define GPIO_OC_GP11_SPI_BYTE_REG      	     	REG8_PTR(GPIO_OC_GP11_SPI_BYTE_ADDR      	    )                    
#define GPIO_OC_GP12_XD_BYTE_REG       	     	REG8_PTR(GPIO_OC_GP12_XD_BYTE_ADDR       	    )                    
#define GPIO_OC_GP13_XDIO_BYTE_REG              REG8_PTR(GPIO_OC_GP13_XDIO_BYTE_ADDR            )                    
#define GPIO_OC_GP14_SD_BYTE_REG       	     	REG8_PTR(GPIO_OC_GP14_SD_BYTE_ADDR       	    )                    
#define GPIO_OC_GP15_NAND_7_0_BYTE_REG          REG8_PTR(GPIO_OC_GP15_NAND_7_0_BYTE_ADDR        )                    
#define GPIO_OC_GP16_NAND_15_8_BYTE_REG         REG8_PTR(GPIO_OC_GP16_NAND_15_8_BYTE_ADDR       )                    
#define GPIO_OC_GP17_IDE_CFG_0_BYTE_REG         REG8_PTR(GPIO_OC_GP17_IDE_CFG_0_BYTE_ADDR       )                    
#define GPIO_OC_GP18_IDE_CFG_1_BYTE_REG         REG8_PTR(GPIO_OC_GP18_IDE_CFG_1_BYTE_ADDR       )                    
#define GPIO_OC_GP19_IDED_7_0_BYTE_REG          REG8_PTR(GPIO_OC_GP19_IDED_7_0_BYTE_ADDR        )                    
#define GPIO_OC_GP20_IDED_15_8_BYTE_REG 	    REG8_PTR(GPIO_OC_GP20_IDED_15_8_BYTE_ADDR 	    )                    
#define GPIO_OC_GP21_I2C_BYTE_REG       	    REG8_PTR(GPIO_OC_GP21_I2C_BYTE_ADDR       	    )                    
#define GPIO_OC_GP22_UART_0_1_BYTE_REG          REG8_PTR(GPIO_OC_GP22_UART_0_1_BYTE_ADDR        )                    
#define GPIO_OC_GP23_UART_2_3_BYTE_REG          REG8_PTR(GPIO_OC_GP23_UART_2_3_BYTE_ADDR        )                    
#define GPIO_OC_GP24_TSDIN_7_0_BYTE_REG         REG8_PTR(GPIO_OC_GP24_TSDIN_7_0_BYTE_ADDR       )                    
#define GPIO_OC_GP25_TS_BYTE_REG       	     	REG8_PTR(GPIO_OC_GP25_TS_BYTE_ADDR       	    )                    
#define GPIO_OC_GP26_KPAD_BYTE_REG              REG8_PTR(GPIO_OC_GP26_KPAD_BYTE_ADDR            )                    
#define GPIO_OC_GP27_SPDIF_BYTE_REG             REG8_PTR(GPIO_OC_GP27_SPDIF_BYTE_ADDR           )                    
#define GPIO_OC_GP28_NAND_CFG_0_BYTE_REG        REG8_PTR(GPIO_OC_GP28_NAND_CFG_0_BYTE_ADDR      )                    
#define GPIO_OC_GP29_NAND_CFG_1_BYTE_REG        REG8_PTR(GPIO_OC_GP29_NAND_CFG_1_BYTE_ADDR      )                    
#define GPIO_OC_GP30_SPI_FLASH_BYTE_REG 	    REG8_PTR(GPIO_OC_GP30_SPI_FLASH_BYTE_ADDR 	    )                    
#define GPIO_OC_GP31_PWM_BYTE_REG 		        REG8_PTR(GPIO_OC_GP31_PWM_BYTE_ADDR 		    )
#define GPIO_OC_GP60_USB_BYTE_REG				REG8_PTR(GPIO_OC_GP60_USB_BYTE_ADDR				)


#define GPIO_OD_GP0_BYTE_REG 			    	REG8_PTR(GPIO_OD_GP0_BYTE_ADDR 			        )                  
#define GPIO_OD_GP1_BYTE_REG 			        REG8_PTR(GPIO_OD_GP1_BYTE_ADDR 			        )                     
#define GPIO_OD_GP2_WAKEUP_SUS_BYTE_REG 	    REG8_PTR(GPIO_OD_GP2_WAKEUP_SUS_BYTE_ADDR 	    )                     
#define GPIO_OD_GP3_STORGE_BYTE_REG 		    REG8_PTR(GPIO_OD_GP3_STORGE_BYTE_ADDR 		    )                     
#define GPIO_OD_GP4_VDOUT_7_0_BYTE_REG          REG8_PTR(GPIO_OD_GP4_VDOUT_7_0_BYTE_ADDR        )                     
#define GPIO_OD_GP5_VDOUT_15_8_BYTE_REG         REG8_PTR(GPIO_OD_GP5_VDOUT_15_8_BYTE_ADDR       )                     
#define GPIO_OD_GP6_VDOUT_23_16_BYTE_REG        REG8_PTR(GPIO_OD_GP6_VDOUT_23_16_BYTE_ADDR      )                     
#define GPIO_OD_GP7_VD_BYTE_REG       	        REG8_PTR(GPIO_OD_GP7_VD_BYTE_ADDR       	    )                     
#define GPIO_OD_GP8_VDIN_BYTE_REG     	        REG8_PTR(GPIO_OD_GP8_VDIN_BYTE_ADDR     	    )                     
#define GPIO_OD_GP9_VSYNC_BYTE_REG 		     	REG8_PTR(GPIO_OD_GP9_VSYNC_BYTE_ADDR 		    )                     
#define GPIO_OD_GP10_I2S_BYTE_REG 		        REG8_PTR(GPIO_OD_GP10_I2S_BYTE_ADDR 		    )                     
#define GPIO_OD_GP11_SPI_BYTE_REG      	     	REG8_PTR(GPIO_OD_GP11_SPI_BYTE_ADDR      	    )                     
#define GPIO_OD_GP12_XD_BYTE_REG       	     	REG8_PTR(GPIO_OD_GP12_XD_BYTE_ADDR       	    )                     
#define GPIO_OD_GP13_XDIO_BYTE_REG              REG8_PTR(GPIO_OD_GP13_XDIO_BYTE_ADDR            )                     
#define GPIO_OD_GP14_SD_BYTE_REG       	     	REG8_PTR(GPIO_OD_GP14_SD_BYTE_ADDR       	    )                     
#define GPIO_OD_GP15_NAND_7_0_BYTE_REG          REG8_PTR(GPIO_OD_GP15_NAND_7_0_BYTE_ADDR        )                     
#define GPIO_OD_GP16_NAND_15_8_BYTE_REG         REG8_PTR(GPIO_OD_GP16_NAND_15_8_BYTE_ADDR       )                     
#define GPIO_OD_GP17_IDE_CFG_0_BYTE_REG         REG8_PTR(GPIO_OD_GP17_IDE_CFG_0_BYTE_ADDR       )                     
#define GPIO_OD_GP18_IDE_CFG_1_BYTE_REG         REG8_PTR(GPIO_OD_GP18_IDE_CFG_1_BYTE_ADDR       )                     
#define GPIO_OD_GP19_IDED_7_0_BYTE_REG          REG8_PTR(GPIO_OD_GP19_IDED_7_0_BYTE_ADDR        )                     
#define GPIO_OD_GP20_IDED_15_8_BYTE_REG 	    REG8_PTR(GPIO_OD_GP20_IDED_15_8_BYTE_ADDR 	    )                     
#define GPIO_OD_GP21_I2C_BYTE_REG       	    REG8_PTR(GPIO_OD_GP21_I2C_BYTE_ADDR       	    )                     
#define GPIO_OD_GP22_UART_0_1_BYTE_REG          REG8_PTR(GPIO_OD_GP22_UART_0_1_BYTE_ADDR        )                     
#define GPIO_OD_GP23_UART_2_3_BYTE_REG          REG8_PTR(GPIO_OD_GP23_UART_2_3_BYTE_ADDR        )                     
#define GPIO_OD_GP24_TSDIN_7_0_BYTE_REG         REG8_PTR(GPIO_OD_GP24_TSDIN_7_0_BYTE_ADDR       )                     
#define GPIO_OD_GP25_TS_BYTE_REG       	     	REG8_PTR(GPIO_OD_GP25_TS_BYTE_ADDR       	    )                     
#define GPIO_OD_GP26_KPAD_BYTE_REG              REG8_PTR(GPIO_OD_GP26_KPAD_BYTE_ADDR            )                     
#define GPIO_OD_GP27_SPDIF_BYTE_REG             REG8_PTR(GPIO_OD_GP27_SPDIF_BYTE_ADDR           )                     
#define GPIO_OD_GP28_NAND_CFG_0_BYTE_REG        REG8_PTR(GPIO_OD_GP28_NAND_CFG_0_BYTE_ADDR      )                     
#define GPIO_OD_GP29_NAND_CFG_1_BYTE_REG        REG8_PTR(GPIO_OD_GP29_NAND_CFG_1_BYTE_ADDR      )                     
#define GPIO_OD_GP30_SPI_FLASH_BYTE_REG 	    REG8_PTR(GPIO_OD_GP30_SPI_FLASH_BYTE_ADDR 	    )                     
#define GPIO_OD_GP31_PWM_BYTE_REG 		        REG8_PTR(GPIO_OD_GP31_PWM_BYTE_ADDR 		    )	                     
#define GPIO_OD_GP60_USB_BYTE_REG				REG8_PTR(GPIO_OD_GP60_USB_BYTE_ADDR				)                     


#define GPIO_STRAP_STATUS_REG					REG32_PTR(GPIO_STRAP_STATUS_ADDR 				    )
#define GPIO_RING_OSC_EXIT_STS_4BYTE_REG 		REG32_PTR(GPIO_RING_OSC_EXIT_STS_4BYTE_ADDR			)
#define GPIO_AHB_CTRL_4BYTE_REG 				REG32_PTR(GPIO_AHB_CTRL_4BYTE_ADDR 				    )
#define GPIO_USB_OP_CTRL_4BYTE_ADDR_REG			REG32_PTR(GPIO_USB_OP_CTRL_4BYTE_ADDR				)
#define GPIO_BONDING_OPTION_4BYTE_REG     		REG32_PTR(GPIO_BONDING_OPTION_4BYTE_ADDR     		)
#define GPIO_PIN_SHARING_SEL_4BYTE_REG 			REG32_PTR(GPIO_PIN_SHARING_SEL_4BYTE_ADDR 		    )

#define GPIO_INT_REQ_TYPE_0_REG					REG32_PTR(GPIO_INT_REQ_TYPE_0_ADDR					)
#define GPIO_UART_INT_REQ_TYPE_REG 				REG32_PTR(GPIO_UART_INT_REQ_TYPE_ADDR 			    )
#define GPIO_I2C_INT_REQ_TYPE_REG 				REG32_PTR(GPIO_I2C_INT_REQ_TYPE_ADDR 			    )
#define GPIO_I2S_KAPD_INT_REQ_TYPE_REG			REG32_PTR(GPIO_I2S_KAPD_INT_REQ_TYPE_ADDR			)
#define GPIO_KAPD_INT_REQ_TYPE_REG				REG32_PTR(GPIO_KAPD_INT_REQ_TYPE_ADDR				)
#define GPIO_PWM_INT_REQ_TYPE_REG 				REG32_PTR(GPIO_PWM_INT_REQ_TYPE_ADDR 			    )
#define GPIO_INT_REQ_TYPE_1_REG 				REG32_PTR(GPIO_INT_REQ_TYPE_1_ADDR					)

#define GPIO_INT_REQ_STS_REG					REG32_PTR(GPIO_INT_REQ_STS_ADDR						)

#define GPIO_IO_DRV_GPIO1_4BYTE_REG				REG32_PTR(GPIO_IO_DRV_GPIO1_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_SD_4BYTE_REG				REG32_PTR(GPIO_IO_DRV_SD_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_VID_4BYTE_REG 				REG32_PTR(GPIO_IO_DRV_VID_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_SF_4BYTE_REG 				REG32_PTR(GPIO_IO_DRV_SF_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_NAND_4BYTE_REG				REG32_PTR(GPIO_IO_DRV_NAND_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_SPI_4BYTE_REG 				REG32_PTR(GPIO_IO_DRV_SPI_4BYTE_ADDR 	    )

#define GPIO_PULL_EN_GP0_BYTE_REG 			        REG8_PTR(GPIO_PULL_EN_GP0_BYTE_ADDR 			    )
#define GPIO_PULL_EN_GP1_BYTE_REG 			        REG8_PTR(GPIO_PULL_EN_GP1_BYTE_ADDR 			    )    
#define GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_REG 	    REG8_PTR(GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR 	    )    
#define GPIO_PULL_EN_GP3_STORGE_BYTE_REG 		    REG8_PTR(GPIO_PULL_EN_GP3_STORGE_BYTE_ADDR 		    )    
#define GPIO_PULL_EN_GP4_VDOUT_7_0_BYTE_REG       	REG8_PTR(GPIO_PULL_EN_GP4_VDOUT_7_0_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP5_VDOUT_15_8_BYTE_REG       	REG8_PTR(GPIO_PULL_EN_GP5_VDOUT_15_8_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP6_VDOUT_23_16_BYTE_REG      	REG8_PTR(GPIO_PULL_EN_GP6_VDOUT_23_16_BYTE_ADDR     )    
#define GPIO_PULL_EN_GP7_VD_BYTE_REG       	    	REG8_PTR(GPIO_PULL_EN_GP7_VD_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP8_VDIN_BYTE_REG     	    	REG8_PTR(GPIO_PULL_EN_GP8_VDIN_BYTE_ADDR     	    )    
#define GPIO_PULL_EN_GP9_VSYNC_BYTE_REG 		    REG8_PTR(GPIO_PULL_EN_GP9_VSYNC_BYTE_ADDR 		    )    
#define GPIO_PULL_EN_GP10_I2S_BYTE_REG 		    	REG8_PTR(GPIO_PULL_EN_GP10_I2S_BYTE_ADDR 		    )    
#define GPIO_PULL_EN_GP11_SPI_BYTE_REG      	    REG8_PTR(GPIO_PULL_EN_GP11_SPI_BYTE_ADDR      	    )    
#define GPIO_PULL_EN_GP12_XD_BYTE_REG       	    REG8_PTR(GPIO_PULL_EN_GP12_XD_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP13_XDIO_BYTE_REG            	REG8_PTR(GPIO_PULL_EN_GP13_XDIO_BYTE_ADDR           )    
#define GPIO_PULL_EN_GP14_SD_BYTE_REG       	    REG8_PTR(GPIO_PULL_EN_GP14_SD_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP15_NAND_7_0_BYTE_REG        	REG8_PTR(GPIO_PULL_EN_GP15_NAND_7_0_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP16_NAND_15_8_BYTE_REG       	REG8_PTR(GPIO_PULL_EN_GP16_NAND_15_8_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP17_IDE_CFG_0_BYTE_REG       	REG8_PTR(GPIO_PULL_EN_GP17_IDE_CFG_0_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP18_IDE_CFG_1_BYTE_REG       	REG8_PTR(GPIO_PULL_EN_GP18_IDE_CFG_1_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP19_IDED_7_0_BYTE_REG        	REG8_PTR(GPIO_PULL_EN_GP19_IDED_7_0_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP20_IDED_15_8_BYTE_REG 	    REG8_PTR(GPIO_PULL_EN_GP20_IDED_15_8_BYTE_ADDR 	    )    
#define GPIO_PULL_EN_GP21_I2C_BYTE_REG       	    REG8_PTR(GPIO_PULL_EN_GP21_I2C_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP22_UART_0_1_BYTE_REG        	REG8_PTR(GPIO_PULL_EN_GP22_UART_0_1_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP23_UART_2_3_BYTE_REG        	REG8_PTR(GPIO_PULL_EN_GP23_UART_2_3_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP24_TSDIN_7_0_BYTE_REG       	REG8_PTR(GPIO_PULL_EN_GP24_TSDIN_7_0_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP25_TS_BYTE_REG       	    REG8_PTR(GPIO_PULL_EN_GP25_TS_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP26_KPAD_BYTE_REG             REG8_PTR(GPIO_PULL_EN_GP26_KPAD_BYTE_ADDR           )    
#define GPIO_PULL_EN_GP27_SPDIF_BYTE_REG            REG8_PTR(GPIO_PULL_EN_GP27_SPDIF_BYTE_ADDR          )    
#define GPIO_PULL_EN_GP28_NAND_CFG_0_BYTE_REG       REG8_PTR(GPIO_PULL_EN_GP28_NAND_CFG_0_BYTE_ADDR     )    
#define GPIO_PULL_EN_GP29_NAND_CFG_1_BYTE_REG       REG8_PTR(GPIO_PULL_EN_GP29_NAND_CFG_1_BYTE_ADDR     )    
#define GPIO_PULL_EN_GP30_SPI_FLASH_BYTE_REG 	    REG8_PTR(GPIO_PULL_EN_GP30_SPI_FLASH_BYTE_ADDR 	    )    
#define GPIO_PULL_EN_GP31_PWM_BYTE_REG 		     	REG8_PTR(GPIO_PULL_EN_GP31_PWM_BYTE_ADDR 		    )        
#define GPIO_PULL_EN_GP60_USB_TYTE_REG				REG8_PTR(GPIO_PULL_EN_GP60_USB_TYTE_ADDR			)    

#define GPIO_PULL_CTRL_GP0_BYTE_REG 			    REG8_PTR(GPIO_PULL_CTRL_GP0_BYTE_ADDR 			    )
#define GPIO_PULL_CTRL_GP1_BYTE_REG 			    REG8_PTR(GPIO_PULL_CTRL_GP1_BYTE_ADDR 			    )    
#define GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_REG 	 	REG8_PTR(GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP3_STORGE_BYTE_REG 		  	REG8_PTR(GPIO_PULL_CTRL_GP3_STORGE_BYTE_ADDR 	    )    
#define GPIO_PULL_CTRL_GP4_VDOUT_7_0_BYTE_REG       REG8_PTR(GPIO_PULL_CTRL_GP4_VDOUT_7_0_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP5_VDOUT_15_8_BYTE_REG      REG8_PTR(GPIO_PULL_CTRL_GP5_VDOUT_15_8_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP6_VDOUT_23_16_BYTE_REG     REG8_PTR(GPIO_PULL_CTRL_GP6_VDOUT_23_16_BYTE_ADDR   )    
#define GPIO_PULL_CTRL_GP7_VD_BYTE_REG       	    REG8_PTR(GPIO_PULL_CTRL_GP7_VD_BYTE_ADDR       	    )    
#define GPIO_PULL_CTRL_GP8_VDIN_BYTE_REG     	    REG8_PTR(GPIO_PULL_CTRL_GP8_VDIN_BYTE_ADDR     	    )    
#define GPIO_PULL_CTRL_GP9_VSYNC_BYTE_REG 		    REG8_PTR(GPIO_PULL_CTRL_GP9_VSYNC_BYTE_ADDR 		)    
#define GPIO_PULL_CTRL_GP10_I2S_BYTE_REG 		    REG8_PTR(GPIO_PULL_CTRL_GP10_I2S_BYTE_ADDR 		    )    
#define GPIO_PULL_CTRL_GP11_SPI_BYTE_REG      	    REG8_PTR(GPIO_PULL_CTRL_GP11_SPI_BYTE_ADDR      	)    
#define GPIO_PULL_CTRL_GP12_XD_BYTE_REG       	    REG8_PTR(GPIO_PULL_CTRL_GP12_XD_BYTE_ADDR       	)    
#define GPIO_PULL_CTRL_GP13_XDIO_BYTE_REG           REG8_PTR(GPIO_PULL_CTRL_GP13_XDIO_BYTE_ADDR         )    
#define GPIO_PULL_CTRL_GP14_SD_BYTE_REG       	    REG8_PTR(GPIO_PULL_CTRL_GP14_SD_BYTE_ADDR       	)    
#define GPIO_PULL_CTRL_GP15_NAND_7_0_BYTE_REG       REG8_PTR(GPIO_PULL_CTRL_GP15_NAND_7_0_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP16_NAND_15_8_BYTE_REG      REG8_PTR(GPIO_PULL_CTRL_GP16_NAND_15_8_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP17_IDE_CFG_0_BYTE_REG      REG8_PTR(GPIO_PULL_CTRL_GP17_IDE_CFG_0_BYTE_ADDR    )     
#define GPIO_PULL_CTRL_GP18_IDE_CFG_1_BYTE_REG      REG8_PTR(GPIO_PULL_CTRL_GP18_IDE_CFG_1_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP19_IDED_7_0_BYTE_REG       REG8_PTR(GPIO_PULL_CTRL_GP19_IDED_7_0_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP20_IDED_15_8_BYTE_REG 	    REG8_PTR(GPIO_PULL_CTRL_GP20_IDED_15_8_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP21_I2C_BYTE_REG       	    REG8_PTR(GPIO_PULL_CTRL_GP21_I2C_BYTE_ADDR          )    
#define GPIO_PULL_CTRL_GP22_UART_0_1_BYTE_REG       REG8_PTR(GPIO_PULL_CTRL_GP22_UART_0_1_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP23_UART_2_3_BYTE_REG       REG8_PTR(GPIO_PULL_CTRL_GP23_UART_2_3_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP24_TSDIN_7_0_BYTE_REG      REG8_PTR(GPIO_PULL_CTRL_GP24_TSDIN_7_0_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP25_TS_BYTE_REG       	    REG8_PTR(GPIO_PULL_CTRL_GP25_TS_BYTE_ADDR       	)    
#define GPIO_PULL_CTRL_GP26_KPAD_BYTE_REG           REG8_PTR(GPIO_PULL_CTRL_GP26_KPAD_BYTE_ADDR         )    
#define GPIO_PULL_CTRL_GP27_SPDIF_BYTE_REG          REG8_PTR(GPIO_PULL_CTRL_GP27_SPDIF_BYTE_ADDR        )    
#define GPIO_PULL_CTRL_GP28_NAND_CFG_0_BYTE_REG     REG8_PTR(GPIO_PULL_CTRL_GP28_NAND_CFG_0_BYTE_ADDR   )    
#define GPIO_PULL_CTRL_GP29_NAND_CFG_1_BYTE_REG     REG8_PTR(GPIO_PULL_CTRL_GP29_NAND_CFG_1_BYTE_ADDR   )    
#define GPIO_PULL_CTRL_GP30_SPI_FLASH_BYTE_REG 	    REG8_PTR(GPIO_PULL_CTRL_GP30_SPI_FLASH_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP31_PWM_BYTE_REG 		    REG8_PTR(GPIO_PULL_CTRL_GP31_PWM_BYTE_ADDR 		    )        
#define GPIO_PULL_CTRL_GP60_USB_BYTE_REG			REG8_PTR(GPIO_PULL_CTRL_GP60_USB_BYTE_ADDR			)    


#define GPIO_ID_GP0_BYTE_VAL 					REG8_VAL(GPIO_ID_GP0_BYTE_ADDR 				    )
#define GPIO_ID_GP1_BYTE_VAL 					REG8_VAL(GPIO_ID_GP1_BYTE_ADDR 				    )
#define GPIO_ID_GP2_WAKEUP_SUS_BYTE_VAL 		REG8_VAL(GPIO_ID_GP2_WAKEUP_SUS_BYTE_ADDR 	    )
#define GPIO_ID_GP3_STORGE_BYTE_VAL 			REG8_VAL(GPIO_ID_GP3_STORGE_BYTE_ADDR 		    )
#define GPIO_ID_GP4_VDOUT_7_0_BYTE_VAL    	    REG8_VAL(GPIO_ID_GP4_VDOUT_7_0_BYTE_ADDR    	)
#define GPIO_ID_GP5_VDOUT_15_8_BYTE_VAL        	REG8_VAL(GPIO_ID_GP5_VDOUT_15_8_BYTE_ADDR       )
#define GPIO_ID_GP6_VDOUT_23_16_BYTE_VAL 		REG8_VAL(GPIO_ID_GP6_VDOUT_23_16_BYTE_ADDR 	    )
#define GPIO_ID_GP7_VD_BYTE_VAL       			REG8_VAL(GPIO_ID_GP7_VD_BYTE_ADDR       		)
#define GPIO_ID_GP8_VDIN_BYTE_VAL     			REG8_VAL(GPIO_ID_GP8_VDIN_BYTE_ADDR     		)
#define GPIO_ID_GP9_VSYNC_BYTE_VAL 				REG8_VAL(GPIO_ID_GP9_VSYNC_BYTE_ADDR 		    )
#define GPIO_ID_GP10_I2S_BYTE_VAL 				REG8_VAL(GPIO_ID_GP10_I2S_BYTE_ADDR 			)
#define GPIO_ID_GP11_SPI_BYTE_VAL      			REG8_VAL(GPIO_ID_GP11_SPI_BYTE_ADDR      	    )
#define GPIO_ID_GP12_XD_BYTE_VAL       			REG8_VAL(GPIO_ID_GP12_XD_BYTE_ADDR       	    )
#define GPIO_ID_GP13_XDIO_BYTE_VAL       		REG8_VAL(GPIO_ID_GP13_XDIO_BYTE_ADDR       	    )
#define GPIO_ID_GP14_SD_BYTE_VAL       			REG8_VAL(GPIO_ID_GP14_SD_BYTE_ADDR       	    )
#define GPIO_ID_GP15_NAND_7_0_BYTE_VAL       	REG8_VAL(GPIO_ID_GP15_NAND_7_0_BYTE_ADDR        )
#define GPIO_ID_GP16_NAND_15_8_BYTE_VAL       	REG8_VAL(GPIO_ID_GP16_NAND_15_8_BYTE_ADDR       )
#define GPIO_ID_GP17_IDE_CFG_0_BYTE_VAL       	REG8_VAL(GPIO_ID_GP17_IDE_CFG_0_BYTE_ADDR       )
#define GPIO_ID_GP18_IDE_CFG_1_BYTE_VAL       	REG8_VAL(GPIO_ID_GP18_IDE_CFG_1_BYTE_ADDR       )
#define GPIO_ID_GP19_IDED_7_0_BYTE_VAL       	REG8_VAL(GPIO_ID_GP19_IDED_7_0_BYTE_ADDR        )
#define GPIO_ID_GP20_IDED_15_8_BYTE_VAL 		REG8_VAL(GPIO_ID_GP20_IDED_15_8_BYTE_ADDR 	    )
#define GPIO_ID_GP21_I2C_BYTE_VAL       		REG8_VAL(GPIO_ID_GP21_I2C_BYTE_ADDR       	    )
#define GPIO_ID_GP22_UART_0_1_BYTE_VAL       	REG8_VAL(GPIO_ID_GP22_UART_0_1_BYTE_ADDR        )
#define GPIO_ID_GP23_UART_2_3_BYTE_VAL       	REG8_VAL(GPIO_ID_GP23_UART_2_3_BYTE_ADDR        )
#define GPIO_ID_GP24_TSDIN_7_0_BYTE_VAL       	REG8_VAL(GPIO_ID_GP24_TSDIN_7_0_BYTE_ADDR       )
#define GPIO_ID_GP25_TS_BYTE_VAL       			REG8_VAL(GPIO_ID_GP25_TS_BYTE_ADDR       	    )
#define GPIO_ID_GP26_KPAD_BYTE_VAL       		REG8_VAL(GPIO_ID_GP26_KPAD_BYTE_ADDR       	    )
#define GPIO_ID_GP27_SPDIF_BYTE_VAL       		REG8_VAL(GPIO_ID_GP27_SPDIF_BYTE_ADDR       	)
#define GPIO_ID_GP28_NAND_CFG_0_BYTE_VAL   	    REG8_VAL(GPIO_ID_GP28_NAND_CFG_0_BYTE_ADDR      )
#define GPIO_ID_GP29_NAND_CFG_1_BYTE_VAL        REG8_VAL(GPIO_ID_GP29_NAND_CFG_1_BYTE_ADDR      )
#define GPIO_ID_GP30_SPI_FLASH_BYTE_VAL 		REG8_VAL(GPIO_ID_GP30_SPI_FLASH_BYTE_ADDR 	    )
#define GPIO_ID_GP31_PWM_BYTE_VAL 				REG8_VAL(GPIO_ID_GP31_PWM_BYTE_ADDR 			)
#define GPIO_ID_GP60_BYTE_VAL 					REG8_VAL(GPIO_ID_GP60_BYTE_ADDR 				)
#define GPIO_CTRL_GP0_BYTE_VAL 					REG8_VAL(GPIO_CTRL_GP0_BYTE_ADDR 			    )  
#define GPIO_CTRL_GP1_BYTE_VAL 					REG8_VAL(GPIO_CTRL_GP1_BYTE_ADDR 			    )              
#define GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_VAL 	    REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR 	)              
#define GPIO_CTRL_GP3_STORGE_BYTE_VAL 		    REG8_VAL(GPIO_CTRL_GP3_STORGE_BYTE_ADDR 		)              
#define GPIO_CTRL_GP4_VDOUT_7_0_BYTE_VAL    	REG8_VAL(GPIO_CTRL_GP4_VDOUT_7_0_BYTE_ADDR      )              
#define GPIO_CTRL_GP5_VDOUT_15_8_BYTE_VAL      	REG8_VAL(GPIO_CTRL_GP5_VDOUT_15_8_BYTE_ADDR     )              
#define GPIO_CTRL_GP6_VDOUT_23_16_BYTE_VAL 		REG8_VAL(GPIO_CTRL_GP6_VDOUT_23_16_BYTE_ADDR    )              
#define GPIO_CTRL_GP7_VD_BYTE_VAL       		REG8_VAL(GPIO_CTRL_GP7_VD_BYTE_ADDR       	    )              
#define GPIO_CTRL_GP8_VDIN_BYTE_VAL     		REG8_VAL(GPIO_CTRL_GP8_VDIN_BYTE_ADDR     	    )              
#define GPIO_CTRL_GP9_VSYNC_BYTE_VAL 		    REG8_VAL(GPIO_CTRL_GP9_VSYNC_BYTE_ADDR 		    )              
#define GPIO_CTRL_GP10_I2S_BYTE_VAL 			REG8_VAL(GPIO_CTRL_GP10_I2S_BYTE_ADDR 		    )              
#define GPIO_CTRL_GP11_SPI_BYTE_VAL      	    REG8_VAL(GPIO_CTRL_GP11_SPI_BYTE_ADDR      	    )              
#define GPIO_CTRL_GP12_XD_BYTE_VAL       	    REG8_VAL(GPIO_CTRL_GP12_XD_BYTE_ADDR       	    )              
#define GPIO_CTRL_GP13_XDIO_BYTE_VAL       		REG8_VAL(GPIO_CTRL_GP13_XDIO_BYTE_ADDR          )              
#define GPIO_CTRL_GP14_SD_BYTE_VAL       	    REG8_VAL(GPIO_CTRL_GP14_SD_BYTE_ADDR       	    )              
#define GPIO_CTRL_GP15_NAND_7_0_BYTE_VAL        REG8_VAL(GPIO_CTRL_GP15_NAND_7_0_BYTE_ADDR      )              
#define GPIO_CTRL_GP16_NAND_15_8_BYTE_VAL       REG8_VAL(GPIO_CTRL_GP16_NAND_15_8_BYTE_ADDR     )             
#define GPIO_CTRL_GP17_IDE_CFG_0_BYTE_VAL       REG8_VAL(GPIO_CTRL_GP17_IDE_CFG_0_BYTE_ADDR     )             
#define GPIO_CTRL_GP18_IDE_CFG_1_BYTE_VAL       REG8_VAL(GPIO_CTRL_GP18_IDE_CFG_1_BYTE_ADDR     )             
#define GPIO_CTRL_GP19_IDED_7_0_BYTE_VAL        REG8_VAL(GPIO_CTRL_GP19_IDED_7_0_BYTE_ADDR      )             
#define GPIO_CTRL_GP20_IDED_15_8_BYTE_VAL 	    REG8_VAL(GPIO_CTRL_GP20_IDED_15_8_BYTE_ADDR 	)             
#define GPIO_CTRL_GP21_I2C_BYTE_VAL       	    REG8_VAL(GPIO_CTRL_GP21_I2C_BYTE_ADDR       	)             
#define GPIO_CTRL_GP22_UART_0_1_BYTE_VAL        REG8_VAL(GPIO_CTRL_GP22_UART_0_1_BYTE_ADDR      )             
#define GPIO_CTRL_GP23_UART_2_3_BYTE_VAL        REG8_VAL(GPIO_CTRL_GP23_UART_2_3_BYTE_ADDR      )             
#define GPIO_CTRL_GP24_TSDIN_7_0_BYTE_VAL       REG8_VAL(GPIO_CTRL_GP24_TSDIN_7_0_BYTE_ADDR     )             
#define GPIO_CTRL_GP25_TS_BYTE_VAL       	    REG8_VAL(GPIO_CTRL_GP25_TS_BYTE_ADDR       	    )             
#define GPIO_CTRL_GP26_KPAD_BYTE_VAL       	 	REG8_VAL(GPIO_CTRL_GP26_KPAD_BYTE_ADDR          )             
#define GPIO_CTRL_GP27_SPDIF_BYTE_VAL       	REG8_VAL(GPIO_CTRL_GP27_SPDIF_BYTE_ADDR         )             
#define GPIO_CTRL_GP28_NAND_CFG_0_BYTE_VAL      REG8_VAL(GPIO_CTRL_GP28_NAND_CFG_0_BYTE_ADDR    )             
#define GPIO_CTRL_GP29_NAND_CFG_1_BYTE_VAL      REG8_VAL(GPIO_CTRL_GP29_NAND_CFG_1_BYTE_ADDR    )             
#define GPIO_CTRL_GP30_SPI_FLASH_BYTE_VAL 	    REG8_VAL(GPIO_CTRL_GP30_SPI_FLASH_BYTE_ADDR 	)             
#define GPIO_CTRL_GP31_PWM_BYTE_VAL 			REG8_VAL(GPIO_CTRL_GP31_PWM_BYTE_ADDR 		    )	              
                     
#define GPIO_CTRL_GP60_USB_BYTE_VAL				REG8_VAL(GPIO_CTRL_GP60_USB_BYTE_ADDR			)
                                          
#define GPIO_OC_GP0_BYTE_VAL 			    	REG8_VAL(GPIO_OC_GP0_BYTE_ADDR 			        )                
#define GPIO_OC_GP1_BYTE_VAL 			        REG8_VAL(GPIO_OC_GP1_BYTE_ADDR 			        )                    
#define GPIO_OC_GP2_WAKEUP_SUS_BYTE_VAL 	    REG8_VAL(GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR 	    )                    
#define GPIO_OC_GP3_STORGE_BYTE_VAL 		    REG8_VAL(GPIO_OC_GP3_STORGE_BYTE_ADDR 		    )                    
#define GPIO_OC_GP4_VDOUT_7_0_BYTE_VAL          REG8_VAL(GPIO_OC_GP4_VDOUT_7_0_BYTE_ADDR        )                    
#define GPIO_OC_GP5_VDOUT_15_8_BYTE_VAL         REG8_VAL(GPIO_OC_GP5_VDOUT_15_8_BYTE_ADDR       )                    
#define GPIO_OC_GP6_VDOUT_23_16_BYTE_VAL        REG8_VAL(GPIO_OC_GP6_VDOUT_23_16_BYTE_ADDR      )                    
#define GPIO_OC_GP7_VD_BYTE_VAL       	        REG8_VAL(GPIO_OC_GP7_VD_BYTE_ADDR       	    )                    
#define GPIO_OC_GP8_VDIN_BYTE_VAL     	        REG8_VAL(GPIO_OC_GP8_VDIN_BYTE_ADDR     	    )                    
#define GPIO_OC_GP9_VSYNC_BYTE_VAL 		     	REG8_VAL(GPIO_OC_GP9_VSYNC_BYTE_ADDR 		    )                    
#define GPIO_OC_GP10_I2S_BYTE_VAL 		        REG8_VAL(GPIO_OC_GP10_I2S_BYTE_ADDR 		    )                    
#define GPIO_OC_GP11_SPI_BYTE_VAL      	     	REG8_VAL(GPIO_OC_GP11_SPI_BYTE_ADDR      	    )                    
#define GPIO_OC_GP12_XD_BYTE_VAL       	     	REG8_VAL(GPIO_OC_GP12_XD_BYTE_ADDR       	    )                    
#define GPIO_OC_GP13_XDIO_BYTE_VAL              REG8_VAL(GPIO_OC_GP13_XDIO_BYTE_ADDR            )                    
#define GPIO_OC_GP14_SD_BYTE_VAL       	     	REG8_VAL(GPIO_OC_GP14_SD_BYTE_ADDR       	    )                    
#define GPIO_OC_GP15_NAND_7_0_BYTE_VAL          REG8_VAL(GPIO_OC_GP15_NAND_7_0_BYTE_ADDR        )                    
#define GPIO_OC_GP16_NAND_15_8_BYTE_VAL         REG8_VAL(GPIO_OC_GP16_NAND_15_8_BYTE_ADDR       )                    
#define GPIO_OC_GP17_IDE_CFG_0_BYTE_VAL         REG8_VAL(GPIO_OC_GP17_IDE_CFG_0_BYTE_ADDR       )                    
#define GPIO_OC_GP18_IDE_CFG_1_BYTE_VAL         REG8_VAL(GPIO_OC_GP18_IDE_CFG_1_BYTE_ADDR       )                    
#define GPIO_OC_GP19_IDED_7_0_BYTE_VAL          REG8_VAL(GPIO_OC_GP19_IDED_7_0_BYTE_ADDR        )                    
#define GPIO_OC_GP20_IDED_15_8_BYTE_VAL 	    REG8_VAL(GPIO_OC_GP20_IDED_15_8_BYTE_ADDR 	    )                    
#define GPIO_OC_GP21_I2C_BYTE_VAL       	    REG8_VAL(GPIO_OC_GP21_I2C_BYTE_ADDR       	    )                    
#define GPIO_OC_GP22_UART_0_1_BYTE_VAL          REG8_VAL(GPIO_OC_GP22_UART_0_1_BYTE_ADDR        )                    
#define GPIO_OC_GP23_UART_2_3_BYTE_VAL          REG8_VAL(GPIO_OC_GP23_UART_2_3_BYTE_ADDR        )                    
#define GPIO_OC_GP24_TSDIN_7_0_BYTE_VAL         REG8_VAL(GPIO_OC_GP24_TSDIN_7_0_BYTE_ADDR       )                    
#define GPIO_OC_GP25_TS_BYTE_VAL       	     	REG8_VAL(GPIO_OC_GP25_TS_BYTE_ADDR       	    )                    
#define GPIO_OC_GP26_KPAD_BYTE_VAL              REG8_VAL(GPIO_OC_GP26_KPAD_BYTE_ADDR            )                    
#define GPIO_OC_GP27_SPDIF_BYTE_VAL             REG8_VAL(GPIO_OC_GP27_SPDIF_BYTE_ADDR           )                    
#define GPIO_OC_GP28_NAND_CFG_0_BYTE_VAL        REG8_VAL(GPIO_OC_GP28_NAND_CFG_0_BYTE_ADDR      )                    
#define GPIO_OC_GP29_NAND_CFG_1_BYTE_VAL        REG8_VAL(GPIO_OC_GP29_NAND_CFG_1_BYTE_ADDR      )                    
#define GPIO_OC_GP30_SPI_FLASH_BYTE_VAL 	    REG8_VAL(GPIO_OC_GP30_SPI_FLASH_BYTE_ADDR 	    )                    
#define GPIO_OC_GP31_PWM_BYTE_VAL 		        REG8_VAL(GPIO_OC_GP31_PWM_BYTE_ADDR 		    )	                     
#define GPIO_OC_GP60_USB_BYTE_VAL				REG8_VAL(GPIO_OC_GP60_USB_BYTE_ADDR				)                     


#define GPIO_OD_GP0_BYTE_VAL 			    	REG8_VAL(GPIO_OD_GP0_BYTE_ADDR 			        )                  
#define GPIO_OD_GP1_BYTE_VAL 			        REG8_VAL(GPIO_OD_GP1_BYTE_ADDR 			        )                     
#define GPIO_OD_GP2_WAKEUP_SUS_BYTE_VAL 	    REG8_VAL(GPIO_OD_GP2_WAKEUP_SUS_BYTE_ADDR 	    )                     
#define GPIO_OD_GP3_STORGE_BYTE_VAL 		    REG8_VAL(GPIO_OD_GP3_STORGE_BYTE_ADDR 		    )                     
#define GPIO_OD_GP4_VDOUT_7_0_BYTE_VAL          REG8_VAL(GPIO_OD_GP4_VDOUT_7_0_BYTE_ADDR        )                     
#define GPIO_OD_GP5_VDOUT_15_8_BYTE_VAL         REG8_VAL(GPIO_OD_GP5_VDOUT_15_8_BYTE_ADDR       )                     
#define GPIO_OD_GP6_VDOUT_23_16_BYTE_VAL        REG8_VAL(GPIO_OD_GP6_VDOUT_23_16_BYTE_ADDR      )                     
#define GPIO_OD_GP7_VD_BYTE_VAL       	        REG8_VAL(GPIO_OD_GP7_VD_BYTE_ADDR       	    )                     
#define GPIO_OD_GP8_VDIN_BYTE_VAL     	        REG8_VAL(GPIO_OD_GP8_VDIN_BYTE_ADDR     	    )                     
#define GPIO_OD_GP9_VSYNC_BYTE_VAL 		     	REG8_VAL(GPIO_OD_GP9_VSYNC_BYTE_ADDR 		    )                     
#define GPIO_OD_GP10_I2S_BYTE_VAL 		        REG8_VAL(GPIO_OD_GP10_I2S_BYTE_ADDR 		    )                     
#define GPIO_OD_GP11_SPI_BYTE_VAL      	     	REG8_VAL(GPIO_OD_GP11_SPI_BYTE_ADDR      	    )                     
#define GPIO_OD_GP12_XD_BYTE_VAL       	     	REG8_VAL(GPIO_OD_GP12_XD_BYTE_ADDR       	    )                     
#define GPIO_OD_GP13_XDIO_BYTE_VAL              REG8_VAL(GPIO_OD_GP13_XDIO_BYTE_ADDR            )                     
#define GPIO_OD_GP14_SD_BYTE_VAL       	     	REG8_VAL(GPIO_OD_GP14_SD_BYTE_ADDR       	    )                     
#define GPIO_OD_GP15_NAND_7_0_BYTE_VAL          REG8_VAL(GPIO_OD_GP15_NAND_7_0_BYTE_ADDR        )                     
#define GPIO_OD_GP16_NAND_15_8_BYTE_VAL         REG8_VAL(GPIO_OD_GP16_NAND_15_8_BYTE_ADDR       )                     
#define GPIO_OD_GP17_IDE_CFG_0_BYTE_VAL         REG8_VAL(GPIO_OD_GP17_IDE_CFG_0_BYTE_ADDR       )                     
#define GPIO_OD_GP18_IDE_CFG_1_BYTE_VAL         REG8_VAL(GPIO_OD_GP18_IDE_CFG_1_BYTE_ADDR       )                     
#define GPIO_OD_GP19_IDED_7_0_BYTE_VAL          REG8_VAL(GPIO_OD_GP19_IDED_7_0_BYTE_ADDR        )                     
#define GPIO_OD_GP20_IDED_15_8_BYTE_VAL 	    REG8_VAL(GPIO_OD_GP20_IDED_15_8_BYTE_ADDR 	    )                     
#define GPIO_OD_GP21_I2C_BYTE_VAL       	    REG8_VAL(GPIO_OD_GP21_I2C_BYTE_ADDR       	    )                     
#define GPIO_OD_GP22_UART_0_1_BYTE_VAL          REG8_VAL(GPIO_OD_GP22_UART_0_1_BYTE_ADDR        )                     
#define GPIO_OD_GP23_UART_2_3_BYTE_VAL          REG8_VAL(GPIO_OD_GP23_UART_2_3_BYTE_ADDR        )                     
#define GPIO_OD_GP24_TSDIN_7_0_BYTE_VAL         REG8_VAL(GPIO_OD_GP24_TSDIN_7_0_BYTE_ADDR       )                     
#define GPIO_OD_GP25_TS_BYTE_VAL       	     	REG8_VAL(GPIO_OD_GP25_TS_BYTE_ADDR       	    )                     
#define GPIO_OD_GP26_KPAD_BYTE_VAL              REG8_VAL(GPIO_OD_GP26_KPAD_BYTE_ADDR            )                     
#define GPIO_OD_GP27_SPDIF_BYTE_VAL             REG8_VAL(GPIO_OD_GP27_SPDIF_BYTE_ADDR           )                     
#define GPIO_OD_GP28_NAND_CFG_0_BYTE_VAL        REG8_VAL(GPIO_OD_GP28_NAND_CFG_0_BYTE_ADDR      )                     
#define GPIO_OD_GP29_NAND_CFG_1_BYTE_VAL        REG8_VAL(GPIO_OD_GP29_NAND_CFG_1_BYTE_ADDR      )                     
#define GPIO_OD_GP30_SPI_FLASH_BYTE_VAL 	    REG8_VAL(GPIO_OD_GP30_SPI_FLASH_BYTE_ADDR 	    )                     
#define GPIO_OD_GP31_PWM_BYTE_VAL 		        REG8_VAL(GPIO_OD_GP31_PWM_BYTE_ADDR 		    )	                     
#define GPIO_OD_GP60_USB_BYTE_VAL				REG8_VAL(GPIO_OD_GP60_USB_BYTE_ADDR				)                     


#define GPIO_STRAP_STATUS_VAL					REG32_VAL(GPIO_STRAP_STATUS_ADDR 				    )
#define GPIO_RING_OSC_EXIT_STS_4BYTE_VAL 		REG32_VAL(GPIO_RING_OSC_EXIT_STS_4BYTE_ADDR			)//fan
#define GPIO_AHB_CTRL_4BYTE_VAL 				REG32_VAL(GPIO_AHB_CTRL_4BYTE_ADDR 				    )
#define GPIO_USB_OP_CTRL_4BYTE_VAL				REG32_VAL(GPIO_USB_OP_CTRL_4BYTE_ADDR				)//fan
#define GPIO_BONDING_OPTION_4BYTE_VAL     		REG32_VAL(GPIO_BONDING_OPTION_4BYTE_ADDR     		)
#define GPIO_PIN_SHARING_SEL_4BYTE_VAL 			REG32_VAL(GPIO_PIN_SHARING_SEL_4BYTE_ADDR 		    )


#define GPIO_INT_REQ_TYPE_0_VAL					REG32_VAL(GPIO_INT_REQ_TYPE_0_ADDR					)
#define GPIO_UART_INT_REQ_TYPE_VAL 				REG32_VAL(GPIO_UART_INT_REQ_TYPE_ADDR 			    )
#define GPIO_I2C_INT_REQ_TYPE_VAL 				REG32_VAL(GPIO_I2C_INT_REQ_TYPE_ADDR 			    )
#define GPIO_I2S_KAPD_INT_REQ_TYPE_VAL			REG32_VAL(GPIO_I2S_KAPD_INT_REQ_TYPE_ADDR			)
#define GPIO_KAPD_INT_REQ_TYPE_VAL				REG32_VAL(GPIO_KAPD_INT_REQ_TYPE_ADDR				)
#define GPIO_PWM_INT_REQ_TYPE_VAL 				REG32_VAL(GPIO_PWM_INT_REQ_TYPE_ADDR 			    )
#define GPIO_INT_REQ_TYPE_1_VAL 				REG32_VAL(GPIO_INT_REQ_TYPE_1_ADDR					)

#define GPIO_INT_REQ_STS_VAL					REG32_VAL(GPIO_INT_REQ_STS_ADDR						)

#define GPIO_IO_DRV_GPIO1_4BYTE_VAL				REG32_VAL(GPIO_IO_DRV_GPIO1_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_SD_4BYTE_VAL				REG32_VAL(GPIO_IO_DRV_SD_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_VID_4BYTE_VAL 				REG32_VAL(GPIO_IO_DRV_VID_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_SF_4BYTE_VAL 				REG32_VAL(GPIO_IO_DRV_SF_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_NAND_4BYTE_VAL				REG32_VAL(GPIO_IO_DRV_NAND_4BYTE_ADDR 	    )
#define GPIO_IO_DRV_SPI_4BYTE_VAL 				REG32_VAL(GPIO_IO_DRV_SPI_4BYTE_ADDR 	    )

#define GPIO_PULL_EN_GP0_BYTE_VAL 			        REG8_VAL(GPIO_PULL_EN_GP0_BYTE_ADDR 			    )
#define GPIO_PULL_EN_GP1_BYTE_VAL 			        REG8_VAL(GPIO_PULL_EN_GP1_BYTE_ADDR 			    )    
#define GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_VAL 	    REG8_VAL(GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR 	    )    
#define GPIO_PULL_EN_GP3_STORGE_BYTE_VAL 		    REG8_VAL(GPIO_PULL_EN_GP3_STORGE_BYTE_ADDR 		    )    
#define GPIO_PULL_EN_GP4_VDOUT_7_0_BYTE_VAL       	REG8_VAL(GPIO_PULL_EN_GP4_VDOUT_7_0_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP5_VDOUT_15_8_BYTE_VAL       	REG8_VAL(GPIO_PULL_EN_GP5_VDOUT_15_8_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP6_VDOUT_23_16_BYTE_VAL      	REG8_VAL(GPIO_PULL_EN_GP6_VDOUT_23_16_BYTE_ADDR     )    
#define GPIO_PULL_EN_GP7_VD_BYTE_VAL       	    	REG8_VAL(GPIO_PULL_EN_GP7_VD_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP8_VDIN_BYTE_VAL     	    	REG8_VAL(GPIO_PULL_EN_GP8_VDIN_BYTE_ADDR     	    )    
#define GPIO_PULL_EN_GP9_VSYNC_BYTE_VAL 		    REG8_VAL(GPIO_PULL_EN_GP9_VSYNC_BYTE_ADDR 		    )    
#define GPIO_PULL_EN_GP10_I2S_BYTE_VAL 		    	REG8_VAL(GPIO_PULL_EN_GP10_I2S_BYTE_ADDR 		    )    
#define GPIO_PULL_EN_GP11_SPI_BYTE_VAL      	    REG8_VAL(GPIO_PULL_EN_GP11_SPI_BYTE_ADDR      	    )    
#define GPIO_PULL_EN_GP12_XD_BYTE_VAL       	    REG8_VAL(GPIO_PULL_EN_GP12_XD_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP13_XDIO_BYTE_VAL            	REG8_VAL(GPIO_PULL_EN_GP13_XDIO_BYTE_ADDR           )    
#define GPIO_PULL_EN_GP14_SD_BYTE_VAL       	    REG8_VAL(GPIO_PULL_EN_GP14_SD_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP15_NAND_7_0_BYTE_VAL        	REG8_VAL(GPIO_PULL_EN_GP15_NAND_7_0_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP16_NAND_15_8_BYTE_VAL       	REG8_VAL(GPIO_PULL_EN_GP16_NAND_15_8_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP17_IDE_CFG_0_BYTE_VAL       	REG8_VAL(GPIO_PULL_EN_GP17_IDE_CFG_0_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP18_IDE_CFG_1_BYTE_VAL       	REG8_VAL(GPIO_PULL_EN_GP18_IDE_CFG_1_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP19_IDED_7_0_BYTE_VAL        	REG8_VAL(GPIO_PULL_EN_GP19_IDED_7_0_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP20_IDED_15_8_BYTE_VAL 	    REG8_VAL(GPIO_PULL_EN_GP20_IDED_15_8_BYTE_ADDR 	    )    
#define GPIO_PULL_EN_GP21_I2C_BYTE_VAL       	    REG8_VAL(GPIO_PULL_EN_GP21_I2C_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP22_UART_0_1_BYTE_VAL        	REG8_VAL(GPIO_PULL_EN_GP22_UART_0_1_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP23_UART_2_3_BYTE_VAL        	REG8_VAL(GPIO_PULL_EN_GP23_UART_2_3_BYTE_ADDR       )    
#define GPIO_PULL_EN_GP24_TSDIN_7_0_BYTE_VAL       	REG8_VAL(GPIO_PULL_EN_GP24_TSDIN_7_0_BYTE_ADDR      )    
#define GPIO_PULL_EN_GP25_TS_BYTE_VAL       	    REG8_VAL(GPIO_PULL_EN_GP25_TS_BYTE_ADDR       	    )    
#define GPIO_PULL_EN_GP26_KPAD_BYTE_VAL             REG8_VAL(GPIO_PULL_EN_GP26_KPAD_BYTE_ADDR           )    
#define GPIO_PULL_EN_GP27_SPDIF_BYTE_VAL            REG8_VAL(GPIO_PULL_EN_GP27_SPDIF_BYTE_ADDR          )    
#define GPIO_PULL_EN_GP28_NAND_CFG_0_BYTE_VAL       REG8_VAL(GPIO_PULL_EN_GP28_NAND_CFG_0_BYTE_ADDR     )    
#define GPIO_PULL_EN_GP29_NAND_CFG_1_BYTE_VAL       REG8_VAL(GPIO_PULL_EN_GP29_NAND_CFG_1_BYTE_ADDR     )    
#define GPIO_PULL_EN_GP30_SPI_FLASH_BYTE_VAL 	    REG8_VAL(GPIO_PULL_EN_GP30_SPI_FLASH_BYTE_ADDR 	    )    
#define GPIO_PULL_EN_GP31_PWM_BYTE_VAL 		     	REG8_VAL(GPIO_PULL_EN_GP31_PWM_BYTE_ADDR 		    )        
#define GPIO_PULL_EN_GP60_USB_BYTE_VAL				REG8_VAL(GPIO_PULL_EN_GP60_USB_TYTE_ADDR			)    

#define GPIO_PULL_CTRL_GP0_BYTE_VAL 		    	REG8_VAL(GPIO_PULL_CTRL_GP0_BYTE_ADDR 			    )
#define GPIO_PULL_CTRL_GP1_BYTE_VAL 		    	REG8_VAL(GPIO_PULL_CTRL_GP1_BYTE_ADDR 			    )    
#define GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_VAL 	    REG8_VAL(GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP3_STORGE_BYTE_VAL          REG8_VAL(GPIO_PULL_CTRL_GP3_STORGE_BYTE_ADDR 	    )    
#define GPIO_PULL_CTRL_GP4_VDOUT_7_0_BYTE_VAL       REG8_VAL(GPIO_PULL_CTRL_GP4_VDOUT_7_0_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP5_VDOUT_15_8_BYTE_VAL      REG8_VAL(GPIO_PULL_CTRL_GP5_VDOUT_15_8_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP6_VDOUT_23_16_BYTE_VAL     REG8_VAL(GPIO_PULL_CTRL_GP6_VDOUT_23_16_BYTE_ADDR   )    
#define GPIO_PULL_CTRL_GP7_VD_BYTE_VAL       	    REG8_VAL(GPIO_PULL_CTRL_GP7_VD_BYTE_ADDR            )    
#define GPIO_PULL_CTRL_GP8_VDIN_BYTE_VAL     	    REG8_VAL(GPIO_PULL_CTRL_GP8_VDIN_BYTE_ADDR          )    
#define GPIO_PULL_CTRL_GP9_VSYNC_BYTE_VAL 	    	REG8_VAL(GPIO_PULL_CTRL_GP9_VSYNC_BYTE_ADDR			)    
#define GPIO_PULL_CTRL_GP10_I2S_BYTE_VAL 	    	REG8_VAL(GPIO_PULL_CTRL_GP10_I2S_BYTE_ADDR 	        )    
#define GPIO_PULL_CTRL_GP11_SPI_BYTE_VAL      	    REG8_VAL(GPIO_PULL_CTRL_GP11_SPI_BYTE_ADDR      	)    
#define GPIO_PULL_CTRL_GP12_XD_BYTE_VAL       	    REG8_VAL(GPIO_PULL_CTRL_GP12_XD_BYTE_ADDR       	)    
#define GPIO_PULL_CTRL_GP13_XDIO_BYTE_VAL           REG8_VAL(GPIO_PULL_CTRL_GP13_XDIO_BYTE_ADDR         )    
#define GPIO_PULL_CTRL_GP14_SD_BYTE_VAL       	    REG8_VAL(GPIO_PULL_CTRL_GP14_SD_BYTE_ADDR       	)    
#define GPIO_PULL_CTRL_GP15_NAND_7_0_BYTE_VAL       REG8_VAL(GPIO_PULL_CTRL_GP15_NAND_7_0_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP16_NAND_15_8_BYTE_VAL      REG8_VAL(GPIO_PULL_CTRL_GP16_NAND_15_8_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP17_IDE_CFG_0_BYTE_VAL      REG8_VAL(GPIO_PULL_CTRL_GP17_IDE_CFG_0_BYTE_ADDR    )     
#define GPIO_PULL_CTRL_GP18_IDE_CFG_1_BYTE_VAL      REG8_VAL(GPIO_PULL_CTRL_GP18_IDE_CFG_1_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP19_IDED_7_0_BYTE_VAL       REG8_VAL(GPIO_PULL_CTRL_GP19_IDED_7_0_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP20_IDED_15_8_BYTE_VAL 	    REG8_VAL(GPIO_PULL_CTRL_GP20_IDED_15_8_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP21_I2C_BYTE_VAL       	    REG8_VAL(GPIO_PULL_CTRL_GP21_I2C_BYTE_ADDR          )    
#define GPIO_PULL_CTRL_GP22_UART_0_1_BYTE_VAL       REG8_VAL(GPIO_PULL_CTRL_GP22_UART_0_1_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP23_UART_2_3_BYTE_VAL       REG8_VAL(GPIO_PULL_CTRL_GP23_UART_2_3_BYTE_ADDR     )    
#define GPIO_PULL_CTRL_GP24_TSDIN_7_0_BYTE_VAL      REG8_VAL(GPIO_PULL_CTRL_GP24_TSDIN_7_0_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP25_TS_BYTE_VAL       	    REG8_VAL(GPIO_PULL_CTRL_GP25_TS_BYTE_ADDR       	)    
#define GPIO_PULL_CTRL_GP26_KPAD_BYTE_VAL           REG8_VAL(GPIO_PULL_CTRL_GP26_KPAD_BYTE_ADDR         )    
#define GPIO_PULL_CTRL_GP27_SPDIF_BYTE_VAL          REG8_VAL(GPIO_PULL_CTRL_GP27_SPDIF_BYTE_ADDR        )    
#define GPIO_PULL_CTRL_GP28_NAND_CFG_0_BYTE_VAL     REG8_VAL(GPIO_PULL_CTRL_GP28_NAND_CFG_0_BYTE_ADDR   )    
#define GPIO_PULL_CTRL_GP29_NAND_CFG_1_BYTE_VAL     REG8_VAL(GPIO_PULL_CTRL_GP29_NAND_CFG_1_BYTE_ADDR   )    
#define GPIO_PULL_CTRL_GP30_SPI_FLASH_BYTE_VAL 	    REG8_VAL(GPIO_PULL_CTRL_GP30_SPI_FLASH_BYTE_ADDR    )    
#define GPIO_PULL_CTRL_GP31_PWM_BYTE_VAL 	    	REG8_VAL(GPIO_PULL_CTRL_GP31_PWM_BYTE_ADDR          )        
#define GPIO_PULL_EN_GP60_USB_TYTE_VAL				REG8_VAL(GPIO_PULL_EN_GP60_USB_TYTE_ADDR			)    


/* [Rx40] GPIO Enable Control Register for SD,MMC */
//#define GPIO_SD_DATA				0xF
//#define GPIO_MMC_DATA				0xF0

/* [Rx56] GPIO Eanble Control Register for UART */
#define GPIO_UR0_RTS				0x8
#define GPIO_UR0_TXD				0x4
#define GPIO_UR0_CTS				0x2
#define GPIO_UR0_RXD				0x1
#define GPIO_UR0_ALL				0xF
/* //fan
#define GPIO_UR1_RTS				0x10
#define GPIO_UR1_TXD				0x20
#define GPIO_UR1_CTS				0x40
#define GPIO_UR1_RXD				0x80
#define GPIO_UR1_ALL				0xF0
#define GPIO_UR2_RTS				0x100
#define GPIO_UR2_TXD				0x200
#define GPIO_UR2_CTS				0x400
#define GPIO_UR2_RXD				0x800
#define GPIO_UR2_ALL				0xF00
#define GPIO_UR3_RTS				0x1000
#define GPIO_UR3_TXD				0x2000
#define GPIO_UR3_CTS				0x4000
#define GPIO_UR3_RXD				0x8000
#define GPIO_UR3_ALL				0xF000
*/
/* [Rx96] GPIO Output Control Register for UARTS */
#define GPIO_OE_UR0_RTS		0x8
#define GPIO_OE_UR0_TXD		0x4
#define GPIO_OE_UR0_CTS		0x2
#define GPIO_OE_UR0_RXD		0x1
#define GPIO_OE_UR0_ALL		0xF
/* //fan
#define GPIO_OE_UR1_RTS		0x10
#define GPIO_OE_UR1_TXD		0x20
#define GPIO_OE_UR1_CTS		0x40
#define GPIO_OE_UR1_RXD		0x80
#define GPIO_OE_UR1_ALL		0xF0
#define GPIO_OE_UR2_RTS		0x100
#define GPIO_OE_UR2_TXD		0x200
#define GPIO_OE_UR2_CTS		0x400
#define GPIO_OE_UR2_RXD		0x800
#define GPIO_OE_UR2_ALL		0xF00
#define GPIO_OE_UR3_RTS		0x1000
#define GPIO_OE_UR3_TXD		0x2000
#define GPIO_OE_UR3_CTS		0x4000
#define GPIO_OE_UR3_RXD		0x8000
#define GPIO_OE_UR3_ALL		0xF000
*/
/* [RxD6] GPIO Output Data Register for UARTS */
#define GPIO_OD_UR0_RTS		0x8
#define GPIO_OD_UR0_TXD		0x4
#define GPIO_OD_UR0_CTS		0x2
#define GPIO_OD_UR0_RXD		0x1
#define GPIO_OD_UR0_ALL		0xF
/* //fan
#define GPIO_OD_UR1_RTS		0x10
#define GPIO_OD_UR1_TXD		0x20
#define GPIO_OD_UR1_CTS		0x40
#define GPIO_OD_UR1_RXD		0x80
#define GPIO_OD_UR1_ALL		0xF0
#define GPIO_OD_UR2_RTS		0x100
#define GPIO_OD_UR2_TXD		0x200
#define GPIO_OD_UR2_CTS		0x400
#define GPIO_OD_UR2_RXD		0x800
#define GPIO_OD_UR2_ALL		0xF00
#define GPIO_OD_UR3_RTS		0x1000
#define GPIO_OD_UR3_TXD		0x2000
#define GPIO_OD_UR3_CTS		0x4000
#define GPIO_OD_UR3_RXD		0x8000
#define GPIO_OD_UR3_ALL		0xF000
*/
/* [Rx16] GPIO Input Data Register for UARTS */
#define GPIO_ID_UR0_RTS		0x8
#define GPIO_ID_UR0_TXD		0x4
#define GPIO_ID_UR0_CTS		0x2
#define GPIO_ID_UR0_RXD		0x1
#define GPIO_ID_UR0_ALL		0xF
/*//fan
#define GPIO_ID_UR1_RTS		0x10
#define GPIO_ID_UR1_TXD		0x20
#define GPIO_ID_UR1_CTS		0x40
#define GPIO_ID_UR1_RXD		0x80
#define GPIO_ID_UR1_ALL		0xF0
#define GPIO_ID_UR2_RTS		0x100
#define GPIO_ID_UR2_TXD		0x200
#define GPIO_ID_UR2_CTS		0x400
#define GPIO_ID_UR2_RXD		0x800
#define GPIO_ID_UR2_ALL		0xF00
#define GPIO_ID_UR3_RTS		0x1000
#define GPIO_ID_UR3_TXD		0x2000
#define GPIO_ID_UR3_CTS		0x4000
#define GPIO_ID_UR3_RXD		0x8000
#define GPIO_ID_UR3_ALL		0xF000
*/
/* [Rx40] GPIO Eanble Control Register for Dedicated GPIO */
#define GPIO_GPIO_ALL		0xFF
#define GPIO_GPIO_0			0x1
#define GPIO_GPIO_1			0x2
#define GPIO_GPIO_2			0x4
#define GPIO_GPIO_3			0x8
#define GPIO_GPIO_4			0x10
#define GPIO_GPIO_5			0x20
#define GPIO_GPIO_6			0x40
#define GPIO_GPIO_7			0x80

/* [Rx80] GPIO Output Control Register for Dedicated GPIO */
#define GPIO_OE_GPIO_ALL		0xFF
#define GPIO_OE_GPIO_0			0x1
#define GPIO_OE_GPIO_1			0x2
#define GPIO_OE_GPIO_2			0x4
#define GPIO_OE_GPIO_3			0x8
#define GPIO_OE_GPIO_4			0x10
#define GPIO_OE_GPIO_5			0x20
#define GPIO_OE_GPIO_6			0x40
#define GPIO_OE_GPIO_7			0x80

/* [RxC0] GPIO Output Data Register for Dedicated GPIO */
#define GPIO_OD_GPIO_ALL		0xFF
#define GPIO_OD_GPIO_0			0x1
#define GPIO_OD_GPIO_1			0x2
#define GPIO_OD_GPIO_2			0x4
#define GPIO_OD_GPIO_3			0x8
#define GPIO_OD_GPIO_4			0x10
#define GPIO_OD_GPIO_5			0x20
#define GPIO_OD_GPIO_6			0x40
#define GPIO_OD_GPIO_7			0x80

/* [Rx00] GPIO Input Data Register for Dedicated GPIO */
#define GPIO_ID_GPIO_ALL		0xFF
#define GPIO_ID_GPIO_0			0x1
#define GPIO_ID_GPIO_1			0x2
#define GPIO_ID_GPIO_2			0x4
#define GPIO_ID_GPIO_3			0x8
#define GPIO_ID_GPIO_4			0x10
#define GPIO_ID_GPIO_5			0x20
#define GPIO_ID_GPIO_6			0x40
#define GPIO_ID_GPIO_7			0x80

/* [Rx300] GPIO Interrupt Request Type Register */
#define GPIO_IRQT0_LOW	  	(BIT1 & BIT0)
#define GPIO_IRQT0_HIGH		((~BIT1) & BIT0)
#define GPIO_IRQT0_FALLING	(BIT1 & (~BIT0))
#define GPIO_IRQT0_RISING	(BIT1 | BIT0)
#define GPIO_IRQT0_NEGEDGE	BIT2
#define GPIO_IRQT0_EN		BIT7
#define GPIO_IRQT1_LOW	   	(BIT9 & BIT8)
#define GPIO_IRQT1_HIGH		((~BIT9) & BIT8)
#define GPIO_IRQT1_FALLING	(BIT9 & (~BIT8))
#define GPIO_IRQT1_RISING	(BIT9 | BIT8)
#define GPIO_IRQT1_NEGEDGE	BIT10
#define GPIO_IRQT1_EN		BIT15
#define GPIO_IRQT2_LOW		(BIT17 & BIT16)
#define GPIO_IRQT2_HIGH		((~BIT17) & BIT16)
#define GPIO_IRQT2_FALLING	(BIT17 & (~BIT16))
#define GPIO_IRQT2_RISING	(BIT17 | BIT16)
#define GPIO_IRQT2_NEGEDGE	BIT18
#define GPIO_IRQT2_EN		BIT23
#define GPIO_IRQT3_LOW		(BIT25 & BIT24)
#define GPIO_IRQT3_HIGH	 	((~BIT25) & BIT24)
#define GPIO_IRQT3_FALLING	(BIT25 &	(~BIT24))
#define GPIO_IRQT3_RISING	(BIT25 | BIT24)
#define GPIO_IRQT3_NEGEDGE	BIT26
#define GPIO_IRQT3_EN		BIT31

/* [Rx318] GPIO Interrupt Request Type Register */
#define GPIO_IRQT4_LOW	  	(BIT1 & BIT0)
#define GPIO_IRQT4_HIGH		((~BIT1) & BIT0)
#define GPIO_IRQT4_FALLING	(BIT1 & (~BIT0))
#define GPIO_IRQT4_RISING	(BIT1 | BIT0)
#define GPIO_IRQT4_NEGEDGE	BIT2
#define GPIO_IRQT4_EN		BIT7
#define GPIO_IRQT5_LOW	   	(BIT9 & BIT8)
#define GPIO_IRQT5_HIGH		((~BIT9) & BIT8)
#define GPIO_IRQT5_FALLING	(BIT9 & (~BIT8))
#define GPIO_IRQT5_RISING	(BIT9 | BIT8)
#define GPIO_IRQT5_NEGEDGE	BIT10
#define GPIO_IRQT5_EN		BIT15
#define GPIO_IRQT6_LOW		(BIT17 & BIT16)
#define GPIO_IRQT6_HIGH		((~BIT17) & BIT16)
#define GPIO_IRQT6_FALLING	(BIT17 & (~BIT16))
#define GPIO_IRQT6_RISING	(BIT17 | BIT16)
#define GPIO_IRQT6_NEGEDGE	BIT18
#define GPIO_IRQT6_EN		BIT23
#define GPIO_IRQT7_LOW		(BIT25 & BIT24)
#define GPIO_IRQT7_HIGH	 	((~BIT25) & BIT24)
#define GPIO_IRQT7_FALLING	(BIT25 &	(~BIT24))
#define GPIO_IRQT7_RISING	(BIT25 | BIT24)
#define GPIO_IRQT7_NEGEDGE	BIT26
#define GPIO_IRQT7_EN		BIT31

/* [Rx320] GPIO Interrupt Request Status Register */
#define GPIO_IRQS0_ACTIVE		(BIT0)
#define GPIO_IRQS1_ACTIVE		(BIT1)
#define GPIO_IRQS2_ACTIVE		(BIT2)
#define GPIO_IRQS3_ACTIVE		(BIT3)
#define GPIO_IRQS4_ACTIVE		(BIT24)
#define GPIO_IRQS5_ACTIVE		(BIT25)
#define GPIO_IRQS6_ACTIVE		(BIT26)
#define GPIO_IRQS7_ACTIVE		(BIT27)

/* [Rx55] GPIO Control Register for I2C */
#define GPIO_I2C0_SCL	BIT0
#define GPIO_I2C0_SDA	BIT1
#define GPIO_I2C1_SCL	BIT2
#define GPIO_I2C1_SDA	BIT3
/* [Rx4D5] */
#define GPIO_I2C0_SCL_PULL_EN BIT0
#define GPIO_I2C0_SDA_PULL_EN BIT1
#define GPIO_I2C1_SCL_PULL_EN BIT2
#define GPIO_I2C1_SDA_PULL_EN BIT3

/* [Rx4E] GPIO Control Register for SD*/
#define GPIO_SD0_CLK BIT1
#define GPIO_SD0_CMD BIT2
#define GPIO_SD0_WP BIT3
#define GPIO_SD0_DATA0 BIT4
#define GPIO_SD0_DATA1 BIT5
#define GPIO_SD0_DATA2 BIT6
#define GPIO_SD0_DATA3 BIT7

/* [Rx4E] GPIO Control Register for SD*/
#define GPIO_SD0_CLK_PULL_EN BIT1
#define GPIO_SD0_CMD_PULL_EN BIT2
#define GPIO_SD0_WP_PULL_EN BIT3
#define GPIO_SD0_DATA0_PULL_EN BIT4
#define GPIO_SD0_DATA1_PULL_EN BIT5
#define GPIO_SD0_DATA2_PULL_EN BIT6
#define GPIO_SD0_DATA3_PULL_EN BIT7

/*[Rx5F] GPIO 31*/
#define GPIO_PWMOUT0	BIT3
#define GPIO_C25MHZCLK	BIT2
#define GPIO_C24MHZCLK	BIT1
#define GPIO_CLK_OUT	BIT0

#define GPIO_PWMOUT0_PULL_EN	BIT3
#define GPIO_C25MHZCLK_PULL_EN	BIT2
#define GPIO_C24MHZCLK_PULL_EN	BIT1
#define GPIO_CLK_OUT_PULL_EN	BIT0

#endif
/*=== END wmt_gpio.h ==========================================================*/

