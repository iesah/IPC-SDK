/* Description: Bluetooth power driver with rfkill interface and bluetooth
 *	host wakeup support.
 *
 * Modified by Sun Jiwei <jwsun@ingenic.cn>
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/bt-rfkill.h>
#include <linux/delay.h>

#ifdef DEBUG
#define	DBG_MSG(fmt, ...)	printk(fmt, ##__VA_ARGS__)
#else
#define DBG_MSG(fmt, ...)
#endif

#define DEV_NAME		"bt_power"
#define RFKILL_STATE_SOFT_BLOCKED	0
#define RFKILL_STATE_UNBLOCKED		1

int bt_power_state = 0;

extern int bluesleep_start(void);
extern void bluesleep_stop(void);
extern int bcm_power_down(void);
extern int bcm_power_on(void);

struct bt_power {
	int bt_rst_n ;
	bool first_called;
	int bt_reg_on;

	struct bt_rfkill_platform_data *pdata;

	struct mutex bt_power_lock;
};

static void bt_enable_power(struct bt_power *bt_power)
{
	if(bt_power->bt_reg_on != -1){
		gpio_direction_output(bt_power->bt_reg_on, 1);
	} else {
		pr_warn("%s bt_reg_on can not be defined to -1\n", __func__);
	}
}

static void bt_disable_power(struct bt_power *bt_power)
{
	if(bt_power->bt_reg_on != -1){
		gpio_direction_output(bt_power->bt_reg_on, 0);
	} else {
		pr_warn("%s bt_reg_on can not be defined to -1\n", __func__);
	}
}

static int bt_power_control(struct bt_power *bt_power, int enable)
{
	if (enable == bt_power_state)
		return 0;

	switch (enable)	{
	case RFKILL_STATE_SOFT_BLOCKED:
		/*bluesleep_stop();*/
		/*mdelay(15);*/

		bcm_power_down();
		bt_disable_power(bt_power);

		if (bt_power->pdata->set_pin_status != NULL){
			(*bt_power->pdata->set_pin_status)(enable);
		} else {
			pr_warn("%s set_pin_status is not defined\n", __func__);
		}
		break;
	case RFKILL_STATE_UNBLOCKED:
		if (bt_power->pdata->restore_pin_status != NULL){
			(*bt_power->pdata->restore_pin_status)(enable);
		} else {
			pr_warn("%s set_pin_status is not defined\n", __func__);
		}

		bcm_power_on();
		if (bt_power->bt_rst_n != -1) {
			gpio_direction_output(bt_power->bt_rst_n, 0);
		} else {
			/*pr_warn("%s bt_rst_n can not be defined to -1\n", __func__);*/
		}

		bt_enable_power(bt_power);

		mdelay(15);

		if (bt_power->bt_rst_n != -1) {
			gpio_set_value(bt_power->bt_rst_n, 1);
		} else {
			/*pr_warn("%s bt_rst_n can not be defined to -1\n", __func__);*/
		}

		/*mdelay(15);*/
		/*bluesleep_start();*/
		break;
	default:
		break;
	}

	bt_power_state = enable;

	return 0;
}


static int bt_rfkill_set_block(void *data, bool blocked)
{
	int ret;
	struct bt_power *bt_power = (struct bt_power *)data;

	if (!bt_power->first_called) {
		mutex_lock(&bt_power->bt_power_lock);
		ret = bt_power_control(bt_power, blocked ? 0 : 1);
		mutex_unlock(&bt_power->bt_power_lock);
	} else {
		bt_power->first_called = false;
		return 0;
	}

	return ret;
}

static const struct rfkill_ops bt_rfkill_ops = {
	.set_block = bt_rfkill_set_block,
};

static int bt_power_rfkill_probe(struct platform_device *pdev, struct bt_power *bt_power)
{
	struct bt_rfkill_platform_data *pdata = pdev->dev.platform_data;
	int ret = -ENOMEM;


	pdata->rfkill = rfkill_alloc("bluetooth", &pdev->dev, RFKILL_TYPE_BLUETOOTH,
                            &bt_rfkill_ops, bt_power);

	if (!pdata->rfkill) {
		goto exit;
	}
	ret = rfkill_register(pdata->rfkill);
	if (ret) {
		rfkill_destroy(pdata->rfkill);
		return ret;
	}
exit:
	return ret;
}

static void bt_power_rfkill_remove(struct bt_rfkill_platform_data *pdata)
{

	if (pdata->rfkill)
		rfkill_unregister(pdata->rfkill);
}

static int __init_or_module bt_power_probe(struct platform_device *pdev)
{
	struct bt_rfkill_platform_data *pdata = pdev->dev.platform_data;
	struct bt_power *bt_power;
	int ret = 0;

	pdata = pdev->dev.platform_data;

	bt_power = kzalloc(sizeof(struct bt_power), GFP_KERNEL);
	if (!bt_power) {
		dev_err(&pdev->dev, "Failed to alloc memory for bt_power\n");
		ret = -ENOMEM;
		goto ERR1;
	}

	bt_power->first_called = true;

	bt_power->pdata = pdata;
	ret = bt_power_rfkill_probe(pdev, bt_power);
	if (ret) {
		dev_err(&pdev->dev, "Failed to probe bluetooth rfkill\n");
		ret = -ENOMEM;
		goto ERR2;
	}

	if (pdata->gpio.bt_rst_n >= 0) {
		bt_power->bt_rst_n = pdata->gpio.bt_rst_n;
	} else
		bt_power->bt_rst_n = -1;

	if (bt_power->bt_rst_n != -1) {
		ret = gpio_request(bt_power->bt_rst_n, "bt_rst_n");
		if(unlikely(ret)){
			dev_err(&pdev->dev, "Failed to request gpio for bt_rst_n\n");
			ret = -EBUSY;
			goto ERR3;
		}
	}

	if (pdata->gpio.bt_reg_on >= 0) {
		bt_power->bt_reg_on = pdata->gpio.bt_reg_on;
	} else {
		bt_power->bt_reg_on = -1;
	}

	if (bt_power->bt_reg_on != -1) {
		ret = gpio_request(bt_power->bt_reg_on, "bt_reg_on");
		if(unlikely(ret)){
			dev_err(&pdev->dev, "Failed to request gpio for bt_reg_on\n");
			ret = -EBUSY;
			goto ERR4;
		}
	}

	if(bt_power->bt_rst_n != -1){
		ret = gpio_direction_output(bt_power->bt_rst_n, 1);
		if (ret) {
			dev_err(&pdev->dev, "Failed to set gpio bt_rst_n to 1\n");
			ret = -EIO;
			goto ERR5;
		}
	}
	mutex_init(&bt_power->bt_power_lock);
	platform_set_drvdata(pdev, bt_power);

	return 0;
ERR5:
	gpio_free(bt_power->bt_reg_on);
ERR4:
	gpio_free(bt_power->bt_rst_n);
ERR3:
	bt_power_rfkill_remove(pdata);
ERR2:
	kfree(bt_power);
ERR1:
	return ret;
}

static int bt_power_remove(struct platform_device *pdev)
{
	int ret;
	struct bt_power *bt_power = platform_get_drvdata(pdev);

	mutex_lock(&bt_power->bt_power_lock);
	bt_power_state = 0;
	ret = bt_power_control(bt_power, bt_power_state);
	mutex_unlock(&bt_power->bt_power_lock);

	gpio_free(bt_power->bt_reg_on);
	gpio_free(bt_power->bt_rst_n);
	bt_power_rfkill_remove(bt_power->pdata);
	kfree(bt_power);
	return ret;
}


static struct platform_driver bt_power_driver = {
	.probe = bt_power_probe,
	.remove = bt_power_remove,
	.driver = {
		.name = DEV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init bt_power_init(void)
{
	int ret;

	ret = platform_driver_register(&bt_power_driver);

	return ret;
}

static void __exit bt_power_exit(void)
{
	platform_driver_unregister(&bt_power_driver);
}

module_init(bt_power_init);
module_exit(bt_power_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Bluetooth power control driver");
MODULE_VERSION("1.0");
