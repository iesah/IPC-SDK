/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  NAND-MTD support template
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <gpio.h>

#include <mach/jz4780_nand.h>

#define DRVNAME "jz4780-nand"

#define GPIO_BUSY0       GPIO_PA(20)
#define GPIO_WP          GPIO_PF(22)

#define SIZE_MB          (1024 * 1024LL)
#define SIZE_ALL         (1024 * SIZE_MB)

#define OFFSET_XBOOT     (0)
#define SIZE_XBOOT       (8      * SIZE_MB)

#define OFFSET_BOOT      (SIZE_XBOOT)
#define SIZE_BOOT        (16     * SIZE_MB)

#define OFFSET_SYSTEM    (OFFSET_BOOT + SIZE_BOOT)
#define SIZE_SYSTEM      (320    * SIZE_MB)

#define OFFSET_DATA      (OFFSET_SYSTEM + SIZE_SYSTEM)
#define SIZE_DATA        (512   * SIZE_MB)

#define OFFSET_CACHE     (OFFSET_DATA + SIZE_DATA)
#define SIZE_CACHE       (48    * SIZE_MB)

#define OFFSET_RECOVERY  (OFFSET_CACHE + SIZE_CACHE)
#define SIZE_RECOVERY    (16     * SIZE_MB)

#define OFFSET_MISC      (OFFSET_RECOVERY + SIZE_RECOVERY)
#define SIZE_MISC        (16     * SIZE_MB)

#define OFFSET_UDISK     (OFFSET_MISC + SIZE_MISC)
#define SIZE_UDISK       (SIZE_ALL - OFFSET_UDISK)


static struct mtd_partition parts[] = {
	{
		.name = "xboot",
		.offset = OFFSET_XBOOT,
		.size = SIZE_XBOOT,
	}, {
		.name = "boot",
		.offset = OFFSET_BOOT,
		.size = SIZE_BOOT,
	}, {
		.name = "system",
		.offset = OFFSET_SYSTEM,
		.size = SIZE_SYSTEM,
	}, {
		.name = "data",
		.offset = OFFSET_DATA,
		.size = SIZE_DATA,
	}, {
		.name = "cache",
		.offset = OFFSET_CACHE,
		.size = SIZE_CACHE,
	}, {
		.name = "recovery",
		.offset = OFFSET_RECOVERY,
		.size = SIZE_RECOVERY,
	}, {
		.name = "misc",
		.offset = OFFSET_MISC,
		.size = SIZE_MISC,
	}, {
		.name = "udisk",
		.offset = OFFSET_UDISK,
		.size = SIZE_UDISK,
	}
};

static nand_flash_if_t nand_interfaces[] = {
	{ COMMON_NAND_INTERFACE(1, GPIO_BUSY0, 1, GPIO_WP, 1) },
};
#if 0
static nand_flash_info_t board_support_nand_info_table[] = {
	#define NAND_FLASH_K9K8G08U0D_NAME           "K9K8G08U0D"
	#define NAND_FLASH_K9K8G08U0D_ID             0xd3

	#define NAND_FLASH_K9GBG08U0A_NANE           "K9GBG08U0A"
	#define NAND_FLASH_K9GBG08U0A_ID             0xd7

	#define NAND_FLASH_MT29F32G08CBACAWP_NAME    "MT29F32G08CBACAWP"
	#define NAND_FLASH_MT29F32G08CBACAWP_ID      0x68

	{
		/*
		 * Datasheet of K9K8G08U0D, Rev-0.2, P4, S1.2
		 * ECC : 1bit/528Byte
		 */
		COMMON_NAND_CHIP_INFO(
			NAND_FLASH_K9K8G08U0D_NAME, NAND_FLASH_K9K8G08U0D_ID,
			1024, 8 /* it should be 8 at least for sw-bch */,
			/*
			 * all timings adjust to +10ns
			 *
			 * we change this parameters
			 * because mtd_torturetest failed
			 * ******************************
			 */
			10,
			/*
			 * TODO: To be modified carefully by datasheet
			 * ******************************
			 */
			12, 5, 12, 5, 20, 5, 12, 5, 12, 10,
			25, 25, 300, 100, 100, 300, 12, 20, 300, 100,
			100, 200 * 1000, 1 * 1000, 200 * 1000,
			5 * 1000 * 1000, 0, BUS_WIDTH_8,
			NAND_OUTPUT_UNDER_DRIVER1,
			CAN_NOT_ADJUST_RB_DOWN_STRENGTH,
			NULL)
	},

	{
		/*
		 * Datasheet of K9GBG08U0A, Rev-1.3, P5, S1.2
		 * ECC : 24bit/1KB
		 */
		COMMON_NAND_CHIP_INFO(
			NAND_FLASH_K9GBG08U0A_NANE, NAND_FLASH_K9GBG08U0A_ID,
			1024, 32,
			/*
			 * all timings adjust to +10ns
			 *
			 * we change this parameters
			 * because mtd_torturetest failed
			 * ******************************
			 */
			10,
			/*
			 * ******************************
			 */
			12, 5, 12, 5, 20, 5, 12, 5, 12, 10,
			25, 25, 300, 100, 100, 300, 12, 20, 300, 100,
			100, 200 * 1000, 1 * 1000, 200 * 1000,
			5 * 1000 * 1000, 0, BUS_WIDTH_8,
			NAND_OUTPUT_UNDER_DRIVER1,
			CAN_NOT_ADJUST_RB_DOWN_STRENGTH,
			samsung_nand_pre_init)
	},

	{
		/*
		 * Datasheet of MT29F32G08CBACA(WP), Rev-E, P109, Table-17
		 * ECC : 24bit/1
		 */
		COMMON_NAND_CHIP_INFO(
			NAND_FLASH_MT29F32G08CBACAWP_NAME,
			NAND_FLASH_MT29F32G08CBACAWP_ID,
			1024, 32, 0,
			10, 5, 10, 5, 15, 5, 7, 5, 10, 7,
			20, 20, 70, 100, 60, 200, 10, 20, 0, 100,
			100, 100 * 1000, 0, 0, 0, 5, BUS_WIDTH_8,
			NAND_OUTPUT_UNDER_DRIVER1,
			NAND_RB_DOWN_FULL_DRIVER,
			micron_nand_pre_init)
	},
};
#endif
static struct jz4780_nand_platform_data nand_pdata = {
	.part_table = parts,
	.num_part = ARRAY_SIZE(parts),

	.nand_flash_if_table = nand_interfaces,
	.num_nand_flash_if = ARRAY_SIZE(nand_interfaces),

	/*
	 * only single thread soft BCH ECC have
	 * already got 20% speed improvement.
	 *
	 * TODO:
	 * does some guys who handle ECC hardware implementation
	 * to look at kernel soft BCH codes.
	 *
	 * I got the patch commits here:
	 *
	 * http://lists.infradead.org/pipermail/linux-mtd/2011-February/033846.html
	 *
	 * and a benchmark of "kernel soft BCH algorithm" VS "Chien search" on that page.
	 *
	 */
	.ecc_type = NAND_ECC_TYPE_SW,
//	.ecc_type = NAND_ECC_TYPE_HW,

	/*
	 * use polled type cause speed gain
	 * is about 10% ~ 15%
	 *
	 * use DMA cause speed gain is about 14%
	 */
	.xfer_type = NAND_XFER_DMA_POLL,


	/*
	 * use board specific NAND timings because the timings
	 * need adjust for this board
	 *
	 * the NAND timings match schema is
	 *
	 * First,  fetch NAND timings from following table if it's exist
	 * Second, if board specific codes did not provide following table
	 *         or driver can not find any matched information from
	 *         following table, it will try to match information from driver
	 *         built-in NAND timings table, if also can not match anything,
	 *         the NAND timings match will fail.
	 */
	//.nand_flash_info_table = board_support_nand_info_table,
	//.num_nand_flash_info = ARRAY_SIZE(board_support_nand_info_table),
};

static struct platform_device nand_dev = {
	.name = DRVNAME,
	.id = 0,
};

static __init int nand_mtd_device_register(void)
{
	printk("====bch ecc size = 1024===\n");
	platform_device_add_data(&nand_dev, &nand_pdata, sizeof(nand_pdata));
	return platform_device_register(&nand_dev);
}
arch_initcall(nand_mtd_device_register);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fighter Sun <wanmyqawdr@126.com>");
MODULE_DESCRIPTION("NAND-MTD board specific support template for JZ4780 SoC");
