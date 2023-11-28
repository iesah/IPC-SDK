#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>





static int inno_mipi_write(struct i2c_client *client, unsigned char reg, unsigned char value)
{
        int ret;
        unsigned char buf[2] = {reg, value};
        struct i2c_msg msg = {
                .addr   = client->addr,
                .flags  = 0,
                .len    = 2,
                .buf    = buf,
        };

        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret > 0)
                ret = 0;

        return ret;
}

static int inno_mipi_read(struct i2c_client *client, unsigned char reg, unsigned char *value)
{
        int ret;
        struct i2c_msg msg[2] = {
                [0] = {
                        .addr   = client->addr,
                        .flags  = 0,
                        .len    = 1,
                        .buf    = &reg,
                },
                [1] = {
                        .addr   = client->addr,
                        .flags  = I2C_M_RD,
                        .len    = 1,
                        .buf    = value,
                }
        };

        ret = i2c_transfer(client->adapter, msg, 2);
        if (ret > 0)
                ret = 0;

        return ret;
}





static int mipi_regs_dump(struct i2c_client *client)
{
	int i;
	for(i = 0; i < 0xff; i++) {
		unsigned char v = 0;
		inno_mipi_read(client, i, &v);
		printk("0x%x : 0x%x\n", i, v);
	}

}


static int __init inno_rx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("=========%s,%d========\n", __func__, __LINE__);
	unsigned char v = 0;
	int ret = 0;
	ret = inno_mipi_read(client, 0, &v);
	if(ret < 0) {
		printk("inno mipi read error!\n");
	}


	mipi_regs_dump(client);
	printk("--------v %x\n", v);

	return 0;

}

static int __exit inno_rx_remove(struct i2c_client *client)
{
        return 0;
}


static const struct i2c_device_id inno_rx_id[] = {
        { "mipi-tx", 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, inno_rx_id);
static struct i2c_driver inno_rx_driver = {
        .driver         = {
                .name   = "mipi-tx",
                .owner  = THIS_MODULE,
        },
        .id_table       = inno_rx_id,
        .probe          = inno_rx_probe,
        .remove         = inno_rx_remove,
};

static int __init inno_rx_init(void)
{
	printk("=======================%s, %d\n", __func__, __LINE__);
        return i2c_add_driver(&inno_rx_driver);
}

static void __exit inno_rx_exit(void)
{
        i2c_del_driver(&inno_rx_driver);
}

module_init(inno_rx_init);
module_exit(inno_rx_exit);

