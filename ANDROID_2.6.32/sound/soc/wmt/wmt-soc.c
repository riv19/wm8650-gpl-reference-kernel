/*++ 
 * linux/sound/soc/wmt/wmt-soc.c
 * WonderMedia audio driver for ALSA
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


#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#include "wmt-soc.h"
#include "wmt-pcm.h"
#include "../codecs/wmt_vt1602.h"
#include "../codecs/wmt_hwdac.h"
#include "../codecs/wm8900.h"
#include "../codecs/vt1603.h"
extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
char wmt_codec_name[80];
char wmt_dai_name[80];
/*
 * Debug
 */
#define AUDIO_NAME "WMT_SOC"

//#define WMT_SOC_DEBUG 1
//#define WMT_SOC_DEBUG_DETAIL 1

#ifdef WMT_SOC_DEBUG
#define DPRINTK(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#else
#define DPRINTK(format, arg...) do {} while (0)
#endif

#ifdef WMT_SOC_DEBUG_DETAIL
#define DBG_DETAIL(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": [%s]" format "\n" , __FUNCTION__, ## arg)
#else
#define DBG_DETAIL(format, arg...) do {} while (0)
#endif

#define err(format, arg...) \
	printk(KERN_ERR AUDIO_NAME ": " format "\n" , ## arg)
#define info(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#define warn(format, arg...) \
	printk(KERN_WARNING AUDIO_NAME ": " format "\n" , ## arg)

static int wmt_soc_startup(struct snd_pcm_substream *substream)
{
	DBG_DETAIL();
	
	return 0;
}

static void wmt_soc_shutdown(struct snd_pcm_substream *substream)
{
	DBG_DETAIL();
}

static int wmt_soc_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int err = 0;

	DBG_DETAIL();
	
	if (!strcmp(wmt_dai_name, "i2s")) {
		if (strcmp(wmt_codec_name, "hwdac")) {
			/* Set codec DAI configuration */
			err = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_CBS_CFS |
					 SND_SOC_DAIFMT_I2S);
		}
		if (err < 0)
			return err;

		/* Set cpu DAI configuration for I2S */
		err = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S);
	}
	else {
		/* Set cpu DAI configuration for AC97 */
		err = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_AC97);
	}

	if (err < 0)
		return err;

	if ((!strcmp(wmt_dai_name, "i2s")) &&
		((!strcmp(wmt_codec_name, "vt1602")) || (!strcmp(wmt_codec_name, "vt1603")))) {
		/* Set the codec system clock for DAC and ADC */
		if (!(params_rate(params) % 11025))
			err = snd_soc_dai_set_sysclk(codec_dai, 0, 11289600,
						    SND_SOC_CLOCK_IN);
		else
			err = snd_soc_dai_set_sysclk(codec_dai, 0, 12288000,
						    SND_SOC_CLOCK_IN);
	}

	return err;
}

static struct snd_soc_ops wmt_soc_ops = {
	.startup = wmt_soc_startup,
	.hw_params = wmt_soc_hw_params,
	.shutdown = wmt_soc_shutdown,
};

static int wmt_soc_dai_init(struct snd_soc_codec *codec)
{
	DBG_DETAIL();
	
	return 0;
}

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link wmt_dai = {
	.name = "WMT_SOC_AUDIO",
	.stream_name = "WMT_SOC_AUDIO",
	.init = wmt_soc_dai_init,
	.ops = &wmt_soc_ops,
};

static int wmt_suspend_post(struct platform_device *pdev, pm_message_t state)
{
	DBG_DETAIL();

	if (!strcmp(wmt_dai_name, "i2s")) {
		/* Disable I2S clock */
		PMCEL_VAL &= ~BIT6;
	}

	return 0;
}

static int wmt_resume_pre(struct platform_device *pdev)
{
	int temp;
	
	DBG_DETAIL();
	
	if (!strcmp(wmt_dai_name, "i2s")) {
		/*
		 *  Enable MCLK before codec enable,
		 *  otherwise the codec will be disabled.
		 */
		/* BIT0:enable I2S MCLK, BIT4:select to I2S, BIT9,BIT10:select to SPDIF,
			BIT16,BIT17:select to sus_gpio0 and sus_gpio1 */
		GPIO_PIN_SHARING_SEL_4BYTE_VAL |= (BIT0);
		GPIO_PIN_SHARING_SEL_4BYTE_VAL &= ~(BIT4);

		GPIO_PIN_SHARING_SEL_4BYTE_VAL &= ~(BIT16 | BIT17);

		/* disable GPIO and Pull Down mode for I2S and SPDIF */
		GPIO_CTRL_GP10_I2S_BYTE_VAL &= ~(0xFF);
		GPIO_PULL_EN_GP10_I2S_BYTE_VAL &= ~(0xFF);

		/* set clock to 11.289MHz */
		do
    	{
        	temp = PMCSH_VAL;
    	}while(temp);
	
		temp = 0;
		temp |= (BIT1 | BIT5);
		temp &= ~(BIT0 | BIT2 | BIT4);
		PMAUD_VAL = temp;

		do
    	{
        	temp = PMCSH_VAL;
    	}while(temp);

		/* enable I2S clock */
		PMCEL_VAL |= BIT6;
	}

	return 0;
}

/* Audio machine driver */
static struct snd_soc_card snd_soc_machine_wmt = {
	.name = "WMT_SOC",
	.platform = &wmt_soc_platform,
	.dai_link = &wmt_dai,
	.num_links = 1,
	.suspend_post = wmt_suspend_post,
	.resume_pre = wmt_resume_pre,
};

/* Audio private data */
static struct vt1602_setup_data wmt_soc_codec_setup = {
	.i2c_bus = 0,
	.i2c_address = 0x1a,
};

/* Audio subsystem */
static struct snd_soc_device wmt_snd_devdata = {
	.card = &snd_soc_machine_wmt,
};

static struct platform_device *wmt_snd_device;

static int __init wmt_soc_init(void)
{
	int err;
	char buf[80];
	char varname[80];
	int varlen = 80;
	int i = 0;

	DBG_DETAIL();

	/* Read u-boot parameter to decide wmt_dai_name and wmt_codec_name */
	strcpy(varname, "wmt.audio.i2s");
	if (wmt_getsyspara(varname, buf, &varlen) != 0) {
		strcpy(wmt_dai_name, "null");	
		strcpy(wmt_codec_name, "null");
	}
	else {
		strcpy(wmt_dai_name, "i2s");	
	}

	if (strcmp(wmt_dai_name, "null")) {
		for (i = 0; i < 80; ++i) {
			if (buf[i] == ':')
				break;
			else
				wmt_codec_name[i] = buf[i];
		}
	}
	else {
#ifdef CONFIG_SND_WMT_SOC_I2S
		strcpy(wmt_dai_name, "i2s");	
#endif
#ifdef CONFIG_I2S_HW_DAC
		strcpy(wmt_codec_name, "hwdac");
#endif
#ifdef CONFIG_I2S_CODEC_VT1602
		strcpy(wmt_codec_name, "vt1602");
#endif
#ifdef CONFIG_I2S_CODEC_VT1603
		strcpy(wmt_codec_name, "vt1603");
#endif
#ifdef CONFIG_I2S_CODEC_WM8900
		strcpy(wmt_codec_name, "wm8900");
#endif
	}

	info("dai_name=%s, codec_name=%s", wmt_dai_name, wmt_codec_name);
	
	/* Plug-in dai and codec function depend on wmt_dai_name and wmt_codec_name */
	i = 0;
	if (!strcmp(wmt_dai_name, "i2s")) {
#ifdef CONFIG_SND_WMT_SOC_I2S		
		wmt_dai.cpu_dai = &wmt_i2s_dai;
		i++;
#endif
	}
	
	if (!i) {
		strcpy(wmt_dai_name, "null");	
	}

	i = 0;

	if (!strcmp(wmt_codec_name, "vt1602")) {
#ifdef CONFIG_I2S_CODEC_VT1602		
		wmt_dai.codec_dai = &vt1602_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_vt1602;
		wmt_snd_devdata.codec_data = &wmt_soc_codec_setup;
		i++;
#endif		
	}
	else if (!strcmp(wmt_codec_name, "wm8900")) {
#ifdef CONFIG_I2S_CODEC_WM8900
		wmt_dai.codec_dai = &wm8900_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_wm8900;
		i++;
#endif		
	}	
	else if (!strcmp(wmt_codec_name, "hwdac")) {
#ifdef CONFIG_I2S_HW_DAC		
		wmt_dai.codec_dai = &hwdac_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_hwdac;
		wmt_snd_devdata.codec_data = &wmt_soc_codec_setup;
		i++;
#endif		
	}
	else if (!strcmp(wmt_codec_name, "vt1603")) {
#ifdef CONFIG_I2S_CODEC_VT1603
		wmt_dai.codec_dai = &vt1603_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_vt1603;
		i++;
#endif
	}
	
	if (!i) {
		strcpy(wmt_codec_name, "null");
	}

	if ((!strcmp(wmt_dai_name, "null")) || (!strcmp(wmt_codec_name, "null"))) {
		info("dai or codec name not matched!");
		return 0;
	}

	/* Doing register process after plug-in */
	wmt_snd_device = platform_device_alloc("soc-audio", -1);
	if (!wmt_snd_device)
		return -ENOMEM;

	platform_set_drvdata(wmt_snd_device, &wmt_snd_devdata);
	wmt_snd_devdata.dev = &wmt_snd_device->dev;
	
	*(unsigned int *)wmt_dai.cpu_dai->private_data = 1;
	
	err = platform_device_add(wmt_snd_device);
	if (err)
		goto err1;

	return 0;
err1:
	platform_device_put(wmt_snd_device);

	return err;

}

static void __exit wmt_soc_exit(void)
{
	DBG_DETAIL();
	
	platform_device_unregister(wmt_snd_device);
}

module_init(wmt_soc_init);
module_exit(wmt_soc_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [ALSA SoC] driver");
MODULE_LICENSE("GPL");

