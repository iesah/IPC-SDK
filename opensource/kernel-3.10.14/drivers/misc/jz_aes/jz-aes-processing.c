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
#include <crypto/aes.h>
#include <linux/fs.h>

#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>

#include "jz-aes.h"

void prepare_aes_asky(struct aes_operation *aes_ope, struct aes_para para)
{
	aes_reg_write(aes_ope, AES_ASKY, para.aeskey[0]);
	aes_reg_write(aes_ope, AES_ASKY, para.aeskey[1]);
	aes_reg_write(aes_ope, AES_ASKY, para.aeskey[2]);
	aes_reg_write(aes_ope, AES_ASKY, para.aeskey[3]);
	__aes_key_exp_start_com();
	while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x01)) {
		;
	}
	__aes_key_done_clr();
}

void prepare_aes_iv(struct aes_operation *aes_ope, struct aes_para para)
{
	aes_reg_write(aes_ope, AES_ASIV, para.aesiv[0]);
	aes_reg_write(aes_ope, AES_ASIV, para.aesiv[1]);
	aes_reg_write(aes_ope, AES_ASIV, para.aesiv[2]);
	aes_reg_write(aes_ope, AES_ASIV, para.aesiv[3]);
	__aes_iv_init();
}

#ifdef USE_AES_CPU_MODE
void en_ecb_cpu_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	unsigned int num = 0;

	struct file *f_read = NULL;
	struct file *f_write = NULL;
	static loff_t f_test_offset = 0;
	static mm_segment_t old_fs;

	static loff_t f_test_offset_r = 0;
	static mm_segment_t old_fs_r;
	unsigned int * ffp = (unsigned int *)kmalloc(1024*1024, GFP_KERNEL);
	memset(ffp, 0, 1024*1024);

	printk("----------- EN ECB CPU MODE ---------\n");

	__aes_enable();
	__aes_ecb_mode();
	__aes_encrypts_input_data();
	__aes_key_length_128b();
	__aes_key_clr();
	while((aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		num++;
		/*aes_reg_write(aes_ope, AES_ASCR, (aes_reg_read(aes_ope, AES_ASCR) & (~0x20)));*/
		__aes_ecb_mode();
		printk("-- %s , num=%d\n", __func__, num);
	}

	/*prepare_aes_asky(aes_ope);*/
	prepare_aes_asky(aes_ope, para);



	/*****/
	f_write = filp_open("/home/hhhhh", O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (!IS_ERR(f_write)) {
		printk("open aes_ori_d sussess %p.\n",f_write);
		f_test_offset = f_write->f_pos;
	} else {
		printk("---->open %s failed %d!\n","/data/driver_data_ch0.dat",f_write);
	}
	f_read = filp_open("/home/aes_ori_d", O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (!IS_ERR(f_read)) {
		printk("open aes_ori_d sussess %p.\n",f_read);
		f_test_offset_r = f_read->f_pos;
	} else {
		printk("---->open %s failed %d!\n","/data/driver_data_ch0.dat",f_read);
	}

	old_fs_r = get_fs();
	set_fs(KERNEL_DS);
	vfs_read(f_read, ffp , 1024*1024, &f_test_offset_r);
	f_test_offset_r = f_read->f_pos;
	set_fs(old_fs_r);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	vfs_write(f_write, ffp , 1024*1024, &f_test_offset);
	f_test_offset = f_write->f_pos;
	set_fs(old_fs);
	if (!IS_ERR(f_write))
		filp_close(f_write, NULL);
	/******/



	for (num = 0; num < 1000; num++) {
		aes_reg_write(aes_ope, AES_ASDI, 0x3243f6a8);
		aes_reg_write(aes_ope, AES_ASDI, 0x885a308d);
		aes_reg_write(aes_ope, AES_ASDI, 0x313198a2);
		aes_reg_write(aes_ope, AES_ASDI, 0xe0370734);
		__aes_aesstart();
		while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x02)) {
			;
		}
		__aes_aes_done_clr();

		printk("-- val = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO),
															aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO));
		/*int i = 0;*/
		/*printk("--ASCR 0x%08x\n", aes_reg_read(aes_ope, AES_ASCR));*/
		/*
		 *for(i = 0; i < 4; i++) {
		 *    printk("-- val.%d = 0x%08x\n", i, aes_reg_read(aes_ope, AES_ASDO));
		 *}
		 */
	}
	printk("done\n");
	__aes_key_clr();
	__aes_aesstop();
	__aes_disable();
}
void de_ecb_cpu_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	int num = 0;
	printk("----------- DE ECB CPU MODE ---------\n");

	__aes_enable();
	__aes_ecb_mode();
	__aes_decrypts_input_data();
	__aes_key_length_128b();
	__aes_key_clr();
	while((aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		/*aes_reg_write(aes_ope, AES_ASCR, (aes_reg_read(aes_ope, AES_ASCR) & (~0x20)));*/
		num++;
		__aes_ecb_mode();
		printk("-- %s , num=%d\n", __func__, num);
	}

	/*prepare_aes_asky(aes_ope);*/
	prepare_aes_asky(aes_ope, para);

	for (num = 0; num < 1000; num++) {
		aes_reg_write(aes_ope, AES_ASDI, 0x3925841d);
		aes_reg_write(aes_ope, AES_ASDI, 0x02dc09fb);
		aes_reg_write(aes_ope, AES_ASDI, 0xdc118597);
		aes_reg_write(aes_ope, AES_ASDI, 0x196a0b32);
		__aes_aesstart();
		while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x02)) {
			;
		}
		__aes_aes_done_clr();

		/*printk("--ASCR 0x%08x\n", aes_reg_read(aes_ope, AES_ASCR));*/
		 printk("-- val = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO),
		                                                     aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO));
		/*
		 *for(i = 0; i < 4; i++) {
		 *    printk("-- val.%d = 0x%08x\n", i, aes_reg_read(aes_ope, AES_ASDO));
		 *}
		 */
	}
	/* add */
	/*int i = 0;*/
/*
 *    aes_reg_write(aes_ope, AES_ASDI, 0x39f60b2e);
 *    aes_reg_write(aes_ope, AES_ASDI, 0x7472d3de);
 *    aes_reg_write(aes_ope, AES_ASDI, 0x62c24e8c);
 *    aes_reg_write(aes_ope, AES_ASDI, 0x8e87e755);
 *    __aes_aesstart();
 *    while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x02)) {
 *        ;
 *    }
 *    __aes_aes_done_clr();
 *
 *    printk("--ASCR 0x%08x\n", aes_reg_read(aes_ope, AES_ASCR));
 *    for(i = 0; i < 4; i++) {
 *        printk("-- val.%d = 0x%08x\n", i, aes_reg_read(aes_ope, AES_ASDO));
 *    }
 */
	/* add end*/

	__aes_key_clr();
	__aes_aesstop();
	__aes_disable();
}
#endif

void debug_aes_para(struct aes_para para)
{
#if 0
	printk("============================ aes para debug ==============================\n");
	printk("Key: key1 = 0x%08x, key2 = 0x%08x, key3 = 0x%08x, key4 = 0x%08x\n",
			para.aeskey[0], para.aeskey[1], para.aeskey[2], para.aeskey[3]);
	printk("IV : iv-1 = 0x%08x, iv-2 = 0x%08x, iv-3 = 0x%08x, iv-4 = 0x%08x\n",
			para.aesiv[0], para.aesiv[1], para.aesiv[2], para.aesiv[3]);
	printk("ADDR: \nsrc_addr_p = 0x%08x, \nsrc_addr_v = 0x%08x, \ndst_addr_p = 0x%08x, \ndst_addr_v = 0x%08x\n",
				  para.src_addr_p, para.src_addr_v, para.dst_addr_p, para.dst_addr_v);
	printk("alg = %d, \nBitWidth = %d, \nenWorkMode = %d, \nenKeyLen = %d, \ndataLen = %d\n",
			para.enAlg, para.enBitWidth, para.enWorkMode, para.enKeyLen, para.dataLen);
	printk("========================== aes para debug end ============================\n");
#endif
}

int en_ecb_dma_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	int num = 0;

	debug_aes_para(para);

	/*
	 *dma_cache_sync(NULL, para.src_addr_v, para.dataLen, DMA_TO_DEVICE);
	 *dma_cache_sync(NULL, para.dst_addr_v, para.dataLen, DMA_FROM_DEVICE);
	 */

	__aes_enable();
	__aes_ecb_mode();
	__aes_encrypts_input_data();

	if (para.enKeyLen == IN_UNF_CIPHER_KEY_AES_128BIT)
		__aes_key_length_128b();
	else {
		printk("Sorry, we are not support this key len!\n");
		return -1;
	}

	__aes_key_clr();
	__aes_dmaenable();
	while((aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		num++;
		__aes_ecb_mode();
	}

	prepare_aes_asky(aes_ope, para);

	aes_reg_write(aes_ope, AES_ASTC, para.dataLen / 16);
	aes_reg_write(aes_ope, AES_ASSA, para.src_addr_p);
	aes_reg_write(aes_ope, AES_ASDA, para.dst_addr_p);
	__aes_dmastart();
	while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x04)) {
		;
	}
	__aes_dma_done_clr();

	__aes_key_clr();

	__aes_dmastop();
	__aes_dmadisable();
	__aes_disable();

	return 0;
}
int de_ecb_dma_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	int num = 0;

	debug_aes_para(para);

	/*
	 *dma_cache_sync(NULL, para.src_addr_v, para.dataLen, DMA_TO_DEVICE);
	 *dma_cache_sync(NULL, para.dst_addr_v, para.dataLen, DMA_FROM_DEVICE);
	 */

	__aes_enable();
	__aes_ecb_mode();
	__aes_decrypts_input_data();

	if (para.enKeyLen == IN_UNF_CIPHER_KEY_AES_128BIT)
		__aes_key_length_128b();
	else {
		printk("Sorry, we are not support this key len!\n");
		return -1;
	}
	__aes_key_clr();
	__aes_dmaenable();
	while((aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		num++;
		__aes_ecb_mode();
	}

	prepare_aes_asky(aes_ope, para);

	aes_reg_write(aes_ope, AES_ASTC, para.dataLen / 16);
	aes_reg_write(aes_ope, AES_ASSA, para.src_addr_p);
	aes_reg_write(aes_ope, AES_ASDA, para.dst_addr_p);
	__aes_dmastart();
	while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x04)) {
		;
	}
	__aes_dma_done_clr();

	__aes_key_clr();

	__aes_dmastop();
	__aes_dmadisable();
	__aes_disable();

	return 0;
}

#ifdef USE_AES_CPU_MODE
void en_cbc_cpu_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	int num = 0;
	printk("----------- EN CBC CPU MODE ---------\n");

	__aes_enable();
	__aes_cbc_mode();
	__aes_encrypts_input_data();
	__aes_key_length_128b();
	__aes_key_clr();
	while(!(aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		/*aes_reg_write(aes_ope, AES_ASCR, (aes_reg_read(aes_ope, AES_ASCR) | (0x20)));*/
		num++;
		__aes_cbc_mode();
		printk("-- %s , num=%d\n", __func__, num);
	}

	/*prepare_aes_asky(aes_ope);*/
	prepare_aes_asky(aes_ope, para);

	aes_reg_write(aes_ope, AES_ASIV, 0x00010203);
	aes_reg_write(aes_ope, AES_ASIV, 0x04050607);
	aes_reg_write(aes_ope, AES_ASIV, 0x08090a0b);
	aes_reg_write(aes_ope, AES_ASIV, 0x0c0d0e0f);
	__aes_iv_init();

	for (num = 0; num < 1000; num++) {
		aes_reg_write(aes_ope, AES_ASDI, 0x6bc1bee2);
		aes_reg_write(aes_ope, AES_ASDI, 0x2e409f96);
		aes_reg_write(aes_ope, AES_ASDI, 0xe93d7e11);
		aes_reg_write(aes_ope, AES_ASDI, 0x7393172a);
		__aes_aesstart();
		while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x02)) {
			;
		}
		__aes_aes_done_clr();

		printk("-- val = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO),
															aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO));
		/*
		 *int i = 0;
		 *printk("--ASCR 0x%08x\n", aes_reg_read(aes_ope, AES_ASCR));
		 *for(i = 0; i < 4; i++) {
		 *    printk("--- val.%d = 0x%08x\n", i, aes_reg_read(aes_ope, AES_ASDO));
		 *}
		 */
	}

	__aes_aesstop();
	__aes_key_clr();
	__aes_disable();
}

void de_cbc_cpu_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	int num = 0;
	printk("----------- DE CBC CPU MODE ---------\n");

	__aes_enable();
	__aes_cbc_mode();
	__aes_decrypts_input_data();
	__aes_key_length_128b();
	__aes_key_clr();
	while(!(aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		/*aes_reg_write(aes_ope, AES_ASCR, (aes_reg_read(aes_ope, AES_ASCR) | (0x20)));*/
		num++;
		__aes_cbc_mode();
		printk("-- %s , num=%d\n", __func__, num);
	}

	/*prepare_aes_asky(aes_ope);*/
	prepare_aes_asky(aes_ope, para);

	aes_reg_write(aes_ope, AES_ASIV, 0x00010203);
	aes_reg_write(aes_ope, AES_ASIV, 0x04050607);
	aes_reg_write(aes_ope, AES_ASIV, 0x08090a0b);
	aes_reg_write(aes_ope, AES_ASIV, 0x0c0d0e0f);
	__aes_iv_init();

	/*
	 *aes_reg_write(aes_ope, AES_ASDI, 0x7649abac);
	 *aes_reg_write(aes_ope, AES_ASDI, 0x8119b246);
	 *aes_reg_write(aes_ope, AES_ASDI, 0xcee98e9b);
	 *aes_reg_write(aes_ope, AES_ASDI, 0x12e9197d);
	 */
	for (num = 0; num < 4000; num +=4) {
#if 0
		aes_reg_write(aes_ope, AES_ASDI, array1[num + 0]);
		aes_reg_write(aes_ope, AES_ASDI, array1[num + 1]);
		aes_reg_write(aes_ope, AES_ASDI, array1[num + 2]);
		aes_reg_write(aes_ope, AES_ASDI, array1[num + 3]);
#endif
		__aes_aesstart();
		while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x02)) {
			;
		}
		__aes_aes_done_clr();

		printk("-- val = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO),
															aes_reg_read(aes_ope, AES_ASDO), aes_reg_read(aes_ope, AES_ASDO));
		/*
		 *int i = 0;
		 *printk("--ASCR 0x%08x\n", aes_reg_read(aes_ope, AES_ASCR));
		 *for(i = 0; i < 4; i++) {
		 *    printk("--- val.%d = 0x%08x\n", i, aes_reg_read(aes_ope, AES_ASDO));
		 *}
		 */
	}

	__aes_key_clr();
	__aes_aesstop();
	__aes_disable();
}
#endif

int en_cbc_dma_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	int num = 0;
	debug_aes_para(para);

	dma_cache_sync(NULL, para.src_addr_v, para.dataLen, DMA_TO_DEVICE);
	dma_cache_sync(NULL, para.dst_addr_v, para.dataLen, DMA_FROM_DEVICE);

	__aes_enable();
	__aes_cbc_mode();
	__aes_encrypts_input_data();

	if (para.enKeyLen == IN_UNF_CIPHER_KEY_AES_128BIT)
		__aes_key_length_128b();
	else {
		printk("Sorry, we are not support this key len!\n");
		return -1;
	}

	__aes_key_clr();
	__aes_dmaenable();
	while(!(aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		num++;
		__aes_cbc_mode();
	}

	prepare_aes_asky(aes_ope, para);
	prepare_aes_iv(aes_ope, para);

	aes_reg_write(aes_ope, AES_ASTC, para.dataLen / 16);
	aes_reg_write(aes_ope, AES_ASSA, para.src_addr_p);
	aes_reg_write(aes_ope, AES_ASDA, para.dst_addr_p);
	__aes_dmastart();
	while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x04)) {
		;
	}
	__aes_dma_done_clr();

	__aes_key_clr();

	__aes_dmastop();
	__aes_dmadisable();
	__aes_disable();

	return 0;
}
int de_cbc_dma_mode(struct aes_operation *aes_ope, struct aes_para para)
{
	int num = 0;

	debug_aes_para(para);

	dma_cache_sync(NULL, para.src_addr_v, para.dataLen, DMA_TO_DEVICE);
	dma_cache_sync(NULL, para.dst_addr_v, para.dataLen, DMA_FROM_DEVICE);

	__aes_enable();
	__aes_cbc_mode();
	__aes_decrypts_input_data();

	if (para.enKeyLen == IN_UNF_CIPHER_KEY_AES_128BIT)
		__aes_key_length_128b();
	else {
		printk("Sorry, we are not support this key len!\n");
		return -1;
	}

	__aes_key_clr();
	__aes_dmaenable();
	while(!(aes_reg_read(aes_ope, AES_ASCR) & 0x20)) {
		num++;
		__aes_cbc_mode();
	}

	prepare_aes_asky(aes_ope, para);
	prepare_aes_iv(aes_ope, para);

	aes_reg_write(aes_ope, AES_ASTC, para.dataLen / 16);
	aes_reg_write(aes_ope, AES_ASSA, para.src_addr_p);
	aes_reg_write(aes_ope, AES_ASDA, para.dst_addr_p);
	__aes_dmastart();
	while(!(aes_reg_read(aes_ope, AES_ASSR) & 0x04)) {
		;
	}
	__aes_dma_done_clr();

	__aes_key_clr();

	__aes_dmastop();
	__aes_dmadisable();
	__aes_disable();

	return 0;
}
