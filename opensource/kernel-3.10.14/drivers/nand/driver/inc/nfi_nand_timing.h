#ifndef _NFI_NAND_TIMING_H_
#define _NFI_NAND_TIMING_H_
/**
 * struct __nand_timing - NAND Flash Device timing
 **/
typedef struct __nfi_nand_timing {
        unsigned int tWH;	/* ... duration/width/time */
        unsigned int tCH;	/* ... duration/width/time */
        unsigned int tRP;	/* ... duration/width/time */
        unsigned int tWP;	/* ... duration/width/time */
        unsigned int tDH;	/* ... duration/width/time */
        unsigned int tWHR;	/* ... duration/width/time */
	unsigned int tWHR2;	/* ... duration/width/time */
	unsigned int tRR;	/* ... duration/width/time */
	unsigned int tWB;	/* ... duration/width/time */
	unsigned int tADL;	/* ... duration/width/time */
	unsigned int tCWAW;	/* ... duration/width/time */
        unsigned int tCS;	/* ... duration/width/time */
        unsigned int tREH;	/* ... duration/width/time */
} nfi_nand_timing;

typedef struct __onfi_toggle_timing {
        unsigned int tRPRE;	/* ... duration/width/time */
        unsigned int tDS;	/* ... duration/width/time */
        unsigned int tWPRE;	/* ... duration/width/time */
        unsigned int tDQSRE;	/* ... duration/width/time */
        unsigned int tWPST;	/* ... duration/width/time */
} nfi_toggle_timing;

typedef struct __nfi_onfi_timing {
        unsigned int tDQSD;	/* ... duration/width/time */
        unsigned int tDQSS;	/* ... duration/width/time */
        unsigned int tCK;	/* ... duration/width/time */
        unsigned int tDQSHZ;	/* ... duration/width/time */
        unsigned int tDQSCK;	/* ... duration/width/time */
} nfi_onfi_timing;

#endif /* _NFI_NAND_TIMING_H_ */
