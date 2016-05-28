/*++ 
 * linux/drivers/input/rmtctl/wmt-rmtctl.c
 * WonderMedia input remote control driver
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

#include <linux/delay.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <mach/wmt_pmc.h>
#include "wmt-rmtctl.h"

#define DRIVER_DESC         "WMT rmtctl driver"
#define RC_INT              55
#define RMT_CFG_INT_CNT
//#define RMT_CFG_WAKUP_BY_ANY_KEY
#define RMTCTL_DEBUG 1

#define DEBUG
#ifdef  DEBUG
static int dbg_mask = 1;
module_param(dbg_mask, int, S_IRUGO | S_IWUSR);
#define ir_dbg(fmt, args...) \
        do {\
            if (dbg_mask) \
                printk(KERN_ERR "[%s]_%d: " fmt, __func__ , __LINE__, ## args);\
        } while(0)
#define ir_trace() \
        do {\
            if (dbg_mask) \
                printk(KERN_ERR "trace in %s %d\n", __func__, __LINE__);\
        } while(0)
#else
#define ir_dbg(fmt, args...)
#define ir_trace()
#endif

/* Sony Projector */
static const unsigned int rmtctl_keycode_0[0x20] = {
	[0] = KEY_RESERVED,  // APA
	[1] = KEY_RESERVED,  // INPUT
	[2] = KEY_RESERVED,
	[3] = KEY_POWER,
	[4] = KEY_RESERVED,  // STONE TILT    
	[5] = KEY_RESERVED,
	[6] = KEY_UP,
	[7] = KEY_RESERVED,
	[8] = KEY_RESERVED,  // LENS
	[9] = KEY_LEFT ,
	[10] = KEY_ENTER,
	[11] = KEY_RIGHT,
	[12] = KEY_RESERVED, // AUTO FOUCS
	[13] = KEY_RESERVED, // RESET
	[14] = KEY_DOWN,
	[15] = KEY_MENU,
	[16] = KEY_RESERVED, //FREEZE
	[17] = KEY_ZOOMIN,
	[18] = KEY_RESERVED,
	[19] = KEY_VOLUMEUP,
	[20] = KEY_RESERVED, // PIC MUTING
	[21] = KEY_ZOOMOUT,
	[22] = KEY_MUTE,
	[23] = KEY_VOLUMEDOWN,
};

static struct input_dev *idev;

static irqreturn_t rmtctl_interrupt(int irq, void *dev_id)
{
	unsigned char *key;
	unsigned int status, ir_data;

	/* Get IR status. */
	status = REG32_VAL(IRSTS);
	/* Check 'IR received data' flag. */
	if ((status & 0x1) == 0x0) {
		printk("IR IRQ was triggered without data received. (0x%x)\n",
			status);
		return IRQ_NONE;
	}
	/* Read IR data. */
	ir_data  = REG32_VAL(IRDATA(0)) ;
	key = (char *)&ir_data;
	/* clear INT status*/
	REG32_VAL(IRSTS)=0x1 ;

	if (RMTCTL_DEBUG)
		printk("ir_data = 0x%08x, status = 0x%x \n", ir_data, status);

#if 0
	/* Get vendor ID. */
	vendor = (key[0] << 8) | (key[1]);
    printk(KERN_ERR "vendor is 0x%08x\n", vendor);

	/* Check if key is valid. Key[3] is XORed t o key[2]. */
	if (key[2] & key[3]) { 
		printk("Invalid IR key received. (0x%x, 0x%x)\n", key[2], key[3]);
		return IRQ_NONE;
	}
	
	/* Keycode mapping. */
	switch (vendor) {  
	case 0x00ff:
        printk(KERN_ERR "get keycode 0, key_num %d\n", key[2]);
		scancode = rmtctl_keycode_0[key[2]];
		break;
	case 0x40bf:
        printk(KERN_ERR "get keycode 1, key_num %d\n", key[2]);
		scancode = rmtctl_keycode_1[key[2]];
		break;
	default:
		scancode = key[2];
		break;
	}
	if (RMTCTL_DEBUG)
		printk("scancode = 0x%08x \n", scancode);
	/* Check 'IR code repeat' flag. */
	if ((status & 0x2) || (scancode == KEY_RESERVED)) {
        printk(KERN_ERR "SOMETHING ERR?!\n");
		/* Ignore repeated or reserved keys. */
	} else {
		printk(KERN_ERR"%d ---------IR report key 0x%x\n" ,cirisrcnt++,scancode);
		input_report_key(idev, scancode, 1);
		input_report_key(idev, scancode, 0);
		input_sync(idev);
	}
#endif
	return IRQ_HANDLED;
}

static void rmtctl_hw_suspend(void)
{
	REG32_VAL(WAKEUP_CMD1(0))=0xf10ebf40;
	REG32_VAL(WAKEUP_CMD1(1))=0x0;
    REG32_VAL(WAKEUP_CMD1(2))=0x0;
    REG32_VAL(WAKEUP_CMD1(3))=0x0;
    REG32_VAL(WAKEUP_CMD1(4))=0x0;
    REG32_VAL(WAKEUP_CMD2(0))=0xff00ff00;
    REG32_VAL(WAKEUP_CMD2(1))=0x0;
    REG32_VAL(WAKEUP_CMD2(2))=0x0;
    REG32_VAL(WAKEUP_CMD2(3))=0x0;
    REG32_VAL(WAKEUP_CMD2(4))=0x0; 
#ifdef RMT_CFG_WAKUP_BY_ANY_KEY
    REG32_VAL(WAKEUP_CTRL) = 0x001;
#else
    REG32_VAL(WAKEUP_CTRL) = 0x101;
#endif
    return ;
}

static void rmtctl_hw_init(void)
{
	unsigned int st;

	/* Turn off CIR SW reset. */
	REG32_VAL(IRSWRST) = 1;
	REG32_VAL(IRSWRST) = 0;

	REG32_VAL(PARAMETER(0)) = 0x44;
	REG32_VAL(PARAMETER(4)) = 0x09; 		
	REG32_VAL(PARAMETER(5)) = 0x13;
	REG32_VAL(PARAMETER(6)) = 0x13;
	REG32_VAL(JVC_CONTI_CTRL) = 0x00;
	REG32_VAL(NEC_REPEAT_TIME_OUT_CTRL) = 0x00;

    /* support sony ir type */
    REG32_VAL(IRCTL)  = 0;
    REG32_VAL(IRCTL) |= (0x02 << 4);
    REG32_VAL(IRCTL) |= (0x00 << 16) | (0x01 << 25);

#ifdef RMT_CFG_INT_CNT
	REG32_VAL(INT_MASK_CTRL) = 0x1;
	REG32_VAL(INT_MASK_COUNT) =50*1000000*1/3;//0x47868C0/4;//count for 1 sec 0x47868C0
#endif

	REG32_VAL(IRCTL) |= 0x1; /*  IR_EN */

	/* Read CIR status to clear IR interrupt. */
	st = REG32_VAL(IRSTS);
}

static int rmtctl_probe(struct platform_device *dev)
{
	int i;

	if ((idev = input_allocate_device()) == NULL)
		return -1;
    
	set_bit(EV_KEY, idev->evbit);
	for (i = 0; i < (sizeof(rmtctl_keycode_0) / sizeof(int)); i++)
		set_bit(rmtctl_keycode_0[i], idev->keybit);

	idev->name = "rmtctl";
	idev->phys = "rmtctl";
	input_register_device(idev);

	/* Initial H/W */
	rmtctl_hw_init();
	/* Register an ISR */
	i = request_irq(RC_INT, rmtctl_interrupt, IRQF_SHARED, "rmtctl", idev);
	printk(KERN_ERR "WonderMedia rmtctl driver v0.98 initialized: ok\n");

	return 0;
}

static int rmtctl_remove(struct platform_device *dev)
{
	if (RMTCTL_DEBUG)
		printk("rmtctl_remove\n");

	free_irq(RC_INT, idev);
	input_unregister_device(idev);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10))
	input_free_device(idev);
#else
	kfree(idev);
#endif

	return 0;
}

#ifdef CONFIG_PM
static int rmtctl_suspend(struct platform_device *dev, u32 state, u32 level)
{
	/* Nothing to suspend? */
	rmtctl_hw_init();

	rmtctl_hw_suspend();
	printk("wmt cir suspend  \n");
	
	disable_irq(RC_INT);
	return 0;
}

static int rmtctl_resume(struct platform_device *dev, u32 level)
{
	volatile  unsigned int regval;
	int i =0 ;
	printk("wmt cir resume \n");

	/* Initial H/W */
	REG32_VAL(WAKEUP_CTRL) &=~ BIT0;

	for (i=0;i<10;i++)
	{
		regval = REG32_VAL(WAKEUP_STS) ;

		if (regval & BIT0){
			REG32_VAL(WAKEUP_STS) |= BIT4;

		}else{
			break;
		}
		msleep_interruptible(5);
	}
	
	regval = REG32_VAL(WAKEUP_STS) ;
	if (regval & BIT0)
		printk(" 1. CIR resume NG  WAKEUP_STS 0x%08x \n",regval);


	rmtctl_hw_init();
	enable_irq(RC_INT);
	return 0;
}
#else
#define rmtctl_suspend NULL
#define rmtctl_resume NULL
#endif

static struct platform_driver  rmtctl_driver = {
	.driver.name = "wmt-rmtctl", 
	.remove  = rmtctl_remove,
	.suspend = rmtctl_suspend,
	.resume  = rmtctl_resume
};

static void rmtctl_release(struct device *dev)
{
	/* Nothing to release? */
	if (RMTCTL_DEBUG)
		printk("rmtctl_release\n");
}

static u64 rmtctl_dmamask = 0xffffffff;
static struct platform_device rmtctl_device = {
	/* 
	 * Platform bus will compare the driver name 
	 * with the platform device name. 
	 */
	.name = "wmt-rmtctl",
	.id = 0,
	.dev = { 
		.release = rmtctl_release,
		.dma_mask = &rmtctl_dmamask,
	},
	.num_resources = 0,
	.resource = NULL,
};

static int __init rmtctl_init(void)
{
	int ret = 0;

	if (RMTCTL_DEBUG)
		printk(KERN_INFO "rmtctl_init\n");

	if (platform_device_register(&rmtctl_device)) {
		ret = -1;
        goto out;
    }
	ret = platform_driver_probe(&rmtctl_driver, rmtctl_probe);
out:
	return ret;
}
module_init(rmtctl_init)

static void __exit rmtctl_exit(void)
{
	if (RMTCTL_DEBUG)
		printk(KERN_INFO "rmtctl_exit\n");

	platform_driver_unregister(&rmtctl_driver);
	platform_device_unregister(&rmtctl_device);
}
module_exit(rmtctl_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc");
MODULE_DESCRIPTION("SONY Projector IR Remoter Control Driver");
MODULE_LICENSE("Dual BSD/GPL");
