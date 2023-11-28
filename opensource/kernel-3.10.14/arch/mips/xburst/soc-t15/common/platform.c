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
#include <mach/jzvo.h>
#include <mach/jzsnd.h>
//#include <mach/jznand.h>
#include <linux/mfd/jz_tcu.h>

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
#ifdef CONFIG_JZMMC_V12_MMC1_PB_4BIT
	MSC1_PORTB,
#endif
#ifdef CONFIG_JZMMC_V12_MMC1_PE_4BIT
	MSC1_PORTE,
#endif
#ifdef CONFIG_JZMMC_V12_MMC2_PB_FUNC1
	MSC2_PORTB_FUNC1,
#endif
#ifdef CONFIG_JZMMC_V12_MMC2_PB_4BIT
	MSC2_PORTB_4BIT,
#endif

#if	(defined(CONFIG_I2C0_V12_JZ))
	#ifdef CONFIG_I2C0_PF
	I2C0_PORTF,
	#else
	I2C0_PORTC,
	#endif
#endif
#if	(defined(CONFIG_I2C1_V12_JZ))
	#ifdef CONFIG_I2C1_PE_1
	I2C1_PORTE_1,
	#else
	I2C1_PORTE_2,
	#endif
#endif

#if	(defined(CONFIG_I2C2_V12_JZ))
	#ifdef CONFIG_I2C2_PB
		I2C2_PORTB,
	#else
		I2C2_PORTF,
	#endif
#endif

#if	(defined(CONFIG_I2C3_V12_JZ))
	#ifdef CONFIG_I2C3_PF
		I2C3_PORTF,
	#else
		I2C3_PORTC,
	#endif
#endif

#if	(defined(CONFIG_I2C4_V12_JZ))
	#ifdef CONFIG_I2C4_PB
		I2C4_PORTB,
	#else
		I2C4_PORTC,
	#endif
#endif

#ifdef CONFIG_VIDEO_TX_ISP
	MCLK_PORTE_BIT2,
#endif

#ifdef CONFIG_OVISP_I2C
	ISP_I2C,
#endif

#if (defined(CONFIG_DVP_CAMERA0))
#ifdef CONFIG_DVP_LOW_10BIT
	DVP_PORTF_10BIT,
#elif defined(CONFIG_DVP_12BIT)
	DVP_PORTF_12BIT,
#endif
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART0
	UART0_PORTF,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	UART1_PORTF,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART2
	UART2_PORTC,
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART3
	UART3_PORTE,
#endif

#ifdef CONFIG_NAND_DRIVER
	NAND_PORTAB_COMMON,
	NAND_PORTA_CS1,
	NAND_PORTA_CS2,
	NAND_PORTA_CS3,
#endif
#ifdef CONFIG_NAND_BUS_WIDTH_16
	NAND_PORTA_BUS16,
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
	I2S_PORTDE,
#endif
#ifdef CONFIG_SOUND_JZ_PCM_V12
	PCM_PORTF,
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

#ifdef CONFIG_LCD_GPIO_FUNC2_SLCD_8BIT
	SLCDC_PORTC_8BIT,
#endif

#ifdef CONFIG_VO_GPIO_16BIT
	VO_PORTC_16BIT,
#endif

#ifdef CONFIG_VO_GPIO_8BIT
	VO_PORTC_8BIT,
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

#ifdef CONFIG_SPI0_PA
	SSI0_PORTA,
#endif
#ifdef CONFIG_SPI0_PBE
	SSI0_PORTBE,
#endif
#ifdef CONFIG_SPI1_PB
	SSI1_PORTB,
#endif
#ifdef CONFIG_MTD_JZ_SFC_NORFLASH
	SFC_PORTA,
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

/* AES controller*/
static struct resource jz_aes_resources[] = {
	[0] = {
		.start  = AES_IOBASE,
		.end    = AES_IOBASE + 0x28,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_AES,
		.end    = IRQ_AES,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device jz_aes_device = {
	.name   = "jz-aes",
	.id = 0,
	.resource   = jz_aes_resources,
	.num_resources  = ARRAY_SIZE(jz_aes_resources),
};

#ifdef CONFIG_JZ_WDT
/* WDT controller*/
static struct resource jz_wdt_resources[] = {
	[0] = {
		.start  = WDT_IOBASE,
		.end    = WDT_IOBASE + 0xC,
		.flags  = IORESOURCE_MEM,
	},
};

struct platform_device jz_wdt_device = {
	.name   = "jz-wdt",
	.id = 0,
	.resource   = jz_wdt_resources,
	.num_resources  = ARRAY_SIZE(jz_wdt_resources),
};
#endif

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
#ifdef CONFIG_JZ_VO
static struct resource jz_vo_resources[] = {
	[0] = {
		.start = VO_IOBASE,
		.end = VO_IOBASE + 0x120 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_VO,
		.end = IRQ_VO,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_vo_device = {
	.name		= "jz-vo",
	.id			= 0,
	.num_resources = ARRAY_SIZE(jz_vo_resources),
	.resource = jz_vo_resources,
};
#endif

/*vpu*/
#if defined(CONFIG_SOC_VPU) && defined(CONFIG_JZ_NVPU)
static u64 jz_vpu_dmamask = ~(u64)0;
#define DEF_VPU(NO)								\
	static struct resource jz_vpu##NO##_resources[] = {			\
		[0] = {								\
			.start          = VPU_IOBASE(NO),			\
			.end            = VPU_IOBASE(NO) + 0x100000 - 1,	\
			.flags          = IORESOURCE_MEM,			\
		},								\
		[1] = {								\
			.start          = IRQ_VPU(NO),			\
			.end            = IRQ_VPU(NO),			\
			.flags          = IORESOURCE_IRQ,			\
		},								\
	};									\
struct platform_device jz_vpu##NO##_device = {					\
	.name = "jz-vpu",							\
	.id = NO,								\
	.dev = {								\
		.dma_mask               = &jz_vpu_dmamask,			\
		.coherent_dma_mask      = 0xffffffff,				\
	},									\
	.num_resources  = ARRAY_SIZE(jz_vpu##NO##_resources),			\
	.resource       = jz_vpu##NO##_resources,				\
};

#if (CONFIG_VPU_NODE_NUM >= 1)
DEF_VPU(0);
#endif
#if (CONFIG_VPU_NODE_NUM >= 2)
DEF_VPU(1);
#endif
#else
#ifdef CONFIG_JZ_VPU_V12
static struct resource jz_vpu_resource[] = {
	[0] = {
		.start = SCH_IOBASE,
		.end = SCH_IOBASE + 0xF0000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_VPU0,
		.end   = IRQ_VPU0,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_vpu_device = {
	.name             = "jz-vpu",
	.id               = 0,
	.num_resources    = ARRAY_SIZE(jz_vpu_resource),
	.resource         = jz_vpu_resource,
};
#endif
#endif

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

#ifdef CONFIG_JZ_EFUSE_V13
/* efuse */
struct platform_device jz_efuse_device = {
       .name = "jz-efuse",
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
	/*
	[8] = {
		.flags = IORESOURCE_MEM,
		.start = NEMC_CS3_IOBASE,
		.end = NEMC_CS3_IOBASE + 0x1000000 -1,
	},
	[9] = {
		.flags = IORESOURCE_MEM,
		.start = NEMC_CS4_IOBASE,
		.end = NEMC_CS4_IOBASE + 0x1000000 -1,
	},*/
};

struct platform_device jz_nand_device = {
	.name = "jz_nand",
	.id = -1,
	.resource = jz_nand_res,
	.num_resources =ARRAY_SIZE(jz_nand_res),
};

#ifdef CONFIG_VIDEO_TX_ISP
#include <mach/tx_isp.h>
static u64 tx_isp_module_dma_mask = ~(u64)0;
static struct resource tx_isp_csi_resource[] = {
	[0] = {
		.name = "mipi_csi",
		.start = MIPI_CSI_IOBASE,
		.end = MIPI_CSI_IOBASE + 0x1000,
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device tx_isp_csi_platform_device = {
	.name = TX_ISP_CSI_NAME,
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(tx_isp_csi_resource),
	.resource = tx_isp_csi_resource,
};

static struct resource tx_isp_vic_resource[] = {
	[0] = {
		.name = "isp_vic",
		.start = ISP_VIC_IOBASE,
		.end = ISP_VIC_IOBASE + 0x10000,
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device tx_isp_vic_platform_device = {
	.name = TX_ISP_VIC_NAME,
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(tx_isp_vic_resource),
	.resource = tx_isp_vic_resource,
};

static struct resource tx_isp_core_resource[] = {
	[0] = {
		.name = "isp_core",
		.start = ISP_CORE_IOBASE,
		.end = ISP_CORE_IOBASE + 0x80000,
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device tx_isp_core_platform_device = {
	.name = TX_ISP_CORE_NAME,
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(tx_isp_core_resource),
	.resource = tx_isp_core_resource,
};

static struct resource tx_isp_device_resource[] = {
	[0] = {
		.name = "isp_device_irq",
		.start = ISP_IRQ_IOBASE,
		.end = ISP_IRQ_IOBASE + 0x10000,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "isp_irq_id",
		.start = IRQ_ISP,
		.end = IRQ_ISP,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device tx_isp_platform_device = {
	.name = "tx-isp",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(tx_isp_device_resource),
	.resource = tx_isp_device_resource,
};

#endif
#ifdef CONFIG_MTD_JZ_SFC_NORFLASH
static struct resource jz_sfc_resources[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
		.start = SFC_IOBASE,
		.end   = SFC_IOBASE + 0x10000 - 1,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
		.start = IRQ_SFC,
		.end   = IRQ_SFC,
	}
};

struct platform_device jz_sfc_device = {
	.name = "jz-sfc",
	.id = -1,
	.resource = jz_sfc_resources,
	.num_resources = ARRAY_SIZE(jz_sfc_resources),
};
#endif

#ifdef CONFIG_JZ_PWM
struct platform_device jz_pwm_device = {
	.name = "jz-pwm",
	.id   = -1,
};
#endif
#ifdef CONFIG_PWM_SDK
struct platform_device jz_pwm_sdk_device = {
	.name = "pwm-sdk",
	.id   = -1,
};
#endif

#ifdef CONFIG_MFD_JZ_TCU

static struct jzpwm_platform_data jzpwm_pdata = {
	.pwm_gpio = {
		96,97,98,99,50,51,188,189,
	},
};

static struct resource jz_tcu_resources[] = {
	{
		.flags = IORESOURCE_MEM,
		.start = TCU_IOBASE,
		.end   = TCU_IOBASE + 0x10000 - 1,
	},
	{
		.flags = IORESOURCE_IRQ,
		.start = IRQ_TCU2,
		.end   = IRQ_TCU2,
	},
	{
		.flags  = IORESOURCE_IRQ,
		.start  = IRQ_TCU_BASE,
		.end    = IRQ_TCU_BASE,
	},
};

struct platform_device jz_tcu_device = {
	.name = "jz-tcu",
	.id = -1,
	.dev = {
		.platform_data = &jzpwm_pdata,
	},
	.resource = jz_tcu_resources,
	.num_resources = ARRAY_SIZE(jz_tcu_resources),
};
#endif
