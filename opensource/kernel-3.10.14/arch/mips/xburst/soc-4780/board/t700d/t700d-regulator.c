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

/**
 * Core voltage Regulator.
 * Couldn't be modified.
 * Voltage was inited at bootloader.
 */
CORE_REGULATOR_DEF(
	t700d,	1000000,	1400000);

/**
 * I/O Regulator.
 * It's the parent regulator of most of devices regulator on board.
 * Voltage was inited at bootloader.
 */
IO_REGULATOR_DEF(
	t700d_vccio,
	"Vcc-IO",	3000000,	1);

/**
 * USB VBUS Regulators.
 * Switch of USB VBUS. It may be a actual or virtual regulator.
 */
VBUS_REGULATOR_DEF(
	t700d,	"VCC5V");

/**
 * Exclusive Regulators.
 * They are only used by one device each other.
 */
EXCLUSIVE_REGULATOR_DEF(
	t700d_vwifi,
	"Wi-Fi",	"vwifi",	NULL,
	NULL,		3000000,	1);

EXCLUSIVE_REGULATOR_DEF(
	t700d_vtsc,
	"Touch Screen",	"vtsc",		NULL,
	NULL,		3000000,	1);
/*
EXCLUSIVE_REGULATOR_DEF(
	t700d_vgsensor,
	"G-sensor",	"vgsensor",
	NULL,		NULL,		3000000);
*/

EXCLUSIVE_REGULATOR_DEF(
	t700d_vcc5v,
	"VCC5V",	"vcc5v",	NULL,
	NULL,		5000000,	0);

EXCLUSIVE_REGULATOR_DEF(
	t700d_vlcd,
	"Vlcd",		"vlcd",		NULL,
	NULL,		3000000,	0);

/**
 * Fixed voltage Regulators.
 * GPIO silulator regulators. Everyone is an independent device.
 */
FIXED_REGULATOR_DEF(
	t700d_vhdmi,
	"Vcc5V",	5000000,	-EINVAL,
	0,		UN_AT_BOOT,	0,
	NULL,		"vhdmi",	"jz-hdmi");

#if 0
FIXED_REGULATOR_DEF(
	t700d_vbus,
	"OTG-Vbus",	5000000,	GPIO_PE(10),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	"Vcc-5V",	"vdrvvbus",	NULL);
#endif

FIXED_REGULATOR_DEF(
	t700d_vgsensor,
	"G-sensor",	3000000,	GPIO_PE(9),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vgsensor",	"gsensor_mma8452");

FIXED_REGULATOR_DEF(
	t700d_vcim,
	"Camera",	2800000,	GPIO_PB(27),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vcim",		"jz-cim");
#if 0
FIXED_REGULATOR_DEF(
	t700d_vlcd,
	"LCD",		3000000,	GPIO_PB(23),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vlcd",		NULL);
#endif
static struct platform_device *fixed_regulator_devices[] __initdata = {
	&t700d_vhdmi_regulator_device,
	&t700d_vgsensor_regulator_device,
	//&t700d_vbus_regulator_device,
	&t700d_vcim_regulator_device,
	//&t700d_vlcd_regulator_device,
};

/*
 * Regulators definitions used in PMU.
 *
 * If +5V is supplied by PMU, please define "VBUS" here with init_data NULL,
 * otherwise it should be supplied by a exclusive DC-DC, and you should define
 * it as a fixed regulator.
 */
static struct regulator_info t700d_pmu_regulators[] = {
	{"OUT1", &t700d_vcore_init_data},
	{"OUT3", &t700d_vccio_init_data},
	{"OUT4", &t700d_vcc5v_init_data},
	{"OUT6", &t700d_vwifi_init_data},
	{"OUT7", &t700d_vtsc_init_data},
//	{"OUT8", &t700d_vgsensor_init_data},
	{"OUT8", &t700d_vlcd_init_data},
	{"VBUS", &t700d_vbus_init_data},
};

static struct charger_board_info charger_board_info = {
	.gpio	= GPIO_PB(2),
	.enable_level	= LOW_ENABLE,
};

static struct pmu_platform_data t700d_pmu_pdata = {
	.gpio = GPIO_PA(28),
	.num_regulators = ARRAY_SIZE(t700d_pmu_regulators),
	.regulators = t700d_pmu_regulators,
	.charger_board_info = &charger_board_info,
};

#define PMU_I2C_BUSNUM 0

struct i2c_board_info t700d_pmu_board_info = {
	I2C_BOARD_INFO("act8600", 0x5a),
	.platform_data = &t700d_pmu_pdata,
};

static int __init t700d_pmu_dev_init(void)
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

	client = i2c_new_device(adap, &t700d_pmu_board_info);
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

subsys_initcall_sync(t700d_pmu_dev_init);

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
