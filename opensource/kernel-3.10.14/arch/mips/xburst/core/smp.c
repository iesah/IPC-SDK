/*
 * Copyright (C) 2001, 2002, 2003 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//#define DEBUG
//#define SMP_DEBUG

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/kernel_stat.h>
#include <linux/earlysuspend.h>

#include <asm/mmu_context.h>
#include <asm/io.h>
#include <asm/uasm.h>
#include <asm/rjzcache.h>

#include <smp_cp0.h>

#include <soc/cache.h>
#include <soc/base.h>
#include <soc/cpm.h>
#include <linux/regulator/consumer.h>
#include <linux/err.h>
#include <mach/jzcpm_pwc.h>
#ifdef SMP_DEBUG
static void jzsoc_smp_showregs(void);
#else
#define jzsoc_smp_showregs() do { } while (0)
#endif

static DEFINE_SPINLOCK(smp_lock);
#define smp_spinlock(flags)     spin_lock_irqsave(&smp_lock,flags)
#define smp_spinunlock(flags) 	spin_unlock_irqrestore(&smp_lock,flags)
static unsigned int cpu_reim = 1;

static int smp_flag;
//static struct regulator *scpu_regulator = NULL;

#define SMP_FLAG (*(volatile int*)KSEG1ADDR(&smp_flag))

static struct cpumask cpu_ready_e, cpu_running, cpu_start;
static struct cpumask *cpu_ready;
static unsigned long boot_sp, boot_gp;
static void *scpu_pwc = NULL;
/* Special cp0 register init */
static void __cpuinit jzsoc_smp_init(void)
{
	unsigned int imask = STATUSF_IP3 | STATUSF_IP2 |
		STATUSF_IP1 | STATUSF_IP0;
	change_c0_status(ST0_IM, imask);
}

/*
 * Code to run on secondary just after probing the CPU
 */
extern void __cpuinit jzcpu_timer_setup(void);
extern void percpu_timer_stop(void);

void percpu_timer_setup(void);
static void __cpuinit jzsoc_init_secondary(void)
{
	int cpu = smp_processor_id();
	smp_disable_interrupt(cpu);
	jzsoc_smp_init();
	jzcpu_timer_setup();
	jzsoc_smp_showregs();
//	percpu_timer_setup();
}

/*
 * Do any tidying up before marking online and running the idle
 * loop
 */
extern void percpu_timer_cpu_affinity(void);
static void __cpuinit jzsoc_smp_finish(void)
{
	int cpu = smp_processor_id();
	cpu_reim |= (1 << cpu);
	smp_enable_interrupt(cpu);
	percpu_timer_cpu_affinity();
	local_irq_enable();
	jzsoc_smp_showregs();
	printk("[SMP] slave cpu%d start up finished.\n",cpu);
}

static struct bounce_addr {
	unsigned long base;
} smp_bounce;

extern void __jzsoc_secondary_start(void);
static void build_bounce_code(unsigned long *spp, unsigned long *gpp)
{
	int i;
	unsigned long base[32] = {0,};
	unsigned int pflag = (unsigned int)KSEG1ADDR(&smp_flag);
	unsigned int entry = (unsigned int)__jzsoc_secondary_start;
	unsigned int *p;

	for(i=0;i<32;i++) {
		base[i] = __get_free_pages(GFP_KERNEL, 0);
		if(!base[i] || (base[i] & 0xffff))
			continue;
		smp_bounce.base = base[i];
		break;
	}

	for(i=i-1;i>=0;i--) {
		free_pages(base[i], 0);
	}

	BUG_ON(!smp_bounce.base || (smp_bounce.base & 0xffff));

	p = (unsigned int*)smp_bounce.base;
	UASM_i_LA(&p, 26, pflag);
	UASM_i_LW(&p, 2, 0, 26);
	UASM_i_ADDIU(&p, 2, 2, 1);
	UASM_i_SW(&p, 2, 0, 26);

	/* t7: cpu_start. t8: cpu_ready. t9: cpu_running. */
	UASM_i_LA(&p, 15, (unsigned long)cpu_start.bits);
	UASM_i_LA(&p, 24, (unsigned long)cpu_ready_e.bits);
	UASM_i_LA(&p, 25, (unsigned long)cpu_running.bits);

	UASM_i_LA(&p, 29, (unsigned long)spp);
	UASM_i_LA(&p, 28, (unsigned long)gpp);
	UASM_i_LA(&p, 31, entry);
	uasm_i_jr(&p, 31);
	uasm_i_nop(&p);
}

static int cpu_boot(int cpu, unsigned long sp, unsigned long gp)
{
	if (cpumask_test_cpu(cpu, &cpu_running))
		return -EINVAL;

	if (cpumask_test_cpu(cpu, cpu_ready)) {
		boot_sp = sp;
		boot_gp = gp;

		smp_wmb();
		cpumask_set_cpu(cpu, &cpu_start);
		while (!cpumask_test_cpu(cpu, &cpu_running))
			;
		return 0;
	}

	return -EINVAL;
}

/*
 * Setup the PC, SP, and GP of a secondary processor and start it
 * running!
 */
extern void __jzsoc_secondary_start(void);
static void __cpuinit jzsoc_boot_secondary(int cpu, struct task_struct *idle)
{
	int err;
	unsigned long flags,ctrl;


	printk("jzsoc_boot_secondary start\n");
	local_irq_save(flags);

	/* set reset bit! */
	ctrl = get_smp_ctrl();
	ctrl |= (1 << cpu);
	set_smp_ctrl(ctrl);

	/* blast all cache before booting secondary cpu */
	blast_dcache_jz();
	blast_icache_jz();

	cpm_clear_bit(15,CPM_CLKGR1);
	cpm_pwc_enable(scpu_pwc);
	preempt_disable();
	preempt_enable_no_resched();

	/* clear reset bit! */
	ctrl = get_smp_ctrl();
	ctrl &= ~(1 << cpu);
	set_smp_ctrl(ctrl);
wait:
	if (!cpumask_test_cpu(cpu, cpu_ready))
		goto wait;
	pr_info("[SMP] Booting CPU%d ...\n", cpu);
//	pr_debug("[SMP] Booting CPU%d ...\n", cpu);
	err = cpu_boot(cpu_logical_map(cpu), __KSTK_TOS(idle),
			(unsigned long)task_thread_info(idle));
	if (err != 0)
		pr_err("start_cpu(%i) returned %i\n" , cpu, err);
	local_irq_restore(flags);
}

static void __init jzsoc_smp_setup(void)
{
	int i, num;
	scpu_pwc = cpm_pwc_get(PWC_SCPU);
	cpus_clear(cpu_possible_map);
	cpus_clear(cpu_present_map);

	cpu_set(0, cpu_possible_map);
	cpu_set(0, cpu_present_map);

	__cpu_number_map[0] = 0;
	__cpu_logical_map[0] = 0;

	for (i = 1, num = 0; i < NR_CPUS; i++) {
		cpu_set(i, cpu_possible_map);
		cpu_set(i, cpu_present_map);

		__cpu_number_map[i] = ++num;
		__cpu_logical_map[num] = i;
	}

	pr_info("[SMP] Slave CPU(s) %i available.\n", num);
}

static void __init jzsoc_prepare_cpus(unsigned int max_cpus)
{
	unsigned long reim;

	if(max_cpus <= 1) return;

	set_smp_mbox0(0);
	set_smp_mbox1(0);
	set_smp_mbox2(0);
	set_smp_mbox3(0);

	cpu_ready = (cpumask_t*)KSEG1ADDR(&cpu_ready_e);

	pr_debug("[SMP] Prepare %d cpus.\n", max_cpus);
	/* prepare slave cpus entry code */
	build_bounce_code(&boot_sp, &boot_gp);

	/* reset register */
	set_smp_reim(0x0100);
	//set_smp_ctrl(0xe0e0e);
	set_smp_ctrl(0xe0e);
	set_smp_status(0);

	reim = KSEG1ADDR(smp_bounce.base);
	reim = (reim & 0xffff0000) | 0x01ff;
	set_smp_reim(reim);
}

static void send_ipi_msg(const struct cpumask *mask, unsigned int action)
{
	unsigned int msg;
	unsigned long flags = 0;

#define SEND_MSG(CPU)	do {					\
	if(cpu_isset(CPU,*mask)) {				\
		msg = action | get_smp_mbox##CPU();		\
		set_smp_mbox##CPU(msg);				\
	} } while(0)

	spin_lock_irqsave(&smp_lock,flags);

	SEND_MSG(0);
	SEND_MSG(1);
	SEND_MSG(2);
	SEND_MSG(3);

	spin_unlock_irqrestore(&smp_lock,flags);
}

static void jzsoc_send_ipi_single(int cpu, unsigned int action)
{
	send_ipi_msg(cpumask_of(cpu), action);
}

static void jzsoc_send_ipi_mask(const struct cpumask *mask, unsigned int action)
{
	send_ipi_msg(mask, action);
}

void jzsoc_cpus_done(void)
{
}

#ifdef CONFIG_HOTPLUG_CPU
void reset_irq_resp_fifo(int cpu);
int jzsoc_cpu_disable(void)
{
	unsigned int cpu = smp_processor_id();
//	unsigned int status;
	if (cpu == 0)		/* FIXME */
		return -EBUSY;
	cpu_clear(cpu, cpu_online_map);
	cpu_clear(cpu, cpu_callin_map);

	local_irq_disable();
	percpu_timer_stop();
	cpu_reim &= ~(1 << cpu);
	smp_disable_interrupt(cpu);
	smp_enable_interrupt(0);
	reset_irq_resp_fifo(cpu);
	return 0;
}
void jzsoc_cpu_die(unsigned int cpu)
{
	unsigned long flags;
	unsigned int status;
	if (cpu == 0)		/* FIXME */
		return;
	local_irq_save(flags);
	cpumask_clear_cpu(cpu, cpu_ready);
	cpumask_clear_cpu(cpu, &cpu_start);
	cpumask_clear_cpu(cpu, &cpu_running);
	wmb();
	do{
		status = get_smp_status();
	}while(!(status & (1<<(cpu+16))));
	cpm_pwc_disable(scpu_pwc);
	cpm_set_bit(15,CPM_CLKGR1);
	local_irq_restore(flags);

	printk("disable cpu %d\n",cpu);
}
#endif

void __play_dead(void)
{
	__asm__ __volatile__ (  ".set	push		\n"
				".set	mips3		\n"
				"sync			\n"
				"wait			\n"
				"nop			\n"
				"nop			\n"
				"nop			\n"
				".set	pop		\n"
				);
}

void play_dead(void)
{
	unsigned int cpu = smp_processor_id();

	void (*do_play_dead)(void) = (void (*)(void))KSEG1ADDR(__play_dead);

	local_irq_disable();
	if(cpu == 0) set_smp_mbox0(0);
	else if(cpu == 1) set_smp_mbox1(0);
	else if(cpu == 2) set_smp_mbox2(0);
	else if(cpu == 3) set_smp_mbox3(0);

	smp_clr_pending(1<<cpu);
	smp_clr_pending(1<<(cpu + 8));
	while(1) {
		while(cpumask_test_cpu(cpu, &cpu_running))
			;
		local_flush_tlb_all();
		blast_icache_jz();
		blast_dcache_jz();

		do_play_dead();
	}
}


struct plat_smp_ops jzsoc_smp_ops = {
	.send_ipi_single	= jzsoc_send_ipi_single,
	.send_ipi_mask		= jzsoc_send_ipi_mask,
	.init_secondary		= jzsoc_init_secondary,
	.smp_finish		= jzsoc_smp_finish,
	.boot_secondary		= jzsoc_boot_secondary,
	.smp_setup		= jzsoc_smp_setup,
	.cpus_done		= jzsoc_cpus_done,
	.prepare_cpus		= jzsoc_prepare_cpus,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable		= jzsoc_cpu_disable,
	.cpu_die		= jzsoc_cpu_die,
#endif
};


#ifdef SMP_DEBUG
static void jzsoc_smp_showregs(void)
{
	int cpu = smp_processor_id();
	unsigned int val;
	printk("CPU%d:\n",cpu);
#define P(reg) do {				\
	val = get_smp_##reg();		\
	printk(#reg ":\t%08x\n",val);	\
} while(0)
	P(ctrl); P(status); P(reim); P(mbox0); P(mbox1);
	//P(val);  P(lock);
	printk("cp0 status:\t%08x\n",read_c0_status());
	printk("cp0 cause:\t%08x\n",read_c0_cause());
	}
#endif

void jz_cputimer_interrupt(void);
void jzsoc_mbox_interrupt(int cpu)
{
	unsigned int action = 0;
	unsigned long flags;
	//kstat_incr_irqs_this_cpu(IRQ_SMP_IPI, irq_to_desc(IRQ_SMP_IPI));
	smp_spinlock(flags);
	switch(cpu) {
#define	CASE_CPU(CPU) case CPU:			\
		action = get_smp_mbox##CPU();	\
		set_smp_mbox##CPU(0);		\
		break

		CASE_CPU(0);
		CASE_CPU(1);
		CASE_CPU(2);
		CASE_CPU(3);
	}
	smp_clr_pending(1<<cpu);
	smp_spinunlock(flags);

	if(!action) {
	    goto ipi_finish;
	}
	/*
	 * Call scheduler_ipi() if SMP_RESCHEDULE_YOURSELF,
	 * we changed here in linux-3.0.8
	 */
	/*
	if (action & SMP_RESCHEDULE_YOURSELF)
		generic_handle_irq(IRQ_SMP_RESCHEDULE_YOURSELF);
	if (action & SMP_CALL_FUNCTION)
		generic_handle_irq(IRQ_SMP_CALL_FUNCTION);
	*/
	if (action & SMP_RESCHEDULE_YOURSELF)
	       do_IRQ(IRQ_SMP_RESCHEDULE_YOURSELF);
	if (action & SMP_CALL_FUNCTION)
		do_IRQ(IRQ_SMP_CALL_FUNCTION);

	if (action & SMP_IPI_TIMER)
		jz_cputimer_interrupt();
//		smp_ipi_timer_interrupt();
 ipi_finish:
	return;
}

long switch_cpu_irq(int cpu) {
	int status,pending;
	int nextcpu;
	unsigned long flags;
	long ret = 0;
	smp_spinlock(flags);
	nextcpu = (cpu + 1) % 2;
	if(cpu_online(nextcpu)){
		status = get_smp_reim();
		status &= ~(3 << 8);
		status |=  1 << (nextcpu + 8);
		set_smp_reim(status);
		ret = nextcpu;
	}else {
		ret = nextcpu | 0x80000000;
	}
	pending = get_smp_status();
	pending &= ~(1 << (cpu + 8));
	set_smp_status(pending);
	smp_spinunlock(flags);
	return ret;
}

void smp_cputimer_broadcast(int cpu) {
	jzsoc_send_ipi_single(cpu,SMP_IPI_TIMER);
}

void smp_enable_interrupt(int cpu) {
	int status;
	unsigned long flags;
	smp_spinlock(flags);
	status = get_smp_reim();
	status |= 1 << (8 + cpu);
	set_smp_reim(status);
	smp_spinunlock(flags);
}
void smp_disable_interrupt(int cpu) {
	int status;
	unsigned long flags;
	smp_spinlock(flags);
	status = get_smp_reim();
	status &= ~(1 << (8 + cpu));
	set_smp_reim(status);
	smp_spinunlock(flags);
}

void smpmask_disable_interrupt(int cpu) {
	int status;
	unsigned long flags;
	smp_spinlock(flags);
	status = get_smp_reim();
	status &= ~(0xf << 8);
	status |= cpu_reim << 8;
	if(cpu != -1)
		status &= ~(1 << (8 + cpu));
	set_smp_reim(status);
	smp_spinunlock(flags);
}
int smpmask_enable_interrupt(int cpu) {
	int status;
	unsigned long flags;
	if(!cpu_online(cpu))
		return -1;
	smp_spinlock(flags);
	status = get_smp_reim();
	status &= ~(0xf << 8);
	status |= 1 << (8 + cpu);
	set_smp_reim(status);
	smp_spinunlock(flags);
	return 0;
}

void smp_clear_cpu_pending(int cpu){
	unsigned int pending;
	unsigned long flags;
	smp_spinlock(flags);
	pending = get_smp_status();
	pending &= ~(1 << (cpu + 8));
	set_smp_status(pending);
	smp_spinunlock(flags);
}

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_EARLYSUSPEND_CPU)
static struct early_suspend smp_early_suspend;
#ifdef CONFIG_DELAY_CPU_UP
#define CPU_UP_DELAY 1500
static struct delayed_work cpuup_work;

static void cpuup_work_func(struct work_struct *work)
{
	enable_nonboot_cpus();
}

void smp_suspend(struct early_suspend *h)
{
	cancel_delayed_work_sync(&cpuup_work);
	if (num_online_cpus() > 1)
		disable_nonboot_cpus();
}

void smp_resume(struct early_suspend *h)
{
	schedule_delayed_work(&cpuup_work, msecs_to_jiffies(CPU_UP_DELAY));
}
#else
void smp_suspend(struct early_suspend *h)
{
	disable_nonboot_cpus();
}

void smp_resume(struct early_suspend *h)
{
	enable_nonboot_cpus();
}
#endif

static int __init init_smp_early_suspend(void)
{
	smp_early_suspend.level	  = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	smp_early_suspend.suspend = smp_suspend;
	smp_early_suspend.resume  = smp_resume;
	register_early_suspend(&smp_early_suspend);
/*
	if(scpu_regulator == NULL) {
		scpu_regulator = regulator_get(NULL, "scpu");
		if (IS_ERR(scpu_regulator)) {
			scpu_regulator = NULL;
		}
	}
*/
#ifdef CONFIG_DELAY_CPU_UP
	INIT_DELAYED_WORK(&cpuup_work, cpuup_work_func);
#endif
	return 0;
}

module_init(init_smp_early_suspend)
#endif
