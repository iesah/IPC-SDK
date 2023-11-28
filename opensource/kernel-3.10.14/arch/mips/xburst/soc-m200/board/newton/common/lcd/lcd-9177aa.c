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
#include "../board_base.h"

int byd_9177aa_init(struct lcd_device *lcd)
{
    int ret = 0;
    if(GPIO_MIPI_RST_N > 0){
        gpio_free(GPIO_MIPI_RST_N);
        ret = gpio_request(GPIO_MIPI_RST_N, "lcd mipi panel rst");
        if (ret) {
            printk(KERN_ERR "can's request lcd panel rst\n");
            return ret;
        }
    }

    if(GPIO_MIPI_PWR > 0){
        gpio_free(GPIO_MIPI_PWR);
        ret = gpio_request(GPIO_MIPI_PWR, "lcd mipi panel avcc");
        if (ret) {
            printk(KERN_ERR "can's request lcd panel avcc\n");
            return ret;
        }
    }
    return 0;
}

int byd_9177aa_reset(struct lcd_device *lcd)
{
	gpio_direction_output(GPIO_MIPI_RST_N, 0);
	msleep(3);
	gpio_direction_output(GPIO_MIPI_RST_N, 1);
	msleep(8);
	return 0;
}

int byd_9177aa_power_on(struct lcd_device *lcd, int enable)
{
    if(byd_9177aa_init(lcd))
        return -EFAULT;
	gpio_direction_output(GPIO_MIPI_PWR, enable); /* 2.8v en*/
	msleep(2);
	return 0;
}
struct lcd_platform_data byd_9177aa_data = {
	.reset = byd_9177aa_reset,
	.power_on= byd_9177aa_power_on,
};

struct mipi_dsim_lcd_device byd_9177aa_device={
	.name		= "byd_9177aa-lcd",
	.id = 0,
	.platform_data = &byd_9177aa_data,
};

struct fb_videomode jzfb_videomode = {
	.name = "byd_9177aa-lcd",
	.refresh = 60,
	.xres = 540,
	.yres = 960,
	.pixclock = KHZ2PICOS(25000),
	.left_margin = 48,
	.right_margin = 48,
	.upper_margin = 9,
	.lower_margin = 13,
	.hsync_len = 48,
	.vsync_len = 2,
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct jzdsi_data jzdsi_pdata = {
	.modes = &jzfb_videomode,
	.video_config.no_of_lanes = 2,
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
	.video_config.byte_clock =  DEFAULT_DATALANE_BPS / 8,	/* KHz  */
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
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

//	gpio_direction_output(GPIO_BL_PWR_EN, 1);

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
