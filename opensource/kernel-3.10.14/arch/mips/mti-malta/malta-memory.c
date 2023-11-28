/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * PROM library functions for acquiring/using memory descriptors given to
 * us from the YAMON.
 *
 * Copyright (C) 1999,2000,2012  MIPS Technologies, Inc.
 * All rights reserved.
 * Authors: Carsten Langgaard <carstenl@mips.com>
 *          Steven J. Hill <sjhill@mips.com>
 */
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/string.h>

#include <asm/bootinfo.h>
#include <asm/sections.h>
#include <asm/fw/fw.h>

static fw_memblock_t mdesc[FW_MAX_MEMBLOCKS];

#ifdef DEBUG
static char *mtypes[3] = {
	"Dont use memory",
	"YAMON PROM memory",
	"Free memmory",
};
#endif

/* determined physical memory size, not overridden by command line args  */
unsigned long physical_memsize = 0L;

#ifndef CONFIG_EVA
static inline fw_memblock_t * __init prom_getmdesc(void)
{
	char *memsize_str, *ptr;
	unsigned int memsize;
	static char cmdline[COMMAND_LINE_SIZE] __initdata;
	long val;
	int tmp;

	/* otherwise look in the environment */
	memsize_str = fw_getenv("memsize");
	if (!memsize_str) {
		pr_warn("memsize not set in YAMON, set to default (32Mb)\n");
		physical_memsize = 0x02000000;
	} else {
		tmp = kstrtol(memsize_str, 0, &val);
		physical_memsize = (unsigned long)val;
	}

#ifdef CONFIG_CPU_BIG_ENDIAN
	/* SOC-it swaps, or perhaps doesn't swap, when DMA'ing the last
	   word of physical memory */
	physical_memsize -= PAGE_SIZE;
#endif

	/* Check the command line for a memsize directive that overrides
	   the physical/default amount */
	strcpy(cmdline, arcs_cmdline);
	ptr = strstr(cmdline, "memsize=");
	if (ptr && (ptr != cmdline) && (*(ptr - 1) != ' '))
		ptr = strstr(ptr, " memsize=");

	if (ptr)
		memsize = memparse(ptr + 8, &ptr);
	else
		memsize = physical_memsize;

	memset(mdesc, 0, sizeof(mdesc));

	mdesc[0].type = fw_dontuse;
	mdesc[0].base = 0x00000000;
	mdesc[0].size = 0x00001000;

	mdesc[1].type = fw_code;
	mdesc[1].base = 0x00001000;
	mdesc[1].size = 0x000ef000;

	/*
	 * The area 0x000f0000-0x000fffff is allocated for BIOS memory by the
	 * south bridge and PCI access always forwarded to the ISA Bus and
	 * BIOSCS# is always generated.
	 * This mean that this area can't be used as DMA memory for PCI
	 * devices.
	 */
	mdesc[2].type = fw_dontuse;
	mdesc[2].base = 0x000f0000;
	mdesc[2].size = 0x00010000;

	mdesc[3].type = fw_dontuse;
	mdesc[3].base = 0x00100000;
	mdesc[3].size = CPHYSADDR(PFN_ALIGN((unsigned long)&_end)) -
		mdesc[3].base;

	mdesc[4].type = fw_free;
	mdesc[4].base = CPHYSADDR(PFN_ALIGN(&_end));
	mdesc[4].size = memsize - mdesc[4].base;

	return &mdesc[0];
}

#else

static unsigned newMapType;

static inline fw_memblock_t * __init prom_getevamdesc(void)
{
	char *memsize_str;
	char *ememsize_str;
	unsigned long memsize = 0;
	unsigned long ememsize = 0;
	char *ptr;
	static char cmdline[COMMAND_LINE_SIZE] __initdata;

	/* otherwise look in the environment */
	memsize_str = fw_getenv("memsize");
#ifdef DEBUG
	pr_debug("prom_memsize = %s\n", memsize_str);
#endif
	if (memsize_str)
		memsize = simple_strtol(memsize_str, NULL, 0);
	ememsize_str = fw_getenv("ememsize");
#ifdef DEBUG
	pr_debug("fw_ememsize = %s\n", ememsize_str);
#endif
	if (ememsize_str)
		ememsize = simple_strtol(ememsize_str, NULL, 0);

	if ((!memsize) && !ememsize) {
		printk(KERN_WARNING
		       "memsize not set in boot prom, set to default (32Mb)\n");
		physical_memsize = 0x02000000;
	} else {
		physical_memsize = ememsize;
		if (!physical_memsize)
			physical_memsize = memsize;
	}

#ifdef CONFIG_CPU_BIG_ENDIAN
	/* SOC-it swaps, or perhaps doesn't swap, when DMA'ing the last
	   word of physical memory */
	physical_memsize -= PAGE_SIZE;
#endif

	memsize = 0;
	/* Check the command line for a memsize directive that overrides
	   the physical/default amount */
	strcpy(cmdline, arcs_cmdline);
	ptr = strstr(cmdline, " memsize=");
	if (ptr && (ptr != cmdline))
		memsize = memparse(ptr + 9, &ptr);
	ptr = strstr(cmdline, " ememsize=");
	if (ptr && (ptr != cmdline))
		memsize = memparse(ptr + 10, &ptr);
	if (!memsize) {
		ptr = strstr(cmdline, "memsize=");
		if (ptr && (ptr != cmdline))
			memsize = memparse(ptr + 8, &ptr);
		ptr = strstr(cmdline, "ememsize=");
		if (ptr && (ptr != cmdline))
			memsize = memparse(ptr + 9, &ptr);
	}
	if (!memsize)
		memsize = physical_memsize;

	if ((memsize == 0x10000000) && !ememsize)
		if ((!ptr) || (ptr == cmdline)) {
			printk("YAMON reports memsize=256M but doesn't report ememsize option\n");
			printk("If you install > 256MB memory, upgrade YAMON or use boot option memsize=XXXM\n");
		}
	newMapType = *((unsigned int *)CKSEG1ADDR(0xbf403004));
	printk("System Controller register = %0x\n",newMapType);
	newMapType &= 0x100;    /* extract map type bit */
#ifdef CONFIG_EVA_OLD_MALTA_MAP
	if (newMapType)
		panic("Malta board has new memory map layout but kernel is configured for legacy map\n");
	/* Don't use last 64KB - it is just for macros arithmetics overflow */
	if (memsize > 0x7fff0000)
		memsize = 0x7fff0000;
#endif

	memset(mdesc, 0, sizeof(mdesc));

	mdesc[0].type = fw_dontuse;
	mdesc[0].base = PHYS_OFFSET;
	mdesc[0].size = 0x00001000;

	mdesc[1].type = fw_code;
	mdesc[1].base = mdesc[0].base + 0x00001000UL;
	mdesc[1].size = 0x000ef000;

	/*
	 * The area 0x000f0000-0x000fffff is allocated for BIOS memory by the
	 * south bridge and PCI access always forwarded to the ISA Bus and
	 * BIOSCS# is always generated.
	 * This mean that this area can't be used as DMA memory for PCI
	 * devices.
	 */
	mdesc[2].type = fw_dontuse;
	mdesc[2].base = mdesc[0].base + 0x000f0000UL;
	mdesc[2].size = 0x00010000;

	mdesc[3].type = fw_dontuse;
	mdesc[3].base = mdesc[0].base + 0x00100000UL;
	mdesc[3].size = CPHYSADDR(PFN_ALIGN((unsigned long)&_end)) - 0x00100000UL;

#ifndef CONFIG_EVA_OLD_MALTA_MAP
	if (memsize > 0x20000000) {
		/* first 256MB */
		mdesc[4].type = fw_free;
		mdesc[4].base = mdesc[0].base + CPHYSADDR(PFN_ALIGN(&_end));
		mdesc[4].size = mdesc[0].base + 0x10000000 - CPHYSADDR(mdesc[4].base);

		/* I/O hole ... */

		/* the rest of memory (256MB behind hole is lost) */
		mdesc[5].type = fw_free;
		mdesc[5].base = mdesc[0].base + 0x20000000;
		mdesc[5].size = memsize - 0x20000000;
	} else {
		/* limit to 256MB, exclude I/O hole */
		memsize = (memsize > 0x10000000)? 0x10000000 : memsize;

		mdesc[4].type = fw_free;
		mdesc[4].base = mdesc[0].base + CPHYSADDR(PFN_ALIGN(&_end));
		mdesc[4].size = memsize - CPHYSADDR(mdesc[4].base);
	}
#else
	mdesc[4].type = fw_free;
	mdesc[4].base = mdesc[0].base + CPHYSADDR(PFN_ALIGN(&_end));
	mdesc[4].size = memsize - CPHYSADDR(mdesc[4].base);
#endif
	return &mdesc[0];
}

#ifndef CONFIG_EVA_OLD_MALTA_MAP
void __init prom_mem_check(int niocu)
{
	if (!newMapType) {
		if (niocu && mdesc[5].size) {
			printk(KERN_WARNING "Malta board has legacy memory map + IOCU, but kernel is configured for new map layout, restrict memsize to 256MB\n");
			boot_mem_map.nr_map--;
		}
	}
}
#endif /* !CONFIG_EVA_OLD_MALTA_MAP */
#endif /* CONFIG_EVA */

static int __init fw_memtype_classify(unsigned int type)
{
	switch (type) {
	case fw_free:
		return BOOT_MEM_RAM;
	case fw_code:
		return BOOT_MEM_ROM_DATA;
	default:
		return BOOT_MEM_RESERVED;
	}
}

fw_memblock_t __init *fw_getmdesc(void)
{
	fw_memblock_t *p;

#ifndef CONFIG_EVA
	p = prom_getmdesc();
#else
	p = prom_getevamdesc();
#endif
	return p;
}

void __init fw_meminit(void)
{
	fw_memblock_t *p;

#ifdef DEBUG
	pr_debug("YAMON MEMORY DESCRIPTOR dump:\n");
	p = fw_getmdesc();

	while (p->size) {
		int i = 0;
		pr_debug("[%d,%p]: base<%08lx> size<%x> type<%s>\n",
			 i, p, p->base, p->size, mtypes[p->type]);
		p++;
		i++;
	}
#endif
	p = fw_getmdesc();

	while (p->size) {
		long type;
		unsigned long base, size;

		type = fw_memtype_classify(p->type);
		base = p->base;
		size = p->size;

		add_memory_region(base, size, type);
		p++;
	}
}

void __init prom_free_prom_memory(void)
{
	unsigned long addr;
	int i;

	for (i = 0; i < boot_mem_map.nr_map; i++) {
		if (boot_mem_map.map[i].type != BOOT_MEM_ROM_DATA)
			continue;

		addr = boot_mem_map.map[i].addr;
		free_init_pages("YAMON memory",
				addr, addr + boot_mem_map.map[i].size);
	}
}
