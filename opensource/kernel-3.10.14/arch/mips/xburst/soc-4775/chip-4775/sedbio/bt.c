#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/bcm4330-rfkill.h>

#include "board.h"


#define GPIO_BT_RST_N    GPIO_PG(8)
#define GPIO_BT_REG_ON   GPIO_PG(11)
#define GPIO_BT_WAKE	 GPIO_PG(7)
#define GPIO_BT_INT		 GPIO_PG(9)





static struct bcm4330_rfkill_platform_data  gpio_data = {

	.gpio = {

 		.bt_rst_n = GPIO_BT_RST_N ,
		.bt_reg_on = GPIO_BT_REG_ON ,
		.bt_wake = GPIO_BT_WAKE ,
		.bt_int = GPIO_BT_INT ,
//		.bt_wake = GPIO_PF(3) ,
//		.bt_int = GPIO_PF(0),
	},


};


struct platform_device bcm4330_bt_power_device  = {
	.name = "bcm4330_bt_power" ,
	.id = -1 ,
	.dev   = {
		.platform_data = &gpio_data,
	},
};













