/*
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * M200 dorado board lcd setup routines.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/lcd.h>
#include <linux/regulator/consumer.h>

#include <mach/jzfb.h>
#include "../board_base.h"

struct cv90_m5377_p30_power{
	struct regulator *vlcdio;
	struct regulator *vlcdvcc;
	int inited;
};

static struct cv90_m5377_p30_power lcd_power = {
	NULL,
	NULL,
	0
};

int cv90_m5377_p30_power_init(struct lcd_device *ld)
{
    int ret ;

    if(GPIO_LCD_RST > 0){
        ret = gpio_request(GPIO_LCD_RST, "lcd rst");
        if (ret) {
            printk(KERN_ERR "can's request lcd rst\n");
            return ret;
        }
    }
    if(GPIO_LCD_BLK > 0){
        ret = gpio_request(GPIO_LCD_BLK, "lcd blk");
        if (ret) {
            printk(KERN_ERR "can's request lcd rst\n");
            return ret;
        }
    }
    if(SLCD_NBUSY_PIN > 0){
        ret = gpio_request(SLCD_NBUSY_PIN, "lcd nbusy pin");
        if (ret) {
            printk(KERN_ERR "can's request lcd nbusy pin\n");
            return ret;
        }
    }

#ifdef GPIO_DEBUG
    if(GPIO_LCD_NRD_E > 0){
        ret = gpio_request(GPIO_LCD_NRD_E, "lcd nrd_e");
        if (ret) {
            printk(KERN_ERR "can's request lcd nrd_e\n");
            return ret;
        }
    }
    if(GPIO_LCD_DNC > 0){
        ret = gpio_request(GPIO_LCD_DNC, "lcd dnc");
        if (ret) {
            printk(KERN_ERR "can's request lcd dnc\n");
            return ret;
        }
    }
    if(GPIO_LCD_NWR_SCL > 0){
        ret = gpio_request(GPIO_LCD_NWR_SCL, "lcd dwr_scl");
        if (ret) {
            printk(KERN_ERR "can's request lcd dwr_scl\n");
            return ret;
        }
    }
#endif
    lcd_power.inited = 1;
    return 0;
}

int cv90_m5377_p30_power_reset(struct lcd_device *ld)
{
	if (!lcd_power.inited)
		return -EFAULT;
	gpio_direction_output(GPIO_LCD_RST, 0);  //reset active low
	mdelay(20);
	gpio_direction_output(GPIO_LCD_RST, 1);
	mdelay(250);
	return 0;
}

static int slcd_cv90_m5377_p30_p1_waiting_NBUSY_rising_high(void)
{
	int count = 0;
	int ret = 0;

	while ( count++ < 100 ) {
		ret = gpio_get_value_cansleep(SLCD_NBUSY_PIN);
		if (ret)
			return 1;
		else
			printk(KERN_ERR "%s -> gpio_get_value_cansleep() return val= %d\n",__func__, ret);
		mdelay(1);
	}

	return 0;
}


int cv90_m5377_p30_power_on(struct lcd_device *ld, int enable)
{
	if (!lcd_power.inited && cv90_m5377_p30_power_init(ld))
		return -EFAULT;

	/*enable backlight, set it according to your board */
	gpio_direction_output(GPIO_LCD_BLK, 1);
	/*NRD_E, must be high, either setting gpio or setting high on your board.*/
	//gpio_direction_output(GPIO_LCD_NRD_E, 1);
	/*reset your panel*/
	cv90_m5377_p30_power_reset(ld);

#ifdef  GPIO_DEBUG
	printk("start important gpio test!\n");
	while(1) {
		gpio_direction_output(GPIO_LCD_DNC, 1);
		gpio_direction_output(GPIO_LCD_NWR_SCL, 0);
		mdelay(10);
		gpio_direction_output(GPIO_LCD_DNC, 0);
		gpio_direction_output(GPIO_LCD_NWR_SCL, 1);
		mdelay(2);
	}
#endif

#if 0
	if (enable) {
		/*fixed*/
		printk("======cv90_m5377_p30_power_on ==enable==\n");
	} else {
		/*fixed*/
		printk("======cv90_m5377_p30_power_on ====disable==\n");
	}
#endif
	return 0;
}

struct lcd_platform_data cv90_m5377_p30_pdata = {
	.reset    = cv90_m5377_p30_power_reset,
	.power_on = cv90_m5377_p30_power_on,
};

/* LCD Panel Device */
struct platform_device cv90_m5377_p30_device = {
	.name		= "cv90_m5377_p30_slcd",
	.dev		= {
		.platform_data	= &cv90_m5377_p30_pdata,
	},
};

static struct smart_lcd_data_table cv90_m5377_p30_data_table[] = {

	/* Extended CMD enable CMD */
	{0, 0xB9},

	{1, 0xff},
	{1, 0x52},
	{1, 0x52},

	/* sleep out command, and wait > 35ms */
	{0, 0x11},
	{2, 200000},

	/* Set Pixel Format Cmd */
	{0, 0x3a},
	//{1, 0x01},	/* 6bit */
	//   {1, 0x05},	/* 16bit */
	{1, 0x06},	/* 24bit? */

	/* Pixel Format = 6bit? */

#if 1
	/* Enable CP */
	{0, 0xF4},
	{1, 0x01},	/* Enable CP */
	{1, 0x01},	/* Photo Mode */
	{1, 0x02},	/* Floyd Steinberg */
#else
	/* Disable CP*/
	{0, 0xF4},
	{1, 0x00},	/* Disable CP */
	{1, 0x00},	/* Photo Mode */
	{1, 0x00},	/* Floyd Steinberg */
#endif

	/* RAM Comma*/
	{1, 0xbd},
	{1, 0x00},


	/* Display On */
	{0, 0x29},



	/* polling NBUSY, proceed on the rising edge. */
	//{0, 0xcd},
	//{2, 50000},
        // {0, 0x2c},


};

unsigned long cv90_m5377_p30_cmd_buf[]= {
	0x2C2C2CCD,
};


static int cv90_m5377_p30_dma_transfer_begin_callback(void*jzfb)
{
	int ret = 0;
	ret = slcd_cv90_m5377_p30_p1_waiting_NBUSY_rising_high();
	return ret;
}

static int cv90_m5377_p30_dma_transfer_end_callback(void*jzfb)
{
	return 0;
}



struct fb_videomode jzfb_videomode = {
	.name = "288x192",
	.refresh = 45,
	.xres = 288,
	.yres = 192,
	.pixclock = KHZ2PICOS(5000*3), /* SLCDC WR_SCL = LCDC_PIXEL_CLOCK/2, so wr_scl is about 7.5MHZ*/
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

struct jzfb_platform_data jzfb_pdata = {
	.num_modes = 1,
	.modes = &jzfb_videomode,

        .lcd_type = LCD_TYPE_SLCD,
        .bpp = 24,
        .dither_enable = 0,

        .smart_config.clkply_active_rising = 0,
        .smart_config.rsply_cmd_high = 0,
        .smart_config.csply_active_high = 0,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.write_gram_cmd = cv90_m5377_p30_cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(cv90_m5377_p30_cmd_buf),

        .smart_config.bus_width = 8,  //due to panel bus width
	.smart_config.length_data_table = ARRAY_SIZE(cv90_m5377_p30_data_table),
        .smart_config.data_table = cv90_m5377_p30_data_table,
	.smart_config.init = 0,//cv90_m5377_p30_init,

	.lcd_callback_ops.dma_transfer_begin = cv90_m5377_p30_dma_transfer_begin_callback,
	.lcd_callback_ops.dma_transfer_end = cv90_m5377_p30_dma_transfer_end_callback,
};

/**************************************************************************************************/

#ifdef CONFIG_BACKLIGHT_PWM
static int backlight_init(struct device *dev)
{
	int ret;
	ret = gpio_request(GPIO_LCD_PWM, "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPF for PWM-OUT1\n");
		return ret;
	}

//	gpio_direction_output(GPIO_BL_PWR_EN, 1);

	return 0;
}


static void backlight_exit(struct device *dev)
{
	gpio_free(GPIO_LCD_PWM);
}

static struct platform_pwm_backlight_data backlight_data = {
	.pwm_id		= 1,
	.max_brightness	= 255,
	.dft_brightness	= 120,
	.pwm_period_ns	= 30000,
	.init		= backlight_init,
	.exit		= backlight_exit,
};

struct platform_device backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &backlight_data,
	},
};

#endif
