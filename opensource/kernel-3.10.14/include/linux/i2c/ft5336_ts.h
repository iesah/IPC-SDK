#ifndef __LINUX_FT5336_TS_H__
#define __LINUX_FT5336_TS_H__

/* -- dirver configure -- */
#define CFG_MAX_TOUCH_POINTS	5

#define PRESS_MAX	0xFF
#define FT_PRESS		0x7F

#define FT5336_NAME 	"ft5336_ts"

#define FT_MAX_ID	0x0F
#define FT_TOUCH_STEP	6
#define FT_TOUCH_X_H_POS		3
#define FT_TOUCH_X_L_POS		4
#define FT_TOUCH_Y_H_POS		5
#define FT_TOUCH_Y_L_POS		6
#define FT_TOUCH_EVENT_POS		3
#define FT_TOUCH_ID_POS			5
#define FT_TOUCH_WEIGHT			7

#define POINT_READ_BUF	(3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)

/*register address*/
#define FT5336_REG_FW_VER		0xA6
#define FT5336_REG_POINT_RATE	0x88
#define FT5336_REG_THGROUP	0x80
#define FT5336_REG_PMODE	0xA5
#define PMODE_HIBERNATE		0x03
#define PMODE_ACTIVE		0x00



/* The platform data for the Focaltech ft5x0x touchscreen driver */
struct ft5336_platform_data {
	unsigned int x_max;
	unsigned int y_max;
	unsigned int va_x_max;
	unsigned int va_y_max;
	unsigned long irqflags;
	unsigned int irq;
	unsigned int reset;
};

#endif
