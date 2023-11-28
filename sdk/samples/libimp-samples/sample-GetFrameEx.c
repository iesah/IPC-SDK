/*
 * sample-Encoder-video.c
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
#define TAG "GetFrameEx"


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
