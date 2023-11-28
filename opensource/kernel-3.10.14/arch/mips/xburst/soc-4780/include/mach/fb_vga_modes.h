/*
 * mach/fb_vga_modes.h
 *
 * Copyright (c) 2013 Engenic Semiconductor Co., Ltd.
 *              http://www.engenic.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __FB_VGA_MODES_H__
#define __FB_VGA_MODES_H__

#define ADD_VGA_VIDEO_MODE(mode) mode

/*
 * flag: contains about several types,
 * We can determine the value of flag
 * to determine the choice of resolution
 */
#define FB_MODE_IS_JZ4780_VGA_1600X1200 ( 1 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1680X1050 ( 2 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1400X1050 ( 3 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1440X900  ( 4 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1366X768  ( 5 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1280X1024 ( 6 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1280X960  ( 7 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1280X800  ( 8 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1152X864  ( 9 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1024X768  (10 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_800X600   (11 | FB_MODE_IS_VGA)
#define FB_MODE_IS_JZ4780_VGA_1920X1080 (12 | FB_MODE_IS_VGA)

/*
 * struct fb_videomode - Defined in kernel/include/linux/fb.h
 * name
 * refresh, xres, yres, pixclock, left_margin, right_margin
 * upper_margin, lower_margin, hsync_len, vsync_len
 * sync
 * vmode, flag
 */

/*
 * Note:
  "flag" is used as vga mode search index.
 */

#define VGA_1440X900_AT_60HZ                      \
{"1440x900@60Hz",       \
	60, 1440, 900, KHZ2PICOS(106507), 232, 80,              \
		25, 3, 152, 6,          \
		0/*FB_SYNC_HOR_HIGH_ACT*/ | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1440X900}

#define VGA_1440X900_AT_65HZ                      \
{"1440x900@60Hz",       \
	60, 1440, 900, KHZ2PICOS(106470), 232, 80,              \
		28, 1, 152, 3,          \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1440X900}

#define VGA_1280X1024_AT_60HZ                      \
{"1280x1024@60Hz",       \
	60, 1280, 1024, KHZ2PICOS(108003), 248, 48,             \
		38, 1, 112, 3,          \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1280X1024}

#define VGA_1280X1024_AT_75HZ                      \
{"1280x1024@75Hz",       \
	75, 1280, 1024, KHZ2PICOS(135007), 248, 16,             \
		38, 1, 144, 3,          \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1280X1024}

#define VGA_1280X1024_AT_85HZ                      \
{"1280x1024@85Hz",       \
	85, 1280, 1024, KHZ2PICOS(157505), 224, 64,             \
		44, 1, 160, 3,          \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1280X1024}

#define VGA_1280X960_AT_60HZ                      \
{"1280x960@60Hz",       \
	60, 1280, 960, KHZ2PICOS(108003), 312, 96,              \
		36, 1, 112, 3,          \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1280X960}

#define VGA_1280X960_AT_85HZ                      \
{"1280x960@85Hz",       \
	85, 1280, 960, KHZ2PICOS(148500), 224, 64,              \
		47, 1, 160, 3,          \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1280X960}

#define VGA_1024X768_AT_60HZ                      \
{"1024x768@60Hz",       \
	60, 1024, 768, KHZ2PICOS(65002), 160, 24,               \
		29, 3, 136, 6,          \
		0 | 0,          \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1024X768}

#define VGA_1024X768_AT_70HZ                      \
{"1024x768@70Hz",       \
	70, 1024, 768, KHZ2PICOS(75001), 144, 24,               \
		29, 3, 136, 6,          \
		0 | 0,          \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1024X768}

#define VGA_1024X768_AT_75HZ                      \
{"1024x768@75Hz",       \
	75, 1024, 768, KHZ2PICOS(78752), 176, 16,               \
		28, 1, 96, 3,           \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1024X768}

#define VGA_1024X768_AT_85HZ                      \
{"1024x768@85Hz",       \
	85, 1024, 768, KHZ2PICOS(94500), 208, 48,               \
		36, 1, 96, 3,           \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1024X768}

#define VGA_800X600_AT_55HZ                      \
{"800x600@55Hz",       \
	55, 800, 600, KHZ2PICOS(36001), 128, 24,                \
		22, 1, 72, 2,           \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_800X600}

#define VGA_800X600_AT_60HZ                      \
{"800x600@60Hz",       \
	60, 800, 600, KHZ2PICOS(40000), 88, 40,         \
		23, 1, 128, 4,          \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_800X600}

#define VGA_800X600_AT_72HZ                      \
{"800x600@72Hz",       \
	72, 800, 600, KHZ2PICOS(50000), 64, 56,         \
		23, 37, 120, 6,         \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_800X600}

#define VGA_800X600_AT_75HZ                      \
{"800x600@75Hz",       \
	75, 800, 600, KHZ2PICOS(49500), 160, 16,                \
		21, 1, 80, 3,           \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_800X600}

#define VGA_800X600_AT_85HZ                      \
{"800x600@85Hz",       \
	85, 800, 600, KHZ2PICOS(56303), 152, 32,                \
		27, 1, 64, 3,           \
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_800X600}

#define  VGA_1920X1080_AT_60HZ			\
{ "1920x1080@60",		\
	60, 1920, 1080, 7220, 80, 48,		\
		23, 3, 32, 5,\
		FB_SYNC_HOR_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1920X1080}

#define  VGA_1366X768_AT_60HZ			\
{ "1366x768@60",	\
	60, 1366, 768,  KHZ2PICOS(85500), 213, 70,			\
		24, 3, 143, 3,		\
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,           \
		FB_VMODE_NONINTERLACED, FB_MODE_IS_JZ4780_VGA_1366X768}

#define DEFAULT_VGA_VIDEO_MODE_LIST					\
	ADD_VGA_VIDEO_MODE(VGA_1024X768_AT_70HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1920X1080_AT_60HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1366X768_AT_60HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1440X900_AT_60HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1440X900_AT_65HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1280X1024_AT_60HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1280X1024_AT_75HZ),                     \
	ADD_VGA_VIDEO_MODE(VGA_1280X1024_AT_85HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1280X960_AT_60HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1280X960_AT_85HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1024X768_AT_60HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1024X768_AT_75HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_1024X768_AT_85HZ),                      \
	ADD_VGA_VIDEO_MODE(VGA_800X600_AT_60HZ)
#endif
