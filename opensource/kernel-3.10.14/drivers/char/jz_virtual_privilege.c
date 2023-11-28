#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/mm.h>
#include <asm/cacheops.h>
#include <linux/dma-mapping.h>

/*#include <asm/system.h>*/
#include <asm/uaccess.h>	/* copy_*_user */
#include <asm/processor.h>

//#define DRIVER_NAME "virtual_privilege"
#define DRIVER_NAME "tcsm"
#define CMD_VPU_CACHE 100
#define CMD_VPU_PHY   101

static int vprivilege_major = 0;
static int vprivilege_minor = 0;
static int vprivilege_nr_devs = 1;

struct vprivilege_dev {
	unsigned long phy_addr;
	struct cdev cdev;	  /* Char device structure */
};

static struct vprivilege_dev *vprivilege_devices;
static struct class *vprivilege_class;

static int jz_vprivilege_open(struct inode *inode, struct file *filp)
{
	/* struct pt_regs *info = task_pt_regs(current); */

	/* info->cp0_status &= ~0x10;// clear UM bit */

	return 0;
}
static int jz_vprivilege_release(struct inode *inode, struct file *filp)
{
	/* struct pt_regs *info = task_pt_regs(current); */

	/* info->cp0_status |= 0x10;// set UM bit */

	return 0;
}
static ssize_t jz_vprivilege_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}
static ssize_t jz_vprivilege_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

static long jz_vprivilege_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int addr, size;
	unsigned int *arg_r;

	switch(cmd) {
	case CMD_VPU_CACHE:
		arg_r = (unsigned int *)arg;

		addr = (unsigned int)arg_r[0];
		size = arg_r[1];
		dma_cache_sync(NULL, (void *)addr, size, DMA_TO_DEVICE);
		break;
	case CMD_VPU_PHY:
		arg_r = (unsigned int *)arg;
		*arg_r = vprivilege_devices->phy_addr;
		break;
	default:
		printk("cmd error!\n");
	}

	return 0;
}

static int jz_vprivilege_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long off, len, vaddr;
	unsigned int count = 32 * 1024;
	unsigned int start;

	vaddr = __get_free_pages(GFP_KERNEL, 4);
	dma_cache_sync(NULL, (void *)vaddr, count, DMA_TO_DEVICE);
	vprivilege_devices->phy_addr = virt_to_phys((void*)vaddr);

	start = vprivilege_devices->phy_addr;
	off = vma->vm_pgoff << PAGE_SHIFT;

	/* frame buffer memory */

	len = PAGE_ALIGN((vaddr & ~PAGE_MASK) + count);
	start &= PAGE_MASK;

	if ((vma->vm_end - vma->vm_start + off) > len)
		return -EINVAL;
	off += start;

	vma->vm_pgoff = off >> PAGE_SHIFT;
	vma->vm_flags |= VM_IO;
#if 0
	/* Uncacheable */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif
	pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
#if 0
	pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED; /* Uncacheable */
#endif
	/* Write-Back */
	pgprot_val(vma->vm_page_prot) |= _CACHE_CACHABLE_NONCOHERENT;
#if 0
	/* Write-Acceleration */
	pgprot_val(vma->vm_page_prot) |= _CACHE_CACHABLE_WA;
#endif
	if (io_remap_pfn_range(vma, vma->vm_start, off >> PAGE_SHIFT,
			       vma->vm_end - vma->vm_start,
			       vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

/* Driver Operation structure */
static struct file_operations vprivilege_fops = {
	.owner = THIS_MODULE,
	.write = jz_vprivilege_write,
	.read =  jz_vprivilege_read,
	.open = jz_vprivilege_open,
	.unlocked_ioctl	= jz_vprivilege_ioctl,
	.mmap	= jz_vprivilege_mmap,
	.release = jz_vprivilege_release,
};

static int __init jz_vprivilege_init(void)
{
	int result, devno;
	dev_t dev = 0;

	printk("Init %s\n", __func__);
	/*
	 * Get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	if (vprivilege_major) {
		dev = MKDEV(vprivilege_major, vprivilege_minor);
		result = register_chrdev_region(dev, vprivilege_nr_devs, DRIVER_NAME);
	} else {
		result = alloc_chrdev_region(&dev, vprivilege_minor, vprivilege_nr_devs,
					     DRIVER_NAME);
		vprivilege_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "vprivilege: can't get major %d\n", vprivilege_major);
		goto fail;
	}

	vprivilege_class = class_create(THIS_MODULE, DRIVER_NAME);

	/*
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	vprivilege_devices = kmalloc(vprivilege_nr_devs * sizeof(struct vprivilege_dev), GFP_KERNEL);
	if (!vprivilege_devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(vprivilege_devices, 0, vprivilege_nr_devs * sizeof(struct vprivilege_dev));

	devno = MKDEV(vprivilege_major, vprivilege_minor);

	cdev_init(&vprivilege_devices->cdev, &vprivilege_fops);
	vprivilege_devices->cdev.owner = THIS_MODULE;
	vprivilege_devices->cdev.ops = &vprivilege_fops;
	result = cdev_add(&vprivilege_devices->cdev, devno, 1);

	if (result) {
		printk(KERN_NOTICE "Error %d:adding vprivilege\n", result);
		goto err_free_mem;
	}

	device_create(vprivilege_class, NULL, devno, NULL, DRIVER_NAME);


	printk("register vprivilege driver OK! Major = %d\n", vprivilege_major);

	return 0; /* succeed */

err_free_mem:
	kfree(vprivilege_devices);
fail:
	return result;
}

static void __exit jz_vprivilege_exit(void)
{
	dev_t devno = MKDEV(vprivilege_major, vprivilege_minor);


	/* Get rid of our char dev entries */
	if (vprivilege_devices) {
		cdev_del(&vprivilege_devices->cdev);
		kfree(vprivilege_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, vprivilege_nr_devs);

	device_destroy(vprivilege_class, devno);
	class_destroy(vprivilege_class);

	return;
}
module_init(jz_vprivilege_init);
module_exit(jz_vprivilege_exit);

MODULE_AUTHOR("bliu<bliu@ingenic.cn>");
MODULE_DESCRIPTION("JZ virtual privilege");
MODULE_LICENSE("GPL");
