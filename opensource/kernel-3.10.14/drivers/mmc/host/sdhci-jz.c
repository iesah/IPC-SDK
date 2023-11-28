#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

#include <mach/sdhci-jz.h>
#include <mach/jzmmc.h>

#include "sdhci.h"
#include "sdhci-jz-regs.h"


#ifdef CONFIG_FPGA_TEST
static void sdhci_jz_fpga_clk(unsigned int clock)
{
#define CPM_MSC_CLK_R CPM_MSC0_CLK_R
//#define CPM_MSC_CLK_R CPM_MSC1_CLK_R
	unsigned int val;

	if(500000 <= clock){
		val = readl((const volatile void*)CPM_MSC_CLK_R);
		val |= MSC_CLK_H_FREQ;
		writel(val, (void*)CPM_MSC_CLK_R);
	} else {
		val = readl((const volatile void*)CPM_MSC_CLK_R);
		val &= ~MSC_CLK_H_FREQ;
		writel(val, (void*)CPM_MSC_CLK_R);
	}
	printk("\tclk=%d, CPM_MSC0_CLK_R: %08x\n\n", clock, readl((const volatile void*)CPM_MSC0_CLK_R));
}
#endif

static LIST_HEAD(manual_list);
/**
 *	jzmmc_manual_detect - insert or remove card manually
 *	@index: host->index, namely the index of the controller.
 *	@on: 1 means insert card, 0 means remove card.
 *
 *	This functions will be called by manually card-detect driver such as
 *	wifi. To enable this mode you can set value pdata.removal = MANUAL.
 */
int jzmmc_manual_detect(int index, int on)
{
	struct sdhci_jz *jz;
	struct list_head *pos;

	list_for_each(pos, &manual_list) {
		jz = list_entry(pos, struct sdhci_jz, list);
		if (jz->id == index)
			break;
		else
			jz = NULL;
	}

	if (!jz) {
		dev_err(&jz->pdev->dev, "no manual card detect\n");
		return -ENXIO;
	}

	dev_info(&jz->pdev->dev, "card insert manually\n");
	mmc_detect_change(jz->host->mmc, 0);

	return 0;
}
EXPORT_SYMBOL(jzmmc_manual_detect);

/**
 * sdhci_jz_get_max_clk - callback to get maximum clock frequency.
 * @host: The SDHCI host instance.
 *
 * Callback to return the maximum clock rate acheivable by the controller.
*/
static unsigned int sdhci_jz_get_max_clk(struct sdhci_host *host)
{
	struct sdhci_jz *jz = sdhci_priv(host);

	return jz->max_freq[jz->id];
}

/**
 * sdhci_jz_set_clock - callback on clock change
 * @host: The SDHCI host being changed
 * @clock: The clock rate being requested.
 *
 * When the card's clock is going to be changed, look at the new frequency
 * and find the best clock source to go with it.
*/
static void sdhci_jz_set_clock(struct sdhci_host *host, unsigned int clock)
{
#ifndef CONFIG_FPGA_TEST
	struct sdhci_jz *jz = sdhci_priv(host);

	clk_set_rate(jz->clk, clock);
	*(volatile unsigned int*)(0xb0000000 + 0xe4) |= 0x1 << 3;  //FIXME:
	//printk("%s, set clk: %d, get_clk_rate=%ld\n", __func__, clock, clk_get_rate(jz->clk));
#else
	sdhci_jz_fpga_clk(clock);
#endif
}

/**
 * sdhci_jz_get_min_clock - callback to get minimal supported clock value
 * @host: The SDHCI host being queried
 *
 * To init mmc host properly a minimal clock value is needed. For high system
 * bus clock's values the standard formula gives values out of allowed range.
 * The clock still can be set to lower values, if clock source other then
 * system bus is selected.
*/
static unsigned int sdhci_jz_get_min_clock(struct sdhci_host *host)
{
	return 300000;
}


static int sdhci_jz_set_uhs_signaling(struct sdhci_host *host, unsigned int uhs)
{
	struct mmc_ios *ios = &host->mmc->ios;
	unsigned int ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;

	/* Select Bus Speed Mode for host */
	if (ios->timing == MMC_TIMING_MMC_HS200)
		ctrl_2 |= SDHCI_EMMC_CTRL_HS_SDR200;
	else if (ios->timing == MMC_TIMING_UHS_SDR12)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
	else if (ios->timing == MMC_TIMING_UHS_SDR25)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
	else if (ios->timing == MMC_TIMING_UHS_SDR50)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
	else if (ios->timing == MMC_TIMING_UHS_SDR104)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
	else if (ios->timing == MMC_TIMING_UHS_DDR50)
		ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
	sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);

	return 0;
}

static void sdhci_jz_set_pwr(struct sdhci_host *host, int enable) {
	struct sdhci_jz *jz = sdhci_priv(host);
	if(enable) {
		if(gpio_is_valid(jz->pdata->ext_pwr_gpio)) {
			if(jz->pdata->ext_pwr_gpio_invert == LOW_ENABLE)
				enable = !enable;
			gpio_direction_output(jz->pdata->ext_pwr_gpio, enable);
		}
		mdelay(10);
		if(jz->host->mmc->index == 0)
			jzgpio_set_func(GPIO_PORT_B, GPIO_FUNC_0, 0x3f);
	} else {
		if(jz->host->mmc->index == 0)
			jzgpio_set_func(GPIO_PORT_B, GPIO_OUTPUT0, 0x3f);
		if(gpio_is_valid(jz->pdata->ext_pwr_gpio)) {
			if(jz->pdata->ext_pwr_gpio_invert == LOW_ENABLE)
				enable = !enable;
			gpio_direction_output(jz->pdata->ext_pwr_gpio, enable);
		}
	}

}

static struct sdhci_ops sdhci_jz_ops = {
	.get_max_clock		= sdhci_jz_get_max_clk,
	.set_clock			= sdhci_jz_set_clock,
	.get_min_clock		= sdhci_jz_get_min_clock,
	.set_uhs_signaling  = sdhci_jz_set_uhs_signaling,
	.set_pwr			= sdhci_jz_set_pwr,
};

static void sdhci_jz_notify_change(struct platform_device *dev, int state)
{
	struct sdhci_host *host = platform_get_drvdata(dev);
	unsigned long flags;

	if (host) {
		spin_lock_irqsave(&host->lock, flags);
		if (state) {
			dev_dbg(&dev->dev, "card inserted.\n");
			host->flags &= ~SDHCI_DEVICE_DEAD;
			host->quirks |= SDHCI_QUIRK_BROKEN_CARD_DETECTION;
		} else {
			dev_dbg(&dev->dev, "card removed.\n");
			host->flags |= SDHCI_DEVICE_DEAD;
			host->quirks &= ~SDHCI_QUIRK_BROKEN_CARD_DETECTION;
		}
		tasklet_schedule(&host->card_tasklet);
		spin_unlock_irqrestore(&host->lock, flags);
	}
}

static irqreturn_t sdhci_jz_gpio_card_detect_thread(int irq, void *dev_id)
{
	struct sdhci_jz *jz = dev_id;
	int status = gpio_get_value(jz->ext_cd_gpio);
	if (jz->pdata->ext_cd_gpio_invert == LOW_ENABLE)
		status = !status;
	sdhci_jz_notify_change(jz->pdev, status);
	return IRQ_HANDLED;
}

static void sdhci_jz_request_card_pwr_gpio(struct sdhci_jz *jz)
{
	struct jz_sdhci_platdata *pdata = jz->pdata;
	struct device *dev = &jz->pdev->dev;
	if(devm_gpio_request(dev, pdata->ext_pwr_gpio, "SDHCI EXT PWR")) {
		dev_err(dev, "cannot request gpio for pwr control");
		pdata->ext_pwr_gpio = -EBUSY;
	}
};

static void sdhci_jz_setup_card_detect_gpio(struct sdhci_jz *jz)
{
	struct jz_sdhci_platdata *pdata = jz->pdata;
	struct device *dev = &jz->pdev->dev;

    if (devm_gpio_request(dev, pdata->ext_cd_gpio, "SDHCI EXT CD") == 0) {
		jz->ext_cd_gpio = pdata->ext_cd_gpio;
		jz->ext_cd_irq = gpio_to_irq(pdata->ext_cd_gpio);
		if (jz->ext_cd_irq &&
		    request_threaded_irq(jz->ext_cd_irq, NULL,
					 sdhci_jz_gpio_card_detect_thread,
					 IRQF_TRIGGER_RISING |
					 IRQF_TRIGGER_FALLING |
					 IRQF_ONESHOT,
					 dev_name(dev), jz) == 0) {
			int status = gpio_get_value(jz->ext_cd_gpio);
			if (pdata->ext_cd_gpio_invert == LOW_ENABLE)
				status = !status;
			sdhci_jz_notify_change(jz->pdev, status);
		} else {
			dev_warn(dev, "cannot request irq for card detect\n");
			jz->ext_cd_irq = 0;
		}
	} else {
		dev_err(dev, "cannot request gpio for card detect\n");
	}
}

static ssize_t sdhci_jz_regs_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sdhci_host *host = dev_get_drvdata(dev);

	sdhci_dumpregs(host);

	return 0;
}

static struct device_attribute sdhci_jz_sysfs_attrs[] = {
	__ATTR(sdhci_regs, S_IRUGO, sdhci_jz_regs_show, NULL),
};

static int sdhci_jz_probe(struct platform_device *pdev)
{
	struct jz_sdhci_platdata *pdata;
	struct device *dev = &pdev->dev;
	struct sdhci_host *host;
	struct sdhci_jz *jz;
	struct resource *res;
	char clk_gate_name[16];
	char clkname[16];
	int ret, irq, i;

	if (!pdev->dev.platform_data && !pdev->dev.of_node) {
		dev_err(dev, "no device data specified\n");
		return -ENOENT;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "no irq specified\n");
		return irq;
	}

	host = sdhci_alloc_host(dev, sizeof(struct sdhci_jz));
	if (IS_ERR(host)) {
		dev_err(dev, "sdhci_alloc_host() failed\n");
		return PTR_ERR(host);
	}
	jz = sdhci_priv(host);

	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		ret = -ENOMEM;
		goto err_pdata_io_clk;
	}

	memcpy(pdata, pdev->dev.platform_data, sizeof(*pdata));
	jz->ext_cd_gpio = -1; /* invalid gpio number */

	jz->dev = &pdev->dev;
	jz->id = pdev->id;
	jz->host = host;
	jz->pdev = pdev;
	jz->pdata = pdata;

	switch(jz->id) {
	case 0:
#ifdef CONFIG_MMC0_MAX_FREQ
		jz->max_freq[jz->id] =  CONFIG_MMC0_MAX_FREQ;
#endif
		break;
	case 1:
#ifdef CONFIG_MMC1_MAX_FREQ
		jz->max_freq[jz->id] =  CONFIG_MMC1_MAX_FREQ;
#endif
		break;
	}

	platform_set_drvdata(pdev, host);

	sprintf(clk_gate_name, "msc%d", pdev->id);
	jz->clk_gate = clk_get(&pdev->dev, clk_gate_name);
	if (IS_ERR(jz->clk_gate)) {
		return PTR_ERR(jz->clk_gate);
	}

	sprintf(clkname, "cgu_msc%d", pdev->id);
	jz->clk = clk_get(&pdev->dev, clkname);
	if (IS_ERR(jz->clk)) {
		return PTR_ERR(jz->clk);
	}

	/* enable the local io clock and keep it running for the moment. */
	clk_prepare_enable(jz->clk_gate);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	host->ioaddr = devm_ioremap_resource(&pdev->dev, res);

	if (IS_ERR(host->ioaddr)) {
		ret = PTR_ERR(host->ioaddr);
		goto err_req_regs;
	}

	/* sdio for WIFI init*/
	if (pdata->private_init) {
		ret = pdata->private_init();
		if (ret < 0)
			goto err_req_regs;
	}

	/* Ensure we have minimal gpio selected CMD/CLK/Detect */
	if (pdata->cfg_gpio)
		pdata->cfg_gpio(pdev, pdata->max_width);

	host->hw_name = "ingenic-msc";
	host->ops = &sdhci_jz_ops;
	host->quirks = 0;
	host->irq = irq;

	/* Software redefinition caps */
	host->quirks |= SDHCI_QUIRK_MISSING_CAPS;
	host->caps  = CAPABILITIES1_SW;
	host->caps1 = CAPABILITIES2_SW;

	/* max clk */
	host->quirks |= SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN;

	/* not check wp */
	host->quirks |= SDHCI_QUIRK_INVERTED_WRITE_PROTECT;

	/* Setup quirks for the controller */
	host->quirks |= SDHCI_QUIRK_NO_ENDATTR_IN_NOPDESC;
	host->quirks |= SDHCI_QUIRK_NO_HISPD_BIT;

	/* This host supports the Auto CMD12 */
	if(jz->pdata->enable_autocmd12)
		host->quirks |= SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12;

	/* PIO transfer mode */
	if(jz->pdata->pio_mode){
		host->quirks |= SDHCI_QUIRK_BROKEN_DMA;
		host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;
	}
	/* TODO:SoCs need BROKEN_ADMA_ZEROLEN_DESC */
/*	host->quirks |= SDHCI_QUIRK_BROKEN_ADMA_ZEROLEN_DESC;*/

	if (pdata->cd_type == JZ_SDHCI_CD_NONE ||
	    pdata->cd_type == JZ_SDHCI_CD_PERMANENT)
		host->quirks |= SDHCI_QUIRK_BROKEN_CARD_DETECTION;

	if (pdata->cd_type == JZ_SDHCI_CD_PERMANENT)
		host->mmc->caps = MMC_CAP_NONREMOVABLE;

	switch (pdata->max_width) {
	case 8:
		host->mmc->caps |= MMC_CAP_8_BIT_DATA;
	case 4:
		host->mmc->caps |= MMC_CAP_4_BIT_DATA;
		break;
	}

	/* force 1 bit debug test*/
#ifdef CONFIG_FORCE_1BIT_TEST
	host->mmc->caps &= ~MMC_CAP_4_BIT_DATA;
	host->quirks |= SDHCI_QUIRK_FORCE_1_BIT_DATA;
#endif
	if (pdata->pm_caps)
		host->mmc->pm_caps |= pdata->pm_caps;

	if (pdata->pm_flags)
		host->mmc->pm_flags |= pdata->pm_flags;

	host->quirks |= (SDHCI_QUIRK_32BIT_DMA_ADDR |
			 SDHCI_QUIRK_32BIT_DMA_SIZE);

	/* It supports additional host capabilities if needed */
	if (pdata->host_caps)
		host->mmc->caps |= pdata->host_caps;

	if (pdata->host_caps2)
		host->mmc->caps2 |= pdata->host_caps2;

	if (!((host->mmc->caps & MMC_CAP_8_BIT_DATA) ||
		(host->mmc->caps & MMC_CAP_4_BIT_DATA)))
			host->quirks |= SDHCI_QUIRK_FORCE_1_BIT_DATA;

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_enable(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, 50);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_suspend_ignore_children(&pdev->dev, 1);
#endif

	if(gpio_is_valid(pdata->ext_pwr_gpio))
		sdhci_jz_request_card_pwr_gpio(jz);

	list_add(&(jz->list), &manual_list);
	ret = sdhci_add_host(host);
	if (ret) {
		dev_err(dev, "sdhci_add_host() failed\n");
		pm_runtime_forbid(&pdev->dev);
		pm_runtime_get_noresume(&pdev->dev);
		goto err_req_regs;
	}

	if (pdata->cd_type == JZ_SDHCI_CD_GPIO &&
	    gpio_is_valid(pdata->ext_cd_gpio))
		sdhci_jz_setup_card_detect_gpio(jz);

#ifdef CONFIG_PM_RUNTIME
	if (pdata->cd_type != JZ_SDHCI_CD_INTERNAL)
		clk_disable_unprepare(jz->clk_gate);
#endif

	for (i = 0; i < ARRAY_SIZE(sdhci_jz_sysfs_attrs); i++) {
		ret = device_create_file(&pdev->dev, &sdhci_jz_sysfs_attrs[i]);
		if (ret)
			dev_warn(&pdev->dev,"attribute %s create failed %x",
					attr_name(sdhci_jz_sysfs_attrs[i]), ret);
	}

	return 0;

 err_req_regs:
#ifndef CONFIG_PM_RUNTIME
	clk_disable_unprepare(jz->clk_gate);
#endif

 err_pdata_io_clk:
	sdhci_free_host(host);

	return ret;
}

static int sdhci_jz_remove(struct platform_device *pdev)
{
	struct sdhci_host *host =  platform_get_drvdata(pdev);
	struct sdhci_jz *jz = sdhci_priv(host);
#ifdef CONFIG_PM_RUNTIME
	struct jz_sdhci_platdata *pdata = jz->pdata;
#endif
	int i;

	if (jz->ext_cd_irq)
		free_irq(jz->ext_cd_irq, jz);

#ifdef CONFIG_PM_RUNTIME
	if (pdata->cd_type != JZ_SDHCI_CD_INTERNAL)
		clk_prepare_enable(jz->clk_gate);
#endif
	sdhci_remove_host(host, 1);

	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	clk_disable_unprepare(jz->clk_gate);

	sdhci_free_host(host);
	platform_set_drvdata(pdev, NULL);

	for (i = 0; i < ARRAY_SIZE(sdhci_jz_sysfs_attrs); i++) {
		device_remove_file(&pdev->dev, &sdhci_jz_sysfs_attrs[i]);
	}

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sdhci_jz_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	int ret = 0;

	if (host->mmc->card && host->mmc->card->type != MMC_TYPE_SDIO)
		ret = sdhci_suspend_host(host);

	return ret;
}

static int sdhci_jz_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	int ret = 0;

	if (host->mmc->card && host->mmc->card->type != MMC_TYPE_SDIO)
		ret = sdhci_resume_host(host);

	return ret;
}
#endif

#ifdef CONFIG_PM_RUNTIME
static int sdhci_jz_runtime_suspend(struct device *dev)
{
	return 0;
}


static int sdhci_jz_runtime_resume(struct device *dev)
{
	return 0;
}
#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops sdhci_jz_pmops = {
	SET_SYSTEM_SLEEP_PM_OPS(sdhci_jz_suspend, sdhci_jz_resume)
	SET_RUNTIME_PM_OPS(sdhci_jz_runtime_suspend, sdhci_jz_runtime_resume,
			   NULL)
};

#define SDHCI_JZ_PMOPS (&sdhci_jz_pmops)

#else
#define SDHCI_JZ_PMOPS NULL
#endif


static struct platform_device_id sdhci_jz_driver_ids[] = {
	{
		.name		= "jz-sdhci",
		.driver_data	= (kernel_ulong_t)NULL,
	},
	{ }
};
MODULE_DEVICE_TABLE(platform, sdhci_jz_driver_ids);

#ifdef CONFIG_OF
static const struct of_device_id sdhci_jz_dt_match[] = {
	{},
};
MODULE_DEVICE_TABLE(of, sdhci_jz_dt_match);
#endif

static struct platform_driver sdhci_jz_driver = {
	.probe		= sdhci_jz_probe,
	.remove		= sdhci_jz_remove,
	.id_table	= sdhci_jz_driver_ids,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "jz-sdhci",
		.pm	= SDHCI_JZ_PMOPS,
	},
};

module_platform_driver(sdhci_jz_driver);


MODULE_DESCRIPTION("Ingenic SDHCI (MSC) driver");
MODULE_AUTHOR("Large Dipper <wangquan.shao@ingenic.cn>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("20160808");
