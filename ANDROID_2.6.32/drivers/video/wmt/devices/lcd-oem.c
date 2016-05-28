/*++ 
 * linux/drivers/video/wmt/lcd-oem.c
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

#define LCD_OEM_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include <linux/kernel.h>
#include <linux/slab.h>
#include "../lcd.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  LCD_OEM_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LCD_OEM_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx lcd_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in lcd.h  -------------*/
static void lcd_oem_initial(void);

/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  lcd_xxx;        *//*Example*/
lcd_parm_t lcd_oem_parm = {
	.name = "WonderMedia OEM LCD (CHILIN LW080AT111)",
	.fps = 48,						/* frame per second */
	.bits_per_pixel = 18,
	.capability = LCD_CAP_CLK_HI,
	.timing = {
		.pixel_clock = 27000000,	/* pixel clock */
		.option = 0,				/* option flags */

#if 1
		.hsync = 10,				/* horizontal sync pulse */
		.hbp = 50,					/* horizontal back porch */
		.hpixel = 800,				/* horizontal pixel */
		.hfp = 50,					/* horizontal front porch */

		.vsync = 5,					/* vertical sync pulse */
		.vbp = 17,					/* vertical back porch */
		.vpixel = 480,				/* vertical pixel */
		.vfp = 16,					/* vertical front porch */
#else
		.hsync = 9,					/* horizontal sync pulse */
		.hbp = 18,					/* horizontal back porch */
		.hpixel = 800,				/* horizontal pixel */
		.hfp = 17,					/* horizontal front porch */

		.vsync = 5,					/* vertical sync pulse */
		.vbp = 10,					/* vertical back porch */
		.vpixel = 480,				/* vertical pixel */
		.vfp = 9,					/* vertical front porch */
#endif		
	},
	
	.initial = lcd_oem_initial,		
};

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lcd_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
static void lcd_oem_initial(void)
{	
	DPRINT("lcd_oem_initial\n");
		
	/* TODO */
}

lcd_parm_t *lcd_oem_get_parm(int arg) 
{	
	return &lcd_oem_parm;
}

static int lcd_oem_init(void){	
	int ret;	

	ret = lcd_panel_register(LCD_WMT_OEM,(void *) lcd_oem_get_parm);	
	return ret;
} /* End of lcd_oem_init */
module_init(lcd_oem_init);

/*--------------------End of Function Body -----------------------------------*/
#undef LCD_OEM_C
