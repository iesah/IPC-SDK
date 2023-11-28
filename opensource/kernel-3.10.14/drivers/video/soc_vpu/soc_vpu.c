#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <mach/libdmmu.h>
#include <asm/delay.h>

#include "soc_vpu.h"

#define SOC_VPU_NAME	"soc_vpu"

/*
 * The vpu_list and channel_list will be clear to be zero when compiling,
 * so we needn't care its default value.
 */
static struct channel_list channel_list[CONFIG_CHANNEL_NODE_NUM];
static int vpu_node_num = 0;
static struct vpu_list vpu_list[CONFIG_VPU_NODE_NUM];
struct soc_channel *soc_channel = NULL;
/*
 * Vpu_register needn't lock because there are only
 * one process using it at the register context.
 * if think like so, this function is so simple.
 */
int vpu_register(struct list_head *vlist)
{
	if (!vlist)
		return -EINVAL;
	if (vpu_node_num > (CONFIG_VPU_NODE_NUM - 1)) {
		printk("ERROR: no space to add the %p vpu node", vlist);
		return -ENOMEM;
	}

	if (vpu_node_num == 0) {
		memset(&vpu_list, 0, sizeof(struct vpu_list));
	}

	/* add the vlist info to vpu_list */
	vpu_list[vpu_node_num].vlist = vlist;
	vpu_list[vpu_node_num].phase = INIT_VPU;
	vpu_list[vpu_node_num].user_cnt = 0;
	vpu_list[vpu_node_num].tgid = -1;
	vpu_list[vpu_node_num].pid = -1;
	spin_lock_init(&vpu_list[vpu_node_num].slock);
	vpu_node_num++;

	return 0;
}

int vpu_unregister(struct list_head *vlist)
{
	/* delete the vlist info from vpu_list */
	int i = 0;

	for (i = 0; i < vpu_node_num; i++) {
		if (vlist == vpu_list[i].vlist) {
			int j = i;
			for (j = i; j < vpu_node_num - 1; j++) {
				vpu_list[j] = vpu_list[j+1];
			}
			vpu_list[j].vlist = NULL;
			vpu_list[j].phase = INIT_VPU;
			vpu_list[j].user_cnt = 0;
			vpu_list[j].tgid = -1;
			vpu_list[j].pid = -1;
			vpu_node_num--;
			break;
		}
	}

	return 0;
}

#define TLB
#ifdef TLB
static int channel_tlb_init(struct device *dev, struct channel_list *clist)
{
	int ret = 0;
	struct channel_tlb_pidmanager *p = NULL;
	unsigned long slock_flag = 0;

	/* first */
	if (clist->tlb_pidmanager == NULL) {
		dev_dbg(dev, "[func:%s, line:%d] clist->tlb_pidmanager == NULL\n", __func__, __LINE__);
#if 0
		ret = dmmu_init();
		if(ret < 0) {
			goto err_dmmu_init;
		}
#endif
	} else {
		if (current->tgid == clist->tlb_pidmanager->tgid) {
			p = clist->tlb_pidmanager;
		}
	}

	/* if current->tgid isn't in tlb_list, alloc a channel_tlb_pidmanager */
	if(!p){
		p = kzalloc(sizeof(*p), GFP_KERNEL);
		if(!p){
			dev_err(dev, "[func:%s, line:%d] kzalloc failed\n", __func__, __LINE__);
			ret = -EINVAL;
			goto err_kzalloc_tlb_pidmanager;
		}
		p->tgid = current->tgid;
		INIT_LIST_HEAD(&(p->vaddr_list));
#if 0
		ret = dmmu_get_page_table_base_phys(&(p->tlbbase));
		if(ret < 0) {
			dev_err(dev, "[func:%s, line:%d] get tlbbase failed\n", __func__, __LINE__);
			goto err_get_tlb_base_phys;
		}
#endif
	}

	spin_lock_irqsave(&clist->slock, slock_flag);
	clist->tlb_pidmanager = p;
	clist->tlb_flag = true;
	spin_unlock_irqrestore(&clist->slock, slock_flag);

	return 0;
#if 0
err_get_tlb_base_phys:
	kfree(p);
#endif
err_kzalloc_tlb_pidmanager:
#if 0
err_dmmu_init:
#endif
	return ret;
}

static int channel_tlb_deinit(struct device *dev, struct channel_list *clist)
{
	int ret = 0;
	struct channel_tlb_pidmanager *p = NULL;
	struct channel_tlb_vaddrmanager *vaddr = NULL;
	struct list_head *pos = NULL;
	unsigned long slock_flag = 0;

	/* if tlb_list isn't empty , check tgid */
	if (clist->tlb_pidmanager && current->tgid == clist->tlb_pidmanager->tgid) {
		p = clist->tlb_pidmanager;
	}

	/* if current->tgid is find, release it  */
	if (p) {
		spin_lock_irqsave(&clist->slock, slock_flag);
		clist->tlb_pidmanager = NULL;
		spin_unlock_irqrestore(&clist->slock, slock_flag);
		/* if vaddr_list of the tlb_pidmanager, release it */
		if(!list_empty(&(p->vaddr_list))){
			list_for_each(pos, &(p->vaddr_list)){
				vaddr = list_entry(pos, struct channel_tlb_vaddrmanager,vaddr_entry);
				list_del(&(vaddr->vaddr_entry));
				kfree(vaddr);
				pos = &(p->vaddr_list);
			}
		}
		kfree(p);
	}

	if (clist->tlb_pidmanager == NULL) {
#if 0
		ret = dmmu_deinit();
#endif
		spin_lock_irqsave(&clist->slock, slock_flag);
		clist->tlb_flag = false;
		spin_unlock_irqrestore(&clist->slock, slock_flag);
	}

	return ret;
}

static int channel_tlb_map_one_vaddr(struct device *dev, struct channel_list *clist, struct buf_info *buf_info)
{
	int ret = 0;
	unsigned long tlbbase = 0;
	struct channel_tlb_pidmanager *p = NULL;
	struct channel_tlb_vaddrmanager *v = NULL;
	struct channel_tlb_vaddrmanager *prev = NULL;
	struct channel_tlb_vaddrmanager *after = NULL;
	struct channel_tlb_vaddrmanager *tmv = NULL;

	if (clist->tlb_pidmanager == NULL) {
		dev_err(dev, "[func:%s, line:%d] vaddr can't map because tlb isn't inited!\n", __func__, __LINE__);
		ret = -EINVAL;
		goto err_tlb_not_init;
	}

	if (current->tgid == clist->tlb_pidmanager->tgid) {
		p = clist->tlb_pidmanager;
	}

	if(!p){
		ret = -EINVAL;
		dev_err(dev, "[func:%s, line:%d] can't find tlb pidmanager!\n", __func__, __LINE__);
		goto err_canot_find_tlb_pidmanager;
	}

	list_for_each_entry(tmv, &(p->vaddr_list), vaddr_entry){
		if(tmv->vaddr <= (unsigned int)buf_info->alloc_vaddr && (unsigned int)buf_info->alloc_vaddr < tmv->vaddr + tmv->size){
			v = tmv;
			break;
		}else if((unsigned int)buf_info->alloc_vaddr < tmv->vaddr){
			after = tmv;
			break;
		}else
			prev = tmv;
	}
	/* after != NULL or prev != NULL */
	if(!v){
		if(prev && (prev->vaddr + prev->size == (unsigned int)buf_info->alloc_vaddr)){
			/* prev and current are sequential */
			dev_dbg(dev, "[func:%s, line:%d] [prev-current:vaddr = %x]\n", __func__, __LINE__, buf_info->alloc_vaddr);
			prev->size += buf_info->alloc_size;
			v = prev;
		}
		if(after && ((unsigned int)buf_info->alloc_vaddr + buf_info->alloc_size == after->vaddr)){
			if(!v){
				/* after and current are sequential */
				dev_dbg(dev, "[func:%s, line:%d] [current-after:vaddr = %x]\n", __func__, __LINE__, buf_info->alloc_vaddr);
				after->vaddr = (unsigned int)buf_info->alloc_vaddr;
				after->size += buf_info->alloc_size;
				v = after;
			}else{
				/* prev, after and current are sequential */
				dev_dbg(dev, "[func:%s, line:%d] [prev-current-after:vaddr = %x]\n", __func__, __LINE__, buf_info->alloc_vaddr);
				prev->size += after->size;
				list_del(&(after->vaddr_entry));
				kfree(after);
			}
		}
		if(!v){
			/* prev ,after and current are discontinuous */
			dev_dbg(dev, "[func:%s, line:%d] alloc_vaddrmanager! vaddr = %x\n", __func__, __LINE__, buf_info->alloc_vaddr);
			v = kzalloc(sizeof(*v), GFP_KERNEL);
			if(!v){
				dev_err(dev, "[func:%s, line:%d] alloc_vaddrmanager failed!\n", __func__, __LINE__);
				ret = -EINVAL;
				goto err_alloc_vaddrmanager;
			}
			v->vaddr = (unsigned int)buf_info->alloc_vaddr;
			v->size = buf_info->alloc_size;
			if(prev){
				dev_dbg(dev, "[func:%s, line:%d] instand prev vaddr = %x\n", __func__, __LINE__, buf_info->alloc_vaddr);
				prev->vaddr_entry.next->prev = &(v->vaddr_entry);
				v->vaddr_entry.prev = &(prev->vaddr_entry);
				v->vaddr_entry.next = prev->vaddr_entry.next;
				prev->vaddr_entry.next = &(v->vaddr_entry);
			}else if(after){
				dev_dbg(dev, "[func:%s, line:%d] instand after vaddr = %x\n", __func__, __LINE__, buf_info->alloc_vaddr);
				after->vaddr_entry.prev->next = &(v->vaddr_entry);
				v->vaddr_entry.next = &(after->vaddr_entry);
				v->vaddr_entry.prev = after->vaddr_entry.prev;
				after->vaddr_entry.prev = &(v->vaddr_entry);
			}else {
				list_add_tail(&(v->vaddr_entry), &(p->vaddr_list));
			}
		}
#if 0
		ret = dmmu_match_user_mem_tlb((void *)buf_info->alloc_vaddr, buf_info->alloc_size);
		if(ret < 0) {
			goto err_dmmu_match_user_mem_tlb;
		}
		ret = dmmu_map_user_mem((void *)buf_info->alloc_vaddr, buf_info->alloc_size);
#endif
		tlbbase = dmmu_map(dev, buf_info->alloc_vaddr, buf_info->alloc_size);
		if(tlbbase < 0)
			goto err_dmmu_map_user_mem;
		p->tlbbase = tlbbase;
	}

	return 0;

err_dmmu_map_user_mem:
#if 0
err_dmmu_match_user_mem_tlb:
#endif
	kfree(v);
err_alloc_vaddrmanager:
err_canot_find_tlb_pidmanager:
err_tlb_not_init:
	return ret;
}

static int channel_tlb_unmap_all_vaddr(struct device *dev, struct channel_list *clist)
{
	struct channel_tlb_pidmanager *p = NULL;
	struct channel_tlb_vaddrmanager *v = NULL;
	struct list_head *pos = NULL;
	/* if tlb_list isn't empty , check tgid */
	if (clist->tlb_pidmanager && current->tgid == clist->tlb_pidmanager->tgid) {
		p = clist->tlb_pidmanager;

		/* if find, release it's vaddr_list */
		if(p){
			dev_dbg(dev, "[func:%s, line:%d] unmap\n", __func__, __LINE__);
			/* if vaddr_list of the tlb_pidmanager isn't empty, release it */
			if(!list_empty(&(p->vaddr_list))){
				//dmmu_unmap_all(dev);
				list_for_each(pos, &(p->vaddr_list)) {
					v = list_entry(pos, struct channel_tlb_vaddrmanager,vaddr_entry);
					list_del(&(v->vaddr_entry));
#if 0
					dmmu_unmap_user_mem((void *)(v->vaddr), v->size);
#endif
					dmmu_unmap(dev, v->vaddr, v->size);
					kfree(v);
					pos = &(p->vaddr_list);
				}
			}
		}
	}

	return 0;
}
#endif

static long soc_channel_request(struct soc_channel *sc, long usr_arg)
{
	struct channel_node cnode;
	long ret = 0;
	unsigned long fcflag = 0, clflag = 0;
	struct free_channel_list *fclist = NULL;
	struct channel_list *clist = NULL;

	if (copy_from_user(&cnode, (void *)usr_arg, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}
	fclist = list_entry(sc->fclist_head, struct free_channel_list, fclist_head);
find_channel_try_point:
	ret = wait_for_completion_interruptible_timeout(&fclist->cdone, msecs_to_jiffies(cnode.mdelay));
	if (ret > 0) {
		spin_lock_irqsave(&fclist->slock, fcflag);
		if (list_empty(&fclist->fclist_head)) {
			spin_unlock_irqrestore(&fclist->slock, fcflag);
			while(try_wait_for_completion(&fclist->cdone));
			ret = -EBUSY;
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] fclist err empty for not match complete\n", __func__, __LINE__);
			goto err_fclist_empty;
		}
		cnode.clist = fclist->fclist_head.next;
		list_del_init(cnode.clist);
		spin_unlock_irqrestore(&fclist->slock, fcflag);

		clist = list_entry(cnode.clist, struct channel_list, list);

		if ((clist->tlb_flag == true) && (clist->tlb_pidmanager)) {
			if ((ret = channel_tlb_unmap_all_vaddr(sc->mdev.this_device, clist)) < 0) {
				dev_err(sc->mdev.this_device, "[fun:%s,line:%d] tlb_unmap failed\n", __func__, __LINE__);
				goto err_tlb_unmap_all_vaddr;
			}

			if ((ret = channel_tlb_deinit(sc->mdev.this_device, clist)) < 0) {
				dev_err(sc->mdev.this_device, "[fun:%s,line:%d] tlb_deinit failed\n", __func__, __LINE__);
				goto err_tlb_deinit;
			}
		}
		spin_lock_irqsave(&clist->slock, clflag);
		clist->tlb_flag = false;
		clist->tlb_pidmanager = NULL;
		clist->phase = REQUEST_CHANNEL;
		clist->tgid = current->tgid;
		clist->pid = current->pid;
		cnode.channel_id = clist->id;
		spin_unlock_irqrestore(&clist->slock, clflag);

		if (copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node))) {
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_to_user failed\n", __func__, __LINE__);
			ret = -EINVAL;
			goto err_copy_to_user;
		}
	} else if (ret == -ERESTARTSYS) {
		dev_dbg(sc->mdev.this_device, "[fun:%s,line:%d] fclist wait is interrupt, ingnore this info\n", __func__, __LINE__);
		goto find_channel_try_point;
	} else {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] fclist is empty\n", __func__, __LINE__);
		return -EBUSY;
	}

	return 0;

err_copy_to_user:
	spin_lock_irqsave(&clist->slock, clflag);
	clist->phase = INIT_CHANNEL;
	clist->tgid = -1;
	clist->pid = -1;
	spin_unlock_irqrestore(&clist->slock, clflag);
err_tlb_deinit:
err_tlb_unmap_all_vaddr:
	spin_lock_irqsave(&fclist->slock, fcflag);
	list_add_tail(cnode.clist, &fclist->fclist_head);
	complete(&fclist->cdone);
	spin_unlock_irqrestore(&fclist->slock, fcflag);
	cnode.clist = NULL;
err_fclist_empty:
err_copy_from_user:

	return ret;
}

static long soc_vpu_request(struct soc_channel *sc, struct channel_node *cnode)
{
	long ret = 0;
	unsigned long fvflag = 0, vlflag = 0;
	struct free_vpu_list *fvlist = NULL;
	struct vpu_list *vlist = NULL;
	volatile int find_vpu_try_times = 0;
	//struct timespec start, end;

	fvlist = list_entry(sc->fvlist_head, struct free_vpu_list, fvlist_head);

	//start = current_kernel_time();
find_vpu_try_point:
	ret = wait_for_completion_interruptible_timeout(&fvlist->vdone, msecs_to_jiffies(cnode->mdelay));
	if (ret > 0) {
		struct vpu *vpu = NULL;
		spin_lock_irqsave(&fvlist->slock, fvflag);
		/* the empty fvlist->fvlist_head represent the done flag is zero */
		if (list_empty(&fvlist->fvlist_head)) {
			fvlist->vdone.done = 0;
			spin_unlock_irqrestore(&fvlist->slock, fvflag);
			while(try_wait_for_completion(&fvlist->vdone));
			ret = -EBUSY;
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] fvlist err empty\n", __func__, __LINE__);
			goto err_fvlist_empty;
		}
		if (cnode->vpu_id != RANDOM_ID) {
			struct vpu_list *vlist_ptr = NULL;
			struct vpu *vpu_ptr = NULL;
			bool vpu_found = false;
			list_for_each_entry(vlist_ptr, &fvlist->fvlist_head, list) {
				vpu_ptr = vlist_ptr ? list_entry(vlist_ptr->vlist, struct vpu, vlist) : NULL;
				if (vpu_ptr && ((vpu_ptr->vpu_id & cnode->vpu_id) == cnode->vpu_id)) {
					vpu_found = true;
					break;
				}
			}
			if (vpu_found == true) {
				cnode->vlist = &vlist_ptr->list;
				vlist = vlist_ptr;
				vpu = vpu_ptr;
				//end = current_kernel_time();
			} else {
				if (find_vpu_try_times++ < FIND_VPU_TRY_TIME_THRESHOLD) {
					complete(&fvlist->vdone);
					spin_unlock_irqrestore(&fvlist->slock, fvflag);
					udelay(2000);
					goto find_vpu_try_point;
				} else {
					spin_unlock_irqrestore(&fvlist->slock, fvflag);
					ret = -EBUSY;
					dev_err(sc->mdev.this_device, "[fun:%s,line:%d] can't find cnode->vpu_id = %d\n", __func__, __LINE__, cnode->vpu_id);
					goto err_not_find_request_vpu;
				}
			}
		} else {
			cnode->vlist = fvlist->fvlist_head.next;
			vlist = list_entry(cnode->vlist, struct vpu_list, list);
			vpu = list_entry(vlist->vlist, struct vpu, vlist);
		}
		list_del_init(cnode->vlist);
		spin_unlock_irqrestore(&fvlist->slock, fvflag);

		spin_lock_irqsave(&vlist->slock, vlflag);
		if (vlist->user_cnt == 0) {
			if ((vlist->phase == INIT_VPU) && (vpu->ops->open)
					&& ((ret = vpu->ops->open(vpu->dev)) >= 0)) {
				vlist->phase = OPEN_VPU;
				dev_dbg(sc->mdev.this_device, "[fun:%s,line:%d] vpu_open success\n", __func__, __LINE__);
				vlist->user_cnt++;
			} else {
				spin_unlock_irqrestore(&vlist->slock, vlflag);
				dev_err(sc->mdev.this_device, "[fun:%s,line:%d] vpu_open failed\n", __func__, __LINE__);
				ret = -EINVAL;
				goto err_vpu_open;
			}
		}
		vlist->tgid = current->tgid;
		vlist->pid = current->pid;
		vlist->phase = REQUEST_VPU;
		spin_unlock_irqrestore(&vlist->slock, vlflag);

		spin_lock_irqsave(&fvlist->slock, fvflag);
		cnode->workphase = WORKING;
		spin_unlock_irqrestore(&fvlist->slock, fvflag);
	} else if (ret == -ERESTARTSYS) {
		dev_dbg(sc->mdev.this_device, "[fun:%s,line:%d] fvlist wait is interrupt, ingnore this info\n", __func__, __LINE__);
		goto find_vpu_try_point;
	} else {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] fvlist is empty\n", __func__, __LINE__);
		return -EBUSY;
	}
	//printk("soc_vpu_start cost time = %lld, find_vpu_try_times = %d\n", ktime_to_us(timespec_to_ktime(end)) - ktime_to_us(timespec_to_ktime(start)), find_vpu_try_times);

	return 0;

err_vpu_open:
	spin_lock_irqsave(&fvlist->slock, fvflag);
	list_add_tail(cnode->vlist, &fvlist->fvlist_head);
	complete(&fvlist->vdone);
	spin_unlock_irqrestore(&fvlist->slock, fvflag);
	cnode->vlist = NULL;
err_not_find_request_vpu:
err_fvlist_empty:
	return ret;
}

static long soc_vpu_release_internal(struct soc_channel *sc, struct vpu_list *vlist)
{
	unsigned long fvflag = 0, vlflag = 0;
	struct vpu *vpu = NULL;
	struct free_vpu_list *fvlist = NULL;

	if (vlist) {
		vpu = list_entry(vlist->vlist, struct vpu, vlist);
		fvlist = list_entry(sc->fvlist_head, struct free_vpu_list, fvlist_head);
		spin_lock_irqsave(&vlist->slock, vlflag);
		if (vlist->phase >= OPEN_VPU) {
			if (vlist->phase < RELASE_VPU) {
				spin_lock_irqsave(&fvlist->slock, fvflag);
				list_add_tail(&vlist->list, &fvlist->fvlist_head);
				complete(&fvlist->vdone);
				vlist->phase = RELASE_VPU;
				spin_unlock_irqrestore(&fvlist->slock, fvflag);
			}
		}
		spin_unlock_irqrestore(&vlist->slock, vlflag);
	}

	return 0;
}

static long soc_vpu_release(struct soc_channel *sc, struct channel_node *cnode)
{
	struct vpu_list *vlist = NULL;
	struct vpu *vpu = NULL;
	unsigned long vlflag;

	if (cnode->vlist) {
		vlist = list_entry(cnode->vlist, struct vpu_list, list);
		vpu = list_entry(vlist->vlist, struct vpu, vlist);
		soc_vpu_release_internal(sc, vlist);
		spin_lock_irqsave(&vlist->slock, vlflag);
		if ((cnode->workphase == CLOSE) && ((vlist->user_cnt > 0) && (--vlist->user_cnt == 0))) {
			if (vpu->ops->release) {
				vpu->ops->release(vpu->dev);
				vlist->phase = INIT_VPU;
				vlist->tgid = -1;
				vlist->pid = -1;
			}
		}
		spin_unlock_irqrestore(&vlist->slock, vlflag);
		cnode->vlist = NULL;
	}

	return 0;
}

static long soc_channel_release_internal(struct soc_channel *sc, struct channel_list *clist)
{
	long ret = 0;
	unsigned long fcflag, clflag;
	struct free_channel_list *fclist = NULL;

	if (clist) {
		fclist = list_entry(sc->fclist_head, struct free_channel_list, fclist_head);
		if ((clist->tlb_flag == true) && (clist->tlb_pidmanager)) {
			if ((ret = channel_tlb_unmap_all_vaddr(sc->mdev.this_device, clist)) < 0) {
				dev_err(sc->mdev.this_device, "[fun:%s,line:%d] tlb_unmap failed\n", __func__, __LINE__);
				goto err_tlb_unmap_all_vaddr;
			}

			if ((ret = channel_tlb_deinit(sc->mdev.this_device, clist)) < 0) {
				dev_err(sc->mdev.this_device, "[fun:%s,line:%d] tlb_deinit failed\n", __func__, __LINE__);
				goto err_tlb_deinit;
			}
		}
		spin_lock_irqsave(&clist->slock, clflag);
		clist->tlb_pidmanager = NULL;
		clist->tlb_flag = false;
		spin_unlock_irqrestore(&clist->slock, clflag);
err_tlb_deinit:
err_tlb_unmap_all_vaddr:

		if (clist->phase >= REQUEST_CHANNEL) {
			spin_lock_irqsave(&clist->slock, clflag);
			clist->phase = INIT_CHANNEL;
			clist->tgid = -1;
			clist->pid = -1;
			spin_unlock_irqrestore(&clist->slock, clflag);

			spin_lock_irqsave(&fclist->slock, fcflag);
			list_add_tail(&clist->list, &fclist->fclist_head);
			complete(&fclist->cdone);
			spin_unlock_irqrestore(&fclist->slock, fcflag);
		}
	}

	return 0;
}

static long soc_channel_release(struct soc_channel *sc, long usr_arg)
{
	struct channel_node cnode;
	struct channel_list *clist = NULL;
	long ret = 0;

	if (copy_from_user(&cnode, (void *)usr_arg, sizeof(struct channel_node))) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		ret = -EINVAL;
		goto err_copy_from_user;
	}

	soc_vpu_release(sc, &cnode);
	if (cnode.clist) {
		clist = list_entry(cnode.clist, struct channel_list, list);
		soc_channel_release_internal(sc, clist);
		cnode.clist = NULL;
	}

	if (copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_to_user failed\n", __func__, __LINE__);
		goto err_copy_to_user;
	}

	return 0;

err_copy_to_user:
err_copy_from_user:
	return ret;
}

static long soc_vpu_reset(struct soc_channel *sc, struct channel_node *cnode)
{
	long ret = 0;
	struct vpu_list *vlist = list_entry(cnode->vlist, struct vpu_list, list);
	struct vpu *vpu = list_entry(vlist->vlist, struct vpu, vlist);

	if (vpu->dev && vpu->ops && vpu->ops->reset) {
		ret = vpu->ops->reset(vpu->dev);
		if (ret < 0) {
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] vpu reset failed\n", __func__, __LINE__);
		}
	} else {
		ret = -ENODEV;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] no vpu device failed\n", __func__, __LINE__);
	}

	return ret;
}

static long soc_vpu_start(struct soc_channel *sc, struct channel_node *cnode)
{
	long ret = 0;
	struct vpu_list *vlist = list_entry(cnode->vlist, struct vpu_list, list);
	struct vpu *vpu = list_entry(vlist->vlist, struct vpu, vlist);

	if (vpu->dev && vpu->ops && vpu->ops->start_vpu) {
		unsigned long vlflag;
		ret = vpu->ops->start_vpu(vpu->dev, cnode);
		if (ret < 0) {
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] set vpu param failed\n", __func__, __LINE__);
			goto err_start_vpu;
		}
		spin_lock_irqsave(&vlist->slock, vlflag);
		vlist->phase = RUN_VPU;
		spin_unlock_irqrestore(&vlist->slock, vlflag);
		if (vpu->ops->wait_complete) {
			ret = vpu->ops->wait_complete(vpu->dev, cnode);
			if (ret < 0) {
				ret = -EIO;
				dev_err(sc->mdev.this_device, "[fun:%s,line:%d] wait timeout\n", __func__, __LINE__);
				goto err_wait_complete;
			}
		}
		spin_lock_irqsave(&vlist->slock, vlflag);
		vlist->phase = COMPLETE_VPU;
		spin_unlock_irqrestore(&vlist->slock, vlflag);
	} else {
		ret = -ENODEV;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] no vpu device failed\n", __func__, __LINE__);
	}

err_wait_complete:
err_start_vpu:
	return ret;
}

static long soc_vpu_start_on(struct soc_channel *sc, struct channel_node *cnode)
{
	long ret = 0;
	struct vpu_list *vlist = list_entry(cnode->vlist, struct vpu_list, list);
	struct vpu *vpu = list_entry(vlist->vlist, struct vpu, vlist);

	if (vpu->dev && vpu->ops && vpu->ops->start_vpu) {
		unsigned long vlflag;
		spin_lock_irqsave(&vlist->slock, vlflag);
		ret = vpu->ops->start_vpu(vpu->dev, cnode);
		if (ret < 0) {
			spin_unlock_irqrestore(&vlist->slock, vlflag);
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] set vpu param failed\n", __func__, __LINE__);
			goto err_start_vpu;
		}
		vlist->phase = RUN_VPU;
		spin_unlock_irqrestore(&vlist->slock, vlflag);
	} else {
		ret = -ENODEV;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] no vpu device failed\n", __func__, __LINE__);
	}

err_start_vpu:
	return ret;
}

static long soc_vpu_wait_complete(struct soc_channel *sc, struct channel_node *cnode)
{
	long ret = 0;
	struct vpu_list *vlist = list_entry(cnode->vlist, struct vpu_list, list);
	struct vpu *vpu = list_entry(vlist->vlist, struct vpu, vlist);

	if (vpu->dev && vpu->ops && vpu->ops->wait_complete) {
		ret = vpu->ops->wait_complete(vpu->dev, cnode);
		if (ret < 0) {
			ret = -EIO;
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] wait timeout\n", __func__, __LINE__);
			goto err_wait_complete;
		}
		vlist->phase = COMPLETE_VPU;
	} else {
		ret = -ENODEV;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] no vpu device failed\n", __func__, __LINE__);
	}

err_wait_complete:
	return ret;
}

static long soc_channel_flush_cache_all(struct soc_channel *sc, struct channel_node *cnode)
{
	struct channel_list *clist = list_entry(cnode->clist, struct channel_list, list);
	void *vir_addr = clist->tlb_flag == true ? (void *)cnode->dma_addr : phys_to_virt((unsigned long)cnode->dma_addr);
	size_t vir_size = (1024 << 10);	/* need more than 512K */

	dma_cache_sync(NULL, vir_addr, vir_size, WBACK_INV);

	return 0;
}

static long soc_channel_run(struct soc_channel *sc, long usr_arg)
{
	struct channel_node cnode;
	long ret = 0;

	if (copy_from_user(&cnode, (void *)usr_arg, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}

	if ((ret = soc_vpu_request(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] request vpu failed\n", __func__, __LINE__);
		goto  err_vpu_request;
	}

	if ((ret = soc_vpu_reset(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] reset vpu failed\n", __func__, __LINE__);
		goto err_vpu_reset;
	}

	soc_channel_flush_cache_all(sc, &cnode);
	if ((ret = soc_vpu_start(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] start vpu failed\n", __func__, __LINE__);
		goto err_vpu_start_vpu;
	}

	if ((ret = soc_vpu_release(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] release vpu failed\n", __func__, __LINE__);
		goto err_vpu_release_vpu;
	}

	if (copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_to_user failed\n", __func__, __LINE__);
		goto err_copy_to_user;
	}

	return 0;

err_copy_to_user:
err_vpu_release_vpu:
	copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node));
err_vpu_start_vpu:
err_vpu_reset:
	soc_vpu_release(sc, &cnode);
err_vpu_request:
err_copy_from_user:
	return ret;
}

static long soc_channel_start(struct soc_channel *sc, long usr_arg)
{
	struct channel_node cnode;
	long ret = 0;

	if (copy_from_user(&cnode, (void *)usr_arg, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}

	if ((ret = soc_vpu_request(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] request vpu failed\n", __func__, __LINE__);
		goto  err_vpu_request;
	}

	if ((ret = soc_vpu_reset(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] reset vpu failed\n", __func__, __LINE__);
		goto err_vpu_reset;
	}

	soc_channel_flush_cache_all(sc, &cnode);
	if ((ret = soc_vpu_start_on(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] start vpu on failed\n", __func__, __LINE__);
		goto err_vpu_start_vpu_on;
	}

	if (copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_to_user failed\n", __func__, __LINE__);
		goto err_copy_to_user;
	}

	return 0;

err_copy_to_user:
	copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node));
err_vpu_start_vpu_on:
err_vpu_reset:
	soc_vpu_release(sc, &cnode);
err_vpu_request:
err_copy_from_user:
	return ret;
}

static long soc_channel_wait_complete(struct soc_channel *sc, long usr_arg)
{
	struct channel_node cnode;
	long ret = 0;

	if (copy_from_user(&cnode, (void *)usr_arg, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}

	if ((ret = soc_vpu_wait_complete(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] start vpu on failed\n", __func__, __LINE__);
		goto err_vpu_wait_complete;
	}

	if ((ret = soc_vpu_release(sc, &cnode)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] release vpu failed\n", __func__, __LINE__);
		goto err_vpu_release_vpu;
	}

	if (copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_to_user failed\n", __func__, __LINE__);
		goto err_copy_to_user;
	}

	return 0;

err_copy_to_user:
err_vpu_release_vpu:
	copy_to_user((void *)usr_arg, &cnode, sizeof(struct channel_node));
err_vpu_wait_complete:
	soc_vpu_release(sc, &cnode);
err_copy_from_user:
	return ret;
}

static long soc_channel_flush_cache(struct soc_channel *sc, long usr_arg)
{
	struct flush_cache_info info;
	long ret = 0;
	if (copy_from_user(&info, (void *)usr_arg, sizeof(info))) {
		return -EFAULT;
	}

	dma_cache_sync(NULL, (void *)info.addr, info.len, info.dir);
	dev_dbg(sc->mdev.this_device, "[%d:%d] flush cache\n", current->tgid, current->pid);

	return ret;
}

static long soc_channel_buf_init(struct soc_channel *sc, long usr_arg)
{
	struct init_buf_info binfo;
	struct channel_list *clist;
	long ret = 0;

	if (copy_from_user(&binfo, (void *)usr_arg, sizeof(binfo))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}
	if (binfo.buf_info.tlb_needed == false) {
		dev_warn(sc->mdev.this_device, "[fun:%s,line:%d] you should check whether tlb_need is false\n", __func__, __LINE__);
		goto warn_tlb_needed_false;
	}

	clist = list_entry(binfo.clist, struct channel_list, list);

	if (clist->tlb_flag == false) {
		if ((ret = channel_tlb_init(sc->mdev.this_device, clist)) < 0) {
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] channel_tlb_init failed\n", __func__, __LINE__);
			goto err_channel_tlb_init;
		}
		clist->tlb_flag = true;
	}
	if ((ret = channel_tlb_map_one_vaddr(sc->mdev.this_device, clist, &binfo.buf_info)) < 0) {
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] tlb_map_vaddr failed\n", __func__, __LINE__);
		goto err_tlb_map_one_vaddr;
	}

	return 0;

err_tlb_map_one_vaddr:
err_channel_tlb_init:
warn_tlb_needed_false:
err_copy_from_user:
	return ret;
}

static long soc_channel_write_read_reg(struct soc_channel *sc, long usr_arg)
{
	struct reg_info reginfo;
	long ret = 0;

	if (copy_from_user(&reginfo, (void *)usr_arg, sizeof(reginfo))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}

	switch (reginfo.dir) {
	case WRITE_DIR:
		writel(reginfo.value, (void *)(reginfo.paddr | 0xa0000000));
		break;
	case READ_DIR:
		reginfo.value = readl((void *)(reginfo.paddr | 0xa0000000));
		if (copy_to_user((void *)usr_arg, &reginfo, sizeof(reginfo))) {
			ret = -EINVAL;
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_to_user failed\n", __func__, __LINE__);
			goto err_copy_to_user;
		}
		break;
	default:
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] invalidate reginfo\n", __func__, __LINE__);
		goto err_invalidate_reginfo;
		break;
	}

	return 0;

err_invalidate_reginfo:
err_copy_to_user:
err_copy_from_user:
	return ret;
}

static long soc_channel_vpu_suspend(struct soc_channel *sc, long usr_arg)
{
	long ret = 0;
	struct list_head *vpu_list = NULL;
	struct vpu_list *vlist = NULL;
	struct vpu *vpu = NULL;

	if (copy_from_user(&vpu_list, (void *)usr_arg, sizeof(vpu_list))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}

	vlist = list_entry(vpu_list, struct vpu_list, list);
	vpu = list_entry(vlist->vlist, struct vpu, vlist);
	if (vpu->dev && vpu->ops && vpu->ops->suspend) {
		ret = vpu->ops->suspend(vpu->dev);
		if (ret < 0) {
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] vpu suspend failed\n", __func__, __LINE__);
			goto err_vpu_suspend_failed;
		}
	} else {
		ret = -ENODEV;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] no vpu suspend device failed\n", __func__, __LINE__);
		goto err_vpu_suspend_nodev;
	}

	return 0;

err_vpu_suspend_nodev:
err_vpu_suspend_failed:
err_copy_from_user:
	return ret;
}

static long soc_channel_vpu_resume(struct soc_channel *sc, long usr_arg)
{
	long ret = 0;
	struct list_head *vpu_list = NULL;
	struct vpu_list *vlist = NULL;
	struct vpu *vpu = NULL;

	if (copy_from_user(&vpu_list, (void *)usr_arg, sizeof(vpu_list))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		goto err_copy_from_user;
	}

	vlist = list_entry(vpu_list, struct vpu_list, list);
	vpu = list_entry(vlist->vlist, struct vpu, vlist);
	if (vpu->dev && vpu->ops && vpu->ops->suspend) {
		ret = vpu->ops->resume(vpu->dev);
		if (ret < 0) {
			dev_err(sc->mdev.this_device, "[fun:%s,line:%d] vpu resume failed\n", __func__, __LINE__);
			goto err_vpu_resume_failed;
		}
	} else {
		ret = -ENODEV;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] no resume vpu device failed\n", __func__, __LINE__);
		goto err_vpu_suspend_nodev;
	}

	return 0;

err_vpu_suspend_nodev:
err_vpu_resume_failed:
err_copy_from_user:
	return ret;
	return 0;
}

static long soc_channel_private_tlb(struct soc_channel *sc, long usr_arg)
{
	struct init_buf_info binfo;
	struct channel_list *clist;
	long ret = 0;

	if (copy_from_user(&binfo, (void *)usr_arg, sizeof(binfo))) {
		ret = -EINVAL;
		dev_err(sc->mdev.this_device, "[fun:%s,line:%d] copy_from_user failed\n", __func__, __LINE__);
		return -1;
	}

	clist = list_entry(binfo.clist, struct channel_list, list);
	if(clist == NULL) {
		dev_warn(sc->mdev.this_device, "[fun:%s,line:%d] get clist is false\n", __func__, __LINE__);
		return -1;
	}

	if(binfo.buf_info.tlb_needed == true) {
		if (clist->private_tlb_flag == false) {
			if(clist->tlb_pidmanager == NULL) {
				clist->tlb_pidmanager = kzalloc(sizeof(struct channel_tlb_pidmanager), GFP_KERNEL);
				if(clist->tlb_pidmanager == NULL) {
					dev_warn(sc->mdev.this_device, "[fun:%s,line:%d] tlb manager alloc is error\n", __func__, __LINE__);
					return -1;
				}
			}
			clist->tlb_pidmanager->private_tlbbase = binfo.buf_info.alloc_paddr;
			clist->private_tlb_flag = true;
		}
	} else {
		if (clist->private_tlb_flag == true) {
			if(clist->tlb_pidmanager) {
				kfree(clist->tlb_pidmanager);
				clist->tlb_pidmanager = NULL;
			}
			clist->private_tlb_flag = false;
		}
	}

	return 0;
}

static int soc_channel_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct miscdevice *mdev = file->private_data;
	struct soc_channel *sc = list_entry(mdev, struct soc_channel, mdev);
	unsigned long scflag = 0;
	bool first_open = false;

	spin_lock_irqsave(&sc->cnt_slock, scflag);
	if (sc->user_cnt++  == 0) {
		first_open = true;
	}
	spin_unlock_irqrestore(&sc->cnt_slock, scflag);

	if (first_open == true) {
		// todo
	}

	//dev_info(sc->mdev.this_device, "in func:%s\n", __func__);

	return ret;
}

static int soc_channel_vpu_release(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct miscdevice *mdev = file->private_data;
	struct soc_channel *sc = list_entry(mdev, struct soc_channel, mdev);
	unsigned long scflag = 0, vlflag = 0;
	bool last_release = false;
	int i = 0;

	spin_lock_irqsave(&sc->cnt_slock, scflag);
	if (--sc->user_cnt  == 0) {
		last_release = true;
	}
	spin_unlock_irqrestore(&sc->cnt_slock, scflag);

	for (i = 0; i < vpu_node_num; i++) {
		if ((((vpu_list[i].tgid == current->tgid) && (vpu_list[i].pid == current->pid))
					|| (last_release == true)) && (vpu_list[i].user_cnt > 0)) {
			struct vpu *vpu = list_entry(vpu_list[i].vlist, struct vpu, vlist);
			soc_vpu_release_internal(sc, &vpu_list[i]);
			spin_lock_irqsave(&vpu_list[i].slock, vlflag);
			if ((((vpu_list[i].user_cnt > 0) && (--vpu_list[i].user_cnt == 0)) || (last_release == true)) && (vpu->ops->release)) {
				vpu->ops->release(vpu->dev);
				vpu_list[i].phase = INIT_VPU;
				vpu_list[i].user_cnt = 0;
				vpu_list[i].tgid = -1;
				vpu_list[i].pid = -1;
			}
			spin_unlock_irqrestore(&vpu_list[i].slock, vlflag);
		}
	}

	for (i = 0; i < CONFIG_CHANNEL_NODE_NUM; i++) {
		if (((channel_list[i].tgid == current->tgid) && (channel_list[i].pid == current->pid))
			|| (last_release == true)) {
			soc_channel_release_internal(sc, &channel_list[i]);
		}
	}

	if (last_release == true) {
		// todo
	}

	//dev_info(soc_channel->mdev.this_device, "in func:%s\n", __func__);

	return ret;
}

static loff_t soc_channel_llseek(struct file *file, loff_t offset, int origin)
{
	return -1;
}

static ssize_t soc_channel_read(struct file *file, char *buf, size_t size, loff_t *offset)
{
	return -1;
}

static ssize_t soc_channel_write(struct file *file, const char *buf, size_t size, loff_t *offset)
{
	return -1;
}

static unsigned int soc_channel_poll(struct file *file, struct poll_table_struct *poll_table)
{
	return -1;
}

static long soc_channel_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = -1;
	struct miscdevice *mdev = file->private_data;
	struct soc_channel *sc = list_entry(mdev, struct soc_channel, mdev);

	switch (cmd) {
	case IOCTL_CHANNEL_REQ:
		ret = soc_channel_request(sc, arg);
		break;
	case IOCTL_CHANNEL_REL:
		ret = soc_channel_release(sc, arg);
		break;
	case IOCTL_CHANNEL_RUN:
		ret = soc_channel_run(sc, arg);
		break;
	case IOCTL_CHANNEL_START:
		ret = soc_channel_start(sc, arg);
		break;
	case IOCTL_CHANNEL_WAIT_COMPLETE:
		ret = soc_channel_wait_complete(sc, arg);
		break;
	case IOCTL_CHANNEL_FLUSH_CACHE:
		ret = soc_channel_flush_cache(sc, arg);
		break;
	case IOCTL_CHANNEL_BUF_INIT:
		ret = soc_channel_buf_init(sc, arg);
		break;
	case IOCTL_CHANNEL_WOR_VPU_REG:
		ret = soc_channel_write_read_reg(sc, arg);
		break;
	case IOCTL_CHANNEL_VPU_SUSPEND:
		ret = soc_channel_vpu_suspend(sc, arg);
		break;
	case IOCTL_CHANNEL_VPU_RESUME:
		ret = soc_channel_vpu_resume(sc, arg);
		break;
	case IOCTL_CHANNEL_PRIVATE_TLB:
		ret = soc_channel_private_tlb(sc, arg);
		break;
	default:
		ret = -1;
	}

	return ret;
}

static int soc_channel_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (io_remap_pfn_range(vma,vma->vm_start,
			       vma->vm_pgoff,
			       vma->vm_end - vma->vm_start,
			       vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static const struct file_operations soc_channel_fops = {
	.owner		= THIS_MODULE,
	.open		= soc_channel_open,
	.release	= soc_channel_vpu_release,
	.llseek		= soc_channel_llseek,
	.read		= soc_channel_read,
	.write		= soc_channel_write,
	.poll		= soc_channel_poll,
	.unlocked_ioctl	= soc_channel_ioctl,
	.mmap		= soc_channel_mmap,
};

static int __init soc_channel_init(void)
{
	int i = 0;
	int ret = 0;
	struct free_channel_list *fclist = NULL;
	struct free_vpu_list *fvlist = NULL;

	if (vpu_node_num <= 0) {
		printk("Error[%s]failed:vpu_node_num is %d\n", __func__, vpu_node_num);
		ret = -EINVAL;
		goto err_vpu_node_num_is_invalid;
	}

	/* 1. alloc free_channel_list, free_vpu_list and soc_channel space */
	fclist = kzalloc(sizeof(*fclist), GFP_KERNEL);
	if (fclist == NULL) {
		printk("%s:kzalloc fclist failed\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_fclist;
	}

	fvlist = kzalloc(sizeof(*fvlist), GFP_KERNEL);
	if (fvlist == NULL) {
		printk("%s:kzalloc fvlist failed\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_fvlist;
	}

	soc_channel = kzalloc(sizeof(*soc_channel), GFP_KERNEL);
	if (soc_channel == NULL) {
		printk("%s:kzalloc soc_channel failed\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_soc_channel;
	}

	/* 2. init fclist */
	INIT_LIST_HEAD(&fclist->fclist_head);
	init_completion(&fclist->cdone);
	for (i = 0; i < CONFIG_CHANNEL_NODE_NUM; i++) {
		list_add_tail(&channel_list[i].list, &fclist->fclist_head);
		complete(&fclist->cdone);
		channel_list[i].id = i;
		channel_list[i].phase = INIT_CHANNEL;
		channel_list[i].tgid = -1;
		channel_list[i].pid = -1;
		spin_lock_init(&channel_list[i].slock);
		channel_list[i].tlb_flag = false;
		channel_list[i].tlb_pidmanager = NULL;
	}
	spin_lock_init(&fclist->slock);

	/* 3. init fvlist */
	INIT_LIST_HEAD(&fvlist->fvlist_head);
	init_completion(&fvlist->vdone);
	for (i = 0; i < vpu_node_num; i++) {
		list_add_tail(&vpu_list[i].list, &fvlist->fvlist_head);
		complete(&fvlist->vdone);
	}
	spin_lock_init(&fvlist->slock);

	/* 4. init soc_channel */
	soc_channel->fclist_head = &fclist->fclist_head;
	soc_channel->fvlist_head = &fvlist->fvlist_head;
	spin_lock_init(&soc_channel->cnt_slock);
	soc_channel->mdev.minor = MISC_DYNAMIC_MINOR;
	soc_channel->mdev.name = SOC_VPU_NAME;
	soc_channel->mdev.fops = &soc_channel_fops;

	if ((ret = misc_register(&soc_channel->mdev)) < 0) {
		printk("%s:misc_register failed\n", __func__);
		goto err_register_soc_channel;
	}
	printk("soc_vpu probe success,version:%s", SOC_VPU_VERSION);

	return 0;

err_register_soc_channel:
	kfree(soc_channel);
err_alloc_soc_channel:
	kfree(fvlist);
err_alloc_fvlist:
	kfree(fclist);
err_alloc_fclist:
err_vpu_node_num_is_invalid:
	return ret;
}

static void __exit soc_channel_exit(void)
{
	if (soc_channel) {
		struct free_channel_list *fclist = NULL;
		struct free_vpu_list *fvlist = NULL;

		if (soc_channel->fclist_head) {
			fclist = list_entry(soc_channel->fclist_head, struct free_channel_list, fclist_head);
			kfree(fclist);
			fclist = NULL;
			memset(channel_list, 0, sizeof(channel_list));
		}

		if (soc_channel->fvlist_head) {
			fvlist = list_entry(soc_channel->fvlist_head, struct free_vpu_list, fvlist_head);
			kfree(fvlist);
			fvlist = NULL;
		}
		misc_deregister(&soc_channel->mdev);
		kfree(soc_channel);
		soc_channel = NULL;
	}
}

late_initcall(soc_channel_init);
module_exit(soc_channel_exit);

MODULE_DESCRIPTION("soc vpu device");
MODULE_AUTHOR("ptkang <ptkang@ingenic.cn>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:soc-vpu");
