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
#include <asm/unaligned.h>

#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include "core.h"
#include "host.h"
#include "debug.h"

#include "dwc2_jz.h"

/**
 * @dump_dir: bit0 --- OUT; bit1 --- IN
 */
static __attribute__((unused)) void dwc2_dump_urb(struct urb *urb, int dump_dir, int detail,
			const char *func, int line) {
	struct usb_host_endpoint *hep = urb->ep;
	int is_in = usb_pipein(urb->pipe);
	int dump_data = 0;

	if (detail) {
		const char *type_str;

		switch(usb_endpoint_type(&hep->desc)) {
		case USB_ENDPOINT_XFER_CONTROL:
			type_str = "ctrl";
			break;
		case USB_ENDPOINT_XFER_BULK:
			type_str = "bulk";
			break;
		case USB_ENDPOINT_XFER_INT:
			type_str = "int";
			break;
		case USB_ENDPOINT_XFER_ISOC:
			type_str = "isoc";
			break;
		default:
			type_str = "unkn";
		}

		printk("%s:%d %s urb=%p ep%d%s type=%s ep=%u dev=%u spd=%u interval=%d mps=%u\n",
			func, line, urb->dev->devpath, urb,
			usb_pipeendpoint (urb->pipe),
			is_in ? "in" : "out",
			type_str, usb_pipeendpoint(urb->pipe),
			usb_pipedevice(urb->pipe), urb->dev->speed, urb->interval,
			usb_maxpacket(urb->dev, urb->pipe, !is_in));
		printk("    dma=0x%08x len=%u flags=0x%08x sdma=0x%08x npkts=%d actual=%d status=%d\n",
			urb->transfer_dma, urb->transfer_buffer_length, urb->transfer_flags,
			urb->setup_dma, urb->number_of_packets, urb->actual_length, urb->status);

		if (usb_endpoint_type(&hep->desc) == USB_ENDPOINT_XFER_CONTROL) {
			struct usb_ctrlrequest	*ctrl_req = (struct usb_ctrlrequest *)(urb->setup_dma | 0xa0000000);
			printk("SETUP %02x.%02x v%04x i%04x l%04x\n",
				ctrl_req->bRequestType, ctrl_req->bRequest,
				le16_to_cpu(ctrl_req->wValue),
				le16_to_cpu(ctrl_req->wIndex),
				le16_to_cpu(ctrl_req->wLength));
		}
	} else {
		printk("%s:%d %s urb=%p status=%d len=%d actual=%d\n",
			func, line, urb->dev->devpath, urb,
			urb->status, urb->transfer_buffer_length,
			urb->actual_length);
	}

	if ( (dump_dir & 0x1) /* dump OUT */
		&& !is_in)
		dump_data = 1;

	if ( (dump_dir & 0x2) /* dump IN */
		&& is_in)
		dump_data = 1;

	if (dump_data) {
		/* use uncache address, so we will not confuse DMA */
		u8 *data = (u8 *)(urb->transfer_dma | 0xa0000000);

		/*
		 * multiple printk maybe split by other system interrupts, which
		 * will make the print mussy, so if the length is greater than 8,
		 * use one printk instead of a loop
		 */
		if (likely(urb->transfer_buffer_length > 8)) {
			printk("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
				data[0], data[1], data[2], data[3],
				data[4], data[5], data[6], data[7]);
		} else {
			int i;

			printk("    ");
			for (i = 0; i < urb->transfer_buffer_length; i++) {
				printk("%02x ", data[i]);
			}
		}
	}
}

static __attribute__((unused)) void dwc2_dump_host_regs(
	int chan_start, int chan_end, const char *func, int line) {

	u32 r0x004, r0x014, r0x018;
	u32 r0x400, r0x404, r0x408, r0x40c;
	u32 r0x410, r0x414, r0x418, r0x41c;
	u32 r0x440;
	u32 hcchar[MAX_EPS_CHANNELS];
	u32 hcsplt[MAX_EPS_CHANNELS];
	u32 hcint[MAX_EPS_CHANNELS];
	u32 hcintmsk[MAX_EPS_CHANNELS];
	u32 hctsiz[MAX_EPS_CHANNELS];
	u32 hcdma[MAX_EPS_CHANNELS];
	int chan_num = 0;

	/* read */
	DWC_RR(0x004);
	DWC_RR(0x014);
	DWC_RR(0x018);
	DWC_RR(0x400);
	DWC_RR(0x404);
	DWC_RR(0x408);
	DWC_RR(0x40c);
	DWC_RR(0x410);
	DWC_RR(0x414);
	DWC_RR(0x418);
	DWC_RR(0x41c);
	DWC_RR(0x440);

	for (chan_num = chan_start; chan_num <= chan_end; chan_num++) {
		DWC_RR_HCCHAR(hcchar[chan_num], chan_num);
		DWC_RR_HCSPLT(hcsplt[chan_num], chan_num);
		DWC_RR_HCINT(hcint[chan_num], chan_num);
		DWC_RR_HCINTMSK(hcintmsk[chan_num], chan_num);
		DWC_RR_HCTSIZ(hctsiz[chan_num], chan_num);
		DWC_RR_HCDMA(hcdma[chan_num], chan_num);
	}

	/* then print */
	printk("======%s:%d channel%d=======\n", func, line, chan_num);
	DWC_P(0x004);
	DWC_P(0x014);
	DWC_P(0x018);
	DWC_P(0x400);
	DWC_P(0x404);
	DWC_P(0x408);
	DWC_P(0x40c);
	DWC_P(0x410);
	DWC_P(0x414);
	DWC_P(0x418);
	DWC_P(0x41c);
	DWC_P(0x440);

	for (chan_num = chan_start; chan_num <= chan_end; chan_num++) {
		printk("---------------------------\n");
		DWC_P_HCCHAR(hcchar[chan_num], chan_num);
		DWC_P_HCSPLT(hcsplt[chan_num], chan_num);
		DWC_P_HCINT(hcint[chan_num], chan_num);
		DWC_P_HCINTMSK(hcintmsk[chan_num], chan_num);
		DWC_P_HCTSIZ(hctsiz[chan_num], chan_num);
		DWC_P_HCDMA(hcdma[chan_num], chan_num);
	}
}

static inline  __attribute__((always_inline))
unsigned short dwc2_hc_get_frame_number(struct dwc2 *dwc) {
	hfnum_data_t	hfnum;
	uint16_t	mod = (DWC2_MAX_MICROFRAME - 1);

	if (dwc->mode == DWC2_HC_UHCI_MODE)
		mod = DWC2_MAX_FRAME - 1;

	hfnum.d32 = dwc_readl(&dwc->host_if.host_global_regs->hfnum);

	return hfnum.b.frnum & mod;
}

/**
 * Returns true if _frame1 is less than or equal to _frame2. The comparison is
 * done modulo DWC_HFNUM_MAX_FRNUM. This accounts for the rollover of the
 * frame number when the max frame number is reached.
 *
 * about 1s torlerance (MAX_FRNUM / 2)
 */
static inline int __dwc_frame_num_le(uint16_t frame1, uint16_t frame2, uint16_t mod) {
	return ((frame2 - frame1) & mod) <= (mod >> 1);
}

static inline int dwc_frame_num_le(struct dwc2 *dwc, uint16_t frame1, uint16_t frame2)
{
	uint16_t mod = (DWC2_MAX_MICROFRAME - 1);

	if (dwc->mode == DWC2_HC_UHCI_MODE)
		mod = DWC2_MAX_FRAME - 1;

	return __dwc_frame_num_le(frame1, frame2, mod);
}


static inline int dwc_frame_num_lt(struct dwc2 *dwc, uint16_t frame1, uint16_t frame2)
{
	return (frame1 != frame2) &&
		dwc_frame_num_le(dwc, frame1, frame2);
}

/**
 * Returns true if _frame1 is greater than _frame2. The comparison is done
 * modulo DWC_HFNUM_MAX_FRNUM. This accounts for the rollover of the frame
 * number when the max frame number is reached.
 */
static inline int dwc_frame_num_gt(struct dwc2* dwc, uint16_t frame1, uint16_t frame2)
{
	uint16_t mod = (DWC2_MAX_MICROFRAME - 1);

	if (dwc->mode == DWC2_HC_UHCI_MODE)
		mod = DWC2_MAX_FRAME - 1;

	return (frame1 != frame2) &&
		(((frame1 - frame2) & mod) < (mod >> 1));
}

/**
 * Increments _frame by the amount specified by _inc. The addition is done
 * modulo DWC_HFNUM_MAX_FRNUM. Returns the incremented value.
 */
static inline uint16_t __dwc_frame_num_inc(uint16_t frame, uint16_t inc, uint16_t mod) {
	return (frame + inc) & mod;
}

static inline uint16_t dwc_frame_num_inc(struct dwc2 *dwc, uint16_t frame, uint16_t inc)
{
	uint16_t mod = (DWC2_MAX_MICROFRAME - 1);

	if (dwc->mode == DWC2_HC_UHCI_MODE)
		mod = DWC2_MAX_FRAME - 1;

	return __dwc_frame_num_inc(frame, inc, mod);
}

static inline uint16_t dwc_full_frame_num(struct dwc2 *dwc, uint16_t frame)
{
	if (dwc->mode == DWC2_HC_UHCI_MODE)
		return frame;
	else
		return frame >> 3;
}

static inline uint16_t dwc_micro_frame_num(struct dwc2 *dwc, uint16_t frame) {
	if (dwc->mode == DWC2_HC_EHCI_MODE) {
		return frame & 0x7;
	} else
		return 0;
}

void dwc2_disable_host_interrupts(struct dwc2 *dwc)
{
	dwc_otg_core_global_regs_t *global_regs = dwc->core_global_regs;
	gintmsk_data_t intr_mask = {.d32 = 0 };

	/*
	 * Disable host mode interrupts without disturbing common
	 * interrupts.
	 */
	intr_mask.d32 = dwc_readl(&global_regs->gintmsk);
	intr_mask.b.sofintr = 0;
	intr_mask.b.portintr = 0;
	intr_mask.b.hcintr = 0;
	dwc_writel(intr_mask.d32, &global_regs->gintmsk);
}

void dwc2_enable_host_interrupts(struct dwc2 *dwc)
{
	dwc_otg_core_global_regs_t *global_regs = dwc->core_global_regs;
	gintmsk_data_t intr_mask = { .d32 = 0 };

	dwc_writel(0, &dwc->host_if.host_global_regs->haintmsk);

	/* Disable all interrupts. */
	dwc_writel(0, &global_regs->gintmsk);

	/* Clear any pending interrupts. */
	dwc_writel(0xFFFFFFFF, &global_regs->gintsts);

	/* Enable the common interrupts */
	dwc2_enable_common_interrupts(dwc);

	intr_mask.d32 = dwc_readl(&global_regs->gintmsk);
	intr_mask.b.disconnect = 1;
	intr_mask.b.portintr = 1;
	intr_mask.b.hcintr = 1;
	dwc_writel(intr_mask.d32, &global_regs->gintmsk);
}

void dwc2_host_mode_init(struct dwc2 *dwc) {
	dwc_otg_core_global_regs_t	*global_regs = dwc->core_global_regs;
	struct dwc2_host_if		*host_if     = &dwc->host_if;
	hprt0_data_t			 hprt0	     = {.d32 = 0 };
	hcchar_data_t			 hcchar;
	hcfg_data_t			 hcfg;
	dwc_otg_hc_regs_t		*hc_regs;
	gotgctl_data_t			 gotgctl     = {.d32 = 0 };
	int				 i;

	/* Restart the Phy Clock */
	dwc_writel(0, dwc->pcgcctl);

	/* High speed PHY running at full speed or high speed */
	hcfg.d32 = dwc_readl(&host_if->host_global_regs->hcfg);
	hcfg.b.fslspclksel = DWC_HCFG_30_60_MHZ;
#ifdef CONFIG_USB_DWC2_FULLSPEED_HOST
	hcfg.b.fslssupp = 1;
#else
	hcfg.b.fslssupp = 0;
#endif
	hcfg.b.descdma = 1;
	hcfg.b.perschedena = 1;
	hcfg.b.frlisten = 3;  /* 64 entries */
	dwc_writel(hcfg.d32, &host_if->host_global_regs->hcfg);

	dwc_writel(dwc->hw_frame_list, &dwc->host_if.host_global_regs->hflbaddr);

	calculate_fifo_size(dwc);

	gotgctl.d32 = dwc_readl(&global_regs->gotgctl);
	gotgctl.b.hstsethnpen = 0;
	dwc_writel(gotgctl.d32, &global_regs->gotgctl);

	/* Halt all channels to put them into a known state. */
	for (i = 0; i < MAX_EPS_CHANNELS; i++) {
		int			 count = 0;
		struct dwc2_channel	*chan;

		hc_regs = host_if->hc_regs[i];
		hcchar.d32 = dwc_readl(&hc_regs->hcchar);
		hcchar.b.chen = 1;
		hcchar.b.chdis = 1;
		hcchar.b.epdir = 0;
		dwc_writel(hcchar.d32, &hc_regs->hcchar);
		do {
			hcchar.d32 = dwc_readl(&hc_regs->hcchar);
			if (++count > 1000) {
				dev_err(dwc->dev, "%s: Unable to clear halt on channel %d\n",
					__func__, i);
				break;
			}
			udelay(1);
		} while (hcchar.b.chen);

		chan = dwc->channel_pool + i;
		dwc_writel(chan->hw_desc_list, &hc_regs->hcdma);
	}

	{
		struct dwc2_channel *chan;
		struct dwc2_channel *chan_n;

		/* the core is re-inited, so isoc channel can set to normal channel */

		list_for_each_entry_safe(chan, chan_n, &dwc->idle_isoc_chan_list, list) {
			list_del(&chan->list); /* del from idle_isoc_chan_list */
			list_add(&chan->list, &dwc->chan_free_list); /* add to common chan_free_list */
		}
	}

	/* Turn on the vbus power. */
	if (dwc->op_state == DWC2_A_HOST) {
		hprt0.d32 = dwc2_hc_read_hprt(dwc);
		if (hprt0.b.prtpwr == 0) {
			hprt0.b.prtpwr = 1;
			dwc_writel(hprt0.d32, host_if->hprt0);
		}
	}

	dwc2_enable_host_interrupts(dwc);
}

static int dwc2_channel_pool_init(struct dwc2 *dwc) {
	int			 i = 0;
	struct dwc2_channel	*ch;
	size_t size;

	size = MAX_DMA_DESC_NUM_HS_ISOC * sizeof(dwc_otg_host_dma_desc_t) * MAX_EPS_CHANNELS;
	dwc->sw_chan_desc_list = dma_alloc_coherent(dwc->dev, size,
						&dwc->hw_chan_desc_list, GFP_KERNEL);
	memset(dwc->sw_chan_desc_list, 0, size);
	if (!dwc->sw_chan_desc_list) {
		dev_err(dwc->dev, "failed to allocate host DDMA channel descriptor list\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&dwc->chan_free_list);
	INIT_LIST_HEAD(&dwc->chan_trans_list);
	INIT_LIST_HEAD(&dwc->idle_isoc_chan_list);
	INIT_LIST_HEAD(&dwc->busy_isoc_chan_list);

	for (i = 0; i < MAX_EPS_CHANNELS; i++) {
		ch = dwc->channel_pool + i;

		ch->number = i;
		ch->urb_priv = NULL;
		ch->sw_desc_list = dwc->sw_chan_desc_list + i * MAX_DMA_DESC_NUM_HS_ISOC;
		ch->hw_desc_list = dwc->hw_chan_desc_list +
			i * MAX_DMA_DESC_NUM_HS_ISOC * sizeof(dwc_otg_host_dma_desc_t);
		ch->remain_slots = MAX_DMA_DESC_NUM_HS_ISOC;

		init_waitqueue_head(&ch->disable_wq);
		ch->disable_stage = 0;

		list_add_tail(&ch->list, &dwc->chan_free_list);
	}

	return 0;
}

static void dwc2_channel_pool_fini(struct dwc2 *dwc) {
	size_t size;

	size = MAX_DMA_DESC_NUM_HS_ISOC * sizeof(dwc_otg_host_dma_desc_t) * MAX_EPS_CHANNELS;
	dma_free_coherent(dwc->dev, size,
			dwc->sw_chan_desc_list, dwc->hw_chan_desc_list);
}

static __attribute__((unused)) void dwc2_channel_clean_up(struct dwc2_channel *chan) {
	memset(chan->sw_desc_list, 0,
		MAX_DMA_DESC_NUM_HS_ISOC * sizeof(dwc_otg_host_dma_desc_t));
}

/* Caller must take care of lock and irq */
static struct dwc2_channel* dwc2_request_channel(
	struct dwc2 *dwc, struct dwc2_urb_priv *urb_priv) {
	struct dwc2_channel *chan;
	struct dwc2_qh *qh = urb_priv->owner_qh;

	if (list_empty(&dwc->chan_free_list)) {
		return NULL;
	}

	chan = list_first_entry(&dwc->chan_free_list, struct dwc2_channel, list);
	list_del(&chan->list); /* del from free_list */
	list_add_tail(&chan->list, &dwc->chan_trans_list);

	chan->urb_priv = urb_priv;
	urb_priv->channel = chan;

	if (qh->trans_td_count == 0) {
		list_del(&qh->list); /* del from idle list */
		list_add(&qh->list, &dwc->busy_qh_list);
	}
	qh->trans_td_count ++;

	return chan;
}

static struct dwc2_channel *dwc2_request_isoc_chan(struct dwc2 *dwc, struct dwc2_qh *qh) {
	struct dwc2_channel *chan = NULL;

	if (!list_empty(&dwc->idle_isoc_chan_list))
		chan = list_first_entry(&dwc->idle_isoc_chan_list, struct dwc2_channel, list);

	if (!chan) {
		if (!list_empty(&dwc->chan_free_list))
			chan = list_first_entry(&dwc->chan_free_list, struct dwc2_channel, list);
	}

	if (!chan)
		return NULL;

	list_del(&chan->list); /* del from chan_free_list or idle_isoc_chan_list */
	list_add(&chan->list, &dwc->busy_isoc_chan_list);

	chan->qh = qh;
	qh->channel = chan;

	list_del(&qh->list); /* del from idle list */
	list_add(&qh->list, &dwc->busy_qh_list);
	qh->trans_td_count++;

	return chan;
}

static void dwc2_dispatch_channel(struct dwc2 *dwc, struct dwc2_channel *chan, struct dwc2_qh *qh) {
	int			 i, bfrm;
	unsigned short		 frame;
	hctsiz_data_t		 hctsiz;
	dwc_otg_hc_regs_t	*hc_regs = dwc->host_if.hc_regs[chan->number];
	unsigned short		 start = qh->start_frame;
	unsigned short		 period	 = qh->period;

	if (qh->type == USB_ENDPOINT_XFER_ISOC) {
		if (dwc->mode == DWC2_HC_EHCI_MODE) {
			/* microframe to frame */
			start = start >> 3;
			period = period >> 3;
		}
	}

	if (period == 0)
		period = 1;

	for (i = 0, frame = start;
	     i < 64;
	     frame = __dwc_frame_num_inc(frame, period, DWC2_MAX_FRAME-1), i++) {
		bfrm = frame % MAX_FRLIST_EN_NUM;

		set_bit(bfrm, chan->frame_inuse);
		dwc->sw_frame_list[bfrm] |= (1 << chan->number);
	}

	hctsiz.d32 = dwc_readl(&hc_regs->hctsiz);
	if (dwc->mode == DWC2_HC_EHCI_MODE)
		hctsiz.b_ddma.schinfo = qh->smask;
	else
		hctsiz.b_ddma.schinfo = 0xFF;
	dwc_writel(hctsiz.d32, &hc_regs->hctsiz);
}

static void dwc2_channel_free_dispatch(struct dwc2 *dwc, struct dwc2_channel *chan) {
	int bfrm;

	for_each_set_bit(bfrm, chan->frame_inuse, MAX_FRLIST_EN_NUM) {
		dwc->sw_frame_list[bfrm] &= ~(1 << chan->number);
	}

	memset(chan->frame_inuse, 0, sizeof(chan->frame_inuse));
}

static void __dwc2_free_channel(struct dwc2 *dwc, struct dwc2_channel *chan) {
	haintmsk_data_t haintmsk = { .d32 = 0 };

	dwc2_channel_free_dispatch(dwc, chan);

	chan->disable_stage = 0;
	list_del(&chan->list); /* del from chan_trans_list */
	list_add_tail(&chan->list, &dwc->chan_free_list);

	haintmsk.d32 = dwc_readl(&dwc->host_if.host_global_regs->haintmsk);
	haintmsk.d32 &= ~(1 << chan->number);
	dwc_writel(haintmsk.d32, &dwc->host_if.host_global_regs->haintmsk);
}

/* called from qh destroy */
static void dwc2_free_channel_simple(struct dwc2 *dwc, struct dwc2_channel *chan) {
	if (chan->urb_priv) {
		chan->urb_priv->channel = NULL;
		chan->urb_priv = NULL;
	}

	__dwc2_free_channel(dwc, chan);
}

/* Caller must take care of lock and irq */
static void dwc2_free_channel(struct dwc2 *dwc, struct dwc2_channel *chan) {
	if (chan->urb_priv) {
		struct dwc2_qh *qh = chan->urb_priv->owner_qh;

		chan->urb_priv->channel = NULL;
		qh->trans_td_count--;
		if (qh->trans_td_count == 0) {
			list_del(&qh->list); /* del from busy list */
			list_add_tail(&qh->list, &dwc->idle_qh_list);
		}
		chan->urb_priv = NULL;
	}

	__dwc2_free_channel(dwc, chan);
}

static void dwc2_free_isoc_chan(struct dwc2 *dwc, struct dwc2_channel *chan) {
	haintmsk_data_t haintmsk = { .d32 = 0 };

	if (chan->qh) {
		struct dwc2_qh * qh = chan->qh;

		panic("dwc2-host-ddma:%d: this code is just reserved for debug, will never executed\n", __LINE__);

		qh->channel = NULL;
		list_del(&qh->list); /* del from busy list */
		list_add_tail(&qh->list, &dwc->idle_qh_list);
		qh->trans_td_count = 0;
	}

	dwc2_channel_free_dispatch(dwc, chan);

	list_del(&chan->list); /* del from busy_isoc_chan_list */
	list_add_tail(&chan->list, &dwc->idle_isoc_chan_list);

	haintmsk.d32 = dwc_readl(&dwc->host_if.host_global_regs->haintmsk);
	haintmsk.d32 &= ~(1 << chan->number);
	dwc_writel(haintmsk.d32, &dwc->host_if.host_global_regs->haintmsk);
}

static void dwc2_hcd_stop(struct usb_hcd *hcd) {
	struct dwc2 *dwc = hcd_to_dwc2(hcd);
	hprt0_data_t hprt0 = {.d32 = 0 };

	/*
	 * The root hub should be disconnected before this function is called.
	 * The disconnect will clear the QTD lists (via urb_dequeue)
	 * and the QH lists (via endpoint_disable).
	 */

	/* Turn off all host-specific interrupts. */
	dwc2_disable_host_interrupts(dwc);

	/* Turn off the vbus power */
	hprt0.b.prtpwr = 0;
	dwc_writel(hprt0.d32, dwc->host_if.hprt0);
	mdelay(1);

	jz_set_vbus(dwc, 0);
}

static int dwc2_hcd_get_frame_number(struct usb_hcd *hcd) {
	struct dwc2 *dwc = hcd_to_dwc2(hcd);

	return dwc2_hc_get_frame_number(dwc);
}

static struct dwc2_urb_context* dwc2_hcd_find_urb_context(struct dwc2 *dwc, void *context) {
	struct dwc2_urb_context *ctx_list;

	list_for_each_entry(ctx_list, &dwc->context_list, list) {
		if (ctx_list->context == context) {
			return ctx_list;
		}
	}

	return NULL;
}

static struct dwc2_urb_context* dwc2_hcd_add_urb_context(struct dwc2 *dwc, void *context) {
	struct dwc2_urb_context *ctx_list;

	ctx_list = dwc2_hcd_find_urb_context(dwc, context);
	if (ctx_list)
		return ctx_list;

	ctx_list = kmem_cache_alloc(dwc->context_cachep, GFP_ATOMIC);
	if (ctx_list) {
		ctx_list->context = context;
		INIT_LIST_HEAD(&ctx_list->urb_list);
		list_add_tail(&ctx_list->list, &dwc->context_list);
	}

	return ctx_list;
}

static void dwc2_giveback_urb(struct dwc2 *dwc, struct urb *urb, int status) {
	urb->hcpriv = NULL;
	//dwc2_trace_clear_urb_hcpriv(urb);
	usb_hcd_unlink_urb_from_ep(dwc->hcd, urb);
	dwc->urb_queued_number--;
	dwc2_spin_unlock(dwc);
	usb_hcd_giveback_urb(dwc->hcd, urb, status);
	dwc2_spin_lock(dwc);
}

static void dwc2_giveback_context_list(struct dwc2 *dwc, void *context) {
	struct dwc2_urb_context *ctx_list;
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_urb_priv	*urb_priv_n;
	struct urb		*urb;

	ctx_list = dwc2_hcd_find_urb_context(dwc, context);

	/* giveback NO_INTERRUPT urbs */
	if (unlikely(ctx_list)) {
		list_for_each_entry_safe(urb_priv, urb_priv_n, &ctx_list->urb_list, list) {
			urb = urb_priv->urb;
			list_del(&urb_priv->list);
			kmem_cache_free(dwc->context_cachep, urb_priv);
			dwc2_giveback_urb(dwc, urb, 0);
		}
		list_del(&ctx_list->list);
		kmem_cache_free(dwc->context_cachep, ctx_list);
	}
}

static void dwc2_schedule(struct dwc2 *dwc);
void dwc2_urb_done(struct dwc2 *dwc, struct urb *urb, int status) {
	struct dwc2_urb_priv *urb_priv = urb->hcpriv;
	struct dwc2_qh *qh = urb->ep->hcpriv;
	struct dwc2_td *td;
	struct dwc2_td *td_n;

	dwc2_trace_urb_done(urb, status);
	list_del(&urb_priv->list);

	list_for_each_entry_safe(td, td_n, &urb_priv->done_td_list, list) {
		list_del(&td->list);
		kmem_cache_free(dwc->td_cachep, td);
	}

	list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
		list_del(&td->list);
		kmem_cache_free(dwc->td_cachep, td);
	}

	if (qh->type == USB_ENDPOINT_XFER_INT) {
		dwc->hcd->self.bandwidth_int_reqs--;
	}

	if (qh->type == USB_ENDPOINT_XFER_ISOC) {
		dwc->hcd->self.bandwidth_isoc_reqs--;
		qh->channel->remain_slots += urb_priv->isoc_slots;
	}

	if (status || !(urb->transfer_flags & URB_NO_INTERRUPT)) {
		unsigned int transfer_flags = urb->transfer_flags;
		if (transfer_flags & URB_NO_INTERRUPT) {
			printk("===>pause schedule for qh %p\n", qh);
			qh->pause_schedule = 1;
		} else {

		}
		/* give back any successfully transfered urbs of this context if any */
		dwc2_giveback_context_list(dwc, urb->context);

		/* giveback this urb */
		kmem_cache_free(dwc->context_cachep, urb_priv);
		dwc2_giveback_urb(dwc, urb, status);

		if (!(transfer_flags & URB_NO_INTERRUPT)) {
			if (qh->pause_schedule && !qh->disabling) {
				qh->pause_schedule = 0;
				printk("===>resume schedule for qh %p\n", qh);
				dwc2_schedule(dwc);
			}
		}
	} else {
		struct dwc2_urb_context *ctx_list;

		ctx_list = dwc2_hcd_find_urb_context(dwc, urb->context);
		BUG_ON(!ctx_list);

		list_add_tail(&urb_priv->list, &ctx_list->urb_list);
	}
}

static void dwc2_urb_priv_reset(struct dwc2_urb_priv* urb_priv,
				struct dwc2_channel *chan);
static void dwc2_qh_destroy(struct dwc2 *dwc, struct dwc2_qh *qh) {
	struct dwc2_urb_priv *urb_priv;

	dwc2_trace_del_qh(qh);

	/* remove qh from QH list */
	switch (qh->type) {
	case USB_ENDPOINT_XFER_CONTROL:
	case USB_ENDPOINT_XFER_BULK:
		break;
	case USB_ENDPOINT_XFER_INT:
		{
			unsigned short		 frame;
			struct dwc2_frame	*dwc_frm;

			for (frame = qh->start_frame;
			     frame < MAX_FRLIST_EN_NUM;
			     frame += qh->period) {
				struct dwc2_qh *prev_qh;
				struct dwc2_qh *pos;

				dwc_frm = dwc->frame_list + frame;

#if 0
				printk("frame[%d] ", frame);
				for (pos = dwc_frm->int_qh_head;
				     pos != NULL;
				     pos = pos->next) {
					printk(" qh%p ---> ", pos);
				}
				printk("\n");
#endif


				if (qh == dwc_frm->int_qh_head) {
					dwc_frm->int_qh_head = qh->next;
				} else if (dwc_frm->int_qh_head) {
					prev_qh = dwc_frm->int_qh_head;

					for (pos = prev_qh->next;
					     pos != NULL;
					     pos = pos->next) {
						if (pos == qh) {
							break;
						} else
							prev_qh = pos;
					}

					if (pos)
						prev_qh->next = qh->next;
				}
			}
		}
		break;
	case USB_ENDPOINT_XFER_ISOC:
		{
			struct dwc2_isoc_qh_ptr *qh_ptr;
			struct dwc2_isoc_qh_ptr *qh_ptr_n;

			list_for_each_entry_safe(qh_ptr, qh_ptr_n, &qh->isoc_qh_ptr_list, qh_list) {
				list_del(&qh_ptr->qh_list);  /* del from dwc2_qh->isoc_qh_ptr_list */
				list_del(&qh_ptr->frm_list);    /* del from dwc2_frame->isoc_qh_list */
				list_add(&qh_ptr->frm_list, &dwc->isoc_qh_ptr_list);
				/* do not call kmem_cache_free */
			}
		}
		break;
	default:
		;
	}

	/* del from dwc->idle/busy_qh_list */
	list_del(&qh->list);

	/* give back bandwidth */
	if ( ((qh->type == USB_ENDPOINT_XFER_INT) ||
			(qh->type == USB_ENDPOINT_XFER_ISOC)) &&
		qh->bandwidth_reserved) {
		int bd;

		/* NOTE: bandwidth_allocated units: microseconds/frame */
		if (dwc->mode == DWC2_HC_EHCI_MODE) {
			bd = qh->usecs * 8;
		} else {
			bd = qh->usecs;
		}

		if (qh->period)
			bd /= qh->period;

		dwc->hcd->self.bandwidth_allocated -= bd;
	}

	while (!list_empty(&qh->urb_list)) {
		/*
		 * note: do not use list_for_each_entry_safe here,
		 * dwc2_urb_done may re-enter into urb_dequeue
		 */
		struct dwc2_channel *chan = NULL;

		urb_priv = list_first_entry(&qh->urb_list, struct dwc2_urb_priv, list);
		if (urb_priv->channel) {
			chan = urb_priv->channel;
			dwc2_free_channel_simple(dwc, chan);
		}

		if (qh->channel)
			chan = qh->channel;

		if (chan)
			dwc2_urb_priv_reset(urb_priv, chan);
		dwc2_urb_done(dwc, urb_priv->urb, -ESHUTDOWN);
	}

	if (qh->channel) {
		qh->channel->qh = NULL;
		dwc2_free_isoc_chan(dwc, qh->channel);
	}

	qh->host_ep->hcpriv = NULL;
	kmem_cache_free(dwc->qh_cachep, qh);
}

static void dwc2_host_cleanup(struct dwc2 *dwc);

static struct dwc2_qh* dwc2_qh_make(struct dwc2 *dwc, struct urb *urb) {
	struct dwc2_qh			*qh;
	struct usb_host_endpoint	*hep = urb->ep;
	int				 mode;

	qh = kmem_cache_zalloc(dwc->qh_cachep, GFP_ATOMIC);
	if (!qh)
		return qh;

	qh->dwc = dwc;
	qh->host_ep = hep;
	hep->hcpriv = qh;
	qh->dev = urb->dev;

	qh->next = NULL;
	INIT_LIST_HEAD(&qh->urb_list);
	INIT_LIST_HEAD(&qh->isoc_qh_ptr_list);

	qh->type = usb_endpoint_type(&hep->desc);
	qh->endpt = usb_pipeendpoint(urb->pipe);
	qh->dev_addr = usb_pipedevice(urb->pipe);
	qh->hub_addr = 0;
	qh->port_number = 0;
	qh->is_in = !!usb_pipein(urb->pipe);
	qh->mps = usb_maxpacket(urb->dev, urb->pipe, !qh->is_in);
	qh->hb_mult = dwc2_hb_mult(qh->mps);
	qh->mps = dwc2_max_packet(qh->mps);
	qh->data_toggle = DWC_HCTSIZ_DATA0;
	qh->speed = urb->dev->speed;
	qh->lspddev = qh->speed == USB_SPEED_LOW;

	/*
	 * if INT or ISOC, calculate bandwidth info
	 *
	 * 1. FS/LS device/hub directly connect to RH, or
	 * 2. HS device directly connect to RH, or
	 * 3. HS hub connect to RH, HS device connect to that hub, or
	 * 4. HS hub connect to RH, FS/LS device connect to that hub
	 *
	 * case1: running in "UHCI" mode
	 * case2~4: running in "EHCI" mode
	 *      case4 need do split transfer
	 *
	 * Because we only have one root port, so the mode is global to the controller
	 * this is why the mode field is in struct dwc2, not in qh
	 */

	/*
	 * urb->dev->parent is our hub, unless we're the root
	 */
	if (urb->dev->parent != dwc->hcd->self.root_hub) { /* not directly connect to RH */
		qh->hub_addr = urb->dev->parent->devnum;

		/* set up tt info if needed */
		if (urb->dev->tt) {
			qh->port_number = urb->dev->ttport;
			if (urb->dev->tt->hub)
				qh->hub_addr = urb->dev->tt->hub->devnum;
		}

		if (urb->dev->parent->speed == USB_SPEED_HIGH) {
			mode = DWC2_HC_EHCI_MODE;
			if ((qh->speed == USB_SPEED_LOW) || (qh->speed == USB_SPEED_FULL)) {
				dev_err(dwc->dev, "Sorry, SPLIT transfer is not supported!\n");
				kmem_cache_free(dwc->qh_cachep, qh);
				hep->hcpriv = NULL;
				return NULL;
			}
		} else {
			/*FIXME: now we assume both hub is full speed*/
			mode = DWC2_HC_UHCI_MODE;
		}
	} else {	      /* directly connect to RH */
		if ((qh->speed == USB_SPEED_LOW) || (qh->speed == USB_SPEED_FULL))
			mode = DWC2_HC_UHCI_MODE;
		else
			mode = DWC2_HC_EHCI_MODE;
	}

	if (unlikely((dwc->mode != DWC2_HC_MODE_UNKNOWN) &&
			(mode != dwc->mode))) {
		dev_err(dwc->dev, "Change controller operating mode on the fly! "
			"curr_mode = %d, want to change to %d\n",
			dwc->mode, mode);
		//kmem_cache_free(dwc->qh_cachep, qh);
		//return NULL;
		dwc2_host_cleanup(dwc);
	}

	dwc->mode = mode;

	if (dwc->mode == DWC2_HC_UHCI_MODE) {
		qh->usecs = NS_TO_US(usb_calc_bus_time(qh->speed,
					usb_endpoint_dir_in(&hep->desc),
					qh->type == USB_ENDPOINT_XFER_ISOC,
					le16_to_cpu(hep->desc.wMaxPacketSize)));

		/* round down INT interval to 2^N, N=0~7 */
		if (qh->type == USB_ENDPOINT_XFER_INT) {
			if (urb->interval > 2) {
				if (urb->interval < (1 << 2))
					urb->interval = 1 << 1;
				else if (urb->interval < (1 << 3))
					urb->interval = 1 << 2;
				else if (urb->interval < (1 << 4))
					urb->interval = 1 << 3;
				else if (urb->interval < (1 << 5))
					urb->interval = 1 << 4;
				else if (urb->interval < (1 << 6))
					urb->interval = 1 << 5;
				else if (urb->interval < (1 << 7))
					urb->interval = 1 << 6;
				else
					urb->interval = 1 << 7;
			}

			qh->period = urb->interval;
		} else if (qh->type == USB_ENDPOINT_XFER_ISOC) /* isoc bInterval is always 2^N */
			qh->period = urb->interval;

		/* IN DDMA Mode, period cannot >64frame */
		if (qh->period > MAX_FRLIST_EN_NUM) {
			qh->period = MAX_FRLIST_EN_NUM;
			urb->interval = qh->period;
		}
	} else {
		if ( (qh->type == USB_ENDPOINT_XFER_CONTROL) ||
			(qh->type == USB_ENDPOINT_XFER_BULK)) {
			qh->usecs = NS_TO_US(usb_calc_bus_time(USB_SPEED_HIGH,
						qh->is_in, 0, qh->mps));
		}

		if (qh->type == USB_ENDPOINT_XFER_INT) {
			qh->usecs = NS_TO_US(usb_calc_bus_time(USB_SPEED_HIGH,
							qh->is_in, 0,
							qh->hb_mult * qh->mps));
			qh->start_frame = NO_FRAME;

			qh->period = urb->interval >> 3; /* in frame */
			if ( (qh->period == 0) && urb->interval != 1)
				urb->interval = 1;
			if (qh->period > MAX_FRLIST_EN_NUM) {
				qh->period = MAX_FRLIST_EN_NUM;
				urb->interval = qh->period << 3;
			}
		} else if (qh->type == USB_ENDPOINT_XFER_ISOC) {
			qh->period = urb->interval;
			if (qh->period > MAX_DMA_DESC_NUM_HS_ISOC) {
				qh->period = MAX_DMA_DESC_NUM_HS_ISOC;
				urb->interval = qh->period;
			}
			qh->usecs = HS_USECS_ISO(qh->hb_mult * qh->mps);
		}
	}

	qh->hcchar.d32 = 0;
	qh->hcchar.b.mps = qh->mps;
	qh->hcchar.b.epnum = qh->endpt;
	qh->hcchar.b.epdir = qh->is_in;
	qh->hcchar.b.lspddev = qh->lspddev;
	qh->hcchar.b.eptype = qh->type;
	qh->hcchar.b.multicnt = qh->hb_mult;
	qh->hcchar.b.devaddr = qh->dev_addr;

	qh->hcsplt.d32 = 0;
	qh->hcsplt.b.prtaddr = qh->port_number;
	qh->hcsplt.b.hubaddr = qh->hub_addr;

	qh->hctsiz.d32 = 0;
	if (dwc->mode == DWC2_HC_UHCI_MODE) {
		qh->hctsiz.b_ddma.schinfo = 0xFF;
	}

	dwc2_trace_new_qh(qh);

	list_add_tail(&qh->list, &dwc->idle_qh_list);

	switch (qh->type) {
	case USB_ENDPOINT_XFER_CONTROL:
	case USB_ENDPOINT_XFER_BULK:
		break;
	case USB_ENDPOINT_XFER_INT:
		qh->bandwidth_reserved = 0;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		qh->bandwidth_reserved = 0;
		break;
	}

	usb_settoggle (urb->dev, usb_pipeendpoint (urb->pipe), !qh->is_in, 0);

	return qh;
}

static int dwc2_isoc_qh_ptr_pool_init(struct dwc2 *dwc) {
	int i;

	dwc->isoc_qh_ptr_pool = (struct dwc2_isoc_qh_ptr *)__get_free_page(GFP_KERNEL);
	if (!dwc->isoc_qh_ptr_pool)
		return -ENOMEM;

	INIT_LIST_HEAD(&dwc->isoc_qh_ptr_list);
	for (i = 0;
	     i < (PAGE_SIZE / sizeof(struct dwc2_isoc_qh_ptr));
	     i++) {
		struct dwc2_isoc_qh_ptr *qh_ptr = dwc->isoc_qh_ptr_pool + i;
		list_add(&qh_ptr->frm_list, &dwc->isoc_qh_ptr_list);
	}

	return 0;
}

static struct dwc2_isoc_qh_ptr *
dwc2_isoc_qh_ptr_alloc(struct dwc2 *dwc, struct dwc2_qh *qh, struct dwc2_frame *dwc_frm, unsigned char  uframe_index)
{
	struct dwc2_isoc_qh_ptr *qh_ptr = NULL;

	if (!list_empty(&dwc->isoc_qh_ptr_list)) {
		qh_ptr = list_first_entry(&dwc->isoc_qh_ptr_list,
					struct dwc2_isoc_qh_ptr, frm_list);
		list_del(&qh_ptr->frm_list);
	}

	if (!qh_ptr) {
		qh_ptr = kmem_cache_zalloc(dwc->context_cachep, GFP_ATOMIC);
	}

	if (!qh_ptr)
		return NULL;

	qh_ptr->qh = qh;
	qh_ptr->uframe_index = uframe_index;

	list_add(&qh_ptr->frm_list, &dwc_frm->isoc_qh_list);
	list_add(&qh_ptr->qh_list, &qh->isoc_qh_ptr_list);

	return qh_ptr;
}

static void dwc2_isoc_qh_ptr_fini(struct dwc2 *dwc) {
	int			 i = 0;
	struct dwc2_isoc_qh_ptr *qh_ptr;
	struct dwc2_isoc_qh_ptr *qh_ptr_n;

	for (i = 0;
	     i < (PAGE_SIZE / sizeof(struct dwc2_isoc_qh_ptr));
	     i++) {
		struct dwc2_isoc_qh_ptr *qh_ptr = dwc->isoc_qh_ptr_pool + i;
		list_del(&qh_ptr->frm_list);
	}

	list_for_each_entry_safe(qh_ptr, qh_ptr_n,
				&dwc->isoc_qh_ptr_list, frm_list) {
		list_del(&qh_ptr->frm_list);
		kmem_cache_free(dwc->context_cachep, qh_ptr);
	}
}

static struct dwc2_td *dwc2_td_alloc(struct dwc2 *dwc, struct urb *urb, struct dwc2_qh *qh) {
	struct dwc2_td *td;

	td = kmem_cache_zalloc(dwc->td_cachep, GFP_ATOMIC);

	if (td) {
		td->dma_desc_idx = -1;
		td->urb = urb;
		td->owner_qh = qh;
		td->isoc_idx = -1;
	}

	return td;
}

static void dwc2_urb_priv_reset(struct dwc2_urb_priv* urb_priv,
				struct dwc2_channel *chan) {
	struct dwc2_qh		*qh  = urb_priv->owner_qh;
	struct dwc2_td		*td;
	struct dwc2_td		*td_n;
	struct list_head	 tmp_head;
	dwc_otg_host_dma_desc_t *dma_desc;
	struct urb		*urb = urb_priv->urb;

	INIT_LIST_HEAD(&tmp_head);

	list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
		list_del(&td->list);
		list_add_tail(&td->list, &tmp_head);

		if (td->dma_desc_idx >= 0) {
			dma_desc = chan->sw_desc_list + td->dma_desc_idx;
			dma_desc->status.b.a = 0;
		}

		td->dma_desc_idx = -1;
	}

	list_for_each_entry_safe(td, td_n, &urb_priv->done_td_list, list) {
		list_del(&td->list);

		td->dma_desc_idx = -1;
		list_add_tail(&td->list, &urb_priv->td_list);
	}

	list_for_each_entry_safe(td, td_n, &tmp_head, list) {
		list_del(&td->list);
		list_add_tail(&td->list, &urb_priv->td_list);

		if (td->isoc_idx >= 0) {
			urb->iso_frame_desc[td->isoc_idx].status = -EINPROGRESS;
			urb->iso_frame_desc[td->isoc_idx].actual_length = 0;
		}
	}

	urb_priv->state = DWC2_URB_IDLE;

	if (qh->type == USB_ENDPOINT_XFER_CONTROL)
		urb_priv->control_stage = EP0_SETUP_PHASE;
}

static struct dwc2_urb_priv *dwc2_urb_priv_alloc(struct dwc2 *dwc, struct urb *urb, struct dwc2_qh *qh) {
	struct dwc2_urb_priv *urb_priv;

	urb_priv = kmem_cache_zalloc(dwc->context_cachep, GFP_ATOMIC);
	if (urb_priv) {
		urb_priv->state = DWC2_URB_IDLE;
		INIT_LIST_HEAD(&urb_priv->td_list);
		INIT_LIST_HEAD(&urb_priv->done_td_list);

		urb_priv->urb = urb;
		urb_priv->owner_qh = qh;
		urb->hcpriv = urb_priv;
	}

	return urb_priv;
}

static int dwc2_start_bc_channel(struct dwc2 *dwc, struct dwc2_channel *channel) {
	struct dwc2_host_if	*host_if  = &dwc->host_if;
	int			 chan_num = channel->number;
	dwc_otg_hc_regs_t	*hc_regs  = host_if->hc_regs[chan_num];

	hcintmsk_data_t hc_intr_mask;
	haintmsk_data_t haintmsk;
	hctsiz_data_t	hctsiz;
	hcchar_data_t	hcchar;

	struct dwc2_urb_priv	*urb_priv = channel->urb_priv;
	struct urb		*urb	  = urb_priv->urb;
	struct dwc2_qh		*qh	  = urb_priv->owner_qh;
	u32			 is_in	  = qh->is_in;
	int			 pid	  = qh->data_toggle;
	struct dwc2_td		*td;
	dwc_otg_host_dma_desc_t	*dma_desc;
	int			 desc_idx = 0;

	if (qh->type == USB_ENDPOINT_XFER_CONTROL)
		is_in = !!usb_pipein(urb->pipe);

	dwc2_trace_start_channel(urb_priv, qh, channel);

	/* Clear old interrupt conditions for this host channel. */
	hc_intr_mask.d32 = 0xFFFFFFFF;
	dwc_writel(hc_intr_mask.d32, &hc_regs->hcint);

	if ((qh->type == USB_ENDPOINT_XFER_CONTROL) &&
		(urb_priv->control_stage == EP0_SETUP_PHASE)) {
		td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);

		td->dma_desc_idx = desc_idx;
		dma_desc = channel->sw_desc_list + desc_idx;

		dma_desc->buf = td->buffer;
		dma_desc->status.d32 = 0;
		dma_desc->status.b.a = 1;
		dma_desc->status.b.sup = 1;
		dma_desc->status.b.n_bytes = td->len;

		pid = DWC_HCTSIZ_SETUP;
		is_in = 0;

		desc_idx++;
	} else if ((qh->type == USB_ENDPOINT_XFER_CONTROL) &&
		(urb_priv->control_stage == EP0_STATUS_PHASE)) {
		td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);

		td->dma_desc_idx = desc_idx;
		dma_desc = channel->sw_desc_list + desc_idx;

		dma_desc->buf = td->buffer;
		dma_desc->status.d32 = 0;
		dma_desc->status.b.a = 1;
		dma_desc->status.b.n_bytes = td->len;
		pid = DWC_HCTSIZ_DATA1;

		/*
		 * Direction is opposite of data direction or IN if no
		 * data.
		 */
		if (urb_priv->urb->transfer_buffer_length == 0) {
			is_in = 1;
		} else {
			is_in = !is_in;
		}

		desc_idx++;
	} else {	      /* Bulk or Control Data */
		list_for_each_entry(td, &urb_priv->td_list, list) {
			if ((td->ctrl_stage == EP0_STATUS_PHASE))
				break;

			td->dma_desc_idx = desc_idx;
			dma_desc = channel->sw_desc_list + desc_idx;

			dma_desc->buf = td->buffer;
			dma_desc->status.d32 = 0;
			dma_desc->status.b.a = 1;
			dma_desc->status.b.n_bytes = td->len;

			desc_idx++;

			if (desc_idx == MAX_DMA_DESC_NUM_GENERIC)
				break;
		}
	}


	/* set IOC and EOL for the last descriptor */
	dma_desc = channel->sw_desc_list + desc_idx - 1;
	dma_desc->status.b.ioc = 1;
	dma_desc->status.b.eol = 1;

	hctsiz.d32 = 0;
	/* High-Speed OUT must do ping when encount error */
	if (!is_in && (qh->speed == USB_SPEED_HIGH) &&
		(urb_priv->error_count > 0))
		hctsiz.b.dopng = 1;

	hctsiz.b_ddma.pid = pid;
	hctsiz.b_ddma.ntd = desc_idx - 1;
	hctsiz.b_ddma.schinfo = 0xFF;

	dwc_writel(hctsiz.d32, &hc_regs->hctsiz);

	/* reset hcdma */
	dwc_writel(channel->hw_desc_list, &hc_regs->hcdma);

	dwc_writel(0, &hc_regs->hcsplt);

	/* Enable channel interrupts required for this transfer. */
	hc_intr_mask.d32 = 0;
	hc_intr_mask.b.chhltd = 1;
	hc_intr_mask.b.xfercompl = 1;
	//hc_intr_mask.b.bna = 1;
	hc_intr_mask.b.ahberr = 1;

	dwc_writel(hc_intr_mask.d32, &hc_regs->hcintmsk);

	/* Enable the top level host channel interrupt. */
	haintmsk.d32 = dwc_readl(&host_if->host_global_regs->haintmsk);
	haintmsk.d32 |= (1 << chan_num);
	dwc_writel(haintmsk.d32, &host_if->host_global_regs->haintmsk);

	hcchar.d32 = qh->hcchar.d32;
	hcchar.b.multicnt = 1;
	hcchar.b.epdir = is_in;
	hcchar.b.chen = 1;
	hcchar.b.chdis = 0;

	dwc2_trace_chan_trans_info(dwc, channel, urb_priv->control_stage, urb_priv->error_count,
				hcchar.d32, hctsiz.d32, dwc_readl(&hc_regs->hcdma));

	dwc_writel(hcchar.d32, &hc_regs->hcchar);

	return 0;
}

static int dwc2_submit_control(struct dwc2 *dwc, struct urb *urb,
			struct dwc2_qh *qh) {
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_td		*td;
	int			 len   = urb->transfer_buffer_length;
	dma_addr_t		 buf   = urb->transfer_dma;
	int			 is_in = !!usb_pipein(urb->pipe);
	int			 max_len;
	int			 ret   = 0;

	urb_priv = dwc2_urb_priv_alloc(dwc, urb, qh);
	if (!urb_priv)
		return -ENOMEM;

	urb_priv->control_stage = EP0_SETUP_PHASE;

	/* SETUP TD */
	td = dwc2_td_alloc(dwc, urb, qh);
	if (!td) {
		urb->hcpriv = NULL;
		kmem_cache_free(dwc->context_cachep, urb_priv);
		return -ENOMEM;
	}

	td->buffer = urb->setup_dma;
	td->len = 8;
	td->ctrl_stage = EP0_SETUP_PHASE;

	list_add_tail(&td->list, &urb_priv->td_list);

	if (is_in)
		max_len = MAX_DMA_DESC_IN_SIZE;
	else
		max_len = MAX_DMA_DESC_OUT_SIZE;

	/* 0~n data packets */
	while (len > 0) {
		td = dwc2_td_alloc(dwc, urb, qh);
		if (!td) {
			ret = -ENOMEM;
			goto rollback;
		}

		td->buffer = buf;
		td->len = (len > max_len) ? max_len: len;
		td->ctrl_stage = EP0_DATA_PHASE;
		list_add_tail(&td->list, &urb_priv->td_list);

		buf += td->len;
		len -= td->len;
	}

	/* IN: the last td->len must be multiple of MPS */
	if (is_in && urb->transfer_buffer_length) {
		td->len = qh->mps * DIV_ROUND_UP(td->len, qh->mps);
	}

	/* Status TD */
	td = dwc2_td_alloc(dwc, urb, qh);
	if (!td) {
		ret = -ENOMEM;
		goto rollback;
	}
	td->buffer = 0;
	td->len = 0;
	td->ctrl_stage = EP0_STATUS_PHASE;
	list_add_tail(&td->list, &urb_priv->td_list);

	dwc2_trace_urb_submit(urb, qh);

	list_add_tail(&urb_priv->list, &qh->urb_list);

	return 0;

rollback:
	{
		struct dwc2_td *td_n;
		list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
			list_del(&td->list);
			kmem_cache_free(dwc->td_cachep, td);
		}
	}

	urb->hcpriv = NULL;
	kmem_cache_free(dwc->context_cachep, urb_priv);
	return ret;
}

static int dwc2_submit_bulk(struct dwc2 *dwc, struct urb *urb, struct dwc2_qh *qh) {
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_td		*td = NULL;
	int			 len = urb->transfer_buffer_length;
	dma_addr_t		 buf = urb->transfer_dma;
	int			 max_len;
	int			 ret = 0;

	if (len == 0)
		return -EMSGSIZE;

	urb_priv = dwc2_urb_priv_alloc(dwc, urb, qh);
	if (!urb_priv)
		return -ENOMEM;

	if (qh->is_in)
		max_len = MAX_DMA_DESC_IN_SIZE;
	else
		max_len = MAX_DMA_DESC_OUT_SIZE;

	while (len > 0) {
		td = dwc2_td_alloc(dwc, urb, qh);
		if (!td) {
			ret = -ENOMEM;
			goto rollback;
		}

		td->buffer = buf;
		td->len = (len > max_len) ? max_len: len;
		list_add_tail(&td->list, &urb_priv->td_list);

		buf += td->len;
		len -= td->len;
	}

	/* IN: the last td->len must be multiple of MPS */
	if (qh->is_in) {
		td->len = qh->mps * DIV_ROUND_UP(td->len, qh->mps);
	}

	/* check need a zero packet */
	if (likely(urb->transfer_buffer_length != 0)) {
		if ( (urb->transfer_flags & URB_ZERO_PACKET)
			&& !(urb->transfer_buffer_length % qh->mps)) {

			td = dwc2_td_alloc(dwc, urb, qh);
			if (!td) {
				ret = -ENOMEM;
				goto rollback;
			}

			td->buffer = 0;
			td->len = 0;
			list_add_tail(&td->list, &urb_priv->td_list);
		}
	}

	dwc2_trace_urb_submit(urb, qh);

	list_add_tail(&urb_priv->list, &qh->urb_list);

	return 0;

rollback:
	{
		struct dwc2_td *td_n;
		list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
			list_del(&td->list);
			kmem_cache_free(dwc->td_cachep, td);
		}
	}

	urb->hcpriv = NULL;
	kmem_cache_free(dwc->context_cachep, urb_priv);
	return ret;
}

static unsigned short dwc2_periodic_usecs(struct dwc2 *dwc,
					unsigned frame, unsigned uframe) {
	struct dwc2_frame	*dwc_frm  = dwc->frame_list + frame;
	unsigned short		 bandwidth = 0;
	struct dwc2_qh		*qh;
	struct dwc2_isoc_qh_ptr *isoc_qh;

	list_for_each_entry(isoc_qh, &dwc_frm->isoc_qh_list, frm_list) {
		if ((isoc_qh->qh->smask & (1 << uframe))&&(isoc_qh->uframe_index == uframe))
			bandwidth += isoc_qh->qh->usecs;
	}

	for (qh = dwc_frm->int_qh_head;
	     qh != NULL;
	     qh = qh->next) {
		/* is it in the S-mask? */
		if (qh->smask & (1 << uframe))
			bandwidth += qh->usecs;
	}

	return bandwidth;
}

static int dwc2_check_period (
	struct dwc2	*dwc,
	unsigned	 frame,
	unsigned	 uframe,
	unsigned	 period,
	unsigned	usecs
	) {
	unsigned claimed;

	if (period == 0) {
		/* check every (micro)frame */
		if (dwc->mode == DWC2_HC_EHCI_MODE) {
			do {
				for (uframe = 0; uframe < 8; uframe++) {
					claimed = dwc2_periodic_usecs(dwc, frame, uframe);
					if ( (claimed + usecs) > 100) {
						return -ENOSPC;
					}
				}
			} while ( (frame += 1) < MAX_FRLIST_EN_NUM);
		} else {
			do {
				claimed = dwc2_periodic_usecs(dwc, frame, 0);
				if ( (claimed + usecs) > 900)
					return -ENOSPC;
			} while ( (frame += 1) < MAX_FRLIST_EN_NUM);
		}
	} else {
		/* just check the specified (micro)frame, at that period */
		if (dwc->mode == DWC2_HC_EHCI_MODE) {
			do {
				claimed = dwc2_periodic_usecs(dwc, frame, uframe);
				if ( (claimed + usecs) > 100) {
					return -ENOSPC;
				}
			} while ( (frame += period) < MAX_FRLIST_EN_NUM);
		} else {
			do {
				claimed = dwc2_periodic_usecs(dwc, frame, 0);
				if ( (claimed + usecs) > 900)
					return -ENOSPC;
			} while ( (frame += period) < MAX_FRLIST_EN_NUM);
		}
	}

	return 0;
}

static int dwc2_qh_schedule(struct dwc2 *dwc, struct dwc2_qh *qh) {
	unsigned	frame;
	unsigned	uframe = 0;
	int		status = -ENOSPC;
	unsigned	period = qh->period;
	unsigned	i;

	/* NOTE: we do not support per (micro)frame schedule, so qh->period won't be 0 */
	if (qh->period == 0) {
		frame = 0;
		status = dwc2_check_period(dwc, 0, 0, 0, qh->usecs);
	} else {
		if (dwc->mode == DWC2_HC_EHCI_MODE) {
			/* granularity is microframe */
			for (i = qh->period; status && i > 0; --i) {
				frame = ++dwc->random_frame % qh->period;
				for (uframe = 0; uframe < 8; uframe++) {
					status = dwc2_check_period(dwc, frame, uframe, qh->period, qh->usecs);
					if (status == 0) /* find one! */
						break;
				}
			}
		} else {
			/* granularity is frame */
			for (i = qh->period; status && i > 0; -- i) {
				frame = ++dwc->random_frame % qh->period;
				status = dwc2_check_period(dwc, frame, 0, qh->period, qh->usecs);
				if (status == 0)
					break;
			}
		}
	}

	if (status)
		goto done;

	qh->start_frame = frame;

	if (qh->period == 0)
		qh->smask = 0xff;
	else
		qh->smask = (1 << uframe);

	if (period == 0)
		period = 1;

	for (i = qh->start_frame; i < MAX_FRLIST_EN_NUM; i += period) {
		struct dwc2_qh		*prev_qh;
		struct dwc2_qh		*pos;
		struct dwc2_frame	*dwc_frm = dwc->frame_list + i;

		prev_qh = dwc_frm->int_qh_head;

		/*
		 * queue this QH into the int_qh list, period low ---> fast
		 * if two QHs has the same period, we are late, so queue after that one
		 */

		if (prev_qh == NULL) { /* list is empty */
			dwc_frm->int_qh_head = qh;
		} else if (period > prev_qh->period) {
			/* insert as the head */
			qh->next = prev_qh;
			dwc_frm->int_qh_head = qh;
		} else {
			/* at least two entries in the list, and we are not before the head */
			for (pos = prev_qh->next;
			     pos != NULL;
			     pos = pos->next) {
				if (period > pos->period)
					break;
				else
					prev_qh = pos;
			}

			/* queue between prev_qh and pos */
			prev_qh->next = qh;
			qh->next = pos; /* note that pos maybe NULL */
		}

#if 0
		{
			printk("frame[%d] ", i);
			for (pos = dwc_frm->int_qh_head;
			     pos != NULL;
			     pos = pos->next) {
				printk(" qh%p ---> ", pos);
			}
			printk("\n");
		}
#endif

		/* now we are in the int_qh list, but at this time, no TDs */
	}

	{
		int bd;

		/* NOTE: bandwidth_allocated units: microseconds/frame */
		if (dwc->mode == DWC2_HC_EHCI_MODE) {
			bd = qh->usecs * 8;
		} else {
			bd = qh->usecs;
		}

		if (qh->period)
			bd /= qh->period;

		dwc->hcd->self.bandwidth_allocated += bd;
		qh->bandwidth_reserved = 1;
	}

done:
	dwc2_trace_qh_schedule_info(qh, status);
	return status;
}

static int dwc2_submit_interrupt(struct dwc2 *dwc, struct urb *urb, struct dwc2_qh *qh) {
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_td		*td;
	int			 ret = 0;

	/* sanity check */
	if (urb->transfer_buffer_length > (qh->hb_mult * qh->mps)) {
		printk("%s: urb->transfer_buffer_length(%d) > (qh->hb_mult(%u) * qh->mps(%u)\n",
			__func__, urb->transfer_buffer_length, qh->hb_mult, qh->mps);

		return -EMSGSIZE;
	}

	if (!qh->bandwidth_reserved) {
		ret = dwc2_qh_schedule(dwc, qh);
		if (ret)
			return ret;
	}

	urb_priv = dwc2_urb_priv_alloc(dwc, urb, qh);
	if (!urb_priv)
		return -ENOMEM;

	td = dwc2_td_alloc(dwc, urb, qh);
	if (!td) {
		ret = -ENOMEM;
		goto fail_alloc_td;
	}

	td->buffer = urb->transfer_dma;
	td->len = urb->transfer_buffer_length;
	/* IN: td->len must be multiple of MPS */
	if (qh->is_in) {
		td->len = qh->mps * DIV_ROUND_UP(td->len, qh->mps);
	}
	list_add_tail(&td->list, &urb_priv->td_list);

	dwc2_trace_urb_submit(urb, qh);

	list_add_tail(&urb_priv->list, &qh->urb_list);

	return 0;

fail_alloc_td:
	urb->hcpriv = NULL;
	kmem_cache_free(dwc->context_cachep, urb_priv);

	return ret;
}

static int dwc2_start_int_channel(struct dwc2 *dwc, struct dwc2_channel *channel) {
	struct dwc2_host_if	*host_if  = &dwc->host_if;
	int			 chan_num = channel->number;
	dwc_otg_hc_regs_t	*hc_regs  = host_if->hc_regs[chan_num];

	hcintmsk_data_t hc_intr_mask;
	haintmsk_data_t haintmsk;
	hctsiz_data_t	hctsiz;
	hcchar_data_t	hcchar;

	struct dwc2_urb_priv	*urb_priv = channel->urb_priv;
	struct dwc2_qh		*qh	  = urb_priv->owner_qh;
	u32			 is_in	  = qh->is_in;
	struct dwc2_td		*td;
	dwc_otg_host_dma_desc_t	*dma_desc;
	int			 hb_mult = 1;

	dwc2_trace_start_channel(urb_priv, qh, channel);

	/* Clear old interrupt conditions for this host channel. */
	hc_intr_mask.d32 = 0xFFFFFFFF;
	dwc_writel(hc_intr_mask.d32, &hc_regs->hcint);

	/* INT only have one TD */
	td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);

	td->dma_desc_idx = 0;
	dma_desc = channel->sw_desc_list;
	dma_desc->buf = td->buffer;
	dma_desc->status.d32 = 0;
	dma_desc->status.b.a = 1;
	dma_desc->status.b.n_bytes = td->len;

	/* set IOC and EOL */
	dma_desc->status.b.ioc = 1;
	dma_desc->status.b.eol = 1;

	hctsiz.d32 = 0;
	hctsiz.b_ddma.pid = qh->data_toggle;
	hctsiz.b_ddma.ntd = 0;
	dwc_writel(hctsiz.d32, &hc_regs->hctsiz);

	dwc2_dispatch_channel(dwc, channel, qh);

	/* reset hcdma */
	dwc_writel(channel->hw_desc_list, &hc_regs->hcdma);

	dwc_writel(0, &hc_regs->hcsplt);

	/* Enable channel interrupts required for this transfer. */
	hc_intr_mask.d32 = 0;
	hc_intr_mask.b.chhltd = 1;
	hc_intr_mask.b.xfercompl = 1;
	hc_intr_mask.b.bna = 1;
	hc_intr_mask.b.ahberr = 1;

	dwc_writel(hc_intr_mask.d32, &hc_regs->hcintmsk);

	/* Enable the top level host channel interrupt. */
	haintmsk.d32 = dwc_readl(&host_if->host_global_regs->haintmsk);
	haintmsk.d32 |= (1 << chan_num);
	dwc_writel(haintmsk.d32, &host_if->host_global_regs->haintmsk);

	hb_mult = td->len / qh->mps;
	if (hb_mult % qh->mps)
		hb_mult ++;

	hcchar.d32 = qh->hcchar.d32;
	hcchar.b.multicnt = hb_mult;
	hcchar.b.epdir = is_in;
	hcchar.b.chen = 1;
	hcchar.b.chdis = 0;

	dwc2_trace_chan_trans_info(dwc, channel, urb_priv->control_stage, urb_priv->error_count,
				hcchar.d32, dwc_readl(&hc_regs->hctsiz), dwc_readl(&hc_regs->hcdma));
	dwc_writel(hcchar.d32, &hc_regs->hcchar);

	return 0;
}

static int dwc2_uhci_slot_ok(struct dwc2 *dwc, u32 frame, u32 usecs, u32 period) {
	u32 claimed;

	do {
		claimed = dwc2_periodic_usecs(dwc, frame, 0);
		if (claimed + usecs > 1000)
			return 0;

		frame += period;
	} while (frame < MAX_FRLIST_EN_NUM);

	return 1;
}

static int dwc2_isoc_qh_schedule_uhci(struct dwc2 *dwc, struct dwc2_qh *qh) {
	u32 start;

	/* select start frame from [0, period-1] */
	for (start = 0; start < qh->period; start++) {
		if (dwc2_uhci_slot_ok(dwc, start, qh->usecs, qh->period))
			break;
	}

	if (start == qh->period) {
		dev_dbg(dwc->dev, "qh %p schedule fail, bandwidth full\n", qh);
		dwc2_trace_qh_schedule_info(qh, -ENOSPC);
		return -ENOSPC;
	}

	qh->start_frame = start;
	qh->smask = 0xFF;

	dwc2_trace_qh_schedule_info(qh, 0);

	return 0;
}

static int dwc2_ehci_slot_ok(struct dwc2 *dwc, u32 uframe, u32 usecs, u32 period) {
	u32 claimed;

	do {
		claimed = dwc2_periodic_usecs(dwc, uframe >> 3, uframe & 0x7);
		if (claimed + usecs > 100)
			return 0;

		uframe += period;
	} while (uframe < MAX_FRLIST_EN_NUM * 8);

	return 1;
}

static int dwc2_isoc_qh_schedule_ehci(struct dwc2 *dwc, struct dwc2_qh *qh) {
	u32 start;

	/* select start microframe from [0, period-1] */
	for (start = 0; start < qh->period; start++) {
		if (dwc2_ehci_slot_ok(dwc, start, qh->usecs, qh->period))
			break;
	}

	if (start == qh->period) {
		dev_dbg(dwc->dev, "qh %p schedule fail, bandwidth full\n", qh);
		dwc2_trace_qh_schedule_info(qh, -ENOSPC);
		return -ENOSPC;
	}

	qh->start_frame = start;

	if (qh->period == 1)
		qh->smask = 0xff;

	if (qh->period == 2) {
		if (start & 0x1)
			qh->smask = 0xaa;
		else
			qh->smask = 0x55;
	}

	if (qh->period == 4) {
		switch(start) {
		case 0:
			qh->smask = 0x11;
			break;
		case 1:
			qh->smask = 0x22;
			break;
		case 2:
			qh->smask = 0x44;
			break;
		case 3:
			qh->smask = 0x88;
		default:
			;
		}

	}

	if (qh->period >= 8) {
		qh->smask = 1 << (start & 0x7);
	}

	dwc2_trace_qh_schedule_info(qh, 0);

	return 0;
}

static void dwc2_start_isoc_channel(struct dwc2 *dwc, struct dwc2_channel *channel) {
	struct dwc2_host_if	*host_if  = &dwc->host_if;
	int			 chan_num = channel->number;
	dwc_otg_hc_regs_t	*hc_regs  = host_if->hc_regs[chan_num];

	hcintmsk_data_t hc_intr_mask;
	haintmsk_data_t haintmsk;
	hctsiz_data_t	hctsiz;
	hcchar_data_t	hcchar;

	struct dwc2_qh		*qh	  = channel->qh;
	u32			 is_in	  = qh->is_in;

	dwc2_trace_start_channel(NULL, qh, channel);

	/* Clear old interrupt conditions for this host channel. */
	hc_intr_mask.d32 = 0xFFFFFFFF;
	dwc_writel(hc_intr_mask.d32, &hc_regs->hcint);

	memset(channel->sw_desc_list, 0,
		MAX_DMA_DESC_NUM_HS_ISOC * sizeof(dwc_otg_host_dma_desc_t));

	hctsiz.d32 = 0;
	if (dwc->mode == DWC2_HC_EHCI_MODE)
		hctsiz.b_ddma.ntd = MAX_DMA_DESC_NUM_HS_ISOC - 1;
	else
		hctsiz.b_ddma.ntd = MAX_FRLIST_EN_NUM - 1;
	dwc_writel(hctsiz.d32, &hc_regs->hctsiz);

	dwc2_dispatch_channel(dwc, channel, qh);

	/* reset hcdma */
	dwc_writel(channel->hw_desc_list, &hc_regs->hcdma);

	if (dwc->mode == DWC2_HC_EHCI_MODE)
		channel->remain_slots = MAX_DMA_DESC_NUM_HS_ISOC;
	else
		channel->remain_slots = MAX_FRLIST_EN_NUM;
	dwc_writel(0, &hc_regs->hcsplt);

	/* Enable channel interrupts required for this transfer. */
	hc_intr_mask.d32 = 0;
	hc_intr_mask.b.chhltd = 1;
	hc_intr_mask.b.xfercompl = 1;
	hc_intr_mask.b.ahberr = 1;

	dwc_writel(hc_intr_mask.d32, &hc_regs->hcintmsk);

	/* Enable the top level host channel interrupt. */
	haintmsk.d32 = dwc_readl(&host_if->host_global_regs->haintmsk);
	haintmsk.d32 |= (1 << chan_num);
	dwc_writel(haintmsk.d32, &host_if->host_global_regs->haintmsk);

	hcchar.d32 = qh->hcchar.d32;
	hcchar.b.multicnt = qh->hb_mult;
	hcchar.b.epdir = is_in;
	hcchar.b.chen = 1;
	hcchar.b.chdis = 0;

	dwc2_trace_chan_trans_info(dwc, channel, 0, 0,
				hcchar.d32, hctsiz.d32, dwc_readl(&hc_regs->hcdma));
	dwc_writel(hcchar.d32, &hc_regs->hcchar);
}

static int dwc2_isoc_qh_schedule(struct dwc2 *dwc, struct dwc2_qh *qh) {
	int retval;
	struct dwc2_channel *chan;
	struct dwc2_frame *dwc_frm;
	unsigned short frame;
	unsigned short full_frame;
	unsigned char  uframe_index;

	chan = dwc2_request_isoc_chan(dwc, qh);
	if (!chan)
		return -ENOMEM;

	if (dwc->mode == DWC2_HC_EHCI_MODE)
		retval = dwc2_isoc_qh_schedule_ehci(dwc, qh);
	else
		retval = dwc2_isoc_qh_schedule_uhci(dwc, qh);

	if (retval)
		return retval;

	frame = qh->start_frame;

	uframe_index = qh->start_frame;


	do {
		full_frame = dwc_full_frame_num(dwc, frame);
		if (full_frame >= MAX_FRLIST_EN_NUM)
			break;

		dwc_frm = dwc->frame_list + full_frame;
		dwc2_isoc_qh_ptr_alloc(dwc, qh, dwc_frm, uframe_index);

		frame = dwc_frame_num_inc(dwc, frame, qh->period);

		uframe_index  += qh->period;
		uframe_index  = uframe_index%8;

	} while (1);


	dwc2_start_isoc_channel(dwc, chan);

	return 0;
}

static int dwc2_isoc_qh_chan_idle(struct dwc2 *dwc, struct dwc2_qh *qh) {
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_td		*td;
	unsigned short		 curr_frame;

	if (unlikely(list_empty(&qh->urb_list)))
		return 1;

	urb_priv = list_first_entry(&qh->urb_list, struct dwc2_urb_priv, list);
	if (unlikely(list_empty(&urb_priv->td_list)))
		return 0;
	else {
		td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);
		if (td->dma_desc_idx < 0)
			return 1;
	}

#if 0
	/* the first urb is scheduled, but this not always means that the channel is not idle */
	curr_frame = dwc2_hc_get_frame_number(dwc);
	if (dwc_frame_num_gt(dwc, curr_frame, qh->last_frame))
		return 1;
#endif
	return 0;
}

#define DWC2_ISOC_SLOP_EHCI	8
#define DWC2_ISOC_SLOP_UHCI	2

/* return -ENOSPC when dma descriptor is not enough, otherwise 0 */
static int dwc2_isoc_schedule_urb(struct dwc2 *dwc, struct dwc2_urb_priv *urb_priv) {
	struct dwc2_qh		*qh	     = urb_priv->owner_qh;
	struct dwc2_channel	*chan	     = qh->channel;
	unsigned short		 curr_frame;
	unsigned short		 frame;
	unsigned short		 start_frame;
	unsigned short		 total_slots = 0;
	unsigned short		 mod;
	unsigned short		 max_slots;
	struct dwc2_td		*td;
	dwc_otg_host_dma_desc_t	*dma_desc;

	if (dwc->mode == DWC2_HC_EHCI_MODE) {
		mod = (DWC2_MAX_MICROFRAME - 1);
		max_slots = MAX_DMA_DESC_NUM_HS_ISOC;
	} else {
		mod = (DWC2_MAX_FRAME - 1);
		max_slots = MAX_FRLIST_EN_NUM;
	}

	total_slots = urb_priv->urb->number_of_packets * qh->period;

	curr_frame = dwc2_hc_get_frame_number(dwc);

	if (dwc->mode == DWC2_HC_EHCI_MODE)
		curr_frame = dwc_frame_num_inc(dwc, curr_frame, DWC2_ISOC_SLOP_EHCI);
	else
		curr_frame = dwc_frame_num_inc(dwc, curr_frame, DWC2_ISOC_SLOP_UHCI);

	if (dwc2_isoc_qh_chan_idle(dwc, qh)) {
		printk(KERN_DEBUG"dwc2 start isco\n");
		/* schedule based on NOW long time no transfer or first time*/
		start_frame = curr_frame + ((qh->start_frame - curr_frame) & (qh->period - 1));
	} else {
		if (dwc_frame_num_le(dwc, curr_frame, qh->last_frame)) {
			/* schedule based on the last frame */
			start_frame = dwc_frame_num_inc(dwc, qh->last_frame, qh->period);
		} else {
			/* schedule based on now underrun happen*/
			printk(KERN_DEBUG"dwc2 usb iso underrun occur!!!\n");
			start_frame = curr_frame + ((qh->start_frame - curr_frame) & (qh->period - 1));
			total_slots += (start_frame - qh->last_frame) & mod;
		}
	}

	dwc2_trace_isoc_schedule(dwc, urb_priv, dwc2_hc_get_frame_number(dwc),
				start_frame, total_slots);

	if (total_slots > qh->channel->remain_slots)
		return -ENOSPC;

	urb_priv->state = DWC2_URB_TRANSFERING;
	urb_priv->isoc_slots = total_slots;
	chan->remain_slots -= total_slots;

	start_frame &= mod;

	urb_priv->urb->start_frame = start_frame;

	frame = start_frame;
	list_for_each_entry(td, &urb_priv->td_list, list) {
		td->frame = frame;
		td->dma_desc_idx = td->frame % max_slots;

		dma_desc = chan->sw_desc_list + td->dma_desc_idx;
		dma_desc->buf = td->buffer;
		dma_desc->status.d32 = 0;
		dma_desc->status.b_isoc.n_bytes = td->len;
		dma_desc->status.b_isoc.a = 1;

		frame = dwc_frame_num_inc(dwc, frame, qh->period);
	}

	dwc2_trace_isoc_urb_priv_slots(urb_priv);

	/* the last entry has ioc */
	td = list_entry(urb_priv->td_list.prev, struct dwc2_td, list);
	dma_desc = chan->sw_desc_list + td->dma_desc_idx;
	dma_desc->status.b_isoc.ioc = 1;

	qh->last_frame = td->frame;

	return 0;
}

static void dwc2_isoc_schedule(struct dwc2 *dwc, struct dwc2_qh *qh) {
	struct dwc2_host_if	*host_if = &dwc->host_if;
	struct dwc2_channel	*chan	 = qh->channel;
	dwc_otg_hc_regs_t	*hc_regs = host_if->hc_regs[chan->number];
	hcchar_data_t		 hcchar;
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_td		*td;

	hcchar.d32 = dwc_readl(&hc_regs->hcchar);
	if (hcchar.b.chen == 0) {
		return;
	}

	list_for_each_entry(urb_priv, &qh->urb_list, list) {
		if ((urb_priv->state == DWC2_URB_IDLE) &&
			!list_empty(&urb_priv->td_list)) {
			td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);
			if (td->dma_desc_idx < 0) {
				if (dwc2_isoc_schedule_urb(dwc, urb_priv))
					break;
			}
		}
	}
}

static int dwc2_submit_isochronous(struct dwc2 *dwc, struct urb *urb, struct dwc2_qh *qh) {
	struct dwc2_urb_priv *urb_priv;
	struct dwc2_td *td;
	int i = 0;
	int retval = 0;

	/* sanity check */
	if (urb->interval >= DWC2_MAX_FRAME ||
		urb->number_of_packets >= DWC2_MAX_FRAME) {
		printk("%s: urb->interval = %d, urb->number_of_packets = %d\n",
			__func__, urb->interval, urb->number_of_packets);
		return -EFBIG;
	}

	for (i = 0; i < urb->number_of_packets; i++) {
		if (urb->iso_frame_desc[i].length > (qh->hb_mult * qh->mps)) {
			printk("urb %p iso_frame_desc[%d].length %u > (qh->hb_mult(%u) * qh->qh->mps(%u))\n",
				urb, i, urb->iso_frame_desc[i].length, qh->hb_mult, qh->mps);
			return -EMSGSIZE;
		}
	}

#if 0
	if (usb_endpoint_type(&urb->ep->desc) == USB_ENDPOINT_XFER_ISOC) {
		dump_stack();
	}
#endif

	urb_priv = dwc2_urb_priv_alloc(dwc, urb, qh);
	if (!urb_priv)
		return -ENOMEM;

	for (i = 0; i < urb->number_of_packets; i++) {
		td = dwc2_td_alloc(dwc, urb, qh);
		if (!td) {
			retval = -ENOMEM;
			goto rollback;
		}

		td->buffer = urb->transfer_dma + urb->iso_frame_desc[i].offset;
		td->len = urb->iso_frame_desc[i].length;
		td->isoc_idx = i;
		list_add_tail(&td->list, &urb_priv->td_list);
	}

	if (!qh->bandwidth_reserved) {
		int bd;

		retval = dwc2_isoc_qh_schedule(dwc, qh);
		if (retval) {
			goto fail_qh_schedule;
		}


		/* NOTE: bandwidth_allocated units: microseconds/frame */
		if (dwc->mode == DWC2_HC_EHCI_MODE) {
			bd = qh->usecs * 8;
		} else {
			bd = qh->usecs;
		}

		if (qh->period)
			bd /= qh->period;

		dwc->hcd->self.bandwidth_allocated += bd;

		qh->bandwidth_reserved = 1;
	}

	dwc2_trace_urb_submit(urb, qh);
	list_add_tail(&urb_priv->list, &qh->urb_list);

	dwc->hcd->self.bandwidth_isoc_reqs++;

	dwc2_isoc_schedule(dwc, qh);

	return 0;

fail_qh_schedule:
rollback:
	{
		struct dwc2_td *td_n;
		list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
			list_del(&td->list);
			kmem_cache_free(dwc->td_cachep, td);
		}
	}

	urb->hcpriv = NULL;
	kmem_cache_free(dwc->context_cachep, urb_priv);
	return retval;
}

static int dwc2_start_channel(struct dwc2 *dwc, struct dwc2_channel *channel) {
	struct dwc2_qh *qh = channel->urb_priv->owner_qh;

	switch(qh->type) {
	case USB_ENDPOINT_XFER_CONTROL:
	case USB_ENDPOINT_XFER_BULK:
		return dwc2_start_bc_channel(dwc, channel);
	case USB_ENDPOINT_XFER_INT:
		return dwc2_start_int_channel(dwc, channel);
	default:
		return -EINVAL;
	}
}

static void  __dwc2_disable_channel_stage1(
	struct dwc2 *dwc, struct dwc2_channel *chan, int urb_priv_state) {

	dwc_otg_hc_regs_t	*hc_regs = dwc->host_if.hc_regs[chan->number];
	struct dwc2_urb_priv	*urb_priv = chan->urb_priv;
	hcintmsk_data_t		 hc_intr_mask;
	hcchar_data_t		 hcchar;

	urb_priv->state = urb_priv_state;
	chan->disable_stage = 1;

	hc_intr_mask.d32 = 0;
	hc_intr_mask.b.chhltd = 1;
	dwc_writel(hc_intr_mask.d32, &hc_regs->hcintmsk);

	/*  Make sure no other interrupts besides halt are currently pending */
	dwc_writel(~hc_intr_mask.d32, &hc_regs->hcint);
	hcchar.d32 = dwc_readl(&hc_regs->hcchar);
	if (hcchar.b.chen != 0) {
		hcchar.b.chen = 1;
		hcchar.b.chdis = 1;
		dwc_writel(hcchar.d32, &hc_regs->hcchar);
	}
}

static void dwc2_schedule(struct dwc2 *dwc);
static void __dwc2_disable_channel_stage2(struct dwc2 *dwc,
					struct dwc2_channel *chan, int urb_status) {
	struct dwc2_urb_priv	*urb_priv = chan->urb_priv;
	struct dwc2_qh		*qh;

	chan->disable_stage = 0;
	dwc2_free_channel(dwc, chan);

	if (!urb_priv)
		return;

	qh = urb_priv->owner_qh;

	if (qh->disabling) {
		dwc2_qh_destroy(dwc, qh);
	} else {
		if (qh->type != USB_ENDPOINT_XFER_ISOC) {
			if (urb_priv->state == DWC2_URB_CANCELING)
				dwc2_urb_done(dwc, urb_priv->urb, urb_status);
			else { /* DWC2_TD_RETRY */
				urb_priv->error_count++;

				if (urb_priv->error_count == 3) {
					dwc2_urb_done(dwc, urb_priv->urb, urb_status);
				} else { /* retry */
					dwc2_urb_priv_reset(urb_priv, chan);
				}
			}
		} /* else ISOC, but isoc will never disable channel */
	}

	dwc2_trace_func_line((void *)__dwc2_disable_channel_stage2, __LINE__);
	dwc2_schedule(dwc);
}

/* caller must take care of lock&irq */
static void dwc2_disable_channel(struct dwc2 *dwc, struct dwc2_channel *chan,
				int urb_status, unsigned long flags) {
	int timeout = 0;

	__dwc2_disable_channel_stage1(dwc, chan, DWC2_URB_CANCELING);
	chan->waiting = 1;

	spin_unlock_irqrestore(&dwc->lock, flags);

	timeout = wait_event_timeout(chan->disable_wq, (chan->disable_stage == 2), HZ);
	WARN((timeout == 0), "wait channel%d disable timeout!\n", chan->number);

	spin_lock_irqsave(&dwc->lock, flags);
	chan->waiting = 0;
	__dwc2_disable_channel_stage2(dwc, chan, urb_status);
}

static void dwc2_schedule(struct dwc2 *dwc) {
	struct dwc2_qh		*qh;
	struct dwc2_qh		*qh_n;
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_channel	*channel;
	int			 ret;

	list_for_each_entry_safe(qh, qh_n, &dwc->idle_qh_list, list) {
		if (qh->type == USB_ENDPOINT_XFER_ISOC)
			continue;
		if (qh->pause_schedule || qh->disabling)
			continue;
		if (!list_empty(&qh->urb_list)) {
			urb_priv = list_first_entry(&qh->urb_list, struct dwc2_urb_priv, list);
			if (urb_priv->state == DWC2_URB_IDLE) {
				channel = dwc2_request_channel(dwc, urb_priv);
				if (!channel) {
					dwc2_trace_schedule_no_channel();
					break;
				}

				ret = dwc2_start_channel(dwc, channel);
				if (ret) {
					dwc2_free_channel(dwc, urb_priv->channel);
				} else {
					//list_add_tail(&channel->list, &dwc->chan_trans_list);
					urb_priv->state = DWC2_URB_TRANSFERING;
				}
			}
		}
	}
}


static int dwc2_hcd_urb_enqueue(struct usb_hcd *hcd,
				struct urb *urb, gfp_t mem_flags) {
	struct dwc2	*dwc = hcd_to_dwc2(hcd);
	unsigned long	 flags;
	struct dwc2_qh	*qh;
	int		 ret = 0;

	switch(usb_endpoint_type(&urb->ep->desc)) {
	case USB_ENDPOINT_XFER_INT:
	case USB_ENDPOINT_XFER_ISOC:
		if (usb_pipeout(urb->pipe)) {
			/* DWC HC do not support periodic out transfer */
			return -EINVAL;
		}
	}

	spin_lock_irqsave(&dwc->lock, flags);

	if (!dwc->device_connected) {
		ret = -ESHUTDOWN;
		goto done_not_linked;
	}

	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (unlikely(ret))
		goto done_not_linked;

	if (urb->ep->hcpriv) {
		qh = urb->ep->hcpriv;
		if (qh->disabling) {
			ret = -ENOENT;
			goto qh_disabling;
		}
	} else {
		qh = dwc2_qh_make(dwc, urb);
		if (!qh) {
			ret = -ENOMEM;
			goto err_no_qh;
		}
	}

	switch (qh->type) {
	case USB_ENDPOINT_XFER_CONTROL:
		ret = dwc2_submit_control(dwc, urb, qh);
		break;
	case USB_ENDPOINT_XFER_BULK:
		ret = dwc2_submit_bulk(dwc, urb, qh);
		break;
	case USB_ENDPOINT_XFER_INT:
		ret = dwc2_submit_interrupt(dwc, urb, qh);
		break;
	case USB_ENDPOINT_XFER_ISOC:
		urb->error_count = 0;
		ret = dwc2_submit_isochronous(dwc, urb, qh);
		break;
	}

	if (ret == 0) {
		if (urb->transfer_flags & URB_NO_INTERRUPT)
			dwc2_hcd_add_urb_context(dwc, urb->context);

		if (qh->type != USB_ENDPOINT_XFER_ISOC)
			dwc2_schedule(dwc);
		dwc->urb_queued_number++;
	}

qh_disabling:
err_no_qh:
	if (ret)
		usb_hcd_unlink_urb_from_ep(hcd, urb);
done_not_linked:
	dwc2_trace_urb_enqueue(urb, ret);
	spin_unlock_irqrestore(&dwc->lock, flags);

	return ret;
}

static int dwc2_hcd_urb_dequeue(struct usb_hcd *hcd,
				struct urb *urb, int status) {
	struct dwc2		*dwc = hcd_to_dwc2(hcd);
	struct dwc2_qh		*qh  = NULL;
	struct dwc2_urb_priv	*urb_priv;
	unsigned long		 flags;
	int			 ret;

	spin_lock_irqsave(&dwc->lock, flags);
	dwc2_trace_urb_dequeue(urb);

	ret = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (ret)
		goto done;

	qh = urb->ep->hcpriv;
	urb_priv = urb->hcpriv;

	if (!qh || !urb_priv) {
		if (!qh && urb_priv) {
			panic("urb %p ->hcpriv %p, but it's ep->hcpriv is NULL\n", urb, urb->hcpriv);
		}
		goto done;
	}

	if (qh->type != USB_ENDPOINT_XFER_ISOC) {
		if ((urb_priv->state == DWC2_URB_TRANSFERING)) {
			if (qh->type == USB_ENDPOINT_XFER_INT)
				dwc2_urb_priv_reset(urb_priv, urb_priv->channel);
			dwc2_disable_channel(dwc, urb_priv->channel, status, flags);
		} else if (urb_priv->state == DWC2_URB_IDLE)
			dwc2_urb_done(dwc, urb, status);
	} else {
		if ((urb_priv->state == DWC2_URB_TRANSFERING)) {
			/*
			 * we do not need to disable the channel, just cancel our TDs
			 * if this urb_priv is JUST complete successfully, but IOC is prevented by us
			 * we can safely reset and giveback this urb and when xfercompl got handled,
			 * it wan't see this urb, so nothing will happen
			 *
			 * if this urb_priv is partially transfering, cancel it is also safe
			 *
			 * if this urb_priv did not got a change to schedule, we are also safe to cancel
			 */
			dwc2_urb_priv_reset(urb_priv, qh->channel);
			/* if we are just transfering, wait a frame to let it done, so memory won't corrupted */
			mdelay(1);
			dwc2_urb_done(dwc, urb, status);
		} else if (urb_priv->state == DWC2_URB_IDLE) {
			dwc2_urb_done(dwc, urb, status);
		}
	}

done:
	spin_unlock_irqrestore(&dwc->lock, flags);
	return ret;
}

static void dwc2_hcd_endpoint_reset(struct usb_hcd *hcd,
				struct usb_host_endpoint *ep) {
	struct dwc2	*dwc	    = hcd_to_dwc2(hcd);
	struct dwc2_qh	*qh;
	int		 epnum	    = usb_endpoint_num(&ep->desc);
	int		 is_out	    = usb_endpoint_dir_out(&ep->desc);
	unsigned long	 flags;

	spin_lock_irqsave(&dwc->lock, flags);
	qh = ep->hcpriv;

	/*
	 * When an endpoint is reset by usb_clear_halt() we must reset
	 * the toggle bit in the QH.
	 */
	if (qh) {
		usb_settoggle(qh->dev, epnum, is_out, 0);
		qh->data_toggle = DWC_HCTSIZ_DATA0;

#if 0			      /* TODO: */
		if (!list_empty(&qh->td_list)) {
			WARN_ONCE(1, "clear_halt for a busy endpoint\n");
		} else {
			/* TODO */
		}
#endif
	}
	spin_unlock_irqrestore(&dwc->lock, flags);
}

static void dwc2_hcd_endpoint_disable(struct usb_hcd *hcd,
				struct usb_host_endpoint *ep) {
	struct dwc2	*dwc = hcd_to_dwc2(hcd);
	struct dwc2_qh	*qh;
	unsigned long	 flags;

	spin_lock_irqsave(&dwc->lock, flags);
	dwc2_trace_ep_disable(ep);
	qh = ep->hcpriv;
	if (!qh) {
		spin_unlock_irqrestore(&dwc->lock, flags);
		return;
	}

	if (!qh->disabling) {
		qh->disabling = 1;

		/* check if the QH has TDs transfering or canceling */
		if (qh->type != USB_ENDPOINT_XFER_ISOC) {
			int			 qh_transfering = 0;
			int			 qh_canceling	= 0;
			struct dwc2_urb_priv	*urb_priv;
			struct dwc2_urb_priv	*trans_urb_priv = NULL;

			if (!list_empty(&qh->urb_list)) {
				urb_priv = list_first_entry(&qh->urb_list, struct dwc2_urb_priv, list);
				if (urb_priv->state == DWC2_URB_TRANSFERING) {
					trans_urb_priv = urb_priv;
					qh_transfering = 1;
				} else if (urb_priv->state != DWC2_URB_IDLE)
					qh_canceling = 1;
			}

			if (qh_transfering) {
				dwc2_disable_channel(dwc, trans_urb_priv->channel, -ESHUTDOWN, flags);
			} else if (!(qh_transfering || qh_canceling)) /* idle */
				dwc2_qh_destroy(dwc, qh);
		} else {
			/*
			 * ISOC channel can't be disabling,
			 * we just give back all urbs, destroy this qh
			 */
			dwc2_qh_destroy(dwc, qh);
		}
	}

	spin_unlock_irqrestore(&dwc->lock, flags);
}

/*
 * Save the starting data toggle for the next transfer
 */
static void dwc2_hcd_save_data_toggle(struct dwc2_channel *chan)
{
	dwc_otg_hc_regs_t	*hc_regs;
	struct dwc2_qh		*qh = chan->urb_priv->owner_qh;
	hctsiz_data_t		 hctsiz;

	hc_regs = qh->dwc->host_if.hc_regs[chan->number];

	hctsiz.d32 = dwc_readl(&hc_regs->hctsiz);
	if (hctsiz.b.pid == DWC_HCTSIZ_DATA0) {
		qh->data_toggle = DWC_HCTSIZ_DATA0;
	} else {
		qh->data_toggle = DWC_HCTSIZ_DATA1;
	}
}

static void dwc2_td_done(struct dwc2_urb_priv *urb_priv, struct dwc2_td *td) {
	td->dma_desc_idx = -1;
	list_del(&td->list); /* del from td_list */
	list_add_tail(&td->list, &urb_priv->done_td_list);
}

/* handle Bulk/Control Interrupts */
static void dwc2_hcd_handle_bc_hc_intr(struct dwc2 *dwc, struct dwc2_channel *channel) {
	int			 chan_num      = channel->number;
	struct dwc2_host_if	*host_if       = &dwc->host_if;
	dwc_otg_hc_regs_t	*hc_regs       = host_if->hc_regs[chan_num];
	struct dwc2_urb_priv	*urb_priv      = channel->urb_priv;
	struct urb		*urb	       = urb_priv->urb;
	struct dwc2_td		*td;
	struct dwc2_td		*td_n;
	struct dwc2_qh		*qh	       = urb_priv->owner_qh;
	hcint_data_t		 hcint;
	int			 short_read    = 0;

	hcint.d32 = dwc_readl(&hc_regs->hcint);

	dwc2_trace_chan_intr(chan_num, hcint.d32,
			dwc_readl(&hc_regs->hcchar),
			dwc_readl(&hc_regs->hctsiz),
			dwc_readl(&hc_regs->hcdma));

	urb_priv->state = DWC2_URB_IDLE;

	if (hcint.b.chhltd) {
		if (!hcint.b.xfercomp) {
			int urb_status = -EILSEQ;

			if (hcint.b.stall) {
				urb_status = -EPIPE;
			}

			if (hcint.b.bblerr) {
				urb_status = -EOVERFLOW;
			}

			dwc2_free_channel(dwc, channel);

			urb_priv->error_count++;

			if (urb_priv->error_count == 3) {
				dwc2_urb_done(dwc, urb_priv->urb, urb_status);
			} else {
				/* retry */
				dwc2_urb_priv_reset(urb_priv, channel);
			}
		}


		dwc2_disable_hc_int(hc_regs, chhltd);
	}

	if (hcint.b.xfercomp) {
		if (qh->type == USB_ENDPOINT_XFER_CONTROL) {
			int is_in = !!usb_pipein(urb_priv->urb->pipe);
			switch (urb_priv->control_stage) {
			case EP0_SETUP_PHASE:
				if (urb_priv->urb->transfer_buffer_length) {
					urb_priv->control_stage = EP0_DATA_PHASE;
					qh->data_toggle = DWC_HCTSIZ_DATA1;
				} else {
					urb_priv->control_stage = EP0_STATUS_PHASE;
				}

				td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);
				dwc2_td_done(urb_priv, td);

				dwc2_free_channel(dwc, channel);

				break;

			case EP0_DATA_PHASE:
				list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
					u32	xfer_len;
					dwc_otg_host_dma_desc_t *dma_desc;

					if (td->dma_desc_idx < 0)
						break;

					dma_desc = channel->sw_desc_list + td->dma_desc_idx;
					xfer_len = td->len - dma_desc->status.b.n_bytes;
					if (is_in && dma_desc->status.b.n_bytes)
						short_read = 1;

					urb->actual_length += xfer_len;

					dwc2_td_done(urb_priv, td);

					if (short_read ||
						(urb->actual_length == urb->transfer_buffer_length)) {
						urb_priv->control_stage = EP0_STATUS_PHASE;
						break;
					}
				}

				dwc2_hcd_save_data_toggle(channel);

				dwc2_free_channel(dwc, channel);

				break;
			case EP0_STATUS_PHASE:
				td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);
				dwc2_td_done(urb_priv, td);

				dwc2_free_channel(dwc, channel);

				dwc2_urb_done(dwc, urb, 0);

				break;

			default:
				;
			}
		} else {
			list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
				u32	xfer_len;
				dwc_otg_host_dma_desc_t *dma_desc;

				if (td->dma_desc_idx < 0)
					break;

				dma_desc = channel->sw_desc_list + td->dma_desc_idx;
				xfer_len = td->len - dma_desc->status.b.n_bytes;
				if (qh->is_in && dma_desc->status.b.n_bytes)
					short_read = 1;

				urb->actual_length += xfer_len;

				dwc2_td_done(urb_priv, td);

				if (short_read ||
					(urb->actual_length == urb->transfer_buffer_length)) {
					break;
				}
			}

			dwc2_hcd_save_data_toggle(channel);

			dwc2_free_channel(dwc, channel);

			if (qh->is_in) { /* Bulk IN */
				if (short_read ||
					(urb->actual_length == urb->transfer_buffer_length)) {
					dwc2_urb_done(dwc, urb, 0);
				}
			} else { /* Bulk OUT */
				if (urb->actual_length == urb->transfer_buffer_length) {
					if (!list_empty(&urb_priv->td_list) &&
						(urb->transfer_flags & URB_ZERO_PACKET) &&
						!(urb->transfer_buffer_length % qh->mps)) {
						/* continue */
					} else {
						dwc2_urb_done(dwc, urb, 0);
					}
				}
			}
		}

		dwc2_disable_hc_int(hc_regs, xfercompl);
	}

	if (unlikely(hcint.b.ahberr)) {
		dev_err(dwc->dev, "DWC AHB error on channel%d\n", chan_num);
		BUG_ON(channel->disable_stage != 0);

		__dwc2_disable_channel_stage1(dwc, channel, DWC2_URB_RETRY);
		channel->waiting = 0;

		dwc2_disable_hc_int(hc_regs, ahberr);
	}
}

/* handle INT Interrupts */
static void dwc2_hcd_handle_int_hc_intr(struct dwc2 *dwc, struct dwc2_channel *channel) {
	int			 chan_num      = channel->number;
	struct dwc2_host_if	*host_if       = &dwc->host_if;
	dwc_otg_hc_regs_t	*hc_regs       = host_if->hc_regs[chan_num];
	struct dwc2_urb_priv	*urb_priv      = channel->urb_priv;
	struct urb		*urb	       = urb_priv->urb;
	struct dwc2_td		*td;
	hcint_data_t		 hcint;

	hcint.d32 = dwc_readl(&hc_regs->hcint);

	dwc2_trace_chan_intr(chan_num, hcint.d32,
			dwc_readl(&hc_regs->hcchar),
			dwc_readl(&hc_regs->hctsiz),
			dwc_readl(&hc_regs->hcdma));

	urb_priv->state = DWC2_URB_IDLE;

	if (hcint.b.chhltd) {
		if (!hcint.b.xfercomp) {
			int urb_status = -EILSEQ;

			if (hcint.b.stall) {
				urb_status = -EPIPE;
			}

			if (hcint.b.bblerr) {
				urb_status = -EOVERFLOW;
			}

			dwc2_free_channel(dwc, channel);

			urb_priv->error_count++;

			if (urb_priv->error_count == 3) {
				dwc2_urb_done(dwc, urb_priv->urb, urb_status);
			} else {
				/* retry */
				dwc2_urb_priv_reset(urb_priv, channel);
			}
		}


		dwc2_disable_hc_int(hc_regs, chhltd);
	}

	if (hcint.b.xfercomp) {
		u32			 xfer_len;
		dwc_otg_host_dma_desc_t *dma_desc;

		td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);
		dma_desc = channel->sw_desc_list + td->dma_desc_idx;
		xfer_len = td->len - dma_desc->status.b.n_bytes;

		dwc2_hcd_save_data_toggle(channel);
		dwc2_free_channel(dwc, channel);

		/* interrupt do not partial IN/OUT, retry if partial IN/OUT */
		if ( (!(urb->transfer_flags & URB_SHORT_NOT_OK)) ||
			(!dma_desc->status.b.n_bytes)) {
			urb->actual_length += xfer_len;
			dwc2_td_done(urb_priv, td);
			dwc2_urb_done(dwc, urb, 0);
		}

		dwc2_disable_hc_int(hc_regs, xfercompl);
	}

	if (hcint.b.bna) {
		printk("==============>BNA!!!!!!!!\n");
	}

	if (unlikely(hcint.b.ahberr)) {
		dev_err(dwc->dev, "DWC AHB error on channel%d\n", chan_num);
		BUG_ON(channel->disable_stage != 0);

		__dwc2_disable_channel_stage1(dwc, channel, DWC2_URB_RETRY);
		channel->waiting = 0;

		dwc2_disable_hc_int(hc_regs, ahberr);
	}
}

//#define DWC2_DEBUG_DESCRIPTOR_HANDLE

/* Handle ISOC Interrupts */
static void dwc2_hcd_handle_isoc_hc_intr(struct dwc2 *dwc, struct dwc2_channel *channel) {
	int			 chan_num = channel->number;
	struct dwc2_host_if	*host_if  = &dwc->host_if;
	dwc_otg_hc_regs_t	*hc_regs  = host_if->hc_regs[chan_num];
	struct dwc2_qh		*qh	  = channel->qh;
	struct dwc2_urb_priv	*urb_priv;
	struct dwc2_td		*td;
	struct dwc2_td		*td_n;
	hcint_data_t		 hcint;

	if (!qh)
		return;

	hcint.d32 = dwc_readl(&hc_regs->hcint);

	dwc2_trace_chan_intr(chan_num, hcint.d32,
			dwc_readl(&hc_regs->hcchar),
			dwc_readl(&hc_regs->hctsiz),
			dwc_readl(&hc_regs->hcdma));

	if (hcint.b.xfercomp || hcint.b.chhltd) {
		dwc_otg_host_dma_desc_t			*dma_desc	     = NULL;
		struct usb_iso_packet_descriptor	*isoc_desc;
		int					 done		     = 0;
		int					 urb_xfer_compl_hint = 0;
#ifdef DWC2_DEBUG_DESCRIPTOR_HANDLE
		int					 handled_desc_count = 0;
#endif

		while (!list_empty(&qh->urb_list)) {
			urb_priv = list_first_entry(&qh->urb_list, struct dwc2_urb_priv, list);

			/* get the status of the last td */
			urb_xfer_compl_hint = 0;
			td = list_entry(urb_priv->td_list.prev, struct dwc2_td, list);
			if (td->dma_desc_idx >= 0) {
				dma_desc = channel->sw_desc_list + td->dma_desc_idx;
				urb_xfer_compl_hint = !dma_desc->status.b_isoc.a;
			}

			list_for_each_entry_safe(td, td_n, &urb_priv->td_list, list) {
				if (td->dma_desc_idx < 0) {
					done = 1;
					break;
				}

				dma_desc = channel->sw_desc_list + td->dma_desc_idx;

				if (unlikely(dma_desc->status.b_isoc.a && urb_xfer_compl_hint)) {
					/* this desc is active, but the whole urb is complete! */
					dma_desc->status.b_isoc.a = 0;
					dma_desc->status.b_isoc.sts = DMA_DESC_STS_PKTERR;
				}

				if (dma_desc->status.b_isoc.a && (dma_desc->status.b_isoc.sts == 0)) {
					done = 1;
					break;
				}

				isoc_desc = urb_priv->urb->iso_frame_desc + td->isoc_idx;
				isoc_desc->status = dma_desc->status.b_isoc.sts ? -EPROTO : 0;
				isoc_desc->actual_length = td->len - dma_desc->status.b_isoc.n_bytes;

#ifdef DWC2_DEBUG_DESCRIPTOR_HANDLE
				handled_desc_count++;
#endif
				dwc2_td_done(urb_priv, td);
			}

			if (list_empty(&urb_priv->td_list)) {
				dwc2_urb_done(dwc, urb_priv->urb, 0);
			}

#ifdef DWC2_DEBUG_DESCRIPTOR_HANDLE
			if (unlikely(hcint.b.xfercomp && (handled_desc_count == 0))) {
				struct dwc2_urb_priv	*m_urb_priv;

				printk(KERN_DEBUG "hcint = 0x%08x handled_desc_count = %d list=%d done=%d \n",
					hcint.d32, handled_desc_count, !list_empty(&qh->urb_list), done);

				if (dma_desc) {
					printk(KERN_DEBUG "dma_desc status: a=%u sts=%u ioc=%u n_bytes=%u qh->mps=%u\n",
						dma_desc->status.b_isoc.a,
						dma_desc->status.b_isoc.sts,
						dma_desc->status.b_isoc.ioc,
						dma_desc->status.b_isoc.n_bytes,
						qh->mps);
				}

				printk(KERN_DEBUG "========all descs=========\n");
				list_for_each_entry(m_urb_priv, &qh->urb_list, list) {
					printk(KERN_DEBUG "     =====urb=%p=====\n", m_urb_priv->urb);
					list_for_each_entry_safe(td, td_n, &m_urb_priv->td_list, list) {
						if (td->dma_desc_idx < 0) {
							break;
						}
						dma_desc = channel->sw_desc_list + td->dma_desc_idx;

						printk(KERN_DEBUG "dma_desc[%u] status: a=%u sts=%u ioc=%u n_bytes=%u\n",
							td->dma_desc_idx,
							dma_desc->status.b_isoc.a,
							dma_desc->status.b_isoc.sts,
							dma_desc->status.b_isoc.ioc,
							dma_desc->status.b_isoc.n_bytes);
					}
				}
			}
#endif

			if (done)
				break;
		}
	}

	if (hcint.b.chhltd) {
		printk(KERN_DEBUG"======>%s: chhltd ERROR!!!\n", __func__);
		/* reset scheduled urbs */
		while (!list_empty(&qh->urb_list)) {
			urb_priv = list_first_entry(&qh->urb_list, struct dwc2_urb_priv, list);
			if (!list_empty(&urb_priv->done_td_list)) {
				dwc2_urb_done(dwc, urb_priv->urb, -EREMOTEIO);
			} else {
				td = list_first_entry(&urb_priv->td_list, struct dwc2_td, list);
				if (td->dma_desc_idx >= 0) {
					dwc2_urb_priv_reset(urb_priv, channel);
				} else {
					break;
				}
			}
		}

		dwc2_start_isoc_channel(dwc, qh->channel);
	}

	if (unlikely(hcint.b.ahberr)) {
		/* TODO */
		printk("======>%s: AHB ERROR!!!\n", __func__);
	}

	dwc2_isoc_schedule(dwc, qh);
}

/** Handles interrupt for a specific Host Channel */
void dwc2_hcd_handle_hc_n_intr(struct dwc2 *dwc, int num) {
	struct dwc2_channel	*channel       = dwc->channel_pool + num;
	dwc_otg_hc_regs_t	*hc_regs       = dwc->host_if.hc_regs[num];

	if (channel->disable_stage == 1) {
		/*
		 * ok, we are in stage1,
		 * there's a gap between stage1 and stage2,
		 * all resources associate with this channel may have been freed
		 * in this gap, so pls take care full when handling stage2
		 */
		dwc_writel(0xFFFFFFFF, &hc_regs->hcint);

		if (channel->waiting) { /* from endpoint_disable or urb_dequeue */
			channel->disable_stage = 2;
			wake_up(&channel->disable_wq);
		} else {
			/* from AHB error */
			__dwc2_disable_channel_stage2(dwc, channel, -EIO);
		}
	} else {
		int need_schedule = 1;
		struct dwc2_qh *qh = NULL;

		if (channel->qh)
			qh = channel->qh;
		else if (channel->urb_priv) {
			qh = channel->urb_priv->owner_qh;
		}

		if (qh) {
			switch (qh->type) {
			case USB_ENDPOINT_XFER_CONTROL:
			case USB_ENDPOINT_XFER_BULK:
				dwc2_hcd_handle_bc_hc_intr(dwc, channel);
				break;
			case USB_ENDPOINT_XFER_INT:
				dwc2_hcd_handle_int_hc_intr(dwc, channel);
				break;
			case USB_ENDPOINT_XFER_ISOC:
				dwc2_hcd_handle_isoc_hc_intr(dwc, channel);
				need_schedule = 0;
				break;
			}
		}

		dwc_writel(0xFFFFFFFF, &hc_regs->hcint);

		if (need_schedule) {
			dwc2_trace_func_line((void *)dwc2_hcd_handle_hc_n_intr, __LINE__);
			dwc2_schedule(dwc);
		}
	}
}

static void dwc2_hcd_handle_hc_intr(struct dwc2 *dwc) {
	int i;
	haint_data_t haint;

	haint.d32 = dwc_readl(&dwc->host_if.host_global_regs->haint);

	dwc2_trace_haint(haint.d32);

	for (i = 0; i < MAX_EPS_CHANNELS; i++) {
		if (haint.b2.chint & (1 << i)) {
			dwc2_hcd_handle_hc_n_intr(dwc, i);
		}
	}
}

static void dwc2_host_cleanup(struct dwc2 *dwc) {
	struct dwc2_qh		*qh;
	struct dwc2_qh		*qh_temp;
	int			 i;
	struct dwc2_channel	*chan;

	/*
	 * the simplest way maybe tail the upper layer using port1_status
	 * but when the user also removed the cable, the host may change to device mode,
	 * in device mode, host registers are not accessible, so we must giveback any queued URBs ASAP
	 */

	/*
	 * we must first set the qh->disabling to 1 because dwc2_urb_done()
	 * will temperally unlock dwc->lock, and cause some funny things
	 */

	list_for_each_entry(qh, &dwc->busy_qh_list, list) {
		if (!qh->disabling)
			qh->disabling = 1;
	}

	for (i = 0; i < MAX_EPS_CHANNELS; i++) {
		chan = dwc->channel_pool + i;
		if (chan->urb_priv) { /* Bulk/Control/INT */
			qh = chan->urb_priv->owner_qh;
			if (chan->disable_stage == 0) {
				__dwc2_disable_channel_stage1(dwc, chan, DWC2_URB_CANCELING);
				chan->waiting = 0;
			}

			if (qh->type == USB_ENDPOINT_XFER_INT) {
				/*
				 * there seem a BUG in DWC HC, when Device Disconnect,
				 * frame number freezed, and periodic transfer frozon
				 *
				 * Note that ISOC will actually not got here
				 */
				dwc2_qh_destroy(dwc, qh);
			}
		} else if (chan->qh) { /* ISOC */
			dwc2_qh_destroy(dwc, chan->qh);
		}
	}

	list_for_each_entry_safe(qh, qh_temp, &dwc->idle_qh_list, list) {
		// printk("%s: qh %p is idle\n", __func__, qh);
		dwc2_qh_destroy(dwc, qh);
	}

	dwc->mode = DWC2_HC_MODE_UNKNOWN;
}

/* A-Cable still connected but device disconnected. */
void dwc2_hcd_handle_device_disconnect_intr(struct dwc2 *dwc) {
	dwc->device_connected = 0;

	/*
	 * Turn off the vbus power only if the core has transitioned to device
	 * mode. If still in host mode, need to keep power on to detect a
	 * reconnection.
	 */
	if (unlikely(dwc2_is_device_mode(dwc))) {
		printk("===========>TODO: enter %s: Device Mode\n", __func__);
	} else {
		hprt0_data_t	hprt0;

		dwc2_host_cleanup(dwc);

		/* TODO: if currently in A-Peripheral mode, do we need report HUB status? */

		dwc->port1_status |= (USB_PORT_STAT_C_CONNECTION << 16);
		dwc->device_connected = !!hprt0.b.prtconnsts;

		if (dwc->hcd->status_urb)
			usb_hcd_poll_rh_status(dwc->hcd);
		else
			usb_hcd_resume_root_hub(dwc->hcd);
	}
}

extern void dwc2_hcd_handle_port_intr(struct dwc2 *dwc);
void dwc2_handle_host_mode_interrupt(struct dwc2 *dwc, gintsts_data_t *gintr_status) {
	if (gintr_status->b.hcintr) {
		dwc2_hcd_handle_hc_intr(dwc);
		gintr_status->b.hcintr = 0;
	}

	if (gintr_status->b.portintr) {
		dwc2_hcd_handle_port_intr(dwc);
		gintr_status->b.portintr = 0;
	}
}

static int dwc2_hcd_start(struct usb_hcd *hcd) {
	struct dwc2	*dwc = hcd_to_dwc2(hcd);
	int		 i;
	size_t		 size;
	int		 ret = 0;

	dwc->sw_frame_list = dma_alloc_coherent(dwc->dev, MAX_FRLIST_EN_NUM * sizeof(u32),
						&dwc->hw_frame_list, GFP_KERNEL);
	if (!dwc->sw_frame_list) {
		dev_err(dwc->dev, "failed to allocate host DDMA frame list\n");
		return -ENOMEM;
	}

	memset(dwc->sw_frame_list, 0, MAX_FRLIST_EN_NUM * sizeof(u32));

	dwc->qh_cachep = kmem_cache_create("dwc2_qh",
					sizeof(struct dwc2_qh), 0, 0, NULL);
	if (!dwc->qh_cachep) {
		dev_err(dwc->dev, "failed to create qh cache\n");
		ret = -ENOMEM;
		goto fail_qh_cachep;
	}

	dwc->td_cachep = kmem_cache_create("dwc2_td",
					sizeof(struct dwc2_td), 0, 0, NULL);
	if (!dwc->td_cachep) {
		dev_err(dwc->dev, "failed to create td cache\n");
		ret = -ENOMEM;
		goto fail_td_cachep;
	}

	size = max(sizeof(struct dwc2_urb_context), sizeof(struct dwc2_urb_priv));
	size = max(size, sizeof(struct dwc2_isoc_qh_ptr));

	dwc->context_cachep = kmem_cache_create("dwc2_urb_context", size, 0, 0, NULL);
	if (!dwc->context_cachep) {
		dev_err(dwc->dev, "failed to create context cache\n");
		ret = -ENOMEM;
		goto fail_context_cachep;
	}

	if (dwc2_channel_pool_init(dwc) < 0) {
		goto fail_init_chan_pool;
	}

	if (dwc2_isoc_qh_ptr_pool_init(dwc)) {
		goto fail_isoc_qh_ptr;
	}

	for (i = 0; i < MAX_FRLIST_EN_NUM; i++) {
		struct dwc2_frame *dwc_frm = dwc->frame_list + i;

		memset(dwc_frm, 0, sizeof(struct dwc2_frame));
		INIT_LIST_HEAD(&dwc_frm->isoc_qh_list);
	}

	dwc->port1_status = 0;

	hcd->state = HC_STATE_RUNNING;
	/* NOTE: The core is always in Device Mode at this time,
	 * so we do not need to configure HW here
	 */

	dwc->hcd_started = 1;

	return 0;

fail_isoc_qh_ptr:
	dwc2_channel_pool_fini(dwc);

fail_init_chan_pool:
	kmem_cache_destroy(dwc->context_cachep);

fail_context_cachep:
	kmem_cache_destroy(dwc->td_cachep);

fail_td_cachep:
	kmem_cache_destroy(dwc->qh_cachep);

fail_qh_cachep:
	dma_free_coherent(dwc->dev, MAX_FRLIST_EN_NUM * sizeof(u32),
			dwc->sw_frame_list, dwc->hw_frame_list);

	return ret;
}

extern int dwc2_rh_hub_status_data(struct usb_hcd *hcd, char *buf);
extern int dwc2_rh_hub_control(struct usb_hcd *hcd,
			u16 typeReq, u16 wValue, u16 wIndex,
			char *buf, u16 wLength);
extern int dwc2_rh_bus_suspend(struct usb_hcd *hcd);
extern int dwc2_rh_bus_resume(struct usb_hcd *hcd);

static const struct hc_driver dwc2_hc_driver = {
	.description =		"dwc2-hcd",
	.product_desc =		"DesignWare USB2.0 High-Speed Host Controller",
	.hcd_priv_size =	sizeof(struct dwc2 *), /* a pointer to struct dwc2 */

	/*
	 * not using irq handler or reset hooks from usbcore, since
	 * those must be shared with peripheral code for OTG configs
	 */

	.flags =		HCD_MEMORY | HCD_USB2,

	/* called to init HCD and root hub */
	.start =		dwc2_hcd_start,

	/* cleanly make HCD stop writing memory and doing I/O */
	.stop =			dwc2_hcd_stop,

	/* return current frame number */
	.get_frame_number =	dwc2_hcd_get_frame_number,

	/*
	 * Managing i/o requests and associated device resources
	 */
	.urb_enqueue =		dwc2_hcd_urb_enqueue,
	.urb_dequeue =		dwc2_hcd_urb_dequeue,
	.endpoint_reset =	dwc2_hcd_endpoint_reset,
	.endpoint_disable =	dwc2_hcd_endpoint_disable,

	/* root hub support */
	.hub_status_data =	dwc2_rh_hub_status_data,
	.hub_control =		dwc2_rh_hub_control,
	.bus_suspend =		dwc2_rh_bus_suspend,
	.bus_resume =		dwc2_rh_bus_resume,
};

int dwc2_host_init(struct dwc2 *dwc) {
	struct usb_hcd *hcd;
	int ret;

	hcd = usb_create_hcd(&dwc2_hc_driver, dwc->dev, dev_name(dwc->dev));
	if (hcd == NULL) {
		dev_err(dwc->dev, "Cannot create hcd\n");
		return -ENOMEM;
	}
	/*
	 * usb_create_hcd() will call dev_set_drvdata(dev, hcd), but nobody call get_drvdata
	 * we need this drvdata so that glue layer can get our struct dwc2
	 */
	dev_set_drvdata(dwc->dev, dwc);

	hcd->has_tt = 1;
	hcd->uses_new_polling = 1;
	hcd->self.otg_port = 1;
	hcd->self.sg_tablesize = 0;

	INIT_LIST_HEAD(&dwc->context_list);
	INIT_LIST_HEAD(&dwc->idle_qh_list);
	INIT_LIST_HEAD(&dwc->busy_qh_list);
	dwc->hcd_started = 0;

	*(struct dwc2 **)(hcd->hcd_priv) = dwc;
	dwc->hcd = hcd;

	ret = usb_add_hcd(hcd, -1, 0);
	if (ret != 0) {
		dev_err(dwc->dev, "usb_add_hcd returned %d\n", ret);
		usb_put_hcd(hcd);
		return ret;
	}

	dev_info(dwc->dev, "DWC2 Host Initialized\n");

	return 0;
}

void dwc2_host_exit(struct dwc2 *dwc) {
	usb_remove_hcd(dwc->hcd);
	usb_put_hcd(dwc->hcd);

	dwc2_isoc_qh_ptr_fini(dwc);
	dwc2_channel_pool_fini(dwc);

	kmem_cache_destroy(dwc->context_cachep);
	kmem_cache_destroy(dwc->td_cachep);
	kmem_cache_destroy(dwc->qh_cachep);
	dma_free_coherent(dwc->dev, MAX_FRLIST_EN_NUM * sizeof(u32),
			dwc->sw_frame_list, dwc->hw_frame_list);
}
