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
#include <asm/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/base.h>
#include <asm/io.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/err.h>
#include <common.h>
#include <malloc.h>
#include <asm/arch/sfc.h>
#ifndef CONFIG_FPGA
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#endif

//#define   ISVP_DEBUG

static void sfc_writel(struct sfc *sfc, unsigned short offset, u32 value)
{
	writel(value, SFC_BASE + offset);
}

static unsigned int sfc_readl(struct sfc *sfc, unsigned short offset)
{
	return readl(SFC_BASE + offset);
}

#ifdef ISVP_DEBUG
void dump_sfc_reg(struct sfc *sfc)
{
	int i = 0;
	printf("SFC_GLB			:%08x\n", sfc_readl(sfc, SFC_GLB ));
	printf("SFC_DEV_CONF	:%08x\n", sfc_readl(sfc, SFC_DEV_CONF ));
	printf("SFC_DEV_STA_EXP	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_EXP));
	printf("SFC_DEV_STA_RT	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_RT ));
	printf("SFC_DEV_STA_MSK	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_MSK ));
	printf("SFC_TRAN_LEN		:%08x\n", sfc_readl(sfc, SFC_TRAN_LEN ));

	for(i = 0; i < 6; i++)
		printf("SFC_TRAN_CONF(%d)	:%08x\n", i,sfc_readl(sfc, SFC_TRAN_CONF(i)));

	for(i = 0; i < 6; i++)
		printf("SFC_DEV_ADDR(%d)	:%08x\n", i,sfc_readl(sfc, SFC_DEV_ADDR(i)));

	printf("SFC_MEM_ADDR :%08x\n", sfc_readl(sfc, SFC_MEM_ADDR ));
	printf("SFC_TRIG	 :%08x\n", sfc_readl(sfc, SFC_TRIG));
	printf("SFC_SR		 :%08x\n", sfc_readl(sfc, SFC_SR));
	printf("SFC_SCR		 :%08x\n", sfc_readl(sfc, SFC_SCR));
	printf("SFC_INTC	 :%08x\n", sfc_readl(sfc, SFC_INTC));
	printf("SFC_FSM		 :%08x\n", sfc_readl(sfc, SFC_FSM ));
	printf("SFC_CGE		 :%08x\n", sfc_readl(sfc, SFC_CGE ));
//	printf("SFC_RM_DR		:%08x\n", sfc_readl(spi, SFC_RM_DR));
}

static void dump_data(unsigned char *buf,size_t len)
{
	int i;
	for(i = 0;i<len;i++){
		if(!(i % 16)){
			printk("\n");
			printk("%08x:",i);
		}
		printk("%02x ",buf[i]);
	}
}
#endif

void sfc_init(struct sfc *sfc)
{
	int n;
	for(n = 0; n < N_MAX; n++) {
		sfc_writel(sfc, SFC_TRAN_CONF(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR_PLUS(n), 0);
	}

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

static void sfc_stop(struct sfc*sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_STOP;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

static void sfc_start(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_START;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

static void sfc_flush_fifo(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_FLUSH;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

static void sfc_ce_invalid_value(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	if(value == 0) {
		tmp &= ~DEV_CONF_CEDL;
	} else {
		tmp |= DEV_CONF_CEDL;
	}
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static void sfc_hold_invalid_value(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	if(value == 0) {
		tmp &= ~DEV_CONF_HOLDDL;
	} else {
		tmp |= DEV_CONF_HOLDDL;
	}
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static void sfc_wp_invalid_value(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	if(value == 0) {
		tmp &= ~DEV_CONF_WPDL;
	} else {
		tmp |= DEV_CONF_WPDL;
	}
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static void sfc_clear_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, 0x1f);
}

static void sfc_mask_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0x1f);
}

static void sfc_set_phase_num(struct sfc *sfc,int num)
{
	unsigned int tmp;

	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB_PHASE_NUM_MSK;
	tmp |= num << GLB_PHASE_NUM_OFFSET;
	sfc_writel(sfc, SFC_GLB, tmp);
}

static void sfc_clock_phase(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	if(value == 0) {
		tmp &= ~DEV_CONF_CPHA;
	} else {
		tmp |= DEV_CONF_CPHA;
	}
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static void sfc_clock_polarity(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	if(value == 0) {
		tmp &= ~DEV_CONF_CPOL;
	} else {
		tmp |= DEV_CONF_CPOL;
	}
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static void sfc_threshold(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB_THRESHOLD_MSK;
	tmp |= value << GLB_THRESHOLD_OFFSET;
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

static void sfc_hold_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_THOLD_MSK;
	tmp |= value << DEV_CONF_THOLD_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static void sfc_setup_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_TSETUP_MSK;
	tmp |= value << DEV_CONF_TSETUP_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static void sfc_interval_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_TSH_MSK;
	tmp |= value << DEV_CONF_TSH_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

int set_flash_timing(struct sfc *sfc, unsigned int t_hold, unsigned int t_setup, unsigned int t_shslrd, unsigned int t_shslwr)
{
	unsigned int c_hold;
	unsigned int c_setup;
	unsigned int t_in, c_in, val = 0;
	unsigned long cycle;
	unsigned int rate;

	rate = sfc->src_clk / 1000000;
	cycle = 1000 / rate;

	c_hold = t_hold / cycle;
	if(c_hold > 0)
		val = c_hold - 1;
	sfc_hold_delay(sfc, val);

	c_setup = t_setup / cycle;
	if(c_setup > 0)
		val = c_setup - 1;
	sfc_setup_delay(sfc, val);

	t_in = max(t_shslrd, t_shslwr);
	c_in = t_in / cycle;
	if(c_in > 0)
		val = c_in - 1;
	sfc_interval_delay(sfc, val);

	return 0;
}

static void sfc_set_length(struct sfc *sfc, int value)
{
	sfc_writel(sfc, SFC_TRAN_LEN, value);
}

static void sfc_transfer_mode(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	if(value == 0) {
		tmp &= ~GLB_OP_MODE;
	} else {
		tmp |= GLB_OP_MODE;
	}
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
	unsigned int data[1] = {0};
	unsigned int last_word = 0;

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
	//printf("--------- %s %d -----------fifo_num = %d last_word = %d\n",__func__,__LINE__,fifo_num,last_word);
	for(i = 0; i < fifo_num; i++) {
		sfc_read_data(sfc, (unsigned int *)sfc->transfer->data);
		sfc->transfer->data += 4;
		sfc->transfer->cur_len += 4;
	}

	/* last word */
	if(last_word == 1) {
		sfc_read_data(sfc, data);
		memcpy((void *)sfc->transfer->data, data, sfc->transfer->len % 4);

		sfc->transfer->data += sfc->transfer->len % 4;
		sfc->transfer->cur_len += 4;
	}
	return 0;
}

static unsigned int cpu_write_txfifo(struct sfc *sfc)
{
	int i;
	unsigned long align_len = 0;
	unsigned int fifo_num = 0;


	align_len = ALIGN(sfc->transfer->len , 4);

	if (((align_len - sfc->transfer->cur_len) / 4) > THRESHOLD){
		fifo_num = THRESHOLD;
	} else {
		fifo_num = (align_len - sfc->transfer->cur_len) / 4;
	}

	for(i = 0; i < fifo_num; i++) {
		sfc_write_data(sfc, *(unsigned int *)sfc->transfer->data);
		sfc->transfer->data += 4;
		sfc->transfer->cur_len += 4;
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
		tmp |= TRAN_CONF0_POLL_EN;
	else
		tmp &= ~(TRAN_CONF0_POLL_EN);

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

static void sfc_set_mem_addr(struct sfc *sfc,unsigned int addr )
{
	sfc_writel(sfc, SFC_MEM_ADDR, addr);
}

static int sfc_sr_handle(struct sfc *sfc)
{
	unsigned int reg_sr = 0;
	unsigned int tmp = 0;

	while (1) {
		reg_sr = sfc_readl(sfc,SFC_SR);

		if (reg_sr & CLR_RREQ) {
			sfc_writel(sfc, SFC_SCR, CLR_RREQ);
			cpu_read_rxfifo(sfc);
		}

		if (reg_sr & CLR_TREQ) {
			sfc_writel(sfc, SFC_SCR, CLR_TREQ);
			cpu_write_txfifo(sfc);
		}

		if(reg_sr & CLR_END){
			tmp = CLR_END;
			break;
		}

		if(reg_sr & CLR_UNDR){
			tmp = CLR_UNDR;
			printf("underun");
			break;
		}

		if(reg_sr & CLR_OVER){
			tmp = CLR_OVER;
			printf("overrun");
			break;
		}
	}
	if (tmp)
		sfc_writel(sfc, SFC_SCR, tmp);

	return 0;
}

static int sfc_start_transfer(struct sfc *sfc)
{
	int ret;
	sfc_mask_all_intc(sfc);
	sfc_clear_all_intc(sfc);
	sfc_start(sfc);

	ret = sfc_sr_handle(sfc);

	return ret;
}

static void sfc_set_tran_config(struct sfc *sfc, struct sfc_transfer *transfer, int channel)
{
	unsigned int tmp = 0;

	tmp = (0 << TRAN_CONF0_CLK_OFFSET)        \
		  | (transfer->addr_len << TRAN_CONF0_ADDR_WIDTH_OFFSET)         \
		  | (TRAN_CONF0_CMD_EN)                     \
		  | (0 << TRAN_CONF0_FMAT_OFFSET)                  \
		  | (transfer->data_dummy_bits << TRAN_CONF0_DMYBITS_OFFSET)         \
		  | (transfer->cmd_info->dataen << TRAN_CONF0_DATEEN_OFFSET)   \
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
	unsigned int tmp = sfc_readl(sfc, SFC_GLB);

	if (transfer->direction == GLB_TRAN_DIR_READ)
		tmp &= ~GLB_TRAN_DIR;
	else
		tmp |= GLB_TRAN_DIR;

	if (transfer->ops_mode == DMA_OPS)
		tmp |= GLB_OP_MODE;
	else
		tmp &= ~GLB_OP_MODE;

	sfc_writel(sfc, SFC_GLB, tmp);
}
static void sfc_glb_info_config(struct sfc *sfc,struct sfc_transfer *transfer)
{
	//sfc_transfer_direction(sfc, transfer->direction);
	sfc_set_length(sfc, transfer->len);
	if((transfer->ops_mode == DMA_OPS)){
		if(transfer->direction == GLB_TRAN_DIR_READ)
			flush_cache_all();
		else
			flush_cache_all();
		sfc_set_mem_addr(sfc, virt_to_phys((volatile void *)transfer->data));
	}else{
		sfc_set_mem_addr(sfc, 0);
	}
	sfc_set_glb_config(sfc, transfer);
}
#ifdef	ISVP_DEBUG
static void  dump_transfer(struct sfc_transfer *xfer,int num)
{
	printf("\n");
	printf("cmd[%d].cmd = 0x%02x\n",num,xfer->cmd_info->cmd);
	printf("cmd[%d].addr_len = %d\n",num,xfer->addr_len);
	printf("cmd[%d].dummy_byte = %d\n",num,xfer->data_dummy_bits);
	printf("cmd[%d].dataen = %d\n",num,xfer->cmd_info->dataen);
	printf("cmd[%d].sta_exp = %d\n",num,xfer->cmd_info->sta_exp);
	printf("cmd[%d].sta_msk = %d\n",num,xfer->cmd_info->sta_msk);


	printf("transfer[%d].addr = 0x%08x\n",num,xfer->addr);
	printf("transfer[%d].len = %d\n",num,xfer->len);
	printf("transfer[%d].data = 0x%p\n",num,xfer->data);
	printf("transfer[%d].direction = %d\n",num,xfer->direction);
	printf("transfer[%d].sfc_mode = %d\n",num,xfer->sfc_mode);
	printf("transfer[%d].ops_mode = %d\n",num,xfer->ops_mode);
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
	memset(m, 0, sizeof(struct sfc_message));
	INIT_LIST_HEAD(&m->transfers);
}


int sfc_ctl_init(struct sfc *sfc)
{
	unsigned int reg_clkgr = cpm_inl(CPM_CLKGR0);
	unsigned int gate = 0 | CPM_CLKGR_SFC | CPM_CLKGR_SSI1;

	reg_clkgr &= ~gate;
	cpm_outl(reg_clkgr,CPM_CLKGR0);
	/* When the SFC SSI clock is turned on, the SD card needs to be started */
	printf("***sfc init!\n");

	sfc_init(sfc);
	sfc_stop(sfc);

	/*set hold high*/
	sfc_hold_invalid_value(sfc, 1);
	/*set wp high*/
	sfc_wp_invalid_value(sfc, 1);

	sfc_clear_all_intc(sfc);
	sfc_mask_all_intc(sfc);

	sfc_threshold(sfc, sfc->threshold);
	/*config the sfc pin init state*/
	sfc_clock_phase(sfc, 0);
	sfc_clock_polarity(sfc, 0);
	sfc_ce_invalid_value(sfc, 1);


	sfc_transfer_mode(sfc, SLAVE_MODE);
	if(sfc->src_clk >= 100000000){
		sfc_smp_delay(sfc,DEV_CONF_SMP_DELAY_180);
	}
	return 0;
}

struct sfc *sfc_res_init(unsigned int sfc_rate)
{
	struct sfc *sfc = NULL;
	sfc = (struct sfc *)malloc(sizeof(struct sfc));
	if (!sfc) {
		printf("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
		return ERR_PTR(-ENOMEM);
	}
	memset(sfc, 0, sizeof(struct sfc));
	sfc->src_clk = sfc_rate;
#ifndef CONFIG_FPGA
	clk_set_rate(SFC, sfc->src_clk);
#endif
	sfc->threshold = THRESHOLD;

	sfc_ctl_init(sfc);

	return sfc;

}
