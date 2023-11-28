
#include <linux/interrupt.h>
#include "mic.h"

#include "dmic_hal.h"

int mic_resource_init(
        struct platform_device *pdev,
        struct mic_dev *mic_dev, int type) {
    int ret;
    struct mic *mic = &mic_dev->dmic;

    mic->type = type;
    mic->mic_dev = mic_dev;

    mic->dma_res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
    if (!mic->dma_res) {
        dev_err(&pdev->dev, "failed to get platform dma resource\n");
        return -1;
    }

    mic->io_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!mic->io_res) {
        dev_err(&pdev->dev, "failed to get platform IO resource\n");
        return -1;
    }

    ret = (int)devm_request_mem_region(&pdev->dev,
            mic->io_res->start, resource_size(mic->io_res), pdev->name);
    if (!ret) {
        dev_err(&pdev->dev, "failed to request mem region");
        return -EBUSY;
    }

    mic->dma_type = GET_MAP_TYPE(mic->dma_res->start);

    mic->iomem = devm_ioremap_nocache(&pdev->dev,
            mic->io_res->start, resource_size(mic->io_res));
    if (!mic->iomem) {
        dev_err(&pdev->dev, "Failed to ioremap mmio memory");
        return -ENOMEM;
    }

    return 0;
}

extern int dmic_dma_config(struct mic_dev *mic_dev);
static int mic_pdata_init(struct mic_dev *mic_dev, struct platform_device *pdev)
{
    int ret;

    ret = mic_resource_init(pdev, mic_dev, DMIC);
    if (ret < 0)
        return ret;

    ret = mic_prepare_dma(mic_dev, &mic_dev->dmic);
    if (ret < 0)
        return ret;


    ret = dmic_dma_config(mic_dev);
    if (ret < 0)
        return ret;


    return 0;
}

static int mic_probe(struct platform_device *pdev)
{
    int ret;
    struct clk *dmic_clk;
    struct mic_dev *mic_dev;


    mic_dev = kmalloc(sizeof(struct mic_dev), GFP_KERNEL);
    if(!mic_dev) {
        printk("voice dev alloc failed\n");
        goto __err_mic;
    }
    memset(mic_dev, 0, sizeof(struct mic_dev));

    mic_dev->dev = &pdev->dev;
    dev_set_drvdata(mic_dev->dev, mic_dev);
    mic_dev->pdev = pdev;

    mutex_init(&mic_dev->buf_lock);
    init_waitqueue_head(&mic_dev->wait_queue);

    dmic_clk = clk_get(mic_dev->dev,"dmic");
    if (IS_ERR(dmic_clk)) {
        dev_err(mic_dev->dev, "Failed to get clk dmic\n");
        goto __err_mic;
    }
    clk_enable(dmic_clk);

    ret = mic_pdata_init(mic_dev, pdev);
    if (ret < 0)
        return ret;

    mic_hrtimer_init(mic_dev);

    mic_sys_init(mic_dev);

    mic_set_periods_ms(NULL, mic_dev, MIC_DEFAULT_PERIOD_MS);

    return 0;

__err_mic:
    return -EFAULT;
}

static const  struct platform_device_id mic_id_table[] = {
        { .name = "dmic", },
        {},
};

static struct platform_driver mic_platform_driver = {
        .probe = mic_probe,
        .driver = {
            .name = "dmic",
            .owner = THIS_MODULE,
        },
        .id_table = mic_id_table,
};

static int __init mic_init(void) {
    return platform_driver_register(&mic_platform_driver);
}

static void __exit mic_exit(void)
{
}

module_init(mic_init);
module_exit(mic_exit);
