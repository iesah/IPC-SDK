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

static DEFINE_SPINLOCK(clkgr0_lock);
static DEFINE_SPINLOCK(clkgr1_lock);

struct clk;
struct clk_ops {
	int (*enable)(struct clk *,int);
	struct clk* (*get_parent)(struct clk *);
	int (*set_parent)(struct clk *,struct clk *);
	unsigned long (*get_rate)(struct clk *);
	int (*set_rate)(struct clk *,unsigned long);
};

struct clk {
	const char *name;
	unsigned long rate;
	struct clk *parent;
	unsigned long flags;
#define CLK_FLG_NOALLOC	BIT(0)
#define CLK_FLG_ENABLE	BIT(1)
#define CLK_GATE_BIT(flg)	((flg) >> 24)
#define CLK_FLG_GATE	BIT(2)
#define CLK_CPCCR_NO(flg)	(((flg) >> 16) & 0xff)
#define CLK_FLG_CPCCR	BIT(3)
#define CLK_CGU_NO(flg) 	(((flg) >> 16) & 0xff)
#define CLK_FLG_CGU	BIT(4)
#define CLK_PLL_NO(flg) 	(((flg) >> 16) & 0xff)
#define CLK_FLG_PLL	BIT(5)
#define CLK_PARENT(flg) 	(((flg) >> 16) & 0xff)
#define CLK_FLG_PARENT	BIT(6)
	struct clk_ops *ops;
	int count;
	struct clk *source;
};

enum {
	CLK_ID_EXT0,
#define CLK_NAME_EXT0		"ext0"
	CLK_ID_EXT1,
#define CLK_NAME_EXT1		"ext1"
	CLK_ID_APLL,
#define CLK_NAME_APLL		"apll"
	CLK_ID_MPLL,
#define CLK_NAME_MPLL		"mpll"
	CLK_ID_EPLL,
#define CLK_NAME_EPLL		"epll"
	CLK_ID_VPLL,
#define CLK_NAME_VPLL		"vpll"
	CLK_ID_SCLKA,
#define CLK_NAME_SCLKA		"sclka"
	/**********************************************************************************/
	CLK_ID_CCLK,
#define CLK_NAME_CCLK		"cclk"
	CLK_ID_L2CLK,
#define CLK_NAME_L2CLK		"l2clk"
	CLK_ID_H0CLK,
#define CLK_NAME_H0CLK		"h0clk"
	CLK_ID_H2CLK,
#define CLK_NAME_H2CLK		"h2clk"
	CLK_ID_PCLK,
#define CLK_NAME_PCLK		"pclk"
	/**********************************************************************************/
	CLK_ID_NEMC,
#define CLK_NAME_NEMC		"nemc"
	CLK_ID_BCH,
#define CLK_NAME_BCH		"bch"
	CLK_ID_OTG0,
#define CLK_NAME_OTG0		"otg0"
	CLK_ID_MSC0,
#define CLK_NAME_MSC0		"msc0"
	CLK_ID_SSI0,
#define CLK_NAME_SSI0		"ssi0"
	CLK_ID_I2C0,
#define CLK_NAME_I2C0		"i2c0"
	CLK_ID_I2C1,
#define CLK_NAME_I2C1		"i2c1"
	CLK_ID_SCC,
#define CLK_NAME_SCC		"scc"
	CLK_ID_AIC0,
#define CLK_NAME_AIC0		"aic0"
	CLK_ID_TSSI0,
#define CLK_NAME_TSSI0		"tssi0"
	CLK_ID_OWI,
#define CLK_NAME_OWI		"owi"
	CLK_ID_MSC1,
#define CLK_NAME_MSC1		"msc1"
	CLK_ID_MSC2,
#define CLK_NAME_MSC2		"msc2"
	CLK_ID_KBC,
#define CLK_NAME_KBC		"kbc"
	CLK_ID_SADC,
#define CLK_NAME_SADC		"sadc"
	CLK_ID_UART0,
#define CLK_NAME_UART0		"uart0"
	CLK_ID_UART1,
#define CLK_NAME_UART1		"uart1"
	CLK_ID_UART2,
#define CLK_NAME_UART2		"uart2"
	CLK_ID_UART3,
#define CLK_NAME_UART3		"uart3"
	CLK_ID_SSI1,
#define CLK_NAME_SSI1		"ssi1"
	CLK_ID_SSI2,
#define CLK_NAME_SSI2		"ssi2"
	CLK_ID_PDMA,
#define CLK_NAME_PDMA		"pdma"
	CLK_ID_GPS,
#define CLK_NAME_GPS		"gps"
	CLK_ID_MAC,
#define CLK_NAME_MAC		"mac"
	CLK_ID_UHC,
#define CLK_NAME_UHC		"uhc"
	CLK_ID_OHCI,
#define CLK_NAME_OHCI		"ohci"
	CLK_ID_EHCI,
#define CLK_NAME_EHCI		"ehci"
	CLK_ID_I2C2,
#define CLK_NAME_I2C2		"i2c2"
	CLK_ID_CIM,
#define CLK_NAME_CIM		"cim"
	CLK_ID_LCD1,
#define CLK_NAME_LCD1		"lcd1"
	CLK_ID_LCD0,
#define CLK_NAME_LCD0		"lcd0"
	CLK_ID_IPU0,
#define CLK_NAME_IPU0		"ipu0"
	CLK_ID_IPU1,
#define CLK_NAME_IPU1		"ipu1"
	CLK_ID_DDR0,
#define CLK_NAME_DDR0		"ddr0"
	CLK_ID_DDR1,
#define CLK_NAME_DDR1		"ddr1"
	/**********************************************************************************/
	CLK_ID_I2C3,
#define CLK_NAME_I2C3		"i2c3"
	CLK_ID_TSSI1,
#define CLK_NAME_TSSI1		"tssi1"
	CLK_ID_VPU,
#define CLK_NAME_VPU		"vpu"
	CLK_ID_PCM,
#define CLK_NAME_PCM		"pcm"
	CLK_ID_GPU,
#define CLK_NAME_GPU		"gpu"
	CLK_ID_COMPRESS,
#define CLK_NAME_COMPRESS 	"compress"
	CLK_ID_AIC1,
#define CLK_NAME_AIC1 		"aic1"
	CLK_ID_GPVLC,
#define CLK_NAME_GPVLC		"gpvlc"
	CLK_ID_OTG1,
#define CLK_NAME_OTG1		"otg1"
	CLK_ID_HDMI,
#define CLK_NAME_HDMI		"hdmi"
	CLK_ID_UART4,
#define CLK_NAME_UART4		"uart4"
	CLK_ID_AHB_MON,
#define CLK_NAME_AHB_MON 	"ahb_mon"
	CLK_ID_I2C4,
#define CLK_NAME_I2C4		"i2c4"
	CLK_ID_DES,
#define CLK_NAME_DES		"des"
	CLK_ID_X2D,
#define CLK_NAME_X2D		"x2d"
	CLK_ID_P1,
#define CLK_NAME_P1		"p1"
	/**********************************************************************************/
	CLK_ID_CGU_DDR,
#define CLK_NAME_CGU_DDR	"cgu_ddr"
	CLK_ID_CGU_VPU,
#define CLK_NAME_CGU_VPU	"cgu_vpu"
	CLK_ID_CGU_AIC,
#define CLK_NAME_CGU_AIC	"cgu_aic"
	CLK_ID_CGU_LCD0,
#define CLK_NAME_CGU_LCD0	"lcd_pclk0"
	CLK_ID_CGU_LCD1,
#define CLK_NAME_CGU_LCD1	"lcd_pclk1"
	CLK_ID_MSC_MUX,
#define CLK_NAME_MSC_MUX	"msc_mux"
	CLK_ID_CGU_MSC0,
#define CLK_NAME_CGU_MSC0	"cgu_msc0"
	CLK_ID_CGU_MSC1,
#define CLK_NAME_CGU_MSC1	"cgu_msc1"
	CLK_ID_CGU_MSC2,
#define CLK_NAME_CGU_MSC2	"cgu_msc2"
	CLK_ID_CGU_UHC,
#define CLK_NAME_CGU_UHC	"cgu_uhc"
	CLK_ID_CGU_SSI,
#define CLK_NAME_CGU_SSI	"cgu_ssi"
	CLK_ID_CGU_CIMMCLK,
#define CLK_NAME_CGU_CIMMCLK	"cgu_cimmclk"
	CLK_ID_CGU_PCM,
#define CLK_NAME_CGU_PCM	"cgu_pcm"
	CLK_ID_CGU_GPU,
#define CLK_NAME_CGU_GPU	"cgu_gpu"
	CLK_ID_CGU_HDMI,
#define CLK_NAME_CGU_HDMI	"cgu_hdmi"
	CLK_ID_CGU_BCH,
#define CLK_NAME_CGU_BCH	"cgu_bch"
};

enum {
	CGU_DDR,CGU_VPU,CGU_AIC,CGU_LCD0,CGU_LCD1,CGU_MSC0,CGU_MSC1,CGU_MSC2,
	CGU_UHC,CGU_SSI,CGU_CIMMCLK,CGU_PCM,CGU_GPU,CGU_HDMI,CGU_BCH,CGU_MSC_MUX,
};

	static struct clk clk_srcs[] = {
#define GATE(x)  (((x)<<24) | CLK_FLG_GATE)
#define CPCCR(x) (((x)<<16) | CLK_FLG_CPCCR)
#define CGU(no)  (((no)<<16) | CLK_FLG_CGU)
#define PLL(no)  (((no)<<16) | CLK_FLG_PLL)
#define PARENT(P)  (((CLK_ID_##P)<<16) | CLK_FLG_PARENT)
#define DEF_CLK(N,FLAG)						\
		[CLK_ID_##N] = { .name = CLK_NAME_##N, .flags = FLAG, }

		DEF_CLK(EXT0,  		CLK_FLG_NOALLOC),
		DEF_CLK(EXT1,  		CLK_FLG_NOALLOC),

		DEF_CLK(APLL,  		PLL(CPM_CPAPCR)),
		DEF_CLK(MPLL,  		PLL(CPM_CPMPCR)),
		DEF_CLK(EPLL,  		PLL(CPM_CPEPCR)),
		DEF_CLK(VPLL,  		PLL(CPM_CPVPCR)),

		DEF_CLK(SCLKA,		0),

		DEF_CLK(CCLK,  		CPCCR(0)),
		DEF_CLK(L2CLK,  	CPCCR(1)),
		DEF_CLK(H0CLK,  	CPCCR(2)),
		DEF_CLK(H2CLK, 		CPCCR(3)),
		DEF_CLK(PCLK, 		CPCCR(4)),

		DEF_CLK(NEMC,  		GATE(0) | PARENT(H2CLK)),
		DEF_CLK(BCH,   		GATE(1)),
		DEF_CLK(OTG0,  		GATE(2)),
		DEF_CLK(MSC0,  		GATE(3)),
		DEF_CLK(SSI0,  		GATE(4)),
		DEF_CLK(I2C0,  		GATE(5) | PARENT(PCLK)),
		DEF_CLK(I2C1,  		GATE(6) | PARENT(PCLK)),
		DEF_CLK(SCC,   		GATE(7)),
		DEF_CLK(AIC0,   	GATE(8)),
		DEF_CLK(TSSI0, 		GATE(9)),
		DEF_CLK(OWI,   		GATE(10)),
		DEF_CLK(MSC1,  		GATE(11)),
		DEF_CLK(MSC2,  		GATE(12)),
		DEF_CLK(KBC,   		GATE(13)),
		DEF_CLK(SADC,  		GATE(14)),
		DEF_CLK(UART0, 		GATE(15) | PARENT(EXT1)),
		DEF_CLK(UART1, 		GATE(16) | PARENT(EXT1)),
		DEF_CLK(UART2, 		GATE(17) | PARENT(EXT1)),
		DEF_CLK(UART3, 		GATE(18) | PARENT(EXT1)),
		DEF_CLK(SSI1,  		GATE(19)),
		DEF_CLK(SSI2,  		GATE(20)),
		DEF_CLK(PDMA,  		GATE(21)),
		DEF_CLK(GPS,   		GATE(22)),
		DEF_CLK(MAC,   		GATE(23)),
		DEF_CLK(UHC,   		GATE(24)),
		DEF_CLK(OHCI,   	PARENT(UHC)),
		DEF_CLK(EHCI,   	PARENT(UHC)),
		DEF_CLK(I2C2,  		GATE(25)| PARENT(PCLK)),
		DEF_CLK(CIM,   		GATE(26)),
		DEF_CLK(LCD0,   	GATE(27)| PARENT(LCD1)),
		DEF_CLK(LCD1,   	GATE(28)),
		DEF_CLK(IPU1,   	GATE(29)),
		DEF_CLK(IPU0,   	GATE(29)),
		DEF_CLK(DDR0,  		GATE(30)),
		DEF_CLK(DDR1,  		GATE(31)),
		DEF_CLK(I2C3,  		GATE(32+0)| PARENT(PCLK)),
		DEF_CLK(TSSI1, 		GATE(32+1)),
		DEF_CLK(VPU,		GATE(32+2)),
		DEF_CLK(PCM,		GATE(32+3)),
		DEF_CLK(GPU,		GATE(32+4)),
		DEF_CLK(COMPRESS,	GATE(32+5)),
		DEF_CLK(AIC1,		GATE(32+6)),
		DEF_CLK(GPVLC,		GATE(32+7)),
		DEF_CLK(OTG1,		GATE(32+8)),
		DEF_CLK(HDMI,		GATE(32+9)),
		DEF_CLK(UART4,		GATE(32+10) | PARENT(EXT1)),
		DEF_CLK(AHB_MON,	GATE(32+11)),
		DEF_CLK(I2C4,		GATE(32+12)| PARENT(PCLK)),
		DEF_CLK(DES,		GATE(32+13)),
		DEF_CLK(X2D,		GATE(32+14)),
		DEF_CLK(P1,		GATE(32+15)),

		DEF_CLK(CGU_DDR,	CGU(CGU_DDR)),
		DEF_CLK(CGU_VPU,	CGU(CGU_VPU)),
		DEF_CLK(CGU_AIC,	CGU(CGU_AIC)),
		DEF_CLK(CGU_LCD0,	CGU(CGU_LCD0)),
		DEF_CLK(CGU_LCD1,	CGU(CGU_LCD1)),
		DEF_CLK(MSC_MUX,	CGU(CGU_MSC_MUX)),
		DEF_CLK(CGU_MSC0,	CGU(CGU_MSC0)),
		DEF_CLK(CGU_MSC1,	CGU(CGU_MSC1)),
		DEF_CLK(CGU_MSC2,	CGU(CGU_MSC2)),
		DEF_CLK(CGU_UHC,	CGU(CGU_UHC)),
		DEF_CLK(CGU_SSI,	CGU(CGU_SSI)),
		DEF_CLK(CGU_CIMMCLK,	CGU(CGU_CIMMCLK)),
		DEF_CLK(CGU_PCM,	CGU(CGU_PCM)),
		DEF_CLK(CGU_GPU,	CGU(CGU_GPU)),
		DEF_CLK(CGU_HDMI,	CGU(CGU_HDMI)),
		DEF_CLK(CGU_BCH,	CGU(CGU_BCH)),
#undef GATE
#undef CPCCR
#undef CGU
#undef PARENT
#undef DEF_CLK
	};

static void __init init_ext_pll(void)
{
	int i;
	unsigned long cppcr,cpccr_sel_src;

	clk_srcs[CLK_ID_EXT0].rate = JZ_EXTAL_RTC;
	clk_srcs[CLK_ID_EXT0].flags |= CLK_FLG_ENABLE;
	clk_srcs[CLK_ID_EXT1].rate = JZ_EXTAL;
	clk_srcs[CLK_ID_EXT1].flags |= CLK_FLG_ENABLE;

	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_PLL))
			continue;

		clk_srcs[i].parent = &clk_srcs[CLK_ID_EXT1];
		cppcr = cpm_inl(CLK_PLL_NO(clk_srcs[i].flags));

		if (cppcr & (1 << 4))
			clk_srcs[i].flags |= CLK_FLG_ENABLE;

		if(cppcr & (0x1<<1)) {
			clk_srcs[i].rate = JZ_EXTAL;
		} else {
			unsigned long m,n,o;
			o = (((cppcr) >> 9) & 0xf) + 1;
			n = (((cppcr) >> 13) & 0x3f) + 1;
			m = (((cppcr) >> 19) & 0x7fff) + 1;
			clk_srcs[i].rate = ((JZ_EXTAL/1000) * m / n / o)*1000; /* fix 32bit overflow: (clock/1000)*1000 */
		}
	}

	cpccr_sel_src = cpm_inl(CPM_CPCCR) >> 30;
	if(cpccr_sel_src == 1) {
		clk_srcs[CLK_ID_SCLKA].parent = &clk_srcs[CLK_ID_APLL];
		clk_srcs[CLK_ID_SCLKA].rate = clk_srcs[CLK_ID_APLL].rate;
		clk_srcs[CLK_ID_SCLKA].flags |= CLK_FLG_ENABLE;
	} else if(cpccr_sel_src == 2) {
		clk_srcs[CLK_ID_SCLKA].parent = &clk_srcs[CLK_ID_EXT1];
		clk_srcs[CLK_ID_SCLKA].rate = clk_srcs[CLK_ID_EXT1].rate;
		clk_srcs[CLK_ID_SCLKA].flags |= CLK_FLG_ENABLE;
	} else if(cpccr_sel_src == 3) {
		clk_srcs[CLK_ID_SCLKA].parent = &clk_srcs[CLK_ID_EXT0];
		clk_srcs[CLK_ID_SCLKA].rate = clk_srcs[CLK_ID_EXT0].rate;
		clk_srcs[CLK_ID_SCLKA].flags |= CLK_FLG_ENABLE;
	} else {
		clk_srcs[CLK_ID_SCLKA].rate = 0;
		clk_srcs[CLK_ID_SCLKA].flags &= ~CLK_FLG_ENABLE;
	}
}

struct cppcr_clk {
	short off,sel;
};

static struct cppcr_clk cppcr_clks[] = {
#define CPPCR_CLK(N,O,D)	\
	[N] = { .off = O, .sel = D, }
	CPPCR_CLK(0, 0, 28),
	CPPCR_CLK(1, 4, 28),
	CPPCR_CLK(2, 8, 26),
	CPPCR_CLK(3, 12, 24),
	CPPCR_CLK(4, 16, 24),
};

static unsigned long cpccr_get_rate(struct clk *clk)
{
	unsigned long cpccr = cpm_inl(CPM_CPCCR);
	int v = (cpccr >> cppcr_clks[CLK_CPCCR_NO(clk->flags)].off) & 0xf;

	return clk->parent->rate / (v + 1);
}

struct cpccr_table {
	unsigned long rate;
	unsigned int cpccr;
};

#define CPNR 7
#define MAX_M_DIV 6
#define MAX_CCLK (2016 * 1000 * 1000)
#define MAX_H0CLK (432 * 1000 * 1000)
#define MPLL_DIV(i, h) ((1<<22) | (2<<28) | (2<<26) | ((h-1)<<8) | ((i*2-1)<<4) | (i-1))
#define APLL_DIV(i, h) ((1<<22) | (1<<28) | (1<<26) | ((h-1)<<8) | ((i*2-1)<<4) | (i-1))
#define M_N_OD(m,n,od) (((m-1) << 19) | ((n-1) << 13) | ((od-1) << 9))
#define SET_PARENT(SON, FATHER)								\
	clk_srcs[CLK_ID_##SON].parent = &clk_srcs[CLK_ID_##FATHER]
#define SET_PLL(P, M, N, OD, TO)							\
	cpm_outl(M_N_OD(M, N, OD) | (1 << 0),CPM_CP##P##PCR);				\
	clk_srcs[CLK_ID_##P##PLL].rate = CONFIG_EXTAL_CLOCK * 1000000 / N / OD * M;	\
	while (!(cpm_inl(CPM_CP##P##PCR) & (1 << 4)) && --TO)

static struct cpccr_table cpccr_table[CPNR];

static int cclk_set_rate(struct clk *clk, unsigned long rate)
{
	int i;
	unsigned int cpccr = 0;

	if (clk->rate == rate)
		return 0;
	cpccr = cpm_inl(CPM_CPCCR);

	for (i = 1; i <= MAX_M_DIV; i++) {
		if(rate == clk_srcs[CLK_ID_MPLL].rate / i) {
			cpccr = cpccr & ~(0xf<<26 | 0xfff);
			cpccr |= MPLL_DIV(i, 3);
			cpm_outl(cpccr, CPM_CPCCR);
			SET_PARENT(CCLK, MPLL);
			SET_PARENT(L2CLK, MPLL);
			SET_PARENT(H0CLK, MPLL);
			SET_PARENT(SCLKA, EXT1);
			clk_srcs[CLK_ID_SCLKA].rate = clk_srcs[CLK_ID_EXT1].rate;
			cpm_outl((cpccr & ~(0x3<<30)) | (2<<30),CPM_CPCCR);
			goto out;
		}
	}
	if (rate <= MAX_CCLK) {
		unsigned int h0_div = 1;

		if (rate != clk_srcs[CLK_ID_APLL].rate) {
			int m = rate / 1000000 / CONFIG_EXTAL_CLOCK;
			int timeout = 0xffffff;

			if (clk->parent == &clk_srcs[CLK_ID_SCLKA]){
				cpccr &= ~(0xf<<26 | 0xfff);
				cpccr |= MPLL_DIV(2, 3);
				cpm_outl(cpccr, CPM_CPCCR);
			}
			cpccr &= ~(0x3<<30);
			cpccr |= (0x2<<30);
			cpm_outl(cpccr,CPM_CPCCR);
			SET_PARENT(SCLKA, EXT1);
			SET_PLL(A, m, 1, 1, timeout);
			if (!timeout) {
				pr_err("set APLL %lu timeout\n", rate);
				return -1;
			}
		}
		if (clk_srcs[CLK_ID_SCLKA].parent != &clk_srcs[CLK_ID_APLL]) {
			cpccr = cpccr & ~(0x3<<30);
			cpccr |= 1<<30;
			cpm_outl(cpccr,CPM_CPCCR);
		}
		while (clk_srcs[CLK_ID_APLL].rate / h0_div > MAX_H0CLK)
			h0_div++;
		SET_PARENT(CCLK, SCLKA);
		SET_PARENT(L2CLK, SCLKA);
		SET_PARENT(H0CLK, SCLKA);
		SET_PARENT(SCLKA, APLL);
		clk_srcs[CLK_ID_SCLKA].rate = clk_srcs[CLK_ID_APLL].rate;
		cpccr = cpccr & ~(0xf<<26 | 0xfff);
		cpm_outl(cpccr | APLL_DIV(1, h0_div),CPM_CPCCR);
		goto out;
	}

	pr_err("set cpu clk %lu failed\n", rate);
	return -1;
out:
	clk_srcs[CLK_ID_L2CLK].rate = cpccr_get_rate(&clk_srcs[CLK_ID_L2CLK]);
	clk_srcs[CLK_ID_H0CLK].rate = cpccr_get_rate(&clk_srcs[CLK_ID_H0CLK]);
	clk->rate = cpccr_get_rate(clk);
	return 0;
}

static struct clk_ops clk_cpccr_ops = {
	.get_rate = cpccr_get_rate,
};

static struct clk_ops clk_cclk_ops = {
	.get_rate = cpccr_get_rate,
	.set_rate = cclk_set_rate,
};

static void __init init_cpccr_clk(void)
{
	int i,sel,select[4] = {0,CLK_ID_SCLKA,CLK_ID_MPLL,CLK_ID_EPLL};
	unsigned long cpccr = cpm_inl(CPM_CPCCR);

	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_CPCCR))
			continue;

		sel = (cpccr >> cppcr_clks[CLK_CPCCR_NO(clk_srcs[i].flags)].sel) & 0x3;
		clk_srcs[i].parent = &clk_srcs[select[sel]];
		clk_srcs[i].ops = &clk_cpccr_ops;
		clk_srcs[i].rate = cpccr_get_rate(&clk_srcs[i]);
		clk_srcs[i].flags |= CLK_FLG_ENABLE;
	}

	clk_srcs[CLK_ID_CCLK].ops = &clk_cclk_ops;

	cpccr_table[0].rate = clk_srcs[CLK_ID_MPLL].rate / 1;
	cpccr_table[0].cpccr = (0x2<<28) | (0x1<<4) | (0x0);
	cpccr_table[1].rate = clk_srcs[CLK_ID_MPLL].rate / 2;
	cpccr_table[1].cpccr = (0x2<<28) | (0x3<<4) | (0x1);
	cpccr_table[2].rate = clk_srcs[CLK_ID_MPLL].rate / 3;
	cpccr_table[2].cpccr = (0x2<<28) | (0x5<<4) | (0x2);
	cpccr_table[3].rate = clk_srcs[CLK_ID_MPLL].rate / 4;
	cpccr_table[3].cpccr = (0x2<<28) | (0x7<<4) | (0x3);
	cpccr_table[4].rate = clk_srcs[CLK_ID_MPLL].rate / 5;
	cpccr_table[4].cpccr = (0x2<<28) | (0x9<<4) | (0x4);
	cpccr_table[5].rate = clk_srcs[CLK_ID_MPLL].rate / 6;
	cpccr_table[5].cpccr = (0x2<<28) | (0xb<<4) | (0x5);
	cpccr_table[6].rate = clk_srcs[CLK_ID_MPLL].rate / 7;
	cpccr_table[6].cpccr = (0x2<<28) | (0xd<<4) | (0x6);
}

struct cgu_clk {
	/* off: reg offset. ce: CE offset. coe : coe for div .div: div bit width */
	/* ext: extal/pll sel bit. sels: {select} */
	int off,ce,stop,coe,div,ext,busy,sel[8],cache;
};

static struct cgu_clk cgu_clks[] = {
	[CGU_DDR] = 	{ CPM_DDRCDR, 29, 27, 1, 4, 30, 28, {-1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_VPU] = 	{ CPM_VPUCDR, 29, 27, 1, 4, 30, 28, {CLK_ID_SCLKA,CLK_ID_MPLL,CLK_ID_EPLL}},
	[CGU_AIC] = 	{ CPM_I2SCDR, 29, 27, 1, 8, 30, 28, {CLK_ID_EXT1,CLK_ID_SCLKA,CLK_ID_EXT1,CLK_ID_EPLL}},
	[CGU_LCD0] = 	{ CPM_LPCDR0, 28, 26, 1, 8, 30, 27, {CLK_ID_APLL,CLK_ID_MPLL,CLK_ID_VPLL}},
	[CGU_LCD1] = 	{ CPM_LPCDR1, 28, 26, 1, 8, 30, 27, {CLK_ID_APLL,CLK_ID_MPLL,CLK_ID_VPLL}},
	[CGU_MSC_MUX]=	{ CPM_MSC0CDR, 29, 27, 2, 0, 30, 28, {-1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_MSC0] = 	{ CPM_MSC0CDR, 29, 27, 2, 8, 30, 28, {CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX}},
	[CGU_MSC1] = 	{ CPM_MSC1CDR, 29, 27, 2, 8, 30, 28, {CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX}},
	[CGU_MSC2] = 	{ CPM_MSC2CDR, 29, 27, 2, 8, 30, 28, {CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX}},
	[CGU_UHC] = 	{ CPM_UHCCDR, 29, 27, 1, 8, 30, 28, {CLK_ID_APLL,CLK_ID_MPLL,CLK_ID_EPLL,0}},
	[CGU_SSI] = 	{ CPM_SSICDR, 29, 27, 1, 8, 30, 28, {CLK_ID_EXT1,CLK_ID_EXT1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_CIMMCLK] = { CPM_CIMCDR, 30, 28, 1, 8, 31, 29, {CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_PCM] = 	{ CPM_PCMCDR, 28, 26, 1, 8, 29, 27, {CLK_ID_EXT1,CLK_ID_EXT1,CLK_ID_EXT1,CLK_ID_EXT1,
		CLK_ID_SCLKA,CLK_ID_MPLL,CLK_ID_EPLL,CLK_ID_VPLL}},
	[CGU_GPU] = 	{ CPM_GPUCDR, 29, 27, 1, 4, 30, 28, {-1,CLK_ID_SCLKA,CLK_ID_MPLL,CLK_ID_EPLL}},
	[CGU_HDMI] = 	{ CPM_HDMICDR, 29, 26, 1, 8, 30, 28, {CLK_ID_APLL,CLK_ID_MPLL,CLK_ID_VPLL}},
	[CGU_BCH] = 	{ CPM_BCHCDR, 29, 27, 1, 4, 30, 28, {-1,CLK_ID_SCLKA,CLK_ID_MPLL,CLK_ID_EPLL}},
};

static int cgu_enable(struct clk *clk,int on)
{
	int no = CLK_CGU_NO(clk->flags);

	if(on) {
		if(cgu_clks[no].cache) {
			cpm_outl(cgu_clks[no].cache,cgu_clks[no].off);

			while(cpm_test_bit(cgu_clks[no].busy,cgu_clks[no].off))
				printk("wait stable.[%d][%s]\n",__LINE__,clk->name);
		} else {
			printk("######################################\n");
			printk("cgu clk should set rate before enable!\n");
			printk("######################################\n");
			BUG();
		}
	} else {
		cpm_set_bit(cgu_clks[no].stop,cgu_clks[no].off);
	}

	return 0;
}

static int cgu_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long x,tmp;
	int i,no = CLK_CGU_NO(clk->flags);

	if(clk->parent == &clk_srcs[CLK_ID_EXT1])
		return -1;

	x = (1 << cgu_clks[no].div) - 1;
	tmp = clk->parent->rate / cgu_clks[no].coe;
	for (i = 1; i <= x+1; i++) {
		if ((tmp / i) <= rate)
			break;
	}
	i--;

	x = cpm_inl(cgu_clks[no].off) & ~x;
	x |= i;

	cgu_clks[no].cache = (x & ~(0x1<<cgu_clks[no].stop)) | (0x1<<cgu_clks[no].ce);

	if(cpm_test_bit(cgu_clks[no].ce,cgu_clks[no].off)
			&& !cpm_test_bit(cgu_clks[no].stop,cgu_clks[no].off)) {
		cpm_outl(cgu_clks[no].cache, cgu_clks[no].off);

		while(cpm_test_bit(cgu_clks[no].busy,cgu_clks[no].off))
			printk("wait stable.[%d][%s]\n",__LINE__,clk->name);
	}

	return 0;
}

static unsigned long cgu_get_rate(struct clk *clk)
{
	unsigned long x;
	int no = CLK_CGU_NO(clk->flags);

	if(clk->parent == &clk_srcs[CLK_ID_EXT1])
		return clk->parent->rate;

	if(cgu_clks[no].div == 0)
		return clk_get_rate(clk->parent);

	if(!cgu_clks[no].cache)
		x = cpm_inl(cgu_clks[no].off);
	else
		x = cgu_clks[no].cache;

	x &= (1 << cgu_clks[no].div) - 1;
	x = (x + 1) * cgu_clks[no].coe;
	return clk->parent->rate / x;
}

static struct clk * cgu_get_parent(struct clk *clk)
{
	unsigned int no,cgu,idx,pidx;

	no = CLK_CGU_NO(clk->flags);
	cgu = cpm_inl(cgu_clks[no].off);
	idx = cgu >> cgu_clks[no].ext;
	pidx = cgu_clks[no].sel[idx];

	return &clk_srcs[pidx];
}

static int cgu_set_parent(struct clk *clk, struct clk *parent)
{
	int i,cgu;
	int no = CLK_CGU_NO(clk->flags);
	int clksrc_off = parent - clk_srcs;
	for(i=0;i<8;i++) {
		if(cgu_clks[no].sel[i] == clksrc_off)
			break;
	}

	if(i>=8)
		return -EINVAL;

	cgu = cpm_inl(cgu_clks[no].off);
	cgu &= ((1<<cgu_clks[no].ext) - 1);
	cgu |= i<<cgu_clks[no].ext;
	cpm_outl(cgu,cgu_clks[no].off);

	cgu_set_rate(clk, clk->rate);
	return 0;
}

static struct clk_ops clk_cgu_ops = {
	.enable	= cgu_enable,
	.get_rate = cgu_get_rate,
	.set_rate = cgu_set_rate,
	.get_parent = cgu_get_parent,
	.set_parent = cgu_set_parent,
};

static void __init init_cgu_clk(void)
{
	int i,no;

	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_CGU))
			continue;
		clk_srcs[i].ops = &clk_cgu_ops;
		clk_srcs[i].parent = cgu_get_parent(&clk_srcs[i]);
		clk_srcs[i].rate = cgu_get_rate(&clk_srcs[i]);

		no = CLK_CGU_NO(clk_srcs[i].flags);
		cgu_clks[no].cache = 0;
	}

	clk_srcs[CLK_ID_MSC_MUX].ops = NULL;
}

static void __init init_gate_clk(void)
{
	unsigned long clkgr[2];
	int i;
	clkgr[0] = cpm_inl(CPM_CLKGR0);
	clkgr[1] = cpm_inl(CPM_CLKGR1);
	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_GATE))
			continue;
		if(!clk_srcs[i].rate && clk_srcs[i].parent)
			clk_srcs[i].rate = clk_srcs[i].parent->rate;
	}
}

static unsigned long clkgr0;
static unsigned long clkgr1;
int clk_suspend(void)
{
	clkgr0 = cpm_inl(CPM_CLKGR0);
	clkgr1 = cpm_inl(CPM_CLKGR1);

	cpm_outl((clkgr0 | 0x3fd0ffe0), CPM_CLKGR0);
	udelay(20);
	cpm_outl(0x5fff,CPM_CLKGR1);
	udelay(20);
	return 0;
}

void clk_resume(void)
{
	cpm_outl(clkgr0,CPM_CLKGR0);
	mdelay(5);
	cpm_outl(clkgr1,CPM_CLKGR1);
	mdelay(5);
}

struct syscore_ops clk_pm_ops = {
	.suspend = clk_suspend,
	.resume = clk_resume,
};

void __init init_all_clk(void)
{
	int i,tmp;

	tmp = cpm_inl(CPM_UHCCDR);
	tmp = (tmp & ~(0x3<<30)) | (0x1<<30);
	cpm_outl(tmp,CPM_UHCCDR);

	init_ext_pll();
	init_cpccr_clk();
	init_cgu_clk();
	init_gate_clk();

	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(clk_srcs[i].rate)
			continue;
		if((clk_srcs[i].flags & CLK_FLG_ENABLE)
				&& !(clk_srcs[i].flags & CLK_FLG_GATE))
			clk_srcs[i].count = 1;
		if(clk_srcs[i].flags & CLK_FLG_PARENT) {
			int id = CLK_PARENT(clk_srcs[i].flags);
			clk_srcs[i].parent = &clk_srcs[id];
		}
		if(!clk_srcs[i].parent) {
			clk_srcs[i].parent = &clk_srcs[CLK_ID_EXT0];
		}
		clk_srcs[i].rate = clk_srcs[i].parent->rate;
	}

	register_syscore_ops(&clk_pm_ops);

	printk("CCLK:%luMHz L2CLK:%luMhz H0CLK:%luMHz H2CLK:%luMhz PCLK:%luMhz\n",
			clk_srcs[CLK_ID_CCLK].rate/1000/1000,
			clk_srcs[CLK_ID_L2CLK].rate/1000/1000,
			clk_srcs[CLK_ID_H0CLK].rate/1000/1000,
			clk_srcs[CLK_ID_H2CLK].rate/1000/1000,
			clk_srcs[CLK_ID_PCLK].rate/1000/1000);
}

static int cpm_clear_bit_lock(int bit, unsigned int reg)
{
    if (CPM_CLKGR0 == reg) {
        //printk("%s, %d\n", __func__, __LINE__);
        spin_lock(&clkgr0_lock);
        cpm_clear_bit(bit, reg);
        spin_unlock(&clkgr0_lock);
        return 0;
    }
    if (CPM_CLKGR1 == reg) {
        spin_lock(&clkgr1_lock);
        cpm_clear_bit(bit, reg);
        spin_unlock(&clkgr1_lock);
        return 0;
    }
    return 0;
}

static int cpm_set_bit_lock(int bit, unsigned int reg)
{
    if (CPM_CLKGR0 == reg) {
        spin_lock(&clkgr0_lock);
        cpm_set_bit(bit, reg);
        spin_unlock(&clkgr0_lock);
        return 0;
    }
    if (CPM_CLKGR1 == reg) {
        spin_lock(&clkgr1_lock);
        cpm_set_bit(bit, reg);
        spin_unlock(&clkgr1_lock);
        return 0;
    }
    return 0;
}

static int clk_gate_ctrl(struct clk *clk, int enable)
{
	int bit = CLK_GATE_BIT(clk->flags);
	unsigned int off;

	if(bit/32 == 0)
		off = CPM_CLKGR0;
	else
		off = CPM_CLKGR1;

	/* change clkgr atomic */
	if(enable)
		cpm_clear_bit_lock(bit%32,off);
	else
		cpm_set_bit_lock(bit%32,off);

	return 0;
}

struct clk *clk_get(struct device *dev, const char *id)
{
	int i;
	struct clk *retval = NULL;
	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(!strcmp(id,clk_srcs[i].name)) {
			if(clk_srcs[i].flags & CLK_FLG_NOALLOC)
				return &clk_srcs[i];
			retval = kzalloc(sizeof(struct clk),GFP_KERNEL);
			if(!retval)
				return ERR_PTR(-ENODEV);
			memcpy(retval,&clk_srcs[i],sizeof(struct clk));
			retval->source = &clk_srcs[i];
			retval->count = 0;
			return retval;
		}
	}
	return ERR_PTR(-EINVAL);
}
EXPORT_SYMBOL(clk_get);

int clk_enable(struct clk *clk)
{
	if(!clk)
		return -EINVAL;

	if(clk->source) {
		if(clk->count) {
			return 0;
		}

		clk->count = 1;
		clk = clk->source;
	}

	if(clk->flags & CLK_FLG_ENABLE) {
		clk->count++;
		return 0;
	}

	clk_enable(clk->parent);

	if(clk->flags & CLK_FLG_GATE)
		clk_gate_ctrl(clk,1);

	if(clk->ops && clk->ops->enable)
		clk->ops->enable(clk,1);

	clk->flags |= CLK_FLG_ENABLE;
	clk->count = 1;

	return 0;
}
EXPORT_SYMBOL(clk_enable);

int clk_is_enabled(struct clk *clk)
{
	if(clk->source)
		clk = clk->source;
	return clk->flags & CLK_FLG_ENABLE;
}
EXPORT_SYMBOL(clk_is_enabled);

void clk_disable(struct clk *clk)
{
	if(!clk)
		return;

	if(clk->source) {
		if(!clk->count) {
			return;
		}

		clk->count = 0;
		clk = clk->source;
	}

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
	if(clk->source)
		clk = clk->source;
	return clk? clk->rate: 0;
}
EXPORT_SYMBOL(clk_get_rate);

void clk_put(struct clk *clk)
{
	if(clk && !(clk->flags & CLK_FLG_NOALLOC))
		kfree(clk);
	return;
}
EXPORT_SYMBOL(clk_put);


/*
 * The remaining APIs are optional for machine class support.
 */
long clk_round_rate(struct clk *clk, unsigned long rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	if (!clk)
		return -EINVAL;
	if(clk->source)
		clk = clk->source;
	if (!clk->ops || !clk->ops->set_rate)
		return -EINVAL;

	clk->ops->set_rate(clk, rate);
	clk->rate = clk->ops->get_rate(clk);
	return 0;
}
EXPORT_SYMBOL(clk_set_rate);

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int err;

	if (!clk)
		return -EINVAL;
	if(clk->source)
		clk = clk->source;
	if (!clk->ops || !clk->ops->set_rate)
		return -EINVAL;

	err = clk->ops->set_parent(clk, parent);
	clk->rate = clk->ops->get_rate(clk);
	return err;
}
EXPORT_SYMBOL(clk_set_parent);

struct clk *clk_get_parent(struct clk *clk)
{
	if (!clk)
		return NULL;
	return clk->source->parent;
}
EXPORT_SYMBOL(clk_get_parent);

int cpm_start_ehci(void)
{
	static int has_reset = 0;
	int tmp;

	cpm_clear_bit(20, CPM_USBPCR);

	/* The PLL uses CLKCORE as reference */
	tmp = cpm_inl(CPM_USBPCR1);
	tmp |= (0x3<<26);
	cpm_outl(tmp,CPM_USBPCR1);

	/* selects the reference clock frequency 48M */
	tmp = cpm_inl(CPM_USBPCR1);
	tmp &= ~(0x3<<24);
	switch(JZ_EXTAL) {
		case 24000000:
			tmp |= (1<<24);break;
		case 48000000:
			tmp |= (2<<24);break;
		case 19200000:
			tmp |= (3<<24);break;
		case 12000000:
		default:
			tmp |= (0<<24);break;
	}
	cpm_outl(tmp,CPM_USBPCR1);

	/* port1(uhc) hasn't forced to entered SUSPEND mode */
	cpm_set_bit(6, CPM_OPCR);

	/* The pull-down resistance on D-/D+ of port1 */
	tmp = cpm_inl(CPM_USBPCR1);
	tmp |= (0x3<<22);
	cpm_outl(tmp,CPM_USBPCR1);

	/* select utmi data bus width of port1 to 16bit/30M */
	cpm_set_bit(18, CPM_USBPCR1);

	/* select utmi data bus width of controller to 16bit */
	*((volatile int *) 0xb34900b0) |= (1 << 6);

	/* phy reset */
	cpm_set_bit(22, CPM_USBPCR);
	udelay(30);
	cpm_clear_bit(22, CPM_USBPCR);
	udelay(300);

	if(!has_reset) {
		/* UHC soft reset */
		cpm_set_bit(14, CPM_SRBC);
		udelay(300);
		cpm_clear_bit(14, CPM_SRBC);
		udelay(300);
		has_reset = 1;
	}

	return 0;
}
EXPORT_SYMBOL(cpm_start_ehci);

int cpm_stop_ehci(void)
{
	cpm_clear_bit(6, CPM_OPCR);
	return 0;
}
EXPORT_SYMBOL(cpm_stop_ehci);

int cpm_start_ohci(void)
{
	return cpm_start_ehci();
}
EXPORT_SYMBOL(cpm_start_ohci);

int cpm_stop_ohci(void)
{
	return cpm_stop_ehci();
}
EXPORT_SYMBOL(cpm_stop_ohci);

static int clk_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int len = 0;
	int i;
#define PRINT(ARGS...) len += sprintf (page+len, ##ARGS)
	PRINT("ID NAME       FRE        stat       count     parent\n");
	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		unsigned int mhz = clk_srcs[i].rate / 10000;
		PRINT("%2d %-10s %4d.%02dMHz %3sable   %d %s\n",i,clk_srcs[i].name
				, mhz/100, mhz%100
				, clk_srcs[i].flags & CLK_FLG_ENABLE? "en": "dis"
				, clk_srcs[i].count
				, clk_srcs[i].parent? clk_srcs[i].parent->name: "root");
	}
	PRINT("CLKGR0\t: %08x\nCLKGR1\t: %08x\n",
			cpm_inl(CPM_CLKGR0),cpm_inl(CPM_CLKGR1));
	return len;
}

static int clk_write_proc(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	int ret;
	char buf[32];
	unsigned int cgr0,cgr1;

	if (count > 32)
		count = 32;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	ret = sscanf(buf,"0x%x@0x%x",&cgr0,&cgr1);

	if(ret >= 1) cpm_outl(cgr0,CPM_CLKGR0);
	if(ret == 2) cpm_outl(cgr1,CPM_CLKGR1);

	return count;
}

static int __init init_clk_proc(void)
{
	struct proc_dir_entry *res;

	res = create_proc_entry("clocks", 0444, NULL);
	if (res) {
		res->read_proc = clk_read_proc;
		res->write_proc = clk_write_proc;
		res->data = NULL;
	}
	return 0;
}

module_init(init_clk_proc);

