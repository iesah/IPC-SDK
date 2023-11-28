/*
 * Copyright (c) 2006-2010  Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <gpio.h>
#ifdef CONFIG_RECONFIG_SLEEP_GPIO
#include <mach/jzsnd.h>
#endif
//#define RDA8851_DEBUG

#define PM_GPIO_NAND_SD0               GPIO_PA(0) 
#define PM_GPIO_NAND_SD1               GPIO_PA(1) 
#define PM_GPIO_NAND_SD2               GPIO_PA(2) 
#define PM_GPIO_NAND_SD3               GPIO_PA(3) 
#define PM_GPIO_NAND_SD4               GPIO_PA(4) 
#define PM_GPIO_NAND_SD5               GPIO_PA(5) 
#define PM_GPIO_NAND_SD6               GPIO_PA(6) 
#define PM_GPIO_NAND_SD7               GPIO_PA(7) 
#define PM_GPIO_NAND_FRE_N             GPIO_PA(18) 
#define PM_GPIO_NAND_FWE_N             GPIO_PA(19) 
#define PM_GPIO_NAND_FRB_N             GPIO_PA(20) 
#define PM_GPIO_NAND_CS1_N             GPIO_PA(21) 
#define PM_GPIO_NAND_CS2_N             GPIO_PA(22) 
#define PM_GPIO_NAND_CS3_N             GPIO_PA(23) 
#define PM_GPIO_NAND_CS4_N             GPIO_PA(24) 
#define PM_GPIO_NAND_FRB1_N            GPIO_PA(27) 
#define PM_GPIO_NAND_FNDQS             GPIO_PA(29) 
#define PM_GPIO_NAND_SA0_CLE           GPIO_PB(0) 
#define PM_GPIO_NAND_SA1_ALE           GPIO_PB(1) 
#define PM_GPIO_NAND_FWP_N             GPIO_PF(22)

#define PM_GPIO_CIM_PCLK                 GPIO_PB(6) 
#define PM_GPIO_CIM_HSYN                 GPIO_PB(7) 
#define PM_GPIO_CIM_VSYN                 GPIO_PB(8) 
#define PM_GPIO_CIM_MCLK                 GPIO_PB(9) 
#define PM_GPIO_CIM_D0                   GPIO_PB(10) 
#define PM_GPIO_CIM_D1                   GPIO_PB(11) 
#define PM_GPIO_CIM_D2                   GPIO_PB(12) 
#define PM_GPIO_CIM_D3                   GPIO_PB(13) 
#define PM_GPIO_CIM_D4                   GPIO_PB(14) 
#define PM_GPIO_CIM_D5                   GPIO_PB(15) 
#define PM_GPIO_CIM_D6                   GPIO_PB(16) 
#define PM_GPIO_CIM_D7                   GPIO_PB(17) 
#define PM_GPIO_CIM_PWDN_FRONT           GPIO_PB(18) 
#define PM_GPIO_CIM_PWDN_BACK            GPIO_PB(19) 
#define PM_GPIO_CIM_RST                  GPIO_PB(26) 
#define PM_GPIO_CIM_VCC_EN               GPIO_PB(27) 

#define PM_GPIO_DC_DETE                  GPIO_PA(16) 
#define PM_GPIO_PMU_IRQ_N                GPIO_PA(28) 
#define PM_GPIO_AP_WAKEUP_N              GPIO_PA(30) 
#define PM_GPIO_CHARGER_DET              GPIO_PB(3) 
#define PM_GPIO_KEY_1                    GPIO_PB(5) 
#define PM_GPIO_MOTOR_EN                 GPIO_PB(24) 
#define PM_GPIO_MIC_SEL                  GPIO_PB(25) 

#define PM_GPIO_LCD_RESET_EN             GPIO_PB(22) 
#define PM_GPIO_LCD_VCC_EN               GPIO_PB(23) 
#define PM_GPIO_LCD_B0                   GPIO_PC(0)
#define PM_GPIO_LCD_B1                   GPIO_PC(1)
#define PM_GPIO_LCD_B2                   GPIO_PC(2)
#define PM_GPIO_LCD_B3                   GPIO_PC(3)
#define PM_GPIO_LCD_B4                   GPIO_PC(4)
#define PM_GPIO_LCD_B5                   GPIO_PC(5)
#define PM_GPIO_LCD_B6                   GPIO_PC(6)
#define PM_GPIO_LCD_B7                   GPIO_PC(7)
#define PM_GPIO_LCD_PCLK                 GPIO_PC(8)
#define PM_GPIO_LCD_DE                   GPIO_PC(9)
#define PM_GPIO_LCD_G0                   GPIO_PC(10)
#define PM_GPIO_LCD_G1                   GPIO_PC(11)
#define PM_GPIO_LCD_G2                   GPIO_PC(12)
#define PM_GPIO_LCD_G3                   GPIO_PC(13)
#define PM_GPIO_LCD_G4                   GPIO_PC(14)
#define PM_GPIO_LCD_G5                   GPIO_PC(15)
#define PM_GPIO_LCD_G6                   GPIO_PC(16)
#define PM_GPIO_LCD_G7                   GPIO_PC(17)
#define PM_GPIO_LCD_HSYN                 GPIO_PC(18)
#define PM_GPIO_LCD_VSYN                 GPIO_PC(19)
#define PM_GPIO_LCD_R0                   GPIO_PC(20)
#define PM_GPIO_LCD_R1                   GPIO_PC(21)
#define PM_GPIO_LCD_R2                   GPIO_PC(22)
#define PM_GPIO_LCD_R3                   GPIO_PC(23)
#define PM_GPIO_LCD_R4                   GPIO_PC(24)
#define PM_GPIO_LCD_R5                   GPIO_PC(25)
#define PM_GPIO_LCD_R6                   GPIO_PC(26)
#define PM_GPIO_LCD_R7                   GPIO_PC(27)
#define PM_GPIO_LCD_PWM                  GPIO_PE(0)

#define PM_GPIO_I2C3_SDA          GPIO_PD(10)
#define PM_GPIO_I2C3_SCK          GPIO_PD(11)
#define PM_GPIO_HP_MUTE           GPIO_PD(13)
#define PM_GPIO_EAR_MIC_DETE      GPIO_PD(15)
#define PM_GPIO_BOOT_SEL0         GPIO_PD(17)
#define PM_GPIO_BOOT_SEL1         GPIO_PD(18)
#define PM_GPIO_BOOT_SEL2         GPIO_PD(19)

#define PM_GPIO_WLAN_PW_EN        GPIO_PD(8)
#define PM_GPIO_WL_WAKE           GPIO_PD(9)
#define PM_GPIO_WL_MSC1_D0        GPIO_PD(20)
#define PM_GPIO_WL_MSC1_D1        GPIO_PD(21)
#define PM_GPIO_WL_MSC1_D2        GPIO_PD(22)
#define PM_GPIO_WL_MSC1_D3        GPIO_PD(23)
#define PM_GPIO_WL_MSC1_CLK       GPIO_PD(24)
#define PM_GPIO_WL_MSC1_CMD       GPIO_PD(25)
#define PM_GPIO_BT_REG_ON            GPIO_PF(4)
#define PM_GPIO_BT_WAKE              GPIO_PF(5)
#define PM_GPIO_BT_INT               GPIO_PF(6)
#define PM_GPIO_WL_REG_ON            GPIO_PF(7)
#define PM_GPIO_BT_RST_N             GPIO_PF(8)
#define PM_GPIO_BT_PCM_DO            GPIO_PD(0)
#define PM_GPIO_BT_PCM_CLK           GPIO_PD(1)
#define PM_GPIO_BT_PCM_SYN           GPIO_PD(2)
#define PM_GPIO_BT_PCM_DI            GPIO_PD(3)
#define PM_GPIO_BT_UART2_RTS_N       GPIO_PD(4)
#define PM_GPIO_BT_UART2_CTS_N       GPIO_PD(5)
#define PM_GPIO_BT_UART2_RXD         GPIO_PD(6)
#define PM_GPIO_BT_UART2_TXD         GPIO_PD(7)

#define PM_GPIO_I2C0_SDA          GPIO_PD(30)
#define PM_GPIO_I2C0_SCK          GPIO_PD(31)


#define PM_GPIO_ID                 GPIO_PE(2)
#define PM_GPIO_AVDEFUSE_EN_N      GPIO_PE(4)
#define PM_GPIO_AMPEN              GPIO_PE(6)
#define PM_GPIO_JD                 GPIO_PE(7)
#define PM_GPIO_MSC0_D0            GPIO_PE(20)
#define PM_GPIO_MSC0_D1            GPIO_PE(21)
#define PM_GPIO_MSC0_D2            GPIO_PE(22)
#define PM_GPIO_MSC0_D3            GPIO_PE(23)
#define PM_GPIO_MSC0_CLK           GPIO_PE(28)
#define PM_GPIO_MSC0_CMD           GPIO_PE(29)
#define PM_GPIO_I2C1_SDA           GPIO_PE(30)
#define PM_GPIO_I2C1_SCK           GPIO_PE(31)

#define PM_GPIO_SENSOR_POWER_EN      GPIO_PE(9)
#define PM_GPIO_SENSOR_INT1          GPIO_PF(9)
#define PM_GPIO_SENSOR_INT2          GPIO_PF(10)
#define PM_GPIO_USB_DETE             GPIO_PF(13)
#define PM_GPIO_I2C2_SDA             GPIO_PF(16)
#define PM_GPIO_I2C2_SCK             GPIO_PF(17)
#define PM_GPIO_CTP_WAKE_UP          GPIO_PF(18)
#define PM_GPIO_CTP_IRQ              GPIO_PF(19)
#define PM_GPIO_SD0_CD_N_VCC_EN_N    GPIO_PF(20)

#define PM_GPIO_HDMI_DETE_N          GPIO_PE(1)
#define PM_GPIO_HDMI_CEC             GPIO_PF(23)
#define PM_GPIO_HDMI_SCL             GPIO_PF(24)
#define PM_GPIO_HDMI_SDA             GPIO_PF(25)

#define PM_GPIO_RDA8851_POWER_EN            GPIO_PE(8)
#define PM_GPIO_RDA8851_URAT_TXD            GPIO_PD(28)
#define PM_GPIO_RDA8851_URAT_RXD            GPIO_PD(26)
#define PM_GPIO_RDA8851_URAT_RTS_N          GPIO_PD(29)
#define PM_GPIO_RDA8851_URAT_CTS_N          GPIO_PD(27)
#define PM_GPIO_RDA8851_AP_STATUS           GPIO_PB(30)
#define PM_GPIO_RDA8851_GSM_STATUS          GPIO_PB(31)
#define PM_GPIO_RDA8851_GSM_WAKE_AP         GPIO_PB(21)
#define PM_GPIO_RDA8851_AP_WAKE_GSM         GPIO_PB(28)
#define PM_GPIO_GSM_SIM_DETE                GPIO_PE(3)

#define PM_GPIO_UART3_PULL                  GPIO_PA(31)

// default gpio state is input pull;
__initdata int gpio_ss_table[][2] = {
#ifdef RDA8851_DEBUG
    {PM_GPIO_RDA8851_POWER_EN     ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_URAT_TXD     ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_URAT_RXD     ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_URAT_RTS_N   ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_URAT_CTS_N   ,     GSS_OUTPUT_LOW},
	{PM_GPIO_GSM_SIM_DETE         ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_AP_STATUS    ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_GSM_STATUS   ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_GSM_WAKE_AP  ,     GSS_OUTPUT_LOW},
    {PM_GPIO_RDA8851_AP_WAKE_GSM  ,     GSS_OUTPUT_LOW},
#else
    {PM_GPIO_RDA8851_POWER_EN     ,     GSS_IGNORE    },
    {PM_GPIO_RDA8851_URAT_TXD     ,     GSS_IGNORE    },
    {PM_GPIO_RDA8851_URAT_RXD     ,     GSS_IGNORE    },
    {PM_GPIO_RDA8851_URAT_RTS_N   ,     GSS_IGNORE    },
    {PM_GPIO_RDA8851_URAT_CTS_N   ,     GSS_IGNORE    },
	{PM_GPIO_GSM_SIM_DETE         ,     GSS_IGNORE	  },
    {PM_GPIO_RDA8851_AP_STATUS    ,     GSS_IGNORE    },
    {PM_GPIO_RDA8851_GSM_STATUS   ,     GSS_IGNORE    },
    {PM_GPIO_RDA8851_GSM_WAKE_AP  ,     GSS_IGNORE    },
    {PM_GPIO_RDA8851_AP_WAKE_GSM  ,     GSS_IGNORE    },
#endif

    {PM_GPIO_NAND_SD0             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_SD1             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_SD2             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_SD3             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_SD4             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_SD5             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_SD6             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_SD7             ,     GSS_INPUT_NOPULL},
    {PM_GPIO_NAND_FRE_N           ,     GSS_IGNORE      },
    {PM_GPIO_NAND_FWE_N           ,     GSS_IGNORE      },
    {PM_GPIO_NAND_FRB_N           ,     GSS_IGNORE      },
    {PM_GPIO_NAND_CS1_N           ,     GSS_IGNORE      },
    {PM_GPIO_NAND_CS2_N           ,     GSS_IGNORE      },
    {PM_GPIO_NAND_CS3_N           ,     GSS_IGNORE      },
    {PM_GPIO_NAND_CS4_N           ,     GSS_INPUT_PULL  },
    {PM_GPIO_NAND_FRB1_N          ,     GSS_IGNORE      },
    {PM_GPIO_NAND_FNDQS           ,     GSS_IGNORE      },
    {PM_GPIO_NAND_SA0_CLE         ,     GSS_IGNORE      }, 
    {PM_GPIO_NAND_SA1_ALE         ,     GSS_IGNORE      },
	{PM_GPIO_NAND_FWP_N           ,     GSS_OUTPUT_LOW  },

    {PM_GPIO_CIM_PCLK             ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_HSYN             ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_VSYN             ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_MCLK             ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D0               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D1               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D2               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D3               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D4               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D5               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D6               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_D7               ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_PWDN_FRONT       ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_PWDN_BACK        ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_RST              ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_CIM_VCC_EN           ,    GSS_OUTPUT_LOW  },

    {PM_GPIO_LCD_RESET_EN         ,    GSS_INPUT_NOPULL}, 
    {PM_GPIO_LCD_VCC_EN           ,    GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B0               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B1               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B2               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B3               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B4               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B5               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B6               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_B7               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_PCLK             ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_DE               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G0               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G1               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G2               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G3               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G4               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G5               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G6               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_G7               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_HSYN             ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_VSYN             ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R0               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R1               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R2               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R3               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R4               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R5               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R6               ,     GSS_OUTPUT_LOW  },
    {PM_GPIO_LCD_R7               ,     GSS_OUTPUT_LOW  },
	{PM_GPIO_LCD_PWM              ,     GSS_OUTPUT_LOW	},

    {PM_GPIO_WLAN_PW_EN           ,      GSS_IGNORE}, /*cljiang*/
    {PM_GPIO_WL_WAKE              ,      GSS_OUTPUT_LOW	},
    {PM_GPIO_WL_MSC1_D0           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_WL_MSC1_D1           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_WL_MSC1_D2           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_WL_MSC1_D3           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_WL_MSC1_CLK          ,      GSS_INPUT_NOPULL},
    {PM_GPIO_WL_MSC1_CMD          ,      GSS_INPUT_NOPULL},
	{PM_GPIO_WL_REG_ON            ,     GSS_OUTPUT_LOW    },
	{PM_GPIO_BT_REG_ON            ,     GSS_IGNORE	}, /*cljiang*/
	{PM_GPIO_BT_WAKE              ,     GSS_IGNORE}, 
	{PM_GPIO_BT_INT               ,   GSS_INPUT_PULL	}, 
	{PM_GPIO_BT_RST_N             ,   GSS_IGNORE    }, /*cljiang*/
    {PM_GPIO_BT_PCM_DO              ,      GSS_OUTPUT_LOW  },
    {PM_GPIO_BT_PCM_CLK             ,      GSS_IGNORE	    },    
    {PM_GPIO_BT_PCM_SYN             ,      GSS_IGNORE	    },    
    {PM_GPIO_BT_PCM_DI              ,      GSS_OUTPUT_LOW	},
    {PM_GPIO_BT_UART2_RTS_N         ,      GSS_INPUT_NOPULL},
    {PM_GPIO_BT_UART2_CTS_N         ,      GSS_INPUT_NOPULL},
    {PM_GPIO_BT_UART2_RXD           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_BT_UART2_TXD           ,      GSS_INPUT_NOPULL},

    {PM_GPIO_DC_DETE              ,     GSS_IGNORE      },
    {PM_GPIO_PMU_IRQ_N            ,     GSS_IGNORE      },
    {PM_GPIO_AP_WAKEUP_N          ,     GSS_IGNORE      },
    {PM_GPIO_CHARGER_DET          ,     GSS_IGNORE  }, 
    {PM_GPIO_KEY_1                ,    GSS_INPUT_NOPULL}, 
    {PM_GPIO_MOTOR_EN             ,    GSS_OUTPUT_LOW  },

    {PM_GPIO_I2C3_SDA            ,      GSS_INPUT_NOPULL},
    {PM_GPIO_I2C3_SCK            ,      GSS_INPUT_NOPULL},
    {PM_GPIO_BOOT_SEL0           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_BOOT_SEL1           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_BOOT_SEL2           ,      GSS_INPUT_NOPULL},
    {PM_GPIO_I2C0_SDA            ,      GSS_INPUT_NOPULL},
    {PM_GPIO_I2C0_SCK            ,      GSS_INPUT_NOPULL},

	{PM_GPIO_ID                 ,       GSS_INPUT_NOPULL},
	{PM_GPIO_AVDEFUSE_EN_N      ,       GSS_INPUT_NOPULL},
	{PM_GPIO_JD                 ,       GSS_INPUT_NOPULL},
	{PM_GPIO_SENSOR_POWER_EN    ,       GSS_INPUT_PULL	},

	{PM_GPIO_SD0_CD_N_VCC_EN_N    ,   GSS_OUTPUT_HIGH   },
	{PM_GPIO_MSC0_D0            ,       GSS_OUTPUT_LOW  },
	{PM_GPIO_MSC0_D1            ,       GSS_OUTPUT_LOW  },
	{PM_GPIO_MSC0_D2            ,       GSS_OUTPUT_LOW  },
	{PM_GPIO_MSC0_D3            ,       GSS_OUTPUT_LOW  },
	{PM_GPIO_MSC0_CLK           ,       GSS_OUTPUT_LOW  },
	{PM_GPIO_MSC0_CMD           ,       GSS_OUTPUT_LOW  },
	{PM_GPIO_I2C1_SDA           ,       GSS_INPUT_NOPULL},
	{PM_GPIO_I2C1_SCK           ,       GSS_INPUT_NOPULL},

	{PM_GPIO_SENSOR_INT1          ,   GSS_INPUT_NOPULL  },
	{PM_GPIO_SENSOR_INT2          ,   GSS_INPUT_NOPULL  },
	{PM_GPIO_USB_DETE             ,   GSS_IGNORE	    },
	{PM_GPIO_I2C2_SDA             ,   GSS_INPUT_NOPULL  },
	{PM_GPIO_I2C2_SCK             ,   GSS_INPUT_NOPULL  },
	{PM_GPIO_CTP_WAKE_UP          ,   GSS_OUTPUT_LOW	},
	{PM_GPIO_CTP_IRQ              ,   GSS_OUTPUT_LOW	},

	{PM_GPIO_HDMI_DETE_N        ,       GSS_INPUT_NOPULL},
	{PM_GPIO_HDMI_CEC             ,   GSS_OUTPUT_LOW	},
	{PM_GPIO_HDMI_SCL             ,   GSS_INPUT_NOPULL  },
	{PM_GPIO_HDMI_SDA             ,   GSS_INPUT_NOPULL  },

    {PM_GPIO_MIC_SEL              ,    GSS_OUTPUT_HIGH },
    {PM_GPIO_HP_MUTE             ,      GSS_OUTPUT_HIGH	},
    {PM_GPIO_EAR_MIC_DETE        ,      GSS_INPUT_NOPULL},
	{PM_GPIO_AMPEN              ,       GSS_OUTPUT_LOW	},


	{PM_GPIO_UART3_PULL              ,       GSS_INPUT_PULL	},


	/* GPIO Group Set End */
	{GSS_TABLET_END,GSS_TABLET_END	}
};


#ifdef CONFIG_RECONFIG_SLEEP_GPIO
bool need_update_gpio_ss(void)
{
    //printk("i2s_is_incall = %d\n", i2s_is_incall());
    return i2s_is_incall();

}

__initdata int gpio_ss_table2[][2] = {
    {PM_GPIO_MIC_SEL              ,     GSS_IGNORE },
    {PM_GPIO_HP_MUTE             ,      GSS_IGNORE },
    {PM_GPIO_EAR_MIC_DETE        ,      GSS_IGNORE },
	{PM_GPIO_AMPEN              ,       GSS_IGNORE },

	/* GPIO Group Set End */
	{GSS_TABLET_END,GSS_TABLET_END	}
};
#endif
