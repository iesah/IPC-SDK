#ifndef __LINUX_SFC_NAND_H
#define __LINUX_SFC_NAND_H

#include <asm/arch/sfc.h>
#include<linker_lists.h>
#include<linux/list.h>
#include <serial.h>

/*nand flash*/
#define SPINAND_CMD_RDID                0x9f    /* read spi nand id */
#define SPINAND_CMD_WREN                0x06    /* spi nand write enable */
#define SPINAND_CMD_PRO_LOAD    		0x02    /* program load */
#define SPINAND_CMD_PRO_LOAD_X4    		0x32    /* program load fast*/
#define SPINAND_CMD_PRO_EN              0x10    /* program load execute */
#define SPINAND_CMD_PARD                0x13    /* read page data to spi nand cache */
#define SPINAND_CMD_PLRd                0x84    /* program load random data */
#define SPINAND_CMD_PLRd_X4             0xc4    /* program load random data x4*/
#define SPINAND_CMD_RDCH                0x03    /* read from spi nand cache */
#define SPINAND_CMD_RDCH_X4             0x6b    /* read from spi nand cache */
#define SPINAND_CMD_FRCH                0x0b    /* fast read from spi nand cache */
#define SPINAND_CMD_FRCH_IO             0xeb	/* for Quad I/O SPI mode */
#define SPINAND_CMD_ERASE_128K			0xd8    /* erase spi nand block 128K */
#define SPINAND_CMD_GET_FEATURE			0x0f    /* get spi nand feature */
#define SPINAND_CMD_SET_FEATURE			0x1f    /* set spi nand feature */


#define SPINAND_ADDR_PROTECT    0xa0    /* protect addr */
#define SPINAND_ADDR_STATUS     0xc0    /* get feature status addr */
#define SPINAND_ADDR_FEATURE    0xb0	/* set feature addr */



#define E_FALI					(1 << 2)        /* erase fail */
#define P_FAIL					(1 << 3)        /* write fail */
#define SPINAND_IS_BUSY			(1 << 0)		/* PROGRAM EXECUTE, PAGE READ, BLOCK ERASE, or RESET command is executing */
#define SPINAND_OP_BL_128K      (128 * 1024)

/********************************************/

#define DEVICE_ID_STRUCT(id, name_string, parameter) {      \
                .id_device = id,          \
                .name = name_string,                      \
		.param = parameter,			    \
}
/********************************************/

struct jz_nand_base_param {
	uint32_t pagesize;
	uint32_t blocksize;
	uint32_t oobsize;
	uint32_t flashsize;

	uint16_t tHOLD;
	uint16_t tSETUP;
	uint16_t tSHSL_R;
	uint16_t tSHSL_W;

	uint8_t ecc_max;
	uint8_t need_quad;
};

struct jz_spinand_partition {
	char name[32];         /* identifier string */
	uint32_t size;          /* partition size */
	uint32_t offset;        /* offset within the master MTD space */
	u_int32_t mask_flags;       /* master MTD flags to mask out for this partition */
	u_int32_t manager_mode;     /* manager_mode mtd or ubi */
};

struct jz_nand_partition_param {
	uint8_t num_partition;
/*	struct mtd_partition *partition;*/
	struct jz_spinand_partition *partition;
};

struct device_id_struct {
	uint8_t id_device;
	char *name;
	struct jz_nand_base_param *param;
};


/*this informtion is used by nand devices*/
struct flash_operation_message {
	struct sfc_flash *flash;
	uint32_t pageaddr;
	uint32_t columnaddr;
	u_char *buffer;
	size_t len;
};
struct jz_nand_read {
	void (*pageread_to_cache)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	int32_t (*get_feature)(struct flash_operation_message *);
	void (*single_read)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	void (*quad_read)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
};

struct jz_nand_write {
	void (*write_enable)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	void (*single_load)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	void (*quad_load)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	void (*program_exec)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	int32_t (*get_feature)(struct flash_operation_message *);
};

struct jz_nand_erase {
	void (*write_enable)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	void (*block_erase)(struct sfc_transfer *, struct cmd_info *, struct flash_operation_message *);
	int32_t (*get_feature)(struct flash_operation_message *);
};

struct jz_nand_ops {
	struct jz_nand_read nand_read_ops;
	struct jz_nand_write nand_write_ops;
	struct jz_nand_erase nand_erase_ops;
};

struct jz_nand_device {
	uint8_t id_manufactory;
	struct device_id_struct *id_device_list;
	uint8_t id_device_count;

	struct jz_nand_ops ops;
	struct list_head nand;
};

struct jz_nand_descriptor {
	uint8_t id_manufactory;
	uint8_t id_device;

	struct jz_nand_base_param param;
	struct jz_nand_partition_param partition;
	struct jz_nand_ops *ops;
};

struct jz_sfc_nand_param {
	char name[32];
	short nand_id;
	struct jz_nand_base_param param;
};

struct jz_sfc_nand_burner_param {
	unsigned int magic_num;
	int partition_num;
	struct jz_spinand_partition *partition;
};

int jz_nand_register(struct jz_nand_device *flash);


#define X_ENV_LENGTH        1024
#define X_COMMAND_LENGTH    128


#define MTD_MODE        0x0
#define UBI_MANAGER     0x1


int jz_spinand_register(struct jz_nand_device *flash);

typedef int (*spinand_regcall_t)(void);

#define SPINAND_MOUDLE_INIT(fn)      \
	ll_entry_declare(spinand_regcall_t, _1##fn, flash) = fn

static inline int spinand_moudle_init(void)
{
	spinand_regcall_t *call = ll_entry_start(spinand_regcall_t, flash);
	int ret = 0 , i, count;

	for (i = 0, count = ll_entry_count(spinand_regcall_t, flash);
			i < count;
			call++, i++) {
		ret = (*call)();
		if (ret) {
			printf("jz spi nand ops init func error\n");
			break;
		}
	}
	return ret;
}

int jz_sfc_nand_init(int sfc_quad_mode,struct jz_sfc_nand_burner_param *param);
int get_partition_from_nand(struct jz_sfc_nand_burner_param *param);
int set_one_partition_to_nand(int index, char*name, int offset, int size, int mask_flags, int manager_mode);
#endif
