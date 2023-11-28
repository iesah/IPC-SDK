#include <common.h>
#include <config.h>
#include <spl.h>
#include <spi.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spi.h>
#include <asm/arch/clk.h>
#include <linux/lzo.h>

//#define TEST

static uint32_t jz_sfc_readl(unsigned int offset)
{
#ifdef CONFIG_SFC0_NOR
	return readl(SFC0_BASE + offset);
#else
	return readl(SFC1_BASE + offset);
#endif
}

static void jz_sfc_writel(unsigned int value, unsigned int offset)
{
#ifdef CONFIG_SFC0_NOR
	writel(value, SFC0_BASE + offset);
#else
	writel(value, SFC1_BASE + offset);
#endif
}

void sfc_set_mode(int channel, int value)
{
	unsigned int tmp;
	/*T40 modify SFC_TRAN_CONF0->SFC_TRAN_CONF1*/
	tmp = jz_sfc_readl(SFC_TRAN_CONF1(channel));
	tmp &= ~(TRAN_CONF1_TRAN_MODE_MSK);
	tmp |= (value << TRAN_CONF1_TRAN_MODE_OFFSET);
	jz_sfc_writel(tmp,SFC_TRAN_CONF1(channel));
}


void sfc_dev_addr_dummy_bytes(int channel, unsigned int value)
{
	unsigned int tmp;
	tmp = jz_sfc_readl(SFC_TRAN_CONF(channel));
	tmp &= ~TRAN_CONF0_DMYBITS_MSK;
	tmp |= (value << TRAN_CONF0_DMYBITS_OFFSET);
	jz_sfc_writel(tmp,SFC_TRAN_CONF(channel));
}


static void sfc_set_read_reg(unsigned int cmd, unsigned int addr,
		unsigned int addr_plus, unsigned int addr_len, unsigned int data_en)
{
	unsigned int tmp;

	tmp = jz_sfc_readl(SFC_GLB);
	tmp &= ~GLB_PHASE_NUM_MSK;
	tmp |= (0x1 << GLB_PHASE_NUM_OFFSET);
	jz_sfc_writel(tmp, SFC_GLB);

	if (data_en) {
		tmp = (addr_len << TRAN_CONF0_ADDR_WIDTH_OFFSET) | TRAN_CONF0_CMD_EN |
			TRAN_CONF0_DATEEN | (cmd << TRAN_CONF0_CMD_OFFSET);
	} else {
		tmp = (addr_len << TRAN_CONF0_ADDR_WIDTH_OFFSET) | TRAN_CONF0_CMD_EN |
			(cmd << TRAN_CONF0_CMD_OFFSET);
	}

	jz_sfc_writel(tmp, SFC_TRAN_CONF(0));
	jz_sfc_writel(addr, SFC_DEV_ADDR(0));
	jz_sfc_writel(addr_plus, SFC_DEV_ADDR_PLUS(0));

	sfc_dev_addr_dummy_bytes(0, 0);
	sfc_set_mode(0, 0);

	jz_sfc_writel(TRIG_START, SFC_TRIG);
}

static int sfc_read_data(unsigned int *data, unsigned int len)
{
	unsigned int tmp_len = 0;
	unsigned int fifo_num = 0;
	unsigned int i;

	while (1) {
		if (jz_sfc_readl(SFC_SR) & RECE_REQ) {
			jz_sfc_writel(CLR_RREQ, SFC_SCR);
			if ((len - tmp_len) > THRESHOLD)
				fifo_num = THRESHOLD;
			else {
				fifo_num = len - tmp_len;
			}

			for (i = 0; i < fifo_num; i++) {
				*data = jz_sfc_readl(SFC_RM_DR);
				data++;
				tmp_len++;
			}
		}

		if (tmp_len == len)
			break;
	}

	while ((jz_sfc_readl(SFC_SR) & END) != END)
		printf("jz_sfc_readl(SFC_SR) : %x\n", jz_sfc_readl(SFC_SR));
	jz_sfc_writel(CLR_END,SFC_SCR);

	return 0;
}

static int sfc_read(unsigned int addr, unsigned int addr_plus,
		unsigned int addr_len, unsigned int *data, unsigned int len)
{
	unsigned int cmd, ret;

	jz_sfc_writel(TRIG_STOP, SFC_TRIG);
	jz_sfc_writel(TRIG_FLUSH, SFC_TRIG);

	cmd  = CMD_READ;

	jz_sfc_writel((len * 4), SFC_TRAN_LEN);

	sfc_set_read_reg(cmd, addr, addr_plus, addr_len, 1);

	ret = sfc_read_data(data, len);
	if (ret)
		return ret;
	else
		return 0;
}

void sfc_init(void)
{
	unsigned int tmp;
#ifndef CONFIG_FPGA
#ifdef CONFIG_SFC0_NOR
	clk_set_rate(SFC0, 50000000);
#endif
#ifdef CONFIG_SFC1_NOR
	clk_set_rate(SFC1, 50000000);
#endif
#endif
	tmp = jz_sfc_readl(SFC_GLB);
	tmp &= ~GLB_THRESHOLD_MSK;
	tmp |= THRESHOLD << GLB_THRESHOLD_OFFSET;
	jz_sfc_writel(tmp, SFC_GLB);

	tmp = DEV_CONF_CEDL | DEV_CONF_HOLDDL | DEV_CONF_WPDL;
	jz_sfc_writel(tmp, SFC_DEV_CONF);

	/* low power consumption */
	jz_sfc_writel(0, SFC_CGE);
}

static int sfc_nor_addrlen(void)
{
	unsigned int ret,retry_count=0;
	unsigned int addr_len, spi_sig[4];
	unsigned char *sig;

	sig = (unsigned char *)spi_sig;

	addr_len = 6;
retry:
	if(retry_count > 3)
		return -1;

	ret = sfc_read(0x0, 0x0, addr_len, spi_sig, 4);
	if (ret) {
		printf("sfc read head info error,ret = %d\n",ret);
	}

	addr_len = sig[0];
	if ((addr_len == 0x6) || (addr_len == 0x5) || (addr_len == 0x4) ||
			(addr_len == 0x3) || (addr_len == 0x2)) {
		if ((sig[addr_len - 1] == 0x55) && (sig[addr_len] == 0xaa) && (sig[addr_len + 1] == 0x55) && (sig[addr_len + 2] == 0xaa))
			return addr_len;
		else{
			retry_count++;
			goto retry;
		}
	} else {
		retry_count++;
		goto retry;
	}

	return -1;
}

void sfc_nor_load(unsigned int src_addr, unsigned int count, unsigned int dst_addr)
{
	unsigned int ret, addr_len, words_of_spl;

	/* spi norflash addr len */
	addr_len = sfc_nor_addrlen();
	if(addr_len < 0)
		printf("sfc get falsh addrlen error,addr_len=%d\n",addr_len);

	/* count word align */
	words_of_spl = (count + 3) / 4;

	ret = sfc_read(src_addr, 0x0, addr_len, (unsigned int *)(dst_addr), words_of_spl);
	if (ret) {
		printf("sfc read error\n");
	}

#ifdef TEST
	int i;
	for(i = 0; i < 50; i++)
		printf("0x%x : %x\n", (dst_addr + 4 * i), *(unsigned int *)(dst_addr + 4 * i));
#endif
	return ;
}
#ifdef CONFIG_SPL_LZOP
void spl_sfc_nor_load_image(void)
{
	size_t dst_len;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_UBOOT_OFFSET);

	spl_parse_image_header(header);

	sfc_nor_load(CONFIG_UBOOT_OFFSET, spl_image.size, CONFIG_DECMP_BUFFER_ADRS);

	lzop_decompress((unsigned char *)(CONFIG_DECMP_BUFFER_ADRS+0x40),spl_image.size-0x40,
			(unsigned char *)CONFIG_SYS_TEXT_BASE,&dst_len);

	return ;
}
#else
void spl_sfc_nor_load_image(void)
{
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);/*zm:there is no header here, using default*/

	sfc_init();

	spl_parse_image_header(header);

	sfc_nor_load(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_TEXT_BASE);

	return ;
}
#endif

