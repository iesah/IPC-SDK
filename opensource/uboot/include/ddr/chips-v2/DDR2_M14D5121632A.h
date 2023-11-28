/*
 * =====================================================================================
 *
 *       Filename:  DDR2_W631GU6NG.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2020年09月21日 17时55分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef __DDR2_M14D5121632A_H__
#define __DDR2_M14D5121632A_H__



/*
 * CL:5, CWL:5  300M ~ 330M
 * CL:6, CWL:5	300M ~ 400M
 * CL:7, CWL:6
 * CL:8, CWL:6	400M ~ 533M
 * CL:9, CWL:7
 * CL:10, CWL:7 533M ~ 666M
 * CL:11, CWL:8
 * CL:13, CWL:9 800M ~ 933M
 * CL:14, CWL:10
 *
 * */

#define CONFIG_DDR_CL	7
#define CONFIG_DDR_CWL	6

static inline void DDR3_M14D5121632A_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;


	c->DDR_ROW  		= 13,
	c->DDR_ROW1 		= 13,
	c->DDR_COL  		= 10,
	c->DDR_COL1 		= 10,
	c->DDR_BANK8 		= 0,
	c->DDR_BL	   	= 8,
	c->DDR_CL	   	= CONFIG_DDR_CL,
	c->DDR_CWL	   	= CONFIG_DDR_CWL,

	c->DDR_RL	   	= DDR__tck(7),
	c->DDR_WL	   	= DDR__tck(6),

	c->DDR_tRAS  		= DDR__ns(45);
	c->DDR_tRTP  		= DDR__ps(7500);
	c->DDR_tRP   		= DDR__ns(15);
	c->DDR_tRCD  		= DDR__ns(15);
	c->DDR_tRC   		= DDR__ps(58125);
	c->DDR_tRRD  		= DDR__ns(10);
	c->DDR_tWR   		= DDR__ns(15);
	c->DDR_tWTR  		= DDR__tck(6) + DDR__tck(2) + DDR__ns(8);
	c->DDR_tCCD  		= DDR__tck(2);
	c->DDR_tFAW  		= DDR__ns(60);

	c->DDR_tRFC  		= DDR__ns(130);
	c->DDR_tREFI 		= DDR__ns(7800);

	c->DDR_tCKE  		= DDR__tck(5);
	c->DDR_tCKESR 		= DDR__tck(0);
	c->DDR_tXP  		= DDR__tck(5);

    c->DDR_tWDLAT       = c->DDR_WL - DDR__tck(1);
    c->DDR_tRTW         = (c->DDR_RL - c->DDR_WL + DDR__tck(5));
    c->DDR_tRDLAT       = (c->DDR_RL - DDR__tck(3));
    c->DDR_tXSNR        = (c->DDR_tRFC + DDR__ns(10));
}


#ifndef CONFIG_DDR2_M14D5121632A_MEM_FREQ
#define CONFIG_DDR2_M14D5121632A_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#define DDR2_M14D5121632A {					\
	.name 	= "M14D5121632A",					\
	.id	= DDR_CHIP_ID(VENDOR_WINBOND, TYPE_DDR2, MEM_64M),	\
	.type	= DDR2,						\
	.freq	= CONFIG_DDR2_M14D5121632A_MEM_FREQ,			\
	.size	= 64,						\
	.init	= DDR3_M14D5121632A_init,				\
}


#endif
