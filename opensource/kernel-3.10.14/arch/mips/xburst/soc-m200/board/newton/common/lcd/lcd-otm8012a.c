/*
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 *  dorado board lcd setup routines.
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
#include "board_base.h"
#include <linux/otm8012a_init_mipi.h>

struct fb_videomode jzfb_videomode = {
	.name = "480x854",
	.refresh = 60,
	.xres = 480,
	.yres = 854,
	.pixclock = KHZ2PICOS(24000),
	.left_margin = 6,
	.right_margin = 6,
	.upper_margin = 12,
	.lower_margin = 12,
	.hsync_len = 2,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct jzdsi_platform_data jzdsi_pdata = {
	.video_config.no_of_lanes = 2,
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
	.video_config.byte_clock = DEFAULT_DATALANE_BPS / 8,	/* KHz  */
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.video_config.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.video_config.pixel_clock = 24000,	/* dpi_clock */
	.video_config.is_18_loosely = 0,
	.video_config.data_en_polarity = 1,

	.video_config.h_polarity = 0,
	.video_config.h_active_pixels = 480,
	.video_config.h_sync_pixels = 2,	/* min 4 pixels */
	.video_config.h_back_porch_pixels = 6,
	.video_config.h_total_pixels = 480 + 2 + 6 + 6,
	.video_config.v_active_lines = 854,
	.video_config.v_polarity = 0,	/*1:active high, 0: active low */
	.video_config.v_sync_lines = 2,
	.video_config.v_back_porch_lines = 12,
	.video_config.v_total_lines = 854 + 2 + 12 + 12,

	.dsi_config.max_lanes = 4,
	.dsi_config.max_hs_to_lp_cycles = 100,
	.dsi_config.max_lp_to_hs_cycles = 40,
	.dsi_config.max_bta_cycles = 4095,
	.dsi_config.color_mode_polarity = 1,
	.dsi_config.shut_down_polarity = 1,

	.cmd_list = otm8012a_cmd_list,
	.cmd_packet_len = 200,
};

struct jzfb_platform_data jzfb_pdata = {
	.num_modes = 1,
	.modes = &jzfb_videomode,
	.dsi_pdata = &jzdsi_pdata,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 55,
	.height = 98,

	.pixclk_falling_edge = 0,
	.data_enable_active_low = 0,

};

/**************************************************************************************************/
#ifdef CONFIG_BACKLIGHT_PWM
static int backlight_init(struct device *dev)
{
	int ret;
	ret = gpio_request(GPIO_LCD_PWM, "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPF for PWM-OUT1\n");
		return ret;
	}

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
