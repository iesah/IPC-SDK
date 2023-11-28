#ifndef _XB_SND_DMIC_H__
#define _XB_SND_DMIC_H__

#include <asm/io.h>
#include <linux/delay.h>
#include "../interface/xb_snd_dsp.h"
#include "../xb_snd_detect.h"

#define NEED_RECONF_DMA         0x00000001
#define NEED_RECONF_TRIGGER     0x00000002
#define NEED_RECONF_FILTER      0x00000004
#define NEED_CONF_SPLIT			0x00000008

/*regs offset*/
#define DMICCR0		0x0
#define DMICGCR		0x4
#define DMICIMR		0x8 //
#define DMICICR		0xc
#define DMICTRICR	0x10
#define DMICTHRH	0x14
#define DMICTHRL	0x18
#define DMICTRIMMAX	0x1c //
#define DMICTRINMAX 0x20 //
#define DMICDR		0x30
#define DMICFCR		0x34
#define DMICFSR		0x38
#define DMICCGDIS	0x50 //

/**
 * DMIC register control
 **/
#define dmic_writel(dmic_dev, reg, value) \
	__raw_writel((value), (dmic_dev)->dmic_iomem + reg)
#define dmic_readl(dmic_dev, reg)\
	__raw_readl((dmic_dev)->dmic_iomem + reg)

enum regs_bit_width {
	REGS_BIT_WIDTH_1BIT = 1,
	REGS_BIT_WIDTH_2BIT,
	REGS_BIT_WIDTH_3BIT,
	REGS_BIT_WIDTH_4BIT,
	REGS_BIT_WIDTH_5BIT,
	REGS_BIT_WIDTH_6BIT,
	REGS_BIT_WIDTH_7BIT,
	REGS_BIT_WIDTH_8BIT,
	REGS_BIT_WIDTH_9BIT,
	REGS_BIT_WIDTH_10BIT,
	REGS_BIT_WIDTH_11BIT,
	REGS_BIT_WIDTH_12BIT,
	REGS_BIT_WIDTH_13BIT,
	REGS_BIT_WIDTH_14BIT,
	REGS_BIT_WIDTH_15BIT,
	REGS_BIT_WIDTH_16BIT,
	REGS_BIT_WIDTH_17BIT,
	REGS_BIT_WIDTH_18BIT,
	REGS_BIT_WIDTH_19BIT,
	REGS_BIT_WIDTH_20BIT,
	REGS_BIT_WIDTH_21BIT,
	REGS_BIT_WIDTH_22BIT,
	REGS_BIT_WIDTH_23BIT,
	REGS_BIT_WIDTH_24BIT,
	REGS_BIT_WIDTH_25BIT,
	REGS_BIT_WIDTH_26BIT,
	REGS_BIT_WIDTH_27BIT,
	REGS_BIT_WIDTH_28BIT,
	REGS_BIT_WIDTH_29BIT,
	REGS_BIT_WIDTH_30BIT,
	REGS_BIT_WIDTH_31BIT,
	REGS_BIT_WIDTH_32BIT,
};

/*DMICCR0*/
#define DMICCR0_DMIC_EN		0 /*shift*/
#define DMICCR0_TRI_EN		1
#define DMICCR0_HPF1_EN		2
#define DMICCR0_LP_MODE		3
#define DMICCR0_DMIC_R		4
#define DMICCR0_STEREO		5
#define DMICCR0_SR			6
#define DMICCR0_PACK_EN		8
#define DMICCR0_MCLK_24		9
#define DMICCR0_SPLIT_DI	10
#define DMICCR0_SW_LR		11
#define DMICCR0_UNPACK_DIS	12
#define DMICCR0_UNPACK_MSB	13
	/*14-29 reserved*/
#define DMICCR0_RESET_TRI	30
#define DMICCR0_RESET		31
enum { /*possible value*/
	SAMPLE_RATE_8K = 0,
	SAMPLE_RATE_16K,
	SAMPLE_RATE_48K,
	SAMPLE_RATE_DIS,
};

/*DMICGCR*/
#define DMIC_GCR	0
/* DMICIMR */
#define DMICIMR_TRI_MASK	(1 << 0)
#define DMICIMR_PRERD_MASK	(1 << 1)
#define DMICIMR_FULL_MASK	(1 << 2)
#define DMICIMR_EMPTY_MASK	(1 << 3)
#define DMICIMR_WAKE_MASK	(1 << 4)
#define DMICIMR_MASK_ALL	(0x1f)

/*DMICICR*/ /*DMICINTCR_XXX (x<<n)*/
#define DMICICR_TRIFLG		(1 << 0)
#define DMICICR_PREREAD		(1 << 1)
#define DMICICR_FULL			(1 << 2)
#define DMICICR_EMPTY			(1 << 3)
#define DMICICR_WAKEUP		(1 << 4)
#define DMICICR_MASK_ALL	(0x1f)

/*DMICTRICR*/
#define DMICTRICR_TRI_CLR	0
#define DMICTRICR_PREFETCH	1 /*2BIT*/
#define DMICTRICR_HPF2_EN	3
#define DMICTRICR_TRI_DEBUG	4
#define DMICTRICR_TRI_MOD	16 /*16-19, 4bit*/

enum {
	DMICTRICR_PREFETCH_DIS = 0,
	DMICTRICR_PREFETCH_8K,
	DMICTRICR_PREFETCH_16K,
};

/*DMICTHRH*/
#define DMIC_THR_H 0

/*DMICTHRL*/
#define DMIC_THR_L 0

/* DMICTRIMMAX */
#define DMIC_M_MAX	0

/* DMICTRINMAX */
#define DMIC_N_MAX	0


/*DMICDR*/

/* DMICFCR */
#define DMICFCR_RDMS	31
#define DMICFCR_THR		0
/*DMICFSR*/

/*DMICCGDIS */

#endif
