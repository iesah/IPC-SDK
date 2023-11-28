/*
 * sample-Encoder-video.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include "sample-common.h"

#define TAG "GetFrameEx"

extern struct chn_conf chn[];

void *getframeex(void *arg)
{
	int chanNum = (int)arg;
	printf("channNum:%d\n",chanNum);
	int i = 0,ret = 0;
	int fd;
	char framefilename[64];
	/*创建存储文件名*/
	sprintf(framefilename, "/tmp/shm%dfrm%d.nv12",(int)getpid(),chanNum);//后面改下这里的路径，然后只dump一张图片
	fd = open(framefilename, O_RDWR | O_CREAT, 0x644);
	if(fd < 0){
		IMP_LOG_ERR(TAG, "[%s][%d]open failed\n",__func__,__LINE__);
		return NULL;
	}

	for(i = 0; i < NR_FRAMES_TO_SAVE;i++)
	{
		IMPFrameInfo *frame =NULL;
		ret = IMP_FrameSource_GetFrameEx(chanNum, &frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrameEx failed\n");
			return NULL;
		}

		if(NR_FRAMES_TO_SAVE/2 == i){
			ret = write(fd,(void *)frame->virAddr, frame->width * frame->height);
			ret |= write(fd,(void *)frame->virAddr + frame->width * ((frame->height + 15) & ~15),frame->width * frame->height / 2);
		}
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



int main(int argc,char *argv[])
{
	int ret = 0;

	ret = sample_ipc_getframeEx();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrameEx failed\n");
		return -1;
	}

	return 0;
}
