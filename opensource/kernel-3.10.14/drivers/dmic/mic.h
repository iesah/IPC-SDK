#ifndef __MIC_H__
#define __MIC_H__
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/hrtimer.h>
#include <linux/dmaengine.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/circ_buf.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/memory.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <mach/jzdma.h>
#include "common.h"

enum mic_type {
    DMIC,
};

struct mic_dev;

struct mic {
    int type;
    struct resource *dma_res;
    struct resource *io_res;

    struct mic_dev *mic_dev;
    struct dma_chan *dma_chan;
    struct dma_slave_config dma_config;
    struct dma_async_tx_descriptor *desc;
    int dma_type;

    resource_size_t dma_res_start;
    resource_size_t io_res_start;
    int io_res_size;
    void __iomem    *iomem;

    int offset;
    unsigned long long cnt;

    char **buf;
    int buf_len;
    int buf_cnt;
    int period_len;
    struct scatterlist sg[2];

    int gain;
};

struct raw_data {
    short dmic_channel[4];
};

struct gain_range {
    int min_dB;
    int max_dB;
};

struct mic_dev {
    struct device *dev;
    struct platform_device *pdev;

    int major;
    int minor;
    int nr_devs;
    struct class *class;
    struct cdev cdev;

    struct list_head filp_data_list;
    spinlock_t list_lock;
    int is_enabled;
    int is_stoped;

    int periods_ms;
    int raws_pre_sec;
    struct mutex buf_lock;
    struct raw_data *data_buf;
    wait_queue_head_t wait_queue;

    struct hrtimer hrtimer;

    struct mic dmic;
};

struct mic_file_data {
    struct list_head entry;
    unsigned long long cnt;

    int dmic_offset;
    int periods_ms;
    int is_enabled;
};

#define MIC_DEFAULT_PERIOD_MS       (250)       //250 ms
#define MIC_BUFFER_TOTAL_LEN        (32 * 1024)

#define MIC_SET_PERIODS_MS          0x100
#define MIC_ENABLE_RECORD           0x101
#define MIC_DISABLE_RECORD          0x102
#define MIC_GET_PERIODS_MS          0x103
#define MIC_SET_DMIC_GAIN           0x200
#define MIC_GET_DMIC_GAIN_RANGE     0x201
#define MIC_GET_DMIC_GAIN           0x202

void dmic_enable(struct mic_dev *mic_dev);
void dmic_disable(struct mic_dev *mic_dev);
void dmic_set_gain(struct mic_dev *mic_dev, int gain);
void dmic_get_gain_range(struct gain_range *range);

void mic_dma_terminate(struct mic *mic);
int mic_dma_submit(struct mic_dev *mic_dev, struct mic *mic);
int mic_dma_submit_cyclic(struct mic_dev *mic_dev, struct mic *mic);
int mic_prepare_dma(struct mic_dev *mic_dev, struct mic *mic);

void mic_hrtimer_start(struct mic_dev *mic_dev);
void mic_hrtimer_stop(struct mic_dev *mic_dev);
void mic_hrtimer_init(struct mic_dev *mic_dev);

int mic_sys_init(struct mic_dev *mic_dev);
int mic_set_periods_ms(struct mic_file_data *fdata, struct mic_dev *mic_dev, int periods_ms);
int mic_enable_record(struct mic_file_data *fdata, struct mic_dev *mic_dev);
int mic_disable_record(struct mic_file_data *fdata, struct mic_dev *mic_dev);

#endif
