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
#include "m80701.h"

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
	name:"ndsystem",
	offset:64 * 0x100000LL,
	size:512 * 0x100000LL,
	mode:1,
	eccbit:8,
	use_planes:ONE_PLANE,
	part_attrib:PART_SYSTEM,
	ex_partition:{{0},{0},{0},{0}}
	},
	{
	name:"nddata",
	offset:1124 * 0x100000LL,
	size:512 * 0x100000LL,
	mode:1,
	eccbit:8,
	use_planes:ONE_PLANE,
	part_attrib:PART_DATA,
	ex_partition:{{0},{0},{0},{0}}
	},
	;

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
	20,
	20,
	20,
};

#else
static struct platform_nand_partition partition_info[] = {
/*	{
                name:"NAND BOOT partition",
                offset:100 * 0x100000LL,
	        size:4 * 0x100000LL,
	        mode:0,
	        eccbit:16,
	        use_planes:ONE_PLANE,
	        part_attrib:PART_XBOOT
        },
	{
                name:"NAND KERNEL partition",
	        offset:104 * 0x100000LL,
	        size:4 * 0x100000LL,
	        mode:1,
	        eccbit:16,
	        use_planes:ONE_PLANE,
	        part_attrib:PART_KERNEL
        },
	{
                name:"NAND SYSTEM partition",
	        offset:108 * 0x100000LL,
	        size:504 * 0x100000LL,
	        mode:1,
	        eccbit:8,
	        use_planes:TWO_PLANES,
	        part_attrib:PART_SYSTEM
        },
*/
	{
            name:"ndsystem",
	        offset:64 * 0x100000LL,
	        size:512 * 0x100000LL,
	        mode:1,
	        eccbit:8,
	        use_planes:ONE_PLANE,
	        part_attrib:PART_SYSTEM
    },
	{
            name:"nddata",
	        offset:1124 * 0x100000LL,
	        size:512 * 0x100000LL,
	        mode:1,
	        eccbit:8,
	        use_planes:ONE_PLANE,
	        part_attrib:PART_DATA
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
//	2,			/* reserved blocks of mtd0 */
//	2,			/* reserved blocks of mtd1 */
//	10,			/* reserved blocks of mtd2 */
	20,			/* reserved blocks of mtd3 */
	20,			/* reserved blocks of mtd4 */
	20,			/* reserved blocks of mtd5 */
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
