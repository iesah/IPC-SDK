#ifndef __TSC_H__
#define __TSC_H__
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

struct jztsc_pin {
	unsigned short			num;
#define LOW_ENABLE			0
#define HIGH_ENABLE			1
	unsigned short 			enable_level;
};

__attribute__((weak)) struct jztsc_platform_data {
	struct jztsc_pin	*gpio;
	unsigned int		x_max;
	unsigned int		y_max;

	char *power_name;
        unsigned long irqflags;
        unsigned int irq;
        unsigned int reset;
	char *vcc_name;
        char *vccio_name;
	int wakeup;

	void		*private;
};

static inline int get_pin_status(struct jztsc_pin *pin)
{
	int val;

	if ((short)pin->num < 0)
		return -1;
	val = gpio_get_value(pin->num);

	if (pin->enable_level == LOW_ENABLE)
		return !val;
	return val;
}

static inline void set_pin_status(struct jztsc_pin *pin, int enable)
{
	if ((short)pin->num < 0)
		return;

	if (pin->enable_level == LOW_ENABLE)
		enable = !enable;
	gpio_set_value(pin->num, enable);
}

static inline void tsc_swap_xy(u16 * x,u16 * y)
{
	u16 tmp = 0;
	tmp = *x;
	*x =  *y;
	*y = tmp;
}

static inline void tsc_swap_x(u16 * x,u16  max_x)
{
	*x =  max_x - *x;
}

static inline void tsc_swap_y(u16 * y,u16 max_y)
{
	*y = max_y - *y;
}
#endif
