/*
 * Real Time Clock interface for Jz.
 *
 * Copyright (C) 2014, Ingenic Semiconductor Inc.
 *
 * Author: Aaron Wang<hfwang@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/bitops.h>
#include <linux/pm.h>

#include "rtc-jz.h"

/* Default time for the first-time power on */
static struct rtc_time default_tm = {
	.tm_year = (2014 - 1900), // year 2014
	.tm_mon = (8 - 1),        // month 8
	.tm_mday = 21,            // day 21
	.tm_hour = 12,
	.tm_min = 0,
	.tm_sec = 0
};

static inline int rtc_periodic_alarm(struct rtc_time *tm)
{
	return  (tm->tm_year == -1) ||
		((unsigned)tm->tm_mon >= 12) ||
		((unsigned)(tm->tm_mday - 1) >= 31) ||
		((unsigned)tm->tm_hour > 23) ||
		((unsigned)tm->tm_min > 59) ||
		((unsigned)tm->tm_sec > 59);
}

static unsigned int jzrtc_readl(struct jz_rtc *dev,int offset)
{
	unsigned int data, timeout = 0x100000;
	do {
		data = readl(dev->iomem + offset);
	} while (readl(dev->iomem + offset) != data && timeout--);
	if (timeout <= 0)
		pr_info("RTC : rtc_read_reg timeout!\n");
	return data;
}

static inline void wait_write_ready(struct jz_rtc *dev)
{
	int timeout = 0x100000;
	while (!(jzrtc_readl(dev,RTC_RTCCR) & RTCCR_WRDY) && timeout--);
	if (timeout <= 0)
		pr_info("RTC : %s timeout!\n",__func__);
}

static void jzrtc_writel(struct jz_rtc *dev,int offset, unsigned int value)
{

	int timeout = 0x100000;
//	wait_write_ready(dev);
	writel(WENR_WENPAT_WRITABLE, dev->iomem + RTC_WENR);
	wait_write_ready(dev);
	while (!(jzrtc_readl(dev,RTC_WENR) & WENR_WEN) && timeout--);
	if (timeout <= 0)
		pr_info("RTC :  wait_writable timeout!\n");
	wait_write_ready(dev);
	writel(value,dev->iomem + offset);
	wait_write_ready(dev);
}

static inline void jzrtc_clrl(struct jz_rtc *dev,int offset, unsigned int value)
{
	jzrtc_writel(dev, offset, jzrtc_readl(dev,offset) & ~(value));
}

static inline void jzrtc_setl(struct jz_rtc *dev,int offset, unsigned int value)
{
	jzrtc_writel(dev,offset,jzrtc_readl(dev,offset) | (value));
}

#define IS_RTC_IRQ(x,y)  (((x) & (y)) == (y))

#ifdef RTC_DEBUG_DUMP
static void jz_rtc_dump(struct jz_rtc *dev)
{

	pr_info ("*******************************************************************\n");
	pr_info ("******************************jz_rtc_dump**********************\n\n");
	pr_info ("jz_rtc_dump-----RTC_RTCCR is --0X%X--\n",jzrtc_readl(dev, RTC_RTCCR));
	pr_info ("jz_rtc_dump-----RTC_RTCSR is --0X%X--\n",jzrtc_readl(dev, RTC_RTCSR));
	pr_info ("jz_rtc_dump-----RTC_RTCSAR is --0X%X--\n",jzrtc_readl(dev,RTC_RTCSAR));
	pr_info ("jz_rtc_dump-----RTC_RTCGR is --0X%X--\n",jzrtc_readl(dev, RTC_RTCGR));
	pr_info ("jz_rtc_dump-----RTC_HCR is --0X%X--\n",jzrtc_readl(dev, RTC_HCR));
	pr_info ("jz_rtc_dump-----RTC_HWFCR is --0X%X--\n",jzrtc_readl(dev, RTC_HWFCR));
	pr_info ("jz_rtc_dump-----RTC_HRCR is --0X%X--\n",jzrtc_readl(dev, RTC_HRCR));
	pr_info ("jz_rtc_dump-----RTC_HWCR is --0X%X--\n",jzrtc_readl(dev, RTC_HWCR));
	pr_info ("jz_rtc_dump-----RTC_HWRSR is --0X%X--\n",jzrtc_readl(dev,RTC_HWRSR));
	pr_info ("jz_rtc_dump-----RTC_HSPR is --0X%X--\n",jzrtc_readl(dev, RTC_HSPR));
	pr_info ("jz_rtc_dump-----RTC_WENR is --0X%X--\n",jzrtc_readl(dev, RTC_WENR));
	pr_info ("jz_rtc_dump-----RTC_CKPCR is --0X%X--\n",jzrtc_readl(dev,RTC_CKPCR));
	pr_info ("jz_rtc_dump-----RTC_PWRONCR is -0X%X-\n",jzrtc_readl(dev,RTC_PWRONCR));
	pr_info ("***************************jz_rtc_dump***************************\n");
	pr_info ("*******************************************************************\n\n");

	return;
}
#endif

static void jzrtc_irq_tasklet(unsigned long data)
{

	unsigned int rtsr,save_rtsr;
	unsigned long events;
	struct jz_rtc *rtc =  (struct jz_rtc *) data;

	rtsr = jzrtc_readl(rtc, RTC_RTCCR);
	save_rtsr = rtsr;
	//is rtc interrupt
	events = 0;
	if (IS_RTC_IRQ(rtsr,RTCCR_AF)) {

		events = RTC_AF | RTC_IRQF;
		rtsr &= ~RTCCR_AF;

		if (rtc_periodic_alarm(&rtc->rtc_alarm) == 0)
			rtsr &= ~(RTCCR_AIE | RTCCR_AE);
	}

	if (IS_RTC_IRQ(rtsr,RTCCR_1HZ)) {
		rtsr &= ~(RTCCR_1HZ);
		events = RTC_UF | RTC_IRQF;
	}

	if(events != 0)
		rtc_update_irq(rtc->rtc, 1, events);

	if(rtsr != save_rtsr)
		jzrtc_writel(rtc, RTC_RTCCR,rtsr);

	enable_irq(rtc->irq);

	return;
}

static irqreturn_t jz_rtc_interrupt(int irq, void *dev_id)
{
	struct jz_rtc *rtc = (struct jz_rtc *) (dev_id);

	disable_irq_nosync(rtc->irq);

	tasklet_schedule(&rtc->tasklet);
	return IRQ_HANDLED;
}

static int jz_rtc_open(struct device *dev)
{
//	int ret;
//	struct jz_rtc *rtc = dev_get_drvdata(dev);

	return 0;
}

static void jz_rtc_release(struct device *dev)
{
     /*	struct jz_rtc *rtc = dev_get_drvdata(dev); */

     /*	free_irq(rtc->irq, rtc); */
}

static int jz_rtc_ioctl(struct device *dev, unsigned int cmd,
		unsigned long arg)
{
	struct jz_rtc *rtc = dev_get_drvdata(dev);
	unsigned int tmp;
	unsigned long flags;
	switch (cmd) {
		case RTC_AIE_OFF:
			spin_lock_irqsave(&rtc->lock,flags);
			jzrtc_clrl(rtc,RTC_RTCCR, RTCCR_AIE | RTCCR_AE | RTCCR_AF);
			spin_unlock_irqrestore(&rtc->lock,flags);
			return 0;
		case RTC_AIE_ON:
			spin_lock_irqsave(&rtc->lock,flags);
			tmp = jzrtc_readl(rtc, RTC_RTCCR);
			tmp &= ~RTCCR_AF;
			tmp |= RTCCR_AIE | RTCCR_AE;
			jzrtc_writel(rtc, RTC_RTCCR, tmp);
			spin_unlock_irqrestore(&rtc->lock,flags);
			return 0;
		case RTC_UIE_OFF:
			spin_lock_irqsave(&rtc->lock,flags);
			jzrtc_clrl(rtc,RTC_RTCCR, RTCCR_1HZ | RTCCR_1HZIE);
			spin_unlock_irqrestore(&rtc->lock,flags);
			return 0;
		case RTC_UIE_ON:
			spin_lock_irqsave(&rtc->lock,flags);
			tmp = jzrtc_readl(rtc, RTC_RTCCR);
			tmp &= ~RTCCR_1HZ;
			tmp |= RTCCR_1HZIE;
			jzrtc_writel(rtc, RTC_RTCCR, tmp);
			spin_unlock_irqrestore(&rtc->lock,flags);
			return 0;
		case RTC_PIE_OFF:
			pr_debug("not implement!\n");
			return 0;
		case RTC_PIE_ON:
			pr_debug("not implement!\n");
			return 0;
	}
	return -ENOIOCTLCMD;
}

static int jz_rtc_set_time(struct device *dev, struct rtc_time *tm)
{

	struct jz_rtc *rtc = NULL;
	unsigned long time = 0;
	int ret = -1;

	if (dev) {
		rtc = dev_get_drvdata(dev);
	} else {
		return ret;
	}

	ret = rtc_tm_to_time(tm, &time);
	if (ret == 0)
		jzrtc_writel(rtc, RTC_RTCSR, time);
	return ret;
}

static int jz_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	unsigned int tmp;
	struct jz_rtc *rtc = dev_get_drvdata(dev);

	tmp = jzrtc_readl(rtc, RTC_RTCSR);
	rtc_time_to_tm(tmp, tm);

	if (rtc_valid_tm(tm) < 0) {
		/* Set the default time */
		jz_rtc_set_time(dev, &default_tm);
		tmp = jzrtc_readl(rtc, RTC_RTCSR);
		rtc_time_to_tm(tmp, tm);
	}
	return 0;
}

static int jz_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	unsigned int rtc_rcr,tmp;
	struct jz_rtc *rtc = dev_get_drvdata(dev);
	unsigned long flags;

	tmp = jzrtc_readl(rtc, RTC_RTCSAR);
	spin_lock_irqsave(&rtc->lock,flags);
	rtc_time_to_tm(tmp, &rtc->rtc_alarm);
	memcpy(&alrm->time, &rtc->rtc_alarm, sizeof(struct rtc_time));
	rtc_rcr = jzrtc_readl(rtc, RTC_RTCCR);
	alrm->enabled = (rtc_rcr & RTCCR_AIE) ? 1 : 0;
	alrm->pending = (rtc_rcr & RTCCR_AF) ? 1 : 0;
	spin_unlock_irqrestore(&rtc->lock,flags);
	return 0;
}

static int jz_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	int ret = 0;
	unsigned long time;
	unsigned int tmp;
	struct jz_rtc *rtc = dev_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&rtc->lock,flags);
	if (alrm->enabled) {
		rtc_tm_to_time(&alrm->time,&time);
		jzrtc_writel(rtc, RTC_RTCSAR, time);
		tmp = jzrtc_readl(rtc, RTC_RTCCR);
		tmp &= ~RTCCR_AF;
		tmp |= RTCCR_AIE | RTCCR_AE;
		jzrtc_writel(rtc, RTC_RTCCR, tmp);
	} else {
		jzrtc_clrl(rtc,RTC_RTCCR, RTCCR_AIE | RTCCR_AE | RTCCR_AF);
	}
	spin_unlock_irqrestore(&rtc->lock,flags);

	return ret;
}

static int jz_rtc_proc(struct device *dev, struct seq_file *seq)
{
	struct jz_rtc *rtc = dev_get_drvdata(dev);
	seq_printf(seq, "RTC regulator\t: 0x%08x\n",
			jzrtc_readl(rtc, RTC_RTCGR));
	seq_printf(seq, "update_IRQ\t: %s\n",
			(jzrtc_readl(rtc, RTC_RTCCR) & RTCCR_1HZIE) ? "yes" : "no");

	return 0;
}

static int jz_rtc_alarm_irq_enable(struct device *dev, unsigned int enable)
{
	unsigned int tmp;
	struct jz_rtc *rtc = dev_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&rtc->lock,flags);
	if (enable){
		tmp = jzrtc_readl(rtc, RTC_RTCCR);
		tmp &= ~RTCCR_AF;
		tmp |= RTCCR_AIE | RTCCR_AE;
		jzrtc_writel(rtc, RTC_RTCCR, tmp);
	} else {
		jzrtc_clrl(rtc,RTC_RTCCR, RTCCR_AIE | RTCCR_AE | RTCCR_AF);
	}
	spin_unlock_irqrestore(&rtc->lock,flags);

	return 0;
}

static const struct rtc_class_ops jz_rtc_ops = {
	.open       = jz_rtc_open,
	.release    = jz_rtc_release,
	.ioctl      = jz_rtc_ioctl,
	.read_time  = jz_rtc_read_time,
	.set_time   = jz_rtc_set_time,
	.read_alarm = jz_rtc_read_alarm,
	.set_alarm  = jz_rtc_set_alarm,
	.alarm_irq_enable = jz_rtc_alarm_irq_enable,
	.proc       = jz_rtc_proc,
};

static void jz_rtc_enable(struct jz_rtc *rtc)
{

	unsigned int cfc,hspr,rgr_1hz;
	unsigned long time = 0;

	/*
	 * When we are powered on for the first time, init the rtc and reset time.
	 *
	 * For other situations, we remain the rtc status unchanged.
	 */


#ifdef CONFIG_USE_EXTERNAL_24M_CLOCK
    /* set external clock 24000000 */
//	writel(WENR_WENPAT_WRITABLE, rtc->iomem + RTC_WENR);
	writel(0x2, rtc->iomem + RTC_RTCCR);
#endif
//	cpm_set_clock(CGU_RTCCLK, 32768); //later to know if we need to set it ,it may decided by hardware.
	cfc = HSPR_RTCV;
	hspr = jzrtc_readl(rtc, RTC_HSPR);
	rgr_1hz = jzrtc_readl(rtc, RTC_RTCGR) & RTCGR_NC1HZ_MASK;

	if ((hspr != cfc) || (rgr_1hz != RTC_FREQ_DIVIDER)) {
		/* We are powered on for the first time !!! */

#ifdef CONFIG_USE_EXTERNAL_24M_CLOCK
		/* Set 24000000 rtc clocks per seconds */
        jzrtc_writel(rtc, RTC_RTCGR, EXTERNAL_RTC_FREQ_DIVIDER);
#else
		/* Set 32768 rtc clocks per seconds */
		jzrtc_writel(rtc, RTC_RTCGR, RTC_FREQ_DIVIDER);
#endif
		/* Set minimum wakeup_n pin low-level assertion time for wakeup: 100ms */
		jzrtc_writel(rtc, RTC_HWFCR, HWFCR_WAIT_TIME(100));
		jzrtc_writel(rtc, RTC_HRCR, HRCR_WAIT_TIME(60));


		/* Reset to the default time */
		rtc_tm_to_time(&default_tm, &time);
		jzrtc_writel(rtc, RTC_RTCSR, time);

		/* clear alarm register */
		jzrtc_writel(rtc, RTC_RTCSAR, 0);

		/* start rtc */
#ifdef CONFIG_USE_EXTERNAL_24M_CLOCK
        jzrtc_writel(rtc, RTC_RTCCR, RTCCR_RTCE | 0x2);
#else
        jzrtc_writel(rtc, RTC_RTCCR, RTCCR_RTCE );
#endif
		jzrtc_writel(rtc, RTC_HSPR, cfc);
	}

	/* clear all rtc flags */
	jzrtc_writel(rtc, RTC_HWRSR, 0);

	/* enabled Power detect*/
	jzrtc_writel(rtc, RTC_HWCR,((jzrtc_readl(rtc, RTC_HWCR) & 0x7) | (EPDET_DEFAULT << 3)));

	return ;

}


static int jz_rtc_probe(struct platform_device *pdev)
{
	struct jz_rtc *rtc;
	int ret;

	pr_debug("%s: probe=%p\n", __func__, pdev);

	rtc = kzalloc(sizeof(*rtc), GFP_KERNEL);//
	if (!rtc)
		return -ENOMEM;

	rtc->irq = platform_get_irq(pdev, 0);
	if (rtc->irq < 0) {
		dev_err(&pdev->dev, "no irq for rtc tick\n");
		return -ENOENT;
	}
	pr_debug("jz_rtc: tick irq %d\n",  rtc->irq);

	rtc->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (rtc->res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		return -ENOENT;
	}

	rtc->res = request_mem_region(rtc->res->start,
			rtc->res->end - rtc->res->start+1,
			pdev->name);
	if (rtc->res == NULL) {
		dev_err(&pdev->dev, "failed to reserve memory region\n");
		ret = -ENOENT;
		goto err_nores;
	}

	rtc->iomem = ioremap_nocache(rtc->res->start, rtc->res->end - rtc->res->start + 1);
	if (rtc->iomem == NULL) {
		dev_err(&pdev->dev, "failed ioremap()\n");
		ret = -EINVAL;
		goto err_nomap;
	}

	spin_lock_init(&rtc->lock);
	platform_set_drvdata(pdev, rtc);
	device_init_wakeup(&pdev->dev, 1);
	rtc->rtc = rtc_device_register(pdev->name, &pdev->dev, &jz_rtc_ops, THIS_MODULE);

	if (IS_ERR(rtc->rtc)) {
		ret = PTR_ERR(rtc->rtc);
		dev_err(&pdev->dev, "Failed to register rtc device: %d\n", ret);
		goto err_unregister_rtc;
	}

	tasklet_init(&rtc->tasklet, jzrtc_irq_tasklet,(unsigned long)rtc);

	ret = request_irq(rtc->irq, jz_rtc_interrupt, IRQF_TRIGGER_LOW | IRQF_DISABLED,
			"rtc 1Hz and alarm", rtc);
	if (ret) {
		pr_debug("IRQ %d already in use.\n", rtc->irq);
		goto err_unregister_rtc;
	}

	jz_rtc_enable(rtc);

	return 0;


err_unregister_rtc:
	rtc_device_unregister(rtc->rtc);

	iounmap(rtc->iomem);

err_nomap:
	release_resource(rtc->res);

err_nores:
	return ret;


}

static int jz_rtc_remove(struct platform_device *pdev)
{
	struct jz_rtc *rtc = platform_get_drvdata(pdev);

	jzrtc_writel(rtc, RTC_RTCCR, 0);
	if (rtc->rtc)
		rtc_device_unregister(rtc->rtc);

	clk_disable(rtc->clk);
	clk_put(rtc->clk);
	rtc->clk = NULL;

	iounmap(rtc->iomem);
	release_resource(rtc->res);
	kfree(rtc);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

#ifdef CONFIG_PM
static int jz_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct jz_rtc *rtc = platform_get_drvdata(pdev);

#ifdef CONFIG_TEST_RESET_DLL
	jzrtc_writel(rtc, RTC_PWRONCR,
			jzrtc_readl(rtc, RTC_PWRONCR) &~ (1 << 0));

	jzrtc_writel(rtc, RTC_RTCGR,
			jzrtc_readl(rtc, RTC_RTCGR) &~ (1 << 31));

	jzrtc_writel(rtc, RTC_RTCGR,
			jzrtc_readl(rtc, RTC_RTCGR) &~ (0x1f << 11));

	jzrtc_writel(rtc, RTC_RTCCR,
			jzrtc_readl(rtc, RTC_RTCCR) | RTCCR_1HZIE);
#endif

	if (device_may_wakeup(&pdev->dev)) {
		enable_irq_wake(rtc->irq);
	}

	return 0;
}

static int jz_rtc_resume(struct platform_device *pdev)
{
	struct jz_rtc *rtc = platform_get_drvdata(pdev);

	if (device_may_wakeup(&pdev->dev)) {
		disable_irq_wake(rtc->irq);
	}

	return 0;
}

#else
#define jz_rtc_suspend	NULL
#define jz_rtc_resume	NULL
#endif

static struct platform_driver jz_rtc_driver = {
	.probe		= jz_rtc_probe,
	.remove		= jz_rtc_remove,
	.suspend	= jz_rtc_suspend,
	.resume		= jz_rtc_resume,
	.driver		= {
	.name		= "jz-rtc",
	},
};

static int __init jz_rtc_init(void)
{
	return platform_driver_register(&jz_rtc_driver);
}

static void __exit jz_rtc_exit(void)
{
	platform_driver_unregister(&jz_rtc_driver);
}

module_init(jz_rtc_init);
module_exit(jz_rtc_exit);

MODULE_AUTHOR("Aaron <hfwang@ingenic.cn>");
MODULE_DESCRIPTION("jz Realtime Clock Driver (RTC)");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:jz-rtc");
