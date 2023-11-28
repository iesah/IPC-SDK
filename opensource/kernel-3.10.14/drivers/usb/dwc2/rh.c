#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <asm/unaligned.h>
#include <linux/gpio.h>

#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include "core.h"
#include "host.h"

#include "dwc2_jz.h"

#ifdef CONFIG_USB_DWC2_VERBOSE_VERBOSE
int dwc2_rh_debug_en = 0;
module_param(dwc2_rh_debug_en, int, 0644);

#define DWC2_RH_DEBUG_MSG(msg...)				\
        do {							\
		if (unlikely(dwc2_rh_debug_en)) {		\
			dwc2_printk("rh", msg);			\
		}						\
	} while(0)

#else
#define DWC2_RH_DEBUG_MSG(msg...)  do {  } while(0)
#endif

/**
 * Calculates and gets the frame Interval value of HFIR register according PHY
 * type and speed.The application can modify a value of HFIR register only after
 * the Port Enable bit of the Host Port Control and Status register
 * (HPRT.PrtEnaPort) has been set.
 */

uint32_t calc_frame_interval(struct dwc2 *dwc)
{
	gusbcfg_data_t usbcfg;
	hwcfg2_data_t hwcfg2;
	hprt0_data_t hprt0;
	int clock = 60;		// default value

	usbcfg.d32 = dwc_readl(&dwc->core_global_regs->gusbcfg);
	hwcfg2.d32 = dwc_readl(&dwc->core_global_regs->ghwcfg2);
	hprt0.d32 = dwc_readl(dwc->host_if.hprt0);

	if (!usbcfg.b.physel && usbcfg.b.ulpi_utmi_sel && !usbcfg.b.phyif)
		clock = 60;
	if (usbcfg.b.physel && hwcfg2.b.fs_phy_type == 3)
		clock = 48;
	if (!usbcfg.b.phylpwrclksel && !usbcfg.b.physel &&
		!usbcfg.b.ulpi_utmi_sel && usbcfg.b.phyif)
		clock = 30;   /* we are! */
	if (!usbcfg.b.phylpwrclksel && !usbcfg.b.physel &&
		!usbcfg.b.ulpi_utmi_sel && !usbcfg.b.phyif)
		clock = 60;
	if (usbcfg.b.phylpwrclksel && !usbcfg.b.physel &&
		!usbcfg.b.ulpi_utmi_sel && usbcfg.b.phyif)
		clock = 48;
	if (usbcfg.b.physel && !usbcfg.b.phyif && hwcfg2.b.fs_phy_type == 2)
		clock = 48;
	if (usbcfg.b.physel && hwcfg2.b.fs_phy_type == 1)
		clock = 48;

	if (hprt0.b.prtspd == 0)
		/* High speed case */
		return 125 * clock;
	else
		/* FS/LS case */
		return 1000 * clock;
}

void dwc2_hcd_handle_port_intr(struct dwc2 *dwc) {
	struct dwc2_host_if	*host_if       = &dwc->host_if;
	hprt0_data_t		 hprt0;
	int			 status_change = 0;
	hprt0.d32 = dwc_readl(host_if->hprt0);

	DWC2_RH_DEBUG_MSG("===>%s:%d: hprt0 = 0x%08x\n", __func__, __LINE__, hprt0.d32);

	if (hprt0.b.prtconndet) {
		DWC2_RH_DEBUG_MSG("Port Connect Detected(%u)\n", hprt0.b.prtconnsts);
		dwc->port1_status |= (USB_PORT_STAT_C_CONNECTION << 16);
		dwc->device_connected = !!hprt0.b.prtconnsts;
		status_change = 1;
	}

	if (hprt0.b.prtenchng) {
		DWC2_RH_DEBUG_MSG("Port Enable Changed, prtena = %d\n", hprt0.b.prtena);
		if (hprt0.b.prtena == 1) {
			hfir_data_t hfir;
			hfir.d32 = dwc_readl(&host_if->host_global_regs->hfir);
			hfir.b.frint = calc_frame_interval(dwc);
			dwc_writel(hfir.d32, &host_if->host_global_regs->hfir);
			dwc->port1_status |= ((USB_PORT_STAT_C_RESET | USB_PORT_STAT_C_ENABLE) << 16);
		} else
			dwc->port1_status |= (USB_PORT_STAT_C_ENABLE << 16);
		status_change = 1;
	}

	if (hprt0.b.prtovrcurrchng) {
		DWC2_RH_DEBUG_MSG("Port Overcurrent Changed\n");
		dwc->port1_status |= (USB_PORT_STAT_C_OVERCURRENT << 16);
		status_change = 1;
	}

	if (status_change) {
		if (dwc->hcd->status_urb)
			usb_hcd_poll_rh_status(dwc->hcd);
		else
			usb_hcd_resume_root_hub(dwc->hcd);
	}

	/* Clear Port Interrupts */
	hprt0.b.prtena = 0;   /* do not disturb prtena  */
	dwc_writel(hprt0.d32, host_if->hprt0);
}

/** Creates Status Change bitmap for the root hub and root port. The bitmap is
 * returned in buf. Bit 0 is the status change indicator for the root hub. Bit 1
 * is the status change indicator for the single root port. Returns 1 if either
 * change indicator is 1, otherwise returns 0. */
int dwc2_rh_hub_status_data(struct usb_hcd *hcd, char *buf) {
	struct dwc2	*dwc	= hcd_to_dwc2(hcd);
	int		 retval = 0;

	DWC2_RH_DEBUG_MSG("port1 status = 0x%08x\n", dwc->port1_status);

	/* called in_irq() via usb_hcd_poll_rh_status() */
	if (dwc->port1_status & 0xffff0000) {
		*buf = 0x02;
		retval = 1;
	}
	return retval;
}

static void dwc2_port_suspend(struct dwc2 *dwc, bool do_suspend) {
	hprt0_data_t hprt0;
	pcgcctl_data_t pcgcctl;

	DWC2_RH_DEBUG_MSG("%s:%d: do_suspend = %d\n", __func__, __LINE__, do_suspend);

	if (unlikely(dwc2_is_device_mode(dwc) || dwc->lx_state == DWC_OTG_L3))
		return;

	if (do_suspend) {
		if (dwc->hcd->self.b_hnp_enable) {
			gotgctl_data_t gotgctl;

			gotgctl.d32 = dwc_readl(&dwc->core_global_regs->gotgctl);
			gotgctl.b.hstsethnpen = 1;
			dwc_writel(gotgctl.d32, &dwc->core_global_regs->gotgctl);
			dwc->op_state = DWC2_A_SUSPEND;
		}

		hprt0.d32 = dwc2_hc_read_hprt(dwc);
		hprt0.b.prtsusp = 1;
		dwc_writel(hprt0.d32, dwc->host_if.hprt0);

		dwc->lx_state = DWC_OTG_L2;

		/* Suspend the Phy Clock */
		pcgcctl.d32 = dwc_readl(dwc->pcgcctl);
		pcgcctl.b.stoppclk = 1;
		dwc_writel(pcgcctl.d32, dwc->pcgcctl);
		udelay(10);

		/* For HNP the bus must be suspended for at least 200ms. */
		if (dwc->hcd->self.b_hnp_enable) {
			pcgcctl.d32 = dwc_readl(dwc->pcgcctl);
			pcgcctl.b.stoppclk = 0;
			pcgcctl.d32 = dwc_readl(dwc->pcgcctl);

			mdelay(200);
		}
	} else {
		dwc_writel(0, dwc->pcgcctl);
		mdelay(5);

		hprt0.d32 = dwc2_hc_read_hprt(dwc);
		hprt0.b.prtres = 1;
		dwc_writel(hprt0.d32, dwc->host_if.hprt0);

		hprt0.b.prtsusp = 0;
		/* Clear Resume bit */
		mdelay(100);
		hprt0.b.prtres = 0;
		dwc_writel(hprt0.d32, dwc->host_if.hprt0);
	}
}

int dwc2_port_reset(struct dwc2 *dwc)
{
	hprt0_data_t hprt0 = {.d32 = 0 };
	pcgcctl_data_t pcgcctl;

	DWC2_RH_DEBUG_MSG("%s:%d: do_reset\n", __func__, __LINE__);

	if (unlikely(dwc2_is_device_mode(dwc) || dwc->lx_state == DWC_OTG_L3))
		return -ENODEV;

	pcgcctl.d32 = dwc_readl(dwc->pcgcctl);
	pcgcctl.b.enbl_sleep_gating = 0;
	pcgcctl.b.stoppclk = 0;
	dwc_writel(pcgcctl.d32, dwc->pcgcctl);
	dwc_writel(0, dwc->pcgcctl);

	hprt0.d32 = dwc2_hc_read_hprt(dwc);
	hprt0.b.prtsusp = 0;

	if (!dwc->hcd->self.is_b_host) { /* aka, if we are A-Host */
		DWC2_RH_DEBUG_MSG("start prt reset\n");
		hprt0.b.prtpwr = 1;
		hprt0.b.prtrst = 1;
		dwc_writel(hprt0.d32, dwc->host_if.hprt0);
	}
	usleep_range(50000, 70000);
	hprt0.b.prtrst = 0;
	dwc_writel(hprt0.d32, dwc->host_if.hprt0);
	dwc->lx_state = DWC_OTG_L0;	/* Now back to the on state */
	return 0;
}

void dwc2_test_mode(struct dwc2 *dwc, int test_mode) {
	struct dwc2_host_if             *host_if     = &dwc->host_if;

	hprt0_data_t                     hprt0       = {.d32 = 0 };
	printk("bus test happend %d\n", test_mode);
	hprt0.d32 = dwc2_hc_read_hprt(dwc);
	hprt0.b.prttstctl = test_mode;
	dwc_writel(hprt0.d32, host_if->hprt0);
	printk("started packet test\n");
	return;
}

int dwc2_rh_hub_control(struct usb_hcd *hcd,
			u16 typeReq, u16 wValue, u16 wIndex,
			char *buf, u16 wLength) {
	struct dwc2	*dwc	= hcd_to_dwc2(hcd);
	hprt0_data_t	 hprt0	= {.d32 = 0 };
	unsigned int	 port_status;
	glpmcfg_data_t	 lpmcfg;
	u32		 temp;
	int		 retval = 0;

	DWC2_RH_DEBUG_MSG("%s:%d: typeReq = 0x%04x wValue = 0x%04x "
			"wIndex = 0x%04x wLength = 0x%04x\n",
			__func__, __LINE__, typeReq, wValue, wIndex, wLength);

	switch (typeReq) {
	case ClearHubFeature:
	case SetHubFeature:
		switch (wValue) {
		case C_HUB_OVER_CURRENT:
		case C_HUB_LOCAL_POWER:
			break;
		default:
			goto error;
		}
		break;

	case ClearPortFeature:
		/* we have only Port 1 */
		if ((wIndex & 0xff) != 1)
			goto error;

		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			hprt0.d32 = dwc2_hc_read_hprt(dwc);
			hprt0.b.prtena = 1;
			dwc_writel(hprt0.d32, dwc->host_if.hprt0);
			break;
		case USB_PORT_FEAT_SUSPEND:
			dwc2_port_suspend(dwc, false);
			break;
		case USB_PORT_FEAT_POWER:
			/* TODO: use platform callback instead of directly call 4780 specific routines */
			if (!hcd->self.is_b_host)
				jz_set_vbus(dwc, 0);

			hprt0.d32 = dwc2_hc_read_hprt(dwc);
			hprt0.b.prtpwr = 0;
			dwc_writel(hprt0.d32, dwc->host_if.hprt0);
			break;

		case USB_PORT_FEAT_C_CONNECTION:
		case USB_PORT_FEAT_C_ENABLE:
		case USB_PORT_FEAT_C_OVER_CURRENT:
		case USB_PORT_FEAT_C_RESET:
		case USB_PORT_FEAT_C_SUSPEND:
		case USB_PORT_FEAT_C_PORT_L1:
			dwc->port1_status &= ~(1 << wValue);
			break;
		default:
			goto error;
		}
		DWC2_RH_DEBUG_MSG("ClearPortFeature: %d\n", wValue);
		break;

	case GetHubDescriptor:
	{
		struct usb_hub_descriptor *desc = (void *)buf;

		desc->bDescLength = 9;
		desc->bDescriptorType = 0x29;
		desc->bNbrPorts = 1;
		desc->wHubCharacteristics = cpu_to_le16(
			0x0001	/* per-port power switching */
			| 0x008	/* per-port overcurrent reporting */
			);
		desc->bPwrOn2PwrGood = 5;
		desc->bHubContrCurrent = 0;
		desc->u.hs.DeviceRemovable[0] = 0x00;
		desc->u.hs.DeviceRemovable[1] = 0xff;
	}
	break;

	case GetHubStatus:
		temp = 0;
		*(__le32 *) buf = cpu_to_le32(temp);
		break;

	case GetPortStatus:
		if (!wIndex || (wIndex > 1))
			goto error;

		/* whoever resumes must GetPortStatus to complete it!! */
		if (dwc->port1_status & USB_PORT_STATE_RESUME) {
			dwc->port1_status &= ~USB_PORT_STATE_RESUME;
			dwc2_port_suspend(dwc, false);
			dwc->port1_status |= (USB_PORT_STAT_C_SUSPEND << 16);
			if (dwc->hcd->status_urb)
				usb_hcd_poll_rh_status(dwc->hcd);
			else
				usb_hcd_resume_root_hub(dwc->hcd);
		}
		port_status = dwc->port1_status;

		if (!dwc->device_connected) {
			/*
			 * The port is disconnected, which means the core is
			 * either in device mode or it soon will be. Just
			 * return 0's for the remainder of the port status
			 * since the port register can't be read if the core
			 * is in device mode.
			 */
			put_unaligned(cpu_to_le32(port_status), (__le32 *) buf);
			break;
		}

		hprt0.d32 = dwc_readl(dwc->host_if.hprt0);
		if (hprt0.b.prtconnsts)
			port_status |= USB_PORT_STAT_CONNECTION;
		if (hprt0.b.prtena)
			port_status |= USB_PORT_STAT_ENABLE;
		if (hprt0.b.prtsusp)
			port_status |= USB_PORT_STAT_SUSPEND;
		if (hprt0.b.prtovrcurract)
			port_status |= USB_PORT_STAT_OVERCURRENT;
		if (hprt0.b.prtrst)
			port_status |= USB_PORT_STAT_RESET;
		if (hprt0.b.prtpwr)
			port_status |= USB_PORT_STAT_POWER;

		if (hprt0.b.prtspd == DWC_HPRT0_PRTSPD_LOW_SPEED) {
			port_status |= USB_PORT_STAT_LOW_SPEED;
		} else if (hprt0.b.prtspd == DWC_HPRT0_PRTSPD_HIGH_SPEED) {
			port_status |= USB_PORT_STAT_HIGH_SPEED;
		}

		if (hprt0.b.prttstctl & DWC_HPRT0_PRTTST_MASK)
			port_status |= USB_PORT_STAT_TEST;

		lpmcfg.d32 = dwc_readl(&dwc->core_global_regs->glpmcfg);
		WARN(((dwc->lx_state == DWC_OTG_L1) ^ lpmcfg.b.prt_sleep_sts),
			"lx_state = %d, lmpcfg.prt_sleep_sts = %d\n",
			dwc->lx_state, lpmcfg.b.prt_sleep_sts);
		if (lpmcfg.b.prt_sleep_sts)
			port_status |= USB_PORT_STAT_L1;

		/* USB_PORT_FEAT_INDICATOR unsupported always 0 */

		DWC2_RH_DEBUG_MSG("GetPortStatus: port1_status = 0x%08x port_status = 0x%08x\n",
				dwc->port1_status, port_status);
		put_unaligned(cpu_to_le32(port_status), (__le32 *) buf);
		break;

	case SetPortFeature:
		if (wValue != USB_PORT_FEAT_TEST && (!wIndex || wIndex > 1))
			goto error;

		if (!dwc->device_connected)
			break;

		switch (wValue) {
		case USB_PORT_FEAT_POWER:
			hprt0.d32 = dwc2_hc_read_hprt(dwc);
			hprt0.b.prtpwr = 1;
			dwc_writel(hprt0.d32, dwc->host_if.hprt0);
			break;
		case USB_PORT_FEAT_RESET:
			retval = dwc2_port_reset(dwc);
			break;
		case USB_PORT_FEAT_SUSPEND:
			dwc2_port_suspend(dwc, true);
			break;
		case USB_PORT_FEAT_TEST:
			dwc2_test_mode(dwc, (wIndex >> 8));
			break;
		default:
			goto error;
		}
		dev_dbg(dwc->dev, "set feature %d\n", wValue);
		break;

	default:
	error:
		/* "protocol stall" on error */
		retval = -EPIPE;
	}
	return retval;
}

int dwc2_rh_bus_suspend(struct usb_hcd *hcd) {
	return 0;
}

int dwc2_rh_bus_resume(struct usb_hcd *hcd) {
	return 0;
}
