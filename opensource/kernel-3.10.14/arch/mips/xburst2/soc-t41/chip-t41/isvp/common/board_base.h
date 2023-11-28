#ifndef __BOARD_BASE_H__
#define __BOARD_BASE_H__
#include <linux/i2c.h>
#include <linux/pwm.h>
#include <linux/spi/spi.h>

#include <board.h>

#include <mach/jzfb.h>
#ifdef CONFIG_KEYBOARD_GPIO
extern struct platform_device jz_button_device;
#endif
#ifdef CONFIG_INV_MPU_IIO
extern struct mpu_platform_data mpu9250_platform_data;
#endif
#if (defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
extern struct i2c_board_info jz_i2c0_devs[];
extern int jz_i2c0_devs_size;
#endif
#if (defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
extern struct i2c_board_info jz_i2c1_devs[];
extern int jz_i2c1_devs_size;
#endif
#if (defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ))
extern struct i2c_board_info jz_i2c2_devs[];
extern int jz_i2c2_devs_size;
#endif
#if defined(CONFIG_SOFT_I2C3_GPIO_V12_JZ)
extern struct i2c_board_info jz_i2c3_devs[];
extern int jz_i2c3_devs_size;
#endif

#ifdef CONFIG_I2C_GPIO
#ifndef CONFIG_I2C0_V12_JZ
extern struct platform_device i2c0_gpio_device;
#endif
#ifndef CONFIG_I2C1_V12_JZ
extern struct platform_device i2c1_gpio_device;
#endif
#ifndef CONFIG_I2C2_V12_JZ
extern struct platform_device i2c2_gpio_device;
#endif
#ifndef CONFIG_I2C3_V12_JZ
extern struct platform_device i2c3_gpio_device;
#endif
#endif	/* CONFIG_I2C_GPIO */

#if defined(CONFIG_FB_JZ_V16) || defined(CONFIG_FB_JZ_V15)
extern struct platform_device jz_fb_device;
extern struct jzfb_platform_data jzfb_data;
#endif
#ifdef CONFIG_SOUND_OSS_XBURST
extern struct snd_codec_data codec_data;
#endif
#ifdef CONFIG_BCM_PM_CORE
extern struct platform_device bcm_power_platform_device;
#endif
#ifndef CONFIG_NAND
#ifdef CONFIG_MMC_SDHCI_MMC0
extern struct jz_sdhci_platdata jz_mmc0_pdata;
#endif
#endif
#ifdef CONFIG_MMC_SDHCI_MMC1
extern struct jz_sdhci_platdata jz_mmc1_pdata;
#endif

#ifdef CONFIG_JZ_EPD_V12
extern struct platform_device jz_epd_device;
extern struct jz_epd_platform_data jz_epd_pdata;
#endif

#ifdef CONFIG_JZ_BATTERY
extern struct jz_adc_platform_data adc_platform_data;
#endif
#ifdef CONFIG_JZ_EFUSE_V13
extern struct jz_efuse_platform_data jz_efuse_pdata;
#endif
#ifdef CONFIG_JZ_MAC
extern struct platform_device jz_mii_bus;
extern struct platform_device jz_mac_device;
#endif
#ifdef CONFIG_SFC0
extern struct jz_sfc_info sfc0_info_cfg;
#endif
#ifdef CONFIG_SFC1
extern struct jz_sfc_info sfc1_info_cfg;
#endif
#ifdef CONFIG_JZ_WDT
extern struct platform_device jz_wdt_device;
#endif
#ifdef CONFIG_JZ_AES
extern struct platform_device jz_aes_device;
#endif
#ifdef CONFIG_JZ_DES
extern struct platform_device jz_des_device;
#endif
#ifdef CONFIG_JZ_PWM
extern struct platform_device jz_pwm_device;
#endif
#ifdef CONFIG_PWM_SDK
extern struct pwm_lookup jz_pwm_lookup[];
extern int jz_pwm_lookup_size;
extern struct platform_device jz_pwm_sdk_device;
#endif
#ifdef CONFIG_MFD_JZ_TCU
extern struct platform_device jz_tcu_device;
#endif

#ifdef CONFIG_JZ_SPI0
extern struct jz_spi_info spi0_info_cfg;
#endif

#ifdef CONFIG_JZ_SPI1
extern struct jz_spi_info spi1_info_cfg;
#endif

#ifdef CONFIG_JZ_SPI_SLAVE
extern struct platform_device jz_spi_slave_device;
#endif

#ifdef CONFIG_JZ_SPI_SPISLV_LOOPTEST
extern struct jz_spi_info spi1_info_cfg;
#endif

#if defined(CONFIG_USB_ANDROID_HID) || defined(CONFIG_USB_G_HID)
extern struct platform_device jz_hidg_keyboard;
extern struct platform_device jz_hidg_mouse;
#endif
#endif	/* __BOARD_BASE_H__ */
