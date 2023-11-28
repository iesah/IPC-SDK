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
#include <linux/power/jz4780-battery.h>
#include <linux/jz4780-adc.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/jz_dwc.h>
#include <linux/android_pmem.h>
#include <linux/input/matrix_keypad.h>
#include <linux/interrupt.h>
#include <linux/lcd.h>
#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <gpio.h>
#include <linux/gpio.h>
#include "npm3701.h"
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
		.code   	= KEY_VOLUMEUP,
		.desc		= "volum down key",
		.active_low	= ACTIVE_LOW_VOLUMEDOWN,
	},
	{
		.gpio		= GPIO_VOLUMEUP,
		.code   	= KEY_VOLUMEDOWN,
		.desc		= "volum up key",
		.active_low	= ACTIVE_LOW_VOLUMEUP,
	},
#endif
#ifdef GPIO_KEY1
	{
		.gpio		= GPIO_KEY1,
		.code		= KEY_1,
		.desc		= "key 1",
		.active_low	= ACTIVE_LOW_KEY1,
	},
#endif 

#ifdef GPIO_KEY4
	{                      
		.gpio		= GPIO_KEY4,
		.code		= KEY_4,
		.desc		= "key 4",
		.active_low	= ACTIVE_LOW_KEY4,               
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
static struct jz_battery_info  npm3701_battery_info = {
		.max_vol        = 4080,
		.min_vol        = 3600,
		.usb_max_vol    = 4150,
		.usb_min_vol    = 3650,
		.ac_max_vol     = 4180,
		.ac_min_vol     = 3760,
		.battery_max_cpt = 3500,
		.ac_chg_current = 800,
		.usb_chg_current = 400,
		.sleep_current = 20,
};
static struct jz_adc_platform_data adc_platform_data;
#endif
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
static struct spi_board_info jz_spi1_board_info[] = {
    [0] = {
	       .modalias       = "spidev",
	       .bus_num	       = 1,
	       .chip_select    = 1,
	       .max_speed_hz   = 120000,
    },
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
	.sck	= (4*32 + 15),
	.mosi	= (4*32 + 17),
	.miso	= (4*32 + 14),
	.num_chipselect	= 2,
};

static struct platform_device jz4780_spi_gpio_device = {
	.name	= "spi_gpio",
	.dev	= {
		.platform_data = &jz4780_spi_gpio_data,
	},
};

/*static struct spi_board_info jz_spi0_board_info[] = {
       [0] = {
	       .modalias       = "spidev",
	       .bus_num	       = 0,
	       .chip_select    = 0,
	       .max_speed_hz   = 120000,
       },
};*/

#ifdef CONFIG_LCD_S369FG06
/*Control the LCD power supply */
static int lcd_power_on(struct lcd_device *ld, int enable)
{
	int ret = 0;
	if (enable) {
	  /* ret = regulator_enable(xxx);  */
	} else {
	  /* ret = regulator_disable(xxx);  */
	}

	return ret;
}

static int reset_lcd(struct lcd_device *ld)
{
	return 0;
}

struct specific_tl2796 {
  const char *ld_name;			/* lcd device name  */
  const char *bd_name;			/* backlight device name  */
  int lcd_reset;				/* lcd reset pin */
  int spi_cs;					/* spi cs pin */
  
  int upper_margin;				/* see struct fb_videomode */
  int lower_margin;				/* 4 >= lower_margin <= 31 */
  int vsync;					/* 4 >= upper_margin + vsync <= 31 */
};

struct specific_tl2796 s369fg06_tl2796 = {
  .ld_name = "s369fg06",
  .bd_name = "s369fg06-bd",
  .lcd_reset = 1*32 + 22, /*3*32 + 10,*/
  .spi_cs = 4*32 + 16,			/* spi_cs pin */
  .lower_margin = 8,		
  .upper_margin = 7,  
  .vsync = 1,
};

static const struct lcd_platform_data s369fg06_pdata = {
	.reset			= reset_lcd,
	.power_on		= lcd_power_on,
	.lcd_enabled		= 0,
	.reset_delay		= 0,
	.power_on_delay		= 25,
	.power_off_delay	= 200,
	.pdata 				= &s369fg06_tl2796,
};

static struct spi_board_info s369fg06_board_info[] = {
       [0] = {
	       .modalias       = "tl2796", //"spidev",
	       .bus_num	       = 0,
	       .chip_select    = 1,
	       .max_speed_hz   = 120000,
	       .platform_data  = &s369fg06_pdata,
	       .controller_data = (void *)(4*32 +16), /* spi_cs pin */
       },
};
#endif

#endif

#ifdef CONFIG_USB_DWC2
struct jzdwc_pin dwc2_id_pin = {
	.num	= GPIO_PE(2),
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

#ifdef CONFIG_KEYBOARD_MATRIX

#define KEY_R0 GPIO_PB(21)
#define KEY_R1 GPIO_PB(30)
#define KEY_R2 GPIO_PB(31)
#define KEY_R3 GPIO_PB(28)

#define KEY_C0 GPIO_PD(28)
#define KEY_C1 GPIO_PD(26)
#define KEY_C2 GPIO_PD(27)
#define KEY_C3 GPIO_PD(29)

static const unsigned int matrix_keypad_cols[] = {KEY_C0,KEY_C1,KEY_C2,KEY_C3};
static const unsigned int matrix_keypad_rows[] = {KEY_R0,KEY_R1,KEY_R2,KEY_R3};
static uint32_t npm3701_keymap[] =  
{
  //KEY(row, col, keycode)
  KEY(0,0, KEY_5), 
  KEY(0,1, KEY_2), 
  KEY(0,2, KEY_3),
  KEY(0,3, KEY_8),
  KEY(1,0, KEY_9),
  KEY(1,1, KEY_6),
  KEY(1,2, KEY_7),
  KEY(1,3, KEY_0),
  KEY(2,1, KEY_RIGHT),
  KEY(2,2, KEY_LEFT),
  KEY(2,3, KEY_DOWN),
  KEY(3,1, KEY_BACK),
  KEY(3,2, KEY_UP),
  KEY(3,3, KEY_ENTER),
};
static struct matrix_keymap_data npm3701_keymap_data =  
{
  .keymap = npm3701_keymap,
  .keymap_size =ARRAY_SIZE(npm3701_keymap),
};
static struct matrix_keypad_platform_data npm3701_keypad_data  =  
{
  .keymap_data = &npm3701_keymap_data,
  .col_gpios = matrix_keypad_cols,
  .row_gpios = matrix_keypad_rows,
  .num_col_gpios = ARRAY_SIZE(matrix_keypad_cols),
  .num_row_gpios = ARRAY_SIZE(matrix_keypad_rows),
  .col_scan_delay_us = 10,
  .debounce_ms =100,
  .wakeup  = 1,
  .active_low  = 0,
  //.no_autorepeat = 1,
};

static struct platform_device jz_matrix_kdb_device = {
	.name		= "matrix-keypad",
	.id			= 0,
	.dev		= {
		.platform_data	= &npm3701_keypad_data,
	}
};

#endif

static int __init npm3701_board_init(void)
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
	jz_device_register(&jz_msc0_device, &npm3701_inand_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4780
	jz_device_register(&jz_msc1_device, &npm3701_sdio_pdata);
#endif
#ifdef CONFIG_MMC2_JZ4780
	jz_device_register(&jz_msc2_device, &npm3701_tf_pdata);
#endif
#else
#ifdef CONFIG_MMC0_JZ4780
	jz_device_register(&jz_msc0_device, &npm3701_tf_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4780
	jz_device_register(&jz_msc1_device, &npm3701_sdio_pdata);
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
#ifdef CONFIG_LCD_KR070LA0S_270
	platform_device_register(&kr070la0s_270_device);
#endif
#ifdef CONFIG_LCD_EK070TN93
	platform_device_register(&ek070tn93_device);
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	platform_device_register(&npm3701_backlight_device);
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
	adc_platform_data.battery_info = npm3701_battery_info;
	jz_device_register(&jz_adc_device,&adc_platform_data);
#endif
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
//       spi_register_board_info(jz_spi0_board_info, ARRAY_SIZE(jz_spi0_board_info));
#ifdef CONFIG_LCD_S369FG06
	   spi_register_board_info(s369fg06_board_info, ARRAY_SIZE(s369fg06_board_info));
//#else
//       spi_register_board_info(jz_spi0_board_info, ARRAY_SIZE(jz_spi0_board_info));
#endif
       platform_device_register(&jz4780_spi_gpio_device);
#endif

#ifdef CONFIG_ANDROID_PMEM
	platform_device_register(&pmem_camera_device);
#endif

#ifdef CONFIG_USB_DWC2
	platform_device_register(&jz_dwc_otg_device);
#endif
#ifdef CONFIG_KEYBOARD_MATRIX
	platform_device_register(&jz_matrix_kdb_device);
#endif
	return 0;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return "npm3701";
}

arch_initcall(npm3701_board_init);
