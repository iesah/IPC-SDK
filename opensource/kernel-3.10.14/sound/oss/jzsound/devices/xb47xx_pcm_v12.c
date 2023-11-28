/**
 * xb_snd_pcm.c
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
#include <mach/jzdma.h>
#include <mach/jzsnd.h>
#include <soc/irq.h>
#include <soc/base.h>
#include "xb47xx_pcm_v12.h"

/**
 * global variable
 **/
struct pcm_conf_info pcm_conf_info = {
		.pcm_sync_clk = 0,
		.pcm_clk	= 0,
		.iss = 16,
		.oss = 16,
		.pcm_device_mode = PCM_SLAVE,
		.pcm_imsb_pos = ONE_SHIFT_MODE,
		.pcm_omsb_pos = ONE_SHIFT_MODE,
		.pcm_sync_len = 1,
		.pcm_slot_num = 1,
};

void volatile __iomem *volatile pcm_iomem;

static spinlock_t pcm_irq_lock;
static struct dsp_endpoints pcm_endpoints;

#define PCM_FIFO_DEPTH 16

static struct pcm_board_info
{
	unsigned long rate;
	unsigned long cpm_epll_clk;
	unsigned long replay_format;
	unsigned long record_format;
	unsigned long pcmclk;
	unsigned long pcm_sysclk;
	snd_pcm_data_start_mode_t pcm_imsb_pos;
	snd_pcm_data_start_mode_t pcm_omsb_pos;
	snd_pcm_mode_t pcm_device_mode;
	snd_pcm_sync_len_t pcm_sync_len;
	snd_pcm_solt_num_t pcm_slot_num;
	unsigned int irq;
	struct dsp_endpoints *endpoint;
}*pcm_priv;

/*
 *	dump
 *
 */
static void dump_pcm_reg(void)
{
	int i = 0;
	unsigned long pcm_regs[] = {
		PCMCTL0, PCMCFG0, PCMDP0, PCMINTC0, PCMINTS0, PCMDIV0 };

	for (i=0; i<ARRAY_SIZE(pcm_regs); i++) {
		printk("pcm reg %4x, = %4x \n",
				(unsigned int)pcm_regs[i], pcm_read_reg(pcm_regs[i]));
	}
}

/*##################################################################*\
 |* suspand func
\*##################################################################*/
static int pcm_suspend(struct platform_device *, pm_message_t state);
static int pcm_resume(struct platform_device *);
static void pcm_shutdown(struct platform_device *);

/*##################################################################*\
|* dev_ioctl
\*##################################################################*/
static void dump_dma_config(struct dsp_pipe *dp)
{
#if 0
	if (dp != NULL) {
		printk("dsp_pipe = 0x%x\n",&dp->dma_config);
		printk("dma_data_direction = %d\n",dp->dma_config.direction);
		printk("src_addr = 0x%x\n",dp->dma_config.src_addr);
		printk("dst_addr= 0x%x\n",dp->dma_config.dst_addr);
		printk("src_addr_width= %x\n",dp->dma_config.src_addr_width);
		printk("dst_addr_width= %x\n",dp->dma_config.dst_addr_width);
		printk("src_maxburst = %d\n",dp->dma_config.src_maxburst);
		printk("dst_maxburst= %p\n",dp->dma_config.dst_maxburst);
	} else
		printk("dsp_pipe = 0x%x\n",dp);
#endif
	return;
}

static int pcm_set_fmt(unsigned long *format,int mode)
{
	int ret = 0;
	int data_width = 0;
	struct dsp_pipe *dp_in = NULL;
	struct dsp_pipe *dp_out = NULL;
	dp_in = pcm_priv->endpoint->in_endpoint;
	dp_out = pcm_priv->endpoint->out_endpoint;

	switch (*format) {

	case AFMT_U8:
	case AFMT_S8:
		*format = AFMT_U8;
		data_width = 8;
		if (mode & CODEC_WMODE) {
			__pcm_set_oss_sample_size(0);
			dp_out->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
			dp_out->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		} if (mode & CODEC_RMODE) {
			__pcm_set_iss_sample_size(0);
			dp_in->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
			dp_in->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		}
		break;
	case AFMT_S16_LE:
	case AFMT_S16_BE:
		data_width = 16;
		*format = AFMT_S16_LE;
		if (mode & CODEC_WMODE) {
			__pcm_set_oss_sample_size(1);
			/*__pcm_set_oss_sample_size(0);*/
			dp_out->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
			dp_out->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		}
		if (mode & CODEC_RMODE) {
			__pcm_set_iss_sample_size(1);
			dp_in->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
			dp_in->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		}
		break;
	default :
		printk("PCM: there is unknown format 0x%x.\n",(unsigned int)*format);
		return -EINVAL;
	}
	if (mode & CODEC_WMODE) {
		dp_out->dma_config.dst_maxburst = 16 * data_width/8/2;
		dump_dma_config(dp_out);
		if (pcm_priv->replay_format != *format) {
			pcm_priv->replay_format = *format;
			ret |= NEED_RECONF_TRIGGER | NEED_RECONF_FILTER;
			ret |= NEED_RECONF_DMA;
		}
	}

	if (mode & CODEC_RMODE) {
		dp_in->dma_config.src_maxburst = 16 * data_width/8/2;
		dump_dma_config(dp_in);
		if (pcm_priv->record_format != *format) {
			pcm_priv->record_format = *format;
			ret |= NEED_RECONF_TRIGGER | NEED_RECONF_FILTER;
			ret |= NEED_RECONF_DMA;
		}
	}

	return ret;
}

static int pcm_set_channel(int* channel,int mode)
{
	return 0;
}

static int pcm_set_rate(unsigned long *rate)
{
	unsigned long div;
	if (pcm_priv->pcm_device_mode == PCM_MASTER) {
		if (!pcm_priv)
			return -ENODEV;
		div = pcm_priv->pcmclk/(8*(*rate)) - 1;
		if (div >= 0 && div < 32) {
			__pcm_set_syndiv(div);
			*rate = pcm_priv->pcmclk/(8*(div+1));
			pcm_priv->rate = *rate;
		} else
			*rate = pcm_priv->rate;
	}
//	__pcm_set_syndiv(div);

	return 0;
}

static int pcm_set_pcmclk(unsigned long pcmclk)
{
	unsigned long div;
	if (!pcm_priv)
		return -ENODEV;
	div = pcm_priv->pcm_sysclk/pcmclk - 1;
	if (div >= 0 && div < 64) {
		__pcm_set_clkdiv(div);
	} else
		return -EINVAL;
//	__pcm_set_clkdiv(div);
	return 0;
}

static int get_burst_length(unsigned long val)
{
	/* burst bytes for 1,2,4,8,16,32,64 bytes */
	int ord;

	ord = ffs(val) - 1;
	if (ord < 0)
		ord = 0;
	else if (ord > 6)
		ord = 6;

	/* if tsz == 8, set it to 4 */
	return (ord == 3 ? 4 : 1 << ord)*8;
}

static void pcm_set_trigger(int mode)
{
	int data_width = 0;
	struct dsp_pipe *dp = NULL;
	int burst_length = 0;

	if (!pcm_priv)
		return;

	if (mode & CODEC_WMODE) {
		switch(pcm_priv->replay_format) {
		case AFMT_U8:
			data_width = 8;
			break;
		case AFMT_S16_LE:
			data_width = 16;
			break;
		}
		dp = pcm_priv->endpoint->out_endpoint;
		burst_length = get_burst_length((int)dp->paddr|(int)dp->fragsize|dp->dma_config.dst_maxburst);
		__pcm_set_transmit_trigger((PCM_FIFO_DEPTH - burst_length/data_width) - 1);
		printk("PCM_FIFO_DEPTH - burst_length/data_width - 1 = %d\n",(PCM_FIFO_DEPTH - burst_length/data_width) - 1);
}
	if (mode &CODEC_RMODE) {
		switch(pcm_priv->record_format) {
		case AFMT_U8:
			data_width = 8;
			break;
		case AFMT_S16_LE:
			data_width = 16;
			break;
		}
		dp = pcm_priv->endpoint->in_endpoint;
		burst_length = get_burst_length((int)dp->paddr|(int)dp->fragsize|dp->dma_config.src_maxburst);
		__pcm_set_receive_trigger((burst_length/data_width) - 1);
	}

	return;
}

static int pcm_enable(int mode)
{
	unsigned long rate = 8000;
	unsigned long replay_format = 16;
	unsigned long record_format = 16;
	int replay_channel = 1;
	int record_channel = 1;
	struct dsp_pipe *dp_other = NULL;

	if (!pcm_priv)
			return -ENODEV;

	if (pcm_priv->replay_format != 0)
		replay_format = pcm_priv->replay_format;
	if (pcm_priv->record_format != 0)
		record_format = pcm_priv->record_format;
	if (pcm_priv->rate != 0)
		rate = pcm_priv->rate;

	if (mode & CODEC_WMODE) {
		dp_other = pcm_priv->endpoint->in_endpoint;
		pcm_set_fmt(&replay_format,mode);
		pcm_set_channel(&replay_channel,mode);
		pcm_set_trigger(mode);
	}
	if (mode & CODEC_RMODE) {
		dp_other = pcm_priv->endpoint->out_endpoint;
		pcm_set_fmt(&record_format,mode);
		pcm_set_channel(&record_channel,mode);
		pcm_set_trigger(mode);
	}
	pcm_set_rate(&rate);

	if (!dp_other->is_used) {
		if (mode & CODEC_WMODE)
			__pcm_flush_fifo();
		__pcm_enable();
		__pcm_clock_enable();
	}
	return 0;
}

static int pcm_disable(int mode) {
	if (mode & CODEC_WMODE) {
		__pcm_disable_transmit_dma();
		__pcm_disable_replay();
	}
	if (mode & CODEC_RMODE) {
		__pcm_disable_receive_dma();
		__pcm_disable_record();
	}
	__pcm_disable();
	__pcm_clock_disable();
	return 0;
}


static int pcm_dma_enable(int mode)
{
	int val;
	if (!pcm_priv)
			return -ENODEV;
	if (mode & CODEC_WMODE) {
		__pcm_enable_underrun_intr();
		__pcm_clear_tur();
		__pcm_enable_transmit_dma();
		__pcm_enable_replay();
	}
	if (mode & CODEC_RMODE) {
		__pcm_flush_fifo();
		__pcm_enable_record();
		/* read the first sample and ignore it */
		val = __pcm_read_fifo();
		__pcm_enable_receive_dma();
	}

//	dump_pcm_reg();
	return 0;
}

static int pcm_dma_disable(int mode)		//CHECK seq dma and func
{
	if (mode & CODEC_WMODE) {
		__pcm_disable_transmit_dma();
		__pcm_disable_replay();
	}
	if (mode & CODEC_RMODE) {
		__pcm_disable_receive_dma();
		__pcm_disable_record();
	}
	return 0;
}

static int pcm_get_fmt_cap(unsigned long *fmt_cap)
{
	*fmt_cap |= AFMT_S16_LE|AFMT_U8;
	return 0;
}

static int pcm_get_fmt(unsigned long *fmt, int mode)
{
	if (!pcm_priv)
			return -ENODEV;

	if (mode & CODEC_WMODE)
		*fmt = pcm_priv->replay_format;
	if (mode & CODEC_RMODE)
		*fmt = pcm_priv->record_format;

	return 0;
}

static void pcm_dma_need_reconfig(int mode)
{
	struct dsp_pipe	*dp = NULL;

	if (!pcm_priv)
			return;
	if (mode & CODEC_WMODE) {
		dp = pcm_priv->endpoint->out_endpoint;
		dp->need_reconfig_dma = true;
	}
	if (mode & CODEC_RMODE) {
		dp = pcm_priv->endpoint->in_endpoint;
		dp->need_reconfig_dma = true;
	}

	return;
}

/********************************************************\
 * dev_ioctl
\********************************************************/
static long pcm_ioctl(unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	switch (cmd) {
	case SND_DSP_ENABLE_REPLAY:
		/* enable pcm record */
		/* set pcm default record format, channels, rate */
		/* set default replay route */
		ret = pcm_enable(CODEC_WMODE);
		break;

	case SND_DSP_DISABLE_REPLAY:
		/* disable pcm replay */
		ret = pcm_disable(CODEC_WMODE);
		break;

	case SND_DSP_ENABLE_RECORD:
		/* enable pcm record */
		/* set pcm default record format, channels, rate */
		/* set default record route */
		ret = pcm_enable(CODEC_RMODE);
		break;

	case SND_DSP_DISABLE_RECORD:
		/* disable pcm record */
		ret = pcm_disable(CODEC_RMODE);
		break;

	case SND_DSP_ENABLE_DMA_RX:
		ret = pcm_dma_enable(CODEC_RMODE);
		break;

	case SND_DSP_DISABLE_DMA_RX:
		ret = pcm_dma_disable(CODEC_RMODE);
		break;

	case SND_DSP_ENABLE_DMA_TX:
		ret = pcm_dma_enable(CODEC_WMODE);
		break;

	case SND_DSP_DISABLE_DMA_TX:
		ret = pcm_dma_disable(CODEC_WMODE);
		break;

	case SND_DSP_SET_REPLAY_RATE:
	case SND_DSP_SET_RECORD_RATE:
		ret = pcm_set_rate((unsigned long *)arg);
		break;

	case SND_DSP_SET_REPLAY_CHANNELS:
	case SND_DSP_SET_RECORD_CHANNELS:
		/* set record channels */
		ret = pcm_set_channel((int*)arg,CODEC_RMODE);
		break;

	case SND_DSP_GET_REPLAY_FMT_CAP:
	case SND_DSP_GET_RECORD_FMT_CAP:
		/* return the support record formats */
		ret = pcm_get_fmt_cap((unsigned long *)arg);
		break;

	case SND_DSP_GET_REPLAY_FMT:
		/* get current replay format */
		pcm_get_fmt((unsigned long *)arg,CODEC_WMODE);
		break;

	case SND_DSP_SET_REPLAY_FMT:
		/* set replay format */
		ret = pcm_set_fmt((unsigned long *)arg,CODEC_WMODE);
		if (ret < 0)
			break;
		/* if need reconfig the trigger, reconfig it */
		if (ret & NEED_RECONF_TRIGGER)
			pcm_set_trigger(CODEC_WMODE);
		/* if need reconfig the dma_slave.max_tsz, reconfig it and
		   set the dp->need_reconfig_dma as true */
		if (ret & NEED_RECONF_DMA)
			pcm_dma_need_reconfig(CODEC_WMODE);
		/* if need reconfig the filter, reconfig it */
		if (ret & NEED_RECONF_FILTER)
			;
		ret = 0;
		break;

	case SND_DSP_GET_RECORD_FMT:
		/* get current record format */
		pcm_get_fmt((unsigned long *)arg,CODEC_RMODE);

		break;

	case SND_DSP_SET_RECORD_FMT:
		/* set record format */
		ret = pcm_set_fmt((unsigned long *)arg,CODEC_RMODE);
		if (ret < 0)
			break;
		/* if need reconfig the trigger, reconfig it */
		if (ret & NEED_RECONF_TRIGGER)
			pcm_set_trigger(CODEC_RMODE);
		/* if need reconfig the dma_slave.max_tsz, reconfig it and
		   set the dp->need_reconfig_dma as true */
		if (ret & NEED_RECONF_DMA)
			pcm_dma_need_reconfig(CODEC_RMODE);
		/* if need reconfig the filter, reconfig it */
		if (ret & NEED_RECONF_FILTER)
			;
		ret = 0;
		break;
	case SND_MIXER_DUMP_REG:
		dump_pcm_reg();
		break;
	default:
		printk("SOUND_ERROR: %s(line:%d) unknown command!",
				__func__, __LINE__);
		ret = -EINVAL;
	}

	return ret;
}

/*##################################################################*\
|* functions
\*##################################################################*/
static irqreturn_t pcm_irq_handler(int irq, void *dev_id)
{
	unsigned long flags;
	irqreturn_t ret = IRQ_HANDLED;

	spin_lock_irqsave(&pcm_irq_lock,flags);
	if (__pcm_test_tur()) {
		printk("UNDERRUN HAPPEN\n");
		__pcm_clear_tur();
	}
	spin_unlock_irqrestore(&pcm_irq_lock,flags);

	return ret;
}

static int pcm_init_pipe(struct dsp_pipe **dp , enum dma_data_direction direction,unsigned long iobase)
{
	if (*dp != NULL || dp == NULL)
		return 0;
	*dp = vmalloc(sizeof(struct dsp_pipe));
	if (*dp == NULL) {
		printk("pcm : init pipe fail vmalloc ");
		return -ENOMEM;
	}

	(*dp)->dma_config.direction = direction;
	(*dp)->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	(*dp)->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	(*dp)->dma_type = JZDMA_REQ_PCM0;

	(*dp)->fragsize = FRAGSIZE_S;
	(*dp)->fragcnt = FRAGCNT_M;
	(*dp)->is_non_block = true;
	(*dp)->is_used = false;
	(*dp)->can_mmap =true;
	INIT_LIST_HEAD(&((*dp)->free_node_list));
	INIT_LIST_HEAD(&((*dp)->use_node_list));

	if (direction == DMA_TO_DEVICE) {
		(*dp)->dma_config.dst_maxburst = 16;
		(*dp)->dma_config.src_maxburst = 16;
		(*dp)->dma_config.dst_addr = iobase + PCMDP0;
		(*dp)->dma_config.src_addr = 0;
	} else if (direction == DMA_FROM_DEVICE) {
		(*dp)->dma_config.dst_maxburst = 16;
		(*dp)->dma_config.src_maxburst = 16;
		(*dp)->dma_config.src_addr = iobase + PCMDP0;
		(*dp)->dma_config.dst_addr = 0;
	} else {
		printk("pcm :deriction error %d\n",direction);
		return -1;
	}

	dump_dma_config(*dp);
	return 0;
}
static int calculate_pcm_clk(void)
{
	unsigned int sys_div = 0;
	unsigned int sys_div_tmp = 0;
	unsigned int pcm_clk_div = 0;
	unsigned int pcm_clk_div_tmp = 0;
	unsigned int div_total = 0;
	unsigned int error = ~0;
	unsigned int error_tmp = 0;
	int i = 0;

	switch (pcm_priv->rate) {
		case 8000:
			switch (pcm_priv->pcmclk/pcm_priv->rate) {
				case 64:
				case 128:
					if (pcm_priv->cpm_epll_clk != 768000000) {
						printk(KERN_WARNING"epll is not 768000000,"
								"pcmclk and pcmsync is error");
						goto pcm_default;

					} else {
						pcm_priv->pcm_sysclk = 51200000;
						return 0;
					}
				case 256:
				default:
					break;
			}
		case 11025:
			//.... for future add
		default:
			printk("rate is may not support\n");
	}

pcm_default:
	/*
	 * Note:
	 * There is a very cursory arithmetic for pcmclk and pcm sync
	 * If possible, do not use it,config the epll in bootlarder
	 */
	if (pcm_priv->pcmclk)
		return -EINVAL;
	div_total = pcm_priv->cpm_epll_clk/pcm_priv->pcmclk;
	if (div_total == 0)
		return -EINVAL;
	if ((pcm_priv->cpm_epll_clk/div_total - pcm_priv->pcmclk) >
			(pcm_priv->pcmclk - pcm_priv->cpm_epll_clk/(div_total+1)))
		div_total = div_total + 1;
	if (div_total > 512) {
		for (i = 2; i <= 512;i++) {
			if (((div_total%i == 0) && (div_total/i <=64)) || (div_total/i == 0)) {
				sys_div = i;
				pcm_clk_div = div_total/i;
				break;
			} else {
				pcm_clk_div_tmp = div_total/i;
				sys_div_tmp = i;
				if (pcm_priv->cpm_epll_clk/(sys_div_tmp*pcm_clk_div_tmp) - pcm_priv->pcmclk >
						pcm_priv->pcmclk - pcm_priv->cpm_epll_clk/(sys_div_tmp*(pcm_clk_div_tmp+1))){
					error_tmp = pcm_priv->pcmclk - pcm_priv->cpm_epll_clk/(sys_div_tmp*(pcm_clk_div_tmp+1));
					pcm_clk_div_tmp = pcm_clk_div_tmp+1;
				} else
					error_tmp = pcm_priv->cpm_epll_clk/(sys_div_tmp*pcm_clk_div_tmp) - pcm_priv->pcmclk;
				if (error > error_tmp) {
					sys_div = sys_div_tmp;
					pcm_clk_div = pcm_clk_div_tmp;
					error = error_tmp;
				}
			}
		}
		pcm_priv->pcm_sysclk = pcm_priv->cpm_epll_clk/sys_div;
	} else
		pcm_priv->pcm_sysclk = pcm_priv->cpm_epll_clk/div_total;

	return 0;
}

static int pcm_clk_init(struct platform_device *pdev)
{
	struct clk *epll_clk = NULL;
	struct clk *pcm_sysclk = NULL;
	struct clk *pcmclk = NULL;

	epll_clk = clk_get(&pdev->dev,"mpll");
	if (IS_ERR(epll_clk)) {
		printk(KERN_ERR"pcm get epll_clk fail\n");
		goto __err_epll_clk_get;
	}
	pcm_priv->cpm_epll_clk = clk_get_rate(epll_clk);
	pcmclk = clk_get(&pdev->dev, "pcm");
	if (IS_ERR(pcmclk)) {
		dev_dbg(&pdev->dev, "pcm clk_get failed\n");
		goto __err_pcmsys_clk_get;
	}
	clk_enable(pcmclk);

	if (pcm_priv->pcm_device_mode == PCM_MASTER)
		calculate_pcm_clk();

	if (pcm_priv->pcm_sysclk != 0) {
		pcm_sysclk = clk_get(&pdev->dev, "cgu_pcm");
		if (IS_ERR(pcm_sysclk)) {
			printk(KERN_ERR"CGU pcm clk_get failed\n");
			goto __err_pcmclk;
		}
		clk_set_rate(pcm_sysclk, pcm_priv->pcm_sysclk);
		if (clk_get_rate(pcm_sysclk) > pcm_priv->pcm_sysclk) {
			printk("codec interface set rate fail clk_get_rate(pcm_sysclk) = %ld ,pcm_priv->pcm_sysclk = %ld\n",
					clk_get_rate(pcm_sysclk),pcm_priv->pcm_sysclk);
		}
		clk_enable(pcm_sysclk);
	}

	if (pcm_priv->pcm_device_mode == PCM_MASTER) {
		__pcm_as_master();
		pcm_set_pcmclk(pcm_priv->pcmclk);
		if (pcm_priv->pcm_sync_len > (0x3f+1))
			pcm_priv->pcm_sync_len = (0x3f+1);
		__pcm_set_sync((pcm_priv->pcm_sync_len - 1) & 0x3f);
	} else
		__pcm_as_slaver();

	if (pcm_priv->pcm_slot_num < 4) {
		if (pcm_priv->pcm_slot_num == 0)
			pcm_priv->pcm_slot_num = 1;
		__pcm_set_slot(((pcm_priv->pcm_slot_num-1) & 0x3));
	}
	if (pcm_priv->pcm_imsb_pos == ONE_SHIFT_MODE)
		__pcm_set_msb_one_shift_in();
	else
		__pcm_set_msb_normal_in();

	if (pcm_priv->pcm_imsb_pos == ONE_SHIFT_MODE)
		__pcm_set_msb_one_shift_out();
	else
		__pcm_set_msb_normal_in();

	return 0;

__err_pcmclk:
	clk_put(pcm_sysclk);
__err_pcmsys_clk_get:
	clk_put(epll_clk);
__err_epll_clk_get:
	return -1;
}

static int pcm_init(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *pcm_resource = NULL;
	struct dsp_pipe *pcm_pipe_out = NULL;
	struct dsp_pipe *pcm_pipe_in = NULL;

	/* map io address */
	pcm_resource = platform_get_resource(pdev,IORESOURCE_MEM,0);
	if (pcm_resource == NULL) {
		printk("pcm: platform_get_resource fail.\n");
		return -1;
	}
	if (!request_mem_region(pcm_resource->start, resource_size(pcm_resource), pdev->name)) {
		printk("pcm: mem region fail busy .\n");
		return -EBUSY;
	}

	pcm_iomem = ioremap(pcm_resource->start, resource_size(pcm_resource));
	if (!pcm_iomem) {
		printk ("pcm: ioremap fail.\n");
		ret =  -ENOMEM;
		goto __err_ioremap;
	}

	/*init pipe*/
	ret = pcm_init_pipe(&pcm_pipe_out,DMA_TO_DEVICE,pcm_resource->start);
	if (ret < 0)
		goto __err_init_pipeout;
	ret = pcm_init_pipe(&pcm_pipe_in,DMA_FROM_DEVICE,pcm_resource->start);
	if (ret < 0)
		goto __err_init_pipein;
	pcm_endpoints.out_endpoint = pcm_pipe_out;
	pcm_endpoints.in_endpoint = pcm_pipe_in;

	/*spin lock init*/
	spin_lock_init(&pcm_irq_lock);

	/* request irq */
	pcm_resource = platform_get_resource(pdev,IORESOURCE_IRQ,0);
	if (pcm_resource == NULL) {
		ret = -1;
		printk("pcm there is not irq resource\n");
		goto __err_irq;
	}
	pcm_priv->irq = pcm_resource->start;
	ret = request_irq(pcm_resource->start, pcm_irq_handler,
			IRQF_DISABLED, "pcm_irq", NULL);
	if (ret < 0) {
		printk("pcm:request irq fail\n");
		goto __err_irq;
	}

	ret = pcm_clk_init(pdev);
	if (ret < 0)
		goto __err_clk_init;

	__pcm_disable_receive_dma();
	__pcm_disable_transmit_dma();
	__pcm_disable_record();
	__pcm_disable_replay();
	__pcm_flush_fifo();
	__pcm_clear_ror();
	__pcm_clear_tur();
	__pcm_set_receive_trigger(7);
	__pcm_set_transmit_trigger(8);
	__pcm_disable_overrun_intr();
	__pcm_disable_underrun_intr();
	__pcm_disable_transmit_intr();
	__pcm_disable_receive_intr();
	/* play zero or last sample when underflow */
	__pcm_play_lastsample();
	//__pcm_enable();

	return 0;

__err_clk_init:
	free_irq(pcm_priv->irq,NULL);
__err_irq:
	vfree(pcm_pipe_in);
__err_init_pipein:
	vfree(pcm_pipe_out);
__err_init_pipeout:
	iounmap(pcm_iomem);
__err_ioremap:
	release_mem_region(pcm_resource->start,resource_size(pcm_resource));
	return ret;
}

static void pcm_shutdown(struct platform_device *pdev)
{
	free_irq(pcm_priv->irq,NULL);
	return;
}

static int pcm_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int pcm_resume(struct platform_device *pdev)
{
	return 0;
}

struct snd_dev_data pcm_data = {
	.dev_ioctl	   	= pcm_ioctl,
	.ext_data		= &pcm_endpoints,
	.minor			= SND_DEV_DSP1,
	.init			= pcm_init,
	.shutdown		= pcm_shutdown,
	.suspend		= pcm_suspend,
	.resume			= pcm_resume,
};

struct snd_dev_data snd_mixer1_data = {
        .dev_ioctl              = pcm_ioctl,
        .minor                  = SND_DEV_MIXER1,
};

static int __init init_pcm(void)
{
	pcm_priv = (struct pcm_board_info *)vzalloc(sizeof(struct pcm_board_info));
	if (!pcm_priv)
		return -1;

	pcm_priv->rate = pcm_conf_info.pcm_sync_clk;
	pcm_priv->pcmclk = pcm_conf_info.pcm_clk;
	pcm_priv->pcm_sysclk = pcm_conf_info.pcm_sysclk;
	pcm_priv->pcm_device_mode = pcm_conf_info.pcm_device_mode;
	pcm_priv->replay_format = pcm_conf_info.iss;
	pcm_priv->record_format = pcm_conf_info.oss;
	pcm_priv->endpoint= &pcm_endpoints;
	pcm_priv->pcm_sync_len = pcm_conf_info.pcm_sync_len;
	pcm_priv->pcm_slot_num = pcm_conf_info.pcm_slot_num;
	pcm_priv->pcm_imsb_pos = pcm_conf_info.pcm_imsb_pos;
	pcm_priv->pcm_omsb_pos = pcm_conf_info.pcm_omsb_pos;
	return 0;
}
device_initcall(init_pcm);
