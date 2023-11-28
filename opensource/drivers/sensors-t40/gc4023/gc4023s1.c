/*
 * gc4023.c
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

#define GC4023_CHIP_ID_H	(0x40)
#define GC4023_CHIP_ID_L	(0x23)
#define GC4023_REG_END		0xffff
#define GC4023_REG_DELAY	0x0000
#define GC4023_SUPPORT_30FPS_SCLK 108*1000*1000

#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20220523a"

static int reset_gpio = GPIO_PC(27);
static int pwdn_gpio = -1;

struct regval_list {
	uint16_t reg_num;
	unsigned char value;
};

/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
	unsigned int index;
	unsigned char reg614;
	unsigned char reg615;
	unsigned char reg218;
	unsigned char reg1467;
	unsigned char reg1468;
	unsigned char regb8;
	unsigned char regb9;
	unsigned int gain;
};



struct again_lut gc4023_again_lut[] = {
 	{0x00, 0x00,0x00,0x00,0x07,0x0f,0x01,0x00,     0 },
 	{0x01, 0x80,0x02,0x00,0x07,0x0f,0x01,0x0B, 16208 },
 	{0x02, 0x01,0x00,0x00,0x07,0x0f,0x01,0x19, 32217 },
 	{0x03, 0x81,0x02,0x00,0x07,0x0f,0x01,0x2A, 47690 },
 	{0x04, 0x02,0x00,0x00,0x08,0x10,0x02,0x00, 65536 },
 	{0x05, 0x82,0x02,0x00,0x08,0x10,0x02,0x17, 81784 },
 	{0x06, 0x03,0x00,0x00,0x08,0x10,0x02,0x33, 97213 },
	{0x07, 0x83,0x02,0x00,0x09,0x11,0x03,0x14,113226 },
	{0x08, 0x04,0x00,0x00,0x0A,0x12,0x04,0x00,131072 },
	{0x09, 0x80,0x02,0x20,0x0A,0x12,0x04,0x2F,147001 },
	{0x0A, 0x01,0x00,0x20,0x0B,0x13,0x05,0x26,162766 },
	{0x0B, 0x81,0x02,0x20,0x0C,0x14,0x06,0x28,178990 },
	{0x0C, 0x02,0x00,0x20,0x0D,0x15,0x08,0x00,196608 },
	{0x0D, 0x82,0x02,0x20,0x0D,0x15,0x09,0x1E,212696 },
	{0x0E, 0x03,0x00,0x20,0x0E,0x16,0x0B,0x0C,228446 },
	{0x0F, 0x83,0x02,0x20,0x0E,0x16,0x0D,0x11,244419 },
	{0x10, 0x04,0x00,0x20,0x0F,0x17,0x10,0x00,262144 },
	{0x11, 0x84,0x02,0x20,0x0F,0x17,0x12,0x3D,278158 },
	{0x12, 0x05,0x00,0x20,0x10,0x18,0x16,0x19,293982 },
	{0x13, 0x85,0x02,0x20,0x10,0x18,0x1A,0x22,310012 },
	{0x14, 0xb5,0x04,0x20,0x11,0x19,0x20,0x00,327680 },
	{0x15, 0x85,0x05,0x20,0x11,0x19,0x25,0x3A,343731 },
	{0x16, 0x05,0x08,0x20,0x12,0x1a,0x2C,0x33,359484 },
	{0x17, 0x45,0x09,0x20,0x15,0x1d,0x35,0x05,375550 },
	{0x18, 0x55,0x0a,0x20,0x16,0x1e,0x40,0x00,393216 },
};

struct tx_isp_sensor_attribute gc4023_attr;

unsigned int gc4023_alloc_integration_time(unsigned int it, unsigned char shift, unsigned int *sensor_it)
{
	unsigned int expo = it >> shift;
	unsigned int isp_it = it;

	*sensor_it = expo;

	return isp_it;
}

unsigned int gc4023_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = gc4023_again_lut;
	while(lut->gain <= gc4023_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = 0;
			return lut[0].gain;
		} else if (isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->index;
			return (lut - 1)->gain;
		} else {
			if((lut->gain == gc4023_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->index;
				return lut->gain;
			}
		}

		lut++;
	}

	return 0;
}

unsigned int gc4023_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus gc4023_mipi_linear = {
	.mode = SENSOR_MIPI_OTHER_MODE,
	.clk = 864,
	.index = 1,
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
};

struct tx_isp_sensor_attribute gc4023_attr={
	.name = "gc4023s1",
	.chip_id = 0x4023,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.data_type = TX_SENSOR_DATA_TYPE_LINEAR,
	.cbus_device = 0x29,
	.max_again = 393216,
	.max_dgain = 0,
	.expo_fs = 1,
	.min_integration_time = 2,
	.min_integration_time_short = 2,
	.min_integration_time_native = 2,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 2,
	.sensor_ctrl.alloc_again = gc4023_alloc_again,
	.sensor_ctrl.alloc_dgain = gc4023_alloc_dgain,
	//	void priv; /* point to struct tx_isp_sensor_board_info */
};

static struct regval_list gc4023_init_regs_2560_1440_25fps_mipi[] = {
	/*
	 * version 0.2 mclk 27Mhz wpclk 216Mhz rpclk 172.2Mhz mipi 864Mbps/lane
	 * cpclk 27Mhz vts = 1500 window 2560 1440
	 */
	/*SYSTEM*/
   	{0x03fe, 0xf0},
   	{0x03fe, 0x00},
   	{0x03fe, 0x10},
   	{0x03fe, 0x00},
   	{0x0a38, 0x00},
   	{0x0a38, 0x01},
   	{0x0a20, 0x07},
   	{0x061c, 0x50},
   	{0x061d, 0x22},
   	{0x061e, 0x78},
   	{0x061f, 0x06},
   	{0x0a21, 0x10},
   	{0x0a34, 0x40},
   	{0x0a35, 0x01},
   	{0x0a36, 0x60},
   	{0x0a37, 0x06},
   	{0x0314, 0x50},
   	{0x0315, 0x00},
   	{0x031c, 0xce},
   	{0x0219, 0x47},
   	{0x0342, 0x04},
   	{0x0343, 0xb0},
   	{0x0259, 0x05},
   	{0x025a, 0xa0},
   	{0x0340, 0x05},
   	{0x0341, 0xdc},
   	{0x0347, 0x02},
   	{0x0348, 0x0a},
   	{0x0349, 0x08},
   	{0x034a, 0x05},
   	{0x034b, 0xa8},
   	{0x0094, 0x0a},
   	{0x0095, 0x00},
   	{0x0096, 0x05},
   	{0x0097, 0xa0},
   	{0x0099, 0x04},
   	{0x009b, 0x04},
   	{0x060c, 0x01},
   	{0x060e, 0x08},
   	{0x060f, 0x05},
   	{0x070c, 0x01},
   	{0x070e, 0x08},
   	{0x070f, 0x05},
   	{0x0909, 0x03},
   	{0x0902, 0x04},
   	{0x0904, 0x0b},
   	{0x0907, 0x54},
   	{0x0908, 0x06},
   	{0x0903, 0x9d},
   	{0x072a, 0x18},
   	{0x0724, 0x0a},
   	{0x0727, 0x0a},
   	{0x072a, 0x18},
   	{0x072b, 0x08},
   	{0x1466, 0x10},
   	{0x1468, 0x0f},
   	{0x1467, 0x07},
   	{0x1469, 0x80},
   	{0x146a, 0xbc},
   	{0x0707, 0x07},
   	{0x0737, 0x0f},
   	{0x061a, 0x00},
   	{0x1430, 0x80},
   	{0x1407, 0x10},
   	{0x1408, 0x16},
   	{0x1409, 0x03},
   	{0x146d, 0x0e},
   	{0x146e, 0x32},
   	{0x146f, 0x33},
   	{0x1470, 0x2c},
   	{0x1471, 0x2d},
   	{0x1472, 0x3a},
   	{0x1473, 0x3a},
   	{0x1474, 0x40},
   	{0x1475, 0x36},
   	{0x1420, 0x14},
   	{0x1464, 0x15},
   	{0x146c, 0x40},
   	{0x146d, 0x40},
   	{0x1423, 0x08},
   	{0x1428, 0x10},
   	{0x1462, 0x08},
   	{0x02ce, 0x04},
   	{0x143a, 0x0b},
   	{0x142b, 0x88},
   	{0x0245, 0xc9},
   	{0x023a, 0x08},
   	{0x02cd, 0x88},
   	{0x0612, 0x02},
   	{0x0613, 0xc7},
   	{0x0243, 0x03},
   	{0x021b, 0x09},
   	{0x0089, 0x03},
   	{0x0040, 0xa3},
   	{0x0075, 0x64},
   	{0x0004, 0x0f},
   	{0x0002, 0xa9},
   	{0x0053, 0x0a},
   	{0x0205, 0x0c},
   	{0x0202, 0x06},
   	{0x0203, 0x27},
   	{0x0614, 0x00},
   	{0x0615, 0x00},
   	{0x0181, 0x0c},
   	{0x0182, 0x05},
   	{0x0185, 0x01},
   	{0x0180, 0x46},
   	{0x0100, 0x08},
   	{0x0106, 0x38},
   	{0x010d, 0x80},
   	{0x010e, 0x0c},
   	{0x0113, 0x02},
   	{0x0114, 0x01},
   	{0x0115, 0x10},
   	{0x0100, 0x09},

	{GC4023_REG_END, 0x00},	/* END MARKER */
};

/*
 * the order of the jxf23_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting gc4023_win_sizes[] = {
	/* [0] 2560*1440 @max 30fps*/
	{
		.width		= 2560,
		.height		= 1440,
		.fps		= 30 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SRGGB10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= gc4023_init_regs_2560_1440_25fps_mipi,
	},
};

struct tx_isp_sensor_win_setting *wsize = &gc4023_win_sizes[0];

/*
 * the part of driver was fixed.
 */

static struct regval_list gc4023_stream_on[] = {
	{GC4023_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list gc4023_stream_off[] = {
	{GC4023_REG_END, 0x00},	/* END MARKER */
};

int gc4023_read(struct tx_isp_subdev *sd,  uint16_t reg,
		unsigned char *value)
{
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
	int ret;
	ret = private_i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;

	return ret;
}

int gc4023_write(struct tx_isp_subdev *sd, uint16_t reg,
		 unsigned char value)
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
static int gc4023_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != GC4023_REG_END) {
		if (vals->reg_num == GC4023_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = gc4023_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		pr_debug("vals->reg_num:0x%x, vals->value:0x%02x\n",vals->reg_num, val);
		vals++;
	}
	return 0;
}
#endif

static int gc4023_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != GC4023_REG_END) {
		if (vals->reg_num == GC4023_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = gc4023_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int gc4023_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int gc4023_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	unsigned char v;
	int ret;
	ret = gc4023_read(sd, 0x03f0, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != GC4023_CHIP_ID_H)
		return -ENODEV;
	ret = gc4023_read(sd, 0x03f1, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != GC4023_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

#if 0
static int gc4023_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	struct again_lut *val_lut = gc4023_again_lut;

	/* printk("it is %d, again is %d\n",expo,again); */
	/*expo*/
	ret = gc4023_write(sd, 0x0203, value & 0xff);
	ret += gc4023_write(sd, 0x0202, value >> 8);
	if (ret < 0) {
		ISP_ERROR("gc4023_write error  %d\n" ,__LINE__ );
		return ret;
	}

	/*gain*/

	ret = gc4023_write(sd, 0x0614, val_lut[value].reg614);
	ret = gc4023_write(sd, 0x0615, val_lut[value].reg615);

	ret = gc4023_write(sd, 0x0218, val_lut[value].reg218);
	ret = gc4023_write(sd, 0x1467, val_lut[value].reg1467);
	ret = gc4023_write(sd, 0x1468, val_lut[value].reg1468);
	ret = gc4023_write(sd, 0x00b8, val_lut[value].regb8);
	ret = gc4023_write(sd, 0x00b9, val_lut[value].regb9);

	if (ret < 0) {
		ISP_ERROR("gc4023_write error  %d" ,__LINE__ );
		return ret;
	}

	return 0;
}
#endif

static int gc4023_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	ret = gc4023_write(sd, 0x0203, value & 0xff);
	ret += gc4023_write(sd, 0x0202, value >> 8);
	if (ret < 0)
		ISP_ERROR("gc4023_write error  %d\n" ,__LINE__ );

	return ret;
}

static int gc4023_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	struct again_lut *val_lut = gc4023_again_lut;


	ret = gc4023_write(sd, 0x0614, val_lut[value].reg614);
	ret = gc4023_write(sd, 0x0615, val_lut[value].reg615);

	ret = gc4023_write(sd, 0x0218, val_lut[value].reg218);
	ret = gc4023_write(sd, 0x1467, val_lut[value].reg1467);
	ret = gc4023_write(sd, 0x1468, val_lut[value].reg1468);
	ret = gc4023_write(sd, 0x00b8, val_lut[value].regb8);
	ret = gc4023_write(sd, 0x00b9, val_lut[value].regb9);
	if (ret < 0) {
		ISP_ERROR("gc4023_write error  %d" ,__LINE__ );
		return ret;
	}

	return 0;
}

static int gc4023_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int gc4023_get_black_pedestal(struct tx_isp_subdev *sd, int value)
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

static int gc4023_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if(!init->enable)
		return ISP_SUCCESS;

	sensor_set_attr(sd, wsize);
	sensor->video.state = TX_ISP_MODULE_DEINIT;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	sensor->priv = wsize;

	return 0;
}

static int gc4023_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = gc4023_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if (sensor->video.state == TX_ISP_MODULE_INIT) {
			ret = gc4023_write_array(sd, gc4023_stream_on);
			sensor->video.state = TX_ISP_MODULE_RUNNING;
			pr_debug("gc4023 stream on\n");
			sensor->video.state = TX_ISP_MODULE_RUNNING;
		}
	} else {
		ret = gc4023_write_array(sd, gc4023_stream_off);
		pr_debug("gc4023 stream off\n");
	}

	return ret;
}

static int gc4023_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	unsigned int wpclk = 0;
	unsigned short vts = 0;
	unsigned short hts = 0;
	unsigned char tmp;
	unsigned int sensor_max_fps;
	unsigned char sensor_min_fps;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	switch(sensor->info.default_boot){
		case 0:
			wpclk = GC4023_SUPPORT_30FPS_SCLK;
			sensor_max_fps = TX_SENSOR_MAX_FPS_30;
			sensor_min_fps = TX_SENSOR_MAX_FPS_5;
			break;
		default:
			ISP_ERROR("Now we do not support this framerate!!!\n");
	}

	/* the format of fps is 16/16. for example 30 << 16 | 2, the value is 30/2 fps. */
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (sensor_max_fps << 8) || newformat < (sensor_min_fps<< 8)){
		ISP_ERROR("warn: fps(%x) no in range\n", fps);
		return -1;
	}

    ret += gc4023_read(sd, 0x0342, &tmp);
	hts = tmp & 0x0f;
	ret += gc4023_read(sd, 0x0343, &tmp);
	if(ret < 0)
		return -1;
	hts = ((hts << 8) + tmp) << 1;

	vts = wpclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16) ;
	ret = gc4023_write(sd, 0x0340, (unsigned char)((vts & 0x3f00) >> 8));
	ret += gc4023_write(sd, 0x0341, (unsigned char)(vts & 0xff));
	if(ret < 0)
		return -1;

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 8;
	sensor->video.attr->integration_time_limit = vts - 8;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 8;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return 0;
}

static int gc4023_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = ISP_SUCCESS;

	if(wsize){
		sensor_set_attr(sd, wsize);
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
	unsigned long rate;
	int ret = 0;

	switch(info->default_boot){
	case 0:
		wsize = &gc4023_win_sizes[0];
		gc4023_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		memcpy(&gc4023_attr.mipi, &gc4023_mipi_linear, sizeof(gc4023_mipi_linear));
		gc4023_attr.one_line_expr_in_us = 22;
		gc4023_attr.total_width = 3840;
		gc4023_attr.total_height = 1500;
		gc4023_attr.max_integration_time_native = 1500 - 8;
		gc4023_attr.integration_time_limit = 1500 - 8;
		gc4023_attr.max_integration_time = 1500 - 8;
		break;
	default:
		ISP_ERROR("Have no this setting!!!\n");
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		gc4023_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		gc4023_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		gc4023_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		gc4023_attr.mipi.index = 1;
		break;
	case TISP_SENSOR_VI_DVP:
		gc4023_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
		break;
	default:
		ISP_ERROR("Have no this interface!!!\n");
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

	if (IS_ERR(sensor->mclk)) {
		ISP_ERROR("Cannot get sensor input clock cgu_cim\n");
		goto err_get_mclk;
	}
	rate = private_clk_get_rate(sensor->mclk);
	if (((rate / 1000) % 27000) != 0) {
		ret = clk_set_parent(sclka, clk_get(NULL, "epll"));
		sclka = private_devm_clk_get(&client->dev, "epll");
		if (IS_ERR(sclka)) {
			pr_err("get sclka failed\n");
		} else {
			rate = private_clk_get_rate(sclka);
			if (((rate / 1000) % 27000) != 0) {
				private_clk_set_rate(sclka, 891000000);
			}
		}
	}

	private_clk_set_rate(sensor->mclk, 27000000);
	private_clk_prepare_enable(sensor->mclk);

	reset_gpio = info->rst_gpio;
	pwdn_gpio = info->pwdn_gpio;

	sensor_set_attr(sd, wsize);
	sensor->priv = wsize;

	return 0;

err_get_mclk:
	return -1;
}

static int gc4023_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"gc4023_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(20);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"gc4023_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = gc4023_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an gc4023 chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("gc4023s1 chip found @ 0x%02x (%s)\n sensor drv version %s", client->addr, client->adapter->name, SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "gc4023s1", sizeof("gc4023s1"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}
	return 0;
}

static int gc4023_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
{
	long ret = 0;
	struct tx_isp_sensor_value *sensor_val = arg;

	if(IS_ERR_OR_NULL(sd)){
		ISP_ERROR("[%d]The pointer is invalid!\n", __LINE__);
		return -EINVAL;
	}
	switch(cmd){
#if 0
	case TX_ISP_EVENT_SENSOR_EXPO:
		if(arg)
			ret = gc4023_set_expo(sd,sensor_val->value);
		break;
#endif
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			ret = gc4023_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = gc4023_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = gc4023_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = gc4023_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = gc4023_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		ret = gc4023_write_array(sd, gc4023_stream_off);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		ret = gc4023_write_array(sd, gc4023_stream_on);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = gc4023_set_fps(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return ret;
}

static int gc4023_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = gc4023_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int gc4023_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	gc4023_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops gc4023_core_ops = {
	.g_chip_ident = gc4023_g_chip_ident,
	.reset = gc4023_reset,
	.init = gc4023_init,
	/*.ioctl = gc4023_ops_ioctl,*/
	.g_register = gc4023_g_register,
	.s_register = gc4023_s_register,
};

static struct tx_isp_subdev_video_ops gc4023_video_ops = {
	.s_stream = gc4023_s_stream,
};

static struct tx_isp_subdev_sensor_ops	gc4023_sensor_ops = {
	.ioctl	= gc4023_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops gc4023_ops = {
	.core = &gc4023_core_ops,
	.video = &gc4023_video_ops,
	.sensor = &gc4023_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "gc4023s1",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int gc4023_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	//gc4023_attr.expo_fs = 1;
	sensor->video.attr = &gc4023_attr;
	tx_isp_subdev_init(&sensor_platform_device, sd, &gc4023_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->gc4023\n");

	return 0;
}

static int gc4023_remove(struct i2c_client *client)
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

static const struct i2c_device_id gc4023_id[] = {
	{ "gc4023s1", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, gc4023_id);

static struct i2c_driver gc4023_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "gc4023s1",
	},
	.probe		= gc4023_probe,
	.remove		= gc4023_remove,
	.id_table	= gc4023_id,
};

static __init int init_gc4023(void)
{
	return private_i2c_add_driver(&gc4023_driver);
}

static __exit void exit_gc4023(void)
{
	private_i2c_del_driver(&gc4023_driver);
}

module_init(init_gc4023);
module_exit(exit_gc4023);

MODULE_DESCRIPTION("A low-level driver for gc4023 sensors");
MODULE_LICENSE("GPL");
