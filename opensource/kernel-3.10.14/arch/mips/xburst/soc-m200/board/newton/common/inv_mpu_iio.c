#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/err.h>
#include <linux/inv_mpu.h>
#include <linux/delay.h>
#include "board_base.h"

static struct regulator *inv_mpu_power_vdd = NULL;
static struct regulator *inv_mpu_power_vio = NULL;
static atomic_t inv_mpu_powered = ATOMIC_INIT(0);
#define USE_INV_MPU_POWE_VIO_CTRL 0

static int inv_mpu_early_init(struct device *dev)
{
	int res;

	if ((res = gpio_request(GPIO_GSENSOR_INT, "mpuirq"))) {
		printk(KERN_ERR "%s -> gpio_request(%d) failed err=%d\n",__func__,GPIO_GSENSOR_INT, res);
		res = -EBUSY;
		goto err_gpio_request;
	}

	if ((res = gpio_direction_input(GPIO_GSENSOR_INT))) {
		printk(KERN_ERR "%s -> gpio_direction_input() failed err=%d\n",__func__,res);
		res = -EBUSY;
		goto err_gpio_set_input;
	}

	if (!inv_mpu_power_vdd) {
		inv_mpu_power_vdd = regulator_get(dev, "vcc_sensor3v3");
		if (IS_ERR(inv_mpu_power_vdd)) {
			pr_err("%s -> get regulator VDD failed\n",__func__);
			res = -ENODEV;
			goto err_vdd;
		}
	}

	if (USE_INV_MPU_POWE_VIO_CTRL && !inv_mpu_power_vio) {
		inv_mpu_power_vio = regulator_get(dev, "vcc_sensor1v8");
		if (IS_ERR(inv_mpu_power_vio)) {
			pr_err("%s -> get regulator VIO failed\n",__func__);
			res = -ENODEV;
			goto err_vio;
		}
	}

	return 0;
err_vio:
	regulator_put(inv_mpu_power_vdd);
	inv_mpu_power_vdd = NULL;
err_vdd:
	inv_mpu_power_vio = NULL;
err_gpio_set_input:
err_gpio_request:
	return res;
}

static int inv_mpu_exit(struct device *dev)
{
	if (inv_mpu_power_vdd != NULL && !IS_ERR(inv_mpu_power_vdd)) {
		regulator_put(inv_mpu_power_vdd);
	}

	if (inv_mpu_power_vio != NULL && !IS_ERR(inv_mpu_power_vio)) {
		regulator_put(inv_mpu_power_vio);
	}

	atomic_set(&inv_mpu_powered, 0);

	return 0;
}

static int inv_mpu_power_on(void)
{
	int res;
	if (!atomic_read(&inv_mpu_powered)) {
		if (!IS_ERR(inv_mpu_power_vdd)) {
			res = regulator_enable(inv_mpu_power_vdd);
		} else {
			pr_err("inv mpu VDD power unavailable!\n");
			res = -ENODEV;
			goto err_vdd;
		}

		if (inv_mpu_power_vio && !IS_ERR(inv_mpu_power_vio)) {
			res = regulator_enable(inv_mpu_power_vio);
		} else if(USE_INV_MPU_POWE_VIO_CTRL){
			pr_err("inv mpu VIO power unavailable!\n");
			res = -ENODEV;
			goto err_vio;
		}

		atomic_set(&inv_mpu_powered, 1);
	}
	return 0;
err_vio:
	regulator_disable(inv_mpu_power_vdd);
err_vdd:
	return res;
}

static int inv_mpu_power_off(void)
{
	int res;
	if (atomic_read(&inv_mpu_powered)) {
		if (inv_mpu_power_vio && !IS_ERR(inv_mpu_power_vio)) {
			regulator_disable(inv_mpu_power_vio);
		} else if(USE_INV_MPU_POWE_VIO_CTRL){
			pr_err("inv mpu VIO power unavailable!\n");
			res = -ENODEV;
			goto err_vio;
		}

		if (!IS_ERR(inv_mpu_power_vdd)) {
			regulator_disable(inv_mpu_power_vdd);
		} else {
			pr_err("inv mpu VDD power unavailable!\n");
			res = -ENODEV;
			goto err_vdd;
		}

		atomic_set(&inv_mpu_powered, 0);
	}

	return 0;
err_vio:
err_vdd:
	return res;
}

struct mpu_platform_data mpu9250_platform_data = {
        .int_config  = 0x90,
		.level_shifter = 0,
		.orientation = {
			0, -1,  0,
			-1,  0,  0,
			0,  0, -1,
		},
        .key = { 0xec, 0x5c, 0xa6, 0x17, 0x54, 0x3, 0x42, 0x90, 0x74, 0x7e,
                 0x3a, 0x6f, 0xc, 0x2c, 0xdd, 0xb },
#if !defined(CONFIG_SENSORS_AK09911)
		.sec_slave_type = SECONDARY_SLAVE_TYPE_COMPASS,
		.sec_slave_id   = COMPASS_ID_AK8963,
		.secondary_i2c_addr = 0x0C,
		.secondary_orientation = {
			-1, 0,  0,
			0, -1,  0,
			0,  0, 1,
		},
#endif
	.board_init = inv_mpu_early_init,
	.board_exit = inv_mpu_exit,
	.power_on = inv_mpu_power_on,
	.power_off = inv_mpu_power_off
};
