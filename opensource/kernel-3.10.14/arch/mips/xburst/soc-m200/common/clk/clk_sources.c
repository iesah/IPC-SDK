/*
 * JZSOC Clock and Power Manager
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/syscore_ops.h>

#include <soc/cpm.h>
#include <soc/base.h>
#include <soc/extal.h>
#include "clk.h"
/*
 *     31 ... 24   GATE_ID or CPCCR_ID or CGU_ID or PLL_ID or CGU_ID.
 *     23 ... 16   PARENR_ID or RELATIVE_ID.
 *     16 ... 0    some FLG.
 */

static struct clk clk_srcs[] = {
#define GATE(x)  (((x)<<24) | CLK_FLG_GATE)
#define CPCCR(x) (((x)<<24) | CLK_FLG_CPCCR)
#define CGU(no)  (((no)<<24) | CLK_FLG_CGU)
#define PLL(no)  (((no)<<24) | CLK_FLG_PLL)
#define PWC(no)  (((no)<<24) | CLK_FLG_PWC)
#define PARENT(P)  (((CLK_ID_##P)<<16) | CLK_FLG_PARENT)
#define RELATIVE(P)  (((CLK_ID_##P)<<16) | CLK_FLG_RELATIVE)
#define DEF_CLK(N,FLAG)						\
	[CLK_ID_##N] = { .name = CLK_NAME_##N, .flags = FLAG, }

	DEF_CLK(EXT0,  		CLK_FLG_NOALLOC),
	DEF_CLK(EXT1,  		CLK_FLG_NOALLOC),
	DEF_CLK(OTGPHY,         CLK_FLG_NOALLOC),

	DEF_CLK(APLL,  		PLL(CPM_CPAPCR)),
	DEF_CLK(MPLL,  		PLL(CPM_CPMPCR)),

	DEF_CLK(SCLKA,		CPCCR(SCLKA)),
	DEF_CLK(CCLK,  		CPCCR(CDIV)),
	DEF_CLK(L2CLK,  	CPCCR(L2CDIV)),
	DEF_CLK(H0CLK,  	CPCCR(H0DIV)),
	DEF_CLK(H2CLK, 		CPCCR(H2DIV)),
	DEF_CLK(PCLK, 		CPCCR(PDIV)),

	DEF_CLK(MSC,  		GATE(32 + 16) | PARENT(PCLK)),
	DEF_CLK(NFI,   		GATE(0) | PARENT(H2CLK)),
	DEF_CLK(NEMC,  		GATE(1) | PARENT(H2CLK)),
	DEF_CLK(BCH,   		GATE(2) | PARENT(H2CLK)),
	DEF_CLK(OTG,   		GATE(3) | PARENT(CGU_USB)),
	DEF_CLK(MSC0,  		GATE(4) | PARENT(MSC)),
	DEF_CLK(MSC1,  		GATE(5) | PARENT(MSC)),
	DEF_CLK(SSI0,  		GATE(6) | PARENT(PCLK)),
	DEF_CLK(I2C0,  		GATE(7) | PARENT(PCLK)),

	DEF_CLK(I2C1,  		GATE(8) | PARENT(PCLK)),
	DEF_CLK(I2C2,  		GATE(9) | PARENT(PCLK)),
	DEF_CLK(I2C3,  		GATE(10) | PARENT(PCLK)),
	DEF_CLK(AIC,  		GATE(11) | PARENT(PCLK)),
	DEF_CLK(MSC2,  		GATE(12) | PARENT(MSC)),
	DEF_CLK(SADC,  		GATE(13) | PARENT(PCLK)),
	DEF_CLK(UART0,  	GATE(14) | PARENT(EXT1)),
	DEF_CLK(UART1,  	GATE(15) | PARENT(EXT1)),

	DEF_CLK(UART2,  	GATE(16) | PARENT(EXT1)),
	DEF_CLK(UART3,  	GATE(17) | PARENT(EXT1)),
	DEF_CLK(UART4,  	GATE(18) | PARENT(EXT1)),
	DEF_CLK(SSI1,  		GATE(19) | PARENT(PCLK)),
	DEF_CLK(SSI2,  		GATE(20) | PARENT(PCLK)),
	DEF_CLK(PDMA,  		GATE(21) | PARENT(PCLK)),
	DEF_CLK(UHC,  		GATE(22) | PARENT(PCLK)),
	DEF_CLK(ISP,  		GATE(23) | PARENT(PCLK)),

	DEF_CLK(LCD,  		GATE(24) | PARENT(H0CLK)),
	DEF_CLK(CSI,  		GATE(25) | PARENT(PCLK)),
	DEF_CLK(DSI,  		GATE(26) | PARENT(EXT1)),
	DEF_CLK(PCM,  		GATE(27) | PARENT(H0CLK)),
	DEF_CLK(DES,  		GATE(28) | PARENT(PCLK)),
	DEF_CLK(RTC,  		GATE(29) | PARENT(EXT0)),
	DEF_CLK(TCU,  		GATE(30) | PARENT(PCLK)),
	DEF_CLK(DDR,  		GATE(31) | PARENT(PCLK)),

	DEF_CLK(VPU,  		GATE(32 + 0) | PARENT(PCLK)),
	DEF_CLK(GPU,  		GATE(32 + 1) | PARENT(PCLK)),
	DEF_CLK(IPU,  		GATE(32 + 2) | PARENT(LCD)),
	DEF_CLK(AHB_MON, 	GATE(32 + 3) | PARENT(PCLK)),
	DEF_CLK(EPD, 	        GATE(32 + 4) | PARENT(PCLK)),
	DEF_CLK(AES, 	        GATE(32 + 5) | PARENT(PCLK)),
	DEF_CLK(HASH, 	        GATE(32 + 6) | PARENT(PCLK)),
	DEF_CLK(DMIC, 	        GATE(32 + 7) | PARENT(PCLK)),

	DEF_CLK(P1,	        GATE(32 + 8)),
	DEF_CLK(P0,	        GATE(32 + 9)),
	DEF_CLK(AHB0,	        GATE(32 + 10)),
	DEF_CLK(SYS_OST,	GATE(32 + 11)),
	DEF_CLK(TCU_EXCLK,	GATE(32 + 12)),
	DEF_CLK(DLINE, 		GATE(32 + 13)),
	DEF_CLK(APB0,  		GATE(32 + 14)),
	DEF_CLK(CPU,  		GATE(32 + 15)),

	DEF_CLK(CGU_MSC_MUX,  	CGU(CGU_MSC_MUX)),
	DEF_CLK(CGU_BCH,	CGU(CGU_BCH)),
	DEF_CLK(CGU_ISP,	CGU(CGU_ISP)),
	DEF_CLK(CGU_GPU,	CGU(CGU_GPU)),
	DEF_CLK(CGU_PCM,	CGU(CGU_PCM)),
	DEF_CLK(CGU_CIM,	CGU(CGU_CIM)),
	DEF_CLK(CGU_SSI,	CGU(CGU_SSI)),
	DEF_CLK(CGU_UHC,	CGU(CGU_UHC)),
	DEF_CLK(CGU_MSC2,	CGU(CGU_MSC2) | PARENT(CGU_MSC_MUX)),
	DEF_CLK(CGU_MSC1,	CGU(CGU_MSC1) | PARENT(CGU_MSC_MUX)),
	DEF_CLK(CGU_MSC0,	CGU(CGU_MSC0) | PARENT(CGU_MSC_MUX)),
	DEF_CLK(CGU_LPC,	CGU(CGU_LPC)),
	DEF_CLK(CGU_I2S,	CGU(CGU_I2S)),
	DEF_CLK(CGU_USB,	CGU(CGU_USB)),
	DEF_CLK(CGU_VPU,	CGU(CGU_VPU)),
	DEF_CLK(CGU_DDR,	CGU(CGU_DDR)),

	DEF_CLK(PWC_P0,         PWC(PWC_P0) | RELATIVE(P0)),
	DEF_CLK(PWC_P1,         PWC(PWC_P1) | RELATIVE(P1)),
	DEF_CLK(PWC_VPU,        PWC(PWC_VPU) | RELATIVE(VPU)),
	DEF_CLK(PWC_GPU,        PWC(PWC_GPU) | RELATIVE(GPU)),
	DEF_CLK(PWC_ISP,        PWC(PWC_ISP) | RELATIVE(ISP)),
	DEF_CLK(PWC_IPU,        PWC(PWC_IPU) | RELATIVE(IPU)),
	DEF_CLK(PWC_DMIC,       PWC(PWC_DMIC) | RELATIVE(DMIC)),
	DEF_CLK(PWC_BCH,        PWC(PWC_BCH)  | RELATIVE(BCH)),
	DEF_CLK(PWC_HASH,       PWC(PWC_HASH) | RELATIVE(HASH)),
	DEF_CLK(PWC_LCD,        PWC(PWC_LCD) | RELATIVE(LCD)),
	DEF_CLK(PWC_USB,        PWC(PWC_USB) | RELATIVE(CGU_USB)),
	DEF_CLK(PWC_UHC,        PWC(PWC_UHC) | RELATIVE(CGU_UHC)),

#undef GATE
#undef CPCCR
#undef CGU
#undef PWC
#undef PARENT
#undef DEF_CLK
#undef RELATIVE
};
int get_clk_sources_size(void){
	return ARRAY_SIZE(clk_srcs);
}
struct clk *get_clk_from_id(int clk_id)
{
	return &clk_srcs[clk_id];
}
int get_clk_id(struct clk *clk)
{
	return (clk - &clk_srcs[0]);
}


