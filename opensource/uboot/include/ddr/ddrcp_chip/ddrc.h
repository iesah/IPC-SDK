/*
 * DDR Controller common data structure.
 * Used for X2000
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#ifndef __DDRC_H__
#define __DDRC_H__

typedef union ddrc_timing1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tWL:6;
		unsigned reserved6_7:2;
		unsigned tWR:6;
		unsigned reserved14_15:2;
		unsigned tWTR:6;
		unsigned reserved22_23:2;
		unsigned tWDLAT:6;
		unsigned reserved30_31:2;
	} b;
} ddrc_timing1_t;

typedef union ddrc_timing2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tRL:6;
		unsigned reserved6_7:2;
		unsigned tRTP:6;
		unsigned reserved14_15:2;
		unsigned tRTW:6;
		unsigned reserved22_23:2;
		unsigned tRDLAT:6;
		unsigned reserved30_31:2;
	} b;

} ddrc_timing2_t;

typedef union ddrc_timing3 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tRP:6;
		unsigned reserved6_7:2;
		unsigned tCCD:6;
		unsigned reserved14_15:2;
		unsigned tRCD:6;
		unsigned reserved22_23:2;
		unsigned tEXTRW:3;
		unsigned reserved27_31:5;
	} b;
} ddrc_timing3_t;

typedef union ddrc_timing4 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tRRD:6;
		unsigned reserved6_7:2;
		unsigned tRAS:6;
		unsigned reserved14_15:2;
#if (defined(CONFIG_X2000_V12) || defined(CONFIG_M300)) || defined(CONFIG_X2100)
		unsigned tRC:7;
		unsigned reserved23:1;
#else
		unsigned tRC:6;
		unsigned reserved22_23:2;
#endif
		unsigned tFAW:8;
	} b;
} ddrc_timing4_t;

typedef union ddrc_timing5 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
#if (defined(CONFIG_X2000_V12) || defined(CONFIG_M300)) || defined(CONFIG_X2100)
		unsigned tCKE:4;
#else
		unsigned tCKE:3;
		unsigned reserved3:1;
#endif
		unsigned tXP:4;
		unsigned reserved8_11:4;
		unsigned tCKSRE:4;
		unsigned tCKESR:8;
		unsigned tXS:8;
	} b;
} ddrc_timing5_t;

typedef union ddrc_cfg {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned CS0EN:1;
		unsigned CS1EN:1;
		unsigned ODTEN:1;
		unsigned TYPE:3;
		unsigned reserved6_8:3;
		unsigned BA0:1;
		unsigned COL0:3;
		unsigned ROW0:3;
		unsigned IMBA:1;
		unsigned reserved17_24:8;
		unsigned BA1:1;
		unsigned COL1:3;
		unsigned ROW1:3;
	} b;
} ddrc_cfg_t;

struct ddrc_reg {
	ddrc_cfg_t cfg;
	uint32_t ctrl;
	uint32_t ddlp;
	uint32_t refcnt;
	uint32_t dlmr;
	uint32_t mmap[2];
	uint32_t remap[5];
	ddrc_timing1_t timing1;
	ddrc_timing2_t timing2;
	ddrc_timing3_t timing3;
	ddrc_timing4_t timing4;
	ddrc_timing5_t timing5;
	uint32_t autosr_cnt;
	uint32_t autosr_en;
	uint32_t hregpro;
	uint32_t pregpro;
	uint32_t cguc0;
	uint32_t cguc1;
};

#endif /* __DDRC_H__ */
