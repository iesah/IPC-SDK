/*
 * linux/arch/mips/jz4760/time.c
 *
 * Setting up the clock on the JZ4760 boards.
 *
 * Copyright (C) 2008 Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
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
#include <linux/time.h>
#include <linux/clockchips.h>
#include <linux/clk.h>
#include <linux/notifier.h>
#include <linux/cpu.h>

#include <soc/base.h>
#include <soc/extal.h>
#include <soc/ost.h>
#include <soc/tcu.h>
#include <soc/irq.h>

#define CLKSOURCE_DIV   16
#define CLKEVENT_DIV    64
#define CSRDIV(x)      ({int n = 0;int d = x; while(d){ d >>= 2;n++;};(n-1) << 3;})

#define ost_readl(reg)			inl(OST_IOBASE + reg)
#define ost_writel(reg,value)		outl(value, OST_IOBASE + reg)
static cycle_t jz_get_cycles(struct clocksource *cs);
static struct clocksource clocksource_jz = {
	.name 		= "jz_clocksource",
	.rating		= 400,
	.read		= jz_get_cycles,
	.mask		= 0x7FFFFFFFFFFFFFFFULL,
	.shift 		= 10,
	.flags		= CLOCK_SOURCE_WATCHDOG | CLOCK_SOURCE_IS_CONTINUOUS,
};

static cycle_t jz_get_cycles(struct clocksource *cs)
{
	union clycle_type
	{
		cycle_t cycle64;
		unsigned int cycle32[2];
	} cycle;

	do{
		cycle.cycle32[1] = ost_readl(OST_CNTH);
		cycle.cycle32[0] = ost_readl(OST_CNTL);
	}while(cycle.cycle32[1] != ost_readl(OST_CNTH));

	return cycle.cycle64;
}

unsigned long long sched_clock(void)
{
	return ((cycle_t)jz_get_cycles(0) * clocksource_jz.mult) >> clocksource_jz.shift;
}

void __cpuinit jz_clocksource_init(void)
{
	struct clk *ext_clk = clk_get(NULL,"ext1");
	ost_writel(OST_CNTL, 0);
	ost_writel(OST_CNTH, 0);
	ost_writel(OST_DR, 0);

	ost_writel(OST_TFCR, TFR_OSTF);
	ost_writel(OST_TMSR, TMR_OSTM);
	ost_writel(OST_TSCR, TSR_OSTS);

	ost_writel(OST_CSR, OSTCSR_CNT_MD);
	ost_writel(OST_TCSR, CSRDIV(CLKSOURCE_DIV));
	ost_writel(OST_TESR, OST_EN);
	clocksource_jz.mult =
		clocksource_hz2mult(clk_get_rate(ext_clk) / CLKSOURCE_DIV, clocksource_jz.shift);
	clk_put(ext_clk);
	clocksource_register(&clocksource_jz);
}
enum timestate{
	INIT,
	FINI,
	STOP,
};
struct jz_timerevent {
	unsigned int count_addr;
	unsigned int latch_addr;
	unsigned int config_addr;
	unsigned int ctrl_addr;
	unsigned int ch;
	unsigned int irq;
	unsigned int requestirq;
	int curmode;
	spinlock_t lock;
	unsigned int rate;
	struct clock_event_device clkevt;
	struct notifier_block  cpu_notify;
	struct irqaction evt_action;
	int cpu;
	enum timestate state;
};
static DEFINE_PER_CPU(struct jz_timerevent, jzclockevent);

static inline void stoptimer(struct jz_timerevent *tc) {
	outl((1 << tc->ch),tc->ctrl_addr + TCU_TECR);
	outl((1 << tc->ch),tc->ctrl_addr + TCU_TFCR);
}
static inline void restarttimer(struct jz_timerevent *tc) {
       	outl((1 << tc->ch),tc->ctrl_addr + TCU_TFCR);
	outl((1 << tc->ch),tc->ctrl_addr + TCU_TESR);
}
static inline void resettimer(struct jz_timerevent *tc,int count) {
	outl(count,tc->latch_addr);
	outl(0,tc->count_addr);
	outl((1 << tc->ch),tc->ctrl_addr + TCU_TFCR);
	outl((1 << tc->ch),tc->ctrl_addr + TCU_TESR);
}
static int jz_set_next_event(unsigned long evt,
			     struct clock_event_device *clk_evt_dev)
{
	struct jz_timerevent *evt_dev = container_of(clk_evt_dev,struct jz_timerevent,clkevt);
	unsigned long flags;
	//int cpu = smp_processor_id();
	spin_lock_irqsave(&evt_dev->lock,flags);
	if(evt <= 1) {
		WARN_ON(1);
		evt = 2;
	}
	resettimer(evt_dev,evt - 1);
	spin_unlock_irqrestore(&evt_dev->lock,flags);
	return 0;
}
static void jz_set_mode(enum clock_event_mode mode,
			struct clock_event_device *clkevt)
{
	struct jz_timerevent *evt_dev = container_of(clkevt,struct jz_timerevent,clkevt);
	unsigned long flags;
	unsigned int latch = (evt_dev->rate + (HZ >> 1)) / HZ;
	spin_lock_irqsave(&evt_dev->lock,flags);
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		evt_dev->curmode = mode;
		resettimer(evt_dev,latch - 1);
                break;
        case CLOCK_EVT_MODE_ONESHOT:
		evt_dev->curmode = mode;
		break;
        case CLOCK_EVT_MODE_UNUSED:
        case CLOCK_EVT_MODE_SHUTDOWN:
		stoptimer(evt_dev);
                break;

        case CLOCK_EVT_MODE_RESUME:
		restarttimer(evt_dev);
                break;
        }
	spin_unlock_irqrestore(&evt_dev->lock,flags);
}
#ifdef CONFIG_SMP
void jz_cputimer_interrupt(void) {
	int cpu = smp_processor_id();
	struct jz_timerevent *evt = &per_cpu(jzclockevent, cpu);
	evt->clkevt.event_handler(&evt->clkevt);
}
void smp_cputimer_broadcast(int cpu);
void jz_set_cpu_affinity(unsigned long irq,int cpu);
#endif

#ifdef CONFIG_EMERGENCY_MSG
int is_emergency_msg(void *handle);
void deinit_emergency_msg(void *handle);
void emergency_msg_outlog(void *handle);
extern void *emergency_msg;
#endif
static irqreturn_t jz_timer_interrupt(int irq, void *dev_id)
{
	struct jz_timerevent *evt_dev = dev_id;
	int ctrl_addr = evt_dev->ctrl_addr;
	int ctrlbit = 1 << evt_dev->ch;
#ifdef CONFIG_EMERGENCY_MSG
	//int cpu = smp_processor_id();
	if(emergency_msg){
		if(is_emergency_msg(emergency_msg)) {
			emergency_msg_outlog(emergency_msg);
		}
	}
#endif
	if(inl(ctrl_addr + TCU_TFR) & ctrlbit) {
		outl(ctrlbit,ctrl_addr + TCU_TFCR);
		if(evt_dev->curmode == CLOCK_EVT_MODE_ONESHOT) {
			stoptimer(evt_dev);
		}
		if(evt_dev->state == INIT && evt_dev->cpu) {
#ifdef CONFIG_SMP
			smp_cputimer_broadcast(evt_dev->cpu);
#endif
		}
		if(evt_dev->state == FINI) {
			evt_dev->clkevt.event_handler(&evt_dev->clkevt);
		}

	}
	return IRQ_HANDLED;
}
/*
static int broadcast_cpuhp_notify(struct notifier_block *n,
					unsigned long action, void *hcpu){
	int hotcpu = (unsigned long)hcpu;
	struct jz_timerevent *evt = &per_cpu(jzclockevent, hotcpu);
	if(hotcpu != 0) {
		switch(action & 0xf) {
		case CPU_DEAD:
			jz_set_mode(CLOCK_EVT_MODE_SHUTDOWN,&evt->clkevt);
			break;
		case CPU_ONLINE:
			jz_set_mode(CLOCK_EVT_MODE_RESUME,&evt->clkevt);
			break;
		}
	}
	return NOTIFY_OK;
}
*/
static void jz_clockevent_init(struct jz_timerevent *evt_dev,int cpu) {
	struct clock_event_device *cd = &evt_dev->clkevt;
	struct clk *ext_clk = clk_get(NULL,"ext1");

	spin_lock_init(&evt_dev->lock);

	evt_dev->rate = clk_get_rate(ext_clk) / CLKEVENT_DIV;
	clk_put(ext_clk);
       	stoptimer(evt_dev);
	outl((1 << evt_dev->ch),evt_dev->ctrl_addr + TCU_TMCR);
	outl(CSRDIV(CLKEVENT_DIV) | CSR_EXT_EN,evt_dev->config_addr);
	if(evt_dev->requestirq == 0){
		evt_dev->evt_action.handler = jz_timer_interrupt;
		evt_dev->evt_action.thread_fn = NULL;
		evt_dev->evt_action.flags = IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER;
		evt_dev->evt_action.name = "jz-timerirq";
		evt_dev->evt_action.dev_id = (void*)evt_dev;

		if(setup_irq(evt_dev->irq, &evt_dev->evt_action) < 0) {
			pr_err("timer request irq error\n");
			BUG();
		}
		evt_dev->requestirq = 1;
	}

	memset(cd,0,sizeof(struct clock_event_device));
	cd->name = "jz-clockenvent";
	cd->features = CLOCK_EVT_FEAT_ONESHOT | CLOCK_EVT_FEAT_PERIODIC;
	cd->shift = 10;
	cd->rating = 400;
	cd->set_mode = jz_set_mode;
	cd->set_next_event = jz_set_next_event;
	cd->irq = evt_dev->irq;
	cd->cpumask = cpumask_of(cpu);
	clockevents_config_and_register(cd,evt_dev->rate,4,65536);
/*
	evt_dev->cpu_notify.notifier_call = broadcast_cpuhp_notify;

	if(cpu == 0){
		register_cpu_notifier(&evt_dev->cpu_notify);
	}
*/
}

#define  APB_OST_IOBASE   0x10002000
#define  OSTCSR        0xec
#define  OSTDR         0xe0
#define  OSTCNTH       0xe8
#define  OSTCNTL       0xe4
#define  OSTCNTHBUF    0xfc

#define apbost_readl(reg)		inl(APB_OST_IOBASE + reg)
#define apbost_writel(reg,value)	outl(value, APB_OST_IOBASE + reg)
#define tcu_readl(reg)			inl(TCU_IOBASE + reg)
#define tcu_writel(reg,value)		outl(value, TCU_IOBASE + reg)

void __cpuinit jzcpu_timer_setup(void)
{
	int cpu = smp_processor_id();
	struct jz_timerevent *evt = &per_cpu(jzclockevent, cpu);
	evt->cpu = cpu;
	evt->state = INIT;
	switch(cpu) {
	case 0:
		evt->state = FINI;
		evt->ch = 5;
		evt->irq = IRQ_TCU1;
		evt->count_addr = TCU_IOBASE + CH_TCNT(evt->ch);
		evt->latch_addr = TCU_IOBASE + CH_TDFR(evt->ch);
		evt->ctrl_addr = TCU_IOBASE;
		evt->config_addr = TCU_IOBASE + CH_TCSR(evt->ch);
		tcu_writel(CH_TDHR(evt->ch), 0xffff);
		tcu_writel(TCU_TMSR, ((1 << evt->ch) | (1 << (evt->ch + 16))));

		break;
	case 1:
		evt->ch = 15;
		evt->irq = IRQ_TCU0;
		evt->count_addr = APB_OST_IOBASE + OSTCNTL;
		evt->latch_addr = APB_OST_IOBASE + OSTDR;
		evt->ctrl_addr = TCU_IOBASE;
		evt->config_addr = APB_OST_IOBASE + OSTCSR;
		apbost_writel(OSTCNTH, 0);
		break;
	}
	jz_set_cpu_affinity(evt->irq,0);
	jz_clockevent_init(evt,cpu);
}


#ifdef CONFIG_HOTPLUG_CPU
void jz_set_cpu_affinity(unsigned long irq,int cpu);
void percpu_timer_cpu_affinity(void)
{
	unsigned int cpu = smp_processor_id();
	struct jz_timerevent *evt = &per_cpu(jzclockevent, cpu);
	jz_set_cpu_affinity(evt->irq,cpu);
	evt->state = FINI;
}
void percpu_timer_stop(void)
{
	unsigned int cpu = smp_processor_id();
	struct jz_timerevent *evt = &per_cpu(jzclockevent, cpu);
	jz_set_cpu_affinity(evt->irq,0);
	//evt->state = INIT;
	evt->state = STOP;
	evt->clkevt.set_mode(CLOCK_EVT_MODE_UNUSED, &evt->clkevt);
}
#endif
