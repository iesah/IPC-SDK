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
#include <linux/digital_pulse_backlight.h>
#include <linux/at070tn93.h>

#include <mach/jzfb.h>
#include <mach/fb_hdmi_modes.h>

#include "board.h"

#ifdef CONFIG_LCD_BYD_BM8766U
#include <linux/byd_bm8766u.h>
static struct platform_byd_bm8766u_data byd_bm8766u_pdata= {
	.gpio_lcd_disp = GPIO_PB(30),
	.gpio_lcd_de   = 0,		//GPIO_PC(9),	/* chose sync mode */
	.gpio_lcd_vsync = 0,		//GPIO_PC(19),
	.gpio_lcd_hsync = 0,		//GPIO_PC(18),
};

/* LCD device */
struct platform_device byd_bm8766u_device = {
	.name		= "byd_bm8766u-lcd",
	.dev		= {
		.platform_data = &byd_bm8766u_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_KD50G2_40NM_A2
#include <linux/kd50g2_40nm_a2.h>
static struct platform_kd50g2_40nm_a2_data kd50g2_40nm_a2_pdata= {
	.gpio_lcd_disp = GPIO_PB(30),
	.gpio_lcd_de   = 0,		//GPIO_PC(9),	/* chose sync mode */
	.gpio_lcd_vsync = 0,		//GPIO_PC(19),
	.gpio_lcd_hsync = 0,		//GPIO_PC(18),
};

/* LCD device */
struct platform_device kd50g2_40nm_a2_device = {
	.name		= "kd50g2_40nm_a2-lcd",
	.dev		= {
		.platform_data = &kd50g2_40nm_a2_pdata,	
	},
};
#endif

#ifdef CONFIG_LCD_KFM701A21_1A
#include <linux/kfm701a21_1a.h>
static struct platform_kfm701a21_1a_data kfm701a21_1a_pdata = {
	.gpio_lcd_cs = GPIO_PC(21),
	.gpio_lcd_reset = GPIO_PB(28),
};

/* LCD Panel Device */
struct platform_device kfm701a21_1a_device = {
	.name		= "kfm701a21_1a-lcd",
	.dev		= {
		.platform_data	= &kfm701a21_1a_pdata,
	},
};

static struct smart_lcd_data_table kfm701a21_1a_data_table[] = {
	/* soft reset */
	{0x0600, 0x0001, 0, 10000},
	/* soft reset */
	{0x0600, 0x0000, 0, 10000},
	{0x0606, 0x0000, 0, 10000},
	{0x0007, 0x0001, 0, 10000},
	{0x0110, 0x0001, 0, 10000},
	{0x0100, 0x17b0, 0, 0},
	{0x0101, 0x0147, 0, 0},
	{0x0102, 0x019d, 0, 0},
	{0x0103, 0x8600, 0, 0},
	{0x0281, 0x0010, 0, 10000},
	{0x0102, 0x01bd, 0, 10000},
	/* initial */
	{0x0000, 0x0000, 0, 0},
	{0x0001, 0x0000, 0, 0},
	{0x0002, 0x0400, 0, 0},
	/* up:0x1288 down:0x12B8 left:0x1290 right:0x12A0 */
	//{0x0003, 0x12b8, 0, 0}, /* BGR */
	{0x0003, 0x02b8, 0, 0}, /* RGB */
	{0x0006, 0x0000, 0, 0},
	{0x0008, 0x0503, 0, 0},
	{0x0009, 0x0001, 0, 0},
	{0x000b, 0x0010, 0, 0},
	{0x000c, 0x0000, 0, 0},
	{0x000f, 0x0000, 0, 0},
	{0x0007, 0x0001, 0, 0},
	{0x0010, 0x0010, 0, 0},
	{0x0011, 0x0202, 0, 0},
	{0x0012, 0x0300, 0, 0},
	{0x0020, 0x021e, 0, 0},
	{0x0021, 0x0202, 0, 0},
	{0x0022, 0x0100, 0, 0},
	{0x0090, 0x0000, 0, 0},
	{0x0092, 0x0000, 0, 0},
	{0x0100, 0x16b0, 0, 0},
	{0x0101, 0x0147, 0, 0},
	{0x0102, 0x01bd, 0, 0},
	{0x0103, 0x2c00, 0, 0},
	{0x0107, 0x0000, 0, 0},
	{0x0110, 0x0001, 0, 0},
	{0x0210, 0x0000, 0, 0},
	{0x0211, 0x00ef, 0, 0},
	{0x0212, 0x0000, 0, 0},
	{0x0213, 0x018f, 0, 0},
	{0x0280, 0x0000, 0, 0},
	{0x0281, 0x0001, 0, 0},
	{0x0282, 0x0000, 0, 0},
	/* gamma corrected value table */
	{0x0300, 0x0101, 0, 0},
	{0x0301, 0x0b27, 0, 0},
	{0x0302, 0x132a, 0, 0},
	{0x0303, 0x2a13, 0, 0},
	{0x0304, 0x270b, 0, 0},
	{0x0305, 0x0101, 0, 0},
	{0x0306, 0x1205, 0, 0},
	{0x0307, 0x0512, 0, 0},
	{0x0308, 0x0005, 0, 0},
	{0x0309, 0x0003, 0, 0},
	{0x030a, 0x0f04, 0, 0},
	{0x030b, 0x0f00, 0, 0},
	{0x030c, 0x000f, 0, 0},
	{0x030d, 0x040f, 0, 0},
	{0x030e, 0x0300, 0, 0},
	{0x030f, 0x0500, 0, 0},
	/* secorrect gamma2 */
	{0x0400, 0x3500, 0, 0},
	{0x0401, 0x0001, 0, 0},
	{0x0404, 0x0000, 0, 0},
	{0x0500, 0x0000, 0, 0},
	{0x0501, 0x0000, 0, 0},
	{0x0502, 0x0000, 0, 0},
	{0x0503, 0x0000, 0, 0},
	{0x0504, 0x0000, 0, 0},
	{0x0505, 0x0000, 0, 0},
	{0x0600, 0x0000, 0, 0},
	{0x0606, 0x0000, 0, 0},
	{0x06f0, 0x0000, 0, 0},
	{0x07f0, 0x5420, 0, 0},
	{0x07f3, 0x288a, 0, 0},
	{0x07f4, 0x0022, 0, 0},
	{0x07f5, 0x0001, 0, 0},
	{0x07f0, 0x0000, 0, 0},
	/* end of gamma corrected value table */
	{0x0007, 0x0173, 0, 0},
	/* Write Data to GRAM */
	{0, 0x0202, 1, 10000},
	/* Set the start address of screen, for example (0, 0) */
	{0x200, 0, 0, 1},
	{0x201, 0, 0, 1},
	{0, 0x202, 1, 100}
};
#endif

/**************************************************************************************************/

struct fb_videomode jzfb0_videomode = {
#ifdef CONFIG_LCD_BYD_BM8766U
	.name = "800x480",
	.refresh = 55,
	.xres = 800,
	.yres = 480,
	.pixclock = KHZ2PICOS(33260),
	.left_margin = 88,
	.right_margin = 40,
	.upper_margin = 8,
	.lower_margin = 35,
	.hsync_len = 128,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
#endif

#ifdef CONFIG_LCD_KFM701A21_1A
	.name = "400x240",
	.refresh = 60,
	.xres = 400,
	.yres = 240,
	.pixclock = KHZ2PICOS(5760),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
#endif

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

#ifdef CONFIG_LCD_KD50G2_40NM_A2 // 60Hz@vpll=888MHz
	.name = "800x480",
	.refresh = 55,
	.xres = 800,
	.yres = 480,
	.pixclock = KHZ2PICOS(33260),
	.left_margin = 88,
	.right_margin = 40,
	.upper_margin = 8,
	.lower_margin = 35,
	.hsync_len = 128,
	.vsync_len = 2,
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
#ifdef CONFIG_LCD_BYD_BM8766U
	.num_modes = 1,
	.modes = &jzfb0_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 108,
	.height = 65,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
#endif

#ifdef CONFIG_LCD_KFM701A21_1A
	.num_modes = 1,
	.modes = &jzfb0_videomode,

	.lcd_type = LCD_TYPE_LCM,
	.bpp = 18,
	.width = 39,
	.height = 65,
	.pinmd = 0,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
	.lvds = 0,

	.smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
	.smart_config.cmd_width = SMART_LCD_CWIDTH_18_BIT_ONCE,  /* KFM701A21_1A: 18-bit? 16-bit? */
	.smart_config.data_width = SMART_LCD_DWIDTH_18_BIT_ONCE_PARALLEL_SERIAL,
	.smart_config.data_width2 = SMART_LCD_DWIDTH_18_BIT_ONCE_PARALLEL_SERIAL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,

	.smart_config.continuous_dma = 0, /* 1: auto continuous dma. 0: trigger DMA_RESTART per-frame dma. */

	.smart_config.write_gram_cmd = 0x00000804, /* 0x00000202(16bit) -->> 0x00000804(18bit) ? */
	.smart_config.bus_width = 18,
	.smart_config.length_data_table = ARRAY_SIZE(kfm701a21_1a_data_table),
	.smart_config.data_table = kfm701a21_1a_data_table,
	.smart_config.init = 0, /* smart lcd special initial function */

	.dither_enable = 1,
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
#endif
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

#ifdef CONFIG_LCD_KD50G2_40NM_A2
	.num_modes = 1,
	.modes = &jzfb0_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 108,
	.height = 65,

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

/***********************************************************************************************/
#ifdef CONFIG_BACKLIGHT_DIGITAL_PULSE
static int init_backlight(struct device *dev)
{
	return 0;
}
static void exit_backlight(struct device *dev)
{

}

struct platform_digital_pulse_backlight_data bl_data = {
	.digital_pulse_gpio = GPIO_PE(1),
	.max_brightness = 255,
	.dft_brightness = 120,
	.max_brightness_step = 16,
	.high_level_delay_us = 5,
	.low_level_delay_us = 5,
	.init = init_backlight,
	.exit = exit_backlight,
};

struct platform_device digital_pulse_backlight_device = {
	.name		= "digital-pulse-backlight",
	.dev		= {
		.platform_data	= &bl_data,
	},
};
#endif
