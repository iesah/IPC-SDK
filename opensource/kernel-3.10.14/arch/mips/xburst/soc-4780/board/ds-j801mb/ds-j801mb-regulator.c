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
#include <linux/mfd/ricoh618.h>
#include <linux/i2c.h>
#include <irq.h>
#include <gpio.h>

/**
 * Core voltage Regulator.
 * Couldn't be modified.
 * Voltage was inited at bootloader.
 */
CORE_REGULATOR_DEF(
	ds_j801mb,	1000000,	1400000);

/**
 * I/O Regulator.
 * It's the parent regulator of most of devices regulator on board.
 * Voltage was inited at bootloader.
 */
IO_REGULATOR_DEF(
	ds_j801mb_vccio,
	"Vcc-IO",	3300000,	1);

/**
 * USB VBUS Regulators.
 * Switch of USB VBUS. It may be a actual or virtual regulator.
 */
VBUS_REGULATOR_DEF(
	ds_j801mb,	"Vcc-5v");

/**
 * Exclusive Regulators.
 * They are only used by one device each other.
 */
EXCLUSIVE_REGULATOR_DEF(
	ds_j801mb_vwifi,
	"Wi-Fi",	"vwifi",	NULL,
	NULL,		3300000,	0);

EXCLUSIVE_REGULATOR_DEF(
	ds_j801mb_vtsc,
	"Touch Screen",	"vtsc",		NULL,
	NULL,		3300000,	0);

EXCLUSIVE_REGULATOR_DEF(
	ds_j801mb_vgsensor,
	"G-sensor",	"vgsensor",	NULL,
	NULL,		3300000,	0);

EXCLUSIVE_REGULATOR_DEF(
	ds_j801mb_vcc5,
	"Vcc-5v",	"vhdmi",	NULL,
	"jz-hdmi",	5000000,	0);

/**
 * Fixed voltage Regulators.
 * GPIO silulator regulators. Everyone is an independent device.
 */
FIXED_REGULATOR_DEF(
	ds_j801mb_vcim,
	"Camera",	2800000,	GPIO_PB(27),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vcim",		"jz-cim");

FIXED_REGULATOR_DEF(
	ds_j801mb_vlcd,
	"LCD",		3300000,	GPIO_PE(9),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vlcd",		NULL);
#ifndef CONFIG_NAND_DRIVER
FIXED_REGULATOR_DEF(
	ds_j801mb_vmmc,
	"TF",		3300000,	GPIO_PF(19),
	LOW_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vmmc.2",	NULL);
#else
FIXED_REGULATOR_DEF(
	ds_j801mb_vmmc,
	"TF",		3300000,	GPIO_PF(19),
	LOW_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vmmc.0",	NULL);
#endif
static struct platform_device *fixed_regulator_devices[] __initdata = {
	&ds_j801mb_vcim_regulator_device,
	&ds_j801mb_vlcd_regulator_device,
	&ds_j801mb_vmmc_regulator_device,
};


/*
 * Regulators definitions used in PMU.
 *
 * If +5V is supplied by PMU, please define "VBUS" here with init_data NULL,
 * otherwise it should be supplied by a exclusive DC-DC, and you should define
 * it as a fixed regulator.
 */
static struct regulator_info ds_j801mb_pmu_regulators[] = {
	{"OUT1", &ds_j801mb_vcore_init_data},
	{"OUT3", &ds_j801mb_vccio_init_data},
	{"OUT4", &ds_j801mb_vcc5_init_data},
	{"OUT6", &ds_j801mb_vwifi_init_data},
	{"OUT7", &ds_j801mb_vtsc_init_data},
	{"OUT8", &ds_j801mb_vgsensor_init_data},
	{"VBUS", &ds_j801mb_vbus_init_data},
};

static struct charger_board_info charger_board_info = {
	.gpio	= -1,		//GPIO_PB(2),
	.enable_level	= LOW_ENABLE,
};

static struct pmu_platform_data ds_j801mb_pmu_pdata = {
	.gpio = GPIO_PA(28),
	.num_regulators = ARRAY_SIZE(ds_j801mb_pmu_regulators),
	.regulators = ds_j801mb_pmu_regulators,
	.charger_board_info = &charger_board_info,
};
#define PMU_I2C_BUSNUM 0

struct i2c_board_info ds_j801mb_pmu_board_info = {
	I2C_BOARD_INFO("act8600", 0x5a),
	.platform_data = &ds_j801mb_pmu_pdata,
};

static int __init ds_j801mb_pmu_dev_init(void)
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

	client = i2c_new_device(adap, &ds_j801mb_pmu_board_info);
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

subsys_initcall_sync(ds_j801mb_pmu_dev_init);
