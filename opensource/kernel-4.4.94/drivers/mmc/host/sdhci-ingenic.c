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
#include <linux/of_address.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>

#include <linux/mmc/host.h>
#include <soc/cpm.h>
#include <soc/base.h>

#include "sdhci.h"
#include "sdhci-ingenic.h"

#define CLK_CTRL
/* Software redefinition caps */
#define CAPABILITIES1_SW	0x276dc898
#define CAPABILITIES2_SW	0

static LIST_HEAD(manual_list);
#define CPM_MSC0_CLK_R		(0xB0000068)
#define CPM_MSC1_CLK_R		(0xB000006C)

#ifdef CONFIG_FPGA_TEST
#define MSC_CLK_H_FREQ		(0x1 << 20)

static void sdhci_ingenic_fpga_clk(unsigned int clock)
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
	//printk("\tclk=%d, CPM_MSC0_CLK_R: %08x\n\n", clock, readl((const volatile void*)CPM_MSC0_CLK_R));
}
#endif

static unsigned int sdhci_ingenic_get_cpm_msc(struct sdhci_host *host)
{
	char msc_ioaddr[16];
	unsigned int cpm_msc;
	sprintf(msc_ioaddr, "0x%x", (unsigned int)host->ioaddr);

	if (!strcmp(msc_ioaddr ,"0xb3060000"))
		cpm_msc = CPM_MSC0_CLK_R;
	if (!strcmp(msc_ioaddr ,"0xb3070000"))
		cpm_msc = CPM_MSC1_CLK_R;
	return cpm_msc;
}

/**
 * sdhci_ingenic_msc_tuning  Enable msc controller tuning
 *
 * Tuning rx phase
 * */
static void sdhci_ingenic_en_msc_tuning(struct sdhci_host *host, unsigned int cpm_msc)
{
	if (host->flags & SDHCI_SDR50_NEEDS_TUNING ||
		host->flags & SDHCI_SDR104_NEEDS_TUNING ||
		host->flags & SDHCI_HS400_TUNING) {
		*(volatile unsigned int*)cpm_msc &= ~(0x1 << 20);
	}
}

static void sdhci_ingenic_sel_rx_phase(unsigned int cpm_msc)
{
	*(volatile unsigned int*)cpm_msc |= (0x1 << 20); // default

	*(volatile unsigned int*)cpm_msc &= ~(0x7 << 17);
	*(volatile unsigned int*)cpm_msc |= (0x7 << 17); // OK  RX 90 TX 270
}

static void sdhci_ingenic_sel_tx_phase(unsigned int cpm_msc)
{
	*(volatile unsigned int*)cpm_msc &= ~(0x3 << 15);
/*	*(volatile unsigned int*)cpm_msc |= (0x2 << 15); // 180  100M OK*/
	*(volatile unsigned int*)cpm_msc |= (0x3 << 15);
}

/**
 * sdhci_ingenic_set_clock - callback on clock change
 * @host: The SDHCI host being changed
 * @clock: The clock rate being requested.
 *
 * When the card's clock is going to be changed, look at the new frequency
 * and find the best clock source to go with it.
*/
static void sdhci_ingenic_set_clock(struct sdhci_host *host, unsigned int clock)
{
#ifndef CONFIG_FPGA_TEST
	struct sdhci_ingenic *sdhci_ing = sdhci_priv(host);
	unsigned int cpm_msc = sdhci_ingenic_get_cpm_msc(host);
	char clkname[16];


	if (clock == 0)
		return;

	sdhci_set_clock(host, clock);

	spin_unlock_irq(&host->lock);
	sprintf(clkname, "mux_msc%d", sdhci_ing->pdev->id);
	sdhci_ing->clk_mux = clk_get(NULL, clkname);

	if (clock > 400000) {
		clk_set_parent(sdhci_ing->clk_mux, sdhci_ing->clk_mpll);
	} else {
		clk_set_parent(sdhci_ing->clk_mux, sdhci_ing->clk_ext);
		*(volatile unsigned int *)0xB0000068 |= 1 << 21;
	}

	clk_set_rate(sdhci_ing->clk_cgu, clock);

	spin_lock_irq(&host->lock);
	//printk("%s, set clk: %d, get_clk_rate=%ld\n", __func__, clock, clk_get_rate(sdhci_ing->clk_cgu));

	if (host->mmc->ios.timing == MMC_TIMING_MMC_HS200 ||
		host->mmc->ios.timing == MMC_TIMING_UHS_SDR104) {

		/* RX phase selecte */
		if (sdhci_ing->pdata->enable_cpm_rx_tuning == 1)
			sdhci_ingenic_sel_rx_phase(cpm_msc);
		else
			sdhci_ingenic_en_msc_tuning(host, cpm_msc);
		/* TX phase selecte */
		if (sdhci_ing->pdata->enable_cpm_tx_tuning == 1)
			sdhci_ingenic_sel_tx_phase(cpm_msc);
	}
#else //CONFIG_FPGA_TEST
	sdhci_ingenic_fpga_clk(clock);
#endif
}

#ifdef VOLTAGE_SWITCH
static void sdhci_ingenic_set_cpm_poc(int power_v18)
{
	if (power_v18) {
		cpm_outl(0x8, CPM_POC);
	} else {
		cpm_outl(0x0, CPM_POC);
	}

	return;
}
#endif

/* I/O Driver Strength Types */
#define INGENIC_TYPE_0  0x0		//30
#define INGENIC_TYPE_1  0x1		//50
#define INGENIC_TYPE_2  0x2		//66
#define INGENIC_TYPE_3  0x3		//100

static int sdhci_ingenic_select_drive_strength(struct sdhci_host *host,
		struct mmc_card *card,
		unsigned int max_dtr,
		int host_drv, int card_drv, int *drv_type)
{
	int drive_strength, type;

	type = INGENIC_TYPE_1;

	switch (type) {
	case INGENIC_TYPE_0:
		/* Type 0: 50 */
		*(volatile unsigned int*)(0xb0010500 + 0xb8) |= 0xfff;
		*(volatile unsigned int*)(0xb0010500 + 0xb4) |= 0x555;
		drive_strength = 0;
		break;
	case INGENIC_TYPE_1:
		/* Type 1: 33 */
		*(volatile unsigned int*)(0xb0010500 + 0xb8) |= 0xfff;
		*drv_type = MMC_CAP_DRIVER_TYPE_A; // GPIO DRV_STR
		drive_strength = 1;
		break;
	case INGENIC_TYPE_2:
		/* Type 2: 66 */
		*(volatile unsigned int*)(0xb0010500 + 0xb8) |= 0xfff;
		*(volatile unsigned int*)(0xb0010500 + 0xb4) |= 0xaaa;
		*drv_type = MMC_CAP_DRIVER_TYPE_C; // GPIO DRV_STR
		drive_strength = 2;
		break;
	case INGENIC_TYPE_3:
		/* Type 3: 100 */
		*(volatile unsigned int*)(0xb0010500 + 0xb8) |= 0xfff;
		*(volatile unsigned int*)(0xb0010500 + 0xb4) |= 0xfff;
		*drv_type = MMC_CAP_DRIVER_TYPE_D; // GPIO DRV_STR
		drive_strength = 3;
		break;
	}

	return drive_strength;
}

#ifdef VOLTAGE_SWITCH
void sdhci_ingenic_voltage_switch(struct sdhci_host *host)
{
	struct sdhci_ingenic *sdhci_ing = sdhci_priv(host);
	struct sdhci_ingenic_pdata *pdata = sdhci_ing->pdata;
	unsigned int val;

	if (pdata->sdr_v18 < 0)
		return;

	/*controlled SD voltage to 1.8V*/
	val = cpm_inl(CPM_EXCLK_DS) | (1 << 31);
	cpm_outl(val, CPM_EXCLK_DS);

	/*Set up hardware circuit 1.8V*/
	val = pdata->sdr_v18;

	gpio_direction_output(val, 1);
}
#endif

static struct sdhci_ops sdhci_ingenic_ops = {
	.set_clock							  = sdhci_ingenic_set_clock,
	.set_bus_width						  = sdhci_set_bus_width,
	.reset								  = sdhci_reset,
	.set_uhs_signaling					  = sdhci_set_uhs_signaling,
	.select_drive_strength				  = sdhci_ingenic_select_drive_strength,
#ifdef VOLTAGE_SWITCH
	.voltage_switch						  = sdhci_ingenic_voltage_switch,
#endif
};

static void sdhci_ingenic_notify_change(struct platform_device *dev, int state)
{
	struct sdhci_host *host = platform_get_drvdata(dev);
	unsigned long flags;
#ifdef VOLTAGE_SWITCH
	struct sdhci_ingenic *sdhci_ing = sdhci_priv(host);
	struct sdhci_ingenic_pdata *pdata = sdhci_ing->pdata;
	unsigned long val;
#endif

	if (host) {
		spin_lock_irqsave(&host->lock, flags);
		if (state) {
			printk("%s: card inserted\n", mmc_hostname(host->mmc));
			host->flags &= ~SDHCI_DEVICE_DEAD;
			host->quirks |= SDHCI_QUIRK_BROKEN_CARD_DETECTION;

			mmc_detect_change(host->mmc, 0);
		} else {
			printk("%s: card removed\n", mmc_hostname(host->mmc));
			host->flags |= SDHCI_DEVICE_DEAD;
			host->quirks &= ~SDHCI_QUIRK_BROKEN_CARD_DETECTION;
#ifdef VOLTAGE_SWITCH
			if (pdata->sdr_v18 > 0) {
				gpio_direction_output(pdata->sdr_v18, 0);

				val = cpm_inl(CPM_EXCLK_DS) & ~(1 << 31);
				cpm_outl(val, CPM_EXCLK_DS);
			}
#endif
			mmc_detect_change(host->mmc, 0);
		}
		tasklet_schedule(&host->finish_tasklet);
		spin_unlock_irqrestore(&host->lock, flags);
	}
}

static irqreturn_t sdhci_ingenic_gpio_card_detect_thread(int irq, void *dev_id)
{
	struct sdhci_ingenic *sdhci_ing = (struct sdhci_ingenic *)dev_id;
	int status = gpio_get_value(sdhci_ing->ext_cd_gpio);

	if (!(sdhci_ing->pdata->gpio->cd.enable_level))
		status = !status;
	sdhci_ingenic_notify_change(sdhci_ing->pdev, status);
	return IRQ_HANDLED;
}

static void sdhci_ingenic_setup_card_detect_gpio(struct sdhci_ingenic *sdhci_ing)
{
	struct sdhci_ingenic_pdata *pdata = sdhci_ing->pdata;
	struct device *dev = &sdhci_ing->pdev->dev;

	if (devm_gpio_request(dev, pdata->gpio->cd.num, "SDHCI EXT CD") == 0) {
		sdhci_ing->ext_cd_gpio = pdata->gpio->cd.num;
		sdhci_ing->ext_cd_irq = gpio_to_irq(pdata->gpio->cd.num);
		if (sdhci_ing->ext_cd_irq &&
		    request_threaded_irq(sdhci_ing->ext_cd_irq, NULL,
								 sdhci_ingenic_gpio_card_detect_thread,
								 IRQF_TRIGGER_RISING |
								 IRQF_TRIGGER_FALLING |
								 IRQF_ONESHOT,
								 dev_name(dev), sdhci_ing) == 0) {
			int status = gpio_get_value(sdhci_ing->ext_cd_gpio);
			if (!(pdata->gpio->cd.enable_level))
				status = !status;
			sdhci_ingenic_notify_change(sdhci_ing->pdev, status);
		} else {
			dev_warn(dev, "cannot request irq for card detect\n");
			sdhci_ing->ext_cd_irq = 0;
		}
	} else {
		dev_err(dev, "cannot request gpio for card detect\n");
	}
}

#ifdef CONFIG_OF
static void ingenic_mmc_get_gpio(struct device_node *np,
								 struct ingenic_mmc_pin *pin, char *gpioname)
{
	int gpio;
	enum of_gpio_flags flags;

	pin->num = -EBUSY;
	gpio = of_get_named_gpio_flags(np, gpioname, 0, &flags);
	if(gpio_is_valid(gpio)) {
		pin->num = gpio;
		pin->enable_level =
			(flags == OF_GPIO_ACTIVE_LOW ? LOW_ENABLE : HIGH_ENABLE);
		//printk("mmc gpio %s num:%d en-level: %d\n", gpioname, pin->num, pin->enable_level);
	}
}

static inline void ingenic_mmc_clk_onoff(struct sdhci_ingenic *ingenic_ing, unsigned int on)
{
	if(on) {
		clk_prepare_enable(ingenic_ing->clk_cgu);
		clk_prepare_enable(ingenic_ing->clk_gate);
	} else {
		clk_disable_unprepare(ingenic_ing->clk_cgu);
		clk_disable_unprepare(ingenic_ing->clk_gate);
	}
}

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
	struct sdhci_ingenic *sdhci_ing;
	struct sdhci_host *host;
	struct list_head *pos;

	list_for_each(pos, &manual_list) {
		sdhci_ing = list_entry(pos, struct sdhci_ingenic, list);
		if (sdhci_ing->pdev->id == index) {
			break;
		} else
			sdhci_ing = NULL;
	}

	if(!sdhci_ing) {
		printk("no manual card detect\n");
		return -1;
	}

	host = sdhci_ing->host;

	if (on) {
		dev_err(&sdhci_ing->pdev->dev, "card insert manually\n");
		set_bit(INGENIC_MMC_CARD_PRESENT, &sdhci_ing->flags);
#ifdef CLK_CTRL
		ingenic_mmc_clk_onoff(sdhci_ing, 1);
#endif
		host->flags &= ~SDHCI_DEVICE_DEAD;
		host->quirks |= SDHCI_QUIRK_BROKEN_CARD_DETECTION;
		mmc_detect_change(sdhci_ing->host->mmc, 0);
	} else {
		dev_err(&sdhci_ing->pdev->dev, "card remove manually\n");
		clear_bit(INGENIC_MMC_CARD_PRESENT, &sdhci_ing->flags);

		host->flags |= SDHCI_DEVICE_DEAD;
		host->quirks &= ~SDHCI_QUIRK_BROKEN_CARD_DETECTION;
		mmc_detect_change(sdhci_ing->host->mmc, 0);
#ifdef CLK_CTRL
		ingenic_mmc_clk_onoff(sdhci_ing, 0);
#endif
	}

	tasklet_schedule(&host->finish_tasklet);

	return 0;
}
EXPORT_SYMBOL(jzmmc_manual_detect);

/**
 *	ingenic_mmc_clk_ctrl - enable or disable msc clock gate
 *	@index: host->index, namely the index of the controller.
 *	@on: 1-enable msc clock gate, 0-disable msc clock gate.
 */
int ingenic_mmc_clk_ctrl(int index, int on)
{
	struct sdhci_ingenic *sdhci_ing;
	struct list_head *pos;

#ifdef CLK_CTRL
	list_for_each(pos, &manual_list) {
		sdhci_ing = list_entry(pos, struct sdhci_ingenic, list);
		if (sdhci_ing->pdev->id == index)
			break;
		else
			sdhci_ing = NULL;
	}

	if (!sdhci_ing) {
		printk("no manual card detect\n");
		return -1;
	}
	ingenic_mmc_clk_onoff(sdhci_ing, on);
#endif
	return 0;
}
EXPORT_SYMBOL(ingenic_mmc_clk_ctrl);

static int sdhci_ingenic_parse_dt(struct device *dev,
								  struct sdhci_host *host,
								  struct sdhci_ingenic_pdata *pdata)
{
	struct device_node *np = dev->of_node;
	struct card_gpio *card_gpio;
	unsigned int val;

	card_gpio = devm_kzalloc(dev, sizeof(struct card_gpio), GFP_KERNEL);
	if(!card_gpio)
		return 0;

	ingenic_mmc_get_gpio(np, &card_gpio->rst, "ingenic,rst-gpios");
	ingenic_mmc_get_gpio(np, &card_gpio->wp, "ingenic,wp-gpios");
	ingenic_mmc_get_gpio(np, &card_gpio->pwr, "ingenic,pwr-gpios");
	ingenic_mmc_get_gpio(np, &card_gpio->cd, "ingenic,cd-gpios");
	pdata->gpio = card_gpio;


	pdata->sdr_v18 = of_get_named_gpio(np, "ingenic,sdr-gpios", 0);

	/* assuming internal card detect that will be configured by pinctrl */
	pdata->cd_type = SDHCI_INGENIC_CD_INTERNAL;

	if(of_property_read_bool(np, "pio-mode")) {
		pdata->pio_mode = 1;
	}
	if(of_property_read_bool(np, "enable_autocmd12")) {
		pdata->enable_autocmd12 = 1;
	}
	if(of_property_read_bool(np, "enable_cpm_rx_tuning")) {
		pdata->enable_cpm_rx_tuning = 1;
	}
	if(of_property_read_bool(np, "enable_cpm_tx_tuning")) {
		pdata->enable_cpm_tx_tuning = 1;
	}

	/* get the card detection method */
	if (of_get_property(np, "broken-cd", NULL)) {
		pdata->cd_type = SDHCI_INGENIC_CD_NONE;
	}

	if (of_get_property(np, "non-removable", NULL)) {
		pdata->cd_type = SDHCI_INGENIC_CD_PERMANENT;
	}

	if (of_get_property(np, "cd-inverted", NULL)) {
		pdata->cd_type = SDHCI_INGENIC_CD_GPIO;
	}

	if(!(of_property_read_u32(np, "ingenic,sdio_clk", &val)))
		pdata->sdio_clk = val;

	/*set Power-On-Control Select of GPIO PE group*/
	if(of_property_read_bool(np, "ingenic,poc-v1.8")) {
		pdata->poc_v18 = 1;
	}

	/* if(of_property_read_bool(np, "ingenic,removal-dontcare")) { */
	/* 	pdata->removal = DONTCARE; */
	/* } else if(of_property_read_bool(np, "ingenic,removal-nonremovable")) { */
	/* 	pdata->removal = NONREMOVABLE; */
	/* } else if(of_property_read_bool(np, "ingenic,removal-removable")) { */
	/* 	pdata->removal = REMOVABLE; */
	/* } else if(of_property_read_bool(np, "ingenic,removal-manual")) { */
	/* 	pdata->removal = MANUAL; */
	/* }; */

	/* mmc_of_parse_voltage(np, &pdata->ocr_avail); */

	return 0;
}
#else
static int sdhci_ingenic_parse_dt(struct device *dev,
								  struct sdhci_host *host,
								  struct sdhci_ingenic_pdata *pdata)
{
	return -EINVAL;
}
#endif

static int sdhci_ingenic_probe(struct platform_device *pdev)
{
	struct sdhci_ingenic_pdata *pdata;
	struct device *dev = &pdev->dev;
	struct sdhci_host *host;
	struct sdhci_ingenic *sdhci_ing;
	char clkname[16];
	int ret, irq;

	if (!pdev->dev.platform_data && !pdev->dev.of_node) {
		dev_err(dev, "no device data specified\n");
		return -ENOENT;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "no irq specified\n");
		return irq;
	}

	host = sdhci_alloc_host(dev, sizeof(struct sdhci_ingenic));
	if (IS_ERR(host)) {
		dev_err(dev, "sdhci_alloc_host() failed\n");
		return PTR_ERR(host);
	}
	sdhci_ing = sdhci_priv(host);

	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		ret = -ENOMEM;
		return ret;
	}

	if (pdev->dev.of_node) {
		ret = sdhci_ingenic_parse_dt(&pdev->dev, host, pdata);
		if (ret)
			return ret;
	} else {
		memcpy(pdata, pdev->dev.platform_data, sizeof(*pdata));
	}

	pdev->id = of_alias_get_id(pdev->dev.of_node, "msc");

	sprintf(clkname, "div_msc%d", pdev->id);
	sdhci_ing->clk_cgu = devm_clk_get(&pdev->dev, clkname);
	if(!sdhci_ing->clk_cgu) {
		dev_err(&pdev->dev, "Failed to Get MSC clk!\n");
		return PTR_ERR(sdhci_ing->clk_cgu);
	}
	sprintf(clkname, "gate_msc%d", pdev->id);
	sdhci_ing->clk_gate = devm_clk_get(&pdev->dev, clkname);
	if(!sdhci_ing->clk_gate) {
		dev_err(&pdev->dev, "Failed to Get PWC MSC clk!\n");
		return PTR_ERR(sdhci_ing->clk_gate);
	}
	sdhci_ing->clk_ext = clk_get(NULL, "ext");
	sdhci_ing->clk_mpll = clk_get(NULL, "mpll");

	ingenic_mmc_clk_onoff(sdhci_ing, 1);

	sdhci_ing->host = host;
	sdhci_ing->dev  = &pdev->dev;
	sdhci_ing->pdev = pdev;
	sdhci_ing->pdata = pdata;

	host->ioaddr= of_iomap(pdev->dev.of_node, 0);
	if (IS_ERR(host->ioaddr)) {
		return PTR_ERR(host->ioaddr);
	}

	platform_set_drvdata(pdev, host);

#ifdef VOLTAGE_SWITCH
	if (pdata->poc_v18){
		sdhci_ingenic_set_cpm_poc(1);
	} else {
		sdhci_ingenic_set_cpm_poc(0);
	}
#endif

	/* sdio for WIFI init*/
	if (pdata->sdio_clk) {
		ingenic_sdio_wlan_init(&pdev->dev, pdev->id);
		list_add(&(sdhci_ing->list), &manual_list);
	}

	if (pdata->sdr_v18 > 0 && \
			devm_gpio_request(dev, pdata->sdr_v18, "sdr-v18")) {
		printk("ERROR: no sdr-v18 pin available !!\n");
	}

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_enable(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, 50);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_suspend_ignore_children(&pdev->dev, 1);
#endif

	ret = mmc_of_parse(host->mmc);
	if (ret) {
		dev_err(dev, "mmc_of_parse() failed\n");
		pm_runtime_forbid(&pdev->dev);
		pm_runtime_get_noresume(&pdev->dev);
		return ret;
	}

	host->hw_name = "ingenic-sdhci";
	host->ops = &sdhci_ingenic_ops;
	host->quirks = 0;
	host->irq = irq;

	/* Software redefinition caps */
	host->quirks |= SDHCI_QUIRK_MISSING_CAPS;
	host->caps  = CAPABILITIES1_SW;
	host->caps1 = CAPABILITIES2_SW;

	/* not check wp */
	host->quirks |= SDHCI_QUIRK_INVERTED_WRITE_PROTECT;

	/* Setup quirks for the controller */
	host->quirks |= SDHCI_QUIRK_NO_ENDATTR_IN_NOPDESC;
	host->quirks |= SDHCI_QUIRK_NO_HISPD_BIT;

	/* Data Timeout Counter Value */
	//host->quirks |= SDHCI_QUIRK_BROKEN_TIMEOUT_VAL;
	host->timeout_clk = 24000; //TMCLK = 24MHz

	/* This host supports the Auto CMD12 */
	if(pdata->enable_autocmd12)
		host->quirks |= SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12;

	/* PIO transfer mode */
	if(pdata->pio_mode){
		host->quirks |= SDHCI_QUIRK_BROKEN_DMA;
		host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;
	}
	/* TODO:SoCs need BROKEN_ADMA_ZEROLEN_DESC */
/*	host->quirks |= SDHCI_QUIRK_BROKEN_ADMA_ZEROLEN_DESC;*/
	if (!((host->mmc->caps & MMC_CAP_8_BIT_DATA) ||
		(host->mmc->caps & MMC_CAP_4_BIT_DATA)))
		host->quirks |= SDHCI_QUIRK_FORCE_1_BIT_DATA;

	if (pdata->cd_type == SDHCI_INGENIC_CD_NONE ||
	    pdata->cd_type == SDHCI_INGENIC_CD_PERMANENT)
		host->quirks |= SDHCI_QUIRK_BROKEN_CARD_DETECTION;

	if (pdata->cd_type == SDHCI_INGENIC_CD_PERMANENT)
		host->mmc->caps |= MMC_CAP_NONREMOVABLE;

	if (pdata->pm_caps)
		host->mmc->pm_caps |= pdata->pm_caps;

	host->quirks |= (SDHCI_QUIRK_32BIT_DMA_ADDR |
					 SDHCI_QUIRK_32BIT_DMA_SIZE);

	/* It supports additional host capabilities if needed */
	if (pdata->host_caps)
		host->mmc->caps |= pdata->host_caps;

	if (pdata->host_caps2)
		host->mmc->caps2 |= pdata->host_caps2;

	ret = sdhci_add_host(host);
	if (ret) {
		dev_err(dev, "sdhci_add_host() failed\n");
		pm_runtime_forbid(&pdev->dev);
		pm_runtime_get_noresume(&pdev->dev);
		return ret;
	}

	if (pdata->cd_type == SDHCI_INGENIC_CD_GPIO &&
	    gpio_is_valid(pdata->gpio->cd.num))
		sdhci_ingenic_setup_card_detect_gpio(sdhci_ing);

#ifdef CONFIG_PM_RUNTIME
	if (pdata->cd_type != SDHCI_INGENIC_CD_INTERNAL) {
		clk_disable_unprepare(sdhci_ing->clk_cgu);
		/* clk_disable_unprepare(sdhci_ing->clk_gate); */
	}
#endif

	return 0;
}

static int sdhci_ingenic_remove(struct platform_device *pdev)
{
	struct sdhci_host *host =  platform_get_drvdata(pdev);
	struct sdhci_ingenic *sdhci_ing = sdhci_priv(host);

	if (sdhci_ing->ext_cd_irq)
		free_irq(sdhci_ing->ext_cd_irq, sdhci_ing);

#ifdef CONFIG_PM_RUNTIME
	if (pdata->cd_type != SDHCI_INGENIC_CD_INTERNAL){
		/* clk_prepare_enable(sdhci_ing->clk_gate); */
		clk_prepare_enable(sdhci_ing->clk_cgu);
	}
#endif
	sdhci_remove_host(host, 1);

	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	clk_disable_unprepare(sdhci_ing->clk_cgu);
	/* clk_disable_unprepare(sdhci_ing->clk_gate); */

	sdhci_free_host(host);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sdhci_ingenic_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_ingenic *sdhci_ing = sdhci_priv(host) ;

	ingenic_mmc_clk_onoff(sdhci_ing, 0);

	return sdhci_suspend_host(host);
}

static int sdhci_ingenic_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_ingenic *sdhci_ing = sdhci_priv(host) ;

	ingenic_mmc_clk_onoff(sdhci_ing, 1);

	return sdhci_resume_host(host);
}
#endif

#ifdef CONFIG_PM
static int sdhci_ingenic_runtime_suspend(struct device *dev)
{
	return 0;
}


static int sdhci_ingenic_runtime_resume(struct device *dev)
{
	return 0;
}
#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops sdhci_ingenic_pmops = {
	SET_SYSTEM_SLEEP_PM_OPS(sdhci_ingenic_suspend, sdhci_ingenic_resume)
	SET_RUNTIME_PM_OPS(sdhci_ingenic_runtime_suspend, sdhci_ingenic_runtime_resume,
			   NULL)
};

#define SDHCI_INGENIC_PMOPS (&sdhci_ingenic_pmops)

#else
#define SDHCI_INGENIC_PMOPS NULL
#endif


static struct platform_device_id sdhci_ingenic_driver_ids[] = {
	{
		.name		= "ingenic,sdhci",
		.driver_data	= (kernel_ulong_t)NULL,
	},
	{ }
};
MODULE_DEVICE_TABLE(platform, sdhci_ingenic_driver_ids);

#ifdef CONFIG_OF
static const struct of_device_id sdhci_ingenic_dt_match[] = {
	{.compatible = "ingenic,sdhci",},
	{},
};
MODULE_DEVICE_TABLE(of, sdhci_ingenic_dt_match);
#endif

static struct platform_driver sdhci_ingenic_driver = {
	.probe		= sdhci_ingenic_probe,
	.remove		= sdhci_ingenic_remove,
	.id_table	= sdhci_ingenic_driver_ids,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "ingenic,sdhci",
		.pm	= SDHCI_INGENIC_PMOPS,
		.of_match_table = of_match_ptr(sdhci_ingenic_dt_match),
	},
};

module_platform_driver(sdhci_ingenic_driver);


MODULE_DESCRIPTION("Ingenic SDHCI (MSC) driver");
MODULE_AUTHOR("Large Dipper <wangquan.shao@ingenic.cn>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("20160808");
