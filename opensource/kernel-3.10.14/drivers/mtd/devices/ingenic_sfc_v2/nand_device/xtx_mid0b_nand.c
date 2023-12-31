#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include "../spinand.h"
#include "../ingenic_sfc_common.h"
#include "nand_common.h"

#define XTX_MID0B_DEVICES_NUM         3
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		260
#define TPP		350
#define TBE		3

static struct ingenic_sfcnand_device *xtx_mid0b_nand;

static struct ingenic_sfcnand_base_param xtx_mid0b_param[XTX_MID0B_DEVICES_NUM] = {

	[0] = {
		/* XT26G01A */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 1 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[1] = {
		/* XT26G02B */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[2] = {
		/* XT26G01C */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 1 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0xf,
#ifdef CONFIG_SPI_STANDARD_MODE
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

};

static struct device_id_struct device_id[XTX_MID0B_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xE1, "XT26G01A ", &xtx_mid0b_param[0]),
	DEVICE_ID_STRUCT(0xF2, "XT26G02B ", &xtx_mid0b_param[1]),
	DEVICE_ID_STRUCT(0x11, "XT26G01C ", &xtx_mid0b_param[2]),
};


static cdt_params_t *xtx_mid0b_get_cdt_params(struct sfc_flash *flash, uint8_t device_id)
{
	CDT_PARAMS_INIT(xtx_mid0b_nand->cdt_params);

	switch(device_id) {
	    case 0xE1:
	    case 0xF2:
	    case 0x11:
		    break;
	    default:
		    dev_err(flash->dev, "device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
		    return NULL;
	}

	return &xtx_mid0b_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status)
{
	int ret = 0;

	switch(device_id) {
		case 0xE1:
			switch((ecc_status >> 2) & 0xf) {
			    case 0x12:
			    case 0x8:
				ret = -EBADMSG;
				break;
			    case 0x0 ... 0x7:
			    default:
				ret = 0;
				break;
			}
			break;

		case 0xF2:
			switch((ecc_status >> 4) & 0x3) {
			    case 0x02:
				    ret = -EBADMSG;
				    break;
			    default:
				    ret = 0;
			}
			break;

		case 0x11:
			switch((ecc_status >> 4) & 0xf) {
			    case 0x0f:
					dev_err(flash->dev, "SFC ECC:Bit errors greater than 8 bits detected and not corrected.\n");
				    ret = -EBADMSG;
				    break;
			    default:
				    ret = 0;
			}
			break;

		default:
			dev_warn(flash->dev, "device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;

	}
	return ret;
}


static int xtx_mid0b_nand_init(void) {

	xtx_mid0b_nand = kzalloc(sizeof(*xtx_mid0b_nand), GFP_KERNEL);
	if(!xtx_mid0b_nand) {
		pr_err("alloc xtx_mid0b_nand struct fail\n");
		return -ENOMEM;
	}

	xtx_mid0b_nand->id_manufactory = 0x0B;
	xtx_mid0b_nand->id_device_list = device_id;
	xtx_mid0b_nand->id_device_count = XTX_MID0B_DEVICES_NUM;

	xtx_mid0b_nand->ops.get_cdt_params = xtx_mid0b_get_cdt_params;
	xtx_mid0b_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	xtx_mid0b_nand->ops.get_feature = NULL;

	return ingenic_sfcnand_register(xtx_mid0b_nand);
}

fs_initcall(xtx_mid0b_nand_init);
