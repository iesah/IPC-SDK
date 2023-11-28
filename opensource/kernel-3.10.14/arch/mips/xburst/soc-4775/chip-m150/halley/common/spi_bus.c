#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <mach/jzssi.h>
#include "board_base.h"


#ifdef CONFIG_MTD_JZ_SPI_NORFLASH

#define SIZE_BOOTLOADER     0x40000  /* 256k */
#define SIZE_KERNEL     0x640000  /* 4M */
#define SIZE_ROOTFS     0x180000  /* 8M */
//#define SIZE_RECOVERY 0x60000  /* 8M */

struct mtd_partition jz_mtd_partition1[] = {
	{
		.name =     "bootloader",
		.offset =   0,
		.size =     SIZE_BOOTLOADER,
	},
	{
		.name =     "kernel",
		.offset =   SIZE_BOOTLOADER,
		.size =     SIZE_KERNEL,
	},
	{
		.name =     "recovery",
		.offset =   SIZE_KERNEL + SIZE_BOOTLOADER,
		.size =     SIZE_ROOTFS,
	},
#if 0
	{
		.name =     "recovery",
		.offset =   SIZE_KERNEL + SIZE_BOOTLOADER + SIZE_ROOTFS,
		.size =     SIZE_RECOVERY,
	},
#endif
};
#endif
#ifdef CONFIG_JZ_SPI_NOR
struct spi_nor_block_info flash_block_info[] = {
	{
		.blocksize      = 64 * 1024,
		.cmd_blockerase = 0xD8,
		.be_maxbusy     = 1200,  /* 1.2s */
	},

	{
		.blocksize      = 32 * 1024,
		.cmd_blockerase = 0x52,
		.be_maxbusy     = 1000,  /* 1s */
	},
};

struct spi_nor_platform_data spi_nor_pdata = {
	.pagesize       = 256,
	.sectorsize     = 4 * 1024,
	.chipsize       = 8192 * 1024,
	.erasesize      = 4 * 1024,
	.id             = 0xc86017,

	.block_info     = flash_block_info,
	.num_block_info = ARRAY_SIZE(flash_block_info),

	.addrsize       = 3,
	.pp_maxbusy     = 3,            /* 3ms */
	.se_maxbusy     = 400,          /* 400ms */
	.ce_maxbusy     = 8 * 10000,    /* 80s */

	.st_regnum      = 3,

	.mtd_partition  = jz_mtd_partition1,
	.num_partition_info = ARRAY_SIZE(jz_mtd_partition1),
};


struct spi_board_info jz_spi0_board_info[]  = {

	[0] ={
		.modalias       =  "jz_spi_norflash",
		.platform_data          = &spi_nor_pdata,
		.controller_data        = (void *)GPIO_PA(23), /* cs for spi gpio */
		.max_speed_hz           = 12000000,
		.bus_num                = 0,
		.chip_select            = 0,

	},
	[1] ={
		.modalias       =  "jz_nor",
		.platform_data          = &spi_nor_pdata,
		.controller_data        = (void *)GPIO_PA(23), /* cs for spi gpio */
		.max_speed_hz           = 12000000,
		.bus_num                = 0,
		.chip_select            = 1,
	},
};
int jz_spi0_devs_size = ARRAY_SIZE(jz_spi0_board_info);
#endif

#ifdef CONFIG_JZ_SPI0
struct jz_spi_info spi0_info_cfg = {
	.chnl = 0,
	.bus_num = 0,
	.max_clk = 54000000,
	.num_chipselect = 2,
	.allow_cs_same  = 1,
	.chipselect     = {GPIO_PA(23),GPIO_PA(23)},
};
#endif

#ifdef CONFIG_SPI_GPIO
static struct spi_gpio_platform_data jz_spi_gpio_data = {

	.sck	= GPIO_SPI_SCK,
	.mosi	= GPIO_SPI_MOSI,
	.miso	= GPIO_SPI_MISO,
	.num_chipselect = 1,
};

struct platform_device jz_spi_gpio_device = {
	.name   = "spi_gpio",
	.dev    = {
		.platform_data = &jz_spi_gpio_data,
	},
};
#endif

