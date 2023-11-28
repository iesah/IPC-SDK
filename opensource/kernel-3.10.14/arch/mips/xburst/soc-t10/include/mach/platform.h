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

/*******************************************************************************************************************/
#define UART0_PORTB							\
	{ .name = "uart0", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0xF << 19, }
#define UART1_PORTB							\
	{ .name = "uart1", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x3 << 23, }
/*******************************************************************************************************************/

#define MSC0_PORTB_4BIT							\
	{ .name = "msc0-pb-4bit",	.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = (0x3<<4|0xf<<0), }
#define MSC1_PORTC							\
	{ .name = "msc1-pC",		.port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = (0x3f<<2), }

/*******************************************************************************************************************/
/*****************************************************************************************************************/

#define SSI0_PORTC							\
	{ .name = "ssi0-pa",	       .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x1F800, }
/* SFC : the function of hold and reset not use, hardware pull up */
#define SFC_PORTA_QUAD							\
	{ .name = "sfc",		.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = (0x3f << 23), }
#define SFC_PORTA							\
	{ .name = "sfc",		.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = (0x3 << 23) | (0x3 << 27), }

/*****************************************************************************************************************/

#define I2C0_PORTA							\
	{ .name = "i2c0-pa", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x3 << 12, }
#define I2C1_PORTB							\
	{ .name = "i2c1-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x3 << 25, }
/*******************************************************************************************************************/

/*******************************************************************************************************************/
#define PWM_PORTB_BIT17					\
	{ .name = "pwm0", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 1 << 17, }
#define PWM_PORTB_BIT18							\
	{ .name = "pwm1", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 1 << 18, }
#define PWM_PORTC_BIT8							\
	{ .name = "pwm2", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 1 << 8, }
#define PWM_PORTC_BIT9							\
	{ .name = "pwm3", .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 1 << 9, }

#define MCLK_PORTA                 \
{ .name = "mclk", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 1 << 15, }

/*******************************************************************************************************************/

#define MII_PORTBDF							\
	{ .name = "mii-0", .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x00000010, }, \
	{ .name = "mii-1", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3c000000, }, \
	{ .name = "mii-2", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x0000fff0, }

/*******************************************************************************************************************/

#define OTG_DRVVUS							\
	{ .name = "otg-drvvbus", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 1 << 13, }

/*******************************************************************************************************************/

#define ISP_PORTD							\
	{ .name = "isp", .port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x000003ff, }

#define ISP_I2C							\
	{ .name = "isp-i2c",    .port = GPIO_PORT_B,  .func = GPIO_FUNC_2, .pins = 3 << 7, }

/*******************************************************************************************************************/

#define DVP_PORTA_LOW_10BIT							\
	{ .name = "dvp-pa-10bit", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x000343ff, }

#define DVP_PORTA_HIGH_10BIT							\
	{ .name = "dvp-pa-10bit", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00034ffc, }

#define DVP_PORTA_12BIT							\
	{ .name = "dvp-pa-12bit", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00034fff, }

/*******************************************************************************************************************/

#define DMIC_PORTC							\
	{ .name = "dmic_pc",	.port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x1<<15 | 0x3 << 11}

/*******************************************************************************************************************/
#define GMAC_PORTB							\
	{ .name = "gmac_pb",	.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x0001efc0}

/*******************************************************************************************************************/

/* JZ SoC on Chip devices list */
extern struct platform_device jz_adc_device;
#if defined(CONFIG_SOC_VPU) && defined(CONFIG_JZ_NVPU)
#if (CONFIG_VPU_NODE_NUM >= 1)
extern struct platform_device jz_vpu0_device;
#endif
#else
#ifdef CONFIG_JZ_VPU_V12
extern struct platform_device jz_vpu_device;
#endif
#endif

extern struct platform_device jz_uart0_device;
extern struct platform_device jz_uart1_device;

extern struct platform_device jz_ssi0_device;

extern struct platform_device jz_i2c0_device;
extern struct platform_device jz_i2c1_device;

extern struct platform_device jz_sfc_device;
extern struct platform_device jz_msc0_device;
extern struct platform_device jz_msc1_device;
extern struct platform_device jz_ipu_device;
extern struct platform_device jz_pdma_device;

extern struct platform_device jz_dwc_otg_device;
/* tx_isp */
extern struct tx_isp_subdev_platform_data tx_isp_subdev_csi;
extern struct tx_isp_subdev_platform_data tx_isp_subdev_vic;
extern struct tx_isp_subdev_platform_data tx_isp_subdev_core;
extern struct tx_isp_subdev_platform_data tx_isp_subdev_video_in;
extern struct platform_device tx_isp_csi_platform_device;
extern struct platform_device tx_isp_vic_platform_device;
extern struct platform_device tx_isp_core_platform_device;
extern struct platform_device tx_isp_platform_device;

extern struct platform_device jz_i2s_device;
extern struct platform_device jz_dmic_device;
extern struct platform_device jz_codec_device;

/* alsa */
#ifdef CONFIG_SND_ALSA_INGENIC
extern struct platform_device jz_aic_device;
extern struct platform_device jz_aic_dma_device;
extern struct platform_device jz_alsa_device;
#endif
extern struct platform_device jz_mixer0_device;
extern struct platform_device jz_mixer1_device;
extern struct platform_device jz_mixer2_device;
extern struct platform_device jz_mixer3_device;
extern struct platform_device jz_rtc_device;
extern struct platform_device jz_efuse_device;
extern struct platform_device jz_tcu_device;

int jz_device_register(struct platform_device *pdev, void *pdata);

#endif
/* __PLATFORM_H__ */
