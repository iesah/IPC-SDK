
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/workqueue.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <jz_proc.h>
#include <linux/seq_file.h>
#include <asm/cpu.h>
#include <smp_cp0.h>
#include <jz_notifier.h>
/* int _regs_stack[64]; */
/* int _tlb_entry_regs[32 * 3 + 4]; */
/* int _ready_flag; */
/* int _wait_flag; */

extern long long save_goto(unsigned int);
extern void switch_cpu(void);
extern void _start_secondary(void);

static struct cpu_core_ctrl
{
	unsigned int coreid;
	unsigned int rate;
	unsigned int target_coreid; //temp use this var
	unsigned int up_limit;
	unsigned int down_limit;
	spinlock_t switch_lock;
	struct clk *clk[2];
	unsigned int used_sw_core;
	struct workqueue_struct *sw_core_q;
	struct work_struct work;
	struct jz_notifier clk_changing;
	struct jz_notifier clk_changed;
} sw_core = {
	.up_limit = 300*1000*1000,
	.down_limit = 150*1000*1000,
	.used_sw_core = 0,
};

#define core_clk_enable(clk,id)			\
	do{					\
		if(clk[id])			\
			clk_enable(clk[id]);	\
	}while(0)
#define core_clk_disable(clk,id)		\
	do{					\
		if(clk[id])			\
			clk_disable(clk[id]);	\
	}while(0)
static int reset_core(int cpuid)
{

	unsigned int ctrl;
	/*
	 * first we save the CTRL.RPC and set RPC = 1
	 * set SW_RSTX = 1 keep reset another cpu
	 */
	ctrl = get_smp_ctrl();
	ctrl |= (1 << cpuid);
	//printk("ctrl0 = %x\n",ctrl);
	set_smp_ctrl(ctrl);

	/*
	 * then start another cpu
	 * hardware request!!!
	 */
	ctrl = get_smp_ctrl();
	ctrl &= ~(1 << cpuid);
	//printk("ctrl0 = %x\n",ctrl);
	set_smp_ctrl(ctrl);
	//printk("status 0x%08x\n",get_smp_status());
	return 0;
}

static void stop_core(int cpuid)
{
	unsigned int tmp;
	tmp = get_smp_ctrl();
	tmp |= (1 << cpuid);
	set_smp_ctrl(tmp);
}
static void core_interrupt(int cpuid,int mask)
{
	unsigned int tmp;
	tmp = get_smp_reim();
	if(mask)
		tmp &= ~(1 << (cpuid + 8));
	else
		tmp |= 1 << (cpuid + 8);
	set_smp_reim(tmp);
	//printk("reim = %x\n",get_smp_reim());

}
static void switch_core_work(struct work_struct *work)
{
	unsigned long flags;
	int cpu_no;
	unsigned int tmp;
	struct cpu_core_ctrl *core = (struct cpu_core_ctrl *)container_of(work,struct cpu_core_ctrl,work);
	spin_lock_irqsave(&core->switch_lock, flags);
	tmp = get_smp_reim();
	tmp &= ~(3 << 8);
	set_smp_reim(tmp);

	//local_flush_tlb_all();
	save_goto((unsigned int)switch_cpu);
	spin_unlock_irqrestore(&core->switch_lock, flags);
	cpu_no = read_c0_ebase() & 1;
	if(core->target_coreid == cpu_no) {
		stop_core(core->coreid);
		core_clk_disable(core->clk,core->coreid);
		core_interrupt(core->target_coreid,0);
		core->coreid = core->target_coreid;
		//printk("convert to id %d\n",core->target_coreid);
	}else {
		printk("convert core fail!\n");
		BUG_ON(1);
	}
	//printk("after:current epc = 0x%x\n", (unsigned int)read_c0_epc());
	//printk("after:current lcr1 = 0x%x\n", *(unsigned int*)(0xb0000004));

}

static int clk_changing_notify(struct jz_notifier *notify,void *v)
{
	struct cpu_core_ctrl *core = container_of(notify,struct cpu_core_ctrl,clk_changing);
	struct clk_notify_data *clk_data = (struct clk_notify_data *)v;
	unsigned long target_rate = clk_data->target_rate;
	unsigned long cur_rate = clk_data->current_rate;
	core->target_coreid = -1;
	//printk("cur_coreid = %d\n", core->coreid);
	if(core->used_sw_core){
		if(core->coreid == 1){
			if(target_rate > core->up_limit && cur_rate <= core->up_limit) {
				core_interrupt(0,1);
				core_clk_enable(core->clk,0);
				reset_core(0);
				core->target_coreid = 0;
			}

		}else{
			if(target_rate < core->down_limit && cur_rate >= core->down_limit) {
				core_interrupt(1,1);
				core_clk_enable(core->clk,1);
				reset_core(1);
				core->target_coreid = 1;
			}
		}
	}
        return 0;
}
static int clk_changed_notify(struct jz_notifier *notify,void *v)
{
	struct cpu_core_ctrl *core = container_of(notify,struct cpu_core_ctrl,clk_changed);
	if(core->target_coreid != -1) {
		if(!(current->flags & PF_KTHREAD)) {
			printk("queue_work\n");
			queue_work(core->sw_core_q,&core->work);
			flush_work(&core->work);
		}else{
			switch_core_work(&core->work);
		}
	}
	return 0;
}
extern unsigned int _wait_flag;
extern unsigned int _ready_flag;
static int __init switch_cpu_init(struct cpu_core_ctrl *pcore)
{
	unsigned int scpu_start_addr = 0, i, j;
	unsigned int *tmp;
	unsigned long flags;
	pcore->sw_core_q = create_singlethread_workqueue("sw_core");
	if(!pcore->sw_core_q)
		return -ENOMEM;

	INIT_WORK(&pcore->work,switch_core_work);

#define MAX_REQUEST_ALLOC_SIZE 4096
	tmp = (unsigned int *)kmalloc(MAX_REQUEST_ALLOC_SIZE, GFP_KERNEL);

	local_irq_save(flags);
	for (i = 0; i < MAX_REQUEST_ALLOC_SIZE / 4; i++) {
		tmp[i] = __get_free_page(GFP_ATOMIC | GFP_DMA);
		if (tmp[i] && !(tmp[i] & 0xffff))
			break;
	}
	if (i < MAX_REQUEST_ALLOC_SIZE / 4)
		scpu_start_addr = tmp[i];
	for (j = 0; j < i; j++)
		free_page(tmp[j]);
	local_irq_restore(flags);

	pcore->clk[0] = clk_get(NULL, "p0");
	if (IS_ERR(pcore->clk[0])) {
		printk("get p0clk fail!\n");
		pcore->clk[0] = NULL;
	}

	pcore->clk[1] = clk_get(NULL, "p1");
	if (IS_ERR(pcore->clk[1])) {
		printk("get p1clk fail!\n");
		pcore->clk[1] = NULL;
	}
	core_clk_enable(pcore->clk,pcore->coreid);
	printk("the secondary cpu base addr is %x\n", scpu_start_addr);
	BUG_ON(scpu_start_addr == 0);
	kfree(tmp);
	memcpy((void *)scpu_start_addr, (void *)_start_secondary, 0x1000);

	/* we need flush L2CACHE */
	dma_cache_sync(NULL, (void *)scpu_start_addr, 0x1000, DMA_TO_DEVICE);
	dma_cache_sync(NULL, (void *)&_wait_flag, 32, DMA_TO_DEVICE);
	dma_cache_sync(NULL, (void *)&_ready_flag, 32, DMA_TO_DEVICE);
	{
		unsigned int ctrl;
		ctrl = get_smp_ctrl();
		ctrl |= (3 << 8);
		set_smp_ctrl(ctrl);
	}
	set_smp_reim((scpu_start_addr & 0xffff0000) | 0x1ff);
	spin_lock_init(&pcore->switch_lock);
	pcore->coreid = read_c0_ebase() & 1;

	pcore->clk_changing.jz_notify = clk_changing_notify;
	pcore->clk_changing.level = NOTEFY_PROI_HIGH;
	pcore->clk_changing.msg = JZ_CLK_CHANGING;
	jz_notifier_register(&pcore->clk_changing, NOTEFY_PROI_HIGH);

	pcore->clk_changed.jz_notify = clk_changed_notify;
	pcore->clk_changed.level = NOTEFY_PROI_HIGH;
	pcore->clk_changed.msg = JZ_CLK_CHANGED;
	jz_notifier_register(&pcore->clk_changed, NOTEFY_PROI_HIGH);

	return 0;
}
/* ------------------------------cpu switch proc------------------------------- */
static void get_str_from_user(unsigned char *str, int strlen,
			      const char *buffer, unsigned long count)
{
	int len = count > strlen-1 ? strlen-1 : count;
	int i;

	if(len == 0) {
		str[0] = 0;
		return;
	}
	copy_from_user(str,buffer,len);
	str[len] = 0;
	for(i = len;i >= 0;i--) {
		if((str[i] == '\r') || (str[i] == '\n'))
			str[i] = 0;
	}
}

#include <soc/base.h>
#define PART_OFF	0x20
#define IMR_OFF		(0x04)
/* #define DDR_AUTOSR_EN   (0xb34f0304) */
/* #define DDR_DDLP        (0xb34f00bc) */
/* #define REG32_ADDR(addr) *((volatile unsigned int*)(addr)) */
static void cpu_entry_wait(void)
{
	unsigned int imr0_val, imr1_val;
	unsigned int *imr0_addr, *imr1_addr;
	void __iomem *intc_base;

	intc_base = ioremap(INTC_IOBASE, 0xfff);
	if(!intc_base) {
		printk("cpu switch ioremap intc reg addr error\n");
		return;
	}

	imr0_addr = intc_base + IMR_OFF;
	imr1_addr = intc_base + PART_OFF + IMR_OFF;
	/* save interrupt */
	imr0_val = readl(imr0_addr);
	imr1_val = readl(imr1_addr);

	/* mask all interrupt except GPIO */
	writel(0xfffc0fff, imr0_addr);
	writel(0xffffffff, imr1_addr);
	/* REG32_ADDR(DDR_AUTOSR_EN) = 0; */
	/* udelay(100); */
	/* REG32_ADDR(DDR_DDLP) = 0x0000f003; */
	__asm__ __volatile__(
		"wait \n\t"
		);
	/* REG32_ADDR(DDR_DDLP) = 0; */
	/* udelay(100); */
	/* REG32_ADDR(DDR_AUTOSR_EN) = 1; */

	/* restore  interrupt */
	writel(imr0_val, imr0_addr);
	writel(imr1_val, imr1_addr);

	iounmap(intc_base);
}
static int cpu_wait_proc_write(struct file *file, const char __user *buffer,
			       size_t count,loff_t *data)
{
	char str[10];
	get_str_from_user(str,10,buffer,count);
	if(strlen(str) > 0) {
		if (strncmp(str, "wait", 5) == 0) {
			cpu_entry_wait();
		}
	}
	return count;
}
static int cpu_switch_proc_show(struct seq_file *m, void *v)
{
	int cpu_no, len;
	cpu_no = read_c0_ebase() & 1;
	len = seq_printf(m,"current cpu status is %s cpu\n", !cpu_no? "big" : "little");

	return len;
}

static int cpu_switch_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cpu_switch_proc_show, PDE_DATA(inode));
}

static int cpu_switch_write_proc(struct file *file, const char __user *buffer,size_t count, loff_t *data)
{
	struct cpu_core_ctrl *p = (struct cpu_core_ctrl *)data;
	if(count && (buffer[0] == '1'))
		p->used_sw_core = 1;
	else if(count && (buffer[0] == '0'))
		p->used_sw_core = 0;
	else
		printk("\"echo 1 > cpu_switch\" or \"echo 0 > cpu_switch \" ");
	return count;
}

static const struct file_operations cpu_switch_proc_fops ={
	.read = seq_read,
	.open = cpu_switch_proc_open,
	.write = cpu_switch_write_proc,
	.llseek = seq_lseek,
	.release = single_release,
};


static const struct file_operations cpu_wait_proc_fops ={
	.read = seq_read,
//	.open = clk_get_proc_open,
	.write = cpu_wait_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};
static int __init create_switch_core(void)
{
	struct proc_dir_entry *p;

	p = jz_proc_mkdir("cpu_switch");
	if (!p) {
		pr_warning("create_proc_entry for common cpu switch failed.\n");
		return -1;
	}
	if(switch_cpu_init(&sw_core) != 0){
		return -1;
	}

	proc_create("cpu_proc_switch", 0600,p,&cpu_switch_proc_fops);
	proc_create("cpu_proc_wait", 0600,p,&cpu_wait_proc_fops);

	return 0;
}
module_init(create_switch_core);
