#ifndef __DDR_CONFIG_H__
#define __DDR_CONFIG_H__

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * DDR3-1333 info
 */
/* DDR3 paramters */
#define DDR_ROW     15  /* ROW : 12 to 18 row address ,1G only 512MB*/
#define DDR_ROW1     15  /* ROW : 12 to 18 row address ,1G only 512MB*/
#define DDR_COL     10  /* COL :  8 to 14 column address */
#define DDR_COL1     10  /* COL :  8 to 14 column address */
#define DDR_BANK8   1 	/* Banks each chip: 0-4bank, 1-8bank */

#ifdef CONFIG_SYS_DDR_DLL_OFF
#define DDR_CL      6   /* dll off */
#define DDR_tCWL    6	/* DDR3 dll off*/
#else
#define DDR_CL      10   /* CAS latency: 5 to 14 ,tCK*/
#define DDR_tCWL   (DDR_CL - 1)	/* DDR3 only: CAS Write Latency, 5 to 8 */
#endif

/*
 * DDR3 controller timing1 register
 */
#define DDR_tRAS DDR__ns(38)  /* tRAS: ACTIVE to PRECHARGE command period to the same bank. ns*/
#define DDR_tRP  DDR__ns(14)  /* tRP: PRECHARGE command period to the same bank. ns*/
#define DDR_tRCD DDR__ns(14)  /* ACTIVE to READ or WRITE command period to the same bank. ns*/
#define DDR_tRC  DDR__ns(51)  /* ACTIVE to ACTIVE command period to the same bank. ns*/
#define DDR_tWR  DDR__ns(15)  /*FIXME WRITE Recovery Time defined by register MR of DDR2 memory, ns*/
#define DDR_tRRD MAX(DDR__tck(4), DDR__ps(7500)) /* FIXME ACTIVE bank A to ACTIVE bank B command period. DDR3 - tCK*/
#define DDR_tRTP MAX(DDR__tck(4), DDR__ps(7500)) /* FIXME READ to PRECHARGE command period. DDR3 spec no. 7.5ns*/
#define DDR_tWTR MAX(DDR__tck(4), DDR__ps(7500)) /* FIXME WRITE to READ command delay. DDR3 spec no. 7.5 ns*/

/*
 * DDR3 controller timing2 register
 */
#define DDR_tRFC   DDR__ns(160) 	/* AUTO-REFRESH command period. DDR3 - ns*/
//#define DDR_tMINSR 60   /*FIXME Minimum Self-Refresh / Deep-Power-Down . DDR3 no*/
#define DDR_tXP    DDR__tck(4)	/*FIXME DDR3 only: Exit active power down to any valid command, ns*/
#define DDR_tMRD   DDR__tck(4)    /*FIXME unit: tCK. Load-Mode-Register to next valid command period: DDR3 rang 4 to 7 tCK. DDR3 spec no */

/* new add */
#define DDR_BL	   8   /* DDR3 Burst length: 0 - 8 burst, 2 - 4 burst , 1 - 4 or 8(on the fly)*/
#define DDR_RL    DDR__tck(DDR_CL) /* RL=AL+CL */
#define DDR_WL    DDR__tck(DDR_tCWL) /* WL=AL+CWL */
#define DDR_tAL    DDR__tck(0)	/* Additive Latency, tCK*/
#define DDR_tCCD   DDR__tck(4)	/* CAS# to CAS# command delay , tCK. 4 or 5 */
#define DDR_tFAW   DDR__ns(45)	/* Four bank activate period, DDR3 - ns */
#define DDR_tCKE   	MAX(DDR__tck(3), DDR__ps(5000))	/* CKE minimum pulse width, DDR3 spec no, tCK */
#define DDR_tCKESR	(DDR_tCKE + 1)      /* CKE minimum pulse width, tCK */
#define DDR_tRL 	(DDR_tAL + DDR_CL)	/* DDR3: Read Latency = tAL + tCL */
#define DDR_tWL 	(DDR_tAL + DDR_tCWL)	/* DDR3: Write Latency = tAL + tCWL */
#define DDR_tRDLAT	(DDR_tRL - 3)
#define DDR_tWDLAT	(DDR_tWL - 1)
#define DDR_tRTW 	(DDR_tRL + DDR_tCCD + 2 - DDR_tWL)	/* Read to Write delay */
#define DDR_tCKSRE 	MAX(DDR__tck(5), DDR__ns(10)) /* Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */

#define DDR_tDLLLOCK	DDR__tck(512)		/* DDR3 only: DLL LOCK, tck */
#define DDR_tXSDLL 	MAX(DDR_tDLLLOCK, 0)		/* DDR3 only: EXit self-refresh to command requiring a locked DLL, tck*/
#define DDR_tMOD   	MAX(DDR__tck(12), DDR__ns(15))	/* DDR3 only: Mode Register Set Command update delay*/
#define DDR_tXPDLL 	MAX(DDR__tck(10), DDR__ns(24))	 /* DDR3 only: Exit active power down to command requirint a locked DLL, ns*/
#define DDR_tXS    	MAX(DDR__tck(5), (DDR_tRFC + DDR__ns(10))) /* DDR3 only: EXit self-refresh to command not requiring a locked DLL, ns*/
#define DDR_tXSRD  	/*100*/10		/* DDR2 only: Exit self refresh to a read command, tck */

#define DDR_tXSR	DDR__tck(512)//	12		/* DDR2 only: Exit self refresh to a read command, tck */

/*
 * DDR3 controller refcnt register
 */
#define DDR_tREFI   DDR__ns(7800)	/* Refresh period: 64ms / 32768 = 1.95 us , 2 ^ 15 = 32768 */
#define DDR_CLK_DIV 1    	/* Clock Divider. auto refresh
			  *	cnt_clk = memclk/(16*(2^DDR_CLK_DIV))
			  */

#endif /* __DDR_CONFIG_H__ */
