#ifndef _BOOTANIMATION_H_
#define _BOOTANIMATION_H_


struct animation_fb_info {
    unsigned char * addr;    // frame buffer start address
    unsigned int width;      // width
    unsigned int height;     // height
    unsigned int color_fmt;  // color format,  0 -- rgb565, 1 -- rgb888
};

#ifndef BOOTANIMATION
extern unsigned int g_framebuffer_ofs;
#endif
int animation_start(void * animation_data, struct animation_fb_info *info);

int animation_stop(void);

#endif
