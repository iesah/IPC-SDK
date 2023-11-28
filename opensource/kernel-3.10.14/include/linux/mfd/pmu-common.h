/*
 * pmu-common.h - Head file for PMUs including common struct and definitions.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Written by Large Dipper <ykli@ingenic.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_MFD_ACT8600_H
#define __LINUX_MFD_ACT8600_H

#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/act8600-private.h>
#include <linux/mfd/ricoh618.h>
#include <linux/power/ricoh618_battery.h>

struct regulator_info {
	const char name[8];
	struct regulator_init_data *init_data;
};

struct pmu_platform_data {
	unsigned short gpio;
	unsigned short num_regulators;
	struct regulator_info *regulators;
	struct charger_board_info *charger_board_info;
	struct ricoh618_platform_data *private;
	struct ricoh618_battery_platform_data *bat_private;
};

enum {
	LOW_ENABLE=0,
	HIGH_ENABLE,
};

enum {
	UN_AT_BOOT=0,
	EN_AT_BOOT,
};

#define CORE_REGULATOR_DEF(BOARD, VCORE_MIN, VCORE_MAX)			\
static struct regulator_consumer_supply BOARD##_vcore_consumer =	\
	REGULATOR_SUPPLY("vcore", NULL);				\
									\
struct regulator_init_data BOARD##_vcore_init_data = {			\
	.constraints = {						\
		.name			= "Core",			\
		.min_uV			= VCORE_MIN,			\
		.max_uV			= VCORE_MAX,			\
		.always_on		= 1,				\
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE,	\
	},								\
	.num_consumer_supplies  = 1,					\
	.consumer_supplies      = &BOARD##_vcore_consumer,		\
}

#define IO_REGULATOR_DEF(REG, NAME, VOL, ALWAYS_ON)			\
struct regulator_init_data REG##_init_data = {				\
	.constraints = {						\
		.name			= NAME,				\
		.min_uV			= VOL,				\
		.max_uV			= VOL,				\
		.apply_uV		= 1,				\
		.always_on		= ALWAYS_ON,			\
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,	\
	},								\
}

#define EXCLUSIVE_REGULATOR_DEF(REG, NAME, SUPPLY, SREG, DEV_NAME, VOL, APPLY_UV)	\
static struct regulator_consumer_supply REG##_consumer =		\
	REGULATOR_SUPPLY(SUPPLY, DEV_NAME);				\
									\
struct regulator_init_data REG##_init_data = {				\
	.supply_regulator = SREG,					\
	.constraints = {						\
		.name			= NAME,				\
		.min_uV			= VOL,				\
		.max_uV			= VOL,				\
		.apply_uV		= APPLY_UV,			\
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,	\
	},								\
	.num_consumer_supplies  = 1,					\
	.consumer_supplies      = &REG##_consumer,			\
}

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

#define VBUS_REGULATOR_DEF(BOARD, SREG)							\
static struct regulator_consumer_supply vbus_consumer =					\
	REGULATOR_SUPPLY("vbus", NULL);							\
											\
static struct regulator_init_data BOARD##_vbus_init_data = {				\
	.supply_regulator = SREG,							\
	.constraints = {								\
		.name			= "Vbus",					\
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,			\
	},										\
	.num_consumer_supplies  = 1,							\
	.consumer_supplies      = &vbus_consumer,					\
}

#define VBUS_V101_REGULATOR_DEF(BOARD)							\
static struct regulator_init_data BOARD##_vbus_init_data = {				\
	.constraints = {								\
		.name			= "Vbus",					\
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,			\
	},										\
}

#endif /* __LINUX_MFD_ACT8600_H */
