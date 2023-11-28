/*
 * OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 * (C) Copyright 2002 Hewlett-Packard Company
 *
 * Bus Glue for Ingenic Jz47xx.
 *
 * Written by Christopher Hoover <ch@hpl.hp.com>
 * Based on fragments of previous driver by Rusell King et al.
 *
 * Modified for LH7A404 from ohci-sa1111.c
 *  by Durgesh Pattamatta <pattamattad@sharpsec.com>
 * Modified for AMD Alchemy Au1xxx
 *  by Matt Porter <mporter@kernel.crashing.org>
 * Modified for Jz47xx from ohci-au1xxx.c
 *  by Peter <jlwei@ingenic.cn>
 *
 * This file is licenced under the GPL.
 */

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <soc/cpm.h>

extern int usb_disabled(void);

struct jz_ohci {
	struct device		*dev;
	struct clk		*clk;
	struct clk		*clk_cgu;
};

#define hcd_to_jz(hcd)	((struct jz_ohci *)(hcd_to_ohci(hcd) + 1))

/*-------------------------------------------------------------------------*/

static void jz_start_ohc(struct jz_ohci *jz_ohci)
{
	clk_enable(jz_ohci->clk);
	clk_set_rate(jz_ohci->clk_cgu,48000000);
	clk_enable(jz_ohci->clk_cgu);
	cpm_start_ohci();
}

static void jz_stop_ohc(struct jz_ohci *jz_ohci)
{
	dev_dbg(jz_ohci->dev, "Stopping JZ OHCI USB Controller\n");

	/* disable host controller */
	cpm_stop_ohci();
	clk_disable(jz_ohci->clk);
}

/*-------------------------------------------------------------------------*/

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */


/**
 * usb_ohci_jz_probe - initialize Jz-based HCDs
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */
static int usb_ohci_jz_probe(const struct hc_driver *driver,
			  struct platform_device *pdev)
{
	int retval;
	int irq;
	struct usb_hcd *hcd;
	struct resource	*regs;
	struct jz_ohci *jz_ohci;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "No irq resource\n");
		return irq;
	}

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs) {
		dev_err(&pdev->dev, "No iomem resource\n");
		return -ENXIO;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, "jz");
	if (!hcd)
		return -ENOMEM;


	hcd->rsrc_start = regs->start;
	hcd->rsrc_len = resource_size(regs);

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		dev_err(&pdev->dev, "request_mem_region failed\n");
		retval = -EBUSY;
		goto err1;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_err(&pdev->dev, "ioremap failed\n");
		retval = -ENOMEM;
		goto err2;
	}

	jz_ohci = hcd_to_jz(hcd);

	jz_ohci->clk= clk_get(&pdev->dev, "uhc");
	if (IS_ERR(jz_ohci->clk)) {
		dev_err(&pdev->dev, "clk gate get error\n");
		retval = PTR_ERR(jz_ohci->clk);
		goto err2;
	}

	jz_ohci->clk_cgu = clk_get(&pdev->dev, "cgu_uhc");
	if (IS_ERR(jz_ohci->clk_cgu)) {
		dev_err(&pdev->dev, "clk cgu get error\n");
		retval = PTR_ERR(jz_ohci->clk_cgu);
		goto err2;
	}

	jz_ohci->dev = &pdev->dev;

	jz_start_ohc(jz_ohci);
	ohci_hcd_init(hcd_to_ohci(hcd));

	retval = usb_add_hcd(hcd, irq, IRQF_DISABLED | IRQF_SHARED);
	if (retval == 0)
		return retval;

	clk_put(jz_ohci->clk);
	clk_put(jz_ohci->clk_cgu);
	jz_stop_ohc(jz_ohci);
	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);
	return retval;
}


/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_hcd_jz_remove - shutdown processing for Jz-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_hcd_jz_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 */
static void usb_ohci_jz_remove(struct usb_hcd *hcd, struct platform_device *dev)
{
	struct jz_ohci *jz_ohci;

	jz_ohci = (struct jz_ohci *)((unsigned char *)hcd + sizeof(struct ohci_hcd));
	usb_remove_hcd(hcd);
	jz_stop_ohc(jz_ohci);
	clk_disable(jz_ohci->clk);
	clk_put(jz_ohci->clk_cgu);
	clk_put(jz_ohci->clk);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

/*-------------------------------------------------------------------------*/

static int  ohci_jz_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	int		ret;

	ohci_dbg(ohci, "ohci_jz_start, ohci:%p", ohci);
	if ((ret = ohci_init (ohci)) < 0)
		return ret;
	if ((ret = ohci_run (ohci)) < 0) {
		ohci_err(ohci,"can't start %s", hcd->self.bus_name);
		ohci_stop (hcd);
		return ret;
	}

	return 0;
}

/*-------------------------------------------------------------------------*/

static const struct hc_driver ohci_jz_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"JZ OHCI",
	.hcd_priv_size =	sizeof(struct ohci_hcd) + sizeof(struct jz_ohci),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.start =		ohci_jz_start,
	.stop =			ohci_stop,
	.shutdown =		ohci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,

#ifdef	CONFIG_PM
	.bus_suspend =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};

/*-------------------------------------------------------------------------*/

static int ohci_hcd_jz_drv_probe(struct platform_device *pdev)
{
	int ret;

	dev_dbg(&pdev->dev, "In ohci_hcd_jz_drv_probe\n");

	if (usb_disabled())
		return -ENODEV;
#ifdef CONFIG_CPU_SUSPEND_TO_IDLE
	device_init_wakeup(&pdev->dev, 1);
#endif
	ret = usb_ohci_jz_probe(&ohci_jz_hc_driver, pdev);
	return ret;
}

static int ohci_hcd_jz_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
#ifdef CONFIG_CPU_SUSPEND_TO_IDLE
	device_init_wakeup(&pdev->dev, 0);
#endif
	usb_ohci_jz_remove(hcd, pdev);
	return 0;
}

/*TBD*/
static int ohci_hcd_jz_drv_suspend(struct platform_device *pdev,pm_message_t state)
{
#ifdef CONFIG_CPU_SUSPEND_TO_IDLE
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	if (device_may_wakeup(&pdev->dev)) {
		enable_irq_wake(hcd->irq);
        }
#endif

	return 0;
}

static int ohci_hcd_jz_drv_resume(struct platform_device *pdev)
{
#ifdef CONFIG_CPU_SUSPEND_TO_IDLE
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	if (device_may_wakeup(&pdev->dev)) {
		disable_irq_wake(hcd->irq);
        }
#endif
	return 0;
}


static struct platform_driver ohci_hcd_jz_driver = {
	.probe		= ohci_hcd_jz_drv_probe,
	.remove		= ohci_hcd_jz_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.suspend	= ohci_hcd_jz_drv_suspend,
	.resume	= ohci_hcd_jz_drv_resume,
	.driver		= {
		.name	= "jz-ohci",
		.owner	= THIS_MODULE,
	},
};

