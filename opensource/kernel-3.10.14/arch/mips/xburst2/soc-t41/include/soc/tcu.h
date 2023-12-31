/*
 * tcu register definition.
 *
 * Copyright (C) 2010 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __TCU_H__
#define __TCU_H__

#define TCU_TER		(0x10)   /* Timer Counter Enable Register */
#define TCU_TESR	(0x14)   /* Timer Counter Enable Set Register */
#define TCU_TECR	(0x18)   /* Timer Counter Enable Clear Register */
#define TCU_TSR		(0x1C)   /* Timer Stop Register */
#define TCU_TFR		(0x20)   /* Timer Flag Register */
#define TCU_TFSR	(0x24)   /* Timer Flag Set Register */
#define TCU_TFCR	(0x28)   /* Timer Flag Clear Register */
#define TCU_TSSR	(0x2C)   /* Timer Stop Set Register */
#define TCU_TMR		(0x30)   /* Timer Mask Register */
#define TCU_TMSR	(0x34)   /* Timer Mask Set Register */
#define TCU_TMCR	(0x38)   /* Timer Mask Clear Register */
#define TCU_TSCR	(0x3C)   /* Timer Stop Clear Register */
#define TCU_TFR0	(0x40)   /* Timer Data Full Reg */
#define TCU_THR0	(0x44)   /* Timer Data Half Reg */
#define TCU_TCT0	(0x48)   /* Timer Counter Reg */
#define TCU_TSR0	(0x4C)   /* Timer Control Reg */
#define TCU_TFR1	(0x50)   /* Timer Data Full Reg */
#define TCU_THR1	(0x54)   /* Timer Data Half Reg */
#define TCU_TCT1	(0x58)   /* Timer Counter Reg */
#define TCU_TSR1	(0x5C)   /* Timer Control Reg */
#define TCU_TFR2	(0x60)   /* Timer Data Full Reg */
#define TCU_THR2	(0x64)   /* Timer Data Half Reg */
#define TCU_TCT2	(0x68)   /* Timer Counter Reg */
#define TCU_TSR2	(0x6C)   /* Timer Control Reg */
#define TCU_TFR3	(0x70)   /* Timer Data Full Reg */
#define TCU_THR3	(0x74)   /* Timer Data Half Reg */
#define TCU_TCT3	(0x78)   /* Timer Counter Reg */
#define TCU_TSR3	(0x7C)   /* Timer Control Reg */
#define TCU_TSTR	(0xF0)   /* Timer Status Register,Only Used In Tcu2 Mode */
#define TCU_TSTSR	(0xF4)   /* Timer Status Set Register */
#define TCU_TSTCR	(0xF8)   /* Timer Status Clear Register */

#define CH_TDFR(n)	(0x40 + (n)*0x10) /* Timer Data Full Reg */
#define CH_TDHR(n)	(0x44 + (n)*0x10) /* Timer Data Half Reg */
#define CH_TCNT(n)	(0x48 + (n)*0x10) /* Timer Counter Reg */
#define CH_TCSR(n)	(0x4C + (n)*0x10) /* Timer Control Reg */

#define TER_OSTEN	(1 << 15)   /* enable the counter in ost */
#define TMR_OSTM	(1 << 15)   /* ost comparison match interrupt mask */
#define TFR_OSTF	(1 << 15)   /* ost interrupt flag */
#define TSR_OSTS	(1 << 15)   /*the clock supplies to osts is stopped */

#define TSR_WDTS	(1 << 16)   /*the clock supplies to wdt is stopped */

#define CSR_EXT_EN	(1 << 2)  /* select extal as the timer clock input */
#define CSR_RTC_EN	(1 << 1)  /* select rtcclk as the timer clock input */
#define CSR_PCK_EN	(1 << 0)  /* select pclk as the timer clock input */
#define CSR_CLK_MSK	(0x7)

#define CSR_DIV1	(0x0 << 3)
#define CSR_DIV4	(0x1 << 3)
#define CSR_DIV16	(0x2 << 3)
#define CSR_DIV64	(0x3 << 3)
#define CSR_DIV256	(0x4 << 3)
#define CSR_DIV1024	(0x5 << 3)
#define CSR_DIV_MSK	(0x7 << 3)

// Register bits definitions
#define TSTR_REAL2		(1 << 18) /* only used in TCU2 mode */
#define TSTR_REAL1		(1 << 17) /* only used in TCU2 mode */
#define TSTR_BUSY2		(1 << 2)  /* only used in TCU2 mode */
#define TSTR_BUSY1		(1 << 1)  /* only used in TCU2 mode */

#define TCSR_PWM_BYPASS (1 << 11) /* clear counter to 0, only used in TCU2 mode */
#define TCSR_CNT_CLRZ	(1 << 10) /* clear counter to 0, only used in TCU2 mode */
#define TCSR_PWM_SD		(1 << 9)  /* shut down the pwm output only used in TCU1 mode */
#define TCSR_PWM_HIGH	(1 << 8)  /* selects an initial output level for pwm output */
#define TCSR_PWM_EN		(1 << 7)  /* pwm pin output enable */
#define TCSR_PWM_IN		(1 << 6)  /* pwm pin output enable */


/*
 * Operating system timer module(OST) address definition
 */

#define OST_DR		    (0xe0)
#define OST_CNTL	    (0xe4)
#define OST_CNTH	    (0xe8)
#define OST_CSR		    (0xec)
#define OST_CNTH_BUF	(0xfc)

/* Operating system control register(OSTCSR) */
#define OSTCSR_CNT_MD	(1 << 15)

#endif

