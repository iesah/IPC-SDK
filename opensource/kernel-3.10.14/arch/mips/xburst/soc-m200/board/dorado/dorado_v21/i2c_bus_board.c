#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/interrupt.h>
#include <linux/i2c/pca953x.h>
#include "board.h"

#ifdef CONFIG_GPIO_PCA953X
static struct pca953x_platform_data dorado_pca953x_pdata = {
	.gpio_base  = PCA9539_GPIO_BASE,
        .irq_base  = IRQ_RESERVED_BASE + 101,
        .reset_n  = PCA9539_RST_N,
	.irq_n  = PCA9539_IRQ_N,
 };
#endif

#if (defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
struct i2c_board_info jz_i2c1_devs[] __initdata = {
#ifdef CONFIG_GPIO_PCA953X
	{
		I2C_BOARD_INFO("pca9539",0x74),
		.platform_data  = &dorado_pca953x_pdata,
	},
#endif
};
#endif  /*I2C1*/

#if (defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ))
struct i2c_board_info jz_i2c2_devs[] __initdata = {
};
#endif  /*I2C2*/

#if     defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ)
int jz_i2c1_devs_size = ARRAY_SIZE(jz_i2c1_devs);
#endif

#if     defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ)
int jz_i2c2_devs_size = ARRAY_SIZE(jz_i2c2_devs);
#endif
