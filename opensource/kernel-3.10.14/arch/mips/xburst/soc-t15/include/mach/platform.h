/*
 *  Copyright (C) 2010 Ingenic Semiconductor Inc.
 *
 *   In this file, here are some macro/device/function to
 * to help the board special file to organize resources
 * on the chip.
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/* devio define list */

#define I2S_PORTDE							\
	{ .name = "i2s-sysclk",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x1<<8, },	\
	{ .name = "i2s-data",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3<<6, },	\
	{ .name = "i2s-bitclk",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x1<<4,},	\
	{ .name = "i2s-sync",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x1<<5,}
	/*{ .name = "i2s-sync1",		.port = GPIO_PORT_F, .func = GPIO_FUNC_2, .pins = 0x1<<3,}*/

#define PCM_PORTF							\
	{ .name = "pcm",			.port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0xf << 0}
#define DMIC_PORTF							\
	{ .name = "dmic0-sysclk",	.port = GPIO_PORT_F, .func = GPIO_FUNC_3, .pins = 0x3<<6}
#define DMIC_PORTE                           \
	{ .name = "dmic1-sysclk",	.port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x1<<10}
/*******************************************************************************************************************/
#define UART0_PORTF							\
	{ .name = "uart0", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x09, }
#define UART0_PORTC							\
	{ .name = "uart0", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 0x1<<11 | 0x1<<0, }
#define UART1_PORTF							\
	{ .name = "uart1", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x3 << 4, }
#define UART2_PORTC							\
	{ .name = "uart2", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 0x3 << 20, }
#define UART2_PORTF							\
	{ .name = "uart2", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x3<<6, }
#define UART3_PORTB						\
	{ .name = "uart3", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x3 << 2, }
#define UART3_PORTE						\
	{ .name = "uart3", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3 << 16, }
/*******************************************************************************************************************/

#define MSC0_PORTA_4BIT							\
	{ .name = "msc0-pa-4bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = (0x3<<18|0xf<<0), }
#define MSC0_PORTA_8BIT							\
	{ .name = "msc0-pa-8bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = (0x3<<18|0xff<<0), }
#define MSC0_PORTE							\
	{ .name = "msc0-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x30f00000, }
#define MSC1_PORTB							\
	{ .name = "msc1-pb",		.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3c300000, }
#define MSC1_PORTE							\
	{ .name = "msc1-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x30f00000, }
#define MSC2_PORTB_FUNC1							\
	{ .name = "msc2-pb",		.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = (0xf<<9|0x3<<6), }
#define MSC2_PORTB_4BIT							\
	{ .name = "msc2-pb",		.port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x0000003f, }

/*******************************************************************************************************************/
/*****************************************************************************************************************/

#define SSI0_PORTA							\
	{ .name = "ssi0-pa",	       .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 0xf<<1 | 0x3<<18, }
#define SSI0_PORTBE							\
	{ .name = "ssi0-pb",	       .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0xf<<18, }, \
	{ .name = "ssi0-pe",	       .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x3<<9, }
#define SSI1_PORTB							\
	{ .name = "ssi1",				.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x0000003f, }
/* SFC : the function of hold and reset not use, hardware pull up */
#define SFC_PORTA							\
	{ .name = "sfc",		.port = GPIO_PORT_A, .func = GPIO_FUNC_3, .pins = (0x3 < 1) | (0x3 << 18), }

/*****************************************************************************************************************/

#define I2C0_PORTC							\
	{ .name = "i2c0-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x3 << 10, }
#define I2C0_PORTF							\
	{ .name = "i2c0-pf", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x3 << 8, }
#define I2C1_PORTE_1							\
	{ .name = "i2c1-1", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3 << 9, }
#define I2C1_PORTE_2							\
	{ .name = "i2c1-2", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3 << 18, }
#define I2C2_PORTB							\
	{ .name = "i2c2-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x3 << 26, }
#define I2C2_PORTF							\
	{ .name = "i2c2-pf", .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x3<<6, }
#define I2C3_PORTC							\
	{ .name = "i2c3-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x3 << 0, }
#define I2C3_PORTF							\
	{ .name = "i2c3-pf", .port = GPIO_PORT_F, .func = GPIO_FUNC_2, .pins = 0x3 << 26, }
#define I2C4_PORTB							\
	{ .name = "i2c4-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x3 << 28, }
#define I2C4_PORTC							\
	{ .name = "i2c4-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x3 << 20, }

/*******************************************************************************************************************/

#define NAND_PORTAB_COMMON						\
	{ .name = "nand-0", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x000f00ff, }
#define NAND_PORTA_CS1							\
	{ .name = "nand-cs1", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1 << 21, }
#define NAND_PORTA_CS2							\
	{ .name = "nand-cs2", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1 << 22, }
#define NAND_PORTA_CS3							\
	{ .name = "nand-cs3", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1 << 23, }
#define NAND_PORTA_CS4							\
	{ .name = "nand-cs4", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1 << 24, }

/*******************************************************************************************************************/
#define LCD_PORTC_18BIT						\
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fcff3fc, }
#define LCD_PORTC							\
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fffffff, }

#define SLCDC_PORTC							\
	{ .name = "slcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 0x0fc7f3f0, }
#define SLCDC_PORTC_8BIT					\
	{ .name = "slcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 0x0004f3f0, }

#define EPD_PORTC							\
	{ .name = "epd-24bit", .port = GPIO_PORT_C, .func = GPIO_FUNC_3, .pins = 0x0e4ff3fc, }

#define VO_PORTC_8BIT						\
	{ .name = "vo", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x0fcf0300, }

#define VO_PORTC_16BIT						\
	{ .name = "vo", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x0fcff3f0, }

#define PWM_PORTE_BIT0							\
	{ .name = "pwm0", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 0, }
#define PWM_PORTE_BIT1							\
	{ .name = "pwm1", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 1, }
#define PWM_PORTE_BIT2							\
	{ .name = "pwm2", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 2, }
#define PWM_PORTE_BIT3							\
	{ .name = "pwm3", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 3, }

#define MCLK_PORTE_BIT2                 \
{ .name = "mclk", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 1 << 13, }

/*******************************************************************************************************************/

#define MII_PORTBDF							\
	{ .name = "mii-0", .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x00000010, }, \
	{ .name = "mii-1", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3c000000, }, \
	{ .name = "mii-2", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x0000fff0, }

/*******************************************************************************************************************/

#define OTG_DRVVUS							\
	{ .name = "otg-drvvbus", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 10, }

/*******************************************************************************************************************/

#define ISP_PORTD							\
	{ .name = "isp", .port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x000003ff, }

#define ISP_I2C							\
	{ .name = "isp-i2c",    .port = GPIO_PORT_B,  .func = GPIO_FUNC_2, .pins = 3 << 7, }

/*******************************************************************************************************************/

#define DVP_PORTF_12BIT							\
	{ .name = "dvp-pf-12bit", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x0fffd000, }

#define DVP_PORTF_10BIT							\
	{ .name = "dvp-pf-10bit", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x03ffd000, }

/*******************************************************************************************************************/

#define DMIC_PORTF1							\
	{ .name = "dmic-f1", .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0xc00, }
#define DMIC_PORTF3							\
	{ .name = "dmic-f3", .port = GPIO_PORT_F, .func = GPIO_FUNC_3, .pins = 0xc0, }
#define PWM_GPIO_E2							\
	{ .name = "pwm-gpio-e2", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 2, }
#define PWM_GPIO_E3							\
	{ .name = "pwm-gpio-e3", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 3, }

/*******************************************************************************************************************/

/* JZ SoC on Chip devices list */
extern struct platform_device jz_adc_device;
#if defined(CONFIG_SOC_VPU) && defined(CONFIG_JZ_NVPU)
#if (CONFIG_VPU_NODE_NUM >= 1)
extern struct platform_device jz_vpu0_device;
#endif
#if (CONFIG_VPU_NODE_NUM >= 2)
extern struct platform_device jz_vpu1_device;
#endif
#else
#ifdef CONFIG_JZ_VPU_V12
extern struct platform_device jz_vpu_device;
#endif
#endif

extern struct platform_device jz_uart0_device;
extern struct platform_device jz_uart1_device;
extern struct platform_device jz_uart2_device;
extern struct platform_device jz_uart3_device;

extern struct platform_device jz_ohci_device;
extern struct platform_device jz_ehci_device;


extern struct platform_device jz_ssi0_device;
extern struct platform_device jz_ssi1_device;


extern struct platform_device jz_i2c0_device;
extern struct platform_device jz_i2c1_device;
extern struct platform_device jz_i2c2_device;
extern struct platform_device jz_i2c3_device;

extern struct platform_device jz_msc0_device;
extern struct platform_device jz_msc1_device;
extern struct platform_device jz_msc2_device;

extern struct platform_device backlight_device;
extern struct platform_device jz_fb_device;
extern struct platform_device jz_vo_device;
extern struct platform_device jz_dsi_device;
extern struct platform_device jz_ipu_device;
extern struct platform_device jz_pdma_device;

extern struct platform_device jz_nand_device;

extern struct platform_device jz_dwc_otg_device;
/* tx_isp */
extern struct platform_device tx_isp_csi_platform_device;
extern struct platform_device tx_isp_vic_platform_device;
extern struct platform_device tx_isp_core_platform_device;
extern struct platform_device tx_isp_platform_device;

int jz_device_register(struct platform_device *pdev, void *pdata);

extern struct platform_device jz_i2s_device;
extern struct platform_device jz_pcm_device;
extern struct platform_device jz_spdif_device;
extern struct platform_device jz_dmic_device;
extern struct platform_device jz_codec_device;

extern struct platform_device jz_mixer0_device;
extern struct platform_device jz_mixer1_device;
extern struct platform_device jz_mixer2_device;
extern struct platform_device jz_mixer3_device;
extern struct platform_device jz_rtc_device;
extern struct platform_device jz_efuse_device;
int jz_device_register(struct platform_device *pdev,void *pdata);

#ifdef CONFIG_ANDROID_PMEM
extern void board_pmem_setup(void);
#endif

#endif
/* __PLATFORM_H__ */
