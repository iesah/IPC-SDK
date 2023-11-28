/*
 * jz47xx_spi_nordev.c - simple synchronous userspace interface to jz47xx SPI NOR Flash
 *
 */
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>

#include "jz_spi_nor.h"

enum nordev_ioctl_cmd {
	NOR_GET_CHIPID = 1,
	NOR_GET_INFO,
	NOR_ERASE,
	NOR_WRITE,
	NOR_READ,
};

struct jz_nordev_args {
	/* Memver for nor read or write */
	void __user	*d_buf;
	size_t		offset;
	size_t		c_size;
	int		is_erase_for_write;

	/* Member for get nor info */
	u32		pagesize;
	u32		sectorsize;
	u32		chipsize;
};

struct jz_nordev_data {
	struct jz_nor_local	*flash;
	struct mutex		dev_lock;

	void (*efuse_id_read)(int is_chip_id, uint32_t *buf);
};

static struct jz_nordev_data *jz_nordev = NULL;
extern void jz_efuse_id_read(int is_chip_id, uint32_t *buf);

#if 0
void mem_dump(uint8_t *buf, int len)
{
	int i;

	printk("\n<0x%08x> Length: %dBytes \n", (u32)buf, len);

	printk("--------| ");
	for (i = 0; i < 16; i++)
		printk("%02d ", i);

	printk("\r\n00000000: ");

	for (i = 0; i < len; i++) {
		printk("%02x ", buf[i]);

		if ((i != 0) && ((i % 16) == 0xF) && (i + 1) < len)
			printk("\r\n%08x: ", i + 1);
	}

	printk("\r\n");
}
#endif

static long jz_nordev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct jz_nordev_data *nordev;
	struct jz_nor_local *flash;
	struct jz_nordev_args jz_args;
	u8 *data_buf = NULL;
	u32 chipid[4];
	int retval = 0;

	nordev = filp->private_data;
	flash = nordev->flash;

	if (cmd == NOR_WRITE || cmd == NOR_READ || cmd == NOR_ERASE) {
		copy_from_user(&jz_args, (void __user *)arg, sizeof(struct jz_nordev_args));
		data_buf = kzalloc(jz_args.c_size, GFP_KERNEL);
		if (!data_buf)
			return -ENOMEM;
	}

	mutex_lock(&nordev->dev_lock);

	switch (cmd) {
	case NOR_GET_CHIPID:
		dev_dbg(&flash->spi->dev, "%s, nor_get_chipid\n", __FUNCTION__);
		if (nordev->efuse_id_read) {
			nordev->efuse_id_read(1, chipid);
			if (copy_to_user((u32 __user *)arg, chipid, 4 * sizeof(chipid[0])))
				retval = -EFAULT;
		} else {
			dev_err(&flash->spi->dev, "%s, has NOT EFUSE read driver\n", __FUNCTION__);
			retval = -ENXIO;
		}
		break;

	case NOR_GET_INFO:
		dev_dbg(&flash->spi->dev, "%s, nor_get_info\n", __FUNCTION__);
		jz_args.pagesize	= flash->pagesize;
		jz_args.sectorsize	= flash->sectorsize;
		jz_args.chipsize	= flash->chipsize;
		if (copy_to_user((void __user *)arg, &jz_args, sizeof(struct jz_nordev_args)))
			retval = -EFAULT;
		break;

	case NOR_ERASE:
		dev_dbg(&flash->spi->dev, "%s, nor_erase\n", __FUNCTION__);
		retval = flash->erase(flash, jz_args.offset, jz_args.c_size);
		if (retval < 0)
			goto err_out;
		break;

	case NOR_WRITE:
		dev_dbg(&flash->spi->dev, "%s, nor_write\n", __FUNCTION__);
		if (jz_args.offset + jz_args.c_size > flash->chipsize) {
			dev_err(&flash->spi->dev, "%s, write over out of chip\n", __FUNCTION__);
			retval = -EINVAL;
			goto err_out;
		}

		if (jz_args.is_erase_for_write) {
			retval = flash->erase(flash, jz_args.offset, jz_args.c_size);
			if (retval < 0)
				goto err_out;
		}

		if (copy_from_user((void *)data_buf, jz_args.d_buf, jz_args.c_size))
			retval = -EFAULT;

		retval = flash->write(flash, jz_args.offset, data_buf, jz_args.c_size);
		if (retval < 0)
			goto err_out;
		break;

	case NOR_READ:
		dev_dbg(&flash->spi->dev, "%s, nor_read\n", __FUNCTION__);
		if (jz_args.offset + jz_args.c_size > flash->chipsize) {
			dev_err(&flash->spi->dev, "%s, read over out of chip\n", __FUNCTION__);
			retval = -EINVAL;
			goto err_out;
		}

		retval = flash->read(flash, jz_args.offset, data_buf, jz_args.c_size);
		if (retval < 0) {
			copy_to_user(jz_args.d_buf, (void *)data_buf, jz_args.c_size);
			goto err_out;
		}

		if (copy_to_user(jz_args.d_buf, (void *)data_buf, jz_args.c_size))
			retval = -EFAULT;
		break;

	default:
		dev_err(&flash->spi->dev, "%s, NOT defined command: %d\n", __FUNCTION__, cmd);
		retval = -EINVAL;
	}

err_out:
	mutex_unlock(&nordev->dev_lock);
	return retval;
}

static int jz_nordev_open(struct inode *inode, struct file *filp)
{
	try_module_get(THIS_MODULE);
	filp->private_data = jz_nordev;

	return 0;
}

static int jz_nordev_release(struct inode *inode , struct file *filp)
{
	module_put(THIS_MODULE);
	return 0;
}

static struct file_operations  jz_nordev_fops = {
	.owner		= THIS_MODULE,
	.open		= jz_nordev_open,
	.unlocked_ioctl = jz_nordev_ioctl,
	.release	= jz_nordev_release,
};

static struct miscdevice jz_nordev_dev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= "jz_nordev",
	.fops		= &jz_nordev_fops,
};

static int jz_nor_device_match(struct device *dev, void *data)
{
	struct device_driver *drv = (struct device_driver *)data;
	return drv->bus->match(dev, drv);
}

static int __init jz_nordev_init(void)
{
	struct device_driver *drv;
	struct device *dev;

	printk("=======================jz_nordev_init===================\n");
	drv = driver_find(JZNOR_DEVICE_NAME, &spi_bus_type);
	if (!drv) {
		printk(KERN_ERR "jz_nordev: Get device driver failed\n");
		return -ENODEV;
	}

	dev = driver_find_device(drv, NULL, drv, jz_nor_device_match);
	if (!dev) {
		printk(KERN_ERR "jz_nordev: Get device failed\n");
		return -ENODEV;
	}

	jz_nordev = kzalloc(sizeof(struct jz_nordev_data), GFP_KERNEL);
	if (!jz_nordev)
		return -ENOMEM;


	jz_nordev->flash = dev_get_drvdata(dev);
#ifdef CONFIG_JZ4775_EFUSE
	jz_nordev->efuse_id_read = jz_efuse_id_read;
#else
	jz_nordev->efuse_id_read = NULL;
#endif
	mutex_init(&jz_nordev->dev_lock);

	misc_register(&jz_nordev_dev);

	return 0;
}
late_initcall(jz_nordev_init);

static void __exit jz_nordev_exit(void)
{
	misc_deregister(&jz_nordev_dev);
}
module_exit(jz_nordev_exit);

MODULE_DESCRIPTION("Jz47xx SPI NOR Flash User Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:jz_nordev");
