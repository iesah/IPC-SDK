#include <linux/err.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/io.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <crypto/scatterwalk.h>
#include <crypto/des.h>
#include <linux/fs.h>
#include <linux/time.h>

#include <linux/scatterlist.h>
#include <linux/dmaengine.h>
#include <asm/dma.h>
#include <mach/jzdma.h>
#include <linux/vmalloc.h>

#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>

#include "jz-des.h"
#include "jz_dmac.h"

#define DMA_BUFFER_SIZE  4000

#define REG32(addr)	(*(volatile unsigned int *)(addr))

void dma_init(unsigned int ch)
{
	REG_DMAC_DMACR &=~(1<<2);   //AR
	REG_DMAC_DMACR &=~(1<<3);   //HLT
	REG_DMAC_DSAR(ch) &=~(1<<4); //AR
	REG_DMAC_DSAR(ch) &=~(1<<2); //HLT
	REG_DMAC_DSAR(ch) &=~(1<<3); //TT
	REG_DMAC_DTCR(ch) = 0;	//TC

	REG_DMAC_DMACR &=~ 1;    //enable
	REG_DMAC_DMACR |= 1;    //enable

	/*printk("dma_init ok\n");*/
}


void dma_write(unsigned int ch,unsigned int *src_buf, unsigned int dst_buf, unsigned int len)
{

	REG_DMAC_DSAR(ch) = (unsigned int)(src_buf) & 0X1fffffff;
	REG_DMAC_DTAR(ch) =	(unsigned int)(dst_buf) & 0X1fffffff;

	/*printk("dma_transfer src_buf =%08x\n",REG_DMAC_DSAR(ch));*/
	/*printk("dma_transfer dst_buf =%08x\n",REG_DMAC_DTAR(ch));*/

	REG_DMAC_DTCR(ch) = len;
	REG_DMAC_DRSR(ch) = 0x2E; //des tx request
	//REG_DMAC_DRSR(ch) = 0X8; //auto-request

	REG_DMAC_DCCSR(ch) = 0;	//AR
	REG_DMAC_DCCSR(ch) |= (1<<31);   //no-descriptor
	REG_DMAC_DCCSR(ch) &=~(1<<4);	//AR

	REG_DMAC_DCMD(ch) = 0;    //src_addr increment 1
	REG_DMAC_DCMD(ch) |= (1<<23);    //src_addr increment 1
	REG_DMAC_DCMD(ch) &=~ (1<<22);    //dst_addr no increment
	REG_DMAC_DCMD(ch) |= (2<<16);    //RDIL
//	REG_DMAC_DCMD(ch) |= (2<<14);    //src port width
//	REG_DMAC_DCMD(ch) |= (2<<12);    //dst port width
	//REG_DMAC_DCMD(ch) |= (2<<8);     //transfer data size  test ok
	REG_DMAC_DCMD(ch) &=~ (3<<14);    //src port width
	REG_DMAC_DCMD(ch) &=~ (3<<12);    //dst port width
	REG_DMAC_DCMD(ch) &= ~(7<<8);     //transfer data size 32BIT
	REG_DMAC_DCMD(ch) &=~ (1<<2); //STDE
	REG_DMAC_DCMD(ch) &=~ (1<<1); //TIE
	REG_DMAC_DCMD(ch) &=~ (1<<0); //


	REG_DMAC_DCCSR(ch) &=~ (1<<3);
	REG_DMAC_DCCSR(ch) |= 1; //enable

	while(!(REG_DMAC_DCCSR(ch)&(1<<3)))
	{
		/*printk("dma_write wait tt\n");*/
	}

	REG_DMAC_DCCSR(ch) &=~1;

}

void dma_read(unsigned int ch, unsigned int src_buf, unsigned int *dst_buf, unsigned int len)
{
	REG_DMAC_DSAR(ch) = (unsigned int)(src_buf) & 0X1fffffff;
	REG_DMAC_DTAR(ch) =	(unsigned int)(dst_buf) & 0X1fffffff;

	/*printk("dma_read src_buf =%08x\n",REG_DMAC_DSAR(ch));*/
	/*printk("dma_read dst_buf =%08x\n",REG_DMAC_DTAR(ch));*/

	REG_DMAC_DTCR(ch) = len;
	//REG_DMAC_DRSR(ch) = 0x2F; //des rx
	REG_DMAC_DRSR(ch) = 0X8; //auto-request
	REG_DMAC_DCCSR(ch) |= (1<<31);   //no-descriptor
	REG_DMAC_DCCSR(ch) &=~(1<<4);	//AR

	REG_DMAC_DCMD(ch) = 0;    //src_addr increment 1
	REG_DMAC_DCMD(ch) &=~ (1<<23);    //src_addr NO INCREMENT
	REG_DMAC_DCMD(ch) |= (1<<22);    //dst_addr increment

	REG_DMAC_DCMD(ch) |= (2<<16);    //RDIL

	REG_DMAC_DCMD(ch) &=~ (3<<14);    //src port width
	REG_DMAC_DCMD(ch) &=~ (3<<12);    //dst port width
	REG_DMAC_DCMD(ch) &= ~(7<<8);     //transfer data size 32BIT

	REG_DMAC_DCMD(ch) &=~ (1<<2); //STDE
	REG_DMAC_DCMD(ch) &=~ (1<<1); //TIE

	REG_DMAC_DCCSR(ch) &=~ (1<<3);
	REG_DMAC_DCCSR(ch) |= 1; //enable

	while(!(REG_DMAC_DCCSR(ch)&(1<<3)))
	{
		/*printk("dma_read wait tt\n");*/
	}
	REG_DMAC_DCCSR(ch) &=~1;

}

/*
 *extern static void des_bit_set(struct des_operation *des_ope, int offset, unsigned int bit);
 *extern static void des_bit_clr(struct des_operation *des_ope, int offset, unsigned int bit);
 */


void prepare_sdes_asky(struct des_operation *des_ope)
{
	des_reg_write(des_ope, DES_DESK1L, 0xbede4bb5);
	des_reg_write(des_ope, DES_DESK1R, 0x0d7ba2b2);
}

int en_ecb_cpu_mode(struct des_operation *des_ope)
{
	int ret = 0;
	unsigned int num = 0;

	printk("----------- EN ECB CPU MODE ---------\n");

	__des_descr2_clear();
	__des_ecb_mode_sdes_encry_li();
	printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));

	prepare_sdes_asky(des_ope);

	__des_enable();
	printk("--DES_DESCR1 0x%08x\n", des_reg_read(des_ope, DES_DESCR1));

	for (num = 0; num < 100; num++) {
		des_reg_write(des_ope, DES_DESDIN, 0xc94cbdd7);
		des_reg_write(des_ope, DES_DESDIN, 0xc75f89f8);

		//printk("-- val = 0x%08x, 0x%08x \n",des_reg_read(des_ope, DES_DESDOUT), des_reg_read(des_ope, DES_DESDOUT));
		if ((des_reg_read(des_ope, DES_DESDOUT) == 0x82b08b9b) || (des_reg_read(des_ope, DES_DESDOUT) == 0x574e1d4f)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}
	printk("done\n");
	__des_disable();

	return ret;
}

int de_ecb_cpu_mode(struct des_operation *des_ope)
{
	int ret = 0;
	int num = 0;
	printk("----------- DE ECB CPU MODE ---------\n");

	__des_descr2_clear();
	__des_ecb_mode_sdes_dencry_li();
	printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));

	prepare_sdes_asky(des_ope);

	__des_enable();

	for (num = 0; num < 100; num++) {
		des_reg_write(des_ope, DES_DESDIN, 0x82b08b9b);
		des_reg_write(des_ope, DES_DESDIN, 0x574e1d4f);

		//printk("-- val = 0x%08x, 0x%08x \n",des_reg_read(des_ope, DES_DESDOUT), des_reg_read(des_ope, DES_DESDOUT));
		if ((des_reg_read(des_ope, DES_DESDOUT) == 0xc94cbdd7) || (des_reg_read(des_ope, DES_DESDOUT) == 0xc75f89f8)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}
	}

	__des_disable();

	return ret;
}

int enn_ecb_dma_mode(struct des_operation *des_ope)
{
	int ret = 0;
	int i;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;

	/*printk("----------- EN ECB DMA MODE ---------\n");*/

	dma_init(5);
	dma_init(6);

	buf = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr, GFP_KERNEL);

	buf2 = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr2, GFP_KERNEL);


	 for( i=0; i<=319; i++){
		 buf[i] = 0xc94cbdd7;
		 buf[++i] = 0xc75f89f8;
	 }


	__des_descr2_clear();
	__des_ecb_mode_sdes_encry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	prepare_sdes_asky(des_ope);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 400);

	dma_read(6, 0xb0061034, buf2,200 );
	for( i=0; i<=319; i++){
		//printk("-- val = 0x%08x \n",buf2[i]);
		if ((buf2[i] == 0x82b08b9b) || (buf2[++i] == 0x574e1d4f)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}

	/*printk("done\n");*/
	__des_disable();

	/*printk("--- end\n");*/
	return ret;
}

int dee_ecb_dma_mode(struct des_operation *des_ope)
{
	int ret = 0;
	int i;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;


	/*printk("----------- DE ECB DMA MODE ---------\n");*/

	dma_init(5);
	dma_init(6);

	buf = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr, GFP_KERNEL);

	buf2 = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr2, GFP_KERNEL);

	 for( i=0; i<=319; i++){
		 buf[i] = 0x82b08b9b;
		 buf[++i] = 0x574e1d4f;
	 }

	__des_descr2_clear();
	__des_ecb_mode_sdes_dencry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	prepare_sdes_asky(des_ope);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 300);

	dma_read(6, 0xb0061034, buf2,300 );
	for( i=0; i<=319; i++){
	//	printk("-- val = 0x%08x \n",buf2[i]);
		if ((buf2[i] == 0xc94cbdd7) || (buf2[++i] == 0xc75f89f8)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}

	/*printk("done\n");*/
	__des_disable();

	/*printk("--------- * done *\n");*/

	return ret;
}

int en_cbc_cpu_mode(struct des_operation *des_ope)
{
	int ret = 0;
	int num = 0;
	printk("----------- EN CBC CPU MODE ---------\n");

	__des_descr2_clear();
	__des_cbc_mode_sdes_encry_li();
	printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));

	prepare_sdes_asky(des_ope);


	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);

	__des_enable();

	for (num = 0; num < 100; num++) {
		des_reg_write(des_ope, DES_DESDIN, 0xc94cbdd7);
		des_reg_write(des_ope, DES_DESDIN, 0xc75f89f8);

//		printk("-- val = 0x%08x, 0x%08x\n",	des_reg_read(des_ope, DES_DESDOUT), des_reg_read(des_ope, DES_DESDOUT));
		if ((des_reg_read(des_ope, DES_DESDOUT) == 0x932ac6e2) || (des_reg_read(des_ope, DES_DESDOUT) == 0xe01ba82a)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}

	printk("done\n");
	__des_disable();

	return ret;
}

int de_cbc_cpu_mode(struct des_operation *des_ope)
{
	int ret = 0;
	int num = 0;
	printk("----------- DE CBC CPU MODE ---------\n");

	__des_descr2_clear();
	__des_cbc_mode_sdes_dencry_li();
	printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));

	prepare_sdes_asky(des_ope);

	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);

	__des_enable();
	/*
	 *des_reg_write(des_ope, DES_DESDIN, 0x7649abac);
	 *des_reg_write(des_ope, DES_DESDIN, 0x8119b246);
	 *des_reg_write(des_ope, DES_DESDIN, 0xcee98e9b);
	 *des_reg_write(des_ope, DES_DESDIN, 0x12e9197d);
	 */
	for (num = 0; num < 100; num ++) {
		des_reg_write(des_ope, DES_DESDIN, 0x932ac6e2);
		des_reg_write(des_ope, DES_DESDIN, 0xe01ba82a);

//		printk("-- val = 0x%08x, 0x%08x\n",	des_reg_read(des_ope, DES_DESDOUT), des_reg_read(des_ope, DES_DESDOUT));
		if ((des_reg_read(des_ope, DES_DESDOUT) == 0xc94cbdd7) || (des_reg_read(des_ope, DES_DESDOUT) == 0xc75f89f8)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}

	printk("done\n");
	__des_disable();

	return ret;
}

int enn_cbc_dma_mode(struct des_operation *des_ope)
{
	int ret = 0;
	int i;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;

/*	printk("----------- EN CBC DMA MODE ---------\n");*/

	dma_init(5);
	dma_init(6);

	buf = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr, GFP_KERNEL);

	buf2 = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr2, GFP_KERNEL);


	for( i=0; i<=319; i++){
		 buf[i] = 0xc94cbdd7;
		 buf[++i] = 0xc75f89f8;
	 }

	__des_descr2_clear();
	__des_cbc_mode_sdes_encry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	prepare_sdes_asky(des_ope);

	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 300);

	dma_read(6, 0xb0061034, buf2,300 );
	for( i=0; i<=319; i++){
	//	printk("-- val = 0x%08x \n",buf2[i]);
		if ((buf2[i] == 0x932ac6e2) || (buf2[++i] == 0xe01ba82a)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}

	/*printk("done\n");*/
	__des_disable();

	/*printk("--- end\n");*/


	return ret;
}

int dee_cbc_dma_mode(struct des_operation *des_ope)
{
	int ret = 0;
	int i;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;

	/*printk("----------- DE CBC DMA MODE ---------\n");*/

	dma_init(5);
	dma_init(6);

	buf = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr, GFP_KERNEL);

	buf2 = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr2, GFP_KERNEL);


	for( i=0; i<=319; i++){
		 buf[i] = 0x932ac6e2;
		 buf[++i] = 0xe01ba82a;
	 }

	__des_descr2_clear();
	__des_cbc_mode_sdes_dencry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);
	prepare_sdes_asky(des_ope);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 300);

	dma_read(6, 0xb0061034, buf2,300 );
	for( i=0; i<=319; i++){
	//	printk("-- val = 0x%08x \n",buf2[i]);
		if ((buf2[i] == 0xc94cbdd7) || (buf2[++i] == 0xc75f89f8)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}

	/*printk("done\n");*/
	__des_disable();

	/*printk("--- end\n");*/

	return ret;
}
