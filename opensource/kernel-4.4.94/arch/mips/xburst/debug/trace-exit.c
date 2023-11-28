#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <trace/events/sched.h>
#include <linux/ftrace.h>

static void
probe_sched_process_exit(void *d,struct task_struct *p)
{
	struct thread_struct *thread = &p->thread;

	printk("[EXIT] %s pid %d, ", p->comm, p->pid);
	printk("exit_stat %d, exit_signal %d\n",p->exit_code,p->exit_signal);
	printk("    badvaddr %lx, baduaddr %lx, error_code %ld\n",
	       thread->cp0_badvaddr,thread->cp0_baduaddr,thread->error_code);

	if (p->pid == 1)
		ftrace_dump(DUMP_ALL);
}

static int init_trace(void)
{
	int ret = -1;

	ret = register_trace_sched_process_exit(probe_sched_process_exit, NULL);
	if (ret)
		printk("register process exit failed.\n");

	return ret;
}
arch_initcall(init_trace);
