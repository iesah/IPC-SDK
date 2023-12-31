/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/regdef.h>
#include <asm/global_info.h>

/* Architecture-specific global data */
struct arch_global_data {
#ifdef CONFIG_JZ4740
	/* There are other clocks in the jz4740 */
	unsigned long per_clk;	/* Peripheral bus clock */
	unsigned long dev_clk;	/* Device clock */
	unsigned long sys_clk;
	unsigned long tbl;
	unsigned long lastinc;
#endif
#if defined(CONFIG_JZ4775) || defined(CONFIG_JZ4780) || defined(CONFIG_M200) \
		|| defined(CONFIG_T10) || defined(CONFIG_T5) || defined(CONFIG_T15G) \
	    || defined(CONFIG_T30) || defined(CONFIG_T21) || defined(CONFIG_T31) \
        || defined(CONFIG_T40) || defined (CONFIG_T41)
	struct global_info *gi;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("k0")

#endif /* __ASM_GBL_DATA_H */
