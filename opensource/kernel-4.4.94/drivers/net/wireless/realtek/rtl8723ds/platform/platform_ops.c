/******************************************************************************
 *
 * Copyright(c) 2013 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef CONFIG_PLATFORM_OPS

#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define normal 1

extern  int bcm_wlan_power_off(int);
extern  int bcm_wlan_power_on(int);
extern int bcm_ap6212_wlan_init(void);

/*
 * Return:
 *	0:	power on successfully
 *	others: power on failed
 */
int platform_wifi_power_on(void)
{
#if 0
    int ret = 0;
    ret = gpio_request(84, "bt");
    if (ret < 0) {
        printk("gpio 83 request error!!!\n");
    }
    gpio_direction_output(84, 0);
    printk("gpio_direction_output fun low %s\n", __func__);
    msleep(1000);
    gpio_direction_output(84, 1);
    printk("gpio_direction_output fun high %s\n", __func__);
    msleep(1000);
	gpio_free(84);
#endif

#if 0

    ret = bcm_ap6212_wlan_init();
    if (ret < 0) {
        printk("=========== platform_ops.c:bcm_ap6212_wlan_init error! ===========\n");
    }
    ret = bcm_wlan_power_on(normal);
    if (ret < 0) {
	    printk("=========== WLAN placed in ERROR ON========\n");
    } else {
	    printk("=========== WLAN placed in NOMAL ON========\n");
    }

	return ret;
#endif
	return 0;
}

void platform_wifi_power_off(void)
{
#if 0
    int ret = 0;
    ret = bcm_wlan_power_off(normal);
    if (ret < 0) {
	    printk("=========== WLAN placed in ERROR OFF ========\n");
    } else {
	    printk("=========== WLAN placed in NOMAL OFF ========\n");
    }
#endif
}

#endif /* !CONFIG_PLATFORM_OPS */
