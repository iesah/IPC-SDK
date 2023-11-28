/*sc200ai.c
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

#define SC200AI_CHIP_ID_H	(0xcb)
#define SC200AI_CHIP_ID_L	(0x1c)
#define SC200AI_REG_END		0xffff
#define SC200AI_REG_DELAY	0xfffe
#define SC200AI_SUPPORT_PCLK_FPS_30 (74250*1000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20210222a"

static int reset_gpio = GPIO_PC(28);
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Reset GPIO NUM");

static int pwdn_gpio = -1;
module_param(pwdn_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwdn_gpio, "Power down GPIO NUM");

static int data_interface = TX_SENSOR_DATA_INTERFACE_MIPI;
module_param(data_interface, int, S_IRUGO);
MODULE_PARM_DESC(data_interface, "Sensor Date interface");

static int sensor_max_fps = TX_SENSOR_MAX_FPS_25;
module_param(sensor_max_fps, int, S_IRUGO);
MODULE_PARM_DESC(sensor_max_fps, "Sensor Max Fps set interface");

static int shvflip = 0;
module_param(shvflip, int, S_IRUGO);
MODULE_PARM_DESC(shvflip, "Sensor HV Flip Enable interface");

static bool dpc_flag = true;

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

struct again_lut sc200ai_again_lut[] = {
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
	{0x2340, 115704},
	{0x2341, 117166},
	{0x2342, 118606},
	{0x2343, 120025},
	{0x2344, 121449},
	{0x2345, 122826},
	{0x2346, 124183},
	{0x2347, 125521},
	{0x2348, 126840},
	{0x2349, 128141},
	{0x234a, 129424},
	{0x234b, 130691},
	{0x234c, 131963},
	{0x234d, 133196},
	{0x234e, 134413},
	{0x234f, 135615},
	{0x2350, 136801},
	{0x2351, 137973},
	{0x2352, 139131},
	{0x2353, 140274},
	{0x2354, 141425},
	{0x2355, 142541},
	{0x2356, 143644},
	{0x2357, 144735},
	{0x2358, 145813},
	{0x2359, 146879},
	{0x235a, 147932},
	{0x235b, 148975},
	{0x235c, 150025},
	{0x235d, 151045},
	{0x235e, 152054},
	{0x235f, 153052},
	{0x2360, 154039},
	{0x2361, 155017},
	{0x2362, 155984},
	{0x2363, 156942},
	{0x2364, 157908},
	{0x2365, 158846},
	{0x2366, 159776},
	{0x2367, 160696},
	{0x2368, 161607},
	{0x2369, 162510},
	{0x236a, 163404},
	{0x236b, 164290},
	{0x236c, 165184},
	{0x236d, 166053},
	{0x236e, 166914},
	{0x236f, 167768},
	{0x2370, 168614},
	{0x2371, 169452},
	{0x2372, 170283},
	{0x2373, 171107},
	{0x2374, 171939},
	{0x2375, 172749},
	{0x2376, 173552},
	{0x2377, 174348},
	{0x2378, 175137},
	{0x2379, 175920},
	{0x237a, 176696},
	{0x237b, 177466},
	{0x237c, 178244},
	{0x237d, 179002},
	{0x237e, 179753},
	{0x237f, 180499},
	{0x2740, 181239},
	{0x2741, 182701},
	{0x2742, 184155},
	{0x2743, 185573},
	{0x2744, 186971},
	{0x2745, 188348},
	{0x2746, 189718},
	{0x2747, 191056},
	{0x2748, 192375},
	{0x2749, 193676},
	{0x274a, 194971},
	{0x274b, 196237},
	{0x274c, 197487},
	{0x274d, 198720},
	{0x274e, 199948},
	{0x274f, 201150},
	{0x2750, 202336},
	{0x2751, 203508},
	{0x2752, 204676},
	{0x2753, 205820},
	{0x2754, 206949},
	{0x2755, 208066},
	{0x2756, 209179},
	{0x2757, 210270},
	{0x2758, 211348},
	{0x2759, 212414},
	{0x275a, 213477},
	{0x275b, 214520},
	{0x275c, 215550},
	{0x275d, 216570},
	{0x275e, 217589},
	{0x275f, 218549},
	{0x2760, 219574},
	{0x2761, 220497},
	{0x2762, 221501},
	{0x2763, 222405},
	{0x2764, 223389},
	{0x2765, 224364},
	{0x2766, 225241},
	{0x2767, 226196},
	{0x2768, 227142},
	{0x2769, 227994},
	{0x276a, 228922},
	{0x276b, 229758},
	{0x276c, 230669},
	{0x276d, 231572},
	{0x276e, 232385},
	{0x276f, 233271},
	{0x2770, 234149},
	{0x2771, 234940},
	{0x2772, 235803},
	{0x2773, 236580},
	{0x2774, 237428},
	{0x2775, 238269},
	{0x2776, 239026},
	{0x2777, 239853},
	{0x2778, 240672},
	{0x2779, 241411},
	{0x277a, 242216},
	{0x277b, 242943},
	{0x277c, 243736},
	{0x277d, 244523},
	{0x277e, 245232},
	{0x277f, 246006},
	{0x2f40, 246774},
	{0x2f41, 248223},
	{0x2f42, 249649},
	{0x2f43, 251055},
	{0x2f44, 252506},
	{0x2f45, 253870},
	{0x2f46, 255215},
	{0x2f47, 256540},
	{0x2f48, 257910},
	{0x2f49, 259199},
	{0x2f4a, 260470},
	{0x2f4b, 261725},
	{0x2f4c, 263022},
	{0x2f4d, 264243},
	{0x2f4e, 265449},
	{0x2f4f, 266640},
	{0x2f50, 267871},
	{0x2f51, 269032},
	{0x2f52, 270179},
	{0x2f53, 271312},
	{0x2f54, 272484},
	{0x2f55, 273590},
	{0x2f56, 274683},
	{0x2f57, 275764},
	{0x2f58, 276883},
	{0x2f59, 277939},
	{0x2f5a, 278983},
	{0x2f5b, 280015},
	{0x2f5c, 281085},
	{0x2f5d, 282096},
	{0x2f5e, 283095},
	{0x2f5f, 284084},
	{0x2f60, 285109},
	{0x2f61, 286078},
	{0x2f62, 287036},
	{0x2f63, 287985},
	{0x2f64, 288969},
	{0x2f65, 289899},
	{0x2f66, 290819},
	{0x2f67, 291731},
	{0x2f68, 292677},
	{0x2f69, 293571},
	{0x2f6a, 294457},
	{0x2f6b, 295335},
	{0x2f6c, 296245},
	{0x2f6d, 297107},
	{0x2f6e, 297960},
	{0x2f6f, 298806},
	{0x2f70, 299684},
	{0x2f71, 300514},
	{0x2f72, 301338},
	{0x2f73, 302154},
	{0x2f74, 303002},
	{0x2f75, 303804},
	{0x2f76, 304599},
	{0x2f77, 305388},
	{0x2f78, 306207},
	{0x2f79, 306982},
	{0x2f7a, 307751},
	{0x2f7b, 308514},
	{0x2f7c, 309307},
	{0x2f7d, 310058},
	{0x2f7e, 310802},
	{0x2f7f, 311541},
	{0x3f40, 312309},
	{0x3f41, 313758},
	{0x3f42, 315218},
	{0x3f43, 316623},
	{0x3f44, 318041},
	{0x3f45, 319405},
	{0x3f46, 320781},
	{0x3f47, 322107},
	{0x3f48, 323445},
	{0x3f49, 324734},
	{0x3f4a, 326035},
	{0x3f4b, 327290},
	{0x3f4c, 328557},
	{0x3f4d, 329778},
	{0x3f4e, 331013},
	{0x3f4f, 332203},
	{0x3f50, 333406},
	{0x3f51, 334567},
	{0x3f52, 335741},
	{0x3f53, 336874},
	{0x3f54, 338019},
	{0x3f55, 339125},
	{0x3f56, 340244},
	{0x3f57, 341324},
	{0x3f58, 342418},
	{0x3f59, 343474},
	{0x3f5a, 344542},
	{0x3f5b, 345575},
	{0x3f5c, 346620},
	{0x3f5d, 347631},
	{0x3f5e, 348654},
	{0x3f5f, 349643},
	{0x3f60, 350644},
	{0x3f61, 351613},
	{0x3f62, 352594},
	{0x3f63, 353542},
	{0x3f64, 354504},
};

struct tx_isp_sensor_attribute sc200ai_attr;

unsigned int sc200ai_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = sc200ai_again_lut;
	while(lut->gain <= sc200ai_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return 0;
		} else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		} else {
			if((lut->gain == sc200ai_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int sc200ai_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_sensor_attribute sc200ai_attr={
	.name = "sc200ai",
	.chip_id = 0xcb1c,
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
	.max_again = 354504,
	.max_dgain = 0,
	.expo_fs = 1,
	.min_integration_time = 4,
	.min_integration_time_native = 4,
	.max_integration_time_native = 0x546 - 4,
	.integration_time_limit = 0x546 - 4,
	.total_width = 2200,
	.total_height = 0x546,
	.max_integration_time = 0x546 - 4,
	.one_line_expr_in_us = 29,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 2,
	.sensor_ctrl.alloc_again = sc200ai_alloc_again,
	.sensor_ctrl.alloc_dgain = sc200ai_alloc_dgain,
};

static struct regval_list sc200ai_init_regs_1920_1080_25fps_mipi[] = {
	{0x0103,0x01},
	{0x0100,0x00},
	{0x36e9,0x80},
	{0x36f9,0x80},
	{0x301f,0x03},
	{0x3243,0x01},
	{0x3248,0x02},
	{0x3249,0x09},
	{0x3253,0x08},
	{0x3271,0x0a},
	{0x3301,0x20},
	{0x3304,0x40},
	{0x3306,0x32},
	{0x330b,0x88},
	{0x330f,0x02},
	{0x331e,0x39},
	{0x3333,0x10},
	{0x3621,0xe8},
	{0x3622,0x16},
	{0x3637,0x1b},
	{0x363a,0x1f},
	{0x363b,0xc6},
	{0x363c,0x0e},
	{0x3670,0x0a},
	{0x3674,0x82},
	{0x3675,0x76},
	{0x3676,0x78},
	{0x367c,0x48},
	{0x367d,0x58},
	{0x3690,0x34},
	{0x3691,0x33},
	{0x3692,0x44},
	{0x369c,0x40},
	{0x369d,0x48},
	{0x3901,0x02},
	{0x3904,0x04},
	{0x3908,0x41},
	{0x391d,0x14},
	{0x391f,0x18},
	{0x3e01,0x8c},
	{0x3e02,0x20},
	{0x3e16,0x00},
	{0x3e17,0x80},
	{0x3f09,0x48},
	{0x4827,0x02},
	{0x4819,0x30},
	{0x481b,0x0b},
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
	{0x59e0,0x60},
	{0x59e1,0x08},
	{0x59e2,0x3f},
	{0x59e3,0x18},
	{0x59e4,0x18},
	{0x59e5,0x3f},
	{0x59e6,0x06},
	{0x59e7,0x02},
	{0x59e8,0x38},
	{0x59e9,0x10},
	{0x59ea,0x0c},
	{0x59eb,0x10},
	{0x59ec,0x04},
	{0x59ed,0x02},
	{0x59ee,0xa0},
	{0x59ef,0x08},
	{0x59f4,0x18},
	{0x59f5,0x10},
	{0x59f6,0x0c},
	{0x59f7,0x10},
	{0x59f8,0x06},
	{0x59f9,0x02},
	{0x59fa,0x18},
	{0x59fb,0x10},
	{0x59fc,0x0c},
	{0x59fd,0x10},
	{0x59fe,0x04},
	{0x59ff,0x02},
	{0x320e,0x05},
	{0x320f,0x46},
	{0x36e9,0x20},
	{0x36f9,0x27},
	{0x0100,0x01},
 	{SC200AI_REG_DELAY,0x10},
	{SC200AI_REG_END, 0x00},	/* END MARKER */
};

static struct tx_isp_sensor_win_setting sc200ai_win_sizes[] = {
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc200ai_init_regs_1920_1080_25fps_mipi,
	},
};
struct tx_isp_sensor_win_setting *wsize = &sc200ai_win_sizes[0];

static struct regval_list sc200ai_stream_on[] = {
	{0x0100, 0x01},
	{SC200AI_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc200ai_stream_off[] = {
	{0x0100, 0x00},
	{SC200AI_REG_END, 0x00},	/* END MARKER */
};

int sc200ai_read(struct tx_isp_subdev *sd, uint16_t reg, unsigned char *value)
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

int sc200ai_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
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
static int sc200ai_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != SC200AI_REG_END) {
		if (vals->reg_num == SC200AI_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc200ai_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		pr_debug("vals->reg_num:0x%x, vals->value:0x%02x\n",vals->reg_num, val);
		vals++;
	}

	return 0;
}
#endif

static int sc200ai_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != SC200AI_REG_END) {
		if (vals->reg_num == SC200AI_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc200ai_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int sc200ai_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int sc200ai_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = sc200ai_read(sd, 0x3107, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC200AI_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = sc200ai_read(sd, 0x3108, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC200AI_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int sc200ai_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	int it = (value & 0xffff) * 2;
	int again = (value & 0xffff0000) >> 16;

	ret = sc200ai_write(sd, 0x3e00, (unsigned char)((it >> 12) & 0x0f));
	ret += sc200ai_write(sd, 0x3e01, (unsigned char)((it >> 4) & 0xff));
	ret += sc200ai_write(sd, 0x3e02, (unsigned char)((it & 0x0f) << 4));
	ret += sc200ai_write(sd, 0x3e09, (unsigned char)(again & 0xff));
	ret += sc200ai_write(sd, 0x3e08, (unsigned char)((again >> 8 & 0xff)));
	if (ret != 0) {
		ISP_ERROR("err: sc200ai write err %d\n",__LINE__);
		return ret;
	}

	if ((again >= 0x3f47) && dpc_flag) {
		sc200ai_write(sd, 0x5799, 0x07);
		dpc_flag = false;
	} else if ((again <= 0x2f5f) && (!dpc_flag)) {
		sc200ai_write(sd, 0x5799, 0x00);
		dpc_flag = true;
	}

	return 0;
}

static int sc200ai_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	ret = sc200ai_write(sd, 0x3e00, (unsigned char)((value >> 12) & 0x0f));
	ret += sc200ai_write(sd, 0x3e01, (unsigned char)((value >> 4) & 0xff));
	ret += sc200ai_write(sd, 0x3e02, (unsigned char)((value & 0x0f) << 4));

	if (ret < 0)
		return ret;

	return 0;
}

static int sc200ai_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	unsigned int again = value;
	ret = sc200ai_write(sd, 0x3e09, (unsigned char)(value & 0xff));
	ret += sc200ai_write(sd, 0x3e08, (unsigned char)((value >> 8 & 0xff)));
	if (ret < 0)
		return ret;

	if ((again >= 0x3f47) && dpc_flag) {
		sc200ai_write(sd, 0x5799, 0x07);
		dpc_flag = false;
	} else if ((again <= 0x2f5f) && (!dpc_flag)) {
		sc200ai_write(sd, 0x5799, 0x00);
		dpc_flag = true;
	}

	return 0;
}

static int sc200ai_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc200ai_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc200ai_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int sc200ai_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = sc200ai_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if (sensor->video.state == TX_ISP_MODULE_INIT) {
			if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
				ret = sc200ai_write_array(sd, sc200ai_stream_on);
			} else {
				ISP_ERROR("Don't support this Sensor Data interface\n");
			}
		}
		ISP_WARNING("sc3335 stream on\n");
		sensor->video.state = TX_ISP_MODULE_RUNNING;
	} else {
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc200ai_write_array(sd, sc200ai_stream_off);
		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		ISP_WARNING("sc200ai stream off\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
	}

	return ret;
}

static int sc200ai_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	unsigned int sclk = 0;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned char tmp = 0;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	sclk = SC200AI_SUPPORT_PCLK_FPS_30;
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (SENSOR_OUTPUT_MAX_FPS << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)) {
		ISP_ERROR("warn: fps(%d) no in range\n", fps);
		return -1;
	}
	ret = sc200ai_read(sd, 0x320c, &tmp);
	hts = tmp;
	ret += sc200ai_read(sd, 0x320d, &tmp);
	if (0 != ret) {
		ISP_ERROR("err: sc200ai read err\n");
		return ret;
	}
	hts = ((hts << 8) + tmp) << 1;
	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);
	ret += sc200ai_write(sd, 0x320f, (unsigned char)(vts & 0xff));
	ret += sc200ai_write(sd, 0x320e, (unsigned char)(vts >> 8));
	if (0 != ret) {
		ISP_ERROR("err: sc200ai_write err\n");
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

static int sc200ai_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = -1;
	unsigned char val = 0x0;

	ret += sc200ai_read(sd, 0x3221, &val);
	if(enable & 0x2)
		val |= 0x60;
	else
		val &= 0x9f;

	ret += sc200ai_write(sd, 0x3221, val);
	if(!ret)
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc200ai_set_mode(struct tx_isp_subdev *sd, int value)
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
	unsigned long rate;
	int ret = 0;

	switch(info->default_boot){
	case 0:
		wsize = &sc200ai_win_sizes[0];
		break;
	default:
		ISP_ERROR("Have no this setting!!!\n");
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		sc200ai_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		sc200ai_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		sc200ai_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		sc200ai_attr.mipi.index = 1;
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

	return 0;

err_get_mclk:
	return -1;
}

static int sc200ai_g_chip_ident(struct tx_isp_subdev *sd, struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"sc200ai_reset");
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
		ret = private_gpio_request(pwdn_gpio,"sc200ai_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = sc200ai_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an sc200ai chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("sc200ai chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "sc200ai", sizeof("sc200ai"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int sc200ai_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = sc200ai_set_expo(sd, sensor_val->value);
		break;
		/*
		  case TX_ISP_EVENT_SENSOR_INT_TIME:
		  if(arg)
		  ret = sc200ai_set_integration_time(sd, sensor_val->value);
		  break;
		  case TX_ISP_EVENT_SENSOR_AGAIN:
		  if(arg)
		  ret = sc200ai_set_analog_gain(sd, sensor_val->value);
		  break;
		*/
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = sc200ai_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = sc200ai_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = sc200ai_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc200ai_write_array(sd, sc200ai_stream_off);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc200ai_write_array(sd, sc200ai_stream_on);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
			ret = -1;
		}
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = sc200ai_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = sc200ai_set_vflip(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return ret;
}

static int sc200ai_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = sc200ai_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int sc200ai_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;

	sc200ai_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops sc200ai_core_ops = {
	.g_chip_ident = sc200ai_g_chip_ident,
	.reset = sc200ai_reset,
	.init = sc200ai_init,
	/*.ioctl = sc200ai_ops_ioctl,*/
	.g_register = sc200ai_g_register,
	.s_register = sc200ai_s_register,
};

static struct tx_isp_subdev_video_ops sc200ai_video_ops = {
	.s_stream = sc200ai_s_stream,
};

static struct tx_isp_subdev_sensor_ops	sc200ai_sensor_ops = {
	.ioctl	= sc200ai_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops sc200ai_ops = {
	.core = &sc200ai_core_ops,
	.video = &sc200ai_video_ops,
	.sensor = &sc200ai_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "sc200ai",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int sc200ai_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

	sc200ai_attr.expo_fs = 1;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->dev = &client->dev;
	sensor->video.shvflip = shvflip;
	sc200ai_attr.expo_fs = 1;
	sensor->video.attr = &sc200ai_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &sc200ai_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->sc200ai\n");

	return 0;
}

static int sc200ai_remove(struct i2c_client *client)
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

static const struct i2c_device_id sc200ai_id[] = {
	{ "sc200ai", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sc200ai_id);

static struct i2c_driver sc200ai_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "sc200ai",
	},
	.probe		= sc200ai_probe,
	.remove		= sc200ai_remove,
	.id_table	= sc200ai_id,
};

static __init int init_sc200ai(void)
{
	return private_i2c_add_driver(&sc200ai_driver);
}

static __exit void exit_sc200ai(void)
{
	private_i2c_del_driver(&sc200ai_driver);
}

module_init(init_sc200ai);
module_exit(exit_sc200ai);

MODULE_DESCRIPTION("A low-level driver for SmartSens sc200ai sensors");
MODULE_LICENSE("GPL");
