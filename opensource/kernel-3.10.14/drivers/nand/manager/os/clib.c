#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/math64.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>

#include "clib.h"

unsigned int nm_sleep(unsigned int seconds)
{
	msleep(seconds * 1000);
	return 0;
}

long long nd_getcurrentsec_ns(void)
{
	return sched_clock();
}

unsigned int nd_get_timestamp(void)
{
	return jiffies_to_msecs(jiffies);
}

int nm_print_message(enum nm_msg_type type, int arg)
{
	switch (type) {
	case NM_MSG_ERASE_PERSENT:
		printk("------------------> persent = %d\n", arg);
		break;
	}

	return 0;
}

unsigned int nd_get_phyaddr(void * addr)
{
	 unsigned int v;
	 if (unlikely((unsigned int)(addr) & 0x40000000)) {
		 v = page_to_phys(vmalloc_to_page((const void *)(addr))) | ((unsigned int)(addr) & ~PAGE_MASK);
	 } else
		 v = ((_ACAST32_((int)(addr))) & 0x1fffffff);
	 return v;
}

unsigned int nd_get_pid(void)
{
	return current->pid;
}
