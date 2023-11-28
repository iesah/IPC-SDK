#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>
#include <linux/jz_dwc.h>

#include "pmu.h"

/* ****************************GPIO I2C START******************************** */
#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
/* #define GPIO_I2C0_SDA GPIO_PF(8) */
/* #define GPIO_I2C0_SCK GPIO_PF(9) */
#define GPIO_I2C0_SDA GPIO_PF(31)
#define GPIO_I2C0_SCK GPIO_PF(30)
#endif
#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
#define GPIO_I2C1_SDA GPIO_PE(9)
#define GPIO_I2C1_SCK GPIO_PE(10)
#endif
/* ****************************GPIO I2C END********************************** */

/* ****************************GPIO SPI START******************************** */
#ifndef CONFIG_SPI_GPIO
#define GPIO_SPI_SCK  GPIO_PA(18)
#define GPIO_SPI_MOSI GPIO_PA(1)
#define GPIO_SPI_MISO GPIO_PA(2)
#endif
/* ****************************GPIO SPI END********************************** */

/* ****************************GPIO KEY START******************************** */
//#define GPIO_POWER_KEY			GPIO_PA(30)
//#define ACTIVE_LOW_POWER		1

#define GPIO_WAKEUP			GPIO_PA(30)
#define ACTIVE_LOW_WAKEUP		1
/* ****************************GPIO KEY END********************************** */

/******************************GPIO PCA9539 START***********************************/
/*GPIO PCA9593*/
#ifdef CONFIG_GPIO_PCA953X
#define PCA9539_IRQ_N       GPIO_PD(19)
#define PCA9539_RST_N       GPIO_PA(14)
#define PCA9539_GPIO_BASE   177
#define PCA9539_EXT_GPIO(x)   (PCA9539_GPIO_BASE + (x))
#endif /* CONFIG_PCA9539 */
/* ****************************GPIO PCA9539 END********************************** */

/* ****************************GPIO EFUSE START****************************** */
#define GPIO_EFUSE_VDDQ      PCA9539_EXT_GPIO(6)
/* ****************************GPIO EFUSE END******************************** */

/* ****************************GPIO USB START******************************** */
#define GPIO_USB_ID			GPIO_PB(23)
#define GPIO_USB_ID_LEVEL		LOW_ENABLE
#define GPIO_USB_DETE			GPIO_PB(22)
#define GPIO_USB_DETE_LEVEL		LOW_ENABLE
#define GPIO_USB_DRVVBUS		GPIO_PE(11)
#define GPIO_USB_DRVVBUS_LEVEL		HIGH_ENABLE
/* ****************************GPIO USB END********************************** */

/* ****************************GPIO CAMERA START***************************** */
#define CAMERA_RST		       PCA9539_EXT_GPIO(4)
#define CAMERA_PWDN_N           PCA9539_EXT_GPIO(3)//GPIO_PA(13) /* pin conflict with USB_ID */
#define CAMERA_MCLK		GPIO_PE(2) /* no use */
/* ****************************GPIO CAMERA END******************************* */

/* ****************************GPIO AUDIO START****************************** */
#define GPIO_HP_MUTE		-1	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL	-1	/*vaild level*/

#define GPIO_SPEAKER_EN	   -1	      /*speaker enable gpio*/
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

/* ****************************GPIO GMAC START******************************* */
#ifdef CONFIG_JZ_MAC
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
#define GMAC_PHY_PORT_GPIO GPIO_PE(11)
#define GMAC_PHY_PORT_START_FUNC GPIO_OUTPUT0
#define GMAC_PHY_PORT_END_FUNC GPIO_OUTPUT1
#define GMAC_PHY_DELAYTIME 100000
#endif
#else /* CONFIG_MDIO_GPIO */
#define MDIO_MDIO_MDC_GPIO GPIO_PF(13)
#define MDIO_MDIO_GPIO GPIO_PF(14)
#endif
#endif /* CONFIG_JZ4775_MAC */
/* ****************************GPIO GMAC END********************************* */

/* ****************************GPIO WIFI START******************************* */
#define WL_WAKE_HOST	GPIO_PB(28)
#define WL_REG_EN	GPIO_PB(29)
#define WLAN_VCC_EN_N GPIO_PA(23)
#define WLAN_CLK32K_EN GPIO_PF(3)
#define WL_MMC_NUM	2 //sdio use MMC2

#define WLAN_PWR_EN	(-1)
#define BCM_PWR_EN	(-1)
/* ****************************GPIO WIFI END********************************* */

#endif /* __BOARD_H__ */
