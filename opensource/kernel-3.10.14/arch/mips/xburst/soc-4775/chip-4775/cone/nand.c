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
#include "board.h"

#define ECCBIT 12

static struct platform_nand_partition partition_info[] = {
	{
	name:"ndxboot",
	offset:0 * 0x100000LL,
	size:6 * 0x100000LL,
	mode:SPL_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_XBOOT
	},
	{
	name:"ndboot",
	offset:6 * 0x100000LL,
	size:12 * 0x100000LL,
	mode:DIRECT_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_KERNEL
	},
    {
    name:"ndrecovery",
    offset:18 * 0x100000LL,
    size:10 * 0x100000LL,
    mode:DIRECT_MANAGER,
    eccbit:ECCBIT,
    use_planes:ONE_PLANE,
    part_attrib:PART_KERNEL
    },
	{
	name:"ndsystem",
	offset:28 * 0x100000LL,
	size:320 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_SYSTEM
    },
	{
	name:"nddata",
	offset:348 * 0x100000LL,
	size:125 * 0x100000LL,
	mode:ZONE_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_DATA
    },
	{
    name:"ndcache",
    offset:473 * 0x100000LL,
    size:10 * 0x100000LL,
    mode:ZONE_MANAGER,
    eccbit:ECCBIT,
    use_planes:ONE_PLANE,
    part_attrib:PART_KERNEL
    },
	{
	name:"ndapanic",
	offset:483 * 0x100000LL,
	size:3 * 0x100000LL,
	mode:DIRECT_MANAGER,
	eccbit:ECCBIT,
	use_planes:ONE_PLANE,
	part_attrib:PART_MISC
    },
	{
	name:"ndmisc",
	offset:486 * 0x100000LL,
	size:26 * 0x100000LL,
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
	8,			/* reserved blocks of ndcache */
	1,			/* reserved blocks of nderror */
	8,			/* reserved blocks of ndmisc */
};

struct platform_nand_data jz_nand_chip_data = {
	.nr_partitions = ARRAY_SIZE(partition_info),
	.partitions = partition_info,
	/* there is no room for bad block info in struct platform_nand_data */
	/* so we have to use chip.priv */
	.priv = &partition_reserved_badblocks,
	/* gpio_wp is a MUST! JZ4775 do NOT have PC31, so we can safely use it as an invalid pin */
        .gpio_wp = GPIO_PA(23),
};
