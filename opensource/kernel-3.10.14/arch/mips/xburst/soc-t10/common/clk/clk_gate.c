#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <soc/cpm.h>
#include <soc/base.h>
#include <soc/extal.h>
#include <jz_notifier.h>


#include "clk.h"

static DEFINE_SPINLOCK(cpm_gate_lock);

static unsigned short sram_pwc[] = {
#define GATE(X)  X
#define SRAM_RELATIVE_CLK(X,Y) [X] = (SRAM_##Y << 0) | (CLK_ID_##Y << 8)
	/* T15 is confused */
#undef SRAM_RELATIVE_CLK
};
static int cpm_gate_enable(struct clk *clk,int on){
	int bit = CLK_GATE_BIT(clk->flags);
	unsigned int clkgr[2] = {CPM_CLKGR,CPM_CLKGR1};
	unsigned long flags;
//	if(on)
//		jz_notifier_call(NOTEFY_PROI_HIGH, JZ_CLKGATE_CHANGE,(void*)(clk->CLK_ID | 0x80000000));
	spin_lock_irqsave(&cpm_gate_lock,flags);
	if(on) {
		cpm_clear_bit(bit % 32, clkgr[bit / 32]);
		if(bit < ARRAY_SIZE(sram_pwc) && sram_pwc[bit]) {
			cpm_clear_bit(sram_pwc[bit] & 0xff,CPM_SPCR0);
		}
	} else {
		if(bit < ARRAY_SIZE(sram_pwc) && sram_pwc[bit])  {
			cpm_clear_bit(bit % 32, clkgr[bit / 32]);
			cpm_set_bit(sram_pwc[bit] & 0xff,CPM_SPCR0);
		}
		cpm_set_bit(bit % 32, clkgr[bit / 32]);
	}
	spin_unlock_irqrestore(&cpm_gate_lock,flags);
//	if(!on)
//		jz_notifier_call(NOTEFY_PROI_HIGH, JZ_CLKGATE_CHANGE,(void*)clk->CLK_ID);
	return 0;
}
static struct clk_ops clk_gate_ops = {
	.enable	= cpm_gate_enable,
};

void __init init_gate_clk(struct clk *clk)
{
	int id = 0;
	static  unsigned long clkgr[2]={0};
	static  int clkgr_init = 0;
	int bit = CLK_GATE_BIT(clk->flags);
	if(clkgr_init == 0){
		clkgr[0] = cpm_inl(CPM_CLKGR);
		clkgr[1] = cpm_inl(CPM_CLKGR1);
		clkgr_init = 1;
	}
	if (clk->flags & CLK_FLG_PARENT) {
		id = CLK_PARENT(clk->flags);
		clk->parent = get_clk_from_id(id);
	}else
		clk->parent = get_clk_from_id(CLK_ID_EXT1);
	clk->rate = clk_get_rate(clk->parent);
	if(clkgr[bit / 32] & (1 << (bit % 32))) {
		clk->flags &= ~(CLK_FLG_ENABLE);
		cpm_gate_enable(clk,0);
	}else {
		clk->flags |= CLK_FLG_ENABLE;
		cpm_gate_enable(clk,1);
	}
	clk->ops = &clk_gate_ops;
}
