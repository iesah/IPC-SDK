/*
 * d2041-core.c  --  Device access for Dialog D2041
 *
 * Copyright(c) 2011 Dialog Semiconductor Ltd.
 *
 * Author: Dialog Semiconductor Ltd. D. Chen, D. Patel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/ioport.h>
#include <linux/gpio.h>

#include <linux/smp.h>

#include <linux/d2041/d2041_version.h>
#include <linux/d2041/d2041_reg.h>
#include <linux/d2041/d2041_core.h>
#include <linux/d2041/d2041_pmic.h>
#include <linux/d2041/d2041_rtc.h>

#include <linux/proc_fs.h>
#include <linux/kthread.h>

#ifdef CONFIG_BOARD_CORI
#include <linux/broadcom/pmu_chip.h>
#include <asm/uaccess.h>
#endif /* CONFIG_BOARD_CORI */

//#define D2041_REG_DEBUG

#ifdef D2041_REG_DEBUG

#define D2041_MAX_HISTORY 100
#define D2041_HISTORY_READ_OP 0
#define D2041_HISTORY_WRITE_OP 1

struct d2041_reg_history{
	u8 mode;
	u8 regnum;
	u8 value;
	long long time;
};

static u8 gD2041RegCache[D2041_MAX_REGISTER_CNT];
static struct d2041_reg_history gD2041RegHistory[D2041_MAX_HISTORY];
static u8 gD2041CurHistory=0;

#define d2041_write_reg_cache(reg,data)   gD2041RegCache[reg]=data;

#endif /* D2041_REG_DEBUG */

/*
 *   Static global variable
 */
static struct d2041 *d2041_dev_info;

/*
 * D2041 Device IO
 */
static DEFINE_MUTEX(io_mutex);

#ifdef D2041_REG_DEBUG
void d2041_write_reg_history(u8 opmode,u8 reg,u8 data) {
	//		int cpu = smp_processor_id();

	if(gD2041CurHistory==D2041_MAX_HISTORY)
		gD2041CurHistory=0;
	gD2041RegHistory[gD2041CurHistory].mode=opmode;
	gD2041RegHistory[gD2041CurHistory].regnum=reg;
	gD2041RegHistory[gD2041CurHistory].value=data;
	//    	gD2041RegHistory[gD2041CurHistory].time= cpu_clock(cpu)/1000;
	gD2041CurHistory++;
}
#endif

static int d2041_read(struct d2041 *d2041, u8 reg, int num_regs, u8 * dest)
{
	int bytes = num_regs;

	if (d2041->read_dev == NULL)
		return -ENODEV;

	if ((reg + num_regs - 1) >= D2041_MAX_REGISTER_CNT) {
		dev_err(d2041->dev, "invalid reg %x\n", reg + num_regs - 1);
		return -EINVAL;
	}

	/* Actually read it out */
	return d2041->read_dev(d2041, reg, bytes, (char *)dest);
}

static int d2041_write(struct d2041 *d2041, u8 reg, int num_regs, u8 * src)
{
	int bytes = num_regs;

	if (d2041->write_dev == NULL)
		return -ENODEV;

	if ((reg + num_regs - 1) >= D2041_MAX_REGISTER_CNT) {
		dev_err(d2041->dev, "invalid reg %x\n",
				reg + num_regs - 1);
		return -EINVAL;
	}
	/* Actually write it out */
	return d2041->write_dev(d2041, reg, bytes, (char *)src);
}

/*
 * d2041_clear_bits -
 * @ d2041 :
 * @ reg :
 * @ mask :
 *
 */
int d2041_clear_bits(struct d2041 * const d2041, u8 const reg, u8 const mask)
{
	u8 data;
	int err;

	mutex_lock(&io_mutex);
	err = d2041_read(d2041, reg, 1, &data);
	if (err != 0) {
		dev_err(d2041->dev, "read from reg R%d failed\n", reg);
		goto out;
	}
#ifdef D2041_REG_DEBUG
	else
		d2041_write_reg_history(D2041_HISTORY_READ_OP,reg,data);
#endif
	data &= ~mask;
	err = d2041_write(d2041, reg, 1, &data);
	if (err != 0)
		dlg_err("write to reg R%d failed\n", reg);
#ifdef D2041_REG_DEBUG
	else  {
		d2041_write_reg_history(D2041_HISTORY_WRITE_OP,reg,data);
		d2041_write_reg_cache(reg,data);
	}
#endif
out:
	mutex_unlock(&io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(d2041_clear_bits);

/*
 * d2041_set_bits -
 * @ d2041 :
 * @ reg :
 * @ mask :
 *
 */
int d2041_set_bits(struct d2041 * const d2041, u8 const reg, u8 const mask)
{
	u8 data;
	int err;

	mutex_lock(&io_mutex);
	err = d2041_read(d2041, reg, 1, &data);
	if (err != 0) {
		dev_err(d2041->dev, "read from reg R%d failed\n", reg);
		goto out;
	}
#ifdef D2041_REG_DEBUG
	else {
		d2041_write_reg_history(D2041_HISTORY_READ_OP,reg,data);
	}
#endif
	data |= mask;
	err = d2041_write(d2041, reg, 1, &data);
	if (err != 0)
		dlg_err("write to reg R%d failed\n", reg);
#ifdef D2041_REG_DEBUG
	else  {
		d2041_write_reg_history(D2041_HISTORY_WRITE_OP,reg,data);
		d2041_write_reg_cache(reg,data);
	}
#endif
out:
	mutex_unlock(&io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(d2041_set_bits);

/*
 * d2041_reg_read -
 * @ d2041 :
 * @ reg :
 * @ *dest :
 *
 */
int d2041_reg_read(struct d2041 * const d2041, u8 const reg, u8 *dest)
{
	u8 data;
	int err;

	mutex_lock(&io_mutex);
	err = d2041_read(d2041, reg, 1, &data);
	if (err != 0)
		dlg_err("read from reg R%d failed\n", reg);
#ifdef D2041_REG_DEBUG
	else
		d2041_write_reg_history(D2041_HISTORY_READ_OP,reg,data);
#endif
	*dest = data;
	mutex_unlock(&io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(d2041_reg_read);

/*
 * d2041_reg_write -
 * @ d2041 :
 * @ reg :
 * @ val :
 *
 */
int d2041_reg_write(struct d2041 * const d2041, u8 const reg, u8 const val)
{
	int ret;
	u8 data = val;


	mutex_lock(&io_mutex);
	ret = d2041_write(d2041, reg, 1, &data);
	if (ret != 0)
		dlg_err("write to reg R%d failed\n", reg);
#ifdef D2041_REG_DEBUG
	else  {
		d2041_write_reg_history(D2041_HISTORY_WRITE_OP,reg,data);
		d2041_write_reg_cache(reg,data);
	}
#endif
	mutex_unlock(&io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(d2041_reg_write);

/*
 * d2041_block_read -
 * @ d2041 :
 * @ start_reg :
 * @ regs :
 * @ *dest :
 *
 */
int d2041_block_read(struct d2041 * const d2041, u8 const start_reg, u8 const regs,
		u8 * const dest)
{
	int err = 0;
#ifdef D2041_REG_DEBUG
	int i;
#endif

	mutex_lock(&io_mutex);
	err = d2041_read(d2041, start_reg, regs, dest);
	if (err != 0)
		dlg_err("block read starting from R%d failed\n", start_reg);
#ifdef D2041_REG_DEBUG
	else {
		for(i=0; i<regs; i++)
			d2041_write_reg_history(D2041_HISTORY_WRITE_OP,start_reg+i,*(dest+i));
	}
#endif
	mutex_unlock(&io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(d2041_block_read);

/*
 * d2041_block_write -
 * @ d2041 :
 * @ start_reg :
 * @ regs :
 * @ *src :
 *
 */
int d2041_block_write(struct d2041 * const d2041, u8 const start_reg, u8 const regs,
		u8 * const src)
{
	int ret = 0;
#ifdef D2041_REG_DEBUG
	int i;
#endif

	mutex_lock(&io_mutex);
	ret = d2041_write(d2041, start_reg, regs, src);
	if (ret != 0)
		dlg_err("block write starting at R%d failed\n", start_reg);
#ifdef D2041_REG_DEBUG
	else {
		for(i=0; i<regs; i++)
			d2041_write_reg_cache(start_reg+i,*(src+i));
	}
#endif
	mutex_unlock(&io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(d2041_block_write);

int d2041_print_all_registers(struct d2041 *d2041)
{
	int i = 0;
	unsigned char reg_val[D2041_BIAS_CTRL_REG + 1] = { 0 };

	d2041_block_read(d2041, 0x00, 0x7A + 1, reg_val);
	for (i = 0; i <= 0x7A; i ++) {
		printk("REG[0x%02x] = 0x%02x\n",i, reg_val[i]);
	}

	memset(reg_val, 0, sizeof(reg_val));

	d2041_block_read(d2041, 0x80, 0xAE - 0x80 + 1, reg_val);
	for (i = 0x80; i <= 0xAE; i ++) {
		printk("REG[0x%02x] = 0x%02x\n",i, reg_val[i - 0x80]);
	}

	return 0;
}


/*
 * Register a client device.  This is non-fatal since there is no need to
 * fail the entire device init due to a single platform device failing.
 */
static void d2041_client_dev_register(struct d2041 *d2041,
		const char *name,
		struct platform_device **pdev)
{
	int ret;

	*pdev = platform_device_alloc(name, -1);
	if (*pdev == NULL) {
		dev_err(d2041->dev, "Failed to allocate %s\n", name);
		return;
	}

	(*pdev)->dev.parent = d2041->dev;
	platform_set_drvdata(*pdev, d2041);
	ret = platform_device_add(*pdev);
	if (ret != 0) {
		dev_err(d2041->dev, "Failed to register %s: %d\n", name, ret);
		platform_device_put(*pdev);
		*pdev = NULL;
	}
}

#if 0
static void d2041_worker_init(unsigned int irq)
{

}
#endif

/*
 *
 */
static irqreturn_t d2041_system_event_handler(int irq, void *data)
{
	//todo DLG export the event??
	//struct d2041 *d2041 = data;
	return IRQ_HANDLED;
}

/*
 *
 */
static void d2041_system_event_init(struct d2041 *d2041)
{
	d2041_register_irq(d2041, D2041_IRQ_EVDDMON, d2041_system_event_handler,
			0, "VDD MON", d2041);
}

/*****************************************/
/* 	Debug using proc entry           */
/*****************************************/
#ifdef CONFIG_BOARD_CORI
static int d2041_ioctl_open(struct inode *inode, struct file *file)
{
	dlg_info("%s\n", __func__);
	file->private_data = PDE(inode)->data;
	return 0;
}

int d2041_ioctl_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

/*
 *
 */
static long d2041_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct d2041 *d2041 =  file->private_data;
	pmu_reg reg;
	int ret = 0;
	u8 reg_val, event_reg[4];

	if (!d2041)
		return -ENOTTY;

	// _kg TODO: Checking if d2041_reg_write() and d2041_reg_read()
	// return with success.
	switch (cmd) {
		case BCM_PMU_IOCTL_ENABLE_INTS:
			ret = d2041_block_read(d2041, D2041_EVENTA_REG, 4, event_reg);
			dlg_info("int register 0x%X = 0x%X\n", D2041_EVENTA_REG, event_reg[0]);
			dlg_info("int register 0x%X = 0x%X\n", D2041_EVENTB_REG, event_reg[1]);
			dlg_info("int register 0x%X = 0x%X\n", D2041_EVENTC_REG, event_reg[2]);
			dlg_info("int register 0x%X = 0x%X\n", D2041_EVENTD_REG, event_reg[3]);

			/* Clear all latched interrupts if any */
			d2041_reg_write(d2041, D2041_EVENTA_REG, 0xFF);
			d2041_reg_write(d2041, D2041_EVENTB_REG, 0xFF);
			d2041_reg_write(d2041, D2041_EVENTC_REG, 0xFF);
			d2041_reg_write(d2041, D2041_EVENTD_REG, 0xFF);

			enable_irq(d2041->chip_irq);
			break;

		case BCM_PMU_IOCTL_DISABLE_INTS:
			disable_irq_nosync(d2041->chip_irq);
			break;

		case BCM_PMU_IOCTL_READ_REG:
			if (copy_from_user(&reg, (pmu_reg *)arg, sizeof(pmu_reg)) != 0)
				return -EFAULT;
			// DLG eric. 03/Nov/2011. Change prototype
			//reg.val = d2041_reg_read(d2041, reg.reg);
			// TODO: Check parameter. &reg.val
			ret = d2041_reg_read(d2041, reg.reg, &reg_val);
			reg.val = (unsigned short)reg_val;
			if (copy_to_user((pmu_reg *)arg, &reg, sizeof(pmu_reg)) != 0)
				return -EFAULT;
			break;

		case BCM_PMU_IOCTL_WRITE_REG:
			if (copy_from_user(&reg, (pmu_reg *)arg, sizeof(pmu_reg)) != 0)
				return -EFAULT;
			d2041_reg_write(d2041, reg.reg, (u8)reg.val);
			break;

		case BCM_PMU_IOCTL_SET_VOLTAGE:
		case BCM_PMU_IOCTL_GET_VOLTAGE:
		case BCM_PMU_IOCTL_GET_REGULATOR_STATE:
		case BCM_PMU_IOCTL_SET_REGULATOR_STATE:
		case BCM_PMU_IOCTL_ACTIVATESIM:
		case BCM_PMU_IOCTL_DEACTIVATESIM:
			ret = d2041_ioctl_regulator(d2041, cmd, arg);
			break;

#if 0	// DLG TODO: Not supported
		case BCM_PMU_IOCTL_START_CHARGING:
		case BCM_PMU_IOCTL_STOP_CHARGING:
		case BCM_PMU_IOCTL_SET_CHARGING_CURRENT:
		case BCM_PMU_IOCTL_GET_CHARGING_CURRENT:
			if (max8986->ioctl_handler[MAX8986_SUBDEV_POWER].handler)
				return
					max8986->ioctl_handler[MAX8986_SUBDEV_POWER].handler(
							cmd, arg,
							max8986->ioctl_handler[MAX8986_SUBDEV_POWER].pri_data);
			else
				return -ENOTTY;

#endif
		case BCM_PMU_IOCTL_POWERONOFF:
			//    d2041_set_bits(d2041,D2041_POWERCONT_REG,D2041_POWERCONT_RTCAUTOEN);
			d2041_shutdown(d2041);
			break;

		default:
			dlg_err("%s: unsupported cmd\n", __func__);
			ret = -ENOTTY;
	}

	return ret;
}

#define MAX_USER_INPUT_LEN      100
#define MAX_REGS_READ_WRITE     10

enum pmu_debug_ops {
	PMUDBG_READ_REG = 0UL,
	PMUDBG_WRITE_REG,
};

struct pmu_debug {
	int read_write;
	int len;
	int addr;
	u8 val[MAX_REGS_READ_WRITE];
};

/*
 *
 */
static void d2041_dbg_usage(void)
{
	printk(KERN_INFO "Usage:\n");
	printk(KERN_INFO "Read a register: echo 0x0800 > /proc/pmu0\n");
	printk(KERN_INFO
			"Read multiple regs: echo 0x0800 -c 10 > /proc/pmu0\n");
	printk(KERN_INFO
			"Write multiple regs: echo 0x0800 0xFF 0xFF > /proc/pmu0\n");
	printk(KERN_INFO
			"Write single reg: echo 0x0800 0xFF > /proc/pmu0\n");
	printk(KERN_INFO "Max number of regs in single write is :%d\n",
			MAX_REGS_READ_WRITE);
	printk(KERN_INFO "Register address is encoded as follows:\n");
	printk(KERN_INFO "0xSSRR, SS: i2c slave addr, RR: register addr\n");
}

/*
 *
 */
static int d2041_dbg_parse_args(char *cmd, struct pmu_debug *dbg)
{
	char *tok;                 /* used to separate tokens             */
	const char ct[] = " \t";   /* space or tab delimits the tokens    */
	bool count_flag = false;   /* whether -c option is present or not */
	int tok_count = 0;         /* total number of tokens parsed       */
	int i = 0;

	dbg->len        = 0;

	/* parse the input string */
	while ((tok = strsep(&cmd, ct)) != NULL) {
		dlg_info("token: %s\n", tok);

		/* first token is always address */
		if (tok_count == 0) {
			sscanf(tok, "%x", &dbg->addr);
		} else if (strnicmp(tok, "-c", 2) == 0) {
			/* the next token will be number of regs to read */
			tok = strsep(&cmd, ct);
			if (tok == NULL)
				return -EINVAL;

			tok_count++;
			sscanf(tok, "%d", &dbg->len);
			count_flag = true;
			break;
		} else {
			int val;

			/* this is a value to be written to the pmu register */
			sscanf(tok, "%x", &val);
			if (i < MAX_REGS_READ_WRITE) {
				dbg->val[i] = val;
				i++;
			}
		}

		tok_count++;
	}

	/* decide whether it is a read or write operation based on the
	 * value of tok_count and count_flag.
	 * tok_count = 0: no inputs, invalid case.
	 * tok_count = 1: only reg address is given, so do a read.
	 * tok_count > 1, count_flag = false: reg address and atleast one
	 *     value is present, so do a write operation.
	 * tok_count > 1, count_flag = true: to a multiple reg read operation.
	 */
	switch (tok_count) {
		case 0:
			return -EINVAL;
		case 1:
			dbg->read_write = PMUDBG_READ_REG;
			dbg->len = 1;
			break;
		default:
			if (count_flag == true) {
				dbg->read_write = PMUDBG_READ_REG;
			} else {
				dbg->read_write = PMUDBG_WRITE_REG;
				dbg->len = i;
			}
	}

	return 0;
}

/*
 *
 */
static ssize_t d2041_ioctl_write(struct file *file, const char __user *buffer,
		size_t len, loff_t *offset)
{
	struct d2041 *d2041 = file->private_data;
	struct pmu_debug dbg;
	char cmd[MAX_USER_INPUT_LEN];
	int ret, i;

	dlg_info("%s\n", __func__);

	if (!d2041) {
		dlg_err("%s: driver not initialized\n", __func__);
		return -EINVAL;
	}

	if (len > MAX_USER_INPUT_LEN)
		len = MAX_USER_INPUT_LEN;

	if (copy_from_user(cmd, buffer, len)) {
		dlg_err("%s: copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	/* chop of '\n' introduced by echo at the end of the input */
	if (cmd[len - 1] == '\n')
		cmd[len - 1] = '\0';

	if (d2041_dbg_parse_args(cmd, &dbg) < 0) {
		d2041_dbg_usage();
		return -EINVAL;
	}

	dlg_info("operation: %s\n", (dbg.read_write == PMUDBG_READ_REG) ?
			"read" : "write");
	dlg_info("address  : 0x%x\n", dbg.addr);
	dlg_info("length   : %d\n", dbg.len);

	if (dbg.read_write == PMUDBG_READ_REG) {
		ret = d2041_read(d2041, dbg.addr, dbg.len, dbg.val);
		if (ret < 0) {
			dlg_err("%s: pmu reg read failed\n", __func__);
			return -EFAULT;
		}

		for (i = 0; i < dbg.len; i++, dbg.addr++)
			dlg_info("[%x] = 0x%02x\n", dbg.addr,
					dbg.val[i]);
	} else {
		ret = d2041_write(d2041, dbg.addr, dbg.len, dbg.val);
		if (ret < 0) {
			dlg_err("%s: pmu reg write failed\n", __func__);
			return -EFAULT;
		}
	}

	*offset += len;

	return len;
}

static const struct file_operations d2041_pmu_ops = {
	.open = d2041_ioctl_open,
	.unlocked_ioctl = d2041_ioctl,
	.write = d2041_ioctl_write,
	.release = d2041_ioctl_release,
	.owner = THIS_MODULE,
};

void d2041_debug_proc_init(struct d2041 *d2041)
{
	struct proc_dir_entry *entry;

	disable_irq(d2041->chip_irq);
	entry = proc_create_data("pmu0", S_IRWXUGO, NULL, &d2041_pmu_ops, d2041);
	enable_irq(d2041->chip_irq);
	dlg_crit("\nD2041-core.c: proc_create_data() = %p; name=\"%s\"\n", entry, (entry?entry->name:""));
}

void d2041_debug_proc_exit(void)
{
	//disable_irq(client->irq);
	remove_proc_entry("pmu0", NULL);
	//enable_irq(client->irq);
}

#endif /* CONFIG_BOARD_CORI */

struct d2041 *d2041_regl_info = NULL;
EXPORT_SYMBOL(d2041_regl_info);

static unsigned char reg_addr = 0;

static ssize_t attr_reg_set(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int ret;
	unsigned long val;
	struct d2041 *d2041 = dev_get_drvdata(dev);

	if (!d2041) {
		printk("%s --> Get driver data failed!\n",__func__);
		return -EINVAL;
	}

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;

	ret = d2041_reg_write(d2041, reg_addr, val);
	if (ret < 0) {
		printk("%s --> Write reg failed!\n",__func__);
		return -EIO;
	}

	return size;
}

static ssize_t attr_reg_get(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	ssize_t ret;
	struct d2041 *d2041 = dev_get_drvdata(dev);
	unsigned char data;

	ret = d2041_reg_read(d2041, reg_addr, &data);
	if (ret < 0) {
		printk("%s --> Read reg failed!\n",__func__);
		return -EIO;
	}

	ret = sprintf(buf, "0x%02x\n", data);
	return ret;
}

static ssize_t attr_addr_get(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	ssize_t ret = sprintf(buf, "0x%02x\n", reg_addr);
	return ret;
}

static ssize_t attr_addr_set(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct d2041 *d2041 = dev_get_drvdata(dev);
	unsigned long val;

	if (!d2041) {
		printk("%s --> Get driver data failed!\n",__func__);
		return -EINVAL;
	}

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;

	reg_addr = val;

	return size;
}

static ssize_t attr_print_all_regs(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct d2041 *d2041 = dev_get_drvdata(dev);
	unsigned long val;

	if (!d2041) {
		printk("%s --> Get driver data failed!\n",__func__);
		return -EINVAL;
	}

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;

	if (1 == val) {
		d2041_print_all_registers(d2041);
	}

	return size;
}

static struct device_attribute attributes[] = {

	__ATTR(reg_value, 0600, attr_reg_get, attr_reg_set),
	__ATTR(reg_addr, 0600, attr_addr_get, attr_addr_set),
	__ATTR(print_all_regs, 0600, NULL, attr_print_all_regs),
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;

error:
	for ( ; i >= 0; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s:Unable to create interface\n", __func__);
	return -1;
}

static int remove_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
	return 0;
}

int d2041_device_init(struct d2041 *d2041, int irq, struct d2041_platform_data *pdata)
{
	int ret = 0;
	uint8_t chip_id = 0;

#ifdef D2041_REG_DEBUG
	int i;
	u8 data;
#endif
	if (d2041 != NULL) {
		d2041_regl_info = d2041;
	} else {
		goto err;
	}

	dlg_info("D2041 Driver version : %s\n", D2041_VERSION);

	d2041->pmic.max_dcdc = 25;
	d2041->pdata = pdata;

	/* Page write for I2C we donot support repeated write and I2C speed set to 400KHz */
	d2041_clear_bits(d2041, D2041_CONTROLB_REG, D2041_CONTROLB_WRITEMODE | D2041_CONTROLB_I2C_SPEED);

	ret = d2041_reg_read(d2041, D2041_CHIPID_REG, &chip_id);
	if (ret < 0)
		goto err_read_chipid;

	dlg_info("D2041 Chip ID : %02x\n",chip_id);

#if 0
	// 20120221 LDO13 issue
	d2041_reg_write(d2041, D2041_PDDIS_REG,0x0);
	d2041_reg_write(d2041, D2041_PULLDOWN_REG_D,0x0);
	// audio
	d2041_reg_write(d2041, D2041_PREAMP_A_CTRL1_REG,0x34);
	d2041_reg_write(d2041, D2041_PREAMP_A_CTRL2_REG,0x0);
	d2041_reg_write(d2041, D2041_SP_CTRL_REG,0xCC);

	// LDO
	d2041_reg_write(d2041, D2041_LDO10_REG, 0x24); //LDO 10 3.0V    // spare
	d2041_reg_write(d2041, D2041_LDO11_REG, 0x24); //LDO 11 3.0V    // VSIM
	d2041_reg_write(d2041, D2041_LDO13_REG, 0x24); //LDO 13 3.0V    // DLDO1
	d2041_reg_write(d2041, D2041_LDO14_REG, 0x22); //LDO 14 2.9V    // VTOUCH
	d2041_reg_write(d2041, D2041_LDO15_REG, 0x2A); //LDO 15 3.3V    // VMOT
	d2041_reg_write(d2041, D2041_LDO16_REG, 0xC ); //LDO 16 1.8V    // VCAMC
	d2041_reg_write(d2041, D2041_LDO17_REG, 0x22); //LDO 17 2.9V    // VBBD
	d2041_reg_write(d2041, D2041_LDO18_REG, 0x2A); //LDO 18 3.3V    // VTOUCH_3.3V
	d2041_reg_write(d2041, D2041_LDO19_REG, 0xC ); //LDO 19 1.8V    // VCAM_IO
	d2041_reg_write(d2041, D2041_LDO20_REG, 0x20); //LDO 20 2.8V    // VCAM_2.8V_AVDD
	d2041_reg_write(d2041, D2041_BUCK4_REG, 0x53); //BUCK 4 3.3V
#endif

#if defined(MFD_DA9024_USE_BBAT_V1_2)
	/* set backup battery charge current and voltage */
	d2041_reg_write(d2041,D2041_BBATCONT_REG,0x1F);
	d2041_set_bits(d2041,D2041_SUPPLY_REG,D2041_SUPPLY_BBCHGEN);
#else
	d2041_clear_bits(d2041,D2041_SUPPLY_REG,D2041_SUPPLY_BBCHGEN);
#endif

	/* DLG todo */
	if (pdata && pdata->irq_init) {
		dlg_crit("\nD2041-core.c: IRQ PIN Configuration \n");
		ret = pdata->irq_init(d2041);
		if (ret != 0) {
			dev_err(d2041->dev, "Platform init() failed: %d\n", ret);
			goto err_irq;
		}
	}

	d2041_dev_info = d2041;
//	pm_power_off = d2041_system_poweroff;

	ret = d2041_irq_init(d2041, irq, pdata);
	if (ret < 0)
		goto err;

//	DLG todo d2041_worker_init(irq); //new for Samsung
	if (pdata && pdata->init) {

		ret = pdata->init(d2041);
		if (ret != 0) {
			dev_err(d2041->dev, "Platform init() failed: %d\n", ret);
			goto err_irq;
		}
	}

	/* Regulator Specific Init */
	ret = d2041_platform_regulator_init(d2041);
	if (ret != 0) {
		dev_err(d2041->dev, "Platform Regulator init() failed: %d\n", ret);
		goto err_irq;
	}

	d2041_client_dev_register(d2041, "linear-power", &(d2041->power.pdev)); // temporary code
	d2041_client_dev_register(d2041, "d2041-rtc", &(d2041->rtc.pdev));
	d2041_client_dev_register(d2041, "d2041-onkey", &(d2041->onkey.pdev));
	d2041_client_dev_register(d2041, "d2041-audio", &(d2041->audio.pdev));

	d2041_system_event_init(d2041);

#ifdef CONFIG_BOARD_CORI
	d2041_debug_proc_init(d2041);
#endif

#ifdef D2041_REG_DEBUG
	for (i = 0; i < D2041_MAX_REGISTER_CNT; i++) {
		d2041_reg_read(d2041, i, &data);
		d2041_write_reg_cache(i,data);
	}
#endif

	/* temporary code */
	if (pdata->pmu_event_cb)
		pdata->pmu_event_cb(0, 0);//PMU_EVENT_INIT_PLATFORM

#if defined(CONFIG_MFD_DA9024_USE_MCTL_V1_2)
	/* set MCTRL_EN enabled */
	set_MCTL_enabled();
#endif

	ret = create_sysfs_interfaces(&d2041->i2c_client->dev);
	if (ret < 0) {
		dev_err(d2041->dev,"device d2041  sysfs register failed!\n");
	}

	return 0;

err_irq:
	d2041_irq_exit(d2041);
	d2041_dev_info = NULL;
	pm_power_off = NULL;
err:
	dlg_crit("\n\nD2041-core.c: device init failed ! \n\n");
err_read_chipid:
	return ret;
}
EXPORT_SYMBOL_GPL(d2041_device_init);

void d2041_device_exit(struct d2041 *d2041)
{
#ifdef CONFIG_BOARD_CORI
	d2041_debug_proc_exit();
#endif /* CONFIG_BOARD_CORI */
	d2041_dev_info = NULL;

	platform_device_unregister(d2041->rtc.pdev);
	platform_device_unregister(d2041->onkey.pdev);
	platform_device_unregister(d2041->audio.pdev);
	d2041_free_irq(d2041, D2041_IRQ_EVDDMON);
	d2041_irq_exit(d2041);

#ifdef D2041_INT_USE_THREAD
	if(d2041->irq_task)
		kthread_stop(d2041->irq_task);
#endif
}
EXPORT_SYMBOL_GPL(d2041_device_exit);

int d2041_shutdown(struct d2041 *d2041)
{

	u8 dst;
	int ret;
	dlg_info("%s\n", __func__);

	if (d2041->read_dev == NULL)
		return -ENODEV;

	d2041_clear_bits(d2041,D2041_CONTROLB_REG,D2041_CONTROLB_OTPREADEN); //otp reload disable

	d2041_clear_bits(d2041,D2041_POWERCONT_REG,D2041_POWERCONT_MCTRLEN); //mctl disable

	d2041_reg_write(d2041, D2041_BUCK1_REG, 0x5c); //buck1 1.2 V 0x1c 1.5V 0x28

	// 20120221 LDO13 issue
	dst = 0x0;
	d2041->write_dev(d2041, D2041_IRQMASKB_REG, 1, &dst); //onkey mask clear

	d2041_clear_bits(d2041, D2041_LDO5_REG,    D2041_REGULATOR_EN); //LDO 5 disable
	d2041_clear_bits(d2041, D2041_LDO6_REG,    D2041_REGULATOR_EN); //LDO 6 disable
	d2041_clear_bits(d2041, D2041_LDO7_REG,    D2041_REGULATOR_EN); //LDO 7 disable
	d2041_clear_bits(d2041, D2041_LDO8_REG,    D2041_REGULATOR_EN); //LDO 8 disable
	d2041_clear_bits(d2041, D2041_LDO9_REG,    D2041_REGULATOR_EN); //LDO 9 disable
	d2041_clear_bits(d2041, D2041_LDO10_REG,   D2041_REGULATOR_EN); //LDO 10 disable
	d2041_clear_bits(d2041, D2041_LDO11_REG,   D2041_REGULATOR_EN); //LDO 11 disable
	d2041_clear_bits(d2041, D2041_LDO12_REG,   D2041_REGULATOR_EN); //LDO 12 disable
	d2041_clear_bits(d2041, D2041_LDO13_REG,   D2041_REGULATOR_EN); //LDO 13 disable
	d2041_clear_bits(d2041, D2041_LDO14_REG,   D2041_REGULATOR_EN); //LDO 14 disable
	d2041_clear_bits(d2041, D2041_LDO15_REG,   D2041_REGULATOR_EN); //LDO 15 disable
	d2041_clear_bits(d2041, D2041_LDO16_REG,   D2041_REGULATOR_EN); //LDO 16 disable
	d2041_clear_bits(d2041, D2041_LDO17_REG,   D2041_REGULATOR_EN); //LDO 17 disable
	d2041_clear_bits(d2041, D2041_LDO18_REG,   D2041_REGULATOR_EN); //LDO 18 disable
	d2041_clear_bits(d2041, D2041_LDO19_REG,   D2041_REGULATOR_EN); //LDO 19 disable
	d2041_clear_bits(d2041, D2041_LDO20_REG,   D2041_REGULATOR_EN); //LDO 20 disable
	d2041_clear_bits(d2041, D2041_LDO_AUD_REG, D2041_REGULATOR_EN); //LDO_AUD disable

#if 0
	dst = 0x0;
	d2041->write_dev(d2041, D2041_BUCK4_REG, 1, &dst); //BUCK 4
#endif

//	dst = 0x10;
//	d2041->write_dev(d2041, D2041_SUPPLY_REG, 1, &dst);

//	dst = 0x0E;
//	d2041->write_dev(d2041, D2041_POWERCONT_REG, 1, &dst);

	dst = 0xEF;
	d2041->write_dev(d2041, D2041_PDDIS_REG, 1, &dst);

	ret = d2041->read_dev(d2041, D2041_CONTROLB_REG, 1, &dst);

	dst |= D2041_CONTROLB_DEEPSLEEP;
	d2041->write_dev(d2041, D2041_CONTROLB_REG, 1, &dst);
	return 0;
}
EXPORT_SYMBOL(d2041_shutdown);


/* D2041 poweroff function */
void d2041_system_poweroff(void)
{
	dlg_info("%s\n", __func__);
	if (d2041_dev_info) {
		d2041_shutdown(d2041_dev_info);
	}
}
EXPORT_SYMBOL(d2041_system_poweroff);

MODULE_AUTHOR("Dialog Semiconductor Ltd <divyang.patel@diasemi.com>");
MODULE_DESCRIPTION("D2041 PMIC Core");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" D2041_I2C);
