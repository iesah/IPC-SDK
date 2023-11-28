#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_nand.h"
#include "nand_common.h"

#define FM_DEVICES_NUM         2
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		80
#define	TSHSL_W		80

#define TRD		100
#define TPP		900
#define TBE		10

static struct jz_nand_base_param fm_param[FM_DEVICES_NUM] = {

	[0] = {
		/* FM25S01A */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x2,
#ifdef CONFIG_SPI_QUAD
	.need_quad = 1,
#else
	.need_quad = 0,
#endif
	},
	[1] = {
		/* FM25S02A */
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

		.ecc_max = 0x2,
#ifdef CONFIG_SPI_QUAD
	.need_quad = 1,
#else
	.need_quad = 0,
#endif
	}
};

static struct device_id_struct device_id[FM_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xE4, "FM25S01A",  &fm_param[0]),
	DEVICE_ID_STRUCT(0xE5, "FM25S02A",  &fm_param[1]),
};

static int32_t fm_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	uint8_t device_id = nand_desc->id_device;
	uint8_t ecc_status = 0;
	int32_t ret = 0;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	cmd.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = SPINAND_ADDR_STATUS;
	transfer.addr_len = 1;

	cmd.dataen = DISABLE;
	transfer.len = 0;

	transfer.data_dummy_bits = 0;
	cmd.sta_exp = (0 << 0);
	cmd.sta_msk = SPINAND_IS_BUSY;
	transfer.cmd_info = &cmd;
	transfer.ops_mode = CPU_OPS;

	sfc_message_add_tail(&transfer, &message);
	if(sfc_sync(flash->sfc, &message)) {
	        dev_err(flash->dev, "sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	ecc_status = sfc_get_sta_rt(flash->sfc);

	switch(device_id) {
		case 0xE4 ... 0xE5:
			switch((ret = ((ecc_status >> 4) & 0x3))) {
				case 0x0 :
					ret = 0;
					break;
				default:
					ret = -EBADMSG;
			}
			break;

		default:
			dev_err(flash->dev, "device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!
	}
	return ret;
}

int __init fm_nand_init(void) {
	struct jz_nand_device *fm_nand;
	fm_nand = kzalloc(sizeof(*fm_nand), GFP_KERNEL);
	if(!fm_nand) {
		pr_err("alloc fm_nand struct fail\n");
		return -ENOMEM;
	}

	fm_nand->id_manufactory = 0xA1;
	fm_nand->id_device_list = device_id;
	fm_nand->id_device_count = FM_DEVICES_NUM;

	fm_nand->ops.nand_read_ops.get_feature = fm_get_read_feature;
	return jz_nand_register(fm_nand);
}
module_init(fm_nand_init);
