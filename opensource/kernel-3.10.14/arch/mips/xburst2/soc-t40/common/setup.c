/*
 * JZSOC Clock and Power Manager
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
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>

#include <soc/base.h>
#include <soc/extal.h>
#include <mach/platform.h>

void __init cpm_reset(void)
{
}

int __init setup_init(void)
{
#if 0
	cpm_reset();
	/* Set bus priority here */
	*(volatile unsigned int *)0xb34f0240 = 0x00010003;
	*(volatile unsigned int *)0xb34f0244 = 0x00010003;
#endif

	return 0;
}
extern void __init init_all_clk(void);
/* used by linux-mti code */

void __init plat_mem_setup(void)
{
	/* use IO_BASE, so that we can use phy addr on hard manual
	 * directly with in(bwlq)/out(bwlq) in io.h.
	 */
	set_io_port_base(IO_BASE);
	ioport_resource.start	= 0x00000000;
	ioport_resource.end	= 0xffffffff;
	iomem_resource.start	= 0x00000000;
	iomem_resource.end	= 0xffffffff;
	setup_init();
	/*init_all_clk();*/

	return;
}

#ifdef CONFIG_SMP
//extern void percpu_timer_setup(void);
#endif
void __init xburst2_timer_setup(void);
void __init plat_time_init(void)
{

	init_all_clk();
	xburst2_timer_setup();

}
