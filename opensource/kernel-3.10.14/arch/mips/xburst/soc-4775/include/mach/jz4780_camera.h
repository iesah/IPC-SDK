#ifndef __JZ4780_CAMERA_H__
#define __JZ4780_CAMERA_H__

#define JZ4780_CAMERA_DATA_HIGH		1
#define JZ4780_CAMERA_PCLK_RISING	2
#define JZ4780_CAMERA_VSYNC_HIGH	4

/* define the maximum number of camera sensor that attach to cim controller */
#define MAX_SOC_CAM_NUM		2

/**
 * struct jz4780_camera_pdata - jz47xx camera platform data
 * @mclk_10khz:	master clock frequency in 10kHz units
 * @flags:	JZ4780 camera platform flags
 * @flacam_sensor_pdata: camera sensor reset and power gpios
 */


struct camera_sensor_priv_data {
	unsigned int gpio_rst;
	unsigned int gpio_power;
};

struct jz4780_camera_pdata {
	unsigned long mclk_10khz;
	unsigned long flags;
	struct camera_sensor_priv_data cam_sensor_pdata[MAX_SOC_CAM_NUM];
};

#endif
