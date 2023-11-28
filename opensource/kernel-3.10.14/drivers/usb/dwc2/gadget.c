#include <linux/kernel.h>
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
#include <linux/module.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#define DWC_EP_PERIODIC_INT 1
#include <soc/base.h>
#include <linux/jz_dwc.h>

#include "core.h"
#include "gadget.h"
#include "debug.h"
#include "dwc2_jz.h"

#ifdef CONFIG_USB_DWC2_VERBOSE_VERBOSE
int dwc2_gadget_debug_en = 0;
module_param(dwc2_gadget_debug_en, int, 0644);

#define DWC2_GADGET_DEBUG_MSG(msg...)				\
	do {							\
		if (unlikely(dwc2_gadget_debug_en)) {		\
			dwc2_printk("gadget", msg);		\
		}						\
	} while(0)
#else
#define DWC2_GADGET_DEBUG_MSG(msg...)	do {  } while(0)
#endif

static __attribute__((unused)) void dwc2_dump_ep_regs(
	struct dwc2 *dwc, int epnum, const char *func, int line) {

	u32 r0x004, r0x014, r0x018;
	u32 r0x800, r0x804, r0x808, r0x80c, r0x810, r0x814, r0x818, r0x81c;
	u32 r0x900 = 0, r0x908 = 0, r0x910 = 0, r0x914 = 0, r0xb00 = 0, r0xb08 = 0, r0xb10 = 0, r0xb14 = 0;
	u32 r0x920 = 0, r0x928 = 0, r0x930 = 0, r0x934 = 0, r0xb20 = 0, r0xb28 = 0, r0xb30 = 0, r0xb34 = 0;
	u32 r0x940 = 0, r0x948 = 0, r0x950 = 0, r0x954 = 0, r0xb40 = 0, r0xb48 = 0, r0xb50 = 0, r0xb54 = 0;
	u32 r0x960 = 0, r0x968 = 0, r0x970 = 0, r0x974 = 0, r0xb60 = 0, r0xb68 = 0, r0xb70 = 0, r0xb74 = 0;

	if (likely(dwc2_gadget_debug_en == 0))
		return;

	DWC_RR(0x004);
	DWC_RR(0x014);
	DWC_RR(0x018);
	DWC_RR(0x800);
	DWC_RR(0x804);
	DWC_RR(0x808);
	DWC_RR(0x80c);
	DWC_RR(0x810);
	DWC_RR(0x814);
	DWC_RR(0x818);
	DWC_RR(0x81c);

	switch (epnum) {
	case -1:
	case 0:
		DWC_RR(0x900);
		DWC_RR(0x908);
		DWC_RR(0x910);
		DWC_RR(0x914);
		DWC_RR(0xb00);
		DWC_RR(0xb08);
		DWC_RR(0xb10);
		DWC_RR(0xb14);

		if (epnum != -1)
			break;
	case 1:
		DWC_RR(0x920);
		DWC_RR(0x928);
		DWC_RR(0x930);
		DWC_RR(0x934);
		DWC_RR(0xb20);
		DWC_RR(0xb28);
		DWC_RR(0xb30);
		DWC_RR(0xb34);

		if (epnum != -1)
			break;
	case 2:
		DWC_RR(0x940);
		DWC_RR(0x948);
		DWC_RR(0x950);
		DWC_RR(0x954);
		DWC_RR(0xb40);
		DWC_RR(0xb48);
		DWC_RR(0xb50);
		DWC_RR(0xb54);

		if (epnum != -1)
			break;
	case 3:
		DWC_RR(0x960);
		DWC_RR(0x968);
		DWC_RR(0x970);
		DWC_RR(0x974);
		DWC_RR(0xb60);
		DWC_RR(0xb68);
		DWC_RR(0xb70);
		DWC_RR(0xb74);
		break;
	default:
		return;
	}

	printk("=====%s:%d ep%d regs=====\n", func, line, epnum);
	DWC_P(0x004);
	DWC_P(0x014);
	DWC_P(0x018);
	DWC_P(0x800);
	DWC_P(0x804);
	DWC_P(0x808);
	DWC_P(0x80c);
	DWC_P(0x810);
	DWC_P(0x814);
	DWC_P(0x818);
	DWC_P(0x81c);

	switch (epnum) {
	case -1:
	case 0:
		DWC_P(0x900);
		DWC_P(0x908);
		DWC_P(0x910);
		DWC_P(0x914);
		DWC_P(0xb00);
		DWC_P(0xb08);
		DWC_P(0xb10);
		DWC_P(0xb14);

		if (epnum != -1)
			return;
	case 1:
		DWC_P(0x920);
		DWC_P(0x928);
		DWC_P(0x930);
		DWC_P(0x934);
		DWC_P(0xb20);
		DWC_P(0xb28);
		DWC_P(0xb30);
		DWC_P(0xb34);

		if (epnum != -1)
			return;
	case 2:
		DWC_P(0x940);
		DWC_P(0x948);
		DWC_P(0x950);
		DWC_P(0x954);
		DWC_P(0xb40);
		DWC_P(0xb48);
		DWC_P(0xb50);
		DWC_P(0xb54);

		if (epnum != -1)
			return;
	case 3:
		DWC_P(0x960);
		DWC_P(0x968);
		DWC_P(0x970);
		DWC_P(0x974);
		DWC_P(0xb60);
		DWC_P(0xb68);
		DWC_P(0xb70);
		DWC_P(0xb74);
	default:
		return;
	}
}

static struct usb_endpoint_descriptor dwc2_gadget_ep0_desc = {
	.bLength	= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bmAttributes	= USB_ENDPOINT_XFER_CONTROL,
};

/*-------------Device Mode Global Routines---------------- */

void dwc2_enable_device_interrupts(struct dwc2 *dwc)
{
	gintmsk_data_t intr_mask = {.d32 = 0 };
	dwc_otg_core_global_regs_t *global_regs = dwc->core_global_regs;

	/* Disable all interrupts. */
	dwc_writel(0, &global_regs->gintmsk);

	/* Clear any pending interrupts */
	dwc_writel(0xFFFFFFFF, &global_regs->gintsts);

	/* Enable the common interrupts */
	dwc2_enable_common_interrupts(dwc);

	/* Enable interrupts */
	intr_mask.d32 = dwc_readl(&global_regs->gintmsk);
	intr_mask.b.usbreset	    = 1;
	intr_mask.b.enumdone	    = 1;
	intr_mask.b.disconnect	    = 0;
	intr_mask.b.inepintr	    = 1;
	intr_mask.b.outepintr	    = 1;
	intr_mask.b.erlysuspend	    = 1;
	//intr_mask.b.incomplisoout = 1;
	intr_mask.b.incomplisoin    = 0;

	dwc_writel(intr_mask.d32, &global_regs->gintmsk);
}

/**
 * Device Mode initialization
 *
 * MUST ensure that dwc2_core_init() is called before you call this function
 */
void dwc2_device_mode_init(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	  = &dwc->dev_if;
	dcfg_data_t		 dcfg	  = {.d32 = 0 };
	grstctl_t		 resetctl = {.d32 = 0 };
	pcgcctl_data_t		 power;

	/* Restart the Phy Clock */
	power.d32 = dwc_readl(dwc->pcgcctl);
	if (power.b.stoppclk) {
		power.b.stoppclk = 0;
		power.b.pwrclmp = 0;
		power.b.rstpdwnmodule = 0;
		dwc_writel(power.d32, dwc->pcgcctl);
	}

	dwc->setup_prepared = 0;
	dwc->last_ep0out_normal = 0;

	/*
	 * DCFG settings
	 *
	 * Speed
	 * -----
	 * 2'b00: high speed (USB 2.0 PHY clock is 30MHZ or 60MHZ)
	 * 2'b01: full speed (USB 2.0 PHY clock is 30MHZ or 60MHZ)
	 * 2'b10: low speed (USB 1.1 FS transceiver clock is 48 MHz)
	 * 2'b11: Full speed (USB 1.1 FS transceiver clock is 48 MHz)
	 */
	dcfg.d32 = dwc_readl(&dev_if->dev_global_regs->dcfg);
#ifdef CONFIG_USB_DWC2_FULLSPEED_DEVICE
	dcfg.b.devspd = 1;
#else
	dcfg.b.devspd = 0;
#endif
	dcfg.b.descdma = (dwc->dma_desc_enable) ? 1 : 0;
	dcfg.b.perfrint = DWC_DCFG_FRAME_INTERVAL_80;
	/* Enable Device OUT NAK in case of DDMA mode*/
	//dcfg.b.endevoutnak = 1;
	dwc_writel(dcfg.d32, &dev_if->dev_global_regs->dcfg);

	/* Configure data FIFO sizes */
	calculate_fifo_size(dwc);

	/* Flush the Learning Queue. */
	resetctl.d32 = dwc_readl(&dwc->core_global_regs->grstctl);
	resetctl.b.intknqflsh = 1;
	dwc_writel(resetctl.d32, &dwc->core_global_regs->grstctl);

	/* Clear all pending Device Interrupts */
	/* TODO: if the condition need to be checked or in any case all pending interrupts should be cleared */
	dwc_writel(0, &dev_if->dev_global_regs->diepmsk);
	dwc_writel(0, &dev_if->dev_global_regs->doepmsk);
	dwc_writel(0xFFFFFFFF, &dev_if->dev_global_regs->daint);
	dwc_writel(0, &dev_if->dev_global_regs->daintmsk);

#ifdef DWC_EP_PERIODIC_INT
	{
		dctl_data_t		dctl;
		dctl.d32 = dwc_readl(&dev_if->dev_global_regs->dctl);
		dctl.b.ifrmnum = 1;
		dwc_writel(dctl.d32, &dev_if->dev_global_regs->dctl);
	}
#endif

	dwc2_enable_device_interrupts(dwc);
}
EXPORT_SYMBOL_GPL(dwc2_device_mode_init);

static int __dwc2_gadget_ep_disable(struct dwc2_ep *dep, int remove);

/*
 * Caller must take care of lock
 */
void dwc2_gadget_handle_session_end(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	int			 num_eps = dev_if->num_out_eps + dev_if->num_in_eps;
	struct dwc2_ep		*dep;
	int			 i	 = 0;

	if (dwc->ep0state != EP0_DISCONNECTED) {
		for (i = 0; i < num_eps; i++) {
			dep = dwc->eps[i];
			if (dep->flags & DWC2_EP_ENABLED) {
				__dwc2_gadget_ep_disable(dep, (dep->number == 0));
			}
		}

		if (dwc2_is_device_mode(dwc) && dwc->lx_state != DWC_OTG_L3)
			dwc2_flush_rx_fifo(dwc);

		dwc->ep0state = EP0_DISCONNECTED;
		dwc->delayed_status = false;
		dwc->delayed_status_sent = true;
		del_timer(&dwc->delayed_status_watchdog);
		dwc->gadget.b_hnp_enable = 0;
		dwc->gadget.a_hnp_support = 0;
		dwc->gadget.a_alt_hnp_support = 0;
	}
}

static void dwc2_wait_1st_ctrl_request(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	    = &dwc->dev_if;
	depctl_data_t		 doepctl;
#if 0
	deptsiz0_data_t		 doeptsiz;
	doepint_data_t		 doepint;
	u32			 setup_addr;
#endif

	int timeout = 300; /* 300ms, actually ~50ms the SETUP pkt will comming,
			    * but what a pity that we must wait in an interrupt context!
			    */
	do {
		mdelay(1);
		doepctl.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doepctl);
		//printk("===>doepctl = 0x%08x\n", doepctl.d32);
		timeout --;
	} while ((doepctl.b.epena) && (timeout > 0));

	if (timeout < 0) {
		dev_warn(dwc->dev, "wait setup pkt failed after USB reset\n");
	}
#if 0
	else {
		doepint.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doepint);
		doeptsiz.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doeptsiz);
		setup_addr = dwc_readl(&dev_if->out_ep_regs[0]->doepdma) - 8;
		//rem_supcnt = doeptsiz.b.supcnt;
		//back2back = doepint.b.back2backsetup;

		printk("===>doepctl = 0x%08x, doeptsiz = 0x%08x doepdma = 0x%08x doepint = 0x%08x, timeout = %d\n",
			doepctl.d32, doeptsiz.d32, setup_addr + 8, doepint.d32, timeout);
	}
#endif
}

void dwc2_notifier_call_chain_async(struct dwc2 *dwc);

static void dwc2_gadget_handle_usb_reset_intr(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	     = &dwc->dev_if;
	dctl_data_t		 dctl;
	depctl_data_t		 doepctl;
	daint_data_t		 daintmsk;
	doepmsk_data_t		 doepmsk;
	diepmsk_data_t		 diepmsk;
	dcfg_data_t		 dcfg;
	gintmsk_data_t		 gintsts;
	int			 i	     = 0;
	static int		 first_reset = 1;

	/* Clear the Remote Wakeup Signalling */
	dctl.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dctl);
	dctl.b.rmtwkupsig = 0;
	dctl.b.tstctl = 0;
	dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);

	dwc->test_mode = false;

	if (dwc->setup_prepared == 0) {
		dwc2_ep0_out_start(dwc);
	}
	dwc2_wait_1st_ctrl_request(dwc);

	if (likely(!first_reset)) {
		dwc2_gadget_handle_session_end(dwc);
	} else
		first_reset = 0;

	// dwc2_dump_ep_regs(dwc, 0, __func__, __LINE__);

	/* NOTE that at this point, ep0 is still not activated, see ENUM DONE */
	//dwc->ep0state = EP0_WAITING_SETUP_PHASE;
	dwc->dev_state = DWC2_DEFAULT_STATE;

	/* only disable in ep here */
	__dwc2_gadget_ep_disable(dwc->eps[dwc->dev_if.num_out_eps], 1);

	/* Set NAK for all OUT EPs */
	for (i = 0; i <= dev_if->num_out_eps; i++) {
		doepctl.d32 = dwc_readl(&dev_if->out_ep_regs[i]->doepctl);
		doepctl.b.snak = 1;
		dwc_writel(doepctl.d32, &dev_if->out_ep_regs[i]->doepctl);
	}

	daintmsk.d32 = 0;
	daintmsk.b.inep0 = 1;
	daintmsk.b.outep0 = 1;
	dwc_writel(daintmsk.d32, &dev_if->dev_global_regs->daintmsk);

	doepmsk.d32 = 0;
	doepmsk.b.setup = 1;
	doepmsk.b.xfercompl = 1;
	doepmsk.b.ahberr = 1;
	//doepmsk.b.epdisabled = 1;

	if (dwc->dma_desc_enable) {
		doepmsk.b.stsphsercvd = 1;
		doepmsk.b.bna = 1;
	}
	dwc_writel(doepmsk.d32, &dev_if->dev_global_regs->doepmsk);

	diepmsk.d32 = 0;
	diepmsk.b.xfercompl = 1;
	diepmsk.b.timeout = 1;
	//diepmsk.b.epdisabled = 1;
	diepmsk.b.ahberr = 1;
	dwc_writel(diepmsk.d32, &dev_if->dev_global_regs->diepmsk);

	/* Reset Device Address */
	dcfg.d32 = dwc_readl(&dev_if->dev_global_regs->dcfg);
	dcfg.b.devaddr = 0;
	dwc_writel(dcfg.d32, &dev_if->dev_global_regs->dcfg);

	/* Clear interrupt */
	gintsts.d32 = 0;
	gintsts.b.usbreset = 1;
	dwc_writel(gintsts.d32, &dwc->core_global_regs->gintsts);
}

static void dwc2_gadget_clear_global_in_nak(struct dwc2 *dwc);

static void dwc2_gadget_ep_activate(struct dwc2_ep *dep) {
	struct dwc2		*dwc	= dep->dwc;
	struct dwc2_dev_if	*dev_if = &dwc->dev_if;
	int			 epnum	= dep->number;
	volatile u32		*depctl_addr;
	depctl_data_t		 depctl;
	daint_data_t		 daintmsk;

	daintmsk.d32 = dwc_readl(&dev_if->dev_global_regs->daintmsk);
	if (dep->is_in) {
		depctl_addr = &dev_if->in_ep_regs[epnum]->diepctl;
		daintmsk.ep.in |= (1 << epnum);
	} else {
		depctl_addr = &dev_if->out_ep_regs[epnum]->doepctl;
		daintmsk.ep.out |= (1 << epnum);
	}

	depctl.d32 = dwc_readl(depctl_addr);

	if (dep->type == USB_ENDPOINT_XFER_ISOC && !dep->is_in) {
		doepmsk_data_t	doepmsk = { .d32 = 0 };
		doepmsk.d32 = dwc_readl(&dev_if->dev_global_regs->doepmsk);
		doepmsk.b.outtknepdis = 1;
		dwc_writel(doepmsk.d32, &dev_if->dev_global_regs->doepmsk);
	}

	if ( (!depctl.b.usbactep) || (depctl.b.mps != dep->maxp) ) {
		depctl.b.mps = dep->maxp;
		depctl.b.eptype = dep->type;
		if (dep->is_in)
			depctl.b.txfnum = epnum;
		depctl.b.setd0pid = 1;
		depctl.b.usbactep = 1;
		dwc_writel(depctl.d32, depctl_addr);
	} else {
		/* TODO: do we need deactivate then activate? */
		if (epnum != 0)
			dev_info(dwc->dev, "%s already activated\n", dep->name);
	}

	/* unmask EP interrupts */
	dwc_writel(daintmsk.d32, &dev_if->dev_global_regs->daintmsk);

	if ( (epnum == 0) && dep->is_in) {
		dwc2_gadget_clear_global_in_nak(dwc);
	}

	dev_dbg(dwc->dev, "%s activated\n", dep->name);
}

static void dwc2_gadget_stop_in_transfer(struct dwc2_ep *dep);
static void dwc2_gadget_stop_out_transfer(struct dwc2_ep *dep);

static void dwc2_gadget_stop_active_transfer(struct dwc2_ep *dep) {
	if (dep->is_in) {
		dwc2_gadget_stop_in_transfer(dep);
	} else {
		dwc2_gadget_stop_out_transfer(dep);
	}
}

static void dwc2_gadget_recover_out_transfer(struct dwc2 *dwc, struct dwc2_ep *exclude_dep);

static void dwc2_gadget_ep_deactivate(struct dwc2_ep *dep) {
	struct dwc2		*dwc	= dep->dwc;
	struct dwc2_dev_if	*dev_if = &dwc->dev_if;
	int			 epnum	= dep->number;
	volatile u32		*depctl_addr;
	depctl_data_t		 depctl;
	daint_data_t		 daintmsk;

	/* EP0 can not deactivate! */
	if (epnum == 0)
		return;

	daintmsk.d32 = dwc_readl(&dev_if->dev_global_regs->daintmsk);
	if (dep->is_in) {
		depctl_addr = &dev_if->in_ep_regs[epnum]->diepctl;
		daintmsk.ep.in &= ~(1 << epnum);
	} else {
		depctl_addr = &dev_if->out_ep_regs[epnum]->doepctl;
		daintmsk.ep.out &= ~(1 << epnum);
	}

	depctl.d32 = dwc_readl(depctl_addr);
	if (!depctl.b.usbactep) {
		dev_dbg(dwc->dev, "%s already deactivated\n", dep->name);
		return;
	}

	dwc2_gadget_stop_active_transfer(dep);
	if (!dep->is_in)
		dwc2_gadget_recover_out_transfer(dwc, dep);

	depctl.b.usbactep = 0;
	dwc_writel(depctl.d32, depctl_addr);

	/* mask EP interrupts */
	dwc_writel(daintmsk.d32, &dev_if->dev_global_regs->daintmsk);

	dev_dbg(dwc->dev, "%s deactivated\n", dep->name);
}

static int __dwc2_gadget_ep_enable(struct dwc2_ep *dep,
		const struct usb_endpoint_descriptor *desc);

static int dwc2_gadget_handle_enum_done_intr(struct dwc2 *dwc)
{
	gintsts_data_t	 gintsts;
	gusbcfg_data_t	 gusbcfg;
	struct dwc2_ep	*dep;
	dsts_data_t	 dsts;
	int		 ret;

	dwc_otg_core_global_regs_t *global_regs = dwc->core_global_regs;
	/* read the DSTS register to determine the enumeration speed */
	dsts.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dsts);

	switch (dsts.b.enumspd) {
	case DWC_DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ:
		dwc2_gadget_ep0_desc.wMaxPacketSize = cpu_to_le16(64);
		dwc->gadget.ep0->maxpacket = 64;
		dwc->gadget.ep0->maxpacket_limit = 64;
		dwc->gadget.speed = USB_SPEED_HIGH;
		break;
	case DWC_DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ:
	case DWC_DSTS_ENUMSPD_FS_PHY_48MHZ:
		dwc2_gadget_ep0_desc.wMaxPacketSize = cpu_to_le16(64);
		dwc->gadget.ep0->maxpacket = 64;
		dwc->gadget.ep0->maxpacket_limit = 64;
		dwc->gadget.speed = USB_SPEED_FULL;
		break;

	case DWC_DSTS_ENUMSPD_LS_PHY_6MHZ:
		dwc2_gadget_ep0_desc.wMaxPacketSize = cpu_to_le16(8);
		dwc->gadget.ep0->maxpacket = 8;
		dwc->gadget.ep0->maxpacket_limit = 8;
		dwc->gadget.speed = USB_SPEED_LOW;
		break;
	}

	/* Set USB turnaround time based on device speed and PHY interface. */
	gusbcfg.d32 = dwc_readl(&global_regs->gusbcfg);
	if (dwc->gadget.speed == USB_SPEED_HIGH) {
		/* 16bit UTMI+ interface */
		gusbcfg.b.usbtrdtim = 5;
	} else {	      /* Full or low speed */
		gusbcfg.b.usbtrdtim = 9;
	}
	dwc_writel(gusbcfg.d32, &global_regs->gusbcfg);

	/* NOTE: MPS is set in dwc2_gadget_ep0_activate() */

	dep = dwc2_ep0_get_in_ep(dwc);
	dep->flags &= ~DWC2_EP_ENABLED;
	ret = __dwc2_gadget_ep_enable(dep, &dwc2_gadget_ep0_desc);
	if (ret) {
		dev_err(dwc->dev, "failed to enable %s\n", dep->name);
	}

	dwc->ep0state = EP0_WAITING_SETUP_PHASE;
	dwc2_notifier_call_chain_async(dwc);

	dep = dwc2_ep0_get_out_ep(dwc);
	dep->flags &= ~DWC2_EP_ENABLED;
	ret = __dwc2_gadget_ep_enable(dep, &dwc2_gadget_ep0_desc);
	if (ret) {
		dev_err(dwc->dev, "failed to enable %s\n", dep->name);
	}

	/* NOTE: EPEna is set in dwc2_ep0_out_start() when handle usb RESET */

	/* Clear interrupt */
	gintsts.d32 = 0;
	gintsts.b.enumdone = 1;
	dwc_writel(gintsts.d32, &global_regs->gintsts);

	return 0;
}

/*
 * If the application sets or clears a STALL for an endpoint
 * due to a SetFeature.Endpoint Halt command or ClearFeature.Endpoint
 * Halt command, the Stall bit must be set or cleared before the application
 * sets up the Status stage transfer on the control endpoint.
 *
 * Caller must take care of lock
 */
static void dwc2_gadget_clear_stall(struct dwc2_ep *dep) {
	struct dwc2		*dwc	= dep->dwc;
	struct dwc2_dev_if	*dev_if	= &dwc->dev_if;
	int			 epnum	= dep->number;
	volatile u32		*depctl_addr;
	depctl_data_t		 depctl;

	if (dep->is_in) {
		depctl_addr = &dev_if->in_ep_regs[epnum]->diepctl;
	} else {
		depctl_addr = &dev_if->out_ep_regs[epnum]->doepctl;
	}

	depctl.d32 = dwc_readl(depctl_addr);

	depctl.b.stall = 0;
	/*
	 * USB Spec 9.4.5: For endpoints using data toggle, regardless
	 * of whether an endpoint has the Halt feature set, a
	 * ClearFeature(ENDPOINT_HALT) request always results in the
	 * data toggle being reinitialized to DATA0.
	 */
	if (dep->type == USB_ENDPOINT_XFER_INT ||
			dep->type == USB_ENDPOINT_XFER_BULK) {
		depctl.b.setd0pid = 1;	/* DATA0 */
	}

	dwc_writel(depctl.d32, depctl_addr);
}

/*--------------------- OUT Endpoint Routines ----------------------*/

/*
 * Caller must take care of lock
 */
static void dwc2_gadget_set_global_out_nak(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	dctl_data_t		 dctl;
	gintmsk_data_t		 gintsts;
	gintmsk_data_t		 gintmsk;
	int			 timeout = 100000;

	/* unmask OUTNakEff interrupt */
	gintmsk.d32 = dwc_readl(&dwc->core_global_regs->gintmsk);
	gintmsk.b.goutnakeff = 0;
	dwc_writel(gintmsk.d32, &dwc->core_global_regs->gintmsk);

	dctl.d32 = dwc_readl(&dev_if->dev_global_regs->dctl);
	dctl.b.sgoutnak = 1;
	dwc_writel(dctl.d32, &dev_if->dev_global_regs->dctl);

	/*
	 * the Global out NAK effective interrupt
	 * ensures that there is no data left in the RxFIFO
	 */
	do
	{
		udelay(1);
		gintsts.d32 = dwc_readl(&dwc->core_global_regs->gintsts);
		timeout --;

		if (timeout == 0) {
			dev_warn(dwc->dev, "%s:%d: gintsts = 0x%08x\n",
				__func__, __LINE__, gintsts.d32);
			dwc2_dump_ep_regs(dwc, -1, __func__, __LINE__);
			dwc->do_reset_core = 1;
		}
	} while ( (!gintsts.b.goutnakeff) && (timeout > 0));

	/* Clear Interrupt */
	gintsts.d32 = 0;
	gintsts.b.goutnakeff = 1;
	dwc_writel(gintsts.d32, &dwc->core_global_regs->gintsts);
}

static void dwc2_gadget_clear_global_out_nak(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if = &dwc->dev_if;
	dctl_data_t		 dctl;

	dctl.d32 = dwc_readl(&dev_if->dev_global_regs->dctl);
	dctl.b.cgoutnak = 1;
	dwc_writel(dctl.d32, &dev_if->dev_global_regs->dctl);

	/* TODO: I am not sure if we need enable outnak interrupt here */

	udelay(100);
}

static void __dwc2_gadget_disable_out_endpoint(struct dwc2 *dwc, int epnum, int stall) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	depctl_data_t		 depctl;
	doepint_data_t		 doepint = {.d32 = 0};
	int			 timeout = 100000;

	/*
	 * The Databook said that the application cannot disable OUT endpoint 0
	 * But what if there's an OUT packet(not SETUP packet) is transfering?
	 *
	 * TODO: I am not sure the following solution can solve this problem,
	 *       But as normal usage, EP0 does not have OUT Data Phase,
	 *       So, if no one use EP0 to transfer data, situation maybe not that BAD.
	 */
	if (epnum == 0) {
		u32 curr_dma = dwc_readl(&dev_if->out_ep_regs[0]->doepdma);
		u32 low_limit = dwc->ctrl_req_addr;
		u32 up_limit = dwc->ctrl_req_addr + DWC2_CTRL_REQ_ACTUAL_ALLOC_SIZE;
		if ( (low_limit <= curr_dma) && (curr_dma < up_limit) ) {
			return;
		}

		dev_warn(dwc->dev, "try to disable out ep0 when non SETUP packet is transfering\n");
		dwc2_ep0_out_start(dwc);
		return;
	}

	depctl.d32 = dwc_readl(&dev_if->out_ep_regs[epnum]->doepctl);
	depctl.b.epdis = 1;
	if (stall)
		depctl.b.stall = 1;
	else
		depctl.b.snak = 1;
	dwc_writel(depctl.d32, &dev_if->out_ep_regs[epnum]->doepctl);

	do
	{
		udelay(1);
		doepint.d32 = dwc_readl(&dev_if->out_ep_regs[epnum]->doepint);
		timeout --;

		if (timeout == 0) {
			dev_warn(dwc->dev, "%s:%d: doepint[%d] = 0x%08x stall = %d\n",
				__func__, __LINE__, epnum, doepint.d32, stall);
			dwc2_dump_ep_regs(dwc, -1, __func__, __LINE__);
		}
	} while ( (!doepint.b.epdisabled) && (timeout > 0));

	doepint.b.epdisabled = 1;
	dwc_writel(doepint.d32, &dev_if->out_ep_regs[epnum]->doepint);
}

/*
 * Caller must take care of lock
 */
static void dwc2_gadget_disable_out_endpoint(struct dwc2_ep *dep, int stall) {
	struct dwc2		*dwc	 = dep->dwc;

	dwc2_gadget_set_global_out_nak(dwc);
	__dwc2_gadget_disable_out_endpoint(dwc, dep->number, stall);
	dwc2_gadget_clear_global_out_nak(dwc);
}

static void dwc2_gadget_stall_non_iso_out_ep(struct dwc2_ep *dep) {
	dwc2_gadget_disable_out_endpoint(dep, 1);
}

static void dwc2_gadget_ep_start_transfer(struct dwc2_ep *dep);
/*
 * The RxFIFO is common for OUT endpoints, therefore there is
 * only one transfer stop programming flow for OUT endpoints.
 *
 * Caller must take care of lock
 */
static void dwc2_gadget_stop_out_transfer(struct dwc2_ep *dep) {
	struct dwc2		*dwc	       = dep->dwc;
	struct dwc2_dev_if	*dev_if	       = &dwc->dev_if;
	depctl_data_t		 depctl;
	int			 i	       = 0;

	//	dev_warn(dwc->dev, "%s: stop OUT transfer\n", dep->name);

	dwc2_gadget_set_global_out_nak(dwc);
	/* after set_global_out_nak, there is no data left in the RxFIFO */

	/* Enable all OUT endpoints by setting DOEPCTL.EPEna = 1'b1 */
	for (i = 0; i <= dwc->hwcfgs.hwcfg2.b.num_dev_ep; i++) {
		depctl.d32 = dwc_readl(&dev_if->out_ep_regs[i]->doepctl);
		if (!depctl.b.epena) {
			depctl.b.epena = 1;
			dwc_writel(depctl.d32, &dev_if->out_ep_regs[i]->doepctl);
		}

	}

	for (i = 0; i <= dwc->hwcfgs.hwcfg2.b.num_dev_ep; i++) {
		__dwc2_gadget_disable_out_endpoint(dwc, i, 0);
	}

	dwc2_gadget_clear_global_out_nak(dwc);
}

/* NOTE: this function MUST only be called after stop_out_transfer() and is need to resume other transfers */
static void dwc2_gadget_recover_out_transfer(struct dwc2 *dwc, struct dwc2_ep *exclude_dep) {
	struct dwc2_ep		*dep;
	struct dwc2_request	*r;
	int			 i;

	/* give bakc all current transfering request */
	for (i = 0; i < dwc->dev_if.num_out_eps; i++) {
		dep = dwc->eps[i];
		if (dep == exclude_dep)
			continue;

		if (!list_empty(&dep->request_list)) {
			r = next_request(&dep->request_list);
			if (r && r->transfering) {
				dwc2_gadget_giveback(dep, r, -ECONNRESET);
			}
		}

		/* start new request(if any) */
		dep->flags &= ~DWC2_EP_BUSY;
		dwc2_gadget_ep_start_transfer(dep);
	}
}



/*--------------------IN Endpoint Routines----------------  */

/*
 * Caller must take care of lock
 */
static __attribute__((unused)) void dwc2_gadget_set_global_np_in_nak(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	gintmsk_data_t		 gintsts;
	gintmsk_data_t		 gintmsk;
	dctl_data_t		 dctl;
	int			 timeout = 10000;

	/* unmask GInNakEff interrupt */
	gintmsk.d32 = dwc_readl(&dwc->core_global_regs->gintmsk);
	gintmsk.b.ginnakeff = 0;
	dwc_writel(gintmsk.d32, &dwc->core_global_regs->gintmsk);

	dctl.d32 = dwc_readl(&dev_if->dev_global_regs->dctl);
	dctl.b.sgnpinnak = 1;
	dwc_writel(dctl.d32, &dev_if->dev_global_regs->dctl);

	do
	{
		udelay(10);
		gintsts.d32 = dwc_readl(&dwc->core_global_regs->gintsts);
		timeout --;

		if (timeout == 0) {
			dev_warn(dwc->dev, "%s:%d: gintsts = 0x%08x\n",
				__func__, __LINE__, gintsts.d32);
			dwc2_dump_ep_regs(dwc, -1, __func__, __LINE__);
		}
	} while ( (!gintsts.b.ginnakeff) && (timeout > 0));

	gintsts.d32 = 0;
	gintsts.b.ginnakeff = 1;
	dwc_writel(gintsts.d32, &dwc->core_global_regs->gintsts);
}

static void dwc2_gadget_clear_global_in_nak(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	= &dwc->dev_if;
	dctl_data_t		 dctl;

	dctl.d32 = dwc_readl(&dev_if->dev_global_regs->dctl);
	dctl.b.cgnpinnak = 1;
	dwc_writel(dctl.d32, &dev_if->dev_global_regs->dctl);
}

/*
 * Caller must take care of lock
 */
static void dwc2_gadget_set_in_nak(struct dwc2 *dwc, int epnum) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	depctl_data_t		 depctl;
	diepint_data_t		 diepint;
	int			 timeout = 100000;

	depctl.d32 = dwc_readl(&dev_if->in_ep_regs[epnum]->diepctl);
	if (!depctl.b.epena)
		return;
	depctl.b.snak = 1;
	dwc_writel(depctl.d32, &dev_if->in_ep_regs[epnum]->diepctl);

	do
	{
		udelay(1);
		diepint.d32 = dwc_readl(&dev_if->in_ep_regs[epnum]->diepint);
		timeout --;
		if (timeout == 0) {
			dev_warn(dwc->dev, "%s:%d: diepint[%d] = 0x%08x\n",
				__func__, __LINE__, epnum, diepint.d32);
			dwc2_dump_ep_regs(dwc, -1, __func__, __LINE__);
		}
	} while ( (!diepint.b.inepnakeff) && (timeout > 0));

	diepint.b.inepnakeff = 1;
	dwc_writel(diepint.d32, &dev_if->in_ep_regs[epnum]->diepint);
}

/**
 * Note: cnak will set when dwc2_ep0_out_start()
 */
static __attribute__((unused)) void dwc2_gadget_clear_in_nak(struct dwc2 *dwc, int epnum) {
	struct dwc2_dev_if	*dev_if = &dwc->dev_if;
	depctl_data_t		 depctl;

	depctl.d32 = dwc_readl(&dev_if->in_ep_regs[epnum]->diepctl);
	depctl.b.cnak = 1;
	dwc_writel(depctl.d32, &dev_if->in_ep_regs[epnum]->diepctl);

	udelay(100);
}

static void dwc2_gadget_set_in_stall(struct dwc2 *dwc, int epnum) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	depctl_data_t		 depctl;

	depctl.d32 = dwc_readl(&dev_if->in_ep_regs[epnum]->diepctl);
	depctl.b.stall = 1;
	dwc_writel(depctl.d32, &dev_if->in_ep_regs[epnum]->diepctl);
}

static void __dwc2_gadget_disable_in_ep(struct dwc2 *dwc, int epnum) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	depctl_data_t		 depctl;
	diepint_data_t		 diepint;
	int			 timeout = 100000;

	depctl.d32 = dwc_readl(&dev_if->in_ep_regs[epnum]->diepctl);
	if (!depctl.b.epena)
		return;

	depctl.b.epdis = 1;
	dwc_writel(depctl.d32, &dev_if->in_ep_regs[epnum]->diepctl);

	do
	{
		udelay(1);
		diepint.d32 = dwc_readl(&dev_if->in_ep_regs[epnum]->diepint);
		timeout --;

		if (timeout == 0) {
			dev_warn(dwc->dev, "%s:%d: diepint[%d] = 0x%08x\n",
				__func__, __LINE__, epnum, diepint.d32);
			dwc2_dump_ep_regs(dwc, -1, __func__, __LINE__);
		}
	} while ( (!diepint.b.epdisabled) && (timeout > 0));

	diepint.b.epdisabled = 1;
	dwc_writel(diepint.d32, &dev_if->in_ep_regs[epnum]->diepint);

	dwc2_flush_tx_fifo(dwc, epnum);
}

static void dwc2_gadget_disable_in_endpoint(struct dwc2_ep *dep) {
	dwc2_gadget_set_in_nak(dep->dwc, dep->number);
	__dwc2_gadget_disable_in_ep(dep->dwc, dep->number);
}

static void dwc2_gadget_stall_non_iso_in_ep(struct dwc2_ep *dep) {
	dwc2_gadget_set_in_stall(dep->dwc, dep->number);
	__dwc2_gadget_disable_in_ep(dep->dwc, dep->number);
}

static void dwc2_gadget_stop_in_transfer(struct dwc2_ep *dep) {
	dwc2_gadget_disable_in_endpoint(dep);
}

/* --------------------------------usb_ep_ops--------------------------- */

int dwc2_has_ep_enabled(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	int			 num_eps = dev_if->num_out_eps + dev_if->num_in_eps;
	struct dwc2_ep		*dep;
	int			 i	 = 0;

	for (i = 0; i < num_eps; i++) {
		dep = dwc->eps[i];
		if (dep->flags & DWC2_EP_ENABLED) {
			return 1;
		}
	}

	return 0;
}

static int dwc2_gadget_ep0_enable(struct usb_ep *ep,
				const struct usb_endpoint_descriptor *desc)
{

	pr_info("Why you enable EP0!\n");
	return -EINVAL;
}

static int dwc2_gadget_ep0_disable(struct usb_ep *ep)
{
	pr_info("Why you disable EP0!\n");
	return -EINVAL;
}


/**
 * __dwc2_gadget_ep_enable - Initializes a HW endpoint
 * @dep: endpoint to be initialized
 * @desc: USB Endpoint Descriptor
 *
 * Caller should take care of locking
 */
static int __dwc2_gadget_ep_enable(struct dwc2_ep *dep,
				const struct usb_endpoint_descriptor *desc)
{
	if (!(dep->flags & DWC2_EP_ENABLED)) {
		dep->flags |= DWC2_EP_ENABLED;

		dep->desc = desc;
		dep->type = usb_endpoint_type(desc);
		dep->maxp = __le16_to_cpu(desc->wMaxPacketSize);
		if (dep->type == USB_ENDPOINT_XFER_INT ||
				dep->type == USB_ENDPOINT_XFER_ISOC) {
			dep->mc = dwc2_hb_mult(dep->maxp);
			dep->maxp = dwc2_max_packet(dep->maxp);
		} else {
			dep->mc = DWC_MAX_TRANSFER_SIZE / dep->maxp;
			if (dep->mc > DWC_MAX_PKT_CNT)
				dep->mc = DWC_MAX_PKT_CNT;
		}

		/* TODO: 2^(bInterval - 1) is the high-speed case, please take care of fs/ls */
		if (desc->bInterval)
			dep->interval = 1 << (desc->bInterval - 1);
	}

	dwc2_gadget_ep_activate(dep);

	return 0;
}

static int dwc2_gadget_ep_enable(struct usb_ep *ep,
				const struct usb_endpoint_descriptor *desc)
{
	struct dwc2_ep			*dep;
	struct dwc2			*dwc;
	unsigned long			flags;
	int				ret;

	if (!ep || !desc || desc->bDescriptorType != USB_DT_ENDPOINT) {
		pr_debug("dwc2: invalid parameters\n");
		return -EINVAL;
	}

	if (!desc->wMaxPacketSize) {
		pr_debug("dwc2: missing wMaxPacketSize\n");
		return -EINVAL;
	}

	dep = to_dwc2_ep(ep);
	dwc = dep->dwc;

	if (dep->flags & DWC2_EP_ENABLED) {
		dev_WARN_ONCE(dwc->dev, true, "%s is already enabled\n",
			dep->name);
		return 0;
	}

	switch (usb_endpoint_type(desc)) {
	case USB_ENDPOINT_XFER_CONTROL:
		strlcat(dep->name, "-control", sizeof(dep->name));
		break;
	case USB_ENDPOINT_XFER_ISOC:
		strlcat(dep->name, "-isoc", sizeof(dep->name));
		break;
	case USB_ENDPOINT_XFER_BULK:
		strlcat(dep->name, "-bulk", sizeof(dep->name));
		break;
	case USB_ENDPOINT_XFER_INT:
		strlcat(dep->name, "-int", sizeof(dep->name));
		break;
	default:
		dev_err(dwc->dev, "invalid endpoint transfer type\n");
	}

	DWC2_GADGET_DEBUG_MSG("Enabling %s\n", dep->name);

	dwc2_spin_lock_irqsave(dwc, flags);
	ret = __dwc2_gadget_ep_enable(dep, desc);
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return ret;
}

static void dwc2_remove_requests(struct dwc2 *dwc, struct dwc2_ep *dep, int remove);

/**
 * __dwc2_gadget_ep_disable - Disables a HW endpoint
 * @dep: the endpoint to disable
 *
 * This function also removes requests which are currently processed ny the
 * hardware and those which are not yet scheduled.
 *
 * Caller should take care of locking.
 *
 * @param remove: to make adbd happy
 */
static int __dwc2_gadget_ep_disable(struct dwc2_ep *dep, int remove)
{
	struct dwc2	*dwc   = dep->dwc;

	DWC2_GADGET_DEBUG_MSG("disable %s\n", dep->name);

	if (dwc2_is_host_mode(dwc) || dwc->lx_state == DWC_OTG_L3) {
		DWC2_GADGET_DEBUG_MSG("ep%d%s disabled when host mode or dwc powerdown\n",
				dep->number, dep->is_in ? "in" : "out");
	} else {
		dwc2_gadget_ep_deactivate(dep);

		if (dep->is_in) {
			dwc2_gadget_disable_in_endpoint(dep);
		} else {
			dwc2_gadget_disable_out_endpoint(dep, 0);
		}
	}
	dwc2_remove_requests(dwc, dep, remove);

	snprintf(dep->name, sizeof(dep->name), "ep%d%s",
		dep->number, dep->is_in ? "in" : "out");

	dep->desc = NULL;
	dep->type = 0;
	dep->maxp = 0;
	dep->flags = 0;

	if (unlikely(!dwc->plugin && !dwc2_has_ep_enabled(dwc) && !dwc->keep_phy_on))
		dwc2_turn_off(dwc, true);
	return 0;
}

static int dwc2_gadget_ep_disable(struct usb_ep *ep)
{
	struct dwc2_ep		*dep;
	struct dwc2		*dwc;
	unsigned long		 flags;
	int			 ret;
	struct dwc2_request	*req;

	DWC2_GADGET_DEBUG_MSG("%s: disable %s\n", __func__, ep->name);

	if (!ep) {
		pr_debug("dwc2: invalid parameters\n");
		return -EINVAL;
	}

	dep = to_dwc2_ep(ep);
	dwc = dep->dwc;

	dwc2_spin_lock_irqsave(dwc, flags);

	while (!list_empty(&dep->garbage_list)) {
		req = next_request(&dep->garbage_list);

		dwc2_gadget_giveback(dep, req, -ESHUTDOWN);
	}

	if (!(dep->flags & DWC2_EP_ENABLED)) {
		DWC2_GADGET_DEBUG_MSG("%s is already disabled\n", dep->name);
		dwc2_spin_unlock_irqrestore(dwc, flags);
		return 0;
	}

	ret = __dwc2_gadget_ep_disable(dep, 1);
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return ret;
}

static struct usb_request *dwc2_gadget_ep_alloc_request(struct usb_ep *ep,
							gfp_t gfp_flags)
{
	struct dwc2_request	*req;
	struct dwc2_ep		*dep = to_dwc2_ep(ep);
	struct dwc2		*dwc = dep->dwc;

	req = kzalloc(sizeof(*req), gfp_flags);
	if (!req) {
		dev_err(dwc->dev, "not enough memory\n");
		return NULL;
	}

	req->dwc2_ep	= dep;

	return &req->request;
}

static void dwc2_gadget_ep_free_request(struct usb_ep *ep,
					struct usb_request *request)
{
	struct dwc2_request	*req = to_dwc2_request(request);

	kfree(req);
}

void dwc2_gadget_giveback(struct dwc2_ep *dep,
			struct dwc2_request *req, int status)
{
	struct dwc2	*dwc = dep->dwc;
	struct usb_request *r = &req->request;
	struct dwc2_ep *real_dep = req->dwc2_ep;

	DWC2_GADGET_DEBUG_MSG("%s give back req 0x%p, status = %d\n",
			dep->name, req, status);

	req->transfering = 0;
	list_del(&req->list);

	if (req->request.status == -EINPROGRESS) {
//#ifdef CONFIG_ANDROID
		/*
		 * cli@ingenic.cn when we use linux File-backed Storage Gadget driver
		 * its seems that short_not_ok should not be saw here by dwc2 driver
		 * maybe it a gadget problem but dwc driver is work well,
		 * because it ignore it so we ignore it for a while
		 */

		if (r->short_not_ok && (r->actual < r->length) && (status == 0)) {
			DWC2_GADGET_DEBUG_MSG("trans complete success but short is not ok! "
					"req = 0x%p, actual = %d, len = %d\n",
					req, r->actual, r->length);
			status = -EINVAL;
		}
//#endif
		req->request.status = status;
	}

	if (likely(r->length && req->mapped)) {
		dma_unmap_single(dwc->dev, r->dma, r->length,
				real_dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		req->mapped = 0;
	}

	if ((unsigned int)(req->request.buf) % 4 &&
			dep->align_dma_addr && status != 0) {
		dma_unmap_single(dwc->dev, dep->align_dma_addr, req->xfersize,
				dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		dep->align_dma_addr = 0;
	}

	dev_dbg(dwc->dev, "request %p from %s completed %d/%d ===> %d\n",
		req, real_dep->name, req->request.actual,
		req->request.length, status);

	dwc2_spin_unlock(dwc);
	req->request.complete(&dep->usb_ep, &req->request);
	dwc2_spin_lock(dwc);
}

static void dwc2_remove_requests(struct dwc2 *dwc, struct dwc2_ep *dep, int remove) {
	struct dwc2_request		*req;

	if (dep->number == 0)
		dep = dwc2_ep0_get_ep0(dwc);

	while (!list_empty(&dep->request_list)) {
		req = next_request(&dep->request_list);

		if (remove) {
			dwc2_gadget_giveback(dep, req, -ESHUTDOWN);
		} else {
			list_del(&req->list);
			list_add_tail(&req->list, &dep->garbage_list);
		}
	}
}

/*
 * Caller must take care of lock
 */
static void dwc2_gadget_start_in_transfer(struct dwc2_ep *dep) {
	struct dwc2			*dwc	 = dep->dwc;
	struct dwc2_dev_if		*dev_if	 = &dwc->dev_if;
	int				 epnum	 = dep->number;
	dwc_otg_dev_in_ep_regs_t	*in_regs = dev_if->in_ep_regs[epnum];
	struct dwc2_request		*req;
	deptsiz_data_t			 deptsiz;
	depctl_data_t			 depctl;
	int	                        pktcnt;

	req = next_request(&dep->request_list);
	if (unlikely(req == NULL)) {
		DWC2_GADGET_DEBUG_MSG("%s: %s start in transfer when no request in the queue\n",
				__func__, dep->name);
		return;
	}

	deptsiz.d32 = dwc_readl(&in_regs->dieptsiz);

	if (req->trans_count_left == 0) {
		DWC2_GADGET_DEBUG_MSG("%s: sent ZLP\n", dep->name);
		req->zlp_transfered = 1;
		deptsiz.b.xfersize = 0;
		deptsiz.b.pktcnt = 1;
		pktcnt = 1;
	} else {
		int sp;
		pktcnt = req->trans_count_left / dep->maxp;
		sp = (req->trans_count_left % dep->maxp);
		if (sp)
			pktcnt++;

		if (pktcnt > dep->mc) {
			pktcnt = dep->mc;
			sp = 0;
		}

		deptsiz.b.xfersize = sp ? (pktcnt - 1) * dep->maxp + sp : pktcnt * dep->maxp;
		deptsiz.b.pktcnt = pktcnt;
	}

	req->xfersize = deptsiz.b.xfersize;
	req->pktcnt = deptsiz.b.pktcnt;
	req->transfering = 1;

	DWC2_GADGET_DEBUG_MSG("%s: trans req 0x%p, addr = 0x%08x xfersize = %d pktcnt = %d left = %d\n",
			dep->name, req, req->next_dma_addr, req->xfersize, req->pktcnt, req->trans_count_left);

	if ((unsigned int)(req->request.buf) % 4) {
		memcpy(dep->align_addr, req->request.buf + req->request.length -
				req->trans_count_left, req->xfersize);
		dep->align_dma_addr = dma_map_single(dwc->dev, dep->align_addr, req->xfersize,
				dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
	}

	/* Write the DMA register */
	if (dwc->dma_enable) {
		if (!dwc->dma_desc_enable) {
			if (dep->type == USB_ENDPOINT_XFER_INT ||
					dep->type == USB_ENDPOINT_XFER_ISOC) {
				deptsiz.b.mc = pktcnt;
			} else {
				deptsiz.b.mc = 1;
			}
			dwc_writel(deptsiz.d32, &in_regs->dieptsiz);
			if ((unsigned int)(req->request.buf) % 4 == 0)
				dwc_writel(req->next_dma_addr, &in_regs->diepdma);
			else
				dwc_writel(dep->align_dma_addr, &in_regs->diepdma);
		} else {
			/* TODO: Scatter/Gather DMA Mode here */
		}
	} else {
		/* TODO: PIO here */
	}

	if (dep->type == USB_ENDPOINT_XFER_ISOC) {
		/* TODO: ISOC here */
	}

	dep->flags |= DWC2_EP_BUSY;
	depctl.d32 = dwc_readl(&in_regs->diepctl);
	depctl.b.cnak = 1;
	depctl.b.epena = 1;
	dwc_writel(depctl.d32, &in_regs->diepctl);
}

static void dwc2_gadget_start_out_transfer(struct dwc2_ep *dep) {
	struct dwc2			*dwc	  = dep->dwc;
	struct dwc2_dev_if		*dev_if	  = &dwc->dev_if;
	int				 epnum	  = dep->number;
	dwc_otg_dev_out_ep_regs_t	*out_regs = dev_if->out_ep_regs[epnum];
	struct dwc2_request		*req;
	deptsiz_data_t			 deptsiz;
	depctl_data_t			 depctl;

	req = next_request(&dep->request_list);
	if (unlikely(req == NULL)) {
		DWC2_GADGET_DEBUG_MSG("%s: %s start out transfer when no request in the queue!\n",
				__func__, dep->name);
		return;
	}

	deptsiz.d32 = dwc_readl(&out_regs->doeptsiz);

	if (req->trans_count_left == 0) {
		DWC2_GADGET_DEBUG_MSG("%s send ZLP\n", dep->name);
		req->zlp_transfered = 1;
		deptsiz.b.xfersize = dep->maxp;
		deptsiz.b.pktcnt = 1;
	} else {
		int n;
		/*
		 * transfer size[epnum] = n * (mps[epnum])
		 * Dword Aligned
		 */

		n = req->trans_count_left / dep->maxp;
		if (n > dep->mc)
			n = dep->mc;

		if (n == 0) n = 1;

		deptsiz.b.pktcnt = n;
		deptsiz.b.xfersize = n * dep->maxp;
	}

	req->xfersize = deptsiz.b.xfersize;
	req->pktcnt = deptsiz.b.pktcnt;
	req->transfering = 1;

	DWC2_GADGET_DEBUG_MSG("%s: trans req 0x%p, addr = 0x%08x xfersize = %d pktcnt = %d left = %d\n",
			dep->name, req, req->next_dma_addr, req->xfersize, req->pktcnt, req->trans_count_left);

	if ((unsigned int)(req->request.buf) % 4) {
		dep->align_dma_addr = dma_map_single(dwc->dev, dep->align_addr, req->xfersize,
				dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
	}

	if (dwc->dma_enable) {
		if (!dwc->dma_desc_enable) {
			dwc_writel(deptsiz.d32, &out_regs->doeptsiz);
			if ((unsigned int)(req->request.buf) % 4 == 0)
				dwc_writel(req->next_dma_addr, &out_regs->doepdma);
			else
				dwc_writel(dep->align_dma_addr, &out_regs->doepdma);
		} else {
			/* TODO: Scatter/Gather DMA Mode here */
		}
	} else {
		dwc_writel(deptsiz.d32, &out_regs->doeptsiz);
	}

	if (dep->type == USB_ENDPOINT_XFER_ISOC)
	{
		/* TODO: ISOC here */
	}

	/* EP enable */
	dep->flags |= DWC2_EP_BUSY;
	depctl.d32 = dwc_readl(&out_regs->doepctl);
	depctl.b.cnak = 1;
	depctl.b.epena = 1;
	dwc_writel(depctl.d32, &out_regs->doepctl);
}

static void dwc2_gadget_ep_start_transfer(struct dwc2_ep *dep) {
	if (dep->flags & DWC2_EP_BUSY) {
		DWC2_GADGET_DEBUG_MSG("%s: %s is still busy\n", __func__, dep->name);
		return;
	}

	if ((dep->flags & DWC2_EP_ENABLED) == 0) {
		DWC2_GADGET_DEBUG_MSG("%s: %s is not enabled\n", __func__, dep->name);
		return;
	}

	if (dep->is_in) {
		dwc2_gadget_start_in_transfer(dep);
	} else {
		dwc2_gadget_start_out_transfer(dep);
	}
}

static int __dwc2_gadget_ep_queue(struct dwc2_ep *dep, struct dwc2_request *req)
{
	struct dwc2		*dwc	 = dep->dwc;
	struct usb_request	*r	 = &req->request;
	int			 ep_idle = 0;

	req->request.actual	= 0;
	req->request.status	= -EINPROGRESS;

	if (likely(r->length)) {
		r->dma = dma_map_single(dwc->dev, r->buf, r->length,
					dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);

		DWC2_GADGET_DEBUG_MSG("%s_queue: req = 0x%p len = %d mapped=0x%08x\n",
				dep->name, req, r->length, r->dma);

		if (dma_mapping_error(dwc->dev, r->dma)) {
			dev_err(dwc->dev, "failed to map buffer\n");
			return -EFAULT;
		}
	} else {
		r->dma = ~0;
	}

	req->trans_count_left = r->length;
	req->next_dma_addr = r->dma;
	req->zlp_transfered = 0;
	if (!(unsigned int)(r->buf) % 4)
		req->mapped = 1;
	else
		req->mapped = 0;

	ep_idle = list_empty(&dep->request_list);

	list_add_tail(&req->list, &dep->request_list);

	if (ep_idle) {
		dwc2_gadget_ep_start_transfer(dep);
	}

	return 0;
}

static int dwc2_gadget_ep_queue(struct usb_ep *ep,
				struct usb_request *request,
				gfp_t gfp_flags) {
	struct dwc2_request	*req = to_dwc2_request(request);
	struct dwc2_ep		*dep = to_dwc2_ep(ep);
	struct dwc2		*dwc = dep->dwc;
	unsigned long		 flags;
	int			 ret;

	DWC2_GADGET_DEBUG_MSG("queing request %p to %s length %d\n",
		request, ep->name, request->length);

	dwc2_spin_lock_irqsave(dwc, flags);

	BUG_ON(dwc2_is_host_mode(dwc) && dwc->lx_state != DWC_OTG_L3);

	if (!dep->desc) {
		dev_dbg(dwc->dev, "trying to queue request %p to disabled %s\n",
			request, ep->name);
		ret = -ESHUTDOWN;
		goto out;
	}

	ret = __dwc2_gadget_ep_queue(dep, req);

out:
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return ret;
}

static int dwc2_gadget_ep_dequeue(struct usb_ep *ep,
				struct usb_request *request) {
	struct dwc2_request	*req = to_dwc2_request(request);
	struct dwc2_request	*r   = NULL;
	struct dwc2_ep		*dep = to_dwc2_ep(ep);
	struct dwc2		*dwc = dep->dwc;
	unsigned long		 flags;
	int			 ret = 0;

	dwc2_spin_lock_irqsave(dwc, flags);

	list_for_each_entry(r, &dep->request_list, list) {
		if (r == req)
			break;
	}

	if (r != req) {
		list_for_each_entry(r, &dep->garbage_list, list) {
			if (r == req)
				break;
		}

		if (r != req) {
			ret = -EINVAL;
			goto out;
		}
	} else if (unlikely(dwc2_is_host_mode(dwc) || dwc->lx_state == DWC_OTG_L3)) {
		DWC2_GADGET_DEBUG_MSG("ep%d%s dequeue when host mode or controller powerdown\n",
				dep->number, dep->is_in ? "in": "out");
	} else if (r->transfering) {
		dwc2_gadget_stop_active_transfer(dep);
		if (dep->is_in) {
			dwc2_gadget_giveback(dep, req, -ECONNRESET);
			dwc2_gadget_ep_start_transfer(dep);
		} else {
			dwc2_gadget_recover_out_transfer(dwc, NULL);
			/* note that recover_out_transfer() will give back this active request*/
		}
		//dev_warn(dwc->dev, "WARNING: stopped an active request 0x%p on %s\n", r, dep->name);
		goto out;
	}

	dwc2_gadget_giveback(dep, req, -ECONNRESET);
out:
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return ret;
}

int __dwc2_gadget_ep_set_halt(struct dwc2_ep *dep, int value) {
	if (value) {
		if (dep->is_in) {
			dwc2_gadget_stall_non_iso_in_ep(dep);
		} else {
			dwc2_gadget_stall_non_iso_out_ep(dep);
		}
		dep->flags |= DWC2_EP_STALL;
	} else {
		if (dep->flags & DWC2_EP_WEDGE)
			return 0;

		dwc2_gadget_clear_stall(dep);

		dep->flags &= ~DWC2_EP_STALL;
	}

	return 0;
}

static int dwc2_gadget_ep_set_halt(struct usb_ep *ep, int value) {
	struct dwc2_ep	*dep = to_dwc2_ep(ep);
	struct dwc2	*dwc = dep->dwc;
	unsigned long	 flags;
	int		 ret;

	dwc2_spin_lock_irqsave(dwc, flags);

	if (dep->desc && usb_endpoint_xfer_isoc(dep->desc)) {
		dev_err(dwc->dev, "%s is of Isochronous type\n", dep->name);
		ret = -EINVAL;
		goto out;
	}
	if (!list_empty(&dep->request_list)) {
		DWC2_GADGET_DEBUG_MSG("%d %s XFer In process\n",
				dep->number,dep->is_in? "IN" : "OUT" );
		ret = -EAGAIN;
		goto out;
	} else
		ret = __dwc2_gadget_ep_set_halt(dep, value);
out:
	dwc2_spin_unlock_irqrestore(dwc, flags);
	return ret;
}

/* sets the halt feature and ignores clear requests */
static int dwc2_gadget_ep_set_wedge(struct usb_ep *ep) {
	struct dwc2_ep	*dep = to_dwc2_ep(ep);
	struct dwc2	*dwc = dep->dwc;
	unsigned long	 flags;

	dwc2_spin_lock_irqsave(dwc, flags);
	dep->flags |= DWC2_EP_WEDGE;
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return dwc2_gadget_ep_set_halt(ep, 1);
}

static const struct usb_ep_ops dwc2_gadget_ep0_ops = {
	.enable		= dwc2_gadget_ep0_enable,
	.disable	= dwc2_gadget_ep0_disable,
	.alloc_request	= dwc2_gadget_ep_alloc_request,
	.free_request	= dwc2_gadget_ep_free_request,
	.queue		= dwc2_gadget_ep0_queue,
	.dequeue	= dwc2_gadget_ep_dequeue,
	.set_halt	= dwc2_gadget_ep0_set_halt,
	.set_wedge	= dwc2_gadget_ep_set_wedge,
};

static const struct usb_ep_ops dwc2_gadget_ep_ops = {
	.enable		= dwc2_gadget_ep_enable,
	.disable	= dwc2_gadget_ep_disable,
	.alloc_request	= dwc2_gadget_ep_alloc_request,
	.free_request	= dwc2_gadget_ep_free_request,
	.queue		= dwc2_gadget_ep_queue,
	.dequeue	= dwc2_gadget_ep_dequeue,
	.set_halt	= dwc2_gadget_ep_set_halt,
	.set_wedge	= dwc2_gadget_ep_set_wedge,
};

/* -------------------------------------------------------------------------- */

static int dwc2_gadget_get_frame(struct usb_gadget *g)
{
	struct dwc2	*dwc = gadget_to_dwc(g);
	dsts_data_t dsts;

	/* read current frame/microframe number from DSTS register */
	dsts.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dsts);
	return dsts.b.soffn;
}

/**
 * This function initiates remote wakeup of the host from suspend state.
 */
static void dwc2_rem_wkup_from_suspend(struct dwc2 *dwc)
{
	dctl_data_t	dctl = { 0 };
	dsts_data_t	dsts;
	unsigned long	flags;


	dwc2_spin_lock_irqsave(dwc, flags);

	dsts.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dsts);
	if (!dsts.b.suspsts) {
		dev_warn(dwc->dev, "Remote wakeup while is not in suspend state\n");
	}

	if (dwc->remote_wakeup_enable) {
		/* TODO: if ADP enable, initiate SRP here */

		dctl.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dctl);
		dctl.b.rmtwkupsig = 1;
		dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);

		mdelay(2);

		dctl.b.rmtwkupsig = 0;
		dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);
	} else {
		dev_info(dwc->dev, "Remote Wakeup is disabled\n");
	}

	dwc2_spin_unlock_irqrestore(dwc, flags);
}

static int dwc2_gadget_wakeup(struct usb_gadget *g)
{
	struct dwc2	*dwc = gadget_to_dwc(g);
	dsts_data_t	 dsts;
	gotgctl_data_t	 gotgctl;

	if (!dwc2_is_device_mode(dwc)) return -EINVAL;

	/*
	 * This function starts the Protocol if no session is in progress. If
	 * a session is already in progress, but the device is suspended,
	 * remote wakeup signaling is started.
	 */

	/* Check if valid session */
	gotgctl.d32 = dwc_readl(&dwc->core_global_regs->gotgctl);
	if (gotgctl.b.bsesvld) {
		/* Check if suspend state */
		dsts.d32 = dwc_readl(&(dwc->dev_if.dev_global_regs->dsts));
		if (dsts.b.suspsts)
			dwc2_rem_wkup_from_suspend(dwc);
	} else {
		// dwc_otg_pcd_initiate_srp(pcd);
	}

	return 0;
}

static int dwc2_gadget_set_selfpowered(struct usb_gadget *g,
				int is_selfpowered)
{
	struct dwc2	*dwc = gadget_to_dwc(g);
	unsigned long	 flags;

	dwc2_spin_lock_irqsave(dwc, flags);
	dwc->is_selfpowered = !!is_selfpowered;
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return 0;
}

/* soft disconnect/connect */
static int dwc2_gadget_pullup(struct usb_gadget *g, int is_on)
{
	struct dwc2	*dwc = gadget_to_dwc(g);
	dctl_data_t	 dctl;
	unsigned long	 flags;

	dwc2_spin_lock_irqsave(dwc, flags);

	dwc->pullup_on = is_on;

	if (unlikely(dwc->lx_state == DWC_OTG_L3))
		goto out;

	if (is_on && !dwc->plugin)
		goto out;

	if (dwc2_is_device_mode(dwc)) {
		gotgctl_data_t gotgctl;

		gotgctl.d32 = dwc_readl(&dwc->core_global_regs->gotgctl);

		if (dwc->plugin) {
			dwc2_start_ep0state_watcher(dwc, DWC2_EP0STATE_WATCH_COUNT);
			gotgctl.b.bvalidoven = 0;
			dwc_writel(gotgctl.d32, &dwc->core_global_regs->gotgctl);
		}
		dctl.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dctl);
		dctl.b.sftdiscon = dwc->pullup_on ? 0 : 1;
		dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);

		/*
		 * Note: if we are diconnected, maybe we must stop Rx/Tx transfers
		 *       or the upper layer must take care it? I am not sure about this,
		 *       so I decide to terminate the current session here!
		 */
		if (!dwc->pullup_on) {
			udelay(300);
			dwc2_gadget_handle_session_end(dwc);
			gotgctl.b.bvalidoven = 1;
			gotgctl.b.bvalidovval = 0;
			dwc_writel(gotgctl.d32, &dwc->core_global_regs->gotgctl);
		}
	} else {
		printk("gadget pullup defered, current mode: %s, plugin: %d\n",
			dwc2_is_device_mode(dwc) ? "device" : "host", dwc->plugin);
	}
out:
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return 0;
}

static struct dwc2 *m_dwc = NULL;
int dwc2_udc_start(struct usb_gadget *gadget,
			struct usb_gadget_driver *driver)
{
	int		 ret;
	struct dwc2	*dwc = m_dwc;
	struct dwc2_ep	*dep;
	unsigned long   flags;
	dctl_data_t     dctl;

	dwc2_spin_lock_irqsave(dwc, flags);
	dctl.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dctl);
	if (unlikely(!dctl.b.sftdiscon && dwc->lx_state != DWC_OTG_L3 && dwc2_is_device_mode(dwc))) {
		/* default to 64 */
		dwc2_gadget_ep0_desc.wMaxPacketSize = cpu_to_le16(64);

		/* IN */
		dep = dwc->eps[dwc->dev_if.num_out_eps];
		ret = __dwc2_gadget_ep_enable(dep, &dwc2_gadget_ep0_desc);
		if (ret) {
			dev_err(dwc->dev, "failed to enable %s\n", dep->name);
		}

		/* OUT */
		dep = dwc->eps[0];
		ret = __dwc2_gadget_ep_enable(dep, &dwc2_gadget_ep0_desc);
		if (ret) {
			dev_err(dwc->dev, "failed to enable %s\n", dep->name);
		}
		dwc2_ep0_out_start(dwc);
	}
	dwc2_spin_unlock_irqrestore(dwc, flags);
	dwc->gadget_driver = driver;

	return 0;
}

int dwc2_udc_stop(struct usb_gadget *gadget,
			struct usb_gadget_driver *driver)
{
	struct dwc2 *dwc = m_dwc;
	dwc->gadget_driver = NULL;
	return 0;
}


static const struct usb_gadget_ops dwc2_gadget_ops = {
	.get_frame		= dwc2_gadget_get_frame,
	.wakeup			= dwc2_gadget_wakeup,
	.set_selfpowered	= dwc2_gadget_set_selfpowered,
	.pullup			= dwc2_gadget_pullup,
	.udc_start		= dwc2_udc_start,
	.udc_stop		= dwc2_udc_stop,
};

static int __dwc2_gadget_init_endpoints(struct dwc2 *dwc, int is_in) {
	struct dwc2_ep *dep;
	int epnum;
	int num_eps = is_in ? dwc->dev_if.num_in_eps : dwc->dev_if.num_out_eps;
	/* OUT then IN */
	int eps_offset = is_in ? dwc->dev_if.num_out_eps : 0;

	for (epnum = 0; epnum < num_eps; epnum++) {
		dep = kzalloc(sizeof(*dep), GFP_KERNEL);
		if (!dep) {
			dev_err(dwc->dev, "can't allocate endpoint %d\n",
				epnum);
			return -ENOMEM;
		}

		dep->dwc = dwc;
		dep->number = epnum;
		dwc->eps[epnum + eps_offset] = dep;

		snprintf(dep->name, sizeof(dep->name), "ep%d%s",
			epnum, is_in ? "in" : "out");
		dep->usb_ep.name = dep->name;
		dep->is_in = is_in;

		if (epnum == 0) {
			dep->usb_ep.maxpacket = 64;
			dep->usb_ep.maxpacket_limit = 64;
			dep->usb_ep.ops = &dwc2_gadget_ep0_ops;
			if (!is_in) {
				dwc->gadget.ep0 = &dep->usb_ep;
			}
			dep->align_addr = NULL;
		} else {
			dep->usb_ep.maxpacket = 1024;
			dep->usb_ep.maxpacket_limit = 1024;
			if (dep->is_in && epnum >= dwc->dev_if.num_in_eps - DWC_NUMBER_OF_HB_EP)
				dep->usb_ep.maxpacket_limit |= (0x2 << 11);
			dep->usb_ep.ops = &dwc2_gadget_ep_ops;
			list_add_tail(&dep->usb_ep.ep_list,
					&dwc->gadget.ep_list);
			dep->align_addr = (void *)(dwc->deps_align_addr +
					DWC2_DEP_ALIGN_ALLOC_SIZE *
					((eps_offset ? 7 : 0) + epnum - 1));
		}
		dep->align_dma_addr = 0;
		INIT_LIST_HEAD(&dep->request_list);
		INIT_LIST_HEAD(&dep->garbage_list);
	}

	return 0;
}

static int dwc2_gadget_init_endpoints(struct dwc2 *dwc)
{
	int ret;
	int epnum = dwc->dev_if.num_in_eps + dwc->dev_if.num_out_eps - 2;

	dwc->deps_align_addr = __get_free_pages(GFP_KERNEL,
			get_order(epnum * DWC2_DEP_ALIGN_ALLOC_SIZE));
	if (!dwc->deps_align_addr) {
		dev_err(dwc->dev, "can't allocate dep align memery\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&dwc->gadget.ep_list);

	/* init OUT endpoints */
	ret = __dwc2_gadget_init_endpoints(dwc, 0);
	if (ret < 0)
		return ret;

	/* init IN endpoints */
	ret = __dwc2_gadget_init_endpoints(dwc, 1);
	if (ret < 0)
		return ret;

	return ret;
}

static void dwc2_gadget_free_endpoints(struct dwc2 *dwc)
{
	struct dwc2_ep			*dep;
	u8				epnum;
	int				num_eps;

	num_eps = dwc->dev_if.num_in_eps + dwc->dev_if.num_out_eps;

	for (epnum = 0; epnum < num_eps; epnum++) {
		dep = dwc->eps[epnum];

		if (epnum != 0 && epnum != dwc->dev_if.num_out_eps)
			list_del(&dep->usb_ep.ep_list);

		kfree(dep);
	}
        free_pages(dwc->deps_align_addr,
                   get_order((num_eps - 2) * DWC2_DEP_ALIGN_ALLOC_SIZE));
}

static void dwc2_gadget_handle_early_suspend_intr(struct dwc2 *dwc)
{
	gintsts_data_t gintsts;
	dsts_data_t dsts;

	/*
	 * the programming guide said:
	 *	If the early suspend is asserted due to an erratic error,
	 *	the application can only perform a soft reset recover.
	 */
	dsts.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dsts);

	dev_dbg(dwc->dev, "Early Suspend Detected, DSTS = 0x%08x\n", dsts.d32);

	if (dsts.b.errticerr) {
		dctl_data_t	 dctl;

		dev_err(dwc->dev, "errticerr! Perform a soft reset recover\n");
		dwc2_core_init(dwc);
		dwc2_device_mode_init(dwc);

		dctl.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dctl);
		dctl.b.sftdiscon = dwc->pullup_on ? 0 : 1;
		dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);
	}

	/* Clear interrupt */
	gintsts.d32 = 0;
	gintsts.b.erlysuspend = 1;
	dwc_writel(gintsts.d32, &dwc->core_global_regs->gintsts);
}

static void dwc2_gadget_in_ep_xfer_complete(struct dwc2_ep *dep) {
	struct dwc2		*dwc	     = dep->dwc;
	struct dwc2_dev_if	*dev_if	     = &dwc->dev_if;
	int			 epnum	     = dep->number;
	int			 trans_count = 0;
	struct dwc2_request	*req;
	deptsiz_data_t		 deptsiz;

	dwc_otg_dev_in_ep_regs_t	*in_ep_regs = dev_if->in_ep_regs[epnum];

	dep->flags &= ~DWC2_EP_BUSY;

	req = next_request(&dep->request_list);
	if (unlikely( (req == NULL) || (!req->transfering))) {
	        dev_err(dwc->dev, "%s: in xfer complete when no request pending!\n", __func__);
		return;
	}

	if (dwc->dma_enable) {
		if (dwc->dma_desc_enable) {
			/* TODO: Scatter/Gather DMA Mode here */
		} else  {
			u32 curr_dma = dwc_readl(&in_ep_regs->diepdma);
			u32 low_limit = req->request.dma;
			u32 up_limit = req->request.dma + ((req->request.length + 0x3) & ~0x3);

			if ((unsigned int)(req->request.buf) % 4) {
				low_limit = dep->align_dma_addr;
				up_limit = low_limit + ((req->request.length + 0x3) & ~0x3);
			}

			if ( (curr_dma < low_limit) || (up_limit < curr_dma) ) {
				dev_err(dwc->dev, "IN_COMPL error: dma address not match, "
					"curr_dma = 0x%08x, r.dma = 0x%08x, len=%d\n",
					curr_dma, req->request.dma, req->request.length);
				return;
			}

			deptsiz.d32 = dwc_readl(&in_ep_regs->dieptsiz);
			WARN(deptsiz.b.xfersize, "xfersize if not zero when xfer complete\n");

			trans_count = req->xfersize - deptsiz.b.xfersize;
			if (unlikely(trans_count < 0)) trans_count = req->xfersize;
			req->trans_count_left -= trans_count;
			req->next_dma_addr += trans_count;

			if (req->trans_count_left || need_send_zlp(req)) {
				DWC2_GADGET_DEBUG_MSG("%s: continue trans req 0x%p\n", dep->name, req);
			} else {
				DWC2_GADGET_DEBUG_MSG("%s: req 0x%p done, give back\n", dep->name, req);
				dwc2_gadget_giveback(dep, req, 0);
				/* start new transfer */
			}

			if ((unsigned int)(req->request.buf) % 4) {
				dma_unmap_single(dwc->dev, dep->align_dma_addr, req->xfersize,
						dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
				dep->align_dma_addr = 0;
			}

			dwc2_gadget_ep_start_transfer(dep);
		}
	} else {
		/* TODO: PIO here! */
	}
}

#define CLEAR_IN_EP_INTR(__epnum,__intr)				\
	do {								\
		diepint_data_t __diepint = {.d32=0};			\
		__diepint.b.__intr = 1;					\
		dwc_writel(__diepint.d32, &dev_if->in_ep_regs[__epnum]->diepint); \
	} while (0)

static void dwc2_in_ep_interrupt(struct dwc2_ep *dep) {
	struct dwc2		*dwc	 = dep->dwc;
	int			 epnum	 = dep->number;
	struct dwc2_dev_if	*dev_if	 = &dwc->dev_if;
	diepint_data_t		 diepint = {.d32 = 0 };
	uint32_t		 msk;

	msk = dwc_readl(&dev_if->dev_global_regs->diepmsk);
	diepint.d32 = dwc_readl(&dev_if->in_ep_regs[epnum]->diepint) & msk;

	/* Transfer complete */
	if (diepint.b.xfercompl) {

		if (dep->type == USB_ENDPOINT_XFER_BULK &&
			dep->interval > 1) {
			/* TODO: need handle ISO special case here */
		}

		dwc2_gadget_in_ep_xfer_complete(dep);

		CLEAR_IN_EP_INTR(epnum, xfercompl);
		diepint.b.xfercompl = 0;
	}

	if (diepint.b.ahberr) {
		dev_err(dwc->dev, "EP%d IN AHB Error\n", epnum);

		CLEAR_IN_EP_INTR(epnum, ahberr);
		diepint.b.ahberr = 0;
	}

	if (diepint.d32) {
		dev_err(dwc->dev, "Unhandled EP%din interrupt(0x%08x)\n",
			epnum, diepint.d32);
	}
}
#undef CLEAR_IN_EP_INTR

static void dwc2_gadget_handle_in_ep_intr(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	   = &dwc->dev_if;
	int			 epnum	   = 0;
	int			 offset	   = dwc->dev_if.num_out_eps;
	uint32_t		 ep_intr;
	struct dwc2_ep		*dep;

	/* Read in the device interrupt bits */
	ep_intr = dwc_readl(&dev_if->dev_global_regs->daint) &
		dwc_readl(&dev_if->dev_global_regs->daintmsk);

	ep_intr &= 0xffff;

	/* Service the Device IN interrupts for each endpoint */
	while (ep_intr) {
		if (ep_intr & 0x1) {
			dep = dwc->eps[epnum + offset];

			if (epnum == 0) {
				dwc2_ep0_interrupt(dep);
			} else {
				dwc2_in_ep_interrupt(dep);
			}
		}

		epnum++;
		ep_intr >>= 1;
	}
}

static void dwc2_gadget_out_ep_xfer_complete(struct dwc2_ep *dep) {
	struct dwc2		*dwc	     = dep->dwc;
	struct dwc2_dev_if	*dev_if	     = &dwc->dev_if;
	int			 epnum	     = dep->number;
	int			 trans_count = 0;
	struct dwc2_request	*req;
	deptsiz_data_t		 deptsiz;

	dwc_otg_dev_out_ep_regs_t *out_ep_regs =
		dev_if->out_ep_regs[epnum];

	dep->flags &= ~DWC2_EP_BUSY;

	req = next_request(&dep->request_list);
	if (unlikely( (req == NULL) || (!req->transfering))) {
	        dev_err(dwc->dev, "%s: out xfer complete when no request pending!\n", __func__);
		return;
	}

	if (dwc->dma_enable) {
		if (dwc->dma_desc_enable) {
			/* TODO: Scatter/Gather DMA Mode here */
		} else {
			u32 curr_dma = dwc_readl(&out_ep_regs->doepdma);
			u32 low_limit = req->request.dma;
			u32 up_limit = req->request.dma + ((req->request.length + 0x3) & ~0x3);

			if ((unsigned int)(req->request.buf) % 4) {
				low_limit = dep->align_dma_addr;
				up_limit = low_limit + ((req->request.length + 0x3) & ~0x3);
			}

			if ( (curr_dma < low_limit) || (up_limit < curr_dma) ) {
				dev_err(dwc->dev, "OUT_COMPL error: dma address not match, "
					"curr_dma = 0x%08x, r.dma = 0x%08x, len=%d\n",
					curr_dma, req->request.dma, req->request.length);
				return;
			}

			if ((unsigned int)(req->request.buf) % 4) {
				dma_unmap_single(dwc->dev, dep->align_dma_addr, req->xfersize,
						dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
				memcpy(req->request.buf + req->request.length -
						req->trans_count_left, dep->align_addr, req->xfersize);
				dep->align_dma_addr = 0;
			}

			deptsiz.d32 = dwc_readl(&out_ep_regs->doeptsiz);
			trans_count = req->xfersize - deptsiz.b.xfersize;
			if (unlikely(trans_count < 0))
				trans_count = req->xfersize;

			req->trans_count_left -= trans_count;
			if (unlikely(req->trans_count_left < 0)) {
				dev_warn(dwc->dev, "%s: trans_count_left < 0! trans_count = %d\n",
					__func__, trans_count);
				deptsiz.b.xfersize = 0; /* force complete */
				req->trans_count_left = 0;
			}

			req->next_dma_addr += trans_count;
			req->request.actual += trans_count;

			DWC2_GADGET_DEBUG_MSG("%s: req 0x%p trans-ed %d bytes\n", dep->name, req, trans_count);

			/* if xfersize is not zero, we receive an short packet, the transfer is complete */
			if (!deptsiz.b.xfersize && (req->trans_count_left || need_send_zlp(req))) {
				DWC2_GADGET_DEBUG_MSG("%s: req 0x%p continue trans\n", dep->name, req);
			} else {
				DWC2_GADGET_DEBUG_MSG("%s: req 0x%p done, give back\n", dep->name, req);
				dwc2_gadget_giveback(dep, req, 0);
				/* start new transfer */
			}

			dwc2_gadget_ep_start_transfer(dep);
		}
	} else {
		/* TODO: PIO here! */
	}
}

#define CLEAR_OUT_EP_INTR(__epnum,__intr)				\
	do {								\
		doepint_data_t __doepint = {.d32=0};			\
		__doepint.b.__intr = 1;					\
		dwc_writel(__doepint.d32, &dev_if->out_ep_regs[__epnum]->doepint); \
	} while (0)

static void dwc2_out_ep_interrupt(struct dwc2_ep *dep) {
	struct dwc2		*dwc	= dep->dwc;
	int			 epnum	= dep->number;
	struct dwc2_dev_if	*dev_if = &dwc->dev_if;
	doepint_data_t		 doepint = {.d32 = 0 };
	u32			 msk;

	msk = dwc_readl(&dev_if->dev_global_regs->doepmsk);
	doepint.d32 = dwc_readl(&dev_if->out_ep_regs[epnum]->doepint);

	/* Transfer complete */
	if (doepint.b.xfercompl) {
		dwc2_gadget_out_ep_xfer_complete(dep);
		CLEAR_OUT_EP_INTR(epnum, xfercompl);
#ifdef DWC_EP_PERIODIC_INT
		if (dwc->dma_desc_enable && dep->type == USB_ENDPOINT_XFER_ISOC &&
				doepint.b.pktdrpsts)
			CLEAR_OUT_EP_INTR(epnum, pktdrpsts);
#endif
		doepint.b.xfercompl = 0;
	}

	/* AHB Error */
	if (doepint.b.ahberr) {
		dev_err(dwc->dev, "EP%d OUT AHB Error\n", epnum);
		CLEAR_OUT_EP_INTR(epnum, ahberr);
		doepint.b.ahberr = 0;
	}

	if (doepint.b.outtknepdis) {
		/*restart request*/
		CLEAR_OUT_EP_INTR(epnum, outtknepdis);
		if (dep->type == USB_ENDPOINT_XFER_ISOC)
			dwc2_gadget_start_out_transfer(dep);
		doepint.b.outtknepdis = 0;
	}

	doepint.d32 &= msk;
	if (doepint.d32) {
		dev_err(dwc->dev, "Unhandled EP%dout interrupt(0x%08x)\n",
			epnum, doepint.d32);
	}
}
#undef CLEAR_OUT_EP_INTR

static void dwc2_gadget_handle_out_ep_intr(struct dwc2 *dwc)
{
	uint32_t		 ep_intr;
	struct dwc2_dev_if	*dev_if	= &dwc->dev_if;
	int			 epnum	= 0;
	struct dwc2_ep		*dep;

	/* Read in the device interrupt bits */
	ep_intr = dwc_readl(&dev_if->dev_global_regs->daint) &
		dwc_readl(&dev_if->dev_global_regs->daintmsk);
	ep_intr &= 0xffff0000;
	ep_intr = ep_intr >> 16;

	while (ep_intr) {
		if (ep_intr & 0x1) {
			dep = dwc->eps[epnum];

			if (epnum == 0) {
				dwc2_ep0_interrupt(dep);
			} else {
				dwc2_out_ep_interrupt(dep);
			}
		}

		epnum++;
		ep_intr >>= 1;
	}
}

void dwc2_handle_device_mode_interrupt(struct dwc2 *dwc, gintsts_data_t *gintr_status) {
	if (gintr_status->b.erlysuspend) {
		dwc2_gadget_handle_early_suspend_intr(dwc);
		gintr_status->b.erlysuspend = 0;
	}
	if (gintr_status->b.usbreset) {
		dwc2_gadget_handle_usb_reset_intr(dwc);
		gintr_status->b.usbreset = 0;
	}
	if (gintr_status->b.enumdone) {
		dwc2_gadget_handle_enum_done_intr(dwc);
		gintr_status->b.enumdone = 0;
	}

	if (gintr_status->b.inepint) {
		dwc2_gadget_handle_in_ep_intr(dwc);
		gintr_status->b.inepint = 0;
	}

	if (gintr_status->b.outepintr) {
		dwc2_gadget_handle_out_ep_intr(dwc);
		gintr_status->b.outepintr = 0;
	}

	if (gintr_status->b.incomplisoin) {
		dev_info(dwc->dev, "incomplisoin!!!\n");
		/* TODO:  */
		//dwc_otg_pcd_handle_incomplete_isoc_in_intr(pcd);
		gintr_status->b.incomplisoin = 0;
	}
}

static BLOCKING_NOTIFIER_HEAD(dwc2_notify_chain);

#ifdef CONFIG_USB_CHARGER_SOFTWARE_JUDGE
int dwc2_register_state_notifier(struct notifier_block *nb)
{
        return blocking_notifier_chain_register(&dwc2_notify_chain, nb);
}
EXPORT_SYMBOL(dwc2_register_state_notifier);

void dwc2_unregister_state_notifier(struct notifier_block *nb)
{
        blocking_notifier_chain_unregister(&dwc2_notify_chain, nb);
}
EXPORT_SYMBOL(dwc2_unregister_state_notifier);

static void dwc2_ep0state_watcher_work(struct work_struct *work)
{
	struct dwc2	*dwc   = container_of(work, struct dwc2, ep0state_watcher_work);
	int		 discon	= 0;
	unsigned long	 flags;

	dwc2_spin_lock_irqsave(dwc, flags);
	discon = dwc->ep0state == EP0_DISCONNECTED;
	dwc2_spin_unlock_irqrestore(dwc, flags);

	DWC2_GADGET_DEBUG_MSG("%s: discon = %d\n", __func__, discon);
	blocking_notifier_call_chain(&dwc2_notify_chain, discon, NULL);
}

void dwc2_notifier_call_chain_async(struct dwc2 *dwc) {
	dwc->ep0state_watch_count = 0;
	schedule_work(&dwc->ep0state_watcher_work);
}

static void dwc2_ep0state_watcher(unsigned long _dwc) {
	struct dwc2	*dwc	= (struct dwc2 *)_dwc;
	int		 notify = 0;
	unsigned long	 flags;

	dwc2_spin_lock_irqsave(dwc, flags);

	dev_dbg(dwc->dev, "enter %s ep0state = %d ep0state_watch_count = %d\n",
		__func__, dwc->ep0state, dwc->ep0state_watch_count);

	if (dwc->ep0state_watch_count) {
		if (dwc->ep0state != EP0_DISCONNECTED) {
			dwc->ep0state_watch_count = 0;
			notify = 1;
		} else {
			/* retry */
			dwc->ep0state_watch_count --;
			mod_timer(&dwc->ep0state_watcher,
				jiffies + msecs_to_jiffies(DWC2_EP0STATE_WATCH_INTERVAL));
		}
	} else {
		notify = 1;
	}
	dwc2_spin_unlock_irqrestore(dwc, flags);

	if (notify)
		schedule_work(&dwc->ep0state_watcher_work);
}

/* caller must takecare of lock */
void dwc2_start_ep0state_watcher(struct dwc2 *dwc, int count) {
	dwc->ep0state_watch_count = count;

	if (count == 0) {
		schedule_work(&dwc->ep0state_watcher_work);
		return;
	}

	mod_timer(&dwc->ep0state_watcher,
		jiffies + msecs_to_jiffies(DWC2_EP0STATE_WATCH_INTERVAL));
}
#endif


/**
 * dwc2_gadget_init - Initializes gadget related registers
 * @dwc: pointer to our controller context structure
 *
 * Returns 0 on success otherwise negative errno.
 */
int dwc2_gadget_init(struct dwc2 *dwc)
{
	int	ret;

	/*
	 * Initialize the DMA buffer for SETUP packets
	 *
	 * NOTE: we know that when call dma_alloc_coherent, actually we will get at least one PAGE,
	 *       if this is not the case in your platform, please use __get_free_pages() directly
	 */
	dwc->ctrl_req_virt = dma_alloc_coherent(dwc->dev,
						DWC2_CTRL_REQ_ACTUAL_ALLOC_SIZE,
						&dwc->ctrl_req_addr, GFP_KERNEL);
	DWC2_GADGET_DEBUG_MSG("ctrl_req_virt = 0x%p, ctrl_req_addr = 0x%08x\n",
			dwc->ctrl_req_virt, dwc->ctrl_req_addr);
	if (!dwc->ctrl_req_virt) {
		dev_err(dwc->dev, "failed to allocate ctrl request\n");
		ret = -ENOMEM;
		goto err0;
	}

	dwc->status_buf = (u16 *)kzalloc(sizeof(u16), GFP_ATOMIC);
	dwc->status_buf_addr = virt_to_phys(dwc->status_buf);
	dwc->ep0out_shadow_buf = (u8 *)kzalloc(DWC_EP0_MAXPACKET, GFP_ATOMIC);
	dwc->ep0out_shadow_uncached = UNCAC_ADDR((void *)dwc->ep0out_shadow_buf);
	dwc->ep0out_shadow_dma = dma_map_single(dwc->dev,
						dwc->ep0out_shadow_buf,
						DWC_EP0_MAXPACKET,
						DMA_FROM_DEVICE);
	dwc->gadget.name	  = "dwc2-gadget";
	dwc->gadget.ops		 = &dwc2_gadget_ops;
	dwc->gadget.max_speed	 = USB_SPEED_HIGH;
	dwc->gadget.is_otg	 = 1;
	dwc->gadget.speed	 = USB_SPEED_UNKNOWN;
	dwc->gadget.state	 = USB_STATE_NOTATTACHED;
	dwc->gadget.out_epnum	 = 0;
	dwc->gadget.in_epnum	 = 0;

	init_timer(&dwc->delayed_status_watchdog);
	dwc->delayed_status_watchdog.function = dwc2_delayed_status_watchdog;
	dwc->delayed_status_watchdog.data = (unsigned long)dwc;

	dwc->ep0state = EP0_DISCONNECTED;

#ifdef CONFIG_USB_CHARGER_SOFTWARE_JUDGE
	init_timer(&dwc->ep0state_watcher);
	dwc->ep0state_watcher.function = dwc2_ep0state_watcher;
	dwc->ep0state_watcher.data = (unsigned long)dwc;
	INIT_WORK(&dwc->ep0state_watcher_work, dwc2_ep0state_watcher_work);
#endif

	ret = dwc2_gadget_init_endpoints(dwc);
	if (ret)
		goto err1;

	if (dwc2_is_device_mode(dwc))
		dwc2_device_mode_init(dwc);

	ret = usb_add_gadget_udc(dwc->dev, &dwc->gadget);
	if (ret) {
		dev_err(dwc->dev, "failed to register gadget device\n");
		put_device(&dwc->gadget.dev);
		goto err2;
	}

	m_dwc = dwc;

	return 0;

err2:
	dwc2_gadget_free_endpoints(dwc);

err1:
	dma_free_coherent(dwc->dev, DWC2_CTRL_REQ_ACTUAL_ALLOC_SIZE,
			dwc->ctrl_req_virt, dwc->ctrl_req_addr);
err0:
	return ret;
}

void dwc2_gadget_exit(struct dwc2 *dwc)
{
	m_dwc = NULL;

	usb_del_gadget_udc(&dwc->gadget);

	dwc2_gadget_free_endpoints(dwc);

	dma_free_coherent(dwc->dev, DWC2_CTRL_REQ_ACTUAL_ALLOC_SIZE,
			dwc->ctrl_req_virt, dwc->ctrl_req_addr);

	dma_unmap_single(dwc->dev, dwc->ep0out_shadow_dma, DWC_EP0_MAXPACKET, DMA_FROM_DEVICE);

	kfree(dwc->ep0out_shadow_buf);

	kfree(dwc->status_buf);
}

void dwc2_gadget_plug_change(int plugin)  {
	dctl_data_t	 dctl;
	unsigned long	 flags;
	struct dwc2	*dwc	      = m_dwc;

	if (!dwc)
		return;

	dwc2_spin_lock_irqsave(dwc, flags);

	dwc->plugin = !!plugin;

	if (dwc->suspended)
		goto out;

	if (!plugin && dwc->lx_state ==	DWC_OTG_L3)
		goto out_print;

	if (plugin)
		dwc2_turn_on(dwc);

	if (!dwc2_is_device_mode(dwc))
		goto unplug;

	dctl.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dctl);
	if (plugin) {
		if (dwc->pullup_on) {
			gotgctl_data_t gotgctl;
			gotgctl.d32 = dwc_readl(&dwc->core_global_regs->gotgctl);
			if (gotgctl.b.bvalidoven) {
				gotgctl.b.bvalidoven = 0;
				dwc_writel(gotgctl.d32, &dwc->core_global_regs->gotgctl);
			}
		}
		dctl.b.sftdiscon = dwc->pullup_on ? 0 : 1;
		dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);
		if (dwc->pullup_on) {
			dwc2_start_ep0state_watcher(dwc, DWC2_EP0STATE_WATCH_COUNT);
		}
	} else {
		dctl.b.sftdiscon = 1;
		dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);

		if (IS_ENABLED(CONFIG_USB_DWC2_DEVICE_ONLY)){
			gotgctl_data_t gotgctl;
			gotgctl.d32 = dwc_readl(&dwc->core_global_regs->gotgctl);
			gotgctl.b.bvalidoven = 1;
			gotgctl.b.bvalidovval = 0;
			dwc_writel(gotgctl.d32, &dwc->core_global_regs->gotgctl);
		}
unplug:
		if (!dwc->keep_phy_on)
			dwc2_turn_off(dwc, true);
	}
out_print:
	dev_info(dwc->dev,"enter %s:%d: plugin = %d pullup_on = %d suspend = %d\n",
		__func__, __LINE__, plugin, dwc->pullup_on, dwc->suspended);
out:
	dwc2_spin_unlock_irqrestore(dwc, flags);
}
EXPORT_SYMBOL(dwc2_gadget_plug_change);

void dwc2_host_trace_print(void) {
	__dwc2_host_trace_print(m_dwc, 0);
}
EXPORT_SYMBOL(dwc2_host_trace_print);
