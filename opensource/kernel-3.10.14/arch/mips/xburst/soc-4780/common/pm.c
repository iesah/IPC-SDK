/*
 * linux/arch/mips/jz4780/pm.c
 *
 *  JZ4780 Power Management Routines
 *  Copyright (C) 2006 - 2012 Ingenic Semiconductor Inc.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

#include <linux/init.h>
#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/suspend.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/sysctl.h>
#include <linux/delay.h>
#include <asm/cacheops.h>
#include <asm/rjzcache.h>
#include <linux/syscore_ops.h>
#include <linux/regulator/consumer.h>

#include <soc/cache.h>
#include <soc/base.h>
#include <soc/cpm.h>

void (*__reset_dll)(void);

#ifdef CONFIG_SUSPEND_SUPREME_DEBUG
#define U3_IOBASE 0xb0033000

#define OFF_TDR         (0x00)
#define OFF_LCR         (0x0C)
#define OFF_LSR         (0x14)

#define LSR_TDRQ        (1 << 5)
#define LSR_TEMT        (1 << 6)
#define TCSM_PCHAR(x)													\
	while ((*((volatile unsigned int*)(U3_IOBASE+OFF_LSR)) & (LSR_TDRQ | LSR_TEMT)) != (LSR_TDRQ | LSR_TEMT))	\
		;													\
	*((volatile unsigned int*)(U3_IOBASE+OFF_TDR)) = x
#else
#define TCSM_PCHAR(x)
#endif

#define TCSM_DELAY(x) \
	i=x;	\
	while(i--)	\
	__asm__ volatile(".set mips32\n\t"\
			"nop\n\t"\
			".set mips32")

void noinline reset_dll(void)
{
#define DELAY 0x1ff
	register int i;
	TCSM_PCHAR('0');
	*(volatile unsigned *)  0xB3010008 |= 0x1<<17;
	__jz_flush_cache_all();

	TCSM_PCHAR('1');
	*((volatile unsigned int*)(0xb30100b8)) &= ~(0x1);
	__asm__ volatile(".set mips32\n\t"
			"sync\n\t"
			"sync\n\t"
			"lw $zero,0(%0)\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"wait\n\t"
			"nop\n\t"
			"nop\n\t"
			".set mips32" : : "r"(0xa0000000));	
	TCSM_PCHAR('2');
	*(volatile unsigned *) 0xb00000d0 = 0x3;
	i = *(volatile unsigned *) 0xb00000d0;
	TCSM_DELAY(DELAY);
	TCSM_PCHAR('3');
	*(volatile unsigned *)  0xB3010008 &= ~(0x1<<17);
	TCSM_DELAY(DELAY);
	TCSM_PCHAR('4');
	*(volatile unsigned *) 0xb00000d0 = 0x1;
	i = *(volatile unsigned *) 0xb00000d0;
	TCSM_DELAY(DELAY);
	*((volatile unsigned int*)(0xb30100b8)) |= (0x1);
	__jz_cache_init();
}

void suspend_to_idle(void) {
        /* cpu enter idle mode when sleep */
        cpm_outl(cpm_inl(CPM_LCR), CPM_LCR);

    	TCSM_PCHAR('i');
        TCSM_PCHAR('d');
        TCSM_PCHAR('l');
	TCSM_PCHAR('e');

	__asm__ volatile(".set mips32\n\t"
			"sync\n\t"
			"sync\n\t"
			"lw $zero,0(%0)\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"wait\n\t"
			"nop\n\t"
			"nop\n\t"
			".set mips32" : : "r"(0xa0000000));
        TCSM_PCHAR('x');
        TCSM_PCHAR('x');
}

#define ENABLE_LCR_MODULES(m) 					\
	do{							\
		unsigned long tmp = cpm_inl(CPM_LCR);		\
		if(tmp &  (1 << ((m) + 28))) {			\
			cpm_outl(tmp & ~(1<<(28+(m))),CPM_LCR);	\
			while(cpm_inl(CPM_LCR) & (1<<(24+ (m))));	\
			udelay(500);					\
		}							\
	}while(0)

#define DISABLE_LCR_MODULES(m)						\
	do{								\
		register unsigned long tmp = cpm_inl(CPM_LCR);		\
		if(!(tmp & (1 << ((m) + 28)))) {			\
			cpm_outl(tmp | (1<<(28+(m))),CPM_LCR);		\
			while(!(cpm_inl(CPM_LCR) & (1<<(24+ (m))))) ;	\
			printk("iiiiiiiiii:%d\n",m);			\
		}							\
	}while(0)

#ifndef CONFIG_CPU_SUSPEND_TO_IDLE
#define SAVE_SIZE   1024
static unsigned int save_tcsm[SAVE_SIZE / 4];
#endif

static int jz4780_pm_enter(suspend_state_t state)
{
#ifndef CONFIG_FPGA_TEST
#ifndef CONFIG_CPU_SUSPEND_TO_IDLE
        unsigned long opcr = cpm_inl(CPM_OPCR);
#endif
	DISABLE_LCR_MODULES(0);
	DISABLE_LCR_MODULES(1);
	DISABLE_LCR_MODULES(2);
	DISABLE_LCR_MODULES(3);
#ifndef CONFIG_CPU_SUSPEND_TO_IDLE
	cpm_outl(cpm_inl(CPM_LCR) | LCR_LPM_SLEEP,CPM_LCR);

	__reset_dll = (void (*)(void))0xb3425800;
	memcpy(save_tcsm,__reset_dll,SAVE_SIZE);
	memcpy(__reset_dll, reset_dll,SAVE_SIZE);
        /* set external clock oscilltor stabilize time */
        /* l2cache enter power down mode when cpu in sleep mode */
	opcr |= 0xff << 8 | (2 << 26);
	/* select 32K crystal as RTC clock in sleep mode */
	opcr |= 1 << 2;
        /* disable enternal clock Oscillator in sleep mode */
	opcr &= ~(1 << 4);
        /* disable P0 power down */
	opcr &= ~(1 << 3);
	cpm_outl(opcr,CPM_OPCR);
	/* Clear previous reset status */
	cpm_outl(0,CPM_RSR);
        __reset_dll();
        cpm_outl(cpm_inl(CPM_LCR) & ~(LCR_LPM_MASK),CPM_LCR);
	memcpy(__reset_dll,save_tcsm,SAVE_SIZE);
#else
        suspend_to_idle();
#endif
#endif
	return 0;
}

/*
 * Initialize power interface
 */

struct platform_suspend_ops pm_ops = {
	.valid = suspend_valid_only_mem,
	.enter = jz4780_pm_enter,
};

int __init jz4780_pm_init(void)
{
	/*
	 * DON'T power down DLL
	 */
	*(volatile unsigned int *)0xb301102c &= ~(1 << 4);

	suspend_set_ops(&pm_ops);
	return 0;
}

arch_initcall(jz4780_pm_init);

