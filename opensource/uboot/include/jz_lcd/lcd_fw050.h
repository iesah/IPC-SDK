/*
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _LCD_FW050_H__
#define _LCD_FW050_H__
#include <jz_lcd/jz_dsim.h>


struct lcd_fw050_data {
	unsigned gpio_lcd_vdd;
	unsigned gpio_lcd_rst;
	unsigned gpio_lcd_pwm;
	unsigned gpio_lcd_te;
};

extern struct lcd_fw050_data lcd_fw050_pdata;
extern void panel_pin_init(void);
extern void panel_power_on(void);
extern void panel_poer_off(void);
extern void fw050_sleep_in(struct dsi_device *dsi);
extern void fw050_sleep_out(struct dsi_device *dsi);
extern void fw050_display_on(struct dsi_device *dsi);
extern void fw050_display_off(struct dsi_device *dsi);
extern void fw050_panel_init(struct dsi_device *dsi);


#endif
