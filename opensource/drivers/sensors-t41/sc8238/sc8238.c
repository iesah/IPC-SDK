/*
 * sc8238.c
 *
 * Copyright (C) 2022 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Settings:
 * sboot        resolution      fps       interface              mode
 *   0          3840*2160       15        mipi_2lane           linear
 *   1          3840*2160       30        mipi_2lane           linear
 *   2          3840*2160       20        mipi_2lane           linear
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

#define SC8238_CHIP_ID_H	(0x82)
#define SC8238_CHIP_ID_L	(0x35)
#define SC8238_REG_END		0xffff
#define SC8238_REG_DELAY	0xfffe
#define SENSOR_OUTPUT_MIN_FPS 5
#define SENSOR_VERSION	"H20220817a"

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

struct again_lut sc8238_again_lut[] = {
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
	{0x352, 23414},
	{0x353, 24587},
	{0x354, 25746},
	{0x355, 26820},
	{0x356, 27953},
	{0x357, 29002},
	{0x358, 30109},
	{0x359, 31203},
	{0x35a, 32217},
	{0x35b, 33287},
	{0x35c, 34345},
	{0x35d, 35326},
	{0x35e, 36361},
	{0x35f, 37322},
	{0x360, 38336},
	{0x361, 39339},
	{0x362, 40270},
	{0x363, 41253},
	{0x364, 42226},
	{0x365, 43129},
	{0x366, 44082},
	{0x367, 44968},
	{0x368, 45904},
	{0x369, 46830},
	{0x36a, 47690},
	{0x36b, 48599},
	{0x36c, 49500},
	{0x36d, 50336},
	{0x36e, 51220},
	{0x36f, 52042},
	{0x370, 52910},
	{0x371, 53771},
	{0x372, 54571},
	{0x373, 55416},
	{0x374, 56254},
	{0x375, 57033},
	{0x376, 57857},
	{0x377, 58623},
	{0x378, 59433},
	{0x379, 60237},
	{0x37a, 60984},
	{0x37b, 61774},
	{0x37c, 62558},
	{0x37d, 63287},
	{0x37e, 64059},
	{0x37f, 64776},
	{0x740, 65536},
	{0x741, 66990},
	{0x742, 68468},
	{0x743, 69878},
	{0x744, 71267},
	{0x745, 72637},
	{0x746, 74030},
	{0x747, 75360},
	{0x748, 76672},
	{0x749, 77965},
	{0x74a, 79283},
	{0x74b, 80541},
	{0x74c, 81784},
	{0x74d, 83010},
	{0x74e, 84259},
	{0x74f, 85454},
	{0x750, 86633},
	{0x751, 87799},
	{0x752, 88986},
	{0x753, 90123},
	{0x754, 91246},
	{0x755, 92356},
	{0x756, 93489},
	{0x757, 94573},
	{0x758, 95645},
	{0x759, 96705},
	{0x75a, 97786},
	{0x75b, 98823},
	{0x75c, 99848},
	{0x75d, 100862},
	{0x75e, 101897},
	{0x75f, 102890},
	{0x760, 103872},
	{0x761, 104844},
	{0x762, 105837},
	{0x763, 106789},
	{0x764, 107731},
	{0x765, 108665},
	{0x766, 109618},
	{0x767, 110533},
	{0x768, 111440},
	{0x769, 112337},
	{0x76a, 113255},
	{0x76b, 114135},
	{0x76c, 115008},
	{0x76d, 115872},
	{0x76e, 116756},
	{0x76f, 117605},
	{0x770, 118446},
	{0x771, 119280},
	{0x772, 120133},
	{0x773, 120952},
	{0x774, 121764},
	{0x775, 122569},
	{0x776, 123393},
	{0x777, 124185},
	{0x778, 124969},
	{0x779, 125748},
	{0x77a, 126545},
	{0x77b, 127310},
	{0x77c, 128070},
	{0x77d, 128823},
	{0x77e, 129595},
	{0x77f, 130336},
	{0xf40, 131072},
	{0xf41, 132549},
	{0xf42, 133981},
	{0xf43, 135414},
	{0xf44, 136803},
	{0xf45, 138195},
	{0xf46, 139544},
	{0xf47, 140896},
	{0xf48, 142208},
	{0xf49, 143522},
	{0xf4a, 144798},
	{0xf4b, 146077},
	{0xf4c, 147320},
	{0xf4d, 148565},
	{0xf4e, 149776},
	{0xf4f, 150990},
	{0xf50, 152169},
	{0xf51, 153353},
	{0xf52, 154504},
	{0xf53, 155659},
	{0xf54, 156782},
	{0xf55, 157910},
	{0xf56, 159007},
	{0xf57, 160109},
	{0xf58, 161181},
	{0xf59, 162258},
	{0xf5a, 163306},
	{0xf5b, 164359},
	{0xf5c, 165384},
	{0xf5d, 166414},
	{0xf5e, 167417},
	{0xf5f, 168426},
	{0xf60, 169408},
	{0xf61, 170395},
	{0xf62, 171357},
	{0xf63, 172325},
	{0xf64, 173267},
	{0xf65, 174216},
	{0xf66, 175140},
	{0xf67, 176069},
	{0xf68, 176976},
	{0xf69, 177888},
	{0xf6a, 178777},
	{0xf6b, 179671},
	{0xf6c, 180544},
	{0xf6d, 181422},
	{0xf6e, 182279},
	{0xf6f, 183141},
	{0xf70, 183982},
	{0xf71, 184829},
	{0xf72, 185656},
	{0xf73, 186488},
	{0xf74, 187300},
	{0xf75, 188118},
	{0xf76, 188916},
	{0xf77, 189721},
	{0xf78, 190505},
	{0xf79, 191296},
	{0xf7a, 192068},
	{0xf7b, 192846},
	{0xf7c, 193606},
	{0xf7d, 194371},
	{0xf7e, 195119},
	{0xf7f, 195872},
	{0x1f40, 196608},
	{0x1f41, 198073},
	{0x1f42, 199517},
	{0x1f43, 200939},
	{0x1f44, 202339},
	{0x1f45, 203720},
	{0x1f46, 205080},
	{0x1f47, 206421},
	{0x1f48, 207744},
	{0x1f49, 209048},
	{0x1f4a, 210334},
	{0x1f4b, 211603},
	{0x1f4c, 212856},
	{0x1f4d, 214092},
	{0x1f4e, 215312},
	{0x1f4f, 216516},
	{0x1f50, 217705},
	{0x1f51, 218880},
	{0x1f52, 220040},
	{0x1f53, 221186},
	{0x1f54, 222318},
	{0x1f55, 223437},
	{0x1f56, 224543},
	{0x1f57, 225636},
	{0x1f58, 226717},
	{0x1f59, 227785},
	{0x1f5a, 228842},
	{0x1f5b, 229886},
	{0x1f5c, 230920},
	{0x1f5d, 231942},
	{0x1f5e, 232953},
	{0x1f5f, 233954},
	{0x1f60, 234944},
	{0x1f61, 235923},
	{0x1f62, 236893},
	{0x1f63, 237853},
	{0x1f64, 238803},
	{0x1f65, 239744},
	{0x1f66, 240676},
	{0x1f67, 241598},
	{0x1f68, 242512},
	{0x1f69, 243416},
	{0x1f6a, 244313},
	{0x1f6b, 245200},
	{0x1f6c, 246080},
	{0x1f6d, 246951},
	{0x1f6e, 247815},
	{0x1f6f, 248670},
	{0x1f70, 249518},
	{0x1f71, 250359},
	{0x1f72, 251192},
	{0x1f73, 252018},
	{0x1f74, 252836},
	{0x1f75, 253648},
	{0x1f76, 254452},
	{0x1f77, 255250},
	{0x1f78, 256041},
	{0x1f79, 256826},
	{0x1f7a, 257604},
	{0x1f7b, 258376},
	{0x1f7c, 259142},
	{0x1f7d, 259901},
	{0x1f7e, 260655},
	{0x1f7f, 261402},
};

struct tx_isp_sensor_attribute sc8238_attr;

unsigned int sc8238_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = sc8238_again_lut;
	while(lut->gain <= sc8238_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return lut[0].gain;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == sc8238_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int sc8238_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus sc8238_mipi={
	.mode = SENSOR_MIPI_OTHER_MODE,
	.clk = 743,
	.lans = 2,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
	.mipi_sc.hcrop_diff_en = 0,
	.mipi_sc.mipi_vcomp_en = 0,
	.mipi_sc.mipi_hcomp_en = 0,
	.mipi_sc.line_sync_mode = 0,
	.mipi_sc.work_start_flag = 0,
	.image_twidth = 3840,
	.image_theight = 2160,
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

struct tx_isp_sensor_attribute sc8238_attr={
	.name = "sc8238",
	.chip_id = 0x8235,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x30,
	.max_again = 261402,
	.max_dgain = 0,
	.min_integration_time = 3,
	.min_integration_time_native = 3,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = sc8238_alloc_again,
	.sensor_ctrl.alloc_dgain = sc8238_alloc_dgain,
};

static struct regval_list sc8238_init_regs_3840_2160_15fps_mipi[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x36e9, 0x80},
	{0x36f9, 0x80},
	{0x3018, 0x32},
	{0x3019, 0x0c},
	{0x301f, 0x30},
	{0x3031, 0x0a},
	{0x3037, 0x20},
	{0x3038, 0x44},
	{0x303f, 0x81},
	{0x320c, 0x08},//
	{0x320d, 0x98},/*HTS = 898 = 2200*/
	{0x320e, 0x08},/*VTS = 8CA = 2250*/
	{0x320f, 0xca},//
	{0x3241, 0x00},
	{0x3243, 0x03},
	{0x3248, 0x04},
	{0x3271, 0x1c},
	{0x3273, 0x1f},
	{0x3301, 0x30},
	{0x3306, 0xa8},
	{0x3309, 0x40},
	{0x330b, 0x68},
	{0x330e, 0x60},
	{0x3314, 0x94},
	{0x331f, 0x31},
	{0x334c, 0x10},
	{0x335d, 0x60},
	{0x3366, 0x92},
	{0x3367, 0x08},
	{0x3368, 0x07},
	{0x3369, 0x00},
	{0x336a, 0x00},
	{0x336b, 0x00},
	{0x336c, 0xc2},
	{0x337f, 0x33},
	{0x33af, 0x24},
	{0x33e0, 0xa0},
	{0x33e1, 0x08},
	{0x33e2, 0x18},
	{0x33e3, 0x10},
	{0x33e4, 0x0c},
	{0x33e5, 0x10},
	{0x33e6, 0x06},
	{0x33e7, 0x02},
	{0x33e8, 0x18},
	{0x33e9, 0x10},
	{0x33ea, 0x0c},
	{0x33eb, 0x10},
	{0x33ec, 0x04},
	{0x33ed, 0x02},
	{0x33ee, 0xa0},
	{0x33ef, 0x08},
	{0x33f4, 0x18},
	{0x33f5, 0x10},
	{0x33f6, 0x0c},
	{0x33f7, 0x10},
	{0x33f8, 0x06},
	{0x33f9, 0x02},
	{0x33fa, 0x18},
	{0x33fb, 0x10},
	{0x33fc, 0x0c},
	{0x33fd, 0x10},
	{0x33fe, 0x04},
	{0x33ff, 0x02},
	{0x360f, 0x01},
	{0x3622, 0xf7},
	{0x3624, 0x45},
	{0x3628, 0x83},
	{0x3630, 0x80},
	{0x3631, 0x80},
	{0x3632, 0xa8},
	{0x3633, 0x22},
	{0x3635, 0x02},
	{0x3637, 0x52},
	{0x3638, 0x08},
	{0x363a, 0x88},
	{0x363b, 0x06},
	{0x363d, 0x01},
	{0x363e, 0x00},
	{0x3641, 0x00},
	{0x3670, 0x4a},
	{0x3671, 0xf7},
	{0x3672, 0x17},
	{0x3673, 0x17},
	{0x3674, 0x80},
	{0x3675, 0x85},
	{0x3676, 0xa5},
	{0x367a, 0x48},
	{0x367b, 0x78},
	{0x367c, 0x48},
	{0x367d, 0x78},
	{0x3690, 0x42},
	{0x3691, 0x33},
	{0x3692, 0x44},
	{0x3699, 0x88},
	{0x369a, 0x9f},
	{0x369b, 0x9f},
	{0x369c, 0x48},
	{0x369d, 0x78},
	{0x36a2, 0x48},
	{0x36a3, 0x78},
	{0x36bb, 0x48},
	{0x36bc, 0x78},
	{0x36c9, 0x05},
	{0x36ca, 0x05},
	{0x36cb, 0x05},
	{0x36cc, 0x00},
	{0x36cd, 0x10},
	{0x36ce, 0x1a},
	{0x36d0, 0x30},
	{0x36d1, 0x48},
	{0x36d2, 0x78},
	{0x36ea, 0x5f},
	{0x36eb, 0x0a},
	{0x36ec, 0x05},
	{0x36ed, 0x04},
	{0x36fa, 0x5f},
	{0x36fb, 0x15},
	{0x36fc, 0x10},
	{0x36fd, 0x04},
	{0x3901, 0x00},
	{0x3902, 0xc5},
	{0x3904, 0x18},
	{0x3907, 0x01},
	{0x3908, 0x11},
	{0x394c, 0x0f},
	{0x394d, 0x20},
	{0x394e, 0x08},
	{0x394f, 0x90},
	{0x3980, 0x71},
	{0x3981, 0x70},
	{0x3982, 0x00},
	{0x3983, 0x00},
	{0x3984, 0x20},
	{0x3987, 0x0b},
	{0x3990, 0x03},
	{0x3991, 0xfd},
	{0x3992, 0x03},
	{0x3993, 0xf8},
	{0x3994, 0x03},
	{0x3995, 0xfd},
	{0x3996, 0x03},
	{0x3997, 0xff},
	{0x3998, 0x00},
	{0x3999, 0x02},
	{0x399a, 0x00},
	{0x399b, 0x09},
	{0x399c, 0x00},
	{0x399d, 0x0c},
	{0x399e, 0x00},
	{0x399f, 0x0c},
	{0x39a0, 0x00},
	{0x39a1, 0x08},
	{0x39a2, 0x03},
	{0x39a3, 0xe3},
	{0x39a4, 0x03},
	{0x39a5, 0xee},
	{0x39a6, 0x03},
	{0x39a7, 0xf4},
	{0x39a8, 0x03},
	{0x39a9, 0xf8},
	{0x39aa, 0x03},
	{0x39ab, 0xfd},
	{0x39ac, 0x00},
	{0x39ad, 0x04},
	{0x39ae, 0x00},
	{0x39af, 0x09},
	{0x39b0, 0x00},
	{0x39b1, 0x0e},
	{0x39b2, 0x00},
	{0x39b3, 0x20},
	{0x39b4, 0x0c},
	{0x39b5, 0x1c},
	{0x39b6, 0x38},
	{0x39b7, 0x5b},
	{0x39b8, 0x50},
	{0x39b9, 0x38},
	{0x39ba, 0x20},
	{0x39bb, 0x10},
	{0x39bc, 0x0c},
	{0x39bd, 0x16},
	{0x39be, 0x21},
	{0x39bf, 0x36},
	{0x39c0, 0x3b},
	{0x39c1, 0x2a},
	{0x39c2, 0x16},
	{0x39c3, 0x0c},
	{0x39c5, 0x30},
	{0x39c6, 0x00},
	{0x39c7, 0x40},
	{0x39c9, 0x07},
	{0x39ca, 0xf8},
	{0x39cc, 0x07},
	{0x39cd, 0xd0},
	{0x39ce, 0x00},
	{0x39cf, 0x00},
	{0x39d0, 0x34},
	{0x39d1, 0x00},
	{0x39e2, 0x15},
	{0x39e3, 0x4d},
	{0x39e4, 0x15},
	{0x39e5, 0xbb},
	{0x39e6, 0x00},
	{0x39e7, 0x2f},
	{0x39e8, 0x00},
	{0x39e9, 0x1b},
	{0x39ea, 0x01},
	{0x39eb, 0x7a},
	{0x39ec, 0x08},
	{0x39ed, 0x00},
	{0x3e00, 0x01},
	{0x3e01, 0x18},
	{0x3e02, 0xa0},
	{0x3e08, 0x03},
	{0x3e09, 0x40},
	{0x3e0e, 0x09},
	{0x3e14, 0x31},
	{0x3e16, 0x00},
	{0x3e17, 0xac},
	{0x3e18, 0x00},
	{0x3e19, 0xac},
	{0x3e1b, 0x3a},
	{0x3e1e, 0x76},
	{0x3e25, 0x23},
	{0x3e26, 0x40},
	{0x4501, 0xa4},
	{0x4509, 0x10},
	{0x4800, 0x64},
	{0x4837, 0x1a},
	{0x5799, 0x06},
	{0x57aa, 0x2f},
	{0x57ab, 0xff},
	{0x5988, 0x86},
	{0x598e, 0x08},
	{0x598f, 0x68},
	{0x36e9, 0x2a},
	{0x36f9, 0x2b},
//	{0x0100, 0x01},
	{SC8238_REG_END, 0x00},/* END MARKER */
};

static struct regval_list sc8238_init_regs_3840_2160_30fps_mipi[] = {
	{0x0103, 0x01},
	{0x0100, 0x00},
	{0x36e9, 0x80},
	{0x36f9, 0x80},
	{0x3018, 0x32},
	{0x3019, 0x00},
	{0x301f, 0x83},
	{0x3031, 0x0a},
	{0x3037, 0x20},
	{0x3038, 0x44},
	{0x320c, 0x08},//
	{0x320d, 0x34},//
	{0x3241, 0x00},
	{0x3243, 0x03},
	{0x3248, 0x04},
	{0x3271, 0x1c},
	{0x3273, 0x1f},
	{0x3301, 0x1c},
	{0x3306, 0xa8},
	{0x3308, 0x20},
	{0x3309, 0x68},
	{0x330b, 0x48},
	{0x330d, 0x28},
	{0x330e, 0x58},
	{0x3314, 0x94},
	{0x331f, 0x59},
	{0x3332, 0x24},
	{0x334c, 0x10},
	{0x3350, 0x24},
	{0x3358, 0x24},
	{0x335c, 0x24},
	{0x335d, 0x60},
	{0x3364, 0x16},
	{0x3366, 0x92},
	{0x3367, 0x08},
	{0x3368, 0x07},
	{0x3369, 0x00},
	{0x336a, 0x00},
	{0x336b, 0x00},
	{0x336c, 0xc2},
	{0x337f, 0x33},
	{0x3390, 0x08},
	{0x3391, 0x18},
	{0x3392, 0x38},
	{0x3393, 0x1c},
	{0x3394, 0x28},
	{0x3395, 0x60},
	{0x3396, 0x08},
	{0x3397, 0x18},
	{0x3398, 0x38},
	{0x3399, 0x1c},
	{0x339a, 0x1c},
	{0x339b, 0x28},
	{0x339c, 0x30},
	{0x339e, 0x24},
	{0x33aa, 0x24},
	{0x33af, 0x48},
	{0x33e1, 0x08},
	{0x33e2, 0x18},
	{0x33e3, 0x10},
	{0x33e4, 0x0c},
	{0x33e5, 0x10},
	{0x33e6, 0x06},
	{0x33e7, 0x02},
	{0x33e8, 0x18},
	{0x33e9, 0x10},
	{0x33ea, 0x0c},
	{0x33eb, 0x10},
	{0x33ec, 0x04},
	{0x33ed, 0x02},
	{0x33ee, 0xa0},
	{0x33ef, 0x08},
	{0x33f4, 0x18},
	{0x33f5, 0x10},
	{0x33f6, 0x0c},
	{0x33f7, 0x10},
	{0x33f8, 0x06},
	{0x33f9, 0x02},
	{0x33fa, 0x18},
	{0x33fb, 0x10},
	{0x33fc, 0x0c},
	{0x33fd, 0x10},
	{0x33fe, 0x04},
	{0x33ff, 0x02},
	{0x360f, 0x01},
	{0x3622, 0xf7},
	{0x3624, 0x45},
	{0x3628, 0x83},
	{0x3630, 0x80},
	{0x3631, 0x80},
	{0x3632, 0xa8},
	{0x3633, 0x53},
	{0x3635, 0x02},
	{0x3637, 0x7b},
	{0x3638, 0x0a},
	{0x363a, 0x88},
	{0x363b, 0x06},
	{0x363d, 0x01},
	{0x363e, 0x00},
	{0x3641, 0x00},
	{0x3670, 0x4a},
	{0x3671, 0xf7},
	{0x3672, 0xf7},
	{0x3673, 0x17},
	{0x3674, 0x80},
	{0x3675, 0xa5},
	{0x3676, 0xb5},
	{0x367a, 0x48},
	{0x367b, 0x78},
	{0x367c, 0x48},
	{0x367d, 0x78},
	{0x3690, 0x53},
	{0x3691, 0x63},
	{0x3692, 0x54},
	{0x3699, 0x88},
	{0x369a, 0x9f},
	{0x369b, 0x9f},
	{0x369c, 0x48},
	{0x369d, 0x78},
	{0x36a2, 0x48},
	{0x36a3, 0x78},
	{0x36bb, 0x48},
	{0x36bc, 0x78},
	{0x36c9, 0x05},
	{0x36ca, 0x05},
	{0x36cb, 0x05},
	{0x36cc, 0x00},
	{0x36cd, 0x10},
	{0x36ce, 0x1a},
	{0x36d0, 0x30},
	{0x36d1, 0x48},
	{0x36d2, 0x78},
	{0x36ea, 0x39},
	{0x36eb, 0x06},
	{0x36ec, 0x05},
	{0x36ed, 0x24},
	{0x36fa, 0x39},
	{0x36fb, 0x13},
	{0x36fc, 0x10},
	{0x36fd, 0x14},
	{0x3901, 0x00},
	{0x3902, 0xc5},
	{0x3904, 0x18},
	{0x3905, 0xd8},
	{0x394c, 0x0f},
	{0x394d, 0x20},
	{0x394e, 0x08},
	{0x394f, 0x90},
	{0x3980, 0x71},
	{0x3981, 0x70},
	{0x3982, 0x00},
	{0x3983, 0x00},
	{0x3984, 0x20},
	{0x3987, 0x0b},
	{0x3990, 0x03},
	{0x3991, 0xfd},
	{0x3992, 0x03},
	{0x3993, 0xfc},
	{0x3994, 0x00},
	{0x3995, 0x00},
	{0x3996, 0x00},
	{0x3997, 0x05},
	{0x3998, 0x00},
	{0x3999, 0x09},
	{0x399a, 0x00},
	{0x399b, 0x12},
	{0x399c, 0x00},
	{0x399d, 0x12},
	{0x399e, 0x00},
	{0x399f, 0x18},
	{0x39a0, 0x00},
	{0x39a1, 0x14},
	{0x39a2, 0x03},
	{0x39a3, 0xe3},
	{0x39a4, 0x03},
	{0x39a5, 0xf2},
	{0x39a6, 0x03},
	{0x39a7, 0xf6},
	{0x39a8, 0x03},
	{0x39a9, 0xfa},
	{0x39aa, 0x03},
	{0x39ab, 0xff},
	{0x39ac, 0x00},
	{0x39ad, 0x06},
	{0x39ae, 0x00},
	{0x39af, 0x09},
	{0x39b0, 0x00},
	{0x39b1, 0x12},
	{0x39b2, 0x00},
	{0x39b3, 0x22},
	{0x39b4, 0x0c},
	{0x39b5, 0x1c},
	{0x39b6, 0x38},
	{0x39b7, 0x5b},
	{0x39b8, 0x50},
	{0x39b9, 0x38},
	{0x39ba, 0x20},
	{0x39bb, 0x10},
	{0x39bc, 0x0c},
	{0x39bd, 0x16},
	{0x39be, 0x21},
	{0x39bf, 0x36},
	{0x39c0, 0x3b},
	{0x39c1, 0x2a},
	{0x39c2, 0x16},
	{0x39c3, 0x0c},
	{0x39c5, 0x30},
	{0x39c6, 0x07},
	{0x39c7, 0xf8},
	{0x39c9, 0x07},
	{0x39ca, 0xf8},
	{0x39cc, 0x00},
	{0x39cd, 0x1b},
	{0x39ce, 0x00},
	{0x39cf, 0x00},
	{0x39d0, 0x1b},
	{0x39d1, 0x00},
	{0x39e2, 0x15},
	{0x39e3, 0x87},
	{0x39e4, 0x12},
	{0x39e5, 0xb7},
	{0x39e6, 0x00},
	{0x39e7, 0x8c},
	{0x39e8, 0x01},
	{0x39e9, 0x31},
	{0x39ea, 0x01},
	{0x39eb, 0xd7},
	{0x39ec, 0x08},
	{0x39ed, 0x00},
	{0x3e00, 0x01},//
	{0x3e01, 0x18},//
	{0x3e02, 0xa0},//
	{0x3e08, 0x03},
	{0x3e09, 0x40},
	{0x3e0e, 0x09},
	{0x3e14, 0x31},
	{0x3e16, 0x00},
	{0x3e17, 0xac},
	{0x3e18, 0x00},
	{0x3e19, 0xac},
	{0x3e1b, 0x3a},
	{0x3e1e, 0x76},
	{0x3e25, 0x23},
	{0x3e26, 0x40},
	{0x4501, 0xa4},
	{0x4509, 0x10},
	{0x4800, 0x64},
	{0x481a, 0x00},
	{0x481b, 0x26},
	{0x481c, 0x00},
	{0x481d, 0xf0},
	{0x4821, 0x02},
	{0x4828, 0x00},
	{0x4829, 0x02},
	{0x4837, 0x0e},
	{0x5799, 0x06},
	{0x57aa, 0x2f},
	{0x57ab, 0xff},
	{0x5988, 0x86},
	{0x598e, 0x07},
	{0x598f, 0xdf},
	{0x36e9, 0x57},
	{0x36f9, 0x57},
	{0x0100, 0x01},
	{SC8238_REG_END, 0x00},/* END MARKER */
};

static struct regval_list sc8238_init_regs_3840_2160_20fps_mipi[] = {
	{0x0103,0x01},
	{0x0100,0x00},
	{0x36e9,0x80},
	{0x36f9,0x80},
	{0x3018,0x32},
	{0x3019,0x0c},
	{0x301f,0x9d},
	{0x3031,0x0a},
	{0x3037,0x20},
	{0x3038,0x44},
	{0x320c,0x08},//
	{0x320d,0x55},//hts = 2133
	{0x3241,0x00},
	{0x3243,0x03},
	{0x3248,0x04},
	{0x3271,0x1c},
	{0x3273,0x1f},
	{0x3301,0x1c},
	{0x3306,0xa8},
	{0x3308,0x20},
	{0x3309,0x68},
	{0x330b,0x48},
	{0x330d,0x28},
	{0x330e,0x58},
	{0x3314,0x94},
	{0x331f,0x59},
	{0x3332,0x24},
	{0x334c,0x10},
	{0x3350,0x24},
	{0x3358,0x24},
	{0x335c,0x24},
	{0x335d,0x60},
	{0x3364,0x16},
	{0x3366,0x92},
	{0x3367,0x08},
	{0x3368,0x07},
	{0x3369,0x00},
	{0x336a,0x00},
	{0x336b,0x00},
	{0x336c,0xc2},
	{0x337f,0x33},
	{0x3390,0x08},
	{0x3391,0x18},
	{0x3392,0x38},
	{0x3393,0x1c},
	{0x3394,0x28},
	{0x3395,0x60},
	{0x3396,0x08},
	{0x3397,0x18},
	{0x3398,0x38},
	{0x3399,0x1c},
	{0x339a,0x1c},
	{0x339b,0x28},
	{0x339c,0x60},
	{0x339e,0x24},
	{0x33aa,0x24},
	{0x33af,0x48},
	{0x33e1,0x08},
	{0x33e2,0x18},
	{0x33e3,0x10},
	{0x33e4,0x0c},
	{0x33e5,0x10},
	{0x33e6,0x06},
	{0x33e7,0x02},
	{0x33e8,0x18},
	{0x33e9,0x10},
	{0x33ea,0x0c},
	{0x33eb,0x10},
	{0x33ec,0x04},
	{0x33ed,0x02},
	{0x33ee,0xa0},
	{0x33ef,0x08},
	{0x33f4,0x18},
	{0x33f5,0x10},
	{0x33f6,0x0c},
	{0x33f7,0x10},
	{0x33f8,0x06},
	{0x33f9,0x02},
	{0x33fa,0x18},
	{0x33fb,0x10},
	{0x33fc,0x0c},
	{0x33fd,0x10},
	{0x33fe,0x04},
	{0x33ff,0x02},
	{0x360f,0x01},
	{0x3622,0xf7},
	{0x3624,0x45},
	{0x3628,0x83},
	{0x3630,0x80},
	{0x3631,0x80},
	{0x3632,0xa8},
	{0x3633,0x53},
	{0x3635,0x02},
	{0x3637,0x52},
	{0x3638,0x0a},
	{0x363a,0x88},
	{0x363b,0x06},
	{0x363d,0x01},
	{0x363e,0x00},
	{0x3641,0x00},
	{0x3670,0x4a},
	{0x3671,0xf7},
	{0x3672,0xf7},
	{0x3673,0x17},
	{0x3674,0x80},
	{0x3675,0x85},
	{0x3676,0xa5},
	{0x367a,0x48},
	{0x367b,0x78},
	{0x367c,0x48},
	{0x367d,0x78},
	{0x3690,0x53},
	{0x3691,0x63},
	{0x3692,0x54},
	{0x3699,0x88},
	{0x369a,0x9f},
	{0x369b,0x9f},
	{0x369c,0x48},
	{0x369d,0x78},
	{0x36a2,0x48},
	{0x36a3,0x78},
	{0x36bb,0x48},
	{0x36bc,0x78},
	{0x36c9,0x05},
	{0x36ca,0x05},
	{0x36cb,0x05},
	{0x36cc,0x00},
	{0x36cd,0x10},
	{0x36ce,0x1a},
	{0x36d0,0x30},
	{0x36d1,0x48},
	{0x36d2,0x78},
	{0x36ea,0x36},
	{0x36eb,0x06},
	{0x36ec,0x05},
	{0x36ed,0x14},
	{0x36fa,0x38},
	{0x36fb,0x13},
	{0x36fc,0x10},
	{0x36fd,0x14},
	{0x3901,0x00},
	{0x3902,0xc5},
	{0x3904,0x18},
	{0x3905,0xd8},
	{0x394c,0x0f},
	{0x394d,0x20},
	{0x394e,0x08},
	{0x394f,0x90},
	{0x3980,0x71},
	{0x3981,0x70},
	{0x3982,0x00},
	{0x3983,0x00},
	{0x3984,0x20},
	{0x3987,0x0b},
	{0x3990,0x03},
	{0x3991,0xfd},
	{0x3992,0x03},
	{0x3993,0xfc},
	{0x3994,0x00},
	{0x3995,0x00},
	{0x3996,0x00},
	{0x3997,0x05},
	{0x3998,0x00},
	{0x3999,0x09},
	{0x399a,0x00},
	{0x399b,0x12},
	{0x399c,0x00},
	{0x399d,0x12},
	{0x399e,0x00},
	{0x399f,0x18},
	{0x39a0,0x00},
	{0x39a1,0x14},
	{0x39a2,0x03},
	{0x39a3,0xe3},
	{0x39a4,0x03},
	{0x39a5,0xf2},
	{0x39a6,0x03},
	{0x39a7,0xf6},
	{0x39a8,0x03},
	{0x39a9,0xfa},
	{0x39aa,0x03},
	{0x39ab,0xff},
	{0x39ac,0x00},
	{0x39ad,0x06},
	{0x39ae,0x00},
	{0x39af,0x09},
	{0x39b0,0x00},
	{0x39b1,0x12},
	{0x39b2,0x00},
	{0x39b3,0x22},
	{0x39b4,0x0c},
	{0x39b5,0x1c},
	{0x39b6,0x38},
	{0x39b7,0x5b},
	{0x39b8,0x50},
	{0x39b9,0x38},
	{0x39ba,0x20},
	{0x39bb,0x10},
	{0x39bc,0x0c},
	{0x39bd,0x16},
	{0x39be,0x21},
	{0x39bf,0x36},
	{0x39c0,0x3b},
	{0x39c1,0x2a},
	{0x39c2,0x16},
	{0x39c3,0x0c},
	{0x39c5,0x30},
	{0x39c6,0x07},
	{0x39c7,0xf8},
	{0x39c9,0x07},
	{0x39ca,0xf8},
	{0x39cc,0x00},
	{0x39cd,0x1b},
	{0x39ce,0x00},
	{0x39cf,0x00},
	{0x39d0,0x1b},
	{0x39d1,0x00},
	{0x39e2,0x15},
	{0x39e3,0x87},
	{0x39e4,0x12},
	{0x39e5,0xb7},
	{0x39e6,0x00},
	{0x39e7,0x8c},
	{0x39e8,0x01},
	{0x39e9,0x31},
	{0x39ea,0x01},
	{0x39eb,0xd7},
	{0x39ec,0x08},
	{0x39ed,0x00},
	{0x3e00,0x01},//
	{0x3e01,0x18},//
	{0x3e02,0xa0},//
	{0x3e08,0x03},//
	{0x3e09,0x40},//0x340
	{0x3e0e,0x09},
	{0x3e14,0x31},
	{0x3e16,0x00},
	{0x3e17,0xac},
	{0x3e18,0x00},
	{0x3e19,0xac},
	{0x3e1b,0x3a},
	{0x3e1e,0x76},
	{0x3e25,0x23},
	{0x3e26,0x40},
	{0x4501,0xa4},
	{0x4509,0x10},
	{0x4800,0x64},
	{0x4837,0x15},
	{0x5799,0x06},
	{0x57aa,0x2f},
	{0x57ab,0xff},
	{0x5988,0x86},
	{0x598e,0x08},
	{0x598f,0x18},
	{0x36e9,0x24},
	{0x36f9,0x24},
	{0x0100,0x01},
	{SC8238_REG_END, 0x00},/* END MARKER */
};

static struct tx_isp_sensor_win_setting sc8238_win_sizes[] = {
	{
		.width		= 3840,
		.height		= 2160,
		.fps		= 15 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc8238_init_regs_3840_2160_15fps_mipi,
	},
	{
		.width		= 3840,
		.height		= 2160,
		.fps		= 30 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc8238_init_regs_3840_2160_30fps_mipi,
	},
	{
		.width		= 3840,
		.height		= 2160,
		.fps		= 20 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc8238_init_regs_3840_2160_20fps_mipi,
	}
};
struct tx_isp_sensor_win_setting *wsize = &sc8238_win_sizes[0];

static struct regval_list sc8238_stream_on_mipi[] = {
	{0x0100, 0x01},
	{SC8238_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc8238_stream_off_mipi[] = {
	{0x0100, 0x00},
	{SC8238_REG_END, 0x00},	/* END MARKER */
};

int sc8238_read(struct tx_isp_subdev *sd, uint16_t reg,
		unsigned char *value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	uint8_t buf[2] = {(reg >> 8) & 0xff, reg & 0xff};
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

int sc8238_write(struct tx_isp_subdev *sd, uint16_t reg,
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
static int sc8238_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != SC8238_REG_END) {
		if (vals->reg_num == SC8238_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = sc8238_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}
#endif

static int sc8238_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;

	while (vals->reg_num != SC8238_REG_END) {
		if (vals->reg_num == SC8238_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = sc8238_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int sc8238_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int sc8238_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = sc8238_read(sd, 0x3107, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC8238_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = sc8238_read(sd, 0x3108, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC8238_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int sc8238_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = -1;
	int it = (value & 0xffff);
	int again = (value & 0xffff0000) >> 16;

	ret += sc8238_write(sd, 0x3e00, (unsigned char)((it >> 12) & 0xf));
	ret += sc8238_write(sd, 0x3e01, (unsigned char)((it >> 4) & 0xff));
	ret += sc8238_write(sd, 0x3e02, (unsigned char)((it & 0x0f) << 4));
	ret = sc8238_write(sd, 0x3e09, (unsigned char)(again & 0xff));
	ret += sc8238_write(sd, 0x3e08, (unsigned char)(((again >> 8) & 0xff)));

	/*
	   ret += sc8238_write(sd,0x3812,0x00);
	   if (again < 0x720) {
	   ret += sc8238_write(sd,0x3301,0x1c);
	   ret += sc8238_write(sd,0x3630,0x30);
	   ret += sc8238_write(sd,0x3633,0x23);
	   ret += sc8238_write(sd,0x3622,0xf6);
	   ret += sc8238_write(sd,0x363a,0x83);
	   }else if (again < 0xf20){
	   ret += sc8238_write(sd,0x3301,0x26);
	   ret += sc8238_write(sd,0x3630,0x23);
	   ret += sc8238_write(sd,0x3633,0x33);
	   ret += sc8238_write(sd,0x3622,0xf6);
	   ret += sc8238_write(sd,0x363a,0x87);
	   }else if(again < 0x1f20){
	   ret += sc8238_write(sd,0x3301,0x2c);
	   ret += sc8238_write(sd,0x3630,0x24);
	   ret += sc8238_write(sd,0x3633,0x43);
	   ret += sc8238_write(sd,0x3622,0xf6);
	   ret += sc8238_write(sd,0x363a,0x9f);
	   }else if(again < 0x1f3f){
	   ret += sc8238_write(sd,0x3301,0x38);
	   ret += sc8238_write(sd,0x3630,0x28);
	   ret += sc8238_write(sd,0x3633,0x43);
	   ret += sc8238_write(sd,0x3622,0xf6);
	   ret += sc8238_write(sd,0x363a,0x9f);
	   }else {
	   ret += sc8238_write(sd,0x3301,0x44);
	   ret += sc8238_write(sd,0x3630,0x19);
	   ret += sc8238_write(sd,0x3633,0x55);
	   ret += sc8238_write(sd,0x3622,0x16);
	   ret += sc8238_write(sd,0x363a,0x9f);
	   }
	   ret += sc8238_write(sd,0x3812,0x30);
	   if (ret < 0)
	   return ret;
	   */

	return 0;
}

#if 0
static int sc8238_set_integration_time(struct tx_isp_subdev *sd, int value)
{
       int ret = 0;

       value *= 2;
       ret = sc8238_write(sd, 0x3e00, (unsigned char)((value >> 12) & 0x0f));
       ret += sc8238_write(sd, 0x3e01, (unsigned char)((value >> 4) & 0xff));
       ret += sc8238_write(sd, 0x3e02, (unsigned char)((value & 0x0f) << 4));
       if (ret < 0)
               return ret;

       return 0;
}

static int sc8238_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
       int ret = 0;

       ret += sc8238_write(sd, 0x3e09, (unsigned char)(value & 0xff));
       ret += sc8238_write(sd, 0x3e08, (unsigned char)((value & 0xff00) >> 8));
       if (ret < 0)
               return ret;

       return 0;
}
#endif

static int sc8238_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc8238_get_black_pedestal(struct tx_isp_subdev *sd, int value)
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

static int sc8238_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int sc8238_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if (init->enable) {
		if(sensor->video.state == TX_ISP_MODULE_INIT){
			ret = sc8238_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_RUNNING;
		}
		if(sensor->video.state == TX_ISP_MODULE_RUNNING){

			ret = sc8238_write_array(sd, sc8238_stream_on_mipi);
			ISP_WARNING("sc8238 stream on\n");
		}
	}
	else {
		ret = sc8238_write_array(sd, sc8238_stream_off_mipi);
		ISP_WARNING("sc8238 stream off\n");
	}

	return ret;
}

static int sc8238_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	unsigned int sclk = 0;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned int max_fps;
	unsigned char val = 0;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	switch(sensor->info.default_boot){
	case 0:
		sclk = 74250000;
		max_fps = TX_SENSOR_MAX_FPS_15;
		break;
	case 1:
		sclk = 2100 * 2250 * 30;
		max_fps = TX_SENSOR_MAX_FPS_30;
		break;
	case 2:
		sclk = 2133 * 2250 * 20;
		max_fps = TX_SENSOR_MAX_FPS_20;
		break;
	default:
		ISP_ERROR("Now we do not support this framerate!!!\n");
	}

	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (max_fps<< 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)) {
		ISP_ERROR("warn: fps(%x) no in range\n", fps);
		return -1;
	}

	ret += sc8238_read(sd, 0x320c, &val);
	hts = val << 8;
	ret += sc8238_read(sd, 0x320d, &val);
	hts = (hts | val);
	if (0 != ret) {
		ISP_ERROR("err: sc8238 read err\n");
		return -1;
	}

	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);
	ret = sc8238_write(sd, 0x320f, (unsigned char)(vts & 0xff));
	ret += sc8238_write(sd, 0x320e, (unsigned char)(vts >> 8));
	if (0 != ret) {
		ISP_ERROR("err: sc8238_write err\n");
		return ret;
	}
	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = 2*vts - 10;
	sensor->video.attr->integration_time_limit = 2*vts - 10;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = 2*vts - 10;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc8238_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = ISP_SUCCESS;

	if(wsize){
		sensor_set_attr(sd, wsize);
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	}

	return ret;
}

static int sc8238_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	int ret = 0;
	uint8_t val;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	/* 2'b01:mirror,2'b10:filp */
	val = sc8238_read(sd, 0x3221, &val);
	switch(enable) {
	case 0:
		sc8238_write(sd, 0x3221, val & 0x99);
		break;
	case 1:
		sc8238_write(sd, 0x3221, val | 0x06);
		break;
	case 2:
		sc8238_write(sd, 0x3221, val | 0x60);
		break;
	case 3:
		sc8238_write(sd, 0x3221, val | 0x66);
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
	int ret;

	switch(info->default_boot){
	case 0:
		wsize = &sc8238_win_sizes[0];
		memcpy(&(sc8238_attr.mipi), &sc8238_mipi, sizeof(sc8238_mipi));
		sc8238_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		sc8238_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		sc8238_attr.max_integration_time_native = 2250*2 -10;
		sc8238_attr.integration_time_limit = 2250*2 -10;
		sc8238_attr.total_width = 2200 * 2;//4460
		sc8238_attr.total_height = 2250;//2250
		sc8238_attr.max_integration_time = 2250*2 -10;
		sc8238_attr.again =0;
		sc8238_attr.integration_time = 0x118a;
		break;
	case 1:
		wsize = &sc8238_win_sizes[1];
		memcpy(&(sc8238_attr.mipi), &sc8238_mipi, sizeof(sc8238_mipi));
		sc8238_attr.mipi.clk = 1418;
		sc8238_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		sc8238_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
		sc8238_attr.max_integration_time_native = 2250*2 -10;
		sc8238_attr.integration_time_limit = 2250*2 -10;
		sc8238_attr.total_width = 2100 * 2;//4200
		sc8238_attr.total_height = 2250;//2250
		sc8238_attr.max_integration_time = 2250*2 -10;
		sc8238_attr.again =0;
		sc8238_attr.integration_time = 0x118a;
		break;
	case 2:
		wsize = &sc8238_win_sizes[2];
		memcpy(&(sc8238_attr.mipi), &sc8238_mipi, sizeof(sc8238_mipi));
		sc8238_attr.mipi.clk = 960;
		sc8238_attr.data_type = TX_SENSOR_DATA_TYPE_LINEAR;
		sc8238_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
		sc8238_attr.max_integration_time_native = 2250*2 -10;
		sc8238_attr.integration_time_limit = 2250*2 -10;
		sc8238_attr.total_width = 2133 * 2;//4200
		sc8238_attr.total_height = 2250;//2250
		sc8238_attr.max_integration_time = 2250*2 -10;
		sc8238_attr.again = 0x340;
		sc8238_attr.integration_time = 0x118a;
		break;
	default:
		ISP_ERROR("Have no this Setting Source!!!\n");
	}

	switch(info->video_interface){
	case TISP_SENSOR_VI_MIPI_CSI0:
		sc8238_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		sc8238_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_DVP:
		sc8238_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
		break;
	default:
		ISP_ERROR("Have no this Interface Source!!!\n");
	}

	switch(info->mclk){
	case TISP_SENSOR_MCLK0:
	case TISP_SENSOR_MCLK1:
	case TISP_SENSOR_MCLK2:
                sclka = private_devm_clk_get(&client->dev, SEN_MCLK);
                sensor->mclk = private_devm_clk_get(sensor->dev, SEN_BCLK);
		set_sensor_mclk_function(0);
		break;
	default:
		ISP_ERROR("Have no this MCLK Source!!!\n");
	}

	rate = private_clk_get_rate(sensor->mclk);
	switch(info->default_boot){
	case 0:
                if (((rate / 1000) % 24000) != 0) {
                        ret = clk_set_parent(sclka, clk_get(NULL, SEN_TCLK));
                        sclka = private_devm_clk_get(&client->dev, SEN_TCLK);
                        if (IS_ERR(sclka)) {
                                pr_err("get sclka failed\n");
                        } else {
                                rate = private_clk_get_rate(sclka);
                                if (((rate / 1000) % 24000) != 0) {
                                        private_clk_set_rate(sclka, 1200000000);
                                }
                        }
                }
                private_clk_set_rate(sensor->mclk, 24000000);
                private_clk_prepare_enable(sensor->mclk);
                break;
        case 1:
                if (((rate / 1000) % 27000) != 0) {
                        ret = clk_set_parent(sclka, clk_get(NULL, SEN_TCLK));
                        sclka = private_devm_clk_get(&client->dev, SEN_TCLK);
                        if (IS_ERR(sclka)) {
                                pr_err("get sclka failed\n");
                        } else {
                                rate = private_clk_get_rate(sclka);
                                if (((rate / 1000) % 27000) != 0) {
                                        private_clk_set_rate(sclka, 1188000000);
                                }
                        }
                }
                private_clk_set_rate(sensor->mclk, 27000000);
                private_clk_prepare_enable(sensor->mclk);
                break;
        case 2:
                if (((rate / 1000) % 24000) != 0) {
                        ret = clk_set_parent(sclka, clk_get(NULL, SEN_TCLK));
                        sclka = private_devm_clk_get(&client->dev, SEN_TCLK);
                        if (IS_ERR(sclka)) {
                                pr_err("get sclka failed\n");
                        } else {
                                rate = private_clk_get_rate(sclka);
                                if (((rate / 1000) % 24000) != 0) {
                                        private_clk_set_rate(sclka, 1200000000);
                                }
                        }
                }
                private_clk_set_rate(sensor->mclk, 24000000);
                private_clk_prepare_enable(sensor->mclk);
                break;
	}

	ISP_WARNING("\n====>[default_boot=%d] [resolution=%dx%d] [video_interface=%d] [MCLK=%d] \n", info->default_boot, wsize->width, wsize->height, info->video_interface, info->mclk);
	reset_gpio = info->rst_gpio;
	pwdn_gpio = info->pwdn_gpio;

	sensor_set_attr(sd, wsize);
	sensor->priv = wsize;
        sensor->video.max_fps = wsize->fps;
	sensor->video.min_fps = SENSOR_OUTPUT_MIN_FPS << 16 | 1;
	return 0;

}

static int sc8238_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"sc8238_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(5);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"sc8238_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(5);
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(5);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = sc8238_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an sc8238 chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("sc8238 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "sc8238", sizeof("sc8238"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int sc8238_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = sc8238_set_expo(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		//if(arg)
		//	ret = sc8238_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		//if(arg)
		//	ret = sc8238_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = sc8238_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = sc8238_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = sc8238_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		ret = sc8238_write_array(sd, sc8238_stream_off_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		ret = sc8238_write_array(sd, sc8238_stream_on_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = sc8238_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = sc8238_set_vflip(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return ret;
}

static int sc8238_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = sc8238_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int sc8238_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	sc8238_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops sc8238_core_ops = {
	.g_chip_ident = sc8238_g_chip_ident,
	.reset = sc8238_reset,
	.init = sc8238_init,
	.g_register = sc8238_g_register,
	.s_register = sc8238_s_register,
};

static struct tx_isp_subdev_video_ops sc8238_video_ops = {
	.s_stream = sc8238_s_stream,
};

static struct tx_isp_subdev_sensor_ops	sc8238_sensor_ops = {
	.ioctl	= sc8238_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops sc8238_ops = {
	.core = &sc8238_core_ops,
	.video = &sc8238_video_ops,
	.sensor = &sc8238_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "sc8238",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int sc8238_probe(struct i2c_client *client,
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
	sc8238_attr.expo_fs = 1;
	sensor->video.shvflip = 1;
	sensor->video.attr = &sc8238_attr;
	tx_isp_subdev_init(&sensor_platform_device, sd, &sc8238_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->sc8238\n");

	return 0;
}

static int sc8238_remove(struct i2c_client *client)
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

static const struct i2c_device_id sc8238_id[] = {
	{ "sc8238", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sc8238_id);

static struct i2c_driver sc8238_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "sc8238",
	},
	.probe		= sc8238_probe,
	.remove		= sc8238_remove,
	.id_table	= sc8238_id,
};

static __init int init_sc8238(void)
{
	return private_i2c_add_driver(&sc8238_driver);
}

static __exit void exit_sc8238(void)
{
	private_i2c_del_driver(&sc8238_driver);
}

module_init(init_sc8238);
module_exit(exit_sc8238);

MODULE_DESCRIPTION("A low-level driver for Smartsens sc8238 sensors");
MODULE_LICENSE("GPL");
