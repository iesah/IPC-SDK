/*
 * Setting up the xburst2 system ost, including:
 *		---->	core system ost;
 *		---->	global system ost.
 *
 * Copyright (C) 2016 Ingenic Semiconductor Inc.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

/* #define DEBUG */
/* #define VERBOSE_DEBUG */

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/clockchips.h>
#include <linux/clk.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/delay.h>

#include <core_base.h>
#include <irq_cpu.h>
#include <ccu.h>

#define OSTCCR 0x00
#define OSTER  0x04
#define OSTCR  0x08
#define OSTFR  0x0C
#define OSTMR  0x10
#define OSTDFR 0x14
#define OSTCNT 0x18

#define G_OSTCCR   0x00
#define G_OSTER    0x04
#define G_OSTCR    0x08
#define G_OSTCNTH  0x0C
#define G_OSTCNTL  0x10
#define G_OSTCNTB  0x14

struct core_ost_address{
	unsigned int cpu_num;
	unsigned int dev_base;
	const char* clk_name;
};
struct core_global_ost_resource{
	unsigned long base;
	char *clk_name;
};

static struct core_global_ost_resource core_global_ost = {
	.base = G_OST_IOBASE,
	.clk_name = "global_ost",
};
#if (NR_CPUS > 4)
#error "core ost max support 4."
#endif
static struct core_ost_address core_ost_addr[4]={
	[0]={
		.cpu_num = 0,
		.dev_base = CORE_OST_IOBASE + 0 * 0x100,
		.clk_name = "ost1",
	},
	[1]={
		.cpu_num = 1,
		.dev_base = CORE_OST_IOBASE + 1 * 0x100,
		.clk_name = "ost2",
	},
	[2]={
		.cpu_num = 2,
		.dev_base = CORE_OST_IOBASE + 2 * 0x100,
		.clk_name = "ost3",
	},
	[3]={
		.cpu_num = 3,
		.dev_base = CORE_OST_IOBASE + 3 * 0x100,
		.clk_name = "ost4",
	},
};
struct core_timerevent {
	struct clock_event_device clkevt;
	struct irqaction evt_action;
	struct clk *clk_gate;
	void __iomem *iobase;
	unsigned int rate;
	unsigned int ops_mode;
	raw_spinlock_t	lock;
	unsigned int prev_set;
};

static struct core_timerevent timerevent_buf[NR_CPUS];
#define CLK_DIV  1
#define CSRDIV(x)      ({int n = 0;int d = x; while(d){ d >>= 2;n++;};(n-1);})

#define ost_readl(reg)			readl(reg)
#define ost_writel(value,reg)	writel(value, reg)

struct tmr_src {
	struct clocksource cs;
	struct core_global_ost_resource *resource;
	struct clk *clk_gate;
	void __iomem *iobase;
	raw_spinlock_t	lock;
};
#ifdef CONFIG_SMP
#define ost_lock(lock,flags) raw_spin_lock_irqsave(&(lock),flags)
#define ost_unlock(lock,flags) raw_spin_unlock_irqrestore(&(lock),flags)
#else
#define ost_lock(lock,flags) (flags = 0)
#define ost_unlock(lock,flags) (flags = 0)
#endif

struct core_ost_chip{
	struct core_timerevent* __percpu *percpu_timerevent;
	struct tmr_src *tmrsrc;
	unsigned int rate;
}core_ost;


static cycle_t core_clocksource_get_cycles(struct clocksource *cs)
{
	union clycle_type {
		cycle_t cycle64;
		unsigned int cycle32[2];
	} cycle;
	struct tmr_src *tmr = container_of(cs,struct tmr_src,cs);
	void __iomem *iobase = tmr->iobase;

	do {
		cycle.cycle32[1] = ost_readl(iobase + G_OSTCNTH);
		cycle.cycle32[0] = ost_readl(iobase + G_OSTCNTL);
	} while(cycle.cycle32[1] != ost_readl(iobase + G_OSTCNTH));

	return cycle.cycle64;
}

static int tmr_src_enable(struct clocksource *cs)
{
	struct tmr_src *tmr = container_of(cs,struct tmr_src,cs);
	void __iomem *iobase = tmr->iobase;
	unsigned long flags;
	ost_lock(tmr->lock,flags);
	ost_writel(1,iobase + G_OSTER);
	ost_unlock(tmr->lock,flags);
	return 0;
}

static 	void tmr_src_disable(struct clocksource *cs)
{
	struct tmr_src *tmr = container_of(cs,struct tmr_src,cs);
	void __iomem *iobase = tmr->iobase;
	unsigned long flags;
	ost_lock(tmr->lock,flags);
	ost_writel(0,iobase + G_OSTER);
	ost_unlock(tmr->lock,flags);
}
static	void tmr_src_suspend(struct clocksource *cs)
{
	struct tmr_src *tmr = container_of(cs,struct tmr_src,cs);
	if(tmr->clk_gate)
		clk_disable_unprepare(tmr->clk_gate);
}
static void tmr_src_resume(struct clocksource *cs)
{
	struct tmr_src *tmr = container_of(cs,struct tmr_src,cs);
	if(tmr->clk_gate)
		clk_prepare_enable(tmr->clk_gate);
}
static struct tmr_src tmr_src = {
	.cs = {
		.name 		= "jz_clocksource",
		.rating		= 400,
		.read		= core_clocksource_get_cycles,
		.mask		= 0x7FFFFFFFFFFFFFFFULL,
		.shift 		= 10,
		.flags		= CLOCK_SOURCE_WATCHDOG | CLOCK_SOURCE_IS_CONTINUOUS,
		.enable         = tmr_src_enable,
		.disable        = tmr_src_disable,
		.suspend        = tmr_src_suspend,
		.resume         = tmr_src_resume,
	}
};

unsigned long long sched_clock(void)
{
	if(!tmr_src.iobase)
		return 0LL;
	if(!ost_readl(tmr_src.iobase + G_OSTER))
		return 0LL;
	return ((cycle_t)core_clocksource_get_cycles(&tmr_src.cs) * tmr_src.cs.mult) >> tmr_src.cs.shift;
}

void __init core_clocksource_init(void)
{
	struct tmr_src *tmrsrc = &tmr_src;
	void __iomem *iobase;
	printk("core_clocksource_init begin.\n");
	core_ost.tmrsrc = tmrsrc;
	tmrsrc->resource = &core_global_ost;
	tmrsrc->cs.mult = clocksource_hz2mult(core_ost.rate, tmrsrc->cs.shift);

#if 0
	tmrsrc->clk_gate = clk_get(NULL,tmrsrc->resource->clk_name);

	if(IS_ERR(tmrsrc->clk_gate)) {
		tmr_src.clk_gate = NULL;
		printk("warning: ost clk get fail!,name:%s\n",tmrsrc->resource->clk_name);
	}
	if(tmrsrc->clk_gate)
		clk_prepare_enable(tmrsrc->clk_gate);
#endif
	iobase = ioremap(tmrsrc->resource->base,0xff);
	if(!iobase){
		pr_err("ERROR: clock source iobase[0x%08lx] map error!\n",tmrsrc->resource->base);
	}
	tmrsrc->iobase = iobase;
	raw_spin_lock_init(&tmrsrc->lock);
	ost_writel(0,iobase + G_OSTER);
	ost_writel(CSRDIV(CLK_DIV),iobase + OSTCCR);
	ost_writel(1,iobase + G_OSTCR);
	clocksource_register(&tmrsrc->cs);
	printk("core_clocksource_init success.\n");
}

static int core_timerevent_set_next_event(unsigned long evt,
			     struct clock_event_device *clk_evt_dev)
{
	struct core_timerevent *evt_dev = container_of(clk_evt_dev, struct core_timerevent, clkevt);
	void __iomem *iobase = evt_dev->iobase;
	unsigned long flags;
	ost_lock(evt_dev->lock,flags);
	if(evt <= 1) {
		WARN_ON(1);
		evt = 2;
	}
	ost_writel(0,iobase + OSTER);
	if(!ost_readl(iobase + OSTFR))
	{
		ost_writel(evt,iobase + OSTDFR);
		ost_writel(1,iobase + OSTCR);
		ost_writel(1,iobase + OSTER);
	}else{
		//pr_err("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO evt = %ld\n",evt);
		evt_dev->prev_set = evt;
	}

	ost_unlock(evt_dev->lock,flags);
	return 0;
}

static void core_timerevent_set_mode(enum clock_event_mode mode,
			struct clock_event_device *clkevt)
{
	struct core_timerevent *evt_dev = container_of(clkevt,struct core_timerevent,clkevt);
	void __iomem *iobase = evt_dev->iobase;
	unsigned int latch = (evt_dev->rate + (HZ >> 1)) / HZ;
	unsigned long flags;
	ost_lock(evt_dev->lock,flags);
	evt_dev->ops_mode = mode;
	switch (mode) {
		case CLOCK_EVT_MODE_PERIODIC:
			if(evt_dev->clk_gate)
				clk_prepare_enable(evt_dev->clk_gate);
			ost_writel(latch,iobase + OSTDFR);
			ost_writel(1,iobase + OSTER);
			break;
		case CLOCK_EVT_MODE_ONESHOT:
			ost_writel(latch,iobase + OSTDFR);
			ost_writel(1,iobase + OSTER);
			break;
		case CLOCK_EVT_MODE_UNUSED:
		case CLOCK_EVT_MODE_SHUTDOWN:
			ost_writel(0,iobase + OSTER);
			if(evt_dev->clk_gate)
				clk_disable_unprepare(evt_dev->clk_gate);
			break;

		case CLOCK_EVT_MODE_RESUME:
			ost_writel(1,iobase + OSTER);
			if(evt_dev->clk_gate)
				clk_prepare_enable(evt_dev->clk_gate);
			break;
	}
	ost_unlock(evt_dev->lock,flags);
}
static int time_count[4] = {100,100,100,100};
static irqreturn_t core_timerevent_interrupt(int irq, void *dev_id)
{
	struct core_timerevent *evt_dev =  *(struct core_timerevent **)dev_id;
	void __iomem *iobase = evt_dev->iobase;
	int cpu = smp_processor_id();
	unsigned long flags;
	if (ost_readl(iobase + OSTFR)){
		if(time_count[cpu] >= 100) {
#if 0
			printk("\n\n---timer interrupt , 100 count, on cpu: %d %x %x\n", cpu,read_c0_status(),read_c0_cause());
			 printk("intc src0:%x: %x\n", 0xb2300000 + cpu * 0x1000, *(volatile unsigned int *)(0xb2300000 + cpu * 0x1000 ));
			 printk("intc msk0:%x: %x\n", 0xb2300004 + cpu * 0x1000, *(volatile unsigned int *)(0xb2300004 + cpu * 0x1000 ));
			 printk("intc pnd0:%x: %x\n", 0xb2300010 + cpu * 0x1000, *(volatile unsigned int *)(0xb2300010 + cpu * 0x1000 ));
			 printk("intc src1:%x: %x\n", 0xb2300020 + cpu * 0x1000, *(volatile unsigned int *)(0xb2300020 + cpu * 0x1000 ));
			 printk("intc msk1:%x: %x\n", 0xb2300024 + cpu * 0x1000, *(volatile unsigned int *)(0xb2300024 + cpu * 0x1000 ));
			 printk("intc pnd1:%x: %x\n", 0xb2300030 + cpu * 0x1000, *(volatile unsigned int *)(0xb2300030 + cpu * 0x1000 ));
			 printk("ccu  msk: %x: %x\n", 0xb2200120, *(volatile unsigned int *)(0xb2200120));
			 printk("ccu  pnd: %x: %x\n", 0xb2200100, *(volatile unsigned int *)(0xb2200100));

			printk("uart0 ier: %x\n", *(volatile unsigned int *)0xb0030004);
			printk("uart0 isr: %x\n", *(volatile unsigned int *)0xb0030014);

			 printk("\n\n");
#endif
			time_count[cpu] = 0;
		}
		time_count[cpu]++;
		ost_lock(evt_dev->lock,flags);
		ost_writel(0,iobase + OSTFR);
		if (evt_dev->ops_mode == CLOCK_EVT_MODE_ONESHOT) {
			if(evt_dev->prev_set == 0){
				ost_writel(0,iobase + OSTER);
			}else{
				ost_writel(evt_dev->prev_set,iobase + OSTDFR);
				ost_writel(1,iobase + OSTCR);
				ost_writel(1,iobase + OSTER);
				evt_dev->prev_set = 0;
			}

		}
		ost_unlock(evt_dev->lock,flags);
		evt_dev->clkevt.event_handler(&evt_dev->clkevt);
	}else{
		pr_err("\nERROR: cpu: %d c0_status:%x c0_cause:%x\n", cpu,read_c0_status(),read_c0_cause());
		pr_err("CPU[%d]: ost couldn't find ostfr.\n",cpu);
	}
	return IRQ_HANDLED;
}
void __cpuinit percpu_timerevent_init(int cpu_num)
{
	int i;
	void __iomem *iobase;
	unsigned int base = 0;
	const char *clk_name;
	struct core_timerevent *timerevent = NULL;
	struct clock_event_device *cd = NULL;
	printk("percpu cpu_num:%d timeevent init\n",cpu_num);
	for(i = 0;i < NR_CPUS;i++)
	{
		if(core_ost_addr[i].cpu_num == cpu_num) {
			base = core_ost_addr[i].dev_base;
			clk_name = core_ost_addr[i].clk_name;
			timerevent = &timerevent_buf[i];
			break;
		}
	}

	if(base == 0){
		pr_err("Error: CPU[%d] don't finded ost base address\n",cpu_num);
		return;
	}
	if(timerevent->iobase == NULL)
	{
		iobase = ioremap(base,0xff);
		timerevent->iobase = iobase;
		timerevent->rate = core_ost.rate;

		raw_spin_lock_init(&timerevent->lock);
#if 0
		timerevent->clk_gate = clk_get(NULL,clk_name);
		if(IS_ERR(timerevent->clk_gate)){
			timerevent->clk_gate = NULL;
			pr_err("Error: cpu[%d] ost cannot finded clk[%s]\n",cpu_num,clk_name);
		}
#endif
		cd = &timerevent->clkevt;
		memset(cd,0,sizeof(struct clock_event_device));
		cd->name = "clockenvent";
		cd->features = CLOCK_EVT_FEAT_ONESHOT | CLOCK_EVT_FEAT_PERIODIC;
		cd->shift = 10;
		cd->rating = 400;
		cd->set_mode = core_timerevent_set_mode;
		cd->set_next_event = core_timerevent_set_next_event;
		cd->irq = CORE_SYS_OST_IRQ;
		cd->cpumask = cpumask_of(smp_processor_id());
	}else {
		iobase = timerevent->iobase;
		cd = &timerevent->clkevt;
	}
	if(timerevent->clk_gate)
		clk_prepare_enable(timerevent->clk_gate);
	ost_writel(0,iobase + OSTER);  //disable ost
	ost_writel(CSRDIV(CLK_DIV),iobase + OSTCCR);
	ost_writel(1,iobase + OSTCR);
	ost_writel(0,iobase + OSTMR);

	clockevents_config_and_register(cd, timerevent->rate, 0xf, 0xffffffff);

	*__this_cpu_ptr(core_ost.percpu_timerevent) = timerevent;
	enable_percpu_irq(CORE_SYS_OST_IRQ, IRQ_TYPE_NONE);
	printk("clockevents_config_and_register success.\n");
}

void percpu_timerevent_deinit(void)
{
	struct core_timerevent *timerevent = *__this_cpu_ptr(core_ost.percpu_timerevent);
	disable_percpu_irq(CORE_SYS_OST_IRQ);
	ost_writel(0,timerevent->iobase + OSTER);  //disable ost
	ost_writel(1,timerevent->iobase + OSTMR);
	if(timerevent->clk_gate)
		clk_disable_unprepare(timerevent->clk_gate);
}
static void __init core_ost_setup(void)
{
	int ret;
	struct clk *extclk = clk_get(NULL, "ext1");
	if(IS_ERR(extclk)){
		pr_err("Error: Ost get ext1 clk failure!\n");
		BUG_ON(1);
	}

	//core_ost.rate = (clk_get_rate(extclk) / CLK_DIV * 13);   /* TODO: should be clk_get_rate(extclk) / CLK_DIV in real chip */
	core_ost.rate = (24000000 / CLK_DIV);   /* TODO: should be clk_get_rate(extclk) / CLK_DIV in real chip */
	clk_put(extclk);

	core_ost.percpu_timerevent = alloc_percpu(struct core_timerevent *);
	if(!core_ost.percpu_timerevent) {
		pr_err("ost percpu alloc fail!\n");
		return;
	}
	ret = request_percpu_irq(CORE_SYS_OST_IRQ,core_timerevent_interrupt,"core_timerevent",core_ost.percpu_timerevent);
	if(ret < 0) {
		pr_err("dddd timer request ost error %d \n",ret);
	}
	core_ost.tmrsrc = &tmr_src;
	memset(timerevent_buf,0,sizeof(timerevent_buf));
	printk("percpu_timerevent_init success.\n");
}

void __init xburst2_timer_setup(void)
{
	core_ost_setup();
	percpu_timerevent_init(0);
	core_clocksource_init();
}
