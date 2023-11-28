/**
 * drivers/mfd/jz4785-adc-aux.c
 *
 * aux1 aux2 channels voltage sample interface for Ingenic SoC
 *
 * Copyright(C)2012 Ingenic Semiconductor Co., LTD.
 * http://www.ingenic.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include <linux/err.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <linux/mfd/core.h>
#include <linux/delay.h>
//#include <linux/jz4780-adc.h>
#include <linux/vmalloc.h>/*for vfree*/
#include <linux/miscdevice.h>

#include <irq.h>

static unsigned int VREF_ADC = 1800;
#define AUXCONST   1024

#define ADC_ENABLE			0
#define ADC_DISABLE			1
#define ADC_SET_VREF		2

#ifndef BITS_H2L
#define BITS_H2L(msb, lsb)  ((0xFFFFFFFF >> (32-((msb)-(lsb)+1))) << (lsb))
#endif

struct jz_adc_aux {
	struct platform_device *pdev;

	struct resource *mem;
	void __iomem *base;

	int irq;

	const struct mfd_cell *cell;

	unsigned int voltage;

	struct completion read_completion;

	struct miscdevice mdev;
};


enum aux_ch {
	SADC_AUX0,
	SADC_AUX1,
	SADC_AUX2,
	SADC_AUX3,
	SADC_AUX4,
	SADC_AUX5,
	SADC_AUX6,
	SADC_AUX7,
};
extern int jz_adc_set_config(struct device *dev, uint32_t mask, uint32_t val);

static irqreturn_t jz_jz_adc_aux_irq_handler(int irq, void *devid)
{
	struct jz_adc_aux *jz_adc_aux = (struct jz_adc_aux *)devid;

	complete(&jz_adc_aux->read_completion);

	return IRQ_HANDLED;
}

static int jz_adc_aux_suspend(struct platform_device *pdev, pm_message_t state)
{
	//struct jz_adc_aux *adc_aux = platform_get_drvdata(pdev);
	//adc_aux->cell->disable(pdev);
	return 0;
}

static int jz_adc_aux_resume(struct platform_device *pdev)
{
	//struct jz_adc_aux *adc_aux = platform_get_drvdata(pdev);
	//adc_aux->cell->enable(pdev);
	return 0;
}

unsigned int jz_adc_aux_read_value(struct jz_adc_aux *jz_adc_aux, uint8_t config)
{
	unsigned long tmp;
	unsigned int value;

	INIT_COMPLETION(jz_adc_aux->read_completion);

	enable_irq(jz_adc_aux->irq);

	tmp = wait_for_completion_interruptible_timeout(&jz_adc_aux->read_completion, HZ);
	if (tmp > 0) {
		value = readw(jz_adc_aux->base) & 0xfff;
	} else {
		value = tmp ? tmp : -ETIMEDOUT;
	}

	if (value < 0) {
		printk("jz_adc_aux read value error!!\n");
		return -EIO;
	}

	disable_irq(jz_adc_aux->irq);

	return value;
}

int jz_adc_aux_sample_volt(enum aux_ch channels,struct jz_adc_aux *jz_adc_aux)
{
	unsigned long tmp;
	unsigned int sadc_volt = 0;

	if (!jz_adc_aux) {
		printk("jz_adc_aux is null ! return\n");
		return -EINVAL;
	}

	INIT_COMPLETION(jz_adc_aux->read_completion);

	jz_adc_aux->cell->enable(jz_adc_aux->pdev);
	enable_irq(jz_adc_aux->irq);

restart:
	tmp = wait_for_completion_interruptible_timeout(&jz_adc_aux->read_completion, HZ);
	if (tmp > 0) {
		sadc_volt = readl(jz_adc_aux->base) & 0xfff;
	} else if(tmp == -ERESTARTSYS){
		goto restart;
	} else {
        goto restart;
	}

	if (sadc_volt < 0) {
        goto restart;
	}

	disable_irq(jz_adc_aux->irq);
	jz_adc_aux->cell->disable(jz_adc_aux->pdev);

	sadc_volt = sadc_volt * VREF_ADC / AUXCONST;

	return sadc_volt;
}


int jz_adc_aux_open(struct inode *inode, struct file *filp)
{
	//struct miscdevice *dev = filp->private_data;
	//struct jz_adc_aux *axu = container_of(dev, struct jz_adc_aux, mdev);
	return 0;
}

int jz_adc_aux_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t jz_adc_aux_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	unsigned int sadc_val = 0;
	struct miscdevice *dev = filp->private_data;
	struct jz_adc_aux *aux = container_of(dev, struct jz_adc_aux, mdev);

	sadc_val = jz_adc_aux_sample_volt(aux->pdev->id,aux);
	if (sadc_val < 0) {
		printk("jz_adc_aux read value error !!\n");
		return -EINVAL;
	}

	if(copy_to_user(buf, &sadc_val, sizeof(int))) {
		return -EFAULT;
	}

	return sizeof(int);
}

static long jz_adc_aux_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct miscdevice *dev = filp->private_data;
	struct jz_adc_aux *adc_aux = container_of(dev, struct jz_adc_aux, mdev);

	switch (cmd) {
		case ADC_ENABLE:
			ret = adc_aux->cell->enable(adc_aux->pdev);
			break;
		case ADC_DISABLE:
			ret = adc_aux->cell->disable(adc_aux->pdev);
			break;
		case ADC_SET_VREF:
			VREF_ADC = *(unsigned int *)arg;
			break;
		default:
			ret = -1;
			printk("%s:unsupported ioctl cmd\n",__func__);
	}

	return ret;
}

struct file_operations jz_adc_aux_fops= {
	.owner= THIS_MODULE,
	.open= jz_adc_aux_open,
	.release= jz_adc_aux_release,
	.read= jz_adc_aux_read,
	.unlocked_ioctl= jz_adc_aux_ioctl,
};

static int jz_adc_aux_probe(struct platform_device *pdev)
{
	int ret = 0;
	char aux_name[16];

	struct jz_adc_aux *jz_adc_aux = NULL;

	jz_adc_aux = kzalloc(sizeof(*jz_adc_aux), GFP_KERNEL);
	if (!jz_adc_aux) {
		dev_err(&pdev->dev, "Failed to allocate driver structre\n");
		return -ENOMEM;
	}

	jz_adc_aux->cell = mfd_get_cell(pdev);
	if (!jz_adc_aux->cell) {
		ret = -ENOENT;
		dev_err(&pdev->dev, "Failed to get mfd cell for jz_adc_aux!\n");
		goto err_free;
	}

	jz_adc_aux->irq = platform_get_irq(pdev, 0);
	if (jz_adc_aux->irq < 0) {
		ret = jz_adc_aux->irq;
		dev_err(&pdev->dev, "Failed to get platform irq: %d\n", ret);
		goto err_free;
	}

	jz_adc_aux->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!jz_adc_aux->mem) {
		ret = -ENOENT;
		dev_err(&pdev->dev, "Failed to get platform mmio resource\n");
		goto err_free;
	}

	jz_adc_aux->mem = request_mem_region(jz_adc_aux->mem->start,
			resource_size(jz_adc_aux->mem), pdev->name);
	if (!jz_adc_aux->mem) {
		ret = -EBUSY;
		dev_err(&pdev->dev, "Failed to request mmio memory region\n");
		goto err_free;
	}

	jz_adc_aux->base = ioremap_nocache(jz_adc_aux->mem->start,resource_size(jz_adc_aux->mem));
	if (!jz_adc_aux->base) {
		ret = -EBUSY;
		dev_err(&pdev->dev, "Failed to ioremap mmio memory\n");
		goto err_free;
	}

	jz_adc_aux->pdev = pdev;
	jz_adc_aux->mdev.minor = MISC_DYNAMIC_MINOR;
	sprintf(aux_name, "jz_adc_aux_%d", pdev->id);
	jz_adc_aux->mdev.name = aux_name;
	jz_adc_aux->mdev.fops = &jz_adc_aux_fops;

	ret = misc_register(&jz_adc_aux->mdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "misc_register failed\n");
		goto err_free;
	}

	init_completion(&jz_adc_aux->read_completion);

	ret = request_irq(jz_adc_aux->irq, jz_jz_adc_aux_irq_handler, 0, pdev->name, jz_adc_aux);
	if (ret) {
		dev_err(&pdev->dev, "Failed to request irq %d\n", ret);
		goto err_free;
	}

	disable_irq(jz_adc_aux->irq);

	platform_set_drvdata(pdev, jz_adc_aux);

	return 0;

err_free :
	kfree(jz_adc_aux);
	return ret;

}

static int jz_adc_aux_remove(struct platform_device *pdev)
{
	struct jz_adc_aux *jz_adc_aux = platform_get_drvdata(pdev);

	misc_deregister(&jz_adc_aux->mdev);
	free_irq(jz_adc_aux->irq, jz_adc_aux);
	iounmap(jz_adc_aux->base);
	release_mem_region(jz_adc_aux->mem->start,resource_size(jz_adc_aux->mem));
	kfree(jz_adc_aux);

	return 0;
}

static struct platform_driver jz_adc_aux_driver = {
	.probe	= jz_adc_aux_probe,
	.remove	= jz_adc_aux_remove,
	.driver = {
		.name	= "jz-aux",
		.owner	= THIS_MODULE,
	},
	.suspend	= jz_adc_aux_suspend,
	.resume		= jz_adc_aux_resume,
};

static int __init jz_adc_aux_init(void)
{
	platform_driver_register(&jz_adc_aux_driver);

	return 0;
}

static void __exit jz_adc_aux_exit(void)
{
	platform_driver_unregister(&jz_adc_aux_driver);
}

module_init(jz_adc_aux_init);
module_exit(jz_adc_aux_exit);

MODULE_ALIAS("platform: JZ jz_adc_aux");
MODULE_AUTHOR("Guo Xu<xu.guo@ingenic.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("JZ adc aux sample driver");
