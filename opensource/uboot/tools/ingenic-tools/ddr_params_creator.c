/*
 * Jz4775 ddr parameters creator.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include "ddr_params_creator.h"

unsigned int __ps_per_tck = -1;
/**
 * ps2cycle:   translate timing(ps) to clk count
 *       ps:   timing(ps)
 *  div_clk:   divider for register.
 *			   eg:
 *                    regcount = ps2cycle(x,div_tck) / div_tck.
 *                    div_tck is divider.
 */

int ps2cycle_ceil(int ps,int div_tck)
{
	return (ps + div_tck * __ps_per_tck - 1) / __ps_per_tck;
}

union ddrc_refcnt {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned reserved0:1;
		unsigned clk_div:3;
		unsigned reserved4_15:12;
		unsigned con:8;
		unsigned rfc:6;
		unsigned reserved30_31:2;
	} refcnt; /* DREFCNT */
};
union ddrc_refcnt drefcnt;
int ps2cycle_floor(int ps)
{
	return ps / __ps_per_tck;
}
static int ddr_refi_div(int reftck, unsigned int *div);

static void get_refcnt_value(struct ddr_params *p, unsigned int *rfc, unsigned int *con, unsigned int *clk_div)
{
	unsigned int tmp = 0, div = 0;

	tmp = ps2cycle_floor(p->private_params.ddr_base_params.tREFI);
	/* TODO: x2000 need??*/
	tmp -= 16; // controller is add 16 cycles.
	if(tmp < 0){
		out_error("tREFI[%d] is too small. check %s %d\n",
				  p->private_params.ddr_base_params.tREFI,
				  __FILE__,__LINE__);
	}

	tmp = ddr_refi_div(tmp, &div);
	if(div > 7){
		out_error("tREFI[%d] is too large. check %s %d\n",
				  p->private_params.ddr_base_params.tREFI,
				  __FILE__,__LINE__);
	}
	ASSERT_MASK(tmp,8);
	*con = tmp;
	*clk_div = div;

	tmp = ps2cycle_ceil(p->private_params.ddr_base_params.tRFC,2) / 2;
	if(tmp < 0)
		tmp = 0;
	ASSERT_MASK(tmp, 6);
	*rfc = tmp;
}
static void get_dynamic_refcnt(struct ddr_params *p)
{
	unsigned int div = 0;
	unsigned int rate = CONFIG_SYS_MEM_FREQ;
	unsigned int rfc, con, clk_div;

	printf("static unsigned int get_refcnt_value(int div)\n");
	printf("{\n");
	printf("\tswitch(div) { \n");
	do {
		rate = CONFIG_SYS_MEM_FREQ / (div + 1);
		__ps_per_tck = (1000000000 / (rate / 1000));

		get_refcnt_value(p, &rfc, &con, &clk_div);

		drefcnt.refcnt.clk_div = clk_div;
		drefcnt.refcnt.con = con;
		drefcnt.refcnt.rfc = rfc;
		/* printf("#define DDRC_REFCNT_VALUE_%d		0x%08x\n", div, drefcnt.d32);
 */
		printf("\t\tcase %d: return 0x%x; \n", div, drefcnt.d32);
		div++;
	} while(rate > 100000000);
	printf("\t\tdefalut :\n");
	printf("\t\tprintf(\"not support \\n\");\n");
	printf("\t}\n");
	printf("}\n");
}
struct ddr_out_impedance* find_nearby_impedance(struct ddr_out_impedance *table,int table_size,int r_ohm)
{
	int i;
	if(r_ohm >= table[0].r)
		return &table[0];
	for(i = 1;i < table_size/sizeof(struct ddr_out_impedance);i++){
		int diff = (table[i].r + table[i-1].r) / 2;
		if(r_ohm > diff)
			return &table[i - 1];
	}
	return &table[i - 1];
}
static unsigned int sdram_size(int cs, struct ddr_params *p)
{
	unsigned int dw;
	unsigned int banks;
	unsigned int size = 0;
	unsigned int row, col;

	switch (cs) {
	case 0:
		if (p->cs0 == 1) {
			row = p->row;
			col = p->col;
			break;
		} else
			return 0;
	case 1:
		if (p->cs1 == 1) {
			row = p->row1;
			col = p->col1;
			break;
		} else
			return 0;
	default:
		return 0;
	}

	banks = p->bank8 ? 8 : 4;
	dw = p->dw32 ? 4 : 2;
	size = (1 << (row + col)) * dw * banks;

	return size;
}

static void ddr_base_params_fill(struct ddr_params *ddr_params)
{
	struct ddr_params_common *params = &ddr_params->private_params.ddr_base_params;
	memset(&ddr_params->private_params, 0, sizeof(union private_params));
#ifdef CONFIG_DDR_TYPE_DDR2
	/* timing1 */
	DDR_PARAMS_FILL(params,WL);
	DDR_PARAMS_FILL(params,tWR);
	DDR_PARAMS_FILL(params,tWTR);
	DDR_PARAMS_FILL(params,tWDLAT);
	/* timing2 */
	DDR_PARAMS_FILL(params,RL);
	DDR_PARAMS_FILL(params,tRTP);
	DDR_PARAMS_FILL(params,tRTW);
	DDR_PARAMS_FILL(params,tRDLAT);
	/* timing3 */
	DDR_PARAMS_FILL(params,tRP);
	DDR_PARAMS_FILL(params,tCCD);
	DDR_PARAMS_FILL(params,tRCD);
	/* tEXTRW = 3; */
	/* timing4 */
	DDR_PARAMS_FILL(params,tRRD);
	DDR_PARAMS_FILL(params,tRAS);
	DDR_PARAMS_FILL(params,tRC);
	DDR_PARAMS_FILL(params,tFAW);
	/* timing5 */
	DDR_PARAMS_FILL(params,tCKE);
	DDR_PARAMS_FILL(params,tXP);
	DDR_PARAMS_FILL(params,tCKSRE);
	DDR_PARAMS_FILL(params,tCKESR);
	DDR_PARAMS_FILL(params,tXSNR);
#else
	DDR_PARAMS_FILL(params,tRAS);
	DDR_PARAMS_FILL(params,tRP);
	DDR_PARAMS_FILL(params,tRCD);
	DDR_PARAMS_FILL(params,tRC);
	DDR_PARAMS_FILL(params,tWR);
	DDR_PARAMS_FILL(params,tRRD);
	DDR_PARAMS_FILL(params,tWTR);
	DDR_PARAMS_FILL(params,tXP);
	DDR_PARAMS_FILL(params,tCKE);
	DDR_PARAMS_FILL(params,WL);
	DDR_PARAMS_FILL(params,RL);
#endif
/* refcnt */
	DDR_PARAMS_FILL(params,tRFC);
	DDR_PARAMS_FILL(params,tREFI);
#ifdef CONFIG_DDR_tREFI
	params->tREFI = CONFIG_DDR_tREFI;
#endif
}
static int ddr_refi_div(int reftck, unsigned *div)
{
	int tmp = 0,factor = 0;
	do{
		tmp = reftck;
		tmp = tmp / (16 << factor);
		factor++;
	}while(tmp > 255);
	*div = (factor - 1);
	return tmp;
}
static void ddrc_base_params_creator_common(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	int tmp;
	int div;
	unsigned int rfc = 0, dasr_cnt = 0;
	/* tWTR is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	/* tRTP is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	/* tCCD is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	/**
	 * timing1
	 */
	DDRC_TIMING_SET(1,ddr_base_params,tWR,6);

	tmp =ps2cycle_ceil(p->private_params.ddr_base_params.WL,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing1.b.tWL = tmp;

	/**
	 * timing2
	 */

	tmp =ps2cycle_ceil(p->private_params.ddr_base_params.RL,1);
	ASSERT_MASK(tmp,6);
	ddrc->timing2.b.tRL = tmp;

	/**
	 * timing3
	 */
	DDRC_TIMING_SET(3,ddr_base_params,tRCD,6);
	DDRC_TIMING_SET(3,ddr_base_params,tRP,6);
	ddrc->timing3.b.tEXTRW = 3; /* default 1, double cs test error so change tEXTRW 3*/

	/**
	 * timing4
	 */
	DDRC_TIMING_SET(4,ddr_base_params,tRAS,6);
	DDRC_TIMING_SET(4,ddr_base_params,tRRD,6);
	DDRC_TIMING_SET(4,ddr_base_params,tRC,6);


	/**
	 * timing5
	 */
	tmp = ps2cycle_ceil(p->private_params.ddr_base_params.tCKE,1);
	ASSERT_MASK(tmp,3);
	ddrc->timing5.b.tCKE = tmp;
	/* tCKSRE is differ in lpddr & lpddr2 & ddr2 & ddr3*/

	tmp = ps2cycle_ceil(p->private_params.ddr_base_params.tXP,1);
	if (tmp == 0)
		tmp += 1;
	ASSERT_MASK(tmp,4);
	ddrc->timing5.b.tXP = tmp;


	/* tRTW is differ in lpddr & lpddr2 & ddr2 & ddr3 */
	/* tRDLAT is differ in lpddr & lpddr2 & ddr2 & ddr3 */
	/* tWDLAT is differ in lpddr & lpddr2 & ddr2 & ddr3 */
	/* tFAW is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	/* tXSR is differ in lpddr & lpddr2 & ddr2 & ddr3*/
	{
		unsigned int rfc, con, clk_div;
		get_refcnt_value(p, &rfc, &con, &clk_div);
		ddrc->refcnt = (con << DDRC_REFCNT_CON_BIT)
			| (clk_div << DDRC_REFCNT_CLK_DIV_BIT)
			| DDRC_REFCNT_REF_EN
#ifdef CONFIG_DDR_TYPE_DDR3
			| DDRC_REFCNT_PREREF_EN
#endif
			| DDRC_REFCNT_PREREF_CNT_DEFAULT
			| rfc << DDRC_REFCNT_TRFC_BIT;
	}

	/* tmp = ps2cycle_floor(p->private_params.ddr_base_params.tREFI);//???????????????????? */

	/* tmp -= 16; // controller is add 16 cycles. */
	/* TODO: x2000 need??*/

	/* } */
	/* tmp = ddr_refi_div(tmp,&div); */
	/* if(div > 7){ */
	/* 	out_error("tREFI[%d] is too large. check %s %d\n", */
	/* 		  p->private_params.ddr_base_params.tREFI, */
	/* 		  __FILE__,__LINE__); */
	/* } */
	/* ASSERT_MASK(tmp,8); */


	/* ddrc->refcnt = (tmp << DDRC_REFCNT_CON_BIT) */
	/* 	| (div << DDRC_REFCNT_CLK_DIV_BIT) */
	/* 	| DDRC_REFCNT_REF_EN */
	/* 	| DDRC_REFCNT_PREREF_EN */
	/* 	| DDRC_REFCNT_PREREF_CNT_DEFAULT; */

	/* tmp = ps2cycle_ceil(p->private_params.ddr_base_params.tRFC,2) / 2; */
	/* if(tmp < 0) */
	/* 	tmp = 0; */
	/* ASSERT_MASK(tmp,6); */
	/* ddrc->refcnt |= tmp << DDRC_REFCNT_TRFC_BIT; */


	rfc = ps2cycle_floor(p->private_params.ddr_base_params.tRFC) / 2;
	dasr_cnt = ps2cycle_floor(p->private_params.ddr_base_params.tREFI);
	ddrc->autosr_cnt = rfc << 24 | dasr_cnt;

	ddrc->autosr_en = 0;
#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddrc->autosr_en = 1;
#endif
}

static void ddrc_config_creator(struct ddrc_reg *ddrc, struct ddr_params *p)
{
	unsigned int  mem_base0 = 0, mem_base1 = 0, mem_mask0 = 0, mem_mask1 = 0;
	unsigned int memsize_cs0, memsize_cs1, memsize;
	/* CFG */
	ddrc->cfg.b.ROW1 = p->row1 - 12;
	ddrc->cfg.b.COL1 = p->col1 - 8;
	ddrc->cfg.b.BA1 = p->bank8;
	ddrc->cfg.b.IMBA = 0;
//	ddrc->cfg.b.BSL = (p->bl == 8) ? 1 : 0; // OLD CFG value

#ifdef CONFIG_DDR_CHIP_ODT
	/*  Only support for DDR2, DDR3 and	LPDDR3 */
	ddrc->cfg.b.ODTEN = 1;
#else
	ddrc->cfg.b.ODTEN = 0;
#endif
//	ddrc->cfg.b.MISPE = 1; // OLD CFG value
	ddrc->cfg.b.ROW0 = p->row - 12;
	ddrc->cfg.b.COL0 = p->col - 8;

	ddrc->cfg.b.CS1EN = p->cs1;
	ddrc->cfg.b.CS0EN = p->cs0;

//	ddrc->cfg.b.CL = 0; /* defalt and NOT used in this version */

	ddrc->cfg.b.BA0 = p->bank8; // OLD CFG value
//	ddrc->cfg.b.DW = p->dw32; // OLD CFG value

	switch (p->type) {
#define _CASE(D, P)				\
		case D:				\
			ddrc->cfg.b.TYPE = P;	\
			break
		_CASE(DDR3, 6);		/* DDR3:0b110 */
		_CASE(LPDDR, 3);	/* LPDDR:0b011 */
		_CASE(LPDDR2, 5);	/* LPDDR2:0b101 */
		_CASE(DDR2, 4);	    /* DDR2:0b100 */
#undef _CASE
	default:
		out_error("don't support the ddr type.!");
		assert(1);
		break;
	}

	/* CTRL */
#ifdef CONFIG_DDR_TYPE_DDR3
	ddrc->ctrl = DDRC_CTRL_ACTPD | DDRC_CTRL_PDT_32  | DDRC_CTRL_CKE |
		DDRC_CTRL_PD_CCE | DDRC_CTRL_SR_CCE;
#else
	ddrc->ctrl = DDRC_CTRL_ACTPD | DDRC_CTRL_PDT_DIS | DDRC_CTRL_CKE |
		DDRC_CTRL_PD_CCE | DDRC_CTRL_SR_CCE;
#endif

	/* MMAP0,1 */
	memsize_cs0 = p->size.chip0;
	memsize_cs1 = p->size.chip1;
	memsize = memsize_cs0 + memsize_cs1;

	if (memsize > 0x20000000) {
		if (memsize_cs1) {
			mem_base0 = 0x0;
			mem_mask0 = (~((memsize_cs0 >> 24) - 1) & ~(memsize >> 24))
				& DDRC_MMAP_MASK_MASK;
			mem_base1 = (memsize_cs1 >> 24) & 0xff;
			mem_mask1 = (~((memsize_cs1 >> 24) - 1) & ~(memsize >> 24))
				& DDRC_MMAP_MASK_MASK;
		} else {
			mem_base0 = 0x0;
			mem_mask0 = ~(((memsize_cs0 * 2) >> 24) - 1) & DDRC_MMAP_MASK_MASK;
			mem_mask1 = 0;
			mem_base1 = 0xff;
		}
	} else {
		mem_base0 = (DDR_MEM_PHY_BASE >> 24) & 0xff;
		mem_mask0 = ~((memsize_cs0 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
		mem_base1 = ((DDR_MEM_PHY_BASE + memsize_cs0) >> 24) & 0xff;
		mem_mask1 = ~((memsize_cs1 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
	}
	ddrc->mmap[0] = mem_base0 << DDRC_MMAP_BASE_BIT | mem_mask0;
	ddrc->mmap[1] = mem_base1 << DDRC_MMAP_BASE_BIT | mem_mask1;

	ddrc->hregpro = DDRC_HREGPRO_HPRO_EN;
	ddrc->pregpro = DDRC_PREGPRO_PPRO_EN;

	ddrc->cguc0 = DDRC_CGU_PORT7 |
		DDRC_CGU_PORT6 |
		DDRC_CGU_PORT5 |
		DDRC_CGU_PORT4 |
		DDRC_CGU_PORT3 |
		DDRC_CGU_PORT2 |
		DDRC_CGU_PORT1 |
		DDRC_CGU_PORT0;

	ddrc->cguc1 = DDRC_CGU_BWM |
		DDRC_CGU_PCTRL |
		DDRC_CGU_SCH |
		DDRC_CGU_PA;
}

#ifndef CONFIG_DDR_INNOPHY
static void ddrp_base_params_creator_common(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	int tmp = 0;

	switch (p->type) {
#define _CASE(D, P)				\
		case D:				\
			tmp = P;		\
			break
		_CASE(DDR3, 3);		/* DDR3:0b110 */
		_CASE(LPDDR, 0);	/* LPDDR:0b011 */
		_CASE(LPDDR2, 4);	/* LPDDR2:0b101 */
		_CASE(DDR2, 2);	    /* DDR2:0b100 */
#undef _CASE
	default:
		break;
	}
	ddrp->dcr = tmp | (p->bank8 << 3);

	/* MR0'Register is differ in lpddr ddr2 lpddr2 ddr3 */

	/* DTPR0 registers */
	/* tMRD is differ in lpddr ddr2 lpddr2 ddr3 */
	/* tRTP is differ for ddr2 */
	DDRP_TIMING_SET(0,ddr_base_params,tWTR,3,1,6);
	DDRP_TIMING_SET(0,ddr_base_params,tRP,4,2,11);
	DDRP_TIMING_SET(0,ddr_base_params,tRCD,4,2,11);
	DDRP_TIMING_SET(0,ddr_base_params,tRAS,5,2,31);
	DDRP_TIMING_SET(0,ddr_base_params,tRRD,4,1,8);
	DDRP_TIMING_SET(0,ddr_base_params,tRC,6,2,42);
	/* tCCD is differ in lpddr ddr2 lpddr2 ddr3 */

	/* DTPR1 registers */
	/* tAOND is only used by DDR2. */
	/* tRTW is differ in lpddr ddr2 lpddr2 ddr3 */
	/* tFAW is differ in lpddr ddr2 lpddr2 ddr3 */
	/* tMOD is used by ddr3 */
	/* tRTODT is used by ddr3 */
	DDRP_TIMING_SET(1,ddr_base_params,tRFC,8,0,255);
	/* tDQSCKmin is used by lpddr2 */
	/* tDQSCKmax is used by lpddr2 */

	/* DTPR2 registers */
	/* tXS is differ in lpddr2 ddr3 */
	/* tXP is differ in lpddr2 ddr3 */
	/* tCKE is differ in ddr3 */

	/* PTRn registers */
	tmp = ps2cycle_ceil(50 * 1000,1);   /* default 50 ns and min clk is 8 cycle */
	ASSERT_MASK(tmp,6);
	if(tmp < 8) tmp = 8;
	ddrp->ptr0.b.tDLLSRST = tmp;


	tmp = ps2cycle_ceil(5120*1000, 1); /* default 5.12 us*/
	ASSERT_MASK(tmp,12);
	ddrp->ptr0.b.tDLLLOCK = tmp;

	ddrp->ptr0.b.tITMSRST = 8;    /* default 8 cycle & more than 8 */

	//BETWEEN(tmp, 2, 1023);
	ddrp->dtpr2.b.tDLLK = 512;
	/* PGCR'Register is differ in lpddr ddr2 lpddr2 ddr3 */
}
#else
static void ddrp_base_params_creator_common(struct ddrp_reg *ddrp, struct ddr_params *p){}
#endif

void init_ddr_params_common(struct ddr_params *ddr_params,int type)
{
	ddr_params->type = type;
	ddr_params->freq = CONFIG_SYS_MEM_FREQ;
	ddr_params->cs0 = CONFIG_DDR_CS0;
	ddr_params->cs1 = CONFIG_DDR_CS1;
	ddr_params->dw32 = CONFIG_DDR_DW32;
	ddr_params->bl = DDR_BL;
	ddr_params->cl = DDR_CL;
#ifdef CONFIG_DDR3
	ddr_params->cwl = DDR_tCWL;
#endif
	ddr_params->col = DDR_COL;
	ddr_params->row = DDR_ROW;

#ifdef DDR_COL1
	ddr_params->col1 = DDR_COL1;
#endif
#ifdef DDR_ROW1
	ddr_params->row1 = DDR_ROW1;
#endif
	ddr_params->bank8 = DDR_BANK8;
	ddr_params->size.chip0 = sdram_size(0, ddr_params);
	ddr_params->size.chip1 = sdram_size(1, ddr_params);
}



/* #define CONFIG_DDR_CHIP_IMPEDANCE */
#ifndef CONFIG_DDR_INNOPHY
static void ddrp_config_creator(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	int i;
	unsigned char rzq[]={
		0x00,0x01,0x02,0x03,0x06,0x07,0x04,0x05,
		0x0C,0x0D,0x0E,0x0F,0x0A,0x0B,0x08,0x09,
		0x18,0x19,0x1A,0x1B,0x1E,0x1F,0x1C,0x1D,
		0x14,0x15,0x16,0x17,0x12,0x13,0x10,0x11};  //from ddr multiphy page 158.
	ddrp->odtcr.d32 = 0;
#ifdef CONFIG_DDR_CHIP_ODT
	ddrp->odtcr.d32 = 0x84210000;   //power on default.
#endif
	/* DDRC registers assign */
	for(i = 0;i < 4;i++)
		ddrp->dxngcrt[i].d32 = 0x00090e80;
	i = 0;
#ifdef CONFIG_DDR_PHY_ODT
	for(i = 0;i < (CONFIG_DDR_DW32 + 1) * 2;i++){
		ddrp->dxngcrt[i].b.dqsrtt = 1;
		ddrp->dxngcrt[i].b.dqrtt = 1;
		ddrp->dxngcrt[i].b.dqsodt = 1;
		ddrp->dxngcrt[i].b.dqodt = 1;
	}
#else
	for(i = 0;i < (CONFIG_DDR_DW32 + 1) * 2;i++){
		ddrp->dxngcrt[i].b.dqsrtt = 0;
		ddrp->dxngcrt[i].b.dqrtt = 0;
		ddrp->dxngcrt[i].b.dxen = 1;
	}
#endif
	for(i = 0;i<sizeof(rzq);i++)
		    ddrp->rzq_table[i] = rzq[i];
}
#else
static void ddrp_config_creator(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	switch (p->type) {
#define _CASE(D, P)				\
		case D:				\
			ddrp->memcfg.b.memsel = P;\
			break
		_CASE(LPDDR2, 3);
		_CASE(DDR2, 1);
		_CASE(DDR3, 0);
#undef _CASE
	default:
		break;
	}

	if(p->bl == 4)
		ddrp->memcfg.b.brusel = 0;
	else if(p->bl == 8)
		ddrp->memcfg.b.brusel = 1;

	ddrp->cl = p->cl;
#ifdef CONFIG_DDR3
	ddrp->cwl = p->cwl;
#else
	ddrp->cwl = p->cl - 2;
#endif

}
#endif

static void Timing_Reg_print(struct ddrc_reg * ddrc)
{
	printf("#define timing1_tWL		%d\n", ddrc->timing1.b.tWL);
	printf("#define timing1_tWR		%d\n", ddrc->timing1.b.tWR);
	printf("#define timing1_tWTR		%d\n", ddrc->timing1.b.tWTR);
	printf("#define timing1_tWDLAT		%d\n", ddrc->timing1.b.tWDLAT);

	printf("#define timing2_tRL		%d\n", ddrc->timing2.b.tRL);
	printf("#define timing2_tRTP		%d\n", ddrc->timing2.b.tRTP);
	printf("#define timing2_tRTW		%d\n", ddrc->timing2.b.tRTW);
	printf("#define timing2_tRDLAT		%d\n", ddrc->timing2.b.tRDLAT);

	printf("#define timing3_tRP		%d\n", ddrc->timing3.b.tRP);
	printf("#define timing3_tCCD		%d\n", ddrc->timing3.b.tCCD);
	printf("#define timing3_tRCD		%d\n", ddrc->timing3.b.tRCD);
	printf("#define timing3_ttEXTRW		%d\n", ddrc->timing3.b.tEXTRW);

	printf("#define timing4_tRRD		%d\n", ddrc->timing4.b.tRRD);
	printf("#define timing4_tRAS		%d\n", ddrc->timing4.b.tRAS);
	printf("#define timing4_tRC		%d\n", ddrc->timing4.b.tRC);
	printf("#define timing4_tFAW		%d\n", ddrc->timing4.b.tFAW);

	printf("#define timing5_tCKE		%d\n", ddrc->timing5.b.tCKE);
	printf("#define timing5_tXP		%d\n", ddrc->timing5.b.tXP);
	printf("#define timing5_tCKSRE		%d\n", ddrc->timing5.b.tCKSRE);
	printf("#define timing5_tCKESR		%d\n", ddrc->timing5.b.tCKESR);
	printf("#define timing5_tXS		%d\n", ddrc->timing5.b.tXS);
}
static void params_print(struct ddrc_reg *ddrc, struct ddrp_reg *ddrp)
{
	int i;
	/* DDRC registers print */
	printf("#define DDRC_CFG_VALUE			0x%08x\n", ddrc->cfg.d32);
	printf("#define DDRC_CTRL_VALUE			0x%08x\n", ddrc->ctrl);
	printf("#define DDRC_DLMR_VALUE			0x%08x\n", ddrc->dlmr);
	printf("#define DDRC_DDLP_VALUE			0x%08x\n", ddrc->ddlp);
	printf("#define DDRC_MMAP0_VALUE		0x%08x\n", ddrc->mmap[0]);
	printf("#define DDRC_MMAP1_VALUE		0x%08x\n", ddrc->mmap[1]);
	printf("#define DDRC_REFCNT_VALUE		0x%08x\n", ddrc->refcnt);
	printf("#define DDRC_TIMING1_VALUE		0x%08x\n", ddrc->timing1.d32);
	printf("#define DDRC_TIMING2_VALUE		0x%08x\n", ddrc->timing2.d32);
	printf("#define DDRC_TIMING3_VALUE		0x%08x\n", ddrc->timing3.d32);
	printf("#define DDRC_TIMING4_VALUE		0x%08x\n", ddrc->timing4.d32);
	printf("#define DDRC_TIMING5_VALUE		0x%08x\n", ddrc->timing5.d32);
	printf("#define DDRC_AUTOSR_CNT_VALUE		0x%08x\n", ddrc->autosr_cnt);
	printf("#define DDRC_AUTOSR_EN_VALUE		0x%08x\n", ddrc->autosr_en);
	printf("#define DDRC_HREGPRO_VALUE		0x%08x\n", ddrc->hregpro);
	printf("#define DDRC_PREGPRO_VALUE		0x%08x\n", ddrc->pregpro);
	printf("#define DDRC_CGUC0_VALUE		0x%08x\n", ddrc->cguc0);
	printf("#define DDRC_CGUC1_VALUE		0x%08x\n", ddrc->cguc1);

#ifndef CONFIG_DDR_INNOPHY
	/* DDRP registers print */
	printf("#define DDRP_DCR_VALUE			0x%08x\n", ddrp->dcr);
	printf("#define	DDRP_MR0_VALUE			0x%08x\n", ddrp->mr0.d32);
	printf("#define	DDRP_MR1_VALUE			0x%08x\n", ddrp->mr1.d32);
	printf("#define	DDRP_MR2_VALUE			0x%08x\n", ddrp->mr2.d32);
	printf("#define	DDRP_MR3_VALUE			0x%08x\n", ddrp->mr3.d32);
	printf("#define	DDRP_PTR0_VALUE			0x%08x\n", ddrp->ptr0.d32);
	printf("#define	DDRP_PTR1_VALUE			0x%08x\n", ddrp->ptr1.d32);
	printf("#define	DDRP_PTR2_VALUE			0x%08x\n", ddrp->ptr2.d32);
	printf("#define	DDRP_DTPR0_VALUE		0x%08x\n", ddrp->dtpr0.d32);
	printf("#define	DDRP_DTPR1_VALUE		0x%08x\n", ddrp->dtpr1.d32);
	printf("#define	DDRP_DTPR2_VALUE		0x%08x\n", ddrp->dtpr2.d32);
	printf("#define	DDRP_PGCR_VALUE			0x%08x\n", ddrp->pgcr);
	printf("#define DDRP_ODTCR_VALUE		0x%08x\n", ddrp->odtcr.d32);
	for(i = 0;i < 4;i++){
		printf("#define DDRP_DX%dGCR_VALUE              0x%08x\n",i,ddrp->dxngcrt[i].d32);
	}
	printf("#define DDRP_ZQNCR1_VALUE               0x%08x\n", ddrp->zqncr1);
	printf("#define DDRP_IMPANDCE_ARRAY             {0x%08x,0x%08x} //0-cal_value 1-req_value\n", ddrp->impedance[0],ddrp->impedance[1]);
	printf("#define DDRP_ODT_IMPANDCE_ARRAY         {0x%08x,0x%08x} //0-cal_value 1-req_value\n", ddrp->odt_impedance[0],ddrp->odt_impedance[1]);
	printf("#define DDRP_RZQ_TABLE  {0x%02x",ddrp->rzq_table[0]);
	for(i = 1;i < sizeof(ddrp->rzq_table);i++){
		printf(",0x%02x",ddrp->rzq_table[i]);
	}
	printf("}\n");
#else
	printf("#define DDRP_MEMCFG_VALUE		0x%08x\n", ddrp->memcfg.d32);
	printf("#define DDRP_CL_VALUE			0x%08x\n", ddrp->cl);
	printf("#define DDRP_CWL_VALUE			0x%08x\n", ddrp->cwl);
#endif
}

static void ddr_mr_print(struct ddr_params *p)
{
	printf("#define	DDR_MR0_VALUE			0x%08x\n", p->mr0.d32);
	printf("#define	DDR_MR1_VALUE			0x%08x\n", p->mr1.d32);
	printf("#define	DDR_MR2_VALUE			0x%08x\n", p->mr2.d32);
	printf("#define	DDR_MR3_VALUE			0x%08x\n", p->mr3.d32);
	printf("#define	DDR_MR10_VALUE			0x%08x\n", p->mr10.d32);
	printf("#define	DDR_MR63_VALUE			0x%08x\n", p->mr63.d32);
}
static void sdram_size_print(struct ddr_params *p)
{
	printf("#define	DDR_CHIP_0_SIZE			%u\n", p->size.chip0);
	printf("#define	DDR_CHIP_1_SIZE			%u\n", p->size.chip1);
}
static unsigned int frandom(int max)
{
	unsigned int rn;
	rn += 0xffffffff;
	max = rn % max;
	return max;
}

#define swap_bytes(buf,b1,b2) do{				\
		unsigned char swap;						\
		swap = buf[b1];							\
		buf[b1] = buf[b2];						\
		buf[b2] = swap;							\
	}while(0)

static void mem_remap_print(struct ddr_params *p)
{
	int address_bits;
	int swap_bits;
	int bank_bits;
	int startA, startB;
	int bit_width;
	unsigned int remap_array[5];
	unsigned char *s;
	int i,width;
	s = (unsigned char *)remap_array;
	for(i = 0;i < sizeof(remap_array);i++)
	{
		s[i] = i;
	}
	if(p->size.chip1 && (p->size.chip0 != p->size.chip1))
		return;

	bank_bits = DDR_BANK8 == 1 ? 3 : 2;
	bit_width = CONFIG_DDR_DW32 == 1 ? 2 : 1;

	/*
	 * count the chips's address space bits.
	 */
	address_bits =  bit_width + DDR_COL + DDR_ROW + bank_bits + (CONFIG_DDR_CS0 + CONFIG_DDR_CS1 - 1);
	/*
	 * count address space bits for swap.
	 */
	swap_bits = bank_bits + (CONFIG_DDR_CS0 + CONFIG_DDR_CS1 - 1);

	startA = bit_width + DDR_COL > 12 ? bit_width + DDR_COL : 12;

	startB = address_bits - swap_bits - startA;
	startA = startA - 12;
    /*
	 * bank and cs swap with row.
	 */
	for(i = startA;i < swap_bits;i++){
		swap_bytes(s,startA + i,startB + i);
		//swap_bytes(s,startA + i,startB + i,startB);
	}


    /*
	 * random high address for securing.
	 */
#if 0
	for(i = 0;i < swap_bits / 2;i++){
		int sw = frandom(swap_bits - 1 - i);
		swap_bytes(s,startA + i,startA + sw);
	}

	width = startB + startA;
	startA = startA + swap_bits;
	for(i = 0;i < width / 2;i++){
		int sw = frandom(width - 1 - i);
		swap_bytes(s,startA + i,startA + sw);
	}
#endif
	printf("#define REMMAP_ARRAY {\\\n");
	for(i = 0;i <sizeof(remap_array) / sizeof(remap_array[0]);i++)
	{
		printf("\t0x%08x,\\\n",remap_array[i]);
	}
	printf("};\n");
}
static void file_head_print(void)
{
	printf("/*\n");
	printf(" * DO NOT MODIFY.\n");
	printf(" *\n");
	printf(" * This file was generated by ddr_params_creator\n");
	printf(" *\n");
	printf(" */\n");
	printf("\n");

	printf("#ifndef __DDR_REG_VALUES_H__\n");
	printf("#define __DDR_REG_VALUES_H__\n\n");
}

static void file_end_print(void)
{
	printf("\n#endif /* __DDR_REG_VALUES_H__ */\n");
}

static struct ddr_creator_ops *p_ddr_creator = NULL;
static int ops_count = 0;
void register_ddr_creator(struct ddr_creator_ops *ops)
{
	if(ops_count++ == 0){
		p_ddr_creator = ops;
	}else{
		out_error("Error: DDR CREATEOR cann't register %d\n",ops->type);
	}
}
/**
 * ddr parameter prev setting :
 *    1.  the ddr chip parameter is filled.
 *    2.  the ddr controller parameter is generated.
 *    3.  the ddr phy paramerter is generated.
 *    4.  all parameter is outputted.
 */
int main(int argc, char *argv[])
{
	struct ddrc_reg ddrc;
	struct ddrp_reg ddrp;
	struct ddr_params ddr_params;
	__ps_per_tck = (1000000000 / (CONFIG_SYS_MEM_FREQ / 1000));
	memset(&ddrc, 0, sizeof(struct ddrc_reg));
	memset(&ddrp, 0, sizeof(struct ddrp_reg));
	memset(&ddr_params, 0, sizeof(struct ddr_params));
	ddr_creator_init();

	init_ddr_params_common(&ddr_params,p_ddr_creator->type);
	ddr_base_params_fill(&ddr_params);
	p_ddr_creator->fill_in_params(&ddr_params);

	ddrc_base_params_creator_common(&ddrc, &ddr_params);
	p_ddr_creator->ddrc_params_creator(&ddrc, &ddr_params);
	ddrc_config_creator(&ddrc,&ddr_params);

	ddrp_base_params_creator_common(&ddrp, &ddr_params);
	p_ddr_creator->ddrp_params_creator(&ddrp, &ddr_params);
	ddrp_config_creator(&ddrp,&ddr_params);
	file_head_print();
	params_print(&ddrc, &ddrp);
#ifdef CONFIG_DDR_INNOPHY
	ddr_mr_print(&ddr_params);
#endif
	sdram_size_print(&ddr_params);
	mem_remap_print(&ddr_params);
	Timing_Reg_print(&ddrc);
	get_dynamic_refcnt(&ddr_params);
	file_end_print();

	return 0;
}
