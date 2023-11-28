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
#if 0 /*no frac in t30 */
static u32 frac_to_value(u32 frac)
{
	u32 t = frac * 125;
	u32 v = t / 0x200000 + (((t % 0x200000) * 2 >= 0x200000) ? 1 : 0);

	return v;
}
#endif
struct pll_rate_setting {
	unsigned long rate;
	int m,n,od,rng;
};

struct pll_rate_setting* cal_pll_setting(unsigned long rate)
{
	struct pll_rate_setting *p;
	unsigned int fvco_mul,pllm,plln,pllod,pllrng;
	static struct pll_rate_setting tmp_rate;
	unsigned char sig = 0;
	unsigned int ext_rate = get_clk_from_id(CLK_ID_EXT1)->rate;

	if( (rate >= 25000000UL) && (rate <= 3000000000UL) ){
		p = &tmp_rate;

		for(plln=0;plln<4;plln++)
		{
			for(pllm=0;pllm<0x1ff;pllm++)
			{
				fvco_mul = ext_rate*2*(pllm+1);
				for(pllod=0;pllod<0x07;pllod++)
				{
					if (
							( fvco_mul == ((plln+1UL)*(1UL<<pllod)*rate) ) &&
							( fvco_mul >= 1500000000UL*(plln+1UL) )	&&
							( fvco_mul <= 3000000000UL*(plln+1UL) )
					   )
						{
							sig = 1;
							break;
						}
				}
				if( sig )
				{
					break;
				}
			}
			if( sig )
			{
				break;
			}
		}


		if ( sig == 0 )
		{
			printk("CALCULATING PLL CLK FAILED\r\n");
			return NULL;
		}

		printk("pllm=%d plln=%d,pllod=%d\n",pllm,plln,pllod);

		pllrng = 3-pllod;
		if(pllrng < 1)
			pllrng = 1;
		p->rate = (unsigned long)((fvco_mul/((plln+1UL)*(1UL<<pllod)))/1000000UL);
		p->m = pllm;
		p->n = plln;
		p->od = pllod;
		p->rng = pllrng;
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

			cpxpcr &= ~((0xfff << 20) | (0x3f << 14) | (0x7 << 11) | (0x7 << 5));
			cpxpcr |= ((p->m) << 20) | ((p->n) << 14) |
				((p->od) << 11) | ((p->rng) << 5) ;
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
	unsigned long pllm,plln,pllod;
	unsigned long rate;
	unsigned long flags;

	spin_lock_irqsave(&cpm_pll_lock,flags);

	cpxpcr = cpm_inl(CLK_PLL_NO(clk->flags));
	if(cpxpcr & 1){
		clk->flags |= CLK_FLG_ENABLE;
		pllm = ((cpxpcr >> 20) & 0x1ff);
		plln = ((cpxpcr >> 14) & 0x3f);
		pllod = ((cpxpcr >> 11) & 0x07);
#if 0 /*no frac regs in t30 */
		if (get_clk_id(clk) == CLK_ID_VPLL)
			frac = cpm_inl(CLK_PLL_NO(clk->flags) + 4);
		else
			frac = cpm_inl(CLK_PLL_NO(clk->flags) + 8);
		frac_value = frac_to_value(frac);
		rate = ((clk->parent->rate / 4000) * m / n / o1 / o0) * 4000
			+ ((clk->parent->rate / 4000) * frac_value / n / o1 / o0) * 4;
#endif
        rate = (unsigned long)((clk->parent->rate/1000)*2*(pllm+1)/(plln+1)/(1<<pllod));
	}else  {
		clk->flags &= ~(CLK_FLG_ENABLE);
		rate = 0;
	}
	spin_unlock_irqrestore(&cpm_pll_lock,flags);
	return rate*1000;
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
