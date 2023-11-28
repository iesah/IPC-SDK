/*
 * sample-Isp-Bypass.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include "sample-common.h"

#define TAG "Sample-Isp-Bypass"

extern struct chn_conf chn[];

int main(int argc, char *argv[])
{
	int ret;
	IMPISPInternalChnAttr attr;
	int i;

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

	IMP_ISP_GetInternalChnAttr(IMPVI_MAIN, &attr);

#if 0
	/* Before using the IR channel, check whether the Sensor supports it */
	attr.ch[1].type = TX_ISP_INTERNAL_CHANNEL_ISP_IR;
	chn[1].fs_chn_attr.pixFmt = PIX_FMT_RAW8;
	chn[1].fs_chn_attr.nrVBs = 2;
	chn[1].enable = 1;
#endif
#if 0
	attr.ch[1].type = TX_ISP_INTERNAL_CHANNEL_VIC_DMA0;
	attr.ch[1].vc_index = 0;
	chn[1].fs_chn_attr.pixFmt = PIX_FMT_RAW16;
	chn[1].fs_chn_attr.nrVBs = 2;
	chn[1].enable = 1;
#endif

#if 0
	attr.ch[2].type = TX_ISP_INTERNAL_CHANNEL_VIC_DMA1;
	attr.ch[2].vc_index = 1;
	chn[2].fs_chn_attr.pixFmt = PIX_FMT_RAW16;
	chn[2].fs_chn_attr.nrVBs = 2;
	chn[2].enable = 1;
#endif

	IMP_ISP_SetInternalChnAttr(IMPVI_MAIN, &attr);

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
	ret = sample_get_frame();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get frame failed\n");
		return -1;
	}

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
