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
#include <linux/i2c/ft6x06_ts.h>
#include <linux/interrupt.h>
#include <sound/jz-aic.h>
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
#ifdef CONFIG_TOUCHSCREEN_GWTC9XXXB
static struct jztsc_pin mensa_tsc_gpio[] = {
	        [0] = {GPIO_TP_INT,         LOW_ENABLE},
		[1] = {GPIO_TP_WAKE,        HIGH_ENABLE},
};

static struct jztsc_platform_data mensa_tsc_pdata = {
	        .gpio           = mensa_tsc_gpio,
		.x_max          = 800,
		.y_max          = 480,
};

static struct i2c_board_info mensa_i2c0_devs[] __initdata = {
		        {
				I2C_BOARD_INFO("gwtc9xxxb_ts", 0x05),
				.platform_data = &mensa_tsc_pdata,
			},
	};
#endif

#ifdef CONFIG_TOUCHSCREEN_FT6X06
static struct ft6x06_platform_data mensa_tsc_pdata = {
		.x_max          = 300,
		.y_max          = 540,
		.va_x_max		= 300,
		.va_y_max		= 480,
		.irqflags = IRQF_TRIGGER_FALLING|IRQF_DISABLED,
		.irq = (32 * 1 + 29),
		.reset = (32 *1 + 28),
};

static struct i2c_board_info mensa_i2c0_devs[] __initdata = {
		        {
				I2C_BOARD_INFO("ft6x06_ts", 0x38),
				.platform_data = &mensa_tsc_pdata,
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
	unsigned char pos;      /* pos = cim_num + 1, pos should be 0 while only cim */
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
#ifdef CONFIG_TVP5150
static struct cam_sensor_plat_data tvp5150_pdata = {
	.facing = 1,
	.orientation = 0,
	.mirror = 0,
	.gpio_en = GPIO_OV3640_EN,
	.gpio_rst = GPIO_OV3640_RST,
	.cap_wait_frame = 6,
};
#endif
#ifdef CONFIG_OV7675
struct ov7675_platform_data {
	int facing;
	int orientation;
	int mirror;   /*camera mirror*/
	uint16_t        gpio_rst;       /* resert  gpio */
	uint16_t        gpio_en;        /* camera enable gpio */
	int cap_wait_frame;   /* filter n frames when capture image */
};
static struct ov7675_platform_data ov7675_pdata = {
	.facing = 1,
	.orientation = 0,
	.mirror = 0,
	.gpio_en = GPIO_OV7675_EN,
	.gpio_rst = GPIO_OV7675_RST,
	.cap_wait_frame = 3,
};
#endif
#ifdef CONFIG_OV2650
struct ov2650_platform_data {
	int facing;
	int orientation;
	int mirror;   /*camera mirror*/
	uint16_t        gpio_rst;       /* resert  gpio */
	uint16_t        gpio_en;        /* camera enable gpio */
	int cap_wait_frame;   /* filter n frames when capture image */
};

static struct ov2650_platform_data ov2650_pdata = {
	.facing = 0,
	.orientation = 0,
	.mirror = 0,
	.gpio_en = GPIO_OV2650_EN,
	.gpio_rst = GPIO_OV2650_RST,
	.cap_wait_frame = 3,
};
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

#endif


#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C1_JZ4775))
#ifdef CONFIG_JZ_CIM
static struct i2c_board_info mensa_i2c1_devs[] __initdata = {
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
#ifdef CONFIG_TVP5150
	{
		I2C_BOARD_INFO("tvp5150", 0x5d),
		.platform_data	= &tvp5150_pdata,
	},
#endif
#ifdef CONFIG_OV7675
	{
		I2C_BOARD_INFO("ov7675", 0x21),
		.platform_data  = &ov7675_pdata,
	},
#endif
#ifdef CONFIG_OV2650
	{
		I2C_BOARD_INFO("ov2650", 0x30),
		.platform_data  = &ov2650_pdata,
	},
#endif
};
#elif defined(CONFIG_VIDEO_JZ4780_CIM_HOST)
struct i2c_board_info mensa_i2c1_devs_v4l2[2] = {

	[FRONT_CAMERA_INDEX] = {
#ifdef CONFIG_SOC_CAMERA_OV5640_FRONT
		I2C_BOARD_INFO("ov5640-front", 0x3c),
#endif
	},

	[BACK_CAMERA_INDEX] = {
#ifdef CONFIG_SOC_CAMERA_OV5640_BACK
		I2C_BOARD_INFO("ov5640-back", 0x3c + 1),
#endif
	},
};
#endif

#endif	/*I2C1*/

#if (defined(CONFIG_USB_DWC2) || defined(CONFIG_USB_DWC_OTG)) && defined(GPIO_USB_DETE)
struct jzdwc_pin dete_pin = {
        .num                            = GPIO_USB_DETE,
        .enable_level                   = HIGH_ENABLE,
};

struct jzdwc_pin __attribute__((weak)) dwc2_id_pin = {
	.num          = GPIO_PB(5),
};
#endif

/* Battery Info */
#ifdef CONFIG_BATTERY_JZ4775
static struct jz_battery_info mensa_battery_pdata = {
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

#ifdef CONFIG_SND_JZ_SOC_AIC_CORE
/* For board info */
#define JZ_AIC_AC97             \
{                               \
	.id     = -1,           \
	.name   = "jz-aic-ac97",\
	.platform_data  = NULL, \
}

#define JZ_AIC_I2S              \
{                               \
	.id     = -1,           \
	.name   = "jz-aic-i2s",\
	.platform_data  = NULL, \
}

#define JZ_AIC_SPDIF            \
{                               \
	.id     = -1,           \
	.name   = "jz-aic-spdif",\
	.platform_data  = NULL, \
}

#define JZ_AIC_DMA		\
{				\
	.id	= -1,		\
	.name	= "jz-audio-dma",\
	.platform_data	= NULL, \
}

struct jz_aic_subdev_info jz_aic_subdev_info[] = {
	JZ_AIC_I2S,
	JZ_AIC_AC97,
	JZ_AIC_SPDIF,
	JZ_AIC_DMA,
};

struct jz_aic_platform_data aic_platform_data = {

	.num_subdevs    = ARRAY_SIZE(jz_aic_subdev_info),
	.subdevs        = jz_aic_subdev_info,
};
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
	.pin	=	7,
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

/* SPI NOR */
#ifdef CONFIG_JZ_SPI_NOR
struct spi_nor_block_info {
        u32 blocksize;
        u8 cmd_blockerase;
        /* MAX Busytime for block erase, unit: ms */
        u32 be_maxbusy;
};

struct spi_nor_platform_data {
        u32 pagesize;
        u32 sectorsize;
        u32 chipsize;

        /* Some NOR flash has different blocksize and block erase command,
         * One command with One blocksize. */
        struct spi_nor_block_info *block_info;
        int num_block_info;

        /* Flash Address size, unit: Bytes */
        int addrsize;

        /* MAX Busytime for page program, unit: ms */
        u32 pp_maxbusy;
        /* MAX Busytime for sector erase, unit: ms */
        u32 se_maxbusy;
        /* MAX Busytime for chip erase, unit: ms */
        u32 ce_maxbusy;

        /* Flash status register num, Max support 3 register */
        int st_regnum;
};

static struct spi_nor_block_info flash_block_info[] = {
        {
                .blocksize      = 64 * 1024,
                .cmd_blockerase = 0xD8,
                .be_maxbusy     = 1200  /* 1.2s */
        },

        {
                .blocksize      = 32 * 1024,
                .cmd_blockerase = 0x52,
                .be_maxbusy     = 1000  /* 1s */
        },
};

static struct spi_nor_platform_data spi_nor_pdata = {
        .pagesize       = 256,
        .sectorsize     = 4 * 1024,
        .chipsize       = 16384 * 1024,

        .block_info     = flash_block_info,
        .num_block_info = ARRAY_SIZE(flash_block_info),

        .addrsize       = 3,
        .pp_maxbusy     = 3,            /* 3ms */
        .se_maxbusy     = 400,          /* 400ms */
        .ce_maxbusy     = 80 * 1000,    /* 80s */

        .st_regnum      = 3,
};
#endif

/* SSI */
#ifdef CONFIG_SPI_JZ4780
#ifdef CONFIG_SPI0_JZ4780
static struct jz47xx_spi_info spi0_info_cfg = {
        .chnl           = 0,
        .bus_num        = 0,
        .max_clk        = 54000000,
        .num_chipselect = 1,
        .chipselect     = {GPIO_PA(23)},
};
#endif
#endif

/* SPI By GPIO */
#ifdef CONFIG_SPI_GPIO
static struct spi_gpio_platform_data jz4780_spi_gpio_data = {
        .sck            = GPIO_PD(24),
        .mosi           = GPIO_PD(21),
        .miso           = GPIO_PD(20),
        .num_chipselect = 2,
};

static struct platform_device jz4780_spi_gpio_device = {
        .name   = "spi_gpio",
        .id     = 0,
        .dev    = {
                .platform_data = &jz4780_spi_gpio_data,
        },
};
#endif /* CONFIG_SPI_GPIO */

/* SPI Devices */
#if defined(CONFIG_SPI_JZ4780) || defined(CONFIG_SPI_GPIO)
static struct spi_board_info jz_spi_board_info[] = {
#ifdef CONFIG_JZ_SPI_NOR
        /* SPI NOR */
        [0] = {
                .modalias               = "jz_nor",
                .platform_data          = &spi_nor_pdata,
                .controller_data        = (void *)GPIO_PD(23), /* cs for spi gpio */
                .max_speed_hz           = 12000000,
                .bus_num                = 0,
                .chip_select            = 0,
                .mode                   = SPI_MODE_3,
        },
#endif
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
#ifdef CONFIG_JZ_CIM
	i2c_register_board_info(1, mensa_i2c1_devs, ARRAY_SIZE(mensa_i2c1_devs));
#endif
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

#ifdef CONFIG_SND_SOC_JZ4775_ICDC
	platform_device_register(&jz4775_codec_device);
#endif

#ifdef CONFIG_SND_JZ47XX_SOC_I2S
	platform_device_register(&jz47xx_i2s_device);
#endif

#ifdef CONFIG_SND_JZ47XX_SOC
	platform_device_register(&jz47xx_pcm_device);
#endif


#ifdef CONFIG_LCD_KD50G2_40NM_A2
 	platform_device_register(&kd50g2_40nm_a2_device);
#endif
/* panel and bl */
#ifdef CONFIG_LCD_BYD_BM8766U
	platform_device_register(&byd_bm8766u_device);
#endif
#ifdef CONFIG_BM347WV_F_8991FTGF_HX8369
	platform_device_register(&byd_8991_device);
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
	i2c_register_board_info(0, mensa_i2c0_devs, ARRAY_SIZE(mensa_i2c0_devs));
#endif
#ifdef CONFIG_RTC_DRV_JZ4775
	platform_device_register(&jz_rtc_device);
#endif

/* SSI */
#ifdef CONFIG_SPI_JZ4780
       spi_register_board_info(jz_spi_board_info, ARRAY_SIZE(jz_spi_board_info));
#ifdef CONFIG_SPI0_JZ4780
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
       spi_register_board_info(jz_spi_board_info, ARRAY_SIZE(jz_spi_board_info));
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
	adc_platform_data.battery_info = mensa_battery_pdata;
	jz_device_register(&jz_adc_device, &adc_platform_data);
#endif

#ifdef CONFIG_SND_JZ_SOC_AIC_CORE
	jz_device_register(&jz_aic_device, &aic_platform_data);
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
#ifdef CONFIG_NAND
    return "mensa_nand";
#else
    return "mensa_mmc";
#endif
}

arch_initcall(board_init);
