#include <stdio.h>
#include "nand_common.h"

#define ESMT_MIDC8_MID			    0xc8
#define ESMT_MIDC8_NAND_DEVICD_COUNT	    1

static unsigned char esmt_midc8_1G41LB[] = {0x2};

static struct device_struct device[1] = {
	DEVICE_STRUCT(0x01, 2048, 2, 4, 2, 1, esmt_midc8_1G41LB),
};

static struct nand_desc esmt_midc8_nand = {

	.id_manufactory = ESMT_MIDC8_MID,
	.device_counts = ESMT_MIDC8_NAND_DEVICD_COUNT,
	.device = device,
};

int esmt_midc8_nand_register_func(void) {
	return nand_register(&esmt_midc8_nand);
}
