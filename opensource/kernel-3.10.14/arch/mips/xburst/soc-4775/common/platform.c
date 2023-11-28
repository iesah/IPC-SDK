/*
 * Platform device support for Jz4780 SoC.
 *
 * Copyright 2007, <zpzhong@ingenic.cn>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/i2c-gpio.h>

#include <gpio.h>

#include <soc/gpio.h>
#include <soc/base.h>
#include <soc/irq.h>

#include <mach/platform.h>
#include <mach/jzdma.h>
#include <mach/jzsnd.h>

/* device IO define array */
struct jz_gpio_func_def platform_devio_array[] = {
#ifdef CONFIG_JZMMC_V11_MMC0_PA_4BIT
	MSC0_PORTA_4BIT,
#endif
#ifdef CONFIG_JZMMC_V11_MMC0_PA_8BIT
	MSC0_PORTA_8BIT,
#endif
#ifdef CONFIG_JZMMC_V11_MMC0_PE_4BIT
	MSC0_PORTE,
#endif
#ifdef CONFIG_JZMMC_V11_MMC1_PD_4BIT
	MSC1_PORTD,
#endif
#ifdef CONFIG_JZMMC_V11_MMC1_PE_4BIT
	MSC1_PORTE,
#endif
#ifdef CONFIG_JZMMC_V11_MMC2_PB_4BIT
	MSC2_PORTB,
#endif
#ifdef CONFIG_JZMMC_V11_MMC2_PE_4BIT
	MSC2_PORTE,
#endif

#ifdef CONFIG_I2C0_V12_JZ
	I2C0_PORTD,
#endif
#ifdef CONFIG_I2C1_V12_JZ
	I2C1_PORTE,
#endif
#ifdef CONFIG_I2C2_V12_JZ
	I2C2_PORTE,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART0
	UART0_PORTF,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	UART1_PORTD,
#endif

#ifndef CONFIG_JZ_IRDA_V11
#ifdef CONFIG_SERIAL_JZ47XX_UART2
	//UART2_PORTC,
	UART2_PORTF,
#endif
#else
	UART2_PORTF,
#endif//irda
#ifdef CONFIG_SERIAL_JZ47XX_UART3
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART4
#endif
#ifdef CONFIG_NAND_COMMON
	NAND_PORTAB_COMMON,
	NAND_PORTA_CS1,
	NAND_PORTA_CS2,
#endif
#ifdef CONFIG_NAND_BUS_WIDTH_16
	NAND_PORTA_BUS16,
#endif
#ifdef CONFIG_NAND_CS3
	NAND_PORTA_CS3,
#endif
#ifdef CONFIG_NAND_CS4
#endif
#ifdef CONFIG_NAND_CS5
#endif
#ifdef CONFIG_NAND_CS6
#endif

#ifdef CONFIG_SOUND_JZ_I2S_V12
	I2S_PORTEF,
#endif

#ifdef CONFIG_HALLEY_INTERNAL_CODEC_V11
	DMIC_PORTF,
#endif

#ifdef CONFIG_SOUND_PCM_JZ47XX
	PCM_PORTD,
#endif
#ifndef CONFIG_DISABLE_LVDS_FUNCTION
	DISABLE_LCD_PORTC,
#else
	LCD_PORTC,
#endif

#ifdef CONFIG_JZ_EPD_GPIO_FUNCTION
	EPD_PORTC,
#endif

#ifdef CONFIG_JZ_PWM_GPIO_E0
	PWM_PORTE_BIT0,
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E1
	PWM_PORTE_BIT1,
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E2
	PWM_PORTE_BIT2,
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E3
	PWM_PORTE_BIT3,
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E4
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E5
#endif
#ifdef CONFIG_JZ_PWM_GPIO_D10
#endif
#ifdef CONFIG_JZ_PWM_GPIO_D11
#endif

#ifdef CONFIG_JZ4775_MAC
	MII_PORTBDF,
#endif

#ifdef CONFIG_USB_DWC2_DRVVBUS_PIN
	OTG_DRVVUS,
#endif

#if defined(CONFIG_JZ_CIM0) || defined(CONFIG_VIDEO_JZ4780_CIM_HOST)
	CIM0_PORTB,
#endif
#if defined(CONFIG_JZ_CIM1) || defined(CONFIG_VIDEO_JZ4780_CIM_HOST)
	CIM1_PORTG,
#endif

#ifdef CONFIG_SPI0_JZ4780_PA
       SSI0_PORTA,
#endif
#ifdef CONFIG_SPI0_JZ4780_PB
#endif
#ifdef CONFIG_SPI0_JZ4780_PD
       SSI0_PORTD,
#endif
#ifdef CONFIG_SPI0_JZ4780_PE
#endif

#ifdef CONFIG_SPI1_JZ4780_PB
#endif
#ifdef CONFIG_SPI1_JZ4780_PD
#endif
#ifdef CONFIG_SPI1_JZ4780_PE
#endif

};

int platform_devio_array_size = ARRAY_SIZE(platform_devio_array);

int jz_device_register(struct platform_device *pdev,void *pdata)
{
	pdev->dev.platform_data = pdata;

	return platform_device_register(pdev);
}

static struct resource jz_pdma_res[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = PDMA_IOBASE,
		.end = PDMA_IOBASE + 0x10000 - 1,
	},
	[1] = {
		.name  = "irq",
		.flags = IORESOURCE_IRQ,
		.start = IRQ_PDMA,
	},
	[2] = {
		.name  = "pdmam",
		.flags = IORESOURCE_IRQ,
		.start = IRQ_PDMAM,
	},
	[3] = {
		.name  = "mcu",
		.flags = IORESOURCE_IRQ,
		.start = IRQ_MCU,
	},
};

static struct jzdma_platform_data jzdma_pdata = {
	.irq_base = IRQ_MCU_BASE,
	.irq_end = IRQ_MCU_END,
	.map = {
#ifdef CONFIG_NAND
                JZDMA_REQ_NAND0,
                JZDMA_REQ_NAND1,
                JZDMA_REQ_NAND2,
                JZDMA_REQ_NAND3,
                JZDMA_REQ_NAND4,
#else
                JZDMA_REQ_AUTO_TXRX,
                JZDMA_REQ_AUTO_TXRX,
                JZDMA_REQ_AUTO_TXRX,
                JZDMA_REQ_AUTO_TXRX,
                JZDMA_REQ_AUTO_TXRX,
#endif
		JZDMA_REQ_I2S0,
		JZDMA_REQ_I2S0,
#ifdef CONFIG_SERIAL_JZ47XX_UART3_DMA
		JZDMA_REQ_UART3,
		JZDMA_REQ_UART3,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART2_DMA
		JZDMA_REQ_UART2,
		JZDMA_REQ_UART2,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1_DMA
		JZDMA_REQ_UART1,
		JZDMA_REQ_UART1,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART0_DMA
		JZDMA_REQ_UART0,
		JZDMA_REQ_UART0,
#endif
		JZDMA_REQ_SSI0,
		JZDMA_REQ_SSI0,
		JZDMA_REQ_SSI1,
		JZDMA_REQ_SSI1,
		JZDMA_REQ_PCM0,
		JZDMA_REQ_PCM0,
		JZDMA_REQ_PCM1,
		JZDMA_REQ_PCM1,
		JZDMA_REQ_I2C0,
		JZDMA_REQ_I2C0,
		JZDMA_REQ_I2C1,
		JZDMA_REQ_I2C1,
		JZDMA_REQ_I2C2,
		JZDMA_REQ_I2C2,
		JZDMA_REQ_DES,
	},
};

struct platform_device jz_pdma_device = {
	.name = "jz-dma",
	.id = -1,
	.dev = {
		.platform_data = &jzdma_pdata,
	},
	.resource = jz_pdma_res,
	.num_resources = ARRAY_SIZE(jz_pdma_res),
};

static u64 jz_msc_dmamask =  ~(u32)0;

#define DEF_MSC(NO)								\
	static struct resource jz_msc##NO##_resources[] = {			\
		{								\
			.start          = MSC##NO##_IOBASE,			\
			.end            = MSC##NO##_IOBASE + 0x1000 - 1,	\
			.flags          = IORESOURCE_MEM,			\
		},								\
		{								\
			.start          = IRQ_MSC##NO,				\
			.end            = IRQ_MSC##NO,				\
			.flags          = IORESOURCE_IRQ,			\
		},								\
	};									\
struct platform_device jz_msc##NO##_device = {					\
	.name = "jzmmc",							\
	.id = NO,								\
	.dev = {								\
		.dma_mask               = &jz_msc_dmamask,			\
		.coherent_dma_mask      = 0xffffffff,				\
	},									\
	.resource       = jz_msc##NO##_resources,				\
	.num_resources  = ARRAY_SIZE(jz_msc##NO##_resources),			\
};
DEF_MSC(0);
DEF_MSC(1);
DEF_MSC(2);

#if (defined(CONFIG_I2C0_V12_JZ) || defined(CONFIG_I2C1_V12_JZ) ||	\
		defined(CONFIG_I2C2_V12_JZ))
static u64 jz_i2c_dmamask =  ~(u32)0;

#define DEF_I2C(NO)								\
	static struct resource jz_i2c##NO##_resources[] = {			\
		[0] = {								\
			.start          = I2C##NO##_IOBASE,			\
			.end            = I2C##NO##_IOBASE + 0x1000 - 1,	\
			.flags          = IORESOURCE_MEM,			\
		},								\
		[1] = {								\
			.start          = IRQ_I2C##NO,				\
			.end            = IRQ_I2C##NO,				\
			.flags          = IORESOURCE_IRQ,			\
		},								\
		[2] = {								\
			.start          = JZDMA_REQ_I2C##NO,			\
			.flags          = IORESOURCE_DMA,			\
		},								\
		[3] = {								\
			.start          = CONFIG_I2C##NO##_SPEED,			\
			.flags          = IORESOURCE_BUS, \
		},								\
	};									\
struct platform_device jz_i2c##NO##_device = {					\
	.name = "jz-i2c",							\
	.id = NO,								\
	.dev = {								\
		.dma_mask               = &jz_i2c_dmamask,			\
		.coherent_dma_mask      = 0xffffffff,				\
	},									\
	.num_resources  = ARRAY_SIZE(jz_i2c##NO##_resources),			\
	.resource       = jz_i2c##NO##_resources,				\
};
#ifdef CONFIG_I2C0_V12_JZ
DEF_I2C(0);
#endif
#ifdef CONFIG_I2C1_V12_JZ
DEF_I2C(1);
#endif
#ifdef CONFIG_I2C2_V12_JZ
DEF_I2C(2);
#endif
#endif

#if (defined(CONFIG_I2C0_DMA_JZ4775) || defined(CONFIG_I2C1_DMA_JZ4775) ||	\
		defined(CONFIG_I2C2_DMA_JZ4775))
static u64 jz_i2c_dmamask =  ~(u32)0;

#define DEF_I2C_DMA(NO)								\
	static struct resource jz_i2c##NO##_dma_resources[] = {			\
		[0] = {								\
			.start          = I2C##NO##_IOBASE,			\
			.end            = I2C##NO##_IOBASE + 0x1000 - 1,	\
			.flags          = IORESOURCE_MEM,			\
		},								\
		[1] = {								\
			.start          = IRQ_I2C##NO,				\
			.end            = IRQ_I2C##NO,				\
			.flags          = IORESOURCE_IRQ,			\
		},								\
		[2] = {								\
			.start          = JZDMA_REQ_I2C##NO,			\
			.flags          = IORESOURCE_DMA,			\
		},								\
		[3] = {								\
			.start          = CONFIG_I2C##NO##_SPEED,			\
			.flags          = IORESOURCE_BUS, \
		},								\
	};									\
struct platform_device jz_i2c##NO##_dma_device = {					\
	.name = "jz-i2c-dma",							\
	.id = NO,								\
	.dev = {								\
		.dma_mask               = &jz_i2c_dmamask,			\
		.coherent_dma_mask      = 0xffffffff,				\
	},									\
	.num_resources  = ARRAY_SIZE(jz_i2c##NO##_dma_resources),			\
	.resource       = jz_i2c##NO##_dma_resources,				\
};

#ifdef CONFIG_I2C0_DMA_JZ4775
DEF_I2C_DMA(0);
#endif
#ifdef CONFIG_I2C1_DMA_JZ4775
DEF_I2C_DMA(1);
#endif
#ifdef CONFIG_I2C2_DMA_JZ4775
DEF_I2C_DMA(2);
#endif
#endif

#ifdef CONFIG_I2C_GPIO /*CONFIG_I2C_GPIO*/

#define DEF_GPIO_I2C(NO,GPIO_I2C_SDA,GPIO_I2C_SCK)		\
static struct i2c_gpio_platform_data i2c##NO##_gpio_data = {	\
	.sda_pin	= GPIO_I2C_SDA,				\
	.scl_pin	= GPIO_I2C_SCK,				\
	.udelay = 1,							\
};								\
struct platform_device i2c##NO##_gpio_device = {     	\
	.name	= "i2c-gpio",					\
	.id	= NO,						\
	.dev	= { .platform_data = &i2c##NO##_gpio_data,},	\
};

DEF_GPIO_I2C(0,GPIO_PD(30),GPIO_PD(31));
#endif

/**
 * sound devices, include i2s,pcm, mixer0 - 1(mixer is used for debug) and an internal codec
 * note, the internal codec can only access by i2s0
 **/
static u64 jz_i2s_dmamask =  ~(u32)0;
static struct resource jz_i2s_resources[] = {
	[0] = {
		.start          = AIC0_IOBASE,
		.end            = AIC0_IOBASE + 0x1000 -1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start			= IRQ_AIC0,
		.end			= IRQ_AIC0,
		.flags			= IORESOURCE_IRQ,
	},
};
struct platform_device jz_i2s_device = {
	.name		= DEV_DSP_NAME,
	.id			= minor2index(SND_DEV_DSP0),
	.dev = {
		.dma_mask               = &jz_i2s_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.resource       = jz_i2s_resources,
	.num_resources  = ARRAY_SIZE(jz_i2s_resources),
};

static u64 jz_pcm_dmamask =  ~(u32)0;
static struct resource jz_pcm_resources[] = {
	[0] = {
		.start          = PCM0_IOBASE,
		.end            = PCM0_IOBASE,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start			= IRQ_PCM0,
		.end			= IRQ_PCM0,
		.flags			= IORESOURCE_IRQ,
	},
};
struct platform_device jz_pcm_device = {
	.name		= DEV_DSP_NAME,
	.id			= minor2index(SND_DEV_DSP1),
	.dev = {
		.dma_mask               = &jz_pcm_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.resource       = jz_pcm_resources,
	.num_resources  = ARRAY_SIZE(jz_pcm_resources),
};

#define DEF_MIXER(NO)				\
struct platform_device jz_mixer##NO##_device = {		\
	.name	= DEV_MIXER_NAME,			\
	.id		= minor2index(SND_DEV_MIXER##NO),	\
};
DEF_MIXER(0);
DEF_MIXER(1);

struct platform_device jz_codec_device = {
	.name		= "jz_codec",
};

/* only for ALSA platform devices */
struct platform_device jz4775_codec_device = {
	.name           = "jz4775-codec",
	.id             = -1,
};


struct platform_device jz47xx_i2s_device = {
	.name           = "jz47xx-i2s",
	.id             = -1,
};


struct platform_device jz47xx_pcm_device = {
	.name           = "jz47xx-pcm-audio",
	.id             = -1,
};

static u64 jz_fb_dmamask = ~(u64)0;

#define DEF_LCD(NO)								\
	static struct resource jz_fb##NO##_resources[] = {			\
		[0] = {								\
			.start          = LCDC##NO##_IOBASE,			\
			.end            = LCDC##NO##_IOBASE+ 0x1800 - 1,	\
			.flags          = IORESOURCE_MEM,			\
		},								\
		[1] = {								\
			.start          = IRQ_LCD##NO,				\
			.end            = IRQ_LCD##NO,				\
			.flags          = IORESOURCE_IRQ,			\
		},								\
	};									\
struct platform_device jz_fb##NO##_device = {					\
	.name = "jz-fb",							\
	.id = NO,								\
	.dev = {								\
		.dma_mask               = &jz_fb_dmamask,			\
		.coherent_dma_mask      = 0xffffffff,				\
	},									\
	.num_resources  = ARRAY_SIZE(jz_fb##NO##_resources),			\
	.resource       = jz_fb##NO##_resources,				\
};
DEF_LCD(0);

/* EPD Controller */
static struct resource jz_epd_resources[] = {
		[0] = {
			.start          = EPDC_IOBASE,
			.end            = EPDC_IOBASE + 0x10000 - 1,
			.flags          = IORESOURCE_MEM,
		},
		[1] = {
			.start          = IRQ_EPDC,
			.end            = IRQ_EPDC,
			.flags          = IORESOURCE_IRQ,
		}
	};

static u64 jz_epd_dmamask = ~(u32)0;
struct platform_device jz_epd_device = {
	.name           = "jz4775-epd",
	.id             = 1,
	.dev = {
		.dma_mask               = &jz_epd_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(jz_epd_resources),
	.resource       = jz_epd_resources,
};

/* EPDCE Controller */
static struct resource jz_epdce_resources[] = {
	[0] = {
		.start          = EPDCE_IOBASE,
		.end            = EPDCE_IOBASE + 0x10000 - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = IRQ_EPDCE,
		.end            = IRQ_EPDCE,
		.flags          = IORESOURCE_IRQ,
	}
};

static u64 jz_epdce_dmamask = ~(u32)0;
struct platform_device jz_epdce_device = {
	.name           = "jz4775-epdce",
	.id             = 1,
	.dev = {
		.dma_mask               = &jz_epdce_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(jz_epdce_resources),
	.resource       = jz_epdce_resources,
};

/* UART ( uart controller) */
static struct resource jz_uart0_resources[] = {
	[0] = {
		.start          = UART0_IOBASE,
		.end            = UART0_IOBASE + 0x1000 - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = IRQ_UART0,
		.end            = IRQ_UART0,
		.flags          = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART0_DMA
	[2] = {
		.start          = JZDMA_REQ_UART0,
		.flags          = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart0_device = {
	.name = "jz-uart",
	.id = 0,
	.num_resources  = ARRAY_SIZE(jz_uart0_resources),
	.resource       = jz_uart0_resources,
};


static struct resource jz_uart1_resources[] = {
	[0] = {
		.start          = UART1_IOBASE,
		.end            = UART1_IOBASE + 0x1000 - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = IRQ_UART1,
		.end            = IRQ_UART1,
		.flags          = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART1_DMA
	[2] = {
		.start          = JZDMA_REQ_UART1,
		.flags          = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart1_device = {
	.name = "jz-uart",
	.id = 1,
	.num_resources  = ARRAY_SIZE(jz_uart1_resources),
	.resource       = jz_uart1_resources,
};


static struct resource jz_uart2_resources[] = {
	[0] = {
		.start          = UART2_IOBASE,
		.end            = UART2_IOBASE + 0x1000 - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = IRQ_UART2,
		.end            = IRQ_UART2,
		.flags          = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART2_DMA
	[2] = {
		.start          = JZDMA_REQ_UART2,
		.flags          = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart2_device = {
	.name = "jz-uart",
	.id = 2,
	.num_resources  = ARRAY_SIZE(jz_uart2_resources),
	.resource       = jz_uart2_resources,
};


static struct resource jz_uart3_resources[] = {
	[0] = {
		.start          = UART3_IOBASE,
		.end            = UART3_IOBASE + 0x1000 - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = IRQ_UART3,
		.end            = IRQ_UART3,
		.flags          = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART3_DMA
	[2] = {
		.start          = JZDMA_REQ_UART3,
		.flags          = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart3_device = {
	.name = "jz-uart",
	.id = 3,
	.num_resources  = ARRAY_SIZE(jz_uart3_resources),
	.resource       = jz_uart3_resources,
};
#ifndef CONFIG_SPI0_PIO_ONLY
static u64 jz_ssi_dmamask =  ~(u32)0;
#endif

#define DEF_SSI(NO)							       \
static struct resource jz_ssi##NO##_resources[] = {			       \
       [0] = {								       \
	       .flags	       = IORESOURCE_MEM,			       \
	       .start	       = SSI##NO##_IOBASE,			       \
	       .end	       = SSI##NO##_IOBASE + 0x1000 - 1,		       \
       },								       \
       [1] = {								       \
	       .flags	       = IORESOURCE_IRQ,			       \
	       .start	       = IRQ_SSI##NO,				       \
	       .end	       = IRQ_SSI##NO,				       \
       },								       \
       [2] = {								       \
       	       .flags	       = IORESOURCE_DMA,			       \
       	       .start	       = JZDMA_REQ_SSI##NO,			       \
       },								       \
};									       \
struct platform_device jz_ssi##NO##_device = {				       \
       .name = "jz-ssi",						       \
       .id = NO,							       \
       .dev = {								       \
	       .dma_mask	       = &jz_ssi_dmamask,		       \
	       .coherent_dma_mask      = 0xffffffff,			       \
       },								       \
       .resource       = jz_ssi##NO##_resources,			       \
       .num_resources  = ARRAY_SIZE(jz_ssi##NO##_resources),		       \
};

#define DEF_PIO_SSI(NO)							       \
static struct resource jz_ssi##NO##_resources[] = {			       \
       [0] = {								       \
	       .flags	       = IORESOURCE_MEM,			       \
	       .start	       = SSI##NO##_IOBASE,			       \
	       .end	       = SSI##NO##_IOBASE + 0x1000 - 1,		       \
       },								       \
       [1] = {								       \
	       .flags	       = IORESOURCE_IRQ,			       \
	       .start	       = IRQ_SSI##NO,				       \
	       .end	       = IRQ_SSI##NO,				       \
       },								       \
};									       \
struct platform_device jz_ssi##NO##_device = {				       \
       .name = "jz-ssi",						       \
       .id = NO,							       \
       .resource       = jz_ssi##NO##_resources,			       \
       .num_resources  = ARRAY_SIZE(jz_ssi##NO##_resources),		       \
};

#ifdef CONFIG_SPI0_PIO_ONLY
DEF_PIO_SSI(0);
#else
DEF_SSI(0);
#endif

/* CIM (camera module interface controller) */
#define DEF_CIM(NO)										\
	static struct resource jz_cim##NO##_resources[] = {	\
		[0] = {									\
			.flags = IORESOURCE_MEM,			\
			.start = CIM##NO##_IOBASE,			\
			.end = CIM##NO##_IOBASE + 0x10000 - 1,	\
		},										\
		[1] = {									\
			.flags = IORESOURCE_IRQ,			\
			.start = IRQ_CIM##NO,				\
			.end   = IRQ_CIM##NO,				\
		},										\
	};											\
struct platform_device jz_cim##NO##_device = {	\
	.name = "jz-cim",							\
	.id = NO,									\
	.resource = jz_cim##NO##_resources,			\
	.num_resources = ARRAY_SIZE(jz_cim##NO##_resources),	\
};

#ifdef CONFIG_JZ_CIM0
DEF_CIM(0);
#endif
#ifdef CONFIG_JZ_CIM1
DEF_CIM(1);
#endif

#if defined(CONFIG_VIDEO_JZ4780_CIM_HOST)
static struct resource jz_cim_resources[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = CIM1_IOBASE,
		.end = CIM1_IOBASE + 0x10000 - 1,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_CIM1,
	}
};

struct platform_device jz_cim_device = {
	.name = "jz4780-cim",
	.id = 0,
	.resource = jz_cim_resources,
	.num_resources = ARRAY_SIZE(jz_cim_resources),
};
#endif

/* X2D (Extreme 2D module interface controller) */
static struct resource jz_x2d_res[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = X2D_IOBASE,
		.end = X2D_IOBASE + 0x10000 - 1,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.name = "x2d_irq",
		.start = IRQ_X2D,
	}
};
struct platform_device jz_x2d_device = {
	.name = "x2d",
	.id = -1,
	.resource = jz_x2d_res,
	.num_resources = ARRAY_SIZE(jz_x2d_res),
};

/* OHCI (USB full speed host controller) */
static struct resource jz_ohci_resources[] = {
	[0] = {
		.start		= OHCI_IOBASE,
		.end		= OHCI_IOBASE + 0x10000 - 1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= IRQ_OHCI,
		.end		= IRQ_OHCI,
		.flags		= IORESOURCE_IRQ,
	},
};

static u64 ohci_dmamask = ~(u32)0;

struct platform_device jz_ohci_device = {
	.name		= "jz-ohci",
	.id		= 0,
	.dev = {
		.dma_mask		= &ohci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(jz_ohci_resources),
	.resource	= jz_ohci_resources,
};

/*  nand device  */
static struct resource jz_nand_res[] ={
	/**  nemc resource  **/
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = NEMC_IOBASE,
		.end = NEMC_IOBASE + 0x10000 -1,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_NEMC,
	},
	/**  bch resource  **/
	[2] = {
		.flags = IORESOURCE_MEM,
		.start = BCH_IOBASE,
		.end = BCH_IOBASE + 0x10000 -1,
	},
	[3] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_BCH,
	},
	/**  pdma resource  **/
	[4] = {
		.flags = IORESOURCE_MEM,
		.start = PDMA_IOBASE,
		.end = PDMA_IOBASE +0x10000 -1,
	},
	[5] = {
		.flags = IORESOURCE_DMA,
		.start = JZDMA_REQ_NAND3,
	},
	/**  csn resource  **/
	[6] = {
		.flags = IORESOURCE_MEM,
		.start = NEMC_CS1_IOBASE,
		.end = NEMC_CS1_IOBASE + 0x1000000 -1,
	},
	[7] = {
		.flags = IORESOURCE_MEM,
		.start = NEMC_CS2_IOBASE,
		.end = NEMC_CS2_IOBASE + 0x1000000 -1,
	},
};

struct platform_device jz_nand_device = {
	.name = "jz_nand",
	.id = -1,
	//	.dev.platform_data = &pisces_nand_chip_data,
	.resource = jz_nand_res,
	.num_resources =ARRAY_SIZE(jz_nand_res),
};

static struct resource jz_hdmi_resources[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = HDMI_IOBASE,
		.end = HDMI_IOBASE + 0x8000 - 1,
	},
#if 0
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_HDMI,
	},
#endif
};

struct platform_device jz_hdmi = {
	.name = "jz-hdmi",
	.id = -1,
	.num_resources = ARRAY_SIZE(jz_hdmi_resources),
	.resource = jz_hdmi_resources,
};



/* EHCI (USB high speed host controller) */
static struct resource jz_ehci_resources[] = {
	[0] = {
		.start          = EHCI_IOBASE,
		.end            = EHCI_IOBASE + 0x10000 - 1,
		.flags          = IORESOURCE_MEM,
	},
#if 0
	[1] = {
		.start          = IRQ_EHCI,
		.end            = IRQ_EHCI,
		.flags          = IORESOURCE_IRQ,
	},
#endif
};

/* The dmamask must be set for OHCI to work */
static u64 ehci_dmamask = ~(u32)0;

struct platform_device jz_ehci_device = {
	.name           = "jz-ehci",
	.id             = 0,
	.dev = {
		.dma_mask               = &ehci_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(jz_ehci_resources),
	.resource       = jz_ehci_resources,
};

static struct resource jz_rtc_resource[] = {
	[0] = {
		.start = RTC_IOBASE,
		.end   = RTC_IOBASE + 0xff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_RTC,
		.end   = IRQ_RTC,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device jz_rtc_device = {
	.name             = "jz-rtc",
	.id               = 0,
	.num_resources    = ARRAY_SIZE(jz_rtc_resource),
	.resource         = jz_rtc_resource,
};

static struct resource jz_vpu_resource[] = {
	[0] = {
		.start = SCH_IOBASE,
		.end = SCH_IOBASE + 0xF0000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_VPU,
		.end   = IRQ_VPU,
		.flags = IORESOURCE_IRQ,
	},

};

struct platform_device jz_vpu_device = {
	.name             = "jz-vpu",
	.id               = 0,
	.num_resources    = ARRAY_SIZE(jz_vpu_resource),
	.resource         = jz_vpu_resource,
};

/* ADC controller*/
static struct resource jz_adc_resources[] = {
	{
		.start	= SADC_IOBASE,
		.end	= SADC_IOBASE + 0x34,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= IRQ_SADC,
		.end	= IRQ_SADC,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= IRQ_SADC_BASE,
		.end	= IRQ_SADC_BASE,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device jz_adc_device = {
	.name	= "jz4775-adc",
	.id	= -1,
	.num_resources	= ARRAY_SIZE(jz_adc_resources),
	.resource	= jz_adc_resources,
};

static struct resource jz_dwc_otg_resources[] = {
	[0] = {
		.start	= OTG_IOBASE,
		.end	= OTG_IOBASE + 0x40000 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_OTG,
		.end   = IRQ_OTG,
	},
};

struct platform_device  jz_dwc_otg_device = {
	.name = "jz-dwc2",
	.id = -1,
	.num_resources	= ARRAY_SIZE(jz_dwc_otg_resources),
	.resource	= jz_dwc_otg_resources,
};
#ifdef CONFIG_JZ_EFUSE_V11
/* efuse */
struct platform_device jz_efuse_device = {
       .name = "jz-efuse",
};
#endif
