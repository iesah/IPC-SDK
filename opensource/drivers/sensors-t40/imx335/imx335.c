/*
 * imx335.c
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

#define IMX335_CHIP_ID_H	(0x38)
#define IMX335_CHIP_ID_L	(0x0a)
#define IMX335_REG_END		0xffff
#define IMX335_REG_DELAY	0xfffe
#define IMX335_SUPPORT_SCLK     (74250000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define AGAIN_MAX_DB 0x64
#define DGAIN_MAX_DB 0x64
#define LOG2_GAIN_SHIFT 16
#define SENSOR_VERSION	"H20220531a"

/* t40 sensor hvflip function only takes effect when shvflip == 1 */
static int shvflip = 1;
module_param(shvflip, int, S_IRUGO);
MODULE_PARM_DESC(shvflip, "Sensor HV Flip Enable interface");

static int reset_gpio = GPIO_PC(28);
static int pwdn_gpio = -1;

struct regval_list {
	uint16_t reg_num;
	unsigned char value;
};

/*
 * the part of driver maybe modify about different sensor and different board.
 */

struct tx_isp_sensor_attribute imx335_attr;

unsigned int imx335_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	uint16_t again=(isp_gain*20)>>LOG2_GAIN_SHIFT;
	// Limit Max gain
	if(again>AGAIN_MAX_DB+DGAIN_MAX_DB) again=AGAIN_MAX_DB+DGAIN_MAX_DB;

	/* p_ctx->again=again; */
	*sensor_again=again;
	isp_gain= (((int32_t)again)<<LOG2_GAIN_SHIFT)/20;

	return isp_gain;
}

unsigned int imx335_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_sensor_attribute imx335_attr={
	.name = "imx335",
	.chip_id = 0x080,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x1a,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.mode = SENSOR_MIPI_SONY_MODE,
		.clk = 594,
		.lans = 2,
		.settle_time_apative_en = 0,
		.image_twidth = 2616,
		.image_theight = 1964,
		.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,
		.mipi_sc.hcrop_diff_en = 0,
		.mipi_sc.mipi_vcomp_en = 0,
		.mipi_sc.mipi_hcomp_en = 0,
		.mipi_sc.mipi_crop_start0x = 12,
		.mipi_sc.mipi_crop_start0y = 33,
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
	},

	.max_again = 458752,
	.max_again_short = 458752,
	.max_dgain = 0,
	.min_integration_time = 1,
	.min_integration_time_native = 1,
	.max_integration_time_native = 4116,
	.min_integration_time_short = 1,
	.max_integration_time_short = 10,
	.integration_time_limit = 4116,
	.total_width = 1200,
	.total_height = 4125,
	.max_integration_time = 4116,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = imx335_alloc_again,
	.sensor_ctrl.alloc_dgain = imx335_alloc_dgain,
};

static struct regval_list imx335_init_regs_2592_1944_15fps_mipi[] = {
	{0x3000, 0x01},
	{0x3001, 0x00},
	{0x3002, 0x01},
	{0x3004, 0x00},
	{0x300C, 0x5B},
	{0x300D, 0x40},
	{0x3018, 0x00},
	{0x302C, 0x30},
	{0x302D, 0x00},
	{0x302E, 0x38},
	{0x302F, 0x0A},
	{0x3030, 0x1d},
	{0x3031, 0x10},
	{0x3032, 0x00},
	{0x3034, 0xb0},
	{0x3035, 0x04},/* 20fps 0x384 */
	{0x3048, 0x00},
	{0x3049, 0x00},
	{0x304A, 0x03},
	{0x304B, 0x01},
	{0x304C, 0x14},
	{0x304E, 0x00},
	{0x304F, 0x00},/* vflip */
	{0x3050, 0x00},
	{0x3056, 0xAC},
	{0x3057, 0x07},
	{0x3058, 0x09},
	{0x3059, 0x00},
	{0x305A, 0x00},
	{0x305C, 0x12},
	{0x305D, 0x00},
	{0x305E, 0x00},
	{0x3060, 0xE8},
	{0x3061, 0x00},
	{0x3062, 0x00},
	{0x3064, 0x09},
	{0x3065, 0x00},
	{0x3066, 0x00},
	{0x3068, 0xCE},
	{0x3069, 0x00},
	{0x306A, 0x00},
	{0x306C, 0x68},
	{0x306D, 0x06},
	{0x306E, 0x00},
	{0x3072, 0x28},
	{0x3073, 0x00},
	{0x3074, 0xB0},
	{0x3075, 0x00},
	{0x3076, 0x58},
	{0x3077, 0x0F},
	{0x3078, 0x01},
	{0x3079, 0x02},
	{0x307A, 0xFF},
	{0x307B, 0x02},
	{0x307C, 0x00},
	{0x307D, 0x00},
	{0x307E, 0x00},
	{0x307F, 0x00},
	{0x3080, 0x01},
	{0x3081, 0x02},
	{0x3082, 0xFF},
	{0x3083, 0x02},
	{0x3084, 0x00},
	{0x3085, 0x00},
	{0x3086, 0x00},
	{0x3087, 0x00},
	{0x30A4, 0x33},
	{0x30A8, 0x10},
	{0x30A9, 0x04},
	{0x30AC, 0x00},
	{0x30AD, 0x00},
	{0x30B0, 0x10},
	{0x30B1, 0x08},
	{0x30B4, 0x00},
	{0x30B5, 0x00},
	{0x30B6, 0x00},
	{0x30B7, 0x00},
	{0x30C6, 0x00},
	{0x30C7, 0x00},
	{0x30CE, 0x00},
	{0x30CF, 0x00},
	{0x30D8, 0x4C},
	{0x30D9, 0x10},
	{0x30E8, 0x00},
	{0x30E9, 0x00},
	{0x30EA, 0x00},
	{0x30EB, 0x00},
	{0x30EC, 0x00},
	{0x30ED, 0x00},
	{0x30EE, 0x00},
	{0x30EF, 0x00},
	{0x3112, 0x08},
	{0x3113, 0x00},
	{0x3116, 0x08},
	{0x3117, 0x00},
	{0x314C, 0x80},
	{0x314D, 0x00},
	{0x315A, 0x06},
	{0x3167, 0x01},
	{0x3168, 0x68},
	{0x316A, 0x7E},
	{0x3199, 0x00},
	{0x319D, 0x00},
	{0x319E, 0x03},
	{0x31A0, 0x2A},
	{0x31A1, 0x00},
	{0x31D4, 0x00},
	{0x31D5, 0x00},
	{0x3288, 0x21},
	{0x328A, 0x02},
	{0x3300, 0x00},
	{0x3302, 0x32},
	{0x3303, 0x00},
	{0x3414, 0x05},
	{0x3416, 0x18},
	{0x341C, 0xff},
	{0x341D, 0x01},
	{0x3648, 0x01},
	{0x364A, 0x04},
	{0x364C, 0x04},
	{0x3678, 0x01},
	{0x367C, 0x31},
	{0x367E, 0x31},
	{0x3706, 0x10},
	{0x3708, 0x03},
	{0x3714, 0x02},
	{0x3715, 0x02},
	{0x3716, 0x01},
	{0x3717, 0x03},
	{0x371C, 0x3D},
	{0x371D, 0x3F},
	{0x372C, 0x00},
	{0x372D, 0x00},
	{0x372E, 0x46},
	{0x372F, 0x00},
	{0x3730, 0x89},
	{0x3731, 0x00},
	{0x3732, 0x08},
	{0x3733, 0x01},
	{0x3734, 0xFE},
	{0x3735, 0x05},
	{0x3740, 0x02},
	{0x375D, 0x00},
	{0x375E, 0x00},
	{0x375F, 0x11},
	{0x3760, 0x01},
	{0x3768, 0x1B},
	{0x3769, 0x1B},
	{0x376A, 0x1B},
	{0x376B, 0x1B},
	{0x376C, 0x1A},
	{0x376D, 0x17},
	{0x376E, 0x0F},
	{0x3776, 0x00},
	{0x3777, 0x00},
	{0x3778, 0x46},
	{0x3779, 0x00},
	{0x377A, 0x89},
	{0x377B, 0x00},
	{0x377C, 0x08},
	{0x377D, 0x01},
	{0x377E, 0x23},
	{0x377F, 0x02},
	{0x3780, 0xD9},
	{0x3781, 0x03},
	{0x3782, 0xF5},
	{0x3783, 0x06},
	{0x3784, 0xA5},
	{0x3788, 0x0F},
	{0x378A, 0xD9},
	{0x378B, 0x03},
	{0x378C, 0xEB},
	{0x378D, 0x05},
	{0x378E, 0x87},
	{0x378F, 0x06},
	{0x3790, 0xF5},
	{0x3792, 0x43},
	{0x3794, 0x7A},
	{0x3796, 0xA1},
	{0x37B0, 0x36},
	{0x3A01, 0x01},
	{0x3A04, 0x48},
	{0x3A05, 0x09},
	{0x3A18, 0x67},
	{0x3A19, 0x00},
	{0x3A1A, 0x27},
	{0x3A1B, 0x00},
	{0x3A1C, 0x27},
	{0x3A1D, 0x00},
	{0x3A1E, 0xB7},
	{0x3A1F, 0x00},
	{0x3A20, 0x2F},
	{0x3A21, 0x00},
	{0x3A22, 0x4F},
	{0x3A23, 0x00},
	{0x3A24, 0x2F},
	{0x3A25, 0x00},
	{0x3A26, 0x47},
	{0x3A27, 0x00},
	{0x3A28, 0x27},
	{0x3A29, 0x00},

	/*pattern generator*/
#ifdef IMX335_TESTPATTERN
	{0x3148, 0x10},
	{0x3280, 0x00},
	{0x329c, 0x01},
	{0x329e, 0x0b},
	{0x32a0, 0x11},/*tpg colorwidth*/
#endif
	{0x3002, 0x00},
	{0x3000, 0x00},

	{IMX335_REG_END, 0x00},/* END MARKER */
};

static struct regval_list imx335_init_regs_2592_1944_30fps_mipi[] = {
	{0x3000, 0x01},
	{0x3001, 0x00},
	{0x3002, 0x01},
	{0x3004, 0x00},
	{0x300c, 0x5b},
	{0x300d, 0x40},
	{0x3018, 0x00},
	{0x302c, 0x30},
	{0x302d, 0x00},
	{0x302e, 0x38},
	{0x302f, 0x0a},
	{0x3030, 0x18},
	{0x3031, 0x15},
	{0x3032, 0x00},
	{0x3034, 0x26},
	{0x3035, 0x02},
	{0x3050, 0x00},
	{0x315a, 0x02},
	{0x316a, 0x7e},
	{0x319d, 0x00},
	{0x31a1, 0x00},
	{0x3288, 0x21},
	{0x328a, 0x02},
	{0x3414, 0x05},
	{0x3416, 0x18},
	{0x341c, 0xff},
	{0x341d, 0x01},
	{0x3648, 0x01},
	{0x364a, 0x04},
	{0x364c, 0x04},
	{0x3678, 0x01},
	{0x367c, 0x31},
	{0x367e, 0x31},
	{0x3706, 0x10},
	{0x3708, 0x03},
	{0x3714, 0x02},
	{0x3715, 0x02},
	{0x3716, 0x01},
	{0x3717, 0x03},
	{0x371c, 0x3d},
	{0x371d, 0x3f},
	{0x372c, 0x00},
	{0x372d, 0x00},
	{0x372e, 0x46},
	{0x372f, 0x00},
	{0x3730, 0x89},
	{0x3731, 0x00},
	{0x3732, 0x08},
	{0x3733, 0x01},
	{0x3734, 0xfe},
	{0x3735, 0x05},
	{0x3740, 0x02},
	{0x375d, 0x00},
	{0x375e, 0x00},
	{0x375f, 0x11},
	{0x3760, 0x01},
	{0x3768, 0x1b},
	{0x3769, 0x1b},
	{0x376a, 0x1b},
	{0x376b, 0x1b},
	{0x376c, 0x1a},
	{0x376d, 0x17},
	{0x376e, 0x0f},
	{0x3776, 0x00},
	{0x3777, 0x00},
	{0x3778, 0x46},
	{0x3779, 0x00},
	{0x377a, 0x89},
	{0x377b, 0x00},
	{0x377c, 0x08},
	{0x377d, 0x01},
	{0x377e, 0x23},
	{0x377f, 0x02},
	{0x3780, 0xd9},
	{0x3781, 0x03},
	{0x3782, 0xf5},
	{0x3783, 0x06},
	{0x3784, 0xa5},
	{0x3788, 0x0f},
	{0x378a, 0xd9},
	{0x378b, 0x03},
	{0x378c, 0xeb},
	{0x378d, 0x05},
	{0x378e, 0x87},
	{0x378f, 0x06},
	{0x3790, 0xf5},
	{0x3792, 0x43},
	{0x3794, 0x7a},
	{0x3796, 0xa1},
	{0x3a01, 0x01},
	{0x3002, 0x00},
	{0x3000, 0x00},

	{IMX335_REG_END, 0x00},/* END MARKER */
};

/*
 * the order of the imx335_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting imx335_win_sizes[] = {
	/* 2592*1944 */
	{
		.width		= 2592,
		.height		= 1944,
		.fps		= 15 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SRGGB12_1X12,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= imx335_init_regs_2592_1944_15fps_mipi,
	},//[0]
	/* 2592*1944 */
	{
		.width		= 2592,
		.height		= 1944,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SRGGB10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= imx335_init_regs_2592_1944_30fps_mipi,
	}//[1]

};
static struct tx_isp_sensor_win_setting *wsize = &imx335_win_sizes[0];

/*
 * the part of driver was fixed.
 */

static struct regval_list imx335_stream_on_mipi[] = {
	{0x3000, 0x00},
	{IMX335_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list imx335_stream_off_mipi[] = {
	{0x3000, 0x01},
	{IMX335_REG_END, 0x00},	/* END MARKER */
};

int imx335_read(struct tx_isp_subdev *sd, uint16_t reg,	unsigned char *value)
{
	int ret;
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	uint8_t buf[2] = {(reg>>8)&0xff, reg&0xff};
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

	ret = private_i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;

	return ret;
}

int imx335_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	uint8_t buf[3] = {(reg>>8)&0xff, reg&0xff, value};
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
static int imx335_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != IMX335_REG_END) {
		if (vals->reg_num == IMX335_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = imx335_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		pr_debug("vals->reg_num:0x%02x, vals->value:0x%02x\n",vals->reg_num, val);
		vals++;
	}
	return 0;
}
#endif

static int imx335_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != IMX335_REG_END) {
		if (vals->reg_num == IMX335_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = imx335_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int imx335_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int imx335_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	unsigned char v;
	int ret;

	ret = imx335_read(sd, 0x302e, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != IMX335_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = imx335_read(sd, 0x302f, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != IMX335_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

#if 0
static int imx335_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	unsigned short shr0 = 0;
	unsigned short vmax = 0;

	vmax = imx335_attr.total_height;
	shr0 = vmax - value;
	ret = imx335_write(sd, 0x3058, (unsigned char)(shr0 & 0xff));
	ret += imx335_write(sd, 0x3059, (unsigned char)((shr0 >> 8) & 0xff));
	ret += imx335_write(sd, 0x305a, (unsigned char)((shr0 >> 16) & 0x0f));
	if (0 != ret) {
		ISP_ERROR("err: imx335_write err\n");
		return ret;
	}

	return 0;
}
#endif

#if 0
static int imx335_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	ret = imx335_write(sd, 0x30e8, (unsigned char)(value & 0xff));
	ret += imx335_write(sd, 0x30e9, (unsigned char)((value >> 8) & 0x07));
	if (ret < 0)
		return ret;

	return 0;
}
#endif

static int imx335_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	unsigned short shr0 = 0;
	unsigned short vmax = 0;
	int it = (value & 0xffff);
	int again = (value & 0xffff0000) >> 16;

	vmax = imx335_attr.total_height;
	shr0 = vmax - it;
	ret = imx335_write(sd, 0x3058, (unsigned char)(shr0 & 0xff));
	ret += imx335_write(sd, 0x3059, (unsigned char)((shr0 >> 8) & 0xff));
	ret += imx335_write(sd, 0x305a, (unsigned char)((shr0 >> 16) & 0x0f));

	ret += imx335_write(sd, 0x30e8, (unsigned char)(again & 0xff));
	ret += imx335_write(sd, 0x30e9, (unsigned char)(((again >> 8) & 0xff)));
	if (ret < 0)
		return ret;

	return 0;
}

static int imx335_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int imx335_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int imx335_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if(!init->enable){
		sensor->video.state = TX_ISP_MODULE_DEINIT;
		return ISP_SUCCESS;
	} else {
		sensor->video.mbus.width = wsize->width;
		sensor->video.mbus.height = wsize->height;
		sensor->video.mbus.code = wsize->mbus_code;
		sensor->video.mbus.field = TISP_FIELD_NONE;
		sensor->video.mbus.colorspace = wsize->colorspace;
		sensor->video.fps = wsize->fps;
		sensor->video.state = TX_ISP_MODULE_DEINIT;

		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
		sensor->priv = wsize;
	}

	return 0;
}

static int imx335_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if (init->enable) {
		if(sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = imx335_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if(sensor->video.state == TX_ISP_MODULE_INIT){
			ret = imx335_write_array(sd, imx335_stream_on_mipi);
			sensor->video.state = TX_ISP_MODULE_RUNNING;
			pr_debug("imx327 stream on\n");
		}
	} else {
		ret = imx335_write_array(sd, imx335_stream_off_mipi);
		sensor->video.state = TX_ISP_MODULE_INIT;
		pr_debug("imx327 stream off\n");
	}

	return ret;
}

static int imx335_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	unsigned int wpclk = 0;
	unsigned short vts = 0;
	unsigned short hts=0;
	unsigned int max_fps = 0;
	unsigned char tmp;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	switch(info->default_boot){
	case 0:
		max_fps = TX_SENSOR_MAX_FPS_15;
		wpclk = IMX335_SUPPORT_SCLK;
		break;
	case 1:
		max_fps = TX_SENSOR_MAX_FPS_30;
		wpclk = IMX335_SUPPORT_SCLK;
		break;
	default:
		ISP_WARNING("%s %d :not support this boot sel yet\n",__func__,__LINE__);
		break;
	}

	/* the format of fps is 16/16. for example 25 << 16 | 2, the value is 25/2 fps. */
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (max_fps << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)){
		ISP_ERROR("warn: fps(%x) no in range\n", fps);
		return -1;
	}
	ret += imx335_read(sd, 0x3035, &tmp);
	hts = tmp & 0x0f;
	ret += imx335_read(sd, 0x3034, &tmp);
	if(ret < 0)
		return -1;
	hts = (hts << 8) + tmp;

	vts = wpclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);
	ret += imx335_write(sd, 0x3001, 0x01);
	ret = imx335_write(sd, 0x3032, (unsigned char)((vts & 0xf0000) >> 16));
	ret += imx335_write(sd, 0x3031, (unsigned char)((vts & 0xff00) >> 8));
	ret += imx335_write(sd, 0x3030, (unsigned char)(vts & 0xff));
	ret += imx335_write(sd, 0x3001, 0x00);
	if(ret < 0)
		return -1;

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 9;
	sensor->video.attr->integration_time_limit = vts - 9;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 9;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return 0;
}

static int imx335_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_win_setting *wsize = NULL;
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

static int imx335_set_hvflip(struct tx_isp_subdev *sd, int enable)
{
	int ret = 0;
	uint8_t val_h;
	uint8_t val_v;
	uint8_t reg_3081 = 0x02;
	uint8_t reg_3083 = 0x02;

	/*
	 * 2'b01:mirror,2'b10:filp
	 * 0x3081 0x3082 must be changed as blow in all-pixel scan mode
	 */
	ret = imx335_read(sd, 0x304e, &val_h);
	ret += imx335_read(sd, 0x304f, &val_v);
	switch(enable) {
	case 0://normal
		val_h &= 0xfc;
		val_v &= 0xfc;
		reg_3081 = 0x02;
		reg_3083 = 0x02;
		break;
	case 1://sensor mirror
		val_h |= 0x01;
		val_v &= 0xfc;
		reg_3081 = 0x02;
		reg_3083 = 0x02;
		break;
	case 2://sensor flip
		val_h &= 0xfc;
		val_v |= 0x01;
		reg_3081 = 0xfe;
		reg_3083 = 0xfe;
		break;
	case 3://sensor mirror&flip
		val_h |= 0x01;
		val_v |= 0x01;
		reg_3081 = 0xfe;
		reg_3083 = 0xfe;
		break;
	}
	ret = imx335_write(sd, 0x304e, val_h);
	ret += imx335_write(sd, 0x304f, val_v);
	ret += imx335_write(sd, 0x3081, reg_3081);
	ret += imx335_write(sd, 0x3083, reg_3083);

	return ret;
}

struct clk *sclka;
static int sensor_attr_check(struct tx_isp_subdev *sd)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned long rate;
	int ret = 0;

	switch(info->default_boot){
	case 0:
		wsize = &imx335_win_sizes[0];
		imx335_attr.mipi.clk = 594;
		imx335_attr.total_width = 2200;
		imx335_attr.total_height = 4125;
		imx335_attr.max_integration_time = 4125 - 9;
		imx335_attr.integration_time_limit = 4125 - 9;
		imx335_attr.max_integration_time_native = 4125 - 9;
		break;
	case 1:
		wsize = &imx335_win_sizes[1];
		imx335_attr.mipi.clk = 1188;
		imx335_attr.total_width = 550;
		imx335_attr.total_height = 5400;
		imx335_attr.max_integration_time = 5400 - 9;
		imx335_attr.integration_time_limit = 5400 - 9;
		imx335_attr.max_integration_time_native = 5400 - 9;
		break;
	default:
		ISP_ERROR("Have no this MCLK Source!!!\n");
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		imx335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		imx335_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		imx335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		imx335_attr.mipi.index = 1;
		break;
	case TISP_SENSOR_VI_DVP:
		imx335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
		break;
	default:
		ISP_ERROR("Have no this MCLK Source!!!\n");
	}

	switch(info->mclk){
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

	rate = private_clk_get_rate(sensor->mclk);
	if (IS_ERR(sensor->mclk)) {
		ISP_ERROR("Cannot get sensor input clock cgu_cim\n");
		goto err_get_mclk;
	}
	if (((rate / 1000) % 37125) != 0) {
		ret = clk_set_parent(sclka, clk_get(NULL, "epll"));
		if(ret != 0)
			pr_err("%s %d set parent clk to epll err!\n",__func__,__LINE__);
		sclka = private_devm_clk_get(&client->dev, "epll");
		if (IS_ERR(sclka)) {
			pr_err("get sclka failed\n");
		} else {
			rate = private_clk_get_rate(sclka);
			if (((rate / 1000) % 37125) != 0) {
				private_clk_set_rate(sclka, 891000000);
			}
		}
	}

	private_clk_set_rate(sensor->mclk, 37125000);
	private_clk_prepare_enable(sensor->mclk);

	reset_gpio = info->rst_gpio;
	pwdn_gpio = info->pwdn_gpio;

	return 0;

err_get_mclk:
	return -1;
}

static int imx335_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"imx335_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(100);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(100);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"imx335_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = imx335_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an imx335 chip.\n",
				client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("imx335 chip found @ 0x%02x (%s) version %s\n", client->addr, client->adapter->name,SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "imx335", sizeof("imx335"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int imx335_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = imx335_set_expo(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		//if(arg)
		//	ret = imx335_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME_SHORT:
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		//if(arg)
		//	ret = imx335_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN_SHORT:
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = imx335_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = imx335_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = imx335_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		if(arg)
			ret = imx335_write_array(sd, imx335_stream_off_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		if(arg)
			ret = imx335_write_array(sd, imx335_stream_on_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = imx335_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = imx335_set_hvflip(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return 0;
}

static int imx335_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = imx335_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int imx335_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	imx335_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops imx335_core_ops = {
	.g_chip_ident = imx335_g_chip_ident,
	.reset = imx335_reset,
	.init = imx335_init,
	.g_register = imx335_g_register,
	.s_register = imx335_s_register,
};

static struct tx_isp_subdev_video_ops imx335_video_ops = {
	.s_stream = imx335_s_stream,
};

static struct tx_isp_subdev_sensor_ops	imx335_sensor_ops = {
	.ioctl	= imx335_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops imx335_ops = {
	.core = &imx335_core_ops,
	.video = &imx335_video_ops,
	.sensor = &imx335_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "imx335",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};


static int imx335_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
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

	sd = &sensor->sd;
	video = &sensor->video;
	sensor->dev = &client->dev;
	imx335_attr.expo_fs = 1;
	sensor->video.shvflip = shvflip;
	sensor->video.attr = &imx335_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &imx335_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->imx335\n");

	return 0;
}

static int imx335_remove(struct i2c_client *client)
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

static const struct i2c_device_id imx335_id[] = {
	{ "imx335", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, imx335_id);

static struct i2c_driver imx335_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "imx335",
	},
	.probe		= imx335_probe,
	.remove		= imx335_remove,
	.id_table	= imx335_id,
};

static __init int init_imx335(void)
{
	return private_i2c_add_driver(&imx335_driver);
}

static __exit void exit_imx335(void)
{
	private_i2c_del_driver(&imx335_driver);
}

module_init(init_imx335);
module_exit(exit_imx335);

MODULE_DESCRIPTION("A low-level driver for Sony imx335 sensor");
MODULE_LICENSE("GPL");
