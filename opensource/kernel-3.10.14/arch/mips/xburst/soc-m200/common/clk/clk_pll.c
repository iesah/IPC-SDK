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
static const struct pll_rate_setting {
	unsigned long rate;
	int m,n,o2,o1;
}pll_setting[] = {
	// rate(M)  m    n   o2   o1
	{ 30,    25,   1,   5,   4},
	{ 40,    25,   1,   5,   3},
	{ 48,    30,   1,   5,   3},
	{ 60,    25,   1,   5,   2},
	{ 72,    30,   1,   5,   2},
	{ 80,    50,   1,   5,   3},
	{ 96,    40,   1,   5,   2},
	{ 120,   25,   1,   5,   1},
	{ 200,   25,   1,   3,   1},
	{ 300,   25,   1,   2,   1},
	{ 400,   50,   1,   3,   1},
	{ 600,   25,   1,   1,   1},
};

static int pll_setting_cmp(const void *key,const void *elt) {
	unsigned long *d = (unsigned long*)key;
	struct pll_rate_setting *p = (struct pll_rate_setting *)elt;
	if(*d > p->rate * 1000 * 1000 + 12 * 1000 * 1000)
		return 1;
	else if(*d < p->rate * 1000 * 1000 - 12 * 1000 * 1000)
		return -1;
	return 0;
}
struct pll_rate_setting* search_pll_setting(unsigned long rate) {
	struct pll_rate_setting *p;

	if(rate / 1000 / 1000 <= pll_setting[ARRAY_SIZE(pll_setting) - 1].rate)
		p = (struct pll_rate_setting *)bsearch((const void*)&rate,(const void*)pll_setting,ARRAY_SIZE(pll_setting),
						       sizeof(struct pll_rate_setting),pll_setting_cmp);
	else {
		unsigned int ext_rate = get_clk_from_id(CLK_ID_EXT1)->rate;
		int div;
		static struct pll_rate_setting tmp_rate;
		p = &tmp_rate;
		div = rate / ext_rate;
		p->rate = div * (ext_rate / 1000 / 1000);
		p->m = div;
		p->n = 1;
		p->o2 = 1;
		p->o1 = 1;
	}
	return p;
}
static int pll_set_rate(struct clk *clk,unsigned long rate) {
	int ret = -1;
	unsigned int cpxpcr;
	struct pll_rate_setting* p = NULL;
	unsigned long flags;
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
		p = search_pll_setting(rate);
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
			while(!(cpm_inl(CLK_PLL_NO(clk->flags)) & (1 << 3)));
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
	spin_lock_irqsave(&cpm_pll_lock,flags);

	cpxpcr = cpm_inl(CLK_PLL_NO(clk->flags));
	if(cpxpcr & 1){
		clk->flags |= CLK_FLG_ENABLE;
		m = ((cpxpcr >> 20) & 0xfff);
		n = ((cpxpcr >> 14) & 0x3f);
		o1 = ((cpxpcr >> 11) & 0x07);
		o0 = ((cpxpcr >> 8) & 0x07);
		rate = ((clk->parent->rate / 4000) * m / n / o1 / o0) * 4000;
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
