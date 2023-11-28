#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>
#include <linux/jz_dwc.h>

/* ****************************GPIO I2C START******************************** */
#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
#define GPIO_I2C0_SDA GPIO_PA(12)
#define GPIO_I2C0_SCK GPIO_PA(13)
#endif

#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
#define GPIO_I2C1_SDA GPIO_PB(25)
#define GPIO_I2C1_SCK GPIO_PB(26)
#endif
/* ****************************GPIO I2C END********************************** */

/* ****************************GPIO SPI START******************************** */
#ifndef CONFIG_SPI_GPIO
#define GPIO_SPI_SCK  GPIO_PC(15)
#define GPIO_SPI_MOSI GPIO_PC(11)
#define GPIO_SPI_MISO GPIO_PC(12)
#endif
/* ****************************GPIO SPI END********************************** */

/* ****************************GPIO KEY START******************************** */
//#define GPIO_POWER_KEY			GPIO_PA(30)
//#define ACTIVE_LOW_POWER		1

#define GPIO_WAKEUP			GPIO_PA(30)
#define ACTIVE_LOW_WAKEUP		1
/* ****************************GPIO KEY END********************************** */

/* ****************************GPIO MMC START******************************** */
#define GPIO_MMC_RST_N			GPIO_PA(29)
#define GPIO_MMC_RST_N_LEVEL	LOW_ENABLE
/* ****************************GPIO MMC END******************************** */

/* ****************************GPIO USB START******************************** */
#define GPIO_USB_ID			-1
#define GPIO_USB_ID_LEVEL		LOW_ENABLE
#define GPIO_USB_DETE			-1
#define GPIO_USB_DETE_LEVEL		LOW_ENABLE
#define GPIO_USB_DRVVBUS		GPIO_PC(13)
#define GPIO_USB_DRVVBUS_LEVEL		HIGH_ENABLE
/* ****************************GPIO USB END********************************** */

/* ****************************GPIO DVP CAMERA START***************************** */
#define DVP_CAMERA_RESET			GPIO_PA(18)
#define DVP_CAMERA_PWDN_N           GPIO_PA(19)
/* ****************************GPIO CAMERA END******************************* */

/* ****************************GPIO AUDIO START****************************** */
#define GPIO_HP_MUTE		-1	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL	-1	/*vaild level*/

#define GPIO_SPEAKER_EN	   GPIO_PB(31)	      /*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL	1

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
#define WL_REG_EN	GPIO_PC(16)
#define WL_MMC_NUM	1 //sdio use MMC1

#define WLAN_PWR_EN	(-1)
#define BCM_PWR_EN	(-1)
#define PWM_32K_OUTPUT 1//1:enable 32k output -1:disable 32k
/* ****************************GPIO WIFI END********************************* */

/* ****************************GPIO BLUETOOTH START************************** */
/* BT gpio */
#define HOST_WAKE_BT	GPIO_PB(25)
#define BT_WAKE_HOST	-1
#define BT_REG_EN	GPIO_PB(26)
#define BT_UART_RTS	GPIO_PB(21)
#define BLUETOOTH_UPORT_NAME  "ttyS0"
/* ****************************GPIO BLUETOOTH END**************************** */

#endif /* __BOARD_H__ */
