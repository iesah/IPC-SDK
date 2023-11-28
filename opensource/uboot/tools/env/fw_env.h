/*
 * (C) Copyright 2002-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/* Pull in the current config to define the default environment */
#ifndef __ASSEMBLY__
#define __ASSEMBLY__ /* get only #defines from config.h */
#include <config.h>
#undef	__ASSEMBLY__
#else
#include <config.h>
#endif

/*
 * To build the utility with the static configuration
 * comment out the next line.
 * See included "fw_env.config" sample file
 * for notes on configuration.
 */
/*#define CONFIG_FILE     "/system/etc/fw_env.config"*/

#ifndef CONFIG_FILE
#ifdef CONFIG_SPL_SFC_NAND
/*#define HAVE_REDUND*/ /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd0"
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define DEVICE2_NAME      "/dev/mtd0"
#define HAVE_REDUND /* For systems with 2 env sectors */
#endif

#define DEVICE1_OFFSET     CONFIG_ENV_OFFSET
#define ENV1_SIZE          CONFIG_ENV_SIZE
#define DEVICE1_ESIZE      SPI_NAND_BLK
#define DEVICE1_ENVSECTORS (ENV1_SIZE/DEVICE1_ESIZE)

#ifdef DEVICE2_NAME
#define DEVICE2_OFFSET     CONFIG_ENV_OFFSET_REDUND
#define ENV2_SIZE          CONFIG_ENV_SIZE
#define DEVICE2_ESIZE      SPI_NAND_BLK
#define DEVICE2_ENVSECTORS (ENV2_SIZE/DEVICE2_ESIZE)
#endif

#else
/*#define HAVE_REDUND*/ /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd0" /* env partition */
#define DEVICE1_OFFSET    CONFIG_ENV_OFFSET
#define ENV1_SIZE         CONFIG_ENV_SIZE
#define DEVICE1_ESIZE     0x8000
#define DEVICE1_ENVSECTORS     1

#if 0
#define DEVICE2_NAME      "/dev/mtd2"
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x68000
#define DEVICE2_ESIZE     0x8000
#define DEVICE2_ENVSECTORS     2
#endif
#endif
#endif

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE		115200
#endif

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"
#endif

extern int   fw_printenv(int argc, char *argv[]);
extern char *fw_getenv  (char *name);
extern int fw_setenv  (int argc, char *argv[]);
extern int fw_parse_script(char *fname);
extern int fw_env_open(void);
extern int fw_env_write(char *name, char *value);
extern int fw_env_close(void);

extern unsigned	long  crc32	 (unsigned long, const unsigned char *, unsigned);
