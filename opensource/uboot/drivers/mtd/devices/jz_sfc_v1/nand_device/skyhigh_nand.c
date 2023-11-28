#include <errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_nand.h"
#include "nand_common.h"

#define SKY_DEVICES_NUM 1
#define TSETUP 5
#define THOLD 5
#define TSHSL_R 100
#define TSHSL_W 100

uint8_t flag = 1;

static struct jz_nand_base_param sky_param[SKY_DEVICES_NUM] = {

	[0] = {
		/* S35ML01G3 */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP = TSETUP,
		.tHOLD = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.ecc_max = 0x6,
#ifdef CONFIG_SPI_STANDARD
		.need_quad = 0,
#else
		.need_quad = 1,
#endif
	},

};

static struct device_id_struct device_id[SKY_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x15, "S35ML01G3", &sky_param[0]),
};

static int32_t set_feature(uint8_t addr, uint8_t val_feature) {

	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int32_t ret = 0;


	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);


	cmd.cmd = SPINAND_CMD_SET_FEATURE;
	cmd.dataen = ENABLE;
	transfer.addr_len = 1;
	transfer.addr = addr;
	transfer.len = 1;
	transfer.data = (char *)&val_feature;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_WRITE;
	transfer.cmd_info = &cmd;

	sfc_message_add_tail(&transfer, &message);
	if (sfc_sync(NULL, &message))
	{
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}
	return 0;

}

static int32_t get_feature(struct flash_operation_message *op_info, uint8_t addr)
{

	struct sfc_flash *flash = op_info->flash;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	uint8_t val_feature = 0;
	int32_t ret = 0;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

retry:
	cmd.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = addr;
	transfer.addr_len = 1;

	cmd.dataen = DISABLE;
	transfer.len = 0;

	transfer.data_dummy_bits = 0;
	cmd.sta_exp = (0 << 0);
	cmd.sta_msk = SPINAND_IS_BUSY;
	transfer.cmd_info = &cmd;
	transfer.ops_mode = CPU_OPS;

	sfc_message_add_tail(&transfer, &message);
	if (sfc_sync(NULL, &message))
	{
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}

	val_feature = sfc_get_sta_rt(flash->sfc);
	if(val_feature & SPINAND_IS_BUSY)
		goto retry;
	return val_feature;
}

static int32_t sky_read_get_feature(struct flash_operation_message *op_info)
{
	struct sfc_flash *flash = op_info->flash;
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	uint8_t device_id = nand_desc->id_device;
	uint8_t ecc_status = 0;
	int32_t ret = 0;

	ecc_status = get_feature(op_info, SPINAND_ADDR_STATUS);

	switch (device_id)
	{
		//S35ML01G3
		case 0x15:
			switch ((ret = ((ecc_status >> 4) & 0x3)))
			{
				case 0x3:
					ret = -EBADMSG;
					break;
				default:
					ret = 0;
			}
			break;

		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x  line:%d\n", device_id, __LINE__);
			ret = -EIO; //notice!!!
	}
	return ret;
}

int sky_nand_init(void)
{
	struct jz_nand_device *sky_nand;
	sky_nand = kzalloc(sizeof(*sky_nand), GFP_KERNEL);
	if (!sky_nand)
	{
		pr_err("alloc sky_nand struct fail\n");
		return -ENOMEM;
	}
#ifndef CONFIG_SPI_STANDARD
	printf("SFC ERROR:The device does not support Quad mode!\n");
#else
	writel(0x02000000, 0x10010024);//#WP high
	set_feature(0xa0,0x7e);//unlock
	set_feature(0xa0,0x00);
#endif

	sky_nand->id_manufactory = 0x01;
	sky_nand->id_device_list = device_id;
	sky_nand->id_device_count = SKY_DEVICES_NUM;

	sky_nand->ops.nand_read_ops.get_feature = sky_read_get_feature;
	sky_nand->ops.nand_erase_ops.get_feature = sky_read_get_feature;
	return jz_spinand_register(sky_nand);
}
SPINAND_MOUDLE_INIT(sky_nand_init);
