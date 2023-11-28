/**
 * xb_snd_dsp.c
 *
 * jbbi <jbbi@ingenic.cn>
 *
 * 24 APR 2012
 *
 */
#include <linux/soundcard.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include "xb_snd_dsp.h"
#include <asm/mipsregs.h>
#include <asm/io.h>
//#define DEBUG_REPLAY  0
//#define AIC_DEBUG
//#define DEBUG_RECORD
#ifdef DEBUG_REPLAY
struct file *f_test = NULL;
static loff_t f_test_offset = 0;
static mm_segment_t old_fs;
#endif
#ifdef DEBUG_RECORD
struct file *file_record = NULL;
static loff_t file_record_offset = 0;
static mm_segment_t old_fs_record;
#endif
#ifdef DEBUG_RECORD
#define DEBUG_RECORD_FILE "audiorecord.pcm"
//#define DEBUG_RECORD_FILE "/data/audiorecord.pcm"
#endif
#ifdef DEBUG_REPLAY
//#define DEBUG_REPLAYE_FILE "/data/audio.pcm"
#define DEBUG_REPLAYE_FILE "audio.pcm"
#endif


/*###########################################################*\
 * sub functions
 \*###########################################################*/


static struct dsp_endpoints * xb_dsp_get_endpoints(struct snd_dev_data *ddata)
{
	return ddata->get_endpoints(ddata);
}

/********************************************************\
 * buffer
\********************************************************/

static struct dsp_node* get_free_dsp_node(struct dsp_pipe *dp)
{
	unsigned long lock_flags;
	struct dsp_node *node = NULL;
	struct list_head *phead = &dp->free_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	{
		if (list_empty(phead)) {
			spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
			return NULL;
		}
		node = list_entry(phead->next, struct dsp_node, list);
		node->size = dp->buffersize;
		list_del(phead->next);
	}
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);

	return node;
}

static void put_use_dsp_node(struct dsp_pipe *dp, struct dsp_node *node,int useful_size)
{
	unsigned long lock_flags;
	struct list_head *phead = &dp->use_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	{
		node->start = 0;
		node->end += useful_size;
		list_add_tail(&node->list, phead);
	}
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
}

static struct dsp_node *push_used_node_to_dma(struct dsp_pipe *dp)
{
	unsigned long lock_flags;
	struct dsp_node *node = NULL;
	struct list_head *phead_d = &dp->dma_node_list;
	struct list_head *phead_u = &dp->use_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	if (list_empty(phead_u)) {
		spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
		return NULL;
	}
	node = list_entry(phead_u->next, struct dsp_node, list);
	list_del(phead_u->next);
	list_add_tail(&node->list,phead_d);
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
	return node;
}

static int pop_dma_node_to_free(struct dsp_pipe *dp)
{
	unsigned long lock_flags;
	struct dsp_node *node = NULL;
	struct list_head *phead_d = &dp->dma_node_list;
	struct list_head *phead_f = &dp->free_node_list;
	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	if (list_empty(phead_d)) {
		spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
		return -1;
	}
	node = list_entry(phead_d->next, struct dsp_node, list);
	list_del(phead_d->next);
	node->start = 0;
	node->end = 0;
	list_add_tail(&node->list,phead_f);
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
	return 0;
}

static int pop_back_dma_node_to_use_head(struct dsp_pipe *dp)
{
	unsigned long lock_flags;
	struct dsp_node *node = NULL;
	struct list_head *phead_d = &dp->dma_node_list;
	struct list_head *phead_u = &dp->use_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	if (list_empty(phead_d)) {
		spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
		return -1;
	}
	node = list_entry(phead_d->prev, struct dsp_node, list);
	list_del(phead_d->prev);
	list_add(&node->list,phead_u);
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
	return 0;
}

static struct dsp_node *get_use_dsp_node_info(struct dsp_pipe *dp)
{
	struct list_head *phead = &dp->use_node_list;
	if (list_empty(phead))
		return NULL	;
	return list_entry(phead->next, struct dsp_node, list);
}

static struct dsp_node *get_use_dsp_node(struct dsp_pipe *dp)
{
	unsigned long lock_flags;
	struct dsp_node *node = NULL;
	struct list_head *phead = &dp->use_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	{
		if (list_empty(phead)) {
			spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
			return NULL;
		}
		node = list_entry(phead->next, struct dsp_node, list);
		list_del(phead->next);
	}
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);

	return node;
}

static void put_free_dsp_node(struct dsp_pipe *dp, struct dsp_node *node)
{
	unsigned long lock_flags;
	struct list_head *phead = &dp->free_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	{
		node->start = 0;
		node->end = 0;
		list_add_tail(&node->list, phead);
	}
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);
}

static void put_use_dsp_node_head(struct dsp_pipe *dp,struct dsp_node *node,int used_size)
{
	unsigned long lock_flags;
	struct list_head *phead = &dp->use_node_list;

	spin_lock_irqsave(&dp->pipe_lock,lock_flags);
	{
		node->start += used_size;
		list_add(&node->list,phead);
	}
	spin_unlock_irqrestore(&dp->pipe_lock,lock_flags);

}

static int get_use_dsp_node_count(struct dsp_pipe *dp)
{
	int count = 0;
	unsigned long lock_flags;
	struct list_head *tmp = NULL;
	struct list_head *phead = &dp->use_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	{
		list_for_each(tmp, phead)
			count ++;
	}
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);

	return count;
}

static int get_free_dsp_node_count(struct dsp_pipe *dp)
{
	int count = 0;
	unsigned long lock_flags;
	struct list_head *tmp = NULL;
	struct list_head *phead = &dp->free_node_list;

	spin_lock_irqsave(&dp->pipe_lock, lock_flags);
	{
		list_for_each(tmp, phead)
			count ++;
	}
	spin_unlock_irqrestore(&dp->pipe_lock, lock_flags);

	return count;
}

static int get_unused_dma_node_num(struct dsp_pipe *dp,uint32_t node_base, int *total_count)
{
	unsigned long lock_flags;
	struct list_head *phead_d = &dp->dma_node_list;
	struct dsp_node *pos = NULL;
	int flags = 0;
	int count = 1;
	*total_count = 0;

	spin_lock_irqsave(&dp->pipe_lock,lock_flags);
	list_for_each_entry(pos,phead_d,list) {
		(*total_count)++;
		if (flags == 1)
			count++;
		if (pos->phyaddr == node_base)
			flags = 1;
	}
	spin_unlock_irqrestore(&dp->pipe_lock,lock_flags);
	return count;
}

static int  put_used_dma_node_free(struct dsp_pipe *dp,uint32_t node_base)
{
	unsigned long lock_flags;
	struct list_head *phead_d = &dp->dma_node_list;
	struct list_head *phead_f = &dp->free_node_list;
	struct dsp_node *pos = NULL;
	struct dsp_node *tmp = NULL;
	int i = 0;

	if (list_empty(phead_d)) {
		return i;
	}
	spin_lock_irqsave(&dp->pipe_lock,lock_flags);
	list_for_each_entry(pos,phead_d,list) {
		if (pos->phyaddr == node_base)
			goto delete_program;
	}
	spin_unlock_irqrestore(&dp->pipe_lock,lock_flags);
	return i;

delete_program:
	list_for_each_entry_safe(pos,tmp,phead_d,list) {
		if (pos->phyaddr == node_base)
			break;
		list_del(&pos->list);
		pos->start = 0;
		pos->end = 0;
		list_add_tail(&pos->list, phead_f);
		atomic_inc(&dp->avialable_couter);
		i++;
	}
	spin_unlock_irqrestore(&dp->pipe_lock,lock_flags);
	return i;
}

/********************************************************\
 * dma
\********************************************************/

static void snd_dma_callback(void *arg);

static void snd_reconfig_dma(struct dsp_pipe *dp)
{
	if (dp->dma_chan == NULL)
		return;

	dmaengine_slave_config(dp->dma_chan,&dp->dma_config);
}
static bool dma_chan_filter(struct dma_chan *chan, void *filter_param)
{
	struct dsp_pipe *dp = filter_param;
	return (void*)dp->dma_type == chan->private;
}

static int snd_reuqest_dma(struct dsp_pipe *dp)
{
	dma_cap_mask_t mask;

	/* Try to grab a DMA channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);
	dp->force_stop_dma = false;
	dp->dma_chan = dma_request_channel(mask, dma_chan_filter, (void*)dp);

	if (dp->dma_chan == NULL) {
		return -ENXIO;
	}
	snd_reconfig_dma(dp);

	/* alloc one scatterlist */
	dp->sg_len = 0;

	dp->sg = vzalloc(sizeof(struct scatterlist) * dp->fragcnt+1);
	if (!dp->sg) {
		return -ENOMEM;
	}

	return 0;
}

#if 0
static void snd_dmic_release_dma(struct dsp_pipe *dp)
{
	int ret = 0x7ffff;

	if (dp) {
		if (dp->dma_chan) {
			if (dp->is_trans != false) {

#ifndef CONFIG_ANDROID         //kkshen added
				{

					if (dp->dma_config.direction == DMA_TO_DEVICE)
						while(ret-- && atomic_read(&dp->avialable_couter) == 0);
					if (ret == 0) {
						printk(KERN_INFO "sound dma transfer data error");
					}

					ret = 1;
				}
#endif
				dp->wait_stop_dma = true;
				while(ret-- && ((*(volatile unsigned int *)0xb0021000 & 0xc0) != 0xc));
				del_timer(&dp->transfer_watchdog);
				dmaengine_terminate_all(dp->dma_chan);
				dp->is_trans = false;
				dp->wait_stop_dma = false;
			}
			dma_release_channel(dp->dma_chan);
			dp->dma_chan = NULL;
		}
		if (dp->sg) {
			vfree(dp->sg);
			dp->sg = NULL;
		}
	}
}
#endif
static void snd_release_dma(struct dsp_pipe *dp)
{
	int ret = 0xf;

	if (dp) {
		dp->force_stop_dma = false;
		if (dp->dma_chan) {
			if (dp->is_trans != false) {
#ifndef CONFIG_ANDROID         //kkshen added
				{

					if (dp->dma_config.direction == DMA_TO_DEVICE)
						while(ret-- && atomic_read(&dp->avialable_couter) == 0);
					if (ret == 0) {
						printk(KERN_INFO "sound dma transfer data error");
					}

					ret = 1;
				}
#endif
				dp->wait_stop_dma = true;
				while(ret-- && wait_event_interruptible(dp->wq,dp->is_trans == false));
				del_timer(&dp->transfer_watchdog);
				dmaengine_terminate_all(dp->dma_chan);
				dp->is_trans = false;
				dp->wait_stop_dma = false;
			}
			dma_release_channel(dp->dma_chan);
			dp->dma_chan = NULL;
		}
		if (dp->sg) {
			vfree(dp->sg);
			dp->sg = NULL;
		}
	}
}

static void snd_release_node(struct dsp_pipe *dp)
{
	struct dsp_node *node = NULL;
	int free_count = 0;

	if (dp == NULL) {
		printk("%s dp is null.\n",__func__);
		return;
	}

	while(1) {
		node = NULL;
		node = get_use_dsp_node(dp);
		if (!node)
			break;
		put_free_dsp_node(dp, node);
	};

	if (dp->save_node != NULL) {
		put_free_dsp_node(dp, dp->save_node);
		dp->save_node = NULL;
	} else
		while(!pop_dma_node_to_free(dp));

	free_count = get_free_dsp_node_count(dp);
	printk("free_count: %d\n", free_count);
	if (free_count < dp->fragcnt)
		printk("%s error buffer is lost when snd_release_node.\n",__func__);
	else if (free_count > dp->fragcnt) {
		printk("%s BUG: error buffer release fail.\n",__func__);
		free_count = dp->fragcnt;
	}
	if (dp->dma_config.direction == DMA_DEV_TO_MEM)
		atomic_set(&dp->avialable_couter,0);
	else
		atomic_set(&dp->avialable_couter,free_count);

}

static int snd_prepare_dma_desc(struct dsp_pipe *dp)
{
	struct dsp_node *node = NULL;
	struct dma_async_tx_descriptor *desc = NULL;

	/* turn the dsp_node to dma desc */
	if (dp->dma_config.direction == DMA_MEM_TO_DEV) {
		dp->sg_len = 0;
		while ((dp->sg_len != dp->fragcnt-1)) {
			if ((node = push_used_node_to_dma(dp)) == NULL)
				break;
			sg_dma_address(&dp->sg[dp->sg_len]) = node->phyaddr;
			sg_dma_len(&dp->sg[dp->sg_len]) = node->end - node->start;
			dp->sg_len++;
		}
		if (!dp->sg_len) {
			printk(KERN_DEBUG"AUDIO:user space write too slow\n");
			return -ENOMEM;
		}

		desc = dp->dma_chan->device->device_prep_slave_sg(dp->dma_chan,
								  dp->sg,
								  dp->sg_len,
								  dp->dma_config.direction,
								  DMA_PREP_INTERRUPT | DMA_CTRL_ACK,
								  NULL);
		if (!desc) {
			while (dp->sg_len-- || !pop_back_dma_node_to_use_head(dp));
			return -EFAULT;
		}
	} else if (dp->dma_config.direction == DMA_DEV_TO_MEM) {
		node = get_free_dsp_node(dp);
		if (!node) {
			printk("free: %d, user: %d\n", get_free_dsp_node_count(dp), get_use_dsp_node_count(dp));
			return -ENOMEM;
		}

		dp->save_node = node;

		/* config sg */
		sg_dma_address(dp->sg) = node->phyaddr;
		sg_dma_len(dp->sg) = node->size;
		dp->sg_len = 1;

		desc = dp->dma_chan->device->device_prep_slave_sg(dp->dma_chan,
								  dp->sg,
								  dp->sg_len,
								  dp->dma_config.direction,
								  DMA_PREP_INTERRUPT | DMA_CTRL_ACK,
								  NULL);
		if (!desc) {
			put_free_dsp_node(dp, node);
			dp->save_node = NULL;
			printk("device_prep_slave_sg failed %d\n", __LINE__);
			return -EFAULT;
		}
	} else {
		return -EFAULT;
	}

	/* set desc callback */
	desc->callback = snd_dma_callback;
	desc->callback_param = (void *)dp;

	/* tx_submit */
	dmaengine_submit(desc);

	return 0;
}

static void snd_start_dma_transfer(struct dsp_pipe *dp ,
				   enum dma_transfer_direction direction)
{
	dp->is_trans = true;

	dma_async_issue_pending(dp->dma_chan);
	if (direction == DMA_MEM_TO_DEV && dp->sg_len >= 3 && dp->mod_timer &&
	    dp->samplerate == 44100) {
		mod_timer(&dp->transfer_watchdog,
			  jiffies+msecs_to_jiffies(10));
		atomic_set(&dp->watchdog_avail,0);
	}
}

static dma_addr_t inline  dma_trans_addr(struct dsp_pipe *dp,
					 enum dma_data_direction direction)
{
	return dp->dma_chan->device->get_current_trans_addr(dp->dma_chan,
							    NULL,
							    NULL,
							    direction);
}

#if 0
static int timer_count = 0;
#endif

static void replay_watch_function(unsigned long _dp)
{
	struct dsp_pipe *dp = (struct dsp_pipe *)_dp;
	dma_addr_t pdma_addr = 0;
	struct dma_async_tx_descriptor *desc = NULL;
	struct dsp_node *node = NULL;
	int total_count = 0;
	uint32_t node_base = 0;
	uint32_t node_offset = 0;
	int delay = 0;
	int success = 0;
	int unused_num = 0;

	if (dp->wait_stop_dma == true || atomic_read(&dp->watchdog_avail) ||
	    dp->samplerate != 44100 /*FIXME NOW we just use 44100,other rate must be test*/)
		return;

	node = get_use_dsp_node_info(dp);
	if (node) {
		desc = dp->dma_chan->device->device_add_desc(dp->dma_chan,
							     node->phyaddr,
							     dp->dma_config.dst_addr,
							     node->end - node->start,
							     dp->dma_config.direction,
							     0x3 | ((dp->fragsize -1)) << 2);
		if (!desc) {
			if (!push_used_node_to_dma(dp)) {
				printk("UNHAPPEND\n");
				return;	/*there will not be happen*/
			} else {
#if 0
				timer_count++;
				if (!(timer_count%100))
					printk(KERN_DEBUG"timer_count = %d\n",timer_count);
#endif
				success = 1;
			}
		} else {
			success = -1;
		}
	}
	pdma_addr = dma_trans_addr(dp,dp->dma_config.direction);
	node_offset = ((unsigned long)pdma_addr & (dp->fragsize - 1));
	node_base = ((unsigned long)pdma_addr & (~(dp->fragsize - 1)));
	delay = (dp->buffersize - node_offset -1) * 500/(dp->channels * dp->samplerate);
	//FIXME node->end - node->start instead dp->buffersize
	if (success <= 0) {
		int delay_time = 0;
		unused_num = get_unused_dma_node_num(dp,node_base,&total_count);
		if ((delay > 15 && unused_num < 2) ||(delay <=15 && unused_num < 3)) {
			delay_time = unused_num*20;
			if ((delay + delay_time) < 3) {
				goto wait_interrupt_clean;
			}
			goto wait_interrupt;
		}

		if (success) {
			if (dp->fragcnt - unused_num - 1 > 2) {
				goto wait_interrupt;
			}
		}
	}
	mod_timer(&dp->transfer_watchdog,jiffies+msecs_to_jiffies(10));		//10ms timer
wait_interrupt:
	if (put_used_dma_node_free(dp,node_base))
		if (dp->is_non_block == false)
			wake_up_interruptible(&dp->wq);
wait_interrupt_clean:
	return;
}

static void record_watch_function(unsigned long _dp)
{
	return;
}

#if 0
static int dma_count = 0;
#endif

static void snd_dma_callback(void *arg)
{
	struct dsp_pipe *dp = (struct dsp_pipe *)arg;
	int available_node = 0;
	int ret = 0;
	atomic_set(&dp->watchdog_avail,1);


	if (dp->dma_config.direction == DMA_MEM_TO_DEV) {
		while (!pop_dma_node_to_free(dp)) {
			atomic_inc(&dp->avialable_couter);
			available_node++;
		}
#if 0
		printk(KERN_DEBUG"dma free available_node = %d\n",available_node);
		if (!((++dma_count)%10)) {
			printk("dma_count %d timer_count %d\n",dma_count,timer_count);
		}
#endif
	} else if (dp->dma_config.direction == DMA_DEV_TO_MEM) {
		put_use_dsp_node(dp, dp->save_node,dp->save_node->size);
		atomic_inc(&dp->avialable_couter);
		available_node++;
	}

	if (dp->is_non_block == false && available_node)
		wake_up_interruptible(&dp->wq);
	dp->save_node =	NULL;

	/* if device closed, release the dma */
	if (dp->wait_stop_dma == true) {
		if (dp->dma_chan) {
			dmaengine_terminate_all(dp->dma_chan);
			dp->is_trans = false;
		}
		dp->wait_stop_dma = false;
		wake_up(&dp->wq);
		return;
	}

	if (dp->need_reconfig_dma == true) {
		snd_reconfig_dma(dp);
		dp->need_reconfig_dma = false;
	}



	if (!(ret = snd_prepare_dma_desc(dp))) {
		snd_start_dma_transfer(dp ,dp->dma_config.direction);
	} else {
		dp->is_trans = false;
		if (-ENOMEM == ret) {
			printk("prepare-buffer underrun\n");
		} else {
			printk("prep failed ret%d\n", ret);
		}
		if (dp->pddata && dp->pddata->dev_ioctl) {
			if (dp->dma_config.direction == DMA_DEV_TO_MEM) {
				dp->pddata->dev_ioctl(SND_DSP_DISABLE_DMA_RX,0);
			}
			else if (dp->dma_config.direction == DMA_MEM_TO_DEV) {
				dp->pddata->dev_ioctl(SND_DSP_DISABLE_DMA_TX,0);
			}
		} else if(dp->pddata && dp->pddata->dev_ioctl_2) {
			if (dp->dma_config.direction == DMA_DEV_TO_MEM) {
				dp->pddata->dev_ioctl_2(dp->pddata, SND_DSP_DISABLE_DMA_RX,0);
			}
			else if (dp->dma_config.direction == DMA_MEM_TO_DEV) {
				dp->pddata->dev_ioctl_2(dp->pddata, SND_DSP_DISABLE_DMA_TX,0);
			}
		}

	}

	return;
}

/********************************************************\
 * filter
\********************************************************/
/*
 * Convert signed byte to unsiged byte
 *
 * Mapping:
 * 	signed		unsigned
 *	0x00 (0)	0x80 (128)
 *	0x01 (1)	0x81 (129)
 *	......		......
 *	0x7f (127)	0xff (255)
 *	0x80 (-128)	0x00 (0)
 *	0x81 (-127)	0x01 (1)
 *	......		......
 *	0xff (-1)	0x7f (127)
 */
int convert_8bits_signed2unsigned(void *buffer, int *counter,int needed_size)
{
	int i;
	int counter_8align = 0;
	unsigned char *ucsrc	= buffer;
	unsigned char *ucdst	= buffer;

	if (needed_size < (*counter)) {
		*counter = needed_size;
	}
	counter_8align = (*counter) & ~0x7;

	for (i = 0; i < counter_8align; i+=8) {
		*(ucdst + i + 0) = *(ucsrc + i + 0) + 0x80;
		*(ucdst + i + 1) = *(ucsrc + i + 1) + 0x80;
		*(ucdst + i + 2) = *(ucsrc + i + 2) + 0x80;
		*(ucdst + i + 3) = *(ucsrc + i + 3) + 0x80;
		*(ucdst + i + 4) = *(ucsrc + i + 4) + 0x80;
		*(ucdst + i + 5) = *(ucsrc + i + 5) + 0x80;
		*(ucdst + i + 6) = *(ucsrc + i + 6) + 0x80;
		*(ucdst + i + 7) = *(ucsrc + i + 7) + 0x80;
	}

	BUG_ON(i != counter_8align);

	for (i = counter_8align; i < *counter; i++) {
		*(ucdst + i) = *(ucsrc + i) + 0x80;
	}

	return *counter;
}

/*
 * Convert stereo data to mono data, data width: 8 bits/channel
 *
 * buff:	buffer address
 * data_len:	data length in kernel space, the length of stereo data
 *		calculated by "node->end - node->start"
 */
int convert_8bits_stereo2mono(void *buff, int *data_len,int needed_size)
{
	/* stride = 16 bytes = 2 channels * 1 byte * 8 pipelines */
	int data_len_16aligned = 0;
	int mono_cur, stereo_cur;
	unsigned char *uc_buff = buff;

	if ((*data_len) > needed_size*2)
		*data_len = needed_size*2;

	*data_len = (*data_len) & (~0x1);

	data_len_16aligned = (*data_len)& ~0xf;

	/* copy 8 times each loop */
	for (stereo_cur = mono_cur = 0;
	     stereo_cur < data_len_16aligned;
	     stereo_cur += 16, mono_cur += 8) {

		uc_buff[mono_cur + 0] = uc_buff[stereo_cur + 0];
		uc_buff[mono_cur + 1] = uc_buff[stereo_cur + 2];
		uc_buff[mono_cur + 2] = uc_buff[stereo_cur + 4];
		uc_buff[mono_cur + 3] = uc_buff[stereo_cur + 6];
		uc_buff[mono_cur + 4] = uc_buff[stereo_cur + 8];
		uc_buff[mono_cur + 5] = uc_buff[stereo_cur + 10];
		uc_buff[mono_cur + 6] = uc_buff[stereo_cur + 12];
		uc_buff[mono_cur + 7] = uc_buff[stereo_cur + 14];
	}

	BUG_ON(stereo_cur != data_len_16aligned);

	/* remaining data */
	for (; stereo_cur < (*data_len); stereo_cur += 2, mono_cur++) {
		uc_buff[mono_cur] = uc_buff[stereo_cur];
	}

	return ((*data_len) >> 1);
}

/*
 * Convert stereo data to mono data, and convert signed byte to unsigned byte.
 *
 * data width: 8 bits/channel
 *
 * buff:	buffer address
 * data_len:	data length in kernel space, the length of stereo data
 *		calculated by "node->end - node->start"
 */
int convert_8bits_stereo2mono_signed2unsigned(void *buff, int *data_len,int needed_size)
{
	/* stride = 16 bytes = 2 channels * 1 byte * 8 pipelines */
	int data_len_16aligned = 0;
	int mono_cur, stereo_cur;
	unsigned char *uc_buff = buff;

	if ((*data_len) > needed_size*2)
		*data_len = needed_size*2;

	*data_len = (*data_len) & (~0x1);

	data_len_16aligned = (*data_len) & ~0xf;

	/* copy 8 times each loop */
	for (stereo_cur = mono_cur = 0;
	     stereo_cur < data_len_16aligned;
	     stereo_cur += 16, mono_cur += 8) {

		uc_buff[mono_cur + 0] = uc_buff[stereo_cur + 0] + 0x80;
		uc_buff[mono_cur + 1] = uc_buff[stereo_cur + 2] + 0x80;
		uc_buff[mono_cur + 2] = uc_buff[stereo_cur + 4] + 0x80;
		uc_buff[mono_cur + 3] = uc_buff[stereo_cur + 6] + 0x80;
		uc_buff[mono_cur + 4] = uc_buff[stereo_cur + 8] + 0x80;
		uc_buff[mono_cur + 5] = uc_buff[stereo_cur + 10] + 0x80;
		uc_buff[mono_cur + 6] = uc_buff[stereo_cur + 12] + 0x80;
		uc_buff[mono_cur + 7] = uc_buff[stereo_cur + 14] + 0x80;
	}

	BUG_ON(stereo_cur != data_len_16aligned);

	/* remaining data */
	for (; stereo_cur < (*data_len); stereo_cur += 2, mono_cur++) {
		uc_buff[mono_cur] = uc_buff[stereo_cur] + 0x80;
	}

	return ((*data_len) >> 1);
}

/*
 * Convert stereo data to mono data, data width: 16 bits/channel
 *
 * buff:	buffer address
 * data_len:	data length in kernel space, the length of stereo data
 *		calculated by "node->end - node->start"
 */
int convert_16bits_stereo2mono(void *buff, int *data_len, int needed_size)
{
	/* stride = 32 bytes = 2 channels * 2 byte * 8 pipelines */
	int data_len_32aligned = 0;
	int data_cnt_ushort = 0;
	int mono_cur, stereo_cur;
	unsigned short *ushort_buff = (unsigned short *)buff;

	if ((*data_len) > needed_size*2)
		*data_len = needed_size*2;

	/*when 16bit format one sample has four bytes
	 *so we can not operat the singular byte*/
	*data_len = (*data_len) & (~0x3);

	data_len_32aligned = (*data_len) & ~0x1f;
	data_cnt_ushort = data_len_32aligned >> 1;

	/* copy 8 times each loop */
	for (stereo_cur = mono_cur = 0;
	     stereo_cur < data_cnt_ushort;
	     stereo_cur += 16, mono_cur += 8) {

		ushort_buff[mono_cur + 0] = ushort_buff[stereo_cur + 0];
		ushort_buff[mono_cur + 1] = ushort_buff[stereo_cur + 2];
		ushort_buff[mono_cur + 2] = ushort_buff[stereo_cur + 4];
		ushort_buff[mono_cur + 3] = ushort_buff[stereo_cur + 6];
		ushort_buff[mono_cur + 4] = ushort_buff[stereo_cur + 8];
		ushort_buff[mono_cur + 5] = ushort_buff[stereo_cur + 10];
		ushort_buff[mono_cur + 6] = ushort_buff[stereo_cur + 12];
		ushort_buff[mono_cur + 7] = ushort_buff[stereo_cur + 14];
	}

	BUG_ON(stereo_cur != data_cnt_ushort);

	/* remaining data */
	for (; stereo_cur < ((*data_len) >> 1); stereo_cur += 2, mono_cur++) {
		ushort_buff[mono_cur] = ushort_buff[stereo_cur];
	}

	return ((*data_len) >> 1);
}

int convert_16bits_stereo2mono_inno(void *buff, int *data_len, int needed_size)
{
	/* stride = 32 bytes = 2 channels * 2 byte * 8 pipelines */
	int data_len_32aligned = 0;
	int data_cnt_ushort = 0;
	int mono_cur, stereo_cur;
	unsigned short *ushort_buff = (unsigned short *)buff;

	if ((*data_len) > needed_size*2)
		*data_len = needed_size*2;

	/*when 16bit format one sample has four bytes
	 *so we can not operat the singular byte*/
	*data_len = (*data_len) & (~0x3);

	data_len_32aligned = (*data_len) & ~0x1f;
	data_cnt_ushort = data_len_32aligned >> 1;

	/* copy 8 times each loop */
	for (stereo_cur = mono_cur = 0;
	     stereo_cur < data_cnt_ushort;
	     stereo_cur += 16, mono_cur += 8) {

		ushort_buff[mono_cur + 0] = ushort_buff[stereo_cur + 0] + ushort_buff[stereo_cur + 1];
		ushort_buff[mono_cur + 1] = ushort_buff[stereo_cur + 2] + ushort_buff[stereo_cur + 3];
		ushort_buff[mono_cur + 2] = ushort_buff[stereo_cur + 4] + ushort_buff[stereo_cur + 5];
		ushort_buff[mono_cur + 3] = ushort_buff[stereo_cur + 6] + ushort_buff[stereo_cur + 7];
		ushort_buff[mono_cur + 4] = ushort_buff[stereo_cur + 8] + ushort_buff[stereo_cur + 9];
		ushort_buff[mono_cur + 5] = ushort_buff[stereo_cur + 10] + ushort_buff[stereo_cur + 11];
		ushort_buff[mono_cur + 6] = ushort_buff[stereo_cur + 12] + ushort_buff[stereo_cur + 13];
		ushort_buff[mono_cur + 7] = ushort_buff[stereo_cur + 14] + ushort_buff[stereo_cur + 15];
	}

	BUG_ON(stereo_cur != data_cnt_ushort);

	/* remaining data */
	for (; stereo_cur < ((*data_len) >> 1); stereo_cur += 2, mono_cur++) {
		ushort_buff[mono_cur] = ushort_buff[stereo_cur] + ushort_buff[stereo_cur +1];
	}

	return ((*data_len) >> 1);
}

/*
 * convert normal 16bit stereo data to mono data
 *
 * buff:	buffer address
 * data_len:	data length in kernel space, the length of stereo data
 *
 */
int convert_16bits_stereomix2mono(void *buff, int *data_len,int needed_size)
{
	/* stride = 32 bytes = 2 channels * 2 byte * 8 pipelines */
	int data_len_32aligned = 0;
	int data_cnt_ushort = 0;
	int left_cur, right_cur, mono_cur;
	short *ushort_buff = (short *)buff;
	/*init*/
	left_cur = 0;
	right_cur = left_cur + 1;
	mono_cur = 0;


	if ( (*data_len) > needed_size*2)
		*data_len = needed_size*2;

	/*when 16bit format one sample has four bytes
	 *so we can not operat the singular byte*/
	*data_len = (*data_len) & (~0x3);

	data_len_32aligned = (*data_len) & (~0x1f);
	data_cnt_ushort = data_len_32aligned >> 1;

	/*because the buff's size is always 4096 bytes,so it will not lost data*/
	while (left_cur < data_cnt_ushort)
	{
		ushort_buff[mono_cur + 0] = ((ushort_buff[left_cur + 0]) + (ushort_buff[right_cur + 0]));
		ushort_buff[mono_cur + 1] = ((ushort_buff[left_cur + 2]) + (ushort_buff[right_cur + 2]));
		ushort_buff[mono_cur + 2] = ((ushort_buff[left_cur + 4]) + (ushort_buff[right_cur + 4]));
		ushort_buff[mono_cur + 3] = ((ushort_buff[left_cur + 6]) + (ushort_buff[right_cur + 6]));
		ushort_buff[mono_cur + 4] = ((ushort_buff[left_cur + 8]) + (ushort_buff[right_cur + 8]));
		ushort_buff[mono_cur + 5] = ((ushort_buff[left_cur + 10]) + (ushort_buff[right_cur + 10]));
		ushort_buff[mono_cur + 6] = ((ushort_buff[left_cur + 12]) + (ushort_buff[right_cur + 12]));
		ushort_buff[mono_cur + 7] = ((ushort_buff[left_cur + 14]) + (ushort_buff[right_cur + 14]));

		left_cur += 16;
		right_cur += 16;
		mono_cur += 8;
	}

	BUG_ON(left_cur != data_cnt_ushort);

	/* remaining data */
	for (;right_cur < ((*data_len) >> 1); left_cur += 2, right_cur += 2)
		ushort_buff[mono_cur++] = ushort_buff[left_cur] + ushort_buff[right_cur];

	return ((*data_len) >> 1);
}

/********************************************************\
 * others
\********************************************************/
static int init_pipe(struct dsp_pipe *dp,struct device *dev,enum dma_data_direction direction)
{
	int i = 0;
	struct dsp_node *node;

	if (dp == NULL)
		return -ENODEV;

	if ((dp->fragsize != FRAGSIZE_S) &&
	    (dp->fragsize != FRAGSIZE_M) &&
	    (dp->fragsize != FRAGSIZE_L))
	{
		return -1;
	}

	if ((dp->fragcnt != FRAGCNT_S) &&
	    (dp->fragcnt != FRAGCNT_M) &&
	    (dp->fragcnt != FRAGCNT_B) &&
	    (dp->fragcnt != FRAGCNT_L))
	{
		return -1;
	}

	INIT_LIST_HEAD(&(dp->free_node_list));
	INIT_LIST_HEAD(&(dp->dma_node_list));
	INIT_LIST_HEAD(&(dp->use_node_list));

	/* alloc memory */

	dp->vaddr = (unsigned long)dmam_alloc_noncoherent(dev,
							  PAGE_ALIGN(dp->fragsize * dp->fragcnt),
							  &dp->paddr,
							  GFP_KERNEL | GFP_DMA);
	if ((void*)dp->vaddr == NULL)
		return -ENOMEM;

	/* init dsp nodes */
	for (i = 0; i < dp->fragcnt; i++) {
//		node = vmalloc(sizeof(struct dsp_node));
		node = kmalloc(sizeof(struct dsp_node),GFP_KERNEL);
		if (!node)
			goto init_pipe_error;

		node->node_number = i;
		node->pBuf = dp->vaddr + dp->fragsize * i;
		node->phyaddr = dp->paddr + dp->fragsize * i;
		node->start = 0;
		node->end = 0;
		node->size = dp->fragsize;
		list_add_tail(&node->list, &dp->free_node_list);
	}

	/* init others */
	dp->dma_chan = NULL;
	dp->save_node = NULL;
	init_waitqueue_head(&dp->wq);
	dp->is_trans = false;
	dp->is_used = false;
	dp->is_mmapd = false;
	dp->wait_stop_dma = false;
	dp->force_stop_dma = false;
	dp->need_reconfig_dma = false;
	dp->is_shared = false;
	dp->is_non_block = false;
	dp->sg = NULL;
	dp->sg_len = 0;
	dp->filter = NULL;
	dp->buffersize = dp->fragsize;
	dp->mod_timer = 1;
	dp->can_mmap = false;
	dp->handle = NULL;

	init_timer(&dp->transfer_watchdog);
	if (direction == DMA_TO_DEVICE) {
		dp->dma_config.direction = DMA_MEM_TO_DEV;
		atomic_set(&dp->avialable_couter,dp->fragcnt);
		dp->watchdog_mdelay = 10;
		dp->transfer_watchdog.function = replay_watch_function;
		dp->samplerate = DEFAULT_REPLAY_SAMPLERATE;
		dp->channels = DEFAULT_REPLAY_CHANNEL;
	} else if (direction == DMA_FROM_DEVICE) {
		dp->dma_config.direction = DMA_DEV_TO_MEM;
		atomic_set(&dp->avialable_couter,0);
		dp->watchdog_mdelay = 128;
		dp->transfer_watchdog.function = record_watch_function;
		dp->samplerate = DEFAULT_RECORD_SAMPLERATE;
		dp->channels = DEFAULT_RECORD_CHANNEL;
	}
	atomic_set(&dp->watchdog_avail,1);
	dp->transfer_watchdog.data = (unsigned long)dp;
	spin_lock_init(&dp->pipe_lock);
	mutex_init(&dp->mutex);
	return 0;
init_pipe_error:
	/* free all the node in free_node_list */
	list_for_each_entry(node, &dp->free_node_list, list)
		kfree(node);
	/* free memory */
	dmam_free_noncoherent(dev,
			      dp->fragsize * dp->fragcnt,
			      (void*)dp->vaddr,
			      dp->paddr);
	return -1;
}

static void deinit_pipe(struct dsp_pipe *dp,struct device *dev)
{
	struct dsp_node *node;

	/* free all the node in free_node_list */
	list_for_each_entry(node, &dp->free_node_list, list)
		kfree(node);
	mutex_destroy(&dp->mutex);
	/* free memory */
	dmam_free_noncoherent(dev,
			      dp->fragsize * dp->fragcnt,
			      (void*)dp->vaddr,
			      dp->paddr);
}

static int mmap_pipe(struct dsp_pipe *dp, struct vm_area_struct *vma)
{
	unsigned long start = 0;
	unsigned long off = 0;
	unsigned long len = 0;

	off = vma->vm_pgoff << PAGE_SHIFT;
	start = dp->paddr;
	len = PAGE_ALIGN(dp->fragcnt * dp->fragsize);
	start &= PAGE_MASK;
	if ((vma->vm_end - vma->vm_start + off) > len)
		return -EINVAL;
	off += start;

	vma->vm_pgoff = off >> PAGE_SHIFT;
	vma->vm_flags |= VM_IO;

	pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
	pgprot_val(vma->vm_page_prot) |= _CACHE_CACHABLE_NONCOHERENT;

	if (io_remap_pfn_range(vma, vma->vm_start, off >> PAGE_SHIFT,
			       vma->vm_end - vma->vm_start,
			       vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

/*###########################################################*\
 * interfacees
 \*###########################################################*/
/********************************************************\
 * llseek
 \********************************************************/
loff_t xb_snd_dsp_llseek(struct file *file,
			 loff_t offset,
			 int origin,
			 struct snd_dev_data *ddata)
{
	return 0;
}

/********************************************************\
 * read
 \********************************************************/
ssize_t xb_snd_dsp_read(struct file *file,
			char __user *buffer,
			size_t count,
			loff_t *ppos,
			struct snd_dev_data *ddata)
{
	int	mcount = count;
	int ret = -EINVAL;
	size_t fixed_buff_cnt = 0;
	size_t node_buff_cnt = 0;
	struct dsp_node *node = NULL;
	struct dsp_pipe *dp = NULL;
	struct dsp_endpoints *endpoints = NULL;
	ENTER_FUNC();
	if (!(file->f_mode & FMODE_READ))
		return -EPERM;

	if (ddata == NULL)
		return -ENODEV;
	if(ddata->priv_data) {
		endpoints = xb_dsp_get_endpoints(ddata);
	} else {
		endpoints = (struct dsp_endpoints *)ddata->ext_data;
	}
	if (endpoints == NULL)
		return -ENODEV;

	dp = endpoints->in_endpoint;

	if (dp == NULL)
		return -ENODEV;

	if (dp->is_mmapd && dp->is_shared)
		return -EBUSY;

	mutex_lock(&dp->mutex);
	do {
		while(1) {
			if (dp->force_stop_dma) {
				dp->force_stop_dma = false;
				mutex_unlock(&dp->mutex);
				return count - mcount;
			}
			node = get_use_dsp_node(dp);
			if (dp->is_trans == false) {
				ret = snd_prepare_dma_desc(dp);
				if (!ret) {
					snd_start_dma_transfer(dp , dp->dma_config.direction);
					if (ddata && ddata->dev_ioctl) {
						ddata->dev_ioctl(SND_DSP_ENABLE_DMA_RX, 0);
					} else if(ddata && ddata->dev_ioctl_2) {
						ddata->dev_ioctl_2(ddata, SND_DSP_ENABLE_DMA_RX, 0);
						/* wait until all devices settings done, then we can get data */
						ddata->dev_ioctl_2(ddata, SND_DSP_FLUSH_SYNC, 0);
					}
				} else if (!node) {
					mutex_unlock(&dp->mutex);
					return -EFAULT;
				}
			}

			if (!node && dp->is_non_block) {
				mutex_unlock(&dp->mutex);
				return count - mcount;
			}

			if (!node) {
				mutex_unlock(&dp->mutex);
				if (wait_event_interruptible(dp->wq,
							     (atomic_read(&dp->avialable_couter) >= 1 || dp->force_stop_dma == true)) < 0) {
					return count - mcount;
				}
				mutex_lock(&dp->mutex);
			} else {
				atomic_dec(&dp->avialable_couter);
				break;
			}
		}

		if ((node_buff_cnt = node->end - node->start) > 0) {

			if (dp->filter) {
				fixed_buff_cnt = dp->filter((void *)(node->pBuf + node->start), &node_buff_cnt ,mcount);
			} else {
				if (mcount < node_buff_cnt) {
					node_buff_cnt = mcount;
				}
				fixed_buff_cnt = node_buff_cnt;
			}
			if (copy_to_user((void *)buffer,
					 (void *)(node->pBuf + node->start), fixed_buff_cnt)) {
				put_use_dsp_node_head(dp,node,0);
				atomic_inc(&dp->avialable_couter);
				mutex_unlock(&dp->mutex);
				return -EFAULT;
			}


			buffer += fixed_buff_cnt;
			mcount -= fixed_buff_cnt;

#ifdef DEBUG_RECORD
			{
				old_fs_record = get_fs();
				set_fs(KERNEL_DS);

				if (!IS_ERR(file_record)) {
					vfs_write(file_record,
						  (void *)(node->pBuf + node->start),
						  fixed_buff_cnt,
						  &file_record_offset);
					file_record_offset = file_record->f_pos;
				}
				set_fs(old_fs_record);
			}
#endif
			if (node_buff_cnt < node->end - node->start) {
				put_use_dsp_node_head(dp,node,node_buff_cnt);
				atomic_inc(&dp->avialable_couter);
			} else
				put_free_dsp_node(dp,node);

			dma_cache_sync(NULL, (void *)node->pBuf, node->size, dp->dma_config.direction);
		}
	} while (mcount > 0);

	LEAVE_FUNC();
	mutex_unlock(&dp->mutex);
	return count - mcount;
}

/********************************************************\
 * write
 \********************************************************/
ssize_t xb_snd_dsp_write(struct file *file,
			 const char __user *buffer,
			 size_t count,
			 loff_t *ppos,
			 struct snd_dev_data *ddata)
{
	int mcount = count;
	int copy_size = 0;
	int ret = 0;
	struct dsp_node *node = NULL;
	struct dsp_pipe *dp = NULL;
	struct dsp_endpoints *endpoints = NULL;

	if (!(file->f_mode & FMODE_WRITE))
		return -EPERM;

	if (ddata == NULL)
		return -ENODEV;

	if(ddata->priv_data) {
		endpoints = xb_dsp_get_endpoints(ddata);
	} else {
		endpoints = (struct dsp_endpoints *)ddata->ext_data;
	}
	if (endpoints == NULL)
		return -ENODEV;

	dp = endpoints->out_endpoint;

	if (dp == NULL)
		return -ENODEV;

	if (dp->is_mmapd && dp->is_shared)
		return -EBUSY;

	mutex_lock(&dp->mutex);
	while (mcount > 0) {
		while (1) {
			if (dp->force_stop_dma) {
				dp->force_stop_dma = false;
				goto write_return;
			}
			node = get_free_dsp_node(dp);
			if (!node) {
				if (dp->is_trans == false) {
					int free_count = 0;
					while (!pop_dma_node_to_free(dp)) ;

					while(1) {
						node = get_use_dsp_node(dp);
						if (!node)
							break;
						put_free_dsp_node(dp, node);
					};
					free_count = get_free_dsp_node_count(dp);
					atomic_set(&dp->avialable_couter,free_count);
					printk("Dma write error release node count %d\n",
					       free_count);
				}
				if (dp->is_non_block == true)
					goto write_return;
				mutex_unlock(&dp->mutex);
				if (wait_event_interruptible(dp->wq, (atomic_read(&dp->avialable_couter) >= 1)
							     || dp->force_stop_dma == true) < 0) {
					return count - mcount;
				}
				mutex_lock(&dp->mutex);
			} else {
				atomic_dec(&dp->avialable_couter);
				break;
			}
		}

		if (mcount >= node->size)
			copy_size = node->size;
		else
			copy_size = mcount;

		if (copy_from_user((void *)node->pBuf + node->start, buffer, copy_size)) {
			put_free_dsp_node(dp,node);
			atomic_inc(&dp->avialable_couter);
			ret = -EFAULT;
			goto write_return;
		}

#ifdef DEBUG_REPLAY
		old_fs = get_fs();
		set_fs(KERNEL_DS);

		if (!IS_ERR(f_test)) {
			vfs_write(f_test, (void*)node->pBuf ,count, &f_test_offset);
			f_test_offset = f_test->f_pos;
		}
		set_fs(old_fs);

#endif
		buffer += copy_size;
		mcount -= copy_size;

		dma_cache_sync(NULL, (void *)node->pBuf, copy_size , dp->dma_config.direction);
		put_use_dsp_node(dp, node, copy_size);


		if (dp->is_trans == false) {
			if (!snd_prepare_dma_desc(dp)) {
				snd_start_dma_transfer(dp , dp->dma_config.direction);
				if (ddata && ddata->dev_ioctl) {
					ddata->dev_ioctl(SND_DSP_ENABLE_DMA_TX, 0);
				} else if(ddata &&ddata->dev_ioctl_2) {
					ddata->dev_ioctl_2(ddata, SND_DSP_ENABLE_DMA_TX, 0);
					/* wait until all devices settings done, then we can really write data */
					ddata->dev_ioctl_2(ddata, SND_DSP_FLUSH_SYNC, 0);
				}
			}
		}
	}

write_return:
	mutex_unlock(&dp->mutex);
	if (ret < 0)
		return ret;
	return count - mcount;
}

/********************************************************\
 * ioctl
 \********************************************************/
unsigned int xb_snd_dsp_poll(struct file *file,
			     poll_table *wait,
			     struct snd_dev_data *ddata)
{
	return -EINVAL;
}

/********************************************************\
 * ioctl
\********************************************************/
/**
 * only the dsp device opend as O_RDONLY or O_WRONLY, ioctl
 * works, if a dsp device opend as O_RDWR, it will return -1
 **/
long xb_snd_dsp_ioctl(struct file *file,
		      unsigned int cmd,
		      unsigned long arg,
		      struct snd_dev_data *ddata)
{
	long ret = -EINVAL;
	struct dsp_pipe *dp = NULL;
	struct dsp_endpoints *endpoints = NULL;

	if (ddata == NULL)
		return -ENODEV;

	if(ddata->priv_data) {
		endpoints = xb_dsp_get_endpoints(ddata);
	} else {
		endpoints = (struct dsp_endpoints *)ddata->ext_data;
	}
	if (endpoints == NULL)
		return -ENODEV;

	/* O_RDWR mode operation, do not allowed */
	if ((file->f_mode & FMODE_READ) && (file->f_mode & FMODE_WRITE))
		return -EPERM;
	switch (cmd) {
		//case SNDCTL_DSP_BIND_CHANNEL:
		/* OSS 4.x: Route setero output to the specified channels(obsolete) */
		/* we do't support here */
		//break;
	case SNDCTL_DSP_STEREO:
	case SNDCTL_DSP_CHANNELS: {
		/* OSS 4.x: set the number of audio channels */
		int channels = -1;

		if (get_user(channels, (int *)arg)) {
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}

		if (cmd == SNDCTL_DSP_STEREO) {
			if (channels > 1){
				ret = -EINVAL;
				goto EXIT_IOCTRL;
			}
			channels = channels ? 2 : 1;
		}

		/* fatal: this command can be well used in O_RDONLY and O_WRONLY mode,
		   if opend as O_RDWR, only replay channels will be set, if record
		   channels also want to be set, use cmd SOUND_PCM_READ_CHANNELS instead*/
		if (file->f_mode & FMODE_WRITE) {
			dp = endpoints->out_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_SET_REPLAY_CHANNELS, (unsigned long)&channels);
				mutex_unlock(&dp->mutex);
			} else if(ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_REPLAY_CHANNELS, (unsigned long)&channels);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
			dp->channels = channels;
		} else if (file->f_mode & FMODE_READ) {
			dp = endpoints->in_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_SET_RECORD_CHANNELS, (unsigned long)&channels);
				mutex_unlock(&dp->mutex);
			} else if (ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_RECORD_CHANNELS, (unsigned long)&channels);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
			dp->channels = channels;
		} else{
			ret = -EPERM;
			goto EXIT_IOCTRL;
		}

		if (cmd == SNDCTL_DSP_STEREO) {
			if (channels > 2) {
				ret = -EFAULT;
				goto EXIT_IOCTRL;
			}
			channels = channels == 2 ? 1 : 0;
		}

		ret = put_user(channels, (int *)arg);
		break;
	}

		//case SNDCTL_DSP_COOKEDMODE:
		/* OSS 4.x: Disable/enable the "on fly" format conversions made by the OSS software */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_CURRENT_IPTR:
		/* OSS 4.x: Returns the current recording position */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_CURRENT_OPTR:
		/* OSS 4.x: Returns the current playback position */
		/* we do't support here */
		//break;

	case SNDCTL_DSP_GETBLKSIZE: {
		/* OSS 4.x: Get the current fragment size (obsolete) */
		int blksize = 0;

		if (file->f_mode & FMODE_WRITE) {
			dp = endpoints->out_endpoint;
		} else if (file->f_mode & FMODE_READ) {
			dp = endpoints->in_endpoint;
		} else {
			ret = -EPERM;
			goto EXIT_IOCTRL;
		}
		blksize = dp->buffersize;

		ret = put_user(blksize, (int *)arg);
		break;
	}

		//case SNDCTL_DSP_GETCAPS:
		/* OSS 4.x: Returns the capabilities of an audio device */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GETCHANNELMASK:
		/* OSS 4.x: Retruns the bindings supported by the device (obsolete) */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GET_CHNORDER:
		/* OSS 4.x: Get the channel ordering of a muti channel device */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GETERROR:
		/* OSS 4.x: Returns audio device error infomation */
		/* we do't support here */
		//break;

	case SNDCTL_DSP_GETFMTS: {
		/* OSS 4.x: Returns a list of natively supported sample formats */
		int mask = -1;

		/* fatal: this command can be well used in O_RDONLY and O_WRONLY mode,
		   if opend as O_RDWR, only replay supported formats will be return */
		if (file->f_mode & FMODE_WRITE) {
			dp = endpoints->out_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_GET_REPLAY_FMT_CAP, (unsigned long)&mask);
				mutex_unlock(&dp->mutex);
			} else if(ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_GET_REPLAY_FMT_CAP, (unsigned long)&mask);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
		} else if (file->f_mode & FMODE_READ) {
			dp = endpoints->in_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_GET_RECORD_FMT_CAP, (unsigned long)&mask);
				mutex_unlock(&dp->mutex);
			} else if(ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_GET_RECORD_FMT_CAP, (unsigned long)&mask);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
		} else {
			ret = -EPERM;
			goto EXIT_IOCTRL;
		}
		ret = put_user(mask, (int *)arg);
		break;
	}

		//case SNDCTL_DSP_GETIPEAKS:
		/* OSS 4.x: The peak levels for all recording channels */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GETIPTR:
		/* OSS 4.x: Returns the current recording pointer (obsolete) */
		/* we do't support here */
		//break;

	case SNDCTL_DSP_GETISPACE: {
		/* OSS 4.x: Returns the amount of recorded data that can be read without blocking */
		audio_buf_info audio_info;

		if (file->f_mode & FMODE_READ) {
			dp = endpoints->in_endpoint;
			mutex_lock(&dp->mutex);
			audio_info.fragments = get_free_dsp_node_count(dp);
			audio_info.fragsize = dp->buffersize;
			audio_info.fragstotal = dp->fragcnt;
			audio_info.bytes = get_free_dsp_node_count(dp) * dp->buffersize;
			mutex_unlock(&dp->mutex);
			ret =  copy_to_user((void *)arg, &audio_info, sizeof(audio_info)) ? -EFAULT : 0;
		} else {
			ret = -EINVAL;
			goto EXIT_IOCTRL;
		}
		break;
	}

		//case SNDCTL_DSP_GETODELAY:
		/* OSS 4.x: Returns the playback buffering delay */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GETOPEAKS:
		/* OSS 4.x: The peak levels for all playback channels */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GETOPTR:
		/* OSS 4.x: Returns the current playback pointer (obsolete) */
		/* we do't support here */
		//break;

	case SNDCTL_DSP_GETOSPACE: {
		/* OSS 4.x: Returns the amount of playback data that can be written without blocking */
		audio_buf_info audio_info;

		if (file->f_mode & FMODE_WRITE) {
			dp = endpoints->out_endpoint;
			mutex_lock(&dp->mutex);
			audio_info.fragments = get_free_dsp_node_count(dp);
			audio_info.fragsize = dp->buffersize;
			audio_info.fragstotal = dp->fragcnt;
			audio_info.bytes = get_free_dsp_node_count(dp) * dp->buffersize;
			mutex_unlock(&dp->mutex);
			ret =  copy_to_user((void *)arg, &audio_info, sizeof(audio_info)) ? -EFAULT : 0;
		} else {
			ret = -EINVAL;
			goto EXIT_IOCTRL;
		}

		break;
	}

		//case SNDCTL_DSP_GET_PLAYTGT_NAMES:
		/* OSS 4.x: Returns labels for the currently available output routings */
		//break;

		//case SNDCTL_DSP_GET_PLAYTGT:
		/* OSS 4.x: Returns the current output routing */
		//break;

		//case SNDCTL_DSP_GETPLAYVOL:
		/* OSS 4.x: Returns the current audio playback volume */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GET_RECSRC_NAMES:
		/* OSS 4.x: Returns labels for the currently available recoring sources */
		//break;

		//case SNDCTL_DSP_GET_RECSRC:
		/* OSS 4.x: Returns the current recording source */
		//break;

		//case SNDCTL_DSP_GET_RECVOL:
		/* OSS 4.x: Returns the current audio recording level */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_GETTRIGGER:
		/* OSS 4.x: Returns the current trigger bits (obsolete) */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_HALT_INPUT:
		/* OSS 4.x: Aborts audio recording operation */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_HALT_OUTPUT:
		/* OSS 4.x: Aborts audio playback operation */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_HALT:
		/* OSS 4.x: Aborts audio recording and/or playback operation */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_LOW_WATER:
		/* OSS 4.x: Sets the trigger treshold for select() */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_NONBLOCK:
		/* OSS 4.x: Force non-blocking mode */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_POLICY:
		/* OSS 4.x: Sets the timing policy of an audio device */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_POST:
		/* OSS 4.x: Forces audio playback to start (obsolete) */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_READCTL:
		/* OSS 4.x: Reads the S/PDIF interface status */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SAMPLESIZE:
		/* OSS 4.x: Sets the sample size (obsolete) */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SETDUPLEX:
		/* OSS 4.x: Truns on the duplex mode */
		/* we do't support here */
		//break;

	case SNDCTL_DSP_SETFMT: {
		/* OSS 4.x: Select the sample format */
		int fmt = -1;

		if (get_user(fmt, (int *)arg)) {
		       ret = -EFAULT;
		       goto EXIT_IOCTRL;
		}

		if (fmt == AFMT_QUERY) {
			/* fatal: this command can be well used in O_RDONLY and O_WRONLY mode,
			   if opend as O_RDWR, only replay current format will be return */
			if (file->f_mode & FMODE_WRITE) {
				dp = endpoints->out_endpoint;
				if (ddata->dev_ioctl) {
					mutex_lock(&dp->mutex);
					ret = (int)ddata->dev_ioctl(SND_DSP_GET_REPLAY_FMT, (unsigned long)&fmt);
					mutex_unlock(&dp->mutex);
				} else if(ddata->dev_ioctl_2) {
					mutex_lock(&dp->mutex);
					ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_GET_REPLAY_FMT, (unsigned long)&fmt);
					mutex_unlock(&dp->mutex);
				}
				if (!ret)
					break;
			} else if (file->f_mode & FMODE_READ) {
				dp = endpoints->in_endpoint;
				if (ddata->dev_ioctl){
					mutex_lock(&dp->mutex);
					ret = (int)ddata->dev_ioctl(SND_DSP_GET_RECORD_FMT, (unsigned long)&fmt);
					mutex_unlock(&dp->mutex);
				} else if(ddata->dev_ioctl_2) {
					mutex_lock(&dp->mutex);
					ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_GET_RECORD_FMT, (unsigned long)&fmt);
					mutex_unlock(&dp->mutex);
				}
				if (!ret)
					break;
			} else {
				ret = -EPERM;
				goto EXIT_IOCTRL;
			}
		}

		/* fatal: this command can be well used in O_RDONLY and O_WRONLY mode,
		   if opend as O_RDWR, only replay format will be set */
		if (file->f_mode & FMODE_WRITE) {
			/* set format */
			dp = endpoints->out_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_SET_REPLAY_FMT, (unsigned long)&fmt);
				mutex_unlock(&dp->mutex);
			} else if(ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_REPLAY_FMT, (unsigned long)&fmt);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
		} else if (file->f_mode & FMODE_READ) {
			/* set format */
			dp = endpoints->in_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_SET_RECORD_FMT, (unsigned long)&fmt);
				mutex_unlock(&dp->mutex);
			} else if(ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_RECORD_FMT, (unsigned long)&fmt);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
		} else {
			ret = -EPERM;
			goto EXIT_IOCTRL;
		}
		ret = put_user(fmt, (int *)arg);
		break;
	}

	case SNDCTL_DSP_SETFRAGMENT: {
#define FRAGMENT_SIZE_MUX 0x0000ffff
#define FRAGMENT_CNT_MUX  0xffff0000
		int fragment = -1;
		int fragcnts = -1;
		int fragsize = 1;
		int i;


		if (get_user(fragment, (int *)arg)) {
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}
		fragcnts = ((fragment & FRAGMENT_CNT_MUX) >> 16);
		if (fragcnts > 8 || fragcnts < 2) {
			ret = -EINVAL;
			goto EXIT_IOCTRL;
		}

		for (i = 0; i < (fragment & FRAGMENT_SIZE_MUX);i++)
			fragsize *= 2;


		if (file->f_mode & FMODE_WRITE) {
			dp = endpoints->out_endpoint;
			if (fragsize < 16 || fragsize > 8192) {
				ret = -EINVAL;
				goto EXIT_IOCTRL;
			}
			printk(KERN_WARNING"CHANGE REPALY BUFFERSIZE NOW.\n");
		} else if (file->f_mode & FMODE_READ) {
			if (dp->channels > 1) {
				if (fragsize < 16 || fragsize > 8192) {
					ret = -EINVAL;
					goto EXIT_IOCTRL;
				}
			} else {
				/*
				 * If mono we use stereo record ,then filter on software
				 * So we use half of a fragment at most
				 */
				if (fragsize < 16 || fragsize > 4096) {
					ret = -EINVAL;
					goto EXIT_IOCTRL;
				}
			}
			dp = endpoints->in_endpoint;
		}

		dp->buffersize = fragsize;
		//dp->fragcnt = fragcnts;
		/* TODO:
		 *	We not support change fragcnts
		 *	Should we support??
		 */
		printk("audio buffersize change to %d",dp->buffersize);
		printk("audio buffercnt change to %d",dp->fragcnt);

		ret = put_user(fragment, (int *)arg);
#undef FRAGMENT_SIZE_MUX
#undef FRAGMENT_CNT_MUX
		break;
	}


		//case SNDCTL_DSP_SET_PLAYTGT:
		/* OSS 4.x: Sets the current output routing */
		//break;

		//case SNDCTL_DSP_SETPLAYVOL:
		/* OSS 4.x: Changes the current audio playback volume */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SET_RECSRC:
		/* OSS 4.x: Sets the current recording source */
		//break;

		//case SNDCTL_DSP_SETRECVOL:
		/* OSS 4.x: Changes the current audio recording level */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SETSYNCRO:
		/* OSS 4.x: Slaves the audio device to the /dev/sequencer driver (obsolete) */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SETTRIGGER:
		/* OSS 4.x: Starts audio recording and/or playback in sync */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SILENCE:
		/* OSS 4.x: clears the playback buffer with silence */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SKIP:
		/* OSS 4.x: Discards all samples in the playback buffer */
		/* we do't support here */
		//break;

	case SNDCTL_DSP_SPEED: {
		/* OSS 4.x: Set the sampling rate */
		int rate = -1;

		if (get_user(rate, (int *)arg)) {
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}

		/* fatal: this command can be well used in O_RDONLY and O_WRONLY mode,
		   if opend as O_RDWR, only replay rate will be set, if record rate
		   also need to be set, use cmd SOUND_PCM_READ_RATE instead */
		if (file->f_mode & FMODE_WRITE) {
			dp = endpoints->out_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_SET_REPLAY_RATE, (unsigned long)&rate);
				mutex_unlock(&dp->mutex);
			} else if(ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_REPLAY_RATE, (unsigned long)&rate);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
			dp->samplerate = rate;
		} else if (file->f_mode & FMODE_READ) {
			dp = endpoints->in_endpoint;
			if (ddata->dev_ioctl) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl(SND_DSP_SET_RECORD_RATE, (unsigned long)&rate);
				mutex_unlock(&dp->mutex);
			} else if (ddata->dev_ioctl_2) {
				mutex_lock(&dp->mutex);
				ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_RECORD_RATE, (unsigned long)&rate);
				mutex_unlock(&dp->mutex);
			}
			if (!ret)
				break;
			dp->samplerate = rate;
		} else {
			ret = -EPERM;
			goto EXIT_IOCTRL;
		}
		ret = put_user(rate, (int *)arg);

		break;
	}

		//case SNDCTL_DSP_SUBDIVIDE:
		/* OSS 4.x: Requests the device to use smaller fragments (obsolete) */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_SYNCGROUP:
		/* OSS 4.x: Creates a synchronization group */
		/* we do't support here */
		//break;

	case SNDCTL_DSP_SYNC:
		/* OSS 4.x: Suspend the application until all samples have been played */
		//TODO: check it will wait until replay complete
		if (file->f_mode & FMODE_WRITE) {
			int i = 0x7fffff;
			dp = endpoints->out_endpoint;
			mutex_lock(&dp->mutex);
			dp->wait_stop_dma = true;
			mutex_unlock(&dp->mutex);
			while (wait_event_interruptible(dp->wq,
							dp->is_trans == false) && i--);
			if (!i) {
				ret = -EFAULT;
				goto EXIT_IOCTRL;
			}
			mutex_lock(&dp->mutex);
			del_timer_sync(&dp->transfer_watchdog);
			snd_release_node(dp);
			mutex_unlock(&dp->mutex);
			ret = 0;
		} else
			ret = -ENOSYS;
		break;

		//case SNDCTL_DSP_RESET:
		//break;

		//case SNDCTL_DSP_SYNCSTART:
		/* OSS 4.x: Starts all devices added to a synchronization group */
		/* we do't support here */
		//break;

		//case SNDCTL_DSP_WRITECTL:
		/* OSS 4.x: Alters the S/PDIF interface setup */
		/* we do't support here */
		//break;
		//
	case SNDCTL_EXT_MOD_TIMER: {
		int mod_timer	= 1;

		if (get_user(mod_timer, (int*)arg)) {
			ret = -EINVAL;
			goto EXIT_IOCTRL;
		}

		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->in_endpoint;
		else
			dp = endpoints->out_endpoint;

		dp->mod_timer = !!mod_timer;

		ret = put_user(dp->mod_timer, (int *)arg);
		break;
	}

	case SNDCTL_EXT_SET_DEVICE: {
		/* extention: used for set audio route */
		int device;
		if (get_user(device, (int*)arg)) {
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}
		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->in_endpoint;
		else
			dp = endpoints->out_endpoint;
		if (ddata->dev_ioctl) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl(SND_DSP_SET_DEVICE, (unsigned long)&device);
			mutex_unlock(&dp->mutex);
		} else if(ddata->dev_ioctl_2) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_DEVICE, (unsigned long)&device);
			mutex_unlock(&dp->mutex);
		}
		if (!ret)
			break;

		ret = put_user(device, (int *)arg);
		break;
	}


	case SNDCTL_EXT_SET_BUFFSIZE: {
		int buffersize = 4096;
		if (get_user(buffersize,(int*)arg)) {
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}

		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->out_endpoint;
		else if (file->f_mode & FMODE_READ)
			dp = endpoints->in_endpoint;

		buffersize = buffersize & (~0xff);
		if (buffersize >= dp->fragsize)
			dp->buffersize = dp->fragsize;
		else if (buffersize < dp->fragsize && buffersize >= 1024)
			dp->buffersize = buffersize;
		else if (dp->fragsize/4 >= 1024)
			dp->buffersize = 1024;
		else
			dp->buffersize = dp->fragsize/4;

		printk("audio buffersize change to %d",dp->buffersize);
		ret = put_user(dp->buffersize, (int *)arg);
		break;
	}

	case SNDCTL_EXT_SET_STANDBY: {
		/* extention: used for set standby and resume from standby */
		int mode = -1;

		if (get_user(mode, (int*)arg)) {
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}
		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->out_endpoint;
		else if (file->f_mode & FMODE_READ)
			dp = endpoints->in_endpoint;
		if (ddata->dev_ioctl) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl(SND_DSP_SET_STANDBY, (unsigned long)&mode);
			mutex_unlock(&dp->mutex);
		} else if (ddata->dev_ioctl_2) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_STANDBY, (unsigned long)&mode);
			mutex_unlock(&dp->mutex);
		}
		if (!ret)
			break;

		ret = put_user(mode, (int*)arg);
		break;
	}


	case SNDCTL_EXT_STOP_DMA: {
		if (file->f_mode & FMODE_READ)
			dp = endpoints->in_endpoint;
		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->out_endpoint;
		if (dp != NULL) {

			mutex_lock(&dp->mutex);
			del_timer_sync(&dp->transfer_watchdog);
			dp->force_stop_dma = true;
			dp->wait_stop_dma = true;
			wake_up(&dp->wq);
			mutex_unlock(&dp->mutex);

			if(wait_event_interruptible_timeout(dp->wq,dp->is_trans == false, HZ) <= 0) {
				dmaengine_terminate_all(dp->dma_chan);
			}

			mutex_lock(&dp->mutex);
			if (dp->force_stop_dma == true)
				msleep(20);
			snd_release_node(dp);
			dp->is_trans = false;
			dp->wait_stop_dma = false;
			mutex_unlock(&dp->mutex);
		}
		ret = 0;
		goto EXIT_IOCTRL;
	}

	case SNDCTL_EXT_SET_REPLAY_VOLUME: {
		int vol;
		if (get_user(vol, (int*)arg)){
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}
		if (file->f_mode & FMODE_READ)
			dp = endpoints->in_endpoint;
		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->out_endpoint;
		if (ddata->dev_ioctl) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl(SND_DSP_SET_REPLAY_VOL, (unsigned long)&vol);
			mutex_unlock(&dp->mutex);
		} else if (ddata->dev_ioctl_2) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_REPLAY_VOL, (unsigned long)&vol);
			mutex_unlock(&dp->mutex);
		}
			if (!ret)
				break;
		ret = put_user(vol, (int *)arg);
		break;
	}

	case SNDCTL_EXT_SET_RECORD_VOLUME: {
		int vol;
		if (get_user(vol, (int*)arg)){
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}
		if (file->f_mode & FMODE_READ)
			dp = endpoints->in_endpoint;
		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->out_endpoint;
		if (ddata->dev_ioctl) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl(SND_DSP_SET_RECORD_VOL, (unsigned long)&vol);
			mutex_unlock(&dp->mutex);
		} else if (ddata->dev_ioctl_2) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_RECORD_VOL, (unsigned long)&vol);
			mutex_unlock(&dp->mutex);
		}
			if (!ret)
				break;
		ret = put_user(vol, (int *)arg);
		break;
	}

	case SNDCTL_DSP_SETTRIGGER: {

		int tri_h;
		if (get_user(tri_h, (int*)arg)){
			ret = -EFAULT;
			goto EXIT_IOCTRL;
		}
		if (file->f_mode & FMODE_READ)
			dp = endpoints->in_endpoint;
		if (file->f_mode & FMODE_WRITE)
			dp = endpoints->out_endpoint;
		if (ddata->dev_ioctl) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl(SND_DSP_SET_VOICE_TRIGGER, (unsigned long)&tri_h);
			mutex_unlock(&dp->mutex);
		} else if (ddata->dev_ioctl_2) {
			mutex_lock(&dp->mutex);
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_VOICE_TRIGGER, (unsigned long)&tri_h);
			mutex_unlock(&dp->mutex);
		}
		if (!ret)
			break;
		ret = put_user(*(int *)arg, (int *)arg);
		break;
	}
	default:
		printk("SOUDND ERROR: %s(line:%d) ioctl command %d is not supported\n",
		       __func__, __LINE__, cmd);
		ret = -1;
		goto EXIT_IOCTRL;
	}

	/* some operation may need to reconfig the dma, such as,
	   if we reset the format, it may cause reconfig the dma */
	if (file->f_mode & FMODE_READ) {
		dp = endpoints->in_endpoint;
		if (dp && dp->need_reconfig_dma == true) {
			if (dp->is_trans == false) {
				mutex_lock(&dp->mutex);
				snd_reconfig_dma(dp);
				mutex_unlock(&dp->mutex);
				dp->need_reconfig_dma = false;
			}
		}
	}
	if (file->f_mode & FMODE_WRITE) {
		dp = endpoints->out_endpoint;
		if (dp && dp->need_reconfig_dma == true) {
			if (dp->is_trans == false) {
				mutex_lock(&dp->mutex);
				snd_reconfig_dma(dp);
				mutex_unlock(&dp->mutex);
				dp->need_reconfig_dma = false;
			}
		}
	}
EXIT_IOCTRL:
	return ret;
}

/********************************************************\
 * mmap
 \********************************************************/
int xb_snd_dsp_mmap(struct file *file,
		    struct vm_area_struct *vma,
		    struct snd_dev_data *ddata)
{
	int ret = -ENODEV;
	struct dsp_pipe *dp = NULL;
	struct dsp_endpoints *endpoints = NULL;

	if (!((file->f_mode & FMODE_READ) && (file->f_mode & FMODE_WRITE)))
		return -EPERM;

	if (ddata == NULL)
		return ret;

	if(ddata->priv_data) {
		endpoints = xb_dsp_get_endpoints(ddata);
	} else {
		endpoints = (struct dsp_endpoints *)ddata->ext_data;
	}
	if (endpoints == NULL)
		return ret;

	if (vma->vm_flags & VM_READ) {
		dp = endpoints->in_endpoint;
		if (dp->is_used) {
			ret = mmap_pipe(dp, vma);
			if (ret)
				return ret;
			dp->is_mmapd = true;
		}
	}

	if (vma->vm_flags & VM_WRITE) {
		dp = endpoints->out_endpoint;
		if (dp->is_used) {
			ret = mmap_pipe(dp, vma);
			if (ret)
				return ret;
			dp->is_mmapd = true;
		}
	}

	return ret;
}

/********************************************************\
 * open
\********************************************************/
int xb_snd_dsp_open(struct inode *inode,
		    struct file *file,
		    struct snd_dev_data *ddata)
{
	int ret = -ENXIO;
	struct dsp_pipe *dpi = NULL;
	struct dsp_pipe *dpo = NULL;
	struct dsp_endpoints *endpoints = NULL;
#ifndef CONFIG_ANDROID
	int arg = 0, state = 0;
#endif
	ENTER_FUNC();

	if (ddata == NULL) {
		while(1);
		return -ENODEV;
	}

	if(ddata->priv_data) {
		endpoints = xb_dsp_get_endpoints(ddata);
	} else {
		endpoints = (struct dsp_endpoints *)ddata->ext_data;
	}
	if (endpoints == NULL) {
		while(1);
		return -ENODEV;
	}
	dpi = endpoints->in_endpoint;
	dpi->pddata = ddata;	//used for callback
	dpo = endpoints->out_endpoint;
	dpo->pddata = ddata;

	if (file->f_mode & FMODE_READ && file->f_mode & FMODE_WRITE) {
		if (dpo->is_used)
			return 0;
		else if (dpi->is_used && dpo->is_used)
			return -ENODEV;
	}

	if (file->f_mode & FMODE_READ) {
		ret = 0;
		if (dpi == NULL)
			return -ENODEV;
		mutex_lock(&dpi->mutex);
		if (dpi->is_used) {
			printk("\nAudio read device is busy!\n");
			ret = -EBUSY;
			goto EXIT_READ_LABLE;
		}

		dpi->is_non_block = file->f_flags & O_NONBLOCK ? true : false;

		/* enable dsp device record */
		if (ddata->dev_ioctl) {
			ret = (int)ddata->dev_ioctl(SND_DSP_ENABLE_RECORD, 0);
		} else if (ddata->dev_ioctl_2) {
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_ENABLE_RECORD, 0);
		}
		if (ret < 0)
			return -EIO;
		dpi->is_used = true;
		/* request dma for record */
		ret = snd_reuqest_dma(dpi);
		if (ret) {
			dpi->is_used = false;
			printk("AUDIO ERROR, can't get dma!\n");
		}
#ifndef CONFIG_ANDROID
		arg = SND_DEVICE_BUILDIN_MIC;
		if (ddata->dev_ioctl) {
			arg = (int)ddata->dev_ioctl(SND_DSP_SET_DEVICE, (unsigned long)&arg);
		} else if (ddata->dev_ioctl_2) {
			arg = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_DEVICE, (unsigned long)&arg);
		}
		if (arg < 0) {
			ret = -EIO;
			goto EXIT_READ_LABLE;
		}
#endif
	EXIT_READ_LABLE:
		mutex_unlock(&dpi->mutex);
		if(ret)
			return ret;
	}

	if (file->f_mode & FMODE_WRITE) {
		ret = 0;
		if (dpo == NULL) {
			return -ENODEV;
		}
		mutex_lock(&dpo->mutex);
		if (dpo->is_used) {
			printk("\nAudio write device is busy!\n");
			ret = -EBUSY;
			goto EXIT_WRITE_LABLE;
		}

		dpo->is_non_block = file->f_flags & O_NONBLOCK ? true : false;

		/* enable dsp device replay */
		if (ddata->dev_ioctl) {
			ret = (int)ddata->dev_ioctl(SND_DSP_ENABLE_REPLAY, 0);
		} else if(ddata->dev_ioctl_2) {
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_ENABLE_REPLAY, 0);
		}
			if (ret < 0) {
				ret = -EIO;
				goto EXIT_WRITE_LABLE;
			}
		dpo->is_used = true;

		/* request dma for replay */
		ret = snd_reuqest_dma(dpo);
		if (ret) {
			dpo->is_used = false;
			printk("AUDIO ERROR, can't get dma!\n");
		}


#ifndef CONFIG_ANDROID
#if defined(CONFIG_HDMI_JZ4780) || defined(CONFIG_HDMI_JZ4780_MODULE)
		dpo->force_hdmi = true;
		arg = SND_DEVICE_HDMI;
		if (ddata->dev_ioctl) {
			arg = (int)ddata->dev_ioctl(SND_DSP_SET_DEVICE, (unsigned long)&arg);
		} else if(ddata->dev_ioctl_2) {
			arg = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_DEVICE, (unsigned long)&arg);
		}
		printk("%s: HDMI output\n",__func__);
		if (arg < 0) {
			ret = -EIO;
			goto EXIT_WRITE_LABLE;
		}
#else
		dpo->force_hdmi = false;
		if (ddata->dev_ioctl) {
			arg = (int)ddata->dev_ioctl(SND_DSP_GET_HP_DETECT, (unsigned long)&state);
			if (arg < 0) {
				ret = -EIO;
				goto EXIT_WRITE_LABLE;
			}
			if (state)
				arg = SND_DEVICE_HEADSET;
			else
				arg = SND_DEVICE_SPEAKER;
			arg = (int)ddata->dev_ioctl(SND_DSP_SET_DEVICE, (unsigned long)&arg);
			if (arg < 0) {
				ret = -EIO;
				goto EXIT_WRITE_LABLE;
			}
		} else if(ddata->dev_ioctl_2) {
			arg = (int)ddata->dev_ioctl_2(ddata, SND_DSP_GET_HP_DETECT, (unsigned long)&state);
			if (arg < 0) {
				return -EIO;
			}
			if (state)
				arg = SND_DEVICE_HEADSET;
			else
				arg = SND_DEVICE_SPEAKER;
			arg = (int)ddata->dev_ioctl_2(ddata, SND_DSP_SET_DEVICE, (unsigned long)&arg);
			if (arg < 0) {
				return -EIO;
			}
		}
#endif
#endif
EXIT_WRITE_LABLE:
		mutex_unlock(&dpo->mutex);
		if(ret)
			return ret;
	}

#ifdef DEBUG_REPLAY
	printk("DEBUG:----open %s  %s\tline:%d\n",DEBUG_REPLAYE_FILE,__func__,__LINE__);
	f_test = filp_open(DEBUG_REPLAYE_FILE, O_RDWR | O_APPEND |O_CREAT, S_IRUSR | S_IWUSR);
	if (!IS_ERR(f_test)) {
		printk("open debug audio sussess %p.\n",f_test);
		f_test_offset = f_test->f_pos;
	}
	else
		printk("---->open %s failed %d!\n",DEBUG_REPLAYE_FILE,f_test);
#endif
#ifdef DEBUG_RECORD
	printk("DEBUG:----open %s %s\tline:%d\n",DEBUG_RECORD_FILE, __func__,__LINE__);
	file_record = filp_open(DEBUG_RECORD_FILE, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (!IS_ERR(file_record)) {
		printk("open debug audiorecord sucssess %p. \n", file_record);
		file_record_offset = file_record->f_pos;
	}
	else
		printk("---->open %s failed %d!\n",DEBUG_RECORD_FILE,file_record);
#endif
	LEAVE_FUNC();
	return ret;
}

/********************************************************\
 * release
 \********************************************************/
int xb_snd_dsp_release(struct inode *inode,
		       struct file *file,
		       struct snd_dev_data *ddata)
{
	int ret = 0;
	struct dsp_pipe *dpi = NULL;
	struct dsp_pipe *dpo = NULL;
	struct dsp_endpoints *endpoints = NULL;

	if (ddata == NULL)
		return -1;

	if (file->f_mode & FMODE_READ && file->f_mode & FMODE_WRITE)
		return 0;

	if(ddata->priv_data) {
		endpoints = xb_dsp_get_endpoints(ddata);
	} else {
		endpoints = (struct dsp_endpoints *)ddata->ext_data;
	}
	if (endpoints == NULL)
		return -1;

	if (file->f_mode & FMODE_READ) {
		dpi = endpoints->in_endpoint;
//		if (ddata->minor == SND_DEV_DSP3)
//			snd_dmic_release_dma(dpi);
//		else
		mutex_lock(&dpi->mutex);
		snd_release_dma(dpi);
		snd_release_node(dpi);
		mutex_unlock(&dpi->mutex);
	}

	if (file->f_mode & FMODE_WRITE) {
		dpo = endpoints->out_endpoint;
		mutex_lock(&dpo->mutex);
		snd_release_dma(dpo);
		snd_release_node(dpo);
		mutex_unlock(&dpo->mutex);
	}

	if (dpi) {
		/* put all used node to free node list */
		if (ddata->dev_ioctl) {
			ret = (int)ddata->dev_ioctl(SND_DSP_DISABLE_DMA_RX, 0);
			ret |= (int)ddata->dev_ioctl(SND_DSP_DISABLE_RECORD, 0);
			if (ret)
				ret = -EFAULT;
		} else if(ddata->dev_ioctl_2) {
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_DISABLE_DMA_RX, 0);
			ret |= (int)ddata->dev_ioctl_2(ddata, SND_DSP_DISABLE_RECORD, 0);
			if (ret)
				ret = -EFAULT;
		}
		dpi->is_used = false;
	}

	if (dpo) {
		/* put all used node to free node list */
		if (ddata->dev_ioctl) {
			ret = (int)ddata->dev_ioctl(SND_DSP_DISABLE_DMA_TX, 0);
			ret |= (int)ddata->dev_ioctl(SND_DSP_DISABLE_REPLAY, 0);
			if (ret)
				ret = -EFAULT;
		} else if(ddata->dev_ioctl_2) {
			ret = (int)ddata->dev_ioctl_2(ddata, SND_DSP_DISABLE_DMA_TX, 0);
			ret |= (int)ddata->dev_ioctl_2(ddata, SND_DSP_DISABLE_REPLAY, 0);
			if (ret)
				ret = -EFAULT;
		}
		dpo->is_used = false;
	}
	/* flush all before another setting */
	if(ddata->dev_ioctl_2) {
		ddata->dev_ioctl_2(ddata, SND_DSP_FLUSH_SYNC, 0);
	}

#ifdef DEBUG_REPLAY
	if (!IS_ERR(f_test))
		filp_close(f_test, NULL);
#endif
#ifdef DEBUG_RECORD
	if (!IS_ERR(file_record))
		filp_close(file_record, NULL);
#endif
	return ret;
}

/********************************************************\
 * xb_snd_probe
\********************************************************/
int xb_snd_dsp_probe(struct snd_dev_data *ddata)
{
	int ret = -1;
	struct dsp_pipe *dp = NULL;
	struct dsp_endpoints *endpoints = NULL;

	if (ddata == NULL)
		return -1;

	if(ddata->priv_data) {
		endpoints = xb_dsp_get_endpoints(ddata);
	} else {
		endpoints = (struct dsp_endpoints *)ddata->ext_data;
	}

	if (endpoints == NULL)
		return -1;

	/* out_endpoint init */
	if ((dp = endpoints->out_endpoint) != NULL) {
		ret = init_pipe(dp , ddata->dev,DMA_TO_DEVICE);
	}
	if (ret)
		goto error1;
	if ((dp = endpoints->in_endpoint) != NULL) {
		ret = init_pipe(dp , ddata->dev,DMA_FROM_DEVICE);
		if (ret)
			goto error2;
	}
	return 0;

error2:
	deinit_pipe(endpoints->out_endpoint, ddata->dev);
error1:
	return ret;
}
int xb_snd_dsp_suspend(struct snd_dev_data *ddata)
{
	if (ddata){
		struct dsp_endpoints * endpoints = NULL;
		bool out_trans = false,in_trans = false;

		if(ddata->priv_data) {
			endpoints = xb_dsp_get_endpoints(ddata);
		} else {
			endpoints = (struct dsp_endpoints *)ddata->ext_data;
		}
		if(endpoints->out_endpoint) {
			mutex_lock(&endpoints->out_endpoint->mutex);
			out_trans = endpoints->out_endpoint->is_trans;
			mutex_unlock(&endpoints->out_endpoint->mutex);
		}
		if(endpoints->in_endpoint){
			mutex_lock(&endpoints->in_endpoint->mutex);
			in_trans = endpoints->in_endpoint->is_trans;
			mutex_unlock(&endpoints->in_endpoint->mutex);
		}
		if(out_trans || in_trans)
			return -1;
	}
	return 0;
}
int xb_snd_dsp_resume(struct snd_dev_data *ddata)
{
	return 0;
}
