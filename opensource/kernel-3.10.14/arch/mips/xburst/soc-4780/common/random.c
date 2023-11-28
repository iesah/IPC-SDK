/*
 * JZSOC Random Generator
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/syscore_ops.h>

#include <mach/jz4780_random.h>

#include <soc/cpm.h>
#include <soc/base.h>
#include <soc/extal.h>

/*********************************************\
|* In the cpm register group, cpm_base + D8  *|
|* is random_generator enable bit,           *|
|* enable: 1, disable: 0                     *|
\*********************************************/
static int enable_random_generator(void)
{
	unsigned int enable_reg;
	int ret = 0;
	int i = 100;

	/* enable random register */
	do {
		enable_reg = cpm_inl(CPM_RANDEN);
		enable_reg |= 1;
		cpm_outl(enable_reg, CPM_RANDEN);

		enable_reg = cpm_inl(CPM_RANDEN);
		if (enable_reg & 1) {
			ret = 0;
			break;
		} else {
			printk("    disable_random failed!\n");
			i--;
			ret = -1;
		}
	} while(i);

	return ret;
}

static int disable_random_generator(void)
{
	unsigned int enable_reg = 0;
	int i = 0;
	int ret = 0;

	for (i = 0; i < 100; i++) {
		enable_reg = cpm_inl(CPM_RANDEN);
		enable_reg &= ~(1<<0);
		cpm_outl(enable_reg, CPM_RANDEN);

		enable_reg = cpm_inl(CPM_RANDEN);
		if (enable_reg & 1) {
			printk("    disable_random failed!\n");
			ret = -1;
		} else {
			ret = 0;
			break;
		}
	}

	return ret;
}

/*********************************************\
|* If you just want one random number, you   *|
|* call this func, it no need to affrent     *|
|* parameter.                                *|
\*********************************************/
unsigned int generate_one_random(void)
{
	int ret = 0;
	unsigned int random_num;

	ret = enable_random_generator();
	if (ret)
		goto err;
	random_num = cpm_inl(CPM_RANDNUM);
		mdelay(1);
	ret = disable_random_generator();
	if (ret)
		goto err;

	return random_num;

err:
	return ret;
}
EXPORT_SYMBOL(generate_one_random);

/*********************************************\
|* If you want a set of number, you call     *|
|* this func, you must affrent parameter.    *|
|* random_num: use to storage random number  *|
|* num: The number of you request.           *|
\*********************************************/
unsigned int generate_random(unsigned int *random_num, int num)
{
	int ret = 0, i = 0;

	ret = enable_random_generator();
	if (ret)
		goto err;
	for (i = 0; i < num; i++) {
		random_num[i] = cpm_inl(CPM_RANDNUM);
		mdelay(1);
	//	printk("random_num[%d]: 0x%08x \n", i, random_num[i]);
	}
	ret = disable_random_generator();

err:
	return ret;
}
EXPORT_SYMBOL(generate_random);

/******************************************************\
|* The proc node, if you want a random number, in     *|
|* serial ports, you will cd in the directory: /proc/ *|
|* , and then : cat random_generator. You will see a  *|
|* random number in the serial ports.                 *|
\******************************************************/
static int read_random_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int len = 0;
	unsigned int random_num = 0;

#define PRINT(ARGS...) len += sprintf (page+len, ##ARGS)
	    PRINT("ID NAME       FRE        stat       count     parent\n");

	random_num = generate_one_random();
	PRINT("random_num: 0x%08x\n", random_num);

#undef PRINT

	return len;
}

static int __init init_generate_random(void)
{
	struct proc_dir_entry *res;

	res = create_proc_entry("random_generator", 0444, NULL);
	if (res) {
		res->read_proc = read_random_proc;
		res->write_proc = NULL;
		res->data = NULL;
	}
	return 0;
}
module_init(init_generate_random);
