
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/sched.h>

#include <asm/ptrace.h>
//#include <asm/thread_info.h>

#ifdef CONFIG_SUSER

#define DIE_PMON_SET_CP0   887
#define DIE_PMON_CLEAR_CP0 889
static int pmon_exceptions_notify(struct notifier_block *self,
				  unsigned long val, void *data)
{

	struct die_args *args = (struct die_args *)data;
	struct pt_regs *regs = args->regs;
	int ret = NOTIFY_DONE;
	long code = args->err;

	if(val != DIE_TRAP)
		return ret;

	switch (code) {
	case DIE_PMON_SET_CP0:
		regs->cp0_status |= 1<<28;
		printk(KERN_DEBUG "[PMON] set cp0, code=%ld\n",code);
		regs->cp0_epc += 4;
		ret = NOTIFY_STOP;
		break;
	case DIE_PMON_CLEAR_CP0:
		regs->cp0_status &= ~(1<<28);
		printk(KERN_DEBUG "[PMON] clear cp0, code=%ld\n",code);
		regs->cp0_epc += 4;
		ret = NOTIFY_STOP;
		break;
	default:
		break;
	}
	return ret;
}
static struct notifier_block pmon_exceptions_nb = {
	.notifier_call = pmon_exceptions_notify,
	.priority = 0x1 /* we needn't to be notified first */
};
#endif

static int __init init_jz_pmon(void)
{
#ifdef CONFIG_SUSER
	register_die_notifier(&pmon_exceptions_nb);
	printk("[PMON] Jz Pmon module startup.\n");
#endif
	return 0;
}

module_init(init_jz_pmon);

