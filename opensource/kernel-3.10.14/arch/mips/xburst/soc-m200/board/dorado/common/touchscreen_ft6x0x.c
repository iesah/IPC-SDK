#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/tsc.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include "board_base.h"

static struct regulator *touch_power_vdd = NULL, *touch_power_vio = NULL;
static atomic_t touch_powered = ATOMIC_INIT(0);

int touch_power_init(struct device *dev)
{
	if (!touch_power_vdd) {
		touch_power_vdd = regulator_get(dev, "tp_vdd");
		if (IS_ERR(touch_power_vdd)) {
			pr_err("can not get regulator tp_vdd\n");
			goto err_vdd;
		}
	}

	if (!touch_power_vio) {
		touch_power_vio = regulator_get(dev, "tp_vio");
		if (IS_ERR(touch_power_vio)) {
			pr_err("can not get regulator tp_vio\n");
			goto err_vio;
		}
	}
	return 0;

err_vio:
	regulator_put(touch_power_vdd);
	touch_power_vdd = NULL;
	touch_power_vio = NULL;
err_vdd:
	return -ENODEV;
}
int touch_power_on(struct device *dev)
{
	int err = 0;
	if (!atomic_read(&touch_powered)) {
		if ((err = gpio_direction_output(GPIO_TP_WAKE, 1))) {
			printk(KERN_ERR "%s -> gpio_direction_output() failed err=%d\n", __func__, err);
		}

		if (!IS_ERR(touch_power_vdd)) {
			regulator_enable(touch_power_vdd);
		} else {
			err = -ENODEV;
			pr_err("touch power on vdd failed !\n");
			goto err_vdd;
		}

//		msleep(10);

		if (!IS_ERR(touch_power_vio)) {
			regulator_enable(touch_power_vio);
		} else {
			err = -ENODEV;
			pr_err("touch power on vio failed !\n");
			goto err_vio;
		}

//		msleep(10);

		atomic_set(&touch_powered, 1);
	}

	return 0;
err_vio:
	regulator_disable(touch_power_vdd);
err_vdd:
	return err;
}
int touch_power_off(struct device *dev)
{
	int err = 0;

	if (atomic_read(&touch_powered)) {
		if (!IS_ERR(touch_power_vio)) {
			regulator_disable(touch_power_vio);
		} else {
			pr_err("touch power off vio failed!\n");
		}

//		mdelay(5);
		if (!IS_ERR(touch_power_vdd)) {
			regulator_disable(touch_power_vdd);
		} else {
			pr_err("touch power off vdd failed!\n");
		}

		if ((err = gpio_direction_input(GPIO_TP_WAKE))) {
			printk(KERN_ERR "%s -> gpio_direction_output() failed err=%d\n", __func__, err);
		}
		atomic_set(&touch_powered, 0);
	}

	return err;
}
