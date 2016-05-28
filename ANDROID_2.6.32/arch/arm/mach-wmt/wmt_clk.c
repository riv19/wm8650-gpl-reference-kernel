/*
 * wmt_clk.c
 *
 *
 * Some descriptions of such software. Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 * WonderMedia Technologies, Inc.
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */


#include <linux/mtd/mtd.h>
#include "wmt_clk.h"

#define PMC_BASE 0xD8130000
#define PMC_PLL 0xD8130200
#define PMC_CLK 0xD8130250

#define SRC_FREQ 25


static void disable_dev_clk(enum dev_id dev)
{
	*(volatile unsigned int *)(PMC_CLK + ((dev/32) ? 4 : 0))
	&= ~(1 << (dev - ((dev/32) ? 32 : 0)));
}

static void enable_dev_clk(enum dev_id dev)
{
	*(volatile unsigned int *)(PMC_CLK + ((dev/32) ? 4 : 0))
	|= 1 << (dev - ((dev/32) ? 32 : 0));
}
/*
* PLLA return 0, PLLB return 1,
* PLLC return 2, PLLD return 3,
* PLLE return 4,
* device not found return 5.
* device has no divisor but has clock enable return 6.
*/
static int calc_pll_num(enum dev_id dev, int *div_offset)
{
	switch (dev) {
		case DEV_I2C1:
		*div_offset = 0x378;
		return 1;
		case DEV_I2C0:
		*div_offset = 0x36C;
		return 1;
		case DEV_PWM:
		*div_offset = 0x348;
		return 1;
		/*case DEV_GPIO:*/
		case DEV_SPI0:
		*div_offset = 0x33C;
		return 1;
		case DEV_SDMMC1:
		*div_offset = 0x340;
		return 1;
		case DEV_DVO:
		*div_offset = 0x374;
		return 2;
		case DEV_DSP:
		*div_offset = 0x308;
		return 5;
		case DEV_DDRMC:
		*div_offset = 0x310;
		return 3;
		case DEV_NA0:
		*div_offset = 0x358;
		return 4;
		case DEV_NA12:
		case DEV_VPP:
		*div_offset = 0x35C;
		return 1;
		case DEV_VDU:
		*div_offset = 0x31C;
		return 1;
		case DEV_NAND:
		*div_offset = 0x330;
		return 1;
		case DEV_SDMMC0:
		*div_offset = 0x328;
		return 1;
		case DEV_SF:
		*div_offset = 0x314;
		return 1;
		case DEV_ARM:
		*div_offset = 0x300;
			return 0;
		case DEV_AHB:
		*div_offset = 0x304;
			return 0;
		case DEV_APB:
		case DEV_CIR:
		*div_offset = 0x320;
			return 0;
		case DEV_UART0:
		case DEV_UART1:
		case DEV_I2S:
		case DEV_SCL444U:
		case DEV_GOVW:
		case DEV_VID:
		case DEV_RTC:
		case DEV_KEYPAD:
		case DEV_SCC:
		case DEV_HDMI:
		case DEV_DMA:
		case DEV_ROT:
 		case DEV_UHDC:
 		case DEV_PERM:
 		case DEV_DSPCFG:
		case DEV_AHBB:
 		case DEV_ETHMAC:
 		case DEV_ETHPHY:
		case DEV_VPU:
		case DEV_MBOX:
		case DEV_GE:
		case DEV_GOVRHD:
		*div_offset = 0x1;
		return 6;
		default:
		return 5;
	};
}


static int get_freq(enum dev_id dev, int *divisor) {

	unsigned int tmp, PLL, div = 0, pmc_clk_en;
	int PLL_NO, j = 0, div_addr_offs;

	PLL_NO = calc_pll_num(dev, &div_addr_offs);
	if (PLL_NO == 5) {
		printk(KERN_INFO"device not found");
		return -1;
	}

	/* if the clk of the dev is not enable, then enable it */
	if (dev < 64) {
		pmc_clk_en = *(volatile unsigned int *)(PMC_CLK + ((dev/32) ? 4 : 0));
		if (!(pmc_clk_en & (1 << (dev - ((dev/32) ? 32 : 0)))))
			enable_dev_clk(dev);
	}
	if (PLL_NO == 6)
		return div_addr_offs*1000000;

	div = *divisor = *(volatile unsigned char *)(PMC_BASE + div_addr_offs);
	/*printk(KERN_INFO"div_addr_offs=0x%x PLL_NO=%d \n", div_addr_offs, PLL_NO);*/
	tmp = *(volatile unsigned int *)(PMC_PLL + 4*PLL_NO);
	PLL = (SRC_FREQ*(tmp&0x3FF))/(((tmp>>10)&7)*(1<<((tmp>>13)&3)));

	PLL *= 1000000;
	if (dev != DEV_SDMMC0 && dev != DEV_SDMMC1) {
		return (PLL/(div&0x1F));
	} else {
		if (div & (1<<5))
			j = 1;
		div &= 0x1F;
		return (PLL/(div*(j?64:1)));
	}
}

static int set_divisor(enum dev_id dev, int unit, int freq, int *divisor) {

	unsigned int tmp, PLL, div = 0, pmc_clk_en;
	int PLL_NO, i, j = 0, div_addr_offs;

	PLL_NO = calc_pll_num(dev, &div_addr_offs);
	if (PLL_NO == 5) {
		printk(KERN_INFO"device not found");
		return -1;
	}

	if (PLL_NO == 6 || dev == DEV_CIR) {
		printk(KERN_INFO"device has no divisor");
		return -1;
	}

	tmp = *(volatile unsigned int *)(PMC_PLL + 4*PLL_NO);
	PLL = (SRC_FREQ*(tmp&0x3FF))/(((tmp>>10)&7)*(1<<((tmp>>13)&3)));

	/* if the clk of the dev is not enable, then enable it */
	if (dev < 64) {
		pmc_clk_en = *(volatile unsigned int *)(PMC_CLK + ((dev/32) ? 4 : 0));
		if (!(pmc_clk_en & (1 << (dev - ((dev/32) ? 32 : 0)))))
			enable_dev_clk(dev);
	}

	PLL *= 1000000;
	if (unit == 1)
		freq *= 1000;
	else if (unit == 2)
		freq *= 1000000;

	if (dev != DEV_SDMMC0 && dev != DEV_SDMMC1) {
		for (i = 1; i < 33; i++) {
			if ((i > 1 && (i%2)) && (PLL_NO == 2))
				continue;
			if ((PLL/i) <= ((unsigned int)freq)) {
				*divisor = div = i;
				break;
			}
		}
	} else {
		if ((PLL/64) >= ((unsigned int)freq))
				j = 1;
		for (i = 1; i < 33; i++)
			if ((PLL/(i*(j?64:1))) <= ((unsigned int)freq)) {
				*divisor = div = i;
				break;
			}
	}
	if (div != 0 && div < 33) {
		while ((*(volatile unsigned int *)PMC_BASE)&0x18)
		;
		while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
		;
		*(volatile unsigned int *)(PMC_BASE + div_addr_offs)
		=	(j?32:0) + ((div == 32) ? 0 : div);
		while ((*(volatile unsigned int *)PMC_BASE)&0x18)
		;
		while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
		;
		return (PLL/(div*(j?64:1)));
	}
	printk(KERN_INFO"no suitable divisor");
	return -1;
}

static int set_pll_speed(enum dev_id dev, int unit, int freq, int *divisor) {

	unsigned int tmp, PLL, PLLN=0, PLLD=0, PLLP=0, pmc_clk_en, base_f=1;
	unsigned int PN, PD, last_freq, base, minor, min_minor = 0xFF000000;
	int PLL_NO, div_addr_offs, PP;

	PLL_NO = calc_pll_num(dev, &div_addr_offs);
	if (PLL_NO == 5) {
		printk(KERN_INFO"device not found");
		return -1;
	}
	*divisor = *(volatile unsigned char *)(PMC_BASE + div_addr_offs);
	base = 1000000;
	if (unit == 1)
		base_f = 1000;
	else if (unit == 2)
		base_f = 1000000;
	else if (unit != 0) {
		printk(KERN_INFO"unit is out of range");
		return -1;
	}

	tmp = (*divisor) * freq;
	for (PD = 5; PD >= 3; PD--) {
		for (PP = 3; PP >= 0; PP--) {
			for (PN = 3; PN <= 1023; PN++) {
				if ((tmp * (PD*(1<<PP))) == (base * SRC_FREQ * PN)) {
					PLLN = PN;
					PLLD = PD;
					PLLP = PP;
					goto find;
				} else if ((tmp * (PD*(1<<PP))) < (base * SRC_FREQ * PN)) {
					minor = (base * SRC_FREQ * PN) - (tmp * (PD*(1<<PP)));
					if (minor < min_minor) {
						PLLN = PN;
						PLLD = PD;
						PLLP = PP;
						min_minor = minor;
					}
				}/* else if ((tmp * (PD*(1<<PP))) > (base * SRC_FREQ * PN))*/
			}//PN
		}//PP
	}//PD
	printk(KERN_INFO"minimum minor=0x%x\n", min_minor);
find:
	last_freq = (SRC_FREQ * PLLN) * (base / (PLLD*(1<<PLLP)*(*divisor)));
	printk(KERN_INFO"PLLN%d, PLLD%d, PLLP%d, freq=%dHz \n", PLLN, PLLD, PLLP, last_freq);
	PLL = (PLLP<<13) + (PLLD<<10) + PLLN;

	/* if the clk of the device is not enable, then enable it */
	if (dev < 64) {
		pmc_clk_en = *(volatile unsigned int *)(PMC_CLK + ((dev/32) ? 4 : 0));
		if (!(pmc_clk_en & (1 << (dev - ((dev/32) ? 32 : 0)))))
			enable_dev_clk(dev);
	}

	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	*(volatile unsigned int *)(PMC_PLL + 4*PLL_NO) = PLL;
	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	return last_freq;

	printk(KERN_INFO"no suitable pll");
	return -1;
}

static int set_pll_divisor(enum dev_id dev, int unit, int freq, int *divisor) {

	unsigned int PLL, PLLN=1, PLLD=1, PLLP=1, pmc_clk_en, ddv, pllx, old_divisor;
	unsigned int PN, PD, VD, last_freq, div=1, base, base_f=1;
	unsigned long long minor, min_minor = 0xFF000000;
	int PLL_NO, div_addr_offs, max_freq = 600, ii = 0, PP;

	PLL_NO = calc_pll_num(dev, &div_addr_offs);
	if (PLL_NO == 5) {
		printk(KERN_INFO"device not belong to PLL A B C D");
		return -1;
	}

	if (PLL_NO == 6  || dev == DEV_CIR) {
		printk(KERN_INFO"device has no divisor");
		return -1;
	}

	old_divisor = *(volatile unsigned char *)(PMC_BASE + div_addr_offs);
	/*printk(KERN_INFO"1div_addr_offs=0x%x PLL_NO=%d \n", div_addr_offs, PLL_NO);*/
	//tmp = *(volatile unsigned int *)(PMC_PLL + 4*PLL_NO);
	base = 1000000;
	if (unit == 1)
		base_f = 1000;
	else if (unit == 2)
		base_f = 1000000;
	else if (unit != 0) {
		printk(KERN_INFO"unit is out of range");
		return -1;
	}

	/*if (PLL_NO == 1 || PLL_NO == 2)*/
	max_freq = 600;
	//tmp = div * freq;
	for (PD = 5; PD >= 3; PD--) {
		for (PP = 2; PP >= 0; PP--) {
			for (VD = 32; VD >= 1; VD--) {
				if ((VD > 1 && (VD%2)) && (PLL_NO == 2))
					continue;
				for (PN = 3; PN <= 1023; PN++) {
					if ((SRC_FREQ * PN)/(PD) >= 600)
						break;
					if ((SRC_FREQ * PN)/(PD) < 300)
						continue;
					if (((SRC_FREQ * PN) * (base/(VD * PD*(1<<PP)))) == (freq * base_f)) {
						PLLN = PN;
						PLLD = PD;
						PLLP = PP;
						div = VD;
						printk(KERN_INFO"find the equal value");
						goto find;
					} else if (((SRC_FREQ * PN) * (base/(VD * PD*(1<<PP)))) < (freq * base_f)) {
						minor = (freq * base_f) - ((SRC_FREQ * PN) * (base/(VD * PD*(1<<PP))));
						/*if (unit == 2)
							minor = ((SRC_FREQ * PN) - (VD * (PD*(1<<PP)))) * base;
						else if (unit == 1)
							minor = (1000 * SRC_FREQ * PN) - (freq * VD * (PD*(1<<PP)));
						else 
							minor = (1000 * SRC_FREQ * PN) - ((freq * VD * (PD*(1<<PP)))/1000);*/
						//printk(KERN_INFO"minor=0x%x, min_minor=0x%x", minor, min_minor);
						if (minor < min_minor) {
							PLLN = PN;
							PLLD = PD;
							PLLP = PP;
							div = VD;
							min_minor = minor;
								/*if (min_minor < 1000000 && unit == 2)
									goto minimun;
								else if (min_minor < 1000 && unit == 1)
								 goto minimun;
								else if (min_minor < 20 && unit == 0)
								 goto minimun;*/
						}
					} else if (((SRC_FREQ * PN) * (base/(VD * PD*(1<<PP)))) > (freq * base_f)) {
						if (PLL_NO == 2) {
							minor = ((SRC_FREQ * PN) * (base/(VD * PD*(1<<PP)))) - (freq * base_f);
							if (minor < min_minor) {
								PLLN = PN;
								PLLD = PD;
								PLLP = PP;
								div = VD;
								min_minor = minor;
							}
						}
						break;
					}
				}//PN
			}//VD
		}//PP
	}//PD
/*minimun:*/
	printk(KERN_INFO"minimum minor=0x%x, unit=%d \n", (unsigned int)min_minor, unit);
find:
	
	ii = 0;
	if ((SRC_FREQ * PLLN)/(PLLD) >= max_freq)
		ii = 1<<18;
	*divisor = div;
	last_freq = (SRC_FREQ * PLLN) * (base / (PLLD*(1<<PLLP)*(*divisor)));
	printk(KERN_INFO"PLLN%d, PLLD%d, PLLP%d, divisor%d freq=%dHz \n",
	PLLN, PLLD, PLLP, *divisor, last_freq);
	PLL = (PLLP<<13) + (PLLD<<10) + PLLN + ii;


	/* if the clk of the device is not enable, then enable it */
	if (dev < 64) {
		pmc_clk_en = *(volatile unsigned int *)(PMC_CLK + ((dev/32) ? 4 : 0));
		if (!(pmc_clk_en & (1 << (dev - ((dev/32) ? 32 : 0)))))
			enable_dev_clk(dev);
	}

	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	printk(KERN_INFO"PLL0x%x, pll addr =0x%x\n", PLL, PMC_PLL + 4*PLL_NO);
	*(volatile unsigned int *)(PMC_PLL + 4*PLL_NO) = PLL;
	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	/*printk(KERN_INFO"PLLN%d, PLLD%d, PLLP%d, div%d div_addr_offs=0x%x\n",
	PLLN, PLLD, PLLP, div, div_addr_offs);*/
	if (old_divisor < *divisor) {
		*(volatile unsigned int *)(PMC_BASE + div_addr_offs)
		= /*(j?32:0) + */((div == 32) ? 0 : div)/* + (div&1) ? (1<<8): 0*/;
		while ((*(volatile unsigned int *)PMC_BASE)&0x18)
		;
		while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
		;
	}
	ddv = *(volatile unsigned int *)(PMC_BASE + div_addr_offs);
	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	if (old_divisor > *divisor) {
		*(volatile unsigned int *)(PMC_BASE + div_addr_offs)
		= /*(j?32:0) + */((div == 32) ? 0 : div)/* + (div&1) ? (1<<8): 0*/;
		while ((*(volatile unsigned int *)PMC_BASE)&0x18)
		;
		while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
		;
	}
	pllx = *(volatile unsigned int *)(PMC_PLL + 4*PLL_NO);
	printk(KERN_INFO"read divisor=%d, pll=0x%x from register\n", 0x1F&ddv, pllx);
	return last_freq;

	printk(KERN_INFO"no suitable divisor");
	return -1;
}

/*
* cmd : CLK_DISABLE disable clock,
*       CLK_ENABLE enable clock,
*       SET_DIV set clock by setting divisor only(auto enable clock),
*       SET_PLL set clock by setting PLL only(auto enable clock),
*       SET_PLLDIV set clock by setting PLL and divisor(auto enable clock).
* dev : Target device ID to be set the clock.
* unit : 0 = Hz, 1 = KHz, 2 = MHz.
* freq : frequency of the target to be set when cmd is "SET_XXX".
* return : The final clock freq, in Hz, will be returned when success,
*            -1 means fail (waiting busy timeout).
* caution :
* 1. The final clock freq maybe an approximative value,
*    equal to or less than the setting freq.
*/

int auto_pll_divisor(enum dev_id dev, enum clk_cmd cmd, int unit, int freq)
{
	int last_freq, divisor;

	switch (cmd) {
		case CLK_DISABLE:
			disable_dev_clk(dev);
			return 0;
		case CLK_ENABLE:
			last_freq = get_freq(dev, &divisor);
			return last_freq;
		case SET_DIV:
			divisor = 0;
			last_freq = set_divisor(dev, unit, freq, &divisor);
			return last_freq;
		case SET_PLL:
			divisor = 0;
			last_freq = set_pll_speed(dev, unit, freq, &divisor);
			return last_freq;
		case SET_PLLDIV:
			divisor = 0;
			last_freq = set_pll_divisor(dev, unit, freq, &divisor);
			return last_freq;
		default:
		printk(KERN_INFO"clock cmd unknow");
		return -1;
	};
}
EXPORT_SYMBOL(auto_pll_divisor);

/**
* freq = src_freq * PLLN/(PLLD*(2^PLLP))
*
* dev : Target device ID to be set the clock.
* PLLN : Feedback divider value.
* PLLD : Reference divider value.
* PLLP : Output divider value.
* dev_div : Divisor belongs to each device, 0 means not changed.
* return : The final clock freq, in Hz, will be returned when success,
*            -1 means fail (waiting busy timeout).
*
*
*/
int manu_pll_divisor(enum dev_id dev, int PLLN, int PLLD, int PLLP, int dev_div)
{

	unsigned int PLL, freq, pmc_clk_en;
	int PLL_NO, div_addr_offs, j = 0;
	
	if (PLLN < 3 || PLLN > 1023){
		printk(KERN_INFO"PLLN is out of range 3 ~ 1023");
		return -1;
	}
	if (PLLD < 3 || PLLD > 5){
		printk(KERN_INFO"PLLD is out of range 3 ~ 5");
		return -1;
	}
	if (PLLP < 0 || PLLP > 3){
		printk(KERN_INFO"PLLP is out of range 0 ~ 3");
		return -1;
	}
	if (((PLLN * SRC_FREQ)/PLLD) >= 600) {
		printk(KERN_INFO"SRC_FREQ*PLLN/PLLD should less then 600");
		return -1;
	}
	if (((PLLN * SRC_FREQ)/PLLD) < 300) {
		printk(KERN_INFO"SRC_FREQ*PLLN/PLLD should great then 299");
		return -1;
	}
	if (dev_div > ((dev == DEV_SDMMC0 || (dev == DEV_SDMMC1))?63:31)){
		printk(KERN_INFO"divisor is out of range 0 ~ 31");
		return -1;
	}

	PLL_NO = calc_pll_num(dev, &div_addr_offs);
	if (PLL_NO == 5) {
		printk(KERN_INFO"device not found");
		return -1;
	}
	//tmp = *(volatile unsigned int *)(PMC_PLL + 4*PLL_NO);
	if ((dev_div&32) && (dev == DEV_SDMMC0 || (dev == DEV_SDMMC1)))
		j = 1; /* sdmmc has a another divider = 64 */		
	freq = (1000 * SRC_FREQ * PLLN)/(PLLD*(1<<PLLP)*dev_div*(j?64:1));
	freq *= 1000;
	printk(KERN_INFO"PLLN%d, PLLD%d, PLLP%d, dev_div%d, freq=%dkHz\n", PLLN, PLLD, PLLP, dev_div, freq);

	PLL = (PLLP<<13) + (PLLD<<10) + PLLN;

	/* if the clk of the device is not enable, then enable it */
	if (dev < 64) {
		pmc_clk_en = *(volatile unsigned int *)(PMC_CLK + ((dev/32) ? 4 : 0));
		if (!(pmc_clk_en & (1 << (dev - ((dev/32) ? 32 : 0)))))
			enable_dev_clk(dev);
	}

	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	*(volatile unsigned int *)(PMC_PLL + 4*PLL_NO) = PLL;
	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	*(volatile unsigned int *)(PMC_BASE + div_addr_offs)
	= (j?32:0) + ((dev_div == 32) ? 0 : dev_div)/* + (dev_div&1) ? (1<<8): 0*/;
	PLL = (j?32:0) + ((dev_div == 32) ? 0 : dev_div) /*+ (dev_div&1) ? (1<<8): 0*/;
	printk(KERN_INFO"set divisor =0x%x, nand address=0x%x\n", PLL, (PMC_BASE + div_addr_offs));
	while ((*(volatile unsigned int *)PMC_BASE)&0x18)
	;
	while ((*(volatile unsigned int *)(PMC_BASE+4))&0x1FF)
	;
	return freq;
}
EXPORT_SYMBOL(manu_pll_divisor);

