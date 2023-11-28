/*
 * sample-common.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include <imp/imp_common.h>
#include <imp/imp_osd.h>
#include <imp/imp_framesource.h>
#include <imp/imp_isp.h>
#include <imp/imp_system.h>
#include <imp/imp_encoder.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/******************************************** Sensor Attribute Table *********************************************/
/* 		NAME		I2C_ADDR		RESOLUTION		Default_Boot			        							*/
/* 		gc2053		0x37 			1920*1080		0:25fps_dvp 1:15fps_dvp	2:25fps_mipi						*/
/* 		jxf35		0x40 			1920*1080		0:25fps_mipi 1:25fps_dvp 3:60fps_mipi						*/
/* 		jxf37		0x40 			1920*1080		0:25fps_dvp 1:25fps_mipi									*/
/* 		imx327		0x36 			1920*1080		0:25fps 1:25fps_2dol										*/
/* 		imx335		0x1a 			2592*1944		0:15fps 1:25fps												*/
/* 		imx415		0x1a 			3840*2160		0:15fps														*/
/* 		os04b10		0x3c 			2560*1440		0:25fps	1:20fps												*/
/* 		os05a10		0x36 			2592*1944		0:12fps	1:15fps 2:25fps 3:30fps 4:40fps						*/
/* 		os08a10		0x36 			3840*2160		0:15fps	1:25fps												*/
/* 		sc2335		0x30 			1920*1080		0:25fps														*/
/* 		sc3235		0x30 			2304*1296		0:25fps														*/
/* 		sc3335		0x30 			2304*1296		0:25fps_mipi 1:25fps_dvp									*/
/* 		sc4335		0x30 			2560*1440		0:25fps_mipi 												*/
/* 		sc8238		0x30 			3840*2160		0:15fps 1:25fps												*/
/******************************************** Sensor Attribute Table *********************************************/

/* first sensor */
#define FIRST_SNESOR_NAME 			"jxf37"			//sensor name (match with snesor driver name)
#define	FIRST_I2C_ADDR				0x40				//sensor i2c address
#define	FIRST_I2C_ADAPTER_ID		1					//sensor controller number used (0/1/2/3)
#define FIRST_SENSOR_WIDTH			1920				//sensor width
#define FIRST_SENSOR_HEIGHT			1080				//sensor height
#define	FIRST_RST_GPIO				GPIO_PC(27)			//sensor reset gpio
#define	FIRST_PWDN_GPIO				-1					//sensor pwdn gpio
#define	FIRST_POWER_GPIO			-1					//sensor power gpio
#define	FIRST_SENSOR_ID				0					//sensor index
#define	FIRST_VIDEO_INTERFACE		IMPISP_SENSOR_VI_MIPI_CSI0 	//sensor interface type (dvp/csi0/csi1)
#define	FIRST_MCLK					IMPISP_SENSOR_MCLK1		//sensor clk source (mclk0/mclk1/mclk2)
#define	FIRST_DEFAULT_BOOT			1					//sensor default mode(0/1/2/3/4)

/* second sensor */
#define SECOND_SNESOR_NAME 			"imx327s1"			//sensor name (match with snesor driver name)
#define	SECOND_I2C_ADDR				0x36				//sensor i2c address
#define	SECOND_I2C_ADAPTER_ID		3					//sensor controller number used (0/1/2/3)
#define SECOND_SENSOR_WIDTH			1920				//sensor width
#define SECOND_SENSOR_HEIGHT		1080				//sensor height
#define	SECOND_RST_GPIO				GPIO_PC(28)			//sensor reset gpio
#define	SECOND_PWDN_GPIO			-1					//sensor pwdn gpio
#define	SECOND_POWER_GPIO			-1					//sensor power gpio
#define	SECOND_SENSOR_ID			1					//sensor index
#define	SECOND_VIDEO_INTERFACE		IMPISP_SENSOR_VI_MIPI_CSI1	//sensor interface type (dvp/csi0/csi1)
#define	SECOND_MCLK					IMPISP_SENSOR_MCLK2		//sensor clk source (mclk0/mclk1/mclk2)
#define	SECOND_DEFAULT_BOOT			0					//sensor default mode(0/1/2/3/4)

/* third sensor */
#define THIRD_SNESOR_NAME 			"gc2053"			//sensor name (match with snesor driver name)
#define	THIRD_I2C_ADDR				0x37				//sensor i2c address
#define	THIRD_I2C_ADAPTER_ID		0					//sensor controller number used (0/1/2/3)
#define THIRD_SENSOR_WIDTH			1920				//sensor width
#define THIRD_SENSOR_HEIGHT			1080				//sensor height
#define	THIRD_RST_GPIO				GPIO_PC(17)			//sensor reset gpio
#define	THIRD_PWDN_GPIO				-1					//sensor pwdn gpio
#define	THIRD_POWER_GPIO			-1					//sensor power gpio
#define	THIRD_SENSOR_ID				2					//sensor index
#define	THIRD_VIDEO_INTERFACE		IMPISP_SENSOR_VI_DVP		//sensor interface type (dvp/csi0/csi1)
#define	THIRD_MCLK					IMPISP_SENSOR_MCLK0		//sensor clk source (mclk0/mclk1/mclk2)
#define	THIRD_DEFAULT_BOOT			0					//sensor default mode(0/1/2/3/4)


/* Multimedia Camera Mode */
#define SENSOR_NUM					IMPISP_TOTAL_ONE			//sensor total number (one/two/thr)
#define DUALSENSOR_MODE				IMPISP_DUALSENSOR_DUAL_ALLCACHED_MODE	//dualsensor mode ()
#define JOINT_MODE					IMPISP_NOT_JOINT			//joint mode ()

/* Channel_en Choose */
/* 1:enable 0:disable  */
/* first sensor output channel (0/1/2) */
/* second sensor output channel (3/4/5) */
/* third sensor output channel (6) */
/* dual sensor joint mode only channel (0/1/2) */
#define CHN0_EN                	1
#define CHN1_EN              	0
#define CHN2_EN                	0

#define CHN3_EN					0
#define CHN4_EN					0
#define CHN5_EN					0

#define CHN6_EN					0

/* Crop_en Choose */
#define FIRST_CROP_EN					0
#define SECOND_CROP_EN					0
#define THIRD_CROP_EN					0

#define FIRST_SENSOR_FRAME_RATE_NUM			25
#define FIRST_SENSOR_FRAME_RATE_DEN			1
#define SECOND_SENSOR_FRAME_RATE_NUM		25
#define SECOND_SENSOR_FRAME_RATE_DEN		1
#define THIRD_SENSOR_FRAME_RATE_NUM			25
#define THIRD_SENSOR_FRAME_RATE_DEN			1

#define FIRST_SENSOR_WIDTH_SECOND			640
#define FIRST_SENSOR_HEIGHT_SECOND			360
#define SECOND_SENSOR_WIDTH_SECOND			640
#define SECOND_SENSOR_HEIGHT_SECOND			360
#define THIRD_SENSOR_WIDTH_SECOND			640
#define THIRD_SENSOR_HEIGHT_SECOND			360

#define FIRST_SENSOR_WIDTH_THIRD			1280
#define FIRST_SENSOR_HEIGHT_THIRD			720
#define SECOND_SENSOR_WIDTH_THIRD			1280
#define SECOND_SENSOR_HEIGHT_THIRD			720
#define THIRD_SENSOR_WIDTH_THIRD			1280
#define THIRD_SENSOR_HEIGHT_THIRD			720


#define BITRATE_720P_Kbs        1000
#define NR_FRAMES_TO_SAVE		200
#define STREAM_BUFFER_SIZE		(1 * 1024 * 1024)

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define STREAM_FILE_PATH_PREFIX		"/tmp"
#define SNAP_FILE_PATH_PREFIX		"/tmp"

#define OSD_REGION_WIDTH		16
#define OSD_REGION_HEIGHT		34
#define OSD_REGION_WIDTH_SEC	8
#define OSD_REGION_HEIGHT_SEC   18


#define SLEEP_TIME			1

#define FS_CHN_NUM			7  //MIN 1,MAX 6
#define IVS_CHN_ID          1

#define CH0_INDEX  	0
#define CH1_INDEX  	1
#define CH2_INDEX  	2
#define CH3_INDEX  	3
#define CH4_INDEX  	4
#define CH5_INDEX  	5
#define CH6_INDEX  	6

#define CHN_ENABLE 		1
#define CHN_DISABLE 	0

/*#define SUPPORT_RGB555LE*/

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
	IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
};

typedef struct {
	uint8_t *streamAddr;
	int streamLen;
}streamInfo;

typedef struct {
	IMPEncoderEncType type;
	IMPEncoderRcMode mode;
	uint16_t frameRate;
	uint16_t gopLength;
	uint32_t targetBitrate;
	uint32_t maxBitrate;
	int16_t initQp;
	int16_t minQp;
	int16_t maxQp;
	uint32_t maxPictureSize;
} encInfo;

#define  CHN_NUM  ARRAY_SIZE(chn)

int sample_system_init();
int sample_system_exit();

int sample_framesource_streamon();
int sample_framesource_streamoff();

int sample_framesource_ext_hsv_streamon();
int sample_framesource_ext_hsv_streamoff();

int sample_framesource_ext_bgra_streamon();
int sample_framesource_ext_bgra_streamoff();

int sample_framesource_init();
int sample_framesource_exit();

int sample_framesource_ext_hsv_init();
int sample_framesource_ext_hsv_exit();

int sample_framesource_ext_bgra_init();
int sample_framesource_ext_bgra_exit();

int sample_encoder_init();
int sample_jpeg_init();
int sample_encoder_exit(void);

IMPRgnHandle *sample_osd_init(int grpNum);
int sample_osd_exit(IMPRgnHandle *prHandle,int grpNum);

int sample_get_frame();
int sample_get_video_stream();
int sample_get_video_stream_byfd();
int sample_get_jpeg_snap();

int sample_SetIRCUT(int enable);
void *sample_soft_photosensitive_ctrl(IMPVI_NUM vinum, void *p);
int sample_framesource_i2dopr(void);
int sample_framesource_i2dpreinit(void);
int sample_mainpro_getframeEx(void);
int sample_ipc_getframeEx(void);

int alcodec_encyuv_init(void **h, int picWidth, int picHight, void *info);
int alcodec_encyuv_encode(void *h, IMPFrameInfo frame, streamInfo *stream);
int alcodec_encyuv_deinit(void *h);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
