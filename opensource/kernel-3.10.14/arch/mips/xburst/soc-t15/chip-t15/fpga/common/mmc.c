#include <linux/mmc/host.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/delay.h>

#include <mach/jzmmc.h>

#include "board_base.h"

#ifndef CONFIG_NAND
#ifdef CONFIG_JZMMC_V12_MMC0

struct card_gpio mmc_card_gpio = {
	.rst = {
		.num = GPIO_MMC_RST_N,
		.enable_level = GPIO_MMC_RST_N_LEVEL,
	},
};
struct jzmmc_platform_data inand_pdata = {
	.removal  			= DONTCARE,
	.sdio_clk			= 0,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED \
	                                  | MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA | MMC_CAP_NONREMOVABLE,
	.pm_flags			= 0,
	.max_freq			= CONFIG_MMC0_MAX_FREQ,
	.gpio				= &mmc_card_gpio,
	.type               = NONREMOVABLE,
#ifdef CONFIG_MMC0_PIO_MODE
	.pio_mode			= 1,
#else
	.pio_mode			= 0,
#endif
	.private_init			= NULL,
};
#endif
#endif

#ifdef CONFIG_JZMMC_V12_MMC1
static struct card_gpio tf_gpio = {
	.cd				= {-1,	-1},
	.wp             = {-1,	-1},
	.pwr			= {-1,	-1},
};

/* common pdata for both tf_card and sdio wifi on fpga board */
struct jzmmc_platform_data tf_pdata = {
	.removal  			= DONTCARE,/*REMOVABLE*/
	.sdio_clk			= 0,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED /*| MMC_CAP_4_BIT_DATA*/,
	.pm_flags			= 0,
	.recovery_info			= NULL,
	.gpio				= &tf_gpio,
	.max_freq                       = CONFIG_MMC1_MAX_FREQ,
#ifdef CONFIG_MMC1_PIO_MODE
	.pio_mode                       = 1,
#else
	.pio_mode                       = 0,
#endif
};
#endif

/* extern int bcm_wlan_init(void); */
/* struct jzmmc_platform_data sdio_pdata = { */
/* 	.removal  			= MANUAL, */
/* 	.sdio_clk			= 1, */
/* 	.ocr_avail			= MMC_VDD_29_30 | MMC_VDD_30_31, */
/* 	.capacity  			= MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ| MMC_CAP_NONREMOVABLE, */
/* 	.max_freq                       = CONFIG_MMC1_MAX_FREQ, */
/* 	.recovery_info			= NULL, */
/* 	.gpio				= NULL, */
/* #ifdef CONFIG_MMC1_PIO_MODE */
/* 	.pio_mode			= 1, */
/* #else */
/* 	.pio_mode			= 0, */
/* #endif */
/* 	.private_init			= bcm_wlan_init, */
/* }; */
/* #endif */
