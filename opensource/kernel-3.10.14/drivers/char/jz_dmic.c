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


#include <asm/uaccess.h>

#include <linux/voice_wakeup_module.h>
/*
 * !!!!!!!! this is ugly , but for the moment being,
 * snd_device_t must be the same as sound/oss/jzsound/interface/xb_snd_dsp.h
 *
 * */
enum snd_device_t {
	SND_DEVICE_DEFAULT = 0,

	SND_DEVICE_CURRENT,
	SND_DEVICE_HANDSET,
	SND_DEVICE_HEADSET, //fix for mid pretest
	SND_DEVICE_SPEAKER, //fix for mid pretest
	SND_DEVICE_HEADPHONE,

	SND_DEVICE_BT,
	SND_DEVICE_BT_EC_OFF,
	SND_DEVICE_HEADSET_AND_SPEAKER,
	SND_DEVICE_TTY_FULL,
	SND_DEVICE_CARKIT,

	SND_DEVICE_FM_SPEAKER,
	SND_DEVICE_FM_HEADSET,
	SND_DEVICE_BUILDIN_MIC,
	SND_DEVICE_HEADSET_MIC,
	SND_DEVICE_HDMI = 15,   //fix for mid pretest

	SND_DEVICE_LOOP_TEST = 16,

	SND_DEVICE_CALL_START,                          //call route start mark
	SND_DEVICE_CALL_HEADPHONE = SND_DEVICE_CALL_START,
	SND_DEVICE_CALL_HEADSET,
	SND_DEVICE_CALL_HANDSET,
	SND_DEVICE_CALL_SPEAKER,
	SND_DEVICE_HEADSET_RECORD_INCALL,
	SND_DEVICE_BUILDIN_RECORD_INCALL,
	SND_DEVICE_CALL_END = SND_DEVICE_BUILDIN_RECORD_INCALL, //call route end mark

	SND_DEVICE_LINEIN_RECORD,
	SND_DEVICE_LINEIN1_RECORD,
	SND_DEVICE_LINEIN2_RECORD,
	SND_DEVICE_LINEIN3_RECORD,

	SND_DEVICE_COUNT
};

/*
 * global define, don't change.
 * */


struct dma_fifo {
	struct circ_buf xfer;
	size_t n_size;
};
struct jzdmic_dev {
	int major;
	int minor;
	int nr_devs;

	struct dma_fifo *record_fifo;
	struct timer_list record_timer;
	struct completion read_completion;

#define F_READING           1
#define F_READBLOCK         2
#define F_WAITING_WAKEUP    3
#define F_WAKEUP_ENABLED    4
#define F_OPENED            5
#define F_WAKEUP_BLOCK      6
	unsigned long flags;
	spinlock_t lock;

	struct class *class;
	struct cdev cdev;
	struct device *dev;
};



static int jzdmic_open(struct inode *inode, struct file *filp)
{
	/* open dmic by wakeup module,
	 * default par: 16KsampleRate, 1Channel.
	 * default status: dmic enabled. data path on.
	 * */
	struct cdev *cdev = inode->i_cdev;
	struct jzdmic_dev *jzdmic = container_of(cdev, struct jzdmic_dev, cdev);
	struct dma_fifo *record_fifo = jzdmic->record_fifo;
	struct circ_buf *xfer = &record_fifo->xfer;

	wakeup_module_open(NORMAL_RECORD);
	filp->private_data = jzdmic;
	record_fifo->n_size	= TCSM_DATA_BUFFER_SIZE; /* dead size, don't change */
	xfer->buf = (char *)TCSM_DATA_BUFFER_ADDR;
	xfer->head = (char *)KSEG1ADDR(wakeup_module_get_dma_address()) - xfer->buf;;
	xfer->tail = xfer->head;

	mod_timer(&jzdmic->record_timer, jiffies + msecs_to_jiffies(20));

	return 0;
}
static int jzdmic_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	struct jzdmic_dev *jzdmic = filp->private_data;
	struct dma_fifo *record_fifo = jzdmic->record_fifo;
	struct circ_buf *xfer = &record_fifo->xfer;
	int mcount = count;

	while( mcount > 0) {

		unsigned long flags;

		while(1) {
			int nread;
			spin_lock_irqsave(&jzdmic->lock, flags);
			nread = CIRC_CNT(xfer->head, xfer->tail, record_fifo->n_size);
			if(nread > CIRC_CNT_TO_END(xfer->head, xfer->tail, record_fifo->n_size)) {
				/* head to end, start to tail */
				nread = CIRC_CNT_TO_END(xfer->head, xfer->tail, record_fifo->n_size);
			}
			if(nread > mcount) {
				/* done a request */
				nread = mcount;
			}
			if(nread == 0) {
				/*nodata in fifo, then break to wait */
				spin_unlock_irqrestore(&jzdmic->lock, flags);
				break;
			}
			spin_unlock_irqrestore(&jzdmic->lock, flags);

			copy_to_user(buf, xfer->buf + xfer->tail, nread);

			spin_lock_irqsave(&jzdmic->lock, flags);
			xfer->tail += nread;
			xfer->tail %= record_fifo->n_size;
			spin_unlock_irqrestore(&jzdmic->lock, flags);

			buf	+= nread;
			mcount -= nread;
			if(mcount == 0) {
				/*read done*/
				break;
			}
		}
		if(mcount > 0) {
			/*means data not complete yet, we block here until ready*/
			set_bit(F_READBLOCK, &jzdmic->flags);
			wait_for_completion(&jzdmic->read_completion);
			clear_bit(F_READBLOCK, &jzdmic->flags);
		}

	}

	return count - mcount;
}
static int jzdmic_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return count;
}
static int jzdmic_close(struct inode *inode, struct file *filp)
{
	struct jzdmic_dev *jzdmic = filp->private_data;
	wakeup_module_close(NORMAL_RECORD);
	del_timer_sync(&jzdmic->record_timer);
	return 0;
}


#define DMIC_SET_CHANNEL	0x100
#define DMIC_ENABLE			0x101
#define DMIC_DISABLE		0x102
#define DMIC_SET_SAMPLERATE	0x103
#define DMIC_GET_ROUTE		0x104
#define DMIC_SET_ROUTE		0x105
#define DMIC_SET_DEVICE		0x106



int support_device[] = {
	SND_DEVICE_BUILDIN_MIC
};


static int dmic_set_device(int device)
{
	int ret = 0;
	/* check if dmic support such device */
	int i;
	for(i = 0; i < sizeof(support_device)/sizeof(int); i++) {
		if(device == support_device[i])
			break;
	}
	if(i >= sizeof(support_device)/sizeof(int)) {
		ret = -EFAULT;
	} else {
		ret = 0;
	}

	return ret;
}

static long jzdmic_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	struct jzdmic_dev *jzdmic = filp->private_data;
	void __user *argp = (void __user *)args;
	int ret;
	int device;
	unsigned long samplerate;
	switch(cmd) {
		case DMIC_SET_CHANNEL:
			printk("not support , DMIC_SET_CHANNEL\n");
			break;
		case DMIC_SET_SAMPLERATE:
			copy_from_user(&samplerate, argp, sizeof(long));
			ret = wakeup_module_ioctl(DMIC_IOCTL_SET_SAMPLERATE, samplerate);
			break;

		case DMIC_GET_ROUTE:
			break;
		case DMIC_SET_ROUTE:
			printk("dmic SET route:\n");
			break;
		case DMIC_SET_DEVICE:
			printk("dmic set device:\n");
			copy_from_user(&device, argp, sizeof(int));
			ret = dmic_set_device(device);
			break;
		case DMIC_ENABLE:
			printk("dmic enable cmd\n");
			break;
		case DMIC_DISABLE:
			printk("dmic disable cmd\n");
			break;
		default:
			break;
	}

	return ret;
}


static struct file_operations jzdmic_ops = {
	.owner = THIS_MODULE,
	.write = jzdmic_write,
	.read = jzdmic_read,
	.open = jzdmic_open,
	.release = jzdmic_close,
	.unlocked_ioctl = jzdmic_ioctl,
};


static void record_timer_handler(unsigned long data)
{
	struct jzdmic_dev *jzdmic = (struct jzdmic_dev *)data;

	unsigned long flags;

	struct dma_fifo *record_fifo = jzdmic->record_fifo;
	struct circ_buf *xfer = &record_fifo->xfer;
	dma_addr_t trans_addr = wakeup_module_get_dma_address();

	//printk("trans_addr:%08x", trans_addr);
	spin_lock_irqsave(&jzdmic->lock, flags);
	/*
	 * we can't controll dma transfer,
	 * so we just change the fifo info according to trans_addr.
	 *
	 * */
	xfer->head = (char *)KSEG1ADDR(trans_addr) - xfer->buf;
	spin_unlock_irqrestore(&jzdmic->lock, flags);

	if(test_bit(F_READBLOCK, &jzdmic->flags)) {
		complete(&jzdmic->read_completion);
	}
	//printk("record_timer:xfer->head:%08x\n", xfer->head);
	mod_timer(&jzdmic->record_timer, jiffies + msecs_to_jiffies(20));


}

static int __init jzdmic_init(void)
{
	dev_t dev = 0;
	int ret, dev_no;
	struct jzdmic_dev *jzdmic;

	jzdmic = kmalloc(sizeof(struct jzdmic_dev), GFP_KERNEL);
	if(!jzdmic) {
		printk("voice dev alloc failed\n");
		goto __err_jzdmic;
	}
	jzdmic->class = class_create(THIS_MODULE, "jz-dmic");
	jzdmic->minor = 0;
	jzdmic->nr_devs = 1;
	ret = alloc_chrdev_region(&dev, jzdmic->minor, jzdmic->nr_devs, "jz-dmic");
	if(ret) {
		printk("alloc chrdev failed\n");
		goto __err_chrdev;
	}
	jzdmic->major = MAJOR(dev);

	dev_no = MKDEV(jzdmic->major, jzdmic->minor);
	cdev_init(&jzdmic->cdev, &jzdmic_ops);
	jzdmic->cdev.owner = THIS_MODULE;
	cdev_add(&jzdmic->cdev, dev_no, 1);

	init_timer(&jzdmic->record_timer);
	jzdmic->record_timer.function = record_timer_handler;
	jzdmic->record_timer.data	= (unsigned long)jzdmic;

	init_completion(&jzdmic->read_completion);

	spin_lock_init(&jzdmic->lock);
	jzdmic->record_fifo = kmalloc(sizeof(struct dma_fifo), GFP_KERNEL);
	if(!jzdmic->record_fifo) {
		printk("failed to alloc mem for record!!\n");
		goto __err_record;
	}

	jzdmic->dev = device_create(jzdmic->class, NULL, dev_no, NULL, "jz-dmic");


	return 0;

__err_record:
__err_chrdev:
	kfree(jzdmic);
__err_jzdmic:

	return -EFAULT;
}
static void __exit jzdmic_exit(void)
{

}

module_init(jzdmic_init);
module_exit(jzdmic_exit);


MODULE_AUTHOR("qipengzhen<aric.pzqi@ingenic.com>");
MODULE_DESCRIPTION("jzdmic driver for record and wakeup ops");
MODULE_LICENSE("GPL");
