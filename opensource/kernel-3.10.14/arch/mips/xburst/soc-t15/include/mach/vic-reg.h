#ifndef __VIC_REG_H__
#define __VIC_REG_H__

#define VIC_SUPPORT_MIPI                 1
#define VIC_DB_CFG		         0x0
#define DVP_DATA_POS			(1<<24)
#define DVP_RGB_ORDER			(1<<21)
#define DVP_RAW_ALIG			(1<<20)
#define DVP_DATA_TYPE			(17)
#define DVP_RAW8			(0<<DVP_DATA_TYPE)
#define DVP_RAW10			(1<<DVP_DATA_TYPE)
#define DVP_RAW12			(2<<DVP_DATA_TYPE)
#define DVP_YUV422_16BIT		(3<<DVP_DATA_TYPE)
#define DVP_RGB565_16BIT		(4<<DVP_DATA_TYPE)
#define DVP_BRG565_16BIT		(5<<DVP_DATA_TYPE)
#define DVP_YUV422_8BIT			(6<<DVP_DATA_TYPE)
#define DVP_RGB565_8BIT			(7<<DVP_DATA_TYPE)

#define DVP_TIMEING_MODE		(1<<15)
#define BT_INTF_WIDE			(1<<11)
#define BT_LINE_MODE			(1<<10)
#define BT_601_TIMING_MODE		(1<<9)
#define YUV_DATA_ORDER			(4)
#define UYVY				(0<<YUV_DATA_ORDER)
#define VYUY				(1<<YUV_DATA_ORDER)
#define YUYV				(2<<YUV_DATA_ORDER)
#define YVYU				(3<<YUV_DATA_ORDER)
#define FIRST_FIELD_TYPE		(1<<3)
#define INTERLACE_EN			(1<<2)
#define HSYN_POLAR			(1<<1)
#define VSYN_POLAR			(1<<0)

#define VIC_HCNT0                       0x04
#define VIC_HCNT1                       0x08
#define VIC_VCNT0                       0x0C
#define VIC_VCNY1                       0x10
#define VIC_VCNY2                       0x14

#define VIC_IDI_TYPE		        0x18
#define MIPI_RAW8			0x2A
#define MIPI_RAW10			0x2B
#define MIPI_RAW12			0x2C
#define MIPI_RGB555			0x21
#define MIPI_RGB565			0x22
#define MIPI_RGB888			0x24
#define MIPI_YUV422			0x1E

#define VIC_INTF_TYPE                   0x1C
#define INTF_TYPE_BT656			0x0
#define INTF_TYPE_BT601			0x1
#define INTF_TYPE_MIPI			0x2
#define INTF_TYPE_DVP			0x3
#define INTF_TYPE_BT1120		0x4

#define VIC_RESOLUTION	 	        0x20
#define H_RESOLUTION			(1<<16)
#define V_RESOLUTION			(1)

#define VIC_H_BLANK_NUM		        (0x24)
#define VIC_V_BLANK_NUM		        (0x28)

#define VIC_AB_VALUE		        (0x2C)
#define A_VALUE				(1<<16)
#define B_VALUE				(1)

#define VIC_GLOBAL_CFG             	(0x30)
#define ISP_PRESET_MODE2		(0<<5)
#define ISP_PRESET_MODE3		(1<<5)
#define ISP_PRESET_MODE1		(2<<5)
#define VCKE_EN				(1<<4)
#define BLANK_EN			(2)
#define AB_MODE_SELECT			(0)

#define VIC_CONTROL			(0x34)
#define VIC_RESET			(1<<4)
#define GLB_SAFE_RST			(1<<3)
#define GLB_RST				(1<<2)
#define REG_ENABLE			(1<<1)
#define VIC_SRART			(1<<0)

#define VIC_PCLK_GATE_EN                0x38
#define VIC_PIXEL			0x3c
#define VIC_LINE			0x40
#define VIC_STATE			0x48
#define VIC_OFIFO_COUNT			0x4C
#define VIC_FLASH_STROBE		0x50
#define VIC_FIRST_CB			0x54
#define VIC_SECOND_CB			0x58
#define VIC_THIRD_CB			0x5c
#define VIC_FOURTH_CB			0x60
#define VIC_FIFTH_CB			0x64
#define VIC_SIXTH_CB			0x68
#define VIC_SEVENTH_CB			0x6c
#define VIC_EIGHTH_CB			0x70
#define CB_MODE0			0x74
#define CB_MODE1			0x78
#define BK_NUM_CB1			0x7c
#endif
