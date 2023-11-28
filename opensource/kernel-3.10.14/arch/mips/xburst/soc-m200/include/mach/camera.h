#ifndef __ASM_ARCH_OVISP_CAMERA_H
#define __ASM_ARCH_OVISP_CAMERA_H

#include <linux/i2c.h>

/* Camera flags. */
#define CAMERA_USE_ISP_I2C		(0x00000001)
#define CAMERA_USE_HIGH_BYTE		(0x00000002)
#define CAMERA_I2C_PIO_MODE		(0x00010000)
#define CAMERA_I2C_STANDARD_SPEED	(0x00020000)
#define CAMERA_I2C_FAST_SPEED		(0x00040000)
#define CAMERA_I2C_HIGH_SPEED		(0x00080000)

/* Client flags. */
#define CAMERA_CLIENT_CLK_EXT		(0x00000001)
#define CAMERA_CLIENT_IF_MIPI		(0x00000100)
#define CAMERA_CLIENT_IF_DVP		(0x00000200)
#define CAMERA_CLIENT_ISP_BYPASS	(0x00010000)
#define CAMERA_CLIENT_INDEP_I2C		(0x01000000)



struct ovisp_camera_client {
	int i2c_adapter_id;
	struct i2c_board_info *board_info;
	unsigned long flags;
	int if_id;
	int mipi_lane_num;
	const char* mclk_parent_name;
	const char* mclk_name;
	unsigned long mclk_rate;
	int max_video_width;
	int max_video_height;
	int (*power)(int);
	int (*reset)(void);
//	int (*flash)(enum camera_led_mode, int);
	int (*flash)(int, int);
};

struct ovisp_camera_platform_data {
	int i2c_adapter_id;
	unsigned long flags;
	unsigned int client_num;
	struct ovisp_camera_client *client;
};

extern void ovisp_set_camera_info(struct ovisp_camera_platform_data *info);

#endif /* __ASM_ARCH_ovisp_CAMERA_H */
