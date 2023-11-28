/*
 * rmem.c - Reserved memory driver for libimp.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/syscalls.h>

#define RMEN_MAGIC			'r'
#define RMEN_CMD_FLUSH_CACHE		_IOWR(RMEN_MAGIC, 0, struct rmem_flush_cache_info)

static struct miscdevice	mdev;

struct rmem_flush_cache_info {
	unsigned int	addr;
	unsigned int	len;
	unsigned int	dir;
};

static int rmem_open(struct inode *inode, struct file *filp)
{
	pr_debug("[%d:%d] %s\n", current->tgid, current->pid, __func__);
	return 0;
}

static int rmem_release(struct inode *inode, struct file *filp)
{
	pr_debug("[%d:%d] %s\n", current->tgid, current->pid, __func__);
	return 0;
}

static long rmem_cmd_flush_cache(long arg)
{
	struct rmem_flush_cache_info info;
	long ret = 0;
	if (copy_from_user(&info, (void *)arg, sizeof(info))) {
		return -EFAULT;
	}

	dma_cache_sync(NULL, (void *)info.addr, info.len, info.dir);

	return ret;
}

static long rmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case RMEN_CMD_FLUSH_CACHE:
		return rmem_cmd_flush_cache(arg);
	default:
		pr_err("Unknown ioctl: 0x%.8X\n", cmd);
		return -EINVAL;
	}
}

static int rmem_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_IO;

	pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
	pgprot_val(vma->vm_page_prot) |= _CACHE_CACHABLE_NONCOHERENT;

	if (io_remap_pfn_range(vma,vma->vm_start,
			       vma->vm_pgoff,
			       vma->vm_end - vma->vm_start,
			       vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static struct file_operations rmem_misc_fops = {
	.open		= rmem_open,
	.release	= rmem_release,
	.unlocked_ioctl	= rmem_ioctl,
	.mmap		= rmem_mmap,
};

static int rmem_probe(struct platform_device *pdev)
{
	int ret;


	mdev.minor = MISC_DYNAMIC_MINOR;
	mdev.name =  "rmem";
	mdev.fops = &rmem_misc_fops;

	ret = misc_register(&mdev);
	if (ret < 0) {
		pr_err("rmem register failed\n");
		return -1;
	}

	return 0;
}

static int rmem_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver rmem_driver = {
	.probe		= rmem_probe,
	.remove		= rmem_remove,
	.driver		= {
		.name	= "rmem",
	},
};

struct platform_device rmem_device = {
	.name = "rmem",
	.id = -1,
	.resource = NULL,
	.num_resources = 0,
};

static int __init rmem_init(void)
{
	platform_device_register(&rmem_device);
	return platform_driver_register(&rmem_driver);
}

static void __exit rmem_exit(void)
{
	platform_device_unregister(&rmem_device);
	platform_driver_unregister(&rmem_driver);
}

module_init(rmem_init);
module_exit(rmem_exit);

MODULE_LICENSE("GPL v2");
