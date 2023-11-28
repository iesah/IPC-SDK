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

#include <soc/base.h>
#include <soc/extal.h>
#include <soc/ost.h>
#include <soc/tcu.h>
#include <soc/irq.h>

#define APB_OST_BASE   0x10002000
#define  OSTCSR        0xec
#define  OSTDR         0xe0
#define  OSTCNTH       0xe8
#define  OSTCNTL       0xe4
#define  OSTCNTHBUF    0xfc

#define ost_readl(reg)			inl(APB_OST_BASE + reg)
#define ost_writel(reg,value)		outl(value, APB_OST_BASE + reg)
#define tcu_readl(reg)			inl(TCU_IOBASE + reg)
#define tcu_writel(reg,value)		outl(value, TCU_IOBASE + reg)
#define OST_TIMER_BIT    15
#define SYS_TIMER_CLK			(JZ_EXTAL / 16)

static int curmode;
static DEFINE_SPINLOCK(timer_lock);
static int jz_set_next_event(unsigned long evt,
			     struct clock_event_device *unused);
static void jz_set_mode(enum clock_event_mode mode,
			struct clock_event_device *evt);

static struct clock_event_device jz_clockevent_device = {
	.name		= "jz-clockenvent",
	.features	= CLOCK_EVT_FEAT_ONESHOT | CLOCK_EVT_FEAT_PERIODIC,
	.shift          = 10,
	.rating		= 400,
	.irq		= IRQ_TCU0,
	.set_mode	= jz_set_mode,
	.set_next_event	= jz_set_next_event,
};

static int jz_set_next_event(unsigned long evt,
			     struct clock_event_device *unused)
{
	unsigned long flags;
	spin_lock_irqsave(&timer_lock,flags);
	ost_writel(OSTCNTL, 0);
	ost_writel(OSTCNTH, 0);
	ost_writel(OSTDR, evt - 1);
	tcu_writel(TCU_TMCR, (1 << OST_TIMER_BIT));
	tcu_writel(TCU_TESR, (1 << OST_TIMER_BIT));
	spin_unlock_irqrestore(&timer_lock,flags);
//	printk("1 jz_set_next_event = %ld\n",evt);
	return 0;
}

static void jz_set_mode(enum clock_event_mode mode,
			struct clock_event_device *evt)
{
	unsigned long flags;
	unsigned int latch = (SYS_TIMER_CLK + (HZ >> 1)) / HZ;
//	printk("%s %d mode = %d\n",__FILE__,__LINE__,mode);
	spin_lock_irqsave(&timer_lock,flags);
	curmode = mode;
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		ost_writel(OSTCNTL, 0);
		ost_writel(OSTCNTH, 0);
		ost_writel(OSTDR, latch - 1);
		tcu_writel(TCU_TFCR, (1 << OST_TIMER_BIT));
		tcu_writel(TCU_TMCR, (1 << OST_TIMER_BIT));
		tcu_writel(TCU_TESR, (1 << OST_TIMER_BIT));
                break;
        case CLOCK_EVT_MODE_ONESHOT:
		break;
        case CLOCK_EVT_MODE_UNUSED:
        case CLOCK_EVT_MODE_SHUTDOWN:
		tcu_writel(TCU_TECR, (1 << OST_TIMER_BIT));
		tcu_writel(TCU_TMSR, (1 << OST_TIMER_BIT));
		tcu_writel(TCU_TFCR, (1 << OST_TIMER_BIT));
                break;

        case CLOCK_EVT_MODE_RESUME:
		tcu_writel(TCU_TFCR, (1 << OST_TIMER_BIT));
		tcu_writel(TCU_TMCR, (1 << OST_TIMER_BIT));
		tcu_writel(TCU_TESR, (1 << OST_TIMER_BIT));
		
                break;
        }
	spin_unlock_irqrestore(&timer_lock,flags);
		
}



static irqreturn_t jz_cpu1timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *cd = dev_id;
//	int cpu=smp_processor_id();
	if(tcu_readl(TCU_TFR) & (1 << OST_TIMER_BIT)) {
		tcu_writel(TCU_TFCR, (1 << OST_TIMER_BIT));
		if(curmode == CLOCK_EVT_MODE_ONESHOT) {
			tcu_writel(TCU_TECR, (1 << OST_TIMER_BIT));
			tcu_writel(TCU_TMSR, (1 << OST_TIMER_BIT));
		}
	}
	cd->event_handler(cd);

	return IRQ_HANDLED;
}

void jz_cpu1_clockevent_init(void)
{
  	unsigned int latch = (SYS_TIMER_CLK + (HZ >> 1)) / HZ;
	int ret;
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *cd = &jz_clockevent_device;
	tcu_writel(TCU_TECR, (1 << OST_TIMER_BIT));

	ret = request_irq(IRQ_TCU0, jz_cpu1timer_interrupt,
			  IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER,
			  "jz-timerirq",
			  &jz_clockevent_device);
	if (ret < 0) {
		pr_err("timer request irq error\n");
		BUG();
	}	

	
	tcu_writel(TCU_TMSR, (1 << OST_TIMER_BIT));

	ost_writel(OSTCSR, CSR_DIV16 | CSR_EXT_EN);

	ost_writel(OSTCNTL, 0);
	ost_writel(OSTCNTH, 0);
	ost_writel(OSTDR, latch - 1);
/*
	cd->mult =
		clocksource_hz2mult(SYS_TIMER_CLK, cd->shift);
	cd->min_delta_ticks = 1;
	cd->max_delta_ticks = 0xfffe;
	cd->cpumask = cpumask_of(cpu);
	clockevents_register_device(cd);
*/
	cd->cpumask = cpumask_of(cpu);
	clockevents_config_and_register(cd,SYS_TIMER_CLK,4,65530);

}
