/**
 * xb_snd_spdif.c
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
#include <linux/earlysuspend.h>
#include <linux/wait.h>
#include <mach/jzdma.h>
#include <mach/jzsnd.h>
#include <soc/irq.h>
#include <soc/base.h>
#include "xb47xx_spdif_v12.h"
#include "xb47xx_i2s_v12.h"
#include <linux/delay.h>
#include "codecs/jz_codec_v12.h"
/**
 * global variable
 **/
void volatile __iomem *volatile spdif_iomem;
static volatile int jz_switch_state = 0;
static struct dsp_endpoints spdif_endpoints;
static struct clk *codec_sysclk = NULL;
static volatile bool spdif_is_incall_state = false;
static LIST_HEAD(codecs_head);
bool spdif_is_incall(void);

#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
/*static struct workqueue_struct *spdif_work_queue;
static struct work_struct	spdif_codec_work;*/
#endif
static int jz_spdif_get_hp_switch_state(void);
static int spdif_global_init(struct platform_device *pdev);

static struct codec_info {
	struct list_head list;
	char *name;
	unsigned long record_rate;
	unsigned long replay_rate;
	int record_codec_channel;
	int replay_codec_channel;
	int record_format;
	int replay_format;
	enum codec_mode codec_mode;
	unsigned long codec_clk;
	int (*codec_ctl)(unsigned int cmd, unsigned long arg);
	struct dsp_endpoints *dsp_endpoints;
} *cur_codec_spdif;

/*##################################################################*\
 | dump
\*##################################################################*/

static void dump_spdif_reg(void)
{
	int i;
	unsigned long reg_addr[] = {
		SPENA,SPCTRL,SPSTATE,SPCFG1,SPCFG2
	};

	for (i = 0;i < 5; i++) {
		printk("##### spdif reg0x%x,=0x%x.\n",
			(unsigned int)reg_addr[i],spdif_read_reg(reg_addr[i]));
	}
}

/*##################################################################*\
 |* suspand func
\*##################################################################*/
static int spdif_suspend(struct platform_device *, pm_message_t state);
static int spdif_resume(struct platform_device *);
static void spdif_shutdown(struct platform_device *);

/*##################################################################*\
  |* codec control
\*##################################################################*/
/**
 * register the codec
 **/

int spdif_register_codec(char *name, void *codec_ctl,unsigned long codec_clk,enum codec_mode mode)
{
	struct codec_info *tmp = vmalloc(sizeof(struct codec_info));
	if ((name != NULL) && (codec_ctl != NULL)) {
		tmp->name = name;
		tmp->codec_ctl = codec_ctl;
		tmp->codec_clk = codec_clk;
		tmp->codec_mode = mode;
		tmp->dsp_endpoints = &spdif_endpoints;
		list_add_tail(&tmp->list, &codecs_head);
		return 0;
	} else {
		return -1;
	}
}
EXPORT_SYMBOL(spdif_register_codec);

static void spdif_match_codec(char *name)
{
	struct codec_info *codec_info;
	struct list_head  *list,*tmp;

	list_for_each_safe(list,tmp,&codecs_head) {
		codec_info = container_of(list,struct codec_info,list);
		if (!strcmp(codec_info->name,name)) {
			cur_codec_spdif = codec_info;
		}
	}
}

bool spdif_is_incall(void)
{
	return spdif_is_incall_state;
}

/*##################################################################*\
|* dev_ioctl
\*##################################################################*/
static int spdif_set_fmt(unsigned long *format,int mode)
{

	int ret = 0;
	int data_width = 0;
	struct dsp_pipe *dp = NULL;

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
		data_width = 16;
		if (mode & CODEC_WMODE) {
			__spdif_set_max_wl(0);
			__spdif_set_oss_sample_size(1);
			printk("---> %s  line: %d \n", __func__, __LINE__);
		}
		break;
	case AFMT_S16_BE:
		data_width = 16;
		if (mode & CODEC_WMODE) {
			__spdif_set_max_wl(0);
			__spdif_set_oss_sample_size(1);
			printk("---> %s  line: %d \n", __func__, __LINE__);
		}
		break;
	default :
		printk("SPDIF: there is unsupport format 0x%x.\n",(unsigned int)*format);
		return -EINVAL;
	}
	if (!cur_codec_spdif)
		return -ENODEV;

	if (mode & CODEC_WMODE) {
		dp = cur_codec_spdif->dsp_endpoints->out_endpoint;
		dp->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		if (cur_codec_spdif->replay_format != *format) {
			cur_codec_spdif->replay_format = *format;
			ret |= NEED_RECONF_TRIGGER;
			ret |= NEED_RECONF_DMA;
		}
	}
	if (mode & CODEC_RMODE) {
		printk("SPDIF: unsurpport rcord!\n");
		ret = -1;
	}

	return ret;
}

/***************************************************************\
 *  Use codec slave mode clock rate list
 *  We do not hope change EPLL,so we use 270.67M (fix) epllclk
 *  for minimum error
 *  270.67M ---	M:203 N:9 OD:1
 *	 rate	 spdifdr	 cguclk		 spdifdv.div	samplerate/error
 *	|192000	|1		|135.335M	|10			|+0.12%
 *	|96000	|3		|67.6675M	|10			|+0.12%
 *	|48000	|7		|33.83375M	|10			|-0.11%
 *	|44100	|7		|33.83375M	|11			|-0.10%
 *	|32000	|11		|22.555833M	|10			|+0.12%
 *	|24000	|15		|16.916875M	|10			|+0.12%
 *	|22050	|15		|16.916875M	|11			|-0.12%
 *	|16000	|23		|11.277916M	|10			|+0.12%
 *	|12000  |31		|8.458437M	|10			|+0.12%
 *	|11025	|31		|8.458437M	|11			|-0.10%
 *	|8000	|47		|5.523877M	|10			|+0.12%
 *	HDMI:
 *	sysclk 11.2896M (theoretical)
 *	spdifdr  23
 *	cguclk 11.277916M (practical)
 *	error  -0.10%
\***************************************************************/
static unsigned long calculate_cgu_spdif_rate(unsigned long *rate)
{
	int i;
	unsigned long mrate[10] = {
		8000, 11025,16000,22050,24000,
		32000,44100,48000,96000,192000,
	};
	unsigned long mcguclk[10] = {
		5523978, 8458438, 8458438, 11277917,16916875,
		16916875,33833750,33833750,67667500,135335000,
	};
	for (i=0; i<9; i++) {
		if (*rate <= mrate[i]) {
			*rate = mrate[i];
			break;
		}
	}
	if (i >= 9) {
		*rate = 44100; /*unsupport rate use default*/
		return mcguclk[6];
	}

	return mcguclk[i];
}

static int spdif_set_rate(unsigned long *rate,int mode)
{
	int ret = 0;
	unsigned long cgu_spdif_clk = 0;
	unsigned long ori_rate = *rate;

	if (!cur_codec_spdif)
		return -ENODEV;
	debug_print("rate = %ld",*rate);
	if (mode & CODEC_WMODE) {
		/*************************************************************\
		|* WARING:spdif have only one data line,                     *|
		|* So we should not to care slave mode or master mode.		 *|
		\*************************************************************/
		cgu_spdif_clk = calculate_cgu_spdif_rate(rate);

		clk_set_rate(codec_sysclk, cgu_spdif_clk * 2);
		if (clk_get_rate(codec_sysclk) > cgu_spdif_clk * 2) {
			printk("external codec set rate fail.\n");
		}
		__i2s_stop_bitclk();
		__i2s_set_sample_rate(cgu_spdif_clk, *rate);
		__i2s_start_bitclk();
		ori_rate = __spdif_set_ori_sample_freq(ori_rate);
		*rate = __spdif_set_sample_freq(*rate);

		cur_codec_spdif->replay_rate = *rate;
	}
	if (mode & CODEC_RMODE) {
		printk("spdif unsurpport record!\n");
		return ret = -1;
	}
	return ret;
}
#define SPDIF_TX_FIFO_DEPTH		64

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

static void spdif_set_trigger(int mode)
{
	if (!cur_codec_spdif)
		return;

	if (mode & CODEC_WMODE) {
		__spdif_set_transmit_trigger(32);
	}
	if (mode &CODEC_RMODE) {
		printk("spdif unsurpport record!\n");
	}

	return;
}

static void spdif_set_i2sdeinit(void)
{
	__i2s_disable();
	__i2s_disable_transmit_dma();
	__i2s_disable_receive_dma();
	__i2s_disable_replay();
	__i2s_disable_record();
	__i2s_disable_loopback();
	printk("----> deinit i2s success\n");
}

static int spdif_replay_deinit(int mode)
{

	__spdif_disable_transmit_dma();

	return 0;
}

static int spdif_replay_init(int mode)
{
	unsigned long replay_rate = DEFAULT_REPLAY_SAMPLERATE;
	unsigned long spdif_sysclk = 0;
	int rst_test = 50000;
	int ret = 0;

	if (mode & CODEC_RMODE)
		return -1;
	spdif_sysclk = calculate_cgu_spdif_rate(&replay_rate);

	clk_set_rate(codec_sysclk, spdif_sysclk*2);

	if (clk_get_rate(codec_sysclk) > spdif_sysclk * 2) {
		printk("codec interface set rate fail.\n");
		goto __err_sys_clk;
	}
	printk("@@@@@@@@  %ld \n",clk_get_rate(codec_sysclk));

	/*clk_enable(codec_sysclk);*/

	__i2s_stop_bitclk();
	__i2s_external_codec();
	__i2s_bclk_output();
	__i2s_sync_output();
	__aic_select_i2s();
	__i2s_send_rfirst();
	__i2s_set_sample_rate(spdif_sysclk, replay_rate);
	__i2s_start_bitclk();

	spdif_set_trigger(CODEC_WMODE);
	__spdif_set_dtype(0);
	__spdif_set_ch1num(0);
	__spdif_set_ch2num(1);
	__spdif_set_srcnum(0);

	__spdif_set_transmit_trigger(32);
	/*select spdif trans*/
	__interface_select_spdif();

	__spdif_play_lastsample();
	__spdif_init_set_low();

	__spdif_choose_consumer();
	__spdif_clear_audion();
	__spdif_set_copyn();
	__spdif_clear_pre();
	__spdif_choose_chmd();
	__spdif_set_category_code_normal();
	__spdif_set_clkacu(0);
	__spdif_set_max_wl(0);
	__spdif_set_oss_sample_size(1);
	__spdif_set_ori_sample_freq(replay_rate);
	__spdif_set_sample_freq(replay_rate);

	__spdif_enable_transmit_dma();
	__spdif_clear_signn();
	__spdif_reset();
	while(__spdif_get_reset()) {
		if (rst_test-- <= 0) {
			printk("-----> rest spdif failed!\n");
			goto __err_reset;
			break;
		}
	}
	__spdif_set_valid();
	__spdif_mask_trig();
	__spdif_disable_underrun_intr();

	printk("=====> %s success\n", __func__);

	return 0;

__err_reset:
	printk("----> reset spdif failed!\n");
__err_sys_clk:
	printk("----> set spdif sysclk failed!\n");

	return ret;
}

static int spdif_enable(int mode)
{
	unsigned long replay_rate = DEFAULT_REPLAY_SAMPLERATE;
	unsigned long replay_format = 16;
	/*int replay_channel = DEFAULT_REPLAY_CHANNEL;*/
//	int record_channel = DEFAULT_RECORD_CHANNEL;
	struct dsp_pipe *dp_other = NULL;
	int ret = 0;

	if (!cur_codec_spdif)
			return -ENODEV;

	if (mode & CODEC_RMODE)
		return -1;

	spdif_set_i2sdeinit();

	ret = spdif_replay_init(mode);
	if (ret)
		return -1;

	if (mode & CODEC_WMODE) {
		printk("come to %s %d set dp_other\n", __func__, __LINE__);
		dp_other = cur_codec_spdif->dsp_endpoints->in_endpoint;
		spdif_set_fmt(&replay_format,mode);
		spdif_set_rate(&replay_rate,mode);
	}

	if (!dp_other->is_used) {
		__spdif_enable();
	}

	spdif_is_incall_state = false;

	return 0;
}

static int spdif_disable(int mode)
{
	spdif_replay_deinit(mode);
	__spdif_disable();

	return 0;
}

static int spdif_dma_enable(int mode)		//CHECK
{
	if (!cur_codec_spdif)
			return -ENODEV;
	if (mode & CODEC_WMODE) {
		__spdif_reset();
		while(__spdif_get_reset());

		__spdif_set_transmit_trigger(32);
		__i2s_enable_transmit_dma();
		__spdif_enable_transmit_dma();
		__i2s_enable_replay();
		__i2s_enable();
		__spdif_clear_tur();
	}
	if (mode & CODEC_RMODE) {
		return -1;
	}

	return 0;
}

static int spdif_dma_disable(int mode)		//CHECK seq dma and func
{
	if (mode & CODEC_WMODE) {
		__i2s_disable();
		__i2s_disable_replay();
		__spdif_disable_transmit_dma();
		__i2s_disable_transmit_dma();
	}
	else
		return -1;

	if (!cur_codec_spdif)
			return -ENODEV;

	return 0;
}

static int spdif_get_fmt_cap(unsigned long *fmt_cap,int mode)
{
	unsigned long spdif_fmt_cap = 0;
	if (!cur_codec_spdif)
			return -ENODEV;
	if (mode & CODEC_WMODE) {
		spdif_fmt_cap |= AFMT_S16_LE|AFMT_S16_BE;
	}
	if (mode & CODEC_RMODE) {
		return -1;
	}

	if (*fmt_cap == 0)
		*fmt_cap = spdif_fmt_cap;
	else
		*fmt_cap &= spdif_fmt_cap;

	return 0;
}


static int spdif_get_fmt(unsigned long *fmt, int mode)
{
	if (!cur_codec_spdif)
			return -ENODEV;

	if (mode & CODEC_WMODE)
		*fmt = cur_codec_spdif->replay_format;
	if (mode & CODEC_RMODE)
		return -1;

	return 0;
}

static void spdif_dma_need_reconfig(int mode)
{
	struct dsp_pipe	*dp = NULL;

	if (!cur_codec_spdif)
			return;
	if (mode & CODEC_WMODE) {
		dp = cur_codec_spdif->dsp_endpoints->out_endpoint;
		dp->need_reconfig_dma = true;
	}
	if (mode & CODEC_RMODE)
		printk("SPDIF: unsurpport record mode\n");

	return;
}


static int spdif_set_device(unsigned long device)
{
	unsigned long tmp_rate = 44100;
	int ret = 0;
	struct dsp_endpoints *endpoints = NULL;
	struct dsp_pipe *dp = NULL;

	endpoints = (struct dsp_endpoints *)((&spdif_data)->ext_data);
	dp = endpoints->out_endpoint;

	if (!cur_codec_spdif)
		return -1;

	/*call state operation*/
	if (*(enum snd_device_t *)device >= SND_DEVICE_CALL_START &&
            *(enum snd_device_t *)device <= SND_DEVICE_CALL_END)
		spdif_is_incall_state = true;
	else
		spdif_is_incall_state = false;

	if (*(enum snd_device_t *)device == SND_DEVICE_LOOP_TEST) {
		spdif_set_rate(&tmp_rate,CODEC_RWMODE);
	}

	/*hdmi operation*/
	if ((tmp_rate = cur_codec_spdif->replay_rate) == 0);
		tmp_rate = 44100;
	if (strcmp(cur_codec_spdif->name,"hdmi")) {
		spdif_match_codec("hdmi");
		spdif_set_rate(&tmp_rate,CODEC_WMODE);
		printk("----> set hdmi virtual codec\n");
	}

	return ret;
}

/********************************************************\
 * dev_ioctl
\********************************************************/
static long spdif_ioctl(unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	switch (cmd) {
	case SND_DSP_ENABLE_REPLAY:
		/* enable spdif replay */
		/* set spdif default record format, channels, rate */
		/* set default replay route */
		printk("  spdif ioctl enable cmd\n");
		ret = spdif_enable(CODEC_WMODE);
		printk("  spdif ioctl enable cmd complete\n");
		break;

	case SND_DSP_DISABLE_REPLAY:
		/* disable spdif replay */
		ret = spdif_disable(CODEC_WMODE);
		break;

	case SND_DSP_ENABLE_RECORD:
		/* enable spdif record */
		/* set spdif default record format, channels, rate */
		/* set default record route */
		printk("spdif not support record!\n");
		break;

	case SND_DSP_DISABLE_RECORD:
		/* disable spdif record */
		printk("spdif not support record!\n");
		ret = -1;
		break;

	case SND_DSP_ENABLE_DMA_RX:
		printk("spdif not support record!\n");
		ret = -1;
		break;

	case SND_DSP_DISABLE_DMA_RX:
		printk("spdif not support record!\n");
		ret = -1;
		break;

	case SND_DSP_ENABLE_DMA_TX:
		ret = spdif_dma_enable(CODEC_WMODE);
		break;

	case SND_DSP_DISABLE_DMA_TX:
		ret = spdif_dma_disable(CODEC_WMODE);
		break;

	case SND_DSP_SET_REPLAY_RATE:
		ret = spdif_set_rate((unsigned long *)arg,CODEC_WMODE);
		break;

	case SND_DSP_SET_RECORD_RATE:
		ret = -1;
		printk("spdif not support record!\n");
		break;

	case SND_DSP_GET_REPLAY_RATE:
		/*if (cur_codec_spdif && cur_codec_spdif->replay_rate)
			*(unsigned long *)arg = cur_codec_spdif->replay_rate;*/
		ret = 0;
		break;

	case SND_DSP_GET_RECORD_RATE:
		ret = -1;
		printk("spdif not support record!\n");
		break;


	case SND_DSP_SET_REPLAY_CHANNELS:
		/* set replay channels */
		ret = 0;
		break;

	case SND_DSP_SET_RECORD_CHANNELS:
		/* set record channels */
		printk("spdif not support record!\n");
		ret = -1;
		break;

	case SND_DSP_GET_REPLAY_CHANNELS:
		if (cur_codec_spdif && cur_codec_spdif->replay_codec_channel)
			*(unsigned long *)arg = cur_codec_spdif->replay_codec_channel;
		ret = 0;
		break;

	case SND_DSP_GET_RECORD_CHANNELS:
		printk("spdif not support record!\n");
		ret = -1;
		break;

	case SND_DSP_GET_REPLAY_FMT_CAP:
		/* return the support replay formats */
		ret = spdif_get_fmt_cap((unsigned long *)arg,CODEC_WMODE);
		break;

	case SND_DSP_GET_REPLAY_FMT:
		/* get current replay format */
		ret = spdif_get_fmt((unsigned long *)arg,CODEC_WMODE);
		break;

	case SND_DSP_SET_REPLAY_FMT:
		/* set replay format */
		ret = spdif_set_fmt((unsigned long *)arg,CODEC_WMODE);
		if (ret < 0)
			break;
		/* if need reconfig the trigger, reconfig it */
		if (ret & NEED_RECONF_TRIGGER)
			spdif_set_trigger(CODEC_WMODE);
		/* if need reconfig the dma_slave.max_tsz, reconfig it and
		   set the dp->need_reconfig_dma as true */
		if (ret & NEED_RECONF_DMA)
			spdif_dma_need_reconfig(CODEC_WMODE);
		ret = 0;
		break;

	case SND_DSP_GET_RECORD_FMT_CAP:
		/* return the support record formats */
		ret = -1;
		printk("spdif not support record!\n");
		break;

	case SND_DSP_GET_RECORD_FMT:
		/* get current record format */
		ret = -1;
		printk("spdif not support record!\n");

		break;

	case SND_DSP_SET_RECORD_FMT:
		/* set record format */
		printk("spdif not support record!\n");
		ret = -1;
		break;

	case SND_MIXER_DUMP_REG:
		dump_spdif_reg();
		break;
	case SND_MIXER_DUMP_GPIO:
		break;

	case SND_DSP_SET_STANDBY:
		break;

	case SND_DSP_SET_DEVICE:
		ret = spdif_set_device(arg);
		break;
	case SND_DSP_GET_HP_DETECT:
		*(int*)arg = jz_spdif_get_hp_switch_state();
		ret = 0;
		break;
	case SND_DSP_SET_RECORD_VOL:
		/*if (cur_codec_spdif)
			ret = cur_codec_spdif->codec_ctl(CODEC_SET_RECORD_VOLUME, arg);*/
		break;
	case SND_DSP_SET_REPLAY_VOL:
		/*if (cur_codec_spdif)
			ret = cur_codec_spdif->codec_ctl(CODEC_SET_REPLAY_VOLUME, arg);*/
		break;
	case SND_DSP_SET_MIC_VOL:
		/*if (cur_codec_spdif)
			ret = cur_codec_spdif->codec_ctl(CODEC_SET_MIC_VOLUME, arg);*/
		break;
	case SND_DSP_CLR_ROUTE:
		/*if (cur_codec_spdif)
			ret = cur_codec_spdif->codec_ctl(CODEC_CLR_ROUTE,arg);*/
		break;
	case SND_DSP_DEBUG:
		/*if (cur_codec_spdif)
			ret = cur_codec_spdif->codec_ctl(CODEC_DEBUG,arg);*/
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

/*static irqreturn_t spdif_irq_handler(int irq, void *dev_id)
{
	unsigned long flags;
	irqreturn_t ret = IRQ_HANDLED;
	spin_lock_irqsave(&spdif_irq_lock,flags);
	if (__spdif_test_tur()) {
		printk(" SOUND: underrun happen!\n");
		__spdif_clear_tur();
	}

	spin_unlock_irqrestore(&spdif_irq_lock,flags);

	return ret;
}*/

static int spdif_init_pipe(struct dsp_pipe **dp , enum dma_data_direction direction,unsigned long iobase)
{
	if (*dp != NULL || dp == NULL)
		return 0;
	*dp = vmalloc(sizeof(struct dsp_pipe));
	if (*dp == NULL) {
		return -ENOMEM;
	}

	(*dp)->dma_config.direction = direction;
	(*dp)->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	(*dp)->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	(*dp)->dma_type = JZDMA_REQ_I2S0;
	(*dp)->fragsize = FRAGSIZE_M;
	(*dp)->fragcnt = FRAGCNT_L;

	if (direction == DMA_TO_DEVICE) {
		(*dp)->dma_config.src_maxburst = 64;
		(*dp)->dma_config.dst_maxburst = 64;
		(*dp)->dma_config.dst_addr = iobase + SPFIFO;
		(*dp)->dma_config.src_addr = 0;
	} else if (direction == DMA_FROM_DEVICE) {
		(*dp)->dma_config.src_maxburst = 32;
		(*dp)->dma_config.dst_maxburst = 32;
		(*dp)->dma_config.src_addr = iobase + AICDR;
		(*dp)->dma_config.dst_addr = 0;
	} else
		return -1;

	return 0;
}

static int spdif_global_init(struct platform_device *pdev)
{
	int ret = 0;
	unsigned long sample_rate = DEFAULT_REPLAY_SAMPLERATE;
	unsigned long spdif_sysclk = 0;
	struct resource *spdif_resource = NULL;
	struct dsp_pipe *spdif_pipe_out = NULL;
	struct dsp_pipe *spdif_pipe_in = NULL;
	struct clk *spdif_clk = NULL;

	printk("----> start %s\n", __func__);

	spdif_resource = platform_get_resource(pdev,IORESOURCE_MEM,0);
	if (spdif_resource == NULL) {
		printk("%s spdif_resource get failed!\n", __func__);
		return -1;
	}

	/* map io address */
	if (!request_mem_region(spdif_resource->start, resource_size(spdif_resource), pdev->name)) {
		printk("%s request mem region failed!\n", __func__);
		return -EBUSY;
	}
	spdif_iomem = ioremap(spdif_resource->start, resource_size(spdif_resource));
	if (!spdif_iomem) {
		printk("%s ioremap failed!\n", __func__);
		ret =  -ENOMEM;
		goto __err_ioremap;
	}

	ret = spdif_init_pipe(&spdif_pipe_out,DMA_TO_DEVICE,spdif_resource->start);
	if (ret < 0) {
		printk("%s init write pipe failed!\n", __func__);
		goto __err_init_pipeout;
	}
	ret = spdif_init_pipe(&spdif_pipe_in,DMA_FROM_DEVICE,spdif_resource->start);
	if (ret < 0) {
		printk("%s init read pipe failed!\n", __func__);
		goto __err_init_pipein;
	}

	spdif_endpoints.out_endpoint = spdif_pipe_out;
	spdif_endpoints.in_endpoint = spdif_pipe_in;

	/*request aic clk */
	spdif_clk = clk_get(&pdev->dev, "aic");
	if (IS_ERR(spdif_clk)) {
		dev_err(&pdev->dev, "----> aic clk_get failed\n");
		goto __err_aic_clk;
	}


	spdif_match_codec("hdmi");
	if (cur_codec_spdif == NULL) {
		printk("----> match codec error\n");
		ret = -1;
		goto __err_match_codec;
	}
	/* request irq */
	/*spdif_resource = platform_get_resource(pdev,IORESOURCE_IRQ,0);
	if (spdif_resource == NULL) {
		printk("----> get resource error\n");
		ret = -1;
		goto __err_irq;
	}

	ret = request_irq(spdif_resource->start, spdif_irq_handler,
					  IRQF_DISABLED, "aic_irq", NULL);
	if (ret < 0) {
		printk("----> request irq error\n");
		goto __err_irq;
	}*/


	__spdif_disable();
	schedule_timeout(5);
	__spdif_disable();


	/*set sysclk output for codec*/
	codec_sysclk = clk_get(&pdev->dev,"cgu_aic");
	if (IS_ERR(codec_sysclk)) {
		dev_dbg(&pdev->dev, "cgu_spdif clk_get failed\n");
		goto __err_codec_clk;
	}
	/*set sysclk output for codec*/
	spdif_sysclk = calculate_cgu_spdif_rate(&sample_rate);


	printk("spdif init success.\n");
	return  0;

__err_codec_clk:
	clk_put(codec_sysclk);
/*__err_irq:*/
	clk_put(spdif_clk);
__err_aic_clk:
__err_match_codec:
__err_init_pipein:
	vfree(spdif_pipe_out);
__err_init_pipeout:
	iounmap(spdif_iomem);
__err_ioremap:
	release_mem_region(spdif_resource->start,resource_size(spdif_resource));
	return ret;
}

static int spdif_init(struct platform_device *pdev)
{
	int ret = 0;

	ret = spdif_global_init(pdev);
	if (ret)
		printk("spdif init error!\n");

	return ret;
}

static void spdif_shutdown(struct platform_device *pdev)
{
	/* close spdif and current codec */

	__spdif_disable();
	return;
}

static int spdif_suspend(struct platform_device *pdev, pm_message_t state)
{

	__spdif_disable();
	return 0;
}

static int spdif_resume(struct platform_device *pdev)
{
	__spdif_enable();
	return 0;
}

/*
 * headphone detect switch function
 *
 */
static int jz_spdif_get_hp_switch_state(void)
{
	return 0;
}

struct snd_dev_data spdif_data = {
	.dev_ioctl	   	= spdif_ioctl,
	.ext_data		= &spdif_endpoints,
	.minor			= SND_DEV_DSP2,
	.init			= spdif_init,
	.shutdown		= spdif_shutdown,
	.suspend		= spdif_suspend,
	.resume			= spdif_resume,
};

struct snd_dev_data snd_mixer2_data = {
	.dev_ioctl	   	= spdif_ioctl,
	.minor			= SND_DEV_MIXER2,
};

static int __init init_spdif(void)
{
	printk("-----> come to this %s\n", __func__);

	return 0;
}
module_init(init_spdif);
