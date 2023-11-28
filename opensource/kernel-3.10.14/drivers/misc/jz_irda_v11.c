#include <linux/completion.h>
#include <linux/crc-ccitt.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/serial_core.h>
#include <linux/clk.h>

struct irda_uart_port{
	struct uart_port    port;
	struct clk      *clk;
	char            name[16];
};

struct irda_uart_port *irda;

#define DEVICE_NAME	"jz-irda"

#define NORMAL_SEND	         0
#define REPEAT_SEND	         1

#define GPIO_JZ_IRDA	  GPIO_PF(4)

#define UART_UDLLR	0 //lab = 1
#define UART_UDLHR	1 //lab = 0
#define UART_FCR	2
#define UART_LCR    3
#define UART_ISR    8

#define UART_FCR_ENABLE_FIFO    0x01
#define UART_FCR_UME	4

#define UART_LCR_WLEN8      0 /* Wordlength: 8 bits */
#define UART_LCR_STOP       2 /* Stop bits: 0=1 bit, 1=2 bits */

#define senddata(x) \
	do {writel(x,irda->port.membase); \
		while(!((readl(irda->port.membase + 0x14)) & 0x60)); \
	}while(0)
static int jz_irda_send(unsigned short code);

static inline unsigned int serial_in(struct irda_uart_port *up, int offset)
{
	offset <<= 2;
	return readl(up->port.membase + offset);
}

static inline void serial_out(struct irda_uart_port *up, int offset, int value)
{
	offset <<= 2;
	writel(value, up->port.membase + offset);
}


static int jz_irda_open(struct inode *inode, struct file *file)
{
	printk("JZ_LED Driver Open Called!\n");
	return 0;
}

static int jz_irda_release(struct inode *inode, struct file *file)
{
	printk("JZ_LED Driver Release Called!\n");
	return 0;
}

static long jz_irda_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned short code = *(unsigned short *)arg;

	if(code > 0xffff){
		printk("ERROR: code error,addr + num should less than 0xffff,please check it\n");
		return -ENOENT;
	}
	switch(cmd){
		case NORMAL_SEND:
			jz_irda_send(code);
			break;
		case REPEAT_SEND:
			printk("Don't support repeat mode \n");
			break;
		default:
			break;
	}
}

static int send_head_code(void)
{
	int i;
	for(i= 0 ; i < 67; i++)
		senddata(0x55);
	mdelay(4);
	udelay(570);

	return 0;
}

static void send_code_1(void)
{
	int i;
	for(i = 0; i < 4; i++)
		senddata(0x55);
	udelay(1800);
}

static void send_code_0(void)
{
	int i;
	for(i = 0; i < 4; i++)
		senddata(0x55);
	udelay(680);
}

static int send_normal_code(unsigned short code)
{
	int i;
	short tmp = ~code;

	for(i = 0; i < 8; i++){
		if((code >> i) & 1)
			send_code_1();
		else
			send_code_0();
	}
	for(i = 0; i < 8; i++){
		if((tmp >> i) & 1)
			send_code_1();
		else
			send_code_0();
	}
	return 0;
}

static int jz_irda_send(unsigned short code)
{

	unsigned short addr_code = (code >> 8) & 0xff;
	unsigned short num_code = code & 0xff;

	send_head_code();
	send_normal_code(addr_code);
	send_normal_code(num_code);

	return 0;
}

static struct file_operations jz_irda_fops =
{
	.owner  =   THIS_MODULE,
	.open   =   jz_irda_open,
	.release =  jz_irda_release,
	.unlocked_ioctl	= jz_irda_ioctl,
};

static int set_uart_port_baud()
{
	int tmp = 0;
	int div = 24000000 / 16 / (38000 * 2) + 1;

	serial_out(irda, UART_FCR, 0);
	tmp |= 1 << 7;
	serial_out(irda,UART_LCR,tmp);

	serial_out(irda,UART_UDLHR,(div >> 8) & 0xff);
	serial_out(irda,UART_UDLLR,div & 0xff);

	serial_out(irda,UART_LCR,((0x3 << UART_LCR_WLEN8) | (0 << UART_LCR_STOP)));
	serial_out(irda,UART_ISR,0);

	serial_out(irda,UART_FCR,(1 << UART_FCR_UME) | (1 << UART_FCR_ENABLE_FIFO));

	return 0;
}

static struct miscdevice miscdev;
static int jz_irda_probe(struct platform_device *pdev)
{
	struct resource *mmres;
	int ret;

	printk("jz IRDA DRIVER MODULE INIT\n");
	mmres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irda = kzalloc(sizeof(struct irda_uart_port), GFP_KERNEL);
	if (!irda)
		return -ENOMEM;

	irda->port.membase = ioremap(mmres->start, mmres->end - mmres->start + 1);
	if (!irda->port.membase) {
		ret = -ENOMEM;
		goto err_map;
	}
	sprintf(irda->name,"uart%d",pdev->id); //uart 2

	irda->clk = clk_get(&pdev->dev, irda->name);
	if (IS_ERR(irda->clk)) {
		ret = PTR_ERR(irda->clk);
		goto err_map;
	}
	clk_enable(irda->clk);

	set_uart_port_baud();// 38k

	miscdev.minor = MISC_DYNAMIC_MINOR;
	miscdev.name = DEVICE_NAME;
	miscdev.fops = &jz_irda_fops ;
	ret = misc_register(&miscdev);
	printk("misc request ok\n");
	if (ret < 0)
	{
		printk(DEVICE_NAME " can't register major number\n");
		return -1;
	}
	printk("register JZ-irda Driver OK! Major = %d\n", ret);

	return 0;
err_map:
	kfree(irda);
	return ret;
}

static struct platform_driver jz_irda_probe_driver = {
	.driver = {
		.name   = DEVICE_NAME,
		.owner  = THIS_MODULE,
	},
	.probe          = jz_irda_probe,
};

static int __init jz_irda_init(void)
{
	return platform_driver_register(&jz_irda_probe_driver);
}

static void __exit jz_irda_exit(void)
{
	platform_driver_unregister(&jz_irda_probe_driver);
}

module_init(jz_irda_init);
module_exit(jz_irda_exit);

MODULE_DESCRIPTION("JZ IRDA Driver");
MODULE_LICENSE("GPL");

