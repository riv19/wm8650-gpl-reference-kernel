/*++ 
 * linux/drivers/video/wmt/osif.c
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

#define VPP_OSIF_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "vpp-osif.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LVDS_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx lvds_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in lvds.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  lvds_xxx;        *//*Example*/

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lvds_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
#ifdef __KERNEL__
#include <asm/io.h>
#include <linux/proc_fs.h>

#else
__inline__ U32 inl(U32 offset)
{
	return REG32_VAL(offset);
}

__inline__ void outl(U32 val,U32 offset)
{
	REG32_VAL(offset) = val;
}

static __inline__ U16 inw(U32 offset)
{
	return REG16_VAL(offset);
}

static __inline__ void outw(U16 val,U32 offset)
{
	REG16_VAL(offset) = val;
}

static __inline__ U8 inb(U32 offset)
{
	return REG8_VAL(offset);
}

static __inline__ void outb(U8 val,U32 offset)
{
	REG8_VAL(offset) = val;
}

int get_key(void) 
{
	int key;

	extern int get_num(unsigned int min,unsigned int max,char *message,unsigned int retry);
	key = get_num(0, 256, "Input:", 5);
	DPRINT("\n");
	return key;
}

void udelay(int us)
{
	extern void vpp_post_delay(U32 tmr);

	vpp_post_delay(us);
}

void mdelay(int ms)
{
	udelay(ms*1000);
}
#endif

//Internal functions
U8 vppif_reg8_in(U32 offset)
{
	return (inb(offset));
}

U8 vppif_reg8_out(U32 offset, U8 val)
{
	outb(val, offset);
	return (val);
}

U16 vppif_reg16_in(U32 offset)
{
	return (inw(offset));
}

U16 vppif_reg16_out(U32 offset, U16 val)
{
	outw(val, offset);
	return (val);
}

U32 vppif_reg32_in(U32 offset)
{
	return (inl(offset));
}

U32 vppif_reg32_out(U32 offset, U32 val)
{
	outl(val, offset);
	return (val);
}

U32 vppif_reg32_write(U32 offset, U32 mask, U32 shift, U32 val)
{
	U32 new_val;

#ifdef VPPIF_DEBUG
	if( val > (mask >> shift) ){
		VPPIFMSG("*E* check the parameter 0x%x 0x%x 0x%x 0x%x\n",offset,mask,shift,val);
	}
#endif	

	new_val = (inl(offset) & ~(mask)) | (((val) << (shift)) & mask);
	outl(new_val, offset);
	return (new_val);
}

U32 vppif_reg32_read(U32 offset, U32 mask, U32 shift)
{
	return ((inl(offset) & mask) >> shift);
}

U32 vppif_reg32_mask(U32 offset, U32 mask, U32 shift)
{
	return (mask);
}

