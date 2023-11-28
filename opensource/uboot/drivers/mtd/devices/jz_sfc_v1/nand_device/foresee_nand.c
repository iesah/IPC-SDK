#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_nand.h"
#include "nand_common.h"

#define FS_DEVICES_NUM         4
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

static struct jz_nand_base_param fs_param[FS_DEVICES_NUM] = {

	[0] = {
		/*FS35ND01G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[1] = {
		/*F35SQA001G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.ecc_max = 0x1,
#ifdef CONFIG_SPI_STANDARD
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

	[2] = {
		/*F35SQA002G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.ecc_max = 0x1,
#ifdef CONFIG_SPI_STANDARD
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},
		[3] = {
		/*FS35ND02G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = 40,
		.tHOLD   = 40,
		.tSHSL_R = 20,
		.tSHSL_W = 50,

		.ecc_max = 0x4,
#ifdef CONFIG_SPI_STANDARD
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

};

static struct device_id_struct device_id[FS_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xEA, "FS35ND01G",		&fs_param[0]),
	DEVICE_ID_STRUCT(0x71, "F35SQA001G",	&fs_param[1]),
	DEVICE_ID_STRUCT(0x72, "F35SQA002G",	&fs_param[2]),
	DEVICE_ID_STRUCT(0xEB, "FS35ND02G", 	&fs_param[3]),
};

static int32_t fs_get_read_feature(struct flash_operation_message *op_info) {

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
	        printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	ecc_status = sfc_get_sta_rt(flash->sfc);

	switch(device_id) {
		case 0xEA ... 0xEB:
			switch((ret = ((ecc_status >> 4) & 0x7))) {
				case 0x0 ... 0x4:
					ret = 0;
					break;
				default:
					ret = -EBADMSG;
			}
			break;
		case 0x71:
		case 0x72:
			switch((ret = ((ecc_status >> 4) & 0x3))) {
				case 0x0:
				case 0x1:
					ret = 0;
					break;
				default:
					ret = -EBADMSG;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!

	}
	return ret;
}

int fs_nand_init(void) {
	struct jz_nand_device *fs_nand;
	fs_nand = kzalloc(sizeof(*fs_nand), GFP_KERNEL);
	if(!fs_nand) {
		pr_err("alloc fs_nand struct fail\n");
		return -ENOMEM;
	}

	fs_nand->id_manufactory = 0xCD;
	fs_nand->id_device_list = device_id;
	fs_nand->id_device_count = FS_DEVICES_NUM;

	fs_nand->ops.nand_read_ops.get_feature = fs_get_read_feature;
	return jz_spinand_register(fs_nand);
}
SPINAND_MOUDLE_INIT(fs_nand_init);
