/*
 * Copyright (c) 2012 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * JZ4780 leaf board lcd setup routines.
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
#include <linux/regulator/consumer.h>
#include <linux/jzlcd_pdata.h>

static struct jzlcd_platform_data lcd_pdata= {
	.gpio_de = GPIO_PC(9),
	.gpio_vs = GPIO_PC(19),
	.gpio_hs = GPIO_PC(18),
	.gpio_reset = GPIO_PB(22),

	.de_mode = 0,
};

/* LCD Panel Device */
struct platform_device jzlcd_device = {
	.name		= "jz-lcd",
	.dev		= {
		.platform_data	= &lcd_pdata,
	},
};

#ifdef CONFIG_FB_JZ4780_LCDC0
/* LCDC0 output to HDMI and the default hdmi video mode list
 * define in soc-4780/include/mach/fb_hdmi_modes.h
 * or initialization a different platform data at here
 */
static struct fb_videomode jzfb0_hdmi_videomode[] = {
	DEFAULT_HDMI_VIDEO_MODE_LIST,
};

struct jzfb_platform_data jzfb0_hdmi_pdata = {
	.num_modes = ARRAY_SIZE(jzfb0_hdmi_videomode),
	.modes = jzfb0_hdmi_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 0,
	.height = 0,

	.pixclk_falling_edge = 1,
	.date_enable_active_low = 0,

	.alloc_vidmem = 0,
};
#endif /* CONFIG_FB_JZ4780_LCDC0 */

#ifdef CONFIG_FB_JZ4780_LCDC1
/* LCD Controller 1 output to LVDS TFT panel */
static struct fb_videomode jzfb1_videomode[] = {
#ifdef CONFIG_LCD_JCMT070T115A18
	{
		.name = "800x480",
		.refresh = 50,
		.xres = 800,
		.yres = 480,
		.pixclock = KHZ2PICOS(33000),
		.left_margin = 16,
		.right_margin = 210,
		.upper_margin = 10,
		.lower_margin = 22,
		.hsync_len = 30,
		.vsync_len = 13,
		.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0
	},
#endif
#ifdef CONFIG_LCD_LXJC070WHM_280_24A1
	{
		.name = "1024x600",
		.refresh = 60,
		.xres = 1024,
		.yres = 600,
		.pixclock = KHZ2PICOS(57000),
		.left_margin = 140,
		.right_margin = 160,
		.upper_margin = 20,
		.lower_margin = 12,
		.hsync_len = 20,
		.vsync_len = 3,
		.sync = FB_SYNC_HOR_HIGH_ACT & FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0
	},
#endif
};

struct jzfb_platform_data jzfb1_pdata = {
	.num_modes = ARRAY_SIZE(jzfb1_videomode),
	.modes = jzfb1_videomode,
#ifdef CONFIG_LCD_JCMT070T115A18
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 154,
	.height = 86,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
	.lvds = 0,
	.dither_enable = 0,
#endif
#ifdef CONFIG_LCD_LXJC070WHM_280_24A1
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 154,
	.height = 86,

	.pixclk_falling_edge = 1,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
	.lvds = 0,
	.dither_enable = 0,
#endif
};
#endif /* CONFIG_FB_JZ4780_LCDC1 */

#ifdef CONFIG_BACKLIGHT_PWM
static int leaf_backlight_init(struct device *dev)
{
#if 0
       struct regulator *vlcd;
       vlcd = regulator_get(dev, "vlcd");
       if(IS_ERR(vlcd)){
               printk("get vlcd power failed!\r");
               return -EINVAL;
       }

       regulator_enable(vlcd);
#endif
       gpio_request(GPIO_PB(23), "lcd_pwr_en");
       gpio_direction_output(GPIO_PB(23), 1);

       return 0;
}

static void leaf_backlight_exit(struct device *dev)
{
}

static struct platform_pwm_backlight_data leaf_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 80,
	//.pwm_period_ns	= 10000, /* 100 kHZ */
	.pwm_period_ns	= 10000000, /* 100 kHZ */
	.init		= leaf_backlight_init,
	.exit		= leaf_backlight_exit,
};

/* Backlight Device */
struct platform_device leaf_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &leaf_backlight_data,
	},
};

#endif
