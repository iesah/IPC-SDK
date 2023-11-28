/*
 * JZ4770 tcu register definition.
 *
 * Copyright (C) 2010 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __OST_H__
#define __OST_H__

#define OST_TSR		(0x1C)  
#define OST_TSSR	(0x2C)   /* Timer Stop Set Register */
#define OST_TSCR	(0x3C)   /* Timer Stop Clear Register */
#define OST_TER		(0x10)   /* Timer Counter Enable Register */
#define OST_TESR	(0x14)   /* Timer Counter Enable Set Register */
#define OST_TECR	(0x18)   /* Timer Counter Enable Clear Register */
#define OST_TFR		(0x20)   /* Timer Flag Register */
#define OST_TFSR	(0x24)   /* Timer Flag Set Register */
#define OST_TFCR	(0x28)   /* Timer Flag Clear Register */
#define OST_TMR		(0x30)   /* Timer Mask Register */
#define OST_TMSR	(0x34)   /* Timer Mask Set Register */
#define OST_TMCR	(0x38)   /* Timer Mask Clear Register */
#define OST_TDFR	(0xb0)
#define OST_TDHR	(0xb4)
#define OST_TCNT	(0xb8)
#define OST_TCSR	(0xbc)
#define OST_TDFSR	(0xc0)
#define OST_TDHSR	(0xc4)
#define OST_SSR		(0xc8)
#define OST_DR		(0xe0)
#define OST_CNTL	(0xe4)
#define OST_CNTH	(0xe8)
#define OST_CSR		(0xec)
#define OST_CNTH_BUF	(0xfc)

#define CSR_DIV1	(0x0 << 3)
#define CSR_DIV4	(0x1 << 3)
#define CSR_DIV16	(0x2 << 3)
#define CSR_DIV64	(0x3 << 3)
#define CSR_DIV256	(0x4 << 3)
#define CSR_DIV1024	(0x5 << 3)
#define CSR_DIV_MSK	(0x7 << 3)

#define OSTCSR_CNT_MD	(1 << 15)
#define OST_EN		(1 << 15)
#define TER_OSTEN	(1 << 15)   /* enable the counter in ost */
#define TMR_OSTM	(1 << 15)   /* ost comparison match interrupt mask */
#define TFR_OSTF	(1 << 15)   /* ost interrupt flag */
#define TSR_OSTS	(1 << 15)   /*the clock supplies to osts is stopped */

#endif

