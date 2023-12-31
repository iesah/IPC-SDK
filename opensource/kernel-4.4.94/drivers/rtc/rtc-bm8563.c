//驱动代码
/*
* An rtc/i2c driver for the BM8563
*
* Based on rtc-bm8563.c
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>

/* Registers */
#define REG_CTRL_1      0x00
#define REG_CTRL_2      0x01
#define REG_WATCH_SEC   0x02
#define REG_WATCH_MIN   0x03
#define REG_WATCH_HOUR  0x04
#define REG_WATCH_DATE  0x05
#define REG_WATCH_DAY   0x06
#define REG_WATCH_MON   0x07
#define REG_WATCH_YEAR  0x08
#define RTC_I2C_NUM     1
#define RTC_ADDR        0x51

#define I2C_WRITE 0
#define I2C_READ  1

static struct i2c_board_info bm8563_info = {
    I2C_BOARD_INFO("bm8563", RTC_ADDR),
};

static struct i2c_client *bm8563_client;

static int bm8563_get_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);

    unsigned char addr = REG_WATCH_SEC;
    unsigned char buf[7] = {0};

    struct i2c_msg msgs[2] = {
        [0] = {
                .addr  = client->addr,
                .flags = I2C_WRITE,
                .len   = 1,
                .buf   = &addr,
        },  /* setup read addr */
        [1] = {
                .addr  = client->addr,
                .flags = I2C_READ,
                .len = 7,
                .buf = buf,
        },   /* read time/date */
    };

    /* read time/date registers */
    if ((i2c_transfer(client->adapter, &msgs[0], 2)) != 2) {
        dev_err(&client->dev, "%s: read error\n", __func__);
        return -EIO;
    }

    //printk("kernel get time\n");
    tm->tm_sec  = bcd2bin(buf[0]&0x7f) ;
    tm->tm_min  = bcd2bin(buf[1]);
    tm->tm_hour = bcd2bin(buf[2]);
    tm->tm_mday = bcd2bin(buf[3]);
    tm->tm_wday = bcd2bin(buf[4]);
    tm->tm_mon  = bcd2bin(buf[5])-1;
    tm->tm_year = bcd2bin(buf[6]) + 100;
    if((tm->tm_sec == 0) && (tm->tm_min == 0) && (tm->tm_hour == 0) )
    rtc_time_to_tm(0, tm);
    return 0;
}

static int bm8563_set_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    unsigned char buf[8] = {0};
    unsigned char data[2] = {0};

    struct i2c_msg msg[2] = {
        [0] = {
                .addr  = client->addr,
                .flags = I2C_WRITE,
                .len   = 8,
                .buf   =  buf,
        },  /* setup rtc time */
        [1] = {
                .addr  = client->addr,
                .flags = I2C_READ,
                .len = 1,
                .buf = data,
        },   /* read time/date */
    };

    //printk("kernel set time\n");
    buf[0] = REG_WATCH_SEC;
    buf[1] = bin2bcd(tm->tm_sec);
    buf[2] = bin2bcd(tm->tm_min);
    buf[3] = bin2bcd(tm->tm_hour);
    buf[4] = bin2bcd(tm->tm_mday);
    buf[5] = bin2bcd(tm->tm_wday);
    buf[6] = bin2bcd(tm->tm_mon)+1;
    buf[7] = bin2bcd(tm->tm_year % 100);

    /* write time/date registers */
    if ((i2c_transfer(client->adapter, msg, 2)) != 1) {
        dev_err(&client->dev, "%s: write error\n", __func__);
        return -EIO;
    }

    return 0;
}

static const struct rtc_class_ops bm8563_rtc_ops = {
    .read_time = bm8563_get_time,
    .set_time = bm8563_set_time,
};

static int bm8563_dev_init(void)
{
    struct rtc_device *rtc;
    struct i2c_adapter *i2c_adap;
    struct i2c_client *client;
    unsigned char buf[2];
    struct i2c_msg msg = {
        RTC_ADDR, I2C_WRITE, 2, buf,    /* write time/date */
    };

    i2c_adap = i2c_get_adapter(RTC_I2C_NUM);
    bm8563_client = i2c_new_device(i2c_adap, &bm8563_info);
    i2c_put_adapter(i2c_adap);
    client = bm8563_client;


    buf[0] = REG_CTRL_1;
    buf[1] = 0;
    if ((i2c_transfer(client->adapter, &msg, 1)) != 1) {
        dev_err(&client->dev, "%s: write error\n", __func__);
        return -EIO;
    }
    rtc = rtc_device_register("bm8563", &client->dev,
                              &bm8563_rtc_ops, THIS_MODULE);
    if (IS_ERR(rtc))
    return PTR_ERR(rtc);

    i2c_set_clientdata(client, rtc);

    return 0;
}
static void bm8563_dev_exit(void)
{
    struct rtc_device *rtc = i2c_get_clientdata(bm8563_client);

    if (rtc)
    rtc_device_unregister(rtc);
    i2c_unregister_device(bm8563_client);
}
module_init(bm8563_dev_init);
module_exit(bm8563_dev_exit);

MODULE_AUTHOR("damon.jszhang@ingenic.com");
MODULE_DESCRIPTION("rtc driver");
MODULE_LICENSE("GPL");
