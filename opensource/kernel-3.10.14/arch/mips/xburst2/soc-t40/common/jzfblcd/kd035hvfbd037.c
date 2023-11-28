/*
 * ingenic_bsp/chip-x2000/fpga/dpu/truly_240_240.c
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
/*#include <linux/backlight.h>*/
#include <linux/lcd.h>
#include <mach/jzfb.h>
#include <soc/gpio.h>
#include <board.h>

static struct smart_lcd_data_table kd035hvfbd037_data_table[] = {
/* LCD init code */
	{SMART_CONFIG_UDELAY, 120000},
	{SMART_CONFIG_UDELAY, 120000},
	{SMART_CONFIG_CMD    , 0xff},   //Command 2 Enable
	{SMART_CONFIG_PRM    , 0x48},
	{SMART_CONFIG_PRM   , 0x02},
	{SMART_CONFIG_PRM   , 0x01},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x80},
	{SMART_CONFIG_CMD    , 0xff},  //ORISE Command Enable
	{SMART_CONFIG_PRM   , 0x48},
	{SMART_CONFIG_PRM   , 0x02},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x90},
	{SMART_CONFIG_CMD    , 0xFF},  //MPU 16bit setting
	{SMART_CONFIG_PRM   , 0x01},	//02-16BIT MCU,01-8BIT MCU

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x93},
	{SMART_CONFIG_CMD    , 0xFF},  //SW MPU enable
	{SMART_CONFIG_PRM   , 0x20},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_CMD    , 0x51},    //Wright Display brightness
	{SMART_CONFIG_PRM   , 0xf0},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_CMD    , 0x53},   // Wright CTRL Display
	{SMART_CONFIG_PRM   , 0x24},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM  , 0xb1},
	{SMART_CONFIG_CMD    , 0xc5},   //VSEL setting
	{SMART_CONFIG_PRM   , 0x00},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0xB0},
	{SMART_CONFIG_CMD    , 0xc4},   //Gate Timing control
	{SMART_CONFIG_PRM   , 0x02},
	{SMART_CONFIG_PRM   , 0x08},
	{SMART_CONFIG_PRM   , 0x05},
	{SMART_CONFIG_PRM   , 0x00},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x90},
	{SMART_CONFIG_CMD    , 0xc0},   //TCON MCLK Shift Control
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x0f},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x15},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x17},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x82},
	{SMART_CONFIG_CMD    , 0xc5},  //Adjust pump phase
	{SMART_CONFIG_PRM   , 0x01},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x90},
	{SMART_CONFIG_CMD    , 0xc5},  //Adjust pump phase
	{SMART_CONFIG_PRM   , 0x47},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_CMD    , 0xd8},  //GVDD/NGVDD Setting
	{SMART_CONFIG_PRM   , 0x58},  //58,17V
	{SMART_CONFIG_PRM   , 0x58},  //58

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_CMD    , 0xd9},  //VCOM Setting
	{SMART_CONFIG_PRM   , 0xb0},  //

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x91},
	{SMART_CONFIG_CMD    , 0xb3},  //Display setting
	{SMART_CONFIG_PRM   , 0xC0},
	{SMART_CONFIG_PRM   , 0x25},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x81},
	{SMART_CONFIG_CMD    , 0xC1}, //Osillator Adjustment:70Hz
	{SMART_CONFIG_PRM   , 0x77},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_CMD    , 0xe1},   //Gamma setting                     ( positive)
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x05},
	{SMART_CONFIG_PRM   , 0x09},
	{SMART_CONFIG_PRM   , 0x04},
	{SMART_CONFIG_PRM   , 0x02},
	{SMART_CONFIG_PRM   , 0x0b},
	{SMART_CONFIG_PRM   , 0x0a},
	{SMART_CONFIG_PRM   , 0x09},
	{SMART_CONFIG_PRM   , 0x05},
	{SMART_CONFIG_PRM   , 0x08},
	{SMART_CONFIG_PRM   , 0x10},
	{SMART_CONFIG_PRM   , 0x05},
	{SMART_CONFIG_PRM   , 0x06},
	{SMART_CONFIG_PRM   , 0x11},
	{SMART_CONFIG_PRM   , 0x09},
	{SMART_CONFIG_PRM   , 0x01},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_CMD    , 0xe2},  //Gamma setting                      ( negative)
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x05},
	{SMART_CONFIG_PRM   , 0x09},
	{SMART_CONFIG_PRM   , 0x04},
	{SMART_CONFIG_PRM   , 0x02},
	{SMART_CONFIG_PRM   , 0x0b},
	{SMART_CONFIG_PRM   , 0x0a},
	{SMART_CONFIG_PRM   , 0x09},
	{SMART_CONFIG_PRM   , 0x05},
	{SMART_CONFIG_PRM   , 0x08},
	{SMART_CONFIG_PRM   , 0x10},
	{SMART_CONFIG_PRM   , 0x05},
	{SMART_CONFIG_PRM   , 0x06},
	{SMART_CONFIG_PRM   , 0x11},
	{SMART_CONFIG_PRM   , 0x09},
	{SMART_CONFIG_PRM   , 0x01},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_CMD    , 0x00},  //End Gamma setting
	{SMART_CONFIG_PRM   , 0x00},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x80},
	{SMART_CONFIG_CMD    , 0xff}, //Orise mode  command Disable
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x00},

	{SMART_CONFIG_CMD    , 0x00},
	{SMART_CONFIG_PRM   , 0x00},

	{SMART_CONFIG_CMD    , 0xff}, //Command 2 Disable
	{SMART_CONFIG_PRM   , 0xff},
	{SMART_CONFIG_PRM   , 0xff},
	{SMART_CONFIG_PRM   , 0xff},

	//{SMART_CONFIG_CMD  , 0x35}, //TE ON
	//{SMART_CONFIG_DATA , 0x00},

	{SMART_CONFIG_CMD    , 0x36}, //set X Y refresh direction
	{SMART_CONFIG_PRM   , 0x00},

	{SMART_CONFIG_CMD    , 0x3A},    //16-bit/pixe 565
	{SMART_CONFIG_PRM   , 0x05},

	{SMART_CONFIG_CMD    , 0x2A}, //Frame rate control	320
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x01},
	{SMART_CONFIG_PRM   , 0x3F},

	{SMART_CONFIG_CMD    , 0x2B}, //Display function control	 480
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x00},
	{SMART_CONFIG_PRM   , 0x01},
	{SMART_CONFIG_PRM   , 0xDF},

	{SMART_CONFIG_CMD    , 0x11},
	{SMART_CONFIG_UDELAY , 120000},
	{SMART_CONFIG_UDELAY , 120000},
	{SMART_CONFIG_CMD    , 0x29}, //display on

	{SMART_CONFIG_CMD    , 0x2c},

};

static struct fb_videomode jzfb_kd035hvfbd037_videomode = {
	.name = "320x480",
	.refresh = 60,
	.xres = 320,
	.yres = 480,
	.pixclock = KHZ2PICOS(18432),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct jzfb_smart_config kd035hvfbd037_cfg = {
	.frm_md = 0,
	.rdy_anti_jit = 0,
	.te_anti_jit = 1,
	.te_md = 0,
	.te_switch = 0,
	.rdy_switch = 0,
	.cs_en = 0,
	.cs_dp = 0,
	.rdy_dp = 1,
	.dc_md = 0,
	.wr_md = 1,
	.te_dp = 1,
	.smart_type = SMART_LCD_TYPE_8080,
	.pix_fmt = SMART_LCD_FORMAT_565,
	.dwidth = SMART_LCD_DWIDTH_8_BIT,
	.cwidth = SMART_LCD_CWIDTH_8_BIT,
	.bus_width = 8,

	.write_gram_cmd = 0x2c,
	.data_table = kd035hvfbd037_data_table,
	.length_data_table = ARRAY_SIZE(kd035hvfbd037_data_table),
};

struct jzfb_platform_data jzfb_data = {
	.num_modes = 1,
	.modes = &jzfb_kd035hvfbd037_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp = 32,
	.width = 320,
	.height = 480,

	.smart_config = &kd035hvfbd037_cfg,

	.dither_enable = 0,
	.dither.dither_red = 0,
	.dither.dither_green = 0,
	.dither.dither_blue = 0,
	.lcd_callback_ops = NULL, //&otm4802a_ops,
};

/****************power and  backlight*********************/

#define MAX_BRIGHTNESS_STEP     16                              /* SGM3146 supports 16 brightness
								   step */
#define CONVERT_FACTOR          (256/MAX_BRIGHTNESS_STEP)       /* System support 256 brightness
								   step */
struct kd035hvfbd037_power{
	struct regulator *vlcdio;
	struct regulator *vlcdvcc;
	int inited;
};

static struct kd035hvfbd037_power lcd_power = {
	NULL,
	NULL,
	0
};

static int kd035hvfbd037_update_status(struct backlight_device *bd)
{
	int brightness = bd->props.brightness;
	unsigned int i;
	int pulse_num = MAX_BRIGHTNESS_STEP - brightness / CONVERT_FACTOR - 1;
	/*
	 *                                                                                    if (bd->props.power != FB_BLANK_UNBLANK)
	 *                                                                                                    brightness = 0;
	 *
	 *                                                                                                                                                                                          if (bd->props.fb_blank != FB_BLANK_UNBLANK)
	 *                                                                                                                                                                                                          brightness = 0;
	 *                                                                                                                                                                                                          */



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

static int kd035hvfbd037_get_brightness(struct backlight_device *bd)
{
	bd->props.brightness = 0;
	return bd->props.brightness;
}

static struct backlight_ops kd035hvfbd037_backlight_ops = {
	.options = BL_CORE_SUSPENDRESUME,
	.update_status = kd035hvfbd037_update_status,
	.get_brightness = kd035hvfbd037_get_brightness,
};

int kd035hvfbd037_power_init(struct lcd_device *ld)
{
	int ret ;
	printk("======power_init==============\n");
	if(GPIO_LCD_RST > 0){
	    ret = gpio_request(GPIO_LCD_RST, "lcd rst");
	    if (ret) {
		printk(KERN_ERR "can's request lcd rst\n");
		    return ret;
	    }
	}
	if(GPIO_LCD_CS > 0){
	    ret = gpio_request(GPIO_LCD_CS, "lcd cs");
	    if (ret) {
		printk(KERN_ERR "can's request lcd cs\n");
		return ret;
	    }
	}
#if 0
	if(GPIO_LCD_RD > 0){
	    ret = gpio_request(GPIO_LCD_RD, "lcd rd");
	    if (ret) {
		printk(KERN_ERR "can's request lcd rd\n");
		return ret;
	    }
	}
#endif
	if(GPIO_LCD_PWM > 0){
	    ret = gpio_request(GPIO_LCD_PWM, "BL PULSE");
	    if (ret) {
		printk(KERN_ERR "failed to reqeust BL PULSE\n");
		return ret;
	    }
	}
	if(GPIO_LCD_VDD_EN > 0){
	    ret = gpio_request(GPIO_LCD_VDD_EN, "lcd vdd en");
	    if (ret) {
		printk(KERN_ERR "can's request lcd rd\n");
		return ret;
	    }
	}
#if 0
	if(GPIO_BL_PWR_EN > 0){
	    ret = gpio_request(GPIO_BL_PWR_EN, "BL PWR");
	    if (ret) {
		printk(KERN_ERR "failed to reqeust BL PWR\n");
		return ret;
	    }
	}
	if(GPIO_TP_EN > 0){
	    ret = gpio_request(GPIO_TP_EN, "TP EN");
	    if (ret) {
		printk(KERN_ERR "failed to reqeust TP EN\n");
		return ret;
	    }
	}
#endif

	printk("set lcd_power.inited  =======1 \n");
	lcd_power.inited = 1;
	return 0;

}

int kd035hvfbd037_power_reset(struct lcd_device *ld)
{
	if (!lcd_power.inited)
		return -EFAULT;
	gpio_direction_output(GPIO_LCD_RST, 0);
	mdelay(500);
	gpio_direction_output(GPIO_LCD_RST, 1);
	mdelay(10);

	return 0;
}

static int kd035hvfbd037_power_on(struct lcd_device *ld, int enable)
{
	if (!lcd_power.inited && kd035hvfbd037_power_init(ld))
		return -EFAULT;

	if (enable) {
		if(GPIO_LCD_VDD_EN > 1)
			gpio_direction_output(GPIO_LCD_VDD_EN, 1);
		gpio_direction_output(GPIO_LCD_RST, 1);
		gpio_direction_output(GPIO_LCD_CS, 1);

		if(GPIO_LCD_RD > 1){
			gpio_direction_output(GPIO_LCD_RD, 1);
		}

		mdelay(5);
		kd035hvfbd037_power_reset(ld);

		gpio_direction_output(GPIO_LCD_CS, 0);
		/* gpio_direction_output(GPIO_BL_PWR_EN, 1); */
/*
		if(GPIO_TP_EN > 1){
			gpio_direction_output(GPIO_TP_EN, 1);
		}
*/
	} else {
/*
		if(GPIO_TP_EN > 1){
			gpio_direction_output(GPIO_TP_EN, 0);
		}
*/
		/* gpio_direction_output(GPIO_BL_PWR_EN, 0); */
		mdelay(5);
		gpio_direction_output(GPIO_LCD_CS, 0);

		if(GPIO_LCD_RD > 1){
			gpio_direction_output(GPIO_LCD_RD, 0);
		}

		gpio_direction_output(GPIO_LCD_RST, 0);
		if(GPIO_LCD_VDD_EN > 1)
			gpio_direction_output(GPIO_LCD_VDD_EN, 0);
	}
	return 0;
}

struct lcd_platform_data kd035hvfbd037_pdata = {
	.reset    = kd035hvfbd037_power_reset,
	.power_on = kd035hvfbd037_power_on,
	.pdata = &kd035hvfbd037_backlight_ops,
};

/* LCD Panel Device */
static struct platform_device kd035hvfbd037_device = {
	.name           = "kd035hvfbd037_slcd",
	.dev            = {
		.platform_data  = &kd035hvfbd037_pdata,
	},
};

int __init jzfb_backlight_init(void)
{
	platform_device_register(&kd035hvfbd037_device);
	return 0;
}

static void __exit jzfb_backlight_exit(void)
{
	platform_device_unregister(&kd035hvfbd037_device);
}
rootfs_initcall(jzfb_backlight_init);
/*module_init(jzfb_backlight_init);*/
module_exit(jzfb_backlight_exit);

MODULE_DESCRIPTION("JZ LCD Controller backlight  Register");
MODULE_LICENSE("GPL");
