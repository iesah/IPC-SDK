/*
 * jxk06.c
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Settings:
 * sboot        resolution      fps       interface              mode
 *   0          2560*1440        30       mipi_2lane           linear
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

#define JXK06_CHIP_ID_H	(0x08)
#define JXK06_CHIP_ID_L	(0x52)
#define JXK06_REG_END		0xff
#define JXK06_REG_DELAY	0xfe
#define JXK06_SUPPORT_30FPS_SCLK (86400000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20220824a"

uint8_t dismode;

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

struct again_lut jxk06_again_lut[] = {
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
};

struct tx_isp_sensor_attribute jxk06_attr;

unsigned int jxk06_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = jxk06_again_lut;

	while(lut->gain <= jxk06_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return lut[0].gain;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == jxk06_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int jxk06_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_sensor_attribute jxk06_attr={
	.name = "jxk06",
	.chip_id = 0x0852,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_8BITS,
	.cbus_device = 0x40,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.mode = SENSOR_MIPI_OTHER_MODE,
		.clk = 800,
		.lans = 2,
		.settle_time_apative_en = 0,
		.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
		.mipi_sc.hcrop_diff_en = 0,
		.mipi_sc.mipi_vcomp_en = 0,
		.mipi_sc.mipi_hcomp_en = 0,
		.image_twidth = 2560,
		.image_theight = 1440,
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
		.mipi_sc.data_type_value = 0,
		.mipi_sc.del_start = 0,
		.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
		.mipi_sc.sensor_fid_mode = 0,
		.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
	},
	.data_type = TX_SENSOR_DATA_TYPE_LINEAR,
	.max_again = 259142,
	.max_dgain = 0,
	.min_integration_time = 4,
	.min_integration_time_native = 4,
	.max_integration_time_native = 1500 -4,
	.integration_time_limit = 1500 - 4,
	.total_width = 3840,
	.total_height = 1500,
	.max_integration_time = 1500 -4,
	.one_line_expr_in_us = 22,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = jxk06_alloc_again,
	.sensor_ctrl.alloc_dgain = jxk06_alloc_dgain,
	//	void priv; /* point to struct tx_isp_sensor_board_info */
};

static struct regval_list jxk06_init_regs_2560_1440_30fps_mipi[] = {
	{0x12, 0x70},
	{0x48, 0x86},
	{0x48, 0x06},
	{0x0E, 0x11},
	{0x0F, 0x04},
	{0x10, 0x48},
	{0x11, 0x80},
	{0x46, 0x08},
	{0x0D, 0xA0},
	{0x57, 0x67},
	{0x58, 0x1F},
	{0x5F, 0x41},
	{0x60, 0x20},
	{0x20, 0xC0},
	{0x21, 0x03},
	{0x22, 0xDC},
	{0x23, 0x05},
	{0x24, 0x80},
	{0x25, 0xA0},
	{0x26, 0x52},
	{0x27, 0xBD},
	{0x28, 0x15},
	{0x29, 0x03},
	{0x2A, 0xB6},
	{0x2B, 0x13},
	{0x2C, 0x00},
	{0x2D, 0x00},
	{0x2E, 0x6E},
	{0x2F, 0x04},
	{0x41, 0x06},
	{0x42, 0x05},
	{0x47, 0x46},
	{0x76, 0x80},
	{0x77, 0x0C},
	{0x80, 0x01},
	{0xAF, 0x12},
	{0xAA, 0x04},
	{0x1D, 0x00},
	{0x1E, 0x04},
	{0x6C, 0x40},
	{0x9E, 0xF8},
	{0x0C, 0x30},
	{0x6F, 0x80},
	{0x6E, 0x2C},
	{0x70, 0xF9},
	{0x71, 0xDD},
	{0x72, 0xD5},
	{0x73, 0x5A},
	{0x74, 0x02},
	{0x78, 0x1D},
	{0x89, 0x01},
	{0x6B, 0x20},
	{0x86, 0x40},
	{0x30, 0x8D},
	{0x31, 0x08},
	{0x32, 0x20},
	{0x33, 0x5C},
	{0x34, 0x30},
	{0x35, 0x30},
	{0x3A, 0xB6},
	{0x56, 0x90},
	{0x59, 0x48},
	{0x5A, 0x01},
	{0x61, 0x00},
	{0x64, 0xC0},
	{0x85, 0x44},
	{0x8A, 0x00},
	{0x91, 0x08},
	{0x94, 0xE0},
	{0x9B, 0x8F},
	{0xA6, 0x02},
	{0xA7, 0xA0},
	{0xA9, 0x48},
	{0x45, 0x09},
	{0x5B, 0xA5},
	{0x5C, 0x4C},
	{0x5D, 0x97},
	{0x5E, 0x48},
	{0x65, 0x32},
	{0x66, 0x80},
	{0x67, 0x44},
	{0x68, 0x00},
	{0x69, 0x74},
	{0x6A, 0x2B},
	{0x7A, 0x82},
	{0x8D, 0x6F},
	{0x8F, 0x90},
	{0xA4, 0xC7},
	{0xA5, 0xAF},
	{0xB7, 0x21},
	{0x97, 0x20},
	{0x13, 0x81},
	{0x96, 0x84},
	{0x4A, 0x01},
	{0x7E, 0x4C},
	{0x50, 0x02},
	{0x93, 0xC0},
	{0xB5, 0x0C},
	{0xB1, 0x00},
	{0xA1, 0x0F},
	{0xA3, 0x40},
	{0x49, 0x10},
	{0x7F, 0x56},
	{0x8C, 0xFF},
	{0x8E, 0x00},
	{0x8B, 0x01},
	{0xBC, 0x11},
	{0x82, 0x00},
	{0x9F, 0x50},
	{0x19, 0x20},
	{0x1B, 0x4F},
	{0x12, 0x30},
	{0x48, 0x86},
	{0x48, 0x06},
	{0x00, 0x30},
	{JXK06_REG_END, 0x00},	/* END MARKER */
};

/*
 * the order of the jxk06_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting jxk06_win_sizes[] = {
	/* [0] 2560*1440@30fps linear */
	{
		.width		= 2560,
		.height		= 1440,
		.fps		= 30 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SGBRG10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= jxk06_init_regs_2560_1440_30fps_mipi,
	},
};
struct tx_isp_sensor_win_setting *wsize = &jxk06_win_sizes[0];

/*
 * the part of driver was fixed.
 */
static struct regval_list jxk06_stream_on_mipi[] = {
	{JXK06_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list jxk06_stream_off_mipi[] = {
	{JXK06_REG_END, 0x00},	/* END MARKER */
};

int jxk06_read(struct tx_isp_subdev *sd, unsigned char reg,
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

int jxk06_write(struct tx_isp_subdev *sd, unsigned char reg,
		unsigned char value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned char buf[2] = {reg, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= buf,
	};
	int ret;
	ret = private_i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}

#if 0
static int jxk06_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != JXK06_REG_END) {
		if (vals->reg_num == JXK06_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = jxk06_read(sd, vals->reg_num, &val);
			/* printk("{0x%x, 0x%x}\n", vals->reg_num, val); */
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}
#endif

static int jxk06_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != JXK06_REG_END) {
		if (vals->reg_num == JXK06_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = jxk06_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int jxk06_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int jxk06_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	unsigned char v;
	int ret;

	ret = jxk06_read(sd, 0x0a, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != JXK06_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = jxk06_read(sd, 0x0b, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;

	if (v != JXK06_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}
static int jxk06_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	int expo = (value & 0xffff);
	int again = (value & 0xffff0000) >> 16;

	/*set integration_time*/
	ret = jxk06_write(sd, 0x00, (unsigned char)(again & 0x7f));

	/*set sensor again*/
	ret += jxk06_write(sd, 0x01, (unsigned char)(expo & 0xff));
	ret += jxk06_write(sd, 0x02, (unsigned char)((expo >> 8) & 0xff));
	if (ret < 0)
		ISP_ERROR("err: jxk06_write err\n");

	return ret;
}

#if 0
static int jxk06_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	unsigned int expo = value;

	ret = jxk06_write(sd,  0x01, (unsigned char)(expo & 0xff));
	ret += jxk06_write(sd, 0x02, (unsigned char)((expo >> 8) & 0xff));
	if (ret < 0)
		return ret;

	return 0;
}

static int jxk06_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	if(value<0x10){
		ret += jxk06_write(sd, 0x0C, 0x00);
		ret += jxk06_write(sd, 0x66, 0x04);
		/* ret += jxk06_write(sd, 0x66, 0x41); */
		if (ret < 0)
			return ret;
	}else if(value>=0x10 && value <0x30){
		ret += jxk06_write(sd, 0x0C, 0x00);
		ret += jxk06_write(sd, 0x66, 0x24);
		if (ret < 0)
			return ret;
	}else{
		ret += jxk06_write(sd, 0x0C, 0x40);
	}

	ret += jxk06_write(sd, 0x00, (unsigned char)(value & 0x7f));

	//complement the steak
	if(value <= 0x40){
		ret += jxk06_write(sd, 0x99, (unsigned char)(val_99 & 0x0F));
		ret += jxk06_write(sd, 0x9b, (unsigned char)(val_9b & 0x0F));
	} else {
		ret += jxk06_write(sd, 0x99, (unsigned char)(val_99));
		ret += jxk06_write(sd, 0x9b, (unsigned char)(val_9b));
	}

	if (ret < 0)
		return ret;

	return 0;
}
#endif

static int jxk06_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int jxk06_get_black_pedestal(struct tx_isp_subdev *sd, int value)
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
	sensor->video.max_fps = wsize->fps;
	sensor->video.min_fps = SENSOR_OUTPUT_MIN_FPS << 16 | 1;

	return 0;
}

static int jxk06_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if(!init->enable)
		return ISP_SUCCESS;

	sensor_set_attr(sd, wsize);
	sensor->video.state = TX_ISP_MODULE_INIT;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	sensor->priv = wsize;

	return 0;
}

static int jxk06_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if (init->enable) {
		if(sensor->video.state == TX_ISP_MODULE_INIT){
			ret = jxk06_write_array(sd, wsize->regs);
			ret = jxk06_read(sd, 0x27, &dismode);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_RUNNING;
		}
		if(sensor->video.state == TX_ISP_MODULE_RUNNING){

			ret = jxk06_write_array(sd, jxk06_stream_on_mipi);
			ISP_WARNING("jxk06 stream on\n");
		}
	}
	else {
		ret = jxk06_write_array(sd, jxk06_stream_off_mipi);
		ISP_WARNING("jxk06 stream off\n");
	}

	return ret;
}

static int jxk06_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	unsigned int sclk = 0;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned int max_fps;
	unsigned char val = 0;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	switch (sensor->info.default_boot) {
	case 0 ... 1:
		sclk = JXK06_SUPPORT_30FPS_SCLK;
		max_fps = SENSOR_OUTPUT_MAX_FPS;
		break;
	default:
		ret = -1;
		ISP_ERROR("Now we do not support this boot sel!!!\n");
	}

	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (max_fps << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)) {
		ISP_WARNING("warn: fps(%d) no in range\n", fps);
		return -1;
	}

	val = 0;
	ret += jxk06_read(sd, 0x21, &val);
	hts = val<<8;
	val = 0;
	ret += jxk06_read(sd, 0x20, &val);
	hts = (hts | val) << 1;
	if (0 != ret) {
		ISP_ERROR("err: jxk06 read err\n");
		return ret;
	}

	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);
#if 0
	/*use group write*/
	jxk06_write(sd, 0xc0, 0x22);
	jxk06_write(sd, 0xc1, (unsigned char)(vts & 0xff));
	jxk06_write(sd, 0xc2, 0x23);
	jxk06_write(sd, 0xc3, (unsigned char)(vts >> 8));
	ret = jxk06_read(sd, 0x1f, &val);
	if(ret < 0)
		return -1;
	val |= (1 << 7); //set bit[7],  register group write function,  auto clean
	jxk06_write(sd, 0x1f, val);
	pr_debug("after register 0x1f value : 0x%02x\n", val);
#else
	ret = jxk06_write(sd, 0x22, (unsigned char)(vts & 0xff));
	ret += jxk06_write(sd, 0x23, (unsigned char)(vts >> 8));
#endif
	if (0 != ret) {
		ISP_ERROR("err: jxk06_write err\n");
		return ret;
	}
	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 4;
	sensor->video.attr->integration_time_limit = vts - 4;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 4;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int jxk06_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = ISP_SUCCESS;

	if(wsize){
		sensor_set_attr(sd, wsize);
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	}

	return ret;
}

static int jxk06_set_vflip(struct tx_isp_subdev *sd, int enable)
{
        int ret = 0;
        uint8_t val;
        struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

        /* 2'b01:mirror,2'b10:filp */
        switch(enable) {
        case 0:	
                jxk06_read(sd, 0x12, &val);
                val &= ~0x30;
                jxk06_write(sd, 0x12, val | 0x30);
                jxk06_read(sd, 0xaa, &val);
                val &= ~0x0f;
                jxk06_write(sd, 0xaa, val | 0x04);
                jxk06_write(sd, 0x27, dismode);
                break;
        case 1:
                jxk06_read(sd, 0x12, &val);
                val &= ~0x30;
                jxk06_write(sd, 0x12, val | 0x10);
                jxk06_read(sd, 0xaa, &val);
                val &= ~0x0f;
                jxk06_write(sd, 0xaa, val | 0x0b);
                jxk06_write(sd, 0x27, dismode + 1);
                break;
        case 2:
                jxk06_read(sd, 0x12, &val);
                val &= ~0x30;
                jxk06_write(sd, 0x12, val | 0x20);
                jxk06_read(sd, 0xaa, &val);
                val &= ~0x0f;
                jxk06_write(sd, 0xaa, val | 0x04);
                jxk06_write(sd, 0x27, dismode);
                break;
        case 3:
                jxk06_read(sd, 0x12, &val);
                val &= ~0x30;
                jxk06_write(sd, 0x12, val | 0x00);
                jxk06_read(sd, 0xaa, &val);
                val &= ~0x0f;
                jxk06_write(sd, 0xaa, val | 0x0b);
                jxk06_write(sd, 0x27, dismode + 1);
                break;
	}

	if(!ret)
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sensor_attr_check(struct tx_isp_subdev *sd)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	struct clk *sclka;
	unsigned long rate;

	switch(info->default_boot){
	case 0:
		wsize = &jxk06_win_sizes[0];
		jxk06_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		jxk06_attr.max_integration_time_native = 1500 - 4;
		jxk06_attr.integration_time_limit = 1500 - 4;
		jxk06_attr.total_width = 3840;
		jxk06_attr.total_height = 1500;
		jxk06_attr.max_integration_time = 1500 - 4;
		jxk06_attr.again = 0;
		jxk06_attr.integration_time = 0x30;
		break;
	default:
		ISP_ERROR("Have no this MCLK Source!!!\n");
		break;
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		jxk06_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		jxk06_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		jxk06_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		jxk06_attr.mipi.index = 1;
		break;
	case TISP_SENSOR_VI_DVP:
		jxk06_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
		break;
	default:
		ISP_ERROR("Have no this interface!!!\n");
	}

	switch(info->mclk){
	case TISP_SENSOR_MCLK0:
	case TISP_SENSOR_MCLK1:
	case TISP_SENSOR_MCLK2:
		sclka = private_devm_clk_get(&client->dev, "mux_cim");
		sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim");
		set_sensor_mclk_function(0);
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

	sensor_set_attr(sd, wsize);
	sensor->priv = wsize;
        sensor->video.max_fps = wsize->fps;
	sensor->video.min_fps = SENSOR_OUTPUT_MIN_FPS << 16 | 1;
	return 0;

err_get_mclk:
	return -1;
}

static int jxk06_g_chip_ident(struct tx_isp_subdev *sd,
			      struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(info->rst_gpio != -1){
		ret = private_gpio_request(info->rst_gpio,"jxk06_reset");
		if(!ret){
			private_gpio_direction_output(info->rst_gpio, 1);
			private_msleep(50);
			private_gpio_direction_output(info->rst_gpio, 0);
			private_msleep(35);
			private_gpio_direction_output(info->rst_gpio, 1);
			private_msleep(35);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",info->rst_gpio);
		}
	}
	if(info->pwdn_gpio != -1){
		ret = private_gpio_request(info->pwdn_gpio,"jxk06_pwdn");
		if(!ret){
			private_gpio_direction_output(info->pwdn_gpio, 1);
			private_msleep(150);
			private_gpio_direction_output(info->pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",info->pwdn_gpio);
		}
	}
	ret = jxk06_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an jxk06 chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("jxk06 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "jxk06", sizeof("jxk06"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}
	return 0;
}

static int jxk06_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = jxk06_set_expo(sd, sensor_val->value);
		break;
/*
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			ret = jxk06_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = jxk06_set_analog_gain(sd, sensor_val->value);
		break;
*/
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = jxk06_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = jxk06_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = jxk06_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		ret = jxk06_write_array(sd, jxk06_stream_off_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		ret = jxk06_write_array(sd, jxk06_stream_on_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = jxk06_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = jxk06_set_vflip(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return ret;
}

static int jxk06_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = jxk06_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int jxk06_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	jxk06_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops jxk06_core_ops = {
	.g_chip_ident = jxk06_g_chip_ident,
	.reset = jxk06_reset,
	.init = jxk06_init,
	/*.ioctl = jxk06_ops_ioctl,*/
	.g_register = jxk06_g_register,
	.s_register = jxk06_s_register,
};

static struct tx_isp_subdev_video_ops jxk06_video_ops = {
	.s_stream = jxk06_s_stream,
};

static struct tx_isp_subdev_sensor_ops	jxk06_sensor_ops = {
	.ioctl	= jxk06_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops jxk06_ops = {
	.core = &jxk06_core_ops,
	.video = &jxk06_video_ops,
	.sensor = &jxk06_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "jxk06",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int jxk06_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

	/*
	  convert sensor-gain into isp-gain,
	*/
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->dev = &client->dev;
	jxk06_attr.expo_fs = 1;
	sensor->video.shvflip = 1;
	sensor->video.attr = &jxk06_attr;
	tx_isp_subdev_init(&sensor_platform_device, sd, &jxk06_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	ISP_WARNING("probe ok ------->jxk06\n");

	return 0;
}

static int jxk06_remove(struct i2c_client *client)
{
	struct tx_isp_subdev *sd = private_i2c_get_clientdata(client);
	struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;

	if(info->rst_gpio != -1)
		private_gpio_free(info->rst_gpio);
	if(info->pwdn_gpio != -1)
		private_gpio_free(info->pwdn_gpio);

	private_clk_disable_unprepare(sensor->mclk);
    	private_devm_clk_put(&client->dev, sensor->mclk);
	tx_isp_subdev_deinit(sd);
	kfree(sensor);

	return 0;
}

static const struct i2c_device_id jxk06_id[] = {
	{ "jxk06", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, jxk06_id);

static struct i2c_driver jxk06_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "jxk06",
	},
	.probe		= jxk06_probe,
	.remove		= jxk06_remove,
	.id_table	= jxk06_id,
};

static __init int init_jxk06(void)
{
	/* ret = private_driver_get_interface(); */
	/* if(ret){ */
	/* 	ISP_ERROR("Failed to init jxk06 dirver.\n"); */
	/* 	return -1; */
	/* } */
	return private_i2c_add_driver(&jxk06_driver);
}

static __exit void exit_jxk06(void)
{
	private_i2c_del_driver(&jxk06_driver);
}

module_init(init_jxk06);
module_exit(exit_jxk06);

MODULE_DESCRIPTION("A low-level driver for SOI jxk06 sensors");
MODULE_LICENSE("GPL");
