/*
 * jxh63.c
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
#include <txx-funcs.h>

#define JXH63_CHIP_ID_H	(0x0a)
#define JXH63_CHIP_ID_L	(0x63)
#define JXH63_REG_END		0xff
#define JXH63_REG_DELAY		0xfe
#define JXH63_SUPPORT_PCLK (36*1000*1000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define DRIVE_CAPABILITY_1
#define SENSOR_VERSION	"H20210412a"

static int reset_gpio = GPIO_PC(28);
static int pwdn_gpio = -1;
static int sensor_max_fps = TX_SENSOR_MAX_FPS_30;

struct regval_list {
	unsigned char reg_num;
	unsigned char value;
};

/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
	unsigned int value;
	unsigned int gain;
};

struct again_lut jxh63_again_lut[] = {
	{0x0, 0},
	{0x1, 5731},
	{0x2, 11136},
	{0x3, 16248},
	{0x4, 21097},
	{0x5, 25710},
	{0x6, 30109},
	{0x7, 34312},
	{0x8, 38336},
	{0x9, 42195},
	{0xa, 45904},
	{0xb, 49472},
	{0xc, 52910},
	{0xd, 56228},
	{0xe, 59433},
	{0xf, 62534},
	{0x10, 65536},
	{0x11, 71267},
	{0x12, 76672},
	{0x13, 81784},
	{0x14, 86633},
	{0x15, 91246},
	{0x16, 95645},
	{0x17, 99848},
	{0x18, 103872},
	{0x19, 107731},
	{0x1a, 111440},
	{0x1b, 115008},
	{0x1c, 118446},
	{0x1d, 121764},
	{0x1e, 124969},
	{0x1f, 128070},
	{0x20, 131072},
	{0x21, 136803},
	{0x22, 142208},
	{0x23, 147320},
	{0x24, 152169},
	{0x25, 156782},
	{0x26, 161181},
	{0x27, 165384},
	{0x28, 169408},
	{0x29, 173267},
	{0x2a, 176976},
	{0x2b, 180544},
	{0x2c, 183982},
	{0x2d, 187300},
	{0x2e, 190505},
	{0x2f, 193606},
	{0x30, 196608},
	{0x31, 202339},
	{0x32, 207744},
	{0x33, 212856},
	{0x34, 217705},
	{0x35, 222318},
	{0x36, 226717},
	{0x37, 230920},
	{0x38, 234944},
	{0x39, 238803},
	{0x3a, 242512},
	{0x3b, 246080},
	{0x3c, 249518},
	{0x3d, 252836},
	{0x3e, 256041},
	{0x3f, 259142},
	{0x40, 262144},
	{0x41, 267875},
	{0x42, 273280},
	{0x43, 278392},
	{0x44, 283241},
	{0x45, 287854},
	{0x46, 292253},
	{0x47, 296456},
	{0x48, 300480},
	{0x49, 304339},
	{0x4a, 308048},
	{0x4b, 311616},
	{0x4c, 315054},
	{0x4d, 318372},
	{0x4e, 321577},
	{0x4f, 324678},
};

struct tx_isp_sensor_attribute jxh63_attr;

unsigned int jxh63_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = jxh63_again_lut;
	while(lut->gain <= jxh63_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut->value;
			return 0;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == jxh63_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int jxh63_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus jxh63_mipi={
	.mode = SENSOR_MIPI_OTHER_MODE,
	.clk = 400,
	.lans = 1,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
	.mipi_sc.hcrop_diff_en = 0,
	.mipi_sc.mipi_vcomp_en = 0,
	.mipi_sc.mipi_hcomp_en = 0,
	.image_twidth = 1280,
	.image_theight = 720,
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

struct tx_isp_sensor_attribute jxh63_attr={
	.name = "jxh63s1",
	.chip_id = 0x0a63,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_8BITS,
	.cbus_device = 0x40,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP,
	.dvp = {
		.mode = SENSOR_DVP_HREF_MODE,
		.blanking = {
			.vblanking = 0,
			.hblanking = 0,
		},
	},
	.expo_fs = 1,
	.max_again = 324678,
	.max_dgain = 0,
	.total_width = 1600,
	.total_height = 900,
	.min_integration_time = 1,
	.max_integration_time = 899,
	.min_integration_time_native = 1,
	.max_integration_time_native = 899,
	.integration_time_limit = 899,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = jxh63_alloc_again,
	.sensor_ctrl.alloc_dgain = jxh63_alloc_dgain,
	.one_line_expr_in_us = 44,
};

static struct regval_list jxh63_init_regs_1280_720_30fps_mipi[] = {
	{0x12, 0x40},
	{0x48, 0x85},
	{0x48, 0x05},
	{0x0E, 0x11},
	{0x0F, 0x14},
	{0x10, 0x1E},
	{0x11, 0x80},
	{0x0D, 0xD0},
	{0x5F, 0x41},
	{0x60, 0x20},
	{0x58, 0x18},
	{0x57, 0x60},
	{0x20, 0x20},//800
	{0x21, 0x03},
	{0x22, 0x84},//750@30fps 900@25fps
	{0x23, 0x03},
	{0x24, 0x80},
	{0x25, 0xD0},
	{0x26, 0x22},
	{0x27, 0xA2},
	{0x28, 0x13},
	{0x29, 0x02},
	{0x2A, 0x96},
	{0x2B, 0x12},
	{0x2C, 0x00},
	{0x2D, 0x00},
	{0x2E, 0xB9},
	{0x2F, 0x40},
	{0x41, 0x84},
	{0x42, 0x32},
	{0x46, 0x00},
	{0x47, 0x42},
	{0x76, 0x40},
	{0x77, 0x06},
	{0x80, 0x01},
	{0xAF, 0x22},
	{0xAB, 0x00},
	{0x9B, 0x83},
	{0x1D, 0x00},
	{0x1E, 0x04},
	{0x6C, 0x50},
	{0x6E, 0x2C},
   	{0x70, 0x6C},
	{0x71, 0x6D},
	{0x72, 0x69},
	{0x73, 0x46},
	{0x74, 0x02},
	{0x78, 0x8D},
	{0x89, 0x01},
	{0x6B, 0x20},
	{0x86, 0x40},
	{0x30, 0x86},
	{0x31, 0x04},
	{0x32, 0x19},
	{0x33, 0x10},
	{0x34, 0x2A},
	{0x35, 0x2A},
	{0x3A, 0xA0},
	{0x3B, 0x00},
	{0x3C, 0x38},
	{0x3D, 0x41},
	{0x3E, 0xE0},
	{0x56, 0x12},
	{0x59, 0x46},
	{0x5A, 0x02},
	{0x85, 0x1E},
	{0x8A, 0x04},
	{0x9C, 0x61},
	{0x5B, 0xAC},
	{0x5C, 0x61},
	{0x5D, 0xA6},
	{0x5E, 0x14},
	{0x64, 0xE0},
	{0x66, 0x04},
	{0x67, 0x53},
	{0x68, 0x00},
	{0x69, 0x74},
	{0x7A, 0x60},
	{0x8F, 0x91},
	{0xAE, 0x30},
	{0x13, 0x81},
	{0x96, 0x84},
	{0x4A, 0x05},
	{0x7E, 0xCD},
	{0x50, 0x02},
	{0x49, 0x10},
	{0x7B, 0x4A},
	{0x7C, 0x0C},
	{0x7F, 0x56},
	{0x62, 0x21},
	{0x90, 0x00},
	{0x8C, 0xFF},
	{0x8D, 0xC7},
	{0x8E, 0x00},
	{0x8B, 0x01},
	{0x0C, 0x00},
	{0xBB, 0x11},
	{0xA0, 0x10},
	{0x6A, 0x17},
	{0x65, 0x34},
	{0x82, 0x00},
	{0x19, 0x20},
	{0x12, 0x04},
	{0x48, 0x85},
	{0x48, 0x05},
	{JXH63_REG_DELAY,0x10},
	{JXH63_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list jxh63_init_regs_1280_720_30fps_dvp[] = {
	{0x12, 0x44},
	{0x48, 0x85},
	{0x48, 0x05},
	{0x0E, 0x11},
	{0x0F, 0x14},
	{0x10, 0x24},
	{0x11, 0x80},
	{0x0D, 0xD0},
	{0x5F, 0x41},
	{0x60, 0x20},
	{0x58, 0x18},
	{0x57, 0x60},
	{0x20, 0xC0},
	{0x21, 0x03},
	{0x22, 0x84},
	{0x23, 0x03},/*25fps*/
	{0x24, 0x80},
	{0x25, 0xD0},
	{0x26, 0x22},
	{0x27, 0xF0},
	{0x28, 0x15},
	{0x29, 0x02},
	{0x2A, 0xE6},
	{0x2B, 0x12},
	{0x2C, 0x00},
	{0x2D, 0x00},
	{0x2E, 0xBA},
	{0x2F, 0x40},
	{0x41, 0x84},
	{0x42, 0x32},
	{0x46, 0x00},
	{0x47, 0x42},
	{0x76, 0x40},
	{0x77, 0x06},
	{0x80, 0x01},
	{0xAF, 0x22},
	{0x1D, 0xFF},
	{0x1E, 0x1F},
	{0x6C, 0xC0},
	{0x30, 0x86},
	{0x31, 0x04},
	{0x32, 0x19},
	{0x33, 0x10},
	{0x34, 0x2A},
	{0x35, 0x2A},
	{0x3A, 0xA0},
	{0x3B, 0x00},
	{0x3C, 0x38},
	{0x3D, 0x41},
	{0x3E, 0xE0},
	{0x56, 0x12},
	{0x59, 0x46},
	{0x5A, 0x02},
	{0x85, 0x1E},
	{0x8A, 0x04},
	{0x9C, 0x61},
	{0x5B, 0xAC},
	{0x5C, 0x61},
	{0x5D, 0xA6},
	{0x5E, 0x14},
	{0x64, 0xE0},
	{0x66, 0x04},
	{0x67, 0x53},
	{0x68, 0x00},
	{0x69, 0x74},
	{0x7A, 0x60},
	{0x8F, 0x91},
	{0xAE, 0x30},
	{0x13, 0x81},
	{0x96, 0x84},
	{0x4A, 0x05},
	{0x7E, 0xCD},
	{0x50, 0x02},
	{0x49, 0x10},
	{0x7B, 0x4A},
	{0x7C, 0x0C},
	{0x7F, 0x56},
	{0x62, 0x21},
	{0x90, 0x00},
	{0x8C, 0xFF},
	{0x8D, 0xC7},
	{0x8E, 0x00},
	{0x8B, 0x01},
	{0x0C, 0x00},
	{0xBB, 0x11},
	{0xA0, 0x10},
	{0x6A, 0x17},
	{0x65, 0x34},
	{0x82, 0x00},
	{0x19, 0x20},
	{0x12, 0x04},
	{0x48, 0x85},
	{0x48, 0x05},
	{JXH63_REG_DELAY,0x10},
	{JXH63_REG_END, 0x00},	/* END MARKER */
};

/*
 * the order of the jxh63_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting jxh63_win_sizes[] = {
	/* [0] 1280*720 @30fps */
	{
		.width		= 1280,
		.height		= 720,
		.fps		= 30 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= jxh63_init_regs_1280_720_30fps_dvp,
	},
	/* [1] 1280*720 @25fps */
	{
		.width		= 1280,
		.height		= 720,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= jxh63_init_regs_1280_720_30fps_mipi,
	},
};
struct tx_isp_sensor_win_setting *wsize = &jxh63_win_sizes[1];

/*
 * the part of driver was fixed.
 */

static struct regval_list jxh63_stream_on[] = {
//	{0x12, 0x00},
	{JXH63_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list jxh63_stream_off[] = {
	/* Sensor enter LP11*/
	{0x12, 0x40},
	{JXH63_REG_END, 0x00},	/* END MARKER */
};

int jxh63_read(struct tx_isp_subdev *sd, unsigned char reg,
	       unsigned char *value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	struct i2c_msg msg[2] = {
		[0] = {
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &reg,
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

int jxh63_write(struct tx_isp_subdev *sd, unsigned char reg,
		unsigned char value)
{
	int ret;
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned char buf[2] = {reg, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= buf,
	};
	ret = private_i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}

#if 0
static int jxh63_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;

	while (vals->reg_num != JXH63_REG_END) {
		if (vals->reg_num == JXH63_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = jxh63_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		/*pr_debug("vals->reg_num:0x%02x, vals->value:0x%02x\n",vals->reg_num, val);*/
		vals++;
	}

	return 0;
}
#endif

static int jxh63_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != JXH63_REG_END) {
		if (vals->reg_num == JXH63_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = jxh63_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		/*pr_debug("vals->reg_num:%x, vals->value:%x\n",vals->reg_num, vals->value);*/
		vals++;
	}

	return 0;
}

static int jxh63_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int jxh63_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	unsigned char v;
	int ret;

	ret = jxh63_read(sd, 0x0a, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != JXH63_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = jxh63_read(sd, 0x0b, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;

	if (v != JXH63_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int ag_last = -1;
static int it_last = -1;
static int jxh63_set_expo(struct tx_isp_subdev* sd, int value)
{
	int ret = 0;
	int expo = (value & 0x0000ffff);
	int again = (value & 0xffff0000) >> 16;

	if(it_last != expo)
	{
		ret = jxh63_write(sd, 0x01, (unsigned char)(expo & 0xff));
		ret += jxh63_write(sd, 0x02, (unsigned char)((expo >> 8) & 0xff));
	}
	if(ag_last != again)
	{
		ret += jxh63_write(sd, 0x00, (unsigned char)(again & 0x7f));
	}

	return ret;
}

#if 0
static int jxh63_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	unsigned int expo = value;
	int ret = 0;
	jxh63_write(sd, 0x01, (unsigned char)(expo & 0xff));
	jxh63_write(sd, 0x02, (unsigned char)((expo >> 8) & 0xff));
	if (ret < 0)
		return ret;

	return 0;
}
static int jxh63_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	/* 0x00 bit[6:0] */
	jxh63_write(sd, 0x00, (unsigned char)(value & 0x7f));
	return 0;
}
#endif

static int jxh63_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int jxh63_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int jxh63_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int jxh63_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = jxh63_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if (sensor->video.state == TX_ISP_MODULE_INIT) {
			ret = jxh63_write_array(sd, jxh63_stream_on);
			sensor->video.state = TX_ISP_MODULE_RUNNING;
			pr_debug("jxh63 stream on\n");
			sensor->video.state = TX_ISP_MODULE_RUNNING;
		}
	}
	else {
		ret = jxh63_write_array(sd, jxh63_stream_off);
		pr_debug("jxh63 stream off\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
	}
	return ret;
}

static int jxh63_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	unsigned int pclk = JXH63_SUPPORT_PCLK;
	unsigned short hts;
	unsigned short vts = 0;
	unsigned char tmp;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	/* the format of fps is 16/16. for example 25 << 16 | 2, the value is 25/2 fps. */
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (sensor_max_fps << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8))
		return -1;
	ret += jxh63_read(sd, 0x21, &tmp);
	hts = tmp;
	ret += jxh63_read(sd, 0x20, &tmp);
	if(ret < 0)
		return -1;
	hts = (hts << 8) + tmp;
	hts <<= 1;

	vts = pclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);
	jxh63_write(sd, 0xc0, 0x22);
	jxh63_write(sd, 0xc1, (unsigned char)(vts & 0xff));
	jxh63_write(sd, 0xc2, 0x23);
	jxh63_write(sd, 0xc3, (unsigned char)(vts >> 8));
	ret = jxh63_read(sd, 0x1f, &tmp);
	if(ret < 0)
		return -1;
	tmp |= (1 << 7); //set bit[7],  register group write function,  auto clean
	jxh63_write(sd, 0x1f, tmp);

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts-1;
	sensor->video.attr->integration_time_limit = vts-1;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts-1;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	return ret;
}

static int jxh63_set_mode(struct tx_isp_subdev *sd, int value)
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

static int jxh63_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;
	unsigned char val = 0;
	unsigned char tmp;

	ret += jxh63_read(sd, 0x12, &val);
	if (enable){
		val = val | 0x10;
		sensor->video.mbus.code = TISP_VI_FMT_SGRBG10_1X10;
	} else {
		val = val & 0xef;
		sensor->video.mbus.code = TISP_VI_FMT_SBGGR10_1X10;
	}
	sensor->video.mbus_change = 1;
	ret += jxh63_write(sd, 0xc4, 0x12);
	ret += jxh63_write(sd, 0xc5, val);
	ret += jxh63_read(sd, 0x1f, &tmp);
	if(ret < 0)
		return -1;
	tmp |= (1 << 7);
	ret += jxh63_write(sd, 0x1f, tmp);
	if(!ret)
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	return ret;
}

static int sensor_attr_check(struct tx_isp_subdev *sd)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	unsigned long rate;

	ISP_INFO("jxh63s1: default boot is %d, video interface is %d, mclk is %d, reset is %d, pwdn is %d\n", info->default_boot, info->video_interface, info->mclk, info->rst_gpio, info->pwdn_gpio);
	switch(info->default_boot){
	case 0:
		wsize = &jxh63_win_sizes[0];
		jxh63_attr.total_width = 1600;
		jxh63_attr.total_height = 900;
		jxh63_attr.min_integration_time = 1;
		jxh63_attr.max_integration_time = 900 -1;
		jxh63_attr.min_integration_time_native = 1;
		jxh63_attr.max_integration_time_native = 900 - 1;
		jxh63_attr.integration_time_limit = 900 - 1;
		pr_debug("----->dvp\n");
		break;
	case 1:
		wsize = &jxh63_win_sizes[1];
		jxh63_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		memcpy((void*)(&(jxh63_attr.mipi)),(void*)(&jxh63_mipi),sizeof(jxh63_mipi));
		jxh63_attr.total_width = 1600;
		jxh63_attr.total_height = 900;
		jxh63_attr.min_integration_time = 1;
		jxh63_attr.max_integration_time = 900 - 4;
		jxh63_attr.min_integration_time_native = 1;
		jxh63_attr.max_integration_time_native = 900 - 4;
		jxh63_attr.integration_time_limit = 900 - 4;
		pr_debug("----->mipi\n");
		break;
	default:
		ISP_ERROR("Have no this setting!!!\n");
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		jxh63_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		jxh63_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		jxh63_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		jxh63_attr.mipi.index = 1;
		break;
	case TISP_SENSOR_VI_DVP:
		jxh63_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
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

static int jxh63_g_chip_ident(struct tx_isp_subdev *sd,
			      struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"jxh63_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(10);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(1);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if (pwdn_gpio != -1) {
		ret = private_gpio_request(pwdn_gpio, "jxh63_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(50);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n", pwdn_gpio);
		}
	}
	ret = jxh63_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an jxh63 chip.\n",
		       client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("jxh63 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	if(chip){
		memcpy(chip->name, "jxh63s1", sizeof("jxh63s1"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int jxh63_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = jxh63_set_expo(sd, sensor_val->value);
		break;
#if 0
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			ret = jxh63_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = jxh63_set_analog_gain(sd, sensor_val->value);
		break;
#endif
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = jxh63_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = jxh63_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = jxh63_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		ret = jxh63_write_array(sd, jxh63_stream_off);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		ret = jxh63_write_array(sd, jxh63_stream_on);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = jxh63_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = jxh63_set_vflip(sd, sensor_val->value);
		break;
	default:
		break;
	}
	return 0;
}

static int jxh63_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = jxh63_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;
	return ret;
}

static int jxh63_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	jxh63_write(sd, reg->reg & 0xffff, reg->val & 0xff);
	return 0;
}

static struct tx_isp_subdev_core_ops jxh63_core_ops = {
	.g_chip_ident = jxh63_g_chip_ident,
	.reset = jxh63_reset,
	.init = jxh63_init,
	/*.ioctl = jxh63_ops_ioctl,*/
	.g_register = jxh63_g_register,
	.s_register = jxh63_s_register,
};

static struct tx_isp_subdev_video_ops jxh63_video_ops = {
	.s_stream = jxh63_s_stream,
};

static struct tx_isp_subdev_sensor_ops	jxh63_sensor_ops = {
	.ioctl	= jxh63_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops jxh63_ops = {
	.core = &jxh63_core_ops,
	.video = &jxh63_video_ops,
	.sensor = &jxh63_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "jxh63s1",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int jxh63_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tx_isp_subdev *sd;
	struct tx_isp_video_in *video;
	struct tx_isp_sensor *sensor;

	it_last = -1;
	ag_last = -1;
	sensor = (struct tx_isp_sensor *)kzalloc(sizeof(*sensor), GFP_KERNEL);
	if(!sensor){
		ISP_ERROR("Failed to allocate sensor subdev.\n");
		return -ENOMEM;
	}
	memset(sensor, 0 ,sizeof(*sensor));

	jxh63_attr.expo_fs = 1;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->dev = &client->dev;
	sensor->video.attr = &jxh63_attr;
	sensor->video.mbus_change = 0;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &jxh63_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("@@@@@@@probe ok ------->jxh63\n");

	return 0;
}

static int jxh63_remove(struct i2c_client *client)
{
	struct tx_isp_subdev *sd = private_i2c_get_clientdata(client);
	struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);

	if(reset_gpio != -1)
		private_gpio_free(reset_gpio);
	if(pwdn_gpio != -1)
		private_gpio_free(pwdn_gpio);

	private_clk_disable_unprepare(sensor->mclk);
	tx_isp_subdev_deinit(sd);
	kfree(sensor);

	return 0;
}

static const struct i2c_device_id jxh63_id[] = {
	{ "jxh63s1", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, jxh63_id);

static struct i2c_driver jxh63_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "jxh63s1",
	},
	.probe		= jxh63_probe,
	.remove		= jxh63_remove,
	.id_table	= jxh63_id,
};

static __init int init_jxh63(void)
{
	return private_i2c_add_driver(&jxh63_driver);
}

static __exit void exit_jxh63(void)
{
	private_i2c_del_driver(&jxh63_driver);
}

module_init(init_jxh63);
module_exit(exit_jxh63);

MODULE_DESCRIPTION("A low-level driver for SOI jxh63 sensors");
MODULE_LICENSE("GPL");
