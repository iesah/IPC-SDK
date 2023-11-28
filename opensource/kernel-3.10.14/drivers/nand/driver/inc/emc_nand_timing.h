#ifndef _EMC_NAND_TIMING_H_
#define _EMC_NAND_TIMING_H_
/**
 * struct __nand_timing - NAND Flash Device timing
 **/
typedef struct __emc_nand_timing {
        unsigned int tALS;	/* ... duration/width/time */
        unsigned int tALH;	/* ... duration/width/time */
        unsigned int tRP;	/* ... duration/width/time */
        unsigned int tWP;	/* ... duration/width/time */
        unsigned int tRHW;	/* ... duration/width/time */
        unsigned int tWHR;	/* ... duration/width/time */
	unsigned int tWHR2;	/* ... duration/width/time */
	unsigned int tRR;	/* ... duration/width/time */
	unsigned int tWB;	/* ... duration/width/time */
	unsigned int tADL;	/* ... duration/width/time */
	unsigned int tCWAW;	/* ... duration/width/time */
        unsigned int tCS;	/* ... duration/width/time */
        unsigned int tCLH;	/* ... duration/width/time */
} emc_nand_timing;

typedef struct __emc_toggle_timing {
        unsigned int tRPRE;	/* ... duration/width/time */
        unsigned int tDS;	/* ... duration/width/time */
        unsigned int tWPRE;	/* ... duration/width/time */
        unsigned int tDQSRE;	/* ... duration/width/time */
        unsigned int tWPST;	/* ... duration/width/time */
} emc_toggle_timing;


#endif /* _EMC_NAND_TIMING_H_ */
