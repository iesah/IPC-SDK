/*
 * [board]-cim.c - This file defines camera host driver (cim) on the board.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Author: YeFei <feiye@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <media/soc_camera.h>
#include <mach/platform.h>
#include <mach/jz4780_camera.h>
#include <linux/regulator/machine.h>
#include <gpio.h>
#include "urboard.h"

extern struct i2c_board_info urboard_i2c2_devs_v4l2[];

#ifdef CONFIG_VIDEO_JZ4780_CIM_HOST
struct jz4780_camera_pdata urboard_camera_pdata = {
	.mclk_10khz = 2400,
	.flags = 0,
#ifdef CONFIG_FRONT_CAMERA
	.cam_sensor_pdata[FRONT_CAMERA_INDEX] = {
		.gpio_rst = CAMERA_SENSOR_RESET,
		.gpio_power = CAMERA_FRONT_SENSOR_EN,
	},
#endif
};

static int camera_sensor_reset(struct device *dev) {
	gpio_set_value(CAMERA_SENSOR_RESET, 0);
	msleep(150);
	gpio_set_value(CAMERA_SENSOR_RESET, 1);
	msleep(150);

	return 0;
}

#ifdef CONFIG_SOC_CAMERA_OV5640_FRONT

static int front_camera_sensor_power(struct device *dev, int on) {
	/* enable or disable the camera */
	gpio_set_value(CAMERA_FRONT_SENSOR_EN, on ? 0 : 1);
	msleep(150);

	return 0;
}

static struct soc_camera_link iclink_front = {
	.bus_id		= 0,		/* Must match with the camera ID */
	.board_info	= &urboard_i2c2_devs_v4l2[FRONT_CAMERA_INDEX],
	.i2c_adapter_id	= 2,
	.power = front_camera_sensor_power,
	.reset = camera_sensor_reset,
};

struct platform_device urboard_front_camera_sensor = {
	.name	= "soc-camera-pdrv",
	.id	= -1,
	.dev	= {
		.platform_data = &iclink_front,
	},
};
#endif
#endif

static int __init urboard_board_cim_init(void) {
	/* camera host */
#ifdef CONFIG_VIDEO_JZ4780_CIM_HOST
	jz_device_register(&jz_cim_device, &urboard_camera_pdata);
#endif
	/* camera sensor */
#ifdef CONFIG_FRONT_CAMERA
	platform_device_register(&urboard_front_camera_sensor);
#endif

	return 0;
}

arch_initcall(urboard_board_cim_init);
