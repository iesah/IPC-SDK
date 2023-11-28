/*
 * SFC controller for SPI protocol, use FIFO and DMA;
 *
 * Copyright (c) 2015 Ingenic
 * Author: <xiaoyang.fu@ingenic.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include "ingenic_sfc_drv.h"
#include "sfc.h"
#include "sfc_flash.h"
#include "ingenic_sfc_common.h"
#include "spinor.h"

static int __init ingenic_sfc_probe(struct platform_device *pdev)
{
	struct sfc_flash *flash;
	int ret = 0;
	const struct of_device_id *of_match;

	flash = kzalloc(sizeof(struct sfc_flash), GFP_KERNEL);
	if (IS_ERR_OR_NULL(flash))
		return -ENOMEM;

	platform_set_drvdata(pdev, flash);
	flash->dev = &pdev->dev;
	flash->id = pdev->id;

	flash->pdata = pdev->dev.platform_data;
	if (flash->pdata == NULL) {
		dev_err(&pdev->dev, "No platform data supplied\n");
		kfree(flash);
		return -ENOENT;
	}
	flash->flash_type = flash->pdata->spiflash_type;

	flash->sfc = sfc_res_init(pdev);
	if(IS_ERR(flash->sfc)) {
		dev_err(flash->dev, "sfc control init error!\n");
		kfree(flash);
		return PTR_ERR(flash->sfc);
	}

	//flash->pdata_params = pdev->dev.platform_data;
	mutex_init(&flash->lock);

	if(flash->flash_type == -1){						/*flash type is not declared in bootargs */
		dev_err(flash->dev, "flash type error!\n");
		kfree(flash);
		return PTR_ERR(flash->sfc);
	}

	switch(flash->flash_type)
	{
		case NOR:
#ifdef CONFIG_MTD_INGENIC_SFC_V2_NORFLASH
			ret = ingenic_sfc_nor_probe(flash);
#endif
			break;
		case NAND:
#ifdef CONFIG_MTD_INGENIC_SFC_V2_NANDFLASH
			ret = ingenic_sfc_nand_probe(flash);
#endif
			break;
		default:
			dev_err(&pdev->dev, "flash type error\n");
			ret = -EINVAL;
	}
	if(ret){
		kfree(flash);
		return ret;
	}

	return 0;
}

static int __exit ingenic_sfc_remove(struct platform_device *pdev)
{
	struct sfc_flash *flash = platform_get_drvdata(pdev);
	struct sfc *sfc = flash->sfc;

	clk_disable_unprepare(sfc->clk_gate);
	clk_put(sfc->clk_gate);
	clk_disable_unprepare(sfc->clk);
	clk_put(sfc->clk);
	free_irq(sfc->irq, flash);
	iounmap(sfc->iomem);
	release_mem_region(sfc->ioarea->start, resource_size(sfc->ioarea));
	platform_set_drvdata(pdev, NULL);
	if(flash->flash_type == NOR){
		sysfs_remove_group(&pdev->dev.kobj, flash->attr_group);
	} else if(flash->flash_type == NAND){

	} else {
		dev_err(&pdev->dev, "flash type error\n");
		return -EINVAL;
	}
	return 0;
}


static int ingenic_sfc_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct sfc_flash *flash = platform_get_drvdata(pdev);
	int ret = 0;

	switch(flash->flash_type)
	{
		case NOR:
#ifdef CONFIG_MTD_INGENIC_SFC_V2_NORFLASH
			ret = ingenic_sfc_nor_suspend(flash);
#endif
			break;
		case NAND:
#ifdef CONFIG_MTD_INGENIC_SFC_V2_NANDFLASH
			ret = ingenic_sfc_nand_suspend(flash);
#endif
			break;
		default:
			dev_err(&pdev->dev, "flash type error\n");
			ret = -EINVAL;
	}
	if(ret){
		kfree(flash);
		return ret;
	}

	return 0;
}

static int ingenic_sfc_resume(struct platform_device *pdev)
{
	struct sfc_flash *flash = platform_get_drvdata(pdev);
	struct sfc *sfc = flash->sfc;
	int ret = 0;

	ingenic_sfc_init_setup(sfc);
	switch(flash->flash_type)
	{
		case NOR:
#ifdef CONFIG_MTD_INGENIC_SFC_V2_NORFLASH
			ret = ingenic_sfc_nor_resume(flash);
#endif
			break;
		case NAND:
#ifdef CONFIG_MTD_INGENIC_SFC_V2_NANDFLASH
			ret = ingenic_sfc_nand_resume(flash);
#endif
			break;
		default:
			dev_err(&pdev->dev, "flash type error\n");
			ret = -EINVAL;
	}
	if(ret){
		kfree(flash);
		return ret;
	}

	return 0;
}

void ingenic_sfc_shutdown(struct platform_device *pdev)
{
	struct sfc_flash *flash = platform_get_drvdata(pdev);
	struct sfc *sfc = flash->sfc;

	disable_irq(sfc->irq);
	clk_disable_unprepare(sfc->clk_gate);
	clk_disable_unprepare(sfc->clk);
	return ;
}
static struct platform_driver ingenic_sfcdrv = {
	.driver		= {
		.name	= "jz-sfc",
		.owner	= THIS_MODULE,
	},
	.remove		= __exit_p(ingenic_sfc_remove),
	.suspend	= ingenic_sfc_suspend,
	.resume		= ingenic_sfc_resume,
	.shutdown	= ingenic_sfc_shutdown,
};
static int __init ingenic_sfc_init(void)
{
	return platform_driver_probe(&ingenic_sfcdrv, ingenic_sfc_probe);
}

static void __exit ingenic_sfc_exit(void)
{
    platform_driver_unregister(&ingenic_sfcdrv);
}

module_init(ingenic_sfc_init);
module_exit(ingenic_sfc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("INGENIC SFC Driver");
