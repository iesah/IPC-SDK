#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>

#include "core.h"
#include "gadget.h"


#ifdef CONFIG_USB_DWC2_VERBOSE_VERBOSE
static int dwc2_ep0_debug_en = 0;
module_param(dwc2_ep0_debug_en, int, 0644);
#define DWC2_EP0_DEBUG_MSG(msg...)				\
        do {							\
		if (unlikely(dwc2_ep0_debug_en)) {		\
			dwc2_printk("ep0", msg);		\
		}						\
        } while(0)
#else
#define DWC2_EP0_DEBUG_MSG(msg...)	do {  } while(0)
#endif


static __attribute__((unused)) void dwc2_ep0_dump_regs(struct dwc2 *dwc) {
	unsigned int r0x800, r0x804, r0x808, r0x80c, r0x810, r0x814, r0x818, r0x81c;
	unsigned int r0x900, r0x908, r0x910, r0x914;
	unsigned int r0xb00, r0xb08, r0xb10, r0xb14;

	DWC_RR(0x900);
	DWC_RR(0x908);
	DWC_RR(0x910);
	DWC_RR(0x914);

	DWC_RR(0xb00);
	DWC_RR(0xb08);
	DWC_RR(0xb10);
	DWC_RR(0xb14);

	DWC_RR(0x800);
	DWC_RR(0x804);
	DWC_RR(0x808);
	DWC_RR(0x80c);
	DWC_RR(0x810);
	DWC_RR(0x814);
	DWC_RR(0x818);
	DWC_RR(0x81c);

	/*--------------------------*/

	DWC_P(0x800);
	DWC_P(0x804);
	DWC_P(0x808);
	DWC_P(0x80c);
	DWC_P(0x810);
	DWC_P(0x814);
	DWC_P(0x818);
	DWC_P(0x81c);

	DWC_P(0x900);
	DWC_P(0x908);
	DWC_P(0x910);
	DWC_P(0x914);

	DWC_P(0xb00);
	DWC_P(0xb08);
	DWC_P(0xb10);
	DWC_P(0xb14);

	/*--------------------------*/

	DWC_PR(0x800);
	DWC_PR(0x804);
	DWC_PR(0x808);
	DWC_PR(0x80c);
	DWC_PR(0x810);
	DWC_PR(0x814);
	DWC_PR(0x818);
	DWC_PR(0x81c);

	DWC_PR(0x900);
	DWC_PR(0x908);
	DWC_PR(0x910);
	DWC_PR(0x914);

	DWC_PR(0xb00);
	DWC_PR(0xb08);
	DWC_PR(0xb10);
	DWC_PR(0xb14);

	/*--------------------------*/
}

static const char *dwc2_ep0_state_string(enum dwc2_ep0_state state)
{
	switch (state) {
	case EP0_WAITING_SETUP_PHASE:
		return "Waiting Setup Phase";
	case EP0_SETUP_PHASE:
		return "Setup Phase";
	case EP0_DATA_PHASE:
		return "Data Phase";
	case EP0_STATUS_PHASE:
		return "Status Phase";
	default:
		return "UNKNOWN";
	}
}

/**
 * This function configures EPO to receive SETUP packets.
 *
 * Priciple: prepare to receive as sooooooon as the OUT ep is idle!
 */
void dwc2_ep0_out_start(struct dwc2 *dwc) {
	struct dwc2_dev_if	*dev_if	    = &dwc->dev_if;
	deptsiz0_data_t		 doeptsize0 = {.d32 = 0 };
	depctl_data_t		 doepctl    = {.d32 = 0 };
	doepmsk_data_t		 doepmsk;
	doepint_data_t		 doepint;
	u32			 doepdma;

	doepctl.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doepctl);
	doeptsize0.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doeptsiz);
	doepdma = dwc_readl(&dev_if->out_ep_regs[0]->doepdma);

	if (unlikely(dwc->last_ep0out_normal))
		goto directly_start;

	if (dwc->setup_prepared) {
		if (!doepctl.b.epena) {
			DWC2_EP0_DEBUG_MSG("setup prepared but ep0out is stoped by someone! "
					"doeptsize0 = 0x%08x doepdma = 0x%08x\n",
					doeptsize0.d32, doepdma);
		} else {
			DWC2_EP0_DEBUG_MSG("setup already prepared and ep0out is running! "
					"doeptsize0 = 0x%08x doepdma = 0x%08x\n",
					doeptsize0.d32, doepdma);
		}

		return;
	}

	if (unlikely(doepctl.b.epena)) {
		DWC2_EP0_DEBUG_MSG("ep0 is already enabled! doepctl = 0x%08x, doeptsize0 = 0x%08x\n",
				doepctl.d32, doeptsize0.d32);
		return;
	}

	doepmsk.d32 = dwc_readl(&dev_if->dev_global_regs->doepmsk);
	doepint.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doepint) & doepmsk.d32;
	if (unlikely(doepint.b.setup)) {
		DWC2_EP0_DEBUG_MSG("%s: a setup packet is already pending\n", __func__);
		return;
	}

directly_start:
	dwc->setup_prepared = 1;
        dwc->last_ep0out_normal = 0;

	doeptsize0.b.supcnt = 3;
	doeptsize0.b.pktcnt = 1;
	doeptsize0.b.xfersize = 8 * 3;

	if (dwc->dma_enable) {
		if (!dwc->dma_desc_enable) {
			/* TODO: if the endpoint is just transfering a SETUP packet,
			 * and we change these registers, what will happend?
			 *
			 * But how do we know that the core is just transfering a SETUP packet?
			 */

			dwc_writel(doeptsize0.d32, &dev_if->out_ep_regs[0]->doeptsiz);
			dwc_writel(dwc->ctrl_req_addr, &dev_if->out_ep_regs[0]->doepdma);
		} else {
			/* TODO: Scatter/Gather DMA here */
		}
	} else {
		dwc_writel(doeptsize0.d32, &dev_if->out_ep_regs[0]->doeptsiz);
	}

	DWC2_EP0_DEBUG_MSG("dwc2_ep0_out_start(0x%08x)\n", dwc->ctrl_req_addr);

	doepctl.b.cnak = 1;
	doepctl.b.epena = 1;
	dwc_writel(doepctl.d32, &dev_if->out_ep_regs[0]->doepctl);
}

static void dwc2_ep0_status_cmpl(struct usb_ep *ep, struct usb_request *req)
{
	/* do nothing */
}

static void dwc2_ep0_start_transfer(struct dwc2 *dwc,
				struct dwc2_request *req);

/**
 * Problem: as soon as we receive the xfercompl intr for status phase,
 *       a new SETUP packet may comming, so we must prepare this.
 *
 * Solution: when do OUT status phase, call dwc2_ep0_out_start() instead,
 *
 *           when do IN status phase, firstly call dwc2_ep0_out_start(),
 *           then do a ZPL IN status transfer
 */
static int dwc2_ep0_do_status_phase(struct dwc2 *dwc) {
	unsigned	is_in = !dwc->ep0_expect_in;

	dwc->ep0state = EP0_STATUS_PHASE;
	dwc2_ep0_out_start(dwc);

	if (!is_in) {	      /* OUT status phase */
		return 0;
	} else {
		dwc->ep0_usb_req.dwc2_ep = dwc2_ep0_get_in_ep(dwc);
		dwc->ep0_usb_req.request.length = 0;
		dwc->ep0_usb_req.request.buf = (void *)0xFFFFFFFF;
		dwc->ep0_usb_req.request.complete = dwc2_ep0_status_cmpl;
		dwc->ep0_usb_req.transfering = 0;
		/* NOTE: this req is not add to request_list, so giveback will not be called */
		dwc2_ep0_start_transfer(dwc, &dwc->ep0_usb_req);
		return 0;
	}
}

/*
 * ch 9.4.6, the spec said:
 *     The USB device does not change its device address until after
 *     the Status stage of this request is completed successfully
 * TODO: do we need to set address after status stage compl?
 */
static int dwc2_ep0_set_address(struct dwc2 *dwc, struct usb_ctrlrequest *ctrl)
{
	dcfg_data_t	dcfg;
	u32		addr;

	addr = le16_to_cpu(ctrl->wValue);
	if (addr > 127) {
		dev_dbg(dwc->dev, "invalid device address %d\n", addr);
		return -EINVAL;
	}

	if (dwc->dev_state == DWC2_CONFIGURED_STATE) {
		dev_dbg(dwc->dev, "trying to set address when configured\n");
		return -EINVAL;
	}

	dcfg.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dcfg);
	dcfg.b.devaddr = addr;
	dwc_writel(dcfg.d32, &dwc->dev_if.dev_global_regs->dcfg);

	if (addr)
		dwc->dev_state = DWC2_ADDRESS_STATE;
	else
		dwc->dev_state = DWC2_DEFAULT_STATE;

	dwc2_ep0_do_status_phase(dwc);

	return 0;
}

static int dwc2_ep0_delegate_req(struct dwc2 *dwc, struct usb_ctrlrequest *ctrl)
{
	int ret;

	dwc2_spin_unlock(dwc);
	ret = dwc->gadget_driver->setup(&dwc->gadget, ctrl);
	dwc2_spin_lock(dwc);
	return ret;
}

static int dwc2_ep0_set_config(struct dwc2 *dwc, struct usb_ctrlrequest *ctrl)
{
	u32 cfg;
	int ret;

	cfg = le16_to_cpu(ctrl->wValue);

	switch (dwc->dev_state) {
	case DWC2_DEFAULT_STATE:
		return -EINVAL;
		break;

	case DWC2_ADDRESS_STATE:
		ret = dwc2_ep0_delegate_req(dwc, ctrl);
		/* if the cfg matches and the cfg is non zero */
		if (cfg && (!ret || (ret == USB_GADGET_DELAYED_STATUS))) {
			dwc->dev_state = DWC2_CONFIGURED_STATE;
		}
		break;

	case DWC2_CONFIGURED_STATE:
		ret = dwc2_ep0_delegate_req(dwc, ctrl);
		if (!cfg)
			dwc->dev_state = DWC2_ADDRESS_STATE;
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static void dwc2_ep0_start_in_transfer(struct dwc2 *dwc,
				struct dwc2_request *req) {
	dwc_otg_dev_in_ep_regs_t	*in_regs = dwc->dev_if.in_ep_regs[0];
	struct dwc2_ep			*dep	 = req->dwc2_ep;
	depctl_data_t			 depctl;
	deptsiz0_data_t			 deptsiz;

	deptsiz.d32 = dwc_readl(&in_regs->dieptsiz);

	if (req->trans_count_left == 0) {
		/* Zero Packet */
		DWC2_EP0_DEBUG_MSG("IN zero packet\n");
		req->zlp_transfered = 1;
		deptsiz.b.xfersize = 0;
	} else {
		if (req->trans_count_left > dep->maxp) {
			deptsiz.b.xfersize = dep->maxp;
		} else {
			deptsiz.b.xfersize = req->trans_count_left;
		}
	}

	deptsiz.b.pktcnt = 1;

	req->xfersize = deptsiz.b.xfersize;
	req->pktcnt = 1;
	req->transfering = 1;

	DWC2_EP0_DEBUG_MSG("IN: xfersize = %d next_dma_addr = 0x%08x zlp_transfered = %d dep->maxp = %d\n",
			req->xfersize, req->next_dma_addr, req->zlp_transfered, dep->maxp);

	/* Write the DMA register */
	if (dwc->dma_enable) {
		if (!dwc->dma_desc_enable) {
			dwc_writel(req->next_dma_addr, &in_regs->diepdma);
			dwc_writel(deptsiz.d32, &in_regs->dieptsiz);
		} else {
			/* TODO: Scatter/Gather DMA mode here */
		}
	} else {
		dwc_writel(deptsiz.d32, &in_regs->dieptsiz);
	}

	/* EP enable, IN data in FIFO */
	dep->flags |= DWC2_EP_BUSY;
	depctl.d32 = dwc_readl(&in_regs->diepctl);
	depctl.b.epena = 1;
	depctl.b.cnak = 1;
	dwc_writel(depctl.d32, &in_regs->diepctl);
}

static void dwc2_ep0_start_out_transfer(struct dwc2 *dwc,
					struct dwc2_request *req) {
	dwc_otg_dev_out_ep_regs_t	*out_regs = dwc->dev_if.out_ep_regs[0];
	depctl_data_t			 depctl;
	struct dwc2_ep			*dep	  = req->dwc2_ep;
	deptsiz0_data_t			 deptsiz;

	/* NOTE: we transfer OUT packet by packet */
	deptsiz.d32 = dwc_readl(&out_regs->doeptsiz);
	deptsiz.b.xfersize = dep->maxp;
	deptsiz.b.supcnt = 0;
	deptsiz.b.pktcnt = 1;

	if (req->trans_count_left == 0)
		req->zlp_transfered = 1;

	req->xfersize = dep->maxp;
	req->pktcnt = 1;
	req->transfering = 1;

	DWC2_EP0_DEBUG_MSG("OUT: next_dma_addr = 0x%08x\n", req->next_dma_addr);

	if (dwc->dma_enable) {
		if (!dwc->dma_desc_enable) {
			dwc_writel(deptsiz.d32, &out_regs->doeptsiz);
			dwc_writel(req->next_dma_addr, &out_regs->doepdma);
		} else {
			/* TODO: Scatter/Gather DMA Mode here */
		}
	} else {
		dwc_writel(deptsiz.d32, &out_regs->doeptsiz);
	}

	/* EP enable */
	dep->flags |= DWC2_EP_BUSY;
	depctl.d32 = dwc_readl(&out_regs->doepctl);
	depctl.b.epena = 1;
	depctl.b.cnak = 1;
	dwc_writel(depctl.d32, &out_regs->doepctl);
}

static void dwc2_ep0_start_transfer(struct dwc2 *dwc,
				struct dwc2_request *req) {
	struct dwc2_ep		*dep = req->dwc2_ep;
	struct usb_request	*r   = &req->request;

	if (dep->flags & DWC2_EP_BUSY) {
		DWC2_EP0_DEBUG_MSG("%s: %s is busy\n", __func__, dep->name);
		return;
	}

	DWC2_EP0_DEBUG_MSG("req 0x%p is_in = %d transfering = %d length = %d mapped = %d\n",
			req, dep->is_in, req->transfering, r->length, req->mapped);

	if (dep->is_in) {
		if (!req->transfering) {
			if (( r->length != 0) && !req->mapped) {
				r->dma = dma_map_single(dwc->dev, r->buf, r->length,
							dep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);

				if (dma_mapping_error(dwc->dev, r->dma)) {
					dev_err(dwc->dev, "failed to map buffer\n");
					return;
				}

				DWC2_EP0_DEBUG_MSG("req 0x%p mapped to 0x%08x, length = %d\n", req, r->dma, r->length);

				req->trans_count_left = r->length;
				req->next_dma_addr = r->dma;
				req->zlp_transfered = 0;
				req->mapped = 1;
			} else if (r->length == 0) {
				req->trans_count_left = 0;
				req->next_dma_addr = 0;
				req->zlp_transfered = 0;
			}
		}

		dwc2_ep0_start_in_transfer(dwc, req);
	} else {
		if (!req->transfering) {
			req->trans_count_left = r->length;
			req->next_dma_addr = dwc->ep0out_shadow_dma;
			req->zlp_transfered = 0;
		}
		dwc2_ep0_start_out_transfer(dwc, req);
	}
}

static int __dwc2_gadget_ep0_queue(struct dwc2_ep *outep0,
				struct dwc2_request *req) {

	struct dwc2	*dwc	   = outep0->dwc;
	struct dwc2_ep	*dep;
	int		 can_queue = 0;

	BUG_ON(outep0->is_in);
	BUG_ON(dwc2_is_host_mode(dwc));

	DWC2_EP0_DEBUG_MSG("ep0_queue: req = 0x%p len = %d zero = %d delayed_status = %d ep0state=%s three=%d\n",
			   req, req->request.length, req->request.zero,
			   dwc->delayed_status,
			   dwc2_ep0_state_string(dwc->ep0state),
			   dwc->three_stage_setup);

	/*
	 * NOTE: currently we do not support queue more than one request on EP0
	 *       if someone wish to use EP0 as "bulk", he may remove this restrict.
	 */
	if (!list_empty(&outep0->request_list)) {
		dev_err(dwc->dev, "ep0 request list is not empty when attemp to queue a new request!!!\n");
		dump_stack();
		return -EBUSY;
	}

	req->request.actual	= 0;
	req->request.status	= -EINPROGRESS;

	/*
	 * Although there's IN and OUT EP0, but EP0 is actually not bi-direction,
	 * so, all EP0 request are queue to OUT EP0 request_list,
	 * request's real ep will assign later
	 */
	list_add_tail(&req->list, &outep0->request_list);

	/*
	 * In case gadget driver asked us to delay the STATUS phase,
	 * handle it here.
	 */
	if (dwc->delayed_status) {
		dwc->delayed_status = false;

		/* Assign Real EP */
		dep = dwc2_ep0_get_in_ep(dwc);;
		req->dwc2_ep = dep;

		if (dwc->delayed_status_sent) {
			dwc2_gadget_giveback(outep0, req, 0);
			return 0;
		} else {
			del_timer(&dwc->delayed_status_watchdog);
		}

		if (unlikely(dwc->ep0state != EP0_STATUS_PHASE)) {
			dev_warn(dwc->dev, "%s: ep0state is not EP0_STATUS_PHASE when ep0_queue! ep0state = %d\n",
				__func__, dwc->ep0state);
			list_del(&req->list);
			return -EINVAL;
		}

		/*
		 * we are going to send a IN status pkt,
		 * status transfer is zero, it will finish very very quickly,
		 * and the host may sent SETUP packets juuuuust after previous STATUS phase,
		 * so prepare to receive next SETUP packet before we begin IN Status phase
		 */
		dwc2_ep0_out_start(dwc);
		dwc2_ep0_start_transfer(dwc, req);

		return 0;
	}

	if (dwc->three_stage_setup) {
		switch(dwc->ep0state) {
		case EP0_SETUP_PHASE:
			/* Assign Real EP */
			dep = dwc2_ep0_get_ep_by_dir(dwc, dwc->ep0_expect_in);
			req->dwc2_ep = dep;

			dwc->ep0state = EP0_DATA_PHASE;

			can_queue = 1;
			break;

		case EP0_DATA_PHASE:
			/* NOTE: we currently do not allow >1 data block */
			break;

		case EP0_STATUS_PHASE:
			break;
		case EP0_DISCONNECTED:
		case EP0_WAITING_SETUP_PHASE:
		default:
			break;
		}
	} else {
		switch(dwc->ep0state) {
		case EP0_SETUP_PHASE:
			if (req->request.zero || (req->request.length == 0)) {
				/* Assign Real EP */
				dep = dwc2_ep0_get_in_ep(dwc);;
				req->dwc2_ep = dep;

				dwc->ep0state = EP0_STATUS_PHASE;

				can_queue = 1;
			}
			break;

		case EP0_DATA_PHASE:
			break;
		case EP0_STATUS_PHASE:
			break;
		case EP0_DISCONNECTED:
		case EP0_WAITING_SETUP_PHASE:
		default:
			break;
		}
	}

	if (unlikely(!can_queue)) {
		list_del(&req->list);
		return -EINVAL;
	}

	dwc2_ep0_start_transfer(dwc, req);

	return 0;
}

void dwc2_delayed_status_watchdog(unsigned long _dwc) {
	struct dwc2 *dwc = (struct dwc2 *)_dwc;
	unsigned long flags;

	if (!dwc->delayed_status)
		return;

	dwc2_spin_lock_irqsave(dwc, flags);
	dev_dbg(dwc->dev, "enter %s\n", __func__);
	if (dwc->delayed_status) {
		dwc2_ep0_do_status_phase(dwc);
		dwc->delayed_status_sent = true;
	}
	dwc2_spin_unlock_irqrestore(dwc, flags);
}

int dwc2_gadget_ep0_queue(struct usb_ep *ep,
			struct usb_request *request,
			gfp_t gfp_flags) {
	struct dwc2_request	*req = to_dwc2_request(request);
	/* NOTE: see __dwc2_gadget_init_endpoints(), ep0 is actually OUT EP0 */
	struct dwc2_ep		*ep0 = to_dwc2_ep(ep);
	struct dwc2		*dwc = ep0->dwc;
	unsigned long		 flags;
	int			 ret;

	dwc2_spin_lock_irqsave(dwc, flags);
	if (!ep0->desc) {
		dev_dbg(dwc->dev, "trying to queue request %p to disabled %s\n",
			request, ep0->name);
		ret = -ESHUTDOWN;
		goto out;
	}

	dev_vdbg(dwc->dev, "queueing request %p to %s length %d, state '%s'\n",
		request, ep0->name, request->length,
		dwc2_ep0_state_string(dwc->ep0state));

	ret = __dwc2_gadget_ep0_queue(ep0, req);

out:
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return ret;
}

static void dwc2_ep0_stall_and_restart(struct dwc2 *dwc) {
	struct dwc2_ep		*dep;

	dev_info(dwc->dev, "%s\n", __func__);

	dep = dwc2_ep0_get_in_ep(dwc);
	 __dwc2_gadget_ep_set_halt(dep, 1);
	dep->flags = DWC2_EP_ENABLED;

	dep = dwc2_ep0_get_out_ep(dwc);
	 __dwc2_gadget_ep_set_halt(dep, 1);
	dep->flags = DWC2_EP_ENABLED;

	dwc->delayed_status = false;
	dwc->delayed_status_sent = true;
	del_timer(&dwc->delayed_status_watchdog);

	if (!list_empty(&dep->request_list)) {
		struct dwc2_request	*req;

		req = next_request(&dep->request_list);
		dwc2_gadget_giveback(dep, req, -ECONNRESET);
	}

	dwc->ep0state = EP0_WAITING_SETUP_PHASE;
	dwc2_ep0_out_start(dwc);
}

int dwc2_gadget_ep0_set_halt(struct usb_ep *ep, int value) {
	struct dwc2_ep	*dep = to_dwc2_ep(ep);
	struct dwc2	*dwc = dep->dwc;
	unsigned long	 flags;
	BUG_ON(dwc2_is_host_mode(dwc));

	dwc2_spin_lock_irqsave(dwc, flags);
	dwc2_ep0_stall_and_restart(dwc);
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return 0;
}

static struct dwc2_ep *dwc2_wIndex_to_dep(struct dwc2 *dwc, __le16 wIndex_le)
{
	struct dwc2_ep	*dep;
	u32		 windex = le16_to_cpu(wIndex_le);
	u32		 epnum;

	epnum = (windex & USB_ENDPOINT_NUMBER_MASK); // cli delete<< 1;
	if ((windex & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
		epnum += dwc->dev_if.num_out_eps;

	dep = dwc->eps[epnum];
	if (dep->flags & DWC2_EP_ENABLED)
		return dep;

	return NULL;
}

/*
 * ch 9.4.5
 */
static int dwc2_ep0_handle_status(struct dwc2 *dwc,
				struct usb_ctrlrequest *ctrl)
{
	struct dwc2_ep		*dep;
	u32			recip;
	u16			usb_status = 0;
	__le16			*response_pkt;


	recip = ctrl->bRequestType & USB_RECIP_MASK;
	switch (recip) {
	case USB_RECIP_DEVICE:
		if (dwc->dev_state == DWC2_DEFAULT_STATE)
			return -EINVAL;
		if (le16_to_cpu(ctrl->wIndex) == 0xF000) {
			/* TODO: handle OTG Status Selector here! */
		} else {
			usb_status |= dwc->is_selfpowered << USB_DEVICE_SELF_POWERED;
			usb_status |= dwc->remote_wakeup_enable << USB_DEVICE_REMOTE_WAKEUP;
		}

		break;

	case USB_RECIP_INTERFACE:
		if (dwc->dev_state != DWC2_CONFIGURED_STATE)
			return -EINVAL;
		/*
		 * Function Remote Wake Capable	D0
		 * Function Remote Wakeup	D1
		 */
		break;

	case USB_RECIP_ENDPOINT:
		if ((dwc->dev_state == DWC2_ADDRESS_STATE &&
				ctrl->wIndex != 0) ||
			dwc->dev_state == DWC2_DEFAULT_STATE)
			return -EINVAL;
		dep = dwc2_wIndex_to_dep(dwc, ctrl->wIndex);
		if (!dep)
			return -EINVAL;

		if (dep->flags & DWC2_EP_STALL)
			usb_status = 1 << USB_ENDPOINT_HALT;
		break;
	default:
		return -EINVAL;
	};

	response_pkt = (__le16 *) dwc->status_buf;
	*response_pkt = cpu_to_le16(usb_status);

	dep = dwc2_ep0_get_in_ep(dwc);
	dwc->ep0_usb_req.dwc2_ep = dep;
	dwc->ep0_usb_req.request.length = sizeof(*response_pkt);
	dwc->ep0_usb_req.request.buf = dwc->status_buf;
	dwc->ep0_usb_req.request.complete = dwc2_ep0_status_cmpl;
	dwc->ep0_usb_req.transfering = 0;

	return __dwc2_gadget_ep0_queue(dwc->eps[0], &dwc->ep0_usb_req);
}

/*
 * ch 9.4.1, 9.4.9
 */
static int dwc2_ep0_handle_feature(struct dwc2 *dwc,
				struct usb_ctrlrequest *ctrl, int set)
{
	struct dwc2_ep		*dep;
	u32			recip;
	u32			wValue;
	u32			wIndex;
	int			ret = 0;
	gotgctl_data_t gotgctl = {.d32 = 0 };
	dwc_otg_core_global_regs_t *global_regs = dwc->core_global_regs;

	wValue = le16_to_cpu(ctrl->wValue);
	wIndex = le16_to_cpu(ctrl->wIndex);
	recip = ctrl->bRequestType & USB_RECIP_MASK;
	switch (recip) {
	case USB_RECIP_DEVICE:

		switch (wValue) {
		case USB_DEVICE_REMOTE_WAKEUP:
			dwc->remote_wakeup_enable = set;
			break;

		case USB_DEVICE_TEST_MODE:
			if ((wIndex & 0xff) != 0)
				return -EINVAL;
			/* 9.4.1: The Test_Mode feature cannot be cleared by the ClearFeature() request. */
			if (!set)
				return -EINVAL;
			/*
			 * The transition to test mode of an upstream facing port must not
			 * happen until after the status stage of the request.
			 *
			 * NOTE: we do not bother to check if the Test Mode Selector is valid,
			 *       the host must take care it!!!
			 */
			dwc->test_mode_nr = wIndex >> 8;
			dwc->test_mode = true;
			break;

		case USB_DEVICE_B_HNP_ENABLE:
			if (set) {
				dev_info(dwc->dev, "Request B HNP\n");

				dwc->b_hnp_enable = 1;
				// TODO: dwc_otg_pcd_update_otg(dwc, 0);

				/**
				 * TODO: Is the gotgctl.devhnpen cleared by a USB Reset?
				 */
				gotgctl.b.devhnpen = 1;
				gotgctl.b.hnpreq = 1;
				dwc_writel(gotgctl.d32, &global_regs->gotgctl);
			} else
				return -EINVAL;

			break;

		case USB_DEVICE_A_HNP_SUPPORT:
			if (set) {
				dwc->a_hnp_support = 1;
				// TODO: dwc_otg_pcd_update_otg(dwc, 0);
			} else
				return -EINVAL;

			break;

		case USB_DEVICE_A_ALT_HNP_SUPPORT:
			if (set) {
				dwc->a_alt_hnp_support = 1;
				// dwc_otg_pcd_update_otg(dwc, 0);
			} else
				return -EINVAL;

			break;

		default:
			return -EINVAL;
		}
		break;

	case USB_RECIP_INTERFACE:
		/* USB2.0 do not support interface feature */
		return -EINVAL;
		break;

	case USB_RECIP_ENDPOINT:
		switch (wValue) {
		case USB_ENDPOINT_HALT:
			dep = dwc2_wIndex_to_dep(dwc, wIndex);
			if (!dep)
				return -EINVAL;
			ret = __dwc2_gadget_ep_set_halt(dep, set);
			if (ret)
				return -EINVAL;
			break;
		default:
			return -EINVAL;
		}

		break;

	default:
		return -EINVAL;
	};

	/* ok, response with status */
	dwc2_ep0_do_status_phase(dwc);

	return 0;
}

static int dwc2_ep0_std_request(struct dwc2 *dwc,
				struct usb_ctrlrequest *ctrl) {
	int ret;

	switch (ctrl->bRequest) {
	case USB_REQ_GET_STATUS:
		dev_vdbg(dwc->dev, "USB_REQ_GET_STATUS\n");
		ret = dwc2_ep0_handle_status(dwc, ctrl);
		break;
	case USB_REQ_CLEAR_FEATURE:
		dev_vdbg(dwc->dev, "USB_REQ_CLEAR_FEATURE\n");
		ret = dwc2_ep0_handle_feature(dwc, ctrl, 0);
		break;
	case USB_REQ_SET_FEATURE:
		dev_vdbg(dwc->dev, "USB_REQ_SET_FEATURE\n");
		ret = dwc2_ep0_handle_feature(dwc, ctrl, 1);
		break;
	case USB_REQ_SET_ADDRESS:
		dev_vdbg(dwc->dev, "USB_REQ_SET_ADDRESS\n");
		ret = dwc2_ep0_set_address(dwc, ctrl);
		break;
	case USB_REQ_SET_CONFIGURATION:
		dev_vdbg(dwc->dev, "USB_REQ_SET_CONFIGURATION\n");
		ret = dwc2_ep0_set_config(dwc, ctrl);
		break;
	default:
		dev_vdbg(dwc->dev, "Forwarding to gadget driver\n");
		ret = dwc2_ep0_delegate_req(dwc, ctrl);
		break;
	};

	return ret;
}

static void dwc2_ep0_inspect_setup(struct dwc2 *dwc)
{
	struct usb_ctrlrequest *ctrl = &dwc->ctrl_req;
	int ret = -EINVAL;
	u32 len;

	if (!dwc->gadget_driver)
		goto out;

	len = le16_to_cpu(ctrl->wLength);
	if (!len) {
		dwc->three_stage_setup = false;
		dwc->ep0_expect_in = false;
	} else {
		dwc->three_stage_setup = true;
		dwc->ep0_expect_in = !!(ctrl->bRequestType & USB_DIR_IN);
	}

	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD)
		ret = dwc2_ep0_std_request(dwc, ctrl);
	else
		ret = dwc2_ep0_delegate_req(dwc, ctrl);

	if (ret == USB_GADGET_DELAYED_STATUS) {
		dwc->ep0state = EP0_STATUS_PHASE;
		dwc->delayed_status = true;
		dwc->delayed_status_sent = false;
		/*
		 * Workaround:
		 * after 2~3s, we force to send a status stage,
		 * it seem that Windows XP will reset us instead of suspend.
		 */
		mod_timer(&dwc->delayed_status_watchdog, jiffies + 3 * HZ);
	}

out:
	if (ret < 0)
		dwc2_ep0_stall_and_restart(dwc);
}

static void dwc2_ep0_in_complete_data(struct dwc2 *dwc) {
	struct dwc2_ep		*ep0	     = dwc2_ep0_get_ep0(dwc);
	struct dwc2_dev_if	*dev_if	     = &dwc->dev_if;
	struct dwc2_request	*curr_req;
	deptsiz0_data_t		 deptsiz;
	int			 trans_count = 0;

	curr_req = next_request(&ep0->request_list);
	if (curr_req == NULL) {
		return;
	}

	if (dwc->dma_enable) {
		if (dwc->dma_desc_enable) {
			/* TODO: Scatter/Gather DMA Mode here! */
		} else {
			deptsiz.d32 = dwc_readl(&dev_if->in_ep_regs[0]->dieptsiz);

			/* The Programming Guide said xfercompl raised when xfersize and pktcnt gets zero */
			WARN(deptsiz.b.xfersize, "%s: xfersize(%u) not zero when xfercompl\n",
				__func__, deptsiz.b.xfersize);

			trans_count = curr_req->xfersize - deptsiz.b.xfersize;

			if (unlikely(trans_count < 0))
				trans_count = curr_req->xfersize;
		}

		curr_req->trans_count_left -= trans_count;
		if (curr_req->trans_count_left < 0)
			curr_req->trans_count_left = 0;
		curr_req->next_dma_addr += trans_count;

		if (curr_req->trans_count_left || need_send_zlp(curr_req)) {
			DWC2_EP0_DEBUG_MSG("req 0x%p continue transfer, is_in = %d\n",
					curr_req, curr_req->dwc2_ep->is_in);
			dwc2_ep0_start_transfer(dwc, curr_req);
		} else {
			DWC2_EP0_DEBUG_MSG("req 0x%p done, do %s status phase\n",
					curr_req, dwc->ep0_expect_in ? "OUT" : "IN");
			dwc2_ep0_do_status_phase(dwc);
		}
	} else {
		/* TODO: PIO Mode */
	}
}

static void dwc2_ep0_out_complete_data(struct dwc2 *dwc) {
	struct dwc2_ep		*ep0	     = dwc2_ep0_get_ep0(dwc);
	struct dwc2_dev_if	*dev_if	     = &dwc->dev_if;
	struct dwc2_request	*curr_req;
	deptsiz0_data_t		 deptsiz;
	int			 trans_count = 0;

	curr_req = next_request(&ep0->request_list);
	if (curr_req == NULL)
		return;

	if (dwc->dma_enable) {
		if (dwc->dma_desc_enable) {
			/* TODO: Scatter/Gather DMA Mode here! */
		} else {
			deptsiz.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doeptsiz);
			trans_count = curr_req->xfersize - deptsiz.b.xfersize;

			if (unlikely(trans_count < 0))
				trans_count = curr_req->xfersize;
		}

		curr_req->trans_count_left -= trans_count;
		if (curr_req->trans_count_left < 0) {
			curr_req->trans_count_left = 0;
		}

		/* copy from shadow buffer to real buffer */
		if (curr_req->request.length != 0) {
			int offset = curr_req->request.actual;
			u32 buf_addr = (u32)curr_req->request.buf;

			memcpy( (void *)(buf_addr + offset),
				(void *)dwc->ep0out_shadow_uncached,
				trans_count);
		}

		/* OUT request dma address are always dwc->ep0out_shadow_dma */
		// curr_req->next_dma_addr += trans_count;
		curr_req->request.actual += trans_count;

		/* if xfersize is not zero, we receive an short packet, the transfer is complete */
		if (!deptsiz.b.xfersize && (curr_req->trans_count_left || need_send_zlp(curr_req))) {
			DWC2_EP0_DEBUG_MSG("req 0x%p continue transfer, is_in = %d\n",
					curr_req, curr_req->dwc2_ep->is_in);
			dwc2_ep0_start_transfer(dwc, curr_req);
		} else {
			DWC2_EP0_DEBUG_MSG("req 0x%p done, do %s status phase\n",
					curr_req, dwc->ep0_expect_in ? "OUT" : "IN");
                        dwc->last_ep0out_normal = 1;
			dwc2_ep0_do_status_phase(dwc);
		}
	} else {
		/* TODO: PIOMode */
	}
}

static void dwc2_ep0_complete_data(struct dwc2 *dwc, int is_in) {

	if (is_in) {
		dwc2_ep0_in_complete_data(dwc);
	} else {
		dwc2_ep0_out_complete_data(dwc);
	}
}

static void dwc2_ep0_complete_status(struct dwc2 *dwc) {
	struct dwc2_ep		*ep0	     = dwc2_ep0_get_ep0(dwc);
	struct dwc2_request	*curr_req;
	dctl_data_t		 dctl;

	/* enter test mode if needed (exit by reset) */
	if (dwc->test_mode) {
		/*
		 * The transition to test mode must be complete no later than 3 ms
		 * after the completion of the status stage of the request.
		 */
		dctl.d32 = dwc_readl(&dwc->dev_if.dev_global_regs->dctl);
		dctl.b.tstctl = dwc->test_mode_nr;
		dwc_writel(dctl.d32, &dwc->dev_if.dev_global_regs->dctl);
	}

	/*
	 * prepare to receive SETUP again to ensure we are prepared!!!
	 */
	dwc2_ep0_out_start(dwc);

	curr_req = next_request(&ep0->request_list);
	if (curr_req) {
		dwc2_gadget_giveback(ep0, curr_req, 0);
	}

	if (dwc->test_mode)
		dev_info(dwc->dev, "entering Test Mode(%d)\n", dwc->test_mode_nr);

	dwc->ep0state = EP0_WAITING_SETUP_PHASE;
}

static void dwc2_ep0_xfer_complete(struct dwc2 *dwc, int is_in, int setup) {
	depctl_data_t	 depctl = {.d32 = 0 };

	if (is_in) {
		depctl.d32 = dwc_readl(&dwc->dev_if.in_ep_regs[0]->diepctl);
	} else {
		depctl.d32 = dwc_readl(&dwc->dev_if.out_ep_regs[0]->doepctl);
	}

	if (depctl.b.epena) {
		DWC2_EP0_DEBUG_MSG("ep0%s is remain enabled when xfercompl\n",
				is_in ? "IN" : "OUT");
	}

	dwc2_ep0_get_ep_by_dir(dwc, is_in)->flags &= ~DWC2_EP_BUSY;

	DWC2_EP0_DEBUG_MSG("xfercompl: is_in = %d, setup = %d, ep0state = %s \n",
			is_in, setup, dwc2_ep0_state_string(dwc->ep0state));

	if ( (setup && (dwc->ep0state != EP0_WAITING_SETUP_PHASE)) ||
		(!setup && !((dwc->ep0state == EP0_DATA_PHASE) || (dwc->ep0state == EP0_STATUS_PHASE)))
		) {
		DWC2_EP0_DEBUG_MSG("Error: %s: setup = %d, ep0state = %d\n", __func__, setup, dwc->ep0state);
		return;
	}

	switch (dwc->ep0state) {
	case EP0_WAITING_SETUP_PHASE:
		dwc->ep0state = EP0_SETUP_PHASE;
		dev_vdbg(dwc->dev, "Inspecting Setup Bytes\n");
		dwc2_ep0_inspect_setup(dwc);
		break;

	case EP0_DATA_PHASE:
		dev_vdbg(dwc->dev, "Data Phase\n");
		dwc2_ep0_complete_data(dwc, is_in);
		break;

	case EP0_STATUS_PHASE:
		dev_vdbg(dwc->dev, "Status Phase\n");
		dwc2_ep0_complete_status(dwc);
		break;

	default:
		WARN(true, "UNKNOWN ep0state %d\n", dwc->ep0state);
	}
}

#define CLEAR_IN_EP0_INTR(__intr)					\
	do {								\
		diepint_data_t __diepint = {.d32=0};			\
		__diepint.b.__intr = 1;					\
		dwc_writel(__diepint.d32, &dev_if->in_ep_regs[0]->diepint); \
	} while (0)

static void dwc2_ep0_handle_in_interrupt(struct dwc2_ep *dep) {
	struct dwc2		*dwc	= dep->dwc;
	struct dwc2_dev_if	*dev_if	= &dwc->dev_if;
	diepmsk_data_t		 diepmsk;
	diepint_data_t		 diepint;

	diepmsk.d32 = dwc_readl(&dev_if->dev_global_regs->diepmsk);
	diepint.d32 = dwc_readl(&dev_if->in_ep_regs[0]->diepint) & diepmsk.d32;

	/* Transfer complete */
	if (diepint.b.xfercompl) {
		dwc2_ep0_xfer_complete(dwc, 1, 0);

		CLEAR_IN_EP0_INTR(xfercompl);
		diepint.b.xfercompl = 0;
	}

	if (diepint.b.ahberr) {
		dev_err(dwc->dev, "EP0 IN AHB Error\n");

		CLEAR_IN_EP0_INTR(ahberr);
		diepint.b.ahberr = 0;
	}

	/*
	 * applies only to Control IN endpoints
	 *
	 * # In shared TX FIFO mode, applies to non-isochronous IN endpoints only.
	 * # In dedicated FIFO mode, applies only to Control IN endpoints.
	 * # In Scatter/Gather DMA mode, the TimeOUT interrupt is not asserted.
	 */
	if (diepint.b.timeout) {
		dev_err(dwc->dev, "EP0 IN Time-out\n");

		// TODO: handle_in_ep_timeout_intr(dep);

		CLEAR_IN_EP0_INTR(timeout);
		diepint.b.timeout = 0;
	}

	if (diepint.d32) {
		dev_err(dwc->dev, "unhandled ep0 IN interrupt(0x%08x)\n", diepint.d32);
	}
}
#undef CLEAR_IN_EP0_INTR

static int dwc2_ep0_get_ctrl_reqeust(struct dwc2 *dwc, deptsiz0_data_t *doeptsiz, int rem_supcnt,
				int back2back, u32 setup_addr) {
	struct usb_ctrlrequest	*ctrl_req;

	DWC2_EP0_DEBUG_MSG("SETUP: back2back=%d doepdma=0x%08x doeptsiz=0x%08x rem_supcnt=%d\n",
			back2back, setup_addr + 8, doeptsiz->d32, rem_supcnt);

	if (unlikely(dwc2_ep0_debug_en)) {
		int si = 0;
		for (si = 0; si < 3; si++) {
			struct usb_ctrlrequest *ctrl = dwc->ctrl_req_virt + si;
			DWC2_EP0_DEBUG_MSG("SETUP[%d] %02x.%02x v%04x i%04x l%04x\n",
					si, ctrl->bRequestType, ctrl->bRequest,
					le16_to_cpu(ctrl->wValue),
					le16_to_cpu(ctrl->wIndex),
					le16_to_cpu(ctrl->wLength));
		}
	}

	if (!back2back) {
		if (unlikely(rem_supcnt > 2)) {
			int setup_idx = (setup_addr - dwc->ctrl_req_addr) / 8;
			if ( (setup_idx < 0) || (setup_idx > 2) ) {
#if 0
				dwc2_ep0_dump_regs(dwc);
				panic("=========>rem_supcnt > 2 when back2back is 0\n");
#endif
				return -EINVAL;
			} else
				rem_supcnt = 2 - setup_idx;
		}

		ctrl_req = dwc->ctrl_req_virt + (2 - rem_supcnt);
	} else {
		int uncached_idx = (setup_addr - dwc->ctrl_req_addr) / 8;

		if (setup_addr > (dwc->ctrl_req_addr + PAGE_SIZE - 8)) {
			panic("Oops, memory leak!!! the kernel maybe broken up soon!\n");
		}

		ctrl_req = dwc->ctrl_req_virt + uncached_idx;
	}

	memcpy(&dwc->ctrl_req, ctrl_req, 8);

	DWC2_EP0_DEBUG_MSG("SETUP %02x.%02x v%04x i%04x l%04x\n",
			dwc->ctrl_req.bRequestType, dwc->ctrl_req.bRequest,
			le16_to_cpu(dwc->ctrl_req.wValue),
			le16_to_cpu(dwc->ctrl_req.wIndex),
			le16_to_cpu(dwc->ctrl_req.wLength));

	return 0;
}

#define CLEAR_OUT_EP0_INTR(__intr)					\
	do {								\
		doepint_data_t __doepint = {.d32=0};			\
		__doepint.b.__intr = 1;					\
		dwc_writel(__doepint.d32, &dev_if->out_ep_regs[0]->doepint); \
	} while (0)

static void dwc2_ep0_handle_out_interrupt(struct dwc2_ep *dep) {
	struct dwc2		*dwc	= dep->dwc;
	struct dwc2_dev_if	*dev_if	= &dwc->dev_if;
	doepmsk_data_t		 doepmsk;
	doepint_data_t		 doepint;

	doepmsk.d32 = dwc_readl(&dev_if->dev_global_regs->doepmsk);
	doepmsk.b.back2backsetup = 1;
	doepint.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doepint) & doepmsk.d32;

	DWC2_EP0_DEBUG_MSG("doepint = 0x%08x\n", doepint.d32);

	/* Transfer complete */
	if (doepint.b.xfercompl) {
		dwc2_ep0_xfer_complete(dwc, 0, 0);

                CLEAR_OUT_EP0_INTR(xfercompl);
                CLEAR_OUT_EP0_INTR(stsphsercvd);

		doepint.b.xfercompl = 0;
	}

	/* AHB Error */
	if (doepint.b.ahberr) {
		dev_err(dwc->dev, "EP0 OUT AHB Error\n");
		CLEAR_OUT_EP0_INTR(ahberr);

		doepint.b.ahberr = 0;
	}

	if (doepint.b.outtknepdis) {
		CLEAR_OUT_EP0_INTR(outtknepdis);
		doepint.b.outtknepdis = 0;
	}

	/* Setup Phase Done*/
	if (doepint.b.setup || doepint.b.back2backsetup) {
		deptsiz0_data_t	doeptsiz;
		int		rem_supcnt;
		int		back2back;
		u32		setup_addr;
		int		retval = 0;

		doeptsiz.d32 = dwc_readl(&dev_if->out_ep_regs[0]->doeptsiz);
		setup_addr = dwc_readl(&dev_if->out_ep_regs[0]->doepdma) - 8;
		rem_supcnt = doeptsiz.b.supcnt;
		back2back = doepint.b.back2backsetup;

		CLEAR_OUT_EP0_INTR(setup);
		if (back2back)
			CLEAR_OUT_EP0_INTR(back2backsetup);

		doepint.b.setup = 0;
		doepint.b.back2backsetup = 0;
		dwc->setup_prepared = 0;

		retval = dwc2_ep0_get_ctrl_reqeust(dwc, &doeptsiz, rem_supcnt, back2back, setup_addr);
		if (retval == 0) {
			// Warning: do not call dwc2_ep0_out_start(dwc) here!
			dwc2_ep0_xfer_complete(dwc, 0, 1);
		} else {
			dwc->do_reset_core = 1;
		}
	}

	if (doepint.d32) {
		dev_err(dwc->dev, "unhandled EP0 OUT interrupt(0x%08x)\n", doepint.d32);
	}
}
#undef CLEAR_OUT_EP0_INTR

void dwc2_ep0_interrupt(struct dwc2_ep *dep) {
	if (dep->is_in) {
		dwc2_ep0_handle_in_interrupt(dep);
	} else {
		dwc2_ep0_handle_out_interrupt(dep);
	}
}
