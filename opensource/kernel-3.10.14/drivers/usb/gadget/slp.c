/*
 * Gadget Driver for SLP based on Android
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 * Modified : Yongsul Oh <yongsul96.oh@samsung.com>
 *
 * Heavily based on android.c
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define DEBUG
#define VERBOSE_DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
//#include <asm/system_info.h>
#include <linux/pm_qos.h>
#include <linux/workqueue.h>

#include "gadget_chips.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */

#include "f_sdb.c"
#include "f_acm.c"
#include "f_mtp_slp.c"
#define USB_ETH_RNDIS y
#define USB_FRNDIS_INCLUDED y
#include "f_rndis.c"
#include "rndis.c"
#include "u_ether.c"

#define USB_MODE_VERSION	"1.1"

MODULE_AUTHOR("Yongsul Oh <yongsul96.oh@samsung.com>");
MODULE_DESCRIPTION("SLP Composite USB Driver similar to Android Compiste");
MODULE_LICENSE("GPL");
MODULE_VERSION(USB_MODE_VERSION);

static const char slp_longname[] = "Gadget SLP";

/* Default vendor and product IDs, overridden by userspace */
#define VENDOR_ID		0x04E8	/* Samsung VID */
#define PRODUCT_ID		0x6860	/* KIES mode PID */

/* DM_PORT NUM : /dev/ttyGS* port number */
#define DM_PORT_NUM            1

/* Moved from include/linux/usb/slp_multi.h */
enum slp_multi_config_id {
	USB_CONFIGURATION_1 = 1,
	USB_CONFIGURATION_2 = 2,
	USB_CONFIGURATION_DUAL = 0xFF,
};

struct slp_multi_func_data {
	const char *name;
	enum slp_multi_config_id usb_config_id;
};

struct slp_multi_usb_function {
	char *name;
	void *config;

	struct device *dev;
	char *dev_name;
	struct device_attribute **attributes;

	/* for slp_multi_dev.funcs_fconf */
	struct list_head fconf_list;

	/* for slp_multi_dev.funcs_sconf */
	struct list_head sconf_list;

	/* for slp_multi_dev.available_functions */
	struct list_head available_list;

	/* Manndatory: initialization during gadget bind */
	int (*init) (struct slp_multi_usb_function *,
					struct usb_composite_dev *);
	/* Optional: cleanup during gadget unbind */
	void (*cleanup) (struct slp_multi_usb_function *);
	/* Optional: called when the function is added the list of
	 *		enabled functions */
	void (*enable)(struct slp_multi_usb_function *);
	/* Optional: called when it is removed */
	void (*disable)(struct slp_multi_usb_function *);

	/* Mandatory: called when the usb enabled */
	int (*bind_config) (struct slp_multi_usb_function *,
			    struct usb_configuration *);
	/* Optional: called when the configuration is removed */
	void (*unbind_config) (struct slp_multi_usb_function *,
			       struct usb_configuration *);
	/* Optional: handle ctrl requests before the device is configured */
	int (*ctrlrequest) (struct slp_multi_usb_function *,
			    struct usb_composite_dev *,
			    const struct usb_ctrlrequest *);
};

struct slp_multi_dev {
	struct list_head available_functions;

	/* for each configuration control */
	struct list_head funcs_fconf;
	struct list_head funcs_sconf;

	struct usb_composite_dev *cdev;
	struct device *dev;

	bool enabled;
	bool dual_config;
	int disable_depth;
	struct mutex mutex;
	bool connected;
	bool sw_connected;

	/* to check USB suspend/resume */
	unsigned int suspended:1;

	/* to control DMA QOS */
	char pm_qos[5];
	s32 swfi_latency;
	s32 curr_latency;
	struct pm_qos_request pm_qos_req_dma;
	struct work_struct qos_work;
	char ffs_aliases[256];
};

/* TODO: only enabled 'rndis' and 'sdb'. need to verify more functions */
static const char * const default_funcs[] = {"rndis", "sdb", "mtp", "acm"};
static unsigned slp_multi_nluns;
static struct class *slp_multi_class;
static struct slp_multi_dev *_slp_multi_dev;
static int slp_multi_bind_config(struct usb_configuration *c);
static void slp_multi_unbind_config(struct usb_configuration *c);

/* string IDs are assigned dynamically */
#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

static char manufacturer_string[256];
static char product_string[256];
static char serial_string[256];

/* String Table */
static struct usb_string strings_dev[] = {
	[STRING_MANUFACTURER_IDX].s = manufacturer_string,
	[STRING_PRODUCT_IDX].s = product_string,
	[STRING_SERIAL_IDX].s = serial_string,
	{}			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language = 0x0409,	/* en-us */
	.strings = strings_dev,
};

static struct usb_gadget_strings *slp_dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength		= sizeof(device_desc),
	.bDescriptorType	= USB_DT_DEVICE,
	.bcdUSB			= __constant_cpu_to_le16(0x0200),
	.bDeviceClass		= USB_CLASS_PER_INTERFACE,
	.idVendor		= __constant_cpu_to_le16(VENDOR_ID),
	.idProduct		= __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice		= __constant_cpu_to_le16(0xffff),
	.bNumConfigurations	= 1,
};

static struct usb_configuration first_config_driver = {
	.label			= "slp_first_config",
	.unbind			= slp_multi_unbind_config,
	.bConfigurationValue	= USB_CONFIGURATION_1,
	.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.MaxPower		= 0x30,	/* 96ma */
};

static struct usb_configuration second_config_driver = {
	.label			= "slp_second_config",
	.unbind			= slp_multi_unbind_config,
	.bConfigurationValue	= USB_CONFIGURATION_2,
	.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.MaxPower		= 0x30,	/* 96ma */
};

/*-------------------------------------------------------------------------*/
/* Supported functions initialization */

static int sdb_function_init(struct slp_multi_usb_function *f,
			     struct usb_composite_dev *cdev)
{
	return sdb_setup(cdev);
}

static void sdb_function_cleanup(struct slp_multi_usb_function *f)
{
	sdb_cleanup();
}

static int sdb_function_bind_config(struct slp_multi_usb_function *f,
				    struct usb_configuration *c)
{
	return sdb_bind_config(c);
}

static struct slp_multi_usb_function sdb_function = {
	.name = "sdb",
	.init = sdb_function_init,
	.cleanup = sdb_function_cleanup,
	.bind_config = sdb_function_bind_config,
};

#define MAX_ACM_INSTANCES 4
struct acm_function_config {
	int instances;
	int instances_on;
	struct usb_function *f_acm[MAX_ACM_INSTANCES];
	struct usb_function_instance *f_acm_inst[MAX_ACM_INSTANCES];
};

static int
acm_function_init(struct slp_multi_usb_function *f,
		  struct usb_composite_dev *cdev)
{
	int i;
	int ret;
	struct acm_function_config *config;

	config = kzalloc(sizeof(struct acm_function_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;
	f->config = config;
	config->instances = 1;

	for (i = 0; i < MAX_ACM_INSTANCES; i++) {
		config->f_acm_inst[i] = usb_get_function_instance("acm");
		if (IS_ERR(config->f_acm_inst[i])) {
			ret = PTR_ERR(config->f_acm_inst[i]);
			goto err_usb_get_function_instance;
		}
	}
	return 0;

err_usb_get_function_instance:
	while (i-- > 0)
		usb_put_function_instance(config->f_acm_inst[i]);
	kfree(f->config);
	f->config = NULL;
	return ret;
}

static void acm_function_cleanup(struct slp_multi_usb_function *f)
{
	int i;
	struct acm_function_config *config = f->config;

	for (i = 0; i < MAX_ACM_INSTANCES; i++)
		usb_put_function_instance(config->f_acm_inst[i]);
	kfree(f->config);
	f->config = NULL;
}

static int
acm_function_bind_config(struct slp_multi_usb_function *f,
			 struct usb_configuration *c)
{
	int i;
	int ret = 0;
	struct acm_function_config *config = f->config;

	for (i = 0; i < MAX_ACM_INSTANCES; i++) {
		config->f_acm[i] = usb_get_function(config->f_acm_inst[i]);
		if (IS_ERR(config->f_acm[i])) {
			ret = PTR_ERR(config->f_acm[i]);
			goto err_usb_get_function;
		}
	}

	config->instances_on = config->instances;
	for (i = 0; i < config->instances_on; i++) {
		ret = usb_add_function(c, config->f_acm[i]);
		if (ret) {
			pr_err("Could not bind acm%u config\n", i);
			goto err_usb_add_function;
		}
	}

	return 0;

err_usb_add_function:
	while (i-- > 0)
		usb_remove_function(c, config->f_acm[i]);
	i = MAX_ACM_INSTANCES;
err_usb_get_function:
	while (i-- > 0)
		usb_put_function(config->f_acm[i]);
	return ret;
}

static void acm_function_unbind_config(struct slp_multi_usb_function *f,
				       struct usb_configuration *c)
{
	int i;
	struct acm_function_config *config = f->config;

	for (i = 0; i < MAX_ACM_INSTANCES; i++)
		usb_put_function(config->f_acm[i]);
}

static ssize_t acm_instances_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	return sprintf(buf, "%d\n", config->instances);
}

static ssize_t acm_instances_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	int value;

	sscanf(buf, "%d", &value);
	if (value > MAX_ACM_INSTANCES)
		value = MAX_ACM_INSTANCES;
	config->instances = value;
	return size;
}

static DEVICE_ATTR(instances, S_IRUGO | S_IWUSR, acm_instances_show,
						 acm_instances_store);
static struct device_attribute *acm_function_attributes[] = {
	&dev_attr_instances,
	NULL
};

static struct slp_multi_usb_function acm_function = {
	.name		= "acm",
	.init		= acm_function_init,
	.cleanup	= acm_function_cleanup,
	.bind_config	= acm_function_bind_config,
	.unbind_config	= acm_function_unbind_config,
	.attributes	= acm_function_attributes,
};

static int
mtp_function_init(struct slp_multi_usb_function *f,
		struct usb_composite_dev *cdev)
{
	return mtp_setup(cdev);
}

static void mtp_function_cleanup(struct slp_multi_usb_function *f)
{
	mtp_cleanup();
}

static int
mtp_function_bind_config(struct slp_multi_usb_function *f,
		struct usb_configuration *c)
{
	return mtp_bind_config(c);
}

static int mtp_function_ctrlrequest(struct slp_multi_usb_function *f,
					struct usb_composite_dev *cdev,
					const struct usb_ctrlrequest *c)
{
	struct usb_request *req = cdev->req;
	struct usb_gadget *gadget = cdev->gadget;
	int value = -EOPNOTSUPP;
	u16 w_length = le16_to_cpu(c->wLength);
	struct usb_string_descriptor *os_func_desc = req->buf;
	char ms_descriptor[38] = {
		/* Header section */
		/* Upper 2byte of dwLength */
		0x00, 0x00,
		/* bcd Version */
		0x00, 0x01,
		/* wIndex, Extended compatID index */
		0x04, 0x00,
		/* bCount, we use only 1 function(MTP) */
		0x01,
		/* RESERVED */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		/* First Function section for MTP */
		/* bFirstInterfaceNumber,
		 * we always use it by 0 for MTP
		 */
		0x00,
		/* RESERVED, fixed value 1 */
		0x01,
		/* CompatibleID for MTP */
		0x4D, 0x54, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* Sub-compatibleID for MTP */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* RESERVED */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	switch (c->bRequest) {
		/* Added handler to respond to host about MS OS Descriptors.
		 * Below handler is requirement if you use MTP.
		 * So, If you set composite included MTP,
		 * you have to respond to host about 0x54 or 0x64 request
		 * refer to following site.
		 * http://msdn.microsoft.com/en-us/windows/hardware/gg463179
		 */
	case 0x54:
	case 0x6F:
		os_func_desc->bLength = 0x28;
		os_func_desc->bDescriptorType = 0x00;
		value = min(w_length, (u16) (sizeof(ms_descriptor) + 2));
		memcpy(os_func_desc->wData, &ms_descriptor, value);

		if (value >= 0) {
			req->length = value;
			req->zero = value < w_length;
			value = usb_ep_queue(gadget->ep0, req, GFP_ATOMIC);
			if (value < 0) {
				req->status = 0;
				cdev->req->complete(gadget->ep0, req);
			}
		}
		break;
	}

	return value;
}

static struct slp_multi_usb_function mtp_function = {
	.name		= "mtp",
	.init		= mtp_function_init,
	.cleanup	= mtp_function_cleanup,
	.bind_config	= mtp_function_bind_config,
	.ctrlrequest	= mtp_function_ctrlrequest,
};

struct rndis_function_config {
	u8 ethaddr[ETH_ALEN];
	u32 vendorID;
	char manufacturer[256];
	bool wceis;
	u8 rndis_string_defs0_id;
	struct usb_function *f_rndis;
	struct usb_function_instance *fi_rndis;
};
static char host_addr_string[18];

static int rndis_function_init(struct slp_multi_usb_function *f,
			       struct usb_composite_dev *cdev)
{
	struct rndis_function_config *config;
	int status, i;

	config = kzalloc(sizeof(struct rndis_function_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	config->fi_rndis = usb_get_function_instance("rndis");
	if (IS_ERR(config->fi_rndis)) {
		status = PTR_ERR(config->fi_rndis);
		goto rndis_alloc_inst_error;
	}

	/* maybe allocate device-global string IDs */
	if (rndis_string_defs[0].id == 0) {

		/* control interface label */
		status = usb_string_id(cdev);
		if (status < 0)
			goto rndis_init_error;
		config->rndis_string_defs0_id = status;
		rndis_string_defs[0].id = status;
		rndis_control_intf.iInterface = status;

		/* data interface label */
		status = usb_string_id(cdev);
		if (status < 0)
			goto rndis_init_error;
		rndis_string_defs[1].id = status;
		rndis_data_intf.iInterface = status;

		/* IAD iFunction label */
		status = usb_string_id(cdev);
		if (status < 0)
			goto rndis_init_error;
		rndis_string_defs[2].id = status;
		rndis_iad_descriptor.iFunction = status;
	}

	/* create a fake MAC address from our serial number. */
	for (i = 0; (i < 256) && serial_string[i]; i++) {
		/* XOR the USB serial across the remaining bytes */
		config->ethaddr[i % (ETH_ALEN - 1) + 1] ^= serial_string[i];
	}
	config->ethaddr[0] &= 0xfe;	/* clear multicast bit */
	config->ethaddr[0] |= 0x02;	/* set local assignment bit (IEEE802) */

	snprintf(host_addr_string, sizeof(host_addr_string),
		"%02x:%02x:%02x:%02x:%02x:%02x",
		config->ethaddr[0],	config->ethaddr[1],
		config->ethaddr[2],	config->ethaddr[3],
		config->ethaddr[4], config->ethaddr[5]);

	f->config = config;
	return 0;

 rndis_init_error:
	usb_put_function_instance(config->fi_rndis);
rndis_alloc_inst_error:
	kfree(config);
	return status;
}

static void rndis_function_cleanup(struct slp_multi_usb_function *f)
{
	struct rndis_function_config *rndis = f->config;

	usb_put_function_instance(rndis->fi_rndis);
	kfree(rndis);
	f->config = NULL;
}

static int rndis_function_bind_config(struct slp_multi_usb_function *f,
				      struct usb_configuration *c)
{
	int ret = -EINVAL;
	struct rndis_function_config *rndis = f->config;
	struct f_rndis_opts *rndis_opts;
	struct net_device *net;

	if (!rndis) {
		dev_err(f->dev, "error rndis_pdata is null\n");
		return ret;
	}

	rndis_opts =
		container_of(rndis->fi_rndis, struct f_rndis_opts, func_inst);
	net = rndis_opts->net;

	gether_set_qmult(net, QMULT_DEFAULT);
	gether_set_host_addr(net, host_addr_string);
	gether_set_dev_addr(net, host_addr_string);
	rndis_opts->vendor_id = rndis->vendorID;
	rndis_opts->manufacturer = rndis->manufacturer;

	if (rndis->wceis) {
		/* "Wireless" RNDIS; auto-detected by Windows */
		rndis_iad_descriptor.bFunctionClass =
		    USB_CLASS_WIRELESS_CONTROLLER;
		rndis_iad_descriptor.bFunctionSubClass = 0x01;
		rndis_iad_descriptor.bFunctionProtocol = 0x03;
		rndis_control_intf.bInterfaceClass =
		    USB_CLASS_WIRELESS_CONTROLLER;
		rndis_control_intf.bInterfaceSubClass = 0x01;
		rndis_control_intf.bInterfaceProtocol = 0x03;
	}

	/* Android team reset "rndis_string_defs[0].id" when RNDIS unbinded
	 * in f_rndis.c but, that makes failure of rndis_bind_config() by
	 * the overflow of "next_string_id" value in usb_string_id().
	 * So, Android team also reset "next_string_id" value in android.c
	 * but SLP does not reset "next_string_id" value. And we decided to
	 * re-update "rndis_string_defs[0].id" by old value.
	 * 20120224 yongsul96.oh@samsung.com
	 */
	if (rndis_string_defs[0].id == 0)
		rndis_string_defs[0].id = rndis->rndis_string_defs0_id;

	rndis->f_rndis = usb_get_function(rndis->fi_rndis);
	if (IS_ERR(rndis->f_rndis))
		return PTR_ERR(rndis->f_rndis);

	ret = usb_add_function(c, rndis->f_rndis);
	if (ret < 0) {
		usb_put_function(rndis->f_rndis);
		dev_err(f->dev, "rndis_bind_config failed(ret:%d)\n", ret);
	}

	return ret;
}

static void rndis_function_unbind_config(struct slp_multi_usb_function *f,
					 struct usb_configuration *c)
{
	struct rndis_function_config *rndis = f->config;

	usb_put_function(rndis->f_rndis);
}

static ssize_t rndis_manufacturer_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return snprintf(buf, PAGE_SIZE, "%s\n", config->manufacturer);
}

static ssize_t rndis_manufacturer_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	if ((size >= sizeof(config->manufacturer)) ||
		(sscanf(buf, "%s", config->manufacturer) != 1))
		return -EINVAL;

	return size;
}

static DEVICE_ATTR(manufacturer, S_IRUGO | S_IWUSR, rndis_manufacturer_show,
		   rndis_manufacturer_store);

static ssize_t rndis_wceis_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return snprintf(buf, PAGE_SIZE, "%d\n", config->wceis);
}

static ssize_t rndis_wceis_store(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%d", &value) == 1) {
		config->wceis = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(wceis, S_IRUGO | S_IWUSR, rndis_wceis_show,
		   rndis_wceis_store);

static ssize_t rndis_ethaddr_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *rndis = f->config;
	return snprintf(buf, PAGE_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		       rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		       rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);
}

static DEVICE_ATTR(ethaddr, S_IRUGO, rndis_ethaddr_show,
		   NULL);

static ssize_t rndis_vendorID_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return snprintf(buf, PAGE_SIZE, "%04x\n", config->vendorID);
}

static ssize_t rndis_vendorID_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t size)
{
	struct slp_multi_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%04x", &value) == 1) {
		config->vendorID = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(vendorID, S_IRUGO | S_IWUSR, rndis_vendorID_show,
		   rndis_vendorID_store);

static struct device_attribute *rndis_function_attributes[] = {
	&dev_attr_manufacturer,
	&dev_attr_wceis,
	&dev_attr_ethaddr,
	&dev_attr_vendorID,
	NULL
};

static struct slp_multi_usb_function rndis_function = {
	.name = "rndis",
	.init = rndis_function_init,
	.cleanup = rndis_function_cleanup,
	.bind_config = rndis_function_bind_config,
	.unbind_config = rndis_function_unbind_config,
	.attributes = rndis_function_attributes,
};

/*-------------------------------------------------------------------------*/
/* Supported functions initialization */

static struct slp_multi_usb_function *supported_functions[] = {
	&sdb_function,
	&acm_function,
	&mtp_function,
	&rndis_function,
	NULL,
};

static void slp_multi_qos_work(struct work_struct *data)
{
	struct slp_multi_dev *smdev =
		container_of(data, struct slp_multi_dev, qos_work);

	if (smdev->suspended) {
		if (smdev->curr_latency != PM_QOS_DEFAULT_VALUE) {
			smdev->curr_latency = PM_QOS_DEFAULT_VALUE;
			pm_qos_update_request(&smdev->pm_qos_req_dma,
				PM_QOS_DEFAULT_VALUE);
			dev_info(smdev->dev, "usb suspended, set default qos\n");
		}
	} else {
		if ((smdev->swfi_latency != PM_QOS_DEFAULT_VALUE) &&
			!(strncmp(smdev->pm_qos, "high", 4)) &&
			(smdev->curr_latency == PM_QOS_DEFAULT_VALUE)) {
			smdev->curr_latency = smdev->swfi_latency;
			pm_qos_update_request(&smdev->pm_qos_req_dma,
				smdev->swfi_latency);
			dev_info(smdev->dev, "usb resumed, set high qos\n");
		}
	}
}

static int slp_multi_init_functions(struct slp_multi_dev *smdev,
				  struct usb_composite_dev *cdev)
{
	struct slp_multi_usb_function *f;
	struct device_attribute **attrs;
	struct device_attribute *attr;
	int err = 0;
	int index = 0;

	list_for_each_entry(f, &smdev->available_functions, available_list) {
		f->dev_name = kasprintf(GFP_KERNEL, "f_%s", f->name);
		f->dev = device_create(slp_multi_class, smdev->dev,
				       MKDEV(0, index++), f, f->dev_name);
		if (IS_ERR(f->dev)) {
			dev_err(smdev->dev,
				"Failed to create dev %s", f->dev_name);
			err = PTR_ERR(f->dev);
			goto init_func_err_create;
		}

		if (f->init) {
			err = f->init(f, cdev);
			if (err) {
				dev_err(smdev->dev,
					"Failed to init %s", f->name);
				goto init_func_err_out;
			}
		}

		attrs = f->attributes;
		if (attrs) {
			while ((attr = *attrs++) && !err)
				err = device_create_file(f->dev, attr);
		}
		if (err) {
			dev_err(f->dev, "Failed to create function %s attributes",
			       f->name);
			goto init_func_err_out;
		}
	}
	return 0;

 init_func_err_out:
	device_destroy(slp_multi_class, f->dev->devt);
 init_func_err_create:
	kfree(f->dev_name);
	return err;
}

static void slp_multi_cleanup_functions(struct slp_multi_dev *smdev)
{
	struct slp_multi_usb_function *f;

	list_for_each_entry(f, &smdev->available_functions, available_list) {
		if (f->dev) {
			device_destroy(slp_multi_class, f->dev->devt);
			kfree(f->dev_name);
		}

		if (f->cleanup)
			f->cleanup(f);
	}
}

static int
slp_multi_bind_enabled_functions(struct slp_multi_dev *smdev,
			       struct usb_configuration *c)
{
	struct slp_multi_usb_function *f;
	int ret;

	if (c->bConfigurationValue == USB_CONFIGURATION_1) {
		list_for_each_entry(f, &smdev->funcs_fconf, fconf_list) {
			dev_dbg(smdev->dev, "usb_bind_conf(1st) f:%s\n",
				f->name);
			ret = f->bind_config(f, c);
			if (ret) {
				dev_err(smdev->dev, "%s bind_conf(1st) failed\n",
					f->name);
				return ret;
			}
		}
	} else if (c->bConfigurationValue == USB_CONFIGURATION_2) {
		list_for_each_entry(f, &smdev->funcs_sconf, sconf_list) {
			dev_dbg(smdev->dev, "usb_bind_conf(2nd) f:%s\n",
				f->name);
			ret = f->bind_config(f, c);
			if (ret) {
				dev_err(smdev->dev, "%s bind_conf(2nd) failed\n",
					f->name);
				return ret;
			}
		}
	} else {
		dev_err(smdev->dev, "Not supported configuraton(%d)\n",
			c->bConfigurationValue);
		return -EINVAL;
	}
	return 0;
}

static void
slp_multi_unbind_enabled_functions(struct slp_multi_dev *smdev,
				 struct usb_configuration *c)
{
	struct slp_multi_usb_function *f;

	if (c->bConfigurationValue == USB_CONFIGURATION_1) {
		list_for_each_entry(f, &smdev->funcs_fconf, fconf_list) {
			if (f->unbind_config)
				f->unbind_config(f, c);
		}
	} else if (c->bConfigurationValue == USB_CONFIGURATION_2) {
		list_for_each_entry(f, &smdev->funcs_sconf, sconf_list) {
			if (f->unbind_config)
				f->unbind_config(f, c);
		}
	}
}

#define ADD_FUNCS_LIST(head, member)	\
static inline int add_##member(struct slp_multi_dev *smdev, char *name)	\
{	\
	struct slp_multi_usb_function *av_f, *en_f;	\
	\
	dev_dbg(smdev->dev, "usb: name=%s\n", name);	\
	list_for_each_entry(av_f, &smdev->available_functions,	\
			available_list) {	\
		if (!strcmp(name, av_f->name)) {	\
			list_for_each_entry(en_f, &smdev->head,	\
					member) {	\
				if (av_f == en_f) {	\
					dev_info(smdev->dev, \
						"usb:%s already enabled!\n", \
						name);	\
					return 0;	\
				}	\
			}	\
			list_add_tail(&av_f->member, &smdev->head);	\
			return 0;	\
		}	\
	}	\
	return -EINVAL;	\
}	\
static ssize_t	show_##head(struct device *pdev,	\
			struct device_attribute *attr, char *buf)	\
{	\
	struct slp_multi_dev *smdev = dev_get_drvdata(pdev);	\
	struct slp_multi_usb_function *f;	\
	char *buff = buf;	\
	\
	list_for_each_entry(f, &smdev->head, member) {	\
		dev_dbg(pdev, "usb: enabled_func=%s\n",	\
		       f->name);	\
		buff += snprintf(buff, PAGE_SIZE, "%s,", f->name);	\
	}	\
	if (buff != buf)	\
		*(buff - 1) = '\n';	\
	\
	return buff - buf;	\
}	\
static ssize_t store_##head(struct device *pdev,	\
		struct device_attribute *attr,	\
		const char *buff, size_t size)	\
{	\
	struct slp_multi_dev *smdev = dev_get_drvdata(pdev);	\
	char *name;	\
	char buf[256], *b;	\
	int err;	\
	\
	if (smdev->enabled) {	\
		dev_info(pdev, "can't change usb functions"	\
			"(already enabled)!!\n");	\
		return -EBUSY;	\
	}	\
	\
	INIT_LIST_HEAD(&smdev->head);	\
	\
	dev_dbg(pdev, "usb: buff=%s\n", buff);	\
	strlcpy(buf, buff, sizeof(buf));	\
	b = strim(buf);	\
	\
	while (b) {	\
		name = strsep(&b, ",");	\
		if (name) {	\
			err = add_##member(smdev, name);	\
			if (err)	\
				dev_err(pdev, \
					"slp_multi_usb: Cannot enable '%s'", \
					name); \
		}	\
	}	\
	\
	return size;	\
}	\
static DEVICE_ATTR(head, S_IRUGO | S_IWUSR, show_##head, store_##head);

ADD_FUNCS_LIST(funcs_fconf, fconf_list)
ADD_FUNCS_LIST(funcs_sconf, sconf_list)

/*-------------------------------------------------------------------------*/
/* /sys/class/usb_mode/usb%d/ interface */

static ssize_t pm_qos_show(struct device *pdev,
			   struct device_attribute *attr, char *buf)
{
	struct slp_multi_dev *smdev = dev_get_drvdata(pdev);

	return snprintf(buf, PAGE_SIZE, "%s\n", smdev->pm_qos);
}

static ssize_t pm_qos_store(struct device *pdev,
			   struct device_attribute *attr,
			   const char *buff, size_t size)
{
	struct slp_multi_dev *smdev = dev_get_drvdata(pdev);

	if (smdev->enabled) {
		dev_info(pdev, "Already usb enabled, can't change qos\n");
		return -EBUSY;
	}

	strlcpy(smdev->pm_qos, buff, sizeof(smdev->pm_qos));
	return size;
}

static DEVICE_ATTR(pm_qos, S_IRUGO | S_IWUSR, pm_qos_show, pm_qos_store);

static ssize_t enable_show(struct device *pdev,
			   struct device_attribute *attr, char *buf)
{
	struct slp_multi_dev *smdev = dev_get_drvdata(pdev);
	dev_dbg(pdev, "usb: smdev->enabled=%d\n", smdev->enabled);
	return snprintf(buf, PAGE_SIZE, "%d\n", smdev->enabled);
}

static ssize_t enable_store(struct device *pdev, struct device_attribute *attr,
			    const char *buff, size_t size)
{
	struct slp_multi_dev *smdev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = smdev->cdev;
	int enabled;
	int ret = 0;

	if (sysfs_streq(buff, "1"))
		enabled = 1;
	else if (sysfs_streq(buff, "0"))
		enabled = 0;
	else {
		dev_err(pdev, "Invalid cmd %c%c..", *buff, *(buff+1));
		return -EINVAL;
	}

	dev_dbg(pdev, "usb: %s enabled=%d, !smdev->enabled=%d\n",
	       __func__, enabled, !smdev->enabled);

	mutex_lock(&smdev->mutex);

	if (enabled && !smdev->enabled) {
		struct slp_multi_usb_function *f;

		/* update values in composite driver's
		 * copy of device descriptor
		 */
		cdev->desc.idVendor = device_desc.idVendor;
		cdev->desc.idProduct = device_desc.idProduct;
		cdev->desc.bcdDevice = device_desc.bcdDevice;

		list_for_each_entry(f, &smdev->funcs_fconf, fconf_list) {
			if (!strcmp(f->name, "acm"))
				cdev->desc.bcdDevice =
					cpu_to_le16(0x0400);
		}

		list_for_each_entry(f, &smdev->funcs_sconf, sconf_list) {
			if (!strcmp(f->name, "acm"))
				cdev->desc.bcdDevice =
					cpu_to_le16(0x0400);
			smdev->dual_config = true;
		}

		cdev->desc.bDeviceClass = device_desc.bDeviceClass;
		cdev->desc.bDeviceSubClass = device_desc.bDeviceSubClass;
		cdev->desc.bDeviceProtocol = device_desc.bDeviceProtocol;

		dev_dbg(pdev, "usb: %s vendor=%x,product=%x,bcdDevice=%x",
		       __func__, cdev->desc.idVendor,
		       cdev->desc.idProduct, cdev->desc.bcdDevice);
		dev_dbg(pdev, ",Class=%x,SubClass=%x,Protocol=%x\n",
		       cdev->desc.bDeviceClass,
		       cdev->desc.bDeviceSubClass, cdev->desc.bDeviceProtocol);
		dev_dbg(pdev, "usb: %s next cmd : usb_add_config\n",
		       __func__);

		ret = usb_add_config(cdev,
				&first_config_driver, slp_multi_bind_config);
		if (ret < 0) {
			dev_err(pdev,
				"usb_add_config fail-1st(%d)\n", ret);
			smdev->dual_config = false;
			goto done;
		}

		if (smdev->dual_config) {
			ret = usb_add_config(cdev, &second_config_driver,
				       slp_multi_bind_config);
			if (ret < 0) {
				dev_err(pdev,
					"usb_add_config fail-2nd(%d)\n", ret);
				usb_remove_config(cdev, &first_config_driver);
				smdev->dual_config = false;
				goto done;
			}
		}

		smdev->enabled = true;
		usb_gadget_connect(cdev->gadget);


	} else if (!enabled && smdev->enabled) {

		usb_gadget_disconnect(cdev->gadget);
		/* Cancel pending control requests */
		usb_ep_dequeue(cdev->gadget->ep0, cdev->req);
		usb_remove_config(cdev, &first_config_driver);
		if (smdev->dual_config)
			usb_remove_config(cdev, &second_config_driver);
		smdev->enabled = false;
		smdev->dual_config = false;
	} else {
		dev_info(pdev, "slp_multi_usb: already %s\n",
		       smdev->enabled ? "enabled" : "disabled");
	}

done:
	mutex_unlock(&smdev->mutex);
	return (ret < 0 ? ret : size);
}

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);

#define DESCRIPTOR_ATTR(field, format_string)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return snprintf(buf, PAGE_SIZE, format_string, device_desc.field);\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)	\
{									\
	int value;	\
	if (sscanf(buf, format_string, &value) == 1) {			\
		device_desc.field = value;				\
		return size;						\
	}								\
	return -EINVAL;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_STRING_ATTR(field, buffer)	\
static ssize_t	\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)	\
{	\
	return snprintf(buf, PAGE_SIZE, "%s", buffer);	\
}	\
static ssize_t	\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)	\
{	\
	if ((size >= sizeof(buffer)) ||	\
		(sscanf(buf, "%s", buffer) != 1)) {	\
		return -EINVAL;	\
	}	\
	return size;	\
}	\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

DESCRIPTOR_ATTR(idVendor, "%04x\n")
DESCRIPTOR_ATTR(idProduct, "%04x\n")
DESCRIPTOR_ATTR(bcdDevice, "%04x\n")
DESCRIPTOR_ATTR(bDeviceClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceSubClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceProtocol, "%d\n")
DESCRIPTOR_STRING_ATTR(iManufacturer, manufacturer_string)
DESCRIPTOR_STRING_ATTR(iProduct, product_string)
DESCRIPTOR_STRING_ATTR(iSerial, serial_string)

static struct device_attribute *slp_multi_usb_attributes[] = {
	&dev_attr_idVendor,
	&dev_attr_idProduct,
	&dev_attr_bcdDevice,
	&dev_attr_bDeviceClass,
	&dev_attr_bDeviceSubClass,
	&dev_attr_bDeviceProtocol,
	&dev_attr_iManufacturer,
	&dev_attr_iProduct,
	&dev_attr_iSerial,
	&dev_attr_funcs_fconf,
	&dev_attr_funcs_sconf,
	&dev_attr_enable,
	&dev_attr_pm_qos,
	NULL
};

/*-------------------------------------------------------------------------*/
/* Composite driver */

static int slp_multi_bind_config(struct usb_configuration *c)
{
	struct slp_multi_dev *smdev = _slp_multi_dev;
	int ret = 0;

	ret = slp_multi_bind_enabled_functions(smdev, c);
	if (ret)
		return ret;

	if ((smdev->swfi_latency != PM_QOS_DEFAULT_VALUE) &&
		!(strncmp(smdev->pm_qos, "high", 4)) &&
			(smdev->curr_latency == PM_QOS_DEFAULT_VALUE)) {
		smdev->curr_latency = smdev->swfi_latency;
		pm_qos_update_request(&smdev->pm_qos_req_dma,
			smdev->swfi_latency);
	}

	return 0;
}

static void slp_multi_unbind_config(struct usb_configuration *c)
{
	struct slp_multi_dev *smdev = _slp_multi_dev;

	cancel_work_sync(&smdev->qos_work);

	if (smdev->curr_latency != PM_QOS_DEFAULT_VALUE) {
		smdev->curr_latency = PM_QOS_DEFAULT_VALUE;
		pm_qos_update_request(&smdev->pm_qos_req_dma,
			PM_QOS_DEFAULT_VALUE);
	}

	slp_multi_unbind_enabled_functions(smdev, c);
}

static int slp_multi_bind(struct usb_composite_dev *cdev)
{
	struct slp_multi_dev *smdev = _slp_multi_dev;
	struct usb_gadget *gadget = cdev->gadget;
	int id, ret;

	dev_dbg(smdev->dev, "usb: %s disconnect\n", __func__);
	usb_gadget_disconnect(gadget);

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	/* Default strings - should be updated by userspace */
	strlcpy(manufacturer_string, "Samsung\0",
		sizeof(manufacturer_string) - 1);
	strlcpy(product_string, "SLP\0", sizeof(product_string) - 1);
	/* snprintf(serial_string, sizeof(serial_string), */
	/* 	 "%08x%08x", system_serial_high, system_serial_low); */

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	ret = slp_multi_init_functions(smdev, cdev);
	if (ret)
		return ret;

	usb_gadget_set_selfpowered(gadget);
	smdev->cdev = cdev;

	return 0;
}

static int slp_multi_usb_unbind(struct usb_composite_dev *cdev)
{
	struct slp_multi_dev *smdev = _slp_multi_dev;
	dev_dbg(smdev->dev, "usb: %s\n", __func__);
	slp_multi_cleanup_functions(smdev);
	return 0;
}

static void slp_multi_usb_resume(struct usb_composite_dev *cdev)
{
	struct slp_multi_dev *smdev = _slp_multi_dev;

	dev_dbg(smdev->dev, "usb: %s\n", __func__);

	smdev->suspended = 0;

	if ((smdev->swfi_latency != PM_QOS_DEFAULT_VALUE) &&
		!(strncmp(smdev->pm_qos, "high", 4)))
		schedule_work(&smdev->qos_work);
}

static void slp_multi_usb_suspend(struct usb_composite_dev *cdev)
{
	struct slp_multi_dev *smdev = _slp_multi_dev;

	dev_dbg(smdev->dev, "usb: %s\n", __func__);

	smdev->suspended = 1;

	if (smdev->curr_latency != PM_QOS_DEFAULT_VALUE)
		schedule_work(&smdev->qos_work);
}

static struct usb_composite_driver slp_multi_composite = {
	.name = "slp_multi_composite",
	.dev = &device_desc,
	.strings = slp_dev_strings,
	.bind = slp_multi_bind,
	.unbind = slp_multi_usb_unbind,
	.max_speed = USB_SPEED_HIGH,
	.resume = slp_multi_usb_resume,
	.suspend = slp_multi_usb_suspend,
};

/* HACK: android needs to override setup for accessory to work */
static int (*composite_setup_func)(struct usb_gadget *gadget,
				   const struct usb_ctrlrequest *c);

static int
slp_multi_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
	struct slp_multi_dev *smdev = _slp_multi_dev;
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	u8 b_requestType = ctrl->bRequestType;
	struct slp_multi_usb_function *f;
	int value = -EOPNOTSUPP;

	if ((b_requestType & USB_TYPE_MASK) == USB_TYPE_VENDOR) {
		struct usb_request *req = cdev->req;

		req->zero = 0;
		req->length = 0;
		gadget->ep0->driver_data = cdev;

		/* To check & report it to platform , we check it all */
		list_for_each_entry(f, &smdev->available_functions,
			available_list) {
			if (f->ctrlrequest) {
				value = f->ctrlrequest(f, cdev, ctrl);
				if (value >= 0)
					break;
			}
		}
	}

	if (value < 0)
		value = composite_setup_func(gadget, ctrl);

	return value;
}

static int slp_multi_create_device(struct slp_multi_dev *smdev)
{
	struct device_attribute **attrs = slp_multi_usb_attributes;
	struct device_attribute *attr;
	int err;

	smdev->dev = device_create(slp_multi_class, NULL,
				  MKDEV(0, 0), NULL, "usb0");
	if (IS_ERR(smdev->dev))
		return PTR_ERR(smdev->dev);

	dev_set_drvdata(smdev->dev, smdev);

	while ((attr = *attrs++)) {
		err = device_create_file(smdev->dev, attr);
		if (err) {
			device_destroy(slp_multi_class, smdev->dev->devt);
			return err;
		}
	}
	return 0;
}

static void slp_multi_destroy_device(struct slp_multi_dev *smdev)
{
	struct device_attribute **attrs = slp_multi_usb_attributes;
	struct device_attribute *attr;

	while ((attr = *attrs++))
		device_remove_file(smdev->dev, attr);

	dev_set_drvdata(smdev->dev, NULL);

	device_destroy(slp_multi_class, smdev->dev->devt);
}

static CLASS_ATTR_STRING(version, S_IRUSR | S_IRGRP | S_IROTH,
			 USB_MODE_VERSION);

static int __init slp_multi_init(void)
{
	int err, i;
	struct slp_multi_dev *smdev;
	struct slp_multi_usb_function *f;
	struct slp_multi_usb_function **functions = supported_functions;

	slp_multi_class = class_create(THIS_MODULE, "usb_mode");
	if (IS_ERR(slp_multi_class)) {
		pr_err("failed to create slp_multi class --> %ld\n",
				PTR_ERR(slp_multi_class));
		return PTR_ERR(slp_multi_class);
	}

	err = class_create_file(slp_multi_class, &class_attr_version.attr);
	if (err) {
		pr_err("usb_mode: can't create sysfs version file\n");
		goto err_class;
	}

	smdev = kzalloc(sizeof(*smdev), GFP_KERNEL);
	if (!smdev) {
		pr_err("usb_mode: can't alloc for smdev\n");
		err = -ENOMEM;
		goto err_attr;
	}

	INIT_LIST_HEAD(&smdev->available_functions);
	INIT_LIST_HEAD(&smdev->funcs_fconf);
	INIT_LIST_HEAD(&smdev->funcs_sconf);
	INIT_WORK(&smdev->qos_work, slp_multi_qos_work);

	mutex_init(&smdev->mutex);

	while ((f = *functions++)) {
		for (i = 0; i < sizeof(default_funcs) / sizeof(char *); i++)
			if (!strcmp(default_funcs[i], f->name))
				list_add_tail(&f->available_list,
					      &smdev->available_functions);
	}

	err = slp_multi_create_device(smdev);
	if (err) {
		pr_err("usb_mode: can't create device\n");
		goto err_alloc;
	}

	slp_multi_nluns = 1;
	_slp_multi_dev = smdev;

	err = usb_composite_probe(&slp_multi_composite);
	if (err) {
		pr_err("usb_mode: can't probe composite\n");
		goto err_create;
	}

	/* HACK: exchange composite's setup with ours */
	composite_setup_func = slp_multi_composite.gadget_driver.setup;
	slp_multi_composite.gadget_driver.setup = slp_multi_setup;

	smdev->swfi_latency = PM_QOS_DEFAULT_VALUE;
	strlcpy(smdev->pm_qos, "NONE", sizeof(smdev->pm_qos));
	smdev->curr_latency = PM_QOS_DEFAULT_VALUE;

	pr_info("usb_mode driver, version:" USB_MODE_VERSION
		"," " init Ok\n");

	return 0;

err_create:
	slp_multi_destroy_device(smdev);

err_alloc:
	kfree(smdev);

err_attr:
	class_remove_file(slp_multi_class, &class_attr_version.attr);
err_class:
	class_destroy(slp_multi_class);

	return err;
}
late_initcall(slp_multi_init);

static void __exit slp_multi_exit(void)
{
	usb_composite_unregister(&slp_multi_composite);
	slp_multi_destroy_device(_slp_multi_dev);

	kfree(_slp_multi_dev);
	_slp_multi_dev = NULL;

	class_remove_file(slp_multi_class, &class_attr_version.attr);
	class_destroy(slp_multi_class);
}
module_exit(slp_multi_exit);
