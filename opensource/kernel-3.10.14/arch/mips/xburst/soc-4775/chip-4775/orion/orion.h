#ifndef __ORION_H__
#define __ORION_H__
#include <gpio.h>

/* MSC GPIO Definition */
#define GPIO_SD2_VCC_EN_N	GPIO_PC(29)
#define GPIO_SD2_CD_N		GPIO_PB(24)

#define GPIO_SD0_VCC_EN_N	GPIO_PB(28)
#define GPIO_SD0_CD_N		GPIO_PE(0)

#define GPIO_SD1_VCC_EN_N	GPIO_PB(29)
#define GPIO_SD1_CD_N		GPIO_PB(2)

/* GT801 Touchscreen GPIO Definition */
#define GPIO_TP_DRV_EN			GPIO_PE(12)
#define GPIO_GT801_IRQ			GPIO_PB(27)
#define GPIO_GT801_SHUTDOWN		GPIO_PB(19)

#define GPIO_CTP_IRQ			GPIO_PD(14)
#define GPIO_CTP_WAKE_UP		GPIO_PD(22)

extern struct jzmmc_platform_data orion_inand_pdata;
extern struct jzmmc_platform_data orion_tf_pdata;
extern struct jzmmc_platform_data orion_sdio_pdata;

#ifdef CONFIG_FB_JZ4780_LCDC0
extern struct jzfb_platform_data jzfb0_pdata;
#endif
#ifdef CONFIG_FB_JZ4780_LCDC1
extern struct jzfb_platform_data jzfb1_pdata;
#endif

/**
 *audio gpio
 **/
#define GPIO_I2S_MUTE		GPIO_PB(30)

#define GPIO_SPEAKER_SHUTDOWN	GPIO_PB(3)

#define GPIO_HP_DETECT		GPIO_PA(17)
/**
 * sound platform data
 **/
extern struct snd_codec_data codec_data;

extern struct platform_device orion_backlight_device;

#ifdef CONFIG_LCD_AUO_A043FL01V2
extern struct platform_device auo_a043fl01v2_device;
#endif
#ifdef CONFIG_LCD_AT070TN93
extern struct platform_device at070tn93_device;
#endif

#endif /* __ORION_H__ */
