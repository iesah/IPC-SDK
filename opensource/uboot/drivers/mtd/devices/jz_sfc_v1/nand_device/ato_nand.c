#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_nand.h"
#include "nand_common.h"

#define ATO_DEVICES_NUM         1
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		30
#define	TSHSL_W		30

static struct jz_nand_base_param ato25d1ga_param = {

	.pagesize = 2 * 1024,
	.oobsize = 64,
	.blocksize = 2 * 1024 * 64,
	.flashsize = 2 * 1024 * 64 * 1024,

	.tSETUP = TSETUP,
	.tHOLD  = THOLD,
	.tSHSL_R = TSHSL_R,
	.tSHSL_W = TSHSL_W,

	.ecc_max = 0,
#ifdef CONFIG_SPI_STANDARD
	.need_quad = 0,
#else
	.need_quad = 1,
#endif

};

static struct device_id_struct device_id[ATO_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x12, "ATO25D1GA", &ato25d1ga_param),
};

static int32_t ato_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	uint8_t device_id = nand_desc->id_device;

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

	/*wait poll time*/
	sfc_message_add_tail(&transfer, &message);
	if(sfc_sync(flash->sfc, &message)) {
	        printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	switch(device_id) {
		case 0x12:
			return 0;
		default:
			pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
			return -EIO;   //notice!!!
	}
}

int ato_nand_init(void) {
	struct jz_nand_device *ato_nand;
	ato_nand = kzalloc(sizeof(*ato_nand), GFP_KERNEL);
	if(!ato_nand) {
		pr_err("alloc ato_nand struct fail\n");
		return -ENOMEM;
	}

	ato_nand->id_manufactory = 0x9B;
	ato_nand->id_device_list = device_id;
	ato_nand->id_device_count = ATO_DEVICES_NUM;

	ato_nand->ops.nand_read_ops.get_feature = ato_get_read_feature;
	return jz_spinand_register(ato_nand);
}
SPINAND_MOUDLE_INIT(ato_nand_init);

