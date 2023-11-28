/*
 * ov2740.c
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

#define OV2740_CHIP_ID_H	(0x27)
#define OV2740_CHIP_ID_L	(0x40)

#define OV2740_REG_END		0xffff
#define OV2740_REG_DELAY	0xfffe

#define OV2740_SUPPORT_SCLK_MIPI (0x870 * 0x460 * 30)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define MCLK 24000000
#define SENSOR_VERSION	"H20220517a"

static int reset_gpio = -1;
static int pwdn_gpio = -1;

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
struct again_lut ov2740_again_lut[] = {
	{0x80, 0},
	{0x84, 3344},
	{0x89, 6485},
	{0x8d, 9611},
	{0x92, 12637},
	{0x96, 15569},
	{0x9b, 18335},
	{0xa0, 21098},
	{0xa4, 23782},
	{0xa9, 26321},
	{0xad, 28863},
	{0xb2, 31339},
	{0xb6, 33751},
	{0xbb, 36039},
	{0xc0, 38336},
	{0xc4, 40578},
	{0xc9, 42709},
	{0xcd, 44851},
	{0xd2, 46946},
	{0xd6, 48995},
	{0xdb, 50946},
	{0xe0, 52911},
	{0xe4, 54836},
	{0xe9, 56671},
	{0xed, 58522},
	{0xf2, 60337},
	{0xf6, 62119},
	{0xfb, 63819},
	{0x100, 65536},
	{0x105, 67640},
	{0x10b, 69743},
	{0x111, 71756},
	{0x117, 73771},
	{0x11d, 75701},
	{0x122, 77634},
	{0x128, 79487},
	{0x12e, 81345},
	{0x134, 83128},
	{0x13a, 84916},
	{0x140, 86634},
	{0x145, 88321},
	{0x14b, 90014},
	{0x151, 91642},
	{0x157, 93278},
	{0x15d, 94851},
	{0x162, 96433},
	{0x168, 97955},
	{0x16e, 99486},
	{0x174, 100960},
	{0x17a, 102443},
	{0x180, 103872},
	{0x187, 105714},
	{0x18f, 107520},
	{0x196, 109262},
	{0x19e, 111003},
	{0x1a5, 112711},
	{0x1ad, 114390},
	{0x1b4, 116039},
	{0x1bc, 117660},
	{0x1c3, 119227},
	{0x1cb, 120795},
	{0x1d2, 122337},
	{0x1da, 123854},
	{0x1e1, 125347},
	{0x1e9, 126818},
	{0x1f0, 128241},
	{0x1f8, 129667},
	{0x200, 131072},
	{0x209, 132875},
	{0x213, 134644},
	{0x21d, 136380},
	{0x227, 138086},
	{0x231, 139761},
	{0x23b, 141406},
	{0x244, 143003},
	{0x24e, 144594},
	{0x258, 146159},
	{0x262, 147698},
	{0x26c, 149212},
	{0x276, 150702},
	{0x280, 152170},
	{0x28c, 154042},
	{0x299, 155878},
	{0x2a6, 157679},
	{0x2b3, 159446},
	{0x2c0, 161181},
	{0x2cc, 162885},
	{0x2d9, 164558},
	{0x2e6, 166203},
	{0x2f3, 167819},
	{0x300, 169408},
	{0x310, 171358},
	{0x320, 173268},
	{0x330, 175140},
	{0x340, 176976},
	{0x350, 178777},
	{0x360, 180544},
	{0x370, 182279},
	{0x380, 183983},
	{0x393, 186040},
	{0x3a7, 188054},
	{0x3bb, 190026},
	{0x3ce, 191945},
	{0x3e2, 193838},
	{0x3f6, 195694},
	{0x40a, 197584},
	{0x420, 199517},
	{0x435, 201412},
	{0x44a, 203259},
	{0x460, 205081},
	{0x475, 206868},
	{0x48a, 208612},
	{0x4a0, 210335},
	{0x4b5, 212026},
	{0x4ca, 213679},
	{0x4e0, 215312},
	{0x4f5, 216918},
	{0x50a, 218487},
	{0x520, 220041},
	{0x535, 221569},
	{0x54a, 223063},
	{0x560, 224544},
	{0x575, 226001},
	{0x58b, 227496},
	{0x5a2, 229035},
	{0x5ba, 230549},
	{0x5d1, 232032},
	{0x5e8, 233499},
	{0x600, 234944},
	{0x617, 236367},
	{0x62e, 237769},
	{0x645, 239144},
	{0x65d, 240505},
	{0x674, 241848},
	{0x68b, 243172},
	{0x6a2, 244477},
	{0x6ba, 245765},
	{0x6d1, 247028},
	{0x6e8, 248282},
	{0x700, 249519},
	{0x717, 250740},
	{0x72e, 251946},
	{0x745, 253130},
	{0x75d, 254305},
	{0x774, 255467},
	{0x78b, 256614},
	{0x7a2, 257747},
	{0x7c0, 259142},
};

struct tx_isp_sensor_attribute ov2740_attr;

unsigned int ov2740_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = ov2740_again_lut;

	while(lut->gain <= ov2740_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut->value;
			return 0;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == ov2740_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int ov2740_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus ov2740_mipi_linear = {
	.clk = 370,
	.lans = 2,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,
	.mipi_sc.hcrop_diff_en = 0,
	.mipi_sc.mipi_vcomp_en = 0,
	.mipi_sc.mipi_hcomp_en = 0,
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

struct tx_isp_sensor_attribute ov2740_attr={
	.name = "ov2740",
	.chip_id = 0x2740,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x36,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.max_again = 259142,
	.max_dgain = 0,
	.min_integration_time = 2,
	.min_integration_time_native = 2,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = ov2740_alloc_again,
	.sensor_ctrl.alloc_dgain = ov2740_alloc_dgain,
};

static struct regval_list ov2740_init_regs_1920_1080_30fps_mipi[] = {
	{0x0103,0x01},
	{0x0302,0x1e},
	{0x0303,0x01},
	{0x030d,0x1e},
	{0x030e,0x02},
	{0x0312,0x01},
	{0x3000,0x00},
	{0x3018,0x32},
	{0x3031,0x0a},
	{0x3080,0x08},
	{0x3083,0xb4},
	{0x3103,0x00},
	{0x3104,0x01},
	{0x3106,0x01},
	{0x3500,0x00},
	{0x3501,0x44},
	{0x3502,0x40},
	{0x3503,0x88},
	{0x3507,0x00},
	{0x3508,0x00},
	{0x3509,0x80},
	{0x350c,0x00},
	{0x350d,0x80},
	{0x3510,0x00},
	{0x3511,0x00},
	{0x3512,0x20},
	{0x3632,0x00},
	{0x3633,0x10},
	{0x3634,0x10},
	{0x3635,0x10},
	{0x3645,0x13},
	{0x3646,0x81},
	{0x3636,0x10},
	{0x3651,0x0a},
	{0x3656,0x02},
	{0x3659,0x04},
	{0x365a,0xda},
	{0x365b,0xa2},
	{0x365c,0x04},
	{0x365d,0x1d},
	{0x365e,0x1a},
	{0x3662,0xd7},
	{0x3667,0x78},
	{0x3669,0x0a},
	{0x366a,0x92},
	{0x3700,0x54},
	{0x3702,0x10},
	{0x3706,0x42},
	{0x3709,0x30},
	{0x370b,0xc2},
	{0x3714,0x63},
	{0x3715,0x01},
	{0x3716,0x00},
	{0x371a,0x3e},
	{0x3732,0x0e},
	{0x3733,0x10},
	{0x375f,0x0e},
	{0x3768,0x30},
	{0x3769,0x44},
	{0x376a,0x22},
	{0x377b,0x20},
	{0x377c,0x00},
	{0x377d,0x0c},
	{0x3798,0x00},
	{0x37a1,0x55},
	{0x37a8,0x6d},
	{0x37c2,0x04},
	{0x37c5,0x00},
	{0x37c8,0x00},
	{0x3800,0x00},
	{0x3801,0x00},
	{0x3802,0x00},
	{0x3803,0x00},
	{0x3804,0x07},
	{0x3805,0x8f},
	{0x3806,0x04},
	{0x3807,0x47},
	{0x3808,0x07},
	{0x3809,0x80},
	{0x380a,0x04},
	{0x380b,0x38},
	{0x380c,0x08},/*hts*/
	{0x380d,0x70},
	{0x380e,0x05},/*vts*/
	{0x380f,0x40},
	{0x3810,0x00},
	{0x3811,0x08},
	{0x3812,0x00},
	{0x3813,0x08},
	{0x3814,0x01},
	{0x3815,0x01},
	{0x3820,0x80},
	{0x3821,0x46},
	{0x3822,0x84},
	{0x3829,0x00},
	{0x382a,0x01},
	{0x382b,0x01},
	{0x3830,0x04},
	{0x3836,0x01},
	{0x3837,0x08},
	{0x3839,0x01},
	{0x383a,0x00},
	{0x383b,0x08},
	{0x383c,0x00},
	{0x3f0b,0x00},
	{0x4001,0x20},
	{0x4009,0x07},
	{0x4003,0x10},
	{0x4010,0xe0},
	{0x4016,0x00},
	{0x4017,0x10},
	{0x4044,0x02},
	{0x4304,0x08},
	{0x4307,0x30},
	{0x4320,0x80},
	{0x4322,0x00},
	{0x4323,0x00},
	{0x4324,0x00},
	{0x4325,0x00},
	{0x4326,0x00},
	{0x4327,0x00},
	{0x4328,0x00},
	{0x4329,0x00},
	{0x432c,0x03},
	{0x432d,0x81},
	{0x4501,0x84},
	{0x4502,0x40},
	{0x4503,0x18},
	{0x4504,0x04},
	{0x4508,0x02},
	{0x4601,0x10},
	{0x4800,0x00},
	{0x4816,0x52},
	{0x4837,0x16},
	{0x5000,0x43},
	{0x5001,0x00},
	{0x5005,0x38},
	{0x501e,0x0d},
	{0x5040,0x00},
	{0x5901,0x00},
	{0x0100,0x01},

	{OV2740_REG_END, 0x00},	/* END MARKER */
};

/*
 * the order of the ov2740_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting ov2740_win_sizes[] = {
	/* 1920*1080_mipi*/
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= ov2740_init_regs_1920_1080_30fps_mipi,
	}
};
struct tx_isp_sensor_win_setting *wsize = &ov2740_win_sizes[0];

static struct regval_list ov2740_stream_on_mipi[] = {
	{OV2740_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list ov2740_stream_off_mipi[] = {
	{OV2740_REG_END, 0x00},	/* END MARKER */
};

int ov2740_read(struct tx_isp_subdev *sd, uint16_t reg,
	unsigned char *value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	uint8_t buf[2] = {(reg>>8)&0xff, reg&0xff};
	int ret;
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

static int ov2740_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
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
static int ov2740_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != OV2740_REG_END) {
		if (vals->reg_num == OV2740_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = ov2740_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		pr_debug("vals->reg_num:0x%02x, vals->value:0x%02x\n",vals->reg_num, val);
		vals++;
	}
	return 0;
}
#endif

static int ov2740_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != OV2740_REG_END) {
		if (vals->reg_num == OV2740_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = ov2740_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}
	return 0;
}

static int ov2740_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int ov2740_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	unsigned char v;
	int ret;
	ret = ov2740_read(sd, 0x300b, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != OV2740_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = ov2740_read(sd, 0x300c, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != OV2740_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int ov2740_set_integration_time(struct tx_isp_subdev *sd, int value)
{

	int ret = 0;
	unsigned int expo = value << 4;

	ret = ov2740_write(sd, 0x3502, (unsigned char)(expo & 0xff));
	ret += ov2740_write(sd, 0x3501, (unsigned char)((expo >> 8) & 0xff));
	ret += ov2740_write(sd, 0x3500, (unsigned char)((expo >> 16) & 0xf));
	if (ret < 0)
		return ret;
	return 0;
}

static int ov2740_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = -1;

	ret = ov2740_write(sd, 0x3509, (unsigned char)(value & 0xff));
	ret += ov2740_write(sd, 0x3508, (unsigned char)((value >> 8) & 0xff));
	if (ret < 0)
		return ret;

	return 0;
}

static int ov2740_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int ov2740_get_black_pedestal(struct tx_isp_subdev *sd, int value)
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

static int ov2740_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int ov2740_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = ov2740_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if(sensor->video.state == TX_ISP_MODULE_INIT){

			ret = ov2740_write_array(sd, ov2740_stream_on_mipi);
			sensor->video.state = TX_ISP_MODULE_RUNNING;
			ISP_WARNING("ov2740 stream on\n");
		}
	} else {
		ret = ov2740_write_array(sd, ov2740_stream_off_mipi);
		sensor->video.state = TX_ISP_MODULE_INIT;
		ISP_WARNING("ov2740 stream off\n");
	}

	return ret;
}

static int ov2740_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	int ret = 0;
	unsigned int sclk = 0;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned char val = 0;

	unsigned int newformat = 0; //the format is 24.8
	/* the format of fps is 16/16. for example 25 << 16 | 2, the value is 25/2 fps. */
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (SENSOR_OUTPUT_MAX_FPS << 8) || fps < (SENSOR_OUTPUT_MIN_FPS << 8)) {
		ISP_ERROR("warn: fps(%d) no in range\n", fps);
		return -1;
	}

	switch(info->default_boot){
		case 0:
			sclk = OV2740_SUPPORT_SCLK_MIPI;
			break;
		default:
			ISP_ERROR("Have no this setting!!!\n");
	}

	ret += ov2740_read(sd, 0x380c, &val);
	hts = val;
	ret += ov2740_read(sd, 0x380d, &val);
	if (0 != ret) {
		ISP_ERROR("err: ov2740 read err\n");
		return ret;
	}
	hts = ((hts << 8) + val);

	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);
	ret += ov2740_write(sd, 0x380f, vts & 0xff);
	ret += ov2740_write(sd, 0x380e, (vts >> 8) & 0xff);
	if (0 != ret) {
		ISP_ERROR("err: ov2740_write err\n");
		return ret;
	}

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts -4;
	sensor->video.attr->integration_time_limit = vts -4;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts -4;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int ov2740_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = ISP_SUCCESS;

	if(wsize){
		sensor_set_attr(sd, wsize);
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	}

	return ret;
}

#if 0
static int ov2740_frame_sync(struct tx_isp_subdev *sd, struct tx_isp_frame_sync *sync)
{
	ov2740_write(sd, 0x381d, 0x00);
	private_msleep(1);
	ov2740_write(sd, 0x3816, 0x00);
	private_msleep(1);
	ov2740_write(sd, 0x3817, 0x00);
	private_msleep(1);
	ov2740_write(sd, 0x3818, 0x00);
	private_msleep(1);
	ov2740_write(sd, 0x3819, 0x01);
	private_msleep(1);
	ov2740_write(sd, 0x3001, 0x04);
	private_msleep(1);
	ov2740_write(sd, 0x3007, 0x44);
	private_msleep(1);

	return 0;
}
#endif

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
		wsize = &ov2740_win_sizes[0];
		memcpy(&ov2740_attr.mipi, &ov2740_mipi_linear, sizeof(ov2740_mipi_linear));
		ov2740_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		ov2740_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
		ov2740_attr.max_integration_time_native = 1340,
		ov2740_attr.integration_time_limit = 1340,
		ov2740_attr.total_width = 2160,
		ov2740_attr.total_height = 1344,
		ov2740_attr.max_integration_time = 1340,
		ov2740_attr.one_line_expr_in_us = 30;
		break;
	default:
		ISP_ERROR("Have no this setting!!!\n");
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		ov2740_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		ov2740_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		ov2740_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		ov2740_attr.mipi.index = 1;
		break;
	case TISP_SENSOR_VI_DVP:
		ov2740_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
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

    private_clk_set_rate(sensor->mclk, MCLK);
    private_clk_prepare_enable(sensor->mclk);

	reset_gpio = info->rst_gpio;
	pwdn_gpio = info->pwdn_gpio;

	sensor_set_attr(sd, wsize);
	sensor->priv = wsize;

	return 0;

err_get_mclk:
	return -1;
}

static int ov2740_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"ov2740_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(20);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(20);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"ov2740_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(150);
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}

		ret = ov2740_detect(sd, &ident);

	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an ov2740 chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("ov2740 chip found @ 0x%02x (%s) version %s \n", client->addr, client->adapter->name, SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "ov2740", sizeof("ov2740"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}
	return 0;
}
static int ov2740_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
{
	long ret = 0;
	struct tx_isp_sensor_value *sensor_val = arg;

	if(IS_ERR_OR_NULL(sd)){
		ISP_ERROR("[%d]The pointer is invalid!\n", __LINE__);
		return -EINVAL;
	}
	switch(cmd){
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			ret = ov2740_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = ov2740_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = ov2740_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = ov2740_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = ov2740_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		ret = ov2740_write_array(sd, ov2740_stream_off_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		ret = ov2740_write_array(sd, ov2740_stream_on_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = ov2740_set_fps(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return 0;
}

static int ov2740_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = ov2740_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int ov2740_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	ov2740_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops ov2740_core_ops = {
	.g_chip_ident = ov2740_g_chip_ident,
	.reset = ov2740_reset,
	.init = ov2740_init,
//	.fsync = ov2740_frame_sync,
	.g_register = ov2740_g_register,
	.s_register = ov2740_s_register,
};

static struct tx_isp_subdev_video_ops ov2740_video_ops = {
	.s_stream = ov2740_s_stream,
};

static struct tx_isp_subdev_sensor_ops	ov2740_sensor_ops = {
	.ioctl	= ov2740_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops ov2740_ops = {
	.core = &ov2740_core_ops,
	.video = &ov2740_video_ops,
	.sensor = &ov2740_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "ov2740",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int ov2740_probe(struct i2c_client *client,
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
	sensor->dev = &client->dev;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->video.attr = &ov2740_attr;
	tx_isp_subdev_init(&sensor_platform_device, sd, &ov2740_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	ISP_WARNING("@@@@@@@probe ok ------->ov2740\n");

	return 0;
}

static int ov2740_remove(struct i2c_client *client)
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

static const struct i2c_device_id ov2740_id[] = {
	{ "ov2740", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ov2740_id);

static struct i2c_driver ov2740_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ov2740",
	},
	.probe		= ov2740_probe,
	.remove		= ov2740_remove,
	.id_table	= ov2740_id,
};

static __init int init_ov2740(void)
{
	return private_i2c_add_driver(&ov2740_driver);
}

static __exit void exit_ov2740(void)
{
	private_i2c_del_driver(&ov2740_driver);
}

module_init(init_ov2740);
module_exit(exit_ov2740);

MODULE_DESCRIPTION("A low-level driver for OmniVision ov2740 sensors");
MODULE_LICENSE("GPL");
