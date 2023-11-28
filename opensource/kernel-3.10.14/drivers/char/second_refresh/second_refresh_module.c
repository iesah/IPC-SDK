#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/mm.h>
#include <asm/cacheops.h>
#include <linux/dma-mapping.h>
#include <mach/jzdma.h>

#include <linux/second_refresh.h>

//#define FIRMWARE_LOAD_ADDRESS	(0x2f900000)
//#define FIRMWARE_LOAD_ADDRESS	(0xC0000000)
#define FIRMWARE_LOAD_ADDRESS	(0x20000000)

static int second_refresh_firmware [] = {
#include "second_refresh_firmware.hex"
};


extern void setup_tlb(void);


struct second_refresh_ops {
	/*pay attention to the order*/
	/*wakeup module for host*/
	int (*open)(int mode);
	int (*handler)(int args);
	int (*close)(int mode);
	int (*cache_prefetch)(void);

	/*host for wakeup module*/
	int (*set_handler)(void *);
};

static struct second_refresh_ops *m_ops;


static void dump_firmware(void)
{
	int i;
	unsigned int *p = (unsigned int *)FIRMWARE_LOAD_ADDRESS;
	printk("###################dump_firmware begine################\n");
	for(i = 0; i < 64; i++) {
		printk("1.%p:%08x\n", &second_refresh_firmware[i], second_refresh_firmware[i]);
		printk("2.%p:%08x\n", p+i, *(p+i));
	}
	printk("###################dump_firmware end################\n");

}

static void setup_ops(void)
{
	printk("###############setup_ops##############\n");
	m_ops = (struct second_refresh_ops *)FIRMWARE_LOAD_ADDRESS;
	//m_ops->set_handler(printk);
	printk("open:%p\n", m_ops->open);
	printk("handler:%p\n", m_ops->handler);
	printk("close:%p\n", m_ops->close);
	printk("set_handler:%p\n", m_ops->set_handler);
	printk("###############ops end##############\n");
}

static void test_ops(void)
{
	printk("printk:%p", printk);
	m_ops->set_handler(printk);
	printk("###############test_ops##############\n");
	printk("m_ops.open:%x\n", m_ops->open(1));
	printk("m_ops.handler:%x\n", m_ops->handler(1));
	printk("m_ops.close:%x\n", m_ops->close(1));
	printk("###############ops end##############\n");
}

int second_refresh_open(int mode)
{	printk("second refres cache prefetch\n");
	memcpy((void *)FIRMWARE_LOAD_ADDRESS, second_refresh_firmware, sizeof(second_refresh_firmware));

}
EXPORT_SYMBOL(second_refresh_open);

int second_refresh_handler(int par)
{
	return m_ops->handler(par);
}
EXPORT_SYMBOL(second_refresh_handler);

int second_refresh_close(int mode)
{
	return m_ops->close(mode);
}
EXPORT_SYMBOL(second_refresh_close);

void second_refresh_cache_prefetch(void)
{
	setup_ops();
	test_ops();
	m_ops->cache_prefetch();
}
EXPORT_SYMBOL(second_refresh_cache_prefetch);


static int __init second_refresh_init(void)
{
	/* load voice wakeup firmware */

	return 0;
}
static void __exit second_refresh_exit(void)
{


}

module_init(second_refresh_init);
module_exit(second_refresh_exit);

MODULE_AUTHOR("qipengzhen<aric.pzqi@ingenic.com>");
MODULE_DESCRIPTION("a simple test driver");
MODULE_LICENSE("GPL");
