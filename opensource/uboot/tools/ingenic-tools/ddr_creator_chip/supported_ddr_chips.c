#include <config.h>
#include <stdio.h>
#include "ddr_params_creator.h"


struct ddr_chip_info supported_ddr_chips[] = {
#ifdef CONFIG_LPDDR2_W97BV6MK
	LPDDR2_W97BV6MK,
#endif
#ifdef CONFIG_LPDDR3_W63AH6NKB_BI
	LPDDR3_W63AH6NKB_BI,
#endif
#ifdef CONFIG_LPDDR2_M54D5121632A
	LPDDR2_M54D5121632A,
#endif
#ifdef CONFIG_DDR3_W631GU6NG
	DDR3_W631GU6NG,
#endif
#ifdef CONFIG_DDR2_M14D5121632A
    DDR2_M14D5121632A,
#endif
#ifdef CONFIG_LPDDR3_W63CH2MBVABE
       LPDDR3_W63CH2MBVABE,
#endif
#ifdef CONFIG_LPDDR3_MT52L256M32D1PF
       LPDDR3_MT52L256M32D1PF
#endif
};


void dump_ddr_info(struct ddr_chip_info *c)
{
    printf("/** Only DDR test\n");
	printf("name 		= %s\n", c->name);
	printf("id 		= %x\n", c->id);
	printf("type 		= %x\n", c->type);
	printf("size 		= %d\n", c->size);
	printf("DDR_ROW 	= %d\n", c->DDR_ROW);
	printf("DDR_ROW1 	= %d\n", c->DDR_ROW1);
	printf("DDR_COL 	= %d\n", c->DDR_COL);
	printf("DDR_COL1 	= %d\n", c->DDR_COL1);
	printf("DDR_BANK8 	= %d\n", c->DDR_BANK8);
	printf("DDR_BL 		= %d\n", c->DDR_BL);
	printf("DDR_RL 		= %d\n", c->DDR_RL);
	printf("DDR_WL 		= %d\n", c->DDR_WL);

	printf("DDR_tMRW 	= %d\n", c->DDR_tMRW);
	printf("DDR_tDQSCK 	= %d\n", c->DDR_tDQSCK);
	printf("DDR_tDQSCKMAX 	= %d\n", c->DDR_tDQSCKMAX);

	printf("DDR_tRAS 	= %d\n", c->DDR_tRAS);
	printf("DDR_tRTP 	= %d\n", c->DDR_tRTP);
	printf("DDR_tRP 	= %d\n", c->DDR_tRP);
	printf("DDR_tRCD 	= %d\n", c->DDR_tRCD);
	printf("DDR_tRC 	= %d\n", c->DDR_tRC);
	printf("DDR_tRRD 	= %d\n", c->DDR_tRRD);
	printf("DDR_tWR 	= %d\n", c->DDR_tWR);
	printf("DDR_tWTR 	= %d\n", c->DDR_tWTR);
	printf("DDR_tCCD 	= %d\n", c->DDR_tCCD);
	printf("DDR_tFAW 	= %d\n", c->DDR_tFAW);

	printf("DDR_tRFC 	= %d\n", c->DDR_tRFC);
	printf("DDR_tREFI 	= %d\n", c->DDR_tREFI);
	printf("DDR_tCKE 	= %d\n", c->DDR_tCKE);
	printf("DDR_tCKESR 	= %d\n", c->DDR_tCKESR);
	printf("DDR_tXSR 	= %d\n", c->DDR_tXSR);
	printf("DDR_tXP 	= %d\n", c->DDR_tXP);
    printf("**/ \n");
}

void dump_timing_reg(void)
{
	struct ddr_chip_info *chip = NULL;

    chip = &supported_ddr_chips[0];

    printf("/** DDR timing\n");
    printf("#define timing1_tWL     %d\n", chip->ddrc_value.timing1.b.tWL);
    printf("#define timing1_tWR     %d\n", chip->ddrc_value.timing1.b.tWR);
    printf("#define timing1_tWTR    %d\n", chip->ddrc_value.timing1.b.tWTR);
    printf("#define timing1_tWDLAT  %d\n", chip->ddrc_value.timing1.b.tWDLAT);

    printf("#define timing2_tRL     %d\n", chip->ddrc_value.timing2.b.tRL);
    printf("#define timing2_tRTP    %d\n", chip->ddrc_value.timing2.b.tRTP);
    printf("#define timing2_tRTW    %d\n", chip->ddrc_value.timing2.b.tRTW);
    printf("#define timing2_tRDLAT  %d\n", chip->ddrc_value.timing2.b.tRDLAT);

    printf("#define timing3_tRP     %d\n", chip->ddrc_value.timing3.b.tRP);
    printf("#define timing3_tCCD    %d\n", chip->ddrc_value.timing3.b.tCCD);
    printf("#define timing3_tRCD    %d\n", chip->ddrc_value.timing3.b.tRCD);
    printf("#define timing3_ttEXTRW %d\n", chip->ddrc_value.timing3.b.tEXTRW);

    printf("#define timing4_tRRD    %d\n", chip->ddrc_value.timing4.b.tRRD);
    printf("#define timing4_tRAS    %d\n", chip->ddrc_value.timing4.b.tRAS);
    printf("#define timing4_tRC     %d\n", chip->ddrc_value.timing4.b.tRC);
    printf("#define timing4_tFAW    %d\n", chip->ddrc_value.timing4.b.tFAW);

    printf("#define timing5_tCKE    %d\n", chip->ddrc_value.timing5.b.tCKE);
    printf("#define timing5_tXP     %d\n", chip->ddrc_value.timing5.b.tXP);
    printf("#define timing5_tCKSRE  %d\n", chip->ddrc_value.timing5.b.tCKSRE);
    printf("#define timing5_tCKESR  %d\n", chip->ddrc_value.timing5.b.tCKESR);
    printf("#define timing5_tXS     %d\n", chip->ddrc_value.timing5.b.tXS);

    printf("#define __ps_per_tck     %d\n", __ps_per_tck);
    printf("**/ \n");
}


int init_supported_ddr(void)
{

	return sizeof(supported_ddr_chips)/ sizeof(struct ddr_chip_info);
}

void dump_supported_ddr(void)
{
	struct ddr_chip_info *c = NULL;
	int i;

	for(i = 0; i < sizeof(supported_ddr_chips) / sizeof(struct ddr_chip_info); i++) {
		c = &supported_ddr_chips[i];
		dump_ddr_info(c);
	}
}

void create_supported_ddr_params(struct ddr_reg_value *reg_values)
{
	struct ddr_chip_info *chip;
	struct ddr_reg_value *reg;

	int i;

	for(i = 0; i < sizeof(supported_ddr_chips) / sizeof(struct ddr_chip_info); i++) {
		chip = &supported_ddr_chips[i];
		reg = &reg_values[i];

		__ps_per_tck = (1000000000 / (chip->freq / 1000));

		chip->init(chip);
		create_one_ddr_params(chip, reg);
	}
}
