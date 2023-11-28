#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/digital_pulse_backlight.h>
#include <mach/jzfb.h>

#include "../board_base.h"

#ifdef CONFIG_LCD_BYD_8991FTGF
#include <linux/byd_8991.h>
static struct platform_byd_8991_data byd_8991_pdata= {
	.gpio_lcd_disp  = GPIO_LCD_DISP,
	.gpio_lcd_de    = GPIO_LCD_DE,		//GPIO_PC(9),	/* chose sync mode */
	.gpio_lcd_vsync = GPIO_LCD_VSYNC,	//GPIO_PC(19),
	.gpio_lcd_hsync = GPIO_LCD_HSYNC,	//GPIO_PC(18),
	/* spi interface */
	.gpio_lcd_cs  = GPIO_LCD_CS,
	.gpio_lcd_clk = GPIO_LCD_CLK,
	.gpio_lcd_sdo = GPIO_LCD_SDO,
	.gpio_lcd_sdi = GPIO_LCD_SDI,
	.gpio_lcd_back_sel = GPIO_LCD_BACK_SEL,
};

/* LCD device */
struct platform_device byd_8991_device = {
	.name		= "byd_8991-lcd",
	.dev		= {
		.platform_data = &byd_8991_pdata,
	},
};

struct fb_videomode jzfb_8991_videomode = {
	.name = "480x800",
	.refresh = 60,
	.xres = 480,
	.yres = 800,
	.pixclock = KHZ2PICOS(36000),
	.left_margin = 70,
	.right_margin = 70,
	.upper_margin = 2,
	.lower_margin = 2,
	.hsync_len = 42,
	.vsync_len = 11,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct jzfb_platform_data jzfb_pdata = {
	.num_modes = 1,
	.modes = &jzfb_8991_videomode,
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,
	.width = 45,
	.height = 75,
	/*this pixclk_edge is for lcd_controller sending data, which is reverse to lcd*/
	.pixclk_falling_edge = 1,
	.data_enable_active_low = 0,

};

#endif

/**************************************************************************************************/

#ifdef CONFIG_BACKLIGHT_PWM
static int backlight_init(struct device *dev)
{
	int ret;
	return 0;
	ret = gpio_request(GPIO_LCD_PWM, "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPF for PWM-OUT1\n");
		return ret;
	}

	/* Configure GPIO pin with S5P6450_GPF15_PWM_TOUT1 */
	//gpio_direction_output(GPIO_LCD_PWM, 1);

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

/***********************************************************************************************/
#ifdef CONFIG_BACKLIGHT_DIGITAL_PULSE
static int init_backlight(struct device *dev)
{
	return 0;
}
static void exit_backlight(struct device *dev)
{

}

struct platform_digital_pulse_backlight_data bl_data = {
	.digital_pulse_gpio = GPIO_GIGITAL_PULSE,
	.max_brightness = 255,
	.dft_brightness = 120,
	.max_brightness_step = 16,
	.high_level_delay_us = 5,
	.low_level_delay_us = 5,
	.init = init_backlight,
	.exit = exit_backlight,
};


struct platform_device digital_pulse_backlight_device = {
	.name		= "digital-pulse-backlight",
	.dev		= {
		.platform_data	= &bl_data,
	},
};
#endif
