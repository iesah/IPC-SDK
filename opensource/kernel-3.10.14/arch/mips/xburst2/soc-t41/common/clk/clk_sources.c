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
	DEF_CLK(OTGPHY,     CLK_FLG_NOALLOC),

	DEF_CLK(APLL,  		PLL(CPM_CPAPCR)),
	DEF_CLK(MPLL,  		PLL(CPM_CPMPCR)),
	DEF_CLK(VPLL,  		PLL(CPM_CPVPCR)),

	DEF_CLK(SCLKA,		CPCCR(SCLKA)),
	DEF_CLK(CCLK,  		CPCCR(CDIV)),
	DEF_CLK(L2CLK,  	CPCCR(L2CDIV)),
	DEF_CLK(H0CLK,  	CPCCR(H0DIV)),
	DEF_CLK(H2CLK, 		CPCCR(H2DIV)),
	DEF_CLK(PCLK, 		CPCCR(PDIV)),

//	DEF_CLK(MSC,  		GATE(32 + 16)),
	DEF_CLK(NEMC,	    GATE(0) | PARENT(H2CLK)),
	DEF_CLK(EFUSE,	    GATE(1) | PARENT(H2CLK)),
	DEF_CLK(HASH,       GATE(2) | PARENT(H2CLK)),
	DEF_CLK(OTG,   		GATE(3) | PARENT(H2CLK)),
	DEF_CLK(MSC0,  		GATE(4) | PARENT(CGU_MSC0)),
	DEF_CLK(MSC1,  		GATE(5) | PARENT(CGU_MSC1)),
	DEF_CLK(SSI0,       GATE(6) | PARENT(CGU_SSI)),

	DEF_CLK(I2C0,  		GATE(7) | PARENT(PCLK)),
	DEF_CLK(I2C1,  		GATE(8) | PARENT(PCLK)),
	DEF_CLK(I2C2,  		GATE(9) | PARENT(PCLK)),
	DEF_CLK(SSI_SLV,    GATE(10) | PARENT(PCLK)),
	DEF_CLK(AIC,  		GATE(11) | PARENT(PCLK)),
	DEF_CLK(DMIC,  		GATE(12) | PARENT(H2CLK)),
	DEF_CLK(SADC,  		GATE(13) | PARENT(PCLK)),
	DEF_CLK(UART0,  	GATE(14) | PARENT(EXT1)),
	DEF_CLK(UART1,  	GATE(15) | PARENT(EXT1)),
	DEF_CLK(UART2,  	GATE(16) | PARENT(EXT1)),
	DEF_CLK(UART3,  	GATE(17) | PARENT(EXT1)),
	DEF_CLK(UART4,  	GATE(18) | PARENT(EXT1)),
	DEF_CLK(UART5,  	GATE(19) | PARENT(EXT1)),

	DEF_CLK(SSI1,       GATE(20) | PARENT(CGU_SSI)),
	DEF_CLK(SFC0,		GATE(21) | PARENT(CGU_SFC0)),
	DEF_CLK(PDMA,  		GATE(22)),
	DEF_CLK(ISP,  		GATE(23) | PARENT(CGU_ISPM)),

	DEF_CLK(LCD,  		GATE(24) | PARENT(CGU_LCD)),
	DEF_CLK(CSI,  		GATE(25) | PARENT(PCLK)),
	DEF_CLK(VO,  	    GATE(26) | PARENT(CGU_BT0)),
	DEF_CLK(RSA,  		GATE(27) | PARENT(CGU_RSA)),
	DEF_CLK(DES,  		GATE(28) | PARENT(PCLK)),
	DEF_CLK(RTC,  		GATE(29) | PARENT(PCLK)),
	DEF_CLK(TCU,  		GATE(30) | PARENT(PCLK)),
	DEF_CLK(DDR,  		GATE(31) | PARENT(CGU_DDR)),

	DEF_CLK(AVPU,  	    GATE(32 + 0) | PARENT(CGU_VPU)),
	DEF_CLK(DTRNG,  	GATE(32 + 1) | PARENT(PCLK)),
	DEF_CLK(IPU,  		GATE(32 + 2) | PARENT(H0CLK)),
	DEF_CLK(LZMA,  		GATE(32 + 3) | PARENT(CGU_ISPA)),
	DEF_CLK(GMAC,  		GATE(32 + 4) | PARENT(H2CLK)),
	DEF_CLK(AES, 	    GATE(32 + 5) | PARENT(H2CLK)),

	DEF_CLK(DBOX,		GATE(32 + 6) | PARENT(H0CLK)),
    DEF_CLK(PWM,	    GATE(32 + 7) | PARENT(CGU_PWM)),
    DEF_CLK(I2D,	    GATE(32 + 8) | PARENT(H0CLK)),
    DEF_CLK(BUSMON,	    GATE(32 + 9)),
    DEF_CLK(AHB0,	    GATE(32 + 10) | PARENT(H0CLK)),
	DEF_CLK(SYS_OST,	GATE(32 + 11)),
	DEF_CLK(SFC1,  		GATE(32 + 12) | PARENT(CGU_SFC1)),
	DEF_CLK(LDC,  		GATE(32 + 13) | PARENT(H0CLK)),
	DEF_CLK(JPEG,  		GATE(32 + 14) | PARENT(CGU_ISPA)),
	DEF_CLK(APB0,  		GATE(32 + 15) | PARENT(PCLK)),
	DEF_CLK(CPU,  		GATE(32 + 16) | PARENT(CCLK)),
	DEF_CLK(IVDC,  		GATE(32 + 17) | PARENT(CGU_ISPA)),

	DEF_CLK(CGU_ISPM,	CGU(CGU_ISPM)),
	DEF_CLK(CGU_ISPA,	CGU(CGU_ISPA)),
	DEF_CLK(CGU_ISPS,	CGU(CGU_ISPS)),
	DEF_CLK(CGU_CIM,	CGU(CGU_CIM)),
	DEF_CLK(CGU_MSC_MUX,  	CGU(CGU_MSC_MUX)),
	DEF_CLK(CGU_SSI,	CGU(CGU_SSI)),
	DEF_CLK(CGU_I2S_SPK,	CGU(CGU_I2S_SPK)),
	DEF_CLK(CGU_I2S_MIC,	CGU(CGU_I2S_MIC)),
	DEF_CLK(CGU_MSC1,	CGU(CGU_MSC1)),
	DEF_CLK(CGU_MSC0,	CGU(CGU_MSC0)),
	DEF_CLK(CGU_LCD,	CGU(CGU_LCD)),
	DEF_CLK(CGU_MACPHY,	CGU(CGU_MACPHY)),
	DEF_CLK(CGU_VPU,	CGU(CGU_VPU)),
	DEF_CLK(CGU_DDR,	CGU(CGU_DDR)),
	DEF_CLK(CGU_RSA,	CGU(CGU_RSA)),
	DEF_CLK(CGU_SFC0,	CGU(CGU_SFC0)),
	DEF_CLK(CGU_SFC1,	CGU(CGU_SFC1)),
	DEF_CLK(CGU_BT0,	CGU(CGU_BT0)),
	DEF_CLK(CGU_PWM,	CGU(CGU_PWM)),
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


