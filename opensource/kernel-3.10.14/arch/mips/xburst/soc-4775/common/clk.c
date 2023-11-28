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
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/syscore_ops.h>

#include <soc/cpm.h>
#include <soc/base.h>
#include <soc/extal.h>
#include <jz_proc.h>

static DEFINE_SPINLOCK(clkgr_lock);

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
	CLK_ID_OTG,
#define CLK_NAME_OTG		"otg1"
	CLK_ID_MSC0,
#define CLK_NAME_MSC0		"msc0"
	CLK_ID_SSI0,
#define CLK_NAME_SSI0		"ssi0"
	CLK_ID_I2C0,
#define CLK_NAME_I2C0		"i2c0"
	CLK_ID_I2C1,
#define CLK_NAME_I2C1		"i2c1"
	CLK_ID_I2C2,
#define CLK_NAME_I2C2		"i2c2"
	CLK_ID_AIC,
#define CLK_NAME_AIC		"aic"//"aic0"
	CLK_ID_X2D,
#define CLK_NAME_X2D		"x2d"
	CLK_ID_AHB_MON,
#define CLK_NAME_AHB_MON 	"ahb_mon"
	CLK_ID_MSC1,
#define CLK_NAME_MSC1		"msc1"
	CLK_ID_MSC2,
#define CLK_NAME_MSC2		"msc2"
	CLK_ID_PCM,
#define CLK_NAME_PCM		"pcm"
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
	CLK_ID_VPU,
#define CLK_NAME_VPU		"vpu"
	CLK_ID_PDMA,
#define CLK_NAME_PDMA		"pdma"
	CLK_ID_GMAC,
#define CLK_NAME_GMAC		"gmac"
	CLK_ID_UHC,
#define CLK_NAME_UHC		"uhc"
	CLK_ID_OHCI,
#define CLK_NAME_OHCI		"ohci"
	CLK_ID_EHCI,
#define CLK_NAME_EHCI		"ehci"
	CLK_ID_CIM0,
#define CLK_NAME_CIM0		"cim0"
	CLK_ID_CIM1,
#define CLK_NAME_CIM1		"cim1"
	CLK_ID_LCD,
#define CLK_NAME_LCD		"lcd0"
	CLK_ID_EPDC,
#define CLK_NAME_EPDC		"epdc"
	CLK_ID_EPDE,
#define CLK_NAME_EPDE		"epde"
	CLK_ID_DDR,
#define CLK_NAME_DDR		"ddr0"

/**********************************************************************************/
	CLK_ID_CGU_DDR,
#define CLK_NAME_CGU_DDR	"cgu_ddr"
	CLK_ID_CGU_VPU,
#define CLK_NAME_CGU_VPU	"cgu_vpu"
	CLK_ID_CGU_AIC,
#define CLK_NAME_CGU_AIC	"cgu_i2s" /*cgu_aic*/
	CLK_ID_CGU_LCD,
#define CLK_NAME_CGU_LCD	"lcd_pclk0"
	CLK_ID_MSC_MUX,
#define CLK_NAME_MSC_MUX	"msc_mux"
	CLK_ID_CGU_MSC0,
#define CLK_NAME_CGU_MSC0	"cgu_msc0"
	CLK_ID_CGU_MSC1,
#define CLK_NAME_CGU_MSC1	"cgu_msc1"
	CLK_ID_CGU_MSC2,
#define CLK_NAME_CGU_MSC2	"cgu_msc2"
	CLK_ID_CGU_USB,
#define CLK_NAME_CGU_USB	"cgu_usb"
	CLK_ID_CGU_UHC,
#define CLK_NAME_CGU_UHC	"cgu_uhc"
	CLK_ID_CGU_SSI,
#define CLK_NAME_CGU_SSI	"cgu_ssi"
	CLK_ID_CIM_MUX,
#define CLK_NAME_CIM_MUX	"cim mux"
	CLK_ID_CGU_CIMMCLK0,
#define CLK_NAME_CGU_CIMMCLK0	"cgu_cimmclk0"
	CLK_ID_CGU_CIMMCLK1,
#define CLK_NAME_CGU_CIMMCLK1	"cgu_cimmclk1"
	CLK_ID_CGU_PCM,
#define CLK_NAME_CGU_PCM	"cgu_pcm"
	CLK_ID_CGU_BCH,
#define CLK_NAME_CGU_BCH	"cgu_bch"
};


enum {
	CGU_DDR,CGU_VPU,CGU_AIC,CGU_LCD,CGU_MSC0,CGU_MSC1,CGU_MSC2,CGU_USB,
	CGU_UHC,CGU_SSI,CGU_CIMMCLK0,CGU_CIMMCLK1,CGU_PCM,CGU_BCH,CGU_MSC_MUX,CGU_CIM_MUX,
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

		DEF_CLK(SCLKA,		0),

		DEF_CLK(CCLK,  		CPCCR(0)),
		DEF_CLK(L2CLK,  	CPCCR(1)),
		DEF_CLK(H0CLK,  	CPCCR(2)),
		DEF_CLK(H2CLK, 		CPCCR(3)),
		DEF_CLK(PCLK, 		CPCCR(4)),

		DEF_CLK(NEMC,  		GATE(0) | PARENT(H2CLK)),
		DEF_CLK(BCH,   		GATE(1)),
		DEF_CLK(OTG,  		GATE(2)),
		DEF_CLK(MSC0,  		GATE(3)),
		DEF_CLK(SSI0,  		GATE(4)),
		DEF_CLK(I2C0,  		GATE(5) | PARENT(PCLK)),
		DEF_CLK(I2C1,  		GATE(6) | PARENT(PCLK)),
		DEF_CLK(I2C2,  		GATE(7) | PARENT(PCLK)),
		DEF_CLK(AIC,		GATE(8)),
		DEF_CLK(X2D, 		GATE(9)),
		DEF_CLK(AHB_MON,   	GATE(10)),
		DEF_CLK(MSC1,  		GATE(11)),
		DEF_CLK(MSC2,  		GATE(12)),
		DEF_CLK(PCM,   		GATE(13)),
		DEF_CLK(SADC,  		GATE(14)),
		DEF_CLK(UART0, 		GATE(15) | PARENT(EXT1)),
		DEF_CLK(UART1, 		GATE(16) | PARENT(EXT1)),
		DEF_CLK(UART2, 		GATE(17) | PARENT(EXT1)),
		DEF_CLK(UART3, 		GATE(18) | PARENT(EXT1)),
		DEF_CLK(VPU,  		GATE(19)),
		DEF_CLK(PDMA,  		GATE(20)),
		DEF_CLK(GMAC,  		GATE(21)),
		DEF_CLK(UHC,   		GATE(22)),
		DEF_CLK(OHCI,   	PARENT(UHC)),
		DEF_CLK(EHCI,   	PARENT(UHC)),
		DEF_CLK(CIM0,   	GATE(23)),
		DEF_CLK(CIM1,   	GATE(24)),
		DEF_CLK(LCD,  		GATE(25)),
		DEF_CLK(EPDC,   	GATE(26)),
		DEF_CLK(EPDE,   	GATE(27)),
		DEF_CLK(DDR,		GATE(31)),

		DEF_CLK(CGU_DDR,	CGU(CGU_DDR)),
		DEF_CLK(CGU_VPU,	CGU(CGU_VPU)),
		DEF_CLK(CGU_AIC,	CGU(CGU_AIC)),
		DEF_CLK(CGU_LCD,	CGU(CGU_LCD)),
		DEF_CLK(MSC_MUX,	CGU(CGU_MSC_MUX)),
		DEF_CLK(CGU_MSC0,	CGU(CGU_MSC0)),
		DEF_CLK(CGU_MSC1,	CGU(CGU_MSC1)),
		DEF_CLK(CGU_MSC2,	CGU(CGU_MSC2)),
		DEF_CLK(CGU_USB,	CGU(CGU_USB)),
		DEF_CLK(CGU_UHC,	CGU(CGU_UHC)),
		DEF_CLK(CGU_SSI,	CGU(CGU_SSI)),
		DEF_CLK(CIM_MUX,	CGU(CGU_CIM_MUX)),
		DEF_CLK(CGU_CIMMCLK0,CGU(CGU_CIMMCLK0)),
		DEF_CLK(CGU_CIMMCLK1,CGU(CGU_CIMMCLK1)),
		DEF_CLK(CGU_PCM,	CGU(CGU_PCM)),
		DEF_CLK(CGU_BCH,	CGU(CGU_BCH)),
#undef GATE
#undef CPCCR
#undef CGU
#undef PARENT
#undef DEF_CLK
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

		if (cppcr & (1 << 10))	//pll on and stable
			clk_srcs[i].flags |= CLK_FLG_ENABLE;

		if(cppcr & (0x1<< 9)) {
			clk_srcs[i].rate = JZ_EXTAL;
		} else {
			unsigned long m,n,o;
			o = (1 << (((cppcr) >> 16) & 0x3));
			n = (((cppcr) >> 18) & 0x1f) + 1;
			m = (((cppcr) >> 24) & 0x7f) + 1;
			clk_srcs[i].rate = ((JZ_EXTAL/1000) * m / n / o)*1000; /* fix 32bit overflow: (clock/1000)*1000 */
		}
	}

	/*sclk_a mux select src*/
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

#define MAX_M_DIV 6
#define MAX_CCLK (1000 * 1000 * 1000)
#define CCLK_MPLL_DIV(i) ((1<<22) | (2<<28) | ((i*2-1)<<4) | (i-1))
#define CCLK_APLL_DIV(i) ((1<<22) | (1<<28) | ((i*2-1)<<4) | (i-1))
#define SET_PARENT(SON, FATHER) \
	clk_srcs[CLK_ID_##SON].parent = &clk_srcs[CLK_ID_##FATHER]

/*
 * nf = OM + 1
 * nr = ON + 1
 * no = 2^OD
 */

#define M_N_OD(nf,nr,no) (((nf-1) << 24) | ((nr-1) << 18) | ((no/2 == 4 ? 3 : no/2) << 16))	// no == 1,2,4,8


#define SET_APLL(BS,NF, NR, NO, TO)							\
	cpm_outl((BS << 31) | M_N_OD(NF, NR, NO) | (1 << 8),CPM_CPAPCR);				\
	clk_srcs[CLK_ID_APLL].rate = CONFIG_EXTAL_CLOCK * 1000000 / NR / NO * NF;	\
	while (!(cpm_inl(CPM_CPAPCR) & (1 << 10)) && --TO)

// we can not support kernel channge mpll clk
#define SET_MPLL(BS,NF, NR, NO, TO)							\
	cpm_outl((BS << 31) | M_N_OD(NF, NR, NO) | (1 << 7),CPM_CPMPCR);				\
	clk_srcs[CLK_ID_MPLL].rate = CONFIG_EXTAL_CLOCK * 1000000 / NR / ND * NF;	\
	while (!(cpm_inl(CPM_CPMPCR) & (1 << 0)) && --TO)

#define SCLKA_SRC_STOP		0x00
#define SCLKA_SRC_APLL		0x01
#define SCLKA_SRC_EXT		0X02
#define SCLKA_SRC_RTC		0X03

#define SET_SCLKA(C,CLK_ID)									\
	do {													\
		unsigned int cpccr;									\
		cpccr = cpm_inl(CPM_CPCCR) & ~(0x3<<30);			\
		cpccr |= C<<30;										\
		cpm_outl(cpccr,CPM_CPCCR);							\
		clk_srcs[CLK_ID_SCLKA].parent = &clk_srcs[CLK_ID];	\
		clk_srcs[CLK_ID_SCLKA].rate = clk_srcs[CLK_ID].rate;\
	} while (0)


#define PLL_OUT_MAX (1000 * 1000000)

#ifdef CPU_FREQ_CONVER
static int cclk_set_rate(struct clk *clk, unsigned long rate)
{
	int i;
	unsigned int cpccr = 0;
	struct clk *clk_t;

	if (clk->rate == rate)
		return 0;

	clk_t = &clk_srcs[CLK_ID_MPLL];
	for (i = 1; i <= MAX_M_DIV; i++) {
		if (rate == clk_t->rate / i) {
			cpccr = cpm_inl(CPM_CPCCR) & ~(0x3<<28 | 0xff);
			cpccr |= CCLK_MPLL_DIV(i);
			SET_PARENT(CCLK, MPLL);
			SET_PARENT(L2CLK, MPLL);
			SET_PARENT(H0CLK, MPLL);
			cpm_outl(cpccr,CPM_CPCCR);
			clk->parent = clk_t;
			clk_srcs[CLK_ID_L2CLK].parent = clk_t;
			SET_SCLKA(SCLKA_SRC_EXT,CLK_ID_EXT1);
			goto out;
		}
	}

	if (rate <= MAX_CCLK) {
		cpccr = cpm_inl(CPM_CPCCR);
		if (rate != clk_srcs[CLK_ID_APLL].rate) {
			int nf,nr,no,bs;
			int timeout = 0xffffff;

			if (rate > PLL_OUT_MAX) {
				printk("PLL output can NOT more than 1000MHZ,we used 1000MHZ\n");
				rate = PLL_OUT_MAX;
			}
			if (rate > (600*1000000)) {
				bs = 1;
				no = 1;			//no = 1,2,4,8
			} else if (rate > (300*1000000)) {
				bs = 0;
				no = 1;
			} else if (rate > (155*1000000)) {
				bs = 0;
				no = 2;
			} else if (rate > (76*1000000)) {
				bs = 0;
				no = 4;
			} else if (rate > (47*1000000)) {
				bs = 0;
				no = 8;
			} else {
				printk("PLL ouptput can NOT less than 48MHZ,we used 48MHZ\n");
				bs = 0;
				no = 8;
				rate = (48*1000000);
			}

			/*
			 * Fout = Fin * nf /nr/no
			 * nf = Fout * nr * no/ Fin
			 */
			nr = 1;
			nf= (rate / 1000000) * nr * no / CONFIG_EXTAL_CLOCK;

			if (clk->parent == &clk_srcs[CLK_ID_SCLKA]){
				cpccr &= ~(0x3<<28 | 0xff);
				cpccr |= CCLK_MPLL_DIV(1);
				cpm_outl(cpccr,CPM_CPCCR);
			}
			SET_SCLKA(SCLKA_SRC_EXT,CLK_ID_EXT1);
			SET_APLL(bs, nf, nr, no, timeout);
			if (!timeout) {
				pr_err("set APLL %lu timeout\n", rate);
				return -1;
			}
		}
		if (clk_srcs[CLK_ID_SCLKA].parent != &clk_srcs[CLK_ID_APLL])
			SET_SCLKA(SCLKA_SRC_APLL,CLK_ID_APLL);
		SET_PARENT(CCLK, SCLKA);
		SET_PARENT(L2CLK,SCLKA);
		SET_PARENT(H0CLK,SCLKA);
		cpccr = cpm_inl(CPM_CPCCR) & ~(0x3<<28 | 0xff);
		cpm_outl(cpccr | CCLK_APLL_DIV(1),CPM_CPCCR);
		goto out;
	}

	pr_err("set cpu clk %lu failed\n", rate);
	return -1;
out:
	clk_srcs[CLK_ID_L2CLK].rate = cpccr_get_rate(&clk_srcs[CLK_ID_L2CLK]);
	clk->rate = cpccr_get_rate(clk);
	return 0;
}
#endif

static struct clk_ops clk_cpccr_ops = {
	.get_rate = cpccr_get_rate,
};


static struct clk_ops clk_cclk_ops = {
	.get_rate = cpccr_get_rate,
#ifdef CPU_FREQ_CONVER
	.set_rate = cclk_set_rate,
#endif
};

static void __init init_cpccr_clk(void)
{
	int i,sel,select[4] = {0,CLK_ID_SCLKA,CLK_ID_MPLL,0};	//check
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
}

struct cgu_clk {
	/* off: reg offset. ce: CE offset. coe : coe for div .div: div bit width */
	/* ext: extal/pll sel bit. sels: {select} */
	int off,ce,stop,coe,div,ext,busy,sel[8],cache;
};

static struct cgu_clk cgu_clks[] = {
	[CGU_DDR] = 	{ CPM_DDRCDR, 29, 27, 1, 4, 30, 28, {-1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_VPU] = 	{ CPM_VPUCDR, 29, 27, 1, 4, 31, 28, {CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_AIC] = 	{ CPM_I2SCDR, 29, 27, 1, 8, 30, 28, {CLK_ID_EXT1,CLK_ID_EXT1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_LCD] = 	{ CPM_LPCDR,  28, 26, 1, 8, 31, 27,	{CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_MSC_MUX]=	{ CPM_MSC0CDR, 29, 27, 2, 0, 30, 28, {-1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_MSC0] = 	{ CPM_MSC0CDR, 29, 27, 2, 8, 30, 28, {CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX}},
	[CGU_MSC1] = 	{ CPM_MSC1CDR, 29, 27, 2, 8, 30, 28, {CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX}},
	[CGU_MSC2] = 	{ CPM_MSC2CDR, 29, 27, 2, 8, 30, 28, {CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX,CLK_ID_MSC_MUX}},
	[CGU_USB] = 	{ CPM_USBCDR, 29, 27, 1, 8, 30, 28, {CLK_ID_EXT1,CLK_ID_EXT1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_UHC] = 	{ CPM_UHCCDR, 29, 27, 1, 8, 30, 28, {CLK_ID_SCLKA,CLK_ID_MPLL,0}},
	[CGU_SSI] = 	{ CPM_SSICDR, 29, 27, 1, 8, 30, 28, {CLK_ID_EXT1,CLK_ID_EXT1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_CIM_MUX] = { CPM_CIM0CDR, 30, 28, 1, 0, 31, 29, {CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_CIMMCLK0] = { CPM_CIM0CDR, 30, 28, 1, 8, 31, 29, {CLK_ID_CIM_MUX,CLK_ID_CIM_MUX}},
	[CGU_CIMMCLK1] = { CPM_CIM1CDR, 30, 28, 1, 8, 31, 29, {CLK_ID_CIM_MUX,CLK_ID_CIM_MUX}},
	[CGU_PCM] = 	{ CPM_PCMCDR, 28, 26, 1, 8, 30, 27, {CLK_ID_EXT1,CLK_ID_EXT1,CLK_ID_SCLKA,CLK_ID_MPLL}},
	[CGU_BCH] = 	{ CPM_BCHCDR, 29, 27, 1, 4, 30, 28, {-1,CLK_ID_SCLKA,CLK_ID_MPLL}},
};

static int cgu_enable(struct clk *clk,int on)
{
	int no = CLK_CGU_NO(clk->flags);

	if(on) {
		if(cgu_clks[no].cache) {
			cpm_outl(cgu_clks[no].cache,cgu_clks[no].off);

			while(cpm_test_bit(cgu_clks[no].busy,cgu_clks[no].off)) {
				printk("wait stable.[%d][%s]\n",__LINE__,clk->name);
			}
		} else {
			printk("######################################\n");
			printk("cgu clk should set rate before enable!\n");
			printk("######################################\n");
			BUG();
		}
	} else {
		cpm_set_bit(cgu_clks[no].ce,cgu_clks[no].off);
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

	if(clk->parent == &clk_srcs[CLK_ID_EXT1]) {
		if ((no == 2) || (no == 14))
			return clk->parent->rate/2;
		else
			return clk->parent->rate;
	}

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

	if (pidx == -1 && pidx == 0)
		return NULL;

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
	clk_srcs[CLK_ID_CIM_MUX].ops = NULL;
}

static void __init init_gate_clk(void)
{
	int i;
	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if(! (clk_srcs[i].flags & CLK_FLG_GATE))
			continue;
		if(!clk_srcs[i].rate && clk_srcs[i].parent)
			clk_srcs[i].rate = clk_srcs[i].parent->rate;
	}
}

static unsigned long clkgr;

int clk_suspend(void)
{
	clkgr = cpm_inl(CPM_CLKGR);

	cpm_outl(clkgr | 0x7fe8ffe0,CPM_CLKGR);
	udelay(20);
	return 0;
}

void clk_resume(void)
{
	cpm_outl(clkgr,CPM_CLKGR);
	mdelay(5);
}

struct syscore_ops clk_pm_ops = {
	.suspend = clk_suspend,
	.resume = clk_resume,
};

void __init init_all_clk(void)
{
	int i , tmp;

	tmp = cpm_inl(CPM_UHCCDR);
	tmp = (tmp & ~(0x3<<30)) | (0x0<<30);
	cpm_outl(tmp,CPM_UHCCDR);

	init_ext_pll();
	init_cpccr_clk();
	init_cgu_clk();
	init_gate_clk();

	for(i=0; i<ARRAY_SIZE(clk_srcs); i++) {
		if ((clk_srcs[i].flags & CLK_FLG_ENABLE)
				&& !(clk_srcs[i].flags & CLK_FLG_GATE))
			clk_srcs[i].count = 1;

		if (clk_srcs[i].rate)
			continue;
		if (clk_srcs[i].flags & CLK_FLG_PARENT) {
			int id = CLK_PARENT(clk_srcs[i].flags);
			clk_srcs[i].parent = &clk_srcs[id];
		}
		if (!clk_srcs[i].parent) {
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


static int cpm_clear_bit_lock(int bit)
{
	spin_lock(&clkgr_lock);
	cpm_clear_bit(bit, CPM_CLKGR);
	spin_unlock(&clkgr_lock);
	return 0;
}

static int cpm_set_bit_lock(int bit)
{
	spin_lock(&clkgr_lock);
	cpm_set_bit(bit, CPM_CLKGR);
	spin_unlock(&clkgr_lock);
	return 0;
}


static int clk_gate_ctrl(struct clk *clk, int enable)
{
	int bit = CLK_GATE_BIT(clk->flags);

	/* change clkgr atomic */
	if(enable)
		cpm_clear_bit_lock(bit%32);
	else
		cpm_set_bit_lock(bit%32);

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

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long tmp = 0;
	int i = 1 ,no , x;

	if (clk->source) {
		clk = clk->source;
		if (clk->flags | CLK_FLG_CGU) {
			if(clk->parent == &clk_srcs[CLK_ID_EXT1])
				return clk->parent->rate;
			no = CLK_CGU_NO(clk->flags);
			x = (1 << cgu_clks[no].div) - 1;
			tmp = clk->parent->rate / cgu_clks[no].coe;
			if (!strcmp(clk->name,CLK_NAME_CGU_AIC)) {
				for (i = 1; i <= x+1; i++) {
					if ((tmp / i) <= (rate / 1000 * 1005))
						break;
				}
			} else {
				for (i = 1; i <= x+1; i++) {
					if ((tmp / i) <= rate)
						break;
				}
			}
		}
	}
	return tmp / i;
}

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

int cpm_start_ohci(void)
{
	int tmp;
	static int has_reset = 0;

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

	/* Configurate UHC IBSOP */
	tmp = cpm_inl(CPM_USBPCR1);
	tmp &= ~(7 << 14);
	tmp |= (1 << 14);
	cpm_outl(tmp,CPM_USBPCR1);

	/* Configurate UHC XP */
	tmp = cpm_inl(CPM_USBPCR1);
	tmp &= ~(3 << 12);
	cpm_outl(tmp,CPM_USBPCR1);

	/* Configurate UHC SP */
	tmp = cpm_inl(CPM_USBPCR1);
	tmp &= ~(7 << 9);
	tmp |= (1 << 9);
	cpm_outl(tmp,CPM_USBPCR1);

	/* Configurate UHC SM */
	tmp = cpm_inl(CPM_USBPCR1);
	tmp &= ~(7 << 6);
	tmp |= (1 << 6);
	cpm_outl(tmp,CPM_USBPCR1);

	/* Disable overcurrent */
	cpm_clear_bit(4,CPM_USBPCR1);

	/* Enable OHCI clock */
	cpm_set_bit(5,CPM_USBPCR1);

	cpm_set_bit(17,CPM_USBPCR1);

	cpm_set_bit(6, CPM_OPCR);

	/* OTG PHY reset */
	cpm_set_bit(22, CPM_USBPCR);
	udelay(30);
	cpm_clear_bit(22, CPM_USBPCR);
	udelay(300);

	/* UHC soft reset */
	if(!has_reset) {
		cpm_set_bit(14, CPM_SRBC);
		udelay(300);
		cpm_clear_bit(14, CPM_SRBC);
		udelay(300);
		has_reset = 1;
	}

	printk(KERN_DEBUG __FILE__
	": Clock to USB host has been enabled \n");

	return 0;
}
EXPORT_SYMBOL(cpm_start_ohci);

int cpm_stop_ohci(void)
{
	/* disable ohci phy power */
	cpm_clear_bit(17,CPM_USBPCR1);

	cpm_clear_bit(6, CPM_OPCR);

	printk(KERN_DEBUG __FILE__
	       ": stop JZ OHCI USB Controller\n");
	return 0;
}
EXPORT_SYMBOL(cpm_stop_ohci);

int cpm_start_ehci(void)
{
	return 0;
}
EXPORT_SYMBOL(cpm_start_ehci);

int cpm_stop_ehci(void)
{
	return 0;
}
EXPORT_SYMBOL(cpm_stop_ehci);

static int clocks_show(struct seq_file *m, void *v)
{
	int i,len=0;
	struct clk *clk_srcs = get_clk_from_id(0);
	if(m->private != NULL) {
		len += seq_printf(m ,"CLKGR\t: %08x\n",cpm_inl(CPM_CLKGR));
		//len += seq_printf(m ,"CLKGR1\t: %08x\n",cpm_inl(CPM_CLKGR1));
		//len += seq_printf(m ,"LCR1\t: %08x\n",cpm_inl(CPM_LCR));
		//len += seq_printf(m ,"PGR\t: %08x\n",cpm_inl(CPM_PGR));
		//len += seq_printf(m ,"SPCR0\t: %08x\n",cpm_inl(CPM_SPCR0));
	} else {
		len += seq_printf(m,"ID NAME       FRE        stat       count     parent\n");
		for(i = 0; i < get_clk_sources_size(); i++) {
			if (clk_srcs[i].name == NULL) {
				len += seq_printf(m ,"--------------------------------------------------------\n");
			} else {
				unsigned int mhz = clk_srcs[i].rate / 10000;
				len += seq_printf(m,"%2d %-10s %4d.%02dMHz %3sable   %d %s\n",i,clk_srcs[i].name
						, mhz/100, mhz%100
						, clk_srcs[i].flags & CLK_FLG_ENABLE? "en": "dis"
						, clk_srcs[i].count
						, clk_srcs[i].parent? clk_srcs[i].parent->name: "root");
			}
		}
	}
	return len;
}

static int clk_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *data)
{
	int ret;
	char buf[32];
	unsigned int cgr;

	if (count > 32)
		count = 32;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	ret = sscanf(buf,"0x%x",&cgr);

	if(ret == 1) cpm_outl(cgr,CPM_CLKGR);

	return count;
}

static int clocks_open(struct inode *inode, struct file *file)
{
	return single_open_size(file, clocks_show, PDE_DATA(inode),8192);
}

static const struct file_operations clocks_proc_fops ={
	.read = seq_read,
	.open = clocks_open,
	.write = clk_write,
	.llseek = seq_lseek,
	.release = single_release,
};


static int __init init_clk_proc(void)
{
#if 0
	struct proc_dir_entry *p;

	p = jz_proc_mkdir("clock");
	if (!p) {
		pr_warning("create_proc_entry for common clock failed.\n");
		return -ENODEV;
	}
#endif
	proc_create_data("clocks", 0600,NULL,&clocks_proc_fops,0);
	return 0;
}

module_init(init_clk_proc);
