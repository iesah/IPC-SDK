#ifndef __HLBA1_H__
#define __HLBA1_H__
#include <gpio.h>

/**
 * mmc platform data
 **/
extern struct jzmmc_platform_data hlba1_inand_pdata;
extern struct jzmmc_platform_data hlba1_tf_pdata;
extern struct jzmmc_platform_data hlba1_sdio_pdata;

/**
 * lcd platform data
 **/
#ifdef CONFIG_FB_JZ4780_LCDC0
extern struct jzfb_platform_data jzfb0_hdmi_pdata;
#endif
#ifdef CONFIG_FB_JZ4780_LCDC1
extern struct jzfb_platform_data jzfb1_pdata;
#endif

/**
 * lcd platform device
 **/
#ifdef CONFIG_LCD_KR070LA0S_270
extern struct platform_device kr070la0s_270_device;
#endif
#ifdef CONFIG_LCD_EK070TN93
extern struct platform_device ek070tn93_device;
#endif
extern struct platform_device hlba1_backlight_device;

/**
 * sound platform data
 **/
extern struct snd_codec_data codec_data;

/**
 * audio gpio
 **/
#define GPIO_HP_MUTE		-1//GPIO_PD(13)	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL	-1//1		/*vaild level*/

#define GPIO_SPEAKER_EN		-1//GPIO_PE(6)	/*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL	-1//1

#define	GPIO_HP_DETECT		-1		/*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL	-1

#define GPIO_MIC_DETECT		-1		/*mic detect gpio*/
#define GPIO_MIC_INSERT_LEVEL	-1

#define GPIO_MIC_SELECT		-1		/*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL	-1		/*buildin mic select level*/

#define GPIO_HANDSET_EN		  -1		/*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL -1

#define GPIO_MIC_DETECT_EN		-1  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL	-1 /*mic detect enable gpio*/

/**
 * tsc gpio interface
 **/
#define GPIO_CTP_IRQ			GPIO_PF(19)
#define GPIO_CTP_WAKE_UP		GPIO_PF(18)

/**
 * g sensor gpio interface
 **/
#define GPIO_MMA8452_INT1		GPIO_PF(9)
#define GPIO_LIS3DH_INT1		GPIO_PF(9)

/**
 * IR remote gpio interface
 **/
#define GPIO_REMOTE_PIN		GPIO_PE(9)//GPIO_PB(24)
 
 /**
 * Ethernet AX88796C
 **/
#define AX_ETH_INT		GPIO_PF(14)	/*pf18*/
#define AX_ETH_RESET	GPIO_PF(15)		/*pf21*/

/**
 * keyboard gpio interface
 **/
#define GPIO_OTG_SEL			GPIO_PD(17)
#define ACTIVE_LOW_OTG_SEL		0

#define GPIO_ENDCALL			GPIO_PA(30)
#define ACTIVE_LOW_ENDCALL		1

#define GPIO_MENU			GPIO_PD(17)
#define ACTIVE_LOW_MENU		0

#define GPIO_SP0838_EN			GPIO_PB(18)
#define GPIO_SP0838_RST			GPIO_PB(26)

/**
 * pmem information
 **/
#define JZ_PMEM_CAMERA_BASE	0x5f000000
#define JZ_PMEM_CAMERA_SIZE	0x01000000

/**
 * motor gpio
 */
#define GPIO_MOTOR_PIN			GPIO_PB(25) /* PB25 */


// #define GPIO_HOME
// #define ACTIVE_LOW_HOME			1
// #define GPIO_CALL
// #define ACTIVE_LOW_CALL			1
#endif /* __HLBA1_H__ */
