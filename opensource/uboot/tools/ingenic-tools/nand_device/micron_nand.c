#include <stdio.h>
#include "nand_common.h"

#define MICRON_MID			    0x2C
#define MICRON_NAND_DEVICD_COUNT	    1

static unsigned char micron_agd[] = {0x2};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x24, 2048, 2, 4, 3, 1, micron_agd),
};

static struct nand_desc micron_nand = {

	.id_manufactory = MICRON_MID,
	.device_counts = MICRON_NAND_DEVICD_COUNT,
	.device = device,
};

int micron_nand_register_func(void) {
	return nand_register(&micron_nand);
}