#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


#include <ddr/chips-v2/ddr_chip.h>


struct soc_ddr_info {
	char soc_name[16];
	unsigned int vendor;
	unsigned int type;
	unsigned int capacity;

};


struct soc_ddr_info soc_info[] = {
	{
		.soc_name = "X2000",
		.vendor = VENDOR_WINBOND,
		.type 	= TYPE_LPDDR3,
		.capacity = MEM_128M,
	},
	{
		.soc_name = "X2000E",
		.vendor = VENDOR_WINBOND,
		.type	= TYPE_LPDDR2,
		.capacity = MEM_256M,
	},
	{
		.soc_name = "M300",
		.vendor = VENDOR_WINBOND,
		.type	= TYPE_LPDDR2,
		.capacity = MEM_256M,
	}
	{
		.soc_name = "X2100",
		.vendor = VENDOR_ESMT,
		.type	= TYPE_LPDDR2,
		.capacity = MEM_64M,
	}

};


/*
 *  add chip info to spl header @ 128 bytes.
 *
 *
 * */

int main(int argc, char *argv[])
{

	char *file = NULL;
	unsigned int seek = 0;
	char *soc = NULL;
	int i = 0;
	struct soc_ddr_info *info = NULL;
	int found = 0;


	if(argc < 4) {
		printf("usage: ./%s [uboot] [seek bytes] [soc]\n", argv[0]);
		printf("eg: ./%s u-boot-with-spl.bin 128 X2000E, add ddr info to 128 bytes\n", argv[0]);
		return -1;
	}


	file = argv[1];
	seek = atoi(argv[2]);
	soc = argv[3];

	for(i = 0; i < sizeof(soc_info) / sizeof(struct soc_ddr_info); i++) {
		info = &soc_info[i];
		if(!strcmp(soc, info->soc_name)) {
			found = 1;
			break;
		}
	}

	if(!found) {
		printf("don't support soc: %s\n", soc);
		return -1;
	}

	int fd = open(file, O_RDWR);
	int ret = 0;
	if(fd < 0) {
		perror("open file");
		return -1;
	}


	ret = lseek(fd, seek, SEEK_SET);
	if(ret < 0) {
		perror("failed to seek file");

		return -1;
	}

	unsigned int chip_id = ((info->type << 6 | info->vendor << 3 | info->capacity) & 0xffff);

	chip_id = (chip_id << 16) | chip_id;

	printf("chip_id: 0x%08x\n", chip_id);

	write(fd, &chip_id, 4);

	close(fd);


}
