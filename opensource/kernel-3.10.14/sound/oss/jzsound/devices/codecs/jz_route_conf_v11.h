/*
 * Linux/sound/oss/jz4760_route_conf.h
 *
 * CODEC CODEC driver for Ingenic Jz4760 MIPS processor
 *
 * 2010-11-xx   jbbi <jbbi@ingenic.cn>
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4775_ROUTE_CONF_H__
#define __JZ4775_ROUTE_CONF_H__

#include <mach/jzsnd.h>

typedef struct __route_conf_base {
	/*--------route-----------*/
	int route_ready_mode;
	//record//
	int route_mic1_mode;
	int route_mic2_mode;
	int route_dmic_mode;
	int route_linein_to_adc_mode; //new
	int route_linein_to_bypass_mode; //new
	int route_record_mux_mode;
	int route_adc_mode;
	int route_record_mixer_mode;
	//replay
	int route_replay_mixer_mode;
	int route_dac_mode;
	int route_hp_mux_mode; //new
	int route_hp_mode;
	int route_lineout_mux_mode; //new
	int route_lineout_mode;
	/*--------attibute-------*/
	int attibute_agc_mode;

	/* gain note: use 32 instead of 0 */
	int attibute_mic1_gain;		//val: 32(0), +4, +8, +16, +20 (dB);
	int attibute_mic2_gain;		//val: 32(0), +4, +8, +16, +20 (dB);
	int attibute_linein_l_gain;	//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
	int attibute_linein_r_gain;	//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
	int attibute_adc_l_gain;	//val: 32(0), +1 ~ +43 (dB);
	int attibute_adc_r_gain;	//val: 32(0), +1 ~ +43 (dB);
	int attibute_record_mixer_gain;	//val: 32(0), -1 ~ -31 (dB);
	int attibute_replay_mixer_gain;	//val: 32(0), -1 ~ -31 (dB);
	int attibute_dac_l_gain;	//val: 32(0), -1 ~ -31 (dB);
	int attibute_dac_r_gain;	//val: 32(0), -1 ~ -31 (dB);
	int attibute_hp_l_gain;		//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
	int attibute_hp_r_gain;		//val: +6 ~ +1, 32(0), -1 ~ -25 (dB);
} route_conf_base;

struct __codec_route_info {
	unsigned int route_name;
	route_conf_base const *route_conf;
};

/*================ route conf ===========================*/

#define DISABLE						99

/*-------------- route part selection -------------------*/

#define ROUTE_READY_FOR_ADC				1
#define ROUTE_READY_FOR_DAC				2
#define ROUTE_READY_FOR_ADC_DAC				3
#define ROUTE_READY_FOR_DMIC				4

#define MIC1_DIFF_WITH_MICBIAS				1
#define MIC1_DIFF_WITHOUT_MICBIAS			2
#define MIC1_SING_WITH_MICBIAS				3
#define MIC1_SING_WITHOUT_MICBIAS			4
#define MIC1_DISABLE					DISABLE

#define MIC2_DIFF_WITH_MICBIAS				1
#define MIC2_DIFF_WITHOUT_MICBIAS			2
#define MIC2_SING_WITH_MICBIAS				3
#define MIC2_SING_WITHOUT_MICBIAS			4
#define MIC2_DISABLE					DISABLE

#define DMIC_DIFF_WITH_HIGH_RATE			1
#define DMIC_DIFF_WITH_LOW_RATE	 		    2
#define DMIC_DISABLE					DISABLE

#define LINEIN_TO_ADC_ENABLE 				1
#define LINEIN_TO_ADC_DISABLE				DISABLE

#define LINEIN_TO_BYPASS_ENABLE				1
#define LINEIN_TO_BYPASS_DISABLE			DISABLE

#define AGC_ENABLE					1
#define AGC_DISABLE					DISABLE

#define RECORD_MUX_MIC1_TO_LR				1
#define RECORD_MUX_MIC2_TO_LR				2
#define RECORD_MUX_MIC1_TO_R_MIC2_TO_L			3
#define RECORD_MUX_MIC2_TO_R_MIC1_TO_L			4
#define RECORD_MUX_LINE_IN				5
#define RECORD_MUX_DIGITAL_MIC				6

#define ADC_STEREO					1
#define ADC_STEREO_WITH_LEFT_ONLY			2
#define ADC_MONO					3
#define ADC_DISABLE					DISABLE

#define RECORD_MIXER_MIX1_INPUT_ONLY			1
#define RECORD_MIXER_MIX1_INPUT_AND_DAC			2

#define REPLAY_MIXER_PLAYBACK_DAC_ONLY			1
#define REPLAY_MIXER_PLAYBACK_DAC_AND_ADC		2

#define DAC_STEREO					1
#define DAC_STEREO_WITH_LEFT_ONLY			2
#define DAC_MONO					3
#define DAC_DISABLE					DISABLE

#define REPLAY_FILTER_STEREO				1
#define REPLAY_FILTER_MONO				2

#define HP_MUX_MIC1_TO_LR 				1
#define HP_MUX_MIC2_TO_LR 				2
#define HP_MUX_MIC1_TO_R_MIC2_TO_L 			3
#define HP_MUX_MIC2_TO_R_MIC1_TO_L			4
#define HP_MUX_BYPASS_PATH 				5
#define HP_MUX_DAC_OUTPUT 				6

#define LO_MUX_MIC1_EN 					1
#define LO_MUX_MIC2_EN 					2
#define LO_MUX_MIC1_AND_MIC2_EN 			3
#define LO_MUX_BYPASS_PATH 				4
#define LO_MUX_DAC_OUTPUT 				5

#define HP_ENABLE_WITH_CAP				1
#define HP_ENABLE_CAP_LESS				2
#define HP_DISABLE					DISABLE

#define LINEOUT_ENABLE					1
#define LINEOUT_DISABLE					DISABLE

/***************************************************************************************\
 *                                                                                     *
 *route table                                                                          *
 *                                                                                     *
\***************************************************************************************/

extern struct __codec_route_info codec_route_info[];

#endif
