/*
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZ4775_FB_H__
#define __JZ4775_FB_H__

#include <linux/fb.h>

/* LCD controller supported display device output mode */
enum jzfb_lcd_type {
	LCD_TYPE_GENERIC_16_BIT = 0,
	LCD_TYPE_GENERIC_18_BIT = 0 | (1 << 7),
	LCD_TYPE_GENERIC_24_BIT = 0 | (1 << 6),
	LCD_TYPE_SPECIAL_TFT_1 = 1,
	LCD_TYPE_SPECIAL_TFT_2 = 2,
	LCD_TYPE_SPECIAL_TFT_3 = 3,
	LCD_TYPE_NON_INTERLACED_TV = 4 | (1 << 26),
	LCD_TYPE_INTERLACED_TV = 6 | (1 << 26) | (1 << 30),
	LCD_TYPE_8BIT_SERIAL = 0xc,
	LCD_TYPE_LCM = 0xd | (1 << 31),
};

/**
 * enum data_format - LVDS output data format
 * @JEIDA: JEIDA model.
 * @VESA: VESA model.
 */
enum data_format {
	JEIDA,
	VESA,
};

/* Output data start-edge tuning in 1x clock output mode. TXCTRL: 15-13 bit */
enum data_start_edge {
        START_EDGE_0 = 0x0, /*0 of T7X*/
        START_EDGE_1, /*1 of T7X*/
        START_EDGE_2, /*2 of T7X*/
        START_EDGE_3, /*3 of T7X*/
        START_EDGE_4, /*4 of T7X*/
        START_EDGE_5, /*5 of T7X*/
        START_EDGE_6, /*6 of T7X*/
        START_EDGE_7, /*7 of T7X*/
};

/* LVDS controller working mode */
enum operate_mode {
	LVDS_7X_CLKOUT = (1 << 30) | (1 << 29) | (1 << 12) | (1 << 0),
	LVDS_1X_CLKOUT = (1 << 30) | (1 << 29) | (1 << 0),
        CMOS_OUTPUT = (1 << 30) | (1 << 29) | (1 << 11) | (1 << 1) | (1 << 0),
	OUTPUT_HI_Z = (1 << 30) | (1 << 29) | (0 << 0),
	OUTPUT_ZERO = (1 << 30) | (1 << 29) | (1 << 1) | (0 << 0),
};

/* LVDS_TX Input Clock Edge-Delay Control. TXCTRL: 10-8 bit */
enum input_edge_delay {
        DELAY_0_1NS = 0x0, /*0.1ns*/
        DELAY_0_2NS, /*0.2ns*/
        DELAY_0_5NS, /*0.5ns*/
        DELAY_1NS, /*1ns*/
        DELAY_1_5NS, /*1.5ns*/
        DELAY_2NS, /*2.0ns*/
        DELAY_2_5NS, /*2.5ns*/
        DELAY_3NS, /*3.0ns*/
};

/* LVDS_TX Output Amplitude Control. TXCTRL: 7 6; 5-3 2 bit */
enum output_amplitude {
       VOD_FIX_200MV  = 0xfe,  /* fix output 200mv, 0xfe is used as a flag*/
       VOD_FIX_350MV  = 0xff,  /* fix output 350mv, 0xff is used as a flag*/

       VOD_150MV  = 0x0,/* 150mv is swing */
       VOD_200MV = 0x2, /* 200mv is swing */
       VOD_250MV = 0x4, /* 250mv is swing */
       VOD_300MV = 0x6, /* 300mv is swing */
       VOD_350MV = 0x8, /* 350mv is swing */
       VOD_400MV = 0xA, /* 400mv is swing */
       VOD_500MV = 0xC, /* 500mv is swing */
       VOD_600MV = 0xE, /* 600mv is swing */
       VOD_650MV = 0x1, /* 650mv is swing */
       VOD_700MV = 0x3, /* 700mv is swing */
       VOD_750MV = 0x5, /* 750mv is swing */
       VOD_800MV = 0x7, /* 800mv is swing */
       VOD_850MV = 0x9, /* 850mv is swing */
       VOD_900MV = 0xB, /* 900mv is swing */
       VOD_1000MV = 0xD, /* 1000mv is swing */
       VOD_1100MV = 0xF, /* 1100mv is swing */
};

/* PLL post divider control bits A */
enum pll_post_divider {
	POST_DIV_1,
	POST_DIV_2,
	POST_DIV_3 = 0x03,
	POST_DIV_4 = 0x02,
};

/*Note: This may not be the correct corresponding*/
enum pll_charge_pump {
        CHARGE_PUMP_4UA = 0x7, /*4uA*/
        CHARGE_PUMP_2UA = 0x3, /*2uA, N_fbk:32-40 */
        CHARGE_PUMP_2_5UA = 0x2, /*2.5uA N_fbk:42-60 */
        CHARGE_PUMP_3_3UA = 0x1, /*3.3uA N_fbk:62-80 */
        CHARGE_PUMP_5UA = 0x6, /*5uA   N_fbk:82-110 */
	CHARGE_PUMP_6_7_UA = 0x5, /*6.7uA N_fbk:112-158 */
        CHARGE_PUMP_10UA = 0x4, /*10uA  N_fbk:160-258 */
};

/*PLL KVCO (F_vco)*/
enum pll_vco_gain {
        VCO_GAIN_900M_1G = 0x0, /* 150M-400M VCO Frequency Range */
        VCO_GAIN_650M_900M = 0x1, /* 400M-650M VCO Frequency Range */
        VCO_GAIN_400M_650M = 0x2, /* 650M-900M VCO Frequency Range */
        VCO_GAIN_150M_400M = 0x3, /* 900M-1G VCO Frequency Range */
};

/* vco output current */
enum pll_vco_biasing_current {
	VCO_BIASING_1_25UA = 0x0, /* 1.25uA*/
        VCO_BIASING_2_5UA = 0x1, /* 2.5uA*/
        VCO_BIASING_3_75UA = 0x2, /* 3.75uA*/
        VCO_BIASING_5UA = 0x3, /* 5uA*/
};

/* Internal LDO output voltage configure */
enum ldo_output_vol {
	LDO_OUTPUT_1V,
	LDO_OUTPUT_1_1V,
	LDO_OUTPUT_1_2V,
	LDO_OUTPUT_1_3V,
};

/* struct lvds_txctrl - used to configure LVDS TXCTRL register */
struct lvds_txctrl {
	enum data_format data_format;
	unsigned clk_edge_falling_7x:1;
	unsigned clk_edge_falling_1x:1;
	enum data_start_edge data_start_edge;
	enum operate_mode operate_mode;
	enum input_edge_delay edge_delay;
	enum output_amplitude output_amplitude;
};

/*
 * struct lvds_txpll0 - used to configure LVDS TXPLL0 register
 * Note:
 F_in == F_pixclk;
 7 * F_in == F_out;
 150M < F_vco < 1G

 F_out = F_vco / N_out;
 F_pfd = F_pixclk / N_in;
 F_vco = (F_in / N_in) * N_fbk;
*/
struct lvds_txpll0 {
	unsigned ssc_enable:1;
        unsigned ssc_mode_center_spread:1;
	enum pll_post_divider post_divider; /* N_out : 1-4 */
        unsigned int feedback_divider; /* N_fbk : 8-260 */
	unsigned input_divider_bypass:1;
        unsigned int input_divider; /* N_in : 2-34 */
};

/*
 * struct lvds_txpll1 - used to configure LVDS TXPLL1 register
 * ssc_counter:
 * 0-8191, if ssc enable. It contain gain(0-15) and
 * count(16-8191). The sscn is smaller than
 * counter when use count.
 */
struct lvds_txpll1 {
	enum pll_charge_pump charge_pump;
	enum pll_vco_gain vco_gain;
	enum pll_vco_biasing_current vco_biasing_current;
        unsigned int sscn;       /* 3-130, if ssc enable */
        unsigned int ssc_counter;
};

/* struct lvds_txectrl - used to configure LVDS TXECTRL register */
struct lvds_txectrl {
	unsigned int emphasis_level;
	unsigned emphasis_enable:1;
	enum ldo_output_vol ldo_output_voltage;
	unsigned phase_interpolator_bypass:1;
	unsigned int fine_tuning_7x_clk;
	unsigned int coarse_tuning_7x_clk;
};

/* smart lcd interface_type */
enum smart_lcd_type {
	SMART_LCD_TYPE_PARALLEL,
	SMART_LCD_TYPE_SERIAL,
};

/* smart lcd command width */
enum smart_lcd_cwidth {
	SMART_LCD_CWIDTH_16_BIT_ONCE = (0 << 8),
	SMART_LCD_CWIDTH_9_BIT_ONCE = SMART_LCD_CWIDTH_16_BIT_ONCE,
	SMART_LCD_CWIDTH_8_BIT_ONCE = (0x1 << 8),
	SMART_LCD_CWIDTH_18_BIT_ONCE = (0x2 << 8),
	SMART_LCD_CWIDTH_24_BIT_ONCE = (0x3 << 8),
};

/* smart lcd data width */
enum smart_lcd_dwidth {
	SMART_LCD_DWIDTH_18_BIT_ONCE_PARALLEL_SERIAL = (0 << 10),
	SMART_LCD_DWIDTH_16_BIT_ONCE_PARALLEL_SERIAL = (0x1 << 10),
	SMART_LCD_DWIDTH_8_BIT_THIRD_TIME_PARALLEL = (0x2 << 10),
	SMART_LCD_DWIDTH_8_BIT_TWICE_TIME_PARALLEL = (0x3 << 10),
	SMART_LCD_DWIDTH_8_BIT_ONCE_PARALLEL_SERIAL = (0x4 << 10),
	SMART_LCD_DWIDTH_24_BIT_ONCE_PARALLEL = (0x5 << 10),
	SMART_LCD_DWIDTH_9_BIT_TWICE_TIME_PARALLEL = (0x7 << 10),
};
/**
 * @reg: the register address
 * @value: the value to be written
 * @type: operation type, 0: write register, 1: write command, 2: write data
 * @udelay: delay time in us
 */
struct smart_lcd_data_table {
	uint32_t reg;
	uint32_t value;
	uint32_t type;
	uint32_t udelay;
};

/**
 * struct jzfb_platform_data - the platform data of frame buffer
 *
 * @num_modes: size of modes
 * @modes: list of valid video modes
 * @lcd_type: lcd type
 * @bpp: bits per pixel for the lcd
 * @width: width of the lcd display in mm
 * @height: height of the lcd display in mm
 * @pinmd: 16bpp lcd data pin mapping. 0: LCD_D[15:0],1: LCD_D[17:10] LCD_D[8:1]
 * @pixclk_falling_edge: pixel clock at falling edge
 * @date_enable_active_low: data enable active low
 * @alloc_vidmem: allocate video memory for LCDC. 0: not allocate, 1: allocate
 * @lvds: using LVDS controller. 0: not use, 1: use
 * @txctrl: the configure of LVDS Transmitter Control Register
 * @txpll0: the configure of LVDS Transmitter's PLL Control Register 0
 * @txpll1: the configure of LVDS Transmitter's PLL Control Register 1
 * @txectrl the configure of LVDS Transmitter's Enhance Control
 * @smart_type: smart lcd transfer type, 0: parrallel, 1: serial
 * @cmd_width: smart lcd command width
 * @data_width:smart lcd data Width
 * @clkply_active_rising: smart lcd clock polarity: 0: Active edge is Falling,
 * 1: Active edge is Rasing
 * @rsply_cmd_high: smart lcd RS polarity. 0: Command_RS=0, Data_RS=1;
 * 1: Command_RS=1, Data_RS=0
 * @csply_active_high: smart lcd CS Polarity. 0: Active level is low,
 * 1: Active level is high
 * @write_gram_cmd: write graphic ram command
 * @bus_width: bus width in bit
 * @length_data_table: array size of data_table
 * @data_table: init data table
 * @dither_enable: enable dither function: 1, disable dither function: 0
 * when LCD'bpp is lower than 24, suggest to enable it
 * @dither_red: 0:8bit dither, 1:6bit dither, 2: 5bit dither, 3: 4bit dither
 * @dither_green: 0:8bit dither, 1:6bit dither, 2: 5bit dither, 3: 4bit dither
 * @dither_blue: 0:8bit dither, 1:6bit dither, 2: 5bit dither, 3: 4bit dither
 * @spl: special_tft SPL signal register setting
 * @cls: special_tft CLS signal register setting
 * @ps: special_tft PS signal register setting
 * @rev: special_tft REV signal register setting
 */

struct jzfb_platform_data {
	size_t num_modes;
	struct fb_videomode *modes;

	enum jzfb_lcd_type lcd_type;
	unsigned int bpp;
	unsigned int width;
	unsigned int height;
	unsigned pinmd:1;

	unsigned pixclk_falling_edge:1;
	unsigned date_enable_active_low:1;

	unsigned alloc_vidmem:1;

	unsigned lvds:1;
	struct lvds_txctrl txctrl;
	struct lvds_txpll0 txpll0;
	struct lvds_txpll1 txpll1;
	struct lvds_txectrl txectrl;

	struct {
		enum smart_lcd_type smart_type;
		enum smart_lcd_cwidth cmd_width;
		enum smart_lcd_dwidth data_width;  /* data_width: the data width when mcu init. */
		enum smart_lcd_dwidth data_width2; /* data_width2: the data width when dma frame buffer. */

		unsigned clkply_active_rising:1;
		unsigned rsply_cmd_high:1;
		unsigned csply_active_high:1;

		int continuous_dma; /* 1: auto continuous dma. 0: trigger DMA_RESTART per-frame dma. */

		unsigned long write_gram_cmd;		/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
		unsigned bus_width;
		size_t length_data_table;
		struct smart_lcd_data_table *data_table;
		int (*init)(void); /* smart lcd special initial function */
	} smart_config;

	unsigned dither_enable:1;
	struct {
		unsigned dither_red;
		unsigned dither_green;
		unsigned dither_blue;
	} dither;

	struct {
		uint32_t spl;
		uint32_t cls;
		uint32_t ps;
		uint32_t rev;
	} special_tft_config;
};

//#define FB_MODE_IS_UNKNOWN	0
//#define FB_MODE_IS_DETAILED	1
//#define FB_MODE_IS_STANDARD	2
//#define FB_MODE_IS_VESA		4
//#define FB_MODE_IS_CALCULATED	8
//#define FB_MODE_IS_FIRST	16
//#define FB_MODE_IS_FROM_VAR     32
#define FB_MODE_IS_VGA    (1 << 30)
#define FB_MODE_IS_HDMI   (1 << 29)

#endif
