/*++
linux/drivers/input/keyboard/wmt_kpad.c

Copyright (c) 2008  WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation,
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <linux/module.h>
#include <linux/init.h> 
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/errno.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/kpad.h>
#include <linux/suspend.h>



/* Debug macros */
#if 0
#define DPRINTK(fmt, args...) printk(KERN_ALERT "[%s]: " fmt, __func__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

#define wmt_kpad_timeout (HZ/100)*10
#define WMT_KPAD_FUNCTION_NUM  4
static unsigned int wmt_kpad_codes[WMT_KPAD_FUNCTION_NUM] = {  
		[0]	= KEY_VOLUMEUP,
		[1]	= KEY_VOLUMEDOWN,
		[2] = KEY_BACK,
		[3] = KEY_MENU,
};

#define WMT_COL0_KEY_NUM 0
#define WMT_COL1_KEY_NUM 1
#define WMT_ROW0_KEY_NUM 2
#define WMT_ROW1_KEY_NUM 3
#define WMT_ROW0_1_KEY_NUM 3

static struct timer_list   wmt_kpad_timer_col0;
static struct timer_list   wmt_kpad_timer_col1;
static struct timer_list   wmt_kpad_timer_row0;
static struct timer_list   wmt_kpad_timer_row1;

static struct input_dev *kpad_dev;


static struct wmt_kpad_s kpad = {
	.ref	= 0,
	.res	= NULL,
	.regs   = NULL,
	.irq	= 0,
	.ints   = { 0, 0, 0, 0, 0 },
};

//fan
#define COL0_pin BIT3
#define COL1_pin BIT4
#define ROW0_pin BIT0
#define ROW1_pin BIT1
#define KPDA_IRQ_MASK (BIT14 | BIT15 | BIT17 | BIT18)
#define COL0_IRQ_MASK BIT17
#define COL1_IRQ_MASK BIT18
#define ROW0_IRQ_MASK BIT14
#define ROW1_IRQ_MASK BIT15

unsigned int enable_keypad = 1;
unsigned int back_menu_timeout = 0; /*0.1s unit*/
unsigned int back_menu_timeout_counter = 0;
bool back_menu_pressed = false;

void wmt_kpad_int_ctrl(int state) {
	if (enable_keypad == 1) {
		if (state == 0) {
			/*Disable ROW0 & ROW1 IRQ*/
			GPIO_I2S_KAPD_INT_REQ_TYPE_VAL &= ~(BIT23 | BIT31);
			/*Disable COL0 & COL1*/
			GPIO_KAPD_INT_REQ_TYPE_VAL &= ~(BIT15 | BIT23);
		} else if (state == 1) {
			/*Clear IRQ status*/
			GPIO_INT_REQ_STS_VAL = KPDA_IRQ_MASK;

			/*Enable IRQ*/
			/*Ensable ROW0 & ROW1 IRQ*/
			GPIO_I2S_KAPD_INT_REQ_TYPE_VAL |= (BIT23 | BIT31);
			/*Ensable COL0 & COL1*/
			GPIO_KAPD_INT_REQ_TYPE_VAL |= (BIT15 | BIT23);
		}
	}
		
}
EXPORT_SYMBOL(wmt_kpad_int_ctrl);

static void wmt_kpad_hw_init(void)
{
	DPRINTK("Start\n");
	/*Disable IRQ*/
	/*Disable ROW0 & ROW1 IRQ*/
	GPIO_I2S_KAPD_INT_REQ_TYPE_VAL &= ~(BIT23 | BIT31);
	/*Disable COL0 & COL1*/
	GPIO_KAPD_INT_REQ_TYPE_VAL &= ~(BIT15 | BIT23);

	/*Set as GPIO pin*/
	GPIO_CTRL_GP26_KPAD_BYTE_VAL |= (ROW0_pin | ROW1_pin | COL0_pin | COL1_pin);

	/*Set as GPI pin*/
	GPIO_OC_GP26_KPAD_BYTE_VAL &= ~(ROW0_pin | ROW1_pin | COL0_pin | COL1_pin);

	/*Set PU enable*/
	GPIO_PULL_EN_GP26_KPAD_BYTE_VAL |= (ROW0_pin | ROW1_pin | COL0_pin | COL1_pin);

	/*Set PU pullup*/
	GPIO_PULL_CTRL_GP26_KPAD_BYTE_VAL |= (ROW0_pin | ROW1_pin | COL0_pin | COL1_pin);

	/*Set IRQ type falling edge*/
	/*Set ROW0*/
	GPIO_I2S_KAPD_INT_REQ_TYPE_VAL &= ~(BIT16 | BIT17 | BIT18);
	GPIO_I2S_KAPD_INT_REQ_TYPE_VAL |= BIT17;

	/*Set ROW1*/
	GPIO_I2S_KAPD_INT_REQ_TYPE_VAL &= ~(BIT24 | BIT25 | BIT26);
	GPIO_I2S_KAPD_INT_REQ_TYPE_VAL |= BIT25;

	/*Set COL0*/
	GPIO_KAPD_INT_REQ_TYPE_VAL &= ~(BIT8 | BIT9 | BIT10);
	GPIO_KAPD_INT_REQ_TYPE_VAL |= BIT9;

	/*Set COL1*/
	GPIO_KAPD_INT_REQ_TYPE_VAL &= ~(BIT16 | BIT17 | BIT18);
	GPIO_KAPD_INT_REQ_TYPE_VAL |= BIT17;

	/*wait gpio pin pull up*/
	udelay(100);
	
	/*Clear IRQ status*/
	GPIO_INT_REQ_STS_VAL = KPDA_IRQ_MASK;

	/*Enable IRQ*/
	/*Ensable ROW0 & ROW1 IRQ*/
	GPIO_I2S_KAPD_INT_REQ_TYPE_VAL |= (BIT23 | BIT31);
	/*Ensable COL0 & COL1*/
	GPIO_KAPD_INT_REQ_TYPE_VAL |= (BIT15 | BIT23);
	DPRINTK("End\n");
	
}

static inline void wmt_kpad_timeout_col0(unsigned long fcontext)
{
	DPRINTK("Start\n");
	if (GPIO_ID_GP26_KPAD_BYTE_VAL & COL0_pin) {
		input_report_key(kpad_dev, wmt_kpad_codes[WMT_COL0_KEY_NUM], 0); /*col0 key is release*/
        input_sync(kpad_dev);
		DPRINTK("WMT_COL0_KEY_NUM release key = %d\n",wmt_kpad_codes[WMT_COL0_KEY_NUM]);
	} else {
		mod_timer(&wmt_kpad_timer_col0, jiffies + wmt_kpad_timeout);
	}
	DPRINTK("End\n");
}

static inline void wmt_kpad_timeout_col1(unsigned long fcontext)
{
	DPRINTK("Start\n");
	if (GPIO_ID_GP26_KPAD_BYTE_VAL & COL1_pin) {
		input_report_key(kpad_dev, wmt_kpad_codes[WMT_COL1_KEY_NUM], 0); /*col1 key is release*/
        input_sync(kpad_dev);
		DPRINTK("WMT_COL1_KEY_NUM release key = %d\n",wmt_kpad_codes[WMT_COL1_KEY_NUM]);
	} else {
		mod_timer(&wmt_kpad_timer_col1, jiffies + wmt_kpad_timeout);
	}
	DPRINTK("End\n");
}

static inline void wmt_kpad_timeout_row0(unsigned long fcontext)
{
	DPRINTK("Start\n");
	if (back_menu_timeout == 0) {
		if (GPIO_ID_GP26_KPAD_BYTE_VAL & ROW0_pin) {
			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 0); /*row0 key is release*/
	        input_sync(kpad_dev);
			DPRINTK("WMT_ROW0_KEY_NUM release 1 key = %d\n",wmt_kpad_codes[WMT_ROW0_KEY_NUM]);
		} else {
			mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
		}
	} else {
		back_menu_timeout_counter++;
		if (GPIO_ID_GP26_KPAD_BYTE_VAL & ROW0_pin) {
			if (back_menu_timeout_counter < back_menu_timeout) {
				DPRINTK("back_menu_timeout_counter = %d back_menu_timeout = %d \n",back_menu_timeout_counter,back_menu_timeout);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 1); /*row0 key is pressed*/
	        	input_sync(kpad_dev);
				DPRINTK("WMT_ROW0_KEY_NUM press 2 key = %d\n",wmt_kpad_codes[WMT_ROW0_KEY_NUM]);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 0); /*row0 key is release*/
	        	input_sync(kpad_dev);
				back_menu_pressed = false;
				DPRINTK("WMT_ROW0_KEY_NUM release 2 key = %d\n",wmt_kpad_codes[WMT_ROW0_KEY_NUM]);
			} else {
				DPRINTK("back_menu_timeout_counter = %d back_menu_timeout = %d \n",back_menu_timeout_counter,back_menu_timeout);
				back_menu_pressed = false;
				DPRINTK("WMT_ROW0_1_KEY_NUM release\n");
			}
		} else {
			if (back_menu_timeout_counter == back_menu_timeout) {
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_1_KEY_NUM], 1); /*row1 key is pressed*/
		        input_sync(kpad_dev);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_1_KEY_NUM], 0); /*row1 key is release*/
	        	input_sync(kpad_dev);
				DPRINTK("WMT_ROW0_1_KEY_NUM report key = %d\n",wmt_kpad_codes[WMT_ROW0_1_KEY_NUM]);
			}
			mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
		}
	}
	DPRINTK("End\n");
}

static inline void wmt_kpad_timeout_row1(unsigned long fcontext)
{
	DPRINTK("Start\n");
	if (GPIO_ID_GP26_KPAD_BYTE_VAL & ROW1_pin) {
		input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW1_KEY_NUM], 0); /*row0 key is release*/
        input_sync(kpad_dev);
		DPRINTK("WMT_ROW1_KEY_NUM release key = %d\n",wmt_kpad_codes[WMT_ROW1_KEY_NUM]);
	} else {
		mod_timer(&wmt_kpad_timer_row1, jiffies + wmt_kpad_timeout);
	}
	DPRINTK("End\n");
}

static irqreturn_t
kpad_interrupt(int irq, void *dev_id)
{
	
	unsigned int status;
	DPRINTK("Start\n");
	/* Disable interrupt */
	disable_irq_nosync(kpad.irq);
	
	/*
	 * Get keypad interrupt status and clean interrput source.
	 */
	status = GPIO_INT_REQ_STS_VAL;
	DPRINTK("Status 1 = %x\n",status);
	status &= KPDA_IRQ_MASK;
	
	/*The IRQ is kpad trigger*/
	if (status) {
		DPRINTK("Status 2 = %x\n",status);
		/*Clean IRQ*/
		GPIO_INT_REQ_STS_VAL = status;

		if (status & COL0_IRQ_MASK) {
			input_report_key(kpad_dev, wmt_kpad_codes[WMT_COL0_KEY_NUM], 1); /*col0 key is pressed*/
        	input_sync(kpad_dev);
			mod_timer(&wmt_kpad_timer_col0, jiffies + wmt_kpad_timeout);
			DPRINTK("WMT_COL0_KEY_NUM press\n");
		}

		if (status & COL1_IRQ_MASK) {
			input_report_key(kpad_dev, wmt_kpad_codes[WMT_COL1_KEY_NUM], 1); /*col1 key is pressed*/
        	input_sync(kpad_dev);
			mod_timer(&wmt_kpad_timer_col1, jiffies + wmt_kpad_timeout);
			DPRINTK("WMT_COL1_KEY_NUM press\n");
		}

		if (status & ROW0_IRQ_MASK) {
			if (back_menu_timeout == 0) {
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 1); /*row0 key is pressed*/
	        	input_sync(kpad_dev);
				mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
				DPRINTK("WMT_ROW0_KEY_NUM press\n");
			} else {
				if (back_menu_pressed == false) {
					back_menu_timeout_counter = 0;
					back_menu_pressed = true;
				}
				mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
				DPRINTK("WMT_ROW0_KEY_NUM press 1\n");
			}
		}

		if (status & ROW1_IRQ_MASK) {
			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW1_KEY_NUM], 1); /*row1 key is pressed*/
        	input_sync(kpad_dev);
			mod_timer(&wmt_kpad_timer_row1, jiffies + wmt_kpad_timeout);
			DPRINTK("WMT_ROW1_KEY_NUM press\n");
		}

	}else {
		enable_irq(kpad.irq);
		return IRQ_NONE;
	}
	
	/* Enable interrupt */

	enable_irq(kpad.irq);
	DPRINTK("End\n");
	return IRQ_HANDLED;
}

static int kpad_open(struct input_dev *dev)
{
	int ret = 0;
	unsigned int i;
	DPRINTK("Start\n");
	if (kpad.ref++) {
		/* Return success, but not initialize again. */
		DPRINTK("End 1\n");
		return 0;
	}

	
	
	/*
	 * Clean all previous keypad status. (Previous bug fixed.)
	 */


	ret = request_irq(kpad.irq, kpad_interrupt, IRQF_SHARED, "keypad", dev);

	if (ret) {
		printk(KERN_ERR "%s: Can't allocate irq %d\n", __func__, IRQ_GPIO);
		kpad.ref--;
		goto kpad_open_out;
	}

	/*init timer*/
	init_timer(&wmt_kpad_timer_col0);
    wmt_kpad_timer_col0.function = wmt_kpad_timeout_col0;
    wmt_kpad_timer_col0.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_col1);
    wmt_kpad_timer_col1.function = wmt_kpad_timeout_col1;
    wmt_kpad_timer_col1.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row0);
    wmt_kpad_timer_row0.function = wmt_kpad_timeout_row0;
    wmt_kpad_timer_row0.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row1);
    wmt_kpad_timer_row1.function = wmt_kpad_timeout_row1;
    wmt_kpad_timer_row1.data = (unsigned long)dev;
	/* Register an input event device. */


	dev->name = "keypad",
	dev->phys = "keypad",

	/*
	 *  Let kpad to implement key repeat.
	 */

	set_bit(EV_KEY, dev->evbit);

	for (i = 0; i < WMT_KPAD_FUNCTION_NUM; i++)
		set_bit(wmt_kpad_codes[i], dev->keybit);


	dev->keycode = wmt_kpad_codes;
	dev->keycodesize = sizeof(unsigned int);
	dev->keycodemax = WMT_KPAD_FUNCTION_NUM;

	/*
	 * For better view of /proc/bus/input/devices
	 */
	dev->id.bustype = 0;
	dev->id.vendor  = 0;
	dev->id.product = 0;
	dev->id.version = 0;

	input_register_device(dev);
	
	wmt_kpad_hw_init();
	DPRINTK("End2\n");
kpad_open_out:
	DPRINTK("End3\n");
	return ret;
}

static void kpad_close(struct input_dev *dev)
{
	DPRINTK("Start\n");
	if (--kpad.ref) {
		DPRINTK("End1\n");
		return;
	}

	/*
	 * Free interrupt resource
	 */
	free_irq(kpad.irq, dev);

	/*Delete timer*/
	del_timer(&wmt_kpad_timer_col0);
	del_timer(&wmt_kpad_timer_col1);
	del_timer(&wmt_kpad_timer_row0);
	del_timer(&wmt_kpad_timer_row1);
	
	/*
	 * Unregister input device driver
	 */
	input_unregister_device(dev);
	DPRINTK("End2\n");
}

static int wmt_kpad_probe(struct platform_device *pdev)
{
	int ret = 0;
	DPRINTK("Start\n");
	kpad_dev = input_allocate_device();
	if (kpad_dev == NULL) {
		DPRINTK("End 1\n");
		return -1;
	}
	/*
	 * Simply check resources parameters.
	 */
	if (pdev->num_resources != 1) {
		ret = -ENODEV;
		goto kpad_probe_out;
	}

	kpad.irq = pdev->resource[0].start;
	kpad_dev->open = kpad_open,
	kpad_dev->close = kpad_close,

	kpad_open(kpad_dev);
	DPRINTK("End2\n");
kpad_probe_out:

#ifndef CONFIG_SKIP_DRIVER_MSG
	printk(KERN_INFO "WMT keypad driver initialized: %s\n",
		  (ret == 0) ? "ok" : "failed");
#endif
	DPRINTK("End3\n");
	return ret;
}

static int wmt_kpad_remove(struct platform_device *pdev)
{
	DPRINTK("Start\n");
	kpad_close(kpad_dev);

	kpad.ref = 0;
	kpad.irq = 0;
	DPRINTK("End\n");
	return 0;
}

static int wmt_kpad_suspend(struct platform_device *pdev, pm_message_t state)
{
	DPRINTK("Start\n");
	switch (state.event) {
	case PM_EVENT_SUSPEND:             //PM_EVENT_SUSPEND == 2
		/*Disable IRQ*/
		/*Disable ROW0 & ROW1 IRQ*/
		GPIO_I2S_KAPD_INT_REQ_TYPE_VAL &= ~(BIT23 | BIT31);
		/*Disable COL0 & COL1*/
		GPIO_KAPD_INT_REQ_TYPE_VAL &= ~(BIT15 | BIT23);
		break;
	case PM_EVENT_FREEZE:
	case PM_EVENT_PRETHAW:
	default:
		break;
	}
	DPRINTK("End\n");
	return 0;
}

static int wmt_kpad_resume(struct platform_device *pdev)
{
	DPRINTK("Start\n");
	wmt_kpad_hw_init();
	DPRINTK("End\n");
	return 0;
}

static struct platform_driver wmt_kpad_driver = {
	.driver.name = "wmt-kpad",
	.probe = &wmt_kpad_probe,
	.remove = &wmt_kpad_remove,
	.suspend = &wmt_kpad_suspend,
	.resume	= &wmt_kpad_resume
};

static struct resource wmt_kpad_resources[] = {
	[0] = {
		.start  = IRQ_GPIO,
		.end    = IRQ_GPIO,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device wmt_kpad_device = {
	.name			= "wmt-kpad",
	.id				= 0,
	.num_resources  = ARRAY_SIZE(wmt_kpad_resources),
	.resource		= wmt_kpad_resources,
};

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

static int __init kpad_init(void)
{
	int ret;
	int retval;
	unsigned char buf[80];
	int varlen = 80;
	char *varname = "wmt.io.keypad";
	unsigned int timeout_value;
	
	DPRINTK(KERN_ALERT "Start\n");
	/*read back&menu button integration enable >1 enable & the value is the timeout value*/
	/*read keypad enable*/
	retval = wmt_getsyspara(varname, buf, &varlen);
	DPRINTK(KERN_ALERT "retval = %x\n",retval);
	if (retval == 0) {
		sscanf(buf,"%d:%d", &enable_keypad, &timeout_value);
		printk(KERN_ALERT "wmt.io.keypad = %d:%d\n",enable_keypad,timeout_value);
		if (enable_keypad == 0)
			return -ENODEV;
		if (timeout_value >= 0) {
			back_menu_timeout = timeout_value;
		}
		DPRINTK(KERN_ALERT "back_menu_timeout = %d \n",back_menu_timeout);
	}
	
#ifdef CONFIG_CPU_FREQ
	ret = cpufreq_register_notifier(&kpad_clock_nblock, \
		CPUFREQ_TRANSITION_NOTIFIER);

	if (ret) {
		printk(KERN_ERR "Unable to register CPU frequency " \
			"change notifier (%d)\n", ret);
	}
#endif
	ret = platform_device_register(&wmt_kpad_device);
	if (ret != 0) {
		DPRINTK("End1 ret = %x\n",ret);
		return -ENODEV;
	}

	ret = platform_driver_register(&wmt_kpad_driver);
	DPRINTK("End2 ret = %x\n",ret);
	return ret;
}

static void __exit kpad_exit(void)
{
	DPRINTK("Start\n");
	platform_driver_unregister(&wmt_kpad_driver);
	platform_device_unregister(&wmt_kpad_device);
	DPRINTK("End\n");
}

module_init(kpad_init);
module_exit(kpad_exit);


MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [generic keypad] driver");
MODULE_LICENSE("GPL");

