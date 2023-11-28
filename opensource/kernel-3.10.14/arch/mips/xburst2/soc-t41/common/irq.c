/*
 * Interrupt controller.
 *
 * Copyright (c) 2006-2007  Ingenic Semiconductor Inc.
 * Author: <lhhuang@ingenic.cn>
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 */
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>

#include <irq_cpu.h>
#include <soc/base.h>
#include <soc/irq.h>

#include <smp_cp0.h>

/* for performance counter */
#include <linux/sched.h>
#include <asm/thread_info.h>

#define TRACE_IRQ        1
#define PART_OFF	0x20

#define ISR_OFF		(0x00)
#define IMR_OFF		(0x04)
#define IMSR_OFF	(0x08)
#define IMCR_OFF	(0x0c)
#define IPR_OFF		(0x10)

struct xburst2_irq_chip_data{
	void __iomem *base;
	unsigned int bit_mask;
	unsigned int irq_num;
};

#define CPU_IRQ_INTC (MIPS_CPU_IRQ_BASE + 2)

struct cpu_dev_base
{
	unsigned int cpu_num;
	unsigned int dev_base;
};

static struct cpu_dev_base cpu_intc_address[NR_CPUS] = {
	[0] = {
		.cpu_num = 0,
		.dev_base = 0x10001000,
	},
	[1] = {
		.cpu_num = 1,
		.dev_base = 0,
	}
};

struct cpu_dev {
	struct cpu_dev_base *cpu_dev;
	void __iomem * iobase;
	struct xburst2_irq_chip_data chip_data[INTC_NR_IRQS];
	unsigned int next_irq_resp;
};


static struct cpu_dev cpu_intc_dev[NR_CPUS];
static struct cpu_dev *__percpu *percpu_cpu_intc_dev;

extern void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume);

static void intc_irq_unmask(struct irq_data *data)
{
	struct xburst2_irq_chip_data *chip = (struct xburst2_irq_chip_data *)irq_data_get_irq_chip_data(data);
	writel(chip->bit_mask, chip->base + IMCR_OFF);
}

static void intc_irq_mask(struct irq_data *data)
{
	struct xburst2_irq_chip_data *chip = (struct xburst2_irq_chip_data *)irq_data_get_irq_chip_data(data);
	writel(chip->bit_mask, chip->base + IMSR_OFF);
}

static int intc_irq_set_wake(struct irq_data *data, unsigned int on)
{
	//TODO: for wake.
	return 0;
}

#ifdef CONFIG_SMP
static int intc_set_affinity(struct irq_data *data, const struct cpumask *dest, bool force)
{
	//TODO: for cpu affinity.
	return 0;
}
#endif

static struct irq_chip jzintc_chip = {
	.name 		= "jz-intc",
	.irq_mask	= intc_irq_mask,
	.irq_mask_ack 	= intc_irq_mask,
	.irq_unmask 	= intc_irq_unmask,

	.irq_ack	= intc_irq_mask,
	.irq_eoi	= intc_irq_unmask,

	.irq_set_wake 	= intc_irq_set_wake,
#ifdef CONFIG_SMP
	.irq_set_affinity = intc_set_affinity,
#endif
};
void prom_printk(const char *fmt, ...);
static irqreturn_t xburst2_intc_handle(int irq, void *data)
{
	struct cpu_dev *cpu_intc_dev = *(struct cpu_dev **)data;
	void __iomem * iobase = cpu_intc_dev->iobase;
	unsigned int irq_num = 0xffffffff;
	int n = 0,i;
	unsigned int next_irq_resp = cpu_intc_dev->next_irq_resp;
	unsigned int ipr = readl(iobase + (next_irq_resp / 32) * PART_OFF + IPR_OFF);
	do{
		if(irq) {
			for(i = next_irq_resp & 31;i < 32;i++)
			{
				if(ipr & (1 << i))
				{
					irq_num = (next_irq_resp & (~31)) + i;
					break;
				}
			}
		}
		if(irq_num == 0xffffffff){
			next_irq_resp = (next_irq_resp & (~31)) + 32;
			if(next_irq_resp >= INTC_NR_IRQS)
				next_irq_resp = 0;
			ipr = readl(iobase + (next_irq_resp / 32) * PART_OFF + IPR_OFF);
			n++;
		}
	}while((n < ((INTC_NR_IRQS + 31) / 32 + 1)) && (irq_num == 0xffffffff));
	if(irq_num != 0xffffffff)
	{
		cpu_intc_dev->next_irq_resp = irq_num + 1;
		if(cpu_intc_dev->next_irq_resp >= INTC_NR_IRQS)
			cpu_intc_dev->next_irq_resp = 0;
		do_IRQ(irq_num + IRQ_INTC_BASE);
	}else{
		pr_err("Error: Not Find any irq,check me %s %d.\n",__FILE__,__LINE__);
	}
	return IRQ_HANDLED;
}

void __cpuinit per_intc_init(unsigned int cpu_num,unsigned int cpu_logic)
{
	int i;
	unsigned int base = 0;
	void __iomem * iobase = cpu_intc_dev[cpu_logic].iobase;
	struct xburst2_irq_chip_data *chip_data = cpu_intc_dev[cpu_logic].chip_data;
	/* struct irqaction *action = &cpu_intc_dev[cpu_logic].action; */
	for(i = 0;i < NR_CPUS;i++)
	{
		if(cpu_intc_address[i].cpu_num == cpu_num){
			base = cpu_intc_address[i].dev_base;
			break;
		}
	}
	if(base == 0){
		pr_err("Error: CPU[%d] don't finded intc base address\n",cpu_num);
		return;
	}
	if(cpu_intc_dev[cpu_logic].iobase == 0){
		iobase = ioremap(base, 0xfff);
		cpu_intc_dev[cpu_logic].cpu_dev = &cpu_intc_address[cpu_logic];
		cpu_intc_dev[cpu_logic].iobase = iobase;
	}

// mask all interrupt per cpu.
	for (i = 0; i < INTC_NR_IRQS; i+=32) {
		writel(0xffffffff, iobase + (PART_OFF * i / 32)+ IMSR_OFF);
	}
	cpu_intc_dev[cpu_logic].next_irq_resp = 0;
// setup intc chips.
	for (i = 0; i < INTC_NR_IRQS; i++) {
		chip_data[i].base = iobase + PART_OFF * (i / 32);
		chip_data[i].bit_mask = 1 << (i & 31);
		chip_data[i].irq_num = i;
		irq_set_chip_data(i + IRQ_INTC_BASE, (void *)&chip_data[i]);
		irq_set_chip_and_handler(i + IRQ_INTC_BASE, &jzintc_chip, handle_level_irq);
	}

	*__this_cpu_ptr(percpu_cpu_intc_dev) = &cpu_intc_dev[cpu_logic];
	enable_percpu_irq(CPU_IRQ_INTC, IRQ_TYPE_NONE);
	printk("percpu %x\n",read_c0_status());
}

void __init arch_init_irq(void)
{
	/* Set up MIPS CPU irq */
	int ret;
	xburst2_cpu_irq_init();
	percpu_cpu_intc_dev = alloc_percpu(struct cpu_dev *);
	if (!percpu_cpu_intc_dev) {
		pr_err("ERROR:alloc cpu intc dev percpu fail!\n");
		return;
	}
	ret = request_percpu_irq(CPU_IRQ_INTC,xburst2_intc_handle,"jz-intc",percpu_cpu_intc_dev);
	if(ret) {
		pr_err("ssssssssssssssss request intc error\n");
	}
	per_intc_init(0,0);
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned long r = (read_c0_status() & read_c0_cause()) >> 8;
	do_IRQ(MIPS_CPU_IRQ_BASE + __ffs(r & 0xff));
}

void arch_suspend_disable_irqs(void)
{
	/* int i,j,irq; */
	/* struct irq_desc *desc; */

	/* intc_saved[0] = readl(intc_base + IMR_OFF); */
	/* intc_saved[1] = readl(intc_base + PART_OFF + IMR_OFF); */

	/* writel(0xffffffff & ~intc_wakeup[0], intc_base + IMSR_OFF); */
	/* writel(0xffffffff & ~intc_wakeup[1], intc_base + PART_OFF + IMSR_OFF); */

	/* for(j=0;j<2;j++) { */
	/* 	for(i=0;i<32;i++) { */
	/* 		if(intc_wakeup[j] & (0x1<<i)) { */
	/* 			irq = i + IRQ_INTC_BASE + 32*j; */
	/* 			desc = irq_to_desc(irq); */
	/* 			__enable_irq(desc, irq, true); */
	/* 		} */
	/* 	} */
	/* } */
}

void arch_suspend_enable_irqs(void)
{
	/* writel(0xffffffff & ~intc_saved[0], intc_base + IMCR_OFF); */
	/* writel(0xffffffff & ~intc_saved[1], intc_base + PART_OFF + IMCR_OFF); */
}
