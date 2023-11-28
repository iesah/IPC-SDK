#ifndef __SFC_NAND_H__
#define __SFC_NAND_H__

#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/irqreturn.h>
/*#include "sfc_params.h"*/

#define SPIFLASH_PARAMER_OFFSET (25 * 1024)
struct cmd_info{
	int cmd;
	int cmd_len;/*reserved; not use*/
	int dataen;
	int sta_exp;
	int sta_msk;
};

struct sfc_transfer {
	int direction;

	struct cmd_info *cmd_info;

	int addr_len;
	unsigned int addr;
	unsigned int addr_plus;
	int addr_dummy_bits;/*cmd + addr_dummy_bits + addr*/

	const unsigned char *data;
	int data_dummy_bits;/*addr + data_dummy_bits + data*/
	unsigned int len;
	unsigned int cur_len;

	int sfc_mode;
	int ops_mode;
	int phase_format;/*we just use default value;phase1:cmd+dummy+addr... phase0:cmd+addr+dummy...*/

	struct list_head transfer_list;
};

struct sfc_message {
	struct list_head    transfers;
	unsigned        actual_length;
	int         status;

};

struct sfc{

	void __iomem		*iomem;
	struct resource     *ioarea;
	int			irq;
	struct clk		*clk;
	struct clk		*clk_gate;
	unsigned long src_clk;
	struct completion	done;
	int			threshold;
	irqreturn_t (*irq_callback)(struct sfc *);
	unsigned long		phys;

	struct sfc_transfer *transfer;
};


struct sfc_flash;

struct spi_nor_flash_ops {
	int (*set_4byte_mode)(struct sfc_flash *flash);
	int (*set_quad_mode)(struct sfc_flash *flash);
};


struct sfc_flash {
	struct mtd_info     mtd;
	struct device		*dev;
	struct resource		*resource;
	struct sfc			*sfc;
	struct jz_sfc_info *pdata;
	void *flash_info;
	unsigned int flash_info_num;

	unsigned int chip_id;
	struct mutex        lock;

	int			status;
	spinlock_t		lock_status;

	int quad_succeed;
	struct spi_nor_info *g_nor_info;
	struct spi_nor_flash_ops	*nor_flash_ops;
	struct spi_nor_cmd_info	 *cur_r_cmd;
	struct spi_nor_cmd_info	 *cur_w_cmd;
	struct burner_params *params;
	struct norflash_partitions *norflash_partitions;

	unsigned char *swap_buf;
};

#define CHANNEL_0		0
#define CHANNEL_1		1
#define CHANNEL_2		2
#define CHANNEL_3		3
#define CHANNEL_4		4
#define CHANNEL_5		5

#define ENABLE			1
#define DISABLE			0

#define COM_CMD			1	// common cmd
#define POLL_CMD		2	// the cmd will poll the status of flash,ext: read status

#define DMA_OPS			1
#define CPU_OPS			0

#define TM_STD_SPI		0
#define TM_DI_DO_SPI	1
#define TM_DIO_SPI		2
#define TM_FULL_DIO_SPI	3
#define TM_QI_QO_SPI	5
#define TM_QIO_SPI		6
#define	TM_FULL_QIO_SPI	7


#define DEFAULT_ADDRSIZE	3


#define THRESHOLD		32

#define SFC_NOR_RATE    110
#define DEF_ADDR_LEN    3
#define DEF_TCHSH       5
#define DEF_TSLCH       5
#define DEF_TSHSL_R     20
#define DEF_TSHSL_W     50

#endif


