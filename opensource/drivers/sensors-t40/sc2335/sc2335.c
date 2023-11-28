/*
 * sc2335.c
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

#define SC2335_CHIP_ID_H	(0xcb)
#define SC2335_CHIP_ID_L	(0x14)
#define SC2335_REG_END		0xffff
#define SC2335_REG_DELAY	0xfffe
#define SC2335_SUPPORT_30FPS_SCLK (74250000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20220225a"

static int reset_gpio = GPIO_PC(28);
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Reset GPIO NUM");

static int pwdn_gpio = -1;
module_param(pwdn_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwdn_gpio, "Power down GPIO NUM");

static int data_interface = TX_SENSOR_DATA_INTERFACE_MIPI;
module_param(data_interface, int, S_IRUGO);
MODULE_PARM_DESC(data_interface, "Sensor Date interface");

static int shvflip = 0;
module_param(shvflip, int, S_IRUGO);
MODULE_PARM_DESC(shvflip, "Sensor HV Flip Enable interface");

struct regval_list {
	uint16_t reg_num;
	unsigned char value;
};

static unsigned short int dpc_flag = 1;
static unsigned int gain_val = 0x37e;
/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
	unsigned int value;
	unsigned int gain;
};

struct again_lut sc2335_again_lut[] = {
    {0x340, 0},
    {0x341, 1500},
    {0x342, 2886},
    {0x343, 4342},
    {0x344, 5776},
    {0x345, 7101},
    {0x346, 8494},
    {0x347, 9781},
    {0x348, 11136},
    {0x349, 12471},
    {0x34a, 13706},
    {0x34b, 15005},
    {0x34c, 16287},
    {0x34d, 17474},
    {0x34e, 18723},
    {0x34f, 19879},
    {0x350, 21097},
    {0x351, 22300},
    {0x352, 23413},
    {0x353, 24587},
    {0x354, 25746},
    {0x355, 26820},
    {0x356, 27952},
    {0x357, 29002},
    {0x358, 30108},
    {0x359, 31202},
    {0x35a, 32216},
    {0x35b, 33286},
    {0x35c, 34344},
    {0x35d, 35325},
    {0x35e, 36361},
    {0x35f, 37321},
    {0x360, 38335},
    {0x361, 39338},
    {0x362, 40269},
    {0x363, 41252},
    {0x364, 42225},
    {0x365, 43128},
    {0x366, 44082},
    {0x367, 44967},
    {0x368, 45903},
    {0x369, 46829},
    {0x36a, 47689},
    {0x36b, 48599},
    {0x36c, 49499},
    {0x36d, 50336},
    {0x36e, 51220},
    {0x36f, 52041},
    {0x370, 52910},
    {0x371, 53770},
    {0x372, 54570},
    {0x373, 55415},
    {0x374, 56253},
    {0x375, 57032},
    {0x376, 57856},
    {0x377, 58622},
    {0x378, 59433},
    {0x379, 60236},
    {0x37a, 60983},
    {0x37b, 61773},
    {0x37c, 62557},
    {0x37d, 63286},
    {0x37e, 64058},
    {0x37f, 64775},
    {0x740, 65535},
    {0x741, 66989},
    {0x742, 68467},
    {0x743, 69877},
    {0x744, 71266},
    {0x745, 72636},
    {0x746, 74029},
    {0x747, 75359},
    {0x748, 76671},
    {0x749, 77964},
    {0x74a, 79281},
    {0x74b, 80540},
    {0x74c, 81782},
    {0x74d, 83009},
    {0x74e, 84258},
    {0x74f, 85452},
    {0x750, 86632},
    {0x751, 87797},
    {0x752, 88985},
    {0x753, 90122},
    {0x754, 91245},
    {0x755, 92355},
    {0x756, 93487},
    {0x757, 94571},
    {0x758, 95643},
    {0x759, 96703},
    {0x75a, 97785},
    {0x75b, 98821},
    {0x75c, 99846},
    {0x75d, 100860},
    {0x75e, 101896},
    {0x75f, 102888},
    {0x760, 103870},
    {0x761, 104842},
    {0x762, 105835},
    {0x763, 106787},
    {0x764, 107730},
    {0x765, 108663},
    {0x766, 109617},
    {0x767, 110532},
    {0x768, 111438},
    {0x769, 112335},
    {0x76a, 113253},
    {0x76b, 114134},
    {0x76c, 115006},
    {0x76d, 115871},
    {0x76e, 116755},
    {0x76f, 117603},
    {0x770, 118445},
    {0x771, 119278},
    {0x772, 120131},
    {0x773, 120950},
    {0x774, 121762},
    {0x775, 122567},
    {0x776, 123391},
    {0x777, 124183},
    {0x778, 124968},
    {0x779, 125746},
    {0x77a, 126543},
    {0x77b, 127308},
    {0x77c, 128068},
    {0x77d, 128821},
    {0x77e, 129593},
    {0x77f, 130334},
    {0xf40, 131070},
    {0xf41, 132547},
    {0xf42, 133979},
    {0xf43, 135412},
    {0xf44, 136801},
    {0xf45, 138193},
    {0xf46, 139542},
    {0xf47, 140894},
    {0xf48, 142206},
    {0xf49, 143520},
    {0xf4a, 144796},
    {0xf4b, 146075},
    {0xf4c, 147317},
    {0xf4d, 148563},
    {0xf4e, 149773},
    {0xf4f, 150987},
    {0xf50, 152167},
    {0xf51, 153351},
    {0xf52, 154502},
    {0xf53, 155657},
    {0xf54, 156780},
    {0xf55, 157908},
    {0xf56, 159005},
    {0xf57, 160106},
    {0xf58, 161178},
    {0xf59, 162255},
    {0xf5a, 163303},
    {0xf5b, 164356},
    {0xf5c, 165381},
    {0xf5d, 166411},
    {0xf5e, 167414},
    {0xf5f, 168423},
    {0xf60, 169405},
    {0xf61, 170393},
    {0xf62, 171355},
    {0xf63, 172322},
    {0xf64, 173265},
    {0xf65, 174213},
    {0xf66, 175137},
    {0xf67, 176067},
    {0xf68, 176973},
    {0xf69, 177885},
    {0xf6a, 178774},
    {0xf6b, 179669},
    {0xf6c, 180541},
    {0xf6d, 181419},
    {0xf6e, 182276},
    {0xf6f, 183138},
    {0xf70, 183980},
    {0xf71, 184827},
    {0xf72, 185653},
    {0xf73, 186485},
    {0xf74, 187297},
    {0xf75, 188115},
    {0xf76, 188914},
    {0xf77, 189718},
    {0xf78, 190503},
    {0xf79, 191293},
    {0xf7a, 192065},
    {0xf7b, 192843},
    {0xf7c, 193603},
    {0xf7d, 194368},
    {0xf7e, 195116},
    {0xf7f, 195869},
    {0x1f40, 196605},
    {0x1f41, 198070},
    {0x1f42, 199514},
    {0x1f43, 200936},
    {0x1f44, 202336},
    {0x1f45, 203717},
    {0x1f46, 205077},
    {0x1f47, 206418},
    {0x1f48, 207741},
    {0x1f49, 209045},
    {0x1f4a, 210331},
    {0x1f4b, 211600},
    {0x1f4c, 212852},
    {0x1f4d, 214088},
    {0x1f4e, 215308},
    {0x1f4f, 216513},
    {0x1f50, 217702},
    {0x1f51, 218877},
    {0x1f52, 220037},
    {0x1f53, 221183},
    {0x1f54, 222315},
    {0x1f55, 223434},
    {0x1f56, 224540},
    {0x1f57, 225633},
    {0x1f58, 226713},
    {0x1f59, 227782},
    {0x1f5a, 228838},
    {0x1f5b, 229883},
    {0x1f5c, 230916},
    {0x1f5d, 231938},
    {0x1f5e, 232949},
    {0x1f5f, 233950},
    {0x1f60, 234940},
    {0x1f61, 235920},
    {0x1f62, 236890},
    {0x1f63, 237849},
    {0x1f64, 238800},
    {0x1f65, 239740},
    {0x1f66, 240672},
    {0x1f67, 241594},
    {0x1f68, 242508},
    {0x1f69, 243413},
    {0x1f6a, 244309},
    {0x1f6b, 245197},
    {0x1f6c, 246076},
    {0x1f6d, 246947},
    {0x1f6e, 247811},
    {0x1f6f, 248667},
    {0x1f70, 249515},
    {0x1f71, 250355},
    {0x1f72, 251188},
    {0x1f73, 252014},
    {0x1f74, 252832},
    {0x1f75, 253644},
    {0x1f76, 254449},
    {0x1f77, 255246},
    {0x1f78, 256038},
    {0x1f79, 256822},
    {0x1f7a, 257600},
    {0x1f7b, 258372},
    {0x1f7c, 259138},
    {0x1f7d, 259897},
    {0x1f7e, 260651},
    {0x1f7f, 261398},
};

struct tx_isp_sensor_attribute sc2335_attr;

unsigned int sc2335_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = sc2335_again_lut;
	while(lut->gain <= sc2335_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return 0;
		} else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		} else {
			if((lut->gain == sc2335_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int sc2335_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_sensor_attribute sc2335_attr={
	.name = "sc2335",
	.chip_id = 0xcb14,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x30,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.mode = SENSOR_MIPI_OTHER_MODE,
		.clk = 371,
		.lans = 2,
		.settle_time_apative_en = 0,
		.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
		.mipi_sc.hcrop_diff_en = 0,
		.mipi_sc.mipi_vcomp_en = 0,
		.mipi_sc.mipi_hcomp_en = 0,
		.mipi_sc.line_sync_mode = 0,
		.mipi_sc.work_start_flag = 0,
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
		.mipi_sc.data_type_en = 0,
		.mipi_sc.data_type_value = RAW10,
		.mipi_sc.del_start = 0,
		.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
		.mipi_sc.sensor_fid_mode = 0,
		.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
	},
	.data_type = TX_SENSOR_DATA_TYPE_LINEAR,
	.max_again = 261398,
	.max_dgain = 0,
	.min_integration_time = 2,
	.min_integration_time_native = 2,
	.max_integration_time_native = 0x546 - 5,
	.integration_time_limit = 0x546 - 5,
	.total_width = 2200,
	.total_height = 0x546,
	.max_integration_time = 0x546 - 5,
	.one_line_expr_in_us = 29,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = sc2335_alloc_again,
	.sensor_ctrl.alloc_dgain = sc2335_alloc_dgain,
};

static struct regval_list sc2335_init_regs_1920_1080_25fps_mipi[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x36e9, 0x80},
	{0x36f9, 0x80},
	{0x301f,0x1b},
	{0x3207,0x3f},
	{0x3249,0x0f},
	{0x3253,0x08},
	{0x3271,0x00},
	{0x3273,0x03},
	{0x3301,0x06},
	{0x3302,0x09},
	{0x3304,0x28},
	{0x3306,0x30},
	{0x330b,0x94},
	{0x330c,0x08},
	{0x330d,0x18},
	{0x330e,0x14},
	{0x330f,0x05},
	{0x3310,0x06},
	{0x3314,0x96},
	{0x3316,0x00},
	{0x331e,0x21},
	{0x332b,0x08},
	{0x3333,0x10},
	{0x3338,0x80},
	{0x333a,0x04},
	{0x334c,0x04},
	{0x335f,0x04},
	{0x3364,0x17},
	{0x3366,0x62},
	{0x337c,0x05},
	{0x337d,0x09},
	{0x337e,0x00},
	{0x3390,0x08},
	{0x3391,0x18},
	{0x3392,0x38},
	{0x3393,0x09},
	{0x3394,0x20},
	{0x3395,0x20},
	{0x33a2,0x07},
	{0x33ac,0x04},
	{0x33ae,0x14},
	{0x3614,0x00},
	{0x3622,0x16},
	{0x3630,0x68},
	{0x3631,0x84},
	{0x3637,0x20},
	{0x363a,0x1f},
	{0x363c,0x0e},
	{0x3670,0x0e},
	{0x3674,0xa1},
	{0x3675,0x9c},
	{0x3676,0x9e},
	{0x3677,0x84},
	{0x3678,0x85},
	{0x3679,0x87},
	{0x367c,0x18},
	{0x367d,0x38},
	{0x367e,0x08},
	{0x367f,0x18},
	{0x3690,0x32},
	{0x3691,0x32},
	{0x3692,0x44},
	{0x369c,0x08},
	{0x369d,0x38},
	{0x36ea,0xf5},
	{0x36fa,0xdf},
	{0x3908,0x82},
	{0x391f,0x18},
	{0x3e01,0x8c},
	{0x3e02,0x00},
	{0x3f00,0x0d},
	{0x3f04,0x02},
	{0x3f05,0x0e},
	{0x3f09,0x48},
	{0x4505,0x0a},
	{0x4509,0x20},
	{0x481d,0x0a},
	{0x4827,0x03},
	{0x5787,0x10},
	{0x5788,0x06},
	{0x578a,0x10},
	{0x578b,0x06},
	{0x5790,0x10},
	{0x5791,0x10},
	{0x5792,0x00},
	{0x5793,0x10},
	{0x5794,0x10},
	{0x5795,0x00},
	{0x5799,0x00},
	{0x57c7,0x10},
	{0x57c8,0x06},
	{0x57ca,0x10},
	{0x57cb,0x06},
	{0x57d1,0x10},
	{0x57d4,0x10},
	{0x57d9,0x00},
	{0x320e,0x05},
	{0x320f,0x46}, 	/*vts 25fps*/
	{0x36e9,0x59},
	{0x36f9,0x5b},
	{0x0100,0x01},



	{SC2335_REG_END, 0x00},	/* END MARKER */
};

static struct tx_isp_sensor_win_setting sc2335_win_sizes[] = {
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc2335_init_regs_1920_1080_25fps_mipi,
	},
};
struct tx_isp_sensor_win_setting *wsize = &sc2335_win_sizes[0];

static struct regval_list sc2335_stream_on_mipi[] = {
	{0x0100, 0x01},
	{SC2335_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc2335_stream_off_mipi[] = {
	{0x0100, 0x00},
	{SC2335_REG_END, 0x00},	/* END MARKER */
};

int sc2335_read(struct tx_isp_subdev *sd, uint16_t reg, unsigned char *value)
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

int sc2335_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
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
static int sc2335_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != SC2335_REG_END) {
		if (vals->reg_num == SC2335_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc2335_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}
#endif

static int sc2335_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != SC2335_REG_END) {
		if (vals->reg_num == SC2335_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc2335_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int sc2335_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int sc2335_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = sc2335_read(sd, 0x3107, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC2335_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = sc2335_read(sd, 0x3108, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC2335_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int sc2335_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	int it = (value & 0xffff) * 2;
	int again = (value & 0xffff0000) >> 16;

	ret += sc2335_write(sd, 0x3e00, (unsigned char)((it >> 12) & 0xf));
	ret += sc2335_write(sd, 0x3e01, (unsigned char)((it >> 4) & 0xff));
	ret += sc2335_write(sd, 0x3e02, (unsigned char)((it & 0x0f) << 4));
	ret = sc2335_write(sd, 0x3e09, (unsigned char)(again & 0xff));
	ret += sc2335_write(sd, 0x3e08, (unsigned char)(((again >> 8) & 0xff)));

	if (ret < 0)
		return ret;
	gain_val = again;

	return 0;
}

#if 0
static int sc2335_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	value *= 2;
	ret += sc2335_write(sd, 0x3e00, (unsigned char)((value >> 12) & 0xf));
	ret += sc2335_write(sd, 0x3e01, (unsigned char)((value >> 4) & 0xff));
	ret += sc2335_write(sd, 0x3e02, (unsigned char)((value & 0x0f) << 4));

	if (ret < 0)
		return ret;

	return 0;
}

static int sc2335_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	ret += sc2335_write(sd, 0x3e09, (unsigned char)(value & 0xff));
	ret += sc2335_write(sd, 0x3e08, (unsigned char)(((value >> 8) & 0xff)));
	if (ret < 0)
		return ret;
	gain_val = value;

	return 0;
}
#endif

static int sc2335_set_logic(struct tx_isp_subdev *sd, int value)
{
	unsigned char reg0;
	unsigned int ret = 0;

	/* analog gain setting logic */
	ret = sc2335_read(sd, 0x3040, &reg0);
	if (0x40 == reg0) {
		if (gain_val < 0x740) {
			ret += sc2335_write(sd, 0x363c, 0x0e);
		} else if (gain_val >= 0x740) {
			ret += sc2335_write(sd, 0x363c, 0x07);
		}
	} else if (0x41 == reg0) {
		if (gain_val < 0x740) {
			ret += sc2335_write(sd, 0x363c, 0x0f);
		} else if (gain_val >= 0x740) {
			ret += sc2335_write(sd, 0x363c, 0x07);
		}
	} else {
		ret += sc2335_write(sd, 0x363c, 0x07);
	}
	if (gain_val >= 0xf60) { //6x
		ret += sc2335_write(sd, 0x5799, 0x7);
		dpc_flag = 2;
	}
	else if (gain_val <= 0xf40) {//4x
		ret += sc2335_write(sd, 0x5799, 0x00);
	}
	if (ret < 0)
		return ret;

	return 0;
}
static int sc2335_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc2335_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc2335_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int sc2335_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
	    if(sensor->video.state == TX_ISP_MODULE_DEINIT){
            ret = sc2335_write_array(sd, wsize->regs);
            if (ret)
                return ret;
            sensor->video.state = TX_ISP_MODULE_INIT;
	    }
	    if(sensor->video.state == TX_ISP_MODULE_INIT){
            if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
                ret = sc2335_write_array(sd, sc2335_stream_on_mipi);
            } else {
                ISP_ERROR("Don't support this Sensor Data interface\n");
            }
            sensor->video.state = TX_ISP_MODULE_RUNNING;
            ISP_WARNING("sc2335 stream on\n");
	    }
	}
	else {
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc2335_write_array(sd, sc2335_stream_off_mipi);
		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
        sensor->video.state = TX_ISP_MODULE_INIT;
		ISP_WARNING("sc2335 stream off\n");
	}

	return ret;
}

static int sc2335_set_fps(struct tx_isp_subdev *sd, int fps)
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
	sclk = SC2335_SUPPORT_30FPS_SCLK;

	ret = sc2335_read(sd, 0x320c, &tmp);
	hts = tmp;
	ret += sc2335_read(sd, 0x320d, &tmp);
	if (0 != ret) {
		ISP_ERROR("err: sc2335 read err\n");
		return ret;
	}
	hts = ((hts << 8) + tmp) << 1;
	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);

	//ret = sc2335_write(sd,0x3812,0x00);
	ret += sc2335_write(sd, 0x320f, (unsigned char)(vts & 0xff));
	ret += sc2335_write(sd, 0x320e, (unsigned char)(vts >> 8));
	//ret += sc2335_write(sd,0x3812,0x30);
	if (0 != ret) {
		ISP_ERROR("err: sc2335_write err\n");
		return ret;
	}

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 5;
	sensor->video.attr->integration_time_limit = vts - 5;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 5;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc2335_set_mode(struct tx_isp_subdev *sd, int value)
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
    unsigned long rate;
    struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
    struct tx_isp_sensor_register_info *info = &sensor->info;

    if(info->default_boot !=0)
        ISP_ERROR("Have no this MCLK Source!!!\n");

    switch (info->video_interface) {
        case TISP_SENSOR_VI_MIPI_CSI0:
            sc2335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            sc2335_attr.mipi.index = 0;
            break;
        case TISP_SENSOR_VI_MIPI_CSI1:
            sc2335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            sc2335_attr.mipi.index = 1;
            break;
        case TISP_SENSOR_VI_DVP:
            sc2335_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
            break;
        default:
            ISP_ERROR("Have no this MCLK Source!!!\n");
    }

    switch (info->mclk) {
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
        return -1;
    }
    private_clk_set_rate(sensor->mclk, 24000000);
    private_clk_prepare_enable(sensor->mclk);

    reset_gpio = info->rst_gpio;
    pwdn_gpio = info->pwdn_gpio;

    return 0;
}

static int sc2335_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"sc2335_reset");
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
		ret = private_gpio_request(pwdn_gpio,"sc2335_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = sc2335_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an sc2335 chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("sc2335 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "sc2335", sizeof("sc2335"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int sc2335_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = -1;
	unsigned char val = 0x0;

	ret += sc2335_read(sd, 0x3221, &val);

	if(enable & 0x2)
		val |= 0x60;
	else
		val &= 0x9f;

	ret += sc2335_write(sd, 0x3221, val);

	if(!ret)
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc2335_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
	     	ret = sc2335_set_expo(sd, sensor_val->value);
		break;
//	case TX_ISP_EVENT_SENSOR_INT_TIME:
//		if(arg)
//			ret = sc2335_set_integration_time(sd, sensor_val->value);
//		break;
//	case TX_ISP_EVENT_SENSOR_AGAIN:
//		if(arg)
//			ret = sc2335_set_analog_gain(sd, sensor_val->value);
//		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = sc2335_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = sc2335_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = sc2335_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc2335_write_array(sd, sc2335_stream_off_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc2335_write_array(sd, sc2335_stream_on_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
			ret = -1;
		}
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = sc2335_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = sc2335_set_vflip(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_LOGIC:
		if(arg)
			ret = sc2335_set_logic(sd, sensor_val->value);
	default:
		break;
	}

	return ret;
}

static int sc2335_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = sc2335_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int sc2335_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;

	sc2335_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops sc2335_core_ops = {
	.g_chip_ident = sc2335_g_chip_ident,
	.reset = sc2335_reset,
	.init = sc2335_init,
	/*.ioctl = sc2335_ops_ioctl,*/
	.g_register = sc2335_g_register,
	.s_register = sc2335_s_register,
};

static struct tx_isp_subdev_video_ops sc2335_video_ops = {
	.s_stream = sc2335_s_stream,
};

static struct tx_isp_subdev_sensor_ops	sc2335_sensor_ops = {
	.ioctl	= sc2335_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops sc2335_ops = {
	.core = &sc2335_core_ops,
	.video = &sc2335_video_ops,
	.sensor = &sc2335_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "sc2335",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int sc2335_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	sensor->video.shvflip = shvflip;
	sensor->video.attr = &sc2335_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &sc2335_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->sc2335\n");

	return 0;
}

static int sc2335_remove(struct i2c_client *client)
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

static const struct i2c_device_id sc2335_id[] = {
	{ "sc2335", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sc2335_id);

static struct i2c_driver sc2335_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "sc2335",
	},
	.probe		= sc2335_probe,
	.remove		= sc2335_remove,
	.id_table	= sc2335_id,
};

static __init int init_sc2335(void)
{
	return private_i2c_add_driver(&sc2335_driver);
}

static __exit void exit_sc2335(void)
{
	private_i2c_del_driver(&sc2335_driver);
}

module_init(init_sc2335);
module_exit(exit_sc2335);

MODULE_DESCRIPTION("A low-level driver for SmartSens sc2335 sensors");
MODULE_LICENSE("GPL");
