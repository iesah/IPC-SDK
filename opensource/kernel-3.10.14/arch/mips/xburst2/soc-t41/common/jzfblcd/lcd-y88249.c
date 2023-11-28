/*
 *
 * Copyright (C) 2019 Ingenic Semiconductor Inc.
 *
 * Author:Gao Wei<wei.gao@ingenic.com>
 *
 * This program is free software, you can redistribute it and/or modify it
 *
 * under the terms of the GNU General Public License version 2 as published by
 *
 * the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/digital_pulse_backlight.h>
#include <linux/lcd_generic.h>
#include <linux/lcd.h>

#include <mach/jzfb.h>
#include <soc/gpio.h>
#include <board.h>

#define Y88249_LCD_X_RES   (320 * 2)
#define Y88249_LCD_Y_RES   (240 * 2)

static struct fb_videomode jzfb_y88249_videomode = {
    .name                   = "640X480",
    .refresh                = 65,
    .xres                   = Y88249_LCD_X_RES,
    .yres                   = Y88249_LCD_Y_RES,
    .pixclock               = KHZ2PICOS(33264),
    .left_margin            = 20,
    .right_margin           = 20,
    .upper_margin           = 6,
    .lower_margin           = 12,

    .hsync_len              = 2,
    .vsync_len              = 2,
    .sync                   = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
    .vmode                  = FB_VMODE_NONINTERLACED,
    .flag                   = 0,
};

static struct jzfb_tft_config y88249_cfg = {
    .pix_clk_inv            = 0,
    .de_dl                  = 0,
    .sync_dl                = 0,
    .color_even             = TFT_LCD_COLOR_EVEN_RGB,
    .color_odd              = TFT_LCD_COLOR_ODD_RGB,
    .mode                   = TFT_LCD_MODE_PARALLEL_888,
};

struct jzfb_platform_data jzfb_data = {
    .num_modes              = 1,
    .modes                  = &jzfb_y88249_videomode,
    .lcd_type               = LCD_TYPE_TFT,
    .bpp                    = 24,
    .width                  = Y88249_LCD_X_RES,
    .height                 = Y88249_LCD_Y_RES,

    .tft_config             = &y88249_cfg,

    .dither_enable          = 1,
    .dither.dither_red      = 1,
    .dither.dither_green    = 1,
    .dither.dither_blue     = 1,
    .lcd_callback_ops       = NULL,
};



static LCD_STRUCT y88249_pdata = {
	.ctl_if.vcc_en = GPIO_LCD_VCC_EN,
	.ctl_if.backlight_en = GPIO_LCD_BACKLIGHT_EN,
	.ctl_if.reset = GPIO_LCD_RESET,
	.ctl_if.spi.cs = GPIO_LCD_CS,
	.ctl_if.spi.clk = GPIO_LCD_CLK,
	.ctl_if.spi.sdo = GPIO_LCD_SDO,
	.ctl_if.spi.sdi = GPIO_LCD_SDI,
};


/* LCD Panel Device */
static struct platform_device y88249_device = {
    .name   = "lcd-y88249",
    .dev    = {
        .platform_data      = &y88249_pdata,
    },
};

static struct platform_pwm_backlight_data backlight_data = {
    .pwm_id                 = 3,
//    .pwm_active_level       = 1,
    .max_brightness         = 255,
    .dft_brightness         = 200,
    .pwm_period_ns          = 30000,
};


struct platform_device backlight_device = {
    .name = "pwm-backlight",
    .dev = {
        .platform_data = &backlight_data,
    },
};


int __init jzfb_backlight_init(void)
{
    int ret = 0;

    pr_debug("======lcd y88249=====backlight==init=================\n");
    ret = platform_device_register(&y88249_device);
    if (ret < 0) {
        printk("register platform device y88249 failed.\n");
        return ret;
    }

    ret = platform_device_register(&backlight_device);
    if (ret < 0) {
        printk("register platform device pwm backlight failed.\n");
        return ret;
    }

    return 0;
}

static void __exit jzfb_backlight_exit(void)
{
    platform_device_unregister(&backlight_device);
    platform_device_unregister(&y88249_device);
}

subsys_initcall(jzfb_backlight_init);
module_exit(jzfb_backlight_exit);
