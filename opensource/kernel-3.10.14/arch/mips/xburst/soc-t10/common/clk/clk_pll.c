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

static DEFINE_SPINLOCK(cpm_pll_lock);

static u32 frac_to_value(u32 frac)
{
	u32 t = frac * 125;
	u32 v = t / 0x200000 + (((t % 0x200000) * 2 >= 0x200000) ? 1 : 0);

	return v;
}

struct pll_rate_setting {
	unsigned long rate;
	int m,n,o2,o1;
};

struct pll_rate_setting* cal_pll_setting(unsigned long rate)
{
	struct pll_rate_setting *p;

	if(rate >= 600000000 && rate <= 1200000000){ /*Now we just supoort 600M~1.2G*/
		unsigned int ext_rate = get_clk_from_id(CLK_ID_EXT1)->rate,div_rate;
		static struct pll_rate_setting tmp_rate;
		int m = -1,n = 1;
		p = &tmp_rate;

		for(n = 1; n < 0x40; n++) {/*PLLN[5:0]*/
			if (ext_rate % n !=0)
				continue;

			div_rate = ext_rate / n;
			if ((rate % div_rate == 0) && (rate / div_rate < 0x1000)){/*PLLM[11:0]*/
				m = rate / div_rate;
				break;
			} else {
				continue;
			}
		}

		if (n == 0x40 || m == -1)
			return NULL;

		printk("m=%d n=%d\n",m,n);
		p->rate = m * (ext_rate / n / 1000 / 1000);
		p->m = m;
		p->n = n;
		p->o2 = 1;
		p->o1 = 1;
	} else {
		return NULL;
	}

	return p;
}

static int pll_set_rate(struct clk *clk,unsigned long rate)
{
	int ret = -1;
	unsigned int cpxpcr;
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
		p = cal_pll_setting(rate);
		if(p) {
			cpxpcr &= ~1;
			cpm_outl(cpxpcr,CLK_PLL_NO(clk->flags));

			cpxpcr &= ~((0xfff << 20) | (0x3f << 14) | (0x7 << 11) | (0x7 << 8));
			cpxpcr |= ((p->m) << 20) | ((p->n) << 14) |
				((p->o2) << 11) | ((p->o1) << 8) ;
			cpm_outl(cpxpcr,CLK_PLL_NO(clk->flags));

			cpxpcr |= 1;
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
	unsigned long m,n,o1,o0;
	unsigned long rate;
	unsigned long flags;
	unsigned long frac, frac_value;

	spin_lock_irqsave(&cpm_pll_lock,flags);

	cpxpcr = cpm_inl(CLK_PLL_NO(clk->flags));
	if(cpxpcr & 1){
		clk->flags |= CLK_FLG_ENABLE;
		m = ((cpxpcr >> 20) & 0xfff);
		n = ((cpxpcr >> 14) & 0x3f);
		o1 = ((cpxpcr >> 11) & 0x07);
		o0 = ((cpxpcr >> 8) & 0x07);
		frac = cpm_inl(CLK_PLL_NO(clk->flags) + 8);
		frac_value = frac_to_value(frac);
		rate = ((clk->parent->rate / 4000) * m / n / o1 / o0) * 4000
			+ ((clk->parent->rate / 4000) * frac_value / n / o1 / o0) * 4;

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
