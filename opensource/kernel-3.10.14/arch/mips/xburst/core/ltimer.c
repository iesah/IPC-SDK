#include <linux/percpu.h>
#include <linux/interrupt.h>

#include <linux/clockchips.h>

#include <asm/smp.h>

#ifndef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
#error "JZSOC SMP need GENERIC_CLOCKEVENTS_BROADCAST !!!"
#endif

/*
 * Timer (local or broadcast) support
 */
static DEFINE_PER_CPU(struct clock_event_device, percpu_clockevent);

void smp_ipi_timer_interrupt(void)
{
	struct clock_event_device *evt = &__get_cpu_var(percpu_clockevent);
	irq_enter();
	evt->event_handler(evt);
	irq_exit();
}

extern void jzsoc_ipi_func(cpumask_t mask, void (*fun)(void));
static void smp_timer_broadcast(const struct cpumask *mask)
{
	extern struct plat_smp_ops *mp_ops;	/* private */
	//cpumask_t cpumask = *mask;

	mp_ops->send_ipi_mask(mask, SMP_IPI_TIMER);
}

static void broadcast_timer_set_mode(enum clock_event_mode mode,
	struct clock_event_device *evt)
{
	
	printk("%s %d mode = %d cpu = %d\n",__FILE__,__LINE__,mode,smp_processor_id());
}

static void local_timer_setup(struct clock_event_device *evt)
{
	evt->name	= "dummy_timer";
	evt->features	= CLOCK_EVT_FEAT_PERIODIC |
			  CLOCK_EVT_FEAT_DUMMY;
	evt->rating	= 100;
	evt->mult	= 1;
	evt->set_mode	= broadcast_timer_set_mode;
	evt->broadcast	= smp_timer_broadcast;
	clockevents_register_device(evt);
}

void __cpuinit percpu_timer_setup(void)
{
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *evt = &per_cpu(percpu_clockevent, cpu);

	memset(evt, 0, sizeof(*evt));
	evt->cpumask = cpumask_of(cpu);
	
	local_timer_setup(evt);
}
