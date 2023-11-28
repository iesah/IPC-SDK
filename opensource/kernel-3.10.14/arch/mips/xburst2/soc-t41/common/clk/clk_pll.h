
#ifndef __INGENIC_CLK_PLLV_H__
#define __INGENIC_CLK_PLLV_H__

#define PLL_RATE(_rate, _m, _n, _od0, _od1,_rg)	\
{					\
	.rate = _rate,			\
	.m = _m,			\
	.n = _n,			\
	.od0 = _od0,			\
	.od1 = _od1,			\
	.rg = _rg,			\
}

struct ingenic_pll_rate_table {
	unsigned int rate;
	unsigned int m;
	unsigned int n;
	unsigned int od0;
	unsigned int od1;
	unsigned int rg;
};

#endif /*__INGENIC_CLK_PLLV1_H__*/
