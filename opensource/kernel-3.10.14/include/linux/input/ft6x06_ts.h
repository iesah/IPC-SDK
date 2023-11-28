#ifndef __LINUX_FT6X06_TS_H__
#define __LINUX_FT6X06_TS_H__

/* -- dirver configure -- */
#define CFG_MAX_TOUCH_POINTS	2

#define PRESS_MAX	0xFF
#define FT_PRESS		0x7F

#define FT6X06_NAME 	"ft6x06_ts"

#define FT_MAX_ID	0x0F
#define FT_TOUCH_STEP	6
#define FT_TOUCH_X_H_POS		3
#define FT_TOUCH_X_L_POS		4
#define FT_TOUCH_Y_H_POS		5
#define FT_TOUCH_Y_L_POS		6
#define FT_TOUCH_EVENT_POS		3
#define FT_TOUCH_ID_POS			5
#define FT_TOUCH_WEIGHT_POS			7

#define POINT_READ_BUF	(3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)

/*register address*/
#define FT6x06_REG_FW_VER		0xA6
#define FT6x06_REG_POINT_RATE	0x88
#define FT6x06_REG_THGROUP	0x80
#define FT6x06_REG_D_MODE	0x00
#define FT6x06_REG_G_MODE	0xA4
#define FT6x06_REG_P_MODE	0xA5
#define FT6x06_REG_CTRL	0x86
#define FT6x06_REG_STATE	0xBC


int ft6x06_i2c_Read(struct i2c_client *client, char *writebuf, int writelen,
		    char *readbuf, int readlen);
int ft6x06_i2c_Write(struct i2c_client *client, char *writebuf, int writelen);

/* The platform data for the Focaltech ft5x0x touchscreen driver */
struct ft6x06_platform_data {
	unsigned int x_max;
	unsigned int y_max;
	unsigned int va_x_max;
	unsigned int va_y_max;
	unsigned long irqflags;
	unsigned int irq;
	unsigned int reset;
};

#endif
