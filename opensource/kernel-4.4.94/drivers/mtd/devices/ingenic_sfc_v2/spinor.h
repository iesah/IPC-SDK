#ifndef __SPINOR_H
#define __SPINOR_H
#include "sfc_flash.h"
#include "spinor_cmd.h"

#define SPIFLASH_PARAMER_OFFSET 0x3c00
#define NOR_MAJOR_VERSION_NUMBER	2
#define NOR_MINOR_VERSION_NUMBER	0
#define NOR_REVERSION_NUMBER	0
#define NOR_VERSION		(NOR_MAJOR_VERSION_NUMBER | (NOR_MINOR_VERSION_NUMBER << 8) | (NOR_REVERSION_NUMBER << 16))


#define SIZEOF_NAME			32

#define NOR_MAGIC	0x726f6e	//ascii "nor"
#define NOR_PART_NUM	10
#define NORFLASH_PART_RW	0
#define NORFLASH_PART_WO	1
#define NORFLASH_PART_RO	2

/* the max number of DMA Descriptor */
#define DESC_MAX_NUM	64


struct spi_nor_cmd_info {
	unsigned short cmd;
	unsigned char dummy_byte;
	unsigned char addr_nbyte;
	unsigned char transfer_mode;

};

struct spi_nor_st_info {
	unsigned short cmd;
	unsigned char bit_shift;
	unsigned char mask;
	unsigned char val;
	unsigned char len; //length of byte to operate from register
	unsigned char dummy;
};

struct spi_nor_info {
	unsigned char name[32];
	unsigned int id;

	struct spi_nor_cmd_info read_standard;
	struct spi_nor_cmd_info read_quad;

	struct spi_nor_cmd_info write_standard;
	struct spi_nor_cmd_info write_quad;

	struct spi_nor_cmd_info sector_erase_32k;
	struct spi_nor_cmd_info sector_erase_64k;

	struct spi_nor_cmd_info wr_en;
	struct spi_nor_cmd_info en4byte;
	struct spi_nor_cmd_info enotp;
	struct spi_nor_cmd_info exotp;
	struct spi_nor_st_info	quad_set;
	struct spi_nor_st_info	quad_get;
	struct spi_nor_st_info	busy;

	unsigned short quad_ops_mode;
	unsigned short addr_ops_mode;
	unsigned short otp_ops_mode;

	unsigned int tCHSH;      //hold
	unsigned int tSLCH;      //setup
	unsigned int tSHSL_RD;   //interval
	unsigned int tSHSL_WR;

	unsigned int chip_size;
	unsigned int page_size;
	unsigned int erase_size;
	unsigned int addr_len;

	unsigned char chip_erase_cmd;
};

struct nor_partition {
	char name[32];
	uint32_t size;
	uint32_t offset;
	uint32_t mask_flags;//bit 0-1 mask the partiton RW mode, 0:RW  1:W  2:R
	uint32_t manager_mode;
};

struct norflash_partitions {
	struct nor_partition nor_partition[NOR_PART_NUM];
	uint32_t num_partition_info;
};

struct nor_private_data {
	unsigned int fs_erase_size;
	unsigned char uk_quad;
};


struct burner_params {
	uint32_t magic;
	uint32_t version;
	struct spi_nor_info spi_nor_info;
	struct norflash_partitions norflash_partitions;
	struct nor_private_data nor_pri_data;
	unsigned int spi_nor_info_size;
};

struct spi_nor_flash_ops {
	int (*set_4byte_mode)(struct sfc_flash *flash);
	int (*set_quad_mode)(struct sfc_flash *flash);
};

struct spinor_flashinfo {

	int quad_succeed;
	struct spi_nor_flash_ops *nor_flash_ops;
	struct spi_nor_info *nor_flash_info;
	struct spi_nor_cmd_info *cur_r_cmd;
	struct spi_nor_cmd_info *cur_w_cmd;
	struct norflash_partitions *norflash_partitions;
	struct nor_private_data *nor_pri_data;

};

/* SFC CDT Maximum INDEX number */
#define INDEX_MAX_NUM 32

enum {
	/* 1. nor reset */
	NOR_RESET_ENABLE,
	NOR_RESET,

	/* 2. nor read id */
	NOR_READ_ID,

	/* 3. nor get status */
	NOR_GET_STATUS,
	NOR_GET_STATUS_1,
	NOR_GET_STATUS_2,

	/* 4. nor singleRead */
	NOR_READ_STANDARD,

	/* 5. nor quadRead */
	NOR_READ_QUAD,

	/* 6. nor writeStandard */
	NOR_WRITE_STANDARD_ENABLE,
	NOR_WRITE_STANDARD,
	NOR_WRITE_STANDARD_FINISH,

	/* 7. nor writeQuad */
	NOR_WRITE_QUAD_ENABLE,
	NOR_WRITE_QUAD,
	NOR_WRITE_QUAD_FINISH,

	/* 8. nor erase */
	NOR_ERASE_WRITE_ENABLE_32K,
	NOR_ERASE_32K,
	NOR_ERASE_FINISH_32K,
	NOR_ERASE_WRITE_ENABLE_64K,
	NOR_ERASE_64K,
	NOR_ERASE_FINISH_64K,

	/* 9. quad mode */
	NOR_QUAD_SET_ENABLE,
	NOR_QUAD_SET,
	NOR_QUAD_FINISH,
	NOR_QUAD_GET,

	/* 10. nor write ENABLE */
	NOR_WRITE_ENABLE,

	/* 11. entry 4byte mode */
	NOR_EN_4BYTE,

	/* index count */
	NOR_MAX_INDEX,

	/*need ecter otp mode to set quad mode*/
	NOR_ENTER_OTP,
	NOR_OTP_QUAD_SET_ENABLE,
	NOR_OTP_QUAD_SET,
	NOR_OTP_QUAD_FINISH,
	NOR_OTP_QUAD_GET,
	NOR_EXIT_OTP,

};

__attribute__((unused)) static struct spi_nor_info spi_nor_info_table[] = {
	{
		.name = "GD25LQ128C",
		.id = 0xc86018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "EN25QH64A",
		.id = 0x1c7017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		/* otp_ops_mode: set quad mode need enter otp, set 1 */
		.otp_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "EN25QH128A",
		.id = 0x1c7018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		/* otp_ops_mode: set quad mode need enter otp, set 1 */
		.otp_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "EN25QH256A",
		.id = 0x1c7019,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		/* otp_ops_mode: set quad mode need enter otp, set 1 */
		.otp_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 32 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "EN25QX128A",
		.id = 0x1c7118,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "GD25Q64C",
		.id = 0xc84017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "GD25Q127C",
		.id = 0xc84018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "GD25Q256D/E",
		.id = 0xc84019,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 32 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "IS25LP064D",
		.id = 0x9d6017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "IS25LP128F",
		.id = 0x9d6018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "MX25L6406F",
		.id = 0xc22017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 3,
		.tSLCH = 3,
		.tSHSL_RD = 7,
		.tSHSL_WR = 30,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "MX25L12835F",
		.id = 0xc22018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 3,
		.tSLCH = 3,
		.tSHSL_RD = 7,
		.tSHSL_WR = 30,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "WIN25Q64",
		.id = 0xef4017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "WIN25Q128",
		.id = 0xef4018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XM25QH64A",
		.id = 0x207017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XM25QH128A",
		.id = 0x207018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		/* otp_ops_mode: set quad mode need enter otp, set 1 */
		.otp_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XM25QH64B",
		.id = 0x206017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "XM25QH128B",
		.id = 0x206018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "XM25QH64C",
		.id = 0x204017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XM25QH128C",
		.id = 0x204018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XM25QH256C",
		.id = 0x204019,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 32 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XM25QU128C",
		.id = 0x204118,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XM25QU256C",
		.id = 0x204119,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 32 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "FM25Q64A",
		.id = 0xF83217,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "MX25L25645G/35F",
		.id = 0xc22019,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 32 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "MX25L51245G",
		.id = 0xc2201A,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 64 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "W25Q256JV",
		.id = 0xef4019,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 32 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "W25Q512JV",
		.id = 0xef4020,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 1,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 64 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 4,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "BY25Q64AS",
		.id = 0x684017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "BY25Q128AS",
		.id = 0x684018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "ZB25VQ64",
		.id = 0x5e4017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "ZB25VQ128",
		.id = 0x5e4018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XT25F64B",
		.id = 0x0b4017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XT25F128B",
		.id = 0x0b4018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "XT25Q128D",
		.id = 0x0b6018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "FM25Q64A",
		.id = 0xa14017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "FM25Q128A",
		.id = 0xa14018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "FM25W128",
		.id = 0xa12818,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "P25Q64H",
		.id = 0x856017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "P25Q128H",
		.id = 0x856018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},

	},
	{
		.name = "GM25Q64A",
		.id = 0x1c4017,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "GM25Q128A",
		.id = 0x1c4018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "NM25Q64EVB",
		.id = 0x522217,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 2,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 2,
		},
	},
	{
		.name = "NM25Q128EVB",
		.id = 0x522118,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 2,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 2,
		},
	},
	{
		.name = "ZD25Q64B",
		.id = 0xba3217,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 8 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
	{
		.name = "SK25P128",
		.id = 0x256018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR,
			.bit_shift = 6,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR,
			.bit_shift = 6,
		},
	},
	{
		.name = "PY25Q128HA",
		.id = 0x852018,
#ifdef CONFIG_SPI_STANDARD_MODE
		.quad_ops_mode = 0,
#else
		.quad_ops_mode = 1,
#endif
		/* addr_ops_mode: en4byte or addr_len > 3, set 1 */
		.addr_ops_mode = 0,
		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 20,
		.chip_size = 16 * 1024 * 1024,
		.page_size = 256,
		.erase_size = ERASE_SIZE,
		.addr_len = 3,
		.chip_erase_cmd = SPINOR_OP_CHIP_ERASE,
		.quad_set = {
			.cmd = SPINOR_OP_WRSR_1,
			.bit_shift = 1,
		},
		.quad_get = {
			.cmd = SPINOR_OP_RDSR_1,
			.bit_shift = 1,
		},
	},
};

#endif

