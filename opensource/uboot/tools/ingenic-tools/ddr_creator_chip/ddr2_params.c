#include "ddr_params_creator.h"
static void fill_in_params_ddr2(struct ddr_params *p, struct ddr_chip_info *chip)
{
	int tmp;

	/* MRn registers */
	p->mr0.ddr2.BA = 0;
	p->mr1.ddr2.BA = 1;
	p->mr2.ddr2.BA = 2;
	p->mr3.ddr2.BA = 3;

	/* BL: 1 is on the fly,???? */
	if(p->bl == 4)
		p->mr0.ddr2.BL = 2;
	else if(p->bl == 8)
		p->mr0.ddr2.BL = 3;
	else{
		out_error("DDR_BL(%d) error,only support 4 or 8\n",p->bl);
		assert(1);
	}

	BETWEEN(p->cl,3,9);
	if(p->cl == 9)
		p->cl = 0;
	if(p->cl == 8)
		p->cl = 1;
	p->mr0.ddr2.CL = p->cl;

	p->mr0.ddr2.DR = 1; //dll reset

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tWR, 1);
	switch(tmp)
	{
	case 2 ... 8:
		p->mr0.ddr2.WR = tmp - 1;
		break;
	/* DDR2 max is 9, if greater than 9, set 9 */
	case 9 ... 15:
		p->mr0.ddr2.WR = 0;
		break;
	default:
		out_error("tWR(%d) is error, valid value is between from 5 to 12.\n",
		       p->private_params.ddr2_params.tWR);
		assert(1);
	}
#define CONFIG_DDR_PD_OFF
#ifdef CONFIG_DDR_PD_OFF
	p->mr0.ddr2.PD = 0;
#else
	p->mr0.ddr2.PD = 1;
#endif

	/* MR1 register. */
//#define CONFIG_DDR_DLL_OFF
#ifdef CONFIG_DDR_DLL_OFF
	p->mr1.ddr2.DE = 1; /* DLL disable. */
#else
	p->mr1.ddr2.DE = 0; /* DLL enable. */
#endif

//#define CONFIG_DDR_DRIVER_OUT_STRENGTH
#ifdef CONFIG_DDR2_DRIVER_OUT_STRENGTH
	/*   0 - normal,1 - weak 60% */
	p->mr1.ddr2.DIC1 = 1;
#else
	p->mr1.ddr2.DIC1 = 0; /* normal */
#endif

#define CONFIG_DDR_CHIP_ODT_VAL
//#define CONFIG_ODT_DIS
#define CONFIG_ODT_75
//#define CONFIG_ODT_150
//#define CONFIG_ODT_50
#ifdef CONFIG_DDR_CHIP_ODT_VAL
	/**********************
	 * 00 - ODT disable. *
	 * 01 - 75ohm.       *
	 * 10 - 150ohm.       *
	 * 11 - 50ohm.       *
	 **********************/
#ifdef CONFIG_ODT_DIS
	p->mr1.ddr2.RTT2 = 0; /* Effective resistance of ODT RZQ/4 */
	p->mr1.ddr2.RTT6 = 0; /* Effective resistance of ODT RZQ/4 */
#endif
#ifdef CONFIG_ODT_75
	p->mr1.ddr2.RTT2 = 0; /* Effective resistance of ODT RZQ/4 */
	p->mr1.ddr2.RTT6 = 1; /* Effective resistance of ODT RZQ/4 */
#endif
#ifdef CONFIG_ODT_150
	p->mr1.ddr2.RTT2 = 1; /* Effective resistance of ODT RZQ/4 */
	p->mr1.ddr2.RTT6 = 0; /* Effective resistance of ODT RZQ/4 */
#endif
#ifdef CONFIG_ODT_50
	p->mr1.ddr2.RTT2 = 1; /* Effective resistance of ODT RZQ/4 */
	p->mr1.ddr2.RTT6 = 1; /* Effective resistance of ODT RZQ/4 */
#endif
#endif

//#define CONFIG_DDR_DCC_ON
#ifdef CONFIG_DDR_DCC_ON
	p->mr2.ddr2.DCC = 1;
#endif
	tmp = -1;
	tmp = ps2cycle_ceil(p->private_params.ddr2_params.WL,1);
	if(tmp < 5 || tmp > 8)
	{
		out_error("ddr frequancy too fast. %d\n",tmp);
		out_error(". %d\n",__ps_per_tck);
		assert(1);
	}
	//p->mr2.ddr2.CWL = tmp - 5;
}

static void ddrc_params_creator_ddr2(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tRTP,1);
	ASSERT_MASK(tmp, 6);
	ddrc->timing2.b.tRTP = tmp;

    tmp = ps2cycle_ceil(p->private_params.ddr2_params.tWTR, 1);
	ASSERT_MASK(tmp, 6);
	ddrc->timing1.b.tWTR = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tRTW, 1);
	ASSERT_MASK(tmp, 6);
	ddrc->timing2.b.tRTW = tmp;

    if (ddrc->timing1.b.tWL > 0) {
        tmp = ps2cycle_ceil(p->private_params.ddr2_params.tWDLAT, 1);
		ASSERT_MASK(tmp, 6);
		ddrc->timing1.b.tWDLAT = tmp;
	} else {
        out_error("DDR_WL too small!! please check %s, %s\n", __FILE__, __func__);
        assert(1);
    }

    if (ddrc->timing2.b.tRL > 0) {
        tmp = ps2cycle_ceil(p->private_params.ddr2_params.tRDLAT, 1);
		ASSERT_MASK(tmp, 6);
		ddrc->timing2.b.tRDLAT = tmp;
	} else {
        out_error("DDR_RL too small!! please check %s, %s\n", __FILE__, __func__);
        assert(1);
    }

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tCCD,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing3.b.tCCD = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tMRD,4) / 4;
	ASSERT_MASK(tmp,5);
	ddrc->dlmr |= tmp << DDRC_LMR_TMRD_BIT;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tCKSRE,1);
	ASSERT_MASK(tmp,4);
	ddrc->timing5.b.tCKSRE = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tCKESR,1);
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tCKESR = tmp;

	tmp = ps2cycle_ceil(p->private_params.ddr2_params.tXSNR,1);
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tXS = tmp;

    tmp = ps2cycle_ceil(p->private_params.ddr2_params.tFAW,1);
	ASSERT_MASK(tmp,8);
	ddrc->timing4.b.tFAW = tmp;
}
static void ddrp_params_creator_ddr2(struct ddrp_reg *ddrp, struct ddr_params *p)
{
    ddrp->cl = p->cl;
    ddrp->cwl = p->cl - 2;
}

static struct ddr_creator_ops ddr2_creator_ops = {
	.type = DDR2,
	.fill_in_params = fill_in_params_ddr2,
	.ddrc_params_creator = ddrc_params_creator_ddr2,
	.ddrp_params_creator = ddrp_params_creator_ddr2,

};
void ddr2_creator_init(void)
{
	register_ddr_creator(&ddr2_creator_ops);
}
