/*
 * sample-Encoder-video.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 *
 * */
#include <stdio.h>
#include <stdlib.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

#include "sample-common.h"

#define TAG "Sample-I2d"

extern struct chn_conf chn[];
static int byGetFd = 0;

int main(int argc, char *argv[])
{
	int i, ret;

    if (argc >= 2) {
        byGetFd = atoi(argv[1]);
    }

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}

	/*Step.2 framesource init*/
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

    while(1) {
	    /* Step.3 I2dAttr init */
        ret = sample_framesource_i2dopr();
	    if (ret < 0) {
		    IMP_LOG_ERR(TAG, "sample_framesource_i2dopr  failed\n");
		    return -1;
	    }

	    /* Step.4 Encoder init */
	    for (i = 0; i < FS_CHN_NUM; i++) {
		    if (chn[i].enable) {
			    ret = IMP_Encoder_CreateGroup(chn[i].index);
			    if (ret < 0) {
				    IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
				    return -1;
			    }
		    }
	    }

	    ret = sample_encoder_init();
	    if (ret < 0) {
		    IMP_LOG_ERR(TAG, "Encoder init failed\n");
		    return -1;
	    }

	    /* Step.5 Bind */
	    for (i = 0; i < FS_CHN_NUM; i++) {
		    if (chn[i].enable) {
			    ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			    if (ret < 0) {
				    IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
				    return -1;
			    }
		    }
	    }

	    /* Step.6 Stream On */
	    ret = sample_framesource_streamon();
	    if (ret < 0) {
		    IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		    return -1;
	    }

        /* Step.7 Get stream */
        if (byGetFd) {
            ret = sample_get_video_stream_byfd();
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "Get video stream byfd failed\n");
                return -1;
            }
        } else {
            ret = sample_get_video_stream();
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "Get video stream failed\n");
                return -1;
            }
        }

	    /* Exit sequence as follow */
	    /* Step.8 Stream Off */
	    ret = sample_framesource_streamoff();
	    if (ret < 0) {
		    IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		    return -1;
	    }

	    /* Step.9 UnBind */
	    for (i = 0; i < FS_CHN_NUM; i++) {
		    if (chn[i].enable) {
			    ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			    if (ret < 0) {
				    IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				    return -1;
			    }
		    }
	    }

	    /* Step.10 Encoder exit */
	    ret = sample_encoder_exit();
	    if (ret < 0) {
		    IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		    return -1;
	    }
    }

	/* Step.11 FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.12 System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	return 0;
}

