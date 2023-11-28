/*
 *  LCD control code for truly
 *
 *  This program is free software; you can redistribute it and/or modify
 *  published by the Free Software Foundation.
 *
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/lcd.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <soc/base.h>
#include <soc/gpio.h>
//#include <soc/irq.h>
#include "../include/jzfb.h"
#include <dt-bindings/interrupt-controller/t41-irq.h>
#define GPIO_BL_PWR_EN         79
#define GPIO_LCD_RST           105
#define FPGA_TEST
struct truly_tft240320_data {
	int lcd_power;
	struct lcd_device *lcd;
	struct lcd_platform_data *ctrl;
	struct regulator *lcd_vcc_reg;
};

static int truly_tft240320_set_power(struct lcd_device *lcd, int power)
{
	struct truly_tft240320_data *dev= lcd_get_data(lcd);
	if (!power && dev->lcd_power) {
		/* if(!regulator_is_enabled(dev->lcd_vcc_reg)) */
		/* 	ret = regulator_enable(dev->lcd_vcc_reg); */
		dev->ctrl->power_on(lcd, 1);
	} else if (power && !dev->lcd_power) {
		if (dev->ctrl->reset) {
			dev->ctrl->reset(lcd);
		}
		dev->ctrl->power_on(lcd, 0);
		/* if(regulator_is_enabled(dev->lcd_vcc_reg)) */
		/* 	regulator_disable(dev->lcd_vcc_reg); */
	}
	dev->lcd_power = power;
	return 0;
}

static int truly_tft240320_get_power(struct lcd_device *lcd)
{
	struct truly_tft240320_data *dev= lcd_get_data(lcd);

	return dev->lcd_power;
}

static int truly_tft240320_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
	return 0;
}

static struct lcd_ops truly_tft240320_ops = {
	.set_power = truly_tft240320_set_power,
	.get_power = truly_tft240320_get_power,
	.set_mode = truly_tft240320_set_mode,
};

static int truly_tft240320_probe(struct platform_device *pdev)
{
	int ret;
	struct truly_tft240320_data *dev;
	dev = kzalloc(sizeof(struct truly_tft240320_data), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->ctrl = pdev->dev.platform_data;
	if (dev->ctrl == NULL) {
		dev_info(&pdev->dev, "no platform data!");
		return -EINVAL;
	}

	dev_set_drvdata(&pdev->dev, dev);

	dev->lcd = lcd_device_register("truly_tft240320_slcd", &pdev->dev,
				       dev, &truly_tft240320_ops);
	if (IS_ERR(dev->lcd)) {
		ret = PTR_ERR(dev->lcd);
		dev->lcd = NULL;
		dev_info(&pdev->dev, "lcd device register error: %d\n", ret);
	} else {
		dev_info(&pdev->dev, "lcd device(TRULY TFT240320) register success\n");
	}

	if (dev->ctrl->power_on) {
		dev->ctrl->power_on(dev->lcd, 1);
		dev->lcd_power = FB_BLANK_UNBLANK;
	}

#if 0
	dev->lcd_vcc_reg = regulator_get(NULL,"lcd_3v3");
	ret = regulator_enable(dev->lcd_vcc_reg);

	ret = gpio_request(GPIO_LCD_RST, "lcd rst");
	if (ret) {
		printk(KERN_ERR "can's request lcd rst\n");
		return ret;
	}
	gpio_direction_output(GPIO_LCD_RST, 0);
	mdelay(20);
	gpio_direction_output(GPIO_LCD_RST, 1);
#endif

	ret = gpio_request(GPIO_BL_PWR_EN, "BL PWR");
	if (ret) {
		printk(KERN_ERR "failed to reqeust BL PWR\n");
		return ret;
	}

	gpio_direction_output(GPIO_BL_PWR_EN, 1);
	return 0;
}

static int truly_tft240320_remove(struct platform_device *pdev)
{
	struct truly_tft240320_data *dev = dev_get_drvdata(&pdev->dev);

	if (dev->lcd_power)
		dev->ctrl->power_on(dev->lcd, 0);
#if 0
	regulator_put(dev->lcd_vcc_reg);
#endif
	lcd_device_unregister(dev->lcd);
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(dev);

	return 0;
}

#ifdef CONFIG_PM
static int truly_tft240320_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	struct truly_tft240320_data *dev;
	truly_tft240320_set_power(dev->lcd, FB_BLANK_POWERDOWN);
	return 0;
}

static int truly_tft240320_resume(struct platform_device *pdev)
{
	struct truly_tft240320_data *dev;
	truly_tft240320_set_power(dev->lcd,FB_BLANK_POWERDOWN);
	return 0;
}
#else
#define truly_tft240320_suspend	NULL
#define truly_tft240320_resume	NULL
#endif

static struct platform_driver truly_tft240320_driver = {
	.driver		= {
		.name	= "truly_tft240320_slcd",
		.owner	= THIS_MODULE,
	},
	.probe		= truly_tft240320_probe,
	.remove		= truly_tft240320_remove,
	.suspend	= truly_tft240320_suspend,
	.resume		= truly_tft240320_resume,
};

/* lcd platform device */
extern struct platform_device truly_tft240320_device;

/* jz_fb platform device */
extern struct jzfb_platform_data jzfb_pdata;

static u64 jz_fb_dmamask = ~(u64) 0;

static struct resource jz_fb_resources[] = {
	[0] = {
		.start = LCDC_IOBASE,
		.end = LCDC_IOBASE + 0x1800 - 1,
	//	.end = LCDC_IOBASE + 0x10000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
//		.start = IRQ_LCD,
//		.end = IRQ_LCD,
		.start = IRQ_LCD,
		.end = IRQ_LCD,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device jz_fb_device = {
	.name = "jz-fb",
	.id = 0,
	.dev = {
		.dma_mask = &jz_fb_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(jz_fb_resources),
	.resource = jz_fb_resources,
};

#if 0
#define SLCDC_PORTB							\
	{ .name = "slcd", .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x0021efc0, }
#endif

static int __init truly_tft240320_init(void)
{
//	jzgpio_set_func(GPIO_PORT_B, GPIO_FUNC_3, 0x3f<<6 | 0x3<<13 | 0x1<<15 | 0x3<<20);
	jzgpio_set_func(GPIO_PORT_D, GPIO_FUNC_0, 0xff<<0 | 0x1<<8 | 0x3<<10 );

	platform_device_add_data(&jz_fb_device, &jzfb_pdata, sizeof(struct jzfb_platform_data));
	platform_device_register(&jz_fb_device);

	platform_device_register(&truly_tft240320_device);
	return platform_driver_register(&truly_tft240320_driver);
}
module_init(truly_tft240320_init);

static void __exit truly_tft240320_exit(void)
{
	platform_device_unregister(&truly_tft240320_device);
	platform_driver_unregister(&truly_tft240320_driver);
}
module_exit(truly_tft240320_exit);

MODULE_DESCRIPTION("TRULY TFT240320 lcd panel driver");
MODULE_LICENSE("GPL");
