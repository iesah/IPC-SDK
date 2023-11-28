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
	{32 * 0 +  0,	GSS_INPUT_PULL},	// NC
	{32 * 0 +  1,	GSS_INPUT_PULL},	// NC
	{32 * 0 +  2,	GSS_INPUT_PULL},	// NC
	{32 * 0 +  3,	GSS_INPUT_PULL},	// NC
	{32 * 0 +  4,	GSS_INPUT_NOPULL},	// MSC0_D4 pullup
	{32 * 0 +  5,	GSS_INPUT_NOPULL},	// MSC0_D5 pullup
	{32 * 0 +  6,	GSS_INPUT_NOPULL},	// MSC0_D6 pullup
	{32 * 0 +  7,	GSS_INPUT_NOPULL},	// MSC0_D7 pullup
	{32 * 0 + 16,	GSS_INPUT_NOPULL},	// SHUTDOWN_N pulldown		twxie_fix
	{32 * 0 + 17,	GSS_INPUT_PULL	},	// NC
	{32 * 0 + 18,	GSS_OUTPUT_LOW	},	// MSC0_CLK connect
	{32 * 0 + 19,	GSS_INPUT_NOPULL },	// MSC0_CMD pullup
	{32 * 0 + 20,	GSS_INPUT_NOPULL },	// MSC0_D0 pullup
	{32 * 0 + 21,	GSS_INPUT_NOPULL },	// MSC0_D1 pullup
	{32 * 0 + 22,	GSS_INPUT_NOPULL },	// MSC0_D2 pullup
	{32 * 0 + 23,	GSS_INPUT_NOPULL },	// MSC0_D3 pullup
	{32 * 0 + 27,	GSS_INPUT_PULL	},	// NC
	{32 * 0 + 29,	GSS_INPUT_PULL	},	// NC
	{32 * 0 + 30,	GSS_IGNORE },		// WKUP_N pullup
	/* GPIO Group - B */
	{32 * 1 + 0,	GSS_INPUT_PULL	},	// NC
	{32 * 1 + 1,	GSS_INPUT_PULL 	},	// NC
	{32 * 1 + 2,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 3,	GSS_INPUT_PULL  },	// NC
	{32 * 1 + 4,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 5,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 6,	GSS_INPUT_PULL	},	// NC
	{32 * 1 + 7,	GSS_OUTPUT_LOW},	// TP_VCC_EN pulldown		twxie_fix
	{32 * 1 + 8,	GSS_OUTPUT_LOW },	// SEC_VCC_EN connect triode
	{32 * 1 + 9,	GSS_INPUT_NOPULL},	// SIM_POWER_EN	pullup mos
	{32 * 1 + 10,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 11,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 12,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 13,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 14,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 15,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 16,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 17,	GSS_INPUT_PULL},	// NC
	{32 * 1 + 20,	GSS_INPUT_NOPULL},	// TOUCH_RES connect		twxie_fix
	{32 * 1 + 21,	GSS_INPUT_NOPULL},	// AVDEFUSE_EN_N pullup mos
	{32 * 1 + 28,	GSS_INPUT_NOPULL},	// LCD_RESET_N connect		twxie_fix
	{32 * 1 + 29,	GSS_INPUT_PULL	},	// NC
	{32 * 1 + 30,	GSS_INPUT_NOPULL},	// WL_RST_N pullup 		twxie_fix
	{32 * 1 + 31,	GSS_INPUT_NOPULL},	// MOTOR_EN pulldown mos

	/* GPIO Group - C */
	{32 * 2 +  0,	GSS_INPUT_NOPULL},	// LCD_SPI_DR
	{32 * 2 +  1,	GSS_INPUT_NOPULL},	// LCD_SPI_DT
	{32 * 2 +  2,	GSS_OUTPUT_LOW	},	// LCD_B2
	{32 * 2 +  3,	GSS_OUTPUT_LOW	},	// LCD_B3
	{32 * 2 +  4,	GSS_OUTPUT_LOW	},	// LCD_B4
	{32 * 2 +  5,	GSS_OUTPUT_LOW	},	// LCD_B5
	{32 * 2 +  6,	GSS_OUTPUT_LOW	},	// LCD_B6
	{32 * 2 +  7,	GSS_OUTPUT_LOW	},	// LCD_B7
	{32 * 2 +  8,	GSS_OUTPUT_LOW	},	// LCD_PCLK
	{32 * 2 +  9,	GSS_OUTPUT_LOW	},	// LCD_DE
	{32 * 2 + 10,	GSS_INPUT_NOPULL},	// LCD_SPI_CLK
	{32 * 2 + 11,	GSS_INPUT_NOPULL},	// LCD_SPI_CE
	{32 * 2 + 12,	GSS_OUTPUT_LOW	},	// LCD_G2
	{32 * 2 + 13,	GSS_OUTPUT_LOW	},	// LCD_G3
	{32 * 2 + 14,	GSS_OUTPUT_LOW	},	// LCD_G4
	{32 * 2 + 15,	GSS_OUTPUT_LOW	},	// LCD_G5
	{32 * 2 + 16,	GSS_OUTPUT_LOW	},	// LCD_G6
	{32 * 2 + 17,	GSS_OUTPUT_LOW	},	// LCD_G7
	{32 * 2 + 18,	GSS_OUTPUT_LOW 	},	// LCD_HSYN
	{32 * 2 + 19,	GSS_OUTPUT_LOW 	},	// LCD_VSYN
	{32 * 2 + 20,	GSS_INPUT_NOPULL},	// LCD_INT_N			twxie_fix
	{32 * 2 + 21,	GSS_INPUT_NOPULL},	// LCD_RS
	{32 * 2 + 22,	GSS_OUTPUT_LOW 	},	// LCD_R2
	{32 * 2 + 23,	GSS_OUTPUT_LOW 	},	// LCD_R3
	{32 * 2 + 24,	GSS_OUTPUT_LOW 	},	// LCD_R4
	{32 * 2 + 25,	GSS_OUTPUT_LOW	},	// LCD_R5
	{32 * 2 + 26,	GSS_OUTPUT_LOW	},	// LCD_R6
	{32 * 2 + 27,	GSS_OUTPUT_LOW	},	// LCD_R7
	/* GPIO Group - D */
	{32 * 3 + 14,	GSS_OUTPUT_LOW},	// CLK32K pulldown
	{32 * 3 + 15,	GSS_INPUT_PULL},	// NC
	{32 * 3 + 17,	GSS_INPUT_NOPULL},	// bootsel0 pullup
	{32 * 3 + 18,	GSS_INPUT_NOPULL},	// bootsel1 pulldown
	{32 * 3 + 19,	GSS_INPUT_NOPULL},	// bootsel2 pullup
	{32 * 3 + 20,	GSS_INPUT_NOPULL},	// WIFI_SD0 pullup	twxie_fix
	{32 * 3 + 21,	GSS_INPUT_NOPULL},	// WIFI_SD1 pullup	twxie_fix
	{32 * 3 + 22,	GSS_INPUT_NOPULL},	// WIFI_SD2 pullup	twxie_fix
	{32 * 3 + 23,	GSS_INPUT_NOPULL},	// WIFI_SD3 pullup	twxie_fix
	{32 * 3 + 24,	GSS_INPUT_NOPULL},	// WIFI_CLK connect	twxie_fix
	{32 * 3 + 25,	GSS_INPUT_NOPULL},	// WIFI_CMD pullup	twxie_fix
	{32 * 3 + 26,	GSS_INPUT_PULL},	// NC
	{32 * 3 + 27,	GSS_INPUT_PULL},	// NC
	{32 * 3 + 28,	GSS_INPUT_PULL},	// NC
	{32 * 3 + 29,	GSS_INPUT_PULL},	// NC
	{32 * 3 + 30,	GSS_INPUT_NOPULL},	// SMB0_SDA pullup	//twxie_fix	
	{32 * 3 + 31,	GSS_INPUT_NOPULL},	// SMB0_SCK pullup	//twxie_fix
	/* GPIO Group - E */
	{32 * 4 +  0,	GSS_INPUT_NOPULL},	// SMB2_SDA pullup
	{32 * 4 +  1,	GSS_OUTPUT_LOW},	// LCD_PWM pulldown
	{32 * 4 +  2,	GSS_INPUT_PULL},	// NC
	{32 * 4 +  3,	GSS_INPUT_NOPULL},	// SMB2_SCK pullup
	{32 * 4 + 10,	GSS_INPUT_PULL},	// NC
	{32 * 4 + 20,	GSS_INPUT_PULL},	// NC
	{32 * 4 + 21,	GSS_INPUT_PULL},	// NC
	{32 * 4 + 22,	GSS_INPUT_PULL},	// NC
	{32 * 4 + 23,	GSS_INPUT_PULL},	// NC
	{32 * 4 + 28,	GSS_INPUT_PULL},	// NC
	{32 * 4 + 29,	GSS_INPUT_PULL},	// NC
	{32 * 4 + 30,	GSS_INPUT_NOPULL},	// SMB1_SDA pullup	//twxie_fix
	{32 * 4 + 31,	GSS_INPUT_NOPULL},	// SMB1_SCK pullup	//twxie_fix
	/* GPIO Group - F */
	{32 * 5 +  0,	GSS_INPUT_NOPULL},	// UART0_RXD connect	//twxie_fix
	{32 * 5 +  1,	GSS_INPUT_PULL},	// NC
	{32 * 5 +  2,	GSS_INPUT_PULL},	// NC 
	{32 * 5 +  3,	GSS_INPUT_NOPULL},	// UART0_TXD connect	//twxie_fix
	{32 * 5 +  4,	GSS_INPUT_PULL},	// NC
	{32 * 5 +  5,	GSS_INPUT_PULL},	// NC
	{32 * 5 +  6,	GSS_INPUT_PULL},	// NC
	{32 * 5 +  7,	GSS_INPUT_NOPULL},	// USB_DETE pulldown mos
	{32 * 5 +  8,	GSS_INPUT_PULL},	// NC
	{32 * 5 +  9,	GSS_INPUT_PULL},	// NC
	{32 * 5 + 10,	GSS_INPUT_NOPULL},	// CHARG_DET_N pullup		twxie_fix
	{32 * 5 + 11,	GSS_INPUT_NOPULL},	// PMU_IRQ_N pullup		twxie_fix
	{32 * 5 + 12,	GSS_INPUT_PULL},	// NC
	{32 * 5 + 13,	GSS_INPUT_NOPULL},	// SENSOR_INT1 connect		twxie_fix
	{32 * 5 + 14,	GSS_INPUT_NOPULL},	// SENSOR_INT2 connect		twxie_fix
	{32 * 5 + 15,	GSS_INPUT_PULL},	// NC
	/* GPIO Group - G */
	{32 * 6 +  6,	GSS_INPUT_PULL},	// NC
	{32 * 6 +  7,	GSS_INPUT_PULL},	// NC
	{32 * 6 +  8,	GSS_INPUT_PULL},	// NC
	{32 * 6 +  9,	GSS_INPUT_PULL},	// NC
	{32 * 6 + 10,	GSS_INPUT_PULL},	// NC
	{32 * 6 + 11,	GSS_INPUT_PULL},	// NC
	{32 * 6 + 12,	GSS_INPUT_PULL},	// NC
	{32 * 6 + 13,	GSS_INPUT_PULL},	// NC
	{32 * 6 + 14,	GSS_INPUT_PULL},	// NC
	{32 * 6 + 15,	GSS_INPUT_NOPULL},	// ENAND_RST_N pullup	twxie_fix
	{32 * 6 + 16,	GSS_INPUT_PULL},	// NC
	{32 * 6 + 17,	GSS_INPUT_PULL},	// NC
	/* GPIO Group Set End */
	{GSS_TABLET_END,GSS_TABLET_END	}
};
