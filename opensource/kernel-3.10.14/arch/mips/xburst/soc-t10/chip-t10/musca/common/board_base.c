#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/interrupt.h>
#include <linux/jz_dwc.h>
#include <linux/delay.h>
#include <mach/jzsnd.h>
#include <mach/platform.h>
#include <mach/jz_efuse.h>
#include <mach/jzmmc.h>
#include <gpio.h>
#include <linux/pwm.h>
#include "board_base.h"
#include <mach/jzssi.h>
struct jz_platform_device
{
	struct platform_device *pdevices;
	void *pdata;
	int size;
};
static struct jz_platform_device platform_devices_array[] __initdata = {
#define DEF_DEVICE(DEVICE, DATA, SIZE)	\
	{ .pdevices = DEVICE,	\
	  .pdata = DATA, .size = SIZE,}
#ifdef CONFIG_KEYBOARD_GPIO
	DEF_DEVICE(&jz_button_device, 0, 0),
#endif
#ifdef CONFIG_I2C_GPIO
#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
	DEF_DEVICE(&i2c0_gpio_device, 0, 0),
#endif
#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
	DEF_DEVICE(&i2c1_gpio_device, 0, 0),
#endif
#endif	/* CONFIG_I2C_GPIO */

#ifdef CONFIG_I2C0_V12_JZ
	DEF_DEVICE(&jz_i2c0_device, 0, 0),
#endif

#ifdef CONFIG_I2C1_V12_JZ
	DEF_DEVICE(&jz_i2c1_device, 0, 0),
#endif

#ifndef CONFIG_NAND
#ifdef CONFIG_JZMMC_V12_MMC0
	DEF_DEVICE(&jz_msc0_device, &tf_pdata, sizeof(struct jzmmc_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V12_MMC1
	DEF_DEVICE(&jz_msc1_device, &sdio_pdata, sizeof(struct jzmmc_platform_data)),
#endif
#else  /* CONFIG_NAND */
#ifdef CONFIG_JZMMC_V12_MMC1
	DEF_DEVICE(&jz_msc1_device, &sdio_pdata, sizeof(struct jzmmc_platform_data)),
#endif
#endif	/* end CONFIG_NAND */
#ifdef CONFIG_XBURST_DMAC
	DEF_DEVICE(&jz_pdma_device, 0, 0),
#endif
#ifdef CONFIG_JZ_MAC
	DEF_DEVICE(&jz_mii_bus, 0, 0),
	DEF_DEVICE(&jz_mac_device, 0, 0),
#endif
#if defined(CONFIG_SOC_VPU) && defined(CONFIG_JZ_NVPU)
#if (CONFIG_VPU_NODE_NUM >= 1)
	DEF_DEVICE(&jz_vpu0_device, 0, 0),
#endif
#else
#ifdef CONFIG_JZ_VPU_V12
	DEF_DEVICE(&jz_vpu_device, 0, 0),
#endif
#endif
#ifdef CONFIG_BCM_PM_CORE
	DEF_DEVICE(&bcm_power_platform_device, 0, 0),
#endif
#ifdef CONFIG_JZ_IPU_V12
	DEF_DEVICE(&jz_ipu_device, 0, 0),
#endif
#ifdef CONFIG_JZ_IPU_V13
	DEF_DEVICE(&jz_ipu_device, 0, 0),
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART0
	DEF_DEVICE(&jz_uart0_device, 0, 0),
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	DEF_DEVICE(&jz_uart1_device, 0, 0),
#endif
#ifdef CONFIG_USB_JZ_DWC2
	DEF_DEVICE(&jz_dwc_otg_device, 0, 0),
#endif
#ifdef CONFIG_RTC_DRV_JZ
	DEF_DEVICE(&jz_rtc_device, 0, 0),
#endif
#ifdef CONFIG_JZ_EPD_V12
	DEF_DEVICE(&jz_epd_device, &jz_epd_pdata, sizeof(struct jz_epd_platform_data));
#endif
#ifdef CONFIG_SOUND_JZ_I2S_V12
	DEF_DEVICE(&jz_i2s_device, &i2s_data, sizeof(struct snd_dev_data)),
	DEF_DEVICE(&jz_mixer0_device, &snd_mixer0_data, sizeof(struct snd_dev_data)),
#endif
#ifdef CONFIG_SOUND_JZ_DMIC_V12
	DEF_DEVICE(&jz_dmic_device, &dmic_data, sizeof(struct snd_dev_data)),
	DEF_DEVICE(&jz_mixer3_device, &snd_mixer3_data, sizeof(struct snd_dev_data)),
#endif

	/* nand */
#ifdef CONFIG_NAND_DRIVER
	DEF_DEVICE(&jz_nand_device, NULL,0),
#endif

#ifdef CONFIG_BCMDHD_1_141_66
DEF_DEVICE(&wlan_device, 0, 0),
#endif

#ifdef CONFIG_BROADCOM_RFKILL
DEF_DEVICE(&bt_power_device, 0, 0),
#endif

#ifdef CONFIG_SOUND_JZ_PCM_V12
	DEF_DEVICE(&jz_pcm_device, &pcm_data, sizeof(struct snd_dev_data)),
	DEF_DEVICE(&jz_mixer1_device, &snd_mixer1_data, sizeof(struct snd_dev_data)),
#endif

#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
	DEF_DEVICE(&jz_codec_device, &codec_data, sizeof(struct snd_codec_data)),
#endif

#ifdef CONFIG_JZ_EFUSE_V13
	DEF_DEVICE(&jz_efuse_device, &jz_efuse_pdata, sizeof(struct jz_efuse_platform_data)),
#endif

#ifdef CONFIG_JZ_BATTERY
	DEF_DEVICE(&jz_adc_device, &adc_platform_data, sizeof(struct jz_adc_platform_data));
#endif

#ifdef CONFIG_MFD_JZ_SADC_V13
	DEF_DEVICE(&jz_adc_device, 0, 0),
#endif

#ifdef CONFIG_SPI0_V12_JZ
       DEF_DEVICE(&jz_spi0_device, &spi0_info_cfg, sizeof(struct jz_spi_info));
#endif

/* isp */
#ifdef CONFIG_VIDEO_TX_ISP
       DEF_DEVICE(&tx_isp_csi_platform_device, 0, 0),
       DEF_DEVICE(&tx_isp_vic_platform_device, 0, 0),
       DEF_DEVICE(&tx_isp_core_platform_device, 0, 0),
       DEF_DEVICE(&tx_isp_platform_device, 0, 0),
#endif

#ifdef	CONFIG_MTD_JZ_SFC_NORFLASH
	DEF_DEVICE(&jz_sfc_device,&sfc_info_cfg, sizeof(struct jz_sfc_info)),
#endif
#ifdef CONFIG_JZ_AES
	DEF_DEVICE(&jz_aes_device, 0, 0),
#endif
#ifdef CONFIG_JZ_DES
	DEF_DEVICE(&jz_des_device, 0, 0),
#endif
#ifdef CONFIG_JZ_WDT
	DEF_DEVICE(&jz_wdt_device, 0, 0),
#endif
#ifdef CONFIG_JZ_PWM
	DEF_DEVICE(&jz_pwm_device, 0, 0),
#endif
#ifdef CONFIG_PWM_SDK
	DEF_DEVICE(&jz_pwm_sdk_device, 0, 0),
#endif
#ifdef CONFIG_MFD_JZ_TCU
	DEF_DEVICE(&jz_tcu_device, 0, 0),
#endif
};

static int __init board_base_init(void)
{
	int pdevices_array_size, i;

	pdevices_array_size = ARRAY_SIZE(platform_devices_array);
	for(i = 0; i < pdevices_array_size; i++) {
		if(platform_devices_array[i].size)
			platform_device_add_data(platform_devices_array[i].pdevices,
						 platform_devices_array[i].pdata, platform_devices_array[i].size);
		platform_device_register(platform_devices_array[i].pdevices);
	}

#if (defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
	i2c_register_board_info(0, jz_i2c0_devs, jz_i2c0_devs_size);
#endif

#if (defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
	i2c_register_board_info(1, jz_i2c1_devs, jz_i2c1_devs_size);
#endif

#ifdef CONFIG_SPI_V12_JZ
#ifdef CONFIG_SPI0_V12_JZ
	spi_register_board_info(jz_spi0_board_info, ARRAY_SIZE(jz_spi0_board_info));
#endif
#endif
#ifdef CONFIG_PWM_SDK
	pwm_add_table(jz_pwm_lookup, jz_pwm_lookup_size);
#endif
	return 0;
}

/*
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return CONFIG_PRODUCT_NAME;
}

/*
 * Called by arch/mips/xburst/$SOC/common/pm_p0.c when deep sleep.
 * high 16bit = gpio output type
 * low  16bit = regulator sleep gpio pin
 */
unsigned int get_pmu_slp_gpio_info(void)
{
	unsigned int pmu_slp_gpio_info = -1;
#ifdef GPIO_REGULATOR_SLP
	pmu_slp_gpio_info = GPIO_REGULATOR_SLP;
#endif
#ifdef GPIO_OUTPUT_TYPE
	pmu_slp_gpio_info |= GPIO_OUTPUT_TYPE << 16;
#endif
	return pmu_slp_gpio_info;
}

arch_initcall(board_base_init);
