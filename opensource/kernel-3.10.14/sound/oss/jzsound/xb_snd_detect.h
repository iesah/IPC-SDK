#ifndef __XB_SND_DETECT_H__
#define __XB_SND_DETECT_H__

#include <linux/workqueue.h>
#include <linux/switch.h>
#include <linux/wait.h>

/*detect id and name*/
#define DEV_DSP_HP_DET_NAME     "hp_detect"
#define DEV_DSP_DOCK_DET_NAME   "dock_detect"

#define SND_DEV_DETECT0_ID  0
#define SND_DEV_DETECT1_ID  1
#define SND_DEV_DETECT2_ID  2
#define SND_DEV_DETECT3_ID  3

#define SND_SWITCH_TYPE_GPIO    0x1
#define SND_SWITCH_TYPE_CODEC   0x2

enum {
	LOW_VALID =0,
	HIGH_VALID,
	INVALID,
};

enum mic_route{
	CODEC_HEADSET_ROUTE,
	CODEC_HEADPHONE_ROUTE,
};

struct snd_switch_data {
	struct switch_dev sdev;
	wait_queue_head_t wq;
	int type;
	const char *name_headset_on;
	const char *name_headphone_on;
	const char *name_off;
	const char *state_headset_on;
	const char *state_headphone_on;
	const char *state_off;
	int hp_irq;
	int hook_irq;
	struct work_struct hp_work;
	struct work_struct hook_work;
	struct workqueue_struct *check_state_workqueue;
	struct input_dev *inpdev;
	int hp_gpio;
	int hp_valid_level;

	/*mic detect and select*/
	int mic_gpio;
	int	mic_vaild_level;

	int hook_gpio;
	int hook_valid_level;

	int mic_detect_en_gpio;
	int mic_detect_en_level;

	int mic_select_gpio;
	int mic_select_level;

	int (*codec_get_sate)(void);
	int (*set_device)(unsigned long device);
	atomic_t flag;
	int hook_pressed;
	int hp_state;
	/* *********for update******** */
	void *priv_data;
	int (*codec_get_state_2)(struct snd_switch_data * switch_data);
	int (*set_device_2)(struct snd_switch_data *switch_data, unsigned long device);
	/* ********for updtate end*****/
};

#endif /*__XB_SND_DETECT_H__*/
