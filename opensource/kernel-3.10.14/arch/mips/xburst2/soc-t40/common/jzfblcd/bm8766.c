/*
 * ingenic_bsp/chip-x2000/fpga/dpu/bm8766.c
 *
 * Copyright (C) 2016 Ingenic Semiconductor Inc.
 *
 * Author:clwang<chunlei.wang@ingenic.com>
 *
 * This program is free software, you can redistribute it and/or modify it
 *
 * under the terms of the GNU General Public License version 2 as published by
 *
 * the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/digital_pulse_backlight.h>

#include <mach/jzfb.h>
#include <soc/gpio.h>

static int bm8766_power_on(struct lcdc_gpio_struct *gpio)
{
	int ret = 0;
#ifdef CONFIG_FPGA_TEST
	ret = gpio_request(GPIO_PD(22), "lcd_bl");
	if (ret) {
		printk(KERN_ERR "can's request lcd rst\n");
		return ret;
	}
	ret = gpio_request(GPIO_PD(25), "lcd_pwm");
	if (ret) {
		printk(KERN_ERR "can's request lcd rst\n");
		return ret;
	}
	gpio_direction_output(GPIO_PD(22), 1);
	gpio_direction_output(GPIO_PD(25), 1);
#endif
	return 0;
}

static int bm8766_power_off(struct lcdc_gpio_struct *gpio)
{
	return 0;
}

struct lcd_callback_ops bm8766_ops = {
	.lcd_power_on_begin  = (void*)bm8766_power_on,
	.lcd_power_off_begin = (void*)bm8766_power_off,
};

struct fb_videomode jzfb_bm8766_videomode = {
	.name = "800x480",
	.refresh = 30,
	//.refresh = 60,
	.xres = 800,
	.yres = 480,
	.pixclock = KHZ2PICOS(16632),
	//.pixclock = KHZ2PICOS(33264),
	.left_margin = 88,
	.right_margin = 40,
	.upper_margin = 8,
	.lower_margin = 35,
	.hsync_len = 128,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct jzfb_tft_config bm8766_cfg = {
	.pix_clk_inv = 0,
	.de_dl = 0,
	.sync_dl = 0,
	.color_even = TFT_LCD_COLOR_EVEN_RGB,
	.color_odd = TFT_LCD_COLOR_ODD_RGB,
	.mode = TFT_LCD_MODE_PARALLEL_888,
};

struct jzfb_platform_data jzfb_data = {
	.num_modes = 1,
	.modes = &jzfb_bm8766_videomode,
	.lcd_type = LCD_TYPE_TFT,
	.bpp = 24,
	.width = 800,
	.height = 480,

	.tft_config = &bm8766_cfg,

	.dither_enable = 0,
	.dither.dither_red = 0,
	.dither.dither_green = 0,
	.dither.dither_blue = 0,
	.lcd_callback_ops = &bm8766_ops,
};
