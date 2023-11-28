/*
 * X1000 SoC SPI_NAND driver
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.

 *  You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * */
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/dmaengine.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/spi/flash.h>
#include <linux/delay.h>

#include <linux/spi/spi.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include "jz_spi_nand.h"

struct jz_spi_nandflash {
	struct spi_device   *spi;
	struct mutex        lock;
	struct mtd_info     mtd;
	struct mtd_partition *jz_mtd_partition;

	u32 tRD;	/* Read From Array */
	u32 tPROG;	/* Page Programming Time */
	u32 tBERS;	/* Block Erase Time */
	unsigned short column_cmdaddr_bits;/* read from cache ,the bits of cmd + addr */
	u8 read_column_addr_order;
	u8 write_column_addr_order;
};
static	u_char *command_read;
static	u_char *command_write;
int y = 1;
int e = 1;
//#define DEBUG_WRITE

struct spi_device_id jz_id_table[] = {
	{
		.name = "jz_spi_nand",
	},
};

#define SIZE_BOOTLOADER  (0x100000 * 16)
#define SIZE_KERNEL 0x800000
#define SIZE_ROOTFS 0x800000
struct mtd_partition jz_mtd_spinand_partition[] = {
	{
//		.name =     "bootloader",
		.name =     "data0",
		.offset =   0,
		.size =     SIZE_BOOTLOADER,
	},
	{
//		.name =     "kernel",
		.name =     "data1",
		.offset =   SIZE_BOOTLOADER,
		.size =     SIZE_KERNEL,
	},
	{
//		.name =     "recovery",
		.name =     "data2",
		.offset =   SIZE_KERNEL + SIZE_BOOTLOADER,
		.size =     SIZE_ROOTFS,
	},
};

static struct jz_spi_support jz_spi_nand_support_table[] = {
	{
		.id_manufactory = 0xc2,
		.id_device = 0x12,
		.name = "MX35LF1GE4AB",
		.page_size = 2 * 1024,
		.oobsize = 64,
		.block_size = 128 * 1024,
		.size = 128 * 1024 * 1024,
		.column_cmdaddr_bits = 24,
		.tRD_maxbusy = 25,
		.tPROG_maxbusy = 600,
		.tBERS_maxbusy = 3500,
		.read_column_addr_order = COLUMN_ADDR_ORDER_0_FIRST,
	},
	{
		.id_manufactory = 0xc2,
		.id_device = 0x22,
		.name = "MX35LF2GE4AB",
		.page_size = 2 * 1024,
		.oobsize = 64,
		.block_size = 128 * 1024,
		.size = 128 * 1024 * 2048,
		.column_cmdaddr_bits = 24,
		.tRD_maxbusy = 25,
		.tPROG_maxbusy = 600,
		.tBERS_maxbusy = 3500,
		.read_column_addr_order = COLUMN_ADDR_ORDER_0_FIRST,
	},
	{
		.id_manufactory = 0xc8,
		.id_device = 0xd1,
		.name = "GD5F1GQ4UBYIG",
		.page_size = 2 * 1024,
		.oobsize = 128,
		.block_size = 128 * 1024,
		.size = 128 * 1024 * 2048,
		.column_cmdaddr_bits = 24,
		.tRD_maxbusy = 80,
		.tPROG_maxbusy = 700,
		.tBERS_maxbusy = 5000,
		.read_column_addr_order = COLUMN_ADDR_ORDER_0_LAST,
	},
};

struct jz_spi_nand_platform_data jz_spi_nand_data = {
	.jz_spi_support = jz_spi_nand_support_table,
	.num_spi_flash  = ARRAY_SIZE(jz_spi_nand_support_table),
	.mtd_partition  = jz_mtd_spinand_partition,
	.num_partitions = ARRAY_SIZE(jz_mtd_spinand_partition),
};

struct spi_board_info jz_spi1_board_info[1] = {
	[0] ={
		.modalias       =  "jz_spi_nand",
		.platform_data  = &jz_spi_nand_data,
		.controller_data        = 0,
		.max_speed_hz           = 25000000,
		.bus_num                = 1,
		.chip_select            = 0,
	},
};

static inline int jz_spi_write(struct spi_device *spi, const void *buf, size_t len)
{
	struct spi_transfer t = {
		.tx_buf     = buf,
		.len        = len,
		.cs_change  = 1,
	};
	struct spi_message  m;
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(spi, &m);
}

#if 0
#define DEBUG_WRITE
#endif
#ifdef DEBUG_WRITE
static void dump_data(u_char *buf,int len)
{
	int i;
	for(i = 0; i < len; i++){
		pr_info("%02x\n",buf[i]);
	}
}

static size_t jz_spi_nandflash_read_ops(struct jz_spi_nandflash *flash,u_char *buffer,int page, int column,size_t rlen,size_t *rel_rlen);
static	u_char *write_debug_buf;
static int check_write_data(struct jz_spi_nandflash *flash ,u_char *write_buf,int page,int column,size_t len)
{
	size_t rel_rlen;
	memset(write_debug_buf,0,len);
	jz_spi_nandflash_read_ops(flash,write_debug_buf,page,column,len,&rel_rlen);
	if(memcmp(write_buf,write_debug_buf,len)){
		pr_info("write data check has ocour error ,please check:\n");
		pr_info("page %d column %d len %d the write buf is:\n",page,column,len);
		dump_data(write_buf,len);
		pr_info("page %d column %d len %d the read buf is:\n",page,column,len);
		dump_data(write_debug_buf,len);
		return 0;
	}else{
		pr_info("write and read right!\n");
		pr_info("page %d column %d len %d the write buf is:\n",page,column,len);
		dump_data(write_buf,len);
		pr_info("page %d column %d len %d the read buf is:\n",page,column,len);
		dump_data(write_debug_buf,len);
		return 0;
	}
	return 0;
}
#endif
static int jz_spi_nandflash_write_enable(struct jz_spi_nandflash *flash)
{

	int ret;
	unsigned char command[2];

	command[0] = SPINAND_CMD_WREN;
	ret = jz_spi_write(flash->spi, command, 1);
	if (ret)
		return ret;

	return 0;
}
static int jz_spi_nandflash_get_status(struct jz_spi_nandflash *flash,unsigned int *status)
{
	int ret;
	unsigned char command[2];
	struct spi_message message;
	struct spi_transfer transfer[2];

	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));

	command[0] = SPINAND_CMD_GET_FEATURE;
	command[1] = SPINAND_FEATURE_ADDR;

	transfer[0].tx_buf = command;
	transfer[0].len = sizeof(command);
	spi_message_add_tail(&transfer[0], &message);

	transfer[1].rx_buf = command;
	transfer[1].len = sizeof(command);
	transfer[1].cs_change = 1;
	spi_message_add_tail(&transfer[1], &message);

	ret = spi_sync(flash->spi, &message);
	if (ret){
		pr_info("spi sync message error ! %s %s %d \n",__FILE__,__func__,__LINE__);
		return ret;
	}
	*status=command[0];
	return ret;
}
static int jz_spi_nandflash_erase_blk(struct jz_spi_nandflash *flash,uint32_t addr)
{
	int ret;
	unsigned char command[4];
	int block = addr / flash->mtd.writesize;
	int timeout = 2000;
	int status;
	if(e)
	{
		printk("erase in!\n");
		e=0;
	}
	ret = jz_spi_nandflash_write_enable(flash);
	if(ret)
		return ret;

	switch(flash->mtd.erasesize){
		case SPINAND_OP_BL_128K:
			command[0] = SPINAND_CMD_ERASE_128K;
			break;
		default:
			pr_info("Don't support the blksize to erase ! \n");
			break;
	}

	command[1] = (block >> 16) & 0xff;
	command[2] = (block >> 8) & 0xff;
	command[3] = block & 0xff;
	ret = jz_spi_write(flash->spi, command, 4);
	if (ret){
		pr_info("spi write error ! %s %s %d \n",__FILE__,__func__,__LINE__);
		return ret;
	}
	msleep((flash->tBERS + 999) / 1000);
	do{
		ret = jz_spi_nandflash_get_status(flash,&status);
		if(ret)return ret;
		timeout--;
	}while((status & SPINAND_IS_BUSY) && (timeout > 0));
	if(timeout<=0)return -ETIMEDOUT;
	if(status & E_FALI){
		pr_info("Erase error,get state error ! %s %s %d \n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return 0;
}

static int jz_spi_nandflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{

	int ret;
	uint32_t addr, end;
	int check_addr;
	struct jz_spi_nandflash *flash;

	flash = container_of(mtd, struct jz_spi_nandflash, mtd);

	check_addr = ((unsigned int)instr->addr) % (mtd->erasesize);
	if (check_addr) {
		pr_info("%s line %d eraseaddr no align\n", __func__,__LINE__);
		ret= -EINVAL;
		goto nandflash_erase;
	}

	addr = (uint32_t)instr->addr;
	end = addr + (uint32_t)instr->len;
	mutex_lock(&flash->lock);
	while (addr < end) {
		ret = jz_spi_nandflash_erase_blk(flash, addr);
		if (ret) {
			pr_info("spi nand erase error blk id  %d !\n",addr / mtd->erasesize);
			instr->state = MTD_ERASE_FAILED;
			goto nandflash_erase;
		}

		addr += mtd->erasesize;
	}
	instr->state = MTD_ERASE_DONE;
	ret=0;
nandflash_erase:
	mutex_unlock(&flash->lock);
	mtd_erase_callback(instr);

	return ret;
}
static size_t jz_spi_nandflash_read_ops(struct jz_spi_nandflash *flash,u_char *buffer,int page, int column,size_t rlen,size_t *rel_rlen)
{
	struct spi_message message;
	unsigned char command[4];
	struct spi_transfer transfer[2];
	int ret,timeout = 2000;
	unsigned int status;
//	printk("page=%p\n",page);
	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));

	command[0] = SPINAND_CMD_PARD;
	command[1] = (page >> 16) & 0xff;
	command[2] = (page >> 8) & 0xff;
	command[3] = page & 0xff;

	transfer[0].tx_buf = command;
	transfer[0].len = 4;
	transfer[0].cs_change = 1;
	spi_message_add_tail(&transfer[0], &message);

	ret = spi_sync(flash->spi, &message);
	if(ret) {
		pr_info("spi_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return ret;
	}
	udelay(flash->tRD);
	do{
		ret = jz_spi_nandflash_get_status(flash,&status);
		if(ret)return ret;
		timeout--;
	}while((status & SPINAND_IS_BUSY) && (timeout > 0));
	if(timeout<=0)return -ETIMEDOUT;
	status=(status>>4)&0x0f;
	if(status == 2) {
		pr_info("spi nand read error page %d column = %d ret = %02x !!! %s %s %d \n",page,column,ret,__FILE__,__func__,__LINE__);
		return -EBADMSG;
	}
	if(status ==3)status--;
	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	if(flash->read_column_addr_order == COLUMN_ADDR_ORDER_0_FIRST){
		switch(flash->column_cmdaddr_bits){
			case 24:
				command[0] = SPINAND_CMD_RDCH;
				command[1] = 0;
				command[2] = (column >> 8) & 0xff;
				command[3] = column & 0xff;

				transfer[0].tx_buf = command;
				transfer[0].len = 4;
				break;
			case 32:
				command[0] = SPINAND_CMD_FRCH;//SPINAND_CMD_RDCH;/* SPINAND_CMD_RDCH read odd addr may be error */
				command[1] = 0;
				command[2] = (column >> 8) & 0xff;
				command[3] = column & 0xff;

				transfer[0].tx_buf = command;
				transfer[0].len = 4;
				break;
			default:
				pr_info("can't support the format of column addr ops !!\n");
				break;
		}
	}else{
		switch(flash->column_cmdaddr_bits){
			case 24:
				command[0] = SPINAND_CMD_RDCH;
				command[1] = (column >> 8) & 0xff;
				command[2] = column & 0xff;
				command[3] = 0;

				transfer[0].tx_buf = command;
				transfer[0].len = 4;
				break;
			case 32:
				command[0] = SPINAND_CMD_FRCH;//SPINAND_CMD_RDCH;/* SPINAND_CMD_RDCH read odd addr may be error */
				command[1] = (column >> 8) & 0xff;
				command[2] = column & 0xff;
				command[3] = 0;

				transfer[0].tx_buf = command;
				transfer[0].len = 4;
				break;
			default:
				pr_info("can't support the format of column addr ops !!\n");
				break;
		}
	}
	spi_message_add_tail(&transfer[0], &message);

	transfer[1].rx_buf = command_read;
	transfer[1].len = rlen;
	transfer[1].cs_change = 1;
	spi_message_add_tail(&transfer[1], &message);

	ret = spi_sync(flash->spi, &message);
#if 0
	int i;
	for(i=0;i<5;i++){
	printk("read=%02x\n",command_read[i]);
	}
#endif
	memcpy(buffer,command_read,rlen);
	if(ret) {
		pr_info("%s -- %s --%d  spi_sync() error !\n",__FILE__,__func__,__LINE__);
		return ret;
	}
	ret=status;
	*rel_rlen = message.actual_length - transfer[0].len;
	return ret;
}
static int jz_spi_nandflash_read(struct mtd_info *mtd,loff_t addr,int column,size_t len,u_char *buf,size_t *retlen)
{
	int ret,read_num,i,rlen,page;
	int page_size = mtd->writesize;
	struct jz_spi_nandflash *flash;
	u_char *buffer = buf;
	size_t page_overlength;
	size_t ops_addr;
	size_t ops_len,rel_rlen = 0;

	flash = container_of(mtd, struct jz_spi_nandflash, mtd);

	mutex_lock(&flash->lock);

	if(column){
		ops_addr = (unsigned int)addr;
		ops_len = len;

		if(len <= (page_size - column))
			page_overlength = len;
		else
			page_overlength = page_size - column;/*random read but len over a page */

		while(ops_addr < addr + len){
			page = ops_addr / page_size;
			if(page_overlength){
				ret = jz_spi_nandflash_read_ops(flash,buffer,page,column,page_overlength,&rel_rlen);
				if(ret<0)
					goto nandflash_read_exit;
				ops_len -= page_overlength;
				buffer += page_overlength;
				ops_addr += page_overlength;
				page_overlength = 0;
			}else{
				column = 0;
				if(ops_len >= page_size)
					rlen = page_size;
				else
					rlen = ops_len;

				ret = jz_spi_nandflash_read_ops(flash,buffer,page,column,rlen,&rel_rlen);
				if(ret<0)
					goto nandflash_read_exit;
				buffer += rlen;
				ops_len -= rlen;
				ops_addr += rlen;
			}
			*retlen += rel_rlen;
		}
	}else{
		read_num = (len + page_size - 1) / page_size;
		page = ((unsigned int)addr) / mtd->writesize;

		for(i = 0; i < read_num; i++){
			if(len >= page_size)
				rlen = page_size;
			else
				rlen = len;

			ret = jz_spi_nandflash_read_ops(flash,buffer,page,column,rlen,&rel_rlen);
			if(ret<0)
				goto nandflash_read_exit;

			buffer += rlen;
			len -= rlen;
			page++;
			*retlen += rel_rlen;
		}
	}
nandflash_read_exit:
	mutex_unlock(&flash->lock);
	return ret;
}

static size_t jz_spi_nandflash_write(struct mtd_info *mtd,loff_t addr,int column,size_t len, const u_char *buf,int *retlen)
{
	int write_num,page_size,i,ret;
	struct spi_message message;
	struct spi_transfer transfer[1];
	struct jz_spi_nandflash *flash;
	int page = ((unsigned int )addr) / mtd->writesize;
	const u_char *buffer = buf;
	unsigned int status;
	int timeout = 2000;
	int wlen = 0;
	*retlen = 0;

	if(y)
	{
		printk("write in!\n");
		y=0;
	}

	if(addr & (mtd->writesize - 1)){
		pr_info("wirte add don't align ,error ! %s %s %d \n",__FILE__,__func__,__LINE__);
		ret=-EINVAL;
		goto nandflash_write_exit;
	}

	flash = container_of(mtd, struct jz_spi_nandflash, mtd);

	mutex_lock(&flash->lock);
	page_size = mtd->writesize;
	write_num = (len + page_size - 1) / (page_size);

	for(i = 0; i < write_num; i++){

		if(len >= page_size)
			wlen = page_size;
		else
			wlen = len;

		jz_spi_nandflash_write_enable(flash);

		spi_message_init(&message);
		memset(&transfer, 0, sizeof(transfer));

		command_write[0] = SPINAND_CMD_PRO_LOAD;
		command_write[1] = (column >> 8) & 0xff;
		command_write[2] = column & 0xff;
		memcpy(command_write + 3, buffer,wlen);
		transfer[0].tx_buf = command_write;
		transfer[0].len = wlen + 3;
		transfer[0].cs_change = 1;
		spi_message_add_tail(&transfer[0], &message);

		ret = spi_sync(flash->spi, &message);
		if(ret) {
			pr_info("spi_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
			ret=-EIO;
			goto  nandflash_write_exit;
		}
		*retlen += message.actual_length - 3;/* delete the len of command */
		//jz_spi_nandflash_write_enable(flash);

		spi_message_init(&message);
		memset(&transfer, 0, sizeof(transfer));
		command_write[0] = SPINAND_CMD_PRO_EN;
		command_write[1] = (page >> 16) & 0xff;
		command_write[2] = (page >> 8) & 0xff;
		command_write[3] = page & 0xff;

		transfer[0].tx_buf = command_write;
		transfer[0].len = 4;
		transfer[0].cs_change = 1;
		spi_message_add_tail(&transfer[0], &message);

		ret = spi_sync(flash->spi, &message);
		if(ret) {
			pr_info("spi_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
			goto nandflash_write_exit;
		}
		udelay(flash->tPROG);
		do{
			ret = jz_spi_nandflash_get_status(flash,&status);
			if(ret<0)goto nandflash_write_exit;
			timeout--;
		}while((status & SPINAND_IS_BUSY) && (timeout > 0));
		if(status & p_FAIL){
			pr_info("spi nand write fail %s %s %d\n",__FILE__,__func__,__LINE__);
			ret=-EBADMSG;
			goto nandflash_write_exit;
		}
#ifdef DEBUG_WRITE
		check_write_data(flash,buffer,page,column,wlen);
#endif
		len -= wlen;
		buffer += wlen;
		page++;
	}
	ret=0;
nandflash_write_exit:
	mutex_unlock(&flash->lock);
	return ret;
}
static int jz_spinand_read(struct mtd_info *mtd, loff_t from,size_t len, size_t *retlen, unsigned char *buf)
{
	int ret;
	size_t column = ((unsigned int)from) % mtd->writesize;
	ret = jz_spi_nandflash_read(mtd,from,column,len,buf,retlen);

	return ret;
}

static int jz_spinand_write(struct mtd_info *mtd, loff_t to, size_t len,size_t *retlen, const u_char *buf)
{
	size_t ret;
	size_t column = ((unsigned int)to) % mtd->writesize;
	ret = jz_spi_nandflash_write(mtd,to,column,len,buf,retlen);
	return ret;
}
static int jz_spinand_read_oob(struct mtd_info *mtd,loff_t addr,struct mtd_oob_ops *ops)
{
	struct jz_spi_nandflash *flash;
	int column = mtd->writesize;
	int page = (unsigned int)addr / mtd->writesize;
	struct spi_message message;
	unsigned char command[4];
	unsigned int status;
	struct spi_transfer transfer[2];
	int ret,timeout = 2000;

	flash = container_of(mtd, struct jz_spi_nandflash, mtd);
	mutex_lock(&flash->lock);
	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));

	command[0] = SPINAND_CMD_PARD;
	command[1] = (page >> 16) & 0xff;
	command[2] = (page >> 8) & 0xff;
	command[3] = page & 0xff;

	transfer[0].tx_buf = command;
	transfer[0].len = 4;
	transfer[0].cs_change = 1;
	spi_message_add_tail(&transfer[0], &message);

	ret = spi_sync(flash->spi, &message);
	if(ret) {
		pr_info("spi_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		goto spinand_read_oob;
	}
	udelay(flash->tRD);

	do{
		ret = jz_spi_nandflash_get_status(flash,&status);
		if(ret<0)goto spinand_read_oob;
		timeout--;
	}while((status & SPINAND_IS_BUSY) && (timeout > 0));
	if(timeout<=0){
		ret=-ETIMEDOUT;
		goto spinand_read_oob;
	}
	if((status & 0x30) == 0x20) {
		pr_info("spi nand read error page %d column = %d ret = %02x !!! %s %s %d \n",page,column,ret,__FILE__,__func__,__LINE__);
		memset(ops->oobbuf,0x0,ops->ooblen);
		ret= -EBADMSG;
		goto spinand_read_oob;
	}
	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	switch(flash->column_cmdaddr_bits){
		case 24:
			command[0] = SPINAND_CMD_RDCH;
			command[1] = (column >> 8) & 0xff;
			command[2] = column & 0xff;
			command[3] = 0;

			transfer[0].tx_buf = command;
			transfer[0].len = 4;
			break;
		case 32:
			command[0] = SPINAND_CMD_FRCH;//SPINAND_CMD_RDCH;/* SPINAND_CMD_RDCH read odd addr may be error */
			command[1] = 0;
			command[2] = (column >> 8) & 0xff;
			command[3] = column & 0xff;

			transfer[0].tx_buf = command;
			transfer[0].len = 5;
			break;
		default:
			pr_info("can't support the format of column addr ops !!\n");
			break;
	}
	spi_message_add_tail(&transfer[0], &message);

	transfer[1].rx_buf = ops->oobbuf;
	transfer[1].len = ops->ooblen;
	transfer[1].cs_change = 1;
	spi_message_add_tail(&transfer[1], &message);

	ret = spi_sync(flash->spi, &message);
	if(ret) {
		pr_info("spi_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
		goto spinand_read_oob;
	}
spinand_read_oob:
	mutex_unlock(&flash->lock);
	return ret;
}
static int jz_spinand_write_oob(struct mtd_info *mtd,loff_t addr,struct mtd_oob_ops *ops)
{
	struct spi_message message;
	struct spi_transfer transfer[1];
	struct jz_spi_nandflash *flash;
	int ret;
	unsigned int status;
	int column = mtd->writesize;
	int page = ((unsigned int)addr) / mtd->writesize;

	flash = container_of(mtd, struct jz_spi_nandflash, mtd);
	printk("writeoob in!\n");
	mutex_lock(&flash->lock);

	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	jz_spi_nandflash_write_enable(flash);
	command_write[0] = SPINAND_CMD_PARD;
	command_write[1] = (page >> 16) & 0xff;
	command_write[2] = (page >> 8) & 0xff;
	command_write[3] = page & 0xff;

	transfer[0].tx_buf = command_write;
	transfer[0].len = 4;
	transfer[0].cs_change = 1;
	spi_message_add_tail(&transfer[0], &message);
	ret = spi_sync(flash->spi, &message);
	if(ret) {
		pr_info("spi_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		goto spinand_write_oob;
	}
	udelay(flash->tRD);
	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));

	command_write[0] = SPINAND_CMD_PLRd;
	command_write[1] = (column >> 8) & 0xff;
	command_write[2] = column & 0xff;

	memcpy(command_write + 3,ops->oobbuf,ops->ooblen);

	transfer[0].tx_buf = command_write;
	transfer[0].len = ops->ooblen + 3;
	transfer[0].cs_change = 1;
	spi_message_add_tail(&transfer[0], &message);
	ret = spi_sync(flash->spi, &message);
	if(ret) {
		pr_info("spi_sync() error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		goto spinand_write_oob;
	}

	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	command_write[0] = SPINAND_CMD_PRO_EN;
	command_write[1] = (page >> 16) & 0xff;
	command_write[2] = (page >> 8) & 0xff;
	command_write[3] = page & 0xff;

	transfer[0].tx_buf = command_write;
	transfer[0].len = 4;
	transfer[0].cs_change = 1;
	spi_message_add_tail(&transfer[0], &message);

	ret = spi_sync(flash->spi, &message);
	if(ret) {
		pr_info("spi_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		goto spinand_write_oob;
	}
	udelay(flash->tPROG);
	ret = jz_spi_nandflash_get_status(flash,&status);
	if(ret<0)
		goto spinand_write_oob;
	if(status & p_FAIL){
		pr_info(" spi nand write fail %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
		goto spinand_write_oob;
	}
spinand_write_oob:
	mutex_unlock(&flash->lock);
	return 0;
}
static int jz_spi_nandflash_block_checkbad(struct mtd_info *mtd, loff_t ofs,int getchip,int allowbbt)
{
	struct nand_chip *chip = mtd->priv;
	if (!chip->bbt)
		return chip->block_bad(mtd, ofs,getchip);

	/* Return info from the table */
	return nand_isbad_bbt(mtd, ofs, allowbbt);

}
static int jz_spinand_block_isbab(struct mtd_info *mtd,loff_t ofs)
{
	int ret;
	ret = jz_spi_nandflash_block_checkbad(mtd, ofs,1, 0);
	return ret;
}

static int jz_spinand_block_markbad(struct mtd_info *mtd,loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	int ret;
	pr_info("jz_spinand_block_markbad\n");
	ret = jz_spinand_block_isbab(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}
	return chip->block_markbad(mtd, ofs);
}
static int jz_spi_nandflash_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	uint8_t buf[2] = { 0, 0 };
	int  res, ret = 0, i = 0;
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
			res = jz_spinand_write_oob(mtd, wr_ofs, &ops);
			if (!ret)
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

static int badblk_check(int len,unsigned char *buf)
{
	int i,bit0_cnt = 0;
	unsigned short *check_buf = (unsigned short *)buf;

	if(check_buf[0] != 0xff){
		for(i = 0; i < len * 8; i++){
			if(!((check_buf[0] >> 1) & 0x1))
				bit0_cnt++;
		}
	}
	if(bit0_cnt > 6 * len)
		return 1; // is bad blk
	return 0;
}
static int jz_spinand_block_bad_check(struct mtd_info *mtd, loff_t ofs,int getchip)
{
	int check_len = 2;
	unsigned char check_buf[] = {0xaa,0xaa};
	struct mtd_oob_ops ops;

	ops.oobbuf = check_buf;
	ops.ooblen = check_len;
	jz_spinand_read_oob(mtd,ofs,&ops);
	if(badblk_check(check_len,check_buf))
		return 1;
	return 0;
}
static int jz_spinand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int addr,ret;
	addr = instr->addr;

	ret = jz_spi_nandflash_erase(mtd,instr);
	if(ret){
		pr_info("WARNING: block %d erase fail !\n",addr / mtd->erasesize);

		ret = jz_spinand_block_markbad(mtd,addr);
		if(ret){
			pr_info("mark bad block error, there will occur error,so exit !\n");
			return ret;
		}
	}
	instr->state = MTD_ERASE_DONE;
	return 0;

}
struct jz_spi_support *jz_spi_flash_probe(struct spi_device *spi)
{
	int ret,i;
	unsigned char send_command[2], recv_command[4];
	struct spi_message message;
	struct spi_transfer transfer[2];
	struct jz_spi_nand_platform_data *pdata = spi->dev.platform_data;
	struct jz_spi_support *jz_spi_nand_support_table = pdata->jz_spi_support;
	struct jz_spi_support *params;
	int num_spi_flash = pdata->num_spi_flash;

	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));

	send_command[0] = SPINAND_CMD_RDID;
	send_command[1] = 0;

	transfer[0].tx_buf = send_command;
	transfer[0].len = sizeof(send_command);
	spi_message_add_tail(&transfer[0], &message);

	transfer[1].rx_buf = recv_command;
	transfer[1].len = sizeof(recv_command);
	transfer[1].cs_change = 1;
	spi_message_add_tail(&transfer[1], &message);

	ret = spi_sync(spi, &message);
	if (ret) {
		pr_info("error reading spi nand device id\n");
		return NULL;
	}
	printk("id_manufactory=%02x,id_device=%02x\n",recv_command[0],recv_command[1]);
	for (i = 0; i < num_spi_flash; i++) {
		params = &jz_spi_nand_support_table[i];
		if (( (params->id_manufactory) == recv_command[0] ) && ( (params->id_device)== recv_command[1] )){
			printk("the flash name is %s\n",params->name);
			break;
		}
	}

	if (i >= num_spi_flash) {
		pr_info("ingenic: Unsupported ID %04x\n", recv_command[0]);
		return NULL;
	}
	return params;
}

static int jz_spi_nand_ext_init(struct spi_device *spi)
{
	struct spi_message message;
	struct spi_transfer transfer[1];
	unsigned char command[4];
	int ret;

	/* enable ecc */
	spi_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));

	command[0] = 0x1f;
	command[1] = 0xa0;
	command[2] = 0x00;

	transfer[0].tx_buf = command;
	transfer[0].len = 3;
	transfer[0].cs_change = 1;
	spi_message_add_tail(&transfer[0], &message);
	ret = spi_sync(spi, &message);
	if(ret){
		pr_info("WARNING: Spi_Nand enable ECC fail !\n");
		return -1;
	}
	return 0;
}
static void convert_burner_to_driver_use(struct jz_spi_support *spinand,struct jz_spi_support_from_burner *burner,int param_num)
{
        int i=0;
        for(i=0;i<param_num;i++){
                spinand[i].id_manufactory=(burner[i].chip_id>>8)&0xff;
                spinand[i].id_device=(burner[i].chip_id)&0x000000ff;
                memcpy(spinand[i].name,burner[i].name,SIZEOF_NAME);
                spinand[i].page_size=burner[i].page_size;
                spinand[i].oobsize=burner[i].oobsize;
                spinand[i].sector_size=burner[i].sector_size;
                spinand[i].block_size=burner[i].block_size;
                spinand[i].size=burner[i].size;
                spinand[i].page_num=burner[i].page_num;
                spinand[i].tRD_maxbusy=burner[i].tRD_maxbusy;
                spinand[i].tPROG_maxbusy=burner[i].tPROG_maxbusy;
                spinand[i].tBERS_maxbusy=burner[i].tBERS_maxbusy;
                spinand[i].column_cmdaddr_bits=burner[i].column_cmdaddr_bits;
        }
}
static int transfer_to_mtddriver_struct(struct get_chip_param *param,struct jz_spi_nand_platform_data **change)
{
        int i=0;
		*change=kzalloc(sizeof(struct jz_spi_nand_platform_data),GFP_KERNEL);
        if(!*change)
                return -ENOMEM;
        (*change)->num_spi_flash=param->para_num;
        (*change)->jz_spi_support=kzalloc(param->para_num*sizeof(struct jz_spi_support),GFP_KERNEL);
        if(!(*change)->jz_spi_support)
                return -ENOMEM;
        memcpy((*change)->jz_spi_support,param->addr,param->para_num*sizeof(struct jz_spi_support));
	convert_burner_to_driver_use((*change)->jz_spi_support,param->addr,param->para_num);

        (*change)->num_partitions=param->partition_num;
        (*change)->mtd_partition=kzalloc(param->partition_num*sizeof(struct mtd_partition),GFP_KERNEL);
        if(!(*change)->mtd_partition){
                                return -ENOMEM;
        }
        for(i=0;i<(*change)->num_partitions;i++)
        {
                (*change)->mtd_partition[i].name=kzalloc(32*sizeof(char),GFP_KERNEL);
                if(!(*change)->mtd_partition[i].name)
                        return -ENOMEM;
                memcpy((*change)->mtd_partition[i].name,param->partition[i].name,32);
                (*change)->mtd_partition[i].size=param->partition[i].size;
                (*change)->mtd_partition[i].offset=param->partition[i].offset;
                (*change)->mtd_partition[i].mask_flags=param->partition[i].mask_flags;

        }
        return 0;
}
static int get_pagesize_from_nand(struct jz_spi_support *spi_flash)
{
	int page_size=0;
    page_size = spi_flash -> page_size;
	return page_size;
}
static int jz_get_spinand_param(struct jz_spi_nand_platform_data **param,struct jz_spi_nandflash *flash,int32_t *nand_magic,struct jz_spi_support *spi_flash)
{
	int rlen;
	int page_size;
	struct get_chip_param param_from_burner;
	char *buffer=NULL;
	char *member_addr;
	int i=0;
	*nand_magic=0;
	for(i=0;i<2;i++){
		flash->column_cmdaddr_bits=24;
		if(i==1)
			flash->column_cmdaddr_bits=32;
		*nand_magic=0;
		page_size=get_pagesize_from_nand(spi_flash);
		if(page_size>0&&page_size<4000)
		{
			buffer=kzalloc(page_size,GFP_KERNEL);
		}else
			continue;
		if(!buffer)
			return -ENOMEM;
		jz_spi_nandflash_read_ops(flash,buffer,SPIFLASH_PARAMER_OFFSET/page_size,SPIFLASH_PARAMER_OFFSET%page_size,
				page_size,&rlen);
		*nand_magic=*(int32_t *)(buffer);
		//printk("nand_magic=0x%x",*nand_magic);
		if(*nand_magic!=0x6e616e64){
			kfree(buffer);
			if(i==1){
				printk("page_size=%d\n",page_size);
				return 0;
			}
		}else
			break;
	}
	member_addr=buffer+sizeof(int32_t);
	param_from_burner.version=*(int *)member_addr;
	member_addr+=sizeof(param_from_burner.version);
	param_from_burner.flash_type=*(int *)member_addr;
	member_addr+=sizeof(param_from_burner.flash_type);
        param_from_burner.para_num=*(int *)member_addr;
	member_addr+=sizeof(param_from_burner.para_num);
        param_from_burner.addr=(struct jz_spi_support_from_burner *)member_addr;
	member_addr+=param_from_burner.para_num*sizeof(struct jz_spi_support_from_burner);
        param_from_burner.partition_num=*((int *)member_addr);
	member_addr+=sizeof(param_from_burner.partition_num);
        param_from_burner.partition=(struct jz_spinand_partition *)member_addr;
        transfer_to_mtddriver_struct(&param_from_burner,param);
	kfree(buffer);
        return 0;
}

static int jz_spi_nandflash_probe(struct spi_device *spi)
{
	struct jz_spi_nand_platform_data *pdata = spi->dev.platform_data;
	struct nand_chip *chip;
	struct jz_spi_nandflash *spi_nandflash;
	struct jz_spi_support *spi_flash;
	int num_partitions,ret;
	int32_t nand_magic;
	struct mtd_partition *mtd_spinand_partition;

	mtd_spinand_partition = pdata->mtd_partition;
	num_partitions = pdata->num_partitions;


    printk("%s[%d]: \n",__func__,__LINE__);
	chip = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
	if(!chip)
		return -ENOMEM;
	spi_nandflash = kzalloc(sizeof(struct jz_spi_nandflash),GFP_KERNEL);
	if(!spi_nandflash)
		return -ENOMEM;

	spi_nandflash->spi = spi;
	mutex_init(&spi_nandflash->lock);
	dev_set_drvdata(&spi->dev, spi_nandflash);
	spi_flash = jz_spi_flash_probe(spi);

	command_read = kzalloc(spi_flash->page_size,GFP_KERNEL);
	if(!command_read){
		pr_info("mlloc command_read error !!!!\n");
		return -1;
	}

	jz_get_spinand_param(&pdata,spi_nandflash,&nand_magic,spi_flash);
	spi->dev.platform_data=pdata;
	mtd_spinand_partition=pdata->mtd_partition;
	num_partitions=pdata->num_partitions;


	spi_nandflash->mtd.name = dev_name(&spi->dev);
	spi_nandflash->mtd.owner = THIS_MODULE;
	spi_nandflash->mtd.type	= MTD_NANDFLASH;
	spi_nandflash->mtd.flags |= MTD_CAP_NANDFLASH;
	spi_nandflash->mtd.erasesize = spi_flash->block_size;
	spi_nandflash->mtd.writesize = spi_flash->page_size;
	spi_nandflash->mtd.size = spi_flash->size;
	spi_nandflash->mtd.oobsize = spi_flash->oobsize;
	spi_nandflash->mtd.writebufsize = spi_nandflash->mtd.writesize;

	spi_nandflash->column_cmdaddr_bits = spi_flash->column_cmdaddr_bits;
	spi_nandflash->read_column_addr_order = spi_flash->read_column_addr_order;
	spi_nandflash->write_column_addr_order = spi_flash->write_column_addr_order;
	spi_nandflash->tRD = spi_flash->tRD_maxbusy;
	spi_nandflash->tPROG = spi_flash->tPROG_maxbusy;
	spi_nandflash->tBERS = spi_flash->tBERS_maxbusy;

	spi_nandflash->mtd.bitflip_threshold = spi_nandflash->mtd.ecc_strength = 2;
	chip->select_chip = NULL;
	chip->badblockbits = 8;
	chip->scan_bbt = nand_default_bbt;
	chip->block_bad = jz_spinand_block_bad_check;
	chip->block_markbad = jz_spi_nandflash_block_markbad;
	//chip->ecc.layout= &gd5f_ecc_layout_128; // for erase ops
	chip->bbt_erase_shift = chip->phys_erase_shift = ffs(spi_nandflash->mtd.erasesize) - 1;
	if (!(chip->options & NAND_OWN_BUFFERS))
		chip->buffers = kmalloc(sizeof(*chip->buffers), GFP_KERNEL);

	/* Set the bad block position */
	if (spi_nandflash->mtd.writesize > 512 || (chip->options & NAND_BUSWIDTH_16))
		chip->badblockpos = NAND_LARGE_BADBLOCK_POS;
	else
		chip->badblockpos = NAND_SMALL_BADBLOCK_POS;

	spi_nandflash->mtd.priv = chip;

	spi_nandflash->mtd._erase = jz_spinand_erase;
	spi_nandflash->mtd._read = jz_spinand_read;
	spi_nandflash->mtd._write = jz_spinand_write;
	spi_nandflash->mtd._read_oob = jz_spinand_read_oob;
	spi_nandflash->mtd._write_oob = jz_spinand_write_oob;
	spi_nandflash->mtd._block_isbad = jz_spinand_block_isbab;
	spi_nandflash->mtd._block_markbad = jz_spinand_block_markbad;

	command_write = kzalloc(spi_nandflash->mtd.writesize + 3,GFP_KERNEL);
	if(!command_write){
		pr_info("mlloc command_write error !!!!\n");
		return -1;
	}
#ifdef DEBUG_WRITE
	write_debug_buf = kzalloc(spi_nandflash->mtd.writesize,GFP_KERNEL);
	if(!write_debug_buf){
		pr_info("mlloc write_debug_buf error !!!!\n");
		return -1;
	}
#endif
	jz_spi_nand_ext_init(spi);
	chip->scan_bbt(&spi_nandflash->mtd);
	ret = mtd_device_parse_register(&spi_nandflash->mtd,NULL,NULL, mtd_spinand_partition, num_partitions);
	if (ret) {
		kfree(spi_nandflash);
		dev_set_drvdata(&spi->dev, NULL);
		return -ENODEV;
	}
	pr_info("SPI Nandflash MTD LOAD OK\n");
    printk("%s[%d]: \n",__func__,__LINE__);
	return 0;
}

static int jz_spi_nandflash_remove(struct spi_device *spi)
{
	int ret;
	struct jz_spi_nandflash *flash;

	flash = dev_get_drvdata(&spi->dev);
	if(!flash)
		return 0;

	ret = mtd_device_unregister(&flash->mtd);
	if (!ret) {
		kfree(flash);
		dev_set_drvdata(&spi->dev, NULL);
	}
	return ret;
}
static struct spi_driver jz_spi_nand_driver = {
	.driver = {
		.name   = "jz_spi_nand",
		.owner  = THIS_MODULE,
	},
	.id_table   = jz_id_table,
	.probe      = jz_spi_nandflash_probe,
	.remove     = jz_spi_nandflash_remove,
	.shutdown   = NULL, //forever start
};

static int __init jz_spi_nand_init(void)
{
    printk("%s[%d]: \n",__func__,__LINE__);
	spi_register_board_info(jz_spi1_board_info, ARRAY_SIZE(jz_spi1_board_info));
	return spi_register_driver(&jz_spi_nand_driver);
}

static void __exit jz_spi_nand_exit(void)
{
	spi_unregister_driver(&jz_spi_nand_driver);
}

module_init(jz_spi_nand_init);
module_exit(jz_spi_nand_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MTD SPI_NAND driver for Ingenic SoC");
MODULE_ALIAS("platform:"DRVNAME);

