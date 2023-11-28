//#define DEBUG
#include <config.h>
#include <common.h>
#include <jz_lcd/jz_dsim.h> /* extern struct dsi_device jz_dsi; */
#include <jz_lcd/jz_lcd_v14.h>
#include <lcd.h>

extern void* lcd_get_fb_base(void);

static struct dsi_device *dsi = &jz_dsi;
static void write_word(unsigned int address, unsigned int data);
static void write_part(unsigned int address, unsigned int data, unsigned char shift, unsigned char width);
static unsigned int read_word(unsigned int reg_address);
static unsigned int read_part(unsigned int address, unsigned char shift, unsigned char width);

static int do_send(int argc, char * const argv[])
{
	struct dsi_cmd_packet my_data;
	unsigned short data_length, parameter_length;
	unsigned short i;

	if (argc < 2)
		return CMD_RET_USAGE;

	data_length = argc - 1; /* first data is the command(register address) */
	parameter_length = data_length - 1; /* format: command data0 data1 ... dataN-1, 1 + N = data_length */
	switch (parameter_length) {
	case 0:
		my_data.packet_type = 0x05; /* DCS Short WRITE, no parameters */
		my_data.cmd0_or_wc_lsb = simple_strtol(argv[1], NULL, 16);
		my_data.cmd1_or_wc_msb = 0x00;
		debug("case 0\n");
		break;
	case 1:
		my_data.packet_type = 0x15; /* DCS Short WRITE, 1 parameters */
		my_data.cmd0_or_wc_lsb = simple_strtol(argv[1], NULL, 16);
		my_data.cmd1_or_wc_msb = simple_strtol(argv[2], NULL, 16);
		debug("case 1\n");
		break;
	default:
		my_data.packet_type = 0x39; /* DCS Long Write/write_LUT Command Packet */
		my_data.cmd0_or_wc_lsb = data_length & 0xff;;
		my_data.cmd1_or_wc_msb = (data_length >> 8) & 0xff;
		for (i = 0; i < data_length; i++)
			my_data.cmd_data[i] = simple_strtol(argv[i + 1], NULL, 16);
		debug("case default\n");
		break;
	}
	write_command(dsi, my_data);

	return CMD_RET_SUCCESS;
}

static int do_recv(int argc, char *const argv[])
{
	printf ("Sorry, Now not support this function!!\n");
	return CMD_RET_USAGE;
}

static int do_clock(int argc, char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (strcmp("on", argv[1]) == 0) {
		jz_dsih_dphy_clock_en(dsi, 1);
	} else if (strcmp("off", argv[1]) == 0) {
		jz_dsih_dphy_clock_en(dsi, 0);
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_gate(int argc, char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (strcmp("on", argv[1]) == 0) {
		*(volatile unsigned int *)0xb0000020 &= ~(1<<26); //open  gate for clk
	} else if (strcmp("off", argv[1]) == 0) {
		*(volatile unsigned int *)0xb0000020 |= (1<<26); //close gate for clk
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_shutdown(int argc, char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (strcmp("yes", argv[1]) == 0) {
		jz_dsih_dphy_shutdown(dsi, 0);
	} else if (strcmp("no", argv[1]) == 0) {
		jz_dsih_dphy_shutdown(dsi, 1);
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_power(int argc, char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (strcmp("on", argv[1]) == 0) {
		mipi_dsih_hal_power(dsi, 1);
	} else if (strcmp("off", argv[1]) == 0) {
		mipi_dsih_hal_power(dsi, 0);
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_mipi_power(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct dsi_cmd_packet my_data;

	if(argc != 2) {
		printf("your parameter is wrong\n");
		return -1;
	}

	if (!strcmp(argv[1], "enable")) {

	} else if (!strcmp(argv[1], "clock")) {
		jz_dsih_dphy_clock_en(dsi, 0);
	} else if (!strcmp(argv[1], "shutdown")) {
		jz_dsih_dphy_shutdown(dsi, 0);
	} else if (!strcmp(argv[1], "gate")) {
		*(volatile unsigned int *)0xb0000020 |= (1<<26); //close gate for clk
	} else if (!strcmp(argv[1], "power")) {
        mipi_dsih_hal_power(dsi, 0);
	} else if (!strcmp(argv[1], "lp")) {
		mipi_dsih_dphy_enable_hs_clk(dsi, 0);
	}

	return 0;
}

static int do_lp(int argc, char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (strcmp("on", argv[1]) == 0) {
		mipi_dsih_dphy_enable_hs_clk(dsi, 0);
	} else if (strcmp("off", argv[1]) == 0) {
		mipi_dsih_dphy_enable_hs_clk(dsi, 1);
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_ulps(int argc, char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (strcmp("on", argv[1]) == 0) {
		jz_dsih_dphy_ulpm_enter(dsi);
	} else if (strcmp("off", argv[1]) == 0) {
		jz_dsih_dphy_ulpm_exit(dsi);
	} else if (strcmp("test", argv[1]) == 0) {
		int i;
		for (i = 1; i <= 1000; i++) {
			jz_dsih_dphy_ulpm_enter(dsi);
			mdelay(2000);
			jz_dsih_dphy_ulpm_exit(dsi);
			printf("test times : [%d]\n", i);
			mdelay(2000);
		}
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

/**
 * Write a 32-bit word to a memory, always a register
 * @param memory address
 * @param data 32-bit word to be written to a memeory address
 */
static void write_word(unsigned int address, unsigned int data)
{
	*((volatile unsigned int *)(address)) = data;
}

/**
 * Write a bit field o a 32-bit word to a memory, always a register
 * @param memory address
 * @param data to be written to memeory
 * @param shift bit shift from the left (system is LITTLE EMDIAN)
 * @param width of bit field
 */
static void write_part(unsigned int address, unsigned int data, unsigned char shift, unsigned char width)
{
	unsigned int mask;
	unsigned int temp;

	if (width == 32) {
		write_word(address, data);
		return ;
	}

	mask = (1 << width) - 1;
	temp = read_word(address);
	temp &= ~(mask << shift);
	temp |= (data & mask) << shift;
	write_word(address, temp);
}

/**
 * Read a 32-bit word from a memory, always a register
 * @param address
 * @return 32-bit word value stored in memory, always in register
 */
static unsigned int read_word(unsigned int address)
{
	return (*((volatile unsigned int *)(address)));
}


/**
 * Read a 32-bit word part from memory, always a register
 * @param address
 * @param shift bit shift from the left (system is LITTLE ENDIAN)
 * @param width of bit field
 * @return bit field read from memory
 */
static unsigned int read_part(unsigned int address, unsigned char shift, unsigned char width)
{
	return (width == 32)? read_word(address) : (read_word(address) >> shift) & ((1 << width) - 1);
}

static int do_read(int argc, char * const argv[])
{
	unsigned int address;
	unsigned int start;
	unsigned int end;
	unsigned int data;

	if(argc < 2 || argc > 4) {
		return CMD_RET_USAGE;
	}

	address = simple_strtoul(argv[1], NULL, 16);

	if (argc > 2) {
		start = simple_strtoul(argv[2], NULL, 10);
	} else {
		start = 0;
	}

	if (argc > 3) {
		end = simple_strtoul(argv[3], NULL, 10);
	} else {
		if (argc > 2)
			end = start; /* read 0x74 0 == read 0x74 0 0 */
		else
			end = 31;    /* read 0x74 == read 0x74 0 31 */
	}

	data = read_part(address + DSI_BASE, start, end - start + 1);

	printf ("data: 0x%x\n", data);

	return 0;
}

static int do_write(int argc, char * const argv[])
{
	unsigned int address;
	unsigned int start;
	unsigned int end;
	unsigned int data;

	if(argc < 3 || argc > 5) {
		return CMD_RET_USAGE;
	}

	address = simple_strtoul(argv[1], NULL, 16);
	data = simple_strtoul(argv[2], NULL, 16);

	if (argc > 4) {
		start = simple_strtoul(argv[3], NULL, 10);
	} else {
		start = 0;
	}

	if (argc > 4) {
		end = simple_strtoul(argv[4], NULL, 10);
	} else {
		if (argc > 3)
			end = start; /* write 0x78 0x01 2 == write 0x78 0x01 2 2 */
		else
			end = 31;    /* write 0x78 0x01 == write 0x78 0x01 0 31 */
	}

	write_part(address + DSI_BASE, data, start, end - start + 1);

	return 0;
}

enum mipi_ops {
	MIPI_OPS_READ,
	MIPI_OPS_WRITE,
	MIPI_OPS_ULPS,
	MIPI_OPS_LP,
	MIPI_OPS_SEND,
	MIPI_OPS_RECV,
	MIPI_OPS_CLOCK,
	MIPI_OPS_GATE,
	MIPI_OPS_SHUTDOWN,
	MIPI_OPS_POWER,
};

static int do_mipiops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	enum mipi_ops mipi_ops_val;

	if (argc < 3 || argc > 10)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "read") == 0) {
		mipi_ops_val = MIPI_OPS_READ;
	} else if (strcmp(argv[1], "write") == 0) {
		mipi_ops_val = MIPI_OPS_WRITE;
	} else if (strcmp(argv[1], "ulps") == 0) {
		mipi_ops_val = MIPI_OPS_ULPS;
	} else if (strcmp(argv[1], "lp") == 0) {
		mipi_ops_val = MIPI_OPS_LP;
	} else if (strcmp(argv[1], "send") == 0) {
		mipi_ops_val = MIPI_OPS_SEND;
	} else if (strcmp(argv[1], "recv") == 0) {
		mipi_ops_val = MIPI_OPS_RECV;
	} else if (strcmp(argv[1], "clock") == 0) {
		mipi_ops_val = MIPI_OPS_CLOCK;
	} else if (strcmp(argv[1], "gate") == 0) {
		mipi_ops_val = MIPI_OPS_GATE;
	} else if (strcmp(argv[1], "shutdown") == 0) {
		mipi_ops_val = MIPI_OPS_SHUTDOWN;
	} else if (strcmp(argv[1], "power") == 0) {
		mipi_ops_val = MIPI_OPS_POWER;
	} else {
		printf("unsupport sub command\n");
		return CMD_RET_USAGE;
	}

	switch (mipi_ops_val) {
	case MIPI_OPS_READ:
		return do_read(argc - 1, &argv[1]);
	case MIPI_OPS_WRITE:
		return do_write(argc - 1, &argv[1]);
	case MIPI_OPS_ULPS:
		return do_ulps(argc - 1, &argv[1]);
	case MIPI_OPS_LP:
		return do_lp(argc - 1, &argv[1]);
	case MIPI_OPS_SEND:
		return do_send(argc - 1, &argv[1]);
	case MIPI_OPS_RECV:
		return do_recv(argc - 1, &argv[1]);
	case MIPI_OPS_CLOCK:
		return do_clock(argc - 1, &argv[1]);
	case MIPI_OPS_GATE:
		return do_gate(argc - 1, &argv[1]);
	case MIPI_OPS_SHUTDOWN:
		return do_shutdown(argc - 1, &argv[1]);
	case MIPI_OPS_POWER:
		return do_power(argc - 1, &argv[1]);
	default:
		return CMD_RET_USAGE;
	}
}

U_BOOT_CMD(mipi, 6, 1, do_mipiops,
		   "MIPI sub system",
		   "mipi read register_addr [bit_from] [bit_to]\n"
		   "mipi write regitser_addr value [bit_from] [bit_to]\n"
		   "mipi ulps on/off/test\n"
		   "mipi lp on/off\n"
		   "mipi send register [data0] [data2] ... [dataN-1]\n"
		   "mipi recv register data_length\n"
		   "mipi clock on/off\n"
		   "mipi shutdown yes/no\n"
		   "mipi power on/off\n"
	);
