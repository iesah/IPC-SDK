
#include "dmic_hal.h"

static void dmic_set_channel(struct mic_dev *mic_dev, unsigned int channels) {
    if (channels > 4) {
        printk("error channle num: %d\n", channels);
    }

    __dmic_set_chnum(mic_dev, channels - 1);
}

static int dmic_set_samplerate(struct mic_dev *mic_dev, enum dmic_rate rate) {
    int ret = 0;

    switch (rate) {
    case DMIC_RATE_8000:
        __dmic_set_sr_8k(mic_dev);
        break;
    case DMIC_RATE_16000:
        __dmic_set_sr_16k(mic_dev);
        break;
    case DMIC_RATE_48000:
        __dmic_set_sr_48k(mic_dev);
        break;
    default:
        ret = -1;
        printk("error samplerate : %d\n", rate);
    }

    return ret;
}

void dmic_init(struct mic_dev *mic_dev) {
    __dmic_reset(mic_dev);
    __dmic_reset_tri(mic_dev);

    dmic_disable(mic_dev);
    dmic_set_samplerate(mic_dev, DMIC_RATE_16000);

    dmic_set_channel(mic_dev, 4);

    __dmic_unpack_msb(mic_dev);
    __dmic_unpack_dis(mic_dev);
    __dmic_enable_pack(mic_dev);

    /**
     * for dma request
     */
    __dmic_enable_rdms(mic_dev);
    __dmic_set_request(mic_dev, 32);


    __dmic_set_thr_high(mic_dev,32);
    __dmic_set_thr_low(mic_dev,32);

    __dmic_enable_hpf1(mic_dev);
    __dmic_enable_hpf2(mic_dev);

//    __dmic_enable_lp(mic_dev);
//    __dmic_prefetch_8k(mic_dev);
    __dmic_set_gcr(mic_dev,10);
    __dmic_mask_all_int(mic_dev);

//    dmic_set_reg(mic_dev,DMICIMR, 0x3b, 0x3f, 0);
    __dmic_disable_sw_lr(mic_dev);

    dmic_enable(mic_dev);
}

void dmic_enable(struct mic_dev *mic_dev) {
    __dmic_enable(mic_dev);
}

void dmic_disable(struct mic_dev *mic_dev) {
    __dmic_disable(mic_dev);
}

int dmic_is_enable(struct mic_dev *mic_dev) {
    return __dmic_is_enable(mic_dev);
}

void dmic_set_gain(struct mic_dev *mic_dev, int gain) {
    if (gain < 0)
        gain = 0;
    if (gain > 15 * 3)
        gain = 15 * 3;

    mic_dev->dmic.gain = gain;

    gain /= 3;

    printk("entry: %s, set: %d\n", __func__, gain);
    __dmic_set_gcr(mic_dev, gain);
}

void dmic_get_gain_range(struct gain_range *range) {
    range->min_dB = 0;
    range->max_dB = 15 * 3;
}

int dmic_store_data_from_32bit_fifo_to_memory(struct mic_dev *mic_dev, char * buffer, int size)
{
    int read_bytes;
    int fifo_cnt;
    int iii;
    /* unsigned int fifo_control; */
    unsigned int * data_in_word; /* 16bit data */


    fifo_cnt = __dmic_read_fifo_lvl(mic_dev);

    fifo_cnt<<=1;       /* 64bit fifo to 32bit data */

    read_bytes = fifo_cnt<<2; /* fifo 32bit sample data */
    if (read_bytes > size) {
        read_bytes = size;
        fifo_cnt = read_bytes>>2;
        read_bytes = fifo_cnt<<2;
    }

    data_in_word = (unsigned int *)buffer;

    /* one channel, 16bit */
    for (iii=0; iii<fifo_cnt; iii++) {
        unsigned int data;
        /* bytes order, endian? */
        data = __dmic_read_dr(mic_dev);

        *data_in_word = data;
        data_in_word++;
        printk("0x%x\n", data);
    }

    return read_bytes;
}

