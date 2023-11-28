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

#define MSC0_PORTB							\
	{ .name = "msc0-pB",		.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = (0x3f<<0), }
#define MSC1_PORTB							\
	{ .name = "msc1-pB",		.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = (0x3f<<17), }
#define MSC1_PORTC							\
	{ .name = "msc1-pC",		.port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = (0x3f<<2), }
#define MSC1_PORTC_1BIT							\
	{ .name = "msc1-pC-1BIT",		.port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = (0xf<<2), },\
	{ .name = "msc1-Dat2-3",		.port = GPIO_PORT_C, .func = GPIO_OUTPUT1, .pins = (0x3<<6), }
#define I2S_PORTC                           \
	{.name = "i2s",              .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = (0xff << 11),}

/*******************************************************************************************************************/

/*******************************************************************************************************************/
#define SSI0_PORTC						\
	{ .name = "ssi0-pc",	       .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = (0xf << 2), }
#define SSI0_PORTA						\
	{ .name = "ssi0-pa",	       .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = (0xf << 6), }
#define SSI1_PORTB						\
	{ .name = "ssi1-pb",	       .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = (0xf << 27), }
#define SSI1_PORTA						\
	{ .name = "ssi1-pa",	       .port = GPIO_PORT_A, .func = GPIO_FUNC_0, .pins = (0x7 << 20) | (0x1 << 29), }
// ssi slave
#define SSISLV_PORTC					\
	{ .name = "ssi-slave-pc",	   .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = (0xf << 15), }

/*****************************************************************************************************************/

/*******************************************************************************************************************/
/* SFC : the function of hold and reset not use, hardware pull up */
#define SFC0_PORTA							\
	{ .name = "sfc",		.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = (0x3f << 23), }
#define SFC1_PORTA							\
	{ .name = "sfc",		.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = (0x7 << 20) | (0x7 << 29), }

/*****************************************************************************************************************/
#define I2C0_PORTA    \
	{ .name = "i2c0-pa", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x3 << 12, }
#define I2C1_PORTB    \
	{ .name = "i2c1-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x3 << 25, }
#define I2C1_PORTC    \
	{ .name = "i2c1-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x3 << 19, }
#define I2C1_PORTD    \
	{ .name = "i2c1-pd", .port = GPIO_PORT_D, .func = GPIO_FUNC_2, .pins = 0x3 << 6, }
#define I2C2_PORTB_1  \
	{ .name = "i2c2-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x3 << 10, }
#define I2C2_PORTB_2  \
	{ .name = "i2c2-pb", .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3 << 27, }
#define I2C2_PORTC    \
	{ .name = "i2c2-pc", .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x3 << 8, }
/*******************************************************************************************************************/

/*******************************************************************************************************************/
#define MCLK_PORTA                 \
{ .name = "mclk", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 1 << 15, }

/*******************************************************************************************************************/

#define MII_PORTBDF							\
	{ .name = "mii-0", .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x00000010, }, \
	{ .name = "mii-1", .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3c000000, }, \
	{ .name = "mii-2", .port = GPIO_PORT_F, .func = GPIO_FUNC_0, .pins = 0x0000fff0, }

/*******************************************************************************************************************/

#define OTG_DRVVUS							\
	{ .name = "otg-drvvbus", .port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 1 << 27, }

/*******************************************************************************************************************/

#define DVP_PORTA_LOW_10BIT							\
	{ .name = "dvp-pa-10bit", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x000343ff, }

#define DVP_PORTA_HIGH_10BIT							\
	{ .name = "dvp-pa-10bit", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00034ffc, }

#define DVP_PORTA_12BIT							\
	{ .name = "dvp-pa-12bit", .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x00034fff, }

/*******************************************************************************************************************/

#define DMIC_PORTB							\
	{ .name = "dmic_pb",	.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x7 << 28,}
#define DMIC_PORTA							\
	{ .name = "dmic_pa",	.port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = (0x1 << 14) || (0x3 << 16),}

/*******************************************************************************************************************/
#define DPU_PORTD_SLCD_12BIT                                                    \
        { .name = "dpu_slcd_12bit",  .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = (0x1fff << 0),}
/*******************************************************************************************************************/
#define GMAC_PORTB							\
	{ .name = "gmac_pb",	.port = GPIO_PORT_B, .func = GPIO_FUNC_0, .pins = 0x0001efc0,}

/*******************************************************************************************************************/
#define BT0_PORTD                            \
	{.name = "bt0-pd",		.port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x000001ff}
/*******************************************************************************************************************/
/* JZ SoC on Chip devices list */
extern struct platform_device jz_adc_device;

#if defined(CONFIG_AVPU) && defined(CONFIG_AVPU_DRIVER)
extern struct platform_device jz_avpu_irq_device;
#else
#ifdef CONFIG_JZ_VPU_IRQ_TEST
extern struct platform_device jz_vpu_irq_device;
#elif defined(CONFIG_SOC_VPU)
#ifdef CONFIG_VPU_HELIX
#if (CONFIG_VPU_HELIX_NUM >= 1)
extern struct platform_device jz_vpu_helix0_device;
#endif
#endif
#ifdef CONFIG_VPU_RADIX
#if (CONFIG_VPU_RADIX_NUM >= 1)
extern struct platform_device jz_vpu_radix0_device;
#endif
#endif
#endif
#endif

extern struct platform_device jz_uart0_device;
extern struct platform_device jz_uart1_device;
extern struct platform_device jz_uart2_device;
extern struct platform_device jz_uart3_device;
extern struct platform_device jz_uart4_device;
extern struct platform_device jz_uart5_device;

extern struct platform_device jz_ssi1_device;
extern struct platform_device jz_ssi0_device;

extern struct platform_device jz_i2c0_device;
extern struct platform_device jz_i2c1_device;
extern struct platform_device jz_i2c2_device;

extern struct platform_device jz_sfc0_device;
extern struct platform_device jz_sfc1_device;
extern struct platform_device jz_msc0_device;
extern struct platform_device jz_msc1_device;
extern struct platform_device jz_ipu_device;
extern struct platform_device jz_i2d_device;
extern struct platform_device jz_vo_device;
extern struct platform_device jz_ldc_device;
//extern struct platform_device jz_aip_device[3];
extern struct platform_device jz_dbox_device;
extern struct platform_device jz_pdma_device;

extern struct platform_device jz_i2s_device;
extern struct platform_device jz_dwc_otg_device;
extern struct platform_device jz_dmic_device;
extern struct platform_device jz_codec_device;
extern struct platform_device es8374_codec_device;
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
extern struct platform_device jz_pwm_device;
extern struct platform_device jz_tcu_device;

#ifdef CONFIG_JZ_BSCALER
extern struct platform_device jz_bscaler_device;
#endif

int jz_device_register(struct platform_device *pdev, void *pdata);
void *get_driver_common_interfaces(void);

#endif
/* __PLATFORM_H__ */
