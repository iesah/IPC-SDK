/*
 * Linux/sound/oss/codec/
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __AK4951_CODEC_H__
#define __AK4951_CODEC_H__

#include <mach/jzsnd.h>
#include <linux/bitops.h>

#define POWER_MANAGEMENT1	0x00
#define POWER_MANAGEMENT2	0x01
#define SIGNAL_SELECT1		0x02
#define SIGNAL_SELECT2		0x03
#define SIGNAL_SELECT3		0x04
#define MODE_CONTROL1		0x05
#define MODE_CONTROL2		0x06

#define PLL_MASTER_MODE		0x03
#define PLL_Slave_Mode		0x02
#define EXT_Master_Mode		0x01
#define EXT_Slave_Mode		0x00

#define PLL_REF_CLOCK_24M		0x07
#define PLL_REF_CLOCK_12M		0x06
#define PLL_REF_CLOCK_12_288M	0x05
#define PLL_REF_CLOCK_11_2896M	0x04

/* Sample Rate*/
#define SAMPLE_RATE_8K			0x0
#define SAMPLE_RATE_12K			0x1
#define SAMPLE_RATE_16K			0x2
#define SAMPLE_RATE_11_025K		0x5
#define SAMPLE_RATE_22_05K		0x7
#define SAMPLE_RATE_24K			0x9
#define SAMPLE_RATE_32K			0xa
#define SAMPLE_RATE_48K			0xb
#define SAMPLE_RATE_44_1K		0xf

/*Set BICK Output Frequency*/
#define BICK_OUTPUT_32FS		0
#define BICK_OUTPUT_64FS		1

/*Set Audio Interface Format*/
#define AUDIO_DATA_FMT_ADC_24MSB_DAC_24LSB		0x0
#define AUDIO_DATA_FMT_ADC_24MSB_DAC_16LSB		0x1
#define AUDIO_DATA_FMT_ADC_24MSB_DAC_24MSB		0x2
#define AUDIO_DATA_FMT_ADC_I2S_DAC_I2S			0x3

/*Set ADC Mono/Stereo Mode*/
#define ADCLD_0_ADCRD_0			0x0
#define ADCLD_L_ADCRD_L			0x1
#define ADCLD_R_ADCRD_R			0x2
#define ADCLD_L_ADCRD_R			0x3

/*Select MIC Input*/
#define SELECT_LIN1_RIN1		0x0
#define SELECT_LIN1_RIN2		0x1
#define SELECT_LIN1_RIN3		0x2
#define SELECT_LIN2_RIN1		0x4
#define SELECT_LIN2_RIN2		0x5
#define SELECT_LIN2_RIN3		0x6
#define SELECT_LIN3_RIN1		0x8
#define SELECT_LIN3_RIN2		0x9
#define SELECT_LIN3_RIN3		0xa

/*Set Microphone Gain Amplifier*/
#define MIC_GAIN_0DB			0x0
#define MIC_GAIN_3DB			0x1
#define MIC_GAIN_6DB			0x2
#define MIC_GAIN_9DB			0x3
#define MIC_GAIN_12DB			0x4
#define MIC_GAIN_15DB			0x5
#define MIC_GAIN_18DB			0x6
#define MIC_GAIN_21DB			0x7

/*Select MIC*/
#define SELECT_INTERNAL_MIC		0x1
#define SELECT_EXTERNAL_MIC		0x3
#define SELECT_MIC_NULL			0x0

/*Set gain and output level*/
#define SET_GAIN_LEVEL_6_4DB	0x0
#define SET_GAIN_LEVEL_8_4DB	0x1
#define SET_GAIN_LEVEL_11_1DB	0x2
#define SET_GAIN_LEVEL_14_9DB	0x3

#endif
