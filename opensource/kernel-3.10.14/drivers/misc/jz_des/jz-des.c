/*
 * Cryptographic API.
 *
 * Support for M200 AES HW acceleration.
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
#include <crypto/des.h>

#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include "jz-des.h"

#define DEBUG
#ifdef DEBUG
#define des_debug(format, ...) {printk(format, ## __VA_ARGS__);}
#else
#define des_debug(format, ...) do{ } while(0)
#endif

//#define FPGA_TEST

struct des_operation *des_ope = NULL;
void des_bit_set(struct des_operation *des_ope, int offset, unsigned int bit)
{
	unsigned int tmp = 0;

	tmp = des_reg_read(des_ope, offset);
	tmp |= bit;
	des_reg_write(des_ope, offset, tmp);
}

void des_bit_clr(struct des_operation *des_ope, int offset, unsigned int bit)
{
	unsigned int tmp = 0;

	tmp = des_reg_read(des_ope, offset);
	tmp &= ~(bit);
	des_reg_write(des_ope, offset, tmp);
}

static int des_open(struct inode *inode, struct file *file)
{
	//struct miscdevice *dev = file->private_data;
	printk("%s: des open successful!\n", __func__);
	/*des_reg_write(des_ope, 0, des_reg_read(des_ope, 0) | 0x01);*/
	/*printk("-- 0x%08x\n", *(volatile unsigned int *)0xb3080004);*/
	/*printk("-- 0x%08x\n", *(volatile unsigned int *)0xb3430000);*/
	/*printk("---- %s ---- 0x%08x\n", __func__, des_reg_read(des_ope, 0));*/
	return 0;
}

static ssize_t des_read(struct file *file, char __user * buffer, size_t count, loff_t * ppos)
{
	printk("%s: des read successful!\n", __func__);
	return 0;
}

static ssize_t des_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	printk("%s: des write successful!\n", __func__);
	return 0;
}

static long des_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	int ret = 0;
	struct miscdevice *dev = filp->private_data;
	struct des_operation * des_ope = container_of(dev, struct des_operation, des_dev);

	switch(cmd) {
		case IOCTL_DES_EBC_CPU_EN:
			ret = en_ecb_cpu_mode(des_ope);
			break;
		case IOCTL_DES_EBC_CPU_DE:
			ret = de_ecb_cpu_mode(des_ope);
			break;
		case IOCTL_DES_CBC_CPU_EN:
			ret = en_cbc_cpu_mode(des_ope);
			break;
		case IOCTL_DES_CBC_CPU_DE:
			ret = de_cbc_cpu_mode(des_ope);
			break;
		case IOCTL_DES_EBC_DMA_EN:
			ret = enn_ecb_dma_mode(des_ope);
			break;
		case IOCTL_DES_EBC_DMA_DE:
			ret = dee_ecb_dma_mode(des_ope);
			break;
		case IOCTL_DES_CBC_DMA_EN:
			ret = enn_cbc_dma_mode(des_ope);
			break;
		case IOCTL_DES_CBC_DMA_DE:
			ret = dee_cbc_dma_mode(des_ope);
			break;
		case IOCTL_DES_EBC_CPU_EN_TDES:
			ret = en_ecb_cpu_mode_tdes(des_ope);
			break;
		case IOCTL_DES_EBC_CPU_DE_TDES:
			ret = de_ecb_cpu_mode_tdes(des_ope);
			break;
		case IOCTL_DES_CBC_CPU_EN_TDES:
			ret = en_cbc_cpu_mode_tdes(des_ope);
			break;
		case IOCTL_DES_CBC_CPU_DE_TDES:
			ret = de_cbc_cpu_mode_tdes(des_ope);
			break;
		case IOCTL_DES_EBC_DMA_EN_TDES:
			ret = en_ecb_dma_mode_tdes(des_ope);
			break;
		case IOCTL_DES_EBC_DMA_DE_TDES:
			ret = de_ecb_dma_mode_tdes(des_ope);
			break;
		case IOCTL_DES_CBC_DMA_EN_TDES:
			ret = en_cbc_dma_mode_tdes(des_ope);
			break;
		case IOCTL_DES_CBC_DMA_DE_TDES:
			ret = de_cbc_dma_mode_tdes(des_ope);
			break;

	}
	return ret;
}

const struct file_operations des_fops = {
	.owner = THIS_MODULE,
	.read = des_read,
	.write = des_write,
	.open = des_open,
	.unlocked_ioctl = des_ioctl,
	/*.release = des_release,*/
};

#if 0
static irqreturn_t des_ope_irq_handler(int irq, void *data)
{
	return 0;
}
#endif

#ifdef FPGA_TEST
/*
 *void test_reg_mode()
 *{
 *    __des_ecb_mode();
 *    while((des_reg_read(des_ope, AES_ASCR) & 0x20)) {
 *        [>des_reg_write(des_ope, AES_ASCR, (des_reg_read(des_ope, AES_ASCR) & (~0x20)));<]
 *        num++;
 *        __des_ecb_mode();
 *        printk("-- %s , num=%d\n", __func__, num);
 *    }
 *    __des_cbc_mode();
 *    while(!(des_reg_read(des_ope, AES_ASCR) & 0x20)) {
 *        [>des_reg_write(des_ope, AES_ASCR, (des_reg_read(des_ope, AES_ASCR) | (0x20)));<]
 *        num++;
 *        __des_cbc_mode();
 *        printk("-- %s , num=%d\n", __func__, num);
 *    }
 *}
 */

/*
static int control = 0;
static int jz_des_read_proc(char *page, char **start, off_t off,int count, int *eof, void *data)
{
	int len = 0;
	len = snprintf(page, count, "%d\n", control);
	return len;
}

static int jz_des_write_proc(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char dat[10]={0};
	unsigned int * des_src_v = NULL;
	unsigned int * des_dst_v = NULL;
	unsigned int des_src_p = 0;
	unsigned int des_dst_p = 0;


	copy_from_user(dat, buffer, count);

	control = simple_strtoul(dat, 0, 0);
	if (control == 1) {
		printk("You are echo [%d] to control\n", control);
		en_ecb_cpu_mode(des_ope);
	} else if (control == 2) {
		printk("You are echo [%d] to control\n", control);
		(void)en_ecb_dma_mode(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else if (control == 3) {
		printk("You are echo [%d] to control\n", control);
		en_cbc_cpu_mode(des_ope);
	} else if (control == 4) {
		printk("You are echo [%d] to control\n", control);
		(void)en_cbc_dma_mode(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else if (control == 5) {
		printk("You are echo [%d] to control\n", control);
		de_ecb_cpu_mode(des_ope);
	} else if (control == 6) {
		printk("You are echo [%d] to control\n", control);
		(void)de_ecb_dma_mode(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else if (control == 7) {
		printk("You are echo [%d] to control\n", control);
		de_cbc_cpu_mode(des_ope);
	} else if (control == 8) {
		printk("You are echo [%d] to control\n", control);
		(void)de_cbc_dma_mode(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else if (control == 11) {
		printk("You are echo [%d] to control\n", control);
		en_ecb_cpu_mode_tdes(des_ope);
	} else if (control == 22) {
		printk("You are echo [%d] to control\n", control);
		(void)en_ecb_dma_mode_tdes(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else if (control == 33) {
		printk("You are echo [%d] to control\n", control);
		en_cbc_cpu_mode_tdes(des_ope);
	} else if (control == 44) {
		printk("You are echo [%d] to control\n", control);
		(void)en_cbc_dma_mode_tdes(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else if (control == 55) {
		printk("You are echo [%d] to control\n", control);
		de_ecb_cpu_mode_tdes(des_ope);
	} else if (control == 66) {
		printk("You are echo [%d] to control\n", control);
		(void)de_ecb_dma_mode_tdes(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else if (control == 77) {
		printk("You are echo [%d] to control\n", control);
		de_cbc_cpu_mode_tdes(des_ope);
	} else if (control == 88) {
		printk("You are echo [%d] to control\n", control);
		(void)de_cbc_dma_mode_tdes(des_ope, des_src_v, des_dst_v, des_src_p, des_dst_p);
	} else {
		printk("Your echo num is not support! Please input 1~8 or 11~18.\n");
		return count;
	}

	kfree(des_src_v);
	kfree(des_dst_v);

	des_src_v = NULL;
	des_dst_v = NULL;
	des_src_p = 0;
	des_dst_p = 0;

	return count;
}
*/
#endif

static int ingenic_des_probe(struct platform_device *pdev)
{
	int ret = 0;
#ifdef FPGA_TEST
	struct proc_dir_entry *p, *res;
#endif
	des_debug("%s: probe() start\n", __func__);

	des_ope = (struct des_operation *)kzalloc(sizeof(struct des_operation), GFP_KERNEL);
	if (!des_ope) {
		dev_err(&pdev->dev, "alloc des mem_region failed!\n");
		return -ENOMEM;
	}
	sprintf(des_ope->name, "jz-des");
	des_debug("%s, des name is : %s\n",__func__, des_ope->name);

	des_ope->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!des_ope->res) {
		dev_err(&pdev->dev, "failed to get dev resources\n");
		return -EINVAL;
	}

	des_ope->res = request_mem_region(des_ope->res->start,
			des_ope->res->end - des_ope->res->start + 1,
			pdev->name);
	if (!des_ope->res) {
		dev_err(&pdev->dev, "failed to request regs memory region");
		return -EINVAL;
	}
	des_ope->iomem = ioremap(des_ope->res->start, resource_size(des_ope->res));
	if (!des_ope->iomem) {
		dev_err(&pdev->dev, "failed to remap regs memory region\n");
		return -EINVAL;
	}
	des_debug("%s, des iomem is :0x%08x\n", __func__, (unsigned int)des_ope->iomem);

#if 0
	des_ope->irq = platform_get_irq(pdev, 0);
	if (request_irq(des_ope->irq, des_ope_irq_handler, IRQF_SHARED, des_ope->name, des_ope)) {
		dev_err(&pdev->dev, "request irq failed\n");
		return -EINVAL;
	}
	des_debug("%s, des irq is : %d\n",__func__, des_ope->irq);
#endif
	/****************************************************************/

	des_ope->des_dev.minor = MISC_DYNAMIC_MINOR;
	des_ope->des_dev.fops = &des_fops;
	des_ope->des_dev.name = "jz-des";
	des_ope->dev = &pdev->dev;

	ret = misc_register(&des_ope->des_dev);
	if(ret) {
		dev_err(&pdev->dev,"request misc device failed!\n");
		return ret;
	}
	des_ope->clk = clk_get(des_ope->dev, "des");
	if (IS_ERR(des_ope->clk)) {
		ret = dev_err(&pdev->dev, "des clk get failed!\n");
		return 0;
	}

#ifndef FPGA_TEST
	clk_enable(des_ope->clk);
#endif

#ifdef FPGA_TEST
	p = proc_mkdir("jz_des_proc", 0);
	if (!p){
		printk("mkdir proc error!\n");
		return -ENODEV;
	}

	res = create_proc_entry("jz_des", 0666, p);
	if (res) {
		res->read_proc = jz_des_read_proc;
		res->write_proc = jz_des_write_proc;
	}
#endif

	/*platform_set_drvdata(pdev,&des_ope);*/

/*
 *    struct device *dev = &pdev->dev;
 *    struct ingenic_des_dev *dd;
 *    struct resource *res;
 *    int err = -ENOMEM, i, j;
 *    u32 reg;
 *
 *    dd = kzalloc(sizeof(struct ingenic_des_dev), GFP_KERNEL);
 *    if (dd == NULL) {
 *        dev_err(dev, "unable to alloc data struct.\n");
 *        goto err_data;
 *    }
 *    dd->dev = dev;
 *    platform_set_drvdata(pdev, dd);
 *
 *    spin_lock_init(&dd->lock);
 */
	des_debug("%s: probe() done\n", __func__);
	return 0;
}

static int ingenic_des_remove(struct platform_device *pdev)
{
/*
 *    struct ingenic_des_dev *dd = platform_get_drvdata(pdev);
 *    int i;
 *
 *    if (!dd)
 *        return -ENODEV;
 *
 *    iounmap(dd->io_base);
 *    clk_put(dd->iclk);
 *    kfree(dd);
 *    dd = NULL;
 *
 */
	return 0;
}

static struct platform_driver ingenic_des_driver = {
	.probe	= ingenic_des_probe,
	.remove	= ingenic_des_remove,
	.driver	= {
		.name	= "jz-des",
		.owner	= THIS_MODULE,
	},
};

static int __init ingenic_des_mod_init(void)
{
	des_debug("loading %s driver\n", "jz-des");

	return  platform_driver_register(&ingenic_des_driver);
}

static void __exit ingenic_des_mod_exit(void)
{
	platform_driver_unregister(&ingenic_des_driver);
}

module_init(ingenic_des_mod_init);
module_exit(ingenic_des_mod_exit);

MODULE_DESCRIPTION("Ingenic AES XG support.");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Matthew Guo");

