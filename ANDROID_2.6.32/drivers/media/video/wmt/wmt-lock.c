/*
 * WMT Lock driver
 * A lock for WMT H/W resource like dsp jpeg/video decoding.
 * 
 * Copyright (c) 2010 www.wondermedia.com.cn
 * 
 * Author: Aimar Ma <AimarMa@wondermedia.com.cn>
 *
 * 
 * Please note an important objective of this module is supporting multi-thread well.
 * 
 * It's SHOULD BE thread-neutral.
 * 
 * You can open fd in first thread then exit(but not close it yet), use it in another thread and
 * close it in third thread.
 * 
 * Also it is error tolerant and free the lock once the fd is closed.
 * 
 * For example:
 * Application may open the dev node in thread 1, require the lock in thread 2
 * and then crash but the lock must be freed by this driver automatically.
 * 
 * another example: thread 1 opens lock file and exit but not close fd
 * thread 2 can use this fd to require lock.* \\
 * 
 * 
 * Note 
 * 
 **/

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/smp_lock.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/major.h>



typedef enum 
{
    lock_jpeg = 0,
    lock_video,   
    lock_max,
}wmt_lock_type;


#define WMT_LOCK_NAME	    "wmt-lock"

#define WMT_LOCK_MAJOR      MB_MAJOR     // same as memblock driver
#define WMT_LOCK_MINOR      1       // 0 is /dev/mbdev, 1 is /dev/wmt-lock

#define LOCK_IOC_MAGIC                    'L'
#define IO_WMT_LOCK     _IOWR(LOCK_IOC_MAGIC, 0, unsigned long)
#define IO_WMT_UNLOCK   _IOWR(LOCK_IOC_MAGIC, 1, unsigned long)

typedef struct {
    long lock_type;      // TYPE_JPEG or TYPE_VIDEO
    long arg2;           // for IO_WMT_LOCK, the timeout value 
} wmt_lock_ioctl_arg;




#undef WMT_LOCK_DEBUG

#ifdef WMT_LOCK_DEBUG
    #define P_DEBUG(fmt,args...)	printk(KERN_INFO "["WMT_LOCK_NAME"] " fmt , ## args)    
#else
    #define P_DEBUG(fmt,args...)	((void)(0))
#endif

#define P_INFO(fmt,args...)	    printk(KERN_INFO "["WMT_LOCK_NAME"] " fmt , ## args)
#define P_WARN(fmt,args...)	    printk(KERN_WARNING "["WMT_LOCK_NAME" *W*] " fmt, ## args)
#define P_ERROR(fmt,args...)	printk(KERN_ERR "["WMT_LOCK_NAME" *E*] " fmt , ## args)


static int lock_minor = WMT_LOCK_MINOR;
static struct cdev wmt_lock_cdev;


struct lock_owner {
    pid_t  pid;
    void * private_data;
    char   comm[TASK_COMM_LEN];      /* for debugging usage, note must clone comm as current may exited*/
    const char * type;               /* text description */
};

static struct lock_owner lock_owners[lock_max] = 
{
    {
        .type = "jpeg"
    },
    
    {
        .type = "video"
    }
};


#define LOCK_FOR_WM8605      // video/jpeg share a single dsp

//  Init the semaphore
#ifdef LOCK_FOR_WM8605
    //single mutex for both jpeg and video
    DECLARE_MUTEX(lock_jpeg_video);
    struct semaphore *lock_sem[lock_max] = {&lock_jpeg_video, &lock_jpeg_video};
#else
    #error "Unsupported platform!"
    DECLARE_MUTEX(lock_sem_jpeg);
    DECLARE_MUTEX(lock_sem_video);
    struct semaphore *lock_sem[lock_max] = {&lock_sem_jpeg, &lock_sem_video};
#endif

static int wmt_lock_read_proc(char *page, char **start, off_t offset, int len, int *eof, void *data)
{
    char *p = page;

    int i;

    for (i = 0; i < lock_max; i++) {
        struct lock_owner * o = &lock_owners[i];
        if ( o->pid != 0)
            p += sprintf(p, "%s lock : occupied by [%s,%d]\n", o->type, o->comm, o->pid);
        else
            p += sprintf(p, "%s lock : Free\n", o->type);
    }
    return(p - page);
}



static int wmt_lock_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    struct lock_owner * o;
    struct semaphore * sem;
    wmt_lock_ioctl_arg * lock_arg;
    long timeout;
    
    /* check ioctl type and number, if fail return EINVAL */
    if ( _IOC_TYPE(cmd) != LOCK_IOC_MAGIC )  {
        P_WARN("ioctl unknown cmd %X, type %X by [%s,%d]\n", cmd, _IOC_TYPE(cmd), current->comm, current->pid);
        return -EINVAL;
    }
    
    if( !access_ok( VERIFY_READ, (void __user *) arg, sizeof(wmt_lock_ioctl_arg))){
        P_WARN("ioctl access_ok failed, cmd %X, type %X by [%s,%d]\n", cmd, _IOC_TYPE(cmd), current->comm, current->pid);
        return -EFAULT;
    }
    
    lock_arg = (wmt_lock_ioctl_arg *)arg;
    
    if (lock_arg->lock_type >= lock_max || lock_arg->lock_type < 0){
        P_WARN("invalid lock type %ld by [%s,%d]\n", lock_arg->lock_type, current->comm, current->pid);
        return -E2BIG;
    }

    o = &lock_owners[lock_arg->lock_type];
    sem  = lock_sem[lock_arg->lock_type];
    

    switch (cmd) {
    case IO_WMT_LOCK:
        timeout = lock_arg->arg2;
        //  check if the current thread already get the lock
        if (o->pid == current->pid ) {
            P_WARN("Recursive %s lock by [%s,%d]\n", o->type, current->comm, current->pid);
            return -EBUSY;
        }
        
        if(timeout == 0) {
            ret = down_trylock(sem);
            if (ret)
                ret = -ETIME;      // reasonable if lock holded by other
        }
        else if(timeout == -1) {
            ret = down_interruptible(sem);
            if (ret)
                P_INFO("Require %s lock error %d by [%s,%d]\n", o->type, ret, current->comm, current->pid);
        }
        else {
            //require lock with a timeout value, please beware this function can't exit when interrupt
            ret = down_timeout(sem, msecs_to_jiffies(timeout));
        }
        
        if(ret == 0) {
            o->private_data = filp->private_data;
            o->pid          = current->pid;
            memcpy(o->comm, current->comm, TASK_COMM_LEN);
        }
        break;

    case IO_WMT_UNLOCK:
        if (o->pid == current->pid ) {
            o->pid = 0;
            up(sem);
            ret = 0;
        }
        else if(o->pid){
            P_WARN("Unexpected %s unlock from [%s,%d], hold by [%s,%d] now\n", o->type, 
                   current->comm, current->pid, o->comm, o->pid);        
            ret = -EACCES;
        }
        else{
            P_WARN("Unnecessary %s unlock from [%s,%d] when lock is free.\n", o->type, current->comm, current->pid);
            ret = -EACCES;
        }
        break;

    default:
        P_WARN("ioctl unknown cmd 0x%X  by [%s,%d]\n", cmd, current->comm, current->pid);
        ret = -EINVAL;
    }
    return ret;
}

static atomic_t lock_seq_id = ATOMIC_INIT(0);
static int wmt_lock_open(struct inode *inode, struct file *filp)
{
    /* use a sequence number as the file open id */
    filp->private_data = (void*)(atomic_add_return(1, &lock_seq_id));
    P_DEBUG("open by [%s,%d], seq %d\n", current->comm, current->pid, (int)filp->private_data);
    return 0;
}


static int wmt_lock_release(struct inode *inode, struct file *filp)
{
    int i;

    for (i = 0; i < lock_max; i++) {
        struct lock_owner * o = &lock_owners[i];
        if (o->pid != 0 && filp->private_data == o->private_data) {
            P_WARN("Auto free %s lock hold by [%s,%d]\n", o->type, o->comm, o->pid);
            o->pid = 0;
            up(lock_sem[i]);
        }
    }

    P_DEBUG("release by [%s,%d]\n", current->comm, current->pid);
    return 0;
}

static const struct file_operations wmt_lock_fops = {
    .owner   = THIS_MODULE,
    .open    = wmt_lock_open,
    .ioctl   = wmt_lock_ioctl,
    .release = wmt_lock_release,
};

static int __init wmt_lock_init(void)
{
    dev_t   dev_id;
    int ret;

    dev_id = MKDEV(WMT_LOCK_MAJOR, lock_minor);
    ret = register_chrdev_region(dev_id, 1, WMT_LOCK_NAME);
    if( ret < 0 ){
        P_ERROR("can't register %s device %d:%d, ret %d\n",WMT_LOCK_NAME, WMT_LOCK_MAJOR, lock_minor, ret);
        return ret;
    }
    
    cdev_init(&wmt_lock_cdev, &wmt_lock_fops);
    ret = cdev_add(&wmt_lock_cdev, dev_id, 1);
    if(ret){
		P_ERROR("cdev add error(%d).\n",ret);
		unregister_chrdev_region(dev_id, 1);
		return ret;
	}
    
    create_proc_read_entry(WMT_LOCK_NAME, 0, NULL, wmt_lock_read_proc, NULL);
    P_INFO("init ok, major=%d, minor=%d\n", WMT_LOCK_MAJOR, lock_minor);

    return ret;
}

static void __exit wmt_lock_cleanup(void)
{
    dev_t dev_id = MKDEV(WMT_LOCK_MAJOR, lock_minor);

    cdev_del(&wmt_lock_cdev);
    unregister_chrdev_region(dev_id, 1);
    remove_proc_entry(WMT_LOCK_NAME, NULL);

    P_INFO("cleanup done\n");
}

module_init(wmt_lock_init);
module_exit(wmt_lock_cleanup);


MODULE_AUTHOR("Aimar Ma <AimarMa@wondermedia.com.cn>");
MODULE_DESCRIPTION("WMT Lock");
MODULE_LICENSE("GPL");

#if 0   /* can't do this as AP hard-coded the minor number */
module_param(lock_minor, int, 0);
MODULE_PARM_DESC(lock_minor, "Minor device number");
#endif



