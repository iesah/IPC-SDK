#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>


#ifndef CONFIG_BOARD_NAME
#define CONFIG_BOARD_NAME "cone_v1_2"
#endif

#ifdef CONFIG_FB_JZ4780_LCDC0
extern struct jzfb_platform_data jzfb0_pdata;
#endif

#ifdef CONFIG_LCD_TM035PDH03
extern struct platform_device tm035_device;
#endif
#ifdef CONFIG_LCD_KD301_M03545_0317A
extern struct platform_device kd301_device;
#endif
extern struct platform_device backlight_device;

extern struct jzmmc_platform_data inand_pdata;
extern struct jzmmc_platform_data sdio_pdata;

extern struct snd_codec_data codec_data;

/*
 * lcd gpio
 */
#define GPIO_LCD_PWM		GPIO_PE(1)

/*
 * speaker gpio
 */
#define GPIO_SPEAKER_EN		GPIO_PA(16)
#define GPIO_SPEAKER_EN_LEVEL   0

#define GPIO_HP_MUTE		-1	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL	-1	/*vaild level*/

#define GPIO_HANDSET_EN		-1	/*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL   -1

#define	GPIO_HP_DETECT		-1
#define GPIO_HP_INSERT_LEVEL    1
#define GPIO_MIC_SELECT		-1	/*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL	-1	/*builin mic select level*/
#define GPIO_MIC_DETECT		-1
#define GPIO_MIC_INSERT_LEVEL   -1
#define GPIO_MIC_DETECT_EN	-1      /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL -1     /*mic detect enable gpio*/

/**
 * KEY gpio
 **/
#define GPIO_ENDCALL            GPIO_PA(30)
#define ACTIVE_LOW_ENDCALL      1

/**
 * USB detect pin
 **/
#define GPIO_USB_DETE           GPIO_PF(7)

/**
 * TP gpio
 **/
#define GPIO_TP_WAKE		GPIO_PB(20)
#define GPIO_TP_INT		GPIO_PC(20)

/**
 * pmem information
 **/
/* auto allocate pmem in arch_mem_init(), do not assigned base addr, just set 0 */
#define JZ_PMEM_ADSP_BASE   0x0        // 0x1e000000
#define JZ_PMEM_ADSP_SIZE   0x02000000

#endif
