/*
 * [board]-pmu.c - This file defines PMU board information.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Author: Large Dipper <ykli@ingenic.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/pmu-common.h>
#include <linux/i2c.h>
#include <linux/cpufreq.h>
#include <gpio.h>
#include <linux/d2041/d2041_pmic.h>
#include <linux/d2041/d2041_core.h>
#include "board_base.h"

#define FIXED_REGULATOR_DEF(NAME, SNAME, MV, GPIO, EH, EB, DELAY, SREG, SUPPLY, DEV_NAME)\
static struct regulator_consumer_supply NAME##_regulator_consumer =			\
	REGULATOR_SUPPLY(SUPPLY, DEV_NAME);						\
											\
static struct regulator_init_data NAME##_regulator_init_data = {			\
	.supply_regulator = SREG,							\
	.constraints = {								\
		.name = SNAME,								\
		.min_uV = MV,								\
		.max_uV = MV,								\
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,				\
	},										\
	.num_consumer_supplies = 1,							\
	.consumer_supplies = &NAME##_regulator_consumer,				\
};											\
											\
static struct fixed_voltage_config NAME##_regulator_data = {				\
	.supply_name = SNAME,								\
	.microvolts = MV,								\
	.gpio = GPIO,									\
	.enable_high = EH,								\
	.enabled_at_boot = EB,								\
	.startup_delay = DELAY,								\
	.init_data = &NAME##_regulator_init_data,					\
};											\
											\
static struct platform_device NAME##_regulator_device = {				\
	.name = "reg-fixed-voltage",							\
	.id = -1,									\
	.dev = {									\
		.platform_data = &NAME##_regulator_data,				\
	}										\
}

#if defined(CONFIG_MFD_DA9024)
static struct regulator_consumer_supply d2041_consumer_supply[D2041_NUMBER_OF_REGULATORS] = {
	[D2041_BUCK_1] = REGULATOR_SUPPLY("cpu_core", NULL),
	[D2041_BUCK_2] = REGULATOR_SUPPLY("cpu_vmema", NULL),
	[D2041_BUCK_3] = REGULATOR_SUPPLY("cpu_mem12", NULL),
	[D2041_BUCK_4] = REGULATOR_SUPPLY("cpu_vddio", NULL),
	[D2041_LDO_2 ] = REGULATOR_SUPPLY("cpu_vddio2", NULL),
	[D2041_LDO_3 ] = REGULATOR_SUPPLY("cpu_2.5v", NULL),
	[D2041_LDO_4 ] = REGULATOR_SUPPLY("cpu_avdd", NULL),
	[D2041_LDO_8 ] = REGULATOR_SUPPLY("cpu_pll25", NULL),
	[D2041_LDO_9 ] = REGULATOR_SUPPLY("dsi_avdd", NULL),
	[D2041_LDO_10] = REGULATOR_SUPPLY("csi_avdd", NULL),
	[D2041_LDO_11] = REGULATOR_SUPPLY("cpu_vdllo", NULL),
	[D2041_LDO_12] = REGULATOR_SUPPLY("v33", NULL),
	[D2041_LDO_13] = REGULATOR_SUPPLY("lcd_1.8v", NULL),
	[D2041_LDO_16] = REGULATOR_SUPPLY("vcc_sensor3v3", NULL),
	[D2041_LDO_17] = REGULATOR_SUPPLY("vcc_sensor1v8", NULL),
	[D2041_LDO_19] = REGULATOR_SUPPLY("wifi_vddio_18", NULL),
	[D2041_LDO_AUD] = REGULATOR_SUPPLY("vrtc25", NULL),
};

static struct regulator_init_data d2041_reg_init_data[D2041_NUMBER_OF_REGULATORS] = {
	[D2041_BUCK_1] = {
		.constraints = {
			.name = "BUCK_1",
			.min_uV = 1100 * 1000,
			.max_uV = 1100 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_BUCK_1],
	},

	[D2041_BUCK_2] = {
		.constraints = {
			.name = "BUCK_2",
			.min_uV = 1200 * 1000,
			.max_uV = 1200 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_BUCK_2],
	},

	[D2041_BUCK_3] = {
		.constraints = {
			.name = "BUCK_3",
			.min_uV = 1200 * 1000,
			.max_uV = 1200 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_BUCK_3],
	},

	[D2041_BUCK_4] = {
		.constraints = {
			.name = "BUCK_4",
			.min_uV = 1800 * 1000,
			.max_uV = 1800 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_BUCK_4],
	},

	[D2041_LDO_2] = {
		.constraints = {
			.name = "LDO_2",
			.min_uV = 1800 * 1000,
			.max_uV = 1800 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_2],
	},

	[D2041_LDO_3] = {
		.constraints = {
			.name = "LDO_3",
			.min_uV = 2500 * 1000,
			.max_uV = 2500 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_3],
	},

	[D2041_LDO_4] = {
		.constraints = {
			.name = "LDO_4",
			.min_uV = 2800 * 1000,
			.max_uV = 2800 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 0,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_4],
	},

	[D2041_LDO_8] = {
		.constraints = {
			.name = "LDO_8",
			.min_uV = 2500 * 1000,
			.max_uV = 2500 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_8],
	},

	[D2041_LDO_9] = {
		.constraints = {
			.name = "LDO_9",
			.min_uV = 2500 * 1000,
			.max_uV = 2500 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_9],
	},

	[D2041_LDO_10] = {
		.constraints = {
			.name = "LDO_10",
			.min_uV = 2500 * 1000,
			.max_uV = 2500 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_10],
	},


	[D2041_LDO_11] = {
		.constraints = {
			.name = "LDO_11",
			.min_uV = 3300 * 1000,
			.max_uV = 3300 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_10],
	},

	[D2041_LDO_12] = {
		.constraints = {
			.name = "LDO_12",
			.min_uV = 3300 * 1000,
			.max_uV = 3300 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_12],
	},

	[D2041_LDO_13] = {
		.constraints = {
			.name = "LDO_13",
			.min_uV = 1800 * 1000,
			.max_uV = 1800 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 0,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_13],
	},

	[D2041_LDO_16] = {
		.constraints = {
			.name = "LDO_16",
			.min_uV = 3300 * 1000,
			.max_uV = 3300 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_16],
	},

	[D2041_LDO_17] = {
		.constraints = {
			.name = "LDO_17",
			.min_uV = 1800 * 1000,
			.max_uV = 1800 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_17],
	},

	[D2041_LDO_19] = {
		.constraints = {
			.name = "LDO_19",
			.min_uV = 1800 * 1000,
			.max_uV = 1800 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 0,
			.boot_on = 0,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_19],
	},

	[D2041_LDO_AUD] = {
		.constraints = {
			.name = "LDO_AUD",
			.min_uV = 1800 * 1000,
			.max_uV = 1800 * 1000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.always_on = 1,
			.boot_on = 1,
			.apply_uV = 1,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &d2041_consumer_supply[D2041_LDO_AUD],
	},
};

static int d2041_board_irq_init(struct d2041 *d2041)
{
	int res;
	if ((res = gpio_request(GPIO_PMU_IRQ, "d2041_irq"))) {
		printk(KERN_ERR "gpio_request(%d) failed err=%d\n",GPIO_GSENSOR_INT1, res);
		goto done;
	}

	if ((res = gpio_direction_input(GPIO_PMU_IRQ))) {
		printk(KERN_ERR "gpio_direction_input() failed err=%d\n",res);
		goto done;
	}

	return 0;
done:
	return res;

}

struct d2041_platform_data d2041_platform_pdata = {
	.irq_init = d2041_board_irq_init,
	.reg_init_data = {
		[D2041_BUCK_1] = &d2041_reg_init_data[D2041_BUCK_1],
		[D2041_BUCK_2] = &d2041_reg_init_data[D2041_BUCK_2],
		[D2041_BUCK_3] = &d2041_reg_init_data[D2041_BUCK_3],
		[D2041_BUCK_4] = &d2041_reg_init_data[D2041_BUCK_4],
		[D2041_LDO_2 ] = &d2041_reg_init_data[D2041_LDO_2],
		[D2041_LDO_3 ] = &d2041_reg_init_data[D2041_LDO_3],
		[D2041_LDO_4 ] = &d2041_reg_init_data[D2041_LDO_4],
		[D2041_LDO_8 ] = &d2041_reg_init_data[D2041_LDO_8],
		[D2041_LDO_9 ] = &d2041_reg_init_data[D2041_LDO_9],
		[D2041_LDO_10] = &d2041_reg_init_data[D2041_LDO_10],
		[D2041_LDO_11] = &d2041_reg_init_data[D2041_LDO_11],
		[D2041_LDO_12] = &d2041_reg_init_data[D2041_LDO_12],
		[D2041_LDO_13] = &d2041_reg_init_data[D2041_LDO_13],
		[D2041_LDO_16] = &d2041_reg_init_data[D2041_LDO_16],
		[D2041_LDO_17] = &d2041_reg_init_data[D2041_LDO_17],
		[D2041_LDO_19] = &d2041_reg_init_data[D2041_LDO_19],
		[D2041_LDO_AUD] = &d2041_reg_init_data[D2041_LDO_AUD],
	},
};
#endif

//FIXED_REGULATOR_DEF(usi_vbat, "USI_VDD", 3900*1000 ,GPIO_WLAN_PW_EN, 1, 0, 0, NULL,"usi_vbat", NULL);

static struct platform_device *fixed_regulator_devices[] __initdata = {
	//&usi_vbat_regulator_device,
};

#define PMU_I2C_BUSNUM 1

struct i2c_board_info pmic_board_info = {
	I2C_BOARD_INFO(D2041_I2C, 0x49),
	.irq = (IRQ_GPIO_BASE + GPIO_PMU_IRQ),
	.platform_data = &d2041_platform_pdata,
};

static int __init pmu_dev_init(void)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	int busnum = PMU_I2C_BUSNUM;
	int i;

	adap = i2c_get_adapter(busnum);
	if (!adap) {
		pr_err("failed to get adapter i2c%d\n", busnum);
		return -1;
	}

	client = i2c_new_device(adap, &pmic_board_info);
	if (!client) {
		pr_err("failed to register pmu to i2c%d\n", busnum);
		return -1;
	}

	i2c_put_adapter(adap);

	for (i = 0; i < ARRAY_SIZE(fixed_regulator_devices); i++)
		fixed_regulator_devices[i]->id = i;

	return platform_add_devices(fixed_regulator_devices,
				    ARRAY_SIZE(fixed_regulator_devices));
}

subsys_initcall_sync(pmu_dev_init);
