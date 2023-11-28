/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 * Licensed under the GPL-2 or later.
 */
#include <common.h>
#include <spi_flash.h>

#include <asm/io.h>

#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED	1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#endif
#ifndef CONFIG_SF_DEFAULT_CS
# define CONFIG_SF_DEFAULT_CS		0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
# define CONFIG_SF_DEFAULT_BUS		0
#endif

static struct spi_flash *flash;
flash_info_t flash_info[1];
static int flash_num = 0;

void* norflash_memory_map_init()
{
	int ret = 1;
	void *buf;
	struct spi_flash *new;

	new = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!new) {
		printf("Failed to initialize SPI flash at %u:%u\n", CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS);
		return NULL;
	}

	if (flash)
		spi_flash_free(flash);
	flash = new;

	flash_info[flash_num].size = flash->size;
	flash_info[flash_num].sector_count = flash->size / flash->sector_size;
	flash_info[flash_num].start[0] = 0;

	/* flash memory map */
	buf = map_physmem(CONFIG_START_VIRTUAL_ADDRESS, CONFIG_JFFS2_PART_SIZE, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return NULL;
	}

	ret = spi_flash_read(flash, CONFIG_JFFS2_PART_OFFSET, CONFIG_JFFS2_PART_SIZE, buf);
	if (ret) {
		printf("SF: spi_flash_read %s\n", ret ? "ERROR" : "OK");
		return NULL;
	}

	flash_info[flash_num].start[0] = (unsigned long*)buf;

	return buf;
}

void norflash_memory_map_deinit(void *buf)
{
	unmap_physmem(buf, CONFIG_JFFS2_PART_SIZE);
}
