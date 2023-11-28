#include <stdio.h>
#include "nand_common.h"

#define XTX_MID0B_MID			    0x0b
#define XTX_MID0B_NAND_DEVICD_COUNT	    3

static unsigned char xtx_mid0b_xaw[] = {0xf};

static struct device_struct device[XTX_MID0B_NAND_DEVICD_COUNT] = {
	DEVICE_STRUCT(0x11, 2048, 2, 4, 4, 1, xtx_mid0b_xaw),
	DEVICE_STRUCT(0x12, 2048, 2, 4, 4, 1, xtx_mid0b_xaw),
	DEVICE_STRUCT(0x15, 2048, 2, 4, 4, 1, xtx_mid0b_xaw),
};

static struct nand_desc xtx_mid0b_nand = {

	.id_manufactory = XTX_MID0B_MID,
	.device_counts = XTX_MID0B_NAND_DEVICD_COUNT,
	.device = device,
};

int xtx_mid0b_nand_register_func(void) {
	return nand_register(&xtx_mid0b_nand);
}
