/*
 * sc3336.c
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

#define SC3336_CHIP_ID_H	(0xcc)
#define SC3336_CHIP_ID_L	(0x41)
#define SC3336_REG_END		0xffff
#define SC3336_REG_DELAY	0xfffe
#define SC3336_SUPPORT_30FPS_SCLK (51000000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 2
#define SENSOR_VERSION	"H20211201a"
#define MCLK 24000000
static int reset_gpio = GPIO_PC(28);
static int pwdn_gpio = -1;
static int data_interface = TX_SENSOR_DATA_INTERFACE_MIPI;
static int shvflip = 0;
static int sensor_max_fps = TX_SENSOR_MAX_FPS_30;
char* __attribute__((weak)) sclk_name[4];
struct regval_list {
	uint16_t reg_num;
	unsigned char value;
};


/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct sensor_gain_lut {
	unsigned int index;
	unsigned char again;
	unsigned char coarse_dgain;
	unsigned char fine_dgain;
	unsigned int gain;
};

struct sensor_gain_lut sc3336_gain_lut[] = {
	/*{index, analog gain, digtal gain, digtal fine gain} // real gain*/
	{0x0, 0x0, 0x0, 0x80, 0},   // 1.000000
	{0x1, 0x0, 0x0, 0x84, 2909},   // 1.031250
	{0x2, 0x0, 0x0, 0x88, 5731},   // 1.062500
	{0x3, 0x0, 0x0, 0x8c, 8471},   // 1.093750
	{0x4, 0x0, 0x0, 0x90, 11135},   // 1.125000
	{0x5, 0x0, 0x0, 0x94, 13726},   // 1.156250
	{0x6, 0x0, 0x0, 0x98, 16247},   // 1.187500
	{0x7, 0x0, 0x0, 0x9c, 18703},   // 1.218750
	{0x8, 0x0, 0x0, 0xa0, 21097},   // 1.250000
	{0x9, 0x0, 0x0, 0xa4, 23431},   // 1.281250
	{0xa, 0x0, 0x0, 0xa8, 25710},   // 1.312500
	{0xb, 0x0, 0x0, 0xac, 27935},   // 1.343750
	{0xc, 0x0, 0x0, 0xb0, 30108},   // 1.375000
	{0xd, 0x0, 0x0, 0xb4, 32233},   // 1.406250
	{0xe, 0x0, 0x0, 0xb8, 34311},   // 1.437500
	{0xf, 0x0, 0x0, 0xbc, 36344},   // 1.468750
	{0x10, 0x0, 0x0, 0xc0, 38335},   // 1.500000
	{0x11, 0x40, 0x0, 0x80, 39558},   // 1.520000
	{0x12, 0x40, 0x0, 0x84, 42489},   // 1.567500
	{0x13, 0x40, 0x0, 0x88, 45275},   // 1.615000
	{0x14, 0x40, 0x0, 0x8c, 48037},   // 1.662500
	{0x15, 0x40, 0x0, 0x90, 50721},   // 1.710000
	{0x16, 0x40, 0x0, 0x94, 53278},   // 1.757500
	{0x17, 0x40, 0x0, 0x98, 55819},   // 1.805000
	{0x18, 0x40, 0x0, 0x9c, 58243},   // 1.852500
	{0x19, 0x40, 0x0, 0xa0, 60656},   // 1.900000
	{0x1a, 0x40, 0x0, 0xa4, 63008},   // 1.947500
	{0x1b, 0x40, 0x0, 0xa8, 65257},   // 1.995000
	{0x1c, 0x40, 0x0, 0xac, 67498},   // 2.042500
	{0x1d, 0x40, 0x0, 0xb0, 69689},   // 2.090000
	{0x1e, 0x40, 0x0, 0xb4, 71787},   // 2.137500
	{0x1f, 0x40, 0x0, 0xb8, 73881},   // 2.185000
	{0x20, 0x40, 0x0, 0xbc, 75929},   // 2.232500
	{0x21, 0x40, 0x0, 0xc0, 77894},   // 2.280000
	{0x22, 0x40, 0x0, 0xc4, 79858},   // 2.327500
	{0x23, 0x40, 0x0, 0xc8, 81783},   // 2.375000
	{0x24, 0x40, 0x0, 0xcc, 83631},   // 2.422500
	{0x25, 0x40, 0x0, 0xd0, 85480},   // 2.470000
	{0x26, 0x40, 0x0, 0xd4, 87258},   // 2.517500
	{0x27, 0x40, 0x0, 0xd8, 89040},   // 2.565000
	{0x28, 0x40, 0x0, 0xdc, 90787},   // 2.612500
	{0x29, 0x40, 0x0, 0xe0, 92469},   // 2.660000
	{0x2a, 0x40, 0x0, 0xe4, 94155},   // 2.707500
	{0x2b, 0x40, 0x0, 0xe8, 95812},   // 2.755000
	{0x2c, 0x40, 0x0, 0xec, 97407},   // 2.802500
	{0x2d, 0x40, 0x0, 0xf0, 99007},   // 2.850000
	{0x2e, 0x40, 0x0, 0xf4, 100582},   // 2.897500
	{0x2f, 0x40, 0x0, 0xf8, 102100},   // 2.945000
	{0x30, 0x40, 0x0, 0xfc, 103624},   // 2.992500
	{0x31, 0x48, 0x0, 0x80, 105094},   // 3.040000
	{0x32, 0x48, 0x0, 0x84, 108025},   // 3.135000
	{0x33, 0x48, 0x0, 0x88, 110839},   // 3.230000
	{0x34, 0x48, 0x0, 0x8c, 113573},   // 3.325000
	{0x35, 0x48, 0x0, 0x90, 116257},   // 3.420000
	{0x36, 0x48, 0x0, 0x94, 118840},   // 3.515000
	{0x37, 0x48, 0x0, 0x98, 121355},   // 3.610000
	{0x38, 0x48, 0x0, 0x9c, 123804},   // 3.705000
	{0x39, 0x48, 0x0, 0xa0, 126217},   // 3.800000
	{0x3a, 0x48, 0x0, 0xa4, 128544},   // 3.895000
	{0x3b, 0x48, 0x0, 0xa8, 130816},   // 3.990000
	{0x3c, 0x48, 0x0, 0xac, 133057},   // 4.085000
	{0x3d, 0x48, 0x0, 0xb0, 135225},   // 4.180000
	{0x3e, 0x48, 0x0, 0xb4, 137344},   // 4.275000
	{0x3f, 0x48, 0x0, 0xb8, 139417},   // 4.370000
	{0x40, 0x48, 0x0, 0xbc, 141465},   // 4.465000
	{0x41, 0x48, 0x0, 0xc0, 143450},   // 4.560000
	{0x42, 0x48, 0x0, 0xc4, 145394},   // 4.655000
	{0x43, 0x48, 0x0, 0xc8, 147319},   // 4.750000
	{0x44, 0x48, 0x0, 0xcc, 149186},   // 4.845000
	{0x45, 0x48, 0x0, 0xd0, 151016},   // 4.940000
	{0x46, 0x48, 0x0, 0xd4, 152813},   // 5.035000
	{0x47, 0x48, 0x0, 0xd8, 154593},   // 5.130000
	{0x48, 0x48, 0x0, 0xdc, 156323},   // 5.225000
	{0x49, 0x48, 0x0, 0xe0, 158022},   // 5.320000
	{0x4a, 0x48, 0x0, 0xe4, 159691},   // 5.415000
	{0x4b, 0x48, 0x0, 0xe8, 161348},   // 5.510000
	{0x4c, 0x48, 0x0, 0xec, 162960},   // 5.605000
	{0x4d, 0x48, 0x0, 0xf0, 164543},   // 5.700000
	{0x4e, 0x48, 0x0, 0xf4, 166118},   // 5.795000
	{0x4f, 0x48, 0x0, 0xf8, 167651},   // 5.890000
	{0x50, 0x48, 0x0, 0xfc, 169160},   // 5.985000
	{0x51, 0x49, 0x0, 0x80, 170645},   // 6.080000
	{0x52, 0x49, 0x0, 0x84, 173561},   // 6.270000
	{0x53, 0x49, 0x0, 0x88, 176390},   // 6.460000
	{0x54, 0x49, 0x0, 0x8c, 179123},   // 6.650000
	{0x55, 0x49, 0x0, 0x90, 181793},   // 6.840000
	{0x56, 0x49, 0x0, 0x94, 184376},   // 7.030000
	{0x57, 0x49, 0x0, 0x98, 186904},   // 7.220000
	{0x58, 0x49, 0x0, 0x9c, 189353},   // 7.410000
	{0x59, 0x49, 0x0, 0xa0, 191753},   // 7.600000
	{0x5a, 0x49, 0x0, 0xa4, 194080},   // 7.790000
	{0x5b, 0x49, 0x0, 0xa8, 196364},   // 7.980000
	{0x5c, 0x49, 0x0, 0xac, 198593},   // 8.170000
	{0x5d, 0x49, 0x0, 0xb0, 200761},   // 8.360000
	{0x5e, 0x49, 0x0, 0xb4, 202891},   // 8.550000
	{0x5f, 0x49, 0x0, 0xb8, 204962},   // 8.740000
	{0x60, 0x49, 0x0, 0xbc, 207001},   // 8.930000
	{0x61, 0x49, 0x0, 0xc0, 208986},   // 9.120000
	{0x62, 0x49, 0x0, 0xc4, 210941},   // 9.310000
	{0x63, 0x49, 0x0, 0xc8, 212855},   // 9.500000
	{0x64, 0x49, 0x0, 0xcc, 214722},   // 9.690000
	{0x65, 0x49, 0x0, 0xd0, 216562},   // 9.880000
	{0x66, 0x49, 0x0, 0xd4, 218358},   // 10.070000
	{0x67, 0x49, 0x0, 0xd8, 220129},   // 10.260000
	{0x68, 0x49, 0x0, 0xdc, 221859},   // 10.450000
	{0x69, 0x49, 0x0, 0xe0, 223567},   // 10.640000
	{0x6a, 0x49, 0x0, 0xe4, 225235},   // 10.830000
	{0x6b, 0x49, 0x0, 0xe8, 226884},   // 11.020000
	{0x6c, 0x49, 0x0, 0xec, 228503},   // 11.210000
	{0x6d, 0x49, 0x0, 0xf0, 230088},   // 11.400000
	{0x6e, 0x49, 0x0, 0xf4, 231654},   // 11.590000
	{0x6f, 0x49, 0x0, 0xf8, 233187},   // 11.780000
	{0x70, 0x49, 0x0, 0xfc, 234704},   // 11.970000
	{0x71, 0x4b, 0x0, 0x80, 236188},   // 12.160000
	{0x72, 0x4b, 0x0, 0x84, 239097},   // 12.540000
	{0x73, 0x4b, 0x0, 0x88, 241926},   // 12.920000
	{0x74, 0x4b, 0x0, 0x8c, 244666},   // 13.300000
	{0x75, 0x4b, 0x0, 0x90, 247329},   // 13.680000
	{0x76, 0x4b, 0x0, 0x94, 249918},   // 14.060000
	{0x77, 0x4b, 0x0, 0x98, 252440},   // 14.440000
	{0x78, 0x4b, 0x0, 0x9c, 254895},   // 14.820000
	{0x79, 0x4b, 0x0, 0xa0, 257289},   // 15.200000
	{0x7a, 0x4b, 0x0, 0xa4, 259622},   // 15.580000
	{0x7b, 0x4b, 0x0, 0xa8, 261906},   // 15.960000
	{0x7c, 0x4b, 0x0, 0xac, 264129},   // 16.340000
	{0x7d, 0x4b, 0x0, 0xb0, 266303},   // 16.719999
	{0x7e, 0x4b, 0x0, 0xb4, 268427},   // 17.100000
	{0x7f, 0x4b, 0x0, 0xb8, 270504},   // 17.480000
	{0x80, 0x4b, 0x0, 0xbc, 272537},   // 17.860001
	{0x81, 0x4b, 0x0, 0xc0, 274527},   // 18.240000
	{0x82, 0x4b, 0x0, 0xc4, 276477},   // 18.620001
	{0x83, 0x4b, 0x0, 0xc8, 278391},   // 19.000000
	{0x84, 0x4b, 0x0, 0xcc, 280263},   // 19.379999
	{0x85, 0x4b, 0x0, 0xd0, 282098},   // 19.760000
	{0x86, 0x4b, 0x0, 0xd4, 283898},   // 20.139999
	{0x87, 0x4b, 0x0, 0xd8, 285665},   // 20.520000
	{0x88, 0x4b, 0x0, 0xdc, 287399},   // 20.900000
	{0x89, 0x4b, 0x0, 0xe0, 289103},   // 21.280001
	{0x8a, 0x4b, 0x0, 0xe4, 290776},   // 21.660000
	{0x8b, 0x4b, 0x0, 0xe8, 292420},   // 22.040001
	{0x8c, 0x4b, 0x0, 0xec, 294039},   // 22.420000
	{0x8d, 0x4b, 0x0, 0xf0, 295628},   // 22.799999
	{0x8e, 0x4b, 0x0, 0xf4, 297190},   // 23.180000
	{0x8f, 0x4b, 0x0, 0xf8, 298727},   // 23.559999
	{0x90, 0x4b, 0x0, 0xfc, 300240},   // 23.940001
	{0x91, 0x4f, 0x0, 0x80, 301728},   // 24.320000
	{0x92, 0x4f, 0x0, 0x84, 304637},   // 25.080000
	{0x93, 0x4f, 0x0, 0x88, 307462},   // 25.840000
	{0x94, 0x4f, 0x0, 0x8c, 310202},   // 26.600000
	{0x95, 0x4f, 0x0, 0x90, 312865},   // 27.360001
	{0x96, 0x4f, 0x0, 0x94, 315454},   // 28.120001
	{0x97, 0x4f, 0x0, 0x98, 317979},   // 28.879999
	{0x98, 0x4f, 0x0, 0x9c, 320434},   // 29.639999
	{0x99, 0x4f, 0x0, 0xa0, 322827},   // 30.400000
	{0x9a, 0x4f, 0x0, 0xa4, 325161},   // 31.160000
	{0x9b, 0x4f, 0x0, 0xa8, 327442},   // 31.920000
	{0x9c, 0x4f, 0x0, 0xac, 329665},   // 32.680000
	{0x9d, 0x4f, 0x0, 0xb0, 331839},   // 33.439999
	{0x9e, 0x4f, 0x0, 0xb4, 333963},   // 34.200001
	{0x9f, 0x4f, 0x0, 0xb8, 336043},   // 34.959999
	{0xa0, 0x4f, 0x0, 0xbc, 338076},   // 35.720001
	{0xa1, 0x4f, 0x0, 0xc0, 340066},   // 36.480000
	{0xa2, 0x4f, 0x0, 0xc4, 342015},   // 37.240002
	{0xa3, 0x4f, 0x0, 0xc8, 343927},   // 38.000000
	{0xa4, 0x4f, 0x0, 0xcc, 345799},   // 38.759998
	{0xa5, 0x4f, 0x0, 0xd0, 347634},   // 39.520000
	{0xa6, 0x4f, 0x0, 0xd4, 349434},   // 40.279999
	{0xa7, 0x4f, 0x0, 0xd8, 351201},   // 41.040001
	{0xa8, 0x4f, 0x0, 0xdc, 352938},   // 41.799999
	{0xa9, 0x4f, 0x0, 0xe0, 354641},   // 42.560001
	{0xaa, 0x4f, 0x0, 0xe4, 356314},   // 43.320000
	{0xab, 0x4f, 0x0, 0xe8, 357958},   // 44.080002
	{0xac, 0x4f, 0x0, 0xec, 359575},   // 44.840000
	{0xad, 0x4f, 0x0, 0xf0, 361164},   // 45.599998
	{0xae, 0x4f, 0x0, 0xf4, 362726},   // 46.360001
	{0xaf, 0x4f, 0x0, 0xf8, 364263},   // 47.119999
	{0xb0, 0x4f, 0x0, 0xfc, 365777},   // 47.880001
	{0xb1, 0x5f, 0x0, 0x80, 367267},   // 48.639999
	{0xb2, 0x5f, 0x0, 0x84, 370175},   // 50.160000
	{0xb3, 0x5f, 0x0, 0x88, 372998},   // 51.680000
	{0xb4, 0x5f, 0x0, 0x8c, 375738},   // 53.200001
	{0xb5, 0x5f, 0x0, 0x90, 378402},   // 54.720001
	{0xb6, 0x5f, 0x0, 0x94, 380992},   // 56.240002
	{0xb7, 0x5f, 0x0, 0x98, 383515},   // 57.759998
	{0xb8, 0x5f, 0x0, 0x9c, 385970},   // 59.279999
	{0xb9, 0x5f, 0x0, 0xa0, 388364},   // 60.799999
	{0xba, 0x5f, 0x0, 0xa4, 390699},   // 62.320000
	{0xbb, 0x5f, 0x0, 0xa8, 392978},   // 63.840000
	{0xbc, 0x5f, 0x0, 0xac, 395201},   // 65.360001
	{0xbd, 0x5f, 0x0, 0xb0, 397375},   // 66.879997
	{0xbe, 0x5f, 0x0, 0xb4, 399499},   // 68.400002
	{0xbf, 0x5f, 0x0, 0xb8, 401579},   // 69.919998
	{0xc0, 0x5f, 0x0, 0xbc, 403612},   // 71.440002
	{0xc1, 0x5f, 0x0, 0xc0, 405602},   // 72.959999
	{0xc2, 0x5f, 0x0, 0xc4, 407551},   // 74.480003
	{0xc3, 0x5f, 0x0, 0xc8, 409463},   // 76.000000
	{0xc4, 0x5f, 0x0, 0xcc, 411335},   // 77.519997
	{0xc5, 0x5f, 0x0, 0xd0, 413170},   // 79.040001
	{0xc6, 0x5f, 0x0, 0xd4, 414970},   // 80.559998
	{0xc7, 0x5f, 0x0, 0xd8, 416737},   // 82.080002
	{0xc8, 0x5f, 0x0, 0xdc, 418474},   // 83.599998
	{0xc9, 0x5f, 0x0, 0xe0, 420177},   // 85.120003
	{0xca, 0x5f, 0x0, 0xe4, 421850},   // 86.639999
	{0xcb, 0x5f, 0x0, 0xe8, 423494},   // 88.160004
	{0xcc, 0x5f, 0x0, 0xec, 425111},   // 89.680000
	{0xcd, 0x5f, 0x0, 0xf0, 426700},   // 91.199997
	{0xce, 0x5f, 0x0, 0xf4, 428262},   // 92.720001
	{0xcf, 0x5f, 0x0, 0xf8, 429799},   // 94.239998
	{0xd0, 0x5f, 0x0, 0xfc, 431313},   // 95.760002
	{0xd1, 0x5f, 0x1, 0x80, 432803},   // 97.279999
};

struct tx_isp_sensor_attribute sc3336_attr;

unsigned int sc3336_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct sensor_gain_lut *lut = sc3336_gain_lut;
	while(lut->gain <= sc3336_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].index;
			return 0;
		} else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->index;
			return (lut - 1)->gain;
		} else {
			if((lut->gain == sc3336_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->index;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int sc3336_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_mipi_bus sc3336_mipi={
	.mode = SENSOR_MIPI_OTHER_MODE,
	.clk = 510,
	.lans = 2,
	.settle_time_apative_en = 0,
	.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
	.mipi_sc.hcrop_diff_en = 0,
	.mipi_sc.mipi_vcomp_en = 0,
	.mipi_sc.mipi_hcomp_en = 0,
	.mipi_sc.line_sync_mode = 0,
	.mipi_sc.work_start_flag = 0,
	.image_twidth = 2304,
	.image_theight = 1296,
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

struct tx_isp_sensor_attribute sc3336_attr={
	.name = "sc3336",
	.chip_id = 0xcc41,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x30,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.mode = SENSOR_MIPI_OTHER_MODE,
		.clk = 510,
		.lans = 2,
		.settle_time_apative_en = 1,
		.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
		.mipi_sc.hcrop_diff_en = 0,
		.mipi_sc.mipi_vcomp_en = 0,
		.mipi_sc.mipi_hcomp_en = 0,
		.mipi_sc.line_sync_mode = 0,
		.mipi_sc.work_start_flag = 0,
		.image_twidth = 2304,
		.image_theight = 1296,
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
	.max_again = 432803,
	.max_dgain = 0,
	.min_integration_time = 1,
	.min_integration_time_native = 1,
	.max_integration_time_native = 1632 - 8,
	.integration_time_limit = 1632 - 8,
	.total_width = 2500,
	.total_height = 1632,
	.max_integration_time = 1632 - 8,
	.one_line_expr_in_us = 25,
	.expo_fs = 1,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = sc3336_alloc_again,
	.sensor_ctrl.alloc_dgain = sc3336_alloc_dgain,
};

static struct regval_list sc3336_init_regs_2304_1296_30fps_mipi[] = {
	/*cleaned 0x03 24Minput 10bit 510Mbps*/
	{0x0103, 0x01},
	{0x36e9, 0x80},
	{0x37f9, 0x80},
	{0x301f, 0x03},
	{0x320e, 0x06},
	{0x320f, 0x60},/*vts 25fps*/
	{0x3253, 0x10},
	{0x325f, 0x20},
	{0x3301, 0x04},
	{0x3306, 0x50},
	{0x330a, 0x00},
	{0x330b, 0xd8},
	{0x3333, 0x10},
	{0x3334, 0x40},
	{0x335e, 0x06},
	{0x335f, 0x0a},
	{0x3364, 0x5e},
	{0x337c, 0x02},
	{0x337d, 0x0e},
	{0x3390, 0x01},
	{0x3391, 0x03},
	{0x3392, 0x07},
	{0x3393, 0x04},
	{0x3394, 0x04},
	{0x3395, 0x04},
	{0x3396, 0x08},
	{0x3397, 0x0b},
	{0x3398, 0x1f},
	{0x3399, 0x04},
	{0x339a, 0x0a},
	{0x339b, 0x3a},
	{0x339c, 0xb0},
	{0x33a2, 0x04},
	{0x33ac, 0x08},
	{0x33ad, 0x1c},
	{0x33ae, 0x10},
	{0x33af, 0x30},
	{0x33b1, 0x80},
	{0x33b3, 0x48},
	{0x33f9, 0x60},
	{0x33fb, 0x74},
	{0x33fc, 0x4b},
	{0x33fd, 0x5f},
	{0x349f, 0x03},
	{0x34a6, 0x4b},
	{0x34a7, 0x5f},
	{0x34a8, 0x20},
	{0x34a9, 0x18},
	{0x34ab, 0xe8},
	{0x34ad, 0xf8},
	{0x34f8, 0x5f},
	{0x34f9, 0x18},
	{0x3630, 0xc0},
	{0x3631, 0x84},
	{0x3633, 0x32},
	{0x363b, 0x26},
	{0x363c, 0x08},
	{0x3641, 0x38},
	{0x3670, 0x4e},
	{0x3674, 0xc0},
	{0x3675, 0xc0},
	{0x3676, 0xc0},
	{0x3677, 0x84},
	{0x3678, 0x8a},
	{0x3679, 0x8c},
	{0x367c, 0x48},
	{0x367d, 0x49},
	{0x367e, 0x4b},
	{0x367f, 0x5f},
	{0x3690, 0x33},
	{0x3691, 0x43},
	{0x3692, 0x54},
	{0x369c, 0x4b},
	{0x369d, 0x5f},
	{0x36b0, 0x87},
	{0x36b1, 0x90},
	{0x36b2, 0xa1},
	{0x36b3, 0xc2},
	{0x36b4, 0x49},
	{0x36b5, 0x4b},
	{0x36b6, 0x4f},
	{0x36ea, 0x11},
	{0x36eb, 0x0d},
	{0x36ec, 0x1c},
	{0x36ed, 0x26},
	{0x370f, 0x01},
	{0x3722, 0x09},
	{0x3724, 0x41},
	{0x3725, 0xc1},
	{0x3771, 0x09},
	{0x3772, 0x09},
	{0x3773, 0x05},
	{0x377a, 0x48},
	{0x377b, 0x5f},
	{0x37fa, 0x11},
	{0x37fb, 0x33},
	{0x37fc, 0x11},
	{0x37fd, 0x08},
	{0x3905, 0x8c},
	{0x391d, 0x04},
	{0x3921, 0x20},
	{0x3926, 0x21},
	{0x3933, 0x80},
	{0x3934, 0x03},
	{0x3935, 0x00},
	{0x3936, 0x12},
	{0x3937, 0x77},
	{0x3938, 0x74},
	{0x39dc, 0x02},
	{0x3e01, 0x65},
	{0x3e02, 0x80},
	{0x3e09, 0x00},
	{0x4509, 0x20},
	{0x5ae0, 0xfe},
	{0x5ae1, 0x40},
	{0x5ae2, 0x38},
	{0x5ae3, 0x30},
	{0x5ae4, 0x28},
	{0x5ae5, 0x38},
	{0x5ae6, 0x30},
	{0x5ae7, 0x28},
	{0x5ae8, 0x3f},
	{0x5ae9, 0x34},
	{0x5aea, 0x2c},
	{0x5aeb, 0x3f},
	{0x5aec, 0x34},
	{0x5aed, 0x2c},
	{0x36e9, 0x54},
	{0x37f9, 0x47},
	{0x0100, 0x01},
	{SC3336_REG_END, 0x00},	/* END MARKER */
};

static struct tx_isp_sensor_win_setting sc3336_win_sizes[] = {
	/* resolution 2304*1296 */
	{
		.width		= 2304,
		.height		= 1296,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SBGGR10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= sc3336_init_regs_2304_1296_30fps_mipi,
	},
};
struct tx_isp_sensor_win_setting *wsize = &sc3336_win_sizes[0];

/*
 * the part of driver was fixed.
 */
static struct regval_list sc3336_stream_on_mipi[] = {
	{0x0100, 0x01},
	{SC3336_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list sc3336_stream_off_mipi[] = {
	{0x0100, 0x00},
	{SC3336_REG_END, 0x00},	/* END MARKER */
};

int sc3336_read(struct tx_isp_subdev *sd, uint16_t reg, unsigned char *value)
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

int sc3336_write(struct tx_isp_subdev *sd, uint16_t reg, unsigned char value)
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
static int sc3336_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != SC3336_REG_END) {
		if (vals->reg_num == SC3336_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc3336_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}
#endif

static int sc3336_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != SC3336_REG_END) {
		if (vals->reg_num == SC3336_REG_DELAY) {
			private_msleep(vals->value);
		} else {
			ret = sc3336_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int sc3336_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int sc3336_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = sc3336_read(sd, 0x3107, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC3336_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = sc3336_read(sd, 0x3108, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != SC3336_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int sc3336_set_expo(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	int it = (value & 0xffff);
	int index = (value & 0xffff0000) >> 16;
	struct sensor_gain_lut *gain_lut = sc3336_gain_lut;

	/*set integration time*/
	ret += sc3336_write(sd, 0x3e00, (unsigned char)((it >> 12) & 0xf));
	ret += sc3336_write(sd, 0x3e01, (unsigned char)((it >> 4) & 0xff));
	ret += sc3336_write(sd, 0x3e02, (unsigned char)((it & 0x0f) << 4));
	/*set analog gain*/
	ret = sc3336_write(sd, 0x3e09, gain_lut[index].again);
	/*set coarse dgain*/
	ret = sc3336_write(sd, 0x3e06, gain_lut[index].coarse_dgain);
	/*set fine dgain*/
	ret = sc3336_write(sd, 0x3e07, gain_lut[index].fine_dgain);
	if (ret < 0)
		return ret;
	return 0;
}

#if 0
static int sc3336_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;

	value *= 2;
	ret += sc3336_write(sd, 0x3e00, (unsigned char)((value >> 12) & 0xf));
	ret += sc3336_write(sd, 0x3e01, (unsigned char)((value >> 4) & 0xff));
	ret += sc3336_write(sd, 0x3e02, (unsigned char)((value & 0x0f) << 4));
	if (ret < 0)
		return ret;

	return 0;
}

static int sc3336_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
	struct sensor_gain_lut *gain_lut = sc3336_gain_lut;
	int index = value;

	/*set analog gain*/
	ret = sc3336_write(sd, 0x3e09, gain_lut[index].again);
	/*set coarse dgain*/
	ret = sc3336_write(sd, 0x3e06, gain_lut[index].coarse_dgain);
	/*set fine dgain*/
	ret = sc3336_write(sd, 0x3e07, gain_lut[index].fine_dgain);
	if (ret < 0)
		return ret;

	return 0;
}
#endif

static int sc3336_set_logic(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc3336_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc3336_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int sc3336_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
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

static int sc3336_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
		if (sensor->video.state == TX_ISP_MODULE_DEINIT){
			ret = sc3336_write_array(sd, wsize->regs);
			if (ret)
				return ret;
			sensor->video.state = TX_ISP_MODULE_INIT;
		}
		if (sensor->video.state == TX_ISP_MODULE_INIT){
		     if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
				ret = sc3336_write_array(sd, sc3336_stream_on_mipi);

			}else{
				ISP_ERROR("Don't support this Sensor Data interface\n");
			}
			ISP_WARNING("sc3336 stream on\n");
			sensor->video.state = TX_ISP_MODULE_RUNNING;
		}
	}
	else {
		 if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc3336_write_array(sd, sc3336_stream_off_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		ISP_WARNING("sc3336 stream off\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
	}

	return ret;
}

static int sc3336_set_fps(struct tx_isp_subdev *sd, int fps)
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
		ISP_ERROR("warn: fps(%d) not in range\n", fps);
		return -1;
	}
	sclk = SC3336_SUPPORT_30FPS_SCLK;

	ret = sc3336_read(sd, 0x320c, &tmp);
	hts = tmp;
	ret += sc3336_read(sd, 0x320d, &tmp);
	if (0 != ret) {
		ISP_ERROR("err: sc3336 read err\n");
		return ret;
	}
	hts = (hts << 8) + tmp;
	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);

	ret += sc3336_write(sd, 0x320f, (unsigned char)(vts & 0xff));
	ret += sc3336_write(sd, 0x320e, (unsigned char)(vts >> 8));
	if (0 != ret) {
		ISP_ERROR("err: sc3336_write err\n");
		return ret;
	}

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 8;
	sensor->video.attr->integration_time_limit = vts - 8;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 8;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc3336_set_mode(struct tx_isp_subdev *sd, int value)
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
        struct clk *tclk;
	unsigned long rate;
	uint8_t i;
	int ret = 0;

	switch (info->default_boot) {
	case 0:
		wsize = &sc3336_win_sizes[0];
		memcpy((void*)(&(sc3336_attr.mipi)),(void*)(&sc3336_mipi),sizeof(sc3336_mipi));
		sensor_max_fps = TX_SENSOR_MAX_FPS_30;
		break;
	default:
		ISP_ERROR("this boot setting is not supported yet!!!\n");
	}

	switch (info->video_interface) {
	case TISP_SENSOR_VI_MIPI_CSI0:
		sc3336_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		sc3336_attr.mipi.index = 0;
		break;
	case TISP_SENSOR_VI_MIPI_CSI1:
		sc3336_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
		sc3336_attr.mipi.index = 1;
		break;
	case TISP_SENSOR_VI_DVP:
		sc3336_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
		break;
	default:
		ISP_ERROR("this interface is not supported yet!!!\n");
	}

	switch (info->mclk) {
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
    if (((rate / 1000) % (MCLK / 1000)) != 0) {
        uint8_t sclk_name_num = sizeof(sclk_name)/sizeof(sclk_name[0]);
        for (i=0; i < sclk_name_num; i++) {
            tclk = private_devm_clk_get(&client->dev, sclk_name[i]);
            ret = clk_set_parent(sclka, clk_get(NULL, sclk_name[i]));
            if (IS_ERR(tclk)) {
                pr_err("get sclka failed\n");
            } else {
                rate = private_clk_get_rate(tclk);
                if (i == sclk_name_num - 1 && ((rate / 1000) % (MCLK / 1000)) != 0) {
                    if (((MCLK / 1000) % 27000) != 0 || ((MCLK / 1000) % 37125) != 0)
                        private_clk_set_rate(tclk, 891000000);
                    else if (((MCLK / 1000) % 24000) != 0)
                        private_clk_set_rate(tclk, 1200000000);
                } else if (((rate / 1000) % (MCLK / 1000)) == 0) {
                    break;
                }
            }
        }
    }

	private_clk_set_rate(sensor->mclk, MCLK);
	private_clk_prepare_enable(sensor->mclk);

	reset_gpio = info->rst_gpio;
	pwdn_gpio = info->pwdn_gpio;

	return 0;

}

static int sc3336_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"sc3336_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(50);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(20);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(20);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"sc3336_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(10);
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(10);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = sc3336_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an sc3336 chip.\n",
			  client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("sc3336 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "sc3336", sizeof("sc3336"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}
	return 0;
}

static int sc3336_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = -1;
	unsigned char val = 0x0;

	ret += sc3336_read(sd, 0x3221, &val);

	if(enable & 0x2)
		val |= 0x60;
	else
		val &= 0x9f;

	ret += sc3336_write(sd, 0x3221, val);

	if(!ret)
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int sc3336_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
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
			ret = sc3336_set_expo(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		//		if(arg)
		//			ret = sc3336_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		//		if(arg)
		//			ret = sc3336_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = sc3336_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = sc3336_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = sc3336_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		 if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc3336_write_array(sd, sc3336_stream_off_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
		}
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		 if (data_interface == TX_SENSOR_DATA_INTERFACE_MIPI){
			ret = sc3336_write_array(sd, sc3336_stream_on_mipi);

		}else{
			ISP_ERROR("Don't support this Sensor Data interface\n");
			ret = -1;
		}
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = sc3336_set_fps(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = sc3336_set_vflip(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_LOGIC:
		if(arg)
			ret = sc3336_set_logic(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return ret;
}

static int sc3336_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
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
	ret = sc3336_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int sc3336_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	sc3336_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops sc3336_core_ops = {
	.g_chip_ident = sc3336_g_chip_ident,
	.reset = sc3336_reset,
	.init = sc3336_init,
	/*.ioctl = sc3336_ops_ioctl,*/
	.g_register = sc3336_g_register,
	.s_register = sc3336_s_register,
};

static struct tx_isp_subdev_video_ops sc3336_video_ops = {
	.s_stream = sc3336_s_stream,
};

static struct tx_isp_subdev_sensor_ops	sc3336_sensor_ops = {
	.ioctl	= sc3336_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops sc3336_ops = {
	.core = &sc3336_core_ops,
	.video = &sc3336_video_ops,
	.sensor = &sc3336_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "sc3336",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int sc3336_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

	sc3336_attr.expo_fs = 1;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->dev = &client->dev;
	sensor->video.shvflip = shvflip;
	sensor->video.attr = &sc3336_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &sc3336_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->sc3336\n");

	return 0;
}

static int sc3336_remove(struct i2c_client *client)
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

static const struct i2c_device_id sc3336_id[] = {
	{ "sc3336", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sc3336_id);

static struct i2c_driver sc3336_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "sc3336",
	},
	.probe		= sc3336_probe,
	.remove		= sc3336_remove,
	.id_table	= sc3336_id,
};

static __init int init_sc3336(void)
{
	return private_i2c_add_driver(&sc3336_driver);
}

static __exit void exit_sc3336(void)
{
	private_i2c_del_driver(&sc3336_driver);
}

module_init(init_sc3336);
module_exit(exit_sc3336);

MODULE_DESCRIPTION("A low-level driver for SmartSens sc3336 sensors");
MODULE_LICENSE("GPL");
