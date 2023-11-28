#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>

/* lcd pdata and display panel */
#ifdef CONFIG_FB_JZ_V12
extern struct jzfb_platform_data jzfb_pdata;
#endif
#ifdef CONFIG_JZ_MIPI_DSI
extern struct jzdsi_platform_data jzdsi_pdata;
#endif
#ifdef CONFIG_LCD_KFM701A21_1A
extern struct platform_device kfm701a21_1a_device;
#endif
#ifdef CONFIG_LCD_CV90_M5377_P30
extern struct platform_device cv90_m5377_p30_device;
#endif
#ifdef CONFIG_LCD_BYD_BM8766U
extern struct platform_device byd_bm8766u_device;
#endif
#ifdef CONFIG_LCD_TRULY_TFT240240_2_E
extern struct platform_device truly_tft240240_device;
#endif

/* pmu d2041 or 9024 gpio def*/
#define GPIO_PMU_IRQ			GPIO_PA(3)
#define GPIO_GSENSOR_INT1       GPIO_PA(15)

/* about lcdc gpio */
#define GPIO_LCD_PWM		GPIO_PE(1)
#define GPIO_LCD_DISP		GPIO_PB(30)
#define GPIO_LCD_RST		GPIO_PB(12)
#define GPIO_LCD_CS		GPIO_PC(9)
#define GPIO_LCD_RD		GPIO_PC(8)
#define GPIO_BL_PWR_EN		GPIO_PD(26)
#define GPIO_MIPI_IF_SEL	GPIO_PC(1)
#define GPIO_MIPI_RST		GPIO_PE(29)
#define GPIO_LCD_B_SYNC		GPIO_PE(20)
#define GPIO_LCD_SPI_CLK	GPIO_PB(10)
#define GPIO_LCD_SPI_CS		GPIO_PB(11)
#define GPIO_LCD_SPI_DT		GPIO_PB(16)
#define GPIO_LCD_SPI_DR		GPIO_PB(17)
#define GPIO_LCD_EXCLKO		GPIO_PD(15)

/* Digital pulse backlight*/
#ifdef CONFIG_BACKLIGHT_DIGITAL_PULSE
extern struct platform_device digital_pulse_backlight_device;
extern struct platform_digital_pulse_backlight_data bl_data;
#endif

#ifndef CONFIG_BOARD_NAME
#define CONFIG_BOARD_NAME "jz4785_fpga"
#endif


extern struct jzmmc_platform_data inand_pdata;
extern struct jzmmc_platform_data tf_pdata;
extern struct jzmmc_platform_data sdio_pdata;

/**
 * TP gpio
 **/
#define GPIO_TP_WAKE		GPIO_PE(1)
#define GPIO_TP_INT		GPIO_PE(2)

#define GPIO_HP_MUTE		-1	/*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL		-1		/*vaild level*/

#define GPIO_SPEAKER_EN			-1/*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL	-1

#define GPIO_HANDSET_EN		  -1		/*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL -1

#define	GPIO_HP_DETECT	-1		/*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL    1
#define GPIO_MIC_SELECT		-1		/*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL	-1		/*builin mic select level*/
#define GPIO_MIC_DETECT		-1
#define GPIO_MIC_INSERT_LEVEL -1
#define GPIO_MIC_DETECT_EN		-1  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL	-1 /*mic detect enable gpio*/

extern struct ovisp_camera_platform_data ovisp_camera_info;

/**
 * sound platform data
 **/
extern struct snd_codec_data codec_data;
#endif /* __BOARD_H__ */
