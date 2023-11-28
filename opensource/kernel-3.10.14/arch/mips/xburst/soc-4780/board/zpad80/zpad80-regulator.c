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
#include <gpio.h>

/**
 * Fixed voltage Regulators.
 * GPIO silulator regulators. Everyone is an independent device.
 */

/* FIXME! when board fixed, remove it */
FIXED_REGULATOR_DEF(
	zpad80_vwifi,
	"Wi-Fi",	3300000,	GPIO_PD(8),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,	"vwifi",	NULL);

FIXED_REGULATOR_DEF(
	zpad80_vtsc,
	"Touch Screen",	3300000,	GPIO_PE(1),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,	"vtsc",	NULL);

FIXED_REGULATOR_DEF(
	zpad80_vbus,
	"OTG-Vbus",	5000000,        GPIO_PE(10),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,	"vbus",	NULL);

FIXED_REGULATOR_DEF(
	zpad80_vcim,
	"Camera",	2800000,		GPIO_PB(27),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vcim",		"jz-cim");

FIXED_REGULATOR_DEF(
	zpad80_vlcd,
	"vlcd",		3300000,	GPIO_PB(23),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vlcd",		NULL);

FIXED_REGULATOR_DEF(
	zpad80_vlcd_vcom,
	"vlcd_vcom",        3300000,	GPIO_PF(11),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,       "vlcd_vcom",        NULL);

FIXED_REGULATOR_DEF(
	zpad80_vlcd_vbacklight,
	"vlcd_vbacklight",        3300000,	GPIO_PF(10),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,       "vlcd_vbacklight",        NULL);

FIXED_REGULATOR_DEF(
	zpad80_vpower_en,
	"vpower_en",		3300000,	GPIO_PF(9),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vpower_en",		NULL);


static struct platform_device *fixed_regulator_devices[] __initdata = {
	&zpad80_vtsc_regulator_device,
	&zpad80_vwifi_regulator_device,
	&zpad80_vbus_regulator_device,
	&zpad80_vcim_regulator_device,
	&zpad80_vlcd_regulator_device,
	&zpad80_vlcd_vcom_regulator_device,
	&zpad80_vlcd_vbacklight_regulator_device,
	&zpad80_vpower_en_regulator_device,
};

static int __init zpad80_pmu_dev_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fixed_regulator_devices); i++)
		fixed_regulator_devices[i]->id = i;

	return platform_add_devices(fixed_regulator_devices,
				    ARRAY_SIZE(fixed_regulator_devices));
}

subsys_initcall_sync(zpad80_pmu_dev_init);
