#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/errno.h>

#include <mach/jzsnd.h>

#include "interface/xb_snd_dsp.h"
#include "xb_snd_detect.h"

//#define DETECT_HOOK_INT_FAST    50
//#define DETECT_HOOK_VOL_MAX     200

//#define OPEN_PRINTK_DEBUG
#ifdef OPEN_PRINTK_DEBUG
#define SWITCH_DEBUG(format, ...) printk(format, ## __VA_ARGS__)
#else
#define	SWITCH_DEBUG(format, ...) do{ } while(0)
#endif


static void snd_switch_set_state(struct snd_switch_data *switch_data, int state)
{
	if (switch_data->hp_state  != state) {
		switch_set_state(&switch_data->sdev, state);
		switch_data->hp_state = state;
	}

	if (switch_data->type == SND_SWITCH_TYPE_GPIO) {
		if (switch_data->hp_valid_level == HIGH_VALID) {
			if (state) {
				//irq_set_irq_type(switch_data->irq, IRQF_TRIGGER_FALLING);
				irq_set_irq_type(switch_data->hp_irq, IRQF_TRIGGER_LOW);
			} else {
				//irq_set_irq_type(switch_data->irq, IRQF_TRIGGER_RISING);
				irq_set_irq_type(switch_data->hp_irq, IRQF_TRIGGER_HIGH);
			}
		} else if (switch_data->hp_valid_level == LOW_VALID) {
			if (state) {
				irq_set_irq_type(switch_data->hp_irq, IRQF_TRIGGER_HIGH);
				//irq_set_irq_type(switch_data->irq, IRQF_TRIGGER_RISING);
			} else {
				//irq_set_irq_type(switch_data->irq, IRQF_TRIGGER_FALLING);
				irq_set_irq_type(switch_data->hp_irq, IRQF_TRIGGER_LOW);
			}
		}
	}
}

static void snd_switch_work(struct work_struct *hp_work)
{
	int state = 0;
	int tmp_state =0;
	int i = 0;
#ifndef CONFIG_ANDROID
	int ret = 0;
	int device;
#endif

	struct snd_switch_data *switch_data =
		container_of(hp_work, struct snd_switch_data, hp_work);


	/* if gipo switch */
	if (switch_data->type == SND_SWITCH_TYPE_GPIO) {
		//__gpio_disable_pull(switch_data->hp_gpio);
		state = gpio_get_value(switch_data->hp_gpio);
		for (i = 0; i < 5; i++) {
			msleep(20);
			//__gpio_disable_pull(data->hp_gpio);
			tmp_state = gpio_get_value(switch_data->hp_gpio);
			if (tmp_state != state) {
				i = -1;
				//__gpio_disable_pull(data->hp_gpio);
				state = gpio_get_value(switch_data->hp_gpio);
				continue;
			}
		}

		if (state == (int)switch_data->hp_valid_level){
			state = 1;
		}else{
			state = 0;
		}
		enable_irq(switch_data->hp_irq);
	}

	/* if codec internal hpsense */
	if (switch_data->type == SND_SWITCH_TYPE_CODEC) {
		if(switch_data->codec_get_sate) {
			state = switch_data->codec_get_sate();
		} else if(switch_data->codec_get_state_2) {
			state = switch_data->codec_get_state_2(switch_data);
		}

	}

	if (state == 1 && switch_data->mic_detect_en_gpio != -1){
		gpio_direction_output(switch_data->mic_detect_en_gpio, switch_data->mic_detect_en_level);
		mdelay(1000);
	}

	if (state == 1 && switch_data->mic_gpio != -1) {
		gpio_direction_input(switch_data->mic_gpio);
		if (gpio_get_value(switch_data->mic_gpio) != switch_data->mic_vaild_level)
			state <<= 1;
		else
			state <<= 0;
	} else
		state <<= 1;

	if (state == 1) {
		if (switch_data->mic_select_gpio != -1)
			gpio_direction_output(switch_data->mic_select_gpio, !switch_data->mic_select_level);
		if (atomic_read(&switch_data->flag) == 0 && switch_data->hook_valid_level != -1) {
			enable_irq(switch_data->hook_irq);
			SWITCH_DEBUG("==========hp work enable irq========\n");
			atomic_set(&switch_data->flag,1);
		}


	} else {
		if (switch_data->mic_select_gpio != -1 )
			gpio_direction_output(switch_data->mic_select_gpio, switch_data->mic_select_level);
		if(atomic_read(&switch_data->flag) == 1 && switch_data->hook_valid_level != -1){
			disable_irq(switch_data->hook_irq);
			SWITCH_DEBUG("==========hp work disable irq========\n");
			atomic_set(&switch_data->flag,0);
		}
	}
    SWITCH_DEBUG("%s,%d,%d\n",__func__,__LINE__,state);
	snd_switch_set_state(switch_data, state);

#ifndef CONFIG_ANDROID
	if (state == 1) {
		device = SND_DEVICE_HEADSET;
		if(switch_data->set_device) {
			ret = switch_data->set_device((unsigned long)&device);
		} else if(switch_data->set_device_2) {
			ret = switch_data->set_device_2(switch_data, (unsigned long)&device);
		}
		if (ret == -1)
			printk("hp inser but dsp not open\n");
		else if (ret < -1)
			printk(" set_device failed in the hp changed!\n");
	} else if (state == 2) {
		device = SND_DEVICE_HEADPHONE;
		if(switch_data->set_device) {
			ret = switch_data->set_device((unsigned long)&device);
		} else if(switch_data->set_device_2) {
			ret = switch_data->set_device_2(switch_data, (unsigned long)&device);
		}
		if (ret == -1)
			printk("hp inser but dsp not open\n");
		else if (ret < -1)
			printk(" set_device failed in the hp changed!\n");
	} else {
		device = SND_DEVICE_DEFAULT;
		if(switch_data->set_device) {
			ret = switch_data->set_device((unsigned long)&device);
		} else if(switch_data->set_device_2) {
			ret = switch_data->set_device_2(switch_data, (unsigned long)&device);
		}
		if (ret == -1)
			printk("hp remove but dsp not open\n");
		else if (ret < -1)
			printk(" set_device failed in the hp changed!\n");
	}
#endif
}
static void hook_do_work(struct work_struct *hook_work)
{
	int state = 0;
	int tmp_state =0;
	int i = 0;
	//int data = -1;
	//int detect_hook_time = DETECT_HOOK_INT_FAST;

	struct snd_switch_data *switch_data =
		container_of(hook_work, struct snd_switch_data, hook_work);
	switch_data->hook_pressed = 0;

	state = gpio_get_value(switch_data->hook_gpio);
	for (i = 0; i < 5; i++) {
		mdelay(10);
		tmp_state = gpio_get_value(switch_data->hook_gpio);
		if (tmp_state != state) {
			i = -1;
			state = gpio_get_value(switch_data->hook_gpio);
			continue;
		}
	}
	if(state == 0){
		goto out;
	}

	while (1) {
		if (state == gpio_get_value(switch_data->hook_gpio)) {
		    msleep(100);
			switch_data->hook_pressed++;
		} else {
			if (switch_data->hook_pressed <= 7 && switch_data->hook_pressed > 0) {
				SWITCH_DEBUG("===== hook key pressed at %d========\n",switch_data->hook_pressed);
				input_report_key(switch_data->inpdev, KEY_MEDIA, 1);
				input_sync(switch_data->inpdev);
				msleep(100);
				input_report_key(switch_data->inpdev, KEY_MEDIA, 0);
				input_sync(switch_data->inpdev);
			} else if (switch_data->hook_pressed > 7) {
				SWITCH_DEBUG("===== hook key pressed at %d====\n", switch_data->hook_pressed);
				input_report_key(switch_data->inpdev, KEY_END, 1);
				input_sync(switch_data->inpdev);
				msleep(100);
				input_report_key(switch_data->inpdev, KEY_END, 0);
				input_sync(switch_data->inpdev);
			}
			break;

		}
	}


out:
	irq_set_irq_type(switch_data->hook_irq, IRQF_TRIGGER_RISING);

	enable_irq(switch_data->hook_irq);
}


static irqreturn_t snd_irq_handler(int hp_irq, void *dev_id)
{
	struct snd_switch_data *switch_data =
	    (struct snd_switch_data *)dev_id;

	SWITCH_DEBUG("come into hp_irq handler\n");

	disable_irq_nosync(switch_data->hp_irq);

	//__gpio_disable_pull(switch_data->hp_gpio);
	gpio_direction_input(switch_data->hp_gpio);

	schedule_work(&switch_data->hp_work);
	return IRQ_HANDLED;
}

static irqreturn_t hook_irq_handler(int hook_irq, void *dev_id)
{

	struct snd_switch_data *switch_data =
		(struct snd_switch_data *)dev_id;

	SWITCH_DEBUG("come into hook_irq handler\n");

	disable_irq_nosync(switch_data->hook_irq);

	gpio_direction_input(switch_data->hook_gpio);

	schedule_work(&switch_data->hook_work);
	return IRQ_HANDLED;
}

static int snd_switch_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_switch_data *switch_data = pdev->dev.platform_data;

	if (switch_data->hook_valid_level != -1 && switch_data->hook_irq > 0)
		disable_irq(switch_data->hook_irq);

	if (switch_data->type == SND_SWITCH_TYPE_GPIO)
		disable_irq(switch_data->hp_irq);

	SWITCH_DEBUG("snd_switch_suspend\n");
	return 0;
}

static int snd_switch_resume(struct platform_device *pdev)
{
	struct snd_switch_data *switch_data = pdev->dev.platform_data;

	if (!switch_data) {
		printk("hp detect device resume fail.\n");
		return 0;
	}

	//snd_switch_work(&switch_data->hp_work); // we will enable_irq(switch_data->hp_irq) in this work;
	schedule_work(&switch_data->hp_work);
	if (switch_data->hook_valid_level != -1 && switch_data->hook_irq > 0)
		enable_irq(switch_data->hook_irq);

    SWITCH_DEBUG("snd_switch_resume\n");
	return 0;
}

static ssize_t switch_snd_print_name(struct switch_dev *sdev, char *buf)
{
	struct snd_switch_data *switch_data =
		container_of(sdev ,struct snd_switch_data, sdev);

	if (!switch_data->name_headset_on &&
			!switch_data->name_headset_on&&
			!switch_data->name_off)
		return sprintf(buf,"%s.\n",sdev->name);

	return -1;
}

static ssize_t switch_snd_print_state(struct switch_dev *sdev, char *buf)
{
	struct snd_switch_data *switch_data =
		container_of(sdev, struct snd_switch_data, sdev);
	const char *state;
	unsigned int state_val = switch_get_state(sdev);
	if ( state_val == 1)
		state = switch_data->state_headset_on;
	else if ( state_val == 2)
		state = switch_data->state_headphone_on;
	else
		state = switch_data->state_off;

	if (state)
		return sprintf(buf, "%s\n", state);

	return -1;
}

static int snd_switch_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_switch_data *switch_data = pdev->dev.platform_data;
	atomic_set(&switch_data->flag,0);
    switch_data->hook_gpio = switch_data->mic_gpio;

	switch_data->sdev.print_state = switch_snd_print_state;
	switch_data->sdev.print_name = switch_snd_print_name;
	switch_data->hp_state = -1;

	platform_set_drvdata(pdev, switch_data);

    SWITCH_DEBUG("snd_switch_probe\n");

	ret = switch_dev_register(&switch_data->sdev);
	if (ret < 0) {
		printk("switch dev register fail.\n");
		goto err_switch_dev_register;
	}

	INIT_WORK(&switch_data->hp_work, snd_switch_work);

	if (switch_data->type == SND_SWITCH_TYPE_GPIO) {

		if (!gpio_is_valid(switch_data->hp_gpio))
			goto err_test_gpio;

		ret = gpio_request(switch_data->hp_gpio, pdev->name);
		if (ret < 0)
			goto err_request_gpio;


		switch_data->hp_irq = gpio_to_irq(switch_data->hp_gpio);
		if (switch_data->hp_irq < 0) {
			printk("get hp_irq error.\n");
			ret = switch_data->hp_irq;
			goto err_detect_irq_num_failed;
		}

		//ret = request_irq(switch_data->irq, snd_irq_handler,
		//				  IRQF_TRIGGER_FALLING, pdev->name, switch_data);
		ret = request_irq(switch_data->hp_irq, snd_irq_handler,
						  IRQF_TRIGGER_LOW, pdev->name, switch_data);
		if (ret < 0) {
			printk("requst hp irq fail.\n");
			goto err_request_irq;
		}
		disable_irq(switch_data->hp_irq);
	} else {
		switch_data->hp_irq = -1;
		wake_up_interruptible(&switch_data->wq);
	}


	/****hook register****/

	if (switch_data->hook_gpio != -1 && switch_data->hook_valid_level != -1 ) {
		switch_data->hook_irq = gpio_to_irq(switch_data->mic_gpio);
		switch_data->inpdev = input_allocate_device();
		if (switch_data->inpdev != NULL) {
			switch_data->inpdev->name = "hook-key";
			input_set_capability(switch_data->inpdev, EV_KEY, KEY_MEDIA);
			input_set_capability(switch_data->inpdev, EV_KEY, KEY_END);
			ret = input_register_device(switch_data->inpdev);
			if (ret < 0)
				return -EBUSY;
		}

		INIT_WORK(&switch_data->hook_work, hook_do_work);


		//irq_set_status_flags(switch_data->hook_irq,IRQ_NOAUTOEN);
		ret = request_irq(switch_data->hook_irq, hook_irq_handler,
				IRQF_TRIGGER_RISING, "hook_irq", switch_data);
		if (ret < 0)
			printk("ear mic requst hook irq fail. ret = %d\n",ret);
		else
			disable_irq(switch_data->hook_irq);
	} else
		switch_data->hook_irq = -1;

	/* Perform initial detection */
	snd_switch_work(&switch_data->hp_work);
	printk("snd_switch_probe susccess\n");
	return 0;

err_request_irq:
err_detect_irq_num_failed:
	gpio_free(switch_data->hp_gpio);
err_request_gpio:
err_test_gpio:
    switch_dev_unregister(&switch_data->sdev);
err_switch_dev_register:
     printk(KERN_ERR "%s : failed!\n", __func__);
     return ret;
}

static int snd_switch_remove(struct platform_device *pdev)
{
	struct snd_switch_data *switch_data = pdev->dev.platform_data;

	cancel_work_sync(&switch_data->hp_work);
	if (switch_data->type == SND_SWITCH_TYPE_GPIO)
		gpio_free(switch_data->hp_gpio);

	if (switch_data->hook_gpio != -1 &&
			switch_data->hook_valid_level != -1) {
		cancel_work_sync(&switch_data->hook_work);
		if (switch_data->hook_gpio != switch_data->mic_gpio) {
			gpio_free(switch_data->hook_gpio);
		}
	}

	if(switch_data->mic_gpio != -1)
		gpio_free(switch_data->mic_gpio);

	switch_dev_unregister(&switch_data->sdev);

	kfree(switch_data);

	return 0;
}


static struct platform_device_id xb_snd_det_ids[] = {
#define JZ_HP_DETECT_TABLE(NO)	{.name = DEV_DSP_HP_DET_NAME,.driver_data = SND_DEV_DETECT##NO##_ID},	\
								{.name = DEV_DSP_DOCK_DET_NAME,.driver_data = SND_DEV_DETECT##NO##_ID}
	JZ_HP_DETECT_TABLE(0),
	{}
#undef JZ_HP_DETECT_TABLE
};


static struct platform_driver snd_switch_driver = {
	.probe		= snd_switch_probe,
	.remove		= snd_switch_remove,
	.driver		= {
		.name	= "snd-det",
		.owner	= THIS_MODULE,
	},
	.id_table	= xb_snd_det_ids,
	.suspend	= snd_switch_suspend,
	.resume		= snd_switch_resume,
};

static int __init snd_switch_init(void)
{
	return platform_driver_register(&snd_switch_driver);
}

static void __exit snd_switch_exit(void)
{
	platform_driver_unregister(&snd_switch_driver);
}

device_initcall_sync(snd_switch_init);
module_exit(snd_switch_exit);
