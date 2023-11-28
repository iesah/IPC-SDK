#ifndef IRQ_CPU_H
#define IRQ_CPU_H

#define MIPS_CPU_IRQ_BASE	(0)

#define CORE_INTC_IRQ       (MIPS_CPU_IRQ_BASE + 2)
#define CORE_MAILBOX_IRQ    (MIPS_CPU_IRQ_BASE + 3)
#define CORE_SYS_OST_IRQ    (MIPS_CPU_IRQ_BASE + 4)

#define CORE_IRQS_END	(7)

#define INTC_NR_IRQS	(64)
#define	IRQ_INTC_BASE   (CORE_IRQS_END + 1)
#define	IRQ_INTC_END    (IRQ_INTC_BASE + INTC_NR_IRQS - 1)

void __init xburst2_cpu_irq_init(void);
void core_irq_distribution(int lock);
void core_irq_cpumask_idle(int idle);

#endif /* IRQ_CPU_H */
