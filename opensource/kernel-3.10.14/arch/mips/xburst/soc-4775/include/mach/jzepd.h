/*
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZEPD_H__
#define __JZEPD_H__

#include <linux/fb.h>

struct jz_epd_power_pin {
	char *name;	// power name
	unsigned active_level:1; // active level if power on
	unsigned int pwr_gpio;	 // gpio
	unsigned int pwr_on_delay; // power-on delay (us)
	unsigned int pwr_off_delay; // power-off delay (us)
};

struct jz_epd_power_ctrl {
	void (*epd_power_init)(void);
	void (*epd_power_on)(void);
	void (*epd_power_off)(void);
#ifdef CONFIG_SLCD_SUSPEND_ALARM_WAKEUP_REFRESH
	void (*epd_suspend_power_on)(void);
	void (*epd_suspend_power_off)(void);
#endif
};

struct jz_epd_platform_data {
	struct jz_epd_power_ctrl epd_power_ctrl;
};

#endif /* __JZEPD_H__ */
