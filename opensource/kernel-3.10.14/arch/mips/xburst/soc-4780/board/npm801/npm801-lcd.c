/*
 * Copyright (c) 2012 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * JZ4780 npm801 board lcd setup routines.
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

#ifdef CONFIG_LCD_KR080LA4S_250
#include <linux/kr080la4s_250.h>
static struct platform_kr080la4s_250_data kr080la4s_250_pdata= {
/* gpio had been hardware control */
};

/* LCD Panel Device */
struct platform_device kr080la4s_250_device = {
	.name		= "kr080la4s_250-lcd",
	.dev		= {
		.platform_data	= &kr080la4s_250_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_CRD080TI01_40NM01
#include <linux/crd080ti01_40nm01.h>
static struct platform_crd080ti01_40nm01_data crd080ti01_40nm01_pdata= {
/* gpio had been hardware control */
};

/* LCD Panel Device */
struct platform_device crd080ti01_40nm01_device = {
	.name		= "crd080ti01_40nm01-lcd",
	.dev		= {
		.platform_data	= &crd080ti01_40nm01_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_EK070TN93
#include <linux/ek070tn93.h>
static struct platform_ek070tn93_data ek070tn93_pdata= {
	.gpio_de = GPIO_PC(9),
	.gpio_vs = GPIO_PC(19),
	.gpio_hs = GPIO_PC(18),
//	.gpio_reset = GPIO_PB(22),

	.de_mode = 0,
};

/* LCD Panel Device */
struct platform_device ek070tn93_device = {
	.name		= "ek070tn93-lcd",
	.dev		= {
		.platform_data	= &ek070tn93_pdata,
	},
};
#endif

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
#ifdef CONFIG_LCD_KR080LA4S_250
	{
		.name = "1024x768",
		.refresh = 65,
		.xres = 1024,
		.yres = 768,
		.pixclock = KHZ2PICOS(48000),
		.left_margin = 171,
		.right_margin = 0,
		.upper_margin = 18,
		.lower_margin = 0,
		.hsync_len = 0,
		.vsync_len = 0,
		.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0
	},
#endif
#ifdef CONFIG_LCD_CRD080TI01_40NM01
	{
		.name = "1024x768",
		.refresh = 60,
		.xres = 1024,
		.yres = 768,
		.pixclock = KHZ2PICOS(52000),
		.left_margin = 90,
		.right_margin = 0,
		.upper_margin = 10,
		.lower_margin = 0,
		.hsync_len = 0,
		.vsync_len = 0,
		.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0
	},
#endif
#ifdef CONFIG_LCD_EK070TN93
	{
		.name = "800x480",
		.refresh = 60,
		.xres = 800,
		.yres = 480,
		.pixclock = KHZ2PICOS(33300),
		.left_margin = 28,
		.right_margin = 210,
		.upper_margin = 15,
		.lower_margin = 22,
		.hsync_len = 18,
		.vsync_len = 8,
		.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0
	},
#endif
};

struct jzfb_platform_data jzfb1_pdata = {
	.num_modes = ARRAY_SIZE(jzfb1_videomode),
	.modes = jzfb1_videomode,

#ifdef CONFIG_LCD_KR080LA4S_250
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 154,
	.height = 90,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,

	.lvds = 1,
	.txctrl.data_format = VESA,
	.txctrl.clk_edge_falling_7x = 0,
	.txctrl.clk_edge_falling_1x = 1,
	.txctrl.data_start_edge = START_EDGE_4,
	.txctrl.operate_mode = LVDS_1X_CLKOUT,
	.txctrl.edge_delay = DELAY_0_1NS,
	.txctrl.output_amplitude = VOD_350MV,

	.txpll0.ssc_enable = 0,
	.txpll0.ssc_mode_center_spread = 0,
	.txpll0.post_divider = POST_DIV_1,
	.txpll0.feedback_divider = 70,
	.txpll0.input_divider_bypass = 0,
	.txpll0.input_divider = 10,

	.txpll1.charge_pump = CHARGE_PUMP_10UA,
	.txpll1.vco_gain = VCO_GAIN_150M_400M,
	.txpll1.vco_biasing_current = VCO_BIASING_2_5UA,
	.txpll1.sscn = 0,
	.txpll1.ssc_counter = 0,

	.txectrl.emphasis_level = 0,
	.txectrl.emphasis_enable = 0,
	.txectrl.ldo_output_voltage = LDO_OUTPUT_1_1V,
	.txectrl.phase_interpolator_bypass = 1,
	.txectrl.fine_tuning_7x_clk = 0,
	.txectrl.coarse_tuning_7x_clk = 0,

	.dither_enable = 0,
#endif
#ifdef CONFIG_LCD_CRD080TI01_40NM01
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 162,
	.height = 122,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,

	.lvds = 1,
	.txctrl.data_format = VESA,
	.txctrl.clk_edge_falling_7x = 0,
	.txctrl.clk_edge_falling_1x = 1,
	.txctrl.data_start_edge = START_EDGE_4,
	.txctrl.operate_mode = LVDS_1X_CLKOUT,
	.txctrl.edge_delay = DELAY_0_1NS,
	.txctrl.output_amplitude = VOD_350MV,

	.txpll0.ssc_enable = 0,
	.txpll0.ssc_mode_center_spread = 0,
	.txpll0.post_divider = POST_DIV_1,
	.txpll0.feedback_divider = 70,
	.txpll0.input_divider_bypass = 0,
	.txpll0.input_divider = 10,

	.txpll1.charge_pump = CHARGE_PUMP_10UA,
	.txpll1.vco_gain = VCO_GAIN_150M_400M,
	.txpll1.vco_biasing_current = VCO_BIASING_2_5UA,
	.txpll1.sscn = 0,
	.txpll1.ssc_counter = 0,

	.txectrl.emphasis_level = 0,
	.txectrl.emphasis_enable = 0,
	.txectrl.ldo_output_voltage = LDO_OUTPUT_1V,
	.txectrl.phase_interpolator_bypass = 1,
	.txectrl.fine_tuning_7x_clk = 0,
	.txectrl.coarse_tuning_7x_clk = 0,

	.dither_enable = 0,
#endif
#ifdef CONFIG_LCD_EK070TN93
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

#ifdef CONFIG_BACKLIGHT_PWM
static int npm801_backlight_init(struct device *dev)
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

static void npm801_backlight_exit(struct device *dev)
{
}

static struct platform_pwm_backlight_data npm801_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 80,
	.pwm_period_ns	= 10000, /* 100 kHZ */
	.init		= npm801_backlight_init,
	.exit		= npm801_backlight_exit,
};

/* Backlight Device */
struct platform_device npm801_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &npm801_backlight_data,
	},
};

#endif
