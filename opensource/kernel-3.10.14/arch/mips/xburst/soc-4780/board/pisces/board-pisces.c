/*
 *  Copyright (C) 2010 Ingenic Semiconductor Inc.
 *
 *   This a board special file, it is used for organizing
 * the resources on the chip and defining the device outside
 * chip.
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/resource.h>

#include <jzsoc/soc-dev.h>
#include <jzsoc/intc-irq.h>
#include <jzsoc/jz_gpio.h>

#include "soc-4770.h"

/* device IO define array */
struct jz_gpio_func_def board_devio_array[] = {
	UART2_PORTC,
	MSC0_PORTA,
	MSC1_PORTD,
	MSC2_PORTE,
	I2C0_PORTD,
	I2C1_PORTE,
	I2C2_PORTF,
	LCD_PORTC,
	PWM1_PORTE,
	MII_PORTBDF,
};
int board_devio_array_size = ARRAY_SIZE(board_devio_array);

#define GPIO_SD1_CD	GPIO_PB(2)
#define GPIO_SD1_PWR	GPIO_PE(9)
#define GPIO_SD2_CD	GPIO_PA(25)
#define GPIO_SD2_PWR	GPIO_PF(20)


#include <mach/jzmsc.h>
#define DEF_MSC(NO,WP,WPE,CD,CDE,PW,PWE)				\
	static struct jzmsc_data jzmsc##NO##_data = {			\
		.pin_wp = { WP, WPE },					\
		.pin_cd = { CD, CDE },					\
		.pin_pwr = { PW, PWE },					\
	}
DEF_MSC(0, -1,0, -1,0, -1,0);
DEF_MSC(1, -1,0, GPIO_SD1_CD,MSC_EL_LOW, GPIO_SD1_PWR,MSC_EL_LOW);
DEF_MSC(2, -1,0, GPIO_SD2_CD,MSC_EL_LOW, GPIO_SD2_PWR,MSC_EL_LOW);


static struct platform_device *jz_platform_devices[] __initdata = {
	&jzsoc_dmac_device,
	&jzsoc_msc0_device,
	&jzsoc_msc1_device,
	&jzsoc_msc2_device,
	&jzsoc_mac_device,
};

static int __init jzsoc_board_init(void) 
{
	int ret = 0;

	jzsoc_msc0_device.dev.platform_data = &jzmsc0_data;
	jzsoc_msc1_device.dev.platform_data = &jzmsc1_data;
	jzsoc_msc2_device.dev.platform_data = &jzmsc2_data;

	ret = platform_add_devices(jz_platform_devices,
                                   ARRAY_SIZE(jz_platform_devices));

	/* register a console base on 8250 serial */
	if (jzsoc_register_8250serial(2))
		pr_err("register serial console failed!\n");
	return ret;
}
arch_initcall(jzsoc_board_init);
