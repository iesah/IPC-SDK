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
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/lcd.h>
#include <linux/of_gpio.h>
#include <soc/gpio.h>
#include <linux/fb.h>
#include <linux/backlight.h>

#include "../ingenicfb.h"
#include "../jz_dsim.h"

#define JZ_LCD_FPS_MAX		0

#define LCD_WIDTH	800
#define LCD_HEIGHT	1280

static struct dsi_cmd_packet mtf070_cmd_list[] = {
#ifdef CONFIG_MIPI_2LANE
	/*{0x15, 0xDE, 0x01, {0x00}}, [>11:4 lane; 01:2 lane<]*/
#endif
#ifdef CONFIG_MIPI_4LANE
	/*{0x15, 0xDE, 0x03, {0x00}}, [>11:4 lane; 01:2 lane<]*/
#endif
	{0x39, 0x05, 0x00, { 0xFF, 0xAA, 0x55, 0xA5, 0x80} },
	{0x39, 0x03, 0x00, { 0x6F,0x11,0x00} },
	{0x39, 0x03, 0x00, { 0xF7,0x20,0x00} },
	{0x15, 0x6F, 0x06, { 0x00}},
	{0x15, 0xF7, 0xA0, { 0x00}},
	{0x15, 0x6F, 0x19, { 0x00}},
	{0x15, 0xF7, 0x12, { 0x00}},
	{0x15, 0xF4, 0x03, { 0x00}},
	{0x15, 0x6F, 0x08, { 0x00}},
	{0x15, 0xFA, 0x40, { 0x00}},
	{0x15, 0x6F, 0x11, { 0x00}},
	{0x15, 0xF3, 0x01, { 0x00}},
	{0x39, 0x06, 0x00, { 0xF0,0x55,0xAA,0x52,0x08,0x00} },
	{0x15, 0xC8, 0x80, { 0x00}},
	{0x39, 0x03, 0x00, { 0xB1,0x6C,0x01} },
	{0x15, 0xB6, 0x08, { 0x00}},
	{0x15, 0x6F, 0x02, { 0x00}},
	{0x15, 0xB8, 0x08, { 0x00}},
	{0x39, 0x03, 0x00, { 0xBB,0x54,0x54} },
	{0x39, 0x03, 0x00, { 0xBC,0x05,0x05} },
	{0x15, 0xC7, 0x01, { 0x00}},
	{0x39, 0x06, 0x00, { 0xBD,0x02,0xB0,0x0C,0x0A,0x00} },
	{0x39, 0x06, 0x00, { 0xF0,0x55,0xAA,0x52,0x08,0x01} },
	{0x39, 0x03, 0x00, { 0xB0,0x05,0x05} },
	{0x39, 0x03, 0x00, { 0xB1,0x05,0x05} },
	{0x39, 0x03, 0x00, { 0xBC,0x3A,0x01} },
	{0x39, 0x03, 0x00, { 0xBD,0x3E,0x01} },
	{0x15, 0xCA, 0x00, { 0x00}},
	{0x15, 0xC0, 0x04, { 0x00}},
	{0x15, 0xB2, 0x00, { 0x00}},
	{0x15, 0xBE, 0x80, { 0x00}},
	{0x39, 0x03, 0x00, { 0xB3,0x19,0x19} },
	{0x39, 0x03, 0x00, { 0xB4,0x12,0x12} },
	{0x39, 0x03, 0x00, { 0xB9,0x24,0x24} },
	{0x39, 0x03, 0x00, { 0xBA,0x14,0x14} },
	{0x39, 0x06, 0x00, { 0xF0,0x55,0xAA,0x52,0x08,0x02} },
	{0x15, 0xEE, 0x01, { 0x00}},
	{0x39, 0x05, 0x00, { 0xEF,0x09,0x06,0x15,0x18} },
	{0x39, 0x07, 0x00, { 0xB0,0x00,0x00,0x00,0x08,0x00,0x17} },
	{0x15, 0x6F, 0x06, { 0x00 }},
	{0x39, 0x07, 0x00, { 0xB0,0x00,0x25,0x00,0x30,0x00,0x45} },
	{0x15, 0x6F, 0x0C, { 0x00 }},
	{0x39, 0x05, 0x00, { 0xB0,0x00,0x56,0x00,0x7a} },
	{0x39, 0x07, 0x00, { 0xB1,0x00,0xa3,0x00,0xe7,0x01,0x20} },
	{0x15, 0x6F, 0x06, { 0x00 }},
	{0x39, 0x07, 0x00, { 0xB1,0x01,0x7a,0x01,0xc2,0x01,0xc5} },
	{0x15, 0x6F, 0x0C, { 0x00 }},
	{0x39, 0x05, 0x00, { 0xB1,0x02,0x06,0x02,0x5f} },
	{0x39, 0x07, 0x00, { 0xB2,0x02,0x92,0x02,0xD0,0x02,0xfc} },
	{0x15, 0x6F, 0x06, { 0x00 }},
	{0x39, 0x07, 0x00, { 0xB2,0x03,0x35,0x03,0x5d,0x03,0x8b} },
	{0x15, 0x6F, 0x0C, { 0x00 }},
	{0x39, 0x05, 0x00, { 0xB2,0x03,0xA2,0x03,0xBF} },
	{0x39, 0x05, 0x00, { 0xB3,0x03,0xD2,0x03,0xFF} },

	{0x39, 0x06, 0x00, { 0xF0,0x55,0xAA,0x52,0x08,0x06} },
	{0x39, 0x03, 0x00, { 0xB0,0x00,0x17} },
	{0x39, 0x03, 0x00, { 0xB1,0x16,0x15} },
	{0x39, 0x03, 0x00, { 0xB2,0x14,0x13} },
	{0x39, 0x03, 0x00, { 0xB3,0x12,0x11} },
	{0x39, 0x03, 0x00, { 0xB4,0x10,0x2D} },
	{0x39, 0x03, 0x00, { 0xB5,0x01,0x08} },
	{0x39, 0x03, 0x00, { 0xB6,0x09,0x31} },
	{0x39, 0x03, 0x00, { 0xB7,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xB8,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xB9,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xBA,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xBB,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xBC,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xBD,0x31,0x09} },
	{0x39, 0x03, 0x00, { 0xBE,0x08,0x01} },
	{0x39, 0x03, 0x00, { 0xBF,0x2D,0x10} },
	{0x39, 0x03, 0x00, { 0xC0,0x11,0x12} },
	{0x39, 0x03, 0x00, { 0xC1,0x13,0x14} },
	{0x39, 0x03, 0x00, { 0xC2,0x15,0x16} },
	{0x39, 0x03, 0x00, { 0xC3,0x17,0x00} },
	{0x39, 0x03, 0x00, { 0xE5,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xC4,0x00,0x17} },
	{0x39, 0x03, 0x00, { 0xC5,0x16,0x15} },
	{0x39, 0x03, 0x00, { 0xC6,0x14,0x13} },
	{0x39, 0x03, 0x00, { 0xC7,0x12,0x11} },
	{0x39, 0x03, 0x00, { 0xC8,0x10,0x2D} },
	{0x39, 0x03, 0x00, { 0xC9,0x01,0x08} },
	{0x39, 0x03, 0x00, { 0xCA,0x09,0x31} },
	{0x39, 0x03, 0x00, { 0xCB,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xCC,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xCD,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xCE,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xCF,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xD0,0x31,0x31} },
	{0x39, 0x03, 0x00, { 0xD1,0x31,0x09} },
	{0x39, 0x03, 0x00, { 0xD2,0x08,0x01} },
	{0x39, 0x03, 0x00, { 0xD3,0x2D,0x10} },
	{0x39, 0x03, 0x00, { 0xD4,0x11,0x12} },
	{0x39, 0x03, 0x00, { 0xD5,0x13,0x14} },
	{0x39, 0x03, 0x00, { 0xD6,0x15,0x16} },
	{0x39, 0x03, 0x00, { 0xD7,0x17,0x00} },
	{0x39, 0x03, 0x00, { 0xE6,0x31,0x31} },
	{0x39, 0x06, 0x00, { 0xD8,0x00,0x00,0x00,0x00,0x00} },
	{0x39, 0x06, 0x00, { 0xD9,0x00,0x00,0x00,0x00,0x00} },
	{0x15, 0xE7, 0x00, { 0x00 } },

	{0x39, 0x06, 0x00, { 0xF0,0x55,0xAA,0x52,0x08,0x03} },
	{0x39, 0x03, 0x00, { 0xB0,0x20,0x00} },
	{0x39, 0x03, 0x00, { 0xB1,0x20,0x00} },
	{0x39, 0x06, 0x00, { 0xB2,0x05,0x00,0x42,0x00,0x00} },
	{0x39, 0x06, 0x00, { 0xB6,0x05,0x00,0x42,0x00,0x00} },
	{0x39, 0x06, 0x00, { 0xBA,0x53,0x00,0x42,0x00,0x00} },
	{0x39, 0x06, 0x00, { 0xBB,0x53,0x00,0x42,0x00,0x00} },
	{0x15, 0xC4, 0x40, { 0x00 } },

	{0x39, 0x06, 0x00, { 0xF0,0x55,0xAA,0x52,0x08,0x05} },
	{0x39, 0x03, 0x00, { 0xB0,0x17,0x06} },
	{0x15, 0xB8, 0x00, { 0x00 } },
	{0x39, 0x06, 0x00, { 0xBD,0x03,0x01,0x01,0x00,0x01} },
	{0x39, 0x03, 0x00, { 0xB1,0x17,0x06} },
	{0x39, 0x03, 0x00, { 0xB9,0x00,0x01} },
	{0x39, 0x03, 0x00, { 0xB2,0x17,0x06} },
	{0x39, 0x03, 0x00, { 0xBA,0x00,0x01} },
	{0x39, 0x03, 0x00, { 0xB3,0x17,0x06} },
	{0x39, 0x03, 0x00, { 0xBB,0x0A,0x00} },
	{0x39, 0x03, 0x00, { 0xB4,0x17,0x06} },
	{0x39, 0x03, 0x00, { 0xB5,0x17,0x06} },
	{0x39, 0x03, 0x00, { 0xB6,0x14,0x03} },
	{0x39, 0x03, 0x00, { 0xB7,0x00,0x00} },
	{0x39, 0x03, 0x00, { 0xBC,0x02,0x01} },
	{0x15, 0xC0, 0x05, { 0x00 } },
	{0x15, 0xC4, 0xA5, { 0x00 } },
	{0x39, 0x03, 0x00, { 0xC8,0x03,0x30} },
	{0x39, 0x03, 0x00, { 0xC9,0x03,0x51} },
	{0x39, 0x06, 0x00, { 0xD1,0x00,0x05,0x03,0x00,0x00} },
	{0x39, 0x06, 0x00, { 0xD2,0x00,0x05,0x09,0x00,0x00} },
	{0x15, 0xE5, 0x02, { 0x00 } },
	{0x15, 0xE6, 0x02, { 0x00 } },
	{0x15, 0xE7, 0x02, { 0x00 } },
	{0x15, 0xE9, 0x02, { 0x00 } },
	{0x15, 0xED, 0x33, { 0x00 } },

	{0x15, 0x11, 0x00, { 0x00 }}, 	//Sleep out
	{0x99, 500, 0x00, {0x00}},		//delay 500ms
	{0x15, 0x29, 0x00, { 0x00}},  	// Display On
	{0x99, 200, 0x00, {0x00}},		//delay 200ms
};

struct board_gpio {
	short gpio;
	short active_level;
};

struct panel_dev {
	/* ingenic frame buffer */
	struct device *dev;
	struct lcd_panel *panel;

	/* common lcd framework */
	struct lcd_device *lcd;
	struct backlight_device *backlight;
	int power;

	struct regulator *vcc;
	struct board_gpio vdd_en;
	struct board_gpio bl;
	struct board_gpio rst;
	struct board_gpio pw;
	struct board_gpio oled;
	struct board_gpio lcd_pwm;

	struct mipi_dsim_lcd_device *dsim_dev;
};

struct panel_dev *panel;

#define lcd_to_master(a)     (a->dsim_dev->master)
#define lcd_to_master_ops(a) ((lcd_to_master(a))->master_ops)

struct mtf070 {
	struct device *dev;
	unsigned int power;
	unsigned int id;

	struct lcd_device *ld;
	struct backlight_device *bd;

	struct mipi_dsim_lcd_device *dsim_dev;
};

struct fb_videomode jzfb_mtf070_videomode[] = {
	{
		.name = "mtf070",
		.refresh = 30,
		.xres = LCD_WIDTH,
		.yres = LCD_HEIGHT,
		.pixclock = KHZ2PICOS(20000),
		.left_margin =  44,
		.right_margin = 48,
		.upper_margin = 112,
		.lower_margin = 120,
		.hsync_len = 10,
		.vsync_len = 4,
		.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0,
	},
};
struct jzdsi_data jzdsi_pdata = {
	.modes = &jzfb_mtf070_videomode[JZ_LCD_FPS_MAX],
#ifdef CONFIG_MIPI_2LANE
	.video_config.no_of_lanes = 2,
#endif
#ifdef CONFIG_MIPI_4LANE
	.video_config.no_of_lanes = 4,
#endif
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
#if 1
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
	.dsi_config.max_bps = 1361,
	/*.max_bps = 650,  // 650 Mbps*/
	.bpp_info = 24,

};

struct tft_config mtf070_cfg = {
	.pix_clk_inv = 0,
	.de_dl = 0,
	.sync_dl = 0,
	.color_even = TFT_LCD_COLOR_EVEN_RGB,
	.color_odd = TFT_LCD_COLOR_ODD_RGB,
	.mode = TFT_LCD_MODE_PARALLEL_888,
};

struct lcd_panel lcd_panel = {
	.num_modes = 1,
	.modes = &jzfb_mtf070_videomode[JZ_LCD_FPS_MAX],
	.dsi_pdata = &jzdsi_pdata,
	//.smart_config = &smart_cfg,
	.tft_config = &mtf070_cfg,

	.lcd_type = LCD_TYPE_TFT ,
	.width = LCD_WIDTH,
	.height = LCD_HEIGHT,
	//.bpp = 24,

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

#define POWER_IS_ON(pwr)    ((pwr) <= FB_BLANK_NORMAL)
static int panel_set_power(struct lcd_device *lcd, int power)
{
	return 0;
}

static int panel_get_power(struct lcd_device *lcd)
{
	struct panel_dev *panel = lcd_get_data(lcd);

	return panel->power;
}

static struct lcd_ops panel_lcd_ops = {
	.early_set_power = panel_set_power,
	.set_power = panel_set_power,
	.get_power = panel_get_power,
};

static int of_panel_parse(struct device *dev)
{
	struct panel_dev *panel = dev_get_drvdata(dev);
	struct device_node *np = dev->of_node;
	enum of_gpio_flags flags;
	int ret = 0;

	panel->bl.gpio = of_get_named_gpio_flags(np, "ingenic,bl-gpio", 0, &flags);
	if (gpio_is_valid(panel->bl.gpio)) {
		panel->bl.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = devm_gpio_request(dev, panel->bl.gpio, "backlight");
		if (ret < 0) {
			dev_err(dev, "Failed to request backlight pin!\n");
			return ret;
		}
	} else
		dev_warn(dev, "invalid gpio backlight.gpio: %d\n", panel->bl.gpio);

	panel->rst.gpio = of_get_named_gpio_flags(np, "ingenic,rst-gpio", 0, &flags);
	if (gpio_is_valid(panel->rst.gpio)) {
		panel->rst.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = devm_gpio_request(dev, panel->rst.gpio, "reset");
		if (ret < 0) {
			dev_err(dev, "Failed to request rst pin!\n");
			return ret;
		}
	} else {
		dev_warn(dev, "invalid gpio rst.gpio: %d\n", panel->rst.gpio);
	}

	panel->pw.gpio = of_get_named_gpio_flags(np, "ingenic,pw-gpio", 0, &flags);
	if (gpio_is_valid(panel->pw.gpio)) {
		panel->pw.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = devm_gpio_request(dev, panel->pw.gpio, "power_on");
		if (ret < 0) {
			dev_err(dev, "Failed to request power_on pin!\n");
			return ret;
		}
	} else {
		dev_warn(dev, "invalid gpio pw.gpio: %d\n", panel->pw.gpio);
	}

	return 0;
}

static void panel_dev_panel_init(struct panel_dev *lcd)
{
#if 0
	int i;
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_device *dsi = lcd_to_master(lcd);

	for (i = 0; i < ARRAY_SIZE(mtf070_cmd_list); i++)
		ops->cmd_write(dsi, mtf070_cmd_list[i]);
	msleep(20);
#endif
	int i, j;
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_device *dsi = lcd_to_master(lcd);
	unsigned char dsi_command_param[MAX_WORD_COUNT] = {0};

	for (i = 0; i < ARRAY_SIZE(mtf070_cmd_list); i++) {
		if (mtf070_cmd_list[i].packet_type == 0x99) {
			/* printk("\nsleep now. time: %dms\n", mtf070_cmd_list[i].cmd0_or_wc_lsb+mtf070_cmd_list[i].cmd1_or_wc_msb); */
			if (mtf070_cmd_list[i].cmd0_or_wc_lsb + mtf070_cmd_list[i].cmd1_or_wc_msb == 510)
				msleep(600);
			else
				msleep(mtf070_cmd_list[i].cmd0_or_wc_lsb + mtf070_cmd_list[i].cmd1_or_wc_msb);
			continue;
		}

		ops->cmd_write(dsi, mtf070_cmd_list[i]);
	}
}

static int panel_dev_ioctl(struct mipi_dsim_lcd_device *dsim_dev, int cmd)
{
	return 0;
}

static void panel_dev_set_sequence(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct mtf070 *lcd = dev_get_drvdata(&dsim_dev->dev);

	panel_dev_panel_init(panel);
	msleep(120);

	lcd->power = FB_BLANK_UNBLANK;
}

static void panel_dev_power_on(struct mipi_dsim_lcd_device *dsim_dev, int power)
{
	struct panel_dev *panel_mtf070 = dev_get_drvdata(panel->dev);

	if (power == 1) {
		/* power on */
		if (gpio_is_valid(panel_mtf070->pw.gpio)){
			printk(">>>>>panel power on\n");
			gpio_direction_output(panel_mtf070->pw.gpio, panel_mtf070->pw.active_level);
			mdelay(150);
		}

		//reset
		if (gpio_is_valid(panel_mtf070->rst.gpio)) {
			gpio_direction_output(panel_mtf070->rst.gpio, !panel_mtf070->rst.active_level);
			mdelay(120);
			gpio_direction_output(panel_mtf070->rst.gpio, panel_mtf070->rst.active_level);
			mdelay(50);
			gpio_direction_output(panel_mtf070->rst.gpio, !panel_mtf070->rst.active_level);
			mdelay(120);
		}
		//open backlight
		if (gpio_is_valid(panel_mtf070->bl.gpio))
			gpio_direction_output(panel_mtf070->bl.gpio, panel_mtf070->bl.active_level);

	} else {
		if (gpio_is_valid(panel_mtf070->bl.gpio))
			gpio_direction_output(panel_mtf070->bl.gpio, !panel_mtf070->bl.active_level);
	}
}

static int panel_dev_probe(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct mtf070 *lcd;

	lcd = devm_kzalloc(&dsim_dev->dev, sizeof(struct mtf070), GFP_KERNEL);
	if (IS_ERR_OR_NULL(lcd)) {
		dev_err(&dsim_dev->dev, "Failed to allocate mtf070 structure!\n");
		return -ENOMEM;
	}

	lcd->dsim_dev = dsim_dev;
	lcd->dev = &dsim_dev->dev;

	lcd->ld = lcd_device_register("mtf070", lcd->dev, lcd,
								  &panel_lcd_ops);
	if (IS_ERR(lcd->ld)) {
		dev_err(lcd->dev, "Failed to register lcd ops.");
		return PTR_ERR(lcd->ld);
	}

	//panel_dev_power_on(dsi);

	dev_set_drvdata(&dsim_dev->dev, lcd);

	dev_dbg(lcd->dev, "Now probed mtf070 panel.\n");

	panel->dsim_dev = dsim_dev;

	return 0;
}

static int panel_dev_suspend(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct board_gpio *vdd_en = &panel->bl;

	gpio_direction_output(vdd_en->gpio, !vdd_en->active_level);

	return 0;
}

static int panel_dev_resume(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct board_gpio *vdd_en = &panel->bl;

	gpio_direction_output(vdd_en->gpio, vdd_en->active_level);

	return 0;
}

static struct mipi_dsim_lcd_driver panel_dev_dsim_ddi_driver = {
	.name = "mtf070",
	.id = -1,

	.power_on = panel_dev_power_on,
	.set_sequence = panel_dev_set_sequence,
	.probe = panel_dev_probe,
	.suspend = panel_dev_suspend,
	.resume = panel_dev_resume,
	.ioctl = panel_dev_ioctl,
};

static struct mipi_dsim_lcd_device panel_dev_device = {
	.name = "mtf070",
	.id = 0,
};

/**
 * @panel_probe
 *
 *   1. register to ingenicfb
 *   2. register to lcd
 *   3. register to backlight if possible
 *
 * @pdev
 *
 * @return -
 */
static int panel_probe(struct platform_device *pdev)
{
	int ret = 0;

	panel = kzalloc(sizeof(struct panel_dev), GFP_KERNEL);
	if (IS_ERR_OR_NULL(panel)) {
		dev_err(&pdev->dev, "Failed to alloc memory!\n");
		return -ENOMEM;
	}
	panel->dev = &pdev->dev;
	dev_set_drvdata(&pdev->dev, panel);

	ret = of_panel_parse(&pdev->dev);
	if (ret < 0)
		goto err_of_parse;

	/* register to mipi-dsi devicelist*/
	mipi_dsi_register_lcd_device(&panel_dev_device);
	mipi_dsi_register_lcd_driver(&panel_dev_dsim_ddi_driver);

	ret = ingenicfb_register_panel(&lcd_panel);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register lcd panel!\n");
		goto err_of_parse;
	}

	return 0;
err_of_parse:
	kfree(panel);
	return ret;
}

static int panel_remove(struct platform_device *dev)
{
	if (NULL != panel)
		kfree(panel);

	return 0;
}

#ifdef CONFIG_PM
static int panel_suspend(struct device *dev)
{
	struct panel_dev *panel = dev_get_drvdata(dev);

	panel_set_power(panel->lcd, FB_BLANK_POWERDOWN);
	return 0;
}

static int panel_resume(struct device *dev)
{
	struct panel_dev *panel = dev_get_drvdata(dev);

	panel_set_power(panel->lcd, FB_BLANK_UNBLANK);
	return 0;
}

static const struct dev_pm_ops panel_pm_ops = {
	.suspend = panel_suspend,
	.resume = panel_resume,
};
#endif
static const struct of_device_id panel_of_match[] = {
	{ .compatible = "ingenic,mtf070", },
	{ .compatible = "mtf070", },
	{},
};

static struct platform_driver panel_driver = {
	.probe = panel_probe,
	.remove = panel_remove,
	.driver = {
		.name = "mtf070",
		.of_match_table = panel_of_match,
#ifdef CONFIG_PM
		.pm = &panel_pm_ops,
#endif
	},
};

module_platform_driver(panel_driver);
