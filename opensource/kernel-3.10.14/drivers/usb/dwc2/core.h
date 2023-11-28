#ifndef __DRIVERS_USB_DWC2_CORE_H
#define __DRIVERS_USB_DWC2_CORE_H

#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/debugfs.h>
#ifdef CONFIG_USB_DWC2_ALLOW_WAKEUP
#include <linux/wakelock.h>
#endif

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>


#ifdef CONFIG_USB_DWC2_VERBOSE_VERBOSE
static int noinline __attribute__((unused))
dwc2_printk(const char *comp, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	int rtn;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;
	rtn = printk("CPU%d: DWC2(%s): %pV", smp_processor_id(), comp, &vaf);
	va_end(args);

	return rtn;
}
#endif

#define DWC2_HOST_MODE_ENABLE	1
#define DWC2_DEVICE_MODE_ENABLE	1

#if IS_ENABLED(CONFIG_USB_DWC2_HOST_ONLY)
#undef DWC2_DEVICE_MODE_ENABLE
#define DWC2_DEVICE_MODE_ENABLE	0
#endif

#if IS_ENABLED(CONFIG_USB_DWC2_DEVICE_ONLY)
#undef DWC2_HOST_MODE_ENABLE
#define DWC2_HOST_MODE_ENABLE	0
#endif
/** Macros defined for DWC OTG HW Release version */

#define OTG_CORE_REV_2_60a	0x4F54260A
#define OTG_CORE_REV_2_71a	0x4F54271A
#define OTG_CORE_REV_2_72a	0x4F54272A
#define OTG_CORE_REV_2_80a	0x4F54280A
#define OTG_CORE_REV_2_81a	0x4F54281A
#define OTG_CORE_REV_2_90a	0x4F54290A
#define OTG_CORE_REV_2_91a	0x4F54291A
#define OTG_CORE_REV_2_92a	0x4F54292A
#define OTG_CORE_REV_2_93a	0x4F54293A
#define OTG_CORE_REV_2_94a	0x4F54294A

/** Maximum number of Periodic FIFOs */
#define MAX_PERIO_FIFOS 15
/** Maximum number of Periodic FIFOs */
#define MAX_TX_FIFOS	15

/** Maximum number of Endpoints/HostChannels */
#define MAX_EPS_CHANNELS	16

#define DWC_MAX_PKT_CNT		1023
#define DWC_MAX_TRANSFER_SIZE	(1024 * 1023)

#include "dwc_otg_regs.h"

#define DWC2_REG_BASE	0xb3500000

#define DWC_PR(r)							\
	printk("DWC2: REG[0x%03x]=0x%08x\n",				\
		(r), *((volatile unsigned int *)((r) | DWC2_REG_BASE)))

#define DWC_RR(_r) 	r##_r = *((volatile unsigned int *)((_r) | DWC2_REG_BASE))
#define DWC_P(v)	printk("DWC2: REG["#v"] = 0x%08x\n", r##v)

#define DWC_RR_CH_REG(name, base, ch) \
	name = *((volatile unsigned int *)((base + (ch) * 0x20) | DWC2_REG_BASE))

#define DWC_RR_HCCHAR(name, ch)   DWC_RR_CH_REG(name, 0x500, (ch))
#define DWC_RR_HCSPLT(name, ch)   DWC_RR_CH_REG(name, 0x504, (ch))
#define DWC_RR_HCINT(name, ch)    DWC_RR_CH_REG(name, 0x508, (ch))
#define DWC_RR_HCINTMSK(name, ch) DWC_RR_CH_REG(name, 0x50c, (ch))
#define DWC_RR_HCTSIZ(name, ch)   DWC_RR_CH_REG(name, 0x510, (ch))
#define DWC_RR_HCDMA(name, ch)    DWC_RR_CH_REG(name, 0x514, (ch))

#define DWC_P_HCCHAR(name, ch)   printk("  hcchar(%d)=0x%08x\n", (ch), name)
#define DWC_P_HCSPLT(name, ch)   printk("  hcsplt(%d)=0x%08x\n", (ch), name)
#define DWC_P_HCINT(name, ch)    printk("   hcint(%d)=0x%08x\n", (ch), name)
#define DWC_P_HCINTMSK(name, ch) printk("hcintmsk(%d)=0x%08x\n", (ch), name)
#define DWC_P_HCTSIZ(name, ch)   printk("  hctsiz(%d)=0x%08x\n", (ch), name)
#define DWC_P_HCDMA(name, ch)     printk("   hcdma(%d)=0x%08x\n", (ch), name)

// high bandwidth multiplier, as encoded in highspeed endpoint descriptors
#define dwc2_hb_mult(wMaxPacketSize) (1 + (((wMaxPacketSize) >> 11) & 0x03))
// ... and packet size, for any kind of endpoint descriptor
#define dwc2_max_packet(wMaxPacketSize) ((wMaxPacketSize) & 0x07ff)


/*-------------------Device----------------- */

/**
 * struct dwc2_ep - device side endpoint/Host side channel representation
 * @usb_ep: usb endpoint
 * @request_list: list of requests for this endpoint
 * @dwc: pointer to DWC controller
 * @flags: endpoint flags (wedged, stalled, ...)
 * @number: endpoint number (1 - 15)
 * @type: set to bmAttributes & USB_ENDPOINT_XFERTYPE_MASK
 * @interval: the intervall on which the ISOC transfer is started
 * @name: a human readable name e.g. ep1out-bulk
 * @is_in: true for Tx(IN), false for Rx(OUT)
 * @tx_fifo_num: GRSTCTL.TxFNum of this ep
 * @desc: endpoint descriptor.  This pointer is set before the endpoint is
 *	enabled and remains valid until the endpoint is disabled.
 */
struct dwc2_ep {
	struct list_head	 request_list;
	struct dwc2		*dwc;

	unsigned		 flags;
#define DWC2_EP_ENABLED		(1 << 0)
#define DWC2_EP_STALL		(1 << 1)
#define DWC2_EP_WEDGE		(1 << 2)
#define DWC2_EP_BUSY		(1 << 3)

	u8			 number;
	u8			 type;
	u16			 maxp;
	u16                      mc;
	u32			 interval;
	unsigned		 is_in:1;

	struct usb_ep		usb_ep;
	char			name[20];
	struct list_head	garbage_list;

	const struct usb_endpoint_descriptor	*desc;

	void                   *align_addr;
	dma_addr_t              align_dma_addr;
#define DWC2_DEP_ALIGN_ALLOC_SIZE (1 * PAGE_SIZE)
};

/**
 * States of EP0.
 */
enum dwc2_ep0_state {
	EP0_DISCONNECTED = 0,
	EP0_WAITING_SETUP_PHASE,
	EP0_SETUP_PHASE,
	EP0_DATA_PHASE,
	EP0_STATUS_PHASE,
};

enum dwc2_device_state {
	DWC2_DEFAULT_STATE,
	DWC2_ADDRESS_STATE,
	DWC2_CONFIGURED_STATE,
};

enum dwc2_lx_state {
	/** On state */
	DWC_OTG_L0 = 0,
	/** LPM sleep state*/
	DWC_OTG_L1,
	/** USB suspend state*/
	DWC_OTG_L2,
	/** Off state*/
	DWC_OTG_L3
};

struct dwc2_request {
	struct usb_request	 request;
	struct list_head	 list;
	struct dwc2_ep		*dwc2_ep;

	int			 transfering;
	unsigned		 mapped:1;

	int			 trans_count_left;
	dma_addr_t		 next_dma_addr;
	int			 zlp_transfered;

	int			 xfersize;
	int			 pktcnt;
};

#define DWC_DEV_GLOBAL_REG_OFFSET 0x800
#define DWC_DEV_IN_EP_REG_OFFSET 0x900
#define DWC_EP_REG_OFFSET 0x20
#define DWC_DEV_OUT_EP_REG_OFFSET 0xB00

/**
 * struct dwc2_dev_if - Device-specific information
 * @dev_global_regs: Pointer to device Global registers 800h
 * @in_ep_regs: Device Logical IN Endpoint-Specific Registers 900h-AFCh
 * @out_ep_regs: Device Logical OUT Endpoint-Specific Registers B00h-CFCh
 * @speed: Device Speed 0: Unknown, 1: LS, 2: FS, 3: HS
 * @num_in_eps: number of Tx EPs
 * @num_out_eps: number of Rx EPs
 * @perio_tx_fifo_size: size of periodic FIFOs (Bytes)
 * @tx_fifo_size: size of Tx FIFOs (Bytes)
 */
struct dwc2_dev_if {
	dwc_otg_device_global_regs_t	*dev_global_regs;
	dwc_otg_dev_in_ep_regs_t	*in_ep_regs[MAX_EPS_CHANNELS];
	dwc_otg_dev_out_ep_regs_t	*out_ep_regs[MAX_EPS_CHANNELS];
	uint8_t				 speed;
	uint8_t				 num_in_eps;
	uint8_t				 num_out_eps;
	uint16_t			 perio_tx_fifo_size[MAX_PERIO_FIFOS];
	uint16_t			 tx_fifo_size[MAX_TX_FIFOS];
};

/*-------------Host--------------- */

struct dwc2_qh;

struct dwc2_isoc_qh_ptr {
	struct list_head frm_list; /* in dwc2 and dwc2_frame */
	struct list_head qh_list;  /* in dwc2_qh */
	struct dwc2_qh *qh;
	unsigned char uframe_index;
};

struct dwc2_frame {
	struct list_head	isoc_qh_list; /* of dwc2_isoc_qh_ptr */
	struct dwc2_qh		*int_qh_head;	/* interrupt qh*/
};

struct dwc2_qh {
	struct dwc2_qh		*next;
	struct list_head	 list;	   /* in all_qh_list */
	struct list_head	 urb_list; /* of urb_priv */
	struct list_head	 isoc_qh_ptr_list;
	int			 pause_schedule;

	/*
	 * +1: each time a TD is associate with a Channel
	 * -1: each time a TD is de-associate with a channel
	 */
	int			 trans_td_count;

	u8			type;
	u8			endpt;
	u8			hb_mult;
	u8			dev_addr;
	u8			hub_addr;
	u8			port_number;
	u16			mps;
	u8			speed;
	u8			xxx;
	unsigned		is_in:1;
	unsigned		lspddev:1;
	unsigned		disabling:1; /* endpoint is disabling */
	unsigned		bandwidth_reserved:1;

	hcchar_data_t		hcchar;
	hcsplt_data_t		hcsplt;
	hctsiz_data_t		hctsiz;

	/*
	 * initial data PID
	 * to be used on the first OUT transaction or
	 * to be expected from the first IN transaction
	 */
	int			 data_toggle;

	/* periodic schedule info */
	u16			usecs;	     /* intr bandwidth */
	u16			period;	     /* polling interval, in frame */
	u8			smask;	     /* microframe in frame */
	u16			start_frame; /* where polling starts */
#define NO_FRAME	((unsigned short)~0)
	u16			last_frame;

	struct dwc2_channel	*channel; /* [ISOC] channel reserved for isoc transfer */

	struct dwc2			*dwc;
	struct usb_host_endpoint	*host_ep; /* of this QH, its hcpriv points to this QH */
	struct usb_device		*dev;
};

struct dwc2_td {
	struct list_head	 list;	/* in urb_priv->td_list */
	dma_addr_t		 buffer;
	u32			 len;
	int			 dma_desc_idx;
	int			 ctrl_stage;

	/* for isoc */
	int			 isoc_idx;
	u16			 frame;

	struct urb		*urb;
	struct dwc2_qh		*owner_qh;
};

struct dwc2_urb_priv {
	struct list_head	 list;	/* of urb_priv */
	struct list_head	 td_list;	/* of td */
	struct list_head	 done_td_list;	/* of done td */
	struct urb		*urb;
	struct dwc2_qh		*owner_qh;
	struct dwc2_channel	*channel;
	enum dwc2_ep0_state	 control_stage;
	int			 error_count;
	int			 state;
#define DWC2_URB_IDLE		0
#define DWC2_URB_TRANSFERING	1
#define DWC2_URB_RETRY		2
#define DWC2_URB_CANCELING	3
#define DWC2_URB_DONE		4	/* this state is not actually existed */
	u16			isoc_slots;
};

/* support for URB_NO_INTERRUPT */
struct dwc2_urb_context {
	struct list_head	 list;	/* of dwc2_urb_context */
	void			*context;

	struct list_head	 urb_list; /* of urb_priv */
};

struct dwc2_channel {
	struct list_head	 list;
	int			 number;
	struct dwc2_urb_priv	*urb_priv;

	dwc_otg_host_dma_desc_t *sw_desc_list;
	dma_addr_t		 hw_desc_list;
	DECLARE_BITMAP(frame_inuse, MAX_FRLIST_EN_NUM);

	/* for isoc schedule */
	u16			remain_slots;
	struct dwc2_qh 		*qh;

	/* for halt(disable) a channel */
	wait_queue_head_t	 disable_wq;
	int 			 waiting;
	int			 disable_stage;	/* 0: not in disable state, 1: disable stage 1, 2: disable stage2 */
};

#define DWC_OTG_HOST_GLOBAL_REG_OFFSET 0x400
#define DWC_OTG_HOST_PORT_REGS_OFFSET 0x440
#define DWC_OTG_HOST_CHAN_REGS_OFFSET 0x500
#define DWC_OTG_CHAN_REGS_OFFSET 0x20

/**
 * struct dwc2_host_if - Host-specific information
 * @host_global_regs: Host Global Registers starting at offset 400h
 * @hprt0: Host Port Control and Status Register, 0 because our root hub only support one port
 * @hc_regs: Host Channel Specific Registers at offsets 500h-5FCh
 */
struct dwc2_host_if {
	dwc_otg_host_global_regs_t	*host_global_regs;
	volatile uint32_t		*hprt0;
	dwc_otg_hc_regs_t		*hc_regs[MAX_EPS_CHANNELS];
};

#define DWC_OTG_PCGCCTL_OFFSET	0xE00

/**
 * struct dwc2_hwcfgs - Hardware Configurations, stored here for convenience
 * @hwcfg1: HWCFG1
 * @hwcfg2: HWCFG2
 * @hwcfg3: HWCFG3
 * @hwcfg4: HWCFG4
 */
struct dwc2_hwcfgs {
	hwcfg1_data_t	hwcfg1;
	hwcfg2_data_t	hwcfg2;
	hwcfg3_data_t	hwcfg3;
	hwcfg4_data_t	hwcfg4;
};

struct dwc2_platform_data {
	int keep_phy_on;
};

#define DWC2_MAX_NUMBER_OF_SETUP_PKT	5
#define DWC_EP0_MAXPACKET		64
#define DWC2_MAX_FRAME			2048
#define DWC2_MAX_MICROFRAME		(2048 * 8)

/**
 * struct dwc2 - representation of our controller
 * @ctrl_req: usb control request which is used for ep0
 * @ctrl_req_addr: dma address of ctrl_req
 * @status_buf: status buf for GET_STATUS request
 * @status_buf_addr: dma address of status_buf
 * @lock: for synchronizing
 * @pdev: for Communication with the glue layer
 * @dev: pointer to our struct device
 * @dma_enable: enable DMA, 0: Slave Mode, 1: DMA Mode
 * @dma_desc_enable: enable Scatter/Gather DMA, 0: enable, 1: disable
 * @core_global_regs: Core Global registers starting at offset 000h
 * @host_if: Host-specific information
 * @pcgcctl: Power and Clock Gating Control Register
 * @hwcfgs: copy of hwcfgs registers
 * @phy_inited: The PHY need only init once, but the core can re-init many times
 * @op_state: The operational State
 * @total_fifo_size: Total RAM for FIFOs (Bytes)
 * @rx_fifo_size: Size of Rx FIFO (Bytes)
 * @nperio_tx_fifo_size: Size of Non-periodic Tx FIFO (Bytes)
 * @power_down: Power Down Enable, 0: disable, 1: enable
 * @adp_enable: ADP support Enable
 * @regs: ioremap-ed HW register base address
 * @snpsid: Value from SNPSID register
 * @otg_ver: OTG revision supported, 0: OTG 1.3, 1: OTG 2.0
 * @otg_sts: OTG status flag used for HNP polling
 * @dev_if: Device-specific information
 * @eps: endpoints, OUT[num_out_eps] then IN[num_in_eps]
 * @gadget: device side representation of the peripheral controller
 * @gadget_driver: pointer to the gadget driver
 * @three_stage_setup: set if we perform a three phase setup
 * @ep0_expect_in: true when we expect a DATA IN transfer
 * @ep0state: state of endpoint zero
 */
struct dwc2 {
	spinlock_t			 lock;
	atomic_t			 in_irq;
	int				 irq;
	int				 owner_cpu;
	int				 do_reset_core;

	struct usb_ctrlrequest		 ctrl_req;
	struct usb_ctrlrequest		*ctrl_req_virt;
	dma_addr_t			 ctrl_req_addr;
#define DWC2_CTRL_REQ_ACTUAL_ALLOC_SIZE	 (4 * PAGE_SIZE)
	int				 setup_prepared;
	int                              last_ep0out_normal;

	u16				 *status_buf;
	dma_addr_t			 status_buf_addr;
	struct dwc2_request		 ep0_usb_req;
	/*
	 * because ep0out did not allow disable operation,
	 * we use a shadow buffer here to avoid memory corruption
	 */
	u8				*ep0out_shadow_buf;
	void 				*ep0out_shadow_uncached;
	u32				 ep0out_shadow_dma;

	struct device			*dev;
	struct platform_device 		*pdev;
	int				 dma_enable;
	int				 dma_desc_enable;
	dwc_otg_core_global_regs_t	*core_global_regs;
	struct dwc2_host_if		 host_if;
	volatile uint32_t		*pcgcctl;
	struct dwc2_hwcfgs		 hwcfgs;
	int				 phy_inited;
	/** The operational State, during transations
	 * (a_host>>a_peripherial and b_device=>b_host) this may not
	 * match the core but allows the software to determine
	 * transitions.
	 */
	uint8_t				 op_state;
	/** A-Device is a_host */
#define DWC2_A_HOST		(1)
	/** A-Device is a_suspend */
#define DWC2_A_SUSPEND		(2)
	/** A-Device is a_peripherial */
#define DWC2_A_PERIPHERAL	(3)
	/** B-Device is operating as a Peripheral. */
#define DWC2_B_PERIPHERAL	(4)
	/** B-Device is operating as a Host. */
#define DWC2_B_HOST		(5)

	enum dwc2_lx_state		 lx_state;
	uint16_t			 total_fifo_size;
	uint16_t			 rx_fifo_size;
	uint16_t			 nperio_tx_fifo_size;
	uint32_t			 power_down;
	uint32_t			 adp_enable;
	void __iomem			*regs;
	uint32_t			 snpsid;
	uint32_t			 otg_ver;
	uint8_t				 otg_sts;
	struct dwc2_dev_if		 dev_if;
	struct dwc2_ep			*eps[MAX_EPS_CHANNELS * 2];
	struct usb_gadget		 gadget;
	struct usb_gadget_driver	*gadget_driver;

	unsigned			 is_selfpowered:1;
	unsigned			 three_stage_setup:1;
	unsigned			 ep0_expect_in:1;
	unsigned			 remote_wakeup_enable:1;
	unsigned			 delayed_status:1;
	unsigned			 delayed_status_sent:1;
	unsigned			 b_hnp_enable:1;
	unsigned			 a_hnp_support:1;
	unsigned			 a_alt_hnp_support:1;
	volatile unsigned	 plugin:1;
	unsigned			 keep_phy_on:1;
	/* for suspend/resume */
	volatile unsigned		 suspended:1;  /* 0: running, 1: suspended */

	unsigned int			 gintmsk;

	int pullup_on;
	enum dwc2_ep0_state		 ep0state;
	enum dwc2_device_state		 dev_state;

	struct timer_list	delayed_status_watchdog;

#define DWC2_EP0STATE_WATCH_COUNT	10
#define DWC2_EP0STATE_WATCH_INTERVAL	50 /* ms */
#ifdef CONFIG_USB_CHARGER_SOFTWARE_JUDGE
	int			ep0state_watch_count;
	struct timer_list	ep0state_watcher;
	struct work_struct	ep0state_watcher_work;
#endif

	/* Host */
	struct usb_hcd		*hcd;
	int			device_connected;
	u32			 port1_status;
	unsigned long		 rh_timer;

	struct dwc2_channel	 channel_pool[MAX_EPS_CHANNELS];
	struct list_head	 chan_free_list;
	struct list_head	 chan_trans_list;
	/*
	 * once a channel becames an isoc channel,
	 * it will always be isoc channel, because isoc channel can't disabled!
	 */
	struct list_head	idle_isoc_chan_list;
	struct list_head	busy_isoc_chan_list;

	struct kmem_cache	*qh_cachep;
	struct kmem_cache	*td_cachep;
	struct kmem_cache	*context_cachep;
	struct dwc2_isoc_qh_ptr *isoc_qh_ptr_pool;
	struct list_head	isoc_qh_ptr_list;

	struct list_head	idle_qh_list;
	struct list_head	busy_qh_list;


	struct list_head	context_list;

	unsigned int		random_frame;
	struct dwc2_frame	frame_list[MAX_FRLIST_EN_NUM];

	unsigned int		urb_queued_number;

	struct work_struct	otg_id_work;

	u32			 hcd_started:1;
	u32			 mode:2;
#define DWC2_HC_MODE_UNKNOWN	0
#define	DWC2_HC_UHCI_MODE	1
#define DWC2_HC_EHCI_MODE	2

	u32			*sw_frame_list;
	dma_addr_t		hw_frame_list;

	dwc_otg_host_dma_desc_t *sw_chan_desc_list;
	dma_addr_t		hw_chan_desc_list;

	/* Debug support */
	struct dentry		*root;
	u8			test_mode;
	u8			test_mode_nr;

	unsigned long           deps_align_addr;
};

#define USECS_INFINITE		((unsigned int)~0)

#if 0
#define dwc_writel(v, p)					\
	({							\
		printk("===>%s:%d writel %p to 0x%08x\n",		\
			__func__, __LINE__, (p), (v));			\
		writel((v), (p));					\
	})

#define dwc_readl(p)							\
	({								\
		u32 val = readl((p));					\
		printk("===>%s:%d readl %p, got 0x%08x\n",		\
			__func__, __LINE__, (p), val);			\
		val;							\
	})
#else
#if 0
#define dwc_writel(v, p) ({						\
			if ( (*(volatile unsigned int *)0xb0000020) & (1 << 2)) { \
				while(1)				\
					printk("====>enter %s:%d\n", __func__, __LINE__); \
			}						\
			writel((v), (p));				\
		})

#define dwc_readl(p) ({							\
			if ( (*(volatile unsigned int *)0xb0000020) & (1 << 2)) { \
				while(1)				\
					printk("====>enter %s:%d\n", __func__, __LINE__); \
			}						\
			readl((p));					\
		})
#else
#define dwc_writel(v, p) writel((v), (p))
#define dwc_readl(p) readl((p))
#endif
#endif

#define dwc2_spin_lock_irqsave(__dwc, flg)				\
	do {								\
		struct dwc2 *_dwc = (__dwc);				\
		spin_lock_irqsave(&_dwc->lock, (flg));			\
		if (atomic_read(&_dwc->in_irq)) {			\
			if (_dwc->owner_cpu != smp_processor_id()) {	\
				spin_unlock_irqrestore(&_dwc->lock, (flg)); \
			} else						\
				break;					\
		} else							\
			break;						\
	} while (1)

#define dwc2_spin_trylock_irqsave(__dwc, flg)				\
	do {								\
		do {							\
			struct dwc2 *_dwc = (__dwc);			\
			unsigned int loops = loops_per_jiffy * HZ / 1000; \
			int ret = 0;					\
									\
			spin_lock_irqsave(&_dwc->lock, (flg));		\
			if (atomic_read(&_dwc->in_irq)) {		\
				if (_dwc->owner_cpu != smp_processor_id()) { \
					spin_unlock_irqrestore(&_dwc->lock, (flg)); \
					loops--;			\
					if (loops == 0) {		\
						ret = -EAGAIN;		\
						break;			\
					}				\
				} else					\
					break;				\
			} else						\
				break;					\
		} while (1);						\
		(ret);							\
	} while (0)

#define dwc2_spin_unlock_irqrestore(__dwc, flg)			\
	do {							\
		struct dwc2 *_dwc = (__dwc);			\
		spin_unlock_irqrestore(&_dwc->lock, (flg));	\
	} while (0)

#define dwc2_spin_lock(__dwc)						\
	do {								\
		struct dwc2 *_dwc = (__dwc);				\
		spin_lock(&_dwc->lock);					\
	} while(0)

#define dwc2_spin_unlock(__dwc)						\
	do {								\
		struct dwc2 *_dwc = (__dwc);				\
		spin_unlock(&_dwc->lock);				\
	} while(0)

/* prototypes */

int dwc2_core_init(struct dwc2 *dwc);

int dwc2_host_init(struct dwc2 *dwc);
void dwc2_host_exit(struct dwc2 *dwc);

int dwc2_gadget_init(struct dwc2 *dwc);
void dwc2_gadget_exit(struct dwc2 *dwc);

#ifndef CONFIG_USB_CHARGER_SOFTWARE_JUDGE
static void __attribute__((unused))
dwc2_notifier_call_chain_async(struct dwc2 *dwc) {  }

static void __attribute__((unused))
dwc2_start_ep0state_watcher(struct dwc2 *dwc, int count) {  }
#else
void dwc2_start_ep0state_watcher(struct dwc2 *dwc, int count);
#endif

void dwc2_enable_common_interrupts(struct dwc2 *dwc);

#define dwc2_is_device_mode(dwc) ({					\
			uint32_t m_curmod = dwc_readl(&dwc->core_global_regs->gintsts);	\
									\
			(m_curmod & 0x1) == 0;				\
		})

#define dwc2_is_host_mode(dwc) ({					\
			uint32_t m_curmod = dwc_readl(&dwc->core_global_regs->gintsts);	\
									\
			(m_curmod & 0x1) == 1;				\
		})

#define dwc2_is_a_device(dwc) ({	\
			uint32_t m_curmod = dwc_readl(&dwc->core_global_regs->gotgctl); \
			((m_curmod >> 16)& 0x1) == 0;	\
		})

void dwc2_wait_3_phy_clocks(void);
void dwc2_core_reset(struct dwc2 *dwc);
int dwc2_disable(struct dwc2 *dwc);
int dwc2_enable(struct dwc2 *dwc);

void calculate_fifo_size(struct dwc2 *dwc);
void dwc2_flush_tx_fifo(struct dwc2 *dwc, const int num);
void dwc2_flush_rx_fifo(struct dwc2 *dwc);

void dwc2_enable_global_interrupts(struct dwc2 *dwc);
void dwc2_disable_global_interrupts(struct dwc2 *dwc);
#endif /* __DRIVERS_USB_DWC2_CORE_H */
