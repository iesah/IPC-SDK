/*
 * linux/arch/mips/jz4775/pm.c
 *
 *  JZ4775 Power Management Routines
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
#include <asm/fpu.h>
#include <linux/syscore_ops.h>
#include <linux/regulator/consumer.h>

#include <soc/cache.h>
#include <soc/base.h>
#include <soc/cpm.h>

#include <rjzcache.h>
#include <tcsm.h>

//#define TEST 1
//#define DUMP_DDR_REGS

#define save_regs_ra(base)			\
	__asm__ __volatile__ (			\
		".set push \n\t"		\
		".set noreorder \n\t"		\
		"sw	$31,116(%0)	\n\t"	\
		".set pop \n\t"			\
		:				\
		: "r" (base)			\
		: "memory"			\
		)

#define save_regs(base)							\
	__asm__ __volatile__ (						\
		".set push \n\t"					\
		".set    noat		\n\t"				\
		".set   noreorder       \n\t"				\
		"addu 	$26, %0,$0	\n\t"				\
		"mfhi	$27		\n\t"				\
		"sw	$0,0($26)	\n\t"				\
		"sw	$1,4($26)	\n\t"				\
		"sw	$27,120($26)	\n\t"				\
		"mflo	$27		\n\t"				\
		"sw	$2,8($26)	\n\t"				\
		"sw	$3,12($26)	\n\t"				\
		"sw	$27,124($26)	\n\t"				\
		"sw	$4,16($26)	\n\t"				\
		"sw	$5,20($26)	\n\t"				\
		"sw	$6,24($26)	\n\t"				\
		"sw	$7,28($26)	\n\t"				\
		"sw	$8,32($26)	\n\t"				\
		"sw	$9,36($26)	\n\t"				\
		"sw	$10,40($26)	\n\t"				\
		"sw	$11,44($26)	\n\t"				\
		"sw	$12,48($26)	\n\t"				\
		"sw	$13,52($26)	\n\t"				\
		"sw	$14,56($26)	\n\t"				\
		"sw	$15,60($26)	\n\t"				\
		"sw	$16,64($26)	\n\t"				\
		"sw	$17,68($26)	\n\t"				\
		"sw	$18,72($26)	\n\t"				\
		"sw	$19,76($26)	\n\t"				\
		"sw	$20,80($26)	\n\t"				\
		"sw	$21,84($26)	\n\t"				\
		"sw	$22,88($26)	\n\t"				\
		"sw	$23,92($26)	\n\t"				\
		"sw	$24,96($26)	\n\t"				\
		"sw	$25,100($26)	\n\t"				\
		"sw	$28,104($26)	\n\t"				\
		"sw	$29,108($26)	\n\t"				\
		"sw	$30,112($26)	\n\t"				\
		"sw	$31,116($26)	\n\t"				\
		"mfc0	$1, $0    	\n\t"				\
		"mfc0	$2, $1    	\n\t"				\
		"mfc0	$3, $2    	\n\t"				\
		"mfc0	$4, $3    	\n\t"				\
		"mfc0	$5, $4    	\n\t"				\
		"mfc0	$6, $5    	\n\t"				\
		"mfc0	$7, $6    	\n\t"				\
		"mfc0	$8, $8    	\n\t"				\
		"mfc0	$9, $10   	\n\t"				\
		"mfc0	$10,$12   	\n\t"				\
		"mfc0	$11, $12,1	\n\t"				\
		"mfc0	$12, $13 	\n\t"				\
		"mfc0	$13, $14    	\n\t"				\
		"mfc0	$14, $15    	\n\t"				\
		"mfc0	$15, $15,1    	\n\t"				\
		"mfc0	$16, $16    	\n\t"				\
		"mfc0	$17, $16,1    	\n\t"				\
		"mfc0	$18, $16,2    	\n\t"				\
		"mfc0	$19, $16,3    	\n\t"				\
		"mfc0	$20, $16, 7    	\n\t"				\
		"mfc0	$21, $17    	\n\t"				\
		"sw	$1,  128($26)    \n\t"				\
		"sw	$2,  132($26)    \n\t"				\
		"sw	$3,  136($26)    \n\t"				\
		"sw	$4,  140($26)    \n\t"				\
		"sw	$5,  144($26)    \n\t"				\
		"sw	$6,  148($26)    \n\t"				\
		"sw	$7,  152($26)    \n\t"				\
		"sw	$8,  156($26)    \n\t"				\
		"sw	$9,  160($26)    \n\t"				\
		"sw	$10, 164($26)    \n\t"				\
		"sw	$11, 168($26)    \n\t"				\
		"sw	$12, 172($26)    \n\t"				\
		"sw	$13, 176($26)    \n\t"				\
		"sw	$14, 180($26)    \n\t"				\
		"sw	$15, 184($26)    \n\t"				\
		"sw	$16, 188($26)    \n\t"				\
		"sw	$17, 192($26)    \n\t"				\
		"sw	$18, 196($26)    \n\t"				\
		"sw	$19, 200($26)    \n\t"				\
		"sw	$20, 204($26)    \n\t"				\
		"sw	$21, 208($26)    \n\t"				\
		"mfc0	$1, $18    	\n\t"				\
		"mfc0	$2, $19    	\n\t"				\
		"mfc0	$3, $23    	\n\t"				\
		"mfc0	$4, $24    	\n\t"				\
		"mfc0	$5, $26    	\n\t"				\
		"mfc0	$6, $28		\n\t"				\
		"mfc0	$7, $28,1	\n\t"				\
		"mfc0	$8, $30		\n\t"				\
		"mfc0	$9, $31		\n\t"				\
		"mfc0	$10,$5,4 	\n\t"				\
		"sw	$1,  212($26)	\n\t"				\
		"sw	$2,  216($26)	\n\t"				\
		"sw	$3,  220($26)	\n\t"				\
		"sw	$4,  224($26)	\n\t"				\
		"sw	$5,  228($26)	\n\t"				\
		"sw	$6,  232($26)	\n\t"				\
		"sw	$7,  236($26)	\n\t"				\
		"sw	$8,  240($26)	\n\t"				\
		"sw	$9,  244($26)	\n\t"				\
		"sw	$10, 248($26)	\n\t"				\
		".set pop \n\t"						\
		:							\
		: "r" (base)						\
		: "memory",						\
		  "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", \
		  "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19", "$20", \
		  "$21", "$22", "$23", "$24", "$25", "$26", "$27",	\
		  "$31"							\
		)

#define load_regs_jmp(base)				\
	__asm__ __volatile__ (			\
		".set push \n\t"		\
		".set    noat		\n\t"	\
		".set   noreorder \n\t"		\
		"addu 	$26, %0,$0	\n\t"	\
		"lw	$1,  128($26)	\n\t"	\
		"lw	$2,  132($26)	\n\t"	\
		"lw	$3,  136($26)	\n\t"	\
		"lw	$4,  140($26)	\n\t"	\
		"lw	$5,  144($26)	\n\t"	\
		"lw	$6,  148($26)	\n\t"	\
		"lw	$7,  152($26)	\n\t"	\
		"lw	$8,  156($26)	\n\t"	\
		"lw	$9,  160($26)	\n\t"	\
		"lw	$10, 164($26)	\n\t"	\
		"lw	$11, 168($26)	\n\t"	\
		"lw	$12, 172($26)	\n\t"	\
		"lw	$13, 176($26)	\n\t"	\
		"lw	$14, 180($26)	\n\t"	\
		"lw	$15, 184($26)	\n\t"	\
		"lw	$16, 188($26)	\n\t"	\
		"lw	$17, 192($26)	\n\t"	\
		"lw	$18, 196($26)	\n\t"	\
		"lw	$19, 200($26)	\n\t"	\
		"lw	$20, 204($26)	\n\t"	\
		"lw	$21, 208($26)	\n\t"	\
		"mtc0	$1, $0		\n\t"	\
		"mtc0	$2, $1		\n\t"	\
		"mtc0	$3, $2		\n\t"	\
		"mtc0	$4, $3		\n\t"	\
		"mtc0	$5, $4		\n\t"	\
		"mtc0	$6, $5		\n\t"	\
		"mtc0	$7, $6		\n\t"	\
		"mtc0	$8, $8		\n\t"	\
		"mtc0	$9, $10		\n\t"	\
		"mtc0	$10,$12		\n\t"	\
		"mtc0	$11, $12,1	\n\t"	\
		"mtc0	$12, $13	\n\t"	\
		"mtc0	$13, $14	\n\t"	\
		"mtc0	$14, $15	\n\t"	\
		"mtc0	$15, $15,1	\n\t"	\
		"mtc0	$16, $16	\n\t"	\
		"mtc0	$17, $16,1	\n\t"	\
		"mtc0	$18, $16,2	\n\t"	\
		"mtc0	$19, $16,3	\n\t"	\
		"mtc0	$20, $16,7	\n\t"	\
		"mtc0	$21, $17	\n\t"	\
		"lw	$1,  212($26)	\n\t"	\
		"lw	$2,  216($26)	\n\t"	\
		"lw	$3,  220($26)	\n\t"	\
		"lw	$4,  224($26)	\n\t"	\
		"lw	$5,  228($26)	\n\t"	\
		"lw	$6,  232($26)	\n\t"	\
		"lw	$7,  236($26)	\n\t"	\
		"lw	$8,  240($26)	\n\t"	\
		"lw	$9,  244($26)	\n\t"	\
		"lw	$10, 248($26)	\n\t"	\
		"mtc0	$1, $18		\n\t"	\
		"mtc0	$2, $19		\n\t"	\
		"mtc0	$3, $23		\n\t"	\
		"mtc0	$4, $24		\n\t"	\
		"mtc0	$5, $26		\n\t"	\
		"mtc0	$6, $28		\n\t"	\
		"mtc0	$7, $28,1	\n\t"	\
		"mtc0	$8, $30		\n\t"	\
		"mtc0	$9, $31		\n\t"	\
		"mtc0	$10,$5,4	\n\t"	\
		"lw	$27,	120($26)\n\t"	\
		"lw	$0,	0($26)	\n\t"	\
		"lw	$1,  	4($26)	\n\t"	\
		"mthi	$27		\n\t"	\
		"lw	$27,	124($26)\n\t"	\
		"lw	$2,	8($26)	\n\t"	\
		"lw	$3,  	12($26)	\n\t"	\
		"mtlo	$27		\n\t"	\
		"lw	$4,  	16($26)	\n\t"	\
		"lw	$5,  	20($26)	\n\t"	\
		"lw	$6,  	24($26)	\n\t"	\
		"lw	$7,  	28($26)	\n\t"	\
		"lw	$8,  	32($26)	\n\t"	\
		"lw	$9,  	36($26)	\n\t"	\
		"lw	$10, 	40($26)	\n\t"	\
		"lw	$11, 	44($26)	\n\t"	\
		"lw	$12, 	48($26)	\n\t"	\
		"lw	$13, 	52($26)	\n\t"	\
		"lw	$14, 	56($26)	\n\t"	\
		"lw	$15, 	60($26)	\n\t"	\
		"lw	$16, 	64($26)	\n\t"	\
		"lw	$17, 	68($26)	\n\t"	\
		"lw	$18, 	72($26)	\n\t"	\
		"lw	$19, 	76($26)	\n\t"	\
		"lw	$20, 	80($26)	\n\t"	\
		"lw	$21, 	84($26)	\n\t"	\
		"lw	$22, 	88($26)	\n\t"	\
		"lw	$23, 	92($26)	\n\t"	\
		"lw	$24, 	96($26)	\n\t"	\
		"lw	$25, 	100($26)\n\t"	\
		"lw	$28, 	104($26)\n\t"	\
		"lw	$29, 	108($26)\n\t"	\
		"lw	$30, 	112($26)\n\t"	\
		"lw	$31, 	116($26)\n\t"	\
		"j      $31             \n\t"	\
		"nop             \n\t"					\
		".set pop \n\t"						\
		:							\
		: "r" (base)						\
		: "memory",						\
		  "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", \
		  "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19", "$20", \
		  "$21", "$22", "$23", "$24", "$25", "$26", "$27",	\
		  "$31"							\
		)

#define TCSM_BASE 	(0xb3422000)
#define RETURN_ADDR 	(TCSM_BASE+0)
#define REG_ADDR 	(TCSM_BASE+4)
#define RESUME_ADDR 	(TCSM_BASE+8)

#define DELAY_0 0x1ff
#define DELAY_1 0x1ff
#define DELAY_2 0x1ff

#define SAVE_SIZE 4096

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


#ifdef DUMP_DDR_REGS

void inline tcsm_put_hex(u32 d)
{
	register u32 i;
	register u32 c;

	TCSM_PCHAR('*');
	for(i = 0; i < 8;i++) {
		c = (d >> ((7 - i) * 4)) & 0xf;
		if(c < 10)
			c += 0x30;
		else
			c += (0x41 - 10);

		TCSM_PCHAR(c);
	}
	TCSM_PCHAR('\n');
	TCSM_PCHAR('\r');
}

static inline void dump_regs(void) {
/*
 *	16'hb301_100c
 *	16'hb301_1034
 *	16'hb301_1038
 *	16'hb301_103c
 *	and from 16'hb301_0060 to 16'hb301_0074
 */
	volatile u32 *p = (volatile u32 *)0xb3011000;
	volatile u32 i = 1;

	TCSM_PCHAR('\n');
	TCSM_PCHAR('\r');
	tcsm_put_hex(p[0xc / 4]);
	i = 1;
	while (i--);

	tcsm_put_hex(p[0x34 / 4]);

	i = 1;
	while (i--);

	tcsm_put_hex(p[0x38 / 4]);

	i = 1;
	while (i--);

	tcsm_put_hex(p[0x3c / 4]);

	i = 1;
	while (i--);

	for (p = (volatile u32 *)0xb3010060; p < (volatile u32 *)0xb3010078; p++)
		tcsm_put_hex(*p);
}
#endif


#define TCSM_DELAY(x) \
	i=x;	\
	while(i--)	\
	__asm__ volatile(".set mips32\n\t"\
			"nop\n\t"\
			".set mips32")

#ifdef CONFIG_SUSPEND_SUPREME_DEBUG
/* store something we can use it for memory testing */
static __section(.mem.test) char test_mem0 = 't';
static __section(.mem.test) char test_mem1 = 'e';
static __section(.mem.test) char test_mem2 = 's';
static __section(.mem.test) char test_mem3 = 't';
#endif

static unsigned int regs[256] __attribute__ ((aligned (32)));
static char tcsm_back[SAVE_SIZE] __attribute__ ((aligned (32)));

#define UNIQUE_ENTRYHI(idx) (CKSEG0 + ((idx) << (PAGE_SHIFT + 1)))
static inline void local_flush_tlb_all(void)
{
	unsigned long old_ctx;
	int entry;

	/* Save old context and create impossible VPN2 value */
	old_ctx = read_c0_entryhi();
	write_c0_entrylo0(0);
	write_c0_entrylo1(0);

	entry = read_c0_wired();

	/* Blast 'em all away. */
	while (entry < current_cpu_data.tlbsize) {
		/* Make sure all entries differ. */
		write_c0_entryhi(UNIQUE_ENTRYHI(entry));
		write_c0_index(entry);
		mtc0_tlbw_hazard();
		tlb_write_indexed();
		entry++;
	}
	tlbw_use_hazard();
	write_c0_entryhi(old_ctx);
}

static noinline void reset_dll(void)
{
	void (*return_func)(void);
#ifndef TEST
	register int i;

	TCSM_PCHAR('0');
	while(!((*(volatile unsigned *)0xb0000010) & (0x1<<10)))//wait pll stable
		;
	TCSM_DELAY(DELAY_0);

	TCSM_PCHAR('1');
	*(volatile unsigned *) 0xb00000d0 = 0x3; //reset the DLL in DDR PHY
	i = *(volatile unsigned *) 0xb00000d0;
	TCSM_DELAY(DELAY_1);
	TCSM_PCHAR('3');
	*(volatile unsigned *)  0xB3010008 &= ~(0x1<<17); //KEEPSR keep to 0
	i=*(volatile unsigned *)0xB3010008;
	TCSM_PCHAR('4');

	*(volatile unsigned *) 0xb00000d0 = 0x1; //disable the reset
	i = *(volatile unsigned *) 0xb00000d0;
	TCSM_DELAY(DELAY_2);
	TCSM_PCHAR('5');
	__jz_cache_init();
	TCSM_PCHAR('6');
#endif

	*((volatile unsigned int*)(0xb30100b8)) |= 0x1;//DDR DFI low power interface enable

#ifdef DUMP_DDR_REGS
	dump_regs();
#endif

	TCSM_PCHAR(test_mem0);
	TCSM_PCHAR(test_mem1);
	TCSM_PCHAR(test_mem2);
	TCSM_PCHAR(test_mem3);

	return_func = (void (*)(void))(*(volatile unsigned int *)RETURN_ADDR);
	return_func();
}

static noinline void jz4775_resume(void)
{
	/*
	 * NOTE: you can do some simple things here
	 * for example: access memory, light a LED, etc.
	 *
	 * BUT, do not call kernel function which might_sleep(),
	 * for example: printk()s, etc.
	 *
	 * as a principle: DO NOTE call any function here
	 */

	/* for example: set PE7 as output 1 */
#if 0
	*(volatile unsigned int *)0xb0010418 = (1 << 7);
	*(volatile unsigned int *)0xb0010424 = (1 << 7);
	*(volatile unsigned int *)0xb0010438 = (1 << 7);
	*(volatile unsigned int *)0xb0010444 = (1 << 7);
#endif

	load_regs_jmp(*(volatile unsigned int *)REG_ADDR);
}

static noinline void jz4775_suspend(void)
{
	/*
	 *  WARNING: should not call any function in here
	 */
	save_regs_ra(*(volatile unsigned int *)REG_ADDR);
	mb();

#ifdef TEST
	reset_dll();
#else
	blast_dcache32();
	blast_icache32();
	cache_prefetch(sleep,sleep_2);

	*((volatile unsigned int*)(0xb30100b8)) &= ~(0x1);
sleep:
	__asm__ volatile(".set mips32\n\t"
		"sync\n\t"
		"sync\n\t"
		"wait\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		".set mips32");
	*((volatile unsigned int*)(0xb30100b8)) |= 0x1;
	jz4775_resume();
	/* BE CARE!!! any code below this are not executed!!! */
sleep_2:
#endif
	return;
}

#ifdef CONFIG_SLCD_SUSPEND_ALARM_WAKEUP_REFRESH
int jz4775_pm_enter(suspend_state_t state)
#else
static int jz4775_pm_enter(suspend_state_t state)
#endif
{
	unsigned int lcr = cpm_inl(CPM_LCR);
	unsigned int opcr = cpm_inl(CPM_OPCR);

	disable_fpu();
#ifdef	CONFIG_TRAPS_USE_TCSM
	cpu0_save_tscm();
#endif
	cpm_outl(LCR_LPM_SLEEP | 0xc000ff00,CPM_LCR);
	while((cpm_inl(CPM_LCR) & 0xcc000000) != 0xcc000000);
	mdelay(1);

	cpm_outl(cpm_inl(CPM_USBPCR) | (1<<25),CPM_USBPCR);

	/* Set resume return address */
	cpm_outl(1,CPM_SLBC);
	memcpy(tcsm_back, (void *)TCSM_BASE, SAVE_SIZE);
	*(volatile unsigned int *)RETURN_ADDR = (unsigned int)jz4775_resume;
	*(volatile unsigned int *)REG_ADDR = (unsigned int)regs;
	cpm_outl(RESUME_ADDR,CPM_SLPC);
	memcpy((void *)RESUME_ADDR, reset_dll, SAVE_SIZE - 8);
	mdelay(1);
	/* set Oscillator Stabilize Time*/
	/* disable externel clock Oscillator in sleep mode */
	/* select 32K crystal as RTC clock in sleep mode */
	cpm_outl(((opcr & 0x22) | 1<<25 | 0xff<<8 | OPCR_PD | OPCR_ERCS) & (~(1<<7)) ,CPM_OPCR);
	/* Clear previous reset status */
	cpm_outl(0,CPM_RSR);

	*(volatile unsigned *)  0xB3010008 |= 0x1<<17;

#ifdef CONFIG_SUSPEND_SUPREME_DEBUG
	printk("enter suspend.\n");
#endif

	mb();
	save_regs(*(volatile unsigned int *)REG_ADDR);
	/*
	 * CheckPoint A
	 *
	 * save_regs() saved the context of jz4780_pm_enter before sleep;
	 */
	//printk("===========>haha\n");
	jz4775_suspend();
	/*
	 * CheckPoint B
	 *
	 * when resume, PC goes to here, as if the code between A and B is never exist!
	 */
	mb();
	local_flush_tlb_all();

#ifdef CONFIG_SUSPEND_SUPREME_DEBUG
	TCSM_PCHAR('x');
	TCSM_PCHAR('x');
	printk("resume.\n");
#endif

	memcpy((void *)TCSM_BASE, tcsm_back, SAVE_SIZE);
	cpm_outl(lcr,CPM_LCR);
	cpm_outl(opcr,CPM_OPCR);

#ifdef	CONFIG_TRAPS_USE_TCSM
	cpu0_restore_tscm();
#endif

	return 0;
}

/*
 * Initialize power interface
 */
#ifdef CONFIG_SLCD_SUSPEND_ALARM_WAKEUP_REFRESH
/* drivers/video/jz4780-fb/lcd_suspend_update/suspend_ops.c */
extern struct platform_suspend_ops pm_ops;

#else  /* CONFIG_SLCD_SUSPEND_ALARM_WAKEUP_REFRESH */
struct platform_suspend_ops pm_ops = {
	.valid = suspend_valid_only_mem,
	.enter = jz4775_pm_enter,
};
#endif	/* CONFIG_SLCD_SUSPEND_ALARM_WAKEUP_REFRESH */

int __init jz4775_pm_init(void)
{
	suspend_set_ops(&pm_ops);
	return 0;
}

arch_initcall(jz4775_pm_init);
