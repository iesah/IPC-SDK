/*
 * jz47xx_spi_nor.h - jz47xx SPI NOR Flash header
 */
#ifndef __JZ47XX_SPI_NOR_H__
#define __JZ47XX_SPI_NOR_H__

#include <mach/jzssi.h>

/* SPI Flash Instructions */
#define CMD_WREN		0x06 /* Write Enable */
#define CMD_WRDI		0x04 /* Write Disable */
#define CMD_RDSR		0x05 /* Read Status Register */
#define CMD_WRSR		0x01 /* Write Status Register */
#define CMD_READ		0x03 /* Read Data */
#define CMD_FAST_READ		0x0B /* Read Data at high speed */
#define CMD_PP			0x02 /* Page Program(write data) */
#define CMD_SE			0x20 /* Sector Erase */
#define CMD_CE			0xC7 /* Bulk or Chip Erase */
#define CMD_DP			0xB9 /* Deep Power-Down */
#define CMD_RES			0xAB /* Release from Power-Down and Read Electronic Signature */
#define CMD_REMS		0x90 /* Read Manufacture ID/ Device ID */
#define CMD_RDID		0x9F /* Read Identification */

/* Status Register define, size: 1byte */
#define STATUS_WIP_LSB		0 /* write in progress's shift */
#define STATUS_WIP_MSK		(1 << STATUS_WIP_LSB) /* write in progress's mask */
#define STATUS_WIP      	(1 << STATUS_WIP_LSB) /* write in progress */
#define STATUS_WEL_LSB  	1 /* write enable latch's shift */
#define STATUS_WEL_MSK  	(1 << STATUS_WEL_LSB) /* write enable latch's mask */
#define STATUS_WEL      	(1 << STATUS_WEL_LSB) /* write enable latch's mask */
#define STATUS_BP_LSB   	2 /* block protect bit's shift */
#define STATUS_BP_MSK   	(3 << STATUS_BP_LSB)  /* block protect bit's mask */

#define STATUS_REG_LEN		1 /* Register length: 1 byte */

/**
 * struct jz_nor_packet
 */
struct jz_nor_start {
	u8 cmd;
	#define MAX_ADDR_SIZE	4
	u8 addr[MAX_ADDR_SIZE];
};

#define JZNOR_DEVICE_NAME		"jz_nor"

/**
 * struct jz_nor_local
 */
struct jz_nor_local {
	struct spi_device		*spi;

	struct spi_transfer		xfer[3];
	struct jz_nor_start		start;

	struct mutex			lock;

	u32 				pagesize;
	u32 				sectorsize;
	u32 				chipsize;

	/* MAX Busytime for page program, unit: ms */
	u32				pp_maxbusy;
	/* MAX Busytime for sector erase, unit: ms */
	u32				se_maxbusy;
	/* MAX Busytime for chip erase, unit: ms */
	u32				ce_maxbusy;

	struct spi_nor_block_info 	*block_info;
	int				num_block_info;

	int				addrsize; /* 2 | 3 */
	/* Flash status register num, Max support 3 register */
	int				statreg_num;

	/* NOR Flash operation */
	int (*read)(struct jz_nor_local *flash, size_t offset, u8 *buf, size_t length);
	int (*write)(struct jz_nor_local *flash, size_t offset, const u8 *buf, size_t length);
	int (*erase)(struct jz_nor_local *flash, size_t offset, size_t len);

	u8 (*get_status)(struct jz_nor_local *flash, int regnum);
};

static void inline spi_node_write_setup(struct spi_transfer *x, struct spi_device *spi,
					const void *tx_buf, int len)
{
	x->rx_buf		= NULL;
	x->tx_buf		= tx_buf;
	x->len			= len;
	x->cs_change		= 0; /* Drivers do NOT affect CS# */
	x->bits_per_word	= spi->bits_per_word;
	x->speed_hz		= spi->max_speed_hz;

}

static void inline spi_node_read_setup(struct spi_transfer *x, struct spi_device *spi,
					void *rx_buf, int len)
{
	x->rx_buf		= rx_buf;
	x->tx_buf		= NULL;
	x->len			= len;
	x->cs_change		= 0; /* Drivers do NOT affect CS# */
	x->bits_per_word	= spi->bits_per_word;
	x->speed_hz		= spi->max_speed_hz;
}

static void inline serial_flash_address(struct jz_nor_start *start, size_t address, int a_count)
{
	int shift, i;
	for (i = 0; i < a_count; i++) {
		shift = 8 * (a_count - 1 - i);
		start->addr[i] = (address >> shift) & 0xFF;
	}
}

#endif /* __JZ47XX_SPI_NOR_H__ */
