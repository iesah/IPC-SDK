/*
 * sample-Encoder-jpeg.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 *
 * 本文件所有调用的api具体说明均可在 proj/sdk-lv3/include/api/cn/imp/ 目录下头文件中查看
 *
 * Step.1 System init 系统初始化
 *		@code
 *			memset(&sensor_info, 0, sizeof(sensor_info));
 *			if(SENSOR_NUM == IMPISP_TOTAL_ONE){
 *				memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo));
 *			} else if(SENSOR_NUM == IMPISP_TOTAL_TWO){
 *				memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo) * 2);
 *			}else if(SENSOR_NUM ==IMPISP_TOTAL_THR){
 *				memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo) * 3)
 *			} //根据sensor个数将对应的大小的Def_Sensor_Info中的内容拷贝到sensor_info
 *
 *			ret = IMP_ISP_Open() //打开ISP模块
 *			ret = IMP_ISP_SetCameraInputMode(&mode) //如果有多个sensor(最大支持三摄),设置多摄的模式(单摄请忽略)
 *			ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info[*]) //添加sensor,在此操作之前sensor驱动已经添加到内核 (IMPVI_MAIN为主摄, IMPVI_SEC为次摄, IMPVI_THR为第三摄)
 *			ret = IMP_ISP_EnableSensor(IMPVI_MAIN, &sensor_info[*])	//使能sensor, 现在sensor开始输出图像 (IMPVI_MAIN为主摄, IMPVI_SEC为次摄, IMPVI_THR为第三摄)
 *			ret = IMP_System_Init() //系统初始化
 *			ret = IMP_ISP_EnableTuning() //使能ISP tuning, 然后才能调用ISP调试接口
 *		@endcode
 * Step.2 FrameSource init Framesource初始化
 *		@code
 *			ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr) //创建通道
 *			ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr) //设置通道的相关属性, 包括:图片宽度 图片高度 图片格式 通道的输出帧率 缓存buf数 裁剪和缩放属性
 *		@endcode
 * Step.3 Encoder init 编码初始化
 *		@code
 *			ret = IMP_Encoder_CreateGroup(chn[i].index) //创建编码Group
 *			ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
 *											imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, 25, 0) //设置编码chnnel默认的属性
 *			ret = IMP_Encoder_CreateChn(4 + chn[i].index, &channel_attr) //创建编码chnnel
 *			ret = IMP_Encoder_RegisterChn(i, 4 + chn[i].index) //注册编码Channel到Group
 *		@endcode
 * Step.4 Bind 绑定framesource和编码chnnel
 *		@code
 *			ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder)	//绑定framesource和编码chnnel, 绑定成功即framesource产生的数据可以自动传送到编码chnnel
 *		@endcode
 * Step.5 Stream On 使能Framesource chnnel, 开始输出图像
 *		@code
 *			ret = IMP_FrameSource_EnableChn(chn[i].index) //使能chnnel, chnnel开始输出图像
 *		@endcode
 * Step.6 Get Snap 获取JPEG编码图片
 *		@code
 *			ret = IMP_Encoder_StartRecvPic(4 + chn[i].index) //编码chnnel开始接受图像
 *			ret = IMP_Encoder_PollingStream(4 + chn[i].index, 1000) //对编码chnnel中码流进行polling
 *			ret = IMP_Encoder_GetStream(4 + chn[i].index, &stream, 1) //获取编码码流
 *			ret = save_stream(snap_fd, &stream) //将结果保存到打开的文件描述符
 *			ret = IMP_Encoder_ReleaseStream(4 + chn[i].index, &stream) //释放编码码流, 与IMP_Encoder_GetStream成对使用
 *			ret = IMP_Encoder_StopRecvPic(4 + chn[i].index) //编码chnnel停止接受图像
 *		@endcode
 * Step.7 Stream Off 不使能Framesource chnnel, 停止输出图像
 *		@code
 *			ret = IMP_FrameSource_DisableChn(chn[i].index) //不使能channel, channel停止输出图像
 *		@endcode
 * Step.8 UnBind 解绑Framesource和编码chnnel
 *		@code
 *			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder) //解绑framesource和编码chnnel
 *		@endcode
 * Step.9 Encoder exit 编码反初始化
 *		@code
 *			ret = IMP_Encoder_Query(chnNum, &chn_stat) //查询编码chnnel状态
 *			ret = IMP_Encoder_UnRegisterChn(chnNum) //如果已经注册了, 反注册编码chnnel到Group
 *			ret = IMP_Encoder_DestroyChn(chnNum) //销毁编码chnnel
 *		@endcode
 * Step.10 FrameSource exit Framesource反初始化
 *		@code
 *			ret = IMP_FrameSource_DestroyChn(chn[i].index) //销毁channel
 *		@endcode
 * Step.11 System exit 系统反初始化
 *		@code
 *			ret = IMP_ISP_DisableTuning() //不使能ISP tuning
 *			ret = IMP_System_Exit() //系统反初始化
 *			ret = IMP_ISP_DisableSensor(IMPVI_MAIN, &sensor_info[*]) //不使能sensor, sensor停止输出图像 (IMPVI_MAIN为主摄, IMPVI_SEC为次摄, IMPVI_THR为第三摄)
 *			ret = IMP_ISP_DelSensor(IMPVI_MAIN, &sensor_info[*]) //删除sensor (IMPVI_MAIN为主摄, IMPVI_SEC为次摄, IMPVI_THR为第三摄)
 *			ret = IMP_ISP_Close() //关闭ISP模块
 *		@endcode
 * */

#include <stdio.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

#include "sample-common.h"

#define TAG "Sample-Encoder-jpeg"

extern struct chn_conf chn[];

int main(int argc, char *argv[])
{
	int i, ret;

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
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -1;
			}
		}
	}

	ret = sample_jpeg_init();
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


	ret = IMP_Encoder_StartRecvPic(4);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 4 + chn[i].index);
		return -1;
	}

	/* Step.5 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}

	/* Step.6 Get Snap */
	ret = sample_get_jpeg_snap(NR_FRAMES_TO_SAVE / 10);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get H264 stream failed\n");
		return -1;
	}
	/* drop several pictures of invalid data */
	/* sleep(SLEEP_TIME); */
	/* Exit sequence as follow... */
	/* Step.7 Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.8 UnBind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.9 Encoder exit */
	ret = sample_jpeg_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder jpeg exit failed\n");
		return -1;
	}

	/* Step.10 FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.11 System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	return 0;
}
