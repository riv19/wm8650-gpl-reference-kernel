/**
 * Author: Aimar Ma <AimarMa@wondermedia.com.cn> 
 *  
 * Show animation during kernel boot stage 
 *  
 *  
 **/

#include <linux/mm.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <mach/memblock.h>
#include <linux/version.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include "animation.h"
#include "buffer.h"
#include "LzmaDec.h"

#define THE_MB_USER "Boot-Animation"

#define DEFAULT_BUF_IMAGES  4       //default buffered image count

#undef THIS_DEBUG
//#define THIS_DEBUG
#define BOOTANIMATION

#ifdef THIS_DEBUG
#define LOG_DBG(fmt,args...)	printk(KERN_DEBUG "[Boot Animation] " fmt , ## args)
#define LOG_INFO(fmt,args...)	printk(KERN_INFO "[Boot Animation] " fmt, ## args)
#define LOG_ERROR(fmt,args...)	printk(KERN_ERR "[Boot Animation] " fmt , ## args)  
#else
#define LOG_DBG(fmt,args...)	
#define LOG_INFO(fmt,args...)
#define LOG_ERROR(fmt,args...)	printk(KERN_ERR "[Boot Animation] " fmt , ## args)  
#endif


// MUST match Windows PC tool. Don't change it.
struct animation_clip_header{
	int  xres;
	int  yres;
    int  linesize;
	unsigned char x_mode;
	unsigned char y_mode;
	short x_offset;
	short y_offset;
	unsigned char repeat;
	unsigned char reserved;
	int  interval;
	int  image_count;
	int  data_len;
};

// MUST match Windows PC tool. Don't change it.
struct file_header {
	int maigc;
	unsigned short version;
	unsigned char clip_count;
	unsigned char color_format;
    unsigned int  file_len;
};



struct play_context {
    struct animation_clip_header *clip;
    struct timer_list timer;
    wait_queue_head_t wait;
    int timer_stop;
    int xpos;           //  top postion
    int ypos;           //  left postion 
    
    animation_buffer buf;
};


//  globe value to stop the animation loop
static volatile int g_logo_stop = 0;
static struct       animation_fb_info fb;

static void *SzAlloc(void *p, size_t size) 
{ 
    void * add = (void *)mb_allocate(size);
    LOG_INFO("alloc: size %d, add = %p \n", size, add);
    return add;
}

static void SzFree(void *p, void *address) { 
    LOG_INFO("free: address = %p \n", address);
    if (address != 0) {
        mb_free((int)address);
    }
}

static ISzAlloc g_Alloc = { SzAlloc, SzFree };

unsigned int g_framebuffer_ofs;
//  draw data to FB
static int show_frame(struct play_context *ctx, unsigned char *data)
{
    unsigned char * dest;
    int linesize = fb.width * (fb.color_fmt + 1) * 2;
    int i = 0;

    struct animation_clip_header *clip = ctx->clip;
    
    if (g_logo_stop)
		return 0;
    dest = fb.addr+g_framebuffer_ofs;
 //   printk(KERN_INFO "dest = 0x%p src = 0x%p\n", dest, data);
    
    dest += ctx->ypos * linesize;
    dest += ctx->xpos * (fb.color_fmt + 1) * 2;
    
    for (i = 0; i <  clip->yres; i++) {
        memcpy(dest, data, clip->xres * (fb.color_fmt + 1) * 2);
        dest += linesize;
        data += clip->linesize;
    }
    
    //LOG_INFO("show_frame %ld\n", jiffies);
    return 0;
}

static int decompress(struct play_context * ctx, unsigned char *src, unsigned int src_len)
{
	SRes res = 0;
	CLzmaDec state;
    size_t inPos = 0;
    unsigned char * inBuf;
    SizeT inProcessed;
    
    
	// 1)  read LZMA properties (5 bytes) and uncompressed size (8 bytes, little-endian) to header
	UInt64 unpackSize = 0;
	int i;
	
	unsigned char * header = src;
	for (i = 0; i < 8; i++)
		unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);
	
	//  2) Allocate CLzmaDec structures (state + dictionary) using LZMA properties
	
	
	LzmaDec_Construct(&state);
    RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));
	if (res != SZ_OK)
		return res;
	
	//  3) Init LzmaDec structure before any new LZMA stream. And call LzmaDec_DecodeToBuf in loop
	LzmaDec_Init(&state);
	
	
	inBuf = header + LZMA_PROPS_SIZE + 8;
	inProcessed = src_len - LZMA_PROPS_SIZE - 8;

	for (;;)
	{		
		unsigned int outSize;
		unsigned char * outBuf = animation_buffer_get_writable(&ctx->buf, &outSize);
		
		unsigned int frame_size = outSize;
		unsigned int decoded = 0;
		ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
		ELzmaStatus status;
		while(1) {
			res = LzmaDec_DecodeToBuf(&state, outBuf + frame_size - outSize, &outSize,
				inBuf + inPos, &inProcessed, finishMode, &status);

			inPos += inProcessed;
			decoded += outSize;
            unpackSize -= outSize;
			outSize = frame_size - decoded;
            
			if(res != SZ_OK) 
				break;

			if (outSize == 0)
				break;
		}

		animation_buffer_write_finish(&ctx->buf, outBuf);
		
		if (res != SZ_OK || unpackSize == 0 || g_logo_stop == 1)
            break;
	}
    
    //  4) decompress finished, do clean job
    LzmaDec_Free(&state, &g_Alloc);
	return res;
}

static void play_timeout(unsigned long arg) 
{
    struct play_context *ctx = (struct play_context *)arg;
    
    //  if stop flag is 1, do not get frame anymore and prepare to quit
    /*
    if(g_stop == 1) {
     //   ctx->clip = NULL;
    //    wake_up_interruptible(&ctx->wait);
    
   //     unsigned char * data = buffer_get_readable(&index);
        LOG_INFO( "g_stop == 1, exit\n");
        return;
    }
    else 
    */

    //  try to get a valid frame and show it
    unsigned char * data = animation_buffer_get_readable(&ctx->buf);
    if (data != NULL) {
        if(!g_logo_stop) {
            show_frame(ctx, data);
            animation_buffer_read_finish(&ctx->buf, data);
            ctx->timer.expires = jiffies + msecs_to_jiffies(ctx->clip->interval);
            add_timer(&ctx->timer);
        }
        else {
            LOG_INFO( "g_logo_stop == 1, return immediately \n");
            animation_buffer_read_finish(&ctx->buf, data);
        }
        return;
    }
    else {
        if(!g_logo_stop) {
            if (ctx->timer_stop != 1) {
                LOG_INFO( "Timed out, frame not ready\n");
                //  wait one jiffy to check if there's a frame that is ready
                ctx->timer.expires = jiffies + 1; 
                add_timer(&ctx->timer);
            }
            else {
                LOG_INFO( "ctx->timer_stop == 1, wake_up_interruptible\n");
                ctx->clip = NULL;
                wake_up_interruptible(&ctx->wait);
                return;
            }
        }
    }
}
    
static void play_clip(struct animation_clip_header *clip, int index)
{
	//	start timer for animation playback
    struct play_context ctx;
    int buf_images;
    
    LOG_DBG("Start playing clip %d\n", index);
    
    init_waitqueue_head(&ctx.wait);
    ctx.clip = clip;
    ctx.timer_stop = 0;
    
    //  init the decompress buffer
	if (clip->repeat == 0) {
        buf_images = DEFAULT_BUF_IMAGES;
        if(buf_images > clip->image_count)
            buf_images = clip->image_count;
	}
	else {
        //  for the repeat clip, alloc a big memory to store all the frames
		buf_images = clip->image_count;
	}
    
    if( 0 != animation_buffer_init(&ctx.buf, clip->linesize * clip->yres, buf_images, &g_Alloc)){
        LOG_ERROR("Can't init animation buffer %dx%d\n", clip->linesize * clip->yres, buf_images);
        return;
    }
	
    
    ctx.xpos = clip->x_mode * (fb.width  / 2  - clip->xres / 2) + clip->x_offset;
    ctx.ypos = clip->y_mode * (fb.height / 2  - clip->yres / 2) + clip->y_offset;

    //int timer = SetTimer(index, clip->interval, NULL);
	//  start a timer
    
    init_timer(&ctx.timer);
    ctx.timer.data = (int)&ctx;
    ctx.timer.function = play_timeout;
    ctx.timer.expires = jiffies + msecs_to_jiffies(clip->interval);
    add_timer(&ctx.timer);
    
    
	LOG_INFO("Start Decompressing ... \n");  
    decompress(&ctx, (unsigned char *)clip + sizeof(struct animation_clip_header), clip->data_len);
    
    if (clip->repeat) {
		while (!g_logo_stop) {
			//  Fake decompress for REPEAT mode. (Only decompress the clip once so we can save more CPU)
			unsigned int outSize;
			unsigned char * outBuf;      
            
            outBuf = animation_buffer_get_writable(&ctx.buf, &outSize);
			animation_buffer_write_finish(&ctx.buf, outBuf);
        }
	}
    
    LOG_INFO("Decompress finished!\n");
   
    //	Kill Timer
    if (!g_logo_stop) {
        ctx.timer_stop = 1;
        LOG_INFO( "Wait the last frame \n");
        wait_event_interruptible(ctx.wait, !ctx.clip);
    }
    else{
        del_timer_sync(&ctx.timer);
    }
    
    LOG_INFO("Play clip %d finished\n",  index);
	animation_buffer_release(&ctx.buf, &g_Alloc);
}

extern void clear_animation_fb(void);

static int play_animation(void * arg)
{
    unsigned char * clip;
    int i;
    
    struct file_header *header = (struct file_header *)arg;
    int clip_count = header->clip_count;
	struct animation_clip_header **p = (struct animation_clip_header **)kmalloc(clip_count * sizeof(void *), GFP_KERNEL);
    
    if(!p) {
        LOG_ERROR ("Can't alloc clip header, count %d\n", clip_count);
        return -1;
    }
	
	clip = (unsigned char *)(header + 1);
    for (i = 0; i< clip_count; i++){
		p[i] = (struct animation_clip_header *)clip;
		clip += p[i]->data_len + sizeof(struct animation_clip_header);
	}

	clear_animation_fb();

    LOG_DBG( "Found %d clip(s)\n", clip_count);
	for (i = 0; i < clip_count; i++) {
        if (g_logo_stop == 0)
            play_clip(p[i], i);
	}
	
    
    g_Alloc.Free(&g_Alloc, arg);
    kfree(p);
    LOG_DBG( "Play animation finished \n");
	return 0;
}

unsigned char * dump;

int animation_start(void * animation_data, struct animation_fb_info *info)
{
    struct file_header *header;    	

   	if (animation_data != NULL) {
    	header = (struct file_header *)animation_data;
    
    	if (header->maigc != 0x12344321) {
			LOG_ERROR ("It's not a valid Animation file at 0x%p, first 4 bytes: 0x%x\n", animation_data, header->maigc);
			return -1;
		}
    
    	dump = (unsigned char * ) g_Alloc.Alloc(&g_Alloc, header->file_len);
	    if(!dump) {
    	    LOG_ERROR ("Can't alloc enough memory, length %d\n", header->file_len);
        	return -1;
    	}

    	memcpy(&fb, info, sizeof(fb));
    
    	//copy it to the new buffer and start the play thread
    	memcpy(dump, header, header->file_len);    
    	g_logo_stop = 0;
	} else {
    	if (dump)
    		kthread_run(play_animation, dump, "wmt-boot-animation");
    }
    
    return 0;
}

int animation_stop(void)
{
    g_logo_stop = 1;
    return 0;
}

EXPORT_SYMBOL(animation_start);
EXPORT_SYMBOL(animation_stop);
#undef BOOTANIMATION

