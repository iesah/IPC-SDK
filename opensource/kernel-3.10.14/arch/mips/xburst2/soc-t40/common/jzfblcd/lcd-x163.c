/*
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * M200 dorado board lcd setup routines.
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
#include <linux/pwm_backlight.h>
#include <linux/lcd.h>
#include <linux/interrupt.h>

#include <mach/jzfb.h>


struct auo_x163_platform_data{
	struct lcd_platform_data *lcd_pdata;
	char *vcc_lcd_1v8_name;
	char *vcc_lcd_3v0_name;
	char *vcc_lcd_blk_name;
};

struct fb_videomode jzfb_videomode = {
	.name = "auo_x163-lcd",
	.refresh = 60,
	.xres = 320,
	.yres = 320,
	.pixclock = KHZ2PICOS(12288),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

int auo_x163_reset(struct lcd_device *lcd)
{
#if 0
	int ret = 0;

	ret = gpio_request(GPIO_LCD_RST, "lcd rst");
	if (ret) {
		printk(KERN_ERR "can's request lcd rst\n");
		return ret;
	}

	gpio_direction_output(GPIO_LCD_RST, 1);
	mdelay(30);
	gpio_direction_output(GPIO_LCD_RST, 0);  //reset active low
	mdelay(10);
	gpio_direction_output(GPIO_LCD_RST, 1);

	gpio_free(GPIO_LCD_RST);
#endif

	return 0;
}


struct lcd_platform_data auo_x163_data = {
	.reset = auo_x163_reset,
};

struct auo_x163_platform_data auo_x163_pdata = {
	.lcd_pdata = &auo_x163_data,
#if 0
	.vcc_lcd_1v8_name = VCC_LCD_1V8_NAME,
	.vcc_lcd_3v0_name = VCC_LCD_3V0_NAME,
#ifdef CONFIG_WATCH_ACRAB
	.vcc_lcd_blk_name = VCC_LCD_BLK_NAME,
#endif
#endif
};


struct mipi_dsim_lcd_device auo_x163_device={
	.name		= "auo_x163-lcd",
	.id = 0,
	.platform_data = &auo_x163_pdata,
};


static __init int auo_x163_init(void)
{

	mipi_dsi_register_lcd_device(&auo_x163_device);

	return 0;
}


arch_initcall(auo_x163_init);

unsigned long auo_x163_cmd_buf[]= {
	0x2C2C2C2C,
};

struct jzdsi_data jzdsi_pdata = {
	.modes = &jzfb_videomode,
	.video_config.no_of_lanes = 1,
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.video_config.receive_ack_packets = 1,	/* enable receiving of ack packets */
	.video_config.is_18_loosely = 0, /*loosely: R0R1R2R3R4R5__G0G1G2G3G4G5G6__B0B1B2B3B4B5B6, not loosely: R0R1R2R3R4R5G0G1G2G3G4G5B0B1B2B3B4B5*/
	.video_config.data_en_polarity = 1,

	.dsi_config.max_lanes = 1,
	.dsi_config.max_hs_to_lp_cycles = 100,
	.dsi_config.max_lp_to_hs_cycles = 40,
	.dsi_config.max_bta_cycles = 4095,
	.dsi_config.color_mode_polarity = 1,
	.dsi_config.shut_down_polarity = 1,
	.dsi_config.te_mipi_en = 0,		/* support mipi TE.*/
#if 0
	.dsi_config.te_gpio = DSI_TE_GPIO,
	.dsi_config.te_irq_level = IRQF_TRIGGER_RISING,
#endif
	.dsi_config.max_bps = 1000 //1000 Mbps
};

struct jzfb_smart_config x163_cfg = {
	.frm_md = 0,
	.rdy_anti_jit = 0,
	.te_anti_jit = 1,
	.te_md = 0,
	.te_switch = 0,		/* te_switch and te_mipi_en　select only 1.*/
	.te_mipi_switch = 0, 	/* use edpi TE from MIPI */
	.rdy_switch = 0,
	.cs_en = 0,
	.cs_dp = 0,
	.rdy_dp = 1,
	.dc_md = 0,
	.wr_md = 1,
	.te_dp = 1,
	.smart_type = SMART_LCD_TYPE_8080,
	.pix_fmt = SMART_LCD_FORMAT_888,
	.dwidth = SMART_LCD_DWIDTH_24_BIT,
	.cwidth = SMART_LCD_CWIDTH_24_BIT,
	.bus_width = 8, // nouse.
};

struct jzfb_platform_data jzfb_data = {
	.num_modes = 1,
	.modes = &jzfb_videomode,
	.dsi_pdata = &jzdsi_pdata,

	.lcd_type = LCD_TYPE_MIPI_SLCD,
	.bpp = 32,
	.width = 32, // inch ??
	.height = 32,

	.smart_config = &x163_cfg,

	.dither_enable = 1,
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
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

