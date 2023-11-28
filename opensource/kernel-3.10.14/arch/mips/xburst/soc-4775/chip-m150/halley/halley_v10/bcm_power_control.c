#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/jzmmc.h>
#include <linux/delay.h>
#include "board.h"

static void wifi_le_set_io(void)
{
	jzgpio_set_func(GPIO_PORT_B, GPIO_INPUT, (0x3<<20)|(0xF<<30));
}

static void wifi_le_restore_io(void)
{
	jzgpio_set_func(GPIO_PORT_B, GPIO_FUNC_0, (0x3<<20)|(0xF<<30));
}

#define RESET               0
#define NORMAL              1

int bcm_wlan_power_on(int flag)
{
	int wl_reg_on	= WL_REG_EN;
	pr_debug("wlan power on:%d\n", flag);
	wifi_le_restore_io();
	switch(flag) {
		case RESET:
			gpio_direction_output(wl_reg_on,1);
			break;
		case NORMAL:
			gpio_request(wl_reg_on, "wl_reg_on");
			gpio_direction_output(wl_reg_on,0);/* halley reset no power down ,need sofe control wifi reg on*/
			gpio_direction_output(wl_reg_on,1);
			jzmmc_manual_detect(2, 1);
			break;
	}
	return 0;
}
EXPORT_SYMBOL(bcm_wlan_power_on);

int bcm_wlan_power_off(int flag)
{
	int wl_reg_on = WL_REG_EN;
	pr_debug("wlan power off:%d\n", flag);
	switch(flag) {
		case RESET:
			gpio_direction_output(wl_reg_on,0);
			break;

		case NORMAL:
			/* *  control wlan reg on pin */
			gpio_direction_output(wl_reg_on,0);
			break;
	}
	wifi_le_set_io();
	return 0;
}
EXPORT_SYMBOL(bcm_wlan_power_off);
