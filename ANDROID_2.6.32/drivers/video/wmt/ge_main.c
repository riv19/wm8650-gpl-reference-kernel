/*
 * ge_main.c
 *
 * GE frame buffer driver for Linux.
 *
 * Supported architectures:
 * ARCH_VT8430, ARCH_WM8510, ARCH_WM8510_ECO2, ARCH_WM8435. ARCH_WM8440
 * ARCH_WM8650, ARCH_WM8710
 *
 * Copyright 2008-2011 WonderMedia Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY WONDERMEDIA CORPORATION ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL WONDERMEDIA CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include "ge_accel.h"

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, u_int32_t)
#endif

#ifdef CONFIG_LOGO_WMT_ANIMATION
#include "bootanimation/animation.h"
#endif

extern ge_info_t *geinfo;
static int vram_total __initdata;

static struct fb_fix_screeninfo __initdata gefb_fix = {
	.id             = "wmtfb",
	.smem_start     = 0,
	.smem_len       = 0,
	.type           = FB_TYPE_PACKED_PIXELS,
	.type_aux       = 0,
	.visual         = FB_VISUAL_TRUECOLOR,
	.xpanstep       = 1,
	.ypanstep       = 1,
	.ywrapstep      = 1,
	.line_length    = 0,
	.mmio_start     = 0xd8050000,
	.mmio_len       = 0x0700,
	.accel          = FB_ACCEL_WM8510
};

static struct fb_var_screeninfo __initdata gefb_var = {
	.xres           = CONFIG_DEFAULT_RESX,
	.yres           = CONFIG_DEFAULT_RESY,
	.xres_virtual   = CONFIG_DEFAULT_RESX,
	.yres_virtual   = (CONFIG_DEFAULT_RESY*2),
	/*
	.bits_per_pixel = 32,
	.red            = {16, 8, 0},
	.green          = {8, 8, 0},
	.blue           = {0, 8, 0},
	.transp         = {0, 0, 0},
	*/
	.bits_per_pixel = 16,
	.red            = {11, 5, 0},
	.green          = {5, 6, 0},
	.blue           = {0, 5, 0},
	.transp         = {0, 0, 0},
	.activate       = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE,
	.height         = -1,
	.width          = -1,
	.pixclock       = 39721,
	.left_margin    = 40,
	.right_margin   = 24,
	.upper_margin   = 32,
	.lower_margin   = 11,
	.hsync_len      = 96,
	.vsync_len      = 2,
	.vmode          = FB_VMODE_NONINTERLACED
};

static int gefb_open(struct fb_info *info, int user)
{
	return 0;
}

static int gefb_release(struct fb_info *info, int user)
{
	return ge_release(info);
}

static int gefb_check_var(struct fb_var_screeninfo *var,
			      struct fb_info *info)
{
	switch (var->bits_per_pixel) {
	case 1:
	case 8:
		if (var->red.offset > 8) {
			/* LUT8 */
			var->red.offset = 0;
			var->red.length = 8;
			var->green.offset = 0;
			var->green.length = 8;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->transp.offset = 0;
			var->transp.length = 0;
		}
		break;
	case 16:
		if (var->transp.length) {
			/* ARGB 1555 */
			var->red.offset = 10;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 5;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 15;
			var->transp.length = 1;
		} else {
			/* RGB 565 */
			var->red.offset = 11;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 6;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 0;
			var->transp.length = 0;
		}
		break;
	case 24:
		/* RGB 888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 32:
		/* ARGB 8888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	}
	return 0;
}

static int gefb_set_par(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;

	/* init your hardware here */
	if (var->bits_per_pixel == 8)
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else
		info->fix.visual = FB_VISUAL_TRUECOLOR;

	info->fix.line_length = var->xres_virtual * var->bits_per_pixel / 8;

	if (ge_init(info))
		return -ENOMEM;

	return 0;
}

static int gefb_setcolreg(unsigned regno, unsigned red,
			      unsigned green, unsigned blue,
			      unsigned transp, struct fb_info *info)
{
	if (regno >= 256)  /* no. of hw registers */
		return -EINVAL;

	/* grayscale */

	if (info->var.grayscale) {
		/* grayscale = 0.30*R + 0.59*G + 0.11*B */
		red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;
	}

	ge_setcolreg(regno, red, green, blue, transp, info);

	/*  The following is for fbcon. */

	if (info->fix.visual == FB_VISUAL_TRUECOLOR ||
		info->fix.visual == FB_VISUAL_DIRECTCOLOR) {

		if (regno >= 16)
			return -EINVAL;

		switch (info->var.bits_per_pixel) {
		case 16:
			((unsigned int *)(info->pseudo_palette))[regno] =
				(red & 0xf800) |
				((green & 0xfc00) >> 5) |
				((blue & 0xf800) >> 11);
				break;
		case 24:
		case 32:
			red   >>= 8;
			green >>= 8;
			blue  >>= 8;
			((unsigned int *)(info->pseudo_palette))[regno] =
				(red << info->var.red.offset) |
				(green << info->var.green.offset) |
				(blue  << info->var.blue.offset);
			break;
		}
	}
	return 0;
}

static int gefb_pan_display(struct fb_var_screeninfo *var,
				struct fb_info *info)
{
	ge_pan_display(var, info);
	return 0;
}

static int gefb_ioctl(struct fb_info *info, unsigned int cmd,
			  unsigned long arg)
{
	int retval = 0;

	if (_IOC_TYPE(cmd) == GEIO_MAGIC)
		return ge_ioctl(info, cmd, arg);

	switch (cmd) {
	case FBIO_WAITFORVSYNC:
		vpp_wait_vsync();
		break;
	default:
		break;
	}

	return retval;
}

int gefb_hw_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	return 0;
}

int gefb_sync(struct fb_info *info)
{
	return ge_sync(info);
}

static struct fb_ops gefb_ops = {
	.owner          = THIS_MODULE,
	.fb_open        = gefb_open,
	.fb_release     = gefb_release,
	.fb_check_var   = gefb_check_var,
	.fb_set_par     = gefb_set_par,
	.fb_setcolreg   = gefb_setcolreg,
	.fb_pan_display = gefb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
	.fb_blank       = ge_blank,
	.fb_cursor      = gefb_hw_cursor,
	.fb_ioctl       = gefb_ioctl,
	.fb_sync	= gefb_sync,
};

static int __init gefb_setup(char *options)
{
	char *this_opt;
	unsigned long val;

	if (!options || !*options)
		return 0;

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt)
			continue;
		if (!strncmp(this_opt, "vtotal:", 7)) {
			if (!strict_strtoul(this_opt + 7, 10, &val))
				vram_total = val;
		}
	}

	return 0;
}

#ifdef CONFIG_LOGO_WMT_ANIMATION
static void *get_animation_data()
{
	unsigned char buf[80];
	const int buflen = 80;
	const char *varname = "wmt.kernel.animation.addr";
	int err = wmt_getsyspara(varname, buf, &buflen);
	unsigned long addr;
	void *data;
	char *endp;

	data = NULL;
	if (!err) {
		/* addr = simple_strtoul(buf, &endp, 16); */
		if (strict_strtoul(buf, 16, &addr) < 0)
			return NULL;
		/*
		 * Someone writes the logo.data here temporarily in U-Boot,
		 * and assume the max length is 1 MB
		 * FIXME: What if n is mapped by other modules?
		 */
		data = ioremap(addr, 0x100000); /* 1MB */
		if (!data)
			printk(KERN_ERR "%s: ioremap fail at 0x%lx\n",
			       varname, addr);
		else {
			printk(KERN_INFO "%s = %s, 0x%lx map to 0x%p\n",
			       varname, buf, addr, data);
		}
	} else {
		printk(KERN_INFO "No %s found\n", varname);
		data = NULL;
		/*
		 * Note: call stop_animation is safe even no start_animation
		 */
	}
	return (void *)data;
}

static void run_animation(struct fb_info *info, void *data)
{
	struct animation_fb_info fb;

	if (data) {
		fb.addr = info->screen_base;
		fb.width = info->var.xres;
		fb.height = info->var.yres;
		/* 0 = 565, 1 = 888 */
		fb.color_fmt = (info->var.bits_per_pixel == 16) ? 0 : 1;
		animation_start(data, &fb);
		iounmap(data);
	}
}
#endif

static int __init gefb_probe(struct platform_device *dev)
{
	struct fb_info *info;
	int cmap_len, retval;
	unsigned int chip_id;
	char mode_option[] = "1024x768@60";
#ifdef CONFIG_LOGO_WMT_ANIMATION
	unsigned int frame_size;
	void *animation_data;
#endif

	/*  Dynamically allocate memory for fb_info and par.*/
	info = framebuffer_alloc(sizeof(unsigned int) * 16, &dev->dev);
	if (!info) {
		release_mem_region(info->fix.smem_start, info->fix.smem_len);
		return -ENOMEM;
	}

	/* Set default fb_info */
	info->fbops = &gefb_ops;
	info->fix = gefb_fix;

	chip_id = SCC_CHIP_ID >> 16;

	/* Auto detect VRAM */
	if (vram_total) {
		info->fix.smem_len = vram_total * 0x100000;
		info->fix.smem_start = ge_vram_addr(vram_total);
	} else if (CONFIG_GE_BUFFER_SIZE) {
		info->fix.smem_len = CONFIG_GE_BUFFER_SIZE * 0x100000;
		info->fix.smem_start = ge_vram_addr(CONFIG_GE_BUFFER_SIZE);
	} else {
		/*
		 * Do NOT modify the following settings!
		 * Use video=gefb:vtotal:N as boot parameter
		 * to change GE buffer size to N MiB.
		 * Or set CONFIG_GE_BUFFER_SIZE to N by either
		 * menuconfig or .config directly.
		 */
		switch (chip_id) {
		case 0x3357: /* WM8430 */
			info->fix.smem_len = 0x400000;
			info->fix.smem_start = ge_vram_addr(4);
			break;
		case 0x3429: /* WM8505 */
		case 0x3465: /* WM8650 */
			info->fix.smem_len = 0x800000;
			info->fix.smem_start = ge_vram_addr(8);
			break;
		case 0x3426: /* WM8510 */
		case 0x3437: /* WM8435 */
		case 0x3451: /* WM8440 */
		default:
			info->fix.smem_len = 0xc00000;
			info->fix.smem_start = ge_vram_addr(12);
		}
	}

	/* Set video memory */
	if (!request_mem_region(info->fix.smem_start,
		info->fix.smem_len, "gefb")) {
		printk(KERN_WARNING
			"%s: request memory region failed at %p\n",
			__func__, (void *)info->fix.smem_start);
	}

	info->screen_base = ioremap(info->fix.smem_start,
		info->fix.smem_len);
	if (!info->screen_base) {
		printk(KERN_ERR "%s: ioremap fail %d bytes at %p\n",
			__func__, info->fix.smem_len,
			(void *)info->fix.smem_start);
		return -EIO;
	}

	printk(KERN_INFO "gefb: framebuffer at %p, mapped to %p, "
		"using %dk, total %dk\n",
		(void *)info->fix.smem_start, (void *)info->screen_base,
		info->fix.smem_len >> 10, info->fix.smem_len >> 10);

#ifdef CONFIG_LOGO_WMT_ANIMATION
	animation_data = get_animation_data();
#endif

	/*
	 *  Do as a normal fbdev does, but allocate a larger memory for GE.
	 */

	/*
	 * The pseudopalette is an 16-member array for fbcon.
	 */
	info->pseudo_palette = info->par;
	info->par = NULL;
	info->flags = FBINFO_DEFAULT;	/* flag for fbcon */

	/*
	 * This should give a reasonable default video mode.
	 */
	retval = fb_find_mode(&info->var, info, mode_option,
			      NULL, 0, NULL, 8);

	if (!retval || retval == 4)
		return -EINVAL;

	/*
	 *  This has to been done !!!
	 */
	cmap_len = 256;	/* Be the same as VESA */
	retval = fb_alloc_cmap(&info->cmap, cmap_len, 0);
	if (retval < 0)
		printk(KERN_ERR "%s: fb_alloc_cmap fail.\n", __func__);

	/*
	 *  The following is done in the case of
	 *  having hardware with a static mode.
	 */
	info->var = gefb_var;

	/*
	 *  Load video output setting from VPP.
	 */
	vpp_get_info(&info->var);

	/*
	 *  For drivers that can...
	 */
	gefb_check_var(&info->var, info);

	/*
	 *  It's safe to allow fbcon to do it for you.
	 *  But in this case, we need it here.
	 */
	gefb_set_par(info);

	if (register_framebuffer(info) < 0) {
		ge_exit(info);
		return -EINVAL;
	}
	printk(KERN_INFO "fb%d: %s frame buffer device\n",
		info->node, info->fix.id);
	dev_set_drvdata(&dev->dev, info);

#ifdef CONFIG_LOGO_WMT_ANIMATION
	/*  start boot animation */
	run_animation(info, animation_data);
#endif

	return 0;
}

static int gefb_remove(struct platform_device *dev)
{
	struct fb_info *info = dev_get_drvdata(&dev->dev);

	if (info) {
		ge_exit(info);
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		framebuffer_release(info);
	}
	return 0;
}

#include "vpp.h"
static u32 *vpp_ge1_reg_ptr;
static u32 *vpp_ge2_reg_ptr;
static u32 *vpp_ge3_reg_ptr;
static int gefb_suspend(struct platform_device *dev, pm_message_t state)
{
	extern unsigned int fb_egl_swap;
    ge_surface_t cs;
    
    //Android 2.2 : WM3465 Task 10852 : Display - Resume incorrect sometimes.
    //Clear EGL swap buffer while suspend.	
    //printk("gefb_suspend\n");
	ge_lock(geinfo);
	amx_get_surface(geinfo, 0, &cs);
	if (fb_egl_swap != 0)
		cs.addr = fb_egl_swap;
	ge_set_color(geinfo, 0, 0, 0, 0, cs.pixelformat);
	ge_set_destination(geinfo, &cs);
	ge_set_pixelformat(geinfo, cs.pixelformat);
	ge_set_command(geinfo, GECMD_BLIT, 0xf0);
	ge_wait_sync(geinfo);
	ge_unlock(geinfo);

	REG32_VAL(GE3_BASE_ADDR+0xd4) = 0x1;
	REG32_VAL(GE3_BASE_ADDR+0xa8) = 0x0;
	REG32_VAL(GE3_BASE_ADDR+0xac) = 0x0;

	vpp_ge1_reg_ptr = vpp_backup_reg(GE1_BASE_ADDR+0x00,0xFC);			/* 0x00 - 0xF4 */
	vpp_ge2_reg_ptr = vpp_backup_reg(GE2_BASE_ADDR+0x00,0xFC);			/* 0x00 - 0xF4 */
	vpp_ge3_reg_ptr = vpp_backup_reg(GE3_BASE_ADDR+0x00,0xFC);			/* 0x00 - 0xF4 */

	return 0;
}

static int gefb_resume(struct platform_device *dev)
{
	vpp_restore_reg(GE1_BASE_ADDR+0x00,0xFC,vpp_ge1_reg_ptr);			/* 0x00 - 0xFC */
	vpp_restore_reg(GE2_BASE_ADDR+0x00,0xFC,vpp_ge2_reg_ptr);			/* 0x00 - 0xFC */
	vpp_restore_reg(GE3_BASE_ADDR+0x00,0xFC,vpp_ge3_reg_ptr);			/* 0x00 - 0xFC */
	REG32_VAL(GE3_BASE_ADDR+0xd4) = 0x0;
	return 0;
}

static struct platform_driver gefb_driver = {
	.driver.name    = "gefb",
	.probe          = gefb_probe,
	.remove         = gefb_remove,
	.suspend        = gefb_suspend,
	.resume         = gefb_resume,
};

static u64 vt8430_fb_dma_mask = 0xffffffffUL;
static struct platform_device gefb_device = {
	.name   = "gefb",
	.dev    = {
		.dma_mask = &vt8430_fb_dma_mask,
		.coherent_dma_mask = ~0,
	},
};

static int __init gefb_init(void)
{
	int ret;
	char *option = NULL;


	vram_total = 0;

	fb_get_options("gefb", &option);
	gefb_setup(option);

	ret = platform_driver_register(&gefb_driver);
	if (!ret) {
		ret = platform_device_register(&gefb_device);
		if (ret)
			platform_driver_unregister(&gefb_driver);
	}
	return ret;
}
module_init(gefb_init);

static void __exit gefb_exit(void)
{
	platform_driver_unregister(&gefb_driver);
	platform_device_unregister(&gefb_device);
	return;
}

module_exit(gefb_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WM GE framebuffer driver");
MODULE_LICENSE("GPL");

