/*
 * Cryptographic API.
 *
 * Support for T15 AES HW.
 *
 * Copyright (c) 2014 Ingenic Corporation
 * Author: Elvis Wang <huan.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/io.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <crypto/scatterwalk.h>
#include <crypto/aes.h>

#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include "jz-aes.h"

/*#define DEBUG*/
#ifdef DEBUG
#define aes_debug(format, ...) {printk(format, ## __VA_ARGS__);}
#else
#define aes_debug(format, ...) do{ } while(0)
#endif

void* aesmem_v = NULL;
unsigned int aesmem_p = 0;
struct aes_para para;

struct aes_operation *aes_ope = NULL;
void aes_bit_set(struct aes_operation *aes_ope, int offset, unsigned int bit)
{
	unsigned int tmp = 0;

	tmp = aes_reg_read(aes_ope, offset);
	tmp |= bit;
	aes_reg_write(aes_ope, offset, tmp);
}

void aes_bit_clr(struct aes_operation *aes_ope, int offset, unsigned int bit)
{
	unsigned int tmp = 0;

	tmp = aes_reg_read(aes_ope, offset);
	tmp &= ~(bit);
	aes_reg_write(aes_ope, offset, tmp);
}

static int aes_open(struct inode *inode, struct file *file)
{
	//struct miscdevice *dev = file->private_data;
	aesmem_v = kmalloc(4096, GFP_KERNEL);
	if (!aesmem_v) {
		printk("aes kmalloc is error\n");
		return -ENOMEM;
	}
	//aesmem_v = (void *)((unsigned long)(aesmem_v + (PAGE_SIZE - 1)) & (~(PAGE_SIZE - 1)));
	aesmem_p = (unsigned long)virt_to_phys(aesmem_v);
	memset(aesmem_v, 0x00, 4096);
	printk("%s: aes open successful!\n", __func__);
	return 0;
}

static int aes_release(struct inode *inode, struct file *file)
{
	kfree(aesmem_v);
	return 0;
}
static ssize_t aes_read(struct file *file, char __user * buffer, size_t count, loff_t * ppos)
{
	return 0;
}

static ssize_t aes_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	return 0;
}

static int aes_start_encrypt_processing(void)
{
	if (para.enAlg == IN_UNF_CIPHER_ALG_AES) {
		switch (para.enWorkMode) {
			case IN_UNF_CIPHER_WORK_MODE_ECB:
				(void)en_ecb_dma_mode(aes_ope, para);
				break;
			case IN_UNF_CIPHER_WORK_MODE_CBC:
				(void)en_cbc_dma_mode(aes_ope, para);
				break;
			default:
				printk("Don't support this mode!\n");
				break;
		}
	} else if (para.enAlg == IN_UNF_CIPHER_ALG_DES) {
		printk("Sorry! we are not support DES now!\n");
		return -1;
	} else {
		printk("Sorry! we are not support this alg!\n");
		return -1;
	}

	return 0;
}

static int aes_start_decrypt_processing(void)
{
	if (para.enAlg == IN_UNF_CIPHER_ALG_AES) {
		switch (para.enWorkMode) {
			case IN_UNF_CIPHER_WORK_MODE_ECB:
				(void)de_ecb_dma_mode(aes_ope, para);
				break;
			case IN_UNF_CIPHER_WORK_MODE_CBC:
				(void)de_cbc_dma_mode(aes_ope, para);
				break;
			default:
				printk("Don't support this mode!\n");
				break;
		}
	} else if (para.enAlg == IN_UNF_CIPHER_ALG_DES) {
		printk("Sorry! we are not support DES now!\n");
		return -1;
	} else {
		printk("Sorry! we are not support this alg!\n");
		return -1;
	}

	return 0;
}

static long aes_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int pbuff = 0;
	void __user *argp  = (void __user *)arg;
	int ret = 0;

	switch(cmd) {
	case IOCTL_AES_GET_PBUFF:
		pbuff = aesmem_p;
		if (copy_to_user(argp, &pbuff, sizeof(unsigned int))) {
			printk("aes get pbuff copy_to_user error!!!\n");
			return -EFAULT;
		}
		break;
	case IOCTL_AES_SET_PARA:
		if (copy_from_user(&para, argp, sizeof(struct aes_para))) {
			printk("aes get para copy_from_user error!!!\n");
			return -EFAULT;
		}
#if 0
	printk("@@@\npara.aeskey[0] = 0x%08x,\npara.aeskey[1] = 0x%08x,\npara.aeskey[2] = 0x%08x,\npara.aeskey[3] = 0x%08x,\n	\
			para.aesiv[0] = 0x%08x,\npara.aesiv[1] = 0x%08x,\npara.aesiv[2] = 0x%08x,\npara.aesiv[3] = 0x%08x,\n	\
			para.enAlg = 0x%08x,\npara.enBitWidth = 0x%08x,\npara.enWorkMode = 0x%08x,\npara.enKeyLen = 0x%08x\n@@@\n",
			para.aeskey[0], para.aeskey[1], para.aeskey[2], para.aeskey[3], para.aesiv[0], para.aesiv[1], para.aesiv[2],
			para.aesiv[3], para.enAlg, para.enBitWidth, para.enWorkMode, para.enKeyLen);
	/*
	 *printk("para.src_addr_v = 0x%08x, \npara.dst_addr_v = 0x%08x, \npara.src_addr_p = 0x%08x, \npara.dst_addr_p = 0x%08x\n",
	 *        para.src_addr_v, para.dst_addr_v, para.src_addr_p, para.dst_addr_p);
	 */
#endif
		break;
	case IOCTL_AES_START_EN_PROCESSING:
		ret = aes_start_encrypt_processing();
		if (ret) {
			printk("aes encrypt error!\n");
			return -1;
		}
		break;
	case IOCTL_AES_START_DE_PROCESSING:
		ret = aes_start_decrypt_processing();
		if (ret) {
			printk("aes decrypt error!\n");
			return -1;
		}
		break;
	}
	return 0;
}

const struct file_operations aes_fops = {
	.owner = THIS_MODULE,
	.read = aes_read,
	.write = aes_write,
	.open = aes_open,
	.unlocked_ioctl = aes_ioctl,
	.release = aes_release,
};

static irqreturn_t aes_ope_irq_handler(int irq, void *data)
{
	return 0;
}

static int jz_aes_probe(struct platform_device *pdev)
{
	int ret = 0;
	aes_debug("%s: probe() start\n", __func__);

	aes_ope = (struct aes_operation *)kzalloc(sizeof(struct aes_operation), GFP_KERNEL);
	if (!aes_ope) {
		dev_err(&pdev->dev, "alloc aes mem_region failed!\n");
		return -ENOMEM;
	}
	sprintf(aes_ope->name, "jz-aes");
	aes_debug("%s, aes name is : %s\n",__func__, aes_ope->name);

	aes_ope->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!aes_ope->res) {
		dev_err(&pdev->dev, "failed to get dev resources\n");
		return -EINVAL;
	}

	aes_ope->res = request_mem_region(aes_ope->res->start,
			aes_ope->res->end - aes_ope->res->start + 1,
			pdev->name);
	if (!aes_ope->res) {
		dev_err(&pdev->dev, "failed to request regs memory region");
		return -EINVAL;
	}
	aes_ope->iomem = ioremap(aes_ope->res->start, resource_size(aes_ope->res));
	if (!aes_ope->iomem) {
		dev_err(&pdev->dev, "failed to remap regs memory region\n");
		return -EINVAL;
	}
	aes_debug("%s, aes iomem is :0x%08x\n", __func__, (unsigned int)aes_ope->iomem);

	aes_ope->irq = platform_get_irq(pdev, 0);
	if (request_irq(aes_ope->irq, aes_ope_irq_handler, IRQF_SHARED, aes_ope->name, aes_ope)) {
		dev_err(&pdev->dev, "request irq failed\n");
		return -EINVAL;
	}
	aes_debug("%s, aes irq is : %d\n",__func__, aes_ope->irq);

	/****************************************************************/

	aes_ope->aes_dev.minor = MISC_DYNAMIC_MINOR;
	aes_ope->aes_dev.fops = &aes_fops;
	aes_ope->aes_dev.name = "aes";

	ret = misc_register(&aes_ope->aes_dev);
	if(ret) {
		dev_err(&pdev->dev,"request misc device failed!\n");
		return ret;
	}
	aes_ope->clk = clk_get(aes_ope->dev, "aes");
	if (IS_ERR(aes_ope->clk)) {
		ret = dev_err(&pdev->dev, "aes clk get failed!\n");
		return 0;
	}

#ifndef FPGA_TEST
	clk_enable(aes_ope->clk);
#endif

	platform_set_drvdata(pdev,&aes_ope);

	aes_debug("%s: probe() done\n", __func__);
	return 0;
}

static int jz_aes_remove(struct platform_device *pdev)
{
	struct aes_operation *aes_ope = platform_get_drvdata(pdev);

	misc_deregister(&aes_ope->aes_dev);
	clk_enable(aes_ope->clk);
	clk_put(aes_ope->clk);
	free_irq(aes_ope->irq, aes_ope);
	iounmap(aes_ope->iomem);
	kfree(aes_ope);

	return 0;
}

static struct platform_driver jz_aes_driver = {
	.probe	= jz_aes_probe,
	.remove	= jz_aes_remove,
	.driver	= {
		.name	= "jz-aes",
		.owner	= THIS_MODULE,
	},
};

static int __init jz_aes_mod_init(void)
{
	aes_debug("loading %s driver\n", "jz-aes");

	return  platform_driver_register(&jz_aes_driver);
}

static void __exit jz_aes_mod_exit(void)
{
	platform_driver_unregister(&jz_aes_driver);
}

module_init(jz_aes_mod_init);
module_exit(jz_aes_mod_exit);

MODULE_DESCRIPTION("Ingenic AES hw support.");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Elvis Wang");

