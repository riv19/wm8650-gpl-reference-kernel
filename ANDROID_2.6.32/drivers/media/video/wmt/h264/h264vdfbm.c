/*++ 
 * linux/drivers/media/video/wmt/h264/h264vdfbm.c
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

#define h264vdbfm_VDFBM_C


#include "h264vdfbm.h"
#include <mach/memblock.h>  /* For MB driver only */
#include "com-h264.h"

//#define DEBUG
//#define DEBUG_DETAIL

#ifdef DEBUG
#define DPRINTK(fmt, args...)   printk("{%s} " fmt, __FUNCTION__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

#ifdef DEBUG_DETAIL
#define DPRINTK_DETAIL(fmt, args...)   printk("{%s} " fmt, __FUNCTION__ , ## args)
#else
#define DPRINTK_DETAIL(fmt, args...)
#endif


#define DPRINTK_ERR(fmt, args...)   printk("[ERR]{ %s} " fmt, __FUNCTION__ , ## args)
#define DPRINTK_WARN(fmt, args...)   printk("[WARN]{ %s} " fmt, __FUNCTION__ , ## args)


 int MAX_FRAME_NUMBERS;

static int fb_idx = 0,disp_fb_cur_size = 0,disp_fb_wr_idx = 0;

wmt_frame_info_t g_fb[H264_MAX_FB] ;
wmt_frame_phy_addr_t g_disp_fb[H264_MAX_FB] ;

#define THE_MB_USER                 "VDFB-MB"

/*!*************************************************************************
* vdfbm_get_fb_index_map
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
* get a fb index mapping 		
*
* PARAMETERS:
*     wmt_frame_phy_addr_t *  
*
* RETURNS: 
* 	fb index 
*/

int vdfbm_get_fb_index_map(wmt_frame_phy_addr_t *src_struct)
{
	int i;
	for (i = 0; i < MAX_FRAME_NUMBERS; i++) {
		if ((g_fb[i].buffer.buf_y) == src_struct->buf_y)
			return i;
       }
	
       return -1;
} /* End of vdfbm_get_fb() */

/*!*************************************************************************
* vdfbm_fb_init
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
* 	init fb variable		
*
* PARAMETERS:
*     wmt_frame_phy_addr_t *  
*
* RETURNS: 
* 	fb index 
*/

int vdfbm_fb_init(unsigned int max_fb_count)
{

	DPRINTK(" wmt_frame_info_t %d wmt_frame_phy_addr_t %d \n",sizeof(wmt_frame_info_t),sizeof(wmt_frame_phy_addr_t));

	memset(g_fb,0,sizeof(wmt_frame_info_t)*H264_MAX_FB);	
	memset(g_disp_fb,0,sizeof(wmt_frame_phy_addr_t)*H264_MAX_FB);
	fb_idx = disp_fb_cur_size = disp_fb_wr_idx = 0;
	MAX_FRAME_NUMBERS = max_fb_count;
	DPRINTK("fb_idx %d \n",fb_idx);
       return 0;
} /* End of vdfbm_get_fb() */
/*!*************************************************************************
* vdfbm_fb_exit
* 
* Private Function by Max Chen, 2010/12/21
* 
* DESCRIPTION:
* 	init fb variable		
*
* PARAMETERS:
*     wmt_frame_phy_addr_t *  
*
* RETURNS: 
* 	fb index 
*/

int vdfbm_fb_exit(void)
{
	DPRINTK("ENTER\n");


	

       return 0;
} /* End of vdfbm_get_fb() */

/*!*************************************************************************
* vdfbm_get_fb
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
* get a free frame buffer		
*
* PARAMETERS:
*<bs_type> get a frame buffer for this type to display
*
* RETURNS: 
* a pointer point to the free frame buffer
*/

wmt_frame_phy_addr_t * vdfbm_get_fb(void)
{
    int  i,base=0;
    
	for (i = 0; i < MAX_FRAME_NUMBERS; i++) {//fb_idx
		if ((fb_idx+i)<MAX_FRAME_NUMBERS)
			base=0;
		else
			base=MAX_FRAME_NUMBERS;
		
            if (mb_counter((unsigned long)(g_fb[fb_idx+i-base].buffer.buf_y)) == g_fb[fb_idx+i-base].ext_level ) {
	           if (!g_fb[fb_idx+i-base].mem_status) {
	                g_fb[fb_idx+i-base].mem_status += 1;
	                fb_idx = fb_idx+i-base;
	        
	                return &g_fb[fb_idx++].buffer;
	            }else{
	            		DPRINTK_DETAIL(" no rel %d sts %d \n",i,g_fb[fb_idx+i-base].mem_status);
	            }
            }else{
	            		DPRINTK_DETAIL(" no mbrel %d \n",i);
            }
        }


        return 0;
} /* End of vdfbm_get_fb() */
/*!*************************************************************************
* vdfbm_get_ref_fb
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
*  [WARN] This function is for H264_TEST_FAKE_BITSTREAM only
* get a reference  frame buffer		
*
* PARAMETERS:
* wmt_frame_phy_addr_t * ret_frame 
*          return frame buffer 
* RETURNS: 
*    OK or not
*/
// [WARN] This function is for H264_TEST_FAKE_BITSTREAM only
 int vdfbm_get_ref_fb(wmt_frame_phy_addr_t * ret_frame)
{
    int  i,base=0;
    int  my_fb_idx=fb_idx;
    
	for (i = 0; i < MAX_FRAME_NUMBERS; i++) {//fb_idx
		if ((my_fb_idx+i)<MAX_FRAME_NUMBERS)
			base=0;
		else
			base=MAX_FRAME_NUMBERS;
		
            if (mb_counter((unsigned long)(g_fb[my_fb_idx+i-base].buffer.buf_y)) == g_fb[fb_idx+i-base].ext_level ) {
	           if (g_fb[my_fb_idx+i-base].mem_status==1) {
	                my_fb_idx = my_fb_idx+i-base;
	        	   *ret_frame=(g_fb[my_fb_idx++].buffer);
	                return 0;
	            }else{
	            		DPRINTK_DETAIL(" no rel %d sts %d \n",i,g_fb[my_fb_idx+i-base].mem_status);
	            }
            }else{
	            		DPRINTK_DETAIL(" no mbrel %d \n",i);
            }
        }


        return -1;
} /* End of vdfbm_get_fb() */



/*!*************************************************************************
* vdfbm_release_fb
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
* release the frame buffer		
*
* PARAMETERS:
*  <pframe>   a pointer point to the frame buffer that needs to be released
*
* RETURNS: 
*
*/
void vdfbm_release_fb(wmt_frame_phy_addr_t * pframe)
{
	// make sure reference count is nonzero
	int fb_index;

       fb_index = vdfbm_get_fb_index_map(pframe);
	DPRINTK("fb_index map %d \n",fb_index);
	if (fb_index<0){
		DPRINTK_ERR("index can't mapping !\n");
		return ;
	}
	g_fb[fb_index].mem_status =0 ;

	
} /* End of vdfbm_release_fb() */

/*!*************************************************************************
* vdfbm_flush
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
* flush all frame buffer in bs_type		
*
* PARAMETERS:
*  <bs_type>   frame buffer in this type need to flush
*
* RETURNS: 
*
*/
void vdfbm_flush(void)
{
    int n, fb_index;
    
    for(n = 0; n < MAX_FRAME_NUMBERS; n++) {
        g_fb[n].mem_status = 0;
    }

    while(disp_fb_cur_size)
    {
		if ((disp_fb_wr_idx+1) > disp_fb_cur_size )
		{
            fb_index=(disp_fb_wr_idx) - disp_fb_cur_size;
		}else{
			fb_index=(disp_fb_wr_idx) + MAX_FRAME_NUMBERS - disp_fb_cur_size;
		}
        if(g_disp_fb [fb_index].buf_y)
        {
            unsigned int yaddr = (unsigned int)__va(g_disp_fb [fb_index].buf_y);
            mb_put(yaddr);
        }
        disp_fb_cur_size--;
    }
    
    fb_idx  = disp_fb_wr_idx = disp_fb_cur_size = 0;
} /* End of vdfbm_flush() */

/*!*************************************************************************
* vdfbm_add_display_fb
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
* mark the frame buffer for display
*
* PARAMETERS:
*  <pframe>   a pointer point to the frame buffer that needs to be display
*
* RETURNS: 
*
*/
int vdfbm_add_display_fb(wmt_frame_phy_addr_t * pframe)
{
	// check whether out of max display que size
	if (disp_fb_cur_size== MAX_FRAME_NUMBERS)
		return -1;


       //add the frame to display que
	if (disp_fb_wr_idx <= (MAX_FRAME_NUMBERS-1))
	{
        if(pframe->buf_y){  //do mb_get before player get the frame from driver
                            //because dsp maybe will add several display and free the buffers at the same time
                            //and then driver buffer some frames, and next time when driver need a framebuffer
                            //these buffered framebuffer don't have any use connt, it will be used as next decode buffer
                            //so the display order should be wrong....
            unsigned int yaddr = (unsigned int)__va(pframe->buf_y);
            mb_get(yaddr);
        }
        
		g_disp_fb [disp_fb_wr_idx]=*pframe;
		
		DPRINTK_DETAIL("add display %d 0x%08x  0x%08x\n",disp_fb_wr_idx,g_disp_fb [disp_fb_wr_idx].buf_y,g_disp_fb [disp_fb_wr_idx].buf_c);

	}else{
		return -1;
	}
	//update the display que's current size and index
	disp_fb_cur_size++;
		
	if (disp_fb_wr_idx == (MAX_FRAME_NUMBERS-1))
		disp_fb_wr_idx=0;
	else
		disp_fb_wr_idx++;

	return 0;
} /* End of vdfbm_add_display_fb() */

/*!*************************************************************************
* vdfbm_get_display_fb
* 
* Private Function by Max Chen, 2009/10/10
* 
* DESCRIPTION:
* get the current display fb		
*
* PARAMETERS:
*     void   
*
* RETURNS: 
* 	fb structure 
*/


int vdfbm_get_display_fb(wmt_frame_phy_addr_t *ret_frame)
{
	wmt_frame_phy_addr_t frame_phy_addr;
	
	int fb_index;
	DPRINTK("enter\n");
	//check the error case 
	if (((disp_fb_wr_idx+1) > MAX_FRAME_NUMBERS) ||
		(disp_fb_cur_size > MAX_FRAME_NUMBERS) )
	{
		DPRINTK_ERR("out of range !\n");
		return -1;
	}

	if (disp_fb_cur_size==0){//current que  is empty
		DPRINTK("display que is empty ! \n");
		return -1;
	}else {
		if ((disp_fb_wr_idx+1) > disp_fb_cur_size )
		{
		      fb_index=(disp_fb_wr_idx+0) - disp_fb_cur_size;
		}else{
			fb_index=(disp_fb_wr_idx+0) + MAX_FRAME_NUMBERS - disp_fb_cur_size;
		}
        if(g_disp_fb [fb_index].buf_y)
        {
            unsigned int yaddr = (unsigned int)__va(g_disp_fb [fb_index].buf_y);
            mb_put(yaddr);
        }
		frame_phy_addr.buf_y= g_disp_fb [fb_index].buf_y;
		frame_phy_addr.buf_c= g_disp_fb [fb_index].buf_c;

		disp_fb_cur_size--;
		
	}

	fb_index = vdfbm_get_fb_index_map(&frame_phy_addr);

	DPRINTK("fb_index map %d \n",fb_index);
	if (fb_index<0){
		DPRINTK_ERR("index can't mapping !\n");
		return -1;
	}

	ret_frame->buf_y = g_fb[fb_index].buf_y_user;
	ret_frame->buf_c = g_fb[fb_index].buf_c_user;

	DPRINTK("leave\n");

	return 0;
} /* End of vdfbm_add_display_fb() */

/*--------------------End of Function Body -----------------------------------*/

#undef h264vdbfm_VDFBM_C
