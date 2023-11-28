#ifndef __SFC_NAND_H__
#define __SFC_NAND_H__

#ifndef CONFIG_SPL_BUILD
#include <linux/list.h>
#endif
#include <linux/mtd/mtd.h>
#include "sfc.h"


struct cmd_info{
	short cmd;
	char cmd_len;/*reserved; not use*/
	char dataen;
#ifndef CONFIG_SPL_BUILD
	int sta_exp;
	int sta_msk;
#endif
};

struct sfc_transfer {

#ifndef CONFIG_SPL_BUILD
	struct cmd_info *cmd_info;
#else
	struct cmd_info cmd_info;
#endif

	char addr_len;
	char direction;
	char addr_dummy_bits;/*cmd + addr_dummy_bits + addr*/
	char data_dummy_bits;/*addr + data_dummy_bits + data*/
	unsigned int addr;
	unsigned int addr_plus;

	char sfc_mode;
	char ops_mode;
	char phase_format;/*we just use default value;phase1:cmd+dummy+addr... phase0:cmd+addr+dummy...*/
	char *data;
	unsigned int len;
	unsigned int cur_len;


#ifndef CONFIG_SPL_BUILD
	struct list_head transfer_list;
#endif
};

#ifndef CONFIG_SPL_BUILD
struct sfc_message {
	struct list_head    transfers;
	unsigned  int   actual_length;
	int         status;

};
#endif

struct sfc{

#ifndef CONFIG_SPL_BUILD
	unsigned long long src_clk;
#endif
	int			threshold;

	struct sfc_transfer *transfer;
};

struct sfc_flash;

struct spi_nor_flash_ops {
	int (*set_4byte_mode)(struct sfc_flash *flash);
	int (*set_quad_mode)(struct sfc_flash *flash);
};


struct sfc_flash {
	struct sfc *sfc;
	struct mtd_info  *mtd;

	void *flash_info;
#ifndef CONFIG_SPL_BUILD
	int quad_succeed;
	struct spi_nor_info *g_nor_info;
#else
	struct mini_spi_nor_info g_nor_info;
#endif
	struct spi_nor_flash_ops	*nor_flash_ops;
	struct spi_nor_cmd_info	 *cur_r_cmd;
	struct spi_nor_cmd_info	 *cur_w_cmd;
#ifndef CONFIG_SPL_BUILD
	struct norflash_partitions *norflash_partitions;
#endif
};



/* SFC register */

#define	SFC_TRAN_CONF0			(0x0014)
#define	SFC_DEV_ADDR0			(0x0030)
#define	SFC_DEV_ADDR_PLUS0		(0x0048)

/* For SFC_GLB */
#define GLB_TRAN_DIR_OFFSET         (13)
#define GLB_TRAN_DIR            (1 << GLB_TRAN_DIR_OFFSET)
#define GLB_TRAN_DIR_WRITE		(1)
#define GLB_TRAN_DIR_READ		(0)
#define	GLB_THRESHOLD_OFFSET		(7)
#define GLB_THRESHOLD_MSK		(0x3f << GLB_THRESHOLD_OFFSET)
#define GLB_OP_MODE			(1 << 6)
#define SLAVE_MODE			(0x0)
#define DMA_MODE			(0x1)
#define GLB_PHASE_NUM_OFFSET		(3)
#define GLB_PHASE_NUM_MSK		(0x7  << GLB_PHASE_NUM_OFFSET)
#define GLB_WP_EN			(1 << 2)
#define GLB_BURST_MD_OFFSET		(0)
#define GLB_BURST_MD_MSK		(0x3  << GLB_BURST_MD_OFFSET)

/* For SFC_DEV_CONF */
#define	DEV_CONF_ONE_AND_HALF_CYCLE_DELAY	(3)
#define	DEV_CONF_ONE_CYCLE_DELAY	(2)
#define	DEV_CONF_HALF_CYCLE_DELAY	(1)
#define	DEV_CONF_NO_DELAY	        (0)
#define	DEV_CONF_SMP_DELAY_OFFSET	(16)
#define	DEV_CONF_SMP_DELAY_MSK		(0x3 << DEV_CONF_SMP_DELAY_OFFSET)
#define DEV_CONF_CMD_TYPE		(0x1 << 15)
#define DEV_CONF_STA_TYPE_OFFSET	(13)
#define DEV_CONF_STA_TYPE_MSK		(0x1 << DEV_CONF_STA_TYPE_OFFSET)
#define DEV_CONF_THOLD_OFFSET		(11)
#define	DEV_CONF_THOLD_MSK		(0x3 << DEV_CONF_THOLD_OFFSET)
#define DEV_CONF_TSETUP_OFFSET		(9)
#define DEV_CONF_TSETUP_MSK		(0x3 << DEV_CONF_TSETUP_OFFSET)
#define DEV_CONF_TSH_OFFSET		(5)
#define DEV_CONF_TSH_MSK		(0xf << DEV_CONF_TSH_OFFSET)
#define DEV_CONF_CPHA			(0x1 << 4)
#define DEV_CONF_CPOL			(0x1 << 3)
#define DEV_CONF_CEDL			(0x1 << 2)
#define DEV_CONF_HOLDDL			(0x1 << 1)
#define DEV_CONF_WPDL			(0x1 << 0)

/* For SFC_TRAN_CONF */
#define	TRAN_CONF_TRAN_MODE_OFFSET	(29)
#define	TRAN_CONF_TRAN_MODE_MSK		(0x7)
#define	TRAN_CONF_ADDR_WIDTH_OFFSET	(26)
#define TRAN_CONF_FMAT_OFFSET		(23)
#define TRAN_CONF_DATEEN_OFFSET		(16)
#define	TRAN_CONF_ADDR_WIDTH_MSK	(0x7 << ADDR_WIDTH_OFFSET)
#define TRAN_CONF_POLLEN		(1 << 25)
#define TRAN_CONF_CMDEN			(1 << 24)
#define TRAN_CONF_FMAT			(1 << 23)
#define TRAN_CONF_DMYBITS_OFFSET	(17)
#define TRAN_CONF_DMYBITS_MSK		(0x3f << DMYBITS_OFFSET)
#define TRAN_CONF_DATEEN		(1 << 16)
#define	TRAN_CONF_CMD_OFFSET		(0)
#define	TRAN_CONF_CMD_MSK		(0xffff << CMD_OFFSET)
#define	TRAN_CONF_CMD_LEN		(1 << 15)

/* For SFC_TRIG */
#define TRIG_FLUSH			(1 << 2)
#define TRIG_STOP			(1 << 1)
#define TRIG_START			(1 << 0)

//For SFC_SR
#define FIFONUM_OFFSET  (16)
#define FIFONUM_MSK     (0x7f << FIFONUM_OFFSET)
#define BUSY_OFFSET     (5)
#define BUSY_MSK        (0x3 << BUSY_OFFSET)
#define END             (1 << 4)
#define TRAN_REQ        (1 << 3)
#define RECE_REQ        (1 << 2)
#define OVER            (1 << 1)
#define UNDER           (1 << 0)

/* For SFC_SCR */
#define	CLR_END			(1 << 4)
#define CLR_TREQ		(1 << 3)
#define CLR_RREQ		(1 << 2)
#define CLR_OVER		(1 << 1)
#define CLR_UNDER		(1 << 0)

/* For SFC_INTC */
#define MASK_END                (1 << 4)
#define MASK_TREQ               (1 << 3)
#define MASK_RREQ               (1 << 2)
#define MASK_OVER               (1 << 1)
#define MASK_UNDR               (1 << 0)

/* For SFC_TRAN_CONFx */
#define	TRAN_MODE_OFFSET	(29)
#define	TRAN_MODE_MSK		(0x7 << TRAN_MODE_OFFSET)
#define TRAN_SPI_STANDARD   (0x0)

//For SFC_FSM
#define FSM_AHB_OFFSET      (16)
#define FSM_AHB_MSK         (0xf << FSM_AHB_OFFSET)
#define FSM_SPI_OFFSET      (11)
#define FSM_SPI_MSK         (0x1f << FSM_SPI_OFFSET)
#define FSM_CLK_OFFSET      (6)
#define FSM_CLK_MSK         (0xf << FSM_CLK_OFFSET)
#define FSM_DMAC_OFFSET     (3)
#define FSM_DMAC_MSK        (0x7 << FSM_DMAC_OFFSET)
#define FSM_RMC_OFFSET      (0)
#define FSM_RMC_MSK         (0x7 << FSM_RMC_OFFSET)

//For SFC_CGE
#define CG_EN           (1 << 0)

#define	ADDR_WIDTH_OFFSET	(26)
#define	ADDR_WIDTH_MSK		(0x7 << ADDR_WIDTH_OFFSET)
#define POLLEN			(1 << 25)
#define CMDEN			(1 << 24)
#define FMAT			(1 << 23)
#define DMYBITS_OFFSET		(17)
#define DMYBITS_MSK		(0x3f << DMYBITS_OFFSET)
#define DATEEN			(1 << 16)
#define	CMD_OFFSET		(0)
#define	CMD_MSK			(0xffff << CMD_OFFSET)

#define N_MAX				6


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


#define DEF_ADDR_LEN    3
#define DEF_TCHSH       5
#define DEF_TSLCH       5
#define DEF_TSHSL_R     20
#define DEF_TSHSL_W     50


#ifdef CONFIG_SPL_SFC_NAND

#ifdef CONFIG_SPL_BUILD
typedef union sfc_tranconf_r {
	/** raw register data */
	unsigned int d32;
	/** register bits */
	struct {
		unsigned cmd:16;
		unsigned data_en:1;
		unsigned dmy_bits:6;
		unsigned phase_format:1;
		unsigned cmd_en:1;
		unsigned poll_en:1;
		unsigned addr_width:3;
		unsigned tran_mode:3;
	} reg;
} sfc_tranconf_r;
struct jz_sfc {
	sfc_tranconf_r tranconf;
    unsigned int  addr;
    unsigned int  len;
    unsigned int  addr_plus;
};
#endif

#ifdef CONFIG_SFC_DEBUG
#define sfc_debug(fmt, args...)         \
    do {                    \
        printf(fmt, ##args);        \
    } while (0)
#else
#define sfc_debug(fmt, args...)         \
    do {                    \
    } while (0)
#endif

#define  SFC_SEND_COMMAND(sfc, a, b, c, d, e, f, g)   do{                       \
        ((struct jz_sfc *)sfc)->tranconf.d32 = 0;				\
        ((struct jz_sfc *)sfc)->tranconf.reg.cmd_en = 1;				\
		((struct jz_sfc *)sfc)->tranconf.reg.cmd = a;					\
        ((struct jz_sfc *)sfc)->len = b;                                        \
        ((struct jz_sfc *)sfc)->addr = c;                                       \
        ((struct jz_sfc *)sfc)->tranconf.reg.addr_width = d;			\
        ((struct jz_sfc *)sfc)->addr_plus = 0;                                  \
        ((struct jz_sfc *)sfc)->tranconf.reg.dmy_bits = e;				\
        ((struct jz_sfc *)sfc)->tranconf.reg.data_en = f;				\
	if(a == CMD_FR_CACHE_QUAD) {						\
			((struct jz_sfc *)sfc)->tranconf.reg.tran_mode = TRAN_SPI_QUAD; \
	} else {								\
			((struct jz_sfc *)sfc)->tranconf.reg.tran_mode = TRAN_SPI_STANDARD; \
	}									\
        sfc_send_cmd(sfc, g);                                                   \
} while(0)

#endif

#endif
