#include "mic.h"

static enum hrtimer_restart mic_hrtimer_callback(struct hrtimer *hrtimer)
{
    unsigned long long dmic_cnt;
    int did;
    int did_tmp;
    struct mic_dev *mic_dev = container_of(hrtimer, struct mic_dev, hrtimer);
    struct mic *dmic = &mic_dev->dmic;
    enum dma_data_direction direction = DMA_DEV_TO_MEM;
    dma_addr_t  dmic_dst;

    if (!mic_dev->is_enabled)
        return HRTIMER_NORESTART;

    dmic->dma_chan->device->get_current_trans_addr(
            dmic->dma_chan, &dmic_dst, NULL, direction);


    dmic_cnt = dmic->cnt;
    did = do_div(dmic_cnt, dmic->buf_cnt);      //id = mic_cnt % buf_cnt
    did_tmp = (dmic_dst - virt_to_phys(dmic->buf[0])) / dmic->buf_len;

    if (did == did_tmp) {

        hrtimer_forward(hrtimer, hrtimer->_softexpires,
                ns_to_ktime(mic_dev->periods_ms * NSEC_PER_MSEC / 2));

        return HRTIMER_RESTART;
    }

    hrtimer_forward(hrtimer, hrtimer->_softexpires,
            ns_to_ktime(mic_dev->periods_ms * NSEC_PER_MSEC));

    dmic->cnt++;

    wake_up_all(&mic_dev->wait_queue);

    return HRTIMER_RESTART;
}

void mic_hrtimer_start(struct mic_dev *mic_dev) {
    hrtimer_forward_now(&mic_dev->hrtimer,
            ns_to_ktime(mic_dev->periods_ms * NSEC_PER_MSEC));
    hrtimer_start_expires(&mic_dev->hrtimer, HRTIMER_MODE_ABS);
}

void mic_hrtimer_stop(struct mic_dev *mic_dev) {
    hrtimer_cancel(&mic_dev->hrtimer);
}

void mic_hrtimer_init(struct mic_dev *mic_dev) {
    hrtimer_init(&mic_dev->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    mic_dev->hrtimer.function = mic_hrtimer_callback;
}
