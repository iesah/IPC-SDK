/**
 * xb_snd_i2s.c
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
#include "xb47xx_i2s_v12.h"
#include "codecs/jz_codec_v12.h"

/* to be modify */
void volatile __iomem *volatile i2s_iomem;

/* head phone detect */
static LIST_HEAD(codecs_head);
struct snd_switch_data switch_data;
static struct platform_device xb47xx_i2s_switch;


static struct i2s_device * g_i2s_dev;

#if 0
extern void codec_irq_set_mask(struct codec_info *codec_dev);
#endif

static int jz_get_hp_switch_state(struct i2s_device * i2s_dev);


/* for interface/dsp and interface/mixer */
static int i2s_set_private_data(struct snd_dev_data *ddata, struct i2s_device * i2s_dev)
{
	ddata->priv_data = (void *)i2s_dev;
	return 0;
}
static struct i2s_device * i2s_get_private_data(struct snd_dev_data *ddata)
{
	return (struct i2s_device *)ddata->priv_data;
}

static struct snd_dev_data *i2s_get_ddata(struct platform_device * pdev)
{
	return pdev->dev.platform_data;
}
/* for hp detect */
static int i2s_set_switch_data(struct snd_switch_data *switch_data, struct i2s_device *i2s_dev)
{

	switch_data->priv_data = (void *)i2s_dev;
	return 0;
}
static struct i2s_device *i2s_get_switch_data(struct snd_switch_data *switch_data)
{
	return (struct i2s_device *)switch_data->priv_data;
}

/*##################################################################*\
 | dump
\*##################################################################*/

static void dump_i2s_reg(struct i2s_device *i2s_dev)
{
	int i;
	unsigned long reg_addr[] = {
		AICFR,AICCR,I2SCR,AICSR,I2SSR,I2SDIV
	};

	for (i = 0;i < 6; i++) {
		printk("##### aic reg0x%x,=0x%x.\n",
			(unsigned int)reg_addr[i],i2s_read_reg(i2s_dev, reg_addr[i]));
	}
}

/*##################################################################*\
 |* suspand func
\*##################################################################*/
static int i2s_suspend(struct platform_device *, pm_message_t state);
static int i2s_resume(struct platform_device *);
static void i2s_shutdown(struct platform_device *);

/*##################################################################*\
  |* codec control
\*##################################################################*/
/**
 * register the codec
 **/
static int codec_ctrl(struct codec_info *cur_codec, unsigned int cmd, unsigned long arg)
{
	if(cur_codec->codec_ctl_2) {
		return cur_codec->codec_ctl_2(cur_codec, cmd, arg);
	} else {
		return cur_codec->codec_ctl(cmd, arg);
	}
}

int i2s_register_codec(char *name, void *codec_ctl,unsigned long codec_clk,enum codec_mode mode)
{	/*to be modify*/
	struct codec_info *tmp = vmalloc(sizeof(struct codec_info));
	if ((name != NULL) && (codec_ctl != NULL)) {
		//tmp->name = name;
		memcpy(tmp->name, name, sizeof(tmp->name));
		tmp->codec_ctl = codec_ctl;
		tmp->codec_clk = codec_clk;
		tmp->codec_ctl_2 = NULL;
		tmp->codec_mode = mode;
		list_add_tail(&tmp->list, &codecs_head);
		printk("i2s register ,codec is :%p\n", tmp);
		return 0;
	} else {
		return -1;
	}
}
EXPORT_SYMBOL(i2s_register_codec);

int i2s_register_codec_2(struct codec_info * codec_dev)
{
	if(!codec_dev) {
		printk("codec_dev register failed\n");
		return -EINVAL;
	}
	list_add_tail(&codec_dev->list, &codecs_head);
	return 0;
}
EXPORT_SYMBOL(i2s_register_codec_2);

static void i2s_match_codec(struct i2s_device *i2s_dev, char *name)
{
	struct codec_info *codec_info;
	struct list_head  *list,*tmp;

	list_for_each_safe(list,tmp,&codecs_head) {
		codec_info = container_of(list,struct codec_info,list);
		if (!strcmp(codec_info->name,name)) {
			codec_info->codec_parent = i2s_dev;
			codec_info->dsp_endpoints = i2s_dev->i2s_endpoints;
			i2s_dev->cur_codec = codec_info;
			printk("current codec is :%p\n", codec_info);
			return;
		}
	}
}

bool i2s_is_incall_1(struct i2s_device *i2s_dev)
{
	return i2s_dev->i2s_is_incall_state;
}

/*##################################################################*\
|* filter opt
\*##################################################################*/
static void i2s_set_filter(struct i2s_device *i2s_dev, int mode , uint32_t channels)
{
	struct dsp_pipe *dp = NULL;
	struct codec_info * cur_codec = i2s_dev->cur_codec;

	if (mode & CODEC_RMODE)
		dp = cur_codec->dsp_endpoints->in_endpoint;
	else
		return;

	switch(cur_codec->record_format) {
		case AFMT_U8:
			if (channels == 1) {
				dp->filter = convert_8bits_stereo2mono_signed2unsigned;
				printk("dp->filter convert_8bits_stereo2mono_signed2unsigned .\n");
			}
			else {
				//dp->filter = convert_8bits_signed2unsigned;
				dp->filter = NULL; //hardware convert
				printk("dp->filter convert_8bits_signed2unsigned.\n");
			}
			break;
		case AFMT_S8:
			if (channels == 1) {
				dp->filter = convert_8bits_stereo2mono;
				printk("dp->filter convert_8bits_stereo2mono\n");
			}
			else {
				dp->filter = NULL;
				printk("dp->filter null\n");
			}
			break;
		case AFMT_U16_LE:
		case AFMT_S16_LE:
			if (channels == 1) {
#if (defined(CONFIG_SOC_T10) || defined(CONFIG_SOC_T20) || defined(CONFIG_SOC_T30))
				dp->filter = convert_16bits_stereo2mono_inno;
#else
				dp->filter = convert_16bits_stereo2mono;
#endif
				printk("dp->filter convert_16bits_stereo2mono\n");
			}
			else {
				dp->filter = NULL;
				printk("dp->filter null\n");
			}
			break;
		default :
			dp->filter = NULL;
			printk("AUDIO DEVICE :filter set error.\n");
	}
}

/*##################################################################*\
|* dev_ioctl
\*##################################################################*/
static int i2s_set_fmt(struct i2s_device *i2s_dev, unsigned long *format,int mode)
{
	int ret = 0;
	int data_width = 0;
	struct dsp_pipe *dp = NULL;
	struct codec_info * cur_codec = i2s_dev->cur_codec;
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
	case AFMT_U8:
		data_width = 8;
		if (mode & CODEC_WMODE) {
			__i2s_set_oss_sample_size(i2s_dev, 0);
			__i2s_disable_byteswap(i2s_dev);
		}
		if (mode & CODEC_RMODE) {
			__i2s_set_iss_sample_size(i2s_dev, 0);
		}
		__i2s_enable_signadj(i2s_dev);
		break;
	case AFMT_S8:
		data_width = 8;
		if (mode & CODEC_WMODE) {
			__i2s_set_oss_sample_size(i2s_dev, 0);
			__i2s_disable_byteswap(i2s_dev);
		}
		if (mode & CODEC_RMODE)
			__i2s_set_iss_sample_size(i2s_dev, 0);
		__i2s_disable_signadj(i2s_dev);
		break;
	case AFMT_S16_LE:
		data_width = 16;
		if (mode & CODEC_WMODE) {
			__i2s_set_oss_sample_size(i2s_dev, 1);
			__i2s_disable_byteswap(i2s_dev);
		}
		if (mode & CODEC_RMODE)
			__i2s_set_iss_sample_size(i2s_dev, 1);
		__i2s_disable_signadj(i2s_dev);
		break;
        case AFMT_U16_LE:
                data_width = 16;
                if (mode & CODEC_WMODE) {
                        __i2s_set_oss_sample_size(i2s_dev, 1);
                        __i2s_disable_byteswap(i2s_dev);
                }
                if (mode & CODEC_RMODE)
                        __i2s_set_iss_sample_size(i2s_dev, 1);
                __i2s_enable_signadj(i2s_dev);
                break;
	case AFMT_S16_BE:
		data_width = 16;
		if (mode & CODEC_WMODE) {
			__i2s_set_oss_sample_size(i2s_dev, 1);
			__i2s_enable_byteswap(i2s_dev);
		}
		if (mode == CODEC_RMODE) {
			__i2s_set_iss_sample_size(i2s_dev, 1);
			*format = AFMT_S16_LE;
		}
		__i2s_disable_signadj(i2s_dev);
		break;
	default :
		printk("I2S: there is unknown format 0x%x.\n",(unsigned int)*format);
		return -EINVAL;
	}
	if (!cur_codec)
		return -ENODEV;

	if (mode & CODEC_WMODE) {
		dp = cur_codec->dsp_endpoints->out_endpoint;
		ret = codec_ctrl(cur_codec, CODEC_SET_REPLAY_DATA_WIDTH, data_width);
		if(ret < 0) {
			printk("JZ I2S: CODEC ioctl error, command: CODEC_SET_REPLAY_FORMAT");
			return ret;
		}
                if(data_width == 8)
                        dp->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
                else if(data_width == 16)
                        dp->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
                else
                        dp->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;

		if (cur_codec->replay_format != *format) {
			cur_codec->replay_format = *format;
			ret |= NEED_RECONF_TRIGGER;
			ret |= NEED_RECONF_DMA;
		}
	}

	if (mode & CODEC_RMODE) {
		dp = cur_codec->dsp_endpoints->in_endpoint;
		ret = codec_ctrl(cur_codec, CODEC_SET_RECORD_DATA_WIDTH, data_width);
		if(ret != 0) {
			printk("JZ I2S: CODEC ioctl error, command: CODEC_SET_RECORD_FORMAT");
			return ret;
		}

                if(data_width == 8)
                        dp->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
                else if(data_width == 16)
                        dp->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
                else
                        dp->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;

		if (cur_codec->record_format != *format) {
			cur_codec->record_format = *format;
			ret |= NEED_RECONF_TRIGGER | NEED_RECONF_FILTER;
			ret |= NEED_RECONF_DMA;
		}
	}

	return ret;
}

static int i2s_set_channel(struct i2s_device * i2s_dev, int* channel,int mode)
{
	int ret = 0;
#ifdef CONFIG_ANDROID
	int channel_save = 0;
#endif
	struct codec_info * cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
		return -ENODEV;
	debug_print("channel = %d",*channel);
	if (mode & CODEC_WMODE) {
			ret = codec_ctrl(cur_codec, CODEC_SET_REPLAY_CHANNEL,(unsigned long)channel);
		if (ret < 0) {
			cur_codec->replay_codec_channel = *channel;
			return ret;
		}
		if (*channel ==2 || *channel == 4||
			*channel ==6 || *channel == 8) {
			__i2s_out_channel_select(i2s_dev, *channel - 1);
			__i2s_disable_mono2stereo(i2s_dev);
		} else if (*channel == 1) {
			__i2s_out_channel_select(i2s_dev, *channel - 1);	//FIXME
			__i2s_enable_mono2stereo(i2s_dev);
		} else
			return -EINVAL;
		if (cur_codec->replay_codec_channel != *channel) {
			cur_codec->replay_codec_channel = *channel;
			ret |= NEED_RECONF_FILTER;
		}
	}
	if (mode & CODEC_RMODE) {
#ifdef CONFIG_ANDROID
		{
			channel_save = *channel;
			if (*channel == 1)
				*channel = 2;
		}
#endif
			ret = codec_ctrl(cur_codec, CODEC_SET_RECORD_CHANNEL,(unsigned long)channel);
		if (ret < 0)
			return ret;
		ret = 0;
#ifdef CONFIG_ANDROID
		*channel = channel_save;
#endif
		if (cur_codec->record_codec_channel != *channel) {
			cur_codec->record_codec_channel = *channel;
			ret |= NEED_RECONF_FILTER;
		}
	}
	return ret;
}

/***************************************************************\
 *  Use codec slave mode clock rate list
 *  We do not hope change EPLL,so we use 270.67M (fix) epllclk
 *  for minimum error
 *  270.67M ---	M:203 N:9 OD:1
 *	 rate	 i2sdr	 cguclk		 i2sdv.div	samplerate/error
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
 *	i2sdr  23
 *	cguclk 11.277916M (practical)
 *	error  -0.10%
 *  If using internal codec, EPLL should be 204MHz to divide 12MHz SYSCLK
 *  If using external codec and AIC support BITCLK, EPLL should be 270MHz to divide exact sample rate
\***************************************************************/
static unsigned long calculate_cgu_aic_rate(struct i2s_device * i2s_dev, unsigned long *rate)
{
	int i;
#if 1
	unsigned long mrate[12] = {
		8000, 11025, 12000, 16000,22050,24000,
		32000,44100, 48000, 88200,96000,192000,
	};

	unsigned long mcguclk[12] = {
		8192000, 11333338, 12288000, 8192000, 11333338, 12288000,
		8192000, 11333338, 12288000, 11333338,12288000, 25500000,
	};
#else
	unsigned long mrate[10] = {
		8000, 11025,16000,22050,24000,
		32000,44100,48000,96000,192000,
	};
	unsigned long mcguclk[10] = {
		5523978, 8458438, 8458438, 11277917,16916875,
		16916875,33833750,33833750,67667500,135335000,
	};
#endif
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

#if 0
static void calculate_cgu_i2s_rate(unsigned long *rate)
{
	int i = 0;
	int cpm_val = 0;
	/* Fin = MPLL : 1228.8M */
	/* M: I2SCDR Bit 13 ~ Bit 21, N: I2SCDR Bit 0  ~ Bit 12 */
	unsigned int M[] = {  1,   1,   1,   1,   1,   1,   1};
	unsigned int N[] = {300, 400, 300, 200, 150, 109, 100};
	double div_rate[] = {2.048, 3.072, 4.096, 6.114, 8.192, 11.273, 12.288};

	unsigned int crate[] = {
		8000, 12000, 16000, 24000, 32000, 44100, 48000
	};

	for (i=0; i<7; i++) {
		if (*rate <= crate[i]) {
			*rate = crate[i];
			break;
		}
	}

	if (i == 7)
		i = 0;
	printk("%s, %d: rate = %ld, i = %d\n", __func__, __LINE__, *rate, i);

	cpm_val = *((volatile unsigned int *)0xb0000060);
	/*cpm_val = readl( ((void __iomem *)0x10000000) + 0x60);*/
	cpm_val &= 0xffc00000;
	cpm_val = cpm_val | (1 << 31) | (1 << 29) | (M[i] << 13) | (N[i] << 0);
	*((volatile unsigned int *)0xb0000060) = cpm_val;
	/*writel(cpm_val, ((void __iomem *)0x10000000) + 0x60);*/

	/*writel(0, ((void __iomem *)0x10000000) + 0x70);*/

	return;
}
#endif

static int i2s_set_rate(struct i2s_device * i2s_dev, unsigned long *rate,int mode)
{
	int ret = 0;
	unsigned long cgu_aic_clk = 0;
	struct codec_info *cur_codec = i2s_dev->cur_codec;

	if (!cur_codec)
		return -ENODEV;
	debug_print("rate = %ld",*rate);
	clk_set_rate(i2s_dev->i2s_clk, (*rate) * 256);
	printk(KERN_DEBUG"%s, i2s clk rate is %ld\n", __func__, clk_get_rate(i2s_dev->i2s_clk));
	clk_enable(i2s_dev->i2s_clk);
	if (mode & CODEC_WMODE) {
		if (cur_codec->codec_mode == CODEC_SLAVE) {
			/*************************************************************\
			|* WARING:when use codec slave mode ,                        *|
			|* EPLL must be output 270.67M clk to support all sample rate*|
			|* SYSCLK not standard over sample rate clock ,so it would   *|
			|* not be output to external codec                           *|
			\*************************************************************/
			if (strcmp(cur_codec->name,"hdmi"))
				cgu_aic_clk = calculate_cgu_aic_rate(i2s_dev, rate);
			else
				cgu_aic_clk = cur_codec->codec_clk;
			__i2s_stop_bitclk(i2s_dev);
			if (cur_codec->codec_clk != cgu_aic_clk || !strcmp(cur_codec->name,"hdmi")) {
				cur_codec->codec_clk = cgu_aic_clk;
				if (i2s_dev->i2s_clk == NULL)
					return -1;
				clk_set_rate(i2s_dev->i2s_clk, cur_codec->codec_clk);
				if (clk_get_rate(i2s_dev->i2s_clk) > cur_codec->codec_clk) {
					printk("external codec set rate fail.\n");
				}
			}
			*rate = __i2s_set_sample_rate(i2s_dev, cur_codec->codec_clk,*rate);
			__i2s_start_bitclk(i2s_dev);
		}
		ret = codec_ctrl(cur_codec, CODEC_SET_REPLAY_RATE,(unsigned long)rate);
		cur_codec->replay_rate = *rate;
	}
	if (mode & CODEC_RMODE) {
		if (cur_codec->codec_mode == CODEC_SLAVE) {
			cgu_aic_clk = calculate_cgu_aic_rate(i2s_dev, rate);
			if (strcmp(cur_codec->name,"hdmi"))
				return 0;
			printk("------ come to %s %d\n", __func__, __LINE__);
			__i2s_stop_ibitclk(i2s_dev);
			if (cur_codec->codec_clk != cgu_aic_clk) {
				cur_codec->codec_clk = cgu_aic_clk;
				if (i2s_dev->i2s_clk == NULL)
					return -1;
				clk_set_rate(i2s_dev->i2s_clk, cur_codec->codec_clk);
				if (clk_get_rate(i2s_dev->i2s_clk) > cur_codec->codec_clk) {
					printk("external codec set rate fail.\n");
				}
			}
			*rate = __i2s_set_isample_rate(i2s_dev, cur_codec->codec_clk,*rate);
			__i2s_start_ibitclk(i2s_dev);
		}
		ret = codec_ctrl(cur_codec, CODEC_SET_RECORD_RATE,(unsigned long)rate);
		cur_codec->record_rate = *rate;
	}
	return ret;
}
#define I2S_TX_FIFO_DEPTH		64
#define I2S_RX_FIFO_DEPTH		32

static int get_burst_length(struct i2s_device *i2s_dev, unsigned long val)
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

static void i2s_set_trigger(struct i2s_device * i2s_dev, int mode)
{
	int data_width = 0;
	struct dsp_pipe *dp = NULL;
	int burst_length = 0;
	struct codec_info *cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
		return;

	if (mode & CODEC_WMODE) {
		switch(cur_codec->replay_format) {
		case AFMT_S8:
		case AFMT_U8:
			data_width = 8;
			break;
		default:
		case AFMT_S16_BE:
		case AFMT_U16_BE:
		case AFMT_S16_LE:
		case AFMT_U16_LE:
			data_width = 16;
			break;
		}
		dp = cur_codec->dsp_endpoints->out_endpoint;
		burst_length = get_burst_length(i2s_dev, (int)dp->paddr|(int)dp->fragsize|
				dp->dma_config.src_maxburst|dp->dma_config.dst_maxburst);
		if (I2S_TX_FIFO_DEPTH - burst_length/data_width >= 60)
			__i2s_set_transmit_trigger(i2s_dev, 30);
		else
			__i2s_set_transmit_trigger(i2s_dev, (I2S_TX_FIFO_DEPTH - burst_length/data_width) >> 1);

	}
	if (mode &CODEC_RMODE) {
		switch(cur_codec->record_format) {
		case AFMT_S8:
		case AFMT_U8:
			data_width = 8;
			break;
		default :
		case AFMT_U16_LE:
		case AFMT_S16_LE:
			data_width = 16;
			break;
		}
		dp = cur_codec->dsp_endpoints->in_endpoint;
		burst_length = get_burst_length(i2s_dev, (int)dp->paddr|(int)dp->fragsize|
				dp->dma_config.src_maxburst|dp->dma_config.dst_maxburst);
		if (I2S_RX_FIFO_DEPTH <= burst_length/data_width)
			__i2s_set_receive_trigger(i2s_dev, ((burst_length/data_width +1) >> 1) - 1);
		else
			__i2s_set_receive_trigger(i2s_dev, 8);
	}

	return;
}

static int i2s_enable(struct i2s_device * i2s_dev, int mode)
{
	unsigned long replay_rate = DEFAULT_REPLAY_SAMPLERATE;
	unsigned long record_rate = DEFAULT_RECORD_SAMPLERATE;
	unsigned long replay_format = 16;
	unsigned long record_format = 16;
	int replay_channel = DEFAULT_REPLAY_CHANNEL;
	int record_channel = DEFAULT_RECORD_CHANNEL;
	struct dsp_pipe *dp_other = NULL;
	struct codec_info *cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
			return -ENODEV;

	if (mode & CODEC_WMODE) {
		dp_other = cur_codec->dsp_endpoints->in_endpoint;
		i2s_set_fmt(i2s_dev, &replay_format,mode);
		i2s_set_channel(i2s_dev, &replay_channel,mode);
		i2s_set_rate(i2s_dev, &replay_rate,mode);
	}
	if (mode & CODEC_RMODE) {
		dp_other = cur_codec->dsp_endpoints->out_endpoint;
		i2s_set_fmt(i2s_dev, &record_format,mode);
		i2s_set_channel(i2s_dev, &record_channel,mode);
		printk("------ come to %s \n", __func__);
		i2s_set_rate(i2s_dev, &record_rate,mode);
		i2s_set_filter(i2s_dev, mode,record_channel);
	}
	i2s_set_trigger(i2s_dev, mode);

	if (!dp_other->is_used) {
		__i2s_select_i2s(i2s_dev);
		__i2s_enable(i2s_dev);
	}

	codec_ctrl(cur_codec, CODEC_ANTI_POP,mode);

	i2s_dev->i2s_is_incall_state = false;

	return 0;
}

void i2s_replay_zero_for_flush_codec(struct i2s_device *i2s_dev)
{
	__i2s_write_tfifo(i2s_dev, 0);	//avoid pop
	__i2s_write_tfifo(i2s_dev, 0);
	__i2s_write_tfifo(i2s_dev, 0);
	__i2s_write_tfifo(i2s_dev, 0);
	__i2s_enable_replay(i2s_dev);
	msleep(2);
	__i2s_disable_replay(i2s_dev);
}


static int i2s_disable_channel(struct i2s_device *i2s_dev, int mode)
{
	struct codec_info * cur_codec = i2s_dev->cur_codec;
	if (cur_codec) {
			codec_ctrl(cur_codec, CODEC_TURN_OFF,mode);
	}

	if (mode & CODEC_WMODE) {
		i2s_replay_zero_for_flush_codec(i2s_dev);
	}
	if (mode & CODEC_RMODE) {
		__i2s_disable_record(i2s_dev);
	}
	return 0;
}


static int i2s_dma_enable(struct i2s_device * i2s_dev, int mode)		//CHECK
{
	int val;
	struct codec_info * cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
			return -ENODEV;
	if (mode & CODEC_WMODE) {
		__i2s_flush_tfifo(i2s_dev);
		codec_ctrl(cur_codec, CODEC_DAC_MUTE,0);
		__i2s_enable_transmit_dma(i2s_dev);
		__i2s_enable_replay(i2s_dev);
	}
	if (mode & CODEC_RMODE) {
		__i2s_flush_rfifo(i2s_dev);
		codec_ctrl(cur_codec, CODEC_ADC_MUTE,0);
		/* read the first sample and ignore it */
		val = __i2s_read_rfifo(i2s_dev);
		__i2s_enable_receive_dma(i2s_dev);
		__i2s_enable_record(i2s_dev);
	}

	return 0;
}

static int i2s_dma_disable(struct i2s_device *i2s_dev, int mode)		//CHECK seq dma and func
{
	struct codec_info *cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
			return -ENODEV;
	if (mode & CODEC_WMODE) {
		__i2s_disable_transmit_dma(i2s_dev);
		__i2s_disable_replay(i2s_dev);
	}
	if (mode & CODEC_RMODE) {
		__i2s_disable_record(i2s_dev);
		__i2s_disable_receive_dma(i2s_dev);
	}
	return 0;
}

static int i2s_get_fmt_cap(struct i2s_device *i2s_dev, unsigned long *fmt_cap,int mode)
{
	unsigned long i2s_fmt_cap = 0;
	struct codec_info * cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
			return -ENODEV;
	if (mode & CODEC_WMODE) {
		i2s_fmt_cap |= AFMT_S16_LE|AFMT_S16_BE|AFMT_U16_LE|AFMT_U16_BE|AFMT_S8|AFMT_U8;
		codec_ctrl(cur_codec, CODEC_GET_REPLAY_FMT_CAP, *fmt_cap);

	}
	if (mode & CODEC_RMODE) {
		i2s_fmt_cap |= AFMT_U16_LE|AFMT_S16_LE|AFMT_S8|AFMT_U8;
		codec_ctrl(cur_codec, CODEC_GET_RECORD_FMT_CAP, *fmt_cap);
	}

	if (*fmt_cap == 0)
		*fmt_cap = i2s_fmt_cap;
	else
		*fmt_cap &= i2s_fmt_cap;

	return 0;
}


static int i2s_get_fmt(struct i2s_device *i2s_dev, unsigned long *fmt, int mode)
{
	struct codec_info * cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
			return -ENODEV;

	if (mode & CODEC_WMODE)
		*fmt = cur_codec->replay_format;
	if (mode & CODEC_RMODE)
		*fmt = cur_codec->record_format;

	return 0;
}

static void i2s_dma_need_reconfig(struct i2s_device *i2s_dev, int mode)
{
	struct dsp_pipe	*dp = NULL;

	struct codec_info * cur_codec = i2s_dev->cur_codec;
	if (!cur_codec)
			return;
	if (mode & CODEC_WMODE) {
		dp = cur_codec->dsp_endpoints->out_endpoint;
		dp->need_reconfig_dma = true;
	}
	if (mode & CODEC_RMODE) {
		dp = cur_codec->dsp_endpoints->in_endpoint;
		dp->need_reconfig_dma = true;
	}
	return;
}


static int i2s_set_device(struct i2s_device * i2s_dev, unsigned long device)
{
	unsigned long tmp_rate = 44100;
  	int ret = 0;
	struct dsp_endpoints *endpoints = NULL;
	struct dsp_pipe *dp = NULL;
	struct codec_info * cur_codec = i2s_dev->cur_codec;

	endpoints = i2s_dev->i2s_endpoints;

	dp = endpoints->out_endpoint;

	if (!cur_codec)
		return -1;

	/*call state operation*/
	if (*(enum snd_device_t *)device >= SND_DEVICE_CALL_START &&
            *(enum snd_device_t *)device <= SND_DEVICE_CALL_END)
		i2s_dev->i2s_is_incall_state = true;
	else
		i2s_dev->i2s_is_incall_state = false;

	if (*(enum snd_device_t *)device == SND_DEVICE_LOOP_TEST) {
		codec_ctrl(cur_codec, CODEC_ADC_MUTE,0);
		codec_ctrl(cur_codec, CODEC_DAC_MUTE,0);
		i2s_set_rate(i2s_dev, &tmp_rate, CODEC_RWMODE);
	}

	/*hdmi operation*/
	if ((tmp_rate = cur_codec->replay_rate) == 0);
		tmp_rate = 44100;
	if ((*(enum snd_device_t *)device) == SND_DEVICE_HDMI) {
		if (strcmp(cur_codec->name,"hdmi")) {
			i2s_match_codec(i2s_dev, "hdmi");
			cur_codec->codec_clk = 0;
			__i2s_stop_bitclk(i2s_dev);
			__i2s_external_codec(i2s_dev);
			__i2s_master_clkset(i2s_dev);
			__i2s_start_bitclk(i2s_dev);
			i2s_set_rate(i2s_dev, &tmp_rate,CODEC_WMODE);
		}
	} else {
		/*restore old device*/
		if (!strcmp(cur_codec->name,"hdmi")) {
#if defined(CONFIG_JZ_INTERNAL_CODEC_V12)
			i2s_match_codec(i2s_dev, "internal_codec");
#elif defined(CONFIG_JZ_EXTERNAL_CODEC_V12)
			i2s_match_codec(i2s_dev, "external_codec");
#endif
			__i2s_stop_bitclk(i2s_dev);
#if defined(CONFIG_JZ_EXTERNAL_CODEC_V12)
			__i2s_external_codec(i2s_dev);
#endif

			if (cur_codec->codec_mode == CODEC_MASTER) {
#if defined(CONFIG_JZ_INTERNAL_CODEC_V12)
				__i2s_internal_codec_master(i2s_dev);
#endif
				printk("   come to codec master\n ");
				__i2s_slave_clkset(i2s_dev);
				__i2s_enable_sysclk_output(i2s_dev);
			} else if (cur_codec->codec_mode == CODEC_SLAVE) {
#if defined(CONFIG_JZ_INTERNAL_CODEC_V12)
				__i2s_internal_codec_slave(i2s_dev);
#endif
				printk("   come to codec slave\n ");
				__i2s_master_clkset(i2s_dev);
				__i2s_disable_sysclk_output(i2s_dev);
			}

			if (dp->is_trans == true) {
				if (dp->pddata && dp->pddata->dev_ioctl_2) {
					 if (dp->dma_config.direction == DMA_MEM_TO_DEV) {
						codec_ctrl(cur_codec, CODEC_DAC_MUTE,0);
					}
				}
			}

			clk_set_rate(i2s_dev->i2s_clk, cur_codec->codec_clk);
			if (clk_get_rate(i2s_dev->i2s_clk) > cur_codec->codec_clk) {
				printk("codec codec set rate fail.\n");
			}
			__i2s_start_bitclk(i2s_dev);
		}
	}

	/*route operatiom*/
	ret = codec_ctrl(cur_codec, CODEC_SET_DEVICE, device);

	return ret;
}

/********************************************************\
 * dev_ioctl
\********************************************************/
static long i2s_ioctl_2(struct snd_dev_data *ddata, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct i2s_device *i2s_dev = i2s_get_private_data(ddata);
	struct codec_info *cur_codec = i2s_dev->cur_codec;
	//printk("[i2s debug]%s:%d, ddata:%p, cmd:%d, arg:%x\n", __func__, __LINE__, ddata, cmd, arg? *(unsigned int *)arg : arg);
	switch(cmd) {
		case SND_DSP_GET_REPLAY_RATE:
			if (cur_codec && cur_codec->replay_rate)
				*(unsigned long *)arg = cur_codec->replay_rate;
			ret = 0;
			break;
		case SND_DSP_GET_RECORD_RATE:
			if (cur_codec && cur_codec->record_rate)
				*(unsigned long *)arg = cur_codec->record_rate;
			ret = 0;
			break;

		case SND_DSP_GET_REPLAY_CHANNELS:
			if (cur_codec && cur_codec->replay_codec_channel)
				*(unsigned long *)arg = cur_codec->replay_codec_channel;
			ret = 0;
			break;

		case SND_DSP_GET_RECORD_CHANNELS:
			if (cur_codec && cur_codec->record_codec_channel)
				*(unsigned long *)arg = cur_codec->record_codec_channel;
			ret = 0;
			break;

		case SND_DSP_GET_REPLAY_FMT_CAP:
			/* return the support replay formats */
			ret = i2s_get_fmt_cap(i2s_dev, (unsigned long *)arg,CODEC_WMODE);
			break;

		case SND_DSP_GET_REPLAY_FMT:
			/* get current replay format */
			i2s_get_fmt(i2s_dev, (unsigned long *)arg,CODEC_WMODE);
			break;
		case SND_DSP_GET_HP_DETECT:
			*(int*)arg = jz_get_hp_switch_state(i2s_dev);
			ret = 0;
			break;
		case SND_DSP_ENABLE_DMA_RX:
			ret = i2s_dma_enable(i2s_dev, CODEC_RMODE);
			break;

		case SND_DSP_DISABLE_DMA_RX:
			ret = i2s_dma_disable(i2s_dev, CODEC_RMODE);
			break;

		case SND_DSP_ENABLE_DMA_TX:
			ret = i2s_dma_enable(i2s_dev, CODEC_WMODE);
			break;

		case SND_DSP_DISABLE_DMA_TX:
			ret = i2s_dma_disable(i2s_dev, CODEC_WMODE);
			break;
		case SND_DSP_FLUSH_SYNC:
			flush_work(&i2s_dev->i2s_work);
			break;
		default:
			flush_work(&i2s_dev->i2s_work);
			i2s_dev->ioctl_cmd = cmd;
			i2s_dev->ioctl_arg = arg ? *(unsigned int *)arg : arg;
			queue_work(i2s_dev->i2s_work_queue_1, &i2s_dev->i2s_work);
			break;


	}
	return ret;
}

static long do_ioctl_work(struct i2s_device *i2s_dev, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	struct codec_info * cur_codec = i2s_dev->cur_codec;
	switch (cmd) {
	case SND_DSP_ENABLE_REPLAY:
		/* enable i2s record */
		/* set i2s default record format, channels, rate */
		/* set default replay route */
		ret = i2s_enable(i2s_dev, CODEC_WMODE);
		break;

	case SND_DSP_DISABLE_REPLAY:
		/* disable i2s replay */
		ret = i2s_disable_channel(i2s_dev, CODEC_WMODE);
		break;

	case SND_DSP_ENABLE_RECORD:
		/* enable i2s record */
		/* set i2s default record format, channels, rate */
		/* set default record route */
		ret = i2s_enable(i2s_dev, CODEC_RMODE);
		break;

	case SND_DSP_DISABLE_RECORD:
		/* disable i2s record */
		ret = i2s_disable_channel(i2s_dev, CODEC_RMODE);
		break;

	case SND_DSP_SET_REPLAY_RATE:
		ret = i2s_set_rate(i2s_dev, (unsigned long *)arg,CODEC_WMODE);
		break;

	case SND_DSP_SET_RECORD_RATE:
		ret = i2s_set_rate(i2s_dev, (unsigned long *)arg,CODEC_RMODE);
		break;

	case SND_DSP_SET_REPLAY_CHANNELS:
		/* set replay channels */
		ret = i2s_set_channel(i2s_dev, (int *)arg,CODEC_WMODE);
		if (ret < 0)
			break;
		//if (ret & NEED_RECONF_FILTER)
		//	i2s_set_filter(i2s_dev, CODEC_WMODE,cur_codec->replay_codec_channel);
		ret = 0;
		break;

	case SND_DSP_SET_RECORD_CHANNELS:
		/* set record channels */
		ret = i2s_set_channel(i2s_dev, (int*)arg,CODEC_RMODE);
		if (ret < 0)
			break;
		if (ret & NEED_RECONF_FILTER)
			i2s_set_filter(i2s_dev, CODEC_RMODE,cur_codec->record_codec_channel);
		ret = 0;
		break;

	case SND_DSP_SET_REPLAY_FMT:
		/* set replay format */
		ret = i2s_set_fmt(i2s_dev, (unsigned long *)arg,CODEC_WMODE);
		if (ret < 0)
			break;
		/* if need reconfig the trigger, reconfig it */
		if (ret & NEED_RECONF_TRIGGER)
			i2s_set_trigger(i2s_dev, CODEC_WMODE);
		/* if need reconfig the dma_slave.max_tsz, reconfig it and
		   set the dp->need_reconfig_dma as true */
		if (ret & NEED_RECONF_DMA)
			i2s_dma_need_reconfig(i2s_dev, CODEC_WMODE);
		/* if need reconfig the filter, reconfig it */
		//if (ret & NEED_RECONF_FILTER)
		//	i2s_set_filter(i2s_dev, CODEC_RMODE,cur_codec->replay_codec_channel);
		ret = 0;
		break;

	case SND_DSP_SET_RECORD_FMT:
		/* set record format */
		ret = i2s_set_fmt(i2s_dev, (unsigned long *)arg,CODEC_RMODE);
		if (ret < 0)
			break;
		/* if need reconfig the trigger, reconfig it */
		if (ret & NEED_RECONF_TRIGGER)
			i2s_set_trigger(i2s_dev, CODEC_RMODE);
		/* if need reconfig the dma_slave.max_tsz, reconfig it and
		   set the dp->need_reconfig_dma as true */
		if (ret & NEED_RECONF_DMA)
			i2s_dma_need_reconfig(i2s_dev, CODEC_RMODE);
		/* if need reconfig the filter, reconfig it */
		if (ret & NEED_RECONF_FILTER)
			i2s_set_filter(i2s_dev, CODEC_RMODE,cur_codec->record_codec_channel);
		ret = 0;
		break;

	case SND_MIXER_DUMP_REG:
		dump_i2s_reg(i2s_dev);
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_DUMP_REG,0);
		break;
	case SND_MIXER_DUMP_GPIO:
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_DUMP_GPIO,0);
		break;

	case SND_DSP_SET_STANDBY:
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_SET_STANDBY,(int)arg);
		break;

	case SND_DSP_SET_DEVICE:
		ret = i2s_set_device(i2s_dev, arg);
		break;
	case SND_DSP_SET_RECORD_VOL:
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_SET_RECORD_VOLUME, arg);
		break;
	case SND_DSP_SET_REPLAY_VOL:
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_SET_REPLAY_VOLUME, arg);
		break;
	case SND_DSP_SET_MIC_VOL:
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_SET_MIC_VOLUME, arg);
		break;
	case SND_DSP_CLR_ROUTE:
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_CLR_ROUTE,arg);
		break;
	case SND_DSP_DEBUG:
		if (cur_codec)
			ret = codec_ctrl(cur_codec, CODEC_DEBUG,arg);
		break;
	case SND_DSP_RESUME_PROCEDURE:
		if (cur_codec && !i2s_is_incall_1(i2s_dev))
			codec_ctrl(cur_codec, CODEC_RESUME,0);
		break;
	default:
		printk("SOUND_ERROR: %s(line:%d) unknown command!",
				__func__, __LINE__);
		ret = -EINVAL;
	}

	return ret;
}

static void i2s_work_handler(struct work_struct *work)
{
	struct i2s_device *i2s_dev = (struct i2s_device *)container_of(work, struct i2s_device, i2s_work);
	unsigned int cmd = i2s_dev->ioctl_cmd;
	unsigned long arg = i2s_dev->ioctl_arg;
	//printk("%s:%d, i2s_dev:%p, cmd:%d, arg:%x\n", __func__, __LINE__, i2s_dev, cmd, arg);
	do_ioctl_work(i2s_dev, cmd, (unsigned long)&arg);
}

/*##################################################################*\
|* functions
\*##################################################################*/

#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
static void i2s_codec_work_handler(struct work_struct *work)
{
	struct i2s_device *i2s_dev = (struct i2s_device *)container_of(work, struct i2s_device, i2s_codec_work);
	struct codec_info *cur_codec = i2s_dev->cur_codec;
#ifdef CONFIG_JZ_HP_DETECT_CODEC_V12
	wait_event_interruptible(switch_data.wq,switch_data.hp_work.entry.next != NULL);
#endif
	codec_ctrl(cur_codec, CODEC_IRQ_HANDLE,(unsigned long)(&(switch_data.hp_work)));
}
#endif

static irqreturn_t i2s_irq_handler(int irq, void *dev_id)
{
	irqreturn_t ret = IRQ_HANDLED;

#if 0
	struct i2s_device * i2s_dev = (struct i2s_device *)dev_id;
	/* check the irq source */

	/* if irq source is codec, call codec irq handler */
#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
	if (read_inter_codec_irq(i2s_dev)){
		codec_irq_set_mask(i2s_dev->cur_codec);
		if(!work_pending(&i2s_dev->i2s_codec_work))
			queue_work(i2s_dev->i2s_work_queue, &i2s_dev->i2s_codec_work);
	}
#endif
	/* if irq source is aic, process it here */

	/*noting to do*/
#endif

	return ret;
}

static int i2s_init_pipe_2(struct dsp_pipe *dp, enum dma_data_direction direction, unsigned long iobase)
{
	if(dp == NULL) {
		printk("%s dp is null !! \n", __func__);
		return -EINVAL;
	}
	dp->dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	dp->dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	dp->dma_type = JZDMA_REQ_I2S0;
	dp->fragsize = FRAGSIZE_M;
	dp->fragcnt = FRAGCNT_L;

	if (direction == DMA_TO_DEVICE) {
		dp->dma_config.direction = DMA_MEM_TO_DEV;
		dp->dma_config.src_maxburst = 64;
		dp->dma_config.dst_maxburst = 64;
		dp->dma_config.dst_addr = iobase + AICDR;
		dp->dma_config.src_addr = 0;
	} else if (direction == DMA_FROM_DEVICE) {
		dp->dma_config.direction = DMA_DEV_TO_MEM;
		dp->dma_config.src_maxburst = 32;
		dp->dma_config.dst_maxburst = 32;
		dp->dma_config.src_addr = iobase + AICDR;
		dp->dma_config.dst_addr = 0;
	} else
		return -1;

	return 0;
}

static int i2s_global_init(struct platform_device *pdev, struct snd_switch_data *switch_data)
{
	int ret = 0;
	struct dsp_pipe *i2s_pipe_out = NULL;
	struct dsp_pipe *i2s_pipe_in = NULL;
	struct i2s_device * i2s_dev;
	printk("i2s global init\n");

	i2s_dev = (struct i2s_device *)kzalloc(sizeof(struct i2s_device), GFP_KERNEL);
	if(!i2s_dev) {
		dev_err(&pdev->dev, "failed to alloc i2s dev\n");
		return -ENOMEM;
	}
	i2s_dev->cur_codec = (struct codec_info *)kmalloc(sizeof(struct codec_info), GFP_KERNEL);
	if(!i2s_dev->cur_codec) {
		dev_err(&pdev->dev, "failed to alloc cur codec\n");
		goto err_alloc_codec;
	}


	i2s_dev->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(i2s_dev->res == NULL) {
		printk("%s i2s resource get failed\n", __func__);
		goto err_get_resource;
	}

	if(!request_mem_region(i2s_dev->res->start, resource_size(i2s_dev->res), pdev->name)) {
		printk("%s: request mem region failed\n", __func__);
		goto err_req_mem_region;
	}

	i2s_dev->i2s_iomem = ioremap(i2s_dev->res->start, resource_size(i2s_dev->res));
	if(!i2s_dev->i2s_iomem) {
		printk("%s, ioremap failed!\n", __func__);
		goto err_ioremap;
	}

	i2s_iomem = i2s_dev->i2s_iomem;/*modify later*/


	i2s_pipe_out = kmalloc(sizeof(struct dsp_pipe), GFP_KERNEL);
	ret = i2s_init_pipe_2(i2s_pipe_out, DMA_TO_DEVICE, i2s_dev->res->start);
	if(ret < 0) {
		printk("%s: init write pipe failed!\n", __func__);
		goto err_init_pipeout;
	}

	i2s_pipe_in = kmalloc(sizeof(struct dsp_pipe), GFP_KERNEL);
	ret = i2s_init_pipe_2(i2s_pipe_in, DMA_FROM_DEVICE, i2s_dev->res->start);
	if(ret < 0) {
		printk("%s: init read pipe failed!\n", __func__);
		goto err_init_pipein;
	}

	i2s_dev->i2s_endpoints = kmalloc(sizeof(struct dsp_endpoints), GFP_KERNEL);
	if(!i2s_dev->i2s_endpoints) {
		printk("%s, unable to malloc dsp endpoints!\n", __func__);
		goto err_init_endpoints;
	}
	i2s_dev->i2s_endpoints->out_endpoint = i2s_pipe_out;
	i2s_dev->i2s_endpoints->in_endpoint = i2s_pipe_in;

	i2s_dev->i2s_clk = clk_get(&pdev->dev, "cgu_i2s");
	if(IS_ERR(i2s_dev->i2s_clk)) {
		dev_err(&pdev->dev, "cgu i2s clk get failed!\n");
		goto err_get_i2s_clk;
	}
	i2s_dev->aic_clk = clk_get(&pdev->dev, "aic");
	if(IS_ERR(i2s_dev->aic_clk)) {
		dev_err(&pdev->dev, "aic clk get failed!\n");
		goto err_get_aic_clk;
	}

	spin_lock_init(&i2s_dev->i2s_irq_lock);
	spin_lock_init(&i2s_dev->i2s_lock);

#if defined(CONFIG_JZ_INTERNAL_CODEC_V12)
	i2s_match_codec(i2s_dev, "internal_codec");
#elif defined(CONFIG_JZ_EXTERNAL_CODEC_V12)
	i2s_match_codec(i2s_dev, "i2s_external_codec");
#else
//	dev_info("WARNING: unless one codec must be select for i2s.\n");
#endif

	if(!i2s_dev->cur_codec) {
		printk("err: no codec matched!\n");
		goto err_codec_found;
	}
	i2s_dev->i2s_irq = platform_get_irq(pdev, 0);
	ret = request_irq(i2s_dev->i2s_irq, i2s_irq_handler, IRQF_DISABLED, "i2s_irq", i2s_dev);
	if(ret < 0) {
		printk("request i2s irq error!\n");
		goto err_irq;
	}

	i2s_dev->i2s_work_queue_1 = create_singlethread_workqueue("i2s_work_1");
	if(!i2s_dev->i2s_work_queue_1) {
		return -ENOMEM;
	}

	INIT_WORK(&i2s_dev->i2s_work, i2s_work_handler);
	i2s_dev->ioctl_arg = -1;
	i2s_dev->ioctl_cmd = -1;

#if defined(CONFIG_JZ_INTERNAL_CODEC_V12)
	INIT_WORK(&i2s_dev->i2s_codec_work, i2s_codec_work_handler);
	i2s_dev->i2s_work_queue = create_singlethread_workqueue("i2s_codec_irq_wq");

	if (!i2s_dev->i2s_work_queue) {
		// this can not happen, if happen, we die!
		BUG();
	}
#endif

	g_i2s_dev = i2s_dev;

	i2s_set_switch_data(switch_data, i2s_dev);


	clk_enable(i2s_dev->aic_clk);
	__i2s_disable(i2s_dev);
	schedule_timeout(5);
	__i2s_disable(i2s_dev);
	__i2s_stop_bitclk(i2s_dev);
	__i2s_stop_ibitclk(i2s_dev);
	/*select i2s trans*/
	__aic_select_i2s(i2s_dev);
	__i2s_select_i2s(i2s_dev);

#if defined(CONFIG_JZ_EXTERNAL_CODEC_V12)
	__i2s_external_codec(i2s_dev);
#endif

	if(i2s_dev->cur_codec->codec_mode == CODEC_MASTER) {
#if defined(CONFIG_JZ_INTERNAL_CODEC_V12)
		__i2s_internal_codec_master(i2s_dev);
#endif
		__i2s_slave_clkset(i2s_dev);
		/*sysclk output*/
		__i2s_enable_sysclk_output(i2s_dev);

	} else if(i2s_dev->cur_codec->codec_mode == CODEC_SLAVE) {
#if defined(CONFIG_JZ_INTERNAL_CODEC_V12)
		__i2s_internal_codec_slave(i2s_dev);
#endif
		__i2s_master_clkset(i2s_dev);
		__i2s_disable_sysclk_output(i2s_dev);
	}



	clk_set_rate(i2s_dev->i2s_clk, 8000*256);
	printk(KERN_DEBUG"%s, i2s clk rate is %ld\n", __func__, clk_get_rate(i2s_dev->i2s_clk));
	clk_enable(i2s_dev->i2s_clk);

	__i2s_start_bitclk(i2s_dev);
	__i2s_start_ibitclk(i2s_dev);

	__i2s_disable_receive_dma(i2s_dev);
	__i2s_disable_transmit_dma(i2s_dev);
	__i2s_disable_record(i2s_dev);
	__i2s_disable_replay(i2s_dev);
	__i2s_disable_loopback(i2s_dev);
	__i2s_clear_ror(i2s_dev);
	__i2s_clear_tur(i2s_dev);
	__i2s_set_receive_trigger(i2s_dev, 3);
	__i2s_set_transmit_trigger(i2s_dev, 4);
	__i2s_disable_overrun_intr(i2s_dev);
	__i2s_disable_underrun_intr(i2s_dev);
	__i2s_disable_transmit_intr(i2s_dev);
	__i2s_disable_receive_intr(i2s_dev);
	__i2s_send_rfirst(i2s_dev);

	/* play zero or last sample when underflow */
	__i2s_play_lastsample(i2s_dev);
	__i2s_enable(i2s_dev);

	printk("i2s init success.\n");
	codec_ctrl(i2s_dev->cur_codec, CODEC_INIT,0);


	return 0;


err_irq:

err_codec_found:

err_get_aic_clk:
	clk_put(i2s_dev->i2s_clk);
err_get_i2s_clk:
	kfree(i2s_dev->i2s_endpoints);
err_init_endpoints:
	kfree(i2s_pipe_in);
err_init_pipein:
	kfree(i2s_pipe_out);
err_init_pipeout:
	iounmap(i2s_dev->i2s_iomem);
err_ioremap:

err_req_mem_region:
err_get_resource:
	kfree(i2s_dev->cur_codec);
err_alloc_codec:
	kfree(i2s_dev);
	return ret;
}

static int i2s_init(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_dev_data *tmp;

	tmp = i2s_get_ddata(pdev);

	if(!g_i2s_dev) {
		init_waitqueue_head(&switch_data.wq);
		ret = i2s_global_init(pdev, &switch_data);
		ret =  platform_device_register(&xb47xx_i2s_switch);

	}
	i2s_set_private_data(tmp, g_i2s_dev);

	return ret;
}

static void i2s_shutdown(struct platform_device *pdev)
{
	/* close i2s and current codec */
	struct snd_dev_data *tmp_ddata = i2s_get_ddata(pdev);
	struct i2s_device *i2s_dev = i2s_get_private_data(tmp_ddata);
	struct codec_info *cur_codec = i2s_dev->cur_codec;

	if (cur_codec) {
		codec_ctrl(cur_codec, CODEC_TURN_OFF,CODEC_RWMODE);
		codec_ctrl(cur_codec, CODEC_SHUTDOWN,0);
	}
	__i2s_disable(i2s_dev);

	return;
}

static int i2s_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_dev_data *tmp_ddata = i2s_get_ddata(pdev);
	struct i2s_device *i2s_dev = i2s_get_private_data(tmp_ddata);
	struct codec_info *cur_codec = i2s_dev->cur_codec;

	if (cur_codec && !i2s_is_incall_1(i2s_dev))
		codec_ctrl(cur_codec, CODEC_SUSPEND,0);
	__i2s_disable(i2s_dev);
	if(clk_is_enabled(i2s_dev->aic_clk)) {
		clk_disable(i2s_dev->aic_clk);
	}
	flush_work(&i2s_dev->i2s_work);
	return 0;
}

static int i2s_resume(struct platform_device *pdev)
{
	struct snd_dev_data *tmp_ddata = i2s_get_ddata(pdev);
	struct i2s_device *i2s_dev = i2s_get_private_data(tmp_ddata);

	if(!clk_is_enabled(i2s_dev->aic_clk)) {
		clk_enable(i2s_dev->aic_clk);
	}
	__i2s_enable(i2s_dev);

	i2s_dev->ioctl_cmd = SND_DSP_RESUME_PROCEDURE;
	i2s_dev->ioctl_arg = 0;
	queue_work(i2s_dev->i2s_work_queue_1, &i2s_dev->i2s_work);
	return 0;
}
struct dsp_endpoints * i2s_get_endpoints(struct snd_dev_data *ddata)
{
	struct i2s_device *i2s_dev = i2s_get_private_data(ddata);

	return i2s_dev->i2s_endpoints;
}

/*
 * headphone detect switch function
 *
 */
static int jz_get_hp_switch_state(struct i2s_device * i2s_dev)
{
	struct codec_info * cur_codec = i2s_dev->cur_codec;
	int value = 0;
	int ret = 0;
    if (cur_codec) {
        ret = codec_ctrl(cur_codec, CODEC_GET_HP_STATE, (unsigned long)&value);
        if (ret < 0) {
            return 0;
        }
    }
	return value;
}
static int jz_get_hp_switch_state_2(struct snd_switch_data *switch_data)
{
	struct i2s_device *i2s_dev = i2s_get_switch_data(switch_data);
	struct codec_info * cur_codec = i2s_dev->cur_codec;

	int value = 0;
	int ret = 0;
    if (cur_codec) {
        ret = codec_ctrl(cur_codec, CODEC_GET_HP_STATE, (unsigned long)&value);
        if (ret < 0) {
            return 0;
        }
    }
	return value;
}

void *jz_set_hp_detect_type(int type,struct snd_board_gpio *hp_det,
		struct snd_board_gpio *mic_det,
		struct snd_board_gpio *mic_detect_en,
		struct snd_board_gpio *mic_select,
		int hook_active_level)
{
	switch_data.type = type;
	switch_data.hook_valid_level = hook_active_level;
	if (type == SND_SWITCH_TYPE_GPIO && hp_det != NULL) {
		switch_data.hp_gpio = hp_det->gpio;
		switch_data.hp_valid_level = hp_det->active_level;
	} else {
		switch_data.hp_gpio = -1;
	}
	if (mic_det != NULL) {
		switch_data.mic_gpio = mic_det->gpio;
		switch_data.mic_vaild_level = mic_det->active_level;
	} else {
		switch_data.mic_gpio = -1;
	}

	if (mic_detect_en != NULL) {
		switch_data.mic_detect_en_gpio = mic_detect_en->gpio;
		switch_data.mic_detect_en_level = mic_detect_en->active_level;
	} else {
		switch_data.mic_detect_en_gpio = -1;
	}

	if (mic_detect_en != NULL) {
		switch_data.mic_select_gpio = mic_select->gpio;
		switch_data.mic_select_level = mic_select->active_level;
	} else {
		switch_data.mic_select_gpio = -1;
	}
	return (&switch_data.hp_work);
}

static int i2s_set_device_2(struct snd_switch_data *switch_data, unsigned long device)
{
	struct i2s_device *i2s_dev = i2s_get_switch_data(switch_data);
	int ret;

	ret = i2s_set_device(i2s_dev, device);

	return ret;
}
struct snd_switch_data switch_data = {
	.sdev = {
		.name = "h2w",
	},
	.state_headset_on	=	"1",
	.state_headphone_on =   "2",
	.state_off	=	"0",
	.codec_get_sate	= NULL,
	.set_device	= NULL,
	.codec_get_state_2 = jz_get_hp_switch_state_2,
	.set_device_2 = i2s_set_device_2,
#if defined(CONFIG_JZ_HP_DETECT_CODEC_V12)
	.type	=	SND_SWITCH_TYPE_CODEC,
#elif defined(CONFIG_JZ_HP_DETECT_GPIO_V12)
	.type	=	SND_SWITCH_TYPE_GPIO,
#endif
};

static struct platform_device xb47xx_i2s_switch = {
#if 1
	.name	= DEV_DSP_HP_DET_NAME,
	.id		= SND_DEV_DETECT0_ID,
	.dev	= {
		.platform_data	= &switch_data,
	},
#endif
};

struct snd_dev_data i2s_data = {
	.dev_ioctl_2	= i2s_ioctl_2,
	.get_endpoints	= i2s_get_endpoints,
	.minor			= SND_DEV_DSP0,
	.init			= i2s_init,
	.shutdown		= i2s_shutdown,
	.suspend		= i2s_suspend,
	.resume			= i2s_resume,
};

struct snd_dev_data snd_mixer0_data = {
	.dev_ioctl_2	= i2s_ioctl_2,
	.minor		= SND_DEV_MIXER0,
	.init		= i2s_init,
};

static int __init init_i2s(void)
{
	return 0;
}
module_init(init_i2s);
