#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/wlan_plat.h>
#include <asm/uaccess.h>

#include "board.h"
static struct wifi_platform_data bcmdhd_wlan_pdata;
#define IMPORT_WIFIMAC_BY_SELF
//#define IMPORT_FWPATH_BY_SELF

#ifdef IMPORT_WIFIMAC_BY_SELF
#define WIFIMAC_ADDR_PATH "/firmware/wifimac.txt"

static int get_wifi_mac_addr(unsigned char* buf)
{
	struct file *fp = NULL;
	mm_segment_t fs;

	unsigned char source_addr[18];
	loff_t pos = 0;
	unsigned char *head, *end;
	int i = 0;

	fp = filp_open(WIFIMAC_ADDR_PATH, O_RDONLY,  0444);
	if (IS_ERR(fp)) {

		printk("Can not access wifi mac file : %s\n",WIFIMAC_ADDR_PATH);
		return -EFAULT;
	}else{
		fs = get_fs();
		set_fs(KERNEL_DS);

		vfs_read(fp, source_addr, 18, &pos);
		source_addr[17] = ':';

		head = end = source_addr;
		for(i=0; i<6; i++) {
			while (end && (*end != ':') )
				end++;

			if (end && (*end == ':') )
				*end = '\0';

			buf[i] = simple_strtoul(head, NULL, 16 );

			if (end) {
				end++;
				head = end;
			}
			printk("wifi mac %02x \n", buf[i]);
		}
		set_fs(fs);
		filp_close(fp, NULL);
	}

	return 0;
}
#endif

#ifdef IMPORT_FWPATH_BY_SELF
#define WIFI_DRIVER_FW_PATH_PARAM "/data/misc/wifi/fwpath"
#define CLJ_DEBUG 1
static unsigned char fwpath[128];

static char* get_firmware_path(void)
{
	struct file *fp = NULL;
	mm_segment_t fs;
	loff_t pos = 0;

	printk("enter  :%s\n",__FUNCTION__);
	fp = filp_open(WIFI_DRIVER_FW_PATH_PARAM, O_RDONLY,  0444);
	if (IS_ERR(fp)) {

		printk("Can not access wifi firmware path file : %s\n",WIFI_DRIVER_FW_PATH_PARAM);
		return NULL;
	}else{
		fs = get_fs();
		set_fs(KERNEL_DS);

		memset(fwpath , 0 ,sizeof(fwpath));

		vfs_read(fp, fwpath, sizeof(fwpath), &pos);

		if(CLJ_DEBUG)
			printk("fetch fwpath = %s\n",fwpath);

		set_fs(fs);
		filp_close(fp, NULL);
		return fwpath;
	}

	return NULL;
}

#endif



static struct resource wlan_resources[] = {
	[0] = {
		.start = WL_WAKE_HOST,
		.end = WL_WAKE_HOST,
		.name = "bcmdhd_wlan_irq",
		.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE,
	},
};
static struct platform_device wlan_device = {
	.name   = "bcmdhd_wlan",
	.id     = 1,
	.dev    = {
		.platform_data = &bcmdhd_wlan_pdata,
	},
	.resource	= wlan_resources,
	.num_resources	= ARRAY_SIZE(wlan_resources),
};

static int __init wlan_device_init(void)
{
	int ret;

	memset(&bcmdhd_wlan_pdata,0,sizeof(bcmdhd_wlan_pdata));

#ifdef IMPORT_WIFIMAC_BY_SELF
	bcmdhd_wlan_pdata.get_mac_addr = get_wifi_mac_addr;
#endif
#ifdef IMPORT_FWPATH_BY_SELF
	bcmdhd_wlan_pdata.get_fwpath = get_firmware_path;
#endif

	ret = platform_device_register(&wlan_device);

	return ret;
}

late_initcall(wlan_device_init);
//MODULE_DESCRIPTION("Broadcomm wlan driver");
//MODULE_LICENSE("GPL");
