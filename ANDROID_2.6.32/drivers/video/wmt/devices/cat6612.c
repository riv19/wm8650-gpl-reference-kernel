/*++ 
 * linux/drivers/video/wmt/cat6612.c
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

#define CAT6612_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../vout.h"
#include <linux/delay.h>
#include "cat6610/hdmitx.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  CAT6612_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define CAT6612_XXXX    1     *//*Example*/
#define CAT6612_ADDR 	0x98

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx ad9389_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in cat6612.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  cat6612_xxx;        *//*Example*/
HDMI_Video_Type cat6612_type = HDMI_480p60;
HDMI_OutputColorMode cat6612_colfmt = HDMI_RGB444;
extern BYTE bInputSignalType;
extern BOOL bHDMIMode, bAudioEnable;
int cat6612_spdif_enable;
int cat6612_cp_enable;

extern unsigned long bInputAudioSampleFreq;
extern int bInputAudioChannel;
extern int bInputAudioFormat;

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void cat6612_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
BYTE HDMITX_ReadI2C_Byte(BYTE RegAddr)
{
	unsigned char p_data;

	vpp_i2c_read(CAT6612_ADDR, RegAddr, &p_data, 1);
	return p_data;
}

SYS_STATUS HDMITX_WriteI2C_Byte(BYTE RegAddr, BYTE d)
{
	int flag;

	flag = vpp_i2c_write(CAT6612_ADDR, RegAddr, &d, 1);

#if 0
{
	BYTE rd;

	vpp_i2c_read(CAT6612_ADDR, RegAddr, &rd, 1);
	if( rd != d ){
		printk("[CAT6612] *E* 0x%x : wr 0x%x,rd 0x%x\n",RegAddr,d,rd);
	}
	else {
		printk("[CAT6612] 0x%x : wr 0x%x,rd 0x%x\n",RegAddr,d,rd);
	}
}
#endif
	return !flag;
}

SYS_STATUS HDMITX_ReadI2C_ByteN(BYTE RegAddr, BYTE *pData, int N)
{
	int flag;

	flag = vpp_i2c_read(CAT6612_ADDR,RegAddr, pData, N);
	return !flag;
}

SYS_STATUS HDMITX_WriteI2C_ByteN(SHORT RegAddr, BYTE *pData, int N)
{
	int flag;

	flag = vpp_i2c_write(CAT6612_ADDR,RegAddr,pData,N);
	return !flag;
}

void DelayMS(USHORT ms)
{
	mdelay(ms);
}

int cat6612_check_plugin(int hotplug)
{
	unsigned char buf[1];
	int plugin;

	vpp_i2c_read(CAT6612_ADDR,0x0E,buf,1);
	plugin = (buf[0] & BIT6)? 1:0;
	printk("[CAT6612] HDMI plug%s\n",(plugin)?"in":"out");

	if( !hotplug ) 
		return plugin;

#if 0
	buf[0] = 0x01;
	vpp_i2c_write(CAT6612_ADDR, 0x0C, buf, 1);	// clear int sts

	buf[0] = 0x01;
	vpp_i2c_write(CAT6612_ADDR, 0x0E, buf, 1);	// clear int active
#endif
	cat6612_cp_enable = g_vpp.hdmi_cp_enable;
	cat6612_spdif_enable = vpp_get_hdmi_spdif();
	if( plugin ){
		unsigned char *buf;
		int option;
		
		buf = vout_get_edid(VOUT_DVO2HDMI);
		option = edid_parse_option(buf);
		bHDMIMode = (option & EDID_OPT_HDMI)? 1:0;
		bAudioEnable = (option & EDID_OPT_AUDIO)? 1:0;
		if( bHDMIMode == 0 ){
			buf[0] = 0x0;	// B_CPDESIRE 
			vpp_i2c_write(CAT6612_ADDR, 0x20, buf, 1);
		}
		HDMITX_SetOutput();
	}
	else {
        DisableAudioOutput() ;
        DisableVideoOutput() ;
	}

	// check int status again
	vpp_i2c_read(CAT6612_ADDR,0x06,buf,1);
	if( buf[0] == 0x1 ){
		buf[0] = 0x01;
		vpp_i2c_write(CAT6612_ADDR, 0x0C, buf, 1);	// clear int sts

		buf[0] = 0x01;
		vpp_i2c_write(CAT6612_ADDR, 0x0E, buf, 1);	// clear int active
	}
	vppif_reg32_write((GPIO_BASE_ADDR+0x300), 0x3<<(VPP_VOINT_NO*2),VPP_VOINT_NO*2,2);	// GPIO0 3:rising edge, 2:falling edge
	return plugin;
}

void cat6612_set_power_down(int enable)
{

}

int cat6612_set_mode(unsigned int *option)
{
	DBGMSG("option %d,%d\n",option[0],option[1]);

	bInputSignalType &= ~T_MODE_SYNCEMB;
	switch(option[0]){
		case VDO_COL_FMT_ARGB:
			cat6612_colfmt = HDMI_RGB444;
			break;
		case VDO_COL_FMT_YUV444:
			cat6612_colfmt = HDMI_YUV444;
			break;
		case VDO_COL_FMT_YUV422H:
			cat6612_colfmt = HDMI_YUV422;
			break;
		case VDO_COL_FMT_YUV422V:
			cat6612_colfmt = HDMI_YUV422;
			bInputSignalType |= T_MODE_SYNCEMB;
			break;
		default:
			return -1;
	}

	if( option[1] & BIT0 ){	// 24 bit
		bInputSignalType &= ~T_MODE_INDDR;
	}
	else {	// 12 bit
		bInputSignalType |= T_MODE_INDDR;
	}

	cat6612_spdif_enable = vpp_get_hdmi_spdif();
	cat6612_cp_enable = g_vpp.hdmi_cp_enable;
	return 0;
}	

int cat6612_config(vout_info_t *info)
{
	HDMI_Video_Type type;
	unsigned int pixclock;

	pixclock = info->timing.pixel_clock;
	type = HDMI_Unkown;
	bInputSignalType &= ~T_MODE_CCIR656;	
	switch( info->resx ){
		case 640:
    		type = HDMI_640x480p60;
			break;
		case 720:
			if( info->resy == 480 ){
				if( pixclock == 13514000 ){
					type = HDMI_480i60;
					// bInputSignalType |= T_MODE_CCIR656;
					// type = HDMI_480i60_16x9;
				}
				else {
					type = HDMI_480p60;
					// type = HDMI_480p60_16x9;
				}
			}
			if( info->resy == 576 ){
				if( pixclock == 27000000 ){
				    type = HDMI_576p50;
				    // type = HDMI_576p50_16x9;
				}
				else {
				    type = HDMI_576i50;
				    // type = HDMI_576i50_16x9;
				}
			}
			break;
		case 1280:
			if( pixclock == 74250050 ){
				type = HDMI_720p50;
			}
			else {
				type = HDMI_720p60;
			}
			break;
		case 1440:
			if( info->resy == 480 ){
				type = HDMI_1440x480i60;
			}
			if( info->resy == 576 ){
				type = HDMI_1440x576i50;
			}
			break;
    	case 1920:
			switch( pixclock ){
				case 148500000:
					type = HDMI_1080p60;
					break;
				case 74250060:
				    type = HDMI_1080i60;
					break;
			    // HDMI_1080p60 = 16,
			    case 74250050:
			    	type = HDMI_1080i50;
					break;
			    // HDMI_1080p50 = 31,
			    // HDMI_1080p24,
			    case 74250025:
			    	type = HDMI_1080p25;
					break;
				case 74250030:
			    	type = HDMI_1080p30;
					break;
			}
			break;
		default:
			break;
	}

	DBGMSG("type %d,colfmt %d,signal type 0x%x,pixclk %d\n",type,cat6612_colfmt,bInputSignalType,pixclock);

	cat6612_type = type;
	HDMITX_ChangeDisplayOption(cat6612_type,cat6612_colfmt);
	HDMITX_SetOutput();
	return 0;
}

int cat6612_get_edid(char *buf)
{
	extern int GetEDIDData(int EDIDBlockID, unsigned char *pEDIDData);
	int i,cnt;

	GetEDIDData(0,buf);
	cnt = buf[0x7E];
	if( cnt >= 3 ) cnt = 3;
	for(i=1;i<=cnt;i++){
		GetEDIDData(i,&buf[128*i]);
	}

	// patch for read EDID will reset issue
	HDMITX_SetOutput();
	return 0;
}

int cat6612_set_audio(vout_audio_t *arg)
{
	extern void HDMITX_SetAudio(void);

	bInputAudioFormat = arg->fmt;
	bInputAudioSampleFreq = arg->sample_rate;
	bInputAudioChannel = arg->channel;
	cat6612_spdif_enable = vpp_get_hdmi_spdif();
	HDMITX_SetAudio();
	return 0;
}

int cat6612_init(void)
{
	unsigned char buf[2];

	buf[0] = 0xff;
	vpp_i2c_read(CAT6612_ADDR, 0x0, buf, 2);
	if( (buf[0] != 0x00) || (buf[1] != 0xCA) ){
		return -1;
	}
	HDMITX_ChangeDisplayOption(cat6612_type, cat6612_colfmt) ; // set initial video mode and initial output color mode
	InitCAT6611() ;
	DPRINT("[CAT6612] HDMI ext device\n");
	return 0;
}

/*----------------------- vout device plugin --------------------------------------*/
vout_dev_ops_t cat6612_vout_dev_ops = {
	.mode = VOUT_DVO2HDMI,

	.init = cat6612_init,
	.set_power_down = cat6612_set_power_down,
	.set_mode = cat6612_set_mode,
	.config = cat6612_config,
	.check_plugin = cat6612_check_plugin,
	.get_edid = cat6612_get_edid,
	.set_audio = cat6612_set_audio,
};

static int cat6612_module_init(void)
{	
	vout_device_register(&cat6612_vout_dev_ops);
	return 0;
} /* End of cat6612_module_init */
module_init(cat6612_module_init);

/*--------------------End of Function Body -----------------------------------*/

#undef CAT6612_C
