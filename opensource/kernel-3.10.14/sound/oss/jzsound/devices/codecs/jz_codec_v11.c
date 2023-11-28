/*
 * Linux/sound/oss/xb47XX/xb4780/jz4780_codec.c
 *
 * CODEC CODEC driver for Ingenic Jz4780 MIPS processor
 *
 * 2010-11-xx   jbbi <jbbi@ingenic.cn>
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/sound.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <linux/proc_fs.h>
#include <linux/soundcard.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/gpio.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>

#include "../xb47xx_i2s.h"
#include "jz_codec_v11.h"
#include "jz_route_conf_v11.h"

/*###############################################*/
#define CODEC_DUMP_IOC_CMD			0
#define CODEC_DUMP_ROUTE_REGS		0
#define CODEC_DUMP_ROUTE_PART_REGS	0
#define CODEC_DUMP_GAIN_PART_REGS	0
#define CODEC_DUMP_ROUTE_NAME		1
#define CODEC_DUMP_GPIO_STATE		0
/*##############################################*/

/***************************************************************************************\                                                                                *
 *global variable and structure interface                                              *
\***************************************************************************************/

static struct snd_board_route *cur_route = NULL;
static struct snd_board_route *keep_old_route = NULL;
static struct snd_codec_data *codec_platform_data = NULL;
static int default_replay_volume_base = 0;
static int default_record_volume_base = 0;
static int default_record_digital_volume_base = 0;
static int default_replay_digital_volume_base = 0;
static int default_bypass_l_volume_base = 0;					/*call uplink or other use*/
static int default_bypass_r_volume_base = 0;					/*call downlink or other use*/
static int user_replay_volume = 100;
static int user_record_volume = 100;


extern int i2s_register_codec(char *name, void *codec_ctl,unsigned long codec_clk,enum codec_mode mode);


#define MIXER_RECORD  0x1
#define	MIXER_REPLAY  0x2

static int g_codec_sleep_mode = 1;

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend early_suspend;
#endif


/*==============================================================*/
/**
 * codec_sleep
 *
 *if use in suspend and resume, should use delay
 */
void codec_sleep(int ms)
{
	if(g_codec_sleep_mode)
		msleep(ms);
	else
		mdelay(ms);
}

static inline void codec_sleep_wait_bitset(int reg, unsigned bit, int stime, int line)
{
	int count = 0;
	while(!(codec_read_reg(reg) & (1 << bit))) {
		//printk("CODEC waiting reg(%2x) bit(%2x) set %d \n",reg, bit, line);
		codec_sleep(stime);
		count++;
		if(count > 10){
			printk("%s %d timeout\n",__FILE__,line);
			break;
		}
	}
}

/***************************************************************************************\
 *debug part                                                                           *
\***************************************************************************************/

#if CODEC_DUMP_IOC_CMD
static void codec_print_ioc_cmd(int cmd)
{
	int i;

	int cmd_arr[] = {
		CODEC_INIT,
		CODEC_TURN_ON,				CODEC_TURN_OFF,
		CODEC_SHUTDOWN,				CODEC_RESET,
		CODEC_SUSPEND,				CODEC_RESUME,
		CODEC_ANTI_POP,				CODEC_SET_DEFROUTE,
		CODEC_SET_DEVICE,			CODEC_SET_RECORD_RATE,
		CODEC_SET_RECORD_DATA_WIDTH,	CODEC_SET_MIC_VOLUME,
		CODEC_SET_RECORD_CHANNEL,		CODEC_SET_REPLAY_RATE,
		CODEC_SET_REPLAY_DATA_WIDTH,	CODEC_SET_REPLAY_VOLUME,
		CODEC_SET_REPLAY_CHANNEL,	CODEC_DAC_MUTE,
		CODEC_DEBUG_ROUTINE,		CODEC_SET_STANDBY,
		CODEC_GET_RECORD_FMT_CAP,		CODEC_GET_RECORD_FMT,
		CODEC_GET_REPLAY_FMT_CAP,	CODEC_GET_REPLAY_FMT,
		CODEC_IRQ_HANDLE,			CODEC_DUMP_REG,
		CODEC_DUMP_GPIO
	};

	char *cmd_str[] = {
		"CODEC_INIT",
		"CODEC_TURN_ON",		"CODEC_TURN_OFF",
		"CODEC_SHUTDOWN", 		"CODEC_RESET",
		"CODEC_SUSPEND",		"CODEC_RESUME",
		"CODEC_ANTI_POP",		"CODEC_SET_DEFROUTE",
		"CODEC_SET_DEVICE",		"CODEC_SET_RECORD_RATE",
		"CODEC_SET_RECORD_DATA_WIDTH", 	"CODEC_SET_MIC_VOLUME",
		"CODEC_SET_RECORD_CHANNEL", 	"CODEC_SET_REPLAY_RATE",
		"CODEC_SET_REPLAY_DATA_WIDTH", 	"CODEC_SET_REPLAY_VOLUME",
		"CODEC_SET_REPLAY_CHANNEL", 	"CODEC_DAC_MUTE",
		"CODEC_DEBUG_ROUTINE",		"CODEC_SET_STANDBY",
		"CODEC_GET_RECORD_FMT_CAP",		"CODEC_GET_RECORD_FMT",
		"CODEC_GET_REPLAY_FMT_CAP",	"CODEC_GET_REPLAY_FMT",
		"CODEC_IRQ_HANDLE",		"CODEC_DUMP_REG",
		"CODEC_DUMP_GPIO"
	};

	for ( i = 0; i < sizeof(cmd_arr) / sizeof(int); i++) {
		if (cmd_arr[i] == cmd) {
			printk("CODEC IOC: Command name : %s\n", cmd_str[i]);
			return;
		}
	}

	if (i == sizeof(cmd_arr) / sizeof(int)) {
		printk("CODEC IOC: command[%d], i[%d] is not under control\n", cmd, i);
	}
}
#endif //CODEC_DUMP_IOC_CMD

#if CODEC_DUMP_ROUTE_NAME
static void codec_print_route_name(int route)
{
	int i;

	int route_arr[] = {
		ROUTE_ALL_CLEAR,
		ROUTE_REPLAY_CLEAR,
		ROUTE_RECORD_CLEAR,
		RECORD_MIC1_MONO_DIFF_WITH_BIAS,
		RECORD_MIC2_MONO_DIFF_WITH_BIAS,
		RECORD_DMIC,
		REPLAY_HP_STEREO_CAP_LESS,
		REPLAY_HP_STEREO_WITH_CAP,
		REPLAY_HP_STEREO_CAP_LESS_AND_LINEOUT,
		REPLAY_HP_STEREO_WITH_CAP_AND_LINEOUT,
		REPLAY_LINEOUT,
		BYPASS_MIC1_DIFF_WITH_BIAS_TO_HP_CAP_LESS,
		BYPASS_MIC1_DIFF_WITH_BIAS_TO_LINEOUT,
		BYPASS_MIC2_DIFF_WITH_BIAS_TO_HP_CAP_LESS,
		BYPASS_MIC2_DIFF_WITH_BIAS_TO_LINEOUT,
		BYPASS_LINEIN_TO_HP_WITH_CAP,
		BYPASS_LINEIN_TO_LINEOUT,
		RECORD_STEREO_MIC_DIFF_WITH_BIAS_BYPASS_MIXER_MIC2_TO_HP_CAP_LESS,
		RECORD_STEREO_MIC_DIFF_WITH_BIAS_BYPASS_MIXER_MIC2_TO_LINEOUT,

		SND_ROUTE_MIC2_AN3_TO_AD_AND_DA_TO_LO,
		SND_ROUTE_MIC2_AN3_TO_AD_AND_DA_TO_HP,
		SND_ROUTE_DMIC_TO_AD_AND_DA_TO_LO,
	};

	char *route_str[] = {
		"ROUTE_ALL_CLEAR",
		"ROUTE_REPLAY_CLEAR",
		"ROUTE_RECORD_CLEAR",
		"RECORD_MIC1_MONO_DIFF_WITH_BIAS",
		"RECORD_MIC2_MONO_DIFF_WITH_BIAS",
		"RECORD_DMIC",
		"REPLAY_HP_STEREO_CAP_LESS",
		"REPLAY_HP_STEREO_WITH_CAP",
		"REPLAY_HP_STEREO_CAP_LESS_AND_LINEOUT",
		"REPLAY_HP_STEREO_WITH_CAP_AND_LINEOUT",
		"REPLAY_LINEOUT",
		"BYPASS_MIC1_DIFF_WITH_BIAS_TO_HP_CAP_LESS",
		"BYPASS_MIC1_DIFF_WITH_BIAS_TO_LINEOUT",
		"BYPASS_MIC2_DIFF_WITH_BIAS_TO_HP_CAP_LESS",
		"BYPASS_MIC2_DIFF_WITH_BIAS_TO_LINEOUT",
		"BYPASS_LINEIN_TO_HP_WITH_CAP",
		"BYPASS_LINEIN_TO_LINEOUT",
		"RECORD_STEREO_MIC_DIFF_WITH_BIAS_BYPASS_MIXER_MIC2_TO_HP_CAP_LESS",
		"RECORD_STEREO_MIC_DIFF_WITH_BIAS_BYPASS_MIXER_MIC2_TO_LINEOUT",


		"SND_ROUTE_MIC2_AN3_TO_AD_AND_DA_TO_LO",
		"SND_ROUTE_MIC2_AN3_TO_AD_AND_DA_TO_HP",
		"SND_ROUTE_DMIC_TO_AD_AND_DA_TO_LO"
	};

	for ( i = 0; i < sizeof(route_arr) / sizeof(unsigned int); i++) {
		if (route_arr[i] == route) {
			printk("\nCODEC SET ROUTE: Route name : %s\n", route_str[i]);
			return;
		}
	}

	if (i == sizeof(route_arr) / sizeof(unsigned int)) {
		printk("\nCODEC SET ROUTE: Route is not configed yet!\n");
	}
}
#endif //CODEC_DUMP_ROUTE_NAME

static void dump_gpio_state(void)
{
	int val = -1;

	if(codec_platform_data &&
	   (codec_platform_data->gpio_hp_mute.gpio != -1)) {
		val = __gpio_get_value(codec_platform_data->gpio_hp_mute.gpio);
		printk("gpio hp mute %d statue is %d.\n",codec_platform_data->gpio_hp_mute.gpio, val);
	}

	if(codec_platform_data &&
	   (codec_platform_data->gpio_spk_en.gpio != -1)) {
		val = __gpio_get_value(codec_platform_data->gpio_spk_en.gpio);
		printk("gpio speaker enable %d statue is %d.\n",codec_platform_data->gpio_spk_en.gpio, val);
	}

	if(codec_platform_data &&
	   (codec_platform_data->gpio_handset_en.gpio != -1)) {
		val = __gpio_get_value(codec_platform_data->gpio_handset_en.gpio);
		printk("gpio handset enable %d statue is %d.\n",codec_platform_data->gpio_handset_en.gpio, val);
	}

	if(codec_platform_data &&
	   (codec_platform_data->gpio_hp_detect.gpio != -1)) {
		val = __gpio_get_value(codec_platform_data->gpio_hp_detect.gpio);
		printk("gpio hp detect %d statue is %d.\n",codec_platform_data->gpio_hp_detect.gpio, val);
	}

	if(codec_platform_data &&
	   (codec_platform_data->gpio_mic_detect.gpio != -1)) {
		val = __gpio_get_value(codec_platform_data->gpio_mic_detect.gpio);
		printk("gpio mic detect %d statue is %d.\n",codec_platform_data->gpio_mic_detect.gpio, val);
	}

	if(codec_platform_data &&
	   (codec_platform_data->gpio_mic_detect_en.gpio != -1)) {
		val = __gpio_get_value(codec_platform_data->gpio_mic_detect_en.gpio);
		printk("gpio mic detect enable %d statue is %d.\n",codec_platform_data->gpio_mic_detect_en.gpio, val);
	}

	if(codec_platform_data &&
	   (codec_platform_data->gpio_buildin_mic_select.gpio != -1)) {
		val = __gpio_get_value(codec_platform_data->gpio_buildin_mic_select.gpio);
		printk("gpio_mic_switch %d statue is %d.\n",codec_platform_data->gpio_buildin_mic_select.gpio, val);
	}
}

static void dump_codec_regs(void)
{
	unsigned int i;
	unsigned char data;
	printk("codec register list:\n");
	for (i = 0; i <= 0x23; i++) {
		data = read_inter_codec_reg(i);
		printk("address = 0x%02x, data = 0x%02x\n", i, data);
	}
}

#if CODEC_DUMP_ROUTE_PART_REGS
static void dump_codec_route_regs(void)
{
	unsigned int i;
	unsigned char data;
	for (i = 0x1; i < 0xA; i++) {
		data = read_inter_codec_reg(i);
		printk("address = 0x%02x, data = 0x%02x\n", i, data);
	}
}
#endif

#if CODEC_DUMP_GAIN_PART_REGS
static void dump_codec_gain_regs(void)
{
	unsigned int i;
	unsigned char data;
	for (i = 0x12; i < 0x1E; i++) {
		data = read_inter_codec_reg(i);
		printk("address = 0x%02x, data = 0x%02x\n", i, data);
	}
}
#endif
/*=========================================================*/

#if CODEC_DUMP_ROUTE_NAME
#define DUMP_ROUTE_NAME(route) codec_print_route_name(route)
#else //CODEC_DUMP_ROUTE_NAME
#define DUMP_ROUTE_NAME(route)
#endif //CODEC_DUMP_ROUTE_NAME

/*-------------------*/
#if CODEC_DUMP_IOC_CMD
#define DUMP_IOC_CMD(value)						\
	do {								\
		printk("[codec IOCTL]++++++++++++++++++++++++++++\n");	\
		printk("%s  cmd = %d, arg = %lu-----%s\n", __func__, cmd, arg, value); \
		codec_print_ioc_cmd(cmd);				\
		printk("[codec IOCTL]----------------------------\n");	\
	} while (0)
#else //CODEC_DUMP_IOC_CMD
#define DUMP_IOC_CMD(value)
#endif //CODEC_DUMP_IOC_CMD

#if CODEC_DUMP_GPIO_STATE
#define DUMP_GPIO_STATE()			\
	do {					\
		dump_gpio_state();		\
	} while (0)
#else
#define DUMP_GPIO_STATE()
#endif

#if CODEC_DUMP_ROUTE_REGS
#define DUMP_ROUTE_REGS(value)						\
	do {								\
		printk("codec register dump,%s\tline:%d-----%s:\n",	\
		       __func__, __LINE__, value);			\
		dump_codec_regs();					\
	} while (0)
#else //CODEC_DUMP_ROUTE_REGS
#define DUMP_ROUTE_REGS(value)				\
	do {						\
		if (!strcmp("enter",value))		\
			ENTER_FUNC();			\
		else if (!strcmp("leave",value))	\
			LEAVE_FUNC();			\
	} while (0)
#endif //CODEC_DUMP_ROUTE_REGS

#if CODEC_DUMP_ROUTE_PART_REGS
#define DUMP_ROUTE_PART_REGS(value)					\
	do {								\
		if (mode != DISABLE) {					\
			printk("codec register dump,%s\tline:%d-----%s:\n", \
			       __func__, __LINE__, value);		\
			dump_codec_route_regs();			\
		}							\
	} while (0)
#else //CODEC_DUMP_ROUTE_PART_REGS
#define DUMP_ROUTE_PART_REGS(value)			\
	do {						\
		if (!strcmp("enter",value))		\
			ENTER_FUNC();			\
		else if (!strcmp("leave",value))	\
			LEAVE_FUNC();			\
	} while (0)
#endif //CODEC_DUMP_ROUTE_PART_REGS

#if CODEC_DUMP_GAIN_PART_REGS
#define DUMP_GAIN_PART_REGS(value)					\
	do {								\
		printk("codec register dump,%s\tline:%d-----%s:\n",	\
		       __func__, __LINE__, value);			\
		dump_codec_gain_regs();					\
	} while (0)
#else //CODEC_DUMP_GAIN_PART_REGS
#define DUMP_GAIN_PART_REGS(value)			\
	do {						\
		if (!strcmp("enter",value))		\
			ENTER_FUNC();			\
		else if (!strcmp("leave",value))	\
			LEAVE_FUNC();			\
	} while (0)
#endif //CODEC_DUMP_GAIN_PART_REGS

/***************************************************************************************\
 *route part and attibute                                                              *
\***************************************************************************************/
/*=========================power on==========================*/

#ifdef CONFIG_HAS_EARLYSUSPEND
static void codec_early_suspend(struct early_suspend *handler)
{
	__codec_switch_sb_micbias(POWER_OFF);
}
static void codec_late_resume(struct early_suspend *handler)
{
     __codec_switch_sb_micbias(POWER_ON);
}
#endif

static void codec_set_route_ready(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	/*wait a typical time 250ms to get into sleep mode*/
	if(__codec_get_sb() == POWER_OFF)
	{
		__codec_switch_sb(POWER_ON);
		msleep(250);
	}
	/*wait a typical time 200ms for adc (450ms for dac) to get into normal mode*/
	if(__codec_get_sb_sleep() == POWER_OFF)
	{
		__codec_switch_sb_sleep(POWER_ON);
		if(mode == ROUTE_READY_FOR_ADC)
			msleep(200);
		else
			msleep(450);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

/*=================route part functions======================*/
/* select mic1 mode */
static void codec_set_mic1(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case MIC1_DIFF_WITH_MICBIAS:
		if(__codec_get_sb_mic1() == POWER_OFF)
		{
			__codec_switch_sb_mic1(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_OFF)
		{
			__codec_switch_sb_micbias(POWER_ON);
			schedule_timeout(2);
		}
		__codec_enable_mic_diff();
		break;

	case MIC1_DIFF_WITHOUT_MICBIAS:
		if(__codec_get_sb_mic1() == POWER_OFF)
		{
			__codec_switch_sb_mic1(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_ON)
		{
			__codec_switch_sb_micbias(POWER_OFF);
			schedule_timeout(2);
		}
		__codec_enable_mic_diff();
		break;

	case MIC1_SING_WITH_MICBIAS:
		if(__codec_get_sb_mic1() == POWER_OFF)
		{
			__codec_switch_sb_mic1(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_OFF)
		{
			__codec_switch_sb_micbias(POWER_ON);
			schedule_timeout(2);
		}
		__codec_disable_mic_diff();
		break;

	case MIC1_SING_WITHOUT_MICBIAS:
		if(__codec_get_sb_mic1() == POWER_OFF)
		{
			__codec_switch_sb_mic1(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_ON)
		{
			__codec_switch_sb_micbias(POWER_OFF);
			schedule_timeout(2);
		}
		__codec_disable_mic_diff();
		break;

	case MIC1_DISABLE:
		if(__codec_get_sb_mic1() == POWER_ON)
		{
			__codec_switch_sb_mic1(POWER_OFF);
			schedule_timeout(2);
		}
		break;

	default:
		printk("JZ_CODEC: line: %d, mic1 mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_mic2(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case MIC2_DIFF_WITH_MICBIAS:
		if(__codec_get_sb_mic2() == POWER_OFF)
		{
			__codec_switch_sb_mic2(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_OFF)
		{
			__codec_switch_sb_micbias(POWER_ON);
			schedule_timeout(2);
		}
		__codec_enable_mic_diff();
		break;

	case MIC2_DIFF_WITHOUT_MICBIAS:
		if(__codec_get_sb_mic2() == POWER_OFF)
		{
			__codec_switch_sb_mic2(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_ON)
		{
			__codec_switch_sb_micbias(POWER_OFF);
			schedule_timeout(2);
		}
		__codec_enable_mic_diff();
		break;
	case MIC2_SING_WITH_MICBIAS:
		if(__codec_get_sb_mic2() == POWER_OFF)
		{
			__codec_switch_sb_mic1(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_OFF)
		{
			__codec_switch_sb_micbias(POWER_ON);
			schedule_timeout(2);
		}
		__codec_disable_mic_diff();
		break;

	case MIC2_SING_WITHOUT_MICBIAS:
		if(__codec_get_sb_mic2() == POWER_OFF)
		{
			__codec_switch_sb_mic2(POWER_ON);
			schedule_timeout(2);
		}
		if(__codec_get_sb_micbias() == POWER_ON)
		{
			__codec_switch_sb_micbias(POWER_OFF);
			schedule_timeout(2);
		}
		__codec_disable_mic_diff();
		break;

	case MIC2_DISABLE:
		if(__codec_get_sb_mic2() == POWER_ON)
		{
			__codec_switch_sb_mic2(POWER_OFF);
			schedule_timeout(2);
		}
		break;

	default:
		printk("JZ_CODEC: line: %d, mic2 mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_dmic(int mode)
{
	int test_dmic_clock = 10;
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case DMIC_DIFF_WITH_HIGH_RATE:
		printk("itang in JZ_CODEC: line: %d, dmic high rate set begin!\n", __LINE__);
		__codec_set_dmic_insel(DMIC_SEL_DIGITAL_MIC); //select Digital microphone
		__codec_set_crystal(CRYSTAL_13_MHz);
		__codec_set_dmic_clock(DMIC_CLK_ON);    //DMIC_CLK  ON

		//__codec_enable_mic_diff(); fix
		printk("itang in JZ_CODEC: line: %d, dmic high rate set end!\n", __LINE__);
		break;

	case DMIC_DIFF_WITH_LOW_RATE:
		printk("itang in JZ_CODEC: line: %d, dmic low rate set begin!\n", __LINE__);
		//jzgpio_set_func(GPIO_PORT_F,GPIO_FUNC_1 , 3 << 10);  //设置GPIO_PORT_F的10和11位为功能1.即dmic的clk和in口
		__codec_set_dmic_insel(DMIC_SEL_DIGITAL_MIC); //select Digital microphone

		do{
			__codec_switch_sb_adc(POWER_ON);
			test_dmic_clock--;
			if (!test_dmic_clock) {
				printk("--------------set adc power on failed\n");
				break;
			}
			printk("--------------set adc power on 0x%d\n",__codec_get_sb_adc());
		} while(__codec_get_sb_adc());
		test_dmic_clock = 10;
		__codec_set_crystal(CRYSTAL_12_MHz);
		if (!(0x80 & __codec_get_dmic_clock()))
		do {
			if(test_dmic_clock == 10)
			printk("-------test 10 times start 0x%x\n", __codec_get_dmic_clock());
			__codec_set_dmic_clock(DMIC_CLK_ON);    //DMIC_CLK  ON
			printk("-------test 10 times start 0x%x\n", __codec_get_dmic_clock());
			if ((__codec_get_dmic_clock()) & 0x80) {
				printk("----------- set dmic clk successful 0x%x\n", __codec_get_dmic_clock());
				break;
			}
			test_dmic_clock--;
			if(test_dmic_clock == 0) {
				printk("-------test 10 times failed reg 0x%x\n",__codec_get_dmic_clock());
			}
		} while(test_dmic_clock);
		else
			printk("----------clk on succussful\n");
			__codec_switch_sb_micbias(POWER_ON);//hwang
		printk("itang in JZ_CODEC: line: %d, dmic low rate set end!\n", __LINE__);
		//__codec_enable_mic_diff();
		break;

	case DMIC_DISABLE:
		printk("itang in JZ_CODEC: line: %d, dmic low rate set disable\n", __LINE__);
		//jzgpio_set_func(GPIO_PORT_F,GPIO_FUNC_1 , 3 << 10);  //设置GPIO_PORT_F的10和11位为功能1.即dmic的clk和in口
		__codec_set_dmic_clock(DMIC_CLK_OFF);    //DMIC_CLK  off
		break;

	default:
		printk("JZ_CODEC: line: %d, dmic mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}


static void codec_set_linein_to_adc(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case LINEIN_TO_ADC_ENABLE:
		if(__codec_get_sb_linein_to_adc() == POWER_OFF)
		{
			__codec_switch_sb_linein_to_adc(POWER_ON);
			schedule_timeout(2);
		}
		break;

	case LINEIN_TO_ADC_DISABLE:
		if(__codec_get_sb_linein_to_adc() == POWER_ON)
		{
			__codec_switch_sb_linein_to_adc(POWER_OFF);
			schedule_timeout(2);
		}
		break;

	default:
		printk("JZ_CODEC: line: %d, linein mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}
static void codec_set_linein_to_bypass(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case LINEIN_TO_BYPASS_ENABLE:
		if(__codec_get_sb_linein_to_bypass() == POWER_OFF)
		{
			__codec_switch_sb_linein_to_bypass(POWER_ON);
			schedule_timeout(2);
		}
		break;

	case LINEIN_TO_BYPASS_DISABLE:
		if(__codec_get_sb_linein_to_bypass() == POWER_ON)
		{
			__codec_switch_sb_linein_to_bypass(POWER_OFF);
			schedule_timeout(2);
		}
		break;

	default:
		printk("JZ_CODEC: line: %d, linein mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_agc(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case AGC_ENABLE:
		__codec_enable_agc();
		break;

	case AGC_DISABLE:
		__codec_disable_agc();
		break;

	default:
		printk("JZ_CODEC: line: %d, agc mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_record_mux(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case RECORD_MUX_MIC1_TO_LR:
		/* if digital micphone is not select, */
		/* 4770 codec auto set ADC_LEFT_ONLY to 1 */
		__codec_set_mic_mono();
		__codec_set_adc_insel(MIC1_TO_LR);
		__codec_set_dmic_insel(DMIC_SEL_ADC);
		break;

	case RECORD_MUX_MIC2_TO_LR:
		/* if digital micphone is not select, */
		/* 4770 codec auto set ADC_LEFT_ONLY to 1 */
		__codec_set_mic_mono();
		__codec_set_adc_insel(MIC2_TO_LR);
		__codec_set_dmic_insel(DMIC_SEL_ADC);
		break;

	case RECORD_MUX_MIC1_TO_R_MIC2_TO_L:
		__codec_set_mic_stereo();
		__codec_set_adc_insel(MIC1_TO_R_MIC2_TO_L);
		__codec_set_dmic_insel(DMIC_SEL_ADC);
		break;

	case RECORD_MUX_MIC2_TO_R_MIC1_TO_L:
		__codec_set_mic_stereo();
		__codec_set_adc_insel(MIC2_TO_R_MIC1_TO_L);
		__codec_set_dmic_insel(DMIC_SEL_ADC);
		break;

	case RECORD_MUX_LINE_IN:
		__codec_set_adc_insel(BYPASS_PATH);
		__codec_set_dmic_insel(DMIC_SEL_ADC);
		break;

	case RECORD_MUX_DIGITAL_MIC:
		__codec_set_dmic_insel(DMIC_SEL_DIGITAL_MIC);
		break;

	default:
		printk("JZ_CODEC: line: %d, record mux mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}
static void codec_set_adc(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case ADC_STEREO:
		if(__codec_get_sb_adc() == POWER_OFF)
		{
			__codec_switch_sb_adc(POWER_ON);
			schedule_timeout(2);
		}
		__codec_set_adc_stereo();
		__codec_disable_adc_left_only();
		break;

	case ADC_STEREO_WITH_LEFT_ONLY:
		if(__codec_get_sb_adc() == POWER_OFF)
		{
			__codec_switch_sb_adc(POWER_ON);
			schedule_timeout(2);
		}
		__codec_set_adc_stereo();
		__codec_enable_adc_left_only();
		break;

	case ADC_MONO:
		/*When ADC_MONO=1, the left and right channels are mixed in digital
		  part: the result is emitted on both left and right channel of ADC digital
		  output. It corresponds to the average of left and right channels when
		  ADC_MONO=0.*/
		if(__codec_get_sb_adc() == POWER_OFF)
		{
			__codec_switch_sb_adc(POWER_ON);
			schedule_timeout(2);
		}
		__codec_set_adc_mono();
		__codec_disable_adc_left_only();
		break;

	case ADC_DISABLE:
		if(__codec_get_sb_adc() == POWER_ON)
		{
			__codec_switch_sb_adc(POWER_OFF);
			schedule_timeout(2);
		}
		break;

	default:
		printk("JZ_CODEC: line: %d, adc mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_record_mixer(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case RECORD_MIXER_MIX1_INPUT_ONLY:
		__codec_set_rec_mix_mode(MIX1_RECORD_INPUT_ONLY);
		break;

	case RECORD_MIXER_MIX1_INPUT_AND_DAC:
		__codec_set_rec_mix_mode(MIX1_RECORD_INPUT_AND_DAC);
		break;

	default:
		printk("JZ_CODEC: line: %d, record mixer mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_replay_mixer(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case REPLAY_MIXER_PLAYBACK_DAC_ONLY:
		__codec_set_dac_mix_mode(MIX2_PLAYBACK_DAC_ONLY);
		break;

	case REPLAY_MIXER_PLAYBACK_DAC_AND_ADC:
		__codec_set_dac_mix_mode(MIX2_PLAYBACK_DAC_AND_ADC);
		break;

	default:
		printk("JZ_CODEC: line: %d, replay mixer mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_dac(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case DAC_STEREO:
		__codec_set_dac_stereo();
		__codec_disable_dac_left_only();
		/*If hp is enable  we use aviod pop sequence to open dac in set hp*/
		if(__codec_get_sb_dac() == POWER_OFF){
			__codec_switch_sb_dac(POWER_ON);
			udelay(500);
		}
		if(__codec_get_dac_mute()){
			/* clear IFR_GUP */
			__codec_set_irq_flag(1 << IFR_GUP);
			/* turn on dac */
			__codec_disable_dac_mute();
			/* wait IFR_GUP set */
			codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GUP, 100,__LINE__);
		}

		break;

	case DAC_STEREO_WITH_LEFT_ONLY:
		__codec_set_dac_stereo();
		__codec_enable_dac_left_only();
		if(__codec_get_sb_dac() == POWER_OFF){
			__codec_switch_sb_dac(POWER_ON);
			udelay(500);
		}
		if(__codec_get_dac_mute()){
			/* clear IFR_GUP */
			__codec_set_irq_flag(1 << IFR_GUP);
			/* turn on dac */
			__codec_disable_dac_mute();
			/* wait IFR_GUP set */
			codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GUP, 100,__LINE__);
		}
		break;

	case DAC_MONO:
		__codec_set_dac_mono();
		__codec_disable_dac_left_only();
		/*When DAC_MONO=1, the left and right channels are mixed in digital
		  part: the result is emitted on both left and right channel of DAC output. It
		  corresponds to the average of left and right channels when
		  DAC_MONO=0.*/
		if(__codec_get_sb_dac() == POWER_OFF){
			__codec_switch_sb_dac(POWER_ON);
			udelay(500);
		}
		if(__codec_get_dac_mute()){
			/* clear IFR_GUP */
			__codec_set_irq_flag(1 << IFR_GUP);
			/* turn on dac */
			__codec_disable_dac_mute();
			/* wait IFR_GUP set */
			codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GUP, 100,__LINE__);
		}
		break;

	case DAC_DISABLE:
		if(__codec_get_sb_dac() == POWER_ON){
			if (!(__codec_get_dac_mute())){
				/* clear IFR_GDO */
				__codec_set_irq_flag(1 << IFR_GDO);
				/* turn off dac */
				__codec_enable_dac_mute();
				/* wait IFR_GDO set */
				codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GDO, 100,__LINE__);
			}
			__codec_switch_sb_dac(POWER_OFF);
		}
		break;

	default:
		printk("JZ_CODEC: line: %d, dac mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_hp_mux(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case HP_MUX_MIC1_TO_LR:
		__codec_set_mic_mono();
		__codec_set_hp_sel(MIC1_TO_LR);
		break;

	case HP_MUX_MIC2_TO_LR:
		__codec_set_mic_mono();
		__codec_set_hp_sel(MIC2_TO_LR);
		break;

	case HP_MUX_MIC1_TO_R_MIC2_TO_L:
		__codec_set_mic_stereo();
		__codec_set_hp_sel(MIC1_TO_R_MIC2_TO_L);
		break;

	case HP_MUX_MIC2_TO_R_MIC1_TO_L:
		__codec_set_mic_stereo();
		__codec_set_hp_sel(MIC2_TO_R_MIC1_TO_L);
		break;

	case HP_MUX_BYPASS_PATH:
		__codec_set_hp_sel(BYPASS_PATH);
		break;

	case HP_MUX_DAC_OUTPUT:
		__codec_set_hp_sel(DAC_OUTPUT);
		break;

	default:
		printk("JZ_CODEC: line: %d, replay mux mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_hp(int mode)
{
	int linein_to_bypass_power_on = 0;
	int dac_mute_not_enable = 0;
	int load_flag = 0;

	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case HP_ENABLE_WITH_CAP:
	case HP_ENABLE_CAP_LESS:
		__codec_set_16ohm_load();

		if((__codec_get_dac_mute() == 0) && (__codec_get_sb_dac() == POWER_ON)){
			/* enable dac mute */
			__codec_set_irq_flag(1 << IFR_GDO);
			__codec_enable_dac_mute();
			codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GDO, 100,__LINE__);
			dac_mute_not_enable = 1;
		}

		/* Before we wanna to change the hpcm mode,
		 * make sure hp is poweroff to avoid big pop
		 */
		if ((mode == HP_ENABLE_CAP_LESS &&
		     __codec_get_sb_hpcm() == POWER_OFF) ||
		    (mode ==  HP_ENABLE_WITH_CAP &&
		     __codec_get_sb_hpcm()== POWER_ON)) {
			if(__codec_get_sb_hp() == POWER_ON)
			{
				/* turn off sb_hp */
				__codec_set_irq_flag(1 << IFR_RDO);
				__codec_switch_sb_hp(POWER_OFF);
				codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_RDO, 100,__LINE__);

				__codec_enable_hp_mute();

				if (__codec_get_sb_dac() == POWER_ON)
					__codec_switch_sb_dac(POWER_OFF);
			}
		}

		if (__codec_get_sb_hp() == POWER_OFF) {
			if(__codec_get_sb_linein_to_bypass() == POWER_ON){
				__codec_switch_sb_linein_to_bypass(POWER_OFF);
				linein_to_bypass_power_on = 1;
			}

			__codec_switch_sb_dac(POWER_ON);

			if (mode == HP_ENABLE_CAP_LESS) {
				if (__codec_get_sb_hpcm() == POWER_OFF)
					__codec_switch_sb_hpcm(POWER_ON);
			} else {
				if (__codec_get_sb_hpcm() == POWER_ON)
					__codec_switch_sb_hpcm(POWER_OFF);
			}

			mdelay(10);			/*At last 10ms to avoid pop*/

			__codec_disable_hp_mute();

			/* turn on sb_hp */
			__codec_set_irq_flag(1 << IFR_RUP);
			__codec_switch_sb_hp(POWER_ON);
			codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_RUP, 100,__LINE__);

			if(linein_to_bypass_power_on){
				__codec_switch_sb_linein_to_bypass(POWER_ON);
			}
		}

		if (dac_mute_not_enable) {
			/*disable dac mute*/
			__codec_set_irq_flag(1 << IFR_GUP);
			__codec_disable_dac_mute();
			codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GUP, 100,__LINE__);
		}
		break;

	case HP_DISABLE:
		if(__codec_get_sb_hp() == POWER_ON)
		{
			if(__codec_get_sb_linein_to_bypass() == POWER_ON){
				__codec_switch_sb_linein_to_bypass(POWER_OFF);
				linein_to_bypass_power_on = 1;
			}

			if((__codec_get_dac_mute() == 0) && (__codec_get_sb_dac() == POWER_ON)){
				/* enable dac mute */
				__codec_set_irq_flag(1 << IFR_GDO);
				__codec_enable_dac_mute();
				codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GDO, 100,__LINE__);

				dac_mute_not_enable = 1;
			}

			/* set 16ohm load to keep away from the bug can not waited RDO */
			if(__codec_get_load() == LOAD_10KOHM)
			{
				__codec_set_16ohm_load();
				load_flag = 1;
			}

			/* turn off sb_hp */
			__codec_set_irq_flag(1 << IFR_RDO);

			/*Power off hpcm,so we can wait RDO,otherwise can not wait RDO */
			if (__codec_get_sb_hpcm() == POWER_ON) {
				__codec_switch_sb_hpcm(POWER_OFF);
				mdelay(1);
			}
			__codec_switch_sb_hp(POWER_OFF);
			codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_RDO, 100,__LINE__);

			if(load_flag)
				__codec_set_10kohm_load();

			if(linein_to_bypass_power_on){
				__codec_switch_sb_linein_to_bypass(POWER_ON);
			}

			if(dac_mute_not_enable){
				/*disable dac mute*/
				__codec_set_irq_flag(1 << IFR_GUP);
				__codec_disable_dac_mute();
				codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GUP, 100,__LINE__);
			}
			__codec_enable_hp_mute();
		}
		break;
	default:
		printk("JZ_CODEC: line: %d, hp mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_lineout_mux(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case LO_MUX_MIC1_EN:
		__codec_set_mic_mono();
		__codec_set_lineout_sel(LO_SEL_MIC1);
		break;

	case LO_MUX_MIC2_EN:
		__codec_set_mic_mono();
		__codec_set_lineout_sel(LO_SEL_MIC2);
		break;

	case LO_MUX_MIC1_AND_MIC2_EN:
		__codec_set_mic_stereo();
		__codec_set_lineout_sel(LO_SEL_MIC1_AND_MIC2);
		break;

	case LO_MUX_BYPASS_PATH:
		__codec_set_lineout_sel(BYPASS_PATH);
		break;

	case LO_MUX_DAC_OUTPUT:
		__codec_set_lineout_sel(DAC_OUTPUT);
		break;

	default:
		printk("JZ_CODEC: line: %d, replay mux mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

static void codec_set_lineout(int mode)
{
	DUMP_ROUTE_PART_REGS("enter");

	switch(mode){

	case LINEOUT_ENABLE:
		//__codec_set_10kohm_load();
		if(__codec_get_sb_line_out() == POWER_OFF)
		{
			__codec_switch_sb_line_out(POWER_ON);
			schedule_timeout(2);
		}
		__codec_disable_lineout_mute();
		break;

	case LINEOUT_DISABLE:
		if(__codec_get_sb_line_out() == POWER_ON)
		{
			__codec_switch_sb_line_out(POWER_OFF);
			schedule_timeout(2);
		}
		break;

	default:
		printk("JZ_CODEC: line: %d, lineout mode error!\n", __LINE__);
	}

	DUMP_ROUTE_PART_REGS("leave");
}

/*=================route attibute(gain) functions======================*/

//--------------------- mic1
static int codec_get_gain_mic1(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

        val =  __codec_get_gm1();
	gain = val * 4;

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

static void codec_set_gain_mic1(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain < 0)
		gain = 0;
	else if (gain > 20)
		gain = 20;

	val = gain / 4;

	__codec_set_gm1(val);

	if (codec_get_gain_mic1() != gain)
		printk("JZ_CODEC: codec_set_gain_mic1 error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

//--------------------- mic2
static int codec_get_gain_mic2(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val =  __codec_get_gm2();
	gain = val * 4;

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

static void codec_set_gain_mic2(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain < 0)
		gain = 0;
	else if (gain > 20)
		gain = 20;

	val = gain / 4;

	__codec_set_gm2(val);

	if (codec_get_gain_mic2() != gain)
		printk("JZ_CODEC: codec_set_gain_mic2 error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

//--------------------- line in left

static int codec_get_gain_linein_left(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gil();

	gain = (6 - val);

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

static void codec_set_gain_linein_left(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 6)
		gain = 6;
	else if (gain < -25)
		gain = -25;

	val = (6 - gain);

	__codec_set_gil(val);

	if (codec_get_gain_linein_left() != gain)
		printk("JZ_CODEC: codec_set_gain_linein_left error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

//--------------------- line in right
static int codec_get_gain_linein_right(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gir();

	gain = (6 - val);

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

static void codec_set_gain_linein_right(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 6)
		gain = 6;
	else if (gain < -25)
		gain = -25;

	val = (6 - gain);

	__codec_set_gir(val);

	if (codec_get_gain_linein_right() != gain)
		printk("JZ_CODEC: codec_set_gain_linein_right error!\n");

	DUMP_GAIN_PART_REGS("leave");
}
//--------------------- adc left
static int codec_get_gain_adc_left(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gidl();

	gain = val;

	DUMP_GAIN_PART_REGS("leave");

	return gain;
}

static int codec_set_gain_adc_left(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain < 0)
		gain = 0;
	else if ( gain > 43)
		gain = 43;

	val = gain;

	__codec_set_gidl(val);

	if ((val = codec_get_gain_adc_left()) != gain)
		printk("JZ_CODEC: codec_set_gain_adc_left error!\n");

	DUMP_GAIN_PART_REGS("leave");

	return val;
}

//--------------------- adc right
static int codec_get_gain_adc_right(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gidr();

	gain = val;

	DUMP_GAIN_PART_REGS("leave");

	return gain;
}

static int codec_set_gain_adc_right(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain < 0)
		gain = 0;
	else if ( gain > 43)
		gain = 43;

	val = gain;

	__codec_set_gidr(val);

	if ((val = codec_get_gain_adc_right()) != gain)
		printk("JZ_CODEC: codec_set_gain_adc_right error!\n");

	DUMP_GAIN_PART_REGS("leave");

	return val;
}

//--------------------- record mixer
int codec_get_gain_record_mixer (void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gimix();

	gain = -val;

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

void codec_set_gain_record_mixer(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 0)
		gain = 0;
	else if (gain < -31)
		gain = -31;

	val = -gain;

	__codec_set_gimix(val);

	if (codec_get_gain_record_mixer() != gain)
		printk("JZ_CODEC: codec_set_gain_record_mixer error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

//--------------------- replay mixer
static int codec_get_gain_replay_mixer(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gomix();

	gain = -val;

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

static void codec_set_gain_replay_mixer(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 0)
		gain = 0;
	else if (gain < -31)
		gain = -31;

	val = -gain;

	__codec_set_gomix(val);

	if (codec_get_gain_replay_mixer() != gain)
		printk("JZ_CODEC: codec_set_gain_replay_mixer error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

//--------------------- dac left
static int codec_get_gain_dac_left(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_godl();

	if (val > 31)
		gain = 64 - val;
	else
		gain = 0 - val;

	DUMP_GAIN_PART_REGS("leave");

	return gain;
}

void codec_set_gain_dac_left(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 32)
		gain = 32;
	else if (gain < -31)
		gain = -31;

	if (gain > 0)
		val = 64 - gain;
	else
		val = 0 - gain;

	__codec_set_godl(val);

	if (codec_get_gain_dac_left() != gain)
		printk("JZ_CODEC: codec_set_gain_dac_left error!\n");

	DUMP_GAIN_PART_REGS("leave");
}
//--------------------- dac right
int codec_get_gain_dac_right(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_godr();

	if (val > 31)
		gain = 64 - val;
	else
		gain = 0 - val;

	DUMP_GAIN_PART_REGS("leave");

	return gain;
}

void codec_set_gain_dac_right(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 32)
		gain = 32;
	else if (gain < -31)
		gain = -31;

	if (gain > 0)
		val = 64 - gain;
	else
		val =  0 - gain;

	__codec_set_godr(val);

	if (codec_get_gain_dac_right() != gain)
		printk("JZ_CODEC: codec_set_gain_dac_right error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

//--------------------- hp left
static int codec_get_gain_hp_left(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gol();

	gain = (6 - val);

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

void codec_set_gain_hp_left(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 6)
		gain = 6;
	else if (gain < -25)
		gain = -25;

	val = (6 - gain);

	__codec_set_gol(val);

	if (codec_get_gain_hp_left() != gain)
		printk("JZ_CODEC: codec_set_gain_hp_left error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

//--------------------- hp right
static int codec_get_gain_hp_right(void)
{
	int val,gain;
	DUMP_GAIN_PART_REGS("enter");

	val = __codec_get_gor();

	gain = (6 - val);

	DUMP_GAIN_PART_REGS("leave");
	return gain;
}

void codec_set_gain_hp_right(int gain)
{
	int val;

	DUMP_GAIN_PART_REGS("enter");

	if (gain > 6)
		gain = 6;
	else if (gain < -25)
		gain = -25;

	val = (6 - gain);

	__codec_set_gor(val);

	if (codec_get_gain_hp_right() != gain)
		printk("JZ_CODEC: codec_set_gain_hp_right error!\n");

	DUMP_GAIN_PART_REGS("leave");
}

/***************************************************************************************\
 *codec route                                                                            *
 \***************************************************************************************/
static void codec_set_route_base(const void *arg)
{
	route_conf_base *conf = (route_conf_base *)arg;

	/*codec turn on sb and sb_sleep*/
	if (conf->route_ready_mode)
		codec_set_route_ready(conf->route_ready_mode);

	/*--------------route---------------*/
	/* record path */
	if (conf->route_mic1_mode)
		codec_set_mic1(conf->route_mic1_mode);

	if (conf->route_mic2_mode)
		codec_set_mic2(conf->route_mic2_mode);

	if (conf->route_dmic_mode) //hwang
		codec_set_dmic(conf->route_dmic_mode);

	if (conf->route_linein_to_adc_mode)
		codec_set_linein_to_adc(conf->route_linein_to_adc_mode);

	if (conf->route_linein_to_bypass_mode)
		codec_set_linein_to_bypass(conf->route_linein_to_bypass_mode);

	if (conf->route_record_mux_mode)
		codec_set_record_mux(conf->route_record_mux_mode);

	if (conf->route_adc_mode)
		codec_set_adc(conf->route_adc_mode);

	if (conf->route_record_mixer_mode)
		codec_set_record_mixer(conf->route_record_mixer_mode);
	/* replay path */
	if (conf->route_replay_mixer_mode)
		codec_set_replay_mixer(conf->route_replay_mixer_mode);


	if (conf->route_hp_mux_mode)
		codec_set_hp_mux(conf->route_hp_mux_mode);

	if (conf->route_hp_mode)
		codec_set_hp(conf->route_hp_mode);

	if (conf->route_dac_mode)
		codec_set_dac(conf->route_dac_mode);

	if (conf->route_lineout_mux_mode)
		codec_set_lineout_mux(conf->route_lineout_mux_mode);

	if (conf->route_lineout_mode)
		codec_set_lineout(conf->route_lineout_mode);

	/*----------------attibute-------------*/
	/* auto gain */
	if (conf->attibute_agc_mode)
		codec_set_agc(conf->attibute_agc_mode);

	/* gain , use 32 instead of 0 */
	if (conf->attibute_record_mixer_gain) {
		if (conf->attibute_record_mixer_gain == 32)
			codec_set_gain_record_mixer(0);
		else
			codec_set_gain_record_mixer(conf->attibute_record_mixer_gain);
	}

	if (conf->attibute_replay_mixer_gain) {
		if (conf->attibute_replay_mixer_gain == 32)
			codec_set_gain_replay_mixer(0);
		else
			codec_set_gain_replay_mixer(conf->attibute_replay_mixer_gain);
	}
	/*Note it should not be set in andriod ,below */

	if (conf->attibute_mic1_gain) {
		if (conf->attibute_mic1_gain == 32)
			codec_set_gain_mic1(0);
		else
			codec_set_gain_mic1(conf->attibute_mic1_gain);
	}

	if (conf->attibute_mic2_gain) {
		if (conf->attibute_mic2_gain == 32)
			codec_set_gain_mic2(0);
		else
			codec_set_gain_mic2(conf->attibute_mic2_gain);
	}

	if (conf->attibute_linein_l_gain) {
		if (conf->attibute_linein_l_gain == 32)
			codec_set_gain_linein_left(0);
		else
			codec_set_gain_linein_left(conf->attibute_linein_l_gain);
	}

	if (conf->attibute_linein_r_gain) {
		if (conf->attibute_linein_r_gain == 32)
			codec_set_gain_linein_right(0);
		else
			codec_set_gain_linein_right(conf->attibute_linein_r_gain);
	}

	if (conf->attibute_adc_l_gain) {
		if (conf->attibute_adc_l_gain == 32)
			codec_set_gain_adc_left(0);
		else
			codec_set_gain_adc_left(conf->attibute_adc_l_gain);
	}

	if (conf->attibute_adc_r_gain) {
		if (conf->attibute_adc_r_gain == 32)
			codec_set_gain_adc_right(0);
		else
			codec_set_gain_adc_right(conf->attibute_adc_r_gain);
	}

	if (conf->attibute_dac_l_gain) {
		if (conf->attibute_dac_l_gain == 32)
			codec_set_gain_dac_left(0);
		else
			codec_set_gain_dac_left(conf->attibute_dac_l_gain);
	}

	if (conf->attibute_dac_r_gain) {
		if (conf->attibute_dac_r_gain == 32)
			codec_set_gain_dac_right(0);
		else
			codec_set_gain_dac_right(conf->attibute_dac_r_gain);
	}

	if (conf->attibute_hp_l_gain) {
		if (conf->attibute_hp_l_gain == 32)
			codec_set_gain_hp_left(0);
		else
			codec_set_gain_hp_left(conf->attibute_hp_l_gain);
	}

	if (conf->attibute_hp_r_gain) {
		if (conf->attibute_hp_r_gain == 32)
			codec_set_gain_hp_right(0);
		else
			codec_set_gain_hp_right(conf->attibute_hp_r_gain);
	}
}

static int codec_set_mic_volume(int* val);
static int codec_set_replay_volume(int *val);

static void codec_set_gain_base(struct snd_board_route *route)
{
	int old_volume_base = 0;
	int tmp_volume = 0;

	if (codec_platform_data) {
		tmp_volume = user_record_volume;
		old_volume_base = codec_platform_data->record_volume_base;
		if (route->record_volume_base) {
			if (route->record_volume_base == 32)
				codec_platform_data->record_volume_base = 0;
			else
				codec_platform_data->record_volume_base = route->record_volume_base;
		} else
			codec_platform_data->record_volume_base = default_record_volume_base;

		if (old_volume_base != codec_platform_data->record_volume_base)
			codec_set_mic_volume(&tmp_volume);
	}

	/* set replay hp output gain base */
	if (codec_platform_data) {
		tmp_volume = user_replay_volume;
		old_volume_base = codec_platform_data->replay_volume_base;
		if (route->replay_volume_base) {
			if (route->replay_volume_base == 32)
				codec_platform_data->replay_volume_base = 0;
			else
				codec_platform_data->replay_volume_base = route->replay_volume_base;
		} else
			codec_platform_data->replay_volume_base = default_replay_volume_base;

		if (old_volume_base !=  codec_platform_data->replay_volume_base)
			codec_set_replay_volume(&tmp_volume);
	}

	/* set record digtal volume base */
	if (codec_platform_data) {
		if (route->record_digital_volume_base) {
			if (route->record_digital_volume_base == 32)
				codec_platform_data->record_digital_volume_base = 0;
			else
				codec_platform_data->record_digital_volume_base = route->record_digital_volume_base;
			codec_set_gain_adc_left(codec_platform_data->record_digital_volume_base);
			codec_set_gain_adc_right(codec_platform_data->record_digital_volume_base);
		} else if (codec_platform_data->record_digital_volume_base !=
			   default_record_digital_volume_base) {
			codec_platform_data->record_digital_volume_base = default_record_digital_volume_base;
			codec_set_gain_adc_left(codec_platform_data->record_digital_volume_base);
			codec_set_gain_adc_right(codec_platform_data->record_digital_volume_base);
		}
	}

	/* set replay digital volume base */
	if (codec_platform_data) {
		if (route->replay_digital_volume_base) {
			if (route->replay_digital_volume_base == 32)
				codec_platform_data->replay_digital_volume_base = 0;
			else
				codec_platform_data->replay_digital_volume_base = route->replay_digital_volume_base;
			codec_set_gain_dac_left(codec_platform_data->replay_digital_volume_base);
			codec_set_gain_dac_right(codec_platform_data->replay_digital_volume_base);
		} else if (codec_platform_data->replay_digital_volume_base !=
			   default_replay_digital_volume_base) {
			codec_platform_data->replay_digital_volume_base = default_replay_digital_volume_base;
			codec_set_gain_dac_left(codec_platform_data->replay_digital_volume_base);
			codec_set_gain_dac_right(codec_platform_data->replay_digital_volume_base);
		}
	}

	/*set bypass left volume base*/
	if (codec_platform_data) {
		if (route->bypass_l_volume_base) {
			if (route->bypass_l_volume_base == 32)
				codec_platform_data->bypass_l_volume_base = 0;
			else
				codec_platform_data->bypass_l_volume_base = route->bypass_l_volume_base;
			codec_set_gain_linein_left(codec_platform_data->bypass_l_volume_base);
		} else if (codec_platform_data->bypass_l_volume_base != default_bypass_l_volume_base) {
			codec_platform_data->bypass_l_volume_base = default_bypass_l_volume_base;
			codec_set_gain_linein_left(codec_platform_data->bypass_l_volume_base);
		}
	}

	/*set bypass right volume base*/
	if (codec_platform_data) {
		if (route->bypass_r_volume_base) {
			if (route->bypass_r_volume_base == 32)
				codec_platform_data->bypass_r_volume_base = 0;
			else
				codec_platform_data->bypass_r_volume_base = route->bypass_r_volume_base;
			codec_set_gain_linein_right(codec_platform_data->bypass_r_volume_base);
		} else if (codec_platform_data->bypass_r_volume_base != default_bypass_r_volume_base) {
			codec_platform_data->bypass_r_volume_base = default_bypass_r_volume_base;
			codec_set_gain_linein_right(codec_platform_data->bypass_r_volume_base);
		}
	}
}

/***************************************************************************************\
 *ioctl support function                                                               *
\***************************************************************************************/
/*------------------sub fun-------------------*/
static int gpio_enable_hp_mute(void)
{
	int val = -1;
	if(codec_platform_data && (codec_platform_data->gpio_hp_mute.gpio != -1)) {
		val = gpio_get_value(codec_platform_data->gpio_hp_mute.gpio);
		if (codec_platform_data->gpio_hp_mute.active_level) {
			gpio_direction_output(codec_platform_data->gpio_hp_mute.gpio , 1);
		} else {
			gpio_direction_output(codec_platform_data->gpio_hp_mute.gpio , 0);
			val = val == 1 ? 0 : val == 0 ? 1 : val ;
		}
	}
	DUMP_GPIO_STATE();
	return val;
}

static void gpio_disable_hp_mute(void)
{
	if(codec_platform_data && (codec_platform_data->gpio_hp_mute.gpio != -1)) {
		if (codec_platform_data->gpio_hp_mute.active_level) {
			gpio_direction_output(codec_platform_data->gpio_hp_mute.gpio , 0);
		} else {
			gpio_direction_output(codec_platform_data->gpio_hp_mute.gpio , 1);
		}
	}
	DUMP_GPIO_STATE();
}

static void gpio_enable_spk_en(void)
{
	if(codec_platform_data && (codec_platform_data->gpio_spk_en.gpio != -1)) {
		if (codec_platform_data->gpio_spk_en.active_level) {
			gpio_direction_output(codec_platform_data->gpio_spk_en.gpio , 1);
		} else {
			gpio_direction_output(codec_platform_data->gpio_spk_en.gpio , 0);
		}
	}
}

static int gpio_disable_spk_en(void)
{
	int val = -1;
	if(codec_platform_data && (codec_platform_data->gpio_spk_en.gpio != -1)) {
		val = gpio_get_value(codec_platform_data->gpio_spk_en.gpio);
		if (codec_platform_data->gpio_spk_en.active_level) {
			gpio_direction_output(codec_platform_data->gpio_spk_en.gpio , 0);
		} else {
			val = val == 1 ? 0 : val == 0 ? 1 : val;
			gpio_direction_output(codec_platform_data->gpio_spk_en.gpio , 1);
		}
	}
	return val;
}

static void gpio_enable_handset_en(void)
{
	if(codec_platform_data && (codec_platform_data->gpio_handset_en.gpio != -1)) {
		if (codec_platform_data->gpio_handset_en.active_level) {
			gpio_direction_output(codec_platform_data->gpio_handset_en.gpio , 1);
		} else {
			gpio_direction_output(codec_platform_data->gpio_handset_en.gpio , 0);
		}
	}
}

static int gpio_disable_handset_en(void)
{
	int val = -1;
	if(codec_platform_data && (codec_platform_data->gpio_handset_en.gpio != -1)) {
		val = gpio_get_value(codec_platform_data->gpio_handset_en.gpio);
		if (codec_platform_data->gpio_handset_en.active_level) {
			gpio_direction_output(codec_platform_data->gpio_handset_en.gpio , 0);
		} else {
			gpio_direction_output(codec_platform_data->gpio_handset_en.gpio , 1);
			val = val == 1 ? 0 : val == 0 ? 1 : val ;
		}
	}
	return val;
}

static void gpio_select_headset_mic(void)
{
	if (codec_platform_data && codec_platform_data->gpio_buildin_mic_select.gpio != -1) {
		if (codec_platform_data->gpio_buildin_mic_select.active_level)
			gpio_direction_output(codec_platform_data->gpio_buildin_mic_select.gpio,0);
		else
			gpio_direction_output(codec_platform_data->gpio_buildin_mic_select.gpio,1);
	}
}

static void gpio_select_buildin_mic(void)
{
	if (codec_platform_data && codec_platform_data->gpio_buildin_mic_select.gpio != -1) {
		if (codec_platform_data->gpio_buildin_mic_select.active_level)
			gpio_direction_output(codec_platform_data->gpio_buildin_mic_select.gpio,1);
		else
			gpio_direction_output(codec_platform_data->gpio_buildin_mic_select.gpio,0);
	}
}

/*-----------------main fun-------------------*/
static int codec_set_board_route(struct snd_board_route *broute)
{
	int i = 0;
	int resave_hp_mute = -1;
	int resave_spk_en = -1;
	int resave_handset_en = -1;
	if (broute == NULL)
		return 0;

	/* set route */
	DUMP_ROUTE_NAME(broute->route);
	if (broute && ((cur_route == NULL) || (cur_route->route != broute->route))) {
		for (i = 0; codec_route_info[i].route_name != SND_ROUTE_NONE ; i ++)
			if (broute->route == codec_route_info[i].route_name) {
				/* set hp mute and disable speaker by gpio */
				if (broute->gpio_hp_mute_stat != KEEP_OR_IGNORE)
					resave_hp_mute = gpio_enable_hp_mute();
				if (broute->gpio_spk_en_stat != KEEP_OR_IGNORE)
					resave_spk_en = gpio_disable_spk_en();
				if (broute->gpio_handset_en_stat != KEEP_OR_IGNORE)
					resave_handset_en = gpio_disable_handset_en();
				/* set route */
				codec_set_route_base(codec_route_info[i].route_conf);
				break;
			}
		if (codec_route_info[i].route_name == SND_ROUTE_NONE) {
			printk("SET_ROUTE: codec set route error!, undecleard route, route = %d\n", broute->route);
			goto err_unclear_route;
		}
	} else
		printk("SET_ROUTE: waring: route not be setted!\n");
	if (broute->route != ROUTE_RECORD_CLEAR) {
		/* keep_old_route is used in resume part and record release */
		if (cur_route == NULL || cur_route->route == ROUTE_ALL_CLEAR) {
			keep_old_route = broute;
			//keep_old_route = &codec_platform_data->replay_def_route;
		}
		else if (cur_route->route >= ROUTE_RECORD_ROUTE_START &&
			 cur_route->route <= ROUTE_RECORD_ROUTE_END && keep_old_route != NULL) {
			/*DO NOTHING IN THIS CASE*/
		} else
			keep_old_route = cur_route;
		/* change cur_route */
		cur_route = broute;
	} else {
		if (cur_route != NULL) {
			if (cur_route->route >= ROUTE_RECORD_ROUTE_START &&
			    cur_route->route <= ROUTE_RECORD_ROUTE_END) {
				cur_route = keep_old_route;
			}
		}
	}

	/*set board gain*/
	codec_set_gain_base(broute);

	/* set gpio after set route */
	if (broute->gpio_hp_mute_stat == STATE_DISABLE ||
	    (resave_hp_mute == 0 && broute->gpio_hp_mute_stat == KEEP_OR_IGNORE))
		gpio_disable_hp_mute();
	else if (broute->gpio_hp_mute_stat == STATE_ENABLE ||
		 (resave_hp_mute == 1 && broute->gpio_hp_mute_stat == KEEP_OR_IGNORE))
		gpio_enable_hp_mute();

	if (broute->gpio_handset_en_stat == STATE_ENABLE ||
	    (resave_handset_en == 1 && broute->gpio_handset_en_stat == KEEP_OR_IGNORE))
		gpio_enable_handset_en();
	else if (broute->gpio_handset_en_stat == STATE_DISABLE ||
		 (resave_handset_en == 0 && broute->gpio_handset_en_stat == KEEP_OR_IGNORE))
		gpio_disable_handset_en();

	if (broute->gpio_spk_en_stat == STATE_ENABLE ||
	    (resave_spk_en == 1 && broute->gpio_spk_en_stat == KEEP_OR_IGNORE))
		gpio_enable_spk_en();
	else if (broute->gpio_spk_en_stat == STATE_DISABLE ||
		 (resave_spk_en == 0 && broute->gpio_spk_en_stat == KEEP_OR_IGNORE))
		gpio_disable_spk_en();

	if (broute->gpio_buildin_mic_en_stat == STATE_DISABLE){
		gpio_select_headset_mic();
		if (codec_platform_data->board_dmic_control)//add xyfu
			codec_platform_data->board_dmic_control(false);
    }else if (broute->gpio_buildin_mic_en_stat == STATE_ENABLE){
		gpio_select_buildin_mic();
		if (codec_platform_data->board_dmic_control)//add xyfu
			codec_platform_data->board_dmic_control(true);
    }

	DUMP_ROUTE_REGS("leave");

	return broute ? broute->route : (cur_route ? cur_route->route : SND_ROUTE_NONE);
err_unclear_route:
	return SND_ROUTE_NONE;
}

static int codec_set_default_route(int mode)
{
	int ret = 0;
	if (codec_platform_data) {
		if (codec_platform_data->replay_def_route.route == SND_ROUTE_NONE){
			codec_platform_data->replay_def_route.route = REPLAY_LINEOUT;
		}

		if (codec_platform_data->record_def_route.route == SND_ROUTE_NONE) {
			codec_platform_data->record_def_route.route = RECORD_MIC1_MONO_DIFF_WITH_BIAS;
		}
	}
	if (mode == CODEC_RWMODE) {
		ret =  codec_set_board_route(&codec_platform_data->replay_def_route);
	} else if (mode == CODEC_WMODE) {
		ret =  codec_set_board_route(&codec_platform_data->replay_def_route);
	} else if (mode == CODEC_RMODE){
		ret =  codec_set_board_route(&codec_platform_data->record_def_route);
	}

	return 0;
}

static struct snd_board_route tmp_broute;

static int codec_set_route(enum snd_codec_route_t route)
{
	tmp_broute.route = route;
	tmp_broute.gpio_handset_en_stat = KEEP_OR_IGNORE;
	tmp_broute.gpio_spk_en_stat = KEEP_OR_IGNORE;
	tmp_broute.gpio_hp_mute_stat = KEEP_OR_IGNORE;
	tmp_broute.gpio_buildin_mic_en_stat = STATE_DISABLE;//KEEP_OR_IGNORE add by xyfu;
	return codec_set_board_route(&tmp_broute);
}

/*----------------------------------------*/
static int codec_init(void)
{

	/* the generated IRQ is high level whit 8 SYS_CLK */
	__codec_set_int_form(ICR_INT_HIGH);


	/* set IRQ mask and clear IRQ flags*/
	__codec_set_irq_mask(ICR_COMMON_MASK);
	__codec_set_irq_flag(REG_IFR_MASK);

	/* set SYS_CLK to 12MHZ */
	__codec_set_crystal(codec_platform_data->codec_sys_clk);

	/* enable DMIC_CLK */
	__codec_set_dmic_clock(DMIC_CLK_ON);

	/* disable ADC/DAC LRSWP */
	__codec_set_adc_lrswap(LRSWAP_DISABLE);
	__codec_set_dac_lrswap(LRSWAP_DISABLE);

	if (codec_platform_data) {
		codec_set_gain_mic1(codec_platform_data->record_volume_base);
		codec_set_gain_mic2(codec_platform_data->record_volume_base);
		default_record_volume_base = codec_platform_data->record_volume_base;
	}
	/* set replay hp output gain base */
	if (codec_platform_data) {
		codec_set_gain_hp_right(codec_platform_data->replay_volume_base);
		codec_set_gain_hp_left(codec_platform_data->replay_volume_base);
		default_replay_volume_base = codec_platform_data->replay_volume_base;
	}
	/* set record digtal volume base */
	if (codec_platform_data) {
		codec_set_gain_adc_left(codec_platform_data->record_digital_volume_base);
		codec_set_gain_adc_right(codec_platform_data->record_digital_volume_base);
		default_record_digital_volume_base = codec_platform_data->record_digital_volume_base;
	}
	/* set replay digital volume base */
	if (codec_platform_data) {
		codec_set_gain_dac_left(codec_platform_data->replay_digital_volume_base);
		codec_set_gain_dac_right(codec_platform_data->replay_digital_volume_base);
		default_replay_digital_volume_base = codec_platform_data->replay_digital_volume_base;
	}

	if (codec_platform_data) {
		codec_set_gain_linein_left(codec_platform_data->bypass_l_volume_base);
		default_bypass_l_volume_base = codec_platform_data->bypass_l_volume_base;
	}

	if (codec_platform_data) {
		codec_set_gain_linein_right(codec_platform_data->bypass_r_volume_base);
		default_bypass_r_volume_base = codec_platform_data->bypass_r_volume_base;
	}

	return 0;
}

/****** codec_turn_on ********/
static int codec_turn_on(int mode)
{
	if (mode == CODEC_RWMODE) {
	} else if (mode & CODEC_WMODE) {
		gpio_enable_spk_en();
	} else if (mode & CODEC_RMODE) {
	}

	return 0;
}
/****** codec_turn_off ********/
static int codec_turn_off(int mode)
{
	int ret;
	struct snd_board_route *route = keep_old_route;

	if (mode == CODEC_RWMODE) {
		printk("JZ CODEC: Close REPLAY & RECORD\n");
#if 1
		/* shutdown sequence */
		__codec_enable_dac_mute();
		codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GDO, 100,__LINE__);
		__codec_set_irq_flag(1 << IFR_GDO);
		__codec_switch_sb_hp(POWER_OFF);
		codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_RDO, 100,__LINE__);
		__codec_set_irq_flag(1 << IFR_RDO);
		__codec_enable_hp_mute();
		udelay(500);
		__codec_switch_sb_dac(POWER_OFF);
		udelay(500);
		__codec_switch_sb_sleep(POWER_OFF);
		codec_sleep(10);
		__codec_switch_sb(POWER_OFF);
		codec_sleep(10);
#endif
	} else if (mode & CODEC_WMODE) {
		printk("JZ CODEC: Close REPLAY\n");
		gpio_disable_spk_en();
	} else if (mode & CODEC_RMODE) {
		printk("JZ CODEC: Close RECORD\n");
		ret = codec_set_route(ROUTE_RECORD_CLEAR);
		if(ret != ROUTE_RECORD_CLEAR)
		{
			printk("JZ CODEC: codec_turn_off_part record mode error!\n");
			return -1;
		}

		ret = codec_set_board_route(route);
		if(ret != route->route)
		{
			printk("JZ CODEC: %s record mode error!\n", __func__);
			return -1;
		}
	}

	return 0;
}

/****** codec_shutdown *******/
static int codec_shutdown(void)
{
	return 0;
}

/****** codec_reset **********/
static int codec_reset(void)
{
	/* select serial interface and work mode of adc and dac */
	__codec_select_adc_digital_interface(SERIAL_INTERFACE);
	__codec_select_dac_digital_interface(SERIAL_INTERFACE);

	__codec_select_adc_work_mode(I2S_MODE);
	__codec_select_dac_work_mode(I2S_MODE);

	/* reset codec ready for set route */
	codec_set_route_ready(ROUTE_READY_FOR_DAC);

	return 0;
}

/******** codec_anti_pop ********/
static int codec_anti_pop(int mode)
{
	codec_set_route_ready(CODEC_WMODE);
	switch(mode) {
	case CODEC_RWMODE:
	case CODEC_RMODE:
		break;
	case CODEC_WMODE:
		__codec_switch_sb_dac(POWER_ON);
		udelay(500);
		//i2s_replay_zero_for_flush_codec();
		break;
	}
	return 0;
}

/******** codec_suspend ************/
static int codec_suspend(void)
{
	int ret = 10;

	g_codec_sleep_mode = 0;

	ret = codec_set_route(ROUTE_ALL_CLEAR);
	if(ret != ROUTE_ALL_CLEAR)
	{
		printk("JZ CODEC: codec_suspend_part error!\n");
		return 0;
	}

	__codec_switch_sb_sleep(POWER_OFF);
	codec_sleep(10);
	__codec_switch_sb(POWER_OFF);

	return 0;
}

static int codec_resume(void)
{
	int ret,tmp_route = 0;

	if (keep_old_route) {
		tmp_route = keep_old_route->route;
		ret = codec_set_board_route(keep_old_route);
		if(ret != tmp_route) {
			printk("JZ CODEC: codec_resume_part error!\n");
			return 0;
		}
	} else {
		__codec_switch_sb(POWER_ON);
		codec_sleep(250);
		__codec_switch_sb_sleep(POWER_ON);
		/*
		 * codec_sleep(400);
		 * this time we ignored becase other device resume waste time
		 */
	}

	g_codec_sleep_mode = 1;

	return 0;
}

/*---------------------------------------*/

/**
 * CODEC set device
 *
 * this is just a demo function, and it will be use as default
 * if it is not realized depend on difficent boards
 *
 */
static int codec_set_device(enum snd_device_t device)
{
	int ret = 0;
	int iserror = 0;


	printk("codec_set_device %d \n",device);
	switch (device) {
	case SND_DEVICE_DEFAULT:
		if (codec_platform_data && codec_platform_data->replay_def_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->replay_def_route));
			if(ret != codec_platform_data->replay_def_route.route) {
				return -1;
			}
		}
		break;
	case SND_DEVICE_HEADSET:
	case SND_DEVICE_HEADPHONE:
		if (codec_platform_data && codec_platform_data->replay_headset_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->replay_headset_route));
			if(ret != codec_platform_data->replay_headset_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_SPEAKER:
		if (codec_platform_data && codec_platform_data->replay_speaker_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->replay_speaker_route));
			if(ret != codec_platform_data->replay_speaker_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_HEADSET_AND_SPEAKER:
		if (codec_platform_data && codec_platform_data->replay_headset_and_speaker_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->replay_headset_and_speaker_route));
			if(ret != codec_platform_data->replay_headset_and_speaker_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_CALL_HEADPHONE:
	case SND_DEVICE_CALL_HEADSET:
		if (codec_platform_data && codec_platform_data->call_headset_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->call_headset_route));
			if(ret != codec_platform_data->call_headset_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_CALL_HANDSET:
		if (codec_platform_data && codec_platform_data->call_handset_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->call_handset_route));
			if(ret != codec_platform_data->call_handset_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_CALL_SPEAKER:
		if (codec_platform_data && codec_platform_data->call_speaker_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->call_speaker_route));
			if(ret != codec_platform_data->call_speaker_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_BUILDIN_MIC:
		if (codec_platform_data && codec_platform_data->record_buildin_mic_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->record_buildin_mic_route));
			if (ret != codec_platform_data->record_buildin_mic_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_HEADSET_MIC:
		if (codec_platform_data && codec_platform_data->record_headset_mic_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->record_headset_mic_route));
			if (ret != codec_platform_data->record_headset_mic_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_BUILDIN_RECORD_INCALL:
		if (codec_platform_data && codec_platform_data->record_buildin_incall_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->record_buildin_incall_route));
			if(ret != codec_platform_data->record_buildin_incall_route.route) {
				return -1;
			}
		}
		break;
	case SND_DEVICE_HEADSET_RECORD_INCALL:
		if (codec_platform_data && codec_platform_data->record_headset_incall_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->record_headset_incall_route));
			if(ret != codec_platform_data->record_headset_incall_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_BT:
		if (codec_platform_data && codec_platform_data->bt_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->bt_route));
			if(ret != codec_platform_data->bt_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_LOOP_TEST:
		if (codec_platform_data && codec_platform_data->replay_loop_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->replay_loop_route));
			if(ret != codec_platform_data->replay_loop_route.route) {
				return -1;
			}
		}
		break;
	case SND_DEVICE_LINEIN_RECORD:
		if (codec_platform_data && codec_platform_data->record_linein_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->record_linein_route));
			if(ret != codec_platform_data->record_linein_route.route) {
				return -1;
			}
		}
		break;

	case SND_DEVICE_LINEIN1_RECORD:
		if (codec_platform_data && codec_platform_data->record_linein1_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->record_linein1_route));
			if(ret != codec_platform_data->record_linein1_route.route) {
				return -1;
			}
		}
		break;
	case SND_DEVICE_LINEIN2_RECORD:
		if (codec_platform_data && codec_platform_data->record_linein2_route.route) {
			ret = codec_set_board_route(&(codec_platform_data->record_linein2_route));
			if(ret != codec_platform_data->record_linein2_route.route) {
				return -1;
			}
		}
		break;
	case SND_DEVICE_LINEIN3_RECORD:
		ret = -1;
		printk("JZ CODEC: our internal codec not have linein3!\n");
		break;
	default:
		iserror = 1;
		printk("JZ CODEC: Unkown ioctl argument %d in SND_SET_DEVICE\n",device);
	};

	return ret;
}

/*---------------------------------------*/

/**
 * CODEC set standby
 *
 * this is just a demo function, and it will be use as default
 * if it is not realized depend on difficent boards
 *
 */

static int codec_set_standby(unsigned int sw)
{
	return 0;
}

/*---------------------------------------*/
/**
 * CODEC set record rate & data width & volume & channel
 *
 */

static int codec_set_record_rate(unsigned long *rate)
{
	int speed = 0, val;
	unsigned long mrate[MAX_RATE_COUNT] = {
		96000, 48000, 44100, 32000, 24000,
		22050, 16000, 12000, 11025, 8000
	};

	for (val = 0; val < MAX_RATE_COUNT; val++) {
		if (*rate >= mrate[val]) {
			speed = val;
			break;
		}
	}
	if (*rate < mrate[MAX_RATE_COUNT - 1]) {
		speed = MAX_RATE_COUNT - 1;
	}
	__codec_select_adc_samp_rate(speed);
	if (val < MAX_RATE_COUNT && val >=0 )
		*rate = mrate[val];
	else
		*rate = 8000;
	return 0;
}

static int codec_set_record_data_width(int width)
{
	int supported_width[4] = {16, 18, 20, 24};
	int fix_width;

	for(fix_width = 0; fix_width < 4; fix_width ++)
	{
		if (width <= supported_width[fix_width])
			break;
	}
	if (fix_width == 4)
		return -EINVAL;

	__codec_select_adc_word_length(fix_width);

	return 0;
}

static int codec_set_record_volume(int *val)
{
	int val_tmp = *val;
	*val = codec_set_gain_adc_left(val_tmp);
	val_tmp = *val;
	*val = codec_set_gain_adc_right(val_tmp);
	return 0;
}

static int codec_set_mic_volume(int* val)
{
#ifdef CONFIG_ANDRIO
#ifndef CONFIG_SOUND_XBURST_DEBUG
	/*just set analog gm1 and gm2*/
	int fixed_vol;
	int volume_base;

	if (codec_platform_data &&
	    codec_platform_data->record_volume_base >= 0 &&
	    codec_platform_data->record_volume_base <=20)
	{
		volume_base = codec_platform_data->record_volume_base;

		fixed_vol = (volume_base >> 2) +
			((5 - (volume_base >> 2)) * (*val) / 100);
	}
	else
		fixed_vol = (5 * (*val) / 100);

	__codec_set_gm1(fixed_vol);
	__codec_set_gm2(fixed_vol);
	user_record_volume = *val;
	return *val;
#endif
#endif
	codec_set_gain_mic1(*val);
	codec_set_gain_mic2(*val);
	return *val;
}

static int codec_set_record_channel(int *channel)
{
	if (*channel != 1)
		*channel = 2;
	return 1;
}

/*---------------------------------------*/
/**
 * CODEC set replay rate & data width & volume & channel
 *
 */

static int codec_set_replay_rate(unsigned long *rate)
{
	int speed = 0, val;
	unsigned long mrate[MAX_RATE_COUNT] = {
		96000, 48000, 44100, 32000, 24000,
		22050, 16000, 12000, 11025, 8000
	};
	for (val = 0; val < MAX_RATE_COUNT; val++) {
		if (*rate >= mrate[val]) {
			speed = val;
			break;
		}
	}
	if (*rate < mrate[MAX_RATE_COUNT - 1]) {
		speed = MAX_RATE_COUNT - 1;
	}

	__codec_select_dac_samp_rate(speed);

	if (val >=0 && val< MAX_RATE_COUNT)
		*rate = mrate[val];
	else
		*rate = 8000;
	return 0;
}

static int codec_set_replay_data_width(int width)
{
	int supported_width[4] = {16, 18, 20, 24};
	int fix_width;

	for(fix_width = 0; fix_width < 4; fix_width ++)
	{
		if (width <= supported_width[fix_width])
			break;
	}

	if (fix_width == 4)
		return -EINVAL;

	__codec_select_dac_word_length(fix_width);

	return 0;
}

/*---------------------------------------*/
/**
 * CODEC set mute
 *
 * set dac mute used for anti pop
 *
 */
static int codec_mute(int val,int mode)
{
	if (mode & CODEC_WMODE) {
		if(val){
			if(!__codec_get_dac_mute()){
				/* enable dac mute */
				__codec_set_irq_flag(1 << IFR_GDO);
				__codec_enable_dac_mute();
				codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GDO, 100,__LINE__);
			}
		} else {
			if(__codec_get_dac_mute() && user_replay_volume){
				/* disable dac mute */
				__codec_set_irq_flag(1 << IFR_GUP);
				__codec_disable_dac_mute();
				codec_sleep_wait_bitset(CODEC_REG_IFR, IFR_GUP, 100,__LINE__);
			}
		}
	}
	if (mode & CODEC_RMODE) {
		/*JZ4775 not support adc mute*/
	}

	return 0;
}

static int codec_set_replay_volume(int *val)
{
	/*just set analog gol and gor*/
	unsigned long fixed_vol;
	int volume_base = 0;

	/* get current volume */
        if (*val < 0){
                *val = user_replay_volume;
                return *val;
        }

	if (codec_platform_data &&
	    codec_platform_data->replay_volume_base >= -25 &&
	    codec_platform_data->replay_volume_base <= 6)
		volume_base = codec_platform_data->replay_volume_base;

	fixed_vol = (6 - volume_base) + ((25 + volume_base) * (100 -(*val))/ 100);

	__codec_set_gol(fixed_vol);
	__codec_set_gor(fixed_vol);

	user_replay_volume = *val;

        if(*val == 0){
                codec_mute(1 ,CODEC_WMODE);
        }else{
                codec_mute(0 ,CODEC_WMODE);
        }

	return *val;
}

static int codec_set_replay_channel(int* channel)
{
	if (*channel != 1)
		*channel = 2;
	return 0;
}

/*---------------------------------------*/
static int codec_debug_routine(void *arg)
{
	return 0;
}

/**
 * CODEC short circut handler
 *
 * To protect CODEC, CODEC will be shutdown when short circut occured.
 * Then we have to restart it.
 */
static inline void codec_short_circut_handler(void)
{
	int	curr_hp_left_vol;
	int	curr_hp_right_vol;
	unsigned int	load_flag = 0;
	unsigned int	delay;
#define VOL_DELAY_BASE 22               //per VOL delay time in ms
	curr_hp_left_vol = codec_get_gain_hp_left();
	curr_hp_right_vol = codec_get_gain_hp_right();
	/* delay */
	delay = VOL_DELAY_BASE * (25 + (curr_hp_left_vol + curr_hp_right_vol)/2);
	/* set hp gain to min */
	codec_set_gain_hp_left(-25);
	codec_set_gain_hp_right(-25);

	/* set 16ohm load to keep away from the bug can not waited RDO */
	if(__codec_get_load() == LOAD_10KOHM)
	{
		__codec_set_16ohm_load();
		load_flag = 1;
	}
	//clear rdo rup
	__codec_clear_rdo();
	__codec_clear_rup();

	/* turn off sb_hp */
	//codec_set_hp(HP_DISABLE);
	__codec_switch_sb_sleep(POWER_OFF);
	codec_sleep(10);
	__codec_switch_sb(POWER_OFF);
	codec_sleep(100);
	printk("JZ CODEC: Short circut detected! restart CODEC.\n");
/* Updata SCMC/SCLR */
	__codec_set_irq_flag((1 << IFR_SCMC));
	__codec_set_irq_flag((1 << IFR_SCLR));
	codec_sleep(10);
	__codec_switch_sb(POWER_ON);
	codec_sleep(250);
	__codec_switch_sb_sleep(POWER_ON);
	codec_sleep(400);
	//codec_set_hp(HP_ENABLE_CAP_LESS);

	if(load_flag)
		__codec_set_10kohm_load();

	/* restore hp gain */
	codec_set_gain_hp_left(curr_hp_left_vol);
	codec_set_gain_hp_right(curr_hp_right_vol);
	codec_sleep(delay);

	printk("JZ CODEC: Short circut restart CODEC hp out finish.\n");
}

static int codec_irq_handle(struct work_struct *detect_work)
{
	unsigned char codec_ifr;
#if defined(CONFIG_JZ_HP_DETECT_CODEC)
	int old_status = 0;
	int new_status = 0;
	int i;
#endif

	codec_ifr = __codec_get_irq_flag();
	/* Mask all irq temporarily */
	if ((codec_ifr & (~ICR_COMMON_MASK & ICR_ALL_MASK))) {
		do {
			if (codec_ifr & (1 << IFR_SCLR))
				codec_short_circut_handler();
			codec_ifr = __codec_get_irq_flag();
		} while(codec_ifr & ((1 << IFR_SCMC) | (1 << IFR_SCLR)));

#if defined(CONFIG_JZ_HP_DETECT_CODEC)
		if (codec_ifr & (1 << IFR_JACK_EVENT)) {
		_ensure_stable:
			old_status = ((__codec_get_sr() & CODEC_JACK_MASK) != 0);
			/* Read status at least 3 times to make sure it is stable. */
			for (i = 0; i < 3; ++i) {
				old_status = ((__codec_get_sr() & CODEC_JACK_MASK) != 0);
				codec_sleep(50);
			}
		}
		__codec_set_irq_flag(codec_ifr);

		codec_ifr = __codec_get_irq_flag();
		codec_sleep(10);

		/* If the jack status has changed, we have to redo the process. */
		if (codec_ifr & (1 << IFR_JACK_EVENT)) {
			codec_sleep(50);
			new_status = ((__codec_get_sr() & CODEC_JACK_MASK) != 0);
			if (new_status != old_status) {
				goto _ensure_stable;
			}
		}

		/* Report status */
		if(!work_pending(detect_work))
			schedule_work(detect_work);
#endif
	}

	__codec_set_irq_flag((~0));
	/* Unmask SCMC & JACK (ifdef CONFIG_JZ_HP_DETECT_CODEC) */
	__codec_set_irq_mask(ICR_COMMON_MASK);

	return 0;
}

static int codec_get_hp_state(int *state)
{
#if defined(CONFIG_JZ_HP_DETECT_CODEC)
	*state = ((__codec_get_sr() & CODEC_JACK_MASK) >> SR_JACK) ^
		(!codec_platform_data->hpsense_active_level);
	if (*state < 0)
		return -EIO;
#elif defined(CONFIG_JZ_HP_DETECT_GPIO)
	if(codec_platform_data &&
	   (codec_platform_data->gpio_hp_detect.gpio != -1)) {
		*state  = __gpio_get_value(codec_platform_data->gpio_hp_detect.gpio);
	}
	else
		return -EIO;
#endif
	return 0;
}

static void codec_get_format_cap(unsigned long *format)
{
	*format = AFMT_S24_LE|AFMT_U24_LE|AFMT_U16_LE|AFMT_S16_LE|AFMT_S8|AFMT_U8;
}

static void codec_debug_default(void)
{
	int ret = 4;
	while(ret--) {
		gpio_disable_spk_en();
		printk("disable %d\n",ret);
		mdelay(10000);
		gpio_enable_spk_en();
		printk("enable %d\n",ret);
		mdelay(10000);
	}
	ret = 4;
	while(ret--) {
		codec_set_hp(HP_DISABLE);
		gpio_disable_spk_en();
		printk("disable good %d\n",ret);
		mdelay(10000);
		gpio_enable_spk_en();
		printk("enable1 good %d\n",ret);
		mdelay(10000);
		printk("enable end %d\n",ret);
		mdelay(100);
		codec_set_hp(HP_ENABLE_CAP_LESS);
		mdelay(10000);
	}
}
static void codec_debug(char arg)
{
	switch(arg) {
		/*...*/
	case '0':
	default:
		codec_debug_default();
	}
}

/***************************************************************************************\
 *                                                                                     *
 *control interface                                                                    *
 *                                                                                     *
 \***************************************************************************************/
/**
 * CODEC ioctl (simulated) routine
 *
 * Provide control interface for i2s driver
 */
static int jzcodec_ctl(unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	DUMP_IOC_CMD("enter");
	{
		switch (cmd) {

		case CODEC_INIT:
			ret = codec_init();
			break;

		case CODEC_TURN_ON:
			ret = codec_turn_on(arg);
			break;

		case CODEC_TURN_OFF:
			ret = codec_turn_off(arg);
			break;

		case CODEC_SHUTDOWN:
			ret = codec_shutdown();
			break;

		case CODEC_RESET:
			ret = codec_reset();
			break;

		case CODEC_SUSPEND:
			ret = codec_suspend();
			break;

		case CODEC_RESUME:
			ret = codec_resume();
			break;

		case CODEC_ANTI_POP:
			ret = codec_anti_pop((int)arg);
			break;

		case CODEC_SET_DEFROUTE:
			ret = codec_set_default_route((int )arg);
			break;

		case CODEC_SET_DEVICE:
			ret = codec_set_device(*(enum snd_device_t *)arg);
			break;

		case CODEC_SET_STANDBY:
			ret = codec_set_standby(*(unsigned int *)arg);
			break;

		case CODEC_SET_RECORD_RATE:
			ret = codec_set_record_rate((unsigned long *)arg);
			break;

		case CODEC_SET_RECORD_DATA_WIDTH:
			ret = codec_set_record_data_width((int)arg);
			break;

		case CODEC_SET_MIC_VOLUME:
			ret = codec_set_mic_volume((int *)arg);
			break;

		case CODEC_SET_RECORD_VOLUME:
			ret = codec_set_record_volume((int *)arg);
			break;

		case CODEC_SET_RECORD_CHANNEL:
			ret = codec_set_record_channel((int*)arg);
			break;

		case CODEC_SET_REPLAY_RATE:
			ret = codec_set_replay_rate((unsigned long*)arg);
			break;

		case CODEC_SET_REPLAY_DATA_WIDTH:
			ret = codec_set_replay_data_width((int)arg);
			break;

		case CODEC_SET_REPLAY_VOLUME:
			ret = codec_set_replay_volume((int*)arg);
			break;

		case CODEC_SET_REPLAY_CHANNEL:
			ret = codec_set_replay_channel((int*)arg);
			break;

		case CODEC_GET_REPLAY_FMT_CAP:
		case CODEC_GET_RECORD_FMT_CAP:
			codec_get_format_cap((unsigned long *)arg);
			break;

		case CODEC_DAC_MUTE:
			ret = codec_mute((int)arg,CODEC_WMODE);
			break;
		case CODEC_ADC_MUTE:
			ret = codec_mute((int)arg,CODEC_RMODE);
			break;
		case CODEC_DEBUG_ROUTINE:
			ret = codec_debug_routine((void *)arg);
			break;

		case CODEC_IRQ_HANDLE:
			ret = codec_irq_handle((struct work_struct*)arg);
			break;

		case CODEC_GET_HP_STATE:
			ret = codec_get_hp_state((int*)arg);
			break;

		case CODEC_DUMP_REG:
			dump_codec_regs();
		case CODEC_DUMP_GPIO:
			dump_gpio_state();
			ret = 0;
			break;
		case CODEC_CLR_ROUTE:
			if ((int)arg == CODEC_RMODE)
				codec_set_route(ROUTE_RECORD_CLEAR);
			else if ((int)arg == CODEC_WMODE)
				codec_set_route(ROUTE_REPLAY_CLEAR);
			else {
				codec_set_route(ROUTE_ALL_CLEAR);
			}
			ret = 0;
			break;
		case CODEC_DEBUG:
			codec_debug((char)arg);
			ret = 0;
			break;
		default:
			printk("JZ CODEC:%s:%d: Unkown IOC commond\n", __FUNCTION__, __LINE__);
			ret = -1;
		}
	}

	DUMP_IOC_CMD("leave");
	return ret;
}

static int jz_codec_probe(struct platform_device *pdev)
{
	codec_platform_data = pdev->dev.platform_data;

	codec_platform_data->codec_sys_clk = SYS_CLK_12M;

#if defined(CONFIG_JZ_HP_DETECT_CODEC_V12)
	jz_set_hp_detect_type(SND_SWITCH_TYPE_CODEC,NULL,
			      &codec_platform_data->gpio_mic_detect,
			      &codec_platform_data->gpio_mic_detect_en,
			      &codec_platform_data->gpio_buildin_mic_select,
			      codec_platform_data->hook_active_level);
#elif  defined(CONFIG_JZ_HP_DETECT_GPIO_V12)
	jz_set_hp_detect_type(SND_SWITCH_TYPE_GPIO,
			      &codec_platform_data->gpio_hp_detect,
			      &codec_platform_data->gpio_mic_detect,
			      &codec_platform_data->gpio_mic_detect_en,
			      &codec_platform_data->gpio_buildin_mic_select,
			      codec_platform_data->hook_active_level);
#endif
	if (codec_platform_data->gpio_mic_detect.gpio != -1 )
		if (gpio_request(codec_platform_data->gpio_mic_detect.gpio,"gpio_mic_detect") < 0) {
			gpio_free(codec_platform_data->gpio_mic_detect.gpio);
			gpio_request(codec_platform_data->gpio_mic_detect.gpio,"gpio_mic_detect");
		}

	if (codec_platform_data->gpio_buildin_mic_select.gpio != -1 )
		if (gpio_request(codec_platform_data->gpio_buildin_mic_select.gpio,"gpio_buildin_mic_switch") < 0) {
			gpio_free(codec_platform_data->gpio_buildin_mic_select.gpio);
			gpio_request(codec_platform_data->gpio_buildin_mic_select.gpio,"gpio_buildin_mic_switch");
		}

	if (codec_platform_data->gpio_mic_detect_en.gpio != -1 &&
	    codec_platform_data->gpio_buildin_mic_select.gpio !=
	    codec_platform_data->gpio_mic_detect_en.gpio)
		/*many times gpio_mic_detect_en is equel to gpio_buildin_mic_select*/
		if (gpio_request(codec_platform_data->gpio_mic_detect_en.gpio,"gpio_mic_detect_en") < 0) {
			gpio_free(codec_platform_data->gpio_mic_detect_en.gpio);
			gpio_request(codec_platform_data->gpio_mic_detect_en.gpio,"gpio_mic_detect_en");
		}

	if (codec_platform_data->gpio_hp_mute.gpio != -1 )
		if (gpio_request(codec_platform_data->gpio_hp_mute.gpio,"gpio_hp_mute") < 0) {
			gpio_free(codec_platform_data->gpio_hp_mute.gpio);
			gpio_request(codec_platform_data->gpio_hp_mute.gpio,"gpio_hp_mute");
		}
	if (codec_platform_data->gpio_spk_en.gpio != -1 )
		if (gpio_request(codec_platform_data->gpio_spk_en.gpio,"gpio_spk_en") < 0) {
			gpio_free(codec_platform_data->gpio_spk_en.gpio);
			gpio_request(codec_platform_data->gpio_spk_en.gpio,"gpio_spk_en");
		}
	if (codec_platform_data->gpio_handset_en.gpio != -1 )
		if (gpio_request(codec_platform_data->gpio_handset_en.gpio,"gpio_handset_en") < 0) {
			gpio_free(codec_platform_data->gpio_handset_en.gpio);
			gpio_request(codec_platform_data->gpio_handset_en.gpio,"gpio_handset_en");
		}

	return 0;
}

static int jz_codec_remove(struct platform_device *pdev)
{
	gpio_free(codec_platform_data->gpio_hp_mute.gpio);
	gpio_free(codec_platform_data->gpio_spk_en.gpio);
	codec_platform_data = NULL;

	return 0;
}

static struct platform_driver jz_codec_driver = {
	.probe		= jz_codec_probe,
	.remove		= jz_codec_remove,
	.driver		= {
		.name	= "jz_codec",
		.owner	= THIS_MODULE,
	},
};

void codec_irq_set_mask(void)
{
	__codec_set_irq_mask(ICR_ALL_MASK);
	//__codec_set_irq_mask2(ICR_ALL_MASK2);
};

/***************************************************************************************\
 *module init                                                                          *
\***************************************************************************************/
/**
 * Module init
 */
#define JZ4780_INTERNAL_CODEC_CLOCK 12000000
static int __init init_codec(void)
{
	int retval;

	i2s_register_codec("internal_codec", (void *)jzcodec_ctl,JZ4780_INTERNAL_CODEC_CLOCK,CODEC_MASTER);
	retval = platform_driver_register(&jz_codec_driver);
	if (retval) {
		printk("JZ CODEC: Could net register jz_codec_driver\n");
		return retval;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	early_suspend.suspend = codec_early_suspend;
	early_suspend.resume = codec_late_resume;
	early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&early_suspend);
#endif


	return 0;
}

/**
 * Module exit
 */
static void __exit cleanup_codec(void)
{
	platform_driver_unregister(&jz_codec_driver);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&early_suspend);
#endif
}
arch_initcall(init_codec);
module_exit(cleanup_codec);
