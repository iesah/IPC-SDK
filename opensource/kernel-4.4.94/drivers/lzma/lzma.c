#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <soc/base.h>
#include <linux/string.h>

//#include <asm/arch/clk.h>
#include <../drivers/media/platform/ingenic-jpeg/jpeg-regs.h>

#define LZMA_CTRL		0x00
#define LZMA_BS_BASE	0x04
#define LZMA_BS_SIZE	0x08
#define LZMA_DST_BASE	0x0C
#define LZMA_ICRC		0x10
#define LZMA_OCRC		0x14
#define LZMA_TIMEOUT	0x18
#define LZMA_FINAL		0x1c

#define CPM_BASE_M      (char *)0xb0000000
#define CPM_CLKGR0		0x20
#define CPM_BSCCDR		0xa0

#define LZMA_BASE      (char *)0xb3090000
#define BSCALER_BASE   (char *)0xb3090000
#define REG32(x) *(volatile unsigned int *)(x)

void dump_lzma_reg(void)
{
	printk("\nDUMP CPM LZMA REG : \n");
	printk("CPM_BSCCDR	= 0x%x \n", readl( CPM_BASE_M + CPM_BSCCDR));
	printk("CPM_CLKGR0	= 0x%x \n", readl( CPM_BASE_M + CPM_CLKGR0));

	printk("DUMP LZMA REG :  \n");
	printk("LZMA_CTRL     = 0x%x \n", readl( LZMA_BASE+ LZMA_CTRL));
	printk("LZMA_BS_BASE  = 0x%x \n", readl(LZMA_BASE + LZMA_BS_BASE));
	printk("LZMA_BS_SIZE  = 0x%x \n",  readl(LZMA_BASE + LZMA_BS_SIZE));
	printk("LZMA_DST_BASE = 0x%x \n", readl( LZMA_BASE + LZMA_DST_BASE));
	printk("LZMA_FINAL	= 0x%x \n",  readl(LZMA_BASE + LZMA_FINAL));
}

int jz_lzma_decompress(unsigned char *src, size_t size, unsigned char *dst)
{
	unsigned int value = 0, outlen = 0;
	unsigned int clk_val = 0, clk_gate = 0,ret = 0;
	unsigned long out_file ,in_file;
	unsigned int out_reg = 0,in_reg=0;
	mm_segment_t fs;

	/* config clk */
	clk_val = readl(CPM_BASE_M  + CPM_BSCCDR);
	clk_val |= ((2 << 30) | (1 << 29) | (1 <<0));
	writel(clk_val, CPM_BASE_M  + CPM_BSCCDR);

	clk_gate = readl(CPM_BASE_M  + CPM_CLKGR0);
	clk_gate &= ~(1 << 2);
	writel(clk_gate, CPM_BASE_M  + CPM_CLKGR0);

	/* start set lzma */

	value = readl(BSCALER_BASE + LZMA_CTRL);
	value |= 1<<2;
	writel(1 << 2, BSCALER_BASE + LZMA_CTRL);
	value = readl(BSCALER_BASE + LZMA_CTRL);
	while(value & 0x04) {
		value = readl(BSCALER_BASE + LZMA_CTRL);
	}

	/* set lzma mode */
	value = readl(BSCALER_BASE + LZMA_CTRL);

	while(!(value & (1 << 31))) {
		value |= 1 << 31;
		writel(value, BSCALER_BASE + LZMA_CTRL);
		value = readl(BSCALER_BASE + LZMA_CTRL);
	};

	/* reset lzma */
	writel(1 << 2, BSCALER_BASE + LZMA_CTRL);
	while(readl(BSCALER_BASE + LZMA_CTRL) & (1 << 2));

	/* init ram */
	writel(1 << 1 ,BSCALER_BASE + LZMA_CTRL);
	while(readl(BSCALER_BASE + LZMA_CTRL) & (1 << 1));

	/* config */
	in_file = __get_free_pages(GFP_KERNEL, 11);//10 == 1024 == 4M ,if page_size=4096bytes,11==8M
	if (in_file <= 0){
		printk("input file space request failed!\n");
		ret = -ENOMEM;
		return -1;
	}
	memset((void*)in_file, 0xff, 1024*1024*8);
	out_file = __get_free_pages(GFP_KERNEL, 12);   //10 == 1024 == 4M ,if page_size=4096bytes,12==16M
	if (out_file <= 0){
		printk("input file space request failed!\n");
		ret = -ENOMEM;
		return -1;
	}
	memset((void*)out_file, 0xff, 1024*1024*16);
	memmove((void *)in_file,(void* )src,size);
	in_reg = virt_to_phys((void *)in_file );
	out_reg = virt_to_phys((void*)out_file);
	fs = get_fs();
	set_fs(KERNEL_DS);

	dma_cache_sync(NULL, (void *)in_file, 1*1024*1024, DMA_BIDIRECTIONAL);
	memmove((void *)in_file,(void *)src,size);
	dma_cache_sync(NULL, (void*)in_file, 1*1024*1024, DMA_BIDIRECTIONAL);
	set_fs(fs);

	writel(in_reg , BSCALER_BASE + LZMA_BS_BASE);
	writel(size, BSCALER_BASE + LZMA_BS_SIZE);
	writel(out_reg , BSCALER_BASE + LZMA_DST_BASE);

	/* close intr and start lzma*/
	writel((1 << 0) | (1 << 3), BSCALER_BASE + LZMA_CTRL);

	/* wait end */
	while(readl(BSCALER_BASE + LZMA_CTRL) & (1 << 0));
	outlen = readl(BSCALER_BASE + LZMA_FINAL);

	memmove((void *)dst,(void *)out_file,outlen);


	free_pages(in_file, 11);;
	free_pages(out_file, 12);
	return outlen;
}
EXPORT_SYMBOL(jz_lzma_decompress);
