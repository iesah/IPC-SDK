#include <linux/mmc/host.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/delay.h>

#include <mach/jzmmc.h>
#include "zpad80.h"

#define GPIO_WIFI_RST_N			GPIO_PF(7)

#define KBYTE				(1024LL)
#define MBYTE				((KBYTE)*(KBYTE))
#define UINT32_MAX			(0xffffffffU)
#define RESET				0
#define NORMAL				1

static struct wifi_data			iw8101_data;

int iw8101_wlan_init(void);

#if !defined(CONFIG_MTD_NAND_JZ4780)
#ifdef CONFIG_MMC0_JZ4780
struct mmc_partition_info zpad80_inand_partition_info[] = {
	[0] = {"mbr",           0,       512, 0}, 	//0 - 512KB
	[1] = {"xboot",		0,     2*MBYTE, 0}, 	//0 - 2MB
	[2] = {"boot",      3*MBYTE,   8*MBYTE, 0}, 	//3MB - 8MB
	[3] = {"recovery", 12*MBYTE,   8*MBYTE, 0}, 	//12MB - 8MB
	[4] = {"misc",     21*MBYTE,   4*MBYTE, 0}, 	//21MB - 4MB
	[5] = {"battery",  26*MBYTE,   1*MBYTE, 0}, 	//26MB - 1MB
	[6] = {"cache",    28*MBYTE,  30*MBYTE, 1}, 	//28MB - 30MB
	[7] = {"device_id",59*MBYTE,   2*MBYTE, 0},	//59MB - 2MB
	[8] = {"system",   64*MBYTE, 512*MBYTE, 1}, 	//64MB - 512MB
	[9] = {"data",    580*MBYTE, 1024*MBYTE, 1}, 	//580MB - 1024MB
};

static struct mmc_recovery_info zpad80_inand_recovery_info = {
	.partition_info			= zpad80_inand_partition_info,
	.partition_num			= ARRAY_SIZE(zpad80_inand_partition_info),
	.permission			= MMC_BOOT_AREA_PROTECTED,
	.protect_boundary		= 21*MBYTE,
};
	
struct jzmmc_platform_data zpad80_inand_pdata = {
	.removal  			= DONTCARE,
	.sdio_clk			= 0,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_4_BIT_DATA | MMC_CAP_NONREMOVABLE,
	.pm_flags			= 0,
	.max_freq			= CONFIG_MMC0_MAX_FREQ,
	.recovery_info			= &zpad80_inand_recovery_info,
	.gpio				= NULL,
#ifdef CONFIG_MMC0_PIO_MODE
	.pio_mode			= 1,
#else
	.pio_mode			= 0,
#endif
	.private_init			= NULL,
};
#endif
#ifdef CONFIG_MMC1_JZ4780
struct jzmmc_platform_data zpad80_sdio_pdata = {
	.removal  			= MANUAL,
	.sdio_clk			= 1,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_4_BIT_DATA,
	.pm_flags			= MMC_PM_IGNORE_PM_NOTIFY,
	.max_freq			= CONFIG_MMC1_MAX_FREQ,
	.recovery_info			= NULL,
	.gpio				= NULL,
#ifdef CONFIG_MMC1_PIO_MODE
	.pio_mode			= 1,
#else
	.pio_mode			= 0,
#endif
	.private_init			= iw8101_wlan_init,
};
#endif
#if defined(CONFIG_MMC2_JZ4780) || defined(CONFIG_MMC0_JZ4780)
/*
 * WARING:
 * If a GPIO is not used or undefined, it must be set -1,
 * or PA0 will be request.
 */
static struct card_gpio zpad80_tf_gpio = {
	.cd				= {GPIO_PA(26),		LOW_ENABLE},//for zpad80
//	.cd				= {-1,		LOW_ENABLE},//for ers
	.wp				= {-1,			-1},
	.pwr			= {-1,			-1},
};

struct jzmmc_platform_data zpad80_tf_pdata = {
//	.removal  			= DONTCARE, //REMOVABLE, for ers
	.removal  			= REMOVABLE, //REMOVABLE, for zpad80
	.sdio_clk			= 0,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_4_BIT_DATA,
	.pm_flags			= 0,
#ifdef CONFIG_MMC0_JZ4780
	.max_freq			= CONFIG_MMC0_MAX_FREQ,
#endif
#ifdef CONFIG_MMC2_JZ4780
	.max_freq			= CONFIG_MMC2_MAX_FREQ,
#endif
	.recovery_info			= NULL,
	.gpio				= &zpad80_tf_gpio,
#ifdef CONFIG_MMC0_PIO_MODE
	.pio_mode			= 1,
#else
	.pio_mode			= 0,
#endif
	.private_init			= NULL,
};
#endif
#else // for Nand boot
#ifdef CONFIG_MMC1_JZ4780
struct jzmmc_platform_data zpad80_sdio_pdata = {
	.removal  			= MANUAL,
	.sdio_clk			= 1,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_4_BIT_DATA,
	.pm_flags			= MMC_PM_IGNORE_PM_NOTIFY,
	.max_freq			= CONFIG_MMC1_MAX_FREQ,
	.recovery_info			= NULL,
	.gpio				= NULL,
#ifdef CONFIG_MMC1_PIO_MODE
	.pio_mode			= 1,
#else
	.pio_mode			= 0,
#endif
	.private_init			= iw8101_wlan_init,
};
#endif
#ifdef CONFIG_MMC0_JZ4780
/*
 * WARING:
 * If a GPIO is not used or undefined, it must be set -1,
 * or PA0 will be request.
 */
static struct card_gpio zpad80_tf_gpio = {
	.cd				= {GPIO_PF(20),		LOW_ENABLE},
	.wp				= {-1,			-1},
	.pwr				= {-1,			-1},
};

struct jzmmc_platform_data zpad80_tf_pdata = {
	.removal  			= REMOVABLE,
	.sdio_clk			= 0,
	.ocr_avail			= MMC_VDD_32_33 | MMC_VDD_33_34,
	.capacity  			= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_4_BIT_DATA,
	.pm_flags			= 0,
#ifdef CONFIG_MMC0_JZ4780
	.max_freq			= CONFIG_MMC0_MAX_FREQ,
#endif
	.recovery_info			= NULL,
	.gpio				= &zpad80_tf_gpio,
#ifdef CONFIG_MMC0_PIO_MODE
	.pio_mode			= 1,
#else
	.pio_mode			= 0,
#endif
	.private_init			= NULL,
};
#endif
#endif
int iw8101_wlan_init(void)
{
	static struct wake_lock	*wifi_wake_lock = &iw8101_data.wifi_wake_lock;
	struct regulator *power;
	int reset;

	power = regulator_get(NULL, "vwifi");
	if (IS_ERR(power)) {
		pr_err("wifi regulator missing\n");
		return -EINVAL;
	}
	iw8101_data.wifi_power = power;

	reset = GPIO_WIFI_RST_N;
	if (gpio_request(GPIO_WIFI_RST_N, "wifi_reset")) {
		pr_err("no wifi_reset pin available\n");
		regulator_put(power);

		return -EINVAL;
	} else {
		gpio_direction_output(reset, 1);
	}
	 iw8101_data.wifi_reset = reset;

	wake_lock_init(wifi_wake_lock, WAKE_LOCK_SUSPEND, "wifi_wake_lock");

	return 0;
}

int IW8101_wlan_power_on(int flag)
{
	static struct wake_lock	*wifi_wake_lock = &iw8101_data.wifi_wake_lock;
	struct regulator *power = iw8101_data.wifi_power;
	int reset = iw8101_data.wifi_reset;

	if (wifi_wake_lock == NULL)
		pr_warn("%s: invalid wifi_wake_lock\n", __func__);
	else if (power == NULL)
		pr_warn("%s: invalid power\n", __func__);
	else if (!gpio_is_valid(reset))
		pr_warn("%s: invalid reset\n", __func__);
	else
		goto start;
	return -ENODEV;
start:
	pr_debug("wlan power on:%d\n", flag);
	jzrtc_enable_clk32k();
	msleep(200);

	switch(flag) {
		case RESET:
			regulator_enable(power);
			jzmmc_clk_ctrl(1, 1);

			gpio_set_value(reset, 0);
			msleep(200);

			gpio_set_value(reset, 1);
			msleep(200);

			break;

		case NORMAL:
			regulator_enable(power);

			gpio_set_value(reset, 0);
			msleep(200);

			gpio_set_value(reset, 1);

			msleep(200);
			jzmmc_manual_detect(1, 1);

			break;
	}
	wake_lock(wifi_wake_lock);

	return 0;
}

int IW8101_wlan_power_off(int flag)
{
	static struct wake_lock	*wifi_wake_lock = &iw8101_data.wifi_wake_lock;
	struct regulator *power = iw8101_data.wifi_power;
	int reset = iw8101_data.wifi_reset;

	if (wifi_wake_lock == NULL)
		pr_warn("%s: invalid wifi_wake_lock\n", __func__);
	else if (power == NULL)
		pr_warn("%s: invalid power\n", __func__);
	else if (!gpio_is_valid(reset))
		pr_warn("%s: invalid reset\n", __func__);
	else
		goto start;
	return -ENODEV;
start:
	pr_debug("wlan power off:%d\n", flag);
	switch(flag) {
		case RESET:
			gpio_set_value(reset, 0);

			regulator_disable(power);
			jzmmc_clk_ctrl(1, 0);
			break;

		case NORMAL:
			gpio_set_value(reset, 0);

			regulator_disable(power);

 			jzmmc_manual_detect(1, 0);
			break;
	}

	wake_unlock(wifi_wake_lock);

	jzrtc_disable_clk32k();

	return 0;
}

void RTL8188_wlan_power_on(void)
{
	regulator_enable(regulator_get(NULL, "vwifi"));
	printk("====== enable PMU out6 3.3v for wifi power on ======\n");
}

void RTL8188_wlan_power_off(void)
{
	regulator_disable(regulator_get(NULL, "vwifi"));
	printk("====== disable PMU out6 3.3v for wifi power off ======\n");
}

EXPORT_SYMBOL(IW8101_wlan_power_on);
EXPORT_SYMBOL(IW8101_wlan_power_off);

EXPORT_SYMBOL(RTL8188_wlan_power_on);
EXPORT_SYMBOL(RTL8188_wlan_power_off);
