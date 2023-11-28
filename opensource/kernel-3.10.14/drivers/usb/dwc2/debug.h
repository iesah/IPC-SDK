#ifndef __DRIVERS_USB_DWC2_DEBUG_H
#define __DRIVERS_USB_DWC2_DEBUG_H

#include "core.h"

#ifdef CONFIG_DEBUG_FS
extern int dwc2_debugfs_init(struct dwc2 *);
extern void dwc2_debugfs_exit(struct dwc2 *);
#else
static inline int dwc2_debugfs_init(struct dwc2 *d)
{  return 0;  }
static inline void dwc2_debugfs_exit(struct dwc2 *d)
{  }
#endif

#ifdef CONFIG_USB_DWC2_HOST_TRACER
extern int req_trace_enable;
extern void __dwc2_host_trace_print(struct dwc2 *dwc, int nolock);

extern void dwc2_trace_func_line(void *func, int line);
extern void dwc2_trace_urb_enqueue(struct urb * urb, int ret);
extern void dwc2_trace_urb_dequeue(struct urb * urb);
extern void dwc2_trace_ep_disable(struct usb_host_endpoint *ep);
extern void dwc2_trace_new_qh(struct dwc2_qh *qh);
extern void dwc2_trace_del_qh(struct dwc2_qh *qh);
extern void dwc2_trace_urb_submit(struct urb *urb, struct dwc2_qh *qh);
extern void dwc2_trace_sof_dis(void);
extern void dwc2_trace_sof_en(unsigned short frame);
extern void dwc2_trace_enter_schedule(int has_int_isoc, u32 last_frame,
				u32 curr_frame, int sof_intr_enabled);
extern void dwc2_trace_frame_missing(unsigned short last_frame, unsigned short curr_frame);
extern void dwc2_trace_clean_up_chan(struct dwc2_channel *chan, unsigned short frame_num);
extern void dwc2_trace_enter_schedule_async(unsigned short frame, unsigned short usecs_left);
extern void dwc2_trace_enter_schedule_periodic(unsigned short frame, unsigned short usecs_left);
extern void dwc2_trace_schedule_no_channel(void);
extern void dwc2_trace_sched_qh_state(struct dwc2_qh *qh);
extern void dwc2_trace_leave_schedule(unsigned short last_scheduled_frame);
extern void dwc2_trace_leave_schedule_async(unsigned short frame, unsigned short usecs_left);
extern void dwc2_trace_leave_schedule_periodic(unsigned short frame, unsigned short usecs_left);
extern void dwc2_trace_start_channel(struct dwc2_urb_priv *urb_priv,
				struct dwc2_qh *qh, struct dwc2_channel *chan);
extern void dwc2_trace_chan_trans_info(
	struct dwc2 *dwc,
	struct dwc2_channel *channel,
	int ctrl_stage,
	int error_count,
	u32 hcchar,
	u32 hcsplt,
	u32 hctsiz);

extern void dwc2_trace_gintsts(u32 gintsts, u32 gintmsk);
extern void dwc2_trace_haint(u32 haint);
extern void dwc2_trace_chan_intr(
	int chan_num,
	u32 hcint,
	u32 hcchar,
	u32 hcsplt,
	u32 hctsiz);

extern void dwc2_trace_urb_done(struct urb *urb, int status);

extern void dwc2_trace_qh_schedule_info(struct dwc2_qh *qh, int status);
extern void dwc2_trace_isoc_schedule(struct dwc2 *dwc, struct dwc2_urb_priv *urb_priv,
			unsigned short curr_frame, unsigned short start_frame,
			unsigned short request_slots);
extern void dwc2_trace_isoc_urb_priv_slots(struct dwc2_urb_priv *urb_priv);
extern void dwc2_trace_clear_urb_hcpriv(struct urb *urb);

#else  /* !CONFIG_USB_DWC2_HOST_TRACE */

static __attribute__((unused)) void __dwc2_host_trace_print(struct dwc2 *dwc, int nolock){}

static __attribute__((unused)) void dwc2_trace_func_line(void *func, int line){}
static __attribute__((unused)) void dwc2_trace_urb_enqueue(struct urb * urb, int ret){}
static __attribute__((unused)) void dwc2_trace_urb_dequeue(struct urb * urb){}
static __attribute__((unused)) void dwc2_trace_ep_disable(struct usb_host_endpoint *ep){}
static __attribute__((unused)) void dwc2_trace_new_qh(struct dwc2_qh *qh){}
static __attribute__((unused)) void dwc2_trace_del_qh(struct dwc2_qh *qh){}
static __attribute__((unused)) void dwc2_trace_urb_submit(struct urb *urb, struct dwc2_qh *qh){}
static __attribute__((unused)) void dwc2_trace_sof_dis(void){}
static __attribute__((unused)) void dwc2_trace_sof_en(unsigned short frame){}
static __attribute__((unused)) void dwc2_trace_enter_schedule(int has_int_isoc, u32 last_frame,
				u32 curr_frame, int sof_intr_enabled){}
static __attribute__((unused)) void dwc2_trace_frame_missing(unsigned short last_frame, unsigned short curr_frame){}
static __attribute__((unused)) void dwc2_trace_clean_up_chan(struct dwc2_channel *chan, unsigned short frame_num){}
static __attribute__((unused)) void dwc2_trace_enter_schedule_async(unsigned short frame, unsigned short usecs_left){}
static __attribute__((unused)) void dwc2_trace_enter_schedule_periodic(unsigned short frame, unsigned short usecs_left){}
static __attribute__((unused)) void dwc2_trace_schedule_no_channel(void){}
static __attribute__((unused)) void dwc2_trace_sched_qh_state(struct dwc2_qh *qh){}
static __attribute__((unused)) void dwc2_trace_leave_schedule(unsigned short last_scheduled_frame){}
static __attribute__((unused)) void dwc2_trace_leave_schedule_async(unsigned short frame, unsigned short usecs_left){}
static __attribute__((unused)) void dwc2_trace_leave_schedule_periodic(unsigned short frame, unsigned short usecs_left){}
static __attribute__((unused)) void dwc2_trace_start_channel(struct dwc2_urb_priv *urb_priv,
				struct dwc2_qh *qh, struct dwc2_channel *chan){}
static __attribute__((unused)) void dwc2_trace_chan_trans_info(
	struct dwc2 *dwc,
	struct dwc2_channel *channel,
	int ctrl_stage,
	int error_count,
	u32 hcchar,
	u32 hcsplt,
	u32 hctsiz){}

static __attribute__((unused)) void dwc2_trace_gintsts(u32 gintsts, u32 gintmsk){}
static __attribute__((unused)) void dwc2_trace_haint(u32 haint){}
static __attribute__((unused)) void dwc2_trace_chan_intr(
	int chan_num,
	u32 hcint,
	u32 hcchar,
	u32 hcsplt,
	u32 hctsiz){}

static __attribute__((unused)) void dwc2_trace_urb_done(struct urb *urb, int status){}

static __attribute__((unused)) void dwc2_trace_qh_schedule_info(struct dwc2_qh *qh, int status){}
static __attribute__((unused)) void dwc2_trace_isoc_schedule(struct dwc2 *dwc, struct dwc2_urb_priv *urb_priv,
				unsigned short curr_frame, unsigned short start_frame,
				unsigned short request_slots){}
static __attribute__((unused)) void dwc2_trace_isoc_urb_priv_slots(struct dwc2_urb_priv *urb_priv){}
static __attribute__((unused)) void dwc2_trace_clear_urb_hcpriv(struct urb *urb){}
#endif

#endif	/* __DRIVERS_USB_DWC2_DEBUG_H */
