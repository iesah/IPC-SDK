/*
 * =====================================================================================
 *
 *       Filename:  DDR3_W631GU6NG.h
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

#ifndef __DDR3_W631GU6NG_H__
#define __DDR3_W631GU6NG_H__



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

#if ((CONFIG_SYS_MEM_FREQ > 300000000) && (CONFIG_SYS_MEM_FREQ < 330000000))
#define CONFIG_DDR_CL	5
#define CONFIG_DDR_CWL	5
#elif((CONFIG_SYS_MEM_FREQ > 330000000) && (CONFIG_SYS_MEM_FREQ < 400000000))
#define CONFIG_DDR_CL	6
#define CONFIG_DDR_CWL	5
#elif((CONFIG_SYS_MEM_FREQ >= 400000000) && (CONFIG_SYS_MEM_FREQ < 533000000))
#define CONFIG_DDR_CL	8
#define CONFIG_DDR_CWL	6
#elif((CONFIG_SYS_MEM_FREQ > 533000000) && (CONFIG_SYS_MEM_FREQ < 666000000))
#define CONFIG_DDR_CL	10
#define CONFIG_DDR_CWL	7
#elif((CONFIG_SYS_MEM_FREQ >= 800000000) && (CONFIG_SYS_MEM_FREQ < 934000000))
#define CONFIG_DDR_CL	13
#define CONFIG_DDR_CWL	9
#elif((CONFIG_SYS_MEM_FREQ >= 700000000) && (CONFIG_SYS_MEM_FREQ < 800000000))
#define CONFIG_DDR_CL	10
#define CONFIG_DDR_CWL	8
#else
#define CONFIG_DDR_CL	0
#define CONFIG_DDR_CWL	0
#endif

static inline void DDR3_W631GU6NG_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;


	c->DDR_ROW  		= 13,
	c->DDR_ROW1 		= 13,
	c->DDR_COL  		= 10,
	c->DDR_COL1 		= 10,
	c->DDR_BANK8 		= 1,
	c->DDR_BL	   	= 8,
	c->DDR_CL	   	= CONFIG_DDR_CL,
	c->DDR_CWL	   	= CONFIG_DDR_CWL,

	c->DDR_RL	   	= DDR__tck(c->DDR_CL),
	c->DDR_WL	   	= DDR__tck(c->DDR_CWL),

	c->DDR_tRAS  		= DDR__ns(34);
	c->DDR_tRTP  		= DDR_SELECT_MAX__tCK_ps(4, 7500);
	c->DDR_tRP   		= DDR__ns(14);
	c->DDR_tRCD  		= DDR__ns(14);
	c->DDR_tRC   		= c->DDR_tRAS + c->DDR_tRP;
	c->DDR_tRRD  		= DDR_SELECT_MAX__tCK_ps(4, 6000);
	c->DDR_tWR   		= DDR__ns(15);
	c->DDR_tWTR  		= DDR_SELECT_MAX__tCK_ps(4, 7500);
	c->DDR_tCCD  		= DDR__tck(4);
	c->DDR_tFAW  		= DDR__ns(35);

	c->DDR_tRFC  		= DDR__ns(110);
	c->DDR_tREFI 		= DDR__ns(7800);

	c->DDR_tCKE  		= DDR_SELECT_MAX__tCK_ps(3, 5000);
	c->DDR_tCKESR 		= c->DDR_tCKE + DDR__tck(1);
	c->DDR_tXP  		= DDR_SELECT_MAX__tCK_ps(3, 6000);
}


#ifndef CONFIG_DDR3_W631GU6NG_MEM_FREQ
#define CONFIG_DDR3_W631GU6NG_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#define DDR3_W631GU6NG {					\
	.name 	= "W631GU6NG",					\
	.id	= DDR_CHIP_ID(VENDOR_WINBOND, TYPE_DDR3, MEM_128M),	\
	.type	= DDR3,						\
	.freq	= CONFIG_DDR3_W631GU6NG_MEM_FREQ,			\
	.size	= 128,						\
	.init	= DDR3_W631GU6NG_init,				\
}


#endif
