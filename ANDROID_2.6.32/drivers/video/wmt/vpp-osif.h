/*++ 
 * linux/drivers/video/wmt/vpp-osif.h
 * WonderMedia video post processor (VPP) driver
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

/*
 * ChangeLog
 *
 * 2010-10-18  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#ifndef VPP_OSIF_H
#define VPP_OSIF_H

/*-------------------- DEPENDENCY -------------------------------------*/
#ifdef __KERNEL__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    #include <mach/hardware.h>
    #include <mach/wmt_mmap.h>
	#include <mach/memblock.h>

    #define SA_INTERRUPT IRQF_DISABLED
#else
    #include <asm/arch-wmt/hardware.h>
    #include <asm/arch/wmt_mmap.h>
	#include <asm/arch-wmt/memblock.h>	
#endif
	#define THE_MB_USER			"VPP-MB"
#else
    #define __ASM_ARCH_HARDWARE_H
    #include "wmt_mmap.h"
	#include "../pmc/wmt_clk.h"

	extern int auto_pll_divisor(enum dev_id dev, enum clk_cmd cmd, int unit, int freq);
	extern void udelay(int us);
#endif

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#define DPRINT printk
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DPRINT printf
#endif

/*	following is the C++ header	*/
#ifdef	__cplusplus
extern	"C" {
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
#ifdef __KERNEL__

#else
#define REG32_VAL(addr) (*(volatile unsigned int *)(addr))
#define REG16_VAL(addr) (*(volatile unsigned short *)(addr))
#define REG8_VAL(addr)  (*(volatile unsigned char *)(addr))

#define U32 unsigned int
#define U16 unsigned short
#define U8 unsigned char

#define kmalloc(a,b) 	malloc(a)
#define kfree(a)		free(a)
#define GFP_KERNEL		0
#define module_init(a)	

#endif

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  hdmi_xxx_t;  *//*Example*/

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef VPP_OSIF_C
#define EXTERN
#else
#define EXTERN   extern
#endif /* ifdef VPP_OSIF_C */

/* EXTERN int      hdmi_xxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
/* #define HDMI_XXX_YYY   xxxx *//*Example*/
#ifdef DEBUG
#define DBGMSG(fmt, args...)  DPRINT("%s: " fmt, __FUNCTION__ , ## args)
#else
#define DBGMSG(fmt, args...) do {} while(0)
#endif

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  hdmi_xxx(void); *//*Example*/

#ifdef __KERNEL__
void vpp_dbg_show(int level,int tmr,char *str);
#else
void vpp_initialization(int FunctionNumber);
void udelay(int us);
void mdelay(int ms);
#endif

#ifdef	__cplusplus
}
#endif
#endif //VPP_OSIF_H

