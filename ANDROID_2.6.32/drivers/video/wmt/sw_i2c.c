/*++ 
 * linux/drivers/video/wmt/sw_i2c.c
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
 * 2010-08-05  Sam Shen
 *     * Add License declaration and ChangeLog
 */

// #include "userdef.h"
#include <linux/version.h>
#include "sw_i2c.h"

#include <linux/spinlock.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#include <mach/hardware.h>
#else
#include <asm/hardware.h>
#endif
#include <linux/delay.h>
#define   SPEED			5000000

#define delay_time 30

spinlock_t swi2c_irqlock = SPIN_LOCK_UNLOCKED;
swi2c_reg_t *swi2c_scl,*swi2c_sda;

void wmt_swi2c_delay(unsigned int time)
{
    udelay(time);
}

void wmt_swi2c_SetSDAInput(void)
{
	REG16_VAL(swi2c_sda->gpio_en) |= swi2c_sda->bit_mask;
	REG16_VAL(swi2c_sda->out_en) &= ~swi2c_sda->bit_mask;
}


void wmt_swi2c_SetSDAOutput(void)
{
	REG16_VAL(swi2c_sda->gpio_en) |= swi2c_sda->bit_mask;
	REG16_VAL(swi2c_sda->out_en) |= swi2c_sda->bit_mask;
}

bool wmt_swi2c_GetSDA(void) // bit
{
	if(*(volatile unsigned short *)(swi2c_sda->data_in) & swi2c_sda->bit_mask)
		return (volatile bool)1;
	return (volatile bool)0;
}

bool wmt_swi2c_GetSCL(void) // bit
{
	if(*(volatile unsigned short *)(swi2c_scl->data_in) & swi2c_scl->bit_mask)
		return (volatile bool)1;
	return (volatile bool)0;
}

void wmt_swi2c_SetSDA(int high)
{
	if( high ){
		//set to GPI and pull high
		REG16_VAL(swi2c_sda->gpio_en) |=  swi2c_sda->bit_mask;
		REG16_VAL(swi2c_sda->out_en) &=~ swi2c_sda->bit_mask;
		if( swi2c_sda->pull_en )
			REG16_VAL(swi2c_sda->pull_en) &=~ swi2c_sda->pull_en_bit_mask;
	}
	else {
		REG16_VAL(swi2c_sda->gpio_en) |= swi2c_sda->bit_mask;
		REG16_VAL(swi2c_sda->out_en) |= swi2c_sda->bit_mask;
		REG16_VAL(swi2c_sda->data_out) &= ~swi2c_sda->bit_mask;
	}
}

void wmt_swi2c_SetSCL(int high)
{
	if( high ){
		REG16_VAL(swi2c_scl->gpio_en) |= swi2c_scl->bit_mask;
		REG16_VAL(swi2c_scl->out_en) &= ~swi2c_scl->bit_mask;
		if( swi2c_scl->pull_en )
			REG16_VAL(swi2c_scl->pull_en) &=~ swi2c_scl->pull_en_bit_mask;
	}
	else {
		REG16_VAL(swi2c_scl->gpio_en) |= swi2c_scl->bit_mask;
		REG16_VAL(swi2c_scl->out_en) |= swi2c_scl->bit_mask;
		REG16_VAL(swi2c_scl->data_out) &= ~swi2c_scl->bit_mask;
	}
}

int wmt_swi2c_SetData(int high)
{
	unsigned int wait = 1;
	
	wmt_swi2c_SetSDA(high);
	do {
		if( wmt_swi2c_GetSDA()==((high)?1:0) )
			return 0;
	} while(wait++ < SPEED);
	return 1;
}

int wmt_swi2c_SetClock(int high)
{
	unsigned int wait=1;

	udelay(5);	// 3-100kHz, 5-80kHz

	wmt_swi2c_SetSCL(high);
	do {
	    if(wmt_swi2c_GetSCL()==((high)?1:0))
	    	return 0;
	}while(wait++<SPEED);
	return 1; // fail
}

int wmt_swi2c_StartI2C(void)
{
	if(wmt_swi2c_SetData(1))    return 1;
	if(wmt_swi2c_SetClock(1))   return 2;
	if(wmt_swi2c_SetData(0))    return 3;
	wmt_swi2c_delay(0);
	if(wmt_swi2c_SetClock(0))   return 4;
	return 0; // success
}

int wmt_swi2c_StopI2C(void)
{
	if(wmt_swi2c_SetData(0))    return 1;
	if(wmt_swi2c_SetClock(0))   return 1;
	if(wmt_swi2c_SetClock(1))   return 1;
	wmt_swi2c_delay(0);
    if(wmt_swi2c_SetData(1))    return 1;
	return 0; // success
}

int wmt_swi2c_WriteAck(unsigned char byte)
{
	int ret;
	int bit;

    for(bit=7; bit>=0; bit--){
    	wmt_swi2c_SetData(byte & 0x80);
		byte<<=1;
		if(wmt_swi2c_SetClock(1))	return 1;
		if(wmt_swi2c_SetClock(0))   return 1;
	}
	ret = 0;
	wmt_swi2c_SetSDAInput();
	if(wmt_swi2c_SetClock(1)){
		ret = 1;
	}
	else if(wmt_swi2c_GetSDA()) {
		ret = (wmt_swi2c_SetClock(0))? 1:0;
	}
	else if(wmt_swi2c_SetClock(0)) {
		ret = 1;
	}
	wmt_swi2c_SetSDAOutput();
	return ret;
}

int wmt_swi2c_ReadAck(unsigned char *byte,int last)
{
	unsigned char i;
	unsigned char Data = 0;

	wmt_swi2c_SetSDAInput();

	for(i = 0; i < 8; i++){
		if(wmt_swi2c_SetClock(1)){
			wmt_swi2c_SetSDAOutput();
			return 1;
		}
		Data <<= 1;
		wmt_swi2c_delay(0);
		Data |= wmt_swi2c_GetSDA();
		if(wmt_swi2c_SetClock(0)){
			wmt_swi2c_SetSDAOutput();
			return 1;
		}
	}

	*byte = Data;
	wmt_swi2c_SetSDAOutput();

	wmt_swi2c_delay(0);
	if( wmt_swi2c_SetData((last)? 1:0) )
		return 1;
	if(wmt_swi2c_SetClock(1))
		return 1;
	wmt_swi2c_delay(0);
	if(wmt_swi2c_SetClock(0))
		return 1;
	wmt_swi2c_delay(0);
	return 0;
}

int wmt_swi2c_tx(
    char     addr,
    char *   buf,
    int      cnt,
    int *    ret_cnt
)
{
    int ret = 0;
	unsigned long flags;
	unsigned char i;
	
	spin_lock_irqsave(&swi2c_irqlock, flags);

	ret |= wmt_swi2c_StartI2C();
	if( ret ) printk("[SWI2C] tx 1 %d\n",ret);
	
    ret |= wmt_swi2c_WriteAck(addr);
	if( ret ) printk("[SWI2C] tx 2\n");
	
    for (i = 0; i < cnt; i++){
         ret |= wmt_swi2c_WriteAck(buf[i]);
    }
	if( ret ) printk("[SWI2C] tx 3\n");
	
    ret |= wmt_swi2c_StopI2C();
	if( ret ) printk("[SWI2C] tx 4\n");

	spin_unlock_irqrestore(&swi2c_irqlock, flags);	
    return ret;
}

int wmt_swi2c_rx(
    char    addr,
    char	*buf,
    int     cnt,
    int 	*ret_cnt
)
{
    int ret = 0;
	unsigned long flags;
	int i;

	spin_lock_irqsave(&swi2c_irqlock, flags);
	
	ret |= wmt_swi2c_StartI2C();
    ret |= wmt_swi2c_WriteAck(addr | 0x01);
    for(i=0;i<cnt;i++){
		ret |= wmt_swi2c_ReadAck(&buf[i],(i==(cnt-1)));
    }
    ret |= wmt_swi2c_StopI2C();
    
	spin_unlock_irqrestore(&swi2c_irqlock, flags);
    return ret;
}

int wmt_swi2c_read(
	swi2c_handle_t *handle,
    char addr,
    char index,
    char *buf,
    int cnt
)
{
    int ret = 0;
    char buffer[24];
    int temp = 0;

	swi2c_scl = handle->scl_reg;
	swi2c_sda = handle->sda_reg;
	
	buffer[0] = index;
    ret = wmt_swi2c_tx(addr, buffer, 1, &temp);
    if (ret){
		printk("[SWI2C] *E* tx fail\n");
    	goto exit;
    }
    ret = wmt_swi2c_rx(addr, buf, cnt, &temp);
    if (ret){
		printk("[SWI2C] *E* rx fail\n");
    	goto exit;
    }
exit:
	return ret;
}

int wmt_swi2c_write(
    swi2c_handle_t *handle,
    char addr,
    char index,
    char *buf,
    int cnt
)
{
    int ret = 0;
    char buffer[24];
    int temp;
 
    swi2c_scl = handle->scl_reg;
    swi2c_sda = handle->sda_reg;

    buffer[0] = index;
    buffer[1] = *buf;
    ret = wmt_swi2c_tx(addr, buffer, cnt, &temp);
    if (ret) {
	   	printk("[SWI2C] *E* tx fail\n");
    	goto exit;
    }
exit:
	return ret;
}
