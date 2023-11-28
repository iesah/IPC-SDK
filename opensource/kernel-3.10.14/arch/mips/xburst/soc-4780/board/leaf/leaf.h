#ifndef __LEAF_H__
#define __LEAF_H__
#include <gpio.h>

/*
bcm4330 bluetooth
*/
#ifdef CONFIG_BCM4330_RFKILL
extern struct platform_device bcm4330_bt_power_device;
#endif

/**
 * mmc platform data
 **/
extern struct jzmmc_platform_data leaf_inand_pdata;
extern struct jzmmc_platform_data leaf_tf_pdata;
extern struct jzmmc_platform_data leaf_sdio_pdata;

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
#ifdef CONFIG_LCD_KR080LA4S_250
extern struct platform_device kr080la4s_250_device;
#endif
#ifdef CONFIG_LCD_CRD080TI01_40NM01
extern struct platform_device crd080ti01_40nm01_device;
#endif
extern struct platform_device jzlcd_device;
extern struct platform_device leaf_backlight_device;

#ifdef CONFIG_JZ_MODEM
extern struct platform_device jz_modem_device;
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

#define	GPIO_HP_DETECT		GPIO_PE(7)		/*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL	0

#define GPIO_MIC_DETECT		GPIO_PD(15)		/*mic detect gpio*/
#define GPIO_MIC_INSERT_LEVEL	0

#define GPIO_MIC_SELECT		GPIO_PB(25)		/*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL	1			/*headset mic select level*/

#define GPIO_HANDSET_EN		  -1		/*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL -1

#define GPIO_MIC_DETECT_EN		GPIO_PB(25)  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL	0		/*mic detect enable gpio*/

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
 * keyboard gpio interface
 **/
#define GPIO_BACK			GPIO_PB(5)
#define ACTIVE_LOW_BACK			1

//#define GPIO_MENU			GPIO_PB(4)
//#define ACTIVE_LOW_MENU			1

#define GPIO_ENDCALL			GPIO_PA(30)
#define ACTIVE_LOW_ENDCALL		1

#ifndef CONFIG_NAND_DRIVER
#define GPIO_VOLUMEUP			GPIO_PD(18)
#define ACTIVE_LOW_VOLUMEUP		0

#define GPIO_VOLUMEDOWN			GPIO_PD(17)
#define ACTIVE_LOW_VOLUMEDOWN		1
#else
#define GPIO_VOLUMEUP			GPIO_PD(17)
#define ACTIVE_LOW_VOLUMEUP		0

#define GPIO_VOLUMEDOWN			GPIO_PD(18)
#define ACTIVE_LOW_VOLUMEDOWN		1
#endif

#define GPIO_GC0308_EN			GPIO_PB(18)
#define GPIO_GC0308_RST         GPIO_PB(26)

#define GPIO_GC2015_EN			GPIO_PB(19)
#define GPIO_GC2015_RST			GPIO_PB(26)

/**
 * USB detect pin
 **/
#define GPIO_USB_DETE			GPIO_PF(13)

/**
 * pmem information
 **/
#define JZ_PMEM_CAMERA_BASE	0x5f000000
#define JZ_PMEM_CAMERA_SIZE	0x01000000

/*
 * modem gpio
 */
#if 0
#define GPIO_BP_PWR			GPIO_PE(8)
#define GPIO_BP_PWR_LEVEL		1
#define GPIO_BP_ONOFF			GPIO_PD(12)
#define GPIO_BP_ONOFF_LEVEL		1
#define GPIO_BP_WAKE_AP			GPIO_PB(21)
#define GPIO_BP_WAKE_AP_LEVEL		0
#define GPIO_BP_STATUS			GPIO_PB(31)
#define GPIO_BP_STATUS_LEVEL		0
#define GPIO_AP_WAKE_BP			GPIO_PB(28)
#define GPIO_AP_WAKE_BP_LEVEL		0
#define GPIO_AP_STATUS			GPIO_PB(30)
#define GPIO_AP_STATUS_LEVEL		0
#endif

/**
 * motor gpio
 */
#define GPIO_MOTOR_PIN			GPIO_PB(24) /* PB24 */

/**
 * USB detect pin
 **/

// #define GPIO_HOME
// #define ACTIVE_LOW_HOME			1
// #define GPIO_CALL
// #define ACTIVE_LOW_CALL			1
#endif /* __LEAF_H__ */
