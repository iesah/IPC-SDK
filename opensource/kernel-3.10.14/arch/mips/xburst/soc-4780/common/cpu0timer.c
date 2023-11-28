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

#define TCU_TIMER_CH			7
#define TCU_TIMER_CLOCK			(JZ_EXTAL / 16)

#define tcu_readl(reg)			inl(TCU_IOBASE + reg)
#define tcu_writel(reg,value)		outl(value, TCU_IOBASE + reg)

static int jz_set_next_event(unsigned long evt,
			     struct clock_event_device *unused)
{
	return 0;
}

static void jz_set_mode(enum clock_event_mode mode,
			struct clock_event_device *evt)
{
	printk("%s %d mode = %d\n",__FILE__,__LINE__,mode);
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
                break;
        case CLOCK_EVT_MODE_ONESHOT:
        case CLOCK_EVT_MODE_UNUSED:
        case CLOCK_EVT_MODE_SHUTDOWN:
                break;
        case CLOCK_EVT_MODE_RESUME:
                break;
        }
}

static struct clock_event_device jz_clockevent_device = {
	.name		= "jz-clockenvent",
	.features	= CLOCK_EVT_FEAT_ONESHOT |
			  CLOCK_EVT_FEAT_PERIODIC,
	.shift          = 10,
	.rating		= 300,
	.irq		= IRQ_TCU2,
	.set_mode	= jz_set_mode,
	.set_next_event	= jz_set_next_event,
};

static irqreturn_t jz_cpu0timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *cd = dev_id;
	if(tcu_readl(TCU_TFR) & (1 << TCU_TIMER_CH)) {
		tcu_writel(TCU_TFCR, (1 << TCU_TIMER_CH));
		cd->event_handler(cd);
	}else{
		printk("tfr = %x\n",tcu_readl(TCU_TFR));
	}
	return IRQ_HANDLED;
}

void jz_cpu0_clockevent_init(void)
{
  	unsigned int latch = (TCU_TIMER_CLOCK + (HZ >> 1)) / HZ;
	int ret;
	struct clock_event_device *cd = &jz_clockevent_device;
	unsigned int cpu = smp_processor_id();
	tcu_writel(TCU_TECR, (1 << TCU_TIMER_CH));

	ret = request_irq(IRQ_TCU2, jz_cpu0timer_interrupt,
			  IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER,
			  "jz-timerirq",
			  &jz_clockevent_device);
	if (ret < 0) {
		pr_err("timer request irq error\n");
		BUG();
	}

	
	tcu_writel(TCU_TMSR, ((1 << TCU_TIMER_CH) | (1 << (TCU_TIMER_CH + 16))));
	tcu_writel(CH_TCSR(TCU_TIMER_CH), CSR_DIV16 | CSR_EXT_EN);
	tcu_writel(CH_TDFR(TCU_TIMER_CH), latch - 1);
	tcu_writel(CH_TDHR(TCU_TIMER_CH), latch + 1);
	tcu_writel(CH_TCNT(TCU_TIMER_CH), 0);

	tcu_writel(TCU_TFCR, (1 << TCU_TIMER_CH));
	tcu_writel(TCU_TMCR, (1 << TCU_TIMER_CH));
	tcu_writel(TCU_TESR, (1 << TCU_TIMER_CH));

	cd->cpumask = cpumask_of(cpu);
	clockevents_register_device(cd);
}

