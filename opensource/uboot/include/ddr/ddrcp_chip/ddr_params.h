/*
 * DDR parameters data structure.
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

#ifndef __DDR_PARAMS_H__
#define __DDR_PARAMS_H__

enum ddr_type{
	DDR3,
	LPDDR,
	LPDDR2,
	LPDDR3,
	DDR2,
	VARIABLE,
	UNKOWN,
};

/* ----------------------- */
struct size {
	uint32_t chip0;
	uint32_t chip1;
};

#define ddr_common_params		\
	uint32_t tRAS;				\
	uint32_t tRP;				\
	uint32_t tRCD;				\
	uint32_t tRC;				\
	uint32_t tWR;				\
	uint32_t tRRD;				\
	uint32_t tWTR;				\
	uint32_t tRFC;				\
	uint32_t tXP;				\
	uint32_t tCKE;				\
	uint32_t RL;				\
	uint32_t WL;				\
	uint32_t tREFI;

struct ddr3_params {
	ddr_common_params;
	uint32_t tCKESR;
	uint32_t tCKSRE;
	uint32_t tXSDLL;
	uint32_t tMOD;
	uint32_t tXPDLL;
	uint32_t tXS;
	uint32_t tRTP;
	uint32_t tCCD;
	uint32_t tFAW;
	uint32_t tMRD;
	uint32_t tCL;
	uint32_t tCWL;
	uint32_t tAL;
};

struct ddr2_params {
	ddr_common_params;
	uint32_t tCKSRE;
	uint32_t tRDLAT;
	uint32_t tWDLAT;
	uint32_t tRTW;
	uint32_t tXS;
	uint32_t tCKESR;
	uint32_t tCWL;
	uint32_t tXSNR;
	uint32_t tXARD;
	uint32_t tXARDS;
	uint32_t tXSRD;
	uint32_t tRTP;
	uint32_t tCCD;
	uint32_t tFAW;
	uint32_t tMRD;
};

struct lpddr3_params {
	ddr_common_params;
	uint32_t tCKESR;
	uint32_t tXSR;
	uint32_t tMOD;
	uint32_t tDQSCK;
	uint32_t tDQSCKMAX;
	uint32_t tRTP;
	uint32_t tCCD;
	uint32_t tFAW;
	uint32_t tMRD;
};

struct lpddr2_params {
	ddr_common_params;
	uint32_t tCKESR;
	uint32_t tXSR;
	uint32_t tMOD;
	uint32_t tDQSCK;
	uint32_t tDQSCKMAX;
	uint32_t tRTP;
	uint32_t tCCD;
	uint32_t tFAW;
	uint32_t tMRD;
};

struct lpddr_params {
	ddr_common_params;
	uint32_t tMRD;
	uint32_t tDQSSMAX;
	uint32_t tXSR;

};
struct ddr_params_common
{
	ddr_common_params;
};

union private_params {
	struct ddr_params_common ddr_base_params;
	struct lpddr_params lpddr_params;
	struct ddr2_params  ddr2_params;
	struct ddr3_params ddr3_params;
	struct lpddr2_params lpddr2_params;
	struct lpddr3_params lpddr3_params;
};

union ddr_mr0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned BL:2;
		unsigned CL_2:1;
		unsigned BT:1;
		unsigned CL_4_6:3;
		unsigned TM:1;
		unsigned DR:1;
		unsigned WR:3;
		unsigned PD:1;
		unsigned RSVD13_15:3;
		unsigned BA:2;
		unsigned RSVD_BA:1;
		unsigned reserved19_31:13;
	} ddr3; /* MR0 */
	struct {
		unsigned BL:3;
		unsigned BT:1;
		unsigned CL:3;
		unsigned TM:1;
		unsigned DR:1;
		unsigned WR:3;
		unsigned PD:1;
		unsigned BA:2;
		unsigned reserved19_31:17;
	} ddr2; /* MR */
};

union ddr_mr1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned DE:1;
		unsigned DIC1:1;
		unsigned RTT2:1;
		unsigned AL:2;
		unsigned DIC5:1;
		unsigned RTT6:1;
		unsigned LEVEL:1;
		unsigned RSVD8:1;
		unsigned RTT9:1;
		unsigned RSVD10:1;
		unsigned TDQS:1;
		unsigned QOFF:1;
		unsigned RSVD13_15:3;
		unsigned BA:2;
		unsigned RSVD_BA:1;
		unsigned reserved19_31:13;
	} ddr3; /* MR1 */
	struct {
		unsigned DE:1;
		unsigned DIC1:1;
		unsigned RTT2:1;
		unsigned AL:3;
		unsigned RTT6:1;
		unsigned OCD:3;
		unsigned DQS:1;
		unsigned RDQS:1;
		unsigned QOFF:1;
		unsigned BA:2;
		unsigned reserved15_31:17;
	} ddr2; /* EMR */
	struct {
		unsigned BL:3;
		unsigned BT:1;
		unsigned WC:1;
		unsigned nWR:3;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr2; /* MR1 */
	struct {
		unsigned BL:3;
		unsigned RSVD3_4:2;
		unsigned nWR:3;
		unsigned MA:8; //should 10 bit
		unsigned reserved16_31:16;
	} lpddr3; /* MR1 */
};

union ddr_mr2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned PASR:3;
		unsigned CWL:3;
		unsigned ASR:1;
		unsigned SRT:1;
		unsigned RSVD8:1;
		unsigned RTTWR:2;
		unsigned RSVD11_15:5;
		unsigned BA:2;
		unsigned RSVD_BA:1;
		unsigned reserved19_31:13;
	} ddr3; /* MR2 */
	struct {
	    unsigned PASR:3;
		unsigned DCC:1;
		unsigned RSVD4_6:3;
		unsigned SRF:1;
		unsigned RSVD8_12:5;
		unsigned BA:2;
		unsigned reserved15_31:17;
	} ddr2; /* EMR2 */
	struct {
		unsigned RL_WL:4;
		unsigned RSVD4_7:4;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr2; /* MR2 */
	struct {
		unsigned RL_WL:4;
		unsigned WRE:1;
		unsigned RSVD5:1;
		unsigned WL_S:1;
		unsigned WR_L:1;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr3; /* MR2 */
};

union ddr_mr3 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned MPRLOC:2;
		unsigned MPR:1;
		unsigned RSVD3_15:13;
		unsigned BA:2;
		unsigned RSVD_BA:1;
		unsigned reserved19_31:13;
	} ddr3; /* MR3 */
    struct {
        unsigned RSVD0_12:13;
        unsigned BA:2;
        unsigned reserved15_31:17;
    } ddr2; /* EMR3 */
	struct {
		unsigned DS:4;
		unsigned RSVD4_7:4;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr2; /* MR3 */
	struct {
		unsigned DS:4;
		unsigned RSVD4_7:4;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr3; /* MR2 */
};

union ddr_mr10 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned CAL_CODE:8;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr2; /* MR_RST */
	struct {
		unsigned CAL_CODE:8;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr3; /* MR_RST */
};

union ddr_mr11 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned ODT:2;
		unsigned PD:1;
		unsigned RSVD3_7:5;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr3; /* MR_ODT */
};

union ddr_mr63 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned RSVD0_7:8;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr2; /* MR_IO_CALIBRATION */
	struct {
		unsigned RST:8;
		unsigned MA:8;
		unsigned reserved16_31:16;
	} lpddr3; /* MR_IO_CALIBRATION */
};


struct ddr_params {
	uint32_t type;
	uint32_t freq;
	uint32_t div;
	uint32_t cs0;
	uint32_t cs1;
	uint32_t dw32;
	uint32_t cl;
	uint32_t cwl;
	uint32_t bl;
	uint32_t col;
	uint32_t row;
	uint32_t col1;
	uint32_t row1;
	uint32_t bank8;
	struct size size;
	union private_params private_params;
#ifdef CONFIG_DDR_INNOPHY
	union ddr_mr0 mr0;
	union ddr_mr1 mr1;
	union ddr_mr2 mr2;
	union ddr_mr3 mr3;
	union ddr_mr10 mr10;
	union ddr_mr11 mr11;
	union ddr_mr63 mr63;
#endif
};

#endif /* __DDR_PARAMS_H__ */
