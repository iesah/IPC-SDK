/*
 * sc4335.c
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

#define SC4335_CHIP_ID_H	(0xcd)
#define SC4335_CHIP_ID_L	(0x01)
#define SC4335_REG_END		0xffff
#define SC4335_REG_DELAY	0xfffe
#define SC4335_SUPPORT_30FPS_SCLK (121500000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20190906a"

static int reset_gpio = GPIO_PC(28);
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Reset GPIO NUM");

static int pwdn_gpio = -1;
module_param(pwdn_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwdn_gpio, "Power down GPIO NUM");

static int data_interface = TX_SENSOR_DATA_INTERFACE_MIPI;
module_param(data_interface, int, S_IRUGO);
MODULE_PARM_DESC(data_interface, "Sensor Date interface");

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

struct again_lut sc4335_again_lut[] = {
	{0x320, 0},
	{0x321, 2886},
	{0x322, 5776},
	{0x323, 8494},
	{0x324, 11136},
	{0x325, 13706},
	{0x326, 16287},
	{0x327, 18723},
	{0x328, 21097},
	{0x329, 23414},
	{0x32a, 25746},
	{0x32b, 27953},
	{0x32c, 30109},
	{0x32d, 32217},
	{0x32e, 34345},
	{0x32f, 36361},
	{0x330, 38336},
	{0x331, 40270},
	{0x332, 42226},
	{0x333, 44082},
	{0x334, 45904},
	{0x335, 47690},
	{0x336, 49500},
	{0x337, 51220},
	{0x338, 52910},
	{0x339, 54571},
	{0x33a, 56254},
	{0x33b, 57857},
	{0x33c, 59433},
	{0x33d, 60984},
	{0x33e, 62558},
	{0x33f, 64059},
	{0x720, 65536},
	{0x721, 68468},
	{0x722, 71267},
	{0x723, 74030},
	{0x724, 76672},
	{0x725, 79283},
	{0x726, 81784},
	{0x727, 84259},
	{0x728, 86633},
	{0x729, 89538},
	{0x72a, 91246},
	{0x72b, 93489},
	{0x72c, 95645},
	{0x72d, 97686},
	{0x72e, 99848},
	{0x72f, 101961},
	{0x730, 103872},
	{0x731, 105744},
	{0x732, 107731},
	{0x733, 109678},
	{0x734, 111440},
	{0x735, 113169},
	{0x736, 115008},
	{0x737, 116811},
	{0x738, 118446},
	{0x739, 120053},
	{0x73a, 121764},
	{0x73b, 123444},
	{0x73c, 124969},
	{0x73d, 126470},
	{0x73e, 128070},
	{0x73f, 129643},
	{0xf20, 131072},
	{0xf21, 134095},
	{0xf22, 136803},
	{0xf23, 139652},
	{0xf24, 142208},
	{0xf25, 144900},
	{0xf26, 147320},
	{0xf27, 149873},
	{0xf28, 152169},
	{0xf29, 154596},
	{0xf2a, 156782},
	{0xf2b, 159095},
	{0xf2c, 161181},
	{0xf2d, 163390},
	{0xf2e, 165384},
	{0xf2f, 167497},
	{0xf30, 169408},
	{0xf31, 171434},
	{0xf32, 173267},
	{0xf33, 175214},
	{0xf34, 176976},
	{0xf35, 178848},
	{0xf36, 180544},
	{0xf37, 182347},
	{0xf38, 183982},
	{0xf39, 185722},
	{0xf3a, 187300},
	{0xf3b, 188980},
	{0xf3c, 190505},
	{0xf3d, 192130},
	{0xf3e, 193606},
	{0xf3f, 195179},
	{0x1f20, 196608},
	{0x1f21, 199517},
	{0x1f22, 202339},
	{0x1f23, 205080},
	{0x1f24, 207744},
	{0x1f25, 210334},
	{0x1f26, 212856},
	{0x1f27, 215312},
	{0x1f28, 217705},
	{0x1f29, 220040},
	{0x1f2a, 222318},
	{0x1f2b, 224543},
	{0x1f2c, 226717},
	{0x1f2d, 228842},
	{0x1f2e, 230920},
	{0x1f2f, 232953},
	{0x1f30, 234944},
	{0x1f31, 236893},
	{0x1f32, 238803},
	{0x1f33, 240676},
	{0x1f34, 242512},
	{0x1f35, 244313},
	{0x1f36, 246080},
	{0x1f37, 247815},
	{0x1f38, 249518},
	{0x1f39, 251192},
	{0x1f3a, 252836},
	{0x1f3b, 254452},
	{0x1f3c, 256041},
	{0x1f3d, 257604},
	{0x1f3e, 259142},
	{0x1f3f, 260655},
};

struct tx_isp_sensor_attribute sc4335_attr;

unsigned int sc4335_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = sc4335_again_lut;
	while(lut->gain <= sc4335_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return 0;
		} else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		} else {
			if((lut->gain == sc4335_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int sc4335_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_sensor_attribute sc4335_attr={
	.name = "sc4335",
	.chip_id = 0xcd01,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x30,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.mode = SENSOR_MIPI_OTHER_MODE,
		.clk = 608,
		.lans = 2,
		.settle_time_apative_en = 0,
		.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
		.mipi_sc.hcrop_diff_en = 0,
		.mipi_sc.mipi_vcomp_en = 0,
		.mipi_sc.mipi_hcomp_en = 0,
		.mipi_sc.line_sync_mode = 0,
		.mipi_sc.work_start_flag = 0,
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
		.mipi_sc.data_type_en = 0,
		.mipi_sc.data_type_value = RAW10,
		.mipi_sc.del_start = 0,
		.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
		.mipi_sc.sensor_fid_mode = 0,
		.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
	},
	.data_type = TX_SENSOR_DATA_TYPE_LINEAR,
	.max_again = 260655,
	.max_dgain = 0,
	.min_integration_time = 2,
	.min_integration_time_native = 2,
	.max_integration_time_native = 1800 - 4,
	.integration_time_limit = 1800 - 4,
	.total_width = 2700,
	.total_height = 1800,
	.max_integration_time = 1800 - 4,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = sc4335_alloc_again,
	.sensor_ctrl.alloc_dgain = sc4335_alloc_dgain,
};


static struct regval_list sc4335_init_regs_2560_1440_25fps_mipi[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x36e9, 0xd4}, //1113
	{0x36f9, 0xd4},
	{0x301f, 0x21},
	{0x3213, 0x06},
	{0x3301, 0x10},
	{0x3306, 0x60},
	{0x3309, 0x88},
	{0x330a, 0x01},
	{0x330b, 0x08},
	{0x330e, 0x38},
	{0x330f, 0x04},
	{0x3310, 0x20},
	{0x3314, 0x94},
	{0x331f, 0x79},
	{0x3342, 0x01},
	{0x3347, 0x05},
	{0x3364, 0x1d},
	{0x3367, 0x10},
	{0x33b6, 0x07},
	{0x33b7, 0x2f},
	{0x33b8, 0x10},
	{0x33b9, 0x18},
	{0x33ba, 0x70},
	{0x360f, 0x05},
	{0x3614, 0x80},
	{0x3620, 0xa8},
	{0x3622, 0xf6},
	{0x3625, 0x0a},
	{0x3630, 0xc0},
	{0x3631, 0x88},
	{0x3632, 0x18},
	{0x3633, 0x33},
	{0x3636, 0x25},
	{0x3637, 0x70},
	{0x3638, 0x22},
	{0x363a, 0x90},
	{0x363b, 0x09},
	{0x3650, 0x06},
	{0x366e, 0x04},
	{0x3670, 0x0a},
	{0x3671, 0xf6},
	{0x3672, 0xf6},
	{0x3673, 0x16},
	{0x3674, 0xc0},
	{0x3675, 0xc8},
	{0x3676, 0xaf},
	{0x367a, 0x08},
	{0x367b, 0x38},
	{0x367c, 0x38},
	{0x367d, 0x3f},
	{0x3690, 0x33},
	{0x3691, 0x34},
	{0x3692, 0x44},
	{0x369c, 0x38},
	{0x369d, 0x3f},
	{0x36ea, 0x65},
	{0x36ed, 0x03},
	{0x36fa, 0x65},
	{0x36fd, 0x04},
	{0x3902, 0xc5},
	{0x3904, 0x10},
	{0x3908, 0x41},
	{0x3933, 0x0a},
	{0x3934, 0x0d},
	{0x3940, 0x65},
	{0x3941, 0x18},
	{0x3942, 0x02},
	{0x3943, 0x12},
	{0x395e, 0xa0},
	{0x3960, 0x9d},
	{0x3961, 0x9d},
	{0x3962, 0x89},
	{0x3963, 0x80},
	{0x3966, 0x4e},
	{0x3980, 0x60},
	{0x3981, 0x30},
	{0x3982, 0x15},
	{0x3983, 0x10},
	{0x3984, 0x0d},
	{0x3985, 0x20},
	{0x3986, 0x30},
	{0x3987, 0x60},
	{0x3988, 0x04},
	{0x3989, 0x0c},
	{0x398a, 0x14},
	{0x398b, 0x24},
	{0x398c, 0x50},
	{0x398d, 0x32},
	{0x398e, 0x1e},
	{0x398f, 0x0a},
	{0x3990, 0xc0},
	{0x3991, 0x50},
	{0x3992, 0x22},
	{0x3993, 0x0c},
	{0x3994, 0x10},
	{0x3995, 0x38},
	{0x3996, 0x80},
	{0x3997, 0xff},
	{0x3998, 0x08},
	{0x3999, 0x16},
	{0x399a, 0x28},
	{0x399b, 0x40},
	{0x399c, 0x50},
	{0x399d, 0x28},
	{0x399e, 0x18},
	{0x399f, 0x0c},
	{0x3e01, 0xbb},
	{0x3e02, 0x00},
	{0x3e09, 0x20},
	{0x3e25, 0x03},
	{0x3e26, 0x20},
	{0x5781, 0x04},
	{0x5782, 0x04},
	{0x5783, 0x02},
	{0x5784, 0x02},
	{0x5785, 0x40},
	{0x5786, 0x20},
	{0x5787, 0x18},
	{0x5788, 0x10},
	{0x5789, 0x10},
	{0x57a4, 0xa0},
	{0x320e, 0x07},//05 for 30fps
	{0x320f, 0x08},//dc
	{0x36e9, 0x52},
	{0x36f9, 0x53},
	{SC4335_REG_END, 0x00},	/* END MARKER */
};

static struct tx_isp_sensor_win_setting sc4335_win_sizes[] = {
	{
		.width		= 2560,
		.height		= 1440,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc4335_init_regs_2560_1440_25fps_mipi,
	},
};
struct tx_isp_sensor_win_setting *wsize = &sc4335_win_sizes[0];

static struct regval_list sc4335_stream_on_mipi[] = {
	{0x0100, 0x01},
	{SC4335_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc4335_stream_off_mipi[] = {
	{0x0100, 0x00},
	{SC4335_REG_END, 0x00},	/* END MARKER */
};

int sc4335_read(struct tx_isp_subdev *sd, uint16_t reg, unsigned char *value)
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

int sc4335_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
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
static int sc4335_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != SC4335_REG_END) {
		if (vals->reg_num == SC4335_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc4335_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}
#endif

static int sc4335_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != SC4335_REG_END) {
		if (vals->reg_num == SC4335_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc4335_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int sc4335_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int sc4335_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = sc4335_read(sd, 0x3107, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC4335_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = sc4335_read(sd, 0x3108, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC4335_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int sc4335_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	int it = (value & 0xffff) * 2;
	int again = (value & 0xffff0000) >> 16;

	ret += sc4335_write(sd, 0x3e00, (unsigned char)((it >> 12) & 0xf));
	ret += sc4335_write(sd, 0x3e01, (unsigned char)((it >> 4) & 0xff));
	ret += sc4335_write(sd, 0x3e02, (unsigned char)((it & 0x0f) << 4));
	ret = sc4335_write(sd, 0x3e09, (unsigned char)(again & 0xff));
	ret += sc4335_write(sd, 0x3e08, (unsigned char)(((again >> 8) & 0xff)));

	if (ret < 0)
		return ret;

//	sc4335_write(sd,0x3812,0x00);
	/* denoise logic */
	if (again < 0x720) { //<2
		sc4335_write(sd,0x3632,0x18);
		sc4335_write(sd,0x3631,0x88);
		sc4335_write(sd,0x3636,0x25);
	} else if (again >= 0x720 &&  again< 0xf20){//>=2 <4
		sc4335_write(sd,0x3632,0x18);
		sc4335_write(sd,0x3631,0x8e);
		sc4335_write(sd,0x3636,0x25);
	} else if (again >= 0xf20 &&  again< 0x1f20){//>=2 <8
		sc4335_write(sd,0x3632,0x18);
		sc4335_write(sd,0x3631,0x80);
		sc4335_write(sd,0x3636,0x65);
	} else { ////>=8x
		sc4335_write(sd,0x3632,0xd8);
		sc4335_write(sd,0x3631,0x80);
		sc4335_write(sd,0x3636,0x65);
	}
	//sc4335_write(sd,0x3812,0x30);

	return 0;
}

#if 0
static int sc4335_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	value *= 2;
	ret += sc4335_write(sd, 0x3e00, (unsigned char)((value >> 12) & 0xf));
	ret += sc4335_write(sd, 0x3e01, (unsigned char)((value >> 4) & 0xff));
	ret += sc4335_write(sd, 0x3e02, (unsigned char)((value & 0x0f) << 4));

	if (ret < 0)
		return ret;

	return 0;
}

static int sc4335_set_analog_gain(struct tx_isp_subdev *sd, int value)
{

	int ret = 0;

	/* denoise logic */
	sc4335_write(sd,0x3812,0x00);
	if (value < 0x720) { //<2
		sc4335_write(sd,0x3632,0x18);
		sc4335_write(sd,0x3631,0x88);
		sc4335_write(sd,0x3636,0x25);
	} else if (value >= 0x720 && value < 0xf20){//>=2 <4
		sc4335_write(sd,0x3632,0x18);
		sc4335_write(sd,0x3631,0x8e);
		sc4335_write(sd,0x3636,0x25);
	} else if (value >= 0xf20 && value < 0x1f20){//>=2 <8
		sc4335_write(sd,0x3632,0x18);
		sc4335_write(sd,0x3631,0x80);
		sc4335_write(sd,0x3636,0x65);
	} else { ////>=8x
		sc4335_write(sd,0x3632,0xd8);
		sc4335_write(sd,0x3631,0x80);
		sc4335_write(sd,0x3636,0x65);
	}
	sc4335_write(sd,0x3812,0x30);

	ret = sc4335_write(sd, 0x3e09, (unsigned char)(value & 0xff));
	ret += sc4335_write(sd, 0x3e08, (unsigned char)(((value >> 8) & 0xff)));
	if (ret < 0)
		return ret;

	return 0;
}
#endif

static int sc4335_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc4335_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc4335_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int sc4335_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
	    if (sensor->video.state == TX_ISP_MODULE_DEINIT){
            ret = sc4335_write_array(sd, wsize->regs);
            if (ret)
                return ret;
            sensor->video.state = TX_ISP_MODULE_INIT;
	    }
	    if (sensor->video.state == TX_ISP_MODULE_INIT){
            if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
                ret = sc4335_write_array(sd, sc4335_stream_on_mipi);
            } else {
                ISP_ERROR("Don't support this Sensor Data interface\n");
            }
            ISP_WARNING("sc4335 stream on\n");
            sensor->video.state = TX_ISP_MODULE_RUNNING;
	    }
	}
	else {
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc4335_write_array(sd, sc4335_stream_off_mipi);
		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		ISP_WARNING("sc4335 stream off\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
	}

	return ret;
}

static int sc4335_set_fps(struct tx_isp_subdev *sd, int fps)
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
	sclk = SC4335_SUPPORT_30FPS_SCLK;

	ret = sc4335_read(sd, 0x320c, &tmp);
	hts = tmp;
	ret += sc4335_read(sd, 0x320d, &tmp);
	if (0 != ret) {
		ISP_ERROR("err: sc4335 read err\n");
		return ret;
	}
	hts = ((hts << 8) + tmp);
	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);

//	ret = sc4335_write(sd,0x3812,0x00);
	ret += sc4335_write(sd, 0x320f, (unsigned char)(vts & 0xff));
	ret += sc4335_write(sd, 0x320e, (unsigned char)(vts >> 8));
//	ret += sc4335_write(sd,0x3812,0x30);
	if (0 != ret) {
		ISP_ERROR("err: sc4335_write err\n");
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

static int sc4335_set_mode(struct tx_isp_subdev *sd, int value)
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

            break;
        default:
            ISP_ERROR("Have no this setting!!!\n");
    }

    switch(info->video_interface){
        case TISP_SENSOR_VI_MIPI_CSI0:
            sc4335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            sc4335_attr.mipi.index = 0;
            break;
        case TISP_SENSOR_VI_MIPI_CSI1:
            sc4335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            sc4335_attr.mipi.index = 1;
            break;
        case TISP_SENSOR_VI_DVP:
            sc4335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
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

static int sc4335_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;
	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"sc4335_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(10);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"sc4335_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = sc4335_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an sc4335 chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("sc4335 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "sc4335", sizeof("sc4335"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int sc4335_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = sc4335_set_expo(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME:
//		if(arg)
		//ret = sc4335_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
//		if(arg)
//			ret = sc4335_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = sc4335_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = sc4335_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = sc4335_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc4335_write_array(sd, sc4335_stream_off_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc4335_write_array(sd, sc4335_stream_on_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
			ret = -1;
		}
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = sc4335_set_fps(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return ret;
}

static int sc4335_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = sc4335_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int sc4335_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;

	sc4335_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops sc4335_core_ops = {
	.g_chip_ident = sc4335_g_chip_ident,
	.reset = sc4335_reset,
	.init = sc4335_init,
	/*.ioctl = sc4335_ops_ioctl,*/
	.g_register = sc4335_g_register,
	.s_register = sc4335_s_register,
};

static struct tx_isp_subdev_video_ops sc4335_video_ops = {
	.s_stream = sc4335_s_stream,
};

static struct tx_isp_subdev_sensor_ops	sc4335_sensor_ops = {
	.ioctl	= sc4335_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops sc4335_ops = {
	.core = &sc4335_core_ops,
	.video = &sc4335_video_ops,
	.sensor = &sc4335_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "sc4335",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int sc4335_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	sensor->video.attr = &sc4335_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &sc4335_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->sc4335\n");

	return 0;
}

static int sc4335_remove(struct i2c_client *client)
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

static const struct i2c_device_id sc4335_id[] = {
	{ "sc4335", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sc4335_id);

static struct i2c_driver sc4335_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "sc4335",
	},
	.probe		= sc4335_probe,
	.remove		= sc4335_remove,
	.id_table	= sc4335_id,
};

static __init int init_sc4335(void)
{
	return private_i2c_add_driver(&sc4335_driver);
}

static __exit void exit_sc4335(void)
{
	private_i2c_del_driver(&sc4335_driver);
}

module_init(init_sc4335);
module_exit(exit_sc4335);

MODULE_DESCRIPTION("A low-level driver for OmniVision sc4335 sensors");
MODULE_LICENSE("GPL");
