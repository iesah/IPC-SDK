/*
   * linux/drivers/watchdog/jz_wdt.c - Ingenic Watchdog Controller driver
   *
   * Copyright (C) 2015 Ingenic Semiconductor Co., Ltd.
   * Written by Mick <dongyue.ye@ingenic.com>.
   *
   * This program is free software; you can redistribute it and/or modify
   * it under the terms of the GNU General Public License version 2 as
   * published by the Free Software Foundation.
   */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <soc/base.h>
#include <soc/tcu.h>

#define WDT_FULL_TDR 0x0
#define WDT_ENABLE   0x4
#define WDT_COUNT    0x8
#define WDT_CONTROL  0xC

 //1<<2n
#define CLK_DIV_1	 0
#define CLK_DIV_4	 1
#define CLK_DIV_16	 2
#define CLK_DIV_64	 3
#define CLK_DIV_256	 4
#define CLK_DIV_1024 5
#define CLK_DIV_n CLK_DIV_256


#define DEFAULT_HEARTBEAT 5
#define MAX_HEARTBEAT     357

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout,
		 "Watchdog cannot be stopped once started (default="
		 __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

static unsigned int heartbeat = DEFAULT_HEARTBEAT;
module_param(heartbeat, uint, 0);
MODULE_PARM_DESC(heartbeat,
		"Watchdog heartbeat period in seconds from 1 to "
		__MODULE_STRING(MAX_HEARTBEAT) ", default "
		__MODULE_STRING(DEFAULT_HEARTBEAT));

struct jz_wdt_drvdata {
	struct watchdog_device wdt;
	void __iomem *base;
	struct clk *rtc_clk;
	unsigned int wdt_control;
	unsigned int clk_rate;
};

static int jz_wdt_ping(struct watchdog_device *wdt_dev)
{
	struct jz_wdt_drvdata *drvdata = watchdog_get_drvdata(wdt_dev);
	writel(drvdata->wdt_control | (1 << 10), drvdata->base + WDT_CONTROL);
	return 0;
}

static int jz_wdt_set_timeout(struct watchdog_device *wdt_dev,unsigned int new_timeout)
{
	struct jz_wdt_drvdata *drvdata = watchdog_get_drvdata(wdt_dev);
	unsigned int tdr;
	tdr = drvdata->clk_rate * new_timeout;
	writel(tdr & 0xffff, drvdata->base + WDT_FULL_TDR);
	wdt_dev->timeout = new_timeout;
	return 0;
}

static int jz_wdt_start(struct watchdog_device *wdt_dev)
{
	struct jz_wdt_drvdata *drvdata = watchdog_get_drvdata(wdt_dev);
	outl(1 << 16,TCU_IOBASE + TCU_TSCR);
	jz_wdt_set_timeout(wdt_dev, wdt_dev->timeout);
	writel(drvdata->wdt_control | (1 << 10), drvdata->base + WDT_CONTROL);
	writel(1, drvdata->base + WDT_ENABLE);
	return 0;
}

static int jz_wdt_stop(struct watchdog_device *wdt_dev)
{
	struct jz_wdt_drvdata *drvdata = watchdog_get_drvdata(wdt_dev);

	writel(0, drvdata->base + WDT_ENABLE);
	outl(1 << 16, TCU_IOBASE + TCU_TSSR);

	return 0;
}

static const struct watchdog_info jz_wdt_info = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
	.identity = "jz Watchdog",
};

static const struct watchdog_ops jz_wdt_ops = {
	.owner = THIS_MODULE,
	.start = jz_wdt_start,
	.stop = jz_wdt_stop,
	.ping = jz_wdt_ping,
	.set_timeout = jz_wdt_set_timeout,
};

static int jz_wdt_probe(struct platform_device *pdev)
{
	struct jz_wdt_drvdata *drvdata;
	struct watchdog_device *jz_wdt;
	struct resource	*res;
	int ret;

	drvdata = devm_kzalloc(&pdev->dev, sizeof(struct jz_wdt_drvdata), GFP_KERNEL);
	if (!drvdata){
		dev_err(&pdev->dev, "Unable to alloacate watchdog device\n");
		return -ENOMEM;
	}

	if (heartbeat < 1 || heartbeat > MAX_HEARTBEAT)
		heartbeat = DEFAULT_HEARTBEAT;

	jz_wdt = &drvdata->wdt;
	jz_wdt->info = &jz_wdt_info;
	jz_wdt->ops = &jz_wdt_ops;
	jz_wdt->timeout = heartbeat;
	jz_wdt->min_timeout = 1;
	jz_wdt->max_timeout = MAX_HEARTBEAT;
	watchdog_set_nowayout(jz_wdt, nowayout);
	watchdog_set_drvdata(jz_wdt, drvdata);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	drvdata->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(drvdata->base)) {
		ret = PTR_ERR(drvdata->base);
		goto err_out;
	}

	drvdata->rtc_clk = clk_get(NULL, "rtc");
	if (IS_ERR(drvdata->rtc_clk)) {
		dev_err(&pdev->dev, "cannot find RTC clock\n");
		ret = PTR_ERR(drvdata->rtc_clk);
		goto err_out;
	}
	drvdata->clk_rate = 24000000 / 512 / (1 << CLK_DIV_n * 2);
	//printk("clk_rate=%u\n", drvdata->clk_rate);

	ret = watchdog_register_device(&drvdata->wdt);
	if (ret < 0)
		goto err_disable_clk;

	platform_set_drvdata(pdev, drvdata);

	drvdata->wdt_control = (CLK_DIV_n << 3) | (1 << 1);

	return 0;

err_disable_clk:
	clk_put(drvdata->rtc_clk);
err_out:
	return ret;
}

static int jz_wdt_remove(struct platform_device *pdev)
{
	struct jz_wdt_drvdata *drvdata = platform_get_drvdata(pdev);

	jz_wdt_stop(&drvdata->wdt);
	watchdog_unregister_device(&drvdata->wdt);
	clk_put(drvdata->rtc_clk);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int jz_wdt_suspend(struct device *dev)
{
	struct jz_wdt_drvdata *drvdata = dev_get_drvdata(dev);
	jz_wdt_stop(&drvdata->wdt);
}

static int jz_wdt_resume(struct device *dev)
{
	// struct jz_wdt_drvdata *drvdata = dev_get_drvdata(dev);
}
static SIMPLE_DEV_PM_OPS(jz_wdt_pm_ops, jz_wdt_suspend, jz_wdt_resume);
#endif

static struct platform_driver jz_wdt_driver = {
	.probe = jz_wdt_probe,
	.remove = jz_wdt_remove,
	.driver = {
		.name = "jz-wdt",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM_SLEEP
		.pm = &jz_wdt_pm_ops,
#endif
	},
};

module_platform_driver(jz_wdt_driver);

MODULE_DESCRIPTION("jz Watchdog Driver");
MODULE_LICENSE("GPL");
