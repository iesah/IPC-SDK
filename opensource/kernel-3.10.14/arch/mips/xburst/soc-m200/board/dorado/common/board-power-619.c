/*
 * board-power-619.c
 *
 * Copyright (C) 2012-2014, Ricoh Company,Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
#include <linux/i2c.h>
#include <linux/pda_power.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/ricoh619.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/ricoh619-regulator.h>
#include <linux/power/ricoh619_battery.h>
#include <linux/rtc.h>
#include "board_base.h"

#define PMIC_IRQ	0

/* If don't use the GPIO function, Set this macro to -1 */
#define PLATFORM_RICOH_GPIO_BASE	(-1)
#define INT_PMIC_BASE	IRQ_RESERVED_BASE

#define PMC_CTRL		0x0
#define PMC_CTRL_INTR_LOW	(1 << 17)

/* RICOH619 IRQs */
#define RICOH619_IRQ_BASE	INT_PMIC_BASE
#define RICOH619_GPIO_BASE  	PLATFORM_RICOH_GPIO_BASE
#define RICOH619_GPIO_IRQ   	(RICOH619_GPIO_BASE + 8)

static struct regulator_consumer_supply ricoh619_dc1_supply_0[] = {
	REGULATOR_SUPPLY(DC1_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_dc2_supply_0[] = {
	REGULATOR_SUPPLY(DC2_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_dc3_supply_0[] = {
	REGULATOR_SUPPLY(DC3_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_dc4_supply_0[] = {
	REGULATOR_SUPPLY(DC4_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_dc5_supply_0[] = {
	REGULATOR_SUPPLY(DC5_NAME, NULL),
};
static struct regulator_consumer_supply ricoh619_ldo1_supply_0[] = {
	REGULATOR_SUPPLY(LDO1_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_ldo2_supply_0[] = {
	REGULATOR_SUPPLY(LDO2_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_ldo3_supply_0[] = {
	REGULATOR_SUPPLY(LDO3_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_ldo4_supply_0[] = {
	REGULATOR_SUPPLY(LDO4_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_ldo5_supply_0[] = {
	REGULATOR_SUPPLY(LDO5_NAME, NULL),
};

static struct regulator_consumer_supply ricoh619_ldo6_supply_0[] = {
	REGULATOR_SUPPLY(LDO6_NAME, NULL),
};
static struct regulator_consumer_supply ricoh619_ldo7_supply_0[] = {
	REGULATOR_SUPPLY(LDO7_NAME, NULL),
};
static struct regulator_consumer_supply ricoh619_ldo8_supply_0[] = {
	REGULATOR_SUPPLY(LDO8_NAME, NULL),
};
static struct regulator_consumer_supply ricoh619_ldo9_supply_0[] = {
	REGULATOR_SUPPLY(LDO9_NAME, NULL),
};
static struct regulator_consumer_supply ricoh619_ldo10_supply_0[] = {
	REGULATOR_SUPPLY(LDO10_NAME, NULL),
};
static struct regulator_consumer_supply ricoh619_ldortc1_supply_0[] = {
	REGULATOR_SUPPLY(LDORTC1_NAME, NULL),
};
static struct regulator_consumer_supply ricoh619_ldortc2_supply_0[] = {
	REGULATOR_SUPPLY(LDORTC2_NAME, NULL),
};

#define RICOH_PDATA_INIT(_name, _sname, _minmv, _maxmv, _supply_reg, _always_on, \
	_boot_on, _apply_uv, _init_uV, _sleep_uV, _init_enable, _init_apply, _flags, \
	_ext_contol, _ds_slots) \
	static struct ricoh619_regulator_platform_data pdata_##_name##_##_sname = \
	{								\
		.regulator = {						\
			.constraints = {				\
				.min_uV = (_minmv)*1000,		\
				.max_uV = (_maxmv)*1000,		\
				.valid_modes_mask = (REGULATOR_MODE_NORMAL |  \
						     REGULATOR_MODE_STANDBY), \
				.valid_ops_mask = (REGULATOR_CHANGE_MODE |    \
						   REGULATOR_CHANGE_STATUS |  \
						   REGULATOR_CHANGE_VOLTAGE), \
				.always_on = _always_on,		\
				.boot_on = _boot_on,			\
				.apply_uV = _apply_uv,			\
			},						\
			.num_consumer_supplies =			\
				ARRAY_SIZE(ricoh619_##_name##_supply_##_sname),	\
			.consumer_supplies = ricoh619_##_name##_supply_##_sname, \
			.supply_regulator = _supply_reg,		\
		},							\
		.init_uV =  _init_uV * 1000,				\
		.sleep_uV = _sleep_uV * 1000,				\
		.init_enable = _init_enable,				\
		.init_apply = _init_apply,				\
		.sleep_slots = _ds_slots,				\
		.flags = _flags,					\
		.ext_pwr_req = _ext_contol,				\
	}

/* min_uV/max_uV : Please set the appropriate value for the devices that
  the power supplied within a range from min to max voltage according
  to RC5T619 specification. */
/*_name,_sname,_minmv,_maxmv,_supply_reg,_always_on,_boot_on,_apply_uv,_init_uV,_init_enable,_init_apply,
 * _flags,_ext_contol,_ds_slots) */
RICOH_PDATA_INIT(dc1, 0,	600,   1200, 0, DC1_ALWAYS_ON, DC1_BOOT_ON, 1,
		 DC1_INIT_UV, DC1_INIT_SLP_UV, DC1_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(dc2, 0,	600,   3500, 0, DC2_ALWAYS_ON, DC2_BOOT_ON, 1,
		 DC2_INIT_UV, DC2_INIT_SLP_UV, DC1_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(dc3, 0,	600,   3500, 0, DC3_ALWAYS_ON, DC3_BOOT_ON, 1,
		 DC3_INIT_UV, DC3_INIT_SLP_UV, DC3_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(dc4, 0,	600,   3500, 0, DC4_ALWAYS_ON, DC4_BOOT_ON, 1,
		 DC4_INIT_UV, DC4_INIT_SLP_UV, DC4_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(dc5, 0,	600,   3500, 0, DC5_ALWAYS_ON, DC5_BOOT_ON, 1,
		 DC5_INIT_UV, DC5_INIT_SLP_UV, DC5_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo1, 0,	900,   3500, 0, LDO1_ALWAYS_ON, LDO1_BOOT_ON, 1,
		 LDO1_INIT_UV, LDO1_INIT_SLP_UV, LDO1_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo2, 0,	900,   3500, 0, LDO2_ALWAYS_ON, LDO2_BOOT_ON, 1,
		 LDO2_INIT_UV, LDO2_INIT_SLP_UV, LDO2_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo3, 0,	900,   3500, 0, LDO3_ALWAYS_ON, LDO3_BOOT_ON, 1,
		 LDO3_INIT_UV, LDO3_INIT_SLP_UV, LDO3_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo4, 0,	900,   3500, 0, LDO4_ALWAYS_ON, LDO4_BOOT_ON, 1,
		 LDO4_INIT_UV, LDO4_INIT_SLP_UV, LDO4_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo5, 0,	600,   3500, 0, LDO5_ALWAYS_ON, LDO5_BOOT_ON, 1,
		 LDO5_INIT_UV, LDO5_INIT_SLP_UV, LDO5_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo6, 0,	600,   3500, 0, LDO6_ALWAYS_ON, LDO6_BOOT_ON, 1,
		 LDO6_INIT_UV, LDO6_INIT_SLP_UV, LDO6_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo7, 0,	900,   3500, 0, LDO7_ALWAYS_ON, LDO7_BOOT_ON, 1,
		 LDO7_INIT_UV, LDO7_INIT_SLP_UV, LDO7_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo8, 0,	900,   3500, 0, LDO8_ALWAYS_ON, LDO8_BOOT_ON, 1,
		 LDO8_INIT_UV, LDO8_INIT_SLP_UV, LDO8_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo9, 0,	900,   3500, 0, LDO9_ALWAYS_ON, LDO9_BOOT_ON, 1,
		 LDO9_INIT_UV, LDO9_INIT_SLP_UV, LDO9_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldo10, 0,	900,   3500, 0, LDO10_ALWAYS_ON, LDO10_BOOT_ON, 1,
		 LDO10_INIT_UV, LDO10_INIT_SLP_UV, LDO10_INIT_ENABLE, 1, 0, 0, 0);
RICOH_PDATA_INIT(ldortc1, 0,	1700,  3500, 0, LDORTC1_ALWAYS_ON, LDORTC1_BOOT_ON, 1,
		 LDORTC1_INIT_UV, LDORTC1_INIT_SLP_UV, LDORTC1_INIT_ENABLE, 1, -1, -1, -1);
RICOH_PDATA_INIT(ldortc2, 0,	900,   3500, 0, LDORTC2_ALWAYS_ON, LDORTC2_BOOT_ON, 1,
		 LDORTC2_INIT_UV, LDORTC2_INIT_SLP_UV, LDORTC2_INIT_ENABLE, 1, -1, -1, -1);

/*-------- if Ricoh RTC exists -----------*/
#ifdef CONFIG_RTC_DRV_R5T619
static struct ricoh_rtc_platform_data rtc_data = {
	.irq = RICOH619_IRQ_BASE,
	.time = {
		.tm_year = 1970,
		.tm_mon = 0,
		.tm_mday = 1,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	},
};

#define RICOH_RTC_REG						\
{								\
	.id		= 0,					\
	.name		= "rtc_ricoh619",			\
	.platform_data	= &rtc_data,				\
}
#endif
/*-------- if Ricoh RTC exists -----------*/

#define RICOH_REG(_id, _name, _sname)				\
{								\
	.id		= RICOH619_ID_##_id,			\
	.name		= "ricoh61x-regulator",			\
	.platform_data	= &pdata_##_name##_##_sname,		\
}

#define RICOH619_BATTERY_REG    				\
{								\
    .id			= -1,    				\
    .name		= "ricoh619-battery",     		\
    .platform_data	= &ricoh619_battery_data,    		\
}

//==========================================
//RICOH619 Power_Key device data
//==========================================
static struct ricoh619_pwrkey_platform_data ricoh619_pwrkey_data = {
	.irq 		= RICOH619_IRQ_BASE,
	.delay_ms 	= 20,
};
#define RICOH619_PWRKEY_REG 					\
{ 								\
	.id 		= -1, 					\
	.name 		= "ricoh619-pwrkey", 			\
	.platform_data 	= &ricoh619_pwrkey_data, 		\
}


static struct ricoh619_battery_platform_data ricoh619_battery_data = {
	.irq 		= RICOH619_IRQ_BASE,
	.alarm_vol_mv 	= 3300,
	// .adc_channel = RICOH619_ADC_CHANNEL_VBAT,
	.multiple	= 100, //100%
	.monitor_time 	= 60,
		/* some parameter is depend of battery type */
	/*the battery fof 4000mA 2000mA*/
	.type[0] = {
		.ch_vfchg 	= 0x02,	/* VFCHG	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.ch_vrchg 	= 0x4,	/* VRCHG	= 0 - 4 (3.85v, 3.90v, 3.95v, 4.00v, 4.10v) */
		.ch_vbatovset 	= 0x0,	/* VBATOVSET	= 0 or 1 (0 : 4.38v(up)/3.95v(down) 1: 4.53v(up)/4.10v(down)) */
		.ch_ichg 	= 0x09,	/* ICHG		= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_adp 	= 0x18,	/* ILIM_ADP	= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_usb 	= 0x04,	/* ILIM_USB	= 0 - 0x1D (100mA - 3000mA) */
		.ch_icchg 	= 0x03,	/* ICCHG	= 0 - 3 (50mA 100mA 150mA 200mA) */
		.fg_target_vsys = 3000,	/* This value is the target one to DSOC=0% */
		.fg_target_ibat = 200, /* This value is the target one to DSOC=0% */
		.fg_poff_vbat 	= 0, 	/* setting value of 0 per Vbat */
		.jt_en 		= 0,	/* JEITA Enable	  = 0 or 1 (1:enable, 0:disable) */
		.jt_hw_sw 	= 1,	/* JEITA HW or SW = 0 or 1 (1:HardWare, 0:SoftWare) */
		.jt_temp_h 	= 50,	/* degree C */
		.jt_temp_l 	= 12,	/* degree C */
		.jt_vfchg_h 	= 0x03,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_vfchg_l 	= 0,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_ichg_h 	= 0x0D,	/* VFCHG Low  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_ichg_l 	= 0x09,	/* ICHG Low   	= 0 - 0x1D (100mA - 3000mA) */
	},
	/*the battery for 260mA 500mA*/
	.type[1] = {
		.ch_vfchg 	= 0x03,	/* VFCHG	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.ch_vrchg 	= 0x01,	/* VRCHG	= 0 - 4 (3.85v, 3.90v, 3.95v, 4.00v, 4.10v) */
		.ch_vbatovset 	= 0x0,	/* VBATOVSET	= 0 or 1 (0 : 4.38v(up)/3.95v(down) 1: 4.53v(up)/4.10v(down)) */
		.ch_ichg 	= 0x03,	/* ICHG		= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_adp 	= 0x18,	/* ILIM_ADP	= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_usb 	= 0x04,	/* ILIM_USB	= 0 - 0x1D (100mA - 3000mA) */
		.ch_icchg 	= 0x00,	/* ICCHG	= 0 - 3 (50mA 100mA 150mA 200mA) */
		.fg_target_vsys = 3000,	/* This value is the target one to DSOC=0% */
		.fg_target_ibat = 100, /* This value is the target one to DSOC=0% */
		.fg_poff_vbat 	= 0, 	/* setting value of 0 per Vbat */
		.fg_rsense_val	= 100,	/* setting value of R Sense */
		.jt_en 		= 0,	/* JEITA Enable	  = 0 or 1 (1:enable, 0:disable) */
		.jt_hw_sw 	= 1,	/* JEITA HW or SW = 0 or 1 (1:HardWare, 0:SoftWare) */
		.jt_temp_h 	= 50,	/* degree C */
		.jt_temp_l 	= 12,	/* degree C */
		.jt_vfchg_h 	= 0x03,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_vfchg_l 	= 0,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_ichg_h 	= 0x04,	/* ICHG Hi   	= 0 - 0x1D (100mA - 3000mA) */
		.jt_ichg_l 	= 0x01,	/* ICHG Low   	= 0 - 0x1D (100mA - 3000mA) */
	},
	/*
	.type[2] = {
		.ch_vfchg = 0x0,
		.ch_vrchg = 0x0,
		.ch_vbatovset = 0x0,
		.ch_ichg = 0x0,
		.ch_ilim_adp = 0x0,
		.ch_ilim_usb = 0x0,
		.ch_icchg = 0x00,
		.fg_target_vsys = 3300,//3000,
		.fg_target_ibat = 1000,//1000,
		.jt_en = 0,
		.jt_hw_sw = 1,
		.jt_temp_h = 40,
		.jt_temp_l = 10,
		.jt_vfchg_h = 0x0,
		.jt_vfchg_l = 0,
		.jt_ichg_h = 0x01,
		.jt_ichg_l = 0x01,
	},
	*/

/*  JEITA Parameter
*
*          VCHG
*            |
* jt_vfchg_h~+~~~~~~~~~~~~~~~~~~~+
*            |                   |
* jt_vfchg_l-| - - - - - - - - - +~~~~~~~~~~+
*            |    Charge area    +          |
*  -------0--+-------------------+----------+--- Temp
*            !                   +
*          ICHG
*            |                   +
*  jt_ichg_h-+ - -+~~~~~~~~~~~~~~+~~~~~~~~~~+
*            +    |              +          |
*  jt_ichg_l-+~~~~+   Charge area           |
*            |    +              +          |
*         0--+----+--------------+----------+--- Temp
*            0   jt_temp_l      jt_temp_h   55
*/
};



#define RICOH619_DEV_REG 		\
	RICOH_REG(DC1, dc1, 0),		\
	RICOH_REG(DC2, dc2, 0),		\
	RICOH_REG(DC3, dc3, 0),		\
	RICOH_REG(DC4, dc4, 0),		\
	RICOH_REG(DC5, dc5, 0),		\
	RICOH_REG(LDO1, ldo1, 0),	\
	RICOH_REG(LDO2, ldo2, 0),	\
	RICOH_REG(LDO3, ldo3, 0),	\
	RICOH_REG(LDO4, ldo4, 0),	\
	RICOH_REG(LDO5, ldo5, 0),	\
	RICOH_REG(LDO6, ldo6, 0),	\
	RICOH_REG(LDO7, ldo7, 0),	\
	RICOH_REG(LDO8, ldo8, 0),	\
	RICOH_REG(LDO9, ldo9, 0),	\
	RICOH_REG(LDO10, ldo10, 0),	\
	RICOH_REG(LDORTC1, ldortc1, 0),	\
	RICOH_REG(LDORTC2, ldortc2, 0)

static struct ricoh619_subdev_info ricoh_devs_dcdc[] = {
	RICOH619_DEV_REG,
	RICOH619_BATTERY_REG,
	RICOH619_PWRKEY_REG,
#ifdef CONFIG_RTC_DRV_R5T619
	RICOH_RTC_REG,
#endif
};

#define RICOH_GPIO_INIT(_init_apply, _output_mode, _output_val, _led_mode, _led_func) \
	{									\
		.output_mode_en = _output_mode,		\
		.output_val		= _output_val,	\
		.init_apply		= _init_apply,	\
		.led_mode		= _led_mode,	\
		.led_func		= _led_func,	\
	}
struct ricoh619_gpio_init_data ricoh_gpio_data[] = {
	RICOH_GPIO_INIT(false, false, 0, 0, 0),
	RICOH_GPIO_INIT(false, false, 0, 0, 0),
	RICOH_GPIO_INIT(false, false, 0, 0, 0),
	RICOH_GPIO_INIT(false, false, 0, 0, 0),
};

static struct ricoh619_platform_data ricoh_platform = {
	.num_subdevs		= ARRAY_SIZE(ricoh_devs_dcdc),
	.subdevs		= ricoh_devs_dcdc,
	.irq_base		= RICOH619_IRQ_BASE,
	.gpio_base		= RICOH619_GPIO_BASE,
	.gpio_init_data		= ricoh_gpio_data,
	.num_gpioinit_data	= ARRAY_SIZE(ricoh_gpio_data),
	.enable_shutdown_pin 	= true,
};

struct i2c_board_info __initdata ricoh619_regulator = {
	I2C_BOARD_INFO("ricoh619", 0x32),
	.irq		= PMIC_IRQ,
	.platform_data	= &ricoh_platform,
};

//FIXED_REGULATOR_DEF(usi_vbat, "USI_VDD", 3900*1000 ,GPIO_WLAN_PW_EN, 1, 0, 0, NULL,"usi_vbat", NULL);
static struct platform_device *fixed_regulator_devices[] __initdata = {
	//&usi_vbat_regulator_device,
};

static int __init pmu_dev_init(void)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	int busnum = PMU_I2C_BUSNUM;
	int i;

	if (gpio_request_one(PMU_IRQ_N,
				GPIOF_DIR_IN, "ricoh619_irq")) {
		pr_err("The GPIO %d is requested by other driver,"
				" not available for ricoh619\n", PMU_IRQ_N);
		ricoh619_regulator.irq = -1;
	} else {
		ricoh619_regulator.irq = gpio_to_irq(PMU_IRQ_N);
	}

	adap = i2c_get_adapter(busnum);
	if (!adap) {
		pr_err("failed to get adapter i2c%d\n", busnum);
		return -1;
	}

	client = i2c_new_device(adap, &ricoh619_regulator);
	if (!client) {
		pr_err("failed to register pmu to i2c%d\n", busnum);
		return -1;
	}

	i2c_put_adapter(adap);

	for (i = 0; i < ARRAY_SIZE(fixed_regulator_devices); i++)
		fixed_regulator_devices[i]->id = i;

	return platform_add_devices(fixed_regulator_devices,
				    ARRAY_SIZE(fixed_regulator_devices));
}
subsys_initcall_sync(pmu_dev_init);
