/*#define DEBUG*/
#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <spl.h>
#include <asm/arch/clk.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spi_nand.h>
#include <generated/sfc_timing_val.h>
#include <generated/sfc_nand_params.h>

#define SPINAND_PARAM_SIZE	1024

/*#define  CONFIG_SPI_STANDARD*/
static struct spl_nand_param *curr_device;

static inline void sfc_writel(unsigned int value, unsigned short offset)
{
	writel(value, SFC_BASE + offset);
}

static inline unsigned int sfc_readl(unsigned short offset)
{
	return readl(SFC_BASE + offset);
}

static inline void sfc_transfer_direction(unsigned int value)
{
	unsigned int val;

	val = sfc_readl(SFC_GLB);
	val &= ~(1 << GLB_TRAN_DIR_OFFSET);
	val |= value << GLB_TRAN_DIR_OFFSET;
	sfc_writel(val, SFC_GLB);
}

static inline void sfc_set_length(unsigned int value)
{
	sfc_writel(value, SFC_TRAN_LEN);
}

static inline void sfc_dev_addr(unsigned int channel, unsigned int value)
{
	sfc_writel(value, SFC_DEV_ADDR(channel));
}

static inline void sfc_dev_addr_plus(unsigned int channel, unsigned int value)
{
	sfc_writel(value, SFC_DEV_ADDR_PLUS(channel));
}

static inline void set_flash_timing(void)
{
	sfc_writel(DEF_TIM_VAL, SFC_DEV_CONF);
}

static inline void sfc_tranconf_init(struct jz_sfc *sfc, unsigned int channel)
{
	sfc_writel(sfc->tranconf.d32, SFC_TRAN_CONF(channel));
}

static void sfc_set_transfer(struct jz_sfc *sfc, unsigned int dir)
{
	sfc_transfer_direction(dir);
	sfc_tranconf_init(sfc, 0);
	sfc_set_length(sfc->len);
	sfc_dev_addr(0, sfc->addr);
}
static void clear_end(void)
{
	while(!(sfc_readl(SFC_SR) & END));
        sfc_writel(CLR_END, SFC_SCR);

}

static void sfc_tranconf1_init(unsigned int tran_mode)
{
	unsigned int tmp;
	tmp = sfc_readl(SFC_TRAN_CONF1(0));
	tmp &= ~(TRAN_CONF1_TRAN_MODE_MSK);
	tmp |= (tran_mode << TRAN_CONF1_TRAN_MODE_OFFSET);
	sfc_writel(tmp, SFC_TRAN_CONF1(0));
}

static void sfc_send_cmd(struct jz_sfc *sfc, unsigned char dir)
{

	sfc_writel(1 << 1, SFC_TRIG);
	sfc_set_transfer(sfc, dir);
	sfc_writel(1 << 2, SFC_TRIG);
	sfc_writel(TRIG_START, SFC_TRIG);

}

static void sfc_write_data(unsigned int *data, unsigned int length)
{
	while (!(sfc_readl(SFC_SR) & TRAN_REQ));
	sfc_writel(CLR_TREQ,SFC_SCR);
	sfc_writel(*data, SFC_RM_DR);
	clear_end();
}

static int sfc_read_data(unsigned int *data, unsigned int length)
{
	unsigned int tmp_len = 0;
	unsigned int fifo_num = 0;
	unsigned int reg_tmp = 0;
	unsigned int len = (length + 3) / 4;
	int i;

	while(1){
		reg_tmp = sfc_readl(SFC_SR);
		if (reg_tmp & RECE_REQ) {
			sfc_writel(CLR_RREQ, SFC_SCR);
			if ((len - tmp_len) > THRESHOLD)
				fifo_num = THRESHOLD;
			else
				fifo_num = len - tmp_len;

			for (i = 0; i < fifo_num; i++) {
				*data++ = sfc_readl(SFC_RM_DR);
				tmp_len++;
			}
		}
		if (tmp_len == len)
			break;
	}

	clear_end();

	return 0;
}

static void sfc_controler_init(void)
{
	unsigned int tmp;
#ifndef CONFIG_FPGA
	clk_set_rate(SFC, CONFIG_SFC_NAND_RATE);
#endif
	tmp = sfc_readl(SFC_GLB);
	tmp &= ~(GLB_THRESHOLD_MSK);
	tmp |= (THRESHOLD << GLB_THRESHOLD_OFFSET);
	sfc_writel(tmp, SFC_GLB);

	set_flash_timing();
}

static int spinand_bad_block_check(int len, unsigned char *check_buf)
{
	int j;

	for(j = 0; j < len; j++)
		if(check_buf[j] != 0xff)
		return 1;
	return 0;
}

static int spinand_read_page(unsigned int page, unsigned int column, unsigned char *dst_addr,
			    unsigned int len, unsigned int pagesize)
{
	struct jz_sfc sfc;
	unsigned int read_buf = 0;
	int oob_flag = 0;
	unsigned char i;
	unsigned char checklen = 1;

read_oob:
	if (oob_flag) {
		column = pagesize;
		len = 4;
		dst_addr = (unsigned char *)&read_buf;
	}

	SFC_SEND_COMMAND(&sfc, CMD_PARD, 0, page, 3, 0, 0, 0);
	clear_end();
	do {
	SFC_SEND_COMMAND(&sfc, CMD_GET_FEATURE, 1, FEATURE_REG_STATUS1, 1, 0, 1, 0);
	read_buf = 0;
	sfc_read_data(&read_buf, 1);
	}while(read_buf & 0x1);
	/*ecc check*/
	for(i = 0; i < curr_device->eccstat_count; i++) {
		if(((read_buf >> curr_device->ecc_bit) &
		(~(0xff << curr_device->bit_counts))) == curr_device->eccerrstatus[i])
			return -1;
	}

	/*
	 * Parity block plane select
	 * MXIC  MX35LF2GE4AB    DID : 0x22
	 * XTX   XT26G02E        DID : 0x24
	 * ESMT  F50L2G41XA      DID : 0x24
	 */
	if(curr_device->device_id == 0x20 || curr_device->device_id == 0x22 || curr_device->device_id == 0x24)
		column |= (((page >> 6) & 1) << 12);
#ifndef CONFIG_SPI_STANDARD
	SFC_SEND_COMMAND(&sfc, CMD_FR_CACHE_QUAD, len, column, curr_device->addrlen, 8, 1, 0);
#else
	SFC_SEND_COMMAND(&sfc, CMD_FR_CACHE, len, column, curr_device->addrlen, 8, 1, 0);
#endif
	sfc_read_data((unsigned int *)dst_addr, len);

	if (!oob_flag && !(page % CONFIG_SPI_NAND_PPB)) {
		oob_flag = 1;
		goto read_oob;
	} else if (oob_flag) {
#if NAND_BUSWIDTH == NAND_BUSWIDTH_16
		checklen = 2;
#endif
		if (spinand_bad_block_check(checklen, (unsigned char *)&read_buf))
			return 1;
	}

	return 0;
}

static int probe_id_list(unsigned char *id)
{
	unsigned char i;

	for (i = 0; i < ARRAY_SIZE(nand_param); i++) {
		if (nand_param[i].id_manufactory == id[0] &&
			    nand_param[i].device_id == id[1]) {

			curr_device = &nand_param[i];
			break;
		}
	}

	if (i == ARRAY_SIZE(nand_param))
		return -ENODEV;

	return 0;
}

static int spinand_probe_id(struct jz_sfc *sfc)
{
	/*
	 * cmd-->addr-->pid
	 */
	unsigned char addrlen[] = {0, 1};
	unsigned char id[2] = {0};
	unsigned char i;

	for(i = 0; i < sizeof(addrlen); i++) {
		SFC_SEND_COMMAND(sfc, CMD_RDID, 2, 0, addrlen[i], 0, 1, 0);
		sfc_read_data((unsigned int *)id, 2);

		if (!probe_id_list(id))
			    break;
	}
	if(i == sizeof(addrlen)) {
		debug("ERR: don`t support this kind of nand device, \
			please add it\n");
		return -ENODEV;
	}
	return 0;
}

static int spinand_init(void)
{
	struct jz_sfc sfc;
	unsigned int x;
	unsigned int tmp = 0;
	unsigned int sfc_dcr = 0;

	/*
	 * Probe nand vid/pid
	 */
	while(spinand_probe_id(&sfc) < 0)
	{
		sfc_dcr = sfc_readl(SFC_DEV_CONF);
		sfc_dcr &= ~DEV_CONF_SMP_DELAY_MSK;
		tmp++;
		sfc_dcr |= tmp << DEV_CONF_SMP_DELAY_OFFSET;
		sfc_writel(sfc_dcr,SFC_DEV_CONF);
		if(tmp > 10){
			printf("ERR: don`t support this kind of nand device,please add it\n");
			break;
		}
	}

	/* disable write protect */
	x = 0;
	SFC_SEND_COMMAND(&sfc, CMD_SET_FEATURE, 1, FEATURE_REG_PROTECT, 1, 0, 1, 1);
	sfc_write_data(&x, 1);

	x = BITS_QUAD_EN | BITS_ECC_EN | BITS_BUF_EN;
	SFC_SEND_COMMAND(&sfc, CMD_SET_FEATURE, 1, FEATURE_REG_FEATURE1, 1, 0, 1, 1);
	sfc_write_data(&x, 1);

	return 0;
}

int sfc_nand_load(unsigned int src_addr, unsigned int count, unsigned int dst_addr)
{
	unsigned int pageaddr, columnaddr, rlen;
	int ret;
	unsigned char *buf = (unsigned char *)dst_addr;
	unsigned int pagesize = curr_device->pagesize;

	while (count) {
		pageaddr = src_addr / pagesize;
		columnaddr = src_addr % pagesize;
		rlen = (pagesize - columnaddr) < count ? (pagesize - columnaddr) : count;
		ret = spinand_read_page(pageaddr, columnaddr, buf, rlen, pagesize);
		if (ret > 0) {
			printf("bad block %d\n", pageaddr / CONFIG_SPI_NAND_PPB);
			src_addr += CONFIG_SPI_NAND_PPB * pagesize;
			continue;
		}

		buf += rlen;
		src_addr += rlen;
		count -= rlen;
	}

	return 0;
}


void nv_map_area(unsigned int *base_addr, unsigned int nv_addr, unsigned int nv_size)
{
	unsigned int buf[6][2];
	unsigned int nv_off = 0, nv_count = 0;
	unsigned int addr, i;
	unsigned int blocksize = 128 * 1024;
	unsigned int nv_num = nv_size / blocksize;

	memset(buf, 0, sizeof(buf));

	if(nv_num > 6) {
		while(1);
	}

	for(i = 0; i < nv_num; i++) {
		addr = nv_addr + i * blocksize;
		sfc_nand_load(addr, 4, (unsigned int)buf[i]);
		if(buf[i][0] == 0x5a5a5a5a) {
			addr += blocksize - 8;
			sfc_nand_load(addr, 8, (unsigned int)buf[i]);
			if(buf[i][1] == 0xa5a5a5a5) {
				if(nv_count < buf[i][0]) {
					nv_count = buf[i][0];
					nv_off = i;
				}
			}
		}
	}
	*base_addr = nv_addr + nv_off *	blocksize;
}


void spl_load_kernel(long offset)
{
	struct image_header *header;
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	sfc_nand_load(offset, sizeof(struct image_header), CONFIG_SYS_TEXT_BASE);
	spl_parse_image_header(header);
	sfc_nand_load(offset, spl_image.size, spl_image.load_addr);
}

void sfc_init(void)
{
	sfc_controler_init();
	spinand_init();
}

static void spl_sfc_nand_load(void)
{
	struct image_header *header;
#ifdef CONFIG_SPL_OS_BOOT
	struct jz_sfc_nand_burner_param *burn_param;
	struct jz_spinand_partition *partition;
	unsigned int bootimg_addr = 0;
	unsigned int i = 0;
#endif
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	sfc_init();

#ifdef CONFIG_SPL_OS_BOOT
	/*read burn param*/
	sfc_nand_load(CONFIG_SPIFLASH_PART_OFFSET, SPINAND_PARAM_SIZE, CONFIG_SYS_TEXT_BASE);
	burn_param = (void *)(CONFIG_SYS_TEXT_BASE);
	partition = (struct jz_spinand_partition *)&burn_param->partition;

	for(i = 0; i < burn_param->partition_num; i++) {
		if (!strncmp(partition[i].name, CONFIG_SPL_OS_NAME, sizeof(CONFIG_SPL_OS_NAME))) {
			bootimg_addr = partition[i].offset;
			break;
		}
	}

	/*read image head*/
	spl_load_kernel(bootimg_addr);
#else
	printf("%s:%d\n",__func__,__LINE__);
	sfc_nand_load(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_TEXT_BASE);
	printf("%x,%x,%x\n",CONFIG_UBOOT_OFFSET,CONFIG_SYS_MONITOR_LEN,CONFIG_SYS_TEXT_BASE);
	printf("%s:%d\n",__func__,__LINE__);
	spl_parse_image_header(header);
/*	{
		unsigned int *buf0 = (unsigned int *)(CONFIG_SYS_TEXT_BASE);
		unsigned int *buf1 = (unsigned int *)(CONFIG_SYS_TEXT_BASE + CONFIG_SYS_MONITOR_LEN - 16 - 0xf00);
	printf("%s:%d\n",__func__,__LINE__);
	printf("buf0: 0x%x 0x%x 0x%x 0x%x\n", buf0[0], buf0[1], buf0[2], buf0[3]);
	buf0 += 2048 / 4;
	printf("#buf0: 0x%x 0x%x 0x%x 0x%x\n", buf0[0], buf0[1], buf0[2], buf0[3]);
	buf0 += (2048 * 64) / 4;
	printf("##buf0: 0x%x 0x%x 0x%x 0x%x\n", buf0[0], buf0[1], buf0[2], buf0[3]);
	printf("#buf1: 0x%x 0x%x 0x%x 0x%x\n", buf1[0], buf1[1], buf1[2], buf1[3]);
	}*/
#endif
}

#if 0
static struct ota_ops ota_ops = {
	.flash_init = sfc_init,
	.flash_read = sfc_nand_load,
	/* .flash_get_partitions = sfc_nand_get_partition, */
};
#endif
void spl_sfc_nand_load_image(void)
{
#ifdef CONFIG_OTA_VERSION20
	register_ota_ops(&ota_ops);
	spl_ota_load_image();
#else
	spl_sfc_nand_load();
#endif
}
