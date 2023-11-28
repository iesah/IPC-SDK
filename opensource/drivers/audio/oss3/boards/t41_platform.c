/*
 * Platform device support for Tomahawk series SoC.
 *
 * Copyright 2017, <xianghui.shen@ingenic.com>
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
#include <soc/gpio.h>
#include <soc/base.h>
#ifdef CONFIG_KERNEL_4_4_94
#include <dt-bindings/interrupt-controller/t40-irq.h>
#include <dt-bindings/dma/ingenic-pdma.h>
#endif
#ifdef CONFIG_KERNEL_3_10
#include <soc/irq.h>
#include <mach/platform.h>
#include <mach/jzdma.h>
#endif
static void audio_release_dmic_fmtcov_device(struct device *dev)
{
	return;
}

static void audio_release_dmic_dbus_device(struct device *dev)
{
	return;
}

static void audio_release_dmic_dma_device(struct device *dev)
{
	return;
}

static void audio_release_dmic_device(struct device *dev)
{
	return;
}

static void audio_release_codec_device(struct device *dev)
{
	return;
}

static void audio_release_aic_device(struct device *dev)
{
	return;
}

static void audio_release_device(struct device *dev)
{
	return;
}

/* inner codec device */
static struct resource jz_codec_resources[] = {
	[0] = {
		.start  = CODEC_IOBASE,
		.end    = CODEC_IOBASE + 0x130,
		.flags  = IORESOURCE_MEM,
	},
};

struct platform_device jz_codec_device = {
	.name   = "jz-inner-codec",
	.id = -1,
	.dev = {
		.release = audio_release_codec_device,
	},
	.resource   = jz_codec_resources,
	.num_resources  = ARRAY_SIZE(jz_codec_resources),
};


/* aic device */
static u64 jz_aic_dmamask = ~(u32) 0;
static struct resource jz_aic_resources[] = {
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

struct platform_device jz_aic_device = {
	.name = "jz-aic",
	.id = -1,
	.dev = {
		.dma_mask = &jz_aic_dmamask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = (void *)&jz_codec_device,
		.release = audio_release_aic_device,
	},
	.resource = jz_aic_resources,
	.num_resources = ARRAY_SIZE(jz_aic_resources),
};

/* dmic fmtcov device */
static struct resource dmic_fmtcov_resources[] = {
	[0] = {
		.start  = 0x134d2000,
		.end    = 0x134d2000 + 0x28 - 1,
		.flags  = IORESOURCE_MEM,
	},
};
struct platform_device dmic_fmtcov_device = {
	.name = "dmic-fmtcov",
	.id = -1,
	.dev = {
		.release = audio_release_dmic_fmtcov_device,
	},
	.resource = dmic_fmtcov_resources,
	.num_resources = ARRAY_SIZE(dmic_fmtcov_resources),
};

/* dmic dbus device */
static struct resource dmic_dbus_resources[] = {
	[0] = {
		.start  = 0x134d4000,
		.end    = 0x134d4000 + 0x30 - 1,
		.flags  = IORESOURCE_MEM,
	},
};
struct platform_device dmic_dbus_device = {
	.name = "dmic-dbus",
	.id = -1,
	.dev = {
		.release = audio_release_dmic_dbus_device,
	},
	.resource = dmic_dbus_resources,
	.num_resources = ARRAY_SIZE(dmic_dbus_resources),
};

/* dmic dma device */
static struct resource dmic_dma_resources[] = {
	[0] = {
		.start  = 0x134d0000,
		.end    = 0x134d0000 + 0x114 - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = 0x134d1000,
		.end    = 0x134d1000 + 0x100 - 1,
		.flags  = IORESOURCE_MEM,
	},
	[2] = {
		.start = 33+8,//IRQ_DMIC,
		.end   = 33+8 ,//IRQ_DMIC,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device dmic_dma_device = {
	.name = "dmic-dma",
	.id = -1,
	.dev = {
		.release = audio_release_dmic_dma_device,
	},
	.resource = dmic_dma_resources,
	.num_resources = ARRAY_SIZE(dmic_dma_resources),
};

struct platform_device *dmic_as_device[] = {
	&dmic_dma_device,
	&dmic_dbus_device,
	&dmic_fmtcov_device,
};


/* dmic device */
static struct resource jz_dmic_resource[] = {
	[0] = {
		.start = 0x134da000,//DMIC_IOBASE,
		.end = 0x134da000 + 0x10 - 1,
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device jz_dmic_device = {
	.name             = "jz-dmic",
	.id               = -1,
	.dev = {
		.platform_data = (void *)dmic_as_device,
		.release = audio_release_dmic_device,
	},
	.num_resources    = ARRAY_SIZE(jz_dmic_resource),
	.resource         = jz_dmic_resource,
};

struct platform_device *audio_devices[] = {
	&jz_aic_device,
	&jz_dmic_device,
	NULL,
};

struct platform_device audio_dsp_platform_device = {
	.name             = "jz-dsp",
	.id               = -1,
	.dev = {
		.dma_mask = &jz_aic_dmamask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = (void *)audio_devices,
		.release = audio_release_device,
	},
	.num_resources = 0,
};
