/*
 * T21 GPIO definitions
 *
 * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
 * Author: Matthew <xu.guo@ingenic.com>
 * Based on: newxboot/modules/gpio/jz4775_gpio.c|jz4780_gpio.c
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

static struct jz_gpio_func_def uart_gpio_func[] = {
	// uart0
	[0] = {.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x9 << 19},
//  [0] = { .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x3 << 8},
	// uart1
	[1] = {.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x3 << 23},
	// uart2
	[2] = {.port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x3 << 13},
//  [2] = { .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3 << 11},
	// uart3
	[3] = {.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3 << 25},
//  [3] = { .port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x3 },
	// uart4
	[4] = {.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3 << 29},
//  [4] = { .port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x3 << 2},
	// uart5
//  [5] = { .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3 << 6},
	[5] = {.port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x3 << 4},
};

static gpio_drive_strength_table_t soc_gpio_drive_strength_table[] = {
	{ GPIO_PORT_B, 0x3f, DS_4_MA }, // MSC0
};

static struct jz_gpio_func_def gpio_func[] = {
#if defined(CONFIG_JZ_MMC_MSC0_PB_4BIT)
	{ .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = (0x3<<4|0xf<<0)},
#endif
#if defined(CONFIG_JZ_MMC_MSC1_PC)
	{ .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = (0x3<<6|0xf<<2)},
#endif
#if defined(CONFIG_JZ_MMC_MSC1_PB)
	{ .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = (0x3<<21|0xf<<17)},
#endif
#ifdef CONFIG_JZ_SFC0_PA
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x3f << 23},
#endif
#ifdef CONFIG_JZ_SFC1_PA
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = (0x7<<20|0x7<<29)},
#endif

};
