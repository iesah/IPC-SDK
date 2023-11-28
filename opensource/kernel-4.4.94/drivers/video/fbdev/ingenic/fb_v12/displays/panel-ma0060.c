/*
 * driver/video/fbdev/ingenic/x2000_v12/displays/panel-ma0060.c
 *
 * Copyright (C) 2016 Ingenic Semiconductor Inc.
 *
 * This program is free software, you can redistribute it and/or modify it
 *
 * under the terms of the GNU General Public License version 2 as published by
 *
 * the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/delay.h>
#include <linux/lcd.h>
#include <linux/of_gpio.h>
#include <linux/fb.h>
#include <linux/backlight.h>


#include "../ingenicfb.h"
#include "../jz_dsim.h"

/* #define _1080P */

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
	struct board_gpio rst;
	struct board_gpio oled;
	struct board_gpio lcd_pwm;
	struct board_gpio swire;

	struct mipi_dsim_lcd_device *dsim_dev;
};

struct panel_dev *panel;

#define lcd_to_master(a)	(a->dsim_dev->master)
#define lcd_to_master_ops(a)	((lcd_to_master(a))->master_ops)

struct ma0060 {
	struct device *dev;
	unsigned int power;
	unsigned int id;

	struct lcd_device *ld;
	struct backlight_device *bd;

	struct mipi_dsim_lcd_device *dsim_dev;

};

static struct dsi_cmd_packet visionox_ma0060_720p_cmd_list[] =
{
	{0X39, 0X02, 0X00, {0XFE, 0XD0}},
	{0X39, 0X02, 0X00, {0X07, 0X84}},
	{0X39, 0X02, 0X00, {0X40, 0X02}},
	{0X39, 0X02, 0X00, {0X4B, 0X4C}},
	{0X39, 0X02, 0X00, {0X49, 0X01}},
	{0X39, 0X02, 0X00, {0XFE, 0X40}},
	{0X39, 0X02, 0X00, {0XC7, 0X85}},
	{0X39, 0X02, 0X00, {0XC8, 0X32}},
	{0X39, 0X02, 0X00, {0XC9, 0X18}},
	{0X39, 0X02, 0X00, {0XCA, 0X09}},
	{0X39, 0X02, 0X00, {0XCB, 0X22}},
	{0X39, 0X02, 0X00, {0XCC, 0X44}},
	{0X39, 0X02, 0X00, {0XCD, 0X11}},
	{0X39, 0X02, 0X00, {0X05, 0X0F}},
	{0X39, 0X02, 0X00, {0X06, 0X09}},
	{0X39, 0X02, 0X00, {0X08, 0X0F}},
	{0X39, 0X02, 0X00, {0X09, 0X09}},
	{0X39, 0X02, 0X00, {0X0A, 0XE6}},
	{0X39, 0X02, 0X00, {0X0B, 0X88}},
	{0X39, 0X02, 0X00, {0X0D, 0X90}},
	{0X39, 0X02, 0X00, {0X0E, 0X10}},
	{0X39, 0X02, 0X00, {0X20, 0X93}},
	{0X39, 0X02, 0X00, {0X21, 0X93}},
	{0X39, 0X02, 0X00, {0X24, 0X02}},
	{0X39, 0X02, 0X00, {0X26, 0X02}},
	{0X39, 0X02, 0X00, {0X28, 0X05}},
	{0X39, 0X02, 0X00, {0X2A, 0X05}},
	{0X39, 0X02, 0X00, {0X2D, 0X23}},
	{0X39, 0X02, 0X00, {0X2F, 0X23}},
	{0X39, 0X02, 0X00, {0X30, 0X23}},
	{0X39, 0X02, 0X00, {0X31, 0X23}},
	{0X39, 0X02, 0X00, {0X36, 0X55}},
	{0X39, 0X02, 0X00, {0X37, 0X80}},
	{0X39, 0X02, 0X00, {0X38, 0X50}},
	{0X39, 0X02, 0X00, {0X39, 0X00}},
	{0X39, 0X02, 0X00, {0X46, 0X27}},
	{0X39, 0X02, 0X00, {0X6F, 0X00}},
	{0X39, 0X02, 0X00, {0X74, 0X2F}},
	{0X39, 0X02, 0X00, {0X75, 0X19}},
	{0X39, 0X02, 0X00, {0X79, 0X00}},
	{0X39, 0X02, 0X00, {0XAD, 0X00}},
	{0X39, 0X02, 0X00, {0XFE, 0X60}},
	{0X39, 0X02, 0X00, {0X00, 0XC4}},
	{0X39, 0X02, 0X00, {0X01, 0X00}},
	{0X39, 0X02, 0X00, {0X02, 0X02}},
	{0X39, 0X02, 0X00, {0X03, 0X00}},
	{0X39, 0X02, 0X00, {0X04, 0X00}},
	{0X39, 0X02, 0X00, {0X05, 0X07}},
	{0X39, 0X02, 0X00, {0X06, 0X00}},
	{0X39, 0X02, 0X00, {0X07, 0X83}},
	{0X39, 0X02, 0X00, {0X09, 0XC4}},
	{0X39, 0X02, 0X00, {0X0A, 0X00}},
	{0X39, 0X02, 0X00, {0X0B, 0X02}},
	{0X39, 0X02, 0X00, {0X0C, 0X00}},
	{0X39, 0X02, 0X00, {0X0D, 0X00}},
	{0X39, 0X02, 0X00, {0X0E, 0X08}},
	{0X39, 0X02, 0X00, {0X0F, 0X00}},
	{0X39, 0X02, 0X00, {0X10, 0X83}},
	{0X39, 0X02, 0X00, {0X12, 0XCC}},
	{0X39, 0X02, 0X00, {0X13, 0X0F}},
	{0X39, 0X02, 0X00, {0X14, 0XFF}},
	{0X39, 0X02, 0X00, {0X15, 0X01}},
	{0X39, 0X02, 0X00, {0X16, 0X00}},
	{0X39, 0X02, 0X00, {0X17, 0X02}},
	{0X39, 0X02, 0X00, {0X18, 0X00}},
	{0X39, 0X02, 0X00, {0X19, 0X00}},
	{0X39, 0X02, 0X00, {0X1B, 0XC4}},
	{0X39, 0X02, 0X00, {0X1C, 0X00}},
	{0X39, 0X02, 0X00, {0X1D, 0X02}},
	{0X39, 0X02, 0X00, {0X1E, 0X00}},
	{0X39, 0X02, 0X00, {0X1F, 0X00}},
	{0X39, 0X02, 0X00, {0X20, 0X08}},
	{0X39, 0X02, 0X00, {0X21, 0X00}},
	{0X39, 0X02, 0X00, {0X22, 0X89}},
	{0X39, 0X02, 0X00, {0X24, 0XC4}},
	{0X39, 0X02, 0X00, {0X25, 0X00}},
	{0X39, 0X02, 0X00, {0X26, 0X02}},
	{0X39, 0X02, 0X00, {0X27, 0X00}},
	{0X39, 0X02, 0X00, {0X28, 0X00}},
	{0X39, 0X02, 0X00, {0X29, 0X07}},
	{0X39, 0X02, 0X00, {0X2A, 0X00}},
	{0X39, 0X02, 0X00, {0X2B, 0X89}},
	{0X39, 0X02, 0X00, {0X2F, 0XC4}},
	{0X39, 0X02, 0X00, {0X30, 0X00}},
	{0X39, 0X02, 0X00, {0X31, 0X02}},
	{0X39, 0X02, 0X00, {0X32, 0X00}},
	{0X39, 0X02, 0X00, {0X33, 0X00}},
	{0X39, 0X02, 0X00, {0X34, 0X06}},
	{0X39, 0X02, 0X00, {0X35, 0X00}},
	{0X39, 0X02, 0X00, {0X36, 0X89}},
	{0X39, 0X02, 0X00, {0X38, 0XC4}},
	{0X39, 0X02, 0X00, {0X39, 0X00}},
	{0X39, 0X02, 0X00, {0X3A, 0X02}},
	{0X39, 0X02, 0X00, {0X3B, 0X00}},
	{0X39, 0X02, 0X00, {0X3D, 0X00}},
	{0X39, 0X02, 0X00, {0X3F, 0X07}},
	{0X39, 0X02, 0X00, {0X40, 0X00}},
	{0X39, 0X02, 0X00, {0X41, 0X89}},
	{0X39, 0X02, 0X00, {0X4C, 0XC4}},
	{0X39, 0X02, 0X00, {0X4D, 0X00}},
	{0X39, 0X02, 0X00, {0X4E, 0X04}},
	{0X39, 0X02, 0X00, {0X4F, 0X01}},
	{0X39, 0X02, 0X00, {0X50, 0X00}},
	{0X39, 0X02, 0X00, {0X51, 0X08}},
	{0X39, 0X02, 0X00, {0X52, 0X00}},
	{0X39, 0X02, 0X00, {0X53, 0X61}},
	{0X39, 0X02, 0X00, {0X55, 0XC4}},
	{0X39, 0X02, 0X00, {0X56, 0X00}},
	{0X39, 0X02, 0X00, {0X58, 0X04}},
	{0X39, 0X02, 0X00, {0X59, 0X01}},
	{0X39, 0X02, 0X00, {0X5A, 0X00}},
	{0X39, 0X02, 0X00, {0X5B, 0X06}},
	{0X39, 0X02, 0X00, {0X5C, 0X00}},
	{0X39, 0X02, 0X00, {0X5D, 0X61}},
	{0X39, 0X02, 0X00, {0X5F, 0XCE}},
	{0X39, 0X02, 0X00, {0X60, 0X00}},
	{0X39, 0X02, 0X00, {0X61, 0X07}},
	{0X39, 0X02, 0X00, {0X62, 0X05}},
	{0X39, 0X02, 0X00, {0X63, 0X00}},
	{0X39, 0X02, 0X00, {0X64, 0X04}},
	{0X39, 0X02, 0X00, {0X65, 0X00}},
	{0X39, 0X02, 0X00, {0X66, 0X60}},
	{0X39, 0X02, 0X00, {0X67, 0X80}},
	{0X39, 0X02, 0X00, {0X9B, 0X03}},
	{0X39, 0X02, 0X00, {0XA9, 0X10}},
	{0X39, 0X02, 0X00, {0XAA, 0X10}},
	{0X39, 0X02, 0X00, {0XAB, 0X02}},
	{0X39, 0X02, 0X00, {0XAC, 0X04}},
	{0X39, 0X02, 0X00, {0XAD, 0X03}},
	{0X39, 0X02, 0X00, {0XAE, 0X10}},
	{0X39, 0X02, 0X00, {0XAF, 0X10}},
	{0X39, 0X02, 0X00, {0XB0, 0X10}},
	{0X39, 0X02, 0X00, {0XB1, 0X10}},
	{0X39, 0X02, 0X00, {0XB2, 0X10}},
	{0X39, 0X02, 0X00, {0XB3, 0X10}},
	{0X39, 0X02, 0X00, {0XB4, 0X10}},
	{0X39, 0X02, 0X00, {0XB5, 0X10}},
	{0X39, 0X02, 0X00, {0XB6, 0X10}},
	{0X39, 0X02, 0X00, {0XB7, 0X10}},
	{0X39, 0X02, 0X00, {0XB8, 0X10}},
	{0X39, 0X02, 0X00, {0XB9, 0X08}},
	{0X39, 0X02, 0X00, {0XBA, 0X09}},
	{0X39, 0X02, 0X00, {0XBB, 0X0A}},
	{0X39, 0X02, 0X00, {0XBC, 0X05}},
	{0X39, 0X02, 0X00, {0XBD, 0X06}},
	{0X39, 0X02, 0X00, {0XBE, 0X02}},
	{0X39, 0X02, 0X00, {0XBF, 0X10}},
	{0X39, 0X02, 0X00, {0XC0, 0X10}},
	{0X39, 0X02, 0X00, {0XC1, 0X03}},
	{0X39, 0X02, 0X00, {0XC4, 0X80}},
	{0X39, 0X02, 0X00, {0XFE, 0X70}},
	{0X39, 0X02, 0X00, {0X48, 0X05}},
	{0X39, 0X02, 0X00, {0X52, 0X00}},
	{0X39, 0X02, 0X00, {0X5A, 0XFF}},
	{0X39, 0X02, 0X00, {0X5C, 0XF6}},
	{0X39, 0X02, 0X00, {0X5D, 0X07}},
	{0X39, 0X02, 0X00, {0X7D, 0X75}},
	{0X39, 0X02, 0X00, {0X86, 0X07}},
	{0X39, 0X02, 0X00, {0XA7, 0X02}},
	{0X39, 0X02, 0X00, {0XA9, 0X2C}},
	{0X39, 0X02, 0X00, {0XFE, 0XA0}},
	{0X39, 0X02, 0X00, {0X2B, 0X18}},

	{0x39, 0x02, 0x00, {0xFE, 0xD0}}, /* 【2 lane 设置】 */
	{0x39, 0x02, 0x00, {0x1E, 0x05}},

	{0x39, 0x02, 0x00, {0xFE, 0x00}},
	{0x39, 0x05, 0x00, {0x2A, 0x00, 0x00, 0x02, 0xCF}},
	{0x39, 0x05, 0x00, {0x2B, 0x00, 0x00, 0x04, 0xFF}},

	{0x39, 0x02, 0x00, {0x11, 0x00}},
	{0x99, 120, 0x00, {0x00}},

	{0x39, 0x02, 0x00, {0x29, 0x00}},
	{0x99, 50, 0x00, {0x00}},

};

static struct dsi_cmd_packet visionox_ma0060_1080p_cmd_list[] =
{
#if 1
	{0x99, 120, 0x00, {0x00}},		//delay 250ms
	{0x39, 0x02, 0x00, {0xFE, 0x04}},
	{0x39, 0x02, 0x00, {0x01, 0x08}},
	{0x39, 0x02, 0x00, {0x0D, 0x80}},
	{0x39, 0x02, 0x00, {0x05, 0x07}},
	{0x39, 0x02, 0x00, {0x06, 0x09}},
	{0x39, 0x02, 0x00, {0x0A, 0xDB}},
	{0x39, 0x02, 0x00, {0x0B, 0x8C}},
	{0x39, 0x02, 0x00, {0x0E, 0x10}},
	{0x39, 0x02, 0x00, {0x0F, 0xE0}},
	{0x39, 0x02, 0x00, {0x1A, 0x11}},
	{0x39, 0x02, 0x00, {0x29, 0x93}},
	{0x39, 0x02, 0x00, {0x2A, 0x93}},
	{0x39, 0x02, 0x00, {0x2F, 0x02}},
	{0x39, 0x02, 0x00, {0x31, 0x02}},
	{0x39, 0x02, 0x00, {0x33, 0x05}},
	{0x39, 0x02, 0x00, {0x37, 0x23}},
	{0x39, 0x02, 0x00, {0x38, 0x23}},
	{0x39, 0x02, 0x00, {0x3A, 0x23}},
	{0x39, 0x02, 0x00, {0x3B, 0x23}},
	{0x39, 0x02, 0x00, {0x3D, 0x2C}},
	{0x39, 0x02, 0x00, {0x3F, 0x80}},
	{0x39, 0x02, 0x00, {0x40, 0x50}},
	{0x39, 0x02, 0x00, {0x41, 0x8E}},
	{0x39, 0x02, 0x00, {0x4F, 0x2F}},
	{0x39, 0x02, 0x00, {0x50, 0x19}},
	{0x39, 0x02, 0x00, {0x51, 0x0A}},

	{0x39, 0x02, 0x00, {0xFE, 0x07}},
	{0x39, 0x02, 0x00, {0x03, 0x40}},
	{0x39, 0x02, 0x00, {0x05, 0x00}},
	{0x39, 0x02, 0x00, {0x07, 0x0A}},

	{0x39, 0x02, 0x00, {0xFE, 0x06}},
	{0x39, 0x02, 0x00, {0xB0, 0x11}},
	{0x39, 0x02, 0x00, {0x00, 0xE4}},
	{0x39, 0x02, 0x00, {0x01, 0x0F}},
	{0x39, 0x02, 0x00, {0x02, 0xFF}},
	{0x39, 0x02, 0x00, {0x03, 0x00}},
	{0x39, 0x02, 0x00, {0x04, 0x00}},
	{0x39, 0x02, 0x00, {0x05, 0x02}},
	{0x39, 0x02, 0x00, {0x06, 0x00}},
	{0x39, 0x02, 0x00, {0x07, 0xC0}},
	{0x39, 0x02, 0x00, {0x08, 0xE4}},
	{0x39, 0x02, 0x00, {0x09, 0x00}},
	{0x39, 0x02, 0x00, {0x0A, 0x02}},
	{0x39, 0x02, 0x00, {0x0B, 0x00}},
	{0x39, 0x02, 0x00, {0x0C, 0x00}},
	{0x39, 0x02, 0x00, {0x0D, 0x08}},
	{0x39, 0x02, 0x00, {0x0E, 0x00}},
	{0x39, 0x02, 0x00, {0x0F, 0x83}},
	{0x39, 0x02, 0x00, {0x10, 0xE4}},
	{0x39, 0x02, 0x00, {0x11, 0x00}},
	{0x39, 0x02, 0x00, {0x12, 0x02}},
	{0x39, 0x02, 0x00, {0x13, 0x00}},
	{0x39, 0x02, 0x00, {0x14, 0x00}},
	{0x39, 0x02, 0x00, {0x15, 0x07}},
	{0x39, 0x02, 0x00, {0x16, 0x00}},
	{0x39, 0x02, 0x00, {0x17, 0x67}},
	{0x39, 0x02, 0x00, {0x18, 0xE4}},
	{0x39, 0x02, 0x00, {0x19, 0x00}},
	{0x39, 0x02, 0x00, {0x1A, 0x02}},
	{0x39, 0x02, 0x00, {0x1B, 0x00}},
	{0x39, 0x02, 0x00, {0x1C, 0x00}},
	{0x39, 0x02, 0x00, {0x1D, 0x07}},
	{0x39, 0x02, 0x00, {0x1E, 0x00}},
	{0x39, 0x02, 0x00, {0x1F, 0x67}},
	{0x39, 0x02, 0x00, {0x20, 0xE4}},
	{0x39, 0x02, 0x00, {0x21, 0x00}},
	{0x39, 0x02, 0x00, {0x22, 0x02}},
	{0x39, 0x02, 0x00, {0x23, 0x00}},
	{0x39, 0x02, 0x00, {0x24, 0x00}},
	{0x39, 0x02, 0x00, {0x25, 0x08}},
	{0x39, 0x02, 0x00, {0x26, 0x00}},
	{0x39, 0x02, 0x00, {0x27, 0x83}},
	{0x39, 0x02, 0x00, {0x28, 0xE4}},
	{0x39, 0x02, 0x00, {0x29, 0x00}},
	{0x39, 0x02, 0x00, {0x2A, 0x02}},
	{0x39, 0x02, 0x00, {0x2B, 0x00}},
	{0x39, 0x02, 0x00, {0x2D, 0x00}},
	{0x39, 0x02, 0x00, {0x2F, 0x07}},
	{0x39, 0x02, 0x00, {0x30, 0x00}},
	{0x39, 0x02, 0x00, {0x31, 0x67}},
	{0x39, 0x02, 0x00, {0x32, 0xE4}},
	{0x39, 0x02, 0x00, {0x33, 0x00}},
	{0x39, 0x02, 0x00, {0x34, 0x02}},
	{0x39, 0x02, 0x00, {0x35, 0x00}},
	{0x39, 0x02, 0x00, {0x36, 0x00}},
	{0x39, 0x02, 0x00, {0x37, 0x08}},
	{0x39, 0x02, 0x00, {0x38, 0x00}},
	{0x39, 0x02, 0x00, {0x39, 0x67}},
	{0x39, 0x02, 0x00, {0x44, 0xE4}},
	{0x39, 0x02, 0x00, {0x45, 0x00}},
	{0x39, 0x02, 0x00, {0x46, 0x04}},
	{0x39, 0x02, 0x00, {0x47, 0x01}},
	{0x39, 0x02, 0x00, {0x48, 0x00}},
	{0x39, 0x02, 0x00, {0x49, 0x08}},
	{0x39, 0x02, 0x00, {0x4A, 0x00}},
	{0x39, 0x02, 0x00, {0x4B, 0x64}},
	{0x39, 0x02, 0x00, {0x4C, 0xE4}},
	{0x39, 0x02, 0x00, {0x4D, 0x00}},
	{0x39, 0x02, 0x00, {0x4E, 0x04}},
	{0x39, 0x02, 0x00, {0x4F, 0x01}},
	{0x39, 0x02, 0x00, {0x50, 0x00}},
	{0x39, 0x02, 0x00, {0x51, 0x06}},
	{0x39, 0x02, 0x00, {0x52, 0x00}},
	{0x39, 0x02, 0x00, {0x53, 0x64}},
	{0x39, 0x02, 0x00, {0x8D, 0xEA}},
	{0x39, 0x02, 0x00, {0x8E, 0x0F}},
	{0x39, 0x02, 0x00, {0x8F, 0xFF}},
	{0x39, 0x02, 0x00, {0x90, 0x05}},
	{0x39, 0x02, 0x00, {0x91, 0x00}},
	{0x39, 0x02, 0x00, {0x92, 0x04}},
	{0x39, 0x02, 0x00, {0x93, 0x00}},
	{0x39, 0x02, 0x00, {0x94, 0xAB}},
	{0x39, 0x02, 0x00, {0xAC, 0x00}},
	{0x39, 0x02, 0x00, {0xA6, 0x20}},
	{0x39, 0x02, 0x00, {0xA7, 0x04}},
	{0x39, 0x02, 0x00, {0xA9, 0x58}},
	{0x39, 0x02, 0x00, {0xB1, 0x17}},
	{0x39, 0x02, 0x00, {0xB2, 0x17}},
	{0x39, 0x02, 0x00, {0xB3, 0x00}},
	{0x39, 0x02, 0x00, {0xB4, 0x03}},
	{0x39, 0x02, 0x00, {0xB5, 0x04}},
	{0x39, 0x02, 0x00, {0xB6, 0x17}},
	{0x39, 0x02, 0x00, {0xB7, 0x17}},
	{0x39, 0x02, 0x00, {0xB8, 0x17}},
	{0x39, 0x02, 0x00, {0xB9, 0x17}},
	{0x39, 0x02, 0x00, {0xBA, 0x17}},
	{0x39, 0x02, 0x00, {0xBB, 0x17}},
	{0x39, 0x02, 0x00, {0xBC, 0x17}},
	{0x39, 0x02, 0x00, {0xBD, 0x17}},
	{0x39, 0x02, 0x00, {0xBE, 0x17}},
	{0x39, 0x02, 0x00, {0xBF, 0x17}},
	{0x39, 0x02, 0x00, {0xC0, 0x17}},
	{0x39, 0x02, 0x00, {0xC1, 0x08}},
	{0x39, 0x02, 0x00, {0xC2, 0x09}},
	{0x39, 0x02, 0x00, {0xC3, 0x11}},
	{0x39, 0x02, 0x00, {0xC4, 0x06}},
	{0x39, 0x02, 0x00, {0xC5, 0x05}},
	{0x39, 0x02, 0x00, {0xC6, 0x00}},
	{0x39, 0x02, 0x00, {0xC7, 0x17}},
	{0x39, 0x02, 0x00, {0xC8, 0x17}},

	{0x39, 0x02, 0x00, {0xFE, 0x09}},
	{0x39, 0x02, 0x00, {0x15, 0x92}},
	{0x39, 0x02, 0x00, {0x16, 0x24}},
	{0x39, 0x02, 0x00, {0x17, 0x49}},
	{0x39, 0x02, 0x00, {0x1B, 0x92}},
	{0x39, 0x02, 0x00, {0x1C, 0x24}},
	{0x39, 0x02, 0x00, {0x1D, 0x49}},
	{0x39, 0x02, 0x00, {0x21, 0x92}},
	{0x39, 0x02, 0x00, {0x22, 0x24}},
	{0x39, 0x02, 0x00, {0x23, 0x49}},
	{0x39, 0x02, 0x00, {0x9B, 0x47}},
	{0x39, 0x02, 0x00, {0x9C, 0x81}},
	{0x39, 0x02, 0x00, {0xA3, 0x18}},

	{0x39, 0x02, 0x00, {0xFE, 0x0A}},
	{0x39, 0x02, 0x00, {0x25, 0x66}},
	{0x39, 0x02, 0x00, {0xFE, 0x0E}},
	{0x39, 0x02, 0x00, {0x12, 0x33}},
	{0x99, 20, 0x00, {0x00}},		//delay 20ms

	{0x39, 0x02, 0x00, {0xFE, 0x00}},
	{0x39, 0x02, 0x00, {0xc2, 0x08}},
	{0x39, 0x02, 0x00, {0x35, 0x00}},

	{0x39, 0x02, 0x00, {0x11, 0x00}},
	{0x99, 120, 0x00, {0x00}},		//delay 120ms

	{0x39, 0x02, 0x00, {0x29, 0x00}},
	{0x99, 50, 0x00, {0x00}},		//delay 120ms
#endif
};

static void panel_dev_sleep_in(struct panel_dev *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x10, 0x00};

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
}

static void panel_dev_sleep_out(struct panel_dev *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x11, 0x00};

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
}

static void panel_dev_display_on(struct panel_dev *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x29, 0x00};
	/* struct dsi_cmd_packet data_to_send1 = {0x15, 0x51, 250}; */

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
	/* ops->cmd_write(lcd_to_master(lcd), data_to_send1); */
}

static void panel_dev_display_off(struct panel_dev *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x28, 0x00};

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
}


static void panel_dev_panel_init(struct panel_dev *lcd)
{
	int  i;
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_device *dsi = lcd_to_master(lcd);


#ifdef _1080P
	for (i = 0; i < ARRAY_SIZE(visionox_ma0060_1080p_cmd_list); i++)
	{
		if (visionox_ma0060_1080p_cmd_list[i].packet_type == 0x99) {
			msleep(visionox_ma0060_1080p_cmd_list[i].cmd0_or_wc_lsb + visionox_ma0060_1080p_cmd_list[i].cmd1_or_wc_msb);
			continue;
		}
		ops->cmd_write(dsi, visionox_ma0060_1080p_cmd_list[i]);
	}
#else
	for (i = 0; i < ARRAY_SIZE(visionox_ma0060_720p_cmd_list); i++)
	{
		if (visionox_ma0060_720p_cmd_list[i].packet_type == 0x99) {
			msleep(visionox_ma0060_720p_cmd_list[i].cmd0_or_wc_lsb + visionox_ma0060_720p_cmd_list[i].cmd1_or_wc_msb);
			continue;
		}
		ops->cmd_write(dsi,  visionox_ma0060_720p_cmd_list[i]);
	}
#endif
}

static int panel_dev_ioctl(struct mipi_dsim_lcd_device *dsim_dev, int cmd)
{
	return 0;
}
static void panel_dev_set_sequence(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct ma0060 *lcd = dev_get_drvdata(&dsim_dev->dev);

	panel_dev_panel_init(panel);
	lcd->power = FB_BLANK_UNBLANK;
}
static void panel_dev_power_on(struct mipi_dsim_lcd_device *dsim_dev, int power)
{
	struct board_gpio *vdd_en = &panel->vdd_en;
	struct board_gpio *rst = &panel->rst;
	struct board_gpio *oled = &panel->oled;
	struct board_gpio *swire = &panel->swire;


	gpio_direction_output(vdd_en->gpio, vdd_en->active_level);

	if (gpio_is_valid(swire->gpio)) {
		gpio_direction_input(swire->gpio);
	}

	if (gpio_is_valid(oled->gpio)) {
		gpio_direction_input(oled->gpio);
	}
	msleep(100);

	gpio_direction_output(rst->gpio, 1);
	msleep(100);
	gpio_direction_output(rst->gpio, 0);
	msleep(150);
	gpio_direction_output(rst->gpio, 1);
	msleep(100);

	panel->power = power;
}

static struct fb_videomode panel_modes = {
	.name = "visionox_ma0060-lcd",
#ifdef _1080P
	.refresh = 45,
	.xres = 1080,
	.yres = 1920,
#else
	.refresh = 60,
	.xres = 720,
	.yres = 1280,
#endif
	.left_margin = 36,
	.right_margin = 26,
	.upper_margin = 4,
	.lower_margin = 8,

	.hsync_len = 2,
	.vsync_len = 4,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct jzdsi_data jzdsi_pdata = {
	.modes = &panel_modes,

#ifdef _1080P
	.video_config.no_of_lanes = 4,
#else
	.video_config.no_of_lanes = 2,
#endif
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.video_config.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.video_config.is_18_loosely = 0,
	.video_config.data_en_polarity = 1,
	.video_config.byte_clock = 0, // driver will auto calculate byte_clock.
	.video_config.byte_clock_coef = MIPI_PHY_BYTE_CLK_COEF_MUL6_DIV5, //for auto calculate byte clock

	.dsi_config.max_lanes = 4,
	.dsi_config.max_hs_to_lp_cycles = 100,
	.dsi_config.max_lp_to_hs_cycles = 40,
	.dsi_config.max_bta_cycles = 4095,
	.dsi_config.color_mode_polarity = 1,
	.dsi_config.shut_down_polarity = 1,
	.dsi_config.max_bps = 2750,
	.bpp_info = 24,
};

struct smart_config smart_cfg = {
	.smart_type = SMART_LCD_TYPE_8080,
	.pix_fmt = SMART_LCD_FORMAT_888,
	.dwidth = SMART_LCD_DWIDTH_24_BIT,
};

struct lcd_panel lcd_panel = {
	.num_modes = 1,
	.modes = &panel_modes,
	.dsi_pdata = &jzdsi_pdata,
	.smart_config = &smart_cfg,
	.lcd_type = LCD_TYPE_MIPI_SLCD,

	.bpp = 24,
#ifdef _1080P
	.width = 1080,
	.height = 1920,
#else
	.width = 720,
	.height = 1280,
#endif
};

#define POWER_IS_ON(pwr)        ((pwr) <= FB_BLANK_NORMAL)
static int panel_set_power(struct lcd_device *lcd, int power)
{
        return 0;
}

static int panel_get_power(struct lcd_device *lcd)
{
	struct panel_dev *panel = lcd_get_data(lcd);

	return panel->power;
}

/**
* @ pannel_ma0060_lcd_ops, register to kernel common backlight/lcd.c framworks.
*/
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

	panel->vdd_en.gpio = of_get_named_gpio_flags(np, "ingenic,vdd-en-gpio", 0, &flags);
	if(gpio_is_valid(panel->vdd_en.gpio)) {
		panel->vdd_en.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = gpio_request_one(panel->vdd_en.gpio, GPIOF_DIR_OUT, "vdd_en");
		if(ret < 0) {
			dev_err(dev, "Failed to request vdd_en pin!\n");
			return ret;
		}
	} else {
		dev_warn(dev, "invalid gpio vdd_en.gpio: %d\n", panel->vdd_en.gpio);
	}

	panel->rst.gpio = of_get_named_gpio_flags(np, "ingenic,rst-gpio", 0, &flags);
	if(gpio_is_valid(panel->rst.gpio)) {
		panel->rst.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = gpio_request_one(panel->rst.gpio, GPIOF_DIR_OUT, "rst");
		if(ret < 0) {
			dev_err(dev, "Failed to request rst pin!\n");
			goto err_request_rst;
		}
	} else {
		dev_warn(dev, "invalid gpio rst.gpio: %d\n", panel->rst.gpio);
	}

	/* panel->oled.gpio = of_get_named_gpio_flags(np, "ingenic,oled-gpio", 0, &flags);
	if(gpio_is_valid(panel->oled.gpio)) {
		panel->oled.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = gpio_request_one(panel->oled.gpio, GPIOF_DIR_OUT, "oled");
		if(ret < 0) {
			dev_err(dev, "Failed to request oled pin!\n");
			goto err_request_oled;
		}
	} else {
		dev_warn(dev, "invalid gpio oled.gpio: %d\n", panel->oled.gpio);
	} */

	panel->lcd_pwm.gpio = of_get_named_gpio_flags(np, "ingenic,lcd-pwm-gpio", 0, &flags);
	if(gpio_is_valid(panel->lcd_pwm.gpio)) {
		panel->lcd_pwm.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = gpio_request_one(panel->lcd_pwm.gpio, GPIOF_DIR_OUT, "lcd-pwm");
		if(ret < 0) {
			dev_err(dev, "Failed to request lcd-pwm pin!\n");
			goto err_request_pwm;
		}
	} else {
		dev_warn(dev, "invalid gpio lcd-pwm.gpio: %d\n", panel->lcd_pwm.gpio);
	}

	/* panel->swire.gpio = of_get_named_gpio_flags(np, "ingenic,swire-gpio", 0, &flags);
	if(gpio_is_valid(panel->swire.gpio)) {
		panel->swire.active_level = (flags & OF_GPIO_ACTIVE_LOW) ? 0 : 1;
		ret = gpio_request_one(panel->swire.gpio, GPIOF_DIR_OUT, "swire");
		if(ret < 0) {
			dev_err(dev, "Failed to request swire pin!\n");
			goto err_request_swire;
		}
	} else {
		dev_warn(dev, "invalid gpio swire.gpio: %d\n", panel->swire.gpio);
	} */

	return 0;
/* err_request_swire:
	if(gpio_is_valid(panel->lcd_pwm.gpio))
		gpio_free(panel->lcd_pwm.gpio); */
err_request_pwm:
	/* if(gpio_is_valid(panel->oled.gpio)) */
		/* gpio_free(panel->oled.gpio); */
/* err_request_oled: */
	if(gpio_is_valid(panel->rst.gpio))
		gpio_free(panel->rst.gpio);
err_request_rst:
	if(gpio_is_valid(panel->vdd_en.gpio))
		gpio_free(panel->vdd_en.gpio);
	return ret;
}

static int panel_dev_probe(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct ma0060 *lcd;
	lcd = devm_kzalloc(&dsim_dev->dev, sizeof(struct ma0060), GFP_KERNEL);
	if (!lcd)
	{
		dev_err(&dsim_dev->dev, "failed to allocate visionox_ma0060 structure.\n");
		return -ENOMEM;
	}

	lcd->dsim_dev = dsim_dev;
	lcd->dev = &dsim_dev->dev;

	lcd->ld = lcd_device_register("visionox_ma0060", lcd->dev, lcd,
	                              &panel_lcd_ops);
	if (IS_ERR(lcd->ld))
	{
		dev_err(lcd->dev, "failed to register lcd ops.\n");
		return PTR_ERR(lcd->ld);
	}

	dev_set_drvdata(&dsim_dev->dev, lcd);


	dev_dbg(lcd->dev, "probed visionox_ma0060 panel driver.\n");


	panel->dsim_dev = dsim_dev;


	return 0;

}

static int panel_suspend(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct board_gpio *vdd_en = &panel->vdd_en;

	panel_dev_display_off(panel);
	panel_dev_sleep_in(panel);
	gpio_direction_output(vdd_en->gpio, !vdd_en->active_level);

	return 0;
}

static int panel_resume(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct board_gpio *vdd_en = &panel->vdd_en;

	gpio_direction_output(vdd_en->gpio, vdd_en->active_level);

	return 0;
}

static struct mipi_dsim_lcd_driver panel_dev_dsim_ddi_driver = {
	.name = "visionox_ma0060-lcd",
	.id = -1,

	.power_on = panel_dev_power_on,
	.set_sequence = panel_dev_set_sequence,
	.probe = panel_dev_probe,
	.suspend = panel_suspend,
	.resume = panel_resume,
};


struct mipi_dsim_lcd_device panel_dev_device={
	.name		= "visionox_ma0060-lcd",
	.id = 0,
};

/**
* @panel_probe
*
* 	1. Register to ingenicfb.
* 	2. Register to lcd.
* 	3. Register to backlight if possible.
*
* @pdev
*
* @Return -
*/
static int panel_probe(struct platform_device *pdev)
{
	int ret = 0;

	panel = kzalloc(sizeof(struct panel_dev), GFP_KERNEL);
	if(panel == NULL) {
		dev_err(&pdev->dev, "Faile to alloc memory!");
		return -ENOMEM;
	}
	panel->dev = &pdev->dev;
	dev_set_drvdata(&pdev->dev, panel);

	ret = of_panel_parse(&pdev->dev);
	if(ret < 0) {
		goto err_of_parse;
	}

	mipi_dsi_register_lcd_device(&panel_dev_device);
	mipi_dsi_register_lcd_driver(&panel_dev_dsim_ddi_driver);

	ret = ingenicfb_register_panel(&lcd_panel);
	if(ret < 0) {
		dev_err(&pdev->dev, "Failed to register lcd panel!\n");
		goto err_of_parse;
	}

	return 0;

err_of_parse:
	kfree(panel);
	return ret;
}

static int panel_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id panel_of_match[] = {
	{ .compatible = "ingenic,ma0060", },
	{},
};

static struct platform_driver panel_driver = {
	.probe		= panel_probe,
	.remove		= panel_remove,
	.driver		= {
		.name	= "ma0060",
		.of_match_table = panel_of_match,
	},
};

module_platform_driver(panel_driver);
