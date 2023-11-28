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

#include <soc/gpio.h>
#include <soc/base.h>
#include <soc/irq.h>
#include <gpio.h>

#include <mach/platform.h>
#include <mach/jzdma.h>
#include <mach/jzsnd.h>

/* device IO define array */
struct jz_gpio_func_def platform_devio_array[] = {
#ifdef CONFIG_MMC0_JZ4780_PA_4BIT
	MSC0_PORTA_4BIT,
#endif
#ifdef CONFIG_MMC0_JZ4780_PA_8BIT
	MSC0_PORTA_8BIT,
#endif
#ifdef CONFIG_MMC0_JZ4780_PE_4BIT
	MSC0_PORTE,
#endif
#ifdef CONFIG_MMC0_JZ4780_PA_4BIT_RESET
	MSC0_PORTA_4BIT_RESET,
#endif
#ifdef CONFIG_MMC0_JZ4780_PA_8BIT_RESET
	MSC0_PORTA_8BIT_RESET,
#endif
#ifdef CONFIG_MMC1_JZ4780_PD_4BIT
	MSC1_PORTD,
#endif
#ifdef CONFIG_MMC1_JZ4780_PE_4BIT
	MSC1_PORTE,
#endif
#ifdef CONFIG_MMC2_JZ4780_PB_4BIT
	MSC2_PORTB,
#endif
#ifdef CONFIG_MMC2_JZ4780_PE_4BIT
	MSC2_PORTE,
#endif
#ifdef CONFIG_I2C0_JZ4780
	I2C0_PORTD,
#endif
#ifdef CONFIG_I2C1_JZ4780
	I2C1_PORTE,
#endif
#ifdef CONFIG_I2C2_JZ4780
	I2C2_PORTF,
#endif
#ifdef CONFIG_I2C3_JZ4780
	I2C3_PORTD,
#endif
#ifdef CONFIG_I2C4_JZ4780_PE3
	I2C4_PORTE_OFF3,
#endif
#ifdef CONFIG_I2C4_JZ4780_PE12
	I2C4_PORTE_OFF12,
#endif
#ifdef CONFIG_I2C4_JZ4780_PF
	I2C4_PORTF,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART0
	UART0_PORTF,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	UART1_PORTD,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART2
	UART2_PORTD,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART3
	UART3_JTAG,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART4
	UART4_PORTC,
#endif
#ifdef CONFIG_NEMC_SRAM_8BIT
	SRAM_CS5_PORTAB_BIT8,
#endif
#ifdef CONFIG_NAND_COMMON
	NAND_PORTAB_COMMON,
	NAND_PORTA_CS1,
	NAND_PORTA_CS2,
	NAND_PORTA_CS3,
	NAND_PORTA_CS4,
#endif
#ifdef CONFIG_NAND_CS5
	NAND_PORTA_CS5,
#endif
#ifdef CONFIG_NEMC_CS6
	NEMC_PORTA_CS6,
#endif

#ifdef CONFIG_DM9000_CDM_SA1
	DM9000_CDM_TO_SA1,
#endif

#ifdef	CONFIG_SOUND_I2S_JZ47XX
#ifndef CONFIG_JZ_INTERNAL_CODEC
	I2S_PORTDE,
#endif
#endif
#ifdef	CONFIG_SOUND_SPDIF_JZ47XX
#ifdef CONFIG_JZ_INTERNAL_CODEC
	I2S_PORTDE,
#endif
#endif
#ifdef CONFIG_SOUND_PCM_JZ47XX
	PCM_PORTD,
#endif
#ifndef CONFIG_DISABLE_LVDS_FUNCTION
#ifndef CONFIG_SERIAL_JZ47XX_UART4
	DISABLE_LCD_AND_UART4_PORTC,
#else
	DISABLE_LCD_PORTC,
#endif
#else
	LCD_PORTC,
#endif
#if defined(CONFIG_HDMI_JZ4780) || defined(CONFIG_HDMI_JZ4780_MODULE)
	HDMI_PORTF,
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
	PWM_PORTE_BIT4,
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E5
	PWM_PORTE_BIT5,
#endif
#ifdef CONFIG_JZ_PWM_GPIO_D10
	PWM_PORTD_BIT10,
#endif
#ifdef CONFIG_JZ_PWM_GPIO_D11
	PWM_PORTD_BIT11,
#endif

#ifdef CONFIG_JZ_MAC
	MII_PORTBDF,
#endif

#ifdef CONFIG_USB_DWC2_DRVVBUS_PIN
	OTG_DRVVUS,
#endif

#if defined(CONFIG_JZ_CIM) || defined(CONFIG_VIDEO_JZ4780_CIM_HOST)
	CIM_PORTB,
#endif

#ifdef CONFIG_SPI0_JZ4780_PA
       SSI0_PORTA,
#endif
#ifdef CONFIG_SPI0_JZ4780_PB
       SSI0_PORTB,
#endif
#ifdef CONFIG_SPI0_JZ4780_PD
       SSI0_PORTD,
#endif
#ifdef CONFIG_SPI0_JZ4780_PE
       SSI0_PORTE,
#endif

#ifdef CONFIG_SPI1_JZ4780_PB
       SSI1_PORTB,
#endif
#ifdef CONFIG_SPI1_JZ4780_PD
       SSI1_PORTD,
#endif
#ifdef CONFIG_SPI1_JZ4780_PE
       SSI1_PORTE,
#endif
#ifdef CONFIG_PS2_KEYBOARD_MOUSE_JTAG
	PS2_JTAG,
#endif
#ifdef CONFIG_PS2_MOUSE_PD
	PS2_MOUSE_PORTD,
#endif
#ifdef CONFIG_PS2_KEYBOARD_PD
	PS2_KEYBOARD_PORTD,
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
		/*
		 * TODO:
		 * you guys mark these channels
		 * are dedicate for NAND ?
		 *
		 * you guys means no matter what driver need,
		 * i must use JZDMA_REQ_NANDXXX to request
		 * a common DMA channel ?
		 *
		 * i think we should level them original
		 */
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
		JZDMA_REQ_I2S1,
		JZDMA_REQ_I2S1,
		JZDMA_REQ_I2S0,
		JZDMA_REQ_I2S0,
#ifdef CONFIG_SERIAL_JZ47XX_UART4_DMA
		JZDMA_REQ_UART4,
		JZDMA_REQ_UART4,
#endif
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
		JZDMA_REQ_I2C3,
		JZDMA_REQ_I2C3,
		JZDMA_REQ_I2C4,
		JZDMA_REQ_I2C4,
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

#if (defined(CONFIG_I2C0_JZ4780) || defined(CONFIG_I2C1_JZ4780) ||	\
		defined(CONFIG_I2C2_JZ4780) || defined(CONFIG_I2C3_JZ4780) ||	\
		defined(CONFIG_I2C4_JZ4780))
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
#ifdef CONFIG_I2C0_JZ4780
DEF_I2C(0);
#endif
#ifdef CONFIG_I2C1_JZ4780
DEF_I2C(1);
#endif
#ifdef CONFIG_I2C2_JZ4780
DEF_I2C(2);
#endif
#ifdef CONFIG_I2C3_JZ4780
DEF_I2C(3);
#endif
#ifdef CONFIG_I2C4_JZ4780
DEF_I2C(4);
#endif
#endif

/**
 * sound devices, include i2s,pcm, mixer0 - 1(mixer is used for debug) and an internal codec
 * note, the internal codec can only access by i2s0
 **/
static u64 jz_i2s_dmamask =  ~(u32)0;
static struct resource jz_i2s_resources[] = {
	[0] = {
		.start          = AIC0_IOBASE,
		.end            = AIC0_IOBASE + 0x70 -1,
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
		.end            = PCM0_IOBASE + 0x1000 -1,
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

static u64 jz_spdif_dmamask =  ~(u32)0;
static struct resource jz_spdif_resources[] = {
	[0] = {
		.start          = SPDIF0_IOBASE,
		.end            = SPDIF0_IOBASE + 0x20 -1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start			= IRQ_AIC0,
		.end			= IRQ_AIC0,
		.flags			= IORESOURCE_IRQ,
	},
};
struct platform_device jz_spdif_device = {
	.name		= DEV_DSP_NAME,
	.id			= minor2index(SND_DEV_DSP2),
	.dev = {
		.dma_mask               = &jz_spdif_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.resource       = jz_spdif_resources,
	.num_resources  = ARRAY_SIZE(jz_spdif_resources),
};

#define DEF_MIXER(NO)				\
struct platform_device jz_mixer##NO##_device = {		\
	.name	= DEV_MIXER_NAME,			\
	.id		= minor2index(SND_DEV_MIXER##NO),	\
};
DEF_MIXER(0);
DEF_MIXER(1);
DEF_MIXER(2);

struct platform_device jz_codec_device = {
	.name		= "jz_codec",
};

/* only for ALSA platform devices */
struct platform_device jz4780_codec_device = {
	.name           = "jz4780-codec",
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

/* GPU */
static struct resource jz_gpu_resources[] = {
	[0] = {
		.start          = GPU_IOBASE,
		.end            = GPU_IOBASE + 0x1000 - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = IRQ_GPU,
		.end            = IRQ_GPU,
		.flags          = IORESOURCE_IRQ,
	},
};

struct platform_device jz_gpu = {
	.name = "pvrsrvkm",
	.id = 0,
	.num_resources  = ARRAY_SIZE(jz_gpu_resources),
	.resource       = jz_gpu_resources,
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
DEF_LCD(1);

static u64 jz_ipu_dmamask = ~(u64)0;

#define DEF_IPU(NO)								\
	static struct resource jz_ipu##NO##_resources[] = {			\
		[0] = {								\
			.start          = IPU##NO##_IOBASE,			\
			.end            = IPU##NO##_IOBASE+ 0x8000 - 1,		\
			.flags          = IORESOURCE_MEM,			\
		},								\
		[1] = {								\
			.start          = IRQ_IPU##NO,				\
			.end            = IRQ_IPU##NO,				\
			.flags          = IORESOURCE_IRQ,			\
		},								\
	};									\
struct platform_device jz_ipu##NO##_device = {					\
	.name = "jz-ipu",							\
	.id = NO,								\
	.dev = {								\
		.dma_mask               = &jz_ipu_dmamask,			\
		.coherent_dma_mask      = 0xffffffff,				\
	},									\
	.num_resources  = ARRAY_SIZE(jz_ipu##NO##_resources),			\
	.resource       = jz_ipu##NO##_resources,				\
};
DEF_IPU(0);
DEF_IPU(1);

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


static struct resource jz_uart4_resources[] = {
	[0] = {
		.start          = UART4_IOBASE,
		.end            = UART4_IOBASE + 0x1000 - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = IRQ_UART4,
		.end            = IRQ_UART4,
		.flags          = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART4_DMA
	[2] = {
		.start          = JZDMA_REQ_UART4,
		.flags          = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart4_device = {
	.name = "jz-uart",
	.id = 4,
	.num_resources  = ARRAY_SIZE(jz_uart4_resources),
	.resource       = jz_uart4_resources,
};

static u64 jz_ssi_dmamask =  ~(u32)0;

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

#ifdef CONFIG_SPI1_PIO_ONLY
DEF_PIO_SSI(1);
#else
DEF_SSI(1);
#endif

/* CIM (camera module interface controller) */
static struct resource jz_cim_resources[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = CIM_IOBASE,
		.end = CIM_IOBASE + 0x10000 - 1,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_CIM,
	}
};

#ifdef CONFIG_JZ_CIM
struct platform_device jz_cim_device = {
	.name = "jz-cim",
	.id = -1,
	.resource = jz_cim_resources,
	.num_resources = ARRAY_SIZE(jz_cim_resources),
};
#else
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

/* EHCI (USB high speed host controller) */
static struct resource jz_ehci_resources[] = {
	[0] = {
		.start		= EHCI_IOBASE,
		.end		= EHCI_IOBASE + 0x10000 - 1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= IRQ_EHCI,
		.end		= IRQ_EHCI,
		.flags		= IORESOURCE_IRQ,
	},
};

/* The dmamask must be set for OHCI to work */
static u64 ehci_dmamask = ~(u32)0;

struct platform_device jz_ehci_device = {
	.name		= "jz-ehci",
	.id		= 0,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(jz_ehci_resources),
	.resource	= jz_ehci_resources,
};

static struct resource	jz_mac_res[] = {
	{ .flags = IORESOURCE_MEM,
		.start = ETHC_IOBASE,
		.end = ETHC_IOBASE + 0xfff,
	},
	{ .flags = IORESOURCE_IRQ,
		.start = IRQ_ETHC,
	},
};

struct platform_device jz_mac = {
	.name = "jzmac",
	.id = 0,
	.num_resources = ARRAY_SIZE(jz_mac_res),
	.resource = jz_mac_res,
	.dev = {
		.platform_data = NULL,
	},
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
	[8] = {
		.flags = IORESOURCE_MEM,
		.start = NEMC_CS3_IOBASE,
		.end = NEMC_CS3_IOBASE + 0x1000000 -1,
	},
	[9] = {
		.flags = IORESOURCE_MEM,
		.start = NEMC_CS4_IOBASE,
		.end = NEMC_CS4_IOBASE + 0x1000000 -1,
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
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_HDMI,
	},
};

struct platform_device jz_hdmi = {
	.name = "jz-hdmi",
	.id = -1,
	.num_resources = ARRAY_SIZE(jz_hdmi_resources),
	.resource = jz_hdmi_resources,
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
		.end	= SADC_IOBASE + 0x32,
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
	.name	= "jz4780-adc",
	.id	= -1,
	.num_resources	= ARRAY_SIZE(jz_adc_resources),
	.resource	= jz_adc_resources,
};

static struct resource jz_aosd_resources[] = {
	[0] = {
		.start	= COMPRESS_IOBASE,
		.end	= COMPRESS_IOBASE + 0x120 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_COMPRESS,
		.end   = IRQ_COMPRESS,
	},
};

struct platform_device jz_aosd_device = {
	.name	= "jz-aosd",
	.id	= -1,
	.num_resources	= ARRAY_SIZE(jz_aosd_resources),
	.resource	= jz_aosd_resources,
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
	.name = "jz4780-dwc2",
	.id = -1,
	.num_resources	= ARRAY_SIZE(jz_dwc_otg_resources),
	.resource	= jz_dwc_otg_resources,
};

/* efuse */

static struct resource jz_efuse_resources[] = {
	[0] = {
		.start	= NEMC_IOBASE + 0xd0,
		.end	= NEMC_IOBASE + 0xfc - 1,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device jz_efuse_device = {
	.name	= "jz4780-efuse",
	.id	= -1,
	.num_resources	= ARRAY_SIZE(jz_efuse_resources),
	.resource	= jz_efuse_resources,
};
