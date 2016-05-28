/*++ 
 * linux/drivers/video/wmt/govm.c
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

#ifndef HDMI_H
/* To assert that only one occurrence is included */
#define HDMI_H
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include "vpp.h"

/*	following is the C++ header	*/
#ifdef	__cplusplus
extern	"C" {
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  HDMI_XXXX  1    *//*Example*/
// #define CONFIG_HDMI_INTERRUPT

typedef enum {
	HDMI_PACKET_NULL = 0x0,
	HDMI_PACKET_AUD_CLOCK_REGEN = 0x1,
	HDMI_PACKET_AUD_SAMPLE = 0x2,
	HDMI_PACKET_GENERAL_CTRL = 0x3,
	HDMI_PACKET_ACP = 0x4,
	HDMI_PACKET_ISRC1 = 0x5,
	HDMI_PACKET_ISRC2 = 0x6,
	HDMI_PACKET_AUD_ONE_BIT_SAMPLE = 0x7,
	HDMI_PACKET_AUD_DST = 0x8,
	HDMI_PACKET_AUD_HBR = 0x9,
	HDMI_PACKET_GAMUT_METADATA = 0xA,
	HDMI_PACKET_INFOFRAME_VENDOR = 0x81,
	HDMI_PACKET_INFOFRAME_AVI = 0x82,
	HDMI_PACKET_INFOFRAME_SRC_PRODUCT_DESC = 0x83,
	HDMI_PACKET_INFOFRAME_AUDIO = 0x84,
	HDMI_PACKET_INFOFRAME_MPEG_SOURCE = 0x85
} hdmi_packet_type_t;

// color depth (CD field)
#define HDMI_COLOR_DEPTH_24		0x4
#define HDMI_COLOR_DEPTH_30		0x5
#define HDMI_COLOR_DEPTH_36		0x6
#define HDMI_COLOR_DEPTH_48		0x7

// pixel packing phase (PP field)
#define HDMI_PHASE_4			0x0
#define HDMI_PHASE_1			0x1
#define HDMI_PHASE_2			0x2
#define HDMI_PHASE_3			0x3

// Scan Information (AVI InfoFrame S0/S1)
#define HDMI_SI_NO_DATA			0x0
#define HDMI_SI_OVERSCAN		0x1
#define HDMI_SI_UNDERSCAN		0x2

// Bar Info (AVI InfoFrame B0/B1)
#define HDMI_BI_DATA_NOT_VALID	0x0
#define HDMI_BI_VERT_VALID		0x1
#define HDMI_BI_HORIZ_VALID		0x2
#define HDMI_BI_V_H_VALID		0x3

// Active Format Information Present (AVI InfoFrame A0)
#define HDMI_AF_INFO_NO_DATA	0x0
#define HDMI_AF_INFO_PRESENT	0x1

// RGB or YCbCr (AVI InfoFrame Y0/Y1)
#define HDMI_OUTPUT_RGB			0x0
#define HDMI_OUTPUT_YUV422		0x1
#define HDMI_OUTPUT_YUV444		0x2

// Aspect Ratio (AVI InfoFrame R0/R1/R2/R3)
#define HDMI_ASPECT_RATIO_PIC	0x8
#define HDMI_ASPECT_RATIO_4_3	0x9
#define HDMI_ASPECT_RATIO_16_9	0xA
#define HDMI_ASPECT_RATIO_14_9	0xB

// Picture Aspect Ratio (AVI InfoFrame M0/M1)
#define HDMI_PIC_ASPECT_NO_DATA	0x0
#define HDMI_PIC_ASPECT_4_3		0x1
#define HDMI_PIC_ASPECT_16_9	0x2

// Colorimetry (AVI InfoFrame C0/C1)
#define HDMI_COLORIMETRY_NO		0x0
#define HDMI_COLORIMETRY_ITU601	0x1
#define HDMI_COLORIMETRY_ITU709	0x2

// Non-uniform Picture Scaling (AVI InfoFrame SC0/SC1)
#define HDMI_NUSCALE_NO			0x0
#define HDMI_NUSCALE_HOR		0x1
#define HDMI_NUSCALE_VERT		0x2
#define HDMI_NUSCALE_HOR_VERT	0x3

// Pixel Repetition (AVI InfoFrame PR0/PR1/PR2/PR3)
#define HDMI_PIXEL_REP_NO		0x0
#define HDMI_PIXEL_REP_2		0x1
#define HDMI_PIXEL_REP_3		0x2
#define HDMI_PIXEL_REP_4		0x3
#define HDMI_PIXEL_REP_5		0x4
#define HDMI_PIXEL_REP_6		0x5
#define HDMI_PIXEL_REP_7		0x6
#define HDMI_PIXEL_REP_8		0x7
#define HDMI_PIXEL_REP_9		0x8
#define HDMI_PIXEL_REP_10		0x9

// Video Code
typedef enum {
	HDMI_UNKNOW = 0,
	HDMI_640x480p60_4x3,
	HDMI_720x480p60_4x3,
	HDMI_720x480p60_16x9,
	HDMI_1280x720p60_16x9,
	HDMI_1920x1080i60_16x9,
	HDMI_1440x480i60_4x3,
	HDMI_1440x480i60_16x9,
	HDMI_1440x240p60_4x3,
	HDMI_1440x240p60_16x9,
	HDMI_2880x480i60_4x3,	// 10
	HDMI_2880x480i60_16x9,
	HDMI_2880x240p60_4x3,
	HDMI_2880x240p60_16x9,
	HDMI_1440x480p60_4x3,
	HDMI_1440x480p60_16x9,
	HDMI_1920x1080p60_16x9,
	HDMI_720x576p50_4x3,
	HDMI_720x576p50_16x9,
	HDMI_1280x720p50_16x9,
	HDMI_1920x1080i50_16x9,	// 20
	HDMI_1440x576i50_4x3,
	HDMI_1440x576i50_16x9,
	HDMI_1440x288p50_4x3,
	HDMI_1440x288p50_16x9,
	HDMI_2880x576i50_4x3,
	HDMI_2880x576i50_16x9,
	HDMI_2880x288p50_4x3,
	HDMI_2880x288p50_16x9,
	HDMI_1440x576p50_4x3,
	HDMI_1440x576p50_16x9,	// 30
	HDMI_1920x1080p50_16x9,
	HDMI_1920x1080p24_16x9,
	HDMI_1920x1080p25_16x9,
	HDMI_1920x1080p30_16x9,
	HDMI_VIDEO_CODE_MAX
} hdmi_video_code_t;

// Audio Channel Count (Audio InfoFrame CC0/CC1/CC2)
typedef enum {
	HDMI_AUD_CHAN_REF_STM = 0,
	HDMI_AUD_CHAN_2CH,
	HDMI_AUD_CHAN_3CH,
	HDMI_AUD_CHAN_4CH,
	HDMI_AUD_CHAN_5CH,
	HDMI_AUD_CHAN_6CH,
	HDMI_AUD_CHAN_7CH,
	HDMI_AUD_CHAN_8CH
} hdmi_audio_channel_count_t;

// Audio Coding type (Audio InfoFrame CT0/CT1/CT2/CT3)
#define HDMI_AUD_TYPE_REF_STM 		0x0
#define HDMI_AUD_TYPE_PCM 			0x1
#define HDMI_AUD_TYPE_AC3			0x2
#define HDMI_AUD_TYPE_MPEG1			0x3
#define HDMI_AUD_TYPE_MP3			0x4
#define HDMI_AUD_TYPE_MPEG2			0x5
#define HDMI_AUD_TYPE_AAC			0x6
#define HDMI_AUD_TYPE_DTS			0x7
#define HDMI_AUD_TYPE_ATRAC			0x8

// Audio Sample size (Audio InfoFrame SS0/SS1)
#define HDMI_AUD_SAMPLE_REF_STM		0x0
#define HDMI_AUD_SAMPLE_16			0x1
#define HDMI_AUD_SAMPLE_20			0x2
#define HDMI_AUD_SAMPLE_24			0x3

// Audio Sample frequency (Audio InfoFrame SF0/SF1/SF2)
#define HDMI_AUD_FREQ_REF_STM		0x0
#define HDMI_AUD_FREQ_32K			0x1
#define HDMI_AUD_FREQ_44_1K			0x2
#define HDMI_AUD_FREQ_48K			0x3
#define HDMI_AUD_FREQ_88_2K			0x4
#define HDMI_AUD_FREQ_96K			0x5
#define HDMI_AUD_FREQ_176_4K		0x6
#define HDMI_AUD_FREQ_192K			0x7

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  hdmi_xxx_t;  *//*Example*/
typedef struct {
	// video
	vdo_color_fmt outfmt;
	int vic;

	// audio
	int channel;
	int freq;
	
} hdmi_info_t;


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef HDMI_C
#define EXTERN
#else
#define EXTERN   extern
#endif /* ifdef HDMI_C */

/* EXTERN int      hdmi_xxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
/* #define HDMI_XXX_YYY   xxxx *//*Example*/
/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  hdmi_xxx(void); *//*Example*/

#ifdef	__cplusplus
}
#endif	
#endif /* ifndef HDMI_H */

