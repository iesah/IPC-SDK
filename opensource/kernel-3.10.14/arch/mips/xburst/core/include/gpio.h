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

#ifndef __ASM_MACH_INGENIC_GPIO_H__
#define __ASM_MACH_INGENIC_GPIO_H__

#ifdef CONFIG_GPIOLIB
#define gpio_get_value	__gpio_get_value
#define gpio_set_value	__gpio_set_value
#define gpio_cansleep	__gpio_cansleep
#define gpio_to_irq	__gpio_to_irq
#else
#warning "Need GPIOLIB !!!"
#endif
int irq_to_gpio(unsigned irq);

#define ARCH_NR_GPIOS	256

#include <asm-generic/gpio.h>		/* cansleep wrappers */

#define GPIO_PA(n) 	(0*32 + n)
#define GPIO_PB(n) 	(1*32 + n)
#define GPIO_PC(n) 	(2*32 + n)
#define GPIO_PD(n) 	(3*32 + n)
#define GPIO_PE(n) 	(4*32 + n)
#define GPIO_PF(n) 	(5*32 + n)

#define GSS_OUTPUT_LOW 		1
#define GSS_OUTPUT_HIGH 	2
#define GSS_INPUT_PULL 		3
#define GSS_INPUT_NOPULL 	4
#define GSS_IGNORE 		5
#define GSS_TABLET_END		0x999

#endif 
