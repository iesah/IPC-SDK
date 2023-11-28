/* linux/drivers/video/exynos/byd_9177aa.c
 *
 * MIPI-DSI based byd_9177aa AMOLED lcd 4.65 inch panel driver.
 *
 * Inki Dae, <inki.dae@samsung.com>
 * Donghwa Lee, <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/lcd.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>
#include <mach/jz_dsim.h>

#define POWER_IS_ON(pwr)	((pwr) == FB_BLANK_UNBLANK)
#define POWER_IS_OFF(pwr)	((pwr) == FB_BLANK_POWERDOWN)
#define POWER_IS_NRM(pwr)	((pwr) == FB_BLANK_NORMAL)

#define lcd_to_master(a)	(a->dsim_dev->master)
#define lcd_to_master_ops(a)	((lcd_to_master(a))->master_ops)

struct byd_9177aa {
	struct device	*dev;
	unsigned int			power;
	unsigned int			id;

	struct lcd_device	*ld;
	struct backlight_device	*bd;

	struct mipi_dsim_lcd_device	*dsim_dev;
	struct lcd_platform_data	*ddi_pd;
	struct mutex			lock;
	struct regulator *lcd_vcc_reg;
	bool  enabled;
};

static struct dsi_cmd_packet byd_9177aa_cmd_list[] = {
	{0x39, 0x04, 0x00,  {0xb9, 0xff, 0x83, 0x89}}, //set extc
	{0x39, 0x14, 0x00,  {0xb1, 0x00, 0x00, 0x06, 0xe8, 0x59, 0x10, 0x11, 0xd1, 0xf1, 0x3d, 0x45, 0x2e, 0x2e, 0x43, 0x01, 0x5a, 0xf0, 0x00, 0xe6}}, //set power
	{0x39, 0x08, 0x00,  {0xb2, 0x00, 0x00, 0x78, 0x0c, 0x07, 0x3f, 0x30}}, //set  display
	{0x39, 0x18, 0x00,  {0xb4, 0x80, 0x08, 0x00, 0x32, 0x10, 0x04, 0x32, 0x10, 0x00, 0x32, 0x10, 0x00, 0x37, 0x0a, 0x40, 0x08, 0x37, 0x00, 0x46, 0x06, 0x58, 0x58, 0x06}}, //column inversion
	{0x39, 0x39, 0x00,  {0xd5, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x60, 0x00, 0x99, 0x88, 0xaa, 0xbb, 0x88, 0x23, 0x88, 0x01, 0x88, 0x67, 0x88, 0x45, 0x01, 0x23, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x99, 0xbb, 0xaa, 0x88, 0x54, 0x88, 0x76, 0x88, 0x10, 0x88, 0x32, 0x32, 0x10, 0x88, 0x88, 0x88, 0x88, 0x88, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }},//set gip
	{0x39, 0x04, 0x00,  {0xb7, 0x00, 0x00, 0x50}}, //set power option
	{0x39, 0x05, 0x00,  {0xb6, 0x00, 0x92, 0x00, 0x92}}, //set vcom
	{0x39, 0x23, 0x00,  {0xe0, 0x05, 0x11, 0x14, 0x37, 0x3f, 0x3f, 0x20, 0x4f, 0x05, 0x0e, 0x0d, 0x12, 0x14, 0x12, 0x14, 0x1d, 0x1c, 0x05, 0x11, 0x14, 0x37, 0x3f, 0x3f, 0x20, 0x4f, 0x05, 0x0e, 0x0d, 0x12, 0x14, 0x12, 0x14, 0x1d, 0x1c}}, //set gamma
	{0x39, 0x80, 0x00,  {0xc1, 0x01, 0x00, 0x1b, 0x20, 0x28, 0x2e, 0x34, 0x3b, 0x43, 0x4b, 0x52, 0x5a, 0x62, 0x6b, 0x72, 0x78, 0x7D, 0x82, 0x87, 0x8D, 0x93, 0x9A, 0x9E, 0xA3, 0xA8, 0xAF, 0xB5, 0xBE, 0xC6, 0xCD, 0xD8, 0xE1, 0xEB, 0xF9, 0x10, 0x12, 0x02, 0x25, 0x65, 0x55, 0x65, 0x65, 0x40, 0x00, 0x1B, 0x20, 0x28, 0x2E, 0x34, 0x3B, 0x43, 0x4B, 0x52, 0x5A, 0x62, 0x6B, 0x72, 0x78, 0x7D, 0x82, 0x87, 0x8D, 0x93, 0x9A, 0x9E, 0xA3, 0xA8, 0xAF, 0xB5, 0xBE, 0xC6, 0xCD, 0xD8, 0xE1, 0xEB, 0xF9, 0x10, 0x12, 0x02, 0x25, 0x65, 0x55, 0x65, 0x65, 0x40, 0x00, 0x1B, 0x20, 0x28, 0x2E, 0x34, 0x3B, 0x43, 0x4B, 0x52, 0x5A, 0x62, 0x6B, 0x72, 0x78, 0x7D, 0x82, 0x87, 0x8D, 0x93, 0x9A, 0x9E, 0xA3, 0xA8, 0xAF, 0xB5, 0xBE, 0xC6, 0xCD, 0xD8, 0xE1, 0xEB, 0xF9, 0x10, 0x12, 0x02, 0x25, 0x65, 0x55, 0x65, 0x65, 0x40}}, //set dgc lut
	{0x39, 0x02, 0x00,  {0xCC, 0x02}}, //set rev panel
};

static void byd_9177aa_regulator_enable(struct byd_9177aa *lcd)
{
	int ret = 0;
	struct lcd_platform_data *pd = NULL;

	pd = lcd->ddi_pd;
	mutex_lock(&lcd->lock);
	if(!regulator_is_enabled(lcd->lcd_vcc_reg)) {
		regulator_enable(lcd->lcd_vcc_reg);
	}

	msleep(pd->power_on_delay);
out:
	mutex_unlock(&lcd->lock);
}

static void byd_9177aa_regulator_disable(struct byd_9177aa *lcd)
{
	int ret = 0;

	mutex_lock(&lcd->lock);
	if(regulator_is_enabled(lcd->lcd_vcc_reg)) {
		regulator_disable(lcd->lcd_vcc_reg);
	}
out:
	mutex_unlock(&lcd->lock);
}

static void byd_9177aa_sleep_in(struct byd_9177aa *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x10, 0x00};

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
}

static void byd_9177aa_sleep_out(struct byd_9177aa *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x11, 0x00};

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
}

static void byd_9177aa_display_on(struct byd_9177aa *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x29, 0x00};

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
}

static void byd_9177aa_display_off(struct byd_9177aa *lcd)
{
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_cmd_packet data_to_send = {0x05, 0x28, 0x00};

	ops->cmd_write(lcd_to_master(lcd), data_to_send);
}

static void byd_9177aa_panel_init(struct byd_9177aa *lcd)
{
	int  i;
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);
	struct dsi_device *dsi = lcd_to_master(lcd);
	for(i = 0; i < ARRAY_SIZE(byd_9177aa_cmd_list); i++) {
		ops->cmd_write(dsi,  byd_9177aa_cmd_list[i]);
	}
}

static int byd_9177aa_set_power(struct lcd_device *ld, int power)
{
	int ret = 0;
#if 0
	struct byd_9177aa *lcd = lcd_get_data(ld);
	struct dsi_master_ops *ops = lcd_to_master_ops(lcd);

	if (power != FB_BLANK_UNBLANK && power != FB_BLANK_POWERDOWN &&
			power != FB_BLANK_NORMAL) {
		dev_err(lcd->dev, "power value should be 0, 1 or 4.\n");
		return -EINVAL;
	}

	if ((power == FB_BLANK_UNBLANK) && ops->set_blank_mode) {
		/* LCD power on */
		if ((POWER_IS_ON(power) && POWER_IS_OFF(lcd->power))
			|| (POWER_IS_ON(power) && POWER_IS_NRM(lcd->power))) {
			ret = ops->set_blank_mode(lcd_to_master(lcd), power);
			if (!ret && lcd->power != power)
				lcd->power = power;
		}
	} else if ((power == FB_BLANK_POWERDOWN) && ops->set_early_blank_mode) {
		/* LCD power off */
		if ((POWER_IS_OFF(power) && POWER_IS_ON(lcd->power)) ||
		(POWER_IS_ON(lcd->power) && POWER_IS_NRM(power))) {
			ret = ops->set_early_blank_mode(lcd_to_master(lcd),
							power);
			if (!ret && lcd->power != power)
				lcd->power = power;
		}
	}

#endif
	return ret;
}

static int byd_9177aa_get_power(struct lcd_device *ld)
{
	struct byd_9177aa *lcd = lcd_get_data(ld);

	return lcd->power;
}


static struct lcd_ops byd_9177aa_lcd_ops = {
	.set_power = byd_9177aa_set_power,
	.get_power = byd_9177aa_get_power,
};


static void byd_9177aa_power_on(struct mipi_dsim_lcd_device *dsim_dev, int power)
{
	struct byd_9177aa *lcd = dev_get_drvdata(&dsim_dev->dev);

	/* lcd power on */
	if (power)
		byd_9177aa_regulator_enable(lcd);
	else
		byd_9177aa_regulator_disable(lcd);
	if (lcd->ddi_pd->power_on)
		lcd->ddi_pd->power_on(lcd->ld, power);
	/* lcd reset */
	if (lcd->ddi_pd->reset)
		lcd->ddi_pd->reset(lcd->ld);
}

static void byd_9177aa_set_sequence(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct byd_9177aa *lcd = dev_get_drvdata(&dsim_dev->dev);

	byd_9177aa_panel_init(lcd);
	byd_9177aa_sleep_out(lcd);
	//msleep(120);
	msleep(10);
	byd_9177aa_display_on(lcd);
	msleep(10);
	lcd->power = FB_BLANK_UNBLANK;
}

static int byd_9177aa_probe(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct byd_9177aa *lcd;
	lcd = devm_kzalloc(&dsim_dev->dev, sizeof(struct byd_9177aa), GFP_KERNEL);
	if (!lcd) {
		dev_err(&dsim_dev->dev, "failed to allocate byd_9177aa structure.\n");
		return -ENOMEM;
	}

	lcd->dsim_dev = dsim_dev;
	lcd->ddi_pd = (struct lcd_platform_data *)dsim_dev->platform_data;
	lcd->dev = &dsim_dev->dev;

	mutex_init(&lcd->lock);

	lcd->lcd_vcc_reg = regulator_get(NULL, "lcd_1v8");
	if (IS_ERR(lcd->lcd_vcc_reg)) {
		dev_err(lcd->dev, "failed to get regulator vlcd\n");
		return PTR_ERR(lcd->lcd_vcc_reg);
	}

	lcd->ld = lcd_device_register("byd_9177aa", lcd->dev, lcd,
			&byd_9177aa_lcd_ops);
	if (IS_ERR(lcd->ld)) {
		dev_err(lcd->dev, "failed to register lcd ops.\n");
		return PTR_ERR(lcd->ld);
	}

	dev_set_drvdata(&dsim_dev->dev, lcd);

	dev_dbg(lcd->dev, "probed byd_9177aa panel driver.\n");

	return 0;
}

#ifdef CONFIG_PM
static int byd_9177aa_suspend(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct byd_9177aa *lcd = dev_get_drvdata(&dsim_dev->dev);

	byd_9177aa_sleep_in(lcd);
	msleep(lcd->ddi_pd->power_off_delay);
	byd_9177aa_display_off(lcd);

	/* power off */
	if (lcd->ddi_pd->power_on)
		lcd->ddi_pd->power_on(lcd->ld, 0);

	byd_9177aa_regulator_disable(lcd);

	return 0;
}

static int byd_9177aa_resume(struct mipi_dsim_lcd_device *dsim_dev)
{
	struct byd_9177aa *lcd = dev_get_drvdata(&dsim_dev->dev);

#if 0
	byd_9177aa_regulator_enable(lcd);

	byd_9177aa_sleep_out(lcd);
	msleep(lcd->ddi_pd->power_on_delay);

	byd_9177aa_set_sequence(dsim_dev);
#endif
	return 0;
}
#else
#define byd_9177aa_suspend		NULL
#define byd_9177aa_resume		NULL
#endif

static struct mipi_dsim_lcd_driver byd_9177aa_dsim_ddi_driver = {
	.name = "byd_9177aa-lcd",
	.id = -1,

	.power_on = byd_9177aa_power_on,
	.set_sequence = byd_9177aa_set_sequence,
	.probe = byd_9177aa_probe,
	.suspend = byd_9177aa_suspend,
	.resume = byd_9177aa_resume,
};

static int byd_9177aa_init(void)
{
	mipi_dsi_register_lcd_driver(&byd_9177aa_dsim_ddi_driver);
	return 0;
}

static void byd_9177aa_exit(void)
{
	return;
}

module_init(byd_9177aa_init);
module_exit(byd_9177aa_exit);

MODULE_DESCRIPTION("BYD_9177aa lcd driver");
MODULE_LICENSE("GPL");
