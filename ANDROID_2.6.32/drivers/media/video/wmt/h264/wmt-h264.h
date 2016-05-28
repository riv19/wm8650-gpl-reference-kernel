/*++ 
 * linux/drivers/media/video/wmt/h264/wm8605-h264.h
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
#ifndef WM8605_H264_H
/* To assert that only one occurrence is included */
#define WM8605_H264_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include "com-h264.h"

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  VIAAPI_XXXX  1    *//*Example*/


/*------------------------------------------------------------------------------
    Definitions of VDU Registers
    About following definitions, please refer "WM3437 VDU_SD Register List"
------------------------------------------------------------------------------*/



/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/

/*------------------------------------------------------------------------------
    Definitions of enum
------------------------------------------------------------------------------*/
/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WM8605_H264_C /* allocate memory for variables only in wm8605-h264.c */
    #define EXTERN
#else
    #define EXTERN   extern
#endif /* ifdef WM8605_H264_C */

/* EXTERN int      viaapi_xxxx; *//*Example*/

#undef EXTERN

#define SKIP_CUR_FRAME	1		//Arm will send this flag to dsp when player hurry up
														//when dsp receive this flag, should confirm whether current frame can be skipped
														//if yes, release the framebuffer and return the normal decode done message to arm
														//if no, decode the frame normally and wait the next skip flag.
typedef struct 
{
  unsigned int buffer_width;
  unsigned int buffer_height;
  unsigned int current_frame_Y_buffer_address;
  unsigned int current_frame_C_buffer_address;  
  unsigned int flag;
}H264_Private_Packet_t;




typedef struct 
{
  unsigned int bs_type;
  unsigned int decode_status;
  unsigned int frame_bitcnt;
  unsigned int decoded_picture_width;
  unsigned int decoded_picture_height;
  unsigned int add_display_framenum;
}H264_Decode_Done_Private_Packet_t;

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/

/*=== END wm8605-h264.h ==========================================================*/

#endif /* ifndef WM8605_H264_H */
