#ifndef __DDR3_CONFIG_H
#define __DDR3_CONFIG_H

//#include "ddr.h"
/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * DDR3-1333 info
 */
/* DDR3 paramters */
#define DDR_ROW     14  /* ROW : 12 to 14 row address */
#define DDR_ROW1     14  /* ROW : 12 to 14 row address */
#define DDR_COL	     10  /* COL :  8 to 10 column address */
#define DDR_COL1     10  /* COL :  8 to 10 column address */
#define DDR_BANK8   1 	/* Banks each chip: 0-4bank, 1-8bank */

#define DDR_CL      6   /* dll off */
#define DDR_tCWL    5	/* DDR3 dll off*/

/*
 * DDR3 controller timing1 register
 */
/**
 * Note:!!!!!!!!! DDR_tRP DDR_tRCD DDR_tFAW DDR_tWR  With the change of frequency, the values may need change
 * */
#define DDR_tRAS DDR__ns(36)	/* tRAS: ACTIVE to PRECHARGE command period to the same bank. ns*/
#define DDR_tRP  DDR__ns(12)	/* tRP: PRECHARGE command teriod to the same bank. ns*/
#define DDR_tRCD DDR__ns(12)	/* ACTIVE to READ or WRITE command period to the same bank. ns*/
#define DDR_tRC  DDR__ns(48)	/* ACTIVE to ACTIVE command period to the same bank. ns*/
#define DDR_tWR  DDR__ns(20)	/*The spec minimum is 15 ns,but need >= 5tck,so num is >= 90 ns*/    /* WRITE Recovery Time defined by register MR of DDR2 memory, ns*/
#define DDR_tRRD DDR_SELECT_MAX__tCK_ps(4, 7500) /* ACTIVE bank A to ACTIVE bank B command period. DDR3 - tCK*/
#define DDR_tRTP DDR_SELECT_MAX__tCK_ps(4, 7500) /* READ to PRECHARGE command period. DDR3 spec no. 7.5ns*/
#define DDR_tWTR DDR_SELECT_MAX__tCK_ps(4, 7500) /* WRITE to READ command delay. DDR3 spec no. 7.5 ns*/

/*
 * DDR3 controller timing2 register
 */
#define DDR_tRFC   DDR__ns(160) 	/* AUTO-REFRESH command period. DDR3 - ns*/
#define DDR_tMINSR DDR__ns(49)		/* Minimum Self-Refresh / Deep-Power-Down . DDR3 no*/
#define DDR_tXP    DDR_SELECT_MAX__tCK_ps(3, 6000)	/* DDR3 only: Exit active power down to any valid command, tck*/
#define DDR_tMRD   DDR__tck(4)		/* unit: tCK. Load-Mode-Register to next valid command period: DDR3 rang 4 to 7 tCK. DDR3 spec no */

/* new add */
#define DDR_BL	   8	/* DDR3 Burst length: 0 - 8 burst, 2 - 4 burst , 1 - 4 or 8(on the fly)*/
#define DDR_tCCD   DDR__tck(4)		/* CAS# to CAS# command delay , tCK. 4 or 5 */
#define DDR_tFAW   DDR__ns(38)		/* Four bank activate period, DDR3 - ns */
#define DDR_tCKE   DDR_SELECT_MAX__tCK_ps(3, 7500)	/* CKE minimum pulse width, DDR3 spec no, tCK */
#define DDR_RL	   DDR__tck(DDR_CL)	/* DDR3: Read Latency = tAL + tCL */
#define DDR_WL 	   DDR__tck(DDR_tCWL)	/* DDR3: Write Latency = tAL + tCWL */
#define DDR_tCKSRE 	DDR_SELECT_MAX__tCK_ps(5, 10000) /* Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */
#define DDR_tCKESR	DDR_tCKE + DDR__tck(1)	// DDR_TCKESR = DDR_TCKE + 1

#define DDR_tDLLLOCK	DDR__tck(256)			/* DDR3 only: DLL LOCK, tck */
#define DDR_tXSDLL 	DDR_SELECT_MAX__tCK_ps(8, 0)		/* DDR3 only: EXit self-refresh to command requiring a locked DLL, tck*/
#define DDR_tMOD   	DDR_SELECT_MAX__tCK_ps(12, 15 * 1000)	/* DDR3 only: Mode Register Set Command update delay*/
#define DDR_tXPDLL 	DDR_SELECT_MAX__tCK_ps(10, 24 * 1000)	 /* DDR3 only: Exit active power down to command requirint a locked DLL, ns*/
#define DDR_tXS    	DDR_SELECT_MAX__tCK_ps(5, (DDR_tRFC + 10) * 1000) /* DDR3 only: EXit self-refresh to command not requiring a locked DLL, ns*/
#define DDR_tXSR	DDR__tck(512)//	12		/* DDR2 only: Exit self refresh to a read command, tck */

/*
 * DDR3 controller refcnt register
 */
#define DDR_tREFI   DDR__ns(7800)	/* Refresh period: 64ms / 32768 = 1.95 us , 2 ^ 15 = 32768 */

#endif /* __DDR3_CONFIG_H */
