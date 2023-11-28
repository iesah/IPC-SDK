/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2011 MIPS Technologies, Inc.
 */
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/mipsregs.h>

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *segments;

static void proc_build_segment_config(char *str, unsigned int cfg)
{
	unsigned int am;
	int len = 0;
	static const char * const am_str[] = {
		"UK  ", "MK  ", "MSK  ", "MUSK  ", "MUSUK  ", "USK  ",
		"*Reserved*  ", "UUSK  "};

	/* Segment access mode. */
	am = (cfg & MIPS_SEGCFG_AM) >> MIPS_SEGCFG_AM_SHIFT;
	len += sprintf(str + len, "%s", am_str[am]);

	/*
	 * Access modes MK, MSK and MUSK are mapped segments. Therefore
	 * there is no direct physical address mapping.
	 */
	if ((am == 0) || (am > 3))
		len += sprintf(str + len, "          %03lx",
			((cfg & MIPS_SEGCFG_PA) >> MIPS_SEGCFG_PA_SHIFT));
	else
		len += sprintf(str + len, "          UND");

	/*
	 * Access modes MK, MSK and MUSK are mapped segments. Therefore
	 * there is no defined cache mode setting.
	 */
	if ((am == 0) || (am > 3))
		len += sprintf(str + len, "         %01ld",
			((cfg & MIPS_SEGCFG_C) >> MIPS_SEGCFG_C_SHIFT));
	else
		len += sprintf(str + len, "         U");

	/* Exception configuration. */
	len += sprintf(str + len, "         %01ld\n",
		((cfg & MIPS_SEGCFG_EU) >> MIPS_SEGCFG_EU_SHIFT));
}

static int proc_read_segments(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;
	unsigned int segcfg;
	char str[42];

	len += sprintf(page + len, "\nSegment   Virtual    Size   Access Mode    Physical    Caching     EU\n");

	len += sprintf(page + len, "-------   --------   ----   -----------   ----------   -------   ------\n");

	segcfg = read_c0_segctl0();
	proc_build_segment_config(str, segcfg);
	len += sprintf(page + len, "   0      e0000000   512M      %s", str);

	segcfg >>= 16;
	proc_build_segment_config(str, segcfg);
	len += sprintf(page + len, "   1      c0000000   512M      %s", str);

	segcfg = read_c0_segctl1();
	proc_build_segment_config(str, segcfg);
	len += sprintf(page + len, "   2      a0000000   512M      %s", str);

	segcfg >>= 16;
	proc_build_segment_config(str, segcfg);
	len += sprintf(page + len, "   3      80000000   512M      %s", str);

	segcfg = read_c0_segctl2();
	proc_build_segment_config(str, segcfg);
	len += sprintf(page + len, "   4      40000000    1G       %s", str);

	segcfg >>= 16;
	proc_build_segment_config(str, segcfg);
	len += sprintf(page + len, "   5      00000000    1G       %s\n", str);

	page += len;
	return len;
}

static int __init segments_info(void)
{
	if (cpu_has_segments) {
		segments = create_proc_read_entry("segments", 0444, NULL,
				proc_read_segments, NULL);
		if (!segments)
			return -ENOMEM;
	}
	return 0;
}

__initcall(segments_info);
#endif /* CONFIG_PROC_FS */
