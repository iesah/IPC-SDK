/*
 * Copyright (c) 2006-2010  Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <gpio.h>

// default gpio state is input pull;
__initdata int gpio_ss_table[][2] = {
	{32 * 0 +  0,	GSS_INPUT_NOPULL}, /* NAND_SD0 */
	{32 * 0 +  1,	GSS_INPUT_NOPULL}, /* NAND_SD1 */
	{32 * 0 +  2,	GSS_INPUT_NOPULL}, /* NAND_SD2 */
	{32 * 0 +  3,	GSS_INPUT_NOPULL}, /* NAND_SD3 */
	{32 * 0 +  4,	GSS_INPUT_NOPULL}, /* NAND_SD4 */
	{32 * 0 +  5,	GSS_INPUT_NOPULL}, /* NAND_SD5 */
	{32 * 0 +  6,	GSS_INPUT_NOPULL}, /* NAND_SD6 */
	{32 * 0 +  7,	GSS_INPUT_NOPULL}, /* NAND_SD7 */
	{32 * 0 +  8,	GSS_INPUT_PULL}, /* NC */
	{32 * 0 +  9,	GSS_INPUT_PULL}, /* NC*/
	{32 * 0 + 10,	GSS_INPUT_PULL}, /* NC */
	{32 * 0 + 11,	GSS_INPUT_PULL}, /* NC */
	{32 * 0 + 12,	GSS_INPUT_PULL}, /* NC */
	{32 * 0 + 13,	GSS_INPUT_PULL}, /* NC */
	{32 * 0 + 14,	GSS_INPUT_PULL}, /* NC */
	{32 * 0 + 15,	GSS_INPUT_PULL}, /* NC */
	{32 * 0 + 16,	GSS_IGNORE	}, /* USB_DETE */
//	{32 * 0 + 17,	GSS_IGNORE	}, /* HP_MUTE */
	{32 * 0 + 17,	GSS_INPUT_NOPULL}, /* HP_MUTE */
	{32 * 0 + 18,	GSS_IGNORE	}, /* NF_RE*/
	{32 * 0 + 19,	GSS_IGNORE	}, /* NF_WE*/
	{32 * 0 + 20,	GSS_IGNORE	}, /* NF_RB*/
	{32 * 0 + 21,	GSS_IGNORE	}, /* NF_CS1*/
	{32 * 0 + 22,	GSS_IGNORE	}, /* NF_CS2*/
	{32 * 0 + 23,	GSS_IGNORE	}, /* NF_CS3*/
	{32 * 0 + 24,	GSS_INPUT_PULL	}, /* NC */
	{32 * 0 + 25,	GSS_INPUT_PULL	}, /* NC */
	{32 * 0 + 26,	GSS_INPUT_PULL  }, /* NC */
	{32 * 0 + 27,	GSS_INPUT_NOPULL}, /* CIM_RST_N */
	{32 * 0 + 28,	GSS_INPUT_PULL	}, /* NC */
	{32 * 0 + 29,	GSS_INPUT_PULL	}, /* NC */
	{32 * 0 + 30,	GSS_IGNORE	}, /* WAKE_UP*/
	{32 * 0 + 31,	GSS_IGNORE	}, /* ?? */
	/* GPIO Group - B */

	{32 * 1 +  0,	GSS_IGNORE  	}, /* SA0_CLE*/
	{32 * 1 +  1,	GSS_IGNORE  	}, /* SA1_CLE*/
	{32 * 1 +  2,	GSS_INPUT_NOPULL}, /* SD0_CD_N*/
	{32 * 1 +  3,	GSS_OUTPUT_LOW  }, /* SD0_VCC_EN*/

	{32 * 1 +  4,	GSS_INPUT_NOPULL}, /* GMAC_CRS*/
	{32 * 1 +  5,	GSS_INPUT_NOPULL}, /* USB_ID */
	{32 * 1 +  6,	GSS_INPUT_PULL	}, /* CIM_PD */
	{32 * 1 +  7,	GSS_INPUT_NOPULL}, /* GMAC_PHY_RESET_N*/
	{32 * 1 +  8,	GSS_INPUT_NOPULL }, /* GMAC_TXER NC*/
	{32 * 1 +  9,	GSS_INPUT_NOPULL}, /* EPD_DD_PWC/GMAC_GTXC*/
	{32 * 1 +  10,	GSS_INPUT_NOPULL}, /*GMAC_TXD4/EPD_PWR0*/
	{32 * 1 +  11,	GSS_INPUT_NOPULL}, /*GMAC_TXD5/EPD_PWR1*/
	{32 * 1 +  12,	GSS_INPUT_NOPULL}, /*GMAC_TXD6/EPD_SCE2*/
	{32 * 1 +  13,	GSS_INPUT_NOPULL}, /*GMAC_TXD7/EPD_SCE3*/
	{32 * 1 + 14,	GSS_INPUT_NOPULL}, /* GMAC_RXD4/EPD_SCE4*/
	{32 * 1 + 15,	GSS_INPUT_NOPULL}, /* GMAC_RXD5/EPD_SCE5*/
	{32 * 1 + 16,	GSS_INPUT_NOPULL}, /* GMAC_RXD6/EPD_PWR2*/
	{32 * 1 + 17,	GSS_INPUT_NOPULL}, /* GMAC_RXD7/EPD_PWR3*/
	{32 * 1 + 18,	GSS_INPUT_PULL	}, /* NC */
	{32 * 1 + 19,	GSS_INPUT_PULL	}, /* NC */
	{32 * 1 + 20,	GSS_INPUT_NOPULL}, /* EPD_SIG_CTRL_N*/
	{32 * 1 + 21,	GSS_IGNORE	}, /* PMU_IRQ_N */
	{32 * 1 + 22,	GSS_INPUT_PULL	}, /* NC */
	{32 * 1 + 23,	GSS_INPUT_PULL	}, /* NC */
	{32 * 1 + 24,	GSS_INPUT_PULL	}, /* NC */
	{32 * 1 + 25,	GSS_INPUT_PULL	}, /* NC */
	{32 * 1 + 26,	GSS_INPUT_PULL	}, /* NC */
	{32 * 1 + 28,	GSS_OUTPUT_LOW	}, /* LCD_RESET_N*/
	{32 * 1 + 29,	GSS_OUTPUT_LOW	}, /* LCD_INT */
	{32 * 1 + 30,	GSS_OUTPUT_LOW	}, /* LCD_DISP_N */
	{32 * 1 + 31,	GSS_INPUT_NOPULL}, /* RTC_IRQ_INT*/

	/* GPIO Group - C */
	{32 * 2 +  0,	GSS_OUTPUT_LOW	}, /* LCD_B0 */
	{32 * 2 +  1,	GSS_OUTPUT_LOW	}, /* LCD_B1*/
	{32 * 2 +  2,	GSS_OUTPUT_LOW	}, /* LCD_B2 */
	{32 * 2 +  3,	GSS_OUTPUT_LOW	}, /* LCD_B3 */
	{32 * 2 +  4,	GSS_OUTPUT_LOW	}, /* LCD_B4 */
	{32 * 2 +  5,	GSS_OUTPUT_LOW	}, /* LCD_B5 */
	{32 * 2 +  6,	GSS_OUTPUT_LOW	}, /* LCD_B6 */
	{32 * 2 +  7,	GSS_OUTPUT_LOW	}, /* LCD_B7 */
	{32 * 2 +  8,	GSS_OUTPUT_LOW	}, /* LCD_PCLK */
	{32 * 2 +  9,	GSS_OUTPUT_LOW	}, /* LCD_DE */
	{32 * 2 + 10,	GSS_OUTPUT_LOW	}, /* LCD_G0 */
	{32 * 2 + 11,	GSS_OUTPUT_LOW	}, /* LCD_G1 */
	{32 * 2 + 12,	GSS_OUTPUT_LOW	}, /* LCD_G2 */
	{32 * 2 + 13,	GSS_OUTPUT_LOW	}, /* LCD_G3 */
	{32 * 2 + 14,	GSS_OUTPUT_LOW	}, /* LCD_G4 */
	{32 * 2 + 15,	GSS_OUTPUT_LOW	}, /* LCD_G5 */
	{32 * 2 + 16,	GSS_OUTPUT_LOW	}, /* LCD_G6 */
	{32 * 2 + 17,	GSS_OUTPUT_LOW	}, /* LCD_G7 */
	{32 * 2 + 18,	GSS_OUTPUT_LOW 	}, /* LCD_HS */
	{32 * 2 + 19,	GSS_OUTPUT_LOW 	}, /* LCD_VS */
	{32 * 2 + 20,	GSS_OUTPUT_LOW 	}, /* LCD_R0 */
	{32 * 2 + 21,	GSS_OUTPUT_LOW 	}, /* LCD_R1 */
	{32 * 2 + 22,	GSS_OUTPUT_LOW 	}, /* LCD_R2 */
	{32 * 2 + 23,	GSS_OUTPUT_LOW 	}, /* LCD_R3 */
	{32 * 2 + 24,	GSS_OUTPUT_LOW 	}, /* LCD_R4 */
	{32 * 2 + 25,	GSS_OUTPUT_LOW	}, /* LCD_R5 */
	{32 * 2 + 26,	GSS_OUTPUT_LOW	}, /* LCD_R6 */
	{32 * 2 + 27,	GSS_OUTPUT_LOW	}, /* LCD_R7 */
	{32 * 2 + 28,	GSS_INPUT_PULL	}, /* NC */
	{32 * 2 + 29,	GSS_INPUT_PULL	}, /* NC */
	{32 * 2 + 30,	GSS_INPUT_PULL	}, /* NC */
	{32 * 2 + 31,	GSS_INPUT_PULL	}, /* NC */
	/* GPIO Group - D */
	{32 * 3 +  0,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  1,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  2,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  3,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  4,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  5,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  6,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  7,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  8,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  9,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  10,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  11,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  12,	GSS_INPUT_PULL  }, /* NC */
	{32 * 3 +  13,	GSS_INPUT_PULL  }, /* NC */
#if 0
	{32 * 3 + 14,	GSS_IGNORE}, /* CLK32 */
	{32 * 3 + 15,	GSS_IGNORE}, /* OSC_24M_OUT */
#endif
	{32 * 3 + 14,	GSS_INPUT_NOPULL}, /* CLK32 */
	{32 * 3 + 15,	GSS_INPUT_NOPULL}, /* OSC_24M_OUT */
	{32 * 3 + 16,	GSS_INPUT_PULL}, /* NC */
	{32 * 3 + 17,	GSS_INPUT_NOPULL}, /* BOOT_SEL0/VOL- */
	{32 * 3 + 18,	GSS_INPUT_NOPULL}, /* BOOT_SEL1/VOL+ */
	{32 * 3 + 19,	GSS_INPUT_NOPULL}, /* BOOT_SEL2/RETURN */
	{32 * 3 + 20,	GSS_INPUT_NOPULL}, /* WL_MSC1_D0 */
	{32 * 3 + 21,	GSS_INPUT_NOPULL}, /* WL_MSC1_D1 */
	{32 * 3 + 22,	GSS_INPUT_NOPULL}, /* WL_MSC1_D2*/
	{32 * 3 + 23,	GSS_INPUT_NOPULL}, /* WL_MSC1_D3 */
	{32 * 3 + 24,	GSS_INPUT_NOPULL}, /* WL_MSC1_CLK */
	{32 * 3 + 25,	GSS_INPUT_NOPULL}, /* WL_MSC1_CMD */
	{32 * 3 + 26,	GSS_INPUT_NOPULL}, /* GMAC_RXD2 */
	{32 * 3 + 27,	GSS_INPUT_NOPULL}, /* GMAC_RXD3 */
	{32 * 3 + 28,	GSS_INPUT_NOPULL}, /* GMAC_TXD2 */
	{32 * 3 + 29,	GSS_INPUT_NOPULL}, /* GMAC_TXD3 */
	{32 * 3 + 30,	GSS_INPUT_NOPULL}, /* I2C0_SDA */
	{32 * 3 + 31,	GSS_INPUT_NOPULL}, /* I2C0_SCK */
	/* GPIO Group - E */
	{32 * 4 +  0,	GSS_INPUT_NOPULL}, /* I2C2_SDK */
	{32 * 4 +  1,	GSS_OUTPUT_LOW}, /* LCD_PWM/LED_EN */
	{32 * 4 +  2,	GSS_INPUT_NOPULL}, /* AVDEFUSE_EN_N*/
	{32 * 4 +  3,	GSS_INPUT_NOPULL}, /* I2C2_SCK */
	{32 * 4 +  4,	GSS_INPUT_PULL  }, /* NC */
	{32 * 4 +  5,	GSS_INPUT_PULL  }, /* NC */
	{32 * 4 +  6,	GSS_INPUT_PULL  }, /* NC */
	{32 * 4 +  7,	GSS_INPUT_PULL  }, /* NC */
	{32 * 4 +  8,	GSS_INPUT_PULL  }, /* NC */
	{32 * 4 +  9,	GSS_INPUT_PULL  }, /* NC */
	{32 * 4 + 10,	GSS_INPUT_NOPULL}, /* DRVVBUS/SD0_WP_N*/
	{32 * 4 + 11,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 12,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 13,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 14,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 15,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 16,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 17,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 18,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 19,	GSS_INPUT_PULL	}, /* NC */
	{32 * 4 + 20,	GSS_INPUT_NOPULL}, /* MSC0_D0 */
	{32 * 4 + 21,	GSS_INPUT_NOPULL}, /* MSC0_D1_ */
	{32 * 4 + 22,	GSS_INPUT_NOPULL}, /* MSC0_D2 */
	{32 * 4 + 23,	GSS_INPUT_NOPULL}, /* MSC0_D3 */
	{32 * 4 + 24,	GSS_INPUT_NOPULL}, /* NC */
	{32 * 4 + 25,	GSS_INPUT_NOPULL}, /* NC */
	{32 * 4 + 26,	GSS_INPUT_NOPULL}, /* NC */
	{32 * 4 + 27,	GSS_INPUT_NOPULL}, /* NC */
	{32 * 4 + 28,	GSS_INPUT_NOPULL}, /* MSC0_CLK */
	{32 * 4 + 29,	GSS_INPUT_NOPULL}, /* MSC0_CMD */
	{32 * 4 + 30,	GSS_INPUT_NOPULL}, /* I2C1_SDA */
	{32 * 4 + 31,	GSS_INPUT_NOPULL}, /* I2C1_SCK */
	/* GPIO Group - F */
	{32 * 5 +  0,	GSS_INPUT_NOPULL}, /* UART0_RXD */
	{32 * 5 +  1,	GSS_INPUT_NOPULL}, /* UART0_CTS_N */
	{32 * 5 +  2,	GSS_INPUT_NOPULL}, /* UART0_RTS_N */
	{32 * 5 +  3,	GSS_INPUT_NOPULL}, /* UART0_TXD */
	{32 * 5 +  4,	GSS_INPUT_NOPULL}, /* GMAC_TXD0/UART2_TXD*/
	{32 * 5 +  5,	GSS_INPUT_NOPULL}, /* GMAC_TXD1/UART2_RXD*/
	{32 * 5 +  6,	GSS_INPUT_NOPULL}, /* GMAC_TXCLK */
	{32 * 5 +  7,	GSS_INPUT_NOPULL}, /* GMAC_RXCLK */
	{32 * 5 +  8,	GSS_INPUT_NOPULL}, /* GMAC_RXER/EPD_PWR4 */
	{32 * 5 +  9,	GSS_INPUT_NOPULL}, /* GMAC_RXDV/EPD_PWR5 */
	{32 * 5 + 10,	GSS_INPUT_NOPULL}, /* GMAC_RXD0/DMIC_CLK/EPD_PWR6 */
	{32 * 5 + 11,	GSS_INPUT_NOPULL}, /* GMAC_RXD1/DMIC_IN/EPD_PWR7 */
	{32 * 5 + 12,	GSS_INPUT_NOPULL}, /* GMAC_TXEN/PCM0_DO*/
	{32 * 5 + 13,	GSS_INPUT_NOPULL}, /* GMAC_MDC/PCM0_CLK */
	{32 * 5 + 14,	GSS_INPUT_NOPULL}, /* GMAC_MDIO/PCM0_SYN */
	{32 * 5 + 15,	GSS_INPUT_NOPULL}, /* GMAC_COL/PCM0_DI */
	{32 * 5 + 16,	GSS_INPUT_PULL}, /* NC */
	{32 * 5 + 17,	GSS_INPUT_PULL}, /* NC */
	{32 * 5 + 18,	GSS_INPUT_PULL}, /* NC */
	{32 * 5 + 19,	GSS_INPUT_PULL}, /* NC */
	{32 * 5 + 20,	GSS_INPUT_PULL}, /* NC */
	{32 * 5 + 21,	GSS_INPUT_PULL}, /* NC */
	{32 * 5 + 22,	GSS_INPUT_PULL}, /* NC */
	/* GPIO Group - G */
	{32 * 6 +  6,	GSS_OUTPUT_LOW}, /* CIM1_PCLK/WL_WAKE*/
	{32 * 6 +  7,	GSS_INPUT_PULL}, /* CIM1_HSYNC/BT_WAKE NC*/
	{32 * 6 +  8,	GSS_INPUT_PULL}, /* CIM1_VSYNC/BT_RST_N  NC*/
	{32 * 6 +  9,	GSS_INPUT_PULL}, /* CIM1_MCLK/BT_INT     NC*/
	{32 * 6 + 10,	GSS_INPUT_PULL}, /* CIM1_D0/SD8/WLAN_PW_EN  NC*/
	{32 * 6 + 11,	GSS_INPUT_PULL}, /* CIM1_D1/SD9/BT_REG_ON  NC*/
	{32 * 6 + 12,	GSS_INPUT_PULL}, /* CIM1_D2/SD10/DMIC_LR NC*/
	{32 * 6 + 13,	GSS_OUTPUT_LOW}, /* CIM1_D3/SD11/WL_REG_ON*/
	{32 * 6 + 14,	GSS_OUTPUT_LOW}, /* CIM1_D4/SD12/SHUTDOWN_N*/
	{32 * 6 + 15,	GSS_INPUT_NOPULL}, /* CIM1_D5/SD13/KEY0*/
	{32 * 6 + 16,	GSS_INPUT_PULL}, /* CIM1_D6/SD14/PCM_RST_N NC*/
	{32 * 6 + 17,	GSS_INPUT_PULL}, /* CIM1_D7/SD15	NC*/
	/* GPIO Group Set End */
	{GSS_TABLET_END,GSS_TABLET_END	}
};

