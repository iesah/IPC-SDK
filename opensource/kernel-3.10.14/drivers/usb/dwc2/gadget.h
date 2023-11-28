
#ifndef __DRIVERS_USB_DWC3_GADGET_H
#define __DRIVERS_USB_DWC3_GADGET_H

#include <linux/list.h>
#include <linux/usb/gadget.h>

#define DWC_NUMBER_OF_HB_EP	1

struct dwc2;
#define to_dwc2_ep(ep)		(container_of(ep, struct dwc2_ep, usb_ep))
#define gadget_to_dwc(g)	(container_of(g, struct dwc2, gadget))

static inline struct dwc2_ep *dwc2_ep0_get_in_ep(struct dwc2 *dwc) {
	return dwc->eps[dwc->dev_if.num_out_eps];
}

static inline struct dwc2_ep *dwc2_ep0_get_out_ep(struct dwc2 *dwc) {
	return dwc->eps[0];
}

static inline struct dwc2_ep *dwc2_ep0_get_ep0(struct dwc2 *dwc) {
	return dwc2_ep0_get_out_ep(dwc);
}

static inline struct dwc2_ep *dwc2_ep0_get_ep_by_dir(struct dwc2 *dwc, int is_in) {
	if (is_in)
		return dwc2_ep0_get_in_ep(dwc);
	else
		return dwc2_ep0_get_out_ep(dwc);
}

#define to_dwc2_request(r)	(container_of(r, struct dwc2_request, request))

static inline struct dwc2_request *next_request(struct list_head *list)
{
	if (list_empty(list))
		return NULL;

	return list_first_entry(list, struct dwc2_request, list);
}

static inline int need_send_zlp(struct dwc2_request *req) {
	struct dwc2_ep *dep = req->dwc2_ep;

	return ( (req->request.zero) &&
		(req->request.length != 0) &&
		(!req->zlp_transfered) &&
		((req->request.length % dep->maxp) == 0));
}

void dwc2_device_mode_init(struct dwc2 *dwc);
void dwc2_gadget_do_pullup(struct dwc2 *dwc);

void dwc2_gadget_giveback(struct dwc2_ep *dep, struct dwc2_request *req,
			int status);

int __dwc2_gadget_ep_set_halt(struct dwc2_ep *dep, int value);

int dwc2_gadget_ep0_set_halt(struct usb_ep *ep, int value);
int dwc2_gadget_ep0_queue(struct usb_ep *ep, struct usb_request *request,
			gfp_t gfp_flags);
void dwc2_ep0_interrupt(struct dwc2_ep *dep);

void dwc2_ep0_out_start(struct dwc2 *dwc);

void dwc2_delayed_status_watchdog(unsigned long _dwc);

void dwc2_handle_device_mode_interrupt(struct dwc2 *dwc, gintsts_data_t *gintr_status);
void dwc2_gadget_handle_session_end(struct dwc2 *dwc);

#ifdef CONFIG_USB_DWC2_HOST_ONLY
static inline int dwc2_has_ep_enabled(struct dwc2 *dwc) { return 0; };
#else
int dwc2_has_ep_enabled(struct dwc2 *dwc);
#endif
#endif /* __DRIVERS_USB_DWC3_GADGET_H */
