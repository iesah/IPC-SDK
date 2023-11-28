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
#include <linux/lcd.h>

#include <mach/jzfb.h>

 struct dsi_cmd_packet ek79007_cmd_list[] = {
	{0x15, 0x80, 0xAC, 0x00},
	{0x15, 0x81, 0xB8, 0x00},
	{0x15, 0x82, 0x09, 0x00},
	{0x15, 0x83, 0x78, 0x00},
	{0x15, 0x84, 0x7F, 0x00},
	{0x15, 0x85, 0xBB, 0x00},
	{0x15, 0x86, 0x70, 0x00},
	//{0x15, 0xB0, 0x80, 0x00}, /* pwm on */
	//{0x15, 0x87, 0x5A, 0x00}, /* test color bar */
	//{0x15, 0xB1, 0x08, 0x00}, /* test color bar */
#ifdef CONFIG_MIPI_2LANE
	{0x15, 0xB2, 0x10, 0x00}, /*00:4 lane; 10:2 lane*/
#endif
};



int startek_ek79007_init(struct lcd_device *lcd)
{
	return 0;
}

int startek_ek79007_reset(struct lcd_device *lcd)
{
	return 0;
}

int startek_ek79007_power_on(struct lcd_device *lcd, int enable)
{
	return 0;
}
#if 0
struct lcd_platform_data startek_ek79007_data = {
	.reset = startek_ek79007_reset,
	.power_on= startek_ek79007_power_on,
};


struct mipi_dsim_lcd_device startek_ek79007_device={
	.name		= "startek_ek79007-lcd",
	.id = 0,
	.platform_data = &startek_ek79007_data,
};
#endif

struct fb_videomode jzfb_ek79007_videomode = {
	.name = "startek_ek79007-lcd",
	.refresh = 15,
	.xres = 1024,
	.yres = 600,
#ifdef CONFIG_MIPI_4LANE
	.pixclock = KHZ2PICOS(10000),
	.left_margin = 160,
	.right_margin = 160,
	.upper_margin = 12,
	.lower_margin = 23,
	.hsync_len = 10,
	.vsync_len = 1,
#endif
#ifdef CONFIG_MIPI_2LANE
	.pixclock = KHZ2PICOS(10000),
	.left_margin = 60,
	.right_margin = 60,
	.upper_margin = 5,
	.lower_margin = 10,
	.hsync_len = 10,
	.vsync_len = 2,
#endif
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
//	.sync = 0,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct jzdsi_data jzdsi_pdata = {
	.modes = &jzfb_ek79007_videomode,
#ifdef CONFIG_MIPI_2LANE
	.video_config.no_of_lanes = 2,
#endif
#ifdef CONFIG_MIPI_4LANE
	.video_config.no_of_lanes = 4,
#endif
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
#if 0
	.video_config.video_mode = VIDEO_NON_BURST_WITH_SYNC_PULSES,
#else
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
#endif
	.video_config.receive_ack_packets = 0,	/* enable receiving of ack packets */
	/*loosely: R0R1R2R3R4R5__G0G1G2G3G4G5G6__B0B1B2B3B4B5B6,
	 * not loosely: R0R1R2R3R4R5G0G1G2G3G4G5B0B1B2B3B4B5*/
	.video_config.is_18_loosely = 0,
	.video_config.data_en_polarity = 1,


	.dsi_config.max_lanes = 4,
	.dsi_config.max_hs_to_lp_cycles = 100,
	.dsi_config.max_lp_to_hs_cycles = 40,
	.dsi_config.max_bta_cycles = 4095,
	.dsi_config.color_mode_polarity = 1,
	.dsi_config.shut_down_polarity = 1,
	.dsi_config.max_bps = 460,
	//	.max_bps = 650,  // 650 Mbps
	.bpp_info = 24,

};

struct jzfb_platform_data jzfb_data = {
	.num_modes = 1,
	.modes = &jzfb_ek79007_videomode,
	.dsi_pdata = &jzdsi_pdata,

	.lcd_type = LCD_TYPE_TFT ,
	.width = 1024,
	.height = 600,

	//.pixclk_falling_edge = 0,
	//.data_enable_active_low = 0,

};

/**************************************************************************************************/
#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 100,
	.pwm_period_ns	= 30000,
};

struct platform_device backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &backlight_data,
	},
};

#endif
