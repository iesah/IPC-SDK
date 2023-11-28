#ifndef __M80_H__
#define __M80_H__
#include <gpio.h>

/**
 * mmc platform data
 **/
extern struct jzmmc_platform_data ji8070a_inand_pdata;
extern struct jzmmc_platform_data ji8070a_tf_pdata;
extern struct jzmmc_platform_data ji8070a_sdio_pdata;

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
#ifdef CONFIG_LCD_HSD101PWW1
extern struct platform_device hsd101pww1_device;
#endif
#ifdef CONFIG_LCD_EK070TN93
extern struct platform_device ek070tn93_device;
#endif
extern struct platform_device ji8070a_backlight_device;

#ifdef CONFIG_LCD_HSD070IDW1
extern struct platform_device android_bl_device;
#endif

#ifdef CONFIG_LCD_HHX070ML208CP21
extern struct platform_device hhx070ml208cp21_device;
#endif

/**
 * sound platform data
 **/
extern struct snd_codec_data codec_data;

/**
 * audio gpio
 **/
#define GPIO_HP_MUTE		GPIO_PD(13)	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL	1		/*vaild level*/

#define GPIO_SPEAKER_EN		GPIO_PE(6)	/*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL	1

#define	GPIO_HP_DETECT		GPIO_PE(7)	/*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL	0
//////#define	GPIO_HP_DETECT		-1		/*hp detect gpio*/
//#define GPIO_HP_INSERT_LEVEL	-1

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

#define GPIO_CTP_BAT_IRQ        GPIO_PB(31)
#define GPIO_CTP_BAT_WAKE_UP    GPIO_PB(30)
/**
 * g sensor gpio interface
 **/
#define GPIO_MMA8452_INT1		GPIO_PF(9)
#define GPIO_LIS3DH_INT1		GPIO_PF(9)

/**
 * keyboard gpio interface
 **/

#define GPIO_ENDCALL			GPIO_PA(30)
#define ACTIVE_LOW_ENDCALL		1

#define GPIO_VOLUMEUP			GPIO_PD(17)
#define ACTIVE_LOW_VOLUMEUP		0

#define GPIO_VOLUMEDOWN			GPIO_PD(18)
#define ACTIVE_LOW_VOLUMEDOWN	1
#define GPIO_SP0838_EN			GPIO_PB(18)
#define GPIO_SP0838_RST			GPIO_PB(26)

#ifdef CONFIG_GC0308_Q8
#define GPIO_GC0308_EN_F		GPIO_PB(19)
#define GPIO_GC0308_EN_B		GPIO_PB(18)
#define GPIO_GC0308_RST			GPIO_PB(26)
#endif

#if defined(CONFIG_GC0308)
#define GPIO_GC0308_EN			GPIO_PB(18)
#define GPIO_GC0308_RST			GPIO_PB(26)
#endif


#ifdef	CONFIG_GC2015
#define GPIO_GC2015_EN			GPIO_PB(19)
#define GPIO_GC2015_RST			GPIO_PB(26)
#endif

/**
 * camera sensor interface
 **/

#ifdef CONFIG_VIDEO_JZ4780_CIM_HOST

#define	CAMERA_SENSOR_RESET		GPIO_PB(26)

#ifndef CONFIG_EXCHANGE_SENSOR_EN_PIN
#define CAMERA_FRONT_SENSOR_EN	GPIO_PB(19)
#define CAMERA_BACK_SENSOR_EN	GPIO_PB(18)
#else
#define CAMERA_FRONT_SENSOR_EN	GPIO_PB(18)
#define CAMERA_BACK_SENSOR_EN	GPIO_PB(19)
#endif

#endif

#define FRONT_CAMERA_INDEX	0
#define BACK_CAMERA_INDEX	1

/**
 * motor gpio
 */
#define GPIO_MOTOR_PIN			GPIO_PB(25) /* PB25 */

/**
 * usb gpio
 */
#define GPIO_USB_ID                     GPIO_PE(2)
#define GPIO_DRVVBUS                    GPIO_PE(10)

/**
 * hdmi gpio
 */
#define GPIO_HDMI_CEC			GPIO_PF(23)
#define GPIO_HDMI_DETE			GPIO_PE(1)

/**
 * USB detect pin
 **/
#define GPIO_USB_DETE			GPIO_PF(13)

/**
 * pmem information
 **/
#define JZ_PMEM_CAMERA_BASE	0x5f000000
#define JZ_PMEM_CAMERA_SIZE	0x01000000


// #define GPIO_HOME
// #define ACTIVE_LOW_HOME			1
// #define GPIO_CALL
// #define ACTIVE_LOW_CALL			1
#endif /* __M80_H__ */
