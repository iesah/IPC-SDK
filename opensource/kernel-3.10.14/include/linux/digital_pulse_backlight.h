/*
 * Generic digital pulse backlight driver data - see drivers/video/backlight/digital_pulse_bl.c
 */
#ifndef __LINUX_DIGITAL_PULSE_BACKLIGHT_H
#define __LINUX_DIGITAL_PULSE_BACKLIGHT_H

#include <linux/backlight.h>

struct platform_digital_pulse_backlight_data {
	unsigned int digital_pulse_gpio;
	unsigned int max_brightness;
	unsigned int dft_brightness;
	unsigned int max_brightness_step;
	unsigned int high_level_delay_us;
	unsigned int low_level_delay_us;
	int (*init)(struct device *dev);
	int (*notify)(struct device *dev, int brightness);
	void (*exit)(struct device *dev);
	int (*check_fb)(struct device *dev, struct fb_info *info);
};

#endif
