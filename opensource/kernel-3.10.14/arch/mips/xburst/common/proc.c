#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/page-flags.h>
#include <asm/uaccess.h>
#include <jz_proc.h>
static struct proc_dir_entry *proc_jz_root;
struct proc_dir_entry * jz_proc_mkdir(char *s)
{
	struct proc_dir_entry *p;
	if(!proc_jz_root) {
		proc_jz_root = proc_mkdir("jz", 0);
		if(!proc_jz_root)
			return NULL;
	}
	p = proc_mkdir(s,proc_jz_root);
	return p;
}
EXPORT_SYMBOL(jz_proc_mkdir);

struct proc_dir_entry * get_jz_proc_root(void)
{
	if(!proc_jz_root) {
		proc_jz_root = proc_mkdir("jz", 0);
		if(!proc_jz_root)
			return NULL;
	}
	return proc_jz_root;
}

