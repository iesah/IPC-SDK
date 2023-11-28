/*
 * linux/arch/mips/xburst/soc-m200/common/pm_p0.c
 *
 *  M200 Power Management Routines
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
#include <asm/fpu.h>
#include <linux/syscore_ops.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/notifier.h>
#ifdef CONFIG_CPU_FREQ
#include <linux/cpufreq.h>
#endif
#include <asm/cacheops.h>
#include <soc/cache.h>
#include <asm/r4kcache.h>
#include <soc/base.h>
#include <soc/cpm.h>
#include <soc/gpio.h>
#include <soc/ddr.h>
#include <tcsm.h>
#include <smp_cp0.h>


#ifdef CONFIG_JZ_DMIC_WAKEUP
#include <linux/voice_wakeup_module.h>
#endif

#ifdef CONFIG_TEST_SECOND_REFRESH
#include <linux/second_refresh.h>
#endif

extern long long save_goto(unsigned int);
extern int restore_goto(void);
extern unsigned int get_pmu_slp_gpio_info(void);


#define get_cp0_ebase()	__read_32bit_c0_register($15, 1)

#define OFF_TDR         (0x00)
#define OFF_LCR         (0x0C)
#define OFF_LSR         (0x14)

#define LSR_TDRQ        (1 << 5)
#define LSR_TEMT        (1 << 6)

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#define U_IOBASE (UART1_IOBASE + 0xa0000000)
#define TCSM_PCHAR(x)							\
	*((volatile unsigned int*)(U_IOBASE+OFF_TDR)) = x;		\
	while ((*((volatile unsigned int*)(U_IOBASE + OFF_LSR)) & (LSR_TDRQ | LSR_TEMT)) != (LSR_TDRQ | LSR_TEMT))
#else
#define TCSM_PCHAR(x)
#endif

#define TCSM_DELAY(x)					\
	do{							\
		register unsigned int i = x;			\
	while(i--)					\
		__asm__ volatile(".set mips32\n\t"	\
				 "nop\n\t"		\
					 ".set mips32");	\
	}while(0)


static inline void serial_put_hex(unsigned int x) {
	int i;
	unsigned int d;
	for(i = 7;i >= 0;i--) {
		d = (x  >> (i * 4)) & 0xf;
		if(d < 10) d += '0';
		else d += 'A' - 10;
		TCSM_PCHAR(d);
	}
}

static void load_func_to_tcsm(unsigned int *tcsm_addr,unsigned int *f_addr,unsigned int size);
/* #define DDR_MEM_TEST */
#ifdef DDR_MEM_TEST
#define MEM_TEST_SIZE   0x100000
static unsigned int test_mem_space[MEM_TEST_SIZE / 4];

static inline void test_ddr_data_init(void) {
	int i;
	unsigned int *test_mem;
	test_mem = (unsigned int *)((unsigned int)test_mem_space | 0xa0000000);
	dma_cache_wback_inv((unsigned int)test_mem_space,0x100000);
	for(i = 0;i < MEM_TEST_SIZE / 4;i++) {
		test_mem[i] = (unsigned int)&test_mem[i];
	}
}
static inline void check_ddr_data(void) {
	int i;
	unsigned int *test_mem;
	test_mem = (unsigned int *)((unsigned int)test_mem_space | 0xa0000000);
	for(i = 0;i < MEM_TEST_SIZE / 4;i++) {
		unsigned int dd;
		dd = test_mem[i];
		if(dd != (unsigned int)&test_mem[i]) {
			serial_put_hex(dd);
			TCSM_PCHAR(' ');
			serial_put_hex(i);
			TCSM_PCHAR(' ');
			serial_put_hex((unsigned int)&test_mem[i]);
			TCSM_PCHAR('\r');
			TCSM_PCHAR('\n');
		}
	}
}
#endif
static inline void dump_ddr_param(void) {
	int i;
	for(i = 0;i < 4;i++) {
		TCSM_PCHAR('<');
		serial_put_hex(i);
		TCSM_PCHAR('>');
		TCSM_PCHAR('<');
		serial_put_hex(ddr_readl(DDRP_DXnDQSTR(i)));
		TCSM_PCHAR('>');
		serial_put_hex(ddr_readl(DDRP_DXnDQTR(i)));
		TCSM_PCHAR('\r');
		TCSM_PCHAR('\n');
	}
	TCSM_PCHAR(':');
	serial_put_hex(ddr_readl(DDRP_PGSR));
	TCSM_PCHAR('\r');
	TCSM_PCHAR('\n');

}
extern void dump_clk(void);
struct save_reg
{
	unsigned int addr;
	unsigned int value;
};
#define SUSPEND_SAVE_REG_SIZE 10
static struct save_reg m_save_reg[SUSPEND_SAVE_REG_SIZE];
static int m_save_reg_count = 0;
static unsigned int read_save_reg_add(unsigned int addr)
{
	unsigned int value = REG32(CKSEG1ADDR(addr));
	if(m_save_reg_count < SUSPEND_SAVE_REG_SIZE) {
		m_save_reg[m_save_reg_count].addr = addr;
		m_save_reg[m_save_reg_count].value = value;
		m_save_reg_count++;
	}else
		printk("suspend_reg buffer size too small!\n");
	return value;
}
static void restore_all_reg(void)
{
	int i;
	for(i = 0;i < m_save_reg_count;i++) {
		REG32(CKSEG1ADDR(m_save_reg[i].addr)) = m_save_reg[i].value;
	}
	m_save_reg_count = 0;
}
static inline void config_powerdown_core(unsigned int *resume_pc) {
	unsigned int cpu_no,opcr;
	unsigned int reim,ctrl;
	unsigned int addr;
	/* set SLBC and SLPC */
	cpm_outl(1,CPM_SLBC);
	/* Clear previous reset status */
	cpm_outl(0,CPM_RSR);
	/* OPCR.PD and OPCR.L2C_PD */
	cpu_no = get_cp0_ebase() & 1;
	opcr = cpm_inl(CPM_OPCR);
	//p 0 or 1 powerdown
	opcr &= ~(3<<25);
	opcr |= (cpu_no + 1) << 25; /* both big and small core power down*/
	cpm_outl(opcr,CPM_OPCR);

	ctrl = get_smp_ctrl();
	ctrl |= 1 << (cpu_no + 8);
	set_smp_ctrl(ctrl);

	printk("ctrl = 0x%08x\n",get_smp_ctrl());


	reim = get_smp_reim();
	reim &= ~(0xffff << 16);
	reim |= (unsigned int)resume_pc & (0xffff << 16);
	set_smp_reim(reim);
	printk("reim = 0x%08x\n",get_smp_reim());
	addr = __read_32bit_c0_register($12, 7);
	printk("addr = 0x%08x\n",addr);
	__write_32bit_c0_register($12, 7, (unsigned int)resume_pc & (0xffff));

	addr = __read_32bit_c0_register($12, 7);
	printk("addr = 0x%08x\n",addr);

	printk("opcr = %x\n",cpm_inl(CPM_OPCR));
	printk("lcr = %x\n",cpm_inl(CPM_LCR));
	printk("cpccr = %x\n",cpm_inl(CPM_CPCCR));

	// set resume pc
	cpm_outl((unsigned int)resume_pc,CPM_SLPC);



}
static inline void set_gpio_func(int gpio, int type) {
	int i;
	int port = gpio / 32;
	int pin = gpio & 0x1f;
	int addr = 0xb0010010 + port * 0x100;

	for(i = 0;i < 4;i++){
		REG32(addr + 0x10 * i) &= ~(1 << pin);
		REG32(addr + 0x10 * i) |= (((type >> (3 - i)) & 1) << pin);
	}
}

/* int get_gpio_func(int gpio) { */
/* 	int i; */
/* 	int ret = 0; */
/* 	int port = gpio / 32; */
/* 	int pin = BIT(gpio & 0x1f); */
/* 	int addr = 0xb0010010 + port * 0x100; */
/* 	for(i = 0;i < 4;i++){ */
/* 		ret |= ((REG32(addr + 0x10 * i) >> pin) & 1)  << (3 - i); */
/* 	} */
/* 	return ret; */
/* } */


/**
 *      |-------------|     <--- SLEEP_TCSM_BOOTCODE_TEXT
 *      | BOOT CODE   |
 *      |-------------|     <--- SLEEP_TCSM_RESUMECODE_TEXT
 *      |    ...      |
 *      | RESUME CODE |
 *      |    ...      |
 *      |-------------|     <--- SLEEP_TCSM_RESUME_DATA
 *      | RESUME DATA |
 *      |_____________|
 */

#define SLEEP_TCSM_SPACE           0xb3423000
#define SLEEP_TCSM_LEN             4096

#define SLEEP_TCSM_BOOT_LEN        256
#define SLEEP_TCSM_DATA_LEN        64
#define SLEEP_TCSM_RESUME_LEN      (SLEEP_TCSM_LEN - SLEEP_TCSM_BOOT_LEN - SLEEP_TCSM_DATA_LEN)

#define SLEEP_TCSM_BOOT_TEXT       (SLEEP_TCSM_SPACE)
#define SLEEP_TCSM_RESUME_TEXT     (SLEEP_TCSM_BOOT_TEXT + SLEEP_TCSM_BOOT_LEN)
#define SLEEP_TCSM_RESUME_DATA     (SLEEP_TCSM_RESUME_TEXT + SLEEP_TCSM_RESUME_LEN)

#define CPU_RESMUE_SP				0xb3425FFC	/* BANK3~BANK2 */

static int __attribute__((aligned(256))) test_l2cache_handle(int val)
{
	val = val % 3;
	serial_put_hex(val);
	TCSM_PCHAR('\r');
	TCSM_PCHAR('\n');
	TCSM_PCHAR('l');
	TCSM_PCHAR('\r');
	TCSM_PCHAR('\n');
	return val;
}

#ifdef CONFIG_TEST_SECOND_REFRESH

#define SECOND_REFRESH_MAP_ADDR_TO			(0x20000000)
#define SECOND_REFRESH_MAP_ADDR_FR_0		(0x2f900000)	/* 249MByte: */
#define SECOND_REFRESH_MAP_ADDR_FR_1		(0x2fa00000)	/* 249MByte: */
#define MAP_PAGE_MASK						(0x001fe000)  /*1MBytes*/

static inline void _setup_tlb(void)
{

	unsigned int pagemask = MAP_PAGE_MASK;    /* 1MB */
	/*                              cached  D:allow-W   V:valid    G */
	unsigned int entrylo0 = (SECOND_REFRESH_MAP_ADDR_FR_0 >> 6) | ((6 << 3) + (1 << 2) + (1 << 1) + 1); /*Data Entry*/
	unsigned int entrylo1 = (SECOND_REFRESH_MAP_ADDR_FR_1 >> 6) | ((6 << 3) + (1 << 2) + (1 << 1) + 1);
	unsigned int entryhi =  SECOND_REFRESH_MAP_ADDR_TO; /* Tag Entry */
	volatile int i;
	volatile unsigned int temp;

	temp = __read_32bit_c0_register($12, 0);
	temp &= ~(1<<2);
	__write_32bit_c0_register($12, 0, temp);

	write_c0_pagemask(pagemask);
	write_c0_wired(0);

	/* 6M Byte*/
	for(i = 0; i < 6; i++)
	{
		asm (
				"mtc0 %0, $0\n\t"    /* write Index */

				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"mtc0 %1, $5\n\t"    /* write PageMask */
				"mtc0 %2, $10\n\t"    /* write EntryHi */
				"mtc0 %3, $2\n\t"    /* write EntryLo0 */
				"mtc0 %4, $3\n\t"    /* write EntryLo1 */
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"ssnop\n\t"
				"tlbwi    \n\t"        /* TLB indexed write */
				: : "Jr" (i), "r" (pagemask), "r" (entryhi),
				"r" (entrylo0), "r" (entrylo1)
			);

		entryhi +=  0x00200000;    /* 1MB */
		entrylo0 += (0x00100000 >> 6);
		entrylo1 += (0x00100000 >> 6);
	}
}
#endif

static noinline void cpu_sleep(void)
{
	register unsigned int val;
	register unsigned int pmu_slp_gpio_info = -1;
	unsigned int save_slp = -1, func;

	pmu_slp_gpio_info = get_pmu_slp_gpio_info();
	if(pmu_slp_gpio_info != -1) {
		save_slp = pmu_slp_gpio_info & 0xffff;
		func = pmu_slp_gpio_info >> 16;
		if(func == GPIO_OUTPUT1) {
			save_slp |= GPIO_OUTPUT0 << 16;
		} else if(func == GPIO_OUTPUT0) {
			save_slp |= GPIO_OUTPUT1 << 16;
		} else {
			printk("regulator sleep gpio set output type error!\n");
			return;
		}
		REG32(SLEEP_TCSM_RESUME_DATA + 8) = save_slp;
	} else {
		REG32(SLEEP_TCSM_RESUME_DATA + 8) = pmu_slp_gpio_info;
	}

	config_powerdown_core((unsigned int *)SLEEP_TCSM_BOOT_TEXT);
	/* printk("sleep!\n"); */
	/* printk("int mask:0x%08x\n",REG32(0xb0001004)); */
	/* printk("gate:0x%08x\n",cpm_inl(CPM_CLKGR)); */
	/* printk("CPM_DDRCDR:0x%08x\n",cpm_inl(CPM_DDRCDR)); */
	/* printk("DDRC_AUTOSR_EN: %x\n",ddr_readl(DDRC_AUTOSR_EN)); */
	/* printk("DDRC_DLP: %x\n",ddr_readl(DDRC_DLP)); */
	/* printk("ddr cs %x\n",ddr_readl(DDRP_DX0GSR)); */
	REG32(SLEEP_TCSM_RESUME_DATA + 12) = ddr_readl(DDRP_DX0GSR) & 3;
	REG32(SLEEP_TCSM_RESUME_DATA + 16) = read_c0_config();
	REG32(SLEEP_TCSM_RESUME_DATA + 20) = read_c0_status();
	REG32(SLEEP_TCSM_RESUME_DATA + 24) = REG32(0xb0000000);



#ifdef CONFIG_JZ_DMIC_WAKEUP
	wakeup_module_open(DEEP_SLEEP);
#endif

#ifdef CONFIG_TEST_SECOND_REFRESH
	printk("test _tlb ...3 \n");
	_setup_tlb();

	second_refresh_open(1);

	blast_dcache32();
	blast_icache32();
	blast_scache32();

	second_refresh_cache_prefetch();
#else

	blast_dcache32();
	blast_icache32();
	blast_scache32();

#endif



#ifdef CONFIG_JZ_DMIC_WAKEUP
	REG32(SLEEP_TCSM_RESUME_DATA + 28) = wakeup_module_is_wakeup_enabled();
	wakeup_module_cache_prefetch();
#endif
	if(0) {
		for(val = 0;val < 64*1024; val += 32)
		{
			func = *(volatile unsigned int *)((unsigned int)test_l2cache_handle + val);
		}
	}
	cache_prefetch(LABLE1,200);
	blast_dcache32();
	__sync();
	__fast_iob();
LABLE1:
	val = ddr_readl(DDRC_AUTOSR_EN);
	REG32(SLEEP_TCSM_RESUME_DATA + 0) = val;
	ddr_writel(0,DDRC_AUTOSR_EN);             // exit auto sel-refresh
	val = ddr_readl(DDRC_DLP);
	REG32(SLEEP_TCSM_RESUME_DATA + 4) = val;
	if(!(ddr_readl(DDRP_PIR) & DDRP_PIR_DLLBYP) && !val)
	{
		ddr_writel(0xf003 , DDRC_DLP);
		val = ddr_readl(DDRP_DSGCR);
		val |= (1 << 4);
		ddr_writel(val,DDRP_DSGCR);
	}
        /**
	 *  DDR keep selrefresh,when it exit the sleep state.
	 */
	val = ddr_readl(DDRC_CTRL);
	val |= (1 << 17);   // enter to hold ddr state
	ddr_writel(val,DDRC_CTRL);

	/*
	 * (1) SCL_SRC source clock changes APLL to EXCLK
	 * (2) AH0/2 source clock changes MPLL to EXCLK
	 * (3) set PDIV H2DIV H0DIV L2CDIV CDIV = 0
	 */
	REG32(0xb0000000) = 0x95800000;
	while((REG32(0xB00000D4) & 7))
		TCSM_PCHAR('w');

	if(pmu_slp_gpio_info != -1) {
		set_gpio_func(pmu_slp_gpio_info & 0xffff,
				pmu_slp_gpio_info >> 16);
	}

#ifdef CONFIG_JZ_DMIC_WAKEUP
	/* set cache writeback */
	REG32(SLEEP_TCSM_RESUME_DATA + 32) = __read_32bit_c0_register($12, 2); /* cache attr */
	__write_32bit_c0_register($12, 2, REG32(SLEEP_TCSM_RESUME_DATA + 32) | (1<<31));
#endif
	/* set pdma deep sleep */
	REG32(0xb00000b8) |= (1<<31);
	__asm__ volatile(".set mips32\n\t"
			"wait\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"jr %0\n\t"
			".set mips32 \n\t" ::"r" (SLEEP_TCSM_BOOT_TEXT));

	while(1)
		TCSM_PCHAR('n');

}
static inline void powerdown_wait(void)
{
	unsigned int opcr;
	unsigned int cpu_no;
	unsigned int lcr;
	__asm__ volatile("ssnop");

	lcr = REG32(0xb0000000 + CPM_LCR);
	lcr &= ~3;
	lcr |= 1;
	REG32(0xb0000000 + CPM_LCR) = lcr;


	cpu_no = get_cp0_ebase() & 1;
	opcr = REG32(0xb0000000 + CPM_OPCR);
	opcr &= ~(3<<25);
	opcr |= (cpu_no + 1) << 25; /* both big and small core power down*/
	opcr |= 1 << 30;
	REG32(0xb0000000 + CPM_OPCR) = opcr;
	//serial_put_hex(opcr);
	TCSM_PCHAR('e');
	__asm__ volatile(".set mips32\n\t"
			 "wait\n\t"
			 "nop\n\t"
			 "nop\n\t"
			 "nop\n\t"
			 ".set mips32 \n\t");
	TCSM_PCHAR('Q');
}
static inline void sleep_wait(void)
{
	unsigned int opcr;
	void (*volatile f)(void);
	opcr = REG32(0xb0000000 + CPM_OPCR);
	opcr &= ~(3<<25);
	opcr |= 1 << 30;
	REG32(0xb0000000 + CPM_OPCR) = opcr;
	//REG32(0xb3420000 + 0x10 + 5*0x20) &= ~(1<<0);
	//REG32(0xb3421000) &= ~(1<<0);
	__asm__ volatile(".set mips32\n\t"
			 "wait\n\t"
			 "nop\n\t"
			 "nop\n\t"
			 "nop\n\t"
			 //"jr %0\n\t"
			 ".set mips32 \n\t"
			);
	//::"r"(SLEEP_TCSM_BOOT_TEXT));
	//REG32(0xb3420000 + 0x10 + 5*0x20) |= (1<<0);
	//REG32(0xb3421000) |= (1<<0);
	TCSM_PCHAR('q');
	f = (void (*)(void))SLEEP_TCSM_BOOT_TEXT;
	f();
}

static noinline void cpu_resume_boot(void)
{

	__asm__ volatile(
		".set mips32\n\t"
		"move $29, %0\n\t"
		".set mips32\n\t"
		:
		:"r" (CPU_RESMUE_SP)
		:
		);
	__asm__ volatile(".set mips32\n\t"
			 "jr %0\n\t"
			 "nop\n\t"
			 ".set mips32 \n\t" :: "r" (SLEEP_TCSM_RESUME_TEXT));

}
static noinline void cpu_resume(void)
{
	int val = 0;
	int bypassmode = 0;
#ifdef CONFIG_JZ_DMIC_WAKEUP
	int (*volatile func)(int);
	int temp;
#endif
#ifdef CONFIG_TEST_SECOND_REFRESH
	int (*volatile s_func)(int);
	int s_temp;
#endif
	TCSM_PCHAR('O');

	write_c0_config(REG32(SLEEP_TCSM_RESUME_DATA + 16));  // restore cachable
	write_c0_status(REG32(SLEEP_TCSM_RESUME_DATA + 20));  // restore cp0 statue

#ifdef CONFIG_JZ_DMIC_WAKEUP
	if(REG32(SLEEP_TCSM_RESUME_DATA + 28) == 1) {
		/* wakeup module is enabled */
		temp = *(unsigned int *)WAKEUP_HANDLER_ADDR;
		func = (int (*)(int))temp;
		val = func(1);
	}
	//serial_put_hex(val);
#endif

	/* restore  CPM CPCCR */
	val = REG32(SLEEP_TCSM_RESUME_DATA + 24);
	val |= (7 << 20);
	REG32(0xb0000000) = val;
	while((REG32(0xB00000D4) & 7))
		TCSM_PCHAR('w');

	val = REG32(SLEEP_TCSM_RESUME_DATA + 8);
	if(val != -1)
		set_gpio_func(val & 0xffff, val >> 16);

	bypassmode = ddr_readl(DDRP_PIR) & DDRP_PIR_DLLBYP;
	if(!bypassmode) {
		/**
		 * reset dll of ddr.
		 * WARNING: 2015-01-08
		 * 	DDR CLK GATE(CPM_DRCG 0xB00000D0), BIT6 must set to 1 (or 0x40).
		 * 	If clear BIT6, chip memory will not stable, gpu hang occur.
		 */

#define CPM_DRCG			(0xB00000D0)

		*(volatile unsigned int *)CPM_DRCG = 0x53 | (1<<6);
		TCSM_DELAY(0x1ff);
		*(volatile unsigned int *)CPM_DRCG = 0x7d | (1<<6);
		TCSM_DELAY(0x1ff);
		/**
		 * for disabled ddr enter power down.
		 */
		*(volatile unsigned int *)0xb301102c &= ~(1 << 4);
		TCSM_PCHAR('d');
		TCSM_DELAY(0xf);
		/**
		 * reset dll of ddr too.
		 */
		*(volatile unsigned int *)CPM_DRCG = 0x53 | (1<<6);
		TCSM_DELAY(0x1ff);
		*(volatile unsigned int *)CPM_DRCG = 0x7d | (1<<6);
		TCSM_DELAY(0x1ff);
                /**
		 * dll reset item & dll locked.
		 */
		val = DDRP_PIR_INIT | DDRP_PIR_ITMSRST | DDRP_PIR_DLLLOCK;
		ddr_writel(val, DDRP_PIR);
		while (ddr_readl(DDRP_PGSR) != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE
						| DDRP_PGSR_DIDONE | DDRP_PGSR_DTDONE)) {
			if(ddr_readl(DDRP_PGSR) & (DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR)) {
				TCSM_PCHAR('e');
				break;
			}
		}
	}
	/**
	 * exit the ddr selrefresh
	 */
	val = ddr_readl(DDRC_CTRL);
	val |= 1 << 1;
	val &= ~(1<< 17);    // exit to hold ddr state
	ddr_writel(val,DDRC_CTRL);
	if(!bypassmode){
		*(volatile unsigned int *)0xb301102c |= (1 << 4);
		TCSM_DELAY(0x1ff);
	}
	if(!REG32(SLEEP_TCSM_RESUME_DATA + 4) && !bypassmode)
	{
		ddr_writel(0x0 , DDRC_DLP);
		{
			val = ddr_readl(DDRP_DSGCR);
			val &= ~(1 << 4);
			ddr_writel(val,DDRP_DSGCR);
		}
	}
	if(REG32(SLEEP_TCSM_RESUME_DATA + 0)) {
		ddr_writel(1,DDRC_AUTOSR_EN);   // enter auto sel-refresh
	}
	dump_ddr_param();
#ifdef DDR_MEM_TEST
	check_ddr_data();
#endif

#ifdef CONFIG_TEST_SECOND_REFRESH
	TCSM_PCHAR('s');
	TCSM_PCHAR('e');
	TCSM_PCHAR('c');
	TCSM_PCHAR('o');
	TCSM_PCHAR('n');
	TCSM_PCHAR('d');
	_setup_tlb();
	TCSM_PCHAR('D');
	s_temp = *(unsigned int *)(SECOND_REFRESH_MAP_ADDR_TO + 4);
	serial_put_hex(s_temp);
	s_func = (int(*)(int)) s_temp;
	s_func(1);

	powerdown_wait();
#endif
	write_c0_ecc(0x0);
	__jz_cache_init();

#ifdef CONFIG_JZ_DMIC_WAKEUP
	/* restore cache attribute */
	__write_32bit_c0_register($12, 2, REG32(SLEEP_TCSM_RESUME_DATA + 32));
#endif
	TCSM_PCHAR('r');
	__asm__ volatile(".set mips32\n\t"
			 "jr %0\n\t"
			 "nop\n\t"
			 ".set mips32 \n\t" :: "r" (restore_goto));

}
static void load_func_to_tcsm(unsigned int *tcsm_addr,unsigned int *f_addr,unsigned int size)
{
	unsigned int instr;
	int offset;
	int i;
	printk("tcsm addr = %p %p size = %d\n",tcsm_addr,f_addr,size);
	for(i = 0;i < size / 4;i++) {
		instr = f_addr[i];
		if((instr >> 26) == 2){
			offset = instr & 0x3ffffff;
			offset = (offset << 2) - ((unsigned int)f_addr & 0xfffffff);
			if(offset > 0) {
				offset = ((unsigned int)tcsm_addr & 0xfffffff) + offset;
				instr = (2 << 26) | (offset >> 2);
			}
		}
		tcsm_addr[i] = instr;
	}
}

static int m200_pm_enter(suspend_state_t state)
{

	unsigned int  lcr_tmp;
	unsigned int opcr_tmp;
	unsigned int gate, spcr0;
	unsigned int core_ctrl;
	unsigned int i;
	unsigned int scpu_start_addr;

#ifdef CONFIG_JZ_DMIC_WAKEUP
	/* if voice identified before deep sleep. just return to wakeup system. */
	int ret;
	ret = wakeup_module_get_sleep_process();
	if(ret == SYS_WAKEUP_OK) {
		return 0;
	}
#endif
	disable_fpu();
#ifdef DDR_MEM_TEST
	test_ddr_data_init();
#endif

	for(i = 0;i < SLEEP_TCSM_DATA_LEN;i += 4)
		REG32(SLEEP_TCSM_RESUME_DATA + i) = 0;
	load_func_to_tcsm((unsigned int *)SLEEP_TCSM_BOOT_TEXT,(unsigned int *)cpu_resume_boot,SLEEP_TCSM_BOOT_LEN);
	load_func_to_tcsm((unsigned int *)SLEEP_TCSM_RESUME_TEXT,(unsigned int *)cpu_resume,SLEEP_TCSM_RESUME_LEN);
	lcr_tmp = read_save_reg_add(CPM_IOBASE + CPM_LCR);
	lcr_tmp &= ~3;
	lcr_tmp |= LCR_LPM_SLEEP;
	lcr_tmp |= (1 << 2);
	cpm_outl(lcr_tmp,CPM_LCR);
	/* OPCR.MASK_INT bit30*/
	/* set Oscillator Stabilize Time bit8*/
	/* disable externel clock Oscillator in sleep mode bit4*/
	/* select 32K crystal as RTC clock in sleep mode bit2*/
	printk("#####opcr:%08x\n", *(volatile unsigned int *)0xb0000024);

	/* set Oscillator Stabilize Time bit8*/
	/* disable externel clock Oscillator in sleep mode bit4*/
	/* select 32K crystal as RTC clock in sleep mode bit2*/
	opcr_tmp = read_save_reg_add(CPM_IOBASE + CPM_OPCR);
	opcr_tmp &= ~((1 << 7) | (1 << 6) | (1 << 4));
	opcr_tmp |= (0xff << 8) | (1<<30) | (1 << 2) | (1 << 27) | (1 << 23);
	cpm_outl(opcr_tmp,CPM_OPCR);
	/*
	 * set sram pdma_ds & open nfi
	 */
	spcr0 = read_save_reg_add(CPM_IOBASE + CPM_SPCR0);
	spcr0 &= ~((1 << 27) | (1 << 2) | (1 << 15) | (1 << 31));
	cpm_outl(spcr0,CPM_SPCR0);

	/*
	 * set clk gate nfi nemc enable pdma
	 */
	gate = read_save_reg_add(CPM_IOBASE + CPM_CLKGR);
	gate &= ~(1 << 21);
	cpm_outl(gate,CPM_CLKGR);

	core_ctrl = get_smp_ctrl();
	set_smp_ctrl(core_ctrl & ~(3 << 8));
	scpu_start_addr = get_smp_reim();

	//read_save_reg_add(0x134f0304);
	//REG32(0xb34f0304) = 0;       // exit auto sel-refresh
	//__fast_iob();
	mb();
	save_goto((unsigned int)cpu_sleep);
	mb();
	restore_all_reg();
#ifdef CONFIG_JZ_DMIC_WAKEUP
	wakeup_module_close(DEEP_SLEEP);
#endif
	set_smp_ctrl(core_ctrl);
	set_smp_reim(scpu_start_addr);
	__write_32bit_c0_register($12, 7, 0);
	return 0;
}
//#define SLEEP_CHANGE_CORE_VCC
static struct m200_early_sleep_t {
#ifdef SLEEP_CHANGE_CORE_VCC
	struct regulator*  core_vcc;
#endif
	struct clk *cpu_clk;
	unsigned int rate_hz;
	unsigned int real_hz;
	unsigned int vol_uv;

}m200_early_sleep;
const unsigned int sleep_rate_hz = 120*1000*1000;
#ifdef SLEEP_CHANGE_CORE_VCC
const unsigned int sleep_vol_uv = 975 * 1000;
#endif
static int m200_prepare(void)
{
#ifdef CONFIG_CPU_FREQ
	struct cpufreq_policy *policy;

	policy = cpufreq_cpu_get(0);
	if(policy) {
		policy->governor->governor(policy, CPUFREQ_GOV_STOP);
		cpufreq_cpu_put(policy);
	}
#endif
#ifdef SLEEP_CHANGE_CORE_VCC
	if(m200_early_sleep.core_vcc == NULL) {
		m200_early_sleep.core_vcc = regulator_get(NULL,"cpu_core_slp");
	}
#endif
	m200_early_sleep.rate_hz = clk_get_rate(m200_early_sleep.cpu_clk);
	clk_set_rate(m200_early_sleep.cpu_clk,sleep_rate_hz);
	m200_early_sleep.real_hz = clk_get_rate(m200_early_sleep.cpu_clk);
#ifdef SLEEP_CHANGE_CORE_VCC
	if(!IS_ERR(m200_early_sleep.core_vcc)) {
		m200_early_sleep.vol_uv = regulator_get_voltage(m200_early_sleep.core_vcc);
		regulator_set_voltage(m200_early_sleep.core_vcc,sleep_vol_uv,sleep_vol_uv);
	}
#endif
	return 0;
}
static void m200_finish(void)
{
#ifdef CONFIG_CPU_FREQ
	struct cpufreq_policy *policy;
#endif
	unsigned int rate;
	rate = clk_get_rate(m200_early_sleep.cpu_clk);
	if(rate != m200_early_sleep.real_hz) {
		printk("warn! current cpu clk %d is not deep sleep set cpu clk %d!\n",
		       rate, m200_early_sleep.real_hz);
	}
#ifdef SLEEP_CHANGE_CORE_VCC
	if(!IS_ERR(m200_early_sleep.core_vcc)) {
		regulator_set_voltage(m200_early_sleep.core_vcc,m200_early_sleep.vol_uv,m200_early_sleep.vol_uv);
	}
#endif
	clk_set_rate(m200_early_sleep.cpu_clk,m200_early_sleep.rate_hz);
#ifdef CONFIG_CPU_FREQ
	policy = cpufreq_cpu_get(0);
	if(policy) {
		policy->governor->governor(policy, CPUFREQ_GOV_START);
		cpufreq_cpu_put(policy);
	}
#endif
}
/*
 * Initialize power interface
 */
struct platform_suspend_ops pm_ops = {
	.valid = suspend_valid_only_mem,
	.enter = m200_pm_enter,
	.prepare = m200_prepare,
	.finish = m200_finish,
};
//extern void ddr_retention_exit(void);
//extern void ddr_retention_entry(void);

int __init m200_pm_init(void)
{
        volatile unsigned int lcr,opcr;//,i;

	suspend_set_ops(&pm_ops);

        /* init opcr and lcr for idle */
        lcr = cpm_inl(CPM_LCR);
        lcr &= ~(0x7);		/* LCR.SLEEP.DS=1'b0,LCR.LPM=2'b00*/
        lcr |= 0xff << 8;	/* power stable time */
        cpm_outl(lcr,CPM_LCR);

        opcr = cpm_inl(CPM_OPCR);
        opcr &= ~(2 << 25);	/* OPCR.PD=2'b00 */
        opcr |= 0xff << 8;	/* EXCLK stable time */
        cpm_outl(opcr,CPM_OPCR);
	m200_early_sleep.cpu_clk = clk_get(NULL, "cclk");
	if (IS_ERR(m200_early_sleep.cpu_clk)) {
		printk("ERROR:cclk request fail!\n");
		suspend_set_ops(NULL);
		return -1;
	}
        /* sysfs */
	return 0;
}

arch_initcall(m200_pm_init);
