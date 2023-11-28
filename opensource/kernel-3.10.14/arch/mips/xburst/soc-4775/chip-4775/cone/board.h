#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>

extern struct jzmmc_platform_data sdio_pdata;

extern struct jzfb_platform_data jzfb0_pdata;
/* panel and bl platform device */
extern struct platform_device backlight_device;

#ifdef CONFIG_LCD_KD301_M03545_0317A
extern struct platform_device kd301_device;
#endif

/**
 * lcd gpio
 **/
#define GPIO_LCD_PWM		GPIO_PE(1)

#define GPIO_HP_MUTE		-1	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL		-1		/*vaild level*/

/* NOTE: although we have AMP EN pin, but we do not have headphone ports,
 * so we will control AMP EN pin when codec standby/wakeup
 */
#define GPIO_SPEAKER_EN			-1/*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL	-1

#define GPIO_HANDSET_EN		  -1		/*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL -1

#define	GPIO_HP_DETECT		-1
#define GPIO_HP_INSERT_LEVEL    1
#define GPIO_MIC_SELECT		-1		/*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL	-1		/*builin mic select level*/
#define GPIO_MIC_DETECT		-1
#define GPIO_MIC_INSERT_LEVEL -1
#define GPIO_MIC_DETECT_EN		-1  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL	-1 /*mic detect enable gpio*/

/**
 * KEY gpio
 **/
#define GPIO_ENDCALL            GPIO_PA(30)
#define ACTIVE_LOW_ENDCALL      1

/**
 * USB detect pin
 **/
#define GPIO_USB_DETE                   GPIO_PF(7)

/**
 * pmem information
 **/
/* auto allocate pmem in arch_mem_init(), do not assigned base addr, just set 0 */
#define JZ_PMEM_ADSP_BASE   0x0          // 0x0e000000
#define JZ_PMEM_ADSP_SIZE   0x02000000

/**
 * sound platform data
 **/
extern struct snd_codec_data codec_data;

#ifdef CONFIG_BCM4330_RFKILL
extern struct platform_device bcm4330_bt_power_device;
#endif

#endif /* __BOARD_H__ */
