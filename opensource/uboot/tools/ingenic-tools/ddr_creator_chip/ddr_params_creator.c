/*
 * X2000 ddr parameters creator.
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
#if (defined(CONFIG_X2000_V12) || defined(CONFIG_M300) || defined(CONFIG_X2100) || defined(CONFIG_X2500))
		unsigned reserved24_31:8;
#else
		unsigned rfc:6;
		unsigned reserved30_31:2;
#endif

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
	unsigned int tmp, div;

	tmp = ps2cycle_floor(p->private_params.ddr_base_params.tREFI);
	/* TODO: x2000 need??*/
#if ((!defined CONFIG_X2000_V12) && (!defined CONFIG_M300) && (!defined CONFIG_X2100) && (!defined CONFIG_X2500))
	tmp -= 16; // controller is add 16 cycles.
#endif
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
#if (defined(CONFIG_X2000_V12) || defined(CONFIG_M300) || defined(CONFIG_X2100) || defined(CONFIG_X2500))
	ASSERT_MASK(tmp, 8);
#else
	ASSERT_MASK(tmp, 6);
#endif
	*rfc = tmp;
}
#if 0
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
#if ((!defined CONFIG_X2000_V12) && (!defined CONFIG_M300))
		drefcnt.refcnt.rfc = rfc;
#endif
/*		printf("#define DDRC_REFCNT_VALUE_%d		0x%08x\n", div, drefcnt.d32); */
		printf("\t\tcase %d: return 0x%x; \n", div, drefcnt.d32);
		div++;
	} while(rate > 100000000);
	printf("\t\tdefalut :\n");
	printf("\t\tprintf(\"not support \\n\");\n");
	printf("\t}\n");
	printf("}\n");
}
#endif
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

static void ddr_base_params_fill(struct ddr_params *ddr_params, struct ddr_chip_info *chip)
{
	struct ddr_params_common *params = &ddr_params->private_params.ddr_base_params;
	memset(&ddr_params->private_params, 0, sizeof(union private_params));

	params->tRAS 	= chip->DDR_tRAS;
	params->tRP	    = chip->DDR_tRP;
	params->tRCD	= chip->DDR_tRCD;
	params->tRC 	= chip->DDR_tRC;
	params->tWR	    = chip->DDR_tWR;
	params->tRRD	= chip->DDR_tRRD;
	params->tWTR	= chip->DDR_tWTR;
	params->tRFC	= chip->DDR_tRFC;
	params->tXP 	= chip->DDR_tXP;
	params->tCKE	= chip->DDR_tCKE;
	params->tREFI	= chip->DDR_tREFI;
	params->WL	    = chip->DDR_WL;
	params->RL	    = chip->DDR_RL;
}

static void ddr_special_params_fill(struct ddr_params *ddr_params, struct ddr_chip_info *chip)
{

#ifdef CONFIG_DDR_TYPE_LPDDR2
    struct lpddr2_params *params = &ddr_params->private_params.lpddr2_params;
    params->tCKESR    =  chip->DDR_tCKESR;
    params->tXSR      =  chip->DDR_tXSR;
    params->tMOD      =  chip->DDR_tMOD;
    params->tDQSCK    =  chip->DDR_tDQSCK;
    params->tDQSCKMAX =  chip->DDR_tDQSCKMAX;
    params->tRTP      =  chip->DDR_tRTP;
    params->tCCD      =  chip->DDR_tCCD;
    params->tFAW      =  chip->DDR_tFAW;
    params->tMRD      =  chip->DDR_tMRD;


#endif

#ifdef CONFIG_DDR_TYPE_LPDDR3
    struct lpddr3_params *params = &ddr_params->private_params.lpddr3_params;
    params->tCKESR    =  chip->DDR_tCKESR;
    params->tXSR      =  chip->DDR_tXSR;
    params->tMOD      =  chip->DDR_tMOD;
    params->tDQSCK    =  chip->DDR_tDQSCK;
    params->tDQSCKMAX =  chip->DDR_tDQSCKMAX;
    params->tRTP      =  chip->DDR_tRTP;
    params->tCCD      =  chip->DDR_tCCD;
    params->tFAW      =  chip->DDR_tFAW;
    params->tMRD      =  chip->DDR_tMRD;


#endif

#ifdef CONFIG_DDR_TYPE_DDR2
    struct ddr2_params *params = &ddr_params->private_params.ddr2_params;
    params->tCKSRE  =  chip->DDR_tCKSRE;
    params->tRDLAT  =  chip->DDR_tRDLAT;
    params->tWDLAT  =  chip->DDR_tWDLAT;
    params->tRTW    =  chip->DDR_tRTW;
    params->tCKESR  =  chip->DDR_tCKESR;
    params->tXSNR   =  chip->DDR_tXSNR;
    params->tXARD   =  chip->DDR_tXARD;
    params->tXARDS  =  chip->DDR_tXARDS;
    params->tXSRD   =  chip->DDR_tXSRD;
    params->tRTP    =  chip->DDR_tRTP;
    params->tCCD    =  chip->DDR_tCCD;
    params->tFAW    =  chip->DDR_tFAW;
    params->tMRD    =  chip->DDR_tMRD;
#endif


#ifdef CONFIG_DDR_TYPE_DDR3
    struct ddr3_params *params = &ddr_params->private_params.ddr3_params;
    params->tCKESR =    chip->DDR_tCKESR;
    params->tCKSRE =    chip->DDR_tCKSRE;
    params->tXSDLL =    chip->DDR_tXSDLL;
    params->tMOD   =    chip->DDR_tMOD;
    params->tXPDLL =    chip->DDR_tXPDLL;
    params->tRTP   =    chip->DDR_tRTP;
    params->tCCD   =    chip->DDR_tCCD;
    params->tFAW   =    chip->DDR_tFAW;
    params->tMRD   =    chip->DDR_tMRD;
    params->tAL    =    chip->DDR_tAL;
#endif

}




static int ddr_refi_div(int reftck, unsigned *div)
{
	int tmp,factor = 0;
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

    unsigned int rfc;
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
			| DDRC_REFCNT_PREREF_EN
			| DDRC_REFCNT_PREREF_CNT_DEFAULT
			| rfc << DDRC_REFCNT_TRFC_BIT;

	}

#if (defined(CONFIG_X2000_V12))
	ddrc->autosr_cnt = (rfc << 24) | CONFIG_DDR_AUTO_SELF_REFRESH_CNT;
#else
    unsigned int dasr_cnt = 0;
    rfc = ps2cycle_floor(p->private_params.ddr_base_params.tRFC) / 2;
    dasr_cnt = ps2cycle_floor(p->private_params.ddr_base_params.tREFI);
    ddrc->autosr_cnt = rfc << 24 | dasr_cnt;
#endif

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
	ddrc->cfg.b.ROW1 = 0;
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
		_CASE(LPDDR, 3);	/* LPDDR:0b011 */
		_CASE(DDR2, 4);	    /* DDR2:0b100 */
		_CASE(LPDDR2, 5);	/* LPDDR2:0b101 */
		_CASE(LPDDR3, 5);	/* LPDDR3:0b111 Please contact IC department for more information */
		_CASE(DDR3, 6);		/* DDR3:0b110 */
#undef _CASE
	default:
		out_error("don't support the ddr type.!");
		assert(1);
		break;
	}

	/* CTRL */
	ddrc->ctrl = DDRC_CTRL_ACTPD | DDRC_CTRL_PDT_32  | DDRC_CTRL_CKE |
		DDRC_CTRL_PD_CCE | DDRC_CTRL_SR_CCE;

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


void init_ddr_params_common(struct ddr_params *ddr_params, struct ddr_chip_info *chip)
{

	ddr_params->type = chip->type;
	ddr_params->freq = chip->freq;
	ddr_params->cs0 = CONFIG_DDR_CS0;
	ddr_params->cs1 = CONFIG_DDR_CS1;
	ddr_params->dw32 = CONFIG_DDR_DW32;
	ddr_params->bl = chip->DDR_BL;
	ddr_params->cl = chip->DDR_CL;
	ddr_params->cwl = chip->DDR_CWL;
	ddr_params->col = chip->DDR_COL;
	ddr_params->row = chip->DDR_ROW;

	if(ddr_params->cs1) {
		ddr_params->col1 = chip->DDR_COL1;
		ddr_params->row1 = chip->DDR_ROW1;
	} else {
		ddr_params->col1 = 0;
		ddr_params->row1 = 0;
	}
	ddr_params->bank8 = chip->DDR_BANK8;
	ddr_params->size.chip0 = sdram_size(0, ddr_params);
	ddr_params->size.chip1 = sdram_size(1, ddr_params);
}



static void ddrp_config_creator(struct ddrp_reg *ddrp, struct ddr_params *p)
{
	switch (p->type) {
#define _CASE(D, P)				\
		case D:				\
			ddrp->memcfg.b.memsel = P;\
			break
		_CASE(DDR2, 0);
		_CASE(LPDDR2, 1);
		_CASE(DDR3, 2);
		_CASE(LPDDR3, 3);
#undef _CASE
	default:
		break;
	}

	if(p->bl == 4)
		ddrp->memcfg.b.brusel = 0;
	else if(p->bl == 8)
		ddrp->memcfg.b.brusel = 1;
}

#define swap_bytes(buf,b1,b2) do{				\
		unsigned char swap;						\
		swap = buf[b1];							\
		buf[b1] = buf[b2];						\
		buf[b2] = swap;							\
	}while(0)


static void fill_mem_remap(struct ddr_reg_value *reg, struct ddr_params *p)
{
	int address_bits;
	int swap_bits;
	int bank_bits;
	int startA, startB;
	int bit_width;
	unsigned int remap_array[5];
	unsigned char *s;
	int i;
	s = (unsigned char *)remap_array;
	for(i = 0;i < sizeof(remap_array);i++)
		s[i] = i;

	if(p->size.chip1 && (p->size.chip0 != p->size.chip1))
		return;

	bank_bits = p->bank8 == 1 ? 3 : 2;
	bit_width = CONFIG_DDR_DW32 == 1 ? 2 : 1;

	/*
	 * count the chips's address space bits.
	 */
	address_bits =  bit_width + p->col + p->row + bank_bits + (CONFIG_DDR_CS0 + CONFIG_DDR_CS1 - 1);

	/*
	 * count address space bits for swap.
	 */
	swap_bits = bank_bits + (CONFIG_DDR_CS0 + CONFIG_DDR_CS1 - 1);

	startA = bit_width + p->col > 12 ? bit_width + p->col : 12;

	startB = address_bits - swap_bits - startA;
	startA = startA - 12;
    /*
	 * bank and cs swap with row.
	 */
	for(i = startA;i < swap_bits;i++){
		swap_bytes(s,startA + i,startB + i);
		//swap_bytes(s,startA + i,startB + i,startB);
	}

	for(i = 0; i <5; i++)
	{
		reg->REMMAP_ARRAY[i] = remap_array[i];
	}
}

static struct ddr_creator_ops *p_ddr_creator[5] = {NULL};
static int ops_count = 0;
void register_ddr_creator(struct ddr_creator_ops *ops)
{
	if(ops_count < 5){
		p_ddr_creator[ops_count++] = ops;
	}else{
		out_error("Error: DDR CREATEOR cann't register %d, ops_cout: %d\n",ops->type, ops_count);
	}
}

static void fill_reg_value(struct ddr_reg_value *reg, struct ddrc_reg *ddrc, struct ddrp_reg *ddrp, struct ddr_params *p)
{

	reg->h.freq		= p->freq;
	reg->DDRC_CFG_VALUE	= ddrc->cfg.d32;
	reg->DDRC_CTRL_VALUE	= ddrc->ctrl;
	reg->DDRC_DLMR_VALUE	= ddrc->dlmr;
	reg->DDRC_DDLP_VALUE	= ddrc->ddlp;
	reg->DDRC_MMAP0_VALUE	= ddrc->mmap[0];
	reg->DDRC_MMAP1_VALUE	= ddrc->mmap[1];
	reg->DDRC_REFCNT_VALUE	=  ddrc->refcnt;
	reg->DDRC_TIMING1_VALUE	= ddrc->timing1.d32;
	reg->DDRC_TIMING2_VALUE	= ddrc->timing2.d32;
	reg->DDRC_TIMING3_VALUE	= ddrc->timing3.d32;
	reg->DDRC_TIMING4_VALUE	= ddrc->timing4.d32;
	reg->DDRC_TIMING5_VALUE	 =  ddrc->timing5.d32;
	reg->DDRC_AUTOSR_CNT_VALUE =  ddrc->autosr_cnt;
	reg->DDRC_AUTOSR_EN_VALUE =  ddrc->autosr_en;
	reg->DDRC_HREGPRO_VALUE	= ddrc->hregpro;
	reg->DDRC_PREGPRO_VALUE	= ddrc->pregpro;
	reg->DDRC_CGUC0_VALUE	= ddrc->cguc0;
	reg->DDRC_CGUC1_VALUE	= ddrc->cguc1;
	reg->DDRP_MEMCFG_VALUE  = ddrp->memcfg.d32;
	reg->DDRP_CL_VALUE	= ddrp->cl;
	reg->DDRP_CWL_VALUE	= ddrp->cwl;

	reg->DDR_MR0_VALUE	= p->mr0.d32;
	reg->DDR_MR1_VALUE	= p->mr1.d32;
	reg->DDR_MR2_VALUE	= p->mr2.d32;
	reg->DDR_MR3_VALUE	= p->mr3.d32;
	reg->DDR_MR10_VALUE	= p->mr10.d32;
	reg->DDR_MR11_VALUE	= p->mr11.d32;
	reg->DDR_MR63_VALUE	= p->mr63.d32;

	reg->DDR_CHIP_0_SIZE	= p->size.chip0;
	reg->DDR_CHIP_1_SIZE	= p->size.chip1;

	fill_mem_remap(reg, p);
}

int create_one_ddr_params(struct ddr_chip_info *chip, struct ddr_reg_value *reg)
{
	struct ddrc_reg ddrc;
	struct ddrp_reg ddrp;
	struct ddr_params ddr_params;

	struct ddr_creator_ops * ddr_creator = NULL;
	int creator_found = 0;
	int i;

	/* 1. search ddr creator by type. */
	for(i = 0; i < ops_count; i++) {
		ddr_creator = p_ddr_creator[i];

		if(ddr_creator->type == chip->type) {
			creator_found = 1;
			break;
		}
	}
	if(!creator_found) {
		printf("cannot find ddr_creator for ddr type: %d, please_check!\n", chip->type);
		return -1;
	}

	memset(&ddrc, 0, sizeof(struct ddrc_reg));
	memset(&ddrp, 0, sizeof(struct ddrp_reg));
	memset(&ddr_params, 0, sizeof(struct ddr_params));

	init_ddr_params_common(&ddr_params, chip);
	ddr_base_params_fill(&ddr_params, chip);
	ddr_special_params_fill(&ddr_params, chip);
	ddr_creator->fill_in_params(&ddr_params, chip);

	/* DDR controller creator.*/
	ddrc_base_params_creator_common(&ddrc, &ddr_params);
	ddr_creator->ddrc_params_creator(&ddrc, &ddr_params);
	ddrc_config_creator(&ddrc,&ddr_params);

	/* DDR phy creator. */
	ddr_creator->ddrp_params_creator(&ddrp, &ddr_params);
	ddrp_config_creator(&ddrp,&ddr_params);

	/* output */
	reg->h.id = chip->id;
	reg->h.type = chip->type;
	memcpy(reg->h.name, chip->name, sizeof(reg->h.name));

	fill_reg_value(reg, &ddrc, &ddrp, &ddr_params);
    memcpy(&chip->ddrc_value,&ddrc,sizeof(struct ddrc_reg));
    memcpy(&chip->ddrp_value,&ddrp,sizeof(struct ddrp_reg));
	return 0;
}

void dump_generated_reg_define(struct ddr_reg_value *reg)
{
    int i =0;
	printf("#define DDR_NAME		        \"%s\"\n", reg->h.name);
	printf("#define	DDR_ID		           0x%08x\n", reg->h.id);
	printf("#define	DDR_TYPE		       0x%08x\n", reg->h.type);
	printf("#define	DDR_FREQ		       0x%08x\n", reg->h.freq);
	printf("#define	DDRC_CFG_VALUE         0x%08x\n", reg->DDRC_CFG_VALUE);
	printf("#define	DDRC_CTRL_VALUE        0x%08x\n", reg->DDRC_CTRL_VALUE);
	printf("#define	DDRC_DLMR_VALUE        0x%08x\n", reg->DDRC_DLMR_VALUE);
	printf("#define	DDRC_DDLP_VALUE        0x%08x\n", reg->DDRC_DDLP_VALUE);
	printf("#define	DDRC_MMAP0_VALUE       0x%08x\n", reg->DDRC_MMAP0_VALUE);
	printf("#define	DDRC_MMAP1_VALUE       0x%08x\n", reg->DDRC_MMAP1_VALUE);
	printf("#define	DDRC_REFCNT_VALUE      0x%08x\n", reg->DDRC_REFCNT_VALUE);
	printf("#define	DDRC_TIMING1_VALUE     0x%08x\n", reg->DDRC_TIMING1_VALUE);
	printf("#define	DDRC_TIMING2_VALUE     0x%08x\n", reg->DDRC_TIMING2_VALUE);
	printf("#define	DDRC_TIMING3_VALUE     0x%08x\n", reg->DDRC_TIMING3_VALUE);
	printf("#define	DDRC_TIMING4_VALUE     0x%08x\n", reg->DDRC_TIMING4_VALUE);
	printf("#define	DDRC_TIMING5_VALUE     0x%08x\n", reg->DDRC_TIMING5_VALUE);
	printf("#define	DDRC_AUTOSR_CNT_VALUE  0x%08x\n", reg->DDRC_AUTOSR_CNT_VALUE);
	printf("#define	DDRC_AUTOSR_EN_VALUE   0x%08x\n", reg->DDRC_AUTOSR_EN_VALUE);
	printf("#define	DDRC_HREGPRO_VALUE     0x%08x\n", reg->DDRC_HREGPRO_VALUE);
	printf("#define	DDRC_PREGPRO_VALUE     0x%08x\n", reg->DDRC_PREGPRO_VALUE);
	printf("#define	DDRC_CGUC0_VALUE       0x%08x\n", reg->DDRC_CGUC0_VALUE);
	printf("#define	DDRC_CGUC1_VALUE       0x%08x\n", reg->DDRC_CGUC1_VALUE);
	printf("#define	DDRP_MEMCFG_VALUE      0x%08x\n", reg->DDRP_MEMCFG_VALUE);
	printf("#define	DDRP_CL_VALUE          0x%08x\n", reg->DDRP_CL_VALUE);
	printf("#define	DDRP_CWL_VALUE         0x%08x\n", reg->DDRP_CWL_VALUE);
	printf("#define	DDR_MR0_VALUE          0x%08x\n", reg->DDR_MR0_VALUE);
	printf("#define	DDR_MR1_VALUE          0x%08x\n", reg->DDR_MR1_VALUE);
	printf("#define	DDR_MR2_VALUE          0x%08x\n", reg->DDR_MR2_VALUE);
	printf("#define	DDR_MR3_VALUE          0x%08x\n", reg->DDR_MR3_VALUE);
	printf("#define	DDR_MR10_VALUE         0x%08x\n", reg->DDR_MR10_VALUE);
	printf("#define	DDR_MR11_VALUE         0x%08x\n", reg->DDR_MR11_VALUE);
	printf("#define	DDR_MR63_VALUE         0x%08x\n", reg->DDR_MR63_VALUE);
	printf("#define	DDR_CHIP_0_SIZE        0x%08x\n", reg->DDR_CHIP_0_SIZE);
	printf("#define	DDR_CHIP_1_SIZE        0x%08x\n", reg->DDR_CHIP_1_SIZE);

	printf("#define REMMAP_ARRAY {\\\n");
	for(i = 0; i < 5; i++) {
		printf("\t0x%08x,\\\n", reg->REMMAP_ARRAY[i]);
	}
	printf("};\n");

}
void dump_debug_reg_define(struct ddrc_reg *ddrc)
{

}

int main(int argc, char *argv[])
{
	struct ddr_reg_value *generated_reg_values;
	int ddr_nums = 0;
	int i;

	ddr_nums = init_supported_ddr();
#ifdef CONFIG_DDR_TYPE_LPDDR2
	lpddr2_creator_init();
#endif

#ifdef CONFIG_DDR_TYPE_LPDDR3
	lpddr3_creator_init();
#endif

#ifdef CONFIG_DDR_TYPE_DDR2
	ddr2_creator_init();
#endif

#ifdef CONFIG_DDR_TYPE_DDR3
	ddr3_creator_init();
#endif

	generated_reg_values = malloc(ddr_nums * sizeof(struct ddr_reg_value));

	create_supported_ddr_params(generated_reg_values);


	printf("#ifndef __DDR_REG_VALUES_H__\n");
	printf("#define __DDR_REG_VALUES_H__\n");
	printf("#include <asm/ddr_innophy.h>\n");
	//printf("struct ddr_reg_value supported_ddr_reg_values[] = {\n");
	for(i = 0; i < ddr_nums; i++) {
		dump_generated_reg_define(&generated_reg_values[i]);
        dump_supported_ddr();
        dump_timing_reg();
	}
	//printf("};\n");
	printf("#endif\n");

	return 0;
}
