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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <linux/syscore_ops.h>
#include <irq.h>

#include <soc/gpio.h>
#include <soc/base.h>
#include <soc/irq.h>

#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <mach/jz4780_efuse.h>
#include <gpio.h>
#include "hdmi-80.h"
#include <../drivers/staging/android/timed_gpio.h>
#include <linux/input/remote.h>

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
#endif
#ifdef GPIO_VOLUMEUP
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

#ifdef CONFIG_HDMI_80_KEY
static struct gpio_keys_button board_keys[] = {
#ifdef GPIO_OTG_SEL
	{
		.gpio		= GPIO_OTG_SEL,
		.code   	= KEY_MENU,
		.desc		= "otg select key",
		.active_low	= ACTIVE_LOW_OTG_SEL,
	},
#endif
};
static struct gpio_keys_platform_data board_key_data = {
	.buttons	= board_keys,
	.nbuttons	= ARRAY_SIZE(board_keys),
};

static struct platform_device jz_key_device = {
	.name		= "hdmi-80-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &board_key_data,
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
static struct jz_battery_info  hdmi_80_battery_info = {
		.max_vol        = 4050,
		.min_vol        = 3600,
		.usb_max_vol    = 4100,
		.usb_min_vol    = 3760,
		.ac_max_vol     = 4100,
		.ac_min_vol     = 3760,
		.battery_max_cpt = 3000,
		.ac_chg_current = 800,
		.usb_chg_current = 400,
		.sleep_current = 30,
};
static struct jz_adc_platform_data adc_platform_data;
#endif
/* efuse */
static struct jz4780_efuse_platform_data jz_efuse_pdata = {
	/* supply 2.5V to VDDQ */
	.gpio_vddq_en_n = -ENODEV,
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

static struct spi_board_info jz_spi0_board_info[] = {
       [0] = {
	       .modalias       = "spidev",
	       .bus_num	       = 0,
	       .chip_select    = 0,
	       .max_speed_hz   = 120000,
       },
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

#ifdef CONFIG_JZ_REMOTE

#define IR_POWER    0
#define IR_HOME     16
#define IR_MENU     17
#define IR_BACK     18

#define IR_VOLUMEUP (4)
#define IR_VOLUMEDOWN   (6)

#define IR_UP       5
#define IR_DOWN     13
#define IR_LEFT     8
#define IR_RIGHT    10
#define IR_OK       9

#define IR_HELP     86
#define IR_SEARCH   94

#define IR_NEXTSONG 81
#define IR_PREVIOUSSONG 93
#define IR_PLAY     85
#define IR_PLAYPAUSE    89

#define IR_MUTE     88

#define IR_1        (0x17)
#define IR_2        (0x1b)
#define IR_3        (0x1f)
#define IR_4        (0x16)
#define IR_5        (0x1a)
#define IR_6        (0x1e)
#define IR_7        (0x15)
#define IR_8        (0x19)
#define IR_9        (0x1d)
#define IR_DOT      (0x14)
#define IR_0        (0x18)
#define IR_BACKSPACE    (0x1c)
unsigned  int jz_remote_key_map[256];
static  int  jz_remote_init(void  *s)
{
    memset(jz_remote_key_map,-1,256*(sizeof(int)));

    jz_remote_key_map[IR_POWER]=KEY_POWER;
    jz_remote_key_map[IR_HOME]=KEY_HOME;
    jz_remote_key_map[IR_MENU]=KEY_MENU;
    jz_remote_key_map[IR_BACK]=KEY_BACK;

    jz_remote_key_map[IR_VOLUMEUP]=KEY_VOLUMEUP;
    jz_remote_key_map[IR_VOLUMEDOWN]=KEY_VOLUMEDOWN;

    jz_remote_key_map[IR_UP]=KEY_UP  ;    
    jz_remote_key_map[IR_DOWN]=KEY_DOWN ;  
    jz_remote_key_map[IR_LEFT]=KEY_LEFT  ;
    jz_remote_key_map[IR_RIGHT]=KEY_RIGHT;
    jz_remote_key_map[IR_OK]=KEY_ENTER  ;

    jz_remote_key_map[IR_HELP]=KEY_HELP;
    jz_remote_key_map[IR_SEARCH]=KEY_SEARCH;

    jz_remote_key_map[IR_NEXTSONG]=KEY_NEXTSONG;
    jz_remote_key_map[IR_PREVIOUSSONG]=KEY_PREVIOUSSONG;
    jz_remote_key_map[IR_PLAY]=KEY_PLAY;
    jz_remote_key_map[IR_PLAYPAUSE]=KEY_PLAYPAUSE;

    jz_remote_key_map[IR_MUTE]=KEY_MUTE;

    jz_remote_key_map[IR_1]=KEY_1;
    jz_remote_key_map[IR_2]=KEY_2;
    jz_remote_key_map[IR_3]=KEY_3;
    jz_remote_key_map[IR_4]=KEY_4;
    jz_remote_key_map[IR_5]=KEY_5;
    jz_remote_key_map[IR_6]=KEY_6;
    jz_remote_key_map[IR_7]=KEY_7;
    jz_remote_key_map[IR_8]=KEY_8;
    jz_remote_key_map[IR_9]=KEY_9;
    jz_remote_key_map[IR_0]=KEY_0;
    jz_remote_key_map[IR_DOT]=KEY_DOT;
    jz_remote_key_map[IR_BACKSPACE]=KEY_BACKSPACE;

    return 0;
}
static struct jz_remote_board_data jz_remote_board_data = {
	.gpio = GPIO_REMOTE_PIN,
	.key_maps=jz_remote_key_map,
	.init= jz_remote_init,
};

static struct platform_device jz_remote_device = {
	.name = "jz-remote",
	.id = -1,
	.dev = {
		.platform_data = &jz_remote_board_data,
	},
};
#endif

#ifdef CONFIG_AX88796C
static struct resource ax88796c_resource[] = {
	[0] = {
		.start = NEMC_CS5_IOBASE,		/* Start of AX88796C base address */
		.end   = NEMC_CS5_IOBASE + 0x3f,	/* End of AX88796C base address */
		.flags = IORESOURCE_MEM,
	},

	[1] = {
		.start = NEMC_IOBASE,
		.end   = NEMC_IOBASE + 0x50,
		.flags = IORESOURCE_MEM,
	},

	[2] = {
		.start = AX_ETH_INT,			/* Interrupt line number */
		.flags = IORESOURCE_IRQ,
	},

	[3] = {
		.name  = "reset_pin",
		.start = AX_ETH_RESET,			/* Reset line number */
		.end   = AX_ETH_RESET,
		.flags = IORESOURCE_IO,
	},
};

struct platform_device net_device_ax88796c = {
	.name  = "ax88796c",
	.id  = -1,
	.num_resources = ARRAY_SIZE(ax88796c_resource),
	.resource = ax88796c_resource,
};

static int __init hdmi_80_board_init(void)
{
	
/* dma */
#ifdef CONFIG_XBURST_DMAC
	platform_device_register(&jz_pdma_device);
#endif
/* mmc */
#ifdef CONFIG_MMC0_JZ4780
	jz_device_register(&jz_msc0_device, &hdmi_80_tf_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4780
	jz_device_register(&jz_msc1_device, &hdmi_80_sdio_pdata);
#endif
#ifdef CONFIG_MMC2_JZ4780
	jz_device_register(&jz_msc2_device, &hdmi_80_tf_pdata);
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
	platform_device_register(&hdmi_80_backlight_device);
#endif

/* lcdc framebuffer*/
#ifdef CONFIG_FB_REG_LCDC1_FIRST
#ifdef CONFIG_FB_JZ4780_LCDC1
       jz_device_register(&jz_fb1_device, &jzfb1_pdata);
#endif
#ifdef CONFIG_FB_JZ4780_LCDC0
       jz_device_register(&jz_fb0_device, &jzfb0_hdmi_pdata);
#endif
#else
#ifdef CONFIG_FB_JZ4780_LCDC0
       jz_device_register(&jz_fb0_device, &jzfb0_hdmi_pdata);
#endif
#ifdef CONFIG_FB_JZ4780_LCDC1
       jz_device_register(&jz_fb1_device, &jzfb1_pdata);
#endif
#endif

/* AOSD */
#ifdef CONFIG_JZ4780_AOSD
	platform_device_register(&jz_aosd_device);
#endif
/* ADC*/
#ifdef CONFIG_BATTERY_JZ4780
	adc_platform_data.battery_info = hdmi_80_battery_info;
	jz_device_register(&jz_adc_device,&adc_platform_data);
#endif
/* efuse */
	jz_device_register(&jz_efuse_device, &jz_efuse_pdata);
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

#ifdef CONFIG_HDMI_80_KEY
	platform_device_register(&jz_key_device);
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

#ifdef CONFIG_JZ_REMOTE
       platform_device_register(&jz_remote_device);
#endif

#ifdef CONFIG_AX88796C
       platform_device_register(&net_device_ax88796c);
#endif

	return 0;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return "hdmi_80";
}

arch_initcall(hdmi_80_board_init);

static struct wake_lock keep_alive_lock;
static int __init hdmi_80_board_lateinit(void)
{
	wake_lock_init(&keep_alive_lock, WAKE_LOCK_SUSPEND, "keep_alive_lock");
	wake_lock(&keep_alive_lock);

	return 0;
}

late_initcall(hdmi_80_board_lateinit);
