/**
 * xb_snd_dmic.c
 *
 * jbbi <jbbi@ingenic.cn>
 *
 * 24 APR 2012
 *
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/sound.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <linux/switch.h>
#include <linux/dma-mapping.h>
#include <linux/soundcard.h>
#include <linux/wait.h>
#include <mach/jzdma.h>
#include <mach/jzsnd.h>
#include <soc/irq.h>
#include <soc/base.h>
#include "xb47xx_dmic_v12.h"
#include "xb47xx_i2s_v12.h"
#include <linux/delay.h>
#include <soc/cpm.h>
/**
 * global variable
 **/
static LIST_HEAD(codecs_head);


#define FIFO_THR_VALE	(65)
#define DMIC_IRQ

struct jz_dmic {
	unsigned long  rate_type;
};

struct dmic_device {

	int dmic_irq;
	spinlock_t dmic_irq_lock;

	spinlock_t dmic_lock; /*P:register*/
	char name[20];
	struct resource * res;
	struct clk * i2s_clk;
	struct clk * clk;
	struct clk * pwc_clk;
	volatile bool dmic_is_incall_state;
	void __iomem * dmic_iomem;

	struct mutex mutex; /*used for ioctl*/

	/*work queue*/
	struct workqueue_struct *dmic_work_queue;
	struct work_struct dmic_work;
	unsigned int ioctl_cmd;
	unsigned long ioctl_arg;

	struct dsp_endpoints *dmic_endpoints;
	struct jz_dmic * cur_dmic;
};

static struct dmic_device *g_dmic_dev;

static int dmic_set_private_data(struct snd_dev_data *ddata, struct dmic_device * dmic_dev)
{
	ddata->priv_data = (void *)dmic_dev;
	return 0;
}
struct dmic_device * dmic_get_private_data(struct snd_dev_data *ddata)
{
	return (struct dmic_device * )ddata->priv_data;
}

struct snd_dev_data * dmic_get_ddata(struct platform_device *pdev) {
	return pdev->dev.platform_data;
}


/* -----------------register access ------------------*/
static inline void dmic_enable_irq(struct dmic_device *dmic_dev, unsigned long bits)
{
	unsigned long imsk, flags;
	spin_lock_irqsave(&dmic_dev->dmic_lock, flags);
	imsk = dmic_readl(dmic_dev, DMICIMR);
	imsk &= ~bits;
	dmic_writel(dmic_dev, DMICIMR, imsk);
	spin_unlock_irqrestore(&dmic_dev->dmic_lock, flags);
}
static inline void dmic_disable_irq(struct dmic_device * dmic_dev, unsigned long bits)
{
	unsigned long imsk, flags;
	spin_lock_irqsave(&dmic_dev->dmic_lock, flags);
	imsk = dmic_readl(dmic_dev, DMICIMR);
	imsk |= bits;
	dmic_writel(dmic_dev, DMICIMR, imsk);
	spin_unlock_irqrestore(&dmic_dev->dmic_lock, flags);

}
static inline void dmic_clear_irq(struct dmic_device *dmic_dev, unsigned long bits)
{
	dmic_writel(dmic_dev, DMICICR, bits);
}

static inline void dmic_write_part(struct dmic_device *dmic_dev, unsigned int reg, \
		unsigned int value, unsigned int shift, enum regs_bit_width width)
{
	unsigned int mask = (1 << width) - 1;
	unsigned int temp;
	unsigned long flags;

	spin_lock_irqsave(&dmic_dev->dmic_lock, flags);

	temp = dmic_readl(dmic_dev, reg);
	temp &= ~(mask << shift);
	temp |= (value & mask) << shift;
	dmic_writel(dmic_dev, reg, temp);

	spin_unlock_irqrestore(&dmic_dev->dmic_lock, flags);
}
/* ---------------register access end--------------*/

/*##################################################################*\
 | dump
\*##################################################################*/

static void dump_dmic_reg(struct dmic_device *dmic_dev)
{
	int i;
	unsigned long reg_addr[] = {
		DMICCR0, DMICGCR, DMICIMR, DMICICR, DMICTRICR, DMICTHRH,
		DMICTHRL, DMICTRIMMAX, DMICTRINMAX, DMICFCR, DMICFSR, DMICCGDIS
	};

	for (i = 0;i < 12; i++) {
		printk("##### dmic reg0x%x,=0x%x.\n",
	   	(unsigned int)reg_addr[i],dmic_readl(dmic_dev, reg_addr[i]));
	}
	//printk("##### intc,=0x%x.\n",*((volatile unsigned int *)(0xb0001000)));
}

/*######DMICTRIMMAX############################################################*\
 |* suspDMICTRINMAXand func
\*##################################################################*/

bool dmic_is_incall(struct dmic_device * dmic_dev)
{
	return dmic_dev->dmic_is_incall_state;
}

/*##################################################################*\
|* dev_ioctl
\*##################################################################*/
static int dmic_set_fmt(struct dmic_device * dmic_dev, unsigned long *format,int mode)
{

	int ret = 0;
	struct dsp_pipe *dp = NULL;
	struct dsp_endpoints * dmic_endpoints = dmic_dev->dmic_endpoints;
    /*
	 * The value of format reference to soundcard.
	 * AFMT_MU_LAW      0x00000001
	 * AFMT_A_LAW       0x00000002
	 * AFMT_IMA_ADPCM   0x00000004
	 * AFMT_U8			0x00000008
	 * AFMT_S16_LE      0x00000010
	 * AFMT_S16_BE      0x00000020
	 * AFMT_S8			0x00000040
	 */
	debug_print("format = %d",*format);
	switch (*format) {
	case AFMT_S16_LE:
		break;
	case AFMT_S16_BE:
		break;
	default :
		printk("DMIC: dmic only support format 16bit 0x%x.\n",(unsigned int)*format);
		return -EINVAL;
	}

	if (mode & CODEC_RMODE) {
		dp = dmic_endpoints->in_endpoint;
		dp->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		ret |= NEED_RECONF_TRIGGER;
		ret |= NEED_RECONF_DMA;
	}
	if (mode & CODEC_WMODE) {
		printk("DMIC: unsurpport replay!\n");
		ret = -1;
	}

	return ret;
}

/*##################################################################*\
|* filter opt
\*##################################################################*/
static void dmic_set_filter(struct dmic_device * dmic_dev, int mode , uint32_t channels)
{
	struct dsp_pipe *dp = NULL;
	struct dsp_endpoints * dmic_endpoints = dmic_dev->dmic_endpoints;

	if (mode & CODEC_RMODE)
		dp = dmic_endpoints->in_endpoint;
	else
		return;

	if (channels == 1) {
		dp->filter = convert_16bits_stereo2mono;
	} else if (channels == 3){
		dp->filter = convert_32bits_2_20bits_tri_mode;
	} else if (channels == 2){
		dp->filter = NULL;
	} else {
		dp->filter = NULL;
		printk("dp->filter null\n");
	}
}

static int dmic_set_trigger(struct dmic_device * dmic_dev, int mode)
{
	if (mode & CODEC_RMODE) {

		dmic_write_part(dmic_dev, DMICFCR, 1, DMICFCR_RDMS, REGS_BIT_WIDTH_1BIT);

		dmic_write_part(dmic_dev, DMICFCR, 65, DMICFCR_THR, REGS_BIT_WIDTH_7BIT);

		return 0;
	}

	if (mode & CODEC_WMODE) {
		return -1;
	}

	return 0;
}

static int dmic_set_voice_trigger(struct dmic_device * dmic_dev, unsigned long *THR ,int mode)
{
	int ret = 0;
	unsigned int iflg = 0;
	unsigned int imsk = 0;
	printk("THR = %ld\n",*THR);
	 if(mode & CODEC_RMODE) {

		clk_enable(dmic_dev->clk);
		iflg |= DMICICR_TRIFLG;
		imsk |= DMICIMR_MASK_ALL;
		dmic_disable_irq(dmic_dev, imsk);
		dmic_clear_irq(dmic_dev, iflg);

		/* close dmic data path*/
		dmic_write_part(dmic_dev, DMICCR0, SAMPLE_RATE_DIS, DMICCR0_SR, REGS_BIT_WIDTH_2BIT);

		/* trigger config*/
		dmic_write_part(dmic_dev, DMICTRICR, 2, DMICTRICR_TRI_MOD, REGS_BIT_WIDTH_4BIT);
		dmic_write_part(dmic_dev, DMICTHRH, 40000, DMIC_THR_H, REGS_BIT_WIDTH_20BIT);
		if(*THR != 0)
			dmic_write_part(dmic_dev, DMICTHRL, *THR, DMIC_THR_L, REGS_BIT_WIDTH_20BIT);
		else
			dmic_write_part(dmic_dev, DMICTHRL, 5000, DMIC_THR_L, REGS_BIT_WIDTH_20BIT);

		dmic_write_part(dmic_dev, DMICTRICR, 1, DMICTRICR_HPF2_EN, REGS_BIT_WIDTH_1BIT);
		/*clear tri module*/
		dmic_write_part(dmic_dev, DMICTRICR, 1 , DMICTRICR_TRI_CLR, REGS_BIT_WIDTH_1BIT);

		/*reset dmic*/
		dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_RESET, REGS_BIT_WIDTH_1BIT);
		while(dmic_readl(dmic_dev, DMICCR0) & (1 << DMICCR0_RESET));

		dmic_write_part(dmic_dev, DMICFCR, 1, DMICFCR_RDMS, REGS_BIT_WIDTH_1BIT);

		/*enable trigger*/
		dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_TRI_EN, REGS_BIT_WIDTH_1BIT);

		clk_disable(dmic_dev->clk);

		imsk = 0;
		imsk |= DMICIMR_WAKE_MASK | DMICIMR_TRI_MASK; /* we only need wake and trigger interrupt */
		dmic_enable_irq(dmic_dev, imsk);
	 }

	if (mode & CODEC_WMODE) {
		return -1;

	}

	return ret;
}

static int dmic_set_channel(struct dmic_device * dmic_dev, int* channel,int mode)
{
	int ret = 0;
	/* no need timing dependent */
	debug_print("channel = %d",*channel);
	if (mode & CODEC_RMODE) {
		if (*channel == 2) {
			dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_STEREO, REGS_BIT_WIDTH_1BIT);
			dmic_write_part(dmic_dev, DMICCR0, 0, DMICCR0_PACK_EN, REGS_BIT_WIDTH_1BIT);
			dmic_write_part(dmic_dev, DMICCR0, 0, DMICCR0_UNPACK_DIS, REGS_BIT_WIDTH_1BIT);

			dmic_set_filter(dmic_dev, CODEC_RMODE, 0);
		} else  {
			dmic_write_part(dmic_dev, DMICCR0, 0, DMICCR0_STEREO, REGS_BIT_WIDTH_1BIT);
			dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_PACK_EN, REGS_BIT_WIDTH_1BIT);
			dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_UNPACK_DIS, REGS_BIT_WIDTH_1BIT);
		}
	}
	if (mode & CODEC_WMODE) {
		return -1;
	}

	return ret;
}

static int dmic_set_rate(struct dmic_device * dmic_dev, unsigned long *rate,int mode)
{
	int ret = 0;
	struct jz_dmic * cur_dmic = dmic_dev->cur_dmic;
	debug_print("rate = %ld",*rate);
	if (mode & CODEC_WMODE) {
		/*************************************************\
		|* WARING:dmic have only one mode,               *|
		|* So we should not to care write mode.          *|
		\*************************************************/
		printk("dmic unsurpport replay!\n");
		return -EPERM;
	}
	if (mode & CODEC_RMODE) {
		if (*rate == 8000){
			cur_dmic->rate_type = 0;
			dmic_write_part(dmic_dev, DMICCR0, SAMPLE_RATE_8K, DMICCR0_SR, REGS_BIT_WIDTH_2BIT);
			dmic_write_part(dmic_dev, DMICTRICR, DMICTRICR_PREFETCH_8K, DMICTRICR_PREFETCH, REGS_BIT_WIDTH_2BIT);
		}
		else if (*rate == 16000){
			cur_dmic->rate_type = 1;
			dmic_write_part(dmic_dev, DMICCR0, SAMPLE_RATE_16K, DMICCR0_SR, REGS_BIT_WIDTH_2BIT);
			dmic_write_part(dmic_dev, DMICTRICR, DMICTRICR_PREFETCH_16K, DMICTRICR_PREFETCH, REGS_BIT_WIDTH_2BIT);
		}
		else if (*rate == 48000){
			dmic_write_part(dmic_dev, DMICCR0, SAMPLE_RATE_48K, DMICCR0_SR, REGS_BIT_WIDTH_2BIT);
		}
		else
			printk("DMIC: unsurpport samplerate: %x\n", *rate);
	}

	return ret;
}

static int dmic_record_deinit(struct dmic_device * dmic_dev, int mode)
{
	dmic_write_part(dmic_dev, DMICTRICR, 1, DMICTRICR_TRI_CLR, REGS_BIT_WIDTH_1BIT);
	dmic_clear_irq(dmic_dev, DMICICR_MASK_ALL);

	dmic_write_part(dmic_dev, DMICFCR, 0, DMICFCR_RDMS, REGS_BIT_WIDTH_1BIT);
	return 0;
}

static int dmic_record_init(struct dmic_device * dmic_dev, int mode)
{
	int rst_test = 50000;

	if (mode & CODEC_WMODE) {
		printk("-----> unsurport !\n");
		return -1;
	}

	dmic_write_part(dmic_dev, DMICCR0, SAMPLE_RATE_8K, DMICCR0_SR, REGS_BIT_WIDTH_2BIT);

	/*dmic reset*/
	dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_RESET, REGS_BIT_WIDTH_1BIT);
	while(dmic_readl(dmic_dev, DMICCR0) & (1 << DMICCR0_RESET)) {
		if (rst_test-- <= 0) {
			printk("-----> rest dmic failed!\n");
			return -1;
		}
	}

	printk("=====> %s success\n", __func__);

	return 0;
}

static int dmic_enable(struct dmic_device * dmic_dev, int mode)
{
	struct dsp_endpoints * dmic_endpoints = dmic_dev->dmic_endpoints;
	unsigned long record_rate = 8000;
	unsigned long record_format = 16;
	/*int record_channel = DEFAULT_RECORD_CHANNEL;*/
	struct dsp_pipe *dp_other = NULL;
	int ret = 0;

	clk_enable(dmic_dev->clk);
	if (mode & CODEC_WMODE)
		return -1;

	ret = dmic_record_init(dmic_dev, mode);
	if (ret)
		return -1;

	if (mode & CODEC_RMODE) {
		printk("come to %s %d set dp_other\n", __func__, __LINE__);
		dp_other = dmic_endpoints->in_endpoint;
		dmic_set_fmt(dmic_dev, &record_format,mode);
		dmic_set_rate(dmic_dev, &record_rate,mode);
	}

	dmic_disable_irq(dmic_dev, DMICIMR_MASK_ALL);
	dmic_clear_irq(dmic_dev, DMICICR_MASK_ALL);

	dmic_write_part(dmic_dev, DMICGCR, 9, DMIC_GCR, REGS_BIT_WIDTH_4BIT);

	dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_HPF1_EN, REGS_BIT_WIDTH_1BIT);

	dmic_write_part(dmic_dev, DMICTRICR, DMICTRICR_PREFETCH_DIS, DMICTRICR_PREFETCH, REGS_BIT_WIDTH_2BIT);

	dmic_write_part(dmic_dev, DMICTRICR, 1, DMICTRICR_HPF2_EN, REGS_BIT_WIDTH_1BIT);
	dmic_write_part(dmic_dev, DMICTRICR, 1, DMICTRICR_TRI_CLR, REGS_BIT_WIDTH_1BIT);


#ifdef CONFIG_JZ_DMIC1
	dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_SPLIT_DI, REGS_BIT_WIDTH_1BIT);
#endif
	dmic_write_part(dmic_dev, DMICCR0, 1, DMICCR0_DMIC_EN, REGS_BIT_WIDTH_1BIT);

	dmic_dev->dmic_is_incall_state = false;

	return 0;
}

static int dmic_disable(struct dmic_device * dmic_dev, int mode)
{
	dmic_record_deinit(dmic_dev, mode);
	dmic_write_part(dmic_dev, DMICCR0, 0, DMICCR0_DMIC_EN, REGS_BIT_WIDTH_1BIT);

	return 0;
}

static int dmic_dma_enable(struct dmic_device * dmic_dev, int mode)		//CHECK
{
	if (mode & CODEC_RMODE) {
		dmic_write_part(dmic_dev, DMICFCR, 65, DMICFCR_THR, REGS_BIT_WIDTH_7BIT);
		dmic_write_part(dmic_dev, DMICFCR, 1, DMICFCR_RDMS, REGS_BIT_WIDTH_1BIT);

		dmic_write_part(dmic_dev, DMICTRICR, 1, DMICTRICR_TRI_CLR, REGS_BIT_WIDTH_1BIT);
		dmic_clear_irq(dmic_dev, DMICICR_MASK_ALL);
	}
	if (mode & CODEC_WMODE) {
		printk("DMIC: dmic unsurpport replay\n");
		return -1;
	}

	return 0;
}

static int dmic_dma_disable(struct dmic_device * dmic_dev, int mode)		//CHECK seq dma and func
{
	if (mode & CODEC_RMODE) {
		dmic_write_part(dmic_dev, DMICFCR, 0, DMICFCR_RDMS, REGS_BIT_WIDTH_1BIT);
	}
	else
		return -1;

	return 0;
}

static int dmic_get_fmt_cap(struct dmic_device * dmic_dev, unsigned long *fmt_cap,int mode)
{
	unsigned long dmic_fmt_cap = 0;
	if (mode & CODEC_WMODE) {
		return -1;
	}
	if (mode & CODEC_RMODE) {
		dmic_fmt_cap |= AFMT_S16_LE|AFMT_S16_BE;
	}

	if (*fmt_cap == 0)
		*fmt_cap = dmic_fmt_cap;
	else
		*fmt_cap &= dmic_fmt_cap;

	return 0;
}


static int dmic_get_fmt(struct dmic_device * dmic_dev, unsigned long *fmt, int mode)
{
	if (mode & CODEC_WMODE)
		return -1;
	if (mode & CODEC_RMODE)
		*fmt = AFMT_S16_LE;

	return 0;
}

static void dmic_dma_need_reconfig(struct dmic_device * dmic_dev, int mode)
{
	struct dsp_pipe	*dp = NULL;
	struct dsp_endpoints * dmic_endpoints = dmic_dev->dmic_endpoints;

	if (mode & CODEC_RMODE) {
		dp = dmic_endpoints->in_endpoint;
		dp->need_reconfig_dma = true;
	}
	if (mode & CODEC_WMODE) {
		printk("DMIC: unsurpport replay mode\n");
	}

	return;
}


static int dmic_set_device(struct dmic_device * dmic_dev, unsigned long device)
{
	return 0;
}

/********************************************************\
 * dev_ioctl
\********************************************************/
static long dmic_ioctl(struct snd_dev_data *ddata, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct dmic_device * dmic_dev = dmic_get_private_data(ddata);

	switch(cmd) {
		case SND_DSP_GET_RECORD_FMT_CAP:
			/* return the support record formats */
			ret = 0;
			ret = dmic_get_fmt_cap(dmic_dev, (unsigned long *)arg,CODEC_RMODE);
			break;

		case SND_DSP_GET_RECORD_FMT:
			ret = 0;
			ret = dmic_get_fmt(dmic_dev, (unsigned long *)arg,CODEC_RMODE);
			/* get current record format */

			break;
		case SND_DSP_GET_REPLAY_RATE:
			ret = -1;
			printk("dmic not support replay!\n");
			break;

		case SND_DSP_GET_RECORD_RATE:
			ret = 0;
			break;
		case SND_DSP_GET_REPLAY_CHANNELS:
			printk("dmic not support record!\n");
			ret = -1;
			break;

		case SND_DSP_GET_RECORD_CHANNELS:
			ret = 0;
			break;

		case SND_DSP_GET_REPLAY_FMT_CAP:
			ret = -1;
			printk("dmic not support replay!\n");
			/* return the support replay formats */
			break;

		case SND_DSP_GET_REPLAY_FMT:
			/* get current replay format */
			ret = -1;
			printk("dmic not support replay!\n");
			break;
		case SND_DSP_FLUSH_SYNC:
			flush_work_sync(&dmic_dev->dmic_work);
			break;
		default:
			flush_work_sync(&dmic_dev->dmic_work);
			dmic_dev->ioctl_cmd = cmd;
			dmic_dev->ioctl_arg = arg ? *(unsigned int *)arg : arg;
			queue_work(dmic_dev->dmic_work_queue, &dmic_dev->dmic_work);
			break;

	}
	return ret;
}
static long do_ioctl_work(struct dmic_device * dmic_dev, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	//printk("####%s, cmd:(%d), arg(%x), arg_value:\n", __func__, cmd, arg);
	switch (cmd) {
	case SND_DSP_ENABLE_REPLAY:
		/* enable dmic replay */
		/* set dmic default record format, channels, rate */
		/* set default replay route */
		printk("dmic not support replay!\n");
		ret = -1;
		break;

	case SND_DSP_DISABLE_REPLAY:
		printk("dmic not support replay!\n");
		ret = -1;
		/* disable dmic replay */
		break;

	case SND_DSP_ENABLE_RECORD:
		/* enable dmic record */
		/* set dmic default record format, channels, rate */
		/* set default record route */
		printk("  dmic ioctl enable cmd\n");
		ret = dmic_enable(dmic_dev, CODEC_RMODE);
		printk("  dmic ioctl enable cmd complete\n");
		break;

	case SND_DSP_DISABLE_RECORD:
		/* disable dmic record */
		ret = 0;
		ret = dmic_disable(dmic_dev, CODEC_WMODE);
		break;

	case SND_DSP_ENABLE_DMA_RX:
		ret = dmic_dma_enable(dmic_dev, CODEC_RMODE);
		break;

	case SND_DSP_DISABLE_DMA_RX:
		ret = 0;
		ret = dmic_dma_disable(dmic_dev, CODEC_RMODE);
		break;

	case SND_DSP_ENABLE_DMA_TX:
		printk("dmic not support replay!\n");
		ret = -1;
		break;

	case SND_DSP_DISABLE_DMA_TX:
		ret = -1;
		printk("dmic not support replay!\n");
		break;

	case SND_DSP_SET_REPLAY_RATE:
		ret = -1;
		printk("dmic not support replay!\n");
		break;

	case SND_DSP_SET_RECORD_RATE:
		ret = 0;
		ret = dmic_set_rate(dmic_dev, (unsigned long *)arg,CODEC_RMODE);
		break;



	case SND_DSP_SET_REPLAY_CHANNELS:
		/* set replay channels */
		printk("dmic not support replay!\n");
		ret = -1;
		break;

	case SND_DSP_SET_RECORD_CHANNELS:
		ret = 0;
		ret = dmic_set_channel(dmic_dev, (int *)arg, CODEC_RMODE);
		/* set record channels */
		break;

	case SND_DSP_SET_REPLAY_FMT:
		/* set replay format */
		printk("dmic not support replay!\n");
		ret = -1;
		break;

	case SND_DSP_SET_RECORD_FMT:
		/* set record format */
		ret = dmic_set_fmt(dmic_dev, (unsigned long *)arg,CODEC_RMODE);
		if (ret < 0)
			break;
	//	[> if need reconfig the trigger, reconfig it <]
		if (ret & NEED_RECONF_TRIGGER)
			dmic_set_trigger(dmic_dev, CODEC_RMODE);
	//	[> if need reconfig the dma_slave.max_tsz, reconfig it and
	//	   set the dp->need_reconfig_dma as true <]
		if (ret & NEED_RECONF_DMA)
			dmic_dma_need_reconfig(dmic_dev, CODEC_RMODE);
		ret = 0;

		break;

	case SND_MIXER_DUMP_REG:
		dump_dmic_reg(dmic_dev);
		break;
	case SND_MIXER_DUMP_GPIO:
		break;

	case SND_DSP_SET_STANDBY:
		break;

	case SND_DSP_SET_DEVICE:
		ret = dmic_set_device(dmic_dev, arg);
		break;
	case SND_DSP_SET_RECORD_VOL:
		break;
	case SND_DSP_SET_REPLAY_VOL:
		break;
	case SND_DSP_SET_MIC_VOL:
		break;
	case SND_DSP_CLR_ROUTE:
		break;
	case SND_DSP_SET_VOICE_TRIGGER:
		ret = 0;
		ret = dmic_set_voice_trigger(dmic_dev, (unsigned long *)arg,CODEC_RMODE);
		break;
	case SND_DSP_RESUME_PROCEDURE:
		break;
	default:
		printk("SOUND_ERROR: %s(line:%d) unknown command!:(%d)\n",
				__func__, __LINE__, cmd);
		ret = -EINVAL;
	}

	return ret;
}

/*##################################################################*\
  |* functions
  \*##################################################################*/
#ifdef DMIC_IRQ
static irqreturn_t dmic_irq_handler(int irq, void *dev_id)
{
	irqreturn_t ret = IRQ_HANDLED;
	struct dmic_device * dmic_dev = (struct dmic_device *)dev_id;
	unsigned int dmic_stat = dmic_readl(dmic_dev, DMICICR);

	if((dmic_stat & DMICICR_TRIFLG) || \
			(dmic_stat & DMICICR_WAKEUP)) {
		/*trigger part*/
		printk("AUDIO TRIGGER!\n");
		dmic_disable_irq(dmic_dev, DMICIMR_TRI_MASK | DMICIMR_WAKE_MASK);
		dmic_clear_irq(dmic_dev, DMICICR_TRIFLG | DMICICR_WAKEUP);

		clk_enable(dmic_dev->clk);
		dmic_write_part(dmic_dev, DMICCR0, dmic_dev->cur_dmic->rate_type, DMICCR0_SR, REGS_BIT_WIDTH_2BIT);

		/*disable trigger*/
		dmic_write_part(dmic_dev, DMICCR0, 0, DMICCR0_TRI_EN, REGS_BIT_WIDTH_1BIT);
	} else {
		/*ignore other irqs*/
	}

	return ret;
}
#endif

static int dmic_init_pipe_2(struct dsp_pipe *dp , enum dma_data_direction direction,unsigned long iobase)
{
	if(dp == NULL) {
		printk("%s dp is null!!\n", __func__);
		return -EINVAL;
	}
	dp->dma_config.direction = direction;
	dp->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dp->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dp->dma_type = JZDMA_REQ_I2S1;
	dp->fragsize = FRAGSIZE_M;
	dp->fragcnt = FRAGCNT_L;
	/*(*dp)->fragcnt = FRAGCNT_B;*/

	if (direction == DMA_FROM_DEVICE) {
		dp->dma_config.src_maxburst = 32;
		dp->dma_config.dst_maxburst = 32;
		dp->dma_config.src_addr = iobase + DMICDR;
		dp->dma_config.dst_addr = 0;
	} else  if (direction == DMA_TO_DEVICE) {
		dp->dma_config.src_maxburst = 32;
		dp->dma_config.dst_maxburst = 32;
		dp->dma_config.dst_addr = iobase + AICDR;
		dp->dma_config.src_addr = 0;
	} else
		return -1;


	return 0;
}
static void dmic_work_handler(struct work_struct *work)
{
	struct dmic_device *dmic_dev = (struct dmic_device *)container_of(work, struct dmic_device, dmic_work);
	unsigned int cmd = dmic_dev->ioctl_cmd;
	unsigned long arg = dmic_dev->ioctl_arg;

	do_ioctl_work(dmic_dev, cmd, &arg);
}

static int dmic_global_init(struct platform_device *pdev)
{
	int ret = 0;
	struct dsp_pipe *dmic_pipe_out = NULL;
	struct dsp_pipe *dmic_pipe_in = NULL;
	struct dmic_device * dmic_dev;

	printk("----> start %s\n", __func__);
	dmic_dev = (struct dmic_device *)kzalloc(sizeof(struct dmic_device), GFP_KERNEL);

	if(!dmic_dev) {
		dev_err(&pdev->dev, "failed to alloc dmic dev!\n");
		return -ENOMEM;
	}

	dmic_dev->cur_dmic = kmalloc(sizeof(struct jz_dmic),GFP_KERNEL);
	sprintf(dmic_dev->name, "dmic");

	dmic_dev->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (dmic_dev->res == NULL) {
		printk("%s dmic_resource get failed!\n", __func__);
		return -1;
	}

	/* map io address */
	if (!request_mem_region(dmic_dev->res->start, resource_size(dmic_dev->res), pdev->name)) {
		printk("%s request mem region failed!\n", __func__);
		return -EBUSY;
	}
	dmic_dev->dmic_iomem = ioremap(dmic_dev->res->start, resource_size(dmic_dev->res));
	if (!dmic_dev->dmic_iomem) {
		printk("%s ioremap failed!\n", __func__);
		ret =  -ENOMEM;
		goto __err_ioremap;
	}

	dmic_pipe_out = kmalloc(sizeof(struct dsp_pipe),GFP_KERNEL);
	ret = dmic_init_pipe_2(dmic_pipe_out,DMA_TO_DEVICE,dmic_dev->res->start);
	if (ret < 0) {
		printk("%s init write pipe failed!\n", __func__);
		goto __err_init_pipeout;
	}

	dmic_pipe_in = kmalloc(sizeof(struct dsp_pipe),GFP_KERNEL);
	ret = dmic_init_pipe_2(dmic_pipe_in,DMA_FROM_DEVICE,dmic_dev->res->start);
	if (ret < 0) {
		printk("%s init read pipe failed!\n", __func__);
		goto __err_init_pipein;
	}

	dmic_dev->dmic_endpoints = kmalloc(sizeof(struct dsp_endpoints), GFP_KERNEL);
	if(!dmic_dev->dmic_endpoints) {
		printk("err, unable to malloc dsp_endpoints\n");
		goto __err_init_endpoints;
	}

	dmic_dev->dmic_endpoints->out_endpoint = dmic_pipe_out;
	dmic_dev->dmic_endpoints->in_endpoint = dmic_pipe_in;

	/*request dmic clk */
	dmic_dev->i2s_clk = clk_get(&pdev->dev, "cgu_i2s");
	if (IS_ERR(dmic_dev->i2s_clk)) {
		dev_err(&pdev->dev, "----> dmic cgu_i2s clk get failed\n");
		goto __err_dmic_clk;
	}
	clk_enable(dmic_dev->i2s_clk);

	dmic_dev->clk = clk_get(&pdev->dev, "dmic");
	if (IS_ERR(dmic_dev->clk)) {
		dev_err(&pdev->dev, "----> dmic clk_get failed\n");
		goto __err_dmic_clk;
	}
	clk_enable(dmic_dev->clk);

	/*request dmic pwc */
	dmic_dev->pwc_clk = clk_get(&pdev->dev, "pwc_dmic");
	if (IS_ERR(dmic_dev->pwc_clk)) {
		dev_err(&pdev->dev, "----> dmic pwc_get failed\n");
		goto __err_dmic_clk;
	}
	clk_enable(dmic_dev->pwc_clk);

	spin_lock_init(&dmic_dev->dmic_irq_lock);
	spin_lock_init(&dmic_dev->dmic_lock);
	mutex_init(&dmic_dev->mutex);

#ifdef DMIC_IRQ
	/* request irq */
	dmic_dev->dmic_irq = platform_get_irq(pdev, 0);
	ret = request_irq(dmic_dev->dmic_irq, dmic_irq_handler,
					  IRQF_DISABLED, "dmic_irq", dmic_dev);
	if (ret < 0) {
		printk("----> request irq error\n");
		goto __err_irq;
	}
#endif



	/*dmic worker*/
	dmic_dev->dmic_work_queue = create_singlethread_workqueue("dmic_work");
	if(!dmic_dev->dmic_work_queue) {
		return -ENOMEM;
	}

	INIT_WORK(&dmic_dev->dmic_work, dmic_work_handler);
	dmic_dev->ioctl_cmd = 0;
	dmic_dev->ioctl_arg = 0;

	g_dmic_dev = dmic_dev;

	printk("dmic init success.\n");
	clk_disable(dmic_dev->clk);

	return  0;
__err_dmic_clk:
__err_irq:
	clk_disable(dmic_dev->clk);
	clk_put(dmic_dev->clk);
__err_init_endpoints:
	vfree(dmic_pipe_in);
__err_init_pipein:
	vfree(dmic_pipe_out);
__err_init_pipeout:
	iounmap(dmic_dev->dmic_iomem);
__err_ioremap:
	release_mem_region(dmic_dev->res->start,resource_size(dmic_dev->res));
	return ret;
}

static int dmic_init(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_dev_data *tmp;
	tmp = dmic_get_ddata(pdev);
	if(!g_dmic_dev) {
		ret = dmic_global_init(pdev);
		if (ret)
			printk("dmic init error!\n");
	}
	dmic_set_private_data(tmp, g_dmic_dev);/*mixer*/
	return ret;
}

static void dmic_shutdown(struct platform_device *pdev)
{
	/* close dmic and current codec */
	struct snd_dev_data *tmp;
	struct dmic_device * dmic_dev;
	tmp = dmic_get_ddata(pdev);
	dmic_dev = dmic_get_private_data(tmp);
	dmic_write_part(dmic_dev, DMICCR0, 0, DMICCR0_DMIC_EN, REGS_BIT_WIDTH_1BIT);

	clk_disable(dmic_dev->clk);
	clk_disable(dmic_dev->pwc_clk);
	clk_disable(dmic_dev->i2s_clk);
	return;
}

static int dmic_suspend(struct platform_device *pdev, pm_message_t state)
{
	unsigned long thr = 0;
	struct snd_dev_data *tmp;
	struct dmic_device * dmic_dev;

	tmp = dmic_get_ddata(pdev);
	dmic_dev = dmic_get_private_data(tmp);

	dmic_set_voice_trigger(dmic_dev, &thr, CODEC_RMODE);
	/* make sure everything has done */
	flush_work_sync(&dmic_dev->dmic_work);
	return 0;
}

static int dmic_resume(struct platform_device *pdev)
{
	struct snd_dev_data *tmp;
	struct dmic_device * dmic_dev;
	tmp = dmic_get_ddata(pdev);
	dmic_dev = dmic_get_private_data(tmp);
	dmic_enable(dmic_dev, CODEC_RMODE);

	/* no need to do work */
	dmic_dev->ioctl_cmd = SND_DSP_RESUME_PROCEDURE;
	dmic_dev->ioctl_arg = 0;
	queue_work(dmic_dev->dmic_work_queue, &dmic_dev->dmic_work);
	return 0;
}

struct dsp_endpoints * dmic_get_endpoints(struct snd_dev_data *ddata)
{
	struct dmic_device * dmic_dev = dmic_get_private_data(ddata);

	return dmic_dev->dmic_endpoints;
}

struct snd_dev_data dmic_data = {
	.dev_ioctl_2	= dmic_ioctl,
	.get_endpoints	= dmic_get_endpoints,
	.minor			= SND_DEV_DSP3,
	.init			= dmic_init,
	.shutdown		= dmic_shutdown,
	.suspend		= dmic_suspend,
	.resume			= dmic_resume,
};

struct snd_dev_data snd_mixer3_data = {
	.dev_ioctl_2	= dmic_ioctl,
	.minor			= SND_DEV_MIXER3,
	.init			= dmic_init,
};

static int __init init_dmic(void)
{
	printk("-----> come to this %s\n", __func__);

	return 0;
}
module_init(init_dmic);
