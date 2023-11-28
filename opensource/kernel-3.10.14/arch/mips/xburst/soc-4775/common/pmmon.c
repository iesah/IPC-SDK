/*
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

#include <soc/cpm.h>
#include <soc/base.h>
#include <soc/extal.h>

struct jz_pmvd_node {
	char *name;
	unsigned int iobase;
	unsigned int offset;
	unsigned int mask;
	int active_low;
};

static struct jz_pmvd_node jz_pmvd_table[] = {
	{.name = "lcd0",	.iobase = LCDC0_IOBASE,	.offset=0x30, .mask = 0x8, .active_low = 0},
	{.name = "lcd1",	.iobase = LCDC1_IOBASE,	.offset=0x30, .mask = 0x8, .active_low = 0},
	{NULL,0,0,0,0},//end flag
};

int pmvd_suspend(void)
{
	struct jz_pmvd_node *n = jz_pmvd_table;
	while(1) {
		if(n->name == NULL)
			break;
		if((!!(readl((void *)CKSEG1ADDR((n->iobase + n->offset)))
					& n->mask)) ^ n->active_low)
			printk("%s module has not shutdown.",n->name);
		n++;
	}

	return 0;
}

struct syscore_ops pmvd_pm_ops = {
	.suspend = pmvd_suspend,
};

int __init setup_pmvd_pins(void)
{
	register_syscore_ops(&pmvd_pm_ops);
	return 0;
}

arch_initcall(setup_pmvd_pins);

