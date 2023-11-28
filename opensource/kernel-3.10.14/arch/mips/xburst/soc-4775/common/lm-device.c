#include <linux/device.h>
#include <linux/resource.h>
#include <mach/lm.h>
#include <soc/irq.h>
#include <soc/base.h>

static u64 dwc_otg_dmamask = ~(u32)0;
static struct lm_device dwc_otg_dev = {
	.dev = {
		.dma_mask               = &dwc_otg_dmamask,
                .coherent_dma_mask      = 0xffffffff,
	},
	.resource = {
		.start			= OTG_IOBASE,
		.end			= OTG_IOBASE + 0x40000 - 1,
		.flags			= IORESOURCE_MEM,
	},

	.irq	= IRQ_OTG, 		
	.id	= 0,
};

static int __init  lm_dev_init(void)
{
	int ret;	

	ret = lm_device_register(&dwc_otg_dev);
	if (ret) {
		printk("register dwc otg device failed.\n");
		return ret;
	}

	pr_debug("dwc otg device init suscessed.\n");

	return ret;
		
}

arch_initcall(lm_dev_init);
