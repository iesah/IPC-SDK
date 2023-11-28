#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/bsearch.h>

#include <soc/cpm.h>
#include <soc/base.h>
#include <soc/extal.h>

#include "clk.h"
#include "clk_pll.h"

static DEFINE_SPINLOCK(cpm_pll_lock);

/*******************************************************************************
 *		PLL
 ********************************************************************************/

static u32 frac_to_value(u32 frac)
{
	u32 t = frac * 125;
	u32 v = t / 0x200000 + (((t % 0x200000) * 2 >= 0x200000) ? 1 : 0);

	return v;
}

struct pll_rate_setting {
	unsigned long rate;
	int m,n,od1,od0,rg;
};

struct pll_rate_setting* cal_pll_setting(struct clk *clk, unsigned long rate)
{
	struct pll_rate_setting *p;
	static struct pll_rate_setting tmp_rate;
	p = &tmp_rate;

	unsigned int m=0, n=0, od1=0, od0=1, pllrg = 0, pllrg_n = 0;
	unsigned int fref=24, pllout=0, fvco=0, fvco_re = 0, reg = 0;

	rate = rate /1000000;

	for (m=0; m<=512; m++) {
		for (n=0; n<=23; n++) {
			for (od0=1; od0<=7; od0++) {
				for (od1=0; od1<=15; od1++) {
					fvco = fref*(m+1)*2/(n+1);
					pllout = fvco/(1 << od0)/(od1+1);
					pllrg_n = 24 / (n + 1);
					fvco_re = 24 % (n + 1);
					if((pllrg_n >= 0) && (pllrg_n < 7))
						pllrg = 0;
					else if((pllrg_n >= 7) && (pllrg_n < 11))
						pllrg = 1;
					else if((pllrg_n >= 11) && (pllrg_n < 18))
						pllrg = 2;
					else
						pllrg = 3;

					if((rate == pllout) && (fvco<=6000) && (fvco>=3000) && (fvco_re == 0)) {
						printk("M= %d N=%d OD0=%d OD1=%d pllrg=%d\n", m, n, od0, od1, pllrg);
						p->rate = pllout;
						p->m =	  m;
						p->n =	  n;
						p->od0 =  od0;
						p->od1 =  od1;
						p->rg =   pllrg;
						return p;
					} else {
						continue;
					}
				}
			}
		}
	}
}

static int pll_set_rate(struct clk *clk,unsigned long rate)
{
	int ret = -1;
	unsigned int cpxpcr;
	unsigned int cpccr, cpccr_def;
	struct pll_rate_setting *p=NULL;
	unsigned long flags;
	unsigned int timeout = 0x1ffff;
	spin_lock_irqsave(&cpm_pll_lock,flags);

	cpxpcr = cpm_inl(CLK_PLL_NO(clk->flags));
	if(rate == 0) {
		cpxpcr &= ~(1 << 0);
		cpm_outl(cpxpcr,CLK_PLL_NO(clk->flags));
		clk->rate = 0;
		ret = 0;
		goto PLL_SET_RATE_FINISH;
	} else if(rate <= clk_get_rate(clk->parent)) {
		ret = -1;
	} else {
		p = cal_pll_setting(clk, rate);
		if(p) {
			cpccr = cpm_inl(0x00);
			cpccr_def = cpccr;
			cpccr = cpccr & ~(0xff << 24) | (0x55 << 24);
			cpm_outl(cpccr, 0x00);
			cpxpcr &= ~(0x1 << 0);
			cpm_outl(cpxpcr,CLK_PLL_NO(clk->flags));

			cpxpcr &= ~((0xfff << 20) | (0x3f << 14) | (0x7 << 11) | (0xf << 7) | (0x7 << 4));
			cpxpcr |= ((p->m) << 20) | ((p->n) << 14) |
				((p->od0) << 11) | ((p->od1) << 7) | ((p->rg) << 4) | (0x1 << 0) ;
			cpm_outl(cpxpcr,CLK_PLL_NO(clk->flags));

			ret = 0;
			clk->rate = p->rate * 1000 * 1000;

			while(!(cpm_inl(CLK_PLL_NO(clk->flags)) & (1 << 3)) && timeout--);
			if (timeout == 0) {
				printk("wait pll stable timeout!");
				ret = -1;
			}
		} else {
			printk("no support this rate [%ld]\n",rate);
		}
	}

PLL_SET_RATE_FINISH:
	spin_unlock_irqrestore(&cpm_pll_lock,flags);
	return ret;
}

static unsigned long pll_get_rate(struct clk *clk) {
	unsigned long cpxpcr;
	unsigned long m,n,od1,od0;
	unsigned long rate;
	unsigned long flags;

	spin_lock_irqsave(&cpm_pll_lock,flags);

	cpxpcr = cpm_inl(CLK_PLL_NO(clk->flags));
	if(cpxpcr & 1) {
		clk->flags |= CLK_FLG_ENABLE;
		m = ((cpxpcr >> 20) & 0xfff);
		n = ((cpxpcr >> 14) & 0x3f);
		od0 = ((cpxpcr >> 11) & 0x07);
		od1 = ((cpxpcr >> 7) & 0x07);
		od0=1<<od0;
		rate = ((24*(m+1)*2)/(n+1))/od0/(od1+1);
		rate = rate * 1000000;
	}else  {
		clk->flags &= ~(CLK_FLG_ENABLE);
		rate = 0;
	}
	spin_unlock_irqrestore(&cpm_pll_lock,flags);
	return rate;
}
static struct clk_ops clk_pll_ops = {
	.get_rate = pll_get_rate,
	.set_rate = pll_set_rate,
};
void __init init_ext_pll(struct clk *clk)
{
	switch (get_clk_id(clk)) {
	case CLK_ID_EXT0:
		clk->rate = JZ_EXTAL_RTC;
		clk->flags |= CLK_FLG_ENABLE;
		break;
	case CLK_ID_EXT1:
		clk->rate = JZ_EXTAL;
		clk->flags |= CLK_FLG_ENABLE;
		break;
	case CLK_ID_OTGPHY:
		clk->rate = 48 * 1000 * 1000;
		clk->flags |= CLK_FLG_ENABLE;
		break;
	default:
		clk->parent = get_clk_from_id(CLK_ID_EXT1);
		clk->rate = pll_get_rate(clk);
		clk->ops = &clk_pll_ops;
		break;
	}
}
