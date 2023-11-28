/*
 * Linux/sound/oss/jz4760_route_conf.h
 *
 * DLV CODEC driver for Ingenic Jz4760 MIPS processor
 *
 * 2010-11-xx   jbbi <jbbi@ingenic.cn>
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __M200_ROUTE_CONF_H__
#define __M200_ROUTE_CONF_H__

#include <mach/jzsnd.h>

typedef struct __route_conf_base {
	/*--------route-----------*/
	int route_ready_mode;
	//record//
	int route_mic1_mode;
	int route_mic2_mode;
	int route_line1_mode;
	int route_line2_mode;
	int route_inputl_mux_mode;
	int route_inputr_mux_mode;
	int route_inputl_mode;
	int route_inputr_mode;
	int route_inputl_to_bypass_mode;
	int route_inputr_to_bypass_mode;
	int route_record_mux_mode;
	int route_adc_mode;
	unsigned long route_record_mixer_mode;
	//replay
	unsigned long route_replay_mixer_mode;
	int route_dac_mode;
	int route_hp_mux_mode;
	int route_hp_mode;
	int route_lineout_mux_mode; //new
	int route_lineout_mode;
	/*--------attibute-------*/
	int attibute_agc_mode;

	/* gain note: use 32 instead of 0 */
	/*gain of mic or linein to adc ??*/
	//NOTE excet mixer gain,other gain better set in board config
	int attibute_input_l_gain;			//val: 32(0), +4, +8, +16, +20 (dB);
	int attibute_input_r_gain;			//val: 32(0), +4, +8, +16, +20 (dB);
	int attibute_input_bypass_l_gain;	//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
	int attibute_input_bypass_r_gain;	//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
	int attibute_adc_l_gain;			//val: 32(0), +1 ~ +43 (dB);
	int attibute_adc_r_gain;			//val: 32(0), +1 ~ +43 (dB);
	int attibute_record_mixer_gain;		//val: 32(0), -1 ~ -31 (dB);
	int attibute_replay_mixer_gain;		//val: 32(0), -1 ~ -31 (dB);
	int attibute_dac_l_gain;			//val: 32(0), -1 ~ -31 (dB);
	int attibute_dac_r_gain;			//val: 32(0), -1 ~ -31 (dB);
	int attibute_hp_l_gain;				//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
	int attibute_hp_r_gain;				//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
} route_conf_base;

struct __codec_route_info {
	enum snd_codec_route_t route_name;
	route_conf_base const *route_conf;
};

/*================ route conf ===========================*/

#define DISABLE								99

/*-------------- route part selection -------------------*/

/*route global init*/
#define ROUTE_READY_FOR_ADC					1
#define ROUTE_READY_FOR_DAC					2
#define ROUTE_READY_FOR_ADC_DAC				3

#define INPUT_MASK_INPUT_MUX				0x3
#define INPUT_MASK_BYPASS_MUX				0xC

/*left input mux */
#define INPUTL_MUX_MIC1_TO_AN1			0x01
#define INPUTL_MUX_MIC1_TO_AN2			0x02
#define INPUTL_MUX_LINEIN1_TO_AN1			0x04
#define INPUTL_MUX_LINEIN1_TO_AN2			0x08

/*right input mux*/
#define INPUTR_MUX_MIC2_TO_AN3			0x01
#define INPUTR_MUX_MIC2_TO_AN4			0x02
#define INPUTR_MUX_LINEIN2_TO_AN3			0x04
#define INPUTR_MUX_LINEIN2_TO_AN4			0x08

/*mic1 mode select*/
#define MIC1_DIFF_WITH_MICBIAS				1
#define MIC1_DIFF_WITHOUT_MICBIAS			2
#define MIC1_SING_WITH_MICBIAS				3
#define MIC1_SING_WITHOUT_MICBIAS			4
#define MIC1_DISABLE						DISABLE

/*input mode select*/
#define INPUTR_TO_ADC_ENABLE				1
#define INPUTR_TO_ADC_DISABLE				DISABLE
#define INPUTL_TO_ADC_ENABLE				1
#define INPUTL_TO_ADC_DISABLE				DISABLE


/*mic2 mode select*/
#define MIC2_SING_WITH_MICBIAS				1
#define MIC2_SING_WITHOUT_MICBIAS			2
#define MIC2_DISABLE						DISABLE

/*line1 mode select*/
#define LINE1_DIFF							1
#define LINE1_SING							2

/*line2 mode select*/
#define LINE2_SING							1

/*left input bypass to output*/
#define INPUTL_TO_BYPASS_ENABLE				1
#define INPUTL_TO_BYPASS_DISABLE			DISABLE

/*right input bypass to output*/
#define INPUTR_TO_BYPASS_ENABLE				1
#define INPUTR_TO_BYPASS_DISABLE			DISABLE


/*record mux*/
#define RECORD_MUX_INPUTL_TO_LR				1		/*mic or linein mono*/
#define RECORD_MUX_INPUTR_TO_LR				2		/*mic or linein mono*/
#define RECORD_MUX_INPUTL_TO_L_INPUTR_TO_R	3		/*mic or linein stereo*/
#define RECORD_MUX_INPUTR_TO_L_INPUTL_TO_R	4		/*mic or linein stereo*/
#define RECORD_MUX_DIGITAL_MIC				5		/*digital mic stereo*/

/*adc mode*/
#define ADC_STEREO							1
#define ADC_STEREO_WITH_LEFT_ONLY			2
#define ADC_DMIC_ENABLE						3
#define ADC_DISABLE							DISABLE

/*record aiadc mixer*/
#define RECORD_MIXER_NO_USE					0x1
#define	RECORD_MIXER_INPUT_ONLY				0x2
#define RECORD_MIXER_INPUT_AND_DAC			0x4
#define RECORD_MIXER_MASK					0x7

#define AIADC_MIXER_INPUTL_TO_L				0x8
#define AIADC_MIXER_INPUTR_TO_L				0x10
#define AIADC_MIXER_INPUTLR_TO_L			0x20
#define AIADC_MIXER_NOINPUT_TO_L			0x40
#define AIADC_MIXER_L_MASK					0x78

#define AIADC_MIXER_INPUTL_TO_R				0x80
#define AIADC_MIXER_INPUTR_TO_R				0x100
#define AIADC_MIXER_INPUTLR_TO_R			0x200
#define AIADC_MIXER_NOINPUT_TO_R			0x400
#define AIADC_MIXER_R_MASK					0x780

/*replay aidac mixer*/
#define REPLAY_MIXER_NO_USE				0x1
#define REPLAY_MIXER_DAC_ONLY			0x2
#define REPLAY_MIXER_DAC_AND_ADC		0x4
#define REPLAY_MIXER_MASK				0X7

#define AIDAC_MIXER_DACL_TO_L			0x8
#define AIDAC_MIXER_DACR_TO_L			0x10
#define AIDAC_MIXER_DACLR_TO_L			0x20
#define AIDAC_MIXER_NODAC_TO_L			0x40
#define AIDAC_MIXER_L_MASK				0x78

#define AIDAC_MIXER_DACL_TO_R			0x80
#define AIDAC_MIXER_DACR_TO_R			0x100
#define AIDAC_MIXER_DACLR_TO_R			0x200
#define AIDAC_MIXER_NODAC_TO_R			0x400
#define AIDAC_MIXER_R_MASK				0x780


/*mixer adc dac*/
#define	MIXADC_MIXER_INPUTL_TO_L			0x800
#define MIXADC_MIXER_INPUTR_TO_L			0x1000
#define MIXADC_MIXER_INPUTLR_TO_L			0x2000
#define MIXADC_MIXER_NOINPUT_TO_L			0x4000
#define MIXADC_MIXER_L_MASK					0x7800

#define MIXADC_MIXER_INPUTL_TO_R			0x8000
#define MIXADC_MIXER_INPUTR_TO_R			0x10000
#define MIXADC_MIXER_INPUTLR_TO_R			0x20000
#define MIXADC_MIXER_NOINPUT_TO_R			0x40000
#define MIXADC_MIXER_R_MASK					0x78000

#define MIXDAC_MIXER_L_TO_DACL				0X80000
#define MIXDAC_MIXER_R_TO_DACL				0x100000
#define MIXDAC_MIXER_LR_TO_DACL				0x200000
#define MIXDAC_MIXER_NO_TO_DACL				0x400000
#define MIXDAC_MIXER_L_MASK					0x780000

#define MIXDAC_MIXER_L_TO_DACR				0X800000
#define MIXDAC_MIXER_R_TO_DACR				0x1000000
#define MIXDAC_MIXER_LR_TO_DACR				0x2000000
#define MIXDAC_MIXER_NO_TO_DACR				0x4000000
#define MIXDAC_MIXER_R_MASK					0x7800000

/*lineout mode*/
#define LINEOUT_ENABLE						1
#define LINEOUT_DISABLE						DISABLE

/*line out mux*/
#define LO_MUX_INPUTL_TO_LO					1
#define LO_MUX_INPUTR_TO_LO					2
#define LO_MUX_INPUTLR_TO_LO				3
#define LO_MUX_DACL_TO_LO					4
#define LO_MUX_DACR_TO_LO					5
#define LO_MUX_DACLR_TO_LO					6

/*headphone mode*/
#define HP_ENABLE							1
#define HP_DISABLE							DISABLE

/*headphone mux*/
#define HP_MUX_DACL_TO_L_DACR_TO_R			1
#define HP_MUX_DACL_TO_LR					2
#define HP_MUX_INPUTL_TO_L_INPUTR_TO_R		3
#define HP_MUX_INPUTL_TO_LR					4
#define HP_MUX_INPUTR_TO_LR					5

/*dac mode*/
#define DAC_STEREO							1
#define DAC_STEREO_WITH_LEFT_ONLY			2
#define DAC_DISABLE							DISABLE


/*other control*/
#define RECORD_WND_FILTER					0X01
#define RECORD_HIGH_PASS_FILTER				0X02

#define AGC_ENABLE							1
#define AGC_DISABLE							DISABLE

extern struct __codec_route_info codec_route_info[];

#endif /*__M200_ROUTE_CONF_H__*/
