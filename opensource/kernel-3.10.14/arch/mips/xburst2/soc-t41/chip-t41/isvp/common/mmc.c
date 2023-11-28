/*
 * Platform device support for T41 SoC.
 *
 * Copyright 2016, <wangquanshao@ingenic.cn>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/mmc/host.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/delay.h>

#include <mach/sdhci-jz.h>
#include "board_base.h"


#ifdef CONFIG_MMC_SDHCI_MMC0
struct jz_sdhci_platdata jz_mmc0_pdata = {
	.cd_type  				= JZ_SDHCI_CD_GPIO,
	.ext_cd_gpio			= GPIO_MMC_CD_N,
	.ext_cd_gpio_invert		= GPIO_MMC_CD_N_LEVEL,
	.ext_pwr_gpio			= GPIO_MMC_PWR,
	.ext_pwr_gpio_invert	= GPIO_MMC_PWR_LEVEL,
	.max_width	            = 4,
	.host_caps	            = (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
	.pm_flags               = MMC_PM_IGNORE_PM_NOTIFY,
#ifdef CONFIG_MMC0_PIO_MODE
	.pio_mode               = 1,
#else
	.pio_mode               = 0,
#endif
	.enable_autocmd12       = 0,
	.private_init	        = NULL,
};
#endif

#ifdef CONFIG_MMC_SDHCI_MMC1
struct jz_sdhci_platdata jz_mmc1_pdata = {
	.ext_cd_gpio			= -1,
	.ext_pwr_gpio			= -1,
	.max_width              = 1,
	.host_caps	            = (MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ | MMC_CAP_NONREMOVABLE),
#ifdef CONFIG_MMC_SDHCI_MMC1_PC_1BIT
	.host_caps	            = (MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SDIO_IRQ | MMC_CAP_NONREMOVABLE),
#else
	.host_caps	            = (MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ | MMC_CAP_NONREMOVABLE),
#endif
	.pm_flags               = MMC_PM_IGNORE_PM_NOTIFY,
#ifdef CONFIG_MMC1_PIO_MODE
	.pio_mode			    = 1,
#else
	.pio_mode			    = 0,
#endif
	.enable_autocmd12       = 0,
	.private_init		    = NULL,
};
#endif
