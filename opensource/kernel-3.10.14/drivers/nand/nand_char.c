#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>

#include "nm_interface.h"
#include "nand_char.h"

#define DBG_FUNC() //printk("##### nand char debug #####: func = %s \n", __func__)

#define NEED_MARK	0x1
#define NO_MARK		0x2

enum nand_char_ops_status {
	NAND_DRIVER_FREE,
	NAND_DRIVER_BUSY,
};

static struct __nand_char {
	dev_t ndev;
	struct cdev nd_cdev;
	struct class *nd_cclass;
	unsigned short status;
	/* data */
	int nm_handler;
	NM_lpt *lptl;
	NM_ppt *pptl;
} nand_char;

extern unsigned int get_nandflash_maxvalidblocks(void);
extern unsigned int get_nandflash_pagesize(void);
static int update_error_pt(NM_ppt *ppt)
{
	return NM_UpdateErrorPartition(ppt);
}

static inline int erase_block(NM_ppt *ppt, int blockid, int markflag)
{
	int ret;

	ret = NM_DirectErase(ppt, blockid);
	if (ret || (markflag == NEED_MARK)) {
		ret = NM_DirectMarkBadBlock(ppt, blockid);
		if (ret) {
			printk("%s: line:%d, nand mark badblock error, pt = %s, blockID = %d, ret = %d\n",
				   __func__, __LINE__, ppt->pt->name, blockid, ret);
			dump_stack();
		}
		return 1;
	}

	return 0;
}

static int erase_ppt(NM_ppt *ppt)
{
	int ret, blockid, markcnt = 0;

	for (blockid = 0; blockid < ppt->pt->totalblocks; blockid++) {
		if (NM_DirectIsBadBlock(ppt, blockid))
			ret = erase_block(ppt, blockid, NEED_MARK);
		else
			ret = erase_block(ppt, blockid, NO_MARK);

		if (ret) {
			printk("mark bad block (%s, %d)\n", ppt->pt->name, blockid);
			markcnt++;
		}
	}

	return markcnt;
}

static long nand_char_unlocked_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct singlelist *plist = NULL;

	DBG_FUNC();
	switch(cmd){
	case CMD_PREPARE_NEW_NAND: {
		ret = NM_PrepareNewFlash(nand_char.nm_handler);
		if (ret) {
			printk("ERROR: prepare new nand error!\n");
		}

		ret = update_error_pt(NULL);
		if (ret) {
			printk("%s: line:%d, update error partition error, ret = %d\n",
				   __func__, __LINE__, ret);
		}
		break;
	}
	case CMD_CHECK_USED_NAND: {
		ret = NM_CheckUsedFlash(nand_char.nm_handler);
		if (ret) {
			printk("ERROR: check used nand error!\n");
		}

		ret = update_error_pt(NULL);
		if (ret) {
			printk("%s: line:%d, update error partition error, ret = %d\n",
				   __func__, __LINE__, ret);
		}
		break;
	}
	case CMD_SOFT_PARTITION_ERASE: {
		char ptname[128];
		int context;
		NM_lpt *lpt = NULL;

		/* yet not support soft erase */
		ret = 0;
		break;

		ret = copy_from_user(&ptname, (unsigned char *)arg, 128);
		if (!ret) {
			singlelist_for_each(plist, &nand_char.lptl->list) {
				lpt = singlelist_entry(plist, NM_lpt, list);
				printk("match partition: ppt = [%s], ptname = [%s]\n", lpt->pt->name, ptname);
				if (!strcmp(lpt->pt->name, ptname))
					break;
			}

			if (!lpt || strcmp(lpt->pt->name, ptname)) {
				printk("ERROR: can't find partition [%s]\n", ptname);
				ret = -1;
				break;
			}

			printk("soft erase nand partition [%s]\n", lpt->pt->name);
			if ((context = NM_ptOpen(nand_char.nm_handler, lpt->pt->name, lpt->pt->mode)) == 0) {
				printk("can not open NM %s, mode = %d\n", lpt->pt->name, lpt->pt->mode);
				return -1;
			}

			ret = NM_ptErase(context);

			NM_ptClose(context);
		}
		break;
	}
	case CMD_SOFT_ERASE_ALL: {
		int context;
		NM_lpt *lpt = NULL;

		/* yet not support soft erase */
		ret = 0;
		break;

		singlelist_for_each(plist, &nand_char.lptl->list) {
			lpt = singlelist_entry(plist, NM_lpt, list);

			if ((context = NM_ptOpen(nand_char.nm_handler, lpt->pt->name, lpt->pt->mode)) == 0) {
				printk("can not open NM %s, mode = %d\n", lpt->pt->name, lpt->pt->mode);
				return -1;
			}

			printk("soft erase nand partition [%s]\n", lpt->pt->name);
			ret = NM_ptErase(context);

			NM_ptClose(context);
		}
		break;
	}
	case CMD_HARD_PARTITION_ERASE: {
		char ptname[128];
		int markcnt;
		NM_ppt *ppt = NULL;

		ret = copy_from_user(&ptname, (unsigned char *)arg, 128);
		if (!ret) {
			singlelist_for_each(plist, &nand_char.pptl->list) {
				ppt = singlelist_entry(plist, NM_ppt, list);
				printk("match partition: ppt = [%s], ptname = [%s]\n", ppt->pt->name, ptname);
				if (!strcmp(ppt->pt->name, ptname))
					break;
			}

			if (!ppt || strcmp(ppt->pt->name, ptname)) {
				printk("ERROR: can't find partition [%s]\n", ptname);
				ret = -1;
				break;
			}

			printk("hard erase nand partition [%s]\n", ppt->pt->name);
			markcnt = erase_ppt(ppt);
			printk("mark [%d] badblocks in partition [%s]\n", markcnt, ppt->pt->name);

			ret = update_error_pt(ppt);
			if (ret)
				printk("%s: line:%d, update error partition error, ret = %d\n",
					   __func__, __LINE__, ret);
		}
		break;
	}
	case CMD_HARD_ERASE_ALL: {
		int force = NANDMANAGER_ERASE_FLASH;
		ret = NM_EraseFlash(nand_char.nm_handler, force);
		if (ret)
			printk("%s: line:%d, erase all nand flash error, ret = %d\n",
				   __func__, __LINE__, ret);

		ret = update_error_pt(NULL);
		if (ret)
			printk("%s: line:%d, update error partition error, ret = %d\n",
				   __func__, __LINE__, ret);
		break;
	}
	case CMD_GET_NANDFLASH_MAXVALIDBLCOKS: {
		ret = get_nandflash_maxvalidblocks();
		return ret;
	}
	case CMD_GET_NAND_PAGESIZE:{
		unsigned int pagesize;
		pagesize=get_nandflash_pagesize();
		copy_to_user((unsigned int *)arg,&pagesize,sizeof(unsigned int));
		break;
	}
	case CMD_SET_SPL_SIZE: {
		int spl_size;
		int context;
		NM_lpt *lpt = NULL;

		ret = copy_from_user(&spl_size, (unsigned char *)arg, sizeof(int));
		if(!ret){
			lpt = singlelist_entry(&nand_char.lptl->list, NM_lpt, list);
			if ((context = NM_ptOpen(nand_char.nm_handler, lpt->pt->name, lpt->pt->mode)) == 0) {
				printk("can not open NM %s, mode = %d\n", lpt->pt->name, lpt->pt->mode);
				return -1;
			}
			NM_ptIoctrl(context, NANDMANAGER_SET_XBOOT_OFFSET, spl_size);

			NM_ptClose(context);
		}else
			printk("nand_char_driver: copy_from_user error!\n");
		break;
	}
	default:
		printk("nand_char_driver: the parameter is wrong!\n");
		ret = -1;
		break;
	}

	return ret ? -EFAULT : 0 ;
}

static ssize_t nand_char_write(struct file * fd, const char __user * pdata, size_t size, loff_t * offset)
{
	int copysize, ret;
	char argbuf[128];
	char *cmd_install = "CMD_INSTALL_PARTITION";
	char *cmdline, *pt;

	DBG_FUNC();

	if (size > 128)
		copysize = 128;
	else
		copysize = size;

	if(copy_from_user(argbuf, pdata, copysize)) {
		printk("%s, line:%d, copy from user error, copysize = %d\n", __func__, __LINE__, copysize);
		return -EFAULT;
	}
	argbuf[copysize] = '\0';

	cmdline = strstrip(argbuf);
	printk("nand_char: cmdline = [%s]\n", cmdline);

	if (!strncmp(cmdline, cmd_install, strlen(cmd_install))) {
		/*cmdline: [CMD_INSTALL_PARTITION:ALL/pt_name]*/
		pt = cmdline + strlen(cmd_install) + 1;
		pt = strstrip(pt);

		if (!pt) {
			printk("%s, line:%d, has no partition to install!\n", __func__, __LINE__);
			return -EFAULT;
		}

		printk("pt = [%s]\n", pt);
		if (!strncmp(pt, "ALL", 3)) {
			/* install all partitions */
			if ((ret = NM_ptInstall(nand_char.nm_handler, NULL))) {
				printk("%s, line:%d, install all partitions error!\n", __func__, __LINE__);
				return ret;
			}
		} else {
			/* install the partition indicated */
			if ((ret = NM_ptInstall(nand_char.nm_handler, pt))) {
				printk("%s, line:%d, install partitions [%s] error!\n", __func__, __LINE__, pt);
				return ret;
			}
		}
	} else {
		printk("%s, cmd error, cmd [%s], need [%s:ALL/pt_name]\n", __func__, cmdline, cmd_install);
		return -EFAULT;
	}

	return size;
}

static int nand_char_open(struct inode *pnode,struct file *fd)
{
	DBG_FUNC();
	if(nand_char.status == NAND_DRIVER_FREE){
		nand_char.status = NAND_DRIVER_BUSY;
		return 0;
	}
	return -EBUSY;
}

static int nand_char_close(struct inode *pnode,struct file *fd)
{
	DBG_FUNC();
	nand_char.status = NAND_DRIVER_FREE;
	return 0;
}

static const struct file_operations nand_char_ops = {
	.owner	= THIS_MODULE,
	.open   = nand_char_open,
	.release = nand_char_close,
	.write   = nand_char_write,
	.unlocked_ioctl	= nand_char_unlocked_ioctl,
};

void nand_char_start(int data)
{
	int ret;

	DBG_FUNC();

	nand_char.lptl = NM_getPartition(nand_char.nm_handler);
	if (!nand_char.lptl) {
		printk("ERROR: %s(%d), can't get lpartiton list!\n", __func__, __LINE__);
		return;
	}

	nand_char.pptl = NM_getPPartition(nand_char.nm_handler);
	if (!nand_char.pptl) {
		printk("ERROR: %s(%d), can't get ppartiton list!\n", __func__, __LINE__);
		return;
	}

	ret = alloc_chrdev_region(&nand_char.ndev, 0, 1, "nand_char");
	if(ret < 0) {
		printk("ERROR: %s(%d), alloc_chrdev_region faild\n", __func__, __LINE__);
		return;
	}

	cdev_init(&nand_char.nd_cdev, &nand_char_ops);
	ret = cdev_add(&nand_char.nd_cdev, nand_char.ndev, 1);
	if (ret < 0) {
		printk("ERROR: %s(%d): cdev_add faild\n", __func__, __LINE__);
		unregister_chrdev_region(nand_char.ndev, 1);
		return;
	}

	device_create(nand_char.nd_cclass, NULL, nand_char.ndev, NULL, "nand_char");

	printk("nand char register ok!!!\n");
}

static int __init nand_char_init(void)
{
	DBG_FUNC();

	nand_char.nd_cclass = class_create(THIS_MODULE, "nd_cclass");
	if (!nand_char.nd_cclass) {
		printk("ERROR: %s(%d), can't create nand char class!\n", __func__, __LINE__);
		return -1;
	}

	if (((nand_char.nm_handler = NM_open())) == 0) {
		printk("ERROR: %s(%d), NM_open error!\n", __func__, __LINE__);
		return -1;
	}

	nand_char.status = NAND_DRIVER_FREE;
	nand_char.pptl = NULL;

	NM_startNotify(nand_char.nm_handler, nand_char_start, 0);

	return 0;
}

static void __exit nand_char_exit(void)
{
	DBG_FUNC();
	cdev_del(&nand_char.nd_cdev);
	unregister_chrdev_region(nand_char.ndev, 1);
}
#ifdef CONFIG_EARLY_INIT_RUN
rootfs_initcall(nand_char_init);
#else
module_init(nand_char_init);
#endif
module_exit(nand_char_exit);
MODULE_LICENSE("GPL v2");
