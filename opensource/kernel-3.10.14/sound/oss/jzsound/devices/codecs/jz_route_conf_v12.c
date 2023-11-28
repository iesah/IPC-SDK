/*
 * Linux/sound/oss/jz4760_route_conf.h
 *
 * DLV CODEC driver for Ingenic Jz4760 MIPS processor
 *
 * 2010-11-xx   jbbi <jbbi@ingenic.cn>
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#include <mach/jzsnd.h>
#include "jz_route_conf_v12.h"
/***************************************************************************************\
 *                                                                                     *
 *typical config for each route                                                        *
 *                                                                                     *
\***************************************************************************************/


/*######################################################################################################*/

route_conf_base const record_mic1_an1_to_adclr  = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,			//fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_DIFF_WITH_MICBIAS,			//fix
	.route_mic2_mode = MIC2_DISABLE,				//fix
	.route_inputl_mux_mode = INPUTL_MUX_MIC1_TO_AN1,		//fix
	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,			//fix
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,			//fix
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,	//fix
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,	//fix
	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_LR,		//fix
	.route_adc_mode = ADC_STEREO_WITH_LEFT_ONLY,			//fix
	/*If you just have one mic ,you should select this for stereo output*/
	.route_record_mixer_mode = RECORD_MIXER_INPUT_ONLY|AIADC_MIXER_INPUTL_TO_L|AIADC_MIXER_INPUTL_TO_R,	//fix
};

route_conf_base const record_mic1_single_an2_to_adclr = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,			//fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_SING_WITH_MICBIAS,			//..
	.route_mic2_mode = MIC2_DISABLE,				//fix
	.route_inputl_mux_mode = INPUTL_MUX_MIC1_TO_AN2,		//fix
	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,			//fix
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,			//fix
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,	//fix
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,	//fix
	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_LR,		//fix
	.route_adc_mode = ADC_STEREO_WITH_LEFT_ONLY,			//fix
	/*If you just have one mic ,you should select this for stereo output*/
	.route_record_mixer_mode = RECORD_MIXER_INPUT_ONLY|AIADC_MIXER_INPUTL_TO_L|AIADC_MIXER_INPUTL_TO_R,	//fix
};

route_conf_base const record_mic2_single_an3_to_adclr  = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,			//fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_DISABLE,				//..
	.route_mic2_mode = MIC2_SING_WITH_MICBIAS,			//fix
	.route_inputr_mux_mode = INPUTR_MUX_MIC2_TO_AN3,		//fix
	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,			//fix
	.route_inputr_mode = INPUTR_TO_ADC_ENABLE,			//fix
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,	//fix
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,	//fix
	.route_record_mux_mode = RECORD_MUX_INPUTR_TO_LR,		//..
	.route_adc_mode = ADC_STEREO_WITH_LEFT_ONLY,			//fix
	/*If you just have one mic ,you should select this for stereo output*/
	.route_record_mixer_mode = RECORD_MIXER_INPUT_ONLY|AIADC_MIXER_INPUTL_TO_L|AIADC_MIXER_INPUTL_TO_R,	//fix
};

route_conf_base const record_line1_to_adrl_and_line2_to_adcr = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,			//fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_SING_WITHOUT_MICBIAS,		//..
	.route_mic2_mode = MIC2_SING_WITHOUT_MICBIAS,		//fix
	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,			//fix
	.route_inputr_mode = INPUTR_TO_ADC_ENABLE,			//fix
	.route_inputl_mux_mode = INPUTL_MUX_MIC1_TO_AN2,		//..
	.route_inputr_mux_mode = INPUTR_MUX_MIC2_TO_AN3,		//fix
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,	//fix
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,	//fix
	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_L_INPUTR_TO_R,		//fix
	.route_adc_mode = ADC_STEREO,			//fix
};

route_conf_base const record_linein1_diff_to_adclr = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,			//fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_DIFF_WITHOUT_MICBIAS,				//..
	.route_mic2_mode = MIC2_DISABLE,				//fix
	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,			//fix
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,			//fix
	.route_inputl_mux_mode = INPUTL_MUX_MIC1_TO_AN2,		//..
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,	//fix
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,	//fix
	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_LR,		//fix
	.route_adc_mode = ADC_STEREO_WITH_LEFT_ONLY,			//fix
	/*If you just have one mic ,you should select this for stereo output*/
	.route_record_mixer_mode = RECORD_MIXER_INPUT_ONLY|AIADC_MIXER_INPUTL_TO_L|AIADC_MIXER_INPUTL_TO_R,	//fix
};

route_conf_base record_linein2_single_to_adclr = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,			//fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_DISABLE,				//..
	.route_mic2_mode = MIC2_SING_WITH_MICBIAS,			//fix
	.route_inputr_mux_mode = INPUTR_MUX_MIC2_TO_AN3,		//fix
	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,			//fix
	.route_inputr_mode = INPUTR_TO_ADC_ENABLE,			//fix
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,	//fix
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,	//fix
	.route_record_mux_mode = RECORD_MUX_INPUTR_TO_LR,		//..
	.route_adc_mode = ADC_STEREO,			//fix
	/*If you just have one mic ,you should select this for stereo output*/
	.route_record_mixer_mode = RECORD_MIXER_INPUT_ONLY|AIADC_MIXER_INPUTR_TO_L|AIADC_MIXER_INPUTR_TO_R,	//fix
};

/*##########################################################################################################*/

route_conf_base const replay_hp_stereo = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,	//fix
	/*--------route-----------*/
	//replay
	.route_dac_mode = DAC_STEREO,			//fix
	.route_hp_mux_mode = HP_MUX_DACL_TO_L_DACR_TO_R,//fix
	.route_hp_mode = HP_ENABLE,			//fix
	.route_lineout_mode = LINEOUT_DISABLE,		//fix
	.route_replay_mixer_mode = REPLAY_MIXER_DAC_ONLY|AIDAC_MIXER_DACL_TO_L|AIDAC_MIXER_DACR_TO_R,
};

route_conf_base const replay_lineout_lr = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,	//fix
	/*--------route-----------*/
	//replay
	.route_dac_mode = DAC_STEREO,			//fix
	.route_lineout_mux_mode = LO_MUX_DACLR_TO_LO,	//fix
	.route_hp_mode = HP_DISABLE,			//fix
	.route_lineout_mode = LINEOUT_ENABLE,		//fix
	.route_replay_mixer_mode = REPLAY_MIXER_DAC_ONLY|AIDAC_MIXER_DACL_TO_L|AIDAC_MIXER_DACR_TO_R,
};

route_conf_base const repaly_hp_stereo_and_lineout_lr = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,	//fix
	/*--------route-----------*/
	//replay
	.route_dac_mode = DAC_STEREO,			//fix
	.route_hp_mux_mode = HP_MUX_DACL_TO_L_DACR_TO_R,//fix
	.route_hp_mode = HP_ENABLE,			//fix
	.route_lineout_mux_mode = LO_MUX_DACLR_TO_LO,	//fix
	.route_lineout_mode = LINEOUT_ENABLE,		//FIX
	.route_replay_mixer_mode = REPLAY_MIXER_DAC_ONLY|AIDAC_MIXER_DACL_TO_L|AIDAC_MIXER_DACR_TO_R,

};
/*########################################################################################################*/
route_conf_base const loop_mic1_an1_to_hp_stereo = {
	.route_ready_mode = ROUTE_READY_FOR_ADC_DAC,
	/*-----route---------------*/
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_DAC_AND_ADC|MIXADC_MIXER_INPUTL_TO_L|
		MIXADC_MIXER_INPUTL_TO_R|MIXDAC_MIXER_NO_TO_DACL|MIXDAC_MIXER_NO_TO_DACR|
		AIDAC_MIXER_DACL_TO_L|AIDAC_MIXER_DACR_TO_R,
	.route_dac_mode = DAC_STEREO,
	.route_hp_mux_mode = HP_MUX_DACL_TO_L_DACR_TO_R,//fix
	.route_hp_mode = HP_ENABLE,			//fix
	.route_lineout_mode = LINEOUT_DISABLE,		//fix

	//record
	.route_mic1_mode = MIC1_DIFF_WITH_MICBIAS,			//fix
	.route_mic2_mode = MIC2_DISABLE,				//fix
	.route_inputl_mux_mode = INPUTL_MUX_MIC1_TO_AN1,		//fix
	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,			//fix
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,			//fix
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,	//fix
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,	//fix
	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_LR,		//fix
	.route_adc_mode = ADC_STEREO_WITH_LEFT_ONLY,			//fix
};

/*########################################################################################################*/

route_conf_base const replay_linein2_bypass_to_hp_lr = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,
	//record
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,
	.route_inputr_mux_mode = INPUTR_MUX_LINEIN2_TO_AN3,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_ENABLE,
	.route_adc_mode = ADC_DISABLE,
	//replay
	.route_dac_mode = DAC_DISABLE,
	.route_hp_mux_mode = HP_MUX_INPUTR_TO_LR,
	.route_hp_mode = HP_ENABLE,
	.route_lineout_mux_mode = LO_MUX_DACLR_TO_LO,	//new
	.route_lineout_mode = LINEOUT_DISABLE,
};

#if 0
route_conf_base const call_mic_bypass_to_hp_lr = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,
	//record
	.route_line1_mode = LINE1_SING,
	.route_line2_mode = LINE2_SING,
	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,
        .route_inputl_mux_mode = INPUTL_MUX_LINEIN1_TO_AN1,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_ENABLE:,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,
	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_L_INPUTR_TO_R,
	.route_adc_mode = ADC_STEREO,
	//replay
	.route_dac_mode = DAC_STEREO,
	.route_hp_mux_mode = HP_MUX_INPUTR_TO_LR,
	.route_hp_mode = HP_ENABLE,
	.route_lineout_mux_mode = LO_MUX_DACL_TO_LO,	//new
	.route_lineout_mode = LINEOUT_ENABLE,
};
#endif
route_conf_base const call_mic_bypass_to_hp_lr = {
	.route_ready_mode = ROUTE_READY_FOR_ADC_DAC,
	//record
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_line1_mode = LINE1_DIFF,
	.route_line2_mode = LINE2_SING,

	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,
    .route_inputl_mux_mode = INPUTL_MUX_LINEIN1_TO_AN1,
    .route_inputr_mux_mode = INPUTR_MUX_LINEIN2_TO_AN3,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_ENABLE,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_ENABLE,
/*	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_LR,*/
	//.route_record_mux_mode = RECORD_MUX_INPUTL_TO_LR,
	.route_adc_mode = ADC_DISABLE,
	//replay
/*	.route_dac_mode = DAC_STEREO,*/
	.route_hp_mux_mode = HP_MUX_INPUTR_TO_LR,
	.route_hp_mode = HP_ENABLE,
	.route_lineout_mux_mode = LO_MUX_INPUTL_TO_LO,	//new
	.route_lineout_mode = LINEOUT_ENABLE,
};
route_conf_base const call_record_mic_bypass_to_hp_lr = {
	.route_ready_mode = ROUTE_READY_FOR_ADC_DAC,
	//record
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_line1_mode = LINE1_DIFF,
	.route_line2_mode = LINE2_SING,

	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,
	.route_inputr_mode = INPUTR_TO_ADC_ENABLE,
	//.route_inputl_mux_mode = INPUTL_MUX_MIC1_TO_AN2,
	//.route_inputr_mux_mode = INPUTR_MUX_MIC2_TO_AN3,
	.route_inputl_mux_mode = INPUTL_MUX_LINEIN1_TO_AN1,
	.route_inputr_mux_mode = INPUTR_MUX_LINEIN2_TO_AN3,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_ENABLE,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_ENABLE,
	//.route_record_mux_mode = RECORD_MUX_INPUTL_TO_L_INPUTR_TO_R,
    .route_record_mux_mode = RECORD_MUX_INPUTR_TO_L_INPUTL_TO_R,
	.route_record_mixer_mode = RECORD_MIXER_INPUT_ONLY|AIADC_MIXER_INPUTL_TO_L|AIADC_MIXER_INPUTL_TO_R,
	.route_adc_mode = ADC_STEREO,
	//replay
	.route_hp_mux_mode = HP_MUX_INPUTR_TO_LR,
	.route_hp_mode = HP_ENABLE,
	.route_lineout_mux_mode = LO_MUX_INPUTL_TO_LO,	//new
	.route_lineout_mode = LINEOUT_ENABLE,
};

route_conf_base const route_linein2_to_adclr_and_daclr_to_lo = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,
	//record
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_line1_mode = LINE1_SING,
	.route_line2_mode = LINE2_SING,

	.route_inputl_mode = INPUTL_TO_ADC_ENABLE,
	.route_inputr_mode = INPUTR_TO_ADC_ENABLE,
	.route_inputl_mux_mode = INPUTL_MUX_MIC1_TO_AN2,
	.route_inputr_mux_mode = INPUTR_MUX_MIC2_TO_AN3,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,
	.route_record_mux_mode = RECORD_MUX_INPUTL_TO_L_INPUTR_TO_R,
	.route_adc_mode = ADC_STEREO,
	.route_record_mixer_mode = RECORD_MIXER_INPUT_ONLY|AIADC_MIXER_INPUTL_TO_L|AIADC_MIXER_INPUTR_TO_R,	//fix
	//replay

	.route_dac_mode = DAC_STEREO,
	.route_hp_mode = HP_DISABLE,
	.route_lineout_mux_mode = LO_MUX_DACLR_TO_LO,  //new
	.route_lineout_mode = LINEOUT_ENABLE,
};

route_conf_base const replay_linein1_bypass_to_hp_lr = {
	//record
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_line1_mode = LINE1_DIFF,
	.route_line2_mode = LINE2_SING,
	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,
	.route_inputr_mux_mode = INPUTL_MUX_LINEIN1_TO_AN1,
	.route_inputr_mux_mode = INPUTR_MUX_LINEIN2_TO_AN3,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_ENABLE,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_ENABLE,
	.route_adc_mode = ADC_DISABLE,
	//replay
	.route_dac_mode = DAC_DISABLE,
	.route_hp_mux_mode = HP_MUX_INPUTL_TO_LR,
	.route_hp_mode = HP_ENABLE,
	.route_lineout_mux_mode = LO_MUX_INPUTL_TO_LO,
	.route_lineout_mode = LINEOUT_ENABLE,

};

route_conf_base const replay_linein2_bypass_to_lo_lr = {
	.route_ready_mode = ROUTE_READY_FOR_ADC,
	//record
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,
	.route_inputr_mux_mode = INPUTR_MUX_LINEIN2_TO_AN3,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_ENABLE,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_ENABLE,
	.route_adc_mode = ADC_DISABLE,
	//replay
	.route_dac_mode = DAC_DISABLE,
	.route_hp_mux_mode = HP_MUX_INPUTR_TO_LR,
	.route_hp_mode = HP_DISABLE,
	.route_lineout_mux_mode = LO_MUX_INPUTR_TO_LO,//new
	.route_lineout_mode = LINEOUT_ENABLE,
};
/*##############################################################################################################*/

route_conf_base const route_all_clear_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,
	.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,
	.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,
	.route_adc_mode = ADC_DISABLE,
	.route_record_mixer_mode = RECORD_MIXER_NO_USE,
	//replay
	.route_dac_mode = DAC_DISABLE,
	.route_hp_mode = HP_DISABLE,
	.route_lineout_mode = LINEOUT_DISABLE,
	.route_replay_mixer_mode = REPLAY_MIXER_NO_USE,
};

route_conf_base const route_replay_clear_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,
	/*--------route-----------*/
	.route_dac_mode = DAC_DISABLE,
#ifdef CONFIG_ANDROID
	.route_hp_mode = HP_DISABLE,
#endif
	.route_lineout_mode = LINEOUT_DISABLE,
	.route_replay_mixer_mode = REPLAY_MIXER_NO_USE,
};

route_conf_base const route_record_clear_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC,
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DISABLE,
	.route_mic2_mode = MIC2_DISABLE,
	.route_inputl_mode = INPUTL_TO_ADC_DISABLE,
	.route_inputr_mode = INPUTR_TO_ADC_DISABLE,
	//.route_inputl_to_bypass_mode = INPUTL_TO_BYPASS_DISABLE,
	//.route_inputr_to_bypass_mode = INPUTR_TO_BYPASS_DISABLE,
	.route_adc_mode = ADC_DISABLE,
	.route_record_mixer_mode = RECORD_MIXER_NO_USE,
};

/*######################################################################################################*/

struct __codec_route_info codec_route_info[] = {

	/************************ route clear ************************/
	{
		.route_name = SND_ROUTE_ALL_CLEAR,
		.route_conf = &route_all_clear_conf,
	},
	{
		.route_name = SND_ROUTE_REPLAY_CLEAR,
		.route_conf = &route_replay_clear_conf,
	},
	{
		.route_name = SND_ROUTE_RECORD_CLEAR,
		.route_conf = &route_record_clear_conf,
	},

	/*********************** record route *************************/
	{
		.route_name = SND_ROUTE_RECORD_MIC1_AN1,
		.route_conf = &record_mic1_an1_to_adclr,
	},
	{
		.route_name = SND_ROUTE_RECORD_MIC1_SIN_AN2,
		.route_conf = &record_mic1_single_an2_to_adclr,
	},
	{
		.route_name = SND_ROUTE_RECORD_MIC2_SIN_AN3,
		.route_conf = &record_mic2_single_an3_to_adclr,
	},
	/*********************** replay route **************************/
	{
		.route_name = SND_ROUTE_REPLAY_DACRL_TO_HPRL,
		.route_conf = &replay_hp_stereo,
	},
	{
		.route_name = SND_ROUTE_REPLAY_DACRL_TO_LO,
		.route_conf = &replay_lineout_lr,
	},
	{
		.route_name = SND_ROUTE_REPLAY_DACRL_TO_ALL,
		.route_conf = &repaly_hp_stereo_and_lineout_lr,
	},
	/*********************** bypass route *****************************/
	{
		.route_name = SND_ROUTE_LOOP_MIC1_AN1_LOOP_TO_HP,
		.route_conf = &loop_mic1_an1_to_hp_stereo,
	},

	{
		.route_name = SND_ROUTE_LINE1IN_BYPASS_TO_HP,
		.route_conf = &replay_linein1_bypass_to_hp_lr,
	},

	{
		.route_name = SND_ROUTE_REPLAY_LINEIN2_BYPASS_TO_LINEOUT,
		.route_conf = &replay_linein2_bypass_to_lo_lr,
	},

	{
		.route_name = SND_ROUTE_RECORD_LINEIN1_AN2_SIN_TO_ADCL_AND_LINEIN2_AN3_SIN_TO_ADCR,
		.route_conf = &record_line1_to_adrl_and_line2_to_adcr,
	},

	{
		.route_name = SND_ROUTE_RECORD_LINEIN1_DIFF_AN2,
		.route_conf = &record_linein1_diff_to_adclr,
	},

	{
		.route_name = SND_ROUTE_RECORD_LINEIN2_SIN_AN3,
		.route_conf = &record_linein2_single_to_adclr,
	},

	{
		.route_name = SND_ROUTE_REPLAY_LINEIN2_BYPASS_TO_HPRL,
		.route_conf = &replay_linein2_bypass_to_hp_lr,
	},
	{
		.route_name = SND_ROUTE_CALL_MIC_BYPASS_TO_HPRL,
		.route_conf = &call_mic_bypass_to_hp_lr,
	},
	/*********************** misc route *******************************/
	{
		.route_name = SND_ROUTE_CALL_RECORD,
		.route_conf = &call_record_mic_bypass_to_hp_lr,
	},
	{
		.route_name = SND_ROUTE_MIC2_AN3_TO_AD_AND_DA_TO_LO,
		.route_conf = &route_linein2_to_adclr_and_daclr_to_lo,
	},

	/***************************end of array***************************/
	{
		.route_name = SND_ROUTE_NONE,
		.route_conf = NULL,
	},
};
