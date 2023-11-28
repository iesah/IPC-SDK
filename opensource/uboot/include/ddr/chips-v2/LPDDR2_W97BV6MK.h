/*
 * =====================================================================================
 *
 *       Filename:  LPDDR2_W97BV6MK.h
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

#ifndef __LPDDR2_W97BV6MK_H__
#define __LPDDR2_W97BV6MK_H__

static inline void LPDDR2_W97BV6MK_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;


	c->DDR_ROW  		= 14,
	c->DDR_ROW1 		= 14,
	c->DDR_COL  		= 10,
	c->DDR_COL1 		= 10,
	c->DDR_BANK8 		= 1,
	c->DDR_BL	   	= 8,
	c->DDR_RL	   	= -1,
	c->DDR_WL	   	= -1,

	c->DDR_tMRW  		= DDR__tck(5);
	c->DDR_tDQSCK 		= DDR__ps(2000);
	c->DDR_tDQSCKMAX 	= DDR__ps(10000);
	c->DDR_tRAS  		= DDR_SELECT_MAX__tCK_ps(3, 42 * 1000);
	c->DDR_tRTP  		= DDR_SELECT_MAX__tCK_ps(2, 7500);
	c->DDR_tRP   		= DDR_SELECT_MAX__tCK_ps(3, 21 * 1000);
	c->DDR_tRCD  		= DDR_SELECT_MAX__tCK_ps(3, 18 * 1000);
	c->DDR_tRC   		= c->DDR_tRAS + c->DDR_tRP;
	c->DDR_tRRD  		= DDR_SELECT_MAX__tCK_ps(2, 10 * 1000);
	c->DDR_tWR   		= DDR_SELECT_MAX__tCK_ps(3, 15 * 1000);
	c->DDR_tWTR  		= DDR_SELECT_MAX__tCK_ps(2, 7500);
	c->DDR_tCCD  		= DDR__tck(2);
	c->DDR_tFAW  		= DDR_SELECT_MAX__tCK_ps(8, 50 * 1000);

	c->DDR_tRFC  		= DDR__ns(130);
	c->DDR_tREFI 		= DDR__ns(3900);

	c->DDR_tCKE  		= DDR__tck(3);
	c->DDR_tCKESR 		= DDR_SELECT_MAX__tCK_ps(3, DDR__ns(150));
	c->DDR_tXSR  		= DDR_SELECT_MAX__tCK_ps(2, c->DDR_tRFC + DDR__ns(10));
	c->DDR_tXP  		= DDR_SELECT_MAX__tCK_ps(2, 7500);
}


#ifndef CONFIG_LPDDR2_W97BV6MK_MEM_FREQ
#define CONFIG_LPDDR2_W97BV6MK_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#define LPDDR2_W97BV6MK {					\
	.name 	= "W97BV6MK",					\
	.id	= DDR_CHIP_ID(VENDOR_WINBOND, TYPE_LPDDR2, MEM_256M),	\
	.type	= LPDDR2,						\
	.freq	= CONFIG_LPDDR2_W97BV6MK_MEM_FREQ,			\
	.size	= 256,						\
	.init	= LPDDR2_W97BV6MK_init,				\
}


#endif
