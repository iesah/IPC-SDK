/*
 * drivers/power/ricoh619-battery.c
 *
 * Charger driver for RICOH R5T619 power management chip.
 *
 * Copyright (C) 2012-2014 RICOH COMPANY,LTD
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
#define RICOH61x_BATTERY_VERSION "RICOH61x_BATTERY_VERSION: 2014.7.21 V3.1.1.1"


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/power_supply.h>
#include <linux/mfd/ricoh619.h>
#include <linux/power/ricoh619_battery.h>
#include <linux/power/ricoh61x_battery_init.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/irq.h>


/* define for function */
#define ENABLE_FUEL_GAUGE_FUNCTION
#define ENABLE_LOW_BATTERY_DETECTION
#define ENABLE_FACTORY_MODE
#define DISABLE_CHARGER_TIMER
/* #define ENABLE_FG_KEEP_ON_MODE */
#define ENABLE_OCV_TABLE_CALIB
/* #define ENABLE_MASKING_INTERRUPT_IN_SLEEP */




/* FG setting */
#define RICOH61x_REL1_SEL_VALUE		64
#define RICOH61x_REL2_SEL_VALUE		0

enum int_type {
	SYS_INT  = 0x01,
	DCDC_INT = 0x02,
	ADC_INT  = 0x08,
	GPIO_INT = 0x10,
	CHG_INT	 = 0x40,
};

#ifdef ENABLE_FUEL_GAUGE_FUNCTION
/* define for FG delayed time */
#define RICOH61x_MONITOR_START_TIME		15
#define RICOH61x_FG_RESET_TIME			6
#define RICOH61x_FG_STABLE_TIME			120
#define RICOH61x_DISPLAY_UPDATE_TIME		15
#define RICOH61x_LOW_VOL_DOWN_TIME		10
#define RICOH61x_CHARGE_MONITOR_TIME		20
#define RICOH61x_CHARGE_RESUME_TIME		1
#define RICOH61x_CHARGE_CALC_TIME		1
#define RICOH61x_JEITA_UPDATE_TIME		60
#define RICOH61x_DELAY_TIME			60
/* define for FG parameter */
#define RICOH61x_MAX_RESET_SOC_DIFF		5
#define RICOH61x_GET_CHARGE_NUM			10
#define RICOH61x_UPDATE_COUNT_DISP		4
#define RICOH61x_UPDATE_COUNT_FULL		4
#define RICOH61x_UPDATE_COUNT_FULL_RESET 	7
#define RICOH61x_CHARGE_UPDATE_TIME		3
#define RICOH61x_FULL_WAIT_TIME			4
#define RE_CAP_GO_DOWN				10	/* 40 */
#define RICOH61x_ENTER_LOW_VOL			70
#define RICOH61x_TAH_SEL2			5
#define RICOH61x_TAL_SEL2			6

#define RICOH61x_OCV_OFFSET_BOUND	3
#define RICOH61x_OCV_OFFSET_RATIO	2

#define RICOH61x_ENTER_FULL_STATE_OCV	9
#define RICOH61x_ENTER_FULL_STATE_DSOC	90

/* define for FG status */
enum {
	RICOH61x_SOCA_START,
	RICOH61x_SOCA_UNSTABLE,
	RICOH61x_SOCA_FG_RESET,
	RICOH61x_SOCA_DISP,
	RICOH61x_SOCA_STABLE,
	RICOH61x_SOCA_ZERO,
	RICOH61x_SOCA_FULL,
	RICOH61x_SOCA_LOW_VOL,
};
#endif

#ifdef ENABLE_LOW_BATTERY_DETECTION
#define LOW_BATTERY_DETECTION_TIME		10
#endif

struct ricoh61x_soca_info {
	int Rbat;
	int n_cap;
	int ocv_table_def[11];
	int ocv_table[11];
	int ocv_table_low[11];
	int soc;		/* Latest FG SOC value */
	int displayed_soc;
	int suspend_soc;
	int status;		/* SOCA status 0: Not initial; 5: Finished */
	int stable_count;
	int chg_status;		/* chg_status */
	int soc_delta;		/* soc delta for status3(DISP) */
	int cc_delta;
	int cc_cap_offset;
	int last_soc;
	int last_displayed_soc;
	int ready_fg;
	int reset_count;
	int reset_soc[3];
	int chg_cmp_times;
	int dischg_state;
	int Vbat[RICOH61x_GET_CHARGE_NUM];
	int Vsys[RICOH61x_GET_CHARGE_NUM];
	int Ibat[RICOH61x_GET_CHARGE_NUM];
	int Vbat_ave;
	int Vbat_old;
	int Vsys_ave;
	int Ibat_ave;
	int chg_count;
	int full_reset_count;
	int soc_full;
	int fc_cap;
	/* for LOW VOL state */
	int target_use_cap;
	int hurry_up_flg;
	int zero_flg;
	int re_cap_old;
	int cutoff_ocv;
	int Rsys;
	int target_vsys;
	int target_ibat;
	int jt_limit;
	int OCV100_min;
	int OCV100_max;
	int R_low;
	int rsoc_ready_flag;
	int init_pswr;
	int last_cc_sum;
};

struct ricoh61x_battery_info {
	struct device      *dev;
	struct power_supply	battery;
	struct delayed_work	monitor_work;
	struct delayed_work	displayed_work;
	struct delayed_work	charge_stable_work;
	struct delayed_work	changed_work;
#ifdef ENABLE_LOW_BATTERY_DETECTION
	struct delayed_work	low_battery_work;
#endif
	struct delayed_work	charge_monitor_work;
	struct delayed_work	get_charge_work;
	struct delayed_work	jeita_work;

	struct work_struct	irq_work;	/* for Charging & VADP/VUSB */

	struct workqueue_struct *monitor_wqueue;
	struct workqueue_struct *workqueue;	/* for Charging & VUSB/VADP */

#ifdef ENABLE_FACTORY_MODE
	struct delayed_work	factory_mode_work;
	struct workqueue_struct *factory_mode_wqueue;
#endif

	struct mutex		lock;
	unsigned long		monitor_time;
	int		adc_vdd_mv;
	int		multiple;
	int		alarm_vol_mv;
	int		status;
	int		min_voltage;
	int		max_voltage;
	int		cur_voltage;
	int		capacity;
	int		battery_temp;
	int		time_to_empty;
	int		time_to_full;
	int		chg_ctr;
	int		chg_stat1;
	unsigned	present:1;
	u16		delay;
	struct		ricoh61x_soca_info *soca;
	int		first_pwon;
	bool		entry_factory_mode;
	int		ch_vfchg;
	int		ch_vrchg;
	int		ch_vbatovset;
	int		ch_ichg;
	int		ch_ilim_adp;
	int		ch_ilim_usb;
	int		ch_icchg;
	int		fg_target_vsys;
	int		fg_target_ibat;
	int		fg_poff_vbat;
	int		fg_rsense_val;
	int		jt_en;
	int		jt_hw_sw;
	int		jt_temp_h;
	int		jt_temp_l;
	int		jt_vfchg_h;
	int		jt_vfchg_l;
	int		jt_ichg_h;
	int		jt_ichg_l;
	uint8_t 	adp_current_val;
	uint8_t 	usb_current_val;

	int 		num;
	};

int g_full_flag;
int charger_irq;
int g_soc;
int g_fg_on_mode;

/*This is for full state*/
static int BatteryTableFlageDef=0;
static int BatteryTypeDef=0;
static int Battery_Type(void)
{
#ifdef CONFIG_LARGE_CAPACITY_BATTERY
	BatteryTypeDef = 0;
#elif CONFIG_SMALL_CAPACITY_BATTERY
	BatteryTypeDef = 1;
#endif
	return BatteryTypeDef;
}

static int Battery_Table(void)
{
	return BatteryTableFlageDef;
}

static void ricoh61x_battery_work(struct work_struct *work)
{
	struct ricoh61x_battery_info *info = container_of(work,
		struct ricoh61x_battery_info, monitor_work.work);

	power_supply_changed(&info->battery);
	queue_delayed_work(info->monitor_wqueue, &info->monitor_work,
			   info->monitor_time);
}

#ifdef ENABLE_FUEL_GAUGE_FUNCTION
static int measure_vbatt_FG(struct ricoh61x_battery_info *info, int *data);
static int measure_Ibatt_FG(struct ricoh61x_battery_info *info, int *data);
static int calc_capacity(struct ricoh61x_battery_info *info);
static int calc_capacity_2(struct ricoh61x_battery_info *info);
static int get_OCV_init_Data(struct ricoh61x_battery_info *info, int index);
static int get_OCV_voltage(struct ricoh61x_battery_info *info, int index);
static int get_check_fuel_gauge_reg(struct ricoh61x_battery_info *info,
					 int Reg_h, int Reg_l, int enable_bit);
static int calc_capacity_in_period(struct ricoh61x_battery_info *info,
				 int *cc_cap, bool *is_charging, bool cc_rst);
static int get_charge_priority(struct ricoh61x_battery_info *info, bool *data);
static int set_charge_priority(struct ricoh61x_battery_info *info, bool *data);
static int get_power_supply_status(struct ricoh61x_battery_info *info);
static int get_power_supply_Android_status(struct ricoh61x_battery_info *info);
static int measure_vsys_ADC(struct ricoh61x_battery_info *info, int *data);
static int Calc_Linear_Interpolation(int x0, int y0, int x1, int y1, int y);
static int get_battery_temp(struct ricoh61x_battery_info *info);
static int get_battery_temp_2(struct ricoh61x_battery_info *info);
static int check_jeita_status(struct ricoh61x_battery_info *info, bool *is_jeita_updated);
static void ricoh61x_scaling_OCV_table(struct ricoh61x_battery_info *info, int cutoff_vol, int full_vol, int *start_per, int *end_per);
static int ricoh61x_Check_OCV_Offset(struct ricoh61x_battery_info *info);

static int calc_ocv(struct ricoh61x_battery_info *info)
{
	int Vbat = 0;
	int Ibat = 0;
	int ret;
	int ocv;

	ret = measure_vbatt_FG(info, &Vbat);
	ret = measure_Ibatt_FG(info, &Ibat);

	ocv = Vbat - Ibat * info->soca->Rbat;

	return ocv;
}


static int set_Rlow(struct ricoh61x_battery_info *info)
{
	int err;
	int Rbat_low_max;
	uint8_t val;
	int Vocv;
	int temp;

	if (info->soca->Rbat == 0)
			info->soca->Rbat = get_OCV_init_Data(info, 12) * 1000 / 512
							 * 5000 / 4095;

	Vocv = calc_ocv(info);
	Rbat_low_max = info->soca->Rbat * 1.5;

	if (Vocv < get_OCV_voltage(info,3))
	{
		info->soca->R_low = Calc_Linear_Interpolation(info->soca->Rbat,get_OCV_voltage(info,3),
			Rbat_low_max, get_OCV_voltage(info,0), Vocv);
		pr_debug("PMU: Modify RBAT from %d to %d ", info->soca->Rbat, info->soca->R_low);
		temp = info->soca->R_low *4095/5000*512/1000;

		val = temp >> 8;
		err = ricoh61x_write_bank1(info->dev->parent, 0xD4, val);
		if (err < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			return err;
		}

		val = info->soca->R_low & 0xff;
		err = ricoh61x_write_bank1(info->dev->parent, 0xD5, val);
		if (err < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			return err;
		}
	}
	else  info->soca->R_low = 0;


	return err;
}

static int Set_back_ocv_table(struct ricoh61x_battery_info *info)
{
	int err;
	uint8_t val;
	int temp;
	int i;
	uint8_t debug_disp[22];

	/* Modify back ocv table */

	if (0 != info->soca->ocv_table_low[0])
	{
		for (i = 0 ; i < 11; i++){
			battery_init_para[info->num][i*2 + 1] = info->soca->ocv_table_low[i];
			battery_init_para[info->num][i*2] = info->soca->ocv_table_low[i] >> 8;
		}
		err = ricoh61x_clr_bits(info->dev->parent, FG_CTRL_REG, 0x01);

		err = ricoh61x_bulk_writes_bank1(info->dev->parent,
			BAT_INIT_TOP_REG, 22, battery_init_para[info->num]);

		err = ricoh61x_set_bits(info->dev->parent, FG_CTRL_REG, 0x01);

		/* debug comment start*/
		err = ricoh61x_bulk_reads_bank1(info->dev->parent,
			BAT_INIT_TOP_REG, 22, debug_disp);
		for (i = 0; i < 11; i++){
			pr_debug("PMU : %s : after OCV table %d 0x%x\n",__func__, i * 10, (debug_disp[i*2] << 8 | debug_disp[i*2+1]));
		}
		/* end */
		/* clear table*/
		for(i = 0; i < 11; i++)
		{
			info->soca->ocv_table_low[i] = 0;
		}
	}

	/* Modify back Rbat */
	if (0!=info->soca->R_low)
	{
		pr_debug("PMU: Modify back RBAT from %d to %d ",  info->soca->R_low,info->soca->Rbat);
		temp = info->soca->Rbat*4095/5000*512/1000;

		val = temp >> 8;
		err = ricoh61x_write_bank1(info->dev->parent, 0xD4, val);
		if (err < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			return err;
		}

		val = info->soca->R_low & 0xff;
		err = ricoh61x_write_bank1(info->dev->parent, 0xD5, val);
		if (err < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			return err;
		}

		info->soca->R_low = 0;
	}
	return 0;
}

static int set_cc_sum_back(struct ricoh61x_battery_info *info, int *val)
{
	int val_set_back;
	uint8_t 	cc_sum_reg[4];
	uint8_t 	fa_cap_reg[2];
	uint16_t 	fa_cap;
	int32_t 	cc_cap_temp;
	uint32_t 	cc_cap_min;
	uint32_t 	cc_sum;
	int err;
	int value = *val;

	err = ricoh61x_bulk_reads(info->dev->parent,
				 FA_CAP_H_REG, 2, fa_cap_reg);
	if (err < 0)
		return -1;

	/* fa_cap = *(uint16_t*)fa_cap_reg & 0x7fff; */
	fa_cap = (fa_cap_reg[0] << 8 | fa_cap_reg[1]) & 0x7fff;

	if(fa_cap == 0)
		return -1;
	else
		if (value>=0)
			{
				cc_sum = value *9 * fa_cap/25;
			}
		else
			{
				cc_sum = -value*9 * fa_cap /25;
				cc_sum = cc_sum -0x01;
				cc_sum = cc_sum^0xffffffff;
			}
	pr_debug("Ross's CC_SUM is %d \n",cc_sum);
	pr_debug("VALis %d \n",value);

	cc_sum_reg[3]= cc_sum & 0xff;
	cc_sum_reg[2]= (cc_sum & 0xff00)>> 8;
	cc_sum_reg[1]= (cc_sum & 0xff0000) >> 16;
	cc_sum_reg[0]= (cc_sum & 0xff000000) >> 24;

	pr_debug("reg0~3 is %x %x %x %x \n",cc_sum_reg[0],cc_sum_reg[1],cc_sum_reg[2],cc_sum_reg[3]);

	err = ricoh61x_set_bits(info->dev->parent, CC_CTRL_REG, 0x01);
	if (err < 0)
		return -1;

	err = ricoh61x_bulk_writes(info->dev->parent,
			CC_SUMREG3_REG, 4, cc_sum_reg);
	if (err < 0)
		return -1;

	/* CC_pause exit */
	err = ricoh61x_clr_bits(info->dev->parent, CC_CTRL_REG, 0x01);
	if (err < 0)
		return -1;

	return 0;
}

static int set_pswr(struct ricoh61x_battery_info *info)
{
	int val;
	int err;
	if ((info->soca->displayed_soc + 50)/100 <= 1) {
				val = 1;
			} else {
				val = (info->soca->displayed_soc + 50)/100;
				val &= 0x7f;
			}
	err = ricoh61x_write(info->dev->parent, PSWR_REG, val);
	pr_debug("display_work:writing display PSWR_REG\n");
	if (err < 0)
		pr_debug("display_work:Error in writing PSWR_REG\n");

	g_soc = val;
	return 1;
}

/**
**/
static int ricoh61x_Check_OCV_Offset(struct ricoh61x_battery_info *info)
{
	int ocv_table[11]; // HEX value
	int i;
	int temp;
	int ret;
	uint8_t debug_disp[22];
	uint8_t val = 0;

	pr_debug("PMU : %s : calc ocv %d get OCV %d\n",__func__,calc_ocv(info),get_OCV_voltage(info, RICOH61x_OCV_OFFSET_BOUND));

	/* check adp/usb status */
	ret = ricoh61x_read(info->dev->parent, CHGSTATE_REG, &val);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return 0;
	}

	val = (val & 0xC0) >> 6;

	if (val != 0){ /* connect adp or usb */
		if (calc_ocv(info) < get_OCV_voltage(info, RICOH61x_OCV_OFFSET_BOUND) )
		{
			if(0 == info->soca->ocv_table_low[0]){
				for (i = 0 ; i < 11; i++){
				ocv_table[i] = (battery_init_para[info->num][i*2]<<8) | (battery_init_para[info->num][i*2+1]);
				pr_debug("PMU : %s : OCV table %d 0x%x\n",__func__,i * 10, ocv_table[i]);
				info->soca->ocv_table_low[i] = ocv_table[i];
				}

				for (i = 0 ; i < 11; i++){
					temp = ocv_table[i] * (100 + RICOH61x_OCV_OFFSET_RATIO) / 100;

					battery_init_para[info->num][i*2 + 1] = temp;
					battery_init_para[info->num][i*2] = temp >> 8;
				}
				ret = ricoh61x_clr_bits(info->dev->parent, FG_CTRL_REG, 0x01);

				ret = ricoh61x_bulk_writes_bank1(info->dev->parent,
					BAT_INIT_TOP_REG, 22, battery_init_para[info->num]);

				ret = ricoh61x_set_bits(info->dev->parent, FG_CTRL_REG, 0x01);

				/* debug comment start*/
				ret = ricoh61x_bulk_reads_bank1(info->dev->parent,
					BAT_INIT_TOP_REG, 22, debug_disp);
				for (i = 0; i < 11; i++){
					pr_debug("PMU : %s : after OCV table %d 0x%x\n",__func__, i * 10, (debug_disp[i*2] << 8 | debug_disp[i*2+1]));
				}
				/* end */
			}
		}
	}

	return 0;
}

static int reset_FG_process(struct ricoh61x_battery_info *info)
{
	int err;

	//err = set_Rlow(info);
	//err = ricoh61x_Check_OCV_Offset(info);
	err = ricoh61x_write(info->dev->parent,
					 FG_CTRL_REG, 0x51);
	info->soca->ready_fg = 0;
	info->soca->rsoc_ready_flag = 1;

	return err;
}


static int check_charge_status_2(struct ricoh61x_battery_info *info, int displayed_soc_temp)
{
	if (displayed_soc_temp < 0)
			displayed_soc_temp = 0;

	get_power_supply_status(info);
	info->soca->soc = calc_capacity(info) * 100;

	if (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status) {
		if ((info->first_pwon == 1)
			&& (RICOH61x_SOCA_START == info->soca->status)) {
				g_full_flag = 1;
				info->soca->soc_full = info->soca->soc;
				info->soca->displayed_soc = 100*100;
				info->soca->full_reset_count = 0;
		} else {
			if ( (displayed_soc_temp > 97*100)
				&& (calc_ocv(info) > (get_OCV_voltage(info, 9) + (get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))*7/10)  )){
				g_full_flag = 1;
				info->soca->soc_full = info->soca->soc;
				info->soca->displayed_soc = 100*100;
				info->soca->full_reset_count = 0;
			} else {
				g_full_flag = 0;
				info->soca->displayed_soc = displayed_soc_temp;
			}

		}
	}
	if (info->soca->Ibat_ave >= 0) {
		if (g_full_flag == 1) {
			info->soca->displayed_soc = 100*100;
		} else {
			if (info->soca->displayed_soc/100 < 99) {
				info->soca->displayed_soc = displayed_soc_temp;
			} else {
				info->soca->displayed_soc = 99 * 100;
			}
		}
	}
	if (info->soca->Ibat_ave < 0) {
		if (g_full_flag == 1) {
			if (calc_ocv(info) < (get_OCV_voltage(info, 9) + (get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))*7/10)  ) {
				g_full_flag = 0;
				//info->soca->displayed_soc = 100*100;
				info->soca->displayed_soc = displayed_soc_temp;
			} else {
				info->soca->displayed_soc = 100*100;
			}
		} else {
			g_full_flag = 0;
			info->soca->displayed_soc = displayed_soc_temp;
		}
	}

	return info->soca->displayed_soc;
}

/**
* Calculate Capacity in a period
* - read CC_SUM & FA_CAP from Coulom Counter
* -  and calculate Capacity.
* @cc_cap: capacity in a period, unit 0.01%
* @is_charging: Flag of charging current direction
*               TRUE : charging (plus)
*               FALSE: discharging (minus)
* @cc_rst: reset CC_SUM or not
*               TRUE : reset
*               FALSE: not reset
**/
static int calc_capacity_in_period(struct ricoh61x_battery_info *info,
				 int *cc_cap, bool *is_charging, bool cc_rst)
{
	int err;
	uint8_t 	cc_sum_reg[4];
	uint8_t 	cc_clr[4] = {0, 0, 0, 0};
	uint8_t 	fa_cap_reg[2];
	uint16_t 	fa_cap;
	uint32_t 	cc_sum;
	int		cc_stop_flag;
	uint8_t 	status;
	uint8_t 	charge_state;
	int 		Ocv;
	uint32_t 	cc_cap_temp;
	uint32_t 	cc_cap_min;
	int		cc_cap_res;

	*is_charging = true;	/* currrent state initialize -> charging */

	if (info->entry_factory_mode)
		return 0;

	/* get  power supply status */
	err = ricoh61x_read(info->dev->parent, CHGSTATE_REG, &status);
	if (err < 0)
		goto out;
	charge_state = (status & 0x1F);
	Ocv = calc_ocv(info);
	if (charge_state == CHG_STATE_CHG_COMPLETE) {
		/* Check CHG status is complete or not */
		cc_stop_flag = 0;
//	} else if (calc_capacity(info) == 100) {
//		/* Check HW soc is 100 or not */
//		cc_stop_flag = 0;
	} else if (Ocv < get_OCV_voltage(info, 9)) {
		/* Check VBAT is high level or not */
		cc_stop_flag = 0;
	} else {
		cc_stop_flag = 1;
	}

	if (cc_stop_flag == 1)
	{
		/* Disable Charging/Completion Interrupt */
		err = ricoh61x_set_bits(info->dev->parent,
						RICOH61x_INT_MSK_CHGSTS1, 0x01);
		if (err < 0)
			goto out;

		/* disable charging */
		err = ricoh61x_clr_bits(info->dev->parent, RICOH61x_CHG_CTL1, 0x03);
		if (err < 0)
			goto out;
	}

	/* CC_pause enter */
	err = ricoh61x_write(info->dev->parent, CC_CTRL_REG, 0x01);
	if (err < 0)
		goto out;

	/* Read CC_SUM */
	err = ricoh61x_bulk_reads(info->dev->parent,
					CC_SUMREG3_REG, 4, cc_sum_reg);
	if (err < 0)
		goto out;

	if (cc_rst == true) {
		/* CC_SUM <- 0 */
		err = ricoh61x_bulk_writes(info->dev->parent,
						CC_SUMREG3_REG, 4, cc_clr);
		if (err < 0)
			goto out;
	}

	/* CC_pause exist */
	err = ricoh61x_write(info->dev->parent, CC_CTRL_REG, 0);
	if (err < 0)
		goto out;
	if (cc_stop_flag == 1)
	{

		/* Enable charging */
		err = ricoh61x_set_bits(info->dev->parent, RICOH61x_CHG_CTL1, 0x03);
		if (err < 0)
			goto out;

		udelay(1000);

		/* Clear Charging Interrupt status */
		err = ricoh61x_clr_bits(info->dev->parent,
					RICOH61x_INT_IR_CHGSTS1, 0x01);
		if (err < 0)
			goto out;

		/* ricoh61x_read(info->dev->parent, RICOH61x_INT_IR_CHGSTS1, &val);
//		pr_debug("INT_IR_CHGSTS1 = 0x%x\n",val); */

		/* Enable Charging Interrupt */
		err = ricoh61x_clr_bits(info->dev->parent,
						RICOH61x_INT_MSK_CHGSTS1, 0x01);
		if (err < 0)
			goto out;
	}
	/* Read FA_CAP */
	err = ricoh61x_bulk_reads(info->dev->parent,
				 FA_CAP_H_REG, 2, fa_cap_reg);
	if (err < 0)
		goto out;

	/* fa_cap = *(uint16_t*)fa_cap_reg & 0x7fff; */
	fa_cap = ((fa_cap_reg[0] << 8 | fa_cap_reg[1]) & 0x7fff);

	/* cc_sum = *(uint32_t*)cc_sum_reg; */
	cc_sum = cc_sum_reg[0] << 24 | cc_sum_reg[1] << 16 |
				cc_sum_reg[2] << 8 | cc_sum_reg[3];

	/* calculation  two's complement of CC_SUM */
	if (cc_sum & 0x80000000) {
		cc_sum = (cc_sum^0xffffffff)+0x01;
		*is_charging = false;		/* discharge */
	}
	/* (CC_SUM x 10000)/3600/FA_CAP */

	if(fa_cap == 0)
		goto out;
	else
		*cc_cap = cc_sum*25/9/fa_cap;		/* unit is 0.01% */

	//////////////////////////////////////////////////////////////////
	cc_cap_min = fa_cap*3600/100/100/100;	/* Unit is 0.0001% */

	if(cc_cap_min == 0)
		goto out;
	else
		cc_cap_temp = cc_sum / cc_cap_min;

	cc_cap_res = cc_cap_temp % 100;

	pr_debug("PMU: cc_sum = %d: cc_cap_res= %d: \n", cc_sum, cc_cap_res);


	if(*is_charging) {
		info->soca->cc_cap_offset += cc_cap_res;
		if (info->soca->cc_cap_offset >= 100) {
			*cc_cap += 1;
			info->soca->cc_cap_offset %= 100;
		}
	} else {
		info->soca->cc_cap_offset -= cc_cap_res;
		if (info->soca->cc_cap_offset <= -100) {
			*cc_cap += 1;
			info->soca->cc_cap_offset %= 100;
		}
	}
	pr_debug("PMU: cc_cap_offset= %d: \n", info->soca->cc_cap_offset);

	//////////////////////////////////////////////////////////////////
	return 0;
out:
	dev_err(info->dev, "Error !!-----\n");
	pr_debug("  ----fa_cap= %d,  cc_cap_min= %d ",fa_cap,cc_cap_min);
	return err;
}
/**
* Calculate target using capacity
**/
static int get_target_use_cap(struct ricoh61x_battery_info *info)
{
	int i,j;
	int ocv_table[11];
	int temp;
	int Target_Cutoff_Vol = 0;
	int Ocv_ZeroPer_now;
	int Ibat_now;
	int fa_cap,use_cap;
	int FA_CAP_now;
	int start_per = 0;
	int RE_CAP_now;
	int CC_OnePer_step;
	int Ibat_min;

	int Ocv_now;
	int Ocv_now_table;
	int soc_per;
	int use_cap_now;
	int Rsys_now;

	/* get const value */
	Ibat_min = -1 * info->soca->target_ibat;
	if (info->soca->Ibat_ave > Ibat_min) /* I bat is minus */
	{
		Ibat_now = Ibat_min;
	} else {
		Ibat_now = info->soca->Ibat_ave;
	}
	fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
								0x7fff);
	use_cap = fa_cap - info->soca->re_cap_old;

	/* get OCV table % */
	for (i = 0; i <= 10; i = i+1) {
		temp = (battery_init_para[info->num][i*2]<<8)
			 | (battery_init_para[info->num][i*2+1]);
		/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
		temp = ((temp * 50000 * 10 / 4095) + 5) / 10;
		ocv_table[i] = temp;
		pr_debug("PMU : %s : ocv_table %d is %d v\n",__func__, i, ocv_table[i]);
	}

	/* Find out Current OCV */
	i = info->soca->soc/1000;
	j = info->soca->soc - info->soca->soc/1000*1000;
	Ocv_now_table = ocv_table[i]*100+(ocv_table[i+1]-ocv_table[i])*j/10;

	//if (info->soca->Ibat_ave < -1000)
		Rsys_now = (info->soca->Vsys_ave - Ocv_now_table) / info->soca->Ibat_ave;
	//else
	//	Rsys_now = info->soca->Rsys;
	//	Rsys_now = max(info->soca->Rsys/2, Rsys_now);


	Ocv_ZeroPer_now = info->soca->target_vsys * 1000 - Ibat_now * Rsys_now;

	pr_debug("PMU: -------  Ocv_now_table= %d: Rsys_now= %d Ibat_now= %d =======\n",
	       Ocv_now_table, Rsys_now, Ibat_now);

	pr_debug("PMU: -------  Rsys= %d: cutoff_ocv= %d: Ocv_ZeroPer_now= %d =======\n",
	       info->soca->Rsys, info->soca->cutoff_ocv, Ocv_ZeroPer_now);

	/* get FA_CAP_now */


	for (i = 1; i < 11; i++) {
		pr_debug("PMU : %s : ocv_table %d is %d v Ocv_ZerPernow is %d\n",__func__, i, ocv_table[i],(Ocv_ZeroPer_now / 100));
		if (ocv_table[i] >= Ocv_ZeroPer_now / 100) {
			/* unit is 0.001% */
			start_per = Calc_Linear_Interpolation(
				(i-1)*1000, ocv_table[i-1], i*1000,
				 ocv_table[i], (Ocv_ZeroPer_now / 100));
			i = 11;
		}
	}

	start_per = max(0, start_per);

	FA_CAP_now = fa_cap * ((10000 - start_per) / 100 ) / 100;

	pr_debug("PMU: -------  Target_Cutoff_Vol= %d: Ocv_ZeroPer_now= %d: start_per= %d =======\n",
	       Target_Cutoff_Vol, Ocv_ZeroPer_now, start_per);

	/* get RE_CAP_now */
	RE_CAP_now = FA_CAP_now - use_cap;

	if (RE_CAP_now < RE_CAP_GO_DOWN) {
		info->soca->hurry_up_flg = 1;
	} else if (info->soca->Vsys_ave < info->soca->target_vsys*1000) {
		info->soca->hurry_up_flg = 1;
	} else if (info->fg_poff_vbat != 0) {
		if (info->soca->Vbat_ave < info->fg_poff_vbat*1000) {
			info->soca->hurry_up_flg = 1;
		} else {
			info->soca->hurry_up_flg = 0;
		}
	} else {
		info->soca->hurry_up_flg = 0;
	}

	/* get CC_OnePer_step */
	if (info->soca->displayed_soc > 0) { /* avoid divide-by-0 */
		CC_OnePer_step = RE_CAP_now / (info->soca->displayed_soc / 100 + 1);
	} else {
		CC_OnePer_step = 0;
	}
	/* get info->soca->target_use_cap */
	info->soca->target_use_cap = use_cap + CC_OnePer_step;

	pr_debug("PMU: ------- FA_CAP_now= %d: RE_CAP_now= %d: CC_OnePer_step= %d: target_use_cap= %d: hurry_up_flg= %d -------\n",
	       FA_CAP_now, RE_CAP_now, CC_OnePer_step, info->soca->target_use_cap, info->soca->hurry_up_flg);

	return 0;
}
#ifdef ENABLE_OCV_TABLE_CALIB
/**
* Calibration OCV Table
* - Update the value of VBAT on 100% in OCV table
*    if battery is Full charged.
* - int vbat_ocv <- unit is uV
**/
static int calib_ocvTable(struct ricoh61x_battery_info *info, int vbat_ocv)
{
	int ret;
	int cutoff_ocv;
	int i;
	int ocv100_new;
	int start_per = 0;
	int end_per = 0;

	if (info->soca->Ibat_ave > RICOH61x_REL1_SEL_VALUE) {
		pr_debug("PMU: %s IBAT > 64mA -- Not Calibration --\n", __func__);
		return 0;
	}

	if (vbat_ocv < info->soca->OCV100_max) {
		if (vbat_ocv < info->soca->OCV100_min)
			ocv100_new = info->soca->OCV100_min;
		else
			ocv100_new = vbat_ocv;
	} else {
		ocv100_new = info->soca->OCV100_max;
	}
	pr_debug("PMU : %s :max %d min %d current %d\n",__func__,info->soca->OCV100_max,info->soca->OCV100_min,vbat_ocv);
	pr_debug("PMU : %s : New OCV 100 = 0x%x\n",__func__,ocv100_new);

	/* FG_En Off */
	ret = ricoh61x_clr_bits(info->dev->parent, FG_CTRL_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev,"Error in FG_En OFF\n");
		goto err;
	}


	//cutoff_ocv = (battery_init_para[info->num][0]<<8) | (battery_init_para[info->num][1]);
	cutoff_ocv = get_OCV_voltage(info, 0);

	info->soca->ocv_table_def[10] = info->soca->OCV100_max;

	ricoh61x_scaling_OCV_table(info, cutoff_ocv/1000, ocv100_new/1000, &start_per, &end_per);

	ret = ricoh61x_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 22, battery_init_para[info->num]);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	for (i = 0; i <= 10; i = i+1) {
		info->soca->ocv_table[i] = get_OCV_voltage(info, i);
		pr_debug("PMU: %s : * %d0%% voltage = %d uV\n",
				 __func__, i, info->soca->ocv_table[i]);
	}

	/* FG_En on & Reset*/
	ret = reset_FG_process(info);
	if (ret < 0) {
		dev_err(info->dev, "Error in FG_En On & Reset %d\n", ret);
		goto err;
	}

	pr_debug("PMU: %s Exit \n", __func__);
	return 0;
err:
	return ret;

}

#endif

static void ricoh61x_displayed_work(struct work_struct *work)
{
	int err;
	uint8_t val;
	uint8_t val2;
	int soc_round;
	int last_soc_round;
	int last_disp_round;
	int displayed_soc_temp;
	int disp_dec;
	int cc_cap = 0;
	bool is_charging = true;
	int re_cap,fa_cap,use_cap;
	bool is_jeita_updated;
	uint8_t reg_val;
	int delay_flag = 0;
	int Vbat = 0;
	int Ibat = 0;
	int Vsys = 0;
	int temp_ocv;
	int fc_delta = 0;
	int temp_soc;
	int current_cc_sum;
	int calculated_ocv;
	int full_rate;
	int soc_diff;
	int cc_left;

	struct ricoh61x_battery_info *info = container_of(work,
	struct ricoh61x_battery_info, displayed_work.work);

	if (info->entry_factory_mode) {
		info->soca->status = RICOH61x_SOCA_STABLE;
		info->soca->displayed_soc = -EINVAL;
		info->soca->ready_fg = 0;
		return;
	}

	mutex_lock(&info->lock);

	is_jeita_updated = false;

	if ((RICOH61x_SOCA_START == info->soca->status)
		 || (RICOH61x_SOCA_STABLE == info->soca->status)
		 || (RICOH61x_SOCA_FULL == info->soca->status))
		{
			info->soca->ready_fg = 1;
		}
	//if (RICOH61x_SOCA_FG_RESET != info->soca->status)
	//	Set_back_ocv_table(info);

	/* judge Full state or Moni Vsys state */
	calculated_ocv = calc_ocv(info);
	if ((RICOH61x_SOCA_DISP == info->soca->status)
		 || (RICOH61x_SOCA_STABLE == info->soca->status)) {
		/* caluc 95% ocv */
		temp_ocv = get_OCV_voltage(info, 10) -
					(get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))/2;

		if(g_full_flag == 1){	/* for issue 1 solution start*/
			info->soca->status = RICOH61x_SOCA_FULL;
			info->soca->last_cc_sum = 0;
		} else if ((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
			&& (calculated_ocv > temp_ocv)) {
			info->soca->status = RICOH61x_SOCA_FULL;
			g_full_flag = 0;
			info->soca->last_cc_sum = 0;
		} else if (info->soca->Ibat_ave >= -20) {
			/* for issue1 solution end */
			/* check Full state or not*/
			if ((calculated_ocv > get_OCV_voltage(info, RICOH61x_ENTER_FULL_STATE_OCV))
 				|| (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
				|| (info->soca->displayed_soc > RICOH61x_ENTER_FULL_STATE_DSOC * 100)) {
				info->soca->status = RICOH61x_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_cc_sum = 0;
			} else if ((calculated_ocv > get_OCV_voltage(info, 9))
				&& (info->soca->Ibat_ave < 300)) {
				info->soca->status = RICOH61x_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_cc_sum = 0;
			}
		} else { /* dis-charging */
			if (info->soca->displayed_soc/100 < RICOH61x_ENTER_LOW_VOL) {
				info->soca->target_use_cap = 0;
				info->soca->status = RICOH61x_SOCA_LOW_VOL;
			}
		}
	}

	if (RICOH61x_SOCA_STABLE == info->soca->status) {
		info->soca->soc = calc_capacity_2(info);
		info->soca->soc_delta = info->soca->soc - info->soca->last_soc;

		if (info->soca->soc_delta >= -100 && info->soca->soc_delta <= 100) {
			info->soca->displayed_soc = info->soca->soc;
		} else {
			info->soca->status = RICOH61x_SOCA_DISP;
		}
		info->soca->last_soc = info->soca->soc;
		info->soca->soc_delta = 0;
	} else if (RICOH61x_SOCA_FULL == info->soca->status) {
		err = check_jeita_status(info, &is_jeita_updated);
		if (err < 0) {
			dev_err(info->dev, "Error in updating JEITA %d\n", err);
			goto end_flow;
		}
		info->soca->soc = calc_capacity(info) * 100;
		info->soca->last_soc = calc_capacity_2(info);	/* for DISP */

		if (info->soca->Ibat_ave >= -20) { /* charging */
			if (0 == info->soca->jt_limit) {
				if (g_full_flag == 1) {

					if (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status) {
						if(info->soca->full_reset_count < RICOH61x_UPDATE_COUNT_FULL_RESET) {
							info->soca->full_reset_count++;
						} else if (info->soca->full_reset_count < (RICOH61x_UPDATE_COUNT_FULL_RESET + 1)) {
							err = reset_FG_process(info);
							if (err < 0)
								dev_err(info->dev, "Error in writing the control register\n");
							info->soca->full_reset_count++;
							info->soca->rsoc_ready_flag =1;
							goto end_flow;
						} else if(info->soca->full_reset_count < (RICOH61x_UPDATE_COUNT_FULL_RESET + 2)) {
							info->soca->full_reset_count++;
							info->soca->fc_cap = 0;
							info->soca->soc_full = info->soca->soc;
						}
					} else {
						if(info->soca->fc_cap < -1 * 200) {
							g_full_flag = 0;
							info->soca->displayed_soc = 99 * 100;
						}
						info->soca->full_reset_count = 0;
					}


					info->soca->chg_cmp_times = 0;
					if(info->soca->rsoc_ready_flag ==1) {
						err = calc_capacity_in_period(info, &cc_cap, &is_charging, true);
						if (err < 0)
							dev_err(info->dev, "Read cc_sum Error !!-----\n");

						fc_delta = (is_charging == true) ? cc_cap : -cc_cap;

						info->soca->fc_cap = info->soca->fc_cap + fc_delta;
					}

					if (g_full_flag == 1){
						info->soca->displayed_soc = 100*100;
					}
				} else {
					if ((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
						&& (info->soca->displayed_soc >= 9890)){
						if(info->soca->chg_cmp_times > RICOH61x_FULL_WAIT_TIME) {
							info->soca->displayed_soc = 100*100;
							g_full_flag = 1;
							info->soca->full_reset_count = 0;
							info->soca->soc_full = info->soca->soc;
							info->soca->fc_cap = 0;
							info->soca->last_cc_sum = 0;
#ifdef ENABLE_OCV_TABLE_CALIB
							err = calib_ocvTable(info,calculated_ocv);
							if (err < 0)
								dev_err(info->dev, "Calibration OCV Error !!\n");
#endif
						} else {
							info->soca->chg_cmp_times++;
						}
					} else {
						fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
							0x7fff);

						if (info->soca->displayed_soc >= 9950) {
							if((info->soca->soc_full - info->soca->soc) < 200) {
								goto end_flow;
							}
						}
						info->soca->chg_cmp_times = 0;


						if(info->soca->rsoc_ready_flag ==1) {
							err = calc_capacity_in_period(info, &cc_cap, &is_charging, true);
							if (err < 0)
								dev_err(info->dev, "Read cc_sum Error !!-----\n");
							info->soca->cc_delta
								 = (is_charging == true) ? cc_cap : -cc_cap;
						} else {
							err = calc_capacity_in_period(info, &cc_cap, &is_charging, false);
							if (err < 0)
								dev_err(info->dev, "Read cc_sum Error !!-----\n");
							if (info->soca->last_cc_sum == 0) { /* initial setting of last cc sum */
								info->soca->last_cc_sum = (is_charging == true) ? cc_cap : -cc_cap;
								info->soca->cc_delta = 0;
							} else {
								current_cc_sum = (is_charging == true) ? cc_cap : -cc_cap;
								info->soca->cc_delta = current_cc_sum - info->soca->last_cc_sum;
								info->soca->cc_delta = min(13, info->soca->cc_delta);
								info->soca->last_cc_sum = current_cc_sum;
							}
						}

						if((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
						|| (info->soca->Ibat_ave < info->ch_icchg*50 + 100) )
						{
							info->soca->displayed_soc += 13 * 3000 / fa_cap;
						}
						else {
							if (calculated_ocv < get_OCV_voltage(info, 10))
							{

								full_rate = 100 * (10000 - info->soca->displayed_soc) /
									((1000* ((get_OCV_voltage(info, 10)) - calculated_ocv)
									/(get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))));
							}
							else full_rate = 251;

							pr_debug("PMU:full_rate= %d: Delta= %d:\n", full_rate, info->soca->cc_delta);

							full_rate = min(250, max(40,full_rate));

							info->soca->displayed_soc
						       = info->soca->displayed_soc + info->soca->cc_delta* full_rate / 100;

						}

						info->soca->displayed_soc
							 = min(10000, info->soca->displayed_soc);
						info->soca->displayed_soc = max(0, info->soca->displayed_soc);

						if (info->soca->displayed_soc >= 9890) {
							info->soca->displayed_soc = 99 * 100;
						}
					}
				}
			} else {
				info->soca->full_reset_count = 0;
			}
		} else { /* discharging */
			if (info->soca->displayed_soc >= 9950) {
				if (info->soca->Ibat_ave <= -1 * RICOH61x_REL1_SEL_VALUE) {
					if ((calculated_ocv < (get_OCV_voltage(info, 9) + (get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))*3/10))
						|| ((info->soca->soc_full - info->soca->soc) > 200)) {

						g_full_flag = 0;
						info->soca->full_reset_count = 0;
						info->soca->displayed_soc = 100 * 100;
						info->soca->status = RICOH61x_SOCA_DISP;
						info->soca->last_soc = info->soca->soc;
						info->soca->soc_delta = 0;
					} else {
						info->soca->displayed_soc = 100 * 100;
					}
				} else { /* into relaxation state */
					ricoh61x_read(info->dev->parent, CHGSTATE_REG, &reg_val);
					if (reg_val & 0xc0) {
						info->soca->displayed_soc = 100 * 100;
					} else {
						g_full_flag = 0;
						info->soca->full_reset_count = 0;
						info->soca->displayed_soc = 100 * 100;
						info->soca->status = RICOH61x_SOCA_DISP;
						info->soca->last_soc = info->soca->soc;
						info->soca->soc_delta = 0;
					}
				}
			} else {
				g_full_flag = 0;
				info->soca->status = RICOH61x_SOCA_DISP;
				info->soca->soc_delta = 0;
				info->soca->full_reset_count = 0;
				info->soca->last_soc = info->soca->soc;
			}
		}
	} else if (RICOH61x_SOCA_LOW_VOL == info->soca->status) {
		if(info->soca->Ibat_ave >= 0) {
			info->soca->soc = calc_capacity(info) * 100;
			info->soca->status = RICOH61x_SOCA_DISP;
			info->soca->last_soc = info->soca->soc;
			info->soca->soc_delta = 0;
		} else {

			fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
								0x7fff);

			if (info->soca->rsoc_ready_flag == 0) {
				temp_soc = calc_capacity_2(info);
				re_cap = fa_cap * temp_soc / (100 * 100);
				pr_debug("PMU LOW VOL: %s : facap is %d re_cap is %d soc is %d\n",__func__,fa_cap,re_cap,temp_soc);
			} else {
				re_cap = get_check_fuel_gauge_reg(info, RE_CAP_H_REG, RE_CAP_L_REG,
								0x7fff);
			}

			use_cap = fa_cap - re_cap;

			if (info->soca->target_use_cap == 0) {
				info->soca->re_cap_old = re_cap;
				get_target_use_cap(info);
			}

			if(use_cap >= info->soca->target_use_cap) {
				info->soca->displayed_soc = info->soca->displayed_soc - 100;
				info->soca->displayed_soc = max(0, info->soca->displayed_soc);
				info->soca->re_cap_old = re_cap;
			} else if (info->soca->hurry_up_flg == 1) {
				info->soca->displayed_soc = info->soca->displayed_soc - 100;
				info->soca->displayed_soc = max(0, info->soca->displayed_soc);
				info->soca->re_cap_old = re_cap;
			}
			get_target_use_cap(info);
			info->soca->soc = calc_capacity(info) * 100;
		}
	}

	if (RICOH61x_SOCA_DISP == info->soca->status) {

		info->soca->soc = calc_capacity_2(info);

		soc_round = (info->soca->soc + 50) / 100;
		last_soc_round = (info->soca->last_soc + 50) / 100;
		last_disp_round = (info->soca->displayed_soc + 50) / 100;

		info->soca->soc_delta =
			info->soca->soc_delta + (info->soca->soc - info->soca->last_soc);

		info->soca->last_soc = info->soca->soc;
		/* six case */
		if (last_disp_round == soc_round) {
			/* if SOC == DISPLAY move to stable */
			info->soca->displayed_soc = info->soca->soc ;
			info->soca->status = RICOH61x_SOCA_STABLE;
			delay_flag = 1;
		} else if (info->soca->Ibat_ave > 0) {
			if ((0 == info->soca->jt_limit) ||
			(POWER_SUPPLY_STATUS_FULL != info->soca->chg_status)) {
				/* Charge */
				if (last_disp_round < soc_round) {
					/* Case 1 : Charge, Display < SOC */
					if (info->soca->soc_delta >= 100) {
						info->soca->displayed_soc
							= last_disp_round * 100 + 50;
	 					info->soca->soc_delta -= 100;
						if (info->soca->soc_delta >= 100)
		 					delay_flag = 1;
					} else {
						info->soca->displayed_soc += 25;
						disp_dec = info->soca->displayed_soc % 100;
						if ((50 <= disp_dec) && (disp_dec <= 74))
							info->soca->soc_delta = 0;
					}
					if ((info->soca->displayed_soc + 50)/100
								 >= soc_round) {
						info->soca->displayed_soc
							= info->soca->soc ;
						info->soca->status
							= RICOH61x_SOCA_STABLE;
						delay_flag = 1;
					}
				} else if (last_disp_round > soc_round) {
					/* Case 2 : Charge, Display > SOC */
					if (info->soca->soc_delta >= 300) {
						info->soca->displayed_soc += 100;
						info->soca->soc_delta -= 300;
					}
					if ((info->soca->displayed_soc + 50)/100
								 <= soc_round) {
						info->soca->displayed_soc
							= info->soca->soc ;
						info->soca->status
						= RICOH61x_SOCA_STABLE;
						delay_flag = 1;
					}
				}
			} else {
				info->soca->soc_delta = 0;
			}
		} else {
			/* Dis-Charge */
			if (last_disp_round > soc_round) {
				/* Case 3 : Dis-Charge, Display > SOC */
				if (info->soca->soc_delta <= -100) {
					info->soca->displayed_soc
						= last_disp_round * 100 - 75;
					info->soca->soc_delta += 100;
					if (info->soca->soc_delta <= -100)
						delay_flag = 1;
				} else {
					info->soca->displayed_soc -= 25;
					disp_dec = info->soca->displayed_soc % 100;
					if ((25 <= disp_dec) && (disp_dec <= 49))
						info->soca->soc_delta = 0;
				}
				if ((info->soca->displayed_soc + 50)/100
							 <= soc_round) {
					info->soca->displayed_soc
						= info->soca->soc ;
					info->soca->status
						= RICOH61x_SOCA_STABLE;
					delay_flag = 1;
				}
			} else if (last_disp_round < soc_round) {
				/* Case 4 : Dis-Charge, Display < SOC */
				if (info->soca->soc_delta <= -300) {
					info->soca->displayed_soc -= 100;
					info->soca->soc_delta += 300;
				}
				if ((info->soca->displayed_soc + 50)/100
							 >= soc_round) {
					info->soca->displayed_soc
						= info->soca->soc ;
					info->soca->status
						= RICOH61x_SOCA_STABLE;
					delay_flag = 1;
				}
			}
		}
	} else if (RICOH61x_SOCA_UNSTABLE == info->soca->status) {
		/* caluc 95% ocv */
		temp_ocv = get_OCV_voltage(info, 10) -
					(get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))/2;

		if(g_full_flag == 1){	/* for issue 1 solution start*/
			info->soca->status = RICOH61x_SOCA_FULL;
			info->soca->last_cc_sum = 0;
			err = reset_FG_process(info);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");

			goto end_flow;
		}else if ((POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
			&& (calculated_ocv > temp_ocv)) {
			info->soca->status = RICOH61x_SOCA_FULL;
			g_full_flag = 0;
			info->soca->last_cc_sum = 0;
			err = reset_FG_process(info);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");
			goto end_flow;
		} else if (info->soca->Ibat_ave >= -20) {
			/* for issue1 solution end */
			/* check Full state or not*/
			if ((calculated_ocv > (get_OCV_voltage(info, 9) + (get_OCV_voltage(info, 10) - get_OCV_voltage(info, 9))*7/10))
				|| (POWER_SUPPLY_STATUS_FULL == info->soca->chg_status)
				|| (info->soca->displayed_soc > 9850))
			{
				info->soca->status = RICOH61x_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_cc_sum = 0;
				err = reset_FG_process(info);
				if (err < 0)
					dev_err(info->dev, "Error in writing the control register\n");
				goto end_flow;
			} else if ((calculated_ocv > (get_OCV_voltage(info, 9)))
				&& (info->soca->Ibat_ave < 300))
			{
				info->soca->status = RICOH61x_SOCA_FULL;
				g_full_flag = 0;
				info->soca->last_cc_sum = 0;
				err = reset_FG_process(info);
				if (err < 0)
					dev_err(info->dev, "Error in writing the control register\n");
				goto end_flow;
			}
		}

		err = ricoh61x_read(info->dev->parent, PSWR_REG, &val);
		val &= 0x7f;
		info->soca->soc = val * 100;
		if (err < 0) {
			dev_err(info->dev,
				 "Error in reading PSWR_REG %d\n", err);
			info->soca->soc
				 = calc_capacity(info) * 100;
		}

		err = calc_capacity_in_period(info, &cc_cap,
						 &is_charging, false);
		if (err < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

		info->soca->cc_delta
			 = (is_charging == true) ? cc_cap : -cc_cap;

		displayed_soc_temp
		       = info->soca->soc + info->soca->cc_delta;
		if (displayed_soc_temp < 0)
			displayed_soc_temp = 0;
		displayed_soc_temp
			 = min(9850, displayed_soc_temp);
		displayed_soc_temp = max(0, displayed_soc_temp);

		info->soca->displayed_soc = displayed_soc_temp;

	} else if (RICOH61x_SOCA_FG_RESET == info->soca->status) {
		/* No update */
	} else if (RICOH61x_SOCA_START == info->soca->status) {

		err = measure_Ibatt_FG(info, &Ibat);
		err = measure_vbatt_FG(info, &Vbat);
		err = measure_vsys_ADC(info, &Vsys);

		info->soca->Ibat_ave = Ibat;
		info->soca->Vbat_ave = Vbat;
		info->soca->Vsys_ave = Vsys;

		err = check_jeita_status(info, &is_jeita_updated);
		is_jeita_updated = false;
		if (err < 0) {
			dev_err(info->dev, "Error in updating JEITA %d\n", err);
		}
		err = ricoh61x_read(info->dev->parent, PSWR_REG, &val);
		val &= 0x7f;
		if (info->first_pwon) {
			info->soca->soc = calc_capacity(info) * 100;
			val = (info->soca->soc + 50)/100;
			val &= 0x7f;
			err = ricoh61x_write(info->dev->parent, PSWR_REG, val);
			if (err < 0)
				dev_err(info->dev, "Error in writing PSWR_REG\n");
			g_soc = val;

			if ((info->soca->soc == 0) && (calculated_ocv
					< get_OCV_voltage(info, 0))) {
				info->soca->displayed_soc = 0;
				info->soca->status = RICOH61x_SOCA_ZERO;
			} else {
				if (0 == info->soca->jt_limit) {
					check_charge_status_2(info, info->soca->soc);
				} else {
					info->soca->displayed_soc = info->soca->soc;
				}
				if (Ibat < 0) {
					if (info->soca->displayed_soc < 300) {
						info->soca->target_use_cap = 0;
						info->soca->status = RICOH61x_SOCA_LOW_VOL;
					} else {
						if ((info->fg_poff_vbat != 0)
						      && (Vbat < info->fg_poff_vbat * 1000) ){
							  info->soca->target_use_cap = 0;
							  info->soca->status = RICOH61x_SOCA_LOW_VOL;
						  } else {
							  info->soca->status = RICOH61x_SOCA_UNSTABLE;
						  }
					}
				} else {
					info->soca->status = RICOH61x_SOCA_UNSTABLE;
				}
			}
		} else if (g_fg_on_mode && (val == 0x7f)) {
			info->soca->soc = calc_capacity(info) * 100;
			if ((info->soca->soc == 0) && (calculated_ocv
					< get_OCV_voltage(info, 0))) {
				info->soca->displayed_soc = 0;
				info->soca->status = RICOH61x_SOCA_ZERO;
			} else {
				if (0 == info->soca->jt_limit) {
					check_charge_status_2(info, info->soca->soc);
				} else {
					info->soca->displayed_soc = info->soca->soc;
				}
				info->soca->last_soc = info->soca->soc;
				info->soca->status = RICOH61x_SOCA_STABLE;
			}
		} else {
			info->soca->soc = val * 100;
			if (err < 0) {
				dev_err(info->dev,
					 "Error in reading PSWR_REG %d\n", err);
				info->soca->soc
					 = calc_capacity(info) * 100;
			}

			err = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, false);
			if (err < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");

			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;
			if (calculated_ocv < get_OCV_voltage(info, 0)) {
				info->soca->displayed_soc = 0;
				info->soca->status = RICOH61x_SOCA_ZERO;
			} else {
				displayed_soc_temp
				       = info->soca->soc + info->soca->cc_delta;
				if (displayed_soc_temp < 0)
					displayed_soc_temp = 0;
				displayed_soc_temp
					 = min(10000, displayed_soc_temp);
				displayed_soc_temp = max(0, displayed_soc_temp);
				if (0 == info->soca->jt_limit) {
					check_charge_status_2(info, displayed_soc_temp);
				} else {
					info->soca->displayed_soc = displayed_soc_temp;
				}
				info->soca->last_soc = calc_capacity(info) * 100;

				if(info->soca->rsoc_ready_flag == 0) {
					info->soca->status = RICOH61x_SOCA_STABLE;
					pr_debug("PMU STABLE : %s : initial  dsoc  is  %d\n",__func__,info->soca->displayed_soc);
				} else if  (Ibat < 0) {
					if (info->soca->displayed_soc < 300) {
						info->soca->target_use_cap = 0;
						info->soca->status = RICOH61x_SOCA_LOW_VOL;
					} else {
						if ((info->fg_poff_vbat != 0)
						      && (Vbat < info->fg_poff_vbat * 1000)){
							  info->soca->target_use_cap = 0;
							  info->soca->status = RICOH61x_SOCA_LOW_VOL;
						  } else {
							  info->soca->status = RICOH61x_SOCA_UNSTABLE;
						  }
					}
				} else {
					info->soca->status = RICOH61x_SOCA_UNSTABLE;
				}
			}
		}
	} else if (RICOH61x_SOCA_ZERO == info->soca->status) {
		if (calculated_ocv > get_OCV_voltage(info, 0)) {
			err = calc_capacity_in_period(info, &cc_cap,
								 &is_charging,true);
			if (err < 0)
				{dev_err(info->dev, "Read cc_sum Error !!-----\n");}
			info->soca->rsoc_ready_flag =0;
			info->soca->init_pswr = 1;
			val = 1;
			val &= 0x7f;
			g_soc = 1;
			err = ricoh61x_write(info->dev->parent, PSWR_REG, val);
			if (err < 0)
				{dev_err(info->dev, "Error in writing PSWR_REG\n");}
			info->soca->last_soc = 100;
			info->soca->status = RICOH61x_SOCA_STABLE;
		}
		info->soca->displayed_soc = 0;
	}
end_flow:
	/* keep DSOC = 1 when Vbat is over 3.4V*/
	if( info->fg_poff_vbat != 0) {
		if (info->soca->zero_flg == 1) {
			if(info->soca->Ibat_ave >= 0) {
				info->soca->zero_flg = 0;
			}
			info->soca->displayed_soc = 0;
		} else if (info->soca->displayed_soc < 50) {
			if (info->soca->Vbat_ave < 2000*1000) { /* error value */
				info->soca->displayed_soc = 100;
			} else if (info->soca->Vbat_ave < info->fg_poff_vbat*1000) {
				info->soca->displayed_soc = 0;
				info->soca->zero_flg = 1;
			} else {
				info->soca->displayed_soc = 100;
			}
		}
	}

	if (g_fg_on_mode
		 && (info->soca->status == RICOH61x_SOCA_STABLE)) {
		err = ricoh61x_write(info->dev->parent, PSWR_REG, 0x7f);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");
		g_soc = 0x7F;
		err = calc_capacity_in_period(info, &cc_cap,
							&is_charging, true);
		if (err < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

	} else if ((RICOH61x_SOCA_UNSTABLE != info->soca->status)
			&& (info->soca->rsoc_ready_flag != 0)){
		if ((info->soca->displayed_soc + 50)/100 <= 1) {
			val = 1;
		} else {
			val = (info->soca->displayed_soc + 50)/100;
			val &= 0x7f;
		}
		err = ricoh61x_write(info->dev->parent, PSWR_REG, val);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");

		g_soc = val;

		err = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, true);
		if (err < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");
	}
	else if(g_fg_on_mode==0 && RICOH61x_SOCA_UNSTABLE != info->soca->status
			&& RICOH61x_SOCA_LOW_VOL != info->soca->status
			&& RICOH61x_SOCA_FULL != info->soca->status)  // Ross Function
	{
			err = set_pswr(info);
			if(err<0)
				pr_debug("READ ERR\n");
			err = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, false);
			if(err<0)
				pr_debug("Wrong when calc_capacity\n");

			if(cc_cap > 100)
				{
					cc_left = cc_cap%100;
					soc_diff = cc_cap/100;
					cc_left = (is_charging == true)?cc_left:-cc_left;
					soc_diff = (is_charging == true)?soc_diff:-soc_diff;
					pr_debug("left is %d,diff is %d,inipswr is %d  \n",cc_left,soc_diff,info->soca->init_pswr);
					info->soca->init_pswr += soc_diff;
					pr_debug("after init_pswr is %d \n",info->soca->init_pswr);
					info->soca->init_pswr = min(100,max(0,info->soca->init_pswr));
					err = set_cc_sum_back(info,&cc_left);
					if(err<0)
							pr_debug("set error!\n");
				}
	}

	pr_debug("PMU:STATUS= %d: IBAT= %d: VSYS= %d: VBAT= %d: DSOC= %d: RSOC= %d:\n",
	       info->soca->status, info->soca->Ibat_ave*20/info->fg_rsense_val, info->soca->Vsys_ave, info->soca->Vbat_ave,
		info->soca->displayed_soc, info->soca->soc);
	/*
	int i;
	for (i=0xb0;i<0xf0;i++)
	{
		err = ricoh61x_read(info->dev->parent, i, &val);
		pr_debug("    ----- 0x%x = %x \n",i,val);
	}
	*/
//      err = ricoh61x_read(info->dev->parent, 0xbd, &val);
//      pr_debug("    ----- Rbat = %d, fg_rsense_val = %d, full_cap = %d, 0xbd = %x \n",
//			info->soca->Rbat, info->fg_rsense_val, info->soca->n_cap, val);
//	pr_debug("PMU AGE*STATUS * %d*IBAT*%d*VSYS*%d*VBAT*%d*DSOC*%d*RSOC*%d*-------\n",
//	      info->soca->status, info->soca->Ibat_ave, info->soca->Vsys_ave, info->soca->Vbat_ave,
//		info->soca->displayed_soc, info->soca->soc);

#ifdef DISABLE_CHARGER_TIMER
	/* clear charger timer */
	if ( info->soca->chg_status == POWER_SUPPLY_STATUS_CHARGING ) {
		err = ricoh61x_read(info->dev->parent, TIMSET_REG, &val);
		if (err < 0)
			dev_err(info->dev,
			"Error in read TIMSET_REG%d\n", err);
		/* to check bit 0-1 */
		val2 = val & 0x03;

		if (val2 == 0x02){
			/* set rapid timer 240 -> 300 */
			err = ricoh61x_set_bits(info->dev->parent, TIMSET_REG, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
			}
		} else {
			/* set rapid timer 300 -> 240 */
			err = ricoh61x_clr_bits(info->dev->parent, TIMSET_REG, 0x01);
			err = ricoh61x_set_bits(info->dev->parent, TIMSET_REG, 0x02);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
			}
		}
	}
#endif

	if (0 == info->soca->ready_fg)
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 RICOH61x_FG_RESET_TIME * HZ);
	else if (delay_flag == 1)
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 RICOH61x_DELAY_TIME * HZ);
	else if (RICOH61x_SOCA_DISP == info->soca->status)
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 RICOH61x_DISPLAY_UPDATE_TIME * HZ);
	else if (info->soca->hurry_up_flg == 1)
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 RICOH61x_LOW_VOL_DOWN_TIME * HZ);
	else
		queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
					 RICOH61x_DISPLAY_UPDATE_TIME * HZ);

	mutex_unlock(&info->lock);

	if((true == is_jeita_updated)
	|| (info->soca->last_displayed_soc/100 != (info->soca->displayed_soc+50)/100))
		power_supply_changed(&info->battery);

	info->soca->last_displayed_soc = info->soca->displayed_soc+50;

	return;
}

static void ricoh61x_stable_charge_countdown_work(struct work_struct *work)
{
	int ret;
	int max = 0;
	int min = 100;
	int i;
	struct ricoh61x_battery_info *info = container_of(work,
		struct ricoh61x_battery_info, charge_stable_work.work);

	if (info->entry_factory_mode)
		return;

	mutex_lock(&info->lock);
	if (RICOH61x_SOCA_FG_RESET == info->soca->status)
		info->soca->ready_fg = 1;

	if (2 <= info->soca->stable_count) {
		if (3 == info->soca->stable_count
			&& RICOH61x_SOCA_FG_RESET == info->soca->status) {
			ret = reset_FG_process(info);
			if (ret < 0)
				dev_err(info->dev, "Error in writing the control register\n");
		}
		info->soca->stable_count = info->soca->stable_count - 1;
		queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 RICOH61x_FG_STABLE_TIME * HZ / 10);
	} else if (0 >= info->soca->stable_count) {
		/* Finished queue, ignore */
	} else if (1 == info->soca->stable_count) {
		if (RICOH61x_SOCA_UNSTABLE == info->soca->status) {
			/* Judge if FG need reset or Not */
			info->soca->soc = calc_capacity(info) * 100;
			if (info->chg_ctr != 0) {
				queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 RICOH61x_FG_STABLE_TIME * HZ / 10);
				mutex_unlock(&info->lock);
				return;
			}
			/* Do reset setting */
			ret = reset_FG_process(info);
			if (ret < 0)
				dev_err(info->dev, "Error in writing the control register\n");

			info->soca->status = RICOH61x_SOCA_FG_RESET;

			/* Delay for addition Reset Time (6s) */
			queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 RICOH61x_FG_RESET_TIME*HZ);
		} else if (RICOH61x_SOCA_FG_RESET == info->soca->status) {
			info->soca->reset_soc[2] = info->soca->reset_soc[1];
			info->soca->reset_soc[1] = info->soca->reset_soc[0];
			info->soca->reset_soc[0] = calc_capacity(info) * 100;
			info->soca->reset_count++;

			if (info->soca->reset_count > 10) {
				/* Reset finished; */
				info->soca->soc = info->soca->reset_soc[0];
				info->soca->stable_count = 0;
				goto adjust;
			}

			for (i = 0; i < 3; i++) {
				if (max < info->soca->reset_soc[i]/100)
					max = info->soca->reset_soc[i]/100;
				if (min > info->soca->reset_soc[i]/100)
					min = info->soca->reset_soc[i]/100;
			}

			if ((info->soca->reset_count > 3) && ((max - min)
					< RICOH61x_MAX_RESET_SOC_DIFF)) {
				/* Reset finished; */
				info->soca->soc = info->soca->reset_soc[0];
				info->soca->stable_count = 0;
				goto adjust;
			} else {
				/* Do reset setting */
				ret = reset_FG_process(info);
				if (ret < 0)
					dev_err(info->dev, "Error in writing the control register\n");

				/* Delay for addition Reset Time (6s) */
				queue_delayed_work(info->monitor_wqueue,
						 &info->charge_stable_work,
						 RICOH61x_FG_RESET_TIME*HZ);
			}
		/* Finished queue From now, select FG as result; */
		} else if (RICOH61x_SOCA_START == info->soca->status) {
			/* Normal condition */
		} else { /* other state ZERO/DISP/STABLE */
			info->soca->stable_count = 0;
		}

		mutex_unlock(&info->lock);
		return;

adjust:
		info->soca->last_soc = info->soca->soc;
		info->soca->status = RICOH61x_SOCA_DISP;
		info->soca->soc_delta = 0;

	}
	mutex_unlock(&info->lock);
	return;
}

static void ricoh61x_charge_monitor_work(struct work_struct *work)
{
	struct ricoh61x_battery_info *info = container_of(work,
		struct ricoh61x_battery_info, charge_monitor_work.work);

	get_power_supply_status(info);

	if (POWER_SUPPLY_STATUS_DISCHARGING == info->soca->chg_status
		|| POWER_SUPPLY_STATUS_NOT_CHARGING == info->soca->chg_status) {
		switch (info->soca->dischg_state) {
		case	0:
			info->soca->dischg_state = 1;
			break;
		case	1:
			info->soca->dischg_state = 2;
			break;

		case	2:
		default:
			break;
		}
	} else {
		info->soca->dischg_state = 0;
	}

	queue_delayed_work(info->monitor_wqueue, &info->charge_monitor_work,
					 RICOH61x_CHARGE_MONITOR_TIME * HZ);

	return;
}

static void ricoh61x_get_charge_work(struct work_struct *work)
{
	struct ricoh61x_battery_info *info = container_of(work,
		struct ricoh61x_battery_info, get_charge_work.work);

	int Vbat_temp, Vsys_temp, Ibat_temp;
	int Vbat_sort[RICOH61x_GET_CHARGE_NUM];
	int Vsys_sort[RICOH61x_GET_CHARGE_NUM];
	int Ibat_sort[RICOH61x_GET_CHARGE_NUM];
	int i, j;
	int ret;

	mutex_lock(&info->lock);

	for (i = RICOH61x_GET_CHARGE_NUM-1; i > 0; i--) {
		if (0 == info->soca->chg_count) {
			info->soca->Vbat[i] = 0;
			info->soca->Vsys[i] = 0;
			info->soca->Ibat[i] = 0;
		} else {
			info->soca->Vbat[i] = info->soca->Vbat[i-1];
			info->soca->Vsys[i] = info->soca->Vsys[i-1];
			info->soca->Ibat[i] = info->soca->Ibat[i-1];
		}
	}

	ret = measure_vbatt_FG(info, &info->soca->Vbat[0]);
	ret = measure_vsys_ADC(info, &info->soca->Vsys[0]);
	ret = measure_Ibatt_FG(info, &info->soca->Ibat[0]);

	info->soca->chg_count++;

	if (RICOH61x_GET_CHARGE_NUM != info->soca->chg_count) {
		queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
					 RICOH61x_CHARGE_CALC_TIME * HZ);
		mutex_unlock(&info->lock);
		return ;
	}

	for (i = 0; i < RICOH61x_GET_CHARGE_NUM; i++) {
		Vbat_sort[i] = info->soca->Vbat[i];
		Vsys_sort[i] = info->soca->Vsys[i];
		Ibat_sort[i] = info->soca->Ibat[i];
	}

	Vbat_temp = 0;
	Vsys_temp = 0;
	Ibat_temp = 0;
	for (i = 0; i < RICOH61x_GET_CHARGE_NUM - 1; i++) {
		for (j = RICOH61x_GET_CHARGE_NUM - 1; j > i; j--) {
			if (Vbat_sort[j - 1] > Vbat_sort[j]) {
				Vbat_temp = Vbat_sort[j];
				Vbat_sort[j] = Vbat_sort[j - 1];
				Vbat_sort[j - 1] = Vbat_temp;
			}
			if (Vsys_sort[j - 1] > Vsys_sort[j]) {
				Vsys_temp = Vsys_sort[j];
				Vsys_sort[j] = Vsys_sort[j - 1];
				Vsys_sort[j - 1] = Vsys_temp;
			}
			if (Ibat_sort[j - 1] > Ibat_sort[j]) {
				Ibat_temp = Ibat_sort[j];
				Ibat_sort[j] = Ibat_sort[j - 1];
				Ibat_sort[j - 1] = Ibat_temp;
			}
		}
	}

	Vbat_temp = 0;
	Vsys_temp = 0;
	Ibat_temp = 0;
	for (i = 3; i < RICOH61x_GET_CHARGE_NUM-3; i++) {
		Vbat_temp = Vbat_temp + Vbat_sort[i];
		Vsys_temp = Vsys_temp + Vsys_sort[i];
		Ibat_temp = Ibat_temp + Ibat_sort[i];
	}
	Vbat_temp = Vbat_temp / (RICOH61x_GET_CHARGE_NUM - 6);
	Vsys_temp = Vsys_temp / (RICOH61x_GET_CHARGE_NUM - 6);
	Ibat_temp = Ibat_temp / (RICOH61x_GET_CHARGE_NUM - 6);

	if (0 == info->soca->chg_count) {
		queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
				 RICOH61x_CHARGE_UPDATE_TIME * HZ);
		mutex_unlock(&info->lock);
		return;
	} else {
		info->soca->Vbat_ave = Vbat_temp;
		info->soca->Vsys_ave = Vsys_temp;
		info->soca->Ibat_ave = Ibat_temp;
	}

	info->soca->chg_count = 0;
	queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
				 RICOH61x_CHARGE_UPDATE_TIME * HZ);
	mutex_unlock(&info->lock);
	return;
}

/* Initial setting of FuelGauge SOCA function */
static int ricoh61x_init_fgsoca(struct ricoh61x_battery_info *info)
{
	int i;
	int err;
	uint8_t val;
	int cc_cap = 0;
	bool is_charging = true;

	for (i = 0; i <= 10; i = i+1) {
		info->soca->ocv_table[i] = get_OCV_voltage(info, i);
		pr_debug("PMU: %s : * %d0%% voltage = %d uV\n",
				 __func__, i, info->soca->ocv_table[i]);
	}

	for (i = 0; i < 3; i = i+1)
		info->soca->reset_soc[i] = 0;
	info->soca->reset_count = 0;


	if (info->first_pwon) {

		err = ricoh61x_read(info->dev->parent, CHGISET_REG, &val);
		if (err < 0)
			dev_err(info->dev,
			"Error in read CHGISET_REG%d\n", err);

		err = ricoh61x_write(info->dev->parent, CHGISET_REG, 0);
		if (err < 0)
			dev_err(info->dev,
			"Error in writing CHGISET_REG%d\n", err);
		/* msleep(1000); */

		if (!info->entry_factory_mode) {
			err = ricoh61x_write(info->dev->parent,
							FG_CTRL_REG, 0x51);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");
		}

		info->soca->rsoc_ready_flag = 1;

		err = calc_capacity_in_period(info, &cc_cap, &is_charging, true);

		/* msleep(6000); */

		err = ricoh61x_write(info->dev->parent, CHGISET_REG, val);
		if (err < 0)
			dev_err(info->dev,
			"Error in writing CHGISET_REG%d\n", err);
	}

	/* Rbat : Transfer */

	info->soca->Rbat = get_OCV_init_Data(info, 12) * 1000 / 512
							 * 5000 / 4095;
	info->soca->n_cap = get_OCV_init_Data(info, 11);


	info->soca->displayed_soc = 0;
	info->soca->last_displayed_soc = 0;
	info->soca->suspend_soc = 0;
	info->soca->ready_fg = 0;
	info->soca->soc_delta = 0;
	info->soca->full_reset_count = 0;
	info->soca->soc_full = 0;
	info->soca->fc_cap = 0;
	info->soca->status = RICOH61x_SOCA_START;
	/* stable count down 11->2, 1: reset; 0: Finished; */
	info->soca->stable_count = 11;
	info->soca->chg_cmp_times = 0;
	info->soca->dischg_state = 0;
	info->soca->Vbat_ave = 0;
	info->soca->Vbat_old = 0;
	info->soca->Vsys_ave = 0;
	info->soca->Ibat_ave = 0;
	info->soca->chg_count = 0;
	info->soca->target_use_cap = 0;
	info->soca->hurry_up_flg = 0;
	info->soca->re_cap_old = 0;
	info->soca->jt_limit = 0;
	info->soca->zero_flg = 0;
	info->soca->cc_cap_offset = 0;
	info->soca->last_cc_sum = 0;

	for (i = 0; i < 11; i++) {
		info->soca->ocv_table_low[i] = 0;
	}

	for (i = 0; i < RICOH61x_GET_CHARGE_NUM; i++) {
		info->soca->Vbat[i] = 0;
		info->soca->Vsys[i] = 0;
		info->soca->Ibat[i] = 0;
	}

	g_full_flag = 0;

#ifdef ENABLE_FG_KEEP_ON_MODE
	g_fg_on_mode = 1;
	info->soca->rsoc_ready_flag = 1;
#else
	g_fg_on_mode = 0;
#endif

	/* Start first Display job */
	queue_delayed_work(info->monitor_wqueue, &info->displayed_work,
						   RICOH61x_FG_RESET_TIME*HZ);

	/* Start first Waiting stable job */
	queue_delayed_work(info->monitor_wqueue, &info->charge_stable_work,
		   RICOH61x_FG_STABLE_TIME*HZ/10);

	queue_delayed_work(info->monitor_wqueue, &info->charge_monitor_work,
					 RICOH61x_CHARGE_MONITOR_TIME * HZ);

	queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
					 RICOH61x_CHARGE_MONITOR_TIME * HZ);
	if (info->jt_en) {
		if (info->jt_hw_sw) {
			/* Enable JEITA function supported by H/W */
			err = ricoh61x_set_bits(info->dev->parent, CHGCTL1_REG, 0x04);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");
		} else {
		 	/* Disable JEITA function supported by H/W */
			err = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, 0x04);
			if (err < 0)
				dev_err(info->dev, "Error in writing the control register\n");
			queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
						 	 RICOH61x_FG_RESET_TIME * HZ);
		}
	} else {
		/* Disable JEITA function supported by H/W */
		err = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, 0x04);
		if (err < 0)
			dev_err(info->dev, "Error in writing the control register\n");
	}

	pr_debug("PMU: %s : * Rbat = %d mOhm   n_cap = %d mAH\n",
			 __func__, info->soca->Rbat, info->soca->n_cap * 20 /info->fg_rsense_val);
	return 1;
}
#endif

static void ricoh61x_changed_work(struct work_struct *work)
{
	struct ricoh61x_battery_info *info = container_of(work,
		struct ricoh61x_battery_info, changed_work.work);

	power_supply_changed(&info->battery);

	return;
}

static int check_jeita_status(struct ricoh61x_battery_info *info, bool *is_jeita_updated)
/*  JEITA Parameter settings
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
{
	int temp;
	int err = 0;
	int vfchg;
	uint8_t chgiset_org;
	uint8_t batset2_org;
	uint8_t set_vchg_h, set_vchg_l;
	uint8_t set_ichg_h, set_ichg_l;

	*is_jeita_updated = false;
	/* No execute if JEITA disabled */
	if (!info->jt_en || info->jt_hw_sw)
		return 0;

	/* Check FG Reset */
	if (info->soca->ready_fg) {
		temp = get_battery_temp_2(info) / 10;
	} else {
		pr_debug("JEITA: %s *** cannot update by resetting FG ******\n", __func__);
		goto out;
	}

	/* Read BATSET2 */
	err = ricoh61x_read(info->dev->parent, BATSET2_REG, &batset2_org);
	if (err < 0) {
		dev_err(info->dev, "Error in readng the battery setting register\n");
		goto out;
	}
	vfchg = (batset2_org & 0x70) >> 4;
	batset2_org &= 0x8F;

	/* Read CHGISET */
	err = ricoh61x_read(info->dev->parent, CHGISET_REG, &chgiset_org);
	if (err < 0) {
		dev_err(info->dev, "Error in readng the chrage setting register\n");
		goto out;
	}
	chgiset_org &= 0xC0;

	set_ichg_h = (uint8_t)(chgiset_org | info->jt_ichg_h);
	set_ichg_l = (uint8_t)(chgiset_org | info->jt_ichg_l);

	set_vchg_h = (uint8_t)((info->jt_vfchg_h << 4) | batset2_org);
	set_vchg_l = (uint8_t)((info->jt_vfchg_l << 4) | batset2_org);

	pr_debug("PMU: %s *** Temperature: %d, vfchg: %d, SW status: %d, chg_status: %d ******\n",
		 __func__, temp, vfchg, info->soca->status, info->soca->chg_status);

	if (temp <= 0 || 55 <= temp) {
		/* 1st and 5th temperature ranges (~0, 55~) */
		pr_debug("PMU: %s *** Temp(%d) is out of 0-55 ******\n", __func__, temp);
		err = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
		info->soca->jt_limit = 0;
		*is_jeita_updated = true;
	} else if (temp < info->jt_temp_l) {
		/* 2nd temperature range (0~12) */
		if (vfchg != info->jt_vfchg_h) {
			pr_debug("PMU: %s *** 0<Temp<12, update to vfchg=%d ******\n",
									__func__, info->jt_vfchg_h);
			err = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
				goto out;
			}

			/* set VFCHG/VRCHG */
			err = ricoh61x_write(info->dev->parent,
							 BATSET2_REG, set_vchg_h);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the battery setting register\n");
				goto out;
			}
			info->soca->jt_limit = 0;
			*is_jeita_updated = true;
		} else
			pr_debug("PMU: %s *** 0<Temp<50, already set vfchg=%d, so no need to update ******\n",
					__func__, info->jt_vfchg_h);

		/* set ICHG */
		err = ricoh61x_write(info->dev->parent, CHGISET_REG, set_ichg_l);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the battery setting register\n");
			goto out;
		}
		err = ricoh61x_set_bits(info->dev->parent, CHGCTL1_REG, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
	} else if (temp < info->jt_temp_h) {
		/* 3rd temperature range (12~50) */
		if (vfchg != info->jt_vfchg_h) {
			pr_debug("PMU: %s *** 12<Temp<50, update to vfchg==%d ******\n", __func__, info->jt_vfchg_h);

			err = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
				goto out;
			}
			/* set VFCHG/VRCHG */
			err = ricoh61x_write(info->dev->parent,
							 BATSET2_REG, set_vchg_h);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the battery setting register\n");
				goto out;
			}
			info->soca->jt_limit = 0;
			*is_jeita_updated = true;
		} else
			pr_debug("PMU: %s *** 12<Temp<50, already set vfchg==%d, so no need to update ******\n",
					__func__, info->jt_vfchg_h);

		/* set ICHG */
		err = ricoh61x_write(info->dev->parent, CHGISET_REG, set_ichg_h);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the battery setting register\n");
			goto out;
		}
		err = ricoh61x_set_bits(info->dev->parent, CHGCTL1_REG, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
	} else if (temp < 55) {
		/* 4th temperature range (50~55) */
		if (vfchg != info->jt_vfchg_l) {
			pr_debug("PMU: %s *** 50<Temp<55, update to vfchg==%d ******\n", __func__, info->jt_vfchg_l);

			err = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, 0x03);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the control register\n");
				goto out;
			}
			/* set VFCHG/VRCHG */
			err = ricoh61x_write(info->dev->parent,
							 BATSET2_REG, set_vchg_l);
			if (err < 0) {
				dev_err(info->dev, "Error in writing the battery setting register\n");
				goto out;
			}
			info->soca->jt_limit = 1;
			*is_jeita_updated = true;
		} else
			pr_debug("JEITA: %s *** 50<Temp<55, already set vfchg==%d, so no need to update ******\n",
					__func__, info->jt_vfchg_l);

		/* set ICHG */
		err = ricoh61x_write(info->dev->parent, CHGISET_REG, set_ichg_h);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the battery setting register\n");
			goto out;
		}
		err = ricoh61x_set_bits(info->dev->parent, CHGCTL1_REG, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto out;
		}
	}

	get_power_supply_status(info);
	pr_debug("PMU: %s *** Hope updating value in this timing after checking jeita, chg_status: %d, is_jeita_updated: %d ******\n",
		 __func__, info->soca->chg_status, *is_jeita_updated);

	return 0;

out:
	pr_debug("PMU: %s ERROR ******\n", __func__);
	return err;
}

static void ricoh61x_jeita_work(struct work_struct *work)
{
	int ret;
	bool is_jeita_updated = false;
	struct ricoh61x_battery_info *info = container_of(work,
		struct ricoh61x_battery_info, jeita_work.work);

	mutex_lock(&info->lock);

	ret = check_jeita_status(info, &is_jeita_updated);
	if (0 == ret) {
		queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
					 RICOH61x_JEITA_UPDATE_TIME * HZ);
	} else {
		pr_debug("PMU: %s *** Call check_jeita_status() in jeita_work, err:%d ******\n",
							__func__, ret);
		queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
					 RICOH61x_FG_RESET_TIME * HZ);
	}

	mutex_unlock(&info->lock);

	if(true == is_jeita_updated)
		power_supply_changed(&info->battery);

	return;
}

#ifdef ENABLE_FACTORY_MODE
/*------------------------------------------------------*/
/* Factory Mode						*/
/*    Check Battery exist or not			*/
/*    If not, disabled Rapid to Complete State change	*/
/*------------------------------------------------------*/
static int ricoh61x_factory_mode(struct ricoh61x_battery_info *info)
{
	int ret = 0;
	uint8_t val = 0;

	ret = ricoh61x_read(info->dev->parent, RICOH61x_INT_MON_CHGCTR, &val);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return ret;
	}
	if (!(val & 0x01)) /* No Adapter connected */
		return ret;

	/* Rapid to Complete State change disable */
	ret = ricoh61x_set_bits(info->dev->parent, CHGCTL1_REG, 0x40);

	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return ret;
	}

	/* Wait 1s for checking Charging State */
	queue_delayed_work(info->factory_mode_wqueue, &info->factory_mode_work,
			 1*HZ);

	return ret;
}

static void check_charging_state_work(struct work_struct *work)
{
	struct ricoh61x_battery_info *info = container_of(work,
		struct ricoh61x_battery_info, factory_mode_work.work);

	int ret = 0;
	uint8_t val = 0;
	int chargeCurrent = 0;

	ret = ricoh61x_read(info->dev->parent, CHGSTATE_REG, &val);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return;
	}


	chargeCurrent = get_check_fuel_gauge_reg(info, CC_AVERAGE1_REG,
						 CC_AVERAGE0_REG, 0x3fff);
	if (chargeCurrent < 0) {
		dev_err(info->dev, "Error in reading the FG register\n");
		return;
	}

	/* Repid State && Charge Current about 0mA */
	if (((chargeCurrent >= 0x3ffc && chargeCurrent <= 0x3fff)
		|| chargeCurrent < 0x05) && val == 0x43) {
		pr_debug("PMU:%s --- No battery !! Enter Factory mode ---\n"
				, __func__);
		info->entry_factory_mode = true;
		/* clear FG_ACC bit */
		ret = ricoh61x_clr_bits(info->dev->parent, RICOH61x_FG_CTRL, 0x10);
		if (ret < 0)
			dev_err(info->dev, "Error in writing FG_CTRL\n");

		return;	/* Factory Mode */
	}

	/* Return Normal Mode --> Rapid to Complete State change enable */
	/* disable the status change from Rapid Charge to Charge Complete */

	ret = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, 0x40);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return;
	}
	pr_debug("PMU:%s --- Battery exist !! Return Normal mode ---0x%2x\n"
			, __func__, val);

	return;
}
#endif /* ENABLE_FACTORY_MODE */

static int Calc_Linear_Interpolation(int x0, int y0, int x1, int y1, int y)
{
	int	alpha;
	int x;

	alpha = (y - y0)*100 / (y1 - y0);

	x = ((100 - alpha) * x0 + alpha * x1) / 100;

	return x;
}

static void ricoh61x_scaling_OCV_table(struct ricoh61x_battery_info *info, int cutoff_vol, int full_vol, int *start_per, int *end_per)
{
	int		i, j;
	int		temp;
	int		percent_step;
	int		OCV_percent_new[11];

	/* get ocv table. this table is calculated by Apprication */
	//pr_debug("PMU : %s : original table\n",__func__);
	//for (i = 0; i <= 10; i = i+1) {
	//	pr_debug("PMU: %s : %d0%% voltage = %d uV\n",
	//			 __func__, i, info->soca->ocv_table_def[i]);
	//}
	//pr_debug("PMU: %s : cutoff_vol %d full_vol %d\n",
	//			 __func__, cutoff_vol,full_vol);

	/* Check Start % */
	if (info->soca->ocv_table_def[0] > cutoff_vol * 1000) {
		*start_per = 0;
		pr_debug("PMU : %s : setting value of cuttoff_vol(%d) is out of range(%d) \n",__func__, cutoff_vol, info->soca->ocv_table_def[0]);
	} else {
		for (i = 1; i < 11; i++) {
			if (info->soca->ocv_table_def[i] >= cutoff_vol * 1000) {
				/* unit is 0.001% */
				*start_per = Calc_Linear_Interpolation(
					(i-1)*1000, info->soca->ocv_table_def[i-1], i*1000,
					info->soca->ocv_table_def[i], (cutoff_vol * 1000));
				break;
			}
		}
	}

	/* Check End % */
	for (i = 1; i < 11; i++) {
		if (info->soca->ocv_table_def[i] >= full_vol * 1000) {
			/* unit is 0.001% */
			*end_per = Calc_Linear_Interpolation(
				(i-1)*1000, info->soca->ocv_table_def[i-1], i*1000,
				 info->soca->ocv_table_def[i], (full_vol * 1000));
			break;
		}
	}

	/* calc new ocv percent */
	percent_step = ( *end_per - *start_per) / 10;
	//pr_debug("PMU : %s : percent_step is %d end per is %d start per is %d\n",__func__, percent_step, *end_per, *start_per);

	for (i = 0; i < 11; i++) {
		OCV_percent_new[i]
			 = *start_per + percent_step*(i - 0);
	}

	/* calc new ocv voltage */
	for (i = 0; i < 11; i++) {
		for (j = 1; j < 11; j++) {
			if (1000*j >= OCV_percent_new[i]) {
				temp = Calc_Linear_Interpolation(
					info->soca->ocv_table_def[j-1], (j-1)*1000,
					info->soca->ocv_table_def[j] , j*1000,
					 OCV_percent_new[i]);

				temp = ( (temp/1000) * 4095 ) / 5000;

				battery_init_para[info->num][i*2 + 1] = temp;
				battery_init_para[info->num][i*2] = temp >> 8;

				break;
			}
		}
	}
	for (i = 0; i <= 10; i = i+1) {
		temp = (battery_init_para[info->num][i*2]<<8)
			 | (battery_init_para[info->num][i*2+1]);
		/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
		temp = ((temp * 50000 * 10 / 4095) + 5) / 10;
	}

}

static int ricoh61x_set_OCV_table(struct ricoh61x_battery_info *info)
{
	int		ret = 0;
	int		i;
	int		full_ocv;
	int		available_cap;
	int		available_cap_ori;
	int		temp;
	int		temp1;
	int		start_per = 0;
	int		end_per = 0;
	int		Rbat;
	int		Ibat_min;
	uint8_t val;
	uint8_t val2;
	uint8_t val_temp;


	//get ocv table
	for (i = 0; i <= 10; i = i+1) {
		info->soca->ocv_table_def[i] = get_OCV_voltage(info, i);
	/*	pr_debug("PMU: %s : %d0%% voltage = %d uV\n",
			 __func__, i, info->soca->ocv_table_def[i]);*/
	}

	temp =  (battery_init_para[info->num][24]<<8) | (battery_init_para[info->num][25]);
	Rbat = temp * 1000 / 512 * 5000 / 4095;
	info->soca->Rsys = Rbat + 55;

	if ((info->fg_target_ibat == 0) || (info->fg_target_vsys == 0)) {	/* normal version */

		temp =  (battery_init_para[info->num][22]<<8) | (battery_init_para[info->num][23]);
		//fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
		//				0x7fff);

		info->soca->target_ibat = temp*2/10; /* calc 0.2C*/
		temp1 =  (battery_init_para[info->num][0]<<8) | (battery_init_para[info->num][1]);
//		temp = get_OCV_voltage(info, 0) / 1000; /* unit is 1mv*/
//		info->soca->cutoff_ocv = info->soca->target_vsys - Ibat_min * info->soca->Rsys / 1000;

		info->soca->target_vsys = temp1 + ( info->soca->target_ibat * info->soca->Rsys ) / 1000;


	} else {
		info->soca->target_ibat = info->fg_target_ibat;
		/* calc min vsys value */
		temp1 =  (battery_init_para[info->num][0]<<8) | (battery_init_para[info->num][1]);
		temp = temp1 + ( info->soca->target_ibat * info->soca->Rsys ) / 1000;
		if( temp < info->fg_target_vsys) {
			info->soca->target_vsys = info->fg_target_vsys;
		} else {
			info->soca->target_vsys = temp;
			pr_debug("PMU : %s : setting value of target vsys(%d) is out of range(%d)\n",__func__, info->fg_target_vsys, temp);
		}
	}

	//for debug
	//pr_debug("PMU : %s : target_vsys is %d target_ibat is %d\n",__func__,info->soca->target_vsys,info->soca->target_ibat * 20 / info->fg_rsense_val);

	if ((info->soca->target_ibat == 0) || (info->soca->target_vsys == 0)) {	/* normal version */
	} else {	/*Slice cutoff voltage version. */

		Ibat_min = -1 * info->soca->target_ibat;
		info->soca->cutoff_ocv = info->soca->target_vsys - Ibat_min * info->soca->Rsys / 1000;

		full_ocv = (battery_init_para[info->num][20]<<8) | (battery_init_para[info->num][21]);
		full_ocv = full_ocv * 5000 / 4095;

		ricoh61x_scaling_OCV_table(info, info->soca->cutoff_ocv, full_ocv, &start_per, &end_per);

		/* calc available capacity */
		/* get avilable capacity */
		/* battery_init_para23-24 is designe capacity */
		available_cap = (battery_init_para[info->num][22]<<8)
					 | (battery_init_para[info->num][23]);

		available_cap = available_cap
			 * ((10000 - start_per) / 100) / 100 ;


		battery_init_para[info->num][23] =  available_cap;
		battery_init_para[info->num][22] =  available_cap >> 8;

	}
	ret = ricoh61x_clr_bits(info->dev->parent, FG_CTRL_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev, "error in FG_En off\n");
		goto err;
	}
	/////////////////////////////////
	ret = ricoh61x_read_bank1(info->dev->parent, 0xDC, &val);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	val_temp = val;
	val	&= 0x0F; //clear bit 4-7
	val	|= 0x10; //set bit 4

	ret = ricoh61x_write_bank1(info->dev->parent, 0xDC, val);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	ret = ricoh61x_read_bank1(info->dev->parent, 0xDC, &val2);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	ret = ricoh61x_write_bank1(info->dev->parent, 0xDC, val_temp);
	if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
	}

	//pr_debug("PMU : %s : original 0x%x, before 0x%x, after 0x%x\n",__func__, val_temp, val, val2);

	if (val != val2) {
		ret = ricoh61x_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 30, battery_init_para[info->num]);
		if (ret < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			goto err;
		}
	} else {
		ret = ricoh61x_read_bank1(info->dev->parent, 0xD2, &val);
		if (ret < 0) {
		dev_err(info->dev, "batterry initialize error\n");
		goto err;
		}

		ret = ricoh61x_read_bank1(info->dev->parent, 0xD3, &val2);
		if (ret < 0) {
			dev_err(info->dev, "batterry initialize error\n");
			goto err;
		}

		available_cap_ori = val2 + (val << 8);
		available_cap = battery_init_para[info->num][23]
						+ (battery_init_para[info->num][22] << 8);

		if (available_cap_ori == available_cap) {
			ret = ricoh61x_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 22, battery_init_para[info->num]);
			if (ret < 0) {
				dev_err(info->dev, "batterry initialize error\n");
				return ret;
			}

			for (i = 0; i < 6; i++) {
				ret = ricoh61x_write_bank1(info->dev->parent, 0xD4+i, battery_init_para[info->num][24+i]);
				if (ret < 0) {
					dev_err(info->dev, "batterry initialize error\n");
					return ret;
				}
			}
		} else {
			ret = ricoh61x_bulk_writes_bank1(info->dev->parent,
				BAT_INIT_TOP_REG, 30, battery_init_para[info->num]);
			if (ret < 0) {
				dev_err(info->dev, "batterry initialize error\n");
				goto err;
			}
		}
	}
	////////////////////////////////

	return 0;
err:
	return ret;
}

/* Initial setting of battery */
static int ricoh61x_init_battery(struct ricoh61x_battery_info *info)
{
	int ret = 0;
	uint8_t val;
	uint8_t val2;
	/* Need to implement initial setting of batery and error */
	/* -------------------------- */
#ifdef ENABLE_FUEL_GAUGE_FUNCTION

	/* set relaxation state */
	if (RICOH61x_REL1_SEL_VALUE > 240)
		val = 0x0F;
	else
		val = RICOH61x_REL1_SEL_VALUE / 16 ;

	/* set relaxation state */
	if (RICOH61x_REL2_SEL_VALUE > 120)
		val2 = 0x0F;
	else
		val2 = RICOH61x_REL2_SEL_VALUE / 8 ;

	val =  val + (val2 << 4);

	ret = ricoh61x_write_bank1(info->dev->parent, BAT_REL_SEL_REG, val);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing BAT_REL_SEL_REG\n");
		return ret;
	}

	ret = ricoh61x_read_bank1(info->dev->parent, BAT_REL_SEL_REG, &val);
	//pr_debug("PMU: -------  BAT_REL_SEL= %xh: =======\n",
	//	val);

	ret = ricoh61x_write_bank1(info->dev->parent, BAT_TA_SEL_REG, 0x00);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing BAT_TA_SEL_REG\n");
		return ret;
	}

//	ret = ricoh61x_read(info->dev->parent, FG_CTRL_REG, &val);
//	if (ret < 0) {
//		dev_err(info->dev, "Error in reading the control register\n");
//		return ret;
//	}

//	val = (val & 0x10) >> 4;
//	info->first_pwon = (val == 0) ? 1 : 0;
	ret = ricoh61x_read(info->dev->parent, PSWR_REG, &val);
	if (ret < 0) {
		dev_err(info->dev,"Error in reading PSWR_REG %d\n", ret);
		return ret;
	}
	info->first_pwon = (val == 0) ? 1 : 0;
	//info->first_pwon = 1;
	g_soc = val & 0x7f;

	info->soca->init_pswr = val & 0x7f;
	pr_debug("PMU FG_RESET : %s : initial pswr = %d\n",__func__,info->soca->init_pswr);

	if(info->first_pwon) {
		info->soca->rsoc_ready_flag = 1;
	}else {
		info->soca->rsoc_ready_flag = 0;
	}

	ret = ricoh61x_set_OCV_table(info);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the OCV Tabler\n");
		return ret;
	}

	ret = ricoh61x_write(info->dev->parent, FG_CTRL_REG, 0x11);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return ret;
	}

#endif

	ret = ricoh61x_write(info->dev->parent, VINDAC_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		return ret;
	}

	if (info->alarm_vol_mv < 2700 || info->alarm_vol_mv > 3400) {
		dev_err(info->dev, "alarm_vol_mv is out of range!\n");
		return -1;
	}

	return ret;
}

/* Initial setting of charger */
static int ricoh61x_init_charger(struct ricoh61x_battery_info *info)
{
	int err;
	uint8_t val;
	uint8_t val2;
	uint8_t val3;
	int charge_status;
	int	vfchg_val;
	int	icchg_val;
	int	rbat;
	int	temp;

	info->chg_ctr = 0;
	info->chg_stat1 = 0;

	err = ricoh61x_set_bits(info->dev->parent, RICOH61x_PWR_FUNC, 0x20);
	if (err < 0) {
		dev_err(info->dev, "Error in writing the PWR FUNC register\n");
		goto free_device;
	}

	charge_status = get_power_supply_status(info);

	if (charge_status != POWER_SUPPLY_STATUS_FULL)
	{
		/* Disable charging */
		err = ricoh61x_clr_bits(info->dev->parent,CHGCTL1_REG, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto free_device;
		}
	}


	/* REGISET1:(0xB6) setting */
	if ((info->ch_ilim_adp != 0xFF) || (info->ch_ilim_adp <= 0x1D)) {
		val = info->ch_ilim_adp;

		err = ricoh61x_write(info->dev->parent, REGISET1_REG,val);
		if (err < 0) {
			dev_err(info->dev, "Error in writing REGISET1_REG %d\n",
										 err);
			goto free_device;
		}
		info->adp_current_val = val;
	}
	else info->adp_current_val = 0xff;

		/* REGISET2:(0xB7) setting */
	err = ricoh61x_read(info->dev->parent, REGISET2_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 	"Error in read REGISET2_REG %d\n", err);
		goto free_device;
	}

	if ((info->ch_ilim_usb != 0xFF) || (info->ch_ilim_usb <= 0x1D)) {
		val2 = info->ch_ilim_usb;
	} else {/* Keep OTP value */
		val2 = (val & 0x1F);
	}

		/* keep bit 5-7 */
	val &= 0xE0;

	val = val + val2;
	info->usb_current_val = val;
	err = ricoh61x_write(info->dev->parent, REGISET2_REG,val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing REGISET2_REG %d\n",
									 err);
		goto free_device;
	}

	err = ricoh61x_read(info->dev->parent, CHGISET_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 	"Error in read CHGISET_REG %d\n", err);
		goto free_device;
	}

		/* Define Current settings value for charging (bit 4~0)*/
	if ((info->ch_ichg != 0xFF) || (info->ch_ichg <= 0x1D)) {
		val2 = info->ch_ichg;
	} else { /* Keep OTP value */
		val2 = (val & 0x1F);
	}

		/* Define Current settings at the charge completion (bit 7~6)*/
	if ((info->ch_icchg != 0xFF) || (info->ch_icchg <= 0x03)) {
		val3 = info->ch_icchg << 6;
	} else { /* Keep OTP value */
		val3 = (val & 0xC0);
	}

	val = val2 + val3;

	err = ricoh61x_write(info->dev->parent, CHGISET_REG, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing CHGISET_REG %d\n",
									 err);
		goto free_device;
	}

		//debug messeage
	err = ricoh61x_read(info->dev->parent, CHGISET_REG,&val);
	pr_debug("PMU : %s : after CHGISET_REG (0x%x) is 0x%x info->ch_ichg is 0x%x info->ch_icchg is 0x%x\n",__func__,CHGISET_REG,val,info->ch_ichg,info->ch_icchg);

		//debug messeage
	err = ricoh61x_read(info->dev->parent, BATSET1_REG,&val);
	pr_debug("PMU : %s : before BATSET1_REG (0x%x) is 0x%x info->ch_vbatovset is 0x%x\n",__func__,BATSET1_REG,val,info->ch_vbatovset);

	/* BATSET1_REG(0xBA) setting */
	err = ricoh61x_read(info->dev->parent, BATSET1_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 	"Error in read BATSET1 register %d\n", err);
		goto free_device;
	}

		/* Define Battery overvoltage  (bit 4)*/
	if ((info->ch_vbatovset != 0xFF) || (info->ch_vbatovset <= 0x1)) {
		val2 = info->ch_vbatovset;
		val2 = val2 << 4;
	} else { /* Keep OTP value */
		val2 = (val & 0x10);
	}

		/* keep bit 0-3 and bit 5-7 */
	val = (val & 0xEF);

	val = val + val2;

	err = ricoh61x_write(info->dev->parent, BATSET1_REG, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing BAT1_REG %d\n",
									 err);
		goto free_device;
	}
		//debug messeage
	err = ricoh61x_read(info->dev->parent, BATSET1_REG,&val);
	//pr_debug("PMU : %s : after BATSET1_REG (0x%x) is 0x%x info->ch_vbatovset is 0x%x\n",__func__,BATSET1_REG,val,info->ch_vbatovset);

		//debug messeage
	err = ricoh61x_read(info->dev->parent, BATSET2_REG,&val);
	//pr_debug("PMU : %s : before BATSET2_REG (0x%x) is 0x%x info->ch_vrchg is 0x%x info->ch_vfchg is 0x%x \n",__func__,BATSET2_REG,val,info->ch_vrchg,info->ch_vfchg);


	/* BATSET2_REG(0xBB) setting */
	err = ricoh61x_read(info->dev->parent, BATSET2_REG, &val);
	if (err < 0) {
		dev_err(info->dev,
	 	"Error in read BATSET2 register %d\n", err);
		goto free_device;
	}

		/* Define Re-charging voltage (bit 2~0)*/
	if ((info->ch_vrchg != 0xFF) || (info->ch_vrchg <= 0x04)) {
		val2 = info->ch_vrchg;
	} else { /* Keep OTP value */
		val2 = (val & 0x07);
	}

		/* Define FULL charging voltage (bit 6~4)*/
	if ((info->ch_vfchg != 0xFF) || (info->ch_vfchg <= 0x04)) {
		val3 = info->ch_vfchg;
		val3 = val3 << 4;
	} else {	/* Keep OTP value */
		val3 = (val & 0x70);
	}

		/* keep bit 3 and bit 7 */
	val = (val & 0x88);

	val = val + val2 + val3;

	err = ricoh61x_write(info->dev->parent, BATSET2_REG, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing RICOH61x_RE_CHARGE_VOLTAGE %d\n",
									 err);
		goto free_device;
	}

	/* Set rising edge setting ([1:0]=01b)for INT in charging */
	/*  and rising edge setting ([3:2]=01b)for charge completion */
	err = ricoh61x_read(info->dev->parent, RICOH61x_CHG_STAT_DETMOD1, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading CHG_STAT_DETMOD1 %d\n",
								 err);
		goto free_device;
	}
	val &= 0xf0;
	val |= 0x05;
	err = ricoh61x_write(info->dev->parent, RICOH61x_CHG_STAT_DETMOD1, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing CHG_STAT_DETMOD1 %d\n",
								 err);
		goto free_device;
	}

	/* Unmask In charging/charge completion */
	err = ricoh61x_write(info->dev->parent, RICOH61x_INT_MSK_CHGSTS1, 0xfc);
	if (err < 0) {
		dev_err(info->dev, "Error in writing INT_MSK_CHGSTS1 %d\n",
								 err);
		goto free_device;
	}

	/* Set both edge for VUSB([3:2]=11b)/VADP([1:0]=11b) detect */
	err = ricoh61x_read(info->dev->parent, RICOH61x_CHG_CTRL_DETMOD1, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading CHG_CTRL_DETMOD1 %d\n",
								 err);
		goto free_device;
	}
	val &= 0xf0;
	val |= 0x0f;
	err = ricoh61x_write(info->dev->parent, RICOH61x_CHG_CTRL_DETMOD1, val);
	if (err < 0) {
		dev_err(info->dev, "Error in writing CHG_CTRL_DETMOD1 %d\n",
								 err);
		goto free_device;
	}

	/* Unmask In VUSB/VADP completion */
	err = ricoh61x_write(info->dev->parent, RICOH61x_INT_MSK_CHGCTR, 0xfc);
	if (err < 0) {
		dev_err(info->dev, "Error in writing INT_MSK_CHGSTS1 %d\n",
								 err);
		goto free_device;
	}


	if (charge_status != POWER_SUPPLY_STATUS_FULL)
	{
		/* Enable charging */
		err = ricoh61x_set_bits(info->dev->parent,CHGCTL1_REG, 0x03);
		if (err < 0) {
			dev_err(info->dev, "Error in writing the control register\n");
			goto free_device;
		}
	}
	/* get OCV100_min, OCV100_min*/
	temp = (battery_init_para[info->num][24]<<8) | (battery_init_para[info->num][25]);
	rbat = temp * 1000 / 512 * 5000 / 4095;

	/* get vfchg value */
	err = ricoh61x_read(info->dev->parent, BATSET2_REG, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading the batset2reg\n");
		goto free_device;
	}
	val &= 0x70;
	val2 = val >> 4;
	if (val2 <= 3) {
		vfchg_val = 4050 + val2 * 50;
	} else {
		vfchg_val = 4350;
	}
	//pr_debug("PMU : %s : test test val %d, val2 %d vfchg %d\n", __func__, val, val2, vfchg_val);

	/* get  value */
	err = ricoh61x_read(info->dev->parent, CHGISET_REG, &val);
	if (err < 0) {
		dev_err(info->dev, "Error in reading the chgisetreg\n");
		goto free_device;
	}
	val &= 0xC0;
	val2 = val >> 6;
	icchg_val = 50 + val2 * 50;
	//pr_debug("PMU : %s : test test val %d, val2 %d icchg %d\n", __func__, val, val2, icchg_val);

	info->soca->OCV100_min = ( vfchg_val * 99 / 100 - (icchg_val * (rbat +20))/1000 - 20 ) * 1000;
	info->soca->OCV100_max = ( vfchg_val * 101 / 100 - (icchg_val * (rbat +20))/1000 + 20 ) * 1000;

//	pr_debug("PMU : %s : 100 min %d, 100 max %d vfchg %d icchg %d rbat %d\n",__func__,
//	info->soca->OCV100_min,info->soca->OCV100_max,vfchg_val,icchg_val,rbat);

#ifdef ENABLE_LOW_BATTERY_DETECTION
	/* Set ADRQ=00 to stop ADC */
	ricoh61x_write(info->dev->parent, RICOH61x_ADC_CNT3, 0x0);
	/* Enable VSYS threshold Low interrupt */
	ricoh61x_write(info->dev->parent, RICOH61x_INT_EN_ADC1, 0x10);
	/* Set ADC auto conversion interval 250ms */
	ricoh61x_write(info->dev->parent, RICOH61x_ADC_CNT2, 0x0);
	/* Enable VSYS pin conversion in auto-ADC */
	ricoh61x_write(info->dev->parent, RICOH61x_ADC_CNT1, 0x10);
	/* Set VSYS threshold low voltage value = (voltage(V)*255)/(3*2.5) */
	val = info->alarm_vol_mv * 255 / 7500;
	ricoh61x_write(info->dev->parent, RICOH61x_ADC_VSYS_THL, val);
	/* Start auto-mode & average 4-time conversion mode for ADC */
	ricoh61x_write(info->dev->parent, RICOH61x_ADC_CNT3, 0x28);

#endif

free_device:
	return err;
}


static int get_power_supply_status(struct ricoh61x_battery_info *info)
{
	uint8_t status;
	uint8_t supply_state;
	uint8_t charge_state;
	int ret = 0;

	/* get  power supply status */
	ret = ricoh61x_read(info->dev->parent, CHGSTATE_REG, &status);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return ret;
	}

	charge_state = (status & 0x1F);
	supply_state = ((status & 0xC0) >> 6);

	if (info->entry_factory_mode)
			return POWER_SUPPLY_STATUS_NOT_CHARGING;

	if (supply_state == SUPPLY_STATE_BAT) {
		info->soca->chg_status = POWER_SUPPLY_STATUS_DISCHARGING;
	} else {
		switch (charge_state) {
		case	CHG_STATE_CHG_OFF:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_CHG_READY_VADP:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_CHG_TRICKLE:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_CHARGING;
				break;
		case	CHG_STATE_CHG_RAPID:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_CHARGING;
				break;
		case	CHG_STATE_CHG_COMPLETE:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_FULL;
				break;
		case	CHG_STATE_SUSPEND:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_VCHG_OVER_VOL:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_BAT_ERROR:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_NO_BAT:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_BAT_OVER_VOL:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_BAT_TEMP_ERR:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_DIE_ERR:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_DIE_SHUTDOWN:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_DISCHARGING;
				break;
		case	CHG_STATE_NO_BAT2:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		case	CHG_STATE_CHG_READY_VUSB:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;
		default:
				info->soca->chg_status
					= POWER_SUPPLY_STATUS_UNKNOWN;
				break;
		}
	}

	return info->soca->chg_status;
}

static int get_power_supply_Android_status(struct ricoh61x_battery_info *info)
{

	get_power_supply_status(info);

	/* get  power supply status */
	if (info->entry_factory_mode)
			return POWER_SUPPLY_STATUS_NOT_CHARGING;

	switch (info->soca->chg_status) {
		case	POWER_SUPPLY_STATUS_UNKNOWN:
				return POWER_SUPPLY_STATUS_UNKNOWN;
				break;

		case	POWER_SUPPLY_STATUS_NOT_CHARGING:
				return POWER_SUPPLY_STATUS_NOT_CHARGING;
				break;

		case	POWER_SUPPLY_STATUS_DISCHARGING:
				return POWER_SUPPLY_STATUS_DISCHARGING;
				break;

		case	POWER_SUPPLY_STATUS_CHARGING:
				return POWER_SUPPLY_STATUS_CHARGING;
				break;

		case	POWER_SUPPLY_STATUS_FULL:
				if(info->soca->displayed_soc == 100 * 100) {
					return POWER_SUPPLY_STATUS_FULL;
				} else {
					return POWER_SUPPLY_STATUS_CHARGING;
				}
				break;
		default:
				return POWER_SUPPLY_STATUS_UNKNOWN;
				break;
	}

	return POWER_SUPPLY_STATUS_UNKNOWN;
}

static void charger_irq_work(struct work_struct *work)
{
	struct ricoh61x_battery_info *info
		 = container_of(work, struct ricoh61x_battery_info, irq_work);
	int ret = 0;
	uint8_t val = 0;
	//uint8_t adp_current_val = 0x0E;
	//uint8_t usb_current_val = 0x04;

	power_supply_changed(&info->battery);

	mutex_lock(&info->lock);

#if 0
	if(info->chg_ctr & 0x02) {
		uint8_t sts;
		ret = ricoh61x_read(info->dev->parent, RICOH61x_INT_MON_CHGCTR, &sts);
		if (ret < 0)
			dev_err(info->dev, "Error in reading the control register\n");

		sts &= 0x02;

		/* If "sts" is true, USB is plugged. If not, unplugged. */
	}
#endif
	info->chg_ctr = 0;
	info->chg_stat1 = 0;

	/* Enable Interrupt for VADP/VUSB */
	ret = ricoh61x_write(info->dev->parent, RICOH61x_INT_MSK_CHGCTR, 0xfc);
	if (ret < 0)
		dev_err(info->dev,
			 "%s(): Error in enable charger mask INT %d\n",
			 __func__, ret);

	/* Enable Interrupt for Charging & complete */
	ret = ricoh61x_write(info->dev->parent, RICOH61x_INT_MSK_CHGSTS1, 0xfc);
	if (ret < 0)
		dev_err(info->dev,
			 "%s(): Error in enable charger mask INT %d\n",
			 __func__, ret);

	/* set USB/ADP ILIM */
	ret = ricoh61x_read(info->dev->parent, CHGSTATE_REG, &val);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		return;
	}

	val = (val & 0xC0) >> 6;
	switch (val) {
	case	0: // plug out USB/ADP
			pr_debug("%s : val = %d plug out\n",__func__, val);
			break;
	case	1: // plug in ADP
			pr_debug("%s : val = %d plug in ADPt\n",__func__, val);
			//Add the code of AC adapter Charge and Limit current settings
			//ret = ricoh61x_write(info->dev->parent, REGISET1_REG, info->adp_current_val);
			break;
	case	2:// plug in USB
			pr_debug("%s : val = %d plug in USB\n",__func__, val);
			//Add the code of USB Charge and Limit current settings
			ret = ricoh61x_write(info->dev->parent, REGISET2_REG, info->usb_current_val);
			break;
	case	3:// plug in USB/ADP
			pr_debug("%s : val = %d plug in ADP USB\n",__func__, val);
			break;
	default:
			pr_debug("%s : val = %d unknown\n",__func__, val);
			break;
	}

	mutex_unlock(&info->lock);
}

#ifdef ENABLE_LOW_BATTERY_DETECTION
static void low_battery_irq_work(struct work_struct *work)
{
	struct ricoh61x_battery_info *info = container_of(work,
		 struct ricoh61x_battery_info, low_battery_work.work);

	int ret = 0;

	power_supply_changed(&info->battery);

	/* Enable VADP threshold Low interrupt */
	ricoh61x_write(info->dev->parent, RICOH61x_INT_EN_ADC1, 0x10);
	if (ret < 0)
		dev_err(info->dev,
			 "%s(): Error in enable adc mask INT %d\n",
			 __func__, ret);
}
#endif

static irqreturn_t charger_in_isr(int irq, void *battery_info)
{
	struct ricoh61x_battery_info *info = battery_info;

	info->chg_stat1 |= 0x01;
	queue_work(info->workqueue, &info->irq_work);
	return IRQ_HANDLED;
}

static irqreturn_t charger_complete_isr(int irq, void *battery_info)
{
	struct ricoh61x_battery_info *info = battery_info;

	info->chg_stat1 |= 0x02;
	queue_work(info->workqueue, &info->irq_work);

	return IRQ_HANDLED;
}

static irqreturn_t charger_usb_isr(int irq, void *battery_info)
{
	struct ricoh61x_battery_info *info = battery_info;

	info->chg_ctr |= 0x02;

	queue_work(info->workqueue, &info->irq_work);

	info->soca->dischg_state = 0;
	info->soca->chg_count = 0;
	if (RICOH61x_SOCA_UNSTABLE == info->soca->status
		|| RICOH61x_SOCA_FG_RESET == info->soca->status)
		info->soca->stable_count = 11;

	return IRQ_HANDLED;
}

static irqreturn_t charger_adp_isr(int irq, void *battery_info)
{
	struct ricoh61x_battery_info *info = battery_info;

	info->chg_ctr |= 0x01;
	queue_work(info->workqueue, &info->irq_work);

	info->soca->dischg_state = 0;
	info->soca->chg_count = 0;
	if (RICOH61x_SOCA_UNSTABLE == info->soca->status
		|| RICOH61x_SOCA_FG_RESET == info->soca->status)
		info->soca->stable_count = 11;

	return IRQ_HANDLED;
}


#ifdef ENABLE_LOW_BATTERY_DETECTION
/*************************************************************/
/* for Detecting Low Battery                                 */
/*************************************************************/

static irqreturn_t adc_vsysl_isr(int irq, void *battery_info)
{

	struct ricoh61x_battery_info *info = battery_info;

	queue_delayed_work(info->monitor_wqueue, &info->low_battery_work,
					LOW_BATTERY_DETECTION_TIME*HZ);

	return IRQ_HANDLED;
}
#endif

/*
 * Get Charger Priority
 * - get higher-priority between VADP and VUSB
 * @ data: higher-priority is stored
 *         true : VUSB
 *         false: VADP
 */
static int get_charge_priority(struct ricoh61x_battery_info *info, bool *data)
{
	int ret = 0;
	uint8_t val = 0;

	ret = ricoh61x_read(info->dev->parent, CHGCTL1_REG, &val);
	val = val >> 7;
	*data = (bool)val;

	return ret;
}

/*
 * Set Charger Priority
 * - set higher-priority between VADP and VUSB
 * - data: higher-priority is stored
 *         true : VUSB
 *         false: VADP
 */
static int set_charge_priority(struct ricoh61x_battery_info *info, bool *data)
{
	int ret = 0;
	uint8_t val = 0x80;

	if (*data == 1)
		ret = ricoh61x_set_bits(info->dev->parent, CHGCTL1_REG, val);
	else
		ret = ricoh61x_clr_bits(info->dev->parent, CHGCTL1_REG, val);

	return ret;
}

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
static int get_check_fuel_gauge_reg(struct ricoh61x_battery_info *info,
					 int Reg_h, int Reg_l, int enable_bit)
{
	uint8_t get_data_h, get_data_l;
	int old_data, current_data;
	int i;
	int ret = 0;

	old_data = 0;

	for (i = 0; i < 5 ; i++) {
		ret = ricoh61x_read(info->dev->parent, Reg_h, &get_data_h);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			return ret;
		}

		ret = ricoh61x_read(info->dev->parent, Reg_l, &get_data_l);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			return ret;
		}

		current_data = ((get_data_h & 0xff) << 8) | (get_data_l & 0xff);
		current_data = (current_data & enable_bit);

		if (current_data == old_data)
			return current_data;
		else
			old_data = current_data;
	}

	return current_data;
}

static int calc_capacity(struct ricoh61x_battery_info *info)
{
	uint8_t capacity;
	long	capacity_l;
	int temp;
	int ret = 0;
	int nt;
	int temperature;
	int cc_cap = 0;
	int cc_delta;
	bool is_charging = true;

	if (info->soca->rsoc_ready_flag  != 0) {
		/* get remaining battery capacity from fuel gauge */
		ret = ricoh61x_read(info->dev->parent, SOC_REG, &capacity);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			return ret;
		}
		capacity_l = (long)capacity;
	} else {
		ret = calc_capacity_in_period(info, &cc_cap, &is_charging, false);
		cc_delta = (is_charging == true) ? cc_cap : -cc_cap;
		capacity_l = (info->soca->init_pswr * 100 + cc_delta) / 100;
		pr_debug("PMU FG_RESET : %s : capacity %d pswr %d cc_delta %d\n",__func__,	capacity_l, info->soca->init_pswr, cc_delta);
	}

	temperature = get_battery_temp_2(info) / 10; /* unit 0.1 degree -> 1 degree */

	if (temperature >= 25) {
		nt = 0;
	} else if (temperature >= 5) {
		nt = (25 - temperature) * RICOH61x_TAH_SEL2 * 625 / 100;
	} else {
		nt = (625  + (5 - temperature) * RICOH61x_TAL_SEL2 * 625 / 100);
	}

	temp = capacity_l * 100 * 100 / (10000 - nt);

	temp = min(100, temp);
	temp = max(0, temp);

	return temp;		/* Unit is 1% */
}

static int calc_capacity_2(struct ricoh61x_battery_info *info)
{
	uint8_t val;
	long capacity;
	int re_cap, fa_cap;
	int temp;
	int ret = 0;
	int nt;
	int temperature;
	int cc_cap = 0;
	int cc_delta;
	bool is_charging = true;


	if (info->soca->rsoc_ready_flag  != 0) {
		re_cap = get_check_fuel_gauge_reg(info, RE_CAP_H_REG, RE_CAP_L_REG,
							0x7fff);
		fa_cap = get_check_fuel_gauge_reg(info, FA_CAP_H_REG, FA_CAP_L_REG,
							0x7fff);

		if (fa_cap != 0) {
			capacity = ((long)re_cap * 100 * 100 / fa_cap);
			capacity = (long)(min(10000, (int)capacity));
			capacity = (long)(max(0, (int)capacity));
		} else {
			ret = ricoh61x_read(info->dev->parent, SOC_REG, &val);
			if (ret < 0) {
				dev_err(info->dev, "Error in reading the control register\n");
				return ret;
			}
			capacity = (long)val * 100;
		}
	} else {
		ret = calc_capacity_in_period(info, &cc_cap, &is_charging, false);
		cc_delta = (is_charging == true) ? cc_cap : -cc_cap;
		capacity = info->soca->init_pswr * 100 + cc_delta;
		pr_debug("PMU FG_RESET : %s : capacity %d pswr %d cc_delta %d\n",__func__,	(int)capacity, info->soca->init_pswr, cc_delta);
	}

	temperature = get_battery_temp_2(info) / 10; /* unit 0.1 degree -> 1 degree */

	if (temperature >= 25) {
		nt = 0;
	} else if (temperature >= 5) {
		nt = (25 - temperature) * RICOH61x_TAH_SEL2 * 625 / 100;
	} else {
		nt = (625  + (5 - temperature) * RICOH61x_TAL_SEL2 * 625 / 100);
	}

	temp = (int)(capacity * 100 * 100 / (10000 - nt));

	temp = min(10000, temp);
	temp = max(0, temp);

	return temp;		/* Unit is 0.01% */
}

static int get_battery_temp(struct ricoh61x_battery_info *info)
{
	int ret = 0;
	int sign_bit;

	ret = get_check_fuel_gauge_reg(info, TEMP_1_REG, TEMP_2_REG, 0x0fff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	/* bit3 of 0xED(TEMP_1) is sign_bit */
	sign_bit = ((ret & 0x0800) >> 11);

	ret = (ret & 0x07ff);

	if (sign_bit == 0)	/* positive value part */
		/* conversion unit */
		/* 1 unit is 0.0625 degree and retun unit
		 * should be 0.1 degree,
		 */
		ret = ret * 625  / 1000;
	else {	/*negative value part */
		ret = (~ret + 1) & 0x7ff;
		ret = -1 * ret * 625 / 1000;
	}

	return ret;
}

static int get_battery_temp_2(struct ricoh61x_battery_info *info)
{
	uint8_t reg_buff[2];
	long temp, temp_off, temp_gain;
	bool temp_sign, temp_off_sign, temp_gain_sign;
	int Vsns = 0;
	int Iout = 0;
	int Vthm, Rthm;
	int reg_val = 0;
	int new_temp;
	long R_ln1, R_ln2;
	int ret = 0;

	/* Calculate TEMP */
	ret = get_check_fuel_gauge_reg(info, TEMP_1_REG, TEMP_2_REG, 0x0fff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge register\n");
		goto out;
	}

	reg_val = ret;
	temp_sign = (reg_val & 0x0800) >> 11;
	reg_val = (reg_val & 0x07ff);

	if (temp_sign == 0)	/* positive value part */
		/* the unit is 0.0001 degree */
		temp = (long)reg_val * 625;
	else {	/*negative value part */
		reg_val = (~reg_val + 1) & 0x7ff;
		temp = -1 * (long)reg_val * 625;
	}

	/* Calculate TEMP_OFF */
	ret = ricoh61x_bulk_reads_bank1(info->dev->parent,
					TEMP_OFF_H_REG, 2, reg_buff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge register\n");
		goto out;
	}

	reg_val = reg_buff[0] << 8 | reg_buff[1];
	temp_off_sign = (reg_val & 0x0800) >> 11;
	reg_val = (reg_val & 0x07ff);

	if (temp_off_sign == 0)	/* positive value part */
		/* the unit is 0.0001 degree */
		temp_off = (long)reg_val * 625;
	else {	/*negative value part */
		reg_val = (~reg_val + 1) & 0x7ff;
		temp_off = -1 * (long)reg_val * 625;
	}

	/* Calculate TEMP_GAIN */
	ret = ricoh61x_bulk_reads_bank1(info->dev->parent,
					TEMP_GAIN_H_REG, 2, reg_buff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge register\n");
		goto out;
	}

	reg_val = reg_buff[0] << 8 | reg_buff[1];
	temp_gain_sign = (reg_val & 0x0800) >> 11;
	reg_val = (reg_val & 0x07ff);

	if (temp_gain_sign == 0)	/* positive value part */
		/* 1 unit is 0.000488281. the result is 0.01 */
		temp_gain = (long)reg_val * 488281 / 100000;
	else {	/*negative value part */
		reg_val = (~reg_val + 1) & 0x7ff;
		temp_gain = -1 * (long)reg_val * 488281 / 100000;
	}

	/* Calculate VTHM */
	if (0 != temp_gain)
		Vthm = (int)((temp - temp_off) / 4095 * 2500 / temp_gain);
	else {
		pr_debug("PMU %s Skip to compensate temperature\n", __func__);
		goto out;
	}

	ret = measure_Ibatt_FG(info, &Iout);
	Vsns = Iout * 2 / 100;

	if (temp < -120000) {
		/* Low Temperature */
		if (0 != (2500 - Vthm)) {
			Rthm = 10 * 10 * (Vthm - Vsns) / (2500 - Vthm);
		} else {
			pr_debug("PMU %s Skip to compensate temperature\n", __func__);
			goto out;
		}

		R_ln1 = Rthm / 10;
		R_ln2 =  (R_ln1 * R_ln1 * R_ln1 * R_ln1 * R_ln1 / 100000
			- R_ln1 * R_ln1 * R_ln1 * R_ln1 * 2 / 100
			+ R_ln1 * R_ln1 * R_ln1 * 11
			- R_ln1 * R_ln1 * 2980
			+ R_ln1 * 449800
			- 784000) / 10000;

		/* the unit of new_temp is 0.1 degree */
		new_temp = (int)((100 * 1000 * B_VALUE / (R_ln2 + B_VALUE * 100 * 1000 / 29815) - 27315) / 10);
		pr_debug("PMU %s low temperature %d\n", __func__, new_temp/10);
	} else if (temp > 520000) {
		/* High Temperature */
		if (0 != (2500 - Vthm)) {
			Rthm = 100 * 10 * (Vthm - Vsns) / (2500 - Vthm);
		} else {
			pr_debug("PMU %s Skip to compensate temperature\n", __func__);
			goto out;
		}
		pr_debug("PMU %s [Rthm] Rthm %d[ohm]\n", __func__, Rthm);

		R_ln1 = Rthm / 10;
		R_ln2 =  (R_ln1 * R_ln1 * R_ln1 * R_ln1 * R_ln1 / 100000 * 15652 / 100
			- R_ln1 * R_ln1 * R_ln1 * R_ln1 / 1000 * 23103 / 100
			+ R_ln1 * R_ln1 * R_ln1 * 1298 / 100
			- R_ln1 * R_ln1 * 35089 / 100
			+ R_ln1 * 50334 / 10
			- 48569) / 100;
		/* the unit of new_temp is 0.1 degree */
		new_temp = (int)((100 * 100 * B_VALUE / (R_ln2 + B_VALUE * 100 * 100 / 29815) - 27315) / 10);
		pr_debug("PMU %s high temperature %d\n", __func__, new_temp/10);
	} else {
		/* the unit of new_temp is 0.1 degree */
		new_temp = temp / 1000;
	}

	return new_temp;

out:
	new_temp = get_battery_temp(info);
	return new_temp;
}

static int get_time_to_empty(struct ricoh61x_battery_info *info)
{
	int ret = 0;

	ret = get_check_fuel_gauge_reg(info, TT_EMPTY_H_REG, TT_EMPTY_L_REG,
								0xffff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	/* conversion unit */
	/* 1unit is 1miniute and return nnit should be 1 second */
	ret = ret * 60;

	return ret;
}

static int get_time_to_full(struct ricoh61x_battery_info *info)
{
	int ret = 0;

	ret = get_check_fuel_gauge_reg(info, TT_FULL_H_REG, TT_FULL_L_REG,
								0xffff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	ret = ret * 60;

	return  ret;
}

/* battery voltage is get from Fuel gauge */
static int measure_vbatt_FG(struct ricoh61x_battery_info *info, int *data)
{
	int ret = 0;

	if(info->soca->ready_fg == 1) {
		ret = get_check_fuel_gauge_reg(info, VOLTAGE_1_REG, VOLTAGE_2_REG,
									0x0fff);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the fuel gauge control register\n");
			return ret;
		}

		*data = ret;
		/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
		*data = *data * 50000 / 4095;
		/* return unit should be 1uV */
		*data = *data * 100;
		info->soca->Vbat_old = *data;
	} else {
		*data = info->soca->Vbat_old;
	}

	return ret;
}

static int measure_Ibatt_FG(struct ricoh61x_battery_info *info, int *data)
{
	int ret = 0;

	ret =  get_check_fuel_gauge_reg(info, CC_AVERAGE1_REG,
						 CC_AVERAGE0_REG, 0x3fff);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the fuel gauge control register\n");
		return ret;
	}

	*data = (ret > 0x1fff) ? (ret - 0x4000) : ret;
	return ret;
}

static int get_OCV_init_Data(struct ricoh61x_battery_info *info, int index)
{
	int ret = 0;
	ret =  (battery_init_para[info->num][index*2]<<8) | (battery_init_para[info->num][index*2+1]);
	return ret;
}

static int get_OCV_voltage(struct ricoh61x_battery_info *info, int index)
{
	int ret = 0;
	ret =  get_OCV_init_Data(info, index);
	/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
	ret = ret * 50000 / 4095;
	/* return unit should be 1uV */
	ret = ret * 100;
	return ret;
}

#else
/* battery voltage is get from ADC */
static int measure_vbatt_ADC(struct ricoh61x_battery_info *info, int *data)
{
	int	i;
	uint8_t data_l = 0, data_h = 0;
	int ret;

	/* ADC interrupt enable */
	ret = ricoh61x_set_bits(info->dev->parent, INTEN_REG, 0x08);
	if (ret < 0) {
		dev_err(info->dev, "Error in setting the control register bit\n");
		goto err;
	}

	/* enable interrupt request of single mode */
	ret = ricoh61x_set_bits(info->dev->parent, EN_ADCIR3_REG, 0x01);
	if (ret < 0) {
		dev_err(info->dev, "Error in setting the control register bit\n");
		goto err;
	}

	/* single request */
	ret = ricoh61x_write(info->dev->parent, ADCCNT3_REG, 0x10);
	if (ret < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
		goto err;
	}

	for (i = 0; i < 5; i++) {
		usleep(1000);
		dev_dbg(info->dev, "ADC conversion times: %d\n", i);
		/* read completed flag of ADC */
		ret = ricoh61x_read(info->dev->parent, EN_ADCIR3_REG, &data_h);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			goto err;
		}

		if (data_h & 0x01)
			goto	done;
	}

	dev_err(info->dev, "ADC conversion too long!\n");
	goto err;

done:
	ret = ricoh61x_read(info->dev->parent, VBATDATAH_REG, &data_h);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		goto err;
	}

	ret = ricoh61x_read(info->dev->parent, VBATDATAL_REG, &data_l);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
		goto err;
	}

	*data = ((data_h & 0xff) << 4) | (data_l & 0x0f);
	/* conversion unit 1 Unit is 1.22mv (5000/4095 mv) */
	*data = *data * 5000 / 4095;
	/* return unit should be 1uV */
	*data = *data * 1000;

	return 0;

err:
	return -1;
}
#endif

static int measure_vsys_ADC(struct ricoh61x_battery_info *info, int *data)
{
	uint8_t data_l = 0, data_h = 0;
	int ret;

	ret = ricoh61x_read(info->dev->parent, VSYSDATAH_REG, &data_h);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
	}

	ret = ricoh61x_read(info->dev->parent, VSYSDATAL_REG, &data_l);
	if (ret < 0) {
		dev_err(info->dev, "Error in reading the control register\n");
	}

	*data = ((data_h & 0xff) << 4) | (data_l & 0x0f);
	*data = *data * 1000 * 3 * 5 / 2 / 4095;
	/* return unit should be 1uV */
	*data = *data * 1000;

	return 0;
}

static void ricoh61x_external_power_changed(struct power_supply *psy)
{
	struct ricoh61x_battery_info *info;

	info = container_of(psy, struct ricoh61x_battery_info, battery);
	queue_delayed_work(info->monitor_wqueue,
			   &info->changed_work, HZ / 2);
	return;
}

static int ricoh61x_batt_get_prop(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct ricoh61x_battery_info *info = dev_get_drvdata(psy->dev->parent);
	int data = 0;
	int ret = 0;
	uint8_t status;

	mutex_lock(&info->lock);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		ret = ricoh61x_read(info->dev->parent, CHGSTATE_REG, &status);
		if (ret < 0) {
			dev_err(info->dev, "Error in reading the control register\n");
			mutex_unlock(&info->lock);
			return ret;
		}
		if (psy->type == POWER_SUPPLY_TYPE_MAINS)
			val->intval = (status & 0x40 ? 1 : 0);
		else if (psy->type == POWER_SUPPLY_TYPE_USB)
			val->intval = (status & 0x80 ? 1 : 0);
		break;
	/* this setting is same as battery driver of 584 */
	case POWER_SUPPLY_PROP_STATUS:
		ret = get_power_supply_Android_status(info);
		val->intval = ret;
		info->status = ret;
		/* dev_dbg(info->dev, "Power Supply Status is %d\n",
							info->status); */
		break;

	/* this setting is same as battery driver of 584 */
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;/*info->present;*/
		break;

	/* current voltage is got from fuel gauge */
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* return real vbatt Voltage */
#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
		if (info->soca->ready_fg)
			ret = measure_vbatt_FG(info, &data);
		else {
			//val->intval = -EINVAL;
			data = info->cur_voltage * 1000;
		}
#else
		ret = measure_vbatt_ADC(info, &data);
#endif
		val->intval = data;
		/* convert unit uV -> mV */
		info->cur_voltage = data / 1000;
		dev_dbg(info->dev, "battery voltage is %d mV\n",
						info->cur_voltage);
		break;

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
	/* current battery capacity is get from fuel gauge */
	case POWER_SUPPLY_PROP_CAPACITY:
		if (info->entry_factory_mode){
			val->intval = 100;
			info->capacity = 100;
		} else if (info->soca->displayed_soc <= 0) {
			val->intval = 0;
			info->capacity = 0;
		} else {
			val->intval = (info->soca->displayed_soc + 50)/100;
			info->capacity = (info->soca->displayed_soc + 50)/100;
		}
		dev_dbg(info->dev, "battery capacity is %d%%\n",
							info->capacity);
		break;

	/* current temperature of battery */
	case POWER_SUPPLY_PROP_TEMP:
		if (info->soca->ready_fg) {
			ret = 0;
			val->intval = get_battery_temp_2(info);
			info->battery_temp = val->intval/10;
			dev_dbg(info->dev,
					 "battery temperature is %d degree\n",
							 info->battery_temp);
		} else {
			val->intval = info->battery_temp * 10;
			dev_dbg(info->dev, "battery temperature is %d degree\n", info->battery_temp);
		}
		break;

	case POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW:
		if (info->soca->ready_fg) {
			ret = get_time_to_empty(info);
			val->intval = ret;
			info->time_to_empty = ret/60;
			dev_dbg(info->dev,
				 "time of empty battery is %d minutes\n",
							 info->time_to_empty);
		} else {
			//val->intval = -EINVAL;
			val->intval = info->time_to_empty * 60;
			dev_dbg(info->dev, "time of empty battery is %d minutes\n", info->time_to_empty);
		}
		break;

	 case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
		if (info->soca->ready_fg) {
			ret = get_time_to_full(info);
			val->intval = ret;
			info->time_to_full = ret/60;
			dev_dbg(info->dev,
				 "time of full battery is %d minutes\n",
							 info->time_to_full);
		} else {
			//val->intval = -EINVAL;
			val->intval = info->time_to_full * 60;
			dev_dbg(info->dev, "time of full battery is %d minutes\n", info->time_to_full);
		}
		break;
#endif
	 case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		ret = 0;
		break;

	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = POWER_SUPPLY_HEALTH_GOOD;
		ret = 0;
		break;

	default:
		mutex_unlock(&info->lock);
		return -ENODEV;
	}

	mutex_unlock(&info->lock);

	return ret;
}

static enum power_supply_property ricoh61x_batt_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
	POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
#endif
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_HEALTH,
};

static enum power_supply_property ricoh61x_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

struct power_supply	powerac = {
		.name = "acpwr",
		.type = POWER_SUPPLY_TYPE_MAINS,
		.properties = ricoh61x_power_props,
		.num_properties = ARRAY_SIZE(ricoh61x_power_props),
		.get_property = ricoh61x_batt_get_prop,
};

struct power_supply	powerusb = {
		.name = "usbpwr",
		.type = POWER_SUPPLY_TYPE_USB,
		.properties = ricoh61x_power_props,
		.num_properties = ARRAY_SIZE(ricoh61x_power_props),
		.get_property = ricoh61x_batt_get_prop,
};

static int ricoh61x_battery_probe(struct platform_device *pdev)
{
	struct ricoh61x_battery_info *info;
	struct ricoh619_battery_platform_data *pdata;
	int type_n;
	int ret, temp;

	pr_debug("PMU: %s : version is %s\n", __func__,RICOH61x_BATTERY_VERSION);

	info = kzalloc(sizeof(struct ricoh61x_battery_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->soca = kzalloc(sizeof(struct ricoh61x_soca_info), GFP_KERNEL);
		if (!info->soca)
			return -ENOMEM;

	info->dev = &pdev->dev;
	info->status = POWER_SUPPLY_STATUS_CHARGING;
	pdata = pdev->dev.platform_data;
	info->monitor_time = pdata->monitor_time * HZ;
	info->alarm_vol_mv = pdata->alarm_vol_mv;

	/* check rage of b,.attery type */
	type_n = Battery_Type();
//	printk("the Battery_Type is type[%d]\n",type_n);
	temp = sizeof(pdata->type)/(sizeof(struct ricoh619_battery_type_data));
	if(type_n  >= temp)
	{
		pr_debug("%s : Battery type num is out of range\n", __func__);
		type_n = 0;
	}
	pr_debug("%s type_n=%d,temp is %d\n", __func__, type_n,temp);

	/* check rage of battery num */
	info->num = Battery_Table();
	temp = sizeof(battery_init_para)/(sizeof(uint8_t)*32);
	if(info->num >= (sizeof(battery_init_para)/(sizeof(uint8_t)*32)))
	{
		pr_debug("%s : Battery num is out of range\n", __func__);
		info->num = 0;
	}
	pr_debug("%s info->num=%d,temp is %d\n", __func__, info->num,temp);


	/* these valuse are set in platform */
	info->ch_vfchg = pdata->type[type_n].ch_vfchg;
	info->ch_vrchg = pdata->type[type_n].ch_vrchg;
	info->ch_vbatovset = pdata->type[type_n].ch_vbatovset;
	info->ch_ichg = pdata->type[type_n].ch_ichg;
	info->ch_ilim_adp = pdata->type[type_n].ch_ilim_adp;
	info->ch_ilim_usb = pdata->type[type_n].ch_ilim_usb;
	info->ch_icchg = pdata->type[type_n].ch_icchg;
	if (pdata->type[type_n].fg_rsense_val ==0 ) info->fg_rsense_val = 20;
		else info->fg_rsense_val = pdata->type[type_n].fg_rsense_val;
	info->fg_target_vsys = pdata->type[type_n].fg_target_vsys;
	info->fg_target_ibat = pdata->type[type_n].fg_target_ibat * info->fg_rsense_val / 20;
	info->fg_poff_vbat = pdata->type[type_n].fg_poff_vbat;
	info->jt_en = pdata->type[type_n].jt_en;
	info->jt_hw_sw = pdata->type[type_n].jt_hw_sw;
	info->jt_temp_h = pdata->type[type_n].jt_temp_h;
	info->jt_temp_l = pdata->type[type_n].jt_temp_l;
	info->jt_vfchg_h = pdata->type[type_n].jt_vfchg_h;
	info->jt_vfchg_l = pdata->type[type_n].jt_vfchg_l;
	info->jt_ichg_h = pdata->type[type_n].jt_ichg_h;
	info->jt_ichg_l = pdata->type[type_n].jt_ichg_l;

	temp = get_OCV_init_Data(info, 11) * info->fg_rsense_val / 20;
	battery_init_para[info->num][22] = (temp >> 8);
	battery_init_para[info->num][23] = (temp & 0xff);
	temp = get_OCV_init_Data(info, 12) * 20 / info->fg_rsense_val;
	battery_init_para[info->num][24] = (temp >> 8);
	battery_init_para[info->num][25] = (temp & 0xff);

	/*
	pr_debug("%s setting value\n", __func__);
	pr_debug("%s info->ch_vfchg = 0x%x\n", __func__, info->ch_vfchg);
	pr_debug("%s info->ch_vrchg = 0x%x\n", __func__, info->ch_vrchg);
	pr_debug("%s info->ch_vbatovset =0x%x\n", __func__, info->ch_vbatovset);
	pr_debug("%s info->ch_ichg = 0x%x\n", __func__, info->ch_ichg);
	pr_debug("%s info->ch_ilim_adp =0x%x \n", __func__, info->ch_ilim_adp);
	pr_debug("%s info->ch_ilim_usb = 0x%x\n", __func__, info->ch_ilim_usb);
	pr_debug("%s info->ch_icchg = 0x%x\n", __func__, info->ch_icchg);
	pr_debug("%s info->fg_target_vsys = 0x%x\n", __func__, info->fg_target_vsys);
	pr_debug("%s info->fg_target_ibat = 0x%x\n", __func__, info->fg_target_ibat);
	pr_debug("%s info->jt_en = 0x%x\n", __func__, info->jt_en);
	pr_debug("%s info->jt_hw_sw = 0x%x\n", __func__, info->jt_hw_sw);
	pr_debug("%s info->jt_temp_h = 0x%x\n", __func__, info->jt_temp_h);
	pr_debug("%s info->jt_temp_l = 0x%x\n", __func__, info->jt_temp_l);
	pr_debug("%s info->jt_vfchg_h = 0x%x\n", __func__, info->jt_vfchg_h);
	pr_debug("%s info->jt_vfchg_l = 0x%x\n", __func__, info->jt_vfchg_l);
	pr_debug("%s info->jt_ichg_h = 0x%x\n", __func__, info->jt_ichg_h);
	pr_debug("%s info->jt_ichg_l = 0x%x\n", __func__, info->jt_ichg_l);
	*/

	info->adc_vdd_mv = ADC_VDD_MV;		/* 2800; */
	info->min_voltage = MIN_VOLTAGE;	/* 3100; */
	info->max_voltage = MAX_VOLTAGE;	/* 4200; */
	info->delay = 500;
	info->entry_factory_mode = false;

	mutex_init(&info->lock);
	platform_set_drvdata(pdev, info);

	info->battery.name = "battery";
	info->battery.type = POWER_SUPPLY_TYPE_BATTERY;
	info->battery.properties = ricoh61x_batt_props;
	info->battery.num_properties = ARRAY_SIZE(ricoh61x_batt_props);
	info->battery.get_property = ricoh61x_batt_get_prop;
	info->battery.set_property = NULL;
	info->battery.external_power_changed
		 = ricoh61x_external_power_changed;

	/* Disable Charger/ADC interrupt */
	ret = ricoh61x_clr_bits(info->dev->parent, RICOH61x_INTC_INTEN,
							 CHG_INT | ADC_INT);
	if (ret)
		goto out;

	ret = ricoh61x_init_battery(info);
	if (ret)
		goto out;

#ifdef ENABLE_FACTORY_MODE
	info->factory_mode_wqueue
		= create_singlethread_workqueue("ricoh61x_factory_mode");
	INIT_DEFERRABLE_WORK(&info->factory_mode_work,
					 check_charging_state_work);

	ret = ricoh61x_factory_mode(info);
	if (ret)
		goto out;

#endif

	ret = power_supply_register(&pdev->dev, &info->battery);

	if (ret)
		info->battery.dev->parent = &pdev->dev;

	ret = power_supply_register(&pdev->dev, &powerac);
	ret = power_supply_register(&pdev->dev, &powerusb);

	info->monitor_wqueue
		= create_singlethread_workqueue("ricoh61x_battery_monitor");
	info->workqueue = create_singlethread_workqueue("r5t61x_charger_in");
	INIT_WORK(&info->irq_work, charger_irq_work);
	INIT_DEFERRABLE_WORK(&info->monitor_work,
					 ricoh61x_battery_work);
	INIT_DEFERRABLE_WORK(&info->displayed_work,
					 ricoh61x_displayed_work);
	INIT_DEFERRABLE_WORK(&info->charge_stable_work,
					 ricoh61x_stable_charge_countdown_work);
	INIT_DEFERRABLE_WORK(&info->charge_monitor_work,
					 ricoh61x_charge_monitor_work);
	INIT_DEFERRABLE_WORK(&info->get_charge_work,
					 ricoh61x_get_charge_work);
	INIT_DEFERRABLE_WORK(&info->jeita_work, ricoh61x_jeita_work);
	INIT_DELAYED_WORK(&info->changed_work, ricoh61x_changed_work);

	/* Charger IRQ workqueue settings */
	charger_irq = pdata->irq;


	ret = request_threaded_irq(charger_irq + RICOH61x_IRQ_FONCHGINT,
					NULL, charger_in_isr, IRQF_ONESHOT,
						"r5t61x_charger_in", info);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't get CHG_INT IRQ for chrager: %d\n",
									ret);
		goto out;
	}

	ret = request_threaded_irq(charger_irq + RICOH61x_IRQ_FCHGCMPINT,
						NULL, charger_complete_isr,
					IRQF_ONESHOT, "r5t61x_charger_comp",
								info);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't get CHG_COMP IRQ for chrager: %d\n",
									 ret);
		goto out;
	}

	ret = request_threaded_irq(charger_irq + RICOH61x_IRQ_FVUSBDETSINT,
					NULL, charger_usb_isr, IRQF_ONESHOT,
						"r5t61x_usb_det", info);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't get USB_DET IRQ for chrager: %d\n",
									 ret);
		goto out;
	}

	ret = request_threaded_irq(charger_irq + RICOH61x_IRQ_FVADPDETSINT,
					NULL, charger_adp_isr, IRQF_ONESHOT,
						"r5t61x_adp_det", info);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"Can't get ADP_DET IRQ for chrager: %d\n", ret);
		goto out;
	}


#ifdef ENABLE_LOW_BATTERY_DETECTION
	ret = request_threaded_irq(charger_irq + RICOH61x_IRQ_VSYSLIR,
					NULL, adc_vsysl_isr, IRQF_ONESHOT,
						"r5t61x_adc_vsysl", info);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"Can't get ADC_VSYSL IRQ for chrager: %d\n", ret);
		goto out;
	}
	INIT_DEFERRABLE_WORK(&info->low_battery_work,
					 low_battery_irq_work);
#endif

	/* Charger init and IRQ setting */
	ret = ricoh61x_init_charger(info);
	if (ret)
		goto out;

#ifdef	ENABLE_FUEL_GAUGE_FUNCTION
	ret = ricoh61x_init_fgsoca(info);
#endif
	queue_delayed_work(info->monitor_wqueue, &info->monitor_work,
					RICOH61x_MONITOR_START_TIME*HZ);


	/* Enable Charger/ADC interrupt */
	ricoh61x_set_bits(info->dev->parent, RICOH61x_INTC_INTEN, CHG_INT | ADC_INT);

	return 0;

out:
	kfree(info);
	return ret;
}

static int ricoh61x_battery_remove(struct platform_device *pdev)
{
	struct ricoh61x_battery_info *info = platform_get_drvdata(pdev);
	uint8_t val;
	int ret;
	int err;
	int cc_cap = 0;
	bool is_charging = true;

	if (g_fg_on_mode
		 && (info->soca->status == RICOH61x_SOCA_STABLE)) {
		err = ricoh61x_write(info->dev->parent, PSWR_REG, 0x7f);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");
		g_soc = 0x7f;
	} else if (info->soca->status != RICOH61x_SOCA_START
		&& info->soca->status != RICOH61x_SOCA_UNSTABLE
		&& info->soca->rsoc_ready_flag != 0) {
		if (info->soca->displayed_soc < 50) {
			val = 1;
		} else {
			val = (info->soca->displayed_soc + 50)/100;
			val &= 0x7f;
		}
		ret = ricoh61x_write(info->dev->parent, PSWR_REG, val);
		if (ret < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");

		g_soc = val;

		ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, true);
		if (ret < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");
	}

	if (g_fg_on_mode == 0) {
		ret = ricoh61x_clr_bits(info->dev->parent,
					 FG_CTRL_REG, 0x01);
		if (ret < 0)
			dev_err(info->dev, "Error in clr FG EN\n");
	}

	/* set rapid timer 300 min */
	err = ricoh61x_set_bits(info->dev->parent, TIMSET_REG, 0x03);
	if (err < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
	}

	free_irq(charger_irq + RICOH61x_IRQ_FONCHGINT, &info);
	free_irq(charger_irq + RICOH61x_IRQ_FCHGCMPINT, &info);
	free_irq(charger_irq + RICOH61x_IRQ_FVUSBDETSINT, &info);
	free_irq(charger_irq + RICOH61x_IRQ_FVADPDETSINT, &info);
#ifdef ENABLE_LOW_BATTERY_DETECTION
	free_irq(charger_irq + RICOH61x_IRQ_VSYSLIR, &info);
#endif

	cancel_delayed_work(&info->monitor_work);
	cancel_delayed_work(&info->charge_stable_work);
	cancel_delayed_work(&info->charge_monitor_work);
	cancel_delayed_work(&info->get_charge_work);
	cancel_delayed_work(&info->displayed_work);
	cancel_delayed_work(&info->changed_work);
#ifdef ENABLE_LOW_BATTERY_DETECTION
	cancel_delayed_work(&info->low_battery_work);
#endif
#ifdef ENABLE_FACTORY_MODE
	cancel_delayed_work(&info->factory_mode_work);
#endif
	cancel_delayed_work(&info->jeita_work);
	cancel_work_sync(&info->irq_work);

	flush_workqueue(info->monitor_wqueue);
	flush_workqueue(info->workqueue);
#ifdef ENABLE_FACTORY_MODE
	flush_workqueue(info->factory_mode_wqueue);
#endif

	destroy_workqueue(info->monitor_wqueue);
	destroy_workqueue(info->workqueue);
#ifdef ENABLE_FACTORY_MODE
	destroy_workqueue(info->factory_mode_wqueue);
#endif

	power_supply_unregister(&info->battery);
	kfree(info);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

#ifdef CONFIG_PM
static int ricoh61x_battery_suspend(struct device *dev)
{
	struct ricoh61x_battery_info *info = dev_get_drvdata(dev);
	uint8_t val;
	int ret;
	int err;
	int cc_cap = 0;
	bool is_charging = true;
	int displayed_soc_temp;

#ifdef ENABLE_MASKING_INTERRUPT_IN_SLEEP
	ricoh61x_clr_bits(dev->parent, RICOH61x_INTC_INTEN, CHG_INT);
#endif

	if (g_fg_on_mode
		 && (info->soca->status == RICOH61x_SOCA_STABLE)) {
		err = ricoh61x_write(info->dev->parent, PSWR_REG, 0x7f);
		if (err < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");
		 g_soc = 0x7F;
		info->soca->suspend_soc = info->soca->displayed_soc;
		ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, true);
		if (ret < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

	} else if (info->soca->status != RICOH61x_SOCA_START
		&& info->soca->status != RICOH61x_SOCA_UNSTABLE
		&& info->soca->rsoc_ready_flag != 0) {
		if (info->soca->displayed_soc < 50) {
			val = 1;
		} else {
			val = (info->soca->displayed_soc + 50)/100;
			val &= 0x7f;
		}
		ret = ricoh61x_write(info->dev->parent, PSWR_REG, val);
		if (ret < 0)
			dev_err(info->dev, "Error in writing PSWR_REG\n");

		g_soc = val;

		ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, true);
		if (ret < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

		if (info->soca->status != RICOH61x_SOCA_STABLE) {
			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			displayed_soc_temp
			       = info->soca->displayed_soc + info->soca->cc_delta;
			displayed_soc_temp = min(10000, displayed_soc_temp);
			displayed_soc_temp = max(0, displayed_soc_temp);
			info->soca->displayed_soc = displayed_soc_temp;
		}
		info->soca->suspend_soc = info->soca->displayed_soc;

	} else if ((info->soca->status == RICOH61x_SOCA_START)
		|| (info->soca->status == RICOH61x_SOCA_UNSTABLE)
		|| (info->soca->rsoc_ready_flag == 0)) {

		ret = ricoh61x_read(info->dev->parent, PSWR_REG, &val);
		if (ret < 0)
			dev_err(info->dev, "Error in reading the pswr register\n");
		val &= 0x7f;

		info->soca->suspend_soc = val * 100;
	}

	if (info->soca->status == RICOH61x_SOCA_DISP
		|| info->soca->status == RICOH61x_SOCA_STABLE
		|| info->soca->status == RICOH61x_SOCA_FULL) {
		info->soca->soc = calc_capacity_2(info);
		info->soca->soc_delta =
			info->soca->soc_delta + (info->soca->soc - info->soca->last_soc);

	} else {
		info->soca->soc_delta = 0;
	}

	if (info->soca->status == RICOH61x_SOCA_STABLE
		|| info->soca->status == RICOH61x_SOCA_FULL)
		info->soca->status = RICOH61x_SOCA_DISP;
	/* set rapid timer 300 min */
	err = ricoh61x_set_bits(info->dev->parent, TIMSET_REG, 0x03);
	if (err < 0) {
		dev_err(info->dev, "Error in writing the control register\n");
	}

	cancel_delayed_work_sync(&info->monitor_work);
	cancel_delayed_work_sync(&info->displayed_work);
	cancel_delayed_work_sync(&info->charge_stable_work);
	cancel_delayed_work_sync(&info->charge_monitor_work);
	cancel_delayed_work_sync(&info->get_charge_work);
	cancel_delayed_work_sync(&info->changed_work);
#ifdef ENABLE_LOW_BATTERY_DETECTION
	cancel_delayed_work_sync(&info->low_battery_work);
#endif
#ifdef ENABLE_FACTORY_MODE
	cancel_delayed_work_sync(&info->factory_mode_work);
#endif
	cancel_delayed_work_sync(&info->jeita_work);
/*	flush_work(&info->irq_work); */


	return 0;
}

static int ricoh61x_battery_resume(struct device *dev)
{
	struct ricoh61x_battery_info *info = dev_get_drvdata(dev);
	uint8_t val;
	int ret;
	int displayed_soc_temp;
	int cc_cap = 0;
	bool is_charging = true;
	bool is_jeita_updated;
	int i;

#ifdef ENABLE_MASKING_INTERRUPT_IN_SLEEP
	ricoh61x_set_bits(dev->parent, RICOH61x_INTC_INTEN, CHG_INT);
#endif
	ret = check_jeita_status(info, &is_jeita_updated);
	if (ret < 0) {
		dev_err(info->dev, "Error in updating JEITA %d\n", ret);
	}

	if (info->entry_factory_mode) {
		info->soca->displayed_soc = -EINVAL;
	} else if (RICOH61x_SOCA_ZERO == info->soca->status) {
		if (calc_ocv(info) > get_OCV_voltage(info, 0)) {
			ret = ricoh61x_read(info->dev->parent, PSWR_REG, &val);
			val &= 0x7f;
			info->soca->soc = val * 100;
			if (ret < 0) {
				dev_err(info->dev,
					 "Error in reading PSWR_REG %d\n", ret);
				info->soca->soc
					 = calc_capacity(info) * 100;
			}

			ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, true);
			if (ret < 0)
				dev_err(info->dev, "Read cc_sum Error !!-----\n");

			info->soca->cc_delta
				 = (is_charging == true) ? cc_cap : -cc_cap;

			displayed_soc_temp
				 = info->soca->soc + info->soca->cc_delta;
			if (displayed_soc_temp < 0)
				displayed_soc_temp = 0;
			displayed_soc_temp = min(10000, displayed_soc_temp);
			displayed_soc_temp = max(0, displayed_soc_temp);
			info->soca->displayed_soc = displayed_soc_temp;

			ret = reset_FG_process(info);

			if (ret < 0)
				dev_err(info->dev, "Error in writing the control register\n");
			info->soca->status = RICOH61x_SOCA_FG_RESET;

		} else
			info->soca->displayed_soc = 0;
	} else {
		info->soca->soc = info->soca->suspend_soc;

		if (RICOH61x_SOCA_START == info->soca->status
			|| RICOH61x_SOCA_UNSTABLE == info->soca->status
			|| info->soca->rsoc_ready_flag == 0) {
			ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, false);
		} else {
			ret = calc_capacity_in_period(info, &cc_cap,
							 &is_charging, true);
		}

		if (ret < 0)
			dev_err(info->dev, "Read cc_sum Error !!-----\n");

		info->soca->cc_delta = (is_charging == true) ? cc_cap : -cc_cap;

		displayed_soc_temp = info->soca->soc + info->soca->cc_delta;
		if (info->soca->zero_flg == 1) {
			if((info->soca->Ibat_ave >= 0)
			|| (displayed_soc_temp >= 100)){
				info->soca->zero_flg = 0;
			} else {
				displayed_soc_temp = 0;
			}
		} else if (displayed_soc_temp < 100) {
			/* keep DSOC = 1 when Vbat is over 3.4V*/
			if( info->fg_poff_vbat != 0) {
				if (info->soca->Vbat_ave < 2000*1000) { /* error value */
					displayed_soc_temp = 100;
				} else if (info->soca->Vbat_ave < info->fg_poff_vbat*1000) {
					displayed_soc_temp = 0;
					info->soca->zero_flg = 1;
				} else {
					displayed_soc_temp = 100;
				}
			}
		}
		displayed_soc_temp = min(10000, displayed_soc_temp);
		displayed_soc_temp = max(0, displayed_soc_temp);

		if (0 == info->soca->jt_limit) {
			check_charge_status_2(info, displayed_soc_temp);
		} else {
			info->soca->displayed_soc = displayed_soc_temp;
		}

		if (RICOH61x_SOCA_DISP == info->soca->status) {
			info->soca->last_soc = calc_capacity_2(info);
		}
	}

	ret = measure_vbatt_FG(info, &info->soca->Vbat_ave);
	ret = measure_vsys_ADC(info, &info->soca->Vsys_ave);
	ret = measure_Ibatt_FG(info, &info->soca->Ibat_ave);

	power_supply_changed(&info->battery);
	queue_delayed_work(info->monitor_wqueue, &info->displayed_work, HZ);

	if (RICOH61x_SOCA_UNSTABLE == info->soca->status) {
		info->soca->stable_count = 10;
		queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 RICOH61x_FG_STABLE_TIME*HZ/10);
	} else if (RICOH61x_SOCA_FG_RESET == info->soca->status) {
		info->soca->stable_count = 1;

		for (i = 0; i < 3; i = i+1)
			info->soca->reset_soc[i] = 0;
		info->soca->reset_count = 0;

		queue_delayed_work(info->monitor_wqueue,
					 &info->charge_stable_work,
					 RICOH61x_FG_RESET_TIME*HZ);
	}

	queue_delayed_work(info->monitor_wqueue, &info->monitor_work,
						 info->monitor_time);

	queue_delayed_work(info->monitor_wqueue, &info->charge_monitor_work,
					 RICOH61x_CHARGE_RESUME_TIME * HZ);

	info->soca->chg_count = 0;
	queue_delayed_work(info->monitor_wqueue, &info->get_charge_work,
					 RICOH61x_CHARGE_RESUME_TIME * HZ);
	if (info->jt_en) {
		if (!info->jt_hw_sw) {
			queue_delayed_work(info->monitor_wqueue, &info->jeita_work,
					 RICOH61x_JEITA_UPDATE_TIME * HZ);
		}
	}


	return 0;
}

static const struct dev_pm_ops ricoh61x_battery_pm_ops = {
	.suspend	= ricoh61x_battery_suspend,
	.resume		= ricoh61x_battery_resume,
};
#endif

static struct platform_driver ricoh61x_battery_driver = {
	.driver	= {
				.name	= "ricoh619-battery",
				.owner	= THIS_MODULE,
#ifdef CONFIG_PM
				.pm	= &ricoh61x_battery_pm_ops,
#endif
	},
	.probe	= ricoh61x_battery_probe,
	.remove	= ricoh61x_battery_remove,
};

static int __init ricoh61x_battery_init(void)
{
	return platform_driver_register(&ricoh61x_battery_driver);
}
module_init(ricoh61x_battery_init);

static void __exit ricoh61x_battery_exit(void)
{
	platform_driver_unregister(&ricoh61x_battery_driver);
}
module_exit(ricoh61x_battery_exit);

MODULE_DESCRIPTION("RICOH R5T619 Battery driver");
MODULE_ALIAS("platform:ricoh619-battery");
MODULE_LICENSE("GPL");
