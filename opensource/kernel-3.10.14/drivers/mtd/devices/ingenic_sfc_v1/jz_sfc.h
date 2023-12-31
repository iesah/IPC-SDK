#ifndef __LINUX_SFC_JZ_H
#define __LINUX_SFC_JZ_H

#include <mach/jzssi.h>
#include <mach/sfc_register.h>

#define UNCACHE(addr)   ((typeof(addr))(((unsigned long)(addr)) | 0xa0000000))
/* Flash opcodes. */
#define SPINOR_OP_WREN		0x06	/* Write enable */
#define SPINOR_OP_RDSR		0x05	/* Read status register */
#define SPINOR_OP_RDSR_1	0x35	/* Read status1 register */
#define SPINOR_OP_RDSR_2	0x15	/* Read status2 register */
#define SPINOR_OP_WRSR		0x01	/* Write status register 1 byte */
#define SPINOR_OP_WRSR_1	0x31	/* Write status1 register 1 byte */
#define SPINOR_OP_WRSR_2	0x11	/* Write status2 register 1 byte */
#define SPINOR_OP_READ		0x03	/* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST	0x0b	/* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2	0x3b	/* Read data bytes (Dual SPI) */
#define SPINOR_OP_READ_1_1_4	0x6b	/* Read data bytes (Quad SPI) */
#define SPINOR_OP_PP		0x02	/* Page program (up to 256 bytes) */
#define SPINOR_OP_QPP		0x32	/* Page program (up to 256 bytes) */
#define SPINOR_OP_BE_4K		0x20	/* Erase 4KiB block */
#define SPINOR_OP_BE_4K_PMC	0xd7	/* Erase 4KiB block on PMC chips */
#define SPINOR_OP_BE_32K	0x52	/* Erase 32KiB block */
#define SPINOR_OP_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define SPINOR_OP_SE		0xd8	/* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID		0x9f	/* Read JEDEC ID */
#define SPINOR_OP_RDCR		0x35	/* Read configuration register */
#define SPINOR_OP_RDFSR		0x70	/* Read flag status register */

/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define SPINOR_OP_READ4		0x13	/* Read data bytes (low frequency) */
#define SPINOR_OP_READ4_FAST	0x0c	/* Read data bytes (high frequency) */
#define SPINOR_OP_READ4_1_1_2	0x3c	/* Read data bytes (Dual SPI) */
#define SPINOR_OP_READ4_1_1_4	0x6c	/* Read data bytes (Quad SPI) */
#define SPINOR_OP_PP_4B		0x12	/* Page program (up to 256 bytes) */
#define SPINOR_OP_SE_4B		0xdc	/* Sector erase (usually 64KiB) */

/* Used for SST flashes only. */
#define SPINOR_OP_BP		0x02	/* Byte program */
#define SPINOR_OP_WRDI		0x04	/* Write disable */
#define SPINOR_OP_AAI_WP	0xad	/* Auto address increment word program */

/* Used for Macronix and Winbond flashes. */
#define SPINOR_OP_EN4B		0xb7	/* Enter 4-byte mode */
#define SPINOR_OP_EX4B		0xe9	/* Exit 4-byte mode */

/* Used for Spansion flashes only. */
#define SPINOR_OP_BRWR		0x17	/* Bank register write */

/* Status Register bits. */
#define SR_WIP			1	/* Write in progress */
#define SR_WEL			2	/* Write enable latch */
#define SR_SQE			(1 << 1)	/* QUAD MODE enable */
/* meaning of other SR_* bits may differ between vendors */
#define SR_BP0			4	/* Block protect 0 */
#define SR_BP1			8	/* Block protect 1 */
#define SR_BP2			0x10	/* Block protect 2 */
#define SR_SRWD			0x80	/* SR write protect */

#define SR_QUAD_EN_MX		0x40	/* Macronix Quad I/O */

/* Flag Status Register bits */
#define FSR_READY		0x80

/* Configuration Register bits. */
#define CR_QUAD_EN_SPAN		0x2	/* Spansion Quad I/O */



#define BUFFER_SIZE PAGE_SIZE

struct sfc_transfer {
	/* it's ok if tx_buf == rx_buf (right?)
	 * for MicroWire, one buffer must be null
	 * buffers must work with dma_*map_single() calls, unless
	 * spi_message.is_dma_mapped reports a pre-existing mapping
	 */
	void        *tx_buf;
	void        *tx_buf1;
	void        *rx_buf;
	unsigned    len;
};

struct jz_sfc {
	struct mtd_info     mtd;
	struct device		*dev;
	struct resource		*resource;
	void __iomem		*iomem;
	void __iomem		*iomem1;

	struct resource     *ioarea;

	unsigned int        clk_flag;
	unsigned int        set_clk_flag;

	u8          chnl;
	int			irq;
	struct clk		*clk;
	struct clk		*clk_gate;
	unsigned long src_clk;
	struct completion	done;
	struct completion	done_rx;
	spinlock_t		lock_status;
	spinlock_t		txrx_lock;
	int			threshold;
	int			clk_gate_flag;
	int			status;
	int			rlen;
	int			len;
	u8          use_dma;
	unsigned int *rx;
	const unsigned int *tx;
	const unsigned int *tx1;

	struct spi_nor_platform_data *board_info;
	unsigned int board_info_size;

	/* temp buffers */
	unsigned char   *swap_buf;
	u8			rw_mode;

	unsigned int		rx_addr_plus;
	unsigned int		tx_addr_plus;

	spinlock_t		lock_rxtx;

	unsigned long		phys;
	struct jz_sfc_info *pdata;
	irqreturn_t (*irq_callback)(struct jz_sfc *);
	struct workqueue_struct *workqueue;
	struct work_struct rw_work;
	struct mutex        lock;
	/*sfc use*/
	unsigned int addr;
	unsigned int sfc_mode;
	struct sfc_nor_info *nor_info;
	u8 addr_len;
};

static void sfc_writel(struct jz_sfc *sfc, unsigned short offset, u32 value)
{
	writel(value, sfc->iomem + offset);
}

static unsigned int sfc_readl(struct jz_sfc *sfc, unsigned short offset)
{
	return readl(sfc->iomem + offset);
}


void sfc_init(struct jz_sfc *sfc)
{
	int n;
	for(n = 0; n < N_MAX; n++) {
		sfc_writel(sfc, SFC_TRAN_CONF(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR_PLUS(n), 0);
		/*T40 ADD*/
		sfc_writel(sfc, SFC_TRAN_CONF1(n), 0);
	}

	sfc_writel(sfc, SFC_GLB, ((1 << 7) | (1 << 3)));
	sfc_writel(sfc, SFC_DEV_CONF, 0);
	sfc_writel(sfc, SFC_DEV_STA_EXP, 0);
	sfc_writel(sfc, SFC_DEV_STA_MSK, 0);
	sfc_writel(sfc, SFC_TRAN_LEN, 0);
	sfc_writel(sfc, SFC_MEM_ADDR, 0);
	sfc_writel(sfc, SFC_TRIG, 0);
	sfc_writel(sfc, SFC_SCR, 0);
	sfc_writel(sfc, SFC_INTC, 0);
	sfc_writel(sfc, SFC_CGE, 0);
	sfc_writel(sfc, SFC_RM_DR, 0);
}

void sfc_stop(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_STOP;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

void sfc_start(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_START;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

void sfc_flush_fifo(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_FLUSH;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

void sfc_ce_invalid_value(struct jz_sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_CEDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_CEDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_hold_invalid_value(struct jz_sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_HOLDDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_HOLDDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_wp_invalid_value(struct jz_sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_WPDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_WPDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_clear_end_intc(struct jz_sfc *sfc)
{
	int tmp = 0;
	tmp = sfc_readl(sfc, SFC_SCR);
	tmp |= CLR_END;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_treq_intc(struct jz_sfc *sfc)
{
	int tmp = 0;
	tmp = sfc_readl(sfc, SFC_SCR);
	tmp |= CLR_TREQ;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_rreq_intc(struct jz_sfc *sfc)
{
	int tmp = 0;
	tmp = sfc_readl(sfc, SFC_SCR);
	tmp |= CLR_RREQ;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_over_intc(struct jz_sfc *sfc)
{
	int tmp = 0;
	tmp = sfc_readl(sfc, SFC_SCR);
	tmp |= CLR_OVER;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_under_intc(struct jz_sfc *sfc)
{
	int tmp = 0;
	tmp = sfc_readl(sfc, SFC_SCR);
	tmp |= CLR_UNDER;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_all_intc(struct jz_sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, 0x1f);
}

void sfc_mask_all_intc(struct jz_sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0x1f);
}

void sfc_mode(struct jz_sfc *sfc, int channel, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRAN_CONF1(channel));
	tmp &= ~(TRAN_CONF1_TRAN_MODE_MSK);
	tmp |= (value << TRAN_CONF1_TRAN_MODE_OFFSET);
	sfc_writel(sfc, SFC_TRAN_CONF1(channel), tmp);
}

void sfc_clock_phase(struct jz_sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_CPHA;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_CPHA;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_clock_polarity(struct jz_sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_CPOL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_CPOL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_threshold(struct jz_sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB0_THRESHOLD_MSK;
	tmp |= value << GLB0_THRESHOLD_OFFSET;
	sfc_writel(sfc, SFC_GLB, tmp);
}


void sfc_smp_delay(struct jz_sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_SMP_DELAY_MSK;
	tmp |= value << DEV_CONF_SMP_DELAY_OFFSET | 1 << 9;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}


void sfc_transfer_direction(struct jz_sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_GLB);
		tmp &= ~GLB0_TRAN_DIR;
		sfc_writel(sfc, SFC_GLB, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_GLB);
		tmp |= GLB0_TRAN_DIR;
		sfc_writel(sfc, SFC_GLB, tmp);
	}
}

void sfc_set_length(struct jz_sfc *sfc, int value)
{
	sfc_writel(sfc, SFC_TRAN_LEN, value);
}

void sfc_transfer_mode(struct jz_sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_GLB);
		tmp &= ~GLB0_OP_MODE;
		sfc_writel(sfc, SFC_GLB, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_GLB);
		tmp |= GLB0_OP_MODE;
		sfc_writel(sfc, SFC_GLB, tmp);
	}
}

void sfc_read_data(struct jz_sfc *sfc, unsigned int *value)
{
	*value = sfc_readl(sfc, SFC_RM_DR);
}

void sfc_write_data(struct jz_sfc *sfc, const unsigned int *value)
{
	sfc_writel(sfc, SFC_RM_DR, *value);
}

unsigned int sfc_fifo_num(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_SR);
	tmp &= (0x7f << 16);
	tmp = tmp >> 16;
	return tmp;
}

int ssi_underrun(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_SR);
	if(tmp & CLR_UNDER)
		return 1;
	else
		return 0;
}

int ssi_overrun(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_SR);
	if(tmp & CLR_OVER)
		return 1;
	else
		return 0;
}

int rxfifo_rreq(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_SR);
	if(tmp & CLR_RREQ)
		return 1;
	else
		return 0;
}
int txfifo_treq(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_SR);
	if(tmp & CLR_TREQ)
		return 1;
	else
		return 0;
}
int sfc_end(struct jz_sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_SR);
	if(tmp & CLR_END)
		return 1;
	else
		return 0;
}

void sfc_set_addr_length(struct jz_sfc *sfc, int channel, unsigned int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
	tmp &= ~(TRAN_CONF0_ADDR_WIDTH_MSK);
	tmp |= (value << TRAN_CONF0_ADDR_WIDTH_OFFSET);
	sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
}

void sfc_cmd_en(struct jz_sfc *sfc, int channel, unsigned int value)
{
	if(value == 1) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
		tmp |= TRAN_CONF0_CMDEN;
		sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
		tmp &= ~TRAN_CONF0_CMDEN;
		sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
	}
}

void sfc_data_en(struct jz_sfc *sfc, int channel, unsigned int value)
{
	if(value == 1) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
		tmp |= TRAN_CONF0_DATEEN;
		sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
		tmp &= ~TRAN_CONF0_DATEEN;
		sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
	}
}

void sfc_write_cmd(struct jz_sfc *sfc, int channel, unsigned int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
	tmp &= ~TRAN_CONF0_CMD_MSK;
	tmp |= value;
	sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
}

void sfc_dev_addr(struct jz_sfc *sfc, int channel, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_ADDR(channel), value);
}


void sfc_dev_addr_dummy_bytes(struct jz_sfc *sfc, int channel, unsigned int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
	tmp &= ~TRAN_CONF0_DMYBITS_MSK;
	tmp |= value << TRAN_CONF0_DMYBITS_OFFSET;
	sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
}

void sfc_dev_addr_plus(struct jz_sfc *sfc, int channel, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_ADDR_PLUS(channel), value);
}

void sfc_dev_pollen(struct jz_sfc *sfc, int channel, unsigned int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
	if(value == 1)
		tmp |= TRAN_CONF0_POLLEN;
	else
		tmp &= ~(TRAN_CONF0_POLLEN);

	sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
}

void sfc_dev_sta_exp(struct jz_sfc *sfc, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_STA_EXP, value);
}

void sfc_dev_sta_msk(struct jz_sfc *sfc, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_STA_MSK, value);
}

void sfc_enable_all_intc(struct jz_sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0);
}

void sfc_set_mem_addr(struct jz_sfc *sfc,unsigned int addr )
{
	sfc_writel(sfc, SFC_MEM_ADDR, addr);
}
void dump_sfc_reg(struct jz_sfc *sfc)
{
	int i = 0;
	printk("SFC_GLB			:%08x\n", sfc_readl(sfc, SFC_GLB ));
	printk("SFC_DEV_CONF	:%08x\n", sfc_readl(sfc, SFC_DEV_CONF ));
	printk("SFC_DEV_STA_EXP	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_EXP));
	printk("SFC_DEV_STA_RT	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_RT ));
	printk("SFC_DEV_STA_MSK	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_MSK ));
	printk("SFC_TRAN_LEN		:%08x\n", sfc_readl(sfc, SFC_TRAN_LEN ));

	for(i = 0; i < 6; i++)
		printk("SFC_TRAN_CONF(%d)	:%08x\n", i,sfc_readl(sfc, SFC_TRAN_CONF(i)));

	for(i = 0; i < 6; i++)
		printk("SFC_DEV_ADDR(%d)	:%08x\n", i,sfc_readl(sfc, SFC_DEV_ADDR(i)));

	printk("SFC_MEM_ADDR :%08x\n", sfc_readl(sfc, SFC_MEM_ADDR ));
	printk("SFC_TRIG	 :%08x\n", sfc_readl(sfc, SFC_TRIG));
	printk("SFC_SR		 :%08x\n", sfc_readl(sfc, SFC_SR));
	printk("SFC_SCR		 :%08x\n", sfc_readl(sfc, SFC_SCR));
	printk("SFC_INTC	 :%08x\n", sfc_readl(sfc, SFC_INTC));
	printk("SFC_FSM		 :%08x\n", sfc_readl(sfc, SFC_FSM ));
	printk("SFC_CGE		 :%08x\n", sfc_readl(sfc, SFC_CGE ));
//	printk("SFC_RM_DR 	 :%08x\n", sfc_readl(spi, SFC_RM_DR));
}
#endif
