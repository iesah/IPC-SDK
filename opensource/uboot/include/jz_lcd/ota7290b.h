/*************************************************************************
 * File Name: ota7290b.h
 * Author: Mark Zhu
 * Mail:   zhufanghua@imilab.com
 * Created Time: 2022年01月06日 星期四 16时15分59秒
 ************************************************************************/
#ifndef OTA7290B_H
#define OTA7290B_H

#include <jz_lcd/jz_dsim.h>

extern vidinfo_t panel_info;
struct auo_ota7290b_platform_data {
	int gpio_rst;
};

//extern struct auo_oat7290b_platform_data auo_ota7290b_pdata;
extern void auo_ota7290b_sleep_in(struct dsi_device *dsi); /* enter sleep */
extern void auo_ota7290b_sleep_out(struct dsi_device *dsi); /* exit sleep */
extern void auo_ota7290b_display_on(struct dsi_device *dsi); /* display on */
extern void auo_ota7290b_display_off(struct dsi_device *dsi); /* display off */
extern void auo_ota7290b_set_pixel_off(struct dsi_device *dsi); /* set_pixels_off */
extern void auo_ota7290b_set_pixel_on(struct dsi_device *dsi); /* set_pixels_on */
extern void auo_ota7290b_set_brightness(struct dsi_device *dsi, unsigned int brightness); /* set brightness */

#endif

