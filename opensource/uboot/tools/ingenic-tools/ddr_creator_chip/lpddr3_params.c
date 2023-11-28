#include "ddr_params_creator.h"
struct ddr_latency_table
{
	unsigned int freq;
	int latency;
};

static struct ddr_latency_table rl_lpddr3[] = {
	{333000000,3},/*Date Rate xxM, RL*/
	{800000000,6},
	{1066000000,8},
	{1200000000,9},
	{1333000000,10},
	{1466000000,11},
	{1600000000,12},
};

static struct ddr_latency_table wl_lpddr3[]= {
	{333000000,1},/*Date Rate xxM, WL*/
	{800000000,3},
	{1066000000,4},
	{1200000000,5},
	{1333000000,6},
	{1466000000,6},
	{1600000000,6},
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

static void fill_mr_params_lpddr3(struct ddr_params *p)
{
	int tmp;
	int rl = 0,wl = 0;
	int  count = 0;
	struct lpddr3_params *params = &p->private_params.lpddr3_params;

	/**
	 * MR1 registers
	*/
	p->mr1.d32 = 0;
	p->mr1.lpddr3.MA = 0x1;

	tmp = ps2cycle_ceil(params->tWR, 1);
	ASSERT_MASK(tmp,6);
	/* BETWEEN(tmp,3,8); */
	BETWEEN(tmp,3,16);
	if(tmp < 10)
		p->mr1.lpddr3.nWR = tmp - 2;
	else
		p->mr1.lpddr3.nWR = tmp - 10;
	if(p->bl != 8) {
		out_error("BL(%d) only support 8\n", p->bl);
		assert(1);
	}
	tmp = p->bl;
	while (tmp >>= 1) count++;
	p->mr1.lpddr3.BL = count;

	/**
	 * MR2 registers
	 */
	p->mr2.d32 = 0;
	p->mr2.lpddr3.MA = 0x2;
	tmp = ps2cycle_ceil(params->RL,1);
	if(tmp < 3 ||
	   tmp > 16)
	{
		out_error("the PHY don't support the RL(%d) \n",params->RL);
		assert(1);
	}
	rl = tmp;
	tmp = ps2cycle_ceil(params->WL,1);
	if(tmp < 1 ||
	   tmp > 8)
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
	case 0x63:
		tmp = 4;
		break;
	case 0x84:
		tmp = 6;
		break;
	case 0x95:
		tmp = 7;
		break;
	case 0xa6:
		tmp = 8;
		break;
	case 0xb6:
		tmp = 9;
		break;
	case 0xc6:
		tmp = 0xa;
		break;
	case 0xe8:
		tmp = 0xc;
		break;
	case 0x108:
		tmp = 0xe;
		break;
	default:
		out_error("the PHY don't support the WL(%d) or RL(%d)\n",
				  params->WL,params->RL);
		assert(1);
	}
	p->mr2.lpddr3.RL_WL = tmp;
	if(ps2cycle_ceil(params->tWR, 1) > 9)
		p->mr2.lpddr3.WRE = 1; //if tWR > 9 ,the value is 1;
	else
		p->mr2.lpddr3.WRE = 0; //if tWR > 9 ,the value is 1;
	p->mr2.lpddr3.WL_S = 0;
	p->mr2.lpddr3.WR_L = 0;

	/**
	 * MR3 registers
	 */
	p->mr3.d32 = 0;
	p->mr3.lpddr3.MA = 0x3;

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
	p->mr3.lpddr3.DS = CONFIG_DDR_DRIVER_STRENGTH;
#else
	p->mr3.lpddr3.DS = 2;
	out_warn("Warnning: Please set ddr driver strength.");
#endif
	/**
	 * MR10 Calibration registers
	 */
	p->mr10.d32 = 0;
	p->mr10.lpddr3.MA = 0x0a;
	/**
	   0xFF: Calibration command after initialization
	   0xAB: Long calibration
	   0x56: Short calibration
	   0xC3: ZQRESET
	*/
	p->mr10.lpddr3.CAL_CODE = 0xAB;

	/**
	 * MR11 ODT ctrl registers
	 */
	p->mr11.d32 = 0;
	p->mr11.lpddr3.MA = 0x0b;
	/**
	   00b: disable
	   01b: RZQ/4
	   10b: RZQ/2
	   11b: RZQ/1
	*/
	p->mr11.lpddr3.ODT = 0x2;

	/**
	 * MR63 reset registers, RESET (MA[7:0] = 3Fh) – MRW Only
	 */
	p->mr63.d32 = 0;
	p->mr63.lpddr3.MA = 0x3f;
	p->mr63.lpddr3.RST = 0xfc;

}

static void fill_in_params_lpddr3(struct ddr_params *ddr_params, struct ddr_chip_info *chip)
{
	int tmp;
	struct lpddr3_params *params = &ddr_params->private_params.lpddr3_params;


	params->tMRD 		= chip->DDR_tMRW;
	params->tDQSCK 		= chip->DDR_tDQSCK;
	params->tDQSCKMAX 	= chip->DDR_tDQSCKMAX;
	params->tXSR 		= chip->DDR_tXSR;
	params->tCKESR 		= chip->DDR_tCKESR;
	params->tRTP 		= chip->DDR_tRTP;
	params->tCCD 		= ps2cycle_ceil(chip->DDR_tCCD,1);
	params->tFAW 		= chip->DDR_tFAW;
	if(params->RL == -1)
	{
		tmp = find_ddr_lattency(rl_lpddr3,sizeof(rl_lpddr3),ddr_params->freq);
		if(tmp == -1) {
			out_error("it cann't find RL latency,when ddr frequancy is %d.check %s %d\n",
				  ddr_params->freq,__FILE__,__LINE__);
			assert(1);
		}
		params->RL = tmp * __ps_per_tck;
	}
	if(params->WL == -1)
	{
		tmp = find_ddr_lattency(wl_lpddr3,sizeof(wl_lpddr3),ddr_params->freq);
		if(tmp == -1) {
			out_error("it cann't find WL latency,when ddr frequancy is %d. check %s %d\n",
				  ddr_params->freq,__FILE__,__LINE__);
			assert(1);
		}
		params->WL = tmp * __ps_per_tck;
	}

	fill_mr_params_lpddr3(ddr_params);
}

static void ddrc_params_creator_lpddr3(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;
	struct lpddr3_params *params = &p->private_params.lpddr3_params;

	tmp = ps2cycle_ceil(params->tMRD,4) / 4;
	ASSERT_MASK(tmp,5);
	ddrc->dlmr |= tmp << DDRC_LMR_TMRD_BIT;

	/**
	 * timing1
	 */
//	tmp = (ps2cycle_ceil(params->WL, 1) - 3 + 2 - 1 ) / 2 ;
	tmp = ps2cycle_ceil(params->WL, 1);
	ASSERT_MASK(tmp,6);
	/* ddrc->timing1.b.tWDLAT = tmp;//for x2000,tWDLAT=(WL-3)/2 */
	ddrc->timing1.b.tWDLAT = tmp;//for x2000,tWDLAT=(WL-3)/2

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
	/* tmp = ps2cycle_ceil(params->RL, 1) - 2; */
//	tmp = ( ps2cycle_ceil(params->RL, 1) - 3 + 2 - 1 ) / 2;//for x2000,tRDLAT=(RL-3)/2
	tmp = ps2cycle_ceil(params->RL, 1) - 2;
	ASSERT_MASK(tmp,6);
	/* ddrc->timing2.b.tRDLAT = tmp; */
	ddrc->timing2.b.tRDLAT = tmp;

	tmp = ps2cycle_ceil(params->RL + params->tDQSCKMAX - params->WL, 1) + params->tCCD + 1;
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tRTW = tmp;

	DDRC_TIMING_SET(2,lpddr2_params,tRTP,6);  //JEDC is diff

	/**
	 * timing3
	 */
	ddrc->timing3.b.tCCD = params->tCCD ;
	/**
	 * timing4
	 */
	tmp = ps2cycle_ceil(params->tFAW,1);
	ASSERT_MASK(tmp,8);
	ddrc->timing4.b.tFAW = tmp;

	/**
	 * timing5
	*/
	tmp = ps2cycle_ceil(params->tXSR,4) / 4;
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tXS = tmp;

	tmp = ps2cycle_ceil(params->tCKESR,1);
	ASSERT_MASK(tmp,8);
	ddrc->timing5.b.tCKESR = tmp;  //JEDC is diff


	/* For LPDDR2,tCKSRE, 2 tCK is recommended. */

	ddrc->timing5.b.tCKSRE = 0; // lpddr3 not used.
}

static void ddrp_params_creator_lpddr3(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	struct lpddr3_params *params = &p->private_params.lpddr3_params;
	int tmp;

	tmp =ps2cycle_ceil(params->WL,1);
	ASSERT_MASK(tmp,4);
	ddrp->cwl = tmp;

	tmp =ps2cycle_ceil(params->RL,1);
	ASSERT_MASK(tmp,4);
	ddrp->cl = tmp;
}

static struct ddr_creator_ops lpddr3_creator_ops = {
	.type = LPDDR3,
	.fill_in_params = fill_in_params_lpddr3,
	.ddrc_params_creator = ddrc_params_creator_lpddr3,
	.ddrp_params_creator = ddrp_params_creator_lpddr3,

};


void lpddr3_creator_init(void)
{
	register_ddr_creator(&lpddr3_creator_ops);
}
