/*
 * jz mtd spi nand driver probe interface
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Author: cli <chen.li@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later versio.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <nand.h>
#include <spi.h>
#include <spi_flash.h>
#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <asm/arch/spi.h>
#include <asm/io.h>
#include <ingenic_nand_mgr/nand_param.h>
#include "../../../spi/jz_spi.h"
#include "jz_sfc_nand.h"
#include "nand_device/nand_common.h"
#include "jz_sfc_common.h"
#include <cloner/cloner.h>
#include <asm/arch/cpm.h>
#ifndef CONFIG_FPGA
#include <asm/arch/clk.h>
#endif

#ifdef MTDIDS_DEFAULT
static const char *const mtdids_default = MTDIDS_DEFAULT;
#else
static const char *const mtdids_default = "nand0:nand";
#endif

static LIST_HEAD(nand_list);
static	struct sfc_flash *flash;

struct nand_param_from_burner nand_param_from_burner;
struct jz_sfc_nand_burner_param jz_sfc_nand_burner_param;

static int sfcnand_block_checkbad(struct mtd_info *mtd, loff_t ofs,int getchip,int allowbbt);
static int jz_sfcnand_block_markbad(struct mtd_info *mtd, loff_t ofs);

static struct nand_ecclayout gd5f_ecc_layout_128 = {
	.oobavail = 0,
};

static int32_t jz_sfc_nand_erase_blk(struct sfc_flash *flash, uint32_t pageaddr)
{
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct jz_nand_erase *nand_erase_ops = &(nand_desc->ops->nand_erase_ops);
	struct sfc_transfer transfer[2];
	struct sfc_message message;
	struct cmd_info cmd[2];
	struct flash_operation_message op_info = {
		.flash = flash,
		.pageaddr = pageaddr,
		.columnaddr = 0,
		.buffer = NULL,
		.len = 0,
	};
	int32_t ret;

	memset(transfer, 0, sizeof(transfer));
	memset(cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	/*1. write enable */
	if(nand_erase_ops->write_enable)
		nand_erase_ops->write_enable(transfer, cmd, &op_info);
	else
		nand_write_enable(transfer, cmd, &op_info);
	sfc_message_add_tail(transfer, &message);

	if(sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/*2. block erase*/
	if(nand_erase_ops->block_erase)
		nand_erase_ops->block_erase(&transfer[1], &cmd[1], &op_info);
	else
		nand_block_erase(&transfer[1], &cmd[1], &op_info);
	sfc_message_add_tail(&transfer[1], &message);
	if(sfc_sync(flash->sfc, &message)) {
		printf( "sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/*3. get feature*/
	if(nand_erase_ops->get_feature)
		ret = nand_erase_ops->get_feature(&op_info);
	else
		ret = nand_get_erase_feature(&op_info);

	if(ret)
		printf("Erase error,get state error ! %s %s %d \n",__FILE__,__func__,__LINE__);

	return ret;
}

static int jz_sfc_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	//struct sfc_flash *flash = TO_SFC_FLASH(mtd);
	uint32_t addr = (uint32_t)instr->addr;
	uint32_t end;
	int32_t ret;

	if(addr % mtd->erasesize) {
		printf("ERROR:%s line %d eraseaddr no align\n", __func__,__LINE__);
		return -EINVAL;
	}
	end = addr + instr->len;
	instr->state = MTD_ERASING;
	while (addr < end) {
		if((ret = jz_sfc_nand_erase_blk(flash, addr / mtd->writesize))) {
			printf("spi nand erase error blk id  %d !\n",addr / mtd->erasesize);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}
		addr += mtd->erasesize;
	}

	instr->state = MTD_ERASE_DONE;
erase_exit:
	//mtd_erase_callback(instr);
	return ret;
}

static int32_t jz_sfc_nand_write(struct sfc_flash *flash, const u_char *buffer, uint32_t pageaddr, uint32_t columnaddr, size_t len)
{
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct jz_nand_write *nand_write_ops = &nand_desc->ops->nand_write_ops;
	struct sfc_transfer transfer[3];
	struct sfc_message message;
	struct cmd_info cmd[3];
	struct flash_operation_message op_info = {
		.flash = flash,
		.pageaddr = pageaddr,
		.columnaddr = columnaddr,
		.buffer = (u_char *)buffer,
		.len = len,
	};
	int32_t ret = 0;

	memset(transfer, 0, sizeof(transfer));
	memset(cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	/*1. write enable*/
	if(nand_write_ops->write_enable)
		nand_write_ops->write_enable(transfer, cmd, &op_info);
	else
		nand_write_enable(transfer, cmd, &op_info);
	sfc_message_add_tail(transfer, &message);
	if(sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/*2. write to cache*/
	if(nand_desc->param.need_quad) {
		if(nand_write_ops->quad_load)
			nand_write_ops->quad_load(&transfer[1], &cmd[1], &op_info);
		else
			nand_quad_load(&transfer[1], &cmd[1], &op_info);
	} else {
		if(nand_write_ops->single_load)
			nand_write_ops->single_load(&transfer[1], &cmd[1], &op_info);
		else
			nand_single_load(&transfer[1], &cmd[1], &op_info);
	}
	sfc_message_add_tail(&transfer[1], &message);
	if(sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/*3. program exec*/
	if(nand_write_ops->program_exec)
		nand_write_ops->program_exec(&transfer[2], &cmd[2], &op_info);
	else
		nand_program_exec(&transfer[2], &cmd[2], &op_info);
	sfc_message_add_tail(&transfer[2], &message);
	if(sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/*4. get status to be sure nand wirte completed*/
	if(nand_write_ops->get_feature)
		ret = nand_write_ops->get_feature(&op_info);
	else
		ret = nand_get_program_feature(&op_info);

	return  ret;
}

static int32_t jz_sfc_nand_read(struct sfc_flash *flash, int32_t pageaddr, int32_t columnaddr, u_char *buffer, size_t len)
{
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct jz_nand_read *nand_read_ops = &nand_desc->ops->nand_read_ops;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	struct flash_operation_message op_info = {
				.flash = flash,
				.pageaddr = pageaddr,
				.columnaddr = columnaddr,
				.buffer = buffer,
				.len = len,
			};

	int32_t ret = 0;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	if(nand_read_ops->pageread_to_cache)
		nand_read_ops->pageread_to_cache(&transfer, &cmd, &op_info);
	else
		nand_pageread_to_cache(&transfer, &cmd, &op_info);
	sfc_message_add_tail(&transfer, &message);
	if(sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(nand_read_ops->get_feature) {
		ret = nand_read_ops->get_feature(&op_info);
	} else {
		printf("ERROR: nand device must have get_read_feature function,id_manufactory= %02x, id_device=%02x\n", nand_desc->id_manufactory, nand_desc->id_device);
		ret = -EIO;
	}

	if(ret == -EIO)
		return ret;

	if(nand_desc->param.need_quad) {
		if(nand_read_ops->quad_read) {
			nand_read_ops->quad_read(&transfer, &cmd, &op_info);
		} else {
			/*
			 *	default transfer format:
			 *
			 *	addrlen = 2, data_dummy_bits = 8;
			 *
			 * */
			nand_quad_read(&transfer, &cmd, &op_info, 2);
		}
	} else {
		if(nand_read_ops->single_read) {
			nand_read_ops->single_read(&transfer, &cmd, &op_info);
		} else {
			/*
			 *	default transfer format:
			 *
			 *	addrlen = 2, data_dummy_bits = 8;
			 *
			 * */
			nand_single_read(&transfer, &cmd, &op_info, 2);
		}
	}
	sfc_message_add_tail(&transfer, &message);
	if(sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		ret = -EIO;
	}
	return ret;
}

static int jz_sfcnand_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t rlen;
	int32_t ret = 0, reterr = 0, ret_eccvalue = 0;

	*retlen = 0;
	while(len) {
		pageaddr = (uint32_t)from / pagesize;
		columnaddr = (uint32_t)from % pagesize;
		rlen = min_t(uint32_t, len, pagesize - columnaddr);
		ret = jz_sfc_nand_read(flash, pageaddr, columnaddr, buf, rlen);
		if(ret < 0) {
			printf("%s %s %d: jz_sfc_nand_read error, ret = %d, \
				pageaddr = %u, columnaddr = %u, rlen = %u\n",
				__FILE__, __func__, __LINE__,
				ret, pageaddr, columnaddr, rlen);
			reterr = ret;
			if(ret == -EIO)
				break;
		} else if (ret > 0) {
			printf("%s %s %d: jz_sfc_nand_read, ecc value = %d\n",
				__FILE__, __func__, __LINE__, ret);
			ret_eccvalue = ret;
		}

		len -= rlen;
		from += rlen;
		buf += rlen;
		*retlen += rlen;
	}
	return reterr ? reterr : (ret_eccvalue ? ret_eccvalue : ret);
}

static int jz_sfcnand_write_oob(struct mtd_info *mtd, loff_t addr, struct mtd_oob_ops *ops)
{
	uint32_t oob_addr = (uint32_t)addr;
	int32_t ret;

	if((ret = jz_sfc_nand_write(flash, (const u_char *)ops->oobbuf, oob_addr / mtd->writesize, mtd->writesize, ops->ooblen))) {
		printf( "spi nand write oob error %s %s %d \n",__FILE__,__func__,__LINE__);
		goto write_oob_exit;
	}
	ops->retlen = ops->ooblen;
write_oob_exit:
	return ret;
}

static int sfcnand_block_isbad(struct mtd_info *mtd,loff_t ofs)
{
	int ret;
	ret = sfcnand_block_checkbad(mtd, ofs,1, 0);
	return ret;
}

static int sfcnand_block_markbad(struct mtd_info *mtd,loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	int ret;

	ret = sfcnand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}

	return chip->block_markbad(mtd, ofs);
}

static int jz_sfcnand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret;
	if((ret = jz_sfc_nand_erase(mtd, instr))) {
		printf("WARNING: block %d erase fail !\n",(uint32_t)instr->addr / mtd->erasesize);
		if((ret = jz_sfcnand_block_markbad(mtd, instr->addr))) {
			printf("mark bad block error, there will occur error,so exit !\n");
			return -1;
		}
	}
	instr->state = MTD_ERASE_DONE;
	return 0;
}

static int32_t jz_sfcnand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	uint32_t addr = (uint32_t)from;
	uint32_t pageaddr = addr / mtd->writesize;
	int32_t ret = 0;

	if(ops->datbuf) {
		ret = jz_sfc_nand_read(flash, pageaddr, 0, ops->datbuf, mtd->writesize);
		if(ret < 0) {
			printf("spi nand read error %s %s %d ,ret = %d\n", __FILE__, __func__,  __LINE__, ret);
			return ret;
		}
	}
	ret = jz_sfc_nand_read(flash, pageaddr, mtd->writesize + ops->ooboffs, ops->oobbuf, ops->ooblen);
	if(ret < 0){
		printf("spi nand read error %s %s %d ,ret = %d\n", __FILE__, __func__, __LINE__, ret);
	}
	return ret;
}

static int badblk_check(int len,unsigned char *buf)
{
	int i,bit0_cnt = 0;
	unsigned short *check_buf = (unsigned short *)buf;

	if(check_buf[0] != 0xff){
		for(i = 0; i < len * 8; i++){
			if(!((check_buf[0] >> i) & 0x1))
				bit0_cnt++;
		}
	}
	if(bit0_cnt > 6 * len)
		return 1; // is bad blk

	return 0;
}

static int sfcnand_block_checkbad(struct mtd_info *mtd, loff_t ofs,int getchip,int allowbbt)
{
	struct nand_chip *chip = mtd->priv;
	if (!(chip->options & NAND_BBT_SCANNED)) {
		chip->options |= NAND_BBT_SCANNED;
		chip->scan_bbt(mtd);
	}
	if (!chip->bbt)
		return chip->block_bad(mtd, ofs,getchip);

	/* Return info from the table */
	return nand_isbad_bbt(mtd, ofs, allowbbt);
}


static int jz_sfcnand_block_bad_check(struct mtd_info *mtd, loff_t ofs,int getchip)
{
	int check_len = 1;
	unsigned char check_buf[] = {0xaa,0xaa};
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct mtd_oob_ops ops;

	memset(&ops, 0, sizeof(ops));
	if (!(chip->options & NAND_BUSWIDTH_16))
		check_len = 2;

	ops.oobbuf = check_buf;
	ops.ooblen = check_len;
	jz_sfcnand_read_oob(mtd, ofs, &ops);

	if(badblk_check(check_len, check_buf))
		return 1;
	return 0;
}

static int jz_sfcnand_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t wlen;
	int32_t ret;

	while(len) {
		pageaddr = (uint32_t)to / pagesize;
		columnaddr = (uint32_t)to % pagesize;
		wlen = min_t(uint32_t, pagesize - columnaddr, len);

		if((ret = jz_sfc_nand_write(flash, buf, pageaddr, columnaddr, wlen))) {
			printf("spi nand write fail %s %s %d\n",__FILE__,__func__,__LINE__);
			break;
		}
		*retlen += wlen;
		len -= wlen;
		to += wlen;
		buf += wlen;
	}
	return ret;
}

static void mtd_sfcnand_init(struct mtd_info *mtd)
{
	mtd->_erase = jz_sfcnand_erase;
	mtd->_point = NULL;
	mtd->_unpoint = NULL;
	mtd->_read = jz_sfcnand_read;
	mtd->_write = jz_sfcnand_write;
	mtd->_read_oob = jz_sfcnand_read_oob;
	mtd->_write_oob = jz_sfcnand_write_oob;
	mtd->_lock = NULL;
	mtd->_unlock = NULL;
	mtd->_block_isbad = sfcnand_block_isbad;
	mtd->_block_markbad = sfcnand_block_markbad;
}

static int jz_sfcnand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	uint8_t buf[2] = { 0, 0 };
	int res = 0, ret = 0, i = 0;
	int write_oob = !(chip->bbt_options & NAND_BBT_NO_OOB_BBM);

	/* Write bad block marker to OOB */
	if (write_oob) {
		struct mtd_oob_ops ops;
		loff_t wr_ofs = ofs;

		ops.datbuf = NULL;
		ops.oobbuf = buf;
		ops.ooboffs = chip->badblockpos;
		if (chip->options & NAND_BUSWIDTH_16) {
			ops.ooboffs &= ~0x01;
			ops.len = ops.ooblen = 2;
		} else {
			ops.len = ops.ooblen = 1;
		}
		ops.mode = MTD_OPS_PLACE_OOB;

		/* Write to first/last page(s) if necessary */
		if (chip->bbt_options & NAND_BBT_SCANLASTPAGE)
			wr_ofs += mtd->erasesize - mtd->writesize;
		do {
			res = jz_sfcnand_write_oob(mtd, wr_ofs, &ops);
			if (!ret)
				ret = res;

			i++;
			wr_ofs += mtd->writesize;
		} while ((chip->bbt_options & NAND_BBT_SCAN2NDPAGE) && i < 2);
	}

	/* Update flash-based bad block table */
	if (chip->bbt_options & NAND_BBT_USE_FLASH) {
		res = nand_update_bbt(mtd, ofs);
		if (!ret)
			ret = res;
	}

	if (!ret)
		mtd->ecc_stats.badblocks++;

	return ret;

}

#define MAX_NAND_PARTION     20

int set_one_partition_to_nand(int index, char*name, int offset, int size, int mask_flags, int manager_mode)
{
	size_t retlen;
	/*sizeof(struct jz_sfc_nand_burner_param) - 4:means just get magic_num and partion_num*/
	int desc_size = sizeof(struct jz_sfc_nand_burner_param) - 4;
	unsigned int erasesize = flash->mtd->erasesize;

	char *bakeup_first =  malloc(erasesize);
	if(bakeup_first == NULL) {
		printf("malloc bakeup_first memory failed.\n");
		return -1;
	}
	memset(bakeup_first, 0, erasesize);

	/* to read */
	retlen = 0;
	jz_sfcnand_read(flash->mtd, 0, erasesize, &retlen, (u_char *)bakeup_first);
	if (retlen != erasesize) {
		printf("read bakeup_first nand size failed.\n");
		return -1;
	}
	char *bakeup_second = bakeup_first + CONFIG_SPIFLASH_PART_OFFSET;

	/* write partion_table */
	struct jz_sfc_nand_burner_param *part_num = (struct jz_sfc_nand_burner_param*)bakeup_second;
	if(part_num->partition_num < 0){
		part_num->magic_num = 0x646e616e;
		part_num->partition_num = 1;
	}
	jz_sfc_nand_burner_param.partition = (struct jz_spinand_partition*)(bakeup_second + desc_size);
	if (index  >= part_num->partition_num) {
		part_num->partition_num = index + 1;
	}
	printf("magic_num is %x\n",part_num->magic_num);
	printf("partition_num is %d\n",part_num->partition_num);

	memset(jz_sfc_nand_burner_param.partition[index].name, 0, sizeof(jz_sfc_nand_burner_param.partition[index].name));
	memcpy(jz_sfc_nand_burner_param.partition[index].name, name, strlen(name));
	jz_sfc_nand_burner_param.partition[index].size = size;
	jz_sfc_nand_burner_param.partition[index].offset = offset;
	jz_sfc_nand_burner_param.partition[index].mask_flags = mask_flags;
	jz_sfc_nand_burner_param.partition[index].manager_mode = manager_mode;

	/* to erase */
	struct erase_info instr;
	instr.addr = 0;
	instr.len = flash->mtd->erasesize;
	jz_sfcnand_erase(flash->mtd, &instr);

	/* to write */
	retlen = 0;
	jz_sfcnand_write(flash->mtd, 0, erasesize, &retlen, (u_char *)bakeup_first);
	if (retlen != erasesize) {
		printf("write bakeup_first nand size failed, retlen = %d.\n", retlen);
		return -1;
	}

	jz_sfc_nand_burner_param.partition = NULL;
	free(bakeup_first);
	return 0;
}

int get_partition_from_nand(struct jz_sfc_nand_burner_param *param)
{
	size_t retlen, i;
	jz_sfcnand_read(flash->mtd, CONFIG_SPIFLASH_PART_OFFSET, sizeof(struct jz_sfc_nand_burner_param) - 4, &retlen, (u_char *)&jz_sfc_nand_burner_param);
	printf("magic_num is %x\n",jz_sfc_nand_burner_param.magic_num);
	printf("partition_num is %d\n",jz_sfc_nand_burner_param.partition_num);
	if(jz_sfc_nand_burner_param.partition_num < 0)
		return -1;
	jz_sfc_nand_burner_param.partition = malloc(sizeof(struct jz_spinand_partition) * jz_sfc_nand_burner_param.partition_num);

	jz_sfcnand_read(flash->mtd, CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct jz_sfc_nand_burner_param) - 4, sizeof(struct jz_spinand_partition) * jz_sfc_nand_burner_param.partition_num, &retlen, (u_char *)jz_sfc_nand_burner_param.partition);

	for(i = 0; i < jz_sfc_nand_burner_param.partition_num; i++) {
		printf("index = %d, name = %s, offset = 0x%x, size = 0x%x, manager_mode = %d\n",
			   i, jz_sfc_nand_burner_param.partition[i].name, jz_sfc_nand_burner_param.partition[i].offset,
			   jz_sfc_nand_burner_param.partition[i].size,jz_sfc_nand_burner_param.partition[i].manager_mode);
	}
	free(jz_sfc_nand_burner_param.partition);
	return 0;
}

#if 0
static void get_partition_from_spinand(struct sfc_flash *flash)
{
	size_t retlen, i;
	jz_sfcnand_read(flash->mtd, CONFIG_SPIFLASH_PART_OFFSET, sizeof(struct jz_sfc_nand_burner_param) - 4, &retlen, (u_char *)&jz_sfc_nand_burner_param);
	jz_sfc_nand_burner_param.partition = malloc(sizeof(struct jz_spinand_partition) * jz_sfc_nand_burner_param.partition_num);
	jz_sfcnand_read(flash->mtd, CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct jz_sfc_nand_burner_param) - 4, sizeof(struct jz_spinand_partition) * jz_sfc_nand_burner_param.partition_num, &retlen, (u_char *)jz_sfc_nand_burner_param.partition);
#ifdef DEBUG
	for(i = 0; i < jz_sfc_nand_burner_param.partition_num; i++) {
		printf("name = %s\n", jz_sfc_nand_burner_param.partition[i].name);
		printf("size = %x\n", jz_sfc_nand_burner_param.partition[i].size);
		printf("offset = %x\n", jz_sfc_nand_burner_param.partition[i].offset);
		printf("manager_mode = %x\n", jz_sfc_nand_burner_param.partition[i].manager_mode);
	}
#endif
}
#endif
static void sfc_nand_reset(void)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);
	transfer.sfc_mode = TM_STD_SPI;
	cmd.cmd = 0xff;

	transfer.direction = 1;

	transfer.cmd_info = &cmd;
	transfer.ops_mode = CPU_OPS;
	sfc_message_add_tail(&transfer, &message);

	sfc_sync(flash->sfc, &message);

}

static int32_t jz_sfc_nand_try_id(struct sfc_flash *flash, struct jz_nand_descriptor *nand_desc)
{
	struct jz_nand_device *nand_device;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	uint8_t addr_len[2] = {0, 1};
	uint8_t id_buf[2] = {0};
	uint8_t i = 0;

	for(i = 0; i < sizeof(addr_len); i++) {

		memset(id_buf, 0, 2);
		memset(&transfer, 0, sizeof(transfer));
		memset(&cmd, 0, sizeof(cmd));
		sfc_message_init(&message);
		transfer.sfc_mode = TM_STD_SPI;
		cmd.cmd = SPINAND_CMD_RDID;

		transfer.addr = 0;
		transfer.addr_len = addr_len[i];

		cmd.dataen = ENABLE;
		transfer.data = (unsigned char *)id_buf;
		transfer.len = sizeof(id_buf);
		transfer.direction = GLB_TRAN_DIR_READ;
		transfer.data_dummy_bits = 0;

		transfer.cmd_info = &cmd;
		transfer.ops_mode = CPU_OPS;
		sfc_message_add_tail(&transfer, &message);

		if(sfc_sync(flash->sfc, &message)){
			printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
			return -EIO;
		}

		list_for_each_entry(nand_device, &nand_list, nand) {
			if(nand_device->id_manufactory == id_buf[0]) {
				nand_desc->id_manufactory = id_buf[0];
				nand_desc->id_device = id_buf[1];
				break;
			}
		}
		if(nand_desc->id_manufactory && nand_desc->id_device) {
			break;
		}
		udelay(500);
	}

	if(!nand_desc->id_manufactory && !nand_desc->id_device) {
		printf("ERROR!: don`t support this nand manufactory, please add nand driver, id_buf[0]= %x\n", id_buf[0]);
		return -ENODEV;
	}else {
		struct device_id_struct *device_id = nand_device->id_device_list;
		int32_t id_count = nand_device->id_device_count;
		while(id_count--) {
			if(device_id->id_device == nand_desc->id_device) {
				/*notice :base_param and partition param should read from nand*/
				nand_desc->param = *device_id->param;
				break;
			}
			device_id++;
		}
		if(id_count < 0) {
			printf("ERROR: do support this device, id_manufactory = 0x%02x, id_device = 0x%02x\n", nand_desc->id_manufactory, nand_desc->id_device);
			return -ENODEV;
		}
	}
	nand_desc->ops = &nand_device->ops;

	return 0;
}

int jz_spinand_register(struct jz_nand_device *flash) {

	if (!flash)
		return -EINVAL;
	list_add_tail(&flash->nand, &nand_list);
	return 0;
}

/******************************************************************/
static int sfc_nand_get_feature(struct sfc_flash *flash, unsigned char addr, char *val)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	cmd.cmd = 0x0f;
	cmd.dataen = ENABLE;
	transfer.addr_len = 1;
	transfer.addr = addr;
	transfer.len = 1;
	transfer.data = (unsigned char *)val;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_READ;
	transfer.cmd_info = &cmd;

	sfc_message_add_tail(&transfer, &message);
	if(sfc_sync(flash->sfc, &message)) {
			printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return 0;
}

static int sfc_nand_set_feature(struct sfc_flash *flash, int addr, int val)
{
	int ret;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;


	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);


	cmd.cmd = 0x1f;
	cmd.dataen = ENABLE;
	transfer.addr_len = 1;
	transfer.addr = addr;
	transfer.len = 1;
	transfer.data = (unsigned char *)&val;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_WRITE;
	transfer.cmd_info = &cmd;

	sfc_message_add_tail(&transfer, &message);
	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}
	return ret;

}

static int sfc_nand_clear_write_protect(struct sfc_flash *flash)
{
	char val = 0;
//	sfc_nand_get_feature(flash, 0xa0, &val);

	return sfc_nand_set_feature(flash, 0xa0, val);
}

static int sfc_nand_enable_ecc(struct sfc_flash *flash)
{
	char val = 0;
	sfc_nand_get_feature(flash, 0xb0, &val);

	val |= 0x10;

	return sfc_nand_set_feature(flash, 0xb0, val);
}

static int32_t sfc_nand_special_init(struct sfc_flash *flash)
{
	sfc_nand_clear_write_protect(flash);
	sfc_nand_enable_ecc(flash);
	return 0;
}

int jz_sfc_nand_init(int sfc_quad_mode,struct jz_sfc_nand_burner_param *param)
{
	struct nand_chip *chip;
	struct mtd_info *mtd;
	struct jz_nand_descriptor *nand_desc;

	if(!flash) {
		flash = malloc(sizeof(struct sfc_flash));
		if (!flash) {
			printf("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
			return -1;
		}
		memset(flash, 0, sizeof(struct sfc_flash));
		flash->sfc = sfc_res_init(CONFIG_SFC_NAND_RATE);
	}
	mtd = &nand_info[0];
	nand_desc = kzalloc(sizeof(struct jz_nand_descriptor), GFP_KERNEL);
	if(!nand_desc) {
		printf("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
		return -ENOMEM;
	}
	flash->flash_info = nand_desc;
	flash->mtd = mtd;

	if (spinand_moudle_init())
		return -EINVAL;
	sfc_nand_reset();

	jz_sfc_nand_try_id(flash, nand_desc);
#ifdef CONFIG_NAND_BURNER
	/* for burner get pt indext */
	nand_desc->partition.num_partition = param->partition_num;
	nand_desc->partition.partition = param->partition;
#else
	mtd->writesize = nand_desc->param.pagesize;
	get_partition_from_spinand(flash);
	nand_desc->partition.num_partition =jz_sfc_nand_burner_param.partition_num;
	nand_desc->partition.partition = jz_sfc_nand_burner_param.partition;
#endif

	chip = malloc(sizeof(struct nand_chip));
	if (!chip)
		return -ENOMEM;
	memset(chip,0,sizeof(struct nand_chip));

	mtd->size = nand_desc->param.flashsize;
	mtd->flags |= MTD_CAP_NANDFLASH;
	mtd->erasesize = nand_desc->param.blocksize;
	mtd->writesize = nand_desc->param.pagesize;
	mtd->oobsize = nand_desc->param.oobsize;

	chip->select_chip = NULL;
	chip->badblockbits = 8;
	chip->scan_bbt = nand_default_bbt;
	chip->block_bad = jz_sfcnand_block_bad_check;
	chip->block_markbad = jz_sfcnand_block_markbad;
	chip->ecc.layout= &gd5f_ecc_layout_128; // for erase ops
	chip->bbt_erase_shift = chip->phys_erase_shift = ffs(mtd->erasesize) - 1;

	if (!(chip->options & NAND_OWN_BUFFERS))
		chip->buffers = memalign(ARCH_DMA_MINALIGN,sizeof(*chip->buffers));

	/* Set the bad block position */
	if (mtd->writesize > 512 || (chip->options & NAND_BUSWIDTH_16))
		chip->badblockpos = NAND_LARGE_BADBLOCK_POS;
	else
		chip->badblockpos = NAND_SMALL_BADBLOCK_POS;

	mtd->priv = chip;

	mtd_sfcnand_init(mtd);

	sfc_nand_special_init(flash);

	nand_register(0);

	return 0;
}
static int mtd_sfcnand_partition_analysis(unsigned int blk_sz,int partcount,struct jz_spinand_partition *jz_mtd_spinand_partition)
{
	char mtdparts_env[X_ENV_LENGTH];
	char command[X_COMMAND_LENGTH];
	int ptcount = partcount;
	int part;

	memset(mtdparts_env, 0, X_ENV_LENGTH);
	memset(command, 0, X_COMMAND_LENGTH);

	/*MTD part*/
	sprintf(mtdparts_env, "mtdparts=nand:");
	for (part = 0; part < ptcount; part++) {
		if (jz_mtd_spinand_partition[part].size == -1) {
			sprintf(mtdparts_env,"%s-(%s)", mtdparts_env,
					jz_mtd_spinand_partition[part].name);
			break;
		} else if (jz_mtd_spinand_partition[part].size  != 0) {
			if(jz_mtd_spinand_partition[part].size % blk_sz != 0)
				printf("ERROR:the partition [%s] don't algin as block size [0x%08x] ,it will be error !\n",jz_mtd_spinand_partition[part].name,blk_sz);

			sprintf(mtdparts_env,"%s%dK@%d(%s),", mtdparts_env,
					jz_mtd_spinand_partition[part].size / 0x400,
					jz_mtd_spinand_partition[part].offset,
					jz_mtd_spinand_partition[part].name);
		} else {
			break;
		}
	}
	debug("env:mtdparts=%s\n", mtdparts_env);
	setenv("mtdids", mtdids_default);
	setenv("mtdparts", mtdparts_env);
	setenv("partition", NULL);
	return 0;
}


struct jz_spinand_partition *get_partion_index(u32 startaddr,u32 length,int *pt_index)
{
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	int i;
	int ptcount = nand_desc->partition.num_partition;
	struct jz_spinand_partition *jz_mtd_spinand_partition=nand_desc->partition.partition;
	for(i = 0; i < ptcount; i++){
		if(startaddr >= jz_mtd_spinand_partition[i].offset && (startaddr + length) <= (jz_mtd_spinand_partition[i].offset + jz_mtd_spinand_partition[i].size)){
			*pt_index = i;
			break;
		}
	}
	if(i >= ptcount){
		printf("startaddr 0x%08x can't find the pt_index or you partition size  is not align with 128K\n",startaddr);
		*pt_index = -1;
		return NULL;
	}
	return &jz_mtd_spinand_partition[i];
}

int mtd_sfcnand_probe_burner(int *erase_mode,int sfc_quad_mode,struct jz_sfc_nand_burner_param *param)
{
	struct mtd_info *mtd;
	mtd = &nand_info[0];
	struct nand_chip *chip;
	//int wppin = pinfo->gpio_wp;

	if(jz_sfc_nand_init(sfc_quad_mode, param)) {
		printf("ERR: jz_sfc_nand_init error!\n");
		return -EIO;
	}

	/*0: none 1, force-erase, force erase contain creat bbt*/
	if (*erase_mode == 1)
		run_command("nand erase.chip -y", 0);

	chip = mtd->priv;
	chip->scan_bbt(mtd);
	chip->options |= NAND_BBT_SCANNED;
	mtd_sfcnand_partition_analysis(mtd->erasesize, param->partition_num, param->partition);
	return 0;
}
