/*
 * Copyright (C) 2005 Mips Technologies
 * Author: Chris Dearman, chris@mips.com derived from fpu.h
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#ifndef _ASM_MXU_H
#define _ASM_MXU_H

#include <asm/cpu.h>
#include <asm/cpu-features.h>
#include <asm/hazards.h>
#include <asm/mipsregs.h>

#include <mxu_media.h>

static inline void __init_mxu(void)
{
	unsigned int register val asm("t0");
	val = 3;
	asm volatile(".word	0x7008042f \n\t"::"r"(val));
}

void __save_mxu(void *);
void __restore_mxu(void * tsk_void);

static inline void init_mxu(void)
{
	if(cpu_has_mxu)
		__init_mxu();
}


#define save_mxu(tsk)							\
	do {										\
		if (cpu_has_mxu)						\
			__save_mxu(tsk);					\
	} while (0)

#define restore_mxu(tsk)						\
	do {										\
		if (cpu_has_mxu)						\
			__restore_mxu(tsk);					\
	} while (0)


#endif /* _ASM_MXU_H */
