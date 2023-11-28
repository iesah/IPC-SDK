#include "../jz_sfc_nand.h"

void nand_pageread_to_cache(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	cmd->cmd = SPINAND_CMD_PARD;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->pageaddr;
	transfer->addr_len = 3;

	cmd->dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;
	return;
}
EXPORT_SYMBOL_GPL(nand_pageread_to_cache);

void nand_single_read(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info, uint8_t columnlen) {

	cmd->cmd = SPINAND_CMD_FRCH;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->columnaddr;
	transfer->addr_len = columnlen;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;
	return;
}
EXPORT_SYMBOL_GPL(nand_single_read);

void nand_quad_read(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info, uint8_t columnlen) {

	cmd->cmd = SPINAND_CMD_RDCH_X4;
	transfer->sfc_mode = TM_QI_QO_SPI;

	transfer->addr = op_info->columnaddr;
	transfer->addr_len = columnlen;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;
	return;
}
EXPORT_SYMBOL_GPL(nand_quad_read);

void nand_write_enable(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	cmd->cmd = SPINAND_CMD_WREN;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = 0;
	transfer->addr_len = 0;

	cmd->dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;
	return;
}
EXPORT_SYMBOL_GPL(nand_write_enable);

void nand_single_load(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	cmd->cmd = SPINAND_CMD_PRO_LOAD;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->columnaddr;
	transfer->addr_len = 2;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_WRITE;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;

}
EXPORT_SYMBOL_GPL(nand_single_load);

void nand_quad_load(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	cmd->cmd = SPINAND_CMD_PRO_LOAD_X4;
	transfer->sfc_mode = TM_QI_QO_SPI;

	transfer->addr = op_info->columnaddr;
	transfer->addr_len = 2;

	cmd->dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_WRITE;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = DMA_OPS;

}
EXPORT_SYMBOL_GPL(nand_quad_load);

void nand_program_exec(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	cmd->cmd = SPINAND_CMD_PRO_EN;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->pageaddr;
	transfer->addr_len = 3;

	cmd->dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;
}
EXPORT_SYMBOL_GPL(nand_program_exec);

int32_t nand_get_program_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;

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
	        dev_err(flash->dev,"sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(sfc_get_sta_rt(flash->sfc) & (1 << 3))
		return -EIO;

	return 0;
}
EXPORT_SYMBOL_GPL(nand_get_program_feature);

void nand_block_erase(struct sfc_transfer *transfer, struct cmd_info *cmd, struct flash_operation_message *op_info) {

	cmd->cmd = SPINAND_CMD_ERASE_128K;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->pageaddr;
	transfer->addr_len = 3;

	cmd->dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;

}
EXPORT_SYMBOL_GPL(nand_block_erase);

int32_t nand_get_erase_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;

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
	        dev_err(flash->dev,"sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(sfc_get_sta_rt(flash->sfc) & (1 << 2))
		return -EIO;

	return 0;
}
EXPORT_SYMBOL_GPL(nand_get_erase_feature);

void nand_set_feature(struct sfc_transfer *transfer, struct cmd_info *cmd, uint8_t addr, uint8_t val) {

	cmd->cmd = SPINAND_CMD_SET_FEATURE;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = addr;
	transfer->addr_len = 1;

	cmd->dataen = ENABLE;
	transfer->data = &val;
	transfer->len = 1;
	transfer->direction = GLB_TRAN_DIR_WRITE;
	transfer->data_dummy_bits = 0;

	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;

}
EXPORT_SYMBOL_GPL(nand_set_feature);

void nand_get_feature(struct sfc_transfer *transfer, struct cmd_info *cmd, uint8_t addr, uint8_t *val) {

	cmd->cmd = SPINAND_CMD_GET_FEATURE;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = addr;
	transfer->addr_len = 1;

	cmd->dataen = ENABLE;
	transfer->data = val;
	transfer->len = 1;
	transfer->direction = GLB_TRAN_DIR_READ;
	transfer->data_dummy_bits = 0;

	transfer->cmd_info = cmd;
	transfer->ops_mode = CPU_OPS;

}
EXPORT_SYMBOL_GPL(nand_get_feature);
