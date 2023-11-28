/*
 * Ingenic isvp T31 configuration
 *
 * Copyright (c) 2019  Ingenic Semiconductor Co.,Ltd
 * Author: Damon <jiansheng.zhang@ingenic.com>
 * Based on: include/configs/urboard.h
 *           Written by Paul Burton <paul.burton@imgtec.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_ISVP_T31_H__
#define __CONFIG_ISVP_T31_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_T31		/* T31 SoC */
/* #define CONFIG_DDR_AUTO_SELF_REFRESH */
#define CONFIG_SPL_DDR_SOFT_TRAINING

#if defined(CONFIG_LITE_VERSION)
#define APLL_1008M
#define DDR_500M
#elif defined(CONFIG_DDR2_128M)
#define APLL_1392M
#define DDR_600M
#elif defined(CONFIG_T31A)
#define APLL_1392M
#define DDR_750M
#else
/* T31N */
#define APLL_1400M
#define DDR_500M
#endif

#ifdef APLL_804M
#define CONFIG_SYS_APLL_FREQ		804000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((67 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_864M
#define CONFIG_SYS_APLL_FREQ		864000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((72 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_900M
#define CONFIG_SYS_APLL_FREQ		900000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((75 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1000M
#define CONFIG_SYS_APLL_FREQ		1000000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (3 << 11) | (1<<8))
#elif defined APLL_1008M
#define CONFIG_SYS_APLL_FREQ		1008000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((84 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1080M
#define CONFIG_SYS_APLL_FREQ		1080000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((90 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1200M
#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1392M
#define CONFIG_SYS_APLL_FREQ		1392000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((116 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1400M
#define CONFIG_SYS_APLL_FREQ		1404000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((117 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1500M
#define CONFIG_SYS_APLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1800M
#define CONFIG_SYS_APLL_FREQ		1800000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((150 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#else
#error please define APLL_FREQ
#endif

#ifdef DDR_400M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_450M
#define CONFIG_SYS_MPLL_FREQ		900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((75 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_500M
#define CONFIG_SYS_MPLL_FREQ		1000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((125 << 20) | (1 << 14) | (3 << 11) | (1<<8))
#elif defined DDR_540M
#define CONFIG_SYS_MPLL_FREQ		1080000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((90 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_600M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_650M
#define CONFIG_SYS_MPLL_FREQ		1308000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((109 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_700M
#define CONFIG_SYS_MPLL_FREQ		1404000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((117 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_750M
#define CONFIG_SYS_MPLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((125 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_762M
#define CONFIG_SYS_MPLL_FREQ		1524000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((127 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_774M
#define CONFIG_SYS_MPLL_FREQ		1548000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((129 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_786M
#define CONFIG_SYS_MPLL_FREQ		1572000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((131 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_798M
#define CONFIG_SYS_MPLL_FREQ		1596000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((133 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_800M
#define CONFIG_SYS_MPLL_FREQ		1608000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((134 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_810M
#define CONFIG_SYS_MPLL_FREQ		1620000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((135 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_816M
#define CONFIG_SYS_MPLL_FREQ		1632000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((136 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_900M
#define CONFIG_SYS_MPLL_FREQ		1800000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((150 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_VPLL_FREQ		1200000000	/*If VPLL not use mast be set 0*/
#define SEL_SCLKA		2
#define SEL_CPU			1
#define SEL_H0			2
#define SEL_H2			2

#ifdef DDR_400M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_450M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_500M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_540M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_600M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_650M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_700M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_750M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_762M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_774M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_786M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_798M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_800M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_810M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_816M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_900M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#else
#error please define DDR_FREQ
#endif

#define DIV_L2			2
#define DIV_CPU			1
#define CONFIG_SYS_CPCCR_SEL		(((SEL_SCLKA & 3) << 30)			\
									 | ((SEL_CPU & 3) << 28)			\
									 | ((SEL_H0 & 3) << 26)				\
									 | ((SEL_H2 & 3) << 24)				\
									 | (((DIV_PCLK - 1) & 0xf) << 16)	\
									 | (((DIV_H2 - 1) & 0xf) << 12)		\
									 | (((DIV_H0 - 1) & 0xf) << 8)		\
									 | (((DIV_L2 - 1) & 0xf) << 4)		\
									 | (((DIV_CPU - 1) & 0xf) << 0))

#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		CONFIG_SYS_APLL_FREQ

#ifdef DDR_400M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 3)
#elif defined DDR_450M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_500M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_540M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_600M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_650M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_700M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_750M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_762M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_774M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_786M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_798M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_800M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_810M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_816M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_900M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_ICACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX		1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_UART_CONTROLLER_STEP		0x1000

/*#define CONFIG_DDR_TEST_CPU
#define CONFIG_DDR_TEST*/
#define CONFIG_DDR_INNOPHY
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_FORCE_SELECT_CS1
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDRC_CTRL_PDT DDRC_CTRL_PDT_128

#ifdef CONFIG_DDR2_128M
#define CONFIG_DDR2_M14D1G1664A
#define CONFIG_DDR_TYPE_DDR2
#elif defined(CONFIG_DDR3_128M)
#define CONFIG_DDR3_M15T1G1664A_2C
#define CONFIG_DDR_TYPE_DDR3
#else
#define CONFIG_DDR2_M14D5121632A
#define CONFIG_DDR_TYPE_DDR2
#endif
#define DDR2_CHIP_DRIVER_OUT_STRENGTH 0
#define DDR2_CHIP_MR0_DLL_RST

#define CONFIG_DDR_PHY_IMPEDANCE 40000
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 50000 //75000
/*#define CONFIG_DDR_PHY_IMPED_PULLUP	0xf*/
/*#define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xf*/

/* #define CONFIG_DDR_DLL_OFF */

/*#define CONFIG_DDR_CHIP_ODT*/
/*#define CONFIG_DDR_PHY_ODT*/
/*#define CONFIG_DDR_PHY_DQ_ODT*/
/*#define CONFIG_DDR_PHY_DQS_ODT*/

/**
 * Boot arguments definitions.
 */
#if (defined(CONFIG_DDR2_128M) || defined(CONFIG_DDR3_128M))
#define BOOTARGS_COMMON "console=ttyS1,115200n8 mem=64M@0x0 rmem=64M@0x4000000"
#else
#define BOOTARGS_COMMON "console=ttyS1,115200n8 mem=42M@0x0 rmem=22M@0x2a00000"
#endif

#if defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND)
#define CONFIG_SPL_SFC_SUPPORT
#define CONFIG_JZ_SFC
#define CONFIG_SPL_VERSION     1
#ifdef CONFIG_SPL_SFC_NOR
#define CONFIG_SFC_NOR
#else
#define CONFIG_SFC_NAND
#endif
#define CONFIG_SPI_DUAL
/*#define CONFIG_SPI_QUAD*/
#endif /* defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND) */

#ifdef CONFIG_SPL_MMC_SUPPORT
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc root=/dev/mmcblk0p2 rw rootdelay=1"
#elif defined(CONFIG_SFC_NOR)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=jz_sfc:256k(boot),2560k(kernel),2048k(root),-(appfs)"
#elif defined(CONFIG_SFC_NAND)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=off init=/linuxrc ubi.mtd=2 root=ubi0:rootfs ubi.mtd=3 rootfstype=ubifs rw"
#endif

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_BOOTCOMMAND "mmc read 0x80600000 0x1800 0x3000; bootm 0x80600000"
#endif

#ifdef CONFIG_SFC_NOR
	#define CONFIG_BOOTCOMMAND "sf probe;sf read 0x80600000 0x40000 0x280000; bootm 0x80600000"
#endif /* CONFIG_SFC_NOR */

#ifdef CONFIG_SFC_NAND
	#define CONFIG_BOOTCOMMAND "sfcnand read 0x100000 0x400000 0x80600000 ;bootm 0x80600000"
#endif

/**
 * Drivers configuration.
 */
/* MMC */
#if defined(CONFIG_JZ_MMC_MSC0) || defined(CONFIG_JZ_MMC_MSC1)
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC			1
#endif  /* JZ_MMC_MSC0 || JZ_MMC_MSC1 */

#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PB 1
#endif

#if 1
#ifdef CONFIG_SFC_COMMOND
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_JZ_SFC_PA
#define CONFIG_JZ_SFC
#define CONFIG_SFC_NOR
#define CONFIG_SPI_FLASH_INGENIC
#define CONFIG_SPI_DUAL
/*#define CONFIG_SPI_QUAD*/
#endif
#endif

/* SFC */
#define CONFIG_SFC_MIN_ALIGN 0x1000  /*0x1000->4K Erase,0x8000->32k 0x10000->64k*/
#if defined(CONFIG_SPL_SFC_SUPPORT)
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_JZ_SFC_PA
#ifdef CONFIG_SPL_SFC_NAND
#define CONFIG_SFC_NAND_RATE    100000000
#define CONFIG_SPIFLASH_PART_OFFSET     ( 26 * 1024)
#define CONFIG_SPI_NAND_BPP     (2048 +64)  /*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB     (64)        /*Page Per Block*/
#define CONFIG_MTD_SFCNAND
#define CONFIG_CMD_SFCNAND
#define CONFIG_CMD_NAND
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_BASE            0xb3441000
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT                  "nand0=nand"
#define MTDPARTS_DEFAULT                "mtdparts=nand:1M(boot),8M(kernel),40M(rootfs),-(data)"
#if 1
#define CONFIG_SPI_STANDARD //if the nand is QUAD mode, please annotate it. the default is one lan.
#endif
/*SFCNAND env*/
/* spi nand environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE 0x20000 /* 128K*/
#define SPI_NAND_BLK            0x20000 /*the spi nand block size */
#define CONFIG_ENV_SIZE         SPI_NAND_BLK /* uboot is 1M but the last block size is the env*/
#define CONFIG_ENV_OFFSET       0xc0000 /* offset is 768k */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_IS_IN_SFC_NAND

/* MTD support */
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_UBOOT_OFFSET     CONFIG_SPL_MAX_SIZE/*(26  * 1024)*/

#elif defined(CONFIG_SPL_SFC_NOR)
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_INGENIC
#define CONFIG_SPI_FLASH
#define CONFIG_UBOOT_OFFSET     CONFIG_SPL_MAX_SIZE/*(26  * 1024)*/
#endif
#endif /* CONFIG_SPL_SFC_SUPPORT */

#ifdef CONFIG_NORFLASH_32M
#define CONFIG_SPI_FLASH_BAR
#endif

/* MTD */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE

/* GMAC */
#define GMAC_PHY_MII	1
#define GMAC_PHY_RMII	2
#define GMAC_PHY_GMII	3
#define GMAC_PHY_RGMII	4

#define CONFIG_NET_GMAC_PHY_MODE GMAC_PHY_RMII

#define PHY_TYPE_DM9161      1
#define PHY_TYPE_88E1111     2
#define PHY_TYPE_8710A     3
#define PHY_TYPE_IP101G     4

#define CONFIG_NET_PHY_TYPE   PHY_TYPE_IP101G

#define CONFIG_NET_GMAC


#define CONFIG_GPIO_IP101G_RESET	GPIO_PB(28)
#define CONFIG_GPIO_IP101G_RESET_ENLEVEL	0

/* DEBUG ETHERNET */
#define CONFIG_SERVERIP		193.169.4.2
#define CONFIG_IPADDR		193.169.4.81
#define CONFIG_GATEWAYIP        193.169.4.1
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_ETHADDR          00:d0:d0:00:95:27

/* GPIO */
#define CONFIG_JZ_GPIO

/**
 * Command configuration.
 */
#ifdef CONFIG_SFC_NOR
#define CONFIG_CMD_TFTPDOWNLOAD     1 /* tftpdownload support */
#endif
#define CONFIG_CMD_WATCHDOG	/* watchdog support */
#define CONFIG_CMD_NET		/* networking support			*/
#define CONFIG_CMD_PING
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/

#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_FAT		/* FAT support			*/
/*#define CONFIG_CMD_JFFS2*/	/* JFFS2 support        */
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_MMC		/* MMC/SD support			*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_GETTIME
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
/*#define CONFIG_CMD_I2C*/

/************************ USB CONFIG ***************************/
#undef CONFIG_CMD_USB
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_DWC2
#define CONFIG_USB_DWC2_REG_ADDR 0x13500000
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
/* #define CONFIG_USB_STORAGE */
#endif

/*#define CONFIG_AUTO_UPDATE			1*/
#ifdef CONFIG_AUTO_UPDATE
	#define CONFIG_CMD_SDUPDATE		1
#endif

/**
 * Serial download configuration
 */
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */

/**
 * Miscellaneous configurable options
 */
#define CONFIG_DOS_PARTITION

#define CONFIG_LZO
#define CONFIG_RBTREE
#define CONFIG_LZMA

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE	0 /* init flash_base as 0 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R	1

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL)

#define CONFIG_SYS_MAXARGS 64
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#ifdef CONFIG_SFC_NAND
#define CONFIG_SYS_MONITOR_LEN      (512 << 10)
#else
#define CONFIG_SYS_MONITOR_LEN		(214 * 1024)
#endif
#define CONFIG_SYS_MALLOC_LEN		(32 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x84000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x82000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x84000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

/**
 * Environment
 */
#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#elif CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE	(1024 * 16)
#define CONFIG_ENV_SIZE			(1024 * 16)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_UBOOT_OFFSET)
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK

#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else /* CONFIG_SPL_NOR_SUPPORT */
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif /* CONFIG_SPL_NOR_SUPPORT */

#ifdef CONFIG_SPL_SFC_NAND
#define CONFIG_SPL_PAD_TO       27648 /* equal to spl max size in M200 */
#else
#define CONFIG_SPL_PAD_TO		26624 /* equal to spl max size in M200 */
#endif

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	86//0x5A //wli changed 0x20 /* 16KB offset */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400 /* 512 KB */

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_TEXT_BASE		0xba000000
#else
#define CONFIG_SPL_TEXT_BASE		0x80001000
#endif	/*CONFIG_SPL_NOR_SUPPORT*/

#ifdef CONFIG_SPL_SFC_NAND
#define CONFIG_SPL_MAX_SIZE     (27 * 1024)
#else
#define CONFIG_SPL_MAX_SIZE		(26 * 1024)
#endif

#undef CONFIG_SPL_LZOP
#ifdef CONFIG_SPL_LZOP
	#define CONFIG_DECMP_BUFFER_ADRS        0x80200000
#endif

#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#endif /* CONFIG_SPL_MMC_SUPPORT */

#ifdef CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SYS_SPI_BOOT_FREQ	1000000
#endif /* CONFIG_SPL_SPI_SUPPORT */

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPL_TEXT_BASE + CONFIG_SPL_PAD_TO - 0x40)	//0x40 = sizeof (image_header)
#define CONFIG_SYS_OS_BASE		0
#define CONFIG_SYS_SPL_ARGS_ADDR	0
#define CONFIG_SYS_FDT_BASE		0
#endif /* CONFIG_SPL_NOR_SUPPORT */

/**
 * GPT configuration
 */
#ifdef CONFIG_GPT_CREATOR
#define CONFIG_GPT_TABLE_PATH	"$(TOPDIR)/board/$(BOARDDIR)"
#else
/* USE MBR + zero-GPT-table instead if no gpt table defined*/
#define CONFIG_MBR_P0_OFF	64mb
#define CONFIG_MBR_P0_END	556mb
#define CONFIG_MBR_P0_TYPE 	linux

#define CONFIG_MBR_P1_OFF	580mb
#define CONFIG_MBR_P1_END 	1604mb
#define CONFIG_MBR_P1_TYPE 	linux

#define CONFIG_MBR_P2_OFF	28mb
#define CONFIG_MBR_P2_END	58mb
#define CONFIG_MBR_P2_TYPE 	linux

#define CONFIG_MBR_P3_OFF	1609mb
#define CONFIG_MBR_P3_END	7800mb
#define CONFIG_MBR_P3_TYPE 	fat
#endif

/* JFFS2 configuration */
#ifdef CONFIG_CMD_JFFS2
#define CONFIG_CMD_FLASH
#define CONFIG_SYS_MAX_FLASH_BANKS 1
#define CONFIG_SYS_MAX_FLASH_SECT 256
#undef CONFIG_CMD_MTDPARTS
#undef CONFIG_JFFS2_CMDLINE
#define COFIG_JFFS2_DEV "nor0"
#define CONFIG_JFFS2_PART_OFFSET        0x4C0000
#define CONFIG_JFFS2_PART_SIZE          0xB40000
#define CONFIG_START_VIRTUAL_ADDRESS    0x80600000
#else
#define CONFIG_SYS_MAX_FLASH_SECT 0
#endif

#endif /*__CONFIG_ISVP_H__*/
