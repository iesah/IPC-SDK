/*
 * Copyright (c) 2012 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * JZ4780 giant board lcd setup routines.
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
#include <linux/at070tn93.h>

#include <mach/jzfb.h>
#include <mach/fb_hdmi_modes.h>

#ifdef CONFIG_LCD_AT070TN93
#define GPIO_LCD_PWM GPIO_PE(0)

static struct platform_at070tn93_data at070tn93_pdata= {
	.gpio_power = GPIO_PC(17),
	.gpio_vsync = GPIO_PC(19),
	.gpio_hsync = GPIO_PC(18),
	.gpio_reset = GPIO_PE(11),
};

struct platform_device at070tn93_device = {
	.name		= "at070tn93-lcd",
	.dev		= {
		.platform_data	= &at070tn93_pdata,
	},
};

#endif

#ifdef CONFIG_LCD_AUO_A043FL01V2
struct platform_device auo_a043fl01v2_device = {
	.name		= "auo_a043fl01v2-lcd",
	.dev		= {
		.platform_data	= NULL,
	},
};
#endif

/**************************************************************************************************/
struct fb_videomode jzfb0_videomode[] = {
	ADD_HDMI_VIDEO_MODE(HDMI_640X480_P_60HZ_4X3),
	ADD_HDMI_VIDEO_MODE(HDMI_720X480_P_60HZ_4X3),
	ADD_HDMI_VIDEO_MODE(HDMI_720X480_P_60HZ_16X9),
	ADD_HDMI_VIDEO_MODE(HDMI_1280X720_P_60HZ_16X9),
	ADD_HDMI_VIDEO_MODE(HDMI_1920X1080_I_60HZ_16X9),
};

struct fb_videomode jzfb1_videomode = {
#ifdef CONFIG_LCD_AT070TN93
	.name = "800x480",
	.refresh = 55,
	.xres = 800,
	.yres = 480,
	.pixclock = KHZ2PICOS(25863),
	.left_margin = 16,
	.right_margin = 48,
	.upper_margin = 7,
	.lower_margin = 23,
	.hsync_len = 30,
	.vsync_len = 16,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
#endif

#ifdef CONFIG_LCD_AUO_A043FL01V2
	.name = "480x272",
	.refresh = 60,
	.xres = 480,
	.yres = 272,
	.pixclock = KHZ2PICOS(9200),
	.left_margin = 4,
	.right_margin = 8,
	.upper_margin = 2,
	.lower_margin = 4,
	.hsync_len = 41,
	.vsync_len = 10,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
#endif
};

struct jzfb_platform_data jzfb0_pdata = {
	.num_modes = ARRAY_SIZE(jzfb0_videomode),
	.modes = jzfb0_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 0,
	.height = 0,

	.pixclk_falling_edge = 1,
	.date_enable_active_low = 0,

	.alloc_vidmem = 0,
};

struct jzfb_platform_data jzfb1_pdata = {
#ifdef CONFIG_LCD_AT070TN93
	.num_modes = 1,
	.modes = &jzfb1_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 154,
	.height = 86,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
#endif

#ifdef CONFIG_LCD_AUO_A043FL01V2
	.num_modes = 1,
	.modes = &jzfb1_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 95,
	.height = 54,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
#endif
};

/**************************************************************************************************/

#ifdef CONFIG_BACKLIGHT_PWM
static int giant_backlight_init(struct device *dev)
{
	int ret;

	ret = gpio_request(GPIO_LCD_PWM, "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPF for PWM-OUT1\n");
		return ret;
	}

	/* Configure GPIO pin with S5P6450_GPF15_PWM_TOUT1 */
	gpio_direction_output(GPIO_LCD_PWM, 0);

	return 0;
}

static void giant_backlight_exit(struct device *dev)
{
	gpio_free(GPIO_LCD_PWM);
}

static struct platform_pwm_backlight_data giant_backlight_data = {
	.pwm_id		= 1,
	.max_brightness	= 255,
	.dft_brightness	= 120,
	.pwm_period_ns	= 30000,
	.init		= giant_backlight_init,
	.exit		= giant_backlight_exit,
};

struct platform_device giant_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &giant_backlight_data,
	},
};

#endif
