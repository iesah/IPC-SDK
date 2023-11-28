#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/mm.h>
#include <asm/cacheops.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/circ_buf.h>
#include <linux/timer.h>
#include <linux/syscore_ops.h>
#include <linux/delay.h>

#include <asm/uaccess.h>

#include <linux/voice_wakeup_module.h>
#include <linux/wait.h>

/*
 * global define, don't change.
 * */

/* we assume that. the time between suspend to cpu sleep is 1 second.
 * that is 16KSamples = 32Kbytes.
 * to ensure memory allocated successfully. we allocate 4Kbytes a time.
 * */

/*
struct sleep_buffer {
	unsigned char *buffer[NR_BUFFERS];
	unsigned int nr_buffers;
	unsigned long total_len;
};
*/
struct wakeup_dev {
	int major;
	int minor;
	int nr_devs;
	unsigned char *resource_buf;

	struct timer_list wakeup_timer;

	wait_queue_head_t wakeup_wq;

	unsigned int wakeup_pending;
	unsigned int wakeup_enable;
	spinlock_t wakeup_lock;

	struct sleep_buffer sleep_buffer; /* buffer used to store data during suspend to cpu sleep */

	struct class *class;
	struct cdev cdev;
	struct device *dev;
};

static struct wakeup_dev *g_wakeup;


static int wakeup_open(struct inode *inode, struct file *filp)
{
	/* open dmic by wakeup module,
	 * default par: 16KsampleRate, 1Channel.
	 * default status: dmic enabled. data path on.
	 * */

	/* if you need to change voice resource file.
	 * you should reopen wakeup driver. and then call write ops.
	 * */
	struct cdev *cdev = inode->i_cdev;
	struct wakeup_dev *wakeup = container_of(cdev, struct wakeup_dev, cdev);

	filp->private_data = wakeup;

	wakeup->resource_buf = (unsigned char *)KSEG1ADDR(wakeup_module_get_resource_addr());


	return 0;
}
static int wakeup_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	//struct wakeup_dev *wakeup = filp->private_data;

	return 0;
}
static int wakeup_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	struct wakeup_dev *wakeup = filp->private_data;
	if(copy_from_user(wakeup->resource_buf, buf, count)) {
		printk("copy_from_user failed.\n");
		return -EFAULT;
	}
	wakeup->resource_buf += count;

	return count;
}
static int wakeup_close(struct inode *inode, struct file *filp)
{
	struct wakeup_dev *wakeup = filp->private_data;
	unsigned long flags;

	wakeup_module_close(NORMAL_WAKEUP);

	spin_lock_irqsave(&wakeup->wakeup_lock, flags);
	if(wakeup->wakeup_pending == 1) {
		wakeup->wakeup_pending = 0;
		wakeup->wakeup_enable = 0;
		wake_up(&wakeup->wakeup_wq);
	}
	spin_unlock_irqrestore(&wakeup->wakeup_lock, flags);

	wakeup_module_wakeup_enable(0);
	del_timer_sync(&wakeup->wakeup_timer);

	return 0;
}


static long wakeup_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	//struct wakeup_dev *wakeup = filp->private_data;
	//void __user *argp = (void __user *)args;

	return 0;
}


static struct file_operations wakeup_ops = {
	.owner = THIS_MODULE,
	.write = wakeup_write,
	.read = wakeup_read,
	.open = wakeup_open,
	.release = wakeup_close,
	.unlocked_ioctl = wakeup_ioctl,
};


static void wakeup_timer_handler(unsigned long data)
{
	struct wakeup_dev *wakeup = (struct wakeup_dev *)data;
	unsigned long flags;

	if(wakeup_module_process_data() == SYS_WAKEUP_OK) {
		printk("sys wakeup ok!--%s():%d, wakeup_pending:%d\n", __func__, __LINE__, wakeup->wakeup_pending);
		spin_lock_irqsave(&wakeup->wakeup_lock, flags);
		if(wakeup->wakeup_pending == 1) {
			wake_up(&wakeup->wakeup_wq);
			wakeup->wakeup_pending = 0;
		}
		spin_unlock_irqrestore(&wakeup->wakeup_lock, flags);
	}
	mod_timer(&wakeup->wakeup_timer, jiffies + msecs_to_jiffies(30));
}

/************************* sysfs for wakeup *****************************/
static ssize_t resource_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	/* set wakeup resource */
	return count;
}
static ssize_t resource_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	return 0;
}



static ssize_t wakeup_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long flags;
	unsigned long ctl;
	int rc;

	struct wakeup_dev *wakeup = dev_get_drvdata(dev);
	/* write: 1 enable wakeup, 0: disable wakeup */

	rc = strict_strtoul(buf, 0, &ctl);
	if(rc)
		return rc;
	if(ctl == 1) {
		printk("enable wakeup function\n");
		wakeup_module_open(NORMAL_WAKEUP);
		spin_lock_irqsave(&wakeup->wakeup_lock, flags);
		wakeup->wakeup_pending = 1;
		wakeup->wakeup_enable = 1;
		spin_unlock_irqrestore(&wakeup->wakeup_lock, flags);
		wakeup_module_wakeup_enable(1);
		mod_timer(&wakeup->wakeup_timer, jiffies + msecs_to_jiffies(20));
	} else if (ctl == 0) {
		printk("disable wakeup function\n");
		wakeup_module_close(NORMAL_WAKEUP);

		spin_lock_irqsave(&wakeup->wakeup_lock, flags);
		if(wakeup->wakeup_pending == 1) {
			wakeup->wakeup_pending = 0;
			wakeup->wakeup_enable = 0;
			wake_up(&wakeup->wakeup_wq);
		}
		spin_unlock_irqrestore(&wakeup->wakeup_lock, flags);

		wakeup_module_wakeup_enable(0);
		del_timer_sync(&wakeup->wakeup_timer);
	}
	return count;
}
static ssize_t wakeup_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct wakeup_dev *wakeup = dev_get_drvdata(dev);
	int ret;
	char *t = buf;
	unsigned long flags;
	ret = wait_event_interruptible(wakeup->wakeup_wq, wakeup->wakeup_pending == 0);
	if(ret && wakeup->wakeup_pending == 1) {
		/* interrupted by signal , just return .*/
		printk("[Voice Wakeup] wakeup by signal.\n");
		return ret;
	} else {
		/* wakeup by event .*/
		t += sprintf(buf, "wakeup_ok");
		printk("[Voice Wakeup] wakeup by voice. \n");

		/*
		 * when called this functio. means wakeup is pending now.
		 * set wakeup_pending before another reading.
		 * */
		spin_lock_irqsave(&wakeup->wakeup_lock, flags);
		wakeup->wakeup_pending = 1;
		spin_unlock_irqrestore(&wakeup->wakeup_lock, flags);

	}
	return t-buf;
}

static DEVICE_ATTR(set_key, 0666, resource_show, resource_store);
static DEVICE_ATTR(wakeup, 0666, wakeup_show, wakeup_store);

static struct attribute *wakeup_attributes[] = {
	&dev_attr_set_key.attr,
	&dev_attr_wakeup.attr,
	NULL
};
static const struct attribute_group wakeup_attr_group = {
	.attrs = wakeup_attributes,
};


/* PM */
static int wakeup_suspend(void)
{
	/* alloc dma buffer. and build dma desc, wakeup_module_set_desc */
	struct wakeup_dev * wakeup = g_wakeup;
	struct sleep_buffer *sleep_buffer = &wakeup->sleep_buffer;
	int i;
	int ret;
	if(wakeup->wakeup_enable == 0) {
		return 0;
	}

	if(wakeup->wakeup_pending == 0) {
		/* voice identified during suspend */
		return -1;
	}
	sleep_buffer->nr_buffers = NR_BUFFERS;
	sleep_buffer->total_len	 = SLEEP_BUFFER_SIZE;
	for(i = 0; i < sleep_buffer->nr_buffers; i++) {
		sleep_buffer->buffer[i] = kmalloc(sleep_buffer->total_len/sleep_buffer->nr_buffers, GFP_KERNEL);
		if(sleep_buffer->buffer[i] == NULL) {
			printk("failed to allocate buffer for sleep!!\n");
			goto _allocate_failed;
		}
	}
	dma_cache_wback_inv(&sleep_buffer->buffer[0], sleep_buffer->total_len);
	ret = wakeup_module_set_sleep_buffer(&wakeup->sleep_buffer);

	return 0;

_allocate_failed:
	for(i = i-1; i>0; i--) {
		kfree(sleep_buffer->buffer[i]);
		sleep_buffer->buffer[i] = NULL;
	}

	return 0;
}
static void wakeup_resume(void)
{
	struct wakeup_dev * wakeup = g_wakeup;
	struct sleep_buffer *sleep_buffer = &wakeup->sleep_buffer;
	unsigned long flags;
	int i;
	/* release buffer */
	for(i = 0; i < sleep_buffer->nr_buffers; i++) {
		if(sleep_buffer->buffer[i] != NULL) {
			kfree(sleep_buffer->buffer[i]);
			sleep_buffer->buffer[i] = NULL;
		}
	}

	if(wakeup_module_is_cpu_wakeup_by_dmic()) {

		spin_lock_irqsave(&wakeup->wakeup_lock, flags);
		if(wakeup->wakeup_pending == 1) {
			wakeup->wakeup_pending = 0;
			wake_up(&wakeup->wakeup_wq);
		}
		spin_unlock_irqrestore(&wakeup->wakeup_lock, flags);
	}
}
struct syscore_ops wakeup_pm_ops = {
	.suspend = wakeup_suspend,
	.resume = wakeup_resume,
};


/************************** sysfs end  **********************************/
static int __init wakeup_init(void)
{
	dev_t dev = 0;
	int ret, dev_no;
	struct wakeup_dev *wakeup;

	wakeup = kzalloc(sizeof(struct wakeup_dev), GFP_KERNEL);
	if(!wakeup) {
		printk("voice dev alloc failed\n");
		goto __err_wakeup;
	}
	wakeup->class = class_create(THIS_MODULE, "jz-wakeup");
	wakeup->minor = 0;
	wakeup->nr_devs = 1;
	ret = alloc_chrdev_region(&dev, wakeup->minor, wakeup->nr_devs, "jz-wakeup");
	if(ret) {
		printk("alloc chrdev failed\n");
		goto __err_chrdev;
	}
	wakeup->major = MAJOR(dev);

	dev_no = MKDEV(wakeup->major, wakeup->minor);
	cdev_init(&wakeup->cdev, &wakeup_ops);
	wakeup->cdev.owner = THIS_MODULE;
	cdev_add(&wakeup->cdev, dev_no, 1);

	init_timer(&wakeup->wakeup_timer);
	wakeup->wakeup_timer.function = wakeup_timer_handler;
	wakeup->wakeup_timer.data	= (unsigned long)wakeup;



	wakeup->dev = device_create(wakeup->class, NULL, dev_no, NULL, "jz-wakeup");

	register_syscore_ops(&wakeup_pm_ops);

	ret = sysfs_create_group(&wakeup->dev->kobj, &wakeup_attr_group);
	if (ret < 0) {
		    printk("cannot create sysfs interface\n");
	}

	init_waitqueue_head(&wakeup->wakeup_wq);
	wakeup->wakeup_pending = 0;

	spin_lock_init(&wakeup->wakeup_lock);

	dev_set_drvdata(wakeup->dev, wakeup);
	g_wakeup = wakeup;

	return 0;

__err_wakeup:
__err_chrdev:
	kfree(wakeup);

	return -EFAULT;
}
static void __exit wakeup_exit(void)
{

}

module_init(wakeup_init);
module_exit(wakeup_exit);


MODULE_AUTHOR("qipengzhen<aric.pzqi@ingenic.com>");
MODULE_DESCRIPTION("wakeup driver using dmic");
MODULE_LICENSE("GPL");
