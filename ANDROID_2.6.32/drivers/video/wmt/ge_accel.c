/*
 * ge_accel.c
 *
 * GE frame buffer driver for Linux.
 *
 * Copyright 2007-2011 WonderMedia Corporation. All rights reserved.
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

/**
 * API reference
 *
 * extern int ge_sys_init(ge_info_t **geinfo, void *priv);
 * extern int ge_sys_exit(ge_info_t *geinfo, void *priv);
 * extern int ge_lock(ge_info_t *geinfo);
 * extern int ge_unlock(ge_info_t *geinfo);
 * extern int ge_trylock(ge_info_t *geinfo);
 * extern void ge_wait_sync(ge_info_t *geinfo);
 * extern void WAIT_PXD_INT(void);
 * extern int ge_set_pixelformat(ge_info_t *geinfo, u32 pixelformat);
 * extern int ge_set_destination(ge_info_t *geinfo, ge_surface_t *dst);
 * extern int ge_set_source(ge_info_t *geinfo, ge_surface_t *src);
 * extern int ge_set_command(ge_info_t *geinfo, u32 cmd, u32 rop);
 * extern void ge_set_color(ge_info_t *geinfo,
 *         u32 r, u32 g, u32 b, u32 a, u32 pixfmt);
 * extern void ge_set_sck(ge_info_t *geinfo,
 *         u32 r, u32 g, u32 b, u32 a, u32 pixfmt);
 * extern void ge_set_dck(ge_info_t *geinfo,
 *         u32 r, u32 g, u32 b, u32 a, u32 pixfmt);
 * extern int ge_blit(ge_info_t *geinfo);
 * extern int ge_fillrect(ge_info_t *geinfo);
 * extern int ge_rotate(ge_info_t *geinfo, u32 arc);
 * extern int ge_mirror(ge_info_t *geinfo, int mode);
 * extern int amx_show_surface(ge_info_t *geinfo, int id,
 *         ge_surface_t *s, int x, int y)
 * extern int amx_get_surface(ge_info_t *geinfo, int id, ge_surface_t *s)
 * extern int amx_hide_surface(ge_info_t *geinfo, int id)
 * extern int amx_set_colorkey(ge_info_t *geinfo, int id,
 *         int enable, u32 r, u32 g, u32 b, u32 pixfmt);
 * extern int amx_set_alpha(ge_info_t *geinfo, int id, u32 alpha)
 * extern int amx_sync(ge_info_t *geinfo)
 */

#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>

#include "ge_accel.h"
#include "vpp.h"

#ifdef CONFIG_LOGO_WMT_ANIMATION
#include "bootanimation/animation.h"
#endif

#ifdef DEBUG
#define DPRINTK(fmt, args...) \
	printk(KERN_DEBUG "%s: " fmt, __func__ , ## args)
#define ENTER() DPRINTK("Enter %s, file:%s line:%d\n", \
	__func__, __FILE__, __LINE__)
#define LEAVE() DPRINTK("Exit %s, file:%s line:%d\n",\
	__func__, __FILE__, __LINE__)
#else
#define DPRINTK(fmt, args...)
#define ENTER()
#define LEAVE()
#endif

DECLARE_WAIT_QUEUE_HEAD(ge_wq);
ge_info_t *geinfo;

/*
 * About FIX_EGL_SWAP_BUFFER
 *
 * The patch from Willian Y.S. Fan is to fix sync problem
 * between EGLSwapBuffer and VT framebuffer on Android.
 * Since it will cause fb_pandisplay broken,
 * The patch should be removed eventually.
 *
 * Written by Vincent Chen.
 */
#define FIX_EGL_SWAP_BUFFER

#ifdef FIX_EGL_SWAP_BUFFER
extern bool bEGL_swap;
unsigned int fb_egl_swap = 0;
#endif /* FIX_EGL_SWAP_BUFFER */

static int allow_pan_display;

/**************************
 *    Export functions    *
 **************************/

/**
 * ge_vram_addr - Detect a valid address for VRAM.
 *
 */
#define M(x) ((x)<<20)
unsigned int ge_vram_addr(u32 size)
{
	unsigned int memsize = (num_physpages << PAGE_SHIFT);

	if (memsize > M(256)) {         /* 512M */
		memsize = M(512);
	} else if (memsize > M(128)) {  /* 256M */
		memsize = M(256);
	} else if (memsize > M(64)) {   /* 128M */
		memsize = M(128);
	} else if (memsize > M(32)) {   /* 64M */
		memsize = M(64);
	} else if (memsize > M(16)) {   /* 32M */
		memsize = M(32);
	} else {
		memsize = M(0);
	}

	DPRINTK("GE has detected that RAM size is %d MB \n", memsize>>20);
	return memsize - M(size);
}
EXPORT_SYMBOL(ge_vram_addr);

static irqreturn_t ge_interrupt(int irq, void *dev_id)
{
	volatile struct ge_regs_8430 *ge_regs;

	ge_regs = geinfo->mmio;

	/* Reset if GE timeout. */
	if ((ge_regs->ge_int_en & BIT9) && (ge_regs->ge_int_flag & BIT9)) {
		printk("%s: GE Engine Time-Out Status! \n", __func__);
		ge_regs->ge_eng_en = 0;
		ge_regs->ge_eng_en = 1;
		while (ge_regs->ge_status & (BIT5 | BIT4 | BIT3));
	}

	/* Clear GE interrupt flags. */
	ge_regs->ge_int_flag |= ~0;

	if (ge_regs->ge_status == 0)
		wake_up_interruptible(&ge_wq);
	else
		printk(KERN_ERR "%s: Incorrect GE status (0x%x)! \n",
			__func__, ge_regs->ge_status);

	return IRQ_HANDLED;
}

/**
 * ge_init - Initial and display framebuffer.
 *
 * Fill the framebuffer with a default color, back.
 * Display the framebuffer using GE AMX.
 *
 * Although VQ is supported in design, I just can't find any benefit
 * from VQ. It wastes extra continuous physical memory, and runs much
 * slower than direct register access. Moreover, the source code
 * becomes more complex and is hard to maintain. Accessing VQ from
 * the user space is also a nightmare. In brief, the overhead of VQ makes
 * it useless. In order to gain the maximum performance
 * from GE and to keep the driver simple, I'm going to stop using VQ.
 * I will use VQ only when it is necessary.
 *
 * @info is the fb_info provided by framebuffer driver.
 * @return zero on success.
 */
int ge_init(struct fb_info *info)
{
	static int boot_init; /* boot_init = 0 */
	volatile struct ge_regs_8430 *regs;
	struct fb_var_screeninfo *var;
	unsigned int offset;
	unsigned int chip_id;
	unsigned int ge_irq;
	ge_surface_t s;
	ge_surface_t cs;

	/*
	 * Booting time initialization
	 */
	if (!boot_init) {
		ge_sys_init(&geinfo, info);

		regs = geinfo->mmio;

		DPRINTK(KERN_INFO "ge: iomem region at 0x%lx, mapped to 0x%x, "
			"using %d, total %d\n",
			info->fix.mmio_start, (u32) geinfo->mmio,
			info->fix.mmio_len, info->fix.mmio_len);

		/* 0x00000 (fastest) - 0x30003 (slowest) */
		regs->ge_delay = 0x10001;

		ge_get_chip_id(geinfo, &chip_id);
		chip_id >>= 16;

		switch (chip_id) {
		case 0x3357:		/* VT8430 */
			ge_irq = 66;
			break;
		case 0x3426:		/* WM8510 */
			ge_irq = 85;	/* IRQ_NA12_6 */
			break;
		case 0x3437:		/* WM8435 */
			ge_irq = 83;	/* IRQ_VPP_IRQ7 */
			break;
		case 0x3429:		/* WM8425 */
			ge_irq = 71;	/* IRQ_VPP_IRQ7 */
			break;
		case 0x3451:		/* WM8440 */
		case 0x3465:		/* WM8650 */
			ge_irq = IRQ_VPP_IRQ7;
			break;
		default:
			ge_irq = 0;
			break;
		}

		/*
		 * GE interrupt is enabled by default.
		 */
		if (ge_irq) {
			request_irq(ge_irq, ge_interrupt, IRQF_DISABLED,
				"ge", NULL);
			regs->ge_int_en = BIT8 | BIT9;
		}

		vpp_set_govm_path(VPP_PATH_GOVM_IN_GE, 1); /* GOV.GE on */

		amx_set_csc(geinfo, AMX_CSC_JFIF_0_255);
		amx_set_alpha(geinfo, 0, 0xff);

		allow_pan_display = 1;

#ifdef FIX_EGL_SWAP_BUFFER
		allow_pan_display = 0;
#endif /* FIX_EGL_SWAP_BUFFER */
	}

	var = &info->var;

	offset = (var->yoffset * var->xres_virtual + var->xoffset);
	offset *= var->bits_per_pixel >> 3;

	s.addr = info->fix.smem_start + offset;
	s.xres = info->var.xres;
	s.yres = info->var.yres;
	s.xres_virtual = info->var.xres_virtual;
	s.yres_virtual = info->var.yres;
	s.x = 0;
	s.y = 0;

	switch (info->var.bits_per_pixel) {
	case 8:
		s.pixelformat = GEPF_LUT8;
		break;
	case 16:
		if ((info->var.red.length == 5) &&
			(info->var.green.length == 6) &&
			(info->var.blue.length == 5)) {
			s.pixelformat = GEPF_RGB16;
		} else if ((info->var.red.length == 5) &&
			(info->var.green.length == 5) &&
			(info->var.blue.length == 5)) {
			s.pixelformat = GEPF_RGB555;
		} else {
			s.pixelformat = GEPF_RGB454;
		}
		break;
	case 32:
		s.pixelformat = GEPF_RGB32;
		break;
	default:
		s.pixelformat = GEPF_RGB16;
		break;
	}

	amx_get_surface(geinfo, 0, &cs);

	if (!boot_init && !g_vpp.govrh_preinit &&
		memcmp(&cs, &s, sizeof(ge_surface_t))) {
		/* ge_clear */
		ge_lock(geinfo);
		ge_set_color(geinfo, 0, 0, 0, 0, s.pixelformat);
		ge_set_destination(geinfo, &s);
		ge_set_pixelformat(geinfo, s.pixelformat);
		ge_set_command(geinfo, GECMD_BLIT, 0xf0);
		ge_unlock(geinfo);		
	}
	boot_init = 1;

#ifdef FIX_EGL_SWAP_BUFFER
	if (!allow_pan_display && bEGL_swap)
		fb_egl_swap = s.addr;
	else
		fb_egl_swap = 0;

	if (!allow_pan_display && fb_egl_swap)
		s.addr = fb_egl_swap;
#endif /* FIX_EGL_SWAP_BUFFER */

	amx_show_surface(geinfo, 0, &s, 0, 0); /* id:0, x:0, y:0 */
	amx_sync(geinfo);

	return 0;
}

/**
 * ge_exit - Disable GE.
 *
 * No memory needs to be released here.
 * Turn off the AMX to stop displaying framebuffer.
 * Update the index of MMU.
 *
 * @info is fb_info from fbdev.
 * @return zero on success.
 */
int ge_exit(struct fb_info *info)
{
	release_mem_region(info->fix.mmio_start, info->fix.mmio_len);
	ge_sys_exit(geinfo, info);

	return 0;
}

static int get_args(unsigned int *to, void *from, int num)
{
	unsigned int count;

	count = sizeof(unsigned int);
	count *= num;

	if (copy_from_user(to, from, count)) {
		printk(KERN_ERR "%s: copy_from_user failure\n", __func__);
		return  -EFAULT;
	}

	return 0;
}

/**
 * ge_ioctl - Extension for fbdev ioctl.
 *
 * Not quite usable now.
 *
 * @return zero on success.
 */
int ge_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	volatile struct ge_regs_8430 *regs;
	int ret = 0;
	unsigned int chip_id;
	unsigned int args[8];

	regs = geinfo->mmio;

	unlock_fb_info(info);

	switch (cmd) {
	case GEIOSET_AMX_EN:
		if (arg) {
			/* Only enable G1 */
			regs->g1_amx_en = 1;
			regs->g2_amx_en = 0;
			regs->ge_reg_upd = 1;
		} else {
			/* Disable G1 and G2 */
			regs->g1_amx_en = 0;
			regs->g2_amx_en = 0;
			regs->ge_reg_upd = 1;
		}
		break;
	case GEIO_ALPHA_BLEND:
	case GEIOSET_OSD:
		ge_alpha_blend((u32)arg);
		break;
	case GEIOSET_COLORKEY:
		ge_set_amx_colorkey((u32)arg, 0);
		break;
	case GEIO_WAIT_SYNC:
		ge_wait_sync(geinfo);
		break;
	case GEIO_LOCK:
		switch (arg) {
		case 0:
			ret = ge_unlock(geinfo);
			break;
		case 1:
			ret = ge_lock(geinfo);
			break;
		default:
			ret = ge_trylock(geinfo);
			break;
		}
		break;
	case GEIOGET_CHIP_ID:
		ge_get_chip_id(geinfo, &chip_id);
		copy_to_user((void *)arg, (void *) &chip_id,
			sizeof(unsigned int));
		break;
	case GEIO_ROTATE:
		ret = get_args(args, (void *) arg, 6);
		if (ret == 0) {
			ge_simple_rotate(args[0], args[1], args[2], args[3],
					 args[4], args[5]);
		}
		break;
#ifdef CONFIG_LOGO_WMT_ANIMATION
	case GEIO_STOP_LOGO:
		printk("GEIO_STOP_LOGO\n");
		ret = animation_stop();
		break;
#endif /* CONFIG_LOGO_WMT_ANIMATION */
	case GEIO_ALLOW_PAN_DISPLAY:
		printk("GEIO_ALLOW_PAN_DISPLAY = %lu\n", arg);
		allow_pan_display = arg ? 1 : 0;
		break;
	default:
		ret = -1;
	}

	lock_fb_info(info);

	return ret;
}

int ge_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue,
	unsigned transp, struct fb_info *info)
{
	volatile struct ge_regs_8430 *regs;
	unsigned int chip_id;
	unsigned int color;

	ge_get_chip_id(geinfo, &chip_id);
	chip_id >>= 16;

	if (chip_id == 0x3357) { /* VT8430 */
		red   >>= 8;
		green >>= 8;
		blue  >>= 8;

		color = red;
		color <<= 8;
		color |= green;
		color <<= 8;
		color |= blue;

		regs = geinfo->mmio;
		regs->g1_lut_en = 1;
		regs->g1_lut_adr = regno;
		regs->g1_lut_dat = color;
	}

	return 0;
}

int wait_vsync(void)
{
	const int has_vbie = 1;

	if (has_vbie) {
		REG_VAL32(0xd8050f04) |= 0x4; /* VBIE */

		while (!(REG_VAL32(0xd8050f04) & 0x4))
			msleep_interruptible(10); /* 10 ms */
	}

	return 0;
}

int ge_sync(struct fb_info *info)
{
	ge_wait_sync(geinfo);

	return 0;
}

int ge_release(struct fb_info *info)
{
	if (ge_sem_owner == current)
		return ge_unlock(geinfo);

	return 0;
}

#define CONFIG_AMX_CROP_EN 0

/**
 * ge_pan_display - Pans the display.
 *
 * Pan (or wrap, depending on the `vmode' field) the display using the
 * `xoffset' and `yoffset' fields of the `var' structure.
 * If the values don't fit, return -EINVAL.
 *
 * @var: frame buffer variable screen structure
 * @info: frame buffer structure that represents a single frame buffer
 *
 * Returns negative errno on error, or zero on success.
 */
int ge_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	ge_surface_t s;
	unsigned int offset;
	volatile struct ge_regs_8430 *regs;
	volatile unsigned int *gov_regs;
	int direct;

	DPRINTK("%s: xoff = %d, yoff = %d, xres = %d, yres = %d \n",
	       __func__,
	       var->xoffset, var->yoffset,
	       info->var.xres, info->var.yres);

	if ((var->xoffset + info->var.xres > info->var.xres_virtual) ||
	    (var->yoffset + info->var.yres > info->var.yres_virtual)) {
		/* Y-pan is used in most case.
		 * So please make sure that yres_virtual is
		 * greater than (yres + yoffset).
		 */
		printk(KERN_ERR "%s: out of range \n", __func__);
		return -EINVAL;
	}

#if CONFIG_AMX_CROP_EN
	s.addr = info->fix.smem_start;
	s.xres = var->xres;
	s.yres = var->yres;
	s.xres_virtual = var->xres_virtual;
	s.yres_virtual = var->yres_virtual;
	s.x = var->xoffset;
	s.y = var->yoffset;
#else /* CONFIG_AMX_CROP_EN */
	s.addr = info->fix.smem_start;
	s.xres = var->xres;
	s.yres = var->yres;
	s.xres_virtual = var->xres_virtual;
	s.yres_virtual = var->yres_virtual;
	s.x = 0;
	s.y = 0;
	offset = var->yoffset * var->xres_virtual + var->xoffset;
	offset *= var->bits_per_pixel >> 3;
	s.addr += offset;
#ifdef CONFIG_LOGO_WMT_ANIMATION
	g_framebuffer_ofs = offset;
#endif /* CONFIG_LOGO_WMT_ANIMATION */
#endif /* CONFIG_AMX_CROP_EN */

	switch (var->bits_per_pixel) {
	case 8:
		s.pixelformat = GEPF_LUT8;
		break;
	case 16:
		if ((info->var.red.length == 5) &&
			(info->var.green.length == 6) &&
			(info->var.blue.length == 5)) {
			s.pixelformat = GEPF_RGB16;
		} else if ((info->var.red.length == 5) &&
			(info->var.green.length == 5) &&
			(info->var.blue.length == 5)) {
			s.pixelformat = GEPF_RGB555;
		} else {
			s.pixelformat = GEPF_RGB454;
		}
		break;
	case 32:
		s.pixelformat = GEPF_RGB32;
		break;
	default:
		s.pixelformat = GEPF_RGB16;
		break;
	}

	regs = geinfo->mmio;
	gov_regs = (unsigned int *)geinfo->mmio;
	gov_regs -= 64;

	direct = !(/*regs->g1_ck_en || */ regs->g2_amx_en ||
		(vpp_get_govm_path() & VPP_PATH_GOVM_IN_VPU) /* VPU */);

	/*
	printk("pan display g1 %d,g2 %d,vpu 0x%x,direct %d\n",
		regs->g1_ck_en, regs->g2_amx_en, gov_regs[3], direct);
	*/

	if (!direct && var->activate != FB_ACTIVATE_NOW)
		vpp_wait_vsync();

#ifdef FIX_EGL_SWAP_BUFFER
	if (!allow_pan_display && fb_egl_swap)
		s.addr = fb_egl_swap;
#endif /* FIX_EGL_SWAP_BUFFER */

	amx_show_surface(geinfo, 0, &s, 0, 0); /* id:0, x:0, y:0 */
	amx_sync(geinfo);

	{
		extern void vpp_set_mutex(int lock);

		vpp_set_mutex(1);
		vpp_pan_display(var, info, direct);
		vpp_set_mutex(0);
	}

	return 0;
}

/**
 *  ge_blank - for APM
 *
 */
int ge_blank(int mode, struct fb_info *info)
{
	volatile struct ge_regs_8430 *regs;

	/* Disable FB_BLANK due to the buggy VT8430 APM. */
	return 0;

	ge_wait_sync(geinfo);

	regs = geinfo->mmio;

	switch (mode) {
	case FB_BLANK_NORMAL:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_POWERDOWN:
		vpp_set_govm_path(VPP_PATH_GOVM_IN_GE,0);	/* Turn off GE by GOV */

		regs->g1_amx_en = 0;	/* G1 AMX disable */
		regs->g2_amx_en = 0;	/* G2 AMX disable */
		regs->ge_reg_upd = 1;	/* register update */
		break;
	case FB_BLANK_UNBLANK:
		regs->g1_amx_en = 1;	/* G1 AMX enable */
		regs->ge_reg_upd = 1;	/* register update */

		vpp_set_govm_path(VPP_PATH_GOVM_IN_GE,1);	/* Turn on GE by GOV */
		break;
	default:
		break;
	}

	return 0;
}

/**
 * ge_alpha_blend - Set alpha and fill transparent color to the RGB screen.
 *
 * The color consists of A, R, G, B. The screen is opaque while alpha{A} equals
 * to zero. The alpha blending formula is as follow:
 * DISPLAY = A*G1 + (0xFF - A)*VPU.
 * Please note that all colors on the G1 is effected by alpha
 * except the transparent color{R,G,B}.
 *
 * @param color is the transparency color of {A,R,G,B}
 */
void ge_alpha_blend(unsigned int color)
{
	return ge_set_amx_colorkey(color, 1);
}

void ge_set_amx_colorkey(unsigned int color, int erase)
{
	ge_surface_t s;
	u8 a, r, g, b;

	a = (u8)((color >> 24) & 0xff);
	r = (u8)((color >> 16) & 0xff);
	g = (u8)((color >> 8) & 0xff);
	b = (u8)(color & 0xff);

	/* Set transparency */
	amx_get_surface(geinfo, 0, &s);

	switch (s.pixelformat) {
	case GEPF_RGB16:
		/* 5:6:5 => 8:8:8 */
		r = (r >> 3) << 3;
		g = (g >> 2) << 2;
		b = (b >> 3) << 3;
		break;
	case GEPF_RGB555:
		/* 5:5:5 => 8:8:8 */
		r = (r >> 3) << 3;
		g = (g >> 3) << 3;
		b = (b >> 3) << 3;
		break;
	case GEPF_RGB454:
		/* 4:5:4 => 8:8:8 */
		r = (r >> 4) << 4;
		g = (g >> 3) << 3;
		b = (b >> 4) << 4;
		break;
	default:
		break;
	}

	ge_wait_sync(geinfo);

	if (a)
		amx_set_colorkey(geinfo, 0, 1, r, g, b, s.pixelformat);
	else
		amx_set_colorkey(geinfo, 0, 0, r, g, b, s.pixelformat);

	amx_sync(geinfo);

	if (erase) {
		ge_lock(geinfo);
		ge_set_color(geinfo, r, g, b, 0, s.pixelformat);
		ge_set_destination(geinfo, &s);
		ge_set_pixelformat(geinfo, s.pixelformat);
		ge_set_command(geinfo, GECMD_BLIT, 0xf0);
		ge_unlock(geinfo);
	}
}

void ge_simple_rotate(unsigned int phy_src, unsigned int phy_dst,
	int width, int height, int bpp, int arc)
{
	volatile struct ge_regs_8430 *regs;
	ge_surface_t s, d;
	unsigned int chip_id;
	unsigned int g1_lut_en = 0;
	unsigned int g2_lut_en = 0;

	regs = geinfo->mmio;

	if (arc == 0) {
		ge_simple_blit(phy_src, phy_dst, width, height, bpp);
		return;
	}

	switch (bpp) {
	case 8:
		s.pixelformat = GEPF_LUT8;
		d.pixelformat = GEPF_LUT8;
		break;
	case 16:
		s.pixelformat = GEPF_RGB16;
		d.pixelformat = GEPF_RGB16;
		break;
	case 32:
		s.pixelformat = GEPF_RGB32;
		d.pixelformat = GEPF_RGB32;
		break;
	default:
		/* Not supported */
		return;
	}

	s.addr = phy_src;
	s.x = 0;
	s.y = 0;
	s.xres = width;
	s.yres = height;
	s.xres_virtual = width;
	s.yres_virtual = height;

	d.addr = phy_dst;
	d.x = 0;
	d.y = 0;

	switch (arc) {
	case 90:
	case 270:
		d.xres = height;
		d.yres = width;
		d.xres_virtual = height;
		d.yres_virtual = width;
		break;
	default:
		d.xres = width;
		d.yres = height;
		d.xres_virtual = width;
		d.yres_virtual = height;
		break;
	}

	ge_get_chip_id(geinfo, &chip_id);
	chip_id >>= 16;


	/* Rotate */
	ge_lock(geinfo);

	switch (chip_id) {
	case 0x3357:		/* VT8430 */
	case 0x3400:		/* VT8500 */
	case 0x3426:		/* WM8510 */
		g1_lut_en = regs->g1_lut_en;
		g2_lut_en = regs->g2_lut_en;
		regs->g1_lut_en = 0;
		regs->g2_lut_en = 0;
		break;
	default:
		break;
	}

	ge_set_source(geinfo, &s);
	ge_set_destination(geinfo, &d);
	ge_set_pixelformat(geinfo, s.pixelformat);
	ge_rotate(geinfo, arc);
	ge_wait_sync(geinfo);

	switch (chip_id) {
	case 0x3357:		/* VT8430 */
	case 0x3400:		/* VT8500 */
	case 0x3426:		/* WM8510 */
		regs->g1_lut_en = g1_lut_en;
		regs->g2_lut_en = g2_lut_en;
		break;
	default:
		break;
	}

	ge_unlock(geinfo);
}

void ge_simple_blit(unsigned int phy_src, unsigned int phy_dst,
	int width, int height, int bpp)
{
	ge_surface_t s, d;

	switch (bpp) {
	case 8:
		s.pixelformat = GEPF_LUT8;
		d.pixelformat = GEPF_LUT8;
		break;
	case 16:
		s.pixelformat = GEPF_RGB16;
		d.pixelformat = GEPF_RGB16;
		break;
	case 32:
		s.pixelformat = GEPF_RGB32;
		d.pixelformat = GEPF_RGB32;
		break;
	default:
		/* Not supported */
		return;
	}

	s.addr = phy_src;
	s.x = 0;
	s.y = 0;
	s.xres = width;
	s.yres = height;
	s.xres_virtual = width;
	s.yres_virtual = height;

	d.addr = phy_dst;
	d.x = 0;
	d.y = 0;
	d.xres = width;
	d.yres = height;
	d.xres_virtual = width;
	d.yres_virtual = height;

	/* Blit */
	ge_lock(geinfo);

	ge_set_source(geinfo, &s);
	ge_set_destination(geinfo, &d);
	ge_set_pixelformat(geinfo, s.pixelformat);
	ge_blit(geinfo);
	ge_wait_sync(geinfo);

	ge_unlock(geinfo);
}

#ifdef CONFIG_LOGO_WMT_ANIMATION
void clear_animation_fb(void)
{
	ge_surface_t s;
	unsigned int offset = 0;

	amx_get_surface(geinfo, 0, &s);
	amx_hide_surface(geinfo, 0);
	vpp_wait_vsync();
	
	ge_lock(geinfo);
	ge_set_color(geinfo, 0, 0, 0, 0, s.pixelformat);
	ge_set_destination(geinfo, &s);
	ge_set_pixelformat(geinfo, s.pixelformat);
	ge_set_command(geinfo, GECMD_BLIT, 0xf0);
	ge_unlock(geinfo);

	vpp_wait_vsync();

#ifdef FIX_EGL_SWAP_BUFFER
	if (!allow_pan_display && fb_egl_swap)
		s.addr = fb_egl_swap;
#endif /* FIX_EGL_SWAP_BUFFER */

	amx_show_surface(geinfo, 0, &s, 0, 0);
	amx_sync(geinfo);
}
#endif
