#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>
#include <linux/jz_dwc.h>

#include "pmu.h"

/* ****************************GPIO SLEEP START******************************* */
#define GPIO_REGULATOR_SLP	GPIO_PB(1)
#define GPIO_OUTPUT_TYPE	GPIO_OUTPUT1
/* ****************************GPIO SLEEP END******************************** */

/* ****************************GPIO LCD START******************************** */
#ifdef CONFIG_LCD_BYD_8991FTGF
#define GPIO_LCD_DISP		GPIO_PB(0)
#define GPIO_LCD_DE		0
#define GPIO_LCD_VSYNC		0
#define GPIO_LCD_HSYNC		0
#define GPIO_LCD_CS		GPIO_PA(11)
#define GPIO_LCD_CLK	        GPIO_PD(28)
#define GPIO_LCD_SDO		GPIO_PE(3)
#define GPIO_LCD_SDI		GPIO_PE(0)
#define GPIO_LCD_BACK_SEL	GPIO_PC(20)
#endif
#ifdef CONFIG_LCD_LH155
#define GPIO_LCD_BLK            GPIO_PC(17)
#define GPIO_LCD_RST            GPIO_PA(12)
#endif
#ifdef CONFIG_LCD_TRULY_TDO_HD0499K
#define GPIO_MIPI_RST_N         GPIO_PA(12)
#define GPIO_MIPI_PWR           GPIO_PC(17)
#endif
#ifdef CONFIG_LCD_BYD_9177AA
#define GPIO_MIPI_RST_N         GPIO_PC(3)
#define GPIO_MIPI_PWR           GPIO_PC(2)
#endif
#ifdef CONFIG_BACKLIGHT_PWM
#define GPIO_LCD_PWM		GPIO_PE(1)
#endif
#ifdef CONFIG_BACKLIGHT_DIGITAL_PULSE
#define GPIO_GIGITAL_PULSE      GPIO_PE(1)
#endif
#ifdef CONFIG_LCD_CV90_M5377_P30
#define GPIO_LCD_BLK		GPIO_PC(17)
#define GPIO_LCD_RST            GPIO_PA(12)
#define GPIO_LCD_NRD_E		GPIO_PC(8)
#define GPIO_LCD_NWR_SCL	GPIO_PC(25)
#define GPIO_LCD_DNC		GPIO_PC(26)
#define SLCD_NBUSY_PIN		GPIO_PA(11)
#endif
#ifdef CONFIG_JZ_EPD_V12
#define GPIO_EPD_PWR0           GPIO_PC(22)
#define GPIO_EPD_PWR1           GPIO_PC(23)
#define GPIO_EPD_PWR2           GPIO_PC(24)
#define GPIO_EPD_PWR3           GPIO_PC(25)
#define GPIO_EPD_EN             GPIO_PC(23)
#define GPIO_EPD_ENOP           GPIO_PC(24)
#endif
/* ****************************GPIO LCD END********************************** */

/* ****************************GPIO I2C START******************************** */
#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
#define GPIO_I2C0_SDA GPIO_PD(30)
#define GPIO_I2C0_SCK GPIO_PD(31)
#endif
#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
#define GPIO_I2C1_SDA GPIO_PE(30)
#define GPIO_I2C1_SCK GPIO_PE(31)
#endif
#ifdef CONFIG_SOFT_I2C2_GPIO_V12_JZ
#define GPIO_I2C2_SDA GPIO_PA(12)
#define GPIO_I2C2_SCK GPIO_PA(13)
//#define GPIO_I2C2_SDA GPIO_PE(00)
//#define GPIO_I2C2_SCK GPIO_PE(03)
#endif
#ifdef CONFIG_SOFT_I2C3_GPIO_V12_JZ
#define GPIO_I2C3_SDA GPIO_PB(7)
#define GPIO_I2C3_SCK GPIO_PB(8)
#endif
/* ****************************GPIO I2C END********************************** */

/* ****************************GPIO SPI START******************************** */
#ifndef CONFIG_SPI_GPIO
#define GPIO_SPI_SCK  GPIO_PE(15)
#define GPIO_SPI_MOSI GPIO_PE(17)
#define GPIO_SPI_MISO GPIO_PE(14)
#endif
/* ****************************GPIO SPI END********************************** */

/* ****************************GPIO TOUCHSCREEN START************************ */
#ifdef CONFIG_TOUCHSCREEN_GWTC9XXXB
#define GPIO_TP_INT		GPIO_PB(0)
#define GPIO_TP_WAKE		GPIO_PE(10)
#endif
#ifdef CONFIG_TOUCHSCREEN_FT6X0X
#define GPIO_TP_INT		GPIO_PB(0)
#define GPIO_TP_WAKE		GPIO_PE(10)
#endif
#if defined(CONFIG_TOUCHSCREEN_FT6X06) || defined(CONFIG_TOUCHSCREEN_FT6X06_MODULE)
#define GPIO_TP_INT		GPIO_PD(17)
#define GPIO_TP_RESET		GPIO_PD(27)
#endif
#if defined(CONFIG_TOUCHSCREEN_FT5336) || defined(CONFIG_TOUCHSCREEN_FT5336_MODULE)
#define GPIO_TP_INT		GPIO_PD(17)
#define GPIO_TP_RESET		GPIO_PD(27)
#endif
/* ****************************GPIO TOUCHSCREEN END************************** */

/* ****************************GPIO KEY START******************************** */
/* #define GPIO_HOME_KEY		GPIO_PD(18) */
/* #define ACTIVE_LOW_HOME		1 */

#define GPIO_VOLUMEDOWN_KEY         GPIO_PD(18)
#define ACTIVE_LOW_VOLUMEDOWN	0

#define GPIO_ENDCALL_KEY            GPIO_PA(30)
#define ACTIVE_LOW_ENDCALL      1

/* ****************************GPIO KEY END********************************** */

/* ****************************GPIO PMU START******************************** */
/* PMU ricoh619 */
#ifdef CONFIG_REGULATOR_RICOH619
#define PMU_IRQ_N		GPIO_PA(3)
#endif /* CONFIG_REGULATOR_RICOH619 */

/* pmu d2041 or 9024 gpio def*/
#ifdef CONFIG_REGULATOR_DA9024
#define GPIO_PMU_IRQ		GPIO_PA(3)
#endif
/* ****************************GPIO PMU END********************************** */

/******************************GPIO PCA9539 START***********************************/
/*GPIO PCA9593*/
#ifdef CONFIG_GPIO_PCA953X
#define PCA9539_IRQ_N       GPIO_PD(19)
#define PCA9539_RST_N       GPIO_PA(14)
#define PCA9539_GPIO_BASE   177
#define PCA9539_EXT_GPIO(x)   (PCA9539_GPIO_BASE + (x))
#endif /* CONFIG_PCA9539 */
/* ****************************GPIO PCA9539 END********************************** */

/* ****************************GPIO GSENSOR START**************************** */
#define GPIO_GSENSOR_INT     GPIO_PA(15)
/* ****************************GPIO GSENSOR END****************************** */

/* ****************************GPIO EFUSE START****************************** */
#define GPIO_EFUSE_VDDQ      PCA9539_EXT_GPIO(6)
/* ****************************GPIO EFUSE END******************************** */

/* ****************************GPIO LI ION START***************************** */
#define GPIO_LI_ION_CHARGE   GPIO_PB(1)
#define GPIO_LI_ION_AC       GPIO_PA(13)
#define GPIO_ACTIVE_LOW      1
/* ****************************GPIO LI ION END******************************* */

/* ****************************GPIO MMC START******************************** */
#define GPIO_MMC_RST_N			GPIO_PA(29)
#define GPIO_MMC_RST_N_LEVEL	LOW_ENABLE
/* ****************************GPIO MMC END******************************** */

/* ****************************GPIO USB START******************************** */
#define GPIO_USB_ID     PCA9539_EXT_GPIO(7)/*GPIO_PA(13)*/
#define GPIO_USB_ID_LEVEL		LOW_ENABLE
#define GPIO_USB_DETE                   PCA9539_EXT_GPIO(8) /*GPIO_PA(14)*/
#define GPIO_USB_DETE_LEVEL		HIGH_ENABLE
#define GPIO_USB_DRVVBUS		GPIO_PE(10)
#define GPIO_USB_DRVVBUS_LEVEL		HIGH_ENABLE
/* ****************************GPIO USB END********************************** */

/* ****************************GPIO CAMERA START***************************** */
#define CAMERA_RST                     PCA9539_EXT_GPIO(4)
#define CAMERA_PWDN_N           PCA9539_EXT_GPIO(3)//GPIO_PA(13) /* pin conflict with USB_ID */
#define CAMERA_MCLK		GPIO_PE(2) /* no use */
#ifdef CONFIG_DVP_OV9712
#define OV9712_POWER	 	GPIO_PC(2) //the power of camera board
#define OV9712_RST		GPIO_PA(11)
#define OV9712_PWDN_EN		GPIO_PD(28)
#endif
/* ****************************GPIO CAMERA END******************************* */

/* ****************************GPIO AUDIO START****************************** */
#define GPIO_HP_MUTE		-1	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL	-1	/*vaild level*/

#define GPIO_SPEAKER_EN		-1      /*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL	-1

#define GPIO_HANDSET_EN		-1	/*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL   -1

#define	GPIO_HP_DETECT	-1		/*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL    1
#define GPIO_MIC_SELECT		-1	/*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL	-1	/*builin mic select level*/
#define GPIO_MIC_DETECT		-1
#define GPIO_MIC_INSERT_LEVEL   -1
#define GPIO_MIC_DETECT_EN	-1  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL -1 /*mic detect enable gpio*/

#define HP_SENSE_ACTIVE_LEVEL	1
#define HOOK_ACTIVE_LEVEL		-1
/* ****************************GPIO AUDIO END******************************** */

/* ****************************GPIO WIFI START******************************* */
#define HOST_WAKE_WL	GPIO_PA(10)
#define WL_WAKE_HOST	GPIO_PA(9)
#define WL_REG_EN	GPIO_PA(8)
#if 0
#define GPIO_WLAN_REG_ON	GPIO_PG(7)
#define GPIO_WLAN_INT	        GPIO_PG(8)
#define GPIO_WLAN_WAKE	        GPIO_PB(28)
#define GPIO_WIFI_RST_N     GPIO_PB(20)
#endif

#define WLAN_PWR_EN	(-1)
//#define WLAN_PWR_EN	GPIO_PE(3)
/* ****************************GPIO WIFI END********************************* */

/* ****************************GPIO NFC START******************************** */
/*
 * For BCM2079X NFC
 */
#define NFC_REQ         PCA9539_EXT_GPIO(1)//GPIO_PC(26)
#define NFC_REG_PU      PCA9539_EXT_GPIO(0)//GPIO_PC(27)
#define HOST_WAKE_NFC   PCA9539_EXT_GPIO(2)//GPIO_PA(11)
/* ****************************GPIO NFC END********************************** */

/* ****************************GPIO BLUETOOTH START************************** */
/* BT gpio */
#define HOST_WAKE_BT	GPIO_PA(1)
#define BT_WAKE_HOST	GPIO_PA(0)
#define BT_REG_EN	GPIO_PA(2)
#define BT_UART_RTS	GPIO_PF(2)
#define BLUETOOTH_UPORT_NAME  "ttyS0"
#if 0
#define BLUETOOTH_UPORT_NAME  "ttyS1"
#define BLUETOOTH_UPORT_NAME  "ttyS2"
#define BLUETOOTH_UPORT_NAME  "ttyS3"
#endif
#if 0
#define GPIO_BT_REG_ON      GPIO_PB(30)
#define GPIO_BT_WAKE        GPIO_PB(20)
#define GPIO_BT_INT    	    GPIO_PB(31)
//#define GPIO_BT_RST_N       GPIO_PB(28)
#define GPIO_BT_UART_RTS    GPIO_PF(2)
#define GPIO_PB_FLGREG      (0x10010158)
#define GPIO_BT_INT_BIT	    (1 << (GPIO_BT_INT % 32))
#endif
/* ****************************GPIO BLUETOOTH END**************************** */

#endif /* __BOARD_H__ */
