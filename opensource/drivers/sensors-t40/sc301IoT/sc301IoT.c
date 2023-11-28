/*
 * sc301IoT.c
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

#define SC301IoT_CHIP_ID_H	(0xcc)
#define SC301IoT_CHIP_ID_L	(0x40)
#define SC301IoT_REG_END		0xffff
#define SC301IoT_REG_DELAY		0xfffe
#define SC301IoT_SUPPORT_30FPS_SCLK (54000000)
#define SC301IoT_SUPPORT_30FPS_HDR_SCLK (108000000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 2
#define SENSOR_VERSION	"H20220527a"

static int reset_gpio = GPIO_PC(27);
static int pwdn_gpio = -1;
static int wdr_bufsize = 2 * 2304 * 200;//cache lines corrponding on VPB1

static int shvflip = 0;
module_param(shvflip, int, S_IRUGO);
MODULE_PARM_DESC(shvflip, "Sensor HV Flip Enable interface");

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

struct again_lut sc301IoT_again_lut[] = {
	{0x80, 0},
	{0x84, 2886},
	{0x88, 5776},
	{0x8c, 8494},
	{0x90, 11136},
	{0x94, 13706},
	{0x98, 16287},
	{0x9c, 18723},
	{0xa0, 21097},
	{0xa4, 23414},
	{0xa8, 25746},
	{0xac, 27953},
	{0xb0, 30109},
	{0xb4, 32217},
	{0xb8, 34345},
	{0xbc, 36361},
	{0xc0, 38336},
	{0xc4, 40270},
	{0xc8, 42226},
	{0x4080, 42588},
	{0x4084, 45495},
	{0x4088, 48373},
	{0x408c, 51055},
	{0x4090, 53717},
	{0x4094, 56306},
	{0x4098, 58877},
	{0x409c, 61331},
	{0x40a0, 63674},
	{0x40a4, 66007},
	{0x40a8, 68330},
	{0x40ac, 70508},
	{0x40b0, 72681},
	{0x40b4, 74804},
	{0x40b8, 76924},
	{0x40bc, 78914},
	{0x40c0, 80904},
	{0x40c4, 82852},
	{0x40c8, 84800},
	{0x40cc, 86633},
	{0x40d0, 88469},
	{0x40d4, 90269},
	{0x40d8, 92071},
	{0x40dc, 93805},
	{0x40e0, 95473},
	{0x40e4, 97146},
	{0x40e8, 98823},
	{0x40ec, 100438},
	{0x40f0, 101994},
	{0x40f4, 103556},
	{0x40f8, 105124},
	{0x40fc, 106636},
	{0x4880, 108124},
	{0x4884, 111002},
	{0x4888, 113909},
	{0x488c, 116619},
	{0x4890, 119253},
	{0x4894, 121842},
	{0x4898, 124413},
	{0x489c, 126842},
	{0x48a0, 129234},
	{0x48a4, 131543},
	{0x48a8, 133866},
	{0x48ac, 136066},
	{0x48b0, 138239},
	{0x48b4, 140340},
	{0x48b8, 142460},
	{0x48bc, 144491},
	{0x48c0, 146460},
	{0x48c4, 148388},
	{0x48c8, 150356},
	{0x48cc, 152207},
	{0x48d0, 154023},
	{0x48d4, 155823},
	{0x48d8, 157625},
	{0x48dc, 159341},
	{0x48e0, 161026},
	{0x48e4, 162699},
	{0x48e8, 164375},
	{0x48ec, 165974},
	{0x48f0, 167562},
	{0x48f4, 169108},
	{0x48f8, 170675},
	{0x48fc, 172187},
	{0x4980, 173660},
	{0x4984, 176553},
	{0x4988, 179431},
	{0x498c, 182155},
	{0x4990, 184803},
	{0x4994, 187365},
	{0x4998, 189949},
	{0x499c, 192378},
	{0x49a0, 194758},
	{0x49a4, 197079},
	{0x49a8, 199402},
	{0x49ac, 201614},
	{0x49b0, 203775},
	{0x49b4, 205876},
	{0x49b8, 208006},
	{0x49bc, 210017},
	{0x49c0, 211996},
	{0x49c4, 213934},
	{0x49c8, 215882},
	{0x49cc, 217743},
	{0x49d0, 219559},
	{0x49d4, 221350},
	{0x49d8, 223161},
	{0x49dc, 224877},
	{0x49e0, 226571},
	{0x49e4, 228235},
	{0x49e8, 229911},
	{0x49ec, 231518},
	{0x49f0, 233090},
	{0x49f4, 234644},
	{0x49f8, 236219},
	{0x49fc, 237715},
	{0x4b80, 239196},
	{0x4b84, 242081},
	{0x4b88, 244974},
	{0x4b8c, 247691},
	{0x4b90, 250332},
	{0x4b94, 252901},
	{0x4b98, 255485},
	{0x4b9c, 257920},
	{0x4ba0, 260294},
	{0x4ba4, 262609},
	{0x4ba8, 264944},
	{0x4bac, 267150},
	{0x4bb0, 269305},
	{0x4bb4, 271412},
	{0x4bb8, 273542},
	{0x4bbc, 275558},
	{0x4bc0, 277532},
	{0x4bc4, 279465},
	{0x4bc8, 281423},
	{0x4bcc, 283279},
	{0x4bd0, 285100},
	{0x4bd4, 286886},
	{0x4bd8, 288697},
	{0x4bdc, 290417},
	{0x4be0, 292107},
	{0x4be4, 293766},
	{0x4be8, 295451},
	{0x4bec, 297054},
	{0x4bf0, 298630},
	{0x4bf4, 300180},
	{0x4bf8, 301755},
	{0x4bfc, 303255},
	{0x4f80, 304732},
	{0x4f84, 307617},
	{0x4f88, 310510},
	{0x4f8c, 313227},
	{0x4f90, 315868},
	{0x4f94, 318437},
	{0x4f98, 321021},
	{0x4f9c, 323456},
	{0x4fa0, 325830},
	{0x4fa4, 328145},
	{0x4fa8, 330480},
	{0x4fac, 332686},
	{0x4fb0, 334841},
	{0x4fb4, 336948},
	{0x4fb8, 339078},
	{0x4fbc, 341094},
	{0x4fc0, 343068},
	{0x4fc4, 345001},
	{0x4fc8, 346959},
	{0x4fcc, 348815},
	{0x4fd0, 350636},
	{0x4fd4, 352422},
	{0x4fd8, 354233},
	{0x4fdc, 355953},
	{0x4fe0, 357643},
	{0x4fe4, 359302},
	{0x4fe8, 360987},
	{0x4fec, 362590},
	{0x4ff0, 364166},
	{0x4ff4, 365716},
	{0x4ff8, 367291},
	{0x4ffc, 368791},

};

struct tx_isp_sensor_attribute sc301IoT_attr;

unsigned int sc301IoT_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = sc301IoT_again_lut;

	while(lut->gain <= sc301IoT_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return 0;
		} else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		} else {
			if((lut->gain == sc301IoT_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}
		lut++;
	}

	return isp_gain;
}

unsigned int sc301IoT_alloc_again_short(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = sc301IoT_again_lut;

	while(lut->gain <= sc301IoT_attr.max_again_short) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return 0;
		} else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		} else {
			if((lut->gain == sc301IoT_attr.max_again_short) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}
		lut++;
	}

	return isp_gain;
}


unsigned int sc301IoT_alloc_integration_time(unsigned int it, unsigned char shift, unsigned int *sensor_it)
{
	unsigned int expo = it >> shift;
	unsigned int isp_it = it;

	/* linear mode, expo step is 1, min expo is 1 */
	if (sc301IoT_attr.data_type == TX_SENSOR_DATA_TYPE_LINEAR) {
		expo = expo;
	}
	/* hdr mode, long expo step is 2, min expo is 1 */
	if (sc301IoT_attr.data_type == TX_SENSOR_DATA_TYPE_WDR_DOL) {
		if (expo % 2 == 0)
			expo = expo - 1;
		if (expo % 2)
			expo = expo;
	}
	isp_it = expo << shift;
	*sensor_it = expo;

	return isp_it;
}

unsigned int sc301IoT_alloc_integration_time_short(unsigned int it, unsigned char shift, unsigned int *sensor_it)
{
	unsigned int expo = it >> shift;
	unsigned int isp_it = it;

	/* hdr mode, short expo step is 2, min expo is 1 */
	if (expo % 2 == 0)
		expo = expo - 1;
	if (expo % 2)
		expo = expo;

	*sensor_it = expo;
	isp_it = expo << shift;

	return isp_it;
}

unsigned int sc301IoT_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus sc301IoT_mipi_linear = {
	.mode = SENSOR_MIPI_OTHER_MODE,
	.clk = 540,
	.lans = 2,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,
	.mipi_sc.hcrop_diff_en = 0,
	.mipi_sc.mipi_vcomp_en = 0,
	.mipi_sc.mipi_hcomp_en = 0,
	.mipi_sc.line_sync_mode = 0,
	.mipi_sc.work_start_flag = 0,
	.image_twidth = 2048,
	.image_theight = 1536,
	.mipi_sc.mipi_crop_start0x = 0,
	.mipi_sc.mipi_crop_start0y = 0,
	.mipi_sc.mipi_crop_start1x = 0,
	.mipi_sc.mipi_crop_start1y = 0,
	.mipi_sc.mipi_crop_start2x = 0,
	.mipi_sc.mipi_crop_start2y = 0,
	.mipi_sc.mipi_crop_start3x = 0,
	.mipi_sc.mipi_crop_start3y = 0,
	.mipi_sc.data_type_en = 0,
	.mipi_sc.data_type_value = RAW10,
	.mipi_sc.del_start = 0,
	.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
	.mipi_sc.sensor_fid_mode = 0,
	.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
};

struct tx_isp_mipi_bus sc301IoT_mipi_dol = {
	.mode = SENSOR_MIPI_OTHER_MODE,
	.clk = 1080,
	.lans = 2,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
	.mipi_sc.hcrop_diff_en = 0,
	.mipi_sc.mipi_vcomp_en = 0,
	.mipi_sc.mipi_hcomp_en = 0,
	.image_twidth = 2048,
	.image_theight = 1536,
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
	.mipi_sc.sensor_frame_mode = TX_SENSOR_WDR_2_FRAME_MODE,
	.mipi_sc.sensor_fid_mode = 0,
	.mipi_sc.sensor_mode = TX_SENSOR_VC_MODE,
};

struct tx_isp_sensor_attribute sc301IoT_attr={
	.name = "sc301IoT",
	.chip_id = 0xcc40,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x30,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.mode = SENSOR_MIPI_OTHER_MODE,
		.clk = 540,
		.lans = 2,
		.settle_time_apative_en = 0,
		.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
		.mipi_sc.hcrop_diff_en = 0,
		.mipi_sc.mipi_vcomp_en = 0,
		.mipi_sc.mipi_hcomp_en = 0,
		.mipi_sc.line_sync_mode = 0,
		.mipi_sc.work_start_flag = 0,
		.image_twidth = 2048,
		.image_theight = 1536,
		.mipi_sc.mipi_crop_start0x = 0,
		.mipi_sc.mipi_crop_start0y = 0,
		.mipi_sc.mipi_crop_start1x = 0,
		.mipi_sc.mipi_crop_start1y = 0,
		.mipi_sc.mipi_crop_start2x = 0,
		.mipi_sc.mipi_crop_start2y = 0,
		.mipi_sc.mipi_crop_start3x = 0,
		.mipi_sc.mipi_crop_start3y = 0,
		.mipi_sc.data_type_en = 0,
		.mipi_sc.data_type_value = RAW10,
		.mipi_sc.del_start = 0,
		.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
		.mipi_sc.sensor_fid_mode = 0,
		.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
	},
	.data_type = TX_SENSOR_DATA_TYPE_LINEAR,
	.max_again = 368791,
	.max_again_short = 368791,
	.max_dgain = 0,
	.min_integration_time = 1,
	.min_integration_time_short = 1,
	.min_integration_time_native = 1,
	.max_integration_time_native = 1920 - 5,
	.integration_time_limit = 1920 - 5,
	.total_width = 2250,
	.total_height = 1920,
	.max_integration_time = 1920 - 5,
	.one_line_expr_in_us = 25,
	.expo_fs = 1,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = sc301IoT_alloc_again,
	.sensor_ctrl.alloc_again_short = sc301IoT_alloc_again_short,
	.sensor_ctrl.alloc_dgain = sc301IoT_alloc_dgain,
	.sensor_ctrl.alloc_integration_time = sc301IoT_alloc_integration_time,
	.sensor_ctrl.alloc_integration_time_short = sc301IoT_alloc_integration_time_short,
};

static struct regval_list sc301IoT_init_regs_2048_1536_25fps_mipi[] = {
	{0x0103,0x01},
	{0x0100,0x00},
	{0x36e9,0x80},
	{0x37f9,0x80},
	{0x301c,0x78},
	{0x301f,0x05},
	{0x30b8,0x44},
	{0x3208,0x08},
	{0x3209,0x00},
	{0x320a,0x06},
	{0x320b,0x00},
	{0x320c,0x04},
	{0x320d,0x65},/* 0x465 = 1125 = hts */
	{0x320e,0x07},
	{0x320f,0x80},/* 0x780 = 1920 = vts */
	{0x3214,0x11},
	{0x3215,0x11},
	{0x3223,0xc0},
	{0x3253,0x0c},
	{0x3274,0x09},
	{0x3301,0x08},
	{0x3306,0x58},
	{0x3308,0x08},
	{0x330a,0x00},
	{0x330b,0xe0},
	{0x330e,0x10},
	{0x331e,0x55},
	{0x331f,0x7d},
	{0x3333,0x10},
	{0x3334,0x40},
	{0x335e,0x06},
	{0x335f,0x08},
	{0x3364,0x5e},
	{0x337c,0x02},
	{0x337d,0x0a},
	{0x3390,0x01},
	{0x3391,0x03},
	{0x3392,0x07},
	{0x3393,0x08},
	{0x3394,0x08},
	{0x3395,0x08},
	{0x3396,0x08},
	{0x3397,0x09},
	{0x3398,0x1f},
	{0x3399,0x08},
	{0x339a,0x20},
	{0x339b,0x40},
	{0x339c,0x78},
	{0x33a2,0x04},
	{0x33ad,0x0c},
	{0x33b1,0x80},
	{0x33b3,0x30},
	{0x33f9,0x68},
	{0x33fb,0x88},
	{0x33fc,0x48},
	{0x33fd,0x5f},
	{0x349f,0x03},
	{0x34a6,0x48},
	{0x34a7,0x5f},
	{0x34a8,0x30},
	{0x34a9,0x30},
	{0x34aa,0x00},
	{0x34ab,0xf0},
	{0x34ac,0x01},
	{0x34ad,0x12},
	{0x34f8,0x5f},
	{0x34f9,0x10},
	{0x3630,0xf0},
	{0x3631,0x85},
	{0x3632,0x74},
	{0x3633,0x22},
	{0x3637,0x4d},
	{0x3638,0xcb},
	{0x363a,0x8b},
	{0x3641,0x00},
	{0x3670,0x4e},
	{0x3674,0xf0},
	{0x3675,0xc0},
	{0x3676,0xc0},
	{0x3677,0x85},
	{0x3678,0x8a},
	{0x3679,0x8d},
	{0x367c,0x48},
	{0x367d,0x49},
	{0x367e,0x49},
	{0x367f,0x5f},
	{0x3690,0x22},
	{0x3691,0x33},
	{0x3692,0x44},
	{0x3699,0x88},
	{0x369a,0x98},
	{0x369b,0xc4},
	{0x369c,0x48},
	{0x369d,0x5f},
	{0x36a2,0x49},
	{0x36a3,0x4f},
	{0x36ea,0x09},
	{0x36eb,0x0d},
	{0x36ec,0x1c},
	{0x36ed,0x25},
	{0x370f,0x01},
	{0x3714,0x80},
	{0x3722,0x09},
	{0x3724,0x41},
	{0x3725,0xc1},
	{0x3728,0x00},
	{0x3771,0x09},
	{0x3772,0x05},
	{0x3773,0x05},
	{0x377a,0x48},
	{0x377b,0x49},
	{0x37fa,0x06},
	{0x37fb,0x33},
	{0x37fc,0x11},
	{0x37fd,0x38},
	{0x3905,0x8d},
	{0x391d,0x08},
	{0x3922,0x1a},
	{0x3926,0x21},
	{0x3933,0x80},
	{0x3934,0x02},
	{0x3937,0x72},
	{0x3939,0x00},
	{0x393a,0x03},
	{0x39dc,0x02},
	{0x3e00,0x00},
	{0x3e01,0x77},
	{0x3e02,0xc0},
	{0x3e03,0x0b},
	{0x3e1b,0x2a},
	{0x4407,0x34},
	{0x440e,0x02},
	{0x5001,0x40},
	{0x5007,0x80},
	{0x36e9,0x24},
	{0x37f9,0x24},
	{0x0100,0x01},
	{SC301IoT_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc301IoT_init_regs_2048_1536_30fps_hdr_mipi[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x36e9, 0x80},
	{0x37f9, 0x80},
	{0x301c, 0x78},
	{0x301f, 0x12},
	{0x30b8, 0x44},
	{0x3208, 0x08},
	{0x3209, 0x00},
	{0x320a, 0x06},
	{0x320b, 0x00},
	{0x320c, 0x04},
	{0x320d, 0x65},
	{0x320e, 0x0c},
	{0x320f, 0x80},
	{0x3214, 0x11},
	{0x3215, 0x11},
	{0x3223, 0xc0},
	{0x3250, 0xff},
	{0x3253, 0x0c},
	{0x3274, 0x09},
	{0x3281, 0x01},
	{0x3301, 0x08},
	{0x3304, 0x80},
	{0x3306, 0x58},
	{0x3308, 0x08},
	{0x3309, 0xa0},
	{0x330a, 0x00},
	{0x330b, 0xe0},
	{0x330e, 0x10},
	{0x3314, 0x14},
	{0x331e, 0x71},
	{0x331f, 0x91},
	{0x3333, 0x10},
	{0x3334, 0x40},
	{0x335e, 0x06},
	{0x335f, 0x08},
	{0x3364, 0x5e},
	{0x337c, 0x02},
	{0x337d, 0x0a},
	{0x3390, 0x01},
	{0x3391, 0x03},
	{0x3392, 0x07},
	{0x3393, 0x08},
	{0x3394, 0x08},
	{0x3395, 0x08},
	{0x3396, 0x08},
	{0x3397, 0x09},
	{0x3398, 0x1f},
	{0x3399, 0x08},
	{0x339a, 0x14},
	{0x339b, 0x28},
	{0x339c, 0x78},
	{0x33a2, 0x04},
	{0x33ad, 0x0c},
	{0x33b1, 0x80},
	{0x33b3, 0x38},
	{0x33f9, 0x58},
	{0x33fb, 0x80},
	{0x33fc, 0x48},
	{0x33fd, 0x4f},
	{0x349f, 0x03},
	{0x34a6, 0x48},
	{0x34a7, 0x4f},
	{0x34a8, 0x38},
	{0x34a9, 0x28},
	{0x34aa, 0x00},
	{0x34ab, 0xe0},
	{0x34ac, 0x01},
	{0x34ad, 0x08},
	{0x34f8, 0x5f},
	{0x34f9, 0x18},
	{0x3630, 0xf0},
	{0x3631, 0x85},
	{0x3632, 0x74},
	{0x3633, 0x22},
	{0x3637, 0x4d},
	{0x3638, 0xcb},
	{0x363a, 0x8b},
	{0x363c, 0x08},
	{0x3641, 0x38},
	{0x3670, 0x4e},
	{0x3674, 0xc0},
	{0x3675, 0xa0},
	{0x3676, 0x90},
	{0x3677, 0x83},
	{0x3678, 0x86},
	{0x3679, 0x89},
	{0x367c, 0x48},
	{0x367d, 0x4f},
	{0x367e, 0x48},
	{0x367f, 0x4b},
	{0x3690, 0x33},
	{0x3691, 0x44},
	{0x3692, 0x55},
	{0x3699, 0x8a},
	{0x369a, 0xa1},
	{0x369b, 0xc2},
	{0x369c, 0x48},
	{0x369d, 0x4f},
	{0x36a2, 0x4b},
	{0x36a3, 0x4f},
	{0x36ea, 0x09},
	{0x36eb, 0x0d},
	{0x36ec, 0x0c},
	{0x36ed, 0x25},
	{0x370f, 0x01},
	{0x3714, 0x80},
	{0x3722, 0x01},
	{0x3724, 0x41},
	{0x3725, 0xc1},
	{0x3728, 0x00},
	{0x3771, 0x09},
	{0x3772, 0x09},
	{0x3773, 0x05},
	{0x377a, 0x48},
	{0x377b, 0x4f},
	{0x37fa, 0x09},
	{0x37fb, 0x31},
	{0x37fc, 0x10},
	{0x37fd, 0x18},
	{0x3905, 0x8d},
	{0x391d, 0x08},
	{0x3922, 0x1a},
	{0x3926, 0x21},
	{0x3933, 0x80},
	{0x3934, 0x0d},
	{0x3937, 0x6a},
	{0x3939, 0x00},
	{0x393a, 0x0e},
	{0x39dc, 0x02},
	{0x3e00, 0x00},
	{0x3e01, 0xb9},
	{0x3e02, 0xc0},
	{0x3e03, 0x0b},
	{0x3e04, 0x0b},
	{0x3e05, 0xa0},
	{0x3e1b, 0x2a},
	{0x3e23, 0x00},
	{0x3e24, 0xbf},
	{0x4407, 0x34},
	{0x440e, 0x02},
	{0x4509, 0x10},
	{0x4816, 0x71},
	{0x5001, 0x40},
	{0x5007, 0x80},
	{0x36e9, 0x24},
	{0x37f9, 0x24},
	{0x0100, 0x01},
	{SC301IoT_REG_END, 0x00},	/* END MARKER */
};

static struct tx_isp_sensor_win_setting sc301IoT_win_sizes[] = {
	/* [0] resolution 2048*1536 @25fps max*/
	{
		.width		= 2048,
		.height		= 1536,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc301IoT_init_regs_2048_1536_25fps_mipi,
	},
	/* [1] resolution 2048*1536 @HDR 30fps max*/
	{
		.width		= 2048,
		.height		= 1536,
		.fps		= 30 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc301IoT_init_regs_2048_1536_30fps_hdr_mipi,
	},
};
struct tx_isp_sensor_win_setting *wsize = &sc301IoT_win_sizes[1];

static struct regval_list sc301IoT_stream_on[] = {
	//{0x0100, 0x01},
	{SC301IoT_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc301IoT_stream_off[] = {
	//{0x0100, 0x00},
	{SC301IoT_REG_END, 0x00},	/* END MARKER */
};

int sc301IoT_read(struct tx_isp_subdev *sd, uint16_t reg, unsigned char *value)
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

int sc301IoT_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
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
static int sc301IoT_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != SC301IoT_REG_END) {
		if (vals->reg_num == SC301IoT_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc301IoT_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}
#endif

static int sc301IoT_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != SC301IoT_REG_END) {
		if (vals->reg_num == SC301IoT_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc301IoT_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int sc301IoT_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int sc301IoT_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = sc301IoT_read(sd, 0x3107, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC301IoT_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = sc301IoT_read(sd, 0x3108, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC301IoT_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}
#if 0
static int sc301IoT_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	int it = (value & 0xffff);
	int again = (value & 0xffff0000) >> 16;

	/* integration time */
	ret = sc301IoT_write(sd, 0x3e00, (unsigned char)((it >> 12) & 0xf));
	ret += sc301IoT_write(sd, 0x3e01, (unsigned char)((it >> 4) & 0xff));
	ret += sc301IoT_write(sd, 0x3e02, (unsigned char)((it & 0x0f) << 4));

	/* sensor analog gain */
	ret += sc301IoT_write(sd, 0x3e09, (unsigned char)(((again >> 8) & 0xff)));
	/* sensor dig fine gain */
	ret += sc301IoT_write(sd, 0x3e07, (unsigned char)(again & 0xff));
	if (ret < 0)
		return ret;

	return 0;
}

#else
static int sc301IoT_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	value *= 1;
	ret += sc301IoT_write(sd, 0x3e00, (unsigned char)((value >> 12) & 0xf));
	ret += sc301IoT_write(sd, 0x3e01, (unsigned char)((value >> 4) & 0xff));
	ret += sc301IoT_write(sd, 0x3e02, (unsigned char)((value & 0x0f) << 4));
	if (ret < 0)
		return ret;

	return 0;
}

static int sc301IoT_set_integration_time_short(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	value *= 1;
	ret += sc301IoT_write(sd, 0x3e22, (unsigned char)((value >> 12) & 0xf));
	ret += sc301IoT_write(sd, 0x3e04, (unsigned char)((value >> 4) & 0xff));
	ret += sc301IoT_write(sd, 0x3e05, (unsigned char)((value & 0x0f) << 4));
	if (ret < 0)
		return ret;

	return 0;
}

static int sc301IoT_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	ret += sc301IoT_write(sd, 0x3e07, (unsigned char)(value & 0xff));
	ret += sc301IoT_write(sd, 0x3e09, (unsigned char)(((value >> 8) & 0xff)));
	if (ret < 0)
		return ret;

	return 0;
}

static int sc301IoT_set_analog_gain_short(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	ret += sc301IoT_write(sd, 0x3e11, (unsigned char)(value & 0xff));
	ret += sc301IoT_write(sd, 0x3e13, (unsigned char)(((value >> 8) & 0xff)));
	if (ret < 0)
		return ret;

	return 0;
}
#endif

static int sc301IoT_set_logic(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc301IoT_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sensor_set_attr(struct tx_isp_subdev *sd, struct tx_isp_sensor_win_setting *wise)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;

	return 0;
}

static int sc301IoT_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if(!init->enable){
		return ISP_SUCCESS;
	} else {
		sensor_set_attr(sd, wsize);
		sensor->video.state = TX_ISP_MODULE_DEINIT;
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
		sensor->priv = wsize;
	}

	return 0;
}

static int sc301IoT_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = sc301IoT_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if(sensor->video.state == TX_ISP_MODULE_INIT){
			ret = sc301IoT_write_array(sd, sc301IoT_stream_on);
			ISP_WARNING("sc301IoT stream on\n");
			sensor->video.state = TX_ISP_MODULE_RUNNING;
		}
	} else {
		ret = sc301IoT_write_array(sd, sc301IoT_stream_off);
		ISP_WARNING("sc301IoT stream off\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
	}

	return ret;
}

static int sc301IoT_set_fps(struct tx_isp_subdev *sd, int fps)
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
		ISP_ERROR("warn: fps(%d) not in range\n", fps);
		return -1;
	}
	if (sc301IoT_attr.data_type == TX_SENSOR_DATA_TYPE_LINEAR)
		sclk = SC301IoT_SUPPORT_30FPS_SCLK;
	if (sc301IoT_attr.data_type == TX_SENSOR_DATA_TYPE_WDR_DOL)
		sclk = SC301IoT_SUPPORT_30FPS_HDR_SCLK;

	ret = sc301IoT_read(sd, 0x320c, &tmp);
	hts = tmp;
	ret += sc301IoT_read(sd, 0x320d, &tmp);
	if (0 != ret) {
		ISP_ERROR("err: sc301IoT read err\n");
		return ret;
	}
	hts = (hts << 8) + tmp;
	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);

	ret += sc301IoT_write(sd, 0x320f, (unsigned char)(vts & 0xff));
	ret += sc301IoT_write(sd, 0x320e, (unsigned char)(vts >> 8));
	if (0 != ret) {
		ISP_ERROR("err: sc301IoT_write err\n");
		return ret;
	}

	if (sc301IoT_attr.data_type == TX_SENSOR_DATA_TYPE_LINEAR) {
		sensor->video.attr->max_integration_time_native = vts - 4;
		sensor->video.attr->integration_time_limit = vts - 4;
		sensor->video.attr->max_integration_time = vts - 4;
	}
	/* in hdr mode
	 * max long expo = vts - short expo start point -7
	 * max short expo = short expo start point - 5
	 */
	if (sc301IoT_attr.data_type == TX_SENSOR_DATA_TYPE_WDR_DOL) {
		sensor->video.attr->max_integration_time_short = (vts - 12) / 17;
		sensor->video.attr->max_integration_time_native = vts - sensor->video.attr->max_integration_time_short * 16;
		sensor->video.attr->integration_time_limit = vts - sensor->video.attr->max_integration_time_short * 16;
		sensor->video.attr->max_integration_time = vts - sensor->video.attr->max_integration_time_short * 16;
	}

	sensor->video.fps = fps;
	sensor->video.attr->total_height = vts;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc301IoT_set_mode(struct tx_isp_subdev *sd, int value)
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

struct clk *sclka;
static int sensor_attr_check(struct tx_isp_subdev *sd)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	struct i2c_client *client = tx_isp_get_subdevdata(sd);

	switch (info->default_boot) {
		case 0:
			sc301IoT_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
			memcpy((void*)(&(sc301IoT_attr.mipi)),(void*)(&sc301IoT_mipi_linear),sizeof(sc301IoT_mipi_linear));
			wsize = &sc301IoT_win_sizes[0];
			sc301IoT_attr.total_width = 2250;
			sc301IoT_attr.total_height = 1920;
			sc301IoT_attr.integration_time_limit = 1920 - 4;
			sc301IoT_attr.max_integration_time_native = 1920 - 4;
			sc301IoT_attr.max_integration_time = 1920 - 4;
			sc301IoT_attr.min_integration_time = 1;
			break;
		case 1:
			sc301IoT_attr.data_type = TX_SENSOR_DATA_TYPE_WDR_DOL;
			memcpy(&sc301IoT_attr.mipi, &sc301IoT_mipi_dol, sizeof(sc301IoT_mipi_dol));
			wsize = &sc301IoT_win_sizes[1];
			sc301IoT_attr.wdr_cache = wdr_bufsize;
			sc301IoT_attr.total_width = 2250;
			sc301IoT_attr.total_height = 3200;
			sc301IoT_attr.one_line_expr_in_us = 28;
			sc301IoT_attr.integration_time_limit = 3000;
			sc301IoT_attr.max_integration_time_native = 3000;
			sc301IoT_attr.max_integration_time = 3000;
			sc301IoT_attr.max_integration_time_short = 200;
			break;
		default:
			ISP_ERROR("this boot setting is not supported yet!!!\n");
	}

	switch (info->video_interface) {
		case TISP_SENSOR_VI_MIPI_CSI0:
			sc301IoT_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
			sc301IoT_attr.mipi.index = 0;
			break;
		case TISP_SENSOR_VI_MIPI_CSI1:
			sc301IoT_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
			sc301IoT_attr.mipi.index = 1;
			break;
		case TISP_SENSOR_VI_DVP:
			sc301IoT_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
			break;
		default:
			ISP_ERROR("this interface is not supported yet!!!\n");
	}

	switch (info->mclk) {
		case TISP_SENSOR_MCLK0:
			sclka = private_devm_clk_get(&client->dev, "mux_cim0");
			sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim0");
			set_sensor_mclk_function(0);
			break;
		case TISP_SENSOR_MCLK1:
			sclka = private_devm_clk_get(&client->dev, "mux_cim1");
			sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim1");
			set_sensor_mclk_function(1);
			break;
		case TISP_SENSOR_MCLK2:
			sclka = private_devm_clk_get(&client->dev, "mux_cim2");
			sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim2");
			set_sensor_mclk_function(2);
			break;
		default:
			ISP_ERROR("Have no this MCLK Source!!!\n");
	}

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

static int sc301IoT_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"sc301IoT_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(50);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(35);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(35);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"sc301IoT_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(50);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = sc301IoT_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an sc301IoT chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("sc301IoT chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "sc301IoT", sizeof("sc301IoT"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int sc301IoT_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	int ret = -1;
	unsigned char val = 0x0;

	ret += sc301IoT_read(sd, 0x3221, &val);

	if(enable & 0x2)
		val |= 0x60;
	else
		val &= 0x9f;

	ret += sc301IoT_write(sd, 0x3221, val);

	return ret;
}

static int sc301IoT_set_wdr(struct tx_isp_subdev *sd, int wdr_en)
{
	int ret = 0;

	return 0;
	private_gpio_direction_output(reset_gpio, 1);
	private_msleep(1);
	private_gpio_direction_output(reset_gpio, 0);
	private_msleep(1);
	private_gpio_direction_output(reset_gpio, 1);
	private_msleep(1);

	ret += sc301IoT_write_array(sd, wsize->regs);
	ret += sc301IoT_write_array(sd, sc301IoT_stream_on);

	return 0;
}

static int sc301IoT_set_wdr_stop(struct tx_isp_subdev *sd, int wdr_en)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);

	return 0;
	/* soft reset */
	ret = sc301IoT_write(sd, 0x0103, 0x01);

	if( wdr_en == 1){
		sc301IoT_attr.data_type = TX_SENSOR_DATA_TYPE_WDR_DOL;
		memcpy(&sc301IoT_attr.mipi, &sc301IoT_mipi_dol, sizeof(sc301IoT_mipi_dol));
		wsize = &sc301IoT_win_sizes[1];
		sc301IoT_attr.wdr_cache = wdr_bufsize;
		sc301IoT_attr.total_width = 2250;
		sc301IoT_attr.total_height = 3200;
		sc301IoT_attr.one_line_expr_in_us = 28;
		sc301IoT_attr.integration_time_limit = 3000;
		sc301IoT_attr.max_integration_time_native = 3000;
		sc301IoT_attr.max_integration_time = 3000;
		sc301IoT_attr.max_integration_time_short = 200;
		sc301IoT_attr.min_integration_time = 1;
		sc301IoT_attr.min_integration_time_short = 1;
		ISP_WARNING("-----------------------------> switch wdr is ok <-----------------------\n");
	}else if(wdr_en == 0){
		memcpy(&sc301IoT_attr.mipi, &sc301IoT_mipi_linear, sizeof(sc301IoT_mipi_linear));
		sc301IoT_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		wsize = &sc301IoT_win_sizes[0];
		sc301IoT_attr.one_line_expr_in_us = 21;
		sc301IoT_attr.total_width = 2250;
		sc301IoT_attr.total_height = 1920;
		sc301IoT_attr.max_integration_time_native = 1920 - 4;
		sc301IoT_attr.integration_time_limit = 1920 - 4;
		sc301IoT_attr.max_integration_time = 1920 - 4;
		ISP_WARNING("-----------------------------> switch linear is ok <-----------------------\n");
	}else{
		ISP_ERROR("Can not support this data type!!!");
		return -1;
	}

	sensor_set_attr(sd, wsize);
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	return ret;
}

static int sc301IoT_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
{
	long ret = 0;
	struct tx_isp_sensor_value *sensor_val = arg;
	struct tx_isp_initarg *init = arg;

	if(IS_ERR_OR_NULL(sd)){
		ISP_ERROR("[%d]The pointer is invalid!\n", __LINE__);
		return -EINVAL;
	}
	switch(cmd){
/*
	case TX_ISP_EVENT_SENSOR_EXPO:
		if(arg)
			ret = sc301IoT_set_expo(sd, sensor_val->value);
		break;
*/
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			ret = sc301IoT_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME_SHORT:
		if(arg)
			ret = sc301IoT_set_integration_time_short(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = sc301IoT_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN_SHORT:
		if(arg)
			ret = sc301IoT_set_analog_gain_short(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = sc301IoT_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = sc301IoT_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		ret = sc301IoT_write_array(sd, sc301IoT_stream_off);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		ret = sc301IoT_write_array(sd, sc301IoT_stream_on);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = sc301IoT_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = sc301IoT_set_vflip(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_LOGIC:
		if(arg)
			ret = sc301IoT_set_logic(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_WDR:
		if(arg)
			ret = sc301IoT_set_wdr(sd, init->enable);
		break;
	case TX_ISP_EVENT_SENSOR_WDR_STOP:
		if(arg)
			ret = sc301IoT_set_wdr_stop(sd, init->enable);
		break;
	default:
		break;
	}

	return ret;
}

static int sc301IoT_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = sc301IoT_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int sc301IoT_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	sc301IoT_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops sc301IoT_core_ops = {
	.g_chip_ident = sc301IoT_g_chip_ident,
	.reset = sc301IoT_reset,
	.init = sc301IoT_init,
	/*.ioctl = sc301IoT_ops_ioctl,*/
	.g_register = sc301IoT_g_register,
	.s_register = sc301IoT_s_register,
};

static struct tx_isp_subdev_video_ops sc301IoT_video_ops = {
	.s_stream = sc301IoT_s_stream,
};

static struct tx_isp_subdev_sensor_ops	sc301IoT_sensor_ops = {
	.ioctl	= sc301IoT_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops sc301IoT_ops = {
	.core = &sc301IoT_core_ops,
	.video = &sc301IoT_video_ops,
	.sensor = &sc301IoT_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "sc301IoT",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int sc301IoT_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

	sc301IoT_attr.expo_fs = 1;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->dev = &client->dev;
	sensor->video.shvflip = shvflip;
	sensor->video.attr = &sc301IoT_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &sc301IoT_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	ISP_WARNING("probe ok ------->sc301IoT\n");

	return 0;
}

static int sc301IoT_remove(struct i2c_client *client)
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

static const struct i2c_device_id sc301IoT_id[] = {
	{ "sc301IoT", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sc301IoT_id);

static struct i2c_driver sc301IoT_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "sc301IoT",
	},
	.probe		= sc301IoT_probe,
	.remove		= sc301IoT_remove,
	.id_table	= sc301IoT_id,
};

static __init int init_sc301IoT(void)
{
	return private_i2c_add_driver(&sc301IoT_driver);
}

static __exit void exit_sc301IoT(void)
{
	private_i2c_del_driver(&sc301IoT_driver);
}

module_init(init_sc301IoT);
module_exit(exit_sc301IoT);

MODULE_DESCRIPTION("A low-level driver for SmartSens sc301IoT sensors");
MODULE_LICENSE("GPL");
