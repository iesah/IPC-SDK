/*
 * sample_Soft_photosensitive.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 *
 * 本文件所有调用的api具体说明均可在 proj/sdk-lv3/include/api/cn/imp/ 目录下头文件中查看
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

#include "sample-common.h"

#define TAG "Sample_Soft_photosensitive"

extern struct chn_conf chn[];
static int byGetFd = 0;
static int  g_soft_ps_running = 1;
//#define SOFT_PHOTOSENSITIVE_DEBUG

int sample_set_ircut(int enable)
{
	/*
	   IRCUTN = PB17
	   IRCUTP = PB18
	*/
	int fd, fd0, fd1;
	char on[4], off[4];

	int gpio0 = 49;
	int gpio1 = 50;
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

static void *sample_soft_photosensitive_thread(void *p)
{
	int i = 0, ret = 0;
	IMPVI_NUM vinum = IMPVI_MAIN;
	float gb_gain, gr_gain;
	float iso_buf;
	bool ircut_status = false;
	int night_count = 0;
	int day_count = 0;
	//int day_oth_count = 0;

	//bayer域的 (g分量/b分量) 统计值
	float gb_gain_record = 200;
	float gr_gain_record = 200;
	float gb_gain_buf = 200, gr_gain_buf = 200;

	int luma = 0, luma_scence = 0;

	IMPISPRunningMode pmode;
	IMPISPAEExprInfo ExpInfo;
	IMPISPAWBGlobalStatisInfo wb;
	IMPISPAEScenceAttr AeSenAttr;
	IMPISPRunningMode mode;

	// init isp running mode
	mode = IMPISP_RUNNING_MODE_DAY;
	ret = IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
	sample_set_ircut(0);

	while (g_soft_ps_running) {
#ifdef SOFT_PHOTOSENSITIVE_DEBUG
		printf("---------------------Record----------------------\n");
		printf("gr_gain_record: %f\n", gr_gain_record);
		printf("gb_gain_record: %f\n", gb_gain_record);
#endif
		//获取曝光AE信息
		ret = IMP_ISP_Tuning_GetAeExprInfo(vinum, &ExpInfo);
		if (ret == 0) {
			iso_buf = ExpInfo.ExposureValue;
#ifdef SOFT_PHOTOSENSITIVE_DEBUG
			printf("-------------------AeExprInfo--------------------\n");
			printf("u32ExposureValue: %lld\n", ExpInfo.ExposureValue);
			printf("u32AeAGain: %d\n", ExpInfo.AeAGain);
			printf("u32AeDGain: %d\n", ExpInfo.AeDGain);
#endif
		} else {
			return NULL;
		}

		ret = IMP_ISP_Tuning_GetAeScenceAttr(vinum, &AeSenAttr);
		if(ret == 0) {
			luma = AeSenAttr.luma;
			luma_scence = AeSenAttr.luma_scence;
#ifdef SOFT_PHOTOSENSITIVE_DEBUG
			printf("--------------------AeScence---------------------\n");
			printf("Luma: %d, Luma Scence: %d\n", AeSenAttr.luma, AeSenAttr.luma_scence);
#endif
		} else {
			return NULL;
		}

		ret = IMP_ISP_Tuning_GetAwbGlobalStatistics(vinum, &wb);
		if (ret == 0) {
			gr_gain = wb.statis_gol_gain.rgain;
			gb_gain = wb.statis_gol_gain.bgain;
#ifdef SOFT_PHOTOSENSITIVE_DEBUG
			printf("---------------AwbGlobalStatistics---------------\n");
			printf("gb_gain: %f\n", gb_gain);
			printf("gr_gain: %f\n", gr_gain);
			printf("-------------------------------------------------\n");
#endif
		} else {
			return NULL;
		}

		//平均亮度小于20，则切到夜视模式
		if ((iso_buf > 200000) && ((luma < 50) || (luma_scence > 1000000))) {
			night_count++;
			printf("night_count == %d\n", night_count);
			if (night_count >= 5) {
				IMP_ISP_Tuning_GetISPRunningMode(vinum, &pmode);
				if (pmode != IMPISP_RUNNING_MODE_NIGHT) {
					printf("### entry night mode ###\n");
					IMPISPRunningMode mode = IMPISP_RUNNING_MODE_NIGHT;
					IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
					sample_set_ircut(1);
					ircut_status = true;
				}
				//切夜视后，取20个gb_gain的最小值，作为切换白天的参考数值gb_gain_record ，gb_gain为bayer的G/B
				for (i=0; i<20; i++) {
					IMP_ISP_Tuning_GetAwbGlobalStatistics(vinum, &wb);
					gr_gain = wb.statis_gol_gain.rgain;
					gb_gain = wb.statis_gol_gain.bgain;
					if (i == 0) {
						gb_gain_buf = gb_gain;
						gr_gain_buf = gr_gain;
					}
					gb_gain_buf = ((gb_gain_buf > gb_gain) ? gb_gain:gb_gain_buf);
					gr_gain_buf = ((gr_gain_buf > gr_gain) ? gr_gain:gr_gain_buf);
					usleep(300000);
					gb_gain_record = gb_gain_buf;
					gr_gain_record = gr_gain_buf;
#ifdef SOFT_PHOTOSENSITIVE_DEBUG
					printf("gb_gain = %.2f, gr_gain = %.2f, iso_buf = %.2f\n", gb_gain, gr_gain, iso_buf);
					printf("gr_gain_record = %.2f, gb_gain_record = %.2f\n", gr_gain_record, gb_gain_record);
#endif
				}
			}
		} else {
			night_count = 0;
		}

		//满足这3个条件进入白天切换判断条件
		if (((int)iso_buf < 10000) && (ircut_status == true) && ((luma > 50) || (luma_scence < 1000000))) {
			if (iso_buf < 10000 || (gb_gain < 145) || gr_gain_record < 500 || gb_gain_record < 130) {
				day_count++;
				printf("day_count == %d\n", day_count);
			} else {
				day_count = 0;
			}

			if (day_count > 3) {
				printf("### entry day mode ###\n");
				IMP_ISP_Tuning_GetISPRunningMode(vinum, &pmode);
				if (pmode!=IMPISP_RUNNING_MODE_DAY) {
					IMPISPRunningMode mode = IMPISP_RUNNING_MODE_DAY;
					IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
					sample_set_ircut(0);
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

pthread_t tid[FS_CHN_NUM];
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

	/* Step.2 FrameSource init */
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.3 Encoder init */
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

	/* Step.6 Create soft photosensitive thread */
	ret = pthread_create(&tid[i], NULL, sample_soft_photosensitive_thread, (void *)argv);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Create sample_soft_photosensitive_thread failed\n");
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
	/* Step.a Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.b UnBind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.c Encoder exit */
	ret = sample_encoder_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -1;
	}

	/* Step.d FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.e System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	return 0;
}
