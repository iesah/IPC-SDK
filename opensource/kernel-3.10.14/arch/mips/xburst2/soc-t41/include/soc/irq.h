/*
 * IRQ number in T41 INTC definition.
 *   Only support T41 now. 2019-7-6
 *
 * Copyright (C) 2019 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __INTC_IRQ_H__
#define __INTC_IRQ_H__

#include <irq.h>

enum {
/* interrupt controller interrupts */
/*ISR0*/
	IRQ_JPEG0=IRQ_INTC_BASE, /*0*/
	IRQ_AIC0,                    /*1*/
	IRQ_SYS_LEP,                /*2*/
    IRQ_RTC,                    /*3*/
	IRQ_LZMA,                   /*4*/
	IRQ_MIPI_RX_2L,             /*5*/
	IRQ_IPU,                    /*6*/
	IRQ_SFC0,                   /*7*/
	IRQ_SSI1,                   /*8*/
	IRQ_SSI0,                   /*9*/
	IRQ_PDMA,                   /*10*/
	IRQ_SSISLV,                 /*11*/
	IRQ_RESERVED12,             /*12*/
	IRQ_IVDC,                   /*13*/
	IRQ_GPIO3,
	IRQ_GPIO2,
	IRQ_GPIO1,
	IRQ_GPIO0,
#define IRQ_GPIO_PORT(N) (IRQ_GPIO0 - (N))
	IRQ_SADC,
	IRQ_SFC1,
	IRQ_LCD,
	IRQ_OTG,
	IRQ_HASH,
	IRQ_AES,
	IRQ_RSA,
	IRQ_TCU2,
	IRQ_TCU1,
	IRQ_TCU0,
	IRQ_RESERVED28,
	IRQ_RESERVED29,
	IRQ_S0_VIC,
	IRQ_ISP,

/*ISR1*/
	IRQ_RESERVED32,
	IRQ_DMIC,
	IRQ_DTRNG,
	IRQ_LDC,
	IRQ_MSC1,
	IRQ_MSC0,
	IRQ_AIP2,
	IRQ_AIP1,
	IRQ_AIP0,
	IRQ_DDR_DWU,            /* DDR watch unit */
	IRQ_DRAW_BOX,
	IRQ_VO,
	IRQ_HARB2,
	IRQ_I2D,
	IRQ_RESERVED46,
	IRQ_CPM,
	IRQ_UART3,
	IRQ_UART2,
	IRQ_UART1,
	IRQ_UART0,
	IRQ_DDR,
	IRQ_MON,
	IRQ_EFUSE,
	IRQ_GMAC0,
	IRQ_UART5,
	IRQ_UART4,
	IRQ_I2C2,
	IRQ_I2C1,
	IRQ_I2C0,
	IRQ_PDMAM,
	IRQ_EL200,
	IRQ_PWM,
#define IRQ_MCU_GPIO_PORT(N) (IRQ_MCU_GPIO0 + (N))
	IRQ_MCU_GPIO0,
	IRQ_MCU_GPIO1,
	IRQ_MCU_GPIO2,
	IRQ_MCU_GPIO3,
	IRQ_MCU_GPIO4,
	IRQ_MCU_GPIO5,
};
#if 0
enum {
	IRQ_OST = IRQ_OST_BASE,
};
#endif
enum {
	IRQ_MCU = IRQ_MCU_BASE,
};

#endif
