/*
 * Linux/sound/oss/xb47XX/xb4770/ojz4770_codec.h
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4760_CODEC_H__
#define __JZ4760_CODEC_H__

#include <linux/switch.h>
#include <mach/jzsnd.h>
#include "../xb47xx_i2s.h"
#include <linux/bitops.h>

/* Standby */
#define STANDBY 		1

/* Power setting */
#define POWER_ON		0
#define POWER_OFF		1

/* File ops mode W & R */
#define REPLAY			1
#define RECORD			2

/*CRYSTAL
 Selection of the SYS_CLK frequency.
*/
#define CRYSTAL_12_MHz    0
#define CRYSTAL_13_MHz    1

/* Channels */
#define LEFT_CHANNEL		1
#define RIGHT_CHANNEL		2

#define MAX_RATE_COUNT		10

#define codec_read_reg(addr)  read_inter_codec_reg(addr)
#define codec_write_reg(addr,data) write_inter_codec_reg(addr,data)
#define codec_write_reg_bit(addr,bitval,mask_bit) write_inter_codec_reg_bit(addr,bitval,mask_bit)

/*
 * JZ4760 CODEC CODEC registers
 */
/*==========================================*/
#define CODEC_REG_SR		0x0
/*------------------*/
 #define SR_PON_ACK		7
 #define SR_IRQ_ACK		6
 #define SR_JACK		5

#define CODEC_REG_AICR_DAC	0x1
/*------------------*/
 #define AICR_DAC_ADWL		6 // 2 bits
   #define AICR_DAC_ADWL_MASK	0xc0
 #define AICR_DAC_SERIAL	1
 #define AICR_DAC_I2S		0

#define CODEC_REG_AICR_ADC	0x2
/*------------------*/
 #define AICR_ADC_ADWL		6 // 2 bits
   #define AICR_ADC_ADWL_MASK	0xc0
 #define AICR_ADC_SERIAL	1
 #define AICR_ADC_I2S		0

#define CODEC_REG_CR_LO 	0x3
/*------------------*/
 #define CR_LO_MUTE		7
 #define CR_SB_LO		4
 #define CR_LO_SEL		0 // 2 bits
   #define LO_SEL_MASK		0x3

#define CODEC_REG_CR_HP 	0x4
/*------------------*/
 #define CR_HP_MUTE		7
 #define CR_HP_LOAD		6
 #define CR_SB_HP		4
 #define CR_SB_HPCM		3
 #define CR_HP_SEL		0 // 2 bits
   #define HP_SEL_MASK		0x3

#define CODEC_REG_CR_DAC		0x6
/*------------------*/
 #define CR_DAC_MUTE		7
 #define CR_DAC_MONO		6
 #define CR_DAC_LEFT_ONLY	5
 #define CR_SB_DAC		4
 #define CR_DAC_LRSWAP		3

#define CODEC_REG_CR_MIC		7
/*------------------*/
 #define CR_MIC_STEREO		0x7
 #define CR_MICIDIFF		6
 #define CR_SB_MIC2		5
 #define CR_SB_MIC1		4
 #define CR_MICBIAS_V0		1
 #define SB_MICBIAS		0

#define CODEC_REG_CR_LI 	0x8
/*------------------*/
 #define CR_SB_LIBY		4
 #define CR_SB_LIN		0

#define CODEC_REG_CR_ADC		0x9
/*------------------*/
 #define CR_DMIC_SEL		7
 #define CR_ADC_MONO		6
 #define CR_ADC_LEFT_ONLY	5
 #define CR_SB_ADC		4
 #define CR_ADC_LRSWAP		3
 #define CR_IN_SEL		0 // 2 bits
   #define IN_SEL_MASK		0x3

#define CODEC_REG_CR_MIX		0xA
/*------------------*/
 #define CR_MIX_REC		2 // 2 bits
   #define MIX_REC_MASK 	0xc
 #define CR_DAC_MIX		0 // 2 bits
   #define DAC_MIX_MASK 	0x3

#define CODEC_REG_CR_VIC		0xB
/*------------------*/
 #define CR_SB_SLEEP		1
 #define CR_SB			0

#define CODEC_REG_CCR		0xC
/*------------------*/
 #define CCR_DMIC_CLKON 	7
 #define CCR_CRYSTAL		0 // 4 bits
   #define CRYSTAL_MASK 	0xf

#define CODEC_REG_FCR_DAC		0xD
/*------------------*/
 #define FCR_DAC_FREQ		0 // 4 bits
   #define FCR_DAC_FREQ_MASK	0xf

#define CODEC_REG_FCR_ADC		0xE
/*------------------*/
 #define FCR_ADC_HPF		6
 #define FCR_ADC_FREQ		0 // 4 bits
   #define FCR_ADC_FREQ_MASK	0xf

#define CODEC_REG_ICR		0xF
/*------------------*/
 #define ICR_INT_FORM		6 // 2 bits
   #define INT_FORM_MASK	0x3

#define CODEC_REG_IMR		0x10
/*------------------*/
   #define REG_IMR_MASK 	0x7f
 #define IMR_SCLR_MASK		(1 << 6)
 #define IMR_JACK_MASK		(1 << 5)
 #define IMR_SCMC_MASK		(1 << 4)
 #define IMR_RUP_MASK		(1 << 3)
 #define IMR_RDO_MASK		(1 << 2)
 #define IMR_GUP_MASK		(1 << 1)
 #define IMR_GDO_MASK		(1 << 0)

#define CODEC_REG_IFR		0x11
/*------------------*/
   #define REG_IFR_MASK 	0x7f
 #define IFR_SCLR		6
 #define IFR_JACK_EVENT 	5
 #define IFR_SCMC		4
 #define IFR_RUP		3
 #define IFR_RDO		2
 #define IFR_GUP		1
 #define IFR_GDO		0

#define CODEC_REG_GCR_HPL		0x12
/*------------------*/
 #define GCR_HPL_LRGO		7
 #define GCR_HPL_GOL		0 // 5 bits
   #define HPL_GOL_MASK 	0x1f

#define CODEC_REG_GCR_HPR		0x13
/*------------------*/
 #define GCR_HPR_GOR		0 // 5 bits
   #define HPR_GOR_MASK 	0x1f

#define CODEC_REG_GCR_LIBYL	0x14
/*------------------*/
 #define GCR_LIBYL_LRGI 	7
 #define GCR_LIBYL_GIL		0 // 5 bits
   #define LIBYL_GIL_MASK	0x1f

#define CODEC_REG_GCR_LIBYR	0x15
/*------------------*/
 #define GCR_LIBYR_GIR		0 // 5 bits
   #define LIBYR_GIR_MASK	0x1f

#define CODEC_REG_GCR_DACL	0x16
/*------------------*/
 #define GCR_DACL_RLGOD 	7
 #define GCR_DACL_HCPDIS	6
 #define GCR_DACL_GODL		0 // 6 bits
   #define DACL_GODL_MASK	0x3f

#define CODEC_REG_GCR_DACR	0x17
/*------------------*/
 #define GCR_DACR_GODR		0 // 6 bits
   #define DACR_GODR_MASK	0x3f

#define CODEC_REG_GCR_MIC1	0x18
/*------------------*/
 #define GCR_MIC1_GIM1		0 // 3 bits
   #define MIC1_GIM1_MASK	0x7

#define CODEC_REG_GCR_MIC2	0x19
/*------------------*/
 #define GCR_MIC2_GIM2		0 // 3 bits
   #define MIX2_GIM2_MASK	0x7

#define CODEC_REG_GCR_ADCL	0x1A
/*------------------*/
 #define GCR_ADCL_LRGID 	7
 #define GCR_ADCL_GIDL		0 // 6 bits
   #define ADCL_GIDL_MASK	0x3f

#define CODEC_REG_GCR_ADCR	0x1B
/*------------------*/
 #define GCR_ADCR_GIDR		0 // 6 bits
   #define ADCR_GIDR_MASK	0x3f

#define CODEC_REG_GCR_MIXADC	0x1D
/*------------------*/
 #define GCR_MIXADC_GIMIX	0 // 5 bits
   #define MIXADC_GIMIX_MASK	0x1f

#define CODEC_REG_GCR_MIXDAC	0x1E
/*------------------*/
 #define GCR_MIXDAC_GOMIX	0 // 5 bits
   #define MIXDAC_GOMIX_MASK	0x1f

#define CODEC_REG_AGC1		0x1F
/*------------------*/
 #define AGC1_AGC_EN		7
 #define AGC1_AGC_STEREO	6
 #define AGC1_TAEGET		2 // 4 bits

#define CODEC_REG_AGC2		0x20
/*------------------*/
 #define AGC2_NG_EN		7
 #define AGC2_NG_THR		4 // 3 bits
 #define AGC2_HOLD		0 // 3 bits

#define CODEC_REG_AGC3		0x21
/*------------------*/
 #define AGC3_ATK		4 // 4 bits
 #define AGC3_DCY		0 // 4 bits

#define CODEC_REG_AGC4		0x22
/*------------------*/
 #define AGC4_AGC_MAX		0 // 5 bits

#define CODEC_REG_AGC5		0x23
/*------------------*/
 #define AGC5_AGC_MIN		0 // 5 bits
/*==========================================*/

#define ICR_ALL_MASK		(IMR_GDO_MASK | IMR_GUP_MASK | IMR_RDO_MASK | IMR_RUP_MASK | IMR_SCMC_MASK | IMR_JACK_MASK | IMR_SCLR_MASK)

#ifdef CONFIG_JZ_HP_DETECT_CODEC
 #define ICR_COMMON_MASK	(IMR_GDO_MASK | IMR_GUP_MASK | IMR_RDO_MASK | IMR_RUP_MASK)
#else
 #define ICR_COMMON_MASK	(IMR_GDO_MASK | IMR_GUP_MASK | IMR_RDO_MASK | IMR_RUP_MASK | IMR_JACK_MASK)
#endif

/*
 * Ops
 */
/* misc ops*/

#define CODEC_PON_ACK_MASK	BIT(SR_PON_ACK)
#define CODEC_IRQ_ACK_MASK	BIT(SR_IRQ_ACK)
#define CODEC_JACK_MASK 	BIT(SR_JACK)

#define __codec_get_sr()	codec_read_reg(CODEC_REG_SR)
/*============================== ADC/DAC ==============================*/

/* adc/dac interface */
#define PARALLEL_INTERFACE	0
#define SERIAL_INTERFACE	1

#define __codec_select_adc_digital_interface(mode)				\
	do {									\
		codec_write_reg_bit(CODEC_REG_AICR_ADC, mode, AICR_ADC_SERIAL); \
										\
	} while (0)

#define __codec_select_dac_digital_interface(mode)				\
	do {									\
		codec_write_reg_bit(CODEC_REG_AICR_DAC, mode, AICR_DAC_SERIAL); \
										\
	} while (0)


/* adc/dac working mode */
#define DSP_MODE		0
#define I2S_MODE		1

#define __codec_select_adc_work_mode(mode)					\
	do {									\
		codec_write_reg_bit(CODEC_REG_AICR_ADC, mode, AICR_ADC_I2S);	\
										\
	} while (0)

#define __codec_select_dac_work_mode(mode)					\
	do {									\
		codec_write_reg_bit(CODEC_REG_AICR_DAC, mode, AICR_DAC_I2S);	\
										\
	} while (0)


/* set adc/dac data word length */
#define __codec_select_adc_word_length(width)						\
	do {										\
		codec_write_reg(CODEC_REG_AICR_ADC,						\
			      ((codec_read_reg(CODEC_REG_AICR_ADC) & ~AICR_ADC_ADWL_MASK) |	\
			       ((width << AICR_ADC_ADWL) & AICR_ADC_ADWL_MASK)));	\
											\
	} while (0)

#define __codec_select_dac_word_length(width)						\
	do {										\
		codec_write_reg(CODEC_REG_AICR_DAC,						\
			      ((codec_read_reg(CODEC_REG_AICR_DAC) & ~AICR_DAC_ADWL_MASK) |	\
			      ((width << AICR_DAC_ADWL) & AICR_DAC_ADWL_MASK)));	\
											\
	} while (0)

/* set adc/dac samplimg rate */
#define __codec_select_adc_samp_rate(val)							\
	do {										\
		codec_write_reg(CODEC_REG_FCR_ADC,						\
			      ((codec_read_reg(CODEC_REG_FCR_ADC) & ~FCR_ADC_FREQ_MASK) |	\
			       (val & FCR_ADC_FREQ_MASK)));				\
											\
	} while (0)

#define __codec_select_dac_samp_rate(val)							\
	do {										\
		codec_write_reg(CODEC_REG_FCR_DAC,						\
			      ((codec_read_reg(CODEC_REG_FCR_DAC) & ~FCR_DAC_FREQ_MASK) |	\
			      (val & FCR_DAC_FREQ_MASK)));				\
											\
	} while (0)

/*========================= SB switch =================================*/

#define __codec_get_sb()			((codec_read_reg(CODEC_REG_CR_VIC) &	\
					  (1 << CR_SB)) ?			\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb(pwrstat)					\
do {									\
	if (__codec_get_sb() != pwrstat) {				\
		codec_write_reg_bit(CODEC_REG_CR_VIC, pwrstat,		\
				  CR_SB);				\
	}								\
									\
} while (0)

#define __codec_get_sb_sleep()		((codec_read_reg(CODEC_REG_CR_VIC) &	\
					  (1 << CR_SB_SLEEP)) ? 		\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_sleep(pwrstat)					\
do {									\
	if (__codec_get_sb_sleep() != pwrstat) {				\
		codec_write_reg_bit(CODEC_REG_CR_VIC, pwrstat,		\
				  CR_SB_SLEEP); 			\
	}								\
									\
} while (0)

#define __codec_get_sb_dac()		((codec_read_reg(CODEC_REG_CR_DAC) &	\
					  (1 << CR_SB_DAC)) ?			\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_dac(pwrstat)					\
do {									\
	if (__codec_get_sb_dac() != pwrstat) {				\
		codec_write_reg_bit(CODEC_REG_CR_DAC, pwrstat,		\
				  CR_SB_DAC);				\
	}								\
									\
} while (0)

#define __codec_get_sb_line_out()		((codec_read_reg(CODEC_REG_CR_LO) &	\
					  (1 << CR_SB_LO)) ?		\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_line_out(pwrstat)				\
do {									\
	if (__codec_get_sb_line_out() != pwrstat) {			\
		codec_write_reg_bit(CODEC_REG_CR_LO, pwrstat,		\
				  CR_SB_LO);				\
	}								\
									\
} while (0)

#define __codec_get_sb_hp()		((codec_read_reg(CODEC_REG_CR_HP) &	\
					  (1 << CR_SB_HP)) ?		\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_hp(pwrstat)					\
do {									\
	if (__codec_get_sb_hp() != pwrstat) {				\
		codec_write_reg_bit(CODEC_REG_CR_HP, pwrstat,		\
				  CR_SB_HP);				\
	}								\
									\
} while (0)

#define __codec_get_sb_hpcm()		((codec_read_reg(CODEC_REG_CR_HP) &	\
					  (1 << CR_SB_HPCM)) ?		\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_hpcm(pwrstat) 				\
do {									\
	if (__codec_get_sb_hpcm() != pwrstat) { 			\
		codec_write_reg_bit(CODEC_REG_CR_HP, pwrstat,		\
				  CR_SB_HPCM);				\
	}								\
									\
} while (0)

#define __codec_get_sb_adc()		((codec_read_reg(CODEC_REG_CR_ADC) &	\
					  (1 << CR_SB_ADC)) ?			\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_adc(pwrstat)					\
do {									\
	if (__codec_get_sb_adc() != pwrstat) {				\
		codec_write_reg_bit(CODEC_REG_CR_ADC, pwrstat,		\
				  CR_SB_ADC);				\
	}								\
									\
} while (0)

#define __codec_get_sb_mic1()		((codec_read_reg(CODEC_REG_CR_MIC) &	\
					  (1 << CR_SB_MIC1)) ?			\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_mic1(pwrstat) 				\
do {									\
	if (__codec_get_sb_mic1() != pwrstat) { 			\
		codec_write_reg_bit(CODEC_REG_CR_MIC, pwrstat,		\
				  CR_SB_MIC1);				\
	}								\
									\
} while (0)

#define __codec_get_sb_mic2()		((codec_read_reg(CODEC_REG_CR_MIC) &	\
					  (1 << CR_SB_MIC2)) ?			\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_mic2(pwrstat) 				\
do {									\
	if (__codec_get_sb_mic2() != pwrstat) { 			\
		codec_write_reg_bit(CODEC_REG_CR_MIC, pwrstat,		\
				  CR_SB_MIC2);				\
	}								\
									\
} while (0)

#define FOUR_OF_SIX_VREF	1
#define FIVE_OF_SIX_VREF	0

#define __codec_get_micbias_v0()		((codec_read_reg(CODEC_REG_CR_MIC) &	\
					  (1 << CR_MICBIAS_V0)) ?		\
					 FIVE_OF_SIX_VREF : FIVE_OF_SIX_VREF)

#define __codec_set_micbias_v0(val)					\
do {									\
	if (__codec_get_micbias_v0() != val) {				\
		codec_write_reg_bit(CODEC_REG_CR_MIC, val,			\
				  CR_MICBIAS_V0);			\
	}								\
									\
} while (0)

#define __codec_get_sb_micbias()		((codec_read_reg(CODEC_REG_CR_MIC) &	\
					  (1 << SB_MICBIAS)) ?			\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_micbias(pwrstat)				\
do {									\
	if (__codec_get_sb_micbias() != pwrstat) {			\
		codec_write_reg_bit(CODEC_REG_CR_MIC, pwrstat,		\
				  SB_MICBIAS);				\
	}								\
									\
} while (0)

#define __codec_get_sb_linein_to_adc()		((codec_read_reg(CODEC_REG_CR_LI) &	\
						  (1 << CR_SB_LIN)) ?		\
						 POWER_OFF : POWER_ON)

#define __codec_switch_sb_linein_to_adc(pwrstat)				\
do {									\
	if (__codec_get_sb_linein_to_adc() != pwrstat) {			\
		codec_write_reg_bit(CODEC_REG_CR_LI, pwrstat,		\
				  CR_SB_LIN);				\
	}								\
									\
} while (0)

#define __codec_get_sb_linein_to_bypass()		((codec_read_reg(CODEC_REG_CR_LI) &	\
					  (1 << CR_SB_LIBY)) ?			\
					 POWER_OFF : POWER_ON)

#define __codec_switch_sb_linein_to_bypass(pwrstat)			\
do {									\
	if (__codec_get_sb_linein_to_bypass() != pwrstat) {		\
		codec_write_reg_bit(CODEC_REG_CR_LI, pwrstat,		\
				  CR_SB_LIBY);				\
	}								\
									\
} while (0)

/*============================== IRQ ==================================*/

#define ICR_INT_HIGH		0
#define ICR_INT_LOW		1
#define ICR_INT_HIGH_8CYCLES	2
#define ICR_INT_LOW_8CYCLES	3

#define __codec_set_int_form(opt)							\
do {										\
	codec_write_reg(CODEC_REG_ICR, ((opt & INT_FORM_MASK) << ICR_INT_FORM));	\
										\
} while (0)

#define __codec_set_irq_mask(mask)					\
do {									\
	codec_write_reg(CODEC_REG_IMR, mask & REG_IMR_MASK);		\
									\
} while (0)

#define __codec_set_irq_flag(flag)					\
do {									\
	codec_write_reg(CODEC_REG_IFR, flag);				\
									\
} while (0)

#define __codec_get_irq_flag()		(codec_read_reg(CODEC_REG_IFR) &	\
					 REG_IFR_MASK)

#define __codec_get_irq_mask()		(codec_read_reg(CODEC_REG_IMR) &	\
					 REG_IMR_MASK)


#define __codec_clear_rup()					  \
do {								\
	codec_write_reg_bit(CODEC_REG_IFR, 1, IFR_RUP); 	    \
								\
} while (0)

#define __codec_clear_gup()					  \
do {								\
	codec_write_reg_bit(CODEC_REG_IFR, 1, IFR_GUP); 	    \
								\
} while (0)

#define __codec_clear_gdo()					  \
do {								\
	codec_write_reg_bit(CODEC_REG_IFR, 1, IFR_GDO); 	    \
								\
} while (0)

#define __codec_clear_rdo()					  \
do {								\
	codec_write_reg_bit(CODEC_REG_IFR, 1, IFR_RDO); 	    \
								\
} while (0)

/*============================== LOAD =================================*/

#define LOAD_16OHM	0
#define LOAD_10KOHM	1

#define __codec_get_load()		((codec_read_reg(CODEC_REG_CR_HP) &	\
					  (1 << CR_HP_LOAD)) ?		\
					 LOAD_10KOHM : LOAD_16OHM)

#define __codec_set_16ohm_load()						\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_HP, LOAD_16OHM, CR_HP_LOAD);	\
									\
} while (0)

#define __codec_set_10kohm_load()						\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_HP, LOAD_10KOHM, CR_HP_LOAD);	\
									\
} while (0)

/*============================== MUTE =================================*/

#define MUTE_ENABLE		1
#define MUTE_DISABLE		0

#define __codec_get_hp_mute()	     ((codec_read_reg(CODEC_REG_CR_HP) &	\
				     (1 << CR_HP_MUTE)) ?		\
				    MUTE_ENABLE : MUTE_DISABLE)

#define __codec_enable_hp_mute()						\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_HP, MUTE_ENABLE, CR_HP_MUTE);	\
									\
} while (0)

#define __codec_disable_hp_mute()						\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_HP, MUTE_DISABLE, CR_HP_MUTE); \
									\
} while (0)

#define __codec_get_lineout_mute()	  ((codec_read_reg(CODEC_REG_CR_LO) &	\
					  (1 << CR1_LINEOUT_MUTE)) ?	\
					 MUTE_ENABLE : MUTE_DISABLE)

#define __codec_enable_lineout_mute()					\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_LO, MUTE_ENABLE, CR_LO_MUTE);	\
									\
} while (0)

#define __codec_disable_lineout_mute()					\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_LO, MUTE_DISABLE, CR_LO_MUTE); \
									\
} while (0)

#define __codec_get_dac_mute()	      ((codec_read_reg(CODEC_REG_CR_DAC) &	\
				      (1 << CR_DAC_MUTE)) ?		\
				     MUTE_ENABLE : MUTE_DISABLE)

#define __codec_enable_dac_mute()						\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_DAC, MUTE_ENABLE, CR_DAC_MUTE);	\
									\
} while (0)

#define __codec_disable_dac_mute()					\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_DAC, MUTE_DISABLE, CR_DAC_MUTE);	\
									\
} while (0)

/*============================== MISC =================================*/

#define MIC1_TO_LR		0 //00b
#define MIC2_TO_LR		1 //01b
#define MIC2_TO_R_MIC1_TO_L	0 //00b
#define MIC1_TO_R_MIC2_TO_L	1 //01b
#define BYPASS_PATH		2 //10b
#define DAC_OUTPUT		3 //11b

#define __codec_set_hp_sel(opt) 					\
do {									\
	codec_write_reg(CODEC_REG_CR_HP, ((codec_read_reg(CODEC_REG_CR_HP) &	\
				       ~HP_SEL_MASK) |			\
				      (opt & HP_SEL_MASK)));		\
									\
} while (0)

#define LO_SEL_MIC1		0 //00b
#define LO_SEL_MIC2		1 //01b
#define LO_SEL_MIC1_AND_MIC2	0 //00b

#define __codec_set_lineout_sel(opt)					\
do {									\
	codec_write_reg(CODEC_REG_CR_LO, ((codec_read_reg(CODEC_REG_CR_LO) &	\
				       ~LO_SEL_MASK) |			\
				      (opt & LO_SEL_MASK)));		\
									\
} while (0)

#define __codec_enable_dac_left_only()					\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_DAC, 1, CR_DAC_LEFT_ONLY);		\
									\
} while (0)

#define __codec_disable_dac_left_only() 				\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_DAC, 0, CR_DAC_LEFT_ONLY);		\
									\
} while (0)

#define LINE_INPUT		2 //10b

#define __codec_set_adc_insel(opt)					\
do {									\
	codec_write_reg(CODEC_REG_CR_ADC, ((codec_read_reg(CODEC_REG_CR_ADC) &	\
					~IN_SEL_MASK) | 		\
				       (opt & IN_SEL_MASK)));		\
									\
} while (0)

#define DMIC_SEL_ADC		0
#define DMIC_SEL_DIGITAL_MIC	1

#define __codec_set_dmic_insel(opt)					\
	do {								\
		codec_write_reg_bit(CODEC_REG_CR_ADC, opt, CR_DMIC_SEL);	\
									\
	} while (0)

#define __codec_set_adc_mono()						\
	do {								\
		codec_write_reg_bit(CODEC_REG_CR_ADC, 1, CR_ADC_MONO);	\
									\
	} while (0)

#define __codec_set_adc_stereo()						\
	do {								\
		codec_write_reg_bit(CODEC_REG_CR_ADC, 0, CR_ADC_MONO);	\
									\
	} while (0)

#define __codec_set_dac_mono()						\
	do {								\
		codec_write_reg_bit(CODEC_REG_CR_DAC, 1, CR_DAC_MONO);	\
									\
	} while (0)

#define __codec_set_dac_stereo()						\
	do {								\
		codec_write_reg_bit(CODEC_REG_CR_DAC, 0, CR_DAC_MONO);	\
									\
	} while (0)

#define LRSWAP_ENABLE		0
#define LRSWAP_DISABLE		1

#define __codec_set_adc_lrswap(opt)					\
	do {								\
		codec_write_reg_bit(CODEC_REG_CR_ADC, opt, CR_ADC_LRSWAP);	\
									\
	} while (0)

#define __codec_set_dac_lrswap(opt)					\
	do {								\
		codec_write_reg_bit(CODEC_REG_CR_DAC, opt, CR_DAC_LRSWAP);	\
									\
	} while (0)

#define HCPDIS_ENABLE	0
#define HCPDIS_DISABLE	1

#define __codec_set_dac_hcpdis(opt)					\
	do {									\
		codec_write_reg_bit(CODEC_REG_GCR_DACL,opt,GCR_DACL_HCPDIS);	\
	} while (0)

#define MIC_STEREO		1
#define MIC_MONO		0

#define __codec_set_mic_stereo()						\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_MIC, MIC_STEREO, CR_MIC_STEREO);	\
									\
} while (0)

#define __codec_set_mic_mono()						\
do {									\
	codec_write_reg_bit(CODEC_REG_CR_MIC, MIC_MONO, CR_MIC_STEREO); \
									\
} while (0)

#define MIC_DIFF		1
#define MIC_SING		0

#define __codec_enable_mic_diff()						\
  do {									\
	codec_write_reg_bit(CODEC_REG_CR_MIC, MIC_DIFF, CR_MICIDIFF);	\
									\
} while (0)

#define __codec_disable_mic_diff()					\
  do {									\
	  codec_write_reg_bit(CODEC_REG_CR_MIC, MIC_SING, CR_MICIDIFF); \
									\
} while (0)

#define ADC_HPF_ENABLE		1
#define ADC_HPF_DISABLE 	0

#define __codec_enable_adc_high_pass()						\
  do {										\
	  codec_write_reg_bit(CODEC_REG_FCR_ADC, ADC_HPF_ENABLE, FCR_ADC_HPF);	\
										\
  } while (0)

#define __codec_disable_adc_high_pass() 					\
  do {										\
	  codec_write_reg_bit(CODEC_REG_FCR_ADC, ADC_HPF_DISABLE, FCR_ADC_HPF); 	\
										\
  } while (0)

#define __codec_enable_adc_left_only()				\
do {								\
	codec_write_reg_bit(CODEC_REG_CR_ADC, 1, CR_ADC_LEFT_ONLY);	\
								\
} while (0)

#define __codec_disable_adc_left_only() 			\
do {								\
	codec_write_reg_bit(CODEC_REG_CR_ADC, 0, CR_ADC_LEFT_ONLY);	\
								\
} while (0)

#define __codec_enable_agc()					\
do {								\
	codec_write_reg_bit(CODEC_REG_AGC1, 1, AGC1_AGC_EN);	\
								\
} while (0)

#define __codec_disable_agc()					\
do {								\
	codec_write_reg_bit(CODEC_REG_AGC1, 0, AGC1_AGC_EN);	\
								\
} while (0)

#define SYS_CLK_12M		0
#define SYS_CLK_13M		1

#define __codec_set_crystal(opt)									\
	do {											\
		codec_write_reg(CODEC_REG_CCR, 0x80 | (codec_read_reg(CODEC_REG_CCR) & ~CRYSTAL_MASK) |	\
				(opt & CRYSTAL_MASK));						\
												\
	  } while (0)

#define DMIC_CLK_ON		1
#define DMIC_CLK_OFF		0

#define __codec_get_dmic_clock() codec_read_reg(CODEC_REG_CCR)

#define __codec_set_dmic_clock(opt)					\
	do {								\
		codec_write_reg_bit(CODEC_REG_CCR, opt, CCR_DMIC_CLKON);	\
									\
	} while (0)

/*=============================== MIXER ===============================*/

#define MIX1_RECORD_INPUT_ONLY	   0
#define MIX1_RECORD_INPUT_AND_DAC  1

#define __codec_set_rec_mix_mode(mode)							\
	  do {										\
		  codec_write_reg(CODEC_REG_CR_MIX,						\
				((codec_read_reg(CODEC_REG_CR_MIX) & ~MIX_REC_MASK) |	\
				 ((mode << CR_MIX_REC) & MIX_REC_MASK)));		\
											\
	  } while (0)

#define MIX2_PLAYBACK_DAC_ONLY	   0
#define MIX2_PLAYBACK_DAC_AND_ADC  1

#define __codec_set_dac_mix_mode(mode)						\
  do {										\
	  codec_write_reg(CODEC_REG_CR_MIX,						\
			((codec_read_reg(CODEC_REG_CR_MIX) & ~DAC_MIX_MASK) |	\
			 ((mode << CR_DAC_MIX) & DAC_MIX_MASK)));		\
										\
  } while (0)

/*============================== GAIN =================================*/


#define __codec_set_gm1(value)							\
	  do {									\
		  codec_write_reg(CODEC_REG_GCR_MIC1, (value & MIC1_GIM1_MASK));	\
										\
	  } while (0)
#define __codec_get_gm1()	 ( codec_read_reg(CODEC_REG_GCR_MIC1) &  MIC1_GIM1_MASK )

#define __codec_set_gm2(value)							\
	  do {									\
		codec_write_reg(CODEC_REG_GCR_MIC2, (value & MIX2_GIM2_MASK));	\
										\
	  } while (0)
#define __codec_get_gm2()	 ( codec_read_reg(CODEC_REG_GCR_MIC2) &  MIX2_GIM2_MASK )

#define __codec_set_gil(value)							\
	do{									\
		codec_write_reg(CODEC_REG_GCR_LIBYL, (value & LIBYL_GIL_MASK)); \
										\
	} while (0)
#define __codec_get_gil() ( codec_read_reg(CODEC_REG_GCR_LIBYL) &  LIBYL_GIL_MASK )

#define __codec_set_gir(value)							\
	do{									\
		codec_write_reg(CODEC_REG_GCR_LIBYR, (value & LIBYR_GIR_MASK)); \
										\
	} while (0)
#define __codec_get_gir() ( codec_read_reg(CODEC_REG_GCR_LIBYR) &  LIBYR_GIR_MASK )

#define __codec_set_gol(value)						\
	do{								\
		codec_write_reg(CODEC_REG_GCR_HPL, (value & HPL_GOL_MASK));	\
									\
	} while (0)
#define __codec_get_gol() ( codec_read_reg(CODEC_REG_GCR_HPL) &  HPL_GOL_MASK )

#define __codec_set_gor(value)						\
	do{								\
		codec_write_reg(CODEC_REG_GCR_HPR, (value & HPR_GOR_MASK));	\
									\
	} while (0)
#define __codec_get_gor() ( codec_read_reg(CODEC_REG_GCR_HPR) &  HPR_GOR_MASK )

#define __codec_set_gidl(value) 						\
	do {									\
		codec_write_reg(CODEC_REG_GCR_ADCL, (value & ADCL_GIDL_MASK));	\
										\
	} while (0)
#define __codec_get_gidl() ( codec_read_reg(CODEC_REG_GCR_ADCL) &  ADCL_GIDL_MASK )

#define __codec_set_gidr(value) 						\
	do {									\
		codec_write_reg(CODEC_REG_GCR_ADCR, (value & ADCR_GIDR_MASK));	\
										\
	} while (0)
#define __codec_get_gidr() ( codec_read_reg(CODEC_REG_GCR_ADCR) &  ADCR_GIDR_MASK )

#define __codec_set_godl(value) 						\
	do {									\
		codec_write_reg(CODEC_REG_GCR_DACL, (value & DACL_GODL_MASK));	\
										\
	} while (0)
#define __codec_get_godl() (codec_read_reg(CODEC_REG_GCR_DACL) &   DACL_GODL_MASK)

#define __codec_set_godr(value) 						\
	do {									\
		codec_write_reg(CODEC_REG_GCR_DACR, (value & DACR_GODR_MASK));	\
										\
	} while (0)
#define __codec_get_godr() (codec_read_reg(CODEC_REG_GCR_DACR) &   DACR_GODR_MASK)

#define __codec_set_gimix(value)							\
	do {									\
		codec_write_reg(CODEC_REG_GCR_MIXADC, (value & MIXADC_GIMIX_MASK));	\
										\
	} while (0)
#define __codec_get_gimix() (codec_read_reg(CODEC_REG_GCR_MIXADC) & MIXADC_GIMIX_MASK)

#define __codec_set_gomix(value)							\
	do {									\
		codec_write_reg(CODEC_REG_GCR_MIXDAC, (value & MIXDAC_GOMIX_MASK));	\
										\
	} while (0)
#define __codec_get_gomix() (codec_read_reg(CODEC_REG_GCR_MIXDAC) & MIXDAC_GOMIX_MASK)

/*############################### misc start #################################*/

/*======================================================*/

#endif // __JZ4760_CODEC_H__
