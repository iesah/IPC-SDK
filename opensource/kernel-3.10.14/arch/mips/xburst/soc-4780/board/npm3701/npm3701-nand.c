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
#include "npm3701.h"

#define ECCBIT 24

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
	name:"ndxboot",
	offset:0 * 0x100000LL,
	size:8 * 0x100000LL,
	mode:SPL_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_XBOOT,
	ex_partition:{{0},{0},{0},{0}}
	},
	{
	name:"ndboot",
	offset:8 * 0x100000LL,
	size:16 * 0x100000LL,
	mode:DIRECT_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_KERNEL,
	ex_partition:{{0},{0},{0},{0}}
	},
	{
	name:"ndrecovery",
	offset:24 * 0x100000LL,
	size:16 * 0x100000LL,
	mode:DIRECT_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_KERNEL,
	ex_partition:{{0},{0},{0},{0}}
	},
	{
	name:"ndsystem",
	offset:64 * 0x100000LL,
	size:512 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_SYSTEM,
	ex_partition:{{0},{0},{0},{0}}
	},
	{
	name:"ndcache",
	offset:576 * 0x100000LL,
	size:128 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_MISC,
	ex_partition:{{0},{0},{0},{0}}
	},
	{
	name:"ndextern",
	offset:704 * 0x100000LL,
	size:3392 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_MISC,
	ex_partition:{
		{
		name:"nddata",
		offset:704 * 0x100000LL,
		size:1024 * 0x100000LL,
		},
		{
		name:"ndmisc",
		offset:1728 * 0x100000LL,
		size:2368 * 0x100000LL,
		}
	    }
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
	4,			/* reserved blocks of ndxboot */
	8,			/* reserved blocks of ndboot */
	8,			/* reserved blocks of ndrecovery */
	32,			/* reserved blocks of ndsystem */
	36,			/* reserved blocks of ndcache */
	256,			/* reserved blocks of ndextern */
	1,			/* reserved blocks of nderror */
};

#else

static struct platform_nand_partition partition_info[] = {
	{
	name:"ndxboot",
	offset:0 * 0x100000LL,
	size:8 * 0x100000LL,
	mode:SPL_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_XBOOT
	},
	{
	name:"ndboot",
	offset:8 * 0x100000LL,
	size:16 * 0x100000LL,
	mode:DIRECT_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_KERNEL
	},
    {
    name:"ndrecovery",
    offset:24 * 0x100000LL,
    size:16 * 0x100000LL,
    mode:DIRECT_MANAGER,
    eccbit:ECCBIT,
    use_planes:ONE_PLANE,
    part_attrib:PART_KERNEL
    },
	{
	name:"ndsystem",
	offset:64 * 0x100000LL,
	size:512 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_SYSTEM
    },
	{
	name:"nddata",
	offset:576 * 0x100000LL,
	size:1024 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_DATA
    },
	{
    name:"ndcache",
    offset:1600 * 0x100000LL,
    size:128 * 0x100000LL,
    mode:ZONE_MANAGER,
    eccbit:ECCBIT,
    use_planes:ONE_PLANE,
    part_attrib:PART_KERNEL
    },
	{
	name:"ndmisc",
	offset:1728 * 0x100000LL,
	size:2368 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_MISC
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
	4,			/* reserved blocks of ndxboot */
	8,			/* reserved blocks of ndboot */
	8,			/* reserved blocks of ndrecovery */
	32,			/* reserved blocks of ndsystem */
	64,			/* reserved blocks of nddata */
	16,			/* reserved blocks of ndcache */
	128,		/* reserved blocks of ndmisc */
	1,			/* reserved blocks of nderror */
};
#endif
struct platform_nand_data jz_nand_chip_data = {
	.nr_partitions = ARRAY_SIZE(partition_info),
	.partitions = partition_info,
	/* there is no room for bad block info in struct platform_nand_data */
	/* so we have to use chip.priv */
	.priv = &partition_reserved_badblocks,
        .gpio_wp = GPIO_PF(22),
};
