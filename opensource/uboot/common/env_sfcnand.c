/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>

static ulong env_offset = CONFIG_ENV_OFFSET;

#define ACTIVE_FLAG	1
#define OBSOLETE_FLAG	0

#define READ_OPS	0
#define WRITE_OPS	1
#define ERASE_OPS	2
#define X_COMMAND_LENGTH 128

DECLARE_GLOBAL_DATA_PTR;

char *env_name_spec = "SFC NAND Flash";

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT /* double env */
static ulong env_new_offset	= CONFIG_ENV_OFFSET_REDUND;
//static struct spi_flash *env_flash;

static int sfc_nand_ops(int ops,u32 offset,size_t len,void *buf)
{
	char command[X_COMMAND_LENGTH];
	int ret;
	char *cmd;
	if(ops == READ_OPS)
		cmd = "read";
	else if(ops == WRITE_OPS)
		cmd = "write";
	else
		cmd = "erase";

	memset(command,0,X_COMMAND_LENGTH);
	if(ops == ERASE_OPS)
		sprintf(command,"nand %s 0x%x 0x%x",cmd,offset,len);
	else
		sprintf(command,"nand %s.jffs2 0x%x 0x%x 0x%x",cmd,(unsigned int)buf,offset,len);

	ret = run_command(command,0);
	if(ret)
		printf("do spinand read error ! please check your param !!\n");

	return ret;
}
int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res, *saved_buffer = NULL;
	u32	saved_size, saved_offset, sector = 1;
	int	ret;

	if (gd->env_valid == 1) {
		env_new_offset = CONFIG_ENV_OFFSET_REDUND;
		env_offset = CONFIG_ENV_OFFSET;
	} else {
		env_new_offset = CONFIG_ENV_OFFSET;
		env_offset = CONFIG_ENV_OFFSET_REDUND;
	}

	ret = sfc_nand_ops(READ_OPS,env_offset,sizeof(env_t),&env_new);
	if (ret) {
		printf("ERROR: Failed to read environment %d !\n", ret);
	}
	env_new.flags++;

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc	= crc32(0, env_new.data, ENV_SIZE);

	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = env_new_offset + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer) {
			ret = 1;
			goto done;
		}
		ret = sfc_nand_ops(READ_OPS,saved_offset,saved_size,saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	puts("Erasing SPI NAND flash...");
	ret = sfc_nand_ops(ERASE_OPS,env_new_offset,sector * CONFIG_ENV_SECT_SIZE,NULL);
	if (ret)
		goto done;

	puts("Writing to SPI NAND flash...");

	ret = sfc_nand_ops(WRITE_OPS,env_new_offset,CONFIG_ENV_SIZE,&env_new);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = sfc_nand_ops(WRITE_OPS,saved_offset,saved_size,saved_buffer);
		if (ret)
			goto done;
	}

	puts("done\n");

	gd->env_valid = gd->env_valid == 2 ? 1 : 2;

	printf("Valid environment: %d\n", (int)gd->env_valid);

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

void env_relocate_spec(void)
{
	int ret;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1 = NULL;
	env_t *tmp_env2 = NULL;
	env_t *ep = NULL;

	tmp_env1 = (env_t *)malloc(CONFIG_ENV_SIZE);
	tmp_env2 = (env_t *)malloc(CONFIG_ENV_SIZE);

	if (!tmp_env1 || !tmp_env2) {
		set_default_env("!malloc() failed");
		goto out;
	}

	ret = sfc_nand_ops(READ_OPS,CONFIG_ENV_OFFSET,CONFIG_ENV_SIZE,tmp_env1);
	if (ret) {
		set_default_env("!spi_flash_read() failed");
		goto err_read;
	}
	if (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc)
		crc1_ok = 1;

	ret = sfc_nand_ops(READ_OPS,CONFIG_ENV_OFFSET_REDUND,CONFIG_ENV_SIZE,tmp_env2);
	if (ret) {
		set_default_env("!spi_flash_read() failed");
		goto err_read;
	}
	if (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc)
		crc2_ok = 1;

	if (!crc1_ok && !crc2_ok) {
		set_default_env("!bad CRC");
		goto err_read;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = 1;
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = 2;
	} else {
		if (tmp_env1->flags == 0XFF && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if ((tmp_env2->flags == 0XFF && tmp_env1->flags == 0) ||
				tmp_env1->flags >= tmp_env2->flags)
			gd->env_valid = 1;
		else
			gd->env_valid = 2;
	}

	if (gd->env_valid == 1)
		ep = tmp_env1;
	else
		ep = tmp_env2;

	ret = env_import((char *)ep, 0);
	if (!ret) {
		error("Cannot import environment: errno = %d\n", errno);
		set_default_env("env_import failed");
	}

err_read:
out:
	free(tmp_env1);
	free(tmp_env2);
}
#else
// static struct spi_flash *env_flash;

static int sfc_nand_ops(int ops, u32 offset, size_t len, void *buf)
{
	char command[X_COMMAND_LENGTH];
	int ret;
	char *cmd;
	if (ops == READ_OPS)
		cmd = "read";
	else if (ops == WRITE_OPS)
		cmd = "write";
	else
		cmd = "erase";

	memset(command, 0, X_COMMAND_LENGTH);
	if (ops == ERASE_OPS)
		sprintf(command, "nand %s 0x%x 0x%x", cmd, offset, len);
	else
		sprintf(command, "nand %s.jffs2 0x%x 0x%x 0x%x", cmd, (unsigned int)buf, offset, len);

	ret = run_command(command, 0);
	if (ret)
		printf("do spinand read error ! please check your param !!\n");

	return 0;
}
int saveenv(void)
{
	env_t env_new;
	ssize_t len;
	env_t *tmp_tmp = NULL;
	char *res, *saved_buffer = NULL, flag = OBSOLETE_FLAG;
	u32 saved_size, saved_offset, sector = 1;
	int ret;
	int env_size = ENV_SIZE;

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0)
	{
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc = crc32(0, (uint8_t *)env_new.data, ENV_SIZE);

	// printf("env_new.crc is %x\n", env_new.crc);
	if (gd->env_valid == 1)
	{
		env_offset = CONFIG_ENV_OFFSET;
	}
	else
	{
		printf("error! only one env!\n");
		return 1;
	}

	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE)
	{
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = env_offset + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer)
		{
			ret = 1;
			goto done;
		}
		ret = sfc_nand_ops(READ_OPS, saved_offset, saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE)
	{
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	printf("Erasing SPI NAND flash...");
	ret = sfc_nand_ops(ERASE_OPS, env_offset, sector * CONFIG_ENV_SECT_SIZE, NULL);

	if (ret)
		goto done;

	printf("Writing to SPI NAND flash...");

	ret = sfc_nand_ops(WRITE_OPS, env_offset, CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE)
	{
		ret = sfc_nand_ops(WRITE_OPS, saved_offset, saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	puts("done\n");

	gd->env_valid = 1;

	printf("Valid environment: %d\n", (int)gd->env_valid);

done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

void env_relocate_spec(void)
{
	int ret;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1 = NULL;
	env_t *ep = NULL;

	tmp_env1 = (env_t *)malloc(CONFIG_ENV_SIZE);

	if (!tmp_env1)
	{
		set_default_env("!malloc() failed");
		goto out;
	}
	ret = sfc_nand_ops(READ_OPS, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, tmp_env1);
	if (ret)
	{
		set_default_env("!spi_flash_read() failed");
		goto err_read;
	}

	if (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc)
		crc1_ok = 1;

	if (!crc1_ok)
	{
		set_default_env("!bad CRC");
		goto err_read;
	}
	else
	{
		gd->env_valid = 1;
	}

	if (gd->env_valid == 1)
		ep = tmp_env1;
	else
		printf("env_valid error! is %d\n");

	ret = env_import((char *)ep, 0);
	if (!ret)
	{
		error("Cannot import environment: errno = %d\n", errno);
		set_default_env("env_import failed");
	}

err_read:
out:
	free(tmp_env1);
}

#endif
int env_init(void)
{
	/* SPI flash isn't usable before relocation */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}
