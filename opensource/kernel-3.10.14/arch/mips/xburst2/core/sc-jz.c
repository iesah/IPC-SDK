/*
 * Copyright (C) 2006 Chris Dearman (chris@mips.com),
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <asm/mipsregs.h>
#include <asm/bcache.h>
#include <asm/cacheops.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/mmu_context.h>
#include <asm/r4kcache.h>

/*
 * MIPS32/MIPS64 L2 cache handling
 */
static unsigned long scache_size __read_mostly;
/*
 * Writeback and invalidate the secondary cache before DMA.
 */
static void mips_sc_wback_inv(unsigned long addr, unsigned long size)
{
	__sync();
	if (size >= scache_size) {
		unsigned long lsize = cpu_scache_line_size();
		if (lsize == 64)
			blast_scache64();
		else if (lsize == 32)
			blast_scache32();
		else
			printk("Error: Second CacheLine size.\n");
	} else {
		blast_scache_range(addr, addr + size);
	}
#ifdef MIPS_BRIDGE_SYNC_WAR
	if (MIPS_BRIDGE_SYNC_WAR)
		__fast_iob();
#endif
}

/*
 * Invalidate the secondary cache before DMA.
 */
static void mips_sc_inv(unsigned long addr, unsigned long size)
{
	unsigned long lsize = cpu_scache_line_size();
	unsigned long almask = ~(lsize - 1);

	if (size >= scache_size) {
		if (lsize == 64)
			blast_scache64();
		else if (lsize == 32)
			blast_scache32();
		else
			printk("Error: Second CacheLine size.\n");
	}else {
		cache_op(Hit_Writeback_Inv_SD, addr & almask);
		cache_op(Hit_Writeback_Inv_SD, (addr + size - 1) & almask);
		blast_inv_scache_range(addr, addr + size);
	}
#ifdef MIPS_BRIDGE_SYNC_WAR
	if (MIPS_BRIDGE_SYNC_WAR)
		__fast_iob();
#endif
}

static void mips_sc_enable(void)
{
	/* L2 cache is permanently enabled */
}

static void mips_sc_disable(void)
{
	/* L2 cache is permanently enabled */
}

static struct bcache_ops mips_sc_ops = {
	.bc_enable = mips_sc_enable,
	.bc_disable = mips_sc_disable,
	.bc_wback_inv = mips_sc_wback_inv,
	.bc_inv = mips_sc_inv
};

/*
 * Check if the L2 cache controller is activated on a particular platform.
 * MTI's L2 controller and the L2 cache controller of Broadcom's BMIPS
 * cores both use c0_config2's bit 12 as "L2 Bypass" bit, that is the
 * cache being disabled.  However there is no guarantee for this to be
 * true on all platforms.  In an act of stupidity the spec defined bits
 * 12..15 as implementation defined so below function will eventually have
 * to be replaced by a platform specific probe.
 */
//static inline int mips_sc_is_activated(struct cpuinfo_mips *c)
//{
//	unsigned int config2 = read_c0_config2();
//	unsigned int tmp;
//
//	/* Check the bypass bit (L2B) */
//	/*switch (c->cputype) {
//	case CPU_34K:
//	case CPU_74K:
//	case CPU_1004K:
//	case CPU_BMIPS5000:
//		if (config2 & (1 << 12))
//			return 0;
//	}*/
//
//	tmp = (config2 >> 4) & 0x0f;
//	if (0 < tmp && tmp <= 7)
//		c->scache.linesz = 2 << tmp;
//	else
//		return 0;
//	return 1;
//}

static inline int __init mips_sc_probe(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;
	unsigned int config1, config2;
	unsigned int tmp;

	/* Mark as not present until probe completed */
	c->scache.flags |= MIPS_CACHE_NOT_PRESENT;

	/* Ignore anything but MIPSxx processors */
	if((c->isa_level & (MIPS_CPU_ISA_M32R1
			| MIPS_CPU_ISA_M32R1
			| MIPS_CPU_ISA_M64R1
			| MIPS_CPU_ISA_M64R2))  == 0) {
		return 0;
	}


	switch (c->processor_id & PRID_CPU_FEATURE_MASK) {
	case PRID_IMP_JZ4775:
	case PRID_IMP_JZ4780:
		return 0;
	case PRID_IMP_M200:
	case PRID_IMP_XBURST2:
		break;
	default:
		printk("pls check processor_id[0x%08x],sc_jz not support!\n",c->processor_id);
	}

	/* Does this MIPS32/MIPS64 CPU have a config2 register? */
	config1 = read_c0_config1();
	if (!(config1 & MIPS_CONF_M))
		return 0;

	config2 = read_c0_config2();

	//if (!mips_sc_is_activated(c))
	tmp = (config2 >> 4) & 0x0f;
	if (0 < tmp && tmp <= 7)
		c->scache.linesz = 2 << tmp;
	else
		return 0;

	tmp = (config2 >> 8) & 0x0f;
	if (0 <= tmp && tmp <= 7) {
		c->scache.sets = 64 << tmp;
	} else {
		return 0;
	}
	tmp = (config2 >> 0) & 0x0f;
	if ((tmp == 7) || (tmp == 15))
		c->scache.ways = tmp + 1;
	else
		return 0;

	c->scache.waysize = c->scache.sets * c->scache.linesz;
	c->scache.waybit = __ffs(c->scache.waysize);

	c->scache.flags &= ~MIPS_CACHE_NOT_PRESENT;
	printk("%s(%d):c->scache.ways=%d, c->scache.sets=%d, c->scache.linesz=%d\n", __func__, __LINE__, c->scache.ways, c->scache.sets, c->scache.linesz);
	scache_size = c->scache.ways * c->scache.sets * c->scache.linesz;
	//write_c0_ecc(0x0);//verify xburst2 whether CP0-$26-0 is implemented.
	return 1;
}

int __cpuinit mips_sc_init(void)
{
	int found = mips_sc_probe();
	//printk("=======found ...... ingenic sc cache ops ...!, found: %d\n\n", found);
	if (found) {
		mips_sc_enable();
		bcops = &mips_sc_ops;
	}
	return found;
}
