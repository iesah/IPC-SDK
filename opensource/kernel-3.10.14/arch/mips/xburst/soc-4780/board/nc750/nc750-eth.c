/*
 *  [board]-eth.c - This file defines Ethernet devices on the board.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <linux/syscore_ops.h>
#include <irq.h>

#include <mach/platform.h>
#include <soc/base.h>
#include <soc/irq.h>
#include "nc750.h"

static struct resource ax88796c_resource[] = {
	[0] = {
		.start = NEMC_CS5_IOBASE,		/* Start of AX88796C base address */
		.end   = NEMC_CS5_IOBASE + 0x3f,	/* End of AX88796C base address */
		.flags = IORESOURCE_MEM,
	},

	[1] = {
		.start = NEMC_IOBASE,
		.end   = NEMC_IOBASE + 0x50,
		.flags = IORESOURCE_MEM,
	},

	[2] = {
		.start = AX_ETH_INT,			/* Interrupt line number */
		.flags = IORESOURCE_IRQ,
	},

	[3] = {
		.name  = "reset_pin",
		.start = AX_ETH_RESET,			/* Reset line number */
		.end   = AX_ETH_RESET,
		.flags = IORESOURCE_IO,
	},
};

struct platform_device net_device_ax88796c = {
	.name  = "ax88796c",
	.id  = -1,
	.num_resources = ARRAY_SIZE(ax88796c_resource),
	.resource = ax88796c_resource,
};
