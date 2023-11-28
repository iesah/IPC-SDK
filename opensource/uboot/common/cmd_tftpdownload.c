/*
 *tftp download support
 */

#include <common.h>
#include <environment.h>
#include <command.h>
#include <malloc.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <spi_flash.h>
#include <linux/mtd/mtd.h>
#include <net.h>

#ifdef CONFIG_CMD_TFTPDOWNLOAD

#define TFTP_CMD_NUM		3
#define AU_FL_FW_ST         0x000000
#define LOAD_ADDR			((unsigned char *)0x80600000)
#define CONFIG_SF_DEFAULT_SPEED    1000000

static struct spi_flash *flash;

static void netboot_update_env(void)
{
	char tmp[22];

	if (NetOurGatewayIP) {
		ip_to_string(NetOurGatewayIP, tmp);
		setenv("gatewayip", tmp);
	}

	if (NetOurSubnetMask) {
		ip_to_string(NetOurSubnetMask, tmp);
		setenv("netmask", tmp);
	}

	if (NetOurHostName[0])
		setenv("hostname", NetOurHostName);

	if (NetOurRootPath[0])
		setenv("rootpath", NetOurRootPath);

	if (NetOurIP) {
		ip_to_string(NetOurIP, tmp);
		setenv("ipaddr", tmp);
	}
#if !defined(CONFIG_BOOTP_SERVERIP)
	/*
	 * Only attempt to change serverip if net/bootp.c:BootpCopyNetParams()
	 * could have set it
	 */
	if (NetServerIP) {
		ip_to_string(NetServerIP, tmp);
		setenv("serverip", tmp);
	}
#endif
	if (NetOurDNSIP) {
		ip_to_string(NetOurDNSIP, tmp);
		setenv("dnsip", tmp);
	}
#if defined(CONFIG_BOOTP_DNS2)
	if (NetOurDNS2IP) {
		ip_to_string(NetOurDNS2IP, tmp);
		setenv("dnsip2", tmp);
	}
#endif
	if (NetOurNISDomain[0])
		setenv("domain", NetOurNISDomain);

#if defined(CONFIG_CMD_SNTP) \
	&& defined(CONFIG_BOOTP_TIMEOFFSET)
	if (NetTimeOffset) {
		sprintf(tmp, "%d", NetTimeOffset);
		setenv("timeoffset", tmp);
	}
#endif
#if defined(CONFIG_CMD_SNTP) \
	&& defined(CONFIG_BOOTP_NTPSERVER)
	if (NetNtpServerIP) {
		ip_to_string(NetNtpServerIP, tmp);
		setenv("ntpserverip", tmp);
	}
#endif
}

static int netboot_common(enum proto_t proto, cmd_tbl_t *cmdtp, int argc,
		char * const argv[])
{
	char *s;
	char *end;
	int   rcode = 0;
	int   size;
	ulong addr;

	/* pre-set load_addr */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	switch (argc) {
		case 1:
			break;

		case 2:/*
				* Only one arg - accept two forms:
				*Just load address, or just boot file name. The latter
				*form must be written in a format which can not be
				*mis-interpreted as a valid number.
				*/
			addr = simple_strtoul(argv[1], &end, 16);
			if (end == (argv[1] + strlen(argv[1])))
				load_addr = addr;
			else
				copy_filename(BootFile, argv[1], sizeof(BootFile));
			break;

		case 3: load_addr = simple_strtoul(argv[1], NULL, 16);
				copy_filename(BootFile, argv[2], sizeof(BootFile));

				break;

#ifdef CONFIG_CMD_TFTPPUT
		case 4:
				if (strict_strtoul(argv[1], 16, &save_addr) < 0 ||
						strict_strtoul(argv[2], 16, &save_size) < 0) {
					printf("Invalid address/size\n");
					return cmd_usage(cmdtp);
				}
				copy_filename(BootFile, argv[3], sizeof(BootFile));
				break;
#endif
		default:
				bootstage_error(BOOTSTAGE_ID_NET_START);
				return CMD_RET_USAGE;
	}
	bootstage_mark(BOOTSTAGE_ID_NET_START);

	if ((size = NetLoop(proto)) < 0) {
		bootstage_error(BOOTSTAGE_ID_NET_NETLOOP_OK);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_NET_NETLOOP_OK);

	/* NetLoop ok, update environment */
	netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if (size == 0) {
		bootstage_error(BOOTSTAGE_ID_NET_LOADED);
		return 0;
	}

	/* flush cache */
	flush_cache(load_addr, size);

	bootstage_mark(BOOTSTAGE_ID_NET_LOADED);

	rcode = bootm_maybe_autostart(cmdtp, argv[0]);

	if (rcode < 0)
		bootstage_error(BOOTSTAGE_ID_NET_DONE_ERR);
	else
		bootstage_mark(BOOTSTAGE_ID_NET_DONE);
	return rcode;
}

static int update_to_flash(unsigned int size)
{
	unsigned long start;
	int rc;
	void *buf;

	start = AU_FL_FW_ST;

	printf("flash erase...\n");
	rc = flash->erase(flash, start, size);
	if (rc) {
		printf("SPI flash sector erase failed\n");
		return 1;
	}

	buf = map_physmem((unsigned long)LOAD_ADDR, size, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	/* copy the data from RAM to FLASH */

	printf("flash write...\n");
	rc = flash->write(flash, start, size, buf);
	if (rc) {
		printf("SPI flash write failed, return %d\n", rc);
		return 1;
	}

	unmap_physmem(buf, size);
	return 0;
}

int do_tftp_download (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret,state;
	int old_ctrlc;
	char *cmd_info[TFTP_CMD_NUM];

	/*tftpboot ram_addr tftpfile*/
	cmd_info[0] = "tftpboot";
	cmd_info[1] = "0x80600000";
	cmd_info[2] = argv[1];
	bootstage_mark_name(BOOTSTAGE_KERNELREAD_START, "tftp_start");
	ret = netboot_common(TFTPGET, cmdtp, TFTP_CMD_NUM, cmd_info);
	bootstage_mark_name(BOOTSTAGE_KERNELREAD_STOP, "tftp_done");
	if(ret)
		return -1;

	old_ctrlc = disable_ctrlc(0);

	flash = spi_flash_probe(0, 0, CONFIG_SF_DEFAULT_SPEED, 0x3);
	if (!flash) {
		printf("Failed to initialize SPI flash\n");
		return -1;
	}

	printf("flash size is %d\n", flash->size);
	state = update_to_flash(flash->size);
	if(state) {
		printf("update to flash fail!\n");
	}

	disable_ctrlc(old_ctrlc);

	return 0;
}

U_BOOT_CMD(
	tftpdownload,   2,  1,  do_tftp_download,
	"auto upgrade file from tftp to flash",
	"<interface> <filename>\n"
	"	- Load binary file 'filename' from 'tftpfile' on 'interface'"
);

#endif
