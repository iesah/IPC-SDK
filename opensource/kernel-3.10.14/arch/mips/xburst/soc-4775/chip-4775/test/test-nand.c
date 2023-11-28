/*
 * board/test/test-nand.c
 *
 * JZ4780 Pisces  board setup routines.
 *
 * Copyright (c) 2006-2015  Ingenic Semiconductor Inc.
 *
 * Author: Jason<xwang@ingenic>
 *		Based on board-altair.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
 
#include <soc/gpio.h>
#include <soc/base.h>
#include <soc/irq.h>
 
#include <mach/platform.h>

#include <mach/jznand.h>
#include "test.h"

/**
 * WARNING!!! DANGER! DANGER! DANGER!
 * modify the partition table should be careful,
 * it may produce effects on burn tools, both usb burn tool
 * and card burn tool should be check and make the appropriate
 * changes if modified this.
 **/
#ifdef CONFIG_MUL_PARTS
static struct platform_nand_partition partition_info[] = {
	{
	name:"ndisk",
	offset:612 * 0x100000LL,
	size:128 * 0x100000LL,
	mode:1,
	eccbit:8,
	use_planes:TWO_PLANES_PLANE,
	part_attrib:PART_DATA,
	ex_partition:{{0},{0},{0},{0}}
	},
	{
	name:"mdisk",
	offset:1124 * 0x100000LL,
	size:128 * 0x100000LL,
	mode:1,
	eccbit:8,
	use_planes:TWO_PLANES,
	part_attrib:PART_DATA,
	ex_partition:{{0},{0},{0},{0}}
	},
};

/* Define max reserved bad blocks for each partition.
 * This is used by the mtdblock-jz.c NAND FTL driver only.
 *
 * The NAND FTL driver reserves some good blocks which can't be
 * seen by the upper layer. When the bad block number of a partition
 * exceeds the max reserved blocks, then there is no more reserved
 * good blocks to be used by the NAND FTL driver when another bad
 * block generated.
 */
static int partition_reserved_badblocks[] = {
	20,			/* reserved blocks of mtd3 */
	20,			/* reserved blocks of mtd4 */
	4			/* reserved blocks of mtd5 */
};

#else

/*-----------------------------------------------------------------------------*/
/*                     NAND partitions definitions                             */
/*-----------------------------------------------------------------------------*/

static struct platform_nand_partition partition_info[] = {
	/*{name:"NAND BOOT partition",
	 offset:100 * 0x100000LL,
	 size:4 * 0x100000LL,
	 mode:0,
	 eccbit:16,
	 use_planes:ONE_PLANE,
	 part_attrib:PART_XBOOT},
	{name:"NAND KERNEL partition",
	 offset:104 * 0x100000LL,
	 size:4 * 0x100000LL,
	 mode:1,
	 eccbit:16,
	 use_planes:ONE_PLANE,
	 part_attrib:PART_KERNEL},
	{name:"NAND SYSTEM partition",
	 offset:108 * 0x100000LL,
	 size:504 * 0x100000LL,
	 mode:1,
	 eccbit:8,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_SYSTEM},*/
	{name:"ndisk",
	 offset:612 * 0x100000LL,
	 size:128 * 0x100000LL,
	 mode:1,
	 eccbit:8,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_DATA},
	{name:"mdisk",
	 offset:1124 * 0x100000LL,
	 size:128 * 0x100000LL,
	 mode:1,
	 eccbit:8,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_MISC},
};

/* Define max reserved bad blocks for each partition.
 * This is used by the mtdblock-jz.c NAND FTL driver only.
 *
 * The NAND FTL driver reserves some good blocks which can't be
 * seen by the upper layer. When the bad block number of a partition
 * exceeds the max reserved blocks, then there is no more reserved
 * good blocks to be used by the NAND FTL driver when another bad
 * block generated.
 */
static int partition_reserved_badblocks[] = {
//	2,			/* reserved blocks of mtd0 */
//	2,			/* reserved blocks of mtd1 */
//	10,			/* reserved blocks of mtd2 */
	20,			/* reserved blocks of mtd3 */
	20,			/* reserved blocks of mtd4 */
	4			/* reserved blocks of mtd5 */
};

#if 0				/* CONFIG_JZ4770_PISCES */

/* Reserve 32MB for bootloader, splash1, splash2 and radiofw */
#define NAND_BLOCK_SIZE		    (2048*64)

#define DEVICE_NAND_SPL_OFFSET	( 0 * 0x100000LL)     // X-boot stage 1.
#define DEVICE_XBOOT_OFFSET	    ( 2 * NAND_BLOCK_SIZE)// X-boot stage 2.
#define DEVICE_ID_OFFSET	    (16 * 0x100000LL)     // Device ID stored at addr 16MB
#define MISC_OFFSET		        (32  * 0x100000LL)

#define DEVICE_NAND_SPL_SIZE  	(DEVICE_XBOOT_OFFSET)
//#define DEVICE_XBOOT_SIZE	    (16 * 0x100000LL - DEVICE_XBOOT_OFFSET)
#define DEVICE_XBOOT_SIZE	    (16 * 0x100000LL)
#define DEVICE_ID_SIZE		    (16 * 0x100000LL)
#define MISC_SIZE		        ( 1 * 0x100000LL)
#define RECOVERY_SIZE		    ( 5 * 0x100000LL)
#define BOOT_SIZE		        ( 4 * 0x100000LL)
#define SYSTEM_SIZE		        ((90 + 45+10+80) * 0x100000LL)
#define USERDATA_SIZE		    ((90 - 45+128-10 +20) * 0x100000LL)
#define CACHE_SIZE	        	(32 * 0x100000LL)
#define KPANIC_SIZE             ( 2 * 0x100000LL)
#define STORAGE_SIZE		    (MTDPART_SIZ_FULL)

static struct platform_nand_partition partition_info[] = {
	/*
	 * misc@mtd0 :      raw
	 * recovery@mtd1:   raw
	 * boot@mtd2:       raw
	 * system@mtd3:     yaffs2
	 * userdata@mtd:   yaffs2
	 * cache@mtd5:      yaffs2
	 * storage@mtd6:    vfat
	 */
	{name:       "misc",
	 offset:     MISC_OFFSET,
	 size:       MISC_SIZE,
	 mode:0,
	 eccbit:4,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_MISC
	},
	{name:       "kernel",
	 offset:     (MISC_OFFSET+MISC_SIZE+RECOVERY_SIZE),
	 size:       BOOT_SIZE,
	 mode:0,
	 eccbit:4,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_KERNEL
	},
	{name:       "recovery",
	 offset:     (MISC_OFFSET+MISC_SIZE),
	 size:       RECOVERY_SIZE,
	 mode:0,
	 eccbit:4,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_RECOVERY
	},
	{name:       "system",
	 offset:     (MISC_OFFSET+MISC_SIZE+RECOVERY_SIZE+BOOT_SIZE),
	 size:       SYSTEM_SIZE,
	 mode:0,
	 eccbit:4,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_SYSTEM
	},
	{name:       "data",
	 offset:     (MISC_OFFSET+MISC_SIZE+RECOVERY_SIZE+BOOT_SIZE+SYSTEM_SIZE),
	 size:       USERDATA_SIZE,
	 mode:0,
	 eccbit:4,
	 use_planes:TWO_PLANES,
	 part_attrib:PART_DATA
	},
	{name:       "xboot",
	 offset:     (DEVICE_NAND_SPL_OFFSET),
	 size:       (DEVICE_XBOOT_SIZE),
	 mode:0,
	 eccbit:4,
	 use_planes:ONE_PLANE,
	 part_attrib:PART_XBOOT
	}
};

/* Define max reserved bad blocks for each partition.
 * This is used by the mtdblock-jz.c NAND FTL driver only.
 *
 * The NAND FTL driver reserves some good blocks which can't be
 * seen by the upper layer. When the bad block number of a partition
 * exceeds the max reserved blocks, then there is no more reserved
 * good blocks to be used by the NAND FTL driver when another bad
 * block generated.
 */
static int partition_reserved_badblocks[] = {
	10,			/* reserved blocks of mtd0 */
	10,			/* reserved blocks of mtd1 */
	10,			/* reserved blocks of mtd2 */
	10,			/* reserved blocks of mtd3 */
	10,			/* reserved blocks of mtd4 */
	10,			/* reserved blocks of mtd5 */
};

#endif				/* CONFIG_NAND */
#endif
struct platform_nand_data jz_nand_chip_data = {
	.nr_partitions = ARRAY_SIZE(partition_info),
	.partitions = partition_info,
	/* there is no room for bad block info in struct platform_nand_data */
	/* so we have to use chip.priv */
	.priv = &partition_reserved_badblocks,
};
