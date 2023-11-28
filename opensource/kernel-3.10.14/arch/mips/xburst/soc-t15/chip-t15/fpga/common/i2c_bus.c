#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/interrupt.h>
#include <linux/i2c/pca953x.h>
#include "board_base.h"
#include "mach/jzsnd.h"
/* *****************************touchscreen******************************* */
#ifdef CONFIG_TOUCHSCREEN_GWTC9XXXB
#include <linux/tsc.h>
static struct jztsc_pin gwtc9xxb_tsc_gpio[] = {
	[0] = {GPIO_TP_INT, LOW_ENABLE},
	[1] = {GPIO_TP_WAKE, HIGH_ENABLE},
};

struct jztsc_platform_data gwtc9xxb_tsc_pdata = {
	.gpio = gwtc9xxb_tsc_gpio,
	.x_max = 800,
	.y_max = 480,
};
#endif

#ifdef CONFIG_TOUCHSCREEN_FT6X0X
#include <linux/i2c/ft6x0x_ts.h>
extern int touch_power_init(struct device *dev);
extern int touch_power_on(struct device *dev);
extern int touch_power_off(struct device *dev);

static struct jztsc_pin ft6x0x_tsc_gpio[] = {
	 [0] = {GPIO_TP_INT, LOW_ENABLE},
	 [1] = {GPIO_TP_WAKE, LOW_ENABLE},
};

static struct ft6x0x_platform_data ft6x0x_tsc_pdata = {
	.gpio           = ft6x0x_tsc_gpio,
	.x_max          = 240,
	.y_max          = 240,
	.fw_ver         = 0x21,
#ifdef CONFIG_KEY_SPECIAL_POWER_KEY
	.blight_off_timer = 3000,   //3s
#endif
	.power_init     = touch_power_init,
	.power_on       = touch_power_on,
	.power_off      = touch_power_off,
};

#endif

#ifdef CONFIG_TOUCHSCREEN_FT6X06
#include <linux/i2c/ft6x06_ts.h>
struct ft6x06_platform_data ft6x06_tsc_pdata = {
	.x_max          = 300,
	.y_max          = 540,
	.va_x_max	= 300,
	.va_y_max	= 480,
	.irqflags = IRQF_TRIGGER_FALLING|IRQF_DISABLED,
	.irq = GPIO_TP_INT,
	.reset = GPIO_TP_RESET,
};
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5336
#include <linux/i2c/ft5336_ts.h>
static struct ft5336_platform_data ft5336_tsc_pdata = {
	.x_max          = 540,
	.y_max          = 1020,
	.va_x_max	= 540,
	.va_y_max	= 960,
	.irqflags = IRQF_TRIGGER_FALLING|IRQF_DISABLED,
	.irq = GPIO_TP_INT,
	.reset = GPIO_TP_RESET,
};
#endif
/* *****************************touchscreen end*************************** */

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C0_V12_JZ))
struct i2c_board_info jz_i2c0_devs[] __initdata = {
};
#endif

#ifdef CONFIG_GPIO_PCA953X
static struct pca953x_platform_data dorado_pca953x_pdata = {
	.gpio_base  = PCA9539_GPIO_BASE,
	.irq_base  = IRQ_RESERVED_BASE + 101,
	.reset_n  = PCA9539_RST_N,
	.irq_n  = PCA9539_IRQ_N,
};
#endif

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C1_V12_JZ))
struct i2c_board_info jz_i2c1_devs[] __initdata = {
#ifdef CONFIG_GPIO_PCA953X
	{
		I2C_BOARD_INFO("pca9539",0x74),
		.platform_data  = &dorado_pca953x_pdata,
	},
#endif
};
#endif  /*I2C1*/

#ifdef CONFIG_WM8594_CODEC_V12
static struct snd_codec_data wm8594_codec_pdata = {
		.codec_sys_clk = 1200000,
};
#endif


#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C3_V12_JZ))
struct i2c_board_info jz_i2c3_devs[] __initdata = {
#ifdef CONFIG_WM8594_CODEC_V12
	{
		I2C_BOARD_INFO("wm8594",0x1a),
		.platform_data = &wm8594_codec_pdata,
	},
#endif
};
#endif  /*I2C3*/
#if	(defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
int jz_i2c0_devs_size = ARRAY_SIZE(jz_i2c0_devs);
#endif

#if	(defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
int jz_i2c1_devs_size = ARRAY_SIZE(jz_i2c1_devs);
#endif

#if	(defined(CONFIG_SOFT_I2C3_GPIO_V12_JZ) || defined(CONFIG_I2C3_V12_JZ))
int jz_i2c3_devs_size = ARRAY_SIZE(jz_i2c3_devs);
#endif
/*
 * define gpio i2c,if you use gpio i2c,
 * please enable gpio i2c and disable i2c controller
 */
#ifdef CONFIG_I2C_GPIO
#define DEF_GPIO_I2C(NO)						\
	static struct i2c_gpio_platform_data i2c##NO##_gpio_data = {	\
		.sda_pin	= GPIO_I2C##NO##_SDA,			\
		.scl_pin	= GPIO_I2C##NO##_SCK,			\
	};								\
	struct platform_device i2c##NO##_gpio_device = {		\
		.name	= "i2c-gpio",					\
		.id	= NO,						\
		.dev	= { .platform_data = &i2c##NO##_gpio_data,},	\
	};

#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
DEF_GPIO_I2C(0);
#endif
#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
DEF_GPIO_I2C(1);
#endif
#ifdef CONFIG_SOFT_I2C2_GPIO_V12_JZ
DEF_GPIO_I2C(2);
#endif
#ifdef CONFIG_SOFT_I2C3_GPIO_V12_JZ
DEF_GPIO_I2C(3);
#endif
#endif /*CONFIG_I2C_GPIO*/
