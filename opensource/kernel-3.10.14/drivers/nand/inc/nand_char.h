/*
 * nand/inc/nand_dug.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __NAND_CHAR_H__
#define __NAND_CHAR_H__

enum nand_char_cmd {
	CMD_PREPARE_NEW_NAND = 94,
	CMD_CHECK_USED_NAND = 95,
	CMD_SOFT_PARTITION_ERASE = 96,
	CMD_SOFT_ERASE_ALL = 97,
	CMD_HARD_PARTITION_ERASE = 98,
	CMD_HARD_ERASE_ALL = 99,
	CMD_GET_NANDFLASH_MAXVALIDBLCOKS = 100,
	CMD_SET_SPL_SIZE = 101,
	CMD_GET_NAND_PAGESIZE=102,
};
#endif /* __NAND_CHAR_H__ */
