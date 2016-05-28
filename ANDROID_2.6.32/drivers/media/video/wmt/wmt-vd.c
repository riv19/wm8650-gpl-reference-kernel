/*--- wmt-vd.c -------------------------------------------------------------------
*   Copyright (C) 2006 VIA Tech. Inc.
*
* MODULE       : wmt-vd.c
* AUTHOR       : Jason Lin
* DATE         : 2008/12/08
* DESCRIPTION  : 
*-----------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Jason Lin, 2008/12/08
* First version
*
*------------------------------------------------------------------------------*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/mman.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/major.h>

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#endif

#include "wmt-vd.h"
#include "wmt-dsplib.h"

#define DRIVER_NAME "wmt-vd"

//#define DEBUG
// #define WORDY

#define INFO(fmt,args...) printk(KERN_INFO "[" DRIVER_NAME "] " fmt , ## args)
#define WARN(fmt,args...) printk(KERN_WARNING "[" DRIVER_NAME "] " fmt , ## args)
#define ERROR(fmt,args...) printk(KERN_ERR "[" DRIVER_NAME "] " fmt , ## args)

#ifdef DEBUG
#define DBG(fmt, args...) printk(KERN_DEBUG "[" DRIVER_NAME "] %s: " fmt, __FUNCTION__ , ## args)
#else
#define DBG(fmt, args...)
#undef WORDY
#endif

#ifdef WORDY
#define WDBG(fmt, args...)	DBG(fmt, args...)
#else
#define WDBG(fmt, args...)
#endif

static int videodecoder_minor = 0;
static int videodecoder_dev_nr = 1;
static struct cdev *videodecoder_cdev = NULL;

static struct videodecoder *decoders[VD_MAX] = 
	{NULL,NULL,NULL,NULL,NULL,NULL};

static struct videodecoder_info vdinfo = {NULL,0};



static DECLARE_MUTEX(vd_sem);

static int videodecoder_open(
	struct inode *inode, 
	struct file *filp
)
{
	unsigned int ret = -EINVAL;
	unsigned idx = VD_MAX;

	idx = iminor(inode);

	if (idx < VD_MAX && decoders[idx]){
		struct videodecoder *vd = decoders[idx];
		if(vd && vd->fops.open){
			down(&vd_sem);
			filp->private_data = (void *)&vdinfo;
			ret = vd->fops.open(inode,filp);
			up(&vd_sem);
		}
		//INFO("decoder %s opened.\n",vd->name);
	}

	return ret;
}

static int videodecoder_release(
	struct inode *inode, 
	struct file *filp
)
{
	unsigned int ret = -EINVAL;
	unsigned idx = VD_MAX;

	idx = iminor(inode);

	if (idx < VD_MAX && decoders[idx]){
		struct videodecoder *vd = decoders[idx];
		if(vd && vd->fops.release){
			down(&vd_sem);
			ret = vd->fops.release(inode,filp);
			up(&vd_sem);
		}
		//INFO("decoder %s closed.\n",vd->name);
	}

	return ret;
}

static int videodecoder_ioctl(
	struct inode *inode, 
	struct file *filp, 
	unsigned int cmd, 
	unsigned long arg
)
{
	unsigned int ret = -EINVAL;
	unsigned idx = VD_MAX;

	/* check type and number, if fail return ENOTTY */
	if( _IOC_TYPE(cmd) != VD_IOC_MAGIC )   return -ENOTTY;
	if( _IOC_NR(cmd) > VD_IOC_MAXNR )   return -ENOTTY;

	/* check argument area */
	if( _IOC_DIR(cmd) & _IOC_READ )
		ret = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	else if ( _IOC_DIR(cmd) & _IOC_WRITE )
		ret = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));

	if( ret ) return -EFAULT;

	idx = iminor(inode);
	if (idx < VD_MAX && decoders[idx]){
		struct videodecoder *vd = decoders[idx];
		if(vd && vd->fops.ioctl){
			down(&vd_sem);
			ret = vd->fops.ioctl(inode,filp,cmd,arg);
			up(&vd_sem);
		}
	}

	return ret;
}

static int videodecoder_mmap(
	struct file *filp, 
	struct vm_area_struct *vma
)
{
	unsigned int ret = -EINVAL;
	unsigned idx = VD_MAX;

	if(filp && filp->f_dentry && filp->f_dentry->d_inode)
		idx = iminor(filp->f_dentry->d_inode);

	if (idx < VD_MAX && decoders[idx]){
		struct videodecoder *vd = decoders[idx];
		if(vd && vd->fops.mmap){
			down(&vd_sem);
			ret = vd->fops.mmap(filp,vma);
			up(&vd_sem);
		}
	}

	return ret;
}

static ssize_t videodecoder_read(
	struct file *filp,
	char __user *buf,
	size_t count,
	loff_t *f_pos
)
{
	ssize_t ret = 0;
	unsigned idx = VD_MAX;

	if(filp && filp->f_dentry && filp->f_dentry->d_inode)
		idx = iminor(filp->f_dentry->d_inode);

	if (idx < VD_MAX && decoders[idx]){
		struct videodecoder *vd = decoders[idx];
		if(vd && vd->fops.read){
			down(&vd_sem);
			ret = vd->fops.read(filp,buf,count,f_pos);
			up(&vd_sem);
		}
	}

	return ret;
} /* videodecoder_read */

static ssize_t videodecoder_write(
	struct file *filp,
	const char __user *buf,
	size_t count,
	loff_t *f_pos
)
{
	ssize_t ret = 0;
	unsigned idx = VD_MAX;

	if(filp && filp->f_dentry && filp->f_dentry->d_inode)
		idx = iminor(filp->f_dentry->d_inode);

	if (idx < VD_MAX && decoders[idx]){
		struct videodecoder *vd = decoders[idx];
		if(vd && vd->fops.write){
			down(&vd_sem);
			ret = vd->fops.write(filp,buf,count,f_pos);
			up(&vd_sem);
		}
	}
  
	return ret;
} /* End of videodecoder_write() */

struct file_operations videodecoder_fops = {
	.owner			= THIS_MODULE,
	.open			= videodecoder_open,
	.release		= videodecoder_release,
	.read			= videodecoder_read,
	.write			= videodecoder_write,
	.ioctl			= videodecoder_ioctl,
	.mmap			= videodecoder_mmap,
};

static int videodecoder_probe(struct platform_device *pdev)
{
	int ret = 0;
	dev_t dev_no;

	dsplib_initial(&pdev->dev);
	
	dev_no = MKDEV(VD_MAJOR, videodecoder_minor);

	/* register char device */
	videodecoder_cdev = cdev_alloc();
	if(!videodecoder_cdev){
		ERROR("alloc dev error.\n");
		return -ENOMEM;
	}

	cdev_init(videodecoder_cdev,&videodecoder_fops);
	ret = cdev_add(videodecoder_cdev,dev_no,1);

	if(ret){
		ERROR("reg char dev error(%d).\n",ret);
		cdev_del(videodecoder_cdev);
		return ret;
	}

    vdinfo.prdt_virt = dma_alloc_coherent(NULL, MAX_INPUT_BUF_SIZE, 
										&vdinfo.prdt_phys, GFP_KERNEL);

	if(!vdinfo.prdt_virt){
		ERROR("allocate video input buffer error.\n");
		cdev_del(videodecoder_cdev);
		vdinfo.prdt_virt = NULL;
		vdinfo.prdt_phys = 0;
		return -ENOMEM;
	}

	INFO("prob /dev/%s major %d, minor %d, prdt %p/%x, size %d KB\n", 
		DRIVER_NAME, VD_MAJOR, videodecoder_minor,
		vdinfo.prdt_virt,vdinfo.prdt_phys,MAX_INPUT_BUF_SIZE/1024);

	return ret;
}

static int videodecoder_remove(struct platform_device *pdev)
{
	unsigned int idx = 0;

	while(idx < VD_MAX){
		if(decoders[idx] && decoders[idx]->remove){
			down(&vd_sem);
			decoders[idx]->remove();
			decoders[idx] = NULL;
			up(&vd_sem);
		}
		idx++;
	}

	if(vdinfo.prdt_virt)
	    dma_free_coherent(NULL, MAX_INPUT_BUF_SIZE, vdinfo.prdt_virt, vdinfo.prdt_phys);

	vdinfo.prdt_virt = NULL;
	vdinfo.prdt_phys = 0;

	return 0;
}

static int videodecoder_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret;
	unsigned int idx = 0;
	struct device *dev = &pdev->dev;

	while(idx < VD_MAX){
		if(decoders[idx] && decoders[idx]->suspend){
			down(&vd_sem);
			ret = decoders[idx]->suspend(state);
			up(&vd_sem);
			if(ret < 0){
				WARN("video decoder %32s suspend error. ret = %d\n",
					decoders[idx]->name,ret);
			}
		}
		idx++;
	}

	dsplib_suspend(dev, state);
	return 0;
}

static int videodecoder_resume(struct platform_device *pdev)
{
	int ret;
	unsigned int idx = 0;
	struct device *dev = &pdev->dev;

	while(idx < VD_MAX){
		if(decoders[idx] && decoders[idx]->resume){
			down(&vd_sem);
			ret = decoders[idx]->resume();
			up(&vd_sem);
			if(ret < 0){
				WARN("video decoder %32s resume error. ret = %d\n",
					decoders[idx]->name,ret);
			}
		}
		idx++;
	}

	dsplib_resume(dev);
	return 0;
}

static void videodecoder_platform_release(struct device *device)
{
	unsigned int idx = 0;

	while(idx < VD_MAX){
		if(decoders[idx] && decoders[idx]->remove){
			down(&vd_sem);
			decoders[idx]->remove();
			decoders[idx] = NULL;
			up(&vd_sem);
		}
		idx++;
	}

	if(vdinfo.prdt_virt)
	    dma_free_coherent(NULL, MAX_INPUT_BUF_SIZE, vdinfo.prdt_virt, vdinfo.prdt_phys);

	vdinfo.prdt_virt = NULL;
	vdinfo.prdt_phys = 0;

	return;
}

static struct platform_driver videodecoder_driver = {
	.driver.name			= "wmt-vd", // This name should equal to platform device name.
	.probe			= videodecoder_probe,
	.remove			= videodecoder_remove,
	.suspend		= videodecoder_suspend,
	.resume			= videodecoder_resume
};

static u64 vd_dma_mask = 0xffffffffUL;

static struct platform_device videodecoder_device = {
	.name           = "wmt-vd",
	.id             = 0,
	.dev            = 	{	
						.release =  videodecoder_platform_release,
						.dma_mask = &vd_dma_mask,	
						.coherent_dma_mask = ~0,
						},
	.num_resources  = 0,		/* ARRAY_SIZE(spi_resources), */
	.resource       = NULL,		/* spi_resources, */
};

#ifdef CONFIG_PROC_FS
static int videodecoder_read_proc(
	char *buf, 
	char **start, 
	off_t offset, 
	int len
)
{
	int size;
	unsigned int idx = 0;
    char *p = buf;

	p += sprintf(p, "***** video decoder information *****\n");

	while(idx < VD_MAX){
		if(!decoders[idx] || !decoders[idx]->get_info)
			continue;
		down(&vd_sem);
		size = decoders[idx]->get_info(p,start,offset,len);
		up(&vd_sem);
		len -= size;
		if(len <= 40)
			break;
		p += size;
		idx++;
	}

	p += sprintf(p, "**************** end ****************\n");
    return (p - buf);
}
#endif

int	videodecoder_register(struct videodecoder *decoder)
{
	int ret = 0;
	dev_t dev_no; 

	if(!decoder || decoder->id >= VD_MAX){
		WARN("register invalid video decoder\n");
		return -1;
	}

	if(decoders[decoder->id]){
		WARN("video decoder (ID=%d) exist.(E:%32s, N:%32s)\n",
			decoder->id,decoders[decoder->id]->name,decoder->name);
		return -1;
	}

	dev_no = MKDEV(VD_MAJOR, decoder->id);
	ret = register_chrdev_region(dev_no,videodecoder_dev_nr,decoder->name);
	if( ret < 0 ){
		ERROR("can't get %s device minor %d\n",decoder->name,decoder->id);
		return ret;
	}

	decoder->device = cdev_alloc();
	if(!decoder->device){
		unregister_chrdev_region(dev_no,videodecoder_dev_nr);
		ERROR("alloc dev error.\n");
		return -ENOMEM;
	}

	cdev_init(decoder->device,&videodecoder_fops);
	ret = cdev_add(decoder->device,dev_no,1);
	if(ret){
		ERROR("reg char dev error(%d).\n",ret);
		unregister_chrdev_region(dev_no,videodecoder_dev_nr);
		cdev_del(decoder->device);
		decoder->device = NULL;
		return ret;
	}

	if(decoder->setup){
		ret = decoder->setup();
	}
	else{
		DBG("%s setup function is not exist\n",decoder->name);
	}

	if(ret >= 0){
		down(&vd_sem);
		decoders[decoder->id] = decoder;
		up(&vd_sem);
		DBG("%s registered major %d minor %d\n",
			decoder->name, VD_MAJOR, decoder->id);
	}
	else{
		DBG("%s register major %d minor %d fail\n",
			decoder->name, VD_MAJOR, decoder->id);
		unregister_chrdev_region(dev_no,videodecoder_dev_nr);
		cdev_del(decoder->device);
		decoder->device = NULL;
		return ret;
	}

	return ret;
}

int	videodecoder_unregister(struct videodecoder *decoder)
{
	int ret = 0;
	dev_t dev_no; 

	if(!decoder || decoder->id >= VD_MAX || !decoder->device){
		WARN("unregister invalid video decoder\n");
		return -1;
	}

	if(decoders[decoder->id] != decoder){
		WARN("unregiseter wrong video decoder. (E:%32s, R:%32s)\n",
			decoders[decoder->id]->name,decoder->name);
		return -1;
	}

	down(&vd_sem);
	decoders[decoder->id] = NULL;
	up(&vd_sem);

	if(decoder->remove)
		ret = decoder->remove();

	dev_no = MKDEV(VD_MAJOR, decoder->id);
	unregister_chrdev_region(dev_no,videodecoder_dev_nr);
	cdev_del(decoder->device);
	decoder->device = NULL;

	return ret;
}

static int __init videodecoder_init (void)
{
	int ret;
	dev_t dev_no; 

	dev_no = MKDEV(VD_MAJOR, videodecoder_minor);
	ret = register_chrdev_region(dev_no,videodecoder_dev_nr,"wmt-vd");
	if( ret < 0 ){
		ERROR("can't get %s device major %d\n",DRIVER_NAME, VD_MAJOR);
		return ret;
	}

#ifdef CONFIG_PROC_FS
    //create_proc_info_entry("wmt-vd", 0, NULL, videodecoder_read_proc);
    struct proc_dir_entry *res=create_proc_entry("wmt-vd", 0, NULL);
    if (res) {
        res->read_proc = videodecoder_read_proc;
    }
	DBG("create video decoder proc\n");
#endif

	ret = platform_driver_register(&videodecoder_driver);
	if (!ret) {
		ret = platform_device_register(&videodecoder_device);
		if (ret)
			platform_driver_unregister(&videodecoder_driver);
	}

	INFO("WonderMedia HW decoder driver inited\n");

	return ret;
}

void __exit videodecoder_exit (void)
{
	dev_t dev_no;

	platform_driver_unregister(&videodecoder_driver);
	platform_device_unregister(&videodecoder_device);
	dev_no = MKDEV(VD_MAJOR, videodecoder_minor);
	unregister_chrdev_region(dev_no,videodecoder_dev_nr);

	INFO("WonderMedia HW decoder driver exit\n");

	return;
}

#ifdef CFG_VD_PERFORM_EN

//#define CFG_VD_USE_OSCR 

typedef struct {    
    unsigned int   initial;         /* initial flag */
    unsigned int   total_tm;        /* total time */
    unsigned int   interval_tm;     /* interval time */
    unsigned int   reset;           /* reset counter */
    unsigned int   total_cnt;       /* total counter */
    unsigned int   count;           /* interval counter */
    unsigned int   max;             /* max time */
    unsigned int   min;             /* min time */
    unsigned int   threshold;
#ifdef CFG_VD_USE_OSCR
    unsigned int   start;           /* start time */
    unsigned int   end;             /* end time */
#else
    struct timeval start;           /* start time */
    struct timeval end;             /* end time */
#endif /* #ifdef CFG_VD_USE_OSCR*/
} wmt_tm_t;
wmt_tm_t vd_tm[VD_MAX];
#endif

#ifdef CFG_VD_PERFORM_EN

/*!*************************************************************************
* wmt_vd_timer_init
* 
* API Function by Willy Chuang, 2010/07/23
*/
/*!
* \brief
*   Initial VD timer
*
* \retval  0 if success
*/ 
int wmt_vd_timer_init(int vd_id, unsigned int count, int threshold_ms)
{
    if( vd_id >= VD_MAX ) {
        ERROR("Illegal ID: %d\n", vd_id);
        return -1;
    }
    memset(&vd_tm[vd_id], 0, sizeof(wmt_tm_t));
    vd_tm[vd_id].reset     = count;
    vd_tm[vd_id].threshold = threshold_ms * 1000; /* us */
    vd_tm[vd_id].max = 0;
    vd_tm[vd_id].min = 0xFFFFFFF;
    vd_tm[vd_id].initial = 1;

    return 0;
} /* End of wmt_vd_timer_init() */

/*!*************************************************************************
* wmt_vd_timer_start
* 
* API Function by Willy Chuang, 2010/07/23
*/
/*!
* \brief
*   Start a VD timer
*
* \retval  0 if success
*/ 
int wmt_vd_timer_start(int vd_id)
{
    if( vd_id >= VD_MAX ) {
        ERROR("Illegal ID: %d\n", vd_id);
        return -1;
    }
    if( vd_tm[vd_id].initial != 1 ){
        ERROR("[%s] VD ID(%d) timer was not initialized!\n", __FUNCTION__, vd_id);
        return -1;
    }
#ifdef CFG_VD_USE_OSCR
    vd_tm[vd_id].start = wmt_read_oscr();
#else
    do_gettimeofday(&vd_tm[vd_id].start); 
#endif
    return 0;
} /* End of wmt_vd_timer_start()*/

/*!*************************************************************************
* wmt_vd_timer_stop
* 
* API Function by Willy Chuang, 2010/07/23
*/
/*!
* \brief
*   Stop a VD timer
*
* \retval  0 if success
*/ 
int wmt_vd_timer_stop(int vd_id)
{
    int this_time;

    if( vd_id >= VD_MAX ) {
        ERROR("Illegal ID: %d\n", vd_id);
        return -1;
    }
    if( vd_tm[vd_id].initial != 1 ){
        ERROR("[%s] VD ID(%d) timer was not initialized!\n", __FUNCTION__, vd_id);
        return -1;
    }
#ifdef CFG_VD_USE_OSCR
    vd_tm[vd_id].end = wmt_read_oscr();
    this_time = (vd_tm[vd_id].end - vd_tm[vd_id].start)/3;  /* us */
    if( this_time < 0 ) {
        printk("Start: %ld,  End: %ld\n", vd_tm[vd_id].start, vd_tm[vd_id].end);
    }
#else
    do_gettimeofday(&vd_tm[vd_id].end); 

    /* unit in us */
    if( vd_tm[vd_id].start.tv_sec == vd_tm[vd_id].end.tv_sec) {
        this_time = vd_tm[vd_id].end.tv_usec - vd_tm[vd_id].start.tv_usec;
    }
    else {
        this_time = (vd_tm[vd_id].end.tv_sec - vd_tm[vd_id].start.tv_sec)*1000000
                  + (vd_tm[vd_id].end.tv_usec - vd_tm[vd_id].start.tv_usec);
    }
    if( this_time < 0 ) {
        printk("Start sec: %ld, usec: %ld\n", vd_tm[vd_id].start.tv_sec, vd_tm[vd_id].start.tv_usec);
        printk("End   sec: %ld, usec: %ld\n", vd_tm[vd_id].end.tv_sec, vd_tm[vd_id].end.tv_usec);
    }
#endif
    vd_tm[vd_id].total_tm += this_time; 
    vd_tm[vd_id].interval_tm += this_time; 
    vd_tm[vd_id].total_cnt++;
    
    if( this_time >= vd_tm[vd_id].max)
        vd_tm[vd_id].max = this_time;
    if( this_time <= vd_tm[vd_id].min)
        vd_tm[vd_id].min = this_time;

    if( this_time >  vd_tm[vd_id].threshold ) {
        printk("[VD:%d] (%d) Decode time(%d) over %d (usec)\n", vd_id, 
                vd_tm[vd_id].total_cnt, this_time, vd_tm[vd_id].threshold);
    }    
    vd_tm[vd_id].count++;
    if( (vd_tm[vd_id].reset != 0 ) && (vd_tm[vd_id].count >= vd_tm[vd_id].reset) ) {
        printk("=================================================\n");
        printk("[VD:%d] Avg. time   = %d (usec)\n", vd_id, vd_tm[vd_id].interval_tm/vd_tm[vd_id].count);        
        printk("(~ %d) Decode Time Range[%d ~ %d](usec)\n", vd_tm[vd_id].total_cnt, 
                                            vd_tm[vd_id].min, vd_tm[vd_id].max);

        vd_tm[vd_id].interval_tm = 0;
        vd_tm[vd_id].count = 0;
    }
    return 0;
} /* End of wmt_vd_timer_stop()*/

/*!*************************************************************************
* wmt_vd_timer_exit
* 
* API Function by Willy Chuang, 2010/07/23
*/
/*!
* \brief
*   Release VD timer
*
* \retval  0 if success
*/ 
int wmt_vd_timer_exit(int vd_id)
{
    if( vd_id >= VD_MAX ) {
        ERROR("Illegal ID: %d\n", vd_id);
        return -1;
    }
    if( vd_tm[vd_id].initial != 1 ){
        ERROR("[%s] VD ID(%d) timer was not initialized!\n", __FUNCTION__, vd_id);
        return -1;
    }
    if( vd_tm[vd_id].total_cnt ) {
        printk("=== [VD:%d] Timer status:\n", vd_id);
        printk("Total count = %d \n", vd_tm[vd_id].total_cnt);
        printk("Total time  = %d (usec)\n", vd_tm[vd_id].total_tm);
        printk("Avg. time   = %d (usec)\n", vd_tm[vd_id].total_tm/vd_tm[vd_id].total_cnt);        
        printk("Max time    = %d (usec)\n", vd_tm[vd_id].max);
        printk("Min time    = %d (usec)\n", vd_tm[vd_id].min);
        printk("==========================================\n");
    }
    /* reset all */
    memset(&vd_tm[vd_id], 0, sizeof(wmt_tm_t));

    return 0;
} /* End of wmt_vd_timer_exit()*/

#endif /* #ifdef CFG_VD_PERFORM_EN */

EXPORT_SYMBOL(videodecoder_register);
EXPORT_SYMBOL(videodecoder_unregister);

#ifdef CFG_VD_PERFORM_EN
EXPORT_SYMBOL(wmt_vd_timer_init);
EXPORT_SYMBOL(wmt_vd_timer_start);
EXPORT_SYMBOL(wmt_vd_timer_stop);
EXPORT_SYMBOL(wmt_vd_timer_exit);
#endif

module_init(videodecoder_init);
module_exit(videodecoder_exit);
MODULE_AUTHOR("WonderMedia SW Team Jason Lin");
MODULE_DESCRIPTION("Video Codec device driver");
MODULE_LICENSE("GPL");

