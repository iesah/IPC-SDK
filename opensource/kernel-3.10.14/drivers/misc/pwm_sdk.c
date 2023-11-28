/*
 * pwm sdk api code;
 *
 * Copyright (c) 2015 Ingenic
 * Author: Tiger <bohu.liang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/fs.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#define PWM_NUM		2

struct pwm_ioctl_t {
	int index;
	int duty;
	int period;
	int polarity;
};

struct pwm_device_t {
	int duty;
	int period;
	int polarity;
	struct pwm_device *pwm_device;
};

struct pwm_sdk_t {
	struct device *dev;
	struct miscdevice mdev;
	struct pwm_device_t *pwm_device_t[PWM_NUM];
};
struct pwm_sdk_t *gpwm;

static int pwm_sdk_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int pwm_sdk_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static long pwm_sdk_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int id, ret = 0;
	struct pwm_ioctl_t *pwm_ioctl;

	switch(cmd) {
		case 1:
			pwm_ioctl = (struct pwm_ioctl_t *)arg;
			if(pwm_ioctl == NULL) {
				dev_err(gpwm->dev, "ioctl error(%d) !\n", __LINE__);
				ret = -1;
				break;
			}

			id = pwm_ioctl->index;
			if((id >= PWM_NUM) || (id < 0)) {
				dev_err(gpwm->dev, "ioctl error(%d) !\n", __LINE__);
				ret = -1;
				break;
			}

			if((pwm_ioctl->period > 1000000000) || (pwm_ioctl->period < 200)) {
				dev_err(gpwm->dev, "period error !\n");
				ret = -1;
				break;
			}

			if((pwm_ioctl->duty > pwm_ioctl->period) || (pwm_ioctl->duty < 0)) {
				dev_err(gpwm->dev, "duty error !\n");
				ret = -1;
				break;
			}

			if((pwm_ioctl->polarity > 1) || (pwm_ioctl->polarity < 0)) {
				dev_err(gpwm->dev, "polarity error !\n");
				ret = -1;
				break;
			}

			gpwm->pwm_device_t[id]->period = pwm_ioctl->period;
			gpwm->pwm_device_t[id]->duty = pwm_ioctl->duty;
			gpwm->pwm_device_t[id]->polarity = pwm_ioctl->polarity;

			break;
		case 2:
			id = (int)arg;

			if((id >= PWM_NUM) || (id < 0)) {
				dev_err(gpwm->dev, "ioctl error(%d) !\n", __LINE__);
				ret = -1;
				break;
			}

			if((gpwm->pwm_device_t[id]->pwm_device == NULL) || (IS_ERR(gpwm->pwm_device_t[id]->pwm_device))) {
				dev_err(gpwm->dev, "pwm could not work !\n");
				ret = -1;
				break;
			}

			if((gpwm->pwm_device_t[id]->period == -1) || (gpwm->pwm_device_t[id]->duty == -1) || (gpwm->pwm_device_t[id]->polarity == -1)) {
				dev_err(gpwm->dev, "the parameter of pwm could not init !\n");
				ret = -1;
				break;
			}

			if(gpwm->pwm_device_t[id]->polarity == 0)
				pwm_set_polarity(gpwm->pwm_device_t[id]->pwm_device, PWM_POLARITY_INVERSED);
			else
				pwm_set_polarity(gpwm->pwm_device_t[id]->pwm_device, PWM_POLARITY_NORMAL);

			pwm_config(gpwm->pwm_device_t[id]->pwm_device, gpwm->pwm_device_t[id]->duty, gpwm->pwm_device_t[id]->period);

			pwm_enable(gpwm->pwm_device_t[id]->pwm_device);

			break;
		case 3:
			id = (int)arg;

			if((id >= PWM_NUM) || (id < 0)) {
				dev_err(gpwm->dev, "ioctl error(%d) !\n", __LINE__);
				ret = -1;
				break;
			}

			if((gpwm->pwm_device_t[id]->pwm_device == NULL) || (IS_ERR(gpwm->pwm_device_t[id]->pwm_device))) {
				dev_err(gpwm->dev, "pwm could not work !\n");
				ret = -1;
				break;
			}

			if((gpwm->pwm_device_t[id]->period == -1) || (gpwm->pwm_device_t[id]->duty == -1) || (gpwm->pwm_device_t[id]->polarity == -1)) {
				dev_err(gpwm->dev, "the parameter of pwm could not init !\n");
				ret = -1;
				break;
			}

			pwm_config(gpwm->pwm_device_t[id]->pwm_device, 0, gpwm->pwm_device_t[id]->period);

			pwm_disable(gpwm->pwm_device_t[id]->pwm_device);

			break;
		default:
			dev_err(gpwm->dev, "unsupport cmd !\n");
			break;
	}

	return ret;
}

static struct file_operations pwm_sdk_fops = {
	.owner		= THIS_MODULE,
	.open		= pwm_sdk_open,
	.release	= pwm_sdk_release,
	.unlocked_ioctl	= pwm_sdk_ioctl,
};

static int jz_pwm_sdk_probe(struct platform_device *pdev)
{
	int i, ret;

	gpwm = devm_kzalloc(&pdev->dev, sizeof(struct pwm_sdk_t), GFP_KERNEL);
	if(gpwm == NULL) {
		dev_err(&pdev->dev, "devm_kzalloc gpwm error !\n");
		return -ENOMEM;
	}

	for(i = 0; i < PWM_NUM; i++) {
		gpwm->pwm_device_t[i] = devm_kzalloc(&pdev->dev, (PWM_NUM * sizeof(struct pwm_device_t)), GFP_KERNEL);
		if(gpwm->pwm_device_t[i] == NULL) {
			dev_err(&pdev->dev, "devm_kzalloc pwm_device_t error !\n");
			return -ENOMEM;
		}
	}

	gpwm->pwm_device_t[0]->pwm_device = devm_pwm_get(&pdev->dev, "pwm-sdk.2");
	if (IS_ERR(gpwm->pwm_device_t[0]->pwm_device)) {
		dev_err(&pdev->dev, "devm_pwm_get error !");
		return -ENOMEM;
	}

	gpwm->pwm_device_t[1]->pwm_device = devm_pwm_get(&pdev->dev, "pwm-sdk.3");
	if (IS_ERR(gpwm->pwm_device_t[1]->pwm_device)) {
		dev_err(&pdev->dev, "devm_pwm_get error !");
		return -ENOMEM;
	}

	gpwm->mdev.minor = MISC_DYNAMIC_MINOR;
	gpwm->mdev.name = "pwm-sdk";
	gpwm->mdev.fops = &pwm_sdk_fops;
	ret = misc_register(&gpwm->mdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "misc_register failed !\n");
		return ret;
	}

	for(i = 0; i < PWM_NUM; i++) {
		gpwm->pwm_device_t[i]->duty = -1;
		gpwm->pwm_device_t[i]->period = -1;
		gpwm->pwm_device_t[i]->polarity = -1;
	}

	gpwm->dev = &pdev->dev;

	dev_info(&pdev->dev, "%s register ok !\n", __func__);

	return 0;
}

static int jz_pwm_sdk_remove(struct platform_device *pdev)
{
	misc_deregister(&gpwm->mdev);

	return 0;
}

static struct platform_driver jz_pwm_sdk_driver = {
	.driver = {
		.name = "pwm-sdk",
		.owner = THIS_MODULE,
	},
	.probe = jz_pwm_sdk_probe,
	.remove = jz_pwm_sdk_remove,
};

module_platform_driver(jz_pwm_sdk_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("JZ PWM SDK Driver");
MODULE_AUTHOR("Tiger <bohu.liang@ingenic.com>");
