/*
 * Usefuls routines based on the LzmaTest.c file from LZMA SDK 4.65
 *
 * Copyright (C) 2007-2009 Industrie Dial Face S.p.A.
 * Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
 *
 * Copyright (C) 1999-2005 Igor Pavlov
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * LZMA_Alone stream format:
 *
 * uchar   Properties[5]
 * uint64  Uncompressed size
 * uchar   data[*]
 *
 */

#include <config.h>
#include <common.h>
#include <watchdog.h>

#ifdef CONFIG_LZMA

#define LZMA_PROPERTIES_OFFSET 0
#define LZMA_SIZE_OFFSET       LZMA_PROPS_SIZE
#define LZMA_DATA_OFFSET       LZMA_SIZE_OFFSET+sizeof(uint64_t)

#include "LzmaTools.h"
#include "LzmaDec.h"

#include <linux/string.h>
#include <malloc.h>

static void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void SzFree(void *p, void *address) { p = p; free(address); }

int lzmaBuffToBuffDecompress (unsigned char *outStream, SizeT *uncompressedSize,
                  unsigned char *inStream,  SizeT  length)
{
    int res = SZ_ERROR_DATA;
    int i;
    ISzAlloc g_Alloc;

    SizeT outSizeFull = 0xFFFFFFFF; /* 4GBytes limit */
    SizeT outProcessed;
    SizeT outSize;
    SizeT outSizeHigh;
    ELzmaStatus state;
    SizeT compressedSize = (SizeT)(length - LZMA_PROPS_SIZE);

    debug ("LZMA: Image address............... 0x%p\n", inStream);
    debug ("LZMA: Properties address.......... 0x%p\n", inStream + LZMA_PROPERTIES_OFFSET);
    debug ("LZMA: Uncompressed size address... 0x%p\n", inStream + LZMA_SIZE_OFFSET);
    debug ("LZMA: Compressed data address..... 0x%p\n", inStream + LZMA_DATA_OFFSET);
    debug ("LZMA: Destination address......... 0x%p\n", outStream);

    memset(&state, 0, sizeof(state));

    outSize = 0;
    outSizeHigh = 0;
    /* Read the uncompressed size */
    for (i = 0; i < 8; i++) {
        unsigned char b = inStream[LZMA_SIZE_OFFSET + i];
            if (i < 4) {
                outSize     += (UInt32)(b) << (i * 8);
        } else {
                outSizeHigh += (UInt32)(b) << ((i - 4) * 8);
        }
    }

    outSizeFull = (SizeT)outSize;
    if (sizeof(SizeT) >= 8) {
        /*
         * SizeT is a 64 bit uint => We can manage files larger than 4GB!
         *
         */
            outSizeFull |= (((SizeT)outSizeHigh << 16) << 16);
    } else if (outSizeHigh != 0 || (UInt32)(SizeT)outSize != outSize) {
        /*
         * SizeT is a 32 bit uint => We cannot manage files larger than
         * 4GB!  Assume however that all 0xf values is "unknown size" and
         * not actually a file of 2^64 bits.
         *
         */
        if (outSizeHigh != (SizeT)-1 || outSize != (SizeT)-1) {
            debug ("LZMA: 64bit support not enabled.\n");
            return SZ_ERROR_DATA;
        }
    }

    debug("LZMA: Uncompresed size............ 0x%zx\n", outSizeFull);
    debug("LZMA: Compresed size.............. 0x%zx\n", compressedSize);

    g_Alloc.Alloc = SzAlloc;
    g_Alloc.Free = SzFree;

    /* Decompress */
    outProcessed = outSizeFull;

    WATCHDOG_RESET();

    res = LzmaDecode(
        outStream, &outProcessed,
        inStream + LZMA_DATA_OFFSET, &compressedSize,
        inStream, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &state, &g_Alloc);
    *uncompressedSize = outProcessed;
    if (res != SZ_OK)  {
        return res;
    }

    return res;
}




/*lzma hard decompress*/
#include <spi.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/clk.h>
#define LZMA_CTRL		0x00
#define LZMA_BS_BASE	0x04
#define LZMA_BS_SIZE	0x08
#define LZMA_DST_BASE	0x0C
#define LZMA_ICRC		0x10
#define LZMA_OCRC		0x14
#define LZMA_TIMEOUT	0x18
#define LZMA_FINAL		0x1c

#define CPM_CLKGR0		0x20
#define CPM_BSCCDR		0xa0

#define LZMA_BASE (0xb3090000)
#define REG32(x) *(volatile unsigned int *)(x)

void dump_lzma_reg(void)
{
	printf("\nDUMP CPM LZMA REG : \n");
	printf("CPM_BSCCDR	= 0x%x \n", REG32(CPM_BASE + CPM_BSCCDR));
	printf("CPM_CLKGR0	= 0x%x \n", REG32(CPM_BASE + CPM_CLKGR0));

	printf("DUMP LZMA REG :  \n");
	printf("LZMA_CTRL     = 0x%x \n", REG32(LZMA_BASE + LZMA_CTRL));
	printf("LZMA_BS_BASE  = 0x%x \n", REG32(LZMA_BASE + LZMA_BS_BASE));
	printf("LZMA_BS_SIZE  = 0x%x \n", REG32(LZMA_BASE + LZMA_BS_SIZE));
	printf("LZMA_DST_BASE = 0x%x \n", REG32(LZMA_BASE + LZMA_DST_BASE));
	printf("LZMA_FINAL	= 0x%x \n", REG32(LZMA_BASE + LZMA_FINAL));
}

int jz_lzma_decompress(unsigned char *src, size_t size, unsigned char *dst)
{
	unsigned int value = 0, outlen = 0;
	unsigned int clk_val = 0, clk_gate = 0;
	/* config clk */
	clk_val = readl(CPM_BASE + CPM_BSCCDR);
	clk_val |= ((2 << 30) | (1 << 29) | (1 <<0));
	writel(clk_val, CPM_BASE + CPM_BSCCDR);

	clk_gate = readl(CPM_BASE + CPM_CLKGR0);
	clk_gate &= ~(1 << 2);
	writel(clk_gate, CPM_BASE + CPM_CLKGR0);

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
	writel(src - 0x80000000 , BSCALER_BASE + LZMA_BS_BASE);
	writel(size, BSCALER_BASE + LZMA_BS_SIZE);
	writel(dst - 0x80000000 , BSCALER_BASE + LZMA_DST_BASE);

	/* close intr and start lzma*/
	writel((1 << 0) | (1 << 3), BSCALER_BASE + LZMA_CTRL);

	/* wait end */
	while(readl(BSCALER_BASE + LZMA_CTRL) & (1 << 0));
	outlen = readl(BSCALER_BASE + LZMA_FINAL);
	//dump_lzma_reg();

	return outlen;
}

#endif
