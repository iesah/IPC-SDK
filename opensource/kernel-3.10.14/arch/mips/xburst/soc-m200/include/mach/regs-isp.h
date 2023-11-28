#ifndef __ASM_ARCH_REGS_ISP_H
#define __ASM_ARCH_REGS_ISP_H

/* 1. program buffer(112KB): 0x0 ~ 0x01BFFF   */
/* 2. data buffer (12KB): 0x01C000 ~ 0x01EFFF */
/* 3. cache buffer (4KB): 0x01F000 ~ 0x01FFFF */
/* 4. line buffer(192KB): 0x020000 ~ 0x04FFFF */
/* 5. HW register(128KB): 0x050000 ~ 0x06FFFF */

#define FIRMWARE_BASE				(0)
#define COMMAND_BUFFER				(0x1f400)

/* Command set.*/
#define COMMAND_REG0				(0x63900)
#define COMMAND_REG1				(0x63901)
#define COMMAND_REG2				(0x63902)
#define COMMAND_REG3				(0x63903)
#define COMMAND_REG4				(0x63904)
#define COMMAND_REG5				(0x63905)
#define COMMAND_REG6				(0x63906)
#define COMMAND_REG7				(0x63907)
#define COMMAND_REG8				(0x63908)
#define COMMAND_REG9				(0x63909)
#define COMMAND_REG10				(0x6390a)
#define COMMAND_REG11				(0x6390b)
#define COMMAND_REG12				(0x6390c)
#define COMMAND_REG13				(0x6390d)
#define COMMAND_REG14				(0x6390e)
#define COMMAND_REG15				(0x6390f)
#define COMMAND_FINISHED			(0x63910)
#define COMMAND_RESULT				(0x63911)

/*register define, firmware and isp*/
/*1. input configuration*/
#define ISP_INPUT_FORMAT		(0x1f000)
#define SENSOR_OUTPUT_WIDTH		(0x1f002)
#define SENSOR_OUTPUT_HEIGHT	(0x1f004)
#define ISP_IDI_CONTROL			(0x1f006)
#define ISP_IDI_OUTPUT_WIDTH	(0x1f008)
#define ISP_IDI_OUTPUT_HEIGHT	(0x1f00a)
#define ISP_IDI_OUTPUT_H_START	(0x1f00c)
#define ISP_IDI_OUTPUT_V_START	(0x1f00e)
/* 2. output configuration */
/*isp output: 1*/
#define ISP_OUTPUT_FORMAT			(0x1f022)
#define ISP_OUTPUT_WIDTH			(0x1f024)
#define ISP_OUTPUT_HEIGHT			(0x1f026)
/*output1 memory width*/
#define MAC_MEMORY_WIDTH			(0x1f028)
#define MAC_MEMORY_UV_WIDTH			(0x1f02a)
/*isp output: 2*/
#define ISP_OUTPUT_FORMAT_2			(0x1f02c)
#define ISP_OUTPUT_WIDTH_2			(0x1f02e)
#define ISP_OUTPUT_HEIGHT_2			(0x1f030)
/*output1 memory width*/
#define MAC_MEMORY_WIDTH_2			(0x1f032)
#define MAC_MEMORY_UV_WIDTH_2		 (0x1f034)

/* 3. isp configuration */
#define ISP_EXPOSURE_RATIO           (0x1f070)
#define ISP_MAX_EXPOSURE             (0x1f072)
#define ISP_MIN_EXPOSURE             (0x1f074)
#define ISP_MAX_GAIN                 (0x1f076)
#define ISP_MIN_GAIN                 (0x1f078)
#define ISP_VTS                      (0x1f07a)
/* banding configuration */
#define ISP_BANDING_50HZ            (0x1f07c)
#define ISP_BANDING_60HZ            (0x1f07e)
#define ISP_BRACKET_RATIO1           (0x1f080)
#define ISP_BRACKET_RATIO2           (0x1f082)
#define ZOOM_RATIO                   (0x1f084)



/*need to be confirm*/
#define ISP_INPUT_H_SENSOR_START		(0x1e90e)
#define ISP_INPUT_V_SENSOR_START		(0x1e910)

/*need to be confirm*/
#define ISP_INPUT_H_START_3D			(0x1e916)
#define ISP_INPUT_V_START_3D			(0x1e918)
#define ISP_INPUT_H_SENSOR_START_3D		(0x1e91a)
#define ISP_INPUT_V_SENSOR_START_3D		(0x1e91c)

/*need to be confirm below*/
#define ISP_INPUT_MODE				(0x1e920)
#define ISP_FUNCTION_CTRL			(0x1e921)
#define ISP_SCALE_DOWN_H_RATIO1 		(0x1e922)
#define ISP_SCALE_DOWN_H_RATIO2 		(0x1e923)
#define ISP_SCALE_DOWN_V_RATIO1 		(0x1e924)
#define ISP_SCALE_DOWN_V_RATIO2 		(0x1e925)
#define ISP_SCALE_UP_H_RATIO			(0x1e926)
#define ISP_SCALE_UP_V_RATIO			(0x1e928)

#define ISP_YUV_CROP_H_START			(0x1e92a)
#define ISP_YUV_CROP_V_START			(0x1e92c)
#define ISP_YUV_CROP_WIDTH			(0x1e92e)
#define ISP_YUV_CROP_HEIGHT			(0x1e930)


/*need to be confirmed*/
#define ISP_MAX_EXPOSURE_FOR_HDR		(0x1e93c)
#define ISP_MIN_EXPOSURE_FOR_HDR		(0x1e93e)
#define ISP_MAX_GAIN_FOR_HDR			(0x1e940)
#define ISP_MIN_GAIN_FOR_HDR			(0x1e942)


/* Capture parameters. */
#define ISP_CAPTURE_MODE			(0x1e900)
#define ISP_CAPTURE_INPUT_MODE			(0x1e901)

#define ISP_BASE_ADDR_LEFT			(0x1f0a0)
#define ISP_BASE_ADDR_RIGHT			(0x1f0ac)


/* Offline parameters. */
#define ISP_OFFLINE_INPUT_WIDTH 		(0x1e902)
#define ISP_OFFLINE_INPUT_HEIGHT		(0x1e904)
#define ISP_OFFLINE_MAC_READ_MEM_WIDTH		(0x1e906)
#define ISP_OFFLINE_OUTPUT_WIDTH		(0x1e908)
#define ISP_OFFLINE_OUTPUT_HEIGHT		(0x1e90a)
#define ISP_OFFLINE_MAC_WRITE_MEM_WIDTH 	(0x1e90c)
#define ISP_OFFLINE_INPUT_BASE_ADDR		(0x1e910)
#define ISP_OFFLINE_INPUT_BASE_ADDR_RIGHT	(0x1e914)
#define ISP_OFFLINE_OUTPUT_BASE_ADDR		(0x1e918)
#define ISP_OFFLINE_OUTPUT_BASE_ADDR_UV 	(0x1e91c)
#define ISP_OFFLINE_OUTPUT_BASE_ADDR_RIGHT	(0x1e920)
#define ISP_OFFLINE_OUTPUT_BASE_ADDR_RIGHT_UV	(0x1e924)
#define ISP_OFFLINE_FUNCTION_CONTROL		(0x1e929)
#define ISP_OFFLINE_SCALE_DOWN_H_TATIO_1	(0x1e92a)
#define ISP_OFFLINE_SCALE_DOWN_H_TATIO_2	(0x1e92b)
#define ISP_OFFLINE_SCALE_DOWN_V_TATIO_1	(0x1e92c)
#define ISP_OFFLINE_SCALE_DOWN_V_TATIO_2	(0x1e92d)
#define ISP_OFFLINE_SCALE_UP_H_TATIO		(0x1e92e)
#define ISP_OFFLINE_SCALE_UP_V_TATIO		(0x1e930)
#define ISP_OFFLINE_CROP_H_START		(0x1e932)
#define ISP_OFFLINE_CROP_V_START		(0x1e934)
#define ISP_OFFLINE_CROP_WIDTH			(0x1e936)
#define ISP_OFFLINE_CROP_HEIGHT 		(0x1e938)

/* HDR parameters. */
#define ISP_HDR_INPUT_WIDTH			(0x1e902)
#define ISP_HDR_INPUT_HEIGHT			(0x1e904)
#define ISP_HDR_MAC_READ_WIDTH			(0x1e906)
#define ISP_HDR_OUTPUT_WIDTH			(0x1e908)
#define ISP_HDR_OUTPUT_HEIGHT			(0x1e90a)
#define ISP_HDR_MAC_WRITE_WIDTH 		(0x1e90c)
#define ISP_HDR_PROCESS_CONTROL 		(0x1e90e)
#define ISP_HDR_INPUT_BASE_ADDR_LONG		(0x1e910)
#define ISP_HDR_INPUT_BASE_ADDR_SHORT		(0x1e914)
#define ISP_HDR_OUTPUT_BASE_ADDR		(0x1e918)
#define ISP_HDR_OUTPUT_BASE_ADDR_UV		(0x1e91C)
#define ISP_HDR_LONG_EXPOSURE			(0x1e920)
#define ISP_HDR_LONG_GAIN			(0x1e922)
#define ISP_HDR_SHORT_EXPOSURE			(0x1e924)
#define ISP_HDR_SHORT_GAIN			(0x1e926)

/* ISP registers */
/*ISP work mode; Combine Mode (HDR)*/
#define REG_ISP_TOP4				(0x65004)
#define REG_ISP_SOFT_STANDBY			(0x60100)
#define REG_ISP_SOFT_RST			(0x60103)
#define REG_ISP_MCU_RST 			(0x63042)

#define REG_ISP_GENERAL_PURPOSE_REG1		(0x63910)
#define REG_ISP_GENERAL_PURPOSE_REG2		(0x63912)
#define REG_ISP_GENERAL_PURPOSE_REG3		(0x63913)
#define REG_ISP_GENERAL_PURPOSE_REG7		(0x63917)

#define REG_ISP_INT_STAT_C3			(0x63928)
#define REG_ISP_INT_STAT_C2			(0x63929)
#define REG_ISP_INT_STAT_C1			(0x6392a)
#define REG_ISP_INT_STAT_C0			(0x6392b)
#define REG_ISP_INT_EN_C3			(0x63924)
#define REG_ISP_INT_EN_C2			(0x63925)
#define REG_ISP_INT_EN_C1			(0x63926)
#define REG_ISP_INT_EN_C0			(0x63927)

#define REG_ISP_MAC_INT_STAT_H			(0x63b32)
#define REG_ISP_MAC_INT_STAT_L			(0x63b33)
#define REG_ISP_MAC_INT_EN_H			(0x63b53)
#define REG_ISP_MAC_INT_EN_L			(0x63b54)
#define REG_ISP_FORMAT_CTRL			(0x63b34)

#define REG_BASE_ADDR0				(0x63b00)
#define REG_BASE_ADDR1				(0x63b10)
#define REG_BASE_ADDR_READY			(0x63b30)

#define REG_BASE_WORKING_ADDR			(0x63b31)
/*#define REG_BASE_WORKING_ADDR			(0x63b31)
 *FRAME_CTRL0 ????
 * */


#define REG_ISP_BANDFILTER_FLAG 		(0x1c13f)
#define REG_ISP_BANDFILTER_EN			(0x1c140)
#define REG_ISP_BANDFILTER_SHORT_EN		(0x1c141)

#define REG_ISP_FOCUS_STATE			(0x1cd0f)

#define REG_ISP_STRIDE_H			(0x63b38)
#define REG_ISP_STRIDE_L			(0x63b39)

#define REG_ISP_CROP_LEFT_H			(0x65f00)
#define REG_ISP_CROP_LEFT_L			(0x65f01)
#define REG_ISP_CROP_TOP_H			(0x65f02)
#define REG_ISP_CROP_TOP_L			(0x65f03)
#define REG_ISP_CROP_WIDTH_H			(0x65f04)
#define REG_ISP_CROP_WIDTH_L			(0x65f05)
#define REG_ISP_CROP_HEIGHT_H			(0x65f06)
#define REG_ISP_CROP_HEIGHT_L			(0x65f07)
#define REG_ISP_SCALEUP_X_H			(0x65026)
#define REG_ISP_SCALEUP_Y_L			(0x65027)
#define REG_ISP_CLK_USED_BY_MCU 		(0x63042)

/* I2C control. */
#define REG_SCCB_MAST1_SPEED			(0x63600)
#define REG_SCCB_MAST1_SLAVE_ID 		(0x63601)
#define REG_SCCB_MAST1_ADDRESS_H		(0x63602)
#define REG_SCCB_MAST1_ADDRESS_L		(0x63603)
#define REG_SCCB_MAST1_OUTPUT_DATA_H		(0x63604)
#define REG_SCCB_MAST1_OUTPUT_DATA_L		(0x63605)
#define REG_SCCB_MAST1_2BYTE_CONTROL		(0x63606)
#define REG_SCCB_MAST1_INPUT_DATA_H		(0x63607)
#define REG_SCCB_MAST1_INPUT_DATA_L		(0x63608)
#define REG_SCCB_MAST1_COMMAND			(0x63609)
#define REG_SCCB_MAST2_SPEED			(0x63700)
#define REG_SCCB_MAST2_SLAVE_ID			(0x63701)
#define REG_SCCB_MAST2_2BYTE_CONTROL		(0x63706)

#define REG_GPIO_R_REQ_CTRL_72			(0x63d72)
#define REG_GPIO_R_REQ_CTRL_74			(0x63d74)

/* Command set id. */
#define CMD_I2C_GRP_WR				(0x01)
#define CMD_SET_FORMAT				(0x02)
#define CMD_RESET_SENSOR			(0x03)
#define CMD_CAPTURE				(0x04)
#define CMD_CAPTURE_IMAGE				(0x04)
#define CMD_CAPTURE_RAW				(0x05)
#define CMD_OFFLINE_PROCESS			(0x05)
#define CMD_HDR_PROCESS 			(0x06)
#define CMD_FULL_HDR_PROCESS          (0x07)
#define CMD_ABORT				(0x09)
#define CMD_ISP_REG_GRP_WRITE			(0x08)
#define CMD_AWB_MODE				(0x11)
#define CMD_ANTI_SHAKE_ENABLE			(0x12)
#define CMD_AUTOFOCUS_MODE			(0x13)
#define CMD_ZOOM_IN_MODE			(0x14)

/*Command set success flag. */
#define CMD_SET_SUCCESS				(0x01)

/* Firmware download flag. */
#define CMD_FIRMWARE_DOWNLOAD			(0xff)

/* Command set mask. */
/* CMD_I2C_GRP_WR. */
/* REG1:I2C choice */
#define SELECT_I2C_NONE				(0x0 << 0)
#define SELECT_I2C_PRIMARY			(0x1 << 0)
#define SELECT_I2C_SECONDARY			(0x2 << 0)
#define SELECT_I2C_BOTH				(0x3 << 0)
#define SELECT_I2C_16BIT_DATA			(0x1 << 2)
#define SELECT_I2C_8BIT_DATA			(0x0 << 2)
#define SELECT_I2C_16BIT_ADDR			(0x1 << 3)
#define SELECT_I2C_8BIT_ADDR			(0x0 << 3)
#define SELECT_I2C_WRITE			(0x1 << 7)
#define SELECT_I2C_READ				(0x0 << 7)

/* REG_BASE_ADDR_READY. */
#define WRITE_ADDR0_READY			(1 << 0)
#define WRITE_ADDR1_READY			(1 << 1)
#define READ_ADDR0_READY			(1 << 2)
#define READ_ADDR1_READY			(1 << 3)

/* CMD_ZOOM_IN_MODE. */
/* REG1 bit[0] : 1 for high quality, 0 for save power mode */
#define HIGH_QUALITY_MODE			(0x1)
#define SAVE_POWER_MODE 			(0x0)

/* I2C control. */
#define MASK_16BIT_ADDR_ENABLE			(1 << 0)
#define MASK_16BIT_DATA_ENABLE			(1 << 1)
#define I2C_SPEED_100				(0x06)
#define I2C_SPEED_200				(0x08)

/* ISP interrupts mask. */
#define MASK_INT_MAC1_DROP			(1 << 25)
#define MASK_INT_CMDSET				(1 << 24)
#define MASK_INT_MAC1_DONE			(1 << 23)
#define MASK_INT_MAC1				(1 << 22)
#define MASK_INT_MAC				(1 << 21)
#define MASK_ISP_AWB_DONE			(1 << 14)
#define MASK_ISP_STATICTIS_DONE		(1 << 11)
#define MASK_ISP_AEC_DONE			(1 << 9)
#define MASK_ISP_INT_EOF			(1 << 8)
#define MASK_ISP_INT_SOF			(1 << 7)
#define MASK_IDI_INT_EOF			(1 << 6)
#define MASK_IDI_INT_SOF			(1 << 5)
#define MASK_INT_VSYNC				(1 << 4)
#define MASK_INT_DMA_DONE			(1 << 3) // the dma is used for moving parameters in firmware
#define MASK_INT_CMDINTR			(1 << 2)
#define MASK_INT_GYIO				(1 << 1)
#define MASK_INT_GPIO				(1 << 0)

/* ISP mac interrupts mask. */
/* Low mask. */
#define MASK_INT_WRITE_START0			(1 << 0)
#define MASK_INT_WRITE_DONE0			(1 << 1)
#define MASK_INT_DROP0				(1 << 2)
#define MASK_INT_OVERFLOW0			(1 << 3)
#define MASK_INT_WRITE_START1			(1 << 4)
#define MASK_INT_WRITE_DONE1			(1 << 5)
#define MASK_INT_DROP1				(1 << 6)
#define MASK_INT_OVERFLOW1			(1 << 7)
/* High mask. */
#define MASK_INT_FRAME_START			(1 << 8)
#define MASK_INT_DONE				(1 << 9)

/* 1. Input format type. */
#define ISP_PROCESS				(1 << 6)
#define ISP_BYPASS				(0 << 6)
#define SENSOR_8LANES_MIPI			(6 << 3)
//#define SENSOR_3D_MEMORY			(5 << 3)
#define SENSOR_MEMORY				(4 << 3)
//#define SENSOR_3D_MIPI				(3 << 3)
#define SENSOR_SECONDARY_MIPI			(2 << 3)
#define SENSOR_PRIMARY_MIPI			(1 << 3)
//#define SENSOR_DVP				(0 << 3)
#define IFORMAT_RGB888				(6 << 0)
#define IFORMAT_RGB565				(5 << 0)
#define IFORMAT_YUV422				(4 << 0)
#define IFORMAT_RAW14				(3 << 0)
#define IFORMAT_RAW12				(2 << 0)
#define IFORMAT_RAW10				(1 << 0)
#define IFORMAT_RAW8				(0 << 0)

/* input sequence */
#define IFORMAT_GRBG				(0 << 0)
#define IFORMAT_RGGB				(1 << 0)
#define IFORMAT_BGGR				(2 << 0)
#define IFORMAT_GBRG				(3 << 0)
/*IDI control*/
#define IDI_SCALE_ENABLE			(1 << 3)
#define IDI_SCALE_RATIO_0			(0 << 0)
#define IDI_SCALE_RATIO_1			(1 << 0)
#define IDI_SCALE_RATIO_2			(2 << 0)
#define IDI_SCALE_RATIO_3			(3 << 0)
#define IDI_SCALE_RATIO_4			(4 << 0)



/* 2. Output format type. */
#define OFORMAT_YUV420				(8 << 0)
#define OFORMAT_RGB888				(7 << 0)
#define OFORMAT_RGB565				(6 << 0)
#define OFORMAT_NV12				(5 << 0)
#define OFORMAT_YUV422				(4 << 0)
#define OFORMAT_RAW14				(3 << 0)
#define OFORMAT_RAW12				(2 << 0)
#define OFORMAT_RAW10				(1 << 0)
#define OFORMAT_RAW8				(0 << 0)

#define ovisp_output_fmt_is_raw(fmt)		(fmt < 4)

/* offline output format type */
#define OFFLINE_OFORMAT_YUV420				(1 << 0)
#define OFFLINE_OFORMAT_YUV422				(0 << 0)

/* Reset type. */
/* REG_ISP_SOFT_STANDBY&REG_ISP_SOFT_RST&REG_ISP_MCU_RST. */
#define DO_SOFTWARE_STAND_BY			(0x01)
#define MCU_SOFT_RST				(0x01)
#define DO_SOFT_RST				(0x01)
#define RELEASE_SOFT_RST			(0x00)

#endif /* __ASM_ARCH_REGS_ISP_H */
