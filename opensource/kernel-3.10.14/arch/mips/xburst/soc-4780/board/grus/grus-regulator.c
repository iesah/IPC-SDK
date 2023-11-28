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
	grus,	1000000,	1400000);

/**
 * I/O Regulator.
 * It's the parent regulator of most of devices regulator on board.
 * Voltage was inited at bootloader.
 */
IO_REGULATOR_DEF(
	grus_vccio,
	"Vcc-IO",	3300000,	1);

#ifdef CONFIG_BOARD_GRUS_V_1_0_1
VBUS_V101_REGULATOR_DEF(
	grus);
#endif
/**
 * USB VBUS Regulators.
 * Switch of USB VBUS. It may be a actual or virtual regulator.
 */
#ifndef CONFIG_BOARD_GRUS_V_1_0_1
VBUS_REGULATOR_DEF(
	grus,	"Vcc-5v");
#endif

/**
 * Exclusive Regulators.
 * They are only used by one device each other.
 */
EXCLUSIVE_REGULATOR_DEF(
	grus_vwifi,
	"Wi-Fi",	"vwifi",	NULL,
	NULL,		3300000,	0);

EXCLUSIVE_REGULATOR_DEF(
	grus_vtsc,
	"Touch Screen",	"vtsc",		NULL,
	NULL,		3300000,	0);

EXCLUSIVE_REGULATOR_DEF(
	grus_vgsensor,
	"G-sensor",	"vgsensor",	NULL,
	NULL,		3300000,	0);

EXCLUSIVE_REGULATOR_DEF(
	grus_vcc5,
	"Vcc-5v",	"vhdmi",	NULL,
	"jz-hdmi",	5000000,	0);

/**
 * Fixed voltage Regulators.
 * GPIO silulator regulators. Everyone is an independent device.
 */
FIXED_REGULATOR_DEF(
	grus_vcim,
	"Camera",	2800000,	GPIO_PB(27),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vcim",		"jz-cim");

FIXED_REGULATOR_DEF(
	grus_vlcd,
	"LCD",		3300000,	GPIO_PE(9),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vlcd",		NULL);
#ifndef CONFIG_NAND_DRIVER
FIXED_REGULATOR_DEF(
	grus_vmmc,
	"TF",		3300000,	GPIO_PF(19),
	LOW_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vmmc.2",	NULL);
#else
FIXED_REGULATOR_DEF(
	grus_vmmc,
	"TF",		3300000,	GPIO_PF(19),
	LOW_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vmmc.0",	NULL);
#endif
static struct platform_device *fixed_regulator_devices[] __initdata = {
	&grus_vcim_regulator_device,
	&grus_vlcd_regulator_device,
	&grus_vmmc_regulator_device,
};

#ifdef CONFIG_BOARD_GRUS_V_1_0_1
/***********************************************************\
| * if you will use ricoh618 pmu, you must use its battery *|
| * detect software to ganerate a *init.h file,            *|
| * and then add it to here.                               *|
\ * ********************************************************/
uint8_t battery_init_parameter[32] = {
/* 0820 4.2v battery */
	0x0B, 0xD0, 0x0B, 0xF2, 0x0C, 0x0A, 0x0C, 0x14, 0x0C, 0x27, 0x0C, 0x4D, 0x0C, 0x78, 0x0C, 0xA3,
	0x0C, 0xD0, 0x0D, 0x0F, 0x0D, 0x5A, 0x08, 0x2F, 0x00, 0x2B, 0x0F, 0xC8, 0x05, 0x2A, 0x22, 0x56
};
#endif

/*
 * Regulators definitions used in PMU.
 *
 * If +5V is supplied by PMU, please define "VBUS" here with init_data NULL,
 * otherwise it should be supplied by a exclusive DC-DC, and you should define
 * it as a fixed regulator.
 */
#ifndef CONFIG_BOARD_GRUS_V_1_0_1
static struct regulator_info grus_pmu_regulators[] = {
	{"OUT1", &grus_vcore_init_data},
	{"OUT3", &grus_vccio_init_data},
	{"OUT4", &grus_vcc5_init_data},
	{"OUT6", &grus_vwifi_init_data},
	{"OUT7", &grus_vtsc_init_data},
	{"OUT8", &grus_vgsensor_init_data},
	{"VBUS", &grus_vbus_init_data},
};

static struct charger_board_info charger_board_info = {
	.gpio	= -1,		//GPIO_PB(2),
	.enable_level	= LOW_ENABLE,
};

static struct pmu_platform_data grus_pmu_pdata = {
	.gpio = GPIO_PA(28),
	.num_regulators = ARRAY_SIZE(grus_pmu_regulators),
	.regulators = grus_pmu_regulators,
	.charger_board_info = &charger_board_info,
};
#else
static struct regulator_info grus_pmu_regulators[] = {
	{"DC1", &grus_vcore_init_data},
	{"DC3", &grus_vccio_init_data},
	{"LDO2", &grus_vwifi_init_data},
	{"LDO3", &grus_vtsc_init_data},
	{"LDO4", &grus_vgsensor_init_data},
	{"VBUS", &grus_vbus_init_data},
};

static struct ricoh618_platform_data ricoh618_private = {
	.gpio_base = -1,
	.irq_base = IRQ_RESERVED_BASE,
};

static struct ricoh618_battery_platform_data ricoh618_bat_private = {
	.alarm_vol_mv = 3000,
	.monitor_time = 60,
	.ch_vfchg 	= 0x3,	/* VFCHG	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */ // full voltage
	.ch_vrchg 	= 0x4,	/* VRCHG	= 0 - 4 (3.85v, 3.90v, 3.95v, 4.00v, 4.10v) */ //charge back
	.ch_vbatovset 	= 0,	 /*VBATOVSET	= 0 or 1 (0 : 4.38v(up)/3.95v(down) 1: 4.53v(up)/4.10v(down)) ????*/
	.ch_ichg 	= 0x0E,	/* ICHG		= 0 - 0x1D (100mA - 3000mA) */
	.ch_ilim_adp 	= 0x18,	/* ILIM_ADP	= 0 - 0x1D (100mA - 3000mA) */
	.ch_ilim_usb 	= 0x04,	/* ILIM_USB	= 0 - 0x1D (100mA - 3000mA) */
	.ch_icchg 	= 0x03,	/* ICCHG	= 0 - 3 (50mA 100mA 150mA 200mA) */
	.fg_target_vsys = 0,	/* This value is the target one to DSOC=0% */
	.fg_target_ibat = 0, /* This value is the target one to DSOC=0% */
	.jt_en 		= 0,	/* JEITA Enable	  = 0 or 1 (1:enable, 0:disable) */
	.jt_hw_sw 	= 1,	/* JEITA HW or SW = 0 or 1 (1:HardWare, 0:SoftWare) */
	.jt_temp_h 	= 50,	/* degree C */
	.jt_temp_l 	= 12,	/* degree C */
	.jt_vfchg_h 	= 0x03,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
	.jt_vfchg_l 	= 0,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
	.jt_ichg_h 	= 0x0D,	/* VFCHG Low  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
	.jt_ichg_l 	= 0x09,	/* ICHG Low   	= 0 - 0x1D (100mA - 3000mA) */

/*  JEITA Parameter
*
*          VCHG
*            |
* jt_vfchg_h~+~~~~~~~~~~~~~~~~~~~+
*            |                   |
* jt_vfchg_l-| - - - - - - - - - +~~~~~~~~~~+
*            |    Charge area    +          |
*  -------0--+-------------------+----------+--- Temp
*            !                   +
*          ICHG
*            |                   +
*  jt_ichg_h-+ - -+~~~~~~~~~~~~~~+~~~~~~~~~~+
*            +    |              +          |
*  jt_ichg_l-+~~~~+   Charge area           |
*            |    +              +          |
*         0--+----+--------------+----------+--- Temp
*            0   jt_temp_l      jt_temp_h   55
*/
	.battery_init_data = battery_init_parameter,

};

static struct pmu_platform_data grus_pmu_pdata = {
	.gpio = GPIO_PA(28),
	.num_regulators = ARRAY_SIZE(grus_pmu_regulators),
	.regulators = grus_pmu_regulators,
	.private = &ricoh618_private,
	.bat_private = &ricoh618_bat_private,
};
#endif
#define PMU_I2C_BUSNUM 1

struct i2c_board_info grus_pmu_board_info = {
#ifndef CONFIG_BOARD_GRUS_V_1_0_1
	I2C_BOARD_INFO("act8600", 0x5a),
#else
	I2C_BOARD_INFO("ricoh618", 0x32),
#endif
	.platform_data = &grus_pmu_pdata,
};

static int __init grus_pmu_dev_init(void)
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

	client = i2c_new_device(adap, &grus_pmu_board_info);
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

subsys_initcall_sync(grus_pmu_dev_init);
