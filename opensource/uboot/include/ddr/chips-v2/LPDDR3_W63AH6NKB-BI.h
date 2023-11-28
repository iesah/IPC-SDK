/*
 * =====================================================================================
 *
 *       Filename:  LPDDR3_W63AH6NKB_BI.h
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

#ifndef __LPDDR3_W63AH6NKB_BI_H__
#define __LPDDR3_W63AH6NKB_BI_H__

static inline void LPDDR3_W63AH6NKB_BI_init(void *data)
{

	struct ddr_chip_info *c = (struct ddr_chip_info *)data;


	c->DDR_ROW  		= 13,
	c->DDR_ROW1 		= 13,
	c->DDR_COL  		= 10,
	c->DDR_COL1 		= 10,
	c->DDR_BANK8 		= 1,
	c->DDR_BL	   	= 8,
/*
0100b: RL = 6 / WL = 3 (≤400 MHz)
0110b: RL = 8 / WL = 4 (≤533 MHz)
0111b: RL = 9 / WL = 5 (≤600 MHz)
1000b: RL = 10 / WL = 6 (≤667 MHz, default)
1001b: RL = 11 / WL = 6 (≤733 MHz)
1010b: RL = 12 / WL = 6 (≤800 MHz)
1100b: RL = 14 / WL = 8 (≤933 MHz)
1110b: RL = 16 / WL = 8 (≤1066 MHz)
*/

	//c->DDR_RL	   	= DDR__tck(12),
	c->DDR_RL	   	= DDR__tck(12),
	c->DDR_WL	   	= DDR__tck(6),

	c->DDR_tMRW  		= DDR_SELECT_MAX__tCK_ps(10, 14 * 1000);
	c->DDR_tDQSCK 		= DDR__ps(2500);
	c->DDR_tDQSCKMAX 	= DDR__ps(5500);
	c->DDR_tRAS  		= DDR_SELECT_MAX__tCK_ps(3, 42 * 1000);
	c->DDR_tRTP  		= DDR_SELECT_MAX__tCK_ps(4, 7500);
	c->DDR_tRP   		= DDR_SELECT_MAX__tCK_ps(3, 21 * 1000);
	c->DDR_tRCD  		= DDR_SELECT_MAX__tCK_ps(3, 18 * 1000);
	c->DDR_tRC   		= c->DDR_tRAS + c->DDR_tRP;
	c->DDR_tRRD  		= DDR_SELECT_MAX__tCK_ps(2, 10 * 1000);
	c->DDR_tWR   		= DDR_SELECT_MAX__tCK_ps(4, 15 * 1000);
	c->DDR_tWTR  		= DDR_SELECT_MAX__tCK_ps(4, 7500);
	c->DDR_tCCD  		= DDR__tck(4);
	c->DDR_tFAW  		= DDR_SELECT_MAX__tCK_ps(8, 50 * 1000);

	c->DDR_tRFC  		= DDR__ns(130);
	c->DDR_tREFI 		= DDR__ns(7800);

	c->DDR_tCKE  		= DDR_SELECT_MAX__tCK_ps(3, 7500);
	c->DDR_tCKESR 		= DDR_SELECT_MAX__tCK_ps(3, 15 * 1000);
	c->DDR_tXSR  		= DDR_SELECT_MAX__tCK_ps(2, (c->DDR_tRFC + 10 * 1000));
	c->DDR_tXP  		= DDR_SELECT_MAX__tCK_ps(3, 7500);
}


#ifndef CONFIG_LPDDR3_W63AH6NKB_BI_MEM_FREQ
#define CONFIG_LPDDR3_W63AH6NKB_BI_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#define LPDDR3_W63AH6NKB_BI {						\
	.name 	= "W63AH6NKB_BI",					\
	.id	= DDR_CHIP_ID(VENDOR_WINBOND, TYPE_LPDDR3, MEM_128M),	\
	.type	= LPDDR3,						\
	.size	= 128,							\
	.freq	= CONFIG_LPDDR3_W63AH6NKB_BI_MEM_FREQ,			\
	.init	= LPDDR3_W63AH6NKB_BI_init,				\
}


#endif
