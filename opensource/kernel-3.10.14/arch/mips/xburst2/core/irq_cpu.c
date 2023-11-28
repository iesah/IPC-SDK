#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <irq_cpu.h>

static inline void unmask_xburst2_irq(struct irq_data *d)
{
	set_c0_status(0x100 << (d->irq - MIPS_CPU_IRQ_BASE));
#ifdef CONFIG_SMP
	core_irq_cpumask_idle(1);
#endif
}

static inline void mask_xburst2_irq(struct irq_data *d)
{
	clear_c0_status(0x100 << (d->irq - MIPS_CPU_IRQ_BASE));
#ifdef CONFIG_SMP
	core_irq_cpumask_idle(0);
	if(d->irq != CORE_INTC_IRQ){
		core_irq_distribution(1);
	}
#endif
}

static struct irq_chip xburst2_cpu_irq_controller = {
	.name		= "XBURST2",
	.irq_ack	= mask_xburst2_irq,
	.irq_mask	= mask_xburst2_irq,
	.irq_mask_ack	= mask_xburst2_irq,
	.irq_unmask	= unmask_xburst2_irq,
	.irq_eoi	= unmask_xburst2_irq,
};
void __init xburst2_cpu_irq_init(void)
{
	int irq_base = MIPS_CPU_IRQ_BASE;
	int i;

	/* Mask interrupts. */
	clear_c0_status(ST0_IM);
	clear_c0_cause(CAUSEF_IP);

	for (i = irq_base; i < irq_base + 8; i++){
		irq_set_percpu_devid(i);
		irq_set_chip_and_handler(i, &xburst2_cpu_irq_controller,
								 handle_percpu_devid_irq);
	}
}
