#ifndef __TEST_H__
#define __TEST_H__
#include <gpio.h>

/* MSC GPIO Definition */
#define GPIO_SD2_VCC_EN_N		GPIO_PC(29)
#define GPIO_SD2_CD_N			GPIO_PB(24)

/* GT801 Touchscreen GPIO Definition */
#define GPIO_TP_DRV_EN			GPIO_PE(12)
#define GPIO_GT801_IRQ			GPIO_PB(27)
#define GPIO_GT801_SHUTDOWN		GPIO_PB(19)

/**
 * mmc platform data
 **/
extern struct jzmmc_platform_data giant_inand_pdata;
extern struct jzmmc_platform_data giant_tf_pdata;
extern struct jzmmc_platform_data giant_sdio_pdata;

/**
 * lcd platform data
 **/
#ifdef CONFIG_FB_JZ4780_LCDC0
extern struct jzfb_platform_data jzfb0_pdata;
#endif
#ifdef CONFIG_FB_JZ4780_LCDC1
extern struct jzfb_platform_data jzfb1_pdata;
#endif

/**
 * lcd platform device
 **/
#ifdef CONFIG_LCD_AUO_A043FL01V2
extern struct platform_device auo_a043fl01v2_device;
#endif
#ifdef CONFIG_LCD_AT070TN93
extern struct platform_device at070tn93_device;
#endif
extern struct platform_device giant_backlight_device;

/**
 * sound platform data
 **/
extern struct snd_codec_data codec_data;

#endif /* __TEST_H__ */
