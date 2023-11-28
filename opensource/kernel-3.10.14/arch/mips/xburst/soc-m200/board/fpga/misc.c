#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/tsc.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/android_pmem.h>
#include <linux/interrupt.h>

#include <mach/jzsnd.h>
#include <mach/platform.h>
#include <gpio.h>
#include "board.h"

#if defined(CONFIG_JZ4775_MAC)
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
static struct jz_gpio_phy_reset gpio_phy_reset = {
	.port = GPIO_PORT_B,
	.pin = 7,
	.start_func = GPIO_OUTPUT0,
	.end_func = GPIO_OUTPUT1,
	.delaytime_usec = 100000,
};
#endif
struct platform_device jz4775_mii_bus = {
	.name = "jz4775_mii_bus",
#ifdef CONFIG_JZGPIO_PHY_RESET
	.dev.platform_data = &gpio_phy_reset,
#endif
};
#else /* CONFIG_MDIO_GPIO */
static struct mdio_gpio_platform_data mdio_gpio_data = {
	.mdc = GPF(13),
	.mdio = GPF(14),
	.phy_mask = 0,
	.irqs = { 0 },
};

struct platform_device jz4775_mii_bus = {
	.name = "mdio-gpio",
	.dev.platform_data = &mdio_gpio_data,
};
#endif /* CONFIG_MDIO_GPIO */
struct platform_device jz4775_mac_device = {
	.name = "jz4775_mac",
	.dev.platform_data = &jz4775_mii_bus,
};
#endif /* CONFIG_JZ4775_MAC */

#ifdef CONFIG_JZ4785_SUPPORT_TSC
static struct jztsc_pin fpga_tsc_gpio[] = {
	[0] = {GPIO_TP_INT, LOW_ENABLE},
	[1] = {GPIO_TP_WAKE, HIGH_ENABLE},
};

static struct jztsc_platform_data fpga_tsc_pdata = {
	.gpio = fpga_tsc_gpio,
	.x_max = 800,
	.y_max = 480,
};

#ifdef CONFIG_TOUCHSCREEN_GWTC9XXXB
static struct i2c_board_info fpga_i2c0_devs[] __initdata = {
	{
	 I2C_BOARD_INFO("gwtc9xxxb_ts", 0x05),
	 .platform_data = &fpga_tsc_pdata,
	 },
};
#endif
#endif

#if (defined(CONFIG_I2C_GPIO_V1_2) || defined(CONFIG_I2C1_JZ_V1_2))

#ifdef CONFIG_WM8594_CODEC_V1_2
static struct snd_codec_data wm8594_codec_pdata = {
	    .codec_sys_clk = 12000000,
};
#endif

static struct i2c_board_info f4785_i2c1_devs[] __initdata = {
#ifdef CONFIG_WM8594_CODEC_V1_2
	{
		I2C_BOARD_INFO("wm8594", 0x1a),
		.platform_data = &wm8594_codec_pdata,
	},
#endif
};
#endif  /*I2C1*/

static int __init board_init(void)
{
/*vpu*/
#ifdef CONFIG_JZ_VPU_V1_2
	platform_device_register(&jz_vpu_device);
#endif

/*i2c*/
#ifdef CONFIG_I2C0_JZ_V1_2
	platform_device_register(&jz_i2c0_device);
#endif

#ifdef CONFIG_I2C1_JZ_V1_2
	platform_device_register(&jz_i2c1_device);
#endif

#ifdef CONFIG_I2C2_JZ_V1_2
	platform_device_register(&jz_i2c2_device);
#endif

#ifdef CONFIG_I2C3_JZ_V1_2
	platform_device_register(&jz_i2c3_device);
#endif

#if (defined(CONFIG_I2C_GPIO_V1_2) || defined(CONFIG_I2C1_JZ_V1_2) || defined(CONFIG_I2C1_DMA_V1_2))
	i2c_register_board_info(1, f4785_i2c1_devs, ARRAY_SIZE(f4785_i2c1_devs));
#endif

/*dma*/
#ifdef CONFIG_XBURST_DMAC
	platform_device_register(&jz_pdma_device);
#endif
/* panel and bl */
#ifdef CONFIG_LCD_KD50G2_40NM_A2
	platform_device_register(&kd50g2_40nm_a2_device);
#endif
#ifdef CONFIG_LCD_BYD_BM8766U
	platform_device_register(&byd_bm8766u_device);
#endif
#ifdef CONFIG_LCD_KFM701A21_1A
	platform_device_register(&kfm701a21_1a_device);
#endif
#ifdef CONFIG_LCD_TRULY_TFT240240_2_E
	platform_device_register(&truly_tft240240_device);
#endif
#ifdef CONFIG_LCD_CV90_M5377_P30
	platform_device_register(&cv90_m5377_p30_device);
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	platform_device_register(&backlight_device);
#endif
#ifdef CONFIG_BACKLIGHT_DIGITAL_PULSE
	platform_device_register(&digital_pulse_backlight_device);
#endif
/* lcdc framebuffer*/
#ifdef CONFIG_FB_JZ_V12
	jz_device_register(&jz_fb_device, &jzfb_pdata);
#endif

/*mipi-dsi */
#ifdef CONFIG_JZ_MIPI_DSI
	jz_device_register(&jz_dsi_device, &jzdsi_pdata);
#endif

/*ipu*/
#if defined CONFIG_JZ_IPU_V1_2
	platform_device_register(&jz_ipu_device);
#endif

/*touchscreen*/
#ifdef CONFIG_JZ4785_SUPPORT_TSC
	i2c_register_board_info(0, fpga_i2c0_devs, ARRAY_SIZE(fpga_i2c0_devs));
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
#ifdef CONFIG_USB_OHCI_HCD
	platform_device_register(&jz_ohci_device);
#endif
#ifdef CONFIG_USB_EHCI_HCD
	platform_device_register(&jz_ehci_device);
#endif
#ifdef CONFIG_USB_JZ_DWC2
	platform_device_register(&jz_dwc_otg_device);
#endif
/* msc */
#ifndef CONFIG_NAND
#ifdef CONFIG_JZMMC_V12_MMC0
	jz_device_register(&jz_msc0_device, &inand_pdata);
#endif
#ifdef CONFIG_JZMMC_V12_MMC1
	jz_device_register(&jz_msc1_device, &tf_pdata);
#endif
#ifdef CONFIG_JZMMC_V12_MMC2
	jz_device_register(&jz_msc2_device, &tf_pdata);
#endif
#else
#ifdef CONFIG_JZMMC_V12_MMC0
	jz_device_register(&jz_msc0_device, &tf_pdata);
#endif
#ifdef CONFIG_JZMMC_V12_MMC1
	jz_device_register(&jz_msc1_device, &sdio_pdata);
#endif
#endif
/* ethnet */
#ifdef CONFIG_JZ4775_MAC
	platform_device_register(&jz4775_mii_bus);
	platform_device_register(&jz4775_mac_device);
#endif

/* audio */
#ifdef CONFIG_SOUND_JZ_I2S_V1_2
	jz_device_register(&jz_i2s_device,&i2s_data);
	jz_device_register(&jz_mixer0_device,&snd_mixer0_data);
#endif
#ifdef CONFIG_SOUND_JZ_SPDIF_V1_2
	jz_device_register(&jz_spdif_device,&spdif_data);
	jz_device_register(&jz_mixer2_device,&snd_mixer2_data);
#endif
#ifdef CONFIG_SOUND_JZ_DMIC_V1_2
	jz_device_register(&jz_dmic_device,&dmic_data);
	jz_device_register(&jz_mixer3_device,&snd_mixer3_data);
#endif

#ifdef CONFIG_SOUND_JZ_PCM_V1_2
	jz_device_register(&jz_pcm_device,&pcm_data);
	jz_device_register(&jz_mixer1_device,&snd_mixer1_data);
#endif
#ifdef CONFIG_JZ_INTERNAL_CODEC_V1_2
	jz_device_register(&jz_codec_device, &codec_data);
#endif
	/* ovisp */
#ifdef CONFIG_VIDEO_OVISP
       jz_device_register(&ovisp_device_camera, &ovisp_camera_info);
#endif
	return 0;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return CONFIG_BOARD_NAME;
}

arch_initcall(board_init);
