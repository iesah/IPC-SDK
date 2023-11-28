/*
 * sc2232h.c
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <soc/gpio.h>

#include <tx-isp-common.h>
#include <sensor-common.h>
//#include <tx-isp-debug.h>

#define SC2232H_CHIP_ID_H	(0xcb)
#define SC2232H_CHIP_ID_M	(0x07)
#define SC2232H_CHIP_ID_L	(0x01)
#define SC2232H_REG_END		0xffff
#define SC2232H_REG_DELAY	0xfffe
#define SC2232H_SUPPORT_SCLK (78000000)
#define SC2232H_SUPPORT_SCLK_15FPS (54000000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20200521a"

static int reset_gpio = GPIO_PC(28);
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Reset GPIO NUM");

static int pwdn_gpio = -1;
module_param(pwdn_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwdn_gpio, "Power down GPIO NUM");

static int sensor_gpio_func = DVP_PA_HIGH_10BIT;
module_param(sensor_gpio_func, int, S_IRUGO);
MODULE_PARM_DESC(sensor_gpio_func, "Sensor GPIO function");

static int data_interface = TX_SENSOR_DATA_INTERFACE_DVP;
module_param(data_interface, int, S_IRUGO);
MODULE_PARM_DESC(data_interface, "Sensor Date interface");

static int data_type = TX_SENSOR_DATA_TYPE_LINEAR;
module_param(data_type, int, S_IRUGO);
MODULE_PARM_DESC(data_type, "Sensor Date Type");

static int sensor_max_fps = TX_SENSOR_MAX_FPS_15;
module_param(sensor_max_fps, int, S_IRUGO);
MODULE_PARM_DESC(sensor_max_fps, "Sensor Max Fps set interface");

struct regval_list {
	uint16_t reg_num;
	unsigned char value;
};

/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
	unsigned int value;
	unsigned int gain;
};

struct again_lut sc2232h_again_lut[] = {
	{0x10, 0},
	{0x11, 5731},
	{0x12, 11136},
	{0x13, 16248},
	{0x14, 21097},
	{0x15, 25710},
	{0x16, 30109},
	{0x17, 34312},
	{0x18, 38336},
	{0x19, 42195},
	{0x1a, 45904},
	{0x1b, 49472},
	{0x1c, 52910},
	{0x1d, 56228},
	{0x1e, 59433},
	{0x1f, 62534},
	{0x110,	65536},
	{0x111,	71267},
	{0x112,	76672},
	{0x113,	81784},
	{0x114,	86633},
	{0x115,	91246},
	{0x116,	95645},
	{0x117,	99848},
	{0x118, 103872},
	{0x119,	107731},
	{0x11a,	111440},
	{0x11b,	115008},
	{0x11c,	118446},
	{0x11d,	121764},
	{0x11e,	124969},
	{0x11f,	128070},
	{0x310, 131072},
	{0x311,	136803},
	{0x312,	142208},
	{0x313,	147320},
	{0x314,	152169},
	{0x315,	156782},
	{0x316,	161181},
	{0x317,	165384},
	{0x318,	169408},
	{0x319,	173267},
	{0x31a,	176976},
	{0x31b,	180544},
	{0x31c,	183982},
	{0x31d,	187300},
	{0x31e,	190505},
	{0x31f,	193606},
	{0x710, 196608},
	{0x711,	202339},
	{0x712,	207744},
	{0x713,	212856},
	{0x714,	217705},
	{0x715,	222318},
	{0x716,	226717},
	{0x717,	230920},
	{0x718,	234944},
	{0x719,	238803},
	{0x71a,	242512},
	{0x71b,	246080},
	{0x71c,	249518},
	{0x71d,	252836},
	{0x71e,	256041},
	/* {0x71f, 259142}, */
};

struct tx_isp_sensor_attribute sc2232h_attr;

unsigned int sc2232h_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = sc2232h_again_lut;
	while(lut->gain <= sc2232h_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return lut[0].gain;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == sc2232h_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int sc2232h_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus sc2232h_mipi = {
	.clk = 400,
	.lans = 2,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
	.mipi_sc.hcrop_diff_en = 0,
	.mipi_sc.mipi_vcomp_en = 0,
	.mipi_sc.mipi_hcomp_en = 1,
	.image_twidth = 1920,
	.image_theight = 1080,
	.mipi_sc.mipi_crop_start0x = 0,
	.mipi_sc.mipi_crop_start0y = 0,
	.mipi_sc.mipi_crop_start1x = 0,
	.mipi_sc.mipi_crop_start1y = 0,
	.mipi_sc.mipi_crop_start2x = 0,
	.mipi_sc.mipi_crop_start2y = 0,
	.mipi_sc.mipi_crop_start3x = 0,
	.mipi_sc.mipi_crop_start3y = 0,
	.mipi_sc.line_sync_mode = 0,
	.mipi_sc.work_start_flag = 0,
	.mipi_sc.data_type_en = 0,
	.mipi_sc.data_type_value = RAW10,
	.mipi_sc.del_start = 0,
	.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
	.mipi_sc.sensor_fid_mode = 0,
	.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
};

struct tx_isp_dvp_bus sc2232h_dvp={
	.mode = SENSOR_DVP_HREF_MODE,
	.blanking = {
		.vblanking = 0,
		.hblanking = 0,
	},
};

struct tx_isp_sensor_attribute sc2232h_attr={
	.name = "sc2232h",
	.chip_id = 0xcb0701,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x30,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP,
	.dvp = {
		.mode = SENSOR_DVP_HREF_MODE,
		.blanking = {
			.vblanking = 0,
			.hblanking = 0,
		},
	},
	.max_again = 256041,
	.max_dgain = 0,
	.min_integration_time = 4,
	.min_integration_time_native = 4,
	.max_integration_time_native = 1498,
	.integration_time_limit = 1498,
	.total_width = 2080,
	.total_height = 1500,
	.max_integration_time = 1498,
	.one_line_expr_in_us = 27,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = sc2232h_alloc_again,
	.sensor_ctrl.alloc_dgain = sc2232h_alloc_dgain,
	//	void priv; /* point to struct tx_isp_sensor_board_info */
};

static struct regval_list sc2232h_init_regs_1920_1080_25fps_mipi[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x3034, 0x81},
	{0x3039, 0xd4},
	{0x3624, 0x08},
	{0x337f, 0x03},
	{0x3368, 0x04},
	{0x3369, 0x00},
	{0x336a, 0x00},
	{0x336b, 0x00},
	{0x3367, 0x08},
	{0x330e, 0x30},
	{0x3366, 0x7c},
	{0x3302, 0x1f},
	{0x3907, 0x00},
	{0x3908, 0x11},
	{0x335e, 0x01},
	{0x335f, 0x03},
	{0x337c, 0x04},
	{0x337d, 0x06},
	{0x33a0, 0x05},
	{0x3633, 0x4f},
	{0x3622, 0x06},
	{0x3631, 0x84},
	{0x366e, 0x08},
	{0x3326, 0x00},
	{0x3303, 0x20},
	{0x3638, 0x1f},
	{0x3636, 0x25},
	{0x3625, 0x02},
	{0x331b, 0x83},
	{0x3333, 0x30},
	{0x3635, 0xa0},
	{0x363c, 0x05},
	{0x3038, 0xff},
	{0x3639, 0x09},
	{0x3621, 0x28},
	{0x3211, 0x0c},
	{0x3320, 0x01},
	{0x331e, 0x19},
	{0x3620, 0x28},
	{0x3309, 0x60},
	{0x331f, 0x59},
	{0x3308, 0x10},
	{0x3f00, 0x07},
	{0x33aa, 0x10},
	{0x3677, 0x86},
	{0x3678, 0x88},
	{0x3679, 0x88},
	{0x367e, 0x08},
	{0x367f, 0x28},
	{0x3670, 0x0c},
	{0x3690, 0x33},
	{0x3691, 0x11},
	{0x3692, 0x43},
	{0x369c, 0x08},
	{0x369d, 0x28},
	{0x360f, 0x01},
	{0x3671, 0xc6},
	{0x3672, 0x06},
	{0x3673, 0x16},
	{0x367a, 0x28},
	{0x367b, 0x3f},
	{0x3221, 0x80},
	{0x3222, 0x29},
	{0x3901, 0x02},
	{0x3905, 0x98},
	{0x3e1e, 0x34},
	{0x3900, 0x19},
	{0x391d, 0x04},
	{0x391e, 0x00},
	{0x3641, 0x01},
	{0x3213, 0x04},
	{0x3614, 0x80},
	{0x363a, 0x9f},
	{0x3630, 0x9c},
	{0x3306, 0x48},
	{0x330b, 0xcd},
	{0x3018, 0x33},
	{0x3031, 0x0a},
	{0x3037, 0x20},
	{0x3001, 0xfe},
	{0x4603, 0x00},
	{0x4827, 0x48},
	{0x301c, 0x78},
	{0x4809, 0x01},
	{0x3314, 0x04},
	{0x3933, 0x0a},
	{0x3934, 0x18},
	{0x3940, 0x60},
	{0x3942, 0x02},
	{0x3943, 0x1f},
	{0x3960, 0xba},
	{0x3961, 0xae},
	{0x3966, 0xba},
	{0x3980, 0xa0},
	{0x3981, 0x40},
	{0x3982, 0x18},
	{0x3903, 0x08},
	{0x3984, 0x08},
	{0x3985, 0x20},
	{0x3986, 0x50},
	{0x3987, 0xb0},
	{0x3988, 0x08},
	{0x3989, 0x10},
	{0x398a, 0x20},
	{0x398b, 0x30},
	{0x398c, 0x60},
	{0x398d, 0x20},
	{0x398e, 0x10},
	{0x398f, 0x08},
	{0x3990, 0x60},
	{0x3991, 0x24},
	{0x3992, 0x15},
	{0x3993, 0x08},
	{0x3994, 0x0a},
	{0x3995, 0x20},
	{0x3996, 0x38},
	{0x3997, 0xa0},
	{0x3998, 0x08},
	{0x3999, 0x10},
	{0x399a, 0x18},
	{0x399b, 0x30},
	{0x399c, 0x30},
	{0x399d, 0x18},
	{0x399e, 0x10},
	{0x399f, 0x08},
	{0x3637, 0x55},
	{0x366f, 0x2c},
	{0x5000, 0x06},
	{0x5780, 0x7f},
	{0x5781, 0x04},
	{0x5782, 0x03},
	{0x5783, 0x02},
	{0x5784, 0x01},
	{0x5785, 0x18},
	{0x5786, 0x10},
	{0x5787, 0x08},
	{0x5788, 0x02},
	{0x57a0, 0x00},
	{0x57a1, 0x71},
	{0x57a2, 0x01},
	{0x57a3, 0xf1},
	{0x395e, 0xc0},
	{0x3962, 0x88},
	{0x303a, 0xb3},
	{0x303c, 0x0e},
	{0x3035, 0x9b},
	{0x320c, 0x08},
	{0x320d, 0x20},
	{0x320e, 0x05},
	{0x320f, 0xdc},/*25fps*/
	{0x3f04, 0x03},
	{0x3f05, 0xec},
	{0x3235, 0x09},
	{0x3236, 0xc2},
	{0x4837, 0x33},
	{0x3802, 0x00},
	{0x3963, 0x80},
	{0x303b, 0x16},
	{0x301f, 0x05},
	{0x363b, 0x26},
	{0x3902, 0xc5},
	{0x3e00, 0x00},
	{0x3e01, 0x9c},
	{0x3e02, 0x00},
	{0x3e03, 0x0b},
	{0x3e06, 0x00},
	{0x3e07, 0x80},
	{0x3e08, 0x03},
	{0x3e09, 0x10},
	{0x3301, 0x12},
	{0x3632, 0x08},
	{0x3034, 0x01},
	{0x3039, 0x24},

	{SC2232H_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc2232h_init_regs_1920_1080_15fps_dvp[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x3034, 0x80},
	{0x3039, 0x80},
	{0x3018, 0x1f},
	{0x3019, 0xff},
	{0x301c, 0xb4},
	{0x3624, 0x08},
	{0x337f, 0x03},
	{0x3368, 0x04},
	{0x3369, 0x00},
	{0x336a, 0x00},
	{0x336b, 0x00},
	{0x3367, 0x08},
	{0x330e, 0x30},
	{0x3366, 0x7c},
	{0x3302, 0x1f},
	{0x303f, 0x81},
	{0x3907, 0x00},
	{0x3908, 0x11},
	{0x335e, 0x01},
	{0x335f, 0x03},
	{0x337c, 0x04},
	{0x337d, 0x06},
	{0x33a0, 0x05},
	{0x3633, 0x4f},
	{0x3622, 0x06},
	{0x3631, 0x84},
	{0x366e, 0x08},
	{0x3326, 0x00},
	{0x3303, 0x20},
	{0x3638, 0x1f},
	{0x3636, 0x25},
	{0x3625, 0x02},
	{0x331b, 0x83},
	{0x3333, 0x30},
	{0x3635, 0xa0},
	{0x363c, 0x05},
	{0x3038, 0xff},
	{0x3639, 0x09},
	{0x3621, 0x28},
	{0x3320, 0x01},
	{0x331e, 0x19},
	{0x3620, 0x28},
	{0x3309, 0x60},
	{0x331f, 0x59},
	{0x3308, 0x10},
	{0x3f00, 0x07},
	{0x33aa, 0x10},
	{0x3677, 0x86},
	{0x3678, 0x88},
	{0x3679, 0x88},
	{0x367e, 0x08},
	{0x367f, 0x28},
	{0x3670, 0x0c},
	{0x3690, 0x33},
	{0x3691, 0x11},
	{0x3692, 0x43},
	{0x369c, 0x08},
	{0x369d, 0x28},
	{0x360f, 0x01},
	{0x3671, 0xc6},
	{0x3672, 0x06},
	{0x3673, 0x16},
	{0x367a, 0x28},
	{0x367b, 0x3f},
	{0x3222, 0x29},
	{0x3901, 0x02},
	{0x3905, 0x98},
	{0x3e1e, 0x34},
	{0x3900, 0x19},
	{0x391d, 0x04},
	{0x391e, 0x00},
	{0x3641, 0x01},
	{0x3213, 0x08},//04
	{0x3614, 0x80},
	{0x363a, 0x9f},
	{0x3630, 0x9c},
	{0x3306, 0x48},
	{0x330b, 0xcd},
	{0x3314, 0x04},
	{0x3933, 0x0a},
	{0x3940, 0x60},
	{0x3942, 0x02},
	{0x3943, 0x1f},
	{0x3960, 0xba},
	{0x3961, 0xae},
	{0x3966, 0xba},
	{0x3980, 0xa0},
	{0x3981, 0x40},
	{0x3982, 0x18},
	{0x3903, 0x08},
	{0x3984, 0x08},
	{0x3988, 0x08},
	{0x3989, 0x10},
	{0x398a, 0x20},
	{0x398b, 0x30},
	{0x398c, 0x60},
	{0x398d, 0x20},
	{0x398e, 0x10},
	{0x398f, 0x08},
	{0x3991, 0x24},
	{0x3992, 0x15},
	{0x3993, 0x08},
	{0x3994, 0x0a},
	{0x3995, 0x20},
	{0x3996, 0x38},
	{0x3998, 0x08},
	{0x3999, 0x10},
	{0x399a, 0x18},
	{0x399b, 0x30},
	{0x399c, 0x30},
	{0x399d, 0x18},
	{0x399e, 0x10},
	{0x399f, 0x08},
	{0x3934, 0x18},
	{0x3985, 0x18},
	{0x3986, 0x28},
	{0x3987, 0x70},
	{0x3990, 0x40},
	{0x3997, 0x70},
	{0x3637, 0x55},
	{0x366f, 0x2c},
	{0x5000, 0x06},
	{0x5780, 0x7f},
	{0x5781, 0x04},
	{0x5782, 0x03},
	{0x5783, 0x02},
	{0x5784, 0x01},
	{0x5785, 0x18},
	{0x5786, 0x10},
	{0x5787, 0x08},
	{0x5788, 0x02},
	{0x57a0, 0x00},
	{0x57a1, 0x71},
	{0x57a2, 0x01},
	{0x57a3, 0xf1},
	{0x395e, 0xc0},
	{0x3962, 0x89},
	{0x3802, 0x00},
	{0x3209, 0x80},//88
	{0x320b, 0x38},//40
	{0x3211, 0x08},//4
	{0x3963, 0x80},
	{0x301f, 0x04},
	{0x363b, 0x26},
	{0x3902, 0xc5},
	{0x3e00, 0x00},
	{0x3e01, 0x9c},
	{0x3e02, 0x00},
	{0x3e03, 0x0b},
	{0x3e06, 0x00},
	{0x3e07, 0x80},
	{0x3e08, 0x03},
	{0x3e09, 0x10},
	{0x3301, 0x12},
	{0x3632, 0x08},
	{0x303a, 0x37},
	{0x303b, 0x16},
	{0x303c, 0x0e},
	{0x303d, 0x23},
	{0x320c, 0x09},
	{0x320d, 0x60},
	{0x3f04, 0x04},
	{0x3f05, 0x8c},
	{0x320e, 0x05},
	{0x320f, 0xdc},
	{0x3235, 0x0b},
	{0x3236, 0xb6},
	{0x3035, 0xbb},
	{0x3034, 0x01},
	{0x3039, 0x20},
	{SC2232H_REG_DELAY, 0x0a},
	{SC2232H_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc2232h_init_regs_1920_1080_25fps_dvp[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x3034, 0x81},
	{0x3039, 0xd4},
	{0x3018, 0x1f},
	{0x3019, 0xff},
	{0x301c, 0xb4},
	{0x3624, 0x08},
	{0x337f, 0x03},
	{0x3368, 0x04},
	{0x3369, 0x00},
	{0x336a, 0x00},
	{0x336b, 0x00},
	{0x3367, 0x08},
	{0x330e, 0x30},
	{0x3366, 0x7c},
	{0x3302, 0x1f},
	{0x303f, 0x81},
	{0x3907, 0x00},
	{0x3908, 0x11},
	{0x335e, 0x01},
	{0x335f, 0x03},
	{0x337c, 0x04},
	{0x337d, 0x06},
	{0x33a0, 0x05},
	{0x3633, 0x4f},
	{0x3622, 0x06},
	{0x3631, 0x84},
	{0x366e, 0x08},
	{0x3326, 0x00},
	{0x3303, 0x20},
	{0x3638, 0x1f},
	{0x3636, 0x25},
	{0x3625, 0x02},
	{0x331b, 0x83},
	{0x3333, 0x30},
	{0x3635, 0xa0},
	{0x363c, 0x05},
	{0x3038, 0xff},
	{0x3639, 0x09},
	{0x3621, 0x28},
	{0x3211, 0x0c},
	{0x3320, 0x01},
	{0x331e, 0x19},
	{0x3620, 0x28},
	{0x3309, 0x60},
	{0x331f, 0x59},
	{0x3308, 0x10},
	{0x3f00, 0x07},
	{0x33aa, 0x10},
	{0x3677, 0x86},
	{0x3678, 0x88},
	{0x3679, 0x88},
	{0x367e, 0x08},
	{0x367f, 0x28},
	{0x3670, 0x0c},
	{0x3690, 0x33},
	{0x3691, 0x11},
	{0x3692, 0x43},
	{0x369c, 0x08},
	{0x369d, 0x28},
	{0x360f, 0x01},
	{0x3671, 0xc6},
	{0x3672, 0x06},
	{0x3673, 0x16},
	{0x367a, 0x28},
	{0x367b, 0x3f},
	{0x3221, 0x80},
	{0x3222, 0x29},
	{0x3901, 0x02},
	{0x3905, 0x98},
	{0x3e1e, 0x34},
	{0x3900, 0x19},
	{0x391d, 0x04},
	{0x391e, 0x00},
	{0x3641, 0x01},
	{0x3213, 0x04},
	{0x3614, 0x80},
	{0x363a, 0x9f},
	{0x3630, 0x9c},
	{0x3306, 0x48},
	{0x330b, 0xcd},
	{0x3314, 0x04},
	{0x3933, 0x0a},
	{0x3934, 0x18},
	{0x3940, 0x60},
	{0x3942, 0x02},
	{0x3943, 0x1f},
	{0x3960, 0xba},
	{0x3961, 0xae},
	{0x3966, 0xba},
	{0x3980, 0xa0},
	{0x3981, 0x40},
	{0x3982, 0x18},
	{0x3903, 0x08},
	{0x3984, 0x08},
	{0x3985, 0x20},
	{0x3986, 0x50},
	{0x3987, 0xb0},
	{0x3988, 0x08},
	{0x3989, 0x10},
	{0x398a, 0x20},
	{0x398b, 0x30},
	{0x398c, 0x60},
	{0x398d, 0x20},
	{0x398e, 0x10},
	{0x398f, 0x08},
	{0x3990, 0x60},
	{0x3991, 0x24},
	{0x3992, 0x15},
	{0x3993, 0x08},
	{0x3994, 0x0a},
	{0x3995, 0x20},
	{0x3996, 0x38},
	{0x3997, 0xa0},
	{0x3998, 0x08},
	{0x3999, 0x10},
	{0x399a, 0x18},
	{0x399b, 0x30},
	{0x399c, 0x30},
	{0x399d, 0x18},
	{0x399e, 0x10},
	{0x399f, 0x08},
	{0x3637, 0x55},
	{0x366f, 0x2c},
	{0x5000, 0x06},
	{0x5780, 0x7f},
	{0x5781, 0x04},
	{0x5782, 0x03},
	{0x5783, 0x02},
	{0x5784, 0x01},
	{0x5785, 0x18},
	{0x5786, 0x10},
	{0x5787, 0x08},
	{0x5788, 0x02},
	{0x57a0, 0x00},
	{0x57a1, 0x71},
	{0x57a2, 0x01},
	{0x57a3, 0xf1},
	{0x395e, 0xc0},
	{0x3962, 0x88},
	{0x303a, 0xb3},
	{0x303c, 0x0e},
	{0x3035, 0x9b},
	{0x320c, 0x08},
	{0x320d, 0x20},
	{0x320e, 0x0b},
	{0x320f, 0xd0},/*vts for 25fps*/
	{0x3f04, 0x03},
	{0x3f05, 0xec},
	{0x3235, 0x09},
	{0x3236, 0xc2},
	{0x3802, 0x00},
	{0x3963, 0x80},
	{0x303b, 0x16},
	{0x301f, 0x03},
	{0x363b, 0x26},
	{0x3902, 0xc5},
	{0x3e00, 0x00},
	{0x3e01, 0x9c},
	{0x3e02, 0x00},
	{0x3e03, 0x0b},
	{0x3e06, 0x00},
	{0x3e07, 0x80},
	{0x3e08, 0x03},
	{0x3e09, 0x10},
	{0x3301, 0x12},
	{0x3632, 0x08},
	{0x3034, 0x01},
	{0x3039, 0x24},
	{SC2232H_REG_DELAY, 0x0a},
	{SC2232H_REG_END, 0x00},	/* END MARKER */
};

/*
 * the order of the sc2232h_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting sc2232h_win_sizes[] = {
	/* 1920*1080 */
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc2232h_init_regs_1920_1080_25fps_dvp,
	},
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc2232h_init_regs_1920_1080_25fps_mipi,
	},
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 15 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc2232h_init_regs_1920_1080_15fps_dvp,
	}
};

struct tx_isp_sensor_win_setting *wsize = &sc2232h_win_sizes[0];

#if 0
static enum v4l2_mbus_pixelcode sc2232h_mbus_code[] = {
	TISP_VI_FMT_SBGGR10_1X10,
};
#endif

/*
 * the part of driver was fixed.
 */

static struct regval_list sc2232h_stream_on_dvp[] = {
	{0x0100, 0x01},
	{SC2232H_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc2232h_stream_off_dvp[] = {
	{0x0100, 0x00},
	{SC2232H_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc2232h_stream_on_mipi[] = {
	{0x0100,0x01},
	{SC2232H_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc2232h_stream_off_mipi[] = {
	{0x0100, 0x00},
	{SC2232H_REG_END, 0x00},	/* END MARKER */
};

int sc2232h_read(struct tx_isp_subdev *sd, uint16_t reg,	unsigned char *value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned char buf[2] = {reg >> 8, reg & 0xff};
	struct i2c_msg msg[2] = {
		[0] = {
			.addr	= client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		},
		[1] = {
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= value,
		}
	};
	int ret;
	ret = private_i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;

	return ret;
}

int sc2232h_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	uint8_t buf[3] = {(reg >> 8) & 0xff, reg & 0xff, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 3,
		.buf	= buf,
	};
	int ret;
	ret = private_i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}

#if 0
static int sc2232h_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != SC2232H_REG_END) {
		if (vals->reg_num == SC2232H_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc2232h_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}
#endif

static int sc2232h_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != SC2232H_REG_END) {
		if (vals->reg_num == SC2232H_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc2232h_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int sc2232h_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int sc2232h_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = sc2232h_read(sd, 0x3107, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC2232H_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = sc2232h_read(sd, 0x3108, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC2232H_CHIP_ID_M)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	ret = sc2232h_read(sd, 0x3109, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC2232H_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

#if 0
static int sc2232h_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	value *= 2;
	ret = sc2232h_write(sd, 0x3e00, (unsigned char)((value >> 12) & 0x0f));
	ret += sc2232h_write(sd, 0x3e01, (unsigned char)((value >> 4) & 0xff));
	ret += sc2232h_write(sd, 0x3e02, (unsigned char)((value & 0x0f) << 4));

	if (value < 250) {
		ret += sc2232h_write(sd, 0x3314, 0x14);
	}
	else if(value > 450){
		ret += sc2232h_write(sd, 0x3314, 0x04);
	}

	if (ret < 0)
		return ret;

	return 0;
}

static int sc2232h_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	/* denoise logic */
	if (value < 0x110) {
		sc2232h_write(sd,0x3812,0x00);
		sc2232h_write(sd,0x3301,0x12);
		sc2232h_write(sd,0x3632,0x08);
		sc2232h_write(sd,0x3812,0x30);
	}
	else if (value>=0x110&&value<0x310){
		sc2232h_write(sd,0x3812,0x00);
		sc2232h_write(sd,0x3301,0x20);
		sc2232h_write(sd,0x3632,0x08);
		sc2232h_write(sd,0x3812,0x30);
	}
	else if(value>=0x310&&value<0x710){
		sc2232h_write(sd,0x3812,0x00);
		sc2232h_write(sd,0x3301,0x28);
		sc2232h_write(sd,0x3632,0x08);
		sc2232h_write(sd,0x3812,0x30);
	}
	else if(value>=0x710&&value<=0x71e){
		sc2232h_write(sd,0x3812,0x00);
		sc2232h_write(sd,0x3301,0x64);
		sc2232h_write(sd,0x3632,0x08);
		sc2232h_write(sd,0x3812,0x30);
	}
	else{ //may be flick
		sc2232h_write(sd,0x3812,0x00);
		sc2232h_write(sd,0x3301,0x64);
		sc2232h_write(sd,0x3632,0x48);
		sc2232h_write(sd,0x3812,0x30);
	}
	ret = sc2232h_write(sd, 0x3e09, (unsigned char)(value & 0xff));
	ret += sc2232h_write(sd, 0x3e08, (unsigned char)((value >> 8 << 2) | 0x03));
	if (ret < 0)
		return ret;

	return 0;
}
#endif

static int sc2232h_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	int it = (value & 0xffff) * 2;
	int again = (value & 0xffff0000) >> 16;

	/*set integration time*/
	ret = sc2232h_write(sd, 0x3e00, (unsigned char)((it >> 12) & 0xf));
	ret += sc2232h_write(sd, 0x3e01, (unsigned char)((it >> 4) & 0xff));
	ret += sc2232h_write(sd, 0x3e02, (unsigned char)((it & 0x0f) << 4));

	if (it < 250) {
		ret += sc2232h_write(sd, 0x3314, 0x14);
	}
	else if(it > 450){
		ret += sc2232h_write(sd, 0x3314, 0x04);
	}
	/* set denoise logic */
	ret += sc2232h_write(sd,0x3812,0x00);
	if (again < 0x110) {
		ret += sc2232h_write(sd,0x3301,0x12);
		ret += sc2232h_write(sd,0x3632,0x08);
	}
	else if (again>=0x110&&again<0x310){
		ret += sc2232h_write(sd,0x3301,0x20);
		ret += sc2232h_write(sd,0x3632,0x08);
	}
	else if(again>=0x310&&again<0x710){
		ret += sc2232h_write(sd,0x3301,0x28);
		ret += sc2232h_write(sd,0x3632,0x08);
	}
	else if(again>=0x710&&again<=0x71e){
		ret += sc2232h_write(sd,0x3301,0x64);
		ret += sc2232h_write(sd,0x3632,0x08);
	}
	else{ //may be flick
		ret += sc2232h_write(sd,0x3301,0x64);
		ret += sc2232h_write(sd,0x3632,0x48);
	}
	ret += sc2232h_write(sd,0x3812,0x30);
	/*set analog gain*/
	ret += sc2232h_write(sd, 0x3e09, (unsigned char)(again & 0xff));
	ret += sc2232h_write(sd, 0x3e08, (unsigned char)((again >> 8 << 2) | 0x03));
	if (ret < 0)
		return ret;

	return 0;
}

static int sc2232h_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc2232h_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc2232h_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if(!init->enable)
		return ISP_SUCCESS;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	sensor->video.state = TX_ISP_MODULE_DEINIT;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	sensor->priv = wsize;

	return 0;
}

static int sc2232h_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = sc2232h_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if (sensor->video.state == TX_ISP_MODULE_INIT) {
			if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
			sensor->video.state = TX_ISP_MODULE_RUNNING;
				ret = sc2232h_write_array(sd, sc2232h_stream_on_dvp);
			sensor->video.state = TX_ISP_MODULE_RUNNING;
		}
		} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc2232h_write_array(sd, sc2232h_stream_on_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
		}
		ISP_WARNING("sc2232h stream on\n");

	}
	else {
		if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
			ret = sc2232h_write_array(sd, sc2232h_stream_off_dvp);
		sensor->video.state = TX_ISP_MODULE_DEINIT;
		} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc2232h_write_array(sd, sc2232h_stream_off_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
		}
		ISP_WARNING("sc2232h stream off\n");
	}

	return ret;
}

static int sc2232h_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	unsigned int sclk = 0;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned char tmp = 0;

	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (SENSOR_OUTPUT_MAX_FPS << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)) {
		ISP_ERROR("warn: fps(%d) no in range\n", fps);
		return -1;
	}

	if((sensor_max_fps == TX_SENSOR_MAX_FPS_15)&&(data_interface == TX_SENSOR_DATA_INTERFACE_DVP))
		sclk = SC2232H_SUPPORT_SCLK_15FPS;
	else
		sclk = SC2232H_SUPPORT_SCLK;

	ret = sc2232h_read(sd, 0x320c, &tmp);
	hts = tmp;
	ret += sc2232h_read(sd, 0x320d, &tmp);
	if (0 != ret) {
		ISP_ERROR("err: sc2232h read err\n");
		return ret;
	}
	hts = ((hts << 8) + tmp);
	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);

	ret += sc2232h_write(sd, 0x320f, (unsigned char)(vts & 0xff));
	ret += sc2232h_write(sd, 0x320e, (unsigned char)(vts >> 8));
	if (0 != ret) {
		ISP_ERROR("err: sc2232h_write err\n");
		return ret;
	}

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 2;
	sensor->video.attr->integration_time_limit = vts - 2;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 2;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc2232h_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = ISP_SUCCESS;

	if(wsize){
		sensor->video.mbus.width = wsize->width;
		sensor->video.mbus.height = wsize->height;
		sensor->video.mbus.code = wsize->mbus_code;
		sensor->video.mbus.field = TISP_FIELD_NONE;
		sensor->video.mbus.colorspace = wsize->colorspace;
		sensor->video.fps = wsize->fps;
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	}

	return ret;
}

static int sensor_attr_check(struct tx_isp_subdev *sd)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	unsigned long rate;

	switch(info->default_boot){
	    case 0:
	        ISP_WARNING("To use mipi, please modify --vi to 1");
            break;
        case 1:
            wsize = &sc2232h_win_sizes[1];
            memcpy((void*)(&(sc2232h_attr.mipi)),(void*)(&sc2232h_mipi),sizeof(sc2232h_mipi));
            break;
        case 2:
            ISP_WARNING("To use mipi, please modify --vi to 1");
//            ret = set_sensor_gpio_function(sensor_gpio_func);
//            if (ret < 0)
//                goto err_set_sensor_gpio;
//            sc2232h_attr.dvp.gpio = sensor_gpio_func;
//            mbus = sc2232h_mbus_code[0];
//            wsize->regs = sc2232h_init_regs_1920_1080_25fps_dvp;
//            memcpy((void*)(&(sc2232h_attr.dvp)),(void*)(&sc2232h_dvp),sizeof(sc2232h_dvp));
//            sc2232h_attr.dbus_type = data_interface;
//            sc2232h_attr.data_type = data_type;
//            sc2232h_attr.dvp.gpio = sensor_gpio_func;
//            if(sensor_max_fps == TX_SENSOR_MAX_FPS_15){
//                wsize = &sc2232h_win_sizes[2];
//                sc2232h_attr.max_integration_time_native = 1498;
//                sc2232h_attr.integration_time_limit = 1498;
//                sc2232h_attr.total_width = 2400;
//                sc2232h_attr.total_height = 1500;
//                sc2232h_attr.max_integration_time = 1498;
//            }
            break;
        default:
            ISP_ERROR("Have no this setting!!!\n");
	}

	switch(info->video_interface){
        case TISP_SENSOR_VI_MIPI_CSI0:
            sc2232h_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            sc2232h_attr.mipi.index = 0;
            break;
        case TISP_SENSOR_VI_MIPI_CSI1:
            sc2232h_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            sc2232h_attr.mipi.index = 1;
            break;
        case TISP_SENSOR_VI_DVP:
            sc2232h_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
            break;
        default:
            ISP_ERROR("Have no this interface!!!\n");
	}

	switch(info->mclk){
        case TISP_SENSOR_MCLK0:
            sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim0");
            set_sensor_mclk_function(0);
            break;
        case TISP_SENSOR_MCLK1:
            sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim1");
            set_sensor_mclk_function(1);
            break;
        case TISP_SENSOR_MCLK2:
            sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim2");
            set_sensor_mclk_function(2);
            break;
        default:
            ISP_ERROR("Have no this MCLK Source!!!\n");
	}

	rate = private_clk_get_rate(sensor->mclk);
	if (IS_ERR(sensor->mclk)) {
		ISP_ERROR("Cannot get sensor input clock cgu_cim\n");
		goto err_get_mclk;
	}
	private_clk_set_rate(sensor->mclk, 24000000);
	private_clk_prepare_enable(sensor->mclk);

	reset_gpio = info->rst_gpio;
	pwdn_gpio = info->pwdn_gpio;

	return 0;

err_get_mclk:
	return -1;
}

static int sc2232h_g_chip_ident(struct tx_isp_subdev *sd,
				struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;
	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"sc2232h_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(10);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"sc2232h_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = sc2232h_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an sc2232h chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("sc2232h chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	if(chip){
		memcpy(chip->name, "sc2232h", sizeof("sc2232h"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int sc2232h_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
{
	long ret = 0;
	struct tx_isp_sensor_value *sensor_val = arg;


	if(IS_ERR_OR_NULL(sd)){
		ISP_ERROR("[%d]The pointer is invalid!\n", __LINE__);
		return -EINVAL;
	}
	switch(cmd){
	case TX_ISP_EVENT_SENSOR_EXPO:
		if(arg)
			ret = sc2232h_set_expo(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			/*ret = sc2232h_set_integration_time(sd, sensor_val->value);*/
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			/*ret = sc2232h_set_analog_gain(sd, sensor_val->value);*/
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = sc2232h_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = sc2232h_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = sc2232h_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
			ret = sc2232h_write_array(sd, sc2232h_stream_off_dvp);
		} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc2232h_write_array(sd, sc2232h_stream_off_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
			ret = sc2232h_write_array(sd, sc2232h_stream_on_dvp);
		} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc2232h_write_array(sd, sc2232h_stream_on_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = sc2232h_set_fps(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return 0;
}

static int sc2232h_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
{
	unsigned char val = 0;
	int len = 0;
	int ret = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = sc2232h_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int sc2232h_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	sc2232h_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops sc2232h_core_ops = {
	.g_chip_ident = sc2232h_g_chip_ident,
	.reset = sc2232h_reset,
	.init = sc2232h_init,
	/*.ioctl = sc2232h_ops_ioctl,*/
	.g_register = sc2232h_g_register,
	.s_register = sc2232h_s_register,
};

static struct tx_isp_subdev_video_ops sc2232h_video_ops = {
	.s_stream = sc2232h_s_stream,
};

static struct tx_isp_subdev_sensor_ops	sc2232h_sensor_ops = {
	.ioctl	= sc2232h_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops sc2232h_ops = {
	.core = &sc2232h_core_ops,
	.video = &sc2232h_video_ops,
	.sensor = &sc2232h_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "sc2232h",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int sc2232h_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tx_isp_subdev *sd;
	struct tx_isp_video_in *video;
	struct tx_isp_sensor *sensor;

	sensor = (struct tx_isp_sensor *)kzalloc(sizeof(*sensor), GFP_KERNEL);
	if(!sensor){
		ISP_ERROR("Failed to allocate sensor subdev.\n");
		return -ENOMEM;
	}
	memset(sensor, 0 ,sizeof(*sensor));
	sensor->dev = &client->dev;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->video.attr = &sc2232h_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &sc2232h_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	ISP_WARNING("probe ok ------->sc2232h\n");

	return 0;
}

static int sc2232h_remove(struct i2c_client *client)
{
	struct tx_isp_subdev *sd = private_i2c_get_clientdata(client);
	struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);

	if(reset_gpio != -1)
		private_gpio_free(reset_gpio);
	if(pwdn_gpio != -1)
		private_gpio_free(pwdn_gpio);

	private_clk_disable_unprepare(sensor->mclk);
	private_devm_clk_put(&client->dev, sensor->mclk);
	tx_isp_subdev_deinit(sd);
	kfree(sensor);

	return 0;
}

static const struct i2c_device_id sc2232h_id[] = {
	{ "sc2232h", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sc2232h_id);

static struct i2c_driver sc2232h_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "sc2232h",
	},
	.probe		= sc2232h_probe,
	.remove		= sc2232h_remove,
	.id_table	= sc2232h_id,
};

static __init int init_sc2232h(void)
{
	return private_i2c_add_driver(&sc2232h_driver);
}

static __exit void exit_sc2232h(void)
{
	private_i2c_del_driver(&sc2232h_driver);
}

module_init(init_sc2232h);
module_exit(exit_sc2232h);

MODULE_DESCRIPTION("A low-level driver for SmartSens sc2232h sensors");
MODULE_LICENSE("GPL");
