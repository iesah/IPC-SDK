/*
 * sample-Change-Resolution.c
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co.,Ltd
 */

/*
 * 此例目的是将输出码流1080p+720p切换成720p+360p，再切换成360p+320x240
 * 主、次码流均采用将encoder和framesource销毁重新创建的方法来切换分辨率
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include "sample-common.h"

#define TAG "sample-Change-Resolution"

extern struct chn_conf chn[];

static int save_stream(int fd, IMPEncoderStream *stream)
{
	int ret, i, nr_pack = stream->packCount;

	for (i = 0; i < nr_pack; i++) {
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
	return 0;
}

static void *res_get_video_stream(void *args)
{
	int val, i, chnNum, ret;
	char stream_path[64];
	IMPEncoderEncType encType;
	int stream_fd = -1, totalSaveCnt = 0;

	val = (int)args;
	chnNum = val & 0xffff;
	encType = (val >> 16) & 0xffff;

	/* Step.1 Start receive picture*/
	ret = IMP_Encoder_StartRecvPic(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chnNum);
		return ((void *)-1);
	}

	sprintf(stream_path, "%s/stream-%d-%dx%d.%s", STREAM_FILE_PATH_PREFIX, chnNum,
		chn[chnNum].fs_chn_attr.picWidth, chn[chnNum].fs_chn_attr.picHeight,
		(encType == IMP_ENC_TYPE_AVC) ? "h264" : "h265");

	IMP_LOG_DBG(TAG, "Video ChnNum=%d Open Stream file %s ", chnNum, stream_path);

	stream_fd = open(stream_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (stream_fd < 0) {
		IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
		return ((void *)-1);
	}
	IMP_LOG_DBG(TAG, "OK\n");
	totalSaveCnt = NR_FRAMES_TO_SAVE;

	for (i = 0; i < totalSaveCnt; i++) {
		/* Step.2 Polling Stream */
		ret = IMP_Encoder_PollingStream(chnNum, 1000);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
			continue;
		}

		/* Step.3 Get H264 or H265 Stream */
		IMPEncoderStream stream;
		ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
			return ((void *)-1);
		}

		/* Step.4 Save H264 or H265 Stream */
		ret = save_stream(stream_fd, &stream);
		if (ret < 0) {
			close(stream_fd);
			return ((void *)ret);
		}

		/* Step.5 Close H264 or H265 Stream */
		IMP_Encoder_ReleaseStream(chnNum, &stream);
	}

	close(stream_fd);

	/* Step.6 Stop receive picture*/
	ret = IMP_Encoder_StopRecvPic(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chnNum);
		return ((void *)-1);
	}

	return ((void *)0);
}

static int sample_res_get_video_stream()
{
	unsigned int i;
	int ret;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
		int arg = 0;
			arg = (((chn[i].payloadType >> 24) << 16) | chn[i].index);
			ret = pthread_create(&tid[i], NULL, res_get_video_stream, (void *)arg);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Create ChnNum%d res_get_video_stream failed\n", chn[i].index);
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

int sample_res_init()
{
	int ret, i;

	/* Step.1 FrameSource init */
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.2 Create encoder group */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_CreateGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
				return -1;
			}
		}
	}

	/* Step.3 Encoder init */
	ret = sample_encoder_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		return -1;
	}

	/* Step.4 Bind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.5 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}
	return 0;
}

int sample_res_deinit()
{
	int ret, i;
	/* Step.1 Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.2 UnBind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.3 Encoder exit */
	ret = sample_encoder_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -1;
	}

	/* Step.4 FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;

	/* only enable chn[0] & chn[1] */
	chn[0].enable = 1;
	chn[1].enable = 1;
	chn[2].enable = 0;
	chn[3].enable = 0;

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}

	/* Step.2  init original resolution 1920x1080 & 1280x720 */
	chn[0].fs_chn_attr.scaler.enable = 0;
	chn[0].fs_chn_attr.crop.enable = 1;
	chn[0].fs_chn_attr.crop.top = 0;
	chn[0].fs_chn_attr.crop.left = 0;
	chn[0].fs_chn_attr.crop.width = 1920;
	chn[0].fs_chn_attr.crop.height = 1080;
	chn[0].fs_chn_attr.picWidth = 1920;
	chn[0].fs_chn_attr.picHeight = 1080;

	chn[1].fs_chn_attr.scaler.enable = 1;
	chn[1].fs_chn_attr.scaler.outwidth = 1920;
	chn[1].fs_chn_attr.scaler.outheight = 1080;
	chn[1].fs_chn_attr.crop.enable = 1;
	chn[1].fs_chn_attr.crop.top = 0;
	chn[1].fs_chn_attr.crop.left = 0;
	chn[1].fs_chn_attr.crop.width = 1280;
	chn[1].fs_chn_attr.crop.height = 720;
	chn[1].fs_chn_attr.picWidth = 1280;
	chn[1].fs_chn_attr.picHeight = 720;

	/* Step.3 init framesource and encoder */
	ret = sample_res_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_res_init failed\n");
		return -1;
	}

	/* Step.4 get frame  */
	ret = sample_res_get_video_stream();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get video stream failed\n");
		return -1;
	}

	/* Step.5  deinit framesource and encoder */
	ret = sample_res_deinit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_res_deinit failed\n");
		return -1;
	}

	/* Step.6 change resolution to 1280x720 & 640x360 */
	chn[0].fs_chn_attr.scaler.enable = 1;
	chn[0].fs_chn_attr.scaler.outwidth = 1280;
	chn[0].fs_chn_attr.scaler.outheight = 720;
	chn[0].fs_chn_attr.crop.enable = 0;
	chn[0].fs_chn_attr.crop.top = 0;
	chn[0].fs_chn_attr.crop.left = 0;
	chn[0].fs_chn_attr.crop.width = 1920;
	chn[0].fs_chn_attr.crop.height = 1080;
	chn[0].fs_chn_attr.picWidth = 1280;
	chn[0].fs_chn_attr.picHeight = 720;

	chn[1].fs_chn_attr.scaler.enable = 1;
	chn[1].fs_chn_attr.scaler.outwidth = 640;
	chn[1].fs_chn_attr.scaler.outheight = 360;
	chn[1].fs_chn_attr.crop.enable = 0;
	chn[1].fs_chn_attr.picWidth = 640;
	chn[1].fs_chn_attr.picHeight = 360;

	/* Step.7 init framesource and encoder */
	ret = sample_res_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_res_init failed\n");
		return -1;
	}

	/* Step.8 get frame  */
	ret = sample_res_get_video_stream();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get video stream failed\n");
		return -1;
	}

	/* Step.9 deinit framesource and encoder */
	ret = sample_res_deinit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_res_deinit failed\n");
		return -1;
	}

	/* Step.10 change resolution to 640x360 & 320x240 */
	chn[0].fs_chn_attr.scaler.enable = 1;
	chn[0].fs_chn_attr.scaler.outwidth = 640;
	chn[0].fs_chn_attr.scaler.outheight = 360;
	chn[0].fs_chn_attr.crop.enable = 0;
	chn[0].fs_chn_attr.picWidth = 640;
	chn[0].fs_chn_attr.picHeight = 360;

	chn[1].fs_chn_attr.scaler.enable = 1;
	chn[1].fs_chn_attr.scaler.outwidth = 320;
	chn[1].fs_chn_attr.scaler.outheight = 240;
	chn[1].fs_chn_attr.crop.enable = 0;
	chn[1].fs_chn_attr.picWidth = 320;
	chn[1].fs_chn_attr.picHeight = 240;

	/* Step.11 init framesource and encoder */
	ret = sample_res_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_res_init failed\n");
		return -1;
	}

	/* Step.12 get frame  */
	ret = sample_res_get_video_stream();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get video stream failed\n");
		return -1;
	}

	/* Step.13 deinit framesource and encoder */
	ret = sample_res_deinit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_res_deinit failed\n");
		return -1;
	}

	/* Step.14 system exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	return 0;
}
