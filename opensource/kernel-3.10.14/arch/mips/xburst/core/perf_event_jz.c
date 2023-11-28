#include <asm/processor.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <asm-generic/errno-base.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/kernel_stat.h>

#include <asm/mmu_context.h>

#define read_jz_perf_counter(source, sel)				\
({ int __res;								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " #source ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})


#define write_jz_perf_counter(register, sel, value)			\
({ do {									\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%0, " #register ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "r" ((unsigned int)(value)));		\
	} while (0);									\
})

void save_perf_event_jz(void *tskvoid)
{
	struct task_struct *tsk = (struct task_struct *)tskvoid;
	tsk->thread.pfc[0].perfctrl = read_jz_perf_counter($25, 0);
	tsk->thread.pfc[0].perfcnt += read_jz_perf_counter($25, 1);

	tsk->thread.pfc[1].perfctrl = read_jz_perf_counter($25, 2);
	tsk->thread.pfc[1].perfcnt += read_jz_perf_counter($25, 3);
}

void restore_perf_event_jz(void *tskvoid)
{
	struct task_struct *tsk = (struct task_struct *)tskvoid;
	write_jz_perf_counter($25, 0, tsk->thread.pfc[0].perfctrl);
	write_jz_perf_counter($25, 1, 0);

	write_jz_perf_counter($25, 2, tsk->thread.pfc[1].perfctrl);
	write_jz_perf_counter($25, 3, 0);
}
