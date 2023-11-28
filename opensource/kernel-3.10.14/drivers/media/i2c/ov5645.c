#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-mediabus.h>
//#include <media/ov5645.h>
#include <mach/ovisp-v4l2.h>
#include "ov5645.h"


#define OV5645_CHIP_ID_H	(0x56)
#define OV5645_CHIP_ID_L	(0x45)

#define SXGA_WIDTH		1280
#define SXGA_HEIGHT		960
#define VGA_WIDTH		640
#define VGA_HEIGHT		480
#define _720P_WIDTH		1280
#define _720P_HEIGHT		720
#define _1080P_WIDTH		1920
#define _1080P_HEIGHT	1080

#define MAX_WIDTH		2592
#define MAX_HEIGHT		1944

#define OV5645_REG_END		0xffff
#define OV5645_REG_DELAY	0xfffe

//#define OV5645_YUV

struct ov5645_format_struct;
struct ov5645_info {
	struct v4l2_subdev sd;
	struct ov5645_format_struct *fmt;
	struct ov5645_win_setting *win;
};

struct regval_list {
	unsigned short reg_num;
	unsigned char value;
};


static struct regval_list ov5645_init_regs_672Mbps_5M[] = {
/*
	@@ MIPI_2lane_5M(RAW8) 15fps MIPI CLK = 336MHz
99 2592 1944
98 1 0
64 1000 7
102 3601 5dc
;
;OV5645 setting Version History
;dated 08/28/2013 A07
;--updated VGA/SXGA binning setting
;--updated CIP auto gain setting.
;
;dated 08/07/2013 A06b
;--updated DPC settings
;
;dated 08/28/2012 A05
;--Based on v05 release
;
;dated 06/13/2012 A03
;--Based on v03 release
;
;dated 08/28/2012 A05
;--Based on V05 release
;
;dated 04/09/2012
;--Add 4050/4051 BLC level trigger
*/
	{0x3103 ,0x11},
	{0x3008 ,0x82},
	{0x4202, 0x0f},

	{0x3008 ,0x42},
	{0x3103 ,0x03},
	{0x3503 ,0x07},
	{0x3002 ,0x1c},
	{0x3006 ,0xc3},
	{0x300e ,0x45},
	{0x3017 ,0x40},
	{0x3018 ,0x00},
	{0x302e ,0x0b},
	{0x3037 ,0x13},
	{0x3108 ,0x01},
	{0x3611 ,0x06},
	{0x3612 ,0xab},
	{0x3614 ,0x50},
	{0x3618 ,0x04},
	{0x3034 ,0x18},
	{0x3035 ,0x11},
	{0x3036 ,0x54},
	{0x3500 ,0x00},
	{0x3501 ,0x01},
	{0x3502 ,0x00},
	{0x350a ,0x00},
	{0x350b ,0x3f},
	{0x3600 ,0x08},
	{0x3601 ,0x33},
	{0x3620 ,0x33},
	{0x3621 ,0xe0},
	{0x3622 ,0x01},
	{0x3630 ,0x2d},
	{0x3631 ,0x00},
	{0x3632 ,0x32},
	{0x3633 ,0x52},
	{0x3634 ,0x70},
	{0x3635 ,0x13},
	{0x3636 ,0x03},
	{0x3702 ,0x6e},
	{0x3703 ,0x52},
	{0x3704 ,0xa0},
	{0x3705 ,0x33},
	{0x3708 ,0x63},
	{0x3709 ,0x12},
	{0x370b ,0x61},
	{0x370c ,0xc0},
	{0x370f ,0x10},
	{0x3715 ,0x08},
	{0x3717 ,0x01},
	{0x371b ,0x20},
	{0x3731 ,0x22},
	{0x3739 ,0x70},
	{0x3901 ,0x0a},
	{0x3905 ,0x02},
	{0x3906 ,0x10},
	{0x3719 ,0x86},
	{0x3800 ,0x00},
	{0x3801 ,0x00},
	{0x3802 ,0x00},
	{0x3803 ,0x00},
	{0x3804 ,0x0a},
	{0x3805 ,0x3f},
	{0x3806 ,0x07},
	{0x3807 ,0x9f},
	{0x3808 ,0x0a},
	{0x3809 ,0x20},
	{0x380a ,0x07},
	{0x380b ,0x98},
	{0x380c ,0x0b},
	{0x380d ,0x1c},
	{0x380e ,0x07},
	{0x380f ,0xb0},
	{0x3810 ,0x00},
	{0x3811 ,0x10},
	{0x3812 ,0x00},
	{0x3813 ,0x06},
	{0x3814 ,0x11},
	{0x3815 ,0x11},
	{0x3820 ,0x40},
	{0x3821 ,0x06},
	{0x3824 ,0x01},
	{0x3826 ,0x03},
	{0x3828 ,0x08},
	{0x3a02 ,0x07},
	{0x3a03 ,0xb0},
	{0x3a08 ,0x01},
	{0x3a09 ,0x27},
	{0x3a0a ,0x00},
	{0x3a0b ,0xf6},
	{0x3a0e ,0x06},
	{0x3a0d ,0x08},
	{0x3a14 ,0x07},
	{0x3a15 ,0xb0},
	{0x3a18 ,0x00},
	{0x3a19 ,0xf8},
	{0x3c01 ,0x34},
	{0x3c04 ,0x28},
	{0x3c05 ,0x98},
	{0x3c07 ,0x07},
	{0x3c09 ,0xc2},
	{0x3c0a ,0x9c},
	{0x3c0b ,0x40},
	{0x3c01 ,0x34},
	{0x4001 ,0x02},
	{0x4004 ,0x06},
	{0x4005 ,0x18},
	{0x4050 ,0x6e},
	{0x4051 ,0x8f},
	{0x4300 ,0x30},
	{0x4514 ,0x00},
	{0x4520 ,0xb0},
	{0x460b ,0x37},
	{0x460c ,0x20},
	{0x4818 ,0x01},
	{0x481d ,0xf0},
	{0x481f ,0x50},
	{0x4823 ,0x70},
	{0x4831 ,0x14},
	{0x4837 ,0x0b},
	{0x5000 ,0xa7},
	{0x5001 ,0x83},
	{0x501d ,0x00},
	{0x501f ,0x00},
	{0x503d ,0x00},
	{0x505c ,0x30},
	{0x5181 ,0x59},
	{0x5183 ,0x00},
	{0x5191 ,0xf0},
	{0x5192 ,0x03},
	{0x5684 ,0x10},
	{0x5685 ,0xa0},
	{0x5686 ,0x0c},
	{0x5687 ,0x78},
	{0x5a00 ,0x08},
	{0x5a21 ,0x00},
	{0x5a24 ,0x00},
	{0x3008 ,0x02},
	{0x3503 ,0x00},
	{0x5180 ,0xff},
	{0x5181 ,0xf2},
	{0x5182 ,0x00},
	{0x5183 ,0x14},
	{0x5184 ,0x25},
	{0x5185 ,0x24},
	{0x5186 ,0x09},
	{0x5187 ,0x09},
	{0x5188 ,0x0a},
	{0x5189 ,0x75},
	{0x518a ,0x52},
	{0x518b ,0xea},
	{0x518c ,0xa8},
	{0x518d ,0x42},
	{0x518e ,0x38},
	{0x518f ,0x56},
	{0x5190 ,0x42},
	{0x5191 ,0xf8},
	{0x5192 ,0x04},
	{0x5193 ,0x70},
	{0x5194 ,0xf0},
	{0x5195 ,0xf0},
	{0x5196 ,0x03},
	{0x5197 ,0x01},
	{0x5198 ,0x04},
	{0x5199 ,0x12},
	{0x519a ,0x04},
	{0x519b ,0x00},
	{0x519c ,0x06},
	{0x519d ,0x82},
	{0x519e ,0x38},
	{0x5381 ,0x1e},
	{0x5382 ,0x5b},
	{0x5383 ,0x08},
	{0x5384 ,0x0b},
	{0x5385 ,0x84},
	{0x5386 ,0x8f},
	{0x5387 ,0x82},
	{0x5388 ,0x71},
	{0x5389 ,0x11},
	{0x538a ,0x01},
	{0x538b ,0x98},
	{0x5300 ,0x08},
	{0x5301 ,0x1e},
	{0x5302 ,0x10},
	{0x5303 ,0x00},
	{0x5304 ,0x08},
	{0x5305 ,0x1e},
	{0x5306 ,0x08},
	{0x5307 ,0x16},
	{0x5309 ,0x08},
	{0x530a ,0x1e},
	{0x530b ,0x04},
	{0x530c ,0x06},
	{0x5480 ,0x01},
	{0x5481 ,0x0e},
	{0x5482 ,0x18},
	{0x5483 ,0x2b},
	{0x5484 ,0x52},
	{0x5485 ,0x65},
	{0x5486 ,0x71},
	{0x5487 ,0x7d},
	{0x5488 ,0x87},
	{0x5489 ,0x91},
	{0x548a ,0x9a},
	{0x548b ,0xaa},
	{0x548c ,0xb8},
	{0x548d ,0xcd},
	{0x548e ,0xdd},
	{0x548f ,0xea},
	{0x5490 ,0x1d},
	{0x5580 ,0x02},
	{0x5583 ,0x40},
	{0x5584 ,0x30},
	{0x5589 ,0x10},
	{0x558a ,0x00},
	{0x558b ,0xf8},
	{0x5780 ,0xfc},
	{0x5781 ,0x13},
	{0x5782 ,0x03},
	{0x5786 ,0x20},
	{0x5787 ,0x40},
	{0x5788 ,0x08},
	{0x5789 ,0x08},
	{0x578a ,0x02},
	{0x578b ,0x01},
	{0x578c ,0x01},
	{0x578d ,0x0c},
	{0x578e ,0x02},
	{0x578f ,0x01},
	{0x5790 ,0x01},
	{0x5800 ,0x3f},
	{0x5801 ,0x16},
	{0x5802 ,0x0e},
	{0x5803 ,0x0d},
	{0x5804 ,0x17},
	{0x5805 ,0x3f},
	{0x5806 ,0x0b},
	{0x5807 ,0x06},
	{0x5808 ,0x04},
	{0x5809 ,0x04},
	{0x580a ,0x06},
	{0x580b ,0x0b},
	{0x580c ,0x09},
	{0x580d ,0x03},
	{0x580e ,0x00},
	{0x580f ,0x00},
	{0x5810 ,0x03},
	{0x5811 ,0x08},
	{0x5812 ,0x0a},
	{0x5813 ,0x03},
	{0x5814 ,0x00},
	{0x5815 ,0x00},
	{0x5816 ,0x04},
	{0x5817 ,0x09},
	{0x5818 ,0x0f},
	{0x5819 ,0x08},
	{0x581a ,0x06},
	{0x581b ,0x06},
	{0x581c ,0x08},
	{0x581d ,0x0c},
	{0x581e ,0x3f},
	{0x581f ,0x1e},
	{0x5820 ,0x12},
	{0x5821 ,0x13},
	{0x5822 ,0x21},
	{0x5823 ,0x3f},
	{0x5824 ,0x68},
	{0x5825 ,0x28},
	{0x5826 ,0x2c},
	{0x5827 ,0x28},
	{0x5828 ,0x08},
	{0x5829 ,0x48},
	{0x582a ,0x64},
	{0x582b ,0x62},
	{0x582c ,0x64},
	{0x582d ,0x28},
	{0x582e ,0x46},
	{0x582f ,0x62},
	{0x5830 ,0x60},
	{0x5831 ,0x62},
	{0x5832 ,0x26},
	{0x5833 ,0x48},
	{0x5834 ,0x66},
	{0x5835 ,0x44},
	{0x5836 ,0x64},
	{0x5837 ,0x28},
	{0x5838 ,0x66},
	{0x5839 ,0x48},
	{0x583a ,0x2c},
	{0x583b ,0x28},
	{0x583c ,0x26},
	{0x583d ,0xae},
	{0x5025 ,0x00},
	{0x3a0f ,0x38},
	{0x3a10 ,0x30},
	{0x3a11 ,0x70},
	{0x3a1b ,0x38},
	{0x3a1e ,0x30},
	{0x3a1f ,0x18},
	{0x4300 ,0xf8},
	{0x501f ,0x03},
	{0x3008 ,0x02},

	{OV5645_REG_END, 0x00},	/* END MARKER */

};

static struct regval_list ov5645_init_regs_672Mbps_1080p[] = {
	/*
	   @@ MIPI_2lane_1080P(RAW8) 30fps MIPI CLK = 336MHz
	   99 1920 1080
	   98 1 0
	   64 1000 7
	   102 3601 bb8
	   */
	{0x3103 ,0x11},
	{0x3008 ,0x82},
	{0x4202, 0x0f},

	{0x3008 ,0x42},
	{0x3103 ,0x03},
	{0x3503 ,0x07},
	{0x3002 ,0x1c},
	{0x3006 ,0xc3},
	{0x300e ,0x45},
	{0x3017 ,0x40},
	{0x3018 ,0x00},
	{0x302e ,0x0b},
	{0x3037 ,0x13},
	{0x3108 ,0x01},
	{0x3611 ,0x06},
	{0x3612 ,0xab},
	{0x3614 ,0x50},
	{0x3618 ,0x04},
	{0x3034 ,0x18},
	{0x3035 ,0x11},
	{0x3036 ,0x54},
	{0x3500 ,0x00},
	{0x3501 ,0x01},
	{0x3502 ,0x00},
	{0x350a ,0x00},
	{0x350b ,0x3f},
	{0x3600 ,0x08},
	{0x3601 ,0x33},
	{0x3620 ,0x33},
	{0x3621 ,0xe0},
	{0x3622 ,0x01},
	{0x3630 ,0x2d},
	{0x3631 ,0x00},
	{0x3632 ,0x32},
	{0x3633 ,0x52},
	{0x3634 ,0x70},
	{0x3635 ,0x13},
	{0x3636 ,0x03},
	{0x3702 ,0x6e},
	{0x3703 ,0x52},
	{0x3704 ,0xa0},
	{0x3705 ,0x33},
	{0x3708 ,0x63},
	{0x3709 ,0x12},
	{0x370b ,0x61},
	{0x370c ,0xc0},
	{0x370f ,0x10},
	{0x3715 ,0x08},
	{0x3717 ,0x01},
	{0x371b ,0x20},
	{0x3731 ,0x22},
	{0x3739 ,0x70},
	{0x3901 ,0x0a},
	{0x3905 ,0x02},
	{0x3906 ,0x10},
	{0x3719 ,0x86},
	{0x3800 ,0x01},
	{0x3801 ,0x50},
	{0x3802 ,0x01},
	{0x3803 ,0xb2},
	{0x3804 ,0x08},
	{0x3805 ,0xef},
	{0x3806 ,0x05},
	{0x3807 ,0xf1},
	{0x3808 ,0x07},
	{0x3809 ,0x80},
	{0x380a ,0x04},
	{0x380b ,0x38},
	{0x380c ,0x09},
	{0x380d ,0xc4},
	{0x380e ,0x04},
	{0x380f ,0x60},
	{0x3810 ,0x00},
	{0x3811 ,0x10},
	{0x3812 ,0x00},
	{0x3813 ,0x04},
	{0x3814 ,0x11},
	{0x3815 ,0x11},
	{0x3820 ,0x40},
	{0x3821 ,0x06},
	{0x3824 ,0x01},
	{0x3826 ,0x03},
	{0x3828 ,0x08},
	{0x3a02 ,0x04},
	{0x3a03 ,0x60},
	{0x3a08 ,0x01},
	{0x3a09 ,0x50},
	{0x3a0a ,0x01},
	{0x3a0b ,0x18},
	{0x3a0e ,0x03},
	{0x3a0d ,0x04},
	{0x3a14 ,0x04},
	{0x3a15 ,0x60},
	{0x3a18 ,0x00},
	{0x3a19 ,0xf8},
	{0x3c01 ,0x34},
	{0x3c04 ,0x28},
	{0x3c05 ,0x98},
	{0x3c07 ,0x07},
	{0x3c09 ,0xc2},
	{0x3c0a ,0x9c},
	{0x3c0b ,0x40},
	{0x3c01 ,0x34},
	{0x4001 ,0x02},
	{0x4004 ,0x06},
	{0x4005 ,0x18},
	{0x4050 ,0x6e},
	{0x4051 ,0x8f},
	{0x4300 ,0x30},
	{0x4514 ,0x00},
	{0x4520 ,0xb0},
	{0x460b ,0x37},
	{0x460c ,0x20},
	{0x4818 ,0x01},
	{0x481d ,0xf0},
	{0x481f ,0x50},
	{0x4823 ,0x70},
	{0x4831 ,0x14},
	{0x4837 ,0x0b},
	{0x5000 ,0xa7},
	{0x5001 ,0x83},
	{0x501d ,0x00},
	{0x501f ,0x00},
	{0x503d ,0x00},
	{0x505c ,0x30},
	{0x5181 ,0x59},
	{0x5183 ,0x00},
	{0x5191 ,0xf0},
	{0x5192 ,0x03},
	{0x5684 ,0x10},
	{0x5685 ,0xa0},
	{0x5686 ,0x0c},
	{0x5687 ,0x78},
	{0x5a00 ,0x08},
	{0x5a21 ,0x00},
	{0x5a24 ,0x00},
	{0x3008 ,0x02},
	{0x3503 ,0x00},
	{0x5180 ,0xff},
	{0x5181 ,0xf2},
	{0x5182 ,0x00},
	{0x5183 ,0x14},
	{0x5184 ,0x25},
	{0x5185 ,0x24},
	{0x5186 ,0x09},
	{0x5187 ,0x09},
	{0x5188 ,0x0a},
	{0x5189 ,0x75},
	{0x518a ,0x52},
	{0x518b ,0xea},
	{0x518c ,0xa8},
	{0x518d ,0x42},
	{0x518e ,0x38},
	{0x518f ,0x56},
	{0x5190 ,0x42},
	{0x5191 ,0xf8},
	{0x5192 ,0x04},
	{0x5193 ,0x70},
	{0x5194 ,0xf0},
	{0x5195 ,0xf0},
	{0x5196 ,0x03},
	{0x5197 ,0x01},
	{0x5198 ,0x04},
	{0x5199 ,0x12},
	{0x519a ,0x04},
	{0x519b ,0x00},
	{0x519c ,0x06},
	{0x519d ,0x82},
	{0x519e ,0x38},
	{0x5381 ,0x1e},
	{0x5382 ,0x5b},
	{0x5383 ,0x08},
	{0x5384 ,0x0b},
	{0x5385 ,0x84},
	{0x5386 ,0x8f},
	{0x5387 ,0x82},
	{0x5388 ,0x71},
	{0x5389 ,0x11},
	{0x538a ,0x01},
	{0x538b ,0x98},
	{0x5300 ,0x08},
	{0x5301 ,0x1e},
	{0x5302 ,0x10},
	{0x5303 ,0x00},
	{0x5304 ,0x08},
	{0x5305 ,0x1e},
	{0x5306 ,0x08},
	{0x5307 ,0x16},
	{0x5309 ,0x08},
	{0x530a ,0x1e},
	{0x530b ,0x04},
	{0x530c ,0x06},
	{0x5480 ,0x01},
	{0x5481 ,0x0e},
	{0x5482 ,0x18},
	{0x5483 ,0x2b},
	{0x5484 ,0x52},
	{0x5485 ,0x65},
	{0x5486 ,0x71},
	{0x5487 ,0x7d},
	{0x5488 ,0x87},
	{0x5489 ,0x91},
	{0x548a ,0x9a},
	{0x548b ,0xaa},
	{0x548c ,0xb8},
	{0x548d ,0xcd},
	{0x548e ,0xdd},
	{0x548f ,0xea},
	{0x5490 ,0x1d},
	{0x5580 ,0x02},
	{0x5583 ,0x40},
	{0x5584 ,0x30},
	{0x5589 ,0x10},
	{0x558a ,0x00},
	{0x558b ,0xf8},
	{0x5780 ,0xfc},
	{0x5781 ,0x13},
	{0x5782 ,0x03},
	{0x5786 ,0x20},
	{0x5787 ,0x40},
	{0x5788 ,0x08},
	{0x5789 ,0x08},
	{0x578a ,0x02},
	{0x578b ,0x01},
	{0x578c ,0x01},
	{0x578d ,0x0c},
	{0x578e ,0x02},
	{0x578f ,0x01},
	{0x5790 ,0x01},
	{0x5800 ,0x3f},
	{0x5801 ,0x16},
	{0x5802 ,0x0e},
	{0x5803 ,0x0d},
	{0x5804 ,0x17},
	{0x5805 ,0x3f},
	{0x5806 ,0x0b},
	{0x5807 ,0x06},
	{0x5808 ,0x04},
	{0x5809 ,0x04},
	{0x580a ,0x06},
	{0x580b ,0x0b},
	{0x580c ,0x09},
	{0x580d ,0x03},
	{0x580e ,0x00},
	{0x580f ,0x00},
	{0x5810 ,0x03},
	{0x5811 ,0x08},
	{0x5812 ,0x0a},
	{0x5813 ,0x03},
	{0x5814 ,0x00},
	{0x5815 ,0x00},
	{0x5816 ,0x04},
	{0x5817 ,0x09},
	{0x5818 ,0x0f},
	{0x5819 ,0x08},
	{0x581a ,0x06},
	{0x581b ,0x06},
	{0x581c ,0x08},
	{0x581d ,0x0c},
	{0x581e ,0x3f},
	{0x581f ,0x1e},
	{0x5820 ,0x12},
	{0x5821 ,0x13},
	{0x5822 ,0x21},
	{0x5823 ,0x3f},
	{0x5824 ,0x68},
	{0x5825 ,0x28},
	{0x5826 ,0x2c},
	{0x5827 ,0x28},
	{0x5828 ,0x08},
	{0x5829 ,0x48},
	{0x582a ,0x64},
	{0x582b ,0x62},
	{0x582c ,0x64},
	{0x582d ,0x28},
	{0x582e ,0x46},
	{0x582f ,0x62},
	{0x5830 ,0x60},
	{0x5831 ,0x62},
	{0x5832 ,0x26},
	{0x5833 ,0x48},
	{0x5834 ,0x66},
	{0x5835 ,0x44},
	{0x5836 ,0x64},
	{0x5837 ,0x28},
	{0x5838 ,0x66},
	{0x5839 ,0x48},
	{0x583a ,0x2c},
	{0x583b ,0x28},
	{0x583c ,0x26},
	{0x583d ,0xae},
	{0x5025 ,0x00},
	{0x3a0f ,0x38},
	{0x3a10 ,0x30},
	{0x3a11 ,0x70},
	{0x3a1b ,0x38},
	{0x3a1e ,0x30},
	{0x3a1f ,0x18},
	{0x4300 ,0xf8},
	{0x501f ,0x03},
	{0x3008 ,0x02},


	{OV5645_REG_END, 0x00},	/* END MARKER */

};

static struct regval_list ov5645_init_regs_raw10[] = {

	/* @@ MIPI_2lane_720P_RAW10_30fps */
	/* 99 1280 720 */
	/* 98 1 0 */
	/* 64 1000 7 */

       {0x3103, 0x11},
       {0x3008, 0x82},
//       {OV5645_REG_DELAY, 50},
      // {0x4800,0x24},
       {0x4202,0x0f},// stream off, sensor enter LP11

       {0x3008, 0x42},
       {0x3103, 0x03},
       {0x3503, 0x07},
       {0x3002, 0x1c},
       {0x3006, 0xc3},
       {0x300e, 0x45},
       {0x3017, 0x40},
       {0x3018, 0x00},
       {0x302e, 0x0b},
       {0x3037, 0x13},
       {0x3108, 0x01},
       {0x3611, 0x06},
       {0x3612, 0xa9},
       {0x3614, 0x50},
       {0x3618, 0x00},
       {0x3034, 0x1a},
       {0x3035, 0x21},   //;30fps
       {0x3036, 0x69},
       {0x3500, 0x00},
       {0x3501, 0x02},
       {0x3502, 0xe4},
       {0x350a, 0x00},
       {0x350b, 0x3f},
       {0x3600, 0x0a},
       {0x3601, 0x75},
       {0x3620, 0x33},
       {0x3621, 0xe0},
       {0x3622, 0x01},
       {0x3630, 0x2d},
       {0x3631, 0x00},
       {0x3632, 0x32},
       {0x3633, 0x52},
       {0x3634, 0x70},
       {0x3635, 0x13},
       {0x3636, 0x03},
       {0x3702, 0x6e},
       {0x3703, 0x52},
       {0x3704, 0xa0},
       {0x3705, 0x33},
       {0x3708, 0x66},
       {0x3709, 0x12},
       {0x370b, 0x61},
       {0x370c, 0xc3},
       {0x370f, 0x10},
       {0x3715, 0x08},
       {0x3717, 0x01},
       {0x371b, 0x20},
       {0x3731, 0x22},
       {0x3739, 0x70},
       {0x3901, 0x0a},
       {0x3905, 0x02},
       {0x3906, 0x10},
       {0x3719, 0x86},
       {0x3800, 0x00},
       {0x3801, 0x00},
       {0x3802, 0x00},
       {0x3803, 0xfa},
       {0x3804, 0x0a},
       {0x3805, 0x3f},
       {0x3806, 0x06},
       {0x3807, 0xa9},
       {0x3808, 0x05},
       {0x3809, 0x00},
       {0x380a, 0x02},
       {0x380b, 0xd0},
       {0x380c, 0x07},
       {0x380d, 0x64},
       {0x380e, 0x03},
       {0x380f, 0x78},
       {0x3810, 0x00},
       {0x3811, 0x10},
       {0x3812, 0x00},
       {0x3813, 0x04},
       {0x3814, 0x31},
       {0x3815, 0x31},
       {0x3820, 0x41},
       {0x3821, 0x07},
       {0x3824, 0x01},
       {0x3826, 0x03},
       {0x3828, 0x08},
       {0x3a02, 0x02},
       {0x3a03, 0xe4},
       {0x3a08, 0x01},
       {0x3a09, 0xbc},
       {0x3a0a, 0x01},
       {0x3a0b, 0x72},
       {0x3a0e, 0x01},
       {0x3a0d, 0x02},
       {0x3a14, 0x02},
       {0x3a15, 0xe4},
       {0x3a18, 0x00},
       {0x3a19, 0xf8},
       {0x3c01, 0x34},
       {0x3c04, 0x28},
       {0x3c05, 0x98},
       {0x3c07, 0x07},
       {0x3c09, 0xc2},
       {0x3c0a, 0x9c},
       {0x3c0b, 0x40},
       {0x3c01, 0x34},
       {0x4001, 0x02},
       {0x4004, 0x02},
       {0x4005, 0x18},
       {0x4050, 0x6e},
       {0x4051, 0x8f},
       {0x4300, 0x30},
       {0x4514, 0x00},
       {0x4520, 0xb0},
       {0x460b, 0x37},
       {0x460c, 0x20},
       {0x4818, 0x01},
       {0x481d, 0xf0},
       {0x481f, 0x50},
       {0x4823, 0x70},
       {0x4831, 0x14},
       {0x4837, 0x0b},
       {0x5000, 0xa7},
       {0x5001, 0x83},
       {0x501d, 0x00},
       {0x501f, 0x00},
       {0x503d, 0x00},
       {0x505c, 0x30},
       {0x5181, 0x59},
       {0x5183, 0x00},
       {0x5191, 0xf0},
       {0x5192, 0x03},
       {0x5684, 0x10},
       {0x5685, 0xa0},
       {0x5686, 0x0c},
       {0x5687, 0x78},
       {0x5a00, 0x08},
       {0x5a21, 0x00},
       {0x5a24, 0x00},
       {0x3008, 0x02},
       {0x3503, 0x07},//manual exposoure
       {0x5180, 0xff},
       {0x5181, 0xf2},
       {0x5182, 0x00},
       {0x5183, 0x14},
       {0x5184, 0x25},
       {0x5185, 0x24},
       {0x5186, 0x09},
       {0x5187, 0x09},
       {0x5188, 0x0a},
       {0x5189, 0x75},
       {0x518a, 0x52},
       {0x518b, 0xea},
       {0x518c, 0xa8},
       {0x518d, 0x42},
       {0x518e, 0x38},
       {0x518f, 0x56},
       {0x5190, 0x42},
       {0x5191, 0xf8},
       {0x5192, 0x04},
       {0x5193, 0x70},
       {0x5194, 0xf0},
       {0x5195, 0xf0},
       {0x5196, 0x03},
       {0x5197, 0x01},
       {0x5198, 0x04},
       {0x5199, 0x12},
       {0x519a, 0x04},
       {0x519b, 0x00},
       {0x519c, 0x06},
       {0x519d, 0x82},
       {0x519e, 0x38},
       {0x5381, 0x1e},
       {0x5382, 0x5b},
       {0x5383, 0x08},
       {0x5384, 0x0b},
       {0x5385, 0x84},
       {0x5386, 0x8f},
       {0x5387, 0x82},
       {0x5388, 0x71},
       {0x5389, 0x11},
       {0x538a, 0x01},
       {0x538b, 0x98},
       {0x5300, 0x08},
       {0x5301, 0x1e},
       {0x5302, 0x10},
       {0x5303, 0x00},
       {0x5304, 0x08},
       {0x5305, 0x1e},
       {0x5306, 0x08},
       {0x5307, 0x16},
       {0x5309, 0x08},
       {0x530a, 0x1e},
       {0x530b, 0x04},
       {0x530c, 0x06},
       {0x5480, 0x01},
       {0x5481, 0x0e},
       {0x5482, 0x18},
       {0x5483, 0x2b},
       {0x5484, 0x52},
       {0x5485, 0x65},
       {0x5486, 0x71},
       {0x5487, 0x7d},
       {0x5488, 0x87},
       {0x5489, 0x91},
       {0x548a, 0x9a},
       {0x548b, 0xaa},
       {0x548c, 0xb8},
       {0x548d, 0xcd},
       {0x548e, 0xdd},
       {0x548f, 0xea},
       {0x5490, 0x1d},
       {0x5580, 0x02},
       {0x5583, 0x40},
       {0x5584, 0x30},
       {0x5589, 0x10},
       {0x558a, 0x00},
       {0x558b, 0xf8},
       {0x5780, 0xfc},
       {0x5781, 0x13},
       {0x5782, 0x03},
       {0x5786, 0x20},
       {0x5787, 0x40},
       {0x5788, 0x08},
       {0x5789, 0x08},
       {0x578a, 0x02},
       {0x578b, 0x01},
       {0x578c, 0x01},
       {0x578d, 0x0c},
       {0x578e, 0x02},
       {0x578f, 0x01},
       {0x5790, 0x01},
       {0x5800, 0x3f},
       {0x5801, 0x16},
       {0x5802, 0x0e},
       {0x5803, 0x0d},
       {0x5804, 0x17},
       {0x5805, 0x3f},
       {0x5806, 0x0b},
       {0x5807, 0x06},
       {0x5808, 0x04},
       {0x5809, 0x04},
       {0x580a, 0x06},
       {0x580b, 0x0b},
       {0x580c, 0x09},
       {0x580d, 0x03},
       {0x580e, 0x00},
       {0x580f, 0x00},
       {0x5810, 0x03},
       {0x5811, 0x08},
       {0x5812, 0x0a},
       {0x5813, 0x03},
       {0x5814, 0x00},
       {0x5815, 0x00},
       {0x5816, 0x04},
       {0x5817, 0x09},
       {0x5818, 0x0f},
       {0x5819, 0x08},
       {0x581a, 0x06},
       {0x581b, 0x06},
       {0x581c, 0x08},
       {0x581d, 0x0c},
       {0x581e, 0x3f},
       {0x581f, 0x1e},
       {0x5820, 0x12},
       {0x5821, 0x13},
       {0x5822, 0x21},
       {0x5823, 0x3f},
       {0x5824, 0x68},
       {0x5825, 0x28},
       {0x5826, 0x2c},
       {0x5827, 0x28},
       {0x5828, 0x08},
       {0x5829, 0x48},
       {0x582a, 0x64},
       {0x582b, 0x62},
       {0x582c, 0x64},
       {0x582d, 0x28},
       {0x582e, 0x46},
       {0x582f, 0x62},
       {0x5830, 0x60},
       {0x5831, 0x62},
       {0x5832, 0x26},
       {0x5833, 0x48},
       {0x5834, 0x66},
       {0x5835, 0x44},
       {0x5836, 0x64},
       {0x5837, 0x28},
       {0x5838, 0x66},
       {0x5839, 0x48},
       {0x583a, 0x2c},
       {0x583b, 0x28},
       {0x583c, 0x26},
       {0x583d, 0xae},
       {0x5025, 0x00},
       {0x3a0f, 0x38},
       {0x3a10, 0x30},
       {0x3a11, 0x70},
       {0x3a1b, 0x38},
       {0x3a1e, 0x30},
       {0x3a1f, 0x18},
       {0x3008, 0x02},

       {0x4300, 0xf8},
       {0x501f, 0x03},
       {0x5000, 0x06},
       {0x5001, 0x00},
    //   {0x503d, 0x80},
       {OV5645_REG_END, 0x00},	/* END MARKER */
};


static struct regval_list ov5645_stream_on[] = {
	{0x4202, 0x00},

	{OV5645_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list ov5645_stream_off[] = {
	/* Sensor enter LP11*/
	{0x4202, 0x0f},

	{OV5645_REG_END, 0x00},	/* END MARKER */
};
static struct regval_list ov5645_win_720p[] = {
	{OV5645_REG_END, 0x00},	/* END MARKER */
};
static struct regval_list ov5645_win_sxga[] = {
	{OV5645_REG_END, 0x00},	/* END MARKER */
};
static struct regval_list ov5645_win_vga[] = {
	{OV5645_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list ov5645_win_5m[] = {
	{OV5645_REG_END, 0x00},	/* END MARKER */
};

int ov5645_read(struct v4l2_subdev *sd, unsigned short reg,
		unsigned char *value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
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

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;

	return ret;
}


static int ov5645_write(struct v4l2_subdev *sd, unsigned short reg,
		unsigned char value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[3] = {reg >> 8, reg & 0xff, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 3,
		.buf	= buf,
	};
	int ret;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}


static int ov5645_read_array(struct v4l2_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != OV5645_REG_END) {
		if (vals->reg_num == OV5645_REG_DELAY) {
			if (vals->value >= (1000 / HZ))
				msleep(vals->value);
			else
				mdelay(vals->value);
		} else {
			ret = ov5645_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		printk("vals->reg_num:%x, vals->value:%x\n",vals->reg_num, val);
		//mdelay(200);
		vals++;
	}
	//printk("vals->reg_num:%x, vals->value:%x\n", vals->reg_num, vals->value);
	ov5645_write(sd, vals->reg_num, vals->value);
	return 0;
}
static int ov5645_write_array(struct v4l2_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != OV5645_REG_END) {
		if (vals->reg_num == OV5645_REG_DELAY) {
			if (vals->value >= (1000 / HZ))
				msleep(vals->value);
			else
				mdelay(vals->value);
		} else {
			ret = ov5645_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		//printk("vals->reg_num:%x, vals->value:%x\n",vals->reg_num, vals->value);
		//mdelay(200);
		vals++;
	}
	//printk("vals->reg_num:%x, vals->value:%x\n", vals->reg_num, vals->value);
	//ov5645_write(sd, vals->reg_num, vals->value);
	return 0;
}

/* R/G and B/G of typical camera module is defined here. */
static unsigned int rg_ratio_typical = 0x58;
static unsigned int bg_ratio_typical = 0x5a;

/*
	R_gain, sensor red gain of AWB, 0x400 =1
	G_gain, sensor green gain of AWB, 0x400 =1
	B_gain, sensor blue gain of AWB, 0x400 =1
	return 0;
*/
static int ov5645_update_awb_gain(struct v4l2_subdev *sd,
				unsigned int R_gain, unsigned int G_gain, unsigned int B_gain)
{
#if 0
	printk("[ov5645] problem function:%s, line:%d\n", __func__, __LINE__);
	printk("R_gain:%04x, G_gain:%04x, B_gain:%04x \n ", R_gain, G_gain, B_gain);
	if (R_gain > 0x400) {
		ov5645_write(sd, 0x5186, (unsigned char)(R_gain >> 8));
		ov5645_write(sd, 0x5187, (unsigned char)(R_gain & 0x00ff));
	}

	if (G_gain > 0x400) {
		ov5645_write(sd, 0x5188, (unsigned char)(G_gain >> 8));
		ov5645_write(sd, 0x5189, (unsigned char)(G_gain & 0x00ff));
	}

	if (B_gain > 0x400) {
		ov5645_write(sd, 0x518a, (unsigned char)(B_gain >> 8));
		ov5645_write(sd, 0x518b, (unsigned char)(B_gain & 0x00ff));
	}
#endif

	return 0;
}

static int ov5645_reset(struct v4l2_subdev *sd, u32 val)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return 0;
}

static int ov5645_detect(struct v4l2_subdev *sd);
static int ov5645_init(struct v4l2_subdev *sd, u32 val)
{
	struct ov5645_info *info = container_of(sd, struct ov5645_info, sd);
	int ret = 0;
	printk("functiong:%s, line:%d\n", __func__, __LINE__);

	info->win = NULL;

	//ret = ov5645_write_array(sd, ov5645_init_regs_raw10);
	//ret = ov5645_write_array(sd, ov5645_init_regs_672Mbps_1080p);
	//ret = ov5645_write_array(sd, ov5645_init_regs_672Mbps_5M);
	if (ret < 0)
		return ret;

	return 0;
}
static int ov5645_get_sensor_vts(struct v4l2_subdev *sd, unsigned short *value)
{
	unsigned char h,l;
	int ret = 0;
	ret = ov5645_read(sd, 0x380e, &h);
	if (ret < 0)
		return ret;
	ret = ov5645_read(sd, 0x380f, &l);
	if (ret < 0)
		return ret;
	*value = h;
	*value = (*value << 8) | l;
	return ret;
}

static int ov5645_get_sensor_lans(struct v4l2_subdev *sd, unsigned char *value)
{
	int ret = 0;
	unsigned char v = 0;
	ret = ov5645_read(sd, 0x300e, &v);
	if (ret < 0)
		return ret;
	*value = v >> 5;
	if(*value > 2 || *value < 1)
		ret = -EINVAL;
	return ret;
}
static int ov5645_detect(struct v4l2_subdev *sd)
{
	unsigned char v;
	int ret;
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
#if 1
	/*test gpio e*/
	{
		printk("0x10010410:%x\n", *((volatile unsigned int *)0xb0010410));
		printk("0x10010420:%x\n", *((volatile unsigned int *)0xb0010420));
		printk("0x10010430:%x\n", *((volatile unsigned int *)0xb0010430));
		printk("0x10010440:%x\n", *((volatile unsigned int *)0xb0010440));
		printk("0x10010010:%x\n", *((volatile unsigned int *)0xb0010010));
		printk("0x10010020:%x\n", *((volatile unsigned int *)0xb0010020));
		printk("0x10010030:%x\n", *((volatile unsigned int *)0xb0010030));
		printk("0x10010040:%x\n", *((volatile unsigned int *)0xb0010040));
	}
#endif

	ret = ov5645_read(sd, 0x300a, &v);
	printk("-----%s: %d ret = %d\n", __func__, __LINE__, ret);
	if (ret < 0)
		return ret;
	printk("-----%s: %d v = %08X\n", __func__, __LINE__, v);
	if (v != OV5645_CHIP_ID_H)
		return -ENODEV;
	ret = ov5645_read(sd, 0x300b, &v);
	printk("-----%s: %d ret = %d\n", __func__, __LINE__, ret);
	if (ret < 0)
		return ret;
	printk("-----%s: %d v = %08X\n", __func__, __LINE__, v);
	if (v != OV5645_CHIP_ID_L)
		return -ENODEV;
	return 0;
}

static struct ov5645_format_struct {
	enum v4l2_mbus_pixelcode mbus_code;
	enum v4l2_colorspace colorspace;
} ov5645_formats[] = {
	{
		/*RAW8 FORMAT, 8 bit per pixel*/
		.mbus_code	= V4L2_MBUS_FMT_SBGGR8_1X8,
		.colorspace	= V4L2_COLORSPACE_SRGB,
	},
	{
		/*RAW10 FORMAT, 10 bit per pixel*/
		.mbus_code	= V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace	= V4L2_COLORSPACE_SRGB,
	},
	{
		.mbus_code = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace	 = V4L2_COLORSPACE_BT878,/*don't know*/

	}
	/*add other format supported*/
};
#define N_OV5645_FMTS ARRAY_SIZE(ov5645_formats)

static struct ov5645_win_setting {
	int	width;
	int	height;
	enum v4l2_mbus_pixelcode mbus_code;
	enum v4l2_colorspace colorspace;
	struct regval_list *regs; /* Regs to tweak */
} ov5645_win_sizes[] = {
	/* 2592*1944 */
	{
		.width		= MAX_WIDTH,
		.height		= MAX_HEIGHT,
		.mbus_code	= V4L2_MBUS_FMT_SBGGR8_1X8,
		.colorspace	= V4L2_COLORSPACE_SRGB,
		.regs 		= ov5645_init_regs_672Mbps_5M,
	},
	/* 1920*1080 */
	{
		.width		= 1920,
		.height		= 1080,
		.mbus_code	= V4L2_MBUS_FMT_SBGGR8_1X8,
		.colorspace	= V4L2_COLORSPACE_SRGB,
		.regs 		= ov5645_init_regs_672Mbps_1080p,
	},
#if 0
	/* SXGA */
	{
		.width		= SXGA_WIDTH,
		.height		= SXGA_HEIGHT,
		.regs 		= NULL,
	},
#endif
	/* 1280*720 */
	{
		.width		= 1280,
		.height		= 720,
		.mbus_code	= V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace	= V4L2_COLORSPACE_SRGB,
		.regs 		= ov5645_init_regs_raw10,
	}
};
#define N_WIN_SIZES (ARRAY_SIZE(ov5645_win_sizes))

static int ov5645_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned index,
					enum v4l2_mbus_pixelcode *code)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	if (index >= N_OV5645_FMTS)
		return -EINVAL;

	*code = ov5645_formats[index].mbus_code;
	return 0;
}

static int ov5645_try_fmt_internal(struct v4l2_subdev *sd,
		struct v4l2_mbus_framefmt *fmt,
		struct ov5645_win_setting **ret_wsize)
{
	struct ov5645_win_setting *wsize;

	if(fmt->width > MAX_WIDTH || fmt->height > MAX_HEIGHT)
		return -EINVAL;
	for (wsize = ov5645_win_sizes; wsize < ov5645_win_sizes + N_WIN_SIZES;
	     wsize++)
		if (fmt->width >= wsize->width && fmt->height >= wsize->height)
			break;
	if (wsize >= ov5645_win_sizes + N_WIN_SIZES)
		wsize--;   /* Take the smallest one */
	if (ret_wsize != NULL)
		*ret_wsize = wsize;
	fmt->width = wsize->width;
	fmt->height = wsize->height;
	fmt->code = wsize->mbus_code;
	fmt->field = V4L2_FIELD_NONE;
	fmt->colorspace = wsize->colorspace;
	printk("%s:------->%d fmt->code,%08X , fmt->width%d fmt->height%d\n", __func__, __LINE__, fmt->code, fmt->width, fmt->height);
	return 0;
}

static int ov5645_try_mbus_fmt(struct v4l2_subdev *sd,
			    struct v4l2_mbus_framefmt *fmt)
{
	printk("%s:------->%d\n", __func__, __LINE__);
	return ov5645_try_fmt_internal(sd, fmt, NULL);
}

static int ov5645_s_mbus_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *fmt)
{

	struct ov5645_info *info = container_of(sd, struct ov5645_info, sd);
	struct v4l2_fmt_data *data = v4l2_get_fmt_data(fmt);
	struct ov5645_win_setting *wsize;
	int ret;


	printk("[ov5645], problem function:%s, line:%d\n", __func__, __LINE__);
	ret = ov5645_try_fmt_internal(sd, fmt, &wsize);
	if (ret)
		return ret;
	if ((info->win != wsize) && wsize->regs) {
		printk("pay attention : ov5645, %s:LINE:%d  size = %d\n", __func__, __LINE__, sizeof(*wsize->regs));
		ret = ov5645_write_array(sd, wsize->regs);
		if (ret)
			return ret;
	}
	data->i2cflags = V4L2_I2C_ADDR_16BIT;
	data->mipi_clk = 282;
	ret = ov5645_get_sensor_vts(sd, &(data->vts));
	if(ret < 0){
		printk("[ov5645], problem function:%s, line:%d\n", __func__, __LINE__);
		return ret;
	}
	ret = ov5645_get_sensor_lans(sd, &(data->lans));
	if(ret < 0){
		printk("[ov5645], problem function:%s, line:%d\n", __func__, __LINE__);
		return ret;
	}
	info->win = wsize;

	return 0;
}

static int ov5645_s_stream(struct v4l2_subdev *sd, int enable)
{
	int ret = 0;
	printk("--------%s: %d enable:%d\n", __func__, __LINE__, enable);
	if (enable) {
		ret = ov5645_write_array(sd, ov5645_stream_on);
		printk("ov5645 stream on\n");
	}
	else {
		ret = ov5645_write_array(sd, ov5645_stream_off);
		printk("ov5645 stream off\n");
	}
	return ret;
}

static int ov5645_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	a->c.left	= 0;
	a->c.top	= 0;
	a->c.width	= MAX_WIDTH;
	a->c.height	= MAX_HEIGHT;
	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return 0;
}

static int ov5645_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	a->bounds.left			= 0;
	a->bounds.top			= 0;
	a->bounds.width			= MAX_WIDTH;
	a->bounds.height		= MAX_HEIGHT;
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	return 0;
}

static int ov5645_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return 0;
}

static int ov5645_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return 0;
}

static int ov5645_frame_rates[] = { 30, 15, 10, 5, 1 };

static int ov5645_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *interval)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	if (interval->index >= ARRAY_SIZE(ov5645_frame_rates))
		return -EINVAL;
	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	interval->discrete.numerator = 1;
	interval->discrete.denominator = ov5645_frame_rates[interval->index];
	return 0;
}

static int ov5645_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	int i;
	int num_valid = -1;
	__u32 index = fsize->index;

	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	/*
	 * If a minimum width/height was requested, filter out the capture
	 * windows that fall outside that.
	 */
	for (i = 0; i < N_WIN_SIZES; i++) {
		struct ov5645_win_setting *win = &ov5645_win_sizes[index];
		if (index == ++num_valid) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = win->width;
			fsize->discrete.height = win->height;
			return 0;
		}
	}

	return -EINVAL;
}

static int ov5645_queryctrl(struct v4l2_subdev *sd,
		struct v4l2_queryctrl *qc)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return 0;
}

static int ov5645_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return 0;
}

static int ov5645_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return 0;
}

static int ov5645_g_chip_ident(struct v4l2_subdev *sd,
		struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	printk("functiong:%s, line:%d\n", __func__, __LINE__);

//	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_OV5645, 0);
	return v4l2_chip_ident_i2c_client(client, chip, 123, 0);
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov5645_g_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char val = 0;
	int ret;

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = ov5645_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;
	return ret;
}

static int ov5645_s_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ov5645_write(sd, reg->reg & 0xffff, reg->val & 0xff);
	return 0;
}
#endif

static int ov5645_s_power(struct v4l2_subdev *sd, int on)
{
		printk("--%s:%d\n", __func__, __LINE__);
	return 0;
}

static const struct v4l2_subdev_core_ops ov5645_core_ops = {
	.g_chip_ident = ov5645_g_chip_ident,
	.g_ctrl = ov5645_g_ctrl,
	.s_ctrl = ov5645_s_ctrl,
	.queryctrl = ov5645_queryctrl,
	.reset = ov5645_reset,
	.init = ov5645_init,
	.s_power = ov5645_s_power,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov5645_g_register,
	.s_register = ov5645_s_register,
#endif
};

static const struct v4l2_subdev_video_ops ov5645_video_ops = {
	.enum_mbus_fmt = ov5645_enum_mbus_fmt,
	.try_mbus_fmt = ov5645_try_mbus_fmt,
	.s_mbus_fmt = ov5645_s_mbus_fmt,
	.s_stream = ov5645_s_stream,
	.cropcap = ov5645_cropcap,
	.g_crop	= ov5645_g_crop,
	.s_parm = ov5645_s_parm,
	.g_parm = ov5645_g_parm,
	.enum_frameintervals = ov5645_enum_frameintervals,
	.enum_framesizes = ov5645_enum_framesizes,
};

static const struct v4l2_subdev_ops ov5645_ops = {
	.core = &ov5645_core_ops,
	.video = &ov5645_video_ops,
};

static ssize_t ov5645_rg_ratio_typical_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return sprintf(buf, "%d", rg_ratio_typical);
}

static ssize_t ov5645_rg_ratio_typical_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	char *endp;
	int value;
	printk("functiong:%s, line:%d\n", __func__, __LINE__);

	value = simple_strtoul(buf, &endp, 0);
	if (buf == endp)
		return -EINVAL;

	rg_ratio_typical = (unsigned int)value;

	return size;
}

static ssize_t ov5645_bg_ratio_typical_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	return sprintf(buf, "%d", bg_ratio_typical);
}

static ssize_t ov5645_bg_ratio_typical_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	char *endp;
	int value;

	printk("functiong:%s, line:%d\n", __func__, __LINE__);
	value = simple_strtoul(buf, &endp, 0);
	if (buf == endp)
		return -EINVAL;

	bg_ratio_typical = (unsigned int)value;

	return size;
}

static DEVICE_ATTR(ov5645_rg_ratio_typical, 0664, ov5645_rg_ratio_typical_show, ov5645_rg_ratio_typical_store);
static DEVICE_ATTR(ov5645_bg_ratio_typical, 0664, ov5645_bg_ratio_typical_show, ov5645_bg_ratio_typical_store);

static int ov5645_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct ov5645_info *info;
	int ret;

	info = kzalloc(sizeof(struct ov5645_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;
	v4l2_i2c_subdev_init(sd, client, &ov5645_ops);

	/* Make sure it's an ov5645 */
//aaa:
	ret = ov5645_detect(sd);
	if (ret) {
		v4l_err(client,
			"chip found @ 0x%x (%s) is not an ov5645 chip.\n",
			client->addr, client->adapter->name);
//		goto aaa;
		kfree(info);
		return ret;
	}
	v4l_info(client, "ov5645 chip found @ 0x%02x (%s)\n",
			client->addr, client->adapter->name);

	ret = device_create_file(&client->dev, &dev_attr_ov5645_rg_ratio_typical);
	if(ret){
		v4l_err(client, "create dev_attr_ov5645_rg_ratio_typical failed!\n");
		goto err_create_dev_attr_ov5645_rg_ratio_typical;
	}

	ret = device_create_file(&client->dev, &dev_attr_ov5645_bg_ratio_typical);
	if(ret){
		v4l_err(client, "create dev_attr_ov5645_bg_ratio_typical failed!\n");
		goto err_create_dev_attr_ov5645_bg_ratio_typical;
	}

	printk("probe ok ------->ov5645\n");
	return 0;

err_create_dev_attr_ov5645_bg_ratio_typical:
	device_remove_file(&client->dev, &dev_attr_ov5645_rg_ratio_typical);
err_create_dev_attr_ov5645_rg_ratio_typical:
	kfree(info);
	return ret;
}

static int ov5645_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ov5645_info *info = container_of(sd, struct ov5645_info, sd);

	device_remove_file(&client->dev, &dev_attr_ov5645_rg_ratio_typical);
	device_remove_file(&client->dev, &dev_attr_ov5645_bg_ratio_typical);

	v4l2_device_unregister_subdev(sd);
	kfree(info);
	return 0;
}

static const struct i2c_device_id ov5645_id[] = {
	{ "ov5645", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ov5645_id);

static struct i2c_driver ov5645_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ov5645",
	},
	.probe		= ov5645_probe,
	.remove		= ov5645_remove,
	.id_table	= ov5645_id,
};

static __init int init_ov5645(void)
{
	printk("init_ov5645 #########\n");
	return i2c_add_driver(&ov5645_driver);
}

static __exit void exit_ov5645(void)
{
	i2c_del_driver(&ov5645_driver);
}

module_init(init_ov5645);
module_exit(exit_ov5645);

MODULE_DESCRIPTION("A low-level driver for OmniVision ov5645 sensors");
MODULE_LICENSE("GPL");
