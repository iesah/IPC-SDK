#ifndef __JZ4785_GPIO_H__
#define __JZ4785_GPIO_H__

#define	GPIO_BASE	0xB0010000

/*************************************************************************
 * GPIO (General-Purpose I/O Ports)
 *************************************************************************/
#define MAX_GPIO_NUM	192

//n = 0,1,2,3,4,5
#define GPIO_PXPIN(n)	(GPIO_BASE + (0x00 + (n)*0x100)) /* PIN Level Register */
#define GPIO_PXINT(n)	(GPIO_BASE + (0x10 + (n)*0x100)) /* Port Interrupt Register */
#define GPIO_PXINTS(n)	(GPIO_BASE + (0x14 + (n)*0x100)) /* Port Interrupt Set Register */
#define GPIO_PXINTC(n)	(GPIO_BASE + (0x18 + (n)*0x100)) /* Port Interrupt Clear Register */
#define GPIO_PXMASK(n)	(GPIO_BASE + (0x20 + (n)*0x100)) /* Port Interrupt Mask Register */
#define GPIO_PXMASKS(n)	(GPIO_BASE + (0x24 + (n)*0x100)) /* Port Interrupt Mask Set Reg */
#define GPIO_PXMASKC(n)	(GPIO_BASE + (0x28 + (n)*0x100)) /* Port Interrupt Mask Clear Reg */
#define GPIO_PXPAT1(n)	(GPIO_BASE + (0x30 + (n)*0x100)) /* Port Pattern 1 Register */
#define GPIO_PXPAT1S(n)	(GPIO_BASE + (0x34 + (n)*0x100)) /* Port Pattern 1 Set Reg. */
#define GPIO_PXPAT1C(n)	(GPIO_BASE + (0x38 + (n)*0x100)) /* Port Pattern 1 Clear Reg. */
#define GPIO_PXPAT0(n)	(GPIO_BASE + (0x40 + (n)*0x100)) /* Port Pattern 0 Register */
#define GPIO_PXPAT0S(n)	(GPIO_BASE + (0x44 + (n)*0x100)) /* Port Pattern 0 Set Register */
#define GPIO_PXPAT0C(n)	(GPIO_BASE + (0x48 + (n)*0x100)) /* Port Pattern 0 Clear Register */
#define GPIO_PXFLG(n)	(GPIO_BASE + (0x50 + (n)*0x100)) /* Port Flag Register */
#define GPIO_PXFLGC(n)	(GPIO_BASE + (0x54 + (n)*0x100)) /* Port Flag clear Register */
#define GPIO_PXOEN(n)	(GPIO_BASE + (0x60 + (n)*0x100)) /* Port Output Disable Register */
#define GPIO_PXOENS(n)	(GPIO_BASE + (0x64 + (n)*0x100)) /* Port Output Disable Set Register */
#define GPIO_PXOENC(n)	(GPIO_BASE + (0x68 + (n)*0x100)) /* Port Output Disable Clear Register */
#define GPIO_PXPEN(n)	(GPIO_BASE + (0x70 + (n)*0x100)) /* Port Pull Disable Register */
#define GPIO_PXPENS(n)	(GPIO_BASE + (0x74 + (n)*0x100)) /* Port Pull Disable Set Register */
#define GPIO_PXPENC(n)	(GPIO_BASE + (0x78 + (n)*0x100)) /* Port Pull Disable Clear Register */
#define GPIO_PXDS(n)	(GPIO_BASE + (0x80 + (n)*0x100)) /* Port Drive Strength Register */
#define GPIO_PXDSS(n)	(GPIO_BASE + (0x84 + (n)*0x100)) /* Port Drive Strength set Register */
#define GPIO_PXDSC(n)	(GPIO_BASE + (0x88 + (n)*0x100)) /* Port Drive Strength clear Register */

#define REG_GPIO_PXPIN(n)	REG32(GPIO_PXPIN((n)))  /* PIN level */
#define REG_GPIO_PXINT(n)	REG32(GPIO_PXINT((n)))  /* 1: interrupt pending */
#define REG_GPIO_PXINTS(n)	REG32(GPIO_PXINTS((n)))
#define REG_GPIO_PXINTC(n)	REG32(GPIO_PXINTC((n)))
#define REG_GPIO_PXMASK(n)	REG32(GPIO_PXMASK((n)))   /* 1: mask pin interrupt */
#define REG_GPIO_PXMASKS(n)	REG32(GPIO_PXMASKS((n)))
#define REG_GPIO_PXMASKC(n)	REG32(GPIO_PXMASKC((n)))
#define REG_GPIO_PXPAT1(n)	REG32(GPIO_PXPAT1((n)))   /* 1: disable pull up/down */
#define REG_GPIO_PXPAT1S(n)	REG32(GPIO_PXPAT1S((n)))
#define REG_GPIO_PXPAT1C(n)	REG32(GPIO_PXPAT1C((n)))
#define REG_GPIO_PXPAT0(n)	REG32(GPIO_PXPAT0((n)))  /* 0:GPIO/INTR, 1:FUNC */
#define REG_GPIO_PXPAT0S(n)	REG32(GPIO_PXPAT0S((n)))
#define REG_GPIO_PXPAT0C(n)	REG32(GPIO_PXPAT0C((n)))
#define REG_GPIO_PXFLG(n)	REG32(GPIO_PXFLG((n))) /* 0:GPIO/Fun0,1:intr/fun1*/
#define REG_GPIO_PXFLGC(n)	REG32(GPIO_PXFLGC((n)))
#define REG_GPIO_PXOEN(n)	REG32(GPIO_PXOEN((n)))
#define REG_GPIO_PXOENS(n)	REG32(GPIO_PXOENS((n))) /* 0:input/low-level-trig/falling-edge-trig, 1:output/high-level-trig/rising-edge-trig */
#define REG_GPIO_PXOENC(n)	REG32(GPIO_PXOENC((n)))
#define REG_GPIO_PXPEN(n)	REG32(GPIO_PXPEN((n)))
#define REG_GPIO_PXPENS(n)	REG32(GPIO_PXPENS((n))) /* 0:Level-trigger/Fun0, 1:Edge-trigger/Fun1 */
#define REG_GPIO_PXPENC(n)	REG32(GPIO_PXPENC((n)))
#define REG_GPIO_PXDS(n)	REG32(GPIO_PXDS((n)))
#define REG_GPIO_PXDSS(n)	REG32(GPIO_PXDSS((n))) /* interrupt flag */
#define REG_GPIO_PXDSC(n)	REG32(GPIO_PXDSC((n))) /* interrupt flag */

#define GPIO_GOS        0x100

#define GPIO_PXFUNS_OFFSET      (0x44)  /*  w, 32, 0x???????? */
#define GPIO_PXSELC_OFFSET      (0x58)  /*  w, 32, 0x???????? */
#define GPIO_PXPES_OFFSET       (0x34)  /*  w, 32, 0x???????? */
#define GPIO_PXFUNC_OFFSET      (0x48)  /*  w, 32, 0x???????? */
#define GPIO_PXPEC_OFFSET       (0x38)  /*  w, 32, 0x???????? */
#define GPIO_PXIMC_OFFSET       (0x28)  /*  w, 32, 0x???????? */
#define GPIO_PXDATC_OFFSET      (0x18)  /*  w, 32, 0x???????? */

#define GPIO_PXDATC(n)  (GPIO_BASE + (n)*GPIO_GOS + GPIO_PXDATC_OFFSET)
#define GPIO_PXIMC(n)   (GPIO_BASE + (n)*GPIO_GOS + GPIO_PXIMC_OFFSET)
#define GPIO_PXPEC(n)   (GPIO_BASE + (n)*GPIO_GOS + GPIO_PXPEC_OFFSET)
#define GPIO_PXFUNC(n)  (GPIO_BASE + (n)*GPIO_GOS + GPIO_PXFUNC_OFFSET)

#define GPIO_PXFUNS(n)  (GPIO_BASE + (n)*GPIO_GOS + GPIO_PXFUNS_OFFSET)
#define GPIO_PXSELC(n)  (GPIO_BASE + (n)*GPIO_GOS + GPIO_PXSELC_OFFSET)
#define GPIO_PXPES(n)   (GPIO_BASE + (n)*GPIO_GOS + GPIO_PXPES_OFFSET)

#define REG_GPIO_PXFUNS(n)      REG32(GPIO_PXFUNS(n))
#define REG_GPIO_PXFUNC(n)      REG32(GPIO_PXFUNC(n))
#define REG_GPIO_PXSELC(n)      REG32(GPIO_PXSELC(n))
#define REG_GPIO_PXPES(n)       REG32(GPIO_PXPES(n))
#define REG_GPIO_PXPEC(n)       REG32(GPIO_PXPEC(n))

//----------------------------------------------------------------
// Function Pins Mode

#define __gpio_as_func0(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXFUNS(p) = (1 << o);	\
		REG_GPIO_PXTRGC(p) = (1 << o);	\
		REG_GPIO_PXSELC(p) = (1 << o);	\
	} while (0)

#define __gpio_as_func1(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXFUNS(p) = (1 << o);	\
		REG_GPIO_PXTRGC(p) = (1 << o);	\
		REG_GPIO_PXSELS(p) = (1 << o);	\
	} while (0)

#define __gpio_as_func2(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXFUNS(p) = (1 << o);	\
		REG_GPIO_PXTRGS(p) = (1 << o);	\
		REG_GPIO_PXSELC(p) = (1 << o);	\
	} while (0)

#define __gpio_as_func3(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXFUNS(p) = (1 << o);	\
		REG_GPIO_PXTRGS(p) = (1 << o);	\
		REG_GPIO_PXSELS(p) = (1 << o);	\
	} while (0)

/*
 * MII_TXD0- D3 MII_TXEN MII_TXCLK MII_COL
 * MII_RXER MII_RXDV MII_RXCLK MII_RXD0 - D3
 * MII_CRS MII_MDC MII_MDIO
 */

#define __gpio_as_eth()					\
	do {						\
		REG_GPIO_PXINTC(1) =  0x00000010;	\
		REG_GPIO_PXMASKC(1) = 0x00000010;	\
		REG_GPIO_PXPAT1S(1) = 0x00000010;	\
		REG_GPIO_PXPAT0C(1) = 0x00000010;	\
		REG_GPIO_PXINTC(3) =  0x3c000000;	\
		REG_GPIO_PXMASKC(3) = 0x3c000000;	\
		REG_GPIO_PXPAT1C(3) = 0x3c000000;	\
		REG_GPIO_PXPAT0S(3) = 0x3c000000;	\
		REG_GPIO_PXINTC(5) =  0xfff0;		\
		REG_GPIO_PXMASKC(5) = 0xfff0;		\
		REG_GPIO_PXPAT1C(5) = 0xfff0;		\
		REG_GPIO_PXPAT0C(5) = 0xfff0;		\
	} while (0)

/*
 * UART0_TxD, UART0_RxD
 */
#define __gpio_as_uart0()				\
	do {						\
		REG_GPIO_PXINTC(5) = 0x00000009;	\
		REG_GPIO_PXMASKC(5) = 0x00000009;	\
		REG_GPIO_PXPAT1C(5) = 0x00000009;	\
		REG_GPIO_PXPAT0C(5) = 0x00000009;	\
	} while (0)

/*
 * UART0_TxD, UART0_RxD, UART0_CTS, UART0_RTS
 */
#define __gpio_as_uart0_ctsrts()			\
	do {						\
		REG_GPIO_PXFUNS(5) = 0x0000000f;	\
		REG_GPIO_PXTRGC(5) = 0x0000000f;	\
		REG_GPIO_PXSELC(5) = 0x0000000f;	\
		REG_GPIO_PXPES(5) = 0x0000000f;		\
	} while (0)
/*
 * GPS_CLK GPS_MAG GPS_SIG
 */
#define __gpio_as_gps()					\
	do {						\
		REG_GPIO_PXFUNS(5) = 0x00000007;	\
		REG_GPIO_PXTRGC(5) = 0x00000007;	\
		REG_GPIO_PXSELS(5) = 0x00000007;	\
		REG_GPIO_PXPES(5) = 0x00000007;		\
	} while (0)

/*
 * UART1_TxD, UART1_RxD
 */
#define __gpio_as_uart1()				\
	do {						\
		REG_GPIO_PXINTC(3)  = 0x14000000;	\
		REG_GPIO_PXMASKC(3) = 0x14000000;	\
		REG_GPIO_PXPAT1C(3) = 0x14000000;	\
		REG_GPIO_PXPAT0C(3) = 0x14000000;	\
	} while (0)

/*
 * UART1_TxD, UART1_RxD, UART1_CTS, UART1_RTS
 */
#define __gpio_as_uart1_ctsrts()			\
	do {						\
		REG_GPIO_PXFUNS(3) = 0x3c000000;	\
		REG_GPIO_PXTRGC(3) = 0x3c000000;	\
		REG_GPIO_PXSELC(3) = 0x3c000000;	\
		REG_GPIO_PXPES(3)  = 0x3c000000;	\
	} while (0)

/*
 * UART2_TxD, UART2_RxD
 */
#define __gpio_as_uart2()				\
	do {						\
		REG_GPIO_PXINTC(2) = 0x50000000;	\
		REG_GPIO_PXMASKC(2) = 0x50000000;	\
		REG_GPIO_PXPAT1C(2) = 0x50000000;	\
		REG_GPIO_PXPAT0C(2)  = 0x50000000;	\
	} while (0)

/*
 * UART2_TxD, UART2_RxD, UART2_CTS, UART2_RTS
 */
#define __gpio_as_uart2_ctsrts()			\
	do {						\
		REG_GPIO_PXFUNS(2) = 0xf0000000;	\
		REG_GPIO_PXTRGC(2) = 0xf0000000;	\
		REG_GPIO_PXSELC(2) = 0xf0000000;	\
		REG_GPIO_PXPES(2)  = 0xf0000000;	\
	} while (0)

/*
 * UART3_TxD, UART3_RxD
 */
#if 0
#define __gpio_as_uart3()				\
	do {						\
		REG_GPIO_PXINTC(4)  = (0x01<<5);        \
		REG_GPIO_PXMASKC(4)  = (0x01<<5);	\
		REG_GPIO_PXPAT1C(4) = (0x01<<5);        \
		REG_GPIO_PXPAT0S(4) = (0x01<<5);        \
		REG_GPIO_PXINTC(3)  = (0x01<<12);       \
		REG_GPIO_PXMASKC(3)  = (0x01<<12);	\
		REG_GPIO_PXPAT1C(3) = (0x01<<12);       \
		REG_GPIO_PXPAT0C(3) = (0x01<<12);       \
		REG_GPIO_PXINTC(0)  = (0x03<<30);       \
		REG_GPIO_PXMASKC(0)  = (0x03<<30);	\
		REG_GPIO_PXPAT1C(0) = (0x03<<30);       \
		REG_GPIO_PXPAT0C(0) = (0x01<<30);       \
		REG_GPIO_PXPAT0S(0) = (0x01<<31);       \
	} while (0)
#else
#define __gpio_as_uart3()				\
        do {						\
		REG_GPIO_PXINTC(3)  = (0x01<<12);       \
		REG_GPIO_PXMASKS(3)  = (0x01<<12);	\
		REG_GPIO_PXPAT1S(3) = (0x01<<12);       \
		REG_GPIO_PXPAT0C(3) = (0x01<<12);       \
		REG_GPIO_PXINTC(0)  = (0x03<<30);       \
		REG_GPIO_PXMASKC(0)  = (0x03<<30);	\
		REG_GPIO_PXPAT1C(0) = (0x03<<30);       \
		REG_GPIO_PXPAT0C(0) = (0x01<<30);       \
		REG_GPIO_PXPAT0S(0) = (0x01<<31);       \
        } while (0)
#endif
/*
 * UART3_TxD, UART3_RxD, UART3_CTS, UART3_RTS
 */
#define __gpio_as_uart3_ctsrts()			\
	do {						\
		REG_GPIO_PXFUNS(4) = 0x00000028;	\
		REG_GPIO_PXTRGC(4) = 0x00000028;	\
		REG_GPIO_PXSELS(4) = 0x00000028;	\
		REG_GPIO_PXFUNS(4) = 0x00000300;	\
		REG_GPIO_PXTRGC(4) = 0x00000300;	\
		REG_GPIO_PXSELC(4) = 0x00000300;	\
		REG_GPIO_PXPES(4)  = 0x00000328;	\
	}

#define __gpio_as_uart4()				\
	do {						\
		REG_GPIO_PXINTC(2)  = 0x00100400;	\
		REG_GPIO_PXMASKC(2)  = 0x00100400;      \
		REG_GPIO_PXPAT1S(2) = 0x00100400;       \
		REG_GPIO_PXPAT0C(2) = 0x00100400;       \
	} while (0)


/*
 * SD0 ~ SD7, CS1#, CLE, ALE, FRE#, FWE#, FRB#
 * @n: chip select number(1 ~ 6)
 */
#define __gpio_as_nand_8bit(n)						\
	do {								\
									\
		REG_GPIO_PXINTC(0) = 0x000c00ff; /* SD0 ~ SD7, FRE#, FWE# */ \
		REG_GPIO_PXMASKC(0) = 0x000c00ff;			\
		REG_GPIO_PXPAT1C(0) = 0x000c00ff;			\
		REG_GPIO_PXPAT0C(0) = 0x000c00ff;			\
		REG_GPIO_PXPENS(0) = 0x000c00ff;			\
									\
		REG_GPIO_PXINTC(1) = 0x00000003; /* CLE(SA2), ALE(SA3) */ \
		REG_GPIO_PXMASKC(1) = 0x00000003;			\
		REG_GPIO_PXPAT1C(1) = 0x00000003;			\
		REG_GPIO_PXPAT0C(1) = 0x00000003;			\
		REG_GPIO_PXPENS(1) = 0x00000003;			\
									\
		REG_GPIO_PXINTC(0) = 0x00200000 << ((n)-1); /* CSn */	\
		REG_GPIO_PXMASKC(0) = 0x00200000 << ((n)-1);		\
		REG_GPIO_PXPAT1C(0) = 0x00200000 << ((n)-1);		\
		REG_GPIO_PXPAT0C(0) = 0x00200000 << ((n)-1);		\
		REG_GPIO_PXPENS(0) = 0x00200000 << ((n)-1);		\
									\
		REG_GPIO_PXINTC(0) = 0x00100000; /* FRB#(input) */	\
		REG_GPIO_PXMASKS(0) = 0x00100000;			\
		REG_GPIO_PXPAT1S(0) = 0x00100000;			\
		REG_GPIO_PXPENS(0) = 0x00100000;			\
	} while (0)

#define __gpio_as_nand_16bit(n)						\
	do {								\
									\
		REG_GPIO_PXINTC(0) = 0x000cffff; /* SD0 ~ SD15, FRE#, FWE# */ \
		REG_GPIO_PXMASKC(0) = 0x000cffff;			\
		REG_GPIO_PXPAT1C(0) = 0x000cffff;			\
		REG_GPIO_PXPAT0C(0) = 0x000cffff;			\
		REG_GPIO_PXPENS(0) = 0x000cffff;			\
									\
		REG_GPIO_PXINTC(1) = 0x00000003; /* CLE(SA2), ALE(SA3) */ \
		REG_GPIO_PXMASKC(1) = 0x00000003;			\
		REG_GPIO_PXPAT1C(1) = 0x00000003;			\
		REG_GPIO_PXPAT0C(1) = 0x00000003;			\
		REG_GPIO_PXPENS(1) = 0x00000003;			\
									\
		REG_GPIO_PXINTC(0) = 0x00200000 << ((n)-1); /* CSn */	\
		REG_GPIO_PXMASKC(0) = 0x00200000 << ((n)-1);		\
		REG_GPIO_PXPAT1C(0) = 0x00200000 << ((n)-1);		\
		REG_GPIO_PXPAT0C(0) = 0x00200000 << ((n)-1);		\
		REG_GPIO_PXPENS(0) = 0x00200000 << ((n)-1);		\
									\
		REG_GPIO_PXINTC(0) = 0x00100000; /* FRB#(input) */	\
		REG_GPIO_PXMASKS(0) = 0x00100000;			\
		REG_GPIO_PXPAT1S(0) = 0x00100000;			\
		REG_GPIO_PXPENS(0) = 0x00100000;			\
	} while (0)


#define __gpio_as_ssi0()				\
	do {						\
		REG_GPIO_PXINTC(4) =  0x0007c000;	\
		REG_GPIO_PXMASKC(4) = 0x0007c000;	\
		REG_GPIO_PXPAT1C(4) = 0x0007c000;	\
		REG_GPIO_PXPAT0C(4) = 0x0007c000;	\
	} while (0)

/*
 * Toggle nDQS
 */
#define __gpio_as_nand_toggle()				\
	do {						\
		REG_GPIO_PXINTC(0) = 0x20000000;	\
		REG_GPIO_PXMASKC(0) = 0x20000000;	\
		REG_GPIO_PXPAT1C(0) = 0x20000000;	\
		REG_GPIO_PXPAT0C(0) = 0x20000000;	\
		REG_GPIO_PXPENC(0) = 0x20000000;	\
	} while (0)

/*
 * SD0 ~ SD7, SA0 ~ SA5, CS2#, RD#, WR#, WAIT#
 */
#define __gpio_as_nor()					\
	do {						\
		/* SD0 ~ SD7, RD#, WR#, CS2#, WAIT# */	\
		REG_GPIO_PXINTC(0) = 0x084300ff;	\
		REG_GPIO_PXMASKC(0) = 0x084300ff;	\
		REG_GPIO_PXPAT1C(0) = 0x084300ff;	\
		REG_GPIO_PXPAT0C(0) = 0x084300ff;	\
		REG_GPIO_PXPENS(0) = 0x084300ff;	\
		/* SA0 ~ SA5 */				\
		REG_GPIO_PXINTC(1) = 0x0000003f;	\
		REG_GPIO_PXMASKC(1) = 0x0000003f;	\
		REG_GPIO_PXPAT1C(1) = 0x0000003f;	\
		REG_GPIO_PXPAT0C(1) = 0x0000003f;	\
		REG_GPIO_PXPENS(1) = 0x0000003f;	\
	} while (0)

/*
 * LCD_D0~LCD_D7, LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_8bit()				\
	do {						\
		REG_GPIO_PXFUNS(2) = 0x000c03ff;	\
		REG_GPIO_PXTRGC(2) = 0x000c03ff;	\
		REG_GPIO_PXSELC(2) = 0x000c03ff;	\
		REG_GPIO_PXPES(2) = 0x000c03ff;		\
	} while (0)

/*
 * LCD_R3~LCD_R7, LCD_G2~LCD_G7, LCD_B3~LCD_B7,
 * LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_16bit()				\
	do {						\
		REG_GPIO_PXFUNS(2) = 0x0f8ff3f8;	\
		REG_GPIO_PXTRGC(2) = 0x0f8ff3f8;	\
		REG_GPIO_PXSELC(2) = 0x0f8ff3f8;	\
		REG_GPIO_PXPES(2) = 0x0f8ff3f8;		\
	} while (0)

/*
 * LCD_R2~LCD_R7, LCD_G2~LCD_G7, LCD_B2~LCD_B7,
 * LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_18bit()				\
	do {						\
		REG_GPIO_PXFUNS(2) = 0x0fcff3fc;	\
		REG_GPIO_PXTRGC(2) = 0x0fcff3fc;	\
		REG_GPIO_PXSELC(2) = 0x0fcff3fc;	\
		REG_GPIO_PXPES(2) = 0x0fcff3fc;		\
	} while (0)

/*
 * LCD_R0~LCD_R7, LCD_G0~LCD_G7, LCD_B0~LCD_B7,
 * LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_24bit()				\
        do {                                            \
                REG_GPIO_PXINTC(2) = 0x0fffffff;        \
                REG_GPIO_PXMASKC(2)  = 0x0fffffff;      \
                REG_GPIO_PXPAT0C(2) = 0x0fffffff;       \
                REG_GPIO_PXPAT1C(2) = 0x0fffffff;	\
        } while (0)

/*
 *  LCD_CLS, LCD_SPL, LCD_PS, LCD_REV
 */
#define __gpio_as_lcd_special()				\
	do {						\
		REG_GPIO_PXFUNS(2) = 0x0fffffff;	\
		REG_GPIO_PXTRGC(2) = 0x0fffffff;	\
		REG_GPIO_PXSELC(2) = 0x0feffbfc;	\
		REG_GPIO_PXSELS(2) = 0x00100403;	\
		REG_GPIO_PXPES(2) = 0x0fffffff;		\
	} while (0)

/*
 * CIM_D0~CIM_D7, CIM_MCLK, CIM_PCLK, CIM_VSYNC, CIM_HSYNC
 */
#define __gpio_as_cim()					\
	do {						\
		REG_GPIO_PXFUNS(1) = 0x0003ffc0;	\
		REG_GPIO_PXTRGC(1) = 0x0003ffc0;	\
		REG_GPIO_PXSELC(1) = 0x0003ffc0;	\
		REG_GPIO_PXPES(1)  = 0x0003ffc0;	\
	} while (0)

/*
 * SDATO, SDATI, BCLK, SYNC, SCLK_RSTN(gpio sepc) or
 * SDATA_OUT, SDATA_IN, BIT_CLK, SYNC, SCLK_RESET(aic spec)
 */
#define __gpio_as_aic()					\
	do {						\
		REG_GPIO_PXFUNS(4) = 0x16c00000;	\
		REG_GPIO_PXTRGC(4) = 0x02c00000;	\
		REG_GPIO_PXTRGS(4) = 0x14000000;	\
		REG_GPIO_PXSELC(4) = 0x14c00000;	\
		REG_GPIO_PXSELS(4) = 0x02000000;	\
		REG_GPIO_PXPES(4)  = 0x16c00000;	\
	} while (0)

/*
 * MSC0_CMD, MSC0_CLK, MSC0_D0 ~ MSC0_D3
 */
#define __gpio_a_as_msc0_4bit()				\
	do {						\
		REG_GPIO_PXINTC(0)  = 0x01fc0000;	\
		REG_GPIO_PXMASKC(0) = 0x01fc0000;	\
		REG_GPIO_PXPAT1C(0) = 0x01fc0000;	\
		REG_GPIO_PXPAT0S(0) = 0x01fc0000;	\
		REG_GPIO_PXPENC(0) =  0x01000000;	\
	} while (0)

#define __gpio_e_as_msc0_4bit()				\
	do {						\
		REG_GPIO_PXINTC(4)  = 0x30f00000;	\
		REG_GPIO_PXMASKC(4) = 0x30f00000;	\
		REG_GPIO_PXPAT1C(4) = 0x30f00000;	\
		REG_GPIO_PXPAT0S(4) = 0x30f00000;	\
	} while (0)

#define __gpio_e_as_msc1_4bit()				\
	do {						\
		REG_GPIO_PXINTC(4)  = 0x30f00000;	\
		REG_GPIO_PXMASKC(4) = 0x30f00000;	\
		REG_GPIO_PXPAT1S(4) = 0x30f00000;	\
		REG_GPIO_PXPAT0C(4) = 0x30f00000;	\
	} while (0)

#define __gpio_e_as_msc2_4bit()				\
	do {						\
		REG_GPIO_PXINTC(4)  = 0x30f00000;	\
		REG_GPIO_PXMASKC(4) = 0x30f00000;	\
		REG_GPIO_PXPAT1S(4) = 0x30f00000;	\
		REG_GPIO_PXPAT0C(4) = 0x30f00000;	\
	} while (0)

#define __gpio_as_msc 	__gpio_a_as_msc0_4bit /* default as msc0 4bit */

/*
 * TSCLK, TSSTR, TSFRM, TSFAIL, TSDI0~7
 */
#define __gpio_as_tssi_1()				\
	do {						\
		REG_GPIO_PXFUNS(1) = 0x0003ffc0;	\
		REG_GPIO_PXTRGC(1) = 0x0003ffc0;	\
		REG_GPIO_PXSELS(1) = 0x0003ffc0;	\
		REG_GPIO_PXPES(1)  = 0x0003ffc0;	\
	} while (0)
#define __gpio_e_as_ssi1()		\
	do {						\
		REG_GPIO_PXINTC(4)  = 0x30f00000;   \
		REG_GPIO_PXMASKC(4) = 0x30f00000;   \
		REG_GPIO_PXPAT1C(4) = 0x30f00000;   \
		REG_GPIO_PXPAT0C(4) = 0x30f00000;   \
	} while(0)

/*
 * TSCLK, TSSTR, TSFRM, TSFAIL, TSDI0~7
 */
#define __gpio_as_tssi_2()				\
	do {						\
		REG_GPIO_PXFUNS(1) = 0xfff00000;	\
		REG_GPIO_PXTRGC(1) = 0x0fc00000;	\
		REG_GPIO_PXTRGS(1) = 0xf0300000;	\
		REG_GPIO_PXSELC(1) = 0xfff00000;	\
		REG_GPIO_PXPES(1)  = 0xfff00000;	\
	} while (0)

/*
 * SSI_CE0, SSI_CE1, SSI_GPC, SSI_CLK, SSI_DT, SSI_DR
 */
#define __gpio_as_ssi()							\
	do {								\
		REG_GPIO_PXFUNS(0) = 0x002c0000; /* SSI0_CE0, SSI0_CLK, SSI0_DT	*/ \
		REG_GPIO_PXTRGS(0) = 0x002c0000;			\
		REG_GPIO_PXSELC(0) = 0x002c0000;			\
		REG_GPIO_PXPES(0)  = 0x002c0000;			\
									\
		REG_GPIO_PXFUNS(0) = 0x00100000; /* SSI0_DR */		\
		REG_GPIO_PXTRGC(0) = 0x00100000;			\
		REG_GPIO_PXSELS(0) = 0x00100000;			\
		REG_GPIO_PXPES(0)  = 0x00100000;			\
	} while (0)

/*
 * SSI_CE0, SSI_CE2, SSI_GPC, SSI_CLK, SSI_DT, SSI1_DR
 */
#define __gpio_as_ssi_1()				\
	do {						\
		REG_GPIO_PXFUNS(5) = 0x0000fc00;	\
		REG_GPIO_PXTRGC(5) = 0x0000fc00;	\
		REG_GPIO_PXSELC(5) = 0x0000fc00;	\
		REG_GPIO_PXPES(5)  = 0x0000fc00;	\
	} while (0)

/* Port B
 * SSI2_CE0, SSI2_CE2, SSI2_GPC, SSI2_CLK, SSI2_DT, SSI12_DR
 */
#define __gpio_as_ssi2_1()				\
	do {						\
		REG_GPIO_PXFUNS(5) = 0xf0300000;	\
		REG_GPIO_PXTRGC(5) = 0xf0300000;	\
		REG_GPIO_PXSELS(5) = 0xf0300000;	\
		REG_GPIO_PXPES(5)  = 0xf0300000;	\
	} while (0)

/*
 * I2C_SCK, I2C_SDA
 */
#define __gpio_as_i2c()					\
	do {						\
		REG_GPIO_PXFUNS(4) = 0x00003000;	\
		REG_GPIO_PXSELC(4) = 0x00003000;	\
		REG_GPIO_PXPES(4)  = 0x00003000;	\
	} while (0)

/*
 * I2C_SCK, I2C_SDA
 */
#define __gpio_b1_as_i2c0()				\
	do {						\
		REG_GPIO_PXINTC(1)  = 0x00000c00;	\
		REG_GPIO_PXMASKC(1) = 0x00000c00;	\
		REG_GPIO_PXPAT1C(1) = 0x00000c00;	\
		REG_GPIO_PXPAT0S(1) = 0x00000c00;	\
	} while (0)

#define __gpio_e1_as_i2c1()				\
	do {						\
		REG_GPIO_PXINTC(4)  = 0xc0000000;	\
		REG_GPIO_PXMASKC(4) = 0xc0000000;	\
		REG_GPIO_PXPAT1C(4) = 0xc0000000;	\
		REG_GPIO_PXPAT0S(4) = 0xc0000000;	\
	} while (0)

#define __gpio_d1_as_i2c0()				\
	do {						\
		REG_GPIO_PXINTC(3)  = 0xc0000000;	\
		REG_GPIO_PXMASKC(3) = 0xc0000000;	\
		REG_GPIO_PXPAT1C(3) = 0xc0000000;	\
		REG_GPIO_PXPAT0S(3) = 0xc0000000;	\
	} while (0)

#define __gpio_e1_as_i2c()				\
	do {						\
		REG_GPIO_PXINTC(4) = 0x00003000;	\
		REG_GPIO_PXMASKC(4) = 0x00003000;	\
		REG_GPIO_PXPAT1C(4)  = 0x00003000;	\
		REG_GPIO_PXPAT0S(4)  = 0x00003000;	\
	} while (0)

#define __gpio_e0_as_i2c()				\
	do {						\
		REG_GPIO_PXINTC(4) = 0xe0000000;	\
		REG_GPIO_PXMASKC(4) = 0xe0000000;	\
		REG_GPIO_PXPAT1C(4)  = 0xe0000000;	\
		REG_GPIO_PXPAT0C(4)  = 0xe0000000;	\
	} while (0)

#define __gpio_f2_as_i2c()				\
	do {						\
		REG_GPIO_PXINTC(5) = 0x00030000;	\
		REG_GPIO_PXMASKC(5) = 0x00030000;	\
		REG_GPIO_PXPAT1S(5)  = 0x00030000;	\
		REG_GPIO_PXPAT0C(5)  = 0x00030000;	\
	} while (0)



#define __gpio_as_pwm0()            \
	do {                        \
		REG_GPIO_PXINTC(4)  = 0x1;      \
		REG_GPIO_PXMASKC(4) = 0x1;      \
		REG_GPIO_PXPAT1C(4) = 0x1;      \
		REG_GPIO_PXPAT0C(4) = 0x1;      \
	} while (0)

#define __gpio_as_pwm1()            \
	do {                        \
		REG_GPIO_PXINTC(4)  = 0x2;      \
		REG_GPIO_PXMASKC(4) = 0x2;      \
		REG_GPIO_PXPAT1C(4) = 0x2;      \
		REG_GPIO_PXPAT0C(4) = 0x2;      \
	} while (0)
#define __gpio_as_pwm2()            \
	do {                        \
		REG_GPIO_PXINTC(4)  = 0x4;      \
		REG_GPIO_PXMASKC(4) = 0x4;      \
		REG_GPIO_PXPAT1C(4) = 0x4;      \
		REG_GPIO_PXPAT0C(4) = 0x4;      \
	} while (0)
#define __gpio_as_pwm3()            \
	do {                        \
		REG_GPIO_PXINTC(4)  = 0x8;      \
		REG_GPIO_PXMASKC(4) = 0x8;      \
		REG_GPIO_PXPAT1C(4) = 0x8;      \
		REG_GPIO_PXPAT0C(4) = 0x8;      \
	} while (0)
#define __gpio_as_pwm4()            \
	do {                        \
		REG_GPIO_PXINTC(4)  = 0x1<<4;      \
		REG_GPIO_PXMASKC(4) = 0x1<<4;      \
		REG_GPIO_PXPAT1C(4) = 0x1<<4;      \
		REG_GPIO_PXPAT0C(4) = 0x1<<4;      \
	} while (0)

#define __gpio_as_pwm5()            \
	do {                        \
		REG_GPIO_PXINTC(4)  = 0x1<<5;      \
		REG_GPIO_PXMASKC(4) = 0x1<<5;      \
		REG_GPIO_PXPAT1C(4) = 0x1<<5;      \
		REG_GPIO_PXPAT0C(4) = 0x1<<5;      \
	} while (0)

#define __gpio_as_pwm6()            \
	do {                        \
		REG_GPIO_PXINTC(3)  = 0x1<<10;      \
		REG_GPIO_PXMASKC(3) = 0x1<<10;      \
		REG_GPIO_PXPAT1C(3) = 0x1<<10;      \
		REG_GPIO_PXPAT0C(3) = 0x1<<10;      \
	} while (0)
#define __gpio_as_pwm7()            \
	do {                        \
		REG_GPIO_PXINTC(3)  = 0x1<<11;      \
		REG_GPIO_PXMASKC(3) = 0x1<<11;      \
		REG_GPIO_PXPAT1C(3) = 0x1<<11;      \
		REG_GPIO_PXPAT0C(3) = 0x1<<11;      \
	} while (0)
#define __gpio_as_pwm(n)	__gpio_as_pwm##n()


//-------------------------------------------
// GPIO or Interrupt Mode

#define __gpio_get_port(p)	(REG_GPIO_PXPIN(p))

#define __gpio_port_as_output0(p, o)			\
	do {						\
		REG_GPIO_PXINTC(p) = (1 << (o));	\
		REG_GPIO_PXMASKS(p) = (1 << (o));	\
		REG_GPIO_PXPAT1C(p) = (1 << (o));	\
		REG_GPIO_PXPAT0C(p) = (1 << (o));	\
	} while (0)

#define __gpio_port_as_output1(p, o)			\
	do {						\
		REG_GPIO_PXINTC(p) = (1 << (o));	\
		REG_GPIO_PXMASKS(p) = (1 << (o));	\
		REG_GPIO_PXPAT1C(p) = (1 << (o));	\
		REG_GPIO_PXPAT0S(p) = (1 << (o));	\
	} while (0)

#define __gpio_port_as_input(p, o)			\
	do {						\
		REG_GPIO_PXINTC(p) = (1 << (o));	\
		REG_GPIO_PXMASKS(p) = (1 << (o));	\
		REG_GPIO_PXPAT1S(p) = (1 << (o));	\
		REG_GPIO_PXPAT0C(p) = (1 << (o));	\
	} while (0)

#define __gpio_as_output0(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		__gpio_port_as_output0(p, o);	\
	} while (0)

#define __gpio_as_output1(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		__gpio_port_as_output1(p, o);	\
	} while (0)

#define __gpio_as_input(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		__gpio_port_as_input(p, o);	\
	} while (0)

#define __gpio_get_pin(n)				\
	({						\
		unsigned int p, o, v;			\
		p = (n) / 32;				\
		o = (n) % 32;				\
		if (__gpio_get_port(p) & (1 << o))	\
			v = 1;				\
		else					\
			v = 0;				\
		v;					\
	})

#define __gpio_set_pin            __gpio_as_output1
#define __gpio_clear_pin          __gpio_as_output0


#define __gpio_as_irq_high_level(n)		\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXIMS(p) = (1 << o);	\
		REG_GPIO_PXTRGC(p) = (1 << o);	\
		REG_GPIO_PXFUNC(p) = (1 << o);	\
		REG_GPIO_PXSELS(p) = (1 << o);	\
		REG_GPIO_PXDIRS(p) = (1 << o);	\
		REG_GPIO_PXFLGC(p) = (1 << o);	\
		REG_GPIO_PXIMC(p) = (1 << o);	\
	} while (0)

#define __gpio_as_irq_low_level(n)		\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXIMS(p) = (1 << o);	\
		REG_GPIO_PXTRGC(p) = (1 << o);	\
		REG_GPIO_PXFUNC(p) = (1 << o);	\
		REG_GPIO_PXSELS(p) = (1 << o);	\
		REG_GPIO_PXDIRC(p) = (1 << o);	\
		REG_GPIO_PXFLGC(p) = (1 << o);	\
		REG_GPIO_PXIMC(p) = (1 << o);	\
	} while (0)

#define __gpio_as_irq_rise_edge(n)		\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXIMS(p) = (1 << o);	\
		REG_GPIO_PXTRGS(p) = (1 << o);	\
		REG_GPIO_PXFUNC(p) = (1 << o);	\
		REG_GPIO_PXSELS(p) = (1 << o);	\
		REG_GPIO_PXDIRS(p) = (1 << o);	\
		REG_GPIO_PXFLGC(p) = (1 << o);	\
		REG_GPIO_PXIMC(p) = (1 << o);	\
	} while (0)

#define __gpio_as_irq_fall_edge(n)		\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXIMS(p) = (1 << o);	\
		REG_GPIO_PXTRGS(p) = (1 << o);	\
		REG_GPIO_PXFUNC(p) = (1 << o);	\
		REG_GPIO_PXSELS(p) = (1 << o);	\
		REG_GPIO_PXDIRC(p) = (1 << o);	\
		REG_GPIO_PXFLGC(p) = (1 << o);	\
		REG_GPIO_PXIMC(p) = (1 << o);	\
	} while (0)

#define __gpio_mask_irq(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXIMS(p) = (1 << o);	\
	} while (0)

#define __gpio_unmask_irq(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXIMC(p) = (1 << o);	\
	} while (0)

#define __gpio_ack_irq(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXFLGC(p) = (1 << o);	\
	} while (0)

#define __gpio_get_irq()				\
	({						\
		unsigned int p, i, tmp, v = 0;		\
		for (p = 3; p >= 0; p--) {		\
			tmp = REG_GPIO_PXFLG(p);	\
			for (i = 0; i < 32; i++)	\
				if (tmp & (1 << i))	\
					v = (32*p + i);	\
		}					\
		v;					\
	})

#define __gpio_group_irq(n)			\
	({					\
		register int tmp, i;		\
		tmp = REG_GPIO_PXFLG((n));	\
		for (i=31;i>=0;i--)		\
			if (tmp & (1 << i))	\
				break;		\
		i;				\
	})

#define __gpio_enable_pull(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXPEC(p) = (1 << o);	\
	} while (0)

#define __gpio_disable_pull(n)			\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXPES(p) = (1 << o);	\
	} while (0)

#endif
