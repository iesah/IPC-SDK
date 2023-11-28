/*
 * driver/mfd/ricoh619-irq.c
 *
 * Interrupt driver for RICOH R5T619 power management chip.
 *
 * Copyright (C) 2012-2014 RICOH COMPANY,LTD
 *
 * Based on code
 *	Copyright (C) 2011 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/mfd/ricoh619.h>


enum int_type {
	SYS_INT  = 0x1,
	DCDC_INT = 0x2,
	RTC_INT  = 0x4,
	ADC_INT  = 0x8,
	GPIO_INT = 0x10,
	CHG_INT	= 0x40,
};

static int gpedge_add[] = {
	RICOH61x_GPIO_GPEDGE1,
	RICOH61x_GPIO_GPEDGE2
};

static int irq_en_add[] = {
	RICOH61x_INT_EN_SYS,
	RICOH61x_INT_EN_DCDC,
	RICOH61x_INT_EN_RTC,
	RICOH61x_INT_EN_ADC1,
	RICOH61x_INT_EN_ADC2,
	RICOH61x_INT_EN_ADC3,
	RICOH61x_INT_EN_GPIO,
	RICOH61x_INT_EN_GPIO2,
	RICOH61x_INT_MSK_CHGCTR,
	RICOH61x_INT_MSK_CHGSTS1,
	RICOH61x_INT_MSK_CHGSTS2,
	RICOH61x_INT_MSK_CHGERR,
	RICOH61x_INT_MSK_CHGEXTIF
};

static int irq_mon_add[] = {
	RICOH61x_INT_IR_SYS,		/* RICOH61x_INT_MON_SYS, */
	RICOH61x_INT_IR_DCDC,		/* RICOH61x_INT_MON_DCDC, */
	RICOH61x_INT_IR_RTC,		/* RICOH61x_INT_MON_RTC, */
	RICOH61x_INT_IR_ADCL,
	RICOH61x_INT_IR_ADCH,
	RICOH61x_INT_IR_ADCEND,
	RICOH61x_INT_IR_GPIOR,
	RICOH61x_INT_IR_GPIOF,
	RICOH61x_INT_IR_CHGCTR,		/* RICOH61x_INT_MON_CHGCTR, */
	RICOH61x_INT_IR_CHGSTS1,	/* RICOH61x_INT_MON_CHGSTS1, */
	RICOH61x_INT_IR_CHGSTS2,	/* RICOH61x_INT_MON_CHGSTS2, */
	RICOH61x_INT_IR_CHGERR,		/* RICOH61x_INT_MON_CHGERR */
	RICOH61x_INT_IR_CHGEXTIF	/* RICOH61x_INT_MON_CHGEXTIF */
};

static int irq_clr_add[] = {
	RICOH61x_INT_IR_SYS,
	RICOH61x_INT_IR_DCDC,
	RICOH61x_INT_IR_RTC,
	RICOH61x_INT_IR_ADCL,
	RICOH61x_INT_IR_ADCH,
	RICOH61x_INT_IR_ADCEND,
	RICOH61x_INT_IR_GPIOR,
	RICOH61x_INT_IR_GPIOF,
	RICOH61x_INT_IR_CHGCTR,
	RICOH61x_INT_IR_CHGSTS1,
	RICOH61x_INT_IR_CHGSTS2,
	RICOH61x_INT_IR_CHGERR,
	RICOH61x_INT_IR_CHGEXTIF
};

static int main_int_type[] = {
	SYS_INT,
	DCDC_INT,
	RTC_INT,
	ADC_INT,
	ADC_INT,
	ADC_INT,
	GPIO_INT,
	GPIO_INT,
	CHG_INT,
	CHG_INT,
	CHG_INT,
	CHG_INT,
	CHG_INT,
};

struct ricoh61x_irq_data {
	u8	int_type;
	u8	master_bit;
	u8	int_en_bit;
	u8	mask_reg_index;
	int	grp_index;
};

#define RICOH61x_IRQ(_int_type, _master_bit, _grp_index, _int_bit, _mask_ind) \
	{						\
		.int_type	= _int_type,		\
		.master_bit	= _master_bit,		\
		.grp_index	= _grp_index,		\
		.int_en_bit	= _int_bit,		\
		.mask_reg_index	= _mask_ind,		\
	}

static const struct ricoh61x_irq_data ricoh61x_irqs[RICOH61x_NR_IRQS] = {
	[RICOH61x_IRQ_POWER_ON]		= RICOH61x_IRQ(SYS_INT,  0, 0, 0, 0),
	[RICOH61x_IRQ_EXTIN]		= RICOH61x_IRQ(SYS_INT,  0, 1, 1, 0),
	[RICOH61x_IRQ_PRE_VINDT]	= RICOH61x_IRQ(SYS_INT,  0, 2, 2, 0),
	[RICOH61x_IRQ_PREOT]		= RICOH61x_IRQ(SYS_INT,  0, 3, 3, 0),
	[RICOH61x_IRQ_POWER_OFF]	= RICOH61x_IRQ(SYS_INT,  0, 4, 4, 0),
	[RICOH61x_IRQ_NOE_OFF]		= RICOH61x_IRQ(SYS_INT,  0, 5, 5, 0),
	[RICOH61x_IRQ_WD]		= RICOH61x_IRQ(SYS_INT,  0, 6, 6, 0),

	[RICOH61x_IRQ_DC1LIM]		= RICOH61x_IRQ(DCDC_INT, 1, 0, 0, 1),
	[RICOH61x_IRQ_DC2LIM]		= RICOH61x_IRQ(DCDC_INT, 1, 1, 1, 1),
	[RICOH61x_IRQ_DC3LIM]		= RICOH61x_IRQ(DCDC_INT, 1, 2, 2, 1),
	[RICOH61x_IRQ_DC4LIM]		= RICOH61x_IRQ(DCDC_INT, 1, 3, 3, 1),
	[RICOH61x_IRQ_DC5LIM]		= RICOH61x_IRQ(DCDC_INT, 1, 4, 4, 1),

	[RICOH61x_IRQ_CTC]		= RICOH61x_IRQ(RTC_INT,  2, 0, 0, 2),
	[RICOH61x_IRQ_DALE]		= RICOH61x_IRQ(RTC_INT,  2, 1, 6, 2),

	[RICOH61x_IRQ_ILIMLIR]		= RICOH61x_IRQ(ADC_INT,  3, 0, 0, 3),
	[RICOH61x_IRQ_VBATLIR]		= RICOH61x_IRQ(ADC_INT,  3, 1, 1, 3),
	[RICOH61x_IRQ_VADPLIR]		= RICOH61x_IRQ(ADC_INT,  3, 2, 2, 3),
	[RICOH61x_IRQ_VUSBLIR]		= RICOH61x_IRQ(ADC_INT,  3, 3, 3, 3),
	[RICOH61x_IRQ_VSYSLIR]		= RICOH61x_IRQ(ADC_INT,  3, 4, 4, 3),
	[RICOH61x_IRQ_VTHMLIR]		= RICOH61x_IRQ(ADC_INT,  3, 5, 5, 3),
	[RICOH61x_IRQ_AIN1LIR]		= RICOH61x_IRQ(ADC_INT,  3, 6, 6, 3),
	[RICOH61x_IRQ_AIN0LIR]		= RICOH61x_IRQ(ADC_INT,  3, 7, 7, 3),

	[RICOH61x_IRQ_ILIMHIR]		= RICOH61x_IRQ(ADC_INT,  3, 8, 0, 4),
	[RICOH61x_IRQ_VBATHIR]		= RICOH61x_IRQ(ADC_INT,  3, 9, 1, 4),
	[RICOH61x_IRQ_VADPHIR]		= RICOH61x_IRQ(ADC_INT,  3, 10, 2, 4),
	[RICOH61x_IRQ_VUSBHIR]		= RICOH61x_IRQ(ADC_INT,  3, 11, 3, 4),
	[RICOH61x_IRQ_VSYSHIR]		= RICOH61x_IRQ(ADC_INT,  3, 12, 4, 4),
	[RICOH61x_IRQ_VTHMHIR]		= RICOH61x_IRQ(ADC_INT,  3, 13, 5, 4),
	[RICOH61x_IRQ_AIN1HIR]		= RICOH61x_IRQ(ADC_INT,  3, 14, 6, 4),
	[RICOH61x_IRQ_AIN0HIR]		= RICOH61x_IRQ(ADC_INT,  3, 15, 7, 4),

	[RICOH61x_IRQ_ADC_ENDIR]	= RICOH61x_IRQ(ADC_INT,  3, 16, 0, 5),

	[RICOH61x_IRQ_GPIO0]		= RICOH61x_IRQ(GPIO_INT, 4, 0, 0, 6),
	[RICOH61x_IRQ_GPIO1]		= RICOH61x_IRQ(GPIO_INT, 4, 1, 1, 6),
	[RICOH61x_IRQ_GPIO2]		= RICOH61x_IRQ(GPIO_INT, 4, 2, 2, 6),
	[RICOH61x_IRQ_GPIO3]		= RICOH61x_IRQ(GPIO_INT, 4, 3, 3, 6),
	[RICOH61x_IRQ_GPIO4]		= RICOH61x_IRQ(GPIO_INT, 4, 4, 4, 6),

	[RICOH61x_IRQ_FVADPDETSINT]	= RICOH61x_IRQ(CHG_INT, 6, 0, 0, 8),
	[RICOH61x_IRQ_FVUSBDETSINT]	= RICOH61x_IRQ(CHG_INT, 6, 1, 1, 8),
	[RICOH61x_IRQ_FVADPLVSINT]	= RICOH61x_IRQ(CHG_INT, 6, 2, 2, 8),
	[RICOH61x_IRQ_FVUSBLVSINT]	= RICOH61x_IRQ(CHG_INT, 6, 3, 3, 8),
	[RICOH61x_IRQ_FWVADPSINT]	= RICOH61x_IRQ(CHG_INT, 6, 4, 4, 8),
	[RICOH61x_IRQ_FWVUSBSINT]	= RICOH61x_IRQ(CHG_INT, 6, 5, 5, 8),

	[RICOH61x_IRQ_FONCHGINT]	= RICOH61x_IRQ(CHG_INT, 6, 6, 0, 9),
	[RICOH61x_IRQ_FCHGCMPINT]	= RICOH61x_IRQ(CHG_INT, 6, 7, 1, 9),
	[RICOH61x_IRQ_FBATOPENINT]	= RICOH61x_IRQ(CHG_INT, 6, 8, 2, 9),
	[RICOH61x_IRQ_FSLPMODEINT]	= RICOH61x_IRQ(CHG_INT, 6, 9, 3, 9),
	[RICOH61x_IRQ_FBTEMPJTA1INT]	= RICOH61x_IRQ(CHG_INT, 6, 10, 4, 9),
	[RICOH61x_IRQ_FBTEMPJTA2INT]	= RICOH61x_IRQ(CHG_INT, 6, 11, 5, 9),
	[RICOH61x_IRQ_FBTEMPJTA3INT]	= RICOH61x_IRQ(CHG_INT, 6, 12, 6, 9),
	[RICOH61x_IRQ_FBTEMPJTA4INT]	= RICOH61x_IRQ(CHG_INT, 6, 13, 7, 9),

	[RICOH61x_IRQ_FCURTERMINT]	= RICOH61x_IRQ(CHG_INT, 6, 14, 0, 10),
	[RICOH61x_IRQ_FVOLTERMINT]	= RICOH61x_IRQ(CHG_INT, 6, 15, 1, 10),
	[RICOH61x_IRQ_FICRVSINT]	= RICOH61x_IRQ(CHG_INT, 6, 16, 2, 10),
	[RICOH61x_IRQ_FPOOR_CHGCURINT]	= RICOH61x_IRQ(CHG_INT, 6, 17, 3, 10),
	[RICOH61x_IRQ_FOSCFDETINT1]	= RICOH61x_IRQ(CHG_INT, 6, 18, 4, 10),
	[RICOH61x_IRQ_FOSCFDETINT2]	= RICOH61x_IRQ(CHG_INT, 6, 19, 5, 10),
	[RICOH61x_IRQ_FOSCFDETINT3]	= RICOH61x_IRQ(CHG_INT, 6, 20, 6, 10),
	[RICOH61x_IRQ_FOSCMDETINT]	= RICOH61x_IRQ(CHG_INT, 6, 21, 7, 10),

	[RICOH61x_IRQ_FDIEOFFINT]	= RICOH61x_IRQ(CHG_INT, 6, 22, 0, 11),
	[RICOH61x_IRQ_FDIEERRINT]	= RICOH61x_IRQ(CHG_INT, 6, 23, 1, 11),
	[RICOH61x_IRQ_FBTEMPERRINT]	= RICOH61x_IRQ(CHG_INT, 6, 24, 2, 11),
	[RICOH61x_IRQ_FVBATOVINT]	= RICOH61x_IRQ(CHG_INT, 6, 25, 3, 11),
	[RICOH61x_IRQ_FTTIMOVINT]	= RICOH61x_IRQ(CHG_INT, 6, 26, 4, 11),
	[RICOH61x_IRQ_FRTIMOVINT]	= RICOH61x_IRQ(CHG_INT, 6, 27, 5, 11),
	[RICOH61x_IRQ_FVADPOVSINT]	= RICOH61x_IRQ(CHG_INT, 6, 28, 6, 11),
	[RICOH61x_IRQ_FVUSBOVSINT]	= RICOH61x_IRQ(CHG_INT, 6, 29, 7, 11),

	[RICOH61x_IRQ_FGCDET]		= RICOH61x_IRQ(CHG_INT, 6, 30, 0, 12),
	[RICOH61x_IRQ_FPCDET]		= RICOH61x_IRQ(CHG_INT, 6, 31, 1, 12),
	[RICOH61x_IRQ_FWARN_ADP]	= RICOH61x_IRQ(CHG_INT, 6, 32, 3, 12),

};

static void ricoh61x_irq_lock(struct irq_data *irq_data)
{
	struct ricoh61x *ricoh61x = irq_data_get_irq_chip_data(irq_data);

	mutex_lock(&ricoh61x->irq_lock);
}

static void ricoh61x_irq_unmask(struct irq_data *irq_data)
{
	struct ricoh61x *ricoh61x = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - ricoh61x->irq_base;
	const struct ricoh61x_irq_data *data = &ricoh61x_irqs[__irq];

	ricoh61x->group_irq_en[data->master_bit] |= (1 << data->grp_index);
	if (ricoh61x->group_irq_en[data->master_bit])
		ricoh61x->intc_inten_reg |= 1 << data->master_bit;

	if (data->master_bit == 6)	/* if Charger */
		ricoh61x->irq_en_reg[data->mask_reg_index]
						&= ~(1 << data->int_en_bit);
	else
		ricoh61x->irq_en_reg[data->mask_reg_index]
						|= 1 << data->int_en_bit;
}

static void ricoh61x_irq_mask(struct irq_data *irq_data)
{
	struct ricoh61x *ricoh61x = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - ricoh61x->irq_base;
	const struct ricoh61x_irq_data *data = &ricoh61x_irqs[__irq];

	ricoh61x->group_irq_en[data->master_bit] &= ~(1 << data->grp_index);
	if (!ricoh61x->group_irq_en[data->master_bit])
		ricoh61x->intc_inten_reg &= ~(1 << data->master_bit);

	if (data->master_bit == 6)	/* if Charger */
		ricoh61x->irq_en_reg[data->mask_reg_index]
						|= 1 << data->int_en_bit;
	else
		ricoh61x->irq_en_reg[data->mask_reg_index]
						&= ~(1 << data->int_en_bit);
}

static void ricoh61x_irq_sync_unlock(struct irq_data *irq_data)
{
	struct ricoh61x *ricoh61x = irq_data_get_irq_chip_data(irq_data);
	int i;

	for (i = 0; i < ARRAY_SIZE(ricoh61x->gpedge_reg); i++) {
		if (ricoh61x->gpedge_reg[i] != ricoh61x->gpedge_cache[i]) {
			if (!WARN_ON(ricoh61x_write(ricoh61x->dev,
						    gpedge_add[i],
						    ricoh61x->gpedge_reg[i])))
				ricoh61x->gpedge_cache[i] =
						ricoh61x->gpedge_reg[i];
		}
	}

	for (i = 0; i < ARRAY_SIZE(ricoh61x->irq_en_reg); i++) {
		if (ricoh61x->irq_en_reg[i] != ricoh61x->irq_en_cache[i]) {
			if (!WARN_ON(ricoh61x_write(ricoh61x->dev,
						irq_en_add[i],
						ricoh61x->irq_en_reg[i])))
				ricoh61x->irq_en_cache[i] =
						ricoh61x->irq_en_reg[i];
		}
	}

	if (ricoh61x->intc_inten_reg != ricoh61x->intc_inten_cache) {
		if (!WARN_ON(ricoh61x_write(ricoh61x->dev,
				RICOH61x_INTC_INTEN, ricoh61x->intc_inten_reg)))
			ricoh61x->intc_inten_cache = ricoh61x->intc_inten_reg;
	}

	mutex_unlock(&ricoh61x->irq_lock);
}

static int ricoh61x_irq_set_type(struct irq_data *irq_data, unsigned int type)
{
	struct ricoh61x *ricoh61x = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - ricoh61x->irq_base;
	const struct ricoh61x_irq_data *data = &ricoh61x_irqs[__irq];
	int val = 0;
	int gpedge_index;
	int gpedge_bit_pos;

	if (data->int_type & GPIO_INT) {
		gpedge_index = data->int_en_bit / 4;
		gpedge_bit_pos = data->int_en_bit % 4;

		if (type & IRQ_TYPE_EDGE_FALLING)
			val |= 0x2;

		if (type & IRQ_TYPE_EDGE_RISING)
			val |= 0x1;

		ricoh61x->gpedge_reg[gpedge_index] &= ~(3 << gpedge_bit_pos);
		ricoh61x->gpedge_reg[gpedge_index] |= (val << gpedge_bit_pos);
		ricoh61x_irq_unmask(irq_data);
	}
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int ricoh61x_irq_set_wake(struct irq_data *irq_data, unsigned int on)
{
	struct ricoh61x *ricoh61x = irq_data_get_irq_chip_data(irq_data);
	return irq_set_irq_wake(ricoh61x->chip_irq, on);	/* i2c->irq */
}
#else
#define ricoh61x_irq_set_wake NULL
#endif

static irqreturn_t ricoh61x_irq(int irq, void *data)
{
	struct ricoh61x *ricoh61x = data;
	u8 int_sts[MAX_INTERRUPT_MASKS];
	u8 master_int;
	int i;
	int ret;
	unsigned int rtc_int_sts = 0;

	/* printk("PMU: %s: irq=%d\n", __func__, irq); */
	/* disable_irq_nosync(irq); */
	/* Clear the status */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++)
		int_sts[i] = 0;

	ret = ricoh61x_read(ricoh61x->dev, RICOH61x_INTC_INTMON,
						&master_int);
	printk(KERN_DEBUG "PMU1: %s: master_int=0x%x\n", __func__, master_int);
	if (ret < 0) {
		dev_err(ricoh61x->dev, "Error in reading reg 0x%02x "
			"error: %d\n", RICOH61x_INTC_INTMON, ret);
		return IRQ_HANDLED;
	}

	for (i = 0; i < MAX_INTERRUPT_MASKS; ++i) {
		/* Even if INTC_INTMON register = 1, INT signal might not
		 * output because INTC_INTMON register indicates only interrupt
		 * facter level.
		 * So remove the following procedure
		 */
		if (!(master_int & main_int_type[i]))
			continue;

		ret = ricoh61x_read(ricoh61x->dev,
				irq_mon_add[i], &int_sts[i]);
		printk(KERN_DEBUG "PMU2: %s: int_sts[%d]=0x%x\n",
						 __func__, i, int_sts[i]);
		if (ret < 0) {
			dev_err(ricoh61x->dev, "Error in reading reg 0x%02x "
				"error: %d\n", irq_mon_add[i], ret);
			int_sts[i] = 0;
			continue;
		}
		if (!int_sts[i])
			continue;

		if (main_int_type[i] & RTC_INT) {
			/* Changes status bit position
				 from RTCCNT2 to RTCCNT1 */
			rtc_int_sts = 0;
			if (int_sts[i] & 0x1)
				rtc_int_sts |= BIT(6);
			if (int_sts[i] & 0x4)
				rtc_int_sts |= BIT(0);
		}

		if(irq_clr_add[i] == RICOH61x_INT_IR_RTC)
		{
			int_sts[i] &= ~0x85;
			ret = ricoh61x_write(ricoh61x->dev,
				irq_clr_add[i], int_sts[i]);
			if (ret < 0) {
				dev_err(ricoh61x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_clr_add[i], ret);
			}
		}
		else
		{
			ret = ricoh61x_write(ricoh61x->dev,
				irq_clr_add[i], ~int_sts[i]);
			if (ret < 0) {
				dev_err(ricoh61x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_clr_add[i], ret);
			}
		}

		/* Mask Charger Interrupt */
		if (main_int_type[i] & CHG_INT) {
			if (int_sts[i])
				ret = ricoh61x_write(ricoh61x->dev,
							irq_en_add[i], 0xff);
				if (ret < 0) {
					dev_err(ricoh61x->dev,
						"Error in write reg 0x%02x error: %d\n",
							irq_en_add[i], ret);
				}
		}
		/* Mask ADC Interrupt */
		if (main_int_type[i] & ADC_INT) {
			if (int_sts[i])
				ret = ricoh61x_write(ricoh61x->dev,
							irq_en_add[i], 0);
				if (ret < 0) {
					dev_err(ricoh61x->dev,
						"Error in write reg 0x%02x error: %d\n",
							irq_en_add[i], ret);
				}
		}

		if (main_int_type[i] & RTC_INT)
			int_sts[i] = rtc_int_sts;

	}

	/* Merge gpio interrupts  for rising and falling case*/
	int_sts[6] |= int_sts[7];

	/* Call interrupt handler if enabled */
	for (i = 0; i < RICOH61x_NR_IRQS; ++i) {
		const struct ricoh61x_irq_data *data = &ricoh61x_irqs[i];
		if ((int_sts[data->mask_reg_index] & (1 << data->int_en_bit)) &&
			(ricoh61x->group_irq_en[data->master_bit] &
					(1 << data->grp_index)))
			handle_nested_irq(ricoh61x->irq_base + i);
	}

	printk(KERN_DEBUG "PMU: %s: out\n", __func__);
	return IRQ_HANDLED;
}

static struct irq_chip ricoh61x_irq_chip = {
	.name = "ricoh619",
	.irq_mask = ricoh61x_irq_mask,
	.irq_unmask = ricoh61x_irq_unmask,
	.irq_bus_lock = ricoh61x_irq_lock,
	.irq_bus_sync_unlock = ricoh61x_irq_sync_unlock,
	.irq_set_type = ricoh61x_irq_set_type,
	.irq_set_wake = ricoh61x_irq_set_wake,
};

int ricoh61x_irq_init(struct ricoh61x *ricoh61x, int irq,
				int irq_base)
{
	int i, ret;
	u8 reg_data = 0;

	if (!irq_base) {
		dev_warn(ricoh61x->dev, "No interrupt support on IRQ base\n");
		return -EINVAL;
	}

	mutex_init(&ricoh61x->irq_lock);

	/* Initialize all locals to 0 */
	for (i = 0; i < 2; i++) {
		ricoh61x->irq_en_cache[i] = 0;
		ricoh61x->irq_en_reg[i] = 0;
	}
	/* Initialize rtc */
	ricoh61x->irq_en_cache[2] = 0x20;
	ricoh61x->irq_en_reg[2] = 0x20;

	/* Initialize all locals to 0 */
	for (i = 3; i < 8; i++) {
		ricoh61x->irq_en_cache[i] = 0;
		ricoh61x->irq_en_reg[i] = 0;
	}
	/* Charger Mask register must be set to 0xff for masking Int output. */
	for (i = 8; i < MAX_INTERRUPT_MASKS; i++) {
		ricoh61x->irq_en_cache[i] = 0xff;
		ricoh61x->irq_en_reg[i] = 0xff;
	}

	ricoh61x->intc_inten_cache = 0;
	ricoh61x->intc_inten_reg = 0;
	for (i = 0; i < MAX_GPEDGE_REG; i++) {
		ricoh61x->gpedge_cache[i] = 0;
		ricoh61x->gpedge_reg[i] = 0;
	}

	/* Initailize all int register to 0 */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++)  {
		ret = ricoh61x_write(ricoh61x->dev,
				irq_en_add[i],
				ricoh61x->irq_en_reg[i]);
		if (ret < 0)
			dev_err(ricoh61x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_en_add[i], ret);
	}

	for (i = 0; i < MAX_GPEDGE_REG; i++)  {
		ret = ricoh61x_write(ricoh61x->dev,
				gpedge_add[i],
				ricoh61x->gpedge_reg[i]);
		if (ret < 0)
			dev_err(ricoh61x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", gpedge_add[i], ret);
	}

	ret = ricoh61x_write(ricoh61x->dev, RICOH61x_INTC_INTEN, 0x0);
	if (ret < 0)
		dev_err(ricoh61x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", RICOH61x_INTC_INTEN, ret);

	/* Clear all interrupts in case they woke up active. */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++)  {
		if (irq_clr_add[i] != RICOH61x_INT_IR_RTC) {
			ret = ricoh61x_write(ricoh61x->dev,
						irq_clr_add[i], 0);
			if (ret < 0)
				dev_err(ricoh61x->dev, "Error in writing reg 0x%02x "
					"error: %d\n", irq_clr_add[i], ret);
		} else {
			ret = ricoh61x_read(ricoh61x->dev,
					RICOH61x_INT_IR_RTC, &reg_data);
			if (ret < 0)
				dev_err(ricoh61x->dev, "Error in reading reg 0x%02x "
					"error: %d\n", RICOH61x_INT_IR_RTC, ret);
			reg_data &= 0xf0;
			ret = ricoh61x_write(ricoh61x->dev,
					RICOH61x_INT_IR_RTC, reg_data);
			if (ret < 0)
				dev_err(ricoh61x->dev, "Error in writing reg 0x%02x "
					"error: %d\n", RICOH61x_INT_IR_RTC, ret);
		}
	}

	ricoh61x->irq_base = irq_base;
	ricoh61x->chip_irq = irq;

	for (i = 0; i < RICOH61x_NR_IRQS; i++) {
		int __irq = i + ricoh61x->irq_base;
		irq_set_chip_data(__irq, ricoh61x);
		irq_set_chip_and_handler(__irq, &ricoh61x_irq_chip,
					 handle_simple_irq);
		irq_set_nested_thread(__irq, 1);
#ifdef CONFIG_ARM
		set_irq_flags(__irq, IRQF_VALID);
#endif
	}

	ret = request_threaded_irq(irq, NULL, ricoh61x_irq,
			IRQ_TYPE_EDGE_FALLING|IRQF_DISABLED|IRQF_ONESHOT,
						"ricoh61x", ricoh61x);
	if (ret < 0)
		dev_err(ricoh61x->dev, "Error in registering interrupt "
				"error: %d\n", ret);
	if (!ret) {
		device_init_wakeup(ricoh61x->dev, 1);
		enable_irq_wake(irq);
	}

	return ret;
}

int ricoh61x_irq_exit(struct ricoh61x *ricoh61x)
{
	if (ricoh61x->chip_irq)
		free_irq(ricoh61x->chip_irq, ricoh61x);
	return 0;
}
