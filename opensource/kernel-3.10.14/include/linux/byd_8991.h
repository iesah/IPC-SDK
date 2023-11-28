/*
 * LCD driver data for BYD_BM8766U
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _BYD_8991_H
#define _BYD_8991_H

/**
 * @gpio_lcd_vsync: vertical sync input
 * @gpio_lcd_hsync: horizontal sync input
 * @gpio_lcd_disp: display on/off
 * @gpio_lcd_de: 1: select DE mode, 0: select SYNC mode
 * @gpio_lcd_cs:  spi chip select
 * @gpio_lcd_clk: spi clk
 * @gpio_lcd_sdi: spi in  [cpu -----> lcd]
 * @gpio_lcd_sdo: spi out [lcd -----> cpu]
 * @gpio_lcd_backlight_sel: backlight select 0: lcd self-ctrl 1: pwm ctrl
 */
struct platform_byd_8991_data {
	unsigned int gpio_lcd_disp;
	unsigned int gpio_lcd_vsync;
	unsigned int gpio_lcd_hsync;
	unsigned int gpio_lcd_de;
	unsigned int gpio_lcd_cs;
	unsigned int gpio_lcd_clk;
	unsigned int gpio_lcd_sdo;
	unsigned int gpio_lcd_sdi;
	unsigned int gpio_lcd_back_sel;
};

#endif /* _BYD_8991_H */

