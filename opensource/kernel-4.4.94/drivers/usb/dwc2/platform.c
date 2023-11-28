/*
 * platform.c - DesignWare HS OTG Controller platform driver
 *
 * Copyright (C) Matthijs Kooijman <matthijs@stdin.nl>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the above-listed copyright holders may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/phy/phy.h>
#include <linux/platform_data/s3c-hsotg.h>

#include <linux/usb/of.h>

#include "core.h"
#include "hcd.h"
#include "debug.h"

static const char dwc2_driver_name[] = "dwc2";

static const struct dwc2_core_params params_jz4780 = {
	.otg_cap			= 2,
	.otg_ver			= 1,
	.dma_enable			=  1,	/* DMA Enabled */
#ifdef CONFIG_USB_DWC2_DMA_BUFFER_MODE
	.dma_desc_enable		=  0,
#endif
#ifdef CONFIG_USB_DWC2_DMA_DESCRIPTOR_MODE
	.dma_desc_enable		=  1,
#endif

	.speed				= -1,
	.enable_dynamic_fifo		= -1,
	.en_multiple_tx_fifo		= -1,
	.host_rx_fifo_size		=  1024, /* 1024 DWORDs */
	.host_nperio_tx_fifo_size	=  1024, /* 1024 DWORDs */
	.host_perio_tx_fifo_size	=  1024, /* 1024 DWORDs */
	.max_transfer_size		= -1,
	.max_packet_count		= -1,
	.host_channels			= -1,
	.phy_type			= -1,
	.phy_utmi_width			= 16,
	.phy_ulpi_ddr			= -1,
	.phy_ulpi_ext_vbus		= -1,
	.i2c_enable			= -1,
	.ulpi_fs_ls			= -1,
	.host_support_fs_ls_low_power	= -1,
	.host_ls_low_power_phy_clk	= -1,
	.ts_dline			= -1,
	.reload_ctl			= -1,
	.ahbcfg				= -1,
	.uframe_sched			= -1,
	.external_id_pin_ctl		= -1,
	.hibernation			= 0,
};

static int __dwc2_lowlevel_hw_enable(struct dwc2_hsotg *hsotg)
{
	int ret;

	if (hsotg->clk) {
		ret = clk_prepare_enable(hsotg->clk);
		if (ret)
			return ret;
		printk("OTG CLK %x\n", *(volatile unsigned int *)(0xb0000020));
	}

	if (hsotg->uphy)
		ret = usb_phy_init(hsotg->uphy);
	printk("CPCCR CLK %x\n", *(volatile unsigned int *)(0xb0000024));
	return ret;
}

/**
 * dwc2_lowlevel_hw_enable - enable platform lowlevel hw resources
 * @hsotg: The driver state
 *
 * A wrapper for platform code responsible for controlling
 * low-level USB platform resources (phy, clock, regulators)
 */
int dwc2_lowlevel_hw_enable(struct dwc2_hsotg *hsotg)
{
	int ret = __dwc2_lowlevel_hw_enable(hsotg);

	if (ret == 0)
		hsotg->ll_hw_enabled = true;
	return ret;
}

static int __dwc2_lowlevel_hw_disable(struct dwc2_hsotg *hsotg)
{
	int ret = 0;

	if (hsotg->uphy)
		usb_phy_shutdown(hsotg->uphy);

	if (hsotg->clk)
		clk_disable_unprepare(hsotg->clk);

	return ret;
}

/**
 * dwc2_lowlevel_hw_disable - disable platform lowlevel hw resources
 * @hsotg: The driver state
 *
 * A wrapper for platform code responsible for controlling
 * low-level USB platform resources (phy, clock, regulators)
 */
int dwc2_lowlevel_hw_disable(struct dwc2_hsotg *hsotg)
{
	int ret = __dwc2_lowlevel_hw_disable(hsotg);

	if (ret == 0)
		hsotg->ll_hw_enabled = false;
	return ret;
}

static int dwc2_lowlevel_hw_init(struct dwc2_hsotg *hsotg)
{
	int ret;

	/* Set default UTMI width */
	hsotg->phyif = GUSBCFG_PHYIF16;

	hsotg->uphy = devm_usb_get_phy_by_phandle(hsotg->dev, "ingenic,usbphy", 0);
	if (IS_ERR(hsotg->uphy)) {
		ret = PTR_ERR(hsotg->uphy);
		switch (ret) {
		case -ENODEV:
		case -ENXIO:
			hsotg->uphy = NULL;
			break;
		case -EPROBE_DEFER:
			return ret;
		default:
			dev_err(hsotg->dev, "error getting usb phy %d\n",
					ret);
			return ret;
		}
	}

	hsotg->plat = dev_get_platdata(hsotg->dev);

	hsotg->clk = devm_clk_get(hsotg->dev, "gate_otg");
	if (IS_ERR(hsotg->clk)) {
		hsotg->clk = NULL;
		dev_dbg(hsotg->dev, "cannot get otg clock\n");
	}
	return 0;
}

/**
 * dwc2_driver_remove() - Called when the DWC_otg core is unregistered with the
 * DWC_otg driver
 *
 * @dev: Platform device
 *
 * This routine is called, for example, when the rmmod command is executed. The
 * device may or may not be electrically present. If it is present, the driver
 * stops device processing. Any resources used on behalf of this device are
 * freed.
 */
static int dwc2_driver_remove(struct platform_device *dev)
{
	struct dwc2_hsotg *hsotg = platform_get_drvdata(dev);

	dwc2_debugfs_exit(hsotg);
	if (hsotg->hcd_enabled)
		dwc2_hcd_remove(hsotg);
	if (hsotg->gadget_enabled)
		dwc2_hsotg_remove(hsotg);
	if (hsotg->ll_hw_enabled)
		dwc2_lowlevel_hw_disable(hsotg);

	return 0;
}

static const struct of_device_id dwc2_of_match_table[] = {
	{ .compatible = "ingenic,dwc2-hsotg", .data = (void *)&params_jz4780},
	{},
};
MODULE_DEVICE_TABLE(of, dwc2_of_match_table);

/**
 * dwc2_driver_probe() - Called when the DWC_otg core is bound to the DWC_otg
 * driver
 *
 * @dev: Platform device
 *
 * This routine creates the driver components required to control the device
 * (core, HCD, and PCD) and initializes the device. The driver components are
 * stored in a dwc2_hsotg structure. A reference to the dwc2_hsotg is saved
 * in the device private data. This allows the driver to access the dwc2_hsotg
 * structure on subsequent calls to driver methods for this device.
 */
static int dwc2_driver_probe(struct platform_device *dev)
{
	const struct of_device_id *match;
	const struct dwc2_core_params *params;
	struct dwc2_core_params defparams;
	struct dwc2_hsotg *hsotg;
	struct resource *res;
	int retval;
	int irq;

	match = of_match_device(dwc2_of_match_table, &dev->dev);
	if (match && match->data) {
		params = match->data;
	} else {
		/* Default all params to autodetect */
		dwc2_set_all_params(&defparams, -1);
		params = &defparams;

		/*
		 * Disable descriptor dma mode by default as the HW can support
		 * it, but does not support it for SPLIT transactions.
		 */
		defparams.dma_desc_enable = 0;
	}

	hsotg = devm_kzalloc(&dev->dev, sizeof(*hsotg), GFP_KERNEL);
	if (!hsotg)
		return -ENOMEM;

	hsotg->dev = &dev->dev;

	/*
	 * Use reasonable defaults so platforms don't have to provide these.
	 */
	if (!dev->dev.dma_mask)
		dev->dev.dma_mask = &dev->dev.coherent_dma_mask;
	retval = dma_set_coherent_mask(&dev->dev, DMA_BIT_MASK(32));
	if (retval)
		return retval;

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	hsotg->regs = devm_ioremap_resource(&dev->dev, res);
	if (IS_ERR(hsotg->regs))
		return PTR_ERR(hsotg->regs);

	dev_dbg(&dev->dev, "mapped PA %08lx to VA %p\n",
		(unsigned long)res->start, hsotg->regs);

	hsotg->dr_mode = usb_get_dr_mode(&dev->dev);
	if (IS_ENABLED(CONFIG_USB_DWC2_HOST) &&
			hsotg->dr_mode != USB_DR_MODE_HOST) {
		hsotg->dr_mode = USB_DR_MODE_HOST;
		dev_warn(hsotg->dev,
			"Configuration mismatch. Forcing host mode\n");
	} else if (IS_ENABLED(CONFIG_USB_DWC2_PERIPHERAL) &&
			hsotg->dr_mode != USB_DR_MODE_PERIPHERAL) {
		hsotg->dr_mode = USB_DR_MODE_PERIPHERAL;
		dev_warn(hsotg->dev,
			"Configuration mismatch. Forcing peripheral mode\n");
	}

	retval = dwc2_lowlevel_hw_init(hsotg);
	if (retval)
		return retval;

	spin_lock_init(&hsotg->lock);

	hsotg->core_params = devm_kzalloc(&dev->dev,
				sizeof(*hsotg->core_params), GFP_KERNEL);
	if (!hsotg->core_params)
		return -ENOMEM;

	dwc2_set_all_params(hsotg->core_params, -1);

	irq = platform_get_irq(dev, 0);
	if (irq < 0) {
		dev_err(&dev->dev, "missing IRQ resource\n");
		return irq;
	}

	dev_dbg(hsotg->dev, "registering common handler for irq%d\n",
		irq);
	retval = devm_request_irq(hsotg->dev, irq,
				  dwc2_handle_common_intr, IRQF_SHARED,
				  dev_name(hsotg->dev), hsotg);
	if (retval)
		return retval;

	retval = dwc2_lowlevel_hw_enable(hsotg);
	if (retval)
		return retval;

	/* Detect config values from hardware */
	retval = dwc2_get_hwparams(hsotg);
	if (retval)
		goto error;

	/* Validate parameter values */
	dwc2_set_parameters(hsotg, params);

	if (hsotg->dr_mode != USB_DR_MODE_HOST) {
		retval = dwc2_gadget_init(hsotg, irq);
		if (retval)
			goto error;
		hsotg->gadget_enabled = 1;
	}

	if (hsotg->dr_mode != USB_DR_MODE_PERIPHERAL) {
		retval = dwc2_hcd_init(hsotg, irq);
		if (retval) {
			if (hsotg->gadget_enabled)
				dwc2_hsotg_remove(hsotg);
			goto error;
		}
		hsotg->hcd_enabled = 1;
	}


	platform_set_drvdata(dev, hsotg);

	dwc2_debugfs_init(hsotg);

	/* Gadget code manages lowlevel hw on its own */
	if (hsotg->dr_mode == USB_DR_MODE_PERIPHERAL)
		dwc2_lowlevel_hw_disable(hsotg);

	return 0;

error:
	dwc2_lowlevel_hw_disable(hsotg);
	return retval;
}

static int __maybe_unused dwc2_suspend(struct device *dev)
{
	struct dwc2_hsotg *dwc2 = dev_get_drvdata(dev);
	int ret = 0;

	if (dwc2_is_device_mode(dwc2))
		dwc2_hsotg_suspend(dwc2);

	if (dwc2->ll_hw_enabled)
		ret = __dwc2_lowlevel_hw_disable(dwc2);

	return ret;
}

static int __maybe_unused dwc2_resume(struct device *dev)
{
	struct dwc2_hsotg *dwc2 = dev_get_drvdata(dev);
	int ret = 0;

	if (dwc2->ll_hw_enabled) {
		ret = __dwc2_lowlevel_hw_enable(dwc2);
		if (ret)
			return ret;
	}

	if (dwc2_is_device_mode(dwc2))
		ret = dwc2_hsotg_resume(dwc2);

	return ret;
}

static const struct dev_pm_ops dwc2_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dwc2_suspend, dwc2_resume)
};

static struct platform_driver dwc2_platform_driver = {
	.driver = {
		.name = dwc2_driver_name,
		.of_match_table = dwc2_of_match_table,
		.pm = &dwc2_dev_pm_ops,
	},
	.probe = dwc2_driver_probe,
	.remove = dwc2_driver_remove,
};

module_platform_driver(dwc2_platform_driver);

MODULE_DESCRIPTION("DESIGNWARE HS OTG Platform Glue");
MODULE_AUTHOR("Matthijs Kooijman <matthijs@stdin.nl>");
MODULE_LICENSE("Dual BSD/GPL");
