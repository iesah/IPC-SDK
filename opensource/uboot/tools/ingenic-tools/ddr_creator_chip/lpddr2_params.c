#include "ddr_params_creator.h"
struct ddr_latency_table
{
	unsigned int freq;
	int latency;
};

static struct ddr_latency_table rl_LPDDR2[] = {
	{333000000,3},/*Date Rate xxM, RL*/
	{400000000,3},
	{533000000,4},
	{667000000,5},
	{800000000,6},
	{933000000,7},
	{1066000000,8},
};

static struct ddr_latency_table wl_LPDDR2[]= {
	{333000000,1},/*Date Rate xxM, WL*/
	{400000000,1},
	{533000000,2},
	{667000000,2},
	{800000000,3},
	{933000000,4},
	{1066000000,4},
};
static struct ddr_out_impedance out_impedance[]={
	{80000,5},
	{60000,7},
	{48000,9},
	{40000,11},
	{34000,13},
};

static int find_ddr_lattency(struct ddr_latency_table *table,int size,unsigned int freq)
{
	int i, max;
	unsigned int data_rate = freq * 2;
	i = size / sizeof(struct ddr_latency_table) - 1;
	max = i;
	for(;i>=0;i--) {
		if(data_rate >= table[i].freq) {
			if ((data_rate % table[i].freq) && (data_rate <= table[max].freq))
				return table[i + 1].latency;
			else
				return table[i].latency;
		}
	}
	return table[0].latency;
}

static void fill_mr_params_lpddr2(struct ddr_params *p)
{
	int tmp;
	int rl = 0,wl = 0;
	int  count = 0;
	struct lpddr2_params *params = &p->private_params.lpddr2_params;

	/**
	 * MR1 registers
	*/
	p->mr1.d32 = 0;
	p->mr1.lpddr2.MA = 0x1;

	tmp = ps2cycle_ceil(params->tWR, 1);
	ASSERT_MASK(tmp,3);
	BETWEEN(tmp,3,8);
	p->mr1.lpddr2.nWR = tmp - 2;

	p->mr1.lpddr2.WC = 0x0; // wrap control, 0b: Wrap, 1b: No wrap.
	p->mr1.lpddr2.BT = 0x0; // burst type, 0b: Sequential, 1b: Interleaved.

	if(p->bl != 8) {
		out_error("BL(%d) only support 8\n", p->bl);
		assert(1);
	}
	tmp = p->bl;
	while (tmp >>= 1) count++;
	p->mr1.lpddr2.BL = count;

	/**
	 * MR2 registers
	 */
	p->mr2.d32 = 0;
	p->mr2.lpddr2.MA = 0x2;

	tmp = ps2cycle_ceil(params->RL,1);
	if(tmp < 3 ||
	   tmp > 8)
	{
		out_error("the PHY don't support the RL(%d) \n",params->RL);
		assert(1);
	}
	rl = tmp;

	tmp = ps2cycle_ceil(params->WL,1);
	if(tmp < 1 ||
	   tmp > 4)
	{
		out_error("the PHY don't support the WL(%d) \n",params->WL);
		assert(1);
	}
	wl = tmp;

	tmp = wl | (rl << 4);

	switch(tmp)
	{
	case 0x31:
		tmp = 1;
		break;
	case 0x42:
		tmp = 2;
		break;
	case 0x52:
		tmp = 3;
		break;
	case 0x63:
		tmp = 4;
		break;
	case 0x74:
		tmp = 5;
		break;
	case 0x84:
		tmp = 6;
		break;
	default:
		out_error("the PHY don't support the WL(%d) or RL(%d)\n",
				  params->WL,params->RL);
		assert(1);
	}
	p->mr2.lpddr2.RL_WL = tmp;
	/**
	 * MR3 registers
	 */
	p->mr3.d32 = 0;
	p->mr3.lpddr2.MA = 0x3;

	/**
	  * 0000b: Reserved
	  * 0001b: 34.3 ohm typical
	  * 0010b: 40 ohm typical (default)
	  * 0011b: 48 ohm typical
	  * 0100b: 60 ohm typical
	  * 0101b: Reserved
	  * 0110b: 80 ohm typical
	  * 0111b: 120 ohm typical
	  * All others: Reserved
	 */
#ifdef CONFIG_DDR_DRIVER_STRENGTH
	p->mr3.lpddr2.DS = CONFIG_DDR_DRIVER_STRENGTH;
#else
	p->mr3.lpddr2.DS = 2;
	out_warn("Warnning: Please set ddr driver strength.");
#endif

	/**
	 * MR10 Calibration registers
	 */
	p->mr10.d32 = 0;
	p->mr10.lpddr2.MA = 0x0a;
	/**
	   0xFF: Calibration command after initialization
	   0xAB: Long calibration
	   0x56: Short calibration
	   0xC3: ZQRESET
	*/
	p->mr10.lpddr2.CAL_CODE = 0xFF;

	/**
	 * MR63 reset registers, RESET (MA[7:0] = 3Fh) – MRW Only
	 */
	p->mr63.d32 = 0;
	p->mr63.lpddr2.MA = 0x3f;
}

static void fill_in_params_lpddr2(struct ddr_params *ddr_params, struct ddr_chip_info *chip)
{
	int tmp;
	struct lpddr2_params *params = &ddr_params->private_params.lpddr2_params;

	params->tDQSCK 		= chip->DDR_tDQSCK;
	params->tDQSCKMAX 	= chip->DDR_tDQSCKMAX;
	params->tCKESR 		= chip->DDR_tCKESR;
	params->tXSR 		= chip->DDR_tXSR;
	params->tRTP 		= chip->DDR_tRTP;
	params->tCCD 		= ddr_params->bl/2 ; /* tCCD = BL / 2 */
	params->tFAW 		= chip->DDR_tFAW;
	params->tMRD 		= chip->DDR_tMRW;

	if(params->RL == -1)
	{
		tmp = find_ddr_lattency(rl_LPDDR2,sizeof(rl_LPDDR2),ddr_params->freq);
		if(tmp == -1) {
			out_error("it cann't find RL latency,when ddr frequancy is %d.check %s %d\n",
				  ddr_params->freq,__FILE__,__LINE__);
			assert(1);
		}
		params->RL = tmp * __ps_per_tck;
	}
	if(params->WL == -1)
	{
		tmp = find_ddr_lattency(wl_LPDDR2,sizeof(wl_LPDDR2),ddr_params->freq);
		if(tmp == -1) {
			out_error("it cann't find WL latency,when ddr frequancy is %d. check %s %d\n",
				  ddr_params->freq,__FILE__,__LINE__);
			assert(1);
		}
		params->WL = tmp * __ps_per_tck;
	}

	fill_mr_params_lpddr2(ddr_params);
}

static void ddrc_params_creator_lpddr2(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;
	struct lpddr2_params *params = &p->private_params.lpddr2_params;

	/**
	 * timing1
	 */
	tmp = ps2cycle_ceil(params->WL, 1);
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tWDLAT = tmp;

	/**
	   The minimum number of clock cycles from the burst WRITE command to the burst READ
	   command for any bank is [WL + 1 + BL/2 + RU(tWTR/tCK)].
	 */
	tmp = ps2cycle_ceil(params->WL + params->tWTR,1) + params->tCCD + 1;
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tWTR = tmp;

	/**
	 * timing2
	 */
	tmp = ps2cycle_ceil(params->RL, 1) - 2;
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tRDLAT = tmp;

	/**
	   LPDDR2 spec 68 page
	   The minimum time from the burst READ command to the burst WRITE command is
	   defined by the read latency (RL) and the burst length (BL). Minimum READ-to-WRITE
	   latency is RL + RU(tDQSCK(MAX)/tCK) + BL/2 + 1 - WL clock cycles. Note that if a READ
	   burst is truncated with a burst TERMINATE (BST) command, the effective burst length
	   of the truncated READ burst should be used for BL when calculating the minimum
	   READ-to-WRITE delay.
	*/
	tmp = ps2cycle_ceil(params->RL + params->tDQSCKMAX - params->WL, 1) + params->tCCD + 1;
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tRTW = tmp;

	DDRC_TIMING_SET(2,lpddr2_params,tRTP,6);

	/**
	 * timing3
	 */
	ddrc->timing3.b.tCCD = params->tCCD;

	/**
	 * timing4
	 */
	tmp = ps2cycle_ceil(params->tFAW,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing4.b.tFAW = tmp;

	/**
	 * timing5
	*/
	tmp = ps2cycle_ceil(params->tXSR,4) / 4;
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tXS = tmp;

	tmp = ps2cycle_ceil(params->tCKESR,1);
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tCKESR = tmp;


	/* For LPDDR2,tCKSRE, 2 tCK is recommended. */
	ddrc->timing5.b.tCKSRE = 2;
}

static void ddrp_params_creator_lpddr2(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	struct lpddr2_params *params = &p->private_params.lpddr2_params;
	int tmp;

	tmp =ps2cycle_ceil(params->WL,1);
	ASSERT_MASK(tmp,4);
	ddrp->cwl = tmp;

	tmp =ps2cycle_ceil(params->RL,1);
	ASSERT_MASK(tmp,8);
	ddrp->cl = tmp;
}
static struct ddr_creator_ops lpddr2_creator_ops = {
	.type = LPDDR2,
	.fill_in_params = fill_in_params_lpddr2,
	.ddrc_params_creator = ddrc_params_creator_lpddr2,
	.ddrp_params_creator = ddrp_params_creator_lpddr2,

};
void lpddr2_creator_init(void)
{
	register_ddr_creator(&lpddr2_creator_ops);
}
