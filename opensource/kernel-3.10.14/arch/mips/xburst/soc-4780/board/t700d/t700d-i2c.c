/*
 * [board]-i2c.c - This file defines i2c devices on the board.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/input.h>
#include <linux/gsensor.h>
#include <linux/lsensor.h>
#include <linux/csensor.h>
#include <linux/tsc.h>

#include <mach/platform.h>
#include <mach/jzmmc.h>
#include <gpio.h>

#include "t700d.h"

#if ((defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C1_JZ4780)) && defined(CONFIG_SENSORS_MMA8452))
static struct gsensor_platform_data mma8452_platform_pdata = {
	.gpio_int = GPIO_MMA8452_INT1,
	.poll_interval = 100,
	.min_interval = 40,
	.max_interval = 200,
	.g_range = GSENSOR_2G,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,
	.negate_x = 0,
	.negate_y = 0,
	.negate_z = 0,

	.ori_pr_swap = 0,
	.ori_pith_negate = 0,
	.ori_roll_negate = 1,
};
#endif

#ifdef CONFIG_SENSORS_MC32X0
static struct gsensor_platform_data mc32x0_platform_data = {
	.gpio_int = GPIO_MC32X0_INT1,
	.poll_interval = 100,
       	.min_interval = 1,
	.max_interval = 200,
	.g_range = GSENSOR_2G,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,
	.negate_x = 1,
	.negate_y = 0,
	.negate_z = 1,

	.ori_pr_swap = 0,
	.ori_pith_negate = 0,
	.ori_roll_negate = 1,
};
#endif

#ifdef CONFIG_SENSORS_STK220X
static struct lsensor_platform_data stk220x_platform_data = {
	.gpio_int = GPIO_PE(5),
};
#endif

#if ((defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C3_JZ4780)) && defined(CONFIG_JZ4780_SUPPORT_TSC))
static struct jztsc_pin t700d_tsc_gpio[] = {
	[0] = {GPIO_CTP_IRQ,		HIGH_ENABLE},
	[1] = {GPIO_CTP_WAKE_UP,	HIGH_ENABLE},
};

static struct jztsc_platform_data t700d_tsc_pdata = {
	.gpio		= t700d_tsc_gpio,
	.x_max		= 960,
	.y_max		= 640,
};
#endif

#if (defined(CONFIG_I2C1_JZ4780) || defined(CONFIG_I2C_GPIO))
static struct i2c_board_info t700d_i2c1_devs[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_MG8698S
	{
		I2C_BOARD_INFO("mg8698s_tsc", 0x44),
		.platform_data	= &t700d_tsc_pdata,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_ZET6221
	{
		I2C_BOARD_INFO("zet6221_tsc", 0x76), 
		.platform_data	= &t700d_tsc_pdata,
	},
#endif
};
#endif	/*I2C1*/

#if (defined(CONFIG_HI704))
static struct cam_sensor_plat_data hi253_pdata = {
	.facing = 0,
	.orientation = 0,
	.mirror = 0,
	.gpio_en = GPIO_HI253_EN,
	.gpio_rst = GPIO_HI253_RST,
	.cap_wait_frame = 2,
};
#endif

#if (defined(CONFIG_HI704))
static struct cam_sensor_plat_data hi704_pdata = {
	.facing = 1,
	.orientation = 0,
	.mirror = 0,
	.gpio_en = GPIO_HI704_EN,
	.gpio_rst = GPIO_HI704_RST,
	.cap_wait_frame = 2,
};
#endif

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C2_JZ4780))
static struct i2c_board_info t700d_i2c2_devs[] __initdata = {
#ifdef CONFIG_HI253
	{
		I2C_BOARD_INFO("hi253", 0x20),
		.platform_data	= &hi253_pdata,
	},
#endif
#ifdef CONFIG_HI704
	{
		I2C_BOARD_INFO("hi704", 0x30),
		.platform_data	= &hi704_pdata,
	},
#endif
};
#endif	/*I2C2*/

#if (defined(CONFIG_I2C3_JZ4780) || defined(CONFIG_I2C_GPIO))
static struct i2c_board_info t700d_i2c3_devs[] __initdata = {
#ifdef CONFIG_SENSORS_MMA8452
	{
		I2C_BOARD_INFO("gsensor_mma8452",0x1c),
		.platform_data = &mma8452_platform_pdata,
	},
#endif
#ifdef CONFIG_SENSORS_MC32X0
	{
	       	I2C_BOARD_INFO("gsensor_mc32x0",0x4c),
		.platform_data = &mc32x0_platform_data,
	},
#endif
#ifdef CONFIG_SENSORS_STK220X
	{
	       	I2C_BOARD_INFO("stk_als",  0x20>>1),
		.platform_data = &stk220x_platform_data,
	},
#endif
};
#endif /*I2C3*/

/*define gpio i2c,if you use gpio i2c,please enable gpio i2c and disable i2c controller*/
#ifdef CONFIG_I2C_GPIO /*CONFIG_I2C_GPIO*/

#define DEF_GPIO_I2C(NO,GPIO_I2C_SDA,GPIO_I2C_SCK)		\
static struct i2c_gpio_platform_data i2c##NO##_gpio_data = {	\
	.sda_pin	= GPIO_I2C_SDA,				\
	.scl_pin	= GPIO_I2C_SCK,				\
};								\
static struct platform_device i2c##NO##_gpio_device = {     	\
	.name	= "i2c-gpio",					\
	.id	= NO,						\
	.dev	= { .platform_data = &i2c##NO##_gpio_data,},	\
};


#ifndef CONFIG_I2C0_JZ4780
DEF_GPIO_I2C(0,GPIO_PD(30),GPIO_PD(31));
#endif
#ifndef CONFIG_I2C1_JZ4780
DEF_GPIO_I2C(1,GPIO_PE(30),GPIO_PE(31));
#endif
#ifndef CONFIG_I2C2_JZ4780
DEF_GPIO_I2C(2,GPIO_PF(16),GPIO_PF(17));
#endif
#ifndef CONFIG_I2C3_JZ4780
DEF_GPIO_I2C(3,GPIO_PD(10),GPIO_PD(11));
#endif
#ifndef CONFIG_I2C4_JZ4780
DEF_GPIO_I2C(4,GPIO_PE(12),GPIO_PE(13));
#endif

#endif /*CONFIG_I2C_GPIO*/


static int __init t700d_i2c_dev_init(void)
{
#ifdef CONFIG_I2C_GPIO

#ifndef CONFIG_I2C0_JZ4780
	platform_device_register(&i2c0_gpio_device);
#endif
#ifndef CONFIG_I2C1_JZ4780
	platform_device_register(&i2c1_gpio_device);
#endif
#ifndef CONFIG_I2C2_JZ4780
	platform_device_register(&i2c2_gpio_device);
#endif
#ifndef CONFIG_I2C3_JZ4780
	platform_device_register(&i2c3_gpio_device);
#endif
#ifndef CONFIG_I2C4_JZ4780
	platform_device_register(&i2c4_gpio_device);
#endif

#endif


#if (defined(CONFIG_I2C1_JZ4780) || defined(CONFIG_I2C_GPIO))
	i2c_register_board_info(1, t700d_i2c1_devs, ARRAY_SIZE(t700d_i2c1_devs));
#endif

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C2_JZ4780))
	i2c_register_board_info(2, t700d_i2c2_devs, ARRAY_SIZE(t700d_i2c2_devs));
#endif

#if (defined(CONFIG_I2C3_JZ4780) || defined(CONFIG_I2C_GPIO))
	i2c_register_board_info(3, t700d_i2c3_devs, ARRAY_SIZE(t700d_i2c3_devs));
#endif
	return 0;
}

arch_initcall(t700d_i2c_dev_init);
