#include "dmic_hal.h"
#include "mic.h"

int dmic_dma_config(struct mic_dev *mic_dev) {
    int ret = 0, buswidth;

    struct mic *dmic = &mic_dev->dmic;

    struct dma_slave_config slave_config;
    slave_config.direction = DMA_FROM_DEVICE;
    slave_config.src_addr = dmic->io_res->start + DMICDR;

    buswidth = DMA_SLAVE_BUSWIDTH_4_BYTES;

    slave_config.dst_addr_width = buswidth;
    slave_config.dst_maxburst = 128;
    slave_config.src_addr_width = buswidth;
    slave_config.src_maxburst = slave_config.dst_maxburst;
    ret = dmaengine_slave_config(dmic->dma_chan, &slave_config);
    if (ret) {
        dev_err(mic_dev->dev, "Failed to config dma chan\n");
        return -1;
    }

    dmic_init(mic_dev);

    return 0;
}
