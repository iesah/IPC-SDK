#include "nandmanagerinterface.h"
#include "nm_interface.h"
#include "blocklist.h"
#include "os/NandAlloc.h"
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/err.h>

#define HEAP_SIZE (4 * 1024 * 1024 / 2)

typedef struct __nm_intf {
	int handler;	   		/* nand manager handler */
	int refcnt;				/* nm_intf open count */
	int is_scaned;
	NM_lpt *lptl;			/* lpartition list head */
	NM_ppt *pptl;			/* ppartition list head */
	void *heap;
	int (*ndPtInstall)(char *);
	//struct __nm_lock_tab lock;
} NM_intf;

static NM_intf *nm_intf = NULL;

/******************************** static ******************************/
static inline NM_lpt* find_lpt_by_name(const char *name)
{
	struct singlelist *plist;
	NM_lpt *lpt = NULL;

	if (nm_intf->lptl) {
		singlelist_for_each(plist, &nm_intf->lptl->list) {
			lpt = singlelist_entry(plist, NM_lpt, list);
			if (!strcmp(lpt->pt->name, name))
				break;
		}
	}
	return lpt;
}

static inline NM_ppt* find_ppt_by_name(const char *name)
{
	struct singlelist *plist;
	NM_ppt *ppt = NULL;

	if (nm_intf->pptl) {
		singlelist_for_each(plist, &nm_intf->pptl->list) {
			ppt = singlelist_entry(plist, NM_ppt, list);
			if (!strcmp(ppt->pt->name, name))
				break;
		}
	}
	return ppt;
}

static inline void free_lptl(void) {
	struct singlelist *plist = NULL;
	struct singlelist *head = &nm_intf->lptl->list;
	NM_lpt *lpt;

	for (plist = head; plist != NULL; plist = head) {
		head = plist->next;
		lpt = singlelist_entry(plist, NM_lpt, list);
		vfree(lpt);
	}
}

static inline int nm_can_close(void) {
	struct singlelist *plist;
	NM_lpt *lpt;

	if (nm_intf->lptl) {
		singlelist_for_each(plist, &nm_intf->lptl->list) {
			lpt = singlelist_entry(plist, NM_lpt, list);
			if (lpt->refcnt) {
				printk("WARNING: partition [%s] is still in use!\n", lpt->pt->name);
				return 0;
			}
		}
	}

	return 1;
}

/******************************** public ******************************/
/**
 * @name: must point to the string
 *  which lpt->pt->name pointed,
 **/
int NM_ptOpen(int handler, const char *name, int mode)
{
	NM_lpt *lpt;

	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return 0;
	}

	lpt = find_lpt_by_name(name);

	if (!lpt->context) {
		lpt->context = NandManger_ptOpen(nm_intf->handler, name, mode);
	}

	if (!lpt->context) {
		printk("ERROR: %s, Open lpartition [%s] faild, mode = [%d]\n", __func__, name, mode);
		return 0;
	}

	lpt->refcnt++;

	return (int)lpt;
}

int NM_ptRead(int context, SectorList* bl)
{
	NM_lpt *lpt = (NM_lpt *)context;

	if (!lpt || IS_ERR(lpt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_ptRead(lpt->context, bl);
}

int NM_ptWrite(int context, SectorList* bl)
{
	NM_lpt *lpt = (NM_lpt *)context;

	if (!lpt || IS_ERR(lpt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_ptWrite(lpt->context, bl);
}

int NM_ptIoctrl(int context, int cmd, int args)
{
	NM_lpt *lpt = (NM_lpt *)context;

	if (!lpt || IS_ERR(lpt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_ptIoctrl(lpt->context, cmd, args);
}

int NM_ptErase(int context)
{
	NM_lpt *lpt = (NM_lpt *)context;

	if (!lpt || IS_ERR(lpt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_ptErase(lpt->context);
}

int NM_ptClose(int context)
{
	NM_lpt *lpt = (NM_lpt *)context;

	if (!lpt || IS_ERR(lpt) || !lpt->context || (--lpt->refcnt < 0)) {
		printk("ERROR: %s, Can't close partition!\n", __func__);
		return -1;
	}

	if (!lpt->refcnt) {
		return NandManger_ptClose(lpt->context);
	}
	return 0;
}

NM_lpt* NM_getPartition(int handler)
{
	NM_lpt *lpt;
	LPartition *phead = NULL;
	struct singlelist *plist;

	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return NULL;
	}

	if (nm_intf->lptl)
		return nm_intf->lptl;

	if (NandManger_getPartition(nm_intf->handler, &phead) || (!phead)) {
		printk("ERROR: %s, get NandManger partition faild! phead = %p\n", __func__, phead);
		return NULL;
	}

	singlelist_for_each(plist, &phead->head) {
		lpt = vzalloc(sizeof(NM_lpt));
		if (!lpt) {
			printk("ERROR: %s, can't alloc memory for NM_lpt\n", __func__);
			free_lptl();
			return NULL;
		}

		lpt->pt = singlelist_entry(plist, LPartition, head);
		if (nm_intf->lptl)
			singlelist_add_tail(&nm_intf->lptl->list, &lpt->list);
		else
			nm_intf->lptl = lpt;
	}

	return nm_intf->lptl;
}

void NM_startNotify(int handler, void (*start)(int), int prdata)
{
	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return;
	}

	NandManger_startNotify(nm_intf->handler, start, prdata);
}

NM_ppt* NM_getPPartition(int handler)
{
	int i;
	PPartArray *ppa;

	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return NULL;
	}

	if (nm_intf->pptl)
		return nm_intf->pptl;

	ppa =  NandManger_getDirectPartition(nm_intf->handler);
	if (!ppa) {
		printk("ERROR: %s, get NandManger partition faild!\n", __func__);
		return NULL;
	}

	nm_intf->pptl = vzalloc(ppa->ptcount * sizeof(NM_ppt));
	if (!nm_intf->pptl) {
		printk("ERROR: %s, can't alloc memory for nm_ppt\n", __func__);
		return NULL;
	}

	for (i = 0; i < ppa->ptcount; i++) {
		nm_intf->pptl[i].pt = &ppa->ppt[i];
		if (i > 0)
			singlelist_add_tail(&nm_intf->pptl[i - 1].list,
								&nm_intf->pptl[i].list);
	}

	return nm_intf->pptl;
}

int NM_DirectRead(NM_ppt *ppt, int pageid, int off_t, int bytes, void *data)
{
	if (!ppt || IS_ERR(ppt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_DirectRead(nm_intf->handler, ppt->pt, pageid, off_t, bytes, data);
}

int NM_DirectWrite(NM_ppt *ppt, int pageid, int off_t, int bytes, void *data)
{
	if (!ppt || IS_ERR(ppt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_DirectWrite(nm_intf->handler, ppt->pt, pageid, off_t, bytes, data);
}

int NM_DirectErase(NM_ppt *ppt, int blockid)
{
	BlockList bl;

	if (!ppt || IS_ERR(ppt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}

	bl.startBlock = blockid;
	bl.BlockCount = 1;
	bl.head.next = NULL;
	return NandManger_DirectErase(nm_intf->handler, ppt->pt, &bl);
}

int NM_DirectIsBadBlock(NM_ppt *ppt, int blockid)
{
	if (!ppt || IS_ERR(ppt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_DirectIsBadBlock(nm_intf->handler, ppt->pt, blockid);
}

int NM_DirectMarkBadBlock(NM_ppt *ppt, int blockid)
{
	if (!ppt || IS_ERR(ppt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}
	return NandManger_DirectMarkBadBlock(nm_intf->handler, ppt->pt, blockid);
}

int NM_UpdateErrorPartition(NM_ppt *ppt)
{
	if (IS_ERR(ppt)) {
		printk("ERROR: %s, error context!\n", __func__);
		return -1;
	}

	return NandManger_Ioctrl(nm_intf->handler, NANDMANAGER_UPDATE_ERRPT, (int)(ppt->pt));
}

int NM_PrepareNewFlash(int handler)
{
	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return -1;
	}

	return NandManger_Ioctrl(nm_intf->handler, NANDMANAGER_PREPARE_NEW_FLASH, 0);
}

int NM_CheckUsedFlash(int handler)
{
	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return -1;
	}

	return NandManger_Ioctrl(nm_intf->handler, NANDMANAGER_CHECK_USED_FLASH, 0);
}

int NM_EraseFlash(int handler, int force)
{
	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return -1;
	}

	return NandManger_Ioctrl(nm_intf->handler, force, 0);
}

int NM_open(void)
{
	if (!nm_intf) {
		nm_intf = vzalloc(sizeof(NM_intf));
		if (!nm_intf) {
			printk("ERROR: %s, Can't alloc memory for nm_interface!\n", __func__);
			return 0;
		}
		nm_intf->heap = (void*)kmalloc(HEAP_SIZE, GFP_KERNEL);
		nm_intf->handler = NandManger_Init(nm_intf->heap, HEAP_SIZE, 0);
		if (!nm_intf->handler) {
			printk("ERROR: %s, Init NandManger faild!\n", __func__);
			return 0;
		}
	}

	nm_intf->refcnt++;

	printk("nm_interface open ok!\n");
	return (int)nm_intf;
}

void NM_close(int handler)
{
	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return;
	}

	if (!nm_intf || (--nm_intf->refcnt < 0)) {
		printk("ERROR: %s(%d), Can't close nm_interface!\n", __func__, __LINE__);
		return;
	}

	if (!nm_intf->refcnt) {
		if (!nm_can_close()) {
			printk("ERROR: %s(%d), Can't close nm_interface!\n", __func__, __LINE__);
			return;
		}

		if (nm_intf->lptl)
			free_lptl();
		if (nm_intf->pptl)
			vfree(nm_intf->pptl);
		NandManger_DeInit(nm_intf->handler);
		Nand_MemoryDeinit();
		kfree(nm_intf->heap);
		vfree(nm_intf);
		nm_intf = NULL;
	}

	printk("nm_interface close ok!\n");
}

void NM_regPtInstallFn(int handler, int data)
{
	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return;
	}

	if (!data)
		printk("ERROR: %s, invalid PtInstallFn!\n", __func__);

	nm_intf->ndPtInstall = (void *)data;
}

int NM_ptInstall(int handler, char *ptname)
{
	if ((NM_intf*)handler != nm_intf) {
		printk("ERROR: %s, Unknown NM handler!\n", __func__);
		return -1;
	}

	if (nm_intf->ndPtInstall)
		return nm_intf->ndPtInstall(ptname);

	printk("ERROR: %s, PtInstallFn has not been installed!\n", __func__);
	return -1;
}
