/*
 * Linux/sound/oss/xb47XX/xb4780/jz4780_codec.c
 *
 * CODEC CODEC driver for Ingenic Jz4780 MIPS processor
 *
 * 2010-11-xx   jbbi <jbbi@ingenic.cn>
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/sound.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <linux/proc_fs.h>
#include <linux/soundcard.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <mach/jzsnd.h>
#include "../xb47xx_i2s.h"

#ifdef CONFIG_SOUND_I2S_JZ47XX
extern int i2s_register_codec(char*, void *,unsigned long,enum codec_mode);
#endif
#ifdef CONFIG_SOUND_SPDIF_JZ47XX
extern int spdif_register_codec(char*, void *,unsigned long,enum codec_mode);
#endif

static void codec_get_format_cap(unsigned long *format)
{
	*format = AFMT_S16_LE|AFMT_S8;
}

static int jzcodec_ctl(unsigned int cmd, unsigned long arg)
{
		switch (cmd) {

		case CODEC_GET_REPLAY_FMT_CAP:
		case CODEC_GET_RECORD_FMT_CAP:
			codec_get_format_cap((unsigned long *)arg);
		default:
			break;
		}
	return 0;
}
/**
 * Module init
 */
#define HDMI_SMAPLE_RATE	11333335//11289600
static int __init init_codec(void)
{
	int ret = 0;
#ifdef CONFIG_SOUND_I2S_JZ47XX
	ret = i2s_register_codec("hdmi", (void *)jzcodec_ctl,HDMI_SMAPLE_RATE,CODEC_SLAVE);
	if (ret < 0)
		printk("hdmi audio is not support\n");
	printk("hdmi audio codec register success\n");
#endif
#ifdef CONFIG_SOUND_SPDIF_JZ47XX
	ret = spdif_register_codec("hdmi", (void *)jzcodec_ctl,HDMI_SMAPLE_RATE,CODEC_SLAVE);
	if (ret < 0)
		printk("hdmi audio is not support\n");
	printk("hdmi audio codec register success\n");
#endif

	return 0;
}

/**
 * Module exit
 */
static void __exit cleanup_codec(void)
{
}

arch_initcall_sync(init_codec);
module_exit(cleanup_codec);
MODULE_LICENSE("GPL");
