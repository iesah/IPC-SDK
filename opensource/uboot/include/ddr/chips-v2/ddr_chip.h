#ifndef __DDR_CHIP_INFO_H__
#define __DDR_CHIP_INFO_H__

#include <ddr/ddrcp_chip/ddrc.h>
#include <ddr/ddrcp_chip/ddrp_inno.h>

#include <ddr/ddr_common.h>
struct ddr_chip_info {
	char name[32];
	unsigned int id;
	unsigned int type;	/*lpddr2/lpddr3...*/
	unsigned int size;	/*MBytes*/
	unsigned int freq;	/*ddr frequency.*/
    struct ddrc_reg ddrc_value;
    struct ddrp_reg ddrp_value;
	void (*init)(void *data);


	unsigned int DDR_ROW;		/* ROW : 12 to 14 row address for chip0 	*/
	unsigned int DDR_ROW1;		/* ROW1 : 12 to 14 row address for chip1 	*/
	unsigned int DDR_COL;		/* COL :  8 to 10 column address for chip0 	*/
	unsigned int DDR_COL1;		/* COL1 :  8 to 10 column address for chip1 	*/
	unsigned int DDR_BANK8;		/* DDR_BANK8: banks each chip: 0-4bank, 1-8bank */
	unsigned int DDR_BL;		/* LPDDR3 Burst length : 3 - 8 burst, 2 - 4 burst , 4 - 16 burst */
	unsigned int DDR_RL;
	unsigned int DDR_WL;


	unsigned int DDR_tMRW;		/* tMRD: Mode Register Delay . max(14nS, 10nCK ) */

	unsigned int DDR_tDQSCK;	/* tDQSCK: QS output access from ck_t/ck_c.*/
	unsigned int DDR_tDQSCKMAX;	/* tDQSCKMAX: MAX DQS output access from ck_t/ck_c.*/

	unsigned int DDR_tRAS;
	unsigned int DDR_tRTP;
	unsigned int DDR_tRP;
	unsigned int DDR_tRCD;
	unsigned int DDR_tRC;
	unsigned int DDR_tRRD;
	unsigned int DDR_tWR;
	unsigned int DDR_tWTR;
	unsigned int DDR_tCCD;
	unsigned int DDR_tFAW;

	unsigned int DDR_tRFC;		/* tRFC: AUTO-REFRESH command period.*/
	unsigned int DDR_tREFI;		/* tREFI: Refresh period: 4096 refresh cycles/64ms , line / 64ms*/
	unsigned int DDR_tCKE;
	unsigned int DDR_tCKESR;
	unsigned int DDR_tXSR;
	unsigned int DDR_tXP;

	/*DDR3 need.*/
	unsigned int DDR_tXSDLL;
	unsigned int DDR_tMOD;
	unsigned int DDR_tXPDLL;
	unsigned int DDR_tCKSRE;
	unsigned int DDR_CL;
	unsigned int DDR_CWL;
	unsigned int DDR_tAL;
    /*DDR2 need*/
    unsigned int DDR_tRDLAT;
    unsigned int DDR_tWDLAT;
    unsigned int DDR_tRTW;
    unsigned int DDR_tXS;
    unsigned int DDR_tCWL;
    unsigned int DDR_tXSNR;
    unsigned int DDR_tXARD;
    unsigned int DDR_tXARDS;
    unsigned int DDR_tXSRD;
    unsigned int DDR_tMRD;
};

enum {
	VENDOR_WINBOND = 0x0,
	VENDOR_ESMT    = 0x1,
};
enum {
	TYPE_LPDDR2 = 0x0,
	TYPE_LPDDR3 = 0x1,
	TYPE_DDR3 = 0x2,
	TYPE_DDR2 = 0x3,
};
enum {
	MEM_256M	= 0x0,
	MEM_128M	= 0x1,
	MEM_64M		= 0x2,
	MEM_32M		= 0x3,
};

#define DDR_CHIP_ID(vendor, type, capacity)	(type << 6 | vendor << 3 | capacity)


#include <asm/ddr_innophy.h>
#endif
