#include <linux/sched.h>
#include <linux/kthread.h>

#include "NandThread.h"

PNandThread CreateThread(PThreadFunction fn,void *data,int prio,char *name)
{
	struct task_struct* thread = NULL;
	struct sched_param param = { .sched_priority = 1 };
	char threadName[80];
	static int index = 1;

	sprintf(threadName, "%s_%d", name, index);
	thread = kthread_create(fn, data, threadName);
	switch(prio)
	{
	case 0:
		sched_setscheduler(thread, SCHED_IDLE, &param);
		break;
	default:
		sched_setscheduler(thread, SCHED_FIFO, &param);
		break;
	}
	if (!IS_ERR(thread))
		wake_up_process(thread);

	index ++;
	return (int)thread;
}

int ExitThread(PNandThread *thread)
{
	return kthread_stop((struct task_struct*)(*thread));
}

void SetThreadPrio(PNandThread *thread,int prio)
{
	return;
}

void SetThreadState(PNandThread *thread, enum nd_thread_state state)
{
	int task_state, schedule_flag = 0;

	switch (state) {
	case ND_THREAD_RUNNING:
		task_state = TASK_RUNNING;
		break;
	case ND_THREAD_INTERRUPTIBLE:
		task_state = TASK_INTERRUPTIBLE;
		schedule_flag = 1;
		break;
	case ND_THREAD_UNINTERRUPTIBLE:
		task_state = TASK_UNINTERRUPTIBLE;
		break;
	default:
		printk("ERROR: %s, Unsupported thread state %d!\n", __func__, state);
		return;
	}

	set_task_state((struct task_struct*)(*thread), task_state);

	if (schedule_flag) {
		schedule();
	}
}
