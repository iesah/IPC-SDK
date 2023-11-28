/*
 *  Copyright (C) 2010 Ingenic Semiconductor Inc.
 *
 *   In this file, here are some macro/device/function to
 * to help the board special file to organize resources
 * on the chip.
 */

#ifndef __SOC_4775_H__
#define __SOC_4775_H__

/* devio define list */

#define I2S_PORTEF                                                      \
		{ .name = "i2s-sysclk",         .port = GPIO_PORT_E, .func = GPIO_FUNC_2, .pins = 0x1<<3, },    \
		{ .name = "i2s-bitclk",         .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x1<<0,},    \
		{ .name = "i2s-sync",           .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x1<<1,},    \
		{ .name = "i2s-data-in",        .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x1<<2, },	\
		{ .name = "i2s-data-out",       .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x1<<3, }

#define SPDIF_PORTF                                                \
	    { .name = "spdif-data-out",     .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x1<<3,}
#define DMIC_PORTF                          \
	    { .name = "dmic",           .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x3 << 10,}


#define PCM_PORTF							\
	{ .name = "pcm",			.port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0xf << 12}
/*******************************************************************************************************************/
#define UART0_PORTF							\
	{ .name = "uart0", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x0f, }
#define UART1_PORTD							\
	{ .name = "uart1", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0xf<<26, }
#define UART2_PORTC							\
	{ .name = "uart2", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 1<<10 | 1<<20, }
#define UART2_PORTF							\
	{ .name = "uart2", .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x3 << 4, }
/*******************************************************************************************************************/

#define MSC0_PORTA_4BIT							\
	{ .name = "msc0-pa-4bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00fc0000, }
#define MSC0_PORTA_8BIT							\
	{ .name = "msc0-pa-8bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00fc00f0, }
#define MSC0_PORTE							\
	{ .name = "msc0-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x30f00000, }
#define MSC1_PORTD							\
	{ .name = "msc1-pd",		.port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x03f00000, }
#define MSC1_PORTE							\
	{ .name = "msc1-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x30f00000, }
#define MSC2_PORTB							\
	{ .name = "msc2-pb",		.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0xf0300000, }
#define MSC2_PORTE							\
	{ .name = "msc2-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_2, .pins = 0x30f00000, }

/*******************************************************************************************************************/
/*****************************************************************************************************************/
#define SSI0_PORTA                                                     \
       { .name = "ssi0-pa",            .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 0x009c0000, }
#define SSI0_PORTD						       \
       { .name = "ssi0-pd",	       .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x03f00000, }


/*****************************************************************************************************************/

#define I2C0_PORTD							\
	{ .name = "i2c0", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3<<30, }
#define I2C1_PORTE							\
	{ .name = "i2c1", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3<<30, }
#define I2C2_PORTE						\
	{ .name = "i2c2", .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x9, }

/*******************************************************************************************************************/

#define NAND_PORTAB_COMMON                                                      \
        { .name = "nand-pa", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x000c00ff, },	\
        { .name = "nand-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x00000003, }
#define NAND_PORTA_BUS16						\
        { .name = "nand-pg", .port = GPIO_PORT_G, .func = GPIO_FUNC_1, .pins = 0x0003fc00, }
#define NAND_PORTA_CS1                                                      \
        { .name = "nand-cs1", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<21, }
#define NAND_PORTA_CS2                                                      \
        { .name = "nand-cs2", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<22, }
#define NAND_PORTA_CS3                                                      \
        { .name = "nand-cs3", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<23, }

/*******************************************************************************************************************/
#define DISABLE_LCD_PORTC							\
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_OUTPUT0, .pins = 0x0fffffff, }

#ifdef CONFIG_BM347WV_F_8991FTGF_HX8369
#define LCD_PORTC \
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fcff3fc, }
#else
#define LCD_PORTC \
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fffffff, }
#endif

#define EPD_PORTC \
	{ .name = "epd-24bit", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fffffff, }

#define PWM_PORTE_BIT0							\
	{ .name = "pwm0", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 0, }
#define PWM_PORTE_BIT1							\
	{ .name = "pwm1", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 1, }
#define PWM_PORTE_BIT2							\
	{ .name = "pwm2", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 2, }
#define PWM_PORTE_BIT3							\
	{ .name = "pwm3", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 3, }

#define MII_PORTBDF                                                 \
        { .name = "mii-0", .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x00000010, },   \
        { .name = "mii-1", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3c000000, }, \
        { .name = "mii-2", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x0000fff0, }


#define OTG_DRVVUS							\
	{ .name = "otg-drvvbus", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 10, }

#define CIM0_PORTB							\
	{ .name = "cim0",    .port = GPIO_PORT_B,  .func = GPIO_FUNC_0, .pins = 0xfff << 6, }

#define CIM1_PORTG							\
	{ .name = "cim1",    .port = GPIO_PORT_G,  .func = GPIO_FUNC_0, .pins = 0xfff << 6, }


/* JZ SoC on Chip devices list */
extern struct platform_device jz_msc0_device;
extern struct platform_device jz_msc1_device;
extern struct platform_device jz_msc2_device;

extern struct platform_device jz_i2c0_device;
extern struct platform_device jz_i2c1_device;
extern struct platform_device jz_i2c2_device;

extern struct platform_device jz_i2c0_dma_device;
extern struct platform_device jz_i2c1_dma_device;
extern struct platform_device jz_i2c2_dma_device;

extern struct platform_device jz_i2s_device;
extern struct platform_device jz_pcm_device;
extern struct platform_device jz_codec_device;

extern struct platform_device jz4775_codec_device;
extern struct platform_device jz47xx_i2s_device;
extern struct platform_device jz47xx_pcm_device;

extern struct platform_device jz_mixer0_device;
extern struct platform_device jz_mixer1_device;

extern struct platform_device jz_gpu;

extern struct platform_device jz_fb0_device;
extern struct platform_device jz_fb1_device;

extern struct platform_device jz_uart0_device;
extern struct platform_device jz_uart1_device;
extern struct platform_device jz_uart2_device;
extern struct platform_device jz_uart3_device;

extern struct platform_device jz_ssi0_device;
extern struct platform_device jz_ssi1_device;

extern struct platform_device jz_pdma_device;

extern struct platform_device jz_cim0_device;
extern struct platform_device jz_cim1_device;
extern struct platform_device jz_cim_device;

extern struct platform_device jz_ohci_device;
extern struct platform_device jz_ehci_device;

extern struct platform_device jz_nand_device;

extern struct platform_device jz_adc_device;

extern struct platform_device jz_hdmi;
extern struct platform_device jz_rtc_device;
extern struct platform_device jz_vpu_device;
extern struct platform_device jz_x2d_device;
extern struct platform_device jz_dwc_otg_device;

#ifdef CONFIG_JZ_IRDA_V11
extern struct platform_device jz_irda_device;
#endif
#ifdef CONFIG_JZ_EFUSE_V11
extern struct platform_device jz_efuse_device;
#endif
#ifdef CONFIG_I2C_GPIO /*CONFIG_I2C_GPIO*/
extern struct platform_device i2c0_gpio_device;
#endif

int jz_device_register(struct platform_device *pdev,void *pdata);

#ifdef CONFIG_ANDROID_PMEM
extern void board_pmem_setup(void);
#endif

#endif
