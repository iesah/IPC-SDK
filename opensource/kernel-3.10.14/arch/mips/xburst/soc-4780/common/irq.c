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

#include <linux/proc_fs.h>

#include <asm/irq_cpu.h>

#include <soc/base.h>
#include <soc/irq.h>
#include <soc/ost.h>

#include <linux/io.h>
#include <smp_cp0.h>
#include <linux/uaccess.h>

#define TRACE_IRQ        1
#define PART_OFF	0x20

#define ISR_OFF		(0x00)
#define IMR_OFF		(0x04)
#define IMSR_OFF	(0x08)
#define IMCR_OFF	(0x0c)
#define IPR_OFF		(0x10)

#define regr(off) 	inl(OST_IOBASE + (off))
#define regw(val,off)	outl(val, OST_IOBASE + (off))

static void __iomem *intc_base;
static unsigned long intc_saved[2];
static unsigned long intc_wakeup[2];
//#define IRQ_TIME_MONITOR_DEBUG
#ifdef IRQ_TIME_MONITOR_DEBUG
unsigned long long time_monitor[500] = {0};
int time_over[500] = {0};
int echo_success = 0;
#endif

#ifdef IRQ_TIME_MONITOR_DEBUG
enum {
	IRQ_ID_AIC = 9,
#define IRQ_NAME_AIC		"aic_irq"
	IRQ_ID_HDMI = 11,
#define IRQ_NAME_HDMI		"HDMI TX_INT"
	IRQ_ID_PDMA = 18,
#define IRQ_NAME_PDMA		"pdma"
	IRQ_ID_GPIO_F = 20,
#define IRQ_NAME_GPIO_F		"GPIO F"
	IRQ_ID_GPIO_E = 21,
#define IRQ_NAME_GPIO_E		"GPIO E"
	IRQ_ID_GPIO_D = 22,
#define IRQ_NAME_GPIO_D		"GPIO D"
	IRQ_ID_GPIO_C = 23,
#define IRQ_NAME_GPIO_C		"GPIO C"
	IRQ_ID_GPIO_B = 24,
#define IRQ_NAME_GPIO_B		"GPIO B"
	IRQ_ID_GPIO_A = 25,
#define IRQ_NAME_GPIO_A		"GPIO A"

	/**********************************************************************************/
	IRQ_ID_X2D = 27,
#define IRQ_NAME_X2D		"x2d"
	IRQ_ID_OTG = 29,
#define IRQ_NAME_OTG		"dwc_otg, dwc_otg_pcd, dwc_otg_hcd:usb1"
	IRQ_ID_IPU0 = 30,
#define IRQ_NAME_IPU0		"ipu0"
	IRQ_ID_LCDC1 = 31,
#define IRQ_NAME_LCDC1		"lcdc1"
	IRQ_ID_CPU0TIMER = 34,
#define IRQ_NAME_CPU0TIMER	"CPU0 jz-timerirq"
	IRQ_ID_CPU1TIMER = 35,
#define IRQ_NAME_CPU1TIMER	"CPU1 jz-timerirq"
	IRQ_ID_IPU1 = 37,
#define IRQ_NAME_IPU1		"ipu1"
	IRQ_ID_CIM = 38,
#define IRQ_NAME_CIM		"jz-cim"
	IRQ_ID_LCDC0 = 39,
#define IRQ_NAME_LCDC0		"lcdc0"
	IRQ_ID_RTC = 40,
#define IRQ_NAME_RTC		"rtc 1Hz and alarm"
	IRQ_ID_MMC1 = 44,
#define IRQ_NAME_MMC1		"jzmmc.1"
	IRQ_ID_MMC0 = 45,
#define IRQ_NAME_MMC0		"jzmmc.0"
	IRQ_ID_AOSD = 51,
#define IRQ_NAME_AOSD		"jz-aosd"
	IRQ_ID_UART3 = 56,
#define IRQ_NAME_UART3		"uart3"
	IRQ_ID_I2C3 = 65,
#define IRQ_NAME_I2C3		"jz-i2c.3"
	IRQ_ID_I2C1 = 67,
#define IRQ_NAME_I2C1		"jz-i2c.1"
	IRQ_ID_PDMAM = 69,
#define IRQ_NAME_PDMAM		"pdmam"
	IRQ_ID_VPU = 70,
#define IRQ_NAME_VPU		"vpu"
	IRQ_ID_SGXISR = 71,
#define IRQ_NAME_SGXISR		"SGX ISR"
};

struct irq_timer_monitor
{
	const char *name;
};

static struct irq_timer_monitor irq_srcs[] = {
#define DEF_IRQ(N)						\
		[IRQ_ID_##N] = { .name = IRQ_NAME_##N }

		DEF_IRQ(AIC),
		DEF_IRQ(HDMI),
		DEF_IRQ(PDMA),
		DEF_IRQ(GPIO_F),
		DEF_IRQ(GPIO_E),
		DEF_IRQ(GPIO_D),
		DEF_IRQ(GPIO_C),
		DEF_IRQ(GPIO_B),
		DEF_IRQ(GPIO_A),
		DEF_IRQ(OTG),
		DEF_IRQ(IPU0),
		DEF_IRQ(LCDC1),
		DEF_IRQ(CPU0TIMER),
		DEF_IRQ(CPU1TIMER),
		DEF_IRQ(IPU1),
		DEF_IRQ(CIM),
		DEF_IRQ(LCDC0),
		DEF_IRQ(RTC),
		DEF_IRQ(MMC1),
		DEF_IRQ(MMC0),
		DEF_IRQ(AOSD),
		DEF_IRQ(UART3),
		DEF_IRQ(I2C3),
		DEF_IRQ(I2C1),
		DEF_IRQ(PDMAM),
		DEF_IRQ(VPU),
		DEF_IRQ(SGXISR),
#undef DEF_IRQ
};
#endif
static DEFINE_SPINLOCK(r_ipr_lock);
#define ipr_spinlock(flags) 		spin_lock_irqsave(&r_ipr_lock,flags)
#define ipr_spinunlock(flags) 	spin_unlock_irqrestore(&r_ipr_lock,flags)

#ifdef CONFIG_SMP

static unsigned int cpu_irq_affinity[NR_CPUS];

//static unsigned int cpu_mask_affinity[NR_CPUS];

//--------------------------------------------------------------
/*
  for Recently the corresponding interrupt of core
*/

static unsigned int irq_resp_fifo[NR_CPUS];
static void init_irq_resp_fifo(void) {
	int i;
	for(i = 0;i < NR_CPUS;i++) {
		irq_resp_fifo[i] = -1;
	}
}
static void push_irq_resp_fifo(int num,int cpu) {
	irq_resp_fifo[cpu] = num;
}

static int find_irq_for_cpu(int num) {
	int j;
	int n = -1;
	for(j = 0; j < NR_CPUS;j++)
		if(irq_resp_fifo[j] == num) {
			n = j;
		}
	// -1 for any core;belse for response coreid;
	return n;
}

void reset_irq_resp_fifo(int cpu) {
	unsigned long flags;
	ipr_spinlock(flags);
	irq_resp_fifo[cpu] = -1;
	ipr_spinunlock(flags);
}

//--------------------------------------------------------------
/*
 *    for Interrupt priority cycle accordingly
 *    when irq to response then prev_irq is this bit,then next irq will
 *    not response
 */
static unsigned int prev_irq[2];
static int get_irq_number(unsigned int irq0,int irq1) {
	int n1 = -1,n0 = -1;
	unsigned int v0,v1;
	v0 = irq0 & (~prev_irq[0]);
	v1 = irq1 & (~prev_irq[1]);
	if(v0) {
		n0 = ffs(v0) - 1;
	}else if(v1) {
		n1 = ffs(v1) - 1;
	} else {
		if(irq0) {
			n0 = ffs(irq0) - 1;
			prev_irq[0] = 0;
		} else if(irq1) {
			n1 = ffs(irq1) - 1;
			prev_irq[1] = 0;
		}
	}
	if(n0 != -1)
		return n0;
	if(n1 != -1)
		return n1 + 32;
	return -1;
}
static void set_irq_number(int n) {
	prev_irq[n / 32] |= 1 << (n % 32);
}
//--------------------------------------------------------------
//* handle mask & unmask for intc
static unsigned int irq_intc_mask[2];
static unsigned int ctrlirq_intc_mask[2];
static void irq_intc_ctrlmask_save(int number,int msk) {
	void *base = intc_base + PART_OFF * (number / 32);
//	if(number == 17)
//		printk("&&&&&&&&&&&&& irq_intc_ctrlmask_save %d = %d\n",number,msk);
	if (msk == 1) {
		irq_intc_mask[number / 32] |= BIT(number % 32);
		writel(BIT(number%32), base + IMSR_OFF);
	}else if(msk == 0) {
		irq_intc_mask[number / 32] &= ~BIT(number % 32);
		if(!(ctrlirq_intc_mask[number / 32] & BIT(number % 32))){
			writel(BIT(number%32), base + IMCR_OFF);
		}
	}
}
static void init_intc_mask(void){
	void *base = intc_base;
	irq_intc_mask[0] = readl(base + IMR_OFF);
	irq_intc_mask[1] = readl(base + IMR_OFF + PART_OFF);
}

static void irq_intc_ctrlmask(int number,int msk) {
	void *base = intc_base + PART_OFF * (number / 32);
	if (msk == 1) {
		ctrlirq_intc_mask[number / 32] |= BIT(number%32);
		writel(BIT(number%32), base + IMSR_OFF);
	}else if(msk == 0) {

		if(!(irq_intc_mask[number / 32] & (1 << number % 32)))
			writel(BIT(number%32), base + IMCR_OFF);
		ctrlirq_intc_mask[number / 32] &= ~BIT(number%32);
	}
//	if(number == 17)
//		printk("irq_intc %d = %x",number,readl(base + IMR_OFF));
}

/*
static void irq_intc_ctrlmask_affinity(int cpu,int msk) {
	void *base = intc_base;
	if(msk == 1) {
		writel(cpu_irq_affinity[cpu], base + IMSR_OFF);
	}else{
		writel(cpu_irq_affinity[cpu] & ~irq_intc_mask[0], base + IMCR_OFF);
	}

}
*/
//--------------------------------------------------------------
static void set_intc_cpu(unsigned long irq_num,long cpu) {
	int mask,i;
	unsigned long flags;
	int num = irq_num / 32;
	mask = 1 << (irq_num % 32);
	BUG_ON(num);
	ipr_spinlock(flags);
	for(i = 0;i < NR_CPUS;i++) {
		if(i != cpu) {
			cpu_irq_affinity[i] &= ~mask;
		}else{
			cpu_irq_affinity[i] |= mask;
		}
	}
	if(cpu_online(cpu)) {
		smp_enable_interrupt(cpu);
	}
	ipr_spinunlock(flags);
	printk("enable cpu %d\n",(int)cpu);
}
static inline void init_intc_affinity(void) {
	int i;
	for(i = 0;i < NR_CPUS;i++) {
		cpu_irq_affinity[i] = 0;
	}
}
static inline int find_intc_affinity_cpu(int irqnumber) {
	int i;
	if(irqnumber < 32) {
		for(i = 0;i < NR_CPUS;i++) {
			if((1 << irqnumber) & cpu_irq_affinity[i])
				return i;
		}
	}
	return -1;
}

static int intc_set_affinity(struct irq_data *data, const struct cpumask *dest, bool force) {
	return 0;
}
#endif

void jz_set_cpu_affinity(unsigned long irq,int cpu) {
	set_intc_cpu(irq - IRQ_INTC_BASE,cpu);
}

extern void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume);

static void intc_irq_ctrl(struct irq_data *data, int msk, int wkup)
{
	int intc = (int)irq_data_get_irq_chip_data(data);
	unsigned long flags;
	ipr_spinlock(flags);
	irq_intc_ctrlmask_save(intc,msk);
	ipr_spinunlock(flags);
	if (wkup == 1)
		intc_wakeup[intc / 32] |= 1 << (intc % 32);
	else if (wkup == 0)
		intc_wakeup[intc / 32] &= ~(1 << (intc % 32));
}

static void intc_irq_unmask(struct irq_data *data)
{
	intc_irq_ctrl(data, 0, -1);
}

static void intc_irq_mask(struct irq_data *data)
{
	intc_irq_ctrl(data, 1, -1);
}

static int intc_irq_set_wake(struct irq_data *data, unsigned int on)
{
	intc_irq_ctrl(data, -1, !!on);
	return 0;
}
static struct irq_chip jzintc_chip = {
	.name 		= "jz-intc",
	.irq_mask	= intc_irq_mask,
	.irq_mask_ack 	= intc_irq_mask,
	.irq_unmask 	= intc_irq_unmask,
	.irq_set_wake 	= intc_irq_set_wake,
#ifdef CONFIG_SMP
	.irq_set_affinity = intc_set_affinity,
#endif
};

#ifdef CONFIG_SMP
extern void jzsoc_mbox_interrupt(int cpuid);
static irqreturn_t ipi_reschedule(int irq, void *d)
{
	scheduler_ipi();
	return IRQ_HANDLED;
}

static irqreturn_t ipi_call_function(int irq, void *d)
{
	smp_call_function_interrupt();
	return IRQ_HANDLED;
}

static int setup_ipi(void)
{
	if (request_irq(IRQ_SMP_RESCHEDULE_YOURSELF, ipi_reschedule, IRQF_DISABLED,
				"ipi_reschedule", NULL))
		BUG();
	if (request_irq(IRQ_SMP_CALL_FUNCTION, ipi_call_function, IRQF_DISABLED,
				"ipi_call_function", NULL))
		BUG();

	set_c0_status(STATUSF_IP3);
	return 0;
}


#endif

static void ost_irq_unmask(struct irq_data *data)
{
	regw(0xffffffff,  OST_TMCR);
}

static void ost_irq_mask(struct irq_data *data)
{
	regw(0xffffffff,  OST_TMSR);
}

static void ost_irq_mask_ack(struct irq_data *data)
{
	regw(0xffffffff,  OST_TMSR);
	regw(0xffffffff,  OST_TFCR);  /* clear ost flag */
}

static struct irq_chip ost_irq_type = {
	.name 		= "ost",
	.irq_mask	= ost_irq_mask,
	.irq_mask_ack 	= ost_irq_mask_ack,
	.irq_unmask 	= ost_irq_unmask,
};

void __init arch_init_irq(void)
{
	int i;

	clear_c0_status(0xff04); /* clear ERL */
	set_c0_status(0x0400);   /* set IP2 */

	/* Set up MIPS CPU irq */
	mips_cpu_irq_init();

	/* Set up INTC irq */
	intc_base = ioremap(INTC_IOBASE, 0xfff);

	writel(0xffffffff, intc_base + IMSR_OFF);
	writel(0xffffffff, intc_base + PART_OFF + IMSR_OFF);
	for (i = IRQ_INTC_BASE; i < IRQ_INTC_BASE + INTC_NR_IRQS; i++) {
		irq_set_chip_data(i, (void *)(i - IRQ_INTC_BASE));
		irq_set_chip_and_handler(i, &jzintc_chip, handle_level_irq);
	}

	for (i = IRQ_OST_BASE; i < IRQ_OST_BASE + OST_NR_IRQS; i++) {
		irq_set_chip_data(i, (void *)(i - IRQ_OST_BASE));
		irq_set_chip_and_handler(i, &ost_irq_type, handle_level_irq);
	}
#ifdef CONFIG_SMP
	init_intc_affinity();
	set_intc_cpu(26,0);
//	set_intc_cpu(27,1);
#endif
	init_intc_mask();

	init_irq_resp_fifo();
	smp_enable_interrupt(0);
	/* enable cpu interrupt mask */
	set_c0_status(IE_IRQ0 | IE_IRQ1);
#ifdef CONFIG_SMP
	setup_ipi();
#endif
	return;
}
#if 0
static unsigned int generic_irq_mask[2] = {0x3f000,0x9f0f0004};
static void do_irq_dispatch(int number) {

	if(generic_irq_mask[number / 32] & (1 << number % 32)) {
		generic_handle_irq(number + IRQ_INTC_BASE);
	}else {
		do_IRQ(number + IRQ_INTC_BASE);
	}
}
#endif
static int cpu_send_irq = -1;
static int irq_to_cpu = -1;
int send_irq_to_cpu(int irq,int cpu) {
	if(smpmask_enable_interrupt(cpu) == 0) {
		irq_to_cpu = cpu;
		cpu_send_irq = irq;
		return 0;
	}
	//printk("cpu%d offline\n",cpu);
	return -1;
}
static unsigned int response_cpu_busy = 0;
#define DEBUG_IRQ
#ifdef DEBUG_IRQ
static unsigned int irq_count[64];
static unsigned int irq_count_msk[64];
static unsigned int gpio_irq_flg[5];
static unsigned int gpio_irq_msk[5];
void debug_check_irqcount(int num) {
	int i;
	unsigned int base = 0;
	irq_count[num]++;
	if(num < 18 && num >=12) {
		base = 0xb0010050 + (5 - (num - 12)) * 0x100;
		gpio_irq_flg[num - 12] = *(unsigned int *)base;
		base = 0xb0010020 + (5 - (num - 12)) * 0x100;
		gpio_irq_msk[num - 12] = *(unsigned int *)base;
	}
	if(num == 26) {
		for(i = 0;i < 64;i++){
			if(irq_count[i] > 2000) {
				printk("number = %d count = %d\n",i,irq_count[i]);
				if(i < 18 && i >= 12) {
					printk("GPIO FLG %x\n",gpio_irq_flg[i - 12]);
					printk("GPIO MSK %x\n",gpio_irq_msk[i - 12]);
				}
				if(irq_count_msk[i] < 100)
					irq_count_msk[i]++;
			}else
				irq_count_msk[i] = 0;
			irq_count[i] = 0;
		}
	}
}
#endif
static void intc_irq_dispatch(int cpuid)
{
	unsigned int ipr_intc = 0,ipr_intc1 = 0;
	int n = -1;
	int for_cpu = 0;
	unsigned long flags;
	//void *base;
	ipr_spinlock(flags);
	if(irq_to_cpu != -1) {
		if(irq_to_cpu == cpuid) {
			n = cpu_send_irq;
			goto irq_affinity;
		}else{
			if(!cpu_online(irq_to_cpu)) {
				n = cpu_send_irq;
				printk("cpu%d offline use %d response %d irq\n",irq_to_cpu,cpuid,n);
				goto irq_affinity;
			}else {
				// should use this core response other irq,but no complete
				goto  irq_unlock;
			}
		}
	}
	ipr_intc = readl(intc_base + IPR_OFF);
	ipr_intc1 = readl(intc_base + PART_OFF + IPR_OFF);
	if((ipr_intc | ipr_intc1) == 0)
		goto irq_unlock;
	n = get_irq_number(ipr_intc,ipr_intc1);
	if(n == -1)
		goto irq_unlock;

	for_cpu = find_intc_affinity_cpu(n);
	if(for_cpu != -1) {
		if(for_cpu != cpuid && send_irq_to_cpu(n,for_cpu) == 0) {
			//printk("ipr_intc = %x mask = %x\n",ipr_intc,cpuid);
			n = -1;
			goto irq_unlock;
		}else{
			goto irq_affinity;
		}
	}
	for_cpu = find_irq_for_cpu(n);
	if(for_cpu != -1 && for_cpu != cpuid){
		if((response_cpu_busy & ~(1 << cpuid)) == 0 && send_irq_to_cpu(n,for_cpu) == 0){
			n = -1;
			goto irq_unlock;
		}
	}
	push_irq_resp_fifo(n,cpuid);
	set_irq_number(n);

irq_affinity:
	irq_intc_ctrlmask(n,1);
#ifdef DEBUG_IRQ
	debug_check_irqcount(n);
#endif
	if(irq_to_cpu != -1){
		smpmask_disable_interrupt(-1);
		irq_to_cpu = -1;
	}
	if(n != -1)
		response_cpu_busy |= (1 << cpuid);
irq_unlock:
	ipr_spinunlock(flags);
	if(n != -1) {
		//base = intc_base + PART_OFF * (n / 32);
		//printk("++++ number %d = %x\n",n,readl(base + IMR_OFF));
		do_IRQ(n + IRQ_INTC_BASE);
		//do_irq_dispatch(n);
		ipr_spinlock(flags);
#ifdef DEBUG_IRQ
		if(irq_count_msk[n] < 6)
#endif
			irq_intc_ctrlmask(n,0);
		//base = intc_base + PART_OFF * (n / 32);
		//printk("---- number %d = %x\n",n,readl(base + IMR_OFF));
		response_cpu_busy &= ~(1 << cpuid);
		ipr_spinunlock(flags);

	}
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned int cause = read_c0_cause();
	unsigned int pending;
	int cpuid = smp_processor_id();
	unsigned long flags;
	pending = cause & read_c0_status() & ST0_IM;
#ifdef CONFIG_SMP
	if(pending & CAUSEF_IP3) {
		ipr_spinlock(flags);
		//irq_intc_ctrlmask_affinity(cpuid,1);
		response_cpu_busy |= 1 << cpuid;
		ipr_spinunlock(flags);

		jzsoc_mbox_interrupt(cpuid);

		ipr_spinlock(flags);
		response_cpu_busy &= ~(1 << cpuid);
		//irq_intc_ctrlmask_affinity(cpuid,0);
		ipr_spinunlock(flags);
	}
#endif
	if (cause & CAUSEF_IP4) {
		do_IRQ(IRQ_OST);
	}

	if(pending & CAUSEF_IP2) {
		intc_irq_dispatch(cpuid);
	}
}

void arch_suspend_disable_irqs(void)
{
	int i,j,irq;
	struct irq_desc *desc;

	local_irq_disable();

	intc_saved[0] = readl(intc_base + IMR_OFF);
	intc_saved[1] = readl(intc_base + PART_OFF + IMR_OFF);

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
	writel(0xffffffff & ~intc_saved[0], intc_base + IMCR_OFF);
	writel(0xffffffff & ~intc_saved[1], intc_base + PART_OFF + IMCR_OFF);
	local_irq_enable();
}

#ifdef IRQ_TIME_MONITOR_DEBUG
/* cat gap_time_irq to show it */
static int irq_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int len = 0;
	int i;
	struct irq_desc *desc;

#define PRINT(ARGS...) len += sprintf (page+len, ##ARGS)
	PRINT("ID NAME       gap_time   over_times       dev_name\n");
	for(i = 0; i < 499; i++) {
		if (time_monitor[i] == 0)
			continue;
		else {
			desc = irq_to_desc(i);
			if (desc->action == NULL)
				PRINT("%3d %18lld   %10d       %s \n", i, time_monitor[i], time_over[i], irq_srcs[i].name);
			else
				PRINT("%3d %18lld   %10d       %s \n", i, time_monitor[i], time_over[i],desc->action->name);
		}
	}
	PRINT("EER: \n");

	return len;
}

/* echo value to clear time_monitor value*/
static int irq_write_proc(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	int ret, i;
	char buf[32];
	unsigned long long cgr;

	if (count > 32)
		count = 32;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	ret = sscanf(buf,"%lld", &cgr);
	if (cgr == 0)
		for (i = 0; i < 499; i++) {
			time_monitor[i] = 0;
			time_over[i] = 0;
			if (i == 498)
				printk(" clear time_monitor successed!! \n");
		}
	else {
		time_monitor[499] = cgr;
		printk(" echo value successed: %lld \n", time_monitor[499]);
		echo_success = 1;
	}

	return count;
}

static int __init init_irq_proc_time(void)
{
	struct proc_dir_entry *res;

	res = create_proc_entry("gap_time_irq", 0666, NULL);
	if (res) {
		res->read_proc = irq_read_proc;
		res->write_proc = irq_write_proc;
		res->data = NULL;
	}
	return 0;
}

module_init(init_irq_proc_time);
#endif
