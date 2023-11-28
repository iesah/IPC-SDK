#ifndef __T15_AES_H__
#define __T15_AES_H__

//#define USE_AES_CPU_MODE

#define JZAES_IOC_MAGIC  'A'
#define IOCTL_AES_GET_PBUFF					_IO(JZAES_IOC_MAGIC, 110)
#define IOCTL_AES_SET_PARA					_IO(JZAES_IOC_MAGIC, 111)
#define IOCTL_AES_START_EN_PROCESSING		_IO(JZAES_IOC_MAGIC, 112)
#define IOCTL_AES_START_DE_PROCESSING		_IO(JZAES_IOC_MAGIC, 113)

struct aes_para {
	unsigned int mem_p;
	unsigned int status;
	unsigned int aeskey[4];
	unsigned int aesiv[4];
	unsigned int src_addr_p;
	unsigned char *src_addr_v;
	unsigned int dst_addr_p;
	unsigned char *dst_addr_v;
	unsigned int enAlg;
	unsigned int enBitWidth;
	unsigned int enWorkMode;
	unsigned int enKeyLen;
	unsigned int dataLen;
};

struct aes_operation {
	struct miscdevice aes_dev;
	struct resource *res;
	void __iomem *iomem;
	struct clk *clk;
	struct device *dev;
	int irq;
	char name[16];
};

typedef enum IN_UNF_CIPHER_ALG_E
{
	IN_UNF_CIPHER_ALG_AES = 0x0,
	IN_UNF_CIPHER_ALG_DES = 0x1
}IN_UNF_CIPHER_ALG;

typedef enum IN_UNF_CIPHER_WORK_MODE_E
{
	IN_UNF_CIPHER_WORK_MODE_ECB = 0x0,
	IN_UNF_CIPHER_WORK_MODE_CBC = 0x1,
	IN_UNF_CIPHER_WORK_MODE_OTHER = 0x2
}IN_UNF_CIPHER_WORK_MODE;

typedef enum IN_UNF_CIPHER_KEY_LENGTH_E
{
	IN_UNF_CIPHER_KEY_AES_128BIT = 0x0,
}IN_UNF_CIPHER_KEY_LENGTH;

typedef enum IN_UNF_CIPHER_BIT_WIDTH_E
{
	IN_UNF_CIPHER_BIT_WIDTH_128BIT = 0x0,
}IN_UNF_CIPHER_BIT_WIDTH;

#ifdef USE_AES_CPU_MODE
void en_ecb_cpu_mode(struct aes_operation *aes_ope ,struct aes_para para);
void en_cbc_cpu_mode(struct aes_operation *aes_ope ,struct aes_para para);
void de_ecb_cpu_mode(struct aes_operation *aes_ope ,struct aes_para para);
void de_cbc_cpu_mode(struct aes_operation *aes_ope ,struct aes_para para);
#endif

int  en_ecb_dma_mode(struct aes_operation *aes_ope ,struct aes_para para);
int  en_cbc_dma_mode(struct aes_operation *aes_ope ,struct aes_para para);

int  de_ecb_dma_mode(struct aes_operation *aes_ope ,struct aes_para para);
int  de_cbc_dma_mode(struct aes_operation *aes_ope ,struct aes_para para);

void aes_bit_set(struct aes_operation *aes_ope, int offset, unsigned int bit);
void aes_bit_clr(struct aes_operation *aes_ope, int offset, unsigned int bit);

static inline unsigned int aes_reg_read(struct aes_operation *aes_ope, int offset)
{
	//printk("%s, read:0x%08x, val = 0x%08x\n", __func__, aes_ope->iomem + offset, readl(aes_ope->iomem + offset));
	return readl(aes_ope->iomem + offset);
}

static inline void aes_reg_write(struct aes_operation *aes_ope, int offset, unsigned int val)
{
	writel(val, aes_ope->iomem + offset);
	//printk("%s, write:0x%08x, val = 0x%08x\n", __func__, aes_ope->iomem + offset, val);
}

#define AES_ASCR	0x00	//AES control register
#define AES_ASSR	0x04	//AES status register
#define AES_ASINTM	0x08	//AES interrupt mask register
#define AES_ASSA	0x0c	//AES DMA source address
#define AES_ASDA	0x10	//AES DMA destine address
#define AES_ASTC	0x14	//AES DMA transfer count
#define AES_ASDI	0x18	//AES data input
#define AES_ASDO	0x1c	//AES data output
#define AES_ASKY	0x20	//AES key input
#define AES_ASIV	0x24	//AES IV input

#define AES_ASCR_CLR		(1 << 10)
#define AES_ASCR_DMAS		(1 << 9)
#define AES_ASCR_DMAE		(1 << 8)
#define AES_ASCR_KEYL_128B	(0 << 6)
#define AES_ASCR_KEYL_192B	(1 << 6)
#define AES_ASCR_KEYL_256B	(2 << 6)
#define AES_ASCR_ECB_MODE	(0 << 5)
#define AES_ASCR_CBC_MODE	(1 << 5)
#define AES_ASCR_INEN_DAT	(0 << 4)
#define AES_ASCR_INDE_DAT	(1 << 4)
#define AES_ASCR_AESS		(1 << 3)
#define AES_ASCR_KEYS		(1 << 2)
#define AES_ASCR_INIT_IV	(1 << 1)
#define AES_ASCR_AES_EN		(1 << 0)

#define AES_ASSR_DMAD		(1 << 2)
#define AES_ASSR_AESD		(1 << 1)
#define AES_ASSR_KEYD		(1 << 0)

#define AES_ASINTM_DMA_INIT_M	(1 << 2)
#define AES_ASINTM_AES_INIT_M	(1 << 1)
#define AES_ASINTM_KEY_INIT_M	(1 << 0)

#define __aes_key_length_128b() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_KEYL_128B)
#define __aes_key_clr() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_CLR)
#define __aes_dmaenable() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_DMAE)
#define __aes_dmadisable() aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_DMAE)
#define __aes_dmastart() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_DMAS)
#define __aes_dmastop() aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_DMAS)
#define __aes_ecb_mode() aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_CBC_MODE)
#define __aes_cbc_mode() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_CBC_MODE)
#define __aes_encrypts_input_data() aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_INDE_DAT)
#define __aes_decrypts_input_data() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_INDE_DAT)
#define __aes_aesstart() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_AESS)
#define __aes_aesstop()  aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_AESS)
#define __aes_key_exp_start_com() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_KEYS)
#define __aes_key_exp_stop_com()  aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_KEYS)
#define __aes_iv_init() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_INIT_IV)
#define __aes_iv_uninit() aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_INIT_IV)
#define __aes_enable() aes_bit_set(aes_ope, AES_ASCR, AES_ASCR_AES_EN)
#define __aes_disable() aes_bit_clr(aes_ope, AES_ASCR, AES_ASCR_AES_EN)

#define __aes_dma_int_en() aes_bit_set(aes_ope, AES_ASINTM, AES_ASINTM_DMA_INIT_M)
#define __aes_dma_int_msk() aes_bit_clr(aes_ope, AES_ASINTM, AES_ASINTM_DMA_INIT_M)
#define __aes_aes_int_en() aes_bit_set(aes_ope, AES_ASINTM, AES_ASINTM_AES_INIT_M)
#define __aes_aes_int_msk() aes_bit_clr(aes_ope, AES_ASINTM, AES_ASINTM_AES_INIT_M)
#define __aes_key_int_en() aes_bit_set(aes_ope, AES_ASINTM, AES_ASINTM_KEY_INIT_M)
#define __aes_key_int_msk() aes_bit_clr(aes_ope, AES_ASINTM, AES_ASINTM_KEY_INIT_M)

#define __aes_key_done_clr() aes_bit_set(aes_ope, AES_ASSR, AES_ASSR_KEYD)
#define __aes_aes_done_clr() aes_bit_set(aes_ope, AES_ASSR, AES_ASSR_AESD)
#define __aes_dma_done_clr() aes_bit_set(aes_ope, AES_ASSR, AES_ASSR_DMAD)



#endif
