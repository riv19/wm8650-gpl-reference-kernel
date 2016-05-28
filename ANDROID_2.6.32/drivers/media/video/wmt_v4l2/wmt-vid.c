/*++ 
 * linux/drivers/media/video/wmt_v4l2/wmt-vid.c
 * WonderMedia v4l video input device driver
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


#define WMT_VID_C


#include <linux/i2c.h>      /* for I2C_M_RD */

#include "wmt-vid.h"


//#define VID_REG_TRACE
#ifdef VID_REG_TRACE
#define VID_REG_SET32(addr, val)  \
        PRINT("REG_SET:0x%x -> 0x%0x\n", addr, val);\
        REG32_VAL(addr) = (val)
#else
#define VID_REG_SET32(addr, val)      REG32_VAL(addr) = (val)
#endif

//#define VID_DEBUG    /* Flag to enable debug message */
//#define VID_DEBUG_DETAIL
//#define VID_TRACE

#ifdef VID_DEBUG
#define DBG_MSG(fmt, args...)      PRINT("{%s} " fmt, __FUNCTION__ , ## args)
#else
#define DBG_MSG(fmt, args...)
#endif

#ifdef VID_DEBUG_DETAIL
#define DBG_DETAIL(fmt, args...)   PRINT("{%s} " fmt, __FUNCTION__ , ## args)
#else
#define DBG_DETAIL(fmt, args...)
#endif

#ifdef VID_TRACE
  #define TRACE(fmt, args...)      PRINT("{%s}:  " fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...) 
#endif

#define DBG_ERR(fmt, args...)      PRINT("*E* {%s} " fmt, __FUNCTION__ , ## args)


#define VID_INT_MODE        

#define ALIGN64(a)          ((((a)+63)>>6)<<6)

extern void wmt_i2c_xfer_continue_if(struct i2c_msg *msg, unsigned int num);
extern void wmt_i2c_xfer_if(struct i2c_msg *msg);

extern int wmt_i2c_xfer_continue_if_4(struct i2c_msg *msg, unsigned int num, int bus_id);



static vid_fb_t *_cur_fb;
static vid_fb_t *_prev_fb;

static unsigned int  cur_y_addr;   // temp
static unsigned int  cur_c_addr;   // temp

static spinlock_t   vid_lock;
static unsigned int  cmos_i2c_gpio_en =0;

swi2c_reg_t vid_i2c0_scl;
swi2c_reg_t vid_i2c0_sda;
swi2c_handle_t vid_swi2c;

#define PMC_ClOCK_ENABLE_LOWER          PM_CTRL_BASE_ADDR+0x250

/*-------------------------------Body Functions------------------------------------*/
#if 0
static void vid_dumpreg(char *buf,int len)
{
    int index=0;
    char *vir_buf=buf;

    DBG_MSG("dump addr 0x %08X length %d\n",vir_buf, len);
    for (index=0;index<len;index+=4) {
        DBG_MSG("%08X %08X \n",vir_buf+index,*(int *)(vir_buf+index));
    }
} /* End of vid_dumpreg() */
#endif

/*!*************************************************************************
* vid_gpio_init
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \Init gpio setting
*
* \retval none
*/
static void vid_gpio_init(vid_mode mode)
{
#ifdef __KERNEL__

     REG32_VAL(PMC_ClOCK_ENABLE_LOWER) |= BIT16; //turn on CMOS clock


    GPIO_CTRL_GP8_VDIN_BYTE_VAL = 0x0;
    GPIO_PULL_EN_GP8_VDIN_BYTE_VAL = 0x0;

    GPIO_CTRL_GP9_VSYNC_BYTE_VAL  &= ~(BIT0|BIT1|BIT2|BIT3);
    GPIO_PULL_EN_GP9_VSYNC_BYTE_VAL &= ~(BIT0|BIT1|BIT2|BIT3);
    GPIO_PIN_SHARING_SEL_4BYTE_VAL &= ~BIT11;//VID clock invert , 0 default falling edge

    
#else
	pGpio_Reg->CTRL_GP8_VDIN_byte = 0x0;	
	pGpio_Reg->PULL_EN_GP8_VDIN_byte = 0x0;
	
    pGpio_Reg->CTRL_GP9_VSYNC_byte &= ~(BIT0|BIT1|BIT2);
    pGpio_Reg->PULL_EN_GP9_VSYNC_byte &= ~ (BIT0|BIT1|BIT2);
	
    pGpio_Reg->PIN_SHARING_SEL_4byte |= BIT31;//DVO disable
    pGpio_Reg->PIN_SHARING_SEL_4byte &= ~BIT23;
#endif
    return;
} /* End of vid_gpio_init()*/

/*!*************************************************************************
* wmt_vid_i2c_write_page
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID mode
* \retval  0 if success
*/

int wmt_vid_i2c_write_page(int chipId ,unsigned int index,char *pdata,int len)
{

    struct i2c_msg msg[1];
    unsigned char buf[len+1];
    
    buf[0] = index;
    memcpy(&buf[1],pdata,len);
    msg[0].addr = chipId;
    msg[0].flags = 0 ;
    msg[0].flags &= ~(I2C_M_RD);
    msg[0].len = len;
    msg[0].buf = buf;

    wmt_i2c_xfer_continue_if_4(msg,1,1);
    return 0;
} /* End of wmt_vid_i2c_write_page() */


/*!*************************************************************************
* wmt_vid_i2c_read_page
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID mode
* \retval  0 if success
*/ 
int wmt_vid_i2c_read_page(int chipId ,unsigned int index,char *pdata,int len) 
{
    struct i2c_msg msg[2];
    unsigned char buf[len+1];

    memset(buf,0x55,len+1);
    buf[0] = index;

    msg[0].addr = chipId;
    msg[0].flags = 0 ;
    msg[0].flags &= ~(I2C_M_RD);
    msg[0].len = 1;
    msg[0].buf = buf;

    msg[1].addr = chipId;
    msg[1].flags = 0 ;
    msg[1].flags |= (I2C_M_RD);
    msg[1].len = len;
    msg[1].buf = buf;

    wmt_i2c_xfer_continue_if_4(msg, 2,1);
    memcpy(pdata,buf,len);

    return 0;
} /* End of wmt_vid_i2c_read_page() */

/*!*************************************************************************
* wmt_vid_i2c_read
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID mode
* \retval  0 if success
*/ 
int wmt_vid_i2c_read(int chipId ,unsigned int index) 
{
    char retval;
    
    if (cmos_i2c_gpio_en)
    {
			wmt_swi2c_read( &vid_swi2c, chipId*2,  index, &retval, 1 );
    }else{
	    wmt_vid_i2c_read_page( chipId ,index,&retval,1) ;    
    }

    
    return retval;
} /* End of wmt_vid_i2c_read() */

/*!*************************************************************************
* wmt_vid_i2c_write
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID mode
* \retval  0 if success
*/ 
int wmt_vid_i2c_write(int chipId ,unsigned int index,char data)
{
    if (cmos_i2c_gpio_en)
    {
				wmt_swi2c_write( &vid_swi2c, chipId*2,  index, &data, 2 );
    }else{
    	    	wmt_vid_i2c_write_page(chipId ,index,&data,2);
    }

    return 0;
} /* End of wmt_vid_i2c_write() */

/*!*************************************************************************
* wmt_vid_i2c_write16addr
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID mode
* \retval  0 if success
*/ 
int wmt_vid_i2c_write16addr(int chipId ,unsigned int index,unsigned int data)
{
	int ret=0;
	struct i2c_msg msg[1];
	unsigned char buf[4];
    if (cmos_i2c_gpio_en)
    {
    	printk("wmt_vid_i2c_write16addr() Not support GPIO mode now");
    	return -1;
    }
	buf[0]=((index>>8) & 0xff);
	buf[1]=(index & 0xff);
	buf[2]=(data & 0xff);

	msg[0].addr = chipId;
	msg[0].flags = 0;
       msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 3;
	msg[0].buf = buf;
	ret = wmt_i2c_xfer_continue_if_4(msg,1,1);
	return 0;
}
/*!*************************************************************************
* wmt_vid_i2c_read16addr
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID mode
* \retval  0 if success
*/ 

int wmt_vid_i2c_read16addr(int chipId ,unsigned int index)
{
	int ret=0;
	struct i2c_msg msg[2];
	unsigned char buf[2];
	unsigned int val;
    if (cmos_i2c_gpio_en)
    {
    	printk("wmt_vid_i2c_read16addr() Not support GPIO mode now");
    	return -1;
    }

	buf[0]=((index>>8) & 0xff);
	buf[1]=(index & 0xff);

	msg[0].addr = chipId;
	msg[0].flags = 0;
       msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 2;
	msg[0].buf = buf;

	msg[1].addr = chipId;
	msg[1].flags = 0;
       msg[1].flags |= (I2C_M_RD);
	msg[1].len = 1;
	msg[1].buf = buf;

	ret = wmt_i2c_xfer_continue_if_4(msg,2,1);
	val=buf[0];
	return val;
}


/*!*************************************************************************
* wmt_vid_set_mode
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID mode
* \retval  0 if success
*/ 
int wmt_vid_set_mode(int width, int height)
{
    TRACE("Enter\n");

    VID_REG_SET32( REG_VID_WIDTH,  width ); /* VID output width */
    VID_REG_SET32( REG_VID_LINE_WIDTH,  ALIGN64(width) );
    VID_REG_SET32( REG_VID_HEIGHT, height );    /* VID output height */
        
    TRACE("Leave\n");

    return 0;
} /* End of wmt_vid_set_mode() */

/*!*************************************************************************
* wmt_vid_set_cur_fb
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID source address for Y and C
* \retval  0 if success
*/ 
int wmt_vid_set_cur_fb(vid_fb_t *fb)
{
    unsigned long flags =0;

    spin_lock_irqsave(&vid_lock, flags);

    _prev_fb = _cur_fb;
    _cur_fb  = fb;

    fb->is_busy = 1;
    fb->done    = 0;
    wmt_vid_set_addr(fb->y_addr, fb->c_addr);		    

    //DBG_MSG("[%d] y_addr: 0x%x, c_addr: 0x%x\n", fb->id, fb->y_addr, fb->c_addr);
    //DBG_MSG("[%d] done: %d, is_busy: %d\n", fb->id, fb->done, fb->is_busy);

    spin_unlock_irqrestore(&vid_lock, flags);

    return 0;
} /* End of wmt_vid_set_cur_fb() */

/*!*************************************************************************
* wmt_vid_get_cur_fb
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID source address for Y and C
* \retval  0 if success
*/ 
vid_fb_t * wmt_vid_get_cur_fb(void)
{
    unsigned long flags =0;
    vid_fb_t *fb;    
    
    spin_lock_irqsave(&vid_lock, flags);
    fb = _cur_fb;
    //DBG_MSG("[%d] y_addr: 0x%x, c_addr: 0x%x\n", fb->id, fb->y_addr, fb->c_addr);
    //DBG_MSG("[%d] done: %d, is_busy: %d\n", fb->id, fb->done, fb->is_busy);
    spin_unlock_irqrestore(&vid_lock, flags);

    return fb;
} /* End of wmt_vid_get_cur_fb() */

/*!*************************************************************************
* wmt_vid_set_addr
* 
* Public Function by Willy Chuang, 2010/06/01
*/
/*!
* \brief
*       set VID source address for Y and C
* \retval  0 if success
*/ 
int wmt_vid_set_addr(unsigned int y_addr, unsigned int c_addr)
{
    TRACE("Enter\n");

    cur_y_addr = REG32_VAL(REG_VID_Y0_SA);
    cur_c_addr = REG32_VAL(REG_VID_C0_SA);

    if( (y_addr != cur_y_addr) || (c_addr != cur_c_addr)) {
        VID_REG_SET32( REG_VID_Y0_SA, y_addr ); /* VID Y FB address */
        VID_REG_SET32( REG_VID_C0_SA, c_addr ); /* VID C FB address */
    }
    TRACE("Leave\n");

    return 0;
} /* End of wmt_vid_set_addr() */

/*!*************************************************************************
* wmt_vid_open
* 
* Public Function by Willy Chuang, 2010/05/28
*/
/*!
* \brief
*       init CMOS module
* \retval  0 if success
*/ 

int wmt_vid_open(vid_mode mode,cmos_uboot_env_t *cmos_uboot_env)
{
    int value, int_ctrl;
    
    TRACE("Enter\n");

       cmos_i2c_gpio_en = cmos_uboot_env->i2c_gpio_en;
       printk("cmos_i2c_gpio_en 0x%08x \n",cmos_i2c_gpio_en);
	if (cmos_i2c_gpio_en)
	{
		memset(&vid_i2c0_scl, 0, sizeof(vid_i2c0_scl));
		memset(&vid_i2c0_sda, 0, sizeof(vid_i2c0_sda));
		
		vid_i2c0_scl.bit_mask = 1 << cmos_uboot_env->i2c_gpio_scl_binum;
		vid_i2c0_scl.pull_en_bit_mask = 1 << cmos_uboot_env->reg_i2c_gpio_scl_gpio_pe_bitnum;

		vid_i2c0_scl.data_in = cmos_uboot_env->reg_i2c_gpio_scl_gpio_in ;
		vid_i2c0_scl.gpio_en = cmos_uboot_env->reg_i2c_gpio_scl_gpio_en ;
		vid_i2c0_scl.out_en = cmos_uboot_env->reg_i2c_gpio_scl_gpio_od ;
		vid_i2c0_scl.data_out = cmos_uboot_env->reg_i2c_gpio_scl_gpio_oc ;
		vid_i2c0_scl.pull_en = cmos_uboot_env->reg_i2c_gpio_scl_gpio_pe ;


		vid_i2c0_sda.bit_mask =  1 << cmos_uboot_env->i2c_gpio_sda_binum;
		vid_i2c0_sda.pull_en_bit_mask = 1 << cmos_uboot_env->reg_i2c_gpio_sda_gpio_pe_bitnum;

		vid_i2c0_sda.data_in = cmos_uboot_env->reg_i2c_gpio_sda_gpio_in ;
		vid_i2c0_sda.gpio_en = cmos_uboot_env->reg_i2c_gpio_sda_gpio_en ;
		vid_i2c0_sda.out_en = cmos_uboot_env->reg_i2c_gpio_sda_gpio_od ;
		vid_i2c0_sda.data_out = cmos_uboot_env->reg_i2c_gpio_sda_gpio_oc ;
		vid_i2c0_sda.pull_en = cmos_uboot_env->reg_i2c_gpio_sda_gpio_pe ;

		if (vid_i2c0_scl.data_in & 0x1)
		{
			vid_i2c0_scl.bit_mask <<= 8;
			vid_i2c0_scl.pull_en_bit_mask <<= 8;
			vid_i2c0_scl.data_in -= 1;
			vid_i2c0_scl.gpio_en -= 1;
			vid_i2c0_scl.out_en -= 1;
			vid_i2c0_scl.data_out -= 1;
			vid_i2c0_scl.pull_en -= 1;

		}
		if (vid_i2c0_sda.data_in & 0x1)
		{
			vid_i2c0_sda.bit_mask <<= 8;
			vid_i2c0_sda.pull_en_bit_mask <<= 8;
			vid_i2c0_sda.data_in -= 1;
			vid_i2c0_sda.gpio_en -= 1;
			vid_i2c0_sda.out_en -= 1;
			vid_i2c0_sda.data_out -= 1;
			vid_i2c0_sda.pull_en -= 1;

		}


		vid_swi2c.scl_reg = &vid_i2c0_scl;
		vid_swi2c.sda_reg = &vid_i2c0_sda;

	}
		printk(" EN %d __GPIO_BASE 0x%08x \n",cmos_uboot_env->i2c_gpio_en,__GPIO_BASE);
		printk("SCL 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",vid_i2c0_scl.bit_mask,vid_i2c0_scl.pull_en_bit_mask,vid_i2c0_scl.data_in,vid_i2c0_scl.gpio_en,vid_i2c0_scl.out_en,vid_i2c0_scl.data_out, vid_i2c0_scl.pull_en);
		printk("SDA 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n",vid_i2c0_sda.bit_mask,vid_i2c0_sda.pull_en_bit_mask,vid_i2c0_sda.data_in,vid_i2c0_sda.gpio_en,vid_i2c0_sda.out_en,vid_i2c0_sda.data_out, vid_i2c0_sda.pull_en );
    /*--------------------------------------------------------------------------
        Step 1: Init GPIO for CMOS or TVDEC mode
    --------------------------------------------------------------------------*/
    vid_gpio_init(mode);

    /*--------------------------------------------------------------------------
        Step 2: Init CMOS or TVDEC module
    --------------------------------------------------------------------------*/
    value = REG32_VAL(REG_VID_TVDEC_CTRL);
    VID_REG_SET32( REG_VID_MEMIF_EN, 0x1 );
    VID_REG_SET32( REG_VID_OUTPUT_FORMAT, 0x0 );  // 0: 422   1: 444

    int_ctrl = 0x00;
    if(mode == VID_MODE_CMOS) {
        VID_REG_SET32( REG_VID_TVDEC_CTRL, (value & 0xFFFFFFE)); /* disable TV decoder */
        VID_REG_SET32( REG_VID_CMOS_PIXEL_SWAP, 0x2);    /* 0x2 for YUYV */
  #ifdef VID_INT_MODE
        int_ctrl = 0x0808;
  #endif
        VID_REG_SET32( REG_VID_INT_CTRL, int_ctrl );
//        VID_REG_SET32( REG_VID_CMOS_EN, 0x1);	/* enable CMOS */
    }
    else {
        VID_REG_SET32( REG_VID_TVDEC_CTRL, (value | 0x1) );	/* enable TV decoder */
  #ifdef VID_INT_MODE
        int_ctrl = 0x0404;
  #endif
        VID_REG_SET32( REG_VID_INT_CTRL, int_ctrl );
        VID_REG_SET32( REG_VID_CMOS_EN, 0x0);	/* disable CMOS */
    }
    cur_y_addr = 0;
    cur_c_addr = 0;
    
    _cur_fb  = 0;
    _prev_fb = 0;





    spin_lock_init(&vid_lock);

    TRACE("Leave\n");

    return 0;
} /* End of wmt_vid_open() */

/*!*************************************************************************
* wmt_vid_close
* 
* Public Function by Willy Chuang, 2010/05/28
*/
/*!
* \brief
*       release CMOS module
* \retval  0 if success
*/ 
int wmt_vid_close(vid_mode mode)
{
    TRACE("Enter\n");

    REG32_VAL(PMC_ClOCK_ENABLE_LOWER) &= ~BIT16; //turn off CMOS clock
     GPIO_CTRL_GP9_VSYNC_BYTE_VAL  |= BIT3;//set to GPIO not CMOS clock 



    if(mode == VID_MODE_CMOS) {
        VID_REG_SET32( REG_VID_CMOS_EN, 0x0);	/* disable CMOS */
    }
    else {
        int value = REG32_VAL(REG_VID_TVDEC_CTRL);
        VID_REG_SET32( REG_VID_TVDEC_CTRL, (value & 0xFFFFFFE)); /* disable TV decoder */
    }
    TRACE("Leave\n");

    return 0;
} /* End of wmt_vid_close() */

/*--------------------End of Function Body -----------------------------------*/
#undef DBG_MSG
#undef DBG_DETAIL
#undef TRACE
#undef DBG_ERR

#undef WMT_VID_C
