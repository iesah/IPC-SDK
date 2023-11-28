/*
 * linux/drivers/misc/jz_efuse_v13.c - Ingenic efuse driver
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Author: Mick <dongyue.ye@ingenic.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <soc/base.h>
#include <mach/jz_efuse.h>
#include <jz_proc.h>

//#define DEBUG
#define DRV_NAME "jz-efuse-v13"

#define EFUSE_CTRL		0x0
#define EFUSE_CFG		0x4
#define EFUSE_STATE		0x8
#define EFUSE_DATA		0xC

#define OFFSET_TO_ADDR(addr,i,read_bytes) (addr+(i*read_bytes))

#define CHIP_ID_ADDR (0x00)
#define USER_ID_ADDR (0x0C)
#define SARADC_CAL (0x10)
#define TRIM_ADDR (0x12)
#define PROGRAM_PROTECT_ADDR (0x15)
#define CPU_ID_ADDR (0x16)
#define CUSTOMER_RESV_ADDR (0x18)

uint32_t seg_addr[] = {
	CHIP_ID_ADDR,
	USER_ID_ADDR,
	SARADC_CAL,
	TRIM_ADDR,
	PROGRAM_PROTECT_ADDR,
	CPU_ID_ADDR,
	CUSTOMER_RESV_ADDR,
};

struct efuse_wr_info {
	uint32_t seg_id;
	uint32_t bytes;
	uint32_t offset;
	uint32_t *buf;
};

#define CMD_READ      _IOWR('k', 51, struct efuse_wr_info *)
#define CMD_WRITE     _IOWR('k', 52, struct efuse_wr_info *)

struct jz_efuse {
	struct jz_efuse_platform_data *pdata;
	struct device *dev;
	struct miscdevice mdev;
	uint32_t *id2addr;
	struct efuse_wr_info *wr_info;
	spinlock_t lock;
	void __iomem *iomem;
	int gpio_vddq_en_n;
	struct timer_list vddq_protect_timer;
};

static struct jz_efuse *efuse;
static struct jz_efuse *extra_efuse = NULL;

static uint32_t efuse_readl(uint32_t reg_off)
{
	return readl(efuse->iomem + reg_off);
}

static void efuse_writel(uint32_t val, uint32_t reg_off)
{
	if (reg_off == EFUSE_CTRL)
		val |= (1 << 31);//keep bit31 in reset value 1;

	writel(val, efuse->iomem + reg_off);
}

static void nemc_off(void)
{
	unsigned int regtmp ;
	regtmp = readl((volatile unsigned int *)0xb00000B8);
	regtmp |= 2;
	writel(regtmp,(volatile unsigned int *)0xb00000B8);
}
static void nemc_on(void)
{
	unsigned int regtmp ;
	regtmp = readl((volatile unsigned int *)0xb00000B8);
	regtmp &= ~2;
	writel(regtmp,(volatile unsigned int *)0xb00000B8);
}

static void efuse_vddq_set(unsigned long is_on)
{
	if (is_on) {
		mod_timer(&efuse->vddq_protect_timer, jiffies + HZ);
	}

	gpio_set_value(efuse->gpio_vddq_en_n, !is_on);
}

static int efuse_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int efuse_release(struct inode *inode, struct file *filp)
{
	/*clear configer register*/
	/*efuse_writel(0, EFUSE_CFG);*/
	return 0;
}


static int jz_efuse_check_arg(uint32_t seg_id, uint32_t bit_num)
{
	if(seg_id == CHIP_ID) {
		if(bit_num > 96) {
			dev_err(efuse->dev, "read segment %d data length %d > 96 bit", seg_id, bit_num);
			return -1;
		}
	} else if(seg_id == USER_ID) {
		if(bit_num > 32) {
			dev_err(efuse->dev, "read segment %d data length %d > 32 bit", seg_id, bit_num);
			return -1;
		}
	} else if(seg_id == SARADC_CAL_DAT) {
		if(bit_num > 16) {
			dev_err(efuse->dev, "read segment %d data length %d > 16 bit", seg_id, bit_num);
			return -1;
		}
	} else if(seg_id == TRIM_DATA) {
		if(bit_num > 24) {
			dev_err(efuse->dev, "read segment %d data length %d > 16 bit", seg_id, bit_num);
			return -1;
		}
	} else if(seg_id == PROGRAM_PROTECT) {
		if(bit_num > 8) {
			dev_err(efuse->dev, "read segment %d data length %d > 8 bit", seg_id, bit_num);
			return -1;
		}
	} else if(seg_id == CPU_ID) {
		if(bit_num > 16) {
			dev_err(efuse->dev, "read segment %d data length %d > 16 bit", seg_id, bit_num);
			return -1;
		}
	} else if(seg_id == CUSTOMER_RESV) {
		if(bit_num > 320) {
			dev_err(efuse->dev, "read segment %d data length %d > 320 bit", seg_id, bit_num);
			return -1;
		}
	} else {
		dev_err(efuse->dev, "read segment num is error(0 ~ 7)");
		return -1;
	}

	return 0;

}

int jz_efuse_read(uint32_t seg_id, uint32_t r_bytes,uint32_t offset, uint32_t *buf)
{
	unsigned long flags;
	unsigned int val, addr = 0,bit_num;
	uint32_t read_step = 1;//read step bytes
	uint32_t rd_n = r_bytes / read_step;
	int i;

	/*printk("[%s] bytes = %d,num = %d seg_id = %d offset = %d\n",__func__,r_bytes,rd_n,seg_id,offset);*/

	nemc_on();

	bit_num = (r_bytes + offset)* 8;
	if (jz_efuse_check_arg(seg_id, bit_num) == -1)
	{
		dev_err(efuse->dev, "efuse arg check error \n");
		return -1;
	}

	spin_lock_irqsave(&efuse->lock, flags);
	/* set read Programming address and data length */

	efuse_writel(0, EFUSE_STATE);

	for (i = 0; i < rd_n; i++) {
		addr = efuse->id2addr[seg_id];
		addr = OFFSET_TO_ADDR(addr + offset,i,read_step);
		/*printk("-----read %d bytes form addr 0x2%02x \n",read_bytes,addr);*/
		val =  addr << 21 | (read_step - 1) << 16;
		efuse_writel(val, EFUSE_CTRL);

		/* enable read */
		val = efuse_readl(EFUSE_CTRL);
		val |= 1;
		efuse_writel(val, EFUSE_CTRL);
		/* wait read done status */
		while(!(efuse_readl(EFUSE_STATE) & 1));

		*((char *)buf + i) = efuse_readl(EFUSE_DATA);
		/* clear read done staus */
		efuse_writel(0, EFUSE_STATE);
		/*printk("------i = %d \n",i);*/
	}

	spin_unlock_irqrestore(&efuse->lock, flags);

	nemc_off();

	return 0;
}
EXPORT_SYMBOL_GPL(jz_efuse_read);

int jz_efuse_write(uint32_t seg_id, uint32_t w_bytes, uint32_t offset, uint32_t *buf)
{
	unsigned long flags;
	unsigned int val, bit_num, addr;
	uint32_t write_step = 1;
	uint32_t wr_n = w_bytes / write_step;
	int i;

	/*printk(" [%s] bytes = %d,num = %d seg_id = %d offset = %d\n",__func__,w_bytes,wr_n,seg_id,offset);*/

	nemc_on();

	bit_num = (w_bytes + offset) * 8;
	if (jz_efuse_check_arg(seg_id, bit_num) == -1)
	{
		dev_err(efuse->dev, "efuse arg check error \n");
		return -1;
	}
	spin_lock_irqsave(&efuse->lock, flags);


	efuse_writel(0, EFUSE_STATE);

	for (i = 0; i < wr_n; i++) {

		efuse_writel(*((char *)buf + i), EFUSE_DATA);
		/*
		 * set write Programming address and data length
		 */
		addr = efuse->id2addr[seg_id];
		addr = OFFSET_TO_ADDR(addr + offset,i,write_step);
		/*printk("-----write %d bytes to addr 0x2%02x \n",write_bytes,addr);*/
		val =  addr << 21 | (write_step - 1) << 16;

		efuse_writel(val, EFUSE_CTRL);
		/* Connect VDDQ pin from 2.5V */
		efuse_vddq_set(1);

		/* enable write */
		val = efuse_readl(EFUSE_CTRL);
		val |= 2;
		efuse_writel(val, EFUSE_CTRL);
		/* wait write done status */
		while(!(efuse_readl(EFUSE_STATE) & 0x2));

		/* Disconnect VDDQ pin from 2.5V. */
		efuse_vddq_set(0);

		efuse_writel(0, EFUSE_CTRL);
	}

	spin_unlock_irqrestore(&efuse->lock, flags);

	nemc_off();

	return 0;
}
EXPORT_SYMBOL_GPL(jz_efuse_write);

static long efuse_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct miscdevice *dev = filp->private_data;
	struct jz_efuse *efuse = container_of(dev, struct jz_efuse, mdev);
	unsigned int * arg_r;
	int ret = 0;

	arg_r =(unsigned int *)arg;
	efuse->wr_info = (struct efuse_wr_info *)arg_r;
	switch (cmd) {
	case CMD_READ:
		ret = jz_efuse_read(efuse->wr_info->seg_id, efuse->wr_info->bytes, efuse->wr_info->offset,efuse->wr_info->buf);
		break;
	case CMD_WRITE:
		ret = jz_efuse_write(efuse->wr_info->seg_id, efuse->wr_info->bytes, efuse->wr_info->offset, efuse->wr_info->buf);
		break;
	default:
		ret = -1;
		printk("no support other cmd\n");
	}
	return ret;
}
static struct file_operations efuse_misc_fops = {
	.open		= efuse_open,
	.release	= efuse_release,
	.unlocked_ioctl	= efuse_ioctl,
};

static int efuse_read_chip_id_proc(struct seq_file *m, void *v)
{
	int len = 0;
	unsigned char buf[12];

	jz_efuse_read(0, 12, 0,(uint32_t *)buf);
	len = seq_printf(m,"--------> chip id: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11]);

	return len;
}

static int efuse_read_user_id_proc(struct seq_file *m, void *v)
{
	int len = 0;
	unsigned char buf[4];

	jz_efuse_read(1, 4, 0,(uint32_t *)buf);
	len = seq_printf(m,"--------> user id: %02x%02x%02x%02x\n",buf[0],buf[1],buf[2],buf[3]);

	return len;
}

static int efuse_read_chipID_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, efuse_read_chip_id_proc, PDE_DATA(inode));
}
static int efuse_read_userID_proc_open(struct inode *inode,struct file *file)
{
	return single_open(file, efuse_read_user_id_proc, PDE_DATA(inode));
}

static const struct file_operations efuse_proc_read_chipID_fops ={
	.read = seq_read,
	.open = efuse_read_chipID_proc_open,
	.write = NULL,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations efuse_proc_read_userID_fops ={
	.read = seq_read,
	.open = efuse_read_userID_proc_open,
	.write = NULL,
	.llseek = seq_lseek,
	.release = single_release,
};

static int jz_efuse_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct clk *h2clk;
	unsigned long rate;
	uint32_t val, ns;
	int i, rd_strobe, wr_strobe;
	uint32_t rd_adj, wr_adj;
	struct proc_dir_entry *res,*p;

	h2clk = clk_get(NULL, "h2clk");
	if (IS_ERR(h2clk)) {
		dev_err(efuse->dev, "get h2clk rate fail!\n");
		return -1;
	}

	rate = clk_get_rate(h2clk);
	ns = 1000000000 / rate;
	printk("rate = %lu, ns = %d\n", rate, ns);

	efuse= kzalloc(sizeof(struct jz_efuse), GFP_KERNEL);
	if (!efuse) {
		printk("efuse:malloc faile\n");
		return -ENOMEM;
	}

	efuse->pdata = pdev->dev.platform_data;
	if(!efuse->pdata) {
		dev_err(&pdev->dev, "No platform data\n");
		ret = -1;
		goto fail_free_efuse;
	}

	efuse->dev = &pdev->dev;
	efuse->gpio_vddq_en_n = -ENODEV;

	efuse->gpio_vddq_en_n = efuse->pdata->gpio_vddq_en_n;
	if (efuse->gpio_vddq_en_n == -ENODEV) {
		dev_err(efuse->dev, "gpio error\n");
		ret = -ENODEV;
		goto fail_free_efuse;
	}

	efuse->iomem = ioremap(EFUSE_IOBASE, 0xfff);
	if (!efuse->iomem) {
		dev_err(efuse->dev, "ioremap failed!\n");
		ret = -EBUSY;
		goto fail_free_efuse;
	}

	ret = gpio_request(efuse->gpio_vddq_en_n, dev_name(efuse->dev));
	if (ret) {
		dev_err(efuse->dev, "Failed to request gpio pin: %d\n", ret);
		goto fail_free_io;
	}
	ret = gpio_direction_output(efuse->gpio_vddq_en_n, 1); /* power off by default */
	if (ret) {
		dev_err(efuse->dev, "Failed to set gpio as output: %d\n", ret);
		goto fail_free_gpio;
	}

	dev_info(efuse->dev, "setup vddq_protect_timer!\n");
	setup_timer(&efuse->vddq_protect_timer, efuse_vddq_set, 0);
	add_timer(&efuse->vddq_protect_timer);

	spin_lock_init(&efuse->lock);

	efuse->mdev.minor = MISC_DYNAMIC_MINOR;
	efuse->mdev.name =  DRV_NAME;
	efuse->mdev.fops = &efuse_misc_fops;

	spin_lock_init(&efuse->lock);

	ret = misc_register(&efuse->mdev);
	if (ret < 0) {
		dev_err(efuse->dev, "misc_register failed\n");
		goto fail_free_gpio;
	}
	platform_set_drvdata(pdev, efuse);


	efuse->id2addr = seg_addr;
	efuse->wr_info = NULL;


	for(i = 0; i < 0xf; i++)
		if(((( i + 1) * ns ) * 10) > 65)
			break;
	if(i == 0xf) {
		dev_err(efuse->dev, "get efuse cfg rd_adj fail!\n");
		return -1;
	}
	rd_adj = wr_adj = i;

	for(i = 0; i < 0xf; i++)
		if(((rd_adj + i + 5) * ns ) > 35)
			break;
	if(i == 0xf) {
		dev_err(efuse->dev, "get efuse cfg rd_strobe fail!\n");
		return -1;
	}
	rd_strobe = i;

	for(i = 1; i < 11000; i += 100) {
		val = (wr_adj + i + 1666) * ns;
		if( val > 9 * 1000 && val < 11 *1000)
			break;
	}
	if(i >= 11000) {
		dev_err(efuse->dev, "get efuse cfg wd_strobe fail!\n");
		return -1;
	}
	wr_strobe = i;

	dev_info(efuse->dev, "rd_adj = %d | rd_strobe = %d | "
		 "wr_adj = %d | wr_strobe = %d\n", rd_adj, rd_strobe,
		 wr_adj, wr_strobe);
	/*set configer register*/
	val = 1 << 31 | rd_adj << 20 | rd_strobe << 16 | wr_adj << 12 | wr_strobe;
	efuse_writel(val, EFUSE_CFG);

	clk_put(h2clk);

	p = jz_proc_mkdir("efuse");
	if (!p) {
		pr_warning("create_proc_entry for common efuse failed.\n");
		return -ENODEV;
	}

	res = proc_create("efuse_chip_id", 0444,p,&efuse_proc_read_chipID_fops);
	if(!res){
		pr_err("create proc of efuse_chip_id error!!!!\n");
	}

	res = proc_create("efuse_user_id",0444,p,&efuse_proc_read_userID_fops);
	if(!res){
		pr_err("create proc of efuse_user_id error!!!!\n");
	}

	extra_efuse = efuse;

	dev_info(efuse->dev, "ingenic efuse interface module registered success.\n");
	return 0;

fail_free_efuse:
	kfree(efuse);
fail_free_gpio:
	gpio_free(efuse->gpio_vddq_en_n);
fail_free_io:
	iounmap(efuse->iomem);

	extra_efuse = NULL;
	return ret;
}


static int jz_efuse_remove(struct platform_device *dev)
{
	struct jz_efuse *efuse = platform_get_drvdata(dev);


	misc_deregister(&efuse->mdev);
	if (efuse->gpio_vddq_en_n != -ENODEV) {
		gpio_free(efuse->gpio_vddq_en_n);
		dev_info(efuse->dev, "del vddq_protect_timer!\n");
		del_timer(&efuse->vddq_protect_timer);
	}
	iounmap(efuse->iomem);
	kfree(efuse);

	return 0;
}

static struct platform_driver jz_efuse_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
	.probe		= jz_efuse_probe,
	.remove		= jz_efuse_remove,
};

static int __init jz_efuse_init(void)
{
	return platform_driver_register(&jz_efuse_driver);
}

static void __exit jz_efuse_exit(void)
{
	platform_driver_unregister(&jz_efuse_driver);
}

module_init(jz_efuse_init);
module_exit(jz_efuse_exit);

MODULE_DESCRIPTION("T10 efuse driver");
MODULE_LICENSE("GPL v2");
