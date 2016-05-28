/*++ 
 * linux/drivers/video/wmt/dev-vpp.c
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

#define DEV_VPP_C
// #define DEBUG

// #include <fcntl.h>
// #include <unistd.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>
#include <linux/sysctl.h>
#include <linux/delay.h>

#include "vpp.h"
#include "govrh.h"
#include "vout.h"

#define VPP_PROC_NUM		10
#define VPP_DISP_FB_MAX		10
#define VPP_DISP_FB_NUM		4

typedef struct {
	int (*func)(void *arg);
	void *arg;
	struct list_head list;
	vpp_int_t type;
	struct semaphore sem;
} vpp_proc_t;

typedef struct {
	vpp_dispfb_t parm;
	vpp_pts_t pts;
	struct list_head list;
} vpp_dispfb_parm_t;

struct list_head vpp_disp_fb_list;
struct list_head vpp_disp_free_list;
#ifdef WMT_FTBLK_PIP
struct list_head vpp_pip_fb_list;
#endif
vpp_dispfb_parm_t vpp_disp_fb_array[VPP_DISP_FB_MAX];

typedef struct {
	struct list_head list;
	struct tasklet_struct tasklet;
	void (*proc)(int arg);
} vpp_irqproc_t;

vpp_irqproc_t *vpp_irqproc_array[32];
struct list_head vpp_free_list;
vpp_proc_t vpp_proc_array[VPP_PROC_NUM];

extern void wmt_i2c_xfer_continue_if(struct i2c_msg *msg, unsigned int num);
extern void wmt_i2c_xfer_if(struct i2c_msg *msg);
extern int wmt_i2c_xfer_continue_if_4(struct i2c_msg *msg, unsigned int num, int bus_id);
extern struct fb_var_screeninfo vfb_var;
int vpp_disp_fb_cnt(struct list_head *list);

#ifdef CONFIG_VPP_VBIE_FREE_MB
static unsigned int vpp_free_y_addr;
static unsigned int vpp_free_c_addr;
#endif
static unsigned int vpp_cur_dispfb_y_addr;
static unsigned int vpp_cur_dispfb_c_addr;
static unsigned int vpp_pre_dispfb_y_addr;
static unsigned int vpp_pre_dispfb_c_addr;

int vpp_vpu_disp_cnt;
int vpp_pip_disp_cnt;

/* error status counter */
int vpp_govw_tg_err_cnt;
int vpp_vpu_disp_skip_cnt;
int vpp_govw_vpu_not_ready_cnt;
int vpp_govw_ge_not_ready_cnt;
int vpp_vpu_y_err_cnt;
int vpp_vpu_c_err_cnt;
int vpp_govr_underrun_cnt;

int vpp_dac_sense_enable = 1;

vpp_path_t vpp_govm_path;
spinlock_t vpp_irqlock = SPIN_LOCK_UNLOCKED;
static unsigned long vpp_lock_flags;
static inline void vpp_lock(void)
{
	spin_lock_irqsave(&vpp_irqlock, vpp_lock_flags);
}

static inline void vpp_unlock(void)
{
	spin_unlock_irqrestore(&vpp_irqlock, vpp_lock_flags);
}

static DECLARE_MUTEX(vpp_sem);
void vpp_set_mutex(int lock)
{
	if( lock )
		down(&vpp_sem);
	else
		up(&vpp_sem);
}

/*!*************************************************************************
* vpp_dbg_show()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	show debug message with time period
*		
* \retval  None
*/ 
#define VPP_DBG_TMR_NUM		3
//#define VPP_DBG_DIAG_NUM	100
#ifdef VPP_DBG_DIAG_NUM
char vpp_dbg_diag_str[VPP_DBG_DIAG_NUM][100];
int vpp_dbg_diag_index;
int vpp_dbg_diag_delay;
#endif

/*----------------------- VPP API in Linux Kernel --------------------------------------*/
void vpp_dbg_show(int level,int tmr,char *str)
{
	static struct timeval pre_tv[VPP_DBG_TMR_NUM];
	struct timeval tv;
	unsigned int tm_usec = 0;

	if( vpp_check_dbg_level(level)==0 )
		return;

	if( tmr && (tmr <= VPP_DBG_TMR_NUM) ){
		do_gettimeofday(&tv);
		if( pre_tv[tmr-1].tv_sec ){
			tm_usec = ( tv.tv_sec == pre_tv[tmr-1].tv_sec )? (tv.tv_usec - pre_tv[tmr-1].tv_usec):(1000000 + tv.tv_usec - pre_tv[tmr-1].tv_usec);
		}
		pre_tv[tmr-1] = tv;
	}

#ifdef VPP_DBG_DIAG_NUM
	if( level == VPP_DBGLVL_DIAG ){
		if( str ){
			char *ptr = &vpp_dbg_diag_str[vpp_dbg_diag_index][0];
			sprintf(ptr,"%s (%d,%d)(T%d %d usec)",str,(int)tv.tv_sec,(int)tv.tv_usec,tmr,(int) tm_usec);
			vpp_dbg_diag_index = (vpp_dbg_diag_index + 1) % VPP_DBG_DIAG_NUM;
		}

		if( vpp_dbg_diag_delay ){
			vpp_dbg_diag_delay--;
			if( vpp_dbg_diag_delay == 0 ){
				int i;
				
				DPRINT("----- VPP DIAG -----\n");
				for(i=0;i<VPP_DBG_DIAG_NUM;i++){
					DPRINT("%02d : %s\n",i,&vpp_dbg_diag_str[vpp_dbg_diag_index][0]);
					vpp_dbg_diag_index = (vpp_dbg_diag_index + 1) % VPP_DBG_DIAG_NUM;				
				}
			}
		}
		return;
	}
#endif
	
	if( str ) {
		if( tmr ){
			DPRINT("[VPP] %s (T%d period %d usec)\n",str,tmr-1,(int) tm_usec);
		}
		else {
			DPRINT("[VPP] %s\n",str);
		}
	}
} /* End of vpp_dbg_show */

static void vpp_dbg_show_val1(int level,int tmr,char *str,int val)
{
	if( vpp_check_dbg_level(level) ){
		char buf[50];

		sprintf(buf,"%s 0x%x",str,val);
		vpp_dbg_show(level,tmr,buf);
	}
}

/*!*************************************************************************
* vpp_i2c_write()
* 
* Private Function by Sam Shen, 2009/02/26
*/
/*!
* \brief	write i2c
*		
* \retval  None
*/ 
int vpp_i2c_write(unsigned int addr,unsigned int index,char *pdata,int len)
{
    struct i2c_msg msg[1];
	unsigned char buf[len+1];

	addr = (addr >> 1);
    buf[0] = index;
	memcpy(&buf[1],pdata,len);
    msg[0].addr = addr;
    msg[0].flags = 0 ;
    msg[0].flags &= ~(I2C_M_RD);
    msg[0].len = len+1;
    msg[0].buf = buf;
#ifdef CONFIG_I2C_WMT
    wmt_i2c_xfer_if(msg);
#endif

#ifdef DEBUG
{
	int i;

	DBGMSG("vpp_i2c_write(addr 0x%x,index 0x%x,len %d\n",addr,index,len);	
	for(i=0;i<len;i+=8){
		DBGMSG("%d : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",i,
			pdata[i],pdata[i+1],pdata[i+2],pdata[i+3],pdata[i+4],pdata[i+5],pdata[i+6],pdata[i+7]);
	}
}	
#endif
    return 0;
} /* End of vpp_i2c_write */

/*!*************************************************************************
* vpp_i2c_read()
* 
* Private Function by Sam Shen, 2009/02/26
*/
/*!
* \brief	read i2c
*		
* \retval  None
*/ 
int vpp_i2c_read(unsigned int addr,unsigned int index,char *pdata,int len) 
{
	struct i2c_msg msg[2];
	unsigned char buf[len+1];

	addr = (addr >> 1);	
	memset(buf,0x55,len+1);
    buf[0] = index;
	buf[1] = 0x0;

    msg[0].addr = addr;
    msg[0].flags = 0 ;
	msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 1;
    msg[0].buf = buf;

	msg[1].addr = addr;
	msg[1].flags = 0 ;
	msg[1].flags |= (I2C_M_RD);
	msg[1].len = len;
	msg[1].buf = buf;

#ifdef CONFIG_I2C_WMT
	wmt_i2c_xfer_continue_if(msg, 2);
#endif
	memcpy(pdata,buf,len);
#ifdef DEBUG
{
	int i;
	
	DBGMSG("vpp_i2c_read(addr 0x%x,index 0x%x,len %d\n",addr,index,len);
	for(i=0;i<len;i+=8){
		DBGMSG("%d : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",i,
			pdata[i],pdata[i+1],pdata[i+2],pdata[i+3],pdata[i+4],pdata[i+5],pdata[i+6],pdata[i+7]);
	}
}
#endif
    return 0;
} /* End of vpp_i2c_read */

int vpp_i2c_enhanced_ddc_read(unsigned int addr,unsigned int index,char *pdata,int len) 
{
	struct i2c_msg msg[3];
	unsigned char buf[len+1];
	unsigned char buf2[2];	

	buf2[0] = 0x1;
	buf2[1] = 0x0;
    msg[0].addr = (0x60 >> 1);
    msg[0].flags = 0 ;
	msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 1;
    msg[0].buf = buf2;

	addr = (addr >> 1);
	memset(buf,0x55,len+1);
    buf[0] = index;
	buf[1] = 0x0;

    msg[1].addr = addr;
    msg[1].flags = 0 ;
	msg[1].flags &= ~(I2C_M_RD);
	msg[1].len = 1;
    msg[1].buf = buf;

	msg[2].addr = addr;
	msg[2].flags = 0 ;
	msg[2].flags |= (I2C_M_RD);
	msg[2].len = len;
	msg[2].buf = buf;

#ifdef CONFIG_I2C_WMT
	wmt_i2c_xfer_continue_if(msg, 3);
#endif
	memcpy(pdata,buf,len);
    return 0;
} /* End of vpp_i2c_enhanced_ddc_read */

/*!*************************************************************************
* vpp_i2c0_read()
* 
* Private Function by Sam Shen, 2009/02/26
*/
/*!
* \brief	read i2c bus0
*		
* \retval  None
*/ 
int vpp_i2c0_read(unsigned int addr,unsigned int index,char *pdata,int len) 
{
#ifdef CONFIG_I2C1_WMT
	extern int wmt_i2c_xfer_continue_if_3(struct i2c_msg *msg, unsigned int num, int bus_id);
#endif
	struct i2c_msg msg[2];
	unsigned char buf[len+1];

	addr = (addr >> 1);	
	memset(buf,0x55,len+1);
    buf[0] = index;
	buf[1] = 0x0;

    msg[0].addr = addr;
    msg[0].flags = 0 ;
	msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 1;
    msg[0].buf = buf;

	msg[1].addr = addr;
	msg[1].flags = 0 ;
	msg[1].flags |= (I2C_M_RD);
	msg[1].len = len;
	msg[1].buf = buf;

#ifdef CONFIG_I2C1_WMT
	wmt_i2c_xfer_continue_if_3(msg, 2, 1);
#endif
	memcpy(pdata,buf,len);
	return 0;
}

int vpp_i2c_bus_read(int id,unsigned int addr,unsigned int index,char *pdata,int len)
{
	int ret = 0;

//	DPRINT("[VPP] vpp_i2c_bus_read(%d,%x,%d,%d)\n",id,addr,index,len);

	switch(id){
		case 0 ... 0xF:	// hw i2c
			{
			unsigned char buf[len+1];
#ifdef CONFIG_I2C_WMT			
			struct i2c_msg msg[2];

			addr = (addr >> 1);	
			memset(buf,0x55,len+1);
		    buf[0] = index;
			buf[1] = 0x0;

		    msg[0].addr = addr;
		    msg[0].flags = 0 ;
			msg[0].flags &= ~(I2C_M_RD);
			msg[0].len = 1;
		    msg[0].buf = buf;

			msg[1].addr = addr;
			msg[1].flags = 0 ;
			msg[1].flags |= (I2C_M_RD);
			msg[1].len = len;
			msg[1].buf = buf;
			ret = wmt_i2c_xfer_continue_if_4(msg,2,id);
#endif
			memcpy(pdata,buf,len);
			}
			break;
		default:
			vo_i2c_proc((id & 0xF),(addr | BIT0),index,pdata,len);
			break;
	}
	return ret;
}

int vpp_i2c_bus_write(int id,unsigned int addr,unsigned int index,char *pdata,int len)
{
	int ret = 0;

//	DPRINT("[VPP] vpp_i2c_bus_write(%d,%x,%d,%d)\n",id,addr,index,len);
	
	switch(id){
		case 0 ... 0xF:	// hw i2c
#ifdef CONFIG_I2C_WMT
			{
		    struct i2c_msg msg[1];
			unsigned char buf[len+1];

			addr = (addr >> 1);
		    buf[0] = index;
			memcpy(&buf[1],pdata,len);
		    msg[0].addr = addr;
		    msg[0].flags = 0 ;
		    msg[0].flags &= ~(I2C_M_RD);
		    msg[0].len = len+1;
		    msg[0].buf = buf;

		    ret = wmt_i2c_xfer_continue_if_4(msg,1,id);
			}
#endif
			break;
		default:
			vo_i2c_proc((id & 0xF),(addr & ~BIT0),index,pdata,len);			
			break;
	}
	return ret;
}

/*!*************************************************************************
* vpp_direct_path_switch()
* 
* Private Function by Sam Shen, 2009/04/13
*/
/*!
* \brief	direct path switch proc function
*		
* \retval  None
*/ 
static int vpp_direct_path_switch(int enable)
{
	if( enable ){
		g_vpp.direct_path_ori_fb = g_vpp.govr->fb_p->fb;
	}
	else {
		g_vpp.govr->fb_p->set_color_fmt(g_vpp.govr->fb_p->fb.col_fmt);
		g_vpp.govr->fb_p->set_csc(g_vpp.govr->fb_p->csc_mode);
	}
	return 0;
} /* End of vpp_direct_path_switch */

/*----------------------- Linux Kernel feature --------------------------------------*/
static int __init vpp_get_boot_arg
(
	char *str			/*!<; // argument string */
)
{
	sscanf(str,"%d:%d:%d:%d:%d:%d",&vpp_vo_boot_arg[0],&vpp_vo_boot_arg[1],&vpp_vo_boot_arg[2],&vpp_vo_boot_arg[3],&vpp_vo_boot_arg[4],&vpp_vo_boot_arg[5]);
	switch( vpp_vo_boot_arg[0] ){
		case VOUT_SD_ANALOG:
		case VOUT_SD_DIGITAL:
		case VOUT_LCD:
		case VOUT_DVI:
		case VOUT_HDMI:
		case VOUT_DVO2HDMI:
		case VOUT_VGA:
			break;
		default:
			vpp_vo_boot_arg[0] = VOUT_MODE_MAX;
			return -1;
	}
	DPRINT("[VPP] vpp boot arg %s opt %d,%d, %dx%d@%d\n",vpp_vout_str[vpp_vo_boot_arg[0]],vpp_vo_boot_arg[1],vpp_vo_boot_arg[2],
														vpp_vo_boot_arg[3],vpp_vo_boot_arg[4],vpp_vo_boot_arg[5]);
  	return 1;
} /* End of lcd_arg_panel_id */
__setup("wmtvo=", vpp_get_boot_arg);

static int __init vpp_get_boot_arg2
(
	char *str			/*!<; // argument string */
)
{
	sscanf(str,"%d:%d:%d",&vpp_vo_boot_arg2[0],&vpp_vo_boot_arg2[1],&vpp_vo_boot_arg2[2]);
	switch( vpp_vo_boot_arg[0] ){
		case VOUT_SD_ANALOG:
		case VOUT_SD_DIGITAL:
		case VOUT_LCD:
		case VOUT_DVI:
		case VOUT_HDMI:
		case VOUT_DVO2HDMI:
		case VOUT_VGA:
			break;
		default:
			vpp_vo_boot_arg2[0] = VOUT_MODE_MAX;
			return -1;
	}
	DPRINT("[VPP] vpp boot arg2 %s opt %d,%d\n",vpp_vout_str[vpp_vo_boot_arg2[0]],vpp_vo_boot_arg2[1],vpp_vo_boot_arg2[2]);
  	return 1;
} /* End of lcd_arg_panel_id */
__setup("wmtvo2=", vpp_get_boot_arg2);

#ifdef CONFIG_PROC_FS
#define CONFIG_VPP_PROC
#ifdef CONFIG_VPP_PROC
unsigned int vpp_proc_value;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static int vpp_do_proc(ctl_table * ctl,int write,void *buffer,size_t * len,loff_t *ppos)
#else
static int vpp_do_proc(ctl_table * ctl,int write,struct file *file,void *buffer,size_t * len,loff_t *ppos)
#endif
{	
	int ret;	

	if( !write ){
		switch( ctl->ctl_name ){
			case 1:
				vpp_proc_value = g_vpp.dbg_msg_level;
				break;
			case 5:
				vpp_proc_value = p_vpu->dei_mode;
				break;
#ifdef WMT_FTBLK_DISP
			case 6:
				vpp_proc_value = p_disp->dac_sense_val;
				break;
#endif
			case 7:
				vpp_proc_value = g_vpp.disp_fb_max;
				break;
			case 8:
				vpp_proc_value = g_vpp.govw_tg_dynamic;
				break;
			case 9:
				vpp_proc_value = g_vpp.govw_skip_all;
				break;
			case 10:
				vpp_proc_value = g_vpp.video_quality_mode;
				break;
			case 11:
				vpp_proc_value = g_vpp.scale_keep_ratio;
				break;
			case 13:
				vpp_proc_value = g_vpp.disp_fb_keep;
				break;
			case 14:
				vpp_proc_value = g_vpp.govrh_field;
				break;
			case 15:
				vpp_proc_value = p_vpu->scale_mode;
				break;
			case 16:
				vpp_proc_value = p_scl->scale_mode;
				break;
			case 17:
				vpp_proc_value = p_vpu->skip_fb;
				break;
			case 18:
				vpp_proc_value = p_vpu->underrun_cnt;
				break;
			case 19:
				vpp_proc_value = g_vpp.vpu_skip_all;
				break;
			case 20:
				vpp_proc_value = g_vpp.govw_hfp;
				break;
			case 21:
				vpp_proc_value = g_vpp.govw_hbp;
				break;
			case 22:
				vpp_proc_value = g_vpp.govw_vfp;
				break;
			case 23:
				vpp_proc_value = g_vpp.govw_vbp;
				break;
			case 24:
				vpp_proc_value = g_vpp.hdmi_audio_interface;
				break;
			case 25:
				vpp_proc_value = g_vpp.hdmi_cp_enable;
				break;
			case 26:
				vpp_proc_value = vpp_get_base_clock(VPP_MOD_GOVRH);
				break;
			case 27:
				vpp_proc_value = g_vpp.fbsync_enable;
				break;
			default:
				break;
		}
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	ret = proc_dointvec(ctl, write, buffer, len, ppos);
#else
	ret = proc_dointvec(ctl, write, file, buffer, len, ppos);
#endif
	if( write ){
		switch( ctl->ctl_name ){
			case 1:
				DPRINT("---------- VPP debug level ----------\n");
				DPRINT("0-disable,255-show all\n");
				DPRINT("1-scale,2-disp fb,3-interrupt,4-TG\n");
				DPRINT("5-ioctl,6-diag,7-deinterlace\n");
				DPRINT("-------------------------------------\n");
				g_vpp.dbg_msg_level = vpp_proc_value;
				break;
#ifdef CONFIG_LCD_WMT			
			case 3:
				lcd_blt_set_level(lcd_blt_id,lcd_blt_level);
				break;
			case 4:
				lcd_blt_set_freq(lcd_blt_id,lcd_blt_freq);
				break;
#endif			
#ifdef WMT_FTBLK_VPU
			case 5:
				p_vpu->dei_mode = vpp_proc_value;
				vpu_dei_set_mode(p_vpu->dei_mode);
				break;
#endif
#ifdef WMT_FTBLK_DISP
			case 6:
				p_disp->dac_sense_val = vpp_proc_value;
				break;
#endif
			case 7:
				g_vpp.disp_fb_max = vpp_proc_value;
				break;
			case 8:
				g_vpp.govw_tg_dynamic = vpp_proc_value;
				break;
			case 9:
				g_vpp.govw_skip_all = vpp_proc_value;
				break;
			case 10:
				g_vpp.video_quality_mode = vpp_proc_value;
				vpp_set_video_quality(g_vpp.video_quality_mode);
				break;
			case 11:
				g_vpp.scale_keep_ratio = vpp_proc_value;
				break;
#ifdef CONFIG_WMT_EDID
			case 12:
				{
					char *edid_buf;
					if( (edid_buf = vout_get_edid(vpp_proc_value)) ){
						if( edid_parse(edid_buf) ){
							DPRINT("*E* read EDID fail\n");
						}
					}
				}
				break;
#endif
			case 13:
				g_vpp.disp_fb_keep = (vpp_proc_value > 3)? 3:vpp_proc_value;
				break;
			case 14:
				g_vpp.govrh_field = (g_vpp.govrh_field == VPP_FIELD_TOP)? VPP_FIELD_BOTTOM:VPP_FIELD_TOP;
				break;
			case 15:
			case 16:
				DPRINT("---------- scale mode ----------\n");
				DPRINT("0-recursive normal\n");
				DPRINT("1-recursive sw bilinear\n");
				DPRINT("2-recursive hw bilinear\n");
				DPRINT("3-realtime noraml (quality but x/32 limit)\n");
				DPRINT("4-realtime bilinear (fit edge but skip line)\n");
				DPRINT("-------------------------------------\n");
				if( ctl->ctl_name == 15 ){
					p_vpu->scale_mode = vpp_proc_value;
				}
				else {
					p_scl->scale_mode = vpp_proc_value;
				}
				break;
			case 17:
				p_vpu->skip_fb = vpp_proc_value;
				break;
			case 18:
				p_vpu->underrun_cnt = vpp_proc_value;
				g_vpp.vpu_skip_all = 0;
				g_vpp.govw_skip_all = 0;
				break;
			case 19:
				g_vpp.vpu_skip_all = vpp_proc_value;
				break;
			case 20:
				g_vpp.govw_hfp = vpp_proc_value;
				break;
			case 21:
				g_vpp.govw_hbp = vpp_proc_value;
				break;
			case 22:
				g_vpp.govw_vfp = vpp_proc_value;
				break;
			case 23:
				g_vpp.govw_vbp = vpp_proc_value;
				break;
			case 24:
				g_vpp.hdmi_audio_interface = vpp_proc_value;
				break;
			case 25:
				{
				g_vpp.hdmi_cp_enable = vpp_proc_value;
				}
				break;
			case 26:
				govrh_set_clock(vpp_proc_value);
				DPRINT("[HDMI] set pixclk %d\n",vpp_proc_value);
				break;
			case 27:
				g_vpp.fbsync_enable = vpp_proc_value;
				break;
			default:
				break;
		}
	}	
	return ret;
}

	struct proc_dir_entry *vpp_proc_dir = 0;

	static ctl_table vpp_table[] = {
	    {
			.ctl_name	= 1,
			.procname	= "dbg_msg",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
	    {
			.ctl_name	= 2,
			.procname	= "dac_sense_en",
			.data		= &vpp_dac_sense_enable,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &proc_dointvec,
		},
#ifdef CONFIG_LCD_BACKLIGHT_WMT
	    {
			.ctl_name	= 3,
			.procname	= "lcd_blt_level",
			.data		= &lcd_blt_level,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
	    {
			.ctl_name	= 4,
			.procname	= "lcd_blt_freq",
			.data		= &lcd_blt_freq,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
#endif
		{
			.ctl_name 	= 5,
			.procname	= "dei_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 6,
			.procname	= "tv_dac_sense_val",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 7,
			.procname	= "disp_fb_max",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 8,
			.procname	= "govw_dynamic_fps",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 9,
			.procname	= "govw_skip_all",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 10,
			.procname	= "video_quality_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 11,
			.procname	= "scale_keep_ratio",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 12,
			.procname	= "vout_edid",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 13,
			.procname	= "disp_fb_keep",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 14,
			.procname	= "govrh_cur_field",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 15,
			.procname	= "vpu_scale_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 16,
			.procname	= "scl_scale_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 17,
			.procname	= "vpu_err_skip",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 18,
			.procname	= "vpu_underrun_cnt",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 19,
			.procname	= "vpu_skip_all",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 20,
			.procname	= "govw_hfp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 21,
			.procname	= "govw_hbp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 22,
			.procname	= "govw_vfp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 23,
			.procname	= "govw_vbp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 24,
			.procname	= "hdmi_audio_interface",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 25,
			.procname	= "hdmi_cp_enable",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 26,
			.procname	= "pixel_clock",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 27,
			.procname	= "govrh_fbsync",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{ .ctl_name = 0 }
	};

	static ctl_table vpp_root_table[] = {
		{
			.ctl_name	= CTL_DEV,
			.procname	= "vpp",	// create path ==> /proc/sys/vpp
			.mode		= 0555,
			.child 		= vpp_table
		},
		{ .ctl_name = 0 }
	};
	static struct ctl_table_header *vpp_table_header;
#endif

/*!*************************************************************************
* vpp_sts_read_proc()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	vpp stauts read proc
*		
* \retval  None
*/ 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static int vpp_sts_read_proc(char *buf, char **start, off_t offset, int len,int *eof,void *data)
#else
static int vpp_sts_read_proc(char *buf, char **start, off_t offset, int len)
#endif
{
	unsigned int yaddr,caddr;
	char *p = buf;
	static struct timeval pre_tv;
	struct timeval tv;
	unsigned int tm_usec;
	
	p += sprintf(p, "--- VPP HW status ---\n");
#ifdef WMT_FTBLK_GOVRH	
	p += sprintf(p, "GOVRH memory read underrun error %d,cnt %d\n",(vppif_reg32_in(REG_GOVRH_INT) & 0x200)?1:0,vpp_govr_underrun_cnt);
	p_govrh->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_GOVW
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "GOVW TG error %d,cnt %d\n",vppif_reg32_read(GOVW_INTSTS_TGERR),vpp_govw_tg_err_cnt);
	p += sprintf(p, "GOVW Y fifo overflow %d\n",vppif_reg32_read(GOVW_INTSTS_MIFYERR));
	p += sprintf(p, "GOVW C fifo overflow %d\n",vppif_reg32_read(GOVW_INTSTS_MIFCERR));
	p_govw->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_GOVM
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "GOVM VPU not ready %d,cnt %d\n",(vppif_reg32_read(GOVM_INTSTS_VPU_READY))?0:1,vpp_govw_vpu_not_ready_cnt);
	p += sprintf(p, "GOVM GE not ready %d,cnt %d\n",(vppif_reg32_read(GOVM_INTSTS_GE_READY))?0:1,vpp_govw_ge_not_ready_cnt);
	p += sprintf(p, "GE not ready G1 %d, G2 %d\n",vppif_reg32_read(GE1_BASE_ADDR+0xF4,BIT0,0),vppif_reg32_read(GE1_BASE_ADDR+0xF4,BIT1,1));
	REG32_VAL(GE1_BASE_ADDR+0xf4) |= 0x3;
#ifdef WMT_FTBLK_PIP
	p += sprintf(p, "GOVM PIP not ready %d\n",(vppif_reg32_read(GOVM_INTSTS_PIP_READY))?0:1);
	p += sprintf(p, "GOVM PIP Y error %d\n",vppif_reg32_read(GOVM_INT_PIP_Y_ERR));
	p += sprintf(p, "GOVM PIP C error %d\n",vppif_reg32_read(GOVM_INT_PIP_C_ERR));
#endif
	p_govm->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_SCL	
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "SCL TG error %d\n",vppif_reg32_read(SCL_INTSTS_TGERR));
	p += sprintf(p, "SCLR MIF1 read error %d\n",vppif_reg32_read(SCLR_INTSTS_R1MIFERR));
	p += sprintf(p, "SCLR MIF2 read error %d\n",vppif_reg32_read(SCLR_INTSTS_R2MIFERR));
	p += sprintf(p, "SCLW RGB fifo overflow %d\n",vppif_reg32_read(SCLW_INTSTS_MIFRGBERR));
	p += sprintf(p, "SCLW Y fifo overflow %d\n",vppif_reg32_read(SCLW_INTSTS_MIFYERR));
	p += sprintf(p, "SCLW C fifo overflow %d\n",vppif_reg32_read(SCLW_INTSTS_MIFCERR));
	p_scl->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_VPU
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "VPU TG error %d\n",vppif_reg32_read(VPU_INTSTS_TGERR));
	p += sprintf(p, "VPUR MIF1 read error %d\n",vppif_reg32_read(VPU_R_INTSTS_R1MIFERR));
	p += sprintf(p, "VPUR MIF2 read error %d\n",vppif_reg32_read(VPU_R2_MIF_ERR));	
	p += sprintf(p, "VPUW Y fifo overflow %d,cnt %d\n",vppif_reg32_read(VPU_W_MIF_YERR),vpp_vpu_y_err_cnt);
//	p += sprintf(p, "VPUW C fifo overflow %d,cnt %d\n",vppif_reg32_read(VPU_W_MIF_CERR),vpp_vpu_c_err_cnt);
	p += sprintf(p, "VPU scale underrun %d,cnt %d\n",vppif_reg32_read(VPU_W_MIF_CERR),vpp_vpu_c_err_cnt);
	p += sprintf(p, "VPUW RGB fifo overflow %d\n",vppif_reg32_read(VPU_W_MIF_RGBERR));
	p += sprintf(p, "VPU MVR fifo overflow %d\n",vppif_reg32_read(VPU_MVR_MIF_ERR));
	p_vpu->clr_sts(VPP_INT_ALL);
#endif

	if( REG32_VAL(GE3_BASE_ADDR+0x50) < vppif_reg32_read(GOVM_DISP_X_CR) ){
		p += sprintf(p, "*E* GE resx %d < GOV resx %d\n",REG32_VAL(GE3_BASE_ADDR+0x50),vppif_reg32_read(GOVM_DISP_X_CR));
	}
	if( REG32_VAL(GE3_BASE_ADDR+0x54) < vppif_reg32_read(GOVM_DISP_Y_CR) ){
		p += sprintf(p, "*E* GE resy %d < GOV resy %d\n",REG32_VAL(GE3_BASE_ADDR+0x54),vppif_reg32_read(GOVM_DISP_Y_CR));
	}
	p += sprintf(p, "G1 Enable %d,G2 Enable %d\n",REG32_VAL(GE3_BASE_ADDR+0xa8),REG32_VAL(GE3_BASE_ADDR+0xac));

	p += sprintf(p, "--- VPP fb Address ---\n");
#ifdef WMT_FTBLK_VPU
	vpu_r_get_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "VPU fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_VPU_R_Y1SA,yaddr,REG_VPU_R_C1SA,caddr);
#else
	sclr_get_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "VPU fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_SCLR_YSA,yaddr,REG_SCLR_CSA,caddr);
#endif

#ifdef WMT_FTBLK_GOVW
	govw_get_hd_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "GOVW fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_GOVW_HD_YSA,yaddr,REG_GOVW_HD_CSA,caddr);	
#endif
#ifdef WMT_FTBLK_GOVRH
	govrh_get_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "GOVRH fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_GOVRH_YSA,yaddr,REG_GOVRH_CSA,caddr);	
#endif
	p += sprintf(p, "--- VPP SW status ---\n");
	p += sprintf(p, "direct path %d,ge direct path %d\n",g_vpp.direct_path,g_vpp.ge_direct_path);
	
	do_gettimeofday(&tv);
	tm_usec=(tv.tv_sec==pre_tv.tv_sec)? (tv.tv_usec-pre_tv.tv_usec):(1000000*(tv.tv_sec-pre_tv.tv_sec)+tv.tv_usec-pre_tv.tv_usec);
	p += sprintf(p, "Time period %d usec\n",(int) tm_usec);
	p += sprintf(p, "GOVR fps %d,GOVW fps %d\n",(1000000*g_vpp.dbg_govrh_vbis_cnt/tm_usec),(1000000*g_vpp.dbg_govw_vbis_cnt/tm_usec));
	pre_tv = tv;
	
	p += sprintf(p, "GOVW VBIS INT cnt %d\n",g_vpp.dbg_govw_vbis_cnt);
	p += sprintf(p, "GOVW PVBI INT cnt %d (toggle dual buf)\n",g_vpp.dbg_govw_pvbi_cnt);
	p += sprintf(p, "GOVW TG ERR INT cnt %d\n",vpp_govw_tg_err_cnt);

	p += sprintf(p, "--- disp fb status ---\n");
	p += sprintf(p, "DISP fb isr cnt %d\n",g_vpp.dbg_dispfb_isr_cnt);
	p += sprintf(p, "queue max %d,full cnt %d\n",g_vpp.disp_fb_max,g_vpp.dbg_dispfb_full_cnt);
	p += sprintf(p, "VPU disp fb cnt %d, skip %d\n",vpp_vpu_disp_cnt,vpp_vpu_disp_skip_cnt);
	p += sprintf(p, "PIP disp fb cnt %d\n",vpp_pip_disp_cnt);
#ifdef WMT_FTBLK_PIP
	p += sprintf(p, "Queue cnt disp:%d,pip %d\n",vpp_disp_fb_cnt(&vpp_disp_fb_list),vpp_disp_fb_cnt(&vpp_pip_fb_list));
#else
	p += sprintf(p, "Queue cnt disp:%d\n",vpp_disp_fb_cnt(&vpp_disp_fb_list));
#endif

	g_vpp.dbg_govrh_vbis_cnt = 0;
	g_vpp.dbg_govw_vbis_cnt = 0;
	g_vpp.dbg_govw_pvbi_cnt = 0;
	g_vpp.dbg_dispfb_isr_cnt = 0;
	g_vpp.dbg_dispfb_full_cnt = 0;
	vpp_vpu_disp_cnt = 0;
	vpp_pip_disp_cnt = 0;
	vpp_govw_tg_err_cnt = 0;
	vpp_vpu_disp_skip_cnt = 0;
	vpp_govw_vpu_not_ready_cnt = 0;
	vpp_govw_ge_not_ready_cnt = 0;
	vpp_vpu_y_err_cnt = 0;
	vpp_vpu_c_err_cnt = 0;
	vpp_govr_underrun_cnt = 0;
	
	return (p - buf);
} /* End of vpp_sts_read_proc */

/*!*************************************************************************
* vpp_reg_read_proc()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	vpp register read proc
*		
* \retval  None
*/ 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static int vpp_reg_read_proc(char *buf, char **start, off_t offset, int len,int *eof,void *data)
#else
static int vpp_reg_read_proc(char *buf,char **start,off_t offset,int len)
#endif
{
	char *p = buf;
	vpp_mod_base_t *mod_p;
	int i;

	DPRINT("Product ID:0x%x\n",vpp_get_chipid());
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->dump_reg ){
			mod_p->dump_reg();
		}
	}
#ifdef 	WMT_FTBLK_HDMI
	hdmi_reg_dump();
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_reg_dump();
#endif
	
	p += sprintf(p, "Dump VPP HW register by kernel message\n");
	
	return (p-buf);
} /* End of vpp_reg_read_proc */

void vpp_show_fb(vdo_framebuf_t *fb)
{
	DPRINT("--- [VPP] show fb ---\n");
	DPRINT("img(%d,%d), fb(%d,%d)\n",fb->img_w,fb->img_h,fb->fb_w,fb->fb_h);
	DPRINT("colfmt %s,bpp %d\n",vpp_colfmt_str[fb->col_fmt],fb->bpp);
	DPRINT("Y addr 0x%x, size %d\n",fb->y_addr,fb->y_size);
	DPRINT("C addr 0x%x, size %d\n",fb->c_addr,fb->c_size);
	DPRINT("crop (%d,%d),flag 0x%x\n",fb->h_crop,fb->v_crop,fb->flag);
}

static char *vpp_show_module(vpp_mod_t mod,char *p)
{
	vpp_mod_base_t *mod_p;
	vpp_fb_base_t *fb_p;
	vdo_framebuf_t *fb;

	mod_p = vpp_mod_get_base(mod);
	p += sprintf(p, "int catch 0x%x\n",mod_p->int_catch);
	
	fb_p = mod_p->fb_p;
	if( fb_p ){
		fb = &fb_p->fb;
		p += sprintf(p, "----- frame buffer -----\n");
		p += sprintf(p, "Y addr 0x%x, size %d\n",fb->y_addr,fb->y_size);
		p += sprintf(p, "C addr 0x%x, size %d\n",fb->c_addr,fb->c_size);
		p += sprintf(p, "W %d, H %d, FB W %d, H %d\n",fb->img_w,fb->img_h,fb->fb_w,fb->fb_h);
		p += sprintf(p, "bpp %d, color fmt %s\n",fb->bpp,vpp_colfmt_str[fb->col_fmt]);
		p += sprintf(p, "H crop %d, V crop %d\n",fb->h_crop,fb->v_crop);

		p += sprintf(p, "CSC mode %d,frame rate %d\n",fb_p->csc_mode,fb_p->framerate);
		p += sprintf(p, "media fmt %d,wait ready %d\n",fb_p->media_fmt,fb_p->wait_ready);
	}
	return p;
}

/*!*************************************************************************
* vpp_info_read_proc()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	vpp infomation read proc
*		
* \retval  None
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static int vpp_info_read_proc(char *buf, char **start, off_t offset, int len,int *eof,void *data)
#else
static int vpp_info_read_proc(char *buf,char **start,off_t offset,int len)
#endif
{
	char *p = buf;

	p += sprintf(p, "========== VPP ==========\n");
	p += sprintf(p, "direct path %d,ge direct path %d\n",g_vpp.direct_path,g_vpp.ge_direct_path);
	p += sprintf(p, "mb0 0x%x,mb1 0x%x\n",g_vpp.mb[0],g_vpp.mb[1]);

#ifdef WMT_FTBLK_GOVRH
	p += sprintf(p, "========== GOVRH ==========\n");
	p += sprintf(p, "VGA DAC SENSE cnt %d\n",p_govrh->vga_dac_sense_cnt);
	p = vpp_show_module(VPP_MOD_GOVRH,p);
#endif

	p += sprintf(p, "========== GOVW ==========\n");
	p = vpp_show_module(VPP_MOD_GOVW,p);

	p += sprintf(p, "========== GOVM ==========\n");
	p += sprintf(p, "path 0x%x\n",p_govm->path);
	p = vpp_show_module(VPP_MOD_GOVM,p);

	p += sprintf(p, "========== VPU ==========\n");
	p += sprintf(p, "visual res (%d,%d),pos (%d,%d)\n",p_vpu->resx_visual,p_vpu->resy_visual,p_vpu->posx,p_vpu->posy);
	p = vpp_show_module(VPP_MOD_VPU,p);

	p += sprintf(p, "========== SCLR ==========\n");
	p = vpp_show_module(VPP_MOD_SCL,p);

	p += sprintf(p, "========== SCLW ==========\n");
	p = vpp_show_module(VPP_MOD_SCLW,p);
	return (p-buf);
} /* End of vpp_info_read_proc */
#endif

/*!*************************************************************************
* vpp_set_audio()
* 
* Private Function by Sam Shen, 2010/08/05
*/
/*!
* \brief	set audio parameters
*		
* \retval  None
*/ 
void vpp_set_audio(int format,int sample_rate,int channel)
{
	vout_audio_t info;

	DPRINT("[VPP] set audio(fmt %d,rate %d,ch %d)\n",format,sample_rate,channel);
	info.fmt = format;
	info.sample_rate = sample_rate;
	info.channel = channel;
	vout_set_audio(&info);
}

/*!*************************************************************************
* vpp_get_info()
* 
* Private Function by Sam Shen, 2009/08/06
*/
/*!
* \brief	get current vpp info
*		
* \retval  None
*/ 
void vpp_get_info(struct fb_var_screeninfo *var)
{
	var->xres = vfb_var.xres;
	var->yres = vfb_var.yres;
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres;
	var->pixclock = vfb_var.pixclock;
}

/*!*************************************************************************
* vpp_fbsync_cal_fps()
* 
* Private Function by Sam Shen, 2010/10/28
*/
/*!
* \brief	calucate govrh fb sync with media fps
*		
* \retval  None
*/ 
#ifdef CONFIG_VPP_GOVRH_FBSYNC
void vpp_fbsync_cal_fps(void)
{
	unsigned int gcd,total,frame;

	gcd = vpp_get_gcd(p_govrh->fb_p->framerate,p_govw->fb_p->framerate);
	total = p_govrh->fb_p->framerate / gcd;
	frame = p_govw->fb_p->framerate / gcd;

	if( frame > total ) frame = total;
	g_vpp.fbsync_frame = frame;
	g_vpp.fbsync_step = total / frame;
	g_vpp.fbsync_substep = ( total % frame )? (frame / (total % frame)):0;
	if( g_vpp.fbsync_step == g_vpp.fbsync_substep ){
		g_vpp.fbsync_step = g_vpp.fbsync_step + 1;
		g_vpp.fbsync_substep = total - (g_vpp.fbsync_step * frame);
	}
	
	g_vpp.fbsync_cnt = 1;
	g_vpp.fbsync_vsync = g_vpp.fbsync_step;
	if( vpp_check_dbg_level(VPP_DBGLVL_SYNCFB) ){
		char buf[50];

		DPRINT("[VPP] govrh fps %d,govw fps %d,gcd %d\n",p_govrh->fb_p->framerate,p_govw->fb_p->framerate,gcd);
		DPRINT("[VPP] total %d,frame %d\n",total,frame);
		sprintf(buf,"sync frame %d,step %d,sub %d",g_vpp.fbsync_frame,g_vpp.fbsync_step,g_vpp.fbsync_substep);
		vpp_dbg_show(VPP_DBGLVL_SYNCFB,3,buf);
	}

	// patch for pre interrupt interval not enough bug
	if(vppif_reg32_read(GOVRH_PVBI_LINE) < 0x4){
		vppif_reg32_write(GOVRH_PVBI_LINE,0x4);
	}
}
#endif							

/*!*************************************************************************
* vpp_disp_fb_cnt()
* 
* Private Function by Sam Shen, 2009/03/05
*/
/*!
* \brief	clear display frame buffer queue
*		
* \retval  0 - success
*/ 
int vpp_disp_fb_cnt(struct list_head *list)
{
	struct list_head *ptr;
	int cnt;
	
	vpp_lock();
	cnt = 0;
	ptr = list;
	while( ptr->next != list ){
		ptr = ptr->next;
		cnt++;
	}
	vpp_unlock();
	return cnt;
}

/*!*************************************************************************
* vpp_disp_fb_compare()
* 
* Private Function by Sam Shen, 2010/11/25
*/
/*!
* \brief	compare two frame buffer info without Y/C address and size
*		
* \retval  0 - not match, 1 - same
*/ 
int vpp_disp_fb_compare(vdo_framebuf_t *fb1,vdo_framebuf_t *fb2)
{
	if( fb1->img_w != fb2->img_w ) return 0;
	if( fb1->img_h != fb2->img_h ) return 0;
	if( fb1->fb_w != fb2->fb_w ) return 0;
	if( fb1->fb_h != fb2->fb_h ) return 0;
	if( fb1->col_fmt != fb2->col_fmt ) return 0;
	if( fb1->h_crop != fb2->h_crop ) return 0;
	if( fb1->v_crop != fb2->v_crop ) return 0;
	if( fb1->flag != fb2->flag ) return 0;
	return 1;
} /* End of vpp_disp_fb_compare */

/*!*************************************************************************
* vpp_disp_fb_add()
* 
* Private Function by Sam Shen, 2009/02/02
*/
/*!
* \brief	add display frame to display queue
*		
* \retval  None
*/ 
static int vpp_disp_fb_add
(
	vpp_dispfb_t *fb		/*!<; // display frame pointer */
)
{
	vpp_dispfb_parm_t *entry;
	struct list_head *ptr;
	unsigned int yaddr,caddr;
	struct list_head *fb_list;

#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
	if( g_vpp.govw_tg_dynamic ){
		g_vpp.govw_tg_rtn_cnt = 0;
		vpp_govw_dynamic_tg_set_rcyc(g_vpp.govw_tg_rcyc);
	}
#endif				

	if( list_empty(&vpp_disp_free_list) ){
		return -1;
	}

	vpp_lock();
	if( (fb->flag & VPP_FLAG_DISPFB_PIP) == 0 ){
		fb_list = &vpp_disp_fb_list;
		if( g_vpp.disp_fb_cnt >= g_vpp.disp_fb_max ){
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"*W* add dispfb full max %d cnt %d",g_vpp.disp_fb_max,g_vpp.disp_fb_cnt);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,1,buf);	
			}
			g_vpp.dbg_dispfb_full_cnt++;
			vpp_unlock();
			return -1;
		}
		g_vpp.disp_fb_cnt++;
	}
#ifdef WMT_FTBLK_PIP
	else {
		fb_list = &vpp_pip_fb_list;
	}
#endif

	ptr = vpp_disp_free_list.next;
	entry = list_entry(ptr,vpp_dispfb_parm_t,list);
	list_del_init(ptr);
	entry->parm = *fb;

#if 1	// patch for VPU bilinear mode
	if( ((fb->flag & VPP_FLAG_DISPFB_PIP) == 0) && !(g_vpp.direct_path)){
		if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
			if( vpp_disp_fb_compare(&entry->parm.info,&p_vpu->fb_p->fb) ){
				entry->parm.flag &= ~VPP_FLAG_DISPFB_INFO;
				entry->parm.flag |= VPP_FLAG_DISPFB_ADDR;
				entry->parm.yaddr = entry->parm.info.y_addr;
				entry->parm.caddr = entry->parm.info.c_addr;
			}
		}
	}
#endif	

	memcpy(&entry->pts,&g_vpp.frame_pts,sizeof(vpp_pts_t));
	list_add_tail(&entry->list,fb_list);
	yaddr = caddr = 0;
	if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
		yaddr = entry->parm.yaddr;
		caddr = entry->parm.caddr;
	}
	else if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
		yaddr = entry->parm.info.y_addr;
		caddr = entry->parm.info.c_addr;
	}

	if( yaddr ){
		yaddr = (unsigned int)phys_to_virt(yaddr);
		mb_get(yaddr);
	}
	
	if( caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE)){
		caddr = (unsigned int)phys_to_virt(caddr);
		mb_get(caddr);
	}

	if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
		char buf[50];

		sprintf(buf,"add dispfb Y 0x%x C 0x%x",yaddr,caddr);
		vpp_dbg_show(VPP_DBGLVL_DISPFB,1,buf);
	}
	
	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_add */

void vpp_disp_fb_free(unsigned int y_addr,unsigned int c_addr)
{
	#define FREE_QUEUE_MAX	4
	static int cnt = 0;
	static unsigned int y_addr_queue[FREE_QUEUE_MAX];
	static unsigned int c_addr_queue[FREE_QUEUE_MAX];
	int i;

	if( y_addr == 0 ){
		for(i=0;i<cnt;i++){
			y_addr = y_addr_queue[i];
			c_addr = c_addr_queue[i];
			if( y_addr ) mb_put(y_addr);
			if( c_addr ) mb_put(c_addr);
			y_addr_queue[i] = c_addr_queue[i] = 0;
		}
		cnt = 0;
	}
	else {
		y_addr_queue[cnt] = y_addr;
		c_addr_queue[cnt] = c_addr;
		cnt++;		
		if( cnt > g_vpp.disp_fb_keep ){
			y_addr = y_addr_queue[0];
			c_addr = c_addr_queue[0];
			cnt--;
			
			for(i=0;i<cnt;i++){
				y_addr_queue[i] = y_addr_queue[i+1];
				c_addr_queue[i] = c_addr_queue[i+1];
			}

			if( y_addr ) mb_put(y_addr);
			if( c_addr ) mb_put(c_addr);
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free dispfb Y 0x%x C 0x%x,keep %d",y_addr,c_addr,cnt);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
	}
}

/*!*************************************************************************
* vpp_disp_fb_clr()
* 
* Private Function by Sam Shen, 2009/03/05
*/
/*!
* \brief	clear display frame buffer queue
*		
* \retval  0 - success
*/ 
static int vpp_disp_fb_clr(int pip)
{
	vpp_dispfb_parm_t *entry;
	struct list_head *fb_list,*ptr;
	unsigned int yaddr,caddr;

	vpp_lock();

#ifdef WMT_FTBLK_PIP	
	fb_list = (pip)? &vpp_pip_fb_list:&vpp_disp_fb_list;
	yaddr = (pip)? p_pip->pre_yaddr:vpp_pre_dispfb_y_addr;
	caddr = (pip)? p_pip->pre_caddr:vpp_pre_dispfb_c_addr;

	if( yaddr ) mb_put(yaddr);
	if( caddr )	mb_put(caddr);
	if( pip ){
		p_pip->pre_yaddr = 0;
		p_pip->pre_caddr = 0;
	}
	else {
		vpp_disp_fb_free(0,0);
		vpp_pre_dispfb_y_addr = 0;
		vpp_pre_dispfb_c_addr = 0;
		g_vpp.disp_fb_cnt = 0;
	}
#else
	fb_list = &vpp_disp_fb_list;
	yaddr = vpp_pre_dispfb_y_addr;
	caddr = vpp_pre_dispfb_c_addr;
	vpp_pre_dispfb_y_addr = 0;
	vpp_pre_dispfb_c_addr = 0;
	if( yaddr ) mb_put(yaddr);
	if( caddr ) mb_put(caddr);
	g_vpp.disp_fb_cnt = 0;		
#endif
	
	while( !list_empty(fb_list) ){
		ptr = fb_list->next;
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_disp_free_list);
		if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
			yaddr = entry->parm.yaddr;
			caddr = entry->parm.caddr;
		}
		else if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
			yaddr = entry->parm.info.y_addr;
			caddr = entry->parm.info.c_addr;
		}

		if( yaddr ){
			yaddr = (unsigned int)phys_to_virt(yaddr);
			mb_put(yaddr);
		}
		
		if( caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE)){
			caddr = (unsigned int)phys_to_virt(caddr);
			mb_put(caddr);
		}
	}
	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_clr */

/*!*************************************************************************
* vpp_disp_fb_isr()
* 
* Private Function by Sam Shen, 2009/02/02
*/
/*!
* \brief	interrupt service for display frame
*		
* \retval  status
*/ 
static int vpp_disp_fb_isr(void)
{
	vpp_mod_t mod;
	vpp_dispfb_parm_t *entry;
	struct list_head *ptr;
	static int isr_cnt = 0;

	vpp_lock();

#ifdef CONFIG_VPP_DISPFB_FREE_POSTPONE
	if( vpp_pre_dispfb_y_addr ){
		vpp_disp_fb_free(vpp_pre_dispfb_y_addr,vpp_pre_dispfb_c_addr);
		vpp_pre_dispfb_y_addr = 0;
		vpp_pre_dispfb_c_addr = 0;
	}
#endif
	
	ptr = 0;
	isr_cnt++;
#ifdef CONFIG_VPP_GOVRH_FBSYNC
	g_vpp.fbsync_isrcnt++;
	if( g_vpp.fbsync_enable ){
		if( g_vpp.direct_path ){
			if( isr_cnt < g_vpp.fbsync_vsync ){
				vpp_unlock();
				return 0;
			}
		}
	}
#endif
	g_vpp.dbg_dispfb_isr_cnt++;
	if( !list_empty(&vpp_disp_fb_list) ){
		unsigned int yaddr,caddr;

		yaddr = caddr = 0;		
		ptr = vpp_disp_fb_list.next;
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		memcpy(&g_vpp.govw_pts,&entry->pts,sizeof(vpp_pts_t));

		if( g_vpp.vpu_skip_all ){
			vpp_vpu_disp_skip_cnt++;			
		}
		else {
			if( g_vpp.direct_path ){
				mod = VPP_MOD_GOVRH;
#ifdef CONFIG_VPP_GOVRH_FBSYNC
				if( g_vpp.fbsync_enable ){
					g_vpp.fbsync_cnt++;
					isr_cnt=0;					
					if( g_vpp.fbsync_cnt > g_vpp.fbsync_frame ){
						g_vpp.fbsync_cnt = 1;
						g_vpp.fbsync_vsync = g_vpp.fbsync_step;
						g_vpp.fbsync_isrcnt = 0;
						if( g_vpp.fbsync_substep < 0 ){
							if( g_vpp.fbsync_cnt <= ( g_vpp.fbsync_substep * (-1))){
								g_vpp.fbsync_vsync -= 1;
							}
						}
					}
					else {
						g_vpp.fbsync_vsync = g_vpp.fbsync_step;
						if( g_vpp.fbsync_substep < 0 ){
							// if( g_vpp.fbsync_cnt > (g_vpp.fbsync_frame + g_vpp.fbsync_substep )){
							if( g_vpp.fbsync_cnt <= ( g_vpp.fbsync_substep * (-1))){
								g_vpp.fbsync_vsync -= 1;
							}
						}
						else if( g_vpp.fbsync_substep ){
							if( (g_vpp.fbsync_cnt % g_vpp.fbsync_substep) == 0 ){
								g_vpp.fbsync_vsync += 1;
							}
						}
					}
					
					if( vpp_check_dbg_level(VPP_DBGLVL_SYNCFB) ){
						char buf[50];

						sprintf(buf,"sync frame %d,sync cnt %d,vsync %d,isr %d",g_vpp.fbsync_frame,
							g_vpp.fbsync_cnt,g_vpp.fbsync_vsync,g_vpp.fbsync_isrcnt );
						vpp_dbg_show(VPP_DBGLVL_SYNCFB,3,buf);
					}
				}
#endif				
			}
			else {
				mod = VPP_MOD_VPU;
#ifdef WMT_FTBLK_VPU
				vpu_set_reg_update(VPP_FLAG_DISABLE);
#else
				scl_set_reg_update(VPP_FLAG_DISABLE);
#endif
				govm_set_reg_update(VPP_FLAG_DISABLE);
			}

			if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
				vpp_fb_base_t *mod_fb_p;
				
				yaddr = entry->parm.yaddr;
				caddr = entry->parm.caddr;
				mod_fb_p = vpp_mod_get_fb_base(mod);
				mod_fb_p->set_addr(yaddr,caddr);
				mod_fb_p->fb.y_addr = yaddr;
				mod_fb_p->fb.c_addr = caddr;
			}

			if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
				vpp_fb_base_t *mod_fb_p;

				mod_fb_p = vpp_mod_get_fb_base(mod);
				yaddr = entry->parm.info.y_addr;
				caddr = entry->parm.info.c_addr;
				if( g_vpp.direct_path ){
#ifdef WMT_FTBLK_GOVRH
					vpp_display_format_t field;

					if( entry->parm.info.col_fmt != mod_fb_p->fb.col_fmt ){
						mod_fb_p->fb.col_fmt = entry->parm.info.col_fmt;
						govrh_set_data_format(mod_fb_p->fb.col_fmt);
						mod_fb_p->set_csc(mod_fb_p->csc_mode);
					}
					field = (entry->parm.info.flag & VDO_FLAG_INTERLACE)?VPP_DISP_FMT_FIELD:VPP_DISP_FMT_FRAME;
					govrh_set_source_format(field);
					govrh_set_fb_addr(yaddr,caddr);
					mod_fb_p->fb = entry->parm.info;
#endif
				}
				else {
					mod_fb_p->fb = entry->parm.info;
					mod_fb_p->set_framebuf(&mod_fb_p->fb);
				}
			}

			if( entry->parm.flag & VPP_FLAG_DISPFB_VIEW ){
				vpp_set_video_scale(&entry->parm.view);
			}

			if( mod == VPP_MOD_VPU ){
#ifdef WMT_FTBLK_VPU
				vpu_set_reg_update(VPP_FLAG_ENABLE);
#else
				scl_set_reg_update(VPP_FLAG_ENABLE);
#endif
				govm_set_reg_update(VPP_FLAG_ENABLE);
			}
		}

		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_disp_free_list);
		g_vpp.disp_fb_cnt--;

		if( g_vpp.vpu_skip_all == 0 ){		
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"show disp fb Y 0x%x C 0x%x",yaddr,caddr);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}

#ifdef CONFIG_VPP_VBIE_FREE_MB
			if( vpp_pre_dispfb_y_addr )
				DPRINT("[VPP] *W* pre dispfb not free\n");

			vpp_pre_dispfb_y_addr = vpp_cur_dispfb_y_addr;
			vpp_pre_dispfb_c_addr = vpp_cur_dispfb_c_addr;
			vpp_cur_dispfb_y_addr = (yaddr)? ((unsigned int) phys_to_virt(yaddr)):0;
			vpp_cur_dispfb_c_addr = (caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? ((unsigned int) phys_to_virt(caddr)):0;
#else
#ifndef CONFIG_VPP_DISPFB_FREE_POSTPONE
			vpp_disp_fb_free(vpp_pre_dispfb_y_addr,vpp_pre_dispfb_c_addr);
#endif
			vpp_pre_dispfb_y_addr = vpp_cur_dispfb_y_addr;
			vpp_pre_dispfb_c_addr = vpp_cur_dispfb_c_addr;
			vpp_cur_dispfb_y_addr = (yaddr)? ((unsigned int) phys_to_virt(yaddr)):0;
			vpp_cur_dispfb_c_addr = (caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? ((unsigned int) phys_to_virt(caddr)):0;
#endif		
			vpp_vpu_disp_cnt++;
		}
	}
	
#ifdef CONFIG_VPP_DYNAMIC_DEI
	#define VPP_DEI_CHECK_PERIOD	150

	if( vppif_reg32_read(VPU_DEI_ENABLE) && (p_vpu->dei_mode == VPP_DEI_DYNAMIC) ){
		static int dei_cnt = VPP_DEI_CHECK_PERIOD;
		static unsigned int weave_sum,bob_sum;
		static unsigned int pre_weave_sum,pre_bob_sum;
		unsigned int usum,vsum;
		static vpp_deinterlace_t dei_mode = 0;
		unsigned int weave_diff,bob_diff,cur_diff;
		
		switch( dei_cnt ){
			case 2:
				if( dei_mode != VPP_DEI_ADAPTIVE_ONE ){
					g_vpp.govw_skip_frame = 1;
				}
				vpu_dei_set_mode(VPP_DEI_ADAPTIVE_ONE);
				break;
			case 1:
				vpu_dei_get_sum(&weave_sum,&usum,&vsum);
				if( dei_mode != VPP_DEI_FIELD ){
					g_vpp.govw_skip_frame = 1;
				}
				vpu_dei_set_mode(VPP_DEI_FIELD);				
				break;
			case 0:
				vpu_dei_get_sum(&bob_sum,&usum,&vsum);
				if( (vpp_calculate_diff(bob_sum,pre_bob_sum)<100000) 
					&& (vpp_calculate_diff(weave_sum,pre_weave_sum)<100000)){
					dei_mode = VPP_DEI_WEAVE;
				}
				else {
					dei_mode = ( bob_sum > (2*weave_sum) )? VPP_DEI_FIELD:VPP_DEI_ADAPTIVE_ONE;
				}
				bob_diff = vpp_calculate_diff(bob_sum,pre_bob_sum);
				weave_diff = vpp_calculate_diff(weave_sum,pre_weave_sum);
				cur_diff = vpp_calculate_diff(weave_sum,bob_sum);
				pre_bob_sum = bob_sum;
				pre_weave_sum = weave_sum;
				vpu_dei_set_mode(dei_mode);
				dei_cnt = VPP_DEI_CHECK_PERIOD;
				if( vpp_check_dbg_level(VPP_DBGLVL_DEI) ){
					static vpp_deinterlace_t pre_mode = 0;
					DPRINT("[VPP] bob %d,weave %d,diff bob %d,weave %d,cur %d\n",bob_sum,weave_sum,bob_diff,weave_diff,cur_diff);
					if( pre_mode != dei_mode ){
						DPRINT("[VPP] dei mode %d -> %d\n",pre_mode,dei_mode);
						pre_mode = dei_mode;
					}
				}
				break;
			default:
				break;
		}
		dei_cnt--;
	}
#endif

#ifdef WMT_FTBLK_PIP
	if( !list_empty(&vpp_pip_fb_list) ){
		unsigned int yaddr,caddr;
		
		ptr = vpp_pip_fb_list.next;
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		
		yaddr = caddr = 0;
		if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
			yaddr = entry->parm.yaddr;
			caddr = entry->parm.caddr;
			p_pip->fb_p->set_addr(yaddr,caddr);
		}

		if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
			yaddr = entry->parm.info.y_addr;
			caddr = entry->parm.info.c_addr;
			p_pip->fb_p->fb = entry->parm.info;
			p_pip->fb_p->set_framebuf(&p_pip->fb_p->fb);
		}

		if( entry->parm.flag & VPP_FLAG_DISPFB_VIEW ){
			p_pip->fb_p->fn_view(VPP_FLAG_WR,&entry->parm.view);
		}

		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_disp_free_list);

		if( p_pip->pre_yaddr ) mb_put(p_pip->pre_yaddr);
		if( p_pip->pre_caddr ) mb_put(p_pip->pre_caddr);
		p_pip->pre_yaddr = (yaddr)? ((unsigned int) phys_to_virt(yaddr)):0;
		p_pip->pre_caddr = (caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? ((unsigned int) phys_to_virt(caddr)):0;
		vpp_pip_disp_cnt++;
	}
#endif
	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_isr */

/*!*************************************************************************
* vpp_govw_int_routine()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	govw interrupt routine
*		
* \retval  None
*/ 
void vpp_govw_int_routine(void)
{
#ifdef CONFIG_VPP_DUAL_BUFFER
	unsigned int govr_y,govr_c;
	unsigned int govw_y,govw_c;

	if( g_vpp.dbg_govw_fb_cnt ){
		g_vpp.dbg_govw_fb_cnt--;
		if( g_vpp.dbg_govw_fb_cnt == 0 ){
			vpp_set_govw_tg(VPP_FLAG_DISABLE);
		}
	}

	if( g_vpp.govw_skip_all ){
		return;
	}

	if( g_vpp.govw_skip_frame ){
		g_vpp.govw_skip_frame--;
		vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVW skip");
		return;
	}

	govr_y = g_vpp.govr->fb_p->fb.y_addr;
	govr_c = g_vpp.govr->fb_p->fb.c_addr;
	govw_y = p_govw->fb_p->fb.y_addr;
	govw_c = p_govw->fb_p->fb.c_addr;
	
	g_vpp.govr->fb_p->set_addr(govw_y,govw_c);
	p_govw->fb_p->set_addr(govr_y,govr_c);
	
	g_vpp.govr->fb_p->fb.y_addr = govw_y;
	g_vpp.govr->fb_p->fb.c_addr = govw_c;
	p_govw->fb_p->fb.y_addr = govr_y;
	p_govw->fb_p->fb.c_addr = govr_c;
	memcpy(&g_vpp.disp_pts,&g_vpp.govw_pts,sizeof(vpp_pts_t));
#endif	
} /* End of vpp_govw_int_routine */

/*!*************************************************************************
* vpp_irqproc_get_no()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	get vpp int no from a mask
*		
* \retval  irq no
*/ 
int vpp_irqproc_get_no(vpp_int_t vpp_int)
{
	int i;
	unsigned int mask;

	if( vpp_int == 0 )
		return 0xFF;
		
	for(i=0,mask=0x1;i<32;i++,mask<<=1){
		if( vpp_int & mask )
			break;
	}
	return i;
} /* End of vpp_irqproc_get_no */

/*!*************************************************************************
* vpp_irqproc_get_entry()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	get irqproc entry
*		
* \retval  entry pointer
*/ 
vpp_irqproc_t *vpp_irqproc_get_entry(vpp_int_t vpp_int)
{
	int no;

	no = vpp_irqproc_get_no(vpp_int);
	if( no >= 32 ) 
		return 0;
	return vpp_irqproc_array[no];
} /* End of vpp_irqproc_get_entry */

/*!*************************************************************************
* vpp_irqproc_do_tasklet()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	irqproc tasklet
*		
* \retval  None
*/ 
static void vpp_irqproc_do_tasklet
(
	unsigned long data		/*!<; // tasklet input data */
)
{
	vpp_proc_t *entry;
	struct list_head *ptr;
	vpp_irqproc_t *irqproc;

	vpp_lock();
	irqproc = vpp_irqproc_get_entry(data);
	if( irqproc ){
		do {
			if( list_empty(&irqproc->list) )
				break;

			/* get task from work head queue */
			ptr = (&irqproc->list)->next;
			entry = list_entry(ptr,vpp_proc_t,list);
			if( entry->func ){
				if( entry->func(entry->arg) )
					break;
			}
			list_del_init(ptr);
			list_add_tail(&entry->list,&vpp_free_list);
			up(&entry->sem);
//			DPRINT("[VPP] vpp_irqproc_do_tasklet\n");
		} while(1);
	}
	vpp_unlock();
} /* End of vpp_irqproc_do_tasklet */

/*!*************************************************************************
* vpp_irqproc_new()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	Create a irqproc entry and init
*		
* \retval  Status
*/ 
int vpp_irqproc_new
(
	vpp_int_t vpp_int,
	void (*proc)(int arg)
)
{
	int no;
	vpp_irqproc_t *irqproc;

	no = vpp_irqproc_get_no(vpp_int);
	if( no >= 32 ) 
		return 1;
	
	irqproc = kmalloc(sizeof(vpp_irqproc_t),GFP_KERNEL);
	vpp_irqproc_array[no] = irqproc;
	INIT_LIST_HEAD(&irqproc->list);
	tasklet_init(&irqproc->tasklet,vpp_irqproc_do_tasklet,vpp_int);
	irqproc->proc = proc;
	return 0;
} /* End of vpp_irqproc_new */

/*!*************************************************************************
* vpp_irqproc_work()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	Insert a task to work in vpp irq and wait complete
*		
* \retval  Status
*/ 
int vpp_irqproc_work
(
	vpp_int_t type,
	int (*func)(void *argc),
	void *arg,
	int wait
)
{
	int ret;
	vpp_proc_t *entry;
	struct list_head *ptr;
	vpp_irqproc_t *irqproc;

	if( list_empty(&vpp_free_list) ){
		DPRINT("[VPP] *E* proc array empty\n");
		return -1;
	}

//	DPRINT("[VPP] vpp_irqproc_work(type 0x%x,wait %d)\n",type,wait);

	if( vppm_get_int_enable(type) == 0 ){
		DPRINT("[VPP] *W* irqproc interrupt not enable 0x%x\n",type);
		if( func ) func(arg);
		return 0;
	}

	ret = 0;
	vpp_lock();
	
	ptr = vpp_free_list.next;
	entry = list_entry(ptr,vpp_proc_t,list);
	list_del_init(ptr);
	entry->func = func;
	entry->arg = arg;
	entry->type = type;
	init_MUTEX(&entry->sem);
	down(&entry->sem);
	
	irqproc = vpp_irqproc_get_entry(type);
	if( irqproc ){
		list_add_tail(&entry->list,&irqproc->list);
	}
	else {
		irqproc = vpp_irqproc_array[31];
		list_add_tail(&entry->list,&irqproc->list);
	}
	vpp_unlock();
	if( wait ) {
		ret = down_timeout(&entry->sem,HZ);
		if( ret ){
			DPRINT("[VPP] *W* vpp_irqproc_work timeout(type 0x%x)\n",type);
			list_del_init(ptr);
			list_add_tail(ptr,&vpp_free_list);
		}
	}
//	DPRINT("[VPP] vpp_irqproc_work end\n");
	return ret;
} /* End of vpp_irqproc_work */

#ifdef CONFIG_VPP_GE_DIRECT_PATH
void vpp_irqproc_direct_path(int arg)
{
	vdo_framebuf_t *fb;
	
	fb = &p_govrh->fb_p->fb;
	govrh_set_reg_update(0);
	govrh_set_data_format(fb->col_fmt);
	govrh_set_csc_mode(p_govrh->fb_p->csc_mode);
	govrh_set_fb_info(fb->fb_w, fb->img_w, fb->h_crop, fb->v_crop);
	govrh_set_fb_addr(fb->y_addr, fb->c_addr);
	govrh_set_reg_update(1);
//	DPRINT("[VPP] irqproc direct_path %d\n",arg);
}
#endif

#ifdef WMT_FTBLK_GOVRH
void vpp_irqproc_govrh_vbis(int arg)
{
	if( arg == 0 ){
		g_vpp.dbg_govrh_vbis_cnt++;
		g_vpp.govrh_field = (g_vpp.govrh_field == VPP_FIELD_TOP)? VPP_FIELD_BOTTOM:VPP_FIELD_TOP;
		if( vppif_reg32_read(GOVRH_TG_MODE) && (g_vpp.govrh_field != VPP_FIELD_TOP) ){
			/* interlace bottom field */
		}
		else {
			vout_plug_detect(VOUT_VGA);
		}
	}
}

void vpp_irqproc_govrh_pvbi(int arg)
{
	if( arg == 0 ){
#ifdef CONFIG_VPP_GE_DIRECT_PATH
		if( g_vpp.direct_path_chg ){
			vpp_irqproc_direct_path(g_vpp.direct_path);
			g_vpp.direct_path_chg = 0;
		}
#endif

		if( p_cursor->chg_flag ){
			govrh_irqproc_set_position(0);
			p_cursor->chg_flag = 0;
		}

		if( g_vpp.direct_path ){
			vpp_disp_fb_isr();
		}
	}
}
#endif

void vpp_irqproc_govw_pvbi(int arg)
{
	if( arg == 0 ){
#ifndef CONFIG_GOVW_FBSWAP_VBIE				
		vpp_govw_int_routine();
		vpp_disp_fb_isr();
#endif			
	}

	if( arg == 1 ){
		g_vpp.dbg_govw_pvbi_cnt++;
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
		vpp_govw_dynamic_tg(0);
#endif
#ifdef CONFIG_VPP_VBIE_FREE_MB
		if( vpp_free_y_addr ) {
			if( vpp_free_y_addr == vpp_cur_dispfb_y_addr ){
				DPRINT("[VPP] *W* cur & free fb are same 0x%x\n",vpp_free_y_addr);
			}
			mb_put(vpp_free_y_addr);
		}
		if( vpp_free_c_addr ) mb_put(vpp_free_c_addr);

		if( vpp_free_y_addr ){				
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free disp fb Y 0x%x C 0x%x",vpp_free_y_addr,vpp_free_c_addr);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
		vpp_free_y_addr = 0;
		vpp_free_c_addr = 0;				
#endif
#if 0
		if( vpp_check_dbg_level(VPP_DBGLVL_INT) ){
			if( (vppif_reg32_in(REG_VPP_INTSTS) & BIT1) || (int_sts & VPP_INT_GOVW_VBIS) ){
				DPRINT("[VPP] *W* pvbi over vbi\n");
			}
		}
#endif
	}
}

void vpp_irqproc_govw_vbis(int arg)
{
	if( arg == 0 ){
		
	}

	if( arg == 1 ){
		g_vpp.dbg_govw_vbis_cnt++;
	}
}
	
void vpp_irqproc_govw_vbie(int arg)
{
	if( arg == 0 ){
#ifdef CONFIG_VPP_VBIE_FREE_MB
		vpp_free_y_addr = vpp_pre_dispfb_y_addr;
		vpp_pre_dispfb_y_addr = 0;
		vpp_free_c_addr = vpp_pre_dispfb_c_addr;
		vpp_pre_dispfb_c_addr = 0;
#endif
#ifdef CONFIG_GOVW_FBSWAP_VBIE
		vpp_govw_int_routine();
		vpp_disp_fb_isr();
#endif				
	}

	if( arg == 1 ){
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
		vpp_govw_dynamic_tg(0);
#endif
		vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVW VBIE");
	}
}

#ifdef WMT_FTBLK_DISP /* sw patch : interlace tv mode field swap */
void vpp_irqproc_disp_vbis(int arg)
{
	if( arg == 0 ){
		if( vppif_reg32_read(DISP_INPUT_FIELD) ){
			if( disp_get_cur_field() ){
				vppif_reg32_out(GOVRH_BASE1_ADDR+0xe8,0x2);					
			}
			else {
				vppif_reg32_out(GOVRH_BASE1_ADDR+0xe8,0x3);
			}
		}
		vout_plug_detect(VOUT_SD_ANALOG);
	}
}
#endif

int vpp_irqproc_enable_govw(void *arg)
{
	vpp_set_govw_tg((vpp_flag_t)arg);
	return 0;
}

#ifdef WMT_FTBLK_DISP
int vpp_irqproc_disable_disp(void *arg)
{
	if( disp_get_cur_field() ){
		vppif_reg32_write(DISP_EN,0);
		return 0;
	}
	return 1;
}
#endif

int vpp_irqproc_enable_vpu(void *arg)
{
	vpu_set_tg_enable((vpp_flag_t)arg);
	return 0;
}

/*!*************************************************************************
* vpp_interrupt_routine()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp interrupt routine
*		
* \retval  None
*/ 
static irqreturn_t vpp_interrupt_routine
(
	int irq, 				/*!<; // irq id */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	void *dev_id 			/*!<; // device id */
#else	
	void *dev_id, 			/*!<; // device id */
	struct pt_regs *regs	/*!<; // reg pointer */
#endif
)
{
	vpp_int_t int_sts;

	switch(irq){
#ifdef WMT_FTBLK_VPU
		case VPP_IRQ_VPU:
			int_sts = p_vpu->get_sts();
			p_vpu->clr_sts(int_sts);
            vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] VPU INT",int_sts);
			g_vpp.govw_skip_frame = p_vpu->skip_fb;
			if( int_sts & VPP_INT_ERR_VPUW_MIFY )
				vpp_vpu_y_err_cnt++;
			if( int_sts & VPP_INT_ERR_VPUW_MIFC ){
				vpp_vpu_c_err_cnt++;
				if(p_vpu->underrun_cnt){
					p_vpu->underrun_cnt--;
					if(p_vpu->underrun_cnt==0){
						// g_vpp.govw_skip_all = 1;
						g_vpp.dbg_govw_fb_cnt = 1;
						g_vpp.vpu_skip_all = 1;
						DPRINT("[VPP] *E* skip all GOVW & VPU fb\n");
					}
				}
			}
			break;
#endif
		case VPP_IRQ_VPPM:	/* VPP */
			int_sts = p_vppm->get_sts();
			p_vppm->clr_sts(int_sts);

			{
				int i;
				unsigned int mask;
				vpp_irqproc_t *irqproc;

				for(i=0,mask=0x1;(i<32) && int_sts;i++,mask<<=1){
					if( (int_sts & mask) == 0 )
						continue;

					if( (irqproc = vpp_irqproc_array[i]) ){
						irqproc->proc(0);	// pre irq handle
						
						if( list_empty(&irqproc->list) == 0 )
							tasklet_schedule(&irqproc->tasklet);

						irqproc->proc(1);	// post irq handle
					}
					else {
						irqproc = vpp_irqproc_array[31];
						if( list_empty(&irqproc->list) == 0 ){
							vpp_proc_t *entry;
							struct list_head *ptr;

							ptr = (&irqproc->list)->next;
							entry = list_entry(ptr,vpp_proc_t,list);
							if( entry->type == mask )
								tasklet_schedule(&irqproc->tasklet);
						}							
					}
					int_sts &= ~mask;
				}
			}
			break;
#ifdef WMT_FTBLK_SCL
		case VPP_IRQ_SCL:	/* SCL */
			int_sts = p_scl->get_sts();
			p_scl->clr_sts(int_sts);
            vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] SCL INT",int_sts);
			break;
#endif			
		case VPP_IRQ_GOVM:	/* GOVM */
			int_sts = p_govm->get_sts();
			p_govm->clr_sts(int_sts);
			vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] GOVM INT",int_sts);
			break;
		case VPP_IRQ_GOVW:	/* GOVW */
			int_sts = p_govw->get_sts();
			vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] GOVW INT",int_sts);
			vpp_govw_tg_err_cnt++;			
			if( int_sts & VPP_INT_ERR_GOVW_TG ){
				vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] GOVW TG err",vpp_govw_tg_err_cnt);
				if( vppif_reg32_read(GOVM_INTSTS_GE_READY) == 0 ){
					vpp_govw_ge_not_ready_cnt++;
				}
				if( vppif_reg32_read(GOVM_INTSTS_VPU_READY) == 0 ){
					vpp_govw_vpu_not_ready_cnt++;
				}
			}
			p_govw->clr_sts(int_sts);
#ifdef CONFIG_VPP_GOVW_TG_ERR_DROP_FRAME
			if( vpp_disp_fb_cnt(&vpp_disp_fb_list) > 1 ){
				vpp_disp_fb_isr();		// drop display frame when bandwidth not enouth
				vpp_vpu_disp_skip_cnt++;
				vpp_dbg_show(VPP_DBGLVL_DISPFB,0,"skip disp fb");
			}
#endif	
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
			else {
				vpp_govw_dynamic_tg(1);
			}
#endif
#ifdef VPP_DBG_DIAG_NUM
			vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVW Err");
			vpp_dbg_diag_delay = 10;
#endif
			g_vpp.govw_skip_frame = 1;
			break;
#ifdef WMT_FTBLK_GOVRH
		case VPP_IRQ_GOVR:	/* GOVR */
			int_sts = p_govrh->get_sts();
			p_govrh->clr_sts(int_sts);
			vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] GOVR INT",int_sts);
			vpp_govr_underrun_cnt++;
#ifdef VPP_DBG_DIAG_NUM
			vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVR MIF Err");
			vpp_dbg_diag_delay = 10;
#endif
			break;
#endif			
		default:
			DPRINT("*E* invalid vpp isr\n");
			break;
	}
	return IRQ_HANDLED;
} /* End of vpp_interrupt_routine */

/*!*************************************************************************
* vpp_alloc_framebuffer()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp alloc frame buffer
*		
* \retval  None
*/ 
int vpp_alloc_framebuffer(unsigned int resx,unsigned int resy)
{
	vdo_framebuf_t *fb;
	unsigned int y_size;
	unsigned int fb_size;

	if( g_vpp.mb[0] ){
		if( (g_vpp.govr->fb_p->fb.fb_w == resx) && (g_vpp.govr->fb_p->fb.fb_h == resy) ){
			return 0;
		}
		mb_free((unsigned int) phys_to_virt(g_vpp.mb[0]));
		mb_free((unsigned int) phys_to_virt(g_vpp.mb[1]));
	}

	if( resx % 64 ){
		resx += (64 - (resx % 64));
	}
	y_size = resx * resy;
	fb_size = (g_vpp.govr->fb_p->fb.col_fmt==VDO_COL_FMT_ARGB)? (4 * y_size):(3 * y_size);
	y_size = (g_vpp.govr->fb_p->fb.col_fmt==VDO_COL_FMT_ARGB)? (fb_size):(y_size);
	g_vpp.mb[0] = (unsigned int) virt_to_phys((void *)mb_allocate(fb_size));
#ifdef CONFIG_VPP_DUAL_BUFFER
	g_vpp.mb[1] = (unsigned int) virt_to_phys((void *)mb_allocate(fb_size));
#else
	g_vpp.mb[1] = g_vpp.mb[0];
#endif

	fb = &p_govw->fb_p->fb;
	fb->y_addr = g_vpp.mb[0];
	fb->c_addr = g_vpp.mb[0] + y_size;
	fb->y_size = y_size;
	fb->c_size = fb_size - y_size;
	fb->fb_w = resx;
	fb->fb_h = resy;

	fb = &g_vpp.govr->fb_p->fb;
	fb->y_addr = g_vpp.mb[1];
	fb->c_addr = g_vpp.mb[1] + y_size;
	fb->y_size = y_size;
	fb->c_size = fb_size - y_size;
	fb->fb_w = resx;
	fb->fb_h = resy;

#if 1
	if( g_vpp.chg_res_blank ){
		int i;
		unsigned int *ptr;
		unsigned int yaddr,caddr;
		
	//	vpp_dbg_show(VPP_DBGLVL_ALL,1,"clr fb begin");
		g_vpp.govw_skip_all = 1;

		govw_get_hd_fb_addr(&yaddr,&caddr);
//		DPRINT("[VPP] govw fb y(0x%x),c(0x%x)\n",yaddr,caddr);
//		DPRINT("[VPP] clr fb y(0x%x,%d),c(0x%x,%d)\n",fb->y_addr,fb->y_size,fb->c_addr,fb->c_size); mdelay(100);

		ptr = phys_to_virt(fb->y_addr);
		for(i=0;i<fb->y_size;i+=4,ptr++)
			*ptr = 0x0;
		ptr = phys_to_virt(fb->c_addr);
		for(i=0;i<fb->c_size;i+=4,ptr++)
			*ptr = 0x80808080;
	//	memset(phys_to_virt(fb->y_addr),0,fb->y_size);
	//	memset(phys_to_virt(fb->c_addr),0,fb->c_size);
	//	vpp_dbg_show(VPP_DBGLVL_ALL,1,"clr fb end");

	}
#endif

//	DBGMSG("alloc frame buf %dx%d, size %d\n",resx,resy,y_size*4);
//	DBGMSG("mb0 0x%x,mb1 0x%x\n",g_vpp.mb[0],g_vpp.mb[1]);
	return 0;
} /* End of vpp_alloc_framebuffer */

/*!*************************************************************************
* vpp_get_sys_parameter()
* 
* Private Function by Sam Shen, 2010/01/27
*/
/*!
* \brief	vpp device initialize
*		
* \retval  None
*/ 
void vpp_get_sys_parameter(void)
{
	unsigned char buf[40];
	int varlen = 40;
	extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

	if( wmt_getsyspara("wmt.display.direct_path",buf,&varlen) == 0 ){
		sscanf(buf,"%d",&g_vpp.direct_path);
		DPRINT("[VPP] direct path %d\n",g_vpp.direct_path);
	}
} /* End of vpp_get_sys_parameter */

int vpp_request_irq(unsigned int irq_no,void *routine,char *name)
{
	if ( request_irq(irq_no,routine,SA_INTERRUPT,name,(void *)&g_vpp) ) {
		DPRINT("[VPP] *E* request irq %s fail\n",name);
		return -1;
	}
	return 0;
}

/*----------------------- Linux Kernel interface --------------------------------------*/
/*!*************************************************************************
* vpp_dev_init()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp device initialize
*		
* \retval  None
*/ 
int vpp_dev_init(void)
{
	vpp_mod_base_t *mod_p;
	int i;
	unsigned int mod_mask;

	vpp_get_sys_parameter();

	g_vpp.disp_fb_max = VPP_DISP_FB_NUM;
	g_vpp.disp_fb_cnt = 0;
	g_vpp.govw_tg_dynamic = 0;
	g_vpp.video_quality_mode = 1;
	g_vpp.scale_keep_ratio = 1;
	g_vpp.govrh_field = VPP_FIELD_BOTTOM;
	g_vpp.disp_fb_keep = 0;
	g_vpp.fbsync_enable = 1;

	// init irq proc	
	INIT_LIST_HEAD(&vpp_free_list);

	vpp_irqproc_new(VPP_INT_MAX,0);
#ifdef WMT_FTBLK_GOVRH
	vpp_irqproc_new(VPP_INT_GOVRH_PVBI,vpp_irqproc_govrh_pvbi);
	vpp_irqproc_new(VPP_INT_GOVRH_VBIS,vpp_irqproc_govrh_vbis);
#endif
	vpp_irqproc_new(VPP_INT_GOVW_PVBI,vpp_irqproc_govw_pvbi);
	vpp_irqproc_new(VPP_INT_GOVW_VBIS,vpp_irqproc_govw_vbis);
	vpp_irqproc_new(VPP_INT_GOVW_VBIE,vpp_irqproc_govw_vbie);
#ifdef WMT_FTBLK_DISP
	vpp_irqproc_new(VPP_INT_DISP_VBIS,vpp_irqproc_disp_vbis);
#endif

	for(i=0;i<VPP_PROC_NUM;i++)
		list_add_tail(&vpp_proc_array[i].list,&vpp_free_list);

	// init disp fb queue
	INIT_LIST_HEAD(&vpp_disp_free_list);
	INIT_LIST_HEAD(&vpp_disp_fb_list);
#ifdef WMT_FTBLK_PIP
	INIT_LIST_HEAD(&vpp_pip_fb_list);
#endif
	for(i=0;i<VPP_DISP_FB_MAX;i++)
		list_add_tail(&vpp_disp_fb_array[i].list,&vpp_disp_free_list);

	// init module
//	p_scl->int_catch = VPP_INT_ERR_SCL_TG | VPP_INT_ERR_SCLR1_MIF | VPP_INT_ERR_SCLR2_MIF 
//						| VPP_INT_ERR_SCLW_MIFRGB | VPP_INT_ERR_SCLW_MIFY |	VPP_INT_ERR_SCLW_MIFC;
#ifdef CONFIG_GOVW_FBSWAP_VBIE
	p_vppm->int_catch = VPP_INT_GOVW_VBIS | VPP_INT_GOVW_VBIE;
#else
	p_vppm->int_catch = VPP_INT_GOVW_PVBI | VPP_INT_GOVW_VBIS;
#endif

#ifdef CONFIG_VPP_VBIE_FREE_MB
	p_vppm->int_catch |= VPP_INT_GOVW_VBIE;
#endif

#ifdef CONFIG_VPP_GOVW_TG_ERR_DROP_FRAME			
	p_govw->int_catch = VPP_INT_ERR_GOVW_TG | VPP_INT_ERR_GOVW_MIFY | VPP_INT_ERR_GOVW_MIFC;
#endif
//	p_govm->int_catch = VPP_INT_ERR_GOVM_VPU | VPP_INT_ERR_GOVM_GE;

#ifdef WMT_FTBLK_GOVRH
	g_vpp.govr = (vpp_mod_base_t*) p_govrh;
#elif defined(WMT_FTBLK_LCDC)
	g_vpp.govr = (vpp_mod_base_t*) p_lcdc;
#endif

	vpp_alloc_framebuffer(VPP_HD_MAX_RESX,VPP_HD_MAX_RESY);

	/* check govrh preinit for uboot logo function */
#ifdef WMT_FTBLK_GOVRH
	g_vpp.govrh_preinit = vppif_reg32_read(GOVRH_MIF_ENABLE);
	if( g_vpp.govrh_preinit ){
		p_vppm->int_catch &= ~VPP_INT_GOVW_PVBI;
	}
	
	if( g_vpp.direct_path ){
	    p_vppm->int_catch |= (VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS);
	}
#else	
	g_vpp.govrh_preinit = 0;
#endif
	DPRINT("vpp_init(boot logo %d)\n",g_vpp.govrh_preinit);

	// init video out module first
	if( g_vpp.govrh_preinit == 0 ){
		mod_mask = BIT(VPP_MOD_GOVRS) | BIT(VPP_MOD_GOVRH) | BIT(VPP_MOD_DISP) | BIT(VPP_MOD_LCDC);
		for(i=0;i<VPP_MOD_MAX;i++){
			if( !(mod_mask & (0x01 << i)) ){
				continue;
			}
			mod_p = vpp_mod_get_base(i);
			if( mod_p && mod_p->init ){
				mod_p->init(mod_p);
			}
		}
	}
	
	// init other module
	mod_mask =  BIT(VPP_MOD_GOVW) | BIT(VPP_MOD_GOVM) | BIT(VPP_MOD_SCL) | BIT(VPP_MOD_SCLW) | BIT(VPP_MOD_VPU) |
				BIT(VPP_MOD_VPUW) | BIT(VPP_MOD_PIP) | BIT(VPP_MOD_VPPM);
	for(i=0;i<VPP_MOD_MAX;i++){
		if( !(mod_mask & (0x01 << i)) ){
			continue;
		}
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->init ){
			mod_p->init(mod_p);
		}
	}

#ifdef CONFIG_VPP_INTERRUPT
	// init interrupt service routine
#ifdef WMT_FTBLK_SCL
	if ( request_irq(VPP_IRQ_SCL, vpp_interrupt_routine, SA_INTERRUPT, "scl", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}
#endif	

	if ( request_irq(VPP_IRQ_VPPM, vpp_interrupt_routine, SA_INTERRUPT, "vpp", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}

	if ( request_irq(VPP_IRQ_GOVM, vpp_interrupt_routine, SA_INTERRUPT, "govm", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}

	if ( request_irq(VPP_IRQ_GOVW, vpp_interrupt_routine, SA_INTERRUPT, "govw", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}

#ifdef WMT_FTBLK_GOVRH
	if ( request_irq(VPP_IRQ_GOVR, vpp_interrupt_routine, SA_INTERRUPT, "govr", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}
#endif	
#ifdef WMT_FTBLK_VPU
	if ( request_irq(VPP_IRQ_VPU, vpp_interrupt_routine, SA_INTERRUPT, "vpu", (void *)&g_vpp) ) {
		DPRINT("*E* request VPU ISR fail\n");
		return -1;
	}
#endif	
#endif

	// init video out device
	vout_init(&vfb_var);
	govm_set_disp_coordinate(vfb_var.xres,vfb_var.yres);
	vpp_set_video_quality(g_vpp.video_quality_mode);

#ifdef CONFIG_VPP_PROC
	// init system proc
	if( vpp_proc_dir == 0 ){
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
		struct proc_dir_entry *res;
		
		vpp_proc_dir = proc_mkdir("driver/vpp", NULL);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
		vpp_proc_dir->owner = THIS_MODULE;
#endif
		
		res=create_proc_entry("sts", 0, vpp_proc_dir);
	    if (res) {
    	    res->read_proc = vpp_sts_read_proc;
	    }
		res=create_proc_entry("reg", 0, vpp_proc_dir);
	    if (res) {
    	    res->read_proc = vpp_reg_read_proc;
	    }
		res=create_proc_entry("info", 0, vpp_proc_dir);
	    if (res) {
    	    res->read_proc = vpp_info_read_proc;
	    }
		vpp_table_header = register_sysctl_table(vpp_root_table);
#else
		
		vpp_proc_dir = proc_mkdir("driver/vpp", NULL);
		vpp_proc_dir->owner = THIS_MODULE;
		create_proc_info_entry("sts", 0, vpp_proc_dir, vpp_sts_read_proc);
		create_proc_info_entry("reg", 0, vpp_proc_dir, vpp_reg_read_proc);
		create_proc_info_entry("info", 0, vpp_proc_dir, vpp_info_read_proc);		

		vpp_table_header = register_sysctl_table(vpp_root_table, 1);
#endif	
	}
#endif
	return 0;
} /* End of vpp_dev_init */
module_init(vpp_dev_init);

/*!*************************************************************************
* vpp_exit()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp device exit
*		
* \retval  None
*/ 
int vpp_exit(struct fb_info *info)
{
	DBGMSG("vpp_exit\n");

	vout_exit();
	unregister_sysctl_table(vpp_table_header);

	return 0;
} /* End of vpp_exit */

/*!*************************************************************************
* vpp_backup_reg()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp register backup
*		
* \retval  None
*/ 
unsigned int *vpp_backup_reg(unsigned int addr,unsigned int size)
{
	u32 *ptr;
	int i;

	size += 4;
	ptr = (u32*) kmalloc(size,GFP_KERNEL);
	for(i=0;i<size;i+=4){
		ptr[i/4] = REG32_VAL(addr+i);
	}
	return ptr;
} /* End of vpp_backup_reg */

/*!*************************************************************************
* vpp_restore_reg()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp register restore
*		
* \retval  None
*/ 
int vpp_restore_reg(unsigned int addr,unsigned int size,unsigned int *reg_ptr)
{
	int i;

	if( reg_ptr == NULL )
		return 0;

	size += 4;
	for(i=0;i<size;i+=4){
		REG32_VAL(addr+i) = reg_ptr[i/4];
	}
	kfree(reg_ptr);
	reg_ptr = 0;
	return 0;
} /* End of vpp_restore_reg */

/*!*************************************************************************
* vpp_suspend()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp suspend
*		
* \retval  None
*/ 
int	vpp_suspend(int state)
{
	vpp_mod_base_t *mod_p;
	int i;

	DPRINT("vpp_suspend\n");
	vout_suspend(VOUT_MODE_ALL,state);

	// disable module
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->suspend ){
			mod_p->suspend(0);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_suspend(0);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_suspend(0);
#endif
	// wait
	mdelay(100);

	// disable tg
	for(i=0;i<VPP_MOD_MAX;i++){
		if( i == VPP_MOD_GOVW )
			continue;
		
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->suspend ){
			mod_p->suspend(1);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_suspend(1);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_suspend(1);
#endif	
	// backup registers
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->suspend ){
			mod_p->suspend(2);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_suspend(2);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_suspend(2);
#endif
	mdelay(100);
	p_govw->suspend(1);
	return 0;
} /* End of vpp_suspend */

/*!*************************************************************************
* vpp_resume()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver resume
*		
* \retval  None
*/ 
int	vpp_resume(void)
{
	vpp_mod_base_t *mod_p;
	int i;

	DPRINT("vpp_resume\n");

	// restore registers
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->resume ){
			mod_p->resume(0);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_resume(0);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_resume(0);
#endif
	// enable tg
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->resume ){
			mod_p->resume(1);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_resume(1);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_resume(1);
#endif
	// wait
	mdelay(100);

	// enable module
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->resume ){
			mod_p->resume(2);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_resume(2);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_resume(2);
#endif
	vout_resume(VOUT_MODE_ALL,0);
	return 0;
} /* End of vpp_resume */

/*!*************************************************************************
* vpp_check_mmap_offset()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	check mmap offset
*		
* \retval  None
*/ 
int vpp_check_mmap_offset(dma_addr_t offset)
{
	vdo_framebuf_t *fb;
	int i;

//	DBGMSG("vpp_check_mmap_offset 0x%x\r\n",offset);

	for(i=0;i<VPP_MOD_MAX;i++){
		fb = vpp_mod_get_framebuf(i);
		if( fb ){
			if( (offset >= fb->y_addr) && (offset < (fb->y_addr + fb->y_size))){
//				DBGMSG("mmap to mod %d Y frame buffer\r\n",i);
				return 0;
			}

			if( (offset >= fb->c_addr) && (offset < (fb->c_addr + fb->c_size))){
//				DBGMSG("mmap to mod %d C frame buffer\r\n",i);
				return 0;
			}
		}
	}
	return -1;
} /* End of vpp_check_mmap_offset */

/*!*************************************************************************
* vpp_mmap()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver mmap
*		
* \retval  None
*/ 
int vpp_mmap(struct vm_area_struct *vma)
{
//	DBGMSG("vpp_mmap\n");
	
	/* which buffer need to remap */
	if( vpp_check_mmap_offset(vma->vm_pgoff << PAGE_SHIFT) != 0 ){
		DPRINT("*E* vpp_mmap 0x%x\n",(int) vma->vm_pgoff << PAGE_SHIFT);
		return -EINVAL;
	}

//	DBGMSG("Enter vpp_mmap remap 0x%x\n",(int) (vma->vm_pgoff << PAGE_SHIFT));
	
	vma->vm_flags |= VM_IO | VM_RESERVED;
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
} /* End of vpp_mmap */

/*!*************************************************************************
* vpp_wait_vsync()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	wait govw frame ready
*		
* \retval  None
*/ 
void vpp_wait_vsync(void)
{
	if( g_vpp.direct_path || g_vpp.ge_direct_path ){
		vpp_irqproc_work(VPP_INT_GOVRH_VBIS,0,0,1);
	}
	else {
		vpp_irqproc_work(VPP_INT_GOVW_VBIS,0,0,1);
	}
	return;
} /* End of vpp_wait_vsync */

void vpp_clr_framebuf(vpp_mod_t mod)
{
	vdo_color_fmt colfmt;
	unsigned int yaddr,caddr;
	unsigned int ysize,csize;
	unsigned int fb_w,act_w,xoff,yoff;
	unsigned int fb_h;

	switch(mod){
		case VPP_MOD_GOVRH:
			colfmt = govrh_get_color_format();
			govrh_get_fb_addr(&yaddr,&caddr);
			govrh_get_fb_info(&fb_w,&act_w,&xoff,&yoff);
			fb_h = p_govrh->fb_p->fb.fb_h;
			break;
		case VPP_MOD_VPU:
			colfmt = vpu_r_get_color_format();
			vpu_r_get_fb_addr(&yaddr,&caddr);
			vpu_r_get_fb_info(&fb_w,&act_w,&xoff,&yoff);
			fb_h = p_vpu->fb_p->fb.fb_h;
			break;
		default:
			return;
	}

	ysize = fb_w * fb_h;
	switch(colfmt){
		case VDO_COL_FMT_ARGB:
			ysize = 4 * ysize;
			csize = 0;
			break;
		case VDO_COL_FMT_YUV444:
			csize = 2 * ysize;
			break;
		case VDO_COL_FMT_YUV422H:
			csize = ysize;
			break;
		case VDO_COL_FMT_YUV420:
		default:			
			csize = ysize / 2;
			break;
	}

	if( ysize ){
		memset(phys_to_virt(yaddr),0,ysize);
	}

	if( csize ){
		memset(phys_to_virt(caddr),0x80,csize);
	}
	DPRINT("[VPP] clr fb Y(0x%x,%d) C(0x%x,%d)\n",yaddr,ysize,caddr,csize);
}

/*!*************************************************************************
* vpp_config()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp config for resolution change
*		
* \retval  None
*/ 
int vpp_config(struct fb_info *info)
{
	vout_info_t vo_info;
	vdo_framebuf_t *fb;

#if(WMT_SUB_PID == WMT_PID_8505)
	if( (info->var.xres > 1024) || (info->var.yres > 600) ){
		DPRINT("*E* WM8505 not support (%dx%d)\n",info->var.xres,info->var.yres);
		return -1;
	}
#endif

	down(&vpp_sem);
	if( g_vpp.chg_res_blank ){
		g_vpp.govw_skip_all = 1;
	}

	if( vppif_reg32_read(GOVW_TG_ENABLE) && vppif_reg32_read(VPP_GOVW_INTEN_PVBI)){
		vpp_irqproc_work(VPP_INT_GOVW_PVBI,(void *)vpp_irqproc_enable_govw,0,1);
	}

	if( g_vpp.chg_res_blank ){
		vpp_clr_framebuf(VPP_MOD_GOVRH);
	}

#ifdef WMT_FTBLK_GOVRH
	if ( !g_vpp.govrh_preinit ){
		govrh_set_tg_enable(VPP_FLAG_DISABLE);
	}
#endif

	g_vpp.resx = info->var.xres;
	g_vpp.resy = info->var.yres;
	vo_info.resx = info->var.xres;
	vo_info.resy = info->var.yres;
	vo_info.timing.pixel_clock = info->var.pixclock;
	vo_info.bpp = info->var.bits_per_pixel;
	vo_info.fps = info->var.pixclock / (info->var.xres * info->var.yres);
	if( info->var.pixclock == (info->var.xres * info->var.yres * vo_info.fps) ){
		vo_info.timing.pixel_clock = vo_info.fps;
	}
	if( vo_info.fps == 0 ) vo_info.fps = VPP_VOUT_FRAMERATE_DEFAULT;
	DBGMSG("vpp_config(%dx%d@%d),pixclock %d\n",vo_info.resx,vo_info.resy,vo_info.fps,info->var.pixclock);
	vout_config((g_vpp.govrh_preinit)? VOUT_BOOT:VOUT_MODE_ALL,&vo_info);
	if( (vo_info.resx != info->var.xres) || (vo_info.resy != info->var.yres) ){
		DBGMSG("vout mode update (%dx%d)\n",vo_info.resx,vo_info.resy);
		info->var.xres = vo_info.resx;
		info->var.yres = vo_info.resy;
	}
	if( g_vpp.direct_path ){
		g_vpp.govr->fb_p->fb = g_vpp.direct_path_ori_fb;
	}
	vpp_alloc_framebuffer(vo_info.resx,vo_info.resy);
	g_vpp.govr->fb_p->fb.img_w = info->var.xres;
	g_vpp.govr->fb_p->fb.img_h = info->var.yres;
	g_vpp.govr->fb_p->framerate = vo_info.fps;
//fan , modify for u-boot logo
	if ( g_vpp.govrh_preinit == 0)
		g_vpp.govr->fb_p->set_framebuf(&g_vpp.govr->fb_p->fb);
//
#ifdef WMT_FTBLK_GOVRH	
	p_govrh->vga_dac_sense_cnt = vo_info.fps * VPP_DAC_SENSE_SECOND;
#endif	
	p_govw->fb_p->fb.img_w = info->var.xres;
	p_govw->fb_p->fb.img_h = info->var.yres;
	p_govw->fb_p->set_framebuf(&p_govw->fb_p->fb);

	if( g_vpp.direct_path ){
		g_vpp.direct_path_ori_fb = g_vpp.govr->fb_p->fb;
		vpp_set_govw_tg(VPP_FLAG_DISABLE);
	}
	else {
		vpp_set_govw_tg(VPP_FLAG_ENABLE);
	}

	fb = &p_vpu->fb_p->fb;
	if( fb->y_addr ){ 
		p_vpu->resx_visual = (fb->fb_w > info->var.xres)? info->var.xres:fb->fb_w;
		p_vpu->resy_visual = (fb->fb_h > info->var.yres)? info->var.yres:fb->fb_h;
	}
	else {
		fb->fb_w = fb->img_w = p_vpu->resx_visual = info->var.xres;
		fb->fb_h = fb->img_h = p_vpu->resy_visual = info->var.yres;
	}
	p_vpu->fb_p->set_framebuf(fb);
	govm_set_vpu_coordinate(p_vpu->posx,p_vpu->posy,p_vpu->posx+p_vpu->resx_visual_scale-1,p_vpu->posy+p_vpu->resy_visual_scale-1);

//	vpp_wait_vsync();
//	vpp_wait_vsync();
#ifdef WMT_FTBLK_GOVRH	
	govrh_set_tg_enable(VPP_FLAG_ENABLE);
#endif

	if( g_vpp.govrh_preinit ){
//fan , modify for u-boot logo
		unsigned int govr_y,govr_c;
		unsigned int govw_y,govw_c;	

		govr_y = g_vpp.govr->fb_p->fb.y_addr;
		govr_c = g_vpp.govr->fb_p->fb.c_addr;
		govw_y = p_govw->fb_p->fb.y_addr;
		govw_c = p_govw->fb_p->fb.c_addr;

		vpp_wait_vsync();
		p_govw->fb_p->set_addr(govr_y,govr_c);
		vpp_wait_vsync();
		p_govw->fb_p->set_addr(govw_y,govw_c);
//
		vpp_wait_vsync();
		vppm_clean_int_status(VPP_INT_GOVW_PVBI);
		vppm_set_int_enable(VPP_FLAG_ENABLE,VPP_INT_GOVW_PVBI);
		g_vpp.govrh_preinit = 0;
	}

	if( g_vpp.vo_enable ){
		vpp_wait_vsync();
		vpp_wait_vsync();
		vout_set_blank(VOUT_MODE_ALL,0);
		g_vpp.vo_enable = 0;
	}
	g_vpp.ge_direct_init = 1;
	up(&vpp_sem);
	return 0;
} /* End of vpp_config */

/*!*************************************************************************
* vpp_common_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver common ioctl
*		
* \retval  None
*/ 
int vpp_common_ioctl(unsigned int cmd,unsigned long arg)
{
	vpp_mod_base_t *mod_p;
	vpp_fb_base_t *mod_fb_p;
	int retval = 0;

	switch(cmd){
		case VPPIO_VPPGET_INFO:
			{
			vpp_cap_t parm;
			int i;
			unsigned int chipid;

			chipid = vpp_get_chipid();
			switch( chipid & 0xFFFF0000 ){
				case 0x34370000:
					chipid = (0x84350000 | (chipid & 0xFFFF));
					break;
				case 0x34260000:
					chipid = (0x85100000 | (chipid & 0xFFFF));
					break;
				case 0x34290000:
					chipid = (0x84250000 | (chipid & 0xFFFF));
					break;
				default:
					break;
			}
			parm.chip_id = chipid;
			parm.version = 0x01;
			parm.resx_max = VPP_HD_MAX_RESX;
			parm.resy_max = VPP_HD_MAX_RESY;
			parm.pixel_clk = 400000000;
			parm.module = 0x0;
			for(i=0;i<VPP_MOD_MAX;i++){
				mod_p = vpp_mod_get_base(i);
				if( mod_p ){
					parm.module |= (0x01 << i);
				}
			}
			parm.option = 0x0;
			copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_cap_t));
			}
			break;
		case VPPIO_VPPSET_INFO:
			{
			vpp_cap_t parm;

			copy_from_user((void *)&parm,(const void *)arg,sizeof(vpp_cap_t));
			}
			break;
		case VPPIO_I2CSET_BYTE:
			{
			vpp_i2c_t parm;
			unsigned int id;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_i2c_t));
			id = (parm.addr & 0x0000FF00) >> 8;
			vpp_i2c_bus_write(id,(parm.addr & 0xFF),parm.index,(char *)&parm.val,1);
			}
			break;
		case VPPIO_I2CGET_BYTE:
			{
			vpp_i2c_t parm;
			unsigned int id;
			int len;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_i2c_t));
			id = (parm.addr & 0x0000FF00) >> 8;
			len = parm.val;
			{
				unsigned char buf[len];
				
				vpp_i2c_bus_read(id,(parm.addr & 0xFF),parm.index,buf,len);
				parm.val = buf[0];
			}
			copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_i2c_t));
			}
			break;
		case VPPIO_VPPSET_DIRECTPATH:
			if( g_vpp.direct_path == arg ){
				break;
			}
			
			g_vpp.direct_path = arg;
			vpp_disp_fb_clr(0);
			if( g_vpp.direct_path ){
#ifdef CONFIG_VPP_GOVRH_FBSYNC
				vpp_fbsync_cal_fps();
#endif
				vpp_set_govw_tg(VPP_FLAG_DISABLE);
				vpp_direct_path_switch(VPP_FLAG_ENABLE);
				vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,VPP_FLAG_ENABLE);
			}
			else {
				g_vpp.govr->fb_p->fb = g_vpp.direct_path_ori_fb;
				vpp_set_govw_tg(VPP_FLAG_ENABLE);
				vpp_irqproc_work(VPP_INT_GOVW_PVBI,0,0,1);	// wait one frame buffer ready
				vpp_irqproc_work(VPP_INT_GOVRH_PVBI,(void *)vpp_direct_path_switch,0,1); // update color fmt & fb in same time
				vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,VPP_FLAG_DISABLE);
			}
			break;
		case VPPIO_VPPSET_FBDISP:
			{
				vpp_dispfb_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_dispfb_t));

				if( vpp_check_dbg_level(VPP_DBGLVL_IOCTL) ){
					DPRINT("[VPP] set fbdisp, flag 0x%x\n",parm.flag);
				}

				retval = vpp_disp_fb_add(&parm);
				if( retval ){
					vpp_dbg_show(VPP_DBGLVL_DISPFB,1,"add disp fb full");
				}
				else {
					// vpp_set_dbg_gpio(4,0xFF);
				}
			}
			break;
		case VPPIO_VPPGET_FBDISP:
			{
				vpp_dispfb_info_t parm;

				parm.queue_cnt = g_vpp.disp_fb_max;
				parm.cur_cnt = g_vpp.disp_fb_cnt;
				parm.isr_cnt = g_vpp.dbg_dispfb_isr_cnt;
				parm.disp_cnt = vpp_vpu_disp_cnt;
				parm.skip_cnt = vpp_vpu_disp_skip_cnt;
				parm.full_cnt = g_vpp.dbg_dispfb_full_cnt;

				g_vpp.dbg_dispfb_isr_cnt = 0;
				vpp_vpu_disp_cnt = 0;
				vpp_vpu_disp_skip_cnt = 0;
				g_vpp.dbg_dispfb_full_cnt = 0;
				copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_dispfb_info_t));
			}
			break;
		case VPPIO_WAIT_FRAME:
			{
				int i;
				for(i=0;i<arg;i++){
					vpp_wait_vsync();
				}
			}
			break;
		case VPPIO_MODULE_FRAMERATE:
			{
				vpp_mod_arg_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_arg_t));
				mod_fb_p = vpp_mod_get_fb_base(parm.mod);
				if( parm.read ){
					parm.arg1 = mod_fb_p->framerate;
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_arg_t));
				}
				else {
					mod_fb_p->framerate = parm.arg1;
					if( parm.mod == VPP_MOD_GOVW ){
						if( g_vpp.direct_path ){
#ifdef CONFIG_VPP_GOVRH_FBSYNC
							vpp_fbsync_cal_fps();
#endif
						}
						else {
							mod_fb_p->set_framebuf(&mod_fb_p->fb);
						}
					}
				}
			}
			break;
		case VPPIO_MODULE_ENABLE:
			{
				vpp_mod_arg_t parm;
				
				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_arg_t));
				mod_p = vpp_mod_get_base(parm.mod);
				if( parm.read ){
					
				}
				else {
					mod_p->set_enable(parm.arg1);
					if( parm.mod == VPP_MOD_CURSOR ){
						vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI,parm.arg1);
					}
				}
			}
			break;
		case VPPIO_MODULE_TIMING:
			{
				vpp_mod_timing_t parm;
				vpp_clock_t clock;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_timing_t));
				mod_p = vpp_mod_get_base(parm.mod);
				if( parm.read ){
					mod_p->get_tg(&clock);					
					vpp_trans_timing(parm.mod,&parm.tmr,&clock,0);
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_timing_t));
				}
				else {
					vpp_alloc_framebuffer(parm.tmr.hpixel,parm.tmr.vpixel);
					vpp_mod_set_timing(parm.mod,&parm.tmr);
//					vpp_trans_timing(parm.mod,&parm.tmr,&clock,1);					
//					mod_p->set_tg(&clock);
				}
			}
			break;
		case VPPIO_MODULE_FBADDR:
			{
				vpp_mod_arg_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_arg_t));
				mod_fb_p = vpp_mod_get_fb_base(parm.mod);
				if( parm.read ){
					mod_fb_p->get_addr(&parm.arg1,&parm.arg2);
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_arg_t));
				}
				else {
					mod_fb_p->set_addr(parm.arg1,parm.arg2);
				}
			}
			break;
		case VPPIO_MODULE_FBINFO:
			{
				vpp_mod_fbinfo_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_fbinfo_t));
				mod_fb_p = vpp_mod_get_fb_base(parm.mod);
				if( parm.read ){
					parm.fb = mod_fb_p->fb;
					if( parm.mod == VPP_MOD_GOVRH ){ // add for hw cursor
						parm.fb.col_fmt = (parm.fb.col_fmt >= VDO_COL_FMT_ARGB)? VDO_COL_FMT_ARGB:parm.fb.col_fmt;
					}
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_fbinfo_t));
				}
				else {
					mod_fb_p->fb = parm.fb;
					mod_fb_p->set_framebuf(&parm.fb);
				}
			}
			break;
		case VPPIO_MODULE_VIEW:
			{
				vpp_mod_view_t parm;
				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_view_t));
				mod_fb_p = vpp_mod_get_fb_base(parm.mod);
				if( parm.read ){
					mod_fb_p->fn_view(VPP_FLAG_RD,&parm.view);
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_view_t));
				}
				else {
					mod_fb_p->fn_view(0,&parm.view);
				}
			}
			break;
		case VPPIO_VPPGET_PTS:
			copy_to_user( (void *)arg, (void *) &g_vpp.disp_pts, sizeof(vpp_pts_t));
			break;
		case VPPIO_VPPSET_PTS:
			copy_from_user( (void *) &g_vpp.frame_pts, (const void *)arg, sizeof(vpp_pts_t));
			{
				int i;
				for(i=0;i<sizeof(vpp_pts_t);i++){
					if( g_vpp.frame_pts.pts[i] )
						break;
				}
				if( i == sizeof(vpp_pts_t )){
					memset(&g_vpp.govw_pts,0x0,sizeof(vpp_pts_t));
					memset(&g_vpp.disp_pts,0x0,sizeof(vpp_pts_t));
				}
			}
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of vpp_common_ioctl */

/*!*************************************************************************
* vout_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	video out ioctl
*		
* \retval  None
*/ 
int vout_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;

//	DBGMSG("vout_ioctl\n");

	switch(cmd){
		case VPPIO_VOGET_INFO:
			{
			vpp_vout_info_t parm;
			vout_t *vo;
			int num;

//			DBGMSG("VPPIO_VOGET_INFO\n");

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_info_t));
			num = parm.num;
			if( num >= VOUT_MODE_MAX ){
				retval = -ENOTTY;
				break;
			}
			memset(&parm,0,sizeof(vpp_vout_info_t));
			vo = vout_get_info(num);
			if( vo ){
				parm.num = num;
				parm.status = vo->status;
				strncpy(parm.name,vo->name,10);
			}
			copy_to_user( (void *)arg, (const void *) &parm, sizeof(vpp_vout_info_t));
			
			}
			break;
		case VPPIO_VOSET_MODE:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			retval = vout_set_mode(parm.num, parm.arg);
			}
			break;
		case VPPIO_VOSET_BLANK:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			retval = vout_set_blank(parm.num,parm.arg);
			}
			break;
#ifdef WMT_FTBLK_GOVRH			
		case VPPIO_VOSET_DACSENSE:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
#ifdef WMT_FTBLK_GOVRH_VGA
			if( parm.num == VOUT_VGA ){
				/* TODO: dac sense timer */
				if( parm.arg == 0 ){
					govrh_DAC_set_pwrdn(VPP_FLAG_DISABLE);
				}
			}
#endif			
			}
			break;
		case VPPIO_VOSET_BRIGHTNESS:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			govrh_set_brightness(parm.arg);
			}
			break;
		case VPPIO_VOGET_BRIGHTNESS:
			{
			vpp_vout_parm_t parm;
			
			parm.num = 0;
			parm.arg = govrh_get_brightness();
			copy_to_user((void *)arg,(void *)&parm, sizeof(vpp_vout_parm_t));
			}
			break;
		case VPPIO_VOSET_CONTRAST:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			govrh_set_contrast(parm.arg);
			}
			break;
		case VPPIO_VOGET_CONTRAST:
			{
			vpp_vout_parm_t parm;
			
			parm.num = 0;
			parm.arg = govrh_get_contrast();
			copy_to_user((void *)arg,(void *) &parm, sizeof(vpp_vout_parm_t));
			}
			break;
#endif			
		case VPPIO_VOSET_OPTION:
			{
			vpp_vout_option_t option;
			vout_t *vo;
			int num;

			copy_from_user( (void *) &option, (const void *)arg, sizeof(vpp_vout_option_t));
			num = option.num;
			if( num >= VOUT_MODE_MAX ){
				retval = -ENOTTY;
				break;
			}
			vo = vout_get_info(num);
			if( vo ){
				vo->option[0] = option.option[0];
				vo->option[1] = option.option[1];
				vo->option[2] = option.option[2];
				vout_set_mode(num,(vo->status & VPP_VOUT_STS_ACTIVE)? 1:0);
			}
			}
			break;
		case VPPIO_VOGET_OPTION:
			{
			vpp_vout_option_t option;
			vout_t *vo;
			int num;

			copy_from_user( (void *) &option, (const void *)arg, sizeof(vpp_vout_option_t));
			num = option.num;
			if( num >= VOUT_MODE_MAX ){
				retval = -ENOTTY;
				break;
			}
			memset(&option,0,sizeof(vpp_vout_info_t));			
			vo = vout_get_info(num);
			if( vo ){
				option.num = num;
				option.option[0] = vo->option[0];
				option.option[1] = vo->option[1];
				option.option[2] = vo->option[2];
			}
			copy_to_user( (void *)arg, (const void *) &option, sizeof(vpp_vout_option_t));
			}
			break;
		case VPPIO_VOUT_VMODE:
			{
			vpp_vout_vmode_t parm;
			int i;
			vpp_timing_t *vmode;
			unsigned int resx,resy,fps;
			unsigned int pre_resx,pre_resy,pre_fps;
			int index,from_index;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_vmode_t));
			from_index = parm.num;
			parm.num = 0;			
#ifdef CONFIG_WMT_EDID
			{
				vout_t *vo;
				if(!(vo = vout_get_info(parm.mode))){
					goto vout_vmode_end;
				}
				
				if( !(vo->status & VPP_VOUT_STS_PLUGIN) ){
					DPRINT("*W* not plugin\n");
					goto vout_vmode_end;
				}
				
				if( vout_get_edid(parm.mode) == 0 ){
					DPRINT("*W* read EDID fail\n");
					goto vout_vmode_end;
				}
				if( edid_parse(vo->edid) ){
					DPRINT("*W* parse EDID fail\n");
					goto vout_vmode_end;
				}
			}
#endif
			index = 0;
			resx = resy = fps = 0;
			pre_resx = pre_resy = pre_fps = 0;
			for(i=0;;i++){
				vmode = (vpp_timing_t *) &vpp_video_mode_table[i];
				if( vmode->pixel_clock == 0 ) 
					break;
				resx = vmode->hpixel;
				resy = vmode->vpixel;
				fps = vpp_get_video_mode_fps(vmode);
				if( vmode->option & VPP_OPT_INTERLACE ){
					resy *= 2;
					i++;
				}
				if( (pre_resx == resx) && (pre_resy == resy) && (pre_fps == fps) ){
					continue;
				}
				pre_resx = resx;
				pre_resy = resy;
				pre_fps = fps;
				
#ifdef CONFIG_WMT_EDID				
				if( edid_find_support( resx, resy, fps) ){
#else
				if( 1 ){
#endif
					if( index >= from_index ){
						parm.parm[parm.num].resx = resx;
						parm.parm[parm.num].resy = resy;
						parm.parm[parm.num].fps = fps;
						parm.parm[parm.num].option = vmode->option;
						parm.num++;
					}
					index++;
					if( parm.num >= VPP_VOUT_VMODE_NUM )
						break;
				}
			}
#ifdef CONFIG_WMT_EDID
vout_vmode_end:
#endif
			// DPRINT("[VPP] get support vmode %d\n",parm.num);
			copy_to_user( (void *)arg, (const void *) &parm, sizeof(vpp_vout_vmode_t));
			}
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of vout_ioctl */

/*!*************************************************************************
* govr_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	govr module ioctl
*		
* \retval  None
*/ 
int govr_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	
 	switch(cmd){
#ifdef WMT_FTBLK_GOVRH		
		case VPPIO_GOVRSET_DVO:
			{
			vdo_dvo_parm_t parm;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vdo_dvo_parm_t));
			govrh_set_dvo_enable(parm.enable);
			govrh_set_dvo_color_format(parm.color_fmt);
			govrh_set_dvo_clock_delay(parm.clk_inv,parm.clk_delay);
			govrh_set_dvo_outdatw(parm.data_w);
			govrh_set_dvo_sync_polar(parm.sync_polar,parm.vsync_polar);
			p_govrh->fb_p->set_csc(p_govrh->fb_p->csc_mode);
			}
			break;
#endif
#ifdef WMT_FTBLK_GOVRH_CURSOR
		case VPPIO_GOVRSET_CUR_COLKEY:
			{
			vpp_mod_arg_t parm;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vdo_dvo_parm_t));
			govrh_CUR_set_color_key(VPP_FLAG_ENABLE,0,parm.arg1);
			}
			break;
		case VPPIO_GOVRSET_CUR_HOTSPOT:
			{
			vpp_mod_arg_t parm;
			vdo_view_t view;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vdo_dvo_parm_t));
			p_cursor->hotspot_x = parm.arg1;
			p_cursor->hotspot_y = parm.arg2;
			view.posx = p_cursor->posx;
			view.posy = p_cursor->posy;
			p_cursor->fb_p->fn_view(0,&view);
			}
			break;
#endif
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of govr_ioctl */

/*!*************************************************************************
* govw_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	govw module ioctl
*		
* \retval  None
*/ 
int govw_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	
	switch(cmd){
		case VPPIO_GOVW_ENABLE:
			if( arg ){
				vpp_set_govw_tg(VPP_FLAG_ENABLE);
			}
			else {
				vpp_irqproc_work(VPP_INT_GOVW_PVBI,(void *)vpp_irqproc_enable_govw,0,1);
			}
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of govw_ioctl */

/*!*************************************************************************
* govm_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	govm module ioctl
*		
* \retval  None
*/ 
int govm_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;

	switch(cmd){
		case VPPIO_GOVMSET_SRCPATH:
			{
			vpp_src_path_t parm;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_src_path_t));
			vpp_set_govm_path(parm.src_path,parm.enable);
            if( (parm.src_path & VPP_PATH_GOVM_IN_VPU) && (parm.enable) )
	            vpp_pan_display(0,0,0);
			}
			break;
		case VPPIO_GOVMGET_SRCPATH:
			retval = vpp_get_govm_path();
			break;
		case VPPIO_GOVMSET_ALPHA:
			{
			vpp_alpha_parm_t parm;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_alpha_parm_t));				
			govm_set_alpha_mode(parm.enable,parm.mode,parm.A,parm.B);
			}
			break;
		case VPPIO_GOVMSET_GAMMA:
			govm_set_gamma_mode(arg);
			break;
		case VPPIO_GOVMSET_CLAMPING:
			govm_set_clamping_enable(arg);
			break;		
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of govm_ioctl */

/*!*************************************************************************
* vpu_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpu module ioctl
*		
* \retval  None
*/ 
int vpu_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	
	switch(cmd){
		case VPPIO_VPUSET_VIEW:
			{
			vdo_view_t view;

			copy_from_user( (void *) &view, (const void *)arg, sizeof(vdo_view_t));
			p_vpu->fb_p->fn_view(0,&view);
			if( vpp_check_dbg_level(VPP_DBGLVL_SCALE) ){
				DPRINT("[VPP] set view\n");
			}
			}
			break;
		case VPPIO_VPUGET_VIEW:
			{
			vdo_view_t view;

			p_vpu->fb_p->fn_view(1,&view);
			copy_to_user( (void *)arg, (void *) &view, sizeof(vdo_view_t));			
			}
			break;
		case VPPIO_VPUSET_FBDISP:
			{
			vpp_dispfb_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_dispfb_t));
			retval = vpp_disp_fb_add(&parm);
			}
			break;
		case VPPIO_VPU_CLR_FBDISP:
			retval = vpp_disp_fb_clr(0);
			break;	
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of vpu_ioctl */

/*!*************************************************************************
* scl_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	scl module ioctl
*		
* \retval  None
*/ 
int scl_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;

	switch(cmd){
		case VPPIO_SCL_SCALE_ASYNC:
		case VPPIO_SCL_SCALE:
			{
			vpp_scale_t parm;

			p_scl->scale_sync = (cmd==VPPIO_SCL_SCALE)? 1:0;
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_scale_t));
			retval = vpp_set_recursive_scale(&parm.src_fb,&parm.dst_fb);
			copy_to_user( (void *) arg, (void *) &parm, sizeof(vpp_scale_t));
			}
			break;
#ifdef WMT_FTBLK_SCL					
		case VPPIO_SCL_DROP_LINE_ENABLE:
			scl_set_drop_line(arg);
			break;
#endif			
		case VPPIO_SCL_SCALE_FINISH:
			retval = p_scl->scale_finish();
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of scl_ioctl */

/*!*************************************************************************
* vpp_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver ioctl
*		
* \retval  None
*/ 
int vpp_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	int err = 0;

//	DBGMSG("vpp_ioctl\n");

	switch( _IOC_TYPE(cmd) ){
		case VPPIO_MAGIC:
			break;
		default:
			return -ENOTTY;
	}
	
	/* check argument area */
	if( _IOC_DIR(cmd) & _IOC_READ )
		err = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	else if ( _IOC_DIR(cmd) & _IOC_WRITE )
		err = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	
	if( err ) return -EFAULT;

	if( vpp_check_dbg_level(VPP_DBGLVL_IOCTL) ){
		DPRINT("[VPP] ioctl cmd 0x%x,arg 0x%x\n",_IOC_NR(cmd),(int)arg);
	}

	down(&vpp_sem);
	switch(_IOC_NR(cmd)){
		case VPPIO_VPP_BASE ... (VPPIO_VOUT_BASE-1):
//			DBGMSG("VPP command ioctl\n");
			retval = vpp_common_ioctl(cmd,arg);
			break;
		case VPPIO_VOUT_BASE ... (VPPIO_GOVR_BASE-1):
//			DBGMSG("VOUT ioctl\n");
			retval = vout_ioctl(cmd,arg);
			break;
		case VPPIO_GOVR_BASE ... (VPPIO_GOVW_BASE-1):
//			DBGMSG("GOVR ioctl\n");
			retval = govr_ioctl(cmd,arg);
			break;
		case VPPIO_GOVW_BASE ... (VPPIO_GOVM_BASE-1):
//			DBGMSG("GOVW ioctl\n");
			retval = govw_ioctl(cmd,arg);
			break;
		case VPPIO_GOVM_BASE ... (VPPIO_VPU_BASE-1):
//			DBGMSG("GOVM ioctl\n");
			retval = govm_ioctl(cmd,arg);
			break;
		case VPPIO_VPU_BASE ... (VPPIO_SCL_BASE-1):
//			DBGMSG("VPU ioctl\n");
			retval = vpu_ioctl(cmd,arg);
			break;
		case VPPIO_SCL_BASE ... (VPPIO_MAX-1):
//			DBGMSG("SCL ioctl\n");
			retval = scl_ioctl(cmd,arg);
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	up(&vpp_sem);

	if( vpp_check_dbg_level(VPP_DBGLVL_IOCTL) ){
		DPRINT("[VPP] ioctl cmd 0x%x,ret 0x%x\n",_IOC_NR(cmd),(int)retval);
	}

	return retval;
} /* End of vpp_ioctl */


int vpp_pan_display(struct fb_var_screeninfo *var, struct fb_info *info,int enable)
{
#ifdef CONFIG_VPP_GE_DIRECT_PATH
	unsigned int addr;

	if( g_vpp.ge_direct_init == 0 )
		return 0;

	if( g_vpp.ge_direct_path != enable ){
		if( vpp_govm_path ){
			// DPRINT("[VPP] skip chg GE direct path %d\n",enable);
			return 0;
		}
		g_vpp.ge_direct_path = enable;
		if( enable ){
			vdo_framebuf_t fb;

			addr = var->yoffset * var->xres_virtual + var->xoffset;
			addr *= var->bits_per_pixel >> 3;
			addr += info->fix.smem_start;
			
			fb.y_addr = addr;
			fb.img_w = var->xres;
			fb.img_h = var->yres;
			fb.fb_w = var->xres_virtual;
			fb.fb_h = var->yres_virtual;
			fb.h_crop = 0;
			fb.v_crop = 0;
			fb.flag = 0;
			fb.c_addr = 0;

			switch (var->bits_per_pixel) {
				case 16:
					if ((info->var.red.length == 5) &&
						(info->var.green.length == 6) &&
						(info->var.blue.length == 5)) {
						fb.col_fmt = VDO_COL_FMT_RGB_565;
					} else if ((info->var.red.length == 5) &&
						(info->var.green.length == 5) &&
						(info->var.blue.length == 5)) {
						fb.col_fmt = VDO_COL_FMT_RGB_1555;
					} else {
						fb.col_fmt = VDO_COL_FMT_RGB_5551;
					}
					break;
				case 32:
					fb.col_fmt = VDO_COL_FMT_ARGB;
					break;
				default:
					fb.col_fmt = VDO_COL_FMT_RGB_565;
					break;
			}

			// govw disable
			vpp_irqproc_work(VPP_INT_GOVW_VBIS,(void *)vpp_irqproc_enable_govw,0,1);

			// backup current govrh
			g_vpp.direct_path_ori_fb = p_govrh->fb_p->fb;

			// pvbi should more than ###
			if( vppif_reg32_read(GOVRH_PVBI_LINE) < 5 ){
				vppif_reg32_write(GOVRH_PVBI_LINE,5);
			}
			vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,VPP_FLAG_ENABLE);

			// set ge fb to govrh
			p_govrh->fb_p->fb = fb;
			g_vpp.direct_path_chg = 1;
			vpp_irqproc_work(VPP_INT_GOVRH_PVBI,0,0,1);
		}
		else {
			g_vpp.govw_skip_all = 1;
			p_govrh->fb_p->fb = g_vpp.direct_path_ori_fb;
			// govw enable
			vpp_set_govw_tg(VPP_FLAG_ENABLE);
			// update govrh fb
			govw_set_hd_fb_addr(p_govrh->fb_p->fb.y_addr, p_govrh->fb_p->fb.c_addr);
			vpp_wait_vsync();
			vpp_wait_vsync();
			vpp_wait_vsync();
			// update govw fb
			govw_set_hd_fb_addr(p_govw->fb_p->fb.y_addr, p_govw->fb_p->fb.c_addr);
			vpp_wait_vsync();
			vpp_wait_vsync();
			vpp_wait_vsync();
			
			// change govrh path
			g_vpp.direct_path_chg = 1;
			vpp_irqproc_work(VPP_INT_GOVRH_PVBI,0,0,1);
			vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,VPP_FLAG_DISABLE);
			g_vpp.govw_skip_all = 0;
		}
		// DPRINT("[VPP] GE direct path %d\n",enable);
	}
	else {
		extern unsigned int fb_egl_swap;
		
		if( enable ){
			if (fb_egl_swap != 0)
				addr = fb_egl_swap;
			else {
				addr = var->yoffset * var->xres_virtual + var->xoffset;
				addr *= var->bits_per_pixel >> 3;
				addr += info->fix.smem_start;
			}
			govrh_set_fb_addr(addr, 0);
			vpp_wait_vsync();
		}
		// DPRINT("[VPP] GE 0x%x egl 0x%x\n",addr,fb_egl_swap);
	}
#endif
	return 0;
}

