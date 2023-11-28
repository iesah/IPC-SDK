#ifndef __DRIVERS_USB_DWC2_HOST_H
#define __DRIVERS_USB_DWC2_HOST_H

#include <linux/usb.h>
#include <linux/usb/hcd.h>

#define USB_PORT_STATE_RESUME  (1 << 31)

struct dwc2;

static inline struct dwc2 *hcd_to_dwc2(struct usb_hcd *hcd)
{
	return *(struct dwc2 **)(hcd->hcd_priv);
}

static inline int dwc2_host_has_int_isoc(struct dwc2 *dwc) {
	return (dwc->hcd->self.bandwidth_int_reqs ||
		dwc->hcd->self.bandwidth_isoc_reqs);
}

void dwc2_host_mode_init(struct dwc2 *dwc);

/**
 * This function Reads HPRT0 in preparation to modify. It keeps the
 * WC bits 0 so that if they are read as 1, they won't clear when you
 * write it back
 */
static inline uint32_t dwc2_hc_read_hprt(struct dwc2 *dwc)
{
	hprt0_data_t hprt0;

	hprt0.d32 = dwc_readl(dwc->host_if.hprt0);
	hprt0.b.prtena = 0;
	hprt0.b.prtconndet = 0;
	hprt0.b.prtenchng = 0;
	hprt0.b.prtovrcurrchng = 0;

	return hprt0.d32;
}

#define dwc2_disable_hc_int(_hc_regs_, _intr_)				\
	do {								\
		hcintmsk_data_t hcintmsk;				\
		hcintmsk.d32 = dwc_readl(&(_hc_regs_)->hcintmsk);	\
		hcintmsk.b._intr_ = 0;					\
		dwc_writel(hcintmsk.d32, &(_hc_regs_)->hcintmsk);	\
	} while (0)

void dwc2_handle_host_mode_interrupt(struct dwc2 *dwc, gintsts_data_t *gintr_status);
void dwc2_hcd_handle_device_disconnect_intr(struct dwc2 *dwc);

void __dwc2_port_reset(struct dwc2 *dwc, bool do_reset);

#endif	/* __DRIVERS_USB_DWC2_HOST_H */
