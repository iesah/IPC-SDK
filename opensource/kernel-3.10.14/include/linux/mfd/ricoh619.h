/*
 * include/linux/mfd/ricoh619.h
 *
 * Core driver interface to access RICOH R5T619 power management chip.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __LINUX_MFD_RICOH619_H
#define __LINUX_MFD_RICOH619_H

#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <jz_notifier.h>

/* Maximum number of main interrupts */
#define MAX_INTERRUPT_MASKS	13
#define MAX_MAIN_INTERRUPT	7
#define MAX_GPEDGE_REG		2

/* Power control register */
#define RICOH61x_PWR_WD			0x0B
#define RICOH61x_PWR_WD_COUNT		0x0C
#define RICOH61x_PWR_FUNC		0x0D
#define RICOH61x_PWR_SLP_CNT		0x0E
#define RICOH61x_PWR_REP_CNT		0x0F
#define RICOH61x_PWR_ON_TIMSET		0x10
#define RICOH61x_PWR_NOE_TIMSET		0x11
#define RICOH61x_PWR_IRSEL		0x15

/* DC/LDO control register */
#define RICOH61x_DC1_CTL		0x2C
#define RICOH61x_DC1_CTL2		0x2D
#define RICOH61x_DC2_CTL		0x2E
#define RICOH61x_DC2_CTL2		0x2F
#define RICOH61x_DC3_CTL		0x30
#define RICOH61x_DC3_CTL2		0x31
#define RICOH61x_DC4_CTL		0x32
#define RICOH61x_DC4_CTL2		0x33
#define RICOH61x_DC5_CTL		0x34
#define RICOH61x_DC5_CTL2		0x35
#define RICOH61x_LDOECO_SLP		0x4a

/* Interrupt enable register */
#define RICOH61x_INT_EN_SYS		0x12
#define RICOH61x_INT_EN_DCDC		0x40
#define RICOH61x_INT_EN_RTC		0xAE
#define RICOH61x_INT_EN_ADC1		0x88
#define RICOH61x_INT_EN_ADC2		0x89
#define RICOH61x_INT_EN_ADC3		0x8A
#define RICOH61x_INT_EN_GPIO		0x94
#define RICOH61x_INT_EN_GPIO2		0x94 /* dummy */
#define RICOH61x_INT_MSK_CHGCTR		0xBE
#define RICOH61x_INT_MSK_CHGSTS1	0xBF
#define RICOH61x_INT_MSK_CHGSTS2	0xC0
#define RICOH61x_INT_MSK_CHGERR		0xC1
#define RICOH61x_INT_MSK_CHGEXTIF	0xD1

/* Interrupt select register */
#define RICOH61x_CHG_CTRL_DETMOD1	0xCA
#define RICOH61x_CHG_CTRL_DETMOD2	0xCB
#define RICOH61x_CHG_STAT_DETMOD1	0xCC
#define RICOH61x_CHG_STAT_DETMOD2	0xCD
#define RICOH61x_CHG_STAT_DETMOD3	0xCE


/* interrupt status registers (monitor regs)*/
#define RICOH61x_INTC_INTPOL		0x9C
#define RICOH61x_INTC_INTEN		0x9D
#define RICOH61x_INTC_INTMON		0x9E

#define RICOH61x_INT_MON_SYS		0x14
#define RICOH61x_INT_MON_DCDC		0x42
#define RICOH61x_INT_MON_RTC		0xAF

#define RICOH61x_INT_MON_CHGCTR		0xC6
#define RICOH61x_INT_MON_CHGSTS1	0xC7
#define RICOH61x_INT_MON_CHGSTS2	0xC8
#define RICOH61x_INT_MON_CHGERR		0xC9
#define RICOH61x_INT_MON_CHGEXTIF	0xD3

/* interrupt clearing registers */
#define RICOH61x_INT_IR_SYS		0x13
#define RICOH61x_INT_IR_DCDC		0x41
#define RICOH61x_INT_IR_RTC		0xAF
#define RICOH61x_INT_IR_ADCL		0x8C
#define RICOH61x_INT_IR_ADCH		0x8D
#define RICOH61x_INT_IR_ADCEND		0x8E
#define RICOH61x_INT_IR_GPIOR		0x95
#define RICOH61x_INT_IR_GPIOF		0x96
#define RICOH61x_INT_IR_CHGCTR		0xC2
#define RICOH61x_INT_IR_CHGSTS1		0xC3
#define RICOH61x_INT_IR_CHGSTS2		0xC4
#define RICOH61x_INT_IR_CHGERR		0xC5
#define RICOH61x_INT_IR_CHGEXTIF	0xD2

/* GPIO register base address */
#define RICOH61x_GPIO_IOSEL		0x90
#define RICOH61x_GPIO_IOOUT		0x91
#define RICOH61x_GPIO_GPEDGE1		0x92
#define RICOH61x_GPIO_GPEDGE2		0x93
/* #define RICOH61x_GPIO_EN_GPIR	0x94 */
/* #define RICOH61x_GPIO_IR_GPR		0x95 */
/* #define RICOH61x_GPIO_IR_GPF		0x96 */
#define RICOH61x_GPIO_MON_IOIN		0x97
#define RICOH61x_GPIO_LED_FUNC		0x98

#define RICOH61x_REG_BANKSEL		0xFF

/* Charger Control register */
#define RICOH61x_CHG_CTL1		0xB3
#define	TIMSET_REG			0xB9

/* ADC Control register */
#define RICOH61x_ADC_CNT1		0x64
#define RICOH61x_ADC_CNT2		0x65
#define RICOH61x_ADC_CNT3		0x66
#define RICOH61x_ADC_VADP_THL		0x7C
#define RICOH61x_ADC_VSYS_THL		0x80

#define	RICOH61x_FG_CTRL		0xE0
#define	RICOH61x_PSWR			0x07

/* RICOH61x IRQ definitions */
enum {
	RICOH61x_IRQ_POWER_ON,
	RICOH61x_IRQ_EXTIN,
	RICOH61x_IRQ_PRE_VINDT,
	RICOH61x_IRQ_PREOT,
	RICOH61x_IRQ_POWER_OFF,
	RICOH61x_IRQ_NOE_OFF,
	RICOH61x_IRQ_WD,
	RICOH61x_IRQ_CLK_STP,

	RICOH61x_IRQ_DC1LIM,
	RICOH61x_IRQ_DC2LIM,
	RICOH61x_IRQ_DC3LIM,
	RICOH61x_IRQ_DC4LIM,
	RICOH61x_IRQ_DC5LIM,

	RICOH61x_IRQ_ILIMLIR,
	RICOH61x_IRQ_VBATLIR,
	RICOH61x_IRQ_VADPLIR,
	RICOH61x_IRQ_VUSBLIR,
	RICOH61x_IRQ_VSYSLIR,
	RICOH61x_IRQ_VTHMLIR,
	RICOH61x_IRQ_AIN1LIR,
	RICOH61x_IRQ_AIN0LIR,

	RICOH61x_IRQ_ILIMHIR,
	RICOH61x_IRQ_VBATHIR,
	RICOH61x_IRQ_VADPHIR,
	RICOH61x_IRQ_VUSBHIR,
	RICOH61x_IRQ_VSYSHIR,
	RICOH61x_IRQ_VTHMHIR,
	RICOH61x_IRQ_AIN1HIR,
	RICOH61x_IRQ_AIN0HIR,

	RICOH61x_IRQ_ADC_ENDIR,

	RICOH61x_IRQ_GPIO0,
	RICOH61x_IRQ_GPIO1,
	RICOH61x_IRQ_GPIO2,
	RICOH61x_IRQ_GPIO3,
	RICOH61x_IRQ_GPIO4,

	RICOH61x_IRQ_CTC,
	RICOH61x_IRQ_DALE,

	RICOH61x_IRQ_FVADPDETSINT,
	RICOH61x_IRQ_FVUSBDETSINT,
	RICOH61x_IRQ_FVADPLVSINT,
	RICOH61x_IRQ_FVUSBLVSINT,
	RICOH61x_IRQ_FWVADPSINT,
	RICOH61x_IRQ_FWVUSBSINT,

	RICOH61x_IRQ_FONCHGINT,
	RICOH61x_IRQ_FCHGCMPINT,
	RICOH61x_IRQ_FBATOPENINT,
	RICOH61x_IRQ_FSLPMODEINT,
	RICOH61x_IRQ_FBTEMPJTA1INT,
	RICOH61x_IRQ_FBTEMPJTA2INT,
	RICOH61x_IRQ_FBTEMPJTA3INT,
	RICOH61x_IRQ_FBTEMPJTA4INT,

	RICOH61x_IRQ_FCURTERMINT,
	RICOH61x_IRQ_FVOLTERMINT,
	RICOH61x_IRQ_FICRVSINT,
	RICOH61x_IRQ_FPOOR_CHGCURINT,
	RICOH61x_IRQ_FOSCFDETINT1,
	RICOH61x_IRQ_FOSCFDETINT2,
	RICOH61x_IRQ_FOSCFDETINT3,
	RICOH61x_IRQ_FOSCMDETINT,

	RICOH61x_IRQ_FDIEOFFINT,
	RICOH61x_IRQ_FDIEERRINT,
	RICOH61x_IRQ_FBTEMPERRINT,
	RICOH61x_IRQ_FVBATOVINT,
	RICOH61x_IRQ_FTTIMOVINT,
	RICOH61x_IRQ_FRTIMOVINT,
	RICOH61x_IRQ_FVADPOVSINT,
	RICOH61x_IRQ_FVUSBOVSINT,

	RICOH61x_IRQ_FGCDET,
	RICOH61x_IRQ_FPCDET,
	RICOH61x_IRQ_FWARN_ADP,

	/* Should be last entry */
	RICOH61x_NR_IRQS,
};

/* RICOH61x gpio definitions */
enum {
	RICOH61x_GPIO0,
	RICOH61x_GPIO1,
	RICOH61x_GPIO2,
	RICOH61x_GPIO3,
	RICOH61x_GPIO4,

	RICOH61x_NR_GPIO,
};

enum ricoh61x_sleep_control_id {
	RICOH619_DS_DC1,
	RICOH619_DS_DC1_SLP,
	RICOH619_DS_DC2,
	RICOH619_DS_DC3,
	RICOH619_DS_DC4,
	RICOH619_DS_DC5,
	RICOH619_DS_LDO1,
	RICOH619_DS_LDO2,
	RICOH619_DS_LDO3,
	RICOH619_DS_LDO4,
	RICOH619_DS_LDO5,
	RICOH619_DS_LDO6,
	RICOH619_DS_LDO7,
	RICOH619_DS_LDO8,
	RICOH619_DS_LDO9,
	RICOH619_DS_LDO10,
	RICOH619_DS_LDORTC1,
	RICOH619_DS_LDORTC2,
	RICOH619_DS_PSO0,
	RICOH619_DS_PSO1,
	RICOH619_DS_PSO2,
	RICOH619_DS_PSO3,
	RICOH619_DS_PSO4,
};


struct ricoh619_subdev_info {
	int			id;
	const char	*name;
	void		*platform_data;
};

struct ricoh619_gpio_init_data {
	unsigned output_mode_en:1;	/* Enable output mode during init */
	unsigned output_val:1;	/* Output value if it is in output mode */
	unsigned init_apply:1;	/* Apply init data on configuring gpios*/
	unsigned led_mode:1;	/* Select LED mode during init */
	unsigned led_func:1;	/* Set LED function if LED mode is 1 */
};

struct ricoh61x {
	struct device		*dev;
	struct i2c_client	*client;
	struct mutex		io_lock;
        struct jz_notifier      ricoh61x_notifier;
	int			gpio_base;
	struct gpio_chip	gpio_chip;
	int			irq_base;
/*	struct irq_chip		irq_chip; */
	int			chip_irq;
	struct mutex		irq_lock;
	unsigned long		group_irq_en[MAX_MAIN_INTERRUPT];

	/* For main interrupt bits in INTC */
	u8			intc_inten_cache;
	u8			intc_inten_reg;

	/* For group interrupt bits and address */
	u8			irq_en_cache[MAX_INTERRUPT_MASKS];
	u8			irq_en_reg[MAX_INTERRUPT_MASKS];

	/* For gpio edge */
	u8			gpedge_cache[MAX_GPEDGE_REG];
	u8			gpedge_reg[MAX_GPEDGE_REG];

	int			bank_num;
};

struct ricoh619_platform_data {
	int		num_subdevs;
	struct	ricoh619_subdev_info *subdevs;
	int (*init_port)(int irq_num); /* Init GPIO for IRQ pin */
	int		gpio_base;
	int		irq_base;
	struct ricoh619_gpio_init_data *gpio_init_data;
	int num_gpioinit_data;
	bool enable_shutdown_pin;
};

/* ==================================== */
/* RICOH61x Power_Key device data	*/
/* ==================================== */
struct ricoh619_pwrkey_platform_data {
	int irq;
	unsigned long delay_ms;
};
extern int pwrkey_wakeup;
/* ==================================== */
/* RICOH61x battery device data		*/
/* ==================================== */
extern int g_soc;
extern int g_fg_on_mode;

extern int ricoh61x_read(struct device *dev, uint8_t reg, uint8_t *val);
extern int ricoh61x_read_bank1(struct device *dev, uint8_t reg, uint8_t *val);
extern int ricoh61x_bulk_reads(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int ricoh61x_bulk_reads_bank1(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int ricoh61x_write(struct device *dev, u8 reg, uint8_t val);
extern int ricoh61x_write_bank1(struct device *dev, u8 reg, uint8_t val);
extern int ricoh61x_bulk_writes(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int ricoh61x_bulk_writes_bank1(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int ricoh61x_set_bits(struct device *dev, u8 reg, uint8_t bit_mask);
extern int ricoh61x_clr_bits(struct device *dev, u8 reg, uint8_t bit_mask);
extern int ricoh61x_update(struct device *dev, u8 reg, uint8_t val,
								uint8_t mask);
extern int ricoh61x_update_bank1(struct device *dev, u8 reg, uint8_t val,
								uint8_t mask);
extern int ricoh619_power_off(void);
extern int ricoh61x_irq_init(struct ricoh61x *ricoh61x, int irq, int irq_base);
extern int ricoh61x_irq_exit(struct ricoh61x *ricoh61x);

#endif
