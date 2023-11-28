/*
 * kernel/drivers/video/panel/jz_byd_8991.c -- Ingenic LCD panel device
 *
 * Copyright (C) 2005-2010, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/lcd.h>
#include <linux/fb.h>
#include <linux/regulator/consumer.h>
#include <soc/gpio.h>
#include <linux/platform_device.h>

#include <linux/byd_8991.h>
#include "../jz_fb_v12/jz_fb.h"
extern void Initial_IC(struct platform_byd_8991_data *pdata);

struct byd_8991_data {
	int lcd_power;
	struct lcd_device *lcd;
	struct platform_byd_8991_data *pdata;
	struct regulator *lcd_vcc_reg;
};

static void byd_8991_on(struct byd_8991_data *dev) {
	dev->lcd_power = 1;
#ifdef CONFIG_NEED_REGULATOR
	if(!regulator_is_enabled(dev->lcd_vcc_reg)) {
		regulator_enable(dev->lcd_vcc_reg);
	}
#endif
	if (dev->pdata->gpio_lcd_disp)
		gpio_direction_output(dev->pdata->gpio_lcd_disp, 1);

	/*spi interface init*/
	if (dev->pdata->gpio_lcd_cs)
		gpio_direction_output(dev->pdata->gpio_lcd_cs, 1);
	if (dev->pdata->gpio_lcd_clk) /* set data mode*/
		gpio_direction_output(dev->pdata->gpio_lcd_clk, 1);
	if (dev->pdata->gpio_lcd_sdo) /*sync mode*/
		gpio_direction_output(dev->pdata->gpio_lcd_sdo, 1);
	if (dev->pdata->gpio_lcd_sdi)
		gpio_direction_input(dev->pdata->gpio_lcd_sdi);
#if 0
	if (dev->pdata->gpio_lcd_back_sel) /*sync mode*/
		gpio_direction_output(dev->pdata->gpio_lcd_back_sel, 1);
#endif
	Initial_IC(dev->pdata);
}

static void byd_8991_off(struct byd_8991_data *dev)
{
	dev->lcd_power = 0;
	if (dev->pdata->gpio_lcd_cs)
		gpio_direction_output(dev->pdata->gpio_lcd_cs, 0);
	if (dev->pdata->gpio_lcd_disp)
		gpio_direction_output(dev->pdata->gpio_lcd_disp, 0);
	mdelay(2);
#ifdef CONFIG_NEED_REGULATOR
	if(regulator_is_enabled(dev->lcd_vcc_reg)) {
		regulator_disable(dev->lcd_vcc_reg);
	}
#endif
	mdelay(10);
}

static int byd_8991_set_power(struct lcd_device *lcd, int power)
{
	struct byd_8991_data *dev = lcd_get_data(lcd);

	if (!power && !(dev->lcd_power)) {
                byd_8991_on(dev);
        } else if (power && (dev->lcd_power)) {
                byd_8991_off(dev);
        }
	return 0;
}

static int byd_8991_get_power(struct lcd_device *lcd)
{
	struct byd_8991_data *dev= lcd_get_data(lcd);

	return dev->lcd_power;
}

static int byd_8991_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
	return 0;
}

static struct lcd_ops byd_8991_ops = {
	.set_power = byd_8991_set_power,
	.get_power = byd_8991_get_power,
	.set_mode = byd_8991_set_mode,
};

static int byd_8991_probe(struct platform_device *pdev)
{
	/* check the parameters from lcd_driver */
	int ret;
	struct byd_8991_data *dev;
	dev = kzalloc(sizeof(struct byd_8991_data), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->pdata = pdev->dev.platform_data;

	dev_set_drvdata(&pdev->dev, dev);
#ifdef CONFIG_NEED_REGULATOR
	dev->lcd_vcc_reg = regulator_get(NULL, "lcd_1v8");
	if (IS_ERR(dev->lcd_vcc_reg)) {
		dev_err(&pdev->dev, "failed to get regulator vlcd\n");
		return PTR_ERR(dev->lcd_vcc_reg);
	}
#endif
	if (dev->pdata->gpio_lcd_disp)
		gpio_request(dev->pdata->gpio_lcd_disp, "display on");

	/* lcd spi interface */
	if (dev->pdata->gpio_lcd_cs)
		gpio_request(dev->pdata->gpio_lcd_cs, "spi_cs");
	if (dev->pdata->gpio_lcd_clk)
		gpio_request(dev->pdata->gpio_lcd_clk, "spi_clk");
	if (dev->pdata->gpio_lcd_sdo)
		gpio_request(dev->pdata->gpio_lcd_sdo, "spi_sdo");
	if (dev->pdata->gpio_lcd_sdi)
		gpio_request(dev->pdata->gpio_lcd_sdi, "spi_sdi");
#if 0
	if (dev->pdata->gpio_lcd_back_sel)
		gpio_request(dev->pdata->gpio_lcd_back_sel, "back_light_ctrl");
#endif
	if ( ! lcd_display_inited_by_uboot() )
		byd_8991_on(dev);

	dev->lcd = lcd_device_register("byd_8991-lcd", &pdev->dev,
				       dev, &byd_8991_ops);

	if (IS_ERR(dev->lcd)) {
		ret = PTR_ERR(dev->lcd);
		dev->lcd = NULL;
		dev_info(&pdev->dev, "lcd device register error: %d\n", ret);
	} else {
		dev_info(&pdev->dev, "lcd device register success\n");
	}

	return 0;
}

static int byd_8991_remove(struct platform_device *pdev)
{

	struct byd_8991_data *dev = dev_get_drvdata(&pdev->dev);

	lcd_device_unregister(dev->lcd);
	byd_8991_off(dev);
#ifdef CONFIG_NEED_REGULATOR
	regulator_put(dev->lcd_vcc_reg);
#endif
	if (dev->pdata->gpio_lcd_disp)
		gpio_free(dev->pdata->gpio_lcd_disp);
	if (dev->pdata->gpio_lcd_cs)
		gpio_free(dev->pdata->gpio_lcd_cs);
	if (dev->pdata->gpio_lcd_clk)
		gpio_free(dev->pdata->gpio_lcd_clk);
	if (dev->pdata->gpio_lcd_sdo)
		gpio_free(dev->pdata->gpio_lcd_sdo);
	if (dev->pdata->gpio_lcd_sdi)
		gpio_free(dev->pdata->gpio_lcd_sdi);

	dev_set_drvdata(&pdev->dev, NULL);
	kfree(dev);

	return 0;
}

#ifdef CONFIG_PM
static int byd_8991_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	return 0;
}

static int byd_8991_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define byd_8991_suspend	NULL
#define byd_8991_resume	NULL
#endif

static struct platform_driver byd_8991_driver = {
	.driver		= {
		.name	= "byd_8991-lcd",
		.owner	= THIS_MODULE,
	},
	.probe		= byd_8991_probe,
	.remove		= byd_8991_remove,
	.suspend	= byd_8991_suspend,
	.resume		= byd_8991_resume,
};

static int __init byd_8991_init(void)
{
	// register the panel with lcd drivers
	return platform_driver_register(&byd_8991_driver);;
}
module_init(byd_8991_init);

static void __exit byd_8991_exit(void)
{
	platform_driver_unregister(&byd_8991_driver);
}
module_exit(byd_8991_exit);

MODULE_DESCRIPTION("byd_8991 lcd driver");
MODULE_LICENSE("GPL");
