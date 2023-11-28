#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>

#include <mach/jzssi.h>
#include "board_base.h"

#ifdef CONFIG_SPI0_V12_JZ
static struct spi_board_info jz_spi0_board_info[] = {
	[0] = {
		.modalias       = "spidev",
		.bus_num        = 0,
		.chip_select    = 0,
		.max_speed_hz   = 1200000,
	},
};

struct jz_spi_info spi0_info_cfg = {
	.chnl = 0,
	.bus_num = 0,
	.max_clk = 54000000,
	.num_chipselect = 2,
};
#endif

#ifdef CONFIG_SPI1_V12_JZ
static struct spi_board_info jz_spi1_board_info[] = {
	[0] = {
		.modalias       = "spidev",
		.bus_num        = 1,
		.chip_select    = 1,
		.max_speed_hz   = 120000,
	},
};

struct jz_spi_info spi1_info_cfg = {
	.chnl = 1,
	.bus_num = 1,
	.max_clk = 54000000,
	.num_chipselect = 2,
};
#endif

#if defined(CONFIG_SPI_GPIO)
static struct spi_gpio_platform_data jz4780_spi_gpio_data = {
	.sck	= GPIO_SPI_SCK,
	.mosi	= GPIO_SPI_MOSI,
	.miso	= GPIO_SPI_MISO,
	.num_chipselect	= 2,
};

static struct platform_device jz4780_spi_gpio_device = {
	.name	= "spi_gpio",
	.dev	= {
		.platform_data = &jz4780_spi_gpio_data,
	},
};

static struct spi_board_info jz_spi0_board_info[] = {
       [0] = {
	       .modalias       = "spidev",
	       .bus_num	       = 0,
	       .chip_select    = 0,
	       .max_speed_hz   = 120000,
       },
};
#endif

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

struct spi_nor_platform_data spi_nor_pdata[] = {
	{
		.name           = "GD25LQ128C",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 16384 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0xc86018,

		.block_info     = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize       = 3,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */

		.st_regnum      = 3,
	},
	{
		.name           = "IS25LP128",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 16384 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0x9d6018,

		.block_info     = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize       = 3,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */

		.st_regnum      = 3,
	},
	{
		.name           = "MX25L12835F",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 16384 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0xc22018,

		.block_info     = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize       = 3,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */

		.st_regnum      = 3,
	},
	{
		.name           = "MX25L6406F",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 8192 * 1024,
		.erasesize      = 64 * 1024,//4 * 1024,
		.id             = 0xc22017,

		.block_info     = flash_block_info,
		.num_block_info = ARRAY_SIZE(flash_block_info),

		.addrsize       = 3,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */

		.st_regnum      = 3,
	},
	{
			.name           = "WIN25Q128",
			.pagesize       = 256,
			.sectorsize     = 4 * 1024,
			.chipsize       = 16384 * 1024,
			.erasesize      = 32 * 1024,//4 * 1024,
			.id             = 0xef4018,

			.block_info     = flash_block_info,
			.num_block_info = ARRAY_SIZE(flash_block_info),

			.addrsize       = 3,
			.pp_maxbusy     = 3,            /* 3ms */
			.se_maxbusy     = 400,          /* 400ms */
			.ce_maxbusy     = 8 * 10000,    /* 80s */

			.st_regnum      = 3,
	}
};

struct jz_sfc_info sfc_info_cfg = {
	.chnl = 0,
	.bus_num = 0,
	.num_chipselect = 1,
	.board_info = spi_nor_pdata,
	.board_info_size = ARRAY_SIZE(spi_nor_pdata),
};


