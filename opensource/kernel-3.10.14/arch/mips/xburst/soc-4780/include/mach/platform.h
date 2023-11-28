/*
 *  Copyright (C) 2010 Ingenic Semiconductor Inc.
 *
 *   In this file, here are some macro/device/function to
 * to help the board special file to organize resources
 * on the chip.
 */

#ifndef __SOC_4780_H__
#define __SOC_4780_H__

/* devio define list */

#define I2S_PORTDE							\
	{ .name = "i2s-sysclk",		.port = GPIO_PORT_E, .func = GPIO_FUNC_2, .pins = 0x1<<5, },	\
	{ .name = "i2s-data",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3<<6, },	\
	{ .name = "i2s-bitclk",		.port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x1<<12,},	\
	{ .name = "i2s-sync",		.port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x1<<13,},	\
	{ .name = "i2s-iclk",		.port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x3<<8, }
#define PCM_PORTD							\
	{ .name = "pcm",			.port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x0f}
/*******************************************************************************************************************/
#define UART0_PORTF							\
	{ .name = "uart0", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x0f, }
#define UART1_PORTD							\
	{ .name = "uart1", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0xf<<26, }
#define UART2_PORTD							\
	{ .name = "uart2", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0xf<<4, }
#define UART3_PORTDE							\
	{ .name = "uart3-pd-f0", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x1<<12, },\
	{ .name = "uart3-pe-f1", .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x1<<5, },	\
	{ .name = "uart3-pe-f0", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3<<7, }
#define UART3_JTAG \
	{ .name = "uart3-jtag-31", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 1<<31, },\
	{ .name = "uart3-jtag-30", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 1<<30, }
#define UART4_PORTC							\
	{ .name = "uart4", .port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 1<<10 | 1<<20, }
/*******************************************************************************************************************/

#define MSC0_PORTA_4BIT							\
	{ .name = "msc0-pa-4bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00fc0000, }
#define MSC0_PORTA_8BIT							\
	{ .name = "msc0-pa-8bit",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00fc00f0, }
#define MSC0_PORTE							\
	{ .name = "msc0-pe",		.port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x30f00000, }
#define MSC0_PORTA_4BIT_RESET						\
	{ .name = "msc0-pa-4bit-reset",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x01fc0000, }
#define MSC0_PORTA_8BIT_RESET						\
	{ .name = "msc0-pa-8bit-reset",	.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x01fc00f0, }
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

#define SSI0_PORTA                                                      \
        { .name = "ssi0-pa",           .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 0x00b40000, }
#define SSI0_PORTB						       \
       { .name = "ssi0-pb",	       .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0xf0300020, }
#define SSI0_PORTD						       \
       { .name = "ssi0-pd",	       .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x03f00000, }
#define SSI0_PORTE						       \
       { .name = "ssi0-pe",	       .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x000fc000, }

#define SSI1_PORTB						       \
       { .name = "ssi1-pb",	       .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0xf0300000, }
#define SSI1_PORTD						       \
       { .name = "ssi1-pd",	       .port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x03f00000, }
#define SSI1_PORTE						       \
       { .name = "ssi1-pe",	       .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x000fc000, }


/*****************************************************************************************************************/

#define I2C0_PORTD							\
	{ .name = "i2c0", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3<<30, }
#define I2C1_PORTE							\
	{ .name = "i2c1", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3<<30, }
#define I2C2_PORTF							\
	{ .name = "i2c2", .port = GPIO_PORT_F, .func = GPIO_FUNC_2, .pins = 0x3<<16, }
#define I2C3_PORTD							\
	{ .name = "i2c3", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3<<10, }
#define I2C4_PORTE_OFF3							\
	{ .name = "i2c4-port-e-func1-off3", .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x3<<3, }
#define I2C4_PORTE_OFF12						\
	{ .name = "i2c4-port-e-func1-off12", .port = GPIO_PORT_E, .func = GPIO_FUNC_1, .pins = 0x3<<12, }
#define I2C4_PORTF							\
	{ .name = "i2c4-port-f-func1", .port = GPIO_PORT_F, .func = GPIO_FUNC_1, .pins = 0x3<<24, }

#define SRAM_CS5_PORTAB_BIT8						\
	{ .name = "sram0", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x02030000, },     \
        { .name = "sram1", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x0000003c, }
/*******************************************************************************************************************/
#define NAND_PORTAB_COMMON                                                      \
        { .name = "nand-0", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x000c00ff, },     \
        { .name = "nand-1", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x00000003, }
#define NAND_PORTA_CS1                                                      \
        { .name = "nand-cs1", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<21, }
#define NAND_PORTA_CS2                                                      \
        { .name = "nand-cs2", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<22, }
#define NAND_PORTA_CS3                                                      \
        { .name = "nand-cs3", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<23, }
#define NAND_PORTA_CS4                                                      \
        { .name = "nand-cs4", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<24, }
#define NAND_PORTA_CS5                                                      \
        { .name = "nand-cs5", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<25, }
#define NEMC_PORTA_CS6                                                      \
        { .name = "nemc-cs6", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x1<<26 | 0x1<<17 | 0x1<<16, }
#define DM9000_CDM_TO_SA1                                                   \
        { .name = "sdio-func", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 0x000000ff, }, \
        { .name = "addr-sa1",  .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x00000002, }
/*******************************************************************************************************************/
#define DISABLE_LCD_AND_UART4_PORTC							\
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_OUTPUT0, .pins = 0x0fffffff, }
#define DISABLE_LCD_PORTC							\
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_OUTPUT0, .pins = 0x0fEffBff, }

#define LCD_PORTC							\
	{ .name = "lcd", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fffffff, }

#define PWM_PORTE_BIT0							\
	{ .name = "pwm0", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 0, }
#define PWM_PORTE_BIT1							\
	{ .name = "pwm1", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 1, }
#define PWM_PORTE_BIT2							\
	{ .name = "pwm2", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 2, }
#define PWM_PORTE_BIT3							\
	{ .name = "pwm3", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 3, }
#define PWM_PORTE_BIT4							\
	{ .name = "pwm4", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 4, }
#define PWM_PORTE_BIT5							\
	{ .name = "pwm5", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 5, }
#define PWM_PORTD_BIT10							\
	{ .name = "pwm6", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 1 << 10, }
#define PWM_PORTD_BIT11							\
	{ .name = "pwm7", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 1 << 11, }

#define MII_PORTBDF							\
	{ .name = "mii-0", .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x10, },	\
	{ .name = "mii-1", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3c000000, }, \
	{ .name = "mii-2", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0xfff0, }

#define OTG_DRVVUS							\
	{ .name = "otg-drvvbus", .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 10, }

#define HDMI_PORTF							\
	{ .name = "hdmi-ddc",    .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x3800000, }

#define CIM_PORTB							\
	{ .name = "cim",    .port = GPIO_PORT_B,  .func = GPIO_FUNC_0, .pins = 0xfff << 6, }

#define PS2_JTAG \
	{ .name = "ps2-jtag-30", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 1<<30, },\
	{ .name = "ps2-jtag-31", .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = 1<<31, }

#define PS2_MOUSE_PORTD							\
	{ .name = "ps2-mouse-pd", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3 << 4, }

#define PS2_KEYBOARD_PORTD						\
	{ .name = "ps2-keyboard-pd", .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3 << 6, }
/* JZ SoC on Chip devices list */
extern struct platform_device jz_msc0_device;
extern struct platform_device jz_msc1_device;
extern struct platform_device jz_msc2_device;

extern struct platform_device jz_i2c0_device;
extern struct platform_device jz_i2c1_device;
extern struct platform_device jz_i2c2_device;
extern struct platform_device jz_i2c3_device;
extern struct platform_device jz_i2c4_device;

extern struct platform_device jz_i2s_device;
extern struct platform_device jz_pcm_device;
extern struct platform_device jz_spdif_device;
extern struct platform_device jz_codec_device;

/* only for ALSA device */
extern struct platform_device jz4780_codec_device;
extern struct platform_device jz47xx_i2s_device;
extern struct platform_device jz47xx_pcm_device;

extern struct platform_device jz_mixer0_device;
extern struct platform_device jz_mixer1_device;
extern struct platform_device jz_mixer2_device;

extern struct platform_device jz_gpu;

extern struct platform_device jz_fb0_device;
extern struct platform_device jz_fb1_device;

extern struct platform_device jz_aosd_device;

extern struct platform_device jz_ipu0_device;
extern struct platform_device jz_ipu1_device;

extern struct platform_device jz_uart0_device;
extern struct platform_device jz_uart1_device;
extern struct platform_device jz_uart2_device;
extern struct platform_device jz_uart3_device;
extern struct platform_device jz_uart4_device;

extern struct platform_device jz_ssi0_device;
extern struct platform_device jz_ssi1_device;

extern struct platform_device jz_pdma_device;

extern struct platform_device jz_cim_device;

extern struct platform_device jz_ohci_device;
extern struct platform_device jz_ehci_device;

extern struct platform_device jz_mac;

extern struct platform_device jz_nand_device;

extern struct platform_device jz_adc_device;

extern struct platform_device jz_hdmi;
extern struct platform_device jz_rtc_device;
extern struct platform_device jz_vpu_device;
extern struct platform_device jz_x2d_device;
extern struct platform_device jz_dwc_otg_device;
extern struct platform_device jz_efuse_device;

int jz_device_register(struct platform_device *pdev,void *pdata);

#endif
