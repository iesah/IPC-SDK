#include <linux/mmc/host.h>

#include <mach/jzmmc.h>

#include "giant.h"

#define KBYTE				(1024LL)
#define MBYTE				((KBYTE)*(KBYTE))
#define UINT32_MAX			(0xffffffffU)

struct mmc_partition_info giant_inand_partition_info[] = {
	[0] = {"mbr",           0,       512, 0}, 	//0 - 512KB
	[1] = {"xboot",		0,     2*MBYTE, 0}, 	//0 - 2MB
	[2] = {"boot",      3*MBYTE,   8*MBYTE, 0}, 	//3MB - 8MB
	[3] = {"recovery", 12*MBYTE,   8*MBYTE, 0}, 	//12MB - 8MB
	[4] = {"misc",     21*MBYTE,   4*MBYTE, 0}, 	//21MB - 4MB
	[5] = {"battery",  26*MBYTE,   1*MBYTE, 0}, 	//26MB - 1MB
	[6] = {"cache",    28*MBYTE,  30*MBYTE, 1}, 	//28MB - 30MB
	[7] = {"device_id",59*MBYTE,   2*MBYTE, 0},	//59MB - 2MB
	[8] = {"system",   64*MBYTE, 256*MBYTE, 1}, 	//64MB - 256MB
	[9] = {"data",    321*MBYTE, 512*MBYTE, 1}, 	//321MB - 512MB
};

static struct mmc_recovery_info giant_inand_recovery_info = {
	.partition_info			= giant_inand_partition_info,
	.partition_num			= ARRAY_SIZE(giant_inand_partition_info),
	.permission			= MMC_BOOT_AREA_PROTECTED,
	.protect_boundary		= 21*MBYTE,
};
	
struct jzmmc_platform_data giant_inand_pdata = {
	.removal  			= DONTCARE,
	.sdio_clk			= 0,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
//	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_4_BIT_DATA | MMC_CAP_NONREMOVABLE,
	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
	.pm_flags			= 0,
	.recovery_info			= &giant_inand_recovery_info,
	.gpio				= NULL,
};

static struct card_gpio giant_tf_gpio = {
	.cd				= {GPIO_SD2_CD_N,	LOW_ENABLE},
	.pwr				= {GPIO_SD2_VCC_EN_N,	HIGH_ENABLE},
};

struct jzmmc_platform_data giant_tf_pdata = {
	.removal  			= REMOVABLE,
	.sdio_clk			= 0,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_4_BIT_DATA,
	.pm_flags			= 0,
	.recovery_info			= NULL,
	.gpio				= &giant_tf_gpio,
};

struct jzmmc_platform_data giant_sdio_pdata = {
	.removal  			= MANUAL,
	.sdio_clk			= 1,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_4_BIT_DATA,
	.pm_flags			= MMC_PM_IGNORE_PM_NOTIFY,
	.recovery_info			= NULL,
	.gpio				= NULL,
};
