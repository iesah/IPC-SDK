/*
 * Linux/sound/oss/codec/
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __WM8594_CODEC_H__
#define __WM8594_CODEC_H__

#include <mach/jzsnd.h>
#include <linux/bitops.h>

/* Standby */
#define STANDBY 1

/* Power setting */
#define POWER_ON	0
#define POWER_OFF	1

/* File ops mode W & R */
#define LEFT_CHANNEL	1
#define RIGHT_CHANNEL	2

#define MAX_RATE_COUNT	11

/*
 * WM8594 CODEC REGISTER
 *
 */

#define CODEC_REG_RSET_ID	0x0
/***********/

#define CODEC_REG_REVISION	0x1
/***********/

#define CODEC_REG_DAC1_CRTL1	0x2
 #define DAC1_FMT	0
 #define DAC1_WL	2
 #define DAC1_BCP	4
 #define DAC1_LRP	5
 #define DAC1_DEEMPH	6
 #define DAC1_ZCEN	7
 #define DAC1_EN	8
 #define DAC1_MUTE	9
 #define DAC1_OP_MUX	10

#define CODEC_REG_DAC1_CRTL2	0x3
/***********/
 #define DAC1_SR	0
 #define DAC1_BCLKDIV	3

#define CODEC_REG_DAC1_CRTL3	0x4
/***********/
 #define DAC1_MSTR	0

#define CODEC_REG_DAC1L_VOL		0x5
/***********/
 #define DAC1L_VOL	0
 #define DAC1L_VU	8

#define CODEC_REG_DAC1R_VOL		0x6
/***********/
 #define DAC1R_VOL	0
 #define DAC1R_VU	8

#define CODEC_REG_DAC2_CRTL1	0x7
 #define DAC2_FMT	0
 #define DAC2_WL	2
 #define DAC2_BCP	4
 #define DAC2_LRP	5
 #define DAC2_DEEMPH	6
 #define DAC2_ZCEN	7
 #define DAC2_EN	8
 #define DAC2_MUTE	9
 #define DAC2_OP_MUX	10

#define CODEC_REG_DAC2_CRTL2	0x8
/***********/
 #define DAC2_SR	0
 #define DAC2_BCLKDIV	3

#define CODEC_REG_DAC2_CRTL3	0x9
/***********/
 #define DAC2_MSTR	0

#define CODEC_REG_DAC2L_VOL		0xa
/***********/
 #define DAC2L_VOL	0
 #define DAC2L_VU	8

#define CODEC_REG_DAC2R_VOL		0xb
/***********/
 #define DAC2R_VOL	0
 #define DAC2R_VU	8

#define CODEC_REG_ENABLE		0xc
/***********/
 #define GLOBAL_ENABLE	0
 #define DAC2_COPY_DAC1	1

#define CODEC_REG_ADC_CTRL1		0xd
 #define ADC_FMT	0
 #define ADC_WL		2
 #define ADC_BCP	4
 #define ADC_LRP	5
 #define ADC_EN		6
 #define ADC_LRSWAP	7
 #define ADCR_INV	8
 #define ADCL_INV	9
 #define ADC_DATA_SEL	10
 #define ADC_HPD	12
 #define ADC_ZCEN	13

#define CODEC_REG_ADC_CTRL2		0xe
/***********/
 #define ADC_SR		0
 #define ADC_BCLKDIV	3

#define CODEC_REG_ADC_CTRL3		0xf
/***********/
 #define ADC_MSTR	0

#define CODEC_REG_ADCL_VOL		0x10
/***********/
 #define ADCL_VOL	0
 #define ADCL_VU	8

#define CODEC_REG_ADCR_VOL		0x11
/***********/
 #define ADCR_VOL	0
 #define ADCR_VU	8

#define CODEC_REG_PGA1L_VOL		0x13
/***********/
 #define PGA1L_VOL	0
 #define PGA1L_VU	8

#define CODEC_REG_PGA1R_VOL		0x14
/***********/
 #define PGA1R_VOL	0
 #define PGA1R_VU	8


#define CODEC_REG_PGA2L_VOL		0x15
/***********/
 #define PGA2L_VOL	0
 #define PGA2L_VU	8

#define CODEC_REG_PGA2R_VOL		0x16
/***********/
 #define PGA2R_VOL	0
 #define PGA2R_VU	8

#define CODEC_REG_PGA3L_VOL		0x17
/***********/
 #define PGA3L_VOL	0
 #define PGA3L_VU	8

#define CODEC_REG_PGA3R_VOL		0x18
/***********/
 #define PGA3R_VOL	0
 #define PGA3R_VU	8

#define CODEC_REG_PGA_CTRL1		0x19
/***********/
 #define DECAY_BYPASS	0
 #define ATTACK_BYPASS	1
 #define PGA1L_ZC		2
 #define PGA1R_ZC		3
 #define PGA2L_ZC		4
 #define PGA2R_ZC		5
 #define PGA3L_ZC		6
 #define PGA3R_ZC		7

#define CODEC_REG_PGA_CTRL2		0x1a
/***********/
 #define MUTE_ALL		0
 #define PGA1L_MUTE		1
 #define PGA1R_MUTE		2
 #define PGA2L_MUTE		3
 #define PGA2R_MUTE		4
 #define PGA3L_MUTE		5
 #define PGA3R_MUTE		6

#define CODEC_REG_ADD_CTRL1		0x1b
/***********/
 #define AUTO_INC		3
 #define PGA_SR			4

#define CODEC_REG_INPUT_CTRL1	0x1c
/***********/
 #define PGA1L_IN_SEL	0
 #define PGA1R_IN_SEL	4
 #define PGA2L_IN_SEL	8

#define CODEC_REG_INPUT_CTRL2	0x1d
/***********/
 #define PGA2R_IN_SEL	0
 #define PGA3L_IN_SEL	4
 #define PGA3R_IN_SEL	8

#define CODEC_REG_INPUT_CTRL3	0x1e
/***********/
 #define ADCL_SEL	0
 #define ADCR_SEL	4
 #define ADC_AMP_VOL	8
 #define ADC_SWITCH_EN	10

#define CODEC_REG_INPUT_CTRL4	0x1f
/***********/
 #define PGA1L_EN	0
 #define PGA1R_EN	1
 #define PGA2L_EN	2
 #define PGA2R_EN	3
 #define PGA3L_EN	4
 #define PGA3R_EN	5
 #define ADCL_AMP_EN	6
 #define ADCR_AMP_EN	7

#define	CODEC_REG_OUTPUT_CTRL1	0x20
/***********/
 #define VOUT1L_SEL	0
 #define VOUT1R_SEL	3
 #define VOUT2L_SEL	6

#define	CODEC_REG_OUTPUT_CTRL2	0x21
/***********/
 #define VOUT2R_SEL	0
 #define VOUT3L_SEL	3
 #define VOUT3R_SEL	6

#define	CODEC_REG_OUTPUT_CTRL3	0x22
/***********/
 #define VOUT1L_TRI	0
 #define VOUT1R_TRI	1
 #define VOUT2L_TRI	2
 #define VOUT2R_TRI	3
 #define VOUT3L_TRI	4
 #define VOUT3R_TRI	5
 #define APE_B		6
 #define VOUT1L_EN	7
 #define VOUT1R_EN	8
 #define VOUT2L_EN	9
 #define VOUT2R_EN	10
 #define VOUT3L_EN	11
 #define VOUT3R_EN	12

#define CODEC_REG_BIAS			0x23
/***********/
 #define POB_CTRL	0
 #define VMID_TOG	1
 #define FAST_EN	2
 #define BUFIO_EN	3
 #define SOFT_ST	4
 #define BIAS_EN	5
 #define VMID_SEL	6

#define CODEC_REG_PGA_CTRL3		0x24
/***********/
 #define PGA_SAFE_FW	0
 #define PGA_SEL	1
 #define PGA_UPD	10

#endif
