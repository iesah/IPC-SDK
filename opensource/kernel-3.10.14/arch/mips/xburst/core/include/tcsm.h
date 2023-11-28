/*
 *  Copyright (C) 2008 Ingenic Semiconductor Inc.
 *
 *  Author: <zpzhong@ingenic.cn>
 *
 * This file is copied from asm/mach-generic/gpio.h
 * ans instead of it.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __TCSM_H__
#define __TCSM_H__

extern void tcsm_init(void);
extern void init_tcsm(void);
extern unsigned int get_cpu_tcsm(int cpu,int len,char *name);
extern void cpu0_save_tscm(void);
extern void cpu0_restore_tscm(void);

#endif 
