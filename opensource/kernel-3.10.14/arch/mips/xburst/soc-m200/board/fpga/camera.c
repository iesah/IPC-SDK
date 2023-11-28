#include <mach/camera.h>
#include <linux/i2c.h>

#if defined(CONFIG_VIDEO_OVISP)
#if defined(CONFIG_VIDEO_OV5647)
static int ov5647_power(int onoff)
{
	printk("############## ov5647 power : %d################\n", onoff);

	if (onoff) {
		;
	} else {
		;
	}

	return 0;
}

static int ov5647_reset(void)
{
	printk("############## ov5647 reset################\n");

	return 0;
}

static struct i2c_board_info ov5647_board_info = {
	.type = "ov5647",
	.addr = 0x36,
};
#endif /* CONFIG_VIDEO_OV5647 */


#if defined(CONFIG_VIDEO_front_camera)
static int front_camera_pwdn = mfp_to_gpio(front_camera_POWERDOWN_PIN);
static int front_camera_power(int onoff)
{
	printk("############## front_camera power : %d################\n", onoff);

	if (onoff) {
		;
	} else {
		;
	}

	return 0;
}

static int front_camera_reset(void)
{
	printk("############## front_camera reset################\n");
	return 0;
}

static struct i2c_board_info front_camera_board_info = {
	.type = "front_camera",
	.addr = 0x31,
};
#endif /* CONFIG_VIDEO_front_camera */

#if defined(CONFIG_VIDEO_OV5645)
static int ov5645_power(int onoff)
{
	printk("############## %s\n", __func__);

	if (onoff) {
		;
	} else {
		;
	}

	return 0;
}

static int ov5645_reset(void)
{
	printk("############## %s\n", __func__);
	return 0;
}

static struct i2c_board_info ov5645_board_info = {
	.type = "ov5645",
	.addr = 0x3c,
};
#endif /* CONFIG_VIDEO_OV5645 */

static struct ovisp_camera_client ovisp_camera_clients[] = {
#if defined(CONFIG_VIDEO_OV5647)
	{
		.board_info = &ov5647_board_info,
		.flags = CAMERA_CLIENT_IF_MIPI,
		.mclk_rate = 26000000,
		.max_video_width = 1280,
		.max_video_height = 960,
		.power = ov5647_power,
		.reset = ov5647_reset,
	},
#endif /* CONFIG_VIDEO_OV5647 */

#if defined(CONFIG_VIDEO_front_camera)
	{
		.board_info = &front_camera_board_info,
		.flags = CAMERA_CLIENT_IF_DVP
				| CAMERA_CLIENT_CLK_EXT
				| CAMERA_CLIENT_ISP_BYPASS,
		.mclk_parent_name = "pll1_mclk",
		.mclk_name = "clkout1_clk",
		.mclk_rate = 24000000,
		.power = front_camera_power,
		.reset = front_camera_reset,
	},
#endif /* CONFIG_VIDEO_front_camera */

#if defined(CONFIG_VIDEO_OV5645)
	{
		.board_info = &ov5645_board_info,
#if 0
		.flags = CAMERA_CLIENT_IF_MIPI
				| CAMERA_CLIENT_CLK_EXT
				| CAMERA_CLIENT_ISP_BYPASS,
#else
		.flags = CAMERA_CLIENT_IF_MIPI,
#endif
		.mclk_rate = 26000000,
		.max_video_width = 1280,
		.max_video_height = 960,
		.power = ov5645_power,
		.reset = ov5645_reset,
	},
#endif /* CONFIG_VIDEO_OV5645 */
};

struct ovisp_camera_platform_data ovisp_camera_info = {
	.i2c_adapter_id = 3,
#ifdef CONFIG_OVISP_I2C
	.flags = CAMERA_USE_ISP_I2C | CAMERA_USE_HIGH_BYTE
			| CAMERA_I2C_PIO_MODE | CAMERA_I2C_STANDARD_SPEED,
#else
	.flags = CAMERA_USE_HIGH_BYTE
			| CAMERA_I2C_PIO_MODE | CAMERA_I2C_STANDARD_SPEED,
#endif
	.client = ovisp_camera_clients,
	.client_num = ARRAY_SIZE(ovisp_camera_clients),
};
#endif /* CONFIG_VIDEO_OVISP */
