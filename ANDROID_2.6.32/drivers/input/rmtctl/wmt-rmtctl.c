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

  
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/input.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10))
#include <linux/platform_device.h>
#endif
#include <asm/irq.h>
#include <mach/hardware.h>
#include <mach/wmt_pmc.h>
#include <linux/delay.h>

#define DRIVER_DESC   "WMT rmtctl driver"

#define RC_INT 55
/* New remote control code */
#include "wmt-rmtctl.h"

#define RMTCTL_DEBUG 1
#define RMT_CFG_INT_CNT
#define RMT_CFG_REPEAT_KEY
//#define RMT_CFG_FACTORY_NEC
//#define RMT_CFG_WAKUP_BY_ANY_KEY

#define FACTORID   0
  
/* Media AD Player: key[0x00 - 0x1f] */
static unsigned int rmtctl_keycode_0[0x20] = {
	[0] = KEY_POWER,
	[1] = KEY_RESERVED,
	[2] = KEY_RESERVED,
	[3] = KEY_MUTE,
	[4] = KEY_CLEAR,
	[5] = KEY_UP,
	[6] = KEY_ESC,
	[7] = KEY_SCREEN, /* P/N */
	[8] = KEY_LEFT,
	[9] = KEY_ENTER,
	[10] = KEY_RIGHT,
	[11] = KEY_SETUP,
	[12] = KEY_F1,
	[13] = KEY_DOWN,
	[14] = KEY_F2,
	[15] = KEY_STOP,
	[16] = KEY_1,
	[17] = KEY_2,
	[18] = KEY_3,
	[19] = KEY_TIME,
	[20] = KEY_4,
	[21] = KEY_5,
	[22] = KEY_6,
	[23] = KEY_PLAYPAUSE,
	[24] = KEY_7,
	[25] = KEY_8,
	[26] = KEY_9,
	[27] = KEY_VOLUMEUP,
	[28] = KEY_0,
	[29] = KEY_BACK,
	[30] = KEY_FORWARD,
	[31] = KEY_VOLUMEDOWN
};

/* VOS remoter: key[0x00 - 0x1f] */
static unsigned int rmtctl_keycode_1[0x5e] = {
	[0] = KEY_RESERVED, /* KARAOKE */
	[1] = KEY_RESERVED, /* FUN- */
	[2] = KEY_RESERVED, /* ANGLE */
	[3] = KEY_VOLUMEDOWN, 
	[4] = KEY_CLEAR, 
	[5] = KEY_0, 
	[6] = KEY_F1, /* DIGEST */
	[7] = KEY_ZOOM, 
	[8] = KEY_7,
	[9] = KEY_8,
	[10] = KEY_NEXT,
	[11] = KEY_VOLUMEUP, 
	[12] = KEY_4,
	[13] = KEY_5,
	[14] = KEY_POWER,
	[15] = KEY_MUTE,
	[16] = KEY_1,
	[17] = KEY_2,
	[18] = KEY_SUBTITLE,
	[19] = KEY_RESERVED, /* RETURN */
	[20] = KEY_RECORD,
	[21] = KEY_RESERVED, /* STEP */
	[22] = KEY_RESERVED, /* A-B */
	[23] = KEY_RESERVED, /* STEP B*/
	[24] = KEY_BACK,
	[25] = KEY_PLAY,
	[26] = KEY_EJECTCD, 
	[27] = KEY_RESERVED, /* FF */
	[28] = KEY_LEFT,
	[29] = KEY_DOWN,
	[30] = KEY_F2, /* Menu/PBC */
	[31] = KEY_PLAYPAUSE, /* SF */
	/* Keycode 32 - 63 are invalid. */
	[64] = KEY_AUDIO,
	[65] = KEY_SETUP,
	[66] = KEY_RESERVED, /* FUN+ */
	[67] = KEY_RESERVED, /* MARK */
	[68] = KEY_UP,
	[69] = KEY_RESERVED, /* +10 */
	[70] = KEY_RESERVED, /* INVALID */
	[71] = KEY_RESERVED, /* SURR */
	[72] = KEY_RIGHT,
	[73] = KEY_9,
	[74] = KEY_RESERVED, /* INVALID */
	[75] = KEY_RESERVED, /* VOCAL */
	[76] = KEY_TV,
	[77] = KEY_6,
	[78] = KEY_RESERVED, /* INVALID */
	[79] = KEY_PROGRAM, /* PROG */
	[80] = KEY_RESERVED, /* DISPLAY */
	[81] = KEY_3,
	[82] = KEY_RESERVED, /* INVALID */
	[83] = KEY_RESERVED, /* INVALID */
	[84] = KEY_GOTO,
	[85] = KEY_PREVIOUS, /* Prev/ASV- */
	[86] = KEY_RESERVED, /* INVALID */
	[87] = KEY_RESERVED, /* INVALID */
	[88] = KEY_RESERVED, /* Repeat */
	[89] = KEY_STOP,
	[90] = KEY_RESERVED, /* INVALID */
	[91] = KEY_RESERVED, /* INVALID */
	[92] = KEY_ENTER,
	[93] = KEY_TITLE
};

static struct input_dev *idev;
int cirisrcnt=0;
unsigned long volatile __jiffy_data prev_jiffies;
int errflag=0;
int repeatcnt=0;

static irqreturn_t rmtctl_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int status, ir_data, scancode, vendor;
	unsigned char *key;

	/* Get IR status. */
	status = REG32_VAL(IRSTS);

	/* Check 'IR received data' flag. */
	if ((status & 0x1) == 0x0) {
		printk("IR IRQ was triggered without data received. (0x%x)\n",
			status);
		return IRQ_NONE;
	}

	/* Read IR data. */
	ir_data = REG32_VAL(IRDATA(0)) ;
	//printk("orig data 0x%08x \n",ir_data);

	key = (char *) &ir_data;

	/* clear INT status*/
	REG32_VAL(IRSTS)=0x1 ;

	if (RMTCTL_DEBUG){
		printk("ir_data = 0x%08x, status = 0x%x \n", ir_data, status);
		//printk("HZ %d jiffies 0x%08x \n ",HZ,jiffies);
	}
	/* Get vendor ID. */
	vendor = (key[0] << 8) | (key[1]);

	/* Check if key is valid. Key[3] is XORed t o key[2]. */
	if (key[2] & key[3]) { 
		printk("Invalid IR key received. (0x%x, 0x%x)\n", key[2], key[3]);
		return IRQ_NONE;
	}
	
	/* Keycode mapping. */
	switch (vendor) {  
	case 0x00ff:
		scancode = rmtctl_keycode_0[key[2]];
		break;
	case 0x40bf:
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
		/* Ignore repeated or reserved keys. */
	} else {
		printk(KERN_ERR"%d ---------IR report key 0x%x\n" ,cirisrcnt++,scancode);
		input_report_key(idev, scancode, 1);
		input_report_key(idev, scancode, 0);
		input_sync(idev);
	}

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
     
}
static void rmtctl_hw_init(void)
{
	unsigned int st;

	/* Turn off CIR SW reset. */
	REG32_VAL(IRSWRST) = 1;
	REG32_VAL(IRSWRST) = 0;

	REG32_VAL(PARAMETER(0)) = 0x10a;
	REG32_VAL(PARAMETER(1)) = 0x8E;  		
	REG32_VAL(PARAMETER(2)) = 0x42;
	REG32_VAL(PARAMETER(3)) = 0x55;		
	REG32_VAL(PARAMETER(4)) = 0x9; 		
	REG32_VAL(PARAMETER(5)) = 0x13;
	REG32_VAL(PARAMETER(6)) = 0x13;

#ifdef RMT_CFG_REPEAT_KEY
	REG32_VAL(NEC_REPEAT_TIME_OUT_CTRL) = 0x1;
	REG32_VAL(NEC_REPEAT_TIME_OUT_COUNT) = 17965000;//(107.9ms * 1000000)/(1000/166.5)
	REG32_VAL(IRCTL)        =  0X100;//NEC repeat key
#else
       REG32_VAL(IRCTL)        =  0;//NEC repeat key
#endif

#ifdef RMT_CFG_FACTORY_NEC
	REG32_VAL(IRCTL) |= (0x20<<16) ; //BIT16-23->0x20,  BIT 24,25 -> 0
#else  //NEC
	REG32_VAL(IRCTL) |= (0x0<<16) |(0x1<<25);
#endif

#ifdef RMT_CFG_INT_CNT
	REG32_VAL(INT_MASK_CTRL) = 0x1;
	REG32_VAL(INT_MASK_COUNT) =50*1000000*1/3;//0x47868C0/4;//count for 1 sec 0x47868C0
#endif



	/* Set IR remoter vendor type */ 
	/* BIT[0]: IR Circuit Enable. */
	REG32_VAL(IRCTL) |= 0x1; /*  IR_EN */

	/* Read CIR status to clear IR interrupt. */
	st = REG32_VAL(IRSTS);

	
}

static int rmtctl_probe(struct platform_device *dev)
{
	int i;

	printk("rmtctl_probe\n");
	/* Keycode_1 32 - 63 are invalid. */
	for (i = 32; i < 64; i++) {
		rmtctl_keycode_1[i] = KEY_RESERVED;
	}

	/* Register an input device. */

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10))
	if ((idev = input_allocate_device()) == NULL)
		return -1;
#else
	if ((idev = kmalloc(sizeof(struct input_dev), GFP_KERNEL)) == NULL)
		return -1;
	memset(idev, 0, sizeof(struct input_dev));
	init_input_dev(idev);
#endif

	set_bit(EV_KEY, idev->evbit);

	for (i = 0; i < 0x20; i++) {
		if (rmtctl_keycode_0[i]) {
			set_bit(rmtctl_keycode_0[i], idev->keybit);
		}
	}

	for (i = 0; i < 0x5e; i++) {
		if (rmtctl_keycode_1[i]) {
			set_bit(rmtctl_keycode_1[i], idev->keybit);
		}
	}

	idev->name = "rmtctl";
	idev->phys = "rmtctl";

	if (!idev) {
		printk("Inavlid input_dev detected.\n");
		return -1;
	}
	input_register_device(idev);

	/* Register an ISR */
	i = request_irq(RC_INT, rmtctl_interrupt, IRQF_SHARED, "rmtctl", idev);

	/* Initial H/W */
	rmtctl_hw_init();

	if (RMTCTL_DEBUG)
		printk("WonderMedia rmtctl driver v0.98 initialized: ok\n");

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
	/* 
	 * Platform bus will compare the driver name 
	 * with the platform device name. 
	 */
	.driver.name = "wmt-rmtctl", 
	//.bus = &platform_bus_type,
	//.probe = rmtctl_probe,
	.remove = rmtctl_remove,
	.suspend = rmtctl_suspend,
	.resume = rmtctl_resume
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
	.num_resources = 0,     /* ARRAY_SIZE(rmtctl_resources), */
	.resource = NULL,       /* rmtctl_resources, */
};

static int __init rmtctl_init(void)
{
	int ret;

	if (RMTCTL_DEBUG)
		printk("rmtctl_init\n");


	if (platform_device_register(&rmtctl_device))//add by jay,for modules support
		return -1;
	ret = platform_driver_probe(&rmtctl_driver, rmtctl_probe);

	return ret;
}

static void __exit rmtctl_exit(void)
{
	if (RMTCTL_DEBUG)
		printk("rmtctl_exit\n");

	(void)platform_driver_unregister(&rmtctl_driver);
	(void)platform_device_unregister(&rmtctl_device);//add by jay,for modules support
}

module_init(rmtctl_init);
module_exit(rmtctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

