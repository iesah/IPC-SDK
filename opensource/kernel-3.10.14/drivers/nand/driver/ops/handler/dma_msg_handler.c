/*
 * we default that nemc is only one.
 * if there are some nemcs,please modify mcu_init and dma_msg_handle_init
 */

#include <mach/jzdma.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <dma_msg_handler.h>
#include <nand_bch.h>
#include <nand_ops.h>
#include <nddata.h>
#include <nand_debug.h>
#include <nand_info.h>
#include <os_clib.h>

#define INIT_MSG          0
#define NONE_INIT_MSG     1
#define MCU_CONTROL       0xb3420030

#define MCU_TEST_INTER_NAND
#ifdef MCU_TEST_INTER_NAND
#define MCU_TEST_DATA_NAND 0xB34257C0 //PDMA_BANK7 - 0x40
#endif

typedef struct __nand_dma {
	struct dma_chan                    	*msg_chan;     /* mv msg to mcu */
	struct dma_async_tx_descriptor  	 *desc;         /* descriptor */
	struct taskmsg_init            		*msg_init;     /* send msg to mcu init*/
	dma_addr_t                         	msginit_phyaddr;   /* msg_init physical address*/
	enum jzdma_type                    	chan_type;
	unsigned int 				mailbox;       /* receive mcu return value*/
	nand_data				*data;
	int					suspend_flag;
	int					resume_flag;
} nand_dma;

static struct completion comp;
static int g_taskmsgret;
static nand_dma *g_nddma = NULL;

static bool filter(struct dma_chan *chan, void *data)
{
	nand_dma *nddma = data;
	return (void *)nddma->chan_type == chan->private;
}
static void mcu_reset(void)
{
	int tmp = *(volatile unsigned int *)MCU_CONTROL;
	tmp |= 0x01;
	*(volatile unsigned int *)MCU_CONTROL = tmp;
	ndd_ndelay(100);
	tmp = *(volatile unsigned int *)MCU_CONTROL;
	tmp &= ~0x01;
	*(volatile unsigned int *)MCU_CONTROL = tmp;
}

static void mcu_complete(void *callbak_param)
{
	unsigned int mailbox = *((unsigned int *)callbak_param);
	if(mailbox & MB_MCU_DONE ){
		if(mailbox & MSG_RET_FAIL)
			g_taskmsgret = ECC_ERROR;
		else if(mailbox & MSG_RET_MOVE)
			g_taskmsgret = BLOCK_MOVE;
		else if(mailbox & MSG_RET_EMPTY)
			g_taskmsgret = ALL_FF;
		else if(mailbox & MSG_RET_WP)
			g_taskmsgret = ENAND;
		else
			g_taskmsgret = SUCCESS;
	}else
		g_taskmsgret = MB_MCU_ERROR;
	complete(&comp);
}

static int wait_dma_finish(struct dma_chan *chan, struct dma_async_tx_descriptor *desc, void *callback, void *callback_param)
{
	volatile unsigned int test[10] ={0};
	int i;
	unsigned int timeout = 0,count = 0;
	dma_cookie_t cookie;
	desc->callback = callback;
	desc->callback_param = callback_param;

	cookie = dmaengine_submit(desc);
	if (dma_submit_error(cookie))
		RETURN_ERR(DMA_AR, "Failed to do DMA submit");

	dma_async_issue_pending(chan);
	do{
		timeout = wait_for_completion_timeout(&comp, HZ);
		if(timeout == -ERESTARTSYS)
			continue;
		//			volatile unsigned int *ret = (volatile unsigned int *)(0xb3425600);
		//			printk("pdma_task ret[0] = 0x%08x  ret[1] = 0x%08x\n",ret[0],ret[1]);
		/*
		   for(i=0;i<10;i++)
		   test[i] = ((volatile unsigned int *)MCU_TEST_DATA_NAND)[i];
		   printk("\ncpu send msg       [0x%08x]\n",test[0]);
		   printk("mcu receive msg    [0x%08x]\n",test[1]);
		   printk("mcu send mailbox   [0x%08x]\n",test[2]);
		   printk("dma receive inte   [0x%08x]\n",test[3]);
		   printk("dma mcu_irq handle [0x%08x]\n",test[4]);
		   printk("pdma_task_handle   [0x%08x]\n",test[5]);
		   printk("pdma_task_handle   [0x%08x]\n",test[6]);
		   printk("pdma_task_handle   [0x%08x]\n",test[7]);
		   printk("pdma_task_handle   [0x%08x]\n",test[8]);
		   printk("pdma_task_handle   [0x%08x]\n",test[9]);
		   printk("bch status         [0x%08x]\n",*(volatile unsigned int *)(0xb34d0184));
		   printk("bch status         [0x%08x]\n",*(volatile unsigned int *)(0xb0010540));
		   ndd_dump_taskmsg((struct task_msg *)PDMA_MSG_TCSMVA, 4);
		 */
#ifdef MCU_TEST_INTER_NAND
		if(!timeout){

			for(i=0;i<7;i++)
				test[i] = ((volatile unsigned int *)MCU_TEST_DATA_NAND)[i];
			printk("cpu send msg       [%u]\n",test[0]);
			printk("mcu receive msg    [%u]\n",test[1]);
			printk("mcu send mailbox   [%u]\n",test[2]);
			printk("dma receive inte   [%u]\n",test[3]);
			printk("dma mcu_irq handle [%u]\n",test[4]);
			printk("pdma_task_handle   [%u]\n",test[5]);
			printk("pdma_task_handle   [%u]\n",test[6]);

			printk("channel 0 state 0x%x\n",*(volatile unsigned int *)(0xb3420010));
			printk("channel 0 count 0x%x\n",*(volatile unsigned int *)(0xb3420008));
			printk("channel 0 source 0x%x\n",*(volatile unsigned int *)(0xb3420000));
			printk("channel 0 target 0x%x\n",*(volatile unsigned int *)(0xb3420004));
			printk("channel 0 command 0x%x\n",*(volatile unsigned int *)(0xb3420014));
			printk("channel 1 state 0x%x\n",*(volatile unsigned int *)(0xb3420030));
			printk("channel 1 count 0x%x\n",*(volatile unsigned int *)(0xb3420028));
			printk("channel 1 source 0x%x\n",*(volatile unsigned int *)(0xb3420020));
			printk("channel 1 target 0x%x\n",*(volatile unsigned int *)(0xb3420024));
			printk("channel 1 command 0x%x\n",*(volatile unsigned int *)(0xb3420034));
			printk("channel 2 state 0x%x\n",*(volatile unsigned int *)(0xb3420050));
			printk("channel 2 count 0x%x\n",*(volatile unsigned int *)(0xb3420048));
			printk("channel 2 source 0x%x\n",*(volatile unsigned int *)(0xb3420040));
			printk("channel 2 target 0x%x\n",*(volatile unsigned int *)(0xb3420044));
			printk("channel 2 command 0x%x\n",*(volatile unsigned int *)(0xb3420054));
			printk("channel 3 state 0x%x\n",*(volatile unsigned int *)(0xb3420070));
			printk("channel 3 count 0x%x\n",*(volatile unsigned int *)(0xb3420068));
			printk("channel 3 source 0x%x\n",*(volatile unsigned int *)(0xb3420060));
			printk("channel 3 target 0x%x\n",*(volatile unsigned int *)(0xb3420064));
			printk("channel 3 command 0x%x\n",*(volatile unsigned int *)(0xb3420074));
			printk("pdma mask register 0x%x\n",*(volatile unsigned int *)(0xb342102c));
			printk("pdma program register 0x%x\n",*(volatile unsigned int *)(0xb342101c));
			printk("mcu status  0x%x\n",*(volatile unsigned int *)(0xb3421030));
		}
#endif
		count++;
	}while(!timeout && count <= 10);

	if(!timeout){
		dmaengine_terminate_all(chan);
		RETURN_ERR(TIMEOUT, "dma wait for completion timeout");
	}

	return 0;
}

static int send_msg_to_mcu(nand_dma *nd_dma, dma_addr_t src_addr, int len)
{
	int ret = 0;
	unsigned long flags = DMA_PREP_INTERRUPT | DMA_CTRL_ACK;

	nd_dma->desc = nd_dma->msg_chan->device->device_prep_dma_memcpy(
			nd_dma->msg_chan,PDMA_MSG_TCSMPA, src_addr, len, flags);
	if(!nd_dma->desc)
		RETURN_ERR(ENAND, "nand_dma->desc is NULL");

#ifdef MCU_TEST_INTER_NAND
	(*(unsigned long long *)MCU_TEST_DATA_NAND)++;
#endif
	ret = wait_dma_finish(nd_dma->msg_chan,nd_dma->desc,mcu_complete,&nd_dma->mailbox);
	if(ret < 0)
		RETURN_ERR(ret, "wait dma finish failed");
	else
		ret = g_taskmsgret;

	return ret;
}

static int mcu_init(nand_dma *nd_dma,Nand_Task *nandtask,int id)
{
	nand_data *nd_data = nd_dma->data;
	struct taskmsg_init *msg = nd_dma->msg_init;
	chip_info *ndinfo = nd_data->cinfo;
	nand_ops_timing *ndtime = &ndinfo->ops_timing;
	unsigned int fcycle;
	int i;
	msg->ops.bits.type = MSG_MCU_INIT;
	msg->info.pagesize = ndinfo->pagesize;
	msg->info.oobsize  = ndinfo->oobsize;
	msg->info.eccsize  = nd_data->eccsize;
	/* we default that ecclevel equals the eccbit of first ndpartition;
	 *  the eccbit will be change,when pdma handles every taskmsg_prepare.
	 */
	msg->info.eccbytes = get_parity_size(24);
	msg->info.ecclevel = 24;
	/* calc firmware cycle in ps
	 *  firware has the same's clock as nemc's clock,which is AHB2.
	 */
//__ndd_dump_chip_info(ndinfo);
	fcycle = 1000000000 / (nd_data->base->nfi.rate / 1000); // unit: ps
	msg->info.twhr2 = ((ndtime->tWHR2 * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tcwaw = ((ndtime->tCWAW * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tadl = ndtime->tADL;
	msg->info.tcs = ((ndtime->tCS * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tclh= ((ndtime->tCLH * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.tsync = ((ndtime->tWC * 64 * 1000 + fcycle - 1) / fcycle) + 1;
	msg->info.fcycle = fcycle;

//	msg->info.eccpos = ndinfo->eccpos;
	msg->info.buswidth = ndinfo->buswidth;
	msg->info.rowcycle = ndinfo->rowcycles;
	/*
	 * we default that nemc is only one, which has no more than two rb.
	 */
	for(i = 0; i < nd_data->csinfo->totalchips; i++)
		msg->info.cs[i] = nd_data->csinfo->csinfo_table[i].id + 1;
	msg->info.rb0 = nd_data->rbinfo->rbinfo_table[0].gpio;
	if(nd_data->rbinfo->totalrbs > 1)
		msg->info.rb1 = nd_data->rbinfo->rbinfo_table[1].gpio;
	else
		msg->info.rb1 = 0xff;
	printk("*****  id = %d, rb0 = %d, rb1 = %d  *****\n",id,msg->info.rb0,msg->info.rb1);
	msg->info.taskmsgaddr = nandtask->msg_phyaddr;
	//	printk("########  msg_phyaddr = 0x%x # PDMA_MSG_TCSMPA = 0x%x ########\n",msg->info.taskmsgaddr,PDMA_MSG_TCSMPA);
	ndd_dma_cache_wback((unsigned long)nd_dma->msg_init,sizeof(struct taskmsg_init));
	return send_msg_to_mcu(nd_dma,nd_dma->msginit_phyaddr,sizeof(struct taskmsg_init));
}

static int dma_handle_msg(int context, Nand_Task *nt)
{
	nand_dma *nd_dma = (nand_dma *)context;
	int ret = 0, flag;
//	volatile unsigned int *retvalue = (volatile unsigned int *)(0xb3425600);

	if(nt->msg == NULL)
		RETURN_ERR(ENAND, "data buffer is NULL!");
//	if(nt->msg_index > 20)
//		ndd_dump_taskmsg(nt->msg,nt->msg_index);
	ndd_dma_cache_wback((unsigned long)nt->msg,sizeof(struct task_msg) * nt->msg_index);
	ret = send_msg_to_mcu(nd_dma, nt->msg_phyaddr, sizeof(struct task_msg) * MSG_NUMBER);
	if (ret == TIMEOUT)
		ndd_dump_taskmsg(nt->msg,nt->msg_index);
	if(ret == MB_MCU_ERROR){
		ndd_print(NDD_ERROR, "%s %d :send msg to mcu failed ! ret = %d \n",__func__,__LINE__,ret);
		mcu_reset();
		ndd_ndelay(5000);
		flag = send_msg_to_mcu(nd_dma,nd_dma->msginit_phyaddr,sizeof(struct taskmsg_init));
		if(flag != SUCCESS){
			ndd_print(NDD_FATE_ERROR, "%s %d :mcu_init is failed again! flag = %d \n",
					__func__,__LINE__,flag);
			while(1);
		}
	}
	//for(j = 0; j < ((nt->msg_index + 7)/ 8); j++)
	//printk("pdma_task retvalue[%d] = 0x%08x\n", j, retvalue[j]);
	return ret;
}
static nand_dma *dma_ops_init(nand_data *data, Nand_Task *nandtask,int id)
{
	/* we default only pdma's unit */
	unsigned int chan_type = data->base->pdma.dma_channel;
	dma_cap_mask_t mask;
	int ret = 0;
	if(g_nddma == NULL){
		g_nddma = (nand_dma *)kmalloc(sizeof(nand_dma),GFP_KERNEL);
		if(!g_nddma)
			GOTO_ERR(err0);

		memset(g_nddma,0,sizeof(nand_dma));
		g_nddma->chan_type = chan_type;
		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE,mask);
		g_nddma->msg_chan = dma_request_channel(mask,filter,g_nddma);
		if(!g_nddma->msg_chan)
			GOTO_ERR(err1);

		/*alloc msg memory*/
		g_nddma->msg_init = (struct taskmsg_init *)dma_alloc_coherent(g_nddma->msg_chan->device->dev,
				sizeof(struct taskmsg_init),&g_nddma->msginit_phyaddr,GFP_ATOMIC);
		if(!g_nddma->msg_init)
			GOTO_ERR(err2);

		ndd_debug("***** msginit_phyaddr = 0x%x ********** \n",g_nddma->msginit_phyaddr);
		init_completion(&comp);

		g_nddma->data = data;
		g_nddma->suspend_flag = 0;
		g_nddma->resume_flag = 0;

		ret = mcu_init(g_nddma,nandtask,id);
		if(ret != SUCCESS)
			GOTO_ERR(err3);

		ndd_print(NDD_INFO, "INFO: Nand DMA ops init success!\n");
	}
	return g_nddma;
	ERR_LABLE(err3):
		dma_free_coherent(g_nddma->msg_chan->device->dev,
				sizeof(struct taskmsg_init),g_nddma->msg_init,g_nddma->msginit_phyaddr);
	ERR_LABLE(err2):
		dma_release_channel(g_nddma->msg_chan);
	ERR_LABLE(err1):
		kfree(g_nddma);
	ERR_LABLE(err0):
		return NULL;
}

int dma_msg_handle_suspend(int msghandler)
{
	if (g_nddma->suspend_flag)
		return 0;

	g_nddma->suspend_flag = 1;
	g_nddma->resume_flag = 0;

	return 0;
}

int dma_msg_handle_resume(int msghandler, Nand_Task *nandtask,int id)
{
	if (g_nddma->resume_flag)
		return 0;

	g_nddma->resume_flag = 1;
	g_nddma->suspend_flag = 0;

	msleep(5);
	return mcu_init(g_nddma, nandtask, id);
}

int dma_msg_handle_init(nand_data *data, Nand_Task *nandtask, int id)
{
	/* we default only pdma's unit */
	struct msg_handler *handler;
	nand_dma *nd_dma;

	handler = (struct msg_handler *)kmalloc(sizeof(struct msg_handler), GFP_KERNEL);
	if(!handler)
		GOTO_ERR(out0);

	nd_dma = dma_ops_init(data,nandtask,id);
	if(!nd_dma)
		GOTO_ERR(out1);

	handler->context = (int)nd_dma;
	handler->handler = dma_handle_msg;

	ndd_print(NDD_INFO, "INFO: Nand DMA ops init success!\n");
	return (int)handler;
	ERR_LABLE(out1):
		kfree(handler);
	ERR_LABLE(out0):
		return 0;
}
void dma_msg_handle_deinit(int msghandler)
{
	struct msg_handler *handler = (struct msg_handler *)msghandler;
	if(g_nddma){
		if(g_nddma->msg_chan) {
			dmaengine_terminate_all(g_nddma->msg_chan);
		}
		dma_free_coherent(g_nddma->msg_chan->device->dev,
				sizeof(struct taskmsg_init),g_nddma->msg_init,g_nddma->msginit_phyaddr);
		dma_release_channel(g_nddma->msg_chan);
		kfree(g_nddma);
		g_nddma = NULL;
	}
	kfree(handler);
}
