#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include "../spinand.h"
#include "../ingenic_sfc_common.h"
#include "nand_common.h"

#define WB_DEVICES_NUM         3
#define TSETUP		5
#define THOLD		3
#define	TSHSL_R		10
#define	TSHSL_W		50

#define TRD		60
#define TPP		700
#define TBE		10

static struct ingenic_sfcnand_device *wb_nand;

static struct ingenic_sfcnand_base_param wb_param[WB_DEVICES_NUM] = {

	[0] = {
		/* W25N01GV */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  =TSETUP,
        .tHOLD   =THOLD,
        .tSHSL_R =TSHSL_R,
        .tSHSL_W =TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},
	[1] = {
		/* W25N01KV */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  =TSETUP,
        .tHOLD   =THOLD,
        .tSHSL_R =TSHSL_R,
        .tSHSL_W =TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},
	[2] = {
		/* W25N01GW */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  =TSETUP,
        .tHOLD   =THOLD,
        .tSHSL_R =TSHSL_R,
        .tSHSL_W =TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},
};

static struct device_id_struct device_id[WB_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xAA, "W25N01GV", &wb_param[0]),//AA21
	DEVICE_ID_STRUCT(0xAE, "W25N01KV", &wb_param[1]),//AE21
	DEVICE_ID_STRUCT(0xBA, "W25N01GW", &wb_param[2]),//BA21
};


static cdt_params_t *wb_get_cdt_params(struct sfc_flash *flash, uint8_t device_id)
{
	CDT_PARAMS_INIT(wb_nand->cdt_params);

	switch(device_id) {
		case 0xAA:
		case 0xAE:
		case 0xBA:
			break;
		default:
		    dev_err(flash->dev, "device_id err, please check your  device id: device_id = 0x%x\n", device_id);
		    return NULL;
	}
	return &wb_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status)
{
	int ret = 0;
	switch(device_id) {
		case 0xAA:
		case 0xAE:
		case 0xBA:
			switch((ret = ((ecc_status >> 4) & 0x3))) {
				case 0x2:
					ret = -EBADMSG;
					break;
				default:
					ret = 0;
			}
			break;
		default:
			dev_warn(flash->dev, "device_id err, it maybe don`t support this device, check your device id: device_id = 0x%x\n", device_id);
			ret = -EIO;   //notice!!!
	}
	return ret;
}


static int wb_nand_init(void) {

	wb_nand = kzalloc(sizeof(*wb_nand), GFP_KERNEL);
	if(!wb_nand) {
		pr_err("alloc wb_nand struct fail\n");
		return -ENOMEM;
	}

	wb_nand->id_manufactory = 0xEF;
	wb_nand->id_device_list = device_id;
	wb_nand->id_device_count = WB_DEVICES_NUM;

	wb_nand->ops.get_cdt_params = wb_get_cdt_params;
	wb_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	wb_nand->ops.get_feature = NULL;

	return ingenic_sfcnand_register(wb_nand);
}

fs_initcall(wb_nand_init);
