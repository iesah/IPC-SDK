/*
 * sample-Encoder-video.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include "sample-common.h"

#define TAG "Sample-FrameSource"

extern struct chn_conf chn[];

static void *mainpro_getframeex_thread(void *args)
{
	int index = (int)args;
	int chnNum = chn[index].index;
	int i = 0, ret = 0;
	IMPFrameInfo *frame = NULL;
	char framefilename[64];
	int fd = -1;

	if (PIX_FMT_NV12 == chn[index].fs_chn_attr.pixFmt) {
		sprintf(framefilename, "frame%dx%d_%d.nv12", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight, index);
	} else {
		sprintf(framefilename, "frame%dx%d_%d.raw", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight, index);
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

int sample_mainpro_getframeex(void)
{
	int ret;
	unsigned int i;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = pthread_create(&tid[i], NULL, mainpro_getframeex_thread, (void *)i);
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

int main(int argc, char *argv[])
{
	int ret;

	printf("Usage:%s [fpsnum0 [fpsnum1]]]\n", argv[0]);

	if (argc >= 2) {
		chn[0].fs_chn_attr.outFrmRateNum = atoi(argv[1]);
	}

	if (argc >= 3) {
		chn[1].fs_chn_attr.outFrmRateNum = atoi(argv[2]);
	}

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}

	/* Step.2 FrameSource init */
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.3 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}

	/* Step.4 Get frame */
#if 1
	ret = sample_mainpro_getframeex();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get frame failed\n");
		return -1;
	}
#else
	ret = sample_get_frame();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get frame failed\n");
		return -1;
	}
#endif

	/* Exit sequence as follow */

	/* Step.5 Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.6 FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.7 System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	return 0;
}
