/*++ 
 * linux/drivers/media/video/wmt/h264/h264vdfbm.h
 * WonderMedia h264 decoder device driver
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
#ifndef VT8430_VDFBM_H
/* To assert that only one occurrence is included */
#define VT8430_VDFBM_H

/*--- vt8430-vdfbm.h---------------------------------------------------------------
*   Copyright (C) 2009 WonderMedia Tech. Inc.
*
* MODULE       : vt8430-vdfbm.h -- 
* AUTHOR       : Max Chen
* DATE         : 2009/06/22
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Max Chen, 2006/11/28
*	First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include <linux/config.h>
#include <linux/types.h>
#include <linux/module.h>


/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/


/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  vdfbm_xxx_t;  *//*Example*/


typedef struct {
	char *buf_y;
	char *buf_c;
} wmt_frame_phy_addr_t;

typedef struct {
	wmt_frame_phy_addr_t  buffer;
	char *buf_y_user;
	char *buf_c_user;

	char mem_status; // memory status , flag for frame reference
	char ext_level;     // external level , flag for video display
	char reserv[2];	// reserved
} wmt_frame_info_t;


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef VT8430_VDFBM_C /* allocate memory for variables only in vt8430-vdfbm.c */
	#define EXTERN
#else
	#define EXTERN   extern
#endif /* ifdef VT8430_VDFBM_C */

/* EXTERN int      vdfbm_xxxx; *//*Example*/



#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/


/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  vdfbm_xxxx(vdp_Void); *//*Example*/

 wmt_frame_phy_addr_t * vdfbm_get_fb(void);
 void vdfbm_release_fb(wmt_frame_phy_addr_t *p);
 void vdfbm_flush(void);
 int vdfbm_add_display_fb(wmt_frame_phy_addr_t * pframe);
int vdfbm_get_display_fb(wmt_frame_phy_addr_t *ret_frame);
int vdfbm_fb_init(unsigned int max_fb_count);
int vdfbm_fb_exit(void);

#endif /* ifndef VT8430_VDFBM_H */

/*=== END vt8430-vdfbm.h ==========================================================*/
