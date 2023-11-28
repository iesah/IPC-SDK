
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include "nm_interface.h"
#include "nand_debug.h"

#define DBG_FUNC() //printk("##### nand char debug #####: func = %s \n", __func__)

enum nand_debug_ops_status{
	NAND_DRIVER_FREE,
	NAND_DRIVER_BUSY,
};

static struct __nand_char {
	dev_t ndev;
	struct cdev nd_ddev;
	struct class *nd_dclass;
	unsigned short status;
	/* data */
	int nm_handler;
	NM_ppt *pptl;
	struct NandInfo nand_info;
} nand_dbg;

static NM_ppt* get_ppt_by_partnum(int partnum)
{
	int index = 0;
	NM_ppt *ppt = NULL;
	struct singlelist *plist = NULL;

	singlelist_for_each(plist, &nand_dbg.pptl->list) {
		ppt = singlelist_entry(plist, NM_ppt, list);
		index ++;
		if (index == partnum) break;
	}

	if (index != partnum) {
		printk("ERROR: %s, can't find partition, partnum = [%d]\n", __func__, partnum);
		return NULL;
	}

	return ppt;
}

static long nand_debug_unlocked_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
	int ret = 0, i = 0;
	int ptcount = 0;
	NM_ppt *ppt;
	struct nand_dug_msg *dug_msg = NULL;
	struct NandInfo *nand_info = &(nand_dbg.nand_info);
	static unsigned char *databuf = NULL;
	struct singlelist *plist = NULL;

	DBG_FUNC();

	singlelist_for_each(plist, &nand_dbg.pptl->list)
		ptcount++;

	switch(cmd){
	case CMD_GET_NAND_PTC:
		put_user(ptcount,(int *)arg);
		break;
	case CMD_GET_NAND_MSG:
		if(ptcount > 0){
			dug_msg = kmalloc(ptcount * sizeof(struct nand_dug_msg),GFP_KERNEL);
			if(dug_msg){
				singlelist_for_each(plist, &nand_dbg.pptl->list) {
					ppt = singlelist_entry(plist, NM_ppt, list);
					strcpy(dug_msg[i].name, ppt->pt->name);
					dug_msg[i].byteperpage = ppt->pt->byteperpage;
					dug_msg[i].pageperblock = ppt->pt->pageperblock;
					dug_msg[i++].totalblocks = ppt->pt->totalblocks;
				}
				ret = copy_to_user((unsigned char *)arg ,(unsigned char *)dug_msg,
								   ptcount * sizeof(struct nand_dug_msg));
				kfree(dug_msg);
			}
		}
		break;
	case CMD_NAND_DUG_READ:
		ret = copy_from_user(nand_info, (unsigned char *)arg, sizeof(struct NandInfo));
		if(!ret) {
			databuf =(unsigned char *)kmalloc(nand_info->bytes, GFP_KERNEL);
			if(!databuf) {
				ret = -1;
				break;
			}

			ppt = get_ppt_by_partnum(nand_info->partnum);
			if (!ppt) {
				ret = -1;
				break;
			}

			ret = NM_DirectRead(ppt, nand_info->id,	0, nand_info->bytes, databuf);
			if (ret == nand_info->bytes) {
				copy_to_user(nand_info->data, databuf, nand_info->bytes);
				ret = 0;
			} else
				ret = -1;

			kfree(databuf);
		}
		break;
	case CMD_NAND_DUG_WRITE:
		ret = copy_from_user(nand_info, (unsigned char *)arg, sizeof(struct NandInfo));
		if(!ret){
			databuf =(unsigned char *)kmalloc(nand_info->bytes, GFP_KERNEL);
			if(!databuf) {
				ret = -1;
				break;
			}
			copy_from_user(databuf, nand_info->data, nand_info->bytes);

			ppt = get_ppt_by_partnum(nand_info->partnum);
			if (!ppt) {
				ret = -1;
				break;
			}

			ret = NM_DirectWrite(ppt, nand_info->id, 0, nand_info->bytes, databuf);
			if(ret == nand_info->bytes)
				ret = 0;
			else
				ret = -1;

			kfree(databuf);
		}
		break;
	case CMD_NAND_DUG_ERASE:
		ret = copy_from_user(nand_info, (unsigned char *)arg, sizeof(struct NandInfo));
		if(!ret){
			ppt = get_ppt_by_partnum(nand_info->partnum);
			if (!ppt) {
				ret = -1;
				break;
			}
			ret = NM_DirectErase(ppt, nand_info->id);
		}
		break;
	default:
		printk("nand_dug_driver: the parameter is wrong!\n");
		ret = -1;
		break;
	}

	return ret != 0 ? -EFAULT : 0 ;
}

static ssize_t nand_debug_write(struct file * fd, const char __user * pdata, size_t size, loff_t * pt)
{
	return size;
}

static int nand_debug_open(struct inode *pnode,struct file *fd)
{
	DBG_FUNC();
	if(nand_dbg.status == NAND_DRIVER_FREE){
		nand_dbg.status = NAND_DRIVER_BUSY;
		return 0;
	}
	return -EBUSY;
}

static int nand_debug_close(struct inode *pnode,struct file *fd)
{
	DBG_FUNC();
	nand_dbg.status = NAND_DRIVER_FREE;
	return 0;
}

static const struct file_operations nand_debug_ops = {
	.owner	= THIS_MODULE,
	.open   = nand_debug_open,
	.release = nand_debug_close,
	.write   = nand_debug_write,
	.unlocked_ioctl	= nand_debug_unlocked_ioctl,
};

void nand_debug_start(int data)
{
	int ret;

	DBG_FUNC();

	nand_dbg.pptl = NM_getPPartition(nand_dbg.nm_handler);
	if (!nand_dbg.pptl) {
		printk("ERROR: %s(%d), can't get ppartiton list!\n", __func__, __LINE__);
		return;
	}

	ret = alloc_chrdev_region(&nand_dbg.ndev, 0, 1, "nand_debug");
	if(ret < 0){
		printk("ERROR: %s(%d), alloc_chrdev_region faild\n", __func__, __LINE__);
		return;
	}

	cdev_init(&nand_dbg.nd_ddev, &nand_debug_ops);
	ret = cdev_add(&nand_dbg.nd_ddev, nand_dbg.ndev, 1);
	if(ret < 0){
		printk("ERROR: %s(%d): cdev_add faild\n", __func__, __LINE__);
		unregister_chrdev_region(nand_dbg.ndev, 1);
		return;
	}

	device_create(nand_dbg.nd_dclass, NULL, nand_dbg.ndev, NULL, "nand_debug");

	printk("nand debug register ok!!!\n");
}

static int __init nand_debug_init(void)
{
	DBG_FUNC();

	nand_dbg.nd_dclass = class_create(THIS_MODULE, "nd_dclass");
	if(!nand_dbg.nd_dclass){
		printk("ERROR: %s(%d), can't create nand debug class!\n", __func__, __LINE__);
		return -1;
	}

	if (((nand_dbg.nm_handler = NM_open())) == 0) {
		printk("ERROR: %s(%d), NM_open error!\n", __func__, __LINE__);
		return -1;
	}

	nand_dbg.status = NAND_DRIVER_FREE;
	nand_dbg.pptl = NULL;

	NM_startNotify(nand_dbg.nm_handler, nand_debug_start, 0);

	return 0;
}

static void __exit nand_debug_exit(void)
{
	DBG_FUNC();
	cdev_del(&nand_dbg.nd_ddev);
	unregister_chrdev_region(nand_dbg.ndev, 1);
}

#ifdef CONFIG_EARLY_INIT_RUN
rootfs_initcall(nand_debug_init);
#else
module_init(nand_debug_init);
#endif
module_exit(nand_debug_exit);
MODULE_DESCRIPTION("JZ4780 debug Nand driver");
MODULE_LICENSE("GPL v2");
