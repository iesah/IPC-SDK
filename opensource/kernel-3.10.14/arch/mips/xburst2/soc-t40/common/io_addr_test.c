#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/syscore_ops.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <linux/string.h>

static void __io_addr_test(void)
{
	unsigned int addr_start = 0xb0000000;
	unsigned int addr_size = 0x10000000;
	unsigned int step = 4096;
	unsigned int val, i;

	for (i = addr_start; i < addr_start + addr_size; i = i + step)
	{
		printk("0x%x", i);
		val = *(volatile unsigned int *)i;
		printk(" e\n");
	}
}


static ssize_t io_addr_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	__io_addr_test();
	return count;
}



static struct device_attribute io_addr_test_attribute[] = {
	__ATTR(io_addr_test, S_IWUSR, NULL, io_addr_test),
};


static struct bus_type io_addr_subsys = {
	.name		= "io_addr",
	.dev_name	= "io_addr",
};

static int __init io_addr_test_init(void)
{
	int ret;
	int i;

	ret = subsys_system_register(&io_addr_subsys, NULL);
	if (ret) {
		printk("register io_addr subsys failed\n");
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(io_addr_test_attribute); i++) {
		ret = device_create_file(io_addr_subsys.dev_root, &io_addr_test_attribute[i]);
		if (ret) {
			printk("io_addr device_create_file failed\n");
			return -1;
		}
	}

	return 0;
}
static void __exit io_addr_test_deinit(void)
{
}

late_initcall(io_addr_test_init);
module_exit(io_addr_test_deinit);
