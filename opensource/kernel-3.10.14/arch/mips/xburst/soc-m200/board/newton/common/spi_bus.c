#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>

#include <mach/jzspi.h>
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
