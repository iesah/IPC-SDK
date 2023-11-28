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

#include <soc/base.h>
#include <soc/extal.h>

#include "clk.h"


static struct clk clk_srcs[] = {
#define GATE(x)  (((x)<<24) | CLK_FLG_GATE)
#define CPCCR(x) (((x)<<24) | CLK_FLG_CPCCR)
#define CGU(no)  (((no)<<24) | CLK_FLG_CGU)
#define PLL(no)  (((no)<<24) | CLK_FLG_PLL)
#define PARENT(P)  (((CLK_ID_##P)<<16) | CLK_FLG_PARENT)
#define DEF_CLK(N,FLAG)						\
		[CLK_ID_##N] = { .name = CLK_NAME_##N, .flags = FLAG, }

	[CLK_ID_EXT] = {.name = "noclk", 1},
		DEF_CLK(EXT0,  		0),
		DEF_CLK(EXT1,  		0),
		DEF_CLK(OTGPHY,		0),

	[CLK_ID_PLL] = {.name = "noclk", 1},
		DEF_CLK(APLL,  		PLL(0)),
		DEF_CLK(MPLL,  		PLL(0)),
		DEF_CLK(SCLKA,		PARENT(APLL)),

	[CLK_ID_CPPCR] = {.name = "noclk", 1},
		DEF_CLK(CCLK,  		CPCCR(0)),
		DEF_CLK(L2CLK,  	CPCCR(0)),
		DEF_CLK(H0CLK,  	CPCCR(H0DIV)),
		DEF_CLK(H2CLK, 		CPCCR(H2DIV)),
		DEF_CLK(PCLK, 		CPCCR(PDIV)),

		DEF_CLK(NFI,   		GATE(0) | PARENT(H2CLK)),
		DEF_CLK(NEMC,  		GATE(1) | PARENT(H2CLK)),
		DEF_CLK(BCH,   		GATE(2) | PARENT(CGU_BCH)),
		DEF_CLK(OTG,   		GATE(3)),
		DEF_CLK(MSC0,  		GATE(4) | PARENT(CGU_MSC_MUX)),
		DEF_CLK(MSC1,  		GATE(5) | PARENT(CGU_MSC_MUX)),
		DEF_CLK(SSI0,  		GATE(6) | PARENT(CGU_SSI)),
		DEF_CLK(SMB0,  		GATE(7) | PARENT(PCLK)),

		DEF_CLK(SMB1,  		GATE(8) | PARENT(PCLK)),
		DEF_CLK(SMB2,  		GATE(9) | PARENT(PCLK)),
		DEF_CLK(SMB3,  		GATE(10) | PARENT(PCLK)),
		DEF_CLK(AIC,  		GATE(11) | PARENT(CGU_I2S)),
		DEF_CLK(MSC2,  		GATE(12) | PARENT(CGU_MSC_MUX)),
		DEF_CLK(SADC,  		GATE(13)),
	DEF_CLK(UART0, 		GATE(14) | PARENT(EXT1)),
	DEF_CLK(UART1, 		GATE(15) | PARENT(EXT1)),
	DEF_CLK(UART2, 		GATE(16) | PARENT(EXT1)),
	DEF_CLK(UART3, 		GATE(17) | PARENT(EXT1)),
	DEF_CLK(UART4, 		GATE(18) | PARENT(EXT1)),
		DEF_CLK(SSI1,  		GATE(19) | PARENT(CGU_SSI)),
		DEF_CLK(SSI2,  		GATE(20) | PARENT(CGU_SSI)),
		DEF_CLK(PDMA,  		GATE(21) | PARENT(CGU_SSI)),
		DEF_CLK(UHC,  		GATE(22) | PARENT(CGU_UHC)),
		DEF_CLK(ISP,  		GATE(23) | PARENT(CGU_ISP)),

		DEF_CLK(LCD,  		GATE(24) | PARENT(CGU_LPC)),
		DEF_CLK(CSI,  		GATE(25)),
		DEF_CLK(DSI,  		GATE(26)),
		DEF_CLK(PCM,  		GATE(27) | PARENT(CGU_PCM)),
		DEF_CLK(DES,  		GATE(28)),
		DEF_CLK(RTC,  		GATE(29)),
		DEF_CLK(TCU,  		GATE(30)),
		DEF_CLK(DDR,  		GATE(31) | PARENT(CGU_DDR)),

	[CLK_ID_DEVICES] = {.name = "gmac", 0},
		DEF_CLK(VPU,  		GATE(32 + 0) | PARENT(CGU_VPU)),
		DEF_CLK(GPU,  		GATE(32 + 1) | PARENT(CGU_GPU)),
		DEF_CLK(IPU,  		GATE(32 + 2)),
		DEF_CLK(AHB_MON, 	GATE(32 + 3)),
		DEF_CLK(EPD, 	        GATE(32 + 4)),
		DEF_CLK(AES, 	        GATE(32 + 5)),
		DEF_CLK(HASH, 	        GATE(32 + 6)),
		DEF_CLK(DMIC, 	        GATE(32 + 7)),

		DEF_CLK(P1,	        GATE(32 + 8)),
		DEF_CLK(P0,	        GATE(32 + 9)),
		DEF_CLK(AHB0,	        GATE(32 + 10)),
		DEF_CLK(SYS_OST,	GATE(32 + 11)),
		DEF_CLK(TCU_EXCLK,	GATE(32 + 12)),
		DEF_CLK(DLINE, 		GATE(32 + 13)),
		DEF_CLK(APB0,  		GATE(32 + 14)),
		DEF_CLK(CPU,  		GATE(32 + 15)),

	[CLK_ID_CGU] = {.name = "noclk", 1},
		DEF_CLK(CGU_MSC_MUX,  	CGU(CGU_MSC_MUX)),
		DEF_CLK(CGU_BCH,	CGU(CGU_BCH)),
		DEF_CLK(CGU_ISP,	CGU(CGU_ISP)),
		DEF_CLK(CGU_GPU,	CGU(CGU_GPU)),
		DEF_CLK(CGU_PCM,	CGU(CGU_PCM)),
		DEF_CLK(CGU_CIM,	CGU(CGU_CIM)),
		DEF_CLK(CGU_SSI,	CGU(CGU_SSI)),
		DEF_CLK(CGU_UHC,	CGU(CGU_UHC)),
		DEF_CLK(CGU_MSC2,	CGU(CGU_MSC2) | PARENT(APLL)),
		DEF_CLK(CGU_MSC1,	CGU(CGU_MSC1) | PARENT(APLL)),
		DEF_CLK(CGU_MSC0,	CGU(CGU_MSC0) | PARENT(APLL)),
		DEF_CLK(CGU_LPC,	CGU(CGU_LPC)),
		DEF_CLK(CGU_I2S,	CGU(CGU_I2S)),
		DEF_CLK(CGU_USB,	CGU(CGU_USB)),
		DEF_CLK(CGU_VPU,	CGU(CGU_VPU)),
		DEF_CLK(CGU_DDR,	CGU(CGU_DDR)),

#undef GATE
#undef CPCCR
#undef CGU
#undef PARENT
#undef DEF_CLK
	};

void __init init_ext_pll(struct clk *clk)
{
	int i;

	clk_srcs[CLK_ID_EXT0].rate = JZ_EXTAL_RTC;
	clk_srcs[CLK_ID_EXT0].flags |= CLK_FLG_ENABLE;
	clk_srcs[CLK_ID_EXT1].rate = JZ_EXTAL;
	clk_srcs[CLK_ID_EXT1].flags |= CLK_FLG_ENABLE;

	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_PLL))
			continue;

		clk_srcs[i].flags |= CLK_FLG_ENABLE;
		clk_srcs[i].parent = &clk_srcs[CLK_ID_EXT1];
	}

	clk_srcs[CLK_ID_SCLKA].flags |= CLK_FLG_ENABLE;

	clk_srcs[CLK_ID_SCLKA].rate = 24000000;
	clk_srcs[CLK_ID_APLL].rate = 24000000;
	clk_srcs[CLK_ID_MPLL].rate = 800000000;
}

void __init init_cpccr_clk(struct clk *clk)
{
	int i;
	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_CPCCR))
			continue;

		clk_srcs[i].rate = 24000000;
		clk_srcs[i].flags |= CLK_FLG_ENABLE;
	}
}

void __init init_cgu_clk(struct clk *clk)
{
	int i;

	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_CGU))
			continue;
		if(clk_srcs[i].flags & CLK_FLG_PARENT) {
			int id = CLK_PARENT(clk_srcs[i].flags);
			clk_srcs[i].parent = &clk_srcs[id];
		}
		if(clk_srcs[i].parent)
			clk_srcs[i].rate = clk_srcs[i].parent->rate;
	}
}

void __init init_gate_clk(struct clk *clk)
{
	int i;
	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_GATE))
			continue;

		clk_srcs[i].flags |= CLK_FLG_ENABLE;

		if(!clk_srcs[i].rate && clk_srcs[i].parent)
			clk_srcs[i].rate = clk_srcs[i].parent->rate;
	}
}

int __init init_all_clk(void)
{
	int i;

	init_ext_pll(NULL);
	init_cpccr_clk(NULL);
	init_cgu_clk(NULL);
	init_gate_clk(NULL);
	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(clk_srcs[i].rate)
			continue;
		if(clk_srcs[i].flags & CLK_FLG_ENABLE)
			clk_srcs[i].count = 1;
		if(clk_srcs[i].flags & CLK_FLG_PARENT) {
			int id = CLK_PARENT(clk_srcs[i].flags);
			clk_srcs[i].parent = &clk_srcs[id];
		}
		if(! clk_srcs[i].parent) {
			clk_srcs[i].parent = &clk_srcs[CLK_ID_EXT0];
			printk(KERN_DEBUG "[CLK] %s no parent.\n",clk_srcs[i].name);
		}
		clk_srcs[i].rate = clk_srcs[i].parent->rate;
	}
	return 0;
}
arch_initcall(init_all_clk);

static int clk_gate_ctrl(struct clk *clk, int enable)
{
	return 0;
}

struct clk *clk_get(struct device *dev, const char *id)
{
	int i;

	for(i = 1; i < ARRAY_SIZE(clk_srcs); i++) {
		if(clk_srcs[i].flags & CLK_FLG_NOALLOC)
			continue;

		if(strcmp(id, clk_srcs[i].name) == 0) {
			clk_srcs[i].flags |= CLK_FLG_NOALLOC;
			return &clk_srcs[i];
		}
	}

	return ERR_PTR(-EINVAL);
}
EXPORT_SYMBOL(clk_get);

int clk_enable(struct clk *clk)
{
	if(!clk)
		return -EINVAL;

	clk->count++;

	if(clk->flags & CLK_FLG_ENABLE)
		return 0;

	clk_enable(clk->parent);

	if(clk->flags & CLK_FLG_GATE)
		clk_gate_ctrl(clk,1);

	if(clk->ops && clk->ops->enable)
		clk->ops->enable(clk,1);

	clk->count = 1;
	clk->flags |= CLK_FLG_ENABLE;

	return 0;
}
EXPORT_SYMBOL(clk_enable);

int clk_is_enabled(struct clk *clk)
{
	if(strcmp(clk->name, "otg1") == 0)
		return clk->flags & CLK_FLG_ENABLE;
	else
		return 0;
}
EXPORT_SYMBOL(clk_is_enabled);

void clk_disable(struct clk *clk)
{
	if(!clk)
		return;

	if(clk->count > 1) {
		clk->count--;
		return;
	}

	if(clk->flags & CLK_FLG_GATE)
		clk_gate_ctrl(clk,0);

	if(clk->ops && clk->ops->enable)
		clk->ops->enable(clk,0);

	clk->count = 0;
	clk->flags &= ~CLK_FLG_ENABLE;

	clk_disable(clk->parent);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	return clk? clk->rate: 0;
}
EXPORT_SYMBOL(clk_get_rate);

void clk_put(struct clk *clk)
{
	if(clk)
		clk->flags &= ~CLK_FLG_NOALLOC;
	return;
}
EXPORT_SYMBOL(clk_put);


/*
 * The remaining APIs are optional for machine class support.
 */
long clk_round_rate(struct clk *clk, unsigned long rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	if (!clk || !clk->ops || !clk->ops->set_rate)
		return -EINVAL;

	clk->ops->set_rate(clk, rate);
	clk->rate = clk->ops->get_rate(clk);

	return 0;
}
EXPORT_SYMBOL(clk_set_rate);

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int err;

	if (!clk || !clk->ops || !clk->ops->set_parent)
		return -EINVAL;

	err = clk->ops->set_parent(clk, parent);
	clk->rate = clk->ops->get_rate(clk);

	return err;
}
EXPORT_SYMBOL(clk_set_parent);

struct clk *clk_get_parent(struct clk *clk)
{
	return clk? clk->parent: NULL;
}
EXPORT_SYMBOL(clk_get_parent);
