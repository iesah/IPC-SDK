/*
 * mis2008.c
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

#define MIS2008_CHIP_ID_H	(0x20)
#define MIS2008_CHIP_ID_L	(0x08)
#define MIS2008_REG_END		0xffff
#define MIS2008_REG_DELAY	0xfffe
#define MIS2008_SUPPORT_PCLK (72000000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20220115a"

static int reset_gpio = GPIO_PC(27);
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Reset GPIO NUM");

static int pwdn_gpio = -1;
module_param(pwdn_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwdn_gpio, "Power down GPIO NUM");

static int sensor_gpio_func = DVP_PA_12BIT;
module_param(sensor_gpio_func, int, S_IRUGO);
MODULE_PARM_DESC(sensor_gpio_func, "Sensor GPIO function");

static int data_interface = TX_SENSOR_DATA_INTERFACE_MIPI;
module_param(data_interface, int, S_IRUGO);
MODULE_PARM_DESC(data_interface, "Sensor Date interface");

static int sensor_max_fps = TX_SENSOR_MAX_FPS_30;
module_param(sensor_max_fps, int, S_IRUGO);
MODULE_PARM_DESC(sensor_max_fps, "Sensor Max Fps set interface");

struct regval_list {
	unsigned int reg_num;
	unsigned int value;
};

/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
	unsigned int value;
	unsigned int gain;
};

struct again_lut mis2008_again_lut[] = {
	{0x0, 0},
	{0x1, 2909},
	{0x2, 5731},
	{0x3, 8472},
	{0x4, 11136},
	{0x5, 13726},
	{0x6, 16247},
	{0x7, 18703},
	{0x8, 21097},
	{0x9, 23432},
	{0xa, 25710},
	{0xb, 27935},
	{0xc, 30108},
	{0xd, 32233},
	{0xe, 34311},
	{0xf, 36344},
	{0x10, 38335},
	{0x11, 40285},
	{0x12, 42195},
	{0x13, 44067},
	{0x14, 45903},
	{0x15, 47704},
	{0x16, 49471},
	{0x17, 51206},
	{0x18, 52910},
	{0x19, 54583},
	{0x1a, 56227},
	{0x1b, 57844},
	{0x1c, 59433},
	{0x1d, 60995},
	{0x1e, 62533},
	{0x1f, 64046},
	{0x20, 65535},
	{0x21, 68444},
	{0x22, 71266},
	{0x23, 74007},
	{0x24, 76671},
	{0x25, 79261},
	{0x26, 81782},
	{0x27, 84238},
	{0x28, 86632},
	{0x29, 88967},
	{0x2a, 91245},
	{0x2b, 93470},
	{0x2c, 95643},
	{0x2d, 97768},
	{0x2e, 99846},
	{0x2f, 101879},
	{0x30, 103870},
	{0x31, 105820},
	{0x32, 107730},
	{0x33, 109602},
	{0x34, 111438},
	{0x35, 113239},
	{0x36, 115006},
	{0x37, 116741},
	{0x38, 118445},
	{0x39, 120118},
	{0x3a, 121762},
	{0x3b, 123379},
	{0x3c, 124968},
	{0x3d, 126530},
	{0x3e, 128068},
	{0x3f, 129581},
	{0x40, 131070},
	{0x41, 133979},
	{0x42, 136801},
	{0x43, 139542},
	{0x44, 142206},
	{0x45, 144796},
	{0x46, 147317},
	{0x47, 149773},
	{0x48, 152167},
	{0x49, 154502},
	{0x4a, 156780},
	{0x4b, 159005},
	{0x4c, 161178},
	{0x4d, 163303},
	{0x4e, 165381},
	{0x4f, 167414},
	{0x50, 169405},
	{0x51, 171355},
	{0x52, 173265},
	{0x53, 175137},
	{0x54, 176973},
	{0x55, 178774},
	{0x56, 180541},
	{0x57, 182276},
	{0x58, 183980},
	{0x59, 185653},
	{0x5a, 187297},
	{0x5b, 188914},
	{0x5c, 190503},
	{0x5d, 192065},
	{0x5e, 193603},
	{0x5f, 195116},
	{0x60, 196605},
	{0x61, 199514},
	{0x62, 202336},
	{0x63, 205077},
	{0x64, 207741},
	{0x65, 210331},
	{0x66, 212852},
	{0x67, 215308},
	{0x68, 217702},
	{0x69, 220037},
	{0x6a, 222315},
	{0x6b, 224540},
	{0x6c, 226713},
	{0x6d, 228838},
	{0x6e, 230916},
	{0x6f, 232949},
	{0x70, 234940},
	{0x71, 236890},
	{0x72, 238800},
	{0x73, 240672},
	{0x74, 242508},
	{0x75, 244309},
	{0x76, 246076},
	{0x77, 247811},
	{0x78, 249515},
	{0x79, 251188},
	{0x7a, 252832},
	{0x7b, 254449},
	{0x7c, 256038},
	{0x7d, 257600},
	{0x7e, 259138},
      //{0x7f, 260651},
};

struct tx_isp_sensor_attribute mis2008_attr;

unsigned int mis2008_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = mis2008_again_lut;

	while(lut->gain <= mis2008_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = 0;
			return 0;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == mis2008_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int mis2008_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus mis2008_mipi={
	.mode = SENSOR_MIPI_OTHER_MODE,
	.clk = 800,
	.lans = 2,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW12,//RAW10
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
	.mipi_sc.data_type_value = RAW12,
	.mipi_sc.del_start = 0,
	.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
	.mipi_sc.sensor_fid_mode = 0,
	.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
};
struct tx_isp_dvp_bus mis2008_dvp={
	.mode = SENSOR_DVP_HREF_MODE,
	.blanking = {
		.vblanking = 0,
		.hblanking = 0,
	},
};

struct tx_isp_sensor_attribute mis2008_attr={
	.name = "mis2008",
	.chip_id = 0x2008,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_8BITS,
	.cbus_device = 0x30,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.clk = 800,
		.lans = 2,
	},
	.max_again = 259138,
	.max_dgain = 0,
	.min_integration_time = 2,
	.min_integration_time_native = 2,
	.max_integration_time_native = 1125 - 1,
	.integration_time_limit = 1125 - 1,
	.total_width = 2133,
	.total_height = 1125,
	.max_integration_time = 1125 - 1,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = mis2008_alloc_again,
	.sensor_ctrl.alloc_dgain = mis2008_alloc_dgain,
	//	void priv; /* point to struct tx_isp_sensor_board_info */
};


static struct regval_list mis2008_init_regs_1920_1080_30fps_mipi[] = {
	{0x300a,0x01},
	{0x3006,0x02},
	{0x3637,0x1e},
	{0x3c40,0x8d},
	{0x3c01,0x10},
	{0x3c0e,0xf7},
	{0x3c0f,0x34},
	{0x3b01,0x3f},
	{0x3b03,0x3f},
	{0x3902,0x01},
	{0x3904,0x00},
	{0x3903,0x00},
	{0x3906,0x1e},
	{0x3905,0x00},
	{0x3908,0xb0},
	{0x3907,0x10},
	{0x390a,0xff},
	{0x3909,0x1f},
	{0x390c,0x83},
	{0x390b,0x03},
	{0x390e,0x77},
	{0x390d,0x00},
	{0x3910,0xb0},
	{0x390f,0x10},
	{0x3912,0xff},
	{0x3911,0x1f},
	{0x3919,0x00},
	{0x3918,0x00},
	{0x391b,0xfd},
	{0x391a,0x00},
	{0x3983,0x5a},
	{0x3982,0x00},
	{0x3985,0x0f},
	{0x3984,0x00},
	{0x391d,0x00},
	{0x391c,0x00},
	{0x391f,0xa4},
	{0x391e,0x10},
	{0x3921,0xff},
	{0x3920,0x1f},
	{0x3923,0xff},
	{0x3922,0x1f},
	{0x3932,0x00},
	{0x3931,0x00},
	{0x3934,0xd0},
	{0x3933,0x00},
	{0x393f,0x61},
	{0x393e,0x00},
	{0x3941,0x89},
	{0x3940,0x00},
	{0x3943,0x16},
	{0x3942,0x01},
	{0x3945,0x10},
	{0x3944,0x03},
	{0x3925,0x95},
	{0x3924,0x00},
	{0x3927,0x3d},
	{0x3926,0x03},
	{0x3947,0xee},
	{0x3946,0x00},
	{0x3949,0x9e},
	{0x3948,0x0f},
	{0x394b,0x9e},
	{0x394a,0x03},
	{0x394d,0x9c},
	{0x394c,0x00},
	{0x3913,0x01},
	{0x3915,0x0f},
	{0x3914,0x00},
	{0x3917,0xc3},
	{0x3916,0x03},
	{0x392a,0x1e},
	{0x3929,0x00},
	{0x392c,0x0f},
	{0x392b,0x00},
	{0x392e,0x0f},
	{0x392d,0x00},
	{0x3930,0xca},
	{0x392f,0x03},
	{0x397f,0x00},
	{0x397e,0x00},
	{0x3981,0x77},
	{0x3980,0x00},
	{0x395d,0xbe},
	{0x395c,0x10},
	{0x3962,0xdc},
	{0x3961,0x10},
	{0x3977,0x22},
	{0x3976,0x00},
	{0x396d,0x10},
	{0x396c,0x03},
	{0x396f,0x10},
	{0x396e,0x03},
	{0x3971,0x10},
	{0x3970,0x03},
	{0x3973,0x10},
	{0x3972,0x03},
	{0x3978,0x00},
	{0x3979,0x04},

	{0x3012,0x01},
	{0x3600,0x13},
	{0x3601,0x02},
	{0x360f,0x00},
	{0x360e,0x00},
	{0x3610,0x02},//修改高温高增益blc跟随快慢
	{0x3707,0x00},
	{0x3708,0x40},
	{0x3709,0x00},
	{0x370a,0x40},
	{0x370b,0x00},
	{0x370c,0x40},
	{0x370d,0x00},
	{0x370e,0x40},
	{0x3800,0x01},
	{0x3a02,0x0b},
	{0x3a03,0x1b},
	{0x3a08,0x34},
	{0x3a1b,0x54},
	{0x3a1e,0x00},
	{0x3100,0x04},
	{0x3101,0x64},
	{0x3a1c,0x1f},
	{0x3a0C,0x04},
	{0x3a0D,0x12},
	{0x3a0E,0x15},
	{0x3a0F,0x18},
	{0x3a10,0x20},
	{0x3a11,0x3C},
	//MCLK=24Mhz,PCLK=72Mhz
	{0x3300,0x24},
	{0x3301,0x00},
	{0x3302,0x02},
	{0x3303,0x04},//0x06
	{0x330d,0x00},
	{0x330b,0x01},
	{0x330f,0x07},
	//Windows（2560*1125）
	{0x3201,0x65},
	{0x3200,0x04},
	{0x3203,0x55},
	{0x3202,0x08},
	{0x3205,0x04},
	{0x3204,0x00},
	{0x3207,0x3f},
	{0x3206,0x04},
	{0x3209,0x09},
	{0x3208,0x00},
	{0x320b,0x88},
	{0x320a,0x07},
	{0x3006,0x00},

	{MIS2008_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list mis2008_init_regs_1920_1080_30fps_dvp[] = {
	{0x300a,0x00},
	{0x3006,0x02},
	{0x3637,0x1e},
	{0x3c40,0x8d},
	{0x3c01,0x10},//
	{0x3c0e,0xf7},//
	{0x3c0f,0x34},//
	{0x3b01,0x3f},
	{0x3b03,0x3f},
	{0x3902,0x01},
	{0x3904,0x00},
	{0x3903,0x00},
	{0x3906,0x1e},
	{0x3905,0x00},
	{0x3908,0xb0},
	{0x3907,0x10},
	{0x390a,0xff},
	{0x3909,0x1f},
	{0x390c,0x83},
	{0x390b,0x03},
	{0x390e,0x77},
	{0x390d,0x00},
	{0x3910,0xb0},
	{0x390f,0x10},
	{0x3912,0xff},
	{0x3911,0x1f},
	{0x3919,0x00},
	{0x3918,0x00},
	{0x391b,0xfd},
	{0x391a,0x00},
	{0x3983,0x5a},
	{0x3982,0x00},
	{0x3985,0x0f},
	{0x3984,0x00},
	{0x391d,0x00},
	{0x391c,0x00},
	{0x391f,0xa4},
	{0x391e,0x10},
	{0x3921,0xff},
	{0x3920,0x1f},
	{0x3923,0xff},
	{0x3922,0x1f},
	{0x3932,0x00},
	{0x3931,0x00},
	{0x3934,0xd0},
	{0x3933,0x00},
	{0x393f,0x61},
	{0x393e,0x00},
	{0x3941,0x89},
	{0x3940,0x00},
	{0x3943,0x16},
	{0x3942,0x01},
	{0x3945,0x10},
	{0x3944,0x03},
	{0x3925,0x95},
	{0x3924,0x00},
	{0x3927,0x3d},
	{0x3926,0x03},
	{0x3947,0xee},
	{0x3946,0x00},
	{0x3949,0x9e},
	{0x3948,0x0f},
	{0x394b,0x9e},
	{0x394a,0x03},
	{0x394d,0x9c},
	{0x394c,0x00},
	{0x3913,0x01},
	{0x3915,0x0f},
	{0x3914,0x00},
	{0x3917,0xc3},
	{0x3916,0x03},
	{0x392a,0x1e},
	{0x3929,0x00},
	{0x392c,0x0f},
	{0x392b,0x00},
	{0x392e,0x0f},
	{0x392d,0x00},
	{0x3930,0xca},
	{0x392f,0x03},
	{0x397f,0x00},
	{0x397e,0x00},
	{0x3981,0x77},
	{0x3980,0x00},
	{0x395d,0xbe},
	{0x395c,0x10},
	{0x3962,0xdc},
	{0x3961,0x10},
	{0x3977,0x22},
	{0x3976,0x00},
	{0x396d,0x10},
	{0x396c,0x03},
	{0x396f,0x10},
	{0x396e,0x03},
	{0x3971,0x10},
	{0x3970,0x03},
	{0x3973,0x10},
	{0x3972,0x03},
	{0x3978,0x00},
	{0x3979,0x04},

	{0x3012,0x01},
	{0x3600,0x13},
	{0x3601,0x02},
	{0x3707,0x00},
	{0x3708,0x40},
	{0x3709,0x00},
	{0x370a,0x40},
	{0x370b,0x00},
	{0x370c,0x40},
	{0x370d,0x00},
	{0x370e,0x40},
	{0x3800,0x01},
	{0x3a02,0x0b},
	{0x3a03,0x1b},
	{0x3a08,0x34},
	{0x3a1b,0x54},
	{0x3a1e,0x00},
	{0x3100,0x04},
	{0x3101,0x64},
	{0x3a1c,0x1f},//
	{0x3a0C,0x04},//
	{0x3a0D,0x12},//
	{0x3a0E,0x15},//
	{0x3a0F,0x18},//
	{0x3a10,0x20},//
	{0x3a11,0x3C},//
	//MCLK=24Mhz,PCLK=72Mhz
	{0x3300,0x24},
	{0x3301,0x00},
	{0x3302,0x02},
	{0x3303,0x04},//0x06
	{0x330d,0x00},
	{0x330b,0x01},
	{0x330f,0x07},
	//Windows（2560*1125）
	{0x3201,0x65},
	{0x3200,0x04},
	{0x3203,0x00},//98
	{0x3202,0x0a},//08
	{0x3205,0x04},
	{0x3204,0x00},
	{0x3207,0x3f},
	{0x3206,0x04},
	{0x3209,0x09},
	{0x3208,0x00},
	{0x320b,0x88},
	{0x320a,0x07},
	{0x3006,0x00},

	{MIS2008_REG_END, 0x00},	/* END MARKER */
};

/*
 * the order of the mis2008_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting mis2008_win_sizes[] = {
	/* 1920*1080 */
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SGRBG12_1X12,//GRBG
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= mis2008_init_regs_1920_1080_30fps_mipi,
	},
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SGRBG12_1X12,//GRBG
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= mis2008_init_regs_1920_1080_30fps_dvp,
	}
};

struct tx_isp_sensor_win_setting *wsize = &mis2008_win_sizes[0];

/*
 * the part of driver was fixed.
 */

static struct regval_list mis2008_stream_on_dvp[] = {
	{MIS2008_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list mis2008_stream_off_dvp[] = {
	{MIS2008_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list mis2008_stream_on_mipi[] = {
	{MIS2008_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list mis2008_stream_off_mipi[] = {
	{MIS2008_REG_END, 0x00},	/* END MARKER */
};

int mis2008_read(struct tx_isp_subdev *sd, uint16_t reg,
	       unsigned char *value)
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

int mis2008_write(struct tx_isp_subdev *sd, uint16_t reg,
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
static int mis2008_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != MIS2008_REG_END) {
		if (vals->reg_num == MIS2008_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = mis2008_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		vals++;
	}
	return 0;
}
#endif

static int mis2008_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != MIS2008_REG_END) {
		if (vals->reg_num == MIS2008_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = mis2008_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}
	return 0;
}

static int mis2008_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int mis2008_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	unsigned char v;
	int ret;

	ret = mis2008_read(sd, 0x3000, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != MIS2008_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = mis2008_read(sd, 0x3001, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;

	if (v != MIS2008_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;
	return 0;
}

static int mis2008_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	ret = mis2008_write(sd,  0x3100, (unsigned char)((value >> 8)& 0xff));
	ret += mis2008_write(sd, 0x3101, (unsigned char)(value & 0xff));
	if (ret < 0)
		return ret;

	return 0;

}

static int mis2008_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	ret = mis2008_write(sd, 0x3102, (unsigned char)(value));

	if (value <= 0)
		ret += mis2008_write(sd, 0x3a02, 0x0b);
	else
		ret += mis2008_write(sd, 0x3a02, 0x0a);

	if (ret < 0)
		return ret;

	return 0;
}

static int mis2008_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int mis2008_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int mis2008_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int mis2008_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = mis2008_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if (sensor->video.state == TX_ISP_MODULE_INIT) {
			if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
				ret = mis2008_write_array(sd, mis2008_stream_on_dvp);
			} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
				ret = mis2008_write_array(sd, mis2008_stream_on_mipi);
			} else{
				ISP_ERROR("Don't support this Sensor Data interface\n");
			}
			sensor->video.state = TX_ISP_MODULE_RUNNING;
			ISP_WARNING("mis2008 stream on\n");
		}
	} else {
		if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
			ret = mis2008_write_array(sd, mis2008_stream_off_dvp);
		} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = mis2008_write_array(sd, mis2008_stream_off_mipi);
		} else {
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		sensor->video.state = TX_ISP_MODULE_INIT;
		ISP_WARNING("mis2008 stream off\n");
	}

	return ret;
}

static int mis2008_set_fps(struct tx_isp_subdev *sd, int fps)
{

	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;
	unsigned int pclk = MIS2008_SUPPORT_PCLK;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned char tmp;
	unsigned int newformat = 0; //the format is 24.8
	/* the format of fps is 16/16. for example 25 << 16 | 2, the value is 25/2 fps. */
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (SENSOR_OUTPUT_MAX_FPS << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)){
		ISP_ERROR("warn: fps(%d) no in range\n", fps);
		return -1;
	}
	ret = mis2008_read(sd, 0x3202, &tmp);
	hts = tmp;
	ret += mis2008_read(sd, 0x3203, &tmp);
	if(ret < 0)
		return -1;
	hts = ((hts << 8) + tmp);
	vts = pclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);

	ret = mis2008_write(sd, 0x3201, (unsigned char)(vts & 0xff));
	ret += mis2008_write(sd, 0x3200, (unsigned char)(vts >> 8));
	if(ret < 0){
		ISP_ERROR("err: mis2008_write err\n");
		return ret;
	}
	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 1;
	sensor->video.attr->integration_time_limit = vts - 1;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 1;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int mis2008_set_mode(struct tx_isp_subdev *sd, int value)
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

#if 0
static int mis2008_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	return 0;
}
#endif

static int sensor_attr_check(struct tx_isp_subdev *sd)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_register_info *info = &sensor->info;
	unsigned long rate;
	int ret = 0;

	switch(info->default_boot){
	case 0:
		wsize = &mis2008_win_sizes[0];
		mis2008_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		memcpy((void*)(&(mis2008_attr.mipi)),(void*)(&mis2008_mipi),sizeof(mis2008_mipi));
		sensor_max_fps = TX_SENSOR_MAX_FPS_30;
		break;
	case 1:
		wsize = &mis2008_win_sizes[1];
		mis2008_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		ret = set_sensor_gpio_function(sensor_gpio_func);
		if (ret < 0)
			goto err_set_sensor_gpio;
		mis2008_attr.dvp.gpio = sensor_gpio_func;
		memcpy((void*)(&(mis2008_attr.dvp)),(void*)(&mis2008_dvp),sizeof(mis2008_dvp));
		sensor_max_fps = TX_SENSOR_MAX_FPS_30;
		break;
	default:
		ISP_ERROR("Have no this setting!!!\n");
		break;
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		mis2008_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		data_interface = TX_SENSOR_DATA_INTERFACE_MIPI;
		mis2008_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		mis2008_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		data_interface = TX_SENSOR_DATA_INTERFACE_MIPI;
		mis2008_attr.mipi.index = 1;
		break;
	case TISP_SENSOR_VI_DVP:
		mis2008_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
		data_interface = TX_SENSOR_DATA_INTERFACE_DVP;
		break;
	default:
		ISP_ERROR("Have no this interface!!!\n");
		break;
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
		ISP_ERROR("not have this MCLK Source!\n");
	}
	ISP_WARNING("put mclk source to %d\n",info->mclk);

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

err_set_sensor_gpio:
err_get_mclk:
	return -1;
}

static int mis2008_g_chip_ident(struct tx_isp_subdev *sd,
			      struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"mis2008_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(15);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"mis2008_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(150);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = mis2008_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an mis2008 chip.\n",
		       client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("mis2008 chip found @ 0x%02x (%s) version %s\n", client->addr, client->adapter->name,SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "mis2008", sizeof("mis2008"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}
	return 0;
}

static int mis2008_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = mis2008_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = mis2008_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = mis2008_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = mis2008_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = mis2008_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
			ret = mis2008_write_array(sd, mis2008_stream_off_dvp);
		} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = mis2008_write_array(sd, mis2008_stream_off_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_DVP){
			ret = mis2008_write_array(sd, mis2008_stream_on_dvp);
		} else if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = mis2008_write_array(sd, mis2008_stream_on_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
			ret = -1;
		}
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = mis2008_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
//			ret = mis2008_set_vflip(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return 0;
}

static int mis2008_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = mis2008_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int mis2008_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	mis2008_write(sd, reg->reg & 0xffff, reg->val & 0xff);
	return 0;
}

static struct tx_isp_subdev_core_ops mis2008_core_ops = {
	.g_chip_ident = mis2008_g_chip_ident,
	.reset = mis2008_reset,
	.init = mis2008_init,
	.g_register = mis2008_g_register,
	.s_register = mis2008_s_register,
};

static struct tx_isp_subdev_video_ops mis2008_video_ops = {
	.s_stream = mis2008_s_stream,
};

static struct tx_isp_subdev_sensor_ops	mis2008_sensor_ops = {
	.ioctl	= mis2008_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops mis2008_ops = {
	.core = &mis2008_core_ops,
	.video = &mis2008_video_ops,
	.sensor = &mis2008_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "mis2008",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int mis2008_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	sensor->video.attr = &mis2008_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &mis2008_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->mis2008\n");
	return 0;
}

static int mis2008_remove(struct i2c_client *client)
{
	struct tx_isp_subdev *sd = private_i2c_get_clientdata(client);
	struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);

	if(reset_gpio != -1)
		private_gpio_free(reset_gpio);
	if(pwdn_gpio != -1)
		private_gpio_free(pwdn_gpio);

	private_clk_disable_unprepare(sensor->mclk);
	//private_devm_clk_put(&client->dev, sensor->mclk);
	tx_isp_subdev_deinit(sd);

	kfree(sensor);

	return 0;
}

static const struct i2c_device_id mis2008_id[] = {
	{ "mis2008", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mis2008_id);

static struct i2c_driver mis2008_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "mis2008",
	},
	.probe		= mis2008_probe,
	.remove		= mis2008_remove,
	.id_table	= mis2008_id,
};

static __init int init_mis2008(void)
{
	return private_i2c_add_driver(&mis2008_driver);
}

static __exit void exit_mis2008(void)
{
	private_i2c_del_driver(&mis2008_driver);
}

module_init(init_mis2008);
module_exit(exit_mis2008);

MODULE_DESCRIPTION("A low-level driver for mis2008 sensors");
MODULE_LICENSE("GPL");
