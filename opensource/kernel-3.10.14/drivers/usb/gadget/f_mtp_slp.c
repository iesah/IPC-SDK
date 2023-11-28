/*
 * drivers/usb/gadget/f_mtp_slp.c
 *
 * Function Driver for USB MTP,
 * mtpg.c -- MTP Driver, for MTP development,
 *
 * Copyright (C) 2009 by Samsung Electronics,
 * Author:Deepak M.G. <deepak.guru@samsung.com>,
 * Author:Madhukar.J  <madhukar.j@samsung.com>,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * f_mtp.c file is the driver for MTP device. Totally three
 * EndPoints will be configured in which 2 Bulk End Points
 * and 1 Interrupt End point. This driver will also register as
 * misc driver and exposes file operation funtions to user space.
 */

/* Includes */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/usb.h>
#include <linux/usb_usual.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/ioctl.h>
#include <linux/printk.h>
#include <linux/mm.h>

#include <linux/sched.h>
#include <asm-generic/siginfo.h>

#include "gadget_chips.h"

/*-------------------------------------------------------------------------*/
/*Only for Debug*/
#define DEBUG_MTP 0
/*#define CSY_TEST */

#if DEBUG_MTP
#define DEBUG_MTP_SETUP
#define DEBUG_MTP_READ
#define DEBUG_MTP_WRITE

#else
#undef DEBUG_MTP_SETUP
#undef DEBUG_MTP_READ
#undef DEBUG_MTP_WRITE
#endif

#ifdef DEBUG_MTP_SETUP
#define DEBUG_MTPB(fmt, args...) printk(KERN_INFO fmt, ##args)
#else
#define DEBUG_MTPB(fmt, args...) do {} while (0)
#endif

#ifdef DEBUG_MTP_READ
#define DEBUG_MTPR(fmt, args...) printk(KERN_INFO fmt, ##args)
#else
#define DEBUG_MTPR(fmt, args...) do {} while (0)
#endif

#ifdef DEBUG_MTP_WRITE
#define DEBUG_MTPW(fmt, args...) printk(KERN_INFO fmt, ##args)
#else
#define DEBUG_MTPW(fmt, args...) do {} while (0)
#endif
/*-------------------------------------------------------------------------*/

/* number of rx and tx requests to allocate */
#define MTP_RX_REQ_MAX		 8
#define MTP_TX_REQ_MAX		 10
#define MTP_INT_REQ_MAX		 2

#define MTP_DRIVER_NAME		 "usb_mtp_gadget"

#define MTP_IOCTL_LETTER	'Z'
#define GET_HIGH_FULL_SPEED	_IOR(MTP_IOCTL_LETTER, 1, int)
#define MTP_DISABLE			_IO(MTP_IOCTL_LETTER, 2)
#define MTP_CLEAR_HALT		_IO(MTP_IOCTL_LETTER, 3)
#define MTP_WRITE_INT_DATA	_IOW(MTP_IOCTL_LETTER, 4, char *)
#define SET_MTP_USER_PID	_IOW(MTP_IOCTL_LETTER, 5, int)
#define GET_SETUP_DATA		_IOR(MTP_IOCTL_LETTER, 6, char *)
#define SET_SETUP_DATA		_IOW(MTP_IOCTL_LETTER, 7, char *)
#define SEND_RESET_ACK		_IO(MTP_IOCTL_LETTER, 8)
#define SET_ZLP_DATA		_IO(MTP_IOCTL_LETTER, 9)
#define GET_MAX_PKT_SIZE	_IOR(MTP_IOCTL_LETTER, 10, void *)
#define SIG_SETUP			44

/*PIMA15740-2000 spec*/
#define USB_PTPREQUEST_CANCELIO   0x64    /* Cancel request */
#define USB_PTPREQUEST_GETEVENT   0x65    /* Get extened event data */
#define USB_PTPREQUEST_RESET      0x66    /* Reset Device */
#define USB_PTPREQUEST_GETSTATUS  0x67    /* Get Device Status */
#define USB_PTPREQUEST_CANCELIO_SIZE 6
#define USB_PTPREQUEST_GETSTATUS_SIZE 12

static const char mtp_longname[] = "mtp";

static DEFINE_MUTEX(mtp_lock);
static const char mtp_shortname[] = MTP_DRIVER_NAME;
static pid_t mtp_pid;

struct mtp_ep_descs {
	struct usb_endpoint_descriptor *bulk_in;
	struct usb_endpoint_descriptor *bulk_out;
	struct usb_endpoint_descriptor *int_in;
};

struct max_pkt_size {
	unsigned int txPktSize;
	unsigned int rxPktSize;
};

struct f_mtp {
	struct usb_function function;
	unsigned allocated_intf_num;

	struct mtp_ep_descs fs;
	struct mtp_ep_descs hs;

	struct usb_ep *bulk_in;
	struct usb_ep *bulk_out;
	struct usb_ep *int_in;

	struct list_head bulk_in_q;
	struct list_head bulk_out_q;
	struct list_head int_in_q;
	struct list_head bulk_in_emg_q;
};

/* MTP Device Structure*/
struct mtpg_dev {
	struct f_mtp *mtp_func;
	spinlock_t lock;

	int online;
	int error;

	wait_queue_head_t read_wq;
	wait_queue_head_t write_wq;
	wait_queue_head_t intr_wq;

	struct usb_request *read_req;
	unsigned char *read_buf;
	unsigned read_count;

	atomic_t read_excl;
	atomic_t write_excl;
	atomic_t ioctl_excl;
	atomic_t open_excl;
	atomic_t wintfd_excl;

	struct list_head *int_idle;
	struct list_head *tx_idle;
	struct list_head *rx_idle;
	struct list_head rx_done;
	struct list_head *tx_emg_idle;

	char cancel_io_buf[USB_PTPREQUEST_CANCELIO_SIZE + 1];

	struct max_pkt_size udc_max;

	unsigned int bulkBufSize;
	unsigned int intrBufSize;
};

struct usb_mtp_ctrlrequest {
	struct usb_ctrlrequest	setup;
};

static inline struct f_mtp *func_to_mtp(struct usb_function *f)
{
	return container_of(f, struct f_mtp, function);
}

/* Global mtpg_dev Structure
* the_mtpg variable be used between mtpg_open() and mtpg_function_bind() */
static struct mtpg_dev *the_mtpg;

/* Three full-speed and high-speed endpoint descriptors: bulk-in, bulk-out,
 * and interrupt-in. */

static struct usb_interface_descriptor mtpg_interface_desc = {
	.bLength = sizeof(mtpg_interface_desc),
	.bDescriptorType = USB_DT_INTERFACE,
	.bNumEndpoints = 3,
	.bInterfaceClass = USB_CLASS_STILL_IMAGE,
	.bInterfaceSubClass = 01,
	.bInterfaceProtocol = 01,
};

static struct usb_endpoint_descriptor fs_mtpg_in_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};

static struct usb_endpoint_descriptor fs_mtpg_out_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};

static struct usb_endpoint_descriptor int_fs_notify_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = __constant_cpu_to_le16(64),
	.bInterval = 6,
};

static struct usb_descriptor_header *fs_mtpg_desc[] = {
	(struct usb_descriptor_header *)&mtpg_interface_desc,
	(struct usb_descriptor_header *)&fs_mtpg_in_desc,
	(struct usb_descriptor_header *)&fs_mtpg_out_desc,
	(struct usb_descriptor_header *)&int_fs_notify_desc,
	NULL,
};

static struct usb_endpoint_descriptor hs_mtpg_in_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	/* bEndpointAddress = DYNAMIC */
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_mtpg_out_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	/* bEndpointAddress = DYNAMIC */
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor int_hs_notify_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = __constant_cpu_to_le16(64),
	.bInterval = 6,
};

static struct usb_descriptor_header *hs_mtpg_desc[] = {
	(struct usb_descriptor_header *)&mtpg_interface_desc,
	(struct usb_descriptor_header *)&hs_mtpg_in_desc,
	(struct usb_descriptor_header *)&hs_mtpg_out_desc,
	(struct usb_descriptor_header *)&int_hs_notify_desc,
	NULL
};

/* string IDs are assigned dynamically */
#define F_MTP_IDX			0
#define MTP_STRING_PRODUCT_IDX		1
#define MTP_STRING_SERIAL_IDX		2

/* default mtp_serial number takes at least two packets */
static char mtp_serial[36] = "0123456789.0123456789.0123456789";

static struct usb_string strings_dev_mtp[] = {
	[F_MTP_IDX].s = "Samsung MTP",
	[MTP_STRING_PRODUCT_IDX].s = mtp_longname,
	[MTP_STRING_SERIAL_IDX].s = mtp_serial,
	{},			/* end of list */
};

static struct usb_gadget_strings stringtab_mtp = {
	.language = 0x0409,	/* en-us */
	.strings = strings_dev_mtp,
};

static struct usb_gadget_strings *mtp_dev_strings[] = {
	&stringtab_mtp,
	NULL,
};

/* -------------------------------------------------------------------------
 *	Main Functionalities Start!
 * ------------------------------------------------------------------------- */

static inline int _mtp_lock(atomic_t *excl)
{
	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);
	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void _mtp_unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

/* add a request to the tail of a list */
static inline void mtp_req_put(struct mtpg_dev *dev, struct list_head *head,
			       struct usb_request *req)
{
	unsigned long flags;

	if (!dev || !req)
		return;

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);
	spin_lock_irqsave(&dev->lock, flags);
	if (head)
		list_add_tail(&req->list, head);
	spin_unlock_irqrestore(&dev->lock, flags);
}

/* remove a request from the head of a list */
static struct usb_request *mtp_req_get(struct mtpg_dev *dev,
				       struct list_head *head)
{
	unsigned long flags;
	struct usb_request *req;

	if (!dev)
		return 0;

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	spin_lock_irqsave(&dev->lock, flags);
	if (!head)
		req = NULL;
	else {
		if (list_empty(head)) {
			req = NULL;
		} else {
			req = list_first_entry(head, struct usb_request, list);
			list_del(&req->list);
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return req;
}

static void mtpg_request_free(struct usb_request *req, struct usb_ep *ep)
{

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	if (req) {
		kfree(req->buf);
		req->buf = NULL;
		req->context = NULL;
		usb_ep_free_request(ep, req);
	}
}

static struct usb_request *mtpg_request_new(struct usb_ep *ep, int buffer_size)
{

	struct usb_request *req = usb_ep_alloc_request(ep, GFP_KERNEL);

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	if (!req) {
		pr_err("******* %s \t line %d ERROR !!!\n",
			__func__, __LINE__);
		return NULL;
	}

	/* now allocate buffers for the requests */
	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	return req;
}

static int mtp_send_signal(int value)
{
	int ret;
	struct siginfo info;
	struct task_struct *t;
	memset(&info, 0, sizeof(struct siginfo));
	info.si_signo = SIG_SETUP;
	info.si_code = SI_QUEUE;
	info.si_int = value;
	rcu_read_lock();
	t = find_task_by_vpid(mtp_pid);
	if (t == NULL) {
		pr_err("no such pid\n");
		rcu_read_unlock();
		return -ENODEV;
	}

	rcu_read_unlock();
	ret = send_sig_info(SIG_SETUP, &info, t);
	if (ret < 0) {
		pr_err("error sending signal !!!!!!!!\n");
		return ret;
	}
	return 0;

}

static int mtpg_open(struct inode *ip, struct file *fp)
{
	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	if (_mtp_lock(&the_mtpg->open_excl)) {
		pr_err("mtpg_open fn -- mtpg device busy\n");
		return -EBUSY;
	}

	fp->private_data = the_mtpg;

	/* clear the error latch */

	DEBUG_MTPB("[%s] mtpg_open and clearing the error = 0\n", __func__);

	the_mtpg->error = 0;

	return 0;
}

static ssize_t mtpg_read(struct file *fp, char __user *buf,
			 size_t count, loff_t *pos)
{
	struct mtpg_dev *dev = fp->private_data;
	struct usb_request *req;
	int r = count, xfer;
	int ret;

	DEBUG_MTPR("*******[%s] and count = (%d)\n", __func__, count);

	if (_mtp_lock(&dev->read_excl)) {
		pr_err("mtpg_read fn -- mtpg device busy\n");
		return -EBUSY;
	}

	while (!(dev->online || dev->error)) {
		DEBUG_MTPR("******[%s] and line is = %d\n",
			__func__, __LINE__);
		ret = wait_event_interruptible(dev->read_wq,
				(dev->online || dev->error));
		if (ret < 0) {
			_mtp_unlock(&dev->read_excl);
			return ret;
		}
	}

	while (count > 0) {
		DEBUG_MTPR("*********[%s] and line is = %d\n",
			__func__, __LINE__);

		if (dev->error) {
			r = -EIO;
			pr_err("*******[%s]\t%d: dev->error so break r=%d\n",
				__func__, __LINE__, r);
			break;
		}

		/* if we have idle read requests, get them queued */
		while (!!(req = mtp_req_get(dev, dev->rx_idle))) {
 requeue_req:
			mutex_lock(&mtp_lock);
			if (!dev->mtp_func) {
				/* already target unbinded */
				mutex_unlock(&mtp_lock);
				pr_err("[%s:%d] target already unbinded, force-free req!!\n",
					__func__, __LINE__);
				mtpg_request_free(req, NULL);
				r = -ENODEV;
				goto fail;
			}

			req->length = dev->bulkBufSize;
			ret = usb_ep_queue(dev->mtp_func->bulk_out,
						req, GFP_ATOMIC);

			if (ret < 0) {
				r = ret;
				dev->error = 1;
				mtp_req_put(dev, dev->rx_idle, req);
				mutex_unlock(&mtp_lock);
				goto fail;
			} else {
				DEBUG_MTPR("********* [%s] rx req queue %p\n",
				   __func__, req);
			}
			mutex_unlock(&mtp_lock);
		}

		DEBUG_MTPR("*******[%s]\t%d: read_count = %d\n", __func__,
			   __LINE__, dev->read_count);

		/* if we have data pending, give it to userspace */
		if (dev->read_count > 0) {
			mutex_lock(&mtp_lock);
			if (!dev->mtp_func) {
				mutex_unlock(&mtp_lock);
				/* already target unbinded */
				pr_err("[%s:%d] target already unbinded, force-free req!!\n",
					__func__, __LINE__);
				mtpg_request_free(dev->read_req, NULL);
				r = -ENODEV;
				break;
			}

			DEBUG_MTPR("*******[%s]\t%d: read_count = %d\n",
				   __func__, __LINE__, dev->read_count);
			if (dev->read_count < count)
				xfer = dev->read_count;
			else
				xfer = count;

			if (copy_to_user(buf, dev->read_buf, xfer)) {
				mutex_unlock(&mtp_lock);
				r = -EFAULT;
				break;
			}

			dev->read_buf += xfer;
			dev->read_count -= xfer;
			buf += xfer;
			count -= xfer;

			/* if we've emptied the buffer, release the request */
			if (dev->read_count == 0) {
				DEBUG_MTPR("******[%s] and line is = %d\n",
					   __func__, __LINE__);
				mtp_req_put(dev, dev->rx_idle, dev->read_req);
				dev->read_req = NULL;
			}

			mutex_unlock(&mtp_lock);

			r = xfer;
			DEBUG_MTPR("***** [%s] \t %d: returning lenght %d\n",
				   __func__, __LINE__, r);
			goto fail;
		}

		/* wait for a request to complete */
		req = 0;
		ret = wait_event_interruptible(dev->read_wq,
			     (!!(req = mtp_req_get(dev, &dev->rx_done))
			      || dev->error));

		DEBUG_MTPR("*******[%s]\t%d: dev->error %d and req = %p\n",
			   __func__, __LINE__, dev->error, req);

		if (req) {
			/* if we got a 0-len one we need to put it back into
			 * service.  if we made it the current read req we'd
			 * be stuck forever
			 */
			if (req->actual == 0)
				goto requeue_req;

			mutex_lock(&mtp_lock);
			if (!dev->mtp_func) {
				mutex_unlock(&mtp_lock);
				/* already target unbinded */
				pr_err("[%s:%d] target already unbinded, force-free req!!\n",
					__func__, __LINE__);
				mtpg_request_free(req, NULL);
				r = -ENODEV;
				break;
			}

			dev->read_req = req;
			dev->read_count = req->actual;
			dev->read_buf = req->buf;

			mutex_unlock(&mtp_lock);
		}

		if (ret < 0) {
			r = ret;
			break;
		}
	}

 fail:
	_mtp_unlock(&dev->read_excl);
	return r;
}

static ssize_t mtpg_write(struct file *fp, const char __user *buf,
			  size_t count, loff_t *pos)
{
	struct mtpg_dev *dev = fp->private_data;
	struct usb_request *req = 0;
	int r = count, xfer, ret, try_cnt, max_try, pg_order;
	size_t tx_buf_size, start_size;
	struct list_head *tx_q_head;

	if (_mtp_lock(&dev->write_excl))
		return -EBUSY;

	while (count > 0) {
		if (dev->error) {
			r = -EIO;
			break;
		}

		/* get an idle tx request to use */
		req = 0;

		if (count > dev->bulkBufSize)
			ret = wait_event_interruptible(dev->write_wq,
			     (!!(req = mtp_req_get(dev, dev->tx_idle))
			      || dev->error));
		else
			ret = wait_event_interruptible(dev->write_wq,
			     (!!(req = mtp_req_get(dev, dev->tx_emg_idle))
			      || dev->error));

		if (ret < 0) {
			r = ret;
			break;
		} else if (!req) {
			pr_err("[%s:%d] mtp_req_get fail for count(%d)\n",
				__func__, __LINE__, count);
			continue;
		}

		mutex_lock(&mtp_lock);
		if (!dev->mtp_func) {
			/* already target unbinded */
			mutex_unlock(&mtp_lock);
			pr_err("[%s:%d] target already unbinded, force-free req!!\n",
				__func__, __LINE__);
			mtpg_request_free(req, NULL);
			r = -ENODEV;
			req = 0;
			break;
		}

		tx_q_head = (struct list_head *)req->context;
		/* check the tx_queue's origin */
		if (tx_q_head == dev->tx_idle) {
			(count > dev->udc_max.txPktSize) ?
				(xfer = dev->udc_max.txPktSize)
					: (xfer = count);

			BUG_ON(req->buf);
			try_cnt = 0;
			pg_order = get_order(xfer);
			start_size = 1UL << (pg_order + PAGE_SHIFT);
			max_try = pg_order - get_order(dev->bulkBufSize);
			BUG_ON(max_try < 0);

			for (tx_buf_size = start_size;
					((!req->buf) && (try_cnt < max_try));
					tx_buf_size = tx_buf_size >> 1) {
				req->buf = kmalloc(tx_buf_size,
					(GFP_ATOMIC | GFP_DMA | __GFP_NOWARN));
				try_cnt++;
			}

			if (!req->buf) {
				pr_info("[%s]malloc retry stop:bytes(%d), try_cnt(%d)\n",
					__func__, start_size, try_cnt);
				mtp_req_put(dev, tx_q_head, req);
				mutex_unlock(&mtp_lock);

				req = 0;
				ret = wait_event_interruptible(dev->write_wq,
				 (!!(req = mtp_req_get(dev, dev->tx_emg_idle))
				 || dev->error));

				if (ret < 0) {
					r = ret;
					break;
				} else if (!req) {
					pr_err("[%s:%d] mtp_req_get fail by emg(%d)",
						__func__, __LINE__, count);
					continue;
				}

				mutex_lock(&mtp_lock);
				if (!dev->mtp_func) {
					/* already target unbinded */
					mutex_unlock(&mtp_lock);
					pr_err("[%s:%d] target already unbinded, force-free req!!\n",
						__func__, __LINE__);
					mtpg_request_free(req, NULL);
					r = -ENODEV;
					req = 0;
					break;
				}

				BUG_ON(!req->buf);
				tx_q_head = (struct list_head *)req->context;
				tx_buf_size = dev->bulkBufSize;
			}

			if (xfer > tx_buf_size)
				xfer = tx_buf_size;

		} else if (tx_q_head == dev->tx_emg_idle) {
			BUG_ON(!req->buf);
			(count > dev->bulkBufSize) ?
				(xfer = dev->bulkBufSize) : (xfer = count);

		} else {
			mtp_req_put(dev, tx_q_head, req);
			mutex_unlock(&mtp_lock);
			pr_err("[%s:%d] check the tx_q_head failed\n",
				__func__, __LINE__);
			pr_err("[%s:%d] req->context(%p), tx_idle(%p), tx_emg(%p)\n",
				__func__, __LINE__, req->context,
				dev->tx_idle, dev->tx_emg_idle);
			dev->error = 1;
			continue;
		}

		DEBUG_MTPW("***** [%s]\t%d copy_from_user length %d\n",
			   __func__, __LINE__, xfer);

		if (copy_from_user(req->buf, buf, xfer)) {
			pr_err("[%s:%d] copy from user failed\n",
				__func__, __LINE__);
			if (tx_q_head == dev->tx_idle) {
				kfree(req->buf);
				req->buf = NULL;
			}
			mtp_req_put(dev, tx_q_head, req);
			mutex_unlock(&mtp_lock);
			r = -EFAULT;
			break;
		}

		req->length = xfer;
		ret = usb_ep_queue(dev->mtp_func->bulk_in,
					req, GFP_ATOMIC);

		if (ret < 0) {
			pr_err("[%s:%d] usb_ep_queue failed\n",
				__func__, __LINE__);
			if (tx_q_head == dev->tx_idle) {
				kfree(req->buf);
				req->buf = NULL;
			}
			mtp_req_put(dev, tx_q_head, req);
			mutex_unlock(&mtp_lock);
			dev->error = 1;
			r = ret;
			break;
		}

		mutex_unlock(&mtp_lock);

		buf += xfer;
		count -= xfer;
	}

	_mtp_unlock(&dev->write_excl);
	return r;
}

/*Fixme for Interrupt Transfer*/
static void interrupt_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct mtpg_dev *dev = the_mtpg;
	struct f_mtp *mtp_func = ep->driver_data;

	DEBUG_MTPB("[%s] \t %d req->status is = %d\n",
		__func__, __LINE__, req->status);

	if (req->status != 0)
		dev->error = 1;

	mtp_req_put(dev, &mtp_func->int_in_q, req);
	wake_up(&dev->intr_wq);
}

/*interrupt_write() is special write function for fixed size*/
static int interrupt_write(struct mtpg_dev *dev, const char __user *buf)
{
	struct usb_request *req = NULL;
	int count = dev->intrBufSize;
	int ret;

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	if (_mtp_lock(&dev->wintfd_excl)) {
		pr_err("wintfd_excl lock failed for interrupt_write\n");
		return -EBUSY;
	}

	/* get an idle tx request to use */
	ret = wait_event_interruptible(dev->intr_wq,
		     (!!(req = mtp_req_get(dev, dev->int_idle))
		      || dev->error));

	DEBUG_MTPW("[%s]\t%d: count : %d, dev->error = %d\n",
		   __func__, __LINE__, count, dev->error);

	if (ret < 0) {
		pr_err("[%s]%d wait_event return fail\n",
						 __func__, __LINE__);
		goto interrupt_write_fail;
	} else if (dev->error) {
		ret = -EIO;
		pr_err("[%s]%d dev->error so brk\n",
						 __func__, __LINE__);
		goto interrupt_write_fail;
	}

	/* we've got a free requtest from int_idle_queue*/
	DEBUG_MTPW("***** [%s]\t%d copy_from_user length %d\n",
		   __func__, __LINE__, count);

	mutex_lock(&mtp_lock);
	if (!dev->mtp_func) {
		mutex_unlock(&mtp_lock);
		ret = -ENODEV;
		goto interrupt_write_fail;
	} else if (copy_from_user(req->buf, buf, count)) {
		mutex_unlock(&mtp_lock);
		ret = -EFAULT;
		goto interrupt_write_fail;
	}

	req->length = count;
	ret = usb_ep_queue(dev->mtp_func->int_in,
				req, GFP_ATOMIC);
	mutex_unlock(&mtp_lock);

	if (ret < 0)
		dev->error = 1;
	else
		req = NULL;

interrupt_write_fail:
	if (req)
		mtp_req_put(dev, dev->int_idle, req);
	_mtp_unlock(&dev->wintfd_excl);

	return ret;
}

/*Fixme for enabling and disabling the MTP*/
static long mtpg_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{

	struct mtpg_dev *dev = fd->private_data;
	struct usb_composite_dev *cdev;
	struct usb_request *req;
	struct usb_ep *bulk_in;
	struct usb_ep *bulk_out;
	int status = 0;
	int size = 0;
	void __user *ubuf = (void __user *)arg;
	char buf[USB_PTPREQUEST_GETSTATUS_SIZE + 1] = { 0 };

	DEBUG_MTPB("[%s] with cmd:[%04x]\n", __func__, code);

	if (code == SET_MTP_USER_PID) {
		if (copy_from_user(&mtp_pid, ubuf, sizeof(mtp_pid)))
			status = -EFAULT;
		else
			pr_info("[%s] SET_MTP_USER_PID; pid = %d \tline = [%d]\n",
			   __func__, mtp_pid, __LINE__);
		return status;
	} else if (code ==  MTP_WRITE_INT_DATA) {
		return interrupt_write(dev, ubuf);
	} else if (code == GET_MAX_PKT_SIZE) {
		struct max_pkt_size size_info;

		/* Now at tx-case, we use variable size in udc-max-tx-pkt-size
		 * limitation to increase transfering perfomance.
		 * tx_queue will be allocated dynamically when it need.
		 */
		size_info.txPktSize = dev->udc_max.txPktSize;

		/* But rx-case, we use small fixed size for memory & stability
		 * rx_queues are already pre-allocated with fixed size
		 */
		size_info.rxPktSize = dev->bulkBufSize;

		if (copy_to_user(ubuf, &size_info, sizeof(struct max_pkt_size)))
			return -EIO;
		else
			return status;
	}

	mutex_lock(&mtp_lock);
	if (!dev->mtp_func) {
		pr_info("mtpg_ioctl fail, usb not yet enabled for MTP\n");
		mutex_unlock(&mtp_lock);
		return -ENODEV;
	}
	cdev = dev->mtp_func->function.config->cdev;
	req = cdev->req;
	bulk_in = dev->mtp_func->bulk_in;
	bulk_out = dev->mtp_func->bulk_out;

	switch (code) {
	case MTP_DISABLE:
		if (cdev && cdev->gadget) {
			pr_info("mtpg_ioctl for MTP_DISABLE, usb_gadget_dicon/connect!!\n");
			usb_gadget_disconnect(cdev->gadget);
			mdelay(5);
			usb_gadget_connect(cdev->gadget);
		}
		break;

	case MTP_CLEAR_HALT:
		status = usb_ep_clear_halt(bulk_in);
		if (!status)
			status = usb_ep_clear_halt(bulk_out);
		break;

	case GET_SETUP_DATA:
		if (copy_to_user(ubuf, dev->cancel_io_buf,
		     USB_PTPREQUEST_CANCELIO_SIZE))
			status = -EIO;
		break;

	case SEND_RESET_ACK:
		req->length = 0;
		status = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (status < 0)
			DEBUG_MTPB("[%s] Error at usb_ep_queue\tline = [%d]\n",
				   __func__, __LINE__);
		break;

	case SET_SETUP_DATA:
		if (copy_from_user(buf, ubuf,
				USB_PTPREQUEST_GETSTATUS_SIZE)) {
			status = -EIO;
			DEBUG_MTPR("*****[%s]\t%d: copy-from-user failed!!!!\n",
				   __func__, __LINE__);
			break;
		}
		size = buf[0];
		DEBUG_MTPB("[SET_SETUP_DATA]check data(%d):%x, %x, %x, %x\n",
				size, buf[0], buf[1], buf[2], buf[3]);
		memcpy(req->buf, buf, size);
		req->zero = 0;
		req->length = size;
		status = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (status < 0)
			DEBUG_MTPB("[%s] Error at usb_ep_queue\tline = [%d]\n",
				   __func__, __LINE__);
		break;

	case SET_ZLP_DATA:
		req = 0;
		status = wait_event_interruptible(dev->write_wq,
				(!!(req = mtp_req_get(dev, dev->tx_emg_idle))
				 || dev->error));
		if (status < 0) {
			pr_err("***** [%s]\t%d status = %d !!!!!\n",
						__func__, __LINE__, status);
			break;
		} else if (!req) {
			status = -EIO;
			break;
		}

		req->length = 0;

		status = usb_ep_queue(bulk_in, req, GFP_ATOMIC);
		if (status < 0) {
			pr_err("[%s] Error at usb_ep_queue\tline = [%d]\n",
			       __func__, __LINE__);
			mtp_req_put(dev, dev->tx_emg_idle, req);
		}
		break;

	case GET_HIGH_FULL_SPEED:
		if (!bulk_in->driver_data) {
			pr_info("USB speed does not negotiate with host\n");
			status = -ENODEV;
		} else {
			int maxpacket = bulk_in->maxpacket;
			if (copy_to_user(ubuf, &maxpacket, sizeof(int)))
				status = -EIO;
		}
		break;

	default:
		status = -ENOTTY;
	}
	mutex_unlock(&mtp_lock);
	return status;
}

static int mtpg_release_device(struct inode *ip, struct file *fp)
{
	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	if (the_mtpg != NULL)
		_mtp_unlock(&the_mtpg->open_excl);

	return 0;
}

/* file operations for MTP device /dev/usb_mtp_gadget */
static const struct file_operations mtpg_fops = {
	.owner = THIS_MODULE,
	.read = mtpg_read,
	.write = mtpg_write,
	.open = mtpg_open,
	.unlocked_ioctl = mtpg_ioctl,
	.release = mtpg_release_device,
};

static struct miscdevice mtpg_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = mtp_shortname,
	.fops = &mtpg_fops,
};

static void mtpg_complete_in(struct usb_ep *ep, struct usb_request *req)
{
	struct mtpg_dev *dev = the_mtpg;
	struct f_mtp *mtp_func = ep->driver_data;
	struct list_head *head = (struct list_head *)(req->context);

	DEBUG_MTPB("[%s] \t %d req->status is = %d\n",
		__func__, __LINE__, req->status);

	if (req->status != 0)
		dev->error = 1;

	if (head == &mtp_func->bulk_in_q) {
		kfree(req->buf);
		req->buf = NULL;
	} else if (!head) {
		pr_err("[%s] context checking fail\n", __func__);
		dev->error = 1;
		wake_up(&dev->write_wq);
		return;
	}

	mtp_req_put(dev, head, req);
	wake_up(&dev->write_wq);
}

static void mtpg_complete_out(struct usb_ep *ep, struct usb_request *req)
{
	struct mtpg_dev *dev = the_mtpg;
	struct f_mtp *mtp_func = ep->driver_data;

	DEBUG_MTPB("[%s] \tline = [%d]req->status is = %d\n",
		__func__, __LINE__, req->status);
	if (req->status != 0) {
		dev->error = 1;

		DEBUG_MTPB(" [%s] \t %d dev->error is = %d for rx_idle\n",
			   __func__, __LINE__, dev->error);
		mtp_req_put(dev, &mtp_func->bulk_out_q, req);
	} else {
		DEBUG_MTPB("[%s] \t %d for rx_done\n", __func__, __LINE__);
		mtp_req_put(dev, &dev->rx_done, req);
	}
	wake_up(&dev->read_wq);
}

/* mtp_create_bulk_endpoints() must be used after function binded */
static int mtp_create_bulk_endpoints(struct f_mtp *mtp_func,
				 struct usb_endpoint_descriptor *in_desc,
				 struct usb_endpoint_descriptor *out_desc,
				 struct usb_endpoint_descriptor *intr_desc)
{
	struct usb_composite_dev *cdev = mtp_func->function.config->cdev;
	struct usb_request *req;
	struct mtpg_dev *dev = the_mtpg;
	struct usb_ep *ep;
	int i;

	ep = usb_ep_autoconfig(cdev->gadget, in_desc);
	if (!ep) {
		pr_err("Error in usb_ep_autoconfig for IN DESC Failed !!!!!!!!!!\n");
		return -ENODEV;
	}
	ep->driver_data = cdev;	/* claim the endpoint */
	mtp_func->bulk_in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, out_desc);
	if (!ep) {
		pr_err("Error in usb_ep_autoconfig for OUT DESC Failed !!!!!!!!!!\n");
		return -ENODEV;
	}
	ep->driver_data = cdev;	/* claim the endpoint */
	mtp_func->bulk_out = ep;

	/* Interrupt Support for MTP */
	ep = usb_ep_autoconfig(cdev->gadget, intr_desc);
	if (!ep) {
		pr_err("Error in usb_ep_autoconfig for INT IN DESC Failed !!!!!!!!!!\n");
		return -ENODEV;
	}
	ep->driver_data = cdev;
	mtp_func->int_in = ep;

	for (i = 0; i < MTP_INT_REQ_MAX; i++) {
		req = mtpg_request_new(mtp_func->int_in,
					dev->intrBufSize);
		if (!req)
			goto ep_alloc_fail_intr;
		req->complete = interrupt_complete;
		mtp_req_put(dev, &mtp_func->int_in_q, req);
	}

	for (i = 0; i < MTP_RX_REQ_MAX; i++) {
		req = mtpg_request_new(mtp_func->bulk_out,
					dev->bulkBufSize);
		if (!req)
			goto ep_alloc_fail_out;
		req->complete = mtpg_complete_out;
		mtp_req_put(dev, &mtp_func->bulk_out_q, req);
	}

	for (i = 0; i < MTP_TX_REQ_MAX; i++) {
		req = usb_ep_alloc_request(mtp_func->bulk_in,
					GFP_KERNEL);
		if (!req)
			goto ep_alloc_fail_in;
		req->complete = mtpg_complete_in;
		req->context = (void *)&mtp_func->bulk_in_q;
		mtp_req_put(dev, &mtp_func->bulk_in_q, req);
	}

	for (i = 0; i < DIV_ROUND_UP(MTP_TX_REQ_MAX, 4); i++) {
		req = mtpg_request_new(mtp_func->bulk_in,
					dev->bulkBufSize);
		if (!req)
			goto ep_alloc_fail_all;
		req->complete = mtpg_complete_in;
		req->context = (void *)&mtp_func->bulk_in_emg_q;
		mtp_req_put(dev, &mtp_func->bulk_in_emg_q, req);
	}

	return 0;

 ep_alloc_fail_all:
	while (!!(req = mtp_req_get(dev, &mtp_func->bulk_in_emg_q)))
		mtpg_request_free(req, mtp_func->bulk_in);

 ep_alloc_fail_in:
	while (!!(req = mtp_req_get(dev, &mtp_func->bulk_in_q)))
		mtpg_request_free(req, mtp_func->bulk_in);

 ep_alloc_fail_out:
	while (!!(req = mtp_req_get(dev, &mtp_func->bulk_out_q)))
		mtpg_request_free(req, mtp_func->bulk_out);

 ep_alloc_fail_intr:
	while (!!(req = mtp_req_get(dev, &mtp_func->int_in_q)))
		mtpg_request_free(req, mtp_func->int_in);

	return -ENOMEM;
}

static void mtpg_function_unbind(struct usb_configuration *c,
				 struct usb_function *f)
{
	struct mtpg_dev *dev = the_mtpg;
	struct f_mtp *mtp_func = func_to_mtp(f);
	struct usb_request *req;

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	dev->online = 0;
	dev->error = 1;

	usb_free_all_descriptors(f);

	mutex_lock(&mtp_lock);

	while (!!(req = mtp_req_get(dev, &mtp_func->int_in_q)))
		mtpg_request_free(req, mtp_func->int_in);

	while (!!(req = mtp_req_get(dev, &mtp_func->bulk_out_q)))
		mtpg_request_free(req, mtp_func->bulk_out);

	while (!!(req = mtp_req_get(dev, &mtp_func->bulk_in_q)))
		mtpg_request_free(req, mtp_func->bulk_in);

	while (!!(req = mtp_req_get(dev, &mtp_func->bulk_in_emg_q)))
		mtpg_request_free(req, mtp_func->bulk_in);

	while (!!(req = mtp_req_get(dev, &dev->rx_done)))
		mtpg_request_free(req, mtp_func->bulk_out);

	if (dev->read_req) {
		mtpg_request_free(dev->read_req, mtp_func->bulk_out);
		dev->read_buf = NULL;
		dev->read_count = 0;
		dev->read_req = NULL;
	}

	kfree(mtp_func);
	dev->mtp_func = NULL;

	mutex_unlock(&mtp_lock);

	wake_up(&dev->read_wq);
	wake_up(&dev->write_wq);
	wake_up(&dev->intr_wq);
}

static int mtpg_function_bind(struct usb_configuration *c,
			      struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_mtp *mtp_func = func_to_mtp(f);
	int rc, id;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	id = usb_interface_id(c, f);
	if (id < 0) {
		pr_err("Error in usb_string_id Failed !!!\n");
		return id;
	}

	mtpg_interface_desc.bInterfaceNumber = id;
	mtp_func->allocated_intf_num = id;

	rc = mtp_create_bulk_endpoints(mtp_func, &fs_mtpg_in_desc,
				   &fs_mtpg_out_desc, &int_fs_notify_desc);
	if (rc) {
		pr_err("mtpg unable to autoconfigure all endpoints\n");
		return rc;
	}

	f->fs_descriptors = usb_copy_descriptors(fs_mtpg_desc);
	if (!f->fs_descriptors)
		goto desc_alloc_fail;

	if (gadget_is_dualspeed(cdev->gadget)) {
		DEBUG_MTPB("[%s] \tdual speed line = [%d]\n", __func__,
				__LINE__);

		/* Assume endpoint addresses are the same for both speeds */
		hs_mtpg_in_desc.bEndpointAddress =
			fs_mtpg_in_desc.bEndpointAddress;
		hs_mtpg_out_desc.bEndpointAddress =
			fs_mtpg_out_desc.bEndpointAddress;
		int_hs_notify_desc.bEndpointAddress =
			int_fs_notify_desc.bEndpointAddress;

		f->hs_descriptors = usb_copy_descriptors(hs_mtpg_desc);
		if (!f->hs_descriptors)
			goto desc_alloc_fail;
	}

	return 0;

 desc_alloc_fail:
	usb_free_all_descriptors(f);

	return -ENOMEM;
}

static int mtpg_function_set_alt(struct usb_function *f,
				 unsigned intf, unsigned alt)
{
	struct f_mtp *mtp_func = func_to_mtp(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct mtpg_dev *dev = the_mtpg;
	int ret;

	if (intf == mtp_func->allocated_intf_num) {
		/* "set_alt" command uses for reset interface, so each ep
		 * can be disabled by this command according to Host state
		 * To prevent ep access from user-space (read/write)
		 * while this command processing, disable "online" status
		 */
		dev->online = 0;

		if (mtp_func->int_in->driver_data)
			usb_ep_disable(mtp_func->int_in);
		else if (config_ep_by_speed(cdev->gadget, f, mtp_func->int_in))
			return -EINVAL;

		ret = usb_ep_enable(mtp_func->int_in);
		if (ret) {
			usb_ep_disable(mtp_func->int_in);
			pr_err("[%s] Enable Int-In EP error!!!(%d)\n",
			       __func__, ret);
			return ret;
		}
		mtp_func->int_in->driver_data = mtp_func;

		if (mtp_func->bulk_in->driver_data)
			usb_ep_disable(mtp_func->bulk_in);
		else if (config_ep_by_speed(cdev->gadget,
				f, mtp_func->bulk_in)) {
			usb_ep_disable(mtp_func->int_in);
			mtp_func->int_in->driver_data = NULL;
			return -EINVAL;
		}

		ret = usb_ep_enable(mtp_func->bulk_in);
		if (ret) {
			usb_ep_disable(mtp_func->int_in);
			mtp_func->int_in->driver_data = NULL;
			pr_err("[%s] Enable Bulk-In EP error!!!(%d)\n",
			       __func__, ret);
			return ret;
		}
		mtp_func->bulk_in->driver_data = mtp_func;

		if (mtp_func->bulk_out->driver_data)
			usb_ep_disable(mtp_func->bulk_out);
		else if (config_ep_by_speed(cdev->gadget,
				f, mtp_func->bulk_out)) {
			usb_ep_disable(mtp_func->int_in);
			mtp_func->int_in->driver_data = NULL;
			usb_ep_disable(mtp_func->bulk_in);
			mtp_func->bulk_in->driver_data = NULL;
			return -EINVAL;
		}

		ret = usb_ep_enable(mtp_func->bulk_out);
		if (ret) {
			usb_ep_disable(mtp_func->int_in);
			mtp_func->int_in->driver_data = NULL;
			usb_ep_disable(mtp_func->bulk_in);
			mtp_func->bulk_in->driver_data = NULL;
			pr_err("[%s] Enable Bulk-Out EP error!!!(%d)\n",
			       __func__, ret);
			return ret;
		}
		mtp_func->bulk_out->driver_data = mtp_func;

		dev->int_idle = &mtp_func->int_in_q;
		dev->tx_idle = &mtp_func->bulk_in_q;
		dev->rx_idle = &mtp_func->bulk_out_q;
		dev->tx_emg_idle = &mtp_func->bulk_in_emg_q;
		dev->mtp_func = mtp_func;
		dev->online = 1;

		/* readers may be blocked waiting for us to go online */
		wake_up(&dev->read_wq);

		return 0;
	} else {
		pr_err("[%s] error , intf = %u , alt = %u",
			__func__, intf, alt);
		return -EINVAL;
	}

}

static void mtpg_function_disable(struct usb_function *f)
{

	struct mtpg_dev *dev = the_mtpg;
	struct f_mtp *mtp_func = func_to_mtp(f);

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	dev->online = 0;
	dev->error = 1;

	spin_lock(&dev->lock);
	dev->int_idle = NULL;
	dev->tx_idle = NULL;
	dev->rx_idle = NULL;
	dev->tx_emg_idle = NULL;
	spin_unlock(&dev->lock);

	usb_ep_disable(mtp_func->int_in);
	mtp_func->int_in->driver_data = NULL;

	usb_ep_disable(mtp_func->bulk_in);
	mtp_func->bulk_in->driver_data = NULL;

	usb_ep_disable(mtp_func->bulk_out);
	mtp_func->bulk_out->driver_data = NULL;

	wake_up(&dev->read_wq);
	wake_up(&dev->write_wq);
	wake_up(&dev->intr_wq);
}

/*PIMA15740-2000 spec: Class specific setup request for MTP*/
static void mtp_complete_cancel_io(struct usb_ep *ep, struct usb_request *req)
{
	struct mtpg_dev *dev = ep->driver_data;
	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);
	if (req->status != 0) {
		DEBUG_MTPB("[%s] req->status !=0 \tline = [%d]\n",
			__func__, __LINE__);
		return;
	}

	if (req->length != USB_PTPREQUEST_CANCELIO_SIZE)
		usb_ep_set_halt(ep);
	else {
		memset(dev->cancel_io_buf, 0, USB_PTPREQUEST_CANCELIO_SIZE + 1);
		memcpy(dev->cancel_io_buf, req->buf,
		       USB_PTPREQUEST_CANCELIO_SIZE);
		mtp_send_signal(USB_PTPREQUEST_CANCELIO);
	}
}

static int mtpg_function_setup(struct usb_function *f,
			       const struct usb_ctrlrequest *ctrl)
{
	struct mtpg_dev *dev = the_mtpg;
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request *req = cdev->req;
	int signal_request = 0;
	int value = -EOPNOTSUPP;
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);
	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case (((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_CANCELIO):
		DEBUG_MTPB("[%s] USB_PTPREQUEST_CANCELIO \tline = [%d]\n",
					 __func__, __LINE__);
		if (w_value == 0x00
		    && w_index == mtpg_interface_desc.bInterfaceNumber
		    && w_length == 0x06) {
			value = w_length;
			cdev->gadget->ep0->driver_data = dev;
			req->complete = mtp_complete_cancel_io;
			req->zero = 0;
			req->length = value;
			value =
			    usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		}
		return value;
		break;
	case (((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_RESET):
		DEBUG_MTPB("[%s] USB_PTPREQUEST_RESET \tline = [%d]\n",
			   __func__, __LINE__);
		signal_request = USB_PTPREQUEST_RESET;
		break;

	case (((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_GETSTATUS):
		DEBUG_MTPB("[%s] USB_PTPREQUEST_GETSTATUS \tline = [%d]\n",
			   __func__, __LINE__);
		signal_request = USB_PTPREQUEST_GETSTATUS;
		break;

	case (((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_PTPREQUEST_GETEVENT):
		DEBUG_MTPB("[%s] USB_PTPREQUEST_GETEVENT \tline = [%d]\n",
			   __func__, __LINE__);
		signal_request = USB_PTPREQUEST_GETEVENT;
		break;
	default:
		DEBUG_MTPB("[%s] INVALID REQUEST \tline = [%d]\n",
			__func__, __LINE__);
		return value;
	}
	value = mtp_send_signal(signal_request);
	return value;
}

static int mtp_bind_config(struct usb_configuration *c)
{
	int rc;
	struct f_mtp *mtp_func;

	DEBUG_MTPB("[%s] \tline = [%d]\n", __func__, __LINE__);

	if (!the_mtpg) {
		pr_err("Error There is no the_mtpg!!\n");
		return -ENODEV;
	}

	mtp_func = kzalloc(sizeof(*mtp_func), GFP_KERNEL);
	if (!mtp_func) {
		pr_err("mtp_func memory alloc failed !!!\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&mtp_func->bulk_out_q);
	INIT_LIST_HEAD(&mtp_func->bulk_in_q);
	INIT_LIST_HEAD(&mtp_func->int_in_q);
	INIT_LIST_HEAD(&mtp_func->bulk_in_emg_q);

	mtp_func->function.name = mtp_longname;
	mtp_func->function.strings = mtp_dev_strings;

	mtp_func->function.bind = mtpg_function_bind;
	mtp_func->function.unbind = mtpg_function_unbind;
	mtp_func->function.set_alt = mtpg_function_set_alt;
	mtp_func->function.setup = mtpg_function_setup;
	mtp_func->function.disable = mtpg_function_disable;

	rc = usb_add_function(c, &mtp_func->function);
	if (rc != 0)
		pr_err("Error in usb_add_function Failed !!!\n");

	return rc;
}

static int mtp_setup(struct usb_composite_dev *cdev)
{
	struct mtpg_dev *mtpg;
	int status;

	mtpg = kzalloc(sizeof(*mtpg), GFP_KERNEL);
	if (!mtpg) {
		pr_err("mtpg_dev_alloc memory  failed !!!\n");
		return -ENOMEM;
	}

	if (strings_dev_mtp[F_MTP_IDX].id == 0) {
		status = usb_string_id(cdev);
		if (status < 0) {
			kfree(mtpg);
			return status;
		}
		strings_dev_mtp[F_MTP_IDX].id = status;
		mtpg_interface_desc.iInterface = status;
	}

	/*
	 * Fix or, add me for UDC specific transferable size
	 * Each UDC has different transferable size at once
	 */
	/* S.LSI udc's max transferable size is 512K-1 */
	mtpg->udc_max.txPktSize = mtpg->udc_max.rxPktSize =
		round_down(0x7FFFF, 512);

	/* bulkBufSize & intrBufSize have to smaller
	 * than udc's max transferable size or same
	 * and will be considering memory usage
	 */
	mtpg->bulkBufSize = round_down(0x8000, 512);
	mtpg->intrBufSize = 22;

	pr_info("[MTP SizeInfo] %s(%d), %s(%d), %s(%d), %s(%d)\n",
		"udc_txPktMaxSize", mtpg->udc_max.txPktSize,
		"udc_rxPktMaxSize", mtpg->udc_max.rxPktSize,
		"BulkBufSize", mtpg->bulkBufSize,
		"IntrBufSize", mtpg->intrBufSize);

	spin_lock_init(&mtpg->lock);
	init_waitqueue_head(&mtpg->read_wq);
	init_waitqueue_head(&mtpg->write_wq);
	init_waitqueue_head(&mtpg->intr_wq);

	atomic_set(&mtpg->open_excl, 0);
	atomic_set(&mtpg->read_excl, 0);
	atomic_set(&mtpg->write_excl, 0);
	atomic_set(&mtpg->wintfd_excl, 0);

	INIT_LIST_HEAD(&mtpg->rx_done);

	/* the_mtpg must be set before calling usb_gadget_register_driver */
	the_mtpg = mtpg;

	status = misc_register(&mtpg_device);
	if (status != 0) {
		pr_err("Error in misc_register of mtpg_device Failed !!!\n");
		kfree(mtpg);
		the_mtpg = NULL;
	}

	return status;
}

static void mtp_cleanup(void)
{
	struct mtpg_dev *mtpg = the_mtpg;

	misc_deregister(&mtpg_device);

	if (!mtpg)
		return;
	the_mtpg = NULL;
	kfree(mtpg);
}

MODULE_AUTHOR("Deepak And Madhukar");
MODULE_LICENSE("GPL");
