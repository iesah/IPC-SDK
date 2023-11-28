#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <gpio.h>

#include "test.h"

#ifdef CONFIG_KEYBOARD_GPIO
static struct gpio_keys_button board_buttons[] = {
	{
		.gpio		= GPIO_PF(29),
		.code   	= KEY_HOME,
		.desc		= "home key",
		.active_low	= 1,
	},
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

#ifdef CONFIG_JZ4780_SUPPORT_TSC
static struct jztsc_pin test_tsc_gpio[] = {
	[0] = {GPIO_GT801_IRQ,		HIGH_ENABLE},
	[1] = {GPIO_GT801_SHUTDOWN,	HIGH_ENABLE},
	[2] = {GPIO_TP_DRV_EN,		HIGH_ENABLE},
};

static struct jztsc_platform_data test_tsc_pdata = {
	.gpio	= test_tsc_gpio,
};

static struct i2c_board_info test_i2c0_devs[] __initdata = {
	{
		I2C_BOARD_INFO("gt801_ts", 0x55),
		.irq = 0,
		.platform_data	= &test_tsc_pdata,
	},
};
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

struct jz47xx_spi_info spi0_info_cfg = {       
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

struct jz47xx_spi_info spi1_info_cfg = {       
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


static int __init test_board_init(void)
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
#ifdef CONFIG_MMC0_JZ4780
	jz_device_register(&jz_msc0_device, &test_inand_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4780
	jz_device_register(&jz_msc1_device, &test_tf_pdata);
#endif
#ifdef CONFIG_MMC2_JZ4780
	jz_device_register(&jz_msc2_device, &test_sdio_pdata);
#endif

/* sound */
#ifdef CONFIG_SOUND_I2S0_JZ47XX
	jz_device_register(&jz_i2s0_device,&i2s0_data);
#endif
#ifdef CONFIG_SOUND_I2S1_JZ47XX
	jz_device_register(&jz_i2s1_device,&i2s1_data);
#endif
#ifdef CONFIG_SOUND_PCM0_JZ47XX
	jz_device_register(&jz_pcm0_device,&pcm0_data);
#endif
#ifdef CONFIG_SOUND_PCM1_JZ47XX
	jz_device_register(&jz_pcm1_device,&pcm1_data);
#endif
#ifdef CONFIG_JZ4780_INTERNAL_CODEC
	jz_device_register(&jz_codec_device, &codec_data);
#endif

/* panel and bl */
#ifdef CONFIG_LCD_AUO_A043FL01V2
	platform_device_register(&auo_a043fl01v2_device);
#endif
#ifdef CONFIG_LCD_AT070TN93
	platform_device_register(&at070tn93_device);
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	platform_device_register(&test_backlight_device);
#endif

/* lcdc framebuffer*/
#ifdef CONFIG_FB_JZ4780_LCDC1
	jz_device_register(&jz_fb1_device, &jzfb1_pdata);
#endif
#ifdef CONFIG_FB_JZ4780_LCDC0
	jz_device_register(&jz_fb0_device, &jzfb0_pdata);
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

#ifdef CONFIG_JZ_CIM
	platform_device_register(&jz_cim_device);
#endif

#ifdef CONFIG_USB_OHCI_HCD
	platform_device_register(&jz_ohci_device);
#endif

#ifdef CONFIG_USB_EHCI_HCD
	platform_device_register(&jz_ehci_device);
#endif

#ifdef CONFIG_JZ_MAC
	platform_device_register(&jz_mac);
#endif

#ifdef CONFIG_KEYBOARD_GPIO
	platform_device_register(&jz_button_device);
#endif
/* nand */
#ifdef CONFIG_NAND_DRIVER
	jz_device_register(&jz_nand_device, NULL);
#endif

#ifdef CONFIG_HDMI_JZ4780
	platform_device_register(&jz_hdmi);
#endif

#ifdef CONFIG_JZ4780_SUPPORT_TSC
	i2c_register_board_info(0, test_i2c0_devs, ARRAY_SIZE(test_i2c0_devs));
#endif	
#ifdef CONFIG_RTC_DRV_JZ4780
	platform_device_register(&jz_rtc_device);
#endif


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


	return 0;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return "test";
}

arch_initcall(test_board_init);
