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
#include <linux/pwm_backlight.h>

#include <mach/jzfb.h>
#include <mach/fb_hdmi_modes.h>

#include <linux/kd301_m03545_0317a.h>
#include "board.h"

#ifdef CONFIG_LCD_KD301_M03545_0317A
static struct platform_kd301_data kd301_pdata = {
	.gpio_lcd_spi_dr = GPIO_PC(0),
	.gpio_lcd_spi_dt = GPIO_PC(1),
	.gpio_lcd_spi_clk = GPIO_PC(10),
	.gpio_lcd_spi_ce = GPIO_PC(11),
	.gpio_lcd_reset = GPIO_PB(28),
	.v33_reg_name = "vlcd",
};

struct platform_device kd301_device = {
	.name = "kd301_m03545_0317a",
	.dev = {
		.platform_data = &kd301_pdata,
	},
};
#endif	/* CONFIG_LCD_KD301_M03545_0317A */

/*************************************************************/

struct fb_videomode jzfb0_videomode = {
#ifdef CONFIG_LCD_KD301_M03545_0317A
	.name = "320x480",
	.refresh = 70,
	.xres = 320,
	.yres = 480,
	.pixclock = KHZ2PICOS(11316),
	.left_margin = 1,
	.right_margin = 10,
	.upper_margin = 0,
	.lower_margin = 2,
	.hsync_len = 3,
	.vsync_len = 2,
	.sync = 0 | 0,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
#endif
};

struct jzfb_platform_data jzfb0_pdata = {
#ifdef CONFIG_LCD_KD301_M03545_0317A
	.num_modes = 1,
	.modes = &jzfb0_videomode,

	.lcd_type = LCD_TYPE_GENERIC_18_BIT,
	.bpp = 24,
	.width = 49,
	.height = 74,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
#endif
};

/*************************************************************/

#ifdef CONFIG_BACKLIGHT_PWM
static int backlight_init(struct device *dev)
{
	int ret;
	ret = gpio_request(GPIO_LCD_PWM, "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPF for PWM-OUT1\n");
		return ret;
	}

	/* Configure GPIO pin with S5P6450_GPF15_PWM_TOUT1 */
	//gpio_direction_output(GPIO_LCD_PWM, 1);

	return 0;
}

static void backlight_exit(struct device *dev)
{
	gpio_free(GPIO_LCD_PWM);
}

static struct platform_pwm_backlight_data backlight_data = {
	.pwm_id		= 1,
	.max_brightness	= 255,
	.dft_brightness	= 120,
	.pwm_period_ns	= 30000,
	.init		= backlight_init,
	.exit		= backlight_exit,
};

struct platform_device backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &backlight_data,
	},
};

#endif
