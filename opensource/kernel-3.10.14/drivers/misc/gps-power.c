/*
 *
 * Description:
 * 		Bluetooth power driver with rfkill interface and bluetooth host wakeup support.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rfkill.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>

#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#if 1
#define	DBG_MSG(fmt, ...)	printk(fmt, ##__VA_ARGS__)
#else
#define DBG_MSG(fmt, ...)
#endif

struct gps_ctrl
{
	struct rfkill *rfkill;
	struct regulator *power;
	struct mutex lock;
	bool poweron;
};

static struct gps_ctrl *gpsctrl = NULL;

static int gps_rfkill_set_block(void *data, bool blocked)
{
	mutex_lock(&gpsctrl->lock);
	if (blocked && gpsctrl->poweron) {
		printk("---------------- blocked = %d, regulator disable\n", blocked);
		regulator_disable(gpsctrl->power);
		gpsctrl->poweron = false;

	} else if (!gpsctrl->poweron) {
		printk("---------------- block = %d, regulator enable\n", blocked);
		regulator_enable(gpsctrl->power);
		gpsctrl->poweron = true;
	}
	mutex_unlock(&gpsctrl->lock);

	return 0;
}

static const struct rfkill_ops gps_rfkill_ops = {
	.set_block = gps_rfkill_set_block,
};

static int __init gps_power_init(void)
{
	int ret = 0;
	printk(" entry gps_power_init()\n");
	gpsctrl = kmalloc(sizeof(struct gps_ctrl), GFP_KERNEL);
	if (!gpsctrl) {
		ret = -ENOMEM;
		goto err_nomem;
	}

	gpsctrl->power = regulator_get(NULL, "vcc_gps_1v8");
	if (IS_ERR(gpsctrl->power)) {
		printk("gps regulator missing\n");
		ret = -EINVAL;
		goto err_nopower;
	}

	gpsctrl->rfkill = rfkill_alloc("gps", NULL, RFKILL_TYPE_GPS,
			&gps_rfkill_ops, NULL);

	if (!gpsctrl->rfkill) {
		ret = -ENOMEM;
		goto err_power;
	}

	mutex_init(&gpsctrl->lock);

	ret = rfkill_register(gpsctrl->rfkill);

	gpsctrl->poweron = false;
	printk(" gps init ok \n");
	return ret;

err_power:
	regulator_put(gpsctrl->power);
err_nopower:
	kfree(gpsctrl);
err_nomem:
	return ret;
}

static void __exit gps_power_exit(void)
{
	rfkill_unregister(gpsctrl->rfkill);
	kfree(gpsctrl->rfkill);
	regulator_put(gpsctrl->power);
	kfree(gpsctrl);
}

module_init(gps_power_init);
module_exit(gps_power_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Gps power control driver");
MODULE_VERSION("1.0");
