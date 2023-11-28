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

/* for performance counter */
#include <linux/sched.h>
#include <asm/thread_info.h>

#include <irq_cpu.h>
#include <core_base.h>

#define PART_OFF	0x20
#define ISR_OFF		(0x00)
#define IMR_OFF		(0x04)
#define IMSR_OFF	(0x08)
#define IMCR_OFF	(0x0c)
#define IPR_OFF		(0x10)


struct core_irq_base
{
	unsigned int cpu_num;
	unsigned int dev_base;
};

#if (NR_CPUS > 4)
#error "core irq max support 4."
#endif
static struct core_irq_base core_irq_addr[4] = {
	[0] = {
		.cpu_num = 0,
		.dev_base = INTC_IOBASE + (0x100 * 0),
	},
#ifdef CONFIG_SMP
	[1] = {
		.cpu_num = 1,
		.dev_base = INTC_IOBASE + (0x100 * 1),
	},
	[2] = {
		.cpu_num = 2,
		.dev_base = INTC_IOBASE + (0x100 * 2),
	},
	[3] = {
		.cpu_num = 3,
		.dev_base = INTC_IOBASE + (0x100 * 3),
	}
#endif /* CONFIG_SMP */
};

struct core_irq_chip{
	void __iomem *iobase;
	unsigned int next_irq_resp;
};
struct irq_chip_data {
	void __iomem *iobase[NR_CPUS];
	raw_spinlock_t	lock;
	struct cpumask irq_idle_mask;
	unsigned int wake_up_flag[(INTC_NR_IRQS + 31) / 32];	/* Interrupts which can generate wakeup signal when cpu asleep. */
	unsigned char intc_num[INTC_NR_IRQS];			/* Which CPU/INTC does an interrupt current connect to, */
	struct cpumask affinity[INTC_NR_IRQS];			/* Which CPU does an interrupt src affinity to. */
};
struct core_irq_chips {
	struct core_irq_chip *__percpu *percpu_irq_chip;
	struct irq_chip_data chip_data;
};

static struct core_irq_chip irq_chip_buf[NR_CPUS];
static struct core_irq_chips irq_chips;


#if 0
static void dump_intc_regs(struct irq_chip_data *chip)
{
	int i, j;
	printk("Current CPU: %d\n", smp_processor_id());
	for(i = 0; i < num_online_cpus(); i++) {
		WARN_ON(!cpu_online(i));
		for(j = 0; j < 2; j++) {
			printk("	ISR_OFF	(%d):%p: %x\n", j, chip->iobase[i], readl(chip->iobase[i] + j * PART_OFF + ISR_OFF	));
			printk("	IMR_OFF	(%d):%p: %x\n", j, chip->iobase[i], readl(chip->iobase[i] + j * PART_OFF + IMR_OFF	));
			printk("	IMSR_OFF(%d):%p: %x\n", j, chip->iobase[i], readl(chip->iobase[i] + j * PART_OFF + IMSR_OFF) );
			printk("	IMCR_OFF(%d):%p: %x\n", j, chip->iobase[i], readl(chip->iobase[i] + j * PART_OFF + IMCR_OFF) );
			printk("	IPR_OFF	(%d):%p: %x\n", j, chip->iobase[i], readl(chip->iobase[i] + j * PART_OFF + IPR_OFF	));
		}
	}
	printk("ccu  msk: %x: %x\n", 0xb2200120, *(volatile unsigned int *)(0xb2200120));
	printk("ccu  pnd: %x: %x\n", 0xb2200100, *(volatile unsigned int *)(0xb2200100));


}
#endif


#ifdef CONFIG_SMP
static inline void irq_lock(struct irq_chip_data *chip)
{
	raw_spin_lock(&chip->lock);
}

static inline void irq_unlock(struct irq_chip_data *chip)
{
	raw_spin_unlock(&chip->lock);
}
#else
static inline void irq_lock(struct irq_chip_data *chip) { }
static inline void irq_unlock(struct irq_chip_data *chip) { }
#endif

static void xburst2_irq_unmask(struct irq_data *data)
{
	struct irq_chip_data *chip = (struct irq_chip_data *)irq_data_get_irq_chip_data(data);
	void __iomem * iobase;
	int irq_num = (data->irq - IRQ_INTC_BASE);
	unsigned int group;
	unsigned int bit;
	int intc_num;

	group = irq_num / 32;
	bit = 1 << ((irq_num) & 31);
	irq_lock(chip);
	intc_num = chip->intc_num[irq_num];
	iobase = chip->iobase[intc_num];
	writel(bit, iobase + group * PART_OFF + IMCR_OFF);
	irq_unlock(chip);


}

static void xburst2_irq_mask(struct irq_data *data)
{
	struct irq_chip_data *chip = (struct irq_chip_data *)irq_data_get_irq_chip_data(data);
	void __iomem * iobase;
	int irq_num = (data->irq - IRQ_INTC_BASE);
	unsigned int group;
	unsigned int bit;
	int intc_num;
	group = irq_num / 32;
	bit = 1 << ((irq_num) & 31);
	irq_lock(chip);
	intc_num = chip->intc_num[irq_num];
	iobase = chip->iobase[intc_num];
	writel(bit, iobase + group * PART_OFF + IMSR_OFF);
#ifdef CONFIG_SMP
	core_irq_distribution(0);
#endif
	irq_unlock(chip);
}

static int xburst2_irq_set_wake(struct irq_data *data, unsigned int on)
{
	struct irq_chip_data *chip = (struct irq_chip_data *)irq_data_get_irq_chip_data(data);
	unsigned int group = (data->irq - IRQ_INTC_BASE) / 32;
	unsigned int bit = 1 << ((data->irq - IRQ_INTC_BASE) & 31);
//	printk("wakeup irq %d\n",data->irq - IRQ_INTC_BASE);
	irq_lock(chip);
	if(on) {
		chip->wake_up_flag[group] |= bit;
	}else {
		chip->wake_up_flag[group] &= ~bit;
	}
	irq_unlock(chip);
	return 0;
}

#ifdef CONFIG_SMP
static int xburst2_set_affinity(struct irq_data *data, const struct cpumask *dest, bool force)
{
	struct irq_chip_data *chip = (struct irq_chip_data *)irq_data_get_irq_chip_data(data);
	int index = data->irq - IRQ_INTC_BASE;
	unsigned int cpu = cpumask_any_and(dest,cpu_online_mask);
	void __iomem * prev_iobase;
	void __iomem * iobase;
	int prev_intc_num;
	int group;
	unsigned int bit;

//	printk("########%s, %d, dest->bits %lx, cpu_online_mask->bits: %lx\n", __func__, __LINE__, dest->bits[0], cpu_online_mask->bits[0]);

	irq_lock(chip);

	cpumask_and(&chip->affinity[index],dest,cpu_online_mask);

	prev_intc_num = chip->intc_num[index];
	prev_iobase = chip->iobase[prev_intc_num];
	iobase = chip->iobase[cpu];
	group = index / 32;
	bit = 1 << ((index) & 31);

	if(!(readl(prev_iobase + PART_OFF * group + IMR_OFF) & bit)){
		writel(bit,prev_iobase + PART_OFF * group + IMSR_OFF);
		writel(bit,iobase + PART_OFF * group + IMCR_OFF);
	} else {
		writel(bit,prev_iobase + PART_OFF * group + IMSR_OFF);
	}
	chip->intc_num[index] = cpu;

	irq_unlock(chip);

	return IRQ_SET_MASK_OK;
}
#endif

static struct irq_chip xburst2_irq_chip = {
	.name 		= "xburst2-irqchip",
	.irq_mask	= xburst2_irq_mask,
	.irq_mask_ack 	= xburst2_irq_mask,
	.irq_unmask 	= xburst2_irq_unmask,

	.irq_ack	= xburst2_irq_mask,
	.irq_eoi	= xburst2_irq_unmask,

	.irq_set_wake 	= xburst2_irq_set_wake,
#ifdef CONFIG_SMP
	.irq_set_affinity = xburst2_set_affinity,
#endif
};
static irqreturn_t xburst2_irq_handle(int irq, void *data)
{
	struct core_irq_chip *irq_chip = *(struct core_irq_chip **)data;
	void __iomem * iobase = irq_chip->iobase;
	unsigned int irq_num = 0xffffffff;
	int n = 0,i;
	unsigned int next_irq_resp = irq_chip->next_irq_resp;
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
		if(irq_num != 0xffffffff)
		   break;
		next_irq_resp = (next_irq_resp & (~31)) + 32;
		if(next_irq_resp >= INTC_NR_IRQS)
			next_irq_resp = 0;
		ipr = readl(iobase + (next_irq_resp / 32) * PART_OFF + IPR_OFF);
		n++;
	}while(n < ((INTC_NR_IRQS + 31) / 32 + 1));
	if(irq_num != 0xffffffff)
	{
#ifdef CONFIG_SMP
		irq_chip->next_irq_resp = irq_num;
#else
		irq_chip->next_irq_resp = irq_num + 1;
		if(irq_chip->next_irq_resp >= INTC_NR_IRQS)
			irq_chip->next_irq_resp = 0;
#endif
		do_IRQ(irq_num + IRQ_INTC_BASE);
	}else{
		pr_err("Error: Not Find any irq,check me %s %d.\n",__FILE__,__LINE__);
	}
	return IRQ_HANDLED;
}

static void __init core_irq_setup(void)
{
	struct irq_chip_data *chip_data = &irq_chips.chip_data;
	int ret;
	int i;
	irq_chips.percpu_irq_chip = alloc_percpu(struct core_irq_chip *);
	if (!irq_chips.percpu_irq_chip) {
		pr_err("ERROR:alloc cpu intc dev percpu fail!\n");
		return;
	}

	ret = request_percpu_irq(CORE_INTC_IRQ,xburst2_irq_handle,"xburst2-intc",irq_chips.percpu_irq_chip);
	if(ret) {
		pr_err("ERROR:intc request error,check %s %d\n",__FILE__,__LINE__);
		return;
	}
	cpumask_clear(&chip_data->irq_idle_mask);
	raw_spin_lock_init(&chip_data->lock);
	for(i = 0;i < ARRAY_SIZE(chip_data->wake_up_flag);i++)
		chip_data->wake_up_flag[i] = 0;
	for (i = 0; i < INTC_NR_IRQS; i++) {
		cpumask_clear(&chip_data->affinity[i]);
		chip_data->intc_num[i] = 0;
		irq_set_chip_data(i + IRQ_INTC_BASE, (void *)chip_data);
		irq_set_chip_and_handler(i + IRQ_INTC_BASE, &xburst2_irq_chip, handle_level_irq);
	}
	pr_info("core irq setup finished\n");
}

void __cpuinit percpu_irq_init(unsigned int cpu_num)
{
	int i;
	unsigned int base = 0;
	void __iomem * iobase;
	struct core_irq_chip *irq_chip;
	int cpu = smp_processor_id();
	for(i = 0;i < NR_CPUS;i++)
	{
		if(core_irq_addr[i].cpu_num == cpu_num) {
			base = core_irq_addr[i].dev_base;
			irq_chip = &irq_chip_buf[i];
			break;
		}
	}

	if(base == 0){
		pr_err("Error: CPU[%d] don't finded intc base address\n",cpu_num);
		return;
	}
	iobase = ioremap(base, 0xfff);

	irq_chip->iobase = iobase;
	irq_chip->next_irq_resp = 0;
	irq_chips.chip_data.iobase[cpu] = iobase;
	*__this_cpu_ptr(irq_chips.percpu_irq_chip) = irq_chip;
	enable_percpu_irq(CORE_INTC_IRQ, IRQ_TYPE_NONE);
	pr_info("percpu irq inited.\n");
}

void percpu_irq_deinit(void)
{
	struct core_irq_chip *irq_chip = *__this_cpu_ptr(irq_chips.percpu_irq_chip);
	disable_percpu_irq(CORE_INTC_IRQ);
	iounmap(irq_chip->iobase);
}


void __init arch_init_irq(void)
{
	/* Set up MIPS CPU irq */
	xburst2_cpu_irq_init();
	core_irq_setup();
	percpu_irq_init(0);
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned long status = read_c0_status();
	unsigned long cause = read_c0_cause();
	unsigned long r = ((status & cause) >> 8) & 0xff;

	if(r) {
		if (r & 8) {  		/* IPI */
			do_IRQ(MIPS_CPU_IRQ_BASE + 3);
		} else if(r & 16) {	/* OST */
			do_IRQ(MIPS_CPU_IRQ_BASE + 4);
		} else if (r & 4) {	/* INTC */
			do_IRQ(MIPS_CPU_IRQ_BASE + 2);
		} else {		/* Others */
			do_IRQ(MIPS_CPU_IRQ_BASE + __ffs(r));
		}

	} else {
		printk("IRQ Error, cpu: %d Cause:0x%08lx, Status:0x%08lx\n", smp_processor_id(), cause, status);
	}
}
void core_irq_distribution(int lock)
{
	struct irq_chip_data *chip = &irq_chips.chip_data;
	unsigned int cpu = smp_processor_id();
	void __iomem * iobase = NULL;
	void __iomem * prev_iobase = NULL;
	int i;
	struct cpumask resp_mask;
	unsigned int ipr = 0;

	if(lock){
		irq_lock(chip);
	}
	prev_iobase = chip->iobase[cpu];
	for(i = 0;i < INTC_NR_IRQS;i++)
	{
		int group = i / 32;
		int bitcount = i & 31;
		unsigned int bit = 1 << bitcount;
		int resp_cpu;

		if(bitcount == 0) {
			ipr = readl(prev_iobase + group * PART_OFF + IPR_OFF);
		}

		if(ipr == 0){
			i = (i & (~31)) + 31;
			continue;
		}

		if(ipr & bit){
			cpumask_and(&resp_mask,&chip->affinity[i],&chip->irq_idle_mask);
			for_each_cpu(resp_cpu,&resp_mask){
				if(resp_cpu != cpu){
					//printk("migrate irq from cpu[%d] to cpu[%d], i: %d\n", cpu, resp_cpu, i);
					iobase = chip->iobase[resp_cpu];
					if(!(readl(prev_iobase + PART_OFF * group + IMR_OFF) & bit)){
						writel(bit,prev_iobase + PART_OFF * group + IMSR_OFF);
						writel(bit,iobase + PART_OFF * group + IMCR_OFF);
					}else{
						writel(bit,iobase + PART_OFF * group + IMSR_OFF);
					}
					cpumask_clear_cpu(resp_cpu,&chip->irq_idle_mask);
					chip->intc_num[i] = resp_cpu;
					break;
				}
			}
		}
	}
	if(lock){
		irq_unlock(chip);
	}
}

void core_irq_cpumask_idle(int idle)
{
	struct irq_chip_data *chip = &irq_chips.chip_data;
	int cpu = smp_processor_id();
	irq_lock(chip);
	if(idle){
		cpumask_set_cpu(cpu,&chip->irq_idle_mask);
	}else{
		cpumask_clear_cpu(cpu,&chip->irq_idle_mask);
	}
	irq_unlock(chip);
}

static unsigned long intc_saved[2];
extern void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume);

void arch_suspend_disable_irqs(void)
{
	struct irq_chip_data *chip_data = &irq_chips.chip_data;
	struct core_irq_chip *irq_chip = *__this_cpu_ptr(irq_chips.percpu_irq_chip);

	unsigned int *intc_wakeup = chip_data->wake_up_flag;
	void __iomem * intc_base = irq_chip->iobase;

	struct irq_desc *desc;
	int i,j,irq;

	local_irq_disable();

	intc_saved[0] = readl(intc_base + IMR_OFF);
	intc_saved[1] = readl(intc_base + PART_OFF + IMR_OFF);

	/* Mask interrupts which are not wakeup src. */
	writel(0xffffffff & ~intc_wakeup[0], intc_base + IMSR_OFF);
	writel(0xffffffff & ~intc_wakeup[1], intc_base + PART_OFF + IMSR_OFF);


	for(j=0;j<2;j++) {
		for(i=0;i<32;i++) {
			if(intc_wakeup[j] & (0x1<<i)) {
				irq = i + IRQ_INTC_BASE + 32*j;
				desc = irq_to_desc(irq);
				__enable_irq(desc, irq, true);
			}
		}
	}

}

void arch_suspend_enable_irqs(void)
{
	struct core_irq_chip *irq_chip = *__this_cpu_ptr(irq_chips.percpu_irq_chip);
	void __iomem * intc_base = irq_chip->iobase;

	writel(0xffffffff & ~intc_saved[0], intc_base + IMCR_OFF);
	writel(0xffffffff & ~intc_saved[1], intc_base + PART_OFF + IMCR_OFF);

	local_irq_enable();
}
