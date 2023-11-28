/*
 * Copyright (c) 2012 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * modem setup routines.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/modem_pm.h>

#include "i88.h"

static struct modem_data modems[] = {
	{
		.id 		= CARDID_LTE_FDD,
		.name		= "LI170",
		.bp_pwr		= {GPIO_LTE_PWR, GPIO_LTE_PWR_LEVEL},
		.bp_onoff 	= {GPIO_LTE_ONOFF, GPIO_LTE_ONOFF_LEVEL},
		.bp_reset 	= {GPIO_LTE_RESET, GPIO_LTE_RESET_LEVEL},
		.bp_wake_ap 	= {GPIO_LTE_WAKE_AP, GPIO_LTE_WAKE_AP_LEVEL},
		.ap_wake_bp	= {GPIO_AP_WAKE_LTE, GPIO_AP_WAKE_LTE_LEVEL},
	}
};

static struct modem_platform_data modem_data = {
	.bp_nums = sizeof(modems) / sizeof(modems[0]),
	.bp = modems
};

struct platform_device jz_modem_device = {
	.name		= "jz-modem",
	.dev		= {
		.platform_data	= &modem_data,
	},
};
