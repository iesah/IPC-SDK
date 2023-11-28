#include "ddr_params_creator.h"
static struct ddr_out_impedance out_impedance[]={
	{40,11},
	{34,13},
};
static struct ddr_out_impedance odt_out_impedance[]={
	{120,1},
	{60,5},
	{40,8},
};

static void fill_mr_params_ddr3(struct ddr_params *p)
{
	int tmp;

	/* MRn registers */
	p->mr0.ddr3.BA = 0;
	p->mr1.ddr3.BA = 1;
	p->mr2.ddr3.BA = 2;
	p->mr3.ddr3.BA = 3;

	/* BL: 1 is on the fly,???? */
	if(p->bl == 4)
		p->mr0.ddr3.BL = 2;
	else if(p->bl == 8)
		p->mr0.ddr3.BL = 0;
	else{
		out_error("DDR_BL(%d) error,only support 4 or 8\n",p->bl);
		assert(1);
	}
	p->mr0.ddr3.BL = (8 - p->bl) / 2;

	BETWEEN(p->cl,5,16);
	p->mr0.ddr3.CL_4_6 = p->cl - 4;

	p->mr0.ddr3.DR = 1; //dll reset

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tWR, 1);
	switch(tmp)
	{
	case 5 ... 8:
		p->mr0.ddr3.WR = tmp - 4;
		break;
	case 9 ... 16:
		p->mr0.ddr3.WR = (tmp + 1) / 2;
		break;
	default:
		out_error("tWR(%d) is error, valid value is between from 5 to 12.\n",
		       p->private_params.ddr3_params.tWR);
		assert(1);
	}

#ifdef CONFIG_DDR_DLL_OFF
	p->mr0.ddr3.PD = 0;
#else
	p->mr0.ddr3.PD = 1;
#endif

	/* MR1 register. */
#ifdef CONFIG_DDR_DLL_OFF
	p->mr1.ddr3.DE = 1; /* DLL disable. */
#else
	p->mr1.ddr3.DE = 0; /* DLL enable. */
#endif
	p->mr1.ddr3.LEVEL = 0;
	p->mr1.ddr3.QOFF = 0;
#ifdef CONFIG_DDR_DRIVER_OUT_STRENGTH
	/**********************
      	DIC5   DIC1
	 * 0     0 - RZQ/6.   *
	 * 0     1 - RZQ/7.   *
	 * 1     0 - RZQ/3.   *
	 * 1     1 - RZQ/4.   *
	 **********************/
	p->mr1.ddr3.DIC5 = CONFIG_DDR_DRIVER_OUT_STRENGTH_1;
	BETWEEN(p->mr1.ddr3.DIC5,0,1);

    	p->mr1.ddr3.DIC1 = CONFIG_DDR_DRIVER_OUT_STRENGTH_0;
	BETWEEN(p->mr1.ddr3.DIC1,0,1);
#else
	p->mr1.ddr3.DIC1 = 1; /* Impedance=RZQ/7 */
#endif

#ifdef CONFIG_DDR_CHIP_ODT_VAL
	/**********************
	 * 000 - ODT disable. *
	 * 001 - RZQ/4.       *
	 * 010 - RZQ/2.       *
	 * 011 - RZQ/6.       *
	 * 100 - RZQ/12.      *
	 * 101 - RZQ/8.       *
	 **********************/
	p->mr1.ddr3.RTT9 = CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_9; /* Effective resistance of ODT RZQ/4 */
	BETWEEN(p->mr1.ddr3.RTT9,0,1);

    	p->mr1.ddr3.RTT6 = CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_6; /* Effective resistance of ODT RZQ/4 */
	BETWEEN(p->mr1.ddr3.RTT6,0,1);

	p->mr1.ddr3.RTT2 = CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_2; /* Effective resistance of ODT RZQ/4 */
	BETWEEN(p->mr1.ddr3.RTT2,0,1);

    	/* RTT_WR */
    	/*****************************
    	A10    A9
    	* 0     0 - rtt_wr disable. *
    	* 0     1 - RZQ/4.          *
    	* 1     0 - RZQ/2.          *
    	* 1     1 - reserved.       *
    	*****************************/
    	p->mr2.ddr3.RTTWR = CONFIG_DDR_CHIP_ODT_VAL_RTT_WR; /* Effective resistance of ODT RZQ/4 */
    	BETWEEN(p->mr2.ddr3.RTTWR,0,3);
#else
		p->mr1.ddr3.RTT2 = 0;
		p->mr1.ddr3.RTT6 = 1;
#endif

	tmp = -1;
	tmp = ps2cycle_ceil(p->private_params.ddr3_params.WL,1);
	if(tmp < 5 || tmp > 10)
	{
		out_error("ddr frequancy too fast. %d\n",tmp);
		out_error(". %d\n",__ps_per_tck);
		assert(1);
	}
	p->mr2.ddr3.CWL = tmp - 5;
}

static void fill_in_params_ddr3(struct ddr_params *ddr_params)
{
	struct ddr3_params *params = &ddr_params->private_params.ddr3_params;
	if(params->RL == -1  || params->WL == -1) {
		out_error("lpddr cann't surpport auto mode!\n");
		assert(1);
	}
	params->tMRD = DDR_tMRD;
	params->tXSDLL = DDR_tXSDLL;
	params->tMOD = DDR_tMOD;
	params->tXPDLL = DDR_tXPDLL;
	params->tCKESR = DDR_tCKESR;
	params->tCKSRE = DDR_tCKSRE;
	params->tXS =  DDR_tXSR;
	params->tRTP = DDR_tRTP;
	params->tCCD = DDR_tCCD;
	params->tWTR = DDR_tWTR;
	params->tFAW = DDR_tFAW;
	params->tRTW = DDR_tRTW;

#ifdef CONFIG_DDR_INNOPHY
	fill_mr_params_ddr3(ddr_params);
#endif
}

static void ddrc_params_creator_ddr3(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tRTP,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tRTP = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCCD,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing3.b.tCCD = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCKSRE,1);
	ASSERT_MASK(tmp,4);
	ddrc->timing5.b.tCKSRE = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tCKESR,1);
	if(tmp <= 0)
		tmp = 2;
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tCKESR = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tXSDLL,4) / 4;
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tXS = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tMRD,4) / 4;
	ASSERT_MASK(tmp,5);
	ddrc->dlmr |= tmp << DDRC_LMR_TMRD_BIT;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.WL +
				p->private_params.ddr3_params.tCCD +
				p->private_params.ddr3_params.tWTR,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tWTR = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tRTW,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tRTW = tmp;

	ddrc->timing1.b.tWDLAT = ddrc->timing1.b.tWL - 1;
#ifdef CONFIG_DDR_INNOPHY
	ddrc->timing2.b.tRDLAT = ddrc->timing2.b.tRL - 3;
#else
	ddrc->timing2.b.tRDLAT = ddrc->timing2.b.tRL - 2;
#endif
	tmp = ps2cycle_ceil(p->private_params.ddr3_params.tFAW,1);
	ASSERT_MASK(tmp,8);
	ddrc->timing4.b.tFAW = tmp;
}

static void ddrp_params_creator_ddr3(struct ddrp_reg *ddrp, struct ddr_params *p){}

static struct ddr_creator_ops ddr3_creator_ops = {
	.type = DDR3,
	.fill_in_params = fill_in_params_ddr3,
	.ddrc_params_creator = ddrc_params_creator_ddr3,
	.ddrp_params_creator = ddrp_params_creator_ddr3,

};
#ifdef CONFIG_DDR_TYPE_DDR3
void ddr_creator_init(void)
{
	register_ddr_creator(&ddr3_creator_ops);
}
#endif
