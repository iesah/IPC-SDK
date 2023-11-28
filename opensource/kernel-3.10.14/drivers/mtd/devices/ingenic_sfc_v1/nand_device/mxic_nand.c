#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_nand.h"
#include "nand_common.h"

#define MXIC_DEVICES_NUM         3
#define MXIC_CMD_GET_ECC	0x7c
#define THOLD	    4
#define TSETUP	    4
#define TSHSL_R	    100
#define TSHSL_W	    100

#define TRD	    70
#define TRD_14AC    25
#define TPP	    600
#define TBE	    4

static struct jz_nand_base_param mxic_param[MXIC_DEVICES_NUM] = {
	[0] = {
	/*MX35LF1GE4AB*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 0,
	},
};

static struct device_id_struct device_id[MXIC_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x12, "MX35LF1GE4AB", &mxic_param[0]),
};

static uint8_t plane_select = 0;

static void mxic_pageread_to_cache(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	uint32_t pageaddr = op_info->pageaddr;

	cmd->cmd = SPINAND_CMD_PARD;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = pageaddr;
	transfer->addr_len = 3;

	cmd->dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;
	return;
}

static int32_t get_ecc_value(struct sfc_flash *flash) {
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	uint32_t buf = 0;
	int8_t count = 5;

	sfc_message_init(&message);
try_read_again:
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	cmd.cmd = MXIC_CMD_GET_ECC;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = 0;
	transfer.addr_len = 0;

	cmd.dataen = ENABLE;
	transfer.len = 1;
	transfer.data = (void *)&buf;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 8;
	transfer.cmd_info = &cmd;
	transfer.ops_mode = CPU_OPS;

	sfc_message_add_tail(&transfer, &message);
	if(sfc_sync(flash->sfc, &message) && count--)
		goto try_read_again;
	if(count < 0)
		return -EIO;
	return buf & 0xf;
}

static int32_t mxic_get_read_feature(struct flash_operation_message *op_info) {

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
		case 0x12:
			switch((ecc_status >> 0x4) & 0x3) {
			    case 0x0:
				    ret = 0;
				    break;
			    case 0x1:
				    ret = get_ecc_value(flash);
				    break;
			    case 0x2:
				    ret = -EBADMSG;
				    break;
			    default:
				   dev_err(flash->dev, "it is flash Unknown state, device_id: 0x%02x\n", device_id);
				    ret = -EIO;
			}
			break;
		default:
			dev_warn(flash->dev, "device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
		ret = -EIO;
	}
	return ret;
}

static void mxic_single_read(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	uint8_t device_id = nand_desc->id_device;
	uint32_t columnaddr = op_info->columnaddr;

	cmd->cmd = SPINAND_CMD_FRCH;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;
	return;
}

static void mxic_quad_read(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	uint8_t device_id = nand_desc->id_device;
	uint32_t columnaddr = op_info->columnaddr;

	cmd->cmd = SPINAND_CMD_RDCH_X4;
	transfer->sfc_mode = TM_QI_QO_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;
	return;
}

static void mxic_single_load(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	uint8_t device_id = nand_desc->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	cmd->cmd = SPINAND_CMD_PRO_LOAD;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_WRITE;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;
}

static void mxic_quad_load(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	uint8_t device_id = nand_desc->id_device;
	uint32_t columnaddr = op_info->columnaddr;

	cmd->cmd = SPINAND_CMD_PRO_LOAD_X4;
	transfer->sfc_mode = TM_QI_QO_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_WRITE;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;

}

static void mxic_program_exec(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	uint32_t pageaddr = op_info->pageaddr;

	cmd->cmd = SPINAND_CMD_PRO_EN;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = pageaddr;
	transfer->addr_len = 3;

	cmd->dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;
}

int __init mxic_nand_init(void) {
	struct jz_nand_device *mxic_nand;
	mxic_nand = kzalloc(sizeof(*mxic_nand), GFP_KERNEL);
	if(!mxic_nand) {
		pr_err("alloc mxic_nand struct fail\n");
		return -ENOMEM;
	}

	mxic_nand->id_manufactory = 0xC2;
	mxic_nand->id_device_list = device_id;
	mxic_nand->id_device_count = MXIC_DEVICES_NUM;

	mxic_nand->ops.nand_read_ops.pageread_to_cache = mxic_pageread_to_cache;
	mxic_nand->ops.nand_read_ops.get_feature = mxic_get_read_feature;
	mxic_nand->ops.nand_read_ops.single_read = mxic_single_read;
	mxic_nand->ops.nand_read_ops.quad_read = mxic_quad_read;

	mxic_nand->ops.nand_write_ops.single_load = mxic_single_load;
	mxic_nand->ops.nand_write_ops.quad_load = mxic_quad_load;
	mxic_nand->ops.nand_write_ops.program_exec = mxic_program_exec;

	return jz_nand_register(mxic_nand);
}
module_init(mxic_nand_init);
