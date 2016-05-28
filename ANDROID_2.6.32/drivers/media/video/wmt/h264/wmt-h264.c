/*++ 
 * linux/drivers/media/video/wmt/h264/wm8605-h264.c
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
#ifndef WM8605_H264_C
/* To assert that only one occurrence is included */
#define WM8605_H264_C

/*--------------------------------------------------
     Include Header
--------------------------------------------------*/

#ifdef __KERNEL__
    #include <linux/kernel.h>
    #include <mach/hardware.h>
#endif

#include "hw-h264.h"
#include "../wmt-dsplib.h"
#include "../wmt-vdlib.h"
#include "wmt-h264.h"
#include "h264vdfbm.h"

/*--------------------------------------------------
     Dump Message & Function Define
--------------------------------------------------*/




/*--------------------------------------------------
     Function Global
--------------------------------------------------*/
#define H264_DSP_DECDONE_MEM_OFFSET    128
#define H264_MAX_IMG_HEIGHT                    600
#define H264_MIN_FRAME_COUNT			24


h264_frameinfo_t    h264info;
h264_frameinfo_t   *h264_info_out = (&h264info);

avc_input_t       *h264_input_in;

unsigned int h264_prd_phy_addr = 0;
unsigned int h264_prd_virt_addr = 0;

static int frame_num_cnt = 0; // decode...
static AVC_STATUS VDU_status = 0; // check VDU decoding processing step

int linewidth=0,lineheight=0;
extern wmt_frame_info_t g_fb[] ;

unsigned int total_frame_count;


/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/



/*--------------------------------------------------
     Function Prototype
--------------------------------------------------*/
//-----------------------------------------------------------------------
static void h264_hw_get_prd_addr(unsigned int phyaddr, unsigned int virtaddr);
//-----------------------------------------------------------------------
// H/W initialization
void h264_var_reset(void);
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
    // Handel decode procedure
//-----------------------------------------------------------------------
//=> Result detect

//-----------------------------------------------------------------------
#ifdef H264_TEST_FAKE_BITSTREAM
#define TEST_METHOD  1

typedef  struct
{
  unsigned int  decoded_picture_width;
  unsigned int  decoded_picture_height;
  unsigned int  output_Y;
  unsigned int  output_U;
  unsigned int  output_V;
  unsigned int  highlight_MBA;
  unsigned int  highlight_output_Y;
  unsigned int  highlight_output_U;
  unsigned int  highlight_output_V;
  unsigned int  add_display_framenum;
}H264_Fake_Bitstream_t;

int   test_fake_pattern_enter_cnt;
int   index_prev_frame=0;

extern int vdfbm_get_ref_fb(wmt_frame_phy_addr_t * ret_frame);






int test_fake_pattern(char *srcptr, wmt_frame_phy_addr_t   *dec_frame_src_s)
{
	H264_Fake_Bitstream_t  *fake_bistream_s;
	unsigned int *p_relase_frame_num ;
	wmt_frame_phy_addr_t   *dec_frame_dest_s;
	int i;
	DBG(" test_fake_pattern enter \n");

       if ( rdsize < 128)
       	return -1;
	
	test_fake_pattern_enter_cnt++;

	fake_bistream_s = ( H264_Fake_Bitstream_t  *) srcptr;

	fake_bistream_s->decoded_picture_width=linewidth;//ALIGN64(720);//320 linewidth
	fake_bistream_s->decoded_picture_height=lineheight;//240
	fake_bistream_s->output_Y = 29;
	fake_bistream_s->output_U = 255;
	fake_bistream_s->output_V = 107;
	fake_bistream_s->highlight_MBA=test_fake_pattern_enter_cnt-1;

	fake_bistream_s->highlight_output_Y = 76;
	fake_bistream_s->highlight_output_U = 84;
	fake_bistream_s->highlight_output_V = 255;
	
	fake_bistream_s->add_display_framenum=1;
       dec_frame_dest_s =(wmt_frame_phy_addr_t   *)(srcptr+4 * 10);
       dec_frame_dest_s->buf_y=dec_frame_src_s->buf_y;
       dec_frame_dest_s->buf_c=dec_frame_src_s->buf_c;

	//dec_frame_prev_s[index_prev_frame]=dec_frame_src_s


	p_relase_frame_num =(int *)( srcptr+4 * (10+2* fake_bistream_s->add_display_framenum));
      dec_frame_dest_s = (wmt_frame_phy_addr_t   *)(p_relase_frame_num + 1);
#if TEST_METHOD == 1 
	if (test_fake_pattern_enter_cnt <= 14)
		*p_relase_frame_num = 0;
	else{
		if (!vdfbm_get_ref_fb(dec_frame_dest_s))
			*p_relase_frame_num = 1;
		else
			*p_relase_frame_num = 0;
	}
#elif TEST_METHOD == 2
      
#endif

      


	for (i=0;i<64;i+=4)
      	{
      		DBG(">> rdptr %08x :  0x%x 0x%x 0x%x 0x%x \n",(unsigned int)srcptr,*srcptr,*(srcptr+1),*(srcptr+2),*(srcptr+3));
      		srcptr +=4;
      	}
	DBG(" test_fake_pattern leave \n");
       return 0;
}
#endif
/*----------------------------------------------------------------------------*/
/*!*************************************************************************
* Private Function by Max Chen,2010/04/19
*
* h264_hw_get_prd_addr
*
* \brief  get physical addr to build prd for H/W
*
* \retval N/A
*/
static void h264_hw_get_prd_addr(unsigned int phyaddr, unsigned int virtaddr)
{
   h264_prd_phy_addr = phyaddr;
   h264_prd_virt_addr = virtaddr;

   DBG_DETAIL("Get Prd Table Phy  Addr: %8x ---\n", phyaddr);
   DBG_DETAIL("Get Prd Table Virt Addr: %8x ---\n", virtaddr);
   
   return;
} /* End of h264_hw_get_prd_addr() */





/*!*************************************************************************
* Private Function by Max Chen, 2010/04/19
*
* h264_var_reset
*
* \brief   Reset global variable
*
* \retval  N/A
*/
void h264_var_reset(void)
{
    //Reset Global variable
    frame_num_cnt = 0;
    wmt_h264_flush();
#ifdef H264_TEST_FAKE_BITSTREAM
    test_fake_pattern_enter_cnt=0;
#endif

    return;
} /* End of h264_var_reset() */






/*!*************************************************************************
* wmt_h264_get_capability
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Get H264 hardware capability
*
* \retval  0 if success
*/ 
int wmt_h264_get_capability(h264_drvinfo_t *drv)
{
    TRACE("Enter\n");

      drv->capab_tbl.mini_frame_count = H264_MIN_FRAME_COUNT;

    TRACE("Leave\n");

    return 0;
} /* End of wmt_h264_get_capability() */


/*!*************************************************************************
* wmt_h264_set_attr
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Set H264 hardware attribution
*
* \retval  0 if success
*/ 
int wmt_h264_set_attr(h264_drvinfo_t *drv)
{

    unsigned int i;
    TRACE("Enter\n");

    total_frame_count= drv->attr.di.total_frame_count;

    
     if (vdfbm_fb_init(total_frame_count)<0)
     		return -1;

    DBG("total_frame_count %d \n",total_frame_count); 

    for (i=0;i<total_frame_count;i++)
    {
	   g_fb[i].buffer.buf_y=(char *)drv->attr.di.fb[i].y_addr_phys;
	   g_fb[i].buffer.buf_c=(char *)drv->attr.di.fb[i].c_addr_phys;
	   g_fb[i].buf_y_user=(char *)drv->attr.di.fb[i].y_addr_user;
	   g_fb[i].buf_c_user=(char *)drv->attr.di.fb[i].c_addr_user;
	   g_fb[i].ext_level= drv->attr.di.fblvl;	
	   g_fb[i].mem_status = 0;

	   DBG(" Y 0x%08x  \n",drv->attr.di.fb[i].y_addr_phys);
	   
    }
  #ifdef H264_TEST_FAKE_BITSTREAM
    linewidth =drv->attr.di.fb[1].linesize & 0xffff;
    lineheight = (drv->attr.di.fb[1].linesize >> 16 ) & 0xffff;
    DBG(" #### line width %d  height %d\n",linewidth,lineheight);
 #else
    linewidth =drv->attr.di.fb[1].linesize & 0xffff;
 #endif


    TRACE("Leave\n");
        
    return 0;
} /* End of wmt_h264_set_attr() */


/*!*************************************************************************
* wmt_h264_decode_proc
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Start H264 hardware decode
*
* \retval  0 if success
*/ 
int wmt_h264_decode_proc(h264_drvinfo_t *drv)
{
    unsigned int* refprdvirt;
    unsigned int  ref_prd_phy;
    H264_Private_Packet_t  *private_packet;
    wmt_frame_phy_addr_t   *dec_frame_dist_addr_s;

    TRACE("Enter\n");
    h264_input_in =(avc_input_t *) &(drv->arg_in);
    
    h264_hw_get_prd_addr(drv->prd_addr, drv->prd_virt);

    
    //----------------------------
    DBG_DETAIL("Store input parameter\n");
   #ifdef H264_TEST_FAKE_BITSTREAM
	    if ( rdsize < 128)
	    {
			DBG_WARN("TEST_FAKE_BITSTREAM input size too small \n");
	       	return -1;
	    }
    #endif
    
    dec_frame_dist_addr_s = vdfbm_get_fb();
    if (dec_frame_dist_addr_s==0)
    {
	       DBG_ERR("frame buffer exausted !\n");
	    	return -1;
    }
    

   #ifdef H264_TEST_FAKE_BITSTREAM
	    if (test_fake_pattern(rdptr,dec_frame_dist_addr_s))
	    {
		       DBG_ERR("input size is not enough \n");
		    	return -1;
	    }
   #endif

    if( (h264_input_in->linesize)%16 ) {
        DBG_ERR("linesize need to do alignment\n");
    }


    refprdvirt = (unsigned int *)vdlib_prd_virt();
    private_packet = (H264_Private_Packet_t *)refprdvirt;

    private_packet->buffer_width = linewidth;
    private_packet->buffer_height = H264_MAX_IMG_HEIGHT;
    private_packet->current_frame_Y_buffer_address=(unsigned int)dec_frame_dist_addr_s->buf_y;
    private_packet->current_frame_C_buffer_address=(unsigned int)dec_frame_dist_addr_s->buf_c;
    private_packet->flag = h264_input_in->flags;//hurry up flag


    ref_prd_phy = vdlib_prd_phy();

    //Send decoding info to DSP
    dsplib_cmd_send(CMD_A2D_VDEC, h264_prd_phy_addr, ref_prd_phy, 
              (unsigned int)((char *) ref_prd_phy+H264_DSP_DECDONE_MEM_OFFSET)); 

    frame_num_cnt++;
    DBG_DETAIL("frame_num_cnt %d\n", frame_num_cnt);

    VDU_status = AVC_STA_DEC_RUN;

    TRACE("Leave\n");

    return 0;
} /* End of wmt_h264_decode_proc() */


/*!*************************************************************************
* wmt_h264_decode_finish
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Detect while H264 hardware decode finish
*
* \retval  0 if success
*/ 
int wmt_h264_decode_finish(h264_drvinfo_t *drv)
{
    int retval = 0;
   
    h264_frameinfo_t *info_out = &(drv->arg_out);
    dsplib_mbox_t mbox;
    char *dec_done_virt_addr;

    wmt_frame_phy_addr_t *cur_frame_s;
    wmt_frame_phy_addr_t  disp_frame_s;
    H264_Decode_Done_Private_Packet_t   *Dec_Done_Packet;

    int  *p_release_framenum,release_framenum;
    int i;

 #ifdef DEBUG   
    char *tmpptr;
#endif


    TRACE("Enter\n");

    //Get decoding info from DSP
    retval = dsplib_cmd_recv(&mbox, CMD_D2A_VDEC_DONE);

    //--get the virtural address of decode done
    dec_done_virt_addr = (char   *)(phys_to_virt(vdlib_prd_phy() + H264_DSP_DECDONE_MEM_OFFSET)); //(char   *)(phys_to_virt(mbox.w1));

#ifdef DEBUG
    tmpptr = (char  *)dec_done_virt_addr;
	for (i=0;i<64;i+=4)
      	{
      		DBG(">> rdptr %08x :  0x%x 0x%x 0x%x 0x%x \n",tmpptr,*tmpptr,*(tmpptr+1),*(tmpptr+2),*(tmpptr+3));
      		tmpptr +=4;
      	}
#endif

    Dec_Done_Packet = (H264_Decode_Done_Private_Packet_t   *)dec_done_virt_addr;

    DBG("bs_type %d \n",Dec_Done_Packet->bs_type);
    DBG("decode_status %d \n",Dec_Done_Packet->decode_status);
    DBG("frame_bitcnt %d \n",Dec_Done_Packet->frame_bitcnt);
    DBG("decoded_picture_height %d \n",Dec_Done_Packet->decoded_picture_height);
    DBG("decoded_picture_width %d \n",Dec_Done_Packet->decoded_picture_width);
    DBG("add_display_framenum %d \n",Dec_Done_Packet->add_display_framenum);

    if (Dec_Done_Packet->decode_status !=0)
    {
    	    DBG_ERR("decode done fail \n");
 	    goto Exit_err_h264_wait_finish;
    }

    //--process the add display frame
    cur_frame_s = ( wmt_frame_phy_addr_t  *)(dec_done_virt_addr + sizeof(H264_Decode_Done_Private_Packet_t));    
    DBG("decoded_picture  sizeof  %d\n",sizeof(H264_Decode_Done_Private_Packet_t));

    for (i = 0; i < Dec_Done_Packet->add_display_framenum; i++) {
	       DBG("decoded_picture i %d buf_y 0x%08x  buf_c 0x%08x\n",i,cur_frame_s->buf_y,cur_frame_s->buf_c);

		if (vdfbm_add_display_fb(cur_frame_s)<0){
	    	    	DBG_ERR("add display fail \n");
 	    		goto Exit_err_h264_wait_finish;
		}
		cur_frame_s++;
    }
    
    //-- process the release frame 
    p_release_framenum = (int *)(cur_frame_s);
    release_framenum = *p_release_framenum;
    DBG("release_framenum  %d\n",release_framenum);
    cur_frame_s = ( wmt_frame_phy_addr_t  *)(p_release_framenum+ 1);
    if ( *p_release_framenum <= 15)
    {
	    for (i = 0; i < release_framenum; i++) {
		       DBG("release_framenum i %d buf_y 0x%08x  buf_c 0x%08x\n",i,cur_frame_s->buf_y,cur_frame_s->buf_c);
			vdfbm_release_fb(cur_frame_s);
			cur_frame_s++;
	    }
    }else{
    	    DBG_ERR("Too many release frames \n");
    }
    //must return picture resolution at once
    //for h264,maybe we can't get one frame to display when we only decode the firts frame.
    h264_info_out->fb.img_w = Dec_Done_Packet->decoded_picture_width;   // width of valid image (unit: pixel)
    h264_info_out->fb.img_h = Dec_Done_Packet->decoded_picture_height;  // height of valid image (unit: line)
    
    //--get the display frame buffer from que
    VDU_status = AVC_STA_DEC_DONE;
    h264_info_out->status =H264_STA_DEC_DONE;
    
    if (vdfbm_get_display_fb(&disp_frame_s))
    {
	    	DBG(">> no disp frame \n");
        DBG("bs_type %d \n",Dec_Done_Packet->bs_type);
        DBG("decode_status %d \n",Dec_Done_Packet->decode_status);
        DBG("frame_bitcnt %d \n",Dec_Done_Packet->frame_bitcnt);
        DBG("decoded_picture_height %d \n",Dec_Done_Packet->decoded_picture_height);
        DBG("decoded_picture_width %d \n",Dec_Done_Packet->decoded_picture_width);
        DBG("add_display_framenum %d \n",Dec_Done_Packet->add_display_framenum);
        for (i = 0; i < Dec_Done_Packet->add_display_framenum; i++) 
	       DBG("decoded_picture i %d buf_y 0x%08x  buf_c 0x%08x\n",i,cur_frame_s->buf_y,cur_frame_s->buf_c);
        DBG("release_framenum  %d\n",release_framenum);
	    for (i = 0; i < release_framenum; i++) 
	       DBG("release_framenum i %d buf_y 0x%08x  buf_c 0x%08x\n",i,cur_frame_s->buf_y,cur_frame_s->buf_c);

	    h264_info_out->y_addr_user = (unsigned int) 0; //when player get this information, it means nothing to display
	    h264_info_out->c_addr_user = (unsigned int) 0;

    }else{
	    h264_info_out->y_addr_user = (unsigned int) disp_frame_s.buf_y;
	    h264_info_out->c_addr_user = (unsigned int) disp_frame_s.buf_c;

	    h264_info_out->consumedbytes = Dec_Done_Packet->frame_bitcnt;
	    h264_info_out->fb.bpp   = 8; // bits per pixel (8/16/24/32)
	    h264_info_out->fb.col_fmt = VDO_COL_FMT_YUV420; // Color format on frame buffer
    }


Exit_err_h264_wait_finish:
	
   memcpy(info_out, &h264info, sizeof(h264_frameinfo_t));


    if( VDU_status == AVC_STA_DEC_DONE ) {
        DBG_DETAIL("vdu deocded ok\n");
        retval = 0;
    }else {
        DBG_DETAIL("vdu deocded fail\n");
        retval = -1;
    }

    TRACE("Leave\n");

    return retval;
} /* End of wmt_h264_decode_finish() */



/*!*************************************************************************
* wmt_h264_open
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Open H264 hardware
*
* \retval  0 if success
*/ 
int wmt_h264_open(h264_drvinfo_t *drv)
{
    int retval = 0;

    TRACE("Enter\n");
    h264_var_reset();
    
    dsplib_cmd_send(CMD_A2D_VDEC_OPEN, VD_H264, 0, 0);

    TRACE("Leave\n");

    return retval;
} /* End of wmt_h264_open() */


/*!*************************************************************************
* wmt_h264_close
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Close H264 hardware
*
* \retval  0 if success
*/ 
int wmt_h264_close(void)
{
    TRACE("Enter\n");

    // Try to free H/W MV decode buffer
    // Disable clock
    h264_var_reset();

    dsplib_cmd_send(CMD_A2D_VDEC_CLOSE, VD_H264, 0, 0);

    TRACE("Leave\n");

    return 0;
} /* End of wmt_h264_close() */


/*!*************************************************************************
* wmt_h264_probe
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Probe function
*
* \retval  0 if success
*/ 
int wmt_h264_probe(void)
{
    TRACE("Enter\n");

    vdlib_prd_init();

    TRACE("Leave\n");

    return 0;
} /* End of wmt_h264_probe() */

int wmt_h264_flush(void)
{
    TRACE("Enter\n");
    vdfbm_flush();
    vdfbm_fb_init(total_frame_count);

    TRACE("Leave\n");

    return 0;
} /* End of wmt_h264_get_capability() */

/*!*************************************************************************
* wmt_h264_suspend
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Suspend function
*
* \retval  0 if success
*/ 
int wmt_h264_suspend(void)
{
    TRACE("Enter\n");

    TRACE("Leave\n");

    return 0;
} /* End of wmt_h264_suspend() */


/*!*************************************************************************
* wmt_h264_resume
* 
* API Function by Max Chen, 2010/04/19
*/
/*!
* \brief
*    Resume function
*
* \retval  0 if success
*/ 
int wmt_h264_resume(void)
{
    TRACE("Enter\n");

    TRACE("Leave\n");

    return 0;
} /* End of wmt_h264_resume() */



/*--------------------End of Function Body -----------------------------------*/

#undef WM8605_H264_C
#endif /* ifndef WM8605_H264_C */
