#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/jzmmc.h>
#include <linux/bcm_pm_core.h>
#include <linux/delay.h>

#include "board.h"

#ifdef CONFIG_BCM_PM_CORE
static void enable_clk32k(void)
{
	jzrtc_enable_clk32k();
}

static void disable_clk32k(void)
{
	jzrtc_disable_clk32k();
}

static struct bcm_power_platform_data bcm_power_platform_data = {
	.wlan_pwr_en = -1,
	.clk_enable = enable_clk32k,
	.clk_disable = disable_clk32k,
};

struct platform_device	bcm_power_platform_device = {
	.name = "bcm_power",
	.id = -1,
	.num_resources = 0,
	.dev = {
		.platform_data = &bcm_power_platform_data,
	},
};
#endif

/*For BlueTooth*/
#ifdef CONFIG_BROADCOM_RFKILL
#include <linux/bt-rfkill.h>
/*restore BT_UART_RTS when resume because it is output 0 when suspend*/
static void restore_pin_status(int bt_power_state)
{
	jzgpio_set_func(BLUETOOTH_UART_GPIO_PORT, BLUETOOTH_UART_GPIO_FUNC, BLUETOOTH_UART_FUNC_SHIFT);
}

static struct bt_rfkill_platform_data  bt_gpio_data = {
	.gpio = {
		.bt_rst_n = HOST_BT_RST,
		.bt_reg_on = BT_REG_EN,
		.bt_wake = HOST_WAKE_BT,
		.bt_int = BT_WAKE_HOST,
		.bt_uart_rts = BT_UART_RTS,
#if 0
		.bt_int_flagreg = -1,
		.bt_int_bit = -1,
#endif
	},

	.restore_pin_status = restore_pin_status,
	.set_pin_status = NULL,
#if 0
	.suspend_gpio_set = NULL,
	.resume_gpio_set = NULL,
#endif
};

struct platform_device bt_power_device  = {
	.name = "bt_power" ,
	.id = -1 ,
	.dev   = {
		.platform_data = &bt_gpio_data,
	},
};

struct platform_device bluesleep_device = {
	.name = "bluesleep" ,
	.id = -1 ,
	.dev   = {
		.platform_data = &bt_gpio_data,
	},

};

#ifdef CONFIG_BT_BLUEDROID_SUPPORT
int bluesleep_tty_strcmp(const char* name)
{
	if(!strcmp(name,BLUETOOTH_UPORT_NAME)){
		return 0;
	} else {
		return -1;
	}
}
EXPORT_SYMBOL(bluesleep_tty_strcmp);
#endif
#endif /* CONFIG_BROADCOM_RFKILL */
