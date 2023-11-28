/*
 * This code is for manager the power of BT, WiFi and NFC
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 *      http://www.ingenic.com
 *      Sun Jiwei<jwsun@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>

#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/version.h>
#include <linux/regulator/consumer.h>
#include <linux/bcm_pm_core.h>

struct bcm_power {
	struct mutex mutex;
	struct regulator *regulator;
	int power_en;
	int use_count;
	bool probe_success;
	void (*clk_enable)(void);
	void (*clk_disable)(void);
};

static struct bcm_power *bcm_power;

static void clk_32k_on(void)
{
	bcm_power->clk_enable();
}

static void clk_32k_off(void)
{
	bcm_power->clk_disable();
}

static int wlan_power_on(void)
{
	int ret = 0;
	if (-1 != bcm_power->power_en){
		gpio_direction_output(bcm_power->power_en, 1);
	}
	if (!IS_ERR(bcm_power->regulator)) {
		ret = regulator_enable(bcm_power->regulator);
	}

	return ret;
}

static int wlan_power_down(void)
{
	int ret = 0;
	if (-1 != bcm_power->power_en){
		gpio_direction_output(bcm_power->power_en, 0);
	}
	if (!IS_ERR(bcm_power->regulator)) {
		ret = regulator_disable(bcm_power->regulator);
	}

	return ret;
}

static int _bcm_power_on(void)
{
	int ret = 0;

	if (bcm_power->use_count < 0) {
		pr_err("%s, bcm power manager unbalanced, use_count is %d\n",
				__func__, bcm_power->use_count);
		return -EIO;
	}

	if (bcm_power->use_count == 0) {
		clk_32k_on();
		msleep(200);
		ret = wlan_power_on();
		if (ret < 0) {
			pr_err("%s, wlan power on failure\n", __func__);
			return -EIO;
		}
	}

	bcm_power->use_count++;

	return bcm_power->use_count;
}

static int _bcm_power_down(void)
{
	int ret = 0;

	if (bcm_power->use_count <= 0) {
		pr_err("%s, bcm power manager unbalanced, use_count is %d\n",
				__func__, bcm_power->use_count);
		return -EIO;
	}

	bcm_power->use_count--;

	if (bcm_power->use_count == 0) {
		ret = wlan_power_down();
		if (ret < 0) {
			pr_err("%s, wlan power on failure\n", __func__);
			return -EIO;
		}
		msleep(100);
		clk_32k_off();
	}

	return bcm_power->use_count;
}

int bcm_power_on(void)
{
	int ret = 0;

	if (!bcm_power)
		return -EIO;

	mutex_lock(&bcm_power->mutex);
	ret = _bcm_power_on();
	mutex_unlock(&bcm_power->mutex);

	if (ret > 0)
		ret = 0;

	return ret;
}
EXPORT_SYMBOL(bcm_power_on);

int bcm_power_down(void)
{
	int ret = 0;

	if (!bcm_power)
		return -EIO;

	mutex_lock(&bcm_power->mutex);
	ret = _bcm_power_down();
	mutex_unlock(&bcm_power->mutex);

	if (ret > 0)
		ret = 0;

	return ret;
}

EXPORT_SYMBOL(bcm_power_down);

static int bcm_power_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcm_power_platform_data *pdata;

	pdata = pdev->dev.platform_data;

	bcm_power = kzalloc(sizeof(struct bcm_power), GFP_KERNEL);
	if (!bcm_power) {
		dev_err(&pdev->dev, "Failed to alloc driver structure\n");
		return -ENOMEM;
	}

	bcm_power->clk_enable = pdata->clk_enable;
	bcm_power->clk_disable = pdata->clk_disable;
	bcm_power->power_en = pdata->wlan_pwr_en;
	if (-1 != bcm_power->power_en){
		ret = gpio_request(bcm_power->power_en, "bcm_power");
		if (ret) {
			kfree(bcm_power);
			bcm_power = NULL;
			return -ENODEV;
		}
		gpio_direction_output(bcm_power->power_en, 0);
		//	gpio_set_value(bcm_power->power_en, 0);
	}

	mutex_init(&bcm_power->mutex);

	bcm_power->regulator = regulator_get(NULL, "wifi_vddio_1v8");
	if (IS_ERR(bcm_power->regulator)) {
		pr_err("wifi regulator missing\n");
		/*ret = -EINVAL;*/
		/*goto ERR1;*/
	}

	bcm_power->probe_success = true;
	return 0;
ERR1:
	if (-1 != bcm_power->power_en){
		gpio_free(bcm_power->power_en);
	}
	bcm_power = NULL;
	kfree(bcm_power);
	return ret;
}

static int bcm_power_remove(struct platform_device *pdev)
{
	if (!IS_ERR(bcm_power->regulator))
		regulator_put(bcm_power->regulator);
	if (-1 != bcm_power->power_en){
		gpio_free(bcm_power->power_en);
	}
	kfree(bcm_power);

	return 0;
}

static struct platform_driver bcm_power_driver = {
	.driver = {
		.name = "bcm_power",
		.owner = THIS_MODULE,
	},
	.probe = bcm_power_probe,
	.remove = __exit_p(bcm_power_remove),
};

static int __init bcm_power_init(void)
{
	return platform_driver_register(&bcm_power_driver);
}

static int __exit bcm_power_exit(void)
{
	platform_driver_unregister(&bcm_power_driver);
}

module_init(bcm_power_init);
module_exit(bcm_power_exit);

MODULE_DESCRIPTION("BCM power manager core driver for NFC, BT and WIFI");
MODULE_AUTHOR("Sun Jiwei <jwsun@ingenic.cn>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("20140605");
