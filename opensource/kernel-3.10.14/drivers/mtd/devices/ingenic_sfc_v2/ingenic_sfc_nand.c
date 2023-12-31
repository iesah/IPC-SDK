/*
 * SFC controller for SPI protocol, use FIFO and DMA;
 *
 * Copyright (c) 2015 Ingenic
 * Author: <xiaoyang.fu@ingenic.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include "sfc_flash.h"
#include "spinand.h"
#include "ingenic_sfc_common.h"
#include "ingenic_sfc_drv.h"
#include "./nand_device/nand_common.h"


#define STATUS_SUSPND	(1<<0)
#define to_ingenic_spi_nand(mtd_info) container_of(mtd_info, struct sfc_flash, mtd)

/*
 * below is the informtion about nand
 * that user should modify according to nand spec
 * */

static LIST_HEAD(nand_list);

void dump_flash_info(struct sfc_flash *flash)
{
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct ingenic_sfcnand_base_param *param = &nand_info->param;
	struct mtd_partition *partition = nand_info->partition.partition;
	uint8_t num_partition = nand_info->partition.num_partition;

	printk("id_manufactory = 0x%02x\n", nand_info->id_manufactory);
	printk("id_device = 0x%02x\n", nand_info->id_device);

	printk("pagesize = %d\n", param->pagesize);
	printk("blocksize = %d\n", param->blocksize);
	printk("oobsize = %d\n", param->oobsize);
	printk("flashsize = %d\n", param->flashsize);

	printk("tHOLD = %d\n", param->tHOLD);
	printk("tSETUP = %d\n", param->tSETUP);
	printk("tSHSL_R = %d\n", param->tSHSL_R);
	printk("tSHSL_W = %d\n", param->tSHSL_W);

	printk("ecc_max = %d\n", param->ecc_max);
	printk("need_quad = %d\n", param->need_quad);

	while(num_partition--) {
		printk("partition(%d) name=%s\n", num_partition, partition[num_partition].name);
		printk("partition(%d) size = 0x%llx\n", num_partition, partition[num_partition].size);
		printk("partition(%d) offset = 0x%llx\n", num_partition, partition[num_partition].offset);
		printk("partition(%d) mask_flags = 0x%x\n", num_partition, partition[num_partition].mask_flags);
	}
	return;
}

static int32_t ingenic_sfc_nand_read(struct sfc_flash *flash, int32_t pageaddr, int32_t columnaddr, u_char *buffer, size_t len)
{
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct ingenic_sfcnand_ops *ops = nand_info->ops;
	struct sfc_cdt_xfer xfer;
	int32_t ret = 0;

	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	if(nand_info->param.need_quad){
		xfer.cmd_index = NAND_QUAD_READ_TO_CACHE;
	}else{
		xfer.cmd_index = NAND_STANDARD_READ_TO_CACHE;
	}

	/* set addr */
	xfer.rowaddr = pageaddr;

	if(nand_info->param.plane_select){
		xfer.columnaddr = CONVERT_COL_ADDR(pageaddr, columnaddr);
	}else{
		xfer.columnaddr = columnaddr;
	}

	xfer.staaddr0 = SPINAND_ADDR_STATUS;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = len;
	xfer.config.data_dir = GLB0_TRAN_DIR_READ;
	xfer.config.ops_mode = DMA_OPS;
	xfer.config.buf = buffer;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		dev_err(flash->dev,"sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/* get status to check nand ecc status */
	ret = ops->get_feature(flash, GET_ECC_STATUS);

	if(xfer.config.ops_mode == DMA_OPS)
		dma_cache_sync(NULL, (void *)xfer.config.buf, xfer.config.datalen, DMA_FROM_DEVICE);

	return ret;
}

static int badblk_check(int len, unsigned char *buf)
{
	int  j;
	unsigned char *check_buf = buf;

	for(j = 0; j < len; j++){
		if(check_buf[j] != 0xff){
			return 1;
		}
	}
	return 0;
}

static int32_t ingenic_sfc_nand_write(struct sfc_flash *flash, u_char *buffer, uint32_t pageaddr, uint32_t columnaddr, size_t len)
{
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct ingenic_sfcnand_ops *ops = nand_info->ops;
	struct sfc_cdt_xfer xfer;
	int32_t ret = 0;

	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	if(nand_info->param.need_quad){
		xfer.cmd_index = NAND_QUAD_WRITE_ENABLE;
	}else{
		xfer.cmd_index = NAND_STANDARD_WRITE_ENABLE;
	}

	/* set addr */
	xfer.rowaddr = pageaddr;

	if(nand_info->param.plane_select){
		xfer.columnaddr = CONVERT_COL_ADDR(pageaddr, columnaddr);
	}else{
		xfer.columnaddr = columnaddr;
	}

	xfer.staaddr0 = SPINAND_ADDR_STATUS;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = len;
	xfer.config.data_dir = GLB0_TRAN_DIR_WRITE;
	xfer.config.ops_mode = DMA_OPS;
	xfer.config.buf = buffer;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		dev_err(flash->dev,"sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/* get status to be sure nand write completed */
	ret = ops->get_feature(flash, GET_WRITE_STATUS);

	return  ret;
}

static int ingenic_sfcnand_write_oob(struct mtd_info *mtd, loff_t addr, struct mtd_oob_ops *ops)
{
	struct sfc_flash *flash = to_ingenic_spi_nand(mtd);
	uint32_t oob_addr = (uint32_t)addr;
	int32_t ret;

	mutex_lock(&flash->lock);
	if(ops->datbuf && ops->len)
	{
		/* create DMA Descriptors */
		ret = create_sfc_desc(flash, ops->datbuf, ops->len);
		if(ret < 0){
			dev_err(flash->dev, "%s create descriptors error. -%d\n", __func__, ret);
			goto write_oob_exit;
		}

		if((ret = ingenic_sfc_nand_write(flash, ops->datbuf, oob_addr / mtd->writesize, oob_addr % mtd->writesize, ops->len))) {
			dev_err(flash->dev, "spi nand write oob error %s %s %d \n",__FILE__,__func__,__LINE__);
			goto write_oob_exit;
		}
	}

	if(ops->oobbuf && ops->ooblen)
	{
		/* create DMA Descriptors */
		ret = create_sfc_desc(flash, ops->oobbuf, ops->ooblen);
		if(ret < 0){
			dev_err(flash->dev, "%s create descriptors error. -%d\n", __func__, ret);
			goto write_oob_exit;
		}

		if((ret = ingenic_sfc_nand_write(flash, ops->oobbuf, oob_addr / mtd->writesize, mtd->writesize, ops->ooblen))) {
			dev_err(flash->dev, "spi nand write oob error %s %s %d \n",__FILE__,__func__,__LINE__);
			goto write_oob_exit;
		}
	}
	ops->retlen = ops->len;
	ops->oobretlen = ops->ooblen;
write_oob_exit:
	mutex_unlock(&flash->lock);
	return ret;
}

static int ingenic_sfcnand_chip_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	uint8_t buf[2] = { 0, 0 };
	int  ret = 0, i = 0;
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
			ret = ingenic_sfcnand_write_oob(mtd, wr_ofs, &ops);
			if (ret)
				return ret;
			wr_ofs += mtd->writesize;
			i++;
		} while ((chip->bbt_options & NAND_BBT_SCAN2NDPAGE) && i < 2);
	}
	/* Update flash-based bad block table */
	if (chip->bbt_options & NAND_BBT_USE_FLASH) {
		ret = nand_update_bbt(mtd, ofs);
	}

	return ret;
}

static int32_t ingenic_sfc_nand_erase_blk(struct sfc_flash *flash, uint32_t pageaddr)
{
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct ingenic_sfcnand_ops *ops = nand_info->ops;
	struct sfc_cdt_xfer xfer;
	int32_t ret = 0;

	memset(&xfer, 0, sizeof(xfer));

	/* set index */
	xfer.cmd_index = NAND_ERASE_WRITE_ENABLE;

	/* set addr */
	xfer.rowaddr = pageaddr;
	xfer.staaddr0 = SPINAND_ADDR_STATUS;

	/* set transfer config */
	xfer.dataen = DISABLE;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		dev_err(flash->dev,"sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/* get status to be sure nand write completed */
	ret = ops->get_feature(flash, GET_ERASE_STATUS);
	if(ret){
		dev_err(flash->dev, "Erase error, get state error ! %s %s %d \n",__FILE__,__func__,__LINE__);
	}

	return ret;
}

static int ingenic_sfcnand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct sfc_flash *flash = to_ingenic_spi_nand(mtd);
	uint32_t addr = (uint32_t)instr->addr;
	uint32_t end;
	int32_t ret;

	if(addr % mtd->erasesize) {
		dev_err(flash->dev, "ERROR:%s line %d eraseaddr no align\n", __func__,__LINE__);
		return -EINVAL;
	}
	end = addr + instr->len;
	instr->state = MTD_ERASING;
	mutex_lock(&flash->lock);
	while (addr < end) {
		if((ret = ingenic_sfc_nand_erase_blk(flash, addr / mtd->writesize))) {
			dev_err(flash->dev, "spi nand erase error blk id  %d !\n",addr / mtd->erasesize);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}
		addr += mtd->erasesize;
	}

	instr->state = MTD_ERASE_DONE;
erase_exit:
	mutex_unlock(&flash->lock);
	return ret;
}

static int ingenic_sfcnand_block_isbab(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	if(!chip->bbt)
		return chip->block_bad(mtd, ofs, 1);
	return nand_isbad_bbt(mtd, ofs, 0);
}

static int ingenic_sfcnand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	int ret = ingenic_sfcnand_block_isbab(mtd, ofs);
	if(ret > 0) {
		/* If it was bad already, return success and do nothing */
			return 0;
	}
	return chip->block_markbad(mtd, ofs);
}

static int ingenic_sfcnand_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct sfc_flash *flash = to_ingenic_spi_nand(mtd);
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t rlen;
	int32_t ret = 0, reterr = 0, ret_eccvalue = 0;

	mutex_lock(&flash->lock);
	while(len) {
		pageaddr = (uint32_t)from / pagesize;
		columnaddr = (uint32_t)from % pagesize;
		rlen = min_t(uint32_t, len, pagesize - columnaddr);

		/* create DMA Descriptors */
		ret = create_sfc_desc(flash, buf, rlen);
		if(ret < 0){
			dev_err(flash->dev, "%s create descriptors error. -%d\n", __func__, ret);
			return ret;
		}

		/* DMA Descriptors read */
		ret = ingenic_sfc_nand_read(flash, pageaddr, columnaddr, buf, rlen);
		if(ret < 0) {
			dev_err(flash->dev, "%s %s %d: ingenic_sfc_nand_read error, ret = %d, \
				pageaddr = %u, columnaddr = %u, rlen = %u\n",
					__FILE__, __func__, __LINE__,
					ret, pageaddr, columnaddr, rlen);
			reterr = ret;
			if(ret == -EIO)
				break;
		} else if (ret > 0) {
			dev_dbg(flash->dev, "%s %s %d: ingenic_sfc_nand_read, ecc value = %d, \
				    pageaddr = %u, columnaddr = %u, rlen = %u\n",
					    __FILE__, __func__, __LINE__,
					    ret, pageaddr, columnaddr, rlen);
			ret_eccvalue = ret;
		}

		len -= rlen;
		from += rlen;
		buf += rlen;
		*retlen += rlen;
	}
	mutex_unlock(&flash->lock);
	return reterr ? reterr : (ret_eccvalue ? ret_eccvalue : ret);
}

static int ingenic_sfcnand_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct sfc_flash *flash = to_ingenic_spi_nand(mtd);
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t wlen;
	int32_t ret;

	mutex_lock(&flash->lock);
	while(len) {
		pageaddr = (uint32_t)to / pagesize;
		columnaddr = (uint32_t)to % pagesize;
		wlen = min_t(uint32_t, pagesize - columnaddr, len);
		/* create DMA Descriptors */
		ret = create_sfc_desc(flash, (unsigned char *)buf, wlen);
		if(ret < 0){
			dev_err(flash->dev, "%s create descriptors error. -%d\n", __func__, ret);
			return ret;
		}

		/* DMA Descriptors write */
		if((ret = ingenic_sfc_nand_write(flash, (u_char *)buf, pageaddr, columnaddr, wlen))) {
			dev_err(flash->dev, "%s %s %d : spi nand write fail, ret = %d, \
				pageaddr = %u, columnaddr = %u, wlen = %u\n",
				__FILE__, __func__, __LINE__, ret,
				pageaddr, columnaddr, wlen);
			break;
		}
		*retlen += wlen;
		len -= wlen;
		to += wlen;
		buf += wlen;
	}
	mutex_unlock(&flash->lock);
	return ret;
}

static int32_t ingenic_sfcnand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	struct sfc_flash *flash = to_ingenic_spi_nand(mtd);
	uint32_t addr = (uint32_t)from;
	uint32_t pageaddr = addr / mtd->writesize;
	uint32_t columnaddr = addr % mtd->writesize;
	int32_t ret = 0, ret_eccvalue = 0;

	mutex_lock(&flash->lock);
	if(ops->datbuf) {
		/* create DMA Descriptors */
		ret = create_sfc_desc(flash, ops->datbuf, ops->len);
		if(ret < 0){
			dev_err(flash->dev, "%s create descriptors error. -%d\n", __func__, ret);
			goto read_oob_exit;
		}
		/* DMA Descriptors read */
		ret = ingenic_sfc_nand_read(flash, pageaddr, columnaddr, ops->datbuf, ops->len);
		if(ret < 0) {
			dev_err(flash->dev, "%s %s %d : spi nand read data error, ret = %d\n",__FILE__,__func__,__LINE__, ret);
			if(ret == -EIO) {
				goto read_oob_exit;
			} else {
				ret_eccvalue = ret;
			}
		}
	}
	if(ops->oobbuf) {
		/* create DMA Descriptors */
		ret = create_sfc_desc(flash, ops->oobbuf, ops->ooblen);
		if(ret < 0){
			dev_err(flash->dev, "%s create descriptors error. -%d\n", __func__, ret);
			goto read_oob_exit;
		}

		ret = ingenic_sfc_nand_read(flash, pageaddr, mtd->writesize + ops->ooboffs, ops->oobbuf, ops->ooblen);
		if(ret < 0)
			dev_err(flash->dev, "%s %s %d : spi nand read oob error ,ret= %d\n", __FILE__, __func__, __LINE__, ret);

		if(ret != -EIO)
			ops->oobretlen = ops->ooblen;

	}
read_oob_exit:
	mutex_unlock(&flash->lock);

	return ret ? ret : ret_eccvalue;
}

static int ingenic_sfcnand_block_bad_check(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int check_len = 1;
	unsigned char check_buf[2] = {0x0};
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct mtd_oob_ops ops;

	memset(&ops, 0, sizeof(ops));
	if (chip->options & NAND_BUSWIDTH_16)
		check_len = 2;

	ops.oobbuf = check_buf;
	ops.ooblen = check_len;
	ingenic_sfcnand_read_oob(mtd, ofs, &ops);
	if(badblk_check(check_len, check_buf))
		return 1;
	return 0;
}

static int ingenic_sfc_nand_set_feature(struct sfc_flash *flash, uint8_t addr, uint32_t val)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set index */
	xfer.cmd_index = NAND_SET_FEATURE;

	/* set addr */
	xfer.staaddr0 = addr;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 1;
	xfer.config.data_dir = GLB0_TRAN_DIR_WRITE;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = (uint8_t *)&val;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		dev_err(flash->dev,"sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return 0;
}

static int ingenic_sfc_nand_get_feature(struct sfc_flash *flash, uint8_t addr, uint8_t *val)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set index */
	xfer.cmd_index = NAND_GET_FEATURE;

	/* set addr */
	xfer.staaddr0 = addr;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 1;
	xfer.config.data_dir = GLB0_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = (uint8_t *)val;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		dev_err(flash->dev,"sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return 0;
}

static int32_t ingenic_sfc_nand_dev_init(struct sfc_flash *flash)
{
	int32_t ret;
	/*release protect*/
	uint8_t feature = 0;
	if((ret = ingenic_sfc_nand_set_feature(flash, SPINAND_ADDR_PROTECT, feature)))
		goto exit;

	if((ret = ingenic_sfc_nand_get_feature(flash, SPINAND_ADDR_FEATURE, &feature)))
		goto exit;

	feature |= (1 << 4) | (1 << 3) | (1 << 0);
	if((ret = ingenic_sfc_nand_set_feature(flash, SPINAND_ADDR_FEATURE, feature)))
		goto exit;

	return 0;
exit:
	return ret;
}

static int32_t __init ingenic_sfc_nand_try_id(struct sfc_flash *flash)
{
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct ingenic_sfcnand_device *nand_device;
	struct sfc_cdt_xfer xfer;
	uint8_t id_buf[2] = {0};
	unsigned short index[2] = {NAND_TRY_ID, NAND_TRY_ID_DMY};
	uint8_t i = 0;

	for(i = 0; i < 2; i++){
		memset(&xfer, 0, sizeof(xfer));

		/* set index */
		xfer.cmd_index = index[i];

		/* set addr */
		xfer.rowaddr = 0;

		/* set transfer config */
		xfer.dataen = ENABLE;
		xfer.config.datalen = sizeof(id_buf);
		xfer.config.data_dir = GLB0_TRAN_DIR_READ;
		xfer.config.ops_mode = CPU_OPS;
		xfer.config.buf = id_buf;

		if(sfc_sync_cdt(flash->sfc, &xfer)) {
			dev_err(flash->dev,"sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
			return -EIO;
		}

		list_for_each_entry(nand_device, &nand_list, list) {
			if(nand_device->id_manufactory == id_buf[0]) {
				nand_info->id_manufactory = id_buf[0];
				nand_info->id_device = id_buf[1];
				break;
			}
		}

		if(nand_info->id_manufactory && nand_info->id_device)
			    break;
	}

	if(!nand_info->id_manufactory && !nand_info->id_device) {
		dev_err(flash->dev, " ERROR!: don`t support this nand manufactory, please add nand driver.\n");
		return -ENODEV;
	} else {
		struct device_id_struct *device_id = nand_device->id_device_list;
		int32_t id_count = nand_device->id_device_count;
		while(id_count--) {
			if(device_id->id_device == nand_info->id_device) {
			/* notice :base_param and partition param should read from nand */
				nand_info->param = *device_id->param;
				break;
			}
			device_id++;
		}
		if(id_count < 0) {
			dev_err(flash->dev, "ERROR: do support this device, id_manufactory = 0x%02x, id_device = 0x%02x\n", nand_info->id_manufactory, nand_info->id_device);
			return -ENODEV;
		}
		dev_info(flash->dev, "Found Supported nand device, id = 0x%02x%02x,name = %s\n", nand_info->id_manufactory, nand_info->id_device, device_id->name);
	}


	/* fill manufactory special operation and cdt params */
	nand_info->ops = &nand_device->ops;
	nand_info->cdt_params = nand_info->ops->get_cdt_params(flash, nand_info->id_device);

	if (!nand_info->ops->get_feature) {
		if (!nand_info->ops->deal_ecc_status) {
			dev_err(flash->dev,"ERROR:xxx_nand.c \"get_feature()\" and \"deal_ecc_status()\" not define.\n");
			return -ENODEV;
		} else {
			nand_info->ops->get_feature = nand_common_get_feature;
			printk("use nand common get feature interface!\n");
		}
	} else {
		printk("use nand private get feature interface!\n");
	}

	return 0;
}

static int32_t __init nand_partition_param_copy(struct sfc_flash *flash, struct ingenic_sfcnand_burner_param *burn_param) {
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	int i = 0, count = 5, ret;
	size_t retlen = 0;

	/* partition param copy */
	nand_info->partition.num_partition = burn_param->partition_num;

	burn_param->partition = kzalloc(nand_info->partition.num_partition * sizeof(struct ingenic_sfcnand_partition), GFP_KERNEL);
	if (IS_ERR_OR_NULL(burn_param->partition)) {
	    	dev_err(flash->dev, "alloc partition space failed!\n");
			return -ENOMEM;
	}

	nand_info->partition.partition = kzalloc(nand_info->partition.num_partition * sizeof(struct mtd_partition), GFP_KERNEL);
	if (IS_ERR_OR_NULL(nand_info->partition.partition)) {
	    	dev_err(flash->dev, "alloc partition space failed!\n");
		kfree(burn_param->partition);
		return -ENOMEM;
	}

partition_retry_read:
	ret = ingenic_sfcnand_read(&flash->mtd, flash->param_offset + sizeof(*burn_param) - sizeof(burn_param->partition),
		nand_info->partition.num_partition * sizeof(struct ingenic_sfcnand_partition),
		&retlen, (u_char *)burn_param->partition);

	if((ret < 0) && count--)
		goto partition_retry_read;
	if(count < 0) {
		dev_err(flash->dev, "read nand partition failed!\n");
		kfree(burn_param->partition);
		kfree(nand_info->partition.partition);
		return -EIO;
	}

	for(i = 0; i < burn_param->partition_num; i++) {
		nand_info->partition.partition[i].name = burn_param->partition[i].name;
		nand_info->partition.partition[i].size = burn_param->partition[i].size;
		nand_info->partition.partition[i].offset = burn_param->partition[i].offset;
		nand_info->partition.partition[i].mask_flags = burn_param->partition[i].mask_flags;
	}
	return 0;
}

static struct ingenic_sfcnand_burner_param *burn_param;
static int32_t __init flash_part_from_chip(struct sfc_flash *flash) {

	int32_t ret = 0, retlen = 0, count = 5;

	burn_param = kzalloc(sizeof(struct ingenic_sfcnand_burner_param), GFP_KERNEL);
	if(IS_ERR_OR_NULL(burn_param)) {
		dev_err(flash->dev, "alloc burn_param space error!\n");
		return -ENOMEM;
	}

	count = 5;
param_retry_read:
	ret = ingenic_sfcnand_read(&flash->mtd, flash->param_offset,
		sizeof(struct ingenic_sfcnand_burner_param), &retlen, (u_char *)burn_param);
	if((ret < 0) && count--)
		goto param_retry_read;
	if(count < 0) {
		dev_err(flash->dev, "read nand base param failed!\n");
		ret = -EIO;
		goto failed;
	}

	if(burn_param->magic_num != SPINAND_MAGIC_NUM) {
		dev_info(flash->dev, "NOTICE: this flash haven`t param, magic_num:%x\n", burn_param->magic_num);
		ret = -EINVAL;
		goto failed;
	}

	if(nand_partition_param_copy(flash, burn_param)) {
		ret = -ENOMEM;
		goto failed;
	}

	return 0;
failed:
	kfree(burn_param);
	return ret;

}

static int32_t __init flash_part_from_board(struct sfc_flash *flash, struct ingenic_sfc_info *board_info) {
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct ingenic_sfcnand_partition *flash_partition = board_info->flash_partition;
	int8_t i = 0;

	nand_info->partition.num_partition = board_info->num_partition;
	nand_info->partition.partition = kzalloc(nand_info->partition.num_partition * sizeof(struct mtd_partition), GFP_KERNEL);
	if (IS_ERR_OR_NULL(nand_info->partition.partition)) {
		dev_err(flash->dev, "alloc partition space failed!\n");
		nand_info->partition.num_partition = 0;
		return -ENOMEM;
	}

	for(i = 0; i < board_info->num_partition; i++) {
		nand_info->partition.partition[i].name = flash_partition[i].name;
		nand_info->partition.partition[i].size = flash_partition[i].size;
		nand_info->partition.partition[i].offset = flash_partition[i].offset;
		nand_info->partition.partition[i].mask_flags = flash_partition[i].mask_flags;
	}
	return 0;
}

static int32_t __init ingenic_sfcnand_partition(struct sfc_flash *flash, struct ingenic_sfc_info *board_info) {
	int32_t ret = 0;
	if(!board_info || !board_info->use_board_info) {
		if((ret = flash_part_from_chip(flash)))
			dev_info(flash->dev, "read partition from flash failed!\n");
	} else {
		if((ret = flash_part_from_board(flash, board_info)))
			dev_err(flash->dev, "copy partition from board failed!\n");
	}
	return ret;
}

int ingenic_sfcnand_register(struct ingenic_sfcnand_device *flash) {
	list_add_tail(&flash->list, &nand_list);
	return 0;
}
EXPORT_SYMBOL_GPL(ingenic_sfcnand_register);

/*
 *MK_CMD(cdt, cmd, LINK, ADDRMODE, DATA_EN)
 *MK_ST(cdt, st, LINK, ADDRMODE, ADDR_WIDTH, POLL_EN, DATA_EN, TRAN_MODE)
 */
static void params_to_cdt(cdt_params_t *params, struct sfc_cdt *cdt)
{
	/* 6. nand standard read */
	MK_CMD(cdt[NAND_STANDARD_READ_TO_CACHE], params->r_to_cache, 1, ROW_ADDR, DISABLE);
	MK_ST(cdt[NAND_STANDARD_READ_GET_FEATURE], params->oip, 1, STA_ADDR0, 1, ENABLE, DISABLE, TM_STD_SPI);
	MK_CMD(cdt[NAND_STANDARD_READ_FROM_CACHE], params->standard_r, 0, COL_ADDR, ENABLE);

	/* 7. nand quad read */
	MK_CMD(cdt[NAND_QUAD_READ_TO_CACHE], params->r_to_cache, 1, ROW_ADDR, DISABLE);
	MK_ST(cdt[NAND_QUAD_READ_GET_FEATURE], params->oip, 1, STA_ADDR0, 1, ENABLE, DISABLE, TM_STD_SPI);
	MK_CMD(cdt[NAND_QUAD_READ_FROM_CACHE], params->quad_r, 0, COL_ADDR, ENABLE);

	/* 8. nand standard write */
	MK_CMD(cdt[NAND_STANDARD_WRITE_ENABLE], params->w_en, 1, DEFAULT_ADDRMODE, DISABLE);
	MK_CMD(cdt[NAND_STANDARD_WRITE_TO_CACHE], params->standard_w_cache, 1, COL_ADDR, ENABLE);
	MK_CMD(cdt[NAND_STANDARD_WRITE_EXEC], params->w_exec, 1, ROW_ADDR, DISABLE);
	MK_ST(cdt[NAND_STANDARD_WRITE_GET_FEATURE], params->oip, 0, STA_ADDR0, 1, ENABLE, DISABLE, TM_STD_SPI);

	/* 9. nand quad write */
	MK_CMD(cdt[NAND_QUAD_WRITE_ENABLE], params->w_en, 1, DEFAULT_ADDRMODE, DISABLE);
	MK_CMD(cdt[NAND_QUAD_WRITE_TO_CACHE], params->quad_w_cache, 1, COL_ADDR, ENABLE);
	MK_CMD(cdt[NAND_QUAD_WRITE_EXEC], params->w_exec, 1, ROW_ADDR, DISABLE);
	MK_ST(cdt[NAND_QUAD_WRITE_GET_FEATURE], params->oip, 0, STA_ADDR0, 1, ENABLE, DISABLE, TM_STD_SPI);

	/* 10. block erase */
	MK_CMD(cdt[NAND_ERASE_WRITE_ENABLE], params->w_en, 1, DEFAULT_ADDRMODE, DISABLE);
	MK_CMD(cdt[NAND_BLOCK_ERASE], params->b_erase, 1, ROW_ADDR, DISABLE);
	MK_ST(cdt[NAND_ERASE_GET_FEATURE], params->oip, 0, STA_ADDR0, 1, ENABLE, DISABLE, TM_STD_SPI);

	/* 11. ecc status read */
	MK_CMD(cdt[NAND_ECC_STATUS_READ], params->ecc_r, 0, DEFAULT_ADDRMODE, ENABLE);

}

static void nand_create_cdt_table(struct sfc_flash *flash, uint32_t flag)
{
	struct ingenic_sfcnand_flashinfo *nand_info = flash->flash_info;
	cdt_params_t *cdt_params;
	struct sfc_cdt sfc_cdt[INDEX_MAX_NUM];

	memset(sfc_cdt, 0, sizeof(sfc_cdt));
	if(flag == DEFAULT_CDT)
	{
		/* 1. reset */
		sfc_cdt[NAND_RESET].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
		sfc_cdt[NAND_RESET].xfer = CMD_XFER(0, DISABLE, 0, DISABLE, SPINAND_CMD_RESET);
		sfc_cdt[NAND_RESET].staExp = 0;
		sfc_cdt[NAND_RESET].staMsk = 0;

		/* 2. try id */
		sfc_cdt[NAND_TRY_ID].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
		sfc_cdt[NAND_TRY_ID].xfer = CMD_XFER(0, DISABLE, 0, ENABLE, SPINAND_CMD_RDID);
		sfc_cdt[NAND_TRY_ID].staExp = 0;
		sfc_cdt[NAND_TRY_ID].staMsk = 0;

		/* 3. try id with dummy */
		/*
		 * There are some NAND flash, try ID operation requires 8-bit dummy value to be all 0,
		 * so use 1 byte address instead of dummy here.
		 */
		sfc_cdt[NAND_TRY_ID_DMY].link = CMD_LINK(0, ROW_ADDR, TM_STD_SPI);
		sfc_cdt[NAND_TRY_ID_DMY].xfer = CMD_XFER(1, DISABLE, 0, ENABLE, SPINAND_CMD_RDID);
		sfc_cdt[NAND_TRY_ID_DMY].staExp = 0;
		sfc_cdt[NAND_TRY_ID_DMY].staMsk = 0;

		/* 4. set feature */
		sfc_cdt[NAND_SET_FEATURE].link = CMD_LINK(0, STA_ADDR0, TM_STD_SPI);
		sfc_cdt[NAND_SET_FEATURE].xfer = CMD_XFER(1, DISABLE, 0, ENABLE, SPINAND_CMD_SET_FEATURE);
		sfc_cdt[NAND_SET_FEATURE].staExp = 0;
		sfc_cdt[NAND_SET_FEATURE].staMsk = 0;

		/* 5. get feature */
		sfc_cdt[NAND_GET_FEATURE].link = CMD_LINK(0, STA_ADDR0, TM_STD_SPI);
		sfc_cdt[NAND_GET_FEATURE].xfer = CMD_XFER(1, DISABLE, 0, ENABLE, SPINAND_CMD_GET_FEATURE);
		sfc_cdt[NAND_GET_FEATURE].staExp = 0;
		sfc_cdt[NAND_GET_FEATURE].staMsk = 0;

		/* first create cdt table */
		write_cdt(flash->sfc, sfc_cdt, NAND_RESET, NAND_GET_FEATURE);
	}

	if(flag == UPDATE_CDT){
		cdt_params = nand_info->cdt_params;
		params_to_cdt(cdt_params, sfc_cdt);

		/* second create cdt table */
		write_cdt(flash->sfc, sfc_cdt, NAND_STANDARD_READ_TO_CACHE, NAND_ECC_STATUS_READ);
	}
	//dump_cdt(flash->sfc);
}

static int request_sfc_desc(struct sfc_flash *flash)
{
	struct sfc *sfc = flash->sfc;
	sfc->desc = (struct sfc_desc *)dma_alloc_coherent(flash->dev, sizeof(struct sfc_desc) * DESC_MAX_NUM, &sfc->desc_pyaddr, GFP_KERNEL);
	if(flash->sfc->desc == NULL){
		return -ENOMEM;
	}
	sfc->desc_max_num = DESC_MAX_NUM;

	return 0;
}

int ingenic_sfc_nand_probe(struct sfc_flash *flash)
{
	const char *ingenic_probe_types[] = {"cmdlinepart",NULL};
	struct ingenic_sfc_info *board_info = NULL;
	struct nand_chip *chip;
	struct ingenic_sfcnand_flashinfo *nand_info;
	int32_t ret;
	int chip_param = 1;
	char mtdname[16];

	chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	if (IS_ERR_OR_NULL(chip)) {
		kfree(flash);
		return -ENOMEM;
	}
	nand_info = kzalloc(sizeof(struct ingenic_sfcnand_flashinfo), GFP_KERNEL);
	if(IS_ERR_OR_NULL(nand_info)) {
		kfree(flash);
	    	kfree(chip);
		return -ENOMEM;
	}

	flash->flash_info = nand_info;
#define THOLD	5
#define TSETUP	5
#define TSHSL_R	    20
#define TSHSL_W	    50

	set_flash_timing(flash->sfc, THOLD, TSETUP, TSHSL_R, TSHSL_W);

	/* request DMA Descriptor space */
	ret = request_sfc_desc(flash);
	if(ret){
		dev_err(flash->dev, "Failure to request DMA descriptor space!\n");
		ret = -ENOMEM;
		dma_free_coherent(flash->dev, sizeof(struct sfc_desc) * DESC_MAX_NUM, flash->sfc->desc, flash->sfc->desc_pyaddr);
		goto free_base;
	}

	/* Try creating default CDT table */
	flash->create_cdt_table = nand_create_cdt_table;
	flash->create_cdt_table(flash, DEFAULT_CDT);

	if((ret = ingenic_sfc_nand_dev_init(flash))) {
		dev_err(flash->dev, "nand device init failed!\n");
		goto free_base;
	}

	if((ret = ingenic_sfc_nand_try_id(flash))) {
		dev_err(flash->dev, "try device id failed\n");
		goto free_base;
	}

	/* Update to private CDT table */
	flash->create_cdt_table(flash, UPDATE_CDT);

	set_flash_timing(flash->sfc, nand_info->param.tHOLD,
			nand_info->param.tSETUP, nand_info->param.tSHSL_R, nand_info->param.tSHSL_W);
	sprintf(mtdname, "sfc%d_nand", flash->id);
	flash->mtd.name = mtdname;
	flash->mtd.sfc_index= flash->id;
	flash->mtd.owner = THIS_MODULE;
	flash->mtd.type = MTD_NANDFLASH;
	flash->mtd.flags |= MTD_CAP_NANDFLASH;
	flash->mtd.erasesize = nand_info->param.blocksize;
	flash->mtd.writesize = nand_info->param.pagesize;
	flash->mtd.size = nand_info->param.flashsize;
	flash->mtd.oobsize = nand_info->param.oobsize;
	flash->mtd.writebufsize = flash->mtd.writesize;
	flash->mtd.bitflip_threshold = flash->mtd.ecc_strength = nand_info->param.ecc_max - 1;

	chip->select_chip = NULL;
	chip->badblockbits = 8;
	chip->scan_bbt = nand_default_bbt;
	chip->block_bad = ingenic_sfcnand_block_bad_check;
	chip->block_markbad = ingenic_sfcnand_chip_block_markbad;
	//chip->ecc.layout= &gd5f_ecc_layout_128; // for erase ops
	chip->bbt_erase_shift = chip->phys_erase_shift = ffs(flash->mtd.erasesize) - 1;
	chip->buffers = kmalloc(sizeof(*chip->buffers), GFP_KERNEL);
	if(IS_ERR_OR_NULL(chip->buffers)) {
		dev_err(flash->dev, "alloc nand buffer failed\n");
		ret = -ENOMEM;
		goto free_base;
	}

	/* Set the bad block position */
	if (flash->mtd.writesize > 512 || (chip->options & NAND_BUSWIDTH_16))
		chip->badblockpos = NAND_LARGE_BADBLOCK_POS;
	else
		chip->badblockpos = NAND_SMALL_BADBLOCK_POS;

	flash->mtd.priv = chip;
	flash->mtd._erase = ingenic_sfcnand_erase;
	flash->mtd._read = ingenic_sfcnand_read;
	flash->mtd._write = ingenic_sfcnand_write;
	flash->mtd._read_oob = ingenic_sfcnand_read_oob;
	flash->mtd._write_oob = ingenic_sfcnand_write_oob;
	flash->mtd._block_isbad = ingenic_sfcnand_block_isbab;
	flash->mtd._block_markbad = ingenic_sfcnand_block_markbad;
	if((ret = chip->scan_bbt(&flash->mtd))) {
		dev_err(flash->dev, "creat and scan bbt failed\n");
		goto free_all;
	}

	if((ret = ingenic_sfcnand_partition(flash, board_info))) {
		if(ret == -EINVAL)
		{
		    chip_param = 0;
			dev_info(flash->dev, "read mtdparts!\n");
		}else{
			dev_err(flash->dev, "read flash partition failed!\n");
			goto free_all;
		}
	}else{
		flash->mtd.name = "chip_param";
	}

	/*	dump_flash_info(flash);*/
	if(chip_param)
		ret = mtd_device_parse_register(&flash->mtd, ingenic_probe_types, NULL, nand_info->partition.partition, nand_info->partition.num_partition);
	else
		ret = mtd_device_parse_register(&flash->mtd, NULL, NULL, NULL, 0);
	if (ret) {
		kfree(nand_info->partition.partition);
		if(!board_info->use_board_info) {
			kfree(burn_param->partition);
			kfree(burn_param);
		}
		ret = -ENODEV;
		goto free_all;
	}

	dev_info(flash->dev,"SPI NAND MTD LOAD OK\n");
	return 0;

free_all:
	kfree(chip->buffers->databuf);
	kfree(chip->buffers);

free_base:
	kfree(flash);
	kfree(chip);
	kfree(nand_info);
	return ret;
}

int ingenic_sfc_nand_suspend(struct sfc_flash *flash)
{
	return 0;
}

int ingenic_sfc_nand_resume(struct sfc_flash *flash)
{
	struct ingenic_sfcnand_flashinfo *nand_info;
	int32_t ret;

	nand_info = flash->flash_info;

	set_flash_timing(flash->sfc, THOLD, TSETUP, TSHSL_R, TSHSL_W);
	flash->create_cdt_table(flash, DEFAULT_CDT);

	if((ret = ingenic_sfc_nand_dev_init(flash))) {
		dev_err(flash->dev, "nand device init failed!\n");
		goto free_base;
	}
	flash->create_cdt_table(flash, UPDATE_CDT);

	set_flash_timing(flash->sfc, nand_info->param.tHOLD,
			nand_info->param.tSETUP, nand_info->param.tSHSL_R, nand_info->param.tSHSL_W);

	return 0;

free_base:
	kfree(flash);
	kfree(nand_info);
	return ret;
}
