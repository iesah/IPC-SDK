#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/ptrace.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/usb/ch9.h>

#include "core.h"
#include "gadget.h"
#include "debug.h"

#ifdef CONFIG_USB_DWC2_HOST_TRACER

enum dwc2_trace_type {
	DWC_TRACE_FUNC_LINE,
	DWC_TRACE_URB_ENQUEUE,
	DWC_TRACE_URB_DEQUEUE,
	DWC_TRACE_EP_DISABLE,
	DWC_TRACE_NEW_QH,
	DWC_TRACE_DEL_QH,
	DWC_TRACE_URB_SUBMIT,
	DWC_TRACE_SOF_EN,
	DWC_TRACE_SOF_DIS,
	DWC_TRACE_ENTER_SCHEDULE,
	DWC_TRACE_FRAME_MISSING,
	DWC_TRACE_CLEAN_UP_CHAN,
	DWC_TRACE_ENTER_SCHEDULE_ASYNC,
	DWC_TRACE_ENTER_SCHEDULE_PERIODIC,
	DWC_TRACE_SCHEDULE_NO_CHANNEL,
	DWC_TRACE_SCHED_QH_STATE,
	DWC_TRACE_LEAVE_SCHEDULE,
	DWC_TRACE_LEAVE_SCHEDULE_ASYNC,
	DWC_TRACE_LEAVE_SCHEDULE_PERIODIC,
	DWC_TRACE_START_CHANNEL,
	DWC_TRACE_CHAN_TRANS_INFO,
	DWC_TRACE_GINTSTS,
	DWC_TRACE_HAINT,
	DWC_TRACE_CHANNEL_INTR,
	DWC_TRACE_URB_DONE,
	DWC_TRACE_QH_SCHED_INFO,
	DWC_TRACE_ISOC_SCHEDULE,
	DWC_TRACE_ISOC_URB_PRIV_SLOTS,
	DWC_TRACE_CLEAR_URB_HCPRIV,
	DWC_TRACE_MAX
};

struct dt_func_line {
	void	*func;
	int	 line;
};

struct dt_urb_enqueue {
	struct urb	*urbp;
	struct urb	 urb;
	int		 eptype;
	u32		 speed;
	u32		 mps;
	int		 ret;
};

struct dt_urb_dequeue {
	struct urb *urbp;
};

struct dt_ep_disable {
	int num;
	int is_in;
	void *hcpriv;
};

struct dt_new_qh {
	struct dwc2_qh *qh;
	int mps;
	u8 hub_addr;
	u8 port_number;
};

struct dt_del_qh {
	struct dwc2_qh *qh;
};

struct dt_urb_submit {
	struct urb_priv *urb_priv;
	struct urb *urb;
	struct dwc2_qh *qh;
};

struct dt_sof_en {
	unsigned short exp_frame;
};

struct dt_enter_schedule {
	int has_int_isoc;
	int sof_intr_enabled;
	u32 last_frame;
	u32 curr_frame;
	int cpu_id;
	u64 cpu_clk[2];
};

struct dt_frame_missing {
	unsigned short last_frame;
	unsigned short curr_frame;
};

struct dt_clean_up_chan {
	int frame_num;
	int chan_num;
};

struct dt_enter_schedule_async {
	int  cpu_id;
	u64  cpu_clk[2];
	unsigned short frame;
	unsigned short usecs_left;
};

struct dt_enter_schedule_periodic {
	int  cpu_id;
	u64  cpu_clk[2];
	unsigned short frame;
	unsigned short usecs_left;
};

struct dt_sched_qh_state {
	struct dwc2_qh *qh;
	int list_state; /* 0: empty, otherwise 1 */
	int disabling;
	int first_urb_priv_state;
};

struct dt_leave_schedule {
	int cpu_id;
	u64 cpu_clk[2];
	unsigned short last_scheduled_frame;
};

struct dt_leave_schedule_async {
	int cpu_id;
	u64 cpu_clk[2];
	unsigned short frame;
	unsigned short usecs_left;
};

struct dt_leave_schedule_periodic {
	int cpu_id;
	u64 cpu_clk[2];
	unsigned short frame;
	unsigned short usecs_left;
};

struct dt_start_channel {
	struct dwc2_urb_priv *urb_priv;
	struct dwc2_qh *qh;
	struct dwc2_channel *chan;
};

struct dt_chan_trans_info {
	struct dwc2_channel *channel;
	u32 frame_inuse[2];
	int ctrl_stage;
	int error_count;
	u32 hcchar;
	u32 hctsiz;
	u32 hcdma;
	int cpu_id;
	u64 cpu_clk[2];
	u32 hfnum;
};

struct dt_gintsts {
	u32	gintsts;
	u32	gintmsk;
};

struct dt_haint {
	u32 haint;
};

struct dt_chan_intr {
	int chan_num;
	u32 hcint;
	u32 hcchar;
	u32 hctsiz;
	u32 hcdma;
	int cpu_id;
	u64 cpu_clk[2];
};

struct dt_urb_done {
	struct urb *urb;
	int status;
};

struct dt_qh_schedule_info {
	struct dwc2_qh *qh;
	int status;
	u32 start_frame;
	u32 period;
	u8 smask;
};

struct dt_isoc_schedule {
	struct dwc2_urb_priv *urb_priv;
	struct dwc2_qh *qh;
	int chan_num;
	int remain_slots;
	int request_slots;
	u16 start_frame;
	u16 curr_frame;
	u16 last_frame;
};

struct dt_isoc_urb_priv_slots {
	struct dwc2_urb_priv *urb_priv;
	u8 slots[32];
	int num_slots;
};

struct dt_clear_urb_hcpriv {
	struct urb *urb;
};

struct dwc_trace_entry {
	enum dwc2_trace_type type;

	union {
		struct dt_func_line			func_line;
		struct dt_urb_enqueue			urb_enqueue;
		struct dt_urb_dequeue			urb_dequeue;
		struct dt_ep_disable			ep_disable;
		struct dt_new_qh			new_qh;
		struct dt_del_qh			del_qh;
		struct dt_urb_submit			urb_submit;
		struct dt_sof_en			sof_en;
		struct dt_enter_schedule		enter_schedule;
		struct dt_frame_missing			frame_missing;
		struct dt_clean_up_chan			clean_up_chan;
		struct dt_enter_schedule_async		enter_schedule_async;
		struct dt_enter_schedule_periodic	enter_schedule_periodic;
		struct dt_sched_qh_state		sched_qh_state;
		struct dt_leave_schedule		leave_schedule;
		struct dt_leave_schedule_async		leave_schedule_async;
		struct dt_leave_schedule_periodic	leave_schedule_periodic;
		struct dt_start_channel			start_channel;
		struct dt_chan_trans_info		chan_trans_info;
		struct dt_gintsts			gintsts;
		struct dt_haint				haint;
		struct dt_chan_intr			chan_intr;
		struct dt_urb_done			urb_done;
		struct dt_qh_schedule_info		qh_schedule_info;
		struct dt_isoc_schedule			isoc_schedule;
		struct dt_isoc_urb_priv_slots		isoc_urb_priv_slots;
		struct dt_clear_urb_hcpriv		clear_urb_hcpriv;
	} trace_info;
};

static struct dwc_trace_entry *dwc_trace_head = (struct dwc_trace_entry *)(192 * 1024 * 1024 | 0xa0000000);
static int dwc_trace_max_entry = ((256 - 192) * 1024 * 1024 / sizeof(struct dwc_trace_entry));
static int req_trace_curr_idx = 0;
static int req_trace_overflow = 0;
int req_trace_enable = 0;

#define DWC_TRACE_GET_ENTRY					\
	struct dwc_trace_entry *entry;				\
								\
	if (!req_trace_enable) return;				\
								\
	entry = dwc_trace_head + req_trace_curr_idx;		\
								\
	req_trace_curr_idx++;					\
								\
	if (req_trace_curr_idx == dwc_trace_max_entry) {	\
		req_trace_curr_idx = 0;				\
		req_trace_overflow = 1;				\
	}							\
								\
	do { } while(0)


void dwc2_trace_func_line(void *func, int line) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_FUNC_LINE;
	entry->trace_info.func_line.func = func;
	entry->trace_info.func_line.line = line;
}

static void dwc2_trace_print_func_line(
	struct dwc_trace_entry *entry) {
	printk("enter %pS:%d\n",
		entry->trace_info.func_line.func,
		entry->trace_info.func_line.line);
}

void dwc2_trace_urb_enqueue(struct urb * urb, int ret) {
	int is_in = usb_pipein(urb->pipe);

	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_URB_ENQUEUE;
	entry->trace_info.urb_enqueue.urbp = urb;
	entry->trace_info.urb_enqueue.eptype = usb_endpoint_type(&urb->ep->desc);
	entry->trace_info.urb_enqueue.speed = urb->dev->speed;
	entry->trace_info.urb_enqueue.mps = usb_maxpacket(urb->dev, urb->pipe, !is_in);
	entry->trace_info.urb_enqueue.ret = ret;
	memcpy(&entry->trace_info.urb_enqueue.urb, urb, sizeof(struct urb));
}

static void dwc2_dump_urb(struct urb *urb_addr, struct urb *urb, int eptype, u32 speed, u32 mps, int ret) {
	int is_in = usb_pipein(urb->pipe);

	const char *type_str;

	switch(eptype) {
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

	printk("urb = %p ep%d%s type=%s ep=%u dev=%u spd=%u interval=%d mps=%u sfrm=%u ret=%d\n",
		urb_addr, usb_pipeendpoint (urb->pipe), is_in ? "in" : "out",
		type_str, usb_pipeendpoint(urb->pipe),
		usb_pipedevice(urb->pipe), speed, urb->interval,
		mps, urb->start_frame, ret);
	printk("    dma=0x%08x len=%u flags=0x%08x sdma=0x%08x npkts=%d actual=%d status=%d\n",
		urb->transfer_dma, urb->transfer_buffer_length, urb->transfer_flags,
		urb->setup_dma, urb->number_of_packets, urb->actual_length, urb->status);
}

static void dwc2_trace_print_urb_enqueue(struct dwc_trace_entry *entry) {
	dwc2_dump_urb(entry->trace_info.urb_enqueue.urbp,
		&entry->trace_info.urb_enqueue.urb,
		entry->trace_info.urb_enqueue.eptype,
		entry->trace_info.urb_enqueue.speed,
		entry->trace_info.urb_enqueue.mps,
		entry->trace_info.urb_enqueue.ret);
}

void dwc2_trace_urb_dequeue(struct urb *urb) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_URB_DEQUEUE;
	entry->trace_info.urb_dequeue.urbp = urb;
}

static void dwc2_trace_print_urb_dequeue(struct dwc_trace_entry *entry) {
	printk("dequeue urb %p\n",
		entry->trace_info.urb_dequeue.urbp);
}

void dwc2_trace_ep_disable(struct usb_host_endpoint *ep) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_EP_DISABLE;
	entry->trace_info.ep_disable.num = usb_endpoint_num(&ep->desc);
	entry->trace_info.ep_disable.is_in = usb_endpoint_dir_in(&ep->desc);
	entry->trace_info.ep_disable.hcpriv = ep->hcpriv;
}

static void dwc2_trace_print_ep_disable(struct dwc_trace_entry *entry) {
	printk("disable ep%d%s, hcpriv = %p\n",
		entry->trace_info.ep_disable.num,
		entry->trace_info.ep_disable.is_in ? "in" : "out",
		entry->trace_info.ep_disable.hcpriv);
}

void dwc2_trace_new_qh(struct dwc2_qh *qh) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_NEW_QH;
	entry->trace_info.new_qh.qh = qh;
	entry->trace_info.new_qh.mps = qh->mps;
	entry->trace_info.new_qh.hub_addr = qh->hub_addr;
	entry->trace_info.new_qh.port_number = qh->port_number;
}

static void dwc2_trace_print_new_qh(struct dwc_trace_entry *entry) {
	printk("new qh %p, mps=%d hub_addr=%u port=%u\n",
		entry->trace_info.new_qh.qh,
		entry->trace_info.new_qh.mps,
		entry->trace_info.new_qh.hub_addr,
		entry->trace_info.new_qh.port_number);
}

void dwc2_trace_del_qh(struct dwc2_qh *qh) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_DEL_QH;
	entry->trace_info.del_qh.qh = qh;
}

static void dwc2_trace_print_del_qh(struct dwc_trace_entry *entry) {
	printk("del qh %p\n", entry->trace_info.del_qh.qh);
}

void dwc2_trace_urb_submit(struct urb *urb, struct dwc2_qh *qh) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_URB_SUBMIT;
	entry->trace_info.urb_submit.urb = urb;
	entry->trace_info.urb_submit.urb_priv = urb->hcpriv;
	entry->trace_info.urb_submit.qh = qh;
}

static void dwc2_trace_print_urb_submit(struct dwc_trace_entry *entry) {
	printk("submit urb %p got urb_priv %p on qh %p\n",
		entry->trace_info.urb_submit.urb,
		entry->trace_info.urb_submit.urb_priv,
		entry->trace_info.urb_submit.qh);
}

void dwc2_trace_sof_en(unsigned short exp_frame) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_SOF_EN;
	entry->trace_info.sof_en.exp_frame = exp_frame;
}

static void dwc2_trace_print_sof_en(struct dwc_trace_entry *entry) {
	printk("enable SOF, expect expect in frame = %u\n",
		entry->trace_info.sof_en.exp_frame);
}

void dwc2_trace_enter_schedule(
	int has_int_isoc,
	u32 last_frame,
	u32 curr_frame,
	int sof_intr_enabled
	) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_ENTER_SCHEDULE;
	entry->trace_info.enter_schedule.has_int_isoc = has_int_isoc;
	entry->trace_info.enter_schedule.sof_intr_enabled = sof_intr_enabled;
	entry->trace_info.enter_schedule.last_frame = last_frame;
	entry->trace_info.enter_schedule.curr_frame = curr_frame;
	entry->trace_info.enter_schedule.cpu_id = smp_processor_id();
	entry->trace_info.enter_schedule.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.enter_schedule.cpu_clk[1] = cpu_clock(1);
}

static void dwc2_trace_print_enter_schedule(struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu]enter schedule, has_int_isoc = %d, last_frame = %u, "
		"curr_frame = %u\n",
		entry->trace_info.enter_schedule.cpu_id,
		entry->trace_info.enter_schedule.cpu_clk[0],
		entry->trace_info.enter_schedule.cpu_clk[1],
		entry->trace_info.enter_schedule.has_int_isoc,
		entry->trace_info.enter_schedule.last_frame,
		entry->trace_info.enter_schedule.curr_frame);
}

void dwc2_trace_frame_missing(unsigned short last_frame, unsigned short curr_frame) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_FRAME_MISSING;
	entry->trace_info.frame_missing.last_frame = last_frame;
	entry->trace_info.frame_missing.curr_frame = curr_frame;
}

static void dwc2_trace_print_frame_missing(struct dwc_trace_entry *entry) {
	printk("frame missing: last = %u curr = %u\n",
		entry->trace_info.frame_missing.last_frame,
		entry->trace_info.frame_missing.curr_frame);
}

void dwc2_trace_clean_up_chan(struct dwc2_channel *chan,
			unsigned short frame_num) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_CLEAN_UP_CHAN;
	entry->trace_info.clean_up_chan.chan_num = chan->number;
	entry->trace_info.clean_up_chan.frame_num = frame_num;
}

static void dwc2_trace_print_clean_up_chan(
	struct dwc_trace_entry *entry) {
	printk("chan %d transfering when cleanup frame %u\n",
		entry->trace_info.clean_up_chan.chan_num,
		entry->trace_info.clean_up_chan.frame_num);
}

void dwc2_trace_enter_schedule_async(unsigned short frame,
				unsigned short usecs_left) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_ENTER_SCHEDULE_ASYNC;
	entry->trace_info.enter_schedule_async.cpu_id = smp_processor_id();
	entry->trace_info.enter_schedule_async.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.enter_schedule_async.cpu_clk[1] = cpu_clock(1);
	entry->trace_info.enter_schedule_async.frame = frame;
	entry->trace_info.enter_schedule_async.usecs_left = usecs_left;
}

static void dwc2_trace_print_enter_schedule_async(
	struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu] enter schedule async, frame=%u usecs_left=%u\n",
		entry->trace_info.enter_schedule_async.cpu_id,
		entry->trace_info.enter_schedule_async.cpu_clk[0],
		entry->trace_info.enter_schedule_async.cpu_clk[1],
		entry->trace_info.enter_schedule_async.frame,
		entry->trace_info.enter_schedule_async.usecs_left);
}

void dwc2_trace_enter_schedule_periodic(unsigned short frame,
					unsigned short usecs_left) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_ENTER_SCHEDULE_PERIODIC;
	entry->trace_info.enter_schedule_periodic.cpu_id = smp_processor_id();
	entry->trace_info.enter_schedule_periodic.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.enter_schedule_periodic.cpu_clk[1] = cpu_clock(1);
	entry->trace_info.enter_schedule_periodic.frame = frame;
	entry->trace_info.enter_schedule_periodic.usecs_left = usecs_left;
}

static void dwc2_trace_print_enter_schedule_periodic(
	struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu] enter schedule periodic, frame=%u usecs_left=%u\n",
		entry->trace_info.enter_schedule_periodic.cpu_id,
		entry->trace_info.enter_schedule_periodic.cpu_clk[0],
		entry->trace_info.enter_schedule_periodic.cpu_clk[1],
		entry->trace_info.enter_schedule_periodic.frame,
		entry->trace_info.enter_schedule_periodic.usecs_left);
}


void dwc2_trace_schedule_no_channel(void) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_SCHEDULE_NO_CHANNEL;
}

static void dwc2_trace_print_schedule_no_channel(
	struct dwc_trace_entry *entry) {
	printk("no channels, all channels assigned!\n");
}

void dwc2_trace_sched_qh_state(struct dwc2_qh *qh) {
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_SCHED_QH_STATE;
	entry->trace_info.sched_qh_state.qh = qh;
	entry->trace_info.sched_qh_state.list_state = !list_empty(&qh->urb_list);
	entry->trace_info.sched_qh_state.disabling = qh->disabling;

	if (!list_empty(&qh->urb_list)) {
		struct dwc2_urb_priv *urb_priv =
			list_first_entry(&qh->urb_list, struct dwc2_urb_priv, list);
		entry->trace_info.sched_qh_state.first_urb_priv_state = urb_priv->state;
	}
}

static void dwc2_trace_print_sched_qh_state(struct dwc_trace_entry *entry) {
	printk("schedule qh %p, td_list=%d disabling=%d first_urb_priv_state=%d\n",
		entry->trace_info.sched_qh_state.qh,
		entry->trace_info.sched_qh_state.list_state,
		entry->trace_info.sched_qh_state.disabling,
		entry->trace_info.sched_qh_state.first_urb_priv_state);
}

void dwc2_trace_leave_schedule(unsigned short last_scheduled_frame) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_LEAVE_SCHEDULE;
	entry->trace_info.leave_schedule.cpu_id = smp_processor_id();
	entry->trace_info.leave_schedule.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.leave_schedule.cpu_clk[1] = cpu_clock(1);
	entry->trace_info.leave_schedule.last_scheduled_frame = last_scheduled_frame;
}

static void dwc2_trace_print_leave_schedule(struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu] leave schedule, last_scheduled_frame = %u\n",
		entry->trace_info.leave_schedule.cpu_id,
		entry->trace_info.leave_schedule.cpu_clk[0],
		entry->trace_info.leave_schedule.cpu_clk[1],
		entry->trace_info.leave_schedule.last_scheduled_frame);
}

void dwc2_trace_leave_schedule_async(unsigned short frame,
				unsigned short usecs_left) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_LEAVE_SCHEDULE_ASYNC;
	entry->trace_info.leave_schedule_async.cpu_id = smp_processor_id();
	entry->trace_info.leave_schedule_async.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.leave_schedule_async.cpu_clk[1] = cpu_clock(1);
	entry->trace_info.leave_schedule_async.frame = frame;
	entry->trace_info.leave_schedule_async.usecs_left = usecs_left;
}

static void dwc2_trace_print_leave_schedule_async(
	struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu] leave async schedule frame=%u usecs_left=%u\n",
		entry->trace_info.leave_schedule_async.cpu_id,
		entry->trace_info.leave_schedule_async.cpu_clk[0],
		entry->trace_info.leave_schedule_async.cpu_clk[1],
		entry->trace_info.leave_schedule_async.frame,
		entry->trace_info.leave_schedule_async.usecs_left);
}

void dwc2_trace_leave_schedule_periodic(unsigned short frame,
					unsigned short usecs_left) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_LEAVE_SCHEDULE_PERIODIC;
	entry->trace_info.leave_schedule_periodic.cpu_id = smp_processor_id();
	entry->trace_info.leave_schedule_periodic.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.leave_schedule_periodic.cpu_clk[1] = cpu_clock(1);
	entry->trace_info.leave_schedule_periodic.frame = frame;
	entry->trace_info.leave_schedule_periodic.usecs_left = usecs_left;
}

static void dwc2_trace_print_leave_schedule_periodic(
	struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu] leave periodic schedule frame=%u usecs_left=%u\n",
		entry->trace_info.leave_schedule_periodic.cpu_id,
		entry->trace_info.leave_schedule_periodic.cpu_clk[0],
		entry->trace_info.leave_schedule_periodic.cpu_clk[1],
		entry->trace_info.leave_schedule_periodic.frame,
		entry->trace_info.leave_schedule_periodic.usecs_left);
}

void dwc2_trace_start_channel(struct dwc2_urb_priv *urb_priv, struct dwc2_qh *qh,
			struct dwc2_channel *chan) {
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_START_CHANNEL;
	entry->trace_info.start_channel.urb_priv = urb_priv;
	entry->trace_info.start_channel.qh = qh;
	entry->trace_info.start_channel.chan = chan;
}

static void dwc2_trace_print_start_channel(struct dwc_trace_entry *entry) {
	printk("start urb_priv %p (qh = %p) on channel %d\n",
		entry->trace_info.start_channel.urb_priv,
		entry->trace_info.start_channel.qh,
		entry->trace_info.start_channel.chan->number);
}

void dwc2_trace_chan_trans_info(
	struct dwc2 *dwc,
	struct dwc2_channel *channel,
	int ctrl_stage,
	int error_count,
	u32 hcchar,
	u32 hctsiz,
	u32 hcdma) {
	hfnum_data_t	hfnum;
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_CHAN_TRANS_INFO;
	entry->trace_info.chan_trans_info.channel = channel;
	entry->trace_info.chan_trans_info.frame_inuse[0] = channel->frame_inuse[0];
	entry->trace_info.chan_trans_info.frame_inuse[1] = channel->frame_inuse[1];
	entry->trace_info.chan_trans_info.ctrl_stage = ctrl_stage;
	entry->trace_info.chan_trans_info.error_count = error_count;
	entry->trace_info.chan_trans_info.hcchar = hcchar;
	entry->trace_info.chan_trans_info.hctsiz = hctsiz;
	entry->trace_info.chan_trans_info.hcdma = hcdma;
	entry->trace_info.chan_trans_info.cpu_id = smp_processor_id();
	entry->trace_info.chan_trans_info.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.chan_trans_info.cpu_clk[1] = cpu_clock(1);

	hfnum.d32 = dwc_readl(&dwc->host_if.host_global_regs->hfnum);
	entry->trace_info.chan_trans_info.hfnum = hfnum.d32;
}

static void dwc2_trace_print_chan_trans_info(struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu] chan%d trans info: ctrl_stage = %d, error_count = %d, "
		"hcchar = 0x%08x hctsiz = 0x%08x hcdma=0x%08x hfnum=0x%08x frame_in_use: 0x%08x 0x%08x\n",
		entry->trace_info.chan_trans_info.cpu_id,
		entry->trace_info.chan_trans_info.cpu_clk[0],
		entry->trace_info.chan_trans_info.cpu_clk[1],
		entry->trace_info.chan_trans_info.channel->number,
		entry->trace_info.chan_trans_info.ctrl_stage,
		entry->trace_info.chan_trans_info.error_count,
		entry->trace_info.chan_trans_info.hcchar,
		entry->trace_info.chan_trans_info.hctsiz,
		entry->trace_info.chan_trans_info.hcdma,
		entry->trace_info.chan_trans_info.hfnum,
		entry->trace_info.chan_trans_info.frame_inuse[0],
		entry->trace_info.chan_trans_info.frame_inuse[1]);
}

void dwc2_trace_gintsts(u32 gintsts, u32 gintmsk) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_GINTSTS;
	entry->trace_info.gintsts.gintsts = gintsts;
	entry->trace_info.gintsts.gintmsk = gintmsk;
}

static void dwc2_trace_print_gintsts(struct dwc_trace_entry *entry) {
	u32 gintsts = entry->trace_info.gintsts.gintsts;
	u32 gintmsk = entry->trace_info.gintsts.gintmsk;

	printk("gintsts=0x%08x & gintmsk=0x%08x = 0x%08x\n",
		gintsts, gintmsk, gintsts & gintmsk);
}

void dwc2_trace_haint(u32 haint) {
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_HAINT;
	entry->trace_info.haint.haint = haint;
}

static void dwc2_trace_print_haint(struct dwc_trace_entry *entry) {
	printk("haint=0x%08x\n", entry->trace_info.haint.haint);
}

void dwc2_trace_chan_intr(
	int chan_num,
	u32 hcint,
	u32 hcchar,
	u32 hctsiz,
	u32 hcdma) {
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_CHANNEL_INTR;
	entry->trace_info.chan_intr.chan_num = chan_num;
	entry->trace_info.chan_intr.hcint = hcint;
	entry->trace_info.chan_intr.hcchar = hcchar;
	entry->trace_info.chan_intr.hctsiz = hctsiz;
	entry->trace_info.chan_intr.hcdma = hcdma;
	entry->trace_info.chan_intr.cpu_id = smp_processor_id();
	entry->trace_info.chan_intr.cpu_clk[0] = cpu_clock(0);
	entry->trace_info.chan_intr.cpu_clk[1] = cpu_clock(1);
}

static void dwc2_trace_print_chan_intr(struct dwc_trace_entry *entry) {
	printk("[CPU%d: %llu:%llu] chan%d intr, hcint = 0x%08x hcchar = 0x%08x "
		"hctsiz = 0x%08x hcdma = 0x%08x\n",
		entry->trace_info.chan_intr.cpu_id,
		entry->trace_info.chan_intr.cpu_clk[0],
		entry->trace_info.chan_intr.cpu_clk[1],
		entry->trace_info.chan_intr.chan_num,
		entry->trace_info.chan_intr.hcint,
		entry->trace_info.chan_intr.hcchar,
		entry->trace_info.chan_intr.hctsiz,
		entry->trace_info.chan_intr.hcdma);
}

void dwc2_trace_urb_done(struct urb *urb, int status) {
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_URB_DONE;
	entry->trace_info.urb_done.urb = urb;
	entry->trace_info.urb_done.status = status;
}

static void dwc2_trace_print_urb_done(struct dwc_trace_entry *entry) {
	printk("urb = %p done,  status=%d\n",
		entry->trace_info.urb_done.urb,
		entry->trace_info.urb_done.status);
}

void dwc2_trace_qh_schedule_info(struct dwc2_qh *qh, int status) {
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_QH_SCHED_INFO;
	entry->trace_info.qh_schedule_info.qh = qh;
	entry->trace_info.qh_schedule_info.status = status;
	entry->trace_info.qh_schedule_info.start_frame = qh->start_frame;
	entry->trace_info.qh_schedule_info.period = qh->period;
	entry->trace_info.qh_schedule_info.smask = qh->smask;
}

static void dwc2_trace_print_qh_schedule_info(struct dwc_trace_entry *entry) {
	printk("qh %p schedule status = %d, start_frame = %u "
		"period = %u smask=0x%02x\n",
		entry->trace_info.qh_schedule_info.qh,
		entry->trace_info.qh_schedule_info.status,
		entry->trace_info.qh_schedule_info.start_frame,
		entry->trace_info.qh_schedule_info.period,
		entry->trace_info.qh_schedule_info.smask);
}

void dwc2_trace_isoc_schedule(struct dwc2 *dwc, struct dwc2_urb_priv *urb_priv,
			unsigned short curr_frame, unsigned short start_frame,
			unsigned short request_slots) {
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_ISOC_SCHEDULE;
	entry->trace_info.isoc_schedule.urb_priv = urb_priv;
	entry->trace_info.isoc_schedule.qh = urb_priv->owner_qh;
	entry->trace_info.isoc_schedule.chan_num = urb_priv->owner_qh->channel->number;
	entry->trace_info.isoc_schedule.remain_slots = urb_priv->owner_qh->channel->remain_slots;
	entry->trace_info.isoc_schedule.request_slots = request_slots;
	entry->trace_info.isoc_schedule.start_frame = start_frame;
	entry->trace_info.isoc_schedule.curr_frame = curr_frame;
	entry->trace_info.isoc_schedule.last_frame = urb_priv->owner_qh->last_frame;
}

static void dwc2_trace_print_isoc_schedule(struct dwc_trace_entry *entry) {
	printk("chan%d schedule urb_priv %p on qh %p, req_slots=%u rem_slots=%u "
		"start_frame = %u curr_frame = %u last_frame=%u\n",
		entry->trace_info.isoc_schedule.chan_num,
		entry->trace_info.isoc_schedule.urb_priv,
		entry->trace_info.isoc_schedule.qh,
		entry->trace_info.isoc_schedule.request_slots,
		entry->trace_info.isoc_schedule.remain_slots,
		entry->trace_info.isoc_schedule.start_frame,
		entry->trace_info.isoc_schedule.curr_frame,
		entry->trace_info.isoc_schedule.last_frame);
}

void dwc2_trace_isoc_urb_priv_slots(struct dwc2_urb_priv *urb_priv) {
	int idx = 0;
	struct dwc2_td *td;
	DWC_TRACE_GET_ENTRY;

	entry->type = DWC_TRACE_ISOC_URB_PRIV_SLOTS;
	entry->trace_info.isoc_urb_priv_slots.urb_priv = urb_priv;
	list_for_each_entry(td, &urb_priv->td_list, list) {
		entry->trace_info.isoc_urb_priv_slots.slots[idx] =
			td->dma_desc_idx;
		idx++;
		if (idx >= 32)
			break;
	}
	entry->trace_info.isoc_urb_priv_slots.num_slots = idx;
}

static void dwc2_trace_print_isoc_urb_priv_slots(struct dwc_trace_entry *entry) {
	int idx;

	printk("insert urb_priv in slots: ");
	for (idx = 0; idx < entry->trace_info.isoc_urb_priv_slots.num_slots; idx++) {
		printk("[%u] ", entry->trace_info.isoc_urb_priv_slots.slots[idx]);
	}
	printk("\n");
}

void dwc2_trace_clear_urb_hcpriv(struct urb *urb) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_CLEAR_URB_HCPRIV;
	entry->trace_info.clear_urb_hcpriv.urb = urb;
}

static void dwc2_trace_print_clear_urb_hcpriv(struct dwc_trace_entry *entry) {
	printk("===>set urb %p hcpriv to NULL\n",
		entry->trace_info.clear_urb_hcpriv.urb);
}

typedef void (*dwc2_trace_print_func)(struct dwc_trace_entry *entry);

void dwc2_trace_sof_dis(void) {
	DWC_TRACE_GET_ENTRY;
	entry->type = DWC_TRACE_SOF_DIS;
}

static void dwc2_trace_print_sof_dis(struct dwc_trace_entry *entry) {
	printk("SOF disable\n");
}

dwc2_trace_print_func print_func[DWC_TRACE_MAX] = {
	[DWC_TRACE_FUNC_LINE]		    = dwc2_trace_print_func_line,
	[DWC_TRACE_URB_ENQUEUE]		    = dwc2_trace_print_urb_enqueue,
	[DWC_TRACE_URB_DEQUEUE]		    = dwc2_trace_print_urb_dequeue,
	[DWC_TRACE_EP_DISABLE]		    = dwc2_trace_print_ep_disable,
	[DWC_TRACE_NEW_QH]		    = dwc2_trace_print_new_qh,
	[DWC_TRACE_DEL_QH]		    = dwc2_trace_print_del_qh,
	[DWC_TRACE_URB_SUBMIT]		    = dwc2_trace_print_urb_submit,
	[DWC_TRACE_SOF_EN]		    = dwc2_trace_print_sof_en,
	[DWC_TRACE_SOF_DIS]		    = dwc2_trace_print_sof_dis,
	[DWC_TRACE_ENTER_SCHEDULE]	    = dwc2_trace_print_enter_schedule,
	[DWC_TRACE_FRAME_MISSING]	    = dwc2_trace_print_frame_missing,
	[DWC_TRACE_CLEAN_UP_CHAN]	    = dwc2_trace_print_clean_up_chan,
	[DWC_TRACE_ENTER_SCHEDULE_ASYNC]    = dwc2_trace_print_enter_schedule_async,
	[DWC_TRACE_ENTER_SCHEDULE_PERIODIC] = dwc2_trace_print_enter_schedule_periodic,
	[DWC_TRACE_SCHEDULE_NO_CHANNEL]	    = dwc2_trace_print_schedule_no_channel,
	[DWC_TRACE_SCHED_QH_STATE]	    = dwc2_trace_print_sched_qh_state,
	[DWC_TRACE_LEAVE_SCHEDULE]	    = dwc2_trace_print_leave_schedule,
	[DWC_TRACE_LEAVE_SCHEDULE_ASYNC]    = dwc2_trace_print_leave_schedule_async,
	[DWC_TRACE_LEAVE_SCHEDULE_PERIODIC] = dwc2_trace_print_leave_schedule_periodic,
	[DWC_TRACE_START_CHANNEL]	    = dwc2_trace_print_start_channel,
	[DWC_TRACE_CHAN_TRANS_INFO]	    = dwc2_trace_print_chan_trans_info,
	[DWC_TRACE_GINTSTS]		    = dwc2_trace_print_gintsts,
	[DWC_TRACE_HAINT]		    = dwc2_trace_print_haint,
	[DWC_TRACE_CHANNEL_INTR]	    = dwc2_trace_print_chan_intr,
	[DWC_TRACE_URB_DONE]		    = dwc2_trace_print_urb_done,
	[DWC_TRACE_QH_SCHED_INFO]	    = dwc2_trace_print_qh_schedule_info,
	[DWC_TRACE_ISOC_SCHEDULE]	    = dwc2_trace_print_isoc_schedule,
	[DWC_TRACE_ISOC_URB_PRIV_SLOTS]     = dwc2_trace_print_isoc_urb_priv_slots,
	[DWC_TRACE_CLEAR_URB_HCPRIV]	    = dwc2_trace_print_clear_urb_hcpriv,
};

void __dwc2_host_trace_print(struct dwc2 *dwc, int nolock) {
	int			 begin_idx = 0;
	int			 end_idx   = req_trace_curr_idx;
	int			 num	   = end_idx;
	int			 i, idx;
	struct dwc_trace_entry	*entry;
	unsigned long		 flags = 0;
	int			 old_enable;

	if (!nolock)
		dwc2_spin_lock_irqsave(dwc, flags);

	old_enable = req_trace_enable;

	if (!req_trace_enable) {
		printk("request trace disabled\n");
		goto out;
	}

	if (req_trace_curr_idx == 0) {
		if (!req_trace_overflow) {
			printk("no trace entry!\n");
			goto out;
		}

		end_idx = dwc_trace_max_entry - 1;
	}

	if (req_trace_overflow) {
		begin_idx = req_trace_curr_idx;
		num = dwc_trace_max_entry;
	}

	req_trace_enable = 0;
	req_trace_curr_idx = 0;
	req_trace_overflow = 0;

	if (!nolock)
		dwc2_spin_unlock_irqrestore(dwc, flags);

	idx = begin_idx;
	for (i = 0; i < num; i++) {
		entry = dwc_trace_head + idx;
		print_func[entry->type](entry);
		idx++;
		if (idx >= dwc_trace_max_entry)
			idx = 0;
	}

	if (!nolock)
		dwc2_spin_lock_irqsave(dwc, flags);
	req_trace_enable = old_enable;
out:
	if (!nolock)
		dwc2_spin_unlock_irqrestore(dwc, flags);
}

static int dwc2_req_trace_show(struct seq_file *s, void *unused)
{
	struct dwc2 *dwc = s->private;

	__dwc2_host_trace_print(dwc, 0);

	return -EIO;
}

static int dwc2_req_trace_open(struct inode *inode, struct file *file)
{
	return single_open(file, dwc2_req_trace_show, inode->i_private);
}

static ssize_t dwc2_req_trace_write(struct file *file,
			const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s   = file->private_data;
        struct dwc2	*dwc = s->private;
	unsigned long	 flags;
	char		 buf[32];

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	dwc2_spin_lock_irqsave(dwc, flags);
	req_trace_enable = simple_strtol(buf, NULL, 0);
	req_trace_curr_idx = 0;
	req_trace_overflow = 0;
	dwc2_spin_unlock_irqrestore(dwc, flags);

	return count;
}

static const struct file_operations dwc2_req_trace_fops = {
	.open			= dwc2_req_trace_open,
	.write			= dwc2_req_trace_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};
#endif	/* CONFIG_USB_DWC2_HOST_TRACER */

int dwc2_debugfs_init(struct dwc2 *dwc)
{
	struct dentry		*root;
#ifdef CONFIG_USB_DWC2_HOST_TRACER
	struct dentry		*file;
#endif
	int			ret;

	root = debugfs_create_dir(dev_name(dwc->dev), NULL);
	if (!root) {
		ret = -ENOMEM;
		goto err0;
	}

	dwc->root = root;

#ifdef CONFIG_USB_DWC2_HOST_TRACER
	file = debugfs_create_file("dwc_req_trace", S_IWUSR, root,
				dwc, &dwc2_req_trace_fops);

	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}
#endif	/* CONFIG_USB_DWC2_HOST_TRACER */

	return 0;

#ifdef CONFIG_USB_DWC2_HOST_TRACER
err1:
	debugfs_remove_recursive(root);
#endif

err0:
	return ret;
}

void dwc2_debugfs_exit(struct dwc2 *dwc)
{
	debugfs_remove_recursive(dwc->root);
	dwc->root = NULL;
}
