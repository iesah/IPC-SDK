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
	{ .name = "i2s-sysclk",		.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3<<30, },	\
	{ .name = "i2s-data",		.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3<<28, },	\
	{ .name = "i2s-bitclk",		.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x1<<20,},	\
	{ .name = "i2s-sync",		.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x1<<21,},	\
	{ .name = "i2s-sync1",		.port = GPIO_PORT_E, .func = GPIO_FUNC_3, .pins = 0x1<<3,}

#define PCM_PORTF							\
	{ .name = "pcm",			.port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0xf << 12}
#define DMIC_PORTF							\
	{ .name = "dmic0-sysclk",	.port = GPIO_PORT_F, .func = GPIO_FUNC_3, .pins = 0x3<<6}
#define DMIC_PORTE                           \
	{ .name = "dmic1-sysclk",	.port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x1<<10}
/*******************************************************************************************************************/
#define UART0_PORTF							\
	{ .name = "uart0", .port = GPIO_PORT_F, .func = GPIO_FUNC_2, .pins = 0x0b, }
#define UART1_PORTD							\
	{ .name = "uart1", .port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x9 << 26, }
#define UART2_PORTC							\
	{ .name = "uart2-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 1<<10 | 1<<20, }
#define UART2_PORTF							\
	{ .name = "uart2-pf", .port = GPIO_PORT_F, .func = GPIO_FUNC_2, .pins = 0xc0, }
#define UART3_PORTA							\
	{ .name = "uart3", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x1 << 31, }
#define UART4_PORTB							\
	{ .name = "uart4-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0xf0300000, }
#define UART4_PORTF							\
	{ .name = "uart4-pf", .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x6, }
/*******************************************************************************************************************/

#define MSC0_PORTA_4BIT							\
	{ .name = "msc0-pa-4bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00fc0000, }
#define MSC0_PORTA_8BIT							\
	{ .name = "msc0-pa-8bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00fc00f0, }
#define MSC0_PORTE							\
	{ .name = "msc0-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x30f00000, }
#define MSC1_PORTD							\
	{ .name = "msc1-pd",		.port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x03f00000, }
#define MSC1_PORTE							\
	{ .name = "msc1-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_2, .pins = 0x30f00000, }
#define MSC2_PORTB							\
	{ .name = "msc2-pb",		.port = GPIO_PORT_B, .func = GPIO_FUNC_3, .pins = 0xf0300000, }
#define MSC2_PORTE							\
	{ .name = "msc2-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_3, .pins = 0x30f00000, }

/*******************************************************************************************************************/
/*****************************************************************************************************************/

#define SSI0_PORTA							\
	{ .name = "ssi0-pa",	       .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 0x00fc0000, }
#define SSI0_PORTB							\
	{ .name = "ssi0-pb",	       .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0xf0300000, }
#define SSI0_PORTD							\
	{ .name = "ssi0-pd",	       .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x03f00000, }
#define SSI1_PORTE							\
	{ .name = "ssi1",				.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x30f00000, }

/*****************************************************************************************************************/

#define I2C0_PORTD							\
	{ .name = "i2c0", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3 << 30, }
#define I2C1_PORTE							\
	{ .name = "i2c1", .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x3 << 30, }
#define I2C2_PORTA							\
	{ .name = "i2c2-pa", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x3 << 16, }
#define I2C2_PORTE							\
	{ .name = "i2c2-pe", .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x9, }
#define I2C3_PORTB							\
	{ .name = "i2c3-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3 << 7, }
#define I2C3_PORTC							\
	{ .name = "i2c3-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x3 << 22, }

/*******************************************************************************************************************/

#define NAND_PORTAB_COMMON						\
	{ .name = "nand-0", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x000c00ff, }, \
	{ .name = "nand-1", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x00000003, }
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
	{ .name = "slcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 0x0e0ff3fc, }

#define EPD_PORTC							\
	{ .name = "epd-24bit", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fcff3fc, }

#define PWM_PORTE_BIT0							\
	{ .name = "pwm0", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 0, }
#define PWM_PORTE_BIT1							\
	{ .name = "pwm1", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 1, }
#define PWM_PORTE_BIT2							\
	{ .name = "pwm2", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 2, }
#define PWM_PORTE_BIT3							\
	{ .name = "pwm3", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 3, }

#define MCLK_PORTE_BIT2                 \
{ .name = "mclk", .port = GPIO_PORT_E, .func = GPIO_FUNC_3, .pins = 1 << 2, }

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

#define ISP_PORTC							\
	{ .name = "isp-clk", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 1 << 24, }
/*******************************************************************************************************************/

#define CIM_PORTC							\
	{ .name = "cim-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x0f0ff00c, }
#define CIM_PORTD							\
	{ .name = "cim-pd", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3fff, }

/*******************************************************************************************************************/

#define DMIC_PORTF1							\
	{ .name = "dmic-f1", .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0xc00, }
#define DMIC_PORTF3							\
	{ .name = "dmic-f3", .port = GPIO_PORT_F, .func = GPIO_FUNC_3, .pins = 0xc0, }

/*******************************************************************************************************************/

/* JZ SoC on Chip devices list */
extern struct platform_device jz_adc_device;
extern struct platform_device jz_vpu_device;

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

extern struct platform_device jz_fb_device;
extern struct platform_device jz_dsi_device;
extern struct platform_device jz_ipu_device;
extern struct platform_device jz_pdma_device;

extern struct platform_device jz_nand_device;

extern struct platform_device jz_dwc_otg_device;
/* ovisp */
extern struct platform_device ovisp_device_camera;

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
extern struct platform_device jz_pwm_device;
extern struct platform_device jz_aic_dma_device;
extern struct platform_device jz_aic_device;
extern struct platform_device jz_icdc_device;
extern struct platform_device jz_pcm_dma_device;
extern struct platform_device jz_pcm_device;
extern struct platform_device jz_dump_cdc_device;
int jz_device_register(struct platform_device *pdev,void *pdata);

#ifdef CONFIG_ANDROID_PMEM
extern void board_pmem_setup(void);
#endif

#endif
/* __PLATFORM_H__ */
