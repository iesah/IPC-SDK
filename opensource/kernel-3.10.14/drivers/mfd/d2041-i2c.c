/*
 * d2041-i2c.c: I2C (Serial Communication) driver for D2041
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
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/d2041/d2041_core.h>
#include <linux/d2041/d2041_reg.h>
#include <linux/slab.h>
#include <jz_notifier.h>

#define MCTL_OFF      0x00
#define MCTL_ON       0x01
#define MCTL_SLEEP    0x02
extern struct d2041 *d2041_regl_info;

static int d2041_register_reset_notifier(struct notifier_block *nb)
{
	return reset_notifier_client_register(nb);
}

static int d2041_unregister_reset_notifier(struct notifier_block *nb)
{
	return reset_notifier_client_unregister(nb);
}

static int d2041_reset_notifier_handler(struct notifier_block *this, unsigned long event, void *ptr)
{
	if (event == JZ_POST_HIBERNATION) {
		d2041_system_poweroff();
                return 0;
	} else {
		printk("\n%s event not match\n", __func__);
                return EINVAL;
        }

}

static int d2041_i2c_read_device(struct d2041 * const d2041, char const reg,
		int const bytes, void * const dest)
{
	int ret;
#ifdef CONFIG_D2041_USE_SMBUS_API_V1_2
	if (bytes > 1)
		ret = i2c_smbus_read_i2c_block_data(d2041->i2c_client, reg, bytes, dest);
	else {
		ret = i2c_smbus_read_byte_data(d2041->i2c_client, reg);
		if (ret < 0)
			return ret;
		*(unsigned char *)dest = (unsigned char)ret;
	}
	return 0;
#else
	ret = i2c_master_send(d2041->i2c_client, &reg, 1);
	if (ret < 0) {
		dlg_err("Err in i2c_master_send(0x%x)\n", reg);
		return ret;
	}

	ret = i2c_master_recv(d2041->i2c_client, dest, bytes);
	if (ret < 0) {
		dlg_err("Err in i2c_master_recv(0x%x)\n", ret);
		return ret;
	}

	if (ret != bytes)
		return -EIO;
#endif /* CONFIG_D2041_USE_SMBUS_API */
	return 0;
}

static int d2041_i2c_write_device(struct d2041 * const d2041, char const reg,
		int const bytes, const u8 *src /*void * const src*/)
{
	int ret = 0;

#ifdef CONFIG_D2041_USE_SMBUS_API_V1_2
	if(bytes > 1)
		ret = i2c_smbus_write_i2c_block_data(d2041->i2c_client, reg, bytes, src);
	else
		ret = i2c_smbus_write_byte_data(d2041->i2c_client, reg, *src);

	return ret;
#else
	u8 msg[bytes + 1];

	msg[0] = reg;
	memcpy(&msg[1], src, bytes);

	ret = i2c_master_send(d2041->i2c_client, msg, bytes + 1);
	if (ret < 0)
		return ret;

	if (ret != bytes + 1)
		return -EIO;

	return 0;
#endif /* CONFIG_D2041_USE_SMBUS_API */
}

static int d2041_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	struct d2041 *d2041;
	int ret = 0;

	d2041 = kzalloc(sizeof(struct d2041), GFP_KERNEL);
	if (d2041 == NULL) {
		kfree(i2c);
		return -ENOMEM;
	}

	i2c_set_clientdata(i2c, d2041);
	d2041->dev = &i2c->dev;
	d2041->i2c_client = i2c;
	d2041->read_dev = d2041_i2c_read_device;
	d2041->write_dev = d2041_i2c_write_device;
        d2041->d2041_notifier.notifier_call = d2041_reset_notifier_handler;

        ret = d2041_register_reset_notifier(&(d2041->d2041_notifier));
	if (ret) {
		printk("d2041_register_reset_notifier failed\n");
		goto err;
	}

        ret = d2041_device_init(d2041, i2c->irq, i2c->dev.platform_data);
	if (ret < 0)
		goto err;

	return ret;
err:
	kfree(d2041);
	return ret;
}

static int d2041_i2c_remove(struct i2c_client *i2c)
{
        int ret = 0;
	struct d2041 *d2041 = i2c_get_clientdata(i2c);
	ret = d2041_unregister_reset_notifier(&(d2041->d2041_notifier));
	if (ret) {
		printk("d2041_unregister_reset_notifier failed\n");
	}
	d2041_device_exit(d2041);
	kfree(d2041);
	return ret;
}

static const struct i2c_device_id d2041_i2c_id[] = {
	{ D2041_I2C, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, d2041_i2c_id);

int d2041_ldo_suspend(struct platform_device *pdev)
{
	u8 val;
	int err;
	u8 ldo_enable_tmp;
	u8 ldo_counter;
	u8 ldo_offset = D2041_LDO1_REG;
	u8 mctl_offset = D2041_LDO1_MCTL_REG;
	u8 m_ctl;

	d2041_reg_write(d2041_regl_info, D2041_SUPPLY_REG, 0x48);
	/* get the M_CTL */
	if(err = d2041_reg_read(d2041_regl_info, D2041_STATUSA_REG,&m_ctl)){
		printk("ldo_suspend i2c read M_CTL error.\n");
		return err;
	}
	m_ctl &= 3 << 5;
	m_ctl = (m_ctl >> 5) * 2;
	if(m_ctl){
		printk("m_ctl depends on hardware.In this version it must be 0.if not,MUST NOTICE!\n");
	}

	/* set LDO1---LDO20 and LDO AUDIO, 21 LDOs in all. */
	for(ldo_counter = 0; ldo_counter < 21; ldo_counter++){
		/* read the LDOs' enable bit */
		if(err = d2041_reg_read(d2041_regl_info, ldo_counter + ldo_offset,&ldo_enable_tmp)){
			printk("ldo_suspend i2c read ENABLE error.\n");
			return err;
		}
		ldo_enable_tmp &= 1<<6;
		/* according the LDOs' bit to set the MCTL bits */
		if(ldo_enable_tmp){
			d2041_reg_write(d2041_regl_info, ldo_counter + mctl_offset, MCTL_SLEEP << m_ctl);
		}else {
			d2041_reg_write(d2041_regl_info, ldo_counter + mctl_offset, MCTL_OFF << m_ctl);
		}
		//printk("reg = 0x%x, is_enalbed = %x \n",ldo_counter + ldo_offset,ldo_enable_tmp);

		/* add the ldo_offset when the next is LDO_13,according to the DA9024 datasheet.*/
		if(ldo_counter == 11){
			ldo_offset += 4;
		}
	}
	//BUCKs must be enabled
	d2041_reg_write(d2041_regl_info, D2041_BUCK1_MCTL_REG, MCTL_ON << m_ctl);
	d2041_reg_write(d2041_regl_info, D2041_BUCK2_MCTL_REG, MCTL_ON << m_ctl);
	d2041_reg_write(d2041_regl_info, D2041_BUCK4_MCTL_REG, MCTL_ON << m_ctl);
	//according to the DA9024 datasheet	this bit MCTL_EN must be set,but if set it, the sys will out of control.
	d2041_reg_read(d2041_regl_info,D2041_POWERCONT_REG, &val);
	val &= 0xFE;
	d2041_reg_write(d2041_regl_info, D2041_POWERCONT_REG, val);

	return 0;
}

int d2041_ldo_resume(struct platform_device *pdev)
{
	unsigned char val;
	d2041_reg_read(d2041_regl_info,D2041_POWERCONT_REG, &val);
	val &= 0xFE;
	d2041_reg_write(d2041_regl_info, D2041_POWERCONT_REG, val);
	return 0;
}

static struct i2c_driver d2041_i2c_driver = {
	.driver = {
		.name = D2041_I2C,
		.owner = THIS_MODULE,
	},
	.probe = d2041_i2c_probe,
	.remove = d2041_i2c_remove,
	.id_table = d2041_i2c_id,
	.suspend = d2041_ldo_suspend,
	.resume = d2041_ldo_resume,
};

static int __init d2041_i2c_init(void)
{
	return i2c_add_driver(&d2041_i2c_driver);
}

/* Initialised very early during bootup (in parallel with Subsystem init) */
subsys_initcall_sync(d2041_i2c_init);

static void __exit d2041_i2c_exit(void)
{
	i2c_del_driver(&d2041_i2c_driver);
}
module_exit(d2041_i2c_exit);

MODULE_AUTHOR("Dialog Semiconductor Ltd <divyang.patel@diasemi.com>"
		"hfwang@ingenic.cn");
MODULE_DESCRIPTION("I2C MFD driver for Dialog D2041 PMIC plus Audio");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" D2041_I2C);
