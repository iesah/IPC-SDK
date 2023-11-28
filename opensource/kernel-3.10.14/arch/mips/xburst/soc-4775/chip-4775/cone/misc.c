#include <linux/platform_device.h>
#include <linux/interrupt.h>
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

#ifdef CONFIG_TOUCHSCREEN_FT6X06
#include <linux/i2c/ft6x06_ts.h>
#endif

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

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C0_JZ4775))
static struct i2c_board_info cone_i2c0_devs[] __initdata = {
#if 0
		        {
				I2C_BOARD_INFO("gwtc9xxxb_ts", 0x05),
				.platform_data = &cone_tsc_pdata,
			},
#endif
	};
#endif

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C1_JZ4775))
static struct i2c_board_info cone_i2c1_devs[] __initdata = {
};
#endif	/*I2C1*/

#ifdef CONFIG_TOUCHSCREEN_FT6X06
static struct ft6x06_platform_data ft6x06_tsc_pdata = {
	.x_max = 320,
	.y_max = 480,
	.irqflags = IRQF_TRIGGER_FALLING,
        .irq = GPIO_PC(20),
	.reset = GPIO_PB(20),
};
#endif

#if (defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C2_JZ4775))
static struct i2c_board_info cone_i2c2_devs[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_GT818X_868_968M
	{ I2C_BOARD_INFO("Goodix-TS", 0x5d), },
#endif
#ifdef CONFIG_TOUCHSCREEN_FT6X06
	{
		I2C_BOARD_INFO(FT6X06_NAME, 0x38),
		.platform_data = &ft6x06_tsc_pdata,
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
static struct jz_battery_info cone_battery_pdata = {
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
#ifdef CONFIG_I2C_GPIO

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

	i2c_register_board_info(0, cone_i2c0_devs, ARRAY_SIZE(cone_i2c0_devs));
	i2c_register_board_info(1, cone_i2c1_devs, ARRAY_SIZE(cone_i2c1_devs));
	i2c_register_board_info(2, cone_i2c2_devs, ARRAY_SIZE(cone_i2c2_devs));

	/* wifi */
	jz_device_register(&jz_msc1_device, &sdio_pdata);

/* sound */
#ifdef CONFIG_SOUND_I2S_JZ47XX
	jz_device_register(&jz_i2s_device,&i2s_data);
	jz_device_register(&jz_mixer0_device,&snd_mixer0_data);
#endif
#ifdef CONFIG_JZ_INTERNAL_CODEC
	jz_device_register(&jz_codec_device, &codec_data);
#endif

#ifdef CONFIG_LCD_KD301_M03545_0317A
	platform_device_register(&kd301_device);
#endif

#ifdef CONFIG_BACKLIGHT_PWM
	platform_device_register(&backlight_device);
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

/* x2d */
#ifdef CONFIG_JZ_X2D
        platform_device_register(&jz_x2d_device);
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

#ifdef CONFIG_RTC_DRV_JZ4775
	platform_device_register(&jz_rtc_device);
#endif

#ifdef CONFIG_ANDROID_PMEM
	platform_device_register(&pmem_adsp_device);
#endif

#ifdef CONFIG_USB_DWC2
        platform_device_register(&jz_dwc_otg_device);
#endif

/* ADC*/
#ifdef CONFIG_BATTERY_JZ4775
	adc_platform_data.battery_info = cone_battery_pdata;
	jz_device_register(&jz_adc_device, &adc_platform_data);
#endif

/*IW8103_bcm4330*/
#ifdef CONFIG_BCM4330_RFKILL
		platform_device_register(&bcm4330_bt_power_device);
#endif
	return 0;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return "cone";
}

arch_initcall(board_init);
