/*
 * kernel/drivers/video/jz_fb_v14/jz_fb.c
 *
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * Core file for Ingenic Display Controller driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/suspend.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/gpio.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>
#include <mach/platform.h>
#include <soc/gpio.h>
#include "jz_fb.h"
#include "dpu_reg.h"
//#include "rgb32_1024_600.h"
//#include "800_480_RGB_888.h"
//#include "rgb888_1024_600.h"

//#define CONFIG_BSP_CHIP_T40_FPGA_SLCD_TRULY_240_240
//#define CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
#define CONFIG_BSP_CHIP_T40_FPGA_TFT_EK79007
//#define CONFIG_BSP_CHIP_T40_FPGA_TFT_KD050HDFIA019

#define CONFIG_DSI_DPI_DEBUG 0

#ifdef CONFIG_MIPI_TX
#include "jz_mipi_dsi/jz_mipi_dsi_regs.h"
#include "jz_mipi_dsi/jz_mipi_dsih_hal.h"
#include "jz_mipi_dsi/jz_mipi_dsi_lowlevel.h"

extern int jz_dsi_video_cfg(struct dsi_device *dsi);
extern void jz_dsi_init(struct dsi_device *dsi);
extern void jz_dsi_phy_open(struct dsi_device *dsi);
extern void jz_dsi_phy_cfg(struct dsi_device *dsi);

struct dsi_device *dsi;
#endif


#define NR_FRAMES	3


#ifdef CONFIG_BSP_CHIP_T40_FPGA_SLCD_TRULY_240_240
#define WIDTH	240
#define	HEIGHT	240
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
#define WIDTH	800
#define	HEIGHT	480
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_EK79007
#define WIDTH	1024
#define	HEIGHT	600
//#define WIDTH	400
//#define	HEIGHT	400
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_LG4591
#define WIDTH	720
#define	HEIGHT	1280
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_OTM8012
#define WIDTH	480
#define	HEIGHT	854
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_KD050HDFIA019
#define WIDTH	480
#define	HEIGHT	854
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_SLCD_AUO163
#define WIDTH	320
#define	HEIGHT	320
#endif



unsigned int data_buf0[WIDTH * HEIGHT];
unsigned int data_buf1[WIDTH * HEIGHT];
unsigned int data_buf2[WIDTH * HEIGHT];
unsigned int data_buf3[WIDTH * HEIGHT];
unsigned int w_buf[WIDTH * HEIGHT] __attribute__((__aligned__(16)));
unsigned int r_buf[WIDTH * HEIGHT] __attribute__((__aligned__(16)));
unsigned int dither_buf[WIDTH * HEIGHT] __attribute__((__aligned__(16)));
unsigned char convert_buf0[WIDTH * HEIGHT * 4] __attribute__((__aligned__(8)));
unsigned char convert_buf1[WIDTH * HEIGHT * 4] __attribute__((__aligned__(8)));
unsigned char convert_buf2[WIDTH * HEIGHT * 4] __attribute__((__aligned__(8)));
unsigned char convert_buf3[WIDTH * HEIGHT * 4] __attribute__((__aligned__(8)));
unsigned char *tlb_buf0;
unsigned char *tlb_buf1;
unsigned char *tlb_buf2;
unsigned char *tlb_buf3;



static void init_data_buf_rgb888(unsigned int *buf)
{
	int i, j, tmp;
#if 1
	tmp = WIDTH / 6;
	for(j = 0; j < HEIGHT; j++){
		for(i = 0 * tmp; i < 1 * tmp; i++){
			buf[i + j * WIDTH] = 0xffffff;
		}
		for(i = 1 * tmp; i < 2 * tmp; i++){
			buf[i + j * WIDTH] = 0x00fc00;
		}
		for(i = 2 * tmp; i < 3 * tmp; i++){
			buf[i + j * WIDTH] = 0x0000fe;
		}
		for(i = 3 * tmp; i < 4 * tmp; i++){
			buf[i + j * WIDTH] = 0xf30000;
		}
		for(i = 4 * tmp; i < 5 * tmp; i++){
			buf[i + j * WIDTH] = 0x00fd00;
		}
		for(i = 5 * tmp; i < 6 * tmp; i++){
			buf[i + j * WIDTH] = 0x000000;
		}
	}
#else
	unsigned int w = WIDTH;
	unsigned int h = HEIGHT / 10;
	for (i = 0; i < w * h; i++) {
		buf[i] = 0x00ff00;
	}
	for (i = w * h; i < w * h * 2; i++) {
		buf[i] = 0xff0000;
	}
	for (i = w * h * 2; i < w * h * 3; i++) {
		buf[i] = 0x00ff00;
	}
	for (i = w * h * 3; i < w * h * 4; i++) {
		buf[i] = 0x0000ff;
	}
	for (i = w * h * 4; i < w * h * 5; i++) {
		buf[i] = 0xffffff;
	}
	for (i = w * h * 5; i < w * h * 6; i++) {
		buf[i] = 0xffff00;
	}
	for (i = w * h * 6; i < w * h * 7; i++) {
		buf[i] = 0xff0000;
	}
	for (i = w * h * 7; i < w * h * 8; i++) {
		buf[i] = 0x00ff00;
	}
	for (i = w * h * 8; i < w * h * 9; i++) {
		buf[i] = 0x0000ff;
	}
	for (i = w * h * 9; i < w * h * 10; i++) {
		buf[i] = 0xffffff;
	}
	for (i = w * h *10; i < HEIGHT; i++) {
		buf[i] = 0xffffff;
	}
#endif
	dma_cache_wback_inv((unsigned long)buf, WIDTH * HEIGHT * 4);
}

static unsigned short rgb888_to_rgb565(unsigned int rgb888)
{
	unsigned short n565Color = 0;

	unsigned char cRed   = (rgb888 & 0x00ff0000) >> 19;
	unsigned char cGreen = (rgb888 & 0x0000ff00) >> 10;
	unsigned char cBlue  = (rgb888 & 0x000000ff) >> 3;

	n565Color = (cRed << 11) + (cGreen << 5) + (cBlue << 0);
	return n565Color;
}


static void init_data_buf_rgb565(unsigned short *buf)
{
	int i;
	unsigned int w = WIDTH;
	unsigned int h = HEIGHT / 10;

	for(i = 0; i < w * h; i++) {
		buf[i] = rgb888_to_rgb565(0xffff00);
		//buf[i] = (0xff << 11) | (0xff << 5) | (0xff << 0);
	}
	for(i = w * h; i < w * h * 2; i++) {
		buf[i] = rgb888_to_rgb565(0xff0000);
		//buf[i] = (0xff << 11) | (0xff << 5) | (0x00 << 0);
	}
	for(i = w * h * 2; i < w * h * 3; i++) {
		buf[i] = rgb888_to_rgb565(0x00ff00);
		//buf[i] = (0xff << 11) | (0x00 << 5) | (0x00 << 0);
	}
	for(i = w * h * 3; i < w * h * 4; i++) {
		buf[i] = rgb888_to_rgb565(0x0000ff);
		//buf[i] = (0x00 << 11) | (0xff << 5) | (0x00 << 0);
	}
	for(i = w * h * 4; i < w * h * 5; i++) {
		buf[i] = rgb888_to_rgb565(0xffffff);
		//buf[i] = (0x00 << 11) | (0x00 << 5) | (0xff << 0);
	}
	for(i = w * h * 5; i < w * h * 6; i++) {
		buf[i] = rgb888_to_rgb565(0xffff00);
		//buf[i] = (0xff << 11) | (0xff << 5) | (0x00 << 0);
	}
	for(i = w * h * 6; i < w * h * 7; i++) {
		buf[i] = rgb888_to_rgb565(0xff0000);
		//buf[i] = (0xff << 11) | (0xff << 5) | (0xff << 0);
	}
	for(i = w * h * 7; i < w * h * 8; i++) {
		buf[i] = rgb888_to_rgb565(0x00ff00);
		//buf[i] = (0xff << 11) | (0x00 << 5) | (0x00 << 0);
	}
	for(i = w * h * 8; i < w * h * 9; i++) {
		buf[i] = rgb888_to_rgb565(0x0000ff);
		//buf[i] = (0x00 << 11) | (0xff << 5) | (0x00 << 0);
	}
	for(i = w * h * 9; i < w * h * 10; i++) {
		buf[i] = rgb888_to_rgb565(0xffffff);
		//buf[i] = (0x00 << 11) | (0x00 << 5) | (0xff << 0);
	}

	dma_cache_wback_inv((unsigned long)buf, WIDTH * HEIGHT * 4);
}


static void init_data_buf_dither(unsigned int *buf)
{
	int i;
	unsigned int w = WIDTH;
	unsigned int h = HEIGHT / 3;


	for(i = 0; i < w * h; i++) {
		buf[i] = 0xff0000;
	}
	for(i = w * h; i < w * h * 2; i++) {
		buf[i] = 0x00ff00;
	}
	for(i = w * h * 2; i < w * h * 3; i++) {
		buf[i] = 0x0000ff;
	}

	dma_cache_wback_inv((unsigned long)buf, WIDTH * HEIGHT * 4);
}



/****************************** buffer format convert ******************************************/

#define CSC0toY(R, G, B) ((299*(R)+587*(G)+114*(B))/1000)
#define CSC0toU(R, G, B) ((-147*(R)-289*(G)+436*(B))/1000)
#define CSC0toV(R, G, B) ((615*(R)-515*(G)-100*(B))/1000)

#define CSC1toY(R, G, B) ((257*(R)+504*(G)+98*(B) + 16000)/1000)
#define CSC1toU(R, G, B) ((-148*(R)-291*(G)+439*(B)+128000)/1000)
#define CSC1toV(R, G, B) ((439*(R)-368*(G)-71*(B)+128000)/1000)

#define CSC2toY(R,G,B) ((299*(R)+587*(G)+114*(B))/1000)
#define CSC2toU(R,G,B) ((-1687*(R)-3313*(G)+5000*(B)+1280000)/10000)
#define CSC2toV(R,G,B) ((5000*(R)-4187*(G)-813*(B)+1280000)/10000)

#define CSC3toY(R,G,B)	((299*(R)+587*(G)+114*(B))/1000)
#define CSC3toU(R,G,B)	(0)
#define CSC3toV(R,G,B)	(0)

#define CSCtoY(mode, R, G, B)	CSC##mode##toY(R, G, B)
#define CSCtoU(mode, R, G, B)	CSC##mode##toU(R, G, B)
#define CSCtoV(mode, R, G, B)	CSC##mode##toV(R, G, B)

static int toY(unsigned int mode, int r, int g, int b)
{
	switch (mode) {
		case 0:
			return CSCtoY(0, r, g, b);
		case 1:
			return CSCtoY(1, r, g, b);
		case 2:
			return CSCtoY(2, r, g, b);
		case 3:
			return CSCtoY(3, r, g, b);
	}
	return -1;
}

static int toU(unsigned int mode, int r, int g, int b)
{
	switch (mode) {
		case 0:
			return CSCtoU(0, r, g, b);
		case 1:
			return CSCtoU(1, r, g, b);
		case 2:
			return CSCtoU(2, r, g, b);
		case 3:
			return CSCtoU(3, r, g, b);
	}
	return -1;
}

static int toV(unsigned int mode, int r, int g, int b)
{
	switch (mode) {
		case 0:
			return CSCtoV(0, r, g, b);
		case 1:
			return CSCtoV(1, r, g, b);
		case 2:
			return CSCtoV(2, r, g, b);
		case 3:
			return CSCtoV(3, r, g, b);
	}
	return -1;
}

static void rgb888_to_nv12(unsigned int *src_buf, unsigned char *dst_buf, unsigned int width, unsigned int height, unsigned int mode)
{
	int w = 0, h = 0;
	int y = 0, u = 0, v = 0;
	int r, g, b;
	int i = 0;


	for (h = 0; h < height; h++) {
		for (w = 0; w < width; w++) {
			r = (src_buf[h * width + w] & 0xff0000) >> 16;
			g = (src_buf[h * width + w] & 0x00ff00) >> 8;
			b = (src_buf[h * width + w] & 0x0000ff) >> 0;

			y = toY(mode, r, g, b);
			dst_buf[h * width + w] = y;

			if ((!(h % 2)) && (!(w % 2))) {
				u = toU(mode, r, g, b);
				v = toV(mode, r, g, b);

				dst_buf[width * height + i] = u;
				dst_buf[width * height + i + 1] = v;

				i += 2;
			}
		}
	}
	printk("yuv = %d %d %d\n", y, u, v);
	dma_cache_wback_inv((unsigned long)dst_buf, WIDTH * HEIGHT * 4);
}

static void rgb888_to_nv21(unsigned int *src_buf, unsigned char *dst_buf, unsigned int width, unsigned int height, unsigned int mode)
{
	int w = 0, h = 0;
	int r, g, b;
	int y, u, v;
	int i = 0;


	for (h = 0; h < height; h++) {
		for (w = 0; w < width; w++) {
			r = (src_buf[h * width + w] & 0xff0000) >> 16;
			g = (src_buf[h * width + w] & 0x00ff00) >> 8;
			b = (src_buf[h * width + w] & 0x0000ff) >> 0;

			y = toY(mode, r, g, b);
			dst_buf[h * width + w] = y;

			if ((!(h % 2)) && (!(w % 2))) {
				u = toU(mode, r, g, b);
				v = toV(mode, r, g, b);

				dst_buf[width * height + i] = v;
				dst_buf[width * height + i + 1] = u;

				i += 2;
			}
		}
	}
	dma_cache_wback_inv((unsigned long)dst_buf, WIDTH * HEIGHT * 4);
}

/****************************** buffer format convert ******************************************/


static void dump_dpu_reg(void)
{
	int i = 0;

	printk("FRM_CFG_ADDR	= 0x%08x\n", *(volatile unsigned int *)0xb3050000);
	printk("FRM_CFG_CTRL 	= 0x%08x\n", *(volatile unsigned int *)0xb3050004);
	printk("SRD_CHAIN_ADDR	= 0x%08x\n", *(volatile unsigned int *)0xb3051000);
	printk("SRD_CHAIN_START	= 0x%08x\n", *(volatile unsigned int *)0xb3051004);
//	printk("CTRL         	= 0x%08x\n", *(volatile unsigned int *)0xb3052000);
	printk("ST	     	= 0x%08x\n", *(volatile unsigned int *)0xb3052004);
	printk("INTC	     	= 0x%08x\n", *(volatile unsigned int *)0xb305200c);
	printk("COM_CFG	     	= 0x%08x\n", *(volatile unsigned int *)0xb3052014);
//	printk("PCFG_RD_CTRL 	= 0x%08x\n", *(volatile unsigned int *)0xb3052018);

	printk("DISP_COM	= 0x%08x\n", *(volatile unsigned int *)0xb3058000);
	printk("SLCD_CFG	= 0x%08x\n", *(volatile unsigned int *)0xb305a000);
	printk("SLCD_WR_DUTY	= 0x%08x\n", *(volatile unsigned int *)0xb305a004);
	printk("SLCD_TIMING	= 0x%08x\n", *(volatile unsigned int *)0xb305a005);
	printk("SLCD_FRM_SIZE	= 0x%08x\n", *(volatile unsigned int *)0xb305a00c);
	printk("SLCD_SLOW_TIME	= 0x%08x\n", *(volatile unsigned int *)0xb305a010);
	printk("SLCD_REG_IF     = 0x%08x\n", *(volatile unsigned int *)0xb305a014);
	printk("SLCD_ST		= 0x%08x\n", *(volatile unsigned int *)0xb305a018);
	printk("SLCD_REG_CTRL   = 0x%08x\n", *(volatile unsigned int *)0xb305a01c);

	printk("TFT_HSYNC   = 0x%08x\n", *(volatile unsigned int *)0xb3059000);
	printk("TFT_VSYNC   = 0x%08x\n", *(volatile unsigned int *)0xb3059004);
	printk("TFT_HDE	    = 0x%08x\n", *(volatile unsigned int *)0xb3059008);
	printk("TFT_VDE	    = 0x%08x\n", *(volatile unsigned int *)0xb305900c);
	printk("TFT_TRAN_CFG= 0x%08x\n", *(volatile unsigned int *)0xb3059010);
	printk("TFT_ST	    = 0x%08x\n", *(volatile unsigned int *)0xb3059014);

	*(volatile unsigned int *)0xb3052000 |= (1 << 2);
	for (i = 0; i < 11; i++) {
		printk("frame_desc[%d] = 0x%x\n", i, *(volatile unsigned int*)0xb3052100);
	}
	for (i = 0; i < 12; i++) {
		printk("layer0_desc[%d] = 0x%x\n", i, *(volatile unsigned int*)0xb3052104);
	}
	for (i = 0; i < 12; i++) {
		printk("layer1_desc[%d] = 0x%x\n", i, *(volatile unsigned int*)0xb3052108);
	}
	for (i = 0; i < 12; i++) {
		printk("layer2_desc[%d] = 0x%x\n", i, *(volatile unsigned int*)0xb305210c);
	}
	for (i = 0; i < 12; i++) {
		printk("layer3_desc[%d] = 0x%x\n", i, *(volatile unsigned int*)0xb3052110);
	}
	for (i = 0; i < 5; i++) {
		printk("rdma_desc[%d] = 0x%x\n", i, *(volatile unsigned int*)0xb3052114);
	}

	printk("frm_site	= 0x%08x\n", *(volatile unsigned int *)0xb3052200);
	printk("layer0_site	= 0x%08x\n", *(volatile unsigned int *)0xb3053100);
	printk("layer1_site	= 0x%08x\n", *(volatile unsigned int *)0xb3053104);
	printk("layer2_site	= 0x%08x\n", *(volatile unsigned int *)0xb3053108);
	printk("layer3_site	= 0x%08x\n", *(volatile unsigned int *)0xb305310c);


}

static void dump_framedesc(struct jzfb *jzfb)
{
	int i;
	struct jzfb_framedesc *framedesc;

	for (i = 0; i < NR_FRAMES; i++) {
		framedesc = jzfb->framedesc[i];
		dma_cache_wback_inv((unsigned long)jzfb->framedesc[i], sizeof(struct jzfb_framedesc));

		printk("FrameNextCfgAddr  = 0x%x\n", framedesc->FrameNextCfgAddr);
		printk("FrameSize	  = 0x%x\n", framedesc->FrameSize);
		printk("FrameCtrl         = 0x%x\n", framedesc->FrameCtrl);
		printk("WritebackAddr     = 0x%x\n", framedesc->WritebackAddr);
		printk("WritebackStride   = 0x%x\n", framedesc->WritebackStride);
		printk("Layer0CfgAddr     = 0x%x\n", framedesc->Layer0CfgAddr);
		printk("Layer1CfgAddr     = 0x%x\n", framedesc->Layer1CfgAddr);
		printk("Layer2CfgAddr     = 0x%x\n", framedesc->Layer2CfgAddr);
		printk("Layer3CfgAddr     = 0x%x\n", framedesc->Layer3CfgAddr);
		printk("LayCfgEn          = 0x%x\n", framedesc->LayCfgEn);
		printk("InterruptControl  = 0x%x\n", framedesc->InterruptControl);
		printk("\n");

	}

}

#ifdef DEBUG
static void dump_layerdesc(struct jzfb *jzfb, unsigned int frame_num)
{
	struct jzfb_layerdesc *layer_desc;
	int i, j;

	for (j = 0; j < frame_num; j++) {
		for (i = 0; i < 4; i++) {
			layer_desc = jzfb->layerdesc[j][i];

			printk("layer_desc[%d][%d]->LayerSize        = 0x%x\n", j, i, layer_desc->LayerSize);
			printk("layer_desc[%d][%d]->LayerCfg         = 0x%x\n", j, i, layer_desc->LayerCfg);
			printk("layer_desc[%d][%d]->LayerBufferAddr  = 0x%x\n", j, i, layer_desc->LayerBufferAddr);
			printk("layer_desc[%d][%d]->LayerScale       = 0x%x\n", j, i, layer_desc->LayerScale);
			printk("layer_desc[%d][%d]->LayerPos         = 0x%x\n", j, i, layer_desc->LayerPos);
			printk("layer_desc[%d][%d]->LayerResizCoefX  = 0x%x\n", j, i, layer_desc->LayerResizCoefX);
			printk("layer_desc[%d][%d]->LayerResizCoefY  = 0x%x\n", j, i, layer_desc->LayerResizCoefY);
			printk("layer_desc[%d][%d]->LayerStride      = 0x%x\n", j, i, layer_desc->LayerStride);
			printk("layer_desc[%d][%d]->UVBufferAddr     = 0x%x\n", j, i, layer_desc->UVBufferAddr);
			printk("layer_desc[%d][%d]->UVStride         = 0x%x\n", j, i, layer_desc->UVStride);
			printk("\n");
		}
	}

}

static void dump_rdmadesc(struct jzfb *jzfb)
{
	struct jzfb_rdmadesc *rdmadesc;
	int i = 0;

	for (i = 0; i < NR_FRAMES; i++) {
		rdmadesc = jzfb->rdmadesc[i];

		printk("RdmaNextCfgAddr       =0x%x\n", rdmadesc->RdmaNextCfgAddr);
		printk("FrameBufferAddr       =0x%x\n", rdmadesc->FrameBufferAddr);
		printk("Stride                =0x%x\n", rdmadesc->Stride);
		printk("RdmaCfg		      =0x%x\n", rdmadesc->RdmaCfg);
		printk("RdmaInterruptControl  =0x%x\n", rdmadesc->RdmaInterruptControl);
		printk("\n");
	}

}
#endif

/***********************************************************************************************/

#ifdef CONFIG_BSP_CHIP_T40_FPGA_SLCD_TRULY_240_240

static struct smart_lcd_data_table truly_slcd240240_data_table[] = {
    /* LCD init code */
    {SMART_CONFIG_CMD, 0x01},		/* soft reset, 120 ms = 120 000 us */
    {SMART_CONFIG_UDELAY, 120000},
    {SMART_CONFIG_CMD, 0x11},
    {SMART_CONFIG_UDELAY, 5000},	/* sleep out 5 ms  */

    {SMART_CONFIG_CMD, 0x36},
#ifdef	CONFIG_TRULY_240X240_ROTATE_180
    /*{0x36, 0xc0, 2, 0}, //40*/
    {SMART_CONFIG_PRM, 0xd0}, //40
#else
    {SMART_CONFIG_PRM, 0x00}, //40
#endif

    {SMART_CONFIG_CMD, 0x2a},	//row
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0xef},

    {SMART_CONFIG_CMD, 0x2b},	//colume
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0xef},


    {SMART_CONFIG_CMD, 0x3a},	//rgb format
#if defined(CONFIG_SLCD_TRULY_18BIT)	/* if 18bit/pixel unusual. try to use 16bit/pixel */
    {SMART_CONFIG_PRM, 0x06}, //6-6-6
#else
    {SMART_CONFIG_PRM, 0x05}, //5-6-5
#endif
    //	{SMART_CONFIG_PRM, 0x55},

    {SMART_CONFIG_CMD, 0xb2},	//porch
    {SMART_CONFIG_PRM, 0x7f},
    {SMART_CONFIG_PRM, 0x7f},
    {SMART_CONFIG_PRM, 0x01},
    {SMART_CONFIG_PRM, 0xde},
    {SMART_CONFIG_PRM, 0x33},

    {SMART_CONFIG_CMD, 0xb3},	//frame rate
    {SMART_CONFIG_PRM, 0x10},
    {SMART_CONFIG_PRM, 0x05},
    {SMART_CONFIG_PRM, 0x0f},

    {SMART_CONFIG_CMD, 0xb4},	//???
    {SMART_CONFIG_PRM, 0x0b},

    {SMART_CONFIG_CMD, 0xb7},	//gate control
    {SMART_CONFIG_PRM, 0x35},

    {SMART_CONFIG_CMD, 0xbb},	//VCOM setting
    {SMART_CONFIG_PRM, 0x28}, //23

    {SMART_CONFIG_CMD, 0xbc},	//???
    {SMART_CONFIG_PRM, 0xec},

    {SMART_CONFIG_CMD, 0xc0},	//LCM control
    {SMART_CONFIG_PRM, 0x2c},

    {SMART_CONFIG_CMD, 0xc2},	//VDV and VRH command enable
    {SMART_CONFIG_PRM, 0x01},

    {SMART_CONFIG_CMD, 0xc3},	//VRH
    {SMART_CONFIG_PRM, 0x1e}, //14

    {SMART_CONFIG_CMD, 0xc4},	//VDV
    {SMART_CONFIG_PRM, 0x20},

    {SMART_CONFIG_CMD, 0xc6},	//FR control
    /*{SMART_CONFIG_PRM, 0x14},*/
    {SMART_CONFIG_PRM, 0x1F},

    {SMART_CONFIG_CMD, 0xd0},	//power control
    {SMART_CONFIG_PRM, 0xa4},
    {SMART_CONFIG_PRM, 0xa1},

    {SMART_CONFIG_CMD, 0xe0},	//positive voltage
    {SMART_CONFIG_PRM, 0xd0},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x08},
    {SMART_CONFIG_PRM, 0x07},
    {SMART_CONFIG_PRM, 0x05},
    {SMART_CONFIG_PRM, 0x29},
    {SMART_CONFIG_PRM, 0x54},
    {SMART_CONFIG_PRM, 0x41},
    {SMART_CONFIG_PRM, 0x3c},
    {SMART_CONFIG_PRM, 0x17},
    {SMART_CONFIG_PRM, 0x15},
    {SMART_CONFIG_PRM, 0x1a},
    {SMART_CONFIG_PRM, 0x20},

    {SMART_CONFIG_CMD, 0xe1},	//negative voltage
    {SMART_CONFIG_PRM, 0xd0},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x00},
    {SMART_CONFIG_PRM, 0x08},
    {SMART_CONFIG_PRM, 0x07},
    {SMART_CONFIG_PRM, 0x04},
    {SMART_CONFIG_PRM, 0x29},
    {SMART_CONFIG_PRM, 0x44},
    {SMART_CONFIG_PRM, 0x42},
    {SMART_CONFIG_PRM, 0x3b},
    {SMART_CONFIG_PRM, 0x16},
    {SMART_CONFIG_PRM, 0x15},
    {SMART_CONFIG_PRM, 0x1b},
    {SMART_CONFIG_PRM, 0x1f},

    {SMART_CONFIG_CMD, 0x35}, // TE on
    {SMART_CONFIG_PRM, 0x00}, // TE mode: 0, mode1; 1, mode2
    //	{SMART_CONFIG_CMD, 0x34}, // TE off

    /* {SMART_CONFIG_CMD, 0x30}, */
    /* {SMART_CONFIG_PRM, 0}, */
    /* {SMART_CONFIG_PRM, 0x30}, */
    /* {SMART_CONFIG_PRM, 0}, */
    /* {SMART_CONFIG_PRM, 0xc0}, */
    /* {SMART_CONFIG_CMD, 0x12}, */


    {SMART_CONFIG_CMD, 0x29}, //Display ON

    /* set window size*/
    //	{SMART_CONFIG_CMD, 0xcd},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_PRM, 0},
    {SMART_CONFIG_PRM, 0},
    {SMART_CONFIG_PRM, (239>> 8) & 0xff},
    {SMART_CONFIG_PRM, 239 & 0xff},
#ifdef	CONFIG_TRULY_240X240_ROTATE_180
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_PRM, ((320-240)>>8)&0xff},
    {SMART_CONFIG_PRM, ((320-240)>>0)&0xff},
    {SMART_CONFIG_PRM, ((320-1)>>8) & 0xff},
    {SMART_CONFIG_PRM, ((320-1)>>0) & 0xff},
#else
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_PRM, 0},
    {SMART_CONFIG_PRM, 0},
    {SMART_CONFIG_PRM, (239>> 8) & 0xff},
    {SMART_CONFIG_PRM, 239 & 0xff},
#endif
    {SMART_CONFIG_CMD, 0x2C},
    {SMART_CONFIG_CMD, 0x2C},
    {SMART_CONFIG_CMD, 0x2C},
    {SMART_CONFIG_CMD, 0x2C},
};

#endif

#ifdef CONFIG_SLCD

static void slcd_reset()
{
	/* RST PC(19)  1 ---> 0 ---> 1 */

	/* RST output 1 */
	*(volatile unsigned int *)0xb0012018 |= (1 << 19);
	*(volatile unsigned int *)0xb0012024 |= (1 << 19);
	*(volatile unsigned int *)0xb0012038 |= (1 << 19);
	*(volatile unsigned int *)0xb0012044 |= (1 << 19);

	msleep(1);

	/* RST output 0 */
	*(volatile unsigned int *)0xb0012018 |= (1 << 19);
	*(volatile unsigned int *)0xb0012024 |= (1 << 19);
	*(volatile unsigned int *)0xb0012038 |= (1 << 19);
	*(volatile unsigned int *)0xb0012048 |= (1 << 19);

	msleep(1);

	/* RST output 1 */
	*(volatile unsigned int *)0xb0012018 |= (1 << 19);
	*(volatile unsigned int *)0xb0012024 |= (1 << 19);
	*(volatile unsigned int *)0xb0012038 |= (1 << 19);
	*(volatile unsigned int *)0xb0012044 |= (1 << 19);

	msleep(5);
}

static void slcd_poweron()
{
	/** BL  PC(18)
	 *  PWM PD(30)
	 *  RD  PD(12)
	 *  CS  PD(10)
	 *  RST PC(19)
	 **/

	/* BL output 1 */
	*(volatile unsigned int *)0xb0012018 |= (1 << 18);
	*(volatile unsigned int *)0xb0012024 |= (1 << 18);
	*(volatile unsigned int *)0xb0012038 |= (1 << 18);
	*(volatile unsigned int *)0xb0012044 |= (1 << 18);

	/* PWM output 1 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 30);
	*(volatile unsigned int *)0xb0013024 |= (1 << 30);
	*(volatile unsigned int *)0xb0013038 |= (1 << 30);
	*(volatile unsigned int *)0xb0013044 |= (1 << 30);

	/* RD output 1 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 12);
	*(volatile unsigned int *)0xb0013024 |= (1 << 12);
	*(volatile unsigned int *)0xb0013038 |= (1 << 12);
	*(volatile unsigned int *)0xb0013044 |= (1 << 12);

	/* CS output 1 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 10);
	*(volatile unsigned int *)0xb0013024 |= (1 << 10);
	*(volatile unsigned int *)0xb0013038 |= (1 << 10);
	*(volatile unsigned int *)0xb0013044 |= (1 << 10);

	slcd_reset();

	/* CS output 0 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 10);
	*(volatile unsigned int *)0xb0013024 |= (1 << 10);
	*(volatile unsigned int *)0xb0013038 |= (1 << 10);
	*(volatile unsigned int *)0xb0013048 |= (1 << 10);
}

static void init_dpu_slcd()
{
	printk("***************SLCD INIT***************\n");
	*(volatile unsigned int *)0xb305a000 = (2 << 23) | (0 << 21) | (1 << 20) | (1 << 18) | (1 << 10) | (1 << 8) | (1 << 6) | (0 << 3) | (0 << 0);

	*(volatile unsigned int *)0xb305a00c = (HEIGHT << 16) | WIDTH;

#ifdef	CONFIG_BSP_CHIP_T40_FPGA_SLCD_AUO163
//	*(volatile unsigned int *)0xb305a010 = 0xf;
	*(volatile unsigned int *)0xb305a000 &= ~(3 << 21);
	*(volatile unsigned int *)0xb305a000 |= (2 << 21);
//	*(volatile unsigned int *)0xb305a000 &= ~(1 << 8);
	*(volatile unsigned int *)0xb305a000 &= ~(1 << 18);
	*(volatile unsigned int *)0xb305a000 |= (4 << 0) | (4 << 3);
#endif
}
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_SLCD_TRULY_240_240
static void slcd_mcu_init(struct smart_lcd_data_table *data_table)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(truly_slcd240240_data_table); i++) {
		if (data_table[i].type == SMART_CONFIG_DATA)
			*(volatile unsigned int *)0xb305a014 = (0 << 30) | data_table[i].value;
		if (data_table[i].type == SMART_CONFIG_PRM)
			*(volatile unsigned int *)0xb305a014 = (1 << 30) | data_table[i].value;
		if (data_table[i].type == SMART_CONFIG_CMD)
			*(volatile unsigned int *)0xb305a014 = (2 << 30) | data_table[i].value;
		if (data_table[i].type == SMART_CONFIG_UDELAY)
			udelay(data_table[i].value);
		while (*(volatile unsigned int *)0xb340a018 & 0x1);	//wait slcd busy
	}
	*(volatile unsigned int *)0xb305a014 |= (1 << 29);	//the last content

}
#endif

/***********************************************************************************************/

#ifdef CONFIG_TFT

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
#define HSYNC	128
#define	VSYNC	2
#define XDISPLAY	800
#define YDISPLAY	480
#define	H_LEFT		88
#define H_RIGHT		40
#define V_UP		8
#define	V_DOWN		35
#endif

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_EK79007
#ifdef CONFIG_MIPI_2LANE
#define HSYNC	10
#define	VSYNC	2
#define XDISPLAY	1024
#define YDISPLAY	600
#define	H_LEFT		60
#define H_RIGHT		60
#define V_UP		5
#define	V_DOWN		10
#endif

#ifdef CONFIG_MIPI_4LANE
#define HSYNC	10
#define	VSYNC	1
#define XDISPLAY	1024
#define YDISPLAY	600
#define	H_LEFT		160
#define H_RIGHT		160
#define V_UP		23
#define	V_DOWN		12
#endif
#endif

#ifdef	CONFIG_BSP_CHIP_T40_FPGA_TFT_LG4591
#define HSYNC	20
#define	VSYNC	20
#define XDISPLAY	720
#define YDISPLAY	1280
#define	H_LEFT		10
#define H_RIGHT		10
#define V_UP		10
#define	V_DOWN		10
#endif

#ifdef	CONFIG_BSP_CHIP_T40_FPGA_TFT_OTM8012
#define HSYNC	2
#define	VSYNC	2
#define XDISPLAY	480
#define YDISPLAY	854
#define	H_LEFT		6
#define H_RIGHT		6
#define V_UP		12
#define	V_DOWN		12
#endif

#ifdef	CONFIG_BSP_CHIP_T40_FPGA_TFT_KD050HDFIA019
#define HSYNC	8
#define	VSYNC	10
#define XDISPLAY	480
#define YDISPLAY	854
#define	H_LEFT		40
#define H_RIGHT		50
#define V_UP		16
#define	V_DOWN		4
#endif

#ifdef CONFIG_TFT
static void tft_reset(void)
{
#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
	/* RST PD(23)  1 ---> 0 ---> 1 */
	/* RST output 1 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 23);
	*(volatile unsigned int *)0xb0013024 |= (1 << 23);
	*(volatile unsigned int *)0xb0013038 |= (1 << 23);
	*(volatile unsigned int *)0xb0013044 |= (1 << 23);

	msleep(1);

	/* RST output 0 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 23);
	*(volatile unsigned int *)0xb0013024 |= (1 << 23);
	*(volatile unsigned int *)0xb0013038 |= (1 << 23);
	*(volatile unsigned int *)0xb0013048 |= (1 << 23);

	msleep(1);

	/* RST output 1 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 23);
	*(volatile unsigned int *)0xb0013024 |= (1 << 23);
	*(volatile unsigned int *)0xb0013038 |= (1 << 23);
	*(volatile unsigned int *)0xb0013044 |= (1 << 23);

	msleep(5);
#endif /* CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766 */

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_EK79007
	/* RST PB05*/
	/* RST output 1 */
	*(volatile unsigned int *)0xb0011018 |= (1 << 5);
	*(volatile unsigned int *)0xb0011024 |= (1 << 5);
	*(volatile unsigned int *)0xb0011038 |= (1 << 5);
	*(volatile unsigned int *)0xb0011044 |= (1 << 5);
	msleep(1);

	/* RST output 0 */
	*(volatile unsigned int *)0xb0011018 |= (1 << 5);
	*(volatile unsigned int *)0xb0011024 |= (1 << 5);
	*(volatile unsigned int *)0xb0011038 |= (1 << 5);
	*(volatile unsigned int *)0xb0011048 |= (1 << 5);
	msleep(1);

	/* RST output 1 */
	*(volatile unsigned int *)0xb0011018 |= (1 << 5);
	*(volatile unsigned int *)0xb0011024 |= (1 << 5);
	*(volatile unsigned int *)0xb0011038 |= (1 << 5);
	*(volatile unsigned int *)0xb0011044 |= (1 << 5);
	msleep(5);
#endif
}

static void tft_poweron(void)
{
#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
	/* BL_EN PD25 */
	*(volatile unsigned int *)0xb0013018 |= (1 << 25);
	*(volatile unsigned int *)0xb0013024 |= (1 << 25);
	*(volatile unsigned int *)0xb0013038 |= (1 << 25);
	*(volatile unsigned int *)0xb0013044 |= (1 << 25);

	/* DISP  PD22*/
	*(volatile unsigned int *)0xb0013018 |= (1 << 22);
	*(volatile unsigned int *)0xb0013024 |= (1 << 22);
	*(volatile unsigned int *)0xb0013038 |= (1 << 22);
	*(volatile unsigned int *)0xb0013044 |= (1 << 22);
#endif /* CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766 */

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_EK79007
	/* DISP  PB04*/
	*(volatile unsigned int *)0xb0011018 |= (1 << 4);
	*(volatile unsigned int *)0xb0011024 |= (1 << 4);
	*(volatile unsigned int *)0xb0011038 |= (1 << 4);
	*(volatile unsigned int *)0xb0011044 |= (1 << 4);
#endif

	tft_reset();
}
#endif

static void init_dpu_tft(void)
{
	/* pulse area */
	unsigned int HPS = HSYNC;
	unsigned int HPE = HSYNC + H_LEFT + XDISPLAY + H_RIGHT;

	unsigned int VPS = VSYNC;
	unsigned int VPE = VSYNC + V_UP + YDISPLAY + V_DOWN;

	/* display area */
	unsigned int HDS = HSYNC + H_LEFT;
	unsigned int HDE = HSYNC + H_LEFT + XDISPLAY;

	unsigned int VDS = VSYNC + V_UP;
	unsigned int VDE = VSYNC + V_UP + YDISPLAY;


	*(volatile unsigned int *)0xb3059000 = (HPS << 16) | HPE;
	*(volatile unsigned int *)0xb3059004 = (VPS << 16) | VPE;

	*(volatile unsigned int *)0xb3059008 = (HDS << 16) | HDE;
	*(volatile unsigned int *)0xb305900c = (VDS << 16) | VDE;

#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
	/* output mode rgb666 */
	*(volatile unsigned int *)0xb3059010 = (0x0 << 19) | (0x0 << 16) | (0 << 10) | (0 << 9) | (0 << 8) | (0x1 << 0);
#else
	/* output mode rgb888 */
	*(volatile unsigned int *)0xb3059010 = (0x0 << 19) | (0x0 << 16) | (0 << 10) | (0 << 9) | (0 << 8) | (0x0 << 0);
#endif
	*(volatile unsigned int *)0xb3059010 |= (1 << 8);
	*(volatile unsigned int *)0xb3059010 |= (1 << 10);
}

#endif

/***********************************************************************************************/


static void lcd_enable_interrupt(void)
{
	*(volatile unsigned int *)0xb305200c = 0x8003ffc6;
}
static void lcd_disable_interrupt(void)
{
	*(volatile unsigned int *)0xb305200c = 0;

}
static void lcd_clear_interrupt(void)
{
	*(volatile unsigned int *)0xb3052008 = 0x8003ffc6;

}

static void init_layer_desc(struct jzfb *jzfb, unsigned int frame)
{
	struct jzfb_layerdesc *layer_desc;
#if 0
	unsigned int w = WIDTH / 2;
	unsigned int h = HEIGHT / 2;
	unsigned int x_pos = WIDTH / 2;
	unsigned int y_pos = HEIGHT / 2;
#else
	unsigned int w = WIDTH;
	unsigned int h = HEIGHT;
	unsigned int x_pos = 0;
	unsigned int y_pos = 0;
#endif
	layer_desc = jzfb->layerdesc[frame][0];
	layer_desc->LayerSize = (h << 16) | w;
	/*                      sharpl      format   premultiplied  galpha      color     alpha*/
	layer_desc->LayerCfg = (0 << 20) | (4 << 16) | (1 << 14) | (1 << 13) | (0 << 10) | 255;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerScale = 0;	//scale
	layer_desc->LayerPos = (0 << 16) | x_pos;
	layer_desc->LayerResizCoefX = 0;
	layer_desc->LayerResizCoefY = 0;
	layer_desc->LayerStride = WIDTH;
	//layer_desc->LayerStride = 800;
	layer_desc->UVBufferAddr = 0;
	layer_desc->UVStride = 0;
#if 0
	layer_desc = jzfb->layerdesc[frame][1];
	layer_desc->LayerSize = (h << 16) | w;
	/*                      sharpl      format   premultiplied  galpha      color     alpha*/
	layer_desc->LayerCfg = (1 << 20) | (4 << 16) | (1 << 14) | (1 << 13) | (0 << 10)| 255;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf1);
	layer_desc->LayerScale = 0;	//scale
	layer_desc->LayerPos = (0 << 16) | 0;
	layer_desc->LayerResizCoefX = 0;
	layer_desc->LayerResizCoefY = 0;
	layer_desc->LayerStride = WIDTH;
	layer_desc->UVBufferAddr = 0;
	layer_desc->UVStride = 0;

	layer_desc = jzfb->layerdesc[frame][2];
	layer_desc->LayerSize = (h << 16) | w;
	/*                      sharpl      format   premultiplied  galpha      color     alpha*/
	layer_desc->LayerCfg = (2 << 20) | (4 << 16) | (1 << 14) | (1 << 13) | (0 << 10)| 255;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf2);
	layer_desc->LayerScale = 0;	//scale
	layer_desc->LayerPos = (y_pos << 16) | 0;
	layer_desc->LayerResizCoefX = 0;
	layer_desc->LayerResizCoefY = 0;
	layer_desc->LayerStride = WIDTH;
	layer_desc->UVBufferAddr = 0;
	layer_desc->UVStride = 0;


	layer_desc = jzfb->layerdesc[frame][3];
	layer_desc->LayerSize = (h << 16) | w;
	/*                      sharpl      format   premultiplied  galpha      color     alpha*/
	layer_desc->LayerCfg = (3 << 20) | (2 << 16) | (1 << 14) | (1 << 13) | (0 << 10)| 255;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf3);
	layer_desc->LayerScale = 0;	//scale
	layer_desc->LayerPos = (y_pos << 16) | x_pos;
	layer_desc->LayerResizCoefX = 0;
	layer_desc->LayerResizCoefY = 0;
	layer_desc->LayerStride = WIDTH;
	layer_desc->UVBufferAddr = 0;
	layer_desc->UVStride = 0;
#endif
}


static void init_frame_desc(struct jzfb *jzfb, unsigned int frame_num)
{
	struct jzfb_framedesc *framedesc;
	int i = 0;

	for (i = 0; i < frame_num; i++) {
		framedesc = jzfb->framedesc[i];

		if (i == (frame_num - 1)) {
			framedesc->FrameNextCfgAddr = virt_to_phys((unsigned int *)jzfb->framedesc[0]);
		} else {
			framedesc->FrameNextCfgAddr = virt_to_phys((unsigned int *)jzfb->framedesc[i + 1]);
		}
		framedesc->FrameSize = (HEIGHT << 16) | WIDTH;
		framedesc->FrameCtrl = (3 << 24) | (2 << 22) | (1 << 20) | (1 << 16) | (1 << 5) | (1 << 4) | (0 << 3) | (1 << 2) | (0 << 1);
		if (i == (frame_num - 1)) {
			framedesc->FrameCtrl |= 1 ;
		} else {
			framedesc->FrameCtrl &= ~1 ;
		}
		framedesc->WritebackAddr = 0;
		framedesc->WritebackStride = 0;
		framedesc->Layer0CfgAddr = virt_to_phys((unsigned int *)jzfb->layerdesc[i][0]);
#if 0
		framedesc->Layer1CfgAddr = virt_to_phys((unsigned int *)jzfb->layerdesc[i][1]);
		framedesc->Layer2CfgAddr = virt_to_phys((unsigned int *)jzfb->layerdesc[i][2]);
		framedesc->Layer3CfgAddr = virt_to_phys((unsigned int *)jzfb->layerdesc[i][3]);
		framedesc->LayCfgEn = (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (0 << 8) | (1 << 10) | (2 << 12) | (3 << 14);
		framedesc->InterruptControl = (1 << 17) | (1 << 15) | (1 << 9) | (1 << 14);
#else
		/* only 1 laye */
		framedesc->LayCfgEn = (1 << 4) | (0 << 5) | (0 << 6) | (0 << 7) | (0 << 8) | (1 << 10) | (2 << 12) | (3 << 14);
		framedesc->InterruptControl = (1 << 17) | (1 << 15) | (1 << 9) | (1 << 14);
#endif
	}
}

static void init_rdma_desc(struct jzfb *jzfb, unsigned int frame_num, unsigned int *buf)
{
	struct jzfb_rdmadesc *rdmadesc;
	int i = 0;

	for (i = 0; i < frame_num; i++) {
		rdmadesc = jzfb->rdmadesc[i];
		if (i == (frame_num - 1)) {
			rdmadesc->RdmaNextCfgAddr = virt_to_phys((unsigned int *)(jzfb->rdmadesc[0]));
		} else {
			rdmadesc->RdmaNextCfgAddr = virt_to_phys((unsigned int *)(jzfb->rdmadesc[i + 1]));
		}

		rdmadesc->FrameBufferAddr = virt_to_phys((unsigned int *)buf);
		rdmadesc->Stride = WIDTH;
		rdmadesc->RdmaCfg = (4 << 19) | (0 << 16);

		if (i == (frame_num - 1)) {
			rdmadesc->RdmaCfg |= 1;
		} else {
			rdmadesc->RdmaCfg &= ~1;
		}

		rdmadesc->RdmaInterruptControl = (1 << 17) | (1 << 2) | (1 << 1);
	}

}


#define LAYER_RGB888	4
#define LAYER_NV12	8
#define LAYER_NV21	9
#define LAYER_RGB565	2
#define LAYER_YUV422	0xa

#define WB_RGB888	6
#define WB_RGB565	1

#define SR_RGB555	0
#define SR_RGB565	2
#define SR_RGB888	4

static void lcd_set_wb_format(struct jzfb *jzfb, unsigned int format, unsigned int frame_num)
{
	struct jzfb_framedesc *framedesc;

	framedesc = jzfb->framedesc[frame_num];

	framedesc->FrameCtrl &= ~(7 << 16);
	framedesc->FrameCtrl |= (format << 16);	//write back format RGB888

}

static void lcd_set_layer_format(struct jzfb *jzfb, unsigned int frame, unsigned int layer, unsigned int format)
{
	struct jzfb_layerdesc *layer_desc;

	layer_desc = jzfb->layerdesc[frame][layer];

	layer_desc->LayerCfg &= ~(0xf << 16);
	layer_desc->LayerCfg |= format << 16;
}
static void lcd_set_sr_format(struct jzfb *jzfb, unsigned int frame, unsigned int format)
{
	struct jzfb_rdmadesc *rdmadesc;

	rdmadesc = jzfb->rdmadesc[frame];

	rdmadesc->RdmaCfg &= ~(0xf << 19);
	rdmadesc->RdmaCfg |= (format << 19);

}


static void lcd_composer_config(struct jzfb *jzfb, unsigned int frame, unsigned int layer, unsigned int format)
{

	struct jzfb_layerdesc *layer_desc;

	layer_desc = jzfb->layerdesc[frame][layer];

	lcd_set_layer_format(jzfb, frame, layer, format);

	if (format == LAYER_RGB888) {

		layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf2);

	} else if (format == LAYER_NV12) {

		layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)convert_buf0);
		layer_desc->UVBufferAddr = virt_to_phys((unsigned int *)((unsigned int)convert_buf0 + WIDTH * HEIGHT) );
		layer_desc->UVStride = WIDTH;

	} else if (format == LAYER_NV21) {

		layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)convert_buf1);
		layer_desc->UVBufferAddr = virt_to_phys((unsigned int *)((unsigned int)convert_buf1 + WIDTH * HEIGHT) );
		layer_desc->UVStride = WIDTH;

	} else if (format == LAYER_YUV422) {

		layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)convert_buf2);
		layer_desc->UVBufferAddr = virt_to_phys((unsigned int *)((unsigned int)convert_buf2 + WIDTH * HEIGHT) );	//TODO
		layer_desc->UVStride = WIDTH;

	} else if (format == LAYER_RGB565) {

		layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf3);

	} else {
		printk("ERR : unsupport layer format\n");
	}
}

static void lcd_alpha_config(struct jzfb *jzfb, unsigned int frame)
{
	struct jzfb_layerdesc *layer_desc;

	unsigned int w = WIDTH / 3 * 2;
	unsigned int h = HEIGHT / 3 * 2;
	unsigned int x_pos = WIDTH - w;
	unsigned int y_pos = HEIGHT - h;

	layer_desc = jzfb->layerdesc[frame][0];
	layer_desc->LayerCfg &= ~ 0xffff;
	layer_desc->LayerCfg |= (1 << 14) | (1 << 13) | 230;
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerPos = (0 << 16) | x_pos;

	layer_desc = jzfb->layerdesc[frame][1];
	layer_desc->LayerCfg &= ~ 0xffff;
	layer_desc->LayerCfg |= (1 << 14) | (1 << 13) | 200;
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerPos = (0 << 16) | 0;

	layer_desc = jzfb->layerdesc[frame][2];
	layer_desc->LayerCfg &= ~ 0xffff;
	layer_desc->LayerCfg |= (1 << 14) | (1 << 13) | 150;
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerPos = (y_pos << 16) | 0;

	layer_desc = jzfb->layerdesc[frame][3];
	layer_desc->LayerCfg &= ~ 0xffff;
	layer_desc->LayerCfg |= (1 << 14) | (1 << 13) | 100;
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerPos = (y_pos << 16) | x_pos;

}

static void lcd_scaler_config(struct jzfb *jzfb, unsigned int frame, unsigned int layer, unsigned int height, unsigned int width)
{
	struct jzfb_framedesc *framedesc;
	struct jzfb_layerdesc *layer_desc;

	unsigned int src_w = WIDTH / 2;
	unsigned int src_h = HEIGHT / 2;

	framedesc = jzfb->framedesc[frame];

	framedesc->LayCfgEn |= (1 << layer); //layer scaleEn

	layer_desc = jzfb->layerdesc[frame][layer];

	layer_desc->LayerScale = (height << 16) | width;	//scale
	layer_desc->LayerResizCoefX = src_w * 512 / width;
	layer_desc->LayerResizCoefY = src_h * 512 / height;

}

static void lcd_scaler_disable(struct jzfb *jzfb, unsigned int frame, unsigned int layer)
{
	struct jzfb_framedesc *framedesc;
	struct jzfb_layerdesc *layer_desc;

	framedesc = jzfb->framedesc[frame];

	framedesc->LayCfgEn &= ~(1 << layer); //layer scaleEn disable

	layer_desc = jzfb->layerdesc[frame][layer];

	layer_desc->LayerScale = 0;	//scale
	layer_desc->LayerResizCoefX = 0;
	layer_desc->LayerResizCoefY = 0;
}

static void lcd_writeback_config(struct jzfb *jzfb, unsigned int frame, unsigned int format)
{
	struct jzfb_framedesc *framedesc;

		framedesc = jzfb->framedesc[frame];

		lcd_set_wb_format(jzfb, format, frame);
		framedesc->FrameCtrl |= (1 << 1);	//write back enable
		framedesc->WritebackAddr = virt_to_phys((unsigned int *)w_buf);
		framedesc->WritebackStride = WIDTH;

}

static void lcd_direct_display_config(struct jzfb *jzfb, unsigned int frame, unsigned int enable)
{
	struct jzfb_framedesc *framedesc;

	framedesc = jzfb->framedesc[frame];

	if (enable) {
		framedesc->FrameCtrl |= (1 << 2);	//direct output enable
	} else {
		framedesc->FrameCtrl &= ~(1 << 2);	//direct output disable
	}
}

static void lcd_rdma_config(struct jzfb *jzfb, unsigned int frame_num, unsigned int *buf)
{
	int i = 0;

	init_rdma_desc(jzfb, frame_num, buf);

	for (i = 0; i < frame_num; i++) {
		lcd_set_sr_format(jzfb, i, SR_RGB888);
	}

	*(volatile unsigned int *)0xb3051000 = virt_to_phys((unsigned int *)(jzfb->rdmadesc[0]));       //rdma frame addr(next) 8byte align
}

static void lcd_7layer_frame_config(struct jzfb *jzfb)
{
	struct jzfb_framedesc *framedesc;

	framedesc = jzfb->framedesc[0];
	framedesc->FrameCtrl &= ~(1 << 3);	//change_to_rdma
	framedesc->FrameCtrl &= ~1;	//keep refreshing
	lcd_writeback_config(jzfb, 0, WB_RGB888);
	lcd_direct_display_config(jzfb, 0, 0);

	framedesc = jzfb->framedesc[1];
	framedesc->FrameCtrl |= (1 << 3);	//change_to_rdma
	framedesc->FrameCtrl &= ~1;	//keep refreshing
	lcd_writeback_config(jzfb, 1, WB_RGB888);
	lcd_direct_display_config(jzfb, 1, 0);

#if 0
	framedesc = jzfb->framedesc[2];
	framedesc->FrameCtrl &= ~(1 << 3);	//change_to_rdma
	framedesc->FrameCtrl |= 1;	//the last frame, stop
	lcd_direct_display_config(jzfb, 2, 1);
#endif

}

static void lcd_1st_4layer_config(struct jzfb *jzfb, unsigned int frame)
{
	struct jzfb_layerdesc *layer_desc;

	unsigned int w = WIDTH / 4;
	unsigned int h = HEIGHT / 2;
	unsigned int y_pos = 0;


	layer_desc = jzfb->layerdesc[frame][0];
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerPos = (y_pos << 16) | 0;
	layer_desc->LayerStride = WIDTH;

	layer_desc = jzfb->layerdesc[frame][1];
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerPos = (y_pos << 16) | (w * 1);
	layer_desc->LayerStride = WIDTH;

	layer_desc = jzfb->layerdesc[frame][2];
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerPos = (y_pos << 16) | (w * 2);
	layer_desc->LayerStride = WIDTH;

	layer_desc = jzfb->layerdesc[frame][3];
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerPos = (y_pos << 16) | (w * 3);
	layer_desc->LayerStride = WIDTH;
}

static void lcd_2nd_3layer_config(struct jzfb *jzfb, unsigned int frame)
{
	struct jzfb_layerdesc *layer_desc;

	unsigned int w = WIDTH / 3;
	unsigned int h = HEIGHT / 2;
	unsigned int y_pos = 120;

	layer_desc = jzfb->layerdesc[frame][0];
	layer_desc->LayerSize = ((HEIGHT / 2) << 16) | WIDTH;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)w_buf);
	layer_desc->LayerPos = (0 << 16) | 0;
	layer_desc->LayerStride = WIDTH;

	layer_desc = jzfb->layerdesc[frame][1];
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerPos = (y_pos << 16) | 0;
	layer_desc->LayerStride = WIDTH;

	layer_desc = jzfb->layerdesc[frame][2];
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerPos = (y_pos << 16) | (w * 1);
	layer_desc->LayerStride = WIDTH;

	layer_desc = jzfb->layerdesc[frame][3];
	layer_desc->LayerSize = (h << 16) | w;
	layer_desc->LayerBufferAddr = virt_to_phys((unsigned int *)data_buf0);
	layer_desc->LayerPos = (y_pos << 16) | (w * 2);
	layer_desc->LayerStride = WIDTH;
}


extern unsigned long dmmu_map(struct device *dev,unsigned long vaddr,unsigned long len);
extern unsigned long dmmu_unmap(struct device *dev,unsigned long vaddr,unsigned long len);
extern int dmmu_unmap_all(struct device *dev);

static void lcd_tlb_config(struct jzfb *jzfb, unsigned int frame_num)
{
	unsigned int tlb_base = 0;
	struct jzfb_layerdesc *layer_desc;
	int i = 0;

	tlb_buf0 = vmalloc(WIDTH * HEIGHT * 4);
	tlb_buf1 = vmalloc(WIDTH * HEIGHT * 4);
	tlb_buf2 = vmalloc(WIDTH * HEIGHT * 4);
	tlb_buf3 = vmalloc(WIDTH * HEIGHT * 4);
	printk("addr is 0x%p\n", tlb_buf0);

	memcpy(tlb_buf0, convert_buf0, sizeof(convert_buf0));
	memcpy(tlb_buf1, convert_buf1, sizeof(convert_buf1));
	memcpy(tlb_buf2, data_buf2, sizeof(data_buf2));
	memcpy(tlb_buf3, data_buf3, sizeof(data_buf3));

	for (i = 0; i < frame_num; i++) {
		layer_desc = jzfb->layerdesc[i][0];
		layer_desc->LayerBufferAddr = (unsigned int)tlb_buf0;
		layer_desc->UVBufferAddr = (unsigned int)tlb_buf0 + WIDTH * HEIGHT;

		layer_desc = jzfb->layerdesc[i][1];
		layer_desc->LayerBufferAddr = (unsigned int)tlb_buf1;
		layer_desc->UVBufferAddr = (unsigned int)tlb_buf1 + WIDTH * HEIGHT;

		layer_desc = jzfb->layerdesc[i][2];
		layer_desc->LayerBufferAddr = (unsigned int)tlb_buf2;

		layer_desc = jzfb->layerdesc[i][3];
		layer_desc->LayerBufferAddr = (unsigned int)tlb_buf3;
	}

	*(volatile unsigned int *)0xb3053000 = 0xf;	//enable 4 layer tlb

	*(volatile unsigned int *)0xb3053020 = 0xf;
	*(volatile unsigned int *)0xb3053050 = 0;	//clear err

	tlb_base = dmmu_map(jzfb->dev, (unsigned long)tlb_buf0, WIDTH * HEIGHT * 4);
	tlb_base = dmmu_map(jzfb->dev, (unsigned long)tlb_buf1, WIDTH * HEIGHT * 4);
	tlb_base = dmmu_map(jzfb->dev, (unsigned long)tlb_buf2, WIDTH * HEIGHT * 4);
	tlb_base = dmmu_map(jzfb->dev, (unsigned long)tlb_buf3, WIDTH * HEIGHT * 4);
	*(volatile unsigned int *)0xb3053010 = tlb_base;
//	printk("tlb_base = 0x%x\n", tlb_base);


	dma_cache_wback_inv((unsigned long)tlb_buf0, WIDTH * HEIGHT * 4);
	dma_cache_wback_inv((unsigned long)tlb_buf1, WIDTH * HEIGHT * 4);
	dma_cache_wback_inv((unsigned long)tlb_buf2, WIDTH * HEIGHT * 4);
	dma_cache_wback_inv((unsigned long)tlb_buf3, WIDTH * HEIGHT * 4);
}

static void lcd_tlb_disable(struct jzfb *jzfb)
{
	unsigned int tlb_err = 0xffffffff;
	*(volatile unsigned int *)0xb3053000 = 0;

	dmmu_unmap_all(jzfb->dev);
	vfree(tlb_buf0);
	vfree(tlb_buf1);
	vfree(tlb_buf2);
	vfree(tlb_buf3);
	tlb_err = *(volatile unsigned int *)0xb3053050 & 0xf;
	if (tlb_err) {
		printk("tlb_err = 0x%x\n", *(volatile unsigned int *)0xb3053050);
	}
}


static void lcd_frame_dma_start(struct jzfb *jzfb, unsigned int frame_num)
{
	int i = 0;

	for (i = 0; i < frame_num; i++) {
		dma_cache_wback_inv((unsigned long)jzfb->framedesc[i], sizeof(struct jzfb_framedesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][0], sizeof(struct jzfb_layerdesc));
#if 0
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][1], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][2], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][3], sizeof(struct jzfb_layerdesc));
#endif
	}

	*(volatile unsigned int *)0xb3052014 &= ~(1 << 1);	//composer output channel

	lcd_clear_interrupt();
	lcd_enable_interrupt();

	*(volatile unsigned int *)0xb3050004 = 1;	//frame desc dma start
}

static void lcd_composer_start(struct jzfb *jzfb, unsigned int frame_num)
{
	int err = 0;

	lcd_frame_dma_start(jzfb, frame_num);

	err = wait_for_completion_timeout(&jzfb->composer_end, msecs_to_jiffies(3000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
		printk("current_site = 0x%x\n", *(volatile unsigned int *)0xb3052200);
	}


//	lcd_disable_interrupt();
//	lcd_clear_interrupt();
}



static void lcd_wb_start(struct jzfb *jzfb, unsigned int frame_num)
{
	int err = 0;

	dma_cache_wback_inv((unsigned long)w_buf, WIDTH * HEIGHT * 4);

	lcd_frame_dma_start(jzfb, frame_num);

	err = wait_for_completion_timeout(&jzfb->wb_end, msecs_to_jiffies(3000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

}


static void lcd_rdma_start(struct jzfb *jzfb, unsigned int frame_num)
{
	//int err = 0;
	int i = 0;

	for (i = 0; i < frame_num; i++) {
		dma_cache_wback_inv((unsigned long)jzfb->rdmadesc[i], sizeof(struct jzfb_rdmadesc));
		dma_cache_wback_inv((unsigned long)jzfb->framedesc[i], sizeof(struct jzfb_framedesc));
	}

	*(volatile unsigned int *)0xb3052014 |= (1 << 1);	//simple read channel

	lcd_clear_interrupt();
	lcd_enable_interrupt();

	*(volatile unsigned int *)0xb3051004 = 1;	//rdma desc DMA start


#if 0
	err = wait_for_completion_timeout(&jzfb->rdma_end, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout,  int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004);
	}

	err = wait_for_completion_timeout(&jzfb->display_end, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout,  int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004);
	}
#endif

//	lcd_disable_interrupt();
//	lcd_clear_interrupt();

}

static void lcd_wb_to_rdma_start(struct jzfb *jzfb, unsigned int frame_num)
{

	int err = 0;
	int i = 0;

	for (i = 0; i < frame_num; i++) {
		dma_cache_wback_inv((unsigned long)jzfb->framedesc[i], sizeof(struct jzfb_framedesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][0], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][1], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][2], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][3], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->rdmadesc[i], sizeof(struct jzfb_rdmadesc));
	}
	dma_cache_wback_inv((unsigned long)w_buf, WIDTH * HEIGHT * 4);

	*(volatile unsigned int *)0xb3052014 |= (1 << 1);       //simple read channel

	lcd_clear_interrupt();
	lcd_enable_interrupt();
	*(volatile unsigned int *)0xb3050004 = 1;	//frame desc dma start
	msleep(100);
	*(volatile unsigned int *)0xb3051004 = 1;       //rdma desc DMA start

	err = wait_for_completion_timeout(&jzfb->wb_end, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

	err = wait_for_completion_timeout(&jzfb->rdma_end, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

#if 0
	err = wait_for_completion_timeout(&jzfb->display_end, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}
#endif

//	lcd_disable_interrupt();
//	lcd_clear_interrupt();
}


static void lcd_composer_gen_stop(struct jzfb *jzfb)
{
	int err = 0;

//	lcd_clear_interrupt();
//	lcd_enable_interrupt();

	*(volatile unsigned int *)0xb3052000 = 1 << 3;

	err = wait_for_completion_timeout(&jzfb->cmp_stopped, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

	lcd_disable_interrupt();
	lcd_clear_interrupt();
}


static void lcd_rdma_gen_stop(struct jzfb *jzfb)
{
	int err = 0;

//	lcd_clear_interrupt();
//	lcd_enable_interrupt();

	*(volatile unsigned int *)0xb3052000 = 1 << 4;

	err = wait_for_completion_timeout(&jzfb->srd_stopped, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

	lcd_disable_interrupt();
	lcd_clear_interrupt();

}

static void lcd_wb_to_rdma_gen_stop(struct jzfb *jzfb)
{
	int err = 0;

//	lcd_clear_interrupt();
//	lcd_enable_interrupt();

	*(volatile unsigned int *)0xb3052000 = 1 << 4;
	*(volatile unsigned int *)0xb3052000 = 1 << 3;

	err = wait_for_completion_timeout(&jzfb->cmp_stopped, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

	err = wait_for_completion_timeout(&jzfb->srd_stopped, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

	lcd_disable_interrupt();
	lcd_clear_interrupt();

}

static void lcd_composer_quick_stop(struct jzfb *jzfb)
{
	//int err = 0;

	*(volatile unsigned int *)0xb3052000 = 1 << 0;
#if 0
	err = wait_for_completion_timeout(&jzfb->cmp_stopped, msecs_to_jiffies(5000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}
#endif
	lcd_disable_interrupt();
	lcd_clear_interrupt();
}


static void lcd_rdma_quick_stop(struct jzfb *jzfb)
{
	//int err = 0;

	*(volatile unsigned int *)0xb3052000 = 1 << 1;
#if 0
	err = wait_for_completion_timeout(&jzfb->srd_stopped, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}
#endif
	lcd_disable_interrupt();
	lcd_clear_interrupt();

}

#if 0
static void lcd_wb_to_rdma_quick_stop(struct jzfb *jzfb)
{
	int err = 0;

	*(volatile unsigned int *)0xb3052000 = 1 << 1;
	*(volatile unsigned int *)0xb3052000 = 1 << 0;

	err = wait_for_completion_timeout(&jzfb->cmp_stopped, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

	err = wait_for_completion_timeout(&jzfb->srd_stopped, msecs_to_jiffies(1000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
	}

	lcd_disable_interrupt();
	lcd_clear_interrupt();

}
#endif

static void lcd_change_to_rdma_start(struct jzfb *jzfb, unsigned int frame_num)
{
	int i = 0;
	int err = 0;

	*(volatile unsigned int *)0xb3052014 |= (1 << 1);	//simple read channel

	for (i = 0; i < frame_num; i++) {
		dma_cache_wback_inv((unsigned long)jzfb->framedesc[i], sizeof(struct jzfb_framedesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][0], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][1], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][2], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->layerdesc[i][3], sizeof(struct jzfb_layerdesc));
		dma_cache_wback_inv((unsigned long)jzfb->rdmadesc[i], sizeof(struct jzfb_rdmadesc));
	}


	lcd_clear_interrupt();
	lcd_enable_interrupt();

	*(volatile unsigned int *)0xb3050004 = 1;	//frame desc dma start
	msleep(100);
	*(volatile unsigned int *)0xb3051004 = 1;	//rdma desc DMA start

	err = wait_for_completion_timeout(&jzfb->composer_end, msecs_to_jiffies(3000));
	if (!err) {
		printk("line:%d dpu timeout, status = 0x%x   int_flag = 0x%x\n", __LINE__, *(volatile unsigned int *)0xb3052004, *(volatile unsigned int *)0xb3052010);
		printk("current_site = 0x%x\n", *(volatile unsigned int *)0xb3052200);

	}
}

static void lcd_change_to_rdma_stop(struct jzfb *jzfb)
{
	lcd_composer_gen_stop(jzfb);
//	lcd_rdma_gen_stop(jzfb);

}

static void lcd_simple_display(struct jzfb *jzfb, unsigned int frame_num, unsigned char *buf)
{

	lcd_rdma_config(jzfb, frame_num, (unsigned int *)buf);

	dma_cache_wback_inv((unsigned long)buf, WIDTH * HEIGHT * 4);

	lcd_rdma_start(jzfb, frame_num);

	msleep(1000);

	lcd_rdma_gen_stop(jzfb);

}

static void lcd_writeback_check(void)
{
	int i;

	for (i = 0; i < WIDTH * HEIGHT; i++) {
		/* write back dither is enabled */
		if ((w_buf[i] & 0x00ffffff) != (data_buf0[i] & 0xf0f8fc)) {
//		if ((w_buf[i] & 0x00ffffff) != (data_buf0[i] & 0xffffff)) {
			printk("xxxxxxxxxxx   w_buf[%d] = 0x%x   data_buf0[%d] = 0x%x\n", i, w_buf[i]  & 0x00ffffff, i, data_buf0[i]);
		}
	}

}


static void lcd_test_write_back(struct jzfb *jzfb, unsigned int frame_num)
{
	int i = 0;
	//struct jzfb_framedesc *framedesc;
	//struct jzfb_layerdesc *layer_desc;

	printk("dpu test function begin : writeback check (enable wb dither)\n");
	memset(w_buf, 0, sizeof(w_buf));

	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {

		init_layer_desc(jzfb, i);

#if 1
		lcd_composer_config(jzfb, i, 0, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 1, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 3, LAYER_RGB888);

		lcd_scaler_config(jzfb, i, 0, HEIGHT / 2, WIDTH / 2);
		lcd_scaler_config(jzfb, i, 1, HEIGHT / 2, WIDTH / 2);
#else
		framedesc = jzfb->framedesc[i];
		framedesc->LayCfgEn &= ~(7 << 5);

		init_layer_desc(jzfb, i);
		layer_desc = jzfb->layerdesc[i][0];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;
#endif

		lcd_writeback_config(jzfb, i, WB_RGB888);

		lcd_direct_display_config(jzfb, i, 0);
	}

	lcd_wb_start(jzfb, frame_num);

	msleep(1000);

	lcd_composer_gen_stop(jzfb);

	lcd_writeback_check();

	printk("dpu test function finish : writeback check (enable wb dither)\n\n");

}

static void lcd_test_dpu_alpha(struct jzfb *jzfb, unsigned int frame_num)
{
	int i;

	printk("dpu test function begin : dpu alpha\n");
	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {
		init_layer_desc(jzfb, i);

		lcd_composer_config(jzfb, i, 0, LAYER_NV12);
		lcd_composer_config(jzfb, i, 1, LAYER_NV21);
		lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 3, LAYER_RGB565);

		lcd_alpha_config(jzfb, i);

		lcd_direct_display_config(jzfb, i, 1);
	}

	lcd_tlb_config(jzfb, frame_num);

	lcd_composer_start(jzfb, frame_num);

	msleep(2000);

	lcd_composer_gen_stop(jzfb);

	lcd_tlb_disable(jzfb);

	printk("dpu test function finish : dpu alpha\n\n");

}

static void lcd_test_wb_to_rdma(struct jzfb *jzfb, unsigned int frame_num)
{
	unsigned int height, width;
	int i = 0;
	int s = 0;
	unsigned int scale = 120;
	unsigned int s_layer, n_layer;

	printk("dpu test function begin : writeback to rdma\n");

	memset(w_buf, 0, sizeof(w_buf));

	init_frame_desc(jzfb, frame_num);
	lcd_rdma_config(jzfb, frame_num, w_buf);

	for (s = 4; s <= scale; s = s + frame_num) {
		height = s;
		width = s;

		for (i = 0; i < frame_num; i++) {
			init_layer_desc(jzfb, i);

			lcd_composer_config(jzfb, i, 0, LAYER_NV12);
			lcd_composer_config(jzfb, i, 1, LAYER_NV21);
			lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
			lcd_composer_config(jzfb, i, 3, LAYER_RGB565);

			s_layer = s % 4;
			n_layer = s_layer + 1;
			if (n_layer == 4)
				n_layer = 0;
#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
			//lcd_scaler_config(jzfb, i, 2, 240, 400);
			//lcd_scaler_config(jzfb, i, 3, 120, 200);
#else
			lcd_scaler_config(jzfb, i, s_layer, height + i, width + i);
			lcd_scaler_config(jzfb, i, n_layer, (scale + 4) - height - i, (scale + 4) - width - i);
#endif

			lcd_writeback_config(jzfb, i, WB_RGB888);

			lcd_direct_display_config(jzfb, i, 0);
		}

		lcd_tlb_config(jzfb, frame_num);

		lcd_wb_to_rdma_start(jzfb, frame_num);

		msleep(500);

		lcd_wb_to_rdma_gen_stop(jzfb);

		lcd_tlb_disable(jzfb);

		for (i = 0; i < frame_num; i++) {
			lcd_scaler_disable(jzfb, i, s_layer);
			lcd_scaler_disable(jzfb, i, n_layer);
		}
	}

	printk("dpu test function finish : writeback to rdma\n\n");

}

#if 0
static void lcd_test_wb_to_rdma_check(struct jzfb *jzfb, unsigned int frame_num)
{
	unsigned int height, width;
	int i = 0;
	int s = 0;
	unsigned int scale = 120;
	unsigned int s_layer, n_layer;
	struct jzfb_framedesc *framedesc;
	struct jzfb_layerdesc *layer_desc;

	printk("dpu test function begin : writeback to rdma\n");

	init_data_buf_rgb888(data_buf0);
	memset(w_buf, 0, sizeof(w_buf));

	init_frame_desc(jzfb, frame_num);
	lcd_rdma_config(jzfb, frame_num, w_buf);


	for (i = 0; i < frame_num; i++) {
		framedesc = jzfb->framedesc[i];
		framedesc->LayCfgEn &= ~(7 << 5);

		init_layer_desc(jzfb, i);
		layer_desc = jzfb->layerdesc[i][0];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		lcd_writeback_config(jzfb, i, WB_RGB888);

		lcd_direct_display_config(jzfb, i, 0);
	}

	lcd_tlb_config(jzfb, frame_num);

	lcd_wb_to_rdma_start(jzfb, frame_num);

	msleep(500);

	lcd_wb_to_rdma_gen_stop(jzfb);

	lcd_tlb_disable(jzfb);

	lcd_writeback_check();

	printk("dpu test function finish : writeback to rdma\n\n");

}
#endif

static void lcd_test_4layer_composer(struct jzfb *jzfb, unsigned int frame_num)
{
	unsigned int height, width;
	int i = 0;
	int s = 0;
	unsigned int scale = 120;
	unsigned int s_layer, n_layer;

	printk("dpu test function begin : 4 layer composer\n");
	init_frame_desc(jzfb, frame_num);

	for (s = 4; s <= scale; s = s + frame_num) {
		height = s;
		width = s;
		for (i = 0; i < frame_num; i++) {
			init_layer_desc(jzfb, i);

			lcd_composer_config(jzfb, i, 0, LAYER_NV12);
			lcd_composer_config(jzfb, i, 1, LAYER_NV21);
			lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
			lcd_composer_config(jzfb, i, 3, LAYER_RGB565);

			s_layer = s % 4;
			n_layer = s_layer + 1;
			if (n_layer == 4)
				n_layer = 0;
#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
			lcd_scaler_config(jzfb, i, s_layer, 240, 400);
			lcd_scaler_config(jzfb, i, n_layer, 120, 200);
#else
			lcd_scaler_config(jzfb, i, s_layer, height + i, width + i);
			lcd_scaler_config(jzfb, i, n_layer, (scale + 4) - height - i, (scale + 4) - width - i);
#endif
			lcd_direct_display_config(jzfb, i, 1);
		}

		lcd_tlb_config(jzfb, frame_num);

		lcd_composer_start(jzfb, frame_num);

		msleep(500);

		lcd_composer_gen_stop(jzfb);

		lcd_tlb_disable(jzfb);

		for (i = 0; i < frame_num; i++) {
			lcd_scaler_disable(jzfb, i, s_layer);
			lcd_scaler_disable(jzfb, i, n_layer);
		}
	}

	printk("dpu test function finish : 4 layer composer\n\n");
}


static void lcd_test_rdma_direct_display(struct jzfb *jzfb, unsigned int frame_num)
{
	printk("dpu test function begin : rdma_direct_display\n");

	memcpy(r_buf, data_buf2, sizeof(r_buf));

	lcd_simple_display(jzfb, frame_num, (unsigned char *)r_buf);

	printk("dpu test function finish : rdma_direct_display\n\n");

}


static void lcd_test_change_to_rdma(struct jzfb *jzfb)
{
	unsigned int frame_num = NR_FRAMES;
	int i = 0;
	printk("dpu test function begin : change_to_rdma\n");

	for (i = 0; i < frame_num; i++) {
		init_layer_desc(jzfb, i);
	}
	init_frame_desc(jzfb, frame_num);

	lcd_7layer_frame_config(jzfb);	//frame 0 + frame 1 + frame 3
	init_rdma_desc(jzfb, frame_num, w_buf);	//frame 2

	lcd_1st_4layer_config(jzfb, 0);
	lcd_2nd_3layer_config(jzfb, 1);

	lcd_rdma_config(jzfb, 1, w_buf);

	lcd_change_to_rdma_start(jzfb, frame_num);

	msleep(2000);

	lcd_change_to_rdma_stop(jzfb);

	printk("dpu test function finish : change_to_rdma\n\n");
}

static void lcd_test_display_dither(struct jzfb *jzfb, unsigned int frame_num)
{
	printk("dpu test function begin : display dither\n");

	init_data_buf_dither(dither_buf);

	lcd_simple_display(jzfb, frame_num, (unsigned char *)dither_buf);

	/* diaplay dither enable */
	*(volatile unsigned int *)0xb3058000 |= (3 << 20) | (2 << 18) | (1 << 16) | (1 << 7) | (1 << 4);
	lcd_simple_display(jzfb, frame_num, (unsigned char *)dither_buf);

	/* diaplay dither disable */
	*(volatile unsigned int *)0xb3058000 &= ~(0x3f << 16) | (1 << 7) | (1 << 4);
	lcd_simple_display(jzfb, frame_num, (unsigned char *)dither_buf);

	printk("dpu test function finish : display dither\n\n");
}

static void lcd_test_init(struct jzfb *jzfb)
{

#if CONFIG_DSI_DPI_DEBUG	/*test pattern */
	unsigned int tmp = 0;
#endif
#if 1
	init_data_buf_rgb888(data_buf0);
	init_data_buf_rgb888(data_buf1);
	init_data_buf_rgb888(data_buf2);
	init_data_buf_rgb565((unsigned short *)data_buf3);

	rgb888_to_nv12(data_buf0, convert_buf0, WIDTH, HEIGHT, 1);
	rgb888_to_nv21(data_buf1, convert_buf1, WIDTH, HEIGHT, 1);
#else
#if 1
	memcpy((void *)data_buf0, (void *)gImage_rgb32_1024_600, sizeof(gImage_rgb32_1024_600));
	memcpy((void *)data_buf1, (void *)gImage_rgb32_1024_600, sizeof(gImage_rgb32_1024_600));
	memcpy((void *)data_buf2, (void *)gImage_rgb32_1024_600, sizeof(gImage_rgb32_1024_600));
#else
	memcpy((void *)data_buf0, (void *)buf_800_480_RGB888, sizeof(buf_800_480_RGB888));
	memcpy((void *)data_buf1, (void *)buf_800_480_RGB888, sizeof(buf_800_480_RGB888));
	memcpy((void *)data_buf2, (void *)buf_800_480_RGB888, sizeof(buf_800_480_RGB888));
#endif
#endif
#ifdef CONFIG_SLCD
	slcd_poweron();
	init_dpu_slcd();
	*(volatile unsigned int *)0xb3058000 = (1 << 6) | 2;	//use slcd, open slcd clk_gate
#ifdef CONFIG_BSP_CHIP_T40_FPGA_SLCD_TRULY_240_240
	slcd_mcu_init(truly_slcd240240_data_table);
#endif
#ifdef	CONFIG_MIPI_TX
	*(volatile unsigned int *)0xb3058000 |= 3;	//use mipi slcd
#endif
#endif

#ifdef CONFIG_TFT
#ifdef CONFIG_MIPI_TX
	jz_dsi_init(dsi);
	printk("jz_dsi_init ok!\n");
	msleep(500);
#endif
	//tft_poweron();
	init_dpu_tft();
	*(volatile unsigned int *)0xb3058000 = (1 << 5) | 1;	//use tft, open tft clk_gate
	printk("init_dpu_tft ok!\n");
#endif

	*(volatile unsigned int *)0xb3050000 = virt_to_phys((unsigned int *)(jzfb->framedesc[0]));	//frame addr(next) 8byte align
	*(volatile unsigned int *)0xb3052008 = 0x8003ffc6;	//clear status
	//*(volatile unsigned int *)0xb305200c = 0x8003ffc6;	//enable all interrupt
	*(volatile unsigned int *)0xb305200c = 0x8003c7c6;	//enable all interrupt
	//*(volatile unsigned int *)0xb3052014 = (0x3f << 16) | (3 << 6) | (3 << 4) | (3 << 2) | (0 << 1); //enable layer clk_gate, burst=32, use composer direct output
	*(volatile unsigned int *)0xb3052014 = (0x1 << 16) | (3 << 6) | (3 << 4) | (3 << 2) | (0 << 1); //enable layer clk_gate, burst=32, use composer direct output

#ifdef CONFIG_MIPI_TX
#ifdef CONFIG_TFT
#if CONFIG_DSI_DPI_DEBUG	/*test pattern */
	printk("dsi test pattern\n");
	jz_dsi_video_cfg(dsi);

	tmp = mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG);
	tmp |= 1 << 16 | 0 << 20 | 0 << 24;
	mipi_dsih_write_word(dsi, R_DSI_HOST_VID_MODE_CFG, tmp);
#else
	printk("dsi normal pattern\n");
	jz_dsi_video_cfg(dsi);
#endif /* CONFIG_DSI_DPI_DEBUG */
#endif /* CONFIG_TFT */
#endif /* CONFIG_MIPI_TX */
}

static void lcd_csc_mode_config(unsigned int layer, unsigned int mode)
{
	int mult_rv = 0;
	int mult_y = 0;
	int mult_gv = 0;
	int mult_gu = 0;
	int mult_bu = 0;
	int sub_uv = 0;
	int sub_y = 0;

	if (mode == 0) {
		mult_rv = 0x48f;
		mult_y = 0x400;
		mult_gv = 0x253;
		mult_gu = 0x193;
		mult_bu = 0x820;
		sub_uv = 0;
		sub_y = 0x0;
	} else if (mode == 1) {
		mult_rv = 0x662;
		mult_y = 0x4a8;
		mult_gv = 0x341;
		mult_gu = 0x190;
		mult_bu = 0x812;
		sub_uv = 0x80;
		sub_y = 0x0;

	} else if (mode == 2) {
		mult_rv = 0x59c;
		mult_y = 0x400;
		mult_gv = 0x2db;
		mult_gu = 0x160;
		mult_bu = 0x717;
		sub_uv = 0x80;
		sub_y = 0x0;

	} else if (mode == 3) {
		mult_rv = 0x570;
		mult_y = 0x47c;
		mult_gv = 0x2cb;
		mult_gu = 0x15a;
		mult_bu = 0x6ee;
		sub_uv = 0x80;
		sub_y = 0x0;

	} else {
		printk("csc mode error !!!\n");
	}


	*(volatile unsigned int *)(0xb3053200 + layer * 0x10) = (mult_rv << 16) | mult_y;
	*(volatile unsigned int *)(0xb3053204 + layer * 0x10)= (mult_gv << 16) | mult_gu;
	*(volatile unsigned int *)(0xb3053208 + layer * 0x10)=mult_bu;
	*(volatile unsigned int *)(0xb305320c + layer * 0x10)= (sub_uv << 16) | sub_y;

}

static void lcd_test_csc_mode(struct jzfb *jzfb, unsigned int frame_num)
{
	int i = 0;
	unsigned int mode;

	printk("dpu test function begin : csc mode\n");
	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {
		init_layer_desc(jzfb, i);

		lcd_composer_config(jzfb, i, 0, LAYER_NV12);
		lcd_composer_config(jzfb, i, 1, LAYER_NV21);
		lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 3, LAYER_RGB565);

		lcd_direct_display_config(jzfb, i, 1);

	}

	for (mode = 0; mode < 4; mode++) {
		rgb888_to_nv12(data_buf0, convert_buf0, WIDTH, HEIGHT, mode);
		rgb888_to_nv21(data_buf0, convert_buf1, WIDTH, HEIGHT, mode);

		lcd_csc_mode_config(0, mode);
		lcd_csc_mode_config(1, mode);
		lcd_csc_mode_config(2, mode);
		lcd_csc_mode_config(3, mode);

		printk("0xb3053200 = 0x%x\n", *(volatile unsigned int*)0xb3053200);
		printk("0xb3053204 = 0x%x\n", *(volatile unsigned int*)0xb3053204);
		printk("0xb3053208 = 0x%x\n", *(volatile unsigned int*)0xb3053208);
		printk("0xb305320c = 0x%x\n", *(volatile unsigned int*)0xb305320c);

		lcd_composer_start(jzfb, frame_num);

		msleep(1000);

		lcd_composer_gen_stop(jzfb);
	}

	printk("dpu test function finish : csc mode\n\n");


}


static ssize_t lcd_test_function(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	int i = 0;
	struct jzfb *jzfb = dev_get_drvdata(dev);

	lcd_test_init(jzfb);

	while (i < 3) {
		lcd_test_4layer_composer(jzfb, NR_FRAMES);
		lcd_test_write_back(jzfb, NR_FRAMES);
		lcd_test_wb_to_rdma(jzfb, NR_FRAMES);
		lcd_test_rdma_direct_display(jzfb, NR_FRAMES);

		if (NR_FRAMES == 3)
			lcd_test_change_to_rdma(jzfb);

		lcd_test_dpu_alpha(jzfb, NR_FRAMES);
		lcd_test_display_dither(jzfb, NR_FRAMES);
		i++;
	}

	return count;

}

static ssize_t lcd_test_csc(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	struct jzfb *jzfb = dev_get_drvdata(dev);

	lcd_test_init(jzfb);

	lcd_test_csc_mode(jzfb, NR_FRAMES);

	return count;

}

static void lcd_rdma_multi_frame_config(struct jzfb *jzfb, unsigned int frame)
{
	struct jzfb_rdmadesc *rdmadesc;

	rdmadesc = jzfb->rdmadesc[frame];

	rdmadesc->RdmaCfg &= ~1;
}


static ssize_t lcd_test_rdma_multi_frame_start(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);

	int i = 0;
	unsigned int frame_num = NR_FRAMES;

	printk("dpu test function begin : rdma multi frame\n");


	lcd_test_init(jzfb);

	memcpy(r_buf, data_buf2, sizeof(r_buf));

	lcd_rdma_config(jzfb, frame_num, r_buf);

	for (i = 0; i < frame_num; i++) {
		lcd_rdma_multi_frame_config(jzfb, i);
	}

	dma_cache_wback_inv((unsigned long)r_buf, WIDTH * HEIGHT * 4);

	lcd_rdma_start(jzfb, frame_num);

	return count;
}

static ssize_t lcd_test_rdma_multi_frame_stop(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);

	lcd_rdma_gen_stop(jzfb);

	printk("dpu test function finish : rdma multi frame\n\n");

	return count;
}
static ssize_t lcd_test_rdma_multi_frame_stop_quick(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);

	lcd_rdma_quick_stop(jzfb);

	printk("dpu test function finish : rdma multi frame\n\n");

	return count;
}

static void lcd_comp_multi_frame_config(struct jzfb *jzfb, unsigned int frame)
{
	struct jzfb_framedesc *framedesc;

	framedesc = jzfb->framedesc[frame];

	framedesc->FrameCtrl &= ~1 ;
}

static ssize_t lcd_test_comp_multi_frame_start(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);
	int i = 0;
	unsigned int frame_num = 0;

	printk("dpu test function begin : multi frame\n");

	frame_num = NR_FRAMES;

	lcd_test_init(jzfb);

	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {
		init_layer_desc(jzfb, i);

		lcd_composer_config(jzfb, i, 0, LAYER_RGB888);
#if 0
		lcd_composer_config(jzfb, i, 0, LAYER_NV12);
		lcd_composer_config(jzfb, i, 1, LAYER_NV21);
		lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 3, LAYER_RGB565);
#endif
#ifdef CONFIG_BSP_CHIP_T40_FPGA_TFT_BM8766
		lcd_scaler_config(jzfb, i, 2, 240, 400);
		lcd_scaler_config(jzfb, i, 3, 30 + (i * 30), 30 + (i * 30));
#else
		//lcd_scaler_config(jzfb, i, 2, 50 + (i * 30), 50 + (i * 30));
		//lcd_scaler_config(jzfb, i, 3, 30 + (i * 30), 30 + (i * 30));
#endif
		lcd_direct_display_config(jzfb, i, 1);

		lcd_comp_multi_frame_config(jzfb, i);
	}

#if CONFIG_DSI_DPI_DEBUG
	printk("dsi test pattern\n");
#else
	lcd_composer_start(jzfb, frame_num);
#endif
	return count;
}


static ssize_t lcd_test_comp_multi_frame_stop(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);

	lcd_composer_gen_stop(jzfb);

	printk("dpu test function finish : multi frame\n\n");

	return count;
}

static ssize_t lcd_test_comp_multi_frame_stop_quick(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);

	lcd_composer_quick_stop(jzfb);

	printk("dpu test function finish : multi frame\n\n");

	return count;
}

static void lcd_soft_reset(void)
{
	*(volatile unsigned int *)0xb00000c4 |= (1 << 13);
	msleep(10);
	*(volatile unsigned int *)0xb00000c4 &= ~(1 << 13);
}

static ssize_t lcd_test_soft_reset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	lcd_soft_reset();
	return count;
}

static ssize_t lcd_test_burst(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	const char *str = buf;
	int ret_count = 0;

	unsigned short wdma, rdma, bdma;

	while (!isxdigit(*str)) {
		str++;
		if(++ret_count >= count)
			return count;
	}
	wdma = simple_strtoul(str, (char **)&str, 10);

	while (!isxdigit(*str)) {
		str++;
		if(++ret_count >= count)
			return count;
	}
	rdma = simple_strtoul(str, (char **)&str, 10);

	while (!isxdigit(*str)) {
		str++;
		if(++ret_count >= count)
			return count;
	}
	bdma = simple_strtoul(str, (char **)&str, 10);

	*(volatile unsigned int *)0xb3052014 &= ~(0x3f << 2);
	*(volatile unsigned int *)0xb3052014 |= (wdma << 6) | (rdma << 4) | (bdma << 2);

	printk("burst is : wdma = %d  rdma = %d  bdma = %d\n", wdma, rdma, bdma);
	printk("reg 0xb3052014 val is : 0x%x\n", *(volatile unsigned int *)0xb3052014);

	return count;

}

#ifdef CONFIG_MIPI_TX
static ssize_t dump_dpu_regs(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);
	dump_dpu_reg();
	dump_framedesc(jzfb);
	return 0;
}
#endif

static void underrun_1layer_full(struct jzfb *jzfb)
{
	unsigned int frame_num = NR_FRAMES;
	struct jzfb_framedesc *framedesc;
	struct jzfb_layerdesc *layer_desc;
	int i = 0;

	printk("test function : %s begin\n", __func__);
	lcd_test_init(jzfb);
	init_data_buf_rgb888(data_buf0);

	init_frame_desc(jzfb, frame_num);
	for (i = 0; i < frame_num; i++) {
		framedesc = jzfb->framedesc[i];
		framedesc->LayCfgEn &= ~(7 << 5);
	}

	for (i = 0; i < frame_num; i++) {
		init_layer_desc(jzfb, i);
		layer_desc = jzfb->layerdesc[i][0];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;
	}

	lcd_composer_start(jzfb, frame_num);

	msleep(500);

	lcd_composer_gen_stop(jzfb);

	printk("test function : %s finish\n\n", __func__);
}

static void underrun_1layer_full_scale(struct jzfb *jzfb)
{
	unsigned int frame_num = NR_FRAMES;
	struct jzfb_framedesc *framedesc;
	struct jzfb_layerdesc *layer_desc;
	int i = 0;

	printk("test function : %s begin\n", __func__);

	lcd_test_init(jzfb);
	init_data_buf_rgb888(data_buf0);

	init_frame_desc(jzfb, frame_num);
	for (i = 0; i < frame_num; i++) {
		framedesc = jzfb->framedesc[i];
		framedesc->LayCfgEn &= ~(7 << 5);
	}

	for (i = 0; i < frame_num; i++) {

		init_layer_desc(jzfb, i);
		layer_desc = jzfb->layerdesc[i][0];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		lcd_scaler_config(jzfb, i, 0, HEIGHT, WIDTH);

	}

	lcd_composer_start(jzfb, frame_num);

	msleep(500);

	lcd_composer_gen_stop(jzfb);

	for (i = 0; i < frame_num; i++) {
		lcd_scaler_disable(jzfb, i, 0);
	}

	printk("test function : %s finish\n\n", __func__);

}

static void underrun_4layer_full(struct jzfb *jzfb)
{
	unsigned int frame_num = NR_FRAMES;
	//struct jzfb_framedesc *framedesc;
	struct jzfb_layerdesc *layer_desc;
	int i = 0;

	printk("test function : %s begin\n", __func__);

	lcd_test_init(jzfb);
	init_data_buf_rgb888(data_buf0);

	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {

		init_layer_desc(jzfb, i);

		layer_desc = jzfb->layerdesc[i][0];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		layer_desc = jzfb->layerdesc[i][1];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		layer_desc = jzfb->layerdesc[i][2];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		layer_desc = jzfb->layerdesc[i][3];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		lcd_composer_config(jzfb, i, 0, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 1, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 3, LAYER_RGB888);

	}

	lcd_composer_start(jzfb, frame_num);

	msleep(500);

	lcd_composer_gen_stop(jzfb);

	printk("test function : %s finish\n\n", __func__);
}

static void underrun_4layer_full_scale(struct jzfb *jzfb)
{
	unsigned int frame_num = NR_FRAMES;
	//struct jzfb_framedesc *framedesc;
	struct jzfb_layerdesc *layer_desc;
	int i = 0;

	printk("test function : %s begin\n", __func__);

	lcd_test_init(jzfb);
	init_data_buf_rgb888(data_buf0);


	lcd_test_init(jzfb);
	init_data_buf_rgb888(data_buf0);

	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {

		init_layer_desc(jzfb, i);

		layer_desc = jzfb->layerdesc[i][0];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		layer_desc = jzfb->layerdesc[i][1];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		layer_desc = jzfb->layerdesc[i][2];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		layer_desc = jzfb->layerdesc[i][3];
		layer_desc->LayerSize = (HEIGHT << 16) | WIDTH;
		layer_desc->LayerPos = (0 << 16) | 0;

		lcd_composer_config(jzfb, i, 0, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 1, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 2, LAYER_RGB888);
		lcd_composer_config(jzfb, i, 3, LAYER_RGB888);

		lcd_scaler_config(jzfb, i, 0, HEIGHT / 2, WIDTH / 2);
		lcd_scaler_config(jzfb, i, 1, HEIGHT / 2, WIDTH / 2);
	}

	lcd_composer_start(jzfb, frame_num);

	msleep(500);

	lcd_composer_gen_stop(jzfb);

	for (i = 0; i < frame_num; i++) {
		lcd_scaler_disable(jzfb, i, 0);
		lcd_scaler_disable(jzfb, i, 1);
	}

	printk("test function : %s finish\n\n", __func__);
}

static void underrun_4layer_quarter(struct jzfb *jzfb)
{
	unsigned int frame_num = NR_FRAMES;
	//struct jzfb_framedesc *framedesc;
	//struct jzfb_layerdesc *layer_desc;
	int i = 0;

	printk("test function : %s begin\n", __func__);

	lcd_test_init(jzfb);
	init_data_buf_rgb888(data_buf0);

	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {
		init_layer_desc(jzfb, i);
	}

	lcd_composer_start(jzfb, frame_num);

	msleep(500);

	lcd_composer_gen_stop(jzfb);

	printk("test function : %s finish\n\n", __func__);
}

static void underrun_4layer_quarter_scale(struct jzfb *jzfb)
{
	unsigned int frame_num = NR_FRAMES;
	//struct jzfb_framedesc *framedesc;
	//struct jzfb_layerdesc *layer_desc;
	int i = 0;

	printk("test function : %s begin\n", __func__);

	lcd_test_init(jzfb);
	init_data_buf_rgb888(data_buf0);

	init_frame_desc(jzfb, frame_num);

	for (i = 0; i < frame_num; i++) {
		init_layer_desc(jzfb, i);
		lcd_scaler_config(jzfb, i, 0, HEIGHT / 2, WIDTH / 2);
		lcd_scaler_config(jzfb, i, 1, HEIGHT / 2, WIDTH / 2);
	}

	lcd_composer_start(jzfb, frame_num);

	msleep(500);

	lcd_composer_gen_stop(jzfb);

	for (i = 0; i < frame_num; i++) {
		lcd_scaler_disable(jzfb, i, 0);
		lcd_scaler_disable(jzfb, i, 1);
	}

	printk("test function : %s finish\n\n", __func__);
}

static ssize_t tft_under_debug(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct jzfb *jzfb = dev_get_drvdata(dev);

	const char *str = buf;
	int ret_count = 0;

	unsigned int test_index;

	while (!isxdigit(*str)) {
		str++;
		if(++ret_count >= count)
			return count;
	}
	test_index = simple_strtoul(str, (char **)&str, 10);

	switch (test_index) {
		case 0 :
			underrun_1layer_full(jzfb);
			break;
		case 1 :
			/* when use this case, should check scaler src size */
			underrun_1layer_full_scale(jzfb);
			break;
		case 2 :
			underrun_4layer_full(jzfb);
			break;
		case 3 :
			/* when use this case, should check scaler src size */
			underrun_4layer_full_scale(jzfb);
			break;
		case 4 :
			underrun_4layer_quarter(jzfb);
			break;
		case 5 :
			underrun_4layer_quarter_scale(jzfb);
			break;
	}

	return count;

}

#ifdef CONFIG_MIPI_TX
static ssize_t dump_dsi_reg(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	printk( "===========>dump dsi reg\n");
	printk( "VERSION------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VERSION));
	printk( "PWR_UP:------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PWR_UP));
	printk( "CLKMGR_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CLKMGR_CFG));
	printk( "DPI_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_VCID));
	printk( "DPI_COLOR_CODING---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_COLOR_CODING));
	printk( "DPI_CFG_POL--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_CFG_POL));
	printk( "DPI_LP_CMD_TIM-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_LP_CMD_TIM));
	printk( "DBI_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_VCID));
	printk( "DBI_CFG------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_CFG));
	printk( "DBI_PARTITIONING_EN:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_PARTITIONING_EN));
	printk( "DBI_CMDSIZE--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_CMDSIZE));
	printk( "PCKHDL_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PCKHDL_CFG));
	printk( "GEN_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_VCID));
	printk( "MODE_CFG-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_MODE_CFG));
	printk( "VID_MODE_CFG-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG));
	printk( "VID_PKT_SIZE-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_PKT_SIZE));
	printk( "VID_NUM_CHUNKS-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NUM_CHUNKS));
	printk( "VID_NULL_SIZE------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NULL_SIZE));
	printk( "VID_HSA_TIME-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HSA_TIME));
	printk( "VID_HBP_TIME-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HBP_TIME));
	printk( "VID_HLINE_TIME-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HLINE_TIME));
	printk( "VID_VSA_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VSA_LINES));
	printk( "VID_VBP_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VBP_LINES));
	printk( "VID_VFP_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VFP_LINES));
	printk( "VID_VACTIVE_LINES--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VACTIVE_LINES));
	printk( "EDPI_CMD_SIZE------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_EDPI_CMD_SIZE));
	printk( "CMD_MODE_CFG-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CMD_MODE_CFG));
	printk( "GEN_HDR------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_HDR));
	printk( "GEN_PLD_DATA-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_PLD_DATA));
	printk( "CMD_PKT_STATUS-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CMD_PKT_STATUS));
	printk( "TO_CNT_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_TO_CNT_CFG));
	printk( "HS_RD_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_HS_RD_TO_CNT));
	printk( "LP_RD_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LP_RD_TO_CNT));
	printk( "HS_WR_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_HS_WR_TO_CNT));
	printk( "LP_WR_TO_CNT_CFG---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LP_WR_TO_CNT));
	printk( "BTA_TO_CNT---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_BTA_TO_CNT));
	printk( "SDF_3D-------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_SDF_3D));
	printk( "LPCLK_CTRL---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LPCLK_CTRL));
	printk( "PHY_TMR_LPCLK_CFG--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TMR_LPCLK_CFG));
	printk( "PHY_TMR_CFG--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TMR_CFG));
	printk( "PHY_RSTZ-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_RSTZ));
	printk( "PHY_IF_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_IF_CFG));
	printk( "PHY_ULPS_CTRL------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_ULPS_CTRL));
	printk( "PHY_TX_TRIGGERS----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TX_TRIGGERS));
	printk( "PHY_STATUS---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS));
	printk( "PHY_TST_CTRL0------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TST_CTRL0));
	printk( "PHY_TST_CTRL1------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TST_CTRL1));
	printk( "INT_ST0------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_ST0));
	printk( "INT_ST1------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_ST1));
	printk( "INT_MSK0-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_MSK0));
	printk( "INT_MSK1-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_MSK1));
	printk( "INT_FORCE0---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_FORCE0));
	printk( "INT_FORCE1---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_FORCE1));
	printk( "VID_SHADOW_CTRL----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_SHADOW_CTRL));
	printk( "DPI_VCID_ACT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_VCID_ACT));
	printk( "DPI_COLOR_CODING_AC:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_COLOR_CODING_ACT));
	printk( "DPI_LP_CMD_TIM_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_LP_CMD_TIM_ACT));
	printk( "VID_MODE_CFG_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG_ACT));
	printk( "VID_PKT_SIZE_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_PKT_SIZE_ACT));
	printk( "VID_NUM_CHUNKS_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NUM_CHUNKS_ACT));
	printk( "VID_HSA_TIME_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HSA_TIME_ACT));
	printk( "VID_HBP_TIME_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HBP_TIME_ACT));
	printk( "VID_HLINE_TIME_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HLINE_TIME_ACT));
	printk( "VID_VSA_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VSA_LINES_ACT));
	printk( "VID_VBP_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VBP_LINES_ACT));
	printk( "VID_VFP_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VFP_LINES_ACT));
	printk( "VID_VACTIVE_LINES_ACT:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VACTIVE_LINES_ACT));
	printk( "SDF_3D_ACT---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_SDF_3D_ACT));
	return 0;
}


static ssize_t mipi_wr_cmd(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
#if 0
	struct dsi_cmd_packet cmd_data = {0x05, 0x0, 0x0, {0x12, 0x34}};
	mipi_dsih_write_word(dsi, R_DSI_HOST_MODE_CFG, 0x1);
	write_command(dsi, cmd_data);
	mipi_dsih_write_word(dsi, R_DSI_HOST_MODE_CFG, 0x0);
	return count;
#endif
	return 0;
}

static ssize_t mipi_lcd_rst(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	ret = gpio_request(GPIO_PD(7), "lcd-rst");
	if(ret)
		printk(KERN_ERR "can's request lcd-rst\n");
	gpio_direction_output(GPIO_PD(7), 0);
	mdelay(500);
	gpio_direction_output(GPIO_PD(7), 1);
	mdelay(10);
	gpio_direction_output(GPIO_PD(7), 0);
	mdelay(15);
	gpio_direction_output(GPIO_PD(7), 1);
	msleep(100);
	gpio_free(GPIO_PD(7));
	return count;
}

static ssize_t mipi_dsi_rst(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	jz_dsih_hal_power(dsi, 0);
	jz_dsih_hal_power(dsi, 1);

	return count;
}
#endif
static irqreturn_t jzfb_irq_handler(int irq, void *data)
{
	unsigned int int_flag = 0;
	struct jzfb *jzfb = (struct jzfb *)data;

	int_flag = *(volatile unsigned int *)0xb3052010;

	if (int_flag & (1 << 31)) {

		/* writeback slow */
		*(volatile unsigned int *)0xb3052008 = 1 << 31;
		printk("writeback slow\n");
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 17)) {

		/* one frame display end */
		*(volatile unsigned int *)0xb3052008 = 1 << 17;
//		complete(&jzfb->display_end);
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 16)) {

		/* writeback dma overrun */
		*(volatile unsigned int *)0xb3052008 = 1 << 16;
		printk("writeback dma overrun\n");
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 15)) {

		/* writeback dma end */
		*(volatile unsigned int *)0xb3052008 = 1 << 15;
		complete(&jzfb->wb_end);
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 14)) {

		/* composer start reading one frame */
		*(volatile unsigned int *)0xb3052008 = 1 << 14;
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 13)) {

		/* layer3 end */
		*(volatile unsigned int *)0xb3052008 = 1 << 13;
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 12)) {

		/* layer2 end */
		*(volatile unsigned int *)0xb3052008 = 1 << 12;
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 11)) {

		/* layer1 end */
		*(volatile unsigned int *)0xb3052008 = 1 << 11;
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 10)) {

		/* layer0 end */
		*(volatile unsigned int *)0xb3052008 = 1 << 10;
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 9)) {

		/* composer end of each frame */
		*(volatile unsigned int *)0xb3052008 = 1 << 9;
		complete(&jzfb->composer_end);
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 8)) {

		/* TFT underrun */
		*(volatile unsigned int *)0xb3052008 = 1 << 8;
		//printk("TFT under run !!!\n");
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 7)) {

		/* rdma gen_stopped */
		*(volatile unsigned int *)0xb3052008 = 1 << 7;
		complete(&jzfb->srd_stopped);
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 6)) {

		/* composer gen_stopped */
		*(volatile unsigned int *)0xb3052008 = 1 << 6;
		complete(&jzfb->cmp_stopped);
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 2)) {

		/* rdma start reading one frame */
		*(volatile unsigned int *)0xb3052008 = 1 << 2;
		return IRQ_HANDLED;

	} else if (int_flag & (1 << 1)) {

		/* rdma one frame end */
		*(volatile unsigned int *)0xb3052008 = 1 << 1;
		complete(&jzfb->rdma_end);
		return IRQ_HANDLED;

	} else {
		*(volatile unsigned int *)0xb3052008 = 0xffffffff;
		printk("unsuppoer dpu interrupt\n");
		return IRQ_NONE;
	}


}

static struct device_attribute lcd_device_attributes[] = {
	__ATTR(lcd_test_function, S_IWUSR, NULL, lcd_test_function),
	__ATTR(lcd_test_comp_multi_frame_start, S_IWUSR, NULL, lcd_test_comp_multi_frame_start),
	__ATTR(lcd_test_comp_multi_frame_stop, S_IWUSR, NULL, lcd_test_comp_multi_frame_stop),
	__ATTR(lcd_test_comp_multi_frame_stop_quick, S_IWUSR, NULL, lcd_test_comp_multi_frame_stop_quick),
	__ATTR(lcd_test_rdma_multi_frame_start, S_IWUSR, NULL, lcd_test_rdma_multi_frame_start),
	__ATTR(lcd_test_rdma_multi_frame_stop, S_IWUSR, NULL, lcd_test_rdma_multi_frame_stop),
	__ATTR(lcd_test_rdma_multi_frame_stop_quick, S_IWUSR, NULL, lcd_test_rdma_multi_frame_stop_quick),
	__ATTR(lcd_test_burst, S_IWUSR, NULL, lcd_test_burst),
	__ATTR(lcd_test_csc, S_IWUSR, NULL, lcd_test_csc),
	__ATTR(lcd_test_soft_reset, S_IWUSR, NULL, lcd_test_soft_reset),
#ifdef CONFIG_MIPI_TX
	__ATTR(dump_dpu_regs, S_IWUSR, NULL, dump_dpu_regs),
	__ATTR(dump_dsi_reg, S_IWUSR, NULL, dump_dsi_reg),
	__ATTR(mipi_wr_cmd, S_IWUSR, NULL, mipi_wr_cmd),
	__ATTR(mipi_dsi_rst, S_IWUSR, NULL, mipi_dsi_rst),
	__ATTR(mipi_lcd_rst, S_IWUSR, NULL, mipi_lcd_rst),
#endif
	__ATTR(tft_under_debug, S_IWUSR, NULL, tft_under_debug),
};

static int jzfb_open(struct fb_info *info, int user)
{
	return 0;
}

static int jzfb_release(struct fb_info *info, int user)
{
	return 0;
}

static struct fb_ops jzfb_ops = {
	.owner = THIS_MODULE,
	.fb_open = jzfb_open,
	.fb_release = jzfb_release,
};

static int jzfb_probe(struct platform_device *pdev)
{
	struct jzfb_platform_data *pdata = pdev->dev.platform_data;
	struct fb_videomode *video_mode;
	struct resource *mem;
	struct fb_info *fb;
	int ret = 0;
	int i;
	struct jzfb *jzfb;

	printk("xxxxxxxxxxxxxxxxxxxxxxxxxxxx %s %s %d\n", __FILE__, __func__, __LINE__);

	if (!pdata) {
		dev_err(&pdev->dev, "Missing platform data\n");
		return -ENXIO;
	}

	fb = framebuffer_alloc(sizeof(struct jzfb), &pdev->dev);
	if (!fb) {
		dev_err(&pdev->dev, "Failed to allocate framebuffer device\n");
		return -ENOMEM;
	}

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		dev_err(&pdev->dev, "Failed to get register memory resource\n");
		ret = -ENXIO;
		goto err_framebuffer_release;
	}

	mem = request_mem_region(mem->start, resource_size(mem), pdev->name);
	if (!mem) {
		dev_err(&pdev->dev,
				"Failed to request register memory region\n");
		ret = -EBUSY;
		goto err_framebuffer_release;
	}

	jzfb = (struct jzfb *)((unsigned int)kmalloc(sizeof(struct jzfb), GFP_KERNEL));
	for (i = 0; i < NR_FRAMES; i++) {
		jzfb->framedesc[i] = (struct jzfb_framedesc *)((unsigned int)kmalloc(sizeof(struct jzfb_framedesc) * 3, GFP_KERNEL));
		jzfb->layerdesc[i][0] = (struct jzfb_layerdesc *)((unsigned int)kmalloc(sizeof(struct jzfb_layerdesc) * 4, GFP_KERNEL));
		jzfb->layerdesc[i][1] = (struct jzfb_layerdesc *)((unsigned int)kmalloc(sizeof(struct jzfb_layerdesc) * 4, GFP_KERNEL));
		jzfb->layerdesc[i][2] = (struct jzfb_layerdesc *)((unsigned int)kmalloc(sizeof(struct jzfb_layerdesc) * 4, GFP_KERNEL));
		jzfb->layerdesc[i][3] = (struct jzfb_layerdesc *)((unsigned int)kmalloc(sizeof(struct jzfb_layerdesc) * 4, GFP_KERNEL));
		jzfb->rdmadesc[i] = (struct jzfb_rdmadesc *)((unsigned int)kmalloc(sizeof(struct jzfb_rdmadesc) * 3, GFP_KERNEL));

		memset(jzfb->framedesc[i], 0, sizeof(struct jzfb_framedesc));
		memset(jzfb->layerdesc[i][0], 0, sizeof(struct jzfb_layerdesc));
		memset(jzfb->layerdesc[i][1], 0, sizeof(struct jzfb_layerdesc));
		memset(jzfb->layerdesc[i][2], 0, sizeof(struct jzfb_layerdesc));
		memset(jzfb->layerdesc[i][3], 0, sizeof(struct jzfb_layerdesc));
		memset(jzfb->rdmadesc[i], 0, sizeof(struct jzfb_rdmadesc));

	}

	printk("%s %d  jzfb->layerdesc[0][0] = 0x%x\n", __func__, __LINE__, (unsigned int)jzfb->layerdesc[0][0]);

	jzfb->fb = fb;
	jzfb->dev = &pdev->dev;
	jzfb->pdata = pdata;
	jzfb->mem = mem;

	printk("%s %d  jzfb->layerdesc[0][0] = 0x%x\n", __func__, __LINE__, (unsigned int)jzfb->layerdesc[0][0]);
	spin_lock_init(&jzfb->irq_lock);
	mutex_init(&jzfb->lock);
	mutex_init(&jzfb->suspend_lock);
	sprintf(jzfb->clk_name, "lcd");
	sprintf(jzfb->pclk_name, "cgu_lcd");

#if 0
	jzfb->clk = clk_get(&pdev->dev, jzfb->clk_name);
	jzfb->pclk = clk_get(&pdev->dev, jzfb->pclk_name);

	if (IS_ERR(jzfb->clk) || IS_ERR(jzfb->pclk)) {
		ret = PTR_ERR(jzfb->clk);
		dev_err(&pdev->dev, "Failed to get lcdc clock: %d\n", ret);
		goto err_release_mem_region;
	}
#endif

	jzfb->base = ioremap(mem->start, resource_size(mem));
	if (!jzfb->base) {
		dev_err(&pdev->dev,
				"Failed to ioremap register memory region\n");
		ret = -EBUSY;
		goto err_put_clk;
	}


	video_mode = jzfb->pdata->modes;
	if (!video_mode) {
		ret = -ENXIO;
		goto err_iounmap;
	}

	fb->fbops = &jzfb_ops;
	fb->flags = FBINFO_DEFAULT;
	fb->var.width = pdata->width;
	fb->var.height = pdata->height;

	/*
	 * #BUG: if uboot pixclock is different from kernel. this may cause problem.
	 *
	 * */

#if 0
	fb->var.pixclock = 21701;
	rate = PICOS2KHZ(fb->var.pixclock) * 1000;
	rate = 46080000;
	clk_set_rate(jzfb->pclk, rate);

	if(rate != clk_get_rate(jzfb->pclk)) {
		dev_err(&pdev->dev, "failed to set pixclock!, rate:%ld, clk_rate = %ld\n", rate, clk_get_rate(jzfb->pclk));

	}

	clk_prepare_enable(jzfb->clk);
	clk_prepare_enable(jzfb->pclk);
	jzfb->is_clk_en = 1;
#endif


	init_waitqueue_head(&jzfb->vsync_wq);
	init_waitqueue_head(&jzfb->gen_stop_wq);

	INIT_LIST_HEAD(&jzfb->desc_run_list);

	jzfb->irq = platform_get_irq(pdev, 0);
	sprintf(jzfb->irq_name, "lcdc%d", pdev->id);

	if (request_irq(jzfb->irq, jzfb_irq_handler, IRQF_DISABLED,
				jzfb->irq_name, jzfb)) {
		dev_err(&pdev->dev, "request irq failed\n");
		ret = -EINVAL;
	}

	platform_set_drvdata(pdev, jzfb);

	ret = register_framebuffer(fb);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register framebuffer: %d\n",
				ret);
	}

	init_completion(&jzfb->display_end);
	init_completion(&jzfb->wb_end);
	init_completion(&jzfb->rdma_end);
	init_completion(&jzfb->composer_end);
	init_completion(&jzfb->srd_stopped);
	init_completion(&jzfb->cmp_stopped);
	printk("xxxxxxxxxxxxxxxxxxxxxxxxxxxx %s %s %d\n", __FILE__, __func__, __LINE__);

	printk("%s %d  jzfb->layerdesc[0][0] = 0x%x\n", __func__, __LINE__, (unsigned int)jzfb->layerdesc[0][0]);



	for (i = 0; i < ARRAY_SIZE(lcd_device_attributes); i++) {
		ret = device_create_file(&pdev->dev, &lcd_device_attributes[i]);
		if (ret)
			dev_warn(&pdev->dev, "attribute %s create failed", attr_name(lcd_device_attributes[i]));
	}

/********************************************************************************/
//#ifdef CONFIG_MIPI_TX
	*(volatile unsigned int *)0xb00030a0 = 0xf;	//control-->phy reset

	dsi = (struct dsi_device *)((unsigned int)kmalloc(sizeof(struct dsi_device), GFP_KERNEL));
	dsi->dsi_phy = (struct dsi_phy *)((unsigned int)kmalloc(sizeof(struct dsi_phy), GFP_KERNEL));
	dsi->dsi_config = &(pdata->dsi_pdata->dsi_config);
	dsi->video_config = &(pdata->dsi_pdata->video_config);
	printk("GATE0: 0x10000028 = %x\n", *(volatile unsigned int *)0xb0000028);
	*(volatile unsigned int *)0xb0000028 &= ~(1<<7); //open gate for clk
	printk("GATE0: 0x10000028 = %x\n", *(volatile unsigned int *)0xb0000028);
#if 0
	jz_dsi_phy_open(dsi);
	jz_dsi_phy_cfg(dsi);
	//jz_dsi_init(dsi);
#endif
#if 0
#define HSA	128
#define HBP	100
#define HACT	16
#define	HFP	128
#define	VSA	100
#define	VBP	100
#define	VACT	2
#define	VFP	100
#define	RATIO	3
	*(volatile unsigned int *)0xb007500c = 0;	//DPI_VCID
//	*(volatile unsigned int *)0xb0075010 = 0x5;	//DPI_COLOR_CODING
	*(volatile unsigned int *)0xb0075014 = 0;	//DPI_CFG_POL
	*(volatile unsigned int *)0xb0075008 = 0x107;	//CLKMGR_CFG
	*(volatile unsigned int *)0xb0075068 = 0;	//CMD_MODE_CFG
	*(volatile unsigned int *)0xb007501c = 0;	//DBI_VCID
	*(volatile unsigned int *)0xb0075020 = 0x10004;	//DBI_CFG
	*(volatile unsigned int *)0xb0075024 = 1;	//DBI_PARTITIONING_EN
	*(volatile unsigned int *)0xb0075028 = 0x640008;//DBI_CMDSIZE
	*(volatile unsigned int *)0xb007502c = 0x18;	//PCKHDL_CFG
	*(volatile unsigned int *)0xb0075030 = 0x0;	//GEN_VCID
	*(volatile unsigned int *)0xb0075034 = 0;	//MODE_CFG
	*(volatile unsigned int *)0xb0075038 = 0x3f01;	//VID_MODE_CFG
	*(volatile unsigned int *)0xb007503c = HACT;	//VID_PKT_SIZE
	*(volatile unsigned int *)0xb0075040 = 0;	//VID_NUM_CHUNKS
	*(volatile unsigned int *)0xb0075044 = 0;	//VID_NULL_SIZE
	*(volatile unsigned int *)0xb0075048 = HSA * RATIO;	//VID_HSA_TIME
	*(volatile unsigned int *)0xb007504c = HBP * RATIO;	//VID_HBP_TIME
	*(volatile unsigned int *)0xb0075050 = (HSA + HBP + HACT + HFP) * RATIO;	//VID_HLINE_TIME
	*(volatile unsigned int *)0xb0075054 = VSA;	//VID_VSA_LINES
	*(volatile unsigned int *)0xb0075058 = VBP;	//VID_VBP_LINES
	*(volatile unsigned int *)0xb007505c = VFP;	//VID_VFP_LINES
	*(volatile unsigned int *)0xb0075018 = 0;	//DPI_LP_CMD_TIM
	*(volatile unsigned int *)0xb0075060 = VACT;	//VID_VACTIVE_LINES
	*(volatile unsigned int *)0xb0075018 = 0;	//DPI_LP_CMD_TIM
	*(volatile unsigned int *)0xb007509c = 0x02030000;	//PHY_TMR_CFG
	*(volatile unsigned int *)0xb0075064 = 0;	//EDPI_CMD_SIZE
	*(volatile unsigned int *)0xb0075094 = 0;	//LPCLK_CTRL
	*(volatile unsigned int *)0xb0075078 = 0;	//TO_CNT_CFG
	*(volatile unsigned int *)0xb007507c = 0;	//HS_RD_TO_CNT
	*(volatile unsigned int *)0xb0075080 = 0;	//LP_RD_TO_CNT
	*(volatile unsigned int *)0xb0075084 = 0;	//HS_WR_TO_CNT
	*(volatile unsigned int *)0xb0075088 = 0;	//lp_WR_TO_CNT
	*(volatile unsigned int *)0xb007508c = 0;	//BTA_TO_CNT
	*(volatile unsigned int *)0xb0075098 = 0x870025;//PHY_TMR_LPCLK_CFG
	*(volatile unsigned int *)0xb00750a4 = 0x2801;	//PHY_IF_CFG
	*(volatile unsigned int *)0xb00750a8 = 0;	//PHY_ULPS_CTRL
	*(volatile unsigned int *)0xb00750ac = 0;	//PHY_TX_TRIGGERS
	*(volatile unsigned int *)0xb0075100 = 0;	//VID_SHADOW_CTRL
	*(volatile unsigned int *)0xb0075004 = 0;	//PWR_UP
	*(volatile unsigned int *)0xb0075004 = 1;	//PWR_UP
	*(volatile unsigned int *)0xb00750a0 = 0xf;	//PHY_RSTZ

	while ((*(volatile unsigned int *)0xb00750b0 &  0x5) != 0x5);	//PHY_STATUS

	*(volatile unsigned int *)0xb0075094 = 1;	//LPCLK_CTRL

#endif
	return 0;
err_iounmap:
	iounmap(jzfb->base);
err_put_clk:

   if (jzfb->clk)
	clk_put(jzfb->clk);
   if (jzfb->pclk)
	clk_put(jzfb->pclk);
#if 0
err_release_mem_region:
	release_mem_region(mem->start, resource_size(mem));
#endif
err_framebuffer_release:
	framebuffer_release(fb);
	return ret;
}

static int jzfb_remove(struct platform_device *pdev)
{
	//struct jzfb *jzfb = platform_get_drvdata(pdev);
	return 0;
}

static void jzfb_shutdown(struct platform_device *pdev)
{
	struct jzfb *jzfb = platform_get_drvdata(pdev);
	int is_fb_blank;
	mutex_lock(&jzfb->suspend_lock);
	is_fb_blank = (jzfb->is_suspend != 1);
	jzfb->is_suspend = 1;
	mutex_unlock(&jzfb->suspend_lock);
	if (is_fb_blank)
		fb_blank(jzfb->fb, FB_BLANK_POWERDOWN);
};

#ifdef CONFIG_PM

static int jzfb_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct jzfb *jzfb = platform_get_drvdata(pdev);

	fb_blank(jzfb->fb, FB_BLANK_POWERDOWN);

	return 0;
}

static int jzfb_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct jzfb *jzfb = platform_get_drvdata(pdev);

	fb_blank(jzfb->fb, FB_BLANK_UNBLANK);

	return 0;
}

static const struct dev_pm_ops jzfb_pm_ops = {
	.suspend = jzfb_suspend,
	.resume = jzfb_resume,
};
#endif
static struct platform_driver jzfb_driver = {
	.probe = jzfb_probe,
	.remove = jzfb_remove,
	.shutdown = jzfb_shutdown,
	.driver = {
		.name = "jz-fb",
#ifdef CONFIG_PM
		.pm = &jzfb_pm_ops,
#endif

	},
};

static int __init jzfb_init(void)
{
	platform_driver_register(&jzfb_driver);
	return 0;
}

static void __exit jzfb_cleanup(void)
{
	platform_driver_unregister(&jzfb_driver);
}


#ifdef CONFIG_EARLY_INIT_RUN
rootfs_initcall(jzfb_init);
#else
module_init(jzfb_init);
#endif

module_exit(jzfb_cleanup);

MODULE_DESCRIPTION("JZ LCD Controller driver");
MODULE_LICENSE("GPL");

