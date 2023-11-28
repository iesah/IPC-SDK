/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  JZ4775 SoC NAND controller driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __MACH_JZ4775_NAND_H__
#define __MACH_JZ4775_NAND_H__

#include <linux/completion.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <soc/gpemc.h>

#define MAX_NUM_NAND_IF    7

struct jz4775_nand;

typedef enum {
	/* NAND_XFER_<Data path driver>_<R/B# indicator> */
	NAND_XFER_CPU_IRQ = 0,
	NAND_XFER_CPU_POLL,
	NAND_XFER_DMA_IRQ,
	NAND_XFER_DMA_POLL
} nand_xfer_type_t;

typedef enum {
	NAND_ECC_TYPE_HW = 0,
	NAND_ECC_TYPE_SW
} nand_ecc_type_t;

typedef enum {
	NAND_OUTPUT_NORMAL_DRIVER = 0,
	NAND_OUTPUT_UNDER_DRIVER1,
	NAND_OUTPUT_UNDER_DRIVER2,
	NAND_OUTPUT_OVER_DRIVER1,
	NAND_OUTPUT_OVER_DRIVER2,

	CAN_NOT_ADJUST_OUTPUT_STRENGTH,
} nand_output_driver_strength_t;

typedef enum {
	NAND_RB_DOWN_FULL_DRIVER = 0,
	NAND_RB_DOWN_THREE_QUARTER_DRIVER,
	NAND_RB_DOWN_ONE_HALF_DRIVER,
	NAND_RB_DOWN_ONE_QUARTER_DRIVER,

	CAN_NOT_ADJUST_RB_DOWN_STRENGTH
} nand_rb_down_driver_strength_t;

typedef struct {
	int bank;

	int busy_gpio;
	int busy_gpio_low_assert;

	int wp_gpio;                      /* -1 if does not exist */
	int wp_gpio_low_assert;

	gpemc_bank_t cs;
	int busy_irq;

	struct completion ready;
	unsigned int ready_timout_ms;

	unsigned int curr_command;
} nand_flash_if_t;

typedef struct {
	common_nand_timing_t common_nand_timing;
	toggle_nand_timing_t toggle_nand_timing;
} nand_timing_t;

typedef struct {
	const char *name;
	unsigned int nand_mfr_id;
	unsigned int nand_dev_id;

	bank_type_t type;

	struct {
		int data_size;
		int ecc_bits;
	} ecc_step;

	nand_timing_t nand_timing;

	nand_output_driver_strength_t output_strength;
	nand_rb_down_driver_strength_t rb_down_strength;

	struct {
		int timing_mode;
	} onfi_special;

	int (*nand_pre_init)(struct jz4775_nand *nand);
} nand_flash_info_t;

struct jz4775_nand_platform_data {
	struct mtd_partition *part_table; /* MTD partitions array */
	int num_part;                     /* number of partitions */

	nand_flash_if_t *nand_flash_if_table;
	int num_nand_flash_if;

	nand_flash_info_t *nand_flash_info_table;
	int num_nand_flash_info;

	nand_xfer_type_t xfer_type;  /* transfer type */

	nand_ecc_type_t ecc_type;

	int num_nand_flash;
	/* not NULL to override default built-in settings in driver */
	struct nand_flash_dev *nand_flash_table;

	int try_to_reloc_hot;
	int flash_bbt;
};

#define COMMON_NAND_CHIP_INFO(_NAME, _MFR_ID, _DEV_ID,	\
		_DATA_SIZE_PRE_ECC_STEP,	\
		_ECC_BITS_PRE_ECC_STEP,	\
		_ALL_TIMINGS_PLUS,	\
		_Tcls, _Tclh, _Tals, _Talh,	\
		_Tcs, _Tch, _Tds, _Tdh, _Twp,	\
		_Twh, _Twc, _Trc, _Tadl, _Tccs, _Trhw, _Twhr, _Twhr2,	\
		_Trp, _Trr,	_Tcwaw, _Twb, _Tww,	\
		_Trst, _Tfeat, _Tdcbsyr, _Tdcbsyr2, _TIMING_MODE, _BW,	\
		_OUTPUT_STRENGTH, _RB_DOWN_STRENGTH,	\
		_NAND_PRE_INIT)	\
		.name = (_NAME),	\
		.nand_mfr_id = (_MFR_ID),	\
		.nand_dev_id = (_DEV_ID),	\
		.type = BANK_TYPE_NAND,	\
		.ecc_step = {	\
			.data_size = (_DATA_SIZE_PRE_ECC_STEP),	\
			.ecc_bits = (_ECC_BITS_PRE_ECC_STEP),	\
		},	\
		.nand_timing = {	\
			.common_nand_timing = {	\
				.Tcls = (_Tcls),	\
				.Tclh = (_Tclh),	\
				.Tals = (_Tals),	\
				.Talh = (_Talh),	\
				.Tch = (_Tch),	\
				.Tds = (_Tds),	\
				.Tdh = (_Tdh),	\
				.Twp = (_Twp),	\
				.Twh = (_Twh),	\
				.Twc = (_Twc),	\
				.Trc = (_Trc),	\
				.Trhw = (_Trhw),	\
				.Trp = (_Trp),	\
					\
				.busy_wait_timing = {	\
					.Tcs = (_Tcs),	\
					.Tadl = (_Tadl),	\
					.Tccs = (_Tccs),	\
					.Trr = (_Trr),	\
					.Tcwaw = (_Tcwaw),	\
					.Twb = (_Twb),	\
					.Tww = (_Tww),	\
					.Trst = (_Trst),	\
					.Tfeat = (_Tfeat),	\
					.Tdcbsyr = (_Tdcbsyr),	\
					.Tdcbsyr2 = (_Tdcbsyr2),	\
					.Twhr = (_Twhr),	\
					.Twhr2 = (_Twhr2),	\
				},	\
					\
				.BW = (_BW),	\
				.all_timings_plus = (_ALL_TIMINGS_PLUS),	\
			},	\
		},	\
		\
		.output_strength = (_OUTPUT_STRENGTH),	\
		.rb_down_strength = (_RB_DOWN_STRENGTH),	\
		.onfi_special.timing_mode = (_TIMING_MODE),	\
		.nand_pre_init = (_NAND_PRE_INIT),

/* TODO: implement it */
#define TOGGLE_NAND_CHIP_INFO(TODO)

#define COMMON_NAND_INTERFACE(BANK,	\
		BUSY_GPIO, BUSY_GPIO_LOW_ASSERT,	\
		WP_GPIO, WP_GPIO_LOW_ASSERT)	\
		.bank = (BANK),	\
		.busy_gpio = (BUSY_GPIO),	\
		.busy_gpio_low_assert = (BUSY_GPIO_LOW_ASSERT),	\
		.wp_gpio = (WP_GPIO),	\
		.wp_gpio_low_assert = (WP_GPIO_LOW_ASSERT),	\
		.cs = {	\
			.bank_type = (BANK_TYPE_NAND),	\
		},	\

/* TODO: implement it */
#define TOGGLE_NAND_INTERFACE(TODO)

#define LP_OPTIONS NAND_SAMSUNG_LP_OPTIONS
#define LP_OPTIONS16 (LP_OPTIONS | NAND_BUSWIDTH_16)

extern int micron_nand_pre_init(struct jz4775_nand *nand);
extern int samsung_nand_pre_init(struct jz4775_nand *nand);

#endif
