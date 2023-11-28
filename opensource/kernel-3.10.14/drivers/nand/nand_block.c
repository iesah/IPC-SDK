/**
 * nand_block.c
 **/
#include <linux/major.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/blk_types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/suspend.h>
#include <linux/vmalloc.h>

#include "nm_interface.h"
#include "nand_block.h"
/* debug switchs */
//#define DEBUG_TIME_WRITE
//#define DEBUG_TIME_READ
//#define DEBUG_FUNC
//#define DEBUG_SYS
//#define DEBUG_REQ
//#define DEBUG_STLIST
//#define DEBUG_REREAD_DATA

#define BLOCK_NAME 		"nand_block"
#define MAX_MINORS		255
#define DISK_MINORS		MUL_PARTS
struct __partition_info {
	int context;
	NM_lpt *lpt;
};

struct __nand_disk {
	struct singlelist list;
	struct gendisk *disk;
	struct request_queue *queue;
	struct request *req;
	struct task_struct *q_thread;
	struct semaphore thread_sem;
	struct device *dev;
	struct device_attribute dattr;
	struct __partition_info *pinfo;
	SectorList *sl;
	unsigned int sl_len;
	unsigned int sectorsize;
	unsigned int segmentsize;
	unsigned long capacity;
	spinlock_t queue_lock;
};

struct __nand_block {
	char *name;
	int major;
	int nm_handler;
	struct notifier_block pm_notify;
	struct singlelist disk_list;
};

static struct __nand_block nand_block;
/*for example: div_s64_32(3,2) = 2*/
static inline int div_s64_32(long long dividend , int divisor)
{
	long long result=0;
	int remainder=0;
	result =div_s64_rem(dividend,divisor,&remainder);
	result = remainder ? (result +1):result;
	return result;
}
/*#################################################################*\
 *# dump
\*#################################################################*/
#ifdef DEBUG_FUNC
#define DBG_FUNC() printk("##### nand block debug #####: func = %s \n", __func__)
#else
#define DBG_FUNC()
#endif

#if defined(DEBUG_TIME_WRITE) || defined(DEBUG_TIME_READ)
#define DEBUG_TIME_BYTES (10 * 1024 *1024) //10MB, max is 40MB
struct __data_distrib {
	unsigned int _1_4sectors;
	unsigned int _5_8sectors;
	unsigned int _9_16sectors;
	unsigned int _17_32sectors;
	unsigned int _33_64sectors;
	unsigned int _65_128sectors;
	unsigned int _129_256sectors;
	unsigned int _257_512sectors;
};

static unsigned long long rd_btime = 0;
static unsigned long long wr_btime = 0;
static unsigned long long rd_sum_time = 0;
static unsigned long long wr_sum_time = 0;
static unsigned int rd_sum_bytes = 0;
static unsigned int wr_sum_bytes = 0;
static struct __data_distrib rd_dbg_distrib = {0};
static struct __data_distrib wr_dbg_distrib = {0};


static inline void calc_bytes(int mode, int bytes)
{
	if (mode == READ)
		rd_sum_bytes += bytes;
	else
		wr_sum_bytes += bytes;
}

static inline void calc_distrib(int mode, int sectors)
{
	struct __data_distrib *distrib;

	if (mode == READ)
		distrib = &rd_dbg_distrib;
	else
		distrib = &wr_dbg_distrib;

	if ((sectors > 0) && (sectors <= 4)) {
		distrib->_1_4sectors ++;
	} else if (sectors <= 8) {
		distrib->_5_8sectors ++;
	} else if (sectors <= 16) {
		distrib->_9_16sectors ++;
	} else if (sectors <= 32) {
		distrib->_17_32sectors ++;
	} else if (sectors <= 64) {
		distrib->_33_64sectors ++;
	} else if (sectors <= 128) {
		distrib->_65_128sectors ++;
	} else if (sectors <= 256) {
		distrib->_129_256sectors ++;
	} else if (sectors <= 512) {
		distrib->_257_512sectors ++;
	} else {
		printk("%s, line:%d, SectorCount error!, count = %d\n", __func__, __LINE__, sectors);
	}
}

static inline void begin_time(int mode)
{
	if (mode == READ)
		rd_btime = sched_clock();
	else
		wr_btime = sched_clock();
}

static inline void end_time(int mode)
{
	int times, bytes, KBps;
	unsigned long long etime = sched_clock();

	if (mode == READ) {
		rd_sum_time += (etime - rd_btime);
		if (rd_sum_bytes >= DEBUG_TIME_BYTES) {
#ifdef DEBUG_TIME_READ
			times = div_s64_32(rd_sum_time, 1000 * 1000);
			bytes = rd_sum_bytes / 1024;
			KBps = (bytes * 1000) / times;
			printk("READ: nand_block speed debug, %dms, %dKb, %d.%d%dMB/s\n",
			       times, bytes,  KBps / 1024, (KBps % 1024) * 10 / 1024,
			       (((KBps % 1024) * 10) % 1024) * 10 / 1024);
			printk("[  1 -   4]:(%d)\n[  5 -   8]:(%d)\n[  9 -  16]:(%d)\n[ 17 -  32]:(%d)\n",
			       rd_dbg_distrib._1_4sectors, rd_dbg_distrib._5_8sectors,
			       rd_dbg_distrib._9_16sectors, rd_dbg_distrib._17_32sectors);
			printk("[ 33 -  64]:(%d)\n[ 65 - 128]:(%d)\n[129 - 256]:(%d)\n[257 - 512]:(%d)\n",
			       rd_dbg_distrib._33_64sectors, rd_dbg_distrib._65_128sectors,
			       rd_dbg_distrib._129_256sectors, rd_dbg_distrib._257_512sectors);
#endif

			rd_sum_bytes = rd_sum_time = 0;
			memset(&rd_dbg_distrib, 0, sizeof(struct __data_distrib));
		}
	} else {
		wr_sum_time += (etime - wr_btime);
		if (wr_sum_bytes >= DEBUG_TIME_BYTES) {
#ifdef DEBUG_TIME_WRITE
			times = div_s64_32(wr_sum_time, 1000 * 1000);
			bytes = wr_sum_bytes / 1024;
			KBps = (bytes * 1000) / times;
			printk("WRITE: nand_block speed debug, %dms, %dKb, %d.%d%dMB/s\n",
			       times, bytes, KBps / 1024, (KBps % 1024) * 10 / 1024,
			       (((KBps % 1024) * 10) % 1024) * 10 / 1024);
			printk("[  1 -   4]:(%d)\n[  5 -   8]:(%d)\n[  9 -  16]:(%d)\n[ 17 -  32]:(%d)\n",
			       wr_dbg_distrib._1_4sectors, wr_dbg_distrib._5_8sectors,
			       wr_dbg_distrib._9_16sectors, wr_dbg_distrib._17_32sectors);
			printk("[ 33 -  64]:(%d)\n[ 65 - 128]:(%d)\n[129 - 256]:(%d)\n[257 - 512]:(%d)\n",
			       wr_dbg_distrib._33_64sectors, wr_dbg_distrib._65_128sectors,
			       wr_dbg_distrib._129_256sectors, wr_dbg_distrib._257_512sectors);
#endif
			wr_sum_bytes = wr_sum_time = 0;
			memset(&wr_dbg_distrib, 0, sizeof(struct __data_distrib));
		}
	}
}
#else
static inline void calc_bytes(int mode, int bytes) {};
static inline void calc_distrib(int mode, int sectors) {};
static inline void begin_time(int mode) {};
static inline void end_time(int mode) {};
#endif

#ifdef DEBUG_SYS
struct driver_attribute drv_attr;
static ssize_t dbg_show(struct device_driver *driver, char *buf)
{
	printk("%s", __func__);
	return 0;
}

static ssize_t dbg_store(struct device_driver *driver, const char *buf, size_t count)
{
	printk("%s", __func__);
	return count;
}
#endif

#ifdef DEBUG_STLIST
static inline void dump_sectorlist(SectorList *top)
{
	struct singlelist *plist = NULL;
	SectorList *sl = NULL;

	if (top) {
		singlelist_for_each(plist, &top->head) {
			sl = singlelist_entry(plist, SectorList, head);
			printk("\ndump SectorList: startSector = %d, sectorCount = %d, pData = %p\n\n",
			       sl->startSector, sl->sectorCount, sl->pData);
		}
	}
}
#endif

#ifdef DEBUG_REREAD_DATA
unsigned char *origin_buf = NULL;
spinlock_t print_lock;
unsigned int writebytes, readbytes;
static int spinlock_inited = 0;
static void dump_reread_data_prepare(struct __nand_disk *ndisk)
{
	unsigned int offset, bytes;
	struct singlelist *pos = NULL;
	SectorList *sl_node, *sl = ndisk->sl;

	if (origin_buf == NULL)
		origin_buf = kmalloc(256 * 1024, GFP_KERNEL);

	if (origin_buf) {
		offset = 0;
		singlelist_for_each(pos, &sl->head) {
			sl_node = singlelist_entry(pos, SectorList, head);
			bytes = sl_node->sectorCount * ndisk->sectorsize;
			memcpy(origin_buf + offset, sl_node->pData, bytes);
			offset += bytes;
		}
		writebytes = offset;
	}
}
static void dump_reread_data_complete(struct __nand_disk *ndisk)
{
	int i, ret;
	unsigned long flags;
	unsigned char v1, v2;
	unsigned int offset, bytes;
	struct singlelist *pos = NULL;
	SectorList *sl_node, *sl = ndisk->sl;

	if (spinlock_inited == 0) {
		spin_lock_init(&print_lock);
		spinlock_inited = 1;
	}

	if (origin_buf) {
		ret = NM_ptRead(ndisk->pinfo->context, ndisk->sl);
		if (ret < 0) {
			printk("ERROR:[%s] dump reread return error! ret = %d\n",
			       ndisk->pinfo->lpt->pt->name, ret);
			while(1);
		}

		offset = 0;
		singlelist_for_each(pos, &sl->head) {
			sl_node = singlelist_entry(pos, SectorList, head);
			bytes = sl_node->sectorCount * ndisk->sectorsize;
			ret = memcmp(origin_buf + offset, sl_node->pData, bytes);
			if (ret) {
				spin_lock_irqsave(&print_lock, flags);
				printk("ERROR:[%s] dump reread data error!\n",
				       ndisk->pinfo->lpt->pt->name);
				printk("============= orign write data:");
				for (i = 0; i < bytes; i++) {
					v1 = *((unsigned char *)origin_buf + offset + i);
					if (!(i % 16))
						printk("\n%04d: ", i / 16);
					printk(" %02x", v1);
				}
				printk("\n============= data of reread:");
				for (i = 0; i < bytes; i++) {
					v2 = *((unsigned char *)sl_node->pData + i);
					if (!(i % 16))
						printk("\n%04d: ", i / 16);
					printk(" %02x", v2);
				}
				printk("\n");
				spin_unlock_irqrestore(&print_lock, flags);
				while(1);
			}
			offset += bytes;
		}
		readbytes = offset;
		if (readbytes != writebytes) {
			printk("ERROR:[%s] write sectorlist has been changed!\n",
			       ndisk->pinfo->lpt->pt->name);
			while(1);
		}
	}
}
#else
static void dump_reread_data_prepare(struct __nand_disk *ndisk) {}
static void dump_reread_data_complete(struct __nand_disk *ndisk) {}
#endif
/*#################################################################*\
 *# request
 \*#################################################################*/

static struct __nand_disk * get_ndisk_form_queue(const struct request_queue *q)
{
	struct singlelist *plist = NULL;
	struct __nand_disk *ndisk = NULL;

	singlelist_for_each(plist, nand_block.disk_list.next) {
		ndisk = singlelist_entry(plist, struct __nand_disk, list);
		if (ndisk->queue == q)
			return ndisk;
	}
	return NULL;
}

/**
 * @name: must point to the string
 *  which ndisk->pinfo->pt->name pointed,
 **/
static struct __nand_disk * get_ndisk_by_name(const char *name)
{
	struct singlelist *plist = NULL;
	struct __nand_disk *ndisk = NULL;

	singlelist_for_each(plist, nand_block.disk_list.next) {
		ndisk = singlelist_entry(plist, struct __nand_disk, list);
		/* here, wo don't use strcmp, so here the to pointer must
		   point to the same string in memory */
		if (ndisk->pinfo->lpt->pt->name == name)
			return ndisk;
	}
	return NULL;
}

/**
 * map a request to SectorList,
 * return number of sl entries setup.
 * this function refer to blk_rq_map_sg()
 * in blk-merge.c
 **/
static int nand_rq_map_sl(struct request_queue *q,
			  struct request *req,
			  SectorList *sl_array,
			  int sectorsize)
{
	struct bio_vec *bvec, *bvprv = NULL;
	struct req_iterator iter;
	unsigned int cluster;
	unsigned int startSector;
	unsigned int index = -1;

	startSector = blk_rq_pos(req);
	cluster = blk_queue_cluster(q);
	rq_for_each_segment(bvec, req, iter) {
		int nbytes = bvec->bv_len;
		if (bvprv && cluster) {
			if ((sl_array[index].sectorCount * sectorsize + nbytes) > queue_max_segment_size(q))
				goto new_segment;
			if (!BIOVEC_PHYS_MERGEABLE(bvprv, bvec))
				goto new_segment;
			if (!BIOVEC_SEG_BOUNDARY(q, bvprv, bvec))
				goto new_segment;

			sl_array[index].sectorCount += nbytes / sectorsize;
			startSector += nbytes / sectorsize;
		} else {
		new_segment:
			index ++;
			if (index)
				sl_array[index - 1].head.next = &sl_array[index].head;

			sl_array[index].head.next = NULL;
			sl_array[index].startSector = startSector;
			sl_array[index].sectorCount = nbytes / sectorsize;
			sl_array[index].pData = page_address(bvec->bv_page) + bvec->bv_offset;
			startSector += sl_array[index].sectorCount;
		}
		bvprv = bvec;
	}

	return index + 1;
}

/* thread to handle request */
static int handle_req_thread(void *data)
{
	int err = 0;
	int ret = 0;
	struct request *req = NULL;
	struct __nand_disk *ndisk = NULL;
	struct request_queue *q = (struct request_queue *)data;

	ndisk = get_ndisk_form_queue(q);
	if (!ndisk || !ndisk->pinfo) {
		printk("can not get ndisk, ndisk = %p\n", ndisk);
		return -ENODEV;
	}

	down(&ndisk->thread_sem);
	while(1) {
		/* set thread state */
		spin_lock_irq(q->queue_lock);
		set_current_state(TASK_INTERRUPTIBLE);
		req = blk_fetch_request(q);
		ndisk->req = req;
		spin_unlock_irq(q->queue_lock);

		if (!req) {
			if (kthread_should_stop()) {
				set_current_state(TASK_RUNNING);
				break;
			}
			up(&ndisk->thread_sem);
			schedule();
			down(&ndisk->thread_sem);
			continue;
		}

		set_current_state(TASK_RUNNING);

		while(req) {
			err = 0;
#ifdef DEBUG_REQ
			printk("%s: req = %p, start sector = %d, total = %d, buffer = %p\n",
			       (rq_data_dir(req) == READ)? "READ":"WRITE",
			       req, (int)blk_rq_pos(req), (int)blk_rq_sectors(req), req->buffer);
#endif
			if (rq_data_dir(req) == READ) {
				calc_bytes(READ, (int)blk_rq_sectors(req) * ndisk->sectorsize);
				calc_distrib(READ, (int)blk_rq_sectors(req));
			} else {
				calc_bytes(WRITE, (int)blk_rq_sectors(req) * ndisk->sectorsize);
				calc_distrib(WRITE, (int)blk_rq_sectors(req));
			}

			/* make SectorList from request */
			spin_lock_irq(q->queue_lock);
			ndisk->sl_len = nand_rq_map_sl(q, req, ndisk->sl, ndisk->sectorsize);
			spin_unlock_irq(q->queue_lock);

			if ((ndisk->sl_len <= 0)) {
				printk("nand_rq_map_sl error, ndisk->sl_len = %d\n", ndisk->sl_len);
				err = -EIO;
			}
#ifdef DEBUG_STLIST
			dump_sectorlist(ndisk->sl);
#endif
			/* nandmanager read || write */
			if (rq_data_dir(req) == READ) {
				begin_time(READ);
				ret = NM_ptRead(ndisk->pinfo->context, ndisk->sl);
				end_time(READ);
			} else {
				dump_reread_data_prepare(ndisk);
				begin_time(WRITE);
				ret = NM_ptWrite(ndisk->pinfo->context, ndisk->sl);
				end_time(WRITE);
				dump_reread_data_complete(ndisk);
			}

			if (ret < 0) {
				printk("nand_block: NM %s error!\n", (rq_data_dir(req) == READ)? "read" : "write");
				err = -EIO;
			}

			spin_lock_irq(q->queue_lock);
			ret = __blk_end_request(req, err, blk_rq_bytes(req));
			spin_unlock_irq(q->queue_lock);
			if (!ret)
				break;
		}
	}
	up(&ndisk->thread_sem);

	return 0;
}

static ssize_t nand_attr_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	ssize_t count;
	count =  sprintf(buf, "%u:%u\n", MAJOR(dev->devt), MINOR(dev->devt));
	return count;
}

static ssize_t nand_attr_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
}
static void do_nand_request(struct request_queue *q)
{
	struct __nand_disk *ndisk = NULL;

	ndisk = get_ndisk_form_queue(q);
	if (!ndisk || !ndisk->pinfo) {
		printk("can not get ndisk, ndisk = %p\n", ndisk);
		return;
	}

	/* wake up the handle thread to process the request */
	if (!ndisk->req)
		wake_up_process(ndisk->q_thread);
}

/*#################################################################*\
 *# block_device_operations
 \*#################################################################*/
static int nand_disk_open(struct block_device *bdev, fmode_t mode)
{
	DBG_FUNC();

	return 0;
}

static int nand_disk_release(struct gendisk *disk, fmode_t mode)
{
	DBG_FUNC();

	return 0;
}

static int nand_disk_ioctl(struct block_device *bdev, fmode_t mode, unsigned cmd, unsigned long arg)
{
	DBG_FUNC();

	return 0;
}

static int nand_disk_compat_ioctl(struct block_device *bdev, fmode_t mode, unsigned cmd, unsigned long arg)
{
	DBG_FUNC();

	return 0;
}

static int nand_disk_direct_access(struct block_device *bdev, sector_t sector, void **kaddr, unsigned long *pfn)
{
	DBG_FUNC();

	return 0;
}

static int nand_disk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->cylinders = get_capacity(bdev->bd_disk) / (4 * 16);
	geo->heads = 4;
	geo->sectors = 16;

	return 0;
}

static const struct block_device_operations nand_disk_fops =
{
	.owner					= THIS_MODULE,
	.open					= nand_disk_open,
	.release				= nand_disk_release,
	.ioctl					= nand_disk_ioctl,
	.compat_ioctl 			= nand_disk_compat_ioctl,
	.direct_access 			= nand_disk_direct_access,
	.getgeo					= nand_disk_getgeo,
};

/*#################################################################*\
 *# pm_notify
 \*#################################################################*/
#ifdef CONFIG_PM
static int nand_pm_notify(struct notifier_block *notify_block, unsigned long mode, void *data)
{
	unsigned long flags;
	struct request_queue *q = NULL;
	struct singlelist *plist = NULL;
	struct __nand_disk *ndisk = NULL;

	switch (mode) {
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		singlelist_for_each(plist, nand_block.disk_list.next) {
			ndisk = singlelist_entry(plist, struct __nand_disk, list);
			q = ndisk->queue;
			if (q) {
				spin_lock_irqsave(q->queue_lock, flags);
				blk_stop_queue(q);
				spin_unlock_irqrestore(q->queue_lock, flags);
				down(&ndisk->thread_sem);
				if (NM_ptIoctrl(ndisk->pinfo->context, SUSPEND, 0)) {
					printk("ERROR: %s, suspend NandManager error!\n", __func__);
					return -1;
				}
			}
		}
		break;

	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		singlelist_for_each(plist, nand_block.disk_list.next) {
			ndisk = singlelist_entry(plist, struct __nand_disk, list);
			q = ndisk->queue;
			if (q) {
				if (NM_ptIoctrl(ndisk->pinfo->context, RESUME, 0)) {
					printk("ERROR: %s, resume NandManager error!\n", __func__);
					return -1;
				}
				up(&ndisk->thread_sem);
				spin_lock_irqsave(q->queue_lock, flags);
				blk_start_queue(q);
				spin_unlock_irqrestore(q->queue_lock, flags);
			}
		}
		break;

	default:
		break;
	}

	return 0;
}
#endif

/*#################################################################*\
 *# bus
 \*#################################################################*/

static void nbb_shutdown(struct device *dev)
{
	struct device_driver *driver = dev->driver;

	DBG_FUNC();

	if (driver->shutdown)
		driver->shutdown(dev);
}

static int nbb_suspend(struct device *dev, pm_message_t state)
{
	struct device_driver *driver = dev->driver;

	DBG_FUNC();

	if (driver->suspend)
		return driver->suspend(dev, state);

	return 0;
}

static int nbb_resume(struct device *dev)
{
	struct device_driver *driver = dev->driver;

	DBG_FUNC();

	if (driver->resume)
		return driver->resume(dev);

	return 0;
}

static struct bus_type nand_block_bus = {
	.name		= "nbb",
	.shutdown	= nbb_shutdown,
	.suspend	= nbb_suspend,
	.resume		= nbb_resume,
};

/*#################################################################*\
 *# device_driver
 \*#################################################################*/
static struct __nand_disk * get_ndisk_by_dev(const struct device *dev)
{
	struct singlelist *plist = NULL;
	struct __nand_disk *ndisk = NULL;

	DBG_FUNC();

	singlelist_for_each(plist, nand_block.disk_list.next) {
		ndisk = singlelist_entry(plist, struct __nand_disk, list);
		if (ndisk->dev == dev)
			return ndisk;
	}

	return NULL;
}

struct hd_struct *add_partition_for_disk(struct gendisk *disk, int partno,
					 sector_t start, sector_t len,
					 int flags, struct partition_meta_info *info,
					 char *name)
{
	struct hd_struct *p;
	dev_t devt = MKDEV(0, 0);
	struct device *ddev = disk_to_dev(disk);
	struct device *pdev;
	struct disk_part_tbl *ptbl;
	const char *dname;
	int err;

	err = disk_expand_part_tbl(disk, partno);
	if (err)
		return ERR_PTR(err);

	ptbl = disk->part_tbl;

	if (ptbl->part[partno])
		return ERR_PTR(-EBUSY);

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return ERR_PTR(-EBUSY);

	if (!init_part_stats(p)) {
		err = -ENOMEM;
		goto out_free;
	}
	pdev = part_to_dev(p);

	p->start_sect = start;
	p->alignment_offset =
		queue_limit_alignment_offset(&disk->queue->limits, start);
	p->discard_alignment =
		queue_limit_discard_alignment(&disk->queue->limits, start);
	p->nr_sects = len;
	p->partno = partno;
	p->policy = get_disk_ro(disk);

	if (info) {
		struct partition_meta_info *pinfo = alloc_part_info(disk);
		if (!pinfo)
			goto out_free_stats;
		memcpy(pinfo, info, sizeof(*info));
		p->info = pinfo;
	}

	dname = dev_name(ddev);
	if(name)
		dev_set_name(pdev, "%s", name);
	else
		dev_set_name(pdev, "%s%d", dname, partno);

	device_initialize(pdev);
	pdev->class = &block_class;
	pdev->type = &part_type;
	pdev->parent = ddev;

	err = blk_alloc_devt(p, &devt);
	if (err)
		goto out_free_info;
	pdev->devt = devt;

	/* delay uevent until 'holders' subdir is created */
	dev_set_uevent_suppress(pdev, 1);
	err = device_add(pdev);
	if (err)
		goto out_put;

	err = -ENOMEM;
	p->holder_dir = kobject_create_and_add("holders", &pdev->kobj);
	if (!p->holder_dir)
		goto out_del;

	dev_set_uevent_suppress(pdev, 0);

	/* everything is up and running, commence */
	rcu_assign_pointer(ptbl->part[partno], p);

	/* suppress uevent if the disk suppresses it */
	if (!dev_get_uevent_suppress(ddev))
		kobject_uevent(&pdev->kobj, KOBJ_ADD);

	hd_ref_init(p);
	return p;

out_free_info:
	free_part_info(p);
out_free_stats:
	free_part_stats(p);
out_free:
	kfree(p);
	return ERR_PTR(err);
out_del:
	kobject_put(p->holder_dir);
	device_del(pdev);
out_put:
	put_device(pdev);
	blk_free_devt(devt);
	return ERR_PTR(err);
}

static int nand_block_probe(struct device *dev)
{
	int i, ret = -ENOMEM;
	static unsigned int cur_minor = 0;
	struct __nand_disk *ndisk = NULL;
	struct __partition_info *pinfo = (struct __partition_info *)dev->platform_data;
	NM_lpt *lpt = pinfo ? pinfo->lpt : NULL;
	struct hd_struct *hd;
	DBG_FUNC();

	if (!pinfo || !lpt) {
		printk("ERROR(nand block): can not get partition info!\n");
		return -EFAULT;
	}

	if (cur_minor > MAX_MINORS - DISK_MINORS) {
		printk("ERROR(nand block): no enough minors left, can't create disk!\n");
		return -EFAULT;
	}

	ndisk = vzalloc(sizeof(struct __nand_disk));
	if (!ndisk) {
		printk("ERROR(nand block): alloc memory for ndisk error!\n");
		goto probe_err0;
	}

	if(lpt->pt->parts_num > 0)
		ndisk->disk = alloc_disk(DISK_MINORS);
	else
		ndisk->disk = alloc_disk(1);

	if (!ndisk->disk) {
		printk("ERROR(nand block): alloc_disk error!\n");
		goto probe_err1;
	}

	/* init queue */
	spin_lock_init(&ndisk->queue_lock);
	ndisk->queue = blk_init_queue(do_nand_request, &ndisk->queue_lock);
	if (!ndisk->queue) {
		printk("ERROR(nand block): blk_init_queue error!\n");
		goto probe_err2;
	}

	/* operate partition of SPL/DIRECT_MANAGER need to keeps write data in order */
	if (pinfo->lpt->pt->mode != ZONE_MANAGER)
		elevator_change(ndisk->queue, "noop");

	/* init ndisk */
	ndisk->sl_len = 0;
	ndisk->dev = dev;
	ndisk->pinfo = pinfo;
	ndisk->sectorsize = lpt->pt->hwsector;
	ndisk->segmentsize = lpt->pt->segmentsize;
	ndisk->capacity = lpt->pt->sectorCount;

	printk("INFO(nand block): pt[%s] capacity is [%d]MB\n", lpt->pt->name,
	       div_s64_32(((unsigned long long)ndisk->capacity * ndisk->sectorsize), (1024 * 1024)));

	ndisk->disk->major = nand_block.major;
	ndisk->disk->first_minor = cur_minor;
	ndisk->disk->minors = DISK_MINORS;
	ndisk->disk->fops = &nand_disk_fops;
	sprintf(ndisk->disk->disk_name, "%s", lpt->pt->name);
	ndisk->disk->queue = ndisk->queue;

	cur_minor += DISK_MINORS;

	/* alloc memory for sectorlist */
	ndisk->sl = kmalloc((ndisk->segmentsize / ndisk->sectorsize) * sizeof(SectorList), GFP_KERNEL);
	if (!ndisk->sl) {
		printk("ERROR(nand block): can not alloc memory for SectorList!\n");
		goto probe_err3;
	}

	/* add gendisk */
	add_disk(ndisk->disk);

	/* add parts of gendisk */
	for(i = 0; i < lpt->pt->parts_num; i++){
		lmul_parts *part = lpt->pt->lparts + i;
		hd = add_partition_for_disk(ndisk->disk, (i + 1),
					    part->startSector, part->sectorCount,
					    ADDPART_FLAG_NONE, ndisk->disk->part0.info,
					    part->name);
		if (!hd)
			printk("ERROR: add_partition [%s] fail!\n", part->name);
	}

	/* add ndisk to disk_list */
	singlelist_add(&nand_block.disk_list, &ndisk->list);

	/* init semaphore */
	sema_init(&ndisk->thread_sem, 1);

	/* create and start a thread to handle request */
	ndisk->q_thread = kthread_run(handle_req_thread,
				      (void *)ndisk->queue,
				      "%s_q_handler",
				      lpt->pt->name);
	if (!ndisk->q_thread) {
		printk("ERROR(nand block): kthread_run error!\n");
		goto probe_err4;
	}

	/* set queue limit & capacity */
	if (ndisk->segmentsize > 0) {
		blk_queue_max_segments(ndisk->queue, ndisk->capacity / (ndisk->segmentsize / ndisk->sectorsize));
		blk_queue_max_segment_size(ndisk->queue, ndisk->segmentsize);
		blk_queue_max_hw_sectors(ndisk->queue, ndisk->segmentsize / ndisk->sectorsize);
	}
	set_capacity(ndisk->disk, ndisk->capacity);

	/* create file */
	sysfs_attr_init(&ndisk->dattr.attr);
	ndisk->dattr.attr.name = "info";
	ndisk->dattr.attr.mode = S_IRUGO | S_IWUSR;
	ndisk->dattr.show = nand_attr_show;
	ndisk->dattr.store = nand_attr_store;
	ret = device_create_file(ndisk->dev, &ndisk->dattr);
	if (ret)
		printk("WARNING(nand block): device_create_file error!\n");

	return 0;

probe_err4:
	del_gendisk(ndisk->disk);
	singlelist_del(&nand_block.disk_list, &ndisk->list);
	kfree(ndisk->sl);
probe_err3:
	blk_cleanup_queue(ndisk->queue);
probe_err2:
	put_disk(ndisk->disk);
probe_err1:
	vfree(ndisk);
probe_err0:
	return ret;
}

static int nand_block_remove(struct device *dev)
{
	struct __nand_disk *ndisk = get_ndisk_by_dev(dev);

	DBG_FUNC();

	if (ndisk) {
		kfree(ndisk->sl);
		del_gendisk(ndisk->disk);
		blk_cleanup_queue(ndisk->queue);
		put_disk(ndisk->disk);
		vfree(dev);
	}

	return 0;
}

static void nand_block_shutdown(struct device *dev)
{
	unsigned long flags;
	struct __nand_disk *ndisk = get_ndisk_by_dev(dev);
	struct request_queue *q = ndisk ? ndisk->queue : NULL;

	DBG_FUNC();

	if (ndisk && q) {
		spin_lock_irqsave(q->queue_lock, flags);
		blk_stop_queue(q);
		spin_unlock_irqrestore(q->queue_lock, flags);
		down(&ndisk->thread_sem);
		if (NM_ptIoctrl(ndisk->pinfo->context, SUSPEND, 0)) {
			printk("ERROR: %s, suspend NandManager error!\n", __func__);
		}
	}
}

#ifndef CONFIG_PM
static int nand_block_suspend(struct device *dev, pm_message_t state)
{
	unsigned long flags;
	struct __nand_disk *ndisk = get_ndisk_by_dev(dev);
	struct request_queue *q = ndisk ? ndisk->queue : NULL;

	DBG_FUNC();

	if (ndisk && q) {
		spin_lock_irqsave(q->queue_lock, flags);
		blk_stop_queue(q);
		spin_unlock_irqrestore(q->queue_lock, flags);
		down(&ndisk->thread_sem);
		if (NM_ptIoctrl(ndisk->pinfo->context, SUSPEND, 0)) {
			printk("ERROR: %s, suspend NandManager error!\n", __func__);
			return -1;
		}
	}

	return 0;
}

static int nand_block_resume(struct device *dev)
{
	unsigned long flags;
	struct __nand_disk *ndisk = get_ndisk_by_dev(dev);
	struct request_queue *q = ndisk ? ndisk->queue : NULL;

	DBG_FUNC();

	if (ndisk && q) {
		if (NM_ptIoctrl(ndisk->pinfo->context, RESUME, 0)) {
			printk("ERROR: %s, resume NandManager error!\n", __func__);
			return -1;
		}
		up(&ndisk->thread_sem);
		spin_lock_irqsave(q->queue_lock, flags);
		blk_start_queue(q);
		spin_unlock_irqrestore(q->queue_lock, flags);
	}

	return 0;
}
#endif

struct device_driver nand_block_driver = {
	.name 		= "nand_block",
	.bus		= &nand_block_bus,
	.probe 		= nand_block_probe,
	.remove 	= nand_block_remove,
	.shutdown 	= nand_block_shutdown,
#ifndef CONFIG_PM
	.suspend 	= nand_block_suspend,
	.resume 	= nand_block_resume,
#endif
};

/*#################################################################*\
 *# start
 \*#################################################################*/
static int nand_disk_install(char *name)
{
	int ret = -EFAULT;
	int installAll = 1;
	int context = 0;
	NM_lpt *lptl, *lpt;
	struct device *dev = NULL;
	struct singlelist *plist = NULL;
	struct __partition_info *pinfo = NULL;
	int install_flag=0;

	DBG_FUNC();

	if (name)
		installAll = 0;

	if (!(lptl = NM_getPartition(nand_block.nm_handler))) {
		printk("ERROR: %s(%d), can not get partition list!\n", __func__, __LINE__);
		return -1;
	}

	singlelist_for_each(plist, &lptl->list) {
		lpt = singlelist_entry(plist, NM_lpt, list);

		if (!installAll && strcmp(lpt->pt->name, name))
			continue;

		if (get_ndisk_by_name(lpt->pt->name)) {
			printk("WARNING(nand block): disk [%s] has been installed!\n", lpt->pt->name);
			if(!installAll)
				break;
			else
				continue;
		}

		install_flag=1;
		printk("nand block, install partition [%s]!\n", lpt->pt->name);

		if ((context = NM_ptOpen(nand_block.nm_handler, lpt->pt->name, lpt->pt->mode)) == 0) {
			printk("can not open NM %s, mode = %d\n", lpt->pt->name, lpt->pt->mode);
			return -1;
		}

		printk("nand block, install partition [%s]%d!\n", lpt->pt->name,__LINE__);
		/* init dev */
		dev = vzalloc(sizeof(struct device));
		if (!dev) {
			printk("can not alloc memory for device!\n");
			goto start_err0;
		}

		pinfo = vzalloc(sizeof(struct __partition_info));
		if (!pinfo) {
			printk("can not alloc memory for pinfo!\n");
			goto start_err1;
		}

		pinfo->context = context;
		pinfo->lpt = lpt;

		device_initialize(dev);
		dev->bus = &nand_block_bus;
		dev->platform_data = pinfo;
		dev_set_name(dev, "%s", lpt->pt->name);

		/* register dev */
		ret = device_add(dev);
		if (ret < 0) {
			printk("device_add error!\n");
			ret = -EFAULT;
			goto start_err2;
		}
		if(!installAll)
			break;
	}
	if(install_flag==0){
		printk("%s can't install partitions %s\n",__func__,name);
		return -1;
	}
	return 0;

start_err2:
	vfree(pinfo);
start_err1:
	vfree(dev);
start_err0:
	NM_close(context);
	return -1;
}

static void nand_block_start(int data)
{
	NM_regPtInstallFn(nand_block.nm_handler, (int)nand_disk_install);
}

/*#################################################################*\
 *# init and deinit
 \*#################################################################*/
static int __init nand_block_init(void)
{
	int ret = -EBUSY;

	DBG_FUNC();

	nand_block.name = BLOCK_NAME;
	nand_block.disk_list.next = NULL;

	nand_block.major = register_blkdev(0, nand_block.name);
	if (nand_block.major <= 0) {
		printk("ERROR(nand block): register_blkdev error!\n");
		goto out_block;
	}

	if (bus_register(&nand_block_bus)) {
		printk("ERROR(nand block): bus_register error!\n");
		goto out_bus;
	}

	if (driver_register(&nand_block_driver)) {
		printk("ERROR(nand block): driver_register error!\n");
		goto out_driver;
	}

#ifdef DEBUG_SYS
	drv_attr.show = dbg_show;
	drv_attr.store = dbg_store;
	sysfs_attr_init(&drv_attr.attr);
	drv_attr.attr.name = "dbg";
	drv_attr.attr.mode = S_IRUGO | S_IWUSR;
	ret = driver_create_file(&nand_block_driver, &drv_attr);
	if (ret)
		printk("WARNING(nand block): driver_create_file error!\n");
#endif
#ifdef CONFIG_PM
	nand_block.pm_notify.notifier_call = nand_pm_notify;
	ret = register_pm_notifier(&nand_block.pm_notify);
	if (ret) {
		printk("ERROR(nand block): register_pm_notifier error!\n");
		goto out_pm;
	}
#endif
	if (((nand_block.nm_handler = NM_open())) == 0) {
		printk("ERROR(nand block): NM_open error!\n");
		goto out_open;
	}
	NM_startNotify(nand_block.nm_handler, nand_block_start, 0);
	return 0;

out_open:
#ifdef CONFIG_PM
	if (unregister_pm_notifier(&nand_block.pm_notify))
		printk("ERROR(nand block): unregister_pm_notifier error!\n");
out_pm:
#endif
	driver_unregister(&nand_block_driver);
out_driver:
	bus_unregister(&nand_block_bus);
out_bus:
	unregister_blkdev(nand_block.major, nand_block.name);
out_block:
	return ret;
}

static void __exit nand_block_exit(void)
{
	DBG_FUNC();

	NM_close(nand_block.nm_handler);
	driver_unregister(&nand_block_driver);
	bus_unregister(&nand_block_bus);
	unregister_blkdev(nand_block.major, nand_block.name);
}
#ifdef CONFIG_EARLY_INIT_RUN
rootfs_initcall(nand_block_init);
#else
module_init(nand_block_init);
#endif
module_exit(nand_block_exit);
MODULE_LICENSE("GPL");
