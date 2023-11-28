/*
 * Ingenic isvp T41 configuration
 *
 * Copyright (c) 2020 Ingenic Semiconductor Co.,Ltd
 * Author: Matthew <tong.yu@ingenic.com>
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

#ifndef __CONFIG_ISVP_T41_H__
#define __CONFIG_ISVP_T41_H__


/**
 * Basic configuration(SOC, Cache, Stack, UART, DDR).
 */
#define CONFIG_MIPS32		                        /* MIPS32 CPU core */
#define CONFIG_CPU_XBURST2
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_T41		                            /* T41 SoC */
#define CONFIG_K0BASE		0x80000000
#define CONFIG_L1CACHE_SIZE		    (32 * 1024)	    /* 32K dcache and icache */
#define CONFIG_L1CACHELINE_SIZE	    (32)		    /* 32bytes dcache and icache line size */
#define CONFIG_L1CACHE_SETS		    (128)		    /* 128 dcache and icache sets */
#define CONFIG_L1CACHE_WAYNUM		(8)		        /* 8 way dcache and icache associativity */
#define CONFIG_L1CACHE_TAG_MASK	    ((~(((CONFIG_L1CACHE_SIZE) / (CONFIG_L1CACHE_WAYNUM)) - 1)) & 0x7fffffff)
#define CONFIG_L2CACHE_REG		    ((*((volatile unsigned int *)(0xb2200060)) >> 10) & 0x7)				/* reg:0xb2200060 */
#define CONFIG_L2CACHE_SIZE		    (CONFIG_L2CACHE_REG ? ((128 << (CONFIG_L2CACHE_REG - 1)) * 1024) : 0)	/* set l2cache size */
#define CONFIG_L2CACHE_SETS		    (128)		    /* 128 l2cache sets */
#define CONFIG_L2CACHELINE_SIZE	    (64)		    /* xburst2: 64bytes l2cache line size */
#define CONFIG_L2CACHE_WAYNUM	    (8)		        /* 8 way l2cache associativity */
#define CONFIG_STACK_SIZE		    CONFIG_L1CACHE_SIZE

/*#define CONFIG_FAST_BOOT*/
#if defined(CONFIG_T41N)
/* T41N */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_550M
#else
#define APLL_1104M
#define DDR_750M
#endif
#elif defined(CONFIG_T41L) || defined(CONFIG_T41LQ)
/* T41L & T41LQ */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_550M
#else
#define APLL_1104M
#define DDR_600M
#endif
#elif defined(CONFIG_T41NQ)
/* T41NQ */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_600M
#else
#define APLL_1104M
#define DDR_700M
#endif
#elif defined(CONFIG_T41A)
/* T41A */
#define APLL_1300M
#define DDR_800M
#endif

#ifdef APLL_804M
#define CONFIG_SYS_APLL_FREQ		804000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x42) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_864M
#define CONFIG_SYS_APLL_FREQ		864000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x47) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_900M
#define CONFIG_SYS_APLL_FREQ		900000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1000M
#define CONFIG_SYS_APLL_FREQ		1000000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x7c) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x2) << 7) | ((0x3) << 4))
#elif defined APLL_1008M
#define CONFIG_SYS_APLL_FREQ		1008000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x53) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1104M
#define CONFIG_SYS_APLL_FREQ		1104000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x5b) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1200M
#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x63) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1300M
#define CONFIG_SYS_APLL_FREQ		1300000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x144) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined APLL_1400M
#define CONFIG_SYS_APLL_FREQ		1400000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x15d) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined APLL_1500M
#define CONFIG_SYS_APLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x7c) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1600M
#define CONFIG_SYS_APLL_FREQ		1600000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0xc7) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x1) << 4))
#else
#error please define APLL_FREQ
#endif

#ifdef DDR_300M
#define CONFIG_SYS_MPLL_FREQ		900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_400M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD	    (((0x63) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_450M
#define CONFIG_SYS_MPLL_FREQ		900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_500M
#define CONFIG_SYS_MPLL_FREQ		1000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x7c) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x2) << 7) | ((0x3) << 4))
#elif defined DDR_540M
#define CONFIG_SYS_MPLL_FREQ		1080000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x59) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_550M
#define CONFIG_SYS_MPLL_FREQ		1100000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x112) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_570M
#define CONFIG_SYS_MPLL_FREQ		1140000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x5e) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_600M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x63) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_650M
#define CONFIG_SYS_MPLL_FREQ		1300000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x144) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_700M
#define CONFIG_SYS_MPLL_FREQ		1400000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x15d) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_750M
#define CONFIG_SYS_MPLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x176) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_800M
#define CONFIG_SYS_MPLL_FREQ		1600000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0xc7) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x1) << 4))
#elif defined DDR_900M
#define CONFIG_SYS_MPLL_FREQ		1800000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x3) << 4))
#elif defined DDR_1000M
#define CONFIG_SYS_MPLL_FREQ		2000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0xf9) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x1) << 4))
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_VPLL_FREQ		1188000000	/*If VPLL not use mast be set 0*/
#define CONFIG_SYS_VPLL_MNOD        ((98 << 20) | (0 << 14) | (1 << 11) | (1 << 7) | (3 << 4))

#define SEL_SCLKA		2
#define SEL_CPU			1
#define SEL_H0			2
#define SEL_H2			2

#ifdef DDR_300M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_400M
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
#elif defined DDR_550M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_570M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
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
#elif defined DDR_792M
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
#elif defined DDR_828M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_840M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_852M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_864M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_876M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_888M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_900M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_1000M
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

#ifdef DDR_300M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 3)
#elif defined DDR_400M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 3)
#elif defined DDR_450M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_500M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_540M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_550M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_570M
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
#elif defined DDR_792M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_798M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_800M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_810M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_816M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_828M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_840M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_852M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_864M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_876M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_888M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_900M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_1000M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_EXTAL		    24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			    1000		/* incrementer freq */

/* CACHE */
#define CONFIG_SYS_DCACHE_SIZE		CONFIG_L1CACHE_SIZE
#define CONFIG_SYS_DCACHELINE_SIZE	CONFIG_L1CACHELINE_SIZE
#define CONFIG_SYS_DCACHE_WAYS		CONFIG_L1CACHE_WAYNUM
#define CONFIG_SYS_ICACHE_SIZE		CONFIG_L1CACHE_SIZE
#define CONFIG_SYS_ICACHELINE_SIZE	CONFIG_L1CACHELINE_SIZE
#define CONFIG_SYS_ICACHE_WAYS		CONFIG_L1CACHE_WAYNUM
#define CONFIG_SYS_CACHELINE_SIZE	CONFIG_L1CACHELINE_SIZE
#define CONFIG_BOARD_L2CACHE
#define CONFIG_SYS_L2CACHE_SIZE		CONFIG_L2CACHE_SIZE
#define CONFIG_SYS_L2CACHELINE_SIZE	CONFIG_L2CACHELINE_SIZE
#define CONFIG_SYS_L2CACHE_WAYS		CONFIG_L2CACHE_WAYNUM

/* UART */
#define CONFIG_SYS_UART_INDEX               1
#define CONFIG_BAUDRATE			            115200
#define CONFIG_SYS_UART_CONTROLLER_STEP     0x1000

#if defined(CONFIG_T41N) || defined(CONFIG_T41NQ)
/* T41N */
#define CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDR3_W631GU6NG
#elif defined(CONFIG_T41L) || defined(CONFIG_T41LQ)
/* T41L */
#define CONFIG_DDR_TYPE_DDR2
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDR2_M14D5121632A
#elif defined (CONFIG_T41A)
/* T41A */
#define CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDR3_W631GU6NG
#else
#define CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDR3_W631GU6NG
#endif

#define CONFIG_DDR_INNOPHY
/*#define CONFIG_DDR_DLL_OFF*/
#define CONFIG_DDR_CHIP_ODT
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */

#define CONFIG_DDR_HARDWARE_TRAINING

/* KGD driver strength is combined with MR1 A5 and A1 */
/* #define CONFIG_OPEN_KGD_DRIVER_STRENGTH */
#ifdef CONFIG_OPEN_KGD_DRIVER_STRENGTH
#if defined(CONFIG_T41N)
#define CONFIG_DDR_DRIVER_OUT_STRENGTH
#define CONFIG_DDR_DRIVER_OUT_STRENGTH_1 1
#define CONFIG_DDR_DRIVER_OUT_STRENGTH_0 0
#endif
#endif

/* KGD ODT is combined with RTT_Nom and RTT_WR */
/* #define CONFIG_OPEN_KGD_ODT */
#ifdef CONFIG_OPEN_KGD_ODT
#if defined(CONFIG_T41N)
#define CONFIG_DDR_CHIP_ODT_VAL
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_9 0 /* RTT_Nom_9 is MR1 A9 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_6 0 /* RTT_Nom_6 is MR1 A6 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_2 1 /* RTT_Nom_2 is MR1 A2 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_WR 0 /* RTT_WR is odt for KGD write of MR2*/
#endif
#endif

#define CONFIG_DDR_PHY_IMPEDANCE 40
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 120
#define CONFIG_DDR_AUTO_SELF_REFRESH_CNT 257

/* Device Tree Configuration*/
/*#define CONFIG_OF_LIBFDT 1*/
#ifdef CONFIG_OF_LIBFDT
#define IMAGE_ENABLE_OF_LIBFDT	1
#define CONFIG_LMB
#endif
/**
 * Boot arguments definitions.
 */
#if defined(CONFIG_T41N) || defined(CONFIG_T41NQ)
#define BOOTARGS_COMMON "console=ttyS1,115200n8 mem=48M@0x0 rmem=64M@0x3000000 nmem=16M@0x7000000"
#elif defined(CONFIG_T41L) || defined(CONFIG_T41LQ)
#define BOOTARGS_COMMON "console=ttyS1,115200n8 mem=32M@0x0 rmem=32M@0x2000000"
#elif defined(CONFIG_T41A)
#define BOOTARGS_COMMON "console=ttyS1,115200n8 mem=100M@0x0 rmem=128M@0x6400000 nmem=28M@0xE400000"
#else
#define BOOTARGS_COMMON "console=ttyS1,115200n8 mem=64M@0x0 rmem=64M@0x4000000"
#endif

#if defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND)
#define CDT_OFF	1 /*1:not use CDT;0:use CDT*/
#define CONFIG_SPL_SFC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPI_SPL_CHECK
#if CDT_OFF
#define CONFIG_JZ_SFC
#else
#define CONFIG_JZ_SFC_CDT
#endif /* endif CDT_OFF */
#define CONFIG_SPL_PAD_TO_BLOCK
#define CONFIG_SPL_VERSION     1
#ifdef CONFIG_SPL_SFC_NOR
#define CONFIG_SFC_NOR
#define CONFIG_SFC_NOR_COMMAND
#else
#define CONFIG_SFC_NAND
#define CONFIG_SFC_NAND_COMMAND
#endif /* defined CONFIG_SPL_SFC_NOR */
#endif /* defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND) */

#ifdef CONFIG_FAST_BOOT
#ifdef CONFIG_SPL_MMC_SUPPORT
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc root=/dev/mmcblk0p2 rw rootdelay=1"
#elif defined(CONFIG_SFC_NOR)
#ifdef CONFIG_OF_LIBFDT
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=sfc0_nor:256k(boot),2560k(kernel),2048k(root),64k(dtb),-(appfs) lpj=11968512 quiet"
#else
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=sfc0_nor:256k(boot),2560k(kernel),2048k(root),-(appfs) lpj=11968512 quiet"
#endif
#elif defined(CONFIG_SFC_NAND)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw mtdparts=sfc0_nand:1M(uboot),3M(kernel),20M(root),-(appfs) lpj=11968512 quiet"
#endif
#else
#ifdef CONFIG_SPL_MMC_SUPPORT
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc root=/dev/mmcblk0p2 rw rootdelay=1"
#elif defined(CONFIG_SFC_NOR)
#ifdef CONFIG_OF_LIBFDT
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=sfc0_nor:256k(boot),2560k(kernel),2048k(root),64k(dtb),-(appfs) lpj=11968512"
#else
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=sfc0_nor:256k(boot),2560k(kernel),2048k(root),-(appfs) lpj=11968512"
#endif
#elif defined(CONFIG_SFC_NAND)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw mtdparts=sfc0_nand:1M(uboot),3M(kernel),20M(root),-(appfs) lpj=11968512"
#endif
#endif

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_BOOTCOMMAND "mmc read 0x80600000 0x1800 0x3000; bootm 0x80600000"
#endif  /* CONFIG_SPL_MMC_SUPPORT */

#ifdef CONFIG_SFC_NOR
#ifdef CONFIG_OF_LIBFDT
	#define CONFIG_BOOTCOMMAND "sf0 probe;sf0 read 0x80600000 0x40000 0x280000;sf0 read 0x83000000 0x540000 0x10000;bootm 0x80600000 - 0x83000000"
#else
	#define CONFIG_BOOTCOMMAND "sf0 probe;sf0 read 0x80600000 0x40000 0x280000;bootm 0x80600000"
#endif
#endif /* CONFIG_SFC_NOR */

#ifdef CONFIG_SFC_NAND
	#define CONFIG_BOOTCOMMAND "nand read 0x80600000 0x100000 0x300000;bootm 0x80600000"
#endif /* CONFIG_SFC_NAND */

/**
 * Drivers configuration.
 */
/* MMC */
#if defined(CONFIG_JZ_MMC_MSC0) || defined(CONFIG_JZ_MMC_MSC1)
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_JZ_SDHCI
#define CONFIG_MMC_SPL_PARAMS
#endif

#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PB_4BIT
#endif

#ifdef CONFIG_JZ_MMC_MSC1
#define CONFIG_JZ_MMC_SPLMSC 1
#define CONFIG_JZ_MMC_MSC1_PC
#endif

#ifdef CONFIG_SFC_COMMAND/* SD card start */
#define CONFIG_SFC_NOR_COMMAND			/* support nor command */
#define CONFIG_SFC0_NOR
/*#define CONFIG_SFC1_NOR*/

/*#define CONFIG_SFC_NAND_COMMAND*/		/* support nand command */
/*#define CONFIG_SFC0_NAND*/
/*#define CONFIG_SFC1_NAND*/
#endif /* CONFIG_SFC_COMMAND */

/* SFC */
#ifdef CONFIG_SFC_NOR_COMMAND
#define CONFIG_SFC_MIN_ALIGN			0x1000	/*0x1000->4K Erase,0x8000->32k 0x10000->64k*/
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_JZ_SFC
#define CONFIG_SFC_NOR
#define CONFIG_SPI_FLASH_INGENIC
#define CONFIG_CMD_TFTPDOWNLOAD			1	/* tftpdownload support */
/*#define CONFIG_SPI_QUAD*/					/* set nor quad mode,default:Standard mode */
#endif /* CONFIG_SFC_NOR_COMMAND */

#ifdef CONFIG_SFC_NAND_COMMAND
#define CONFIG_NAND_BURNER
#define CONFIG_SPIFLASH_PART_OFFSET     (26 * 1024)
#define CONFIG_SPI_NAND_BPP				(2048 + 64)  /*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB				(64)        /*Page Per Block*/
#define CONFIG_SFC_NAND_RATE			50000000
#define CONFIG_MTD_SFCNAND
#define CONFIG_CMD_SFCNAND
#define CONFIG_CMD_NAND
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_BASE            0xb3441000
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SPI_STANDARD				/* set nand quad mode,default:Standard mode */
#if 0 /* ubi/ubifs/mtdparts cmd, default: off */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define MTDIDS_DEFAULT                  "nand0=nand"
#define MTDPARTS_DEFAULT                "mtdparts=nand:1M(uboot),3M(kernel),20M(root),-(appfs)"
#endif
#endif /* CONFIG_SFC_NAND_COMMAND */

/* SFC GPIO init */
#if defined(CONFIG_SFC0_NOR) || defined(CONFIG_SFC0_NAND)
#define CONFIG_JZ_SFC0_PA
#endif /* CONFIG_SFC0_NOR || CONFIG_SFC0_NAND */
#if defined(CONFIG_SFC1_NOR) || defined(CONFIG_SFC1_NAND)
#define CONFIG_JZ_SFC1_PA
#endif /* CONFIG_SFC1_NOR || CONFIG_SFC1_NAND */

/*UART LOAD*/
#ifdef CONFIG_UART_LOAD
#define CONFIG_YMODEM_NO_PRINTF     /*When using ymodem, there should be no other print information*/
#define CONFIG_SPL_YMODEM_SUPPORT   /*UART download supported protocols.Ymodem*/
#endif

/* PMU */
#define CONFIG_REGULATOR
#define CONFIG_PMU_D2041

/* GMAC */
#define GMAC_PHY_MII	1
#define GMAC_PHY_RMII	2
#define GMAC_PHY_GMII	3
#define GMAC_PHY_RGMII	4

#define PHY_TYPE_DM9161      1
#define PHY_TYPE_88E1111     2
#define PHY_TYPE_DP83867     3
#define PHY_TYPE_LAN8720     4

#define CONFIG_NET_GMAC_PHY_MODE    GMAC_PHY_RMII
#define CONFIG_NET_PHY_TYPE         PHY_TYPE_LAN8720

#define CONFIG_NET_GMAC

#define CONFIG_GPIO_PHY_RESET	GPIO_PB(28)
#define CONFIG_GPIO_PHY_RESET_ENLEVEL	0

/* DEBUG ETHERNET */
#define CONFIG_SERVERIP			193.169.4.2
#define CONFIG_IPADDR			193.169.4.151
#define CONFIG_GATEWAYIP        193.169.4.1
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_ETHADDR          00:11:22:56:96:69

/* GPIO */
#define CONFIG_JZ_GPIO

/* OST */
#define CONFIG_SYS_OST_FREQ         12500000        /* OST_FREQ 12.5MHz */

/**
 * Command configuration.
 */
#define CONFIG_CMD_WATCHDOG	/* watchdog support */
#define CONFIG_CMD_NET		/* networking support			*/
#define CONFIG_CMD_PING
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_FAT		/* FAT support			*/
/*#define CONFIG_CMD_L2CACHE*/	/* allcate l2cache support */
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
/* #define CONFIG_CMD_I2C */
/* #define CONFIG_AUTO_UPDATE */
#ifdef CONFIG_AUTO_UPDATE
#define CONFIG_CMD_SDUPDATE		1
#endif

/************************ USB CONFIG ***************************/
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_DWC2
#define CONFIG_USB_DWC2_REG_ADDR 0x13500000
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
/* #define CONFIG_USB_STORAGE */
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
#define CONFIG_HARD_LZMA_CHANNEL 0

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE	0 /* init flash_base as 0 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R	1

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL)

#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#if defined(CONFIG_SFC_NAND) || defined(CONFIG_SFC_NAND_COMMAND)
#define CONFIG_SYS_MONITOR_LEN      (400 * 1024)
#else
#ifdef CONFIG_OF_LIBFDT /* support device tree */
#define CONFIG_SYS_MONITOR_LEN		(230 * 1024)
#else
#define CONFIG_SYS_MONITOR_LEN		(214 * 1024)
#endif
#endif /* CONFIG_SFC_NAND || CONFIG_SFC_NAND_COMMOD */

#define CONFIG_SYS_MALLOC_LEN		(32 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

/* #define DBG_USE_UNCACHED_MEMORY */
#ifdef DBG_USE_UNCACHED_MEMORY
#define CONFIG_SYS_SDRAM_BASE		0xA0000000 /* uncached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0xB0000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0xA8000000
#define CONFIG_SYS_MEMTEST_START	0xA0000000
#define CONFIG_SYS_MEMTEST_END		0xA8000000
#define CONFIG_SYS_TEXT_BASE		0xA0100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#else /* DBG_USE_UNCACHED_MEMORY */
#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x88000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#endif /* DBG_USE_UNCACHED_MEMORY */

/**
 * SPL configuration
 */
#define CONFIG_SPL_FRAMEWORK

#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif

/*#define CONFIG_BIG_SPL*/
#ifdef CONFIG_BIG_SPL
#define CONFIG_SPL_MAX_SIZE     102400  /* 100KB */
#define CONFIG_SPL_PAD_TO       102400 /* equal to spl max size in T15g */
#elif  defined (CONFIG_SPL_SFC_NAND)
#define CONFIG_SPL_MAX_SIZE     27648
#define CONFIG_SPL_PAD_TO		27648 /* equal to spl max size in T15g */
#else
#define CONFIG_SPL_MAX_SIZE		26624	/* 26624=26KB*/
#define CONFIG_SPL_PAD_TO		26624	/* equal to spl max size in T15g */
#endif

#define CONFIG_UBOOT_OFFSET     CONFIG_SPL_MAX_SIZE /* equal to spl max size in T15g */

#define CONFIG_MMC_RAW_UBOOT_OFFSET	(CONFIG_UBOOT_OFFSET / 1024 + 17)	/* offset = spl size + 17k*/
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	(CONFIG_MMC_RAW_UBOOT_OFFSET * 2) /* mmc offset = (spl size + 17k) * 2 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400 /* 512 KB */

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_CORE_VOLTAGE		1100

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_TEXT_BASE		0xba000000
#else
#define CONFIG_SPL_TEXT_BASE		0x80001000
#endif	/*CONFIG_SPL_NOR_SUPPORT*/

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
#endif

/**
 * Environment
 */
#ifdef CONFIG_ENV_IS_IN_MMC
#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_SYS_MMC_ENV_DEV		0
#endif
#ifdef CONFIG_JZ_MMC_MSC1
#define CONFIG_SYS_MMC_ENV_DEV      1
#endif
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#elif CONFIG_ENV_IS_IN_SPI_FLASH /* nor env */
#define CONFIG_ENV_SECT_SIZE	(1024 * 16)
#define CONFIG_ENV_SIZE			(1024 * 16)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_UBOOT_OFFSET)
#elif CONFIG_ENV_IS_IN_SFC_NAND /* nand env */
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K*/
#define SPI_NAND_BLK            0x20000 /*the spi nand block size */
#define CONFIG_ENV_SIZE         SPI_NAND_BLK /* uboot is 1M but the last block size is the env*/
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT /* double env partition ; If there is only one env partition, please annotate it.*/
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET       0xc0000 /* offset is 768k */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#else
#define CONFIG_ENV_OFFSET       0xe0000 /* offset is 896k */
#endif
#endif /* endif CONFIG_ENV_IS_IN_MMC */

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

/*
 * JFFS2 configuration
 */
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

#endif /* __CONFIG_ISVP_T41_H__ */
