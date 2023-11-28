#ifndef __LPDDR3_CONFIG_H
#define __LPDDR3_CONFIG_H

/*
 * This file contains the memory configuration parameters
 */

// LPDDR3 paramters
#define DDR_ROW 	13 /* ROW : 12 to 14 row address */
#define DDR_ROW1 	13 /* ROW : 12 to 14 row address */
#define DDR_COL 	10  /* COL :  8 to 10 column address */
#define DDR_COL1 	10  /* COL :  8 to 10 column address */
#define DDR_BANK8 	1  /* Banks each chip: 0-4bank, 1-8bank 0 for falcon fpga, 1 for develop board */
#define DDR_BL	 	8   /* LPDDR3 Burst length : 3 - 8 burst, 2 - 4 burst , 4 - 16 burst */


#define DDR_tMRD  	DDR_SELECT_MAX__tCK_ps(10, 14 * 1000)

/*
 * refresh parameters
 */
#define DDR_tRFC 	DDR__ns(130) /* AUTO-REFRESH command period. */
//#define DDR_tREFI	DDR__ns(3900)	/* Refresh period: 4096 refresh cycles/64ms , line / 64ms */
#define DDR_tREFI	DDR__ns(7800)	/* Refresh period: 4096 refresh cycles/64ms , line / 64ms */
/*
 * read parameters
 */
#define DDR_tDQSCK      DDR__ps(2500)/* LPDDR2 only: DQS output access from ck_t/ck_c. */
#define DDR_tDQSCKMAX   DDR__ps(5500)/* LPDDR2 only: MAX DQS output access from ck_t/ck_c. */
/*
 * CKE input parameters
 */
#define DDR_tCKE	DDR_SELECT_MAX__tCK_ps(3, 7500) /* CKE minimum pulse width, tCK */
/*
 * core parameters
 */
#define DDR_RL  	DDR__tck(6)  /* LPDDR3 Read Latency : - 3 6 8 9 ..., tck */
#define DDR_WL		DDR__tck(3)  /* LPDDR3 Write Latency : - 1 3 4 5 ..., tck */
#define DDR_tRAS 	DDR_SELECT_MAX__tCK_ps(3, 42 * 1000)/* ACTIVE to PRECHARGE command period to the same bank. */
#define DDR_tRP 	DDR_SELECT_MAX__tCK_ps(3, 21 * 1000)/* tRP: PRECHARGE command period to the same bank */
#define DDR_tRC 	(DDR_tRAS + DDR_tRP)/* ACTIVE to ACTIVE command period to the same bank. */
#define DDR_tCKESR      DDR_SELECT_MAX__tCK_ps(3, 15 * 1000)/* LPDDR2 no: Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */
#define DDR_tXSR 	DDR_SELECT_MAX__tCK_ps(2, (DDR_tRFC + 10 * 1000))/* EXIT-POWER-DOWN to next valid command period. */
#define DDR_tXP 	DDR_SELECT_MAX__tCK_ps(3, 7500)/* EXIT-POWER-DOWN to next valid command period. */
#define DDR_tCCD 	DDR__tck(4) /* CAS# to CAS# command delay , tCK */
#define DDR_tRTP 	DDR_SELECT_MAX__tCK_ps(4, 7500)/* READ to PRECHARGE command period. */
#define DDR_tRCD 	DDR_SELECT_MAX__tCK_ps(3, 18 * 1000)/* ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tWR 	DDR_SELECT_MAX__tCK_ps(4, 15 * 1000)/* WRITE Recovery Time defined by register MR of DDR2 memory. */
#define DDR_tWTR 	DDR_SELECT_MAX__tCK_ps(4, 7500)/* WRITE to READ command delay. */
#define DDR_tRRD 	DDR_SELECT_MAX__tCK_ps(2, 10 * 1000)/* ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tFAW 	DDR_SELECT_MAX__tCK_ps(8, 50 * 1000)/* Four bank activate period. */

#endif /* __LPDDR3_CONFIG_H */
