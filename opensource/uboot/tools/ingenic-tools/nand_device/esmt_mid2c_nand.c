#include <stdio.h>
#include "nand_common.h"

#define ESMT_MID2C_MID			    0x2C
#define ESMT_MID2C_NAND_DEVICD_COUNT	    1

static unsigned char esmt_mid2c_24w[] = {0x2};

static struct device_struct device[1] = {
	DEVICE_STRUCT(0x24, 2048, 2, 4, 3, 1, esmt_mid2c_24w),
};

static struct nand_desc esmt_mid2c_nand = {

	.id_manufactory = ESMT_MID2C_MID,
	.device_counts = ESMT_MID2C_NAND_DEVICD_COUNT,
	.device = device,
};

int esmt_mid2c_nand_register_func(void) {
	return nand_register(&esmt_mid2c_nand);
}
