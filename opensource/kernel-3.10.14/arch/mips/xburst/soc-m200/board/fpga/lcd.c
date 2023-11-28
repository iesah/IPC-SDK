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
/*
#include <linux/digital_pulse_backlight.h>
#include <linux/at070tn93.h>
*/

#include <mach/jzfb.h>
#include "board.h"

int gpio_set_for_slcd(void)
{
	int ret = 0;

	printk("now use gpio func 2!!!!\n");
	ret = gpio_request(GPIO_PC(10), "PC 10");
	if (ret) {
		printk(KERN_ERR "failed to request RD");
		return ret;
	}

	ret = gpio_request(GPIO_PC(9), "PC 9");
	if (ret) {
		printk(KERN_ERR "failed to request PC9");
		return ret;
	}

	ret = gpio_request(GPIO_PC(8), "PC 8");
	if (ret) {
		printk(KERN_ERR "failed to request PC8");
		return ret;
	}

	ret = gpio_request(GPIO_PC(7), "PC 7");
	if (ret) {
		printk(KERN_ERR "failed to request PC7");
		return ret;
	}

	ret = gpio_request(GPIO_PC(6), "PC 6");
	if (ret) {
		printk(KERN_ERR "failed to request PC6");
		return ret;
	}

	ret = gpio_request(GPIO_PC(5), "PC 5");
	if (ret) {
		printk(KERN_ERR "failed to request PC5");
		return ret;
	}

	ret = gpio_request(GPIO_PC(4), "PC 4");
	if (ret) {
		printk(KERN_ERR "failed to request PC4");
		return ret;
	}

	ret = gpio_request(GPIO_PC(3), "PC 3");
	if (ret) {
		printk(KERN_ERR "failed to request PC3");
		return ret;
	}

	ret = gpio_request(GPIO_PC(2), "PC 2");
	if (ret) {
		printk(KERN_ERR "failed to request PC2");
		return ret;
	}

	ret = gpio_request(GPIO_PC(27), "TE");
	if (ret) {
		printk(KERN_ERR "failed to request TE");
		return ret;
	}

	ret = gpio_request(GPIO_PC(26), "RS");
	if (ret) {
		printk(KERN_ERR "failed to request RS");
		return ret;
	}

	ret = gpio_request(GPIO_PC(25), "WR");
	if (ret) {
		printk(KERN_ERR "failed to request WR");
		return ret;
	}

	ret = gpio_request(GPIO_PC(21), "Cs");
	if (ret) {
		printk(KERN_ERR "failed to request cs");
		return ret;
	}

	ret = gpio_request(GPIO_PC(20), "RST");
	if (ret) {
		printk(KERN_ERR "failed to request RST");
		return ret;
	}

	ret = gpio_request(GPIO_PC(19), "SDA for SLCDC");
	if (ret) {
		printk(KERN_ERR "failed to request BL");
		return ret;
	}

	ret = gpio_request(GPIO_PC(17), "LCD_DD12 for BL");
	if (ret) {
		printk(KERN_ERR "failed to request BL");
		return ret;
	}

	ret = gpio_request(GPIO_PC(0), "LCD_DD12 for BL");
	if (ret) {
		printk(KERN_ERR "failed to request BL");
		return ret;
	}

	ret = gpio_request(GPIO_PE(0), "PWM0");
	if (ret) {
		printk(KERN_ERR "failed to request PWM0");
		return ret;
	}
	gpio_direction_output(GPIO_PC(21), 1);	/*cs */
	gpio_direction_output(GPIO_PC(10), 1);	/*RD,pixclk */

	gpio_direction_output(GPIO_PC(20), 0);	/*rst */
	mdelay(10);
	gpio_direction_output(GPIO_PC(20), 1);
	mdelay(5);

	gpio_direction_output(GPIO_PE(0), 1);	/*PWM0 */
	gpio_direction_output(GPIO_PC(17), 1);	/*LCD_DD11 */

	gpio_direction_output(GPIO_PC(21), 0);
	gpio_direction_input(GPIO_PC(0));

	return 0;
}

#ifdef CONFIG_LCD_BYD_BM8766U
#include <linux/byd_bm8766u.h>
static struct platform_byd_bm8766u_data byd_bm8766u_pdata = {
	.gpio_lcd_disp = GPIO_PB(30),
	.gpio_lcd_de = 0,	/*GPIO_PC(9) *//* chose sync mode */
	.gpio_lcd_vsync = 0,	/*GPIO_PC(19) */
	.gpio_lcd_hsync = 0,	/*GPIO_PC(18) */
};

/* LCD device */
struct platform_device byd_bm8766u_device = {
	.name = "byd_bm8766u-lcd",
	.dev = {
		.platform_data = &byd_bm8766u_pdata,
		},
};
#endif

#ifdef CONFIG_LCD_KD50G2_40NM_A2
#include <linux/kd50g2_40nm_a2.h>
static struct platform_kd50g2_40nm_a2_data kd50g2_40nm_a2_pdata = {
	.gpio_lcd_disp = GPIO_PB(30),
	.gpio_lcd_de = 0,	/*GPIO_PC(9) *//* chose sync mode */
	.gpio_lcd_vsync = 0,	/*GPIO_PC(19) */
	.gpio_lcd_hsync = 0,	/*GPIO_PC(18) */
};

/* LCD device */
struct platform_device kd50g2_40nm_a2_device = {
	.name = "kd50g2_40nm_a2-lcd",
	.dev = {
		.platform_data = &kd50g2_40nm_a2_pdata,
		},
};
#endif

#ifdef CONFIG_LCD_CV90_M5377_P30
#include <linux/cv90_m5377_p30.h>
static struct platform_cv90_m5377_p30_data cv90_m5377_p30_pdata = {
	.gpio_lcd_rst = GPIO_PC(20),
	.gpio_lcd_cs = GPIO_PC(21),	/*GPIO_PC(9) *//* chose sync mode */
	.gpio_lcd_nbusy = 0,	/*GPIO_PC(19) */
	.gpio_lcd_bl = GPIO_PE(0),
	.gpio_lcd_dd11 = GPIO_PC(17),	/*GPIO_PC(18) */
	.gpio_lcd_rd = GPIO_PC(10),
};

struct platform_device cv90_m5377_p30_device = {
	.name = "cv90_m5377_p30-lcd",
	.dev = {
		.platform_data = &cv90_m5377_p30_pdata,
		},
};

extern int gpio_set_for_slcd(void);
#endif

#ifdef CONFIG_LCD_KFM701A21_1A
#include <linux/kfm701a21_1a.h>
static struct platform_kfm701a21_1a_data kfm701a21_1a_pdata = {
#ifdef CONFIG_LCD_GPIO_FUNC0_16BIT
	.gpio_lcd_cs = GPIO_PC(21),
#elif defined CONFIG_LCD_GPIO_FUNC2_SLCD
	.gpio_lcd_cs = GPIO_PC(23),	/*gpio func2 */
	.gpio_lcd_reset = GPIO_PB(22),
#endif
};

/* LCD Panel Device */
struct platform_device kfm701a21_1a_device = {
	.name = "kfm701a21_1a-lcd",
	.dev = {
		.platform_data = &kfm701a21_1a_pdata,
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
	{0x0003, 0x12b8, 0, 0},	/* BGR */
	/*{0x0003, 0x02b8, 0, 0}, *//* RGB */
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

struct fb_videomode jzfb_videomode = {
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

#ifdef CONFIG_LCD_OTM8012A_TFT1P6449
	.name = "480x854",
	.refresh = 60,
	.xres = 480,
	.yres = 854,
	.pixclock = KHZ2PICOS(24000),
	.left_margin = 6,
	.right_margin = 6,
	.upper_margin = 12,
	.lower_margin = 12,
	.hsync_len = 2,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
#endif

#ifdef CONFIG_LCD_LH155
	.name = "240x240",
	.refresh = 60,
	.xres = 240,
	.yres = 240,
	.pixclock = KHZ2PICOS(30000),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
#endif

#ifdef CONFIG_LCD_CV90_M5377_P30
	.name = "192x192",
	.refresh = 45,
	.xres = 288,
	.yres = 192,
	.pixclock = KHZ2PICOS(5000),
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

#ifdef CONFIG_LCD_KD50G2_40NM_A2
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
};

#ifdef CONFIG_JZ_MIPI_DSI
#ifdef CONFIG_LCD_OTM8012A_TFT1P6449
#include <linux/otm8012a_init_mipi.h>
#endif
#ifdef CONFIG_LCD_LH155
#include <linux/lh155_init_mipi.h>
#endif
struct jzdsi_platform_data jzdsi_pdata = {
#ifdef CONFIG_LCD_OTM8012A_TFT1P6449
	.video_config.no_of_lanes = 2,
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
	.video_config.byte_clock = DEFAULT_DATALANE_BPS / 8,	/* KHz  */
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.video_config.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.video_config.pixel_clock = 24000,	/* dpi_clock */
	.video_config.is_18_loosely = 0,
	.video_config.data_en_polarity = 1,

	.video_config.h_polarity = 0,
	.video_config.h_active_pixels = 480,
	.video_config.h_sync_pixels = 2,	/* min 4 pixels */
	.video_config.h_back_porch_pixels = 6,
	.video_config.h_total_pixels = 480 + 2 + 6 + 6,
	.video_config.v_active_lines = 854,
	.video_config.v_polarity = 0,	/*1:active high, 0: active low */
	.video_config.v_sync_lines = 2,
	.video_config.v_back_porch_lines = 12,
	.video_config.v_total_lines = 854 + 2 + 12 + 12,

	.dsi_config.max_lanes = 4,
	.dsi_config.max_hs_to_lp_cycles = 100,
	.dsi_config.max_lp_to_hs_cycles = 40,
	.dsi_config.max_bta_cycles = 4095,
	.dsi_config.color_mode_polarity = 1,
	.dsi_config.shut_down_polarity = 1,

	.cmd_list = otm8012a_cmd_list,
	.cmd_packet_len = 200,

#endif

#ifdef CONFIG_LCD_LH155
	.video_config.no_of_lanes = 1,
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
	.video_config.byte_clock = DEFAULT_DATALANE_BPS / 8,	/* KHz  */
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.video_config.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.video_config.pixel_clock = 100000,	/* dpi_clock */
	.video_config.is_18_loosely = 0,
	.video_config.data_en_polarity = 1,

	.video_config.h_polarity = 0,
	.video_config.h_active_pixels = 240,
	.video_config.h_sync_pixels = 0,	/* min 4 pixels */
	.video_config.h_back_porch_pixels = 0,
	.video_config.h_total_pixels = 240,
	.video_config.v_active_lines = 240,
	.video_config.v_polarity = 0,	/*1:active high, 0: active low */
	.video_config.v_sync_lines = 0,
	.video_config.v_back_porch_lines = 0,
	.video_config.v_total_lines = 240,

	.dsi_config.max_lanes = 4,
	.dsi_config.max_hs_to_lp_cycles = 100,
	.dsi_config.max_lp_to_hs_cycles = 40,
	.dsi_config.max_bta_cycles = 4095,
	.dsi_config.color_mode_polarity = 1,
	.dsi_config.shut_down_polarity = 1,

	.cmd_list = lh155_cmd_list,
	.cmd_packet_len = 30,
#endif

};
#endif

#ifdef CONFIG_LCD_CV90_M5377_P30
static struct smart_lcd_data_table smart_lcd_data_table[] = {
#if 0
	{0x01, 0x01, 1, 10},
	{0xB9, 0xB9, 1, 0},
	{0xB9, 0xff, 2, 0},
	{0xB9, 0x52, 2, 0},
	{0xB9, 0x52, 2, 0},
	{0x11, 0x11, 1, 250},
	{0xbd, 0xbd, 1, 0},
	{0xbd, 0x00, 2, 0},
	{0xf4, 0xf4, 1, 0},
	{0xf4, 0x00, 2, 0},
	{0xf4, 0x01, 2, 0},
	{0xf4, 0x02, 2, 0},
	/*{0xc4, 0xc4, 1, 0},
	   {0xc4, 0x00, 2, 0},
	   {0xb2, 0xb2, 1, 0},
	   {0xb2, 0x05, 2, 0},
	   {0xb2, 0x0c, 2, 0},
	   {0x36, 0x36, 1, 0},
	   {0x36, 0x20, 2, 10}, */
	{0x3a, 0x3a, 1, 0},
	{0x3a, 0x06, 2, 0},
	{0x36, 0x36, 1, 0},
	{0x36, 0x00, 2, 0},
	/*{0xc8, 0xc8, 1, 0},
	   {0xc8, 0x01, 2, 0},
	   {0xc8, 0x20, 2, 0}, */
	{0x29, 0x29, 1, 0},
	/*{0xcd, 0xcd, 1, 0}, */
	{0x2a, 0x2a, 1, 0},
	{0x2a, 0, 2, 0},
	{0x2a, 0, 2, 0},
#define WIDTH  (240-1)
#define HIEGHT (240-1)
	{0x2a, (WIDTH >> 8) & 0xff, 2, 0},
	{0x2a, WIDTH & 0xff, 2, 0},
	{0x2b, 0x2b, 1, 0},
	{0x2b, 0, 2, 0},
	{0x2b, 0, 2, 0},
	{0x2b, (HIEGHT >> 8) & 0xff, 2, 0},
	{0x2b, HIEGHT & 0xff, 2, 0},
	{0xcd, 0xcd, 1, 0},
	{0x2c, 0x2c, 1, 0},
#else
	/*{0x01, 0x01, 1, 5}, */
	{0xB9, 0xB9, 1, 0},
	{0xB9, 0xff, 2, 0},
	{0xB9, 0x52, 2, 0},
	{0xB9, 0x52, 2, 0},
	{0x11, 0x11, 1, 200000},
	{0xf0, 0xf0, 1, 0},
	{0xf0, 0x01, 2, 0},
	{0xf4, 0x00, 2, 0},
	{0xf4, 0xf4, 1, 0},
	{0xf4, 0x01, 2, 0},
	{0xf4, 0x01, 2, 0},
	{0xf4, 0x02, 2, 0},
	/*{0xc4, 0xc4, 1, 0},
	   {0xc4, 0x00, 2, 0},
	   {0xb2, 0xb2, 1, 0},
	   {0xb2, 0x05, 2, 0},
	   {0xb2, 0x0c, 2, 0}, */
	{0x3a, 0x3a, 1, 0},
	{0x3a, 0x06, 2, 0},
#ifdef CONFIG_SLCDC_USE_TE
	{0Xb7, 0x35, 1, 0},	/*open TE */
	{0Xb7, 0x00, 2, 0},	/*TEM:0(TE mode 1) or 1(TE mode 2) */
#endif
	/*{0x36, 0x36, 1, 0},
	   {0x36, 0x09, 2, 0},
	   {0xc8, 0xc8, 1, 0},
	   {0xc8, 0x01, 2, 0},
	   {0xc8, 0x20, 2, 0}, */
	{0xbd, 0xbd, 1, 0},
	{0xbd, 0x00, 2, 0},
	{0x29, 0x29, 1, 0},
	{0xcd, 0xcd, 1, 0},
	{0x2a, 0x2a, 1, 0},
	{0x2a, 0, 2, 0},
	{0x2a, 0, 2, 0},
	{0x2a, (287 >> 8) & 0xff, 2, 0},
	{0x2a, 287 & 0xff, 2, 0},
	{0x2b, 0x2b, 1, 0},
	{0x2b, 0, 2, 0},
	{0x2b, 0, 2, 0},
	{0x2b, (191 >> 8) & 0xff, 2, 0},
	{0x2b, 191 & 0xff, 2, 0},
	{0xcd, 0xcd, 1, 0},
	{0x2c, 0x2c, 1, 0},

#endif
};
#endif

struct jzfb_platform_data jzfb_pdata = {
#ifdef CONFIG_LCD_BYD_BM8766U
	.num_modes = 1,
	.modes = &jzfb_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 108,
	.height = 65,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,

	.dither_enable = 1,
	.dither.dither_red = 0,	/* 8bit */
	.dither.dither_red = 0,	/* 8bit */
	.dither.dither_red = 0,	/* 8bit */

#endif

#ifdef CONFIG_LCD_OTM8012A_TFT1P6449
	.num_modes = 1,
	.modes = &jzfb_videomode,
	.dsi_pdata = &jzdsi_pdata,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 55,
	.height = 98,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,

#endif

#ifdef CONFIG_LCD_LH155
	.num_modes = 1,
	.modes = &jzfb_videomode,
	.dsi_pdata = &jzdsi_pdata,

	.lcd_type = LCD_TYPE_LCM,
	.bpp = 18,
	.width = 31,
	.height = 31,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,

	.smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
	.smart_config.cmd_width = SMART_LCD_CWIDTH_8_BIT_ONCE,
	.smart_config.data_width = SMART_LCD_DWIDTH_8_BIT_ONCE_PARALLEL_SERIAL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,
	.smart_config.write_gram_cmd = 0x2C2C,
	.smart_config.bus_width = 8,
	.dither_enable = 1,
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
#endif

#ifdef CONFIG_LCD_CV90_M5377_P30
	.num_modes = 1,
	.modes = &jzfb_videomode,

	.lcd_type = LCD_TYPE_LCM,
	.bpp = 24,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
	.dither_enable = 0,

	.smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
	.smart_config.cmd_width = SMART_LCD_CWIDTH_18_BIT_ONCE,
	.smart_config.data_width = SMART_LCD_DWIDTH_24_BIT_ONCE_PARALLEL,
	.smart_config.data_width2 = SMART_LCD_DWIDTH_24_BIT_ONCE_PARALLEL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,
	.smart_config.write_gram_cmd = 0x2ccd2ccd,
	.smart_config.bus_width = 8,
	.smart_config.length_data_table = ARRAY_SIZE(smart_lcd_data_table),
	.smart_config.data_table = smart_lcd_data_table,
	.smart_config.init = 0,
	.smart_config.gpio_for_slcd = gpio_set_for_slcd,
#endif

#ifdef CONFIG_LCD_KFM701A21_1A
	.num_modes = 1,
	.modes = &jzfb_videomode,

	.lcd_type = LCD_TYPE_LCM,
	.bpp = 18,
	.width = 39,
	.height = 65,
	.pinmd = 0,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
	.smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
	.smart_config.cmd_width = SMART_LCD_CWIDTH_18_BIT_ONCE,

	.smart_config.data_width = SMART_LCD_DWIDTH_24_BIT_ONCE_PARALLEL,
	.smart_config.data_width2 = SMART_LCD_DWIDTH_24_BIT_ONCE_PARALLEL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,
	.smart_config.data_new_width = SMART_LCD_NEW_DWIDTH_16_BIT,
	.smart_config.data_new_times = SMART_LCD_NEW_DTIMES_ONCE,
	.smart_config.newcfg_6800_md = 0,
	.smart_config.newcfg_fmt_conv = 1,
	.smart_config.newcfg_datatx_type = 0,
	.smart_config.newcfg_cmdtx_type = 0,
	.smart_config.newcfg_cmd_9bit = 0,
	.smart_config.write_gram_cmd = 0x0202,
	.smart_config.bus_width = 18,
	.smart_config.length_data_table = ARRAY_SIZE(kfm701a21_1a_data_table),
	.smart_config.data_table = kfm701a21_1a_data_table,

	.dither_enable = 1,
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
#endif

#ifdef CONFIG_LCD_KD50G2_40NM_A2
	.num_modes = 1,
	.modes = &jzfb_videomode,

	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 108,
	.height = 65,

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
	/*gpio_direction_output(GPIO_LCD_PWM, 1); */

	return 0;
}

static void backlight_exit(struct device *dev)
{
	gpio_free(GPIO_LCD_PWM);
}

static struct platform_pwm_backlight_data backlight_data = {
	.pwm_id = 1,
	.max_brightness = 255,
	.dft_brightness = 120,
	.pwm_period_ns = 30000,
	.init = backlight_init,
	.exit = backlight_exit,
};

struct platform_device backlight_device = {
	.name = "pwm-backlight",
	.dev = {
		.platform_data = &backlight_data,
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
	.name = "digital-pulse-backlight",
	.dev = {
		.platform_data = &bl_data,
		},
};
#endif
