/*
 * [board]-misc.c - This file defines most of devices on the board.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/power/gpio-charger.h>
#include <linux/power/li-ion-charger.h>
#include <linux/power/jz4780-battery.h>
#include <linux/jz4780-adc.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/jz_dwc.h>
#include <linux/android_pmem.h>

#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <gpio.h>

#include "leaf.h"
#include <../drivers/staging/android/timed_gpio.h>

#ifdef CONFIG_KEYBOARD_GPIO
static struct gpio_keys_button board_buttons[] = {
#ifdef GPIO_CALL
	{
		.gpio		= GPIO_CALL,
		.code   	= KEY_SEND,
		.desc		= "call key",
		.active_low	= ACTIVE_LOW_CALL,
	},
#endif
#ifdef GPIO_HOME
	{
		.gpio		= GPIO_HOME,
		.code   	= KEY_HOME,
		.desc		= "home key",
		.active_low	= ACTIVE_LOW_HOME,
	},
#endif
#ifdef GPIO_BACK
	{
		.gpio		= GPIO_BACK,
		.code   	= KEY_BACK,
		.desc		= "back key",
		.active_low	= ACTIVE_LOW_BACK,
	},
#endif
#ifdef GPIO_MENU
	{
		.gpio		= GPIO_MENU,
		.code   	= KEY_MENU,
		.desc		= "menu key",
		.active_low	= ACTIVE_LOW_MENU,
	},
#endif
#ifdef GPIO_ENDCALL
	{
		.gpio		= GPIO_ENDCALL,
		.code   	= KEY_POWER,
		.desc		= "end call key",
		.active_low	= ACTIVE_LOW_ENDCALL,
		.wakeup		= 1,
	},
#endif
#ifdef GPIO_VOLUMEDOWN
	{
		.gpio		= GPIO_VOLUMEDOWN,
		.code   	= KEY_VOLUMEDOWN,
		.desc		= "volum down key",
		.active_low	= ACTIVE_LOW_VOLUMEDOWN,
	},
	{
		.gpio		= GPIO_VOLUMEUP,
		.code   	= KEY_VOLUMEUP,
		.desc		= "volum up key",
		.active_low	= ACTIVE_LOW_VOLUMEUP,
	},
#endif
};
static struct gpio_keys_platform_data board_button_data = {
	.buttons	= board_buttons,
	.nbuttons	= ARRAY_SIZE(board_buttons),
};

static struct platform_device jz_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &board_button_data,
	}
};
#endif

struct timed_gpio vibrator_timed_gpio = {
	.name		= "vibrator",
	.gpio		= GPIO_MOTOR_PIN,
	.active_low	= 0,
	.max_timeout	= 15000,
};
static struct timed_gpio_platform_data vibrator_platform_data = {
	.num_gpios	= 1,
	.gpios		= &vibrator_timed_gpio,
};
static struct platform_device jz_timed_gpio_device = {
	.name	= TIMED_GPIO_NAME,
	.id	= 0,
	.dev	= {
		.platform_data	= &vibrator_platform_data,
	},
};

/* Battery Info */
#ifdef CONFIG_BATTERY_JZ4780
static struct jz_battery_info  leaf_battery_info = {
		.max_vol        = 4080,
		.min_vol        = 3600,
		.usb_max_vol    = 4100,
		.usb_min_vol    = 3760,
		.ac_max_vol     = 4150,
		.ac_min_vol     = 3760,
		.battery_max_cpt = 3500,
		.ac_chg_current = 1000,
		.usb_chg_current = 400,
		.sleep_current = 30,
};
static struct jz_adc_platform_data adc_platform_data;
#endif


/* ac charger */
static char *leaf_ac_supplied_to[] = {
	"li_ion_charge",
};

static struct gpio_charger_platform_data leaf_ac_charger_pdata = {
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.gpio = GPIO_PA(16),
	.gpio_active_low = 0,
	.supplied_to = leaf_ac_supplied_to,
	.num_supplicants = ARRAY_SIZE(leaf_ac_supplied_to),
};

static struct platform_device leaf_ac_charger_device = {
	.name = "gpio-charger",
	.dev = {
		.platform_data = &leaf_ac_charger_pdata,
	},
};

/* li-ion charger */
static struct li_ion_charger_platform_data leaf_li_ion_charger_pdata = {
	.gpio = GPIO_PB(3),
	.gpio_active_low = 1,
};

static struct platform_device leaf_li_ion_charger_device = {
	.name = "li-ion-charger",
	.dev = {
		.platform_data = &leaf_li_ion_charger_pdata,
	},
};

#ifdef CONFIG_SPI_JZ4780
#ifdef CONFIG_SPI0_JZ4780
static struct spi_board_info jz_spi0_board_info[] = {
       [0] = {
	       .modalias       = "spidev",
	       .bus_num	       = 0,
	       .chip_select    = 0,
	       .max_speed_hz   = 1200000,
       },
};

static struct jz47xx_spi_info spi0_info_cfg = {
       .chnl = 0,
       .bus_num = 0,
       .max_clk = 54000000,
       .num_chipselect = 2,
};
#endif

#ifdef CONFIG_SPI1_JZ4780
#if defined(CONFIG_TD_SC8800S) || defined(CONFIG_TD_SC8800S_MODULE)
#include <linux/sc8800s.h>
static struct sc8800s_platform_data sc8800s_pdata = {
        //.td_rts_irq     = IRQ_GPIO_0 + BB_AP_SPI_RTS,
        //.td_rdy_irq     = IRQ_GPIO_0 + BB_AP_SPI_RDY,
        .td_rts_pin     = GPIO_PE(3), //BB_AP_SPI_RTS
        .td_rdy_pin     = GPIO_PE(16), //BB_AP_SPI_RDY
	.ap_bb_spi_rts	= GPIO_PE(5),
	.ap_bb_spi_rdy = GPIO_PE(14),
	.power_on	= GPIO_PD(12),     
};
#endif

static struct spi_board_info jz_spi1_board_info[] = {
#if defined(CONFIG_TD_SC8800S) || defined(CONFIG_TD_SC8800S_MODULE)
    [0] = {
	       .modalias       = "sc8800s",
	       .chip_select    = 1,
	       .bus_num	       = 1,
	       .max_speed_hz   = 12000000,
	       .mode           = SPI_MODE_3 | SPI_LSB_FIRST,
	       .platform_data  = &sc8800s_pdata,
    },
#endif
};

static struct jz47xx_spi_info spi1_info_cfg = {
       .chnl = 1,
       .bus_num = 1,
       .max_clk = 54000000,
       .num_chipselect = 2,
};
#endif
#endif
#if defined(CONFIG_SPI_GPIO)
static struct spi_gpio_platform_data jz4780_spi_gpio_data = {
	.sck    = (1*32 + 28),
	.mosi   = (1*32 + 29),
	.miso   = (1*32 + 20),
	.num_chipselect	= 2,
};

static struct platform_device jz4780_spi_gpio_device = {
	.name	= "spi_gpio",
	.dev	= {
		.platform_data = &jz4780_spi_gpio_data,
	},
};

static struct spi_board_info jz_spi0_board_info[] = {
       [0] = {
	       .modalias       = "spidev",
	       .bus_num	       = 0,
	       .chip_select    = 0,
	       .max_speed_hz   = 120000,
       },
};
#endif

#if (defined(CONFIG_USB_DWC2) || defined(CONFIG_USB_DWC_OTG)) && defined(GPIO_USB_DETE)
struct jzdwc_pin dete_pin = {
	.num				= GPIO_USB_DETE,
	.enable_level			= HIGH_ENABLE,
};
#endif

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_camera_pdata = {
	.name = "pmem_camera",
	.no_allocator = 0,
	.cached = 1,
	.start = JZ_PMEM_CAMERA_BASE,
	.size = JZ_PMEM_CAMERA_SIZE,
};


static struct platform_device pmem_camera_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_camera_pdata },
};
#endif

static int __init leaf_board_init(void)
{
/* dma */
#ifdef CONFIG_XBURST_DMAC
	platform_device_register(&jz_pdma_device);
#endif
/* i2c */
#ifdef CONFIG_I2C0_JZ4780
	platform_device_register(&jz_i2c0_device);
#endif
#ifdef CONFIG_I2C1_JZ4780
	platform_device_register(&jz_i2c1_device);
#endif
#ifdef CONFIG_I2C2_JZ4780
	platform_device_register(&jz_i2c2_device);
#endif
#ifdef CONFIG_I2C3_JZ4780
	platform_device_register(&jz_i2c3_device);
#endif
#ifdef CONFIG_I2C4_JZ4780
	platform_device_register(&jz_i2c4_device);
#endif
/* ipu */
#ifdef CONFIG_JZ4780_IPU
	platform_device_register(&jz_ipu0_device);
#endif
#ifdef CONFIG_JZ4780_IPU
	platform_device_register(&jz_ipu1_device);
#endif
/* mmc */
#ifndef CONFIG_NAND_DRIVER
#ifdef CONFIG_MMC0_JZ4780
	jz_device_register(&jz_msc0_device, &leaf_inand_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4780
	jz_device_register(&jz_msc1_device, &leaf_sdio_pdata);
#endif
#ifdef CONFIG_MMC2_JZ4780
	jz_device_register(&jz_msc2_device, &leaf_tf_pdata);
#endif
#else
#ifdef CONFIG_MMC0_JZ4780
	jz_device_register(&jz_msc0_device, &leaf_tf_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4780
	jz_device_register(&jz_msc1_device, &leaf_sdio_pdata);
#endif
#endif
/* sound */
#ifdef CONFIG_SOUND_I2S_JZ47XX
	jz_device_register(&jz_i2s_device,&i2s_data);
	jz_device_register(&jz_mixer0_device,&snd_mixer0_data);
#endif
#ifdef CONFIG_SOUND_PCM_JZ47XX
	jz_device_register(&jz_pcm_device,&pcm_data);
	jz_device_register(&jz_mixer1_device,&snd_mixer1_data);
#endif
#ifdef CONFIG_JZ4780_INTERNAL_CODEC
	jz_device_register(&jz_codec_device, &codec_data);
#endif
/* GPU */
#ifdef CONFIG_PVR_SGX
	platform_device_register(&jz_gpu);
#endif
/* panel and bl */
#ifdef CONFIG_BACKLIGHT_PWM
	platform_device_register(&leaf_backlight_device);
#endif
#ifdef CONFIG_LCD_KR080LA4S_250
	platform_device_register(&kr080la4s_250_device);
#endif
#ifdef CONFIG_LCD_CRD080TI01_40NM01
	platform_device_register(&crd080ti01_40nm01_device);
#endif
#ifdef CONFIG_LCD_EK070TN93
	platform_device_register(&jzlcd_device);
#endif
/* lcdc framebuffer*/
#ifdef CONFIG_FB_JZ4780_LCDC1
	jz_device_register(&jz_fb1_device, &jzfb1_pdata);
#endif
#ifdef CONFIG_FB_JZ4780_LCDC0
	jz_device_register(&jz_fb0_device, &jzfb0_hdmi_pdata);
#endif
/* AOSD */
#ifdef CONFIG_JZ4780_AOSD
	platform_device_register(&jz_aosd_device);
#endif
/* ADC*/
#ifdef CONFIG_BATTERY_JZ4780
	adc_platform_data.battery_info = leaf_battery_info;
	jz_device_register(&jz_adc_device,&adc_platform_data);
#endif

/*bcm4330 bt*/
#ifdef CONFIG_BCM4330_RFKILL
	platform_device_register(&bcm4330_bt_power_device);
#endif


/* ac charger */
	platform_device_register(&leaf_ac_charger_device);
/* li-ion charger */
	platform_device_register(&leaf_li_ion_charger_device);

/* uart */
#ifdef CONFIG_SERIAL_JZ47XX_UART0
	platform_device_register(&jz_uart0_device);
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	platform_device_register(&jz_uart1_device);
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART2
	platform_device_register(&jz_uart2_device);
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART3
	platform_device_register(&jz_uart3_device);
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART4
	platform_device_register(&jz_uart4_device);
#endif
/* camera */
#ifdef CONFIG_JZ_CIM
	platform_device_register(&jz_cim_device);
#endif
/* x2d */
#ifdef CONFIG_JZ_X2D
	platform_device_register(&jz_x2d_device);
#endif
/* USB */
#ifdef CONFIG_USB_OHCI_HCD
	platform_device_register(&jz_ohci_device);
#endif
#ifdef CONFIG_USB_EHCI_HCD
	platform_device_register(&jz_ehci_device);
#endif
/* net */
#ifdef CONFIG_JZ_MAC
	platform_device_register(&jz_mac);
#endif
/* nand */
#ifdef CONFIG_NAND_DRIVER
	jz_device_register(&jz_nand_device, NULL);
#endif
/* hdmi */
#ifdef CONFIG_HDMI_JZ4780
	platform_device_register(&jz_hdmi);
#endif
/* rtc */
#ifdef CONFIG_RTC_DRV_JZ4780
	platform_device_register(&jz_rtc_device);
#endif
/* timed_gpio */
	platform_device_register(&jz_timed_gpio_device);
/* gpio keyboard */
#ifdef CONFIG_KEYBOARD_GPIO
	platform_device_register(&jz_button_device);
#endif
/* tcsm */
#ifdef CONFIG_JZ_VPU
	platform_device_register(&jz_vpu_device);
#endif
/* spi */
#ifdef CONFIG_SPI_JZ4780
#ifdef CONFIG_SPI0_JZ4780
       spi_register_board_info(jz_spi0_board_info, ARRAY_SIZE(jz_spi0_board_info));
       platform_device_register(&jz_ssi0_device);
       platform_device_add_data(&jz_ssi0_device, &spi0_info_cfg, sizeof(struct jz47xx_spi_info));
#endif

#ifdef CONFIG_SPI1_JZ4780
       spi_register_board_info(jz_spi1_board_info, ARRAY_SIZE(jz_spi1_board_info));
       platform_device_register(&jz_ssi1_device);
       platform_device_add_data(&jz_ssi1_device, &spi1_info_cfg, sizeof(struct jz47xx_spi_info));
#endif
#endif

#ifdef CONFIG_SPI_GPIO
       spi_register_board_info(jz_spi0_board_info, ARRAY_SIZE(jz_spi0_board_info));
       platform_device_register(&jz4780_spi_gpio_device);
#endif

#ifdef CONFIG_ANDROID_PMEM
	platform_device_register(&pmem_camera_device);
#endif

#ifdef CONFIG_USB_DWC2
	platform_device_register(&jz_dwc_otg_device);
#endif

#ifdef CONFIG_JZ_MODEM
	platform_device_register(&jz_modem_device);
#endif

	return 0;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return "leaf";
}

arch_initcall(leaf_board_init);
