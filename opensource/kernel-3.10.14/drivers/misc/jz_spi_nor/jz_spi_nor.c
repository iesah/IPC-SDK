/*
 * jz47xx_spi_nor.c - Jz47xx SPI NOR Flash driver
 *
 */
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

#include "jz_spi_nor.h"

/**
 * jz_nor_wel_setup - Enable/Disable SPI NOR Write Enable Latch
 * @flash:	device parameter, buffers for send command.
 * @sel_en:	1, enable WEL once; 0, disable WEL.
 *
 * This must be called prior to every Page Program(PP), Write
 * Status Register(WRSR), Sector Erase(SE), Block Erase(BE), Chip
 * Erase(CE).
 */
static int jz_nor_wel_setup(struct jz_nor_local *flash, int sel_en)
{
	struct spi_message msg;
	struct spi_transfer *x = flash->xfer;

	spi_message_init(&msg);
	flash->start.cmd = sel_en ? CMD_WREN : CMD_WRDI;

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, 1);
	spi_message_add_tail(x, &msg);

	return spi_sync(flash->spi, &msg);
}

/**
 * jz_nor_read_status - Read SPI NOR status register
 * @flash:	device with which data will be exchanged.
 * @regnum:	the number of status register.
 *
 * This can get 8bit status register value. different register has
 * different command.
 *
 * Return status register value read success, else 0 read failed.
 */
static u8 jz_nor_read_status(struct jz_nor_local *flash, int regnum)
{
	struct spi_message msg;
	struct spi_transfer *x = flash->xfer;
	u8 cmd_rdsr[3] = {0x05, 0x35, 0x15}; /* reg0, reg1, reg2 */
	u8 status_reg;
	int err;

	if (regnum > flash->statreg_num - 1) {
		dev_err(&flash->spi->dev, "read status regnum is invalid.");
		return 0x00;
	}

	spi_message_init(&msg);

	/* ----> CMD_RDSR ----> REGn.S7~REGn.S0 */
	flash->start.cmd = cmd_rdsr[regnum];

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, 1);
	spi_message_add_tail(x, &msg);

	x++;
	spi_node_read_setup(x, flash->spi, &status_reg, 1);
	spi_message_add_tail(x, &msg);

	err = spi_sync(flash->spi, &msg);
	if (err < 0) {
		dev_err(&flash->spi->dev, "read status spi sync error: %d\n", err);
		status_reg = 0;
	}

	return status_reg;
}

/**
 * jz_nor_wait_busy - Wait SPI NOR Write In Process finished
 * @flash:		jz_nor_read_status needed.
 * @max_busytime:	Max Busy times, unit: ms.
 *
 * SPI NOR WIP bit only be set 1, while nor WRSR, PP, CE, SE and BE.
 */
static int jz_nor_wait_busy(struct jz_nor_local *flash, u32 max_busytime)
{
	int this_cpu = smp_processor_id();
	u64 t_start = cpu_clock(this_cpu); /* Nano seconds get */
	u64 m_sec = 0;
	while ((jz_nor_read_status(flash, 0) & STATUS_WIP)) {
		m_sec = cpu_clock(this_cpu) - t_start;
		do_div(m_sec, 1000000);
		if ((u32)m_sec >= max_busytime) {
			dev_dbg(&flash->spi->dev, "WIP wait busy timeout, max_busytime: %d ms\n", max_busytime);
			break;
		}
	}
	return (u32)m_sec < max_busytime ? 0 : -EIO;
}

/**
 * jz_nor_read_bytes - Read datas from NOR flash used SPI bus
 * @flash:	device parameter, buffers for send command.
 * @offset:	data start address at NOR flash.
 * @buf:	buffer store readed datas.
 * @length:	read data length.
 *
 * This read SPI NOR data begin offset, length Bytes to buf serially.
 * NOT be limited by page or sector.
 */
static size_t jz_nor_read_bytes(struct jz_nor_local *flash,
				size_t offset,
				u8 *buf,
				size_t length)
{
	struct spi_message msg;
	struct spi_transfer *x = flash->xfer;
	size_t retlen = 0;
	int ret;

	spi_message_init(&msg);

	flash->start.cmd = CMD_READ;
	serial_flash_address(&flash->start, offset, flash->addrsize);

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, flash->addrsize + 1);
	spi_message_add_tail(x, &msg);

	x++;
	spi_node_read_setup(x, flash->spi, buf, length);
	spi_message_add_tail(x, &msg);

	retlen = length;
	ret = spi_sync(flash->spi, &msg);
	if (ret < 0) {
		dev_dbg(&flash->spi->dev, "%s, spi bus read failed: %d\n", __FUNCTION__, ret);
		retlen = 0;
	}

	return retlen;
}

/**
 * jz_nor_read - Read NOR flash datas
 * @flash:	jz_nor_read_bytes needed.
 * @offset:	data start address at NOR flash.
 * @buf:	buffer store readed datas.
 * @length:	read data length.
 *
 * This function just check the conditions of jz_nor_read_bytes and
 * return read status.
 */
static int jz_nor_read(struct jz_nor_local *flash,
		       size_t offset,
		       u8 *buf,
		       size_t length)
{
	struct spi_device *spi = flash->spi;
	size_t retlen;

	if (!buf || !length) {
		dev_err(&spi->dev, "Invalid buf or read length\n");
		return -EINVAL;
	}

	retlen = jz_nor_read_bytes(flash, offset, buf, length);
	if (retlen < length) {
		dev_err(&spi->dev, "spi nor read failed\n");
		return -EIO;
	}

	return 0;
}

/**
 * jz_nor_write_page - Write one page to NOR flash
 * @flash:	device parameter, buffers for send command.
 * @offset:	start address of data in NOR flash.
 */
static size_t jz_nor_write_page(struct jz_nor_local *flash,
				size_t offset,
				const u8 *buf,
				size_t pagelen)
{
	struct spi_message msg;
	struct spi_transfer *x = flash->xfer;
	size_t len, retlen;
	int ret;

	if (!pagelen || pagelen > flash->pagesize)
		len = flash->pagesize;
	else
		len = pagelen;

	if (offset % flash->pagesize + len > flash->pagesize)
		len -= offset % flash->pagesize + len - flash->pagesize;

	spi_message_init(&msg);

	flash->start.cmd = CMD_PP;
	serial_flash_address(&flash->start, offset, flash->addrsize);

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, flash->addrsize + 1);
	spi_message_add_tail(x, &msg);

	x++;
	spi_node_write_setup(x, flash->spi, buf, len);
	spi_message_add_tail(x, &msg);

	retlen = len;
	dev_dbg(&flash->spi->dev, "%s, spi bus write len: %d\n", __FUNCTION__, retlen);
	ret = spi_sync(flash->spi, &msg);
	if (ret < 0) {
		dev_dbg(&flash->spi->dev, "%s, spi bus write failed: %d\n", __FUNCTION__, ret);
		retlen = 0;
		goto err_out;
	}

	ret = jz_nor_wait_busy(flash, flash->pp_maxbusy);
	if (ret < 0) {
		dev_dbg(&flash->spi->dev, "%s, write wait busy timeout.\n", __FUNCTION__);
		retlen = 0;
	}

err_out:
	return retlen;
}

/**
 * jz_nor_write -
 */
static int jz_nor_write(struct jz_nor_local *flash,
			   size_t offset,
			   const u8 *buf,
			   size_t length)
{
	struct spi_device *spi = flash->spi;
	size_t pagelen, retlen;

	if (!buf || !length) {
		dev_err(&spi->dev, "Invalid buf or program length\n");
		return -EINVAL;
	}

	if (offset + length > flash->chipsize) {
		dev_err(&spi->dev, "Data write overflow this chip\n");
		return -EINVAL;
	}

	while (length) {
		if (length >= flash->pagesize)
			pagelen = 0;
		else
			pagelen = length % flash->pagesize;

		jz_nor_wel_setup(flash, 1);
		retlen = jz_nor_write_page(flash, offset, buf, pagelen);
		if (!retlen) {
			dev_err(&spi->dev, "spi nor write failed\n");
			return -EIO;
		}

		offset += retlen;
		buf += retlen;
		length -= retlen;
	}

	return 0;
}

/**
 * _jz_nor_sector_erase -
 */
static int _jz_nor_sector_erase(struct jz_nor_local *flash, size_t offset)
{
	struct spi_message msg;
	struct spi_transfer *x = flash->xfer;
	int err;

	spi_message_init(&msg);

	flash->start.cmd = CMD_SE;
	serial_flash_address(&flash->start, offset, flash->addrsize);

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, flash->addrsize + 1);
	spi_message_add_tail(x, &msg);

	err = spi_sync(flash->spi, &msg);
	jz_nor_wait_busy(flash, flash->se_maxbusy);

	return err;
}

/**
 * _jz_nor_block_erase -
 */
static int _jz_nor_block_erase(struct jz_nor_local *flash,
			       size_t offset,
			       int block_mode)
{
	struct spi_message msg;
	struct spi_transfer * x = flash->xfer;
	int err;

	spi_message_init(&msg);

	flash->start.cmd = flash->block_info[block_mode].cmd_blockerase;
	serial_flash_address(&flash->start, offset, flash->addrsize);

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, flash->addrsize + 1);
	spi_message_add_tail(x, &msg);

	err = spi_sync(flash->spi, &msg);
	jz_nor_wait_busy(flash, flash->block_info[block_mode].be_maxbusy);

	return err;
}

/**
 8 _jz_nor_chip_erase -
 */
static int _jz_nor_chip_erase(struct jz_nor_local *flash)
{
	struct spi_message msg;
	struct spi_transfer *x = flash->xfer;
	int err;

	spi_message_init(&msg);

	flash->start.cmd = CMD_CE;

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, 1);
	spi_message_add_tail(x, &msg);

	err = spi_sync(flash->spi, &msg);
	jz_nor_wait_busy(flash, flash->ce_maxbusy);

	return err;
}

/**
 * jz_nor_erase -
 */
static int jz_nor_erase(struct jz_nor_local *flash,
			size_t offset,
			size_t len)
{
	struct spi_device *spi = flash->spi;
	int erase_block, erase_size;
	int i, flag;

	if (!len || !flash->sectorsize) {
		dev_err(&spi->dev, "serial nor erase length is 0 or this chip NOT support erase.\n");
		return -EINVAL;
	}

	if (offset % flash->sectorsize) {
		dev_dbg(&spi->dev, "serial nor erase address NOT aligned, use the aligned address.\n");
		offset = offset / flash->sectorsize * flash->sectorsize;
	}

	if (offset == 0 && len >= flash->chipsize) {
		jz_nor_wel_setup(flash, 1);
		_jz_nor_chip_erase(flash);
		return 0;
	}

	if (offset + len > flash->chipsize) {
		dev_err(&spi->dev, "erase overflow this chip\n");
		return -EINVAL;
	}

	erase_size = 0;
	while (erase_size < len) {
		int blocksize;
		erase_block = 0;
		flag = 0;

		for (i = 0; !flag && i < flash->num_block_info; i++) {
			blocksize = flash->block_info[i].blocksize;
			if (!(offset % blocksize) && (len - erase_size) >= blocksize) {
				erase_block = blocksize;
				jz_nor_wel_setup(flash, 1);
				_jz_nor_block_erase(flash, offset, i);
				flag = 1;
			}
		}

		if (!flag) {
			if (!(offset % flash->sectorsize)) {
				erase_block = flash->sectorsize;
				jz_nor_wel_setup(flash, 1);
				_jz_nor_sector_erase(flash, offset);
				flag = 1;
			}
		}

		offset += erase_block;
		erase_size += erase_block;
	}

	return 0;
}

#if 0
static int jz_nor_read_manufacture_id(struct jz_nor_local *flash, )
{
}
#endif

static int jz_nor_read_id(struct jz_nor_local *flash, u8 *id_buf)
{
	struct spi_message msg;
	struct spi_transfer *x = flash->xfer;

	spi_message_init(&msg);

	flash->start.cmd = CMD_RDID;

	spi_node_write_setup(x, flash->spi, (u8 *)&flash->start, 1);
	spi_message_add_tail(x, &msg);

	x++;
	spi_node_read_setup(x, flash->spi, id_buf, 3);
	spi_message_add_tail(x, &msg);

	return spi_sync(flash->spi, &msg);
}

static int jz_nor_probe(struct spi_device *spi)
{
	struct jz_nor_local *flash;
	struct spi_nor_platform_data *pdata = spi->dev.platform_data;
	u8 id_buf[4] = {0};
	int err;
	int ret;
	dev_info(&spi->dev, "max_speed_hz: %dkHz\n", spi->max_speed_hz / 1000);
	dev_info(&spi->dev, "bits_per_word: %dbit\n", spi->bits_per_word);

	err = spi_setup(spi);

	if (!pdata) {
		dev_err(&spi->dev, "no platform data.\n");
		return -ENODEV;
	}

	flash = kzalloc(sizeof(struct jz_nor_local), GFP_KERNEL);
	if (!flash) {
		dev_err(&spi->dev, "Get flash mem failed.\n");
		return -ENOMEM;
	}

	dev_set_drvdata(&spi->dev, flash);

	flash->spi		= spi;
	flash->pagesize		= pdata->pagesize;
	flash->sectorsize	= pdata->sectorsize;
	flash->block_info	= pdata->block_info;
	flash->num_block_info	= pdata->num_block_info;
	flash->chipsize		= pdata->chipsize;
	flash->addrsize		= pdata->addrsize;
	flash->statreg_num	= pdata->st_regnum;
	flash->pp_maxbusy	= pdata->pp_maxbusy;
	flash->se_maxbusy	= pdata->se_maxbusy;
	flash->ce_maxbusy	= pdata->ce_maxbusy;

	flash->read		= jz_nor_read;
	flash->write		= jz_nor_write;
	flash->erase		= jz_nor_erase;

	flash->get_status	= jz_nor_read_status;

	if (flash->addrsize > MAX_ADDR_SIZE) {
		dev_err(&spi->dev, "flash addrsize: %dBytes invalid.\n", flash->addrsize);
		err = -EINVAL;
		goto err_out;
	}

	//mutex_init(&flash->lock);

	ret = jz_nor_read_id(flash, id_buf);
	dev_info(&spi->dev, "spi-nor id: 0x%08x, status: %d\n", *((u8 *)id_buf), ret);
	*((u32 *)id_buf) = 0;
	ret = jz_nor_read_id(flash, id_buf);
	dev_info(&spi->dev, "spi-nor id: 0x%08x, status: %d\n", *((u8 *)id_buf), ret);

#if 0
	flash->start.cmd = 0x00;
	serial_flash_address(&flash->start, 0x44332211, 1);
	{
		int i;
		dev_info(&spi->dev, "flash->start:\t");
		for (i = 0; i < 5; i++)
			printk("0x%02x ", ((u8 *)&flash->start)[i]);
		printk("\n");
	}
#endif

	return 0;

err_out:
	kfree(flash);
	return err;
}

static int jz_nor_remove(struct spi_device *spi)
{
	struct jz_nor_local *flash = dev_get_drvdata(&spi->dev);
	kfree(flash);

	return 0;
}

static struct spi_driver jz_nor_driver = {
	.driver = {
		.name	= JZNOR_DEVICE_NAME,
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
		//.pm	= NULL,
	},

	.probe		= jz_nor_probe,
	.remove		= jz_nor_remove,
};

static int __init jz_nor_init(void)
{
	return spi_register_driver(&jz_nor_driver);
}
late_initcall(jz_nor_init);

static void __exit jz_nor_exit(void)
{
	spi_unregister_driver(&jz_nor_driver);
}
module_exit(jz_nor_exit);

MODULE_DESCRIPTION("Jz47xx SPI NOR Flash Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:jz_nor");
