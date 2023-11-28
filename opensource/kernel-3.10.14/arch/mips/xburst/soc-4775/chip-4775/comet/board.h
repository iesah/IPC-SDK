#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>


#ifndef CONFIG_BOARD_NAME
#define CONFIG_BOARD_NAME "comet"
#endif

/* MSC GPIO Definition */
#define GPIO_SD0_VCC_EN_N	GPIO_PB(3)
#define GPIO_SD0_CD_N		GPIO_PB(2)

extern struct jzmmc_platform_data inand_pdata;
extern struct jzmmc_platform_data tf_pdata;
extern struct jzmmc_platform_data sdio_pdata;

#ifdef CONFIG_FB_JZ4780_LCDC0
extern struct jzfb_platform_data jzfb0_pdata;
#endif
#ifdef CONFIG_FB_JZ4780_LCDC1
extern struct jzfb_platform_data jzfb1_pdata;
#endif
/* panel and bl platform device */
#ifdef CONFIG_LCD_KFM701A21_1A
extern struct platform_device kfm701a21_1a_device;
#endif

/**
 * lcd gpio
 **/
#define GPIO_LCD_PWM		GPIO_PE(1)
#define GPIO_LCD_DISP		GPIO_PB(30)

/**
 * TP gpio
 **/
#define GPIO_TP_WAKE		GPIO_PB(28)
#define GPIO_TP_INT		GPIO_PB(29)

#define GPIO_HP_MUTE		-1	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL	-1		/*vaild level*/

#define GPIO_SPEAKER_EN		GPIO_PD(15)/*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL	1

#define GPIO_HANDSET_EN		-1		/*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL   -1

#define	GPIO_HP_DETECT		-1	/*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL    -1	
#define GPIO_MIC_SELECT		-1		/*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL	-1		/*builin mic select level*/
#define GPIO_MIC_DETECT		-1
#define GPIO_MIC_INSERT_LEVEL   -1
#define GPIO_MIC_DETECT_EN		-1  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL	-1 /*mic detect enable gpio*/

/**
 * CIM gpio
 **/
#define GPIO_OV3640_EN		GPIO_PB(6)
#define GPIO_OV3640_RST		GPIO_PA(27)

/**
 * KEY gpio
 **/
#define GPIO_HOME		GPIO_PG(15)
#define GPIO_BACK		GPIO_PD(19)
#define GPIO_VOLUMEDOWN		GPIO_PD(17)
#define GPIO_VOLUMEUP		GPIO_PD(18)
#define GPIO_ENDCALL            GPIO_PA(30)

#define ACTIVE_LOW_HOME		1
#define ACTIVE_LOW_BACK		1
#define ACTIVE_LOW_ENDCALL      1

#if defined(CONFIG_NAND)
#define ACTIVE_LOW_VOLUMEDOWN	0
#define ACTIVE_LOW_VOLUMEUP	1
#else
#define ACTIVE_LOW_VOLUMEDOWN	1
#define ACTIVE_LOW_VOLUMEUP	0
#endif

/**
 * USB detect pin
 **/
#define GPIO_USB_DETE                   -1

/**
 * pmem information
 **/
/* auto allocate pmem in arch_mem_init(), do not assigned base addr, just set 0 */
#define JZ_PMEM_ADSP_BASE   0x0        // 0x5e000000
#define JZ_PMEM_ADSP_SIZE   0x02000000

/**
 * sound platform data
 **/
extern struct snd_codec_data codec_data;

extern struct platform_device backlight_device;

#ifdef CONFIG_LCD_BYD_BM8766U
extern struct platform_device byd_bm8766u_device;
#endif

#ifdef CONFIG_BCM4330_RFKILL
extern struct platform_device bcm4330_bt_power_device;
#endif

#ifdef CONFIG_LCD_KD50G2_40NM_A2
extern struct platform_device kd50g2_40nm_a2_device;
#endif

/**
 * Digital pulse backlight
 **/
#ifdef CONFIG_BACKLIGHT_DIGITAL_PULSE
extern struct platform_device digital_pulse_backlight_device;
extern struct platform_digital_pulse_backlight_data bl_data;
#endif

#endif /* __BOARD_H__ */
