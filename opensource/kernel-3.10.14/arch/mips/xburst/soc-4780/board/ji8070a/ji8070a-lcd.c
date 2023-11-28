/*
 * Copyright (c) 2012 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * JZ4780 ji8070a board lcd setup routines.
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

	#ifdef CONFIG_LCD_HSD070IDW1
		#ifdef CONFIG_BOARD_JI8070B
			{
				.name = "1024x600",
				.refresh = 60,
				.xres = 1024,
				.yres = 600,
				.pixclock = KHZ2PICOS(51200),
				.left_margin = 139,
				.right_margin = 160,
				.upper_margin = 20,
				.lower_margin = 12,
				.hsync_len = 20,
				.vsync_len = 3,
				.sync = 0 | 0, /* FB_SYNC_HOR_HIGH_ACT:0, FB_SYNC_VERT_HIGH_ACT:0 */
				.vmode = FB_VMODE_NONINTERLACED,
				.flag = 0
			},

		#else

			{
				.name = "800x480",
				.refresh = 60,
				.xres = 800,
				.yres = 480,
				.pixclock = KHZ2PICOS(33300),

			#ifdef CONFIG_BOARD_Q8

				.left_margin = 5,
				.right_margin = 118, //128
				.upper_margin = 20,
				.lower_margin = 0,
				.hsync_len = 40,
			#else

				.left_margin = 40,
				.right_margin = 40,
				.upper_margin = 29,
				.lower_margin = 13,
				.hsync_len = 48,
			#endif

				.vsync_len = 3,
				.sync = 0 | 0, /* FB_SYNC_HOR_HIGH_ACT:0, FB_SYNC_VERT_HIGH_ACT:0 */
				.vmode = FB_VMODE_NONINTERLACED,
				.flag = 0
			},
		#endif
	#endif
	};

	struct jzfb_platform_data jzfb1_pdata = {
		.num_modes = ARRAY_SIZE(jzfb1_videomode),
		.modes = jzfb1_videomode,

	#ifdef CONFIG_LCD_HSD070IDW1
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

	};
#endif /* CONFIG_FB_JZ4780_LCDC1 */


#ifdef CONFIG_LCD_HSD070IDW1
	#include <linux/android_bl.h>

	static int bk_is_on = 1;

	void bk_on(int on)
	{
		bk_is_on = on;
	}
	static struct android_bl_platform_data bl_pdata= {
#ifdef CONFIG_BOARD_Q8
		.delay_before_bkon = 0,
#else
		.delay_before_bkon = 0,
#endif
		.notify_on = bk_on,
		.bootloader_unblank = 1
	};

	/* LCD Panel Device */
	struct platform_device android_bl_device = {
		.name		= "android-bl",
		.dev		= {
			.platform_data	= &bl_pdata,
		},
	};
#endif

#ifdef CONFIG_BACKLIGHT_PWM
static int ji8070a_backlight_init(struct device *dev)
{
	return 0;
}

static void ji8070a_backlight_exit(struct device *dev)
{
}


static int bk_notify(struct device *dev, int brightness)
{
	if (!bk_is_on)
		return 0;

	brightness = 35 + (brightness * 7)/10;
	return brightness;
}

static struct platform_pwm_backlight_data ji8070a_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,//255
	.dft_brightness	= 80,
#ifdef CONFIG_BOARD_Q8
	.pwm_period_ns	= 1000000,/* 1 KHz */
#else
	.pwm_period_ns	= 100000,/* 10 KHz */
#endif
	.init		= ji8070a_backlight_init,
	.exit		= ji8070a_backlight_exit,
    .notify = bk_notify
};

/* Backlight Device */
struct platform_device ji8070a_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &ji8070a_backlight_data,
	},
};

#endif
