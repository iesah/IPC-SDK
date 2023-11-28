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

#if !defined CONFIG_GPIOLIB || !defined CONFIG_GENERIC_GPIO
#error  "Need GPIOLIB !!!"
#endif

#define GPIO_PORT_OFF	0x100
#define NR_GPIO_PORT	6
#define NR_GPIO_NUM	(NR_GPIO_PORT*32)

#define PXPIN		0x00   /* PIN Level Register */
#define PXINT		0x10   /* Port Interrupt Register */
#define PXINTS		0x14   /* Port Interrupt Set Register */
#define PXINTC		0x18   /* Port Interrupt Clear Register */
#define PXMSK		0x20   /* Port Interrupt Mask Reg */
#define PXMSKS		0x24   /* Port Interrupt Mask Set Reg */
#define PXMSKC		0x28   /* Port Interrupt Mask Clear Reg */
#define PXPAT1		0x30   /* Port Pattern 1 Set Reg. */
#define PXPAT1S		0x34   /* Port Pattern 1 Set Reg. */
#define PXPAT1C		0x38   /* Port Pattern 1 Clear Reg. */
#define PXPAT0		0x40   /* Port Pattern 0 Register */
#define PXPAT0S		0x44   /* Port Pattern 0 Set Register */
#define PXPAT0C		0x48   /* Port Pattern 0 Clear Register */
#define PXFLG		0x50   /* Port Flag Register */
#define PXFLGC		0x58   /* Port Flag clear Register */
#define PXOENS		0x64   /* Port Output Disable Set Register */
#define PXOENC		0x68   /* Port Output Disable Clear Register */
#define PXPEN		0x70   /* Port Pull Disable Register */
#define PXPENS		0x74   /* Port Pull Disable Set Register */
#define PXPENC		0x78   /* Port Pull Disable Clear Register */
#define PXDSS		0x84   /* Port Drive Strength set Register */
#define PXDSC		0x88   /* Port Drive Strength clear Register */

extern int gpio_ss_table[][2];
#ifdef CONFIG_RECONFIG_SLEEP_GPIO
extern int gpio_ss_table2[][2];
extern bool need_update_gpio_ss(void);
int __init gpio_ss_recheck(void);
#endif

extern void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume);

struct jzgpio_state {
	unsigned int output_low;
	unsigned int output_high;
	unsigned int input_pull;
	unsigned int input_nopull;
};

struct jzgpio_chip {
	void __iomem *reg;
	int irq_base;
	DECLARE_BITMAP(dev_map, 32);
	DECLARE_BITMAP(gpio_map, 32);
	DECLARE_BITMAP(irq_map, 32);
	DECLARE_BITMAP(out_map, 32);
	struct gpio_chip gpio_chip;
	struct irq_chip  irq_chip;
	unsigned int wake_map;
	struct jzgpio_state sleep_state;
	struct jzgpio_state sleep_state2;
	unsigned int save[5];
	spinlock_t gpiolock;
	unsigned int *mcu_gpio_reg;
};

static struct jzgpio_chip jz_gpio_chips[];

static inline struct jzgpio_chip *gpio2jz(struct gpio_chip *gpio)
{
	return container_of(gpio, struct jzgpio_chip, gpio_chip);
}

static inline struct jzgpio_chip *irqdata2jz(struct irq_data *data)
{
	return container_of(data->chip, struct jzgpio_chip, irq_chip);
}

static inline int gpio_pin_level(struct jzgpio_chip *jz, int pin)
{
	return !!(readl(jz->reg + PXPIN) & BIT(pin));
}
static void gpio_set_func(struct jzgpio_chip *chip,
		enum gpio_function func, unsigned int pins)
{
	unsigned long flags;
	unsigned long comp;

	spin_lock_irqsave(&chip->gpiolock,flags);

	comp = pins & readl(chip->reg + PXMSK);
	if(comp != pins){
		writel(comp ^ pins, chip->reg + PXMSKS);
	}

	comp = pins & readl(chip->reg + PXINT);
	if((func & 0x8) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXINTS);
	}
	comp = pins & readl(chip->reg + PXPAT1);
	if((func & 0x2) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXPAT1S);
	}
	comp = pins & readl(chip->reg + PXPAT0);
	if((func & 0x1) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXPAT0S);
	}


	comp = pins & (~readl(chip->reg + PXINT));
	if(!(func & 0x8) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXINTC);
	}
	comp = pins & (~readl(chip->reg + PXPAT1));
	if(!(func & 0x2) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXPAT1C);
	}
	comp = pins & (~readl(chip->reg + PXPAT0));
	if(!(func & 0x1) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXPAT0C);
	}

	comp = pins & (~readl(chip->reg + PXPEN));
	if((func & 0x10) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXPENC);
	}
	comp = pins & readl(chip->reg + PXPEN);
	if(!(func & 0x10) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXPENS);
	}
	comp = pins & (~readl(chip->reg + PXMSK));
	if(!(func & 0x4) && (comp != pins)){
		writel(comp ^ pins, chip->reg + PXMSKC);
	}
	comp = pins & (~readl(chip->reg + PXFLG));
	if(comp != pins){
		writel(comp ^ pins, chip->reg + PXFLGC);
	}
	spin_unlock_irqrestore(&chip->gpiolock,flags);
}

int jzgpio_set_func(enum gpio_port port,
		enum gpio_function func,unsigned long pins)
{
	struct jzgpio_chip *jz = &jz_gpio_chips[port];

	if (~jz->dev_map[0] & pins)
		return -EINVAL;

	gpio_set_func(jz,func,pins);
	return 0;
}

int jz_gpio_set_func(int gpio, enum gpio_function func)
{
	int port = gpio / 32;
	int pin = BIT(gpio & 0x1f);

	struct jzgpio_chip *jz = &jz_gpio_chips[port];

/*
 * TODO: ugly stuff
 * should i check and mark the pin has been requested?
 * it's a duplicate of request_gpio
 */
#if 0
	if (jz->dev_map[0] & pins)
		return -EINVAL;

	jz->dev_map[0] |= pins;
#endif

	gpio_set_func(jz, func, pin);
	return 0;
}
EXPORT_SYMBOL(jz_gpio_set_func);

int jzgpio_ctrl_pull(enum gpio_port port, int enable_pull,unsigned long pins)
{	
	struct jzgpio_chip *jz = &jz_gpio_chips[port];

	if (~jz->dev_map[0] & pins)
		return -EINVAL;

	if (enable_pull)
		writel(pins, jz->reg + PXPENC);
	else
		writel(pins, jz->reg + PXPENS);

	return 0;
}

/* Functions followed for GPIOLIB */
static int jz_gpio_set_pull(struct gpio_chip *chip,
		unsigned offset, unsigned pull)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	if (test_bit(offset, jz->gpio_map)) {
		pr_err("BAD pull to input gpio.\n");
		return -EINVAL;
	}

	if (!pull)
		writel(BIT(offset), jz->reg + PXPENS);
	else
		writel(BIT(offset), jz->reg + PXPENC);

	return 0;
}

static void jz_gpio_set(struct gpio_chip *chip,
		unsigned offset, int value)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	if (! test_bit(offset, jz->out_map)) {
		pr_err("BAD set to input gpio.\n");
		return;
	}

	if (value)
		writel(BIT(offset), jz->reg + PXPAT0S);
	else
		writel(BIT(offset), jz->reg + PXPAT0C);
}

static int jz_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	return !!(readl(jz->reg + PXPIN) & BIT(offset));
}

static int jz_gpio_input(struct gpio_chip *chip, unsigned offset)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	/* Can NOT operate gpio which used as irq line */
	if (test_bit(offset, jz->irq_map))
		return -EINVAL;

	clear_bit(offset, jz->out_map);
	gpio_set_func(jz, GPIO_INPUT, BIT(offset));
	return 0;
}

static int jz_gpio_output(struct gpio_chip *chip,
		unsigned offset, int value)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	/* Can NOT operate gpio which used as irq line */
	if (test_bit(offset, jz->irq_map))
		return -EINVAL;

	set_bit(offset, jz->out_map);
	gpio_set_func(jz, value? GPIO_OUTPUT1: GPIO_OUTPUT0
			, BIT(offset));
	return 0;
}

static int jz_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	return jz->irq_base + offset;
}

static int jz_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	if(! test_bit(offset, jz->gpio_map))
		return -EINVAL;

	/* Disable pull up/down as default */
	writel(BIT(offset), jz->reg + PXPENS);

	clear_bit(offset, jz->gpio_map);
	return 0;
}

static void jz_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	struct jzgpio_chip *jz = gpio2jz(chip);

	gpio_set_func(jz, GPIO_INPUT, BIT(offset));

	/* Enable pull up/down when release */
	writel(BIT(offset), jz->reg + PXPENC);

	set_bit(offset, jz->gpio_map);
}

/* Functions followed for GPIO IRQ */

static void gpio_unmask_irq(struct irq_data *data)
{
	struct jzgpio_chip *jz = irqdata2jz(data);
	int pin  = data->irq - jz->irq_base;

	writel(BIT(pin), jz->reg + PXMSKC);
}

static void gpio_mask_irq(struct irq_data *data)
{
	struct jzgpio_chip *jz = irqdata2jz(data);
	int pin  = data->irq - jz->irq_base;

	writel(BIT(pin), jz->reg + PXMSKS);
}

static void gpio_mask_and_ack_irq(struct irq_data *data)
{
	struct jzgpio_chip *jz = irqdata2jz(data);
	int pin  = data->irq - jz->irq_base;

	if (irqd_get_trigger_type(data) != IRQ_TYPE_EDGE_BOTH)
		goto end;

	/* emulate double edge trigger interrupt */
	if (gpio_pin_level(jz, pin))
		writel(BIT(pin), jz->reg + PXPAT0C);
	else
		writel(BIT(pin), jz->reg + PXPAT0S);

end:
	writel(BIT(pin), jz->reg + PXMSKS);
	writel(BIT(pin), jz->reg + PXFLGC);
}

static int gpio_set_type(struct irq_data *data, unsigned int flow_type)
{
	struct jzgpio_chip *jz = irqdata2jz(data);
	int pin  = data->irq - jz->irq_base;
	int func;

	if (flow_type & IRQ_TYPE_PROBE)
		return 0;
	switch (flow_type & IRQD_TRIGGER_MASK) {
		case IRQ_TYPE_LEVEL_HIGH:	func = GPIO_INT_HI;	break;
		case IRQ_TYPE_LEVEL_LOW:	func = GPIO_INT_LO;	break;
		case IRQ_TYPE_EDGE_RISING:	func = GPIO_INT_RE;	break;
		case IRQ_TYPE_EDGE_FALLING:	func = GPIO_INT_FE;	break;
		case IRQ_TYPE_EDGE_BOTH:
						if (gpio_pin_level(jz, pin))
							func = GPIO_INT_LO;
						else
							func = GPIO_INT_HI;
						break;
		default:
						return -EINVAL;
	}

	irqd_set_trigger_type(data, flow_type);
	gpio_set_func(jz, func, BIT(pin));
	if (irqd_irq_disabled(data))
		writel(BIT(pin), jz->reg + PXMSKS);
	return 0;
}

static int gpio_set_wake(struct irq_data *data, unsigned int on)
{
	struct jzgpio_chip *jz = irqdata2jz(data);
	int pin  = data->irq - jz->irq_base;

	/* gpio must set be interrupt */
	if(!test_bit(pin, jz->irq_map))
		return -EINVAL;

	if(on)
		jz->wake_map |= 0x1 << pin;
	else
		jz->wake_map &= ~(0x1 << pin);
	return 0;
}

static unsigned int gpio_startup_irq(struct irq_data *data)
{
	struct jzgpio_chip *jz = irqdata2jz(data);
	int pin  = data->irq - jz->irq_base;

	/* gpio must be requested */
	if(test_bit(pin, jz->gpio_map))
		return -EINVAL;

	set_bit(pin, jz->irq_map);
	writel(BIT(pin), jz->reg + PXINTS);
	writel(BIT(pin), jz->reg + PXFLGC);
	writel(BIT(pin), jz->reg + PXMSKC);
	return 0;
}

static void gpio_shutdown_irq(struct irq_data *data)
{
	struct jzgpio_chip *jz = irqdata2jz(data);
	int pin  = data->irq - jz->irq_base;

	clear_bit(pin, jz->irq_map);
	writel(BIT(pin), jz->reg + PXINTC);
	writel(BIT(pin), jz->reg + PXMSKS);
}

static struct jzgpio_chip jz_gpio_chips[] = {
#define ADD_JZ_CHIP(NAME,NUM)						\
	[NUM] = {							\
		.irq_base = IRQ_GPIO_BASE + NUM*32,			\
		.irq_chip = {						\
			.name 	= NAME,					\
			.irq_startup 	= gpio_startup_irq,		\
			.irq_shutdown 	= gpio_shutdown_irq,		\
			.irq_enable	= gpio_unmask_irq,		\
			.irq_disable	= gpio_mask_and_ack_irq,	\
			.irq_unmask 	= gpio_unmask_irq,		\
			.irq_mask 	= gpio_mask_irq,		\
			.irq_mask_ack	= gpio_mask_and_ack_irq,	\
			.irq_set_type 	= gpio_set_type,		\
			.irq_set_wake	= gpio_set_wake,		\
		},							\
		.gpio_chip = {						\
			.base			= (NUM)*32,		\
			.label			= NAME,			\
			.ngpio			= 32,			\
			.direction_input	= jz_gpio_input,	\
			.direction_output	= jz_gpio_output,	\
			.set			= jz_gpio_set,		\
			.set_pull		= jz_gpio_set_pull,	\
			.get			= jz_gpio_get,		\
			.to_irq			= jz_gpio_to_irq,	\
			.request		= jz_gpio_request,	\
			.free			= jz_gpio_free,		\
		}							\
	}
	ADD_JZ_CHIP("GPIO A",0),
	ADD_JZ_CHIP("GPIO B",1),
	ADD_JZ_CHIP("GPIO C",2),
	ADD_JZ_CHIP("GPIO D",3),
	ADD_JZ_CHIP("GPIO E",4),
	ADD_JZ_CHIP("GPIO F",5),
#undef ADD_JZ_CHIP
};

static __init int jz_gpiolib_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(jz_gpio_chips); i++)
		gpiochip_add(&jz_gpio_chips[i].gpio_chip);

	return 0;
}

static irqreturn_t gpio_handler(int irq, void *data)
{
	struct jzgpio_chip *jz = data;
	unsigned long pend,mask;

	pend = readl(jz->reg + PXFLG);
	mask = readl(jz->reg + PXMSK);

	/*
	 * PXFLG may be 0 because of GPIO's bounce in level triggered mode,
	 * so we ignore it when it occurs.
	 */
	pend = pend & ~mask;
	if (pend)
		generic_handle_irq(ffs(pend) -1 + jz->irq_base);

	return IRQ_HANDLED;
}

static irqreturn_t mcu_gpio_handler(int irq, void *data)
{
	struct jzgpio_chip *jz = data;
	unsigned long pend = *(jz->mcu_gpio_reg);
	int i;

	for(i = 0;i < 32;i++)
	{
		if((pend >> i) & 1)
			generic_handle_irq(i + jz->irq_base);
	}
	return IRQ_HANDLED;
}
int mcu_gpio_register(unsigned int reg) {
	int i;
	int ret = -1;
	for(i = 0;i < ARRAY_SIZE(jz_gpio_chips); i++) {
		jz_gpio_chips[i].mcu_gpio_reg =(unsigned int *)reg + i;

		ret = request_irq(IRQ_MCU_GPIO_PORT(i), mcu_gpio_handler, IRQF_DISABLED,"mcu gpio irq",
				  (void*)&jz_gpio_chips[i]);
		if (ret) {
			pr_err("mcu irq[%d] register error!\n",i);
			break;
		}
	}
	return ret;
}
static int __init setup_gpio_irq(void)
{
	int i,j;
	for (i = 0; i < ARRAY_SIZE(jz_gpio_chips); i++) {
		spin_lock_init(&jz_gpio_chips[i].gpiolock);
		if (request_irq(IRQ_GPIO_PORT(i), gpio_handler, IRQF_DISABLED,
					jz_gpio_chips[i].irq_chip.name, 
					(void*)&jz_gpio_chips[i]))
			continue;

		enable_irq_wake(IRQ_GPIO_PORT(i));
		irq_set_handler(IRQ_GPIO_PORT(i), handle_simple_irq);
		for (j = 0; j < 32; j++)
			irq_set_chip_and_handler(jz_gpio_chips[i].irq_base + j,
					&jz_gpio_chips[i].irq_chip, 
					handle_level_irq);
	}
	return 0;
}

void gpio_suspend_set(struct jzgpio_chip *jz)
{
#ifdef CONFIG_TRAPS_USE_TCSM_CHECK
	return;
#endif
#ifdef CONFIG_SUSPEND_SUPREME_DEBUG
	return;
#endif

#ifdef CONFIG_RECONFIG_SLEEP_GPIO
	if (need_update_gpio_ss()) {
		gpio_set_func(jz,GPIO_OUTPUT0,
				jz->sleep_state2.output_low & ~jz->wake_map);
		gpio_set_func(jz,GPIO_OUTPUT1,
				jz->sleep_state2.output_high & ~jz->wake_map);
		gpio_set_func(jz,GPIO_INPUT,
				jz->sleep_state2.input_nopull & ~jz->wake_map);
		gpio_set_func(jz,GPIO_INPUT_PULL,
				jz->sleep_state2.input_pull & ~jz->wake_map);
	} else {
#endif
		gpio_set_func(jz,GPIO_OUTPUT0,
				jz->sleep_state.output_low & ~jz->wake_map);
		gpio_set_func(jz,GPIO_OUTPUT1,
				jz->sleep_state.output_high & ~jz->wake_map);
		gpio_set_func(jz,GPIO_INPUT,
				jz->sleep_state.input_nopull & ~jz->wake_map);
		gpio_set_func(jz,GPIO_INPUT_PULL,
				jz->sleep_state.input_pull & ~jz->wake_map);
#ifdef CONFIG_RECONFIG_SLEEP_GPIO
	}
#endif
}

int gpio_suspend(void)
{
	int i,j,irq = 0;
	struct irq_desc *desc;
	struct jzgpio_chip *jz;

	for(i = 0; i < GPIO_NR_PORTS; i++) {
		jz = &jz_gpio_chips[i];

		for(j=0;j<32;j++) {
			if (jz->wake_map & (0x1<<j)) {
				irq = jz->irq_base + j;
				desc = irq_to_desc(irq);
				__enable_irq(desc, irq, true);
			}
        	}

		jz->save[0] = readl(jz->reg + PXINT);
		jz->save[1] = readl(jz->reg + PXMSK);
		jz->save[2] = readl(jz->reg + PXPAT1);
		jz->save[3] = readl(jz->reg + PXPAT0);
		jz->save[4] = readl(jz->reg + PXPEN);
	
        gpio_suspend_set(jz);
	}
	return 0;
}

void gpio_resume(void)
{
	int i;
	struct jzgpio_chip *jz;

	for(i = 0; i < GPIO_NR_PORTS; i++) {
		jz = &jz_gpio_chips[i];
		writel(jz->save[0], jz->reg + PXINTS);
		writel(~jz->save[0], jz->reg + PXINTC);
		writel(jz->save[1], jz->reg + PXMSKS);
		writel(~jz->save[1], jz->reg + PXMSKC);
		writel(jz->save[2], jz->reg + PXPAT1S);
		writel(~jz->save[2], jz->reg + PXPAT1C);
		writel(jz->save[3], jz->reg + PXPAT0S);
		writel(~jz->save[3], jz->reg + PXPAT0C);
		writel(jz->save[4], jz->reg + PXPENS);
		writel(~jz->save[4], jz->reg + PXPENC);
	}
}

struct syscore_ops gpio_pm_ops = {
	.suspend = gpio_suspend,
	.resume = gpio_resume,
};

int __init gpio_ss_check(void)
{
	unsigned int i,state,group,index;
	unsigned int panic_flags[6] = {0,};

	for (i = 0; i < GPIO_NR_PORTS; i++) {
		jz_gpio_chips[i].sleep_state.input_pull = 0xffffffff;
	}

	for(i = 0; gpio_ss_table[i][1] != GSS_TABLET_END;i++) {
		group = gpio_ss_table[i][0] / 32;
		index = gpio_ss_table[i][0] % 32;
		state = gpio_ss_table[i][1];

		jz_gpio_chips[group].sleep_state.input_pull =
			jz_gpio_chips[group].sleep_state.input_pull & ~(1 << index);

		if(panic_flags[group] & (1 << index)) {
			printk("\nwarning : (%d line) same gpio already set before this line!\n",i);
			printk("\nwarning : (%d line) same gpio already set before this line!\n",i);
			printk("\nwarning : (%d line) same gpio already set before this line!\n",i);
			printk("\nwarning : (%d line) same gpio already set before this line!\n",i);
			printk("\nwarning : (%d line) same gpio already set before this line!\n",i);
			printk("\nwarning : (%d line) same gpio already set before this line!\n",i);
			printk("\nwarning : (%d line) same gpio already set before this line!\n",i);
			panic("gpio_ss_table has iterant gpio set , system halt\n");
			while(1);
		} else {
			panic_flags[group] |= 1 << index;
		}

		switch(state) {
			case GSS_OUTPUT_HIGH:
				jz_gpio_chips[group].sleep_state.output_high |= 1 << index;
				break;
			case GSS_OUTPUT_LOW:
				jz_gpio_chips[group].sleep_state.output_low |= 1 << index;
				break;
			case GSS_INPUT_PULL:
				jz_gpio_chips[group].sleep_state.input_pull |= 1 << index;
				break;
			case GSS_INPUT_NOPULL:
				jz_gpio_chips[group].sleep_state.input_nopull |= 1 << index;
				break;
		}
	}

	pr_info("GPIO sleep states:\n");
	for(i = 0; i < GPIO_NR_PORTS; i++) {
		pr_info("OH:%08x OL:%08x IP:%08x IN:%08x\n", 
				jz_gpio_chips[i].sleep_state.output_high,
				jz_gpio_chips[i].sleep_state.output_low,
				jz_gpio_chips[i].sleep_state.input_pull,
				jz_gpio_chips[i].sleep_state.input_nopull);
	}

	return 0;
}

#ifdef CONFIG_RECONFIG_SLEEP_GPIO
int __init gpio_ss_recheck(void)
{
	unsigned int i,state,group,index;

    for (i = 0; i < GPIO_NR_PORTS; i++) {
        jz_gpio_chips[i].sleep_state2.output_high = jz_gpio_chips[i].sleep_state.output_high;
        jz_gpio_chips[i].sleep_state2.output_low  = jz_gpio_chips[i].sleep_state.output_low;
        jz_gpio_chips[i].sleep_state2.input_pull  = jz_gpio_chips[i].sleep_state.input_pull;
        jz_gpio_chips[i].sleep_state2.input_nopull = jz_gpio_chips[i].sleep_state.input_nopull;
    }

	for(i = 0; gpio_ss_table2[i][1] != GSS_TABLET_END;i++) {
		group = gpio_ss_table2[i][0] / 32;
		index = gpio_ss_table2[i][0] % 32;
		state = gpio_ss_table2[i][1];

        printk("%s: group=%d, index=%d, state=%d\n",__func__, group, index, state);
        jz_gpio_chips[group].sleep_state2.output_high &= ~(1 << index);
        jz_gpio_chips[group].sleep_state2.output_low  &= ~(1 << index);
		jz_gpio_chips[group].sleep_state2.input_pull  &= ~(1 << index);
        jz_gpio_chips[group].sleep_state2.input_nopull &= ~(1 << index);

		switch(state) {
			case GSS_OUTPUT_HIGH:
				jz_gpio_chips[group].sleep_state2.output_high |= 1 << index;
				break;
			case GSS_OUTPUT_LOW:
				jz_gpio_chips[group].sleep_state2.output_low |= 1 << index;
				break;
			case GSS_INPUT_PULL:
				jz_gpio_chips[group].sleep_state2.input_pull |= 1 << index;
				break;
			case GSS_INPUT_NOPULL:
				jz_gpio_chips[group].sleep_state2.input_nopull |= 1 << index;
				break;
            default:

                break;
		}
	}

	return 0;
}
#endif

int __init setup_gpio_pins(void)
{
	int i;
	pr_debug("setup gpio function.\n");
	for (i = 0; i < GPIO_NR_PORTS; i++) {
		jz_gpio_chips[i].reg = ioremap(GPIO_IOBASE + i*GPIO_PORT_OFF,
				GPIO_PORT_OFF - 1);
		jz_gpio_chips[i].gpio_map[0] = 0xffffffff;
	}
	setup_gpio_irq();

	for (i = 0; i < platform_devio_array_size ; i++) {
		struct jz_gpio_func_def *g = &platform_devio_array[i];
		struct jzgpio_chip *jz;

		if (g->port >= GPIO_NR_PORTS) {
			pr_info("Bad gpio define! i:%d\n",i);
			continue;
		} else if (g->port < 0) {
			/* Wrong defines end */
			break;
		}

		jz = &jz_gpio_chips[g->port];
		if (GPIO_AS_FUNC(g->func)) {
			if(jz->dev_map[0] & g->pins) {
				panic("%s:gpio functions has redefinition of '%s'",__FILE__,g->name);
				while(1);
			}
			jz->dev_map[0] |= g->pins;
		}
		gpio_set_func(jz, g->func, g->pins);
	}

	jz_gpiolib_init();
	register_syscore_ops(&gpio_pm_ops);

	gpio_ss_check();
#ifdef CONFIG_RECONFIG_SLEEP_GPIO
    gpio_ss_recheck();
#endif

	return 0;
}

arch_initcall(setup_gpio_pins);

static int gpio_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int len = 0;
	int i;
#define PRINT(ARGS...) len += sprintf (page+len, ##ARGS)
	PRINT("INT\t\tMASK\t\tPAT1\t\tPAT0\n");
	for(i = 0; i < GPIO_NR_PORTS; i++) {
		PRINT("0x%08x\t0x%08x\t0x%08x\t0x%08x\n",
				readl(jz_gpio_chips[i].reg + PXINT),
				readl(jz_gpio_chips[i].reg + PXMSK),
				readl(jz_gpio_chips[i].reg + PXPAT1),
				readl(jz_gpio_chips[i].reg + PXPAT0));
	}

	return len;
}

static int __init init_gpio_proc(void)
{
	struct proc_dir_entry *res;

	res = create_proc_entry("gpios", 0444, NULL);
	if (res) {
		res->read_proc = gpio_read_proc;
		res->write_proc = NULL;
		res->data = NULL;
	}
	return 0;
}

module_init(init_gpio_proc);
