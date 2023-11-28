/*
 * Copyright (c) 2012 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * JZ4780 orion board lcd setup routines.
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

#include <mach/jzepd.h>

#include "board_base.h"

static struct jz_epd_power_pin power_pin[] = {
#if 0
	{ "epd enable", 0, GPIO_EPD_SIG_CTRL_N, 0, 0 },
	{ "epd pwr0",   1, GPIO_EPD_PWR0,     100, 0 },
	{ "epd pwr1",   1, GPIO_EPD_PWR1,    1000, 0 },
	{ "epd pwr3",   1, GPIO_EPD_PWR3,       0, 0 },
	{ "epd pwr2",   1, GPIO_EPD_PWR2,     100, 0 },
	{ "epd pwr4",   1, GPIO_EPD_PWR4,    1000, 0 }
#endif
#if 1
	{ "EN",           1, GPIO_EPD_EN,         0, 0 },
	{ "ENOP",         1, GPIO_EPD_ENOP,         0, 0 },
#endif
};

typedef volatile unsigned int * vuip;
#define PZBASE	0xB0010F00
#define PCCFG3	(*(vuip)(PZBASE+0xFEC))
#define PCCFG2	(*(vuip)(PZBASE+0xFE8))
#define PCCFG1	(*(vuip)(PZBASE+0xE4))
#define PCCFG0	(*(vuip)(PZBASE+0xE0))
#define EPD13	(0x3F<<14)
static void mensa_epd_power_init(void)
{
	int i = 0;
	for ( i = 0; i < ARRAY_SIZE(power_pin); i++ ) {
		gpio_request(power_pin[i].pwr_gpio, power_pin[i].name);
	}
}

//EPD power up sequence function for epd driver
static void mensa_epd_power_on(void)
{
	int i = 0;
	for ( i = 0; i < ARRAY_SIZE(power_pin); i++ ) {
		gpio_direction_output(power_pin[i].pwr_gpio, power_pin[i].active_level);
		udelay(power_pin[i].pwr_on_delay);
	}
}

//EPD power down sequence function for epd driver
static void mensa_epd_power_off(void)
{
	int i = 0;
	for ( i = 0; i < ARRAY_SIZE(power_pin); i++ ) {
		gpio_direction_output(power_pin[i].pwr_gpio, 1^power_pin[i].active_level);
		udelay(power_pin[i].pwr_off_delay);
	}
}

#ifdef CONFIG_SLCD_SUSPEND_ALARM_WAKEUP_REFRESH
static void mensa_epd_suspend_power_on(void)
{
	//GPIO_PC(20) EPD_POWER_EN output 1
	*(volatile unsigned int *)0xb0010244 = (0x1 << 20);
	//GPIO_PC(21) EN output 1
	*(volatile unsigned int *)0xb0010244 = (0x1 << 21);
	//GPIO_PC(22) ENOP output 1
	*(volatile unsigned int *)0xb0010244 = (0x1 << 22);
}

static void mensa_epd_suspend_power_off(void)
{
	//GPIO_PC(20) EPD_POWER_EN output 0
	*(volatile unsigned int *)0xb0010248 = (0x1 << 20);
	//GPIO_PC(21) EN output 0
	*(volatile unsigned int *)0xb0010248 = (0x1 << 21);
	//GPIO_PC(22) ENOP output 0
	*(volatile unsigned int *)0xb0010248 = (0x1 << 22);
}
#endif

struct jz_epd_platform_data jz_epd_pdata = {
	.epd_power_ctrl.epd_power_init = mensa_epd_power_init,
	.epd_power_ctrl.epd_power_on = mensa_epd_power_on,
	.epd_power_ctrl.epd_power_off = mensa_epd_power_off,
#ifdef CONFIG_SLCD_SUSPEND_ALARM_WAKEUP_REFRESH
	.epd_power_ctrl.epd_suspend_power_on = mensa_epd_suspend_power_on,
	.epd_power_ctrl.epd_suspend_power_off = mensa_epd_suspend_power_off,
#endif
};
