/*
 * nand/inc/nand_dug.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __NAND_DUG_H__
#define __NAND_DUG_H__

struct nand_dug_msg{
        unsigned char name[20];
        unsigned int  byteperpage;
        unsigned int  pageperblock;
        unsigned int  totalblocks;
};

enum nand_dug_cmd {
        CMD_GET_NAND_PTC,  // the count of nand's partition
        CMD_GET_NAND_MSG,
        CMD_NAND_DUG_READ,
        CMD_NAND_DUG_WRITE,
        CMD_NAND_DUG_ERASE,
};

struct NandInfo{
        int id;
        int bytes;
        int partnum;
        unsigned char *data;
};
#endif /* __NAND_DUG_H__ */
