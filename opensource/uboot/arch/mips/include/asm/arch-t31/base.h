/*
 * T31 REG_BASE definitions
 *
 * Copyright (c) 2019 Ingenic Semiconductor Co.,Ltd
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

#ifndef __BASE_H__
#define __BASE_H__

/*
 * Define the module base addresses
 */

/* AHB0 BUS Devices Base */
#define HARB0_BASE	    0xb3000000
#define DDRC_BASE	    0xb34f0000
#define DDR_PHY_OFFSET	(-0x4e0000 + 0x1000)
#define X2D_BASE	    0xb3030000		// T31 delete
#define GPU_BASE	    0xb3040000		// T31 delete
#define LCDC0_BASE	    0xb3050000
#define CIM_BASE	    0xb3060000		// T31 delete
#define COMPRESS_BASE	0xb3070000	// T31 delete
#define IPU0_BASE	    0xb3080000
#define GPVLC_BASE	    0xb3090000		// T31 delete
/*#define LCDC1_BASE	0xb30a0000*/
#define IPU1_BASE	    0xb30b0000		// T31 delete
#define MONITOR_BASE	0xb30f0000	// T31 delete

/* AHB1 BUS Devices Base */
#define SCH_BASE	    0xb3200000
#define	VDMA_BASE	    0xb3210000
#define	EFE_BASE	    0xb3240000
#define	MCE_BASE	    0xb3250000
#define	DBLK_BASE	    0xb3270000
#define	VMAU_BASE	    0xb3280000
#define	SDE_BASE	    0xb3290000
#define	AUX_BASE	    0xb32a0000
#define	TCSM_BASE	    0xb32c0000
#define	JPGC_BASE	    0xb32e0000
#define	SRAM_BASE	    0xb32f0000

/* AHB2 BUS Devices Base */
#define HARB2_BASE	0xb3400000
#define NEMC_BASE	0xb3410000
#define PDMA_BASE	0xb3420000
#define AES_BASE	0xb3430000
#define SFC_BASE	0xb3440000
#define MSC0_BASE	0xb3450000
#define MSC1_BASE	0xb3460000
#define MSC2_BASE	0xb3470000 	// T31 delete
//#define GPS_BASE	0xb3480000
#define HASH_BASE	0xb3480000	// hash
#define EHCI_BASE	0xb3490000	// T31 delete
#define OHCI_BASE	0xb34a0000	// T31 delete
#define ETHC_BASE	0xb34b0000  // gmac
#define BCH_BASE	0xb34d0000	// T31 delete
#define TSSI0_BASE	0xb34e0000	// T31 delete
#define TSSI1_BASE	0xb34f0000	// T31 delete
#define OTG_BASE	0xb3500000
#define EFUSE_BASE	0xb3540000

#define	OST_BASE	0xb2000000
#define	HDMI_BASE	0xb0180000	// T31 delete

/* APB BUS Devices Base */
#define	CPM_BASE	0xb0000000
#define	INTC_BASE	0xb0001000
#define	TCU_BASE	0xb0002000
#define	RTC_BASE	0xb0003000
#define	GPIO_BASE	0xb0010000
#define	AIC0_BASE	0xb0020000
//#define	AIC1_BASE	0xb0021000	//codec
#define	CODEC_BASE	0xb0021000	//codec
#define	UART0_BASE	0xb0030000
#define	UART1_BASE	0xb0031000
#define	UART2_BASE	0xb0032000
#define	UART3_BASE	0xb0033000	// T31 delete
//#define	UART4_BASE	0xb0034000	// T31 delete
#define	DMIC_BASE	0xb0034000	// T31 add
#define	SSC_BASE	0xb0040000	// ssi_slv
#define	SSI0_BASE	0xb0043000
#define	SSI1_BASE	0xb0044000
#define	I2C0_BASE	0xb0050000  // smb0
#define	I2C1_BASE	0xb0051000  // smb1
#define	I2C2_BASE	0xb0052000	// T31 delete
#define	I2C3_BASE	0xb0053000	// T31 delete
#define	I2C4_BASE	0xb0054000	// T31 delete
//#define	KMC_BASE	0xb0060000	// usb-phy
#define	USBPHY_BASE	0xb0060000	// usb-phy
#define	DES_BASE	0xb0061000
#define	SADC_BASE	0xb0070000
#define	PCM0_BASE	0xb0071000	// T31 delete
//#define	OWI_BASE	0xb0072000	//DTRNG
#define	DTRNG_BASE	0xb0072000	// DTRNG
#define	PCM1_BASE	0xb0074000	// T31 delete
#define	WDT_BASE	0xb0002000	// TCU == WDT

/* NAND CHIP Base Address*/
#define NEMC_CS1_BASE 0xbb000000
#define NEMC_CS2_BASE 0xba000000
#define NEMC_CS3_BASE 0xb9000000
#define NEMC_CS4_BASE 0xb8000000
#define NEMC_CS5_BASE 0xb7000000
#define NEMC_CS6_BASE 0xb6000000

#endif /* __BASE_H__ */
