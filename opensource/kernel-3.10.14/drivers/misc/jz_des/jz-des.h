#ifndef __JZT10_DES_H__
#define __JZT10_DES_H__

#define JZT05_DES_BASE	0xAFFD0000

#define JZDES_IOC_MAGIC  'B'
#define IOCTL_DES_EBC_CPU_EN                 _IO(JZDES_IOC_MAGIC, 115)
#define IOCTL_DES_EBC_CPU_DE                 _IO(JZDES_IOC_MAGIC, 116)
#define IOCTL_DES_CBC_CPU_EN                 _IO(JZDES_IOC_MAGIC, 117)
#define IOCTL_DES_CBC_CPU_DE                 _IO(JZDES_IOC_MAGIC, 118)
#define IOCTL_DES_EBC_DMA_EN                 _IO(JZDES_IOC_MAGIC, 119)
#define IOCTL_DES_EBC_DMA_DE                 _IO(JZDES_IOC_MAGIC, 120)
#define IOCTL_DES_CBC_DMA_EN                 _IO(JZDES_IOC_MAGIC, 121)
#define IOCTL_DES_CBC_DMA_DE                 _IO(JZDES_IOC_MAGIC, 122)
#define IOCTL_DES_EBC_CPU_EN_TDES                 _IO(JZDES_IOC_MAGIC, 123)
#define IOCTL_DES_EBC_CPU_DE_TDES                 _IO(JZDES_IOC_MAGIC, 124)
#define IOCTL_DES_CBC_CPU_EN_TDES                 _IO(JZDES_IOC_MAGIC, 125)
#define IOCTL_DES_CBC_CPU_DE_TDES                 _IO(JZDES_IOC_MAGIC, 126)
#define IOCTL_DES_EBC_DMA_EN_TDES                 _IO(JZDES_IOC_MAGIC, 127)
#define IOCTL_DES_EBC_DMA_DE_TDES                 _IO(JZDES_IOC_MAGIC, 128)
#define IOCTL_DES_CBC_DMA_EN_TDES                 _IO(JZDES_IOC_MAGIC, 129)
#define IOCTL_DES_CBC_DMA_DE_TDES                 _IO(JZDES_IOC_MAGIC, 130)

struct des_operation {
	struct miscdevice des_dev;
	struct resource *res;
	void __iomem *iomem;
	struct clk *clk;
	struct device *dev;
	int irq;
	char name[16];
};

int en_ecb_cpu_mode(struct des_operation *des_ope);
int  enn_ecb_dma_mode(struct des_operation *des_ope);
int en_cbc_cpu_mode(struct des_operation *des_ope);
int  enn_cbc_dma_mode(struct des_operation *des_ope);

int de_ecb_cpu_mode(struct des_operation *des_ope);
int  dee_ecb_dma_mode(struct des_operation *des_ope);
int de_cbc_cpu_mode(struct des_operation *des_ope);
int  dee_cbc_dma_mode(struct des_operation *des_ope);

int en_ecb_cpu_mode_tdes(struct des_operation *des_ope);
int  en_ecb_dma_mode_tdes(struct des_operation *des_ope);
int en_cbc_cpu_mode_tdes(struct des_operation *des_ope);
int  en_cbc_dma_mode_tdes(struct des_operation *des_ope);

int de_ecb_cpu_mode_tdes(struct des_operation *des_ope);
int  de_ecb_dma_mode_tdes(struct des_operation *des_ope);
int de_cbc_cpu_mode_tdes(struct des_operation *des_ope);
int  de_cbc_dma_mode_tdes(struct des_operation *des_ope);


void des_bit_set(struct des_operation *des_ope, int offset, unsigned int bit);
void des_bit_clr(struct des_operation *des_ope, int offset, unsigned int bit);

static inline unsigned int des_reg_read(struct des_operation *des_ope, int offset)
{
	//printk("%s, read:0x%08x, val = 0x%08x\n", __func__, des_ope->iomem + offset, readl(des_ope->iomem + offset));
	return readl(des_ope->iomem + offset);
}

static inline void des_reg_write(struct des_operation *des_ope, int offset, unsigned int val)
{
	writel(val, des_ope->iomem + offset);
	//printk("%s, write:0x%08x, val = 0x%08x\n", __func__, des_ope->iomem + offset, val);
}

#define DES_DESCR1	0x00	//DES control register
#define DES_DESCR2	0x04	//DES status register
#define DES_DESSR 	0x08	//DES interrupt mask register
#define DES_DESK1L	0x10	//DES DMA source address
#define DES_DESK1R	0x14	//DES DMA destine address
#define DES_DESK2L	0x18	//DES DMA transfer count
#define DES_DESK2R	0x1C	//DES data input
#define DES_DESK3L	0x20	//DES data output
#define DES_DESK3R	0x24	//DES key input
#define DES_DESIVL	0x28	//DES IV input
#define DES_DESIVR	0x2C	//DES IV input
#define DES_DESDIN	0x30	//DES IV input
#define DES_DESDOUT	0x34	//DES IV input

#define DES_DES KEY Registers_CLR		(1 << 10)


#define DES_DESCR1_DES_EN		(1 << 0)
#define DES_DESCR1_DES_DISEN		(0)
#define DES_DESCR2_DMAE		(1 << 0)
#define DES_DESCR2_DMADISE		(0 << 0)
#define DES_DESCR2_ALG_SDES		(0 << 1)
#define DES_DESCR2_ALG_TDES		(1 << 1)
#define DES_DESCR2_ECB_MODE	(0 << 2)
#define DES_DESCR2_CBC_MODE	(1 << 2)
#define DES_DESCR2_INEN_DAT	(0 << 3)
#define DES_DESCR2_INDE_DAT	(1 << 3)
#define DES_DESCR2_DATENDIAN_BIG (0 << 4)
#define DES_DESCR2_DATENDIAN_LITTLE (1 << 4)

#define DES_DESSR_INIT_IV	(1 << 1)

#define __des_descr2_clear() des_bit_clr(des_ope, DES_DESCR2, 0xF)

#define __des_ecb_mode() des_bit_clr(des_ope, DES_DESCR2, DES_DESCR2_CBC_MODE)
#define __des_cbc_mode() des_bit_set(des_ope, DES_DESCR2, DES_DESCR2_CBC_MODE)

#define __des_iv_init() des_bit_set(des_ope, DES_DESCR2, DES_ASCR_INIT_IV)
#define __des_iv_uninit() des_bit_clr(des_ope, DES_DESCR2, DES_ASCR_INIT_IV)

#define __des_enable() des_bit_set(des_ope, DES_DESCR1, DES_DESCR1_DES_EN)
#define __des_disable() des_bit_clr(des_ope, DES_DESCR1, DES_DESCR1_DES_EN)

#define __des_ecb_mode_sdes_encry_li() des_bit_set(des_ope, DES_DESCR2, 0x10)
#define __des_ecb_mode_sdes_dencry_li() des_bit_set(des_ope, DES_DESCR2, 0x18)

#define __des_cbc_mode_sdes_encry_li() des_bit_set(des_ope, DES_DESCR2, 0x14)
#define __des_cbc_mode_sdes_dencry_li() des_bit_set(des_ope, DES_DESCR2, 0x1c)

#define __des_ecb_mode_sdes_encry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x11)
#define __des_ecb_mode_sdes_dencry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x19)

#define __des_cbc_mode_sdes_encry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x15)
#define __des_cbc_mode_sdes_dencry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x1d)


#define __des_ecb_mode_tdes_encry_li() des_bit_set(des_ope, DES_DESCR2, 0x12)
#define __des_ecb_mode_tdes_dencry_li() des_bit_set(des_ope, DES_DESCR2, 0x1a)

#define __des_cbc_mode_tdes_encry_li() des_bit_set(des_ope, DES_DESCR2, 0x16)
#define __des_cbc_mode_tdes_dencry_li() des_bit_set(des_ope, DES_DESCR2, 0x1e)

#define __des_ecb_mode_tdes_encry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x13)
#define __des_ecb_mode_tdes_dencry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x1b)

#define __des_cbc_mode_tdes_encry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x17)
#define __des_cbc_mode_tdes_dencry_li_dma() des_bit_set(des_ope, DES_DESCR2, 0x1f)


#endif
