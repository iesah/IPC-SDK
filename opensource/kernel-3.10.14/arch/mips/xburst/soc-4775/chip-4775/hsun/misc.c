#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/tsc.h>
#include <linux/jz4780-adc.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/android_pmem.h>
#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <mach/jz4780_efuse.h>
#include <gpio.h>
#include <linux/jz_dwc.h>
#include <linux/power/jz4780-battery.h>

#include "board.h"

#ifdef CONFIG_KEYBOARD_GPIO
static struct gpio_keys_button board_buttons[] = {
#ifdef GPIO_HOME
	{
		.gpio		= GPIO_HOME,
		.code   	= KEY_HOME,
		.desc		= "home key",
		.active_low	= ACTIVE_LOW_HOME,
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
#ifdef GPIO_BACK
	{
		.gpio		= GPIO_BACK,
		.code   	= KEY_BACK,
		.desc		= "back key",
		.active_low	= ACTIVE_LOW_BACK,
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

#ifdef GPIO_ENDCALL
	{
		.gpio           = GPIO_ENDCALL,
		.code           = KEY_POWER,
		.desc           = "end call key",
		.active_low     = ACTIVE_LOW_ENDCALL,
		.wakeup         = 1,
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

#ifdef CONFIG_JZ4775_SUPPORT_TSC
static struct jztsc_pin hsun_tsc_gpio[] = {
	        [0] = {GPIO_TP_INT,         LOW_ENABLE},
		[1] = {GPIO_TP_WAKE,        HIGH_ENABLE},
};

static struct jztsc_platform_data hsun_tsc_pdata = {
	        .gpio           = hsun_tsc_gpio,
		.x_max          = 800,
		.y_max          = 480,
};

#ifdef CONFIG_TOUCHSCREEN_GWTC9XXXB
static struct i2c_board_info hsun_i2c0_devs[] __initdata = {
		        {
				I2C_BOARD_INFO("gwtc9xxxb_ts", 0x05),
				.platform_data = &hsun_tsc_pdata,
			},
	};
#endif
#endif

#if ((defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C1_JZ4775)) && (defined(CONFIG_JZ_CIM0) || defined(CONFIG_JZ_CIM1)))
struct cam_sensor_plat_data {
	int facing;
	int orientation;
	int mirror;   //camera mirror
	//u16	gpio_vcc;	/* vcc enable gpio */   remove the gpio_vcc   , DO NOT use this pin for sensor power up ,cim will controls this
	uint16_t	gpio_rst;	/* resert  gpio */
	uint16_t	gpio_en;	/* camera enable gpio */
	int cap_wait_frame;    /* filter n frames when capture image */
};

#ifdef CONFIG_OV3640
static struct cam_sensor_plat_data ov3640_pdata = {
	.facing = 1,
	.orientation = 0,
	.mirror = 0,
	.gpio_en = GPIO_OV3640_EN,
	.gpio_rst = GPIO_OV3640_RST,
	.cap_wait_frame = 6,
};
#endif
#endif

#ifdef CONFIG_OV5640
struct ov5640_platform_data {
	int facing;
	int orientation;
	int mirror;   //camera mirror
	//u16	gpio_vcc;	/* vcc enable gpio */   remove the gpio_vcc   , DO NOT use this pin for sensor power up ,cim will controls this
	uint16_t	gpio_rst;	/* resert  gpio */
	uint16_t	gpio_en;	/* sensor enable gpio */
	uint16_t	gpio_pwdn;	/* sensor powerdown gpio */
	int cap_wait_frame;   /* filter n frames when capture image */
};
static struct ov5640_platform_data ov5640_pdata = {
	.facing = 0,
	.orientation = 270,
	.mirror = 0,
	.gpio_en = -1,
	.gpio_rst = GPIO_OV5640_RST,	// active low
	.gpio_pwdn = GPIO_OV5640_PWDN,
	.cap_wait_frame = 3,
};
#endif

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C1_JZ4775))
static struct i2c_board_info hsun_i2c1_devs[] __initdata = {
#ifdef CONFIG_OV3640
	{
		I2C_BOARD_INFO("ov3640", 0x3c),
		.platform_data	= &ov3640_pdata,
	},
#endif
#ifdef CONFIG_OV5640
	{
		I2C_BOARD_INFO("ov5640", 0x3c),
		.platform_data	= &ov5640_pdata,
	},
#endif
};
#endif	/*I2C1*/

#if (defined(CONFIG_USB_DWC2) || defined(CONFIG_USB_DWC_OTG)) && defined(GPIO_USB_DETE)
struct jzdwc_pin dete_pin = {
        .num                            = GPIO_USB_DETE,
        .enable_level                   = HIGH_ENABLE,
};
#endif

/* Battery Info */
#ifdef CONFIG_BATTERY_JZ4775
static struct jz_battery_info hsun_battery_pdata = {
	.max_vol        = 4070,
	.min_vol        = 3650,
	.usb_max_vol    = 4140,
	.usb_min_vol    = 3700,
	.ac_max_vol     = 4170,
	.ac_min_vol     = 3780,
	.battery_max_cpt = 2300,
	.ac_chg_current = 800,
	.usb_chg_current = 400,
        .sleep_current = 30,
};
static struct jz_adc_platform_data adc_platform_data;
#endif

#if 0
static struct resource	jz_mac_res[] = {
	{ .flags = IORESOURCE_MEM,
		.start = ETHC_IOBASE,
		.end = ETHC_IOBASE + 0xfff,
	},
#if 0
	{ .flags = IORESOURCE_IRQ,
		.start = IRQ_ETHC,
	},
#endif
};

struct platform_device jz_mac = {
	.name = "jz_mac",
	.id = 0,
	.num_resources = ARRAY_SIZE(jz_mac_res),
	.resource = jz_mac_res,
	.dev = {
		.platform_data = NULL,
	},
};
#else
#if defined(CONFIG_JZ4775_MAC)
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
static struct jz_gpio_phy_reset gpio_phy_reset = {
	.port	=	GPIO_PORT_B,
	.pin	=	28,
	.start_func	=	GPIO_OUTPUT0,
	.end_func	=	GPIO_OUTPUT1,
	.delaytime_usec	=	100000,
};
#endif
struct platform_device jz4775_mii_bus = {
        .name = "jz4775_mii_bus",
#ifdef CONFIG_JZGPIO_PHY_RESET
	.dev.platform_data = &gpio_phy_reset,
#endif
};
#else
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
#endif

struct platform_device jz4775_mac_device = {
        .name = "jz4775_mac",
        .dev.platform_data = &jz4775_mii_bus,
};
#endif
#endif

#ifdef CONFIG_ANDROID_PMEM
/* arch/mips/kernel/setup.c */
extern unsigned long set_reserved_pmem_total_size(unsigned long size);
void board_pmem_setup(void)
{
	/* reserve memory for pmem. */
	unsigned long pmem_total_size=0;
#if defined(JZ_PMEM_ADSP_SIZE) && (JZ_PMEM_ADSP_SIZE>0)
	pmem_total_size += JZ_PMEM_ADSP_SIZE;
#endif
#if defined(JZ_PMEM_CAMERA_SIZE) && (JZ_PMEM_CAMERA_SIZE>0)
	pmem_total_size += JZ_PMEM_CAMERA_SIZE;
#endif
	set_reserved_pmem_total_size(pmem_total_size);
}

static struct android_pmem_platform_data pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.no_allocator = 0,
	.cached = 1,
	.start = JZ_PMEM_ADSP_BASE,
	.size = JZ_PMEM_ADSP_SIZE,
};

static struct platform_device pmem_adsp_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_adsp_pdata },
};
#endif

/* efuse */
#ifdef CONFIG_JZ4775_EFUSE
static struct jz4780_efuse_platform_data jz_efuse_pdata = {
       /* supply 2.5V to VDDQ */
       .gpio_vddq_en_n = -ENODEV,
};
#endif

/*define gpio i2c,if you use gpio i2c,please enable gpio i2c and disable i2c controller*/
#ifdef CONFIG_I2C_GPIO /*CONFIG_I2C_GPIO*/

#define DEF_GPIO_I2C(NO,GPIO_I2C_SDA,GPIO_I2C_SCK)		\
static struct i2c_gpio_platform_data i2c##NO##_gpio_data = {	\
	.sda_pin	= GPIO_I2C_SDA,				\
	.scl_pin	= GPIO_I2C_SCK,				\
};								\
static struct platform_device i2c##NO##_gpio_device = {     	\
	.name	= "i2c-gpio",					\
	.id	= NO,						\
	.dev	= { .platform_data = &i2c##NO##_gpio_data,},	\
};


#if (!defined(CONFIG_I2C0_JZ4775) && !defined(CONFIG_I2C0_DMA_JZ4775))
DEF_GPIO_I2C(0,GPIO_PD(30),GPIO_PD(31));
#endif
#if (!defined(CONFIG_I2C1_JZ4775) && !defined(CONFIG_I2C1_DMA_JZ4775))
DEF_GPIO_I2C(1,GPIO_PE(30),GPIO_PE(31));
#endif
#if (!defined(CONFIG_I2C2_JZ4775) && !defined(CONFIG_I2C2_DMA_JZ4775))
DEF_GPIO_I2C(2,GPIO_PE(0),GPIO_PE(3));
#endif
#endif

static int __init board_init(void)
{
/* dma */
#ifdef CONFIG_XBURST_DMAC
	platform_device_register(&jz_pdma_device);
#endif
/* i2c */
#ifdef CONFIG_I2C0_JZ4775
	platform_device_register(&jz_i2c0_device);
#endif
#ifdef CONFIG_I2C1_JZ4775
	platform_device_register(&jz_i2c1_device);
#endif
#ifdef CONFIG_I2C2_JZ4775
	platform_device_register(&jz_i2c2_device);
#endif

#ifdef CONFIG_I2C0_DMA_JZ4775
	platform_device_register(&jz_i2c0_dma_device);
#endif
#ifdef CONFIG_I2C1_DMA_JZ4775
	platform_device_register(&jz_i2c1_dma_device);
#endif
#ifdef CONFIG_I2C2_DMA_JZ4775
	platform_device_register(&jz_i2c2_dma_device);
#endif

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C1_JZ4775) || defined(CONFIG_I2C1_DMA_JZ4775))
	i2c_register_board_info(1, hsun_i2c1_devs, ARRAY_SIZE(hsun_i2c1_devs));
#endif

#ifdef CONFIG_I2C_GPIO

#if (!defined(CONFIG_I2C0_JZ4775) && !defined(CONFIG_I2C0_DMA_JZ4775))
	platform_device_register(&i2c0_gpio_device);
#endif
#if (!defined(CONFIG_I2C1_JZ4775) && !defined(CONFIG_I2C1_DMA_JZ4775))
	platform_device_register(&i2c1_gpio_device);
#endif
#if (!defined(CONFIG_I2C2_JZ4775) && !defined(CONFIG_I2C2_DMA_JZ4775))
	platform_device_register(&i2c2_gpio_device);
#endif

#endif

/* mmc */
#ifndef CONFIG_NAND
#ifdef CONFIG_MMC0_JZ4775
	jz_device_register(&jz_msc0_device, &inand_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4775
	jz_device_register(&jz_msc1_device, &sdio_pdata);
#endif
#ifdef CONFIG_MMC2_JZ4775
	jz_device_register(&jz_msc2_device, &tf_pdata);
#endif
#else
#ifdef CONFIG_MMC0_JZ4775
	jz_device_register(&jz_msc0_device, &tf_pdata);
#endif
#ifdef CONFIG_MMC1_JZ4775
	jz_device_register(&jz_msc1_device, &sdio_pdata);
#endif
#endif


/* sound */
#ifdef CONFIG_SOUND_I2S_JZ47XX
	jz_device_register(&jz_i2s_device,&i2s_data);
	jz_device_register(&jz_mixer0_device,&snd_mixer0_data);
#endif
#ifdef CONFIG_SOUND_PCM_JZ47XX
	jz_device_register(&jz_pcm_device,&pcm_data);
#endif
#ifdef CONFIG_JZ_INTERNAL_CODEC
	jz_device_register(&jz_codec_device, &codec_data);
#endif


#ifdef CONFIG_LCD_KD50G2_40NM_A2
 	platform_device_register(&kd50g2_40nm_a2_device);
#endif
/* panel and bl */
#ifdef CONFIG_LCD_BYD_BM8766U
	platform_device_register(&byd_bm8766u_device);
#endif
#ifdef CONFIG_LCD_KFM701A21_1A
	platform_device_register(&kfm701a21_1a_device);
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	platform_device_register(&backlight_device);
#endif
#ifdef CONFIG_BACKLIGHT_DIGITAL_PULSE
	platform_device_register(&digital_pulse_backlight_device);
#endif

#ifdef CONFIG_FB_JZ4775_ANDROID_EPD
	platform_device_register(&jz_epdce_device);
	jz_device_register(&jz_epd_device, &jz_epd_pdata);
#endif
/* lcdc framebuffer*/
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

#ifdef CONFIG_JZ_CIM0
	platform_device_register(&jz_cim0_device);
#endif
#ifdef CONFIG_JZ_CIM1
	platform_device_register(&jz_cim1_device);
#endif

/* x2d */
#ifdef CONFIG_JZ_X2D
        platform_device_register(&jz_x2d_device);
#endif

#ifdef CONFIG_USB_OHCI_HCD
	platform_device_register(&jz_ohci_device);
#endif

#ifdef CONFIG_USB_EHCI_HCD
	platform_device_register(&jz_ehci_device);
#endif

/* ethnet */
#ifdef CONFIG_JZ4775_MAC
	platform_device_register(&jz4775_mii_bus);
	platform_device_register(&jz4775_mac_device);

#endif


#ifdef CONFIG_JZ_VPU
	platform_device_register(&jz_vpu_device);
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

#ifdef CONFIG_JZ4775_SUPPORT_TSC
	i2c_register_board_info(0, hsun_i2c0_devs, ARRAY_SIZE(hsun_i2c0_devs));
#endif
#ifdef CONFIG_RTC_DRV_JZ4775
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

#ifdef CONFIG_ANDROID_PMEM
       platform_device_register(&pmem_adsp_device);
#endif

#ifdef CONFIG_USB_DWC2
        platform_device_register(&jz_dwc_otg_device);
#endif

/* ADC*/
#ifdef CONFIG_BATTERY_JZ4775
	adc_platform_data.battery_info = hsun_battery_pdata;
	jz_device_register(&jz_adc_device, &adc_platform_data);
#endif

/*IW8103_bcm4330*/
#ifdef CONFIG_BCM4330_RFKILL
		platform_device_register(&bcm4330_bt_power_device);
#endif
/* efuse */
#ifdef CONFIG_JZ4775_EFUSE
       jz_device_register(&jz_efuse_device, &jz_efuse_pdata);
#endif
	return 0;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return "hsun";
}

arch_initcall(board_init);
