#include "mic.h"

static void mic_dma_callback(void *data) {
    printk("%s\n", __func__);
}

int mic_dma_submit_cyclic(struct mic_dev *mic_dev, struct mic *mic) {
    unsigned long flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    enum dma_data_direction direction = DMA_DEV_TO_MEM;

    mic->desc = mic->dma_chan->device->device_prep_dma_cyclic(mic->dma_chan,
            virt_to_phys(mic->buf[0]),
            mic->buf_len * mic->buf_cnt,
            mic->buf_len,
            direction,
            flags,
            NULL);
    if (!mic->desc) {
        dev_err(mic_dev->dev, "failed to prepare dma desc\n");
        return -1;
    }

    mic->desc->callback = mic_dma_callback;
    mic->desc->callback_param = mic;

    dmaengine_submit(mic->desc);
    dma_async_issue_pending(mic->dma_chan);

    return 0;
}

void mic_dma_terminate(struct mic *mic) {
    dmaengine_terminate_all(mic->dma_chan);
}

static bool mic_filter(struct dma_chan *chan, void *filter_param)
{
    struct mic *mic = (struct mic *)filter_param;

    return mic->dma_type == (int)chan->private;
}

int mic_prepare_dma(struct mic_dev *mic_dev, struct mic *mic) {
    dma_cap_mask_t mask;
    dma_cap_zero(mask);
    dma_cap_set(DMA_SLAVE, mask);
    dma_cap_set(DMA_CYCLIC, mask);

    mic->dma_chan = dma_request_channel(mask, mic_filter, mic);
    if (!mic->dma_chan) {
        dev_err(mic_dev->dev, "Failed to request dma chan\n");
        return -1;
    }

    return 0;
}
