/*
 * cmd_nand_zm.c
 *
 * NAND cmd for which nand support the way of zone manager;
 *
 * Copyright (c) 2005-2008 Ingenic Semiconductor Inc.
 *
 */
#include <common.h>
#include <command.h>
#include <nand.h>

#include <linux/mtd/mtd.h>

#define X_COMMAND_LENGTH 128


int do_sfcnand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;
	unsigned int dst_addr,offset,len;
	char command[X_COMMAND_LENGTH];
	int ret;
	long long off;
	unsigned int index, size, manager_mode;
	char name[32] = {0};
	struct mtd_info *nand;
	cmd = argv[1];

	nand = &nand_info[0];
	if ((strcmp(argv[1], "pt") == 0) && (strcmp(argv[2], "show") == 0 ) && (argc == 3))  {
		/* operate nand partion table */
		/* nand pt show */
		ret=get_partition_from_nand(NULL);
		if(ret < 0){
			printf("Partition table has no information, please add it first!\r\n");
			return CMD_RET_USAGE;
		}else
			return CMD_RET_SUCCESS;
	}
	if ((strcmp(argv[1], "pt") == 0) && (argc == 7)) {

		for (off = 0; off < nand->erasesize; off += nand->erasesize)
			if (nand_block_isbad(nand, off)){
				printf("\nuboot partition have bad blocks:\n");
				printf("  %08llx\n", (unsigned long long)off);
				return CMD_RET_FAILURE;
			}
		/* modify partion table : sfcnand pt index name offset size manger_mode */
		/* nand pt index name offset size manager_mode */
		index = (unsigned int)simple_strtoul(argv[2], NULL, 0);
		memcpy(name, argv[3], strlen(argv[3]));
		offset = (unsigned int)simple_strtoul(argv[4], NULL, 16);
		size = (unsigned int)simple_strtoul(argv[5], NULL, 16);
		manager_mode = (unsigned int)simple_strtoul(argv[6], NULL, 0);
		printf("set partion: index = %d, name = %s, offset = 0x%x, size = 0x%x, manager_mode = %d.\n",
			   index, name, offset, size, manager_mode);
		set_one_partition_to_nand(index, name, offset, size, 0, manager_mode);
		return CMD_RET_SUCCESS;
	}

	printf("ERROR: argv error,please check the param of cmd !!!\n");
	return CMD_RET_USAGE;
}
U_BOOT_CMD(sfcnand, 7, 1, do_sfcnand,
		"sfcnand    - SFC_NAND sub-system",
		"\nsfcnand pt show   -Displays the current partition information\n"
		"sfcnand pt [index] [name] [offset] [len] [manager_mode]\n"
		"-`index' Which partition, starting from 0\n"
		"`name' the name of the partition\n"
		"`offset' the offset address of the partition\n"
		"`len' the size of the partition\n"
		"`manager_mode' 0 For MTD_ Mode,1 for UBI_MANAGER\n"
);
void sfc_nand_init(void)
{
	struct nand_chip *chip;
	struct mtd_info *mtd;
	mtd = &nand_info[0];

	jz_sfc_nand_init(0,NULL);

	chip =mtd->priv;
	chip->scan_bbt(mtd);
	chip->options |= NAND_BBT_SCANNED;
}
