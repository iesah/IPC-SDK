/*
 * ingenic_bsp/chip-x2000/fpga/dpu/kd035hvfmd057.c
 *
 * Copyright (C) 2016 Ingenic Semiconductor Inc.
 *
 * Author:clwang<chunlei.wang@ingenic.com>
 *
 * This program is free software, you can redistribute it and/or modify it
 *
 * under the terms of the GNU General Public License version 2 as published by
 *
 * the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/digital_pulse_backlight.h>
#include <linux/delay.h>
#include <mach/jzfb.h>
#include <soc/gpio.h>
#include <linux/lcd.h>


#define GPIO_LCD_RST	    GPIO_PB(11)
#define GPIO_LCD_CS	    GPIO_PC(5)
#define GPIO_LCD_PWM	    GPIO_PB(8)
#define GPIO_SSI_CLK	    GPIO_PC(2)
#define GPIO_SSI_DT	    GPIO_PC(4)

#define SSI_CS(n)	    gpio_direction_output(GPIO_LCD_CS, n)
#define SSI_CLK(n)	    gpio_direction_output(GPIO_SSI_CLK, n)
#define SSI_DT(n)	    gpio_direction_output(GPIO_SSI_DT, n)

static void lcd_send_cmd(uint8_t cmd) {

	int i = 8;
	SSI_CS(0);
	SSI_CLK(0);
	SSI_DT(0);
	SSI_CLK(1);
	while(i--) {
		SSI_CLK(0);
		SSI_DT(!!(cmd & 0x80));
		SSI_CLK(1);
		cmd = cmd << 1;
	}
	SSI_CLK(0);
	SSI_CS(1);
}

static void lcd_send_param(uint8_t param) {

	int i = 8;
	SSI_CS(0);
	SSI_CLK(0);
	SSI_DT(1);
	SSI_CLK(1);
	while(i--) {
		SSI_CLK(0);
		SSI_DT(!!(param & 0x80));
		SSI_CLK(1);
		param = param << 1;
	}
	SSI_CLK(0);
	SSI_CS(1);

}

static void kd035hvfmd057_init(void) {

	lcd_send_cmd(0xC0);
	lcd_send_param(0x17);
	lcd_send_param(0x17);

	lcd_send_cmd(0xC1);
	lcd_send_param(0x44);

	lcd_send_cmd(0xC5);
	lcd_send_param(0x00);
	lcd_send_param(0x3A);
	lcd_send_param(0x80);

	lcd_send_cmd(0x36);
	lcd_send_param(0x48);

	lcd_send_cmd(0x3A); //Interface Mode Control
	lcd_send_param(0x60);

	lcd_send_cmd(0xB1);   //Frame rate 70HZ
	lcd_send_param(0xA0);

	lcd_send_cmd(0xB4);
	lcd_send_param(0x02);

	lcd_send_cmd(0xB7);
	lcd_send_param(0xC6);

	lcd_send_cmd(0xE9);
	lcd_send_param(0x00);

	lcd_send_cmd(0XF7);
	lcd_send_param(0xA9);
	lcd_send_param(0x51);
	lcd_send_param(0x2C);
	lcd_send_param(0x82);

	lcd_send_cmd(0xE0);
	lcd_send_param(0x01);
	lcd_send_param(0x13);
	lcd_send_param(0x1E);
	lcd_send_param(0x00);
	lcd_send_param(0x0D);
	lcd_send_param(0x03);
	lcd_send_param(0x3D);
	lcd_send_param(0x55);
	lcd_send_param(0x4F);
	lcd_send_param(0x06);
	lcd_send_param(0x10);
	lcd_send_param(0x0B);
	lcd_send_param(0x2C);
	lcd_send_param(0x32);
	lcd_send_param(0x0F);

	lcd_send_cmd(0xE1);
	lcd_send_param(0x08);
	lcd_send_param(0x10);
	lcd_send_param(0x15);
	lcd_send_param(0x03);
	lcd_send_param(0x0E);
	lcd_send_param(0x03);
	lcd_send_param(0x32);
	lcd_send_param(0x34);
	lcd_send_param(0x44);
	lcd_send_param(0x07);
	lcd_send_param(0x10);
	lcd_send_param(0x0E);
	lcd_send_param(0x23);
	lcd_send_param(0x2E);
	lcd_send_param(0x0F);

	lcd_send_cmd(0xB6);
	lcd_send_param(0xB0); //set rgb bypass mode
	lcd_send_param(0x02); //GS,SS
	lcd_send_param(0x3B);

	/****************************************/
	lcd_send_cmd(0XB0);  //Interface Mode Control
	lcd_send_param(0x00);
	 /**************************************************/
	lcd_send_cmd(0x2A); //Frame rate control
	lcd_send_param(0x00);
	lcd_send_param(0x00);
	lcd_send_param(0x01);
	lcd_send_param(0x3F);

	lcd_send_cmd(0x2B); //Display function control
	lcd_send_param(0x00);
	lcd_send_param(0x00);
	lcd_send_param(0x01);
	lcd_send_param(0xDF);

	lcd_send_cmd(0x21);

	lcd_send_cmd(0x11);
	mdelay(120);
	lcd_send_cmd(0x29); //display on
	lcd_send_cmd(0x2c);

}

static int kd035hvfmd057_on_begin(struct lcdc_gpio_struct *gpio)
{
	int ret = 0;

	ret = gpio_request(GPIO_SSI_CLK, "lcd spi clk");
	if(ret) {
		printk("%s %s %d: lcd ssi clk pin request failed, ret = %d\n",
			__FILE__, __func__, __LINE__, ret);
		goto failed;
	}

	ret = gpio_request(GPIO_SSI_DT, "lcd spi clk");
	if(ret) {
		printk("%s %s %d: lcd ssi dt pin request failed, ret = %d\n",
			__FILE__, __func__, __LINE__, ret);
		goto failed;
	}

	kd035hvfmd057_init();
	SSI_CS(0);

failed:
	return ret;
}

static int kd035hvfmd057_off_begin(struct lcdc_gpio_struct *gpio)
{
	return 0;
}

static struct lcd_callback_ops kd035hvfmd057_ops = {
	.lcd_power_on_begin  = (void*)kd035hvfmd057_on_begin,
	.lcd_power_off_begin = (void*)kd035hvfmd057_off_begin,
};


static struct fb_videomode jzfb_kd035hvfmd057_videomode = {
	.name = "320x480",
	.refresh = 60,
	.xres = 320,
	.yres = 480,
	.pixclock = KHZ2PICOS(33264),
	.left_margin  = 3,
	.right_margin = 60,
	.upper_margin = 2,
	.lower_margin = 2,
	.hsync_len = 20,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

static struct jzfb_tft_config kd035hvfmd057_cfg = {
	.pix_clk_inv = 0,
	.de_dl = 0,
	.sync_dl = 0,
	.color_even = TFT_LCD_COLOR_EVEN_RGB,
	.color_odd = TFT_LCD_COLOR_ODD_RGB,
	.mode = TFT_LCD_MODE_PARALLEL_666,
};

struct jzfb_platform_data jzfb_data = {
	.num_modes = 1,
	.modes = &jzfb_kd035hvfmd057_videomode,
	.lcd_type = LCD_TYPE_TFT,
	.bpp = 24,
	.width = 320,
	.height = 480,

	.tft_config = &kd035hvfmd057_cfg,

	.dither_enable = 0,
	.dither.dither_red = 0,
	.dither.dither_green = 0,
	.dither.dither_blue = 0,
	.lcd_callback_ops = &kd035hvfmd057_ops,
};


/****************power and  backlight*********************/

#define MAX_BRIGHTNESS_STEP     16                              /* SGM3146 supports 16 brightness
								   step */
#define CONVERT_FACTOR          (256/MAX_BRIGHTNESS_STEP)       /* System support 256 brightness
								   step */

struct kd035hvfmd057_power {
	struct regulator *vlcdio;
	struct regulator *vlcdvcc;
	int inited;
};

static struct kd035hvfmd057_power lcd_power = {
	NULL,
	NULL,
	0
};

static int kd035hvfmd057_update_status(struct backlight_device *bd)
{
	int brightness = bd->props.brightness;
	unsigned int i;
	int pulse_num = MAX_BRIGHTNESS_STEP - brightness / CONVERT_FACTOR - 1;

	if (bd->props.fb_blank == FB_BLANK_POWERDOWN) {
		return 0;
	}

	if (bd->props.state & BL_CORE_SUSPENDED)
		brightness = 0;

	if (brightness) {
		gpio_direction_output(GPIO_LCD_PWM,0);
		udelay(5000);
		gpio_direction_output(GPIO_LCD_PWM,1);
		udelay(100);

		for (i = pulse_num; i > 0; i--) {
			gpio_direction_output(GPIO_LCD_PWM,0);
			udelay(1);
			gpio_direction_output(GPIO_LCD_PWM,1);
			udelay(3);
		}
	} else
		gpio_direction_output(GPIO_LCD_PWM, 0);

	bd->props.brightness = brightness;
	return 0;
}

static int kd035hvfmd057_get_brightness(struct backlight_device *bd)
{
	bd->props.brightness = 0;
	return bd->props.brightness;
}

static struct backlight_ops kd035hvfmd057_backlight_ops = {
	.options = BL_CORE_SUSPENDRESUME,
	.update_status = kd035hvfmd057_update_status,
	.get_brightness = kd035hvfmd057_get_brightness,
};

static int kd035hvfmd057_power_init(void)
{
	int ret = 0;
	ret = gpio_request(GPIO_LCD_RST, "lcd rst");
	if (ret) {
		printk(KERN_ERR "can's request lcd rst\n");
		goto failed;
	}

	ret = gpio_request(GPIO_LCD_CS, "lcd cs");
	if (ret) {
		printk(KERN_ERR "can's request lcd cs\n");
		goto failed;
	}

	ret = gpio_request(GPIO_LCD_PWM, "lcd backlight pwm");
	if (ret) {
		printk(KERN_ERR "failed to reqeust lcd backlight pwm\n");
		goto failed;
	}

	lcd_power.inited = 1;

failed:
	return ret;
}

static int kd035hvfmd057_power_reset(struct lcd_device *ld)
{
	if (!lcd_power.inited)
		return -EFAULT;

	gpio_direction_output(GPIO_LCD_RST, 0);
	udelay(3);
	gpio_direction_output(GPIO_LCD_RST, 1);
	mdelay(1);
	gpio_direction_output(GPIO_LCD_RST, 0);
	udelay(10);
	gpio_direction_output(GPIO_LCD_RST, 1);
	mdelay(120);

	return 0;
}

static int kd035hvfmd057_power_on(struct lcd_device *ld, int enable)
{
	if (!lcd_power.inited && kd035hvfmd057_power_init())
		return -EFAULT;

	if (enable) {
		gpio_direction_output(GPIO_LCD_RST, 1);
		gpio_direction_output(GPIO_LCD_CS, 1);

		kd035hvfmd057_power_reset(NULL);

		gpio_direction_output(GPIO_LCD_CS, 0);
	} else {
		gpio_direction_output(GPIO_LCD_CS, 1);
		gpio_direction_output(GPIO_LCD_RST, 0);
	}
	return 0;
}

static struct lcd_platform_data kd035hvfmd057_pdata = {
	.reset    = kd035hvfmd057_power_reset,
	.power_on = kd035hvfmd057_power_on,
	.pdata = &kd035hvfmd057_backlight_ops,
};

/* LCD Panel Device */
static struct platform_device kd035hvfmd057_device = {
	.name           = "kd035hvfmd057_tft",
	.dev            = {
		.platform_data  = &kd035hvfmd057_pdata,
	},
};

int __init jzfb_backlight_init(void)
{
	platform_device_register(&kd035hvfmd057_device);
	return 0;
}

static void __exit jzfb_backlight_exit(void)
{
	platform_device_unregister(&kd035hvfmd057_device);
}
rootfs_initcall(jzfb_backlight_init);
module_exit(jzfb_backlight_exit);

MODULE_DESCRIPTION("JZ LCD Controller backlight Register");
MODULE_LICENSE("GPL");

