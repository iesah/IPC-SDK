/*
 * linux/regulator/ricoh618-regulator.h
 *
 * Regulator driver for RICOH618 power management chip.
 *
 * Copyright (C) 2012-2013 RICOH COMPANY,LTD
 *
 * Based on code
 *	Copyright (C) 2011 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __LINUX_REGULATOR_RICOH618_H
#define __LINUX_REGULATOR_RICOH618_H

#include <linux/regulator/machine.h>


#define ricoh618_rails(_name) ""#_name

/* RICHOH Regulator IDs */
enum regulator_id {
	RICOH618_ID_DC1,
	RICOH618_ID_DC2,
	RICOH618_ID_DC3,
	RICOH618_ID_LDO1,
	RICOH618_ID_LDO2,
	RICOH618_ID_LDO3,
	RICOH618_ID_LDO4,
	RICOH618_ID_LDO5,
	RICOH618_ID_LDORTC1,
	RICOH618_ID_LDORTC2,
	RICOH618_ID_VBUS,
};

uint8_t ricoh618_longpress_pwr[13] = {
		0x00, 0x10, 0x20, 0x30, 0x30, 0x30, 0x40,
		0x40, 0x50, 0x50, 0x60, 0x60, 0x70
};

struct ricoh618_regulator_platform_data {
		struct regulator_init_data regulator;
		int init_uV;
		unsigned init_enable:1;
		unsigned init_apply:1;
		int sleep_uV;
		int sleep_slots;
		unsigned long ext_pwr_req;
		unsigned long flags;
};

#endif
