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

/*
 *extern static void des_bit_set(struct des_operation *des_ope, int offset, unsigned int bit);
 *extern static void des_bit_clr(struct des_operation *des_ope, int offset, unsigned int bit);
 */

void prepare_tdes_asky(struct des_operation *des_ope)
{
	des_reg_write(des_ope, DES_DESK1L, 0xbede4bb5);
	des_reg_write(des_ope, DES_DESK1R, 0x0d7ba2b2);
	des_reg_write(des_ope, DES_DESK2L, 0x319fb7c7);
	des_reg_write(des_ope, DES_DESK2R, 0xc72f5f77);
	des_reg_write(des_ope, DES_DESK3L, 0x5f72979f);
	des_reg_write(des_ope, DES_DESK3R, 0x694f373c);
}

int en_ecb_cpu_mode_tdes(struct des_operation *des_ope)
{
	int ret  = 0;
	unsigned int num = 0;

	printk("----------- EN ECB CPU TDES MODE ---------\n");

	__des_descr2_clear();
	__des_ecb_mode_tdes_encry_li();
	printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));

	prepare_tdes_asky(des_ope);

	__des_enable();
	printk("--DES_DESCR1 0x%08x\n", des_reg_read(des_ope, DES_DESCR1));

	for (num = 0; num < 100; num++) {
		des_reg_write(des_ope, DES_DESDIN, 0xc94cbdd7);
		des_reg_write(des_ope, DES_DESDIN, 0xc75f89f8);
		des_reg_read(des_ope, DES_DESDOUT);
//		printk("-- val = 0x%08x, 0x%08x \n",des_reg_read(des_ope, DES_DESDOUT), des_reg_read(des_ope, DES_DESDOUT));
		if ((des_reg_read(des_ope, DES_DESDOUT) == 0x7f59ba15) || (des_reg_read(des_ope, DES_DESDOUT) == 0x14836860)){
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

int de_ecb_cpu_mode_tdes(struct des_operation *des_ope)
{
	int ret = 0 ;
	int num = 0;
	printk("----------- DE ECB CPU TDES MODE ---------\n");

	__des_descr2_clear();
	__des_ecb_mode_tdes_dencry_li();
	printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));

	prepare_tdes_asky(des_ope);

	__des_enable();

	for (num = 0; num < 100; num++) {
		des_reg_write(des_ope, DES_DESDIN, 0x7f59ba15);
		des_reg_write(des_ope, DES_DESDIN, 0x14836860);
		des_reg_read(des_ope, DES_DESDOUT);

		/*printk("--ASCR 0x%08x\n", des_reg_read(des_ope, AES_ASCR));*/
//		printk("-- val = 0x%08x, 0x%08x \n",des_reg_read(des_ope, DES_DESDOUT), des_reg_read(des_ope, DES_DESDOUT));
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

int en_ecb_dma_mode_tdes(struct des_operation *des_ope)
{
	int i = 0;
	int ret = 0;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;

	/*printk("----------- EN ECB DMA TDES MODE ---------\n");*/

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
	 __des_ecb_mode_tdes_encry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	prepare_tdes_asky(des_ope);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 300);

	dma_read(6, 0xb0061034, buf2,300 );
	for( i=0; i<=319; i++){
	//	printk("-- val = 0x%08x \n",buf2[i]);
		if ((buf2[i] == 0x7f59ba15) || (buf2[++i] == 0x14836860)){
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

int de_ecb_dma_mode_tdes(struct des_operation *des_ope)
{
	int i = 0;
	int ret = 0;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;

	/*printk("----------- DE ECB DMA TDES MODE ---------\n");*/

	dma_init(5);
	dma_init(6);

	buf = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr, GFP_KERNEL);

	buf2 = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr2, GFP_KERNEL);


	 for( i=0; i<=319; i++){
		 buf[i] = 0x7f59ba15;
		 buf[++i] = 0x14836860;
	 }

	__des_descr2_clear();
	__des_ecb_mode_tdes_dencry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	prepare_tdes_asky(des_ope);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 300);

	dma_read(6, 0xb0061034, buf2,300 );
	for( i=0; i<=319; i++){
		//printk("-- val = 0x%08x \n",buf2[i]);
		if ((buf2[i] == 0xc94cbdd7) || (buf2[++i] == 0xc75f89f8)){
			ret = 1;
			break;
		} else {
			ret = 0;
		}

	}

	/*printk("done\n");*/
	__des_disable();

	return ret;
}

int en_cbc_cpu_mode_tdes(struct des_operation *des_ope)
{
	int ret = 0;
	int num = 0;
	printk("----------- EN CBC CPU TDES MODE ---------\n");


	__des_descr2_clear();
	__des_cbc_mode_tdes_encry_li();

	prepare_tdes_asky(des_ope);


	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);

	__des_enable();

	for (num = 0; num < 100; num++) {
		des_reg_write(des_ope, DES_DESDIN, 0xc94cbdd7);
		des_reg_write(des_ope, DES_DESDIN, 0xc75f89f8);
		des_reg_read(des_ope, DES_DESDOUT);

//		printk("-- val = 0x%08x, 0x%08x\n",	des_reg_read(des_ope, DES_DESDOUT), des_reg_read(des_ope, DES_DESDOUT));
		if ((des_reg_read(des_ope, DES_DESDOUT) == 0x8df4d80e) || (des_reg_read(des_ope, DES_DESDOUT) == 0x129f752c)){
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

int de_cbc_cpu_mode_tdes(struct des_operation *des_ope)
{
	int ret = 0;
	int num = 0;
	printk("----------- DE CBC CPU TDES MODE ---------\n");

	__des_descr2_clear();
	__des_cbc_mode_tdes_dencry_li();
	printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));

	prepare_tdes_asky(des_ope);

	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);

	__des_enable();
	/*
	 *des_reg_write(des_ope, DES_DESDIN, 0x7649abac);
	 *des_reg_write(des_ope, DES_DESDIN, 0x8119b246);
	 *des_reg_write(des_ope, DES_DESDIN, 0xcee98e9b);
	 *des_reg_write(des_ope, DES_DESDIN, 0x12e9197d);
	 */
	for (num = 0; num < 100; num++) {
		des_reg_write(des_ope, DES_DESDIN, 0x8df4d80e);
		des_reg_write(des_ope, DES_DESDIN, 0x129f752c);
		des_reg_read(des_ope, DES_DESDOUT);

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

int en_cbc_dma_mode_tdes(struct des_operation *des_ope)
{
	int i = 0;
	int ret = 0;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;

	/*printk("----------- EN CBC DMA TDES MODE ---------\n");*/

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
	__des_cbc_mode_tdes_encry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	prepare_tdes_asky(des_ope);

	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 300);

	dma_read(6, 0xb0061034, buf2,300 );
	for( i=0; i<=319; i++){
		//printk("-- val = 0x%08x \n",buf2[i]);
		if ((buf2[i] == 0x82b08b9b) || (buf2[++i] == 0x129f752c)){
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

int de_cbc_dma_mode_tdes(struct des_operation *des_ope)
{
	int i = 0;
	int ret = 0;

	unsigned int * buf, *buf2;
	dma_addr_t phy_addr,phy_addr2;

	/*printk("----------- DE CBC DMA TDES MODE ---------\n");*/

	dma_init(5);
	dma_init(6);

	buf = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr, GFP_KERNEL);

	buf2 = dma_alloc_coherent(des_ope->dev,
			DMA_BUFFER_SIZE, &phy_addr2, GFP_KERNEL);

	for( i=0; i<=319; i++){
		 buf[i] = 0x8df4d80e;
		 buf[++i] = 0x129f752c;
	 }

	__des_descr2_clear();
	__des_cbc_mode_tdes_dencry_li_dma();
	/*printk("--DES_DESCR2 0x%08x\n", des_reg_read(des_ope, DES_DESCR2));*/

	des_reg_write(des_ope, DES_DESIVL, 0x054c4566);
	des_reg_write(des_ope, DES_DESIVR, 0x4aeb0008);
	prepare_tdes_asky(des_ope);

	__des_enable();

	/*printk("--- start\n");*/

	dma_write(5, buf, 0x10061030, 300);

	dma_read(6, 0xb0061034, buf2,300 );
	for( i=0; i<=319; i++){
		//printk("-- val = 0x%08x \n",buf2[i]);
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
