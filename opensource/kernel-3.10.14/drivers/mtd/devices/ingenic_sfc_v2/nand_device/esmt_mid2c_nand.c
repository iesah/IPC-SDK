#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include "../spinand.h"
#include "../ingenic_sfc_common.h"
#include "nand_common.h"

#define ESMT_MID2C_DEVICES_NUM         1
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		50
#define TPP		400
#define TBE		10

static struct ingenic_sfcnand_device *esmt_mid2c_nand;

static struct ingenic_sfcnand_base_param esmt_mid2c_param[ESMT_MID2C_DEVICES_NUM] = {

	[0] = {
		/* F50L2G41XA */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x8,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},
};

static struct device_id_struct device_id[ESMT_MID2C_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x24, "F50L2G41XA", &esmt_mid2c_param[0]),
};


static cdt_params_t *esmt_mid2c_get_cdt_params(struct sfc_flash *flash, uint8_t device_id)
{
	CDT_PARAMS_INIT(esmt_mid2c_nand->cdt_params);

	switch(device_id) {
	    case 0x24:
		    break;
	    default:
		    dev_err(flash->dev, "device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
		    return NULL;
	}

	return &esmt_mid2c_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status)
{
	int ret = 0;

	switch(device_id) {
		case 0x24:
			switch((ecc_status >> 4) & 0x7) {
				case 0x2:
					dev_err(flash->dev, "SFC ECC:Bit errors greater than 8 bits detected and not corrected.\n");
					ret = -EBADMSG;
					break;
			    default:
					ret = 0x0;
			}
			break;
		default:
			dev_err(flash->dev, "device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!
	}
	return ret;
}


static int __init esmt_mid2c_nand_init(void) {

	esmt_mid2c_nand = kzalloc(sizeof(*esmt_mid2c_nand), GFP_KERNEL);
	if(!esmt_mid2c_nand) {
		pr_err("alloc esmt_mid2c_nand struct fail\n");
		return -ENOMEM;
	}

	esmt_mid2c_nand->id_manufactory = 0x2C;
	esmt_mid2c_nand->id_device_list = device_id;
	esmt_mid2c_nand->id_device_count = ESMT_MID2C_DEVICES_NUM;

	esmt_mid2c_nand->ops.get_cdt_params = esmt_mid2c_get_cdt_params;
	esmt_mid2c_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	esmt_mid2c_nand->ops.get_feature = NULL;

	return ingenic_sfcnand_register(esmt_mid2c_nand);
}

fs_initcall(esmt_mid2c_nand_init);
