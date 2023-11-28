/*
 * SFC controller for SPI protocol, use FIFO and DMA;
 *
 * Copyright (c) 2015 Ingenic
 * Author: <xiaoyang.fu@ingenic.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#include "jz_sfc_common.h"


//#define SFC_DEBUG


#define GET_PHYADDR(a)  \
({						\
	unsigned int v;        \
	if (unlikely((unsigned int)(a) & 0x40000000)) {    \
	v = page_to_phys(vmalloc_to_page((const void *)(a))) | ((unsigned int)(a) & ~PAGE_MASK); \
	} else     \
	v = ((unsigned int)(a) & 0x1fffffff);                   \
	v;                                             \
 })
static inline void sfc_writel(struct sfc *sfc, unsigned short offset, u32 value)
{
	writel(value, sfc->iomem + offset);
}

static inline unsigned int sfc_readl(struct sfc *sfc, unsigned short offset)
{
	return readl(sfc->iomem + offset);
}

#ifdef SFC_DEBUG
void dump_sfc_reg(struct sfc *sfc)
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
static void dump_reg(struct sfc *sfc)
{
	printk("SFC_GLB = %08x\n",sfc_readl(sfc,0x0000));
	printk("SFC_DEV_CONF = %08x\n",sfc_readl(sfc,0x0004));
	printk("SFC_DEV_STA_EXP = %08x\n",sfc_readl(sfc,0x0008));
	printk("SFC_DEV_STA_RT	 = %08x\n",sfc_readl(sfc,0x000c));
	printk("SFC_DEV_STA_MASK = %08x\n",sfc_readl(sfc,0x0010));
	printk("SFC_TRAN_CONF0 = %08x\n",sfc_readl(sfc,0x0014));
	printk("SFC_TRAN_LEN = %08x\n",sfc_readl(sfc,0x002c));
	printk("SFC_DEV_ADDR0 = %08x\n",sfc_readl(sfc,0x0030));
	printk("SFC_DEV_ADDR_PLUS0 = %08x\n",sfc_readl(sfc,0x0048));
	printk("SFC_MEM_ADDR = %08x\n",sfc_readl(sfc,0x0060));
	printk("SFC_TRIG = %08x\n",sfc_readl(sfc,0x0064));
	printk("SFC_SR = %08x\n",sfc_readl(sfc,0x0068));
	printk("SFC_SCR = %08x\n",sfc_readl(sfc,0x006c));
	printk("SFC_INTC = %08x\n",sfc_readl(sfc,0x0070));
	printk("SFC_FSM = %08x\n",sfc_readl(sfc,0x0074));
	printk("SFC_CGE = %08x\n",sfc_readl(sfc,0x0078));
//	printk("SFC_DR = %08x\n",sfc_readl(sfc,0x1000));
}
#endif

static inline void sfc_init(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_TRIG, TRIG_STOP);
	sfc_writel(sfc, SFC_DEV_CONF, 0);

	/* X1000 need set to 0,but X2000 can be set to 1*/
	sfc_writel(sfc, SFC_CGE, 0);

}
static inline void sfc_start(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_START;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

static inline void sfc_flush_fifo(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_FLUSH;
	sfc_writel(sfc, SFC_TRIG, tmp);
}
static inline void  sfc_clear_end_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, CLR_END);
}

static inline void sfc_clear_treq_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, CLR_TREQ);
}

static inline void sfc_clear_rreq_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, CLR_RREQ);
}

static inline void sfc_clear_over_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, CLR_OVER);
}

static inline void sfc_clear_under_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, CLR_UNDER);
}
static inline void sfc_clear_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, 0x1f);
}

static inline void sfc_mask_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0x1f);
}


static void sfc_set_phase_num(struct sfc *sfc,int num)
{
	unsigned int tmp;

	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB0_PHASE_NUM_MSK;
	tmp |= num << GLB0_PHASE_NUM_OFFSET;
	sfc_writel(sfc, SFC_GLB, tmp);
}
static void sfc_dev_hw_init(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);

	/*cpha bit:0 , cpol bit:0 */
	tmp &= ~(DEV_CONF_CPHA | DEV_CONF_CPOL);
	/*ce_dl bit:1, hold bit:1,wp bit:1*/
	tmp |= (DEV_CONF_CEDL | DEV_CONF_HOLDDL | DEV_CONF_WPDL);
	sfc_writel(sfc, SFC_DEV_CONF, tmp);

}
static void sfc_threshold(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB0_THRESHOLD_MSK;
	tmp |= value << GLB0_THRESHOLD_OFFSET;
	sfc_writel(sfc, SFC_GLB, tmp);
}


static void sfc_smp_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_SMP_DELAY_MSK;
	tmp |= value << DEV_CONF_SMP_DELAY_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

int set_flash_timing(struct sfc *sfc, unsigned int t_hold, unsigned int t_setup, unsigned int t_shslrd, unsigned int t_shslwr)
{
	unsigned int c_hold = 0;
	unsigned int c_setup = 0;
	unsigned int t_in = 0, c_in = 0;
	unsigned long cycle;
	unsigned long long ns;
	unsigned int tmp;

	ns = 1000000000ULL;
	cycle = do_div(ns, sfc->src_clk);
	cycle = ns;

	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~(DEV_CONF_THOLD_MSK | DEV_CONF_TSETUP_MSK | DEV_CONF_TSH_MSK);

	c_hold = t_hold / cycle;
	if(c_hold > 0)
		c_hold -= 1;

	c_setup = t_setup / cycle;
	if(c_setup > 0)
		c_setup -= 1;

	t_in = max(t_shslrd, t_shslwr);
	c_in = t_in / cycle;
	if(c_in > 0)
		c_in -= 1;

	tmp |= (c_hold << DEV_CONF_THOLD_OFFSET) | \
		  (c_setup << DEV_CONF_TSETUP_OFFSET) | \
		  (c_in << DEV_CONF_TSH_OFFSET);

	sfc_writel(sfc, SFC_DEV_CONF, tmp);
	return 0;
}

static void sfc_set_length(struct sfc *sfc, int value)
{
	sfc_writel(sfc, SFC_TRAN_LEN, value);
}

static inline void sfc_transfer_mode(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	if(value == 0)
		tmp &= ~GLB0_OP_MODE;
	else
		tmp |= GLB0_OP_MODE;
	sfc_writel(sfc, SFC_GLB, tmp);
}

static void sfc_read_data(struct sfc *sfc, unsigned int *value)
{
	*value = sfc_readl(sfc, SFC_RM_DR);
}

static void sfc_write_data(struct sfc *sfc, const unsigned int value)
{
	sfc_writel(sfc, SFC_RM_DR, value);
}

static unsigned int cpu_read_rxfifo(struct sfc *sfc)
{
	int i;
	unsigned long align_len = 0;
	unsigned int fifo_num = 0;
	unsigned int last_word = 0;
	unsigned int unalign_data;
	unsigned char *c;

	align_len = ALIGN(sfc->transfer->len, 4);

	if(((align_len - sfc->transfer->cur_len) / 4) > THRESHOLD) {
		fifo_num = THRESHOLD;
		last_word = 0;
	} else {
		/* last aligned THRESHOLD data*/
		if(sfc->transfer->len % 4) {
			fifo_num = (align_len - sfc->transfer->cur_len) / 4 - 1;
			last_word = 1;
		} else {
			fifo_num = (align_len - sfc->transfer->cur_len) / 4;
			last_word = 0;
		}
	}

	if ((unsigned int)sfc->transfer->data & 3) {
		/* addr not align */
		for (i = 0; i < fifo_num; i++) {
			sfc_read_data(sfc, &unalign_data);
			c = (unsigned char *)sfc->transfer->data;
			c[0] = (unalign_data >> 0) &0xff;
			c[1] = (unalign_data >> 8) &0xff;
			c[2] = (unalign_data >> 16) &0xff;
			c[3] = (unalign_data >> 24) &0xff;

			sfc->transfer->data += 4;
			sfc->transfer->cur_len += 4;
		}
	} else {
		/* addr align */
		for (i = 0; i < fifo_num; i++) {
			sfc_read_data(sfc, (unsigned int *)sfc->transfer->data);
			sfc->transfer->data += 4;
			sfc->transfer->cur_len += 4;
		}
	}

	/* last word */
	if(last_word == 1) {
		sfc_read_data(sfc, &unalign_data);
		c = (unsigned char *)sfc->transfer->data;
		for(i = 0; i < sfc->transfer->len % 4; i++) {
			c[i] = (unalign_data >> (i * 8)) & 0xff;
		}

		sfc->transfer->data += sfc->transfer->len % 4;
		sfc->transfer->cur_len += sfc->transfer->len % 4;
	}

	return 0;
}

static unsigned int cpu_write_txfifo(struct sfc *sfc)
{
	unsigned long align_len = 0;
	unsigned int fifo_num = 0;
	unsigned int nbytes = sfc->transfer->len % 4;
	unsigned int data;
	int i;

	align_len = ALIGN(sfc->transfer->len , 4);

	if (((align_len - sfc->transfer->cur_len) / 4) > THRESHOLD){
		fifo_num = THRESHOLD;
	} else {
		fifo_num = (align_len - sfc->transfer->cur_len) / 4;
	}

	if ((unsigned int)sfc->transfer->data & 3) {
		/* addr not align */
		for(i = 0; i < fifo_num; i++) {
			data = sfc->transfer->data[3] << 24 | sfc->transfer->data[2] << 16 | sfc->transfer->data[1] << 8 | sfc->transfer->data[0];
			sfc_write_data(sfc, data);
			sfc->transfer->data += 4;
			sfc->transfer->cur_len += 4;
		}
	} else {
		/* addr align */
		for(i = 0; i < fifo_num; i++) {
			sfc_write_data(sfc, *(unsigned int *)sfc->transfer->data);
			sfc->transfer->data += 4;
			sfc->transfer->cur_len += 4;
		}
	}

	/* len not align */
	if (nbytes) {
		for (i = 0; i < nbytes; i++) {
			data |= sfc->transfer->data[i] << i * 8;
		}
		sfc_write_data(sfc, data);
	}

	return 0;
}

unsigned int sfc_get_sta_rt(struct sfc *sfc)
{
	return sfc_readl(sfc,SFC_DEV_STA_RT);
}


static void sfc_dev_addr(struct sfc *sfc, int channel, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_ADDR(channel), value);
}

static void sfc_dev_addr_plus(struct sfc *sfc, int channel, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_ADDR_PLUS(channel), value);
}

static void sfc_dev_pollen(struct sfc *sfc, int channel, unsigned int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRAN_CONF(channel));
	if(value == 1)
		tmp |= TRAN_CONF0_POLLEN;
	else
		tmp &= ~(TRAN_CONF0_POLLEN);

	sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
}

static void sfc_dev_sta_exp(struct sfc *sfc, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_STA_EXP, value);
}

static void sfc_dev_sta_msk(struct sfc *sfc, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_STA_MSK, value);
}

static void sfc_enable_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0);
}

static void sfc_set_mem_addr(struct sfc *sfc,unsigned int addr )
{
	sfc_writel(sfc, SFC_MEM_ADDR, addr);
}
#define SFC_TRANSFER_TIMEOUT	3000	//3000ms for timeout
static int sfc_start_transfer(struct sfc *sfc)
{
	int err;
	sfc_clear_all_intc(sfc);
	sfc_enable_all_intc(sfc);
	sfc_start(sfc);
	err = wait_for_completion_timeout(&sfc->done, msecs_to_jiffies(SFC_TRANSFER_TIMEOUT));
	if (!err) {
		sfc_mask_all_intc(sfc);
		sfc_clear_all_intc(sfc);
		printk("line:%d Timeout for ACK from SFC device\n",__LINE__);
		return -ETIMEDOUT;
	}
	return 0;
}

static void sfc_set_tran_config(struct sfc *sfc, struct sfc_transfer *transfer, int channel)
{
	unsigned int tmp = 0;

	/*T40 modify*/
	tmp = (0 << TRAN_CONF0_CLK_MODE)		\
		| (transfer->addr_len << TRAN_CONF0_ADDR_WIDTH_OFFSET)			\
		| (TRAN_CONF0_CMDEN)						\
		| (0 << TRAN_CONF0_FMAT_OFFSET)					\
		| (transfer->data_dummy_bits << TRAN_CONF0_DMYBITS_OFFSET)			\
		| (transfer->cmd_info->dataen << TRAN_CONF0_DATEEN_OFFSET)	\
		| transfer->cmd_info->cmd;
	sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);

	tmp = sfc_readl(sfc, SFC_TRAN_CONF1(channel));
	tmp &= ~(TRAN_CONF1_TRAN_MODE_MSK);
	tmp |= transfer->sfc_mode << TRAN_CONF1_TRAN_MODE_OFFSET;
	sfc_writel(sfc, SFC_TRAN_CONF1(channel), tmp);
}

static void sfc_phase_transfer(struct sfc *sfc,struct sfc_transfer *
		transfer,int channel)
{
	sfc_dev_addr(sfc, channel,transfer->addr);
	sfc_dev_addr_plus(sfc,channel,transfer->addr_plus);
	sfc_set_tran_config(sfc, transfer, channel);

}
static void common_cmd_request_transfer(struct sfc *sfc,struct sfc_transfer *transfer,int channel)
{
	sfc_phase_transfer(sfc,transfer,channel);
	sfc_dev_sta_exp(sfc,0);
	sfc_dev_sta_msk(sfc,0);
	sfc_dev_pollen(sfc,channel,DISABLE);
}

static void poll_cmd_request_transfer(struct sfc *sfc,struct sfc_transfer *transfer,int channel)
{
	struct cmd_info *cmd = transfer->cmd_info;
	sfc_phase_transfer(sfc,transfer,channel);
	sfc_dev_sta_exp(sfc,cmd->sta_exp);
	sfc_dev_sta_msk(sfc,cmd->sta_msk);
	sfc_dev_pollen(sfc,channel,ENABLE);
}

static void sfc_set_glb_config(struct sfc *sfc, struct sfc_transfer *transfer)
{
	unsigned int tmp = 0;

	tmp = sfc_readl(sfc, SFC_GLB);

	if (transfer->direction == GLB0_TRAN_DIR_READ)
		tmp &= ~GLB0_TRAN_DIR;
	else
		tmp |= GLB0_TRAN_DIR;

	if (transfer->ops_mode == DMA_OPS)
		tmp |= GLB0_OP_MODE;
	else
		tmp &= ~GLB0_OP_MODE;

	sfc_writel(sfc, SFC_GLB, tmp);
}

static void sfc_glb_info_config(struct sfc *sfc,struct sfc_transfer *transfer)
{
	if((transfer->ops_mode == DMA_OPS)){
		sfc_set_length(sfc, transfer->len);
		if(transfer->direction == GLB0_TRAN_DIR_READ)
			dma_cache_sync(NULL, (void *)transfer->data,transfer->len, DMA_FROM_DEVICE);
		else
			dma_cache_sync(NULL, (void *)transfer->data,transfer->len, DMA_TO_DEVICE);
		sfc_set_mem_addr(sfc, GET_PHYADDR(transfer->data));
	}else{
		sfc_set_length(sfc, transfer->len);
		sfc_set_mem_addr(sfc, 0);
	}
	sfc_set_glb_config(sfc, transfer);
}
#ifdef SFC_DEBUG
static void  dump_transfer(struct sfc_transfer *xfer,int num)
{
	printk("\n");
	printk("cmd[%d].cmd = 0x%02x\n",num,xfer->cmd_info->cmd);
	printk("cmd[%d].addr_len = %d\n",num,xfer->addr_len);
	printk("cmd[%d].dummy_byte = %d\n",num,xfer->data_dummy_bits);
	printk("cmd[%d].dataen = %d\n",num,xfer->cmd_info->dataen);
	printk("cmd[%d].sta_exp = %d\n",num,xfer->cmd_info->sta_exp);
	printk("cmd[%d].sta_msk = %d\n",num,xfer->cmd_info->sta_msk);


	printk("transfer[%d].addr = 0x%08x\n",num,xfer->addr);
	printk("transfer[%d].len = %d\n",num,xfer->len);
	printk("transfer[%d].data = 0x%p\n",num,xfer->data);
	printk("transfer[%d].direction = %d\n",num,xfer->direction);
	printk("transfer[%d].sfc_mode = %d\n",num,xfer->sfc_mode);
	printk("transfer[%d].ops_mode = %d\n",num,xfer->ops_mode);
}
#endif
int sfc_sync(struct sfc *sfc, struct sfc_message *message)
{
	struct sfc_transfer *xfer;
	int phase_num = 0;

	sfc_flush_fifo(sfc);
	sfc_set_length(sfc, 0);
	list_for_each_entry(xfer, &message->transfers, transfer_list) {
		if(xfer->cmd_info->sta_msk == 0){
			common_cmd_request_transfer(sfc,xfer,phase_num);
		}else{
			poll_cmd_request_transfer(sfc,xfer,phase_num);
		}
		if(xfer->cmd_info->dataen && xfer->len) {
			sfc_glb_info_config(sfc,xfer);
			message->actual_length += xfer->len;
			sfc->transfer = xfer;
		}
		phase_num++;
	}
	sfc_set_phase_num(sfc,phase_num);
	list_del_init(&message->transfers);
	return sfc_start_transfer(sfc);
}

void sfc_transfer_del(struct sfc_transfer *t)
{
	list_del(&t->transfer_list);
}

void sfc_message_add_tail(struct sfc_transfer *t, struct sfc_message *m)
{
	list_add_tail(&t->transfer_list, &m->transfers);
}

void sfc_message_init(struct sfc_message *m)
{
	memset(m, 0, sizeof(*m));
	INIT_LIST_HEAD(&m->transfers);
}

static irqreturn_t jz_sfc_pio_irq_callback(struct sfc *sfc)
{
	unsigned int val;

	val = sfc_readl(sfc, SFC_SR) & 0x1f;
	if(val & CLR_RREQ){
		sfc_clear_rreq_intc(sfc);
		cpu_read_rxfifo(sfc);
	}else if(val & CLR_TREQ) {
		sfc_clear_treq_intc(sfc);
		cpu_write_txfifo(sfc);
	}else if(val & CLR_OVER)
	{
		sfc_clear_over_intc(sfc);
		printk("sfc OVER !\n");
		complete(&sfc->done);
	}else if(val & CLR_UNDER) {
		sfc_clear_under_intc(sfc);
		printk("sfc UNDR !\n");
		complete(&sfc->done);

	}else if(val & CLR_END) {
		sfc_mask_all_intc(sfc);
		sfc_clear_end_intc(sfc);
		complete(&sfc->done);
	}else{
		printk("current staus %x not support \n", sfc_readl(sfc, SFC_SR));
		printk("entry handler staus %x  \n", val);
	}
	return IRQ_HANDLED;

}
static irqreturn_t jz_sfc_irq(int irq, void *dev)
{
	struct sfc *sfc = dev;
	return sfc->irq_callback(sfc);
}

static int jz_sfc_init_setup(struct sfc *sfc)
{
	sfc_init(sfc);
	sfc_threshold(sfc, sfc->threshold);
	sfc_dev_hw_init(sfc);

	sfc_transfer_mode(sfc, SLAVE_MODE);
	if(sfc->src_clk >= 100000000){
		sfc_smp_delay(sfc,DEV_CONF_SMP_DELAY_180);
	}
	sfc->irq_callback = &jz_sfc_pio_irq_callback;
	return 0;
}
struct sfc *sfc_res_init(struct platform_device *pdev)
{
	struct sfc *sfc;
	struct resource *res;
	char clkname[16];
	int err;
	sfc = kzalloc(sizeof(struct sfc), GFP_KERNEL);
	if (!sfc) {
		printk("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
		return ERR_PTR(-ENOMEM);
	}
	/* find and map our resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	sfc->ioarea = request_mem_region(res->start, resource_size(res),
					pdev->name);
	if (sfc->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve iomem region\n");
		err = -ENXIO;
		goto err_mem_region;
	}

	sfc->phys = res->start;

	sfc->iomem = ioremap(res->start, (res->end - res->start)+1);
	if (sfc->iomem == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		err = -ENXIO;
		goto err_no_iomap;
	}

	sfc->irq = platform_get_irq(pdev, 0);
	if (sfc->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}

	sprintf(clkname, "div_sfc%d", pdev->id);
	sfc->clk = clk_get(&pdev->dev, clkname);
	if (IS_ERR(sfc->clk)) {
		dev_err(&pdev->dev, "Cannot get cgu_sfc clock\n");
		err = -ENOENT;
		goto err_no_clk;
	}

	sprintf(clkname, "gate_sfc%d", pdev->id);
	sfc->clk_gate = clk_get(&pdev->dev, clkname);
	if (IS_ERR(sfc->clk_gate)) {
		dev_err(&pdev->dev, "Cannot get sfc clock\n");
		err = -ENOENT;
		goto err_no_clk_gate;
	}

	res = platform_get_resource(pdev, IORESOURCE_BUS, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_BUS\n");
		err = -ENOENT;
		goto err_no_iores_bus;
	}

	sfc->src_clk = res->start * 1000000;

	if (clk_get_rate(sfc->clk) >= sfc->src_clk) {
		clk_set_rate(sfc->clk, sfc->src_clk);
	} else {
		clk_set_rate(sfc->clk, sfc->src_clk);
	}

	clk_enable(sfc->clk);
	clk_enable(sfc->clk_gate);

	sfc->threshold = THRESHOLD;
	/* request SFC irq */
	err = request_irq(sfc->irq, jz_sfc_irq, 0, pdev->name, sfc);
	if (err) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		err = -EINVAL;
		goto err_request_irq;
	}

	/* SFC controller initializations for SFC */
	jz_sfc_init_setup(sfc);
	init_completion(&sfc->done);
	return sfc;

err_request_irq:
err_no_iores_bus:
	clk_put(sfc->clk_gate);
err_no_clk_gate:
	clk_put(sfc->clk);
err_no_clk:
err_no_irq:
	iounmap(sfc->iomem);
err_no_iomap:
	release_mem_region(res->start, resource_size(res));
err_mem_region:
	release_resource(sfc->ioarea);
err_no_iores:
	kfree(sfc->ioarea);
	return ERR_PTR(err);
}

void sfc_res_deinit(struct sfc *sfc)
{
	clk_put(sfc->clk_gate);
	clk_put(sfc->clk);
	free_irq(sfc->irq, sfc);
	iounmap(sfc->iomem);
	release_resource(sfc->ioarea);
	kfree(sfc->ioarea);
	kfree(sfc);;
}
