#include <mach/camera.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "board_base.h"

int temp = 1;
#if defined(CONFIG_VIDEO_OV9724)
static int ov9724_power(int onoff)
{
	if(temp) {
		gpio_request(CAMERA_PWDN_N, "CAMERA_PWDN_N");
		gpio_request(CAMERA_RST, "CAMERA_RST");
		temp = 0;
	}
	if (onoff) { /* conflict with USB_ID pin */
		//gpio_direction_output(CAMERA_PWDN_N, 0);
		mdelay(10); /* this is necesary */
		//gpio_direction_output(CAMERA_PWDN_N, 1);
		;
	} else {
		//gpio_direction_output(CAMERA_PWDN_N, 0);
		gpio_direction_output(CAMERA_RST, 0);   /*PWM0 */
	}

	return 0;
}

static int ov9724_reset(void)
{
	/*reset*/
	gpio_direction_output(CAMERA_RST, 1);   /*PWM0 */
	mdelay(10);
	gpio_direction_output(CAMERA_RST, 0);   /*PWM0 */
	mdelay(10);
	gpio_direction_output(CAMERA_RST, 1);   /*PWM0 */

	return 0;
}

static struct i2c_board_info ov9724_board_info = {
	.type = "ov9724",
	.addr = 0x10,
};
#endif /* CONFIG_VIDEO_OV9724 */
#if defined(CONFIG_VIDEO_OV5645)
static int ov5645_power(int onoff)
{
	if(temp) {
//		gpio_request(CAMERA_PWDN_N, "CAMERA_PWDN_N");
		gpio_request(CAMERA_RST, "CAMERA_RST");
		temp = 0;
	}
	if (onoff) {
		mdelay(10);
		//gpio_direction_output(CAMERA_RST, 1);   /*PWM0 */
		;
	} else {
		//gpio_direction_output(CAMERA_PWDN_N, 0);
		gpio_direction_output(CAMERA_RST, 0);   /*PWM0 */
		;
	}

	return 0;
}

static int ov5645_reset(void)
{
	/*reset*/
	gpio_direction_output(CAMERA_RST, 1);   /*PWM0 */
	mdelay(10);
	gpio_direction_output(CAMERA_RST, 0);   /*PWM0 */
	mdelay(10);
	gpio_direction_output(CAMERA_RST, 1);   /*PWM0 */
	return 0;
}

static struct i2c_board_info ov5645_board_info = {
	.type = "ov5645",
	.addr = 0x3c,
};
#endif /* CONFIG_VIDEO_OV5645 */

#if defined(CONFIG_VIDEO_OV8858)
static int ov8858_power(int onoff)
{
	if(temp) {
	//gpio_request(CAMERA_PWDN_N, "CAMERA_PWDN_N");
	gpio_request(CAMERA_RST, "CAMERA_RST");
		temp = 0;
	}
	if (onoff) {
	//	gpio_direction_output(CAMERA_PWDN_N, 0);
		mdelay(10);
	//	gpio_direction_output(CAMERA_PWDN_N, 1);
		;
	} else {
	//	gpio_direction_output(CAMERA_PWDN_N, 0);
		gpio_direction_output(CAMERA_RST, 0);   /*PWM0 */
		;
	}
	return 0;
}

static int ov8858_reset(void)
{
	/*reset*/
	gpio_direction_output(CAMERA_RST, 1);   /*PWM0 */
	mdelay(10);
	gpio_direction_output(CAMERA_RST, 0);   /*PWM0 */
	mdelay(10);
	gpio_direction_output(CAMERA_RST, 1);   /*PWM0 */
	mdelay(1000);

	return 0;
}

static struct i2c_board_info ov8858_board_info = {
	.type = "ov8858",
	.addr = 0x10,
	/* .addr = 0x36, */
};
#endif /* CONFIG_VIDEO_OV8858 */

#ifdef CONFIG_DVP_OV9712
static int ov9712_power(int onoff)
{
	if(temp) {
		gpio_request(OV9712_POWER, "OV9712_POWER");
		gpio_request(OV9712_PWDN_EN, "OV9712_PWDN_EN");
		gpio_request(OV9712_RST, "OV9712_RST");
		temp = 0;
	}
	if (onoff) { /* conflict with USB_ID pin */
		gpio_direction_output(OV9712_PWDN_EN, 1);
//		gpio_direction_output(OV9712_RST, 1);
		mdelay(1); /* this is necesary */
		gpio_direction_output(OV9712_POWER, 1);
		mdelay(100); /* this is necesary */
		gpio_direction_output(OV9712_PWDN_EN, 0);
		mdelay(50); /* this is necesary */
		gpio_direction_output(OV9712_RST, 0);
		mdelay(150); /* this is necesary */
		gpio_direction_output(OV9712_RST, 1);
		mdelay(150); /* this is necesary */
	} else {
		//printk("##### power off ######### %s\n", __func__);
		//gpio_direction_output(OV9712_PWDN_EN, 1);
		//gpio_direction_output(OV9712_POWER, 0);
	}

	return 0;
}

static int ov9712_reset(void)
{
	/*reset*/
	gpio_direction_output(OV9712_RST, 0);	/*PWM0*/
	mdelay(150);
	gpio_direction_output(OV9712_RST, 1);	/*PWM0*/
	mdelay(150);

	return 0;
}

static struct i2c_board_info ov9712_board_info = {
	.type = "ov9712",
	.addr = 0x30,
};
#endif /* CONFIG_DVP_OV9712 */


static struct ovisp_camera_client ovisp_camera_clients[] = {
#ifdef CONFIG_VIDEO_OV5645
	{
		.board_info = &ov5645_board_info,
		.flags = CAMERA_CLIENT_IF_MIPI,
		.mclk_rate = 24000000,
		.max_video_width = 1280,
		.max_video_height = 720,
		.power = ov5645_power,
		.reset = ov5645_reset,
	},
#endif /* CONFIG_VIDEO_OV5645 */

#ifdef CONFIG_VIDEO_OV9724
	{
		.board_info = &ov9724_board_info,
#if 0
		.flags = CAMERA_CLIENT_IF_MIPI
		| CAMERA_CLIENT_CLK_EXT
		| CAMERA_CLIENT_ISP_BYPASS,
#else
		.flags = CAMERA_CLIENT_IF_MIPI,
#endif
		.mclk_rate = 26000000,
		//.max_video_width = 1280,
		//.max_video_height = 960,
		.max_video_width = 1280,
		.max_video_height = 720,
		.power = ov9724_power,
		.reset = ov9724_reset,
	},
#endif /* CONFIG_VIDEO_OV9724 */

#ifdef CONFIG_VIDEO_OV8858
	{
		.board_info = &ov8858_board_info,
#if 0
		.flags = CAMERA_CLIENT_IF_MIPI
		| CAMERA_CLIENT_CLK_EXT
		| CAMERA_CLIENT_ISP_BYPASS,
#else
		.flags = CAMERA_CLIENT_IF_MIPI,
#endif
		.mclk_rate = 26000000,
		.max_video_width = 1632,
		.max_video_height = 1224,
		.power = ov8858_power,
		.reset = ov8858_reset,
	},
#endif /* CONFIG_VIDEO_OV8858 */

#ifdef CONFIG_DVP_OV9712
	{
		.board_info = &ov9712_board_info,
#if 0
		.flags = CAMERA_CLIENT_IF_DVP
		| CAMERA_CLIENT_CLK_EXT
		| CAMERA_CLIENT_ISP_BYPASS,
#else
		.flags = CAMERA_CLIENT_IF_DVP | CAMERA_CLIENT_CLK_EXT,
//		.flags = CAMERA_CLIENT_IF_DVP,
#endif
		.mclk_name = "cgu_cim",
		.mclk_rate = 24000000,
		.max_video_width = 1280,
		.max_video_height = 800,
		.power = ov9712_power,
		.reset = ov9712_reset,
	},
#endif /* CONFIG_DVP_OV9712 */
};

struct ovisp_camera_platform_data ovisp_camera_info = {
#ifdef CONFIG_OVISP_I2C
	.i2c_adapter_id = 4, /* larger than host i2c nums */
	.flags = CAMERA_USE_ISP_I2C | CAMERA_USE_HIGH_BYTE
	| CAMERA_I2C_PIO_MODE | CAMERA_I2C_STANDARD_SPEED,
#else
	.i2c_adapter_id = 3, /* use cpu's i2c adapter */
	.flags = CAMERA_USE_HIGH_BYTE
	| CAMERA_I2C_PIO_MODE | CAMERA_I2C_STANDARD_SPEED,
#endif
	.client = ovisp_camera_clients,
	.client_num = ARRAY_SIZE(ovisp_camera_clients),
};
