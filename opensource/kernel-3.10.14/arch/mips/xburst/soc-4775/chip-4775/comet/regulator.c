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

#include "board.h"

/**
 * Core voltage Regulator.
 * Couldn't be modified.
 * Voltage was inited at bootloader.
 */
CORE_REGULATOR_DEF(
	comet,	1000000,	1300000);

/**
 * I/O Regulator.
 * It's the parent regulator of most of devices regulator on board.
 * Voltage was inited at bootloader.
 */
IO_REGULATOR_DEF(
	vccio,
	"Vcc-IO",	3300000,	1);

/**
 * USB VBUS Regulators.
 * Switch of USB VBUS. It may be a actual or virtual regulator.
 */
VBUS_REGULATOR_DEF(
	comet,	"VCC5V");

/**
 * Exclusive Regulators.
 * They are only used by one device each other.
 */
EXCLUSIVE_REGULATOR_DEF(
	vwifi,
	"Wi-Fi",	"vwifi",	NULL,
	NULL,		3300000,	0);


EXCLUSIVE_REGULATOR_DEF(
	vcc5v,
	"VCC5V",	"vcc5v",	NULL,
	NULL,	5000000,	0);

EXCLUSIVE_REGULATOR_DEF(
	vlcd,
	"Vlcd",	"vlcd",		NULL,
	NULL,	3300000,	0);

#if 1
#ifndef CONFIG_NAND
FIXED_REGULATOR_DEF(
		vmmc,
		"TF",           3000000,        GPIO_PB(3),
		HIGH_ENABLE,    UN_AT_BOOT,     0,  
		NULL,           "vmmc.2",       NULL);
#else
FIXED_REGULATOR_DEF(
		vmmc,
		"TF",           3000000,        GPIO_PB(3),
		HIGH_ENABLE,    UN_AT_BOOT,     0,
		NULL,           "vmmc.0",       NULL);
#endif
FIXED_REGULATOR_DEF(
		vtsc,
		"Touch Screen",           3000000,       -1,
		HIGH_ENABLE,    UN_AT_BOOT,     0,  
		NULL,           "vtsc",       NULL);


static struct platform_device *fixed_regulator_devices[] __initdata = {
	&vmmc_regulator_device,
	&vtsc_regulator_device,
};
#endif
/*
 * Regulators definitions used in PMU.
 *
 * If +5V is supplied by PMU, please define "VBUS" here with init_data NULL,
 * otherwise it should be supplied by a exclusive DC-DC, and you should define
 * it as a fixed regulator.
 */
static struct regulator_info pmu_regulators[] = {
	{"OUT1", &comet_vcore_init_data},
	{"OUT3", &vccio_init_data},
	{"OUT4", &vcc5v_init_data},
	{"OUT6", &vwifi_init_data},
	{"OUT7", &vlcd_init_data},
	{"VBUS", &comet_vbus_init_data},
};

static struct charger_board_info charger_board_info = {
	.gpio	= -1,
	.enable_level	= LOW_ENABLE,
};

static struct pmu_platform_data pmu_pdata = {
	.gpio = GPIO_PB(21),
	.num_regulators = ARRAY_SIZE(pmu_regulators),
	.regulators = pmu_regulators,
	.charger_board_info = &charger_board_info,
};

#define PMU_I2C_BUSNUM 2

struct i2c_board_info pmu_board_info = {
	I2C_BOARD_INFO("act8600", 0x5a),
	.platform_data = &pmu_pdata,
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

	client = i2c_new_device(adap, &pmu_board_info);
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

struct cpufreq_frequency_table  freq_table[] = {
    {1,   400  * 1000},
    {2,   600  * 1000},
    {3,   720  * 1000},
    {4,   816  * 1000},
    {5,   912  * 1000},
    {6,   1008 * 1000},
    {7,   1200 * 1000},
    {8,   1344 * 1000},
    {9,   1440 * 1000},
    {10,   1536 * 1000},
    {11,  CPUFREQ_TABLE_END},
};
