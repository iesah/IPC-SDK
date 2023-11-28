#ifndef __LINUX_ITE7258_H
#define __LINUX_ITE7258_H

#define PRESS_MAX   0xFF
#define ITE7258_NAME "ite7258_ts"

#define MAX_BUFFER_SIZE 144

#define CMD_BUF_ADDR            0x20    /* write only */
#define CMD_RESPONSE_BUF_ADDR   0xA0    /* read  only */
#define QUERY_BUF_ADDR          0x80    /* read  only */
#define POINT_INFO_BUF_ADDR     0xE0    /* read  only */

#define POINT0_FLAG             0x01


/*
 *ite7258 command
 **/

        /*  command 0   */
#define INDENTIFY_CAP           0x00    /* Identify Cap Sensor */


        /*  command 1   */
#define INQUIRY_INFO            0x01    /* Inquiry Cap Sensor Information */
/*  sub command */
#define FIRMWARE_INFO           0x00    /* Firmware Information */
#define IT_2D_RESOLUTIONS       0x02    /* 2D Resolutions */
#define FLASH_SIZE              0x03    /* Flash Size */
#define IRQ_STATUS              0x04    /* Interrupt Notification Status */
#define GESTURE_INFO            0x05    /* Gesture Information */
#define CONF_VERSION            0x06    /* Configuration Version */

        /*  command 2   */
#define SET_CAP_INFO            0x02    /* Set Cap Sensor Information */

#define SET_PWR_MODE            0x04    /* Set Power Mode */
#define GET_VARIAVLE_VAL        0x05    /* Get Variable Value */
#define SET_VARIAVLE_VAL        0x06    /* Set Variable Value */
#define ENTER_UPGRADE_MODE      0x08    /* Enter Firmware Upgrade Mode */
#define EXIT_UPGRADE_MODE       0x60    /* Exit Firmware Upgrade Mode */
#define SET_OFFSET_FLASH_WR     0x09    /* Set Start Offset of Flash for Read/Write */
#define WRITE_FLASH             0x0A    /* Write Flash */
#define READ_FALSH              0x0B    /* Read Flash */
#define REINIT_FIRMWARE         0x0C    /* Reinitialize Firmware */
#define WRITE_MEMORY            0x0D    /* Write Memory */
#define WRITE_REG               0xE0    /* Write Register */
#define READ_MEMORU             0x0E    /* Read Memory */
#define READ_REG                0xE1    /* Read Register */
#define EN_DIS_IDLE_SLEEP       0x11    /* Enable/Disable Idle/Sleep Mode */
#define SET_IDLE_SLEPP_TIM      0x12    /* Set Idle/Sleep Time Interval */
#endif  /*__LINUX_ITE7258_H*/

