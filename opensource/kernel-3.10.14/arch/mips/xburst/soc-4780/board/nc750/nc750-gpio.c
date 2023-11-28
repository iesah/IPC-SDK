/* Ingenic gpiolib
 *
 * GPIOlib support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <linux/syscore_ops.h>
#include <irq.h>

#include <soc/base.h>
#include <soc/gpio.h>
#include <soc/irq.h>
#include "nc750.h"

/*
 * GPD control on/off
 */
static int gpd_read_proc (char *page, char **start, off_t off,
                         int count, int *eof, void *data)
{
        int gpd=0;
        //int len = 0;
        //gpd = __gpio_get_pin(3*32 + 8);
        printk("gpd 8 status is %d\n",gpd);
        //sprintf (page+len, "gpd 8 status is %d\n", gpd);
        return 0;
}
static int gpd_write_proc (struct file *file, const char *buffer, unsigned long count, void *data)
{
        char *buf1 = "1";
        char *buf2 = "0";
        if(strncmp(buffer,buf1,1) == 0){
                //__gpio_port_as_output1(3,8);
                jzgpio_set_func(GPIO_PORT_D, GPIO_OUTPUT1, 8);
                jz_gpio_set_func(104,GPIO_OUTPUT1);
                printk("gpio D enable 8 pull success!\n");
        }
        if(strncmp(buffer,buf2,1) == 0){
                //__gpio_port_as_output0(3,8);
                jzgpio_set_func(GPIO_PORT_D, GPIO_OUTPUT0, 8);
                jz_gpio_set_func(104,GPIO_OUTPUT0);
                printk("gpio D disable 8 pull success!\n");
        }
        return count;
}


static int __init init_gpiod_proc(void)
{
	struct proc_dir_entry *res;
	
	 /* gpio D en/dis able pull*/
        res = create_proc_entry("gpd", 0666, NULL);
        if (res) {
                res->read_proc = gpd_read_proc;
                res->write_proc = gpd_write_proc;
                res->data = NULL;
        }

        //__gpio_port_as_output1(3,8);
        jzgpio_set_func(GPIO_PORT_D, GPIO_OUTPUT1, 8);
        jz_gpio_set_func(104,GPIO_OUTPUT1);
        printk("gpio D enable 8 pull finish\n");

	return 0;
}

module_init(init_gpiod_proc);
