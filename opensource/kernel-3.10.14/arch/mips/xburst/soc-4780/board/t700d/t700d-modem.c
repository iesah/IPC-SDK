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

#include "t700d.h"

static struct modem_data modems[] = {
	{
		.id 		= CARDID_GSM_1,
		.name		= "RDA8851CL",
		.bp_pwr		= {GPIO_BP_PWR, GPIO_BP_PWR_LEVEL},
		.bp_onoff 	= {GPIO_BP_ONOFF, GPIO_BP_ONOFF_LEVEL},
		.bp_wake_ap 	= {GPIO_BP_WAKE_AP, GPIO_BP_WAKE_AP_LEVEL},
		.bp_status 	= {GPIO_BP_STATUS, GPIO_BP_STATUS_LEVEL},
		.ap_wake_bp	= {GPIO_AP_WAKE_BP, GPIO_AP_WAKE_BP_LEVEL},
		.ap_status	= {GPIO_AP_STATUS, GPIO_AP_STATUS_LEVEL},
		.sim_sw1		= {0, 0},
		.sim_sw2		= {0, 0}
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


