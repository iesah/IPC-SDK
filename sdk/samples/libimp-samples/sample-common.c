/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>
#include "logodata_100x100_bgra.h"

#include "sample-common.h"

#define TAG "Sample-Common"

static const IMPEncoderRcMode S_RC_METHOD = IMP_ENC_RC_MODE_CBR;

//#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
		.payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
            .i2dattr.i2d_enable = 0,
            .i2dattr.flip_enable = 0,
            .i2dattr.mirr_enable = 0,
            .i2dattr.rotate_enable = 1,
            .i2dattr.rotate_angle = 270,

			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.scaler.enable = 0,
			.scaler.outwidth = FIRST_SENSOR_WIDTH,
			.scaler.outheight = FIRST_SENSOR_HEIGHT,

            .crop.enable = FIRST_CROP_EN,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = FIRST_SENSOR_WIDTH,
			.crop.height = FIRST_SENSOR_HEIGHT,

			.picWidth = FIRST_SENSOR_WIDTH,
			.picHeight = FIRST_SENSOR_HEIGHT,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		.enable = CHN1_EN,
        .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
            .i2dattr.i2d_enable = 0,
            .i2dattr.flip_enable = 0,
            .i2dattr.mirr_enable = 0,
            .i2dattr.rotate_enable = 1,
            .i2dattr.rotate_angle = 270,

			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.scaler.enable = 1,
			.scaler.outwidth = FIRST_SENSOR_WIDTH_SECOND,
			.scaler.outheight = FIRST_SENSOR_HEIGHT_SECOND,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = FIRST_SENSOR_WIDTH_SECOND,
			.crop.height = FIRST_SENSOR_HEIGHT_SECOND,

			.picWidth = FIRST_SENSOR_WIDTH_SECOND,
			.picHeight = FIRST_SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH1_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH1_INDEX, 0},
	},
	{
		.index = CH2_INDEX,
		.enable = CHN2_EN,
        .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
            .i2dattr.i2d_enable = 0,
            .i2dattr.flip_enable = 0,
            .i2dattr.mirr_enable = 0,
            .i2dattr.rotate_enable = 1,
            .i2dattr.rotate_angle = 270,

			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.scaler.enable = 1,
			.scaler.outwidth = FIRST_SENSOR_WIDTH_THIRD,
			.scaler.outheight = FIRST_SENSOR_HEIGHT_THIRD,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = FIRST_SENSOR_WIDTH_THIRD,
			.crop.height = FIRST_SENSOR_HEIGHT_THIRD,

			.picWidth = FIRST_SENSOR_WIDTH_THIRD,
			.picHeight = FIRST_SENSOR_HEIGHT_THIRD,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH2_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH2_INDEX, 0},
	},
	{
		.index = CH3_INDEX,
		.enable = CHN3_EN,
        .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
            .i2dattr.i2d_enable = 0,
            .i2dattr.flip_enable = 0,
            .i2dattr.mirr_enable = 0,
            .i2dattr.rotate_enable = 1,
            .i2dattr.rotate_angle = 270,

            .pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SECOND_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SECOND_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = SECOND_CROP_EN,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SECOND_SENSOR_WIDTH,
			.crop.height = SECOND_SENSOR_HEIGHT,

			.scaler.enable = 0,

			.picWidth = SECOND_SENSOR_WIDTH,
			.picHeight = SECOND_SENSOR_HEIGHT,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH3_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH3_INDEX, 0},
	},
	{
		.index = CH4_INDEX,
		.enable = CHN4_EN,
        .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
            .i2dattr.i2d_enable = 0,
            .i2dattr.flip_enable = 0,
            .i2dattr.mirr_enable = 0,
            .i2dattr.rotate_enable = 1,
            .i2dattr.rotate_angle = 270,

			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SECOND_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SECOND_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SECOND_SENSOR_WIDTH,
			.crop.height = SECOND_SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SECOND_SENSOR_WIDTH_THIRD,
			.scaler.outheight = SECOND_SENSOR_HEIGHT_THIRD,

			.picWidth = SECOND_SENSOR_WIDTH_THIRD,
			.picHeight = SECOND_SENSOR_HEIGHT_THIRD,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH4_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH4_INDEX, 0},
	},
	{
		.index = CH5_INDEX,
		.enable = CHN5_EN,
        .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
            .i2dattr.i2d_enable = 0,
            .i2dattr.flip_enable = 0,
            .i2dattr.mirr_enable = 0,
            .i2dattr.rotate_enable = 1,
            .i2dattr.rotate_angle = 270,

			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SECOND_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SECOND_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SECOND_SENSOR_WIDTH,
			.crop.height = SECOND_SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SECOND_SENSOR_WIDTH_SECOND,
			.scaler.outheight = SECOND_SENSOR_HEIGHT_SECOND,

			.picWidth = SECOND_SENSOR_WIDTH_SECOND,
			.picHeight = SECOND_SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH5_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH5_INDEX, 0},
	},
	{
		.index = CH6_INDEX,
		.enable = CHN6_EN,
        .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_RAW,
			.outFrmRateNum = THIRD_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = THIRD_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = THIRD_SENSOR_WIDTH,
			.crop.height = THIRD_SENSOR_HEIGHT,

			.scaler.enable = 0,
			.scaler.outwidth = THIRD_SENSOR_WIDTH,
			.scaler.outheight = THIRD_SENSOR_HEIGHT,

			.picWidth = THIRD_SENSOR_WIDTH,
			.picHeight = THIRD_SENSOR_HEIGHT,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH6_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH6_INDEX, 0},
	},
};

IMPSensorInfo Def_Sensor_Info[3] = {
	{
		FIRST_SNESOR_NAME,
		TX_SENSOR_CONTROL_INTERFACE_I2C,
		{FIRST_SNESOR_NAME, FIRST_I2C_ADDR, FIRST_I2C_ADAPTER_ID},
		FIRST_RST_GPIO,
		FIRST_PWDN_GPIO,
		FIRST_POWER_GPIO,
		FIRST_SENSOR_ID,
		FIRST_VIDEO_INTERFACE,
		FIRST_MCLK,
		FIRST_DEFAULT_BOOT
	},
	{
		SECOND_SNESOR_NAME,
		TX_SENSOR_CONTROL_INTERFACE_I2C,
		{SECOND_SNESOR_NAME, SECOND_I2C_ADDR, SECOND_I2C_ADAPTER_ID},
		SECOND_RST_GPIO,
		SECOND_PWDN_GPIO,
		SECOND_POWER_GPIO,
		SECOND_SENSOR_ID,
		SECOND_VIDEO_INTERFACE,
		SECOND_MCLK,
		SECOND_DEFAULT_BOOT
	},
	{
		THIRD_SNESOR_NAME,
		TX_SENSOR_CONTROL_INTERFACE_I2C,
		{THIRD_SNESOR_NAME, THIRD_I2C_ADDR, THIRD_I2C_ADAPTER_ID},
		THIRD_RST_GPIO,
		THIRD_PWDN_GPIO,
		THIRD_POWER_GPIO,
		THIRD_SENSOR_ID,
		THIRD_VIDEO_INTERFACE,
		THIRD_MCLK,
		THIRD_DEFAULT_BOOT
	},
};

IMPSensorInfo sensor_info[3];
IMPISPCameraInputMode mode = {
	.sensor_num = SENSOR_NUM,
	.dual_mode = DUALSENSOR_MODE,
	.dual_mode_switch = {
		.en = 0,
	},
	.joint_mode = IMPISP_NOT_JOINT,
};

int sensor_bypass[3] = {0, 0, 1};

int sample_system_init()
{
	int i = 0;
	int ret = 0;

	//IMP_OSD_SetPoolSize(512*1024);
	/*分配一些内存供ISPOSD使用*/
	IMP_ISP_Tuning_SetOsdPoolSize(512*1024);

	memset(&sensor_info, 0, sizeof(sensor_info));
	if(mode.sensor_num == IMPISP_TOTAL_ONE){
		memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo));
	} else if(mode.sensor_num == IMPISP_TOTAL_TWO){
		memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo) * 2);
	}else if(mode.sensor_num ==IMPISP_TOTAL_THR){
		memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo) * 3);
	}	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	ret = IMP_ISP_Open();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	for(i = 0; i < mode.sensor_num; i++) {
		int index;
		IMPISPTuningOpsMode bypass_en;

		if(!sensor_bypass[i])
			continue;

		index = i * 3;
		/* chn[index].fs_chn_attr.pixFmt = PIX_FMT_RAW; */
		chn[index].fs_chn_attr.nrVBs = 2;
		bypass_en = IMPISP_TUNING_OPS_MODE_ENABLE;

		ret = IMP_ISP_Tuning_SetISPBypass(i, &bypass_en);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_ISP_Tuning_SetISPBpass failed\n", __func__, __LINE__);
			return -1;
		}
	}

	/* set dual sensor mode */
	if(mode.sensor_num > IMPISP_TOTAL_ONE){
		ret = IMP_ISP_SetCameraInputMode(&mode);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to camera input mode!\n");
			return -1;
		}
	}

	/* add sensor */
	ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info[0]);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	if(mode.sensor_num > IMPISP_TOTAL_ONE){
		ret = IMP_ISP_AddSensor(IMPVI_SEC, &sensor_info[1]);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to AddSensor\n");
			return -1;
		}
	}

	if(mode.sensor_num > IMPISP_TOTAL_TWO){
		ret = IMP_ISP_AddSensor(IMPVI_THR, &sensor_info[2]);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to AddSensor\n");
			return -1;
		}
	}

	/* enable sensor */
	ret = IMP_ISP_EnableSensor(IMPVI_MAIN, &sensor_info[0]);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	if(mode.sensor_num > IMPISP_TOTAL_ONE){
		ret = IMP_ISP_EnableSensor(IMPVI_SEC, &sensor_info[1]);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}
	}

	if(mode.sensor_num > IMPISP_TOTAL_TWO){
		ret = IMP_ISP_EnableSensor(IMPVI_THR, &sensor_info[2]);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}

	}

	ret = IMP_System_Init();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}
	unsigned char value = 128;
	IMP_ISP_Tuning_SetContrast(IMPVI_MAIN, &value);
	IMP_ISP_Tuning_SetSharpness(IMPVI_MAIN, &value);
	IMP_ISP_Tuning_SetSaturation(IMPVI_MAIN, &value);
	IMP_ISP_Tuning_SetBrightness(IMPVI_MAIN, &value);

	if(mode.sensor_num > IMPISP_TOTAL_ONE){
		unsigned char value = 128;
		IMP_ISP_Tuning_SetContrast(IMPVI_SEC, &value);
		IMP_ISP_Tuning_SetSharpness(IMPVI_SEC, &value);
		IMP_ISP_Tuning_SetSaturation(IMPVI_SEC, &value);
		IMP_ISP_Tuning_SetBrightness(IMPVI_SEC, &value);
	}

#if 1
	IMPISPRunningMode dn = IMPISP_RUNNING_MODE_DAY;
	ret = IMP_ISP_Tuning_SetISPRunningMode(IMPVI_MAIN, &dn);
	if (ret < 0){
		IMP_LOG_ERR(TAG, "failed to set running mode\n");
		return -1;
	}

	if(mode.sensor_num > IMPISP_TOTAL_ONE){
		ret = IMP_ISP_Tuning_SetISPRunningMode(IMPVI_SEC, &dn);
		if (ret < 0){
			IMP_LOG_ERR(TAG, "failed to set running mode\n");
			return -1;
		}
	}
#endif

#if 0
	uint32_t fps_num = SENSOR_FRAME_RATE_NUM;
	uint32_t fps_den = SENSOR_FRAME_RATE_DEN;
	ret = IMP_ISP_Tuning_SetSensorFPS(0, &fps_num, &fps_den);
    if (ret < 0){
        IMP_LOG_ERR(TAG, "failed to set sensor fps\n");
        return -1;
    }
#endif

	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int i = 0;
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");


	IMP_System_Exit();

	for(i = 0; i < mode.sensor_num; i++) {
		IMPISPTuningOpsMode bypass_en;

		if(!sensor_bypass[i])
			continue;

		bypass_en = IMPISP_TUNING_OPS_MODE_DISABLE;
		ret = IMP_ISP_Tuning_SetISPBypass(i, &bypass_en);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_ISP_Tuning_SetISPBpass failed\n", __func__, __LINE__);
			return -1;
		}
	}

	/* disable sensor */
	ret = IMP_ISP_DisableSensor(IMPVI_MAIN);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	if(mode.sensor_num > IMPISP_TOTAL_ONE){
		ret = IMP_ISP_DisableSensor(IMPVI_SEC);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}
	}

	if(mode.sensor_num > IMPISP_TOTAL_TWO){
		ret = IMP_ISP_DisableSensor(IMPVI_THR);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}
	}

	/* delete sensor */
	ret = IMP_ISP_DelSensor(IMPVI_MAIN, &sensor_info[0]);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	if(mode.sensor_num > IMPISP_TOTAL_ONE){
		ret = IMP_ISP_DelSensor(IMPVI_SEC, &sensor_info[1]);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to AddSensor\n");
			return -1;
		}
	}

	if(mode.sensor_num > IMPISP_TOTAL_TWO){
		ret = IMP_ISP_DelSensor(IMPVI_THR, &sensor_info[2]);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to AddSensor\n");
			return -1;
		}
	}

	/* disable turning */
	ret = IMP_ISP_DisableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	if(IMP_ISP_Close()){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " sample_system_exit success\n");

	return 0;
}

int sample_framesource_streamon()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_EnableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}

int sample_framesource_streamoff()
{
	int ret = 0, i = 0;
	/* Disable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable){
			ret = IMP_FrameSource_DisableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}

static void *get_frame(void *args)
{
	int index = (int)args;
	int chnNum = chn[index].index;
	int i = 0, ret = 0;
	IMPFrameInfo *frame = NULL;
	char framefilename[64];
	int fd = -1;

	if (PIX_FMT_NV12 == chn[index].fs_chn_attr.pixFmt) {
		sprintf(framefilename, "frame%dx%d_%d.nv12", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight,index);
	} else {
		sprintf(framefilename, "frame%dx%d_%d.raw", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight,index);
	}

	fd = open(framefilename, O_RDWR | O_CREAT, 0x644);
	if (fd < 0) {
		IMP_LOG_ERR(TAG, "open %s failed:%s\n", framefilename, strerror(errno));
		goto err_open_framefilename;
	}

	ret = IMP_FrameSource_SetFrameDepth(chnNum, chn[index].fs_chn_attr.nrVBs * 2);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetFrameDepth(%d,%d) failed\n", chnNum, chn[index].fs_chn_attr.nrVBs * 2);
		goto err_IMP_FrameSource_SetFrameDepth_1;
	}

	for (i = 0; i < NR_FRAMES_TO_SAVE; i++) {
		printf("IMP_FrameSource_GetFrame(%d) i=%d\n",chnNum,i);
		ret = IMP_FrameSource_GetFrame(chnNum, &frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrame(%d) i=%d failed\n", chnNum, i);
			goto err_IMP_FrameSource_GetFrame_i;
		}

		if (NR_FRAMES_TO_SAVE/2 == i) {
			if (write(fd, (void *)frame->virAddr, frame->size) != frame->size) {
				IMP_LOG_ERR(TAG, "chnNum=%d write frame i=%d failed\n", chnNum, i);
				goto err_write_frame;
			}
		}
		ret = IMP_FrameSource_ReleaseFrame(chnNum, frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_ReleaseFrame(%d) i=%d failed\n", chnNum, i);
			goto err_IMP_FrameSource_ReleaseFrame_i;
		}
	}

	IMP_FrameSource_SetFrameDepth(chnNum, 0);

	close(fd);
	return (void *)0;

err_IMP_FrameSource_ReleaseFrame_i:
err_write_frame:
	IMP_FrameSource_ReleaseFrame(chnNum, frame);
err_IMP_FrameSource_GetFrame_i:
	goto err_IMP_FrameSource_SetFrameDepth_1;
	IMP_FrameSource_SetFrameDepth(chnNum, 0);
err_IMP_FrameSource_SetFrameDepth_1:
	close(fd);
err_open_framefilename:
	return (void *)-1;
}

int sample_get_frame()
{
	unsigned int i;
	int ret;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = pthread_create(&tid[i], NULL, get_frame, (void *)i);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Create ChnNum%d get_frame failed\n", chn[i].index);
				return -1;
			}
		}
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			pthread_join(tid[i],NULL);
		}
	}

	return 0;
}

static void *get_frameEx(void *args)
{
	int index = (int)args;
	int chnNum = chn[index].index;
	int i = 0, ret = 0;
	IMPFrameInfo *frame = NULL;
	char framefilename[64];
	int fd = -1;

	if (PIX_FMT_NV12 == chn[index].fs_chn_attr.pixFmt) {
		sprintf(framefilename, "frame%dx%d_%d.nv12", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight,index);
	} else {
		sprintf(framefilename, "frame%dx%d_%d.raw", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight,index);
	}

	fd = open(framefilename, O_RDWR | O_CREAT, 0x644);
	if (fd < 0) {
		IMP_LOG_ERR(TAG, "open %s failed:%s\n", framefilename, strerror(errno));
		return NULL;
	}

	for (i = 0; i < NR_FRAMES_TO_SAVE; i++) {
		printf("IMP_FrameSource_GetFrameEx%d i=%d\n",chnNum,i);
		ret = IMP_FrameSource_GetFrameEx(chnNum, &frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrameEx(%d) i=%d failed\n", chnNum, i);
			return NULL;
		}
#if 0
		if (NR_FRAMES_TO_SAVE/2 == i) {
			 write(fd, (void *)frame->virAddr, frame->width * frame->height);
			 write(fd, (void *)frame->virAddr + frame->width * ((frame->height + 15) & ~15),frame->width * frame->height / 2);
		}
#endif

		ret = IMP_FrameSource_ReleaseFrameEx(chnNum, frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_ReleaseFrameEx(%d) i=%d failed\n", chnNum, i);
			return NULL;
		}

	}

	close(fd);
	return (void *)0;
}

int sample_mainpro_getframeEx(void)
{
	unsigned int i;
	int ret;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = pthread_create(&tid[i], NULL, get_frameEx, (void *)i);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Create ChnNum%d get_frame failed\n", chn[i].index);
				return -1;
			}
		}
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			pthread_join(tid[i],NULL);
		}
	}

	return 0;
}

int sample_framesource_init()
{
	int i, ret;

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
			if(ret < 0){
				IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
				return -1;
			}

			ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n",  chn[i].index);
				return -1;
			}
		}
	}

	return 0;
}

int sample_framesource_exit()
{
	int ret,i;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			/*Destroy channel */
			ret = IMP_FrameSource_DestroyChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
				return -1;
			}
		}
	}
	return 0;
}

int sample_jpeg_init()
{
	int i, ret;
	IMPEncoderChnAttr channel_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			imp_chn_attr_tmp = &chn[i].fs_chn_attr;
			memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
			ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
					imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
					imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, 25, 0);

			/* Create Channel */
			ret = IMP_Encoder_CreateChn(4 + chn[i].index, &channel_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}

			/* Resigter Channel */
			ret = IMP_Encoder_RegisterChn(i, 4 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}
		}
	}

	return 0;
}

int sample_encoder_init()
{
	int i, ret, chnNum = 0;
    int s32picWidth = 0,s32picHeight = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderChnAttr channel_attr;
    IMPFSI2DAttr sti2dattr;
    for (i = 0; i <  FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            imp_chn_attr_tmp = &chn[i].fs_chn_attr;
            chnNum = chn[i].index;

            memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
            memset(&sti2dattr,0,sizeof(IMPFSI2DAttr));

            ret = IMP_FrameSource_GetI2dAttr(chn[i].index,&sti2dattr);
            if(ret < 0){
                IMP_LOG_ERR(TAG, "IMP_FrameSource_GetI2dAttr(%d) error !\n", chn[i].index);
                return -1;
            }

            if((1 == sti2dattr.i2d_enable) &&
                ((sti2dattr.rotate_enable) && (sti2dattr.rotate_angle == 90 || sti2dattr.rotate_angle == 270))){
				s32picWidth = (chn[i].fs_chn_attr.picHeight);/*this depend on your sensor or channels*/
				s32picHeight = (chn[i].fs_chn_attr.picWidth);

            }else{
                s32picWidth = chn[i].fs_chn_attr.picWidth;
                s32picHeight =chn[i].fs_chn_attr.picHeight;
            }
			float ratio = 1;
			if (((uint64_t)s32picWidth * s32picHeight) > (1280 * 720)) {
				ratio = log10f(((uint64_t)s32picWidth * s32picHeight) / (1280 * 720.0)) + 1;
			} else {
				ratio = 1.0 / (log10f((1280 * 720.0) / ((uint64_t)s32picWidth * s32picHeight)) + 1);
			}
			ratio = ratio > 0.1 ? ratio : 0.1;
			unsigned int uTargetBitRate = BITRATE_720P_Kbs * ratio;
			printf("rcMode:%d.\n", S_RC_METHOD);
            ret = IMP_Encoder_SetDefaultParam(&channel_attr, chn[i].payloadType, S_RC_METHOD,
                    s32picWidth, s32picHeight,
                    imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
                    imp_chn_attr_tmp->outFrmRateNum * 2 / imp_chn_attr_tmp->outFrmRateDen, 2,
                    (S_RC_METHOD == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
                    uTargetBitRate);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_SetDefaultParam(%d) error !\n", chnNum);
                return -1;
            }
#ifdef LOW_BITSTREAM
			IMPEncoderRcAttr *rcAttr = &channel_attr.rcAttr;
			uTargetBitRate /= 2;

			switch (rcAttr->attrRcMode.rcMode) {
				case IMP_ENC_RC_MODE_FIXQP:
					rcAttr->attrRcMode.attrFixQp.iInitialQP = 38;
					break;
				case IMP_ENC_RC_MODE_CBR:
					rcAttr->attrRcMode.attrCbr.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrCbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrCbr.iMinQP = 34;
					rcAttr->attrRcMode.attrCbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrCbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrCbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrCbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
					break;
				case IMP_ENC_RC_MODE_VBR:
					rcAttr->attrRcMode.attrVbr.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrVbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrVbr.iMinQP = 34;
					rcAttr->attrRcMode.attrVbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrVbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrVbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
					break;
				case IMP_ENC_RC_MODE_CAPPED_VBR:
					rcAttr->attrRcMode.attrCappedVbr.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrCappedVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedVbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrCappedVbr.iMinQP = 34;
					rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrCappedVbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrCappedVbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCappedVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
					break;
				case IMP_ENC_RC_MODE_CAPPED_QUALITY:
					rcAttr->attrRcMode.attrCappedQuality.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrCappedQuality.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedQuality.iInitialQP = -1;
					rcAttr->attrRcMode.attrCappedQuality.iMinQP = 34;
					rcAttr->attrRcMode.attrCappedQuality.iMaxQP = 51;
					rcAttr->attrRcMode.attrCappedQuality.iIPDelta = -1;
					rcAttr->attrRcMode.attrCappedQuality.iPBDelta = -1;
					rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
					break;
				case IMP_ENC_RC_MODE_INVALID:
					IMP_LOG_ERR(TAG, "unsupported rcmode:%d, we only support fixqp, cbr vbr and capped vbr\n", rcAttr->attrRcMode.rcMode);
					return -1;
			}
#endif

            ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", chnNum);
                return -1;
            }

			ret = IMP_Encoder_RegisterChn(chn[i].index, chnNum);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chn[i].index, chnNum, ret);
				return -1;
			}
		}
	}

	return 0;
}

int sample_jpeg_exit(void)
{
	int ret = 0, i = 0, chnNum = 0;
	IMPEncoderChnStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			chnNum = 4 + chn[i].index;
			memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
			ret = IMP_Encoder_Query(chnNum, &chn_stat);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
				return -1;
			}

			if (chn_stat.registered) {
				ret = IMP_Encoder_UnRegisterChn(chnNum);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
					return -1;
				}

				ret = IMP_Encoder_DestroyChn(chnNum);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
					return -1;
				}
			}
		}
	}

	return 0;
}


int sample_encoder_exit(void)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderChnStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
            chnNum = chn[i].index;
            memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
            ret = IMP_Encoder_Query(chnNum, &chn_stat);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
                return -1;
            }

			if (chn_stat.registered) {
				ret = IMP_Encoder_UnRegisterChn(chnNum);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
					return -1;
				}

				ret = IMP_Encoder_DestroyChn(chnNum);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
					return -1;
				}

                ret = IMP_Encoder_DestroyGroup(chnNum);
				if (ret < 0) {
				    IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyGroup(%d) error: %d\n", chnNum, ret);
					return -1;
                }

			}
		}
	}

	return 0;
}
IMPRgnHandle *sample_osd_init(int grpNum)
{
	int ret = 0;
	IMPRgnHandle *prHander = NULL;
	IMPRgnHandle rHanderFont = 0;
	IMPRgnHandle rHanderLogo = 0;
	IMPRgnHandle rHanderCover = 0;
	IMPRgnHandle rHanderRect = 0;
	IMPRgnHandle rHanderLine = 0;
	IMPRgnHandle rHanderMosaic = 0;

	prHander = malloc(6 * sizeof(IMPRgnHandle));
	if (prHander <= 0) {
		IMP_LOG_ERR(TAG, "malloc() error !\n");
		return NULL;
	}

	rHanderFont = IMP_OSD_CreateRgn(NULL);
	if (rHanderFont == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn TimeStamp error !\n");
		return NULL;
	}
    //query osd rgn create status
    IMPOSDRgnCreateStat stStatus;
    memset(&stStatus,0x0,sizeof(IMPOSDRgnCreateStat));
    ret = IMP_OSD_RgnCreate_Query(rHanderFont,&stStatus);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "IMP_OSD_RgnCreate_Query error !\n");
        return NULL;
    }

	rHanderLogo = IMP_OSD_CreateRgn(NULL);
	if (rHanderLogo == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Logo error !\n");
		return NULL;
	}

	rHanderCover = IMP_OSD_CreateRgn(NULL);
	if (rHanderCover == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Cover error !\n");
		return NULL;
	}

	rHanderRect = IMP_OSD_CreateRgn(NULL);
	if (rHanderRect == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Rect error !\n");
		return NULL;
	}
	rHanderLine = IMP_OSD_CreateRgn(NULL);
	if (rHanderLine == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Line error !\n");
		return NULL;
	}
	rHanderMosaic = IMP_OSD_CreateRgn(NULL);
	if (rHanderMosaic == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Line error !\n");
		return NULL;
	}
	ret = IMP_OSD_RegisterRgn(rHanderFont, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

    //query osd rgn register status
    IMPOSDRgnRegisterStat stRigStatus;
    memset(&stRigStatus,0x0,sizeof(IMPOSDRgnRegisterStat));
    ret = IMP_OSD_RgnRegister_Query(rHanderFont, grpNum,&stRigStatus);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_RgnRegister_Query failed\n");
        return NULL;
    }
	ret = IMP_OSD_RegisterRgn(rHanderLogo, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}
	ret = IMP_OSD_RegisterRgn(rHanderCover, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}
	ret = IMP_OSD_RegisterRgn(rHanderRect, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}
	ret = IMP_OSD_RegisterRgn(rHanderLine, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}
	ret = IMP_OSD_RegisterRgn(rHanderMosaic, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrFont;
	memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));

	rAttrFont.type = OSD_REG_BITMAP;
	rAttrFont.rect.p0.x = 0;
	rAttrFont.rect.p0.y = 0;
	rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + 20 * 32 - 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + 33 - 1;
#ifdef SUPPORT_COLOR_REVERSE
	rAttrFont.fontData.invertColorSwitch = 1;	//Color reverse switch 0-close 1-open.
	rAttrFont.fontData.luminance = 200;
	rAttrFont.fontData.data.fontWidth = 16;
	rAttrFont.fontData.data.fontHeight = 33;
	rAttrFont.fontData.length = 19;
	rAttrFont.fontData.istimestamp = 1;
#endif
#ifdef SUPPORT_RGB555LE
	rAttrFont.fmt = PIX_FMT_RGB555LE;
#else
	rAttrFont.fmt = PIX_FMT_MONOWHITE;
#endif
	rAttrFont.data.bitmapData = valloc(20 * 32 * 33);
	if (rAttrFont.data.bitmapData == NULL) {
		IMP_LOG_ERR(TAG, "alloc rAttr.data.bitmapData TimeStamp error !\n");
		return NULL;
	}
	memset(rAttrFont.data.bitmapData, 0, 20 * 32 * 33);

	ret = IMP_OSD_SetRgnAttr(rHanderFont, &rAttrFont);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr TimeStamp error !\n");
		return NULL;
	}

	IMPOSDGrpRgnAttr grAttrFont;

	if (IMP_OSD_GetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return NULL;

	}
	memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrFont.show = 0;

	/* Disable Font global alpha, only use pixel alpha. */
	grAttrFont.gAlphaEn = 1;
	grAttrFont.fgAlhpa = 0xff;
	grAttrFont.layer = 3;
	if (IMP_OSD_SetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return NULL;
	}
	IMPOSDRgnAttr rAttrLogo;
	memset(&rAttrLogo, 0, sizeof(IMPOSDRgnAttr));
	int picw = 100;
	int pich = 100;
	rAttrLogo.type = OSD_REG_PIC;
	rAttrLogo.rect.p0.x = 0;
	rAttrLogo.rect.p0.y = 400;

    rAttrLogo.rect.p1.x = rAttrLogo.rect.p0.x+picw-1;	 //p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrLogo.rect.p1.y = rAttrLogo.rect.p0.y+pich-1;
	rAttrLogo.fmt = PIX_FMT_BGRA;
	rAttrLogo.data.picData.pData = logodata_100x100_bgra;
	ret = IMP_OSD_SetRgnAttr(rHanderLogo, &rAttrLogo);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Logo error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrLogo;

	if (IMP_OSD_GetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return NULL;

	}
	memset(&grAttrLogo, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrLogo.show = 0;

	/* Set Logo global alpha to 0x7f, it is semi-transparent. */
	grAttrLogo.gAlphaEn = 1;
	grAttrLogo.fgAlhpa = 0x7f;
	grAttrLogo.layer = 2;

	if (IMP_OSD_SetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrCover;
	memset(&rAttrCover, 0, sizeof(IMPOSDRgnAttr));
	rAttrCover.type = OSD_REG_COVER;
	rAttrCover.rect.p0.x = 100;
	rAttrCover.rect.p0.y = 100;
	rAttrCover.rect.p1.x = rAttrCover.rect.p0.x+300 -1;
	rAttrCover.rect.p1.y = rAttrCover.rect.p0.y+300 -1 ;
    rAttrCover.fmt = PIX_FMT_BGRA;
	rAttrCover.data.coverData.color = OSD_IPU_RED;
	ret = IMP_OSD_SetRgnAttr(rHanderCover, &rAttrCover);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Cover error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrCover;

	if (IMP_OSD_GetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Cover error !\n");
		return NULL;

	}
	memset(&grAttrCover, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrCover.show = 0;

	/* Disable Cover global alpha, it is absolutely no transparent. */
	grAttrCover.gAlphaEn = 1;
	grAttrCover.fgAlhpa = 0x7f;
	grAttrCover.layer = 2;
	if (IMP_OSD_SetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Cover error !\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrRect;
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));

	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = 400;
	rAttrRect.rect.p0.y = 400;
	rAttrRect.rect.p1.x = rAttrRect.rect.p0.x + 300 - 1;
	rAttrRect.rect.p1.y = rAttrRect.rect.p0.y + 300 - 1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
	rAttrRect.data.lineRectData.color = OSD_YELLOW;
	rAttrRect.data.lineRectData.linewidth = 5;
	ret = IMP_OSD_SetRgnAttr(rHanderRect, &rAttrRect);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Rect error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrRect;

	if (IMP_OSD_GetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Rect error !\n");
		return NULL;

	}
	memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrRect.show = 0;
	grAttrRect.layer = 1;
	grAttrRect.scalex = 1;
	grAttrRect.scaley = 1;
	if (IMP_OSD_SetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Rect error !\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrLine;
	memset(&rAttrLine, 0, sizeof(IMPOSDRgnAttr));

	rAttrLine.type = OSD_REG_HORIZONTAL_LINE;
	rAttrLine.line.p0.x = 800;
	rAttrLine.line.p0.y = 800;
	rAttrLine.data.lineRectData.color = OSD_RED;//4 line
	rAttrLine.data.lineRectData.linewidth = 5;
	rAttrLine.data.lineRectData.linelength = 200;

	ret = IMP_OSD_SetRgnAttr(rHanderLine, &rAttrLine);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Line error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrLine;

	if (IMP_OSD_GetGrpRgnAttr(rHanderLine, grpNum, &grAttrLine) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Line error !\n");
		return NULL;

	}
	memset(&grAttrLine, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrLine.show = 0;
	grAttrLine.layer = 1;
	grAttrLine.scalex = 1;
	grAttrLine.scaley = 1;
	if (IMP_OSD_SetGrpRgnAttr(rHanderLine, grpNum, &grAttrLine) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Line error !\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrMosaic;
	memset(&rAttrMosaic, 0, sizeof(IMPOSDRgnAttr));

	rAttrMosaic.type = OSD_REG_MOSAIC;
	rAttrMosaic.mosaicAttr.x = 1000;
	rAttrMosaic.mosaicAttr.y = 100;
	rAttrMosaic.mosaicAttr.mosaic_width = 200;
	rAttrMosaic.mosaicAttr.mosaic_height = 200;
	rAttrMosaic.mosaicAttr.frame_width = FIRST_SENSOR_WIDTH;
	rAttrMosaic.mosaicAttr.frame_height = FIRST_SENSOR_HEIGHT;
	rAttrMosaic.mosaicAttr.mosaic_min_size = 64;

	ret = IMP_OSD_SetRgnAttr(rHanderMosaic, &rAttrMosaic);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Line error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrMosaic;

	if (IMP_OSD_GetGrpRgnAttr(rHanderMosaic, grpNum, &grAttrMosaic) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Line error !\n");
		return NULL;

	}
	memset(&grAttrMosaic, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrMosaic.show = 0;
	grAttrMosaic.layer = 1;
	grAttrMosaic.scalex = 1;
	grAttrMosaic.scaley = 1;
	if (IMP_OSD_SetGrpRgnAttr(rHanderMosaic, grpNum, &grAttrMosaic) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Line error !\n");
		return NULL;
	}

	ret = IMP_OSD_Start(grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_Start TimeStamp, Logo, Cover and Rect error !\n");
		return NULL;
	}

	prHander[0] = rHanderFont;
	prHander[1] = rHanderLogo;
	prHander[2] = rHanderCover;
	prHander[3] = rHanderRect;
	prHander[4] = rHanderLine;
	prHander[5] = rHanderMosaic;
	return prHander;
}

int sample_osd_exit(IMPRgnHandle *prHander,int grpNum)
{
	int ret;

	ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close timeStamp error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[1], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Logo error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[2], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close cover error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[3], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Rect error\n");
	}


	ret = IMP_OSD_UnRegisterRgn(prHander[0], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn timeStamp error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[1], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn logo error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[2], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Cover error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[3], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Rect error\n");
	}


	IMP_OSD_DestroyRgn(prHander[0]);
	IMP_OSD_DestroyRgn(prHander[1]);
	IMP_OSD_DestroyRgn(prHander[2]);
	IMP_OSD_DestroyRgn(prHander[3]);

	ret = IMP_OSD_DestroyGroup(grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_DestroyGroup(0) error\n");
		return -1;
	}
	free(prHander);
	prHander = NULL;

	return 0;
}

static int save_stream(int fd, IMPEncoderStream *stream)
{
	int ret, i, nr_pack = stream->packCount;

  //IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u start----------\n", stream->packCount, stream->seq);
	for (i = 0; i < nr_pack; i++) {
	//IMP_LOG_DBG(TAG, "[%d]:%10u,%10lld,%10u,%10u,%10u\n", i, stream->pack[i].length, stream->pack[i].timestamp, stream->pack[i].frameEnd, *((uint32_t *)(&stream->pack[i].nalType)), stream->pack[i].sliceType);
		IMPEncoderPack *pack = &stream->pack[i];
		if(pack->length){
			uint32_t remSize = stream->streamSize - pack->offset;
			if(remSize < pack->length){
				ret = write(fd, (void *)(stream->virAddr + pack->offset), remSize);
				if (ret != remSize) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].remSize(%d) error:%s\n", ret, i, remSize, strerror(errno));
					return -1;
				}
				ret = write(fd, (void *)stream->virAddr, pack->length - remSize);
				if (ret != (pack->length - remSize)) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].(length-remSize)(%d) error:%s\n", ret, i, (pack->length - remSize), strerror(errno));
					return -1;
				}
			}else {
				ret = write(fd, (void *)(stream->virAddr + pack->offset), pack->length);
				if (ret != pack->length) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
					return -1;
				}
			}
		}
	}
  //IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u end----------\n", stream->packCount, stream->seq);
	return 0;
}

static int save_stream_by_name(char *stream_prefix, int idx, IMPEncoderStream *stream)
{
	int stream_fd = -1;
	char stream_path[128];
	int ret, i, nr_pack = stream->packCount;

	sprintf(stream_path, "%s_%d", stream_prefix, idx);

	IMP_LOG_DBG(TAG, "Open Stream file %s ", stream_path);
	stream_fd = open(stream_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (stream_fd < 0) {
		IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
		return -1;
	}
	IMP_LOG_DBG(TAG, "OK\n");

	for (i = 0; i < nr_pack; i++) {
		IMPEncoderPack *pack = &stream->pack[i];
		if(pack->length){
			uint32_t remSize = stream->streamSize - pack->offset;
			if(remSize < pack->length){
				ret = write(stream_fd, (void *)(stream->virAddr + pack->offset),
						remSize);
				if (ret != remSize) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].remSize(%d) error:%s\n", ret, i, remSize, strerror(errno));
					return -1;
				}
				ret = write(stream_fd, (void *)stream->virAddr, pack->length - remSize);
				if (ret != (pack->length - remSize)) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].(length-remSize)(%d) error:%s\n", ret, i, (pack->length - remSize), strerror(errno));
					return -1;
				}
			}else {
				ret = write(stream_fd, (void *)(stream->virAddr + pack->offset), pack->length);
				if (ret != pack->length) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
					return -1;
				}
			}
		}
	}

	close(stream_fd);

	return 0;
}

static void *get_video_stream(void *args)
{
	int val, i, chnNum, ret;
	char stream_path[64];
	IMPEncoderEncType encType;
	int stream_fd = -1, totalSaveCnt = 0;

	val = (int)args;
	chnNum = val & 0xffff;
	encType = (val >> 16) & 0xffff;

	ret = IMP_Encoder_StartRecvPic(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chnNum);
		return ((void *)-1);
	}
	sprintf(stream_path, "%s/stream-%d.%s", STREAM_FILE_PATH_PREFIX, chnNum,
			(encType == IMP_ENC_TYPE_AVC) ? "h264" : ((encType == IMP_ENC_TYPE_HEVC) ? "h265" : "jpeg"));

    if (encType == IMP_ENC_TYPE_JPEG) {
		totalSaveCnt = ((NR_FRAMES_TO_SAVE / 50) > 0) ? (NR_FRAMES_TO_SAVE / 50) : 1;
	} else {
		IMP_LOG_DBG(TAG, "Video ChnNum=%d Open Stream file %s ", chnNum, stream_path);
		stream_fd = open(stream_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
		if (stream_fd < 0) {
			IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
			return ((void *)-1);
		}
		IMP_LOG_DBG(TAG, "OK\n");
		totalSaveCnt = NR_FRAMES_TO_SAVE;
	}

	for (i = 0; i < totalSaveCnt; i++) {
		ret = IMP_Encoder_PollingStream(chnNum, 1000);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
			continue;
		}

		IMPEncoderStream stream;
		/* Get H264 or H265 Stream */
		ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
#ifdef SHOW_FRM_BITRATE
		int i, len = 0;
		for (i = 0; i < stream.packCount; i++) {
			len += stream.pack[i].length;
		}
		bitrate_sp[chnNum] += len;
		frmrate_sp[chnNum]++;

		int64_t now = IMP_System_GetTimeStamp() / 1000;
		if(((int)(now - statime_sp[chnNum]) / 1000) >= FRM_BIT_RATE_TIME){
			double fps = (double)frmrate_sp[chnNum] / ((double)(now - statime_sp[chnNum]) / 1000);
			double kbr = (double)bitrate_sp[chnNum] * 8 / (double)(now - statime_sp[chnNum]);

			printf("streamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)\n", chnNum, fps, kbr);
			//fflush(stdout);

			frmrate_sp[chnNum] = 0;
			bitrate_sp[chnNum] = 0;
			statime_sp[chnNum] = now;
		}
#endif
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
			return ((void *)-1);
		}

    if (encType == IMP_ENC_TYPE_JPEG) {
      ret = save_stream_by_name(stream_path, i, &stream);
      if (ret < 0) {
        return ((void *)ret);
      }
    }
#if 1
    else {
      ret = save_stream(stream_fd, &stream);
      if (ret < 0) {
        close(stream_fd);
        return ((void *)ret);
      }
    }
#endif
    IMP_Encoder_ReleaseStream(chnNum, &stream);
  }

	close(stream_fd);

	ret = IMP_Encoder_StopRecvPic(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chnNum);
		return ((void *)-1);
	}

	return ((void *)0);
}

int sample_get_video_stream()
{
	unsigned int i;
	int ret;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
            int arg = 0;
            if (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) {
                arg = (((chn[i].payloadType >> 24) << 16) | (4 + chn[i].index));
            } else {
                arg = (((chn[i].payloadType >> 24) << 16) | chn[i].index);
            }
			ret = pthread_create(&tid[i], NULL, get_video_stream, (void *)arg);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Create ChnNum%d get_video_stream failed\n", (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) ? (4 + chn[i].index) : chn[i].index);
			}
		}
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			pthread_join(tid[i],NULL);
		}
	}

	return 0;
}

int sample_get_jpeg_snap()
{
	int i, ret;
	char snap_path[64];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_StartRecvPic(4 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 4 + chn[i].index);
				return -1;
			}

			sprintf(snap_path, "%s/snap-%d.jpg",
					SNAP_FILE_PATH_PREFIX, chn[i].index);

			IMP_LOG_ERR(TAG, "Open Snap file %s ", snap_path);
			int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
			if (snap_fd < 0) {
				IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
				return -1;
			}
			IMP_LOG_DBG(TAG, "OK\n");

			/* Polling JPEG Snap, set timeout as 1000msec */
			ret = IMP_Encoder_PollingStream(4 + chn[i].index, 10000);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Polling stream timeout\n");
				continue;
			}

			IMPEncoderStream stream;
			/* Get JPEG Snap */
			ret = IMP_Encoder_GetStream(chn[i].index + 4, &stream, 1);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
				return -1;
			}

			ret = save_stream(snap_fd, &stream);
			if (ret < 0) {
				close(snap_fd);
				return ret;
			}

			IMP_Encoder_ReleaseStream(4 + chn[i].index, &stream);

			close(snap_fd);

			ret = IMP_Encoder_StopRecvPic(4 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
				return -1;
			}
		}
	}
	return 0;
}

int sample_get_video_stream_byfd()
{
	int streamFd[FS_CHN_NUM], vencFd[FS_CHN_NUM], maxVencFd = 0;
	char stream_path[FS_CHN_NUM][128];
	fd_set readfds;
	struct timeval selectTimeout;
	int saveStreamCnt[FS_CHN_NUM], totalSaveStreamCnt[FS_CHN_NUM];
	int i = 0, ret = 0, chnNum = 0;
	memset(streamFd, 0, sizeof(streamFd));
	memset(vencFd, 0, sizeof(vencFd));
	memset(stream_path, 0, sizeof(stream_path));
	memset(saveStreamCnt, 0, sizeof(saveStreamCnt));
	memset(totalSaveStreamCnt, 0, sizeof(totalSaveStreamCnt));

	for (i = 0; i < FS_CHN_NUM; i++) {
        streamFd[i] = -1;
        vencFd[i] = -1;
        saveStreamCnt[i] = 0;
        if (chn[i].enable) {
            if (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) {
                chnNum = 4 + chn[i].index;
                totalSaveStreamCnt[i] = (NR_FRAMES_TO_SAVE / 50 > 0) ? NR_FRAMES_TO_SAVE / 50 : NR_FRAMES_TO_SAVE;
            } else {
                chnNum = chn[i].index;
                totalSaveStreamCnt[i] = NR_FRAMES_TO_SAVE;
            }
            sprintf(stream_path[i], "%s/stream-%d.%s", STREAM_FILE_PATH_PREFIX, chnNum,
                    ((chn[i].payloadType >> 24) == IMP_ENC_TYPE_AVC) ? "h264" : (((chn[i].payloadType >> 24) == IMP_ENC_TYPE_HEVC) ? "h265" : "jpeg"));

            if (chn[i].payloadType != IMP_ENC_PROFILE_JPEG) {
                streamFd[i] = open(stream_path[i], O_RDWR | O_CREAT | O_TRUNC, 0777);
                if (streamFd[i] < 0) {
                    IMP_LOG_ERR(TAG, "open %s failed:%s\n", stream_path[i], strerror(errno));
                    return -1;
                }
            }

			vencFd[i] = IMP_Encoder_GetFd(chnNum);
			if (vencFd[i] < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_GetFd(%d) failed\n", chnNum);
				return -1;
			}

			if (maxVencFd < vencFd[i]) {
				maxVencFd = vencFd[i];
			}

			ret = IMP_Encoder_StartRecvPic(chnNum);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chnNum);
				return -1;
			}
		}
	}

	while(1) {
		int breakFlag = 1;
		for (i = 0; i < FS_CHN_NUM; i++) {
			breakFlag &= (saveStreamCnt[i] >= totalSaveStreamCnt[i]);
		}
		if (breakFlag) {
			break;  // save frame enough
		}

		FD_ZERO(&readfds);
		for (i = 0; i < FS_CHN_NUM; i++) {
			if (chn[i].enable && saveStreamCnt[i] < totalSaveStreamCnt[i]) {
				FD_SET(vencFd[i], &readfds);
			}
		}
		selectTimeout.tv_sec = 2;
		selectTimeout.tv_usec = 0;

        ret = select(maxVencFd + 1, &readfds, NULL, NULL, &selectTimeout);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "select failed:%s\n", strerror(errno));
            return -1;
        } else if (ret == 0) {
            continue;
        } else {
            for (i = 0; i < FS_CHN_NUM; i++) {
                if (chn[i].enable && FD_ISSET(vencFd[i], &readfds)) {
                    IMPEncoderStream stream;

                    if (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) {
                        chnNum = 4 + chn[i].index;
                    } else {
                        chnNum = chn[i].index;
                    }
                    /* Get H264 or H265 Stream */
                    ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
                    if (ret < 0) {
                        IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
                        return -1;
                    }

                    if (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) {
                        ret = save_stream_by_name(stream_path[i], saveStreamCnt[i], &stream);
                        if (ret < 0) {
                            return -1;
                        }
                    } else {
                        ret = save_stream(streamFd[i], &stream);
                        if (ret < 0) {
                            close(streamFd[i]);
                            return -1;
                        }
                    }

					IMP_Encoder_ReleaseStream(chnNum, &stream);
					saveStreamCnt[i]++;
				}
			}
		}
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            if (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) {
                chnNum = 4 + chn[i].index;
            } else {
                chnNum = chn[i].index;
            }
            IMP_Encoder_StopRecvPic(chnNum);
            close(streamFd[i]);
        }
    }

	return 0;
}

int sample_SetIRCUT(int enable)
{
	/*
	   IRCUTN = PD26
	   IRCUTP = PD27
	   */
	int fd, fd0, fd1;
	char on[4], off[4];

	int gpio0 = 122;
	int gpio1 = 123;
	char tmp[128];

	if (!access("/tmp/setir",0)) {
		if (enable) {
			system("/tmp/setir 0 1");
		} else {
			system("/tmp/setir 1 0");
		}
		return 0;
	}

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/export error !");
		return -1;
	}

	sprintf(tmp, "%d", gpio0);
	write(fd, tmp, 2);
	sprintf(tmp, "%d", gpio1);
	write(fd, tmp, 2);

	close(fd);

	sprintf(tmp, "/sys/class/gpio/gpio%d/direction", gpio0);
	fd0 = open(tmp, O_RDWR);
	if(fd0 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/direction error !", gpio0);
		return -1;
	}

	sprintf(tmp, "/sys/class/gpio/gpio%d/direction", gpio1);
	fd1 = open(tmp, O_RDWR);
	if(fd1 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/direction error !", gpio1);
		return -1;
	}

	write(fd0, "out", 3);
	write(fd1, "out", 3);

	close(fd0);
	close(fd1);

	sprintf(tmp, "/sys/class/gpio/gpio%d/active_low", gpio0);
	fd0 = open(tmp, O_RDWR);
	if(fd0 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/active_low error !", gpio0);
		return -1;
	}

	sprintf(tmp, "/sys/class/gpio/gpio%d/active_low", gpio1);
	fd1 = open(tmp, O_RDWR);
	if(fd1 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/active_low error !", gpio1);
		return -1;
	}

	write(fd0, "0", 1);
	write(fd1, "0", 1);

	close(fd0);
	close(fd1);

	sprintf(tmp, "/sys/class/gpio/gpio%d/value", gpio0);
	fd0 = open(tmp, O_RDWR);
	if(fd0 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/value error !", gpio0);
		return -1;
	}

	sprintf(tmp, "/sys/class/gpio/gpio%d/value", gpio1);
	fd1 = open(tmp, O_RDWR);
	if(fd1 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/value error !", gpio1);
		return -1;
	}

	sprintf(on, "%d", enable);
	sprintf(off, "%d", !enable);

	write(fd0, "0", 1);
	usleep(10*1000);

	write(fd0, on, strlen(on));
	write(fd1, off, strlen(off));

	if (!enable) {
		usleep(10*1000);
		write(fd0, off, strlen(off));
	}

	close(fd0);
	close(fd1);

	return 0;
}

static int  g_soft_ps_running = 1;
void *sample_soft_photosensitive_ctrl(IMPVI_NUM vinum, void *p)
{
	int i = 0;
	float gb_gain,gr_gain;
	float iso_buf;
	bool ircut_status = true;
	g_soft_ps_running = 1;
	int night_count = 0;
	int day_count = 0;
	//int day_oth_count = 0;
	//bayer域的 (g分量/b分量) 统计值
	float gb_gain_record = 200;
	//float gr_gain_record = 200;
	float gb_gain_buf = 200, gr_gain_buf = 200;
	IMPISPRunningMode pmode;
	IMPISPAEExprInfo ExpInfo;
	IMPISPAWBGlobalStatisInfo wb;
	IMP_ISP_Tuning_SetISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
	sample_SetIRCUT(1);

	while (g_soft_ps_running) {
		//获取曝光AE信息
		int ret = IMP_ISP_Tuning_GetAeExprInfo(vinum, &ExpInfo);
		if (ret ==0) {
			printf("u32ExposureTime: %d\n", ExpInfo.ExposureValue);
			printf("u32AnalogGain: %d\n",	ExpInfo.AeAGain);
			printf("u32DGain: %d\n",	ExpInfo.AeDGain);
		} else {
			return NULL;
		}
		iso_buf = ExpInfo.ExposureValue;
		printf(" iso buf ==%f\n",iso_buf);
		ret = IMP_ISP_Tuning_GetAwbGlobalStatistics(vinum, &wb);
		if (ret == 0) {
			gr_gain =wb.statis_gol_gain.rgain;
			gb_gain =wb.statis_gol_gain.bgain;
			// printf("gb_gain: %f\n", gb_gain);
			// printf("gr_gain: %f\n", gr_gain);
			// printf("gr_gain_record: %f\n", gr_gain_record);
		} else {
			return NULL;
		}

		//平均亮度小于20，则切到夜视模式
		if (iso_buf >1900000) {
			night_count++;
			printf("night_count==%d\n",night_count);
			if (night_count>5) {
				IMP_ISP_Tuning_GetISPRunningMode(vinum, &pmode);
				if (pmode!=IMPISP_RUNNING_MODE_NIGHT) {
					printf("### entry night mode ###\n");
					IMPISPRunningMode mode = IMPISP_RUNNING_MODE_NIGHT;
					IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
					sample_SetIRCUT(0);
					ircut_status = true;
				}
				//切夜视后，取20个gb_gain的的最小值，作为切换白天的参考数值gb_gain_record ，gb_gain为bayer的G/B
				for (i=0; i<20; i++) {
					IMP_ISP_Tuning_GetAwbGlobalStatistics(vinum, &wb);
					gr_gain =wb.statis_gol_gain.rgain;
					gb_gain =wb.statis_gol_gain.bgain;
					if (i==0) {
						gb_gain_buf = gb_gain;
						gr_gain_buf = gr_gain;
					}
					gb_gain_buf = ((gb_gain_buf>gb_gain)?gb_gain:gb_gain_buf);
					gr_gain_buf = ((gr_gain_buf>gr_gain)?gr_gain:gr_gain_buf);
					usleep(300000);
					gb_gain_record = gb_gain_buf;
					//gr_gain_record = gr_gain_buf;
					// printf("gb_gain == %f,iso_buf=%f",gb_gain,iso_buf);
					// printf("gr_gain_record == %f\n ",gr_gain_record);
				}
			}
		} else {
			night_count = 0;
		}
		//满足这3个条件进入白天切换判断条件
		if (((int)iso_buf < 479832) &&(ircut_status == true) &&(gb_gain>gb_gain_record+15)) {
			if ((iso_buf<361880)||(gb_gain >145)) {
				day_count++;
			} else {
				day_count=0;
			}
			// printf("gr_gain_record == %f gr_gain =%f line=%d\n",gr_gain_record,gr_gain,__LINE__);
			// printf("day_count == %d\n",day_count);
			if (day_count>3) {
				printf("### entry day mode ###\n");
				IMP_ISP_Tuning_GetISPRunningMode(vinum, &pmode);
				if (pmode!=IMPISP_RUNNING_MODE_DAY) {
					IMPISPRunningMode mode = IMPISP_RUNNING_MODE_DAY;
					IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
					sample_SetIRCUT(1);
					ircut_status = false;
				}
			}
		} else {
			day_count = 0;
		}
		sleep(1);
	}
	return NULL;
}

int64_t sample_gettimeus(void)
{
    struct timeval sttime;
    gettimeofday(&sttime,NULL);
    return (sttime.tv_sec  * 1000000 + (sttime.tv_usec));
}

int sample_framesource_i2dopr(void)
{
    int i = 0,ret = 0;
    static int s32cnt = 0;
    IMPFSI2DAttr sti2dattr;
    for(i = 0;i < FS_CHN_NUM;i++){
        if(chn[i].enable){
            if(s32cnt++ % 2 == 0)
            {
                /*i2d enable*/
                ret = IMP_FrameSource_GetI2dAttr(chn[i].index,&sti2dattr);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_FrameSource_GetI2dAttr(%d) error !\n", chn[i].index);
                    return -1;
                }
                memset(&sti2dattr,0x0,sizeof(IMPFSI2DAttr));
                sti2dattr.i2d_enable = 1;
                sti2dattr.flip_enable = 0;
                sti2dattr.mirr_enable = 0;
                sti2dattr.rotate_enable = 1;
                sti2dattr.rotate_angle = 90;
                ret = IMP_FrameSource_SetI2dAttr(chn[i].index,&sti2dattr);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_FrameSource_SetI2dAttr(%d) error !\n", chn[i].index);
                    return -1;
                }

            }else{
                /*i2d disable*/
                ret = IMP_FrameSource_GetI2dAttr(chn[i].index,&sti2dattr);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_FrameSource_GetI2dAttr(%d) error !\n", chn[i].index);
                    return -1;
                }
                memset(&sti2dattr,0x0,sizeof(IMPFSI2DAttr));
                sti2dattr.i2d_enable = 1;/*if this off,all i2d off*/
                sti2dattr.flip_enable = 0;
                sti2dattr.mirr_enable = 0;
                sti2dattr.rotate_enable = 0;
                sti2dattr.rotate_angle = 90;
                ret = IMP_FrameSource_SetI2dAttr(chn[i].index,&sti2dattr);
                if(ret < 0){
                    IMP_LOG_ERR(TAG, "IMP_FrameSource_SetI2dAttr(%d) error !\n", chn[i].index);
                    return -1;
                }
            }
        }
    }
    return 0;
}

void *getframeex(void *arg)
{
	int chanNum = (int)arg;
	printf("channNum:%d\n",chanNum);
	int i = 0,ret = 0;
	int fd;
	char framefilename[64];
	/*创建存储文件名*/
	sprintf(framefilename, "/mnt/t40_temp/shm%dfrm%d.nv12",(int)getpid(),chanNum);
	fd = open(framefilename, O_RDWR | O_CREAT, 0x644);
	if(fd < 0){
		IMP_LOG_ERR(TAG, "[%s][%d]open failed\n",__func__,__LINE__);
		return NULL;
	}

	for(i = 0; i < 100;i++)
	{
		IMPFrameInfo *frame =NULL;
		ret = IMP_FrameSource_GetFrameEx(chanNum, &frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrameEx failed\n");
			return NULL;
		}

#if 1
		ret = write(fd,(void *)frame->virAddr, frame->width * frame->height);
		ret |= write(fd,(void *)frame->virAddr + frame->width * ((frame->height + 15) & ~15),frame->width * frame->height / 2);
#endif
		ret = IMP_FrameSource_ReleaseFrameEx(chanNum, frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_ReleaseFrameEx failed\n");
			return NULL;
		}
		printf("IMP_FrameSource_ReleaseFrameEx%d succeed\n",chanNum);
	}

	return "succeed";
}

int sample_ipc_getframeEx(void)
{
	int i = 0,ret = 0;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = pthread_create(&tid[i],NULL,getframeex,(void *)i);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrameEx failed\n");
				return -1;
			}
		}
	}
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = pthread_join(tid[i],NULL);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrameEx failed\n");
				return -1;
			}
		}
	}
	return 0;
}

