/*
 * Linux/sound/oss/jz4760_route_conf.h
 *
 * CODEC CODEC driver for Ingenic Jz4760 MIPS processor
 *
 * 2010-11-xx   jbbi <jbbi@ingenic.cn>
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#include "jz_route_conf_v11.h"

/*######################################################################################################*/

route_conf_base const record_mic1_mono_diff_with_bias_conf = {
	.route_ready_mode = ROUTE_READY_FOR_ADC, //fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //fix
	.route_mic2_mode = MIC2_DISABLE, //..
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
	.route_record_mux_mode = RECORD_MUX_MIC1_TO_LR, //fix
	.route_adc_mode = ADC_MONO, //fix
	.route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //fix
};
route_conf_base const record_mic1_mono_diff_without_bias_conf;

route_conf_base const record_mic1_mono_sing_with_bias_conf;
route_conf_base const record_mic1_mono_sing_without_bias_conf;

route_conf_base const record_mic2_mono_diff_with_bias_conf = {
	.route_ready_mode = ROUTE_READY_FOR_ADC, //fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DIFF_WITH_MICBIAS, //fix
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
	.route_record_mux_mode = RECORD_MUX_MIC2_TO_LR, //fix
	.route_adc_mode = ADC_MONO, //fix
	.route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //fix

};
//tang add
route_conf_base const record_dmic_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DMIC, //fix
	/*--------route-----------*/
	//record
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DISABLE, //fix
	.route_dmic_mode = DMIC_DIFF_WITH_LOW_RATE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
	.route_record_mux_mode = RECORD_MUX_DIGITAL_MIC, //fix   //set reg:CR_ADC  bit:DMIC_SEL=dmic
//	.route_adc_mode = DAC_DISABLE, //fix
	.route_adc_mode = ADC_MONO, //fix
	.route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //tang test
};

route_conf_base const record_mic2_mono_diff_without_bias_conf;

route_conf_base const record_mic2_mono_sing_with_bias_conf;
route_conf_base const record_mic2_mono_sing_without_bias_conf;

route_conf_base const record_stereo_mic1_left_diff_with_bias_conf;
route_conf_base const record_stereo_mic1_left_diff_without_bias_conf;

route_conf_base const record_stereo_mic1_left_sing_with_bias_conf;
route_conf_base const record_stereo_mic1_left_sing_without_bias_conf;

route_conf_base const record_stereo_mic2_left_diff_with_bias_conf;
route_conf_base const record_stereo_mic2_left_diff_without_bias_conf;

route_conf_base const record_stereo_mic2_left_sing_with_bias_conf;
route_conf_base const record_stereo_mic2_left_sing_without_bias_conf;

route_conf_base const record_linein_conf;

/*##########################################################################################################*/

route_conf_base const replay_hp_stereo_cap_less_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO, //fix
	.route_hp_mux_mode = HP_MUX_DAC_OUTPUT, //new fix
	.route_hp_mode = HP_ENABLE_CAP_LESS, //fix
	.route_lineout_mode = LINEOUT_DISABLE, //..
};

route_conf_base const replay_hp_stereo_with_cap_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO, //fix
	.route_hp_mux_mode = HP_MUX_DAC_OUTPUT, //new fix
	.route_hp_mode = HP_ENABLE_WITH_CAP, //fix
	//.route_lineout_mode = LINEOUT_DISABLE, //..
};

route_conf_base const replay_hp_stereo_cap_less_and_lineout_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO, //fix
	.route_hp_mux_mode = HP_MUX_DAC_OUTPUT, //fix
	.route_lineout_mux_mode = LO_MUX_DAC_OUTPUT, //fix
	.route_hp_mode = HP_ENABLE_CAP_LESS, //fix
	.route_lineout_mode = LINEOUT_ENABLE, //fix
};

route_conf_base const replay_hp_stereo_with_cap_and_lineout_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO, //fix
	.route_hp_mux_mode = HP_MUX_DAC_OUTPUT, //fix
	.route_lineout_mux_mode = LO_MUX_DAC_OUTPUT, //fix
	.route_hp_mode = HP_ENABLE_WITH_CAP, //fix
	.route_lineout_mode = LINEOUT_ENABLE, //fix
};

route_conf_base const replay_lineout_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO, //fix
	.route_lineout_mux_mode = LO_MUX_DAC_OUTPUT, //new fix
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_ENABLE, //fix
};

/*########################################################################################################*/

route_conf_base const bypass_mic1_diff_with_bias_to_hp_cap_less_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //fix
	.route_mic2_mode = MIC2_DISABLE, //..
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //new ..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //new ..
	.route_adc_mode = ADC_DISABLE, //..
	//replay
	.route_dac_mode = DAC_DISABLE, //fix
	.route_hp_mux_mode = HP_MUX_MIC1_TO_LR, //new fix
	.route_hp_mode = HP_ENABLE_CAP_LESS, //fix
	.route_lineout_mode = LINEOUT_DISABLE,//..
};

route_conf_base const bypass_mic1_diff_with_bias_to_lineout_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //fix
	.route_mic2_mode = MIC2_DISABLE, //..
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //new ..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //new ..
	.route_adc_mode = ADC_DISABLE, //..
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO, //fix
	.route_lineout_mux_mode = LO_MUX_BYPASS_PATH, //new fix
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_ENABLE,//fix
};

route_conf_base const bypass_mic2_diff_with_bias_to_hp_cap_less_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DIFF_WITH_MICBIAS, //fix
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //new ..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //new ..
	.route_record_mux_mode = RECORD_MUX_MIC2_TO_LR, //fix
	.route_adc_mode = ADC_DISABLE, //..
	//replay
	.route_dac_mode = DAC_DISABLE, //..
	.route_hp_mux_mode = HP_MUX_MIC2_TO_LR, //new fix
	.route_hp_mode = HP_ENABLE_CAP_LESS, //fix
	.route_lineout_mode = LINEOUT_DISABLE,//..
};

route_conf_base const bypass_mic2_diff_with_bias_to_lineout_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DIFF_WITH_MICBIAS, //fix
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //new ..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //new ..
	.route_record_mux_mode = RECORD_MUX_MIC2_TO_LR, //fix
	.route_adc_mode = ADC_DISABLE, //..
	//replay
	.route_dac_mode = DAC_DISABLE, //..
	.route_lineout_mux_mode = LO_MUX_MIC2_EN, //new fix
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_ENABLE,//fix
};

route_conf_base const bypass_linein_to_hp_with_cap_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DISABLE, //..
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_ENABLE, //new fix
	.route_adc_mode = ADC_DISABLE, //..
	//replay
	//.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_DISABLE, //fix
	.route_hp_mux_mode = HP_MUX_BYPASS_PATH, //new fix
	.route_hp_mode = HP_ENABLE_WITH_CAP, //fix
	.route_lineout_mode = LINEOUT_DISABLE,//..
};

route_conf_base const bypass_linein_to_lineout_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DISABLE, //..
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_ENABLE, //new fix
	.route_adc_mode = ADC_DISABLE, //..
	//replay
	//.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_DISABLE, //fix
	.route_lineout_mux_mode = LO_MUX_BYPASS_PATH, //new fix
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_ENABLE,//fix
};

/*######################################################################################################*/

route_conf_base const record_stereo_mic_diff_with_bias_bypass_mixer_mic2_to_hp_cap_less_conf = {
	.route_ready_mode = ROUTE_READY_FOR_ADC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //fix
	.route_mic2_mode = MIC2_DIFF_WITH_MICBIAS, //fix
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
	.route_record_mux_mode = RECORD_MUX_MIC1_TO_R_MIC2_TO_L, //fix
	.route_adc_mode = ADC_STEREO, //fix
	.route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //fix
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO_WITH_LEFT_ONLY, //fix
	.route_hp_mux_mode = HP_MUX_DAC_OUTPUT, //fix
	.route_hp_mode = HP_ENABLE_CAP_LESS, //fix
	.route_lineout_mode = LINEOUT_DISABLE,//..
};

route_conf_base const record_stereo_mic_diff_with_bias_bypass_mixer_mic2_to_lineout_conf = {
	.route_ready_mode = ROUTE_READY_FOR_ADC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //fix
	.route_mic2_mode = MIC2_DIFF_WITH_MICBIAS, //fix
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
	.route_record_mux_mode = RECORD_MUX_MIC1_TO_R_MIC2_TO_L, //fix
	.route_adc_mode = ADC_STEREO, //fix
	.route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //fix
	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO_WITH_LEFT_ONLY, //fix
	.route_lineout_mux_mode = LO_MUX_DAC_OUTPUT, //fix
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_ENABLE,//fix
};


//bt

route_conf_base const route_dmic_to_adclr_and_daclr_to_lo = {
	.route_ready_mode = ROUTE_READY_FOR_ADC, //fix

	//record
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DISABLE, //fix
	.route_dmic_mode = DMIC_DIFF_WITH_LOW_RATE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
	.route_record_mux_mode = RECORD_MUX_DIGITAL_MIC, //fix   //set reg:CR_ADC  bit:DMIC_SEL=dmic
	.route_adc_mode = ADC_MONO, //fix
	.route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //tang test

	//replay
	.route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
	.route_dac_mode = DAC_STEREO, //fix
	.route_lineout_mux_mode = LO_MUX_DAC_OUTPUT, //new fix
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_ENABLE, //fix

};


route_conf_base const route_linein2_to_adclr_and_daclr_to_hp = {
    .route_ready_mode = ROUTE_READY_FOR_ADC, //fix
    /*--------route-----------*/
    //record
    .route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //fix
    .route_mic2_mode = MIC2_DISABLE, //..
    .route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
    .route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
    .route_record_mux_mode = RECORD_MUX_MIC1_TO_LR, //fix
    .route_adc_mode = ADC_MONO, //fix
    .route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //fix

    //replay
    .route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
    .route_dac_mode = DAC_STEREO, //fix
    .route_hp_mux_mode = HP_MUX_DAC_OUTPUT, //new fix
    .route_hp_mode = HP_ENABLE_CAP_LESS, //fix
    .route_lineout_mode = LINEOUT_DISABLE, //..
};


route_conf_base const route_linein2_to_adclr_and_daclr_to_lo = {
        .route_ready_mode = ROUTE_READY_FOR_ADC, //fix
        /*--------route-----------*/
        //record
        .route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //fix
        .route_mic2_mode = MIC2_DISABLE, //..
        .route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //..
        .route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //..
        .route_record_mux_mode = RECORD_MUX_MIC1_TO_LR, //fix
        .route_adc_mode = ADC_MONO, //fix
        .route_record_mixer_mode = RECORD_MIXER_MIX1_INPUT_ONLY, //fix

        //replay
        .route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
        .route_dac_mode = DAC_STEREO, //fix
        .route_hp_mux_mode = HP_MUX_DAC_OUTPUT, //new fix
        .route_hp_mode = HP_DISABLE, //fix
        .route_lineout_mode = LINEOUT_ENABLE, //..


//              .route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
//              .route_dac_mode = DAC_MONO, //fix
/*
        .route_ready_mode = ROUTE_READY_FOR_ADC,
            //record
.route_record_mux_mode = RECORD_MUX_MIC1_TO_LR, //fix
            //replay
                .route_mic1_mode = MIC1_DIFF_WITH_MICBIAS, //..
                .route_replay_mixer_mode = REPLAY_MIXER_PLAYBACK_DAC_ONLY, //fix
                .route_dac_mode = DAC_MONO, //fix
                .route_adc_mode = ADC_MONO, //fix
                .route_lineout_mux_mode = LO_MUX_DAC_OUTPUT, //new fix
                .route_linein_to_adc_mode = LINEIN_TO_ADC_ENABLE,
                //.route_hp_mode = HP_DISABLE, //..
                .route_lineout_mode = LINEOUT_ENABLE, //fix    */
};


/*##############################################################################################################*/

route_conf_base const route_all_clear_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DISABLE, //..
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //new ..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //new ..
	.route_adc_mode = ADC_DISABLE, //..
	//replay
	.route_dac_mode = DAC_DISABLE, //..
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_DISABLE, //..
};

route_conf_base const route_replay_clear_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_dac_mode = DAC_DISABLE, //..
	.route_hp_mode = HP_DISABLE, //..
	.route_lineout_mode = LINEOUT_DISABLE, //..
};

route_conf_base const route_record_clear_conf = {
	.route_ready_mode = ROUTE_READY_FOR_DAC, //fix
	/*--------route-----------*/
	.route_mic1_mode = MIC1_DISABLE, //..
	.route_mic2_mode = MIC2_DISABLE, //..
	.route_dmic_mode = DMIC_DISABLE,
	.route_linein_to_adc_mode = LINEIN_TO_ADC_DISABLE, //new ..
	.route_linein_to_bypass_mode = LINEIN_TO_BYPASS_DISABLE, //new ..
	.route_adc_mode = ADC_DISABLE, //..
};

/*######################################################################################################*/

struct __codec_route_info codec_route_info[] = {

	/************************ route clear ************************/
	{
		.route_name = ROUTE_ALL_CLEAR,
		.route_conf = &route_all_clear_conf,
	},
	{
		.route_name = ROUTE_REPLAY_CLEAR,
		.route_conf = &route_replay_clear_conf,
	},
	{
		.route_name = ROUTE_RECORD_CLEAR,
		.route_conf = &route_record_clear_conf,
	},

	/*********************** record route *************************/
	{
		.route_name = RECORD_MIC1_MONO_DIFF_WITH_BIAS,
		.route_conf = &record_mic1_mono_diff_with_bias_conf,
	},
	{
		.route_name = RECORD_MIC2_MONO_DIFF_WITH_BIAS,
		.route_conf = &record_mic2_mono_diff_with_bias_conf,
	},
	{
		.route_name = RECORD_DMIC,
		.route_conf = &record_dmic_conf,
	},
	/*********************** replay route **************************/
	{
		.route_name = REPLAY_HP_STEREO_CAP_LESS,
		.route_conf = &replay_hp_stereo_cap_less_conf,
	},
	{
		.route_name = REPLAY_HP_STEREO_WITH_CAP,
		.route_conf = &replay_hp_stereo_with_cap_conf,
	},
	{
		.route_name = REPLAY_HP_STEREO_CAP_LESS_AND_LINEOUT,
		.route_conf = &replay_hp_stereo_cap_less_and_lineout_conf,
	},
	{
		.route_name = REPLAY_HP_STEREO_WITH_CAP_AND_LINEOUT,
		.route_conf = &replay_hp_stereo_with_cap_and_lineout_conf,
	},
	{
		.route_name = REPLAY_LINEOUT,
		.route_conf = &replay_lineout_conf,
	},

	/*********************** bypass route *************************/
	{
		.route_name = BYPASS_MIC1_DIFF_WITH_BIAS_TO_HP_CAP_LESS,
		.route_conf = &bypass_mic1_diff_with_bias_to_hp_cap_less_conf,
	},
	{
		.route_name = BYPASS_MIC1_DIFF_WITH_BIAS_TO_LINEOUT,
		.route_conf = &bypass_mic1_diff_with_bias_to_lineout_conf,
	},
	{
		.route_name = BYPASS_MIC2_DIFF_WITH_BIAS_TO_HP_CAP_LESS,
		.route_conf = &bypass_mic2_diff_with_bias_to_hp_cap_less_conf,
	},
	{
		.route_name = BYPASS_MIC2_DIFF_WITH_BIAS_TO_LINEOUT,
		.route_conf = &bypass_mic2_diff_with_bias_to_lineout_conf,
	},
	{
		.route_name = BYPASS_LINEIN_TO_HP_WITH_CAP,
		.route_conf = &bypass_linein_to_hp_with_cap_conf,
	},
	{
		.route_name = BYPASS_LINEIN_TO_LINEOUT,
		.route_conf = &bypass_linein_to_lineout_conf,
	},

	/********************* bypass mixer route ********************/
	{
		.route_name = RECORD_STEREO_MIC_DIFF_WITH_BIAS_BYPASS_MIXER_MIC2_TO_HP_CAP_LESS,
		.route_conf = &record_stereo_mic_diff_with_bias_bypass_mixer_mic2_to_hp_cap_less_conf,
	},
	{
		.route_name = RECORD_STEREO_MIC_DIFF_WITH_BIAS_BYPASS_MIXER_MIC2_TO_LINEOUT,
		.route_conf = &record_stereo_mic_diff_with_bias_bypass_mixer_mic2_to_lineout_conf,
	},
	/***************************end of array***************************/
	{
		.route_name = SND_ROUTE_NONE,
		.route_conf = NULL,
	},
};
