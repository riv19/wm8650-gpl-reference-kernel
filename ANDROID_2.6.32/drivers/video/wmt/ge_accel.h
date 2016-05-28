/*
 * ge_accel.h
 *
 * GE frame buffer driver for Linux.
 *
 * Copyright 2008-2009 WonderMedia Corporation. All rights reserved.
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

#ifndef GE_ACCEL_H
#define GE_ACCEL_H 1

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include "ge_regs.h"

#define FB_ACCEL_WM8510 0x8510
#define V_MAX 2048
#define H_MAX 2048

extern struct task_struct *ge_sem_owner;

unsigned int ge_vram_addr(unsigned int size);
int ge_init(struct fb_info *info);
int ge_exit(struct fb_info *info);
int ge_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg);
int ge_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue,
	unsigned transp, struct fb_info *info);
int ge_pan_display(struct fb_var_screeninfo *var, struct fb_info *info);
int ge_sync(struct fb_info *info);
int ge_release(struct fb_info *info);
int ge_blank(int mode, struct fb_info *info);
void ge_set_amx_colorkey(unsigned int color, int erase);
void ge_alpha_blend(unsigned int color);
void ge_simple_rotate(unsigned int phy_src, unsigned int phy_dst,
	int width, int height, int bpp, int arc);
void ge_simple_blit(unsigned int phy_src, unsigned int phy_dst,
	int width, int height, int bpp);
int wait_vsync(void);
extern void vpp_wait_vsync(void);
extern void vpp_get_info(struct fb_var_screeninfo *var);
extern int vpp_pan_display(struct fb_var_screeninfo *var, struct fb_info *info,int enable);
extern unsigned long msleep_interruptible(unsigned int msecs);
#ifdef CONFIG_LOGO_WMT_ANIMATION
extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
extern int wmt_setsyspara(char *varname, char *varval);
#endif
#endif
