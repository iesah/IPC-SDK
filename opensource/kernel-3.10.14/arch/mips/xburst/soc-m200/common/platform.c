/*
 * Platform device support for M200 SoC.
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
//#include <mach/jznand.h>

/* device IO define array */
struct jz_gpio_func_def platform_devio_array[] = {
#ifdef CONFIG_JZMMC_V12_MMC0_PA_4BIT
	MSC0_PORTA_4BIT,
#endif
#ifdef CONFIG_JZMMC_V12_MMC0_PA_8BIT
	MSC0_PORTA_8BIT,
#endif
#ifdef CONFIG_JZMMC_V12_MMC0_PE_4BIT
	MSC0_PORTE,
#endif
#ifdef CONFIG_JZMMC_V12_MMC1_PD_4BIT
	MSC1_PORTD,
#endif
#ifdef CONFIG_JZMMC_V12_MMC1_PE_4BIT
	MSC1_PORTE,
#endif
#ifdef CONFIG_JZMMC_V12_MMC2_PB_4BIT
	MSC2_PORTB,
#endif
#ifdef CONFIG_JZMMC_V12_MMC2_PE_4BIT
	MSC2_PORTE,
#endif
#if	(defined(CONFIG_I2C0_V12_JZ))
	I2C0_PORTD,
#endif
#if	(defined(CONFIG_I2C1_V12_JZ))
	I2C1_PORTE,
#endif

#if	(defined(CONFIG_I2C2_V12_JZ))
	#ifdef CONFIG_I2C2_PA
		I2C2_PORTA,
	#else
		I2C2_PORTE,
	#endif
#endif

#if	(defined(CONFIG_I2C3_V12_JZ))
	#ifdef CONFIG_I2C3_PB
		I2C3_PORTB,
	#else
		I2C3_PORTC,
	#endif
#endif
#ifdef CONFIG_OVISP_I2C
	ISP_I2C,
#ifdef CONFIG_MCLK_GPIO_PC24
	ISP_PORTC,
#endif
#ifdef CONFIG_MCLK_GPIO_PE02
	MCLK_PORTE_BIT2,
#endif
#endif

#if (defined(CONFIG_DVP_CAMERA0))
	CIM_PORTC,
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART0
	UART0_PORTF,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	UART1_PORTD,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART2
	UART2_PORTC,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART3
	UART3_PORTA,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART4
#ifdef SERIAL_JZ47XX_UART4_PB
	UART4_PORTB,
#else
	UART4_PORTF,
#endif
#endif
#ifdef CONFIG_NAND_DRIVER
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

#ifdef	CONFIG_SOUND_JZ_I2S_V12
#ifndef CONFIG_JZ_INTERNAL_CODEC_V12
	I2S_PORTDE,
#endif
#endif
#ifdef	CONFIG_SOUND_JZ_SPDIF_V12
#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
	I2S_PORTDE,
#endif
#endif
#ifdef CONFIG_SOUND_JZ_PCM_V12
	PCM_PORTF,
#endif

#if defined(CONFIG_JZ_DMIC0)
	DMIC_PORTF,
#endif

#if defined(CONFIG_JZ_DMIC1)
#ifndef CONFIG_USB_DWC2_DUAL_ROLE
#ifndef CONFIG_USB_DWC2_HOST_ONLY
	DMIC_PORTE,
#endif
#endif
#endif

#if defined(CONFIG_LCD_GPIO_FUNC0_16BIT) || defined(CONFIG_LCD_GPIO_FUNC0_24BIT)
	LCD_PORTC,
#endif

#if defined(CONFIG_LCD_GPIO_FUNC0_18BIT)
	LCD_PORTC_18BIT,
#endif

#ifdef CONFIG_LCD_GPIO_FUNC2_SLCD
	SLCDC_PORTC,
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

#ifdef CONFIG_USB_DWC2_DRVVBUS_FUNCTION_PIN
	OTG_DRVVUS,
#endif

#ifdef CONFIG_SPI0_JZ_V1_2_PA
	SSI0_PORTA,
#endif
#ifdef CONFIG_SPI0_JZ_V1_2_PB
	SSI0_PORTB,
#endif
#ifdef CONFIG_SPI0_JZ_V1_2_PD
	SSI0_PORTD,
#endif
#ifdef CONFIG_SPI1_JZ_V1_2_PE
	SSI1_PORTE,
#endif

};

int platform_devio_array_size = ARRAY_SIZE(platform_devio_array);

int jz_device_register(struct platform_device *pdev, void *pdata)
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
	       .name = "irq",
	       .flags = IORESOURCE_IRQ,
	       .start = IRQ_PDMA,
	       },
	[2] = {
	       .name = "pdmam",
	       .flags = IORESOURCE_IRQ,
	       .start = IRQ_PDMAM,
	       },
	[3] = {
	       .name = "mcu",
	       .flags = IORESOURCE_IRQ,
	       .start = IRQ_MCU,
	       },
};

static struct jzdma_platform_data jzdma_pdata = {
	.irq_base = IRQ_MCU_BASE,
	.irq_end = IRQ_MCU_END,
	.map = {
		JZDMA_REQ_NAND0,
		JZDMA_REQ_NAND1,
		JZDMA_REQ_NAND2,
		JZDMA_REQ_NAND3,
		JZDMA_REQ_NAND4,
		JZDMA_REQ_I2S1,
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
		JZDMA_REQ_PCM0,
		JZDMA_REQ_PCM0,
#ifdef CONFIG_JZMMC_USE_PDMA
		JZDMA_REQ_MSC0,
		JZDMA_REQ_MSC0,
		JZDMA_REQ_MSC1,
		JZDMA_REQ_MSC1,
		JZDMA_REQ_MSC2,
		JZDMA_REQ_MSC2,
#endif
		JZDMA_REQ_I2C0,
		JZDMA_REQ_I2C0,
		JZDMA_REQ_I2C1,
		JZDMA_REQ_I2C1,
		JZDMA_REQ_I2C2,
		JZDMA_REQ_I2C2,
		JZDMA_REQ_I2C3,
		JZDMA_REQ_I2C3,
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


/* ADC controller*/
static struct resource jz_adc_resources[] = {
	{
		.start  = SADC_IOBASE,
		.end    = SADC_IOBASE + 0x32,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = IRQ_SADC,
		.end    = IRQ_SADC,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = IRQ_SADC_BASE,
		.end    = IRQ_SADC_BASE,
		.flags  = IORESOURCE_IRQ
	},
};

struct platform_device jz_adc_device = {
	.name   = "jz-adc",
	.id = -1,
	.num_resources  = ARRAY_SIZE(jz_adc_resources),
	.resource   = jz_adc_resources,
};

/* MSC ( msc controller v1.2) */
static u64 jz_msc_dmamask = ~(u32) 0;

#define DEF_MSC(NO)							\
	static struct resource jz_msc##NO##_resources[] = {		\
		{							\
			.start          = MSC##NO##_IOBASE,		\
			.end            = MSC##NO##_IOBASE + 0x1000 - 1, \
			.flags          = IORESOURCE_MEM,		\
		},							\
		{							\
			.start          = IRQ_MSC##NO,			\
			.end            = IRQ_MSC##NO,			\
			.flags          = IORESOURCE_IRQ,		\
		},							\
	};								\
	struct platform_device jz_msc##NO##_device = {                  \
		.name = "jzmmc_v1.2",					\
		.id = NO,						\
		.dev = {						\
			.dma_mask               = &jz_msc_dmamask,	\
			.coherent_dma_mask      = 0xffffffff,		\
		},							\
		.resource       = jz_msc##NO##_resources,               \
		.num_resources  = ARRAY_SIZE(jz_msc##NO##_resources),	\
	};
DEF_MSC(0);
DEF_MSC(1);
DEF_MSC(2);

static u64 jz_fb_dmamask = ~(u64) 0;

static struct resource jz_fb_resources[] = {
	[0] = {
		.start = LCDC_IOBASE,
		.end = LCDC_IOBASE + 0x1800 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD,
		.end = IRQ_LCD,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_fb_device = {
	.name = "jz-fb",
	.id = 0,
	.dev = {
		.dma_mask = &jz_fb_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(jz_fb_resources),
	.resource = jz_fb_resources,
};

/*vpu*/
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

/* ipu */
static u64 jz_ipu_dmamask = ~(u64) 0;
static struct resource jz_ipu_resources[] = {
	[0] = {
		.start = IPU_IOBASE,
		.end = IPU_IOBASE + 0x8000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_IPU,
		.end = IRQ_IPU,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_ipu_device = {
	.name = "jz-ipu",
	.id = 0,
	.dev = {
		.dma_mask = &jz_ipu_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(jz_ipu_resources),
	.resource = jz_ipu_resources,
};

/*DWC OTG*/
static struct resource jz_dwc_otg_resources[] = {
	[0] = {
		.start = OTG_IOBASE,
		.end = OTG_IOBASE + 0x40000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_OTG,
		.end = IRQ_OTG,
	},
};

struct platform_device jz_dwc_otg_device = {
	.name = "jz-dwc2",
	.id = -1,
	.num_resources = ARRAY_SIZE(jz_dwc_otg_resources),
	.resource = jz_dwc_otg_resources,
};

/* UART ( uart controller) */
static struct resource jz_uart0_resources[] = {
	[0] = {
		.start = UART0_IOBASE,
		.end = UART0_IOBASE + 0x1000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_UART0,
		.end = IRQ_UART0,
		.flags = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART0_DMA
	[2] = {
		.start = JZDMA_REQ_UART0,
		.flags = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart0_device = {
	.name = "jz-uart",
	.id = 0,
	.num_resources = ARRAY_SIZE(jz_uart0_resources),
	.resource = jz_uart0_resources,
};

static struct resource jz_uart1_resources[] = {
	[0] = {
		.start = UART1_IOBASE,
		.end = UART1_IOBASE + 0x1000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_UART1,
		.end = IRQ_UART1,
		.flags = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART1_DMA
	[2] = {
		.start = JZDMA_REQ_UART1,
		.flags = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart1_device = {
	.name = "jz-uart",
	.id = 1,
	.num_resources = ARRAY_SIZE(jz_uart1_resources),
	.resource = jz_uart1_resources,
};

static struct resource jz_uart2_resources[] = {
	[0] = {
		.start = UART2_IOBASE,
		.end = UART2_IOBASE + 0x1000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_UART2,
		.end = IRQ_UART2,
		.flags = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART2_DMA
	[2] = {
		.start = JZDMA_REQ_UART2,
		.flags = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart2_device = {
	.name = "jz-uart",
	.id = 2,
	.num_resources = ARRAY_SIZE(jz_uart2_resources),
	.resource = jz_uart2_resources,
};

static struct resource jz_uart3_resources[] = {
	[0] = {
		.start = UART3_IOBASE,
		.end = UART3_IOBASE + 0x1000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_UART3,
		.end = IRQ_UART3,
		.flags = IORESOURCE_IRQ,
	},
#ifdef CONFIG_SERIAL_JZ47XX_UART3_DMA
	[2] = {
		.start = JZDMA_REQ_UART3,
		.flags = IORESOURCE_DMA,
	},
#endif
};

struct platform_device jz_uart3_device = {
	.name = "jz-uart",
	.id = 3,
	.num_resources = ARRAY_SIZE(jz_uart3_resources),
	.resource = jz_uart3_resources,
};

/* OHCI (USB full speed host controller) */
static struct resource jz_ohci_resources[] = {
	[0] = {
		.start = OHCI_IOBASE,
		.end = OHCI_IOBASE + 0x10000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_OHCI,
		.end = IRQ_OHCI,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 ohci_dmamask = ~(u32) 0;

struct platform_device jz_ohci_device = {
	.name = "jz-ohci",
	.id = 0,
	.dev = {
		.dma_mask = &ohci_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(jz_ohci_resources),
	.resource = jz_ohci_resources,
};

/* EHCI (USB high speed host controller) */
static struct resource jz_ehci_resources[] = {
	[0] = {
		.start = EHCI_IOBASE,
		.end = EHCI_IOBASE + 0x10000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_EHCI,
	       .end = IRQ_EHCI,
		.flags = IORESOURCE_IRQ,
	       },
};

/* The dmamask must be set for OHCI to work */
static u64 ehci_dmamask = ~(u32) 0;

struct platform_device jz_ehci_device = {
	.name = "jz-ehci",
	.id = 0,
	.dev = {
		.dma_mask = &ehci_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(jz_ehci_resources),
	.resource = jz_ehci_resources,
};

#if (defined(CONFIG_I2C0_V12_JZ) || defined(CONFIG_I2C1_V12_JZ) ||	\
     defined(CONFIG_I2C2_V12_JZ) || defined(CONFIG_I2C3_V12_JZ))
static u64 jz_i2c_dmamask = ~(u32) 0;

#define DEF_I2C(NO)							\
	static struct resource jz_i2c##NO##_resources[] = {		\
		[0] = {							\
			.start          = I2C##NO##_IOBASE,		\
			.end            = I2C##NO##_IOBASE + 0x1000 - 1, \
			.flags          = IORESOURCE_MEM,		\
		},							\
		[1] = {							\
			.start          = IRQ_I2C##NO,			\
			.end            = IRQ_I2C##NO,			\
			.flags          = IORESOURCE_IRQ,		\
		},							\
		[2] = {							\
			.start          = JZDMA_REQ_I2C##NO,		\
			.flags          = IORESOURCE_DMA,		\
		},							\
		[3] = {							\
			.start          = CONFIG_I2C##NO##_SPEED,	\
			.flags          = IORESOURCE_BUS,		\
		},							\
	};								\
	struct platform_device jz_i2c##NO##_device = {                  \
		.name = "jz-i2c",					\
		.id = NO,						\
		.dev = {						\
			.dma_mask               = &jz_i2c_dmamask,	\
			.coherent_dma_mask      = 0xffffffff,		\
		},							\
		.num_resources  = ARRAY_SIZE(jz_i2c##NO##_resources),	\
		.resource       = jz_i2c##NO##_resources,		\
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
#ifdef CONFIG_I2C3_V12_JZ
DEF_I2C(3);
#endif
#endif
/**
 * sound devices, include i2s,pcm, mixer0 - 1(mixer is used for debug) and an internal codec
 * note, the internal codec can only access by i2s0
 **/
#if defined(CONFIG_SOUND_PRIME)
static u64 jz_i2s_dmamask = ~(u32) 0;
static struct resource jz_i2s_resources[] = {
	[0] = {
	       .start = AIC0_IOBASE,
	       .end = AIC0_IOBASE + 0x70 - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = IRQ_AIC0,
	       .end = IRQ_AIC0,
	       .flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_i2s_device = {
	.name = DEV_DSP_NAME,
	.id = minor2index(SND_DEV_DSP0),
	.dev = {
		.dma_mask = &jz_i2s_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.resource = jz_i2s_resources,
	.num_resources = ARRAY_SIZE(jz_i2s_resources),
};

static u64 jz_pcm_dmamask = ~(u32) 0;
static struct resource jz_pcm_resources[] = {
	[0] = {
		.start = PCM0_IOBASE,
		.end = PCM0_IOBASE,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_PCM0,
		.end = IRQ_PCM0,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_pcm_device = {
	.name = DEV_DSP_NAME,
	.id = minor2index(SND_DEV_DSP1),
	.dev = {
		.dma_mask = &jz_pcm_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.resource = jz_pcm_resources,
	.num_resources = ARRAY_SIZE(jz_pcm_resources),
};

static u64 jz_spdif_dmamask = ~(u32) 0;
static struct resource jz_spdif_resources[] = {
	[0] = {
		.start = SPDIF0_IOBASE,
		.end = SPDIF0_IOBASE + 0x20 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_AIC0,
		.end = IRQ_AIC0,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_spdif_device = {
	.name = DEV_DSP_NAME,
	.id = minor2index(SND_DEV_DSP2),
	.dev = {
		.dma_mask = &jz_spdif_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.resource = jz_spdif_resources,
	.num_resources = ARRAY_SIZE(jz_spdif_resources),
};

static u64 jz_dmic_dmamask = ~(u32) 0;
static struct resource jz_dmic_resources[] = {
	[0] = {
		.start = DMIC_IOBASE,
		.end = DMIC_IOBASE + 0x70 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_DMIC,
		.end = IRQ_DMIC,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_dmic_device = {
	.name = DEV_DSP_NAME,
	.id = minor2index(SND_DEV_DSP3),
	.dev = {
		.dma_mask = &jz_dmic_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.resource = jz_dmic_resources,
	.num_resources = ARRAY_SIZE(jz_dmic_resources),
};

#define DEF_MIXER(NO)							\
	struct platform_device jz_mixer##NO##_device = {		\
		.name	= DEV_MIXER_NAME,				\
		.id	= minor2index(SND_DEV_MIXER##NO),		\
	};
DEF_MIXER(0);
DEF_MIXER(1);
DEF_MIXER(2);
DEF_MIXER(3);

struct platform_device jz_codec_device = {
	.name = "jz_codec",
};
#endif

#if defined(CONFIG_SND) && (defined(CONFIG_SND_ASOC_INGENIC) || defined(CONFIG_SND_ASOC_INGENIC_MODULE))
static u64 jz_asoc_dmamask =  ~(u64)0;
#if defined(CONFIG_SND_ASOC_JZ_AIC) || defined(CONFIG_SND_ASOC_JZ_AIC_MODULE)
static struct resource jz_aic_dma_resources[] = {
	[0] = {
		.start          = JZDMA_REQ_I2S0,
		.end		= JZDMA_REQ_I2S0,
		.flags          = IORESOURCE_DMA,
	},
};
struct platform_device jz_aic_dma_device = {
	.name		= "jz-asoc-aic-dma",
	.id		= -1,
	.dev = {
		.dma_mask               = &jz_asoc_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.resource       = jz_aic_dma_resources,
	.num_resources  = ARRAY_SIZE(jz_aic_dma_resources),
};

static struct resource jz_aic_resources[] = {
	[0] = {
		.start          = AIC0_IOBASE,
		.end            = AIC0_IOBASE + 0xA0 -1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start		= IRQ_AIC0,
		.end		= IRQ_AIC0,
		.flags		= (IORESOURCE_IRQ| IORESOURCE_IRQ_SHAREABLE),
	},
};
struct platform_device jz_aic_device = {
	.name		= "jz-asoc-aic",
	.id		= -1,
	.resource       = jz_aic_resources,
	.num_resources  = ARRAY_SIZE(jz_aic_resources),
};
#endif
#if defined(CONFIG_SND_ASOC_JZ_PCM) || defined(CONFIG_SND_ASOC_JZ_PCM_MODULE)
static struct resource jz_pcm_dma_resources[] = {
	[0] = {
		.start          = JZDMA_REQ_PCM0,
		.end		= JZDMA_REQ_PCM0,
		.flags          = IORESOURCE_DMA,
	},
};
struct platform_device jz_pcm_dma_device = {
	.name		= "jz-asoc-pcm-dma",
	.id		= -1,
	.dev = {
		.dma_mask               = &jz_asoc_dmamask,
		.coherent_dma_mask      = 0xffffffff,
	},
	.resource       = jz_pcm_dma_resources,
	.num_resources  = ARRAY_SIZE(jz_pcm_dma_resources),
};

static struct resource jz_pcm_resources[] = {
	[0] = {
		.start          = PCM0_IOBASE,
		.end            = PCM0_IOBASE + 0x18 -1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start		= IRQ_PCM0,
		.end		= IRQ_PCM0,
		.flags		= IORESOURCE_IRQ,
	},
};
struct platform_device jz_pcm_device = {
	.name		= "jz-asoc-pcm",
	.id		= -1,
	.resource       = jz_pcm_resources,
	.num_resources  = ARRAY_SIZE(jz_pcm_resources),
};
#endif

#if defined(CONFIG_SND_ASOC_JZ_ICDC_D1) || defined(CONFIG_SND_ASOC_JZ_ICDC_D1_MODULE)
static struct resource jz_icdc_resources[] = {
	[0] = {
		.start          = AIC0_IOBASE + 0xA0,
		.end            = AIC0_IOBASE + 0xAA -1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start		= IRQ_AIC0,
		.end		= IRQ_AIC0,
		.flags		= (IORESOURCE_IRQ | IORESOURCE_IRQ_SHAREABLE),
	},
};

struct platform_device jz_icdc_device = {	/*jz internal codec*/
	.name		= "icdc-d1",
	.id		= -1,
	.resource	= jz_icdc_resources,
	.num_resources	= ARRAY_SIZE(jz_icdc_resources),
};
#endif

#if defined(CONFIG_SND_ASOC_JZ_DUMP_CDC) || defined(CONFIG_SND_ASOC_JZ_DUMP_CDC_MODULE)
struct platform_device jz_dump_cdc_device = {	/*jz dump codec*/
	.name		= "dump",
	.id		= -1,
};
#endif
#endif /* CONFIG_SND && (CONFIG_SND_ASOC_INGENIC || CONFIG_SND_ASOC_INGENIC_MODULE) */

static u64 jz_ssi_dmamask =  ~(u32)0;
#define DEF_SSI(NO)							\
	static struct resource jz_ssi##NO##_resources[] = {		\
		[0] = {							\
			.flags	       = IORESOURCE_MEM,		\
			.start	       = SSI##NO##_IOBASE,		\
			.end	       = SSI##NO##_IOBASE + 0x1000 - 1,	\
		},							\
		[1] = {							\
			.flags	       = IORESOURCE_IRQ,		\
			.start	       = IRQ_SSI##NO,			\
			.end	       = IRQ_SSI##NO,			\
		},							\
		[2] = {							\
			.flags	       = IORESOURCE_DMA,		\
			.start	       = JZDMA_REQ_SSI##NO,		\
		},							\
	};								\
	struct platform_device jz_ssi##NO##_device = {			\
		.name = "jz-ssi",					\
		.id = NO,						\
		.dev = {						\
			.dma_mask	       = &jz_ssi_dmamask,	\
			.coherent_dma_mask      = 0xffffffff,		\
		},							\
		.resource       = jz_ssi##NO##_resources,		\
		.num_resources  = ARRAY_SIZE(jz_ssi##NO##_resources),	\
	};

#define DEF_PIO_SSI(NO)							\
	static struct resource jz_ssi##NO##_resources[] = {		\
		[0] = {							\
			.flags	       = IORESOURCE_MEM,		\
			.start	       = SSI##NO##_IOBASE,		\
			.end	       = SSI##NO##_IOBASE + 0x1000 - 1,	\
		},							\
		[1] = {							\
			.flags	       = IORESOURCE_IRQ,		\
			.start	       = IRQ_SSI##NO,			\
			.end	       = IRQ_SSI##NO,			\
		},							\
	};								\
	struct platform_device jz_ssi##NO##_device = {			\
		.name = "jz-ssi",					\
		.id = NO,						\
		.resource       = jz_ssi##NO##_resources,		\
		.num_resources  = ARRAY_SIZE(jz_ssi##NO##_resources),	\
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


#ifdef CONFIG_VIDEO_OVISP
static u64 ovisp_camera_dma_mask = ~(u64)0;
static struct resource ovisp_resource_camera[] = {
	[0] = {
		.start = ISP_IOBASE,
		.end = ISP_IOBASE + 0x50000,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_ISP,
		.end = IRQ_ISP,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device ovisp_device_camera = {
	.name = "ovisp-camera",
	.id = -1,
	.dev = {
		.dma_mask = &ovisp_camera_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(ovisp_resource_camera),
	.resource = ovisp_resource_camera,
};
#endif
#ifdef CONFIG_RTC_DRV_JZ
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
#endif

#ifdef CONFIG_JZ_EFUSE_V12
/* efuse */
struct platform_device jz_efuse_device = {
       .name = "jz-efuse-v12",
};
#endif

#ifdef CONFIG_JZ_PWM
struct platform_device jz_pwm_device = {
	.name = "jz-pwm",
	.id   = -1,
};
#endif

/*  nand device  */
static struct resource jz_nand_res[] ={
	/**  nemc resource  **/
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = NFI_IOBASE,
		.end = NFI_IOBASE + 0x160 -1,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_NFI,
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
	.resource = jz_nand_res,
	.num_resources =ARRAY_SIZE(jz_nand_res),
};
